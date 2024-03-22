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
#include <stdint.h>
#include <string.h>
#include "vpe10_cm_common.h"
#include "custom_float.h"
#include "reg_helper.h"

#define CTX_BASE dpp
#define CTX      vpe10_dpp

static bool cm_helper_convert_to_custom_float(struct pwl_result_data *rgb_resulted,
    struct curve_points3 *corner_points, uint32_t hw_points_num, bool fixpoint)
{
    struct custom_float_format fmt = {0};

    struct pwl_result_data *rgb = rgb_resulted;

    uint32_t i = 0;

    fmt.exponenta_bits = 6;
    fmt.mantissa_bits  = 12;
    fmt.sign           = false;

    /* corner_points[0] - beginning base, slope offset for R,G,B
     * corner_points[1] - end base, slope offset for R,G,B
     */
    if (!vpe_convert_to_custom_float_format(
            corner_points[0].red.x, &fmt, &corner_points[0].red.custom_float_x)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[0].green.x, &fmt, &corner_points[0].green.custom_float_x)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[0].blue.x, &fmt, &corner_points[0].blue.custom_float_x)) {
        VPE_ASSERT(0);
        return false;
    }

    if (!vpe_convert_to_custom_float_format(
            corner_points[0].red.offset, &fmt, &corner_points[0].red.custom_float_offset)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[0].green.offset, &fmt, &corner_points[0].green.custom_float_offset)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[0].blue.offset, &fmt, &corner_points[0].blue.custom_float_offset)) {
        VPE_ASSERT(0);
        return false;
    }

    if (!vpe_convert_to_custom_float_format(
            corner_points[0].red.slope, &fmt, &corner_points[0].red.custom_float_slope)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[0].green.slope, &fmt, &corner_points[0].green.custom_float_slope)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[0].blue.slope, &fmt, &corner_points[0].blue.custom_float_slope)) {
        VPE_ASSERT(0);
        return false;
    }

    if (fixpoint == true) {
        corner_points[1].red.custom_float_y   = vpe_fixpt_clamp_u0d14(corner_points[1].red.y);
        corner_points[1].green.custom_float_y = vpe_fixpt_clamp_u0d14(corner_points[1].green.y);
        corner_points[1].blue.custom_float_y  = vpe_fixpt_clamp_u0d14(corner_points[1].blue.y);
    } else {
        if (!vpe_convert_to_custom_float_format(
                corner_points[1].red.y, &fmt, &corner_points[1].red.custom_float_y)) {
            VPE_ASSERT(0);
            return false;
        }
        if (!vpe_convert_to_custom_float_format(
                corner_points[1].green.y, &fmt, &corner_points[1].green.custom_float_y)) {
            VPE_ASSERT(0);
            return false;
        }
        if (!vpe_convert_to_custom_float_format(
                corner_points[1].blue.y, &fmt, &corner_points[1].blue.custom_float_y)) {
            VPE_ASSERT(0);
            return false;
        }
    }

    fmt.mantissa_bits = 10;
    fmt.sign          = false;

    if (!vpe_convert_to_custom_float_format(
            corner_points[1].red.x, &fmt, &corner_points[1].red.custom_float_x)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[1].green.x, &fmt, &corner_points[1].green.custom_float_x)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[1].blue.x, &fmt, &corner_points[1].blue.custom_float_x)) {
        VPE_ASSERT(0);
        return false;
    }

    if (!vpe_convert_to_custom_float_format(
            corner_points[1].red.slope, &fmt, &corner_points[1].red.custom_float_slope)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[1].green.slope, &fmt, &corner_points[1].green.custom_float_slope)) {
        VPE_ASSERT(0);
        return false;
    }
    if (!vpe_convert_to_custom_float_format(
            corner_points[1].blue.slope, &fmt, &corner_points[1].blue.custom_float_slope)) {
        VPE_ASSERT(0);
        return false;
    }

    if (hw_points_num == 0 || rgb_resulted == NULL || fixpoint == true)
        return true;

    fmt.mantissa_bits = 12;

    while (i != hw_points_num) {
        if (!vpe_convert_to_custom_float_format(rgb->red, &fmt, &rgb->red_reg)) {
            VPE_ASSERT(0);
            return false;
        }

        if (!vpe_convert_to_custom_float_format(rgb->green, &fmt, &rgb->green_reg)) {
            VPE_ASSERT(0);
            return false;
        }

        if (!vpe_convert_to_custom_float_format(rgb->blue, &fmt, &rgb->blue_reg)) {
            VPE_ASSERT(0);
            return false;
        }

        if (!vpe_convert_to_custom_float_format(rgb->delta_red, &fmt, &rgb->delta_red_reg)) {
            VPE_ASSERT(0);
            return false;
        }

        if (!vpe_convert_to_custom_float_format(rgb->delta_green, &fmt, &rgb->delta_green_reg)) {
            VPE_ASSERT(0);
            return false;
        }

        if (!vpe_convert_to_custom_float_format(rgb->delta_blue, &fmt, &rgb->delta_blue_reg)) {
            VPE_ASSERT(0);
            return false;
        }

        ++rgb;
        ++i;
    }

    return true;
}

