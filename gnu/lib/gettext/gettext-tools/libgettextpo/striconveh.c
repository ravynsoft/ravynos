/* Character set conversion with error handling.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.
   Written by Bruno Haible and Simon Josefsson.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "striconveh.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_ICONV
# include <iconv.h>
# include "unistr.h"
#endif

#include "c-strcase.h"
#include "c-strcaseeq.h"

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif


#if HAVE_ICONV

/* The caller must provide an iconveh_t, not just an iconv_t, because when a
   conversion error occurs, we may have to determine the Unicode representation
   of the inconvertible character.  */

int
iconveh_open (const char *to_codeset, const char *from_codeset, iconveh_t *cdp)
{
  iconv_t cd;
  iconv_t cd1;
  iconv_t cd2;

  /* Avoid glibc-2.1 bug with EUC-KR.  */
# if ((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
     && !defined _LIBICONV_VERSION
  if (c_strcasecmp (from_codeset, "EUC-KR") == 0
      || c_strcasecmp (to_codeset, "EUC-KR") == 0)
    {
      errno = EINVAL;
      return -1;
    }
# endif

  cd = iconv_open (to_codeset, from_codeset);

  if (STRCASEEQ (from_codeset, "UTF-8", 'U','T','F','-','8',0,0,0,0))
    cd1 = (iconv_t)(-1);
  else
    {
      cd1 = iconv_open ("UTF-8", from_codeset);
      if (cd1 == (iconv_t)(-1))
        {
          int saved_errno = errno;
          if (cd != (iconv_t)(-1))
            iconv_close (cd);
          errno = saved_errno;
          return -1;
        }
    }

  if (STRCASEEQ (to_codeset, "UTF-8", 'U','T','F','-','8',0,0,0,0)
# if (((__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2) || __GLIBC__ > 2) \
      && !defined __UCLIBC__) \
     || _LIBICONV_VERSION >= 0x0105
      || c_strcasecmp (to_codeset, "UTF-8//TRANSLIT") == 0
# endif
     )
    cd2 = (iconv_t)(-1);
  else
    {
      cd2 = iconv_open (to_codeset, "UTF-8");
      if (cd2 == (iconv_t)(-1))
        {
          int saved_errno = errno;
          if (cd1 != (iconv_t)(-1))
            iconv_close (cd1);
          if (cd != (iconv_t)(-1))
            iconv_close (cd);
          errno = saved_errno;
          return -1;
        }
    }

  cdp->cd = cd;
  cdp->cd1 = cd1;
  cdp->cd2 = cd2;
  return 0;
}

int
iconveh_close (const iconveh_t *cd)
{
  if (cd->cd2 != (iconv_t)(-1) && iconv_close (cd->cd2) < 0)
    {
      /* Return -1, but preserve the errno from iconv_close.  */
      int saved_errno = errno;
      if (cd->cd1 != (iconv_t)(-1))
        iconv_close (cd->cd1);
      if (cd->cd != (iconv_t)(-1))
        iconv_close (cd->cd);
      errno = saved_errno;
      return -1;
    }
  if (cd->cd1 != (iconv_t)(-1) && iconv_close (cd->cd1) < 0)
    {
      /* Return -1, but preserve the errno from iconv_close.  */
      int saved_errno = errno;
      if (cd->cd != (iconv_t)(-1))
        iconv_close (cd->cd);
      errno = saved_errno;
      return -1;
    }
  if (cd->cd != (iconv_t)(-1) && iconv_close (cd->cd) < 0)
    return -1;
  return 0;
}

/* iconv_carefully is like iconv, except that it stops as soon as it encounters
   a conversion error, and it returns in *INCREMENTED a boolean telling whether
   it has incremented the input pointers past the error location.  */
# if !defined _LIBICONV_VERSION && !(defined __GLIBC__ && !defined __UCLIBC__)
/* Irix iconv() inserts a NUL byte if it cannot convert.
   NetBSD iconv() inserts a question mark if it cannot convert.
   Only GNU libiconv and GNU libc are known to prefer to fail rather
   than doing a lossy conversion.  */
