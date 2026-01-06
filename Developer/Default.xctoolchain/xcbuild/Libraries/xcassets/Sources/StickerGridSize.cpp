/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/StickerGridSize.h>

#include <cstdlib>

using xcassets::StickerGridSize;
using xcassets::StickerGridSizes;

ext::optional<StickerGridSize> StickerGridSizes::
Parse(std::string const &value)
{
    if (value == "small") {
        return StickerGridSize::Small;
    } else if (value == "regular") {
        return StickerGridSize::Regular;
    } else if (value == "large") {
        return StickerGridSize::Large;
    } else {
        fprintf(stderr, "warning: unknown sticker grid size %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string StickerGridSizes::
String(StickerGridSize stickerGridSize)
{
    switch (stickerGridSize) {
        case StickerGridSize::Small:
            return "small";
        case StickerGridSize::Regular:
            return "regular";
        case StickerGridSize::Large:
            return "large";
    }

    abort();
}
