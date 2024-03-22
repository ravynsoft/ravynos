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

#include "dpp.h"
#include "transform.h"
#include "reg_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

// Used to resolve corner case
#define DPP_SFRB(field_name, reg_name, post_fix) .field_name = reg_name##_##field_name##post_fix

#define DPP_REG_LIST_VPE10(id)                                                                     \
    SRIDFVL(VPCNVC_SURFACE_PIXEL_FORMAT, VPCNVC_CFG, id),                                          \
        SRIDFVL(VPCNVC_FORMAT_CONTROL, VPCNVC_CFG, id),                                            \
        SRIDFVL(VPCNVC_FCNV_FP_BIAS_R, VPCNVC_CFG, id),                                            \
        SRIDFVL(VPCNVC_FCNV_FP_BIAS_G, VPCNVC_CFG, id),                                            \
        SRIDFVL(VPCNVC_FCNV_FP_BIAS_B, VPCNVC_CFG, id),                                            \
        SRIDFVL(VPCNVC_FCNV_FP_SCALE_R, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_FCNV_FP_SCALE_G, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_FCNV_FP_SCALE_B, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_COLOR_KEYER_CONTROL, VPCNVC_CFG, id),                                       \
        SRIDFVL(VPCNVC_COLOR_KEYER_ALPHA, VPCNVC_CFG, id),                                         \
        SRIDFVL(VPCNVC_COLOR_KEYER_RED, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_COLOR_KEYER_GREEN, VPCNVC_CFG, id),                                         \
        SRIDFVL(VPCNVC_COLOR_KEYER_BLUE, VPCNVC_CFG, id),                                          \
        SRIDFVL(VPCNVC_ALPHA_2BIT_LUT, VPCNVC_CFG, id),                                            \
        SRIDFVL(VPCNVC_PRE_DEALPHA, VPCNVC_CFG, id), SRIDFVL(VPCNVC_PRE_CSC_MODE, VPCNVC_CFG, id), \
        SRIDFVL(VPCNVC_PRE_CSC_C11_C12, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_PRE_CSC_C13_C14, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_PRE_CSC_C21_C22, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_PRE_CSC_C23_C24, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_PRE_CSC_C31_C32, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_PRE_CSC_C33_C34, VPCNVC_CFG, id),                                           \
        SRIDFVL(VPCNVC_COEF_FORMAT, VPCNVC_CFG, id), SRIDFVL(VPCNVC_PRE_DEGAM, VPCNVC_CFG, id),    \
        SRIDFVL(VPCNVC_PRE_REALPHA, VPCNVC_CFG, id),                                               \
        SRIDFVL(VPDSCL_COEF_RAM_TAP_SELECT, VPDSCL, id),                                           \
        SRIDFVL(VPDSCL_COEF_RAM_TAP_DATA, VPDSCL, id), SRIDFVL(VPDSCL_MODE, VPDSCL, id),           \
        SRIDFVL(VPDSCL_TAP_CONTROL, VPDSCL, id), SRIDFVL(VPDSCL_CONTROL, VPDSCL, id),              \
        SRIDFVL(VPDSCL_2TAP_CONTROL, VPDSCL, id),                                                  \
        SRIDFVL(VPDSCL_MANUAL_REPLICATE_CONTROL, VPDSCL, id),                                      \
        SRIDFVL(VPDSCL_HORZ_FILTER_SCALE_RATIO, VPDSCL, id),                                       \
        SRIDFVL(VPDSCL_HORZ_FILTER_INIT, VPDSCL, id),                                              \
        SRIDFVL(VPDSCL_HORZ_FILTER_SCALE_RATIO_C, VPDSCL, id),                                     \
        SRIDFVL(VPDSCL_HORZ_FILTER_INIT_C, VPDSCL, id),                                            \
        SRIDFVL(VPDSCL_VERT_FILTER_SCALE_RATIO, VPDSCL, id),                                       \
        SRIDFVL(VPDSCL_VERT_FILTER_INIT, VPDSCL, id),                                              \
        SRIDFVL(VPDSCL_VERT_FILTER_SCALE_RATIO_C, VPDSCL, id),                                     \
        SRIDFVL(VPDSCL_VERT_FILTER_INIT_C, VPDSCL, id), SRIDFVL(VPDSCL_BLACK_COLOR, VPDSCL, id),   \
        SRIDFVL(VPDSCL_UPDATE, VPDSCL, id), SRIDFVL(VPDSCL_AUTOCAL, VPDSCL, id),                   \
        SRIDFVL(VPDSCL_EXT_OVERSCAN_LEFT_RIGHT, VPDSCL, id),                                       \
        SRIDFVL(VPDSCL_EXT_OVERSCAN_TOP_BOTTOM, VPDSCL, id), SRIDFVL(VPOTG_H_BLANK, VPDSCL, id),   \
        SRIDFVL(VPOTG_V_BLANK, VPDSCL, id), SRIDFVL(VPDSCL_RECOUT_START, VPDSCL, id),              \
        SRIDFVL(VPDSCL_RECOUT_SIZE, VPDSCL, id), SRIDFVL(VPMPC_SIZE, VPDSCL, id),                  \
        SRIDFVL(VPLB_DATA_FORMAT, VPDSCL, id), SRIDFVL(VPLB_MEMORY_CTRL, VPDSCL, id),              \
        SRIDFVL(VPLB_V_COUNTER, VPDSCL, id), SRIDFVL(VPDSCL_MEM_PWR_CTRL, VPDSCL, id),             \
        SRIDFVL(VPDSCL_MEM_PWR_STATUS, VPDSCL, id), SRIDFVL(VPCM_CONTROL, VPCM, id),               \
        SRIDFVL(VPCM_POST_CSC_CONTROL, VPCM, id), SRIDFVL(VPCM_POST_CSC_C11_C12, VPCM, id),        \
        SRIDFVL(VPCM_POST_CSC_C13_C14, VPCM, id), SRIDFVL(VPCM_POST_CSC_C21_C22, VPCM, id),        \
        SRIDFVL(VPCM_POST_CSC_C23_C24, VPCM, id), SRIDFVL(VPCM_POST_CSC_C31_C32, VPCM, id),        \
        SRIDFVL(VPCM_POST_CSC_C33_C34, VPCM, id), SRIDFVL(VPCM_GAMUT_REMAP_CONTROL, VPCM, id),     \
        SRIDFVL(VPCM_GAMUT_REMAP_C11_C12, VPCM, id), SRIDFVL(VPCM_GAMUT_REMAP_C13_C14, VPCM, id),  \
        SRIDFVL(VPCM_GAMUT_REMAP_C21_C22, VPCM, id), SRIDFVL(VPCM_GAMUT_REMAP_C23_C24, VPCM, id),  \
        SRIDFVL(VPCM_GAMUT_REMAP_C31_C32, VPCM, id), SRIDFVL(VPCM_GAMUT_REMAP_C33_C34, VPCM, id),  \
        SRIDFVL(VPCM_BIAS_CR_R, VPCM, id), SRIDFVL(VPCM_BIAS_Y_G_CB_B, VPCM, id),                  \
        SRIDFVL(VPCM_GAMCOR_CONTROL, VPCM, id), SRIDFVL(VPCM_GAMCOR_LUT_INDEX, VPCM, id),          \
        SRIDFVL(VPCM_GAMCOR_LUT_DATA, VPCM, id), SRIDFVL(VPCM_GAMCOR_LUT_CONTROL, VPCM, id),       \
        SRIDFVL(VPCM_GAMCOR_RAMA_START_CNTL_B, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_START_CNTL_G, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_START_CNTL_R, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_B, VPCM, id),                                    \
        SRIDFVL(VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_G, VPCM, id),                                    \
        SRIDFVL(VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_R, VPCM, id),                                    \
        SRIDFVL(VPCM_GAMCOR_RAMA_START_BASE_CNTL_B, VPCM, id),                                     \
        SRIDFVL(VPCM_GAMCOR_RAMA_START_BASE_CNTL_G, VPCM, id),                                     \
        SRIDFVL(VPCM_GAMCOR_RAMA_START_BASE_CNTL_R, VPCM, id),                                     \
        SRIDFVL(VPCM_GAMCOR_RAMA_END_CNTL1_B, VPCM, id),                                           \
        SRIDFVL(VPCM_GAMCOR_RAMA_END_CNTL2_B, VPCM, id),                                           \
        SRIDFVL(VPCM_GAMCOR_RAMA_END_CNTL1_G, VPCM, id),                                           \
        SRIDFVL(VPCM_GAMCOR_RAMA_END_CNTL2_G, VPCM, id),                                           \
        SRIDFVL(VPCM_GAMCOR_RAMA_END_CNTL1_R, VPCM, id),                                           \
        SRIDFVL(VPCM_GAMCOR_RAMA_END_CNTL2_R, VPCM, id),                                           \
        SRIDFVL(VPCM_GAMCOR_RAMA_OFFSET_B, VPCM, id),                                              \
        SRIDFVL(VPCM_GAMCOR_RAMA_OFFSET_G, VPCM, id),                                              \
        SRIDFVL(VPCM_GAMCOR_RAMA_OFFSET_R, VPCM, id),                                              \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_0_1, VPCM, id),                                            \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_2_3, VPCM, id),                                            \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_4_5, VPCM, id),                                            \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_6_7, VPCM, id),                                            \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_8_9, VPCM, id),                                            \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_10_11, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_12_13, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_14_15, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_16_17, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_18_19, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_20_21, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_22_23, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_24_25, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_26_27, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_28_29, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_30_31, VPCM, id),                                          \
        SRIDFVL(VPCM_GAMCOR_RAMA_REGION_32_33, VPCM, id), SRIDFVL(VPCM_HDR_MULT_COEF, VPCM, id),   \
        SRIDFVL(VPCM_MEM_PWR_CTRL, VPCM, id), SRIDFVL(VPCM_MEM_PWR_STATUS, VPCM, id),              \
        SRIDFVL(VPCM_DEALPHA, VPCM, id), SRIDFVL(VPCM_COEF_FORMAT, VPCM, id),                      \
        SRIDFVL(VPDPP_CONTROL, VPDPP_TOP, id), SRIDFVL(VPDPP_CRC_CTRL, VPDPP_TOP, id),

