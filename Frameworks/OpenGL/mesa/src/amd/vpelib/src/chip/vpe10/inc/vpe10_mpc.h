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

#include "mpc.h"
#include "reg_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MPC_REG_LIST_VPE10(id)                                                                     \
    SRIDFVL(VPMPC_CLOCK_CONTROL, VPMPC_CFG, id), SRIDFVL(VPMPC_SOFT_RESET, VPMPC_CFG, id),         \
        SRIDFVL(VPMPC_CRC_CTRL, VPMPC_CFG, id), SRIDFVL(VPMPC_CRC_SEL_CONTROL, VPMPC_CFG, id),     \
        SRIDFVL(VPMPC_CRC_RESULT_AR, VPMPC_CFG, id), SRIDFVL(VPMPC_CRC_RESULT_GB, VPMPC_CFG, id),  \
        SRIDFVL(VPMPC_CRC_RESULT_C, VPMPC_CFG, id), SRIDFVL(VPMPC_BYPASS_BG_AR, VPMPC_CFG, id),    \
        SRIDFVL(VPMPC_BYPASS_BG_GB, VPMPC_CFG, id),                                                \
        SRIDFVL(VPMPC_HOST_READ_CONTROL, VPMPC_CFG, id),                                           \
        SRIDFVL(VPMPC_PENDING_STATUS_MISC, VPMPC_CFG, id),                                         \
        SRIDFVL(VPMPC_OUT0_MUX, VPMPC_OCSC, id),                                                   \
        SRIDFVL(VPMPC_OUT0_FLOAT_CONTROL, VPMPC_OCSC, id),                                         \
        SRIDFVL(VPMPC_OUT0_DENORM_CONTROL, VPMPC_OCSC, id),                                        \
        SRIDFVL(VPMPC_OUT0_DENORM_CLAMP_G_Y, VPMPC_OCSC, id),                                      \
        SRIDFVL(VPMPC_OUT0_DENORM_CLAMP_B_CB, VPMPC_OCSC, id),                                     \
        SRIDFVL(VPMPC_OUT_CSC_COEF_FORMAT, VPMPC_OCSC, id),                                        \
        SRIDFVL(VPMPC_OUT0_CSC_MODE, VPMPC_OCSC, id),                                              \
        SRIDFVL(VPMPC_OUT0_CSC_C11_C12_A, VPMPC_OCSC, id),                                         \
        SRIDFVL(VPMPC_OUT0_CSC_C13_C14_A, VPMPC_OCSC, id),                                         \
        SRIDFVL(VPMPC_OUT0_CSC_C21_C22_A, VPMPC_OCSC, id),                                         \
        SRIDFVL(VPMPC_OUT0_CSC_C23_C24_A, VPMPC_OCSC, id),                                         \
        SRIDFVL(VPMPC_OUT0_CSC_C31_C32_A, VPMPC_OCSC, id),                                         \
        SRIDFVL(VPMPC_OUT0_CSC_C33_C34_A, VPMPC_OCSC, id), SRIDFVL(VPMPCC_TOP_SEL, VPMPCC, id),    \
        SRIDFVL(VPMPCC_BOT_SEL, VPMPCC, id), SRIDFVL(VPMPCC_VPOPP_ID, VPMPCC, id),                 \
        SRIDFVL(VPMPCC_CONTROL, VPMPCC, id), SRIDFVL(VPMPCC_TOP_GAIN, VPMPCC, id),                 \
        SRIDFVL(VPMPCC_BOT_GAIN_INSIDE, VPMPCC, id), SRIDFVL(VPMPCC_BOT_GAIN_OUTSIDE, VPMPCC, id), \
        SRIDFVL(VPMPCC_MOVABLE_CM_LOCATION_CONTROL, VPMPCC, id),                                   \
        SRIDFVL(VPMPCC_BG_R_CR, VPMPCC, id), SRIDFVL(VPMPCC_BG_G_Y, VPMPCC, id),                   \
        SRIDFVL(VPMPCC_BG_B_CB, VPMPCC, id), SRIDFVL(VPMPCC_MEM_PWR_CTRL, VPMPCC, id),             \
        SRIDFVL(VPMPCC_STATUS, VPMPCC, id), SRIDFVL(VPMPCC_OGAM_CONTROL, VPMPCC_OGAM, id),         \
        SRIDFVL(VPMPCC_OGAM_LUT_INDEX, VPMPCC_OGAM, id),                                           \
        SRIDFVL(VPMPCC_OGAM_LUT_DATA, VPMPCC_OGAM, id),                                            \
        SRIDFVL(VPMPCC_OGAM_LUT_CONTROL, VPMPCC_OGAM, id),                                         \
        SRIDFVL(VPMPCC_OGAM_RAMA_START_CNTL_B, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_START_CNTL_G, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_START_CNTL_R, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_B, VPMPCC_OGAM, id),                             \
        SRIDFVL(VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_G, VPMPCC_OGAM, id),                             \
        SRIDFVL(VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_R, VPMPCC_OGAM, id),                             \
        SRIDFVL(VPMPCC_OGAM_RAMA_START_BASE_CNTL_B, VPMPCC_OGAM, id),                              \
        SRIDFVL(VPMPCC_OGAM_RAMA_START_BASE_CNTL_G, VPMPCC_OGAM, id),                              \
        SRIDFVL(VPMPCC_OGAM_RAMA_START_BASE_CNTL_R, VPMPCC_OGAM, id),                              \
        SRIDFVL(VPMPCC_OGAM_RAMA_END_CNTL1_B, VPMPCC_OGAM, id),                                    \
        SRIDFVL(VPMPCC_OGAM_RAMA_END_CNTL2_B, VPMPCC_OGAM, id),                                    \
        SRIDFVL(VPMPCC_OGAM_RAMA_END_CNTL1_G, VPMPCC_OGAM, id),                                    \
        SRIDFVL(VPMPCC_OGAM_RAMA_END_CNTL2_G, VPMPCC_OGAM, id),                                    \
        SRIDFVL(VPMPCC_OGAM_RAMA_END_CNTL1_R, VPMPCC_OGAM, id),                                    \
        SRIDFVL(VPMPCC_OGAM_RAMA_END_CNTL2_R, VPMPCC_OGAM, id),                                    \
        SRIDFVL(VPMPCC_OGAM_RAMA_OFFSET_B, VPMPCC_OGAM, id),                                       \
        SRIDFVL(VPMPCC_OGAM_RAMA_OFFSET_G, VPMPCC_OGAM, id),                                       \
        SRIDFVL(VPMPCC_OGAM_RAMA_OFFSET_R, VPMPCC_OGAM, id),                                       \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_0_1, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_2_3, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_4_5, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_6_7, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_8_9, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_10_11, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_12_13, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_14_15, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_16_17, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_18_19, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_20_21, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_22_23, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_24_25, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_26_27, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_28_29, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_30_31, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_OGAM_RAMA_REGION_32_33, VPMPCC_OGAM, id),                                   \
        SRIDFVL(VPMPCC_GAMUT_REMAP_COEF_FORMAT, VPMPCC_OGAM, id),                                  \
        SRIDFVL(VPMPCC_GAMUT_REMAP_MODE, VPMPCC_OGAM, id),                                         \
        SRIDFVL(VPMPC_GAMUT_REMAP_C11_C12_A, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPC_GAMUT_REMAP_C13_C14_A, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPC_GAMUT_REMAP_C21_C22_A, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPC_GAMUT_REMAP_C23_C24_A, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPC_GAMUT_REMAP_C31_C32_A, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPC_GAMUT_REMAP_C33_C34_A, VPMPCC_OGAM, id),                                     \
        SRIDFVL(VPMPCC_MCM_SHAPER_CONTROL, VPMPCC_MCM, id),                                        \
        SRIDFVL(VPMPCC_MCM_SHAPER_OFFSET_R, VPMPCC_MCM, id),                                       \
        SRIDFVL(VPMPCC_MCM_SHAPER_OFFSET_G, VPMPCC_MCM, id),                                       \
        SRIDFVL(VPMPCC_MCM_SHAPER_OFFSET_B, VPMPCC_MCM, id),                                       \
        SRIDFVL(VPMPCC_MCM_SHAPER_SCALE_R, VPMPCC_MCM, id),                                        \
        SRIDFVL(VPMPCC_MCM_SHAPER_SCALE_G_B, VPMPCC_MCM, id),                                      \
        SRIDFVL(VPMPCC_MCM_SHAPER_LUT_INDEX, VPMPCC_MCM, id),                                      \
        SRIDFVL(VPMPCC_MCM_SHAPER_LUT_DATA, VPMPCC_MCM, id),                                       \
        SRIDFVL(VPMPCC_MCM_SHAPER_LUT_WRITE_EN_MASK, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_START_CNTL_B, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_START_CNTL_G, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_START_CNTL_R, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_END_CNTL_B, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_END_CNTL_G, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_END_CNTL_R, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_0_1, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_2_3, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_4_5, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_6_7, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_8_9, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_10_11, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_12_13, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_14_15, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_16_17, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_18_19, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_20_21, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_22_23, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_24_25, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_26_27, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_28_29, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_30_31, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_SHAPER_RAMA_REGION_32_33, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_3DLUT_MODE, VPMPCC_MCM, id),                                            \
        SRIDFVL(VPMPCC_MCM_3DLUT_INDEX, VPMPCC_MCM, id),                                           \
        SRIDFVL(VPMPCC_MCM_3DLUT_DATA, VPMPCC_MCM, id),                                            \
        SRIDFVL(VPMPCC_MCM_3DLUT_DATA_30BIT, VPMPCC_MCM, id),                                      \
        SRIDFVL(VPMPCC_MCM_3DLUT_READ_WRITE_CONTROL, VPMPCC_MCM, id),                              \
        SRIDFVL(VPMPCC_MCM_3DLUT_OUT_NORM_FACTOR, VPMPCC_MCM, id),                                 \
        SRIDFVL(VPMPCC_MCM_3DLUT_OUT_OFFSET_R, VPMPCC_MCM, id),                                    \
        SRIDFVL(VPMPCC_MCM_3DLUT_OUT_OFFSET_G, VPMPCC_MCM, id),                                    \
        SRIDFVL(VPMPCC_MCM_3DLUT_OUT_OFFSET_B, VPMPCC_MCM, id),                                    \
        SRIDFVL(VPMPCC_MCM_1DLUT_CONTROL, VPMPCC_MCM, id),                                         \
        SRIDFVL(VPMPCC_MCM_1DLUT_LUT_INDEX, VPMPCC_MCM, id),                                       \
        SRIDFVL(VPMPCC_MCM_1DLUT_LUT_DATA, VPMPCC_MCM, id),                                        \
        SRIDFVL(VPMPCC_MCM_1DLUT_LUT_CONTROL, VPMPCC_MCM, id),                                     \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_START_CNTL_B, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_START_CNTL_G, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_START_CNTL_R, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_B, VPMPCC_MCM, id),                         \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_G, VPMPCC_MCM, id),                         \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_R, VPMPCC_MCM, id),                         \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_B, VPMPCC_MCM, id),                          \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_G, VPMPCC_MCM, id),                          \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_R, VPMPCC_MCM, id),                          \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_B, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_B, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_G, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_G, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_R, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_R, VPMPCC_MCM, id),                                \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_OFFSET_B, VPMPCC_MCM, id),                                   \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_OFFSET_G, VPMPCC_MCM, id),                                   \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_OFFSET_R, VPMPCC_MCM, id),                                   \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_0_1, VPMPCC_MCM, id),                                 \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_2_3, VPMPCC_MCM, id),                                 \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_4_5, VPMPCC_MCM, id),                                 \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_6_7, VPMPCC_MCM, id),                                 \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_8_9, VPMPCC_MCM, id),                                 \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_10_11, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_12_13, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_14_15, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_16_17, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_18_19, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_20_21, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_22_23, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_24_25, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_26_27, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_28_29, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_30_31, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_1DLUT_RAMA_REGION_32_33, VPMPCC_MCM, id),                               \
        SRIDFVL(VPMPCC_MCM_MEM_PWR_CTRL, VPMPCC_MCM, id),

