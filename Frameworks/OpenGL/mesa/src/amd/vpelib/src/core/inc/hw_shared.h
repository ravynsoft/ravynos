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

#include "fixed31_32.h"

#ifdef __cplusplus
extern "C" {
#endif

enum cm_type {
    CM_DEGAM,
    CM_REGAM,
};

struct bias_and_scale {
    uint32_t scale_red;
    uint32_t bias_red;
    uint32_t scale_green;
    uint32_t bias_green;
    uint32_t scale_blue;
    uint32_t bias_blue;
};

struct gamma_curve {
    uint32_t offset;
    uint32_t segments_num;
};

struct curve_points {
    struct fixed31_32 x;
    struct fixed31_32 y;
    struct fixed31_32 offset;
    struct fixed31_32 slope;

    uint32_t custom_float_x;
    uint32_t custom_float_y;
    uint32_t custom_float_offset;
    uint32_t custom_float_slope;
};

struct curve_points3 {
    struct curve_points red;
    struct curve_points green;
    struct curve_points blue;
};

struct pwl_result_data {
    struct fixed31_32 red;
    struct fixed31_32 green;
    struct fixed31_32 blue;

    struct fixed31_32 delta_red;
    struct fixed31_32 delta_green;
    struct fixed31_32 delta_blue;

    uint32_t red_reg;
    uint32_t green_reg;
    uint32_t blue_reg;

    uint32_t delta_red_reg;
    uint32_t delta_green_reg;
    uint32_t delta_blue_reg;
};

/* arr_curve_points - regamma regions/segments specification
 * arr_points - beginning and end point specified separately (only one on DCE)
 * corner_points - beginning and end point for all 3 colors (DCN)
 * rgb_resulted - final curve
 */
struct pwl_params {
    struct gamma_curve arr_curve_points[34];
    union {
        struct curve_points  arr_points[2];
        struct curve_points3 corner_points[2];
    };
    struct pwl_result_data rgb_resulted[256 + 3];
    uint32_t               hw_points_num;
};

struct hw_x_point {
    uint32_t          custom_float_x;
    struct fixed31_32 x;
    struct fixed31_32 regamma_y_red;
    struct fixed31_32 regamma_y_green;
    struct fixed31_32 regamma_y_blue;
};

struct gamma_coefficients {
    struct fixed31_32 a0[3];
    struct fixed31_32 a1[3];
    struct fixed31_32 a2[3];
    struct fixed31_32 a3[3];
    struct fixed31_32 user_gamma[3];
    struct fixed31_32 user_contrast;
    struct fixed31_32 user_brightness;
};

struct pwl_float_data_ex {
    struct fixed31_32 r;
    struct fixed31_32 g;
    struct fixed31_32 b;
    struct fixed31_32 delta_r;
    struct fixed31_32 delta_g;
    struct fixed31_32 delta_b;
};

enum hw_point_position {
    /* hw point sits between left and right sw points */
    HW_POINT_POSITION_MIDDLE,
    /* hw point lays left from left (smaller) sw point */
    HW_POINT_POSITION_LEFT,
    /* hw point lays stays from right (bigger) sw point */
    HW_POINT_POSITION_RIGHT
};

struct gamma_point {
    int32_t                left_index;
    int32_t                right_index;
    enum hw_point_position pos;
    struct fixed31_32      coeff;
};

struct pixel_gamma_point {
    struct gamma_point r;
    struct gamma_point g;
    struct gamma_point b;
};

enum gamut_remap_select {
    GAMUT_REMAP_BYPASS = 0,
    GAMUT_REMAP_COMA_COEFF,
};

struct vpe_rgb {
    uint32_t red;
    uint32_t green;
    uint32_t blue;
};

struct tetrahedral_17x17x17 {
    struct vpe_rgb lut0[1229];
    struct vpe_rgb lut1[1228];
    struct vpe_rgb lut2[1228];
    struct vpe_rgb lut3[1228];
};
struct tetrahedral_9x9x9 {
    struct vpe_rgb lut0[183];
    struct vpe_rgb lut1[182];
    struct vpe_rgb lut2[182];
    struct vpe_rgb lut3[182];
};

struct tetrahedral_params {
    union {
        struct tetrahedral_17x17x17 tetrahedral_17;
        struct tetrahedral_9x9x9    tetrahedral_9;
    };
    bool use_tetrahedral_9;
    bool use_12bits;
};

enum vpe_lut_mode {
    LUT_BYPASS,
    LUT_RAM_A,
    LUT_RAM_B
};

#ifdef __cplusplus
}
#endif
