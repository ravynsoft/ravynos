/* Test of conversion to UTF-8 from legacy encodings.
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

#include "uniconv.h"

#include <stdlib.h>
#include <string.h>

#include "unistr.h"
#include "macros.h"
extern int iconv_supports_encoding (const char *encoding);

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
    { iconveh_error, iconveh_question_mark, iconveh_escape_sequence };
  size_t h;
  size_t o;
  size_t i;

  /* Assume that iconv() supports at least the encodings ASCII, ISO-8859-1,
     ISO-8859-2, and UTF-8.  */

  /* Test conversion from ISO-8859-1 to UTF-8 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
      static const uint8_t expected[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          size_t length;
          uint8_t *result = u8_conv_from_encoding ("ISO-8859-1", handler,
                                                   input, strlen (input),
                                                   offsets,
                                                   NULL, &length);
          ASSERT (result != NULL);
          ASSERT (length == u8_strlen (expected));
          ASSERT (u8_cmp (result, expected, u8_strlen (expected)) == 0);
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

  /* Test conversion from ISO-8859-2 to UTF-8 with no errors.  */
  for (h = 0; h < SIZEOF (handlers); h++)
    {
      enum iconv_ilseq_handler handler = handlers[h];
      static const char input[] = "Rafa\263 Maszkowski"; /* Rafał Maszkowski */
      static const uint8_t expected[] = "Rafa\305\202 Maszkowski";
      for (o = 0; o < 2; o++)
        {
          size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
          size_t length;
          uint8_t *result = u8_conv_from_encoding ("ISO-8859-2", handler,
                                                   input, strlen (input),
                                                   offsets,
                                                   NULL, &length);
          ASSERT (result != NULL);
          ASSERT (length == u8_strlen (expected));
          ASSERT (u8_cmp (result, expected, u8_strlen (expected)) == 0);
          if (o)
            {
              for (i = 0; i < 16; i++)
                ASSERT (offsets[i] == (i < 5 ? i :
                                       i + 1));
              ASSERT (offsets[16] == MAGIC);
              free (offsets);
            }
          free (result);
        }
    }

  /* autodetect_jp is only supported when iconv() support ISO-2022-JP-2.  */
# if defined _LIBICONV_VERSION || !(defined _AIX || defined __sgi || defined __hpux || defined __osf__ || defined __sun)
  if (iconv_supports_encoding ("ISO-2022-JP-2"))
    {
      /* Test conversions from autodetect_jp to UTF-8.  */
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          static const char input[] = "\244\263\244\363\244\313\244\301\244\317"; /* こんにちは in EUC-JP */
          static const uint8_t expected[] = "\343\201\223\343\202\223\343\201\253\343\201\241\343\201\257"; /* こんにちは */
          for (o = 0; o < 2; o++)
            {
              size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
              size_t length;
              uint8_t *result = u8_conv_from_encoding ("autodetect_jp", handler,
                                                       input, strlen (input),
                                                       offsets,
                                                       NULL, &length);
              ASSERT (result != NULL);
              ASSERT (length == u8_strlen (expected));
              ASSERT (u8_cmp (result, expected, u8_strlen (expected)) == 0);
              if (o)
                {
                  for (i = 0; i < 10; i++)
                    ASSERT (offsets[i] == ((i % 2) == 0 ? (i / 2) * 3 : (size_t)(-1)));
                  ASSERT (offsets[10] == MAGIC);
                  free (offsets);
                }
              free (result);
            }
        }
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          static const char input[] = "\202\261\202\361\202\311\202\277\202\315"; /* こんにちは in Shift_JIS */
          static const uint8_t expected[] = "\343\201\223\343\202\223\343\201\253\343\201\241\343\201\257"; /* こんにちは */
          for (o = 0; o < 2; o++)
            {
              size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
              size_t length;
              uint8_t *result = u8_conv_from_encoding ("autodetect_jp", handler,
                                                       input, strlen (input),
                                                       offsets,
                                                       NULL, &length);
              ASSERT (result != NULL);
              ASSERT (length == u8_strlen (expected));
              ASSERT (u8_cmp (result, expected, u8_strlen (expected)) == 0);
              if (o)
                {
                  for (i = 0; i < 10; i++)
                    ASSERT (offsets[i] == ((i % 2) == 0 ? (i / 2) * 3 : (size_t)(-1)));
                  ASSERT (offsets[10] == MAGIC);
                  free (offsets);
                }
              free (result);
            }
        }
      for (h = 0; h < SIZEOF (handlers); h++)
        {
          enum iconv_ilseq_handler handler = handlers[h];
          static const char input[] = "\033$B$3$s$K$A$O\033(B"; /* こんにちは in ISO-2022-JP-2 */
          static const uint8_t expected[] = "\343\201\223\343\202\223\343\201\253\343\201\241\343\201\257"; /* こんにちは */
          for (o = 0; o < 2; o++)
            {
              size_t *offsets = (o ? new_offsets (strlen (input)) : NULL);
              size_t length;
              uint8_t *result = u8_conv_from_encoding ("autodetect_jp", handler,
                                                       input, strlen (input),
                                                       offsets,
                                                       NULL, &length);
              ASSERT (result != NULL);
              ASSERT (length == u8_strlen (expected));
              ASSERT (u8_cmp (result, expected, u8_strlen (expected)) == 0);
              if (o)
                {
                  for (i = 0; i < 16; i++)
                    ASSERT (offsets[i] == (i == 0 ? 0 :
                                           i == 5 ? 3 :
                                           i == 7 ? 6 :
                                           i == 9 ? 9 :
                                           i == 11 ? 12 :
                                           i == 13 ? 15 :
                                           (size_t)(-1)));
                  ASSERT (offsets[16] == MAGIC);
                  free (offsets);
                }
              free (result);
            }
        }
    }
# endif

#endif

  return 0;
}
