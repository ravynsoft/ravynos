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

#include "opp.h"
#include "reg_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OPP_REG_LIST_VPE10(id)                                                                     \
    SRIDFVL(VPFMT_CLAMP_COMPONENT_R, VPFMT, id), SRIDFVL(VPFMT_CLAMP_COMPONENT_G, VPFMT, id),      \
        SRIDFVL(VPFMT_CLAMP_COMPONENT_B, VPFMT, id), SRIDFVL(VPFMT_DYNAMIC_EXP_CNTL, VPFMT, id),   \
        SRIDFVL(VPFMT_CONTROL, VPFMT, id), SRIDFVL(VPFMT_BIT_DEPTH_CONTROL, VPFMT, id),            \
        SRIDFVL(VPFMT_DITHER_RAND_R_SEED, VPFMT, id),                                              \
        SRIDFVL(VPFMT_DITHER_RAND_G_SEED, VPFMT, id),                                              \
        SRIDFVL(VPFMT_DITHER_RAND_B_SEED, VPFMT, id), SRIDFVL(VPFMT_CLAMP_CNTL, VPFMT, id),        \
        SRIDFVL(VPOPP_PIPE_CONTROL, VPOPP_PIPE, id),                                               \
        SRIDFVL(VPOPP_TOP_CLK_CONTROL, VPOPP_TOP, id),                                             \
        SRIDFVL(VPOPP_PIPE_CRC_CONTROL, VPOPP_PIPE_CRC, id),

#define OPP_FIELD_LIST_VPE10(post_fix)                                                             \
    SFRB(VPFMT_CLAMP_LOWER_R, VPFMT_CLAMP_COMPONENT_R, post_fix),                                  \
        SFRB(VPFMT_CLAMP_UPPER_R, VPFMT_CLAMP_COMPONENT_R, post_fix),                              \
        SFRB(VPFMT_CLAMP_LOWER_G, VPFMT_CLAMP_COMPONENT_G, post_fix),                              \
        SFRB(VPFMT_CLAMP_UPPER_G, VPFMT_CLAMP_COMPONENT_G, post_fix),                              \
        SFRB(VPFMT_CLAMP_LOWER_B, VPFMT_CLAMP_COMPONENT_B, post_fix),                              \
        SFRB(VPFMT_CLAMP_UPPER_B, VPFMT_CLAMP_COMPONENT_B, post_fix),                              \
        SFRB(VPFMT_DYNAMIC_EXP_EN, VPFMT_DYNAMIC_EXP_CNTL, post_fix),                              \
        SFRB(VPFMT_DYNAMIC_EXP_MODE, VPFMT_DYNAMIC_EXP_CNTL, post_fix),                            \
        SFRB(VPFMT_SPATIAL_DITHER_FRAME_COUNTER_MAX, VPFMT_CONTROL, post_fix),                     \
        SFRB(VPFMT_SPATIAL_DITHER_FRAME_COUNTER_BIT_SWAP, VPFMT_CONTROL, post_fix),                \
        SFRB(VPFMT_CBCR_BIT_REDUCTION_BYPASS, VPFMT_CONTROL, post_fix),                            \
        SFRB(VPFMT_DOUBLE_BUFFER_REG_UPDATE_PENDING, VPFMT_CONTROL, post_fix),                     \
        SFRB(VPFMT_TRUNCATE_EN, VPFMT_BIT_DEPTH_CONTROL, post_fix),                                \
        SFRB(VPFMT_TRUNCATE_MODE, VPFMT_BIT_DEPTH_CONTROL, post_fix),                              \
        SFRB(VPFMT_TRUNCATE_DEPTH, VPFMT_BIT_DEPTH_CONTROL, post_fix),                             \
        SFRB(VPFMT_SPATIAL_DITHER_EN, VPFMT_BIT_DEPTH_CONTROL, post_fix),                          \
        SFRB(VPFMT_SPATIAL_DITHER_MODE, VPFMT_BIT_DEPTH_CONTROL, post_fix),                        \
        SFRB(VPFMT_SPATIAL_DITHER_DEPTH, VPFMT_BIT_DEPTH_CONTROL, post_fix),                       \
        SFRB(VPFMT_FRAME_RANDOM_ENABLE, VPFMT_BIT_DEPTH_CONTROL, post_fix),                        \
        SFRB(VPFMT_RGB_RANDOM_ENABLE, VPFMT_BIT_DEPTH_CONTROL, post_fix),                          \
        SFRB(VPFMT_HIGHPASS_RANDOM_ENABLE, VPFMT_BIT_DEPTH_CONTROL, post_fix),                     \
        SFRB(VPFMT_RAND_R_SEED, VPFMT_DITHER_RAND_R_SEED, post_fix),                               \
        SFRB(VPFMT_OFFSET_R_CR, VPFMT_DITHER_RAND_R_SEED, post_fix),                               \
        SFRB(VPFMT_RAND_G_SEED, VPFMT_DITHER_RAND_G_SEED, post_fix),                               \
        SFRB(VPFMT_OFFSET_G_Y, VPFMT_DITHER_RAND_G_SEED, post_fix),                                \
        SFRB(VPFMT_RAND_B_SEED, VPFMT_DITHER_RAND_B_SEED, post_fix),                               \
        SFRB(VPFMT_OFFSET_B_CB, VPFMT_DITHER_RAND_B_SEED, post_fix),                               \
        SFRB(VPFMT_CLAMP_DATA_EN, VPFMT_CLAMP_CNTL, post_fix),                                     \
        SFRB(VPFMT_CLAMP_COLOR_FORMAT, VPFMT_CLAMP_CNTL, post_fix),                                \
        SFRB(VPOPP_PIPE_CLOCK_ON, VPOPP_PIPE_CONTROL, post_fix),                                   \
        SFRB(VPOPP_PIPE_DIGITAL_BYPASS_EN, VPOPP_PIPE_CONTROL, post_fix),                          \
        SFRB(VPOPP_PIPE_ALPHA, VPOPP_PIPE_CONTROL, post_fix),                                      \
        SFRB(VPOPP_VPECLK_R_GATE_DIS, VPOPP_TOP_CLK_CONTROL, post_fix),                            \
        SFRB(VPOPP_VPECLK_G_GATE_DIS, VPOPP_TOP_CLK_CONTROL, post_fix),                            \
        SFRB(VPOPP_PIPE_CRC_EN, VPOPP_PIPE_CRC_CONTROL, post_fix),                                 \
        SFRB(VPOPP_PIPE_CRC_CONT_EN, VPOPP_PIPE_CRC_CONTROL, post_fix),                            \
        SFRB(VPOPP_PIPE_CRC_PIXEL_SELECT, VPOPP_PIPE_CRC_CONTROL, post_fix)

