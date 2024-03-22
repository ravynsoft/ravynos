/*
 * Copyright © 2016 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <gtest/gtest.h>

#include "c11/time.h"
#include "util/timespec.h"

#include <limits>

TEST(timespec_test, timespec_add)
{
   struct timespec a, b, r;

   a.tv_sec = 1;
   a.tv_nsec = NSEC_PER_SEC - 1;
   b.tv_sec = 1;
   b.tv_nsec = 2;
   timespec_add(&r, &a, &b);
   EXPECT_EQ(r.tv_sec, 3);
   EXPECT_EQ(r.tv_nsec, 1);
}

TEST(timespec_test, timespec_sub)
{
   struct timespec a, b, r;

   a.tv_sec = 1;
   a.tv_nsec = 1;
   b.tv_sec = 0;
   b.tv_nsec = 2;
   timespec_sub(&r, &a, &b);
   EXPECT_EQ(r.tv_sec, 0);
   EXPECT_EQ(r.tv_nsec, NSEC_PER_SEC - 1);
}

TEST(timespec_test, timespec_to_nsec)
{
   struct timespec a;

   a.tv_sec = 4;
   a.tv_nsec = 4;
   EXPECT_EQ(timespec_to_nsec(&a), (NSEC_PER_SEC * 4ULL) + 4);
}

TEST(timespec_test, timespec_to_usec)
{
   struct timespec a;

   a.tv_sec = 4;
   a.tv_nsec = 4000;
   EXPECT_EQ(timespec_to_usec(&a), (4000000ULL) + 4);
}

TEST(timespec_test, timespec_to_msec)
{
   struct timespec a;

   a.tv_sec = 4;
   a.tv_nsec = 4000000;
   EXPECT_EQ(timespec_to_msec(&a), (4000ULL) + 4);
}

TEST(timespec_test, timespec_to_proto)
{
   struct timespec a;
   uint32_t tv_sec_hi;
   uint32_t tv_sec_lo;
   uint32_t tv_nsec;

   a.tv_sec = 0;
   a.tv_nsec = 0;
   timespec_to_proto(&a, &tv_sec_hi, &tv_sec_lo, &tv_nsec);
   EXPECT_EQ(0, tv_sec_hi);
   EXPECT_EQ(0, tv_sec_lo);
   EXPECT_EQ(0, tv_nsec);

   a.tv_sec = 1234;
   a.tv_nsec = NSEC_PER_SEC - 1;
   timespec_to_proto(&a, &tv_sec_hi, &tv_sec_lo, &tv_nsec);
   EXPECT_EQ(0, tv_sec_hi);
   EXPECT_EQ(1234, tv_sec_lo);
   EXPECT_EQ(NSEC_PER_SEC - 1, tv_nsec);

   a.tv_sec = (time_t)0x7000123470005678LL;
   a.tv_nsec = 1;
   timespec_to_proto(&a, &tv_sec_hi, &tv_sec_lo, &tv_nsec);
   EXPECT_EQ((uint64_t)a.tv_sec >> 32, tv_sec_hi);
   EXPECT_EQ(0x70005678, tv_sec_lo);
   EXPECT_EQ(1, tv_nsec);
}

TEST(timespec_test, millihz_to_nsec)
{
   EXPECT_EQ(millihz_to_nsec(60000), 16666666);
}

TEST(timespec_test, time_t_max)
{
   /* The TIME_T_MAX macro assumes it's no more than 64 bits */
   EXPECT_LE(sizeof(time_t), sizeof(uint64_t));

   time_t t = TIME_T_MAX;
   EXPECT_EQ((uint64_t)t, (uint64_t)TIME_T_MAX);

   /* Since the tests are C++ code, we have std::numeric_limits */
   EXPECT_EQ(std::numeric_limits<time_t>::max(), TIME_T_MAX);
}

