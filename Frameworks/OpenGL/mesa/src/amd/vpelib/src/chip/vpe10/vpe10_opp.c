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

#include <string.h>
#include "common.h"
#include "vpe_priv.h"
#include "vpe10_opp.h"
#include "vpe_command.h"
#include "hw_shared.h"
#include "reg_helper.h"

#define CTX_BASE opp
#define CTX      vpe10_opp

static struct opp_funcs opp_funcs = {
    .program_pipe_alpha          = vpe10_opp_program_pipe_alpha,
    .program_pipe_bypass         = vpe10_opp_program_pipe_bypass,
    .program_pipe_crc            = vpe10_opp_program_pipe_crc,
    .set_clamping                = vpe10_opp_set_clamping,
    .set_truncation              = vpe10_opp_set_truncation,
    .set_spatial_dither          = vpe10_opp_set_spatial_dither,
    .program_bit_depth_reduction = vpe10_opp_program_bit_depth_reduction,
    .set_dyn_expansion           = vpe10_opp_set_dyn_expansion,
    .program_fmt                 = vpe10_opp_program_fmt,
};

void vpe10_construct_opp(struct vpe_priv *vpe_priv, struct opp *opp)
{
    opp->vpe_priv = vpe_priv;
    opp->funcs    = &opp_funcs;
}

void vpe10_opp_set_clamping(
    struct opp *opp, const struct clamping_and_pixel_encoding_params *params)
{
    PROGRAM_ENTRY();

    switch (params->clamping_level) {
    case CLAMPING_LIMITED_RANGE_8BPC:
    case CLAMPING_LIMITED_RANGE_10BPC:
    case CLAMPING_LIMITED_RANGE_12BPC:
        REG_SET_2(VPFMT_CLAMP_CNTL, 0, VPFMT_CLAMP_DATA_EN, 1, VPFMT_CLAMP_COLOR_FORMAT,
            params->clamping_level);
        break;
    case CLAMPING_LIMITED_RANGE_PROGRAMMABLE:
        REG_SET_2(VPFMT_CLAMP_CNTL, 0, VPFMT_CLAMP_DATA_EN, 1, VPFMT_CLAMP_COLOR_FORMAT, 7);
        REG_SET_2(VPFMT_CLAMP_COMPONENT_R, 0, VPFMT_CLAMP_LOWER_R, params->r_clamp_component_lower,
            VPFMT_CLAMP_UPPER_R, params->r_clamp_component_upper);
        REG_SET_2(VPFMT_CLAMP_COMPONENT_G, 0, VPFMT_CLAMP_LOWER_G, params->g_clamp_component_lower,
            VPFMT_CLAMP_UPPER_G, params->g_clamp_component_upper);
        REG_SET_2(VPFMT_CLAMP_COMPONENT_B, 0, VPFMT_CLAMP_LOWER_B, params->b_clamp_component_lower,
            VPFMT_CLAMP_UPPER_B, params->b_clamp_component_upper);
        break;
    case CLAMPING_FULL_RANGE:
    default:
        REG_SET_2(VPFMT_CLAMP_CNTL, 0, VPFMT_CLAMP_DATA_EN, 0, VPFMT_CLAMP_COLOR_FORMAT, 0);
        break;
    }
}

void vpe10_opp_set_dyn_expansion(struct opp *opp, bool enable, enum color_depth color_dpth)
{
    PROGRAM_ENTRY();

    if (!enable) {
        REG_SET_2(VPFMT_DYNAMIC_EXP_CNTL, 0, VPFMT_DYNAMIC_EXP_EN, 0, VPFMT_DYNAMIC_EXP_MODE, 0);
        return;
    }

    /*00 - 10-bit -> 12-bit dynamic expansion*/
    /*01 - 8-bit  -> 12-bit dynamic expansion*/
    switch (color_dpth) {
    case COLOR_DEPTH_888:
        REG_SET_2(VPFMT_DYNAMIC_EXP_CNTL, 0, VPFMT_DYNAMIC_EXP_EN, 1, VPFMT_DYNAMIC_EXP_MODE, 1);
        break;
    case COLOR_DEPTH_101010:
        REG_SET_2(VPFMT_DYNAMIC_EXP_CNTL, 0, VPFMT_DYNAMIC_EXP_EN, 1, VPFMT_DYNAMIC_EXP_MODE, 0);
        break;
    case COLOR_DEPTH_121212:
        REG_SET_2(VPFMT_DYNAMIC_EXP_CNTL, 0, VPFMT_DYNAMIC_EXP_EN,
            1, /*otherwise last two bits are zero*/
            VPFMT_DYNAMIC_EXP_MODE, 0);
        break;
    default:
        REG_SET_2(VPFMT_DYNAMIC_EXP_CNTL, 0, VPFMT_DYNAMIC_EXP_EN, 0, VPFMT_DYNAMIC_EXP_MODE, 0);
        break;
    }
}

void vpe10_opp_set_truncation(struct opp *opp, const struct bit_depth_reduction_params *params)
{
    PROGRAM_ENTRY();

    REG_UPDATE_3(VPFMT_BIT_DEPTH_CONTROL, VPFMT_TRUNCATE_EN, params->flags.TRUNCATE_ENABLED,
        VPFMT_TRUNCATE_DEPTH, params->flags.TRUNCATE_DEPTH, VPFMT_TRUNCATE_MODE,
        params->flags.TRUNCATE_MODE);
}

