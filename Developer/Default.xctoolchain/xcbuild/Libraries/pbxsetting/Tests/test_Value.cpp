/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <pbxsetting/Value.h>

using pbxsetting::Value;

TEST(Value, Simple)
{
    Value explicit_empty1 = Value::Empty();
    ASSERT_EQ(explicit_empty1.entries().size(), 0);

    Value explicit_empty2 = Value::String("");
    ASSERT_EQ(explicit_empty2.entries().size(), 0);

    Value explicit_string = Value::String("test");
    ASSERT_EQ(explicit_string.entries().size(), 1);
    ASSERT_EQ(explicit_string.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*explicit_string.entries().at(0).string(), "test");

    Value explicit_variable = Value::Variable("VARIABLE");
    EXPECT_EQ(explicit_variable.entries().size(), 1);
    ASSERT_EQ(explicit_variable.entries().at(0).type(), Value::Entry::Type::Value);
    ASSERT_EQ(explicit_variable.entries().at(0).value()->entries().size(), 1);
    ASSERT_EQ(explicit_variable.entries().at(0).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*explicit_variable.entries().at(0).value()->entries().at(0).string(), "VARIABLE");

    Value empty = Value::Parse("");
    ASSERT_EQ(empty.entries().size(), 0);

    Value basic = Value::Parse("value");
    ASSERT_EQ(basic.entries().size(), 1);
    ASSERT_EQ(basic.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*basic.entries().at(0).string(), "value");

    Value variable1 = Value::Parse("$(VARIABLE)");
    EXPECT_EQ(variable1.entries().size(), 1);
    ASSERT_EQ(variable1.entries().at(0).type(), Value::Entry::Type::Value);
    ASSERT_EQ(variable1.entries().at(0).value()->entries().size(), 1);
    ASSERT_EQ(variable1.entries().at(0).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*variable1.entries().at(0).value()->entries().at(0).string(), "VARIABLE");

    Value variable2 = Value::Parse("${VARIABLE}");
    ASSERT_EQ(variable2.entries().size(), 1);
    ASSERT_EQ(variable2.entries().at(0).type(), Value::Entry::Type::Value);
    ASSERT_EQ(variable2.entries().at(0).value()->entries().size(), 1);
    ASSERT_EQ(variable2.entries().at(0).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*variable2.entries().at(0).value()->entries().at(0).string(), "VARIABLE");

    Value variable3 = Value::Parse("$VARIABLE");
    ASSERT_EQ(variable3.entries().size(), 1);
    ASSERT_EQ(variable3.entries().at(0).type(), Value::Entry::Type::Value);
    ASSERT_EQ(variable3.entries().at(0).value()->entries().size(), 1);
    ASSERT_EQ(variable3.entries().at(0).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*variable3.entries().at(0).value()->entries().at(0).string(), "VARIABLE");

    Value nested1 = Value::Parse("$($(NAME))");
    ASSERT_EQ(nested1.entries().size(), 1);
    ASSERT_EQ(nested1.entries().at(0).type(), Value::Entry::Type::Value);
    ASSERT_EQ(nested1.entries().at(0).value()->entries().size(), 1);
    ASSERT_EQ(nested1.entries().at(0).value()->entries().at(0).type(), Value::Entry::Type::Value);
    EXPECT_EQ(nested1.entries().at(0).value()->entries().at(0).value()->entries().size(), 1);
    ASSERT_EQ(nested1.entries().at(0).value()->entries().at(0).value()->entries().at(0).type(), Value::Entry::Type::String);
    ASSERT_EQ(*nested1.entries().at(0).value()->entries().at(0).value()->entries().at(0).string(), "NAME");

    Value partial1 = Value::Parse("$(open");
    ASSERT_EQ(partial1.entries().size(), 1);
    ASSERT_EQ(partial1.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*partial1.entries().at(0).string(), "$(open");

    Value partial2 = Value::Parse("$((open)");
    ASSERT_EQ(partial2.entries().size(), 1);
    ASSERT_EQ(partial2.entries().at(0).type(), Value::Entry::Type::Value);
    ASSERT_EQ(partial2.entries().at(0).value()->entries().size(), 1);
    ASSERT_EQ(partial2.entries().at(0).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*partial2.entries().at(0).value()->entries().at(0).string(), "(open");

    Value partial3 = Value::Parse("$");
    ASSERT_EQ(partial3.entries().size(), 1);
    ASSERT_EQ(partial3.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*partial3.entries().at(0).string(), "$");

    Value partial4 = Value::Parse("$$");
    ASSERT_EQ(partial4.entries().size(), 1);
    ASSERT_EQ(partial4.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*partial4.entries().at(0).string(), "$$");
}

