/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_WatchSubtype_h
#define __xcassets_Slot_WatchSubtype_h

#include <ext/optional>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * The subtype of a watch.
 */
enum class WatchSubtype {
    Small,
    Large,
};

class WatchSubtypes {
private:
    WatchSubtypes();
    ~WatchSubtypes();

public:
    /*
     * Parse a matching watch subtype physical size from a string, if valid.
     */
    static ext::optional<WatchSubtype> ParsePhysicalSize(std::string const &value);

    /*
     * Convert a watch subtype to a physical size string.
     */
    static std::string PhysicalSizeString(WatchSubtype watchSubtype);

public:
    /*
     * Parse a matching watch subtype screen width from a string, if valid.
     */
    static ext::optional<WatchSubtype> ParseScreenWidth(std::string const &value);

    /*
     * Convert a watch subtype to a screen width string.
     */
    static std::string ScreenWidthString(WatchSubtype watchSubtype);
};

}
}

#endif // !__xcassets_Slot_WatchSubtype_h

