/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Compile/LaunchImage.h>
#include <acdriver/Compile/Convert.h>
#include <acdriver/Compile/Output.h>
#include <acdriver/Result.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using acdriver::Compile::LaunchImage;
using acdriver::Compile::Convert;
using acdriver::Compile::Output;
using acdriver::Result;

static std::string
LaunchImageSystemVersionValue(xcassets::Slot::SystemVersion const &version)
{
    std::string name;
    name += std::to_string(version.major());
    name += ".";
    name += std::to_string(version.minor());
    if (version.patch()) {
        name += ".";
        name += std::to_string(*version.patch());
    }
    return name;
}

static std::string
LaunchImageSystemVersionSuffix(xcassets::Slot::SystemVersion const &version)
{
    std::string suffix = "-";
    suffix += std::to_string(version.major());
    suffix += std::to_string(version.minor());
    suffix += std::to_string(version.patch().value_or(0));
    return suffix;
}

static std::string
LaunchImageOrientationValue(xcassets::Slot::Orientation orientation)
{
    switch (orientation) {
        case xcassets::Slot::Orientation::Portrait:
            return "Portrait";
        case xcassets::Slot::Orientation::Landscape:
            return "Landscape";
    }

    abort();
}

static std::string
LaunchImageOrientationSuffix(xcassets::Slot::Orientation orientation)
{
    return "-" + LaunchImageOrientationValue(orientation);
}

static bool
LaunchImageSupportsLandscape(xcassets::Slot::Idiom idiom, ext::optional<xcassets::Slot::DeviceSubtype> subtype)
{
    switch (idiom) {
        case xcassets::Slot::Idiom::Universal:
            return false;
        case xcassets::Slot::Idiom::Phone:
            if (!subtype) {
                return false;
            } else {
                switch (*subtype) {
                    case xcassets::Slot::DeviceSubtype::Retina4:
                        return false;
                    case xcassets::Slot::DeviceSubtype::Height667:
                        return false;
                    case xcassets::Slot::DeviceSubtype::Height736:
                        return true;
                }

                abort();
            }
        case xcassets::Slot::Idiom::Pad:
            return true;
        case xcassets::Slot::Idiom::Desktop:
            return false;
        case xcassets::Slot::Idiom::TV:
            return false;
        case xcassets::Slot::Idiom::Watch:
            return false;
        case xcassets::Slot::Idiom::Car:
            return false;
        case xcassets::Slot::Idiom::iOSMarketing:
            return false;
    }

    abort();
}

static bool
LaunchImageScreenSize(xcassets::Slot::Idiom idiom, ext::optional<xcassets::Slot::DeviceSubtype> subtype, size_t *width, size_t *height)
{
    if (width && height) {
        switch (idiom) {
            case xcassets::Slot::Idiom::Universal:
                return false;
            case xcassets::Slot::Idiom::Phone:
                if (!subtype) {
                    *width = 320;
                    *height = 480;
                    return true;
                } else {
                    switch (*subtype) {
                        case xcassets::Slot::DeviceSubtype::Retina4:
                            *width = 320;
                            *height = 568;
                            return true;
                        case xcassets::Slot::DeviceSubtype::Height667:
                            *width = 375;
                            *height = 667;
                            return true;
                        case xcassets::Slot::DeviceSubtype::Height736:
                            *width = 414;
                            *height = 736;
                            return true;
                    }

                    abort();
                }
            case xcassets::Slot::Idiom::Pad:
                /* Note there is no subtype for larger sizes. */
                if (subtype) {
                    return false;
                }
                *width = 768;
                *height = 1024;
                return true;
            case xcassets::Slot::Idiom::Desktop:
                return false;
            case xcassets::Slot::Idiom::TV:
                if (subtype) {
                    return false;
                }
                *width = 1920;
                *height = 1080;
                return true;
            case xcassets::Slot::Idiom::Watch:
                return false;
            case xcassets::Slot::Idiom::Car:
                return false;
            case xcassets::Slot::Idiom::iOSMarketing:
                return false;
        }
    }

    abort();
}

static void
LaunchImageExtentSize(size_t w, size_t h, xcassets::Slot::LaunchImageExtent extent, xcassets::Slot::Orientation orientation, size_t *width, size_t *height)
{
    static size_t const StatusBarHeight = 20;
    if (width && height) {
        switch (extent) {
            case xcassets::Slot::LaunchImageExtent::ToStatusBar:
                switch (orientation) {
                    case xcassets::Slot::Orientation::Portrait:
                        /* Subtract from height. */
                        *width = w;
                        *height = h - StatusBarHeight;
                        return;
                    case xcassets::Slot::Orientation::Landscape:
                        /* Subtract from width. */
                        *width = w - StatusBarHeight;
                        *height = h;
                        return;
                }

                abort();
            case xcassets::Slot::LaunchImageExtent::FullScreen:
                /* No changes needed. */
                *width = w;
                *height = h;
                return;
        }
    }

    abort();
}

