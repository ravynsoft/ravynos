/* Test of conversion of wide character to multibyte character.
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

#include <wchar.h>

#include "signature.h"
SIGNATURE_CHECK (wcrtomb, size_t, (char *, wchar_t, mbstate_t *));

#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"

/* Check the multibyte character s[0..n-1].  */
static void
check_character (const char *s, size_t n)
{
  wchar_t wc;
  char buf[64];
  int iret;
  size_t ret;

  wc = (wchar_t) 0xBADFACE;
  iret = mbtowc (&wc, s, n);
  ASSERT (iret == n);

  ret = wcrtomb (buf, wc, NULL);
  ASSERT (ret == n);
  ASSERT (memcmp (buf, s, n) == 0);

  /* Test special calling convention, passing a NULL pointer.  */
  ret = wcrtomb (NULL, wc, NULL);
  ASSERT (ret == 1);
}

int
main (int argc, char *argv[])
{
  char buf[64];
  size_t ret;

  /* configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  /* Test NUL character.  */
  {
    buf[0] = 'x';
    ret = wcrtomb (buf, 0, NULL);
    ASSERT (ret == 1);
    ASSERT (buf[0] == '\0');
  }

  /* Test single bytes.  */
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
          ret = wcrtomb (buf, btowc (c), NULL);
          ASSERT (ret == 1);
          ASSERT (buf[0] == (char) c);
          break;
        }
  }

  /* Test special calling convention, passing a NULL pointer.  */
  {
    ret = wcrtomb (NULL, '\0', NULL);
    ASSERT (ret == 1);
    ret = wcrtomb (NULL, btowc ('x'), NULL);
    ASSERT (ret == 1);
  }

  if (argc > 1)
    switch (argv[1][0])
      {
      case '1':
        /* C locale; tested above.  */
        return 0;

      case '2':
        /* Locale encoding is ISO-8859-1 or ISO-8859-15.  */
        {
          const char input[] = "B\374\337er"; /* "Büßer" */

          check_character (input + 1, 1);
          check_character (input + 2, 1);
        }
        return 0;

      case '3':
        /* Locale encoding is UTF-8.  */
        {
          const char input[] = "B\303\274\303\237er"; /* "Büßer" */

          check_character (input + 1, 2);
          check_character (input + 3, 2);
        }
        return 0;

      case '4':
        /* Locale encoding is EUC-JP.  */
        {
          const char input[] = "<\306\374\313\334\270\354>"; /* "<日本語>" */

          check_character (input + 1, 2);
          check_character (input + 3, 2);
          check_character (input + 5, 2);
        }
        return 0;

      case '5':
        /* Locale encoding is GB18030.  */
        {
          const char input[] = "B\250\271\201\060\211\070er"; /* "Büßer" */

          check_character (input + 1, 2);
          check_character (input + 3, 4);
        }
        return 0;
      }

  return 1;
}
