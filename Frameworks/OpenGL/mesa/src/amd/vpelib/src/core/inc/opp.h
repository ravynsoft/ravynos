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

#ifdef __cplusplus
extern "C" {
#endif

struct opp;
struct vpe_priv;

enum clamping_range {
    CLAMPING_FULL_RANGE = 0,      /* No Clamping */
    CLAMPING_LIMITED_RANGE_8BPC,  /* 8  bpc: Clamping 1  to FE */
    CLAMPING_LIMITED_RANGE_10BPC, /* 10 bpc: Clamping 4  to 3FB */
    CLAMPING_LIMITED_RANGE_12BPC, /* 12 bpc: Clamping 10 to FEF */
    /* Use programmable clampping value on FMT_CLAMP_COMPONENT_R/G/B. */
    CLAMPING_LIMITED_RANGE_PROGRAMMABLE
};

struct clamping_and_pixel_encoding_params {
    // enum vpe_pixel_encoding pixel_encoding; /* Pixel Encoding, not used and not initialized yet
    // */
    enum clamping_range clamping_level; /* Clamping identifier */
    enum color_depth    c_depth;        /* Deep color use. */
    uint32_t            r_clamp_component_upper;
    uint32_t            b_clamp_component_upper;
    uint32_t            g_clamp_component_upper;
    uint32_t            r_clamp_component_lower;
    uint32_t            b_clamp_component_lower;
    uint32_t            g_clamp_component_lower;
};

struct bit_depth_reduction_params {
    struct {
        /* truncate/round */
        /* trunc/round enabled*/
        uint32_t TRUNCATE_ENABLED : 1;
        /* 2 bits: 0=6 bpc, 1=8 bpc, 2 = 10bpc*/
        uint32_t TRUNCATE_DEPTH : 2;
        /* truncate or round*/
        uint32_t TRUNCATE_MODE : 1;

        /* spatial dither */
        /* Spatial Bit Depth Reduction enabled*/
        uint32_t SPATIAL_DITHER_ENABLED : 1;
        /* 2 bits: 0=6 bpc, 1 = 8 bpc, 2 = 10bpc*/
        uint32_t SPATIAL_DITHER_DEPTH : 2;
        /* 0-3 to select patterns*/
        uint32_t SPATIAL_DITHER_MODE : 2;
        /* Enable RGB random dithering*/
        uint32_t RGB_RANDOM : 1;
        /* Enable Frame random dithering*/
        uint32_t FRAME_RANDOM : 1;
        /* Enable HighPass random dithering*/
        uint32_t HIGHPASS_RANDOM : 1;

        /* temporal dither*/
        /* frame modulation enabled*/
        uint32_t FRAME_MODULATION_ENABLED : 1;
        /* same as for trunc/spatial*/
        uint32_t FRAME_MODULATION_DEPTH : 2;
        /* 2/4 gray levels*/
        uint32_t TEMPORAL_LEVEL : 1;
        uint32_t FRC25          : 2;
        uint32_t FRC50          : 2;
        uint32_t FRC75          : 2;
    } flags;

    uint32_t r_seed_value;
    uint32_t b_seed_value;
    uint32_t g_seed_value;
    //   enum vpe_pixel_encoding pixel_encoding;  // not used and not initialized yet
};

struct opp_funcs {

    void (*set_clamping)(struct opp *opp, const struct clamping_and_pixel_encoding_params *params);

    void (*set_dyn_expansion)(struct opp *opp, bool enable, enum color_depth color_dpth);

    void (*set_truncation)(struct opp *opp, const struct bit_depth_reduction_params *params);

    void (*set_spatial_dither)(struct opp *opp, const struct bit_depth_reduction_params *params);

    void (*program_bit_depth_reduction)(
        struct opp *opp, const struct bit_depth_reduction_params *params);

    void (*program_fmt)(struct opp *opp, struct bit_depth_reduction_params *fmt_bit_depth,
        struct clamping_and_pixel_encoding_params *clamping);

    void (*program_pipe_alpha)(struct opp *opp, uint16_t alpha);

    void (*program_pipe_bypass)(struct opp *opp, bool enable);
    void (*program_pipe_crc)(struct opp *opp, bool enable);
};

struct opp {
    struct vpe_priv  *vpe_priv;
    struct opp_funcs *funcs;
};

#ifdef __cplusplus
}
#endif
