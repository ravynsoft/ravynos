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

#ifndef MESA_GTEST_EXTRAS_H
#define MESA_GTEST_EXTRAS_H

#include <gtest/gtest.h>

template <typename T>
static testing::AssertionResult
array_equal_pred(const char *a_expr,
                 const char *b_expr,
                 const char *c_expr,
                 const T *a,
                 const T *b,
                 size_t count)
{
   if (memcmp(a, b, count * sizeof(T))) {
      std::stringstream result;

      unsigned mismatches = 0;
      for (size_t i = 0; i < count; i++) {
         if (a[i] != b[i])
            mismatches++;
      }

      result << "Expected " << count << " values to be equal but found "
             << mismatches << " that differ:\n\n";

      result << std::right << std::setfill('0');

      const unsigned values_per_line = 16 / sizeof(T);

      result << a_expr << " values are:\n";
      for (size_t i = 0; i < count; i++) {
         if (i % values_per_line == 0)
            result << "\n  [" << std::dec << std::setw(3) << i << "]";
         result << " "
                << (a[i] == b[i] ? ' ' : '*')
                << std::hex << std::setw(sizeof(T) * 2) << +a[i];
      }
      result << "\n\n";

      result << b_expr << " values are:\n";
      for (size_t i = 0; i < count; i++) {
         if (i % values_per_line == 0)
            result << "\n  [" << std::dec << std::setw(3) << i << "]";
         result << " "
                << (a[i] == b[i] ? ' ' : '*')
                << std::hex << std::setw(sizeof(T) * 2) << +b[i];
      }
      result << "\n";

      return testing::AssertionFailure() << result.str();
   } else {
      return testing::AssertionSuccess();
   }
}

#define EXPECT_U8_ARRAY_EQUAL(a, b, count)  EXPECT_PRED_FORMAT3(array_equal_pred<uint8_t>, a, b, count)
#define ASSERT_U8_ARRAY_EQUAL(a, b, count)  ASSERT_PRED_FORMAT3(array_equal_pred<uint8_t>, a, b, count)
#define EXPECT_U16_ARRAY_EQUAL(a, b, count) EXPECT_PRED_FORMAT3(array_equal_pred<uint16_t>, a, b, count)
#define ASSERT_U16_ARRAY_EQUAL(a, b, count) ASSERT_PRED_FORMAT3(array_equal_pred<uint16_t>, a, b, count)
#define EXPECT_U32_ARRAY_EQUAL(a, b, count) EXPECT_PRED_FORMAT3(array_equal_pred<uint32_t>, a, b, count)
#define ASSERT_U32_ARRAY_EQUAL(a, b, count) ASSERT_PRED_FORMAT3(array_equal_pred<uint32_t>, a, b, count)
#define EXPECT_U64_ARRAY_EQUAL(a, b, count) EXPECT_PRED_FORMAT3(array_equal_pred<uint64_t>, a, b, count)
#define ASSERT_U64_ARRAY_EQUAL(a, b, count) ASSERT_PRED_FORMAT3(array_equal_pred<uint64_t>, a, b, count)

#endif /* MESA_GTEST_EXTRAS_H */
