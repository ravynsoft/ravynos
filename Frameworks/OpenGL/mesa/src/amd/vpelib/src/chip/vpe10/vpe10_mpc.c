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
#include "vpe_assert.h"
#include "common.h"
#include "vpe_priv.h"
#include "vpe10_mpc.h"
#include "reg_helper.h"
#include "vpe10_cm_common.h"
#include "fixed31_32.h"
#include "conversion.h"
#include "color_pwl.h"

#define CTX_BASE mpc
#define CTX      vpe10_mpc

static struct mpc_funcs mpc_funcs = {
    .program_mpcc_mux            = vpe10_mpc_program_mpcc_mux,
    .program_mpcc_blending       = vpe10_mpc_program_mpcc_blending,
    .program_mpc_bypass_bg_color = vpe10_mpc_program_mpc_bypass_bg_color,
    .power_on_ogam_lut           = vpe10_mpc_power_on_ogam_lut,
    .set_output_csc              = vpe10_mpc_set_output_csc,
    .set_ocsc_default            = vpe10_mpc_set_ocsc_default,
    .program_output_csc          = vpe10_program_output_csc,
    .set_output_gamma            = vpe10_mpc_set_output_gamma,
    .set_gamut_remap             = vpe10_mpc_set_gamut_remap,
    .power_on_1dlut_shaper_3dlut = vpe10_mpc_power_on_1dlut_shaper_3dlut,
    .program_shaper              = vpe10_mpc_program_shaper,
    .program_3dlut               = vpe10_mpc_program_3dlut,
    .program_3dlut_indirect      = vpe10_mpc_program_3dlut_indirect,
    .program_1dlut               = vpe10_mpc_program_1dlut,
    .program_cm_location         = vpe10_mpc_program_cm_location,
    .set_denorm                  = vpe10_mpc_set_denorm,
    .set_out_float_en            = vpe10_mpc_set_out_float_en,
    .program_mpc_out             = vpe10_mpc_program_mpc_out,
    .set_output_transfer_func    = vpe10_mpc_set_output_transfer_func,
    .set_mpc_shaper_3dlut        = vpe10_mpc_set_mpc_shaper_3dlut,
    .set_blend_lut               = vpe10_mpc_set_blend_lut,
    .program_movable_cm          = vpe10_mpc_program_movable_cm,
    .program_crc                 = vpe10_mpc_program_crc,
};

void vpe10_construct_mpc(struct vpe_priv *vpe_priv, struct mpc *mpc)
{
    mpc->vpe_priv = vpe_priv;
    mpc->funcs    = &mpc_funcs;
}

void vpe10_mpc_program_mpcc_mux(struct mpc *mpc, enum mpc_mpccid mpcc_idx,
    enum mpc_mux_topsel topsel, enum mpc_mux_botsel botsel, enum mpc_mux_outmux outmux,
    enum mpc_mux_oppid oppid)
{
    PROGRAM_ENTRY();

    VPE_ASSERT(mpcc_idx == MPC_MPCCID_0);

    REG_SET(VPMPCC_TOP_SEL, 0, VPMPCC_TOP_SEL, topsel);
    REG_SET(VPMPCC_BOT_SEL, 0, VPMPCC_BOT_SEL, botsel);
    REG_SET(VPMPC_OUT0_MUX, 0, VPMPC_OUT_MUX, outmux);
    REG_SET(VPMPCC_VPOPP_ID, 0, VPMPCC_VPOPP_ID, oppid);

    /* program mux and MPCC_MODE */
    if (mpc->vpe_priv->init.debug.mpc_bypass) {
        REG_UPDATE(VPMPCC_CONTROL, VPMPCC_MODE, MPCC_BLEND_MODE_BYPASS);
    } else if (botsel != MPC_MUX_BOTSEL_DISABLE) {
        // ERROR: Actually VPE10 only supports 1 MPCC so botsel should always disable
        VPE_ASSERT(0);
        REG_UPDATE(VPMPCC_CONTROL, VPMPCC_MODE, MPCC_BLEND_MODE_TOP_BOT_BLENDING);
    } else {
        // single layer, use Top layer bleneded with background color
        if (topsel != MPC_MUX_TOPSEL_DISABLE)
            REG_UPDATE(VPMPCC_CONTROL, VPMPCC_MODE, MPCC_BLEND_MODE_TOP_LAYER_ONLY);
        else // both layer disabled, pure bypass mode
            REG_UPDATE(VPMPCC_CONTROL, VPMPCC_MODE, MPCC_BLEND_MODE_BYPASS);
    }
}

void vpe10_mpc_program_mpcc_blending(
    struct mpc *mpc, enum mpc_mpccid mpcc_idx, struct mpcc_blnd_cfg *blnd_cfg)
{
    PROGRAM_ENTRY();
    float    r_cr, g_y, b_cb;
    uint32_t bg_r_cr, bg_g_y, bg_b_cb;
    uint32_t factor;

    VPE_ASSERT(mpcc_idx == MPC_MPCCID_0);

    REG_UPDATE_7(VPMPCC_CONTROL, VPMPCC_ALPHA_BLND_MODE, blnd_cfg->alpha_mode,
        VPMPCC_ALPHA_MULTIPLIED_MODE, blnd_cfg->pre_multiplied_alpha,
        VPMPCC_BLND_ACTIVE_OVERLAP_ONLY, blnd_cfg->overlap_only, VPMPCC_GLOBAL_ALPHA,
        blnd_cfg->global_alpha, VPMPCC_GLOBAL_GAIN, blnd_cfg->global_gain, VPMPCC_BG_BPC,
        blnd_cfg->background_color_bpc, VPMPCC_BOT_GAIN_MODE, blnd_cfg->bottom_gain_mode);

    REG_SET(VPMPCC_TOP_GAIN, 0, VPMPCC_TOP_GAIN, blnd_cfg->top_gain);
    REG_SET(VPMPCC_BOT_GAIN_INSIDE, 0, VPMPCC_BOT_GAIN_INSIDE, blnd_cfg->bottom_inside_gain);
    REG_SET(VPMPCC_BOT_GAIN_OUTSIDE, 0, VPMPCC_BOT_GAIN_OUTSIDE, blnd_cfg->bottom_outside_gain);

    if (blnd_cfg->bg_color.is_ycbcr) {
        r_cr = blnd_cfg->bg_color.ycbcra.cr;
        g_y  = blnd_cfg->bg_color.ycbcra.y;
        b_cb = blnd_cfg->bg_color.ycbcra.cb;
    } else {
        r_cr = blnd_cfg->bg_color.rgba.r;
        g_y  = blnd_cfg->bg_color.rgba.g;
        b_cb = blnd_cfg->bg_color.rgba.b;
    }

    switch (blnd_cfg->background_color_bpc) {
    case 0: // 8bit
        factor = 0xff;
        break;
    case 1: // 9bit
        factor = 0x1ff;
        break;
    case 2: // 10bit
        factor = 0x3ff;
        break;
    case 3: // 11bit
        factor = 0x7ff;
        break;
    case 4: // 12bit
    default:
        factor = 0xfff;
        break;
    }
    bg_r_cr = (uint32_t)(r_cr * (float)factor);
    bg_b_cb = (uint32_t)(b_cb * (float)factor);
    bg_g_y  = (uint32_t)(g_y * (float)factor);

    // Set background color
    REG_SET(VPMPCC_BG_R_CR, 0, VPMPCC_BG_R_CR, bg_r_cr);
    REG_SET(VPMPCC_BG_G_Y, 0, VPMPCC_BG_G_Y, bg_g_y);
    REG_SET(VPMPCC_BG_B_CB, 0, VPMPCC_BG_B_CB, bg_b_cb);
}

void vpe10_mpc_program_mpc_bypass_bg_color(struct mpc *mpc, struct mpcc_blnd_cfg *blnd_cfg)
{
    PROGRAM_ENTRY();
    float    r_cr, g_y, b_cb, alpha;
    uint32_t bg_r_cr, bg_g_y, bg_b_cb, bg_alpha;

    if (blnd_cfg->bg_color.is_ycbcr) {
        r_cr  = blnd_cfg->bg_color.ycbcra.cr;
        g_y   = blnd_cfg->bg_color.ycbcra.y;
        b_cb  = blnd_cfg->bg_color.ycbcra.cb;
        alpha = blnd_cfg->bg_color.ycbcra.a;
    } else {
        r_cr  = blnd_cfg->bg_color.rgba.r;
        g_y   = blnd_cfg->bg_color.rgba.g;
        b_cb  = blnd_cfg->bg_color.rgba.b;
        alpha = blnd_cfg->bg_color.rgba.a;
    }

    bg_r_cr  = (uint32_t)(r_cr * 0xffff);
    bg_g_y   = (uint32_t)(g_y * 0xffff);
    bg_b_cb  = (uint32_t)(b_cb * 0xffff);
    bg_alpha = (uint32_t)(alpha * 0xffff);

    // Set background color
    REG_SET(VPMPC_BYPASS_BG_AR, 0, VPMPC_BYPASS_BG_ALPHA, bg_alpha);
    REG_SET(VPMPC_BYPASS_BG_AR, 0, VPMPC_BYPASS_BG_R_CR, bg_r_cr);
    REG_SET(VPMPC_BYPASS_BG_GB, 0, VPMPC_BYPASS_BG_G_Y, bg_g_y);
    REG_SET(VPMPC_BYPASS_BG_GB, 0, VPMPC_BYPASS_BG_B_CB, bg_b_cb);
}

