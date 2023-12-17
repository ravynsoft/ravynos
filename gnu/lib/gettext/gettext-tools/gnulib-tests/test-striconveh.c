/* Test of character set conversion with error handling.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#include "striconveh.h"

#if HAVE_ICONV
# include <iconv.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"

/* Magic number for detecting bounds violations.  */
#define MAGIC 0x1983EFF1

static size_t *
new_offsets (size_t n)
{
  size_t *offsets = (size_t *) malloc ((n + 1) * sizeof (size_t));
  offsets[n] = MAGIC;
  return offsets;
}

int
main ()
{
#if HAVE_ICONV
  static enum iconv_ilseq_handler handlers[] =
    {
      iconveh_error,
      iconveh_question_mark,
      iconveh_replacement_character,
      iconveh_escape_sequence
    };
  size_t indirect;
  size_t h;
  size_t o;
  size_t i;

  /* Assume that iconv() supports at least the encodings ASCII, ISO-8859-1,
     ISO-8859-2, UTF-8, and with libiconv or glibc also GB18030.  */
  iconv_t cd_ascii_to_88591 = iconv_open ("ISO-8859-1", "ASCII");
  iconv_t cd_88591_to_88592 = iconv_open ("ISO-8859-2", "ISO-8859-1");
  iconv_t cd_88592_to_88591 = iconv_open ("ISO-8859-1", "ISO-8859-2");
  iconv_t cd_ascii_to_utf8 = iconv_open ("UTF-8", "ASCII");
  iconv_t cd_88591_to_utf8 = iconv_open ("UTF-8", "ISO-8859-1");
  iconv_t cd_utf8_to_88591 = iconv_open ("ISO-8859-1", "UTF-8");
  iconv_t cd_88592_to_utf8 = iconv_open ("UTF-8", "ISO-8859-2");
  iconv_t cd_utf8_to_88592 = iconv_open ("ISO-8859-2", "UTF-8");
  iconv_t cd_utf7_to_utf8 = iconv_open ("UTF-8", "UTF-7");
# if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
  iconv_t cd_ascii_to_gb18030 = iconv_open ("GB18030", "ASCII");
  iconv_t cd_utf8_to_gb18030 = iconv_open ("GB18030", "UTF-8");
  iconv_t cd_88591_to_gb18030 = iconv_open ("GB18030", "ISO-8859-1");
  iconv_t cd_utf7_to_gb18030 = iconv_open ("GB18030", "UTF-7");
# endif
  iconveh_t cdeh_ascii_to_88591;
  iconveh_t cdeh_ascii_to_88591_indirectly;
  iconveh_t cdeh_88592_to_88591;
  iconveh_t cdeh_88592_to_88591_indirectly;
  iconveh_t cdeh_ascii_to_utf8;
  iconveh_t cdeh_88591_to_utf8;
  iconveh_t cdeh_utf8_to_88591;
  iconveh_t cdeh_utf7_to_utf8;
# if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
  iconveh_t cdeh_ascii_to_gb18030;
  iconveh_t cdeh_88591_to_gb18030;
  iconveh_t cdeh_utf7_to_gb18030;
# endif

  ASSERT (cd_ascii_to_utf8 != (iconv_t)(-1));
  ASSERT (cd_88591_to_utf8 != (iconv_t)(-1));
  ASSERT (cd_utf8_to_88591 != (iconv_t)(-1));
  ASSERT (cd_88592_to_utf8 != (iconv_t)(-1));
  ASSERT (cd_utf8_to_88592 != (iconv_t)(-1));
# if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
  ASSERT (cd_ascii_to_gb18030 != (iconv_t)(-1));
  ASSERT (cd_utf8_to_gb18030 != (iconv_t)(-1));
# endif

  cdeh_ascii_to_88591.cd = cd_ascii_to_88591;
  cdeh_ascii_to_88591.cd1 = cd_ascii_to_utf8;
  cdeh_ascii_to_88591.cd2 = cd_utf8_to_88591;

  cdeh_ascii_to_88591_indirectly.cd = (iconv_t)(-1);
  cdeh_ascii_to_88591_indirectly.cd1 = cd_ascii_to_utf8;
  cdeh_ascii_to_88591_indirectly.cd2 = cd_utf8_to_88591;

  cdeh_88592_to_88591.cd = cd_88592_to_88591;
  cdeh_88592_to_88591.cd1 = cd_88592_to_utf8;
  cdeh_88592_to_88591.cd2 = cd_utf8_to_88591;

  cdeh_88592_to_88591_indirectly.cd = (iconv_t)(-1);
  cdeh_88592_to_88591_indirectly.cd1 = cd_88592_to_utf8;
  cdeh_88592_to_88591_indirectly.cd2 = cd_utf8_to_88591;

  cdeh_ascii_to_utf8.cd = cd_ascii_to_utf8;
  cdeh_ascii_to_utf8.cd1 = cd_ascii_to_utf8;
  cdeh_ascii_to_utf8.cd2 = (iconv_t)(-1);

  cdeh_88591_to_utf8.cd = cd_88591_to_utf8;
  cdeh_88591_to_utf8.cd1 = cd_88591_to_utf8;
  cdeh_88591_to_utf8.cd2 = (iconv_t)(-1);

  cdeh_utf8_to_88591.cd = cd_utf8_to_88591;
  cdeh_utf8_to_88591.cd1 = (iconv_t)(-1);
  cdeh_utf8_to_88591.cd2 = cd_utf8_to_88591;

  cdeh_utf7_to_utf8.cd = cd_utf7_to_utf8;
  cdeh_utf7_to_utf8.cd1 = cd_utf7_to_utf8;
  cdeh_utf7_to_utf8.cd2 = (iconv_t)(-1);

# if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
  cdeh_ascii_to_gb18030.cd = cd_ascii_to_gb18030;
  cdeh_ascii_to_gb18030.cd1 = cd_ascii_to_utf8;
  cdeh_ascii_to_gb18030.cd2 = cd_utf8_to_gb18030;

  cdeh_88591_to_gb18030.cd = cd_88591_to_gb18030;
  cdeh_88591_to_gb18030.cd1 = cd_88591_to_utf8;
  cdeh_88591_to_gb18030.cd2 = cd_utf8_to_gb18030;

  cdeh_utf7_to_gb18030.cd = cd_utf7_to_gb18030;
  cdeh_utf7_to_gb18030.cd1 = cd_utf7_to_utf8;
  cdeh_utf7_to_gb18030.cd2 = cd_utf8_to_gb18030;
# endif

  /* ------------------------ Test mem_cd_iconveh() ------------------------ */

  /* Test conversion from ISO-8859-2 to ISO-8859-1 with no errors.  */
  for (indirect = 0; indirect <= 1; indirect++)
    {
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
          static const char expected[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
          for (o = 0; o < 2; o++)
            {
              size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
              char *result = NULL;
              size_t length = 0;
              int retval = mem_cd_iconveh (input, strlen (input),
                                           (indirect
                                            ? &cdeh_88592_to_88591_indirectly
                                            : &cdeh_88592_to_88591),
                                           handler,
                                           offsets,
                                           &result, &length);
              ASSERT (retval == 0);
              ASSERT (length == strlen (expected));
              ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
              if (o)
                {
                  for (i = 0; i < 37; i++)
                    ASSERT (offsets[i] == i);
                  ASSERT (offsets[37] == MAGIC);
                  free (offsets);
                }
              free (result);
            }
        }
    }

  /* Test conversion from ASCII to ISO-8859-1 with invalid input (EILSEQ).  */
  for (indirect = 0; indirect <= 1; indirect++)
    {
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          static const char input[] = "Rafa\263 Maszkowski"; /* Rafa? Maszkowski */
          for (o = 0; o < 2; o++)
            {
              size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
              char *result = NULL;
              size_t length = 0;
              int retval = mem_cd_iconveh (input, strlen (input),
                                           (indirect
                                            ? &cdeh_ascii_to_88591_indirectly
                                            : &cdeh_ascii_to_88591),
                                           handler,
                                           offsets,
                                           &result, &length);
              switch (handler)
                {
                case iconveh_error:
                  ASSERT (retval == -1 && errno == EILSEQ);
                  ASSERT (result == NULL);
                  if (o)
                    free (offsets);
                  break;
                case iconveh_question_mark:
                case iconveh_replacement_character:
                case iconveh_escape_sequence:
                  {
                    static const char expected[] = "Rafa? Maszkowski";
                    ASSERT (retval == 0);
                    ASSERT (length == strlen (expected));
                    ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                    if (o)
                      {
                        for (i = 0; i < 16; i++)
                          ASSERT (offsets[i] == i);
                        ASSERT (offsets[16] == MAGIC);
                        free (offsets);
                      }
                    free (result);
                  }
                  break;
                }
            }
        }
    }

  /* Test conversion from ISO-8859-2 to ISO-8859-1 with EILSEQ.  */
  for (indirect = 0; indirect <= 1; indirect++)
    {
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          static const char input[] = "Rafa\263 Maszkowski"; /* Rafał Maszkowski */
          for (o = 0; o < 2; o++)
            {
              size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
              char *result = NULL;
              size_t length = 0;
              int retval = mem_cd_iconveh (input, strlen (input),
                                           (indirect
                                            ? &cdeh_88592_to_88591_indirectly
                                            : &cdeh_88592_to_88591),
                                           handler,
                                           offsets,
                                           &result, &length);
              switch (handler)
                {
                case iconveh_error:
                  ASSERT (retval == -1 && errno == EILSEQ);
                  ASSERT (result == NULL);
                  if (o)
                    free (offsets);
                  break;
                case iconveh_question_mark:
                case iconveh_replacement_character:
                  {
                    static const char expected[] = "Rafa? Maszkowski";
                    ASSERT (retval == 0);
                    ASSERT (length == strlen (expected));
                    ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                    if (o)
                      {
                        for (i = 0; i < 16; i++)
                          ASSERT (offsets[i] == i);
                        ASSERT (offsets[16] == MAGIC);
                        free (offsets);
                      }
                    free (result);
                  }
                  break;
                case iconveh_escape_sequence:
                  {
                    static const char expected[] = "Rafa\\u0142 Maszkowski";
                    ASSERT (retval == 0);
                    ASSERT (length == strlen (expected));
                    ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                    if (o)
                      {
                        for (i = 0; i < 16; i++)
                          ASSERT (offsets[i] == (i < 5 ? i :
                                                 i + 5));
                        ASSERT (offsets[16] == MAGIC);
                        free (offsets);
                      }
                    free (result);
                  }
                  break;
                }
            }
        }
    }

  /* Test conversion from ISO-8859-1 to UTF-8 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, strlen (input),
                                       &cdeh_88591_to_utf8,
                                       handler,
                                       offsets,
                                       &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == strlen (expected));
          ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
          if (o)
            {
              for (i = 0; i < 37; i++)
                ASSERT (offsets[i] == (i < 1 ? i :
                                       i < 12 ? i + 1 :
                                       i < 18 ? i + 2 :
                                       i + 3));
              ASSERT (offsets[37] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }

# if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
  /* Test conversion from ISO-8859-1 to GB18030 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\2010\2072rger mit b\2010\2132sen B\250\271bchen ohne Augenma\2010\2118";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, strlen (input),
                                       &cdeh_88591_to_gb18030,
                                       handler,
                                       offsets,
                                       &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == strlen (expected));
          ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
          if (o)
            {
              for (i = 0; i < 37; i++)
                ASSERT (offsets[i] == (i < 1 ? i :
                                       i < 12 ? i + 3 :
                                       i < 18 ? i + 6 :
                                       i + 7));
              ASSERT (offsets[37] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }
# endif

  /* Test conversion from UTF-8 to ISO-8859-1 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
      static const char expected[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, strlen (input),
                                       &cdeh_utf8_to_88591,
                                       handler,
                                       offsets,
                                       &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == strlen (expected));
          ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
          if (o)
            {
              for (i = 0; i < 41; i++)
                ASSERT (offsets[i] == (i < 1 ? i :
                                       i == 1 ? (size_t)(-1) :
                                       i < 13 ? i - 1 :
                                       i == 13 ? (size_t)(-1) :
                                       i < 20 ? i - 2 :
                                       i == 20 ? (size_t)(-1) :
                                       i < 40 ? i - 3 :
                                       (size_t)(-1)));
              ASSERT (offsets[41] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }

  /* Test conversion from ASCII to UTF-8 with invalid input (EILSEQ).  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Rafa\263 Maszkowski"; /* Rafa? Maszkowski */
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, strlen (input),
                                       &cdeh_ascii_to_utf8,
                                       handler,
                                       offsets,
                                       &result, &length);
          switch (handler)
            {
            case iconveh_error:
              ASSERT (retval == -1 && errno == EILSEQ);
              ASSERT (result == NULL);
              if (o)
                free (offsets);
              break;
            case iconveh_question_mark:
            case iconveh_escape_sequence:
              {
                static const char expected[] = "Rafa? Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 16; i++)
                      ASSERT (offsets[i] == i);
                    ASSERT (offsets[16] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            case iconveh_replacement_character:
              {
                static const char expected[] = "Rafa\357\277\275 Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 16; i++)
                      ASSERT (offsets[i] == (i < 5 ? i : i + 2));
                    ASSERT (offsets[16] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            }
        }
    }

# if defined _LIBICONV_VERSION || ((__GLIBC__ + (__GLIBC_MINOR__ >= 16) > 2) && !defined __UCLIBC__)
  /* Test conversion from ASCII to GB18030 with invalid input (EILSEQ).
     Note: glibc's GB18030 converter was buggy in glibc-2.15; fixed by
     Andreas Schwab on 2012-02-06.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Rafa\263 Maszkowski"; /* Rafa? Maszkowski */
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, strlen (input),
                                       &cdeh_ascii_to_gb18030,
                                       handler,
                                       offsets,
                                       &result, &length);
          switch (handler)
            {
            case iconveh_error:
              ASSERT (retval == -1 && errno == EILSEQ);
              ASSERT (result == NULL);
              if (o)
                free (offsets);
              break;
            case iconveh_question_mark:
            case iconveh_escape_sequence:
              {
                static const char expected[] = "Rafa? Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 16; i++)
                      ASSERT (offsets[i] == i);
                    ASSERT (offsets[16] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            case iconveh_replacement_character:
              {
                static const char expected[] = "Rafa\2041\2447 Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 16; i++)
                      ASSERT (offsets[i] == (i < 5 ? i : i + 3));
                    ASSERT (offsets[16] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            }
        }
    }
# endif

  /* Test conversion from UTF-8 to ISO-8859-1 with EILSEQ.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Rafa\305\202 Maszkowski"; /* Rafał Maszkowski */
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, strlen (input),
                                       &cdeh_utf8_to_88591,
                                       handler,
                                       offsets,
                                       &result, &length);
          switch (handler)
            {
            case iconveh_error:
              ASSERT (retval == -1 && errno == EILSEQ);
              ASSERT (result == NULL);
              if (o)
                free (offsets);
              break;
            case iconveh_question_mark:
            case iconveh_replacement_character:
              {
                static const char expected[] = "Rafa? Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 17; i++)
                      ASSERT (offsets[i] == (i < 5 ? i :
                                             i == 5 ? (size_t)(-1) :
                                             i - 1));
                    ASSERT (offsets[17] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            case iconveh_escape_sequence:
              {
                static const char expected[] = "Rafa\\u0142 Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 17; i++)
                      ASSERT (offsets[i] == (i < 5 ? i :
                                             i == 5 ? (size_t)(-1) :
                                             i + 4));
                    ASSERT (offsets[17] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            }
        }
    }

  /* Test conversion from UTF-8 to ISO-8859-1 with EINVAL.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\342";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, strlen (input),
                                       &cdeh_utf8_to_88591,
                                       handler,
                                       offsets,
                                       &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == 0);
          if (o)
            {
              ASSERT (offsets[0] == 0);
              ASSERT (offsets[1] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }

  if (cd_utf7_to_utf8 != (iconv_t)(-1))
    {
      /* Disabled on Solaris, because Solaris 9 iconv() is buggy: it returns
         -1 / EILSEQ when converting the 7th byte of the input "+VDLYP9hA".  */
# if !(defined __sun && !defined _LIBICONV_VERSION)
      /* Test conversion from UTF-7 to UTF-8 with EINVAL.  */
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          /* This is base64 encoded 0x54 0x32 0xD8 0x3F 0xD8 0x40.  It would
             convert to U+5432 U+D83F U+D840 but these are Unicode surrogates.  */
          static const char input[] = "+VDLYP9hA";
          static const char expected1[] = "\345\220\262"; /* 吲 glibc */
          static const char expected2[] = ""; /* libiconv */
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, 7,
                                       &cdeh_utf7_to_utf8,
                                       handler,
                                       NULL,
                                       &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == strlen (expected1) || length == strlen (expected2));
          ASSERT (result != NULL);
          if (length == strlen (expected1))
            ASSERT (memcmp (result, expected1, strlen (expected1)) == 0);
          else
            ASSERT (memcmp (result, expected2, strlen (expected2)) == 0);
          free (result);
        }

#  if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
      /* Test conversion from UTF-7 to GB18030 with EINVAL.  */
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          /* This is base64 encoded 0x54 0x32 0xD8 0x3F 0xD8 0x40.  It would
             convert to U+5432 U+D83F U+D840 but these are Unicode surrogates.  */
          static const char input[] = "+VDLYP9hA";
          static const char expected1[] = "\337\305"; /* 吲 glibc */
          static const char expected2[] = ""; /* libiconv */
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, 7,
                                       &cdeh_utf7_to_gb18030,
                                       handler,
                                       NULL,
                                       &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == strlen (expected1) || length == strlen (expected2));
          ASSERT (result != NULL);
          if (length == strlen (expected1))
            ASSERT (memcmp (result, expected1, strlen (expected1)) == 0);
          else
            ASSERT (memcmp (result, expected2, strlen (expected2)) == 0);
          free (result);
        }
