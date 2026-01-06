/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/MemoryRequirement.h>

#include <cstdlib>

using xcassets::Slot::MemoryRequirement;
using xcassets::Slot::MemoryRequirements;

ext::optional<MemoryRequirement> MemoryRequirements::
Parse(std::string const &value)
{
    if (value == "1GB") {
        return MemoryRequirement::Minimum1GB;
    } else if (value == "2GB") {
        return MemoryRequirement::Minimum2GB;
    } else if (value == "4GB") {
        return MemoryRequirement::Minimum4GB;
    } else {
        fprintf(stderr, "warning: unknown memory requirement %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string MemoryRequirements::
String(MemoryRequirement memoryRequirement)
{
    switch (memoryRequirement) {
        case MemoryRequirement::Minimum1GB:
            return "1GB";
        case MemoryRequirement::Minimum2GB:
            return "2GB";
        case MemoryRequirement::Minimum4GB:
            return "4GB";
    }

    abort();
}
