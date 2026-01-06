/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <dependency/BinaryDependencyInfo.h>

using dependency::BinaryDependencyInfo;
using dependency::DependencyInfo;

TEST(BinaryDependencyInfo, Empty)
{
    auto info = BinaryDependencyInfo::Deserialize(std::vector<uint8_t>());
    ASSERT_TRUE(info);
    EXPECT_TRUE(info->version().empty());
    EXPECT_TRUE(info->missing().empty());
    EXPECT_TRUE(info->dependencyInfo().inputs().empty());
    EXPECT_TRUE(info->dependencyInfo().outputs().empty());
}

TEST(BinaryDependencyInfo, Malformed)
{
    /* Unknown command. */
    auto info1 = BinaryDependencyInfo::Deserialize({ 42, 'v', 0 });
    EXPECT_FALSE(info1);

    /* Missing null terminator. */
    auto info2 = BinaryDependencyInfo::Deserialize({ 0, 'v' });
    EXPECT_FALSE(info2);
}

TEST(BinaryDependencyInfo, Version)
{
    auto info1 = BinaryDependencyInfo::Deserialize({ 0, 'v', 'e', 'r', 's', 'i', 'o', 'n', '\0' });
    ASSERT_TRUE(info1);
    EXPECT_EQ(info1->version(), "version");
    EXPECT_TRUE(info1->missing().empty());
    EXPECT_TRUE(info1->dependencyInfo().inputs().empty());
    EXPECT_TRUE(info1->dependencyInfo().outputs().empty());

    auto info2 = BinaryDependencyInfo::Deserialize({ 0, 'v', '1', '\0', 0, 'v', '2', '\0' });
    EXPECT_FALSE(info2);
}

TEST(BinaryDependencyInfo, Inputs)
{
    auto info1 = BinaryDependencyInfo::Deserialize({ 0x10, 'i', 'n', '\0' });
    ASSERT_TRUE(info1);
    EXPECT_TRUE(info1->version().empty());
    EXPECT_TRUE(info1->missing().empty());
    EXPECT_EQ(info1->dependencyInfo().inputs(), std::vector<std::string>({ "in" }));
    EXPECT_TRUE(info1->dependencyInfo().outputs().empty());

    auto info2 = BinaryDependencyInfo::Deserialize({ 0x10, 'i', 'n', '1', '\0', 0x10, 'i', 'n', '2', '\0' });
    ASSERT_TRUE(info2);
    EXPECT_TRUE(info2->version().empty());
    EXPECT_TRUE(info2->missing().empty());
    EXPECT_EQ(info2->dependencyInfo().inputs(), std::vector<std::string>({ "in1", "in2" }));
    EXPECT_TRUE(info2->dependencyInfo().outputs().empty());
}

TEST(BinaryDependencyInfo, Outputs)
{
    auto info1 = BinaryDependencyInfo::Deserialize({ 0x40, 'o', 'u', 't', '\0' });
    ASSERT_TRUE(info1);
    EXPECT_TRUE(info1->version().empty());
    EXPECT_TRUE(info1->missing().empty());
    EXPECT_TRUE(info1->dependencyInfo().inputs().empty());
    EXPECT_EQ(info1->dependencyInfo().outputs(), std::vector<std::string>({ "out" }));

    auto info2 = BinaryDependencyInfo::Deserialize({ 0x40, 'o', 'u', 't', '1', '\0', 0x40, 'o', 'u', 't', '2', '\0' });
    ASSERT_TRUE(info2);
    EXPECT_TRUE(info2->version().empty());
    EXPECT_TRUE(info2->missing().empty());
    EXPECT_TRUE(info2->dependencyInfo().inputs().empty());
    EXPECT_EQ(info2->dependencyInfo().outputs(), std::vector<std::string>({ "out1", "out2" }));
}

TEST(BinaryDependencyInfo, InputsOutputs)
{
    auto info1 = BinaryDependencyInfo::Deserialize({ 0x40, 'o', 'u', 't', '\0', 0x10, 'i', 'n', '\0' });
    ASSERT_TRUE(info1);
    EXPECT_TRUE(info1->version().empty());
    EXPECT_TRUE(info1->missing().empty());
    EXPECT_EQ(info1->dependencyInfo().inputs(), std::vector<std::string>({ "in" }));
    EXPECT_EQ(info1->dependencyInfo().outputs(), std::vector<std::string>({ "out" }));
}

TEST(BinaryDependencyInfo, SerializeVersion)
{
    BinaryDependencyInfo info;
    info.version() = "version";

    EXPECT_EQ(info.serialize(), std::vector<uint8_t>({ 0, 'v', 'e', 'r', 's', 'i', 'o', 'n', '\0' }));
}

TEST(BinaryDependencyInfo, SerializeInputsOutputs)
{
    BinaryDependencyInfo info;
    info.dependencyInfo().inputs() = { "in" };
    info.dependencyInfo().outputs() = { "out" };

    EXPECT_EQ(info.serialize(), std::vector<uint8_t>({ 0x40, 'o', 'u', 't', '\0', 0x10, 'i', 'n', '\0' }));
}