/* driver uses 32 regions or less, but DCN HW has 34, extra 2 are set to 0 */
#define MAX_REGIONS_NUMBER 34
#define MAX_LOW_POINT      25
#define NUMBER_REGIONS     32
#define NUMBER_SW_SEGMENTS 16

bool vpe10_cm_helper_translate_curve_to_hw_format(
    const struct transfer_func *output_tf, struct pwl_params *lut_params, bool fixpoint)
{
    struct curve_points3   *corner_points;
    struct pwl_result_data *rgb_resulted;
    struct pwl_result_data *rgb;
    struct pwl_result_data *rgb_plus_1;
    struct pwl_result_data *rgb_minus_1;

    int32_t  region_start, region_end;
    int32_t  i;
    uint32_t j, k, seg_distr[MAX_REGIONS_NUMBER], increment, start_index, hw_points;

    if (output_tf == NULL || lut_params == NULL || output_tf->type == TF_TYPE_BYPASS)
        return false;

    corner_points = lut_params->corner_points;
    rgb_resulted  = lut_params->rgb_resulted;
    hw_points     = 0;

    memset(lut_params, 0, sizeof(struct pwl_params));
    memset(seg_distr, 0, sizeof(seg_distr));

    if (output_tf->tf == TRANSFER_FUNC_PQ2084) {

        for (i = 0; i < MAX_LOW_POINT; i++)
            seg_distr[i] = 3;

        // Extra magic point to account for incorrect programming of the lut
        seg_distr[i] = 1;
        region_start = -MAX_LOW_POINT;
        region_end   = 1;
    } else if (output_tf->tf == TRANSFER_FUNC_LINEAR_0_125) {

        int num_regions_linear = MAX_LOW_POINT + 3;

        for (i = 0; i < num_regions_linear; i++)
            seg_distr[i] = 3;

        region_start = -MAX_LOW_POINT;
        region_end   = 3;
    } else {
        seg_distr[0]  = 3;
        seg_distr[1]  = 4;
        seg_distr[2]  = 4;
        seg_distr[3]  = 4;
        seg_distr[4]  = 4;
        seg_distr[5]  = 4;
        seg_distr[6]  = 4;
        seg_distr[7]  = 4;
        seg_distr[8]  = 4;
        seg_distr[9]  = 4;
        seg_distr[10] = 4;
        seg_distr[11] = 4;
        seg_distr[12] = 1;

        region_start = -12;
        region_end   = 1;
    }

    for (i = region_end - region_start; i < MAX_REGIONS_NUMBER; i++)
        seg_distr[i] = (uint32_t)-1;

    for (k = 0; k < MAX_REGIONS_NUMBER; k++) {
        if (seg_distr[k] != (uint32_t)-1)
            hw_points += (1 << seg_distr[k]);
    }

    j = 0;
    for (k = 0; k < (uint32_t)(region_end - region_start); k++) {
        increment   = NUMBER_SW_SEGMENTS / (1 << seg_distr[k]);
        start_index = ((uint32_t)region_start + k + MAX_LOW_POINT) * NUMBER_SW_SEGMENTS;
        for (i = (int32_t)start_index; i < (int32_t)start_index + NUMBER_SW_SEGMENTS;
             i += increment) {
            if (j == hw_points - 1)
                break;
            rgb_resulted[j].red   = output_tf->tf_pts.red[i];
            rgb_resulted[j].green = output_tf->tf_pts.green[i];
            rgb_resulted[j].blue  = output_tf->tf_pts.blue[i];
            j++;
        }
    }

    /* last point */
    start_index                     = (uint32_t)((region_end + MAX_LOW_POINT) * NUMBER_SW_SEGMENTS);
    rgb_resulted[hw_points - 1].red = output_tf->tf_pts.red[start_index];
    rgb_resulted[hw_points - 1].green = output_tf->tf_pts.green[start_index];
    rgb_resulted[hw_points - 1].blue  = output_tf->tf_pts.blue[start_index];

    rgb_resulted[hw_points].red   = rgb_resulted[hw_points - 1].red;
    rgb_resulted[hw_points].green = rgb_resulted[hw_points - 1].green;
    rgb_resulted[hw_points].blue  = rgb_resulted[hw_points - 1].blue;

    // All 3 color channels have same x
    corner_points[0].red.x = vpe_fixpt_pow(vpe_fixpt_from_int(2), vpe_fixpt_from_int(region_start));
    corner_points[0].green.x = corner_points[0].red.x;
    corner_points[0].blue.x  = corner_points[0].red.x;

    corner_points[1].red.x   = vpe_fixpt_pow(vpe_fixpt_from_int(2), vpe_fixpt_from_int(region_end));
    corner_points[1].green.x = corner_points[1].red.x;
    corner_points[1].blue.x  = corner_points[1].red.x;

    corner_points[0].red.y   = rgb_resulted[0].red;
    corner_points[0].green.y = rgb_resulted[0].green;
    corner_points[0].blue.y  = rgb_resulted[0].blue;

    corner_points[0].red.slope = vpe_fixpt_div(corner_points[0].red.y, corner_points[0].red.x);
    corner_points[0].green.slope =
        vpe_fixpt_div(corner_points[0].green.y, corner_points[0].green.x);
    corner_points[0].blue.slope = vpe_fixpt_div(corner_points[0].blue.y, corner_points[0].blue.x);

    /* see comment above, m_arrPoints[1].y should be the Y value for the
     * region end (m_numOfHwPoints), not last HW point(m_numOfHwPoints - 1)
     */
    corner_points[1].red.y       = rgb_resulted[hw_points - 1].red;
    corner_points[1].green.y     = rgb_resulted[hw_points - 1].green;
    corner_points[1].blue.y      = rgb_resulted[hw_points - 1].blue;
    corner_points[1].red.slope   = vpe_fixpt_zero;
    corner_points[1].green.slope = vpe_fixpt_zero;
    corner_points[1].blue.slope  = vpe_fixpt_zero;

    lut_params->hw_points_num = hw_points;

    k = 0;
    for (i = 1; i < MAX_REGIONS_NUMBER; i++) {
        if (seg_distr[k] != (uint32_t)-1) {
            lut_params->arr_curve_points[k].segments_num = seg_distr[k];
            lut_params->arr_curve_points[i].offset =
                lut_params->arr_curve_points[k].offset + (1 << seg_distr[k]);
        }
        k++;
    }

    if (seg_distr[k] != (uint32_t)-1)
        lut_params->arr_curve_points[k].segments_num = seg_distr[k];

    rgb         = rgb_resulted;
    rgb_plus_1  = rgb_resulted + 1;
    rgb_minus_1 = rgb;

    i = 1;
    while (i != (int32_t)(hw_points + 1)) {
        if (i >= (int32_t)(hw_points - 1)) {
            if (vpe_fixpt_lt(rgb_plus_1->red, rgb->red))
                rgb_plus_1->red = vpe_fixpt_add(rgb->red, rgb_minus_1->delta_red);
            if (vpe_fixpt_lt(rgb_plus_1->green, rgb->green))
                rgb_plus_1->green = vpe_fixpt_add(rgb->green, rgb_minus_1->delta_green);
            if (vpe_fixpt_lt(rgb_plus_1->blue, rgb->blue))
                rgb_plus_1->blue = vpe_fixpt_add(rgb->blue, rgb_minus_1->delta_blue);
        }

        rgb->delta_red   = vpe_fixpt_sub(rgb_plus_1->red, rgb->red);
        rgb->delta_green = vpe_fixpt_sub(rgb_plus_1->green, rgb->green);
        rgb->delta_blue  = vpe_fixpt_sub(rgb_plus_1->blue, rgb->blue);

        if (fixpoint == true) {
            rgb->delta_red_reg   = vpe_fixpt_clamp_u0d10(rgb->delta_red);
            rgb->delta_green_reg = vpe_fixpt_clamp_u0d10(rgb->delta_green);
            rgb->delta_blue_reg  = vpe_fixpt_clamp_u0d10(rgb->delta_blue);
            rgb->red_reg         = vpe_fixpt_clamp_u0d14(rgb->red);
            rgb->green_reg       = vpe_fixpt_clamp_u0d14(rgb->green);
            rgb->blue_reg        = vpe_fixpt_clamp_u0d14(rgb->blue);
        }

        ++rgb_plus_1;
        rgb_minus_1 = rgb;
        ++rgb;
        ++i;
    }
    cm_helper_convert_to_custom_float(rgb_resulted, lut_params->corner_points, hw_points, fixpoint);

    return true;
}

