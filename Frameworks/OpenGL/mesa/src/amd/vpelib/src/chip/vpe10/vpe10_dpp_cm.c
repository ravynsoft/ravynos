
#include "vpe_priv.h"
#include "reg_helper.h"
#include "vpe10/inc/vpe10_cm_common.h"
#include "vpe10_dpp.h"
#include "conversion.h"
#include "color_pwl.h"

#define CTX      vpe10_dpp
#define CTX_BASE dpp

static void vpe10_enable_cm_block(struct dpp *dpp)
{
    unsigned int cm_bypass_mode = 0;

    PROGRAM_ENTRY();

    // debug option: put CM in bypass mode
    if (vpe_priv->init.debug.cm_in_bypass)
        cm_bypass_mode = 1;

    REG_SET(VPCM_CONTROL, 0, VPCM_BYPASS, cm_bypass_mode);
}

static void vpe10_power_on_gamcor_lut(struct dpp *dpp, bool power_on)
{
    PROGRAM_ENTRY();

    if (vpe_priv->init.debug.enable_mem_low_power.bits.cm) {
        if (power_on) {
            REG_SET_2(VPCM_MEM_PWR_CTRL, REG_DEFAULT(VPCM_MEM_PWR_CTRL), GAMCOR_MEM_PWR_DIS, 0,
                GAMCOR_MEM_PWR_FORCE, 0);

            // two dummy updates (10-15clks each) for wake up delay
            REG_SET_2(VPCM_MEM_PWR_CTRL, REG_DEFAULT(VPCM_MEM_PWR_CTRL), GAMCOR_MEM_PWR_DIS, 0,
                GAMCOR_MEM_PWR_FORCE, 0);
            REG_SET_2(VPCM_MEM_PWR_CTRL, REG_DEFAULT(VPCM_MEM_PWR_CTRL), GAMCOR_MEM_PWR_DIS, 0,
                GAMCOR_MEM_PWR_FORCE, 0);
        } else {
            REG_SET_2(VPCM_MEM_PWR_CTRL, REG_DEFAULT(VPCM_MEM_PWR_CTRL), GAMCOR_MEM_PWR_DIS, 0,
                GAMCOR_MEM_PWR_FORCE, 3);
        }
    } else {
        REG_SET_2(VPCM_MEM_PWR_CTRL, REG_DEFAULT(VPCM_MEM_PWR_CTRL), GAMCOR_MEM_PWR_DIS,
            power_on == true ? 1 : 0, GAMCOR_MEM_PWR_FORCE, 0);
    }
}

static void vpe10_configure_gamcor_lut(struct dpp *dpp)
{
    PROGRAM_ENTRY();

    REG_SET(VPCM_GAMCOR_LUT_CONTROL, 0, VPCM_GAMCOR_LUT_WRITE_COLOR_MASK, 7);
    REG_SET(VPCM_GAMCOR_LUT_INDEX, 0, VPCM_GAMCOR_LUT_INDEX, 0);
}

static void vpe10_dpp_gamcor_reg_field(struct dpp *dpp, struct vpe10_xfer_func_reg *reg)
{
    struct vpe10_dpp *vpe10_dpp = (struct vpe10_dpp *)dpp;

    reg->shifts.field_region_start_base =
        vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION_START_BASE_B;
    reg->masks.field_region_start_base = vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION_START_BASE_B;
    reg->shifts.field_offset           = vpe10_dpp->shift->VPCM_GAMCOR_RAMA_OFFSET_B;
    reg->masks.field_offset            = vpe10_dpp->mask->VPCM_GAMCOR_RAMA_OFFSET_B;

    reg->shifts.exp_region0_lut_offset = vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION0_LUT_OFFSET;
    reg->masks.exp_region0_lut_offset  = vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION0_LUT_OFFSET;
    reg->shifts.exp_region0_num_segments =
        vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION0_NUM_SEGMENTS;
    reg->masks.exp_region0_num_segments =
        vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION0_NUM_SEGMENTS;
    reg->shifts.exp_region1_lut_offset = vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION1_LUT_OFFSET;
    reg->masks.exp_region1_lut_offset  = vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION1_LUT_OFFSET;
    reg->shifts.exp_region1_num_segments =
        vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION1_NUM_SEGMENTS;
    reg->masks.exp_region1_num_segments =
        vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION1_NUM_SEGMENTS;

    reg->shifts.field_region_end       = vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION_END_B;
    reg->masks.field_region_end        = vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION_END_B;
    reg->shifts.field_region_end_slope = vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION_END_SLOPE_B;
    reg->masks.field_region_end_slope  = vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION_END_SLOPE_B;
    reg->shifts.field_region_end_base  = vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION_END_BASE_B;
    reg->masks.field_region_end_base   = vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION_END_BASE_B;
    reg->shifts.field_region_linear_slope =
        vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION_START_SLOPE_B;
    reg->masks.field_region_linear_slope =
        vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION_START_SLOPE_B;
    reg->shifts.exp_region_start = vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION_START_B;
    reg->masks.exp_region_start  = vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION_START_B;
    reg->shifts.exp_region_start_segment =
        vpe10_dpp->shift->VPCM_GAMCOR_RAMA_EXP_REGION_START_SEGMENT_B;
    reg->masks.exp_region_start_segment =
        vpe10_dpp->mask->VPCM_GAMCOR_RAMA_EXP_REGION_START_SEGMENT_B;
}

