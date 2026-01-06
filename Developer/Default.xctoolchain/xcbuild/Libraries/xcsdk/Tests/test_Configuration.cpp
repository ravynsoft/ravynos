/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcsdk/Configuration.h>
#include <libutil/MemoryFilesystem.h>

using xcsdk::Configuration;
using libutil::MemoryFilesystem;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

TEST(Configuration, Load)
{
    auto filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::File("Configuration.plist", Contents("{ \
            ExtraPlatformsPaths = ( one, three ); \
            ExtraToolchainsPaths = ( two, four ); \
        }")),
    });

    auto configuration = Configuration::Load(&filesystem, { filesystem.path("Configuration.plist") });
    ASSERT_NE(configuration, ext::nullopt);
    EXPECT_EQ(configuration->extraPlatformsPaths(), std::vector<std::string>({ "one", "three" }));
    EXPECT_EQ(configuration->extraToolchainsPaths(), std::vector<std::string>({ "two", "four" }));
}

TEST(Configuration, LoadRealFile)
{
    auto filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::File("Configuration.plist", Contents("{ \
            ExtraPlatformsPaths = ( one, three ); \
            ExtraToolchainsPaths = ( two, four ); \
        }")),
    });

    auto configuration = Configuration::Load(&filesystem, { filesystem.path("FakeConfiguration.plist"), filesystem.path("Configuration.plist") });
    ASSERT_NE(configuration, ext::nullopt);
    EXPECT_EQ(configuration->extraPlatformsPaths(), std::vector<std::string>({ "one", "three" }));
    EXPECT_EQ(configuration->extraToolchainsPaths(), std::vector<std::string>({ "two", "four" }));
}

TEST(Configuration, LoadFirstValidFile)
{
    auto filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::File("Configuration.plist", Contents("{ \
            ExtraPlatformsPaths = ( one, three ); \
            ExtraToolchainsPaths = ( two, four ); \
        }")),
        MemoryFilesystem::Entry::File("Configuration2.plist", Contents("{ \
            ExtraPlatformsPaths = ( five, six ); \
            ExtraToolchainsPaths = ( seven, eight ); \
        }")),
    });

    auto configuration = Configuration::Load(&filesystem, { filesystem.path("Configuration.plist"), filesystem.path("Configuration2.plist") });
    ASSERT_NE(configuration, ext::nullopt);
    EXPECT_EQ(configuration->extraPlatformsPaths(), std::vector<std::string>({ "one", "three" }));
    EXPECT_EQ(configuration->extraToolchainsPaths(), std::vector<std::string>({ "two", "four" }));
}
