/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_MipmapLevelMode_h
#define __xcassets_MipmapLevelMode_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * How mipmap levels are interpreted.
 */
enum class MipmapLevelMode {
    None,
    All,
    Fixed,
};

class MipmapLevelModes {
private:
    MipmapLevelModes();
    ~MipmapLevelModes();

public:
    /*
     * Parse a matching mipmap level mode from a string, if valid.
     */
    static ext::optional<MipmapLevelMode> Parse(std::string const &value);

    /*
     * Convert an mipmap level mode to a string.
     */
    static std::string String(MipmapLevelMode mipmapLevelMode);
};

}

#endif // !__xcassets_MipmapLevelMode_h


