/*
 * Copyright 2021-2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_test.h"

#include <gtest/gtest.h>

TEST(Minifloat, RepresentativeValues)
{
   EXPECT_EQ(agx_minifloat_decode(0), 0.0f);
   EXPECT_EQ(agx_minifloat_decode(25), 0.390625f);
   EXPECT_EQ(agx_minifloat_decode(135), -0.109375f);
   EXPECT_EQ(agx_minifloat_decode(255), -31.0);
}

TEST(Minifloat, Exactness)
{
   EXPECT_TRUE(agx_minifloat_exact(0.0f));
   EXPECT_TRUE(agx_minifloat_exact(0.390625f));
   EXPECT_TRUE(agx_minifloat_exact(-0.109375f));
   EXPECT_TRUE(agx_minifloat_exact(-31.0));
   EXPECT_FALSE(agx_minifloat_exact(3.141f));
   EXPECT_FALSE(agx_minifloat_exact(2.718f));
   EXPECT_FALSE(agx_minifloat_exact(1.618f));
}

TEST(Minifloat, AllValuesRoundtrip)
{
   for (unsigned i = 0; i < 0x100; ++i) {
      float f = agx_minifloat_decode(i);
      EXPECT_EQ(agx_minifloat_encode(f), i);
      EXPECT_TRUE(agx_minifloat_exact(f));
   }
}
