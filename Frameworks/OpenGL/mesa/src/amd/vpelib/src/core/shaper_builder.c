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

#include "shaper_builder.h"
#include "custom_fp16.h"
#include "fixed31_32.h"

struct x_axis_config {
    int offset;
    int segments_num;
};

struct point_config {
    int custom_float_x;
    int custom_float_y;
    int custom_float_slope;
};

struct curve_points32 {
    struct point_config red;
    struct point_config green;
    struct point_config blue;
};

struct lut_point {
    int red;
    int green;
    int blue;
    int delta_red;
    int delta_green;
    int delta_blue;
};

struct pwl_parameter2 {
    struct x_axis_config  arr_curve_points[34];
    struct curve_points32 corner_points[2];
    struct lut_point      rgb_resulted[256];
    int                   hw_points_num;
};

struct shaper_setup_out {
    int exp_begin_raw;
    int exp_end_raw;
    int begin_custom_1_6_12;
    int end_custom_0_6_10;
    int end_base_fixed_0_14;
};

static bool calculate_shaper_properties_const_hdr_mult(
    const struct vpe_shaper_setup_in *shaper_in, struct shaper_setup_out *shaper_out)
{
    double                          x;
    struct vpe_custom_float_format2 fmt;
    struct vpe_custom_float_value2  custom_float;
    int                             num_exp;

    bool   ret     = false;
    int    isize   = 1 << 14;
    double divider = isize - 1;
    double x_double_begin;

    double multiplyer = shaper_in->source_luminance / 10000.0 * shaper_in->shaper_in_max;

    fmt.flags.Uint      = 0;
    fmt.flags.bits.sign = 1;
    fmt.mantissaBits    = 12;
    fmt.exponentaBits   = 6;

    x = pow(1.0 / divider, 2.2) * multiplyer;
    if (!vpe_convert_to_custom_float_ex_generic(x, &fmt, &custom_float))
        goto release;
    shaper_out->exp_begin_raw = custom_float.exponenta;

    if (!vpe_from_1_6_12_to_double(false, custom_float.exponenta, 0, &x_double_begin))
        goto release;

    if (!vpe_convert_to_custom_float_generic(
            x_double_begin, &fmt, &shaper_out->begin_custom_1_6_12))
        goto release;

    fmt.flags.bits.sign = 0;
    fmt.mantissaBits    = 10;
    if (!vpe_convert_to_custom_float_ex_generic(multiplyer, &fmt, &custom_float))
        goto release;
    shaper_out->exp_end_raw = custom_float.exponenta;
    if (!vpe_convert_to_custom_float_generic(multiplyer, &fmt, &shaper_out->end_custom_0_6_10))
        goto release;
    shaper_out->end_base_fixed_0_14 = isize - 1;
    num_exp                         = shaper_out->exp_end_raw - shaper_out->exp_begin_raw + 1;
    if (num_exp > 34)
        goto release;
    ret = true;
release:
    return ret;
}

static bool calculate_shaper_properties_variable_hdr_mult(
    const struct vpe_shaper_setup_in *shaper_in, struct shaper_setup_out *shaper_out)
{
    struct vpe_custom_float_format2 fmt;
    struct vpe_custom_float_value2  custom_float;
    int                             num_exp;

    bool   ret            = false;
    int    isize          = 1 << 14;
    double divider        = isize - 1;
    double x_double_begin = 0;

    fmt.flags.Uint    = 0;
    fmt.exponentaBits = 6;
    fmt.mantissaBits  = 10;
    if (!vpe_convert_to_custom_float_ex_generic(shaper_in->shaper_in_max, &fmt, &custom_float))
        goto release;

    if (!vpe_convert_to_custom_float_generic(
            shaper_in->shaper_in_max, &fmt, &shaper_out->end_custom_0_6_10))
        goto release;

    shaper_out->exp_end_raw   = custom_float.exponenta;
    shaper_out->exp_begin_raw = shaper_out->exp_end_raw - 33;

    shaper_out->end_base_fixed_0_14 = isize - 1;

    if (!vpe_from_1_6_12_to_double(false, shaper_out->exp_begin_raw, 0, &x_double_begin))
        goto release;

    fmt.mantissaBits    = 12;
    fmt.flags.bits.sign = 1;

    if (!vpe_convert_to_custom_float_generic(
            x_double_begin, &fmt, &shaper_out->begin_custom_1_6_12))
        goto release;

    num_exp = shaper_out->exp_end_raw - shaper_out->exp_begin_raw + 1;
    if (num_exp > 34)
        goto release;
    ret = true;
release:
    return ret;
}

