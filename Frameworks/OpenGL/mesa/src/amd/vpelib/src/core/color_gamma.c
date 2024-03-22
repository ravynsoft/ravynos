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
#include "color.h"
#include "color_gamma.h"
#include "hw_shared.h"

#define PRECISE_LUT_REGION_START 224
#define PRECISE_LUT_REGION_END   239

static struct hw_x_point coordinates_x[MAX_HW_POINTS + 2];
static struct hw_x_point coordinates_x_degamma[MAX_HW_POINTS_DEGAMMA];

// these are helpers for calculations to reduce stack usage
// do not depend on these being preserved across calls

/* Helper to optimize gamma calculation, only use in translate_from_linear, in
 * particular the vpe_fixpt_pow function which is very expensive
 * The idea is that our regions for X points are exponential and currently they all use
 * the same number of points (NUM_PTS_IN_REGION) and in each region every point
 * is exactly 2x the one at the same index in the previous region. In other words
 * X[i] = 2 * X[i-NUM_PTS_IN_REGION] for i>=16
 * The other fact is that (2x)^gamma = 2^gamma * x^gamma
 * So we compute and save x^gamma for the first 16 regions, and for every next region
 * just multiply with 2^gamma which can be computed once, and save the result so we
 * recursively compute all the values.
 */

/*
 * Regamma coefficients are used for both regamma and degamma. Degamma
 * coefficients are calculated in our formula using the regamma coefficients.
 */
/*sRGB     709     2.2 2.4 P3*/
static const int32_t numerator01[] = {31308, 180000, 0, 0, 0};
static const int32_t numerator02[] = {12920, 4500, 0, 0, 0};
static const int32_t numerator03[] = {55, 99, 0, 0, 0};
static const int32_t numerator04[] = {55, 99, 0, 0, 0};
static const int32_t numerator05[] = {
    2400, 2222, 2200, 2400, 2600}; // the standard REC 709 states 0.45. Inverse of that is 2.22

                                   /* one-time setup of X points */
void vpe_color_setup_x_points_distribution(void)
{
    struct fixed31_32 region_size = vpe_fixpt_from_int(128);
    int32_t           segment;
    uint32_t          seg_offset;
    uint32_t          index;
    struct fixed31_32 increment;

    coordinates_x[MAX_HW_POINTS].x     = region_size;
    coordinates_x[MAX_HW_POINTS + 1].x = region_size;

    for (segment = 6; segment > (6 - NUM_REGIONS); segment--) {
        region_size = vpe_fixpt_div_int(region_size, 2);
        increment   = vpe_fixpt_div_int(region_size, NUM_PTS_IN_REGION);
        seg_offset  = (uint32_t)((segment + (NUM_REGIONS - 7)) * NUM_PTS_IN_REGION);

        coordinates_x[seg_offset].x = region_size;

        for (index = seg_offset + 1; index < seg_offset + NUM_PTS_IN_REGION; index++) {
            coordinates_x[index].x = vpe_fixpt_add(coordinates_x[index - 1].x, increment);
        }
    }
}

/* Setting up x points for DEGAMMA once */
void vpe_color_setup_x_points_distribution_degamma(void)
{
    struct fixed31_32 region_size = vpe_fixpt_from_int(1);
    int32_t           segment;
    uint32_t          index         = 0;
    uint32_t          numptsdegamma = 1;
    uint32_t          segment_offset;

    /* Since region = -8 only has 1 point setting it up before the loop */
    coordinates_x_degamma[0].x = vpe_fixpt_div(vpe_fixpt_from_int(1), vpe_fixpt_from_int(512));

    for (segment = -7; segment <= 0; segment++) {
        segment_offset = numptsdegamma;
        numptsdegamma *= 2;

        for (index = segment_offset; index < numptsdegamma; index++) {
            coordinates_x_degamma[index].x =
                vpe_fixpt_div(vpe_fixpt_from_int(index), vpe_fixpt_from_int(256));
        }
    }
    coordinates_x_degamma[MAX_HW_POINTS_DEGAMMA - 1].x = region_size;
}

