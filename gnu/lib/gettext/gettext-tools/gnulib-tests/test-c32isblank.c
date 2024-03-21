/* Test of c32isblank() function.
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

#include <uchar.h>

#include "signature.h"
SIGNATURE_CHECK (c32isblank, int, (wint_t));

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "macros.h"

/* Returns the value of c32isblank for the multibyte character s[0..n-1].  */
static int
for_character (const char *s, size_t n)
{
  mbstate_t state;
  char32_t wc;
  size_t ret;

  memset (&state, '\0', sizeof (mbstate_t));
  wc = (char32_t) 0xBADFACE;
  ret = mbrtoc32 (&wc, s, n, &state);
  ASSERT (ret == n);

  return c32isblank (wc);
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
  is = c32isblank (WEOF);
  ASSERT (is == 0);

  /* Test single-byte characters.
     POSIX specifies in
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap07.html>
     that
       - in all locales, the blank characters include the <space> and <tab>
         characters,
       - in the "POSIX" locale (which is usually the same as the "C" locale),
         the blank characters include only the ASCII <space> and <tab>
         characters.  */
  {
    int c;

    for (c = 0; c < 0x100; c++)
      switch (c)
        {
        case '\t':
        #if !(defined __FreeBSD__ || defined __NetBSD__)
        case '\v':
        #endif
        case '\f':
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
          if (c == '\t' || c == ' ')
            ASSERT (is != 0);
          else
            ASSERT (is == 0);
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
        #if defined __GLIBC__
          /* U+00A0 NO-BREAK SPACE */
          is = for_character ("\240", 1);
          ASSERT (is == 0);
        #endif
          /* U+00B7 MIDDLE DOT */
          is = for_character ("\267", 1);
          ASSERT (is == 0);
        }
        return 0;

      case '2':
        /* Locale encoding is EUC-JP.  */
        {
          /* U+3002 IDEOGRAPHIC FULL STOP */
          is = for_character ("\241\243", 2);
          ASSERT (is == 0);
        }
        return 0;

      case '3':
        /* Locale encoding is UTF-8.  */
        {
        #if defined __GLIBC__
          /* U+00A0 NO-BREAK SPACE */
          is = for_character ("\302\240", 2);
          ASSERT (is == 0);
        #endif
          /* U+00B7 MIDDLE DOT */
          is = for_character ("\302\267", 2);
          ASSERT (is == 0);
        #if defined __GLIBC__
          /* U+202F NARROW NO-BREAK SPACE */
          is = for_character ("\342\200\257", 3);
          ASSERT (is == 0);
        #endif
          /* U+3002 IDEOGRAPHIC FULL STOP */
          is = for_character ("\343\200\202", 3);
          ASSERT (is == 0);
          /* U+1D13D MUSICAL SYMBOL QUARTER REST */
          is = for_character ("\360\235\204\275", 4);
          ASSERT (is == 0);
          /* U+E0020 TAG SPACE */
          is = for_character ("\363\240\200\240", 4);
          ASSERT (is == 0);
        }
        return 0;

      case '4':
        /* Locale encoding is GB18030.  */
        #if (defined __GLIBC__ && __GLIBC__ == 2 && __GLIBC_MINOR__ >= 13 && __GLIBC_MINOR__ <= 15) || (GL_CHAR32_T_IS_UNICODE && (defined __NetBSD__ || defined __sun))
        fputs ("Skipping test: The GB18030 converter in this system's iconv is broken.\n", stderr);
        return 77;
        #endif
        {
        #if defined __GLIBC__
          /* U+00A0 NO-BREAK SPACE */
          is = for_character ("\201\060\204\062", 4);
          ASSERT (is == 0);
        #endif
          /* U+00B7 MIDDLE DOT */
          is = for_character ("\241\244", 2);
          ASSERT (is == 0);
        #if defined __GLIBC__
          /* U+202F NARROW NO-BREAK SPACE */
          is = for_character ("\201\066\247\062", 4);
          ASSERT (is == 0);
        #endif
          /* U+3002 IDEOGRAPHIC FULL STOP */
          is = for_character ("\241\243", 2);
          ASSERT (is == 0);
          /* U+1D13D MUSICAL SYMBOL QUARTER REST */
          is = for_character ("\224\062\301\065", 4);
          ASSERT (is == 0);
          /* U+E0020 TAG SPACE */
          is = for_character ("\323\066\231\060", 4);
          ASSERT (is == 0);
        }
        return 0;

      }

  return 1;
}
