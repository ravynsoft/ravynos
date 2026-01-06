/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <ninja/Value.h>

using ninja::Value;

TEST(Value, EscapeValue)
{
    Value::EscapeMode mode = Value::EscapeMode::Value;

    EXPECT_EQ(Value::String("").resolve(mode), "");
    EXPECT_EQ(Value::String("$").resolve(mode), "$$");
    EXPECT_EQ(Value::String("strings").resolve(mode), "strings");
    EXPECT_EQ(Value::String("stri ngs").resolve(mode), "stri ngs");
    EXPECT_EQ(Value::String("$tri:ngs").resolve(mode), "$$tri:ngs");
    EXPECT_EQ(Value::String("$tring$").resolve(mode), "$$tring$$");
    EXPECT_EQ(Value::String("one $string").resolve(mode), "one $$string");
    EXPECT_EQ(Value::String("one$ two").resolve(mode), "one$$ two");
    EXPECT_EQ(Value::String("two $$").resolve(mode), "two $$$$");

    EXPECT_EQ(Value::Expression("").resolve(mode), "");
    EXPECT_EQ(Value::Expression("$").resolve(mode), "$");
    EXPECT_EQ(Value::Expression("strings").resolve(mode), "strings");
    EXPECT_EQ(Value::Expression("stri ngs").resolve(mode), "stri ngs");
    EXPECT_EQ(Value::Expression("$tri:ngs").resolve(mode), "$tri:ngs");
    EXPECT_EQ(Value::Expression("$tring$").resolve(mode), "$tring$");
    EXPECT_EQ(Value::Expression("one $string").resolve(mode), "one $string");
    EXPECT_EQ(Value::Expression("one$ two").resolve(mode), "one$ two");
    EXPECT_EQ(Value::Expression("two $$").resolve(mode), "two $$");
}

TEST(Value, EscapePathList)
{
    Value::EscapeMode mode = Value::EscapeMode::PathList;

    EXPECT_EQ(Value::String("").resolve(mode), "");
    EXPECT_EQ(Value::String("$").resolve(mode), "$$");
    EXPECT_EQ(Value::String("strings").resolve(mode), "strings");
    EXPECT_EQ(Value::String("stri ngs").resolve(mode), "stri$ ngs");
    EXPECT_EQ(Value::String("$tri:ngs").resolve(mode), "$$tri:ngs");
    EXPECT_EQ(Value::String("$tring$").resolve(mode), "$$tring$$");
    EXPECT_EQ(Value::String("one $string").resolve(mode), "one$ $$string");
    EXPECT_EQ(Value::String("one$ two").resolve(mode), "one$$$ two");
    EXPECT_EQ(Value::String("two $$").resolve(mode), "two$ $$$$");

    EXPECT_EQ(Value::Expression("").resolve(mode), "");
    EXPECT_EQ(Value::Expression("$").resolve(mode), "$");
    EXPECT_EQ(Value::Expression("strings").resolve(mode), "strings");
    EXPECT_EQ(Value::Expression("stri ngs").resolve(mode), "stri$ ngs");
    EXPECT_EQ(Value::Expression("$tri:ngs").resolve(mode), "$tri:ngs");
    EXPECT_EQ(Value::Expression("$tring$").resolve(mode), "$tring$");
    EXPECT_EQ(Value::Expression("one $string").resolve(mode), "one$ $string");
    EXPECT_EQ(Value::Expression("one$ two").resolve(mode), "one$$$ two");
    EXPECT_EQ(Value::Expression("two $$").resolve(mode), "two$ $$");
}

TEST(Value, EscapeBuildPathList)
{
    Value::EscapeMode mode = Value::EscapeMode::BuildPathList;

    EXPECT_EQ(Value::String("").resolve(mode), "");
    EXPECT_EQ(Value::String("$").resolve(mode), "$$");
    EXPECT_EQ(Value::String("strings").resolve(mode), "strings");
    EXPECT_EQ(Value::String("stri ngs").resolve(mode), "stri$ ngs");
    EXPECT_EQ(Value::String("$tri:ngs").resolve(mode), "$$tri$:ngs");
    EXPECT_EQ(Value::String("$tring$").resolve(mode), "$$tring$$");
    EXPECT_EQ(Value::String("one $string").resolve(mode), "one$ $$string");
    EXPECT_EQ(Value::String("one$ two").resolve(mode), "one$$$ two");
    EXPECT_EQ(Value::String("two $$").resolve(mode), "two$ $$$$");

    EXPECT_EQ(Value::Expression("").resolve(mode), "");
    EXPECT_EQ(Value::Expression("$").resolve(mode), "$");
    EXPECT_EQ(Value::Expression("strings").resolve(mode), "strings");
    EXPECT_EQ(Value::Expression("stri ngs").resolve(mode), "stri$ ngs");
    EXPECT_EQ(Value::Expression("$tri:ngs").resolve(mode), "$tri$:ngs");
    EXPECT_EQ(Value::Expression("$tring$").resolve(mode), "$tring$");
    EXPECT_EQ(Value::Expression("one $string").resolve(mode), "one$ $string");
    EXPECT_EQ(Value::Expression("one$ two").resolve(mode), "one$$$ two");
    EXPECT_EQ(Value::Expression("two $$").resolve(mode), "two$ $$");
}

TEST(Value, String)
{
    Value::EscapeMode mode = Value::EscapeMode::Value;
    EXPECT_EQ(Value::String("").resolve(mode), "");
    EXPECT_EQ(Value::String("string").resolve(mode), "string");
    EXPECT_EQ(Value::String("$").resolve(mode), "$$");
    EXPECT_EQ(Value::String("path/$variable").resolve(mode), "path/$$variable");
}

TEST(Value, Expression)
{
    Value::EscapeMode mode = Value::EscapeMode::Value;
    EXPECT_EQ(Value::Expression("").resolve(mode), "");
    EXPECT_EQ(Value::Expression("string").resolve(mode), "string");
    EXPECT_EQ(Value::Expression("$").resolve(mode), "$");
    EXPECT_EQ(Value::Expression("path/$variable").resolve(mode), "path/$variable");
}

TEST(Value, Equal)
{
    EXPECT_EQ(Value::String(""), Value::Expression(""));
    EXPECT_NE(Value::String("$hello"), Value::Expression("$$hello"));
}

TEST(Wildcard, Concatenate)
{
    Value::EscapeMode mode = Value::EscapeMode::Value;
    EXPECT_EQ((Value::String("$") + Value::Expression("variable")).resolve(mode), "$$variable");
    EXPECT_EQ((Value::String("$variable") + Value::Expression("$variable")).resolve(mode), "$$variable$variable");
    EXPECT_EQ((Value::Expression("$variable") + Value::String("two")).resolve(mode), "$variabletwo");
}