#define NUM_DEGAMMA_REGIONS        9
#define MAX_REGIONS_NUMBER_DEGAMMA 16
#define MAX_HW_POINTS_DEGAMMA      257

bool vpe10_cm_helper_translate_curve_to_degamma_hw_format(
    const struct transfer_func *output_tf, struct pwl_params *lut_params)
{
    struct curve_points3   *corner_points;
    struct pwl_result_data *rgb_resulted;
    struct pwl_result_data *rgb;
    struct pwl_result_data *rgb_plus_1;

    int32_t  region_start, region_end;
    int32_t  i;
    uint32_t k, seg_distr[MAX_REGIONS_NUMBER_DEGAMMA], num_segments, hw_points;

    if (output_tf == NULL || lut_params == NULL || output_tf->type == TF_TYPE_BYPASS)
        return false;

    corner_points = lut_params->corner_points;
    rgb_resulted  = lut_params->rgb_resulted;
    num_segments  = 0;

    memset(lut_params, 0, sizeof(struct pwl_params));
    memset(seg_distr, 0, sizeof(seg_distr));

    region_start = -NUM_DEGAMMA_REGIONS;
    region_end   = 0;

    for (i = 0; i < MAX_HW_POINTS_DEGAMMA; i++) {
        rgb_resulted[i].red   = output_tf->tf_pts.red[i];
        rgb_resulted[i].green = output_tf->tf_pts.green[i];
        rgb_resulted[i].blue  = output_tf->tf_pts.blue[i];
    }

    for (k = (uint32_t)(region_end - region_start); k < MAX_REGIONS_NUMBER_DEGAMMA; k++)
        seg_distr[k] = (uint32_t)-1;

    /* 9 segments
     * segments are from 2^-8 to 0
     */
    seg_distr[0] = 0; /* Since we only have one point in last region */
    num_segments += 1;

    for (k = 1; k < NUM_DEGAMMA_REGIONS; k++) {
        seg_distr[k] = k - 1; /* Depends upon the regions' points 2^n; seg_distr = n */
        num_segments += (1 << seg_distr[k]);
    }
    hw_points = num_segments + 1;

    corner_points[0].red.x = vpe_fixpt_pow(vpe_fixpt_from_int(2), vpe_fixpt_from_int(region_start));
    corner_points[0].green.x     = corner_points[0].red.x;
    corner_points[0].blue.x      = corner_points[0].red.x;
    corner_points[0].red.y       = rgb_resulted[0].red;
    corner_points[0].green.y     = rgb_resulted[0].green;
    corner_points[0].blue.y      = rgb_resulted[0].blue;
    corner_points[0].red.slope   = vpe_fixpt_div(corner_points[0].red.y, corner_points[0].red.x);
    corner_points[0].green.slope = corner_points[0].red.slope;
    corner_points[0].blue.slope  = corner_points[0].red.slope;

    corner_points[1].red.x   = vpe_fixpt_pow(vpe_fixpt_from_int(2), vpe_fixpt_from_int(region_end));
    corner_points[1].green.x = corner_points[1].red.x;
    corner_points[1].blue.x  = corner_points[1].red.x;

    corner_points[1].red.y       = rgb_resulted[num_segments].red;
    corner_points[1].green.y     = rgb_resulted[num_segments].green;
    corner_points[1].blue.y      = rgb_resulted[num_segments].blue;
    corner_points[1].red.slope   = vpe_fixpt_zero;
    corner_points[1].green.slope = vpe_fixpt_zero;
    corner_points[1].blue.slope  = vpe_fixpt_zero;

    // The number of HW points is equal to num_segments+1, however due to bug in lower layer, it
    // must be set to num_segments
    lut_params->hw_points_num = num_segments;

    lut_params->arr_curve_points[0].segments_num = seg_distr[0];
    for (i = 1; i < NUM_DEGAMMA_REGIONS; i++) {
        lut_params->arr_curve_points[i].segments_num = seg_distr[i];
        lut_params->arr_curve_points[i].offset =
            lut_params->arr_curve_points[i - 1].offset + (1 << seg_distr[i - 1]);
    }

    if (seg_distr[i] != (uint32_t)-1)
        lut_params->arr_curve_points[k].segments_num = seg_distr[k];

    rgb        = rgb_resulted;
    rgb_plus_1 = rgb_resulted + 1;

    i = 1;
    while (i != (int32_t)(hw_points)) {
        if (vpe_fixpt_lt(rgb_plus_1->red, rgb->red))
            rgb_plus_1->red = rgb->red;
        if (vpe_fixpt_lt(rgb_plus_1->green, rgb->green))
            rgb_plus_1->green = rgb->green;
        if (vpe_fixpt_lt(rgb_plus_1->blue, rgb->blue))
            rgb_plus_1->blue = rgb->blue;

        rgb->delta_red   = vpe_fixpt_sub(rgb_plus_1->red, rgb->red);
        rgb->delta_green = vpe_fixpt_sub(rgb_plus_1->green, rgb->green);
        rgb->delta_blue  = vpe_fixpt_sub(rgb_plus_1->blue, rgb->blue);

        ++rgb_plus_1;
        ++rgb;
        ++i;
    }

    cm_helper_convert_to_custom_float(rgb_resulted, lut_params->corner_points, hw_points, false);

    return true;
}