#define DPP_FIELD_LIST_VPE10(post_fix)                                                             \
    SFRB(VPCNVC_SURFACE_PIXEL_FORMAT, VPCNVC_SURFACE_PIXEL_FORMAT, post_fix),                      \
        SFRB(FORMAT_EXPANSION_MODE, VPCNVC_FORMAT_CONTROL, post_fix),                              \
        SFRB(FORMAT_CNV16, VPCNVC_FORMAT_CONTROL, post_fix),                                       \
        DPP_SFRB(FORMAT_CONTROL__ALPHA_EN, VPCNVC, post_fix),                                      \
        SFRB(VPCNVC_BYPASS, VPCNVC_FORMAT_CONTROL, post_fix),                                      \
        SFRB(VPCNVC_BYPASS_MSB_ALIGN, VPCNVC_FORMAT_CONTROL, post_fix),                            \
        SFRB(CLAMP_POSITIVE, VPCNVC_FORMAT_CONTROL, post_fix),                                     \
        SFRB(CLAMP_POSITIVE_C, VPCNVC_FORMAT_CONTROL, post_fix),                                   \
        SFRB(VPCNVC_UPDATE_PENDING, VPCNVC_FORMAT_CONTROL, post_fix),                              \
        SFRB(FCNV_FP_BIAS_R, VPCNVC_FCNV_FP_BIAS_R, post_fix),                                     \
        SFRB(FCNV_FP_BIAS_G, VPCNVC_FCNV_FP_BIAS_G, post_fix),                                     \
        SFRB(FCNV_FP_BIAS_B, VPCNVC_FCNV_FP_BIAS_B, post_fix),                                     \
        SFRB(FCNV_FP_SCALE_R, VPCNVC_FCNV_FP_SCALE_R, post_fix),                                   \
        SFRB(FCNV_FP_SCALE_G, VPCNVC_FCNV_FP_SCALE_G, post_fix),                                   \
        SFRB(FCNV_FP_SCALE_B, VPCNVC_FCNV_FP_SCALE_B, post_fix),                                   \
        SFRB(COLOR_KEYER_EN, VPCNVC_COLOR_KEYER_CONTROL, post_fix),                                \
        SFRB(COLOR_KEYER_MODE, VPCNVC_COLOR_KEYER_CONTROL, post_fix),                              \
        SFRB(COLOR_KEYER_ALPHA_LOW, VPCNVC_COLOR_KEYER_ALPHA, post_fix),                           \
        SFRB(COLOR_KEYER_ALPHA_HIGH, VPCNVC_COLOR_KEYER_ALPHA, post_fix),                          \
        SFRB(COLOR_KEYER_RED_LOW, VPCNVC_COLOR_KEYER_RED, post_fix),                               \
        SFRB(COLOR_KEYER_RED_HIGH, VPCNVC_COLOR_KEYER_RED, post_fix),                              \
        SFRB(COLOR_KEYER_GREEN_LOW, VPCNVC_COLOR_KEYER_GREEN, post_fix),                           \
        SFRB(COLOR_KEYER_GREEN_HIGH, VPCNVC_COLOR_KEYER_GREEN, post_fix),                          \
        SFRB(COLOR_KEYER_BLUE_LOW, VPCNVC_COLOR_KEYER_BLUE, post_fix),                             \
        SFRB(COLOR_KEYER_BLUE_HIGH, VPCNVC_COLOR_KEYER_BLUE, post_fix),                            \
        SFRB(ALPHA_2BIT_LUT0, VPCNVC_ALPHA_2BIT_LUT, post_fix),                                    \
        SFRB(ALPHA_2BIT_LUT1, VPCNVC_ALPHA_2BIT_LUT, post_fix),                                    \
        SFRB(ALPHA_2BIT_LUT2, VPCNVC_ALPHA_2BIT_LUT, post_fix),                                    \
        SFRB(ALPHA_2BIT_LUT3, VPCNVC_ALPHA_2BIT_LUT, post_fix),                                    \
        SFRB(PRE_DEALPHA_EN, VPCNVC_PRE_DEALPHA, post_fix),                                        \
        SFRB(PRE_DEALPHA_ABLND_EN, VPCNVC_PRE_DEALPHA, post_fix),                                  \
        SFRB(PRE_CSC_MODE, VPCNVC_PRE_CSC_MODE, post_fix),                                         \
        SFRB(PRE_CSC_MODE_CURRENT, VPCNVC_PRE_CSC_MODE, post_fix),                                 \
        SFRB(PRE_CSC_C11, VPCNVC_PRE_CSC_C11_C12, post_fix),                                       \
        SFRB(PRE_CSC_C12, VPCNVC_PRE_CSC_C11_C12, post_fix),                                       \
        SFRB(PRE_CSC_C13, VPCNVC_PRE_CSC_C13_C14, post_fix),                                       \
        SFRB(PRE_CSC_C14, VPCNVC_PRE_CSC_C13_C14, post_fix),                                       \
        SFRB(PRE_CSC_C21, VPCNVC_PRE_CSC_C21_C22, post_fix),                                       \
        SFRB(PRE_CSC_C22, VPCNVC_PRE_CSC_C21_C22, post_fix),                                       \
        SFRB(PRE_CSC_C23, VPCNVC_PRE_CSC_C23_C24, post_fix),                                       \
        SFRB(PRE_CSC_C24, VPCNVC_PRE_CSC_C23_C24, post_fix),                                       \
        SFRB(PRE_CSC_C31, VPCNVC_PRE_CSC_C31_C32, post_fix),                                       \
        SFRB(PRE_CSC_C32, VPCNVC_PRE_CSC_C31_C32, post_fix),                                       \
        SFRB(PRE_CSC_C33, VPCNVC_PRE_CSC_C33_C34, post_fix),                                       \
        SFRB(PRE_CSC_C34, VPCNVC_PRE_CSC_C33_C34, post_fix),                                       \
        SFRB(PRE_CSC_COEF_FORMAT, VPCNVC_COEF_FORMAT, post_fix),                                   \
        SFRB(PRE_DEGAM_MODE, VPCNVC_PRE_DEGAM, post_fix),                                          \
        SFRB(PRE_DEGAM_SELECT, VPCNVC_PRE_DEGAM, post_fix),                                        \
        SFRB(PRE_REALPHA_EN, VPCNVC_PRE_REALPHA, post_fix),                                        \
        SFRB(PRE_REALPHA_ABLND_EN, VPCNVC_PRE_REALPHA, post_fix),                                  \
        SFRB(SCL_COEF_RAM_TAP_PAIR_IDX, VPDSCL_COEF_RAM_TAP_SELECT, post_fix),                     \
        SFRB(SCL_COEF_RAM_PHASE, VPDSCL_COEF_RAM_TAP_SELECT, post_fix),                            \
        SFRB(SCL_COEF_RAM_FILTER_TYPE, VPDSCL_COEF_RAM_TAP_SELECT, post_fix),                      \
        SFRB(SCL_COEF_RAM_EVEN_TAP_COEF, VPDSCL_COEF_RAM_TAP_DATA, post_fix),                      \
        SFRB(SCL_COEF_RAM_EVEN_TAP_COEF_EN, VPDSCL_COEF_RAM_TAP_DATA, post_fix),                   \
        SFRB(SCL_COEF_RAM_ODD_TAP_COEF, VPDSCL_COEF_RAM_TAP_DATA, post_fix),                       \
        SFRB(SCL_COEF_RAM_ODD_TAP_COEF_EN, VPDSCL_COEF_RAM_TAP_DATA, post_fix),                    \
        SFRB(VPDSCL_MODE, VPDSCL_MODE, post_fix),                                                  \
        SFRB(SCL_COEF_RAM_SELECT_CURRENT, VPDSCL_MODE, post_fix),                                  \
        SFRB(SCL_CHROMA_COEF_MODE, VPDSCL_MODE, post_fix),                                         \
        SFRB(SCL_ALPHA_COEF_MODE, VPDSCL_MODE, post_fix),                                          \
        SFRB(SCL_COEF_RAM_SELECT_RD, VPDSCL_MODE, post_fix),                                       \
        SFRB(SCL_V_NUM_TAPS, VPDSCL_TAP_CONTROL, post_fix),                                        \
        SFRB(SCL_H_NUM_TAPS, VPDSCL_TAP_CONTROL, post_fix),                                        \
        SFRB(SCL_V_NUM_TAPS_C, VPDSCL_TAP_CONTROL, post_fix),                                      \
        SFRB(SCL_H_NUM_TAPS_C, VPDSCL_TAP_CONTROL, post_fix),                                      \
        SFRB(SCL_BOUNDARY_MODE, VPDSCL_CONTROL, post_fix),                                         \
        SFRB(SCL_H_2TAP_HARDCODE_COEF_EN, VPDSCL_2TAP_CONTROL, post_fix),                          \
        SFRB(SCL_H_2TAP_SHARP_EN, VPDSCL_2TAP_CONTROL, post_fix),                                  \
        SFRB(SCL_H_2TAP_SHARP_FACTOR, VPDSCL_2TAP_CONTROL, post_fix),                              \
        SFRB(SCL_V_2TAP_HARDCODE_COEF_EN, VPDSCL_2TAP_CONTROL, post_fix),                          \
        SFRB(SCL_V_2TAP_SHARP_EN, VPDSCL_2TAP_CONTROL, post_fix),                                  \
        SFRB(SCL_V_2TAP_SHARP_FACTOR, VPDSCL_2TAP_CONTROL, post_fix),                              \
        SFRB(SCL_V_MANUAL_REPLICATE_FACTOR, VPDSCL_MANUAL_REPLICATE_CONTROL, post_fix),            \
        SFRB(SCL_H_MANUAL_REPLICATE_FACTOR, VPDSCL_MANUAL_REPLICATE_CONTROL, post_fix),            \
        SFRB(SCL_H_SCALE_RATIO, VPDSCL_HORZ_FILTER_SCALE_RATIO, post_fix),                         \
        SFRB(SCL_H_INIT_FRAC, VPDSCL_HORZ_FILTER_INIT, post_fix),                                  \
        SFRB(SCL_H_INIT_INT, VPDSCL_HORZ_FILTER_INIT, post_fix),                                   \
        SFRB(SCL_H_SCALE_RATIO_C, VPDSCL_HORZ_FILTER_SCALE_RATIO_C, post_fix),                     \
        SFRB(SCL_H_INIT_FRAC_C, VPDSCL_HORZ_FILTER_INIT_C, post_fix),                              \
        SFRB(SCL_H_INIT_INT_C, VPDSCL_HORZ_FILTER_INIT_C, post_fix),                               \
        SFRB(SCL_V_SCALE_RATIO, VPDSCL_VERT_FILTER_SCALE_RATIO, post_fix),                         \
        SFRB(SCL_V_INIT_FRAC, VPDSCL_VERT_FILTER_INIT, post_fix),                                  \
        SFRB(SCL_V_INIT_INT, VPDSCL_VERT_FILTER_INIT, post_fix),                                   \
        SFRB(SCL_V_SCALE_RATIO_C, VPDSCL_VERT_FILTER_SCALE_RATIO_C, post_fix),                     \
        SFRB(SCL_V_INIT_FRAC_C, VPDSCL_VERT_FILTER_INIT_C, post_fix),                              \
        SFRB(SCL_V_INIT_INT_C, VPDSCL_VERT_FILTER_INIT_C, post_fix),                               \
        SFRB(SCL_BLACK_COLOR_RGB_Y, VPDSCL_BLACK_COLOR, post_fix),                                 \
        SFRB(SCL_BLACK_COLOR_CBCR, VPDSCL_BLACK_COLOR, post_fix),                                  \
        SFRB(SCL_UPDATE_PENDING, VPDSCL_UPDATE, post_fix),                                         \
        SFRB(AUTOCAL_MODE, VPDSCL_AUTOCAL, post_fix),                                              \
        SFRB(EXT_OVERSCAN_RIGHT, VPDSCL_EXT_OVERSCAN_LEFT_RIGHT, post_fix),                        \
        SFRB(EXT_OVERSCAN_LEFT, VPDSCL_EXT_OVERSCAN_LEFT_RIGHT, post_fix),                         \
        SFRB(EXT_OVERSCAN_BOTTOM, VPDSCL_EXT_OVERSCAN_TOP_BOTTOM, post_fix),                       \
        SFRB(EXT_OVERSCAN_TOP, VPDSCL_EXT_OVERSCAN_TOP_BOTTOM, post_fix),                          \
        SFRB(OTG_H_BLANK_START, VPOTG_H_BLANK, post_fix),                                          \
        SFRB(OTG_H_BLANK_END, VPOTG_H_BLANK, post_fix),                                            \
        SFRB(OTG_V_BLANK_START, VPOTG_V_BLANK, post_fix),                                          \
        SFRB(OTG_V_BLANK_END, VPOTG_V_BLANK, post_fix),                                            \
        SFRB(RECOUT_START_X, VPDSCL_RECOUT_START, post_fix),                                       \
        SFRB(RECOUT_START_Y, VPDSCL_RECOUT_START, post_fix),                                       \
        SFRB(RECOUT_WIDTH, VPDSCL_RECOUT_SIZE, post_fix),                                          \
        SFRB(RECOUT_HEIGHT, VPDSCL_RECOUT_SIZE, post_fix),                                         \
        SFRB(VPMPC_WIDTH, VPMPC_SIZE, post_fix), SFRB(VPMPC_HEIGHT, VPMPC_SIZE, post_fix),         \
        SFRB(ALPHA_EN, VPLB_DATA_FORMAT, post_fix),                                                \
        SFRB(MEMORY_CONFIG, VPLB_MEMORY_CTRL, post_fix),                                           \
        SFRB(LB_MAX_PARTITIONS, VPLB_MEMORY_CTRL, post_fix),                                       \
        SFRB(LB_NUM_PARTITIONS, VPLB_MEMORY_CTRL, post_fix),                                       \
        SFRB(LB_NUM_PARTITIONS_C, VPLB_MEMORY_CTRL, post_fix),                                     \
        SFRB(V_COUNTER, VPLB_V_COUNTER, post_fix), SFRB(V_COUNTER_C, VPLB_V_COUNTER, post_fix),    \
        SFRB(LUT_MEM_PWR_FORCE, VPDSCL_MEM_PWR_CTRL, post_fix),                                    \
        SFRB(LUT_MEM_PWR_DIS, VPDSCL_MEM_PWR_CTRL, post_fix),                                      \
        SFRB(LB_G1_MEM_PWR_FORCE, VPDSCL_MEM_PWR_CTRL, post_fix),                                  \
        SFRB(LB_G1_MEM_PWR_DIS, VPDSCL_MEM_PWR_CTRL, post_fix),                                    \
        SFRB(LB_G2_MEM_PWR_FORCE, VPDSCL_MEM_PWR_CTRL, post_fix),                                  \
        SFRB(LB_G2_MEM_PWR_DIS, VPDSCL_MEM_PWR_CTRL, post_fix),                                    \
        SFRB(LB_MEM_PWR_MODE, VPDSCL_MEM_PWR_CTRL, post_fix),                                      \
        SFRB(LUT_MEM_PWR_STATE, VPDSCL_MEM_PWR_STATUS, post_fix),                                  \
        SFRB(LB_G1_MEM_PWR_STATE, VPDSCL_MEM_PWR_STATUS, post_fix),                                \
        SFRB(LB_G2_MEM_PWR_STATE, VPDSCL_MEM_PWR_STATUS, post_fix),                                \
        SFRB(VPCM_BYPASS, VPCM_CONTROL, post_fix),                                                 \
        SFRB(VPCM_UPDATE_PENDING, VPCM_CONTROL, post_fix),                                         \
        SFRB(VPCM_POST_CSC_MODE, VPCM_POST_CSC_CONTROL, post_fix),                                 \
        SFRB(VPCM_POST_CSC_MODE_CURRENT, VPCM_POST_CSC_CONTROL, post_fix),                         \
        SFRB(VPCM_POST_CSC_C11, VPCM_POST_CSC_C11_C12, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C12, VPCM_POST_CSC_C11_C12, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C13, VPCM_POST_CSC_C13_C14, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C14, VPCM_POST_CSC_C13_C14, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C21, VPCM_POST_CSC_C21_C22, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C22, VPCM_POST_CSC_C21_C22, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C23, VPCM_POST_CSC_C23_C24, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C24, VPCM_POST_CSC_C23_C24, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C31, VPCM_POST_CSC_C31_C32, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C32, VPCM_POST_CSC_C31_C32, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C33, VPCM_POST_CSC_C33_C34, post_fix),                                  \
        SFRB(VPCM_POST_CSC_C34, VPCM_POST_CSC_C33_C34, post_fix),                                  \
        SFRB(VPCM_GAMUT_REMAP_MODE, VPCM_GAMUT_REMAP_CONTROL, post_fix),                           \
        SFRB(VPCM_GAMUT_REMAP_MODE_CURRENT, VPCM_GAMUT_REMAP_CONTROL, post_fix),                   \
        SFRB(VPCM_GAMUT_REMAP_C11, VPCM_GAMUT_REMAP_C11_C12, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C12, VPCM_GAMUT_REMAP_C11_C12, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C13, VPCM_GAMUT_REMAP_C13_C14, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C14, VPCM_GAMUT_REMAP_C13_C14, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C21, VPCM_GAMUT_REMAP_C21_C22, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C22, VPCM_GAMUT_REMAP_C21_C22, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C23, VPCM_GAMUT_REMAP_C23_C24, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C24, VPCM_GAMUT_REMAP_C23_C24, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C31, VPCM_GAMUT_REMAP_C31_C32, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C32, VPCM_GAMUT_REMAP_C31_C32, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C33, VPCM_GAMUT_REMAP_C33_C34, post_fix),                            \
        SFRB(VPCM_GAMUT_REMAP_C34, VPCM_GAMUT_REMAP_C33_C34, post_fix),                            \
        SFRB(VPCM_BIAS_CR_R, VPCM_BIAS_CR_R, post_fix),                                            \
        SFRB(VPCM_BIAS_Y_G, VPCM_BIAS_Y_G_CB_B, post_fix),                                         \
        SFRB(VPCM_BIAS_CB_B, VPCM_BIAS_Y_G_CB_B, post_fix),                                        \
        SFRB(VPCM_GAMCOR_MODE, VPCM_GAMCOR_CONTROL, post_fix),                                     \
        SFRB(VPCM_GAMCOR_PWL_DISABLE, VPCM_GAMCOR_CONTROL, post_fix),                              \
        SFRB(VPCM_GAMCOR_MODE_CURRENT, VPCM_GAMCOR_CONTROL, post_fix),                             \
        SFRB(VPCM_GAMCOR_SELECT_CURRENT, VPCM_GAMCOR_CONTROL, post_fix),                           \
        SFRB(VPCM_GAMCOR_LUT_INDEX, VPCM_GAMCOR_LUT_INDEX, post_fix),                              \
        SFRB(VPCM_GAMCOR_LUT_DATA, VPCM_GAMCOR_LUT_DATA, post_fix),                                \
        SFRB(VPCM_GAMCOR_LUT_WRITE_COLOR_MASK, VPCM_GAMCOR_LUT_CONTROL, post_fix),                 \
        SFRB(VPCM_GAMCOR_LUT_READ_COLOR_SEL, VPCM_GAMCOR_LUT_CONTROL, post_fix),                   \
        SFRB(VPCM_GAMCOR_LUT_READ_DBG, VPCM_GAMCOR_LUT_CONTROL, post_fix),                         \
        SFRB(VPCM_GAMCOR_LUT_HOST_SEL, VPCM_GAMCOR_LUT_CONTROL, post_fix),                         \
        SFRB(VPCM_GAMCOR_LUT_CONFIG_MODE, VPCM_GAMCOR_LUT_CONTROL, post_fix),                      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_START_B, VPCM_GAMCOR_RAMA_START_CNTL_B, post_fix),        \
        SFRB(                                                                                      \
            VPCM_GAMCOR_RAMA_EXP_REGION_START_SEGMENT_B, VPCM_GAMCOR_RAMA_START_CNTL_B, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_START_G, VPCM_GAMCOR_RAMA_START_CNTL_G, post_fix),        \
        SFRB(                                                                                      \
            VPCM_GAMCOR_RAMA_EXP_REGION_START_SEGMENT_G, VPCM_GAMCOR_RAMA_START_CNTL_G, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_START_R, VPCM_GAMCOR_RAMA_START_CNTL_R, post_fix),        \
        SFRB(                                                                                      \
            VPCM_GAMCOR_RAMA_EXP_REGION_START_SEGMENT_R, VPCM_GAMCOR_RAMA_START_CNTL_R, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_START_SLOPE_B, VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_B,       \
            post_fix),                                                                             \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_START_SLOPE_G, VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_G,       \
            post_fix),                                                                             \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_START_SLOPE_R, VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_R,       \
            post_fix),                                                                             \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_START_BASE_B, VPCM_GAMCOR_RAMA_START_BASE_CNTL_B,         \
            post_fix),                                                                             \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_START_BASE_G, VPCM_GAMCOR_RAMA_START_BASE_CNTL_G,         \
            post_fix),                                                                             \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_START_BASE_R, VPCM_GAMCOR_RAMA_START_BASE_CNTL_R,         \
            post_fix),                                                                             \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_END_BASE_B, VPCM_GAMCOR_RAMA_END_CNTL1_B, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_END_B, VPCM_GAMCOR_RAMA_END_CNTL2_B, post_fix),           \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_END_SLOPE_B, VPCM_GAMCOR_RAMA_END_CNTL2_B, post_fix),     \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_END_BASE_G, VPCM_GAMCOR_RAMA_END_CNTL1_G, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_END_G, VPCM_GAMCOR_RAMA_END_CNTL2_G, post_fix),           \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_END_SLOPE_G, VPCM_GAMCOR_RAMA_END_CNTL2_G, post_fix),     \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_END_BASE_R, VPCM_GAMCOR_RAMA_END_CNTL1_R, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_END_R, VPCM_GAMCOR_RAMA_END_CNTL2_R, post_fix),           \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION_END_SLOPE_R, VPCM_GAMCOR_RAMA_END_CNTL2_R, post_fix),     \
        SFRB(VPCM_GAMCOR_RAMA_OFFSET_B, VPCM_GAMCOR_RAMA_OFFSET_B, post_fix),                      \
        SFRB(VPCM_GAMCOR_RAMA_OFFSET_G, VPCM_GAMCOR_RAMA_OFFSET_G, post_fix),                      \
        SFRB(VPCM_GAMCOR_RAMA_OFFSET_R, VPCM_GAMCOR_RAMA_OFFSET_R, post_fix),                      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION0_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_0_1, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION0_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_0_1, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION1_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_0_1, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION1_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_0_1, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION2_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_2_3, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION2_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_2_3, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION3_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_2_3, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION3_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_2_3, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION4_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_4_5, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION4_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_4_5, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION5_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_4_5, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION5_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_4_5, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION6_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_6_7, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION6_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_6_7, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION7_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_6_7, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION7_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_6_7, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION8_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_8_9, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION8_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_8_9, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION9_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_8_9, post_fix),      \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION9_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_8_9, post_fix),    \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION10_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_10_11, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION10_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_10_11, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION11_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_10_11, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION11_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_10_11, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION12_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_12_13, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION12_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_12_13, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION13_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_12_13, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION13_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_12_13, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION14_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_14_15, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION14_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_14_15, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION15_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_14_15, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION15_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_14_15, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION16_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_16_17, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION16_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_16_17, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION17_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_16_17, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION17_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_16_17, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION18_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_18_19, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION18_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_18_19, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION19_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_18_19, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION19_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_18_19, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION20_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_20_21, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION20_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_20_21, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION21_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_20_21, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION21_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_20_21, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION22_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_22_23, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION22_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_22_23, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION23_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_22_23, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION23_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_22_23, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION24_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_24_25, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION24_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_24_25, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION25_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_24_25, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION25_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_24_25, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION26_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_26_27, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION26_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_26_27, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION27_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_26_27, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION27_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_26_27, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION28_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_28_29, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION28_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_28_29, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION29_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_28_29, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION29_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_28_29, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION30_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_30_31, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION30_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_30_31, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION31_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_30_31, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION31_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_30_31, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION32_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_32_33, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION32_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_32_33, post_fix), \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION33_LUT_OFFSET, VPCM_GAMCOR_RAMA_REGION_32_33, post_fix),   \
        SFRB(VPCM_GAMCOR_RAMA_EXP_REGION33_NUM_SEGMENTS, VPCM_GAMCOR_RAMA_REGION_32_33, post_fix), \
        SFRB(VPCM_HDR_MULT_COEF, VPCM_HDR_MULT_COEF, post_fix),                                    \
        SFRB(GAMCOR_MEM_PWR_FORCE, VPCM_MEM_PWR_CTRL, post_fix),                                   \
        SFRB(GAMCOR_MEM_PWR_DIS, VPCM_MEM_PWR_CTRL, post_fix),                                     \
        SFRB(GAMCOR_MEM_PWR_STATE, VPCM_MEM_PWR_STATUS, post_fix),                                 \
        SFRB(VPCM_DEALPHA_EN, VPCM_DEALPHA, post_fix),                                             \
        SFRB(VPCM_DEALPHA_ABLND, VPCM_DEALPHA, post_fix),                                          \
        SFRB(VPCM_BIAS_FORMAT, VPCM_COEF_FORMAT, post_fix),                                        \
        SFRB(VPCM_POST_CSC_COEF_FORMAT, VPCM_COEF_FORMAT, post_fix),                               \
        SFRB(VPCM_GAMUT_REMAP_COEF_FORMAT, VPCM_COEF_FORMAT, post_fix),                            \
        SFRB(VPDPP_CLOCK_ENABLE, VPDPP_CONTROL, post_fix),                                         \
        SFRB(VPECLK_G_GATE_DISABLE, VPDPP_CONTROL, post_fix),                                      \
        SFRB(VPECLK_G_DYN_GATE_DISABLE, VPDPP_CONTROL, post_fix),                                  \
        SFRB(VPECLK_G_VPDSCL_GATE_DISABLE, VPDPP_CONTROL, post_fix),                               \
        SFRB(VPECLK_R_GATE_DISABLE, VPDPP_CONTROL, post_fix),                                      \
        SFRB(DISPCLK_R_GATE_DISABLE, VPDPP_CONTROL, post_fix),                                     \
        SFRB(DISPCLK_G_GATE_DISABLE, VPDPP_CONTROL, post_fix),                                     \
        SFRB(VPDPP_FGCG_REP_DIS, VPDPP_CONTROL, post_fix),                                         \
        SFRB(VPDPP_TEST_CLK_SEL, VPDPP_CONTROL, post_fix),                                         \
        SFRB(VPDPP_CRC_EN, VPDPP_CRC_CTRL, post_fix),                                              \
        SFRB(VPDPP_CRC_CONT_EN, VPDPP_CRC_CTRL, post_fix),                                         \
        SFRB(VPDPP_CRC_420_COMP_SEL, VPDPP_CRC_CTRL, post_fix),                                    \
        SFRB(VPDPP_CRC_SRC_SEL, VPDPP_CRC_CTRL, post_fix),                                         \
        SFRB(VPDPP_CRC_PIX_FORMAT_SEL, VPDPP_CRC_CTRL, post_fix),                                  \
        SFRB(VPDPP_CRC_MASK, VPDPP_CRC_CTRL, post_fix)

