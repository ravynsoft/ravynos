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

#include "cdc.h"
#include "reg_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VPE10_CDC_VUPDATE_OFFSET_DEFAULT (21)
#define VPE10_CDC_VUPDATE_WIDTH_DEFAULT  (60)
#define VPE10_CDC_VREADY_OFFSET_DEFAULT  (150)

/* macros for filing variable or field list
   SRI, SFRB should be defined in the resource file */
#define CDC_REG_LIST_VPE10(id)                                                                     \
    SRIDFVL(VPEP_MGCG_CNTL, CDC, id), SRIDFVL(VPCDC_SOFT_RESET, CDC, id),                          \
        SRIDFVL(VPCDC_FE0_SURFACE_CONFIG, CDC, id), SRIDFVL(VPCDC_FE0_CROSSBAR_CONFIG, CDC, id),   \
        SRIDFVL(VPCDC_FE0_VIEWPORT_START_CONFIG, CDC, id),                                         \
        SRIDFVL(VPCDC_FE0_VIEWPORT_DIMENSION_CONFIG, CDC, id),                                     \
        SRIDFVL(VPCDC_FE0_VIEWPORT_START_C_CONFIG, CDC, id),                                       \
        SRIDFVL(VPCDC_FE0_VIEWPORT_DIMENSION_C_CONFIG, CDC, id),                                   \
        SRIDFVL(VPCDC_BE0_P2B_CONFIG, CDC, id), SRIDFVL(VPCDC_BE0_GLOBAL_SYNC_CONFIG, CDC, id),    \
        SRIDFVL(VPCDC_GLOBAL_SYNC_TRIGGER, CDC, id),                                               \
        SRIDFVL(VPEP_MEM_GLOBAL_PWR_REQ_CNTL, CDC, id), SRIDFVL(VPFE_MEM_PWR_CNTL, CDC, id),       \
        SRIDFVL(VPBE_MEM_PWR_CNTL, CDC, id)

