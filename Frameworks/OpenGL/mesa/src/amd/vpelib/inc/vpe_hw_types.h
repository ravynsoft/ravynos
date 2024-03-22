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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 * Note: do *not* add any types which are *not* used for HW programming.
 * this will ensure separation of Logic layer from HW layer
 ***********************************************************************/
union large_integer {
    struct {
        uint32_t low_part;
        int32_t  high_part;
    };

    struct {
        uint32_t low_part;
        int32_t  high_part;
    } u;

    int64_t quad_part;
};

#define PHYSICAL_ADDRESS_LOC union large_integer

enum vpe_plane_addr_type {
    VPE_PLN_ADDR_TYPE_GRAPHICS = 0,
    VPE_PLN_ADDR_TYPE_VIDEO_PROGRESSIVE
};

struct vpe_plane_address {
    enum vpe_plane_addr_type type;
    bool                     tmz_surface;
    union {
        struct {
            PHYSICAL_ADDRESS_LOC addr;
            PHYSICAL_ADDRESS_LOC meta_addr;
            union large_integer  dcc_const_color;
        } grph;

        /*video  progressive*/
        struct {
            PHYSICAL_ADDRESS_LOC luma_addr;
            PHYSICAL_ADDRESS_LOC luma_meta_addr;
            union large_integer  luma_dcc_const_color;

            PHYSICAL_ADDRESS_LOC chroma_addr;
            PHYSICAL_ADDRESS_LOC chroma_meta_addr;
            union large_integer  chroma_dcc_const_color;
        } video_progressive;
    };
};

/* Rotation angle */
enum vpe_rotation_angle {
    VPE_ROTATION_ANGLE_0 = 0,
    VPE_ROTATION_ANGLE_90,
    VPE_ROTATION_ANGLE_180,
    VPE_ROTATION_ANGLE_270,
    VPE_ROTATION_ANGLE_COUNT
};

/* mirror */
enum vpe_mirror {
    VPE_MIRROR_NONE,
    VPE_MIRROR_HORIZONTAL,
    VPE_MIRROR_VERTICAL
};

enum vpe_scan_direction {
    VPE_SCAN_DIRECTION_UNKNOWN    = 0,
    VPE_SCAN_DIRECTION_HORIZONTAL = 1, /* 0, 180 rotation */
    VPE_SCAN_DIRECTION_VERTICAL   = 2, /* 90, 270 rotation */
};

struct vpe_size {
    uint32_t width;
    uint32_t height;
};

struct vpe_rect {
    int32_t  x;
    int32_t  y;
    uint32_t width;
    uint32_t height;
};

struct vpe_plane_size {
    struct vpe_rect surface_size;
    struct vpe_rect chroma_size;

    // actual aligned pitch and height
    uint32_t surface_pitch;
    uint32_t chroma_pitch;

    uint32_t surface_aligned_height;
    uint32_t chrome_aligned_height;
};

struct vpe_plane_dcc_param {
    bool enable;

    uint32_t meta_pitch;
    bool     independent_64b_blks;
    uint8_t  dcc_ind_blk;

    uint32_t meta_pitch_c;
    bool     independent_64b_blks_c;
    uint8_t  dcc_ind_blk_c;
};

/** Displayable pixel format in fb */
enum vpe_surface_pixel_format {
    VPE_SURFACE_PIXEL_FORMAT_GRPH_BEGIN = 0,
    /*16 bpp*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB1555,
    /*16 bpp*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB565,
    /*32 bpp*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888,
    /*32 bpp swaped*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888,
    /*32 bpp alpha rotated*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888,
    /*32 bpp swaped & alpha rotated*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888,

    VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010,
    /*swaped*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010,
    /*alpha rotated*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102,
    /*swaped & alpha rotated*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102,

    /*64 bpp */
    VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616,
    /*float*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F,
    /*swaped & float*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F,
    /*alpha rotated*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F,
    /*swaped & alpha rotated*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F,

    VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888,
    /*swaped*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888,
    /*rotated*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888,
    /*swaped & rotated*/
    VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888,
    /*grow graphics here if necessary */
    VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB111110_FIX,
    VPE_SURFACE_PIXEL_FORMAT_GRPH_BGR101111_FIX,
    VPE_SURFACE_PIXEL_FORMAT_GRPH_RGB111110_FLOAT,
    VPE_SURFACE_PIXEL_FORMAT_GRPH_BGR101111_FLOAT,
    VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBE,
    VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBE_ALPHA,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_BEGIN,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr = VPE_SURFACE_PIXEL_FORMAT_VIDEO_BEGIN,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_16bpc_YCrCb,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_422_CrYCbY,
    VPE_SURFACE_PIXEL_FORMAT_SUBSAMPLE_END = VPE_SURFACE_PIXEL_FORMAT_VIDEO_422_CrYCbY,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_ACrYCb2101010,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_CrYCbA1010102,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCrCb8888,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCbCr8888,
    VPE_SURFACE_PIXEL_FORMAT_VIDEO_END = VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCbCr8888,
    VPE_SURFACE_PIXEL_FORMAT_INVALID

    /*grow 444 video here if necessary */
};

enum vpe_swizzle_mode_values {
    VPE_SW_LINEAR   = 0,
    VPE_SW_256B_S   = 1,
    VPE_SW_256B_D   = 2,
    VPE_SW_256B_R   = 3,
    VPE_SW_4KB_Z    = 4,
    VPE_SW_4KB_S    = 5,
    VPE_SW_4KB_D    = 6,
    VPE_SW_4KB_R    = 7,
    VPE_SW_64KB_Z   = 8,
    VPE_SW_64KB_S   = 9,
    VPE_SW_64KB_D   = 10,
    VPE_SW_64KB_R   = 11,
    VPE_SW_VAR_Z    = 12,
    VPE_SW_VAR_S    = 13,
    VPE_SW_VAR_D    = 14,
    VPE_SW_VAR_R    = 15,
    VPE_SW_64KB_Z_T = 16,
    VPE_SW_64KB_S_T = 17,
    VPE_SW_64KB_D_T = 18,
    VPE_SW_64KB_R_T = 19,
    VPE_SW_4KB_Z_X  = 20,
    VPE_SW_4KB_S_X  = 21,
    VPE_SW_4KB_D_X  = 22,
    VPE_SW_4KB_R_X  = 23,
    VPE_SW_64KB_Z_X = 24,
    VPE_SW_64KB_S_X = 25,
    VPE_SW_64KB_D_X = 26,
    VPE_SW_64KB_R_X = 27,
    VPE_SW_VAR_Z_X  = 28,
    VPE_SW_VAR_S_X  = 29,
    VPE_SW_VAR_D_X  = 30,
    VPE_SW_VAR_R_X  = 31,
    VPE_SW_MAX      = 32,
    VPE_SW_UNKNOWN  = VPE_SW_MAX
};

/** specify the number of taps.
 * if 0 is specified, it will use 4 taps by default */
struct vpe_scaling_taps {
    uint32_t v_taps;
    uint32_t h_taps;
    uint32_t v_taps_c;
    uint32_t h_taps_c;
};

#ifdef __cplusplus
}
#endif