#define MPC_FIELD_LIST_VPE10(post_fix)                                                             \
    SFRB(VPECLK_R_GATE_DISABLE, VPMPC_CLOCK_CONTROL, post_fix),                                    \
        SFRB(VPMPC_TEST_CLK_SEL, VPMPC_CLOCK_CONTROL, post_fix),                                   \
        SFRB(VPMPCC0_SOFT_RESET, VPMPC_SOFT_RESET, post_fix),                                      \
        SFRB(VPMPC_SFR0_SOFT_RESET, VPMPC_SOFT_RESET, post_fix),                                   \
        SFRB(VPMPC_SFT0_SOFT_RESET, VPMPC_SOFT_RESET, post_fix),                                   \
        SFRB(VPMPC_SOFT_RESET, VPMPC_SOFT_RESET, post_fix),                                        \
        SFRB(VPMPC_CRC_EN, VPMPC_CRC_CTRL, post_fix),                                              \
        SFRB(VPMPC_CRC_CONT_EN, VPMPC_CRC_CTRL, post_fix),                                         \
        SFRB(VPMPC_CRC_SRC_SEL, VPMPC_CRC_CTRL, post_fix),                                         \
        SFRB(VPMPC_CRC_ONE_SHOT_PENDING, VPMPC_CRC_CTRL, post_fix),                                \
        SFRB(VPMPC_CRC_UPDATE_ENABLED, VPMPC_CRC_CTRL, post_fix),                                  \
        SFRB(VPMPC_CRC_UPDATE_LOCK, VPMPC_CRC_CTRL, post_fix),                                     \
        SFRB(VPMPC_CRC_VPDPP_SEL, VPMPC_CRC_SEL_CONTROL, post_fix),                                \
        SFRB(VPMPC_CRC_VPOPP_SEL, VPMPC_CRC_SEL_CONTROL, post_fix),                                \
        SFRB(VPMPC_CRC_MASK, VPMPC_CRC_SEL_CONTROL, post_fix),                                     \
        SFRB(VPMPC_CRC_RESULT_A, VPMPC_CRC_RESULT_AR, post_fix),                                   \
        SFRB(VPMPC_CRC_RESULT_R, VPMPC_CRC_RESULT_AR, post_fix),                                   \
        SFRB(VPMPC_CRC_RESULT_G, VPMPC_CRC_RESULT_GB, post_fix),                                   \
        SFRB(VPMPC_CRC_RESULT_B, VPMPC_CRC_RESULT_GB, post_fix),                                   \
        SFRB(VPMPC_CRC_RESULT_C, VPMPC_CRC_RESULT_C, post_fix),                                    \
        SFRB(VPMPC_BYPASS_BG_ALPHA, VPMPC_BYPASS_BG_AR, post_fix),                                 \
        SFRB(VPMPC_BYPASS_BG_R_CR, VPMPC_BYPASS_BG_AR, post_fix),                                  \
        SFRB(VPMPC_BYPASS_BG_G_Y, VPMPC_BYPASS_BG_GB, post_fix),                                   \
        SFRB(VPMPC_BYPASS_BG_B_CB, VPMPC_BYPASS_BG_GB, post_fix),                                  \
        SFRB(HOST_READ_RATE_CONTROL, VPMPC_HOST_READ_CONTROL, post_fix),                           \
        SFRB(VPMPCC0_CONFIG_UPDATE_PENDING, VPMPC_PENDING_STATUS_MISC, post_fix),                  \
        SFRB(VPMPC_OUT_MUX, VPMPC_OUT0_MUX, post_fix),                                             \
        SFRB(VPMPC_OUT_FLOAT_EN, VPMPC_OUT0_FLOAT_CONTROL, post_fix),                              \
        SFRB(VPMPC_OUT_DENORM_CLAMP_MIN_R_CR, VPMPC_OUT0_DENORM_CONTROL, post_fix),                \
        SFRB(VPMPC_OUT_DENORM_CLAMP_MAX_R_CR, VPMPC_OUT0_DENORM_CONTROL, post_fix),                \
        SFRB(VPMPC_OUT_DENORM_MODE, VPMPC_OUT0_DENORM_CONTROL, post_fix),                          \
        SFRB(VPMPC_OUT_DENORM_CLAMP_MIN_G_Y, VPMPC_OUT0_DENORM_CLAMP_G_Y, post_fix),               \
        SFRB(VPMPC_OUT_DENORM_CLAMP_MAX_G_Y, VPMPC_OUT0_DENORM_CLAMP_G_Y, post_fix),               \
        SFRB(VPMPC_OUT_DENORM_CLAMP_MIN_B_CB, VPMPC_OUT0_DENORM_CLAMP_B_CB, post_fix),             \
        SFRB(VPMPC_OUT_DENORM_CLAMP_MAX_B_CB, VPMPC_OUT0_DENORM_CLAMP_B_CB, post_fix),             \
        SFRB(VPMPC_OCSC0_COEF_FORMAT, VPMPC_OUT_CSC_COEF_FORMAT, post_fix),                        \
        SFRB(VPMPC_OCSC_MODE, VPMPC_OUT0_CSC_MODE, post_fix),                                      \
        SFRB(VPMPC_OCSC_MODE_CURRENT, VPMPC_OUT0_CSC_MODE, post_fix),                              \
        SFRB(VPMPC_OCSC_C11_A, VPMPC_OUT0_CSC_C11_C12_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C12_A, VPMPC_OUT0_CSC_C11_C12_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C13_A, VPMPC_OUT0_CSC_C13_C14_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C14_A, VPMPC_OUT0_CSC_C13_C14_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C21_A, VPMPC_OUT0_CSC_C21_C22_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C22_A, VPMPC_OUT0_CSC_C21_C22_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C23_A, VPMPC_OUT0_CSC_C23_C24_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C24_A, VPMPC_OUT0_CSC_C23_C24_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C31_A, VPMPC_OUT0_CSC_C31_C32_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C32_A, VPMPC_OUT0_CSC_C31_C32_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C33_A, VPMPC_OUT0_CSC_C33_C34_A, post_fix),                                \
        SFRB(VPMPC_OCSC_C34_A, VPMPC_OUT0_CSC_C33_C34_A, post_fix),                                \
        SFRB(VPMPCC_TOP_SEL, VPMPCC_TOP_SEL, post_fix),                                            \
        SFRB(VPMPCC_BOT_SEL, VPMPCC_BOT_SEL, post_fix),                                            \
        SFRB(VPMPCC_VPOPP_ID, VPMPCC_VPOPP_ID, post_fix),                                          \
        SFRB(VPMPCC_MODE, VPMPCC_CONTROL, post_fix),                                               \
        SFRB(VPMPCC_ALPHA_BLND_MODE, VPMPCC_CONTROL, post_fix),                                    \
        SFRB(VPMPCC_ALPHA_MULTIPLIED_MODE, VPMPCC_CONTROL, post_fix),                              \
        SFRB(VPMPCC_BLND_ACTIVE_OVERLAP_ONLY, VPMPCC_CONTROL, post_fix),                           \
        SFRB(VPMPCC_BG_BPC, VPMPCC_CONTROL, post_fix),                                             \
        SFRB(VPMPCC_BOT_GAIN_MODE, VPMPCC_CONTROL, post_fix),                                      \
        SFRB(VPMPCC_GLOBAL_ALPHA, VPMPCC_CONTROL, post_fix),                                       \
        SFRB(VPMPCC_GLOBAL_GAIN, VPMPCC_CONTROL, post_fix),                                        \
        SFRB(VPMPCC_TOP_GAIN, VPMPCC_TOP_GAIN, post_fix),                                          \
        SFRB(VPMPCC_BOT_GAIN_INSIDE, VPMPCC_BOT_GAIN_INSIDE, post_fix),                            \
        SFRB(VPMPCC_BOT_GAIN_OUTSIDE, VPMPCC_BOT_GAIN_OUTSIDE, post_fix),                          \
        SFRB(VPMPCC_MOVABLE_CM_LOCATION_CNTL, VPMPCC_MOVABLE_CM_LOCATION_CONTROL, post_fix),       \
        SFRB(VPMPCC_MOVABLE_CM_LOCATION_CNTL_CURRENT, VPMPCC_MOVABLE_CM_LOCATION_CONTROL,          \
            post_fix),                                                                             \
        SFRB(VPMPCC_BG_R_CR, VPMPCC_BG_R_CR, post_fix),                                            \
        SFRB(VPMPCC_BG_G_Y, VPMPCC_BG_G_Y, post_fix),                                              \
        SFRB(VPMPCC_BG_B_CB, VPMPCC_BG_B_CB, post_fix),                                            \
        SFRB(VPMPCC_OGAM_MEM_PWR_FORCE, VPMPCC_MEM_PWR_CTRL, post_fix),                            \
        SFRB(VPMPCC_OGAM_MEM_PWR_DIS, VPMPCC_MEM_PWR_CTRL, post_fix),                              \
        SFRB(VPMPCC_OGAM_MEM_LOW_PWR_MODE, VPMPCC_MEM_PWR_CTRL, post_fix),                         \
        SFRB(VPMPCC_OGAM_MEM_PWR_STATE, VPMPCC_MEM_PWR_CTRL, post_fix),                            \
        SFRB(VPMPCC_IDLE, VPMPCC_STATUS, post_fix), SFRB(VPMPCC_BUSY, VPMPCC_STATUS, post_fix),    \
        SFRB(VPMPCC_DISABLED, VPMPCC_STATUS, post_fix),                                            \
        SFRB(VPMPCC_OGAM_MODE, VPMPCC_OGAM_CONTROL, post_fix),                                     \
        SFRB(VPMPCC_OGAM_PWL_DISABLE, VPMPCC_OGAM_CONTROL, post_fix),                              \
        SFRB(VPMPCC_OGAM_MODE_CURRENT, VPMPCC_OGAM_CONTROL, post_fix),                             \
        SFRB(VPMPCC_OGAM_SELECT_CURRENT, VPMPCC_OGAM_CONTROL, post_fix),                           \
        SFRB(VPMPCC_OGAM_LUT_INDEX, VPMPCC_OGAM_LUT_INDEX, post_fix),                              \
        SFRB(VPMPCC_OGAM_LUT_DATA, VPMPCC_OGAM_LUT_DATA, post_fix),                                \
        SFRB(VPMPCC_OGAM_LUT_WRITE_COLOR_MASK, VPMPCC_OGAM_LUT_CONTROL, post_fix),                 \
        SFRB(VPMPCC_OGAM_LUT_READ_COLOR_SEL, VPMPCC_OGAM_LUT_CONTROL, post_fix),                   \
        SFRB(VPMPCC_OGAM_LUT_READ_DBG, VPMPCC_OGAM_LUT_CONTROL, post_fix),                         \
        SFRB(VPMPCC_OGAM_LUT_HOST_SEL, VPMPCC_OGAM_LUT_CONTROL, post_fix),                         \
        SFRB(VPMPCC_OGAM_LUT_CONFIG_MODE, VPMPCC_OGAM_LUT_CONTROL, post_fix),                      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_START_B, VPMPCC_OGAM_RAMA_START_CNTL_B, post_fix),        \
        SFRB(                                                                                      \
            VPMPCC_OGAM_RAMA_EXP_REGION_START_SEGMENT_B, VPMPCC_OGAM_RAMA_START_CNTL_B, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_START_G, VPMPCC_OGAM_RAMA_START_CNTL_G, post_fix),        \
        SFRB(                                                                                      \
            VPMPCC_OGAM_RAMA_EXP_REGION_START_SEGMENT_G, VPMPCC_OGAM_RAMA_START_CNTL_G, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_START_R, VPMPCC_OGAM_RAMA_START_CNTL_R, post_fix),        \
        SFRB(                                                                                      \
            VPMPCC_OGAM_RAMA_EXP_REGION_START_SEGMENT_R, VPMPCC_OGAM_RAMA_START_CNTL_R, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_START_SLOPE_B, VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_B,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_START_SLOPE_G, VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_G,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_START_SLOPE_R, VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_R,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_START_BASE_B, VPMPCC_OGAM_RAMA_START_BASE_CNTL_B,         \
            post_fix),                                                                             \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_START_BASE_G, VPMPCC_OGAM_RAMA_START_BASE_CNTL_G,         \
            post_fix),                                                                             \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_START_BASE_R, VPMPCC_OGAM_RAMA_START_BASE_CNTL_R,         \
            post_fix),                                                                             \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_END_BASE_B, VPMPCC_OGAM_RAMA_END_CNTL1_B, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_END_B, VPMPCC_OGAM_RAMA_END_CNTL2_B, post_fix),           \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_END_SLOPE_B, VPMPCC_OGAM_RAMA_END_CNTL2_B, post_fix),     \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_END_BASE_G, VPMPCC_OGAM_RAMA_END_CNTL1_G, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_END_G, VPMPCC_OGAM_RAMA_END_CNTL2_G, post_fix),           \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_END_SLOPE_G, VPMPCC_OGAM_RAMA_END_CNTL2_G, post_fix),     \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_END_BASE_R, VPMPCC_OGAM_RAMA_END_CNTL1_R, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_END_R, VPMPCC_OGAM_RAMA_END_CNTL2_R, post_fix),           \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION_END_SLOPE_R, VPMPCC_OGAM_RAMA_END_CNTL2_R, post_fix),     \
        SFRB(VPMPCC_OGAM_RAMA_OFFSET_B, VPMPCC_OGAM_RAMA_OFFSET_B, post_fix),                      \
        SFRB(VPMPCC_OGAM_RAMA_OFFSET_G, VPMPCC_OGAM_RAMA_OFFSET_G, post_fix),                      \
        SFRB(VPMPCC_OGAM_RAMA_OFFSET_R, VPMPCC_OGAM_RAMA_OFFSET_R, post_fix),                      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION0_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_0_1, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION0_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_0_1, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION1_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_0_1, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION1_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_0_1, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION2_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_2_3, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION2_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_2_3, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION3_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_2_3, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION3_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_2_3, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION4_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_4_5, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION4_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_4_5, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION5_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_4_5, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION5_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_4_5, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION6_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_6_7, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION6_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_6_7, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION7_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_6_7, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION7_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_6_7, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION8_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_8_9, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION8_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_8_9, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION9_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_8_9, post_fix),      \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION9_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_8_9, post_fix),    \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION10_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_10_11, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION10_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_10_11, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION11_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_10_11, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION11_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_10_11, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION12_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_12_13, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION12_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_12_13, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION13_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_12_13, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION13_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_12_13, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION14_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_14_15, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION14_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_14_15, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION15_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_14_15, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION15_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_14_15, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION16_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_16_17, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION16_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_16_17, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION17_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_16_17, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION17_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_16_17, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION18_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_18_19, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION18_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_18_19, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION19_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_18_19, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION19_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_18_19, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION20_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_20_21, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION20_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_20_21, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION21_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_20_21, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION21_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_20_21, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION22_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_22_23, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION22_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_22_23, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION23_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_22_23, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION23_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_22_23, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION24_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_24_25, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION24_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_24_25, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION25_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_24_25, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION25_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_24_25, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION26_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_26_27, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION26_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_26_27, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION27_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_26_27, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION27_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_26_27, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION28_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_28_29, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION28_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_28_29, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION29_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_28_29, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION29_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_28_29, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION30_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_30_31, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION30_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_30_31, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION31_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_30_31, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION31_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_30_31, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION32_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_32_33, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION32_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_32_33, post_fix), \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION33_LUT_OFFSET, VPMPCC_OGAM_RAMA_REGION_32_33, post_fix),   \
        SFRB(VPMPCC_OGAM_RAMA_EXP_REGION33_NUM_SEGMENTS, VPMPCC_OGAM_RAMA_REGION_32_33, post_fix), \
        SFRB(VPMPCC_GAMUT_REMAP_COEF_FORMAT, VPMPCC_GAMUT_REMAP_COEF_FORMAT, post_fix),            \
        SFRB(VPMPCC_GAMUT_REMAP_MODE, VPMPCC_GAMUT_REMAP_MODE, post_fix),                          \
        SFRB(VPMPCC_GAMUT_REMAP_MODE_CURRENT, VPMPCC_GAMUT_REMAP_MODE, post_fix),                  \
        SFRB(VPMPCC_GAMUT_REMAP_C11_A, VPMPC_GAMUT_REMAP_C11_C12_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C12_A, VPMPC_GAMUT_REMAP_C11_C12_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C13_A, VPMPC_GAMUT_REMAP_C13_C14_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C14_A, VPMPC_GAMUT_REMAP_C13_C14_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C21_A, VPMPC_GAMUT_REMAP_C21_C22_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C22_A, VPMPC_GAMUT_REMAP_C21_C22_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C23_A, VPMPC_GAMUT_REMAP_C23_C24_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C24_A, VPMPC_GAMUT_REMAP_C23_C24_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C31_A, VPMPC_GAMUT_REMAP_C31_C32_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C32_A, VPMPC_GAMUT_REMAP_C31_C32_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C33_A, VPMPC_GAMUT_REMAP_C33_C34_A, post_fix),                     \
        SFRB(VPMPCC_GAMUT_REMAP_C34_A, VPMPC_GAMUT_REMAP_C33_C34_A, post_fix),                     \
        SFRB(VPMPCC_MCM_SHAPER_LUT_MODE, VPMPCC_MCM_SHAPER_CONTROL, post_fix),                     \
        SFRB(VPMPCC_MCM_SHAPER_MODE_CURRENT, VPMPCC_MCM_SHAPER_CONTROL, post_fix),                 \
        SFRB(VPMPCC_MCM_SHAPER_SELECT_CURRENT, VPMPCC_MCM_SHAPER_CONTROL, post_fix),               \
        SFRB(VPMPCC_MCM_SHAPER_OFFSET_R, VPMPCC_MCM_SHAPER_OFFSET_R, post_fix),                    \
        SFRB(VPMPCC_MCM_SHAPER_OFFSET_G, VPMPCC_MCM_SHAPER_OFFSET_G, post_fix),                    \
        SFRB(VPMPCC_MCM_SHAPER_OFFSET_B, VPMPCC_MCM_SHAPER_OFFSET_B, post_fix),                    \
        SFRB(VPMPCC_MCM_SHAPER_SCALE_R, VPMPCC_MCM_SHAPER_SCALE_R, post_fix),                      \
        SFRB(VPMPCC_MCM_SHAPER_SCALE_G, VPMPCC_MCM_SHAPER_SCALE_G_B, post_fix),                    \
        SFRB(VPMPCC_MCM_SHAPER_SCALE_B, VPMPCC_MCM_SHAPER_SCALE_G_B, post_fix),                    \
        SFRB(VPMPCC_MCM_SHAPER_LUT_INDEX, VPMPCC_MCM_SHAPER_LUT_INDEX, post_fix),                  \
        SFRB(VPMPCC_MCM_SHAPER_LUT_DATA, VPMPCC_MCM_SHAPER_LUT_DATA, post_fix),                    \
        SFRB(VPMPCC_MCM_SHAPER_LUT_WRITE_EN_MASK, VPMPCC_MCM_SHAPER_LUT_WRITE_EN_MASK, post_fix),  \
        SFRB(VPMPCC_MCM_SHAPER_LUT_WRITE_SEL, VPMPCC_MCM_SHAPER_LUT_WRITE_EN_MASK, post_fix),      \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_B, VPMPCC_MCM_SHAPER_RAMA_START_CNTL_B,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_SEGMENT_B,                                    \
            VPMPCC_MCM_SHAPER_RAMA_START_CNTL_B, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_G, VPMPCC_MCM_SHAPER_RAMA_START_CNTL_G,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_SEGMENT_G,                                    \
            VPMPCC_MCM_SHAPER_RAMA_START_CNTL_G, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_R, VPMPCC_MCM_SHAPER_RAMA_START_CNTL_R,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_SEGMENT_R,                                    \
            VPMPCC_MCM_SHAPER_RAMA_START_CNTL_R, post_fix),                                        \
        SFRB(                                                                                      \
            VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_B, VPMPCC_MCM_SHAPER_RAMA_END_CNTL_B, post_fix), \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_BASE_B, VPMPCC_MCM_SHAPER_RAMA_END_CNTL_B,      \
            post_fix),                                                                             \
        SFRB(                                                                                      \
            VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_G, VPMPCC_MCM_SHAPER_RAMA_END_CNTL_G, post_fix), \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_BASE_G, VPMPCC_MCM_SHAPER_RAMA_END_CNTL_G,      \
            post_fix),                                                                             \
        SFRB(                                                                                      \
            VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_R, VPMPCC_MCM_SHAPER_RAMA_END_CNTL_R, post_fix), \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_BASE_R, VPMPCC_MCM_SHAPER_RAMA_END_CNTL_R,      \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION0_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_0_1,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION0_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_0_1,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION1_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_0_1,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION1_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_0_1,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION2_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_2_3,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION2_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_2_3,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION3_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_2_3,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION3_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_2_3,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION4_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_4_5,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION4_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_4_5,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION5_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_4_5,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION5_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_4_5,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION6_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_6_7,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION6_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_6_7,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION7_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_6_7,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION7_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_6_7,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION8_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_8_9,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION8_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_8_9,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION9_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_8_9,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION9_NUM_SEGMENTS, VPMPCC_MCM_SHAPER_RAMA_REGION_8_9,   \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION10_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_10_11,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION10_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_10_11, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION11_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_10_11,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION11_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_10_11, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION12_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_12_13,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION12_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_12_13, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION13_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_12_13,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION13_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_12_13, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION14_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_14_15,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION14_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_14_15, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION15_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_14_15,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION15_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_14_15, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION16_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_16_17,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION16_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_16_17, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION17_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_16_17,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION17_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_16_17, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION18_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_18_19,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION18_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_18_19, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION19_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_18_19,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION19_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_18_19, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION20_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_20_21,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION20_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_20_21, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION21_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_20_21,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION21_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_20_21, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION22_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_22_23,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION22_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_22_23, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION23_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_22_23,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION23_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_22_23, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION24_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_24_25,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION24_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_24_25, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION25_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_24_25,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION25_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_24_25, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION26_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_26_27,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION26_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_26_27, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION27_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_26_27,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION27_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_26_27, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION28_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_28_29,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION28_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_28_29, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION29_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_28_29,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION29_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_28_29, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION30_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_30_31,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION30_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_30_31, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION31_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_30_31,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION31_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_30_31, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION32_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_32_33,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION32_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_32_33, post_fix),                                        \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION33_LUT_OFFSET, VPMPCC_MCM_SHAPER_RAMA_REGION_32_33,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION33_NUM_SEGMENTS,                                     \
            VPMPCC_MCM_SHAPER_RAMA_REGION_32_33, post_fix),                                        \
        SFRB(VPMPCC_MCM_3DLUT_MODE, VPMPCC_MCM_3DLUT_MODE, post_fix),                              \
        SFRB(VPMPCC_MCM_3DLUT_SIZE, VPMPCC_MCM_3DLUT_MODE, post_fix),                              \
        SFRB(VPMPCC_MCM_3DLUT_MODE_CURRENT, VPMPCC_MCM_3DLUT_MODE, post_fix),                      \
        SFRB(VPMPCC_MCM_3DLUT_SELECT_CURRENT, VPMPCC_MCM_3DLUT_MODE, post_fix),                    \
        SFRB(VPMPCC_MCM_3DLUT_INDEX, VPMPCC_MCM_3DLUT_INDEX, post_fix),                            \
        SFRB(VPMPCC_MCM_3DLUT_DATA0, VPMPCC_MCM_3DLUT_DATA, post_fix),                             \
        SFRB(VPMPCC_MCM_3DLUT_DATA1, VPMPCC_MCM_3DLUT_DATA, post_fix),                             \
        SFRB(VPMPCC_MCM_3DLUT_DATA_30BIT, VPMPCC_MCM_3DLUT_DATA_30BIT, post_fix),                  \
        SFRB(VPMPCC_MCM_3DLUT_WRITE_EN_MASK, VPMPCC_MCM_3DLUT_READ_WRITE_CONTROL, post_fix),       \
        SFRB(VPMPCC_MCM_3DLUT_RAM_SEL, VPMPCC_MCM_3DLUT_READ_WRITE_CONTROL, post_fix),             \
        SFRB(VPMPCC_MCM_3DLUT_30BIT_EN, VPMPCC_MCM_3DLUT_READ_WRITE_CONTROL, post_fix),            \
        SFRB(VPMPCC_MCM_3DLUT_READ_SEL, VPMPCC_MCM_3DLUT_READ_WRITE_CONTROL, post_fix),            \
        SFRB(VPMPCC_MCM_3DLUT_OUT_NORM_FACTOR, VPMPCC_MCM_3DLUT_OUT_NORM_FACTOR, post_fix),        \
        SFRB(VPMPCC_MCM_3DLUT_OUT_OFFSET_R, VPMPCC_MCM_3DLUT_OUT_OFFSET_R, post_fix),              \
        SFRB(VPMPCC_MCM_3DLUT_OUT_SCALE_R, VPMPCC_MCM_3DLUT_OUT_OFFSET_R, post_fix),               \
        SFRB(VPMPCC_MCM_3DLUT_OUT_OFFSET_G, VPMPCC_MCM_3DLUT_OUT_OFFSET_G, post_fix),              \
        SFRB(VPMPCC_MCM_3DLUT_OUT_SCALE_G, VPMPCC_MCM_3DLUT_OUT_OFFSET_G, post_fix),               \
        SFRB(VPMPCC_MCM_3DLUT_OUT_OFFSET_B, VPMPCC_MCM_3DLUT_OUT_OFFSET_B, post_fix),              \
        SFRB(VPMPCC_MCM_3DLUT_OUT_SCALE_B, VPMPCC_MCM_3DLUT_OUT_OFFSET_B, post_fix),               \
        SFRB(VPMPCC_MCM_1DLUT_MODE, VPMPCC_MCM_1DLUT_CONTROL, post_fix),                           \
        SFRB(VPMPCC_MCM_1DLUT_PWL_DISABLE, VPMPCC_MCM_1DLUT_CONTROL, post_fix),                    \
        SFRB(VPMPCC_MCM_1DLUT_MODE_CURRENT, VPMPCC_MCM_1DLUT_CONTROL, post_fix),                   \
        SFRB(VPMPCC_MCM_1DLUT_SELECT_CURRENT, VPMPCC_MCM_1DLUT_CONTROL, post_fix),                 \
        SFRB(VPMPCC_MCM_1DLUT_LUT_INDEX, VPMPCC_MCM_1DLUT_LUT_INDEX, post_fix),                    \
        SFRB(VPMPCC_MCM_1DLUT_LUT_DATA, VPMPCC_MCM_1DLUT_LUT_DATA, post_fix),                      \
        SFRB(VPMPCC_MCM_1DLUT_LUT_WRITE_COLOR_MASK, VPMPCC_MCM_1DLUT_LUT_CONTROL, post_fix),       \
        SFRB(VPMPCC_MCM_1DLUT_LUT_READ_COLOR_SEL, VPMPCC_MCM_1DLUT_LUT_CONTROL, post_fix),         \
        SFRB(VPMPCC_MCM_1DLUT_LUT_READ_DBG, VPMPCC_MCM_1DLUT_LUT_CONTROL, post_fix),               \
        SFRB(VPMPCC_MCM_1DLUT_LUT_HOST_SEL, VPMPCC_MCM_1DLUT_LUT_CONTROL, post_fix),               \
        SFRB(VPMPCC_MCM_1DLUT_LUT_CONFIG_MODE, VPMPCC_MCM_1DLUT_LUT_CONTROL, post_fix),            \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_B, VPMPCC_MCM_1DLUT_RAMA_START_CNTL_B,         \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SEGMENT_B, VPMPCC_MCM_1DLUT_RAMA_START_CNTL_B, \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_G, VPMPCC_MCM_1DLUT_RAMA_START_CNTL_G,         \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SEGMENT_G, VPMPCC_MCM_1DLUT_RAMA_START_CNTL_G, \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_R, VPMPCC_MCM_1DLUT_RAMA_START_CNTL_R,         \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SEGMENT_R, VPMPCC_MCM_1DLUT_RAMA_START_CNTL_R, \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SLOPE_B,                                       \
            VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_B, post_fix),                                   \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SLOPE_G,                                       \
            VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_G, post_fix),                                   \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SLOPE_R,                                       \
            VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_R, post_fix),                                   \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_BASE_B,                                        \
            VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_B, post_fix),                                    \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_BASE_G,                                        \
            VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_G, post_fix),                                    \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_BASE_R,                                        \
            VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_R, post_fix),                                    \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_BASE_B, VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_B,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_B, VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_B, post_fix), \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_SLOPE_B, VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_B,      \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_BASE_G, VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_G,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_G, VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_G, post_fix), \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_SLOPE_G, VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_G,      \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_BASE_R, VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_R,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_R, VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_R, post_fix), \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_SLOPE_R, VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_R,      \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_OFFSET_B, VPMPCC_MCM_1DLUT_RAMA_OFFSET_B, post_fix),            \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_OFFSET_G, VPMPCC_MCM_1DLUT_RAMA_OFFSET_G, post_fix),            \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_OFFSET_R, VPMPCC_MCM_1DLUT_RAMA_OFFSET_R, post_fix),            \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION0_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_0_1,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION0_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_0_1,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION1_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_0_1,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION1_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_0_1,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION2_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_2_3,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION2_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_2_3,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION3_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_2_3,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION3_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_2_3,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION4_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_4_5,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION4_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_4_5,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION5_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_4_5,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION5_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_4_5,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION6_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_6_7,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION6_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_6_7,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION7_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_6_7,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION7_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_6_7,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION8_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_8_9,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION8_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_8_9,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION9_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_8_9,       \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION9_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_8_9,     \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION10_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_10_11,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION10_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_10_11,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION11_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_10_11,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION11_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_10_11,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION12_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_12_13,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION12_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_12_13,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION13_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_12_13,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION13_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_12_13,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION14_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_14_15,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION14_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_14_15,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION15_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_14_15,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION15_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_14_15,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION16_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_16_17,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION16_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_16_17,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION17_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_16_17,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION17_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_16_17,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION18_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_18_19,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION18_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_18_19,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION19_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_18_19,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION19_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_18_19,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION20_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_20_21,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION20_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_20_21,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION21_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_20_21,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION21_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_20_21,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION22_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_22_23,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION22_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_22_23,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION23_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_22_23,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION23_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_22_23,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION24_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_24_25,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION24_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_24_25,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION25_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_24_25,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION25_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_24_25,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION26_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_26_27,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION26_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_26_27,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION27_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_26_27,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION27_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_26_27,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION28_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_28_29,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION28_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_28_29,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION29_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_28_29,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION29_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_28_29,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION30_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_30_31,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION30_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_30_31,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION31_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_30_31,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION31_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_30_31,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION32_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_32_33,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION32_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_32_33,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION33_LUT_OFFSET, VPMPCC_MCM_1DLUT_RAMA_REGION_32_33,    \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_1DLUT_RAMA_EXP_REGION33_NUM_SEGMENTS, VPMPCC_MCM_1DLUT_RAMA_REGION_32_33,  \
            post_fix),                                                                             \
        SFRB(VPMPCC_MCM_SHAPER_MEM_PWR_FORCE, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                  \
        SFRB(VPMPCC_MCM_SHAPER_MEM_PWR_DIS, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                    \
        SFRB(VPMPCC_MCM_SHAPER_MEM_LOW_PWR_MODE, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),               \
        SFRB(VPMPCC_MCM_3DLUT_MEM_PWR_FORCE, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                   \
        SFRB(VPMPCC_MCM_3DLUT_MEM_PWR_DIS, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                     \
        SFRB(VPMPCC_MCM_3DLUT_MEM_LOW_PWR_MODE, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                \
        SFRB(VPMPCC_MCM_1DLUT_MEM_PWR_FORCE, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                   \
        SFRB(VPMPCC_MCM_1DLUT_MEM_PWR_DIS, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                     \
        SFRB(VPMPCC_MCM_1DLUT_MEM_LOW_PWR_MODE, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                \
        SFRB(VPMPCC_MCM_SHAPER_MEM_PWR_STATE, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                  \
        SFRB(VPMPCC_MCM_3DLUT_MEM_PWR_STATE, VPMPCC_MCM_MEM_PWR_CTRL, post_fix),                   \
        SFRB(VPMPCC_MCM_1DLUT_MEM_PWR_STATE, VPMPCC_MCM_MEM_PWR_CTRL, post_fix)

#define MPC_REG_VARIABLE_LIST_VPE10                                                                \
    reg_id_val VPMPC_CLOCK_CONTROL;                                                                \
    reg_id_val VPMPC_SOFT_RESET;                                                                   \
    reg_id_val VPMPC_CRC_CTRL;                                                                     \
    reg_id_val VPMPC_CRC_SEL_CONTROL;                                                              \
    reg_id_val VPMPC_CRC_RESULT_AR;                                                                \
    reg_id_val VPMPC_CRC_RESULT_GB;                                                                \
    reg_id_val VPMPC_CRC_RESULT_C;                                                                 \
    reg_id_val VPMPC_BYPASS_BG_AR;                                                                 \
    reg_id_val VPMPC_BYPASS_BG_GB;                                                                 \
    reg_id_val VPMPC_HOST_READ_CONTROL;                                                            \
    reg_id_val VPMPC_PENDING_STATUS_MISC;                                                          \
    reg_id_val VPMPC_OUT0_MUX;                                                                     \
    reg_id_val VPMPC_OUT0_FLOAT_CONTROL;                                                           \
    reg_id_val VPMPC_OUT0_DENORM_CONTROL;                                                          \
    reg_id_val VPMPC_OUT0_DENORM_CLAMP_G_Y;                                                        \
    reg_id_val VPMPC_OUT0_DENORM_CLAMP_B_CB;                                                       \
    reg_id_val VPMPC_OUT_CSC_COEF_FORMAT;                                                          \
    reg_id_val VPMPC_OUT0_CSC_MODE;                                                                \
    reg_id_val VPMPC_OUT0_CSC_C11_C12_A;                                                           \
    reg_id_val VPMPC_OUT0_CSC_C13_C14_A;                                                           \
    reg_id_val VPMPC_OUT0_CSC_C21_C22_A;                                                           \
    reg_id_val VPMPC_OUT0_CSC_C23_C24_A;                                                           \
    reg_id_val VPMPC_OUT0_CSC_C31_C32_A;                                                           \
    reg_id_val VPMPC_OUT0_CSC_C33_C34_A;                                                           \
    reg_id_val VPMPCC_TOP_SEL;                                                                     \
    reg_id_val VPMPCC_BOT_SEL;                                                                     \
    reg_id_val VPMPCC_VPOPP_ID;                                                                    \
    reg_id_val VPMPCC_CONTROL;                                                                     \
    reg_id_val VPMPCC_TOP_GAIN;                                                                    \
    reg_id_val VPMPCC_BOT_GAIN_INSIDE;                                                             \
    reg_id_val VPMPCC_BOT_GAIN_OUTSIDE;                                                            \
    reg_id_val VPMPCC_MOVABLE_CM_LOCATION_CONTROL;                                                 \
    reg_id_val VPMPCC_BG_R_CR;                                                                     \
    reg_id_val VPMPCC_BG_G_Y;                                                                      \
    reg_id_val VPMPCC_BG_B_CB;                                                                     \
    reg_id_val VPMPCC_MEM_PWR_CTRL;                                                                \
    reg_id_val VPMPCC_STATUS;                                                                      \
    reg_id_val VPMPCC_OGAM_CONTROL;                                                                \
    reg_id_val VPMPCC_OGAM_LUT_INDEX;                                                              \
    reg_id_val VPMPCC_OGAM_LUT_DATA;                                                               \
    reg_id_val VPMPCC_OGAM_LUT_CONTROL;                                                            \
    reg_id_val VPMPCC_OGAM_RAMA_START_CNTL_B;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_START_CNTL_G;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_START_CNTL_R;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_B;                                                \
    reg_id_val VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_G;                                                \
    reg_id_val VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_R;                                                \
    reg_id_val VPMPCC_OGAM_RAMA_START_BASE_CNTL_B;                                                 \
    reg_id_val VPMPCC_OGAM_RAMA_START_BASE_CNTL_G;                                                 \
    reg_id_val VPMPCC_OGAM_RAMA_START_BASE_CNTL_R;                                                 \
    reg_id_val VPMPCC_OGAM_RAMA_END_CNTL1_B;                                                       \
    reg_id_val VPMPCC_OGAM_RAMA_END_CNTL2_B;                                                       \
    reg_id_val VPMPCC_OGAM_RAMA_END_CNTL1_G;                                                       \
    reg_id_val VPMPCC_OGAM_RAMA_END_CNTL2_G;                                                       \
    reg_id_val VPMPCC_OGAM_RAMA_END_CNTL1_R;                                                       \
    reg_id_val VPMPCC_OGAM_RAMA_END_CNTL2_R;                                                       \
    reg_id_val VPMPCC_OGAM_RAMA_OFFSET_B;                                                          \
    reg_id_val VPMPCC_OGAM_RAMA_OFFSET_G;                                                          \
    reg_id_val VPMPCC_OGAM_RAMA_OFFSET_R;                                                          \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_0_1;                                                        \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_2_3;                                                        \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_4_5;                                                        \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_6_7;                                                        \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_8_9;                                                        \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_10_11;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_12_13;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_14_15;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_16_17;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_18_19;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_20_21;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_22_23;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_24_25;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_26_27;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_28_29;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_30_31;                                                      \
    reg_id_val VPMPCC_OGAM_RAMA_REGION_32_33;                                                      \
    reg_id_val VPMPCC_GAMUT_REMAP_COEF_FORMAT;                                                     \
    reg_id_val VPMPCC_GAMUT_REMAP_MODE;                                                            \
    reg_id_val VPMPC_GAMUT_REMAP_C11_C12_A;                                                        \
    reg_id_val VPMPC_GAMUT_REMAP_C13_C14_A;                                                        \
    reg_id_val VPMPC_GAMUT_REMAP_C21_C22_A;                                                        \
    reg_id_val VPMPC_GAMUT_REMAP_C23_C24_A;                                                        \
    reg_id_val VPMPC_GAMUT_REMAP_C31_C32_A;                                                        \
    reg_id_val VPMPC_GAMUT_REMAP_C33_C34_A;                                                        \
    reg_id_val VPMPCC_MCM_SHAPER_CONTROL;                                                          \
    reg_id_val VPMPCC_MCM_SHAPER_OFFSET_R;                                                         \
    reg_id_val VPMPCC_MCM_SHAPER_OFFSET_G;                                                         \
    reg_id_val VPMPCC_MCM_SHAPER_OFFSET_B;                                                         \
    reg_id_val VPMPCC_MCM_SHAPER_SCALE_R;                                                          \
    reg_id_val VPMPCC_MCM_SHAPER_SCALE_G_B;                                                        \
    reg_id_val VPMPCC_MCM_SHAPER_LUT_INDEX;                                                        \
    reg_id_val VPMPCC_MCM_SHAPER_LUT_DATA;                                                         \
    reg_id_val VPMPCC_MCM_SHAPER_LUT_WRITE_EN_MASK;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_START_CNTL_B;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_START_CNTL_G;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_START_CNTL_R;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_END_CNTL_B;                                                  \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_END_CNTL_G;                                                  \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_END_CNTL_R;                                                  \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_0_1;                                                  \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_2_3;                                                  \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_4_5;                                                  \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_6_7;                                                  \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_8_9;                                                  \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_10_11;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_12_13;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_14_15;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_16_17;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_18_19;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_20_21;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_22_23;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_24_25;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_26_27;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_28_29;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_30_31;                                                \
    reg_id_val VPMPCC_MCM_SHAPER_RAMA_REGION_32_33;                                                \
    reg_id_val VPMPCC_MCM_3DLUT_MODE;                                                              \
    reg_id_val VPMPCC_MCM_3DLUT_INDEX;                                                             \
    reg_id_val VPMPCC_MCM_3DLUT_DATA;                                                              \
    reg_id_val VPMPCC_MCM_3DLUT_DATA_30BIT;                                                        \
    reg_id_val VPMPCC_MCM_3DLUT_READ_WRITE_CONTROL;                                                \
    reg_id_val VPMPCC_MCM_3DLUT_OUT_NORM_FACTOR;                                                   \
    reg_id_val VPMPCC_MCM_3DLUT_OUT_OFFSET_R;                                                      \
    reg_id_val VPMPCC_MCM_3DLUT_OUT_OFFSET_G;                                                      \
    reg_id_val VPMPCC_MCM_3DLUT_OUT_OFFSET_B;                                                      \
    reg_id_val VPMPCC_MCM_1DLUT_CONTROL;                                                           \
    reg_id_val VPMPCC_MCM_1DLUT_LUT_INDEX;                                                         \
    reg_id_val VPMPCC_MCM_1DLUT_LUT_DATA;                                                          \
    reg_id_val VPMPCC_MCM_1DLUT_LUT_CONTROL;                                                       \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_START_CNTL_B;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_START_CNTL_G;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_START_CNTL_R;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_B;                                           \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_G;                                           \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_R;                                           \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_B;                                            \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_G;                                            \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_R;                                            \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_B;                                                  \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_B;                                                  \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_G;                                                  \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_G;                                                  \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_R;                                                  \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_R;                                                  \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_OFFSET_B;                                                     \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_OFFSET_G;                                                     \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_OFFSET_R;                                                     \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_0_1;                                                   \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_2_3;                                                   \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_4_5;                                                   \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_6_7;                                                   \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_8_9;                                                   \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_10_11;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_12_13;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_14_15;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_16_17;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_18_19;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_20_21;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_22_23;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_24_25;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_26_27;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_28_29;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_30_31;                                                 \
    reg_id_val VPMPCC_MCM_1DLUT_RAMA_REGION_32_33;                                                 \
    reg_id_val VPMPCC_MCM_MEM_PWR_CTRL;

#define MPC_FIELD_VARIABLE_LIST_VPE10(type)                                                        \
    type VPECLK_R_GATE_DISABLE;                                                                    \
    type VPMPC_TEST_CLK_SEL;                                                                       \
    type VPMPCC0_SOFT_RESET;                                                                       \
    type VPMPC_SFR0_SOFT_RESET;                                                                    \
    type VPMPC_SFT0_SOFT_RESET;                                                                    \
    type VPMPC_SOFT_RESET;                                                                         \
    type VPMPC_CRC_EN;                                                                             \
    type VPMPC_CRC_CONT_EN;                                                                        \
    type VPMPC_CRC_SRC_SEL;                                                                        \
    type VPMPC_CRC_ONE_SHOT_PENDING;                                                               \
    type VPMPC_CRC_UPDATE_ENABLED;                                                                 \
    type VPMPC_CRC_UPDATE_LOCK;                                                                    \
    type VPMPC_CRC_VPDPP_SEL;                                                                      \
    type VPMPC_CRC_VPOPP_SEL;                                                                      \
    type VPMPC_CRC_MASK;                                                                           \
    type VPMPC_CRC_RESULT_A;                                                                       \
    type VPMPC_CRC_RESULT_R;                                                                       \
    type VPMPC_CRC_RESULT_G;                                                                       \
    type VPMPC_CRC_RESULT_B;                                                                       \
    type VPMPC_CRC_RESULT_C;                                                                       \
    type VPMPC_BYPASS_BG_ALPHA;                                                                    \
    type VPMPC_BYPASS_BG_R_CR;                                                                     \
    type VPMPC_BYPASS_BG_G_Y;                                                                      \
    type VPMPC_BYPASS_BG_B_CB;                                                                     \
    type HOST_READ_RATE_CONTROL;                                                                   \
    type VPMPCC0_CONFIG_UPDATE_PENDING;                                                            \
    type VPMPC_OUT_MUX;                                                                            \
    type VPMPC_OUT_FLOAT_EN;                                                                       \
    type VPMPC_OUT_DENORM_CLAMP_MIN_R_CR;                                                          \
    type VPMPC_OUT_DENORM_CLAMP_MAX_R_CR;                                                          \
    type VPMPC_OUT_DENORM_MODE;                                                                    \
    type VPMPC_OUT_DENORM_CLAMP_MIN_G_Y;                                                           \
    type VPMPC_OUT_DENORM_CLAMP_MAX_G_Y;                                                           \
    type VPMPC_OUT_DENORM_CLAMP_MIN_B_CB;                                                          \
    type VPMPC_OUT_DENORM_CLAMP_MAX_B_CB;                                                          \
    type VPMPC_OCSC0_COEF_FORMAT;                                                                  \
    type VPMPC_OCSC_MODE;                                                                          \
    type VPMPC_OCSC_MODE_CURRENT;                                                                  \
    type VPMPC_OCSC_C11_A;                                                                         \
    type VPMPC_OCSC_C12_A;                                                                         \
    type VPMPC_OCSC_C13_A;                                                                         \
    type VPMPC_OCSC_C14_A;                                                                         \
    type VPMPC_OCSC_C21_A;                                                                         \
    type VPMPC_OCSC_C22_A;                                                                         \
    type VPMPC_OCSC_C23_A;                                                                         \
    type VPMPC_OCSC_C24_A;                                                                         \
    type VPMPC_OCSC_C31_A;                                                                         \
    type VPMPC_OCSC_C32_A;                                                                         \
    type VPMPC_OCSC_C33_A;                                                                         \
    type VPMPC_OCSC_C34_A;                                                                         \
    type VPMPCC_TOP_SEL;                                                                           \
    type VPMPCC_BOT_SEL;                                                                           \
    type VPMPCC_VPOPP_ID;                                                                          \
    type VPMPCC_MODE;                                                                              \
    type VPMPCC_ALPHA_BLND_MODE;                                                                   \
    type VPMPCC_ALPHA_MULTIPLIED_MODE;                                                             \
    type VPMPCC_BLND_ACTIVE_OVERLAP_ONLY;                                                          \
    type VPMPCC_BG_BPC;                                                                            \
    type VPMPCC_BOT_GAIN_MODE;                                                                     \
    type VPMPCC_GLOBAL_ALPHA;                                                                      \
    type VPMPCC_GLOBAL_GAIN;                                                                       \
    type VPMPCC_TOP_GAIN;                                                                          \
    type VPMPCC_BOT_GAIN_INSIDE;                                                                   \
    type VPMPCC_BOT_GAIN_OUTSIDE;                                                                  \
    type VPMPCC_MOVABLE_CM_LOCATION_CNTL;                                                          \
    type VPMPCC_MOVABLE_CM_LOCATION_CNTL_CURRENT;                                                  \
    type VPMPCC_BG_R_CR;                                                                           \
    type VPMPCC_BG_G_Y;                                                                            \
    type VPMPCC_BG_B_CB;                                                                           \
    type VPMPCC_OGAM_MEM_PWR_FORCE;                                                                \
    type VPMPCC_OGAM_MEM_PWR_DIS;                                                                  \
    type VPMPCC_OGAM_MEM_LOW_PWR_MODE;                                                             \
    type VPMPCC_OGAM_MEM_PWR_STATE;                                                                \
    type VPMPCC_IDLE;                                                                              \
    type VPMPCC_BUSY;                                                                              \
    type VPMPCC_DISABLED;                                                                          \
    type VPMPCC_OGAM_MODE;                                                                         \
    type VPMPCC_OGAM_PWL_DISABLE;                                                                  \
    type VPMPCC_OGAM_MODE_CURRENT;                                                                 \
    type VPMPCC_OGAM_SELECT_CURRENT;                                                               \
    type VPMPCC_OGAM_LUT_INDEX;                                                                    \
    type VPMPCC_OGAM_LUT_DATA;                                                                     \
    type VPMPCC_OGAM_LUT_WRITE_COLOR_MASK;                                                         \
    type VPMPCC_OGAM_LUT_READ_COLOR_SEL;                                                           \
    type VPMPCC_OGAM_LUT_READ_DBG;                                                                 \
    type VPMPCC_OGAM_LUT_HOST_SEL;                                                                 \
    type VPMPCC_OGAM_LUT_CONFIG_MODE;                                                              \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_B;                                                      \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_SEGMENT_B;                                              \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_G;                                                      \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_SEGMENT_G;                                              \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_R;                                                      \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_SEGMENT_R;                                              \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_SLOPE_B;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_SLOPE_G;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_SLOPE_R;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_BASE_B;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_BASE_G;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION_START_BASE_R;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION_END_BASE_B;                                                   \
    type VPMPCC_OGAM_RAMA_EXP_REGION_END_B;                                                        \
    type VPMPCC_OGAM_RAMA_EXP_REGION_END_SLOPE_B;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION_END_BASE_G;                                                   \
    type VPMPCC_OGAM_RAMA_EXP_REGION_END_G;                                                        \
    type VPMPCC_OGAM_RAMA_EXP_REGION_END_SLOPE_G;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION_END_BASE_R;                                                   \
    type VPMPCC_OGAM_RAMA_EXP_REGION_END_R;                                                        \
    type VPMPCC_OGAM_RAMA_EXP_REGION_END_SLOPE_R;                                                  \
    type VPMPCC_OGAM_RAMA_OFFSET_B;                                                                \
    type VPMPCC_OGAM_RAMA_OFFSET_G;                                                                \
    type VPMPCC_OGAM_RAMA_OFFSET_R;                                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION0_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION0_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION1_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION1_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION2_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION2_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION3_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION3_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION4_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION4_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION5_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION5_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION6_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION6_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION7_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION7_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION8_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION8_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION9_LUT_OFFSET;                                                  \
    type VPMPCC_OGAM_RAMA_EXP_REGION9_NUM_SEGMENTS;                                                \
    type VPMPCC_OGAM_RAMA_EXP_REGION10_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION10_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION11_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION11_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION12_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION12_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION13_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION13_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION14_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION14_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION15_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION15_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION16_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION16_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION17_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION17_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION18_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION18_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION19_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION19_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION20_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION20_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION21_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION21_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION22_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION22_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION23_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION23_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION24_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION24_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION25_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION25_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION26_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION26_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION27_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION27_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION28_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION28_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION29_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION29_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION30_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION30_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION31_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION31_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION32_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION32_NUM_SEGMENTS;                                               \
    type VPMPCC_OGAM_RAMA_EXP_REGION33_LUT_OFFSET;                                                 \
    type VPMPCC_OGAM_RAMA_EXP_REGION33_NUM_SEGMENTS;                                               \
    type VPMPCC_GAMUT_REMAP_COEF_FORMAT;                                                           \
    type VPMPCC_GAMUT_REMAP_MODE;                                                                  \
    type VPMPCC_GAMUT_REMAP_MODE_CURRENT;                                                          \
    type VPMPCC_GAMUT_REMAP_C11_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C12_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C13_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C14_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C21_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C22_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C23_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C24_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C31_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C32_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C33_A;                                                                 \
    type VPMPCC_GAMUT_REMAP_C34_A;                                                                 \
    type VPMPCC_MCM_SHAPER_LUT_MODE;                                                               \
    type VPMPCC_MCM_SHAPER_MODE_CURRENT;                                                           \
    type VPMPCC_MCM_SHAPER_SELECT_CURRENT;                                                         \
    type VPMPCC_MCM_SHAPER_OFFSET_R;                                                               \
    type VPMPCC_MCM_SHAPER_OFFSET_G;                                                               \
    type VPMPCC_MCM_SHAPER_OFFSET_B;                                                               \
    type VPMPCC_MCM_SHAPER_SCALE_R;                                                                \
    type VPMPCC_MCM_SHAPER_SCALE_G;                                                                \
    type VPMPCC_MCM_SHAPER_SCALE_B;                                                                \
    type VPMPCC_MCM_SHAPER_LUT_INDEX;                                                              \
    type VPMPCC_MCM_SHAPER_LUT_DATA;                                                               \
    type VPMPCC_MCM_SHAPER_LUT_WRITE_EN_MASK;                                                      \
    type VPMPCC_MCM_SHAPER_LUT_WRITE_SEL;                                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_B;                                                \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_SEGMENT_B;                                        \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_G;                                                \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_SEGMENT_G;                                        \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_R;                                                \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_SEGMENT_R;                                        \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_B;                                                  \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_BASE_B;                                             \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_G;                                                  \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_BASE_G;                                             \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_R;                                                  \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_BASE_R;                                             \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION0_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION0_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION1_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION1_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION2_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION2_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION3_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION3_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION4_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION4_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION5_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION5_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION6_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION6_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION7_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION7_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION8_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION8_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION9_LUT_OFFSET;                                            \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION9_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION10_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION10_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION11_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION11_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION12_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION12_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION13_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION13_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION14_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION14_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION15_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION15_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION16_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION16_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION17_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION17_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION18_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION18_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION19_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION19_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION20_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION20_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION21_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION21_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION22_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION22_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION23_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION23_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION24_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION24_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION25_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION25_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION26_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION26_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION27_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION27_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION28_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION28_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION29_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION29_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION30_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION30_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION31_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION31_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION32_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION32_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION33_LUT_OFFSET;                                           \
    type VPMPCC_MCM_SHAPER_RAMA_EXP_REGION33_NUM_SEGMENTS;                                         \
    type VPMPCC_MCM_3DLUT_MODE;                                                                    \
    type VPMPCC_MCM_3DLUT_SIZE;                                                                    \
    type VPMPCC_MCM_3DLUT_MODE_CURRENT;                                                            \
    type VPMPCC_MCM_3DLUT_SELECT_CURRENT;                                                          \
    type VPMPCC_MCM_3DLUT_INDEX;                                                                   \
    type VPMPCC_MCM_3DLUT_DATA0;                                                                   \
    type VPMPCC_MCM_3DLUT_DATA1;                                                                   \
    type VPMPCC_MCM_3DLUT_DATA_30BIT;                                                              \
    type VPMPCC_MCM_3DLUT_WRITE_EN_MASK;                                                           \
    type VPMPCC_MCM_3DLUT_RAM_SEL;                                                                 \
    type VPMPCC_MCM_3DLUT_30BIT_EN;                                                                \
    type VPMPCC_MCM_3DLUT_READ_SEL;                                                                \
    type VPMPCC_MCM_3DLUT_OUT_NORM_FACTOR;                                                         \
    type VPMPCC_MCM_3DLUT_OUT_OFFSET_R;                                                            \
    type VPMPCC_MCM_3DLUT_OUT_SCALE_R;                                                             \
    type VPMPCC_MCM_3DLUT_OUT_OFFSET_G;                                                            \
    type VPMPCC_MCM_3DLUT_OUT_SCALE_G;                                                             \
    type VPMPCC_MCM_3DLUT_OUT_OFFSET_B;                                                            \
    type VPMPCC_MCM_3DLUT_OUT_SCALE_B;                                                             \
    type VPMPCC_MCM_1DLUT_MODE;                                                                    \
    type VPMPCC_MCM_1DLUT_PWL_DISABLE;                                                             \
    type VPMPCC_MCM_1DLUT_MODE_CURRENT;                                                            \
    type VPMPCC_MCM_1DLUT_SELECT_CURRENT;                                                          \
    type VPMPCC_MCM_1DLUT_LUT_INDEX;                                                               \
    type VPMPCC_MCM_1DLUT_LUT_DATA;                                                                \
    type VPMPCC_MCM_1DLUT_LUT_WRITE_COLOR_MASK;                                                    \
    type VPMPCC_MCM_1DLUT_LUT_READ_COLOR_SEL;                                                      \
    type VPMPCC_MCM_1DLUT_LUT_READ_DBG;                                                            \
    type VPMPCC_MCM_1DLUT_LUT_HOST_SEL;                                                            \
    type VPMPCC_MCM_1DLUT_LUT_CONFIG_MODE;                                                         \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_B;                                                 \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SEGMENT_B;                                         \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_G;                                                 \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SEGMENT_G;                                         \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_R;                                                 \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SEGMENT_R;                                         \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SLOPE_B;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SLOPE_G;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SLOPE_R;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_BASE_B;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_BASE_G;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_BASE_R;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_BASE_B;                                              \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_B;                                                   \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_SLOPE_B;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_BASE_G;                                              \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_G;                                                   \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_SLOPE_G;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_BASE_R;                                              \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_R;                                                   \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_SLOPE_R;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_OFFSET_B;                                                           \
    type VPMPCC_MCM_1DLUT_RAMA_OFFSET_G;                                                           \
    type VPMPCC_MCM_1DLUT_RAMA_OFFSET_R;                                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION0_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION0_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION1_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION1_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION2_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION2_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION3_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION3_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION4_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION4_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION5_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION5_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION6_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION6_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION7_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION7_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION8_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION8_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION9_LUT_OFFSET;                                             \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION9_NUM_SEGMENTS;                                           \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION10_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION10_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION11_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION11_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION12_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION12_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION13_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION13_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION14_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION14_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION15_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION15_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION16_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION16_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION17_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION17_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION18_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION18_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION19_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION19_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION20_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION20_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION21_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION21_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION22_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION22_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION23_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION23_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION24_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION24_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION25_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION25_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION26_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION26_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION27_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION27_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION28_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION28_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION29_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION29_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION30_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION30_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION31_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION31_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION32_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION32_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION33_LUT_OFFSET;                                            \
    type VPMPCC_MCM_1DLUT_RAMA_EXP_REGION33_NUM_SEGMENTS;                                          \
    type VPMPCC_MCM_SHAPER_MEM_PWR_FORCE;                                                          \
    type VPMPCC_MCM_SHAPER_MEM_PWR_DIS;                                                            \
    type VPMPCC_MCM_SHAPER_MEM_LOW_PWR_MODE;                                                       \
    type VPMPCC_MCM_3DLUT_MEM_PWR_FORCE;                                                           \
    type VPMPCC_MCM_3DLUT_MEM_PWR_DIS;                                                             \
    type VPMPCC_MCM_3DLUT_MEM_LOW_PWR_MODE;                                                        \
    type VPMPCC_MCM_1DLUT_MEM_PWR_FORCE;                                                           \
    type VPMPCC_MCM_1DLUT_MEM_PWR_DIS;                                                             \
    type VPMPCC_MCM_1DLUT_MEM_LOW_PWR_MODE;                                                        \
    type VPMPCC_MCM_SHAPER_MEM_PWR_STATE;                                                          \
    type VPMPCC_MCM_3DLUT_MEM_PWR_STATE;                                                           \
    type VPMPCC_MCM_1DLUT_MEM_PWR_STATE;

struct vpe10_mpc_registers {
    MPC_REG_VARIABLE_LIST_VPE10
};

struct vpe10_mpc_shift {
    MPC_FIELD_VARIABLE_LIST_VPE10(uint8_t)
};

struct vpe10_mpc_mask {
    MPC_FIELD_VARIABLE_LIST_VPE10(uint32_t)
};

struct vpe10_mpc {
    struct mpc                    base;
    struct vpe10_mpc_registers   *regs;
    const struct vpe10_mpc_shift *shift;
    const struct vpe10_mpc_mask  *mask;
};

void vpe10_construct_mpc(struct vpe_priv *vpe_priv, struct mpc *mpc);

void vpe10_mpc_program_mpcc_mux(struct mpc *mpc, enum mpc_mpccid mpcc_idx,
    enum mpc_mux_topsel topsel, enum mpc_mux_botsel botsel, enum mpc_mux_outmux outmux,
    enum mpc_mux_oppid oppid);

void vpe10_mpc_program_mpcc_blending(
    struct mpc *mpc, enum mpc_mpccid mpcc_idx, struct mpcc_blnd_cfg *blnd_cfg);

void vpe10_mpc_program_mpc_bypass_bg_color(struct mpc *mpc, struct mpcc_blnd_cfg *blnd_cfg);

void vpe10_mpc_power_on_ogam_lut(struct mpc *mpc, bool power_on);

void vpe10_mpc_set_output_csc(
    struct mpc *mpc, const uint16_t *regval, enum mpc_output_csc_mode ocsc_mode);

void vpe10_mpc_set_ocsc_default(struct mpc *mpc, enum vpe_surface_pixel_format pixel_format,
    enum color_space color_space, enum mpc_output_csc_mode ocsc_mode);

void vpe10_program_output_csc(struct mpc *mpc, enum vpe_surface_pixel_format pixel_format,
    enum color_space colorspace, uint16_t *matrix);

void vpe10_mpc_set_output_gamma(struct mpc *mpc, const struct pwl_params *params);

void vpe10_mpc_set_gamut_remap(struct mpc *mpc, struct colorspace_transform *gamut_remap);

void vpe10_mpc_power_on_1dlut_shaper_3dlut(struct mpc *mpc, bool power_on);

bool vpe10_mpc_program_shaper(struct mpc *mpc, const struct pwl_params *params);

// using direct config to program the 3dlut specified in params
void vpe10_mpc_program_3dlut(struct mpc *mpc, const struct tetrahedral_params *params);

// using indirect config to configure the 3DLut
// note that we still need direct config to switch the mask between lut0 - lut3
bool vpe10_mpc_program_3dlut_indirect(struct mpc *mpc,
    struct vpe_buf *lut0_3_buf, // 3d lut buf which contains the data for lut0-lut3
    bool use_tetrahedral_9, bool use_12bits);

// Blend-gamma control.
void vpe10_mpc_program_1dlut(struct mpc *mpc, const struct pwl_params *params);

void vpe10_mpc_program_cm_location(struct mpc *mpc, uint8_t location);

void vpe10_mpc_set_denorm(struct mpc *mpc, int opp_id, enum color_depth output_depth,
    struct mpc_denorm_clamp *denorm_clamp);

void vpe10_mpc_set_out_float_en(struct mpc *mpc, bool float_enable);

void vpe10_mpc_program_mpc_out(struct mpc *mpc, enum vpe_surface_pixel_format format);

void vpe10_mpc_set_output_transfer_func(struct mpc *mpc, struct output_ctx *output_ctx);

void vpe10_mpc_set_mpc_shaper_3dlut(
    struct mpc *mpc, const struct transfer_func *func_shaper, const struct vpe_3dlut *lut3d_func);

void vpe10_mpc_set_blend_lut(struct mpc *mpc, const struct transfer_func *blend_tf);

bool vpe10_mpc_program_movable_cm(struct mpc *mpc, const struct transfer_func *func_shaper,
    const struct vpe_3dlut *lut3d_func, const struct transfer_func *blend_tf, bool afterblend);

void vpe10_mpc_program_crc(struct mpc *mpc, bool enable);
#ifdef __cplusplus
}
#endif
