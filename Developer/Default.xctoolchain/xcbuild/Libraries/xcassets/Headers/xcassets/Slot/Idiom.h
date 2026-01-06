/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_Idiom_h
#define __xcassets_Slot_Idiom_h

#include <ext/optional>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * A class of device interaction.
 */
enum class Idiom {
    Universal,
    Phone,
    Pad,
    Desktop,
    TV,
    Watch,
    Car,
    iOSMarketing,
};

class Idioms {
private:
    Idioms();
    ~Idioms();

public:
    /*
     * Parse a matching idiom from a string, if valid.
     */
    static ext::optional<Idiom> Parse(std::string const &value);

    /*
     * Convert an idiom to a string.
     */
    static std::string String(Idiom idiom);
};

}
}

#endif // !__xcassets_Slot_Idiom_h
