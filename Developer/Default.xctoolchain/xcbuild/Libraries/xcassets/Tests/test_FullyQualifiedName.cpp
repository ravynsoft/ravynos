/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcassets/FullyQualifiedName.h>

using xcassets::FullyQualifiedName;

TEST(FullyQualifiedName, Parse)
{
    auto name1 = FullyQualifiedName::Parse("");
    EXPECT_EQ(name1.name(), "");
    EXPECT_EQ(name1.groups(), std::vector<std::string>());

    auto name2 = FullyQualifiedName::Parse("name");
    EXPECT_EQ(name2.name(), "name");
    EXPECT_EQ(name2.groups(), std::vector<std::string>());

    auto name3 = FullyQualifiedName::Parse("group1/group2/name");
    EXPECT_EQ(name3.name(), "name");
    EXPECT_EQ(name3.groups(), std::vector<std::string>({ "group1", "group2" }));
    EXPECT_EQ(name3.string(), "group1/group2/name");

    auto name4 = FullyQualifiedName::Parse("//group//name");
    EXPECT_EQ(name4.name(), "name");
    EXPECT_EQ(name4.groups(), std::vector<std::string>({ "group" }));
    EXPECT_EQ(name4.string(), "group/name");
}