#  endif

      /* Disabled on NetBSD, because NetBSD 5.0 iconv() is buggy: it converts
         the input "+2D/YQNhB" to U+1FED8 U+3FD8 U+40D8.  */
#  if !(defined __NetBSD__ && !defined _LIBICONV_VERSION)
      /* Test conversion from UTF-7 to UTF-8 with EILSEQ.  */
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          /* This is base64 encoded 0xD8 0x3F 0xD8 0x40 0xD8 0x41.  It would
             convert to U+D83F U+D840 U+D841 but these are Unicode surrogates.  */
          static const char input[] = "+2D/YQNhB";
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, strlen (input),
                                       &cdeh_utf7_to_utf8,
                                       handler,
                                       NULL,
                                       &result, &length);
          switch (handler)
            {
            case iconveh_error:
              ASSERT (retval == -1 && errno == EILSEQ);
              ASSERT (result == NULL);
              break;
            case iconveh_question_mark:
            case iconveh_escape_sequence:
              {
                /* glibc result */
                static const char expected1[] = "?????";
                /* libiconv <= 1.12 result */
                static const char expected2[] = "?2D/YQNhB";
                /* libiconv behaviour changed in version 1.13: the result is
                   '?' U+0FF6 U+1036; this is U+D83F U+D840 U+D841 shifted left
                   by 6 bits.  */
                static const char expected3[] = "?\340\277\266\341\200\266";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected1)
                        || length == strlen (expected2)
                        || length == strlen (expected3));
                ASSERT (result != NULL);
                if (length == strlen (expected1))
                  ASSERT (memcmp (result, expected1, strlen (expected1)) == 0);
                else if (length == strlen (expected2))
                  ASSERT (memcmp (result, expected2, strlen (expected2)) == 0);
                else
                  ASSERT (memcmp (result, expected3, strlen (expected3)) == 0);
                free (result);
              }
              break;
            case iconveh_replacement_character:
              {
                /* glibc result */
                static const char expected1[] = "\357\277\275\357\277\275\357\277\275\357\277\275\357\277\275";
                /* libiconv <= 1.12 result */
                static const char expected2[] = "\357\277\2752D/YQNhB";
                /* libiconv >= 1.13 result */
                static const char expected3[] = "\357\277\275\340\277\266\341\200\266";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected1)
                        || length == strlen (expected2)
                        || length == strlen (expected3));
                ASSERT (result != NULL);
                if (length == strlen (expected1))
                  ASSERT (memcmp (result, expected1, strlen (expected1)) == 0);
                else if (length == strlen (expected2))
                  ASSERT (memcmp (result, expected2, strlen (expected2)) == 0);
                else
                  ASSERT (memcmp (result, expected3, strlen (expected3)) == 0);
                free (result);
              }
            }
        }

