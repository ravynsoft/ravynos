/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#if !IS_CPLUSPLUS
#define iconv_ostream_representation any_ostream_representation
#endif
#line 1 "iconv-ostream.oo.c"
/* Output stream that converts the output to another encoding.
   Copyright (C) 2006-2007, 2010, 2019-2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "iconv-ostream.h"

#if HAVE_ICONV

#include <errno.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>

#include "c-strcase.h"
#include "error.h"
#include "xalloc.h"
#include "gettext.h"

#define _(str) gettext (str)

#endif /* HAVE_ICONV */

#line 46 "iconv-ostream.c"
#include "iconv_ostream.priv.h"

const typeinfo_t iconv_ostream_typeinfo = { "iconv_ostream" };

static const typeinfo_t * const iconv_ostream_superclasses[] =
  { iconv_ostream_SUPERCLASSES };

#define super ostream_vtable

#line 56 "iconv-ostream.oo.c"

#if HAVE_ICONV

/* Implementation of ostream_t methods.  */

static void
iconv_ostream__write_mem (iconv_ostream_t stream, const void *data, size_t len)
{
  if (len > 0)
    {
      #define BUFFERSIZE 256
      char inbuffer[BUFFERSIZE];
      size_t inbufcount;

      inbufcount = stream->buflen;
      if (inbufcount > 0)
        memcpy (inbuffer, stream->buf, inbufcount);
      for (;;)
        {
          /* At this point, inbuffer[0..inbufcount-1] is filled.  */
          {
            /* Combine the previous rest with a chunk of new input.  */
            size_t n =
              (len <= BUFFERSIZE - inbufcount ? len : BUFFERSIZE - inbufcount);

            if (n > 0)
              {
                memcpy (inbuffer + inbufcount, data, n);
                data = (const char *) data + n;
                inbufcount += n;
                len -= n;
              }
          }
          {
            /* Attempt to convert the combined input.  */
            char outbuffer[8*BUFFERSIZE];

            const char *inptr = inbuffer;
            size_t insize = inbufcount;
            char *outptr = outbuffer;
            size_t outsize = sizeof (outbuffer);

            size_t res = iconv (stream->cd,
                                (ICONV_CONST char **) &inptr, &insize,
                                &outptr, &outsize);
            #if !defined _LIBICONV_VERSION \
                && !(defined __GLIBC__ && !defined __UCLIBC__)
            /* Irix iconv() inserts a NUL byte if it cannot convert.
               NetBSD iconv() inserts a question mark if it cannot convert.
               Only GNU libiconv and GNU libc are known to prefer to fail rather
               than doing a lossy conversion.  */
            if (res > 0)
              {
                errno = EILSEQ;
                res = -1;
              }
            #endif
            if (res == (size_t)(-1) && errno != EINVAL)
              error (EXIT_FAILURE, 0, _("%s: cannot convert from %s to %s"),
                     "iconv_ostream",
                     stream->from_encoding, stream->to_encoding);
            /* Output the converted part.  */
            if (sizeof (outbuffer) - outsize > 0)
              ostream_write_mem (stream->destination,
                                 outbuffer, sizeof (outbuffer) - outsize);
            /* Put back the unconverted part.  */
            if (insize > BUFSIZE)
              error (EXIT_FAILURE, 0, _("%s: shift sequence too long"),
                     "iconv_ostream");
            if (len == 0)
              {
                if (insize > 0)
                  memcpy (stream->buf, inptr, insize);
                stream->buflen = insize;
                break;
              }
            if (insize > 0)
              memmove (inbuffer, inptr, insize);
            inbufcount = insize;
          }
        }
      #undef BUFFERSIZE
    }
}

static void
iconv_ostream__flush (iconv_ostream_t stream, ostream_flush_scope_t scope)
{
  /* For scope == FLUSH_THIS_STREAM, there's nothing we can do here, since
     stream->buf[] contains only a few bytes that don't correspond to a
     character.  */
  if (scope != FLUSH_THIS_STREAM)
    ostream_flush (stream->destination, scope);
}

static void
iconv_ostream__free (iconv_ostream_t stream)
{
  /* Silently ignore the few bytes in stream->buf[] that don't correspond to a
     character.  */

  /* Avoid glibc-2.1 bug and Solaris 2.7 bug.  */
  #if defined _LIBICONV_VERSION \
      || !(((__GLIBC__ - 0 == 2 && __GLIBC_MINOR__ - 0 <= 1) \
            && !defined __UCLIBC__) \
           || defined __sun)
  {
    char outbuffer[2048];
    char *outptr = outbuffer;
    size_t outsize = sizeof (outbuffer);
    size_t res = iconv (stream->cd, NULL, NULL, &outptr, &outsize);
    if (res == (size_t)(-1))
      error (EXIT_FAILURE, 0, _("%s: cannot convert from %s to %s"),
             "iconv_ostream", stream->from_encoding, stream->to_encoding);
    /* Output the converted part.  */
    if (sizeof (outbuffer) - outsize > 0)
      ostream_write_mem (stream->destination,
                         outbuffer, sizeof (outbuffer) - outsize);
  }
  #endif

  iconv_close (stream->cd);
  free (stream->from_encoding);
  free (stream->to_encoding);
  free (stream);
}

