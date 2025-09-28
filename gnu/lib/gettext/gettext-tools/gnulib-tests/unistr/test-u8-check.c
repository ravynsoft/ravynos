/* Test of u8_check() function.
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
  /* Test empty string.  */
  {
    static const uint8_t input[] = "";
    ASSERT (u8_check (input, 0) == NULL);
  }

  /* Test valid non-empty string.  */
  {
    static const uint8_t input[] = /* "Данило Шеган" */
      "\320\224\320\260\320\275\320\270\320\273\320\276 \320\250\320\265\320\263\320\260\320\275";
    ASSERT (u8_check (input, sizeof (input) - 1) == NULL);
  }

  /* Test out-of-range character with 4 bytes: U+110000.  */
  {
    static const uint8_t input[] = "\320\224\320\260\364\220\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test out-of-range character with 5 bytes: U+200000.  */
  {
    static const uint8_t input[] = "\320\224\320\260\370\210\200\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test out-of-range character with 6 bytes: U+4000000.  */
  {
    static const uint8_t input[] = "\320\224\320\260\374\204\200\200\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test invalid lead byte.  */
  {
    static const uint8_t input[] = "\320\224\320\260\376\200\200\200\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\377\200\200\200\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test overlong 2-byte character.  */
  {
    static const uint8_t input[] = "\320\224\320\260\301\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test overlong 3-byte character.  */
  {
    static const uint8_t input[] = "\320\224\320\260\340\200\277";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test overlong 4-byte character.  */
  {
    static const uint8_t input[] = "\320\224\320\260\360\200\277\277";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test invalid bytes in 2-byte character.  */
  {
    static const uint8_t input[] = "\320\224\320\260\302\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == NULL);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\302\100";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\302\300";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test invalid bytes in 3-byte character.  */
  {
    static const uint8_t input[] = "\320\224\320\260\342\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == NULL);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\342\100\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\342\300\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\342\200\100";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\342\200\300";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test invalid bytes in 4-byte character.  */
  {
    static const uint8_t input[] = "\320\224\320\260\362\200\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == NULL);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\362\100\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\362\300\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\362\200\100\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\362\200\300\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\362\200\200\100";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\362\200\200\300";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test truncated/incomplete 2-byte character.  */
  {
    static const uint8_t input[] = "\320\224\320\260\302";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test truncated/incomplete 3-byte character.  */
  {
    static const uint8_t input[] = "\320\224\320\260\342\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test truncated/incomplete 4-byte character.  */
  {
    static const uint8_t input[] = "\320\224\320\260\362\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test missing lead byte.  */
  {
    static const uint8_t input[] = "\320\224\320\260\200\200\200\200\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  /* Test surrogate codepoints.  */
  {
    static const uint8_t input[] = "\320\224\320\260\355\240\200\355\260\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }
  {
    static const uint8_t input[] = "\320\224\320\260\355\260\200";
    ASSERT (u8_check (input, sizeof (input) - 1) == input + 4);
  }

  return 0;
}
