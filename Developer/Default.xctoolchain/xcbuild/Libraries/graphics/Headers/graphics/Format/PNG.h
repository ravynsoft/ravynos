/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __graphics_Format_PNG_h
#define __graphics_Format_PNG_h

#include <graphics/Image.h>
#include <graphics/PixelFormat.h>

#include <string>
#include <utility>
#include <vector>
#include <ext/optional>

namespace graphics {
namespace Format {

/*
 * Utilities for PNG images.
 */
class PNG {
private:
    PNG();
    ~PNG();

public:
    /*
     * Read a PNG image.
     */
    static std::pair<ext::optional<Image>, std::string>
    Read(std::vector<uint8_t> const &contents);

public:
    /*
     * Write a PNG image.
     */
    static std::pair<ext::optional<std::vector<uint8_t>>, std::string>
    Write(Image const &image);
};

}
}

#endif // !__graphics_Format_PNG_h
