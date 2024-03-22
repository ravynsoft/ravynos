/* Copyright 2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#pragma once

#include <string.h>
#include "vpe_types.h"
#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADDR_LO(addr) ((addr) & 0xFFFFFFFF)
#define ADDR_HI(addr) (((addr) >> 32) & 0xFFFFFFFF)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define swap(x, y)                                                                                 \
    do {                                                                                           \
        unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed int)sizeof(x) : -1];              \
        memcpy(swap_temp, &y, sizeof(x));                                                          \
        y = x;                                                                                     \
        memcpy(&x, swap_temp, sizeof(x));                                                          \
    } while (0)

#ifndef min
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef max
#define max(x, y) (((x) > (y)) ? (x) : (y))
#endif

bool vpe_find_color_space_from_table(
    const struct vpe_color_space *table, int table_size, const struct vpe_color_space *cs);

bool vpe_is_dual_plane_format(enum vpe_surface_pixel_format format);

// RGB checkers
bool vpe_is_32bit_packed_rgb(enum vpe_surface_pixel_format format);
bool vpe_is_rgb8(enum vpe_surface_pixel_format format);
bool vpe_is_rgb10(enum vpe_surface_pixel_format format);
bool vpe_is_fp16(enum vpe_surface_pixel_format format);

// yuv 4:2:0 check
bool vpe_is_yuv420_8(enum vpe_surface_pixel_format format);
bool vpe_is_yuv420_10(enum vpe_surface_pixel_format format);
bool vpe_is_yuv420(enum vpe_surface_pixel_format format);

// yuv 4:4:4 check
bool vpe_is_yuv444_8(enum vpe_surface_pixel_format format);
bool vpe_is_yuv444_10(enum vpe_surface_pixel_format format);

enum color_depth vpe_get_color_depth(enum vpe_surface_pixel_format format);

bool vpe_has_per_pixel_alpha(enum vpe_surface_pixel_format format);

enum vpe_status vpe_check_output_support(struct vpe *vpe, const struct vpe_build_param *param);

enum vpe_status vpe_check_input_support(struct vpe *vpe, const struct vpe_stream *stream);

enum vpe_status vpe_cache_tone_map_params(struct stream_ctx *, const struct vpe_build_param *param);

enum vpe_status vpe_check_tone_map_support(
    struct vpe *vpe, const struct vpe_stream *stream, const struct vpe_build_param *param);

#ifdef __cplusplus
}
#endif
