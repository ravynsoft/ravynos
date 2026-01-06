/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcsdk/Configuration.h>
#include <xcsdk/SDK/Manager.h>
#include <xcsdk/SDK/Platform.h>
#include <xcsdk/SDK/Toolchain.h>
#include <libutil/MemoryFilesystem.h>

using xcsdk::Configuration;
using xcsdk::SDK::Manager;
using xcsdk::SDK::Platform;
using xcsdk::SDK::Toolchain;
using libutil::MemoryFilesystem;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

TEST(Manager, ConfigurationExtraPaths)
{
    auto filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::Directory("ExtraPlatforms", {
            MemoryFilesystem::Entry::Directory("Extra.platform", {
                MemoryFilesystem::Entry::File("Info.plist", Contents("{ \
                    Identifier = extra; \
                    Name = Extra; \
                }")),
            }),
        }),
        MemoryFilesystem::Entry::Directory("ExtraToolchains", {
            MemoryFilesystem::Entry::Directory("Extra.xctoolchain", {
                MemoryFilesystem::Entry::File("ToolchainInfo.plist", Contents("{ \
                    Identifier = extra; \
                }")),
            }),
        }),
    });

    auto configuration = Configuration({ filesystem.path("ExtraPlatforms") }, { filesystem.path("ExtraToolchains") });
    auto manager = Manager::Open(&filesystem, filesystem.path(""), configuration);
    ASSERT_NE(manager, nullptr);

    ASSERT_EQ(manager->platforms().size(), 1);
    Platform::shared_ptr const &platform = manager->platforms().front();
    EXPECT_EQ(platform->identifier(), std::string("extra"));
    EXPECT_EQ(platform->name(), "Extra");

    ASSERT_EQ(manager->toolchains().size(), 1);
    Toolchain::shared_ptr const &toolchain = manager->toolchains().front();
    EXPECT_EQ(toolchain->identifier(), std::string("extra"));
}
