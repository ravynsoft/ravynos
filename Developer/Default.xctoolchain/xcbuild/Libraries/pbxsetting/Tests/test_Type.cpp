/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <pbxsetting/Type.h>

using pbxsetting::Type;

TEST(Type, ParseBoolean)
{
    EXPECT_EQ(Type::ParseBoolean(""), false);
    EXPECT_EQ(Type::ParseBoolean("random"), false);

    EXPECT_EQ(Type::ParseBoolean("YES"), true);
    EXPECT_EQ(Type::ParseBoolean("yes"), true);
    EXPECT_EQ(Type::ParseBoolean("Yes"), true);
    EXPECT_EQ(Type::ParseBoolean("Y"), false);

    EXPECT_EQ(Type::ParseBoolean("NO"), false);
    EXPECT_EQ(Type::ParseBoolean("no"), false);
    EXPECT_EQ(Type::ParseBoolean("No"), false);
    EXPECT_EQ(Type::ParseBoolean("N"), false);
}

TEST(Type, ParseInteger)
{
    EXPECT_EQ(Type::ParseInteger(""), 0LL);
    EXPECT_EQ(Type::ParseInteger("random"), 0LL);

    EXPECT_EQ(Type::ParseInteger("0"), 0LL);
    EXPECT_EQ(Type::ParseInteger("10"), 10LL);
    EXPECT_EQ(Type::ParseInteger("100"), 100LL);
    EXPECT_EQ(Type::ParseInteger("99"), 99LL);

    EXPECT_EQ(Type::ParseInteger("text 22"), 0LL);
    EXPECT_EQ(Type::ParseInteger("22 text"), 22LL);
    EXPECT_EQ(Type::ParseInteger("text 22 text"), 0LL);
}

TEST(Type, ParseList)
{
    EXPECT_EQ(Type::ParseList("hello"), std::vector<std::string>({ "hello" }));
    EXPECT_EQ(Type::ParseList("hello world"), std::vector<std::string>({ "hello", "world" }));
    EXPECT_EQ(Type::ParseList("  hello  world  "), std::vector<std::string>({ "hello", "world" }));
    EXPECT_EQ(Type::ParseList("hello 'world'"), std::vector<std::string>({ "hello", "world" }));
    EXPECT_EQ(Type::ParseList("'hello world'"), std::vector<std::string>({ "hello world" }));
    EXPECT_EQ(Type::ParseList("\"hello world\""), std::vector<std::string>({ "hello world" }));
    EXPECT_EQ(Type::ParseList("\"hello\" \"world\""), std::vector<std::string>({ "hello", "world" }));
    EXPECT_EQ(Type::ParseList("'hello wo'rld"), std::vector<std::string>({ "hello world" }));
    EXPECT_EQ(Type::ParseList("hell'o wo'rld"), std::vector<std::string>({ "hello world" }));
    EXPECT_EQ(Type::ParseList("'' '' 'test'"), std::vector<std::string>({ "test" }));
    EXPECT_EQ(Type::ParseList("hello=\\\"world\\\""), std::vector<std::string>({ "hello=\"world\"" }));
}

TEST(Type, FormatBoolean)
{
    EXPECT_EQ(Type::FormatBoolean(true), "YES");
    EXPECT_EQ(Type::FormatBoolean(false), "NO");
}

TEST(Type, FormatInteger)
{
    EXPECT_EQ(Type::FormatInteger(0), "0");
    EXPECT_EQ(Type::FormatInteger(1), "1");
    EXPECT_EQ(Type::FormatInteger(-1), "-1");
    EXPECT_EQ(Type::FormatInteger(65536), "65536");
}

TEST(Type, FormatReal)
{
    EXPECT_EQ(Type::FormatReal(0), "0");
    EXPECT_EQ(Type::FormatReal(1), "1");
    EXPECT_EQ(Type::FormatReal(-1), "-1");
    EXPECT_EQ(Type::FormatReal(65536), "65536");

    EXPECT_EQ(Type::FormatReal(0.5), "0.5");
    EXPECT_EQ(Type::FormatReal(3.14), "3.14");
    EXPECT_EQ(Type::FormatReal(-3.14), "-3.14");
    EXPECT_EQ(Type::FormatReal(2.71828), "2.71828");
}

TEST(Type, FormatList)
{
    EXPECT_EQ(Type::FormatList({ }), "");
    EXPECT_EQ(Type::FormatList({ "one" }), "one");
    EXPECT_EQ(Type::FormatList({ "one", "two" }), "one two");

    EXPECT_EQ(Type::FormatList({ "one one", "two" }), "one\\ one two");
    EXPECT_EQ(Type::FormatList({ "'one", "two'" }), "\\'one two\\'");
    EXPECT_EQ(Type::FormatList({ "\"one", "two\"" }), "\\\"one two\\\"");
}
