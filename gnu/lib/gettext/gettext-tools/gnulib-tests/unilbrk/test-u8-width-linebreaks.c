/* Test of line breaking of UTF-8 strings.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2008.  */

#include <config.h>

#include "unilbrk.h"

#include <stdlib.h>

#include "macros.h"

static void
test_function (int (*my_u8_width_linebreaks) (const uint8_t *, size_t, int, int, int, const char *, const char *, char *_UC_RESTRICT),
               int version)
{
  /* Test case n = 0.  */
  my_u8_width_linebreaks (NULL, 0, 80, 0, 0, NULL, "GB18030", NULL);

  {
    static const uint8_t input[91] =
      /* "Grüß Gott. Здравствуйте! x=(-b±sqrt(b²-4ac))/(2a)  日本語,中文,한글" */
      "Gr\303\274\303\237 Gott. \320\227\320\264\321\200\320\260\320\262\321\201\321\202\320\262\321\203\320\271\321\202\320\265! x=(-b\302\261sqrt(b\302\262-4ac))/(2a)  \346\227\245\346\234\254\350\252\236,\344\270\255\346\226\207,\355\225\234\352\270\200\n";

    {
      char *p = (char *) malloc (SIZEOF (input));
      size_t i;

      my_u8_width_linebreaks (input, SIZEOF (input), 25, 0, 0, NULL, "GB18030", p);
      for (i = 0; i < 91; i++)
        {
          ASSERT (p[i] == (i == 90 ? UC_BREAK_MANDATORY :
                           i == 39 || i == 61 ? UC_BREAK_POSSIBLE :
                           UC_BREAK_PROHIBITED));
        }
      free (p);
    }

    {
      char *p = (char *) malloc (SIZEOF (input));
      size_t i;

      my_u8_width_linebreaks (input, SIZEOF (input), 25, 0, 0, NULL, "GB2312", p);
      for (i = 0; i < 91; i++)
        {
          ASSERT (p[i] == (i == 90 ? UC_BREAK_MANDATORY :
                           i == 13 || i == 39 || i == 61 ? UC_BREAK_POSSIBLE :
                           UC_BREAK_PROHIBITED));
        }
      free (p);
    }
  }
}

int
main ()
{
  test_function (u8_width_linebreaks, 2);
#ifdef IN_LIBUNISTRING_GNULIB_TESTS
# undef u8_width_linebreaks
  test_function (u8_width_linebreaks, 1);
#endif

  return 0;
}
