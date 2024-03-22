/*
 * Copyright © 2014-2017 Broadcom
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

/** @file v3d_tiling.c
 *
 * Handles information about the V3D tiling formats, and loading and storing
 * from them.
 */

#include <stdint.h>
#include "util/u_box.h" /* FIXME: avoid include this */ 
#include "v3d_tiling.h"
#include "broadcom/common/v3d_cpu_tiling.h"

/** Return the width in pixels of a 64-byte microtile. */
uint32_t
v3d_utile_width(int cpp)
{
        switch (cpp) {
        case 1:
        case 2:
                return 8;
        case 4:
        case 8:
                return 4;
        case 16:
                return 2;
        default:
                unreachable("unknown cpp");
        }
}

/** Return the height in pixels of a 64-byte microtile. */
uint32_t
v3d_utile_height(int cpp)
{
        switch (cpp) {
        case 1:
                return 8;
        case 2:
        case 4:
                return 4;
        case 8:
        case 16:
                return 2;
        default:
                unreachable("unknown cpp");
        }
}

/**
 * Returns the byte address for a given pixel within a utile.
 *
 * Utiles are 64b blocks of pixels in raster order, with 32bpp being a 4x4
 * arrangement.
 */
static inline uint32_t
v3d_get_utile_pixel_offset(uint32_t cpp, uint32_t x, uint32_t y)
{
        uint32_t utile_w = v3d_utile_width(cpp);

        assert(x < utile_w && y < v3d_utile_height(cpp));

        return x * cpp + y * utile_w * cpp;
}

/**
 * Returns the byte offset for a given pixel in a LINEARTILE layout.
 *
 * LINEARTILE is a single line of utiles in either the X or Y direction.
 */
static inline uint32_t
v3d_get_lt_pixel_offset(uint32_t cpp, uint32_t image_h, uint32_t x, uint32_t y)
{
        uint32_t utile_w = v3d_utile_width(cpp);
        uint32_t utile_h = v3d_utile_height(cpp);
        uint32_t utile_index_x = x / utile_w;
        uint32_t utile_index_y = y / utile_h;

        assert(utile_index_x == 0 || utile_index_y == 0);

        return (64 * (utile_index_x + utile_index_y) +
                v3d_get_utile_pixel_offset(cpp,
                                           x & (utile_w - 1),
                                           y & (utile_h - 1)));
}

/**
 * Returns the byte offset for a given pixel in a UBLINEAR layout.
 *
 * UBLINEAR is the layout where pixels are arranged in UIF blocks (2x2
 * utiles), and the UIF blocks are in 1 or 2 columns in raster order.
 */
static inline uint32_t
v3d_get_ublinear_pixel_offset(uint32_t cpp, uint32_t x, uint32_t y,
                              int ublinear_number)
{
        uint32_t utile_w = v3d_utile_width(cpp);
        uint32_t utile_h = v3d_utile_height(cpp);
        uint32_t ub_w = utile_w * 2;
        uint32_t ub_h = utile_h * 2;
        uint32_t ub_x = x / ub_w;
        uint32_t ub_y = y / ub_h;

        return (256 * (ub_y * ublinear_number +
                       ub_x) +
                ((x & utile_w) ? 64 : 0) +
                ((y & utile_h) ? 128 : 0) +
                + v3d_get_utile_pixel_offset(cpp,
                                             x & (utile_w - 1),
                                             y & (utile_h - 1)));
}

static inline uint32_t
v3d_get_ublinear_2_column_pixel_offset(uint32_t cpp, uint32_t image_h,
                                       uint32_t x, uint32_t y)
{
        return v3d_get_ublinear_pixel_offset(cpp, x, y, 2);
}

static inline uint32_t
v3d_get_ublinear_1_column_pixel_offset(uint32_t cpp, uint32_t image_h,
                                       uint32_t x, uint32_t y)
{
        return v3d_get_ublinear_pixel_offset(cpp, x, y, 1);
}

