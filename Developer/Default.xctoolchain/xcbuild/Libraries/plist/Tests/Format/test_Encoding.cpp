/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <plist/Format/Encoding.h>
#include <plist/Objects.h>

using plist::Format::Encoding;
using plist::Format::Encodings;

static std::vector<Encoding> const AllEncodings = {
    Encoding::UTF8,
    Encoding::UTF16BE,
    Encoding::UTF16LE,
    Encoding::UTF32BE,
    Encoding::UTF32LE,
};

/* Content: "hello <emoji> world" */

static std::vector<uint8_t> const Content_UTF8 = {
    0x68,
    0x65,
    0x6C,
    0x6C,
    0x6F,
    0x20,
    0xF0, 0x9F, 0x92, 0xA9,
    0x20,
    0x77,
    0x6F,
    0x72,
    0x6C,
    0x64,
};

static std::vector<uint8_t> const Content_UTF16BE = {
    0x00, 0x68,
    0x00, 0x65,
    0x00, 0x6C,
    0x00, 0x6C,
    0x00, 0x6F,
    0x00, 0x20,
    0xD8, 0x3D, 0xDC, 0xA9,
    0x00, 0x20,
    0x00, 0x77,
    0x00, 0x6F,
    0x00, 0x72,
    0x00, 0x6C,
    0x00, 0x64,
};

static std::vector<uint8_t> const Content_UTF16LE = {
    0x68, 0x00,
    0x65, 0x00,
    0x6C, 0x00,
    0x6C, 0x00,
    0x6F, 0x00,
    0x20, 0x00,
    0x3D, 0xD8, 0xA9, 0xDC,
    0x20, 0x00,
    0x77, 0x00,
    0x6F, 0x00,
    0x72, 0x00,
    0x6C, 0x00,
    0x64, 0x00,
};

static std::vector<uint8_t> const Content_UTF32BE = {
    0x00, 0x00, 0x00, 0x68,
    0x00, 0x00, 0x00, 0x65,
    0x00, 0x00, 0x00, 0x6C,
    0x00, 0x00, 0x00, 0x6C,
    0x00, 0x00, 0x00, 0x6F,
    0x00, 0x00, 0x00, 0x20,
    0x00, 0x01, 0xF4, 0xA9,
    0x00, 0x00, 0x00, 0x20,
    0x00, 0x00, 0x00, 0x77,
    0x00, 0x00, 0x00, 0x6F,
    0x00, 0x00, 0x00, 0x72,
    0x00, 0x00, 0x00, 0x6C,
    0x00, 0x00, 0x00, 0x64,
};

static std::vector<uint8_t> const Content_UTF32LE = {
    0x68, 0x00, 0x00, 0x00,
    0x65, 0x00, 0x00, 0x00,
    0x6C, 0x00, 0x00, 0x00,
    0x6C, 0x00, 0x00, 0x00,
    0x6F, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x00, 0x00,
    0xA9, 0xF4, 0x01, 0x00,
    0x20, 0x00, 0x00, 0x00,
    0x77, 0x00, 0x00, 0x00,
    0x6F, 0x00, 0x00, 0x00,
    0x72, 0x00, 0x00, 0x00,
    0x6C, 0x00, 0x00, 0x00,
    0x64, 0x00, 0x00, 0x00,
};

static std::vector<std::pair<Encoding, std::vector<uint8_t>>> AllContent = {
    { Encoding::UTF8, Content_UTF8 },
    { Encoding::UTF16BE, Content_UTF16BE },
    { Encoding::UTF16LE, Content_UTF16LE },
    { Encoding::UTF32BE, Content_UTF32BE },
    { Encoding::UTF32LE, Content_UTF32LE },
};


TEST(Encoding, Detect)
{
    /* BOM-less should assume UTF-8. */
    EXPECT_EQ(Encodings::Detect(Content_UTF8), Encoding::UTF8);
    EXPECT_EQ(Encodings::Detect(Content_UTF16BE), Encoding::UTF8);
    EXPECT_EQ(Encodings::Detect(Content_UTF16LE), Encoding::UTF8);
    EXPECT_EQ(Encodings::Detect(Content_UTF32BE), Encoding::UTF8);
    EXPECT_EQ(Encodings::Detect(Content_UTF16LE), Encoding::UTF8);
    EXPECT_EQ(Encodings::Detect(std::vector<uint8_t>()), Encoding::UTF8);
    EXPECT_EQ(Encodings::Detect({ 0xFF }), Encoding::UTF8);
    EXPECT_EQ(Encodings::Detect({ 0xFE }), Encoding::UTF8);
    EXPECT_EQ(Encodings::Detect({ 0xFF, 0xFF, 0xFF }), Encoding::UTF8);

    /* Just a BOM should detect as that encoding. */
    for (Encoding encoding : AllEncodings) {
        EXPECT_EQ(Encodings::Detect(Encodings::BOM(encoding)), encoding);
    }
}

TEST(Encoding, Convert)
{
    /* Test converting each into each other. */
    for (auto const &source : AllContent) {
        for (auto const &dest : AllContent) {
            EXPECT_EQ(Encodings::Convert(source.second, source.first, dest.first), dest.second);
        }
    }

    /* Test converting each into another and back. */
    for (auto const &source : AllContent) {
        for (Encoding test : AllEncodings) {
            std::vector<uint8_t> buffer = source.second;
            buffer = Encodings::Convert(buffer, source.first, test);
            buffer = Encodings::Convert(buffer, test, source.first);
            EXPECT_EQ(buffer, source.second);
        }
    }
}

TEST(Encoding, ConvertStripBOM)
{
    for (auto const &source : AllContent) {
        /* Insert BOM at the front. */
        std::vector<uint8_t> content = source.second;
        std::vector<uint8_t> BOM = Encodings::BOM(source.first);
        content.insert(content.begin(), BOM.begin(), BOM.end());

        /* Re-encode in the same encoding. */
        std::vector<uint8_t> converted = Encodings::Convert(content, source.first, source.first);

        /* Should not have BOM after conversion. */
        ASSERT_TRUE(converted.size() >= BOM.size());
        EXPECT_FALSE(std::equal(BOM.begin(), BOM.end(), converted.begin()));
    }
}
