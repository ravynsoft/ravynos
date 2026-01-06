/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcdriver/Options.h>

using xcdriver::Options;

TEST(Options, UnknownOption)
{
    Options unknown;
    auto result1 = libutil::Options::Parse<Options>(&unknown, { "-help", "-unknown" });
    EXPECT_FALSE(result1.first);

    Options equals;
    auto result2 = libutil::Options::Parse<Options>(&equals, { "-nota=buildsetting" });
    EXPECT_FALSE(result2.first);

    Options setting;
    auto result3 = libutil::Options::Parse<Options>(&setting, { "yesa=buildsetting" });
    EXPECT_TRUE(result3.first);
    EXPECT_EQ(setting.settings().size(), 1);
}

TEST(Options, CaseSensitivity)
{
    Options valid;
    auto result1 = libutil::Options::Parse<Options>(&valid, { "-showBuildSettings" });
    EXPECT_TRUE(result1.first);
    EXPECT_TRUE(valid.showBuildSettings());

    Options invalid;
    auto result2 = libutil::Options::Parse<Options>(&invalid, { "-showbuildsettings" });
    EXPECT_FALSE(result2.first);
}