void vpe10_cm_get_tf_pwl_params(
    const struct transfer_func *output_tf, struct pwl_params **lut_params, enum cm_type vpe_cm_type)
{
    int table_index = 0;

    switch (output_tf->tf) {
    case TRANSFER_FUNC_SRGB:
        table_index = 0;
        break;
    case TRANSFER_FUNC_BT1886:
        table_index = 1;
        break;
    case TRANSFER_FUNC_PQ2084:
        table_index = 2;
        break;
    case TRANSFER_FUNC_BT709:
    case TRANSFER_FUNC_LINEAR_0_125:
        table_index = 3;
        break;
    default:
        *lut_params = NULL;
        return;
    }
    *lut_params = &tf_pwl_param_table[vpe_cm_type][table_index];
}

#define REG_FIELD_VALUE_CM(field, value)                                                           \
    ((uint32_t)((value) << reg->shifts.field) & reg->masks.field)
#define REG_FIELD_MASK_CM(field) reg->masks.field

#define REG_SET_CM(reg_offset, init_val, field, val)                                               \
    do {                                                                                           \
        config_writer_fill(                                                                        \
            config_writer, VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_DATA_SIZE, 0) |                        \
                               VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_REGISTER_OFFSET, reg_offset));     \
        config_writer_fill(config_writer,                                                          \
            ((init_val & ~(REG_FIELD_MASK_CM(field))) | REG_FIELD_VALUE_CM(field, val)));          \
    } while (0)

