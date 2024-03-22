/*
 * Copyright © 2014-2018 Broadcom
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#define V3D_OUTPUT_IMAGE_FORMAT_NO 255

#include <stdbool.h>
#include <stdint.h>

struct v3d_format {
        /** Set if the pipe format is defined in the table. */
        bool present;

        /** One of V3D42_OUTPUT_IMAGE_FORMAT_*, or OUTPUT_IMAGE_FORMAT_NO */
        uint8_t rt_type;

        /** One of V3D42_TEXTURE_DATA_FORMAT_*. */
        uint8_t tex_type;

        /**
         * Swizzle to apply to the RGBA shader output for storing to the tile
         * buffer, to the RGBA tile buffer to produce shader input (for
         * blending), and for turning the rgba8888 texture sampler return
         * value into shader rgba values.
         */
        uint8_t swizzle[4];

        /* Whether the return value is 16F/I/UI or 32F/I/UI. */
        uint8_t return_size;

        /* If return_size == 32, how many channels are returned by texturing.
         * 16 always returns 2 pairs of 16 bit values.
         */
        uint8_t return_channels;
};