#define OPP_REG_VARIABLE_LIST_VPE10                                                                \
    reg_id_val VPFMT_CLAMP_COMPONENT_R;                                                            \
    reg_id_val VPFMT_CLAMP_COMPONENT_G;                                                            \
    reg_id_val VPFMT_CLAMP_COMPONENT_B;                                                            \
    reg_id_val VPFMT_DYNAMIC_EXP_CNTL;                                                             \
    reg_id_val VPFMT_CONTROL;                                                                      \
    reg_id_val VPFMT_BIT_DEPTH_CONTROL;                                                            \
    reg_id_val VPFMT_DITHER_RAND_R_SEED;                                                           \
    reg_id_val VPFMT_DITHER_RAND_G_SEED;                                                           \
    reg_id_val VPFMT_DITHER_RAND_B_SEED;                                                           \
    reg_id_val VPFMT_CLAMP_CNTL;                                                                   \
    reg_id_val VPOPP_PIPE_CONTROL;                                                                 \
    reg_id_val VPOPP_TOP_CLK_CONTROL;                                                              \
    reg_id_val VPOPP_PIPE_CRC_CONTROL;

#define OPP_FIELD_VARIABLE_LIST_VPE10(type)                                                        \
    type VPFMT_CLAMP_LOWER_R;                                                                      \
    type VPFMT_CLAMP_UPPER_R;                                                                      \
    type VPFMT_CLAMP_LOWER_G;                                                                      \
    type VPFMT_CLAMP_UPPER_G;                                                                      \
    type VPFMT_CLAMP_LOWER_B;                                                                      \
    type VPFMT_CLAMP_UPPER_B;                                                                      \
    type VPFMT_DYNAMIC_EXP_EN;                                                                     \
    type VPFMT_DYNAMIC_EXP_MODE;                                                                   \
    type VPFMT_SPATIAL_DITHER_FRAME_COUNTER_MAX;                                                   \
    type VPFMT_SPATIAL_DITHER_FRAME_COUNTER_BIT_SWAP;                                              \
    type VPFMT_CBCR_BIT_REDUCTION_BYPASS;                                                          \
    type VPFMT_DOUBLE_BUFFER_REG_UPDATE_PENDING;                                                   \
    type VPFMT_TRUNCATE_EN;                                                                        \
    type VPFMT_TRUNCATE_MODE;                                                                      \
    type VPFMT_TRUNCATE_DEPTH;                                                                     \
    type VPFMT_SPATIAL_DITHER_EN;                                                                  \
    type VPFMT_SPATIAL_DITHER_MODE;                                                                \
    type VPFMT_SPATIAL_DITHER_DEPTH;                                                               \
    type VPFMT_FRAME_RANDOM_ENABLE;                                                                \
    type VPFMT_RGB_RANDOM_ENABLE;                                                                  \
    type VPFMT_HIGHPASS_RANDOM_ENABLE;                                                             \
    type VPFMT_RAND_R_SEED;                                                                        \
    type VPFMT_OFFSET_R_CR;                                                                        \
    type VPFMT_RAND_G_SEED;                                                                        \
    type VPFMT_OFFSET_G_Y;                                                                         \
    type VPFMT_RAND_B_SEED;                                                                        \
    type VPFMT_OFFSET_B_CB;                                                                        \
    type VPFMT_CLAMP_DATA_EN;                                                                      \
    type VPFMT_CLAMP_COLOR_FORMAT;                                                                 \
    type VPOPP_PIPE_CLOCK_ON;                                                                      \
    type VPOPP_PIPE_DIGITAL_BYPASS_EN;                                                             \
    type VPOPP_PIPE_ALPHA;                                                                         \
    type VPOPP_VPECLK_R_GATE_DIS;                                                                  \
    type VPOPP_VPECLK_G_GATE_DIS;                                                                  \
    type VPOPP_PIPE_CRC_EN;                                                                        \
    type VPOPP_PIPE_CRC_CONT_EN;                                                                   \
    type VPOPP_PIPE_CRC_PIXEL_SELECT;