void vpe_compute_pq(struct fixed31_32 in_x, struct fixed31_32 *out_y)
{
    /* consts for PQ gamma formula. */
    const struct fixed31_32 m1 = vpe_fixpt_from_fraction(159301758, 1000000000);
    const struct fixed31_32 m2 = vpe_fixpt_from_fraction(7884375, 100000);
    const struct fixed31_32 c1 = vpe_fixpt_from_fraction(8359375, 10000000);
    const struct fixed31_32 c2 = vpe_fixpt_from_fraction(188515625, 10000000);
    const struct fixed31_32 c3 = vpe_fixpt_from_fraction(186875, 10000);

    struct fixed31_32 l_pow_m1;
    struct fixed31_32 base;

    if (vpe_fixpt_le(vpe_fixpt_one, in_x)) {
        *out_y = vpe_fixpt_one;
        return;
    }

    if (vpe_fixpt_lt(in_x, vpe_fixpt_zero))
        in_x = vpe_fixpt_zero;

    l_pow_m1 = vpe_fixpt_pow(in_x, m1);
    base     = vpe_fixpt_div(vpe_fixpt_add(c1, (vpe_fixpt_mul(c2, l_pow_m1))),
            vpe_fixpt_add(vpe_fixpt_one, (vpe_fixpt_mul(c3, l_pow_m1))));
    *out_y   = vpe_fixpt_pow(base, m2);
}

static void compute_de_pq(struct fixed31_32 in_x, struct fixed31_32 *out_y)
{
    /* consts for dePQ gamma formula. */
    const struct fixed31_32 m1 = vpe_fixpt_from_fraction(159301758, 1000000000);
    const struct fixed31_32 m2 = vpe_fixpt_from_fraction(7884375, 100000);
    const struct fixed31_32 c1 = vpe_fixpt_from_fraction(8359375, 10000000);
    const struct fixed31_32 c2 = vpe_fixpt_from_fraction(188515625, 10000000);
    const struct fixed31_32 c3 = vpe_fixpt_from_fraction(186875, 10000);

    struct fixed31_32 l_pow_m1;
    struct fixed31_32 base, div;
    struct fixed31_32 base2;

    if (vpe_fixpt_lt(in_x, vpe_fixpt_zero))
        in_x = vpe_fixpt_zero;

    if (vpe_fixpt_le(vpe_fixpt_one, in_x)) {
        *out_y = vpe_fixpt_one;
        return;
    }

    l_pow_m1 = vpe_fixpt_pow(in_x, vpe_fixpt_div(vpe_fixpt_one, m2));
    base     = vpe_fixpt_sub(l_pow_m1, c1);

    div = vpe_fixpt_sub(c2, vpe_fixpt_mul(c3, l_pow_m1));

    base2 = vpe_fixpt_div(base, div);
    // avoid complex numbers
    if (vpe_fixpt_lt(base2, vpe_fixpt_zero))
        base2 = vpe_fixpt_sub(vpe_fixpt_zero, base2);

    *out_y = vpe_fixpt_pow(base2, vpe_fixpt_div(vpe_fixpt_one, m1));
}

/* one-time pre-compute PQ values - only for sdr_white_level 80 */
static void precompute_pq(void)
{
    int                      i;
    struct fixed31_32        x;
    const struct hw_x_point *coord_x        = coordinates_x + 32;
    struct fixed31_32        scaling_factor = vpe_fixpt_from_fraction(80, 10000);

    struct fixed31_32 *pq_table = vpe_color_get_table(type_pq_table);

    /* pow function has problems with arguments too small */
    for (i = 0; i < 32; i++)
        pq_table[i] = vpe_fixpt_zero;

    for (i = 32; i <= MAX_HW_POINTS; i++) {
        x = vpe_fixpt_mul(coord_x->x, scaling_factor);
        vpe_compute_pq(x, &pq_table[i]);
        ++coord_x;
    }
}

/* one-time pre-compute dePQ values - only for max pixel value 125 FP16.
   yuv2rgbScaling is used when the output yuv->rgb is scaled down
   due to limited range of the yuv2rgb matrix
*/

static void precompute_de_pq(struct fixed31_32 x_scale, struct fixed31_32 y_scale)
{
    uint32_t           i;
    struct fixed31_32  y;
    struct fixed31_32 *de_pq_table = vpe_color_get_table(type_de_pq_table);

    for (i = 0; i < MAX_HW_POINTS_DEGAMMA; i++) {
        compute_de_pq(vpe_fixpt_mul(coordinates_x_degamma[i].x, x_scale), &y);
        de_pq_table[i] = vpe_fixpt_mul(y, y_scale);
    }
}

