/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/StickerDurationType.h>

#include <cstdlib>

using xcassets::StickerDurationType;
using xcassets::StickerDurationTypes;

ext::optional<StickerDurationType> StickerDurationTypes::
Parse(std::string const &value)
{
    if (value == "fixed") {
        return StickerDurationType::Fixed;
    } else if (value == "fps") {
        return StickerDurationType::FPS;
    } else {
        fprintf(stderr, "warning: unknown sticker duration type %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string StickerDurationTypes::
String(StickerDurationType stickerDurationType)
{
    switch (stickerDurationType) {
        case StickerDurationType::Fixed:
            return "fixed";
        case StickerDurationType::FPS:
            return "fps";
    }

    abort();
}

