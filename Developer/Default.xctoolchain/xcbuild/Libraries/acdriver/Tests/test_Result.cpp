/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <acdriver/Result.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using acdriver::Result;

TEST(Result, Success)
{
    Result result;
    EXPECT_TRUE(result.success());

    result.normal(Result::Severity::Notice, "notice");
    EXPECT_TRUE(result.success());

    result.normal(Result::Severity::Warning, "warning");
    EXPECT_TRUE(result.success());

    result.normal(Result::Severity::Error, "error");
    EXPECT_FALSE(result.success());
}

TEST(Result, NormalText)
{
    Result result;
    result.normal(Result::Severity::Notice, "message1", std::string("reason1"), std::string("file"));
    result.normal(Result::Severity::Notice, "message2", std::string("reason2"));
    result.normal(Result::Severity::Notice, "message3");

    ext::optional<std::string> text = result.normalText(Result::Severity::Notice);
    ASSERT_TRUE(text);
    EXPECT_EQ(*text, "file: notice: message1\n    Failure Reason: reason1\n: notice: message2\n    Failure Reason: reason2\n: notice: message3\n");
}

TEST(Result, NormalArray)
{
    Result result;
    result.normal(Result::Severity::Warning, "message1", std::string("reason1"), std::string("file"));
    result.normal(Result::Severity::Warning, "message2", std::string("reason2"));
    result.normal(Result::Severity::Warning, "message3");

    std::unique_ptr<plist::Array> array = result.normalArray(Result::Severity::Warning);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array->count(), 3);

    plist::Dictionary const *first = array->value<plist::Dictionary>(0);
    ASSERT_NE(first, nullptr);
    ASSERT_EQ(first->count(), 2);
    ASSERT_EQ(first->value<plist::String>("description")->value(), "message1");
    ASSERT_EQ(first->value<plist::String>("failure-reason")->value(), "reason1");

    plist::Dictionary const *second = array->value<plist::Dictionary>(1);
    ASSERT_NE(second, nullptr);
    ASSERT_EQ(second->count(), 2);
    ASSERT_EQ(second->value<plist::String>("description")->value(), "message2");
    ASSERT_EQ(second->value<plist::String>("failure-reason")->value(), "reason2");

    plist::Dictionary const *third = array->value<plist::Dictionary>(2);
    ASSERT_NE(third, nullptr);
    ASSERT_EQ(third->count(), 1);
    ASSERT_EQ(third->value<plist::String>("description")->value(), "message3");
}
