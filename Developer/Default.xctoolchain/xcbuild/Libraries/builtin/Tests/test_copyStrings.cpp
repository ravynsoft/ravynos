/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <builtin/copyStrings/Driver.h>
#include <libutil/MemoryFilesystem.h>
#include <process/MemoryContext.h>
#include <plist/Format/Encoding.h>

using builtin::copyStrings::Driver;
using libutil::MemoryFilesystem;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

TEST(copyStrings, Name)
{
    Driver driver;
    EXPECT_EQ(driver.name(), "builtin-copyStrings");
}

TEST(copyStrings, CopyMultiple)
{
    std::vector<uint8_t> contents;
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::File("in1.strings", Contents("string1 = value1;")),
        MemoryFilesystem::Entry::File("in2.strings", Contents("string2 = value2;")),
        MemoryFilesystem::Entry::Directory("output", { }),
    });

    Driver driver;
    process::MemoryContext processContext = process::MemoryContext(
        driver.name(),
        filesystem.path(""),
        {
            "in1.strings",
            "in2.strings",
            "--outdir", "output",
            "--outputencoding", "utf-8",
        },
        std::unordered_map<std::string, std::string>());
    EXPECT_EQ(0, driver.run(&processContext, &filesystem));

    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("output/in1.strings")));
    EXPECT_EQ(contents, Contents("string1 = value1;\n"));

    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("output/in2.strings")));
    EXPECT_EQ(contents, Contents("string2 = value2;\n"));
}

TEST(copyStrings, InputOutputEncoding)
{
    std::unordered_map<std::string, plist::Format::Encoding> encodings = {
        { "utf-8", plist::Format::Encoding::UTF8 },
        { "utf-16", plist::Format::Encoding::UTF16LE },
        { "utf-32", plist::Format::Encoding::UTF32LE },
    };

    for (auto const &entry1 : encodings) {
        for (auto const &entry2 : encodings) {
            /* Convert input to each encoding. */
            MemoryFilesystem filesystem = MemoryFilesystem({
                MemoryFilesystem::Entry::File("in.strings", plist::Format::Encodings::Convert(
                    Contents("string = value;\n"),
                    plist::Format::Encoding::UTF8,
                    entry1.second
                )),
                MemoryFilesystem::Entry::Directory("output", { }),
            });

            Driver driver;
            process::MemoryContext processContext = process::MemoryContext(
                driver.name(),
                filesystem.path(""),
                {
                    "in.strings",
                    "--outdir", "output",
                    "--inputencoding", entry1.first,
                    "--outputencoding", entry2.first,
                },
                std::unordered_map<std::string, std::string>());
            EXPECT_EQ(0, driver.run(&processContext, &filesystem));

            std::vector<uint8_t> contents;
            EXPECT_TRUE(filesystem.read(&contents, filesystem.path("output/in.strings")));

            /* Expect output in each encoding. */
            EXPECT_EQ(contents, plist::Format::Encodings::Convert(
                Contents("string = value;\n"),
                plist::Format::Encoding::UTF8,
                entry2.second
            ));
        }
    }
}

