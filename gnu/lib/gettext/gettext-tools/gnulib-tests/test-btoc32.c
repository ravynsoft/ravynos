/* Test of conversion of unibyte character to 32-bit wide character.
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

#include <uchar.h>

#include "signature.h"
SIGNATURE_CHECK (btoc32, wint_t, (int));

#include <locale.h>
#include <stdio.h>
#include <wchar.h>

#include "macros.h"

int
main (int argc, char *argv[])
{
  int c;

  /* configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  ASSERT (btoc32 (EOF) == WEOF);

#ifdef __ANDROID__
  /* On Android ≥ 5.0, the default locale is the "C.UTF-8" locale, not the
     "C" locale.  Furthermore, when you attempt to set the "C" or "POSIX"
     locale via setlocale(), what you get is a "C" locale with UTF-8 encoding,
     that is, effectively the "C.UTF-8" locale.  */
  if (argc > 1 && strcmp (argv[1], "1") == 0 && MB_CUR_MAX > 1)
    argv[1] = "3";
#endif

  if (argc > 1)
    switch (argv[1][0])
      {
      case '1':
        /* C or POSIX locale.  */
        for (c = 0; c < 0x100; c++)
          if (c != 0)
            {
              /* We are testing all nonnull bytes.  */
              wint_t wc = btoc32 (c);
              /* POSIX:2018 says regarding btowc: "In the POSIX locale, btowc()
                 shall not return WEOF if c has a value in the range 0 to 255
                 inclusive."  It is reasonable to expect btoc32 to behave in
                 the same way.  */
              if (c < 0x80)
                /* c is an ASCII character.  */
                ASSERT (wc == c);
              else
                /* On most platforms, the bytes 0x80..0xFF map to U+0080..U+00FF.
                   But on musl libc, the bytes 0x80..0xFF map to U+DF80..U+DFFF.  */
                ASSERT (wc == c || wc == 0xDF00 + c);
            }
        return 0;

      case '2':
        /* Locale encoding is ISO-8859-1 or ISO-8859-15.  */
        for (c = 0; c < 0x80; c++)
          ASSERT (btoc32 (c) == c);
        for (c = 0xA0; c < 0x100; c++)
          ASSERT (btoc32 (c) != WEOF);
        return 0;

      case '3':
        /* Locale encoding is UTF-8.  */
        for (c = 0; c < 0x80; c++)
          ASSERT (btoc32 (c) == c);
        for (c = 0x80; c < 0x100; c++)
          ASSERT (btoc32 (c) == WEOF);
        return 0;
      }

  return 1;
}