void vpe10_mpc_power_on_ogam_lut(struct mpc *mpc, bool power_on)
{
    PROGRAM_ENTRY();

    /*
     * Powering on: force memory active so the LUT can be updated.
     * Powering off: allow entering memory low power mode
     *
     * Memory low power mode is controlled during MPC OGAM LUT init.
     */
    REG_UPDATE(VPMPCC_MEM_PWR_CTRL, VPMPCC_OGAM_MEM_PWR_DIS, power_on ? 1 : 0);

    /* Wait for memory to be powered on - we won't be able to write to it otherwise. */
    if (power_on) {
        // dummy write as delay in power up
        REG_UPDATE(VPMPCC_MEM_PWR_CTRL, VPMPCC_OGAM_MEM_PWR_DIS, power_on ? 1 : 0);
        REG_UPDATE(VPMPCC_MEM_PWR_CTRL, VPMPCC_OGAM_MEM_PWR_DIS, power_on ? 1 : 0);
    }
}

void vpe10_mpc_set_output_csc(
    struct mpc *mpc, const uint16_t *regval, enum mpc_output_csc_mode ocsc_mode)
{
    PROGRAM_ENTRY();
    struct color_matrices_reg ocsc_regs;

    REG_SET(VPMPC_OUT_CSC_COEF_FORMAT, 0, VPMPC_OCSC0_COEF_FORMAT, 0);
    REG_SET(VPMPC_OUT0_CSC_MODE, 0, VPMPC_OCSC_MODE, ocsc_mode);

    if (ocsc_mode == MPC_OUTPUT_CSC_DISABLE)
        return;

    if (regval == NULL)
        return;

    ocsc_regs.shifts.csc_c11 = REG_FIELD_SHIFT(VPMPC_OCSC_C11_A);
    ocsc_regs.masks.csc_c11  = REG_FIELD_MASK(VPMPC_OCSC_C11_A);
    ocsc_regs.shifts.csc_c12 = REG_FIELD_SHIFT(VPMPC_OCSC_C12_A);
    ocsc_regs.masks.csc_c12  = REG_FIELD_MASK(VPMPC_OCSC_C12_A);

    if (ocsc_mode == MPC_OUTPUT_CSC_COEF_A) {
        ocsc_regs.csc_c11_c12 = REG_OFFSET(VPMPC_OUT0_CSC_C11_C12_A);
        ocsc_regs.csc_c33_c34 = REG_OFFSET(VPMPC_OUT0_CSC_C33_C34_A);
    } else {
        VPE_ASSERT(0);
        return;
    }

    vpe10_cm_helper_program_color_matrices(config_writer, regval, &ocsc_regs);
}

void vpe10_mpc_set_ocsc_default(struct mpc *mpc, enum vpe_surface_pixel_format pixel_format,
    enum color_space color_space, enum mpc_output_csc_mode ocsc_mode)
{
    PROGRAM_ENTRY();
    struct color_matrices_reg ocsc_regs;
    uint32_t                  arr_size;
    const uint16_t           *regval = NULL;

    REG_SET(VPMPC_OUT_CSC_COEF_FORMAT, 0, VPMPC_OCSC0_COEF_FORMAT, 0);
    REG_SET(VPMPC_OUT0_CSC_MODE, 0, VPMPC_OCSC_MODE, ocsc_mode);

    if (ocsc_mode == MPC_OUTPUT_CSC_DISABLE)
        return;

    regval = vpe_find_color_matrix(color_space, pixel_format, &arr_size);
    if (regval == NULL)
        return;

    ocsc_regs.shifts.csc_c11 = REG_FIELD_SHIFT(VPMPC_OCSC_C11_A);
    ocsc_regs.masks.csc_c11  = REG_FIELD_MASK(VPMPC_OCSC_C11_A);
    ocsc_regs.shifts.csc_c12 = REG_FIELD_SHIFT(VPMPC_OCSC_C12_A);
    ocsc_regs.masks.csc_c12  = REG_FIELD_MASK(VPMPC_OCSC_C12_A);

    if (ocsc_mode == MPC_OUTPUT_CSC_COEF_A) {
        ocsc_regs.csc_c11_c12 = REG_OFFSET(VPMPC_OUT0_CSC_C11_C12_A);
        ocsc_regs.csc_c33_c34 = REG_OFFSET(VPMPC_OUT0_CSC_C33_C34_A);
    } else {
        VPE_ASSERT(0);
        return;
    }

    vpe10_cm_helper_program_color_matrices(config_writer, regval, &ocsc_regs);
}

void vpe10_program_output_csc(struct mpc *mpc, enum vpe_surface_pixel_format pixel_format,
    enum color_space colorspace, uint16_t *matrix)
{
    PROGRAM_ENTRY();

    enum mpc_output_csc_mode ocsc_mode = MPC_OUTPUT_CSC_COEF_A;

    if (mpc->funcs->power_on_ogam_lut)
        mpc->funcs->power_on_ogam_lut(mpc, true);

    if (matrix != NULL) {
        if (mpc->funcs->set_output_csc != NULL)
            mpc->funcs->set_output_csc(mpc, matrix, ocsc_mode);
    } else {
        if (mpc->funcs->set_ocsc_default != NULL)
            mpc->funcs->set_ocsc_default(mpc, pixel_format, colorspace, ocsc_mode);
    }
}

enum vpmpcc_ogam_mode {
    VPMPCC_OGAM_DISABLE,
    VPMPCC_OGAM_RESERVED1,
    VPMPCC_OGAM_RAMLUT,
    VPMPCC_OGAM_RESERVED2
};

enum mpcc_ogam_lut_host_sel {
    RAM_LUT_A,
    //    RAM_LUT_B,
};

static void vpe10_mpc_ogam_get_reg_field(struct mpc *mpc, struct vpe10_xfer_func_reg *reg)
{
    struct vpe10_mpc *vpe10_mpc = (struct vpe10_mpc *)mpc;

    reg->shifts.field_region_start_base =
        vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION_START_BASE_B;
    reg->masks.field_region_start_base = vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION_START_BASE_B;
    reg->shifts.field_offset           = vpe10_mpc->shift->VPMPCC_OGAM_RAMA_OFFSET_B;
    reg->masks.field_offset            = vpe10_mpc->mask->VPMPCC_OGAM_RAMA_OFFSET_B;

    reg->shifts.exp_region0_lut_offset = vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION0_LUT_OFFSET;
    reg->masks.exp_region0_lut_offset  = vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION0_LUT_OFFSET;
    reg->shifts.exp_region0_num_segments =
        vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION0_NUM_SEGMENTS;
    reg->masks.exp_region0_num_segments =
        vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION0_NUM_SEGMENTS;
    reg->shifts.exp_region1_lut_offset = vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION1_LUT_OFFSET;
    reg->masks.exp_region1_lut_offset  = vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION1_LUT_OFFSET;
    reg->shifts.exp_region1_num_segments =
        vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION1_NUM_SEGMENTS;
    reg->masks.exp_region1_num_segments =
        vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION1_NUM_SEGMENTS;

    reg->shifts.field_region_end       = vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION_END_B;
    reg->masks.field_region_end        = vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION_END_B;
    reg->shifts.field_region_end_slope = vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION_END_SLOPE_B;
    reg->masks.field_region_end_slope  = vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION_END_SLOPE_B;
    reg->shifts.field_region_end_base  = vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION_END_BASE_B;
    reg->masks.field_region_end_base   = vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION_END_BASE_B;
    reg->shifts.field_region_linear_slope =
        vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION_START_SLOPE_B;
    reg->masks.field_region_linear_slope =
        vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION_START_SLOPE_B;
    reg->shifts.exp_region_start = vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION_START_B;
    reg->masks.exp_region_start  = vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION_START_B;
    reg->shifts.exp_region_start_segment =
        vpe10_mpc->shift->VPMPCC_OGAM_RAMA_EXP_REGION_START_SEGMENT_B;
    reg->masks.exp_region_start_segment =
        vpe10_mpc->mask->VPMPCC_OGAM_RAMA_EXP_REGION_START_SEGMENT_B;
}