static void vpe10_dpp_program_gammcor_lut(
    struct dpp *dpp, const struct pwl_result_data *rgb, uint32_t num)
{
    uint32_t last_base_value_red   = rgb[num].red_reg;
    uint32_t last_base_value_green = rgb[num].blue_reg;
    uint32_t last_base_value_blue  = rgb[num].green_reg;

    PROGRAM_ENTRY();

    /*fill in the LUT with all base values to be used by pwl module
     * HW auto increments the LUT index: back-to-back write
     */
    if (vpe_is_rgb_equal(rgb, num)) {
        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_red, num,
            REG_OFFSET(VPCM_GAMCOR_LUT_DATA), REG_FIELD_SHIFT(VPCM_GAMCOR_LUT_DATA),
            REG_FIELD_MASK(VPCM_GAMCOR_LUT_DATA), CM_PWL_R);
    } else {
        REG_UPDATE(VPCM_GAMCOR_LUT_CONTROL, VPCM_GAMCOR_LUT_WRITE_COLOR_MASK, 4);

        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_red, num,
            REG_OFFSET(VPCM_GAMCOR_LUT_DATA), REG_FIELD_SHIFT(VPCM_GAMCOR_LUT_DATA),
            REG_FIELD_MASK(VPCM_GAMCOR_LUT_DATA), CM_PWL_R);

        REG_SET(VPCM_GAMCOR_LUT_INDEX, 0, VPCM_GAMCOR_LUT_INDEX, 0);
        REG_UPDATE(VPCM_GAMCOR_LUT_CONTROL, VPCM_GAMCOR_LUT_WRITE_COLOR_MASK, 2);

        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_green, num,
            REG_OFFSET(VPCM_GAMCOR_LUT_DATA), REG_FIELD_SHIFT(VPCM_GAMCOR_LUT_DATA),
            REG_FIELD_MASK(VPCM_GAMCOR_LUT_DATA), CM_PWL_G);

        REG_SET(VPCM_GAMCOR_LUT_INDEX, 0, VPCM_GAMCOR_LUT_INDEX, 0);
        REG_UPDATE(VPCM_GAMCOR_LUT_CONTROL, VPCM_GAMCOR_LUT_WRITE_COLOR_MASK, 1);

        vpe10_cm_helper_program_pwl(config_writer, rgb, last_base_value_blue, num,
            REG_OFFSET(VPCM_GAMCOR_LUT_DATA), REG_FIELD_SHIFT(VPCM_GAMCOR_LUT_DATA),
            REG_FIELD_MASK(VPCM_GAMCOR_LUT_DATA), CM_PWL_B);
    }
}

