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

#include <math.h>
#include <gtest/gtest.h>

#include "util/half_float.h"
#include "util/u_math.h"

/* math.h has some defines for these, but they have some compiler dependencies
 * and can potentially raise exceptions.
 */
#define TEST_POS_INF (uif(0x7f800000))
#define TEST_NEG_INF (uif(0xff800000))
#define TEST_NAN (uif(0x7fc00000))

#define HALF_POS_INF 0x7c00
#define HALF_NEG_INF 0xfc00
#define HALF_NAN 0x7e00

#ifndef HAVE_ISSIGNALING
static bool issignaling(float x)
{
   uint32_t ui = fui(x);
   return (((ui >> 23) & 0xff) == 0xff) && !(ui & (1 << 22));
}
#endif

/* The sign of the bit for signaling is different on some old processors
 * (PA-RISC, old MIPS without IEEE-754-2008 support).
 *
 * Disable the tests on those platforms, because it's not clear how to
 * correctly handle NaNs when the CPU and GPU differ in their convention.
 */
#if DETECT_ARCH_HPPA || ((DETECT_ARCH_MIPS || DETECT_ARCH_MIPS64) && !defined __mips_nan2008)
#define IEEE754_2008_NAN 0
#else
#define IEEE754_2008_NAN 1
#endif

/* Sanity test our inf test values */
TEST(half_to_float_test, inf_test)
{
   EXPECT_TRUE(isinf(TEST_POS_INF));
   EXPECT_TRUE(isinf(TEST_NEG_INF));
}

/* Make sure that our 32-bit float nan test value we're using is a
 * non-signaling NaN.
 */
#if IEEE754_2008_NAN
TEST(half_to_float_test, nan_test)
#else
TEST(half_to_float_test, DISABLED_nan_test)
#endif
{
   EXPECT_TRUE(isnan(TEST_NAN));
   EXPECT_FALSE(issignaling(TEST_NAN));
}

static void
test_half_to_float_limits(float (*func)(uint16_t))
{
   /* Positive and negative 0. */
   EXPECT_EQ(func(0), 0.0f);
   EXPECT_EQ(fui(func(0x8000)), fui(-0.0f));

   /* Max normal number */
   EXPECT_EQ(func(0x7bff), 65504.0f);

   float nan = func(HALF_NAN);
   EXPECT_TRUE(isnan(nan));
   EXPECT_FALSE(issignaling(nan));

   /* inf */
   EXPECT_EQ(func(HALF_POS_INF), TEST_POS_INF);
   /* -inf */
   EXPECT_EQ(func(HALF_NEG_INF), TEST_NEG_INF);
}

/* Test the optionally HW instruction-using path. */
#if IEEE754_2008_NAN
TEST(half_to_float_test, half_to_float_test)
#else
TEST(half_to_float_test, DISABLED_half_to_float_test)
#endif
{
   test_half_to_float_limits(_mesa_half_to_float);
}

#if IEEE754_2008_NAN
TEST(half_to_float_test, half_to_float_slow_test)
#else
TEST(half_to_float_test, DISABLED_half_to_float_slow_test)
#endif
{
   test_half_to_float_limits(_mesa_half_to_float_slow);
}

static void
test_float_to_half_limits(uint16_t (*func)(float))
{
   /* Positive and negative 0. */
   EXPECT_EQ(func(0.0f), 0);
   EXPECT_EQ(func(-0.0f), 0x8000);

   /* Max normal number */
   EXPECT_EQ(func(65504.0f), 0x7bff);

   uint16_t nan = func(TEST_NAN);
   EXPECT_EQ((nan & 0xfc00), 0x7c00); /* exponent is all 1s */
   EXPECT_TRUE(nan & (1 << 9)); /* mantissa is quiet nan */

   EXPECT_EQ(func(TEST_POS_INF), HALF_POS_INF);
   EXPECT_EQ(func(TEST_NEG_INF), HALF_NEG_INF);
}

TEST(float_to_half_test, float_to_half_test)
{
   test_float_to_half_limits(_mesa_float_to_half);
}

TEST(float_to_float16_rtne_test, float_to_float16_rtne_test)
{
   test_float_to_half_limits(_mesa_float_to_float16_rtne);
}

/* no rtne_slow variant -- rtne is just _mesa_float_to_half(). */

TEST(float_to_float16_rtz_test, float_to_float16_rtz_test)
{
   test_float_to_half_limits(_mesa_float_to_float16_rtz);
}

TEST(float_to_float16_rtz_slow_test, float_to_float16_rtz_test)
{
   test_float_to_half_limits(_mesa_float_to_float16_rtz_slow);
}