static std::string
ImageSizeValue(size_t width, size_t height)
{
    return "{" + std::to_string(width) + ", " + std::to_string(height) + "}";
}

bool LaunchImage::
Compile(
    xcassets::Asset::LaunchImage const *launchImage,
    Output *compileOutput,
    Result *result)
{
    auto images = plist::Array::New();
    bool useLegacyLaunchImageName = false;

    /*
     * Copy the launch images into the output.
     */
    if (launchImage->images()) {
        for (xcassets::Asset::LaunchImage::Image const &image : *launchImage->images()) {
            /*
             * Verify the image has the required information.
             */
            // TODO: what information is required?
            if (!image.fileName() || !image.idiom()) {
                result->document(
                    Result::Severity::Warning,
                    launchImage->path(),
                    { Output::AssetReference(launchImage) },
                    "Ambiguous Content",
                    "a launch image in \"" + launchImage->name().name() + "\" is unassigned");
                continue;
            }

            // TODO: skip images with idioms inappropriate for target platform

            /*
             * Get the expected size for the launch image.
             */
            // TODO: verify image size matches expected
            size_t width = 0;
            size_t height = 0;
            if (!LaunchImageScreenSize(*image.idiom(), image.subtype(), &width, &height)) {
                result->document(
                    Result::Severity::Error,
                    launchImage->path(),
                    { Output::AssetReference(launchImage) },
                    "Invalid Content",
                    "a launch image in \"" + launchImage->name().name() + "\" has an invalid idiom and/or subtype");
                continue;
            }

            /* Adjust size for extent; i.e., the status bar. */
            if (image.extent()) {
                xcassets::Slot::Orientation orientation = image.orientation().value_or(xcassets::Slot::Orientation::Portrait);
                LaunchImageExtentSize(width, height, *image.extent(), orientation, &width, &height);
            }

            /*
             * Determine the name of the launch image file.
             */
            std::string name = launchImage->name().name();
            if (image.minimumSystemVersion()) {
                name += LaunchImageSystemVersionSuffix(*image.minimumSystemVersion());
            }
            if (image.orientation()) {
                /* If landscape isn't supported, there's no need to disambiguate orientation. */
                if (LaunchImageSupportsLandscape(*image.idiom(), image.subtype())) {
                    name += LaunchImageOrientationSuffix(*image.orientation());
                }
            }
            if (image.subtype()) {
                name += Convert::DeviceSubtypeSuffix(*image.subtype());
            }

            /*
             * Copy the launch image into the output.
             */
            std::string source = launchImage->path() + "/" + *image.fileName();

            std::string destination = compileOutput->root() + "/" + name;
            if (image.scale()) {
                destination += Convert::ScaleSuffix(*image.scale());
            }
            destination += Convert::IdiomSuffix(*image.idiom());
            destination += ".png";

            compileOutput->copies().push_back({ source, destination });
            compileOutput->outputs().push_back(destination);

            /*
             * Configure the image info. This is only relevant if there is a minimum system
             * version of at least 7, as earlier versions don't support the `UILaunchImages` key.
             */
            if (image.minimumSystemVersion() && image.minimumSystemVersion()->major() >= 7) {
                auto info = plist::Dictionary::New();

                std::string minimumSystemVersionValue = LaunchImageSystemVersionValue(*image.minimumSystemVersion());
                info->set("UILaunchImageMinimumOSVersion", plist::String::New(minimumSystemVersionValue));

                info->set("UILaunchImageName", plist::String::New(name));

                if (image.orientation()) {
                    std::string orientationValue = LaunchImageOrientationValue(*image.orientation());
                    info->set("UILaunchImageOrientation", plist::String::New(orientationValue));
                }

                std::string imageSizeValue = ImageSizeValue(width, height);
                info->set("UILaunchImageSize", plist::String::New(imageSizeValue));

                images->append(std::move(info));
            } else {
                useLegacyLaunchImageName = true;
            }
        }

        /*
         * Add launch image information.
         */
        plist::Dictionary *info = compileOutput->additionalInfo();
        if (useLegacyLaunchImageName) {
            info->set("UILaunchImageName", plist::String::New(launchImage->name().name()));
        }
        if (images->count() > 0) {
            info->set("UILaunchImages", std::move(images));
        }
    }

    return true;
}