#define DPP_REG_VARIABLE_LIST_VPE10                                                                \
    reg_id_val VPCNVC_SURFACE_PIXEL_FORMAT;                                                        \
    reg_id_val VPCNVC_FORMAT_CONTROL;                                                              \
    reg_id_val VPCNVC_FCNV_FP_BIAS_R;                                                              \
    reg_id_val VPCNVC_FCNV_FP_BIAS_G;                                                              \
    reg_id_val VPCNVC_FCNV_FP_BIAS_B;                                                              \
    reg_id_val VPCNVC_FCNV_FP_SCALE_R;                                                             \
    reg_id_val VPCNVC_FCNV_FP_SCALE_G;                                                             \
    reg_id_val VPCNVC_FCNV_FP_SCALE_B;                                                             \
    reg_id_val VPCNVC_COLOR_KEYER_CONTROL;                                                         \
    reg_id_val VPCNVC_COLOR_KEYER_ALPHA;                                                           \
    reg_id_val VPCNVC_COLOR_KEYER_RED;                                                             \
    reg_id_val VPCNVC_COLOR_KEYER_GREEN;                                                           \
    reg_id_val VPCNVC_COLOR_KEYER_BLUE;                                                            \
    reg_id_val VPCNVC_ALPHA_2BIT_LUT;                                                              \
    reg_id_val VPCNVC_PRE_DEALPHA;                                                                 \
    reg_id_val VPCNVC_PRE_CSC_MODE;                                                                \
    reg_id_val VPCNVC_PRE_CSC_C11_C12;                                                             \
    reg_id_val VPCNVC_PRE_CSC_C13_C14;                                                             \
    reg_id_val VPCNVC_PRE_CSC_C21_C22;                                                             \
    reg_id_val VPCNVC_PRE_CSC_C23_C24;                                                             \
    reg_id_val VPCNVC_PRE_CSC_C31_C32;                                                             \
    reg_id_val VPCNVC_PRE_CSC_C33_C34;                                                             \
    reg_id_val VPCNVC_COEF_FORMAT;                                                                 \
    reg_id_val VPCNVC_PRE_DEGAM;                                                                   \
    reg_id_val VPCNVC_PRE_REALPHA;                                                                 \
    reg_id_val VPDSCL_COEF_RAM_TAP_SELECT;                                                         \
    reg_id_val VPDSCL_COEF_RAM_TAP_DATA;                                                           \
    reg_id_val VPDSCL_MODE;                                                                        \
    reg_id_val VPDSCL_TAP_CONTROL;                                                                 \
    reg_id_val VPDSCL_CONTROL;                                                                     \
    reg_id_val VPDSCL_2TAP_CONTROL;                                                                \
    reg_id_val VPDSCL_MANUAL_REPLICATE_CONTROL;                                                    \
    reg_id_val VPDSCL_HORZ_FILTER_SCALE_RATIO;                                                     \
    reg_id_val VPDSCL_HORZ_FILTER_INIT;                                                            \
    reg_id_val VPDSCL_HORZ_FILTER_SCALE_RATIO_C;                                                   \
    reg_id_val VPDSCL_HORZ_FILTER_INIT_C;                                                          \
    reg_id_val VPDSCL_VERT_FILTER_SCALE_RATIO;                                                     \
    reg_id_val VPDSCL_VERT_FILTER_INIT;                                                            \
    reg_id_val VPDSCL_VERT_FILTER_SCALE_RATIO_C;                                                   \
    reg_id_val VPDSCL_VERT_FILTER_INIT_C;                                                          \
    reg_id_val VPDSCL_BLACK_COLOR;                                                                 \
    reg_id_val VPDSCL_UPDATE;                                                                      \
    reg_id_val VPDSCL_AUTOCAL;                                                                     \
    reg_id_val VPDSCL_EXT_OVERSCAN_LEFT_RIGHT;                                                     \
    reg_id_val VPDSCL_EXT_OVERSCAN_TOP_BOTTOM;                                                     \
    reg_id_val VPOTG_H_BLANK;                                                                      \
    reg_id_val VPOTG_V_BLANK;                                                                      \
    reg_id_val VPDSCL_RECOUT_START;                                                                \
    reg_id_val VPDSCL_RECOUT_SIZE;                                                                 \
    reg_id_val VPMPC_SIZE;                                                                         \
    reg_id_val VPLB_DATA_FORMAT;                                                                   \
    reg_id_val VPLB_MEMORY_CTRL;                                                                   \
    reg_id_val VPLB_V_COUNTER;                                                                     \
    reg_id_val VPDSCL_MEM_PWR_CTRL;                                                                \
    reg_id_val VPDSCL_MEM_PWR_STATUS;                                                              \
    reg_id_val VPCM_CONTROL;                                                                       \
    reg_id_val VPCM_POST_CSC_CONTROL;                                                              \
    reg_id_val VPCM_POST_CSC_C11_C12;                                                              \
    reg_id_val VPCM_POST_CSC_C13_C14;                                                              \
    reg_id_val VPCM_POST_CSC_C21_C22;                                                              \
    reg_id_val VPCM_POST_CSC_C23_C24;                                                              \
    reg_id_val VPCM_POST_CSC_C31_C32;                                                              \
    reg_id_val VPCM_POST_CSC_C33_C34;                                                              \
    reg_id_val VPCM_GAMUT_REMAP_CONTROL;                                                           \
    reg_id_val VPCM_GAMUT_REMAP_C11_C12;                                                           \
    reg_id_val VPCM_GAMUT_REMAP_C13_C14;                                                           \
    reg_id_val VPCM_GAMUT_REMAP_C21_C22;                                                           \
    reg_id_val VPCM_GAMUT_REMAP_C23_C24;                                                           \
    reg_id_val VPCM_GAMUT_REMAP_C31_C32;                                                           \
    reg_id_val VPCM_GAMUT_REMAP_C33_C34;                                                           \
    reg_id_val VPCM_BIAS_CR_R;                                                                     \
    reg_id_val VPCM_BIAS_Y_G_CB_B;                                                                 \
    reg_id_val VPCM_GAMCOR_CONTROL;                                                                \
    reg_id_val VPCM_GAMCOR_LUT_INDEX;                                                              \
    reg_id_val VPCM_GAMCOR_LUT_DATA;                                                               \
    reg_id_val VPCM_GAMCOR_LUT_CONTROL;                                                            \
    reg_id_val VPCM_GAMCOR_RAMA_START_CNTL_B;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_START_CNTL_G;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_START_CNTL_R;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_B;                                                \
    reg_id_val VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_G;                                                \
    reg_id_val VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_R;                                                \
    reg_id_val VPCM_GAMCOR_RAMA_START_BASE_CNTL_B;                                                 \
    reg_id_val VPCM_GAMCOR_RAMA_START_BASE_CNTL_G;                                                 \
    reg_id_val VPCM_GAMCOR_RAMA_START_BASE_CNTL_R;                                                 \
    reg_id_val VPCM_GAMCOR_RAMA_END_CNTL1_B;                                                       \
    reg_id_val VPCM_GAMCOR_RAMA_END_CNTL2_B;                                                       \
    reg_id_val VPCM_GAMCOR_RAMA_END_CNTL1_G;                                                       \
    reg_id_val VPCM_GAMCOR_RAMA_END_CNTL2_G;                                                       \
    reg_id_val VPCM_GAMCOR_RAMA_END_CNTL1_R;                                                       \
    reg_id_val VPCM_GAMCOR_RAMA_END_CNTL2_R;                                                       \
    reg_id_val VPCM_GAMCOR_RAMA_OFFSET_B;                                                          \
    reg_id_val VPCM_GAMCOR_RAMA_OFFSET_G;                                                          \
    reg_id_val VPCM_GAMCOR_RAMA_OFFSET_R;                                                          \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_0_1;                                                        \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_2_3;                                                        \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_4_5;                                                        \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_6_7;                                                        \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_8_9;                                                        \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_10_11;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_12_13;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_14_15;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_16_17;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_18_19;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_20_21;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_22_23;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_24_25;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_26_27;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_28_29;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_30_31;                                                      \
    reg_id_val VPCM_GAMCOR_RAMA_REGION_32_33;                                                      \
    reg_id_val VPCM_HDR_MULT_COEF;                                                                 \
    reg_id_val VPCM_MEM_PWR_CTRL;                                                                  \
    reg_id_val VPCM_MEM_PWR_STATUS;                                                                \
    reg_id_val VPCM_DEALPHA;                                                                       \
    reg_id_val VPCM_COEF_FORMAT;                                                                   \
    reg_id_val VPDPP_CONTROL;                                                                      \
    reg_id_val VPDPP_CRC_CTRL;

