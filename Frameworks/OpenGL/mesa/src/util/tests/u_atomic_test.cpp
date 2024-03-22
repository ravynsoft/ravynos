/**************************************************************************
 *
 * Copyright 2014 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <gtest/gtest.h>
#include <stdint.h>
#include <inttypes.h>

#include "u_atomic.h"

#ifdef _MSC_VER
#pragma warning( disable : 28112 ) /* Accessing a local variable via an Interlocked function */
#pragma warning( disable : 28113 ) /* A variable which is accessed via an Interlocked function must always be accessed via an Interlocked function */
#endif

template <typename T> class AtomicAssignment : public testing::Test {};

using AtomicAssignmentTypes =
   testing::Types<int, unsigned, bool,
                  int8_t, uint8_t, int16_t, uint16_t,
                  int32_t, uint32_t, int64_t, uint64_t>;

TYPED_TEST_SUITE(AtomicAssignment, AtomicAssignmentTypes);

TYPED_TEST(AtomicAssignment, Test)
{
   TypeParam v, r;
   const TypeParam ones = TypeParam(-1);

   p_atomic_set(&v, ones);
   ASSERT_EQ(v, ones) << "p_atomic_set";

   r = p_atomic_read(&v);
   ASSERT_EQ(r, ones) << "p_atomic_read";

   r = p_atomic_read_relaxed(&v);
   ASSERT_EQ(r, ones) << "p_atomic_read_relaxed";

   v = ones;
   r = p_atomic_cmpxchg(&v, 0, 1);
   ASSERT_EQ(v, ones) << "p_atomic_cmpxchg";
   ASSERT_EQ(r, ones) << "p_atomic_cmpxchg";

   r = p_atomic_cmpxchg(&v, ones, 0);
   ASSERT_EQ(v, 0) << "p_atomic_cmpxchg";
   ASSERT_EQ(r, ones) << "p_atomic_cmpxchg";

   v = 0;
   r = p_atomic_xchg(&v, ones);
   ASSERT_EQ(v, ones) << "p_atomic_xchg";
   ASSERT_EQ(r, 0) << "p_atomic_xchg";
}


template <typename T> class AtomicIncrementDecrement : public testing::Test {};

using AtomicIncrementDecrementTypes =
   testing::Types<int, unsigned,
                  int8_t, uint8_t,
                  int16_t, uint16_t,
                  int32_t, uint32_t, int64_t, uint64_t>;

TYPED_TEST_SUITE(AtomicIncrementDecrement, AtomicIncrementDecrementTypes);

TYPED_TEST(AtomicIncrementDecrement, Test)
{
   TypeParam v, r;
   bool b;

   const TypeParam ones = TypeParam(-1);

   v = 2;

   b = p_atomic_dec_zero(&v);
   ASSERT_EQ(v, 1) << "p_atomic_dec_zero";
   ASSERT_FALSE(b) << "p_atomic_dec_zero";

   b = p_atomic_dec_zero(&v);
   ASSERT_EQ(v, 0) << "p_atomic_dec_zero";
   ASSERT_TRUE(b)  << "p_atomic_dec_zero";

   b = p_atomic_dec_zero(&v);
   ASSERT_EQ(v, ones) << "p_atomic_dec_zero";
   ASSERT_FALSE(b) << "p_atomic_dec_zero";

   v = ones;
   p_atomic_inc(&v);
   ASSERT_EQ(v, 0) << "p_atomic_inc";

   v = ones;
   r = p_atomic_inc_return(&v);
   ASSERT_EQ(v, 0) << "p_atomic_inc_return";
   ASSERT_EQ(r, v) << "p_atomic_inc_return";

   v = 0;
   p_atomic_dec(&v);
   ASSERT_EQ(v, ones) << "p_atomic_dec";

   v = 0;
   r = p_atomic_dec_return(&v);
   ASSERT_EQ(v, ones) << "p_atomic_dec_return";
   ASSERT_EQ(v, r) << "p_atomic_dec_return";

   v = 0;
   r = p_atomic_add_return(&v, -1);
   ASSERT_EQ(v, ones) << "p_atomic_add_return";
   ASSERT_EQ(v, r) << "p_atomic_add_return";

   v = 0;
   r = p_atomic_fetch_add(&v, -1);
   ASSERT_EQ(v, ones) << "p_atomic_fetch_add";
   ASSERT_EQ(r, 0) << "p_atomic_fetch_add";
}

template <typename T> class AtomicAdd : public testing::Test {};

using AtomicAddTypes =
   testing::Types<int, unsigned,
                  int8_t, uint8_t, int16_t, uint16_t,
                  int32_t, uint32_t, int64_t, uint64_t>;

TYPED_TEST_SUITE(AtomicAdd, AtomicAddTypes);

TYPED_TEST(AtomicAdd, Test)
{
   TypeParam v, r;

   v = 23;

   p_atomic_add(&v, 42);
   r = p_atomic_read(&v);

   ASSERT_EQ(r, 65) << "p_atomic_add";
}
