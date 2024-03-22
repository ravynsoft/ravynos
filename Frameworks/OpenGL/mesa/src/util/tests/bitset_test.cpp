/*
 * Copyright © 2019 Red Hat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <gtest/gtest.h>
#include "util/bitset.h"

TEST(bitset, sizes)
{
   EXPECT_EQ(sizeof(BITSET_WORD), 4);

   BITSET_DECLARE(mask32, 32);
   BITSET_DECLARE(mask64, 64);
   BITSET_DECLARE(mask128, 128);

   EXPECT_EQ(sizeof(mask32), 4);
   EXPECT_EQ(sizeof(mask64), 8);
   EXPECT_EQ(sizeof(mask128), 16);
}

TEST(bitset, test_set_clear)
{
   BITSET_DECLARE(mask128, 128);
   BITSET_ZERO(mask128);

   for (int i = 0; i < 128; i++) {
      EXPECT_EQ(BITSET_TEST(mask128, i), false);
      BITSET_SET(mask128, i);
      EXPECT_EQ(BITSET_TEST(mask128, i), true);
      BITSET_CLEAR(mask128, i);
      EXPECT_EQ(BITSET_TEST(mask128, i), false);
   }
}

TEST(bitset, test_set_ones)
{
   BITSET_DECLARE(mask128, 128);
   BITSET_ONES(mask128);

   EXPECT_EQ(BITSET_FFS(mask128), 1);

   for (int i = 0; i < 128; i++) {
      EXPECT_EQ(BITSET_TEST(mask128, i), true);
      BITSET_CLEAR(mask128, i);
      EXPECT_EQ(BITSET_TEST(mask128, i), false);
      BITSET_SET(mask128, i);
      EXPECT_EQ(BITSET_TEST(mask128, i), true);
   }
}

TEST(bitset, test_basic_range)
{
   BITSET_DECLARE(mask128, 128);
   BITSET_ZERO(mask128);

   const int max_set = 15;
   BITSET_SET_RANGE_INSIDE_WORD(mask128, 0, max_set);
   EXPECT_EQ(!BITSET_TEST_RANGE_INSIDE_WORD(mask128, 0, max_set, 0), true);
   EXPECT_EQ(!BITSET_TEST_RANGE_INSIDE_WORD(mask128, max_set + 1, max_set + 15, 0), false);
   for (int i = 0; i < 128; i++) {
      if (i <= max_set)
         EXPECT_EQ(BITSET_TEST(mask128, i), true);
      else
         EXPECT_EQ(BITSET_TEST(mask128, i), false);
   }
   BITSET_CLEAR_RANGE(mask128, 0, max_set);
   EXPECT_EQ(!BITSET_TEST_RANGE_INSIDE_WORD(mask128, 0, max_set, 0), false);
   for (int i = 0; i < 128; i++) {
      EXPECT_EQ(BITSET_TEST(mask128, i), false);
   }
}

TEST(bitset, test_bitset_ffs)
{
   BITSET_DECLARE(mask128, 128);
   BITSET_ZERO(mask128);

   EXPECT_EQ(BITSET_FFS(mask128), 0);

   BITSET_SET(mask128, 14);
   EXPECT_EQ(BITSET_FFS(mask128), 15);

   BITSET_SET(mask128, 28);
   EXPECT_EQ(BITSET_FFS(mask128), 15);

   BITSET_CLEAR(mask128, 14);
   EXPECT_EQ(BITSET_FFS(mask128), 29);

   BITSET_SET_RANGE_INSIDE_WORD(mask128, 14, 18);
   EXPECT_EQ(BITSET_FFS(mask128), 15);
}

TEST(bitset, test_range_bits)
{
   BITSET_DECLARE(mask128, 128);
   BITSET_ZERO(mask128);

   BITSET_SET_RANGE_INSIDE_WORD(mask128, 0, 31);
   BITSET_SET_RANGE_INSIDE_WORD(mask128, 32, 63);
   BITSET_SET_RANGE_INSIDE_WORD(mask128, 64, 95);
   BITSET_SET_RANGE_INSIDE_WORD(mask128, 96, 127);
   for (int i = 0; i < 128; i++) {
      EXPECT_EQ(BITSET_TEST(mask128, i), true);
   }

   BITSET_ZERO(mask128);
   BITSET_SET_RANGE(mask128, 20, 80);
   for (int i = 0; i <= 19; i++)
      EXPECT_EQ(BITSET_TEST(mask128, i), false);
   for (int i = 20; i <= 80; i++)
      EXPECT_EQ(BITSET_TEST(mask128, i), true);
   for (int i = 81; i <= 127; i++)
      EXPECT_EQ(BITSET_TEST(mask128, i), false);

   BITSET_ZERO(mask128);
   BITSET_SET(mask128, 20);
   BITSET_SET(mask128, 80);
   EXPECT_EQ(BITSET_TEST_RANGE(mask128, 0, 128), true);
   EXPECT_EQ(BITSET_TEST_RANGE(mask128, 0, 19), false);
   EXPECT_EQ(BITSET_TEST_RANGE(mask128, 21, 79), false);
   EXPECT_EQ(BITSET_TEST_RANGE(mask128, 81, 127), false);
   EXPECT_EQ(BITSET_TEST_RANGE(mask128, 0, 79), true);
   EXPECT_EQ(BITSET_TEST_RANGE(mask128, 21, 128), true);

   BITSET_ONES(mask128);
   BITSET_CLEAR_RANGE(mask128, 20, 80);
   for (int i = 0; i <= 19; i++)
      EXPECT_EQ(BITSET_TEST(mask128, i), true);
   for (int i = 20; i <= 80; i++)
      EXPECT_EQ(BITSET_TEST(mask128, i), false);
   for (int i = 81; i <= 127; i++)
      EXPECT_EQ(BITSET_TEST(mask128, i), true);
}

TEST(bitset, test_and)
{
   BITSET_DECLARE(r, 128);
   BITSET_DECLARE(a, 128);
   BITSET_DECLARE(b, 128);
   BITSET_ZERO(r);
   BITSET_ZERO(a);
   BITSET_ZERO(b);

   BITSET_AND(r, a, b);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);


   BITSET_SET_RANGE_INSIDE_WORD(a, 32, 63);
   BITSET_SET_RANGE_INSIDE_WORD(b, 96, 127);
   BITSET_AND(r, a, b);

   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);


   BITSET_SET(a, 80);
   BITSET_SET(b, 80);
   BITSET_AND(r, a, b);

   EXPECT_EQ(BITSET_TEST(r, 80), true);

   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);
}

TEST(bitset, test_or)
{
   BITSET_DECLARE(r, 128);
   BITSET_DECLARE(a, 128);
   BITSET_DECLARE(b, 128);
   BITSET_ZERO(r);
   BITSET_ZERO(a);
   BITSET_ZERO(b);

   BITSET_OR(r, a, b);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);


   BITSET_SET_RANGE_INSIDE_WORD(a, 32, 63);
   BITSET_SET_RANGE_INSIDE_WORD(b, 96, 127);
   BITSET_OR(r, a, b);

   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), true);


   BITSET_SET(a, 80);
   BITSET_OR(r, a, b);
   EXPECT_EQ(BITSET_TEST(r, 80), true);

   BITSET_SET(b, 81);
   BITSET_OR(r, a, b);
   EXPECT_EQ(BITSET_TEST(r, 81), true);

   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), true);
}

TEST(bitset, test_not)
{
   BITSET_DECLARE(r, 128);
   BITSET_ZERO(r);

   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);

   BITSET_NOT(r);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), true);

   BITSET_CLEAR_RANGE(r, 32, 63);
   BITSET_NOT(r);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);
}

TEST(bitset, test_shr_zero)
{
   BITSET_DECLARE(r, 128);

   BITSET_ZERO(r);
   BITSET_SET(r, 127);

   BITSET_SHR(r, 0);

   EXPECT_EQ(BITSET_TEST(r, 127), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), true);
}

TEST(bitset, test_shl_zero)
{
   BITSET_DECLARE(r, 128);

   BITSET_ZERO(r);
   BITSET_SET(r, 0);

   BITSET_SHL(r, 0);

   EXPECT_EQ(BITSET_TEST(r, 0), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);
}

TEST(bitset, test_shr_walking_bit)
{
   BITSET_DECLARE(r, 128);

   BITSET_ZERO(r);
   BITSET_SET(r, 127);

   for (int i = 127; i >= 0; i--) {
      EXPECT_EQ(BITSET_TEST(r, i), true);
      BITSET_SHR(r, 1);
   }

   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);
}

TEST(bitset, test_shl_walking_bit)
{
   BITSET_DECLARE(r, 128);

   BITSET_ZERO(r);
   BITSET_SET(r, 0);

   for (unsigned int i = 0; i < 128; i++) {
      EXPECT_EQ(BITSET_TEST(r, i), true);
      BITSET_SHL(r, 1);
   }

   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);
}

TEST(bitset, test_shr_multiple_words)
{
   BITSET_DECLARE(r, 128);

   BITSET_ZERO(r);
   BITSET_SET(r, 127);
   BITSET_SHR(r, 50);

   EXPECT_EQ(BITSET_TEST(r, 127), false);
   EXPECT_EQ(BITSET_TEST(r, 77), true);


   BITSET_ZERO(r);
   BITSET_SET(r, 127);
   BITSET_SHR(r, 80);

   EXPECT_EQ(BITSET_TEST(r, 127), false);
   EXPECT_EQ(BITSET_TEST(r, 47), true);


   BITSET_ZERO(r);
   BITSET_SET(r, 127);
   BITSET_SHR(r, 126);

   EXPECT_EQ(BITSET_TEST(r, 127), false);
   EXPECT_EQ(BITSET_TEST(r, 1), true);
}

TEST(bitset, test_shl_multiple_words)
{
   BITSET_DECLARE(r, 128);

   BITSET_ZERO(r);
   BITSET_SET(r, 0);
   BITSET_SHL(r, 50);

   EXPECT_EQ(BITSET_TEST(r, 0), false);
   EXPECT_EQ(BITSET_TEST(r, 50), true);


   BITSET_ZERO(r);
   BITSET_SET(r, 0);
   BITSET_SHL(r, 80);

   EXPECT_EQ(BITSET_TEST(r, 0), false);
   EXPECT_EQ(BITSET_TEST(r, 80), true);


   BITSET_ZERO(r);
   BITSET_SET(r, 0);
   BITSET_SHL(r, 126);

   EXPECT_EQ(BITSET_TEST(r, 0), false);
   EXPECT_EQ(BITSET_TEST(r, 126), true);
}

TEST(bitset, test_shr_two_words)
{
   BITSET_DECLARE(r, 64);

   BITSET_ZERO(r);
   BITSET_SET(r, 63);
   BITSET_SHR(r, 50);

   EXPECT_EQ(BITSET_TEST(r, 63), false);
   EXPECT_EQ(BITSET_TEST(r, 13), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), false);
}

TEST(bitset, test_shl_two_words)
{
   BITSET_DECLARE(r, 64);

   BITSET_ZERO(r);
   BITSET_SET(r, 0);
   BITSET_SHL(r, 50);

   EXPECT_EQ(BITSET_TEST(r, 0), false);
   EXPECT_EQ(BITSET_TEST(r, 50), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), true);
}

TEST(bitset, test_setrange_across_word_boundary)
{
   BITSET_DECLARE(r, 128);
   BITSET_ZERO(r);

   BITSET_SET_RANGE(r, 62, 65);

   EXPECT_EQ(BITSET_TEST_RANGE(r, 0, 31), false);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 32, 63), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 64, 95), true);
   EXPECT_EQ(BITSET_TEST_RANGE(r, 96, 127), false);

   EXPECT_EQ(BITSET_TEST(r, 61), false);

   for (int i = 62; i <= 65; i++)
      EXPECT_EQ(BITSET_TEST(r, i), true);

   EXPECT_EQ(BITSET_TEST(r, 66), false);
}
