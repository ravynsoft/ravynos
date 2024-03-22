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

#include "vpe_types.h"
#include "fixed31_32.h"
#include "hw_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SDR_VIDEO_WHITE_POINT 100 // nits
#define SDR_WHITE_POINT       80  // nits
#define HDR_PEAK_WHITE        10000

struct vpe_priv;
struct stream_ctx;

enum color_depth {
    COLOR_DEPTH_UNDEFINED,
    COLOR_DEPTH_666,
    COLOR_DEPTH_888,
    COLOR_DEPTH_101010,
    COLOR_DEPTH_121212,
    COLOR_DEPTH_141414,
    COLOR_DEPTH_161616,
    COLOR_DEPTH_999,
    COLOR_DEPTH_111111,
    COLOR_DEPTH_COUNT
};

enum color_transfer_func {
    TRANSFER_FUNC_UNKNOWN,
    TRANSFER_FUNC_SRGB,
    TRANSFER_FUNC_BT709,
    TRANSFER_FUNC_BT1886,
    TRANSFER_FUNC_PQ2084,
    TRANSFER_FUNC_LINEAR_0_125,
    TRANSFER_FUNC_NORMALIZED_PQ
};

enum dither_option {
    DITHER_OPTION_DEFAULT,
    DITHER_OPTION_DISABLE,
    DITHER_OPTION_FM6,
    DITHER_OPTION_FM8,
    DITHER_OPTION_FM10,
    DITHER_OPTION_SPATIAL6_FRAME_RANDOM,
    DITHER_OPTION_SPATIAL8_FRAME_RANDOM,
    DITHER_OPTION_SPATIAL10_FRAME_RANDOM,
    DITHER_OPTION_SPATIAL6,
    DITHER_OPTION_SPATIAL8,
    DITHER_OPTION_SPATIAL10,
    DITHER_OPTION_TRUN6,
    DITHER_OPTION_TRUN8,
    DITHER_OPTION_TRUN10,
    DITHER_OPTION_TRUN10_SPATIAL8,
    DITHER_OPTION_TRUN10_SPATIAL6,
    DITHER_OPTION_TRUN10_FM8,
    DITHER_OPTION_TRUN10_FM6,
    DITHER_OPTION_TRUN10_SPATIAL8_FM6,
    DITHER_OPTION_SPATIAL10_FM8,
    DITHER_OPTION_SPATIAL10_FM6,
    DITHER_OPTION_TRUN8_SPATIAL6,
    DITHER_OPTION_TRUN8_FM6,
    DITHER_OPTION_SPATIAL8_FM6,
    DITHER_OPTION_MAX = DITHER_OPTION_SPATIAL8_FM6,
    DITHER_OPTION_INVALID
};

enum color_space {
    COLOR_SPACE_UNKNOWN,
    COLOR_SPACE_SRGB,
    COLOR_SPACE_SRGB_LIMITED,
    COLOR_SPACE_MSREF_SCRGB,
    COLOR_SPACE_YCBCR601,
    COLOR_SPACE_YCBCR709,
    COLOR_SPACE_JFIF,
    COLOR_SPACE_YCBCR601_LIMITED,
    COLOR_SPACE_YCBCR709_LIMITED,
    COLOR_SPACE_2020_RGB_FULLRANGE,
    COLOR_SPACE_2020_RGB_LIMITEDRANGE,
    COLOR_SPACE_2020_YCBCR,
    COLOR_SPACE_2020_YCBCR_LIMITED,
    COLOR_SPACE_MAX,
};

enum transfer_func_type {
    TF_TYPE_PREDEFINED,
    TF_TYPE_DISTRIBUTED_POINTS,
    TF_TYPE_BYPASS,
    TF_TYPE_HWPWL
};

enum {
    TRANSFER_FUNC_POINTS = 1025
};

typedef struct fixed31_32 white_point_gain;

struct transfer_func_distributed_points {
    struct fixed31_32 red[TRANSFER_FUNC_POINTS];
    struct fixed31_32 green[TRANSFER_FUNC_POINTS];
    struct fixed31_32 blue[TRANSFER_FUNC_POINTS];