#define DPP_FIELD_VARIABLE_LIST_VPE10(type)                                                        \
    type VPCNVC_SURFACE_PIXEL_FORMAT;                                                              \
    type FORMAT_EXPANSION_MODE;                                                                    \
    type FORMAT_CNV16;                                                                             \
    type FORMAT_CONTROL__ALPHA_EN;                                                                 \
    type VPCNVC_BYPASS;                                                                            \
    type VPCNVC_BYPASS_MSB_ALIGN;                                                                  \
    type CLAMP_POSITIVE;                                                                           \
    type CLAMP_POSITIVE_C;                                                                         \
    type VPCNVC_UPDATE_PENDING;                                                                    \
    type FCNV_FP_BIAS_R;                                                                           \
    type FCNV_FP_BIAS_G;                                                                           \
    type FCNV_FP_BIAS_B;                                                                           \
    type FCNV_FP_SCALE_R;                                                                          \
    type FCNV_FP_SCALE_G;                                                                          \
    type FCNV_FP_SCALE_B;                                                                          \
    type COLOR_KEYER_EN;                                                                           \
    type COLOR_KEYER_MODE;                                                                         \
    type COLOR_KEYER_ALPHA_LOW;                                                                    \
    type COLOR_KEYER_ALPHA_HIGH;                                                                   \
    type COLOR_KEYER_RED_LOW;                                                                      \
    type COLOR_KEYER_RED_HIGH;                                                                     \
    type COLOR_KEYER_GREEN_LOW;                                                                    \
    type COLOR_KEYER_GREEN_HIGH;                                                                   \
    type COLOR_KEYER_BLUE_LOW;                                                                     \
    type COLOR_KEYER_BLUE_HIGH;                                                                    \
    type ALPHA_2BIT_LUT0;                                                                          \
    type ALPHA_2BIT_LUT1;                                                                          \
    type ALPHA_2BIT_LUT2;                                                                          \
    type ALPHA_2BIT_LUT3;                                                                          \
    type PRE_DEALPHA_EN;                                                                           \
    type PRE_DEALPHA_ABLND_EN;                                                                     \
    type PRE_CSC_MODE;                                                                             \
    type PRE_CSC_MODE_CURRENT;                                                                     \
    type PRE_CSC_C11;                                                                              \
    type PRE_CSC_C12;                                                                              \
    type PRE_CSC_C13;                                                                              \
    type PRE_CSC_C14;                                                                              \
    type PRE_CSC_C21;                                                                              \
    type PRE_CSC_C22;                                                                              \
    type PRE_CSC_C23;                                                                              \
    type PRE_CSC_C24;                                                                              \
    type PRE_CSC_C31;                                                                              \
    type PRE_CSC_C32;                                                                              \
    type PRE_CSC_C33;                                                                              \
    type PRE_CSC_C34;                                                                              \
    type PRE_CSC_COEF_FORMAT;                                                                      \
    type PRE_DEGAM_MODE;                                                                           \
    type PRE_DEGAM_SELECT;                                                                         \
    type PRE_REALPHA_EN;                                                                           \
    type PRE_REALPHA_ABLND_EN;                                                                     \
    type SCL_COEF_RAM_TAP_PAIR_IDX;                                                                \
    type SCL_COEF_RAM_PHASE;                                                                       \
    type SCL_COEF_RAM_FILTER_TYPE;                                                                 \
    type SCL_COEF_RAM_EVEN_TAP_COEF;                                                               \
    type SCL_COEF_RAM_EVEN_TAP_COEF_EN;                                                            \
    type SCL_COEF_RAM_ODD_TAP_COEF;                                                                \
    type SCL_COEF_RAM_ODD_TAP_COEF_EN;                                                             \
    type VPDSCL_MODE;                                                                              \
    type SCL_COEF_RAM_SELECT_CURRENT;                                                              \
    type SCL_CHROMA_COEF_MODE;                                                                     \
    type SCL_ALPHA_COEF_MODE;                                                                      \
    type SCL_COEF_RAM_SELECT_RD;                                                                   \
    type SCL_V_NUM_TAPS;                                                                           \
    type SCL_H_NUM_TAPS;                                                                           \
    type SCL_V_NUM_TAPS_C;                                                                         \
    type SCL_H_NUM_TAPS_C;                                                                         \
    type SCL_BOUNDARY_MODE;                                                                        \
    type SCL_H_2TAP_HARDCODE_COEF_EN;                                                              \
    type SCL_H_2TAP_SHARP_EN;                                                                      \
    type SCL_H_2TAP_SHARP_FACTOR;                                                                  \
    type SCL_V_2TAP_HARDCODE_COEF_EN;                                                              \
    type SCL_V_2TAP_SHARP_EN;                                                                      \
    type SCL_V_2TAP_SHARP_FACTOR;                                                                  \
    type SCL_V_MANUAL_REPLICATE_FACTOR;                                                            \
    type SCL_H_MANUAL_REPLICATE_FACTOR;                                                            \
    type SCL_H_SCALE_RATIO;                                                                        \
    type SCL_H_INIT_FRAC;                                                                          \
    type SCL_H_INIT_INT;                                                                           \
    type SCL_H_SCALE_RATIO_C;                                                                      \
    type SCL_H_INIT_FRAC_C;                                                                        \
    type SCL_H_INIT_INT_C;                                                                         \
    type SCL_V_SCALE_RATIO;                                                                        \
    type SCL_V_INIT_FRAC;                                                                          \
    type SCL_V_INIT_INT;                                                                           \
    type SCL_V_SCALE_RATIO_C;                                                                      \
    type SCL_V_INIT_FRAC_C;                                                                        \
    type SCL_V_INIT_INT_C;                                                                         \
    type SCL_BLACK_COLOR_RGB_Y;                                                                    \
    type SCL_BLACK_COLOR_CBCR;                                                                     \
    type SCL_UPDATE_PENDING;                                                                       \
    type AUTOCAL_MODE;                                                                             \
    type EXT_OVERSCAN_RIGHT;                                                                       \
    type EXT_OVERSCAN_LEFT;                                                                        \
    type EXT_OVERSCAN_BOTTOM;                                                                      \
    type EXT_OVERSCAN_TOP;                                                                         \
    type OTG_H_BLANK_START;                                                                        \
    type OTG_H_BLANK_END;                                                                          \
    type OTG_V_BLANK_START;                                                                        \
    type OTG_V_BLANK_END;                                                                          \
    type RECOUT_START_X;                                                                           \
    type RECOUT_START_Y;                                                                           \
    type RECOUT_WIDTH;                                                                             \
    type RECOUT_HEIGHT;                                                                            \
    type VPMPC_WIDTH;                                                                              \
    type VPMPC_HEIGHT;                                                                             \
    type ALPHA_EN;                                                                                 \
    type MEMORY_CONFIG;                                                                            \
    type LB_MAX_PARTITIONS;                                                                        \
    type LB_NUM_PARTITIONS;                                                                        \
    type LB_NUM_PARTITIONS_C;                                                                      \
    type V_COUNTER;                                                                                \
    type V_COUNTER_C;                                                                              \
    type LUT_MEM_PWR_FORCE;                                                                        \
    type LUT_MEM_PWR_DIS;                                                                          \
    type LB_G1_MEM_PWR_FORCE;                                                                      \
    type LB_G1_MEM_PWR_DIS;                                                                        \
    type LB_G2_MEM_PWR_FORCE;                                                                      \
    type LB_G2_MEM_PWR_DIS;                                                                        \
    type LB_MEM_PWR_MODE;                                                                          \
    type LUT_MEM_PWR_STATE;                                                                        \
    type LB_G1_MEM_PWR_STATE;                                                                      \
    type LB_G2_MEM_PWR_STATE;                                                                      \
    type VPCM_BYPASS;                                                                              \
    type VPCM_UPDATE_PENDING;                                                                      \
    type VPCM_POST_CSC_MODE;                                                                       \
    type VPCM_POST_CSC_MODE_CURRENT;                                                               \
    type VPCM_POST_CSC_C11;                                                                        \
    type VPCM_POST_CSC_C12;                                                                        \
    type VPCM_POST_CSC_C13;                                                                        \
    type VPCM_POST_CSC_C14;                                                                        \
    type VPCM_POST_CSC_C21;                                                                        \
    type VPCM_POST_CSC_C22;                                                                        \
    type VPCM_POST_CSC_C23;                                                                        \
    type VPCM_POST_CSC_C24;                                                                        \
    type VPCM_POST_CSC_C31;                                                                        \
    type VPCM_POST_CSC_C32;                                                                        \
    type VPCM_POST_CSC_C33;                                                                        \
    type VPCM_POST_CSC_C34;                                                                        \
    type VPCM_GAMUT_REMAP_MODE;                                                                    \
    type VPCM_GAMUT_REMAP_MODE_CURRENT;                                                            \
    type VPCM_GAMUT_REMAP_C11;                                                                     \
    type VPCM_GAMUT_REMAP_C12;                                                                     \
    type VPCM_GAMUT_REMAP_C13;                                                                     \
    type VPCM_GAMUT_REMAP_C14;                                                                     \
    type VPCM_GAMUT_REMAP_C21;                                                                     \
    type VPCM_GAMUT_REMAP_C22;                                                                     \
    type VPCM_GAMUT_REMAP_C23;                                                                     \
    type VPCM_GAMUT_REMAP_C24;                                                                     \
    type VPCM_GAMUT_REMAP_C31;                                                                     \
    type VPCM_GAMUT_REMAP_C32;                                                                     \
    type VPCM_GAMUT_REMAP_C33;                                                                     \
    type VPCM_GAMUT_REMAP_C34;                                                                     \
    type VPCM_BIAS_CR_R;                                                                           \
    type VPCM_BIAS_Y_G;                                                                            \
    type VPCM_BIAS_CB_B;                                                                           \
    type VPCM_GAMCOR_MODE;                                                                         \
    type VPCM_GAMCOR_PWL_DISABLE;                                                                  \
    type VPCM_GAMCOR_MODE_CURRENT;                                                                 \
    type VPCM_GAMCOR_SELECT_CURRENT;                                                               \
    type VPCM_GAMCOR_LUT_INDEX;                                                                    \
    type VPCM_GAMCOR_LUT_DATA;                                                                     \
    type VPCM_GAMCOR_LUT_WRITE_COLOR_MASK;                                                         \
    type VPCM_GAMCOR_LUT_READ_COLOR_SEL;                                                           \
    type VPCM_GAMCOR_LUT_READ_DBG;                                                                 \
    type VPCM_GAMCOR_LUT_HOST_SEL;                                                                 \
    type VPCM_GAMCOR_LUT_CONFIG_MODE;                                                              \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_B;                                                      \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_SEGMENT_B;                                              \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_G;                                                      \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_SEGMENT_G;                                              \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_R;                                                      \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_SEGMENT_R;                                              \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_SLOPE_B;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_SLOPE_G;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_SLOPE_R;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_BASE_B;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_BASE_G;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION_START_BASE_R;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION_END_BASE_B;                                                   \
    type VPCM_GAMCOR_RAMA_EXP_REGION_END_B;                                                        \
    type VPCM_GAMCOR_RAMA_EXP_REGION_END_SLOPE_B;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION_END_BASE_G;                                                   \
    type VPCM_GAMCOR_RAMA_EXP_REGION_END_G;                                                        \
    type VPCM_GAMCOR_RAMA_EXP_REGION_END_SLOPE_G;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION_END_BASE_R;                                                   \
    type VPCM_GAMCOR_RAMA_EXP_REGION_END_R;                                                        \
    type VPCM_GAMCOR_RAMA_EXP_REGION_END_SLOPE_R;                                                  \
    type VPCM_GAMCOR_RAMA_OFFSET_B;                                                                \
    type VPCM_GAMCOR_RAMA_OFFSET_G;                                                                \
    type VPCM_GAMCOR_RAMA_OFFSET_R;                                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION0_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION0_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION1_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION1_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION2_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION2_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION3_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION3_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION4_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION4_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION5_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION5_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION6_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION6_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION7_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION7_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION8_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION8_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION9_LUT_OFFSET;                                                  \
    type VPCM_GAMCOR_RAMA_EXP_REGION9_NUM_SEGMENTS;                                                \
    type VPCM_GAMCOR_RAMA_EXP_REGION10_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION10_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION11_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION11_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION12_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION12_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION13_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION13_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION14_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION14_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION15_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION15_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION16_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION16_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION17_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION17_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION18_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION18_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION19_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION19_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION20_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION20_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION21_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION21_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION22_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION22_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION23_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION23_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION24_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION24_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION25_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION25_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION26_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION26_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION27_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION27_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION28_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION28_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION29_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION29_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION30_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION30_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION31_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION31_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION32_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION32_NUM_SEGMENTS;                                               \
    type VPCM_GAMCOR_RAMA_EXP_REGION33_LUT_OFFSET;                                                 \
    type VPCM_GAMCOR_RAMA_EXP_REGION33_NUM_SEGMENTS;                                               \
    type VPCM_HDR_MULT_COEF;                                                                       \
    type GAMCOR_MEM_PWR_FORCE;                                                                     \
    type GAMCOR_MEM_PWR_DIS;                                                                       \
    type GAMCOR_MEM_PWR_STATE;                                                                     \
    type VPCM_DEALPHA_EN;                                                                          \
    type VPCM_DEALPHA_ABLND;                                                                       \
    type VPCM_BIAS_FORMAT;                                                                         \
    type VPCM_POST_CSC_COEF_FORMAT;                                                                \
    type VPCM_GAMUT_REMAP_COEF_FORMAT;                                                             \
    type VPDPP_CLOCK_ENABLE;                                                                       \
    type VPECLK_G_GATE_DISABLE;                                                                    \
    type VPECLK_G_DYN_GATE_DISABLE;                                                                \
    type VPECLK_G_VPDSCL_GATE_DISABLE;                                                             \
    type VPECLK_R_GATE_DISABLE;                                                                    \
    type DISPCLK_R_GATE_DISABLE;                                                                   \
    type DISPCLK_G_GATE_DISABLE;                                                                   \
    type VPDPP_FGCG_REP_DIS;                                                                       \
    type VPDPP_TEST_CLK_SEL;                                                                       \
    type VPDPP_CRC_EN;                                                                             \
    type VPDPP_CRC_CONT_EN;                                                                        \
    type VPDPP_CRC_420_COMP_SEL;                                                                   \
    type VPDPP_CRC_SRC_SEL;                                                                        \
    type VPDPP_CRC_PIX_FORMAT_SEL;                                                                 \
    type VPDPP_CRC_MASK;