#   if defined _LIBICONV_VERSION || ((__GLIBC__ + (__GLIBC_MINOR__ >= 16) > 2) && !defined __UCLIBC__)
      /* Test conversion from UTF-7 to GB18030 with EILSEQ.
         Note: glibc's GB18030 converter was buggy in glibc-2.15; fixed by
         Andreas Schwab on 2012-02-06.  */
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          /* This is base64 encoded 0xD8 0x3F 0xD8 0x40 0xD8 0x41.  It would
             convert to U+D83F U+D840 U+D841 but these are Unicode surrogates.  */
          static const char input[] = "+2D/YQNhB";
          char *result = NULL;
          size_t length = 0;
          int retval = mem_cd_iconveh (input, strlen (input),
                                       &cdeh_utf7_to_gb18030,
                                       handler,
                                       NULL,
                                       &result, &length);
          switch (handler)
            {
            case iconveh_error:
              ASSERT (retval == -1 && errno == EILSEQ);
              ASSERT (result == NULL);
              break;
            case iconveh_question_mark:
            case iconveh_escape_sequence:
              {
                /* glibc result */
                static const char expected1[] = "?????";
                /* libiconv <= 1.12 result */
                static const char expected2[] = "?2D/YQNhB";
                /* libiconv behaviour changed in version 1.13: the result is
                   '?' U+0FF6 U+1036; this is U+D83F U+D840 U+D841 shifted left
                   by 6 bits.  */
                static const char expected3[] = "?\2013\2030\2013\2114";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected1)
                        || length == strlen (expected2)
                        || length == strlen (expected3));
                ASSERT (result != NULL);
                if (length == strlen (expected1))
                  ASSERT (memcmp (result, expected1, strlen (expected1)) == 0);
                else if (length == strlen (expected2))
                  ASSERT (memcmp (result, expected2, strlen (expected2)) == 0
                          || memcmp (result, expected3, strlen (expected3)) == 0);
                free (result);
              }
              break;
            case iconveh_replacement_character:
              {
                /* glibc result */
                static const char expected1[] = "\2041\2447\2041\2447\2041\2447\2041\2447\2041\2447";
                /* libiconv <= 1.12 result */
                static const char expected2[] = "\2041\24472D/YQNhB";
                /* libiconv >= 1.13 result */
                static const char expected3[] = "\2041\2447\2013\2030\2013\2114";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected1)
                        || length == strlen (expected2)
                        || length == strlen (expected3));
                ASSERT (result != NULL);
                if (length == strlen (expected1))
                  ASSERT (memcmp (result, expected1, strlen (expected1)) == 0);
                else if (length == strlen (expected2))
                  ASSERT (memcmp (result, expected2, strlen (expected2)) == 0
                          || memcmp (result, expected3, strlen (expected3)) == 0);
                free (result);
              }
            }
        }
