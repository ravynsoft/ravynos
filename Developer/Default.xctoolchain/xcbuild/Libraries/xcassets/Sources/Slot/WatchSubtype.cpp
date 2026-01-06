/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/WatchSubtype.h>

#include <cstdlib>

using xcassets::Slot::WatchSubtype;
using xcassets::Slot::WatchSubtypes;

ext::optional<WatchSubtype> WatchSubtypes::
ParsePhysicalSize(std::string const &value)
{
    if (value == "38mm") {
        return WatchSubtype::Small;
    } else if (value == "42mm") {
        return WatchSubtype::Large;
    } else {
        fprintf(stderr, "warning: unknown watch physical size %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string WatchSubtypes::
PhysicalSizeString(WatchSubtype watchSubtype)
{
    switch (watchSubtype) {
        case WatchSubtype::Small:
            return "38mm";
        case WatchSubtype::Large:
            return "42mm";
    }

    abort();
}

ext::optional<WatchSubtype> WatchSubtypes::
ParseScreenWidth(std::string const &value)
{
    if (value == "<=145") {
        return WatchSubtype::Small;
    } else if (value == ">145") {
        return WatchSubtype::Large;
    } else {
        fprintf(stderr, "warning: unknown watch screen width %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string WatchSubtypes::
ScreenWidthString(WatchSubtype watchSubtype)
{
    switch (watchSubtype) {
        case WatchSubtype::Small:
            return "<=145";
        case WatchSubtype::Large:
            return ">145";
    }

    abort();
}

