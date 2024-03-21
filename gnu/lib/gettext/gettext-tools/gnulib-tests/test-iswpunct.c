/* Test of iswpunct() function.
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
SIGNATURE_CHECK (iswpunct, int, (wint_t));

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "macros.h"

/* Returns the value of iswpunct for the multibyte character s[0..n-1].  */
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
    return iswpunct (wc);
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
  is = iswpunct (WEOF);
  ASSERT (is == 0);

  /* Test single-byte characters.
     POSIX specifies in
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap07.html>
     no explicit list of punctuation or symbol characters.  */
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
            case ' ':
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
              /* c is an alphanumeric or space character.  */
              ASSERT (is == 0);
              break;
            case '!': case '"': case '#': case '%':
            case '&': case '\'': case '(': case ')': case '*':
            case '+': case ',': case '-': case '.': case '/':
            case ':': case ';': case '<': case '=': case '>':
            case '?':
            case '[': case '\\': case ']': case '^': case '_':
            case '{': case '|': case '}': case '~':
              /* These characters are usually expected to be punctuation or
                 symbol characters.  */
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
        /* These characters are not in the ISO C "basic character set", but
           are nevertheless usually expected to be punctuation or symbol
           characters.  */
        is = for_character ("$", 1);
        ASSERT (is != 0);
        is = for_character ("@", 1);
        ASSERT (is != 0);
        is = for_character ("`", 1);
        ASSERT (is != 0);
        return 0;
      }

  return 1;
}
