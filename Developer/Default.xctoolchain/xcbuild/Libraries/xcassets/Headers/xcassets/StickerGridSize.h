/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_StickerGridSize_h
#define __xcassets_StickerGridSize_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * The sizes to display stickers to pick.
 */
enum class StickerGridSize {
    Small,
    Regular,
    Large,
};

class StickerGridSizes {
private:
    StickerGridSizes();
    ~StickerGridSizes();

public:
    /*
     * Parse a grid size string.
     */
    static ext::optional<StickerGridSize> Parse(std::string const &value);

    /*
     * String representation of a grid size.
     */
    static std::string String(StickerGridSize stickerGridSize);
};

}

#endif // !__xcassets_StickerGridSize_h
