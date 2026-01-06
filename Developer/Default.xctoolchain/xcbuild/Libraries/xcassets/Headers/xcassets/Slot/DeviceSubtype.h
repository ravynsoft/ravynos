/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_DeviceSubtype_h
#define __xcassets_Slot_DeviceSubtype_h

#include <ext/optional>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * The subtype of device a launch image is for.
 */
enum class DeviceSubtype {
    Retina4,
    Height667,
    Height736,
};

class DeviceSubtypes {
private:
    DeviceSubtypes();
    ~DeviceSubtypes();

public:
    /*
     * Parse a device subtype from a string.
     */
    static ext::optional<DeviceSubtype> Parse(std::string const &value);

    /*
     * Convert a device subtype to a string.
     */
    static std::string String(DeviceSubtype deviceSubtype);
};

}
}

#endif // !__xcassets_Slot_DeviceSubtype_h
