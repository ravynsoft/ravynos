/*
 * Copyright © 2014 Broadcom
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

#ifndef V3D_TILING_H
#define V3D_TILING_H

#include "util/format/u_format.h"

/* A UIFblock is a 256-byte region of memory that's 256-byte aligned.  These
 * will be grouped in 4x4 blocks (left-to-right, then top-to-bottom) in a 4KB
 * page.  Those pages are then arranged left-to-right, top-to-bottom, to cover
 * an image.
 *
 * The inside of a UIFblock, for packed pixels, will be split into 4 64-byte
 * utiles.  Utiles may be 8x8 (8bpp), 8x4(16bpp) or 4x4 (32bpp).
 */

/**
 * Tiling mode enum used for v3d_resource.c, which maps directly to the Memory
 * Format field of render target and Z/Stencil config.
 */
enum v3d_tiling_mode {
        /* Untiled resources.  Not valid as texture inputs. */
        V3D_TILING_RASTER,

        /* Single line of u-tiles. */
        V3D_TILING_LINEARTILE,

        /* Departure from standard 4-UIF block column format. */
        V3D_TILING_UBLINEAR_1_COLUMN,

        /* Departure from standard 4-UIF block column format. */
        V3D_TILING_UBLINEAR_2_COLUMN,

        /* Normal tiling format: grouped in 4x4 UIFblocks, each of which is
         * split 2x2 into utiles.
         */
        V3D_TILING_UIF_NO_XOR,

        /* Normal tiling format: grouped in 4x4 UIFblocks, each of which is
         * split 2x2 into utiles.
         */
        V3D_TILING_UIF_XOR,
};

struct pipe_box;

uint32_t v3d_utile_width(int cpp) ATTRIBUTE_CONST;
uint32_t v3d_utile_height(int cpp) ATTRIBUTE_CONST;
bool v3d_size_is_lt(uint32_t width, uint32_t height, int cpp) ATTRIBUTE_CONST;
void v3d_load_tiled_image(void *dst, uint32_t dst_stride,
                          void *src, uint32_t src_stride,
                          enum v3d_tiling_mode tiling_format, int cpp,
                          uint32_t image_h,
                          const struct pipe_box *box);
void v3d_store_tiled_image(void *dst, uint32_t dst_stride,
                           void *src, uint32_t src_stride,
                           enum v3d_tiling_mode tiling_format, int cpp,
                           uint32_t image_h,
                           const struct pipe_box *box);

#endif /* V3D_TILING_H */
