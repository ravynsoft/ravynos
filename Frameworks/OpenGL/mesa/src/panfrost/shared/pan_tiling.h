/*
 * Copyright (c) 2011-2013 Luc Verhaegen <libv@skynet.be>
 * Copyright (c) 2018 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (c) 2018 Vasily Khoruzhick <anarsoul@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef H_PANFROST_TILING
#define H_PANFROST_TILING

#include <stdint.h>
#include <util/format/u_format.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Load a rectangular region from a tiled image to a linear staging image.
 *
 * @dst Linear destination
 * @src Tiled source
 * @x Region of interest of source in pixels, aligned to block size
 * @y Region of interest of source in pixels, aligned to block size
 * @z Region of interest of source in pixels, aligned to block size
 * @w Region of interest of source in pixels, aligned to block size
 * @dst_stride Stride in bytes of linear destination
 * @src_stride Number of bytes between adjacent rows of tiles in source.
 * @format Format of the source and destination image
 */
void panfrost_load_tiled_image(void *dst, const void *src, unsigned x,
                               unsigned y, unsigned w, unsigned h,
                               uint32_t dst_stride, uint32_t src_stride,
                               enum pipe_format format);

/**
 * Store a linear staging image to a rectangular region of a tiled image.
 *
 * @dst Tiled destination
 * @src Linear source
 * @x Region of interest of destination in pixels, aligned to block size
 * @y Region of interest of destination in pixels, aligned to block size
 * @z Region of interest of destination in pixels, aligned to block size
 * @w Region of interest of destination in pixels, aligned to block size
 * @dst_stride Number of bytes between adjacent rows of tiles in destination.
 * @src_stride Stride in bytes of linear source
 * @format Format of the source and destination image
 */
void panfrost_store_tiled_image(void *dst, const void *src, unsigned x,
                                unsigned y, unsigned w, unsigned h,
                                uint32_t dst_stride, uint32_t src_stride,
                                enum pipe_format format);

#ifdef __cplusplus
} /* extern C */
#endif

#endif
