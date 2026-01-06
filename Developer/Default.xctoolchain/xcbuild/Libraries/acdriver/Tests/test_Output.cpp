/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <acdriver/Output.h>
#include <plist/String.h>

using acdriver::Output;

TEST(Output, Text)
{
    Output output;
    output.add("output.test1", plist::String::New("test1"), "text-output1");
    output.add("output.test2", plist::String::New("test2"), "text-output2");

    ext::optional<std::vector<uint8_t>> contents = output.serialize(Output::Format::Text);
    ASSERT_TRUE(contents);

    std::string value = std::string(contents->begin(), contents->end());
    EXPECT_EQ(value, "/* output.test1 */\ntext-output1\n/* output.test2 */\ntext-output2\n");
}

TEST(Output, Empty)
{
    Output output;
    EXPECT_EQ(output.serialize(Output::Format::Text), std::vector<uint8_t>());
    EXPECT_EQ(output.serialize(Output::Format::XML), std::vector<uint8_t>());
    EXPECT_EQ(output.serialize(Output::Format::Binary), std::vector<uint8_t>());
}
