/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/TextureOrigin.h>

#include <cstdlib>

using xcassets::TextureOrigin;
using xcassets::TextureOrigins;

ext::optional<TextureOrigin> TextureOrigins::
Parse(std::string const &value)
{
    if (value == "bottom-left") {
        return TextureOrigin::BottomLeft;
    } else {
        fprintf(stderr, "warning: unknown texture origin %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string TextureOrigins::
String(TextureOrigin textureOrigin)
{
    switch (textureOrigin) {
        case TextureOrigin::BottomLeft:
            return "bottom-left";
    }

    abort();
}
