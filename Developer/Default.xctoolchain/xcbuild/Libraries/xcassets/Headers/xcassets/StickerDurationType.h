/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_StickerDurationType_h
#define __xcassets_StickerDurationType_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * How a sticker duration is interpreted.
 */
enum class StickerDurationType {
    /*
     * Fixed number of seconds.
     */
    Fixed,
    /*
     * Frames to show per second.
     */
    FPS,
};

class StickerDurationTypes {
private:
    StickerDurationTypes();
    ~StickerDurationTypes();

public:
    /*
     * Parse a duration type string.
     */
    static ext::optional<StickerDurationType> Parse(std::string const &value);

    /*
     * String representation of a duration type.
     */
    static std::string String(StickerDurationType stickerDurationType);
};

}

#endif // !__xcassets_StickerDurationType_h
