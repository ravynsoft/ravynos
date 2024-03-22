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
#include "vpe10_dpp.h"
#include "color.h"
#include "vpe10/inc/vpe10_cm_common.h"
#include "hw_shared.h"
#include "reg_helper.h"

#define CTX_BASE dpp
#define CTX      vpe10_dpp

static struct dpp_funcs vpe10_dpp_funcs = {

    // cnv
    .program_cnv            = vpe10_dpp_program_cnv,
    .program_pre_dgam       = vpe10_dpp_cnv_program_pre_dgam,
    .program_cnv_bias_scale = vpe10_dpp_program_cnv_bias_scale,
    .program_alpha_keyer    = vpe10_dpp_cnv_program_alpha_keyer,
    .program_crc            = vpe10_dpp_program_crc,

    // cm
    .program_input_transfer_func = vpe10_dpp_program_input_transfer_func,
    .program_gamut_remap         = vpe10_dpp_program_gamut_remap,
    .program_post_csc            = vpe10_dpp_program_post_csc,
    .set_hdr_multiplier          = vpe10_dpp_set_hdr_multiplier,

    // scaler
    .get_optimal_number_of_taps  = vpe10_dpp_get_optimal_number_of_taps,
    .dscl_calc_lb_num_partitions = vpe10_dscl_calc_lb_num_partitions,
    .set_segment_scaler          = vpe10_dpp_set_segment_scaler,
    .set_frame_scaler            = vpe10_dpp_set_frame_scaler,
    .get_line_buffer_size        = vpe10_get_line_buffer_size,
    .validate_number_of_taps     = vpe10_dpp_validate_number_of_taps,
};

void vpe10_construct_dpp(struct vpe_priv *vpe_priv, struct dpp *dpp)
{
    dpp->vpe_priv = vpe_priv;
    dpp->funcs    = &vpe10_dpp_funcs;
}

bool vpe10_dpp_get_optimal_number_of_taps(
    struct dpp *dpp, struct scaler_data *scl_data, const struct vpe_scaling_taps *in_taps)
{
    struct vpe_priv *vpe_priv   = dpp->vpe_priv;
    uint32_t         h_taps_min = 0, v_taps_min = 0;
    /*
     * Set default taps if none are provided
     * From programming guide: taps = min{ ceil(2*H_RATIO,1), 8} for downscaling
     * taps = 4 for upscaling
     */
    if (in_taps->h_taps > 8 || in_taps->v_taps > 8 || in_taps->h_taps_c > 8 ||
        in_taps->v_taps_c > 8)
        return false;

    if (vpe_fixpt_ceil(scl_data->ratios.horz) > 1)
        h_taps_min = (uint32_t)max(4, min(2 * vpe_fixpt_ceil(scl_data->ratios.horz), 8));
    else
        h_taps_min = (uint32_t)4;

    if (in_taps->h_taps == 0) {
        scl_data->taps.h_taps = h_taps_min;
    } else {
        if (in_taps->h_taps < h_taps_min)
            return false;

        scl_data->taps.h_taps = in_taps->h_taps;
    }

    if (vpe_fixpt_ceil(scl_data->ratios.vert) > 1)
        v_taps_min =
            (uint32_t)max(4, min(vpe_fixpt_ceil(vpe_fixpt_mul_int(scl_data->ratios.vert, 2)), 8));
    else
        v_taps_min = (uint32_t)4;

    if (in_taps->v_taps == 0) {
        scl_data->taps.v_taps = v_taps_min;
    } else {
        if (in_taps->v_taps < v_taps_min)
            return false;

        scl_data->taps.v_taps = in_taps->v_taps;
    }

    if (in_taps->h_taps_c == 0) {
        // default to 2 as mmd only uses bilinear for chroma
        scl_data->taps.h_taps_c = (uint32_t)2;
    } else
        scl_data->taps.h_taps_c = in_taps->h_taps_c;

    if (in_taps->v_taps_c == 0) {
        // default to 2 as mmd only uses bilinear for chroma
        scl_data->taps.v_taps_c = (uint32_t)2;
    } else
        scl_data->taps.v_taps_c = in_taps->v_taps_c;

    /* taps can be either 1 or an even number */
    if (scl_data->taps.h_taps % 2 && scl_data->taps.h_taps != 1)
        scl_data->taps.h_taps++;