static void vpe10_mpc_program_luta(struct mpc *mpc, const struct pwl_params *params)
{
    PROGRAM_ENTRY();

    struct vpe10_xfer_func_reg gam_regs;

    vpe10_mpc_ogam_get_reg_field(mpc, &gam_regs);

    gam_regs.start_cntl_b       = REG_OFFSET(VPMPCC_OGAM_RAMA_START_CNTL_B);
    gam_regs.start_cntl_g       = REG_OFFSET(VPMPCC_OGAM_RAMA_START_CNTL_G);
    gam_regs.start_cntl_r       = REG_OFFSET(VPMPCC_OGAM_RAMA_START_CNTL_R);
    gam_regs.start_slope_cntl_b = REG_OFFSET(VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_B);
    gam_regs.start_slope_cntl_g = REG_OFFSET(VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_G);
    gam_regs.start_slope_cntl_r = REG_OFFSET(VPMPCC_OGAM_RAMA_START_SLOPE_CNTL_R);
    gam_regs.start_end_cntl1_b  = REG_OFFSET(VPMPCC_OGAM_RAMA_END_CNTL1_B);
    gam_regs.start_end_cntl2_b  = REG_OFFSET(VPMPCC_OGAM_RAMA_END_CNTL2_B);
    gam_regs.start_end_cntl1_g  = REG_OFFSET(VPMPCC_OGAM_RAMA_END_CNTL1_G);
    gam_regs.start_end_cntl2_g  = REG_OFFSET(VPMPCC_OGAM_RAMA_END_CNTL2_G);
    gam_regs.start_end_cntl1_r  = REG_OFFSET(VPMPCC_OGAM_RAMA_END_CNTL1_R);
    gam_regs.start_end_cntl2_r  = REG_OFFSET(VPMPCC_OGAM_RAMA_END_CNTL2_R);
    gam_regs.region_start       = REG_OFFSET(VPMPCC_OGAM_RAMA_REGION_0_1);
    gam_regs.region_end         = REG_OFFSET(VPMPCC_OGAM_RAMA_REGION_32_33);
    gam_regs.offset_b           = REG_OFFSET(VPMPCC_OGAM_RAMA_OFFSET_B);
    gam_regs.offset_g           = REG_OFFSET(VPMPCC_OGAM_RAMA_OFFSET_G);
    gam_regs.offset_r           = REG_OFFSET(VPMPCC_OGAM_RAMA_OFFSET_R);
    gam_regs.start_base_cntl_b  = REG_OFFSET(VPMPCC_OGAM_RAMA_START_BASE_CNTL_B);
    gam_regs.start_base_cntl_g  = REG_OFFSET(VPMPCC_OGAM_RAMA_START_BASE_CNTL_G);
    gam_regs.start_base_cntl_r  = REG_OFFSET(VPMPCC_OGAM_RAMA_START_BASE_CNTL_R);

    vpe10_cm_helper_program_gamcor_xfer_func(config_writer, params, &gam_regs);
}

static void vpe10_mpc_program_ogam_pwl(
    struct mpc *mpc, const struct pwl_result_data *rgb, uint32_t num)
{
    PROGRAM_ENTRY();

    uint32_t last_base_value_red   = rgb[num - 1].red_reg + rgb[num - 1].delta_red_reg;
    uint32_t last_base_value_green = rgb[num - 1].green_reg + rgb[num - 1].delta_green_reg;
    uint32_t last_base_value_blue  = rgb[num - 1].blue_reg + rgb[num - 1].delta_blue_reg;

    if (vpe_is_rgb_equal(rgb, num)) {
        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_red, num,
            REG_OFFSET(VPMPCC_OGAM_LUT_DATA), REG_FIELD_SHIFT(VPMPCC_OGAM_LUT_DATA),
            REG_FIELD_MASK(VPMPCC_OGAM_LUT_DATA), CM_PWL_R);
    } else {
        REG_UPDATE(VPMPCC_OGAM_LUT_CONTROL, VPMPCC_OGAM_LUT_WRITE_COLOR_MASK, 4);

        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_red, num,
            REG_OFFSET(VPMPCC_OGAM_LUT_DATA), REG_FIELD_SHIFT(VPMPCC_OGAM_LUT_DATA),
            REG_FIELD_MASK(VPMPCC_OGAM_LUT_DATA), CM_PWL_R);

        REG_SET(VPMPCC_OGAM_LUT_INDEX, 0, VPMPCC_OGAM_LUT_INDEX, 0);
        REG_UPDATE(VPMPCC_OGAM_LUT_CONTROL, VPMPCC_OGAM_LUT_WRITE_COLOR_MASK, 2);

        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_green, num,
            REG_OFFSET(VPMPCC_OGAM_LUT_DATA), REG_FIELD_SHIFT(VPMPCC_OGAM_LUT_DATA),
            REG_FIELD_MASK(VPMPCC_OGAM_LUT_DATA), CM_PWL_G);

        REG_SET(VPMPCC_OGAM_LUT_INDEX, 0, VPMPCC_OGAM_LUT_INDEX, 0);
        REG_UPDATE(VPMPCC_OGAM_LUT_CONTROL, VPMPCC_OGAM_LUT_WRITE_COLOR_MASK, 1);

        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_blue, num,
            REG_OFFSET(VPMPCC_OGAM_LUT_DATA), REG_FIELD_SHIFT(VPMPCC_OGAM_LUT_DATA),
            REG_FIELD_MASK(VPMPCC_OGAM_LUT_DATA), CM_PWL_B);
    }
}

void vpe10_mpc_set_output_gamma(struct mpc *mpc, const struct pwl_params *params)
{
    PROGRAM_ENTRY();

    if (vpe_priv->init.debug.cm_in_bypass ||                  // debug option: put CM in bypass mode
        vpe_priv->init.debug.bypass_ogam || params == NULL) { // disable OGAM
        REG_SET(VPMPCC_OGAM_CONTROL, 0, VPMPCC_OGAM_MODE, VPMPCC_OGAM_DISABLE);
        return;
    }

    // enable OGAM RAM LUT mode/Enable PWL
    REG_SET_2(VPMPCC_OGAM_CONTROL, REG_DEFAULT(VPMPCC_OGAM_CONTROL), VPMPCC_OGAM_MODE,
        VPMPCC_OGAM_RAMLUT, VPMPCC_OGAM_PWL_DISABLE, 0);

    mpc->funcs->power_on_ogam_lut(mpc, true);

    // configure_ogam_lut as LUT_A and all RGB channels to be written
    REG_SET_2(VPMPCC_OGAM_LUT_CONTROL,
        0, // disable READ_DBG, set CONFIG_MODE to diff start/end mode implicitly
        VPMPCC_OGAM_LUT_WRITE_COLOR_MASK, 7, VPMPCC_OGAM_LUT_HOST_SEL, RAM_LUT_A);

    REG_SET(VPMPCC_OGAM_LUT_INDEX, 0, VPMPCC_OGAM_LUT_INDEX, 0);

    // Always program LUTA in VPE10
    vpe10_mpc_program_luta(mpc, params);

    vpe10_mpc_program_ogam_pwl(mpc, params->rgb_resulted, params->hw_points_num);

    // Assume we prefer to enable_mem_low_power
    if (vpe_priv->init.debug.enable_mem_low_power.bits.mpc)
        mpc->funcs->power_on_ogam_lut(mpc, false);
}

static void vpe10_program_gamut_remap(
    struct mpc *mpc, const uint16_t *regval, enum gamut_remap_select select)
{
    uint16_t                  selection = 0;
    struct color_matrices_reg gam_regs;
    PROGRAM_ENTRY();

    if (regval == NULL || select == GAMUT_REMAP_BYPASS) {
        REG_SET(VPMPCC_GAMUT_REMAP_MODE, 0, VPMPCC_GAMUT_REMAP_MODE, GAMUT_REMAP_BYPASS);
        return;
    }

    gam_regs.shifts.csc_c11 = REG_FIELD_SHIFT(VPMPCC_GAMUT_REMAP_C11_A);
    gam_regs.masks.csc_c11  = REG_FIELD_MASK(VPMPCC_GAMUT_REMAP_C11_A);
    gam_regs.shifts.csc_c12 = REG_FIELD_SHIFT(VPMPCC_GAMUT_REMAP_C12_A);
    gam_regs.masks.csc_c12  = REG_FIELD_MASK(VPMPCC_GAMUT_REMAP_C12_A);

    gam_regs.csc_c11_c12 = REG_OFFSET(VPMPC_GAMUT_REMAP_C11_C12_A);
    gam_regs.csc_c33_c34 = REG_OFFSET(VPMPC_GAMUT_REMAP_C33_C34_A);

    vpe10_cm_helper_program_color_matrices(config_writer, regval, &gam_regs);