static bool build_coefficients(
    struct gamma_coefficients *coefficients, enum color_transfer_func type)
{

    uint32_t i     = 0;
    uint32_t index = 0;
    bool     ret   = true;

    if (type == TRANSFER_FUNC_SRGB)
        index = 0;
    else if (type == TRANSFER_FUNC_BT709)
        index = 1;
    else if (type == TRANSFER_FUNC_BT1886)
        index = 3;
    else {
        VPE_ASSERT(0);
        ret = false;
        goto release;
    }

    do {
        coefficients->a0[i]         = vpe_fixpt_from_fraction(numerator01[index], 10000000);
        coefficients->a1[i]         = vpe_fixpt_from_fraction(numerator02[index], 1000);
        coefficients->a2[i]         = vpe_fixpt_from_fraction(numerator03[index], 1000);
        coefficients->a3[i]         = vpe_fixpt_from_fraction(numerator04[index], 1000);
        coefficients->user_gamma[i] = vpe_fixpt_from_fraction(numerator05[index], 1000);

        ++i;
    } while (i != ARRAY_SIZE(coefficients->a0));
release:
    return ret;
}

// bt.1886
static struct fixed31_32 translate_to_linear_space(struct fixed31_32 arg, struct fixed31_32 a0,
    struct fixed31_32 a1, struct fixed31_32 a2, struct fixed31_32 a3, struct fixed31_32 gamma)
{
    struct fixed31_32 linear;

    a0 = vpe_fixpt_mul(a0, a1);
    if (vpe_fixpt_le(arg, vpe_fixpt_neg(a0)))

        linear = vpe_fixpt_neg(vpe_fixpt_pow(
            vpe_fixpt_div(vpe_fixpt_sub(a2, arg), vpe_fixpt_add(vpe_fixpt_one, a3)), gamma));

    else if (vpe_fixpt_le(vpe_fixpt_neg(a0), arg) && vpe_fixpt_le(arg, a0))
        linear = vpe_fixpt_div(arg, a1);
    else
        linear = vpe_fixpt_pow(
            vpe_fixpt_div(vpe_fixpt_add(a2, arg), vpe_fixpt_add(vpe_fixpt_one, a3)), gamma);

    return linear;
}

static inline struct fixed31_32 translate_to_linear_space_ex(
    struct fixed31_32 arg, struct gamma_coefficients *coeff, uint32_t color_index)
{
    if (vpe_fixpt_le(vpe_fixpt_one, arg))
        return vpe_fixpt_one;

    return translate_to_linear_space(arg, coeff->a0[color_index], coeff->a1[color_index],
        coeff->a2[color_index], coeff->a3[color_index], coeff->user_gamma[color_index]);
}

static struct fixed31_32 translate_from_linear_space(struct translate_from_linear_space_args *args)
{
    const struct fixed31_32 one = vpe_fixpt_from_int(1);

    struct fixed31_32        scratch_1, scratch_2;
    struct calculate_buffer *cal_buffer = args->cal_buffer;

    if (vpe_fixpt_le(one, args->arg))
        return one;

    if (vpe_fixpt_le(args->arg, vpe_fixpt_neg(args->a0))) {
        scratch_1 = vpe_fixpt_add(one, args->a3);
        scratch_2 = vpe_fixpt_pow(vpe_fixpt_neg(args->arg), vpe_fixpt_recip(args->gamma));
        scratch_1 = vpe_fixpt_mul(scratch_1, scratch_2);
        scratch_1 = vpe_fixpt_sub(args->a2, scratch_1);

        return scratch_1;
    } else if (vpe_fixpt_le(args->a0, args->arg)) {
        if (cal_buffer->buffer_index == 0) {
            cal_buffer->gamma_of_2 =
                vpe_fixpt_pow(vpe_fixpt_from_int(2), vpe_fixpt_recip(args->gamma));
        }
        scratch_1 = vpe_fixpt_add(one, args->a3);
        // In the first region (first 16 points) and in the
        // region delimited by START/END we calculate with
        // full precision to avoid error accumulation.
        if ((cal_buffer->buffer_index >= PRECISE_LUT_REGION_START &&
                cal_buffer->buffer_index <= PRECISE_LUT_REGION_END) ||
            (cal_buffer->buffer_index < 16))
            scratch_2 = vpe_fixpt_pow(args->arg, vpe_fixpt_recip(args->gamma));
        else
            scratch_2 = vpe_fixpt_mul(
                cal_buffer->gamma_of_2, cal_buffer->buffer[cal_buffer->buffer_index % 16]);

        if (cal_buffer->buffer_index != -1) {
            cal_buffer->buffer[cal_buffer->buffer_index % 16] = scratch_2;
            cal_buffer->buffer_index++;
        }

        scratch_1 = vpe_fixpt_mul(scratch_1, scratch_2);
        scratch_1 = vpe_fixpt_sub(scratch_1, args->a2);

        return scratch_1;
    } else
        return vpe_fixpt_mul(args->arg, args->a1);
}

