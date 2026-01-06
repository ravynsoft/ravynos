/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <graphics/PixelFormat.h>

using graphics::PixelFormat;

TEST(PixelFormat, Properties)
{
    /* Grayscale is one-byte. */
    PixelFormat f0 = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::None);
    EXPECT_EQ(f0.channels(), 1);
    EXPECT_EQ(f0.bytesPerPixel(), 1);
    EXPECT_EQ(f0.bitsPerPixel(), 8);

    /* Ignored alpha is a byte but not a channel. */
    PixelFormat f1 = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::IgnoredFirst);
    EXPECT_EQ(f1.channels(), 1);
    EXPECT_EQ(f1.bytesPerPixel(), 2);
    EXPECT_EQ(f1.bitsPerPixel(), 16);

    /* Real alpha is a channel too. */
    PixelFormat f2 = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::First);
    EXPECT_EQ(f2.channels(), 2);
    EXPECT_EQ(f2.bytesPerPixel(), 2);
    EXPECT_EQ(f2.bitsPerPixel(), 16);

    /* RGB has three channels. */
    PixelFormat f3 = PixelFormat(PixelFormat::Color::RGB, PixelFormat::Order::Forward, PixelFormat::Alpha::None);
    EXPECT_EQ(f3.channels(), 3);
    EXPECT_EQ(f3.bytesPerPixel(), 3);
    EXPECT_EQ(f3.bitsPerPixel(), 24);

    /* RGB can also have alpha. */
    PixelFormat f4 = PixelFormat(PixelFormat::Color::RGB, PixelFormat::Order::Forward, PixelFormat::Alpha::PremultipliedLast);
    EXPECT_EQ(f4.channels(), 4);
    EXPECT_EQ(f4.bytesPerPixel(), 4);
    EXPECT_EQ(f4.bitsPerPixel(), 32);
}

static std::vector<uint8_t>
Expected(std::vector<uint8_t> const &value)
{
    return value;
}

TEST(PixelFormat, ConvertAlpha)
{
    PixelFormat none = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::None);
    PixelFormat last = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::Last);

    /* Should add alpha as 0xFF (solid). */
    EXPECT_EQ(PixelFormat::Convert({ 0x6A, 0x6B }, none, last), Expected({ 0x6A, 0xFF, 0x6B, 0xFF }));

    /* Should strip out alpha, compositing against black. */
    EXPECT_EQ(PixelFormat::Convert({ 0x60, 0x7F, 0x6B, 0xFF }, last, none), Expected({ 0x30, 0x6B }));

    PixelFormat normal = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::Last);
    PixelFormat premult = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::PremultipliedLast);

    /* Should multiply in alpha. */
    EXPECT_EQ(PixelFormat::Convert({ 0x60, 0x7F, 0x80, 0x40 }, normal, premult), Expected({ 0x30, 0x7F, 0x20, 0x40 }));

    /* Should remove premultiplication. */
    EXPECT_EQ(PixelFormat::Convert({ 0x30, 0x7F, 0x20, 0x40 }, premult, normal), Expected({ 0x60, 0x7F, 0x80, 0x40 }));
}

TEST(PixelFormat, ConvertGrayscale)
{
    PixelFormat gray = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::None);
    PixelFormat color = PixelFormat(PixelFormat::Color::RGB, PixelFormat::Order::Forward, PixelFormat::Alpha::None);

    /* Should repeat the gray across the channels. */
    EXPECT_EQ(PixelFormat::Convert({ 0x6A, 0x6B }, gray, color), Expected({ 0x6A, 0x6A, 0x6A, 0x6B, 0x6B, 0x6B }));

    /* Should average the channels to grayscale. */
    EXPECT_EQ(PixelFormat::Convert({ 0x1A, 0x3A, 0x5A, 0x10, 0x40, 0xA0 }, color, gray), Expected({ 0x3A, 0x50 }));
}

TEST(PixelFormat, ConvertRearrange)
{
    /* Should flip alpha position. */
    PixelFormat first = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::First);
    PixelFormat last = PixelFormat(PixelFormat::Color::Grayscale, PixelFormat::Order::Forward, PixelFormat::Alpha::Last);
    EXPECT_EQ(PixelFormat::Convert({ 0x6A, 0x7F }, last, first), Expected({ 0x7F, 0x6A }));
    EXPECT_EQ(PixelFormat::Convert({ 0x7F, 0x6A }, first, last), Expected({ 0x6A, 0x7F }));

    /* Should flip color channels. */
    PixelFormat forward = PixelFormat(PixelFormat::Color::RGB, PixelFormat::Order::Forward, PixelFormat::Alpha::None);
    PixelFormat reversed = PixelFormat(PixelFormat::Color::RGB, PixelFormat::Order::Reversed, PixelFormat::Alpha::None);
    EXPECT_EQ(PixelFormat::Convert({ 0x6A, 0x6C, 0x6E }, forward, reversed), Expected({ 0x6E, 0x6C, 0x6A }));
    EXPECT_EQ(PixelFormat::Convert({ 0x6E, 0x6C, 0x6A }, reversed, forward), Expected({ 0x6A, 0x6C, 0x6E }));
}
