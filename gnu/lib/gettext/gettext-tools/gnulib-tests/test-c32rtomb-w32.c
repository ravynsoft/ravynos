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

#include <config.h>

#include <uchar.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "localcharset.h"
#include "macros.h"

#if defined _WIN32 && !defined __CYGWIN__

static int
test_one_locale (const char *name, int codepage)
{
  char buf[64];
  size_t ret;

# if 1
  /* Portable code to set the locale.  */
  {
    char name_with_codepage[1024];

    sprintf (name_with_codepage, "%s.%d", name, codepage);

    /* Set the locale.  */
    if (setlocale (LC_ALL, name_with_codepage) == NULL)
      return 77;
  }
# else
  /* Hacky way to set a locale.codepage combination that setlocale() refuses
     to set.  */
  {
    /* Codepage of the current locale, set with setlocale().
       Not necessarily the same as GetACP().  */
    extern __declspec(dllimport) unsigned int __lc_codepage;

    /* Set the locale.  */
    if (setlocale (LC_ALL, name) == NULL)
      return 77;

    /* Clobber the codepage and MB_CUR_MAX, both set by setlocale().  */
    __lc_codepage = codepage;
    switch (codepage)
      {
      case 1252:
      case 1256:
        MB_CUR_MAX = 1;
        break;
      case 932:
      case 950:
      case 936:
        MB_CUR_MAX = 2;
        break;
      case 54936:
      case 65001:
        MB_CUR_MAX = 4;
        break;
      }

    /* Test whether the codepage is really available.  */
    {
      mbstate_t state;
      wchar_t wc;

      memset (&state, '\0', sizeof (mbstate_t));
      if (mbrtowc (&wc, " ", 1, &state) == (size_t)(-1))
        return 77;
    }
  }
# endif

  /* Test NUL character.  */
  {
    buf[0] = 'x';
    ret = c32rtomb (buf, 0, NULL);
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
          ret = c32rtomb (buf, btoc32 (c), NULL);
          ASSERT (ret == 1);
          ASSERT (buf[0] == (char) c);
          break;
        }
  }

  /* Test special calling convention, passing a NULL pointer.  */
  {
    ret = c32rtomb (NULL, '\0', NULL);
    ASSERT (ret == 1);
    ret = c32rtomb (NULL, btoc32 ('x'), NULL);
    ASSERT (ret == 1);
  }

  switch (codepage)
    {
    case 1252:
      /* Locale encoding is CP1252, an extension of ISO-8859-1.  */
      {
        /* Convert "B\374\337er": "Büßer" */
        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x00FC, NULL);
        ASSERT (ret == 1);
        ASSERT (memcmp (buf, "\374", 1) == 0);
        ASSERT (buf[1] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x00DF, NULL);
        ASSERT (ret == 1);
        ASSERT (memcmp (buf, "\337", 1) == 0);
        ASSERT (buf[1] == 'x');
      }
      return 0;

    case 1256:
      /* Locale encoding is CP1256, not the same as ISO-8859-6.  */
      {
        /* Convert "x\302\341\346y": "xآلوy" */
        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x0622, NULL);
        ASSERT (ret == 1);
        ASSERT (memcmp (buf, "\302", 1) == 0);
        ASSERT (buf[1] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x0644, NULL);
        ASSERT (ret == 1);
        ASSERT (memcmp (buf, "\341", 1) == 0);
        ASSERT (buf[1] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x0648, NULL);
        ASSERT (ret == 1);
        ASSERT (memcmp (buf, "\346", 1) == 0);
        ASSERT (buf[1] == 'x');
      }
      return 0;

    case 65001:
      /* Locale encoding is CP65001 = UTF-8.  */
      if (strcmp (locale_charset (), "UTF-8") != 0)
        return 77;
      {
        /* Convert "s\303\274\303\237\360\237\230\213!"; "süß😋!" */
        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x00FC, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\303\274", 2) == 0);
        ASSERT (buf[2] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x00DF, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\303\237", 2) == 0);
        ASSERT (buf[2] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x1F60B, NULL);
        ASSERT (ret == 4);
        ASSERT (memcmp (buf, "\360\237\230\213", 4) == 0);
        ASSERT (buf[4] == 'x');
      }
      return 0;

    case 932:
      /* Locale encoding is CP932, similar to Shift_JIS.  */
      {
        /* Convert "<\223\372\226\173\214\352>": "<日本語>" */
        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x65E5, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\223\372", 2) == 0);
        ASSERT (buf[2] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x672C, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\226\173", 2) == 0);
        ASSERT (buf[2] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x8A9E, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\214\352", 2) == 0);
        ASSERT (buf[2] == 'x');
      }
      return 0;

    case 950:
      /* Locale encoding is CP950, similar to Big5.  */
      {
        /* Convert "<\244\351\245\273\273\171>": "<日本語>" */
        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x65E5, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\244\351", 2) == 0);
        ASSERT (buf[2] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x672C, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\245\273", 2) == 0);
        ASSERT (buf[2] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x8A9E, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\273\171", 2) == 0);
        ASSERT (buf[2] == 'x');
      }
      return 0;

    case 936:
      /* Locale encoding is CP936 = GBK, an extension of GB2312.  */
      {
        /* Convert "<\310\325\261\276\325\132>": "<日本語>" */
        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x65E5, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\310\325", 2) == 0);
        ASSERT (buf[2] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x672C, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\261\276", 2) == 0);
        ASSERT (buf[2] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x8A9E, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\325\132", 2) == 0);
        ASSERT (buf[2] == 'x');
      }
      return 0;

    case 54936:
      /* Locale encoding is CP54936 = GB18030.  */
      if (strcmp (locale_charset (), "GB18030") != 0)
        return 77;
      {
        /* Convert "s\250\271\201\060\211\070\224\071\375\067!"; "süß😋!" */
        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x00FC, NULL);
        ASSERT (ret == 2);
        ASSERT (memcmp (buf, "\250\271", 2) == 0);
        ASSERT (buf[2] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x00DF, NULL);
        ASSERT (ret == 4);
        ASSERT (memcmp (buf, "\201\060\211\070", 4) == 0);
        ASSERT (buf[4] == 'x');

        memset (buf, 'x', 8);
        ret = c32rtomb (buf, 0x1F60B, NULL);
        ASSERT (ret == 4);
        ASSERT (memcmp (buf, "\224\071\375\067", 4) == 0);
        ASSERT (buf[4] == 'x');
      }
      return 0;

    default:
      return 1;
    }
}

int
main (int argc, char *argv[])
{
  int codepage = atoi (argv[argc - 1]);
  int result;
  int i;

  result = 77;
  for (i = 1; i < argc - 1; i++)
    {
      int ret = test_one_locale (argv[i], codepage);

      if (ret != 77)
        result = ret;
    }

  if (result == 77)
    {
      fprintf (stderr, "Skipping test: found no locale with codepage %d\n",
               codepage);
    }
  return result;
}

#else

int
main (int argc, char *argv[])
{
  fputs ("Skipping test: not a native Windows system\n", stderr);
  return 77;
}

#endif