/**
 * Returns the byte offset for a given pixel in a UIF layout.
 *
 * UIF is the general V3D tiling layout shared across 3D, media, and scanout.
 * It stores pixels in UIF blocks (2x2 utiles), and UIF blocks are stored in
 * 4x4 groups, and those 4x4 groups are then stored in raster order.
 */
static inline uint32_t
v3d_get_uif_pixel_offset(uint32_t cpp, uint32_t image_h, uint32_t x, uint32_t y,
                         bool do_xor)
{
        uint32_t utile_w = v3d_utile_width(cpp);
        uint32_t utile_h = v3d_utile_height(cpp);
        uint32_t mb_width = utile_w * 2;
        uint32_t mb_height = utile_h * 2;
        uint32_t log2_mb_width = ffs(mb_width) - 1;
        uint32_t log2_mb_height = ffs(mb_height) - 1;

        /* Macroblock X, y */
        uint32_t mb_x = x >> log2_mb_width;
        uint32_t mb_y = y >> log2_mb_height;
        /* X, y within the macroblock */
        uint32_t mb_pixel_x = x - (mb_x << log2_mb_width);
        uint32_t mb_pixel_y = y - (mb_y << log2_mb_height);

        if (do_xor && (mb_x / 4) & 1)
                mb_y ^= 0x10;

        uint32_t mb_h = align(image_h, 1 << log2_mb_height) >> log2_mb_height;
        uint32_t mb_id = ((mb_x / 4) * ((mb_h - 1) * 4)) + mb_x + mb_y * 4;

        uint32_t mb_base_addr = mb_id * 256;

        bool top = mb_pixel_y < utile_h;
        bool left = mb_pixel_x < utile_w;

        /* Docs have this in pixels, we do bytes here. */
        uint32_t mb_tile_offset = (!top * 128 + !left * 64);

        uint32_t utile_x = mb_pixel_x & (utile_w - 1);
        uint32_t utile_y = mb_pixel_y & (utile_h - 1);

        uint32_t mb_pixel_address = (mb_base_addr +
                                     mb_tile_offset +
                                     v3d_get_utile_pixel_offset(cpp,
                                                                utile_x,
                                                                utile_y));

        return mb_pixel_address;
}

static inline uint32_t
v3d_get_uif_xor_pixel_offset(uint32_t cpp, uint32_t image_h,
                             uint32_t x, uint32_t y)
{
        return v3d_get_uif_pixel_offset(cpp, image_h, x, y, true);
}

static inline uint32_t
v3d_get_uif_no_xor_pixel_offset(uint32_t cpp, uint32_t image_h,
                                uint32_t x, uint32_t y)
{
        return v3d_get_uif_pixel_offset(cpp, image_h, x, y, false);
}

/* Loads/stores non-utile-aligned boxes by walking over the destination
 * rectangle, computing the address on the GPU, and storing/loading a pixel at
 * a time.
 */
static inline void
v3d_move_pixels_unaligned(void *gpu, uint32_t gpu_stride,
                          void *cpu, uint32_t cpu_stride,
                          int cpp, uint32_t image_h,
                          const struct pipe_box *box,
                          uint32_t (*get_pixel_offset)(uint32_t cpp,
                                                       uint32_t image_h,
                                                       uint32_t x, uint32_t y),
                          bool is_load)
{
        for (uint32_t y = 0; y < box->height; y++) {
                void *cpu_row = cpu + y * cpu_stride;

                for (int x = 0; x < box->width; x++) {
                        uint32_t pixel_offset = get_pixel_offset(cpp, image_h,
                                                                 box->x + x,
                                                                 box->y + y);

                        if (false) {
                                fprintf(stderr, "%3d,%3d -> %d\n",
                                        box->x + x, box->y + y,
                                        pixel_offset);
                        }

                        if (is_load) {
                                memcpy(cpu_row + x * cpp,
                                       gpu + pixel_offset,
                                       cpp);
                        } else {
                                memcpy(gpu + pixel_offset,
                                       cpu_row + x * cpp,
                                       cpp);
                        }
                }
        }
}

