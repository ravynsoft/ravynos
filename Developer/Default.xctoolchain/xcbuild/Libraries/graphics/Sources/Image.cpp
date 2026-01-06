/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <graphics/Image.h>

#include <cassert>

using graphics::Image;
using graphics::PixelFormat;

Image::
Image(size_t width, size_t height, PixelFormat format, std::vector<uint8_t> const &data) :
    _width (width),
    _height(height),
    _format(format),
    _data  (data)
{
    assert(data.size() == _width * _height * _format.bytesPerPixel());
}

