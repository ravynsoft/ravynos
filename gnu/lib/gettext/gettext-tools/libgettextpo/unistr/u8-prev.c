/* Iterate over previous character in UTF-8 string.
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
u8_prev (ucs4_t *puc, const uint8_t *s, const uint8_t *start)
{
  /* Keep in sync with unistr.h and u8-mbtouc-aux.c.  */
  if (s != start)
    {
      uint8_t c_1 = s[-1];

      if (c_1 < 0x80)
        {
          *puc = c_1;
          return s - 1;
        }
      if ((c_1 ^ 0x80) < 0x40)
        if (s - 1 != start)
          {
            uint8_t c_2 = s[-2];

            if (c_2 >= 0xc2 && c_2 < 0xe0)
              {
                *puc = ((unsigned int) (c_2 & 0x1f) << 6)
                       | (unsigned int) (c_1 ^ 0x80);
                return s - 2;
              }
            if ((c_2 ^ 0x80) < 0x40)
              if (s - 2 != start)
                {
                  uint8_t c_3 = s[-3];

                  if (c_3 >= 0xe0 && c_3 < 0xf0
                      && (c_3 >= 0xe1 || c_2 >= 0xa0)
                      && (c_3 != 0xed || c_2 < 0xa0))
                    {
                      *puc = ((unsigned int) (c_3 & 0x0f) << 12)
                             | ((unsigned int) (c_2 ^ 0x80) << 6)
                             | (unsigned int) (c_1 ^ 0x80);
                      return s - 3;
                    }
                  if ((c_3 ^ 0x80) < 0x40)
                    if (s - 3 != start)
                      {
                        uint8_t c_4 = s[-4];

                        if (c_4 >= 0xf0 && c_4 <= 0xf4
                            && (c_4 >= 0xf1 || c_3 >= 0x90)
                            && (c_4 < 0xf4 || (/* c_4 == 0xf4 && */ c_3 < 0x90)))
                          {
                            *puc = ((unsigned int) (c_4 & 0x07) << 18)
                                   | ((unsigned int) (c_3 ^ 0x80) << 12)
                                   | ((unsigned int) (c_2 ^ 0x80) << 6)
                                   | (unsigned int) (c_1 ^ 0x80);
                            return s - 4;
                          }
                      }
                }
          }
    }
  return NULL;
}
