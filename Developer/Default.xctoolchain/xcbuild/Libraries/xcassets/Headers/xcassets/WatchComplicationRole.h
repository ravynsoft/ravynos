/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_WatchComplicationRole_h
#define __xcassets_WatchComplicationRole_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * How an asset in a watch complication is used.
 */
enum class WatchComplicationRole {
    Circular,
    Modular,
    Utilitarian,
};

class WatchComplicationRoles {
private:
    WatchComplicationRoles();
    ~WatchComplicationRoles();

public:
    /*
     * Parse a watch complication role string.
     */
    static ext::optional<WatchComplicationRole> Parse(std::string const &value);

    /*
     * String representation of a watch complication role.
     */
    static std::string String(WatchComplicationRole watchComplicationRole);
};

}

#endif // !__xcassets_WatchComplicationRole_h
