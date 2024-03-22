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

#include "color.h"
#include "hw_shared.h"
#include "color_pwl.h"

#ifdef __cplusplus
extern "C" {
#endif

struct config_writer;

#define TF_HELPER_REG_FIELD_LIST(type)                                                             \
    type exp_region0_lut_offset;                                                                   \
    type exp_region0_num_segments;                                                                 \
    type exp_region1_lut_offset;                                                                   \
    type exp_region1_num_segments;                                                                 \
    type field_region_end;                                                                         \
    type field_region_end_slope;                                                                   \
    type field_region_end_base;                                                                    \
    type exp_region_start;                                                                         \
    type exp_region_start_segment;                                                                 \
    type field_region_linear_slope;                                                                \
    type field_region_start_base;                                                                  \
    type field_offset

#define TF_HELPER_REG_LIST                                                                         \
    uint32_t start_cntl_b;                                                                         \
    uint32_t start_cntl_g;                                                                         \
    uint32_t start_cntl_r;                                                                         \
    uint32_t start_slope_cntl_b;                                                                   \
    uint32_t start_slope_cntl_g;                                                                   \
    uint32_t start_slope_cntl_r;                                                                   \
    uint32_t start_end_cntl1_b;                                                                    \
    uint32_t start_end_cntl2_b;                                                                    \
    uint32_t start_end_cntl1_g;                                                                    \
    uint32_t start_end_cntl2_g;                                                                    \
    uint32_t start_end_cntl1_r;                                                                    \
    uint32_t start_end_cntl2_r;                                                                    \
    uint32_t region_start;                                                                         \
    uint32_t region_end

struct vpe10_xfer_func_shift {
    TF_HELPER_REG_FIELD_LIST(uint8_t);
};

struct vpe10_xfer_func_mask {
    TF_HELPER_REG_FIELD_LIST(uint32_t);
};

struct vpe10_xfer_func_reg {
    struct vpe10_xfer_func_shift shifts;
    struct vpe10_xfer_func_mask  masks;

    TF_HELPER_REG_LIST;
    uint32_t offset_b;
    uint32_t offset_g;
    uint32_t offset_r;
    uint32_t start_base_cntl_b;
    uint32_t start_base_cntl_g;
    uint32_t start_base_cntl_r;
};

#define TF_CM_REG_FIELD_LIST(type)                                                                 \
    type csc_c11;                                                                                  \
    type csc_c12

struct cm_color_matrix_shift {
    TF_CM_REG_FIELD_LIST(uint8_t);
};

struct cm_color_matrix_mask {
    TF_CM_REG_FIELD_LIST(uint32_t);
};

struct color_matrices_reg {
    struct cm_color_matrix_shift shifts;
    struct cm_color_matrix_mask  masks;

    uint32_t csc_c11_c12;
    uint32_t csc_c33_c34;
};

enum cm_rgb_channel {
    CM_PWL_R,
    CM_PWL_G,
    CM_PWL_B
};

void vpe10_cm_helper_program_pwl(struct config_writer *config_writer,
    const struct pwl_result_data *rgb, uint32_t last_base_value, uint32_t num,
    uint32_t lut_data_reg_offset, uint8_t lut_data_reg_shift, uint32_t lut_data_reg_mask,
    enum cm_rgb_channel channel);

void vpe10_cm_helper_program_color_matrices(struct config_writer *config_writer,
    const uint16_t *regval, const struct color_matrices_reg *reg);

void vpe10_cm_helper_program_gamcor_xfer_func(struct config_writer *config_writer,
    const struct pwl_params *params, const struct vpe10_xfer_func_reg *reg);

bool vpe10_cm_helper_translate_curve_to_hw_format(
    const struct transfer_func *output_tf, struct pwl_params *lut_params, bool fixpoint);

bool vpe10_cm_helper_translate_curve_to_degamma_hw_format(
    const struct transfer_func *output_tf, struct pwl_params *lut_params);

void vpe10_cm_get_tf_pwl_params(const struct transfer_func *output_tf,
    struct pwl_params **lut_params, enum cm_type vpe_cm_type);

#ifdef __cplusplus
}
#endif
