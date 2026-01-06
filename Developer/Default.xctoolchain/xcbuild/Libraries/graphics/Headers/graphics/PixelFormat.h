/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __graphics_PixelFormat_h
#define __graphics_PixelFormat_h

#include <vector>

#include <cstddef>
#include <cstdint>

namespace graphics {

/*
 * Describes the format of pixels in a bitmap. Assumes 8 bits per channel.
 */
class PixelFormat {
public:
    /*
     * The channels stored in the bitmap.
     */
    enum class Color {
        /*
         * Only a gray channel.
         */
        Grayscale,
        /*
         * RGB color.
         */
        RGB,
    };

    /*
     * The order of the channels.
     */
    enum class Order {
        /*
         * Forward order in memory, i.e. big-endian.
         */
        Forward,
        /*
         * Reversed order in memory, i.e. little-endian.
         */
        Reversed,
    };

    /*
     * The presence and location of the alpha channel. Note the names
     * are reversed if the order is reversed.
     */
    enum class Alpha {
        /*
         * No alpha.
         */
        None,
        /*
         * Ignored alpha channel first.
         */
        IgnoredFirst,
        /*
         * Ignored alpha channel last.
         */
        IgnoredLast,

        /*
         * Alpha channel is first.
         */
        First,
        /*
         * Alpha channel is last.
         */
        Last,
        /*
         * Alpha channel is premultiplied and is first.
         */
        PremultipliedFirst,
        /*
         * Alpha channel is premultiplied and is last.
         */
        PremultipliedLast,
    };

private:
    Color _color;
    Order _order;
    Alpha _alpha;

public:
    PixelFormat(Color color, Order order, Alpha alpha) :
        _color(color),
        _order(order),
        _alpha(alpha)
    {
    }

public:
    /*
     * The specified color channels.
     */
    Color color() const
    { return _color; }

    /*
     * The order of the channels.
     */
    Order order() const
    { return _order; }

    /*
     * The alpha channel.
     */
    Alpha alpha() const
    { return _alpha; }

public:
    /*
     * The total number of channels per pixel.
     */
    size_t channels() const;

    /*
     * The total number of bytes per pixel.
     */
    size_t bytesPerPixel() const;

    /*
     * The total number of bits per pixel.
     */
    size_t bitsPerPixel() const;

public:
    /*
     * Convert from one color format to another.
     */
    static std::vector<uint8_t> Convert(
        std::vector<uint8_t> const &pixels,
        PixelFormat const &from,
        PixelFormat const &to);
};

}

#endif  // !__graphics_PixelFormat_h
