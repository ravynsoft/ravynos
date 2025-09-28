/* Check UTF-8 string.
   Copyright (C) 2002, 2006-2007, 2009-2023 Free Software Foundation, Inc.
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
#include "unistr.h"

const uint8_t *
u8_check (const uint8_t *s, size_t n)
{
  const uint8_t *s_end = s + n;

  while (s < s_end)
    {
      /* Keep in sync with unistr.h and u8-mbtouc-aux.c.  */
      uint8_t c = *s;

      if (c < 0x80)
        {
          s++;
          continue;
        }
      if (c >= 0xc2)
        {
          if (c < 0xe0)
            {
              if (s + 2 <= s_end
                  && (s[1] ^ 0x80) < 0x40)
                {
                  s += 2;
                  continue;
                }
            }
          else if (c < 0xf0)
            {
              if (s + 3 <= s_end
                  && (s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                  && (c >= 0xe1 || s[1] >= 0xa0)
                  && (c != 0xed || s[1] < 0xa0))
                {
                  s += 3;
                  continue;
                }
            }
          else if (c <= 0xf4)
            {
              if (s + 4 <= s_end
                  && (s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                  && (s[3] ^ 0x80) < 0x40
                  && (c >= 0xf1 || s[1] >= 0x90)
                  && (c < 0xf4 || (/* c == 0xf4 && */ s[1] < 0x90)))
                {
                  s += 4;
                  continue;
                }
            }
        }
      /* invalid or incomplete multibyte character */
      return s;
    }
  return NULL;
}