/* Constructor.  */

iconv_ostream_t
iconv_ostream_create (const char *from_encoding, const char *to_encoding,
                      ostream_t destination)
{
  iconv_ostream_t stream = XMALLOC (struct iconv_ostream_representation);

  stream->base.vtable = &iconv_ostream_vtable;
  stream->destination = destination;
  stream->from_encoding = xstrdup (from_encoding);
  stream->to_encoding = xstrdup (to_encoding);

  /* Avoid glibc-2.1 bug with EUC-KR.  */
  #if ((__GLIBC__ - 0 == 2 && __GLIBC_MINOR__ - 0 <= 1) \
       && !defined __UCLIBC__) \
      && !defined _LIBICONV_VERSION
  if (c_strcasecmp (from_encoding, "EUC-KR") == 0
      || c_strcasecmp (to_encoding, "EUC-KR") == 0)
    stream->cd = (iconv_t)(-1):
  else
  #endif
    stream->cd = iconv_open (to_encoding, from_encoding);
  if (stream->cd == (iconv_t)(-1))
    {
      if (iconv_open ("UTF-8", from_encoding) == (iconv_t)(-1))
        error (EXIT_FAILURE, 0, _("%s does not support conversion from %s"),
               "iconv", from_encoding);
      else if (iconv_open (to_encoding, "UTF-8") == (iconv_t)(-1))
        error (EXIT_FAILURE, 0, _("%s does not support conversion to %s"),
               "iconv", to_encoding);
      else
        error (EXIT_FAILURE, 0,
               _("%s does not support conversion from %s to %s"),
               "iconv", from_encoding, to_encoding);
    }

  stream->buflen = 0;

  return stream;
}

/* Accessors.  */

static const char *
iconv_ostream__get_from_encoding (iconv_ostream_t stream)
{
  return stream->from_encoding;
}

static const char *
iconv_ostream__get_to_encoding (iconv_ostream_t stream)
{
  return stream->to_encoding;
}

static ostream_t
iconv_ostream__get_destination (iconv_ostream_t stream)
{
  return stream->destination;
}

/* Instanceof test.  */

bool
is_instance_of_iconv_ostream (ostream_t stream)
{
  return IS_INSTANCE (stream, ostream, iconv_ostream);
}

#else

static void
iconv_ostream__write_mem (iconv_ostream_t stream, const void *data, size_t len)
{
  abort ();
}

static void
iconv_ostream__flush (iconv_ostream_t stream)
{
  abort ();
}

static void
iconv_ostream__free (iconv_ostream_t stream)
{
  abort ();
}

/* Accessors.  */

static const char *
iconv_ostream__get_from_encoding (iconv_ostream_t stream)
{
  abort ();
}

static const char *
iconv_ostream__get_to_encoding (iconv_ostream_t stream)
{
  abort ();
}

static ostream_t
iconv_ostream__get_destination (iconv_ostream_t stream)
{
  abort ();
}

/* Instanceof test.  */

bool
is_instance_of_iconv_ostream (ostream_t stream)
{
  return false;
}

#endif /* HAVE_ICONV */

#line 304 "iconv-ostream.c"

const struct iconv_ostream_implementation iconv_ostream_vtable =
{
  iconv_ostream_superclasses,
  sizeof (iconv_ostream_superclasses) / sizeof (iconv_ostream_superclasses[0]),
  sizeof (struct iconv_ostream_representation),
  iconv_ostream__write_mem,
  iconv_ostream__flush,
  iconv_ostream__free,
  iconv_ostream__get_from_encoding,
  iconv_ostream__get_to_encoding,
  iconv_ostream__get_destination,
};

#if !HAVE_INLINE

/* Define the functions that invoke the methods.  */

void
iconv_ostream_write_mem (iconv_ostream_t first_arg, const void *data, size_t len)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

void
iconv_ostream_flush (iconv_ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

void
iconv_ostream_free (iconv_ostream_t first_arg)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

const char *
iconv_ostream_get_from_encoding (iconv_ostream_t first_arg)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  return vtable->get_from_encoding (first_arg);
}

const char *
iconv_ostream_get_to_encoding (iconv_ostream_t first_arg)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  return vtable->get_to_encoding (first_arg);
}

ostream_t
iconv_ostream_get_destination (iconv_ostream_t first_arg)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  return vtable->get_destination (first_arg);
}

#endif
