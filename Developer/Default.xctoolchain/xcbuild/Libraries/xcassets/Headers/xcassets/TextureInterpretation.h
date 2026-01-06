/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_TextureInterpretation_h
#define __xcassets_TextureInterpretation_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * The interpretation of the texture.
 */
enum class TextureInterpretation {
    Colors,
    Data,
};

class TextureInterpretations {
private:
    TextureInterpretations();
    ~TextureInterpretations();

public:
    /*
     * Parse a matching texture interpretation from a string, if valid.
     */
    static ext::optional<TextureInterpretation> Parse(std::string const &value);

    /*
     * Convert an texture interpretation to a string.
     */
    static std::string String(TextureInterpretation textureInterpretation);
};

}

#endif // !__xcassets_TextureInterpretation_h