    if (scl_data->taps.v_taps % 2 && scl_data->taps.v_taps != 1)
        scl_data->taps.v_taps++;

    if (scl_data->taps.h_taps_c % 2 && scl_data->taps.h_taps_c != 1)
        scl_data->taps.h_taps_c++;

    if (scl_data->taps.v_taps_c % 2 && scl_data->taps.v_taps_c != 1)
        scl_data->taps.v_taps_c++;

    // bypass scaler if all ratios are 1
    if (IDENTITY_RATIO(scl_data->ratios.horz))
        scl_data->taps.h_taps = 1;
    if (IDENTITY_RATIO(scl_data->ratios.vert))
        scl_data->taps.v_taps = 1;

    return true;
}

void vpe10_dscl_calc_lb_num_partitions(const struct scaler_data *scl_data,
    enum lb_memory_config lb_config, uint32_t *num_part_y, uint32_t *num_part_c)
{
    uint32_t memory_line_size_y, memory_line_size_c, memory_line_size_a, lb_memory_size,
        lb_memory_size_c, lb_memory_size_a, num_partitions_a;

    uint32_t line_size   = scl_data->viewport.width < scl_data->recout.width
                               ? scl_data->viewport.width
                               : scl_data->recout.width;
    uint32_t line_size_c = scl_data->viewport_c.width < scl_data->recout.width
                               ? scl_data->viewport_c.width
                               : scl_data->recout.width;

    if (line_size == 0)
        line_size = 1;

    if (line_size_c == 0)
        line_size_c = 1;

    memory_line_size_y = (line_size + 5) / 6;   /* +5 to ceil */
    memory_line_size_c = (line_size_c + 5) / 6; /* +5 to ceil */
    memory_line_size_a = (line_size + 5) / 6;   /* +5 to ceil */

    // only has 1-piece lb config in vpe1
    lb_memory_size   = 696;
    lb_memory_size_c = 696;
    lb_memory_size_a = 696;

    *num_part_y      = lb_memory_size / memory_line_size_y;
    *num_part_c      = lb_memory_size_c / memory_line_size_c;
    num_partitions_a = lb_memory_size_a / memory_line_size_a;

    if (scl_data->lb_params.alpha_en && (num_partitions_a < *num_part_y))
        *num_part_y = num_partitions_a;

    if (*num_part_y > 12)
        *num_part_y = 12;
    if (*num_part_c > 12)
        *num_part_c = 12;
}

/* Not used as we don't enable prealpha dealpha currently
 * Can skip for optimize performance and use default val
 */
static void vpe10_dpp_program_prealpha_dealpha(struct dpp *dpp)
{
    uint32_t dealpha_en = 0, dealpha_ablnd_en = 0;
    uint32_t realpha_en = 0, realpha_ablnd_en = 0;
    uint32_t program_prealpha_dealpha = 0;
    PROGRAM_ENTRY();

    if (program_prealpha_dealpha) {
        dealpha_en = 1;
        realpha_en = 1;
    }
    REG_SET_2(
        VPCNVC_PRE_DEALPHA, 0, PRE_DEALPHA_EN, dealpha_en, PRE_DEALPHA_ABLND_EN, dealpha_ablnd_en);
    REG_SET_2(
        VPCNVC_PRE_REALPHA, 0, PRE_REALPHA_EN, realpha_en, PRE_REALPHA_ABLND_EN, realpha_ablnd_en);
}

/* Not used as we don't have special 2bit LUt currently
 * Can skip for optimize performance and use default val
 */
static void vpe10_dpp_program_alpha_2bit_lut(
    struct dpp *dpp, struct cnv_alpha_2bit_lut *alpha_2bit_lut)
{
    PROGRAM_ENTRY();

    if (alpha_2bit_lut != NULL) {
        REG_SET_4(VPCNVC_ALPHA_2BIT_LUT, 0, ALPHA_2BIT_LUT0, alpha_2bit_lut->lut0, ALPHA_2BIT_LUT1,
            alpha_2bit_lut->lut1, ALPHA_2BIT_LUT2, alpha_2bit_lut->lut2, ALPHA_2BIT_LUT3,
            alpha_2bit_lut->lut3);
    } else { // restore to default
        REG_SET_DEFAULT(VPCNVC_ALPHA_2BIT_LUT);
    }
}

