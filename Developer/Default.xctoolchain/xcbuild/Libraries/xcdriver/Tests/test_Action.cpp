/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcdriver/Action.h>
#include <xcdriver/Options.h>

using xcdriver::Action;
using xcdriver::Options;

TEST(Action, Empty)
{
    Options options;
    auto result = libutil::Options::Parse<Options>(&options, { });
    ASSERT_TRUE(result.first);

    EXPECT_EQ(Action::Determine(options), Action::Build);
}

TEST(Action, VersionOverrides)
{
    Options options;
    auto result = libutil::Options::Parse<Options>(&options, { "-help", "-usage", "-version", "-license" });
    ASSERT_TRUE(result.first);

    EXPECT_EQ(Action::Determine(options), Action::Version);
}

