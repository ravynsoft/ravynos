/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/LaunchImageExtent.h>

#include <cstdlib>

using xcassets::Slot::LaunchImageExtent;
using xcassets::Slot::LaunchImageExtents;

ext::optional<LaunchImageExtent> LaunchImageExtents::
Parse(std::string const &value)
{
    if (value == "to-status-bar") {
        return LaunchImageExtent::ToStatusBar;
    } else if (value == "full-screen") {
        return LaunchImageExtent::FullScreen;
    } else {
        fprintf(stderr, "warning: unknown extent '%s'\n", value.c_str());
        return ext::nullopt;
    }
}

std::string LaunchImageExtents::
String(LaunchImageExtent extent)
{
    switch (extent) {
        case LaunchImageExtent::ToStatusBar:
            return "to-status-bar";
        case LaunchImageExtent::FullScreen:
            return "full-screen";
    }

    abort();
}