static size_t
iconv_carefully (iconv_t cd,
                 const char **inbuf, size_t *inbytesleft,
                 char **outbuf, size_t *outbytesleft,
                 bool *incremented)
{
  const char *inptr = *inbuf;
  const char *inptr_end = inptr + *inbytesleft;
  char *outptr = *outbuf;
  size_t outsize = *outbytesleft;
  const char *inptr_before;
  size_t res;

  do
    {
      size_t insize;

      inptr_before = inptr;
      res = (size_t)(-1);

      for (insize = 1; inptr + insize <= inptr_end; insize++)
        {
          res = iconv (cd,
                       (ICONV_CONST char **) &inptr, &insize,
                       &outptr, &outsize);
          if (!(res == (size_t)(-1) && errno == EINVAL))
            break;
          /* iconv can eat up a shift sequence but give EINVAL while attempting
             to convert the first character.  E.g. libiconv does this.  */
          if (inptr > inptr_before)
            {
              res = 0;
              break;
            }
        }

      if (res == 0)
        {
          *outbuf = outptr;
          *outbytesleft = outsize;
        }
    }
  while (res == 0 && inptr < inptr_end);

  *inbuf = inptr;
  *inbytesleft = inptr_end - inptr;
  if (res != (size_t)(-1) && res > 0)
    {
      /* iconv() has already incremented INPTR.  We cannot go back to a
         previous INPTR, otherwise the state inside CD would become invalid,
         if FROM_CODESET is a stateful encoding.  So, tell the caller that
         *INBUF has already been incremented.  */
      *incremented = (inptr > inptr_before);
      errno = EILSEQ;
      return (size_t)(-1);
    }
  else
    {
      *incremented = false;
      return res;
    }
}
# else
#  define iconv_carefully(cd, inbuf, inbytesleft, outbuf, outbytesleft, incremented) \
     (*(incremented) = false, \
      iconv (cd, (ICONV_CONST char **) (inbuf), inbytesleft, outbuf, outbytesleft))
# endif

/* iconv_carefully_1 is like iconv_carefully, except that it stops after
   converting one character or one shift sequence.  */
static size_t
iconv_carefully_1 (iconv_t cd,
                   const char **inbuf, size_t *inbytesleft,
                   char **outbuf, size_t *outbytesleft,
                   bool *incremented)
{
  const char *inptr_before = *inbuf;
  const char *inptr = inptr_before;
  const char *inptr_end = inptr_before + *inbytesleft;
  char *outptr = *outbuf;
  size_t outsize = *outbytesleft;
  size_t res = (size_t)(-1);
  size_t insize;

  for (insize = 1; inptr_before + insize <= inptr_end; insize++)
    {
      inptr = inptr_before;
      res = iconv (cd,
                   (ICONV_CONST char **) &inptr, &insize,
                   &outptr, &outsize);
      if (!(res == (size_t)(-1) && errno == EINVAL))
        break;
      /* iconv can eat up a shift sequence but give EINVAL while attempting
         to convert the first character.  E.g. libiconv does this.  */
      if (inptr > inptr_before)
        {
          res = 0;
          break;
        }
    }

  *inbuf = inptr;
  *inbytesleft = inptr_end - inptr;
# if !defined _LIBICONV_VERSION && !(defined __GLIBC__ && !defined __UCLIBC__)
  /* Irix iconv() inserts a NUL byte if it cannot convert.
     NetBSD iconv() inserts a question mark if it cannot convert.
     Only GNU libiconv and GNU libc are known to prefer to fail rather
     than doing a lossy conversion.  */
  if (res != (size_t)(-1) && res > 0)
    {
      /* iconv() has already incremented INPTR.  We cannot go back to a
         previous INPTR, otherwise the state inside CD would become invalid,
         if FROM_CODESET is a stateful encoding.  So, tell the caller that
         *INBUF has already been incremented.  */
      *incremented = (inptr > inptr_before);
      errno = EILSEQ;
      return (size_t)(-1);
    }
# endif

  if (res != (size_t)(-1))
    {
      *outbuf = outptr;
      *outbytesleft = outsize;
    }
  *incremented = false;
  return res;
}

/* utf8conv_carefully is like iconv, except that
     - it converts from UTF-8 to UTF-8,
     - it stops as soon as it encounters a conversion error, and it returns
       in *INCREMENTED a boolean telling whether it has incremented the input
       pointers past the error location,
     - if one_character_only is true, it stops after converting one
       character.  */