#define IDENTITY_RATIO(ratio) (vpe_fixpt_u3d19(ratio) == (1 << 19))

struct vpe10_dpp_registers {
    DPP_REG_VARIABLE_LIST_VPE10
};

struct vpe10_dpp_shift {
    DPP_FIELD_VARIABLE_LIST_VPE10(uint8_t)
};

struct vpe10_dpp_mask {
    DPP_FIELD_VARIABLE_LIST_VPE10(uint32_t)
};

struct vpe10_dpp {
    struct dpp                    base; // base class, must be the 1st field
    struct vpe10_dpp_registers   *regs;
    const struct vpe10_dpp_shift *shift;
    const struct vpe10_dpp_mask  *mask;
};

void vpe10_construct_dpp(struct vpe_priv *vpe_priv, struct dpp *dpp);

bool vpe10_dpp_get_optimal_number_of_taps(
    struct dpp *dpp, struct scaler_data *scl_data, const struct vpe_scaling_taps *in_taps);

void vpe10_dscl_calc_lb_num_partitions(const struct scaler_data *scl_data,
    enum lb_memory_config lb_config, uint32_t *num_part_y, uint32_t *num_part_c);

/***** share register programming *****/
void vpe10_dpp_program_cnv(
    struct dpp *dpp, enum vpe_surface_pixel_format format, enum vpe_expansion_mode mode);

