/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <plist/Format/XML.h>
#include <plist/Objects.h>

using plist::Format::XML;
using plist::Format::Encoding;
using plist::String;
using plist::Boolean;
using plist::Integer;
using plist::Real;
using plist::UID;
using plist::Dictionary;
using plist::Array;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

static char const *const XMLHeader =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
    "<plist version=\"1.0\">\n";

static char const *const XMLFooter =
    "</plist>\n";

TEST(XML, Boolean)
{
    auto contents = Contents(std::string(XMLHeader) + "<dict>\n\t<key>true</key>\n\t<true />\n\t<key>false</key>\n\t<false />\n</dict>\n" + std::string(XMLFooter));
    EXPECT_NE(XML::Identify(contents), nullptr);

    auto deserialize = XML::Deserialize(contents, XML::Create(Encoding::UTF8));
    ASSERT_NE(deserialize.first, nullptr);

    auto dictionary = Dictionary::New();
    dictionary->set("true", Boolean::New(true));
    dictionary->set("false", Boolean::New(false));
    EXPECT_TRUE(deserialize.first->equals(dictionary.get()));

    auto serialize = XML::Serialize(dictionary.get(), XML::Create(Encoding::UTF8));
    ASSERT_NE(serialize.first, nullptr);
    EXPECT_EQ(*serialize.first, contents);
}

TEST(XML, UID)
{
    auto contents = Contents(std::string(XMLHeader) + "<dict>\n\t<key>object</key>\n\t<dict>\n\t\t<key>CF$UID</key>\n\t\t<integer>4</integer>\n\t</dict>\n</dict>\n" + std::string(XMLFooter));
    EXPECT_NE(XML::Identify(contents), nullptr);

    auto deserialize = XML::Deserialize(contents, XML::Create(Encoding::UTF8));
    ASSERT_NE(deserialize.first, nullptr);

    auto dictionary = Dictionary::New();
    dictionary->set("object", UID::New(4));
    EXPECT_TRUE(deserialize.first->equals(dictionary.get()));

    auto serialize = XML::Serialize(dictionary.get(), XML::Create(Encoding::UTF8));
    ASSERT_NE(serialize.first, nullptr);
    EXPECT_EQ(*serialize.first, contents);
}

