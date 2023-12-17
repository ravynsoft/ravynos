/* Test of u8_mbtoucr() function.
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
  ucs4_t uc;
  int ret;

  /* Test NUL unit input.  */
  {
    static const uint8_t input[] = "";
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 1);
    ASSERT (ret == 1);
    ASSERT (uc == 0);
  }

  /* Test ISO 646 unit input.  */
  {
    ucs4_t c;
    uint8_t buf[1];

    for (c = 0; c < 0x80; c++)
      {
        buf[0] = c;
        uc = 0xBADFACE;
        ret = u8_mbtoucr (&uc, buf, 1);
        ASSERT (ret == 1);
        ASSERT (uc == c);
      }
  }

  /* Test 2-byte character input.  */
  {
    static const uint8_t input[] = { 0xC3, 0x97 };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 2);
    ASSERT (ret == 2);
    ASSERT (uc == 0x00D7);
  }

  /* Test 3-byte character input.  */
  {
    static const uint8_t input[] = { 0xE2, 0x82, 0xAC };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 3);
    ASSERT (ret == 3);
    ASSERT (uc == 0x20AC);
  }

  /* Test 4-byte character input.  */
  {
    static const uint8_t input[] = { 0xF4, 0x8F, 0xBF, 0xBD };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 4);
    ASSERT (ret == 4);
    ASSERT (uc == 0x10FFFD);
  }

  /* Test incomplete/invalid 1-byte input.  */
  {
    static const uint8_t input[] = { 0xC1 };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 1);
    ASSERT (ret == -1);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xC3 };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 1);
    ASSERT (ret == -2);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xE2 };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 1);
    ASSERT (ret == -2);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xF4 };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 1);
    ASSERT (ret == -2);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xFE };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 1);
    ASSERT (ret == -1);
    ASSERT (uc == 0xFFFD);
  }

  /* Test incomplete/invalid 2-byte input.  */
  {
    static const uint8_t input[] = { 0xE0, 0x9F };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 2);
    ASSERT (ret == -1);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xE2, 0x82 };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 2);
    ASSERT (ret == -2);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xE2, 0xD0 };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 2);
    ASSERT (ret == -1);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xF0, 0x8F };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 2);
    ASSERT (ret == -1);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xF3, 0x8F };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 2);
    ASSERT (ret == -2);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xF3, 0xD0 };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 2);
    ASSERT (ret == -1);
    ASSERT (uc == 0xFFFD);
  }

  /* Test incomplete/invalid 3-byte input.  */
  {
    static const uint8_t input[] = { 0xF3, 0x8F, 0xBF };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 3);
    ASSERT (ret == -2);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xF3, 0xD0, 0xBF };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 3);
    ASSERT (ret == -1);
    ASSERT (uc == 0xFFFD);
  }
  {
    static const uint8_t input[] = { 0xF3, 0x8F, 0xD0 };
    uc = 0xBADFACE;
    ret = u8_mbtoucr (&uc, input, 3);
    ASSERT (ret == -1);
    ASSERT (uc == 0xFFFD);
  }

  return 0;
}
