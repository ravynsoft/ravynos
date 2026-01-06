/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Compression_h
#define __xcassets_Compression_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * An image compression type.
 */
enum class Compression {
    Automatic,
    Lossless,
    Lossy,
    GPUOptimizedBest,
    GPUOptimizedSmallest,
};

class Compressions {
private:
    Compressions();
    ~Compressions();

public:
    /*
     * Parse a matching compression from a string, if valid.
     */
    static ext::optional<Compression> Parse(std::string const &value);

    /*
     * Convert a compression to a string.
     */
    static std::string String(Compression compression);
};

}

#endif // !__xcassets_Compression_h
