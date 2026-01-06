/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_CubeFace_h
#define __xcassets_CubeFace_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * The faces of a cube.
 */
enum class CubeFace {
    NegativeX,
    PositiveX,
    NegativeY,
    PositiveY,
    NegativeZ,
    PositiveZ,
};

class CubeFaces {
private:
    CubeFaces();
    ~CubeFaces();

public:
    /*
     * Parse a matching cube face from a string, if valid.
     */
    static ext::optional<CubeFace> Parse(std::string const &value);

    /*
     * Convert a cube face to a string.
     */
    static std::string String(CubeFace cubeFace);
};

}

#endif // !__xcassets_CubeFace_h
