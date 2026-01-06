/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <builtin/copyPlist/Driver.h>
#include <libutil/MemoryFilesystem.h>
#include <process/MemoryContext.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Format/Encoding.h>
#include <plist/Format/XML.h>
#include <plist/Format/Binary.h>

using builtin::copyPlist::Driver;
using libutil::MemoryFilesystem;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

TEST(copyPlist, Name)
{
    Driver driver;
    EXPECT_EQ(driver.name(), "builtin-copyPlist");
}

TEST(copyPlist, CopyMultiple)
{
    auto plist1 = plist::Dictionary::New();
    plist1->set("in1", plist::String::New("one"));
    auto format1 = plist::Format::Binary::Create();
    auto serialized1 = plist::Format::Binary::Serialize(plist1.get(), format1);
    ASSERT_NE(serialized1.first, nullptr);

    auto plist2 = plist::Dictionary::New();
    plist2->set("in2", plist::String::New("two"));
    auto format2 = plist::Format::XML::Create(plist::Format::Encoding::UTF8);
    auto serialized2 = plist::Format::XML::Serialize(plist2.get(), format2);
    ASSERT_NE(serialized2.first, nullptr);

    std::vector<uint8_t> contents;
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::File("in1.plist", *serialized1.first),
        MemoryFilesystem::Entry::File("in2.plist", *serialized2.first),
        MemoryFilesystem::Entry::File("in3.plist", Contents("{ in3 = \"three\"; }")),
        MemoryFilesystem::Entry::Directory("output", { }),
    });

    Driver driver;
    process::MemoryContext processContext = process::MemoryContext(
        driver.name(),
        filesystem.path(""),
        {
            "in1.plist",
            "in2.plist",
            "in3.plist",
            "--outdir", "output",
            "--convert", "ascii1",
        },
        std::unordered_map<std::string, std::string>());
    EXPECT_EQ(0, driver.run(&processContext, &filesystem));

    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("output/in1.plist")));
    EXPECT_EQ(contents, Contents("{\n\tin1 = one;\n}\n"));

    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("output/in2.plist")));
    EXPECT_EQ(contents, Contents("{\n\tin2 = two;\n}\n"));

    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("output/in3.plist")));
    EXPECT_EQ(contents, Contents("{\n\tin3 = three;\n}\n"));
}

