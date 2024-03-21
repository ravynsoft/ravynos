/* Test of character handling in C locale.
   Copyright (C) 2005, 2007-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2005.  */

#include <config.h>

#include "c-ctype.h"

#include <ctype.h>
#include <limits.h>
#include <locale.h>

#include "macros.h"

static void
test_agree_with_C_locale (void)
{
  int c;

  for (c = 0; c <= UCHAR_MAX; c++)
    {
      ASSERT (c_isascii (c) == (isascii (c) != 0));
      if (c_isascii (c))
        {
          ASSERT (c_isalnum (c) == (isalnum (c) != 0));
          ASSERT (c_isalpha (c) == (isalpha (c) != 0));
          ASSERT (c_isblank (c) == (isblank (c) != 0));
          ASSERT (c_iscntrl (c) == (iscntrl (c) != 0));
          ASSERT (c_isdigit (c) == (isdigit (c) != 0));
          ASSERT (c_islower (c) == (islower (c) != 0));
          ASSERT (c_isgraph (c) == (isgraph (c) != 0));
          ASSERT (c_isprint (c) == (isprint (c) != 0));
          ASSERT (c_ispunct (c) == (ispunct (c) != 0));
          ASSERT (c_isspace (c) == (isspace (c) != 0));
          ASSERT (c_isupper (c) == (isupper (c) != 0));
          ASSERT (c_isxdigit (c) == (isxdigit (c) != 0));
          ASSERT (c_tolower (c) == tolower (c));
          ASSERT (c_toupper (c) == toupper (c));
        }
    }
}

static void
test_all (void)
{
  int c;
  int n_isascii = 0;

  for (c = CHAR_MIN; c <= UCHAR_MAX; c++)
    {
      if (! (0 <= c && c <= CHAR_MAX))
        {
          ASSERT (! c_isascii (c));
          ASSERT (! c_isalnum (c));
          ASSERT (! c_isalpha (c));
          ASSERT (! c_isblank (c));
          ASSERT (! c_iscntrl (c));
          ASSERT (! c_isdigit (c));
          ASSERT (! c_islower (c));
          ASSERT (! c_isgraph (c));
          ASSERT (! c_isprint (c));
          ASSERT (! c_ispunct (c));
          ASSERT (! c_isspace (c));
          ASSERT (! c_isupper (c));
          ASSERT (! c_isxdigit (c));
          ASSERT (c_tolower (c) == c);
          ASSERT (c_toupper (c) == c);
        }

      n_isascii += c_isascii (c);

#ifdef C_CTYPE_ASCII
      ASSERT (c_isascii (c) == (0 <= c && c <= 0x7f));
#endif

      ASSERT (c_isascii (c) == (c_isprint (c) || c_iscntrl (c)));

      ASSERT (c_isalnum (c) == (c_isalpha (c) || c_isdigit (c)));

      ASSERT (c_isalpha (c) == (c_islower (c) || c_isupper (c)));

      switch (c)
        {
        case '\t': case ' ':
          ASSERT (c_isblank (c) == 1);
          break;
        default:
          ASSERT (c_isblank (c) == 0);
          break;
        }

#ifdef C_CTYPE_ASCII
      ASSERT (c_iscntrl (c) == ((c >= 0 && c < 0x20) || c == 0x7f));
#endif

      switch (c)
        {
        case '\a': case '\b': case '\f': case '\n':
        case '\r': case '\t': case '\v':
          ASSERT (c_iscntrl (c));
          break;
        }

      ASSERT (! (c_iscntrl (c) && c_isprint (c)));

      switch (c)
        {
        case '0': case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
          ASSERT (c_isdigit (c) == 1);
          break;
        default:
          ASSERT (c_isdigit (c) == 0);
          break;
        }

      switch (c)
        {
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
          ASSERT (c_islower (c) == 1);
          ASSERT (c_toupper (c) == c - 'a' + 'A');
          break;
        default:
          ASSERT (c_islower (c) == 0);
          ASSERT (c_toupper (c) == c);
          break;
        }

#ifdef C_CTYPE_ASCII
      ASSERT (c_isgraph (c) == ((c >= 0x20 && c < 0x7f) && c != ' '));

      ASSERT (c_isprint (c) == (c >= 0x20 && c < 0x7f));
#endif

      ASSERT (c_isgraph (c) == (c_isalnum (c) || c_ispunct (c)));

      ASSERT (c_isprint (c) == (c_isgraph (c) || c == ' '));

      switch (c)
        {
        case '!': case '"': case '#': case '$': case '%': case '&': case '\'':
        case '(': case ')': case '*': case '+': case ',': case '-': case '.':
        case '/': case ':': case ';': case '<': case '=': case '>': case '?':
        case '@': case '[': case'\\': case ']': case '^': case '_': case '`':
        case '{': case '|': case '}': case '~':
          ASSERT (c_ispunct (c) == 1);
          break;
        default:
          ASSERT (c_ispunct (c) == 0);
          break;
        }

      switch (c)
        {
        case ' ': case '\t': case '\n': case '\v': case '\f': case '\r':
          ASSERT (c_isspace (c) == 1);
          break;
        default:
          ASSERT (c_isspace (c) == 0);
          break;
        }

      switch (c)
        {
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
          ASSERT (c_isupper (c) == 1);
          ASSERT (c_tolower (c) == c - 'A' + 'a');
          break;
        default:
          ASSERT (c_isupper (c) == 0);
          ASSERT (c_tolower (c) == c);
          break;
        }

      switch (c)
        {
        case '0': case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
          ASSERT (c_isxdigit (c) == 1);
          break;
        default:
          ASSERT (c_isxdigit (c) == 0);
          break;
        }
    }

  ASSERT (n_isascii == 128);
}

int
main ()
{
  test_agree_with_C_locale ();

  test_all ();

  setlocale (LC_ALL, "de_DE");
  test_all ();

  setlocale (LC_ALL, "ja_JP.EUC-JP");
  test_all ();

  return 0;
}
