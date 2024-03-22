/*
 * Copyright © 2018 Intel Corporation
 * Copyright © 2019 Valve Corporation
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

#undef NDEBUG

#include <gtest/gtest.h>
#include "util/fast_urem_by_const.h"

#define RAND_TEST_ITERATIONS 100000

static uint32_t
rand_uint(unsigned min)
{
   /* Make sure we get some small and large numbers and powers of two every
    * once in a while
    */
   int k = rand() % 64;
   if (k == 17) {
      return min + (rand() % 16);
   } else if (k == 42) {
      return UINT32_MAX - (rand() % 16);
   } else if (k == 9) {
      uint32_t r;
      do {
         r = 1ull << (rand() % 32);
      } while (r < min);
      return r;
   }

   if (min == 0) {
      uint32_t r = 0;
      for (unsigned i = 0; i < 4; i++)
         r |= ((uint32_t)rand() & 0xf) << i * 8;
      return r >> (31 - (rand() % 32));
   } else {
      uint64_t r;
      do {
         r = rand_uint(0);
      } while (r < min);
      return r;
   }
}

static void
test_case(uint32_t n, uint32_t d)
{
   assert(d >= 1);
   uint64_t magic = REMAINDER_MAGIC(d);
   /* Note: there's already an assert inside util_fast_urem32(), so the
    * EXPECT_EQ is only here to make sure the test fails with a wrong result
    * even if asserts are disabled. Ideally we could disable the assert just
    * for the test to get a better error message, but that doesn't seem too
    * easy.
    */
   EXPECT_EQ(util_fast_urem32(n, d, magic), n % d);
}

TEST(fast_urem_by_const, random)
{
   for (unsigned i = 0; i < RAND_TEST_ITERATIONS; i++) {
      uint64_t n = rand_uint(0);
      uint64_t d = rand_uint(1);
      test_case(n, d);
   }
}

TEST(fast_urem_by_const, special_cases)
{
   test_case(0, 1);
   test_case(0, UINT32_MAX);
   test_case(UINT32_MAX, 1);
   test_case(1, UINT32_MAX);
   test_case(UINT32_MAX, UINT32_MAX);
   test_case(UINT32_MAX, UINT32_MAX - 1);
   test_case(UINT32_MAX - 1, UINT32_MAX - 1);
   test_case(UINT32_MAX - 2, UINT32_MAX - 1);
}