void vpe10_dpp_program_cnv(
    struct dpp *dpp, enum vpe_surface_pixel_format format, enum vpe_expansion_mode mode)
{
    uint32_t alpha_en     = 1;
    uint32_t pixel_format = 0;
    uint32_t hw_expansion_mode = 0;

    PROGRAM_ENTRY();

    switch (mode) {
    case VPE_EXPANSION_MODE_DYNAMIC:
        hw_expansion_mode = 0;
        break;
    case VPE_EXPANSION_MODE_ZERO:
        hw_expansion_mode = 1;
        break;
    default:
        VPE_ASSERT(0);
        break;
    }

    switch (format) {
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888:
        alpha_en = 0;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888:
        pixel_format = 8;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888:
        alpha_en = 0;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888:
        pixel_format = 9;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010:
        pixel_format = 10;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102:
        pixel_format = 11;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCrCb8888:
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_AYCbCr8888:
        pixel_format = 12;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb:
        pixel_format = 64;
        alpha_en     = 0;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr:
        pixel_format = 65;
        alpha_en     = 0;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCrCb:
        pixel_format = 66;
        alpha_en     = 0;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr:
        pixel_format = 67;
        alpha_en     = 0;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616:
        pixel_format = 22;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR16161616F:
        pixel_format = 24;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA16161616F:
    case VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA16161616F:
        pixel_format = 25;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_ACrYCb2101010:
        pixel_format = 114;
        break;
    case VPE_SURFACE_PIXEL_FORMAT_VIDEO_CrYCbA1010102:
        pixel_format = 115;
        break;
    default:
        break;
    }

    REG_SET(VPCNVC_SURFACE_PIXEL_FORMAT, 0, VPCNVC_SURFACE_PIXEL_FORMAT, pixel_format);

    REG_SET_7(VPCNVC_FORMAT_CONTROL, 0, FORMAT_EXPANSION_MODE, hw_expansion_mode, FORMAT_CNV16, 0,
        FORMAT_CONTROL__ALPHA_EN, alpha_en, VPCNVC_BYPASS, dpp->vpe_priv->init.debug.vpcnvc_bypass,
        VPCNVC_BYPASS_MSB_ALIGN, 0, CLAMP_POSITIVE, 0, CLAMP_POSITIVE_C, 0);
}

void vpe10_dpp_program_cnv_bias_scale(struct dpp *dpp, struct bias_and_scale *bias_and_scale)
{
    PROGRAM_ENTRY();

    REG_SET(VPCNVC_FCNV_FP_BIAS_R, 0, FCNV_FP_BIAS_R, bias_and_scale->bias_red);
    REG_SET(VPCNVC_FCNV_FP_BIAS_G, 0, FCNV_FP_BIAS_G, bias_and_scale->bias_green);
    REG_SET(VPCNVC_FCNV_FP_BIAS_B, 0, FCNV_FP_BIAS_B, bias_and_scale->bias_blue);

    REG_SET(VPCNVC_FCNV_FP_SCALE_R, 0, FCNV_FP_SCALE_R, bias_and_scale->scale_red);
    REG_SET(VPCNVC_FCNV_FP_SCALE_G, 0, FCNV_FP_SCALE_G, bias_and_scale->scale_green);
    REG_SET(VPCNVC_FCNV_FP_SCALE_B, 0, FCNV_FP_SCALE_B, bias_and_scale->scale_blue);
}

void vpe10_dpp_cnv_program_pre_dgam(struct dpp *dpp, enum color_transfer_func tr)
{
    int pre_degam_en          = 1;
    int degamma_lut_selection = 0;

    PROGRAM_ENTRY();

    switch (tr) {
    case TRANSFER_FUNC_LINEAR_0_125:
        pre_degam_en = 0; // bypass
        break;
    case TRANSFER_FUNC_SRGB:
        degamma_lut_selection = 0;
        break;
    case TRANSFER_FUNC_BT709:
        degamma_lut_selection = 4;
        break;
    case TRANSFER_FUNC_PQ2084:
        degamma_lut_selection = 5;
        break;
    default:
        pre_degam_en = 0;
        break;
    }

    REG_SET_2(
        VPCNVC_PRE_DEGAM, 0, PRE_DEGAM_MODE, pre_degam_en, PRE_DEGAM_SELECT, degamma_lut_selection);
}

