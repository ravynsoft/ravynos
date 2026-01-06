/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <acdriver/Compile/LaunchImage.h>
#include <acdriver/Compile/Output.h>
#include <acdriver/Result.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <libutil/Filesystem.h>
#include <libutil/MemoryFilesystem.h>

using acdriver::Compile::LaunchImage;
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
VerifyLaunchImage(plist::Array const *images, size_t index, std::string const &name, std::string const &minimumVersion, std::string const &orientation, std::string const &size)
{
    plist::Dictionary const *image = images->value<plist::Dictionary>(index);
    ASSERT_NE(image, nullptr);
    EXPECT_EQ(image->count(), 4);

    plist::String const *imageName = image->value<plist::String>("UILaunchImageName");
    ASSERT_NE(imageName, nullptr);
    EXPECT_EQ(imageName->value(), name);

    plist::String const *imageVersion = image->value<plist::String>("UILaunchImageMinimumOSVersion");
    ASSERT_NE(imageVersion, nullptr);
    EXPECT_EQ(imageVersion->value(), minimumVersion);

    plist::String const *imageOrientation = image->value<plist::String>("UILaunchImageOrientation");
    ASSERT_NE(imageOrientation, nullptr);
    EXPECT_EQ(imageOrientation->value(), orientation);

    plist::String const *imageSize = image->value<plist::String>("UILaunchImageSize");
    ASSERT_NE(imageSize, nullptr);
    EXPECT_EQ(imageSize->value(), size);
}

TEST(LaunchImage, Compile)
{
    /* Define asset. */
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::Directory("LaunchImage.launchimage", {
            MemoryFilesystem::Entry::File("Contents.json", CONTENTS({
                "images" : [
                    {
                        "filename" : "phone.png",
                        "idiom" : "iphone",
                        "orientation" : "portrait",
                        "extent" : "to-status-bar",
                        "minimum-system-version" : "7.0",
                        "scale" : "1x"
                    },
                    {
                        "filename" : "giraffe.png",
                        "idiom" : "iphone",
                        "subtype" : "retina4",
                        "orientation" : "portrait",
                        "extent" : "full-screen",
                        "minimum-system-version" : "8.0",
                        "scale" : "2x"
                    },
                    {
                        "filename" : "phone-landscape.png",
                        "idiom" : "iphone",
                        "subtype" : "736h",
                        "orientation" : "landscape",
                        "extent" : "full-screen",
                        "minimum-system-version" : "8.0",
                        "scale" : "3x"
                    },
                    {
                        "filename" : "pad-portrait.png",
                        "idiom" : "ipad",
                        "orientation" : "portrait",
                        "extent" : "full-screen",
                        "minimum-system-version" : "7.0",
                        "scale" : "1x"
                    },
                    {
                        "filename" : "pad-landscape.png",
                        "idiom" : "ipad",
                        "orientation" : "landscape",
                        "extent" : "full-screen",
                        "minimum-system-version" : "7.0",
                        "scale" : "1x"
                    },
                ],
                "info" : {
                    "version" : 1,
                    "author" : "xcode"
                }
            })),
            MemoryFilesystem::Entry::File("phone.png", Contents("phone")),
            MemoryFilesystem::Entry::File("giraffe.png", Contents("giraffe")),
            MemoryFilesystem::Entry::File("phone-landscape.png", Contents("phone-landscape")),
            MemoryFilesystem::Entry::File("pad-portrait.png", Contents("pad-portrait")),
            MemoryFilesystem::Entry::File("pad-landscape.png", Contents("pad-landscape")),
        }),
    });

    /* Load asset. */
    auto asset = xcassets::Asset::Asset::Load(
        &filesystem,
        filesystem.path("LaunchImage.launchimage"),
        { },
        xcassets::Asset::LaunchImage::Extension());
    auto launchImage = libutil::static_unique_pointer_cast<xcassets::Asset::LaunchImage>(std::move(asset));
    ASSERT_NE(launchImage, nullptr);

    /* Compile asset. */
    Result result;
    Output output = Output(filesystem.path("output"), Output::Format::Compiled, ext::nullopt, std::string("LaunchImage"));
    ASSERT_TRUE(LaunchImage::Compile(launchImage.get(), &output, &result));
    EXPECT_TRUE(result.success());

    /* Should copy images to output. */
    using Copy = std::pair<std::string, std::string>;
    EXPECT_EQ(output.copies(), std::vector<Copy>({
        { filesystem.path("LaunchImage.launchimage/phone.png"), filesystem.path("output/LaunchImage-700.png") },
        { filesystem.path("LaunchImage.launchimage/giraffe.png"), filesystem.path("output/LaunchImage-800-568h@2x.png") },
        { filesystem.path("LaunchImage.launchimage/phone-landscape.png"), filesystem.path("output/LaunchImage-800-Landscape-736h@3x.png") },
        { filesystem.path("LaunchImage.launchimage/pad-portrait.png"), filesystem.path("output/LaunchImage-700-Portrait~ipad.png") },
        { filesystem.path("LaunchImage.launchimage/pad-landscape.png"), filesystem.path("output/LaunchImage-700-Landscape~ipad.png") },
    }));

    /* Should note outputs. Only the root asset catalog is an input. */
    EXPECT_EQ(output.inputs(), std::vector<std::string>());
    EXPECT_EQ(output.outputs(), std::vector<std::string>({
        filesystem.path("output/LaunchImage-700.png"),
        filesystem.path("output/LaunchImage-800-568h@2x.png"),
        filesystem.path("output/LaunchImage-800-Landscape-736h@3x.png"),
        filesystem.path("output/LaunchImage-700-Portrait~ipad.png"),
        filesystem.path("output/LaunchImage-700-Landscape~ipad.png"),
    }));

    /* Should generate additional info. */
    plist::Dictionary *info = output.additionalInfo();
    EXPECT_EQ(info->count(), 1);

    plist::Array *images = info->value<plist::Array>("UILaunchImages");
    ASSERT_NE(images, nullptr);
    EXPECT_EQ(images->count(), 5);

    VerifyLaunchImage(images, 0, "LaunchImage-700", "7.0", "Portrait", "{320, 460}");
    VerifyLaunchImage(images, 1, "LaunchImage-800-568h", "8.0", "Portrait", "{320, 568}");
    VerifyLaunchImage(images, 2, "LaunchImage-800-Landscape-736h", "8.0", "Landscape", "{414, 736}");
    VerifyLaunchImage(images, 3, "LaunchImage-700-Portrait", "7.0", "Portrait", "{768, 1024}");
    VerifyLaunchImage(images, 4, "LaunchImage-700-Landscape", "7.0", "Landscape", "{768, 1024}");
}

