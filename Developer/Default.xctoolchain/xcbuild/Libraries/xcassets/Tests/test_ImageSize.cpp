/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcassets/Slot/ImageSize.h>

using xcassets::Slot::ImageSize;

TEST(ImageSize, Parse)
{
    /* Invalid image sizes. */
    EXPECT_EQ(ImageSize::Parse(""), ext::nullopt);
    EXPECT_EQ(ImageSize::Parse("1"), ext::nullopt);
    EXPECT_EQ(ImageSize::Parse("1a"), ext::nullopt);
    EXPECT_EQ(ImageSize::Parse("1a1"), ext::nullopt);
    EXPECT_EQ(ImageSize::Parse("x1"), ext::nullopt);
    EXPECT_EQ(ImageSize::Parse("1x"), ext::nullopt);
    EXPECT_EQ(ImageSize::Parse("1xA"), ext::nullopt);
    EXPECT_EQ(ImageSize::Parse("AxB"), ext::nullopt);

    /* Valid image sizes. */
    auto size1 = ImageSize::Parse("1x1");
    ASSERT_NE(size1, ext::nullopt);
    EXPECT_EQ(size1->width(), 1.0);
    EXPECT_EQ(size1->height(), 1.0);

    auto size2 = ImageSize::Parse("72x320");
    ASSERT_NE(size2, ext::nullopt);
    EXPECT_EQ(size2->width(), 72.0);
    EXPECT_EQ(size2->height(), 320.0);

    auto size3 = ImageSize::Parse("0.5x89.32");
    ASSERT_NE(size3, ext::nullopt);
    EXPECT_EQ(size3->width(), 0.5);
    EXPECT_EQ(size3->height(), 89.32);
}

