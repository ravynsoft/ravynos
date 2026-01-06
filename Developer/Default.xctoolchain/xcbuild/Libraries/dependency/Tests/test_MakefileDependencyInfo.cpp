/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <dependency/MakefileDependencyInfo.h>

using dependency::MakefileDependencyInfo;
using dependency::DependencyInfo;

TEST(MakefileDependencyInfo, Empty)
{
    auto info = MakefileDependencyInfo::Deserialize("");
    ASSERT_TRUE(info);
    EXPECT_EQ(info->dependencyInfo().size(), 0);
}

TEST(MakefileDependencyInfo, Invalid)
{
    EXPECT_FALSE(MakefileDependencyInfo::Deserialize(":"));
    EXPECT_FALSE(MakefileDependencyInfo::Deserialize(": input"));
    EXPECT_FALSE(MakefileDependencyInfo::Deserialize("out%put:"));
    EXPECT_FALSE(MakefileDependencyInfo::Deserialize("output: :"));
    EXPECT_FALSE(MakefileDependencyInfo::Deserialize("output: input%"));
    EXPECT_FALSE(MakefileDependencyInfo::Deserialize("output: input\ninput"));
}

TEST(MakefileDependencyInfo, NoInputs)
{
    auto info1 = MakefileDependencyInfo::Deserialize("output:");
    ASSERT_TRUE(info1);
    ASSERT_EQ(info1->dependencyInfo().size(), 1);
    EXPECT_TRUE(info1->dependencyInfo().front().inputs().empty());
    EXPECT_EQ(info1->dependencyInfo().front().outputs(), std::vector<std::string>({ "output" }));

    auto info2 = MakefileDependencyInfo::Deserialize("output1: \n output2: \n");
    ASSERT_TRUE(info2);
    ASSERT_EQ(info2->dependencyInfo().size(), 2);
    EXPECT_TRUE(info2->dependencyInfo().front().inputs().empty());
    EXPECT_EQ(info2->dependencyInfo().front().outputs(), std::vector<std::string>({ "output1" }));
    EXPECT_TRUE(info2->dependencyInfo().back().inputs().empty());
    EXPECT_EQ(info2->dependencyInfo().back().outputs(), std::vector<std::string>({ "output2" }));
}

TEST(MakefileDependencyInfo, Inputs)
{
    auto info1 = MakefileDependencyInfo::Deserialize("output: input1 input2");
    ASSERT_TRUE(info1);
    ASSERT_EQ(info1->dependencyInfo().size(), 1);
    EXPECT_EQ(info1->dependencyInfo().front().inputs(), std::vector<std::string>({ "input1", "input2" }));
    EXPECT_EQ(info1->dependencyInfo().front().outputs(), std::vector<std::string>({ "output" }));

    auto info2 = MakefileDependencyInfo::Deserialize("output1: input1\noutput2: input2");
    ASSERT_TRUE(info2);
    ASSERT_EQ(info2->dependencyInfo().size(), 2);
    EXPECT_EQ(info2->dependencyInfo().front().inputs(), std::vector<std::string>({ "input1" }));
    EXPECT_EQ(info2->dependencyInfo().front().outputs(), std::vector<std::string>({ "output1" }));
    EXPECT_EQ(info2->dependencyInfo().back().inputs(), std::vector<std::string>({ "input2" }));
    EXPECT_EQ(info2->dependencyInfo().back().outputs(), std::vector<std::string>({ "output2" }));

    auto info3 = MakefileDependencyInfo::Deserialize("output: input1\\\n    input2  input3");
    ASSERT_TRUE(info3);
    ASSERT_EQ(info3->dependencyInfo().size(), 1);
    EXPECT_EQ(info3->dependencyInfo().front().inputs(), std::vector<std::string>({ "input1", "input2", "input3" }));
    EXPECT_EQ(info3->dependencyInfo().front().outputs(), std::vector<std::string>({ "output" }));
}