void vpe10_opp_set_spatial_dither(struct opp *opp, const struct bit_depth_reduction_params *params)
{
    PROGRAM_ENTRY();

    /*Disable spatial (random) dithering*/
    REG_UPDATE_6(VPFMT_BIT_DEPTH_CONTROL, VPFMT_SPATIAL_DITHER_EN, 0, VPFMT_SPATIAL_DITHER_MODE, 0,
        VPFMT_SPATIAL_DITHER_DEPTH, 0, VPFMT_HIGHPASS_RANDOM_ENABLE, 0, VPFMT_FRAME_RANDOM_ENABLE,
        0, VPFMT_RGB_RANDOM_ENABLE, 0);

    if (params->flags.SPATIAL_DITHER_ENABLED == 0)
        return;

    /* only use FRAME_COUNTER_MAX if frameRandom == 1*/
    if (params->flags.FRAME_RANDOM == 1) {
        if (params->flags.SPATIAL_DITHER_DEPTH == 0 || params->flags.SPATIAL_DITHER_DEPTH == 1) {
            REG_UPDATE_2(VPFMT_CONTROL, VPFMT_SPATIAL_DITHER_FRAME_COUNTER_MAX, 15,
                VPFMT_SPATIAL_DITHER_FRAME_COUNTER_BIT_SWAP, 2);
        } else if (params->flags.SPATIAL_DITHER_DEPTH == 2) {
            REG_UPDATE_2(VPFMT_CONTROL, VPFMT_SPATIAL_DITHER_FRAME_COUNTER_MAX, 3,
                VPFMT_SPATIAL_DITHER_FRAME_COUNTER_BIT_SWAP, 1);
        } else
            return;
    } else {
        REG_UPDATE_2(VPFMT_CONTROL, VPFMT_SPATIAL_DITHER_FRAME_COUNTER_MAX, 0,
            VPFMT_SPATIAL_DITHER_FRAME_COUNTER_BIT_SWAP, 0);
    }

    /* Set seed for random values for
     * spatial dithering for R,G,B channels
     */
    REG_SET(VPFMT_DITHER_RAND_R_SEED, 0, VPFMT_RAND_R_SEED, params->r_seed_value);

    REG_SET(VPFMT_DITHER_RAND_G_SEED, 0, VPFMT_RAND_G_SEED, params->g_seed_value);

    REG_SET(VPFMT_DITHER_RAND_B_SEED, 0, VPFMT_RAND_B_SEED, params->b_seed_value);

    /* FMT_OFFSET_R_Cr  31:16 0x0 Setting the zero
     * offset for the R/Cr channel, lower 4LSB
     * is forced to zeros. Typically set to 0
     * RGB and 0x80000 YCbCr.
     */
    /* FMT_OFFSET_G_Y   31:16 0x0 Setting the zero
     * offset for the G/Y  channel, lower 4LSB is
     * forced to zeros. Typically set to 0 RGB
     * and 0x80000 YCbCr.
     */
    /* FMT_OFFSET_B_Cb  31:16 0x0 Setting the zero
     * offset for the B/Cb channel, lower 4LSB is
     * forced to zeros. Typically set to 0 RGB and
     * 0x80000 YCbCr.
     */

    REG_UPDATE_6(VPFMT_BIT_DEPTH_CONTROL,
        /*Enable spatial dithering*/
        VPFMT_SPATIAL_DITHER_EN, params->flags.SPATIAL_DITHER_ENABLED,
        /* Set spatial dithering mode
         * (default is Seed patterrn AAAA...)
         */
        VPFMT_SPATIAL_DITHER_MODE, params->flags.SPATIAL_DITHER_MODE,
        /*Set spatial dithering bit depth*/
        VPFMT_SPATIAL_DITHER_DEPTH, params->flags.SPATIAL_DITHER_DEPTH,
        /*Disable High pass filter*/
        VPFMT_HIGHPASS_RANDOM_ENABLE, params->flags.HIGHPASS_RANDOM,
        /*Reset only at startup*/
        VPFMT_FRAME_RANDOM_ENABLE, params->flags.FRAME_RANDOM,
        /*Set RGB data dithered with x^28+x^3+1*/
        VPFMT_RGB_RANDOM_ENABLE, params->flags.RGB_RANDOM);
}

void vpe10_opp_program_bit_depth_reduction(
    struct opp *opp, const struct bit_depth_reduction_params *fmt_bit_depth)
{
    opp->funcs->set_truncation(opp, fmt_bit_depth);
    opp->funcs->set_spatial_dither(opp, fmt_bit_depth);
}

void vpe10_opp_program_fmt(struct opp *opp, struct bit_depth_reduction_params *fmt_bit_depth,
    struct clamping_and_pixel_encoding_params *clamping)
{
    opp->funcs->program_bit_depth_reduction(opp, fmt_bit_depth);
    opp->funcs->set_clamping(opp, clamping);
}

void vpe10_opp_program_pipe_alpha(struct opp *opp, uint16_t alpha)
{
    PROGRAM_ENTRY();
    REG_UPDATE(VPOPP_PIPE_CONTROL, VPOPP_PIPE_ALPHA, alpha);
}

void vpe10_opp_program_pipe_bypass(struct opp *opp, bool enable)
{
    PROGRAM_ENTRY();
    REG_UPDATE(VPOPP_PIPE_CONTROL, VPOPP_PIPE_DIGITAL_BYPASS_EN, enable);
}

void vpe10_opp_program_pipe_crc(struct opp *opp, bool enable)
{
    PROGRAM_ENTRY();
    REG_UPDATE(VPOPP_PIPE_CRC_CONTROL, VPOPP_PIPE_CRC_EN, enable);
}