#define CDC_FLIED_LIST_VPE10(post_fix)                                                             \
    SFRB(VPDPP0_CLK_GATE_DIS, VPEP_MGCG_CNTL, post_fix),                                           \
        SFRB(VPMPC_CLK_GATE_DIS, VPEP_MGCG_CNTL, post_fix),                                        \
        SFRB(VPOPP_CLK_GATE_DIS, VPEP_MGCG_CNTL, post_fix),                                        \
        SFRB(VPCDC_SOCCLK_G_GATE_DIS, VPEP_MGCG_CNTL, post_fix),                                   \
        SFRB(VPCDC_SOCCLK_R_GATE_DIS, VPEP_MGCG_CNTL, post_fix),                                   \
        SFRB(VPCDC_VPECLK_G_GATE_DIS, VPEP_MGCG_CNTL, post_fix),                                   \
        SFRB(VPCDC_VPECLK_R_GATE_DIS, VPEP_MGCG_CNTL, post_fix),                                   \
        SFRB(VPCDC_SOCCLK_SOFT_RESET, VPCDC_SOFT_RESET, post_fix),                                 \
        SFRB(VPCDC_VPECLK_SOFT_RESET, VPCDC_SOFT_RESET, post_fix),                                 \
        SFRB(SURFACE_PIXEL_FORMAT_FE0, VPCDC_FE0_SURFACE_CONFIG, post_fix),                        \
        SFRB(ROTATION_ANGLE_FE0, VPCDC_FE0_SURFACE_CONFIG, post_fix),                              \
        SFRB(H_MIRROR_EN_FE0, VPCDC_FE0_SURFACE_CONFIG, post_fix),                                 \
        SFRB(PIX_SURFACE_LINEAR_FE0, VPCDC_FE0_SURFACE_CONFIG, post_fix),                          \
        SFRB(CROSSBAR_SRC_ALPHA_FE0, VPCDC_FE0_CROSSBAR_CONFIG, post_fix),                         \
        SFRB(CROSSBAR_SRC_Y_G_FE0, VPCDC_FE0_CROSSBAR_CONFIG, post_fix),                           \
        SFRB(CROSSBAR_SRC_CB_B_FE0, VPCDC_FE0_CROSSBAR_CONFIG, post_fix),                          \
        SFRB(CROSSBAR_SRC_CR_R_FE0, VPCDC_FE0_CROSSBAR_CONFIG, post_fix),                          \
        SFRB(VIEWPORT_X_START_FE0, VPCDC_FE0_VIEWPORT_START_CONFIG, post_fix),                     \
        SFRB(VIEWPORT_Y_START_FE0, VPCDC_FE0_VIEWPORT_START_CONFIG, post_fix),                     \
        SFRB(VIEWPORT_WIDTH_FE0, VPCDC_FE0_VIEWPORT_DIMENSION_CONFIG, post_fix),                   \
        SFRB(VIEWPORT_HEIGHT_FE0, VPCDC_FE0_VIEWPORT_DIMENSION_CONFIG, post_fix),                  \
        SFRB(VIEWPORT_X_START_C_FE0, VPCDC_FE0_VIEWPORT_START_C_CONFIG, post_fix),                 \
        SFRB(VIEWPORT_Y_START_C_FE0, VPCDC_FE0_VIEWPORT_START_C_CONFIG, post_fix),                 \
        SFRB(VIEWPORT_WIDTH_C_FE0, VPCDC_FE0_VIEWPORT_DIMENSION_C_CONFIG, post_fix),               \
        SFRB(VIEWPORT_HEIGHT_C_FE0, VPCDC_FE0_VIEWPORT_DIMENSION_C_CONFIG, post_fix),              \
        SFRB(VPCDC_BE0_P2B_XBAR_SEL0, VPCDC_BE0_P2B_CONFIG, post_fix),                             \
        SFRB(VPCDC_BE0_P2B_XBAR_SEL1, VPCDC_BE0_P2B_CONFIG, post_fix),                             \
        SFRB(VPCDC_BE0_P2B_XBAR_SEL2, VPCDC_BE0_P2B_CONFIG, post_fix),                             \
        SFRB(VPCDC_BE0_P2B_XBAR_SEL3, VPCDC_BE0_P2B_CONFIG, post_fix),                             \
        SFRB(VPCDC_BE0_P2B_FORMAT_SEL, VPCDC_BE0_P2B_CONFIG, post_fix),                            \
        SFRB(BE0_VUPDATE_OFFSET, VPCDC_BE0_GLOBAL_SYNC_CONFIG, post_fix),                          \
        SFRB(BE0_VUPDATE_WIDTH, VPCDC_BE0_GLOBAL_SYNC_CONFIG, post_fix),                           \
        SFRB(BE0_VREADY_OFFSET, VPCDC_BE0_GLOBAL_SYNC_CONFIG, post_fix),                           \
        SFRB(VPBE_GS_TRIG, VPCDC_GLOBAL_SYNC_TRIGGER, post_fix),                                   \
        SFRB(VPFE_VR_STATUS, VPCDC_VREADY_STATUS, post_fix),                                       \
        SFRB(MEM_GLOBAL_PWR_REQ_DIS, VPEP_MEM_GLOBAL_PWR_REQ_CNTL, post_fix),                      \
        SFRB(VPFE0_MEM_PWR_FORCE, VPFE_MEM_PWR_CNTL, post_fix),                                    \
        SFRB(VPFE0_MEM_PWR_MODE, VPFE_MEM_PWR_CNTL, post_fix),                                     \
        SFRB(VPFE0_MEM_PWR_STATE, VPFE_MEM_PWR_CNTL, post_fix),                                    \
        SFRB(VPFE0_MEM_PWR_DIS, VPFE_MEM_PWR_CNTL, post_fix),                                      \
        SFRB(VPBE0_MEM_PWR_FORCE, VPBE_MEM_PWR_CNTL, post_fix),                                    \
        SFRB(VPBE0_MEM_PWR_MODE, VPBE_MEM_PWR_CNTL, post_fix),                                     \
        SFRB(VPBE0_MEM_PWR_STATE, VPBE_MEM_PWR_CNTL, post_fix),                                    \
        SFRB(VPBE0_MEM_PWR_DIS, VPBE_MEM_PWR_CNTL, post_fix)

/* define all structure register variables below */
#define CDC_REG_VARIABLE_LIST_VPE10                                                                \
    reg_id_val VPEP_MGCG_CNTL;                                                                     \
    reg_id_val VPCDC_SOFT_RESET;                                                                   \
    reg_id_val VPCDC_FE0_SURFACE_CONFIG;                                                           \
    reg_id_val VPCDC_FE0_CROSSBAR_CONFIG;                                                          \
    reg_id_val VPCDC_FE0_VIEWPORT_START_CONFIG;                                                    \
    reg_id_val VPCDC_FE0_VIEWPORT_DIMENSION_CONFIG;                                                \
    reg_id_val VPCDC_FE0_VIEWPORT_START_C_CONFIG;                                                  \
    reg_id_val VPCDC_FE0_VIEWPORT_DIMENSION_C_CONFIG;                                              \
    reg_id_val VPCDC_BE0_P2B_CONFIG;                                                               \
    reg_id_val VPCDC_BE0_GLOBAL_SYNC_CONFIG;                                                       \
    reg_id_val VPCDC_GLOBAL_SYNC_TRIGGER;                                                          \
    reg_id_val VPEP_MEM_GLOBAL_PWR_REQ_CNTL;                                                       \
    reg_id_val VPFE_MEM_PWR_CNTL;                                                                  \
    reg_id_val VPBE_MEM_PWR_CNTL;

