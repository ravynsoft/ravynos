/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_SizeClass_h
#define __xcassets_Slot_SizeClass_h

#include <ext/optional>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * A general class of the available screen space in a dimension.
 */
enum class SizeClass {
    Compact,
    Regular,
};

class SizeClasses {
private:
    SizeClasses();
    ~SizeClasses();

public:
    /*
     * Parse a matching size class from a string, if valid.
     */
    static ext::optional<SizeClass> Parse(std::string const &value);

    /*
     * Convert an size class to a string.
     */
    static std::string String(SizeClass sizeClass);
};

}
}

#endif // !__xcassets_Slot_SizeClass_h
