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
#include "util/macros.h"

#define MESA_UINT24_MAX  16777215
#define MESA_INT24_MAX   8388607
#define MESA_INT24_MIN   (-8388607-1)

#define MESA_UINT12_MAX  4095
#define MESA_INT12_MAX   2047
#define MESA_INT12_MIN   (-2047-1)

#define MESA_UINT10_MAX  1023
#define MESA_INT10_MAX   511
#define MESA_INT10_MIN   (-511-1)

TEST(int_min_max, u_intN_min)
{
   EXPECT_EQ(INT64_MIN, u_intN_min(64));
   EXPECT_EQ(INT32_MIN, u_intN_min(32));
   EXPECT_EQ(INT16_MIN, u_intN_min(16));
   EXPECT_EQ(INT8_MIN,  u_intN_min(8));

   EXPECT_EQ(MESA_INT24_MIN, u_intN_min(24));
   EXPECT_EQ(MESA_INT12_MIN, u_intN_min(12));
   EXPECT_EQ(MESA_INT10_MIN, u_intN_min(10));
}

TEST(int_min_max, u_intN_max)
{
   EXPECT_EQ(INT64_MAX, u_intN_max(64));
   EXPECT_EQ(INT32_MAX, u_intN_max(32));
   EXPECT_EQ(INT16_MAX, u_intN_max(16));
   EXPECT_EQ(INT8_MAX,  u_intN_max(8));

   EXPECT_EQ(MESA_INT24_MAX, u_intN_max(24));
   EXPECT_EQ(MESA_INT12_MAX, u_intN_max(12));
   EXPECT_EQ(MESA_INT10_MAX, u_intN_max(10));
}

TEST(int_min_max, u_uintN_max)
{
   EXPECT_EQ(UINT64_MAX, u_uintN_max(64));
   EXPECT_EQ(UINT32_MAX, u_uintN_max(32));
   EXPECT_EQ(UINT16_MAX, u_uintN_max(16));
   EXPECT_EQ(UINT8_MAX,  u_uintN_max(8));

   EXPECT_EQ(MESA_UINT24_MAX, u_uintN_max(24));
   EXPECT_EQ(MESA_UINT12_MAX, u_uintN_max(12));
   EXPECT_EQ(MESA_UINT10_MAX, u_uintN_max(10));
}
