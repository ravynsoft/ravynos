/* Determine display width of Unicode character.
   Copyright (C) 2001-2002, 2006-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2002.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "uniwidth.h"

#include "cjk.h"

/* The non-spacing attribute table consists of:
   * Non-spacing characters; generated from PropList.txt or
     "grep '^[^;]*;[^;]*;[^;]*;[^;]*;NSM;' UnicodeData.txt"
   * Format control characters; generated from
     "grep '^[^;]*;[^;]*;Cf;' UnicodeData.txt"
   * Zero width characters; generated from
     "grep '^[^;]*;ZERO WIDTH ' UnicodeData.txt"
   * Hangul Jamo characters that have conjoining behaviour:
       - jungseong = syllable-middle vowels
       - jongseong = syllable-final consonants
     Rationale:
     1) These characters act like combining characters. They have no
     equivalent in legacy character sets. Therefore the EastAsianWidth.txt
     file does not really matter for them; UAX #11 East Asian Width
     <https://www.unicode.org/reports/tr11/> makes it clear that it focus
     is on compatibility with traditional Japanese layout.
     By contrast, the same glyphs without conjoining behaviour are available
     in the U+3130..U+318F block, and these characters are mapped to legacy
     character sets, and traditional Japanese layout matters for them.
     2) glibc does the same thing, see
     <https://sourceware.org/bugzilla/show_bug.cgi?id=21750>
     <https://sourceware.org/bugzilla/show_bug.cgi?id=26120>
 */
#include "uniwidth/width0.h"

#include "uniwidth/width2.h"
#include "unictype/bitmap.h"

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* Determine number of column positions required for UC.  */
int
uc_width (ucs4_t uc, const char *encoding)
{
  /* Test for non-spacing or control character.  */
  if ((uc >> 9) < SIZEOF (nonspacing_table_ind))
    {
      int ind = nonspacing_table_ind[uc >> 9];
      if (ind >= 0)
        if ((nonspacing_table_data[64*ind + ((uc >> 3) & 63)] >> (uc & 7)) & 1)
          {
            if (uc > 0 && uc < 0xa0)
              return -1;
            else
              return 0;
          }
    }
  else if ((uc >> 9) == (0xe0000 >> 9))
    {
      if (uc >= 0xe0100)
        {
          if (uc <= 0xe01ef)
            return 0;
        }
      else
        {
          if (uc >= 0xe0020 ? uc <= 0xe007f : uc == 0xe0001)
            return 0;
        }
    }
  /* Test for double-width character.  */
  if (bitmap_lookup (&u_width2, uc))
    return 2;
  /* In ancient CJK encodings, Cyrillic and most other characters are
     double-width as well.  */
  if (uc >= 0x00A1 && uc < 0xFF61 && uc != 0x20A9
      && is_cjk_encoding (encoding))
    return 2;
  return 1;
}
