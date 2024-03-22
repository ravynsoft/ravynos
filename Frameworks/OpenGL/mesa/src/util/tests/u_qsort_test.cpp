/*
 * Copyright © 2021 Intel Corporation
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

#include "util/u_qsort.h"

constexpr int CONTEXT_CHECK = 12345;

static int
cmp_func(const void *a, const void *b, void *ctx)
{
   int check = *reinterpret_cast<const int*>(ctx);
   EXPECT_EQ(check, CONTEXT_CHECK);

   int elem1 = *reinterpret_cast<const int*>(a);
   int elem2 = *reinterpret_cast<const int*>(b);
   return elem1 - elem2;
}

TEST(u_qsort_test, qsort_test)
{
   int data[] = { 3, 6, 4, 9, 10, 2, 5, 7, 8, 1 };
   int ctx = CONTEXT_CHECK;

   util_qsort_r(data, sizeof(data) / sizeof(data[0]),
                sizeof(data[0]), cmp_func,
                reinterpret_cast<void *>(&ctx));

   for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); ++i) {
      EXPECT_EQ(data[i], i + 1);
   }
}