    // select coefficient set to use
    REG_SET(VPMPCC_GAMUT_REMAP_MODE, 0, VPMPCC_GAMUT_REMAP_MODE, GAMUT_REMAP_COMA_COEFF);
}

void vpe10_mpc_set_gamut_remap(struct mpc *mpc, struct colorspace_transform *gamut_remap)
{
    uint16_t arr_reg_val[12];
    PROGRAM_ENTRY();
    int i = 0;

    if (!gamut_remap || !gamut_remap->enable_remap)
        vpe10_program_gamut_remap(mpc, NULL, GAMUT_REMAP_BYPASS);
    else {
        conv_convert_float_matrix(arr_reg_val, gamut_remap->matrix, 12);

        vpe10_program_gamut_remap(mpc, arr_reg_val, GAMUT_REMAP_COMA_COEFF);
    }
}

static void vpe10_mpc_configure_shaper_lut(struct mpc *mpc, bool is_ram_a)
{
    PROGRAM_ENTRY();

    REG_SET_2(VPMPCC_MCM_SHAPER_LUT_WRITE_EN_MASK, 0, VPMPCC_MCM_SHAPER_LUT_WRITE_EN_MASK, 7,
        VPMPCC_MCM_SHAPER_LUT_WRITE_SEL, is_ram_a == true ? 0 : 1);

    REG_SET(VPMPCC_MCM_SHAPER_LUT_INDEX, 0, VPMPCC_MCM_SHAPER_LUT_INDEX, 0);
}

static void vpe10_mpc_program_shaper_luta_settings(struct mpc *mpc, const struct pwl_params *params)
{
    PROGRAM_ENTRY();
    const struct gamma_curve *curve;
    uint16_t                  packet_data_size;
    uint16_t                  i;

    REG_SET_2(VPMPCC_MCM_SHAPER_RAMA_START_CNTL_B, 0, VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_B,
        params->corner_points[0].blue.custom_float_x,
        VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_SEGMENT_B, 0);
    REG_SET_2(VPMPCC_MCM_SHAPER_RAMA_START_CNTL_G, 0, VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_B,
        params->corner_points[0].green.custom_float_x,
        VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_SEGMENT_B, 0);
    REG_SET_2(VPMPCC_MCM_SHAPER_RAMA_START_CNTL_R, 0, VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_B,
        params->corner_points[0].red.custom_float_x,
        VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_START_SEGMENT_B, 0);

    REG_SET_2(VPMPCC_MCM_SHAPER_RAMA_END_CNTL_B, 0, VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_B,
        params->corner_points[1].blue.custom_float_x, VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_BASE_B,
        params->corner_points[1].blue.custom_float_y);
    REG_SET_2(VPMPCC_MCM_SHAPER_RAMA_END_CNTL_G, 0, VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_B,
        params->corner_points[1].green.custom_float_x, VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_BASE_B,
        params->corner_points[1].green.custom_float_y);
    REG_SET_2(VPMPCC_MCM_SHAPER_RAMA_END_CNTL_R, 0, VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_B,
        params->corner_points[1].red.custom_float_x, VPMPCC_MCM_SHAPER_RAMA_EXP_REGION_END_BASE_B,
        params->corner_points[1].red.custom_float_y);

    // Optimized by single VPEP config packet with auto inc

    packet_data_size = (uint16_t)(REG_OFFSET(VPMPCC_MCM_SHAPER_RAMA_REGION_32_33) -
                                  REG_OFFSET(VPMPCC_MCM_SHAPER_RAMA_REGION_0_1) + 1);

    VPE_ASSERT(packet_data_size <= MAX_CONFIG_PACKET_DATA_SIZE_DWORD);
    packet.bits.INC = 1;      // set the auto increase bit
    packet.bits.VPEP_CONFIG_DATA_SIZE =
        packet_data_size - 1; // number of "continuous" dwords, 1-based
    packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(VPMPCC_MCM_SHAPER_RAMA_REGION_0_1);

    config_writer_fill_direct_config_packet_header(config_writer, &packet);

    curve = params->arr_curve_points;

    for (i = 0; i < packet_data_size; i++) {
        config_writer_fill(config_writer,
            REG_FIELD_VALUE(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION0_LUT_OFFSET, curve[0].offset) |
                REG_FIELD_VALUE(
                    VPMPCC_MCM_SHAPER_RAMA_EXP_REGION0_NUM_SEGMENTS, curve[0].segments_num) |
                REG_FIELD_VALUE(VPMPCC_MCM_SHAPER_RAMA_EXP_REGION1_LUT_OFFSET, curve[1].offset) |
                REG_FIELD_VALUE(
                    VPMPCC_MCM_SHAPER_RAMA_EXP_REGION1_NUM_SEGMENTS, curve[1].segments_num));
        curve += 2;
    }
}

static void vpe10_mpc_program_shaper_lut(
    struct mpc *mpc, const struct pwl_result_data *rgb, uint32_t num)
{
    PROGRAM_ENTRY();
    uint32_t i, red, green, blue;
    uint32_t red_delta, green_delta, blue_delta;
    uint32_t red_value, green_value, blue_value;
    uint16_t packet_data_size;

    // Optimized by single VPEP config packet for same address with multiple write
    packet_data_size = (uint16_t)num * 3; // num writes for each channel in (R, G, B)

    VPE_ASSERT(packet_data_size <= MAX_CONFIG_PACKET_DATA_SIZE_DWORD);
    packet.bits.INC = 0;
    packet.bits.VPEP_CONFIG_DATA_SIZE =
        packet_data_size - 1; // number of "continuous" dwords, 1-based
    packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(VPMPCC_MCM_SHAPER_LUT_DATA);

    config_writer_fill_direct_config_packet_header(config_writer, &packet);

    for (i = 0; i < num; i++) {

        red   = rgb[i].red_reg;
        green = rgb[i].green_reg;
        blue  = rgb[i].blue_reg;

        red_delta   = rgb[i].delta_red_reg;
        green_delta = rgb[i].delta_green_reg;
        blue_delta  = rgb[i].delta_blue_reg;

        red_value   = ((red_delta & 0x3ff) << 14) | (red & 0x3fff);
        green_value = ((green_delta & 0x3ff) << 14) | (green & 0x3fff);
        blue_value  = ((blue_delta & 0x3ff) << 14) | (blue & 0x3fff);

        config_writer_fill(config_writer, REG_FIELD_VALUE(VPMPCC_MCM_SHAPER_LUT_DATA, red_value));
        config_writer_fill(config_writer, REG_FIELD_VALUE(VPMPCC_MCM_SHAPER_LUT_DATA, green_value));
        config_writer_fill(config_writer, REG_FIELD_VALUE(VPMPCC_MCM_SHAPER_LUT_DATA, blue_value));
    }
}

void vpe10_mpc_power_on_1dlut_shaper_3dlut(struct mpc *mpc, bool power_on)
{
    PROGRAM_ENTRY();
    // int max_retries = 10;

    REG_SET_3(VPMPCC_MCM_MEM_PWR_CTRL, REG_DEFAULT(VPMPCC_MCM_MEM_PWR_CTRL),
        VPMPCC_MCM_SHAPER_MEM_PWR_DIS, power_on == true ? 1 : 0, VPMPCC_MCM_3DLUT_MEM_PWR_DIS,
        power_on == true ? 1 : 0, VPMPCC_MCM_1DLUT_MEM_PWR_DIS, power_on == true ? 1 : 0);

    /* wait for memory to fully power up */
    if (power_on && vpe_priv->init.debug.enable_mem_low_power.bits.mpc) {
        // REG_WAIT(VPMPCC_MCM_MEM_PWR_CTRL, VPMPCC_MCM_SHAPER_MEM_PWR_STATE, 0, 1, max_retries);
        //  Use two REG_SET instead of wait for State
        //  TODO: Confirm if this delay is enough
        REG_SET_3(VPMPCC_MCM_MEM_PWR_CTRL, REG_DEFAULT(VPMPCC_MCM_MEM_PWR_CTRL),
            VPMPCC_MCM_SHAPER_MEM_PWR_DIS, power_on == true ? 1 : 0, VPMPCC_MCM_3DLUT_MEM_PWR_DIS,
            power_on == true ? 1 : 0, VPMPCC_MCM_1DLUT_MEM_PWR_DIS, power_on == true ? 1 : 0);
        REG_SET_3(VPMPCC_MCM_MEM_PWR_CTRL, REG_DEFAULT(VPMPCC_MCM_MEM_PWR_CTRL),
            VPMPCC_MCM_SHAPER_MEM_PWR_DIS, power_on == true ? 1 : 0, VPMPCC_MCM_3DLUT_MEM_PWR_DIS,
            power_on == true ? 1 : 0, VPMPCC_MCM_1DLUT_MEM_PWR_DIS, power_on == true ? 1 : 0);

        // REG_WAIT(VPMPCC_MCM_MEM_PWR_CTRL, VPMPCC_MCM_3DLUT_MEM_PWR_STATE, 0, 1, max_retries);
    }
}

