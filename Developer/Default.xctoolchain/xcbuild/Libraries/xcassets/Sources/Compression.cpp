/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Compression.h>

#include <cstdlib>

using xcassets::Compression;
using xcassets::Compressions;

ext::optional<Compression> Compressions::
Parse(std::string const &value)
{
    if (value == "automatic") {
        return Compression::Automatic;
    } else if (value == "lossless") {
        return Compression::Lossless;
    } else if (value == "lossy") {
        return Compression::Lossy;
    } else if (value == "gpu-optimized-best") {
        return Compression::GPUOptimizedBest;
    } else if (value == "gpu-optimized-smallest") {
        return Compression::GPUOptimizedSmallest;
    } else {
        fprintf(stderr, "warning: unknown compression %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string Compressions::
String(Compression platform)
{
    switch (platform) {
        case Compression::Automatic:
            return "automatic";
        case Compression::Lossless:
            return "lossless";
        case Compression::Lossy:
            return "lossy";
        case Compression::GPUOptimizedBest:
            return "gpu-optimized-best";
        case Compression::GPUOptimizedSmallest:
            return "gpu-optimized-smallest";
    }

    abort();
}
