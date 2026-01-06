/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcassets/Slot/Scale.h>

using xcassets::Slot::Scale;

TEST(Scale, Parse)
{
    /* Invalid scales. */
    EXPECT_EQ(Scale::Parse(""), ext::nullopt);
    EXPECT_EQ(Scale::Parse("1"), ext::nullopt);
    EXPECT_EQ(Scale::Parse("1a"), ext::nullopt);
    EXPECT_EQ(Scale::Parse("x1"), ext::nullopt);
    EXPECT_EQ(Scale::Parse("1xx"), ext::nullopt);

    /* Valid scales. */
    auto scale1 = Scale::Parse("1x");
    ASSERT_NE(scale1, ext::nullopt);
    EXPECT_EQ(scale1->value(), 1.0);

    auto scale2 = Scale::Parse("4x");
    ASSERT_NE(scale2, ext::nullopt);
    EXPECT_EQ(scale2->value(), 4.0);

    auto scale3 = Scale::Parse("0.5x");
    ASSERT_NE(scale3, ext::nullopt);
    EXPECT_EQ(scale3->value(), 0.5);
}