bool vpe10_mpc_program_shaper(struct mpc *mpc, const struct pwl_params *params)
{
    PROGRAM_ENTRY();

    if (params == NULL) {
        REG_SET(VPMPCC_MCM_SHAPER_CONTROL, 0, VPMPCC_MCM_SHAPER_LUT_MODE, 0);
        return false;
    }

    // if (vpe_priv->init.debug.enable_mem_low_power.bits.mpc)
    //  should always turn it on
    vpe10_mpc_power_on_1dlut_shaper_3dlut(mpc, true);

    vpe10_mpc_configure_shaper_lut(mpc, true); // Always use LUT_RAM_A

    vpe10_mpc_program_shaper_luta_settings(mpc, params);

    vpe10_mpc_program_shaper_lut(mpc, params->rgb_resulted, params->hw_points_num);

    REG_SET(VPMPCC_MCM_SHAPER_CONTROL, 0, VPMPCC_MCM_SHAPER_LUT_MODE, 1);

    //? Should we check debug option before turn off shaper? -- should be yes
    if (vpe_priv->init.debug.enable_mem_low_power.bits.mpc)
        vpe10_mpc_power_on_1dlut_shaper_3dlut(mpc, false);

    return true;
}

static void vpe10_mpc_select_3dlut_ram(
    struct mpc *mpc, enum vpe_lut_mode mode, bool is_color_channel_12bits)
{
    PROGRAM_ENTRY();

    VPE_ASSERT(mode == LUT_RAM_A);

    REG_UPDATE_2(VPMPCC_MCM_3DLUT_READ_WRITE_CONTROL, VPMPCC_MCM_3DLUT_RAM_SEL,
        mode == LUT_RAM_A ? 0 : 1, VPMPCC_MCM_3DLUT_30BIT_EN, is_color_channel_12bits ? 0 : 1);
}

static void vpe10_mpc_select_3dlut_ram_mask(struct mpc *mpc, uint32_t ram_selection_mask)
{
    PROGRAM_ENTRY();

    REG_UPDATE(
        VPMPCC_MCM_3DLUT_READ_WRITE_CONTROL, VPMPCC_MCM_3DLUT_WRITE_EN_MASK, ram_selection_mask);
    REG_SET(VPMPCC_MCM_3DLUT_INDEX, 0, VPMPCC_MCM_3DLUT_INDEX, 0);
}

static void vpe10_mpc_set3dlut_ram12(struct mpc *mpc, const struct vpe_rgb *lut, uint32_t entries)
{
    PROGRAM_ENTRY();
    uint32_t i, red, green, blue, red1, green1, blue1;
    uint16_t MaxLutEntriesPerPacket =
        (MAX_CONFIG_PACKET_DATA_SIZE_DWORD / 3) * 2; // each two entries consumes 3 DWORDs
    uint16_t ActualEntriesInPacket = 0;
    uint16_t ActualPacketSize;

    // Optimized by single VPEP config packet for same address with multiple write

    for (i = 0; i < entries; i += 2) {
        if (i % MaxLutEntriesPerPacket == 0) { // need generate one another new packet
            ActualEntriesInPacket = MaxLutEntriesPerPacket;

            // If single packet is big enough to contain remaining entries
            if ((entries - i) < MaxLutEntriesPerPacket) {
                ActualEntriesInPacket = (uint16_t)(entries - i);
                if ((entries - i) % 2) {
                    // odd entries, round up to even as we need to program in pair
                    ActualEntriesInPacket++;
                }
            }

            ActualPacketSize = ActualEntriesInPacket * 3 / 2;

            VPE_ASSERT(ActualPacketSize <= MAX_CONFIG_PACKET_DATA_SIZE_DWORD);
            packet.bits.INC = 0;
            packet.bits.VPEP_CONFIG_DATA_SIZE =
                ActualPacketSize - 1; // number of "continuous" dwords, 1-based
            packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(VPMPCC_MCM_3DLUT_DATA);

            config_writer_fill_direct_config_packet_header(config_writer, &packet);
        }

        red   = lut[i].red << 4;
        green = lut[i].green << 4;
        blue  = lut[i].blue << 4;
        if (i + 1 < entries) {
            red1   = lut[i + 1].red << 4;
            green1 = lut[i + 1].green << 4;
            blue1  = lut[i + 1].blue << 4;
        } else {
            // last odd entry, program 0 for extra one that accompany with it.
            red1   = 0;
            green1 = 0;
            blue1  = 0;
        }

        config_writer_fill(config_writer, REG_FIELD_VALUE(VPMPCC_MCM_3DLUT_DATA0, red) |
                                              REG_FIELD_VALUE(VPMPCC_MCM_3DLUT_DATA1, red1));
        config_writer_fill(config_writer, REG_FIELD_VALUE(VPMPCC_MCM_3DLUT_DATA0, green) |
                                              REG_FIELD_VALUE(VPMPCC_MCM_3DLUT_DATA1, green1));
        config_writer_fill(config_writer, REG_FIELD_VALUE(VPMPCC_MCM_3DLUT_DATA0, blue) |
                                              REG_FIELD_VALUE(VPMPCC_MCM_3DLUT_DATA1, blue1));
    }
}

static void vpe10_mpc_set3dlut_ram12_indirect(
    struct mpc *mpc, const uint64_t lut_gpuva, uint32_t entries)
{
    PROGRAM_ENTRY();
    // The layout inside the lut buf must be: (each element is 16bit, but LSB[3:0] are always 0)
    // DW0: R1<<16 | R0
    // DW1: G1<<16 | G0
    // DW2: B1<<16 | B0
    // DW3: R3<<16 | R2
    // DW4: G3<<16 | G2
    // DW5: B3<<16 | B2
    //...

    uint32_t data_array_size = (entries / 2 * 3); // DW size of config data array, actual size

    config_writer_set_type(config_writer, CONFIG_TYPE_INDIRECT);

    // Optimized by single VPEP indirect config packet
    // Fill the 3dLut array pointer
    config_writer_fill_indirect_data_array(config_writer, lut_gpuva, data_array_size);

    // Start from index 0
    config_writer_fill_indirect_destination(
        config_writer, REG_OFFSET(VPMPCC_MCM_3DLUT_INDEX), 0, REG_OFFSET(VPMPCC_MCM_3DLUT_DATA));

    // restore back to direct
    config_writer_set_type(config_writer, CONFIG_TYPE_DIRECT);
}

static void vpe10_mpc_set3dlut_ram10(struct mpc *mpc, const struct vpe_rgb *lut, uint32_t entries)
{
    PROGRAM_ENTRY();
    uint32_t i, red, green, blue, value;
    uint16_t MaxLutEntriesPerPacket =
        MAX_CONFIG_PACKET_DATA_SIZE_DWORD; // each entries consumes 1 DWORDs
    uint16_t ActualPacketSize;

    // Optimize to VPEP direct with multiple data
    for (i = 0; i < entries; i++) {
        // Need to revisit about the new config writer handling , DO WE STILL NEED IT?
        // Yes, this is to ensure how many "packets" we need due to each packet have max data size
        // i.e. need to split into diff packets (but might still in one direct config descriptor)
        // The new config writer handles the "descriptor" size exceeded issue.
        // i.e. need to split into diff direct config descriptors.
        if (i % MaxLutEntriesPerPacket == 0) { // need generate one another new packet
            if ((entries - i) <
                MaxLutEntriesPerPacket) // Single packet is big enough to contain remaining entries
                MaxLutEntriesPerPacket = (uint16_t)(entries - i);

            ActualPacketSize = MaxLutEntriesPerPacket;

            VPE_ASSERT(ActualPacketSize <= MAX_CONFIG_PACKET_DATA_SIZE_DWORD);
            packet.bits.INC = 0;
            packet.bits.VPEP_CONFIG_DATA_SIZE =
                ActualPacketSize - 1; // number of "continuous" dwords, 1-based
            packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(VPMPCC_MCM_3DLUT_DATA_30BIT);

            config_writer_fill_direct_config_packet_header(config_writer, &packet);
        }

        red   = lut[i].red;
        green = lut[i].green;
        blue  = lut[i].blue;
        // should we shift red 22bit and green 12?
        //  Yes, accroding to spec.
        //  let's do it instead of just shift 10 bits
        value = (red << 22) | (green << 12) | blue << 2;

        config_writer_fill(config_writer, REG_FIELD_VALUE(VPMPCC_MCM_3DLUT_DATA_30BIT, value));
    }
}