static struct fixed31_32 translate_from_linear_space_ex(struct fixed31_32 arg,
    struct gamma_coefficients *coeff, uint32_t color_index, struct calculate_buffer *cal_buffer)
{
    struct translate_from_linear_space_args scratch_gamma_args = {0};

    scratch_gamma_args.arg        = arg;
    scratch_gamma_args.a0         = coeff->a0[color_index];
    scratch_gamma_args.a1         = coeff->a1[color_index];
    scratch_gamma_args.a2         = coeff->a2[color_index];
    scratch_gamma_args.a3         = coeff->a3[color_index];
    scratch_gamma_args.gamma      = coeff->user_gamma[color_index];
    scratch_gamma_args.cal_buffer = cal_buffer;

    return translate_from_linear_space(&scratch_gamma_args);
}

static void build_pq(struct pwl_float_data_ex *rgb_regamma, uint32_t hw_points_num,
    const struct hw_x_point *coordinate_x, uint32_t hdr_normalization)
{
    uint32_t i, start_index;

    struct pwl_float_data_ex *rgb     = rgb_regamma;
    const struct hw_x_point  *coord_x = coordinate_x;
    struct fixed31_32         output;
    struct fixed31_32         scaling_factor = vpe_fixpt_from_fraction(1, hdr_normalization);

    /* TODO: start index is from segment 2^-24, skipping first segment
     * due to x values too small for power calculations
     */
    start_index = 32;
    rgb += start_index;
    coord_x += start_index;

    for (i = start_index; i <= hw_points_num; i++) {

        vpe_compute_pq(vpe_fixpt_mul(coord_x->x, scaling_factor), &output);

        /* should really not happen? */
        if (vpe_fixpt_lt(output, vpe_fixpt_zero))
            output = vpe_fixpt_zero;
        else if (vpe_fixpt_lt(vpe_fixpt_one, output))
            output = vpe_fixpt_one;

        rgb->r = output;
        rgb->g = output;
        rgb->b = output;

        ++coord_x;
        ++rgb;
    }
    coord_x                 = coordinates_x;
    rgb                     = rgb_regamma;
    struct fixed31_32 slope = vpe_fixpt_div(rgb[start_index].r, coord_x[start_index].x);
    for (i = 0; i < start_index; i++) {
        output = vpe_fixpt_mul(coord_x->x, slope);
        rgb->r = output;
        rgb->g = output;
        rgb->b = output;

        ++coord_x;
        ++rgb;
    }
}

static void build_de_pq(struct transfer_func_distributed_points *de_pq, uint32_t hw_points_num,
    const struct hw_x_point *coordinate_x_degamma, struct fixed31_32 x_scale,
    struct fixed31_32 y_scale)
{
    uint32_t           i;
    struct fixed31_32  output;
    struct fixed31_32 *de_pq_table = vpe_color_get_table(type_de_pq_table);

    precompute_de_pq(x_scale, y_scale);

    for (i = 0; i < hw_points_num; i++) {
        output = de_pq_table[i];
        /* should really not happen? */
        if (vpe_fixpt_lt(output, vpe_fixpt_zero))
            output = vpe_fixpt_zero;

        de_pq->red[i]   = output;
        de_pq->green[i] = output;
        de_pq->blue[i]  = output;
    }
}

static bool build_degamma(struct transfer_func_distributed_points *curve, uint32_t hw_points_num,
    const struct hw_x_point *coordinate_x_degamma, enum color_transfer_func type,
    struct fixed31_32 yuv2rgbScaling)
{
    uint32_t                  i;
    struct gamma_coefficients coeff;
    struct fixed31_32         scaledX;
    struct fixed31_32         scaledY;
    bool                      ret = false;

