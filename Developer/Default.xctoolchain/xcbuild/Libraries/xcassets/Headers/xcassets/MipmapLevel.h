/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_MipmapLevel_h
#define __xcassets_MipmapLevel_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * The level of a mipmap.
 */
enum class MipmapLevel {
    Base,
    Level1,
    Level2,
    Level3,
    Level4,
    Level5,
    Level6,
    Level7,
    Level8,
    Level9,
    Level10,
    Level11,
    Level12,
    Level13,
    Level14,
    Level15,
    Level16,
};

class MipmapLevels {
private:
    MipmapLevels();
    ~MipmapLevels();

public:
    /*
     * Parse a matching mipmap level from a string, if valid.
     */
    static ext::optional<MipmapLevel> Parse(std::string const &value);

    /*
     * Convert an mipmap level to a string.
     */
    static std::string String(MipmapLevel mipmapLevel);
};

}

#endif // !__xcassets_MipmapLevel_h
