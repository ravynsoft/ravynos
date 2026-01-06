/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/Orientation.h>

#include <cstdlib>

using xcassets::Slot::Orientation;
using xcassets::Slot::Orientations;

ext::optional<Orientation> Orientations::
Parse(std::string const &value)
{
    if (value == "portrait") {
        return Orientation::Portrait;
    } else if (value == "landscape") {
        return Orientation::Landscape;
    } else {
        fprintf(stderr, "warning: unknown orientation %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string Orientations::
String(Orientation orientation)
{
    switch (orientation) {
        case Orientation::Portrait:
            return "portrait";
        case Orientation::Landscape:
            return "landscape";
    }

    abort();
}
