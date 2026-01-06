/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/MipmapLevelMode.h>

#include <cstdlib>

using xcassets::MipmapLevelMode;
using xcassets::MipmapLevelModes;

ext::optional<MipmapLevelMode> MipmapLevelModes::
Parse(std::string const &value)
{
    if (value == "none") {
        return MipmapLevelMode::None;
    } else if (value == "all") {
        return MipmapLevelMode::All;
    } else if (value == "fixed") {
        return MipmapLevelMode::Fixed;
    } else {
        fprintf(stderr, "warning: unknown mipmap level mode %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string MipmapLevelModes::
String(MipmapLevelMode mipmapLevelMode)
{
    switch (mipmapLevelMode) {
        case MipmapLevelMode::None:
            return "none";
        case MipmapLevelMode::All:
            return "all";
        case MipmapLevelMode::Fixed:
            return "fixed";
    }

    abort();
}