static size_t
utf8conv_carefully (bool one_character_only,
                    const char **inbuf, size_t *inbytesleft,
                    char **outbuf, size_t *outbytesleft,
                    bool *incremented)
{
  const char *inptr = *inbuf;
  size_t insize = *inbytesleft;
  char *outptr = *outbuf;
  size_t outsize = *outbytesleft;
  size_t res;

  res = 0;
  do
    {
      ucs4_t uc;
      int n;
      int m;

      n = u8_mbtoucr (&uc, (const uint8_t *) inptr, insize);
      if (n < 0)
        {
          errno = (n == -2 ? EINVAL : EILSEQ);
          n = u8_mbtouc (&uc, (const uint8_t *) inptr, insize);
          inptr += n;
          insize -= n;
          res = (size_t)(-1);
          *incremented = true;
          break;
        }
      if (outsize == 0)
        {
          errno = E2BIG;
          res = (size_t)(-1);
          *incremented = false;
          break;
        }
      m = u8_uctomb ((uint8_t *) outptr, uc, outsize);
      if (m == -2)
        {
          errno = E2BIG;
          res = (size_t)(-1);
          *incremented = false;
          break;
        }
      inptr += n;
      insize -= n;
      if (m == -1)
        {
          errno = EILSEQ;
          res = (size_t)(-1);
          *incremented = true;
          break;
        }
      outptr += m;
      outsize -= m;
    }
  while (!one_character_only && insize > 0);

  *inbuf = inptr;
  *inbytesleft = insize;
  *outbuf = outptr;
  *outbytesleft = outsize;
  return res;
}

