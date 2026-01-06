/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <pbxbuild/DerivedDataHash.h>

using pbxbuild::DerivedDataHash;

TEST(DerivedDataHash, Project)
{
    EXPECT_EQ(DerivedDataHash::Create("/tmp/test.xcodeproj").derivedDataHash(), "test-gebdbcuasvwschgbtxnniurxjukt");
    EXPECT_EQ(DerivedDataHash::Create("/var/tmp/test.xcodeproj").derivedDataHash(), "test-bcocmrrfgjqtchaacscrvidwhtxm");
}

TEST(DerivedDataHash, Workspace)
{
    EXPECT_EQ(DerivedDataHash::Create("/tmp/test.xcworkspace").derivedDataHash(), "test-gwbccjllutzxldarzgubidlxvvrx");
    EXPECT_EQ(DerivedDataHash::Create("/var/tmp/test.xcworkspace").derivedDataHash(), "test-fmtsuzukhpemgpcoyvsqwhcyyesj");
}

TEST(DerivedDataHash, Extension)
{
    EXPECT_EQ(DerivedDataHash::Create("/tmp/test.extension.xcodeproj").derivedDataHash(), "test.extension-gxmolhxorxkzjraqwuwsazdcensd");
}

