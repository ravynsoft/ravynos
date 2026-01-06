/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/WatchIconRole.h>

#include <cstdlib>

using xcassets::Slot::WatchIconRole;
using xcassets::Slot::WatchIconRoles;

ext::optional<WatchIconRole> WatchIconRoles::
Parse(std::string const &value)
{
    if (value == "notificationCenter") {
        return WatchIconRole::NotificationCenter;
    } else if (value == "companionSettings") {
        return WatchIconRole::CompanionSettings;
    } else if (value == "appLauncher") {
        return WatchIconRole::AppLauncher;
    } else if (value == "longLook") {
        return WatchIconRole::LongLookNotification;
    } else if (value == "quickLook") {
        return WatchIconRole::ShortLookNotification;
    } else {
        fprintf(stderr, "warning: unknown watch icon role %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string WatchIconRoles::
String(WatchIconRole watchIconRole)
{
    switch (watchIconRole) {
        case WatchIconRole::NotificationCenter:
            return "notificationCenter";
        case WatchIconRole::CompanionSettings:
            return "companionSettings";
        case WatchIconRole::AppLauncher:
            return "appLauncher";
        case WatchIconRole::LongLookNotification:
            return "longLook";
        case WatchIconRole::ShortLookNotification:
            return "quickLook";
    }

    abort();
}
