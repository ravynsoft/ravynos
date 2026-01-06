/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/ColorSpace.h>

#include <cstdlib>

using xcassets::Slot::ColorSpace;
using xcassets::Slot::ColorSpaces;

ext::optional<ColorSpace> ColorSpaces::
Parse(std::string const &value)
{
    if (value == "sRGB") {
        return ColorSpace::sRGB;
    } else if (value == "display-P3") {
        return ColorSpace::DisplayP3;
    } else {
        fprintf(stderr, "warning: unknown platform %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string ColorSpaces::
String(ColorSpace platform)
{
    switch (platform) {
        case ColorSpace::sRGB:
            return "sRGB";
        case ColorSpace::DisplayP3:
            return "display-P3";
    }

    abort();
}
