/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_GraphicsFeatureSet_h
#define __xcassets_Slot_GraphicsFeatureSet_h

#include <ext/optional>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * Available graphics features.
 */
enum class GraphicsFeatureSet {
    MetalFamily1Version1,
    MetalFamily1Version2,
    MetalFamily2Version1,
    MetalFamily2Version2,
    MetalFamily3Version1,
};

class GraphicsFeatureSets {
private:
    GraphicsFeatureSets();
    ~GraphicsFeatureSets();

public:
    /*
     * Parse a matching graphics feature set from a string, if valid.
     */
    static ext::optional<GraphicsFeatureSet> Parse(std::string const &value);

    /*
     * Convert an graphics feature set to a string.
     */
    static std::string String(GraphicsFeatureSet graphicsFeatureSet);
};

}
}

#endif // !__xcassets_Slot_GraphicsFeatureSet_h
