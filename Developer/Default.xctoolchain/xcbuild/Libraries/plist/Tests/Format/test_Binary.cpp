/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <plist/Format/Binary.h>
#include <plist/Objects.h>

using plist::Format::Binary;
using plist::String;
using plist::Dictionary;

TEST(Binary, UnicodeString)
{
    /*
     * UTF-8 encoding of these emoji:
     *
     *   U+1F643 U+1F636 U+1F62E U+1F62D
     *   U+1F626 U+1F610 U+1F602 U+1F644
     *   U+1F62E U+1F604 U+1F605 U+1F61B
     */
    static char const *expected =
        "\xf0\x9f\x99\x83" "\xf0\x9f\x98\xb6" "\xf0\x9f\x98\xae" "\xf0\x9f\x98\xad"
        "\xf0\x9f\x98\xa6" "\xf0\x9f\x98\x90" "\xf0\x9f\x98\x82" "\xf0\x9f\x99\x84"
        "\xf0\x9f\x98\xae" "\xf0\x9f\x98\x84" "\xf0\x9f\x98\x85" "\xf0\x9f\x98\x9b";

    /*
     * Binary property list containing the above characters.
     */
    std::vector<uint8_t> contents = {
        0x62, 0x70, 0x6c, 0x69, 0x73, 0x74, 0x30, 0x30, 0x6f, 0x10, 0x18, 0xd8,
        0x3d, 0xde, 0x43, 0xd8, 0x3d, 0xde, 0x36, 0xd8, 0x3d, 0xde, 0x2e, 0xd8,
        0x3d, 0xde, 0x2d, 0xd8, 0x3d, 0xde, 0x26, 0xd8, 0x3d, 0xde, 0x10, 0xd8,
        0x3d, 0xde, 0x02, 0xd8, 0x3d, 0xde, 0x44, 0xd8, 0x3d, 0xde, 0x2e, 0xd8,
        0x3d, 0xde, 0x04, 0xd8, 0x3d, 0xde, 0x05, 0xd8, 0x3d, 0xde, 0x1b, 0x08,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3b,
    };

    std::unique_ptr<Binary> format = Binary::Identify(contents);
    ASSERT_NE(format, nullptr);

    auto deserialize = Binary::Deserialize(contents, Binary::Create());
    ASSERT_NE(deserialize.first, nullptr);

    auto string = String::New(expected);
    EXPECT_TRUE(deserialize.first->equals(string.get()));

    auto serialize = Binary::Serialize(deserialize.first.get(), Binary::Create());
    ASSERT_NE(serialize.first, nullptr);
    EXPECT_EQ(*serialize.first, contents);
}

