/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcsdk/SDK/Toolchain.h>
#include <libutil/MemoryFilesystem.h>

using xcsdk::SDK::Toolchain;
using libutil::MemoryFilesystem;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

TEST(Toolchain, Info)
{
    std::string name1 = "Test1.xctoolchain";
    std::string name2 = "Test2.xctoolchain";
    std::string name3 = "Test3.xctoolchain";

    std::string identifier = "test.toolchain";
    std::vector<uint8_t> info = Contents("{ \
        Identifier = \"" + identifier + "\"; \
    }");

    auto filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::Directory(name1, {
            MemoryFilesystem::Entry::File("ToolchainInfo.plist", info),
        }),
        MemoryFilesystem::Entry::Directory(name2, {
            MemoryFilesystem::Entry::File("Info.plist", info),
        }),
        MemoryFilesystem::Entry::Directory(name3, { }),
    });

    /* ToolchainInfo.plist is preferred. */
    auto toolchain1 = Toolchain::Open(&filesystem, filesystem.path("") + name1);
    ASSERT_NE(toolchain1, nullptr);
    EXPECT_EQ(toolchain1->identifier(), identifier);

    /* Info.plist is supported. */
    auto toolchain2 = Toolchain::Open(&filesystem, filesystem.path("") + name2);
    ASSERT_NE(toolchain2, nullptr);
    EXPECT_EQ(toolchain2->identifier(), identifier);

    /* Some info is required. */
    auto toolchain3 = Toolchain::Open(&filesystem, filesystem.path("") + name3);
    ASSERT_EQ(toolchain3, nullptr);
}

