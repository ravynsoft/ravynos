/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <graphics/PixelFormat.h>

#include <cmath>
#include <ext/optional>

using graphics::PixelFormat;

size_t PixelFormat::
channels() const
{
    size_t channels = 0;

    switch (_color) {
        case Color::Grayscale:
            channels += 1;
            break;
        case Color::RGB:
            channels += 3;
            break;
    }

    switch (_alpha) {
        case Alpha::None:
        case Alpha::IgnoredFirst:
        case Alpha::IgnoredLast:
            break;
        case Alpha::First:
        case Alpha::Last:
        case Alpha::PremultipliedFirst:
        case Alpha::PremultipliedLast:
            channels += 1;
            break;
    }

    return channels;
}

size_t PixelFormat::
bytesPerPixel() const
{
    /* Assume 8 bits per channel. */
    size_t bytes = channels();

    /* Ignored alpha adds a byte. */
    switch (_alpha) {
        case Alpha::IgnoredFirst:
        case Alpha::IgnoredLast:
            bytes += 1;
            break;
        default:
            break;
    }

    return bytes;
}

size_t PixelFormat::
bitsPerPixel() const
{
    return 8 * bytesPerPixel();
}

static bool
AlphaPremultiplied(PixelFormat::Alpha alpha)
{
    switch (alpha) {
        case PixelFormat::Alpha::None:
        case PixelFormat::Alpha::IgnoredFirst:
        case PixelFormat::Alpha::IgnoredLast:
        case PixelFormat::Alpha::First:
        case PixelFormat::Alpha::Last:
            return false;
        case PixelFormat::Alpha::PremultipliedFirst:
        case PixelFormat::Alpha::PremultipliedLast:
            return true;
        default: abort();
    }
}

static size_t
OrderChannel(size_t channel, PixelFormat::Order order, size_t channels)
{
    switch (order) {
        case PixelFormat::Order::Forward:
            return channel;
        case PixelFormat::Order::Reversed:
            return (channels - 1 - channel);
        default: abort();
    }
}

static ext::optional<size_t>
AlphaChannel(PixelFormat::Alpha alpha, PixelFormat::Order order, size_t channels)
{
    switch (alpha) {
        case PixelFormat::Alpha::None:
        case PixelFormat::Alpha::IgnoredFirst:
        case PixelFormat::Alpha::IgnoredLast:
            return ext::nullopt;
        case PixelFormat::Alpha::PremultipliedFirst:
        case PixelFormat::Alpha::First:
            return OrderChannel(0, order, channels);
        case PixelFormat::Alpha::PremultipliedLast:
        case PixelFormat::Alpha::Last:
            return OrderChannel(channels - 1, order, channels);
        default: abort();
    }
}

static size_t
AlphaOffset(PixelFormat::Alpha alpha)
{
    switch (alpha) {
        case PixelFormat::Alpha::IgnoredFirst:
        case PixelFormat::Alpha::PremultipliedFirst:
        case PixelFormat::Alpha::First:
            return 1;
        case PixelFormat::Alpha::IgnoredLast:
        case PixelFormat::Alpha::PremultipliedLast:
        case PixelFormat::Alpha::Last:
        case PixelFormat::Alpha::None:
            return 0;
        default: abort();
    }
}

static uint8_t
Premultiply(uint8_t value, bool inPremultiplied, bool outPremultiplied, uint8_t alpha)
{
    /* Nothing to do. */
    if (alpha == 0xFF || inPremultiplied == outPremultiplied) {
        return value;
    }

    double v = (value / 255.0);
    double a = (alpha / 255.0);
    if (inPremultiplied) {
        /* Unpremultiply. */
        double rounded = std::round((a ? v / a : 0) * 255.0);
        return static_cast<uint8_t>(rounded);
    } else if (outPremultiplied) {
        /* Premultply. */
        double rounded = std::round((v * a) * 255.0);
        return static_cast<uint8_t>(rounded);
    }

    abort();
}

static void
ColorChannels(size_t *red, size_t *green, size_t *blue, PixelFormat::Color color, PixelFormat::Order order, PixelFormat::Alpha alpha, size_t channels)
{
    size_t alphaOffset = AlphaOffset(alpha);

    switch (color) {
        case PixelFormat::Color::RGB:
            *red = OrderChannel(0 + alphaOffset, order, channels);
            *green = OrderChannel(1 + alphaOffset, order, channels);
            *blue = OrderChannel(2 + alphaOffset, order, channels);
            break;
        case PixelFormat::Color::Grayscale:
            *red = *green = *blue = OrderChannel(0 + alphaOffset, order, channels);
            break;
        default: abort();
    }
}

