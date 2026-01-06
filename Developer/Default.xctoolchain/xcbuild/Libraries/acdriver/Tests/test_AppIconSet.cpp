/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <acdriver/Compile/AppIconSet.h>
#include <acdriver/Compile/Output.h>
#include <acdriver/Result.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <libutil/Filesystem.h>
#include <libutil/MemoryFilesystem.h>

using acdriver::Compile::AppIconSet;
using acdriver::Compile::Output;
using acdriver::Result;
using libutil::Filesystem;
using libutil::MemoryFilesystem;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

#define CONTENTS(...) Contents(#__VA_ARGS__)

static void
VerifyIcons(plist::Dictionary *info, std::string const &key, std::vector<std::string> const &iconFiles)
{
    plist::Dictionary *icons = info->value<plist::Dictionary>(key);
    ASSERT_NE(icons, nullptr);
    EXPECT_EQ(icons->count(), 1);

    plist::Dictionary *primary = icons->value<plist::Dictionary>("CFBundlePrimaryIcon");
    ASSERT_NE(primary, nullptr);
    EXPECT_EQ(primary->count(), 1);

    plist::Array *files = primary->value<plist::Array>("CFBundleIconFiles");
    ASSERT_NE(files, nullptr);
    ASSERT_EQ(files->count(), iconFiles.size());

    for (size_t i = 0; i < files->count(); i++) {
        plist::String *first = files->value<plist::String>(i);
        ASSERT_NE(first, nullptr);
        EXPECT_EQ(first->value(), iconFiles[i]);
    }
}

TEST(AppIconSet, Compile)
{
    /* Define asset. */
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::Directory("AppIcon.appiconset", {
            MemoryFilesystem::Entry::File("Contents.json", CONTENTS({
                "images" : [
                    {
                        "size" : "29x29",
                        "idiom" : "iphone",
                        "filename" : "small.png",
                        "scale" : "2x"
                    },
                    {
                        "size" : "60x60",
                        "idiom" : "iphone",
                        "filename" : "two.png",
                        "scale" : "2x"
                    },
                    {
                        "size" : "60x60",
                        "idiom" : "iphone",
                        "filename" : "three.png",
                        "scale" : "3x"
                    },
                    {
                      "size" : "76x76",
                      "idiom" : "ipad",
                      "filename" : "pad.png",
                      "scale" : "1x"
                    },
                ],
                "info" : {
                    "version" : 1,
                    "author" : "xcode"
                }
            })),
            MemoryFilesystem::Entry::File("small.png", Contents("2x")),
            MemoryFilesystem::Entry::File("two.png", Contents("2x")),
            MemoryFilesystem::Entry::File("three.png", Contents("3x")),
            MemoryFilesystem::Entry::File("pad.png", Contents("pad")),
        }),
    });

    /* Load asset. */
    auto asset = xcassets::Asset::Asset::Load(
        &filesystem,
        filesystem.path("AppIcon.appiconset"),
        { },
        xcassets::Asset::AppIconSet::Extension());
    auto appIconSet = libutil::static_unique_pointer_cast<xcassets::Asset::AppIconSet>(std::move(asset));
    ASSERT_NE(appIconSet, nullptr);

    /* Compile asset. */
    Result result;
    Output output = Output(filesystem.path("output"), Output::Format::Compiled, std::string("AppIcon"), ext::nullopt);
    ASSERT_TRUE(AppIconSet::Compile(appIconSet.get(), &output, &result));
    EXPECT_TRUE(result.success());

    /* Should copy images to output. */
    using Copy = std::pair<std::string, std::string>;
    EXPECT_EQ(output.copies(), std::vector<Copy>({
        { filesystem.path("AppIcon.appiconset/small.png"), filesystem.path("output/AppIcon29x29@2x.png") },
        { filesystem.path("AppIcon.appiconset/two.png"), filesystem.path("output/AppIcon60x60@2x.png") },
        { filesystem.path("AppIcon.appiconset/three.png"), filesystem.path("output/AppIcon60x60@3x.png") },
        { filesystem.path("AppIcon.appiconset/pad.png"), filesystem.path("output/AppIcon76x76~ipad.png") },
    }));

    /* Should note outputs. Only the root asset catalog is an input. */
    EXPECT_EQ(output.inputs(), std::vector<std::string>());
    EXPECT_EQ(output.outputs(), std::vector<std::string>({
        filesystem.path("output/AppIcon29x29@2x.png"),
        filesystem.path("output/AppIcon60x60@2x.png"),
        filesystem.path("output/AppIcon60x60@3x.png"),
        filesystem.path("output/AppIcon76x76~ipad.png"),
    }));

    /* Should generate additional info. */
    plist::Dictionary *info = output.additionalInfo();
    EXPECT_EQ(info->count(), 2);
    VerifyIcons(info, "CFBundleIcons", { "AppIcon29x29", "AppIcon60x60" });
    VerifyIcons(info, "CFBundleIcons~ipad", { "AppIcon76x76" });
}

