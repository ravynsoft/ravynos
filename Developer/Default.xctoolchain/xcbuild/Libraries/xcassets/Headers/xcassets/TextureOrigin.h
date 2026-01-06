/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_TextureOrigin_h
#define __xcassets_TextureOrigin_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * The origin point of the texture.
 */
enum class TextureOrigin {
    BottomLeft,
};

class TextureOrigins {
private:
    TextureOrigins();
    ~TextureOrigins();

public:
    /*
     * Parse a matching texture origin from a string, if valid.
     */
    static ext::optional<TextureOrigin> Parse(std::string const &value);

    /*
     * Convert an texture origin to a string.
     */
    static std::string String(TextureOrigin textureOrigin);
};

}

#endif // !__xcassets_TextureOrigin_h
