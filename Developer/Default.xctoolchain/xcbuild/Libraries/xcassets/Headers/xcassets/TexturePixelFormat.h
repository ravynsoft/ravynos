/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_TexturePixelFormat_h
#define __xcassets_TexturePixelFormat_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * The pixel format of the texture.
 */
enum class TexturePixelFormat {
    NormalizedR8,
    NormalizedRG8,
    NormalizedRGBA8,
    NormalizedRGBA8sRGB,
    FloatingR16,
    FloatingRG16,
    FloatingRGBA16,
    ExtendedRangeRBG10sRGB,
    ASTC4x4,
    ASTC4x4sRGB,
    ASTC8x8,
    ASTC8x8sRGB,
};

class TexturePixelFormats {
private:
    TexturePixelFormats();
    ~TexturePixelFormats();

public:
    /*
     * Parse a matching texture pixel format from a string, if valid.
     */
    static ext::optional<TexturePixelFormat> Parse(std::string const &value);

    /*
     * Convert an texture pixel format to a string.
     */
    static std::string String(TexturePixelFormat texturePixelFormat);
};

}

#endif // !__xcassets_TexturePixelFormat_h
