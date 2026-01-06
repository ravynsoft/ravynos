/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <plist/Objects.h>

using plist::Real;
using plist::Integer;
using plist::String;

TEST(Real, Coerce)
{
    auto s1 = String::New("");
    auto r1 = Real::Coerce(s1.get());
    EXPECT_EQ(r1, nullptr);

    auto s2 = String::New("one");
    auto r2 = Real::Coerce(s2.get());
    EXPECT_EQ(r2, nullptr);

    auto s3 = String::New("1");
    auto r3 = Real::Coerce(s3.get());
    EXPECT_EQ(r3->value(), 1.0);

    auto s4 = String::New("1.0");
    auto r4 = Real::Coerce(s4.get());
    EXPECT_EQ(r4->value(), 1.0);

    auto s5 = Integer::New(1);
    auto r5 = Real::Coerce(s5.get());
    EXPECT_EQ(r5->value(), 1.0);
}

