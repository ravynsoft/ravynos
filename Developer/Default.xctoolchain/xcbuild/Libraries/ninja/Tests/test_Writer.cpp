/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <ninja/Writer.h>
#include <ninja/Value.h>

using ninja::Writer;
using ninja::Value;
using ninja::Binding;

TEST(Writer, Basic)
{
    Writer writer;
    EXPECT_EQ(writer.serialize(), "");
}

TEST(Writer, Newline)
{
    Writer writer;
    writer.newline();
    EXPECT_EQ(writer.serialize(), "\n");
}

TEST(Writer, Comment)
{
    Writer writer;

    writer.comment("comment");
    EXPECT_EQ(writer.serialize(), "# comment\n");

    writer.comment("comment 2");
    EXPECT_EQ(writer.serialize(), "# comment\n# comment 2\n");
}

TEST(Writer, Binding)
{
    Writer writer;

    writer.binding({ "variable", Value::String("value") });
    EXPECT_EQ(writer.serialize(), "variable = value\n");

    writer.binding({ "expression", Value::Expression("value is $variable") });
    EXPECT_EQ(writer.serialize(), "variable = value\nexpression = value is $variable\n");

    Writer indent;
    indent.binding({ "indented", Value::String("value") }, 1);
    EXPECT_EQ(indent.serialize(), "  indented = value\n");
}

TEST(Writer, Command)
{
    Writer simple;
    simple.command("command", "remaining");
    EXPECT_EQ(simple.serialize(), "command remaining\n\n");

    Writer scoped;
    scoped.command("command", "remaining", {
        { "scoped", Value::String("value") },
    });
    EXPECT_EQ(scoped.serialize(), "command remaining\n  scoped = value\n\n");

    Writer multiple;
    multiple.command("commandone", "remainingone", {
        { "variableone", Value::String("valueone") },
    });
    multiple.command("commandtwo", "remainingtwo", {
        { "variabletwo", Value::String("valuetwo") },
    });
    EXPECT_EQ(multiple.serialize(), "commandone remainingone\n  variableone = valueone\n\ncommandtwo remainingtwo\n  variabletwo = valuetwo\n\n");
}

TEST(Writer, Subninja)
{
    Writer subninja;
    subninja.subninja(Value::String("simple/path"));
    EXPECT_EQ(subninja.serialize(), "subninja simple/path\n\n");

    Writer subninja2;
    subninja2.subninja(Value::Expression("e$ca ped/pa:th"));
    EXPECT_EQ(subninja2.serialize(), "subninja e$ca$ ped/pa:th\n\n");
}

TEST(Writer, Include)
{
    Writer include;
    include.include(Value::String("simple/path"));
    EXPECT_EQ(include.serialize(), "include simple/path\n\n");

    Writer include2;
    include2.include(Value::Expression("e$ca ped/pa:th"));
    EXPECT_EQ(include2.serialize(), "include e$ca$ ped/pa:th\n\n");
}

TEST(Writer, Default)
{
    Writer writer;
    writer.default_({
        /* Should not escape. */
        Value::String("one"),
        Value::Expression("two"),
        /* Should escape. */
        Value::String("th$ree"),
        /* Should not escape colons, not a build rule. */
        Value::Expression("fo:ur"),
        /* Should escape the space, but not the $. */
        Value::Expression("f$i ve"),
        /* Should escape the spaces. */
        Value::String("s i x"),
    });
    EXPECT_EQ(writer.serialize(), "default one two th$$ree fo:ur f$i$ ve s$ i$ x\n\n");
}

TEST(Writer, Rule)
{
    Writer writer;
    writer.rule("name", Value::String("echo \"hello world\""), {
        { "description", Value::String("echo$ing") },
        { "rspfile", Value::Expression("$out.rsp") },
    });
    EXPECT_EQ(writer.serialize(), "rule name\n  command = echo \"hello world\"\n  description = echo$$ing\n  rspfile = $out.rsp\n\n");
}

TEST(Writer, Build)
{
    std::vector<Binding> bindings = {
        { "description", Value::String("hello world") },
    };

    std::vector<Value> inputs = {
        Value::String("in1.txt"),
        Value::String("in2.txt"),
    };

    std::vector<Value> dependencies = {
        Value::String("dep1.txt"),
        Value::String("dep2.txt"),
    };

    std::vector<Value> orders = {
        Value::String("order1.txt"),
        Value::String("order2.txt"),
    };

    Writer writer;
    writer.build({ Value::String("file/pa$th.ext") }, "rule", inputs, bindings, dependencies, orders);
    EXPECT_EQ(writer.serialize(), "build file/pa$$th.ext: rule in1.txt in2.txt | dep1.txt dep2.txt || order1.txt order2.txt\n  description = hello world\n\n");
}

TEST(Writer, Pool)
{
    Writer writer;
    writer.pool("name", 4);
    EXPECT_EQ(writer.serialize(), "pool name\n  depth = 4\n\n");
}