/* Breaks the image down into utiles and calls either the fast whole-utile
 * load/store functions, or the unaligned fallback case.
 */
static inline void
v3d_move_pixels_general_percpp(void *gpu, uint32_t gpu_stride,
                               void *cpu, uint32_t cpu_stride,
                               int cpp, uint32_t image_h,
                               const struct pipe_box *box,
                               uint32_t (*get_pixel_offset)(uint32_t cpp,
                                                            uint32_t image_h,
                                                            uint32_t x, uint32_t y),
                               bool is_load)
{
        uint32_t utile_w = v3d_utile_width(cpp);
        uint32_t utile_h = v3d_utile_height(cpp);
        uint32_t utile_gpu_stride = utile_w * cpp;
        uint32_t x1 = box->x;
        uint32_t y1 = box->y;
        uint32_t x2 = box->x + box->width;
        uint32_t y2 = box->y + box->height;
        uint32_t align_x1 = align(x1, utile_w);
        uint32_t align_y1 = align(y1, utile_h);
        uint32_t align_x2 = x2 & ~(utile_w - 1);
        uint32_t align_y2 = y2 & ~(utile_h - 1);

        /* Load/store all the whole utiles first. */
        for (uint32_t y = align_y1; y < align_y2; y += utile_h) {
                void *cpu_row = cpu + (y - box->y) * cpu_stride;

                for (uint32_t x = align_x1; x < align_x2; x += utile_w) {
                        void *utile_gpu = (gpu +
                                           get_pixel_offset(cpp, image_h, x, y));
                        void *utile_cpu = cpu_row + (x - box->x) * cpp;

                        if (is_load) {
                                v3d_load_utile(utile_cpu, cpu_stride,
                                               utile_gpu, utile_gpu_stride);
                        } else {
                                v3d_store_utile(utile_gpu, utile_gpu_stride,
                                                utile_cpu, cpu_stride);
                        }
                }
        }

        /* If there were no aligned utiles in the middle, load/store the whole
         * thing unaligned.
         */
        if (align_y2 <= align_y1 ||
            align_x2 <= align_x1) {
                v3d_move_pixels_unaligned(gpu, gpu_stride,
                                          cpu, cpu_stride,
                                          cpp, image_h,
                                          box,
                                          get_pixel_offset, is_load);
                return;
        }

        /* Load/store the partial utiles. */
        struct pipe_box partial_boxes[4] = {
                /* Top */
                {
                        .x = x1,
                        .width = x2 - x1,
                        .y = y1,
                        .height = align_y1 - y1,
                },
                /* Bottom */
                {
                        .x = x1,
                        .width = x2 - x1,
                        .y = align_y2,
                        .height = y2 - align_y2,
                },
                /* Left */
                {
                        .x = x1,
                        .width = align_x1 - x1,
                        .y = align_y1,
                        .height = align_y2 - align_y1,
                },
                /* Right */
                {
                        .x = align_x2,
                        .width = x2 - align_x2,
                        .y = align_y1,
                        .height = align_y2 - align_y1,
                },
        };
        for (int i = 0; i < ARRAY_SIZE(partial_boxes); i++) {
                void *partial_cpu = (cpu +
                                     (partial_boxes[i].y - y1) * cpu_stride +
                                     (partial_boxes[i].x - x1) * cpp);

                v3d_move_pixels_unaligned(gpu, gpu_stride,
                                          partial_cpu, cpu_stride,
                                          cpp, image_h,
                                          &partial_boxes[i],
                                          get_pixel_offset, is_load);
        }
}

