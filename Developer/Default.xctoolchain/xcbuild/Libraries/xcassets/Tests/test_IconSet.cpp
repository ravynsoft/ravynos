/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcassets/Asset/IconSet.h>

using xcassets::Asset::IconSet;

TEST(IconSet, Icon)
{
    /* Invalid icon names. */
    EXPECT_EQ(IconSet::Icon::Parse(""), ext::nullopt);
    EXPECT_EQ(IconSet::Icon::Parse("icon_16x16"), ext::nullopt);
    EXPECT_EQ(IconSet::Icon::Parse(".png"), ext::nullopt);
    EXPECT_EQ(IconSet::Icon::Parse("icon_.png"), ext::nullopt);
    EXPECT_EQ(IconSet::Icon::Parse("icon_x.png"), ext::nullopt);
    EXPECT_EQ(IconSet::Icon::Parse("icon_16x.png"), ext::nullopt);
    EXPECT_EQ(IconSet::Icon::Parse("icon_16x16a.png"), ext::nullopt);
    EXPECT_EQ(IconSet::Icon::Parse("icon_16x16@2xa.png"), ext::nullopt);
    EXPECT_EQ(IconSet::Icon::Parse("icon_16x16.png/invalid"), ext::nullopt);

    /* Valid icon name without scale. */
    auto icon1 = IconSet::Icon::Parse("icon_16x16.png");
    ASSERT_NE(icon1, ext::nullopt);
    EXPECT_EQ(icon1->width(), 16);
    EXPECT_EQ(icon1->height(), 16);
    EXPECT_EQ(icon1->scale(), ext::nullopt);

    /* Valid icon name with scale. */
    auto icon2 = IconSet::Icon::Parse("icon_32x32@4x.png");
    ASSERT_NE(icon2, ext::nullopt);
    EXPECT_EQ(icon2->width(), 32);
    EXPECT_EQ(icon2->height(), 32);
    EXPECT_NE(icon2->scale(), ext::nullopt);
    EXPECT_EQ(icon2->scale()->value(), 4.0);
}