    if (!build_coefficients(&coeff, type))
        goto release;

    /* De-gamma X is 2^-8 to 2^0 i.e. 9 regions
     */

    i = 0;
    while (i != MAX_HW_POINTS_DEGAMMA) {
        scaledX         = vpe_fixpt_mul(coordinate_x_degamma[i].x, yuv2rgbScaling);
        scaledY         = translate_to_linear_space_ex(scaledX, &coeff, 0);
        curve->red[i]   = scaledY;
        curve->green[i] = scaledY;
        curve->blue[i]  = scaledY;
        i++;
    }
    ret = true;
release:
    return ret;
}

static bool build_regamma(struct vpe_priv *vpe_priv, struct pwl_float_data_ex *rgb_regamma,
    uint32_t hw_points_num, const struct hw_x_point *coordinate_x, enum color_transfer_func type,
    struct calculate_buffer *cal_buffer)
{
    uint32_t i;
    bool     ret = false;

    struct gamma_coefficients *coeff;
    struct pwl_float_data_ex  *rgb     = rgb_regamma;
    const struct hw_x_point   *coord_x = coordinate_x;

    coeff = (struct gamma_coefficients *)vpe_zalloc(sizeof(*coeff));
    if (!coeff)
        goto release;

    if (!build_coefficients(coeff, type))
        goto release;

    memset(cal_buffer->buffer, 0, NUM_PTS_IN_REGION * sizeof(struct fixed31_32));
    cal_buffer->buffer_index = 0; // see variable definition for more info

    i = 0;
    while (i <= hw_points_num) {
        /* TODO use y vs r,g,b */
        rgb->r = translate_from_linear_space_ex(coord_x->x, coeff, 0, cal_buffer);
        rgb->g = rgb->r;
        rgb->b = rgb->r;
        ++coord_x;
        ++rgb;
        ++i;
    }
    cal_buffer->buffer_index = -1;
    ret                      = true;
release:
    vpe_free(coeff);
    return ret;
}

static void build_new_custom_resulted_curve(
    uint32_t hw_points_num, struct transfer_func_distributed_points *tf_pts)
{
    uint32_t i = 0;

    while (i != hw_points_num + 1) {
        tf_pts->red[i]   = vpe_fixpt_clamp(tf_pts->red[i], vpe_fixpt_zero, vpe_fixpt_one);
        tf_pts->green[i] = vpe_fixpt_clamp(tf_pts->green[i], vpe_fixpt_zero, vpe_fixpt_one);
        tf_pts->blue[i]  = vpe_fixpt_clamp(tf_pts->blue[i], vpe_fixpt_zero, vpe_fixpt_one);

        ++i;
    }
}

static bool map_regamma_hw_to_x_user(struct pixel_gamma_point *coeff128,
    struct hw_x_point *coords_x, const struct pwl_float_data_ex *rgb_regamma,
    uint32_t hw_points_num, struct transfer_func_distributed_points *tf_pts, bool doClamping)
{
    /* setup to spare calculated ideal regamma values */

    uint32_t                        i       = 0;
    struct hw_x_point              *coords  = coords_x;
    const struct pwl_float_data_ex *regamma = rgb_regamma;

    /* just copy current rgb_regamma into  tf_pts */
    while (i <= hw_points_num) {
        tf_pts->red[i]   = regamma->r;
        tf_pts->green[i] = regamma->g;
        tf_pts->blue[i]  = regamma->b;

        ++regamma;
        ++i;
    }

    if (doClamping) {
        /* this should be named differently, all it does is clamp to 0-1 */
        build_new_custom_resulted_curve(hw_points_num, tf_pts);
    }

    return true;
}