struct vpe10_opp_registers {
    OPP_REG_VARIABLE_LIST_VPE10
};

struct vpe10_opp_shift {
    OPP_FIELD_VARIABLE_LIST_VPE10(uint8_t)
};

struct vpe10_opp_mask {
    OPP_FIELD_VARIABLE_LIST_VPE10(uint32_t)
};

struct vpe10_opp {
    struct opp                    base;
    struct vpe10_opp_registers   *regs;
    const struct vpe10_opp_shift *shift;
    const struct vpe10_opp_mask  *mask;
};

void vpe10_construct_opp(struct vpe_priv *vpe_priv, struct opp *opp);

enum color_depth vpe10_opp_check_color_depth(enum vpe_surface_pixel_format format);

void vpe10_opp_set_clamping(
    struct opp *opp, const struct clamping_and_pixel_encoding_params *params);

void vpe10_opp_set_dyn_expansion(struct opp *opp, bool enable, enum color_depth color_dpth);

void vpe10_opp_set_truncation(struct opp *opp, const struct bit_depth_reduction_params *params);

void vpe10_opp_set_spatial_dither(struct opp *opp, const struct bit_depth_reduction_params *params);

void vpe10_opp_program_bit_depth_reduction(
    struct opp *opp, const struct bit_depth_reduction_params *fmt_bit_depth);

void vpe10_opp_program_fmt(struct opp *opp, struct bit_depth_reduction_params *fmt_bit_depth,
    struct clamping_and_pixel_encoding_params *clamping);

void vpe10_opp_program_pipe_alpha(struct opp *opp, uint16_t alpha);

void vpe10_opp_program_pipe_bypass(struct opp *opp, bool enable);

void vpe10_opp_program_pipe_crc(struct opp *opp, bool enable);
#ifdef __cplusplus
}
#endif
