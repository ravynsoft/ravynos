/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <pbxsetting/Condition.h>

using pbxsetting::Condition;

TEST(Condition, Create)
{
    Condition cond = Condition(std::unordered_map<std::string, std::string>({ { "arch", "*" } }));
    ASSERT_EQ(cond.values().size(), 1);
    ASSERT_NE(cond.values().find("arch"), cond.values().end());
    EXPECT_EQ(cond.values().find("arch")->second, "*");

    Condition multiple = Condition(std::unordered_map<std::string, std::string>({{ "arch", "*" }, { "sdk", "*" }}));
    ASSERT_EQ(multiple.values().size(), 2);
    ASSERT_NE(multiple.values().find("arch"), multiple.values().end());
    EXPECT_EQ(multiple.values().find("arch")->second, "*");
    ASSERT_NE(multiple.values().find("sdk"), multiple.values().end());
    EXPECT_EQ(multiple.values().find("sdk")->second, "*");
}

TEST(Condition, MatchSingle)
{
    Condition arch_armv7 = Condition(std::unordered_map<std::string, std::string>({ { "arch", "armv7" } }));
    Condition arch_i386 = Condition(std::unordered_map<std::string, std::string>({ { "arch", "i386" } }));
    EXPECT_TRUE(arch_i386.match(arch_i386));
    EXPECT_TRUE(arch_armv7.match(arch_armv7));
    EXPECT_FALSE(arch_armv7.match(arch_i386));
    EXPECT_FALSE(arch_i386.match(arch_armv7));

    Condition arch_any = Condition(std::unordered_map<std::string, std::string>({ { "arch", "*" } }));
    EXPECT_TRUE(arch_any.match(arch_armv7));
    EXPECT_TRUE(arch_any.match(arch_i386));

    Condition arch_some1 = Condition(std::unordered_map<std::string, std::string>({ { "arch", "arm*" } }));
    Condition arch_some2 = Condition(std::unordered_map<std::string, std::string>({ { "arch", "*m*" } }));
    Condition arch_some3 = Condition(std::unordered_map<std::string, std::string>({ { "arch", "arm*" } }));
    Condition arch_some4 = Condition(std::unordered_map<std::string, std::string>({ { "arch", "arm*" } }));
    EXPECT_TRUE(arch_some1.match(arch_armv7));
    EXPECT_TRUE(arch_some2.match(arch_armv7));
    EXPECT_TRUE(arch_some3.match(arch_armv7));
    EXPECT_TRUE(arch_some4.match(arch_armv7));
    EXPECT_FALSE(arch_some1.match(arch_i386));
    EXPECT_FALSE(arch_some2.match(arch_i386));
    EXPECT_FALSE(arch_some3.match(arch_i386));
    EXPECT_FALSE(arch_some4.match(arch_i386));
}

TEST(Condition, MatchMultiple)
{
    Condition arch = Condition(std::unordered_map<std::string, std::string>({ { "arch", "armv7" } }));
    Condition arch_sdk = Condition(std::unordered_map<std::string, std::string>({ { "arch", "armv7" }, { "sdk", "macosx" } }));
    EXPECT_TRUE(arch.match(arch_sdk));
    EXPECT_FALSE(arch_sdk.match(arch));
}