static bool calculate_curve(struct vpe_priv *vpe_priv, enum color_transfer_func trans,
    struct transfer_func_distributed_points *points, struct pwl_float_data_ex *rgb_regamma,
    uint32_t hdr_normalization, struct calculate_buffer *cal_buffer)
{
    bool ret = false;

    if (trans == TRANSFER_FUNC_PQ2084) {
        points->end_exponent        = 0;
        points->x_point_at_y1_red   = 1;
        points->x_point_at_y1_green = 1;
        points->x_point_at_y1_blue  = 1;

        build_pq(rgb_regamma, MAX_HW_POINTS, coordinates_x, hdr_normalization);
        ret = true;
    } else if (trans == TRANSFER_FUNC_LINEAR_0_125) {
        for (int i = 0; i < MAX_HW_POINTS; i++) {
            rgb_regamma[i].r =
                vpe_fixpt_mul(coordinates_x[i].x, vpe_fixpt_from_fraction(125, hdr_normalization));
            rgb_regamma[i].g = rgb_regamma[i].r;
            rgb_regamma[i].b = rgb_regamma[i].r;
        }
        ret = true;
    } else {
        // trans == TRANSFER_FUNC_SRGB
        // trans == TRANSFER_FUNC_BT709
        // trans == TRANSFER_FUNCTION_GAMMA22
        // trans == TRANSFER_FUNCTION_GAMMA24
        // trans == TRANSFER_FUNCTION_GAMMA26
        points->end_exponent        = 0;
        points->x_point_at_y1_red   = 1;
        points->x_point_at_y1_green = 1;
        points->x_point_at_y1_blue  = 1;

        build_regamma(vpe_priv, rgb_regamma, MAX_HW_POINTS, coordinates_x, trans, cal_buffer);

        ret = true;
    }
    return ret;
}

#define _EXTRA_POINTS 3

bool vpe_color_calculate_degamma_params(struct vpe_priv *vpe_priv, struct fixed31_32 x_scale,
    struct fixed31_32 y_scale, struct transfer_func *input_tf)
{
    struct transfer_func_distributed_points *tf_pts = &input_tf->tf_pts;
    enum color_transfer_func                 tf;
    uint32_t                                 i;
    bool                                     ret = true;

    tf = input_tf->tf;

    if (tf == TRANSFER_FUNC_PQ2084 || tf == TRANSFER_FUNC_NORMALIZED_PQ)
        build_de_pq(tf_pts, MAX_HW_POINTS_DEGAMMA, coordinates_x_degamma, x_scale, y_scale);
    else if (tf == TRANSFER_FUNC_SRGB || tf == TRANSFER_FUNC_BT709 || tf == TRANSFER_FUNC_BT1886)
        build_degamma(tf_pts, MAX_HW_POINTS_DEGAMMA, coordinates_x_degamma, tf, x_scale);
    else if (tf == TRANSFER_FUNC_LINEAR_0_125) {
        // just copy coordinates_x_degamma into curve
        i = 0;
        while (i != MAX_HW_POINTS_DEGAMMA) {
            tf_pts->red[i] = vpe_fixpt_mul(coordinates_x[i].x, y_scale);
            tf_pts->red[i] = tf_pts->red[i];
            tf_pts->red[i] = tf_pts->red[i];
            i++;
        }
    } else
        ret = false;

    return ret;
}

bool vpe_color_calculate_regamma_params(
    struct vpe_priv *vpe_priv, struct transfer_func *output_tf, struct calculate_buffer *cal_buffer)
{
    struct transfer_func_distributed_points *tf_pts      = &output_tf->tf_pts;
    struct pwl_float_data_ex                *rgb_regamma = NULL;
    struct pixel_gamma_point                *coeff       = NULL;
    enum color_transfer_func                 tf;
    bool                                     ret = false;

    rgb_regamma = (struct pwl_float_data_ex *)vpe_zalloc(
        (MAX_HW_POINTS + _EXTRA_POINTS) * sizeof(*rgb_regamma));
    if (!rgb_regamma)
        goto rgb_regamma_alloc_fail;

    coeff =
        (struct pixel_gamma_point *)vpe_zalloc((MAX_HW_POINTS + _EXTRA_POINTS) * sizeof(*coeff));
    if (!coeff)
        goto coeff_alloc_fail;

    tf = output_tf->tf;

    ret = calculate_curve(vpe_priv, tf, tf_pts, rgb_regamma,
        vpe_priv->resource.internal_hdr_normalization, cal_buffer);

    if (ret) {
        map_regamma_hw_to_x_user(coeff, coordinates_x, rgb_regamma, MAX_HW_POINTS, tf_pts, false);
    }

    vpe_free(coeff);
coeff_alloc_fail:
    vpe_free(rgb_regamma);
rgb_regamma_alloc_fail:
    return ret;
}
