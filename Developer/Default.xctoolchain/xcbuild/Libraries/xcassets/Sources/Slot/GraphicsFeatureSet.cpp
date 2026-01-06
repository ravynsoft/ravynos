/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/GraphicsFeatureSet.h>

#include <cstdlib>

using xcassets::Slot::GraphicsFeatureSet;
using xcassets::Slot::GraphicsFeatureSets;

ext::optional<GraphicsFeatureSet> GraphicsFeatureSets::
Parse(std::string const &value)
{
    if (value == "metal1v1") {
        return GraphicsFeatureSet::MetalFamily1Version1;
    } else if (value == "metal1v2") {
        return GraphicsFeatureSet::MetalFamily1Version2;
    } else if (value == "metal2v1") {
        return GraphicsFeatureSet::MetalFamily2Version1;
    } else if (value == "metal2v2") {
        return GraphicsFeatureSet::MetalFamily2Version2;
    } else if (value == "metal3v1") {
        return GraphicsFeatureSet::MetalFamily3Version1;
    } else {
        fprintf(stderr, "warning: unknown graphics feature set %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string GraphicsFeatureSets::
String(GraphicsFeatureSet graphicsFeatureSet)
{
    switch (graphicsFeatureSet) {
        case GraphicsFeatureSet::MetalFamily1Version1:
            return "metal1v1";
        case GraphicsFeatureSet::MetalFamily1Version2:
            return "metal1v2";
        case GraphicsFeatureSet::MetalFamily2Version1:
            return "metal2v1";
        case GraphicsFeatureSet::MetalFamily2Version2:
            return "metal2v2";
        case GraphicsFeatureSet::MetalFamily3Version1:
            return "metal3v1";
    }

    abort();
}
