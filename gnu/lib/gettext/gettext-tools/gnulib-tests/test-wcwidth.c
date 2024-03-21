/* Test of wcwidth() function.
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

#include <wchar.h>

#include "signature.h"
SIGNATURE_CHECK (wcwidth, int, (wchar_t));

#include <locale.h>
#include <string.h>

#include "c-ctype.h"
#include "localcharset.h"
#include "macros.h"

int
main ()
{
  wchar_t wc;

#if !GNULIB_WCHAR_SINGLE_LOCALE
# ifdef C_CTYPE_ASCII
  /* Test width of ASCII characters.  */
  for (wc = 0x20; wc < 0x7F; wc++)
    ASSERT (wcwidth (wc) == 1);
# endif
#endif

  /* Switch to an UTF-8 locale.  */
  if (setlocale (LC_ALL, "fr_FR.UTF-8") != NULL
      /* Check whether it's really an UTF-8 locale.
         On OpenBSD 4.0, the setlocale call succeeds only for the LC_CTYPE
         category and therefore returns "C/fr_FR.UTF-8/C/C/C/C", but the
         LC_CTYPE category is effectively set to an ASCII LC_CTYPE category;
         in particular, locale_charset() returns "ASCII".  */
      && strcmp (locale_charset (), "UTF-8") == 0)
    {
      /* Test width of ASCII characters.  */
      for (wc = 0x20; wc < 0x7F; wc++)
        ASSERT (wcwidth (wc) == 1);

      /* Test width of some non-spacing characters.  */
      ASSERT (wcwidth (0x0301) == 0);
      ASSERT (wcwidth (0x05B0) == 0);

      /* Test width of some format control characters.  */
      ASSERT (wcwidth (0x200E) <= 0);
      ASSERT (wcwidth (0x2060) <= 0);
#if 0  /* wchar_t may be only 16 bits.  */
      ASSERT (wcwidth (0xE0001) <= 0);
      ASSERT (wcwidth (0xE0044) <= 0);
#endif

      /* Test width of some zero width characters.  */
      /* While it is desirable that U+200B, U+200C, U+200D have width 0,
         because this makes wcswidth work better on strings that contain these
         characters, it is acceptable if an implementation treats these
         characters like control characters.  */
      ASSERT (wcwidth (0x200B) <= 0);
      ASSERT (wcwidth (0xFEFF) <= 0);

      /* Test width of some math symbols.
         U+2202 is marked as having ambiguous width (A) in EastAsianWidth.txt
         (see <https://www.unicode.org/Public/12.0.0/ucd/EastAsianWidth.txt>).
         The Unicode Standard Annex 11
         <https://www.unicode.org/reports/tr11/tr11-36.html>
         says
           "Ambiguous characters behave like wide or narrow characters
            depending on the context (language tag, script identification,
            associated font, source of data, or explicit markup; all can
            provide the context). If the context cannot be established
            reliably, they should be treated as narrow characters by default."
         For wcwidth(), the only available context information is the locale.
         "fr_FR.UTF-8" is a Western locale, not an East Asian locale, therefore
         U+2202 should be treated like a narrow character.  */
      ASSERT (wcwidth (0x2202) == 1);

      /* Test width of some CJK characters.  */
      ASSERT (wcwidth (0x3000) == 2);
      ASSERT (wcwidth (0xB250) == 2);
      ASSERT (wcwidth (0xFF1A) == 2);
#if 0  /* wchar_t may be only 16 bits.  */
      ASSERT (wcwidth (0x20369) == 2);
      ASSERT (wcwidth (0x2F876) == 2);
#endif
    }

  return 0;
}