static void vpe10_mpc_set3dlut_ram10_indirect(
    struct mpc *mpc, const uint64_t lut_gpuva, uint32_t entries)
{
    PROGRAM_ENTRY();

    uint32_t data_array_size = entries; // DW size of config data array, actual size
    // Optimized by single VPEP indirect config packet
    // The layout inside the lut buf must be: (each element is 10bit, but LSB[1:0] are always 0)
    // DW0: R0<<22 | G0<<12 | B0 <<2
    // DW0: R1<<22 | G1<<12 | B1 <<2
    //...

    config_writer_set_type(config_writer, CONFIG_TYPE_INDIRECT);

    // Optimized by single VPEP indirect config packet
    // Fill the 3dLut array pointer
    config_writer_fill_indirect_data_array(config_writer, lut_gpuva, data_array_size);

    // Start from index 0
    config_writer_fill_indirect_destination(
        config_writer, REG_OFFSET(VPMPCC_MCM_3DLUT_INDEX), 0, REG_OFFSET(VPMPCC_MCM_3DLUT_DATA));

    // resume back to direct
    config_writer_set_type(config_writer, CONFIG_TYPE_DIRECT);
}

static void vpe10_mpc_set_3dlut_mode(
    struct mpc *mpc, enum vpe_lut_mode mode, bool is_lut_size17x17x17)
{
    PROGRAM_ENTRY();
    uint32_t lut_mode;

    if (mode == LUT_BYPASS)
        lut_mode = 0;
    else if (mode == LUT_RAM_A)
        lut_mode = 1;
    else
        lut_mode = 2;

    REG_SET_2(VPMPCC_MCM_3DLUT_MODE, 0, VPMPCC_MCM_3DLUT_MODE, lut_mode, VPMPCC_MCM_3DLUT_SIZE,
        is_lut_size17x17x17 == true ? 0 : 1);
}

// using direct config to program the 3dlut specified in params
void vpe10_mpc_program_3dlut(struct mpc *mpc, const struct tetrahedral_params *params)
{
    PROGRAM_ENTRY();
    enum vpe_lut_mode     mode;
    bool                  is_17x17x17;
    bool                  is_12bits_color_channel;
    const struct vpe_rgb *lut0;
    const struct vpe_rgb *lut1;
    const struct vpe_rgb *lut2;
    const struct vpe_rgb *lut3;
    uint32_t              lut_size0;
    uint32_t              lut_size;

    if (params == NULL) {
        vpe10_mpc_set_3dlut_mode(mpc, LUT_BYPASS, false);
        return;
    }

    vpe10_mpc_power_on_1dlut_shaper_3dlut(mpc, true);

    // always use LUT_RAM_A except for bypass mode which is not the case here
    mode = LUT_RAM_A;

    is_17x17x17             = !params->use_tetrahedral_9;
    is_12bits_color_channel = params->use_12bits;
    if (is_17x17x17) {
        lut0      = params->tetrahedral_17.lut0;
        lut1      = params->tetrahedral_17.lut1;
        lut2      = params->tetrahedral_17.lut2;
        lut3      = params->tetrahedral_17.lut3;
        lut_size0 = sizeof(params->tetrahedral_17.lut0) / sizeof(params->tetrahedral_17.lut0[0]);
        lut_size  = sizeof(params->tetrahedral_17.lut1) / sizeof(params->tetrahedral_17.lut1[0]);
    } else {
        lut0      = params->tetrahedral_9.lut0;
        lut1      = params->tetrahedral_9.lut1;
        lut2      = params->tetrahedral_9.lut2;
        lut3      = params->tetrahedral_9.lut3;
        lut_size0 = sizeof(params->tetrahedral_9.lut0) / sizeof(params->tetrahedral_9.lut0[0]);
        lut_size  = sizeof(params->tetrahedral_9.lut1) / sizeof(params->tetrahedral_9.lut1[0]);
    }

    vpe10_mpc_select_3dlut_ram(mpc, mode, is_12bits_color_channel);
    // set mask to LUT0
    vpe10_mpc_select_3dlut_ram_mask(mpc, 0x1);
    if (is_12bits_color_channel)
        vpe10_mpc_set3dlut_ram12(mpc, lut0, lut_size0);
    else
        vpe10_mpc_set3dlut_ram10(mpc, lut0, lut_size0);

    // set mask to LUT1
    vpe10_mpc_select_3dlut_ram_mask(mpc, 0x2);
    if (is_12bits_color_channel)
        vpe10_mpc_set3dlut_ram12(mpc, lut1, lut_size);
    else
        vpe10_mpc_set3dlut_ram10(mpc, lut1, lut_size);

    // set mask to LUT2
    vpe10_mpc_select_3dlut_ram_mask(mpc, 0x4);
    if (is_12bits_color_channel)
        vpe10_mpc_set3dlut_ram12(mpc, lut2, lut_size);
    else
        vpe10_mpc_set3dlut_ram10(mpc, lut2, lut_size);

    // set mask to LUT3
    vpe10_mpc_select_3dlut_ram_mask(mpc, 0x8);
    if (is_12bits_color_channel)
        vpe10_mpc_set3dlut_ram12(mpc, lut3, lut_size);
    else
        vpe10_mpc_set3dlut_ram10(mpc, lut3, lut_size);

    vpe10_mpc_set_3dlut_mode(mpc, mode, is_17x17x17);

    if (vpe_priv->init.debug.enable_mem_low_power.bits.mpc)
        vpe10_mpc_power_on_1dlut_shaper_3dlut(mpc, false);

    return;
}

// using indirect config to configure the 3DLut
// note that we still need direct config to switch the mask between lut0 - lut3
bool vpe10_mpc_program_3dlut_indirect(struct mpc *mpc,
    struct vpe_buf *lut0_3_buf, // 3d lut buf which contains the data for lut0-lut3
    bool use_tetrahedral_9, bool use_12bits)
{
    PROGRAM_ENTRY();
    enum vpe_lut_mode            mode;
    bool                         is_17x17x17;
    bool                         is_12bits_color_channel;
    uint64_t                     lut0_gpuva;
    uint64_t                     lut1_gpuva;
    uint64_t                     lut2_gpuva;
    uint64_t                     lut3_gpuva;
    uint32_t                     lut_size0;
    uint32_t                     lut_size;
    struct tetrahedral_17x17x17 *tetra17 = NULL;
    struct tetrahedral_9x9x9    *tetra9  = NULL;

    // make sure it is in DIRECT type
    config_writer_set_type(config_writer, CONFIG_TYPE_DIRECT);

    if (lut0_3_buf == NULL) {
        vpe10_mpc_set_3dlut_mode(mpc, LUT_BYPASS, false);
        return false;
    }

    vpe10_mpc_power_on_1dlut_shaper_3dlut(mpc, true);

    // always use LUT_RAM_A except for bypass mode which is not the case here
    mode = LUT_RAM_A;

    is_17x17x17             = !use_tetrahedral_9;
    is_12bits_color_channel = use_12bits;
    if (is_17x17x17) {
        lut0_gpuva = lut0_3_buf->gpu_va;
        lut1_gpuva = lut0_3_buf->gpu_va + (uint64_t)(offsetof(struct tetrahedral_17x17x17, lut1));
        lut2_gpuva = lut0_3_buf->gpu_va + (uint64_t)(offsetof(struct tetrahedral_17x17x17, lut2));
        lut3_gpuva = lut0_3_buf->gpu_va + (uint64_t)(offsetof(struct tetrahedral_17x17x17, lut3));
        lut_size0  = sizeof(tetra17->lut0) / sizeof(tetra17->lut0[0]);
        lut_size   = sizeof(tetra17->lut1) / sizeof(tetra17->lut1[0]);
    } else {
        lut0_gpuva = lut0_3_buf->gpu_va;
        lut1_gpuva = lut0_3_buf->gpu_va + (uint64_t)(offsetof(struct tetrahedral_9x9x9, lut1));
        lut2_gpuva = lut0_3_buf->gpu_va + (uint64_t)(offsetof(struct tetrahedral_9x9x9, lut2));
        lut3_gpuva = lut0_3_buf->gpu_va + (uint64_t)(offsetof(struct tetrahedral_9x9x9, lut3));
        lut_size0  = sizeof(tetra9->lut0) / sizeof(tetra9->lut0[0]);
        lut_size   = sizeof(tetra9->lut1) / sizeof(tetra9->lut1[0]);
    }

    vpe10_mpc_select_3dlut_ram(mpc, mode, is_12bits_color_channel);

    // set mask to LUT0
    vpe10_mpc_select_3dlut_ram_mask(mpc, 0x1);
    if (is_12bits_color_channel)
        vpe10_mpc_set3dlut_ram12_indirect(mpc, lut0_gpuva, lut_size0);
    else
        vpe10_mpc_set3dlut_ram10_indirect(mpc, lut0_gpuva, lut_size0);

    // set mask to LUT1
    vpe10_mpc_select_3dlut_ram_mask(mpc, 0x2);
    if (is_12bits_color_channel)
        vpe10_mpc_set3dlut_ram12_indirect(mpc, lut1_gpuva, lut_size);
    else
        vpe10_mpc_set3dlut_ram10_indirect(mpc, lut1_gpuva, lut_size);

    // set mask to LUT2
    vpe10_mpc_select_3dlut_ram_mask(mpc, 0x4);
    if (is_12bits_color_channel)
        vpe10_mpc_set3dlut_ram12_indirect(mpc, lut2_gpuva, lut_size);
    else
        vpe10_mpc_set3dlut_ram10_indirect(mpc, lut2_gpuva, lut_size);

    // set mask to LUT3
    vpe10_mpc_select_3dlut_ram_mask(mpc, 0x8);
    if (is_12bits_color_channel)
        vpe10_mpc_set3dlut_ram12_indirect(mpc, lut3_gpuva, lut_size);
    else
        vpe10_mpc_set3dlut_ram10_indirect(mpc, lut3_gpuva, lut_size);

    vpe10_mpc_set_3dlut_mode(mpc, mode, is_17x17x17);

    if (vpe_priv->init.debug.enable_mem_low_power.bits.mpc)
        vpe10_mpc_power_on_1dlut_shaper_3dlut(mpc, false);

    return true;
}

