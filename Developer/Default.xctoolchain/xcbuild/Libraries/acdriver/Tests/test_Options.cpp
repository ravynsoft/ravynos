/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <acdriver/Options.h>

using acdriver::Options;

TEST(Options, InvalidDocumentedOptions)
{
    Options unknown;
    auto result1 = libutil::Options::Parse<Options>(&unknown, { "--write", "path" });
    EXPECT_FALSE(result1.first);
}

TEST(Options, OnDemandResourcesFlag)
{
    Options invalid;
    auto result1 = libutil::Options::Parse<Options>(&invalid, { "--enable-on-demand-resources" });
    EXPECT_FALSE(result1.first);

    Options yes;
    auto result2 = libutil::Options::Parse<Options>(&yes, { "--enable-on-demand-resources", "YES" });
    EXPECT_TRUE(result2.first);
    EXPECT_TRUE(yes.enableOnDemandResources());

    Options no;
    auto result3 = libutil::Options::Parse<Options>(&no, { "--enable-on-demand-resources", "NO" });
    EXPECT_TRUE(result3.first);
    EXPECT_FALSE(no.enableOnDemandResources());
}

