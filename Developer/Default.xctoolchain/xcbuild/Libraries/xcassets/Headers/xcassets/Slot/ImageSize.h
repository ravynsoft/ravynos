/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_ImageSize_h
#define __xcassets_Slot_ImageSize_h

#include <ext/optional>
#include <vector>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * The size of an image.
 */
class ImageSize {
private:
    double _width;
    double _height;

private:
    ImageSize(double width, double height);

public:
    /*
     * The represented image width.
     */
    double width() const
    { return _width; }

    /*
     * The represented image height.
     */
    double height() const
    { return _height; }

public:
    /*
     * Parse a matching image scale from a string, if valid.
     */
    static ext::optional<ImageSize> Parse(std::string const &value);

    /*
     * Convert an image size to a string.
     */
    static std::string String(ImageSize imageSize);
};

}
}

#endif // !__xcassets_Slot_ImageSize_h