#define REG_SET_2_CM(reg_offset, init_val, f1, v1, f2, v2)                                         \
    do {                                                                                           \
        config_writer_fill(                                                                        \
            config_writer, VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_DATA_SIZE, 0) |                        \
                               VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_REGISTER_OFFSET, reg_offset));     \
        config_writer_fill(                                                                        \
            config_writer, ((init_val & ~(REG_FIELD_MASK_CM(f1)) & ~(REG_FIELD_MASK_CM(f2))) |     \
                               REG_FIELD_VALUE_CM(f1, v1) | REG_FIELD_VALUE_CM(f2, v2)));          \
    } while (0)

#define REG_SET_4_CM(reg_offset, init_val, f1, v1, f2, v2, f3, v3, f4, v4)                         \
    do {                                                                                           \
        config_writer_fill(                                                                        \
            config_writer, VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_DATA_SIZE, 0) |                        \
                               VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_REGISTER_OFFSET, reg_offset));     \
        config_writer_fill(                                                                        \
            config_writer, ((init_val & ~(REG_FIELD_MASK_CM(f1)) & ~(REG_FIELD_MASK_CM(f2)) &      \
                                ~(REG_FIELD_MASK_CM(f3)) & ~(REG_FIELD_MASK_CM(f4))) |             \
                               REG_FIELD_VALUE_CM(f1, v1) | REG_FIELD_VALUE_CM(f2, v2) |           \
                               REG_FIELD_VALUE_CM(f3, v3) | REG_FIELD_VALUE_CM(f4, v4)));          \
    } while (0)