    uint16_t end_exponent;
    uint16_t x_point_at_y1_red;
    uint16_t x_point_at_y1_green;
    uint16_t x_point_at_y1_blue;
};

struct transfer_func {
    enum transfer_func_type  type;
    enum color_transfer_func tf;

    /* FP16 1.0 reference level in nits, default is 80 nits, only for PQ*/
    uint32_t sdr_ref_white_level;
    union {
        struct pwl_params                       pwl;
        struct transfer_func_distributed_points tf_pts;
    };
    bool use_pre_calculated_table;
};

enum color_white_point_type {
    color_white_point_type_unknown,
    color_white_point_type_5000k_horizon,
    color_white_point_type_6500k_noon,
    color_white_point_type_7500k_north_sky,
    color_white_point_type_9300k,
    color_white_point_type_custom_coordinates
};

struct color_space_coordinates {
    unsigned int redX;
    unsigned int redY;
    unsigned int greenX;
    unsigned int greenY;
    unsigned int blueX;
    unsigned int blueY;
    unsigned int whiteX;
    unsigned int whiteY;
};

enum predefined_gamut_type {
    gamut_type_bt709,
    gamut_type_bt601,
    gamut_type_adobe_rgb,
    gamut_type_srgb,
    gamut_type_bt2020,
    gamut_type_dcip3,
    gamut_type_unknown,
};

enum predefined_white_point_type {
    white_point_type_5000k_horizon,
    white_point_type_6500k_noon,
    white_point_type_7500k_north_sky,
    white_point_type_9300k,
    white_point_type_unknown,
};

struct colorspace_transform {
    struct fixed31_32 matrix[12];
    bool              enable_remap;
};

struct color_gamut_data {
    enum color_space               color_space;
    enum color_white_point_type    white_point;
    struct color_space_coordinates gamut;
};

union vpe_3dlut_state {
    struct {
        uint32_t initialized : 1; /*if 3dlut is went through color module for initialization */
        uint32_t reserved    : 15;
    } bits;
    uint32_t raw;
};

struct vpe_3dlut {
    // struct kref refcount;
    struct tetrahedral_params lut_3d;
    struct fixed31_32         hdr_multiplier;
    union vpe_3dlut_state     state;
};

enum vpe_status vpe_color_update_color_space_and_tf(
    struct vpe_priv *vpe_priv, const struct vpe_build_param *param);

enum vpe_status vpe_color_update_movable_cm(
    struct vpe_priv *vpe_priv, const struct vpe_build_param *param);

void vpe_color_get_color_space_and_tf(
    const struct vpe_color_space *vcs, enum color_space *cs, enum color_transfer_func *tf);

bool vpe_use_csc_adjust(const struct vpe_color_adjust *adjustments);

bool vpe_is_rgb_equal(const struct pwl_result_data *rgb, uint32_t num);

bool vpe_is_HDR(enum color_transfer_func tf);

void vpe_convert_full_range_color_enum(enum color_space *cs);

enum vpe_status vpe_color_update_whitepoint(
    const struct vpe_priv *vpe_priv, const struct vpe_build_param *param);

enum vpe_status vpe_color_tm_update_hdr_mult(uint16_t shaper_in_exp_max, uint32_t peak_white,
    struct fixed31_32 *hdr_multiplier, bool enable_3dlut);

enum vpe_status vpe_color_update_shaper(
    uint16_t shaper_in_exp_max, struct transfer_func *shaper_func, bool enable_3dlut);

enum vpe_status vpe_color_update_blnd_gam(struct vpe_priv *vpe_priv,
    const struct vpe_build_param *param, const struct vpe_tonemap_params *tm_params,
    struct transfer_func *blnd_tf_func, bool enable_3dlut);

enum vpe_status vpe_color_build_tm_cs(const struct vpe_tonemap_params *tm_params,
    struct vpe_surface_info surface_info, struct vpe_color_space *vcs);

#ifdef __cplusplus
}
#endif