static int
mem_cd_iconveh_internal (const char *src, size_t srclen,
                         iconv_t cd, iconv_t cd1, iconv_t cd2,
                         enum iconv_ilseq_handler handler,
                         size_t extra_alloc,
                         size_t *offsets,
                         char **resultp, size_t *lengthp)
{
  /* When a conversion error occurs, we cannot start using CD1 and CD2 at
     this point: FROM_CODESET may be a stateful encoding like ISO-2022-KR.
     Instead, we have to start afresh from the beginning of SRC.  */
  /* Use a temporary buffer, so that for small strings, a single malloc()
     call will be sufficient.  */
# define tmpbufsize 4096
  /* The alignment is needed when converting e.g. to glibc's WCHAR_T or
     libiconv's UCS-4-INTERNAL encoding.  */
  union { unsigned int align; char buf[tmpbufsize]; } tmp;
# define tmpbuf tmp.buf

  char *initial_result;
  char *result;
  size_t allocated;
  size_t length;
  size_t last_length = (size_t)(-1); /* only needed if offsets != NULL */

  if (*resultp != NULL && *lengthp >= sizeof (tmpbuf))
    {
      initial_result = *resultp;
      allocated = *lengthp;
    }
  else
    {
      initial_result = tmpbuf;
      allocated = sizeof (tmpbuf);
    }
  result = initial_result;

  /* Test whether a direct conversion is possible at all.  */
  if (cd == (iconv_t)(-1))
    goto indirectly;

  if (offsets != NULL)
    {
      size_t i;

      for (i = 0; i < srclen; i++)
        offsets[i] = (size_t)(-1);

      last_length = (size_t)(-1);
    }
  length = 0;

  /* First, try a direct conversion, and see whether a conversion error
     occurs at all.  */
  {
    const char *inptr = src;
    size_t insize = srclen;

    /* Avoid glibc-2.1 bug and Solaris 2.7-2.9 bug.  */
# if defined _LIBICONV_VERSION \
     || !(((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
          || defined __sun)
    /* Set to the initial state.  */
    iconv (cd, NULL, NULL, NULL, NULL);
# endif

    while (insize > 0)
      {
        char *outptr = result + length;
        size_t outsize = allocated - extra_alloc - length;
        bool incremented;
        size_t res;
        bool grow;

        if (offsets != NULL)
          {
            if (length != last_length) /* ensure that offset[] be increasing */
              {
                offsets[inptr - src] = length;
                last_length = length;
              }
            res = iconv_carefully_1 (cd,
                                     &inptr, &insize,
                                     &outptr, &outsize,
                                     &incremented);
          }
        else
          /* Use iconv_carefully instead of iconv here, because:
             - If TO_CODESET is UTF-8, we can do the error handling in this
               loop, no need for a second loop,
             - With iconv() implementations other than GNU libiconv and GNU
               libc, if we use iconv() in a big swoop, checking for an E2BIG
               return, we lose the number of irreversible conversions.  */
          res = iconv_carefully (cd,
                                 &inptr, &insize,
                                 &outptr, &outsize,
                                 &incremented);

        length = outptr - result;
        grow = (length + extra_alloc > allocated / 2);
        if (res == (size_t)(-1))
          {
            if (errno == E2BIG)
              grow = true;
            else if (errno == EINVAL)
              break;
            else if (errno == EILSEQ && handler != iconveh_error)
              {
                if (cd2 == (iconv_t)(-1))
                  {
                    /* TO_CODESET is UTF-8.  */
                    /* Error handling can produce up to 1 or 3 bytes of
                       output.  */
                    size_t extra_need =
                      (handler == iconveh_replacement_character ? 3 : 1);
                    if (length + extra_need + extra_alloc > allocated)
                      {
                        char *memory;

                        allocated = 2 * allocated;
                        if (length + extra_need + extra_alloc > allocated)
                          allocated = 2 * allocated;
                        if (length + extra_need + extra_alloc > allocated)
                          abort ();
                        if (result == initial_result)
                          memory = (char *) malloc (allocated);
                        else
                          memory = (char *) realloc (result, allocated);
                        if (memory == NULL)
                          {
                            if (result != initial_result)
                              free (result);
                            errno = ENOMEM;
                            return -1;
                          }
                        if (result == initial_result)
                          memcpy (memory, initial_result, length);
                        result = memory;
                        grow = false;
                      }
                    /* The input is invalid in FROM_CODESET.  Eat up one byte
                       and emit a replacement character or a question mark.  */
                    if (!incremented)
                      {
                        if (insize == 0)
                          abort ();
                        inptr++;
                        insize--;
                      }
                    if (handler == iconveh_replacement_character)
                      {
                        /* U+FFFD in UTF-8 encoding.  */
                        result[length+0] = '\357';
                        result[length+1] = '\277';
                        result[length+2] = '\275';
                        length += 3;
                      }
                    else
                      {
                        result[length] = '?';
                        length++;
                      }
                  }
                else
                  goto indirectly;
              }
            else
              {
                if (result != initial_result)
                  free (result);
                return -1;
              }
          }
        if (insize == 0)
          break;
        if (grow)
          {
            char *memory;

            allocated = 2 * allocated;
            if (result == initial_result)
              memory = (char *) malloc (allocated);
            else
              memory = (char *) realloc (result, allocated);
            if (memory == NULL)
              {
                if (result != initial_result)
                  free (result);
                errno = ENOMEM;
                return -1;
              }
            if (result == initial_result)
              memcpy (memory, initial_result, length);
            result = memory;
          }
      }
  }

  /* Now get the conversion state back to the initial state.
     But avoid glibc-2.1 bug and Solaris 2.7 bug.  */
#if defined _LIBICONV_VERSION \
    || !(((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
         || defined __sun)
  for (;;)
    {
      char *outptr = result + length;
      size_t outsize = allocated - extra_alloc - length;
      size_t res;

      res = iconv (cd, NULL, NULL, &outptr, &outsize);
      length = outptr - result;
      if (res == (size_t)(-1))
        {
          if (errno == E2BIG)
            {
              char *memory;

              allocated = 2 * allocated;
              if (result == initial_result)
                memory = (char *) malloc (allocated);
              else
                memory = (char *) realloc (result, allocated);
              if (memory == NULL)
                {
                  if (result != initial_result)
                    free (result);
                  errno = ENOMEM;
                  return -1;
                }
              if (result == initial_result)
                memcpy (memory, initial_result, length);
              result = memory;
            }
          else
            {
              if (result != initial_result)
                free (result);
              return -1;
            }
        }
      else
        break;
    }
#endif

  /* The direct conversion succeeded.  */
  goto done;

 indirectly:
  /* The direct conversion failed.
     Use a conversion through UTF-8.  */
  if (offsets != NULL)
    {
      size_t i;

      for (i = 0; i < srclen; i++)
        offsets[i] = (size_t)(-1);

      last_length = (size_t)(-1);
    }
  length = 0;
  {
    const bool slowly = (offsets != NULL || handler == iconveh_error);
# define utf8bufsize 4096 /* may also be smaller or larger than tmpbufsize */
    char utf8buf[utf8bufsize + 3];
    size_t utf8len = 0;
    const char *in1ptr = src;
    size_t in1size = srclen;
    bool do_final_flush1 = true;
    bool do_final_flush2 = true;

    /* Avoid glibc-2.1 bug and Solaris 2.7-2.9 bug.  */
# if defined _LIBICONV_VERSION \
     || !(((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
          || defined __sun)
    /* Set to the initial state.  */
    if (cd1 != (iconv_t)(-1))
      iconv (cd1, NULL, NULL, NULL, NULL);
    if (cd2 != (iconv_t)(-1))
      iconv (cd2, NULL, NULL, NULL, NULL);
# endif

    while (in1size > 0 || do_final_flush1 || utf8len > 0 || do_final_flush2)
      {
        char *out1ptr = utf8buf + utf8len;
        size_t out1size = utf8bufsize - utf8len;
        bool incremented1;
        size_t res1;
        int errno1;

        /* Conversion step 1: from FROM_CODESET to UTF-8.  */
        if (in1size > 0)
          {
            if (offsets != NULL
                && length != last_length) /* ensure that offset[] be increasing */
              {
                offsets[in1ptr - src] = length;
                last_length = length;
              }
            if (cd1 != (iconv_t)(-1))
              {
                if (slowly)
                  res1 = iconv_carefully_1 (cd1,
                                            &in1ptr, &in1size,
                                            &out1ptr, &out1size,
                                            &incremented1);
                else
                  res1 = iconv_carefully (cd1,
                                          &in1ptr, &in1size,
                                          &out1ptr, &out1size,
                                          &incremented1);
              }
            else
              {
                /* FROM_CODESET is UTF-8.  */
                res1 = utf8conv_carefully (slowly,
                                           &in1ptr, &in1size,
                                           &out1ptr, &out1size,
                                           &incremented1);
              }
          }
        else if (do_final_flush1)
          {
            /* Now get the conversion state of CD1 back to the initial state.
               But avoid glibc-2.1 bug and Solaris 2.7 bug.  */
# if defined _LIBICONV_VERSION \
     || !(((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
          || defined __sun)
            if (cd1 != (iconv_t)(-1))
              res1 = iconv (cd1, NULL, NULL, &out1ptr, &out1size);
            else
# endif
              res1 = 0;
            do_final_flush1 = false;
            incremented1 = true;
          }
        else
          {
            res1 = 0;
            incremented1 = true;
          }
        if (res1 == (size_t)(-1)
            && !(errno == E2BIG || errno == EINVAL || errno == EILSEQ))
          {
            if (result != initial_result)
              free (result);
            return -1;
          }
        if (res1 == (size_t)(-1)
            && errno == EILSEQ && handler != iconveh_error)
          {
            /* The input is invalid in FROM_CODESET.  Eat up one byte and
               emit a U+FFFD character or a question mark.  Room for this
               character was allocated at the end of utf8buf.  */
            if (!incremented1)
              {
                if (in1size == 0)
                  abort ();
                in1ptr++;
                in1size--;
              }
            if (handler == iconveh_replacement_character)
              {
                /* U+FFFD in UTF-8 encoding.  */
                out1ptr[0] = '\357';
                out1ptr[1] = '\277';
                out1ptr[2] = '\275';
                out1ptr += 3;
              }
            else
              *out1ptr++ = '?';
            res1 = 0;
          }
        errno1 = errno;
        utf8len = out1ptr - utf8buf;

        if (offsets != NULL
            || in1size == 0
            || utf8len > utf8bufsize / 2
            || (res1 == (size_t)(-1) && errno1 == E2BIG))
          {
            /* Conversion step 2: from UTF-8 to TO_CODESET.  */
            const char *in2ptr = utf8buf;
            size_t in2size = utf8len;

            while (in2size > 0
                   || (in1size == 0 && !do_final_flush1 && do_final_flush2))
              {
                char *out2ptr = result + length;
                size_t out2size = allocated - extra_alloc - length;
                bool incremented2;
                size_t res2;
                bool grow;

                if (in2size > 0)
                  {
                    if (cd2 != (iconv_t)(-1))
                      res2 = iconv_carefully (cd2,
                                              &in2ptr, &in2size,
                                              &out2ptr, &out2size,
                                              &incremented2);
                    else
                      /* TO_CODESET is UTF-8.  */
                      res2 = utf8conv_carefully (false,
                                                 &in2ptr, &in2size,
                                                 &out2ptr, &out2size,
                                                 &incremented2);
                  }
                else /* in1size == 0 && !do_final_flush1
                        && in2size == 0 && do_final_flush2 */
                  {
                    /* Now get the conversion state of CD1 back to the initial
                       state.  But avoid glibc-2.1 bug and Solaris 2.7 bug.  */
# if defined _LIBICONV_VERSION \
     || !(((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
          || defined __sun)
                    if (cd2 != (iconv_t)(-1))
                      res2 = iconv (cd2, NULL, NULL, &out2ptr, &out2size);
                    else
# endif
                      res2 = 0;
                    do_final_flush2 = false;
                    incremented2 = true;
                  }

                length = out2ptr - result;
                grow = (length + extra_alloc > allocated / 2);
                if (res2 == (size_t)(-1))
                  {
                    if (errno == E2BIG)
                      grow = true;
                    else if (errno == EINVAL)
                      break;
                    else if (errno == EILSEQ && handler != iconveh_error)
                      {
                        /* Error handling can produce up to 10 bytes of UTF-8
                           output.  But TO_CODESET may be UCS-2, UTF-16 or
                           UCS-4, so use CD2 here as well.  */
                        char scratchbuf[10];
                        size_t scratchlen;
                        ucs4_t uc;
                        const char *inptr;
                        size_t insize;
                        size_t res;

                        if (incremented2)
                          {
                            if (u8_prev (&uc, (const uint8_t *) in2ptr,
                                         (const uint8_t *) utf8buf)
                                == NULL)
                              abort ();
                          }
                        else
                          {
                            int n;
                            if (in2size == 0)
                              abort ();
                            n = u8_mbtouc_unsafe (&uc, (const uint8_t *) in2ptr,
                                                  in2size);
                            in2ptr += n;
                            in2size -= n;
                          }

                        if (handler == iconveh_escape_sequence)
                          {
                            static char const hex[16] = "0123456789ABCDEF";
                            scratchlen = 0;
                            scratchbuf[scratchlen++] = '\\';
                            if (uc < 0x10000)
                              scratchbuf[scratchlen++] = 'u';
                            else
                              {
                                scratchbuf[scratchlen++] = 'U';
                                scratchbuf[scratchlen++] = hex[(uc>>28) & 15];
                                scratchbuf[scratchlen++] = hex[(uc>>24) & 15];
                                scratchbuf[scratchlen++] = hex[(uc>>20) & 15];
                                scratchbuf[scratchlen++] = hex[(uc>>16) & 15];
                              }
                            scratchbuf[scratchlen++] = hex[(uc>>12) & 15];
                            scratchbuf[scratchlen++] = hex[(uc>>8) & 15];
                            scratchbuf[scratchlen++] = hex[(uc>>4) & 15];
                            scratchbuf[scratchlen++] = hex[uc & 15];
                          }
                        else if (handler == iconveh_replacement_character)
                          {
                            /* U+FFFD in UTF-8 encoding.  */
                            scratchbuf[0] = '\357';
                            scratchbuf[1] = '\277';
                            scratchbuf[2] = '\275';
                            scratchlen = 3;
                          }
                        else
                          {
                            scratchbuf[0] = '?';
                            scratchlen = 1;
                          }

                        inptr = scratchbuf;
                        insize = scratchlen;
                        if (cd2 != (iconv_t)(-1))
                          {
                            char *out2ptr_try = out2ptr;
                            size_t out2size_try = out2size;
                            res = iconv (cd2,
                                         (ICONV_CONST char **) &inptr, &insize,
                                         &out2ptr_try, &out2size_try);
                            if (handler == iconveh_replacement_character
                                && (res == (size_t)(-1)
                                    ? errno == EILSEQ
                                    /* FreeBSD iconv(), NetBSD iconv(), and
                                       Solaris 11 iconv() insert a '?' if they
                                       cannot convert.  This is what we want.
                                       But IRIX iconv() inserts a NUL byte if it
                                       cannot convert.
                                       And musl libc iconv() inserts a '*' if it
                                       cannot convert.  */
                                    : (res > 0
                                       && !(out2ptr_try - out2ptr == 1
                                            && *out2ptr == '?'))))
                              {
                                /* The iconv() call failed.
                                   U+FFFD can't be converted to TO_CODESET.
                                   Use '?' instead.  */
                                scratchbuf[0] = '?';
                                scratchlen = 1;
                                inptr = scratchbuf;
                                insize = scratchlen;
                                res = iconv (cd2,
                                             (ICONV_CONST char **) &inptr, &insize,
                                             &out2ptr, &out2size);
                              }
                            else
                              {
                                /* Accept the results of the iconv() call.  */
                                out2ptr = out2ptr_try;
                                out2size = out2size_try;
                                res = 0;
                              }
                          }
                        else
                          {
                            /* TO_CODESET is UTF-8.  */
                            if (out2size >= insize)
                              {
                                memcpy (out2ptr, inptr, insize);
                                out2ptr += insize;
                                out2size -= insize;
                                inptr += insize;
                                insize = 0;
                                res = 0;
                              }
                            else
                              {
                                errno = E2BIG;
                                res = (size_t)(-1);
                              }
                          }
                        length = out2ptr - result;
                        if (res == (size_t)(-1) && errno == E2BIG)
                          {
                            char *memory;

                            allocated = 2 * allocated;
                            if (length + 1 + extra_alloc > allocated)
                              abort ();
                            if (result == initial_result)
                              memory = (char *) malloc (allocated);
                            else
                              memory = (char *) realloc (result, allocated);
                            if (memory == NULL)
                              {
                                if (result != initial_result)
                                  free (result);
                                errno = ENOMEM;
                                return -1;
                              }
                            if (result == initial_result)
                              memcpy (memory, initial_result, length);
                            result = memory;
                            grow = false;

                            out2ptr = result + length;
                            out2size = allocated - extra_alloc - length;
                            if (cd2 != (iconv_t)(-1))
                              res = iconv (cd2,
                                           (ICONV_CONST char **) &inptr,
                                           &insize,
                                           &out2ptr, &out2size);
                            else
                              {
                                /* TO_CODESET is UTF-8.  */
                                if (!(out2size >= insize))
                                  abort ();
                                memcpy (out2ptr, inptr, insize);
                                out2ptr += insize;
                                out2size -= insize;
                                inptr += insize;
                                insize = 0;
                                res = 0;
                              }
                            length = out2ptr - result;
                          }
# if !defined _LIBICONV_VERSION && !(defined __GLIBC__ && !defined __UCLIBC__)
                        /* IRIX iconv() inserts a NUL byte if it cannot convert.
                           FreeBSD iconv(), NetBSD iconv(), and Solaris 11
                           iconv() insert a '?' if they cannot convert.
                           musl libc iconv() inserts a '*' if it cannot convert.
                           Only GNU libiconv and GNU libc are known to prefer
                           to fail rather than doing a lossy conversion.  */
                        if (res != (size_t)(-1) && res > 0)
                          {
                            errno = EILSEQ;
                            res = (size_t)(-1);
                          }
# endif
                        if (res == (size_t)(-1))
                          {
                            /* Failure converting the ASCII replacement.  */
                            if (result != initial_result)
                              free (result);
                            return -1;
                          }
                      }
                    else
                      {
                        if (result != initial_result)
                          free (result);
                        return -1;
                      }
                  }
                if (!(in2size > 0
                      || (in1size == 0 && !do_final_flush1 && do_final_flush2)))
                  break;
                if (grow)
                  {
                    char *memory;

                    allocated = 2 * allocated;
                    if (result == initial_result)
                      memory = (char *) malloc (allocated);
                    else
                      memory = (char *) realloc (result, allocated);
                    if (memory == NULL)
                      {
                        if (result != initial_result)
                          free (result);
                        errno = ENOMEM;
                        return -1;
                      }
                    if (result == initial_result)
                      memcpy (memory, initial_result, length);
                    result = memory;
                  }
              }

            /* Move the remaining bytes to the beginning of utf8buf.  */
            if (in2size > 0)
              memmove (utf8buf, in2ptr, in2size);
            utf8len = in2size;
          }

        if (res1 == (size_t)(-1))
          {
            if (errno1 == EINVAL)
              in1size = 0;
            else if (errno1 == EILSEQ)
              {
                if (result != initial_result)
                  free (result);
                errno = errno1;
                return -1;
              }
          }
      }
# undef utf8bufsize
  }

 done:
  /* Now the final memory allocation.  */
  if (result == tmpbuf)
    {
      size_t memsize = length + extra_alloc;

      if (*resultp != NULL && *lengthp >= memsize)
        result = *resultp;
      else
        {
          char *memory;

          memory = (char *) malloc (memsize > 0 ? memsize : 1);
          if (memory != NULL)
            result = memory;
          else
            {
              errno = ENOMEM;
              return -1;
            }
        }
      memcpy (result, tmpbuf, length);
    }
  else if (result != *resultp && length + extra_alloc < allocated)
    {
      /* Shrink the allocated memory if possible.  */
      size_t memsize = length + extra_alloc;
      char *memory;

      memory = (char *) realloc (result, memsize > 0 ? memsize : 1);
      if (memory != NULL)
        result = memory;
    }
  *resultp = result;
  *lengthp = length;
  return 0;
# undef tmpbuf
# undef tmpbufsize
}

int
mem_cd_iconveh (const char *src, size_t srclen,
                const iconveh_t *cd,
                enum iconv_ilseq_handler handler,
                size_t *offsets,
                char **resultp, size_t *lengthp)
{
  return mem_cd_iconveh_internal (src, srclen, cd->cd, cd->cd1, cd->cd2,
                                  handler, 0, offsets, resultp, lengthp);
}

char *
str_cd_iconveh (const char *src,
                const iconveh_t *cd,
                enum iconv_ilseq_handler handler)
{
  /* For most encodings, a trailing NUL byte in the input will be converted
     to a trailing NUL byte in the output.  But not for UTF-7.  So that this
     function is usable for UTF-7, we have to exclude the NUL byte from the
     conversion and add it by hand afterwards.  */
  char *result = NULL;
  size_t length = 0;
  int retval = mem_cd_iconveh_internal (src, strlen (src),
                                        cd->cd, cd->cd1, cd->cd2, handler, 1,
                                        NULL, &result, &length);

  if (retval < 0)
    {
      free (result);
      return NULL;
    }

  /* Add the terminating NUL byte.  */
  result[length] = '\0';

  return result;
}

#endif

int
mem_iconveh (const char *src, size_t srclen,
             const char *from_codeset, const char *to_codeset,
             enum iconv_ilseq_handler handler,
             size_t *offsets,
             char **resultp, size_t *lengthp)
{
  if (srclen == 0)
    {
      /* Nothing to convert.  */
      *lengthp = 0;
      return 0;
    }
  else if (offsets == NULL && c_strcasecmp (from_codeset, to_codeset) == 0)
    {
      char *result;

      if (*resultp != NULL && *lengthp >= srclen)
        result = *resultp;
      else
        {
          result = (char *) malloc (srclen);
          if (result == NULL)
            {
              errno = ENOMEM;
              return -1;
            }
        }
      memcpy (result, src, srclen);
      *resultp = result;
      *lengthp = srclen;
      return 0;
    }
  else
    {
#if HAVE_ICONV
      iconveh_t cd;
      char *result;
      size_t length;
      int retval;

      if (iconveh_open (to_codeset, from_codeset, &cd) < 0)
        return -1;

      result = *resultp;
      length = *lengthp;
      retval = mem_cd_iconveh (src, srclen, &cd, handler, offsets,
                               &result, &length);

      if (retval < 0)
        {
          /* Close cd, but preserve the errno from str_cd_iconv.  */
          int saved_errno = errno;
          iconveh_close (&cd);
          errno = saved_errno;
        }
      else
        {
          if (iconveh_close (&cd) < 0)
            {
              if (result != *resultp)
                free (result);
              return -1;
            }
          *resultp = result;
          *lengthp = length;
        }
      return retval;
#else
      /* This is a different error code than if iconv_open existed but didn't
         support from_codeset and to_codeset, so that the caller can emit
         an error message such as
           "iconv() is not supported. Installing GNU libiconv and
            then reinstalling this package would fix this."  */
      errno = ENOSYS;
      return -1;
#endif
    }
}

char *
str_iconveh (const char *src,
             const char *from_codeset, const char *to_codeset,
             enum iconv_ilseq_handler handler)
{
  if (*src == '\0' || c_strcasecmp (from_codeset, to_codeset) == 0)
    {
      char *result = strdup (src);

      if (result == NULL)
        errno = ENOMEM;
      return result;
    }
  else
    {
#if HAVE_ICONV
      iconveh_t cd;
      char *result;

      if (iconveh_open (to_codeset, from_codeset, &cd) < 0)
        return NULL;

      result = str_cd_iconveh (src, &cd, handler);

      if (result == NULL)
        {
          /* Close cd, but preserve the errno from str_cd_iconv.  */
          int saved_errno = errno;
          iconveh_close (&cd);
          errno = saved_errno;
        }
      else
        {
          if (iconveh_close (&cd) < 0)
            {
              free (result);
              return NULL;
            }
        }
      return result;
#else
      /* This is a different error code than if iconv_open existed but didn't
         support from_codeset and to_codeset, so that the caller can emit
         an error message such as
           "iconv() is not supported. Installing GNU libiconv and
            then reinstalling this package would fix this."  */
      errno = ENOSYS;
      return NULL;
#endif
    }
}