static void vpe10_mpc_configure_1dlut(struct mpc *mpc, bool is_ram_a)
{
    PROGRAM_ENTRY();

    REG_SET_2(VPMPCC_MCM_1DLUT_LUT_CONTROL, 0, VPMPCC_MCM_1DLUT_LUT_WRITE_COLOR_MASK, 7,
        VPMPCC_MCM_1DLUT_LUT_HOST_SEL, is_ram_a == true ? 0 : 1);

    REG_SET(VPMPCC_MCM_1DLUT_LUT_INDEX, 0, VPMPCC_MCM_1DLUT_LUT_INDEX, 0);
}

static void vpe10_mpc_1dlut_get_reg_field(struct mpc *mpc, struct vpe10_xfer_func_reg *reg)
{
    struct vpe10_mpc *vpe10_mpc = (struct vpe10_mpc *)mpc;

    reg->shifts.field_region_start_base =
        vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_BASE_B;
    reg->masks.field_region_start_base =
        vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_BASE_B;
    reg->shifts.field_offset = vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_OFFSET_B;
    reg->masks.field_offset  = vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_OFFSET_B;

    reg->shifts.exp_region0_lut_offset =
        vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION0_LUT_OFFSET;
    reg->masks.exp_region0_lut_offset =
        vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION0_LUT_OFFSET;
    reg->shifts.exp_region0_num_segments =
        vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION0_NUM_SEGMENTS;
    reg->masks.exp_region0_num_segments =
        vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION0_NUM_SEGMENTS;
    reg->shifts.exp_region1_lut_offset =
        vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION1_LUT_OFFSET;
    reg->masks.exp_region1_lut_offset =
        vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION1_LUT_OFFSET;
    reg->shifts.exp_region1_num_segments =
        vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION1_NUM_SEGMENTS;
    reg->masks.exp_region1_num_segments =
        vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION1_NUM_SEGMENTS;

    reg->shifts.field_region_end = vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_B;
    reg->masks.field_region_end  = vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_B;
    reg->shifts.field_region_end_slope =
        vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_SLOPE_B;
    reg->masks.field_region_end_slope =
        vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_SLOPE_B;
    reg->shifts.field_region_end_base =
        vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_BASE_B;
    reg->masks.field_region_end_base = vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_END_BASE_B;
    reg->shifts.field_region_linear_slope =
        vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SLOPE_B;
    reg->masks.field_region_linear_slope =
        vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SLOPE_B;
    reg->shifts.exp_region_start = vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_B;
    reg->masks.exp_region_start  = vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_B;
    reg->shifts.exp_region_start_segment =
        vpe10_mpc->shift->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SEGMENT_B;
    reg->masks.exp_region_start_segment =
        vpe10_mpc->mask->VPMPCC_MCM_1DLUT_RAMA_EXP_REGION_START_SEGMENT_B;
}

/*program blnd lut RAM A*/
static void vpe10_mpc_program_1dlut_luta_settings(struct mpc *mpc, const struct pwl_params *params)
{
    PROGRAM_ENTRY();
    struct vpe10_xfer_func_reg gam_regs;

    vpe10_mpc_1dlut_get_reg_field(mpc, &gam_regs);

    gam_regs.start_cntl_b       = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_START_CNTL_B);
    gam_regs.start_cntl_g       = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_START_CNTL_G);
    gam_regs.start_cntl_r       = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_START_CNTL_R);
    gam_regs.start_slope_cntl_b = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_B);
    gam_regs.start_slope_cntl_g = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_G);
    gam_regs.start_slope_cntl_r = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_START_SLOPE_CNTL_R);
    gam_regs.start_end_cntl1_b  = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_B);
    gam_regs.start_end_cntl2_b  = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_B);
    gam_regs.start_end_cntl1_g  = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_G);
    gam_regs.start_end_cntl2_g  = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_G);
    gam_regs.start_end_cntl1_r  = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_END_CNTL1_R);
    gam_regs.start_end_cntl2_r  = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_END_CNTL2_R);
    gam_regs.region_start       = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_REGION_0_1);
    gam_regs.region_end         = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_REGION_32_33);
    gam_regs.offset_b           = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_OFFSET_B);
    gam_regs.offset_g           = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_OFFSET_G);
    gam_regs.offset_r           = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_OFFSET_R);
    gam_regs.start_base_cntl_b  = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_B);
    gam_regs.start_base_cntl_g  = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_G);
    gam_regs.start_base_cntl_r  = REG_OFFSET(VPMPCC_MCM_1DLUT_RAMA_START_BASE_CNTL_R);

    vpe10_cm_helper_program_gamcor_xfer_func(config_writer, params, &gam_regs);
}

static void vpe10_mpc_program_1dlut_pwl(
    struct mpc *mpc, const struct pwl_result_data *rgb, uint32_t num)
{
    PROGRAM_ENTRY();

    uint32_t last_base_value_red   = rgb[num].red_reg;
    uint32_t last_base_value_green = rgb[num].blue_reg;
    uint32_t last_base_value_blue  = rgb[num].green_reg;

    if (vpe_is_rgb_equal(rgb, num)) {
        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_red, num,
            REG_OFFSET(VPMPCC_MCM_1DLUT_LUT_DATA), REG_FIELD_SHIFT(VPMPCC_MCM_1DLUT_LUT_DATA),
            REG_FIELD_MASK(VPMPCC_MCM_1DLUT_LUT_DATA), CM_PWL_R);
    } else {
        REG_SET(VPMPCC_MCM_1DLUT_LUT_INDEX, 0, VPMPCC_MCM_1DLUT_LUT_INDEX, 0);
        REG_UPDATE(VPMPCC_MCM_1DLUT_LUT_CONTROL, VPMPCC_MCM_1DLUT_LUT_WRITE_COLOR_MASK, 4);

        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_red, num,
            REG_OFFSET(VPMPCC_MCM_1DLUT_LUT_DATA), REG_FIELD_SHIFT(VPMPCC_MCM_1DLUT_LUT_DATA),
            REG_FIELD_MASK(VPMPCC_MCM_1DLUT_LUT_DATA), CM_PWL_R);

        REG_SET(VPMPCC_MCM_1DLUT_LUT_INDEX, 0, VPMPCC_MCM_1DLUT_LUT_INDEX, 0);
        REG_UPDATE(VPMPCC_MCM_1DLUT_LUT_CONTROL, VPMPCC_MCM_1DLUT_LUT_WRITE_COLOR_MASK, 2);

        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_green, num,
            REG_OFFSET(VPMPCC_MCM_1DLUT_LUT_DATA), REG_FIELD_SHIFT(VPMPCC_MCM_1DLUT_LUT_DATA),
            REG_FIELD_MASK(VPMPCC_MCM_1DLUT_LUT_DATA), CM_PWL_G);

        REG_SET(VPMPCC_MCM_1DLUT_LUT_INDEX, 0, VPMPCC_MCM_1DLUT_LUT_INDEX, 0);
        REG_UPDATE(VPMPCC_MCM_1DLUT_LUT_CONTROL, VPMPCC_MCM_1DLUT_LUT_WRITE_COLOR_MASK, 1);

        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_blue, num,
            REG_OFFSET(VPMPCC_MCM_1DLUT_LUT_DATA), REG_FIELD_SHIFT(VPMPCC_MCM_1DLUT_LUT_DATA),
            REG_FIELD_MASK(VPMPCC_MCM_1DLUT_LUT_DATA), CM_PWL_B);
    }
}

