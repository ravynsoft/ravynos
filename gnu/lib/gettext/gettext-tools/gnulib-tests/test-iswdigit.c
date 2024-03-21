/* Test of iswdigit() function.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

#include <config.h>

#include <wctype.h>

#include "signature.h"
SIGNATURE_CHECK (iswdigit, int, (wint_t));

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "macros.h"

/* Returns the value of iswdigit for the multibyte character s[0..n-1].  */
static int
for_character (const char *s, size_t n)
{
  mbstate_t state;
  wchar_t wc;
  size_t ret;

  memset (&state, '\0', sizeof (mbstate_t));
  wc = (wchar_t) 0xBADFACE;
  ret = mbrtowc (&wc, s, n, &state);
  if (ret == n)
    return iswdigit (wc);
  else
    return 0;
}

int
main (int argc, char *argv[])
{
  int is;
  char buf[4];

  /* configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  /* Test WEOF.  */
  is = iswdigit (WEOF);
  ASSERT (is == 0);

  /* Test single-byte characters.
     ISO C 99 sections 7.25.2.1.5 and 5.2.1 specify that the decimal digits
     include only the ASCII 0 ... 9 characters.  */
  {
    int c;

    for (c = 0; c < 0x100; c++)
      switch (c)
        {
        case '\t': case '\v': case '\f':
        case ' ': case '!': case '"': case '#': case '%':
        case '&': case '\'': case '(': case ')': case '*':
        case '+': case ',': case '-': case '.': case '/':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case ':': case ';': case '<': case '=': case '>':
        case '?':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y':
        case 'Z':
        case '[': case '\\': case ']': case '^': case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z': case '{': case '|': case '}': case '~':
          /* c is in the ISO C "basic character set".  */
          buf[0] = (unsigned char) c;
          is = for_character (buf, 1);
          switch (c)
            {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
              ASSERT (is != 0);
              break;
            default:
              ASSERT (is == 0);
              break;
            }
          break;
        }
  }

  if (argc > 1)
    switch (argv[1][0])
      {
      case '0':
        /* C locale; tested above.  */
        return 0;

      case '1':
        /* Locale encoding is ISO-8859-1 or ISO-8859-15.  */
        {
          /* U+00B2 SUPERSCRIPT TWO */
          is = for_character ("\262", 1);
          ASSERT (is == 0);
          /* U+00B3 SUPERSCRIPT THREE */
          is = for_character ("\263", 1);
          ASSERT (is == 0);
          /* U+00B9 SUPERSCRIPT ONE */
          is = for_character ("\271", 1);
          ASSERT (is == 0);
        }
        return 0;

      case '2':
        /* Locale encoding is EUC-JP.  */
        {
          /* U+FF11 FULLWIDTH DIGIT ONE */
          is = for_character ("\243\261", 2);
          ASSERT (is == 0);
        }
        return 0;

      case '3':
        /* Locale encoding is UTF-8.  */
        {
          /* U+00B2 SUPERSCRIPT TWO */
          is = for_character ("\302\262", 2);
          ASSERT (is == 0);
          /* U+00B3 SUPERSCRIPT THREE */
          is = for_character ("\302\263", 2);
          ASSERT (is == 0);
          /* U+00B9 SUPERSCRIPT ONE */
          is = for_character ("\302\271", 2);
          ASSERT (is == 0);
          /* U+0663 ARABIC-INDIC DIGIT THREE */
          is = for_character ("\331\243", 2);
          ASSERT (is == 0);
          /* U+2070 SUPERSCRIPT ZERO */
          is = for_character ("\342\201\260", 3);
          ASSERT (is == 0);
          /* U+2079 SUPERSCRIPT NINE */
          is = for_character ("\342\201\271", 3);
          ASSERT (is == 0);
          /* U+FF11 FULLWIDTH DIGIT ONE */
          is = for_character ("\357\274\221", 3);
          ASSERT (is == 0);
          /* U+1D7D1 MATHEMATICAL BOLD DIGIT THREE */
          is = for_character ("\360\235\237\221", 4);
          ASSERT (is == 0);
          /* U+1D7DB MATHEMATICAL DOUBLE-STRUCK DIGIT THREE */
          is = for_character ("\360\235\237\233", 4);
          ASSERT (is == 0);
          /* U+1D7E5 MATHEMATICAL SANS-SERIF DIGIT THREE */
          is = for_character ("\360\235\237\245", 4);
          ASSERT (is == 0);
          /* U+1D7EF MATHEMATICAL SANS-SERIF BOLD DIGIT THREE */
          is = for_character ("\360\235\237\257", 4);
          ASSERT (is == 0);
          /* U+1D7F9 MATHEMATICAL MONOSPACE DIGIT THREE */
          is = for_character ("\360\235\237\271", 4);
          ASSERT (is == 0);
          /* U+E0033 TAG DIGIT THREE */
          is = for_character ("\363\240\200\263", 4);
          ASSERT (is == 0);
        }
        return 0;

      case '4':
        /* Locale encoding is GB18030.  */
        {
          /* U+00B2 SUPERSCRIPT TWO */
          is = for_character ("\201\060\205\065", 4);
          ASSERT (is == 0);
          /* U+00B3 SUPERSCRIPT THREE */
          is = for_character ("\201\060\205\066", 4);
          ASSERT (is == 0);
          /* U+00B9 SUPERSCRIPT ONE */
          is = for_character ("\201\060\206\061", 4);
          ASSERT (is == 0);
          /* U+0663 ARABIC-INDIC DIGIT THREE */
          is = for_character ("\201\061\211\071", 4);
          ASSERT (is == 0);
          /* U+2070 SUPERSCRIPT ZERO */
          is = for_character ("\201\066\255\062", 4);
          ASSERT (is == 0);
          /* U+2079 SUPERSCRIPT NINE */
          is = for_character ("\201\066\256\061", 4);
          ASSERT (is == 0);
          /* U+FF11 FULLWIDTH DIGIT ONE */
          is = for_character ("\243\261", 2);
          ASSERT (is == 0);
          /* U+1D7D1 MATHEMATICAL BOLD DIGIT THREE */
          is = for_character ("\224\063\353\071", 4);
          ASSERT (is == 0);
          /* U+1D7DB MATHEMATICAL DOUBLE-STRUCK DIGIT THREE */
          is = for_character ("\224\063\354\071", 4);
          ASSERT (is == 0);
          /* U+1D7E5 MATHEMATICAL SANS-SERIF DIGIT THREE */
          is = for_character ("\224\063\355\071", 4);
          ASSERT (is == 0);
          /* U+1D7EF MATHEMATICAL SANS-SERIF BOLD DIGIT THREE */
          is = for_character ("\224\063\356\071", 4);
          ASSERT (is == 0);
          /* U+1D7F9 MATHEMATICAL MONOSPACE DIGIT THREE */
          is = for_character ("\224\063\357\071", 4);
          ASSERT (is == 0);
          /* U+E0033 TAG DIGIT THREE */
          is = for_character ("\323\066\232\071", 4);
          ASSERT (is == 0);
        }
        return 0;

      }

  return 1;
}