TEST(timespec_test, timespec_add_nsec)
{
   struct timespec a, r;

   a.tv_sec = 0;
   a.tv_nsec = NSEC_PER_SEC - 1;
   EXPECT_FALSE(timespec_add_nsec(&r, &a, 1));
   EXPECT_EQ(1, r.tv_sec);
   EXPECT_EQ(0, r.tv_nsec);

   EXPECT_FALSE(timespec_add_nsec(&r, &a, 2));
   EXPECT_EQ(1, r.tv_sec);
   EXPECT_EQ(1, r.tv_nsec);

   EXPECT_FALSE(timespec_add_nsec(&r, &a, (NSEC_PER_SEC * 2ULL)));
   EXPECT_EQ(2, r.tv_sec);
   EXPECT_EQ(NSEC_PER_SEC - 1, r.tv_nsec);

   EXPECT_FALSE(timespec_add_nsec(&r, &a, (NSEC_PER_SEC * 2ULL) + 2));
   EXPECT_EQ(r.tv_sec, 3);
   EXPECT_EQ(r.tv_nsec, 1);

   r.tv_sec = 4;
   r.tv_nsec = 0;
   EXPECT_FALSE(timespec_add_nsec(&r, &r, NSEC_PER_SEC + 10ULL));
   EXPECT_EQ(5, r.tv_sec);
   EXPECT_EQ(10, r.tv_nsec);

   EXPECT_FALSE(timespec_add_nsec(&r, &r, (NSEC_PER_SEC * 3ULL) - 9ULL));
   EXPECT_EQ(8, r.tv_sec);
   EXPECT_EQ(1, r.tv_nsec);

   EXPECT_FALSE(timespec_add_nsec(&r, &r, (NSEC_PER_SEC * 7ULL) +
                                          (NSEC_PER_SEC - 1ULL)));
   EXPECT_EQ(16, r.tv_sec);
   EXPECT_EQ(0, r.tv_nsec);

   a.tv_sec = TIME_T_MAX;
   a.tv_nsec = 0;
   EXPECT_TRUE(timespec_add_nsec(&r, &a, UINT64_MAX));

   a.tv_sec = TIME_T_MAX;
   a.tv_nsec = 0;
   EXPECT_TRUE(timespec_add_nsec(&r, &a, NSEC_PER_SEC));

   a.tv_sec = TIME_T_MAX;
   a.tv_nsec = NSEC_PER_SEC / 2;
   EXPECT_TRUE(timespec_add_nsec(&r, &a, NSEC_PER_SEC / 2));
}

TEST(timespec_test, timespec_add_msec)
{
   struct timespec a, r;

   a.tv_sec = 1000;
   a.tv_nsec = 1;
   timespec_add_msec(&r, &a, 2002);
   EXPECT_EQ(1002, r.tv_sec);
   EXPECT_EQ(2000001, r.tv_nsec);
}

TEST(timespec_test, timespec_sub_to_nsec)
{
   struct timespec a, b;

   a.tv_sec = 1000;
   a.tv_nsec = 1;
   b.tv_sec = 1;
   b.tv_nsec = 2;
   EXPECT_EQ((999LL * NSEC_PER_SEC) - 1, timespec_sub_to_nsec(&a, &b));
}

TEST(timespec_test, timespec_sub_to_msec)
{
   struct timespec a, b;

   a.tv_sec = 1000;
   a.tv_nsec = 2000000L;
   b.tv_sec = 2;
   b.tv_nsec = 1000000L;
   EXPECT_EQ((998 * 1000) + 1, timespec_sub_to_msec(&a, &b));
}