void vpe10_dpp_cnv_program_pre_dgam(struct dpp *dpp, enum color_transfer_func tr);

void vpe10_dpp_program_cnv_bias_scale(struct dpp *dpp, struct bias_and_scale *bias_and_scale);

void vpe10_dpp_cnv_program_alpha_keyer(struct dpp *dpp, struct cnv_color_keyer_params *color_keyer);

void vpe10_dpp_program_input_transfer_func(struct dpp *dpp, struct transfer_func *input_tf);

void vpe10_dpp_program_gamut_remap(struct dpp *dpp, struct colorspace_transform *gamut_remap);

/*program post scaler scs block in dpp CM*/
void vpe10_dpp_program_post_csc(struct dpp *dpp, enum color_space color_space,
    enum input_csc_select input_select, struct vpe_csc_matrix *input_cs);

void vpe10_dpp_set_hdr_multiplier(struct dpp *dpp, uint32_t multiplier);

/*Program Scaler*/
void vpe10_dpp_set_segment_scaler(struct dpp *dpp, const struct scaler_data *scl_data);

void vpe10_dpp_set_frame_scaler(struct dpp *dpp, const struct scaler_data *scl_data);

uint32_t vpe10_get_line_buffer_size(void);

bool vpe10_dpp_validate_number_of_taps(struct dpp *dpp, struct scaler_data *scl_data);

void vpe10_dpp_program_crc(struct dpp *dpp, bool enable);
#ifdef __cplusplus
}
#endif
