/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_WatchIconRole_h
#define __xcassets_Slot_WatchIconRole_h

#include <ext/optional>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * How an icon is used on a watch.
 */
enum class WatchIconRole {
    NotificationCenter,
    CompanionSettings,
    AppLauncher,
    LongLookNotification,
    ShortLookNotification,
};

class WatchIconRoles {
private:
    WatchIconRoles();
    ~WatchIconRoles();

public:
    /*
     * Parse a matching watch icon role from a string, if valid.
     */
    static ext::optional<WatchIconRole> Parse(std::string const &value);

    /*
     * Convert an watch icon role to a string.
     */
    static std::string String(WatchIconRole watchIconRole);
};

}
}

#endif // !__xcassets_Slot_WatchIconRole_h
