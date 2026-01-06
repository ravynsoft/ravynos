/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/CubeFace.h>

#include <cstdlib>

using xcassets::CubeFace;
using xcassets::CubeFaces;

ext::optional<CubeFace> CubeFaces::
Parse(std::string const &value)
{
    if (value == "x-") {
        return CubeFace::NegativeX;
    } else if (value == "x+") {
        return CubeFace::PositiveX;
    } else if (value == "y-") {
        return CubeFace::NegativeY;
    } else if (value == "y+") {
        return CubeFace::PositiveY;
    } else if (value == "z-") {
        return CubeFace::NegativeZ;
    } else if (value == "z+") {
        return CubeFace::PositiveZ;
    } else {
        fprintf(stderr, "warning: unknown cube face %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string CubeFaces::
String(CubeFace cubeFace)
{
    switch (cubeFace) {
        case CubeFace::NegativeX:
            return "x-";
        case CubeFace::PositiveX:
            return "x+";
        case CubeFace::NegativeY:
            return "y-";
        case CubeFace::PositiveY:
            return "y+";
        case CubeFace::NegativeZ:
            return "z-";
        case CubeFace::PositiveZ:
            return "z+";
    }

    abort();
}
