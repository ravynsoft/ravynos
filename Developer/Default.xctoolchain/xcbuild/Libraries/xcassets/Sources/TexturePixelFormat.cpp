/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/TexturePixelFormat.h>

#include <cstdlib>

using xcassets::TexturePixelFormat;
using xcassets::TexturePixelFormats;

ext::optional<TexturePixelFormat> TexturePixelFormats::
Parse(std::string const &value)
{
    if (value == "r-8-unorm") {
        return TexturePixelFormat::NormalizedR8;
    } else if (value == "rg-8-unorm") {
        return TexturePixelFormat::NormalizedRG8;
    } else if (value == "rgba-8-unorm") {
        return TexturePixelFormat::NormalizedRGBA8;
    } else if (value == "rgba-8-unorm-sRGB") {
        return TexturePixelFormat::NormalizedRGBA8sRGB;
    } else if (value == "r-16-float") {
        return TexturePixelFormat::FloatingR16;
    } else if (value == "rg-16-float") {
        return TexturePixelFormat::FloatingRG16;
    } else if (value == "rgba-16-float") {
        return TexturePixelFormat::FloatingRGBA16;
    } else if (value == "rbg-10-extended-range-sRGB") {
        return TexturePixelFormat::ExtendedRangeRBG10sRGB;
    } else if (value == "astc-4x4") {
        return TexturePixelFormat::ASTC4x4;
    } else if (value == "astc-4x4-sRGB") {
        return TexturePixelFormat::ASTC4x4sRGB;
    } else if (value == "astc-8x8") {
        return TexturePixelFormat::ASTC8x8;
    } else if (value == "astc-8x8-sRGB") {
        return TexturePixelFormat::ASTC8x8sRGB;
    } else {
        fprintf(stderr, "warning: unknown texture interpretation %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string TexturePixelFormats::
String(TexturePixelFormat texturePixelFormat)
{
    switch (texturePixelFormat) {
        case TexturePixelFormat::NormalizedR8:
            return "r-8-unorm";
        case TexturePixelFormat::NormalizedRG8:
            return "rg-8-unorm";
        case TexturePixelFormat::NormalizedRGBA8:
            return "rgba-8-unorm";
        case TexturePixelFormat::NormalizedRGBA8sRGB:
            return "rgba-8-unorm-sRGB";
        case TexturePixelFormat::FloatingR16:
            return "r-16-float";
        case TexturePixelFormat::FloatingRG16:
            return "rg-16-float";
        case TexturePixelFormat::FloatingRGBA16:
            return "rgba-16-float";
        case TexturePixelFormat::ExtendedRangeRBG10sRGB:
            return "rbg-10-extended-range-sRGB";
        case TexturePixelFormat::ASTC4x4:
            return "astc-4x4";
        case TexturePixelFormat::ASTC4x4sRGB:
            return "astc-4x4-sRGB";
        case TexturePixelFormat::ASTC8x8:
            return "astc-8x8";
        case TexturePixelFormat::ASTC8x8sRGB:
            return "astc-8x8-sRGB";
    }

    abort();
}
