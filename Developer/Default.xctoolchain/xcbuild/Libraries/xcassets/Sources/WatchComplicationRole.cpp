/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/WatchComplicationRole.h>

#include <cstdlib>

using xcassets::WatchComplicationRole;
using xcassets::WatchComplicationRoles;

ext::optional<WatchComplicationRole> WatchComplicationRoles::
Parse(std::string const &value)
{
    if (value == "circular") {
        return WatchComplicationRole::Circular;
    } else if (value == "modular") {
        return WatchComplicationRole::Modular;
    } else if (value == "utilitarian") {
        return WatchComplicationRole::Utilitarian;
    } else {
        fprintf(stderr, "warning: unknown watch complication role %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string WatchComplicationRoles::
String(WatchComplicationRole watchComplicationRole)
{
    switch (watchComplicationRole) {
        case WatchComplicationRole::Circular:
            return "circular";
        case WatchComplicationRole::Modular:
            return "modular";
        case WatchComplicationRole::Utilitarian:
            return "utilitarian";
    }

    abort();
}
