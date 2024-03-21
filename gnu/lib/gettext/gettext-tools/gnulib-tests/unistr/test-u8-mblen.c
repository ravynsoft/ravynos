/* Test of u8_mblen() function.
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2010.  */

#include <config.h>

#include "unistr.h"

#include "macros.h"

int
main ()
{
  int ret;

  /* Test zero-length input.  */
  {
    static const uint8_t input[] = "";
    ret = u8_mblen (input, 0);
    ASSERT (ret == -1);
  }

  /* Test NUL unit input.  */
  {
    static const uint8_t input[] = "";
    ret = u8_mblen (input, 1);
    ASSERT (ret == 0);
  }

  /* Test ISO 646 unit input.  */
  {
    ucs4_t c;
    uint8_t buf[1];

    for (c = 1; c < 0x80; c++)
      {
        buf[0] = c;
        ret = u8_mblen (buf, 1);
        ASSERT (ret == 1);
      }
  }

  /* Test 2-byte character input.  */
  {
    static const uint8_t input[] = { 0xC3, 0x97 };
    ret = u8_mblen (input, 2);
    ASSERT (ret == 2);
  }

  /* Test 3-byte character input.  */
  {
    static const uint8_t input[] = { 0xE2, 0x82, 0xAC };
    ret = u8_mblen (input, 3);
    ASSERT (ret == 3);
  }

  /* Test 4-byte character input.  */
  {
    static const uint8_t input[] = { 0xF4, 0x8F, 0xBF, 0xBD };
    ret = u8_mblen (input, 4);
    ASSERT (ret == 4);
  }

  /* Test incomplete/invalid 1-byte input.  */
  {
    static const uint8_t input[] = { 0xC1 };
    ret = u8_mblen (input, 1);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xC3 };
    ret = u8_mblen (input, 1);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xE2 };
    ret = u8_mblen (input, 1);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xF4 };
    ret = u8_mblen (input, 1);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xFE };
    ret = u8_mblen (input, 1);
    ASSERT (ret == -1);
  }

  /* Test incomplete/invalid 2-byte input.  */
  {
    static const uint8_t input[] = { 0xE0, 0x9F };
    ret = u8_mblen (input, 2);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xE2, 0x82 };
    ret = u8_mblen (input, 2);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xE2, 0xD0 };
    ret = u8_mblen (input, 2);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xF0, 0x8F };
    ret = u8_mblen (input, 2);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xF3, 0x8F };
    ret = u8_mblen (input, 2);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xF3, 0xD0 };
    ret = u8_mblen (input, 2);
    ASSERT (ret == -1);
  }

  /* Test incomplete/invalid 3-byte input.  */
  {
    static const uint8_t input[] = { 0xF3, 0x8F, 0xBF };
    ret = u8_mblen (input, 3);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xF3, 0xD0, 0xBF };
    ret = u8_mblen (input, 3);
    ASSERT (ret == -1);
  }
  {
    static const uint8_t input[] = { 0xF3, 0x8F, 0xD0 };
    ret = u8_mblen (input, 3);
    ASSERT (ret == -1);
  }

  return 0;
}