TEST(LaunchImage, CompileLegacy)
{
    /* Define asset. */
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::Directory("LaunchImage.launchimage", {
            MemoryFilesystem::Entry::File("Contents.json", CONTENTS({
                "images" : [
                    {
                        "filename" : "legacy.png",
                        "idiom" : "iphone",
                        "orientation" : "portrait",
                        "extent" : "full-screen",
                        /* Legacy: no minimum system version. */
                        "scale" : "1x"
                    },
                    {
                        "filename" : "modern.png",
                        "idiom" : "iphone",
                        "orientation" : "portrait",
                        "extent" : "full-screen",
                        "minimum-system-version" : "7.0",
                        "scale" : "1x"
                    },
                ],
                "info" : {
                    "version" : 1,
                    "author" : "xcode"
                }
            })),
            MemoryFilesystem::Entry::File("legacy.png", Contents("legacy")),
            MemoryFilesystem::Entry::File("modern.png", Contents("modern")),
        }),
    });

    /* Load asset. */
    auto asset = xcassets::Asset::Asset::Load(
        &filesystem,
        filesystem.path("LaunchImage.launchimage"),
        { },
        xcassets::Asset::LaunchImage::Extension());
    auto launchImage = libutil::static_unique_pointer_cast<xcassets::Asset::LaunchImage>(std::move(asset));
    ASSERT_NE(launchImage, nullptr);

    /* Compile asset. */
    Result result;
    Output output = Output(filesystem.path("output"), Output::Format::Compiled, ext::nullopt, std::string("LaunchImage"));
    ASSERT_TRUE(LaunchImage::Compile(launchImage.get(), &output, &result));
    EXPECT_TRUE(result.success());

    /* Should copy images to output. */
    using Copy = std::pair<std::string, std::string>;
    EXPECT_EQ(output.copies(), std::vector<Copy>({
        { filesystem.path("LaunchImage.launchimage/legacy.png"), filesystem.path("output/LaunchImage.png") },
        { filesystem.path("LaunchImage.launchimage/modern.png"), filesystem.path("output/LaunchImage-700.png") },
    }));

    /* Should note outputs. Only the root asset catalog is an input. */
    EXPECT_EQ(output.inputs(), std::vector<std::string>());
    EXPECT_EQ(output.outputs(), std::vector<std::string>({
        filesystem.path("output/LaunchImage.png"),
        filesystem.path("output/LaunchImage-700.png"),
    }));

    /* Should generate additional info, including legacy info. */
    plist::Dictionary const *info = output.additionalInfo();
    EXPECT_EQ(info->count(), 2);

    plist::Array const *images = info->value<plist::Array>("UILaunchImages");
    ASSERT_NE(images, nullptr);
    EXPECT_EQ(images->count(), 1);
    VerifyLaunchImage(images, 0, "LaunchImage-700", "7.0", "Portrait", "{320, 480}");

    plist::String const *imageName = info->value<plist::String>("UILaunchImageName");
    ASSERT_NE(imageName, nullptr);
    EXPECT_EQ(imageName->value(), "LaunchImage");
}