void vpe10_cm_helper_program_gamcor_xfer_func(struct config_writer *config_writer,
    const struct pwl_params *params, const struct vpe10_xfer_func_reg *reg)
{
    // Total: 13 * 4 + (region_end - region_start + 4) = 13*4 + 68 = 120 bytes
    uint32_t     reg_region_cur;
    unsigned int i                = 0;
    uint16_t     packet_data_size = (uint16_t)((reg->region_end - reg->region_start + 1));

    REG_SET_2_CM(reg->start_cntl_b, 0, exp_region_start,
        params->corner_points[0].blue.custom_float_x, exp_region_start_segment, 0);
    REG_SET_2_CM(reg->start_cntl_g, 0, exp_region_start,
        params->corner_points[0].green.custom_float_x, exp_region_start_segment, 0);
    REG_SET_2_CM(reg->start_cntl_r, 0, exp_region_start,
        params->corner_points[0].red.custom_float_x, exp_region_start_segment, 0);

    REG_SET_CM(reg->start_slope_cntl_b, 0, // linear slope at start of curve
        field_region_linear_slope, params->corner_points[0].blue.custom_float_slope);
    REG_SET_CM(reg->start_slope_cntl_g, 0, field_region_linear_slope,
        params->corner_points[0].green.custom_float_slope);
    REG_SET_CM(reg->start_slope_cntl_r, 0, field_region_linear_slope,
        params->corner_points[0].red.custom_float_slope);

    REG_SET_CM(reg->start_end_cntl1_b, 0, field_region_end_base,
        params->corner_points[1].blue.custom_float_y);
    REG_SET_CM(reg->start_end_cntl1_g, 0, field_region_end_base,
        params->corner_points[1].green.custom_float_y);
    REG_SET_CM(reg->start_end_cntl1_r, 0, field_region_end_base,
        params->corner_points[1].red.custom_float_y);

    REG_SET_2_CM(reg->start_end_cntl2_b, 0, field_region_end_slope,
        params->corner_points[1].blue.custom_float_slope, field_region_end,
        params->corner_points[1].blue.custom_float_x);
    REG_SET_2_CM(reg->start_end_cntl2_g, 0, field_region_end_slope,
        params->corner_points[1].green.custom_float_slope, field_region_end,
        params->corner_points[1].green.custom_float_x);
    REG_SET_2_CM(reg->start_end_cntl2_r, 0, field_region_end_slope,
        params->corner_points[1].red.custom_float_slope, field_region_end,
        params->corner_points[1].red.custom_float_x);

    // program all the *GAM_RAM?_REGION_start ~ region_end regs in one VPEP_DIRECT_CONFIG packet
    // with auto inc
    config_writer_fill(
        config_writer, VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_DATA_SIZE, packet_data_size - 1) |
                           VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_REGISTER_OFFSET, reg->region_start) |
                           0x01); // auto increase on

    for (reg_region_cur = reg->region_start; reg_region_cur <= reg->region_end; reg_region_cur++) {

        const struct gamma_curve *curve0 = &(params->arr_curve_points[2 * i]);
        const struct gamma_curve *curve1 = &(params->arr_curve_points[(2 * i) + 1]);

        config_writer_fill(
            config_writer, (((curve0->offset << reg->shifts.exp_region0_lut_offset) &
                                reg->masks.exp_region0_lut_offset) |
                               ((curve0->segments_num << reg->shifts.exp_region0_num_segments) &
                                   reg->masks.exp_region0_num_segments) |
                               ((curve1->offset << reg->shifts.exp_region1_lut_offset) &
                                   reg->masks.exp_region1_lut_offset) |
                               ((curve1->segments_num << reg->shifts.exp_region1_num_segments) &
                                   reg->masks.exp_region1_num_segments)));

        i++;
    }
}