TEST(Value, Complex)
{
    Value mixed1 = Value::Parse("STRING_$(VARIABLE)");
    ASSERT_EQ(mixed1.entries().size(), 2);
    ASSERT_EQ(mixed1.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*mixed1.entries().at(0).string(), "STRING_");
    ASSERT_EQ(mixed1.entries().at(1).type(), Value::Entry::Type::Value);
    ASSERT_EQ(mixed1.entries().at(1).value()->entries().size(), 1);
    ASSERT_EQ(mixed1.entries().at(1).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*mixed1.entries().at(1).value()->entries().at(0).string(), "VARIABLE");

    Value mixed2 = Value::Parse("STRING_$(VARIABLE)_STRING");
    ASSERT_EQ(mixed2.entries().size(), 3);
    ASSERT_EQ(mixed2.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*mixed2.entries().at(0).string(), "STRING_");
    ASSERT_EQ(mixed2.entries().at(1).type(), Value::Entry::Type::Value);
    ASSERT_EQ(mixed2.entries().at(1).value()->entries().size(), 1);
    ASSERT_EQ(mixed2.entries().at(1).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*mixed2.entries().at(1).value()->entries().at(0).string(), "VARIABLE");
    ASSERT_EQ(mixed2.entries().at(2).type(), Value::Entry::Type::String);
    EXPECT_EQ(*mixed2.entries().at(2).string(), "_STRING");

    Value mixed3 = Value::Parse("STRING_$(VARIABLE)_STRING_$(VARIABLE)");
    ASSERT_EQ(mixed3.entries().size(), 4);
    ASSERT_EQ(mixed3.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*mixed3.entries().at(0).string(), "STRING_");
    ASSERT_EQ(mixed3.entries().at(1).type(), Value::Entry::Type::Value);
    ASSERT_EQ(mixed3.entries().at(1).value()->entries().size(), 1);
    ASSERT_EQ(mixed3.entries().at(1).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*mixed3.entries().at(1).value()->entries().at(0).string(), "VARIABLE");
    ASSERT_EQ(mixed3.entries().at(2).type(), Value::Entry::Type::String);
    EXPECT_EQ(*mixed3.entries().at(2).string(), "_STRING_");
    ASSERT_EQ(mixed3.entries().at(3).type(), Value::Entry::Type::Value);
    ASSERT_EQ(mixed3.entries().at(3).value()->entries().size(), 1);
    ASSERT_EQ(mixed3.entries().at(3).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*mixed3.entries().at(3).value()->entries().at(0).string(), "VARIABLE");

    Value nested1 = Value::Parse("ONE_$(TWO_$(THREE))");
    ASSERT_EQ(nested1.entries().size(), 2);
    ASSERT_EQ(nested1.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*nested1.entries().at(0).string(), "ONE_");
    ASSERT_EQ(nested1.entries().at(1).type(), Value::Entry::Type::Value);
    ASSERT_EQ(nested1.entries().at(1).value()->entries().size(), 2);
    ASSERT_EQ(nested1.entries().at(1).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*nested1.entries().at(1).value()->entries().at(0).string(), "TWO_");
    ASSERT_EQ(nested1.entries().at(1).value()->entries().at(1).type(), Value::Entry::Type::Value);
    EXPECT_EQ(nested1.entries().at(1).value()->entries().at(1).value()->entries().size(), 1);
    ASSERT_EQ(nested1.entries().at(1).value()->entries().at(1).value()->entries().at(0).type(), Value::Entry::Type::String);
    ASSERT_EQ(*nested1.entries().at(1).value()->entries().at(1).value()->entries().at(0).string(), "THREE");

    Value nested2 = Value::Parse("ONE_$(TWO_${THREE}_FOUR_$(FIVE)_SIX)_SEVEN");
    ASSERT_EQ(nested2.entries().size(), 3);
    ASSERT_EQ(nested2.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*nested2.entries().at(0).string(), "ONE_");
    ASSERT_EQ(nested2.entries().at(1).type(), Value::Entry::Type::Value);
    ASSERT_EQ(nested2.entries().at(1).value()->entries().size(), 5);
    ASSERT_EQ(nested2.entries().at(1).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*nested2.entries().at(1).value()->entries().at(0).string(), "TWO_");
    ASSERT_EQ(nested2.entries().at(1).value()->entries().at(1).type(), Value::Entry::Type::Value);
    EXPECT_EQ(nested2.entries().at(1).value()->entries().at(1).value()->entries().size(), 1);
    ASSERT_EQ(nested2.entries().at(1).value()->entries().at(1).value()->entries().at(0).type(), Value::Entry::Type::String);
    ASSERT_EQ(*nested2.entries().at(1).value()->entries().at(1).value()->entries().at(0).string(), "THREE");
    ASSERT_EQ(nested2.entries().at(1).value()->entries().at(2).type(), Value::Entry::Type::String);
    EXPECT_EQ(*nested2.entries().at(1).value()->entries().at(2).string(), "_FOUR_");
    ASSERT_EQ(nested2.entries().at(1).value()->entries().at(3).type(), Value::Entry::Type::Value);
    EXPECT_EQ(nested2.entries().at(1).value()->entries().at(3).value()->entries().size(), 1);
    ASSERT_EQ(nested2.entries().at(1).value()->entries().at(3).value()->entries().at(0).type(), Value::Entry::Type::String);
    ASSERT_EQ(*nested2.entries().at(1).value()->entries().at(3).value()->entries().at(0).string(), "FIVE");
    ASSERT_EQ(nested2.entries().at(1).value()->entries().at(4).type(), Value::Entry::Type::String);
    EXPECT_EQ(*nested2.entries().at(1).value()->entries().at(4).string(), "_SIX");
    ASSERT_EQ(nested2.entries().at(2).type(), Value::Entry::Type::String);
    EXPECT_EQ(*nested2.entries().at(2).string(), "_SEVEN");
}