static inline void
v3d_move_pixels_general(void *gpu, uint32_t gpu_stride,
                               void *cpu, uint32_t cpu_stride,
                               int cpp, uint32_t image_h,
                               const struct pipe_box *box,
                               uint32_t (*get_pixel_offset)(uint32_t cpp,
                                                            uint32_t image_h,
                                                            uint32_t x, uint32_t y),
                               bool is_load)
{
        switch (cpp) {
        case 1:
                v3d_move_pixels_general_percpp(gpu, gpu_stride,
                                               cpu, cpu_stride,
                                               1, image_h, box,
                                               get_pixel_offset,
                                               is_load);
                break;
        case 2:
                v3d_move_pixels_general_percpp(gpu, gpu_stride,
                                               cpu, cpu_stride,
                                               2, image_h, box,
                                               get_pixel_offset,
                                               is_load);
                break;
        case 4:
                v3d_move_pixels_general_percpp(gpu, gpu_stride,
                                               cpu, cpu_stride,
                                               4, image_h, box,
                                               get_pixel_offset,
                                               is_load);
                break;
        case 8:
                v3d_move_pixels_general_percpp(gpu, gpu_stride,
                                               cpu, cpu_stride,
                                               8, image_h, box,
                                               get_pixel_offset,
                                               is_load);
                break;
        case 16:
                v3d_move_pixels_general_percpp(gpu, gpu_stride,
                                               cpu, cpu_stride,
                                               16, image_h, box,
                                               get_pixel_offset,
                                               is_load);
                break;
        }
}

static inline void
v3d_move_tiled_image(void *gpu, uint32_t gpu_stride,
                     void *cpu, uint32_t cpu_stride,
                     enum v3d_tiling_mode tiling_format,
                     int cpp,
                     uint32_t image_h,
                     const struct pipe_box *box,
                     bool is_load)
{
        switch (tiling_format) {
        case V3D_TILING_UIF_XOR:
                v3d_move_pixels_general(gpu, gpu_stride,
                                        cpu, cpu_stride,
                                        cpp, image_h, box,
                                        v3d_get_uif_xor_pixel_offset,
                                        is_load);
                break;
        case V3D_TILING_UIF_NO_XOR:
                v3d_move_pixels_general(gpu, gpu_stride,
                                        cpu, cpu_stride,
                                        cpp, image_h, box,
                                        v3d_get_uif_no_xor_pixel_offset,
                                        is_load);
                break;
        case V3D_TILING_UBLINEAR_2_COLUMN:
                v3d_move_pixels_general(gpu, gpu_stride,
                                        cpu, cpu_stride,
                                        cpp, image_h, box,
                                        v3d_get_ublinear_2_column_pixel_offset,
                                        is_load);
                break;
        case V3D_TILING_UBLINEAR_1_COLUMN:
                v3d_move_pixels_general(gpu, gpu_stride,
                                        cpu, cpu_stride,
                                        cpp, image_h, box,
                                        v3d_get_ublinear_1_column_pixel_offset,
                                        is_load);
                break;
        case V3D_TILING_LINEARTILE:
                v3d_move_pixels_general(gpu, gpu_stride,
                                        cpu, cpu_stride,
                                        cpp, image_h, box,
                                        v3d_get_lt_pixel_offset,
                                        is_load);
                break;
        default:
                unreachable("Unsupported tiling format");
                break;
        }
}

/**
 * Loads pixel data from the start (microtile-aligned) box in \p src to the
 * start of \p dst according to the given tiling format.
 */
void
v3d_load_tiled_image(void *dst, uint32_t dst_stride,
                     void *src, uint32_t src_stride,
                     enum v3d_tiling_mode tiling_format, int cpp,
                     uint32_t image_h,
                     const struct pipe_box *box)
{
        v3d_move_tiled_image(src, src_stride,
                             dst, dst_stride,
                             tiling_format,
                             cpp,
                             image_h,
                             box,
                             true);
}

/**
 * Stores pixel data from the start of \p src into a (microtile-aligned) box in
 * \p dst according to the given tiling format.
 */
void
v3d_store_tiled_image(void *dst, uint32_t dst_stride,
                      void *src, uint32_t src_stride,
                      enum v3d_tiling_mode tiling_format, int cpp,
                      uint32_t image_h,
                      const struct pipe_box *box)
{
        v3d_move_tiled_image(dst, dst_stride,
                             src, src_stride,
                             tiling_format,
                             cpp,
                             image_h,
                             box,
                             false);
}