static void vpe10_dpp_program_gamcor_lut(struct dpp *dpp, const struct pwl_params *params)
{
    struct vpe10_xfer_func_reg gam_regs = {0};

    PROGRAM_ENTRY();

    vpe10_enable_cm_block(dpp);

    if (dpp->vpe_priv->init.debug.bypass_gamcor || params == NULL) {
        // bypass
        REG_SET(VPCM_GAMCOR_CONTROL, 0, VPCM_GAMCOR_MODE, 0);
        vpe10_power_on_gamcor_lut(dpp, false);
        return;
    }

    vpe10_power_on_gamcor_lut(dpp, true);
    vpe10_configure_gamcor_lut(dpp);

    REG_SET(VPCM_GAMCOR_CONTROL, 0, VPCM_GAMCOR_MODE, 2); // programmable RAM

    gam_regs.start_cntl_b       = REG_OFFSET(VPCM_GAMCOR_RAMA_START_CNTL_B);
    gam_regs.start_cntl_g       = REG_OFFSET(VPCM_GAMCOR_RAMA_START_CNTL_G);
    gam_regs.start_cntl_r       = REG_OFFSET(VPCM_GAMCOR_RAMA_START_CNTL_R);
    gam_regs.start_slope_cntl_b = REG_OFFSET(VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_B);
    gam_regs.start_slope_cntl_g = REG_OFFSET(VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_G);
    gam_regs.start_slope_cntl_r = REG_OFFSET(VPCM_GAMCOR_RAMA_START_SLOPE_CNTL_R);
    gam_regs.start_end_cntl1_b  = REG_OFFSET(VPCM_GAMCOR_RAMA_END_CNTL1_B);
    gam_regs.start_end_cntl2_b  = REG_OFFSET(VPCM_GAMCOR_RAMA_END_CNTL2_B);
    gam_regs.start_end_cntl1_g  = REG_OFFSET(VPCM_GAMCOR_RAMA_END_CNTL1_G);
    gam_regs.start_end_cntl2_g  = REG_OFFSET(VPCM_GAMCOR_RAMA_END_CNTL2_G);
    gam_regs.start_end_cntl1_r  = REG_OFFSET(VPCM_GAMCOR_RAMA_END_CNTL1_R);
    gam_regs.start_end_cntl2_r  = REG_OFFSET(VPCM_GAMCOR_RAMA_END_CNTL2_R);
    gam_regs.region_start       = REG_OFFSET(VPCM_GAMCOR_RAMA_REGION_0_1);
    gam_regs.region_end         = REG_OFFSET(VPCM_GAMCOR_RAMA_REGION_32_33);
    gam_regs.offset_b           = REG_OFFSET(VPCM_GAMCOR_RAMA_OFFSET_B);
    gam_regs.offset_g           = REG_OFFSET(VPCM_GAMCOR_RAMA_OFFSET_G);
    gam_regs.offset_r           = REG_OFFSET(VPCM_GAMCOR_RAMA_OFFSET_R);
    gam_regs.start_base_cntl_b  = REG_OFFSET(VPCM_GAMCOR_RAMA_START_BASE_CNTL_B);
    gam_regs.start_base_cntl_g  = REG_OFFSET(VPCM_GAMCOR_RAMA_START_BASE_CNTL_G);
    gam_regs.start_base_cntl_r  = REG_OFFSET(VPCM_GAMCOR_RAMA_START_BASE_CNTL_R);

    vpe10_dpp_gamcor_reg_field(dpp, &gam_regs);

    vpe10_cm_helper_program_gamcor_xfer_func(config_writer, params, &gam_regs);
    vpe10_dpp_program_gammcor_lut(dpp, params->rgb_resulted, params->hw_points_num);
}

void vpe10_dpp_program_input_transfer_func(struct dpp *dpp, struct transfer_func *input_tf)
{
    struct pwl_params *params = NULL;

    PROGRAM_ENTRY();

    // There should always have input_tf
    VPE_ASSERT(input_tf);
    // Only accept either DISTRIBUTED_POINTS or BYPASS
    // No support for PREDEFINED case
    VPE_ASSERT(input_tf->type == TF_TYPE_DISTRIBUTED_POINTS || input_tf->type == TF_TYPE_BYPASS);

    // VPE always do NL scaling using gamcor, thus skipping dgam (default bypass)
    // dpp->funcs->program_pre_dgam(dpp, tf);
    if (input_tf->type == TF_TYPE_DISTRIBUTED_POINTS) {
        if (!input_tf->use_pre_calculated_table || dpp->vpe_priv->init.debug.force_tf_calculation) {
            vpe10_cm_helper_translate_curve_to_degamma_hw_format(input_tf, &dpp->degamma_params);
            params = &dpp->degamma_params;
        } else {
            vpe10_cm_get_tf_pwl_params(input_tf, &params, CM_DEGAM);
            VPE_ASSERT(params != NULL);
            if (params == NULL)
                return;
        }
    }
    vpe10_dpp_program_gamcor_lut(dpp, params);
}