void vpe10_dpp_cnv_program_alpha_keyer(struct dpp *dpp, struct cnv_color_keyer_params *color_keyer)
{
    PROGRAM_ENTRY();

    REG_SET_2(VPCNVC_COLOR_KEYER_CONTROL, 0, COLOR_KEYER_EN, color_keyer->color_keyer_en,
        COLOR_KEYER_MODE, color_keyer->color_keyer_mode);

    REG_SET_2(VPCNVC_COLOR_KEYER_ALPHA, 0, COLOR_KEYER_ALPHA_LOW,
        color_keyer->color_keyer_alpha_low, COLOR_KEYER_ALPHA_HIGH,
        color_keyer->color_keyer_alpha_high);

    REG_SET_2(VPCNVC_COLOR_KEYER_RED, 0, COLOR_KEYER_RED_LOW, color_keyer->color_keyer_red_low,
        COLOR_KEYER_RED_HIGH, color_keyer->color_keyer_red_high);

    REG_SET_2(VPCNVC_COLOR_KEYER_GREEN, 0, COLOR_KEYER_GREEN_LOW,
        color_keyer->color_keyer_green_low, COLOR_KEYER_GREEN_HIGH,
        color_keyer->color_keyer_green_high);

    REG_SET_2(VPCNVC_COLOR_KEYER_BLUE, 0, COLOR_KEYER_BLUE_LOW, color_keyer->color_keyer_blue_low,
        COLOR_KEYER_BLUE_HIGH, color_keyer->color_keyer_blue_high);
}

uint32_t vpe10_get_line_buffer_size()
{
    return MAX_LINE_SIZE * MAX_LINE_CNT;
}

bool vpe10_dpp_validate_number_of_taps(struct dpp *dpp, struct scaler_data *scl_data)
{
    uint32_t num_part_y, num_part_c;
    uint32_t max_taps_y, max_taps_c;
    uint32_t min_taps_y, min_taps_c;

    /*Ensure we can support the requested number of vtaps*/
    min_taps_y = (uint32_t)vpe_fixpt_ceil(scl_data->ratios.vert);
    min_taps_c = (uint32_t)vpe_fixpt_ceil(scl_data->ratios.vert_c);

    dpp->funcs->dscl_calc_lb_num_partitions(scl_data, LB_MEMORY_CONFIG_1, &num_part_y, &num_part_c);

    /* MAX_V_TAPS = MIN (NUM_LINES - MAX(CEILING(V_RATIO,1)-2, 0), 8) */
    if (vpe_fixpt_ceil(scl_data->ratios.vert) > 2)
        max_taps_y = num_part_y - ((uint32_t)vpe_fixpt_ceil(scl_data->ratios.vert) - 2);
    else
        max_taps_y = num_part_y;

    if (vpe_fixpt_ceil(scl_data->ratios.vert_c) > 2)
        max_taps_c = num_part_c - ((uint32_t)vpe_fixpt_ceil(scl_data->ratios.vert_c) - 2);
    else
        max_taps_c = num_part_c;

    if (max_taps_y < min_taps_y)
        return false;
    else if (max_taps_c < min_taps_c)
        return false;

    if (scl_data->taps.v_taps > max_taps_y)
        scl_data->taps.v_taps = max_taps_y;

    if (scl_data->taps.v_taps_c > max_taps_c)
        scl_data->taps.v_taps_c = max_taps_c;

    if (IDENTITY_RATIO(scl_data->ratios.vert))
        scl_data->taps.v_taps = 1;

    if (scl_data->taps.v_taps % 2 && scl_data->taps.v_taps != 1)
        scl_data->taps.v_taps++;

    if (scl_data->taps.v_taps_c % 2 && scl_data->taps.v_taps_c != 1)
        scl_data->taps.v_taps_c++;

    return true;
}

void vpe10_dpp_program_crc(struct dpp *dpp, bool enable)
{
    PROGRAM_ENTRY();
    REG_UPDATE(VPDPP_CRC_CTRL, VPDPP_CRC_EN, enable);
}