TEST(MakefileDependencyInfo, Escapes)
{
    auto info1 = MakefileDependencyInfo::Deserialize("out\\%put: in\\%put");
    ASSERT_TRUE(info1);
    ASSERT_EQ(info1->dependencyInfo().size(), 1);
    EXPECT_EQ(info1->dependencyInfo().front().inputs(), std::vector<std::string>({ "in%put" }));
    EXPECT_EQ(info1->dependencyInfo().back().outputs(), std::vector<std::string>({ "out%put" }));

    auto info2 = MakefileDependencyInfo::Deserialize("out\\#put: in\\#put");
    ASSERT_TRUE(info2);
    ASSERT_EQ(info2->dependencyInfo().size(), 1);
    EXPECT_EQ(info2->dependencyInfo().front().inputs(), std::vector<std::string>({ "in#put" }));
    EXPECT_EQ(info2->dependencyInfo().back().outputs(), std::vector<std::string>({ "out#put" }));

    auto info3 = MakefileDependencyInfo::Deserialize("out\\put: in\\put");
    ASSERT_TRUE(info3);
    ASSERT_EQ(info3->dependencyInfo().size(), 1);
    EXPECT_EQ(info3->dependencyInfo().front().inputs(), std::vector<std::string>({ "in\\put" }));
    EXPECT_EQ(info3->dependencyInfo().back().outputs(), std::vector<std::string>({ "out\\put" }));

    auto info4 = MakefileDependencyInfo::Deserialize("out\\ put: in\\ put");
    ASSERT_TRUE(info4);
    ASSERT_EQ(info4->dependencyInfo().size(), 1);
    EXPECT_EQ(info4->dependencyInfo().front().inputs(), std::vector<std::string>({ "in put" }));
    EXPECT_EQ(info4->dependencyInfo().back().outputs(), std::vector<std::string>({ "out put" }));
}

TEST(MakefileDependencyInfo, Comments)
{
    auto info1 = MakefileDependencyInfo::Deserialize("output: # input1 \\\n input2");
    ASSERT_TRUE(info1);
    ASSERT_EQ(info1->dependencyInfo().size(), 1);
    EXPECT_TRUE(info1->dependencyInfo().front().inputs().empty());
    EXPECT_EQ(info1->dependencyInfo().front().outputs(), std::vector<std::string>({ "output" }));

    auto info2 = MakefileDependencyInfo::Deserialize("# output: input1 input2");
    ASSERT_TRUE(info2);
    EXPECT_EQ(info2->dependencyInfo().size(), 0);

    auto info3 = MakefileDependencyInfo::Deserialize("# output1: input1 input2\noutput2: input3 input4");
    ASSERT_TRUE(info3);
    ASSERT_EQ(info3->dependencyInfo().size(), 1);
    EXPECT_EQ(info3->dependencyInfo().front().inputs(), std::vector<std::string>({ "input3", "input4" }));
    EXPECT_EQ(info3->dependencyInfo().front().outputs(), std::vector<std::string>({ "output2" }));

    auto info4 = MakefileDependencyInfo::Deserialize("# comment1\n# comment2\n# comment3\noutput:");
    ASSERT_TRUE(info4);
    ASSERT_EQ(info4->dependencyInfo().size(), 1);
    EXPECT_TRUE(info4->dependencyInfo().front().inputs().empty());
    EXPECT_EQ(info4->dependencyInfo().front().outputs(), std::vector<std::string>({ "output" }));
}

TEST(MakefileDependencyInfo, SerializeBasic)
{
    DependencyInfo dependencyInfo;
    dependencyInfo.inputs() = { "in1", "in2" };
    dependencyInfo.outputs() = { "out1", "out2" };

    MakefileDependencyInfo info;
    info.dependencyInfo() = { dependencyInfo };

    std::string serialized = info.serialize();
    EXPECT_EQ(info.serialize(), "out1 out2: \\\n  in1 \\\n  in2");
}

TEST(MakefileDependencyInfo, SerializeMultiple)
{
    DependencyInfo dependencyInfo1;
    dependencyInfo1.inputs() = { "in1a", "in1b" };
    dependencyInfo1.outputs() = { "out1" };

    DependencyInfo dependencyInfo2;
    dependencyInfo2.inputs() = { "in2a", "in2b" };
    dependencyInfo2.outputs() = { "out2" };

    MakefileDependencyInfo info;
    info.dependencyInfo() = {
        dependencyInfo1,
        dependencyInfo2,
    };

    std::string serialized = info.serialize();
    EXPECT_EQ(info.serialize(), "out1: \\\n  in1a \\\n  in1b\n\nout2: \\\n  in2a \\\n  in2b");
}