void vpe10_dpp_program_gamut_remap(struct dpp *dpp, struct colorspace_transform *gamut_remap)
{
    struct color_matrices_reg gam_regs;
    uint16_t                  arr_reg_val[12];

    PROGRAM_ENTRY();

    if (!gamut_remap || !gamut_remap->enable_remap ||
        dpp->vpe_priv->init.debug.bypass_dpp_gamut_remap) {
        REG_SET(VPCM_GAMUT_REMAP_CONTROL, 0, VPCM_GAMUT_REMAP_MODE, 0);
        return;
    }

    gam_regs.shifts.csc_c11 = REG_FIELD_SHIFT(VPCM_GAMUT_REMAP_C11);
    gam_regs.masks.csc_c11  = REG_FIELD_MASK(VPCM_GAMUT_REMAP_C11);
    gam_regs.shifts.csc_c12 = REG_FIELD_SHIFT(VPCM_GAMUT_REMAP_C12);
    gam_regs.masks.csc_c12  = REG_FIELD_MASK(VPCM_GAMUT_REMAP_C12);
    gam_regs.csc_c11_c12    = REG_OFFSET(VPCM_GAMUT_REMAP_C11_C12);
    gam_regs.csc_c33_c34    = REG_OFFSET(VPCM_GAMUT_REMAP_C33_C34);

    conv_convert_float_matrix(arr_reg_val, gamut_remap->matrix, 12);

    vpe10_cm_helper_program_color_matrices(config_writer, arr_reg_val, &gam_regs);

    REG_SET(VPCM_GAMUT_REMAP_CONTROL, 0, VPCM_GAMUT_REMAP_MODE, 1);
}

/*program post scaler scs block in dpp CM*/
void vpe10_dpp_program_post_csc(struct dpp *dpp, enum color_space color_space,
    enum input_csc_select input_select, struct vpe_csc_matrix *input_cs)
{
    PROGRAM_ENTRY();
    int             i;
    int             arr_size = sizeof(vpe_input_csc_matrix_fixed) / sizeof(struct vpe_csc_matrix);
    const uint16_t *regval   = NULL;
    struct color_matrices_reg gam_regs;

    if (input_select == INPUT_CSC_SELECT_BYPASS || dpp->vpe_priv->init.debug.bypass_post_csc) {
        REG_SET(VPCM_POST_CSC_CONTROL, 0, VPCM_POST_CSC_MODE, 0);
        return;
    }

    if (input_cs == NULL) {
        for (i = 0; i < arr_size; i++)
            if (vpe_input_csc_matrix_fixed[i].cs == color_space) {
                regval = vpe_input_csc_matrix_fixed[i].regval;
                break;
            }

        if (regval == NULL) {
            VPE_ASSERT(0);
            return;
        }
    } else {
        regval = input_cs->regval;
    }

    /* Always use the only one set of CSC matrix
     */

    gam_regs.shifts.csc_c11 = REG_FIELD_SHIFT(VPCM_POST_CSC_C11);
    gam_regs.masks.csc_c11  = REG_FIELD_MASK(VPCM_POST_CSC_C11);
    gam_regs.shifts.csc_c12 = REG_FIELD_SHIFT(VPCM_POST_CSC_C12);
    gam_regs.masks.csc_c12  = REG_FIELD_MASK(VPCM_POST_CSC_C12);
    gam_regs.csc_c11_c12    = REG_OFFSET(VPCM_POST_CSC_C11_C12);
    gam_regs.csc_c33_c34    = REG_OFFSET(VPCM_POST_CSC_C33_C34);

    vpe10_cm_helper_program_color_matrices(config_writer, regval, &gam_regs);

    REG_SET(VPCM_POST_CSC_CONTROL, 0, VPCM_POST_CSC_MODE, input_select);
}

void vpe10_dpp_set_hdr_multiplier(struct dpp *dpp, uint32_t multiplier)
{
    PROGRAM_ENTRY();

    REG_SET(VPCM_HDR_MULT_COEF, REG_DEFAULT(VPCM_HDR_MULT_COEF), VPCM_HDR_MULT_COEF, multiplier);
}