void vpe10_cm_helper_program_pwl(struct config_writer *config_writer,
    const struct pwl_result_data *rgb, uint32_t last_base_value, uint32_t num,
    uint32_t lut_data_reg_offset, uint8_t lut_data_reg_shift, uint32_t lut_data_reg_mask,
    enum cm_rgb_channel channel)
{
    uint32_t i;
    uint32_t lut_data = 0;

    // For LUT, we keep write the same address with entire LUT data, so don't set INC bit
    config_writer_fill(
        config_writer, VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_DATA_SIZE, num) |
                           VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_REGISTER_OFFSET, lut_data_reg_offset));

    for (i = 0; i < num; i++) {
        switch (channel) {
        case CM_PWL_R:
            lut_data = rgb[i].red_reg;
            break;
        case CM_PWL_G:
            lut_data = rgb[i].green_reg;
            break;
        case CM_PWL_B:
            lut_data = rgb[i].blue_reg;
            break;
        }
        config_writer_fill(config_writer, ((lut_data << lut_data_reg_shift) & lut_data_reg_mask));
    }

    config_writer_fill(
        config_writer, ((last_base_value << lut_data_reg_shift) & lut_data_reg_mask));
}

void vpe10_cm_helper_program_color_matrices(struct config_writer *config_writer,
    const uint16_t *regval, const struct color_matrices_reg *reg)
{
    uint32_t     cur_csc_reg;
    unsigned int i                = 0;
    uint16_t     packet_data_size = (uint16_t)((reg->csc_c33_c34 - reg->csc_c11_c12 + 1));

    config_writer_fill(
        config_writer, VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_DATA_SIZE, packet_data_size - 1) |
                           VPEC_FIELD_VALUE(VPE_DIR_CFG_PKT_REGISTER_OFFSET, reg->csc_c11_c12) |
                           0x01); // auto increase on

    for (cur_csc_reg = reg->csc_c11_c12; cur_csc_reg <= reg->csc_c33_c34; cur_csc_reg++) {

        const uint16_t *regval0 = &(regval[2 * i]);
        const uint16_t *regval1 = &(regval[(2 * i) + 1]);

        // use C11/C12 mask value for all CSC regs to ease programing
        config_writer_fill(
            config_writer, ((uint32_t)(*regval0 << reg->shifts.csc_c11) & reg->masks.csc_c11) |
                               ((uint32_t)(*regval1 << reg->shifts.csc_c12) & reg->masks.csc_c12));

        // Due to the program nature of CSC regs are switchable to different sets
        // Skip record REG_IS_WRITTEN and LAST_WRITTEN_VAL used in REG_SET* macros.
        // and those CSC regs will always write at once for all fields

        i++;
    }
}
