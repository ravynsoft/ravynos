/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/TextureInterpretation.h>

#include <cstdlib>

using xcassets::TextureInterpretation;
using xcassets::TextureInterpretations;

ext::optional<TextureInterpretation> TextureInterpretations::
Parse(std::string const &value)
{
    if (value == "colors") {
        return TextureInterpretation::Colors;
    } else if (value == "data") {
        return TextureInterpretation::Data;
    } else {
        fprintf(stderr, "warning: unknown texture interpretation %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string TextureInterpretations::
String(TextureInterpretation textureInterpretation)
{
    switch (textureInterpretation) {
        case TextureInterpretation::Colors:
            return "colors";
        case TextureInterpretation::Data:
            return "data";
    }

    abort();
}