// Blend-gamma control.
void vpe10_mpc_program_1dlut(struct mpc *mpc, const struct pwl_params *params)
{
    PROGRAM_ENTRY();

    if (params == NULL) {
        REG_SET(VPMPCC_MCM_1DLUT_CONTROL, REG_DEFAULT(VPMPCC_MCM_1DLUT_CONTROL),
            VPMPCC_MCM_1DLUT_MODE, 0);

        if (vpe_priv->init.debug.enable_mem_low_power.bits.mpc)
            vpe10_mpc_power_on_1dlut_shaper_3dlut(mpc, false);
        return;
    }

    vpe10_mpc_power_on_1dlut_shaper_3dlut(mpc, true);

    vpe10_mpc_configure_1dlut(mpc, true);
    vpe10_mpc_program_1dlut_luta_settings(mpc, params);
    vpe10_mpc_program_1dlut_pwl(mpc, params->rgb_resulted, params->hw_points_num);

    REG_SET(
        VPMPCC_MCM_1DLUT_CONTROL, REG_DEFAULT(VPMPCC_MCM_1DLUT_CONTROL), VPMPCC_MCM_1DLUT_MODE, 2);
}

void vpe10_mpc_program_cm_location(struct mpc *mpc, uint8_t location)
{
    PROGRAM_ENTRY();
    // Location 0 == before blending,
    // Location 1 == after blending
    REG_SET(VPMPCC_MOVABLE_CM_LOCATION_CONTROL, REG_DEFAULT(VPMPCC_MOVABLE_CM_LOCATION_CONTROL),
        VPMPCC_MOVABLE_CM_LOCATION_CNTL, location);
}

void vpe10_mpc_set_denorm(struct mpc *mpc, int opp_id, enum color_depth output_depth,
    struct mpc_denorm_clamp *denorm_clamp)
{
    PROGRAM_ENTRY();
    /* De-normalize Fixed U1.13 color data to different target bit depths. 0 is bypass*/
    int denorm_mode = 0;

    VPE_ASSERT(opp_id == 0); // Only support opp0 in v1

    switch (output_depth) {
    case COLOR_DEPTH_666:
        denorm_mode = 1;
        break;
    case COLOR_DEPTH_888:
        denorm_mode = 2;
        break;
    case COLOR_DEPTH_999:
        denorm_mode = 3;
        break;
    case COLOR_DEPTH_101010:
        denorm_mode = 4;
        break;
    case COLOR_DEPTH_111111:
        denorm_mode = 5;
        break;
    case COLOR_DEPTH_121212:
        denorm_mode = 6;
        break;
    case COLOR_DEPTH_141414:
    case COLOR_DEPTH_161616:
    default:
        /* not valid used case! */
        break;
    }

    /*program min and max clamp values for the pixel components*/
    if (denorm_clamp) {
        REG_SET_3(VPMPC_OUT0_DENORM_CONTROL, 0, VPMPC_OUT_DENORM_MODE, denorm_mode,
            VPMPC_OUT_DENORM_CLAMP_MAX_R_CR, denorm_clamp->clamp_max_r_cr,
            VPMPC_OUT_DENORM_CLAMP_MIN_R_CR, denorm_clamp->clamp_min_r_cr);
        REG_SET_2(VPMPC_OUT0_DENORM_CLAMP_G_Y, 0, VPMPC_OUT_DENORM_CLAMP_MAX_G_Y,
            denorm_clamp->clamp_max_g_y, VPMPC_OUT_DENORM_CLAMP_MIN_G_Y,
            denorm_clamp->clamp_min_g_y);
        REG_SET_2(VPMPC_OUT0_DENORM_CLAMP_B_CB, 0, VPMPC_OUT_DENORM_CLAMP_MAX_B_CB,
            denorm_clamp->clamp_max_b_cb, VPMPC_OUT_DENORM_CLAMP_MIN_B_CB,
            denorm_clamp->clamp_min_b_cb);
    } else {
        REG_SET(VPMPC_OUT0_DENORM_CONTROL, REG_DEFAULT(VPMPC_OUT0_DENORM_CONTROL),
            VPMPC_OUT_DENORM_MODE, denorm_mode);
        REG_SET_DEFAULT(VPMPC_OUT0_DENORM_CLAMP_G_Y);
        REG_SET_DEFAULT(VPMPC_OUT0_DENORM_CLAMP_B_CB);
    }
}

void vpe10_mpc_set_out_float_en(struct mpc *mpc, bool float_enable)
{
    PROGRAM_ENTRY();

    REG_SET(VPMPC_OUT0_FLOAT_CONTROL, 0, VPMPC_OUT_FLOAT_EN, float_enable);
}

void vpe10_mpc_program_mpc_out(struct mpc *mpc, enum vpe_surface_pixel_format format)
{
    // check output format/color depth
    mpc->funcs->set_out_float_en(mpc, vpe_is_fp16(format));
    mpc->funcs->set_denorm(mpc, 0, vpe_get_color_depth(format), NULL);
}

void vpe10_mpc_set_mpc_shaper_3dlut(
    struct mpc *mpc, const struct transfer_func *func_shaper, const struct vpe_3dlut *lut3d_func)
{
    const struct pwl_params *shaper_lut = NULL;
    // get the shaper lut params
    if (func_shaper) {
        if (func_shaper->type == TF_TYPE_DISTRIBUTED_POINTS) {
            vpe10_cm_helper_translate_curve_to_hw_format(
                func_shaper, &mpc->shaper_params, true); // should init shaper_params first
            shaper_lut = &mpc->shaper_params;            // are there shaper prams in dpp instead?
        } else if (func_shaper->type == TF_TYPE_HWPWL) {
            shaper_lut = &func_shaper->pwl;
        }
    }

    mpc->funcs->program_shaper(mpc, shaper_lut);

    if (lut3d_func) {
        if (lut3d_func->state.bits.initialized)
            mpc->funcs->program_3dlut(mpc, &lut3d_func->lut_3d);
        else
            mpc->funcs->program_3dlut(mpc, NULL);
    }
    return;
}

void vpe10_mpc_set_output_transfer_func(struct mpc *mpc, struct output_ctx *output_ctx)
{
    /* program OGAM only for the top pipe*/
    struct pwl_params *params = NULL;
    bool               ret    = false;

    if (ret == false && output_ctx->output_tf) {
        // No support HWPWL as it is legacy
        if (output_ctx->output_tf->type == TF_TYPE_DISTRIBUTED_POINTS) {
            if (!output_ctx->output_tf->use_pre_calculated_table ||
                mpc->vpe_priv->init.debug.force_tf_calculation) {
                vpe10_cm_helper_translate_curve_to_hw_format( // this is cm3.0 version instead 1.0
                                                              // as DCN3.2
                    output_ctx->output_tf, &mpc->regamma_params, false);
                params = &mpc->regamma_params;
            } else {
                vpe10_cm_get_tf_pwl_params(output_ctx->output_tf, &params, CM_REGAM);
                VPE_ASSERT(params != NULL);
                if (params == NULL)
                    return;
            }
        }
        /* there are no ROM LUTs in OUTGAM */
        if (output_ctx->output_tf->type == TF_TYPE_PREDEFINED)
            VPE_ASSERT(0);
    }
    mpc->funcs->set_output_gamma(mpc, params);
}

void vpe10_mpc_set_blend_lut(struct mpc *mpc, const struct transfer_func *blend_tf)
{
    struct pwl_params *blend_lut = NULL;

    if (blend_tf) {
        if (blend_tf->type == TF_TYPE_DISTRIBUTED_POINTS) {
            vpe10_cm_helper_translate_curve_to_degamma_hw_format(
                blend_tf, &mpc->blender_params); // TODO should init regamma_params first
            blend_lut = &mpc->blender_params;
        }
    }
    mpc->funcs->program_1dlut(mpc, blend_lut);

    return;
}

bool vpe10_mpc_program_movable_cm(struct mpc *mpc, const struct transfer_func *func_shaper,
    const struct vpe_3dlut *lut3d_func, const struct transfer_func *blend_tf, bool afterblend)
{
    struct pwl_params *params = NULL;
    bool               ret    = false;

    /*program shaper and 3dlut and 1dlut in MPC*/
    mpc->funcs->set_mpc_shaper_3dlut(mpc, func_shaper, lut3d_func);
    mpc->funcs->set_blend_lut(mpc, blend_tf);
    mpc->funcs->program_cm_location(mpc, afterblend);

    return ret;
}

void vpe10_mpc_program_crc(struct mpc *mpc, bool enable)
{
    PROGRAM_ENTRY();
    REG_UPDATE(VPMPC_CRC_CTRL, VPMPC_CRC_EN, enable);
}

