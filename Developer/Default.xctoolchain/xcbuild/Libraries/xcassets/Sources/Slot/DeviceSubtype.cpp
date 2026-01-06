/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/DeviceSubtype.h>

#include <cstdlib>

using xcassets::Slot::DeviceSubtype;
using xcassets::Slot::DeviceSubtypes;

ext::optional<DeviceSubtype> DeviceSubtypes::
Parse(std::string const &value)
{
    if (value == "retina4") {
        return DeviceSubtype::Retina4;
    } else if (value == "667h") {
        return DeviceSubtype::Height667;
    } else if (value == "736h") {
        return DeviceSubtype::Height736;
    } else {
        fprintf(stderr, "warning: unknown device subtype '%s'\n", value.c_str());
        return ext::nullopt;
    }
}

std::string DeviceSubtypes::
String(DeviceSubtype deviceSubtype)
{
    switch (deviceSubtype) {
        case DeviceSubtype::Retina4:
            return "retina4";
        case DeviceSubtype::Height667:
            return "667h";
        case DeviceSubtype::Height736:
            return "736h";
    }

    abort();
}

