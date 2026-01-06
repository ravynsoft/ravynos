/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_MemoryRequirement_h
#define __xcassets_Slot_MemoryRequirement_h

#include <ext/optional>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * The minimum required amount of memory.
 */
enum class MemoryRequirement {
    Minimum1GB,
    Minimum2GB,
    Minimum4GB,
};

class MemoryRequirements {
private:
    MemoryRequirements();
    ~MemoryRequirements();

public:
    /*
     * Parse a matching memory requirement from a string, if valid.
     */
    static ext::optional<MemoryRequirement> Parse(std::string const &value);

    /*
     * Convert an memory requirement to a string.
     */
    static std::string String(MemoryRequirement memoryRequirement);
};

}
}

#endif // !__xcassets_Slot_MemoryRequirement_h
