/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcassets/Slot/SystemVersion.h>

using xcassets::Slot::SystemVersion;

TEST(SystemVersion, Parse)
{
    /* Invalid system versions. */
    EXPECT_EQ(SystemVersion::Parse(""), ext::nullopt);
    EXPECT_EQ(SystemVersion::Parse("1"), ext::nullopt);
    EXPECT_EQ(SystemVersion::Parse("1x"), ext::nullopt);
    EXPECT_EQ(SystemVersion::Parse("1x.0"), ext::nullopt);
    EXPECT_EQ(SystemVersion::Parse("1.0x"), ext::nullopt);
    EXPECT_EQ(SystemVersion::Parse("1."), ext::nullopt);
    EXPECT_EQ(SystemVersion::Parse(".1.0"), ext::nullopt);
    EXPECT_EQ(SystemVersion::Parse("1.0."), ext::nullopt);
    EXPECT_EQ(SystemVersion::Parse("1..0"), ext::nullopt);
    EXPECT_EQ(SystemVersion::Parse("1.0.0.0"), ext::nullopt);

    /* Valid system versions. */
    auto version1 = SystemVersion::Parse("1.0");
    ASSERT_NE(version1, ext::nullopt);
    EXPECT_EQ(version1->major(), 1);
    EXPECT_EQ(version1->minor(), 0);
    EXPECT_EQ(version1->patch(), ext::nullopt);

    auto version2 = SystemVersion::Parse("3.1.2");
    ASSERT_NE(version2, ext::nullopt);
    EXPECT_EQ(version2->major(), 3);
    EXPECT_EQ(version2->minor(), 1);
    EXPECT_EQ(version2->patch(), 2);

    auto version3 = SystemVersion::Parse("10.4.11");
    ASSERT_NE(version3, ext::nullopt);
    EXPECT_EQ(version3->major(), 10);
    EXPECT_EQ(version3->minor(), 4);
    EXPECT_EQ(version3->patch(), 11);
}