#define CDC_FIELD_VARIABLE_LIST_VPE10(type)                                                        \
    type VPDPP0_CLK_GATE_DIS;                                                                      \
    type VPMPC_CLK_GATE_DIS;                                                                       \
    type VPOPP_CLK_GATE_DIS;                                                                       \
    type VPCDC_SOCCLK_G_GATE_DIS;                                                                  \
    type VPCDC_SOCCLK_R_GATE_DIS;                                                                  \
    type VPCDC_VPECLK_G_GATE_DIS;                                                                  \
    type VPCDC_VPECLK_R_GATE_DIS;                                                                  \
    type VPCDC_SOCCLK_SOFT_RESET;                                                                  \
    type VPCDC_VPECLK_SOFT_RESET;                                                                  \
    type SURFACE_PIXEL_FORMAT_FE0;                                                                 \
    type ROTATION_ANGLE_FE0;                                                                       \
    type H_MIRROR_EN_FE0;                                                                          \
    type PIX_SURFACE_LINEAR_FE0;                                                                   \
    type CROSSBAR_SRC_ALPHA_FE0;                                                                   \
    type CROSSBAR_SRC_Y_G_FE0;                                                                     \
    type CROSSBAR_SRC_CB_B_FE0;                                                                    \
    type CROSSBAR_SRC_CR_R_FE0;                                                                    \
    type VIEWPORT_X_START_FE0;                                                                     \
    type VIEWPORT_Y_START_FE0;                                                                     \
    type VIEWPORT_WIDTH_FE0;                                                                       \
    type VIEWPORT_HEIGHT_FE0;                                                                      \
    type VIEWPORT_X_START_C_FE0;                                                                   \
    type VIEWPORT_Y_START_C_FE0;                                                                   \
    type VIEWPORT_WIDTH_C_FE0;                                                                     \
    type VIEWPORT_HEIGHT_C_FE0;                                                                    \
    type VPCDC_BE0_P2B_XBAR_SEL0;                                                                  \
    type VPCDC_BE0_P2B_XBAR_SEL1;                                                                  \
    type VPCDC_BE0_P2B_XBAR_SEL2;                                                                  \
    type VPCDC_BE0_P2B_XBAR_SEL3;                                                                  \
    type VPCDC_BE0_P2B_FORMAT_SEL;                                                                 \
    type BE0_VUPDATE_OFFSET;                                                                       \
    type BE0_VUPDATE_WIDTH;                                                                        \
    type BE0_VREADY_OFFSET;                                                                        \
    type VPBE_GS_TRIG;                                                                             \
    type VPFE_VR_STATUS;                                                                           \
    type MEM_GLOBAL_PWR_REQ_DIS;                                                                   \
    type VPFE0_MEM_PWR_FORCE;                                                                      \
    type VPFE0_MEM_PWR_MODE;                                                                       \
    type VPFE0_MEM_PWR_STATE;                                                                      \
    type VPFE0_MEM_PWR_DIS;                                                                        \
    type VPBE0_MEM_PWR_FORCE;                                                                      \
    type VPBE0_MEM_PWR_MODE;                                                                       \
    type VPBE0_MEM_PWR_STATE;                                                                      \
    type VPBE0_MEM_PWR_DIS;

struct vpe10_cdc_registers {
    CDC_REG_VARIABLE_LIST_VPE10
};

struct vpe10_cdc_shift {
    CDC_FIELD_VARIABLE_LIST_VPE10(uint8_t)
};

struct vpe10_cdc_mask {
    CDC_FIELD_VARIABLE_LIST_VPE10(uint32_t)
};

struct vpe10_cdc {
    struct cdc                    base; // base class, must be the first field
    struct vpe10_cdc_registers   *regs;
    const struct vpe10_cdc_shift *shift;
    const struct vpe10_cdc_mask  *mask;
};

void vpe10_construct_cdc(struct vpe_priv *vpe_priv, struct cdc *cdc);

bool vpe10_cdc_check_input_format(struct cdc *cdc, enum vpe_surface_pixel_format format);

bool vpe10_cdc_check_output_format(struct cdc *cdc, enum vpe_surface_pixel_format format);

void vpe10_cdc_program_surface_config(struct cdc *cdc, enum vpe_surface_pixel_format format,
    enum vpe_rotation_angle rotation, bool horizontal_mirror, enum vpe_swizzle_mode_values swizzle);

void vpe10_cdc_program_crossbar_config(struct cdc *cdc, enum vpe_surface_pixel_format format);

void vpe10_cdc_program_global_sync(
    struct cdc *cdc, uint32_t vupdate_offset, uint32_t vupdate_width, uint32_t vready_offset);

void vpe10_cdc_program_p2b_config(struct cdc *cdc, enum vpe_surface_pixel_format format);

/***** segment register programming *****/
void vpe10_cdc_program_viewport(
    struct cdc *cdc, const struct vpe_rect *viewport, const struct vpe_rect *viewport_c);

#ifdef __cplusplus
}
#endif
