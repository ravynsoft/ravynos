/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __graphics_Image_h
#define __graphics_Image_h

#include <graphics/PixelFormat.h>

#include <cstddef>
#include <cstdint>

namespace graphics {

/*
 * Contains an image data and metadata.
 */
class Image {
private:
    size_t               _width;
    size_t               _height;

private:
    PixelFormat          _format;
    std::vector<uint8_t> _data;

public:
    Image(size_t width, size_t height, PixelFormat format, std::vector<uint8_t> const &data);

public:
    /*
     * The width of the image, in pixels.
     */
    size_t width() const
    { return _width; }

    /*
     * The height of the image, in pixels.
     */
    size_t height() const
    { return _height; }

public:
    /*
     * The pixel format of the image data.
     */
    PixelFormat const &format() const
    { return _format; }

    /*
     * The alpha channel.
     */
    std::vector<uint8_t> const &data() const
    { return _data; }
};

}

#endif  // !__graphics_Image_h
