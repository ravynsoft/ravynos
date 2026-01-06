/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <pbxsetting/XC/Config.h>
#include <libutil/Filesystem.h>
#include <libutil/MemoryFilesystem.h>

using pbxsetting::XC::Config;
using pbxsetting::Environment;
using pbxsetting::Setting;
using pbxsetting::Value;
using libutil::Filesystem;
using libutil::MemoryFilesystem;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

TEST(Config, Empty)
{
    Environment environment = Environment();
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::File("empty.xcconfig", Contents("")),
    });

    auto config = Config::Load(&filesystem, environment, filesystem.path("empty.xcconfig"));
    ASSERT_NE(config, ext::nullopt);
    EXPECT_EQ(config->path(), filesystem.path("empty.xcconfig"));
    EXPECT_TRUE(config->contents().empty());
}

TEST(Config, Setting)
{
    Environment environment = Environment();
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::File("settings.xcconfig", Contents(
            "NAME1 = VALUE1\n"
            "NAME2 = VALUE2\n")),
    });

    auto config = Config::Load(&filesystem, environment, filesystem.path("settings.xcconfig"));
    ASSERT_NE(config, ext::nullopt);
    EXPECT_EQ(config->path(), filesystem.path("settings.xcconfig"));

    ASSERT_EQ(config->contents().size(), 2);
    ASSERT_EQ(config->contents().at(0).type(), Config::Entry::Type::Setting);
    ASSERT_EQ(config->contents().at(1).type(), Config::Entry::Type::Setting);

    auto level = config->level();

    std::vector<std::vector<Setting>> all = {
        {
            *config->contents().at(0).setting(),
            *config->contents().at(1).setting(),
        },
        level.settings(),
    };

    for (std::vector<Setting> const &settings : all) {
        ASSERT_EQ(settings.size(), 2);
        EXPECT_EQ(settings.at(0).name(), "NAME1");
        EXPECT_EQ(settings.at(0).value(), Value::String("VALUE1"));
        EXPECT_EQ(settings.at(1).name(), "NAME2");
        EXPECT_EQ(settings.at(1).value(), Value::String("VALUE2"));
    }
}

TEST(Config, Include)
{
    Environment environment = Environment();
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::File("common.xcconfig", Contents("NAME = VALUE")),
        MemoryFilesystem::Entry::File("include.xcconfig", Contents("#include \"common.xcconfig\"")),
    });

    auto config = Config::Load(&filesystem, environment, filesystem.path("include.xcconfig"));
    ASSERT_NE(config, ext::nullopt);
    EXPECT_EQ(config->path(), filesystem.path("include.xcconfig"));

    ASSERT_EQ(config->contents().size(), 1);
    ASSERT_EQ(config->contents().at(0).type(), Config::Entry::Type::Include);
    EXPECT_EQ(config->contents().at(0).config()->path(), filesystem.path("common.xcconfig"));
    ASSERT_EQ(config->contents().at(0).config()->contents().size(), 1);
    ASSERT_EQ(config->contents().at(0).config()->contents().at(0).type(), Config::Entry::Type::Setting);
    EXPECT_EQ(config->contents().at(0).config()->contents().at(0).setting()->name(), "NAME");
    EXPECT_EQ(config->contents().at(0).config()->contents().at(0).setting()->value(), Value::String("VALUE"));
}

