/* Test of c32isalnum() function.
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
SIGNATURE_CHECK (c32isalnum, int, (wint_t));

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "macros.h"

/* Returns the value of c32isalnum for the multibyte character s[0..n-1].  */
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

  return c32isalnum (wc);
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
  is = c32isalnum (WEOF);
  ASSERT (is == 0);

  /* Test single-byte characters.
     POSIX specifies in
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap07.html>
     that
       - in all locales, the alphanumeric characters include the uppercase and
         lowercase characters and digits and, consequently, include the A ... Z
         and a ... z and 0 ... 9 characters.
       - in the "POSIX" locale (which is usually the same as the "C" locale),
         the alphanumeric characters include only the ASCII A ... Z and a ... z
         and 0 ... 9 characters.  */
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
            case 'A': case 'B': case 'C': case 'D': case 'E':
            case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O':
            case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y':
            case 'Z':
            case 'a': case 'b': case 'c': case 'd': case 'e':
            case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o':
            case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y':
            case 'z':
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
          /* U+00D7 MULTIPLICATION SIGN */
          is = for_character ("\327", 1);
          ASSERT (is == 0);
          /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
          is = for_character ("\330", 1);
          ASSERT (is != 0);
        }
        return 0;

      case '2':
        /* Locale encoding is EUC-JP.  */
        {
          /* U+00D7 MULTIPLICATION SIGN */
          is = for_character ("\241\337", 2);
          ASSERT (is == 0);
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__)
          /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
          is = for_character ("\217\251\254", 3);
          ASSERT (is != 0);
          /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
          is = for_character ("\217\251\250", 3);
          ASSERT (is != 0);
        #endif
          /* U+3001 IDEOGRAPHIC COMMA */
          is = for_character ("\241\242", 2);
          ASSERT (is == 0);
        #if defined __GLIBC__
          /* U+FF11 FULLWIDTH DIGIT ONE */
          is = for_character ("\243\261", 2);
          ASSERT (is != 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__)
          /* U+FF4D FULLWIDTH LATIN SMALL LETTER M */
          is = for_character ("\243\355", 2);
          ASSERT (is != 0);
        #endif
        }
        return 0;

      case '3':
        /* Locale encoding is UTF-8.  */
        {
          /* U+00D7 MULTIPLICATION SIGN */
          is = for_character ("\303\227", 2);
          ASSERT (is == 0);
          /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
          is = for_character ("\303\230", 2);
          ASSERT (is != 0);
          /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
          is = for_character ("\305\201", 2);
          ASSERT (is != 0);
          /* U+3001 IDEOGRAPHIC COMMA */
          is = for_character ("\343\200\201", 3);
          ASSERT (is == 0);
        #if defined __GLIBC__
          /* U+FF11 FULLWIDTH DIGIT ONE */
          is = for_character ("\357\274\221", 3);
          ASSERT (is != 0);
        #endif
          /* U+FF4D FULLWIDTH LATIN SMALL LETTER M */
          is = for_character ("\357\275\215", 3);
          ASSERT (is != 0);
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
          /* U+10330 GOTHIC LETTER AHSA */
          is = for_character ("\360\220\214\260", 4);
          ASSERT (is != 0);
        #endif
          /* U+1D100 MUSICAL SYMBOL SINGLE BARLINE */
          is = for_character ("\360\235\204\200", 4);
          ASSERT (is == 0);
          /* U+E0061 TAG LATIN SMALL LETTER A */
          is = for_character ("\363\240\201\241", 4);
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
          /* U+00D7 MULTIPLICATION SIGN */
          is = for_character ("\241\301", 2);
          ASSERT (is == 0);
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
          /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
          is = for_character ("\201\060\211\061", 4);
          ASSERT (is != 0);
          /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
          is = for_character ("\201\060\221\071", 4);
          ASSERT (is != 0);
        #endif
          /* U+3001 IDEOGRAPHIC COMMA */
          is = for_character ("\241\242", 2);
          ASSERT (is == 0);
        #if defined __GLIBC__
          /* U+FF11 FULLWIDTH DIGIT ONE */
          is = for_character ("\243\261", 2);
          ASSERT (is != 0);
        #endif
        #if !defined __DragonFly__
          /* U+FF4D FULLWIDTH LATIN SMALL LETTER M */
          is = for_character ("\243\355", 2);
          ASSERT (is != 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
          /* U+10330 GOTHIC LETTER AHSA */
          is = for_character ("\220\060\322\066", 4);
          ASSERT (is != 0);
        #endif
          /* U+1D100 MUSICAL SYMBOL SINGLE BARLINE */
          is = for_character ("\224\062\273\064", 4);
          ASSERT (is == 0);
          /* U+E0061 TAG LATIN SMALL LETTER A */
          is = for_character ("\323\066\237\065", 4);
          ASSERT (is == 0);
        }
        return 0;

      }

  return 1;
}
