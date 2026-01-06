/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <libutil/Escape.h>

using libutil::Escape;

TEST(Escape, Shell)
{
    EXPECT_EQ(Escape::Shell(""), "");
    EXPECT_EQ(Escape::Shell("alpha"), "alpha");
    EXPECT_EQ(Escape::Shell("alpha9numeric"), "alpha9numeric");
    EXPECT_EQ(Escape::Shell("dollar$"), "'dollar$'");
    EXPECT_EQ(Escape::Shell("back\\slash"), "'back\\slash'");
    EXPECT_EQ(Escape::Shell("quo\"te\"s"), "'quo\"te\"s'");
    EXPECT_EQ(Escape::Shell("sin'gle"), "'sin'\\''gle'");

    EXPECT_EQ(Escape::Shell("$\\\""), "'$\\\"'");
    EXPECT_EQ(Escape::Shell("'"), "''\\'''");
}

TEST(Escape, Makefile)
{
    EXPECT_EQ(Escape::Makefile(""), "");
    EXPECT_EQ(Escape::Makefile("alpha"), "alpha");
    EXPECT_EQ(Escape::Makefile("alpha9numeric"), "alpha9numeric");
    EXPECT_EQ(Escape::Makefile("dollar$"), "dollar\\$");
    EXPECT_EQ(Escape::Makefile("co:l:on"), "co\\:l\\:on");
    EXPECT_EQ(Escape::Makefile("ha#sh"), "ha\\#sh");
    EXPECT_EQ(Escape::Makefile("per%cent"), "per\\%cent");
    EXPECT_EQ(Escape::Makefile("'\"\\"), "'\"\\");
}