TEST(Value, Add)
{
    Value empty = Value::Empty();
    Value string = Value::String("test");
    Value string2 = Value::String("string");
    Value variable = Value::Variable("VARIABLE");

    Value empty_string = empty + string;
    ASSERT_EQ(empty_string.entries().size(), 1);
    ASSERT_EQ(empty_string.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*empty_string.entries().at(0).string(), "test");

    Value empty_variable = empty + variable;
    EXPECT_EQ(empty_variable.entries().size(), 1);
    ASSERT_EQ(empty_variable.entries().at(0).type(), Value::Entry::Type::Value);
    ASSERT_EQ(empty_variable.entries().at(0).value()->entries().size(), 1);
    ASSERT_EQ(empty_variable.entries().at(0).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*empty_variable.entries().at(0).value()->entries().at(0).string(), "VARIABLE");

    Value string_variable = string + variable;
    ASSERT_EQ(string_variable.entries().size(), 2);
    ASSERT_EQ(string_variable.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*string_variable.entries().at(0).string(), "test");
    ASSERT_EQ(string_variable.entries().at(1).type(), Value::Entry::Type::Value);
    ASSERT_EQ(string_variable.entries().at(1).value()->entries().size(), 1);
    ASSERT_EQ(string_variable.entries().at(1).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*string_variable.entries().at(1).value()->entries().at(0).string(), "VARIABLE");

    Value string_empty_variable = string + empty + variable;
    ASSERT_EQ(string_empty_variable.entries().size(), 2);
    ASSERT_EQ(string_empty_variable.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*string_empty_variable.entries().at(0).string(), "test");
    ASSERT_EQ(string_empty_variable.entries().at(1).type(), Value::Entry::Type::Value);
    ASSERT_EQ(string_empty_variable.entries().at(1).value()->entries().size(), 1);
    ASSERT_EQ(string_empty_variable.entries().at(1).value()->entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*string_empty_variable.entries().at(1).value()->entries().at(0).string(), "VARIABLE");

    Value string_string = string + string2;
    ASSERT_EQ(string_string.entries().size(), 1);
    ASSERT_EQ(string_string.entries().at(0).type(), Value::Entry::Type::String);
    EXPECT_EQ(*string_string.entries().at(0).string(), "teststring");
}
