/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_LaunchImageExtent_h
#define __xcassets_Slot_LaunchImageExtent_h

#include <ext/optional>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * The visual extent of a launch image.
 */
enum class LaunchImageExtent {
    ToStatusBar,
    FullScreen,
};

class LaunchImageExtents {
private:
    LaunchImageExtents();
    ~LaunchImageExtents();

public:
    /*
     * Parse an extent from a string.
     */
    static ext::optional<LaunchImageExtent> Parse(std::string const &value);

    /*
     * Convert an extent to a string.
     */
    static std::string String(LaunchImageExtent extent);
};

}
}

#endif // !__xcassets_Slot_LaunchImageExtent_h