#   endif
#  endif
# endif
    }

  /* ------------------------ Test str_cd_iconveh() ------------------------ */

  /* Test conversion from ISO-8859-2 to ISO-8859-1 with no errors.  */
  for (indirect = 0; indirect <= 1; indirect++)
    {
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
          static const char expected[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
          char *result = str_cd_iconveh (input,
                                         (indirect
                                          ? &cdeh_88592_to_88591_indirectly
                                          : &cdeh_88592_to_88591),
                                         handler);
          ASSERT (result != NULL);
          ASSERT (strcmp (result, expected) == 0);
          free (result);
        }
    }

  /* Test conversion from ASCII to ISO-8859-1 with invalid input (EILSEQ).  */
  for (indirect = 0; indirect <= 1; indirect++)
    {
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          static const char input[] = "Rafa\263 Maszkowski"; /* Rafa? Maszkowski */
          char *result = str_cd_iconveh (input,
                                         (indirect
                                          ? &cdeh_ascii_to_88591_indirectly
                                          : &cdeh_ascii_to_88591),
                                         handler);
          switch (handler)
            {
            case iconveh_error:
              ASSERT (result == NULL && errno == EILSEQ);
              break;
            case iconveh_question_mark:
            case iconveh_replacement_character:
            case iconveh_escape_sequence:
              {
                static const char expected[] = "Rafa? Maszkowski";
                ASSERT (result != NULL);
                ASSERT (strcmp (result, expected) == 0);
                free (result);
              }
              break;
            }
        }
    }

  /* Test conversion from ISO-8859-2 to ISO-8859-1 with EILSEQ.  */
  for (indirect = 0; indirect <= 1; indirect++)
    {
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          static const char input[] = "Rafa\263 Maszkowski"; /* Rafał Maszkowski */
          char *result = str_cd_iconveh (input,
                                         (indirect
                                          ? &cdeh_88592_to_88591_indirectly
                                          : &cdeh_88592_to_88591),
                                         handler);
          switch (handler)
            {
            case iconveh_error:
              ASSERT (result == NULL && errno == EILSEQ);
              break;
            case iconveh_question_mark:
            case iconveh_replacement_character:
              {
                static const char expected[] = "Rafa? Maszkowski";
                ASSERT (result != NULL);
                ASSERT (strcmp (result, expected) == 0);
                free (result);
              }
              break;
            case iconveh_escape_sequence:
              {
                static const char expected[] = "Rafa\\u0142 Maszkowski";
                ASSERT (result != NULL);
                ASSERT (strcmp (result, expected) == 0);
                free (result);
              }
              break;
            }
        }
    }

  /* Test conversion from ISO-8859-1 to UTF-8 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
      char *result = str_cd_iconveh (input,
                                     &cdeh_88591_to_utf8,
                                     handler);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, expected) == 0);
      free (result);
    }

# if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
  /* Test conversion from ISO-8859-1 to GB18030 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\2010\2072rger mit b\2010\2132sen B\250\271bchen ohne Augenma\2010\2118";
      char *result = str_cd_iconveh (input,
                                     &cdeh_88591_to_gb18030,
                                     handler);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, expected) == 0);
      free (result);
    }
# endif

  /* Test conversion from UTF-8 to ISO-8859-1 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
      static const char expected[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      char *result = str_cd_iconveh (input,
                                     &cdeh_utf8_to_88591,
                                     handler);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, expected) == 0);
      free (result);
    }

  /* Test conversion from ASCII to UTF-8 with invalid input (EILSEQ).  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Rafa\263 Maszkowski"; /* Rafa? Maszkowski */
      char *result = str_cd_iconveh (input,
                                     &cdeh_ascii_to_utf8,
                                     handler);
      switch (handler)
        {
        case iconveh_error:
          ASSERT (result == NULL && errno == EILSEQ);
          break;
        case iconveh_question_mark:
        case iconveh_escape_sequence:
          {
            static const char expected[] = "Rafa? Maszkowski";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        case iconveh_replacement_character:
          {
            static const char expected[] = "Rafa\357\277\275 Maszkowski";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        }
    }

# if defined _LIBICONV_VERSION || ((__GLIBC__ + (__GLIBC_MINOR__ >= 16) > 2) && !defined __UCLIBC__)
  /* Test conversion from ASCII to GB18030 with invalid input (EILSEQ).
     Note: glibc's GB18030 converter was buggy in glibc-2.15; fixed by
     Andreas Schwab on 2012-02-06.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Rafa\263 Maszkowski"; /* Rafa? Maszkowski */
      char *result = str_cd_iconveh (input,
                                     &cdeh_ascii_to_gb18030,
                                     handler);
      switch (handler)
        {
        case iconveh_error:
          ASSERT (result == NULL && errno == EILSEQ);
          break;
        case iconveh_question_mark:
        case iconveh_escape_sequence:
          {
            static const char expected[] = "Rafa? Maszkowski";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        case iconveh_replacement_character:
          {
            static const char expected[] = "Rafa\2041\2447 Maszkowski";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        }
    }
# endif

  /* Test conversion from UTF-8 to ISO-8859-1 with EILSEQ.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Costs: 27 \342\202\254"; /* EURO SIGN */
      char *result = str_cd_iconveh (input,
                                     &cdeh_utf8_to_88591,
                                     handler);
      switch (handler)
        {
        case iconveh_error:
          ASSERT (result == NULL && errno == EILSEQ);
          break;
        case iconveh_question_mark:
        case iconveh_replacement_character:
          {
            static const char expected[] = "Costs: 27 ?";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        case iconveh_escape_sequence:
          {
            static const char expected[] = "Costs: 27 \\u20AC";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        }
    }

  /* Test conversion from UTF-8 to ISO-8859-1 with EINVAL.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\342";
      char *result = str_cd_iconveh (input,
                                     &cdeh_utf8_to_88591,
                                     handler);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, "") == 0);
      free (result);
    }

  if (cd_88591_to_88592 != (iconv_t)(-1))
    iconv_close (cd_88591_to_88592);
  if (cd_88592_to_88591 != (iconv_t)(-1))
    iconv_close (cd_88592_to_88591);
  iconv_close (cd_88591_to_utf8);
  iconv_close (cd_utf8_to_88591);
  iconv_close (cd_88592_to_utf8);
  iconv_close (cd_utf8_to_88592);

  /* ------------------------- Test mem_iconveh() ------------------------- */

  /* Test conversion from ISO-8859-2 to ISO-8859-1 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_iconveh (input, strlen (input),
                                    "ISO-8859-2", "ISO-8859-1",
                                    handler,
                                    offsets,
                                    &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == strlen (expected));
          ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
          if (o)
            {
              for (i = 0; i < 37; i++)
                ASSERT (offsets[i] == i);
              ASSERT (offsets[37] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }

  /* Test conversion from ISO-8859-2 to ISO-8859-1 with EILSEQ.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Rafa\263 Maszkowski"; /* Rafał Maszkowski */
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_iconveh (input, strlen (input),
                                    "ISO-8859-2", "ISO-8859-1",
                                    handler,
                                    offsets,
                                    &result, &length);
          switch (handler)
            {
            case iconveh_error:
              ASSERT (retval == -1 && errno == EILSEQ);
              ASSERT (result == NULL);
              if (o)
                free (offsets);
              break;
            case iconveh_question_mark:
            case iconveh_replacement_character:
              {
                static const char expected[] = "Rafa? Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 16; i++)
                      ASSERT (offsets[i] == i);
                    ASSERT (offsets[16] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            case iconveh_escape_sequence:
              {
                static const char expected[] = "Rafa\\u0142 Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 16; i++)
                      ASSERT (offsets[i] == (i < 5 ? i :
                                             i + 5));
                    ASSERT (offsets[16] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            }
        }
    }

  /* Test conversion from ISO-8859-1 to UTF-8 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_iconveh (input, strlen (input),
                                    "ISO-8859-1", "UTF-8",
                                    handler,
                                    offsets,
                                    &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == strlen (expected));
          ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
          if (o)
            {
              for (i = 0; i < 37; i++)
                ASSERT (offsets[i] == (i < 1 ? i :
                                       i < 12 ? i + 1 :
                                       i < 18 ? i + 2 :
                                       i + 3));
              ASSERT (offsets[37] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }

# if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
  /* Test conversion from ISO-8859-1 to GB18030 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\2010\2072rger mit b\2010\2132sen B\250\271bchen ohne Augenma\2010\2118";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_iconveh (input, strlen (input),
                                    "ISO-8859-1", "GB18030",
                                    handler,
                                    offsets,
                                    &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == strlen (expected));
          ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
          if (o)
            {
              for (i = 0; i < 37; i++)
                ASSERT (offsets[i] == (i < 1 ? i :
                                       i < 12 ? i + 3 :
                                       i < 18 ? i + 6 :
                                       i + 7));
              ASSERT (offsets[37] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }
# endif

  /* Test conversion from UTF-8 to ISO-8859-1 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
      static const char expected[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_iconveh (input, strlen (input),
                                    "UTF-8", "ISO-8859-1",
                                    handler,
                                    offsets,
                                    &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == strlen (expected));
          ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
          if (o)
            {
              for (i = 0; i < 41; i++)
                ASSERT (offsets[i] == (i < 1 ? i :
                                       i == 1 ? (size_t)(-1) :
                                       i < 13 ? i - 1 :
                                       i == 13 ? (size_t)(-1) :
                                       i < 20 ? i - 2 :
                                       i == 20 ? (size_t)(-1) :
                                       i < 40 ? i - 3 :
                                       (size_t)(-1)));
              ASSERT (offsets[41] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }

  /* Test conversion from UTF-8 to ISO-8859-1 with EILSEQ.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Rafa\305\202 Maszkowski"; /* Rafał Maszkowski */
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_iconveh (input, strlen (input),
                                    "UTF-8", "ISO-8859-1",
                                    handler,
                                    offsets,
                                    &result, &length);
          switch (handler)
            {
            case iconveh_error:
              ASSERT (retval == -1 && errno == EILSEQ);
              ASSERT (result == NULL);
              if (o)
                free (offsets);
              break;
            case iconveh_question_mark:
            case iconveh_replacement_character:
              {
                static const char expected[] = "Rafa? Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 17; i++)
                      ASSERT (offsets[i] == (i < 5 ? i :
                                             i == 5 ? (size_t)(-1) :
                                             i - 1));
                    ASSERT (offsets[17] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            case iconveh_escape_sequence:
              {
                static const char expected[] = "Rafa\\u0142 Maszkowski";
                ASSERT (retval == 0);
                ASSERT (length == strlen (expected));
                ASSERT (result != NULL && memcmp (result, expected, strlen (expected)) == 0);
                if (o)
                  {
                    for (i = 0; i < 17; i++)
                      ASSERT (offsets[i] == (i < 5 ? i :
                                             i == 5 ? (size_t)(-1) :
                                             i + 4));
                    ASSERT (offsets[17] == MAGIC);
                    free (offsets);
                  }
                free (result);
              }
              break;
            }
        }
    }

  /* Test conversion from UTF-8 to ISO-8859-1 with EINVAL.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\342";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          char *result = NULL;
          size_t length = 0;
          int retval = mem_iconveh (input, strlen (input),
                                    "UTF-8", "ISO-8859-1",
                                    handler,
                                    offsets,
                                    &result, &length);
          ASSERT (retval == 0);
          ASSERT (length == 0);
          if (o)
            {
              ASSERT (offsets[0] == 0);
              ASSERT (offsets[1] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }

  /* ------------------------- Test str_iconveh() ------------------------- */

  /* Test conversion from ISO-8859-2 to ISO-8859-1 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      char *result = str_iconveh (input, "ISO-8859-2", "ISO-8859-1", handler);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, expected) == 0);
      free (result);
    }

  /* Test conversion from ISO-8859-2 to ISO-8859-1 with EILSEQ.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Rafa\263 Maszkowski"; /* Rafał Maszkowski */
      char *result = str_iconveh (input, "ISO-8859-2", "ISO-8859-1", handler);
      switch (handler)
        {
        case iconveh_error:
          ASSERT (result == NULL && errno == EILSEQ);
          break;
        case iconveh_question_mark:
        case iconveh_replacement_character:
          {
            static const char expected[] = "Rafa? Maszkowski";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        case iconveh_escape_sequence:
          {
            static const char expected[] = "Rafa\\u0142 Maszkowski";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        }
    }

  /* Test conversion from ISO-8859-1 to UTF-8 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
      char *result = str_iconveh (input, "ISO-8859-1", "UTF-8", handler);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, expected) == 0);
      free (result);
    }

# if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
  /* Test conversion from ISO-8859-1 to GB18030 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const char expected[] = "\2010\2072rger mit b\2010\2132sen B\250\271bchen ohne Augenma\2010\2118";
      char *result = str_iconveh (input, "ISO-8859-1", "GB18030", handler);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, expected) == 0);
      free (result);
    }
# endif

  /* Test conversion from UTF-8 to ISO-8859-1 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
      static const char expected[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      char *result = str_iconveh (input, "UTF-8", "ISO-8859-1", handler);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, expected) == 0);
      free (result);
    }

  /* Test conversion from UTF-8 to ISO-8859-1 with EILSEQ.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Costs: 27 \342\202\254"; /* EURO SIGN */
      char *result = str_iconveh (input, "UTF-8", "ISO-8859-1", handler);
      switch (handler)
        {
        case iconveh_error:
          ASSERT (result == NULL && errno == EILSEQ);
          break;
        case iconveh_question_mark:
        case iconveh_replacement_character:
          {
            static const char expected[] = "Costs: 27 ?";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        case iconveh_escape_sequence:
          {
            static const char expected[] = "Costs: 27 \\u20AC";
            ASSERT (result != NULL);
            ASSERT (strcmp (result, expected) == 0);
            free (result);
          }
          break;
        }
    }

  /* Test conversion from UTF-8 to ISO-8859-1 with EINVAL.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\342";
      char *result = str_iconveh (input, "UTF-8", "ISO-8859-1", handler);
      ASSERT (result != NULL);
      ASSERT (strcmp (result, "") == 0);
      free (result);
    }

  /* -------------------------------- Done. -------------------------------- */

  if (cd_ascii_to_88591 != (iconv_t)(-1))
    iconv_close (cd_ascii_to_88591);
  iconv_close (cd_ascii_to_utf8);
  if (cd_utf7_to_utf8 != (iconv_t)(-1))
    iconv_close (cd_utf7_to_utf8);

#endif

  return 0;
}
