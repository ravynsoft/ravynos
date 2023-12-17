/* Test of u8_prev() function.
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

static int
check (const uint8_t *input, size_t input_length, ucs4_t *puc)
{
  ucs4_t uc;

  /* Test recognition when at the beginning of the string.  */
  if (u8_prev (&uc, input + input_length, input) != input)
    return 1;

  /* Test recognition when preceded by a 1-unit character.  */
  {
    uint8_t buf[100];
    uint8_t *ptr;
    size_t i;
    ucs4_t uc1;

    ptr = buf;
    *ptr++ = 'x';
    for (i = 0; i < input_length; i++)
      ptr[i] = input[i];

    if (u8_prev (&uc1, ptr + input_length, buf) != ptr)
      return 2;
    if (uc1 != uc)
      return 3;
  }

  /* Test recognition when preceded by a 2-unit character.  */
  {
    uint8_t buf[100];
    uint8_t *ptr;
    size_t i;
    ucs4_t uc1;

    ptr = buf;
    *ptr++ = 0xC3;
    *ptr++ = 0x97;
    for (i = 0; i < input_length; i++)
      ptr[i] = input[i];

    if (u8_prev (&uc1, ptr + input_length, buf) != ptr)
      return 4;
    if (uc1 != uc)
      return 5;
  }

  /* Test recognition when preceded by a 3-unit character.  */
  {
    uint8_t buf[100];
    uint8_t *ptr;
    size_t i;
    ucs4_t uc1;

    ptr = buf;
    *ptr++ = 0xE2;
    *ptr++ = 0x84;
    *ptr++ = 0x82;
    for (i = 0; i < input_length; i++)
      ptr[i] = input[i];

    if (u8_prev (&uc1, ptr + input_length, buf) != ptr)
      return 6;
    if (uc1 != uc)
      return 7;
  }

  /* Test recognition when preceded by a 4-unit character.  */
  {
    uint8_t buf[100];
    uint8_t *ptr;
    size_t i;
    ucs4_t uc1;

    ptr = buf;
    *ptr++ = 0xF0;
    *ptr++ = 0x9D;
    *ptr++ = 0x94;
    *ptr++ = 0x9E;
    for (i = 0; i < input_length; i++)
      ptr[i] = input[i];

    if (u8_prev (&uc1, ptr + input_length, buf) != ptr)
      return 8;
    if (uc1 != uc)
      return 9;
  }

  *puc = uc;
  return 0;
}

static int
check_invalid (const uint8_t *input, size_t input_length)
{
  ucs4_t uc;

  /* Test recognition when at the beginning of the string.  */
  uc = 0xBADFACE;
  if (u8_prev (&uc, input + input_length, input) != NULL)
    return 1;
  if (uc != 0xBADFACE)
    return 2;

  /* Test recognition when preceded by a 1-unit character.  */
  {
    uint8_t buf[100];
    uint8_t *ptr;
    size_t i;

    ptr = buf;
    *ptr++ = 'x';
    for (i = 0; i < input_length; i++)
      ptr[i] = input[i];

    uc = 0xBADFACE;
    if (u8_prev (&uc, ptr + input_length, buf) != NULL)
      return 3;
    if (uc != 0xBADFACE)
      return 4;
  }

  /* Test recognition when preceded by a 2-unit character.  */
  {
    uint8_t buf[100];
    uint8_t *ptr;
    size_t i;

    ptr = buf;
    *ptr++ = 0xC3;
    *ptr++ = 0x97;
    for (i = 0; i < input_length; i++)
      ptr[i] = input[i];

    uc = 0xBADFACE;
    if (u8_prev (&uc, ptr + input_length, buf) != NULL)
      return 5;
    if (uc != 0xBADFACE)
      return 6;
  }

  /* Test recognition when preceded by a 3-unit character.  */
  {
    uint8_t buf[100];
    uint8_t *ptr;
    size_t i;

    ptr = buf;
    *ptr++ = 0xE2;
    *ptr++ = 0x84;
    *ptr++ = 0x82;
    for (i = 0; i < input_length; i++)
      ptr[i] = input[i];

    uc = 0xBADFACE;
    if (u8_prev (&uc, ptr + input_length, buf) != NULL)
      return 7;
    if (uc != 0xBADFACE)
      return 8;
  }

  /* Test recognition when preceded by a 4-unit character.  */
  {
    uint8_t buf[100];
    uint8_t *ptr;
    size_t i;

    ptr = buf;
    *ptr++ = 0xF0;
    *ptr++ = 0x9D;
    *ptr++ = 0x94;
    *ptr++ = 0x9E;
    for (i = 0; i < input_length; i++)
      ptr[i] = input[i];

    uc = 0xBADFACE;
    if (u8_prev (&uc, ptr + input_length, buf) != NULL)
      return 9;
    if (uc != 0xBADFACE)
      return 10;
  }

  return 0;
}

int
main ()
{
  ucs4_t uc;

  /* Test ISO 646 unit input.  */
  {
    ucs4_t c;
    uint8_t buf[1];

    for (c = 0; c < 0x80; c++)
      {
        buf[0] = c;
        uc = 0xBADFACE;
        ASSERT (check (buf, 1, &uc) == 0);
        ASSERT (uc == c);
      }
  }

  /* Test 2-byte character input.  */
  {
    static const uint8_t input[] = { 0xC3, 0x97 };
    uc = 0xBADFACE;
    ASSERT (check (input, SIZEOF (input), &uc) == 0);
    ASSERT (uc == 0x00D7);
  }

  /* Test 3-byte character input.  */
  {
    static const uint8_t input[] = { 0xE2, 0x82, 0xAC };
    uc = 0xBADFACE;
    ASSERT (check (input, SIZEOF (input), &uc) == 0);
    ASSERT (uc == 0x20AC);
  }

  /* Test 4-byte character input.  */
  {
    static const uint8_t input[] = { 0xF4, 0x8F, 0xBF, 0xBD };
    uc = 0xBADFACE;
    ASSERT (check (input, SIZEOF (input), &uc) == 0);
    ASSERT (uc == 0x10FFFD);
  }

  /* Test incomplete/invalid 1-byte input.  */
  {
    static const uint8_t input[] = { 0xC1 };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xC3 };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xE2 };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xF4 };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xFE };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }

  /* Test incomplete/invalid 2-byte input.  */
  {
    static const uint8_t input[] = { 0xE0, 0x9F };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xE2, 0x82 };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xE2, 0xD0 };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xF0, 0x8F };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xF3, 0x8F };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xF3, 0xD0 };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }

  /* Test incomplete/invalid 3-byte input.  */
  {
    static const uint8_t input[] = { 0xF3, 0x8F, 0xBF };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xF3, 0xE4, 0xBF };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }
  {
    static const uint8_t input[] = { 0xF3, 0x8F, 0xD0 };
    ASSERT (check_invalid (input, SIZEOF (input)) == 0);
  }

  return 0;
}