TEST(timespec_test, timespec_from_nsec)
{
   struct timespec a;

   timespec_from_nsec(&a, 0);
   EXPECT_EQ(0, a.tv_sec);
   EXPECT_EQ(0, a.tv_nsec);

   timespec_from_nsec(&a, NSEC_PER_SEC - 1);
   EXPECT_EQ(0, a.tv_sec);
   EXPECT_EQ(NSEC_PER_SEC - 1, a.tv_nsec);

   timespec_from_nsec(&a, NSEC_PER_SEC);
   EXPECT_EQ(1, a.tv_sec);
   EXPECT_EQ(0, a.tv_nsec);

   timespec_from_nsec(&a, (5LL * NSEC_PER_SEC) + 1);
   EXPECT_EQ(5, a.tv_sec);
   EXPECT_EQ(1, a.tv_nsec);

   timespec_from_nsec(&a, UINT64_MAX);
   EXPECT_EQ(a.tv_nsec, UINT64_MAX % NSEC_PER_SEC);
   EXPECT_EQ(a.tv_sec, (time_t)(UINT64_MAX / NSEC_PER_SEC));
}

TEST(timespec_test, timespec_from_usec)
{
   struct timespec a;

   timespec_from_usec(&a, 0);
   EXPECT_EQ(0, a.tv_sec);
   EXPECT_EQ(0, a.tv_nsec);

   timespec_from_usec(&a, 999999);
   EXPECT_EQ(0, a.tv_sec);
   EXPECT_EQ(999999 * 1000, a.tv_nsec);

   timespec_from_usec(&a, 1000000);
   EXPECT_EQ(1, a.tv_sec);
   EXPECT_EQ(0, a.tv_nsec);

   timespec_from_usec(&a, 5000001);
   EXPECT_EQ(5, a.tv_sec);
   EXPECT_EQ(1000, a.tv_nsec);
}

TEST(timespec_test, timespec_from_msec)
{
   struct timespec a;

   timespec_from_msec(&a, 0);
   EXPECT_EQ(0, a.tv_sec);
   EXPECT_EQ(0, a.tv_nsec);

   timespec_from_msec(&a, 999);
   EXPECT_EQ(0, a.tv_sec);
   EXPECT_EQ(999 * 1000000, a.tv_nsec);

   timespec_from_msec(&a, 1000);
   EXPECT_EQ(1, a.tv_sec);
   EXPECT_EQ(0, a.tv_nsec);

   timespec_from_msec(&a, 5001);
   EXPECT_EQ(5, a.tv_sec);
   EXPECT_EQ(1000000, a.tv_nsec);
}

TEST(timespec_test, timespec_from_proto)
{
   struct timespec a;

   timespec_from_proto(&a, 0, 0, 0);
   EXPECT_EQ(0, a.tv_sec);
   EXPECT_EQ(0, a.tv_nsec);

   timespec_from_proto(&a, 0, 1234, 9999);
   EXPECT_EQ(1234, a.tv_sec);
   EXPECT_EQ(9999, a.tv_nsec);

   timespec_from_proto(&a, 0x1234, 0x5678, 1);
   EXPECT_EQ((time_t)0x0000123400005678LL, a.tv_sec);
   EXPECT_EQ(1, a.tv_nsec);
}

TEST(timespec_test, timespec_is_zero)
{
   struct timespec zero = { 0 };
   struct timespec non_zero_sec = { 1, 0 };
   struct timespec non_zero_nsec = { 0, 1 };

   EXPECT_TRUE(timespec_is_zero(&zero));
   EXPECT_FALSE(timespec_is_zero(&non_zero_nsec));
   EXPECT_FALSE(timespec_is_zero(&non_zero_sec));
}

TEST(timespec_test, timespec_eq)
{
   struct timespec a = { 2, 1 };
   struct timespec b = { -1, 2 };

   EXPECT_TRUE(timespec_eq(&a, &a));
   EXPECT_TRUE(timespec_eq(&b, &b));

   EXPECT_FALSE(timespec_eq(&a, &b));
   EXPECT_FALSE(timespec_eq(&b, &a));
}

TEST(timespec_test, timespec_get)
{
   struct timespec a;
   struct timespec b;
   time_t t;
   timespec_get(&a, TIME_UTC);
   time(&t);
   timespec_get(&b, TIME_UTC);
   /* `t + 1` and `t - 1` are used intentionally for avoid flakes */
   EXPECT_LE(a.tv_sec, t + 1);
   EXPECT_LE(t - 1, b.tv_sec);
}