static int build_shaper_2_2_segments_distribution(int num_regions, int *arr_segments)
{
    int       i;
    int       counter;
    int       num_segments                = 0;
    int       num_segments_total          = 0;
    const int proposed_2_2_distribution[] = {5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 2, 2, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int proposed_regions = ARRAY_SIZE(proposed_2_2_distribution);

    if (proposed_regions < num_regions)
        goto release;
    counter = 0;

    for (i = num_regions - 1; i >= 0; i--) {
        arr_segments[counter] = proposed_2_2_distribution[i];
        num_segments += 1 << proposed_2_2_distribution[i];
        counter++;
    }
release:
    return num_segments;
}

enum vpe_status vpe_build_shaper(
    const struct vpe_shaper_setup_in *shaper_in, struct pwl_params *shaper)
{
    enum vpe_status ret = VPE_STATUS_ERROR;

    int                     num_points = 0;
    int                     arr_regions[34];
    struct shaper_setup_out shaper_params;
    int                     i, j;
    int                     num_exp;

    unsigned int exp;
    double       x, delta_segments;
    int          lut_counter = 0;
    int          segments_current;
    int          segments_offset;

    unsigned int decimalBits = 14;

    unsigned int mask    = (1 << decimalBits) - 1;
    double       d_norm  = mask;
    double       divider = shaper_in->shaper_in_max;

    if (shaper_in->use_const_hdr_mult &&
        !calculate_shaper_properties_const_hdr_mult(shaper_in, &shaper_params))
        goto release;
    else if (!calculate_shaper_properties_variable_hdr_mult(shaper_in, &shaper_params))
        goto release;

    exp = shaper_params.exp_begin_raw;

    num_exp    = shaper_params.exp_end_raw - shaper_params.exp_begin_raw + 1;
    num_points = build_shaper_2_2_segments_distribution(num_exp, arr_regions);

    segments_offset = 0;

    for (i = 0; i < num_exp; i++) {
        segments_current                         = 1 << arr_regions[i];
        shaper->arr_curve_points[i].segments_num = arr_regions[i];
        shaper->arr_curve_points[i].offset       = segments_offset;
        segments_offset                          = segments_offset + segments_current;
        if (!vpe_from_1_6_12_to_double(false, exp, 0, &x))
            goto release;
        x /= divider;
        shaper->rgb_resulted[lut_counter].red_reg =
            vpe_to_fixed_point(decimalBits, x, mask, d_norm);
        shaper->rgb_resulted[lut_counter].green_reg = shaper->rgb_resulted[lut_counter].red_reg;
        shaper->rgb_resulted[lut_counter].blue_reg  = shaper->rgb_resulted[lut_counter].red_reg;

        delta_segments = x / segments_current;
        lut_counter++;
        for (j = 0; j < segments_current - 1; j++) {
            x += delta_segments;
            shaper->rgb_resulted[lut_counter].red_reg =
                vpe_to_fixed_point(decimalBits, x, mask, d_norm);
            shaper->rgb_resulted[lut_counter].green_reg = shaper->rgb_resulted[lut_counter].red_reg;
            shaper->rgb_resulted[lut_counter].blue_reg  = shaper->rgb_resulted[lut_counter].red_reg;
            lut_counter++;
        }
        exp++;
    }

    shaper->corner_points[0].red.custom_float_x   = shaper_params.begin_custom_1_6_12;
    shaper->corner_points[0].green.custom_float_x = shaper->corner_points[0].red.custom_float_x;
    shaper->corner_points[0].blue.custom_float_x  = shaper->corner_points[0].red.custom_float_x;

    shaper->corner_points[1].red.custom_float_x   = shaper_params.end_custom_0_6_10;
    shaper->corner_points[1].green.custom_float_x = shaper->corner_points[1].red.custom_float_x;
    shaper->corner_points[1].blue.custom_float_x  = shaper->corner_points[1].red.custom_float_x;

    shaper->corner_points[1].red.custom_float_y   = shaper_params.end_base_fixed_0_14;
    shaper->corner_points[1].green.custom_float_y = shaper->corner_points[1].red.custom_float_y;
    shaper->corner_points[1].blue.custom_float_y  = shaper->corner_points[1].red.custom_float_y;

    for (i = 1; i < num_points; i++) {
        shaper->rgb_resulted[i - 1].delta_red_reg =
            shaper->rgb_resulted[i].red_reg - shaper->rgb_resulted[i - 1].red_reg;
        shaper->rgb_resulted[i - 1].delta_green_reg = shaper->rgb_resulted[i - 1].delta_red_reg;
        shaper->rgb_resulted[i - 1].delta_blue_reg  = shaper->rgb_resulted[i - 1].delta_red_reg;
    }

    shaper->hw_points_num = num_points;
    ret                   = VPE_STATUS_OK;

release:
    return ret;
}