std::vector<uint8_t> PixelFormat::
Convert(std::vector<uint8_t> const &pixels, PixelFormat const &from, PixelFormat const &to)
{
    /* Determine number of pixels. */
    size_t fromBytesPerPixel = from.bytesPerPixel();
    size_t pixelCount = pixels.size() / fromBytesPerPixel;

    /* Allocate output. */
    size_t toBytesPerPixel = to.bytesPerPixel();
    std::vector<uint8_t> result = std::vector<uint8_t>(pixelCount * toBytesPerPixel);

    /* Find alpha channels. */
    ext::optional<size_t> fromAlphaChannel = AlphaChannel(from.alpha(), from.order(), from.channels());
    bool fromAlphaPremultiplied = AlphaPremultiplied(from.alpha());
    ext::optional<size_t> toAlphaChannel = AlphaChannel(to.alpha(), to.order(), to.channels());
    bool toAlphaPremultiplied = AlphaPremultiplied(to.alpha());

    /* Create channel mapping. */
    size_t fromRed, fromGreen, fromBlue;
    ColorChannels(&fromRed, &fromGreen, &fromBlue, from.color(), from.order(), from.alpha(), from.channels());
    size_t toRed, toGreen, toBlue;
    ColorChannels(&toRed, &toGreen, &toBlue, to.color(), to.order(), to.alpha(), to.channels());

    /*
     * Additionally premultiply alpha when removing the alpha channel; essentially, composite
     * on black. This preserves appearance at the cost of some color data.
     */
    bool toPremultiplied = (toAlphaPremultiplied || !toAlphaChannel);

    if (from.color() == to.color() && (bool)fromAlphaChannel == (bool)toAlphaChannel && fromAlphaPremultiplied == toPremultiplied) {
        /*
         * Fast path: not converting color formats or changing alpha.
         */
        for (size_t i = 0; i < pixelCount; ++i) {
            uint8_t const *fromPixel = &pixels[i * fromBytesPerPixel];
            uint8_t *toPixel = &result[i * toBytesPerPixel];

            /* Copy alpha channel. */
            if (fromAlphaChannel && toAlphaChannel) {
                toPixel[*toAlphaChannel] = fromPixel[*fromAlphaChannel];
            }

            /* Copy data channels. */
            toPixel[toRed] = fromPixel[fromRed];
            toPixel[toGreen] = fromPixel[fromGreen];
            toPixel[toBlue] = fromPixel[fromBlue];
        }
    } else {
        /*
         * Slow path: have to modify pixel data, either to convert color formats or adjust alpha.
         */
        bool convertingToGrayscale = (from.color() == Color::RGB && to.color() == Color::Grayscale);
        for (size_t i = 0; i < pixelCount; ++i) {
            uint8_t const *fromPixel = &pixels[i * fromBytesPerPixel];
            uint8_t *toPixel = &result[i * toBytesPerPixel];

            /* Copy alpha channel. */
            uint8_t alpha = 0xFF;
            if (fromAlphaChannel) {
                alpha = fromPixel[*fromAlphaChannel];
            }
            if (toAlphaChannel) {
                toPixel[*toAlphaChannel] = alpha;
            }

            /* Copy data channels. */
            uint8_t red = fromPixel[fromRed];
            uint8_t green = fromPixel[fromGreen];
            uint8_t blue = fromPixel[fromBlue];

            /* If converting to grayscale, average the channels. */
            if (convertingToGrayscale) {
                double value = ((red / 255.0) + (green / 255.0) + (blue / 255.0)) / 3.0;
                double rounded = std::round(value * 255.0);
                red = green = blue = static_cast<uint8_t>(rounded);
            }

            toPixel[toRed] = Premultiply(red, fromAlphaPremultiplied, toPremultiplied, alpha);
            toPixel[toGreen] = Premultiply(green, fromAlphaPremultiplied, toPremultiplied, alpha);
            toPixel[toBlue] = Premultiply(blue, fromAlphaPremultiplied, toPremultiplied, alpha);
        }
    }

    return result;
}

