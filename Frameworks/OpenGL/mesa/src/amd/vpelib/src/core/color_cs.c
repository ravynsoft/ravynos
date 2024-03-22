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
#include "color_cs.h"
#include "hw_shared.h"
#include "conversion.h"

#define DIVIDER 10000
/* S2D13 value in [-3.999...3.999] */
#define S2D13_MIN (-39990)
#define S2D13_MAX (39990)

static void translate_blt_to_internal_adjustments(
    const struct vpe_color_adjust *blt_adjust, struct vpe_color_adjustments *dal_adjust);

/* these values are defaults: 0 brightness, 1 contrast, 0 hue, 1 saturation*/
static struct vpe_color_adjust defaultClrAdjust = {0.0f, 1.0f, 0.0f, 1.0f};

void vpe_color_set_adjustments_to_default(struct vpe_color_adjust *crt_vpe_adjusts)
{
    *crt_vpe_adjusts = defaultClrAdjust;
}

bool vpe_color_different_color_adjusts(
    const struct vpe_color_adjust *new_vpe_adjusts, struct vpe_color_adjust *crt_vpe_adjsuts)
{
    if ((crt_vpe_adjsuts->brightness != new_vpe_adjusts->brightness) ||
        (crt_vpe_adjsuts->saturation != new_vpe_adjusts->saturation) ||
        (crt_vpe_adjsuts->hue != new_vpe_adjusts->hue) ||
        (crt_vpe_adjsuts->contrast != new_vpe_adjusts->contrast)) {
        return true;
    }
    return false;
}

/**
 * Adjustment     Min      Max    default   step
 *
 * Input range
 * Brightness  -100.0f,  100.0f,   0.0f,    0.1f
 * Contrast       0.0f,    2.0f,    1.0f,   0.01f
 * Hue         -180.0f,  180.0f,   0.0f,    1.0f
 * Saturation     0.0f,    3.0f,   1.0f,    0.01f
 *
 * DAL range
 * Brightness    -100,     100,      0,      1
 * Contrast         0,     200,    100,      1
 * Hue            -30,      30,      0,      1
 * Saturation       0,     200,    100,      1
 */

static void translate_blt_to_internal_adjustments(
    const struct vpe_color_adjust *blt_adjust, struct vpe_color_adjustments *dal_adjust)
{
    dal_adjust->brightness.current = (int)(10 * blt_adjust->brightness);
    dal_adjust->brightness.min     = -1000;
    dal_adjust->brightness.max     = 1000;

    dal_adjust->contrast.current = (int)(100 * blt_adjust->contrast);
    dal_adjust->contrast.min     = 0;
    dal_adjust->contrast.max     = 200;

    dal_adjust->saturation.current = (int)(100 * blt_adjust->saturation);
    dal_adjust->saturation.min     = 0;
    dal_adjust->saturation.max     = 300; // assuming input bigger range

    dal_adjust->hue.current = (int)(blt_adjust->hue);
    dal_adjust->hue.min     = -180;
    dal_adjust->hue.max     = 180; // assuming input bigger range
}

static int get_hw_value_from_sw_value(int swVal, int swMin, int swMax, int hwMin, int hwMax)
{
    int dSW = swMax - swMin; /*software adjustment range size*/
    int dHW = hwMax - hwMin; /*hardware adjustment range size*/
    int hwVal;               /*HW adjustment value*/

    /* error case, I preserve the behavior from the predecessor
     *getHwStepFromSwHwMinMaxValue (removed in Feb 2013)
     *which was the FP version that only computed SCLF (i.e. dHW/dSW).
     *it would return 0 in this case so
     *hwVal = hwMin from the formula given in @brief
     */
    if (dSW == 0)
        return hwMin;

    /*it's quite often that ranges match,
     *e.g. for overlay colors currently (Feb 2013)
     *only brightness has a different
     *HW range, and in this case no multiplication or division is needed,
     *and if minimums match, no calculation at all
     */

    if (dSW != dHW) {
        hwVal = (swVal - swMin) * dHW / dSW + hwMin;
    } else {
        hwVal = swVal;
        if (swMin != hwMin)
            hwVal += (hwMin - swMin);
    }

    return hwVal;
}

static void color_adjustments_to_fixed_point(const struct vpe_color_adjustments *vpe_adjust,
    bool               icsc, // input csc or output csc
    struct fixed31_32 *grph_cont, struct fixed31_32 *grph_sat, struct fixed31_32 *grph_bright,
    struct fixed31_32 *sin_grph_hue, struct fixed31_32 *cos_grph_hue)
{
    /* Hue adjustment could be negative. -45 ~ +45 */
    struct fixed31_32 hue;
    const int         hw_hue_min      = -30;
    const int         hw_hue_max      = 30;
    const int         hw_sat_min      = 0;
    const int         hw_sat_max      = 200;
    const int         hw_contrast_min = 0;
    const int         hw_contrast_max = 200;
    const int         hw_bright_min   = -1000;
    const int         hw_bright_max   = 1000;
    if (icsc) {
        hue = vpe_fixpt_mul(
            vpe_fixpt_from_fraction(
                get_hw_value_from_sw_value(vpe_adjust->hue.current, vpe_adjust->hue.min,
                    vpe_adjust->hue.max, -hw_hue_min, hw_hue_max),
                180),
            vpe_fixpt_pi);

        // In MMD is -100 to +100 in 16-235 range; which when scaled to full
        // range is ~-116 to +116. When normalized this is about 0.4566.
        *grph_bright = vpe_fixpt_from_fraction(
            get_hw_value_from_sw_value(vpe_adjust->brightness.current, vpe_adjust->brightness.min,
                vpe_adjust->brightness.max, hw_bright_min, hw_bright_max),
            1000);

        *grph_cont = vpe_fixpt_from_fraction(
            get_hw_value_from_sw_value(vpe_adjust->contrast.current, vpe_adjust->contrast.min,
                vpe_adjust->contrast.max, hw_contrast_min, hw_contrast_max),
            100);

        *grph_sat = vpe_fixpt_from_fraction(
            get_hw_value_from_sw_value(vpe_adjust->saturation.current, vpe_adjust->saturation.min,
                vpe_adjust->saturation.max, hw_sat_min, hw_sat_max),
            100);
    } else {
        hue = vpe_fixpt_mul(
            vpe_fixpt_from_fraction(
                get_hw_value_from_sw_value(vpe_adjust->hue.current, vpe_adjust->hue.min,
                    vpe_adjust->hue.max, -hw_hue_min, hw_hue_max),
                180),
            vpe_fixpt_pi);

        *grph_bright = vpe_fixpt_from_fraction(
            get_hw_value_from_sw_value(vpe_adjust->brightness.current, vpe_adjust->brightness.min,
                vpe_adjust->brightness.max, hw_bright_min, hw_bright_max),
            100);

        *grph_cont = vpe_fixpt_from_fraction(
            get_hw_value_from_sw_value(vpe_adjust->contrast.current, vpe_adjust->contrast.min,
                vpe_adjust->contrast.max, hw_contrast_min, hw_contrast_max),
            100);

        *grph_sat = vpe_fixpt_from_fraction(
            get_hw_value_from_sw_value(vpe_adjust->saturation.current, vpe_adjust->saturation.min,
                vpe_adjust->saturation.max, hw_sat_min, hw_sat_max),
            100);
    }

    *sin_grph_hue = vpe_fixpt_sin(hue);
    *cos_grph_hue = vpe_fixpt_cos(hue);
}

static void calculate_rgb_matrix_legacy(
    struct vpe_color_adjustments *vpe_adjust, struct fixed31_32 *rgb_matrix)
{
    const struct fixed31_32 k1  = vpe_fixpt_from_fraction(787400, 1000000);
    const struct fixed31_32 k2  = vpe_fixpt_from_fraction(180428, 1000000);
    const struct fixed31_32 k3  = vpe_fixpt_from_fraction(-715200, 1000000);
    const struct fixed31_32 k4  = vpe_fixpt_from_fraction(606972, 1000000);
    const struct fixed31_32 k5  = vpe_fixpt_from_fraction(-72200, 1000000);
    const struct fixed31_32 k6  = vpe_fixpt_from_fraction(-787400, 1000000);
    const struct fixed31_32 k7  = vpe_fixpt_from_fraction(-212600, 1000000);
    const struct fixed31_32 k8  = vpe_fixpt_from_fraction(-147296, 1000000);
    const struct fixed31_32 k9  = vpe_fixpt_from_fraction(284800, 1000000);
    const struct fixed31_32 k10 = vpe_fixpt_from_fraction(-95354, 1000000);
    const struct fixed31_32 k11 = vpe_fixpt_from_fraction(-72200, 1000000);
    const struct fixed31_32 k12 = vpe_fixpt_from_fraction(242650, 1000000);
    const struct fixed31_32 k13 = vpe_fixpt_from_fraction(-212600, 1000000);
    const struct fixed31_32 k14 = vpe_fixpt_from_fraction(927800, 1000000);
    const struct fixed31_32 k15 = vpe_fixpt_from_fraction(-715200, 1000000);
    const struct fixed31_32 k16 = vpe_fixpt_from_fraction(-842726, 1000000);
    const struct fixed31_32 k17 = vpe_fixpt_from_fraction(927800, 1000000);
    const struct fixed31_32 k18 = vpe_fixpt_from_fraction(-85074, 1000000);

    const struct fixed31_32 luma_r = vpe_fixpt_from_fraction(2126, 10000);
    const struct fixed31_32 luma_g = vpe_fixpt_from_fraction(7152, 10000);
    const struct fixed31_32 luma_b = vpe_fixpt_from_fraction(722, 10000);

    struct fixed31_32 grph_cont;
    struct fixed31_32 grph_sat;
    struct fixed31_32 grph_bright;
    struct fixed31_32 sin_grph_hue;
    struct fixed31_32 cos_grph_hue;

    color_adjustments_to_fixed_point(
        vpe_adjust, true, &grph_cont, &grph_sat, &grph_bright, &sin_grph_hue, &cos_grph_hue);

    /* COEF_1_1 = GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K1 +*/
    /* Sin(GrphHue) * K2))*/
    /* (Cos(GrphHue) * K1 + Sin(GrphHue) * K2)*/
    rgb_matrix[0] = vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k1), vpe_fixpt_mul(sin_grph_hue, k2));
    /* GrphSat * (Cos(GrphHue) * K1 + Sin(GrphHue) * K2 */
    rgb_matrix[0] = vpe_fixpt_mul(grph_sat, rgb_matrix[0]);
    /* (LumaR + GrphSat * (Cos(GrphHue) * K1 + Sin(GrphHue) * K2))*/
    rgb_matrix[0] = vpe_fixpt_add(luma_r, rgb_matrix[0]);
    /* GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K1 + Sin(GrphHue)**/
    /* K2))*/
    rgb_matrix[0] = vpe_fixpt_mul(grph_cont, rgb_matrix[0]);

    /* COEF_1_2 = GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K3 +*/
    /* Sin(GrphHue) * K4))*/
    /* (Cos(GrphHue) * K3 + Sin(GrphHue) * K4)*/
    rgb_matrix[1] = vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k3), vpe_fixpt_mul(sin_grph_hue, k4));
    /* GrphSat * (Cos(GrphHue) * K3 + Sin(GrphHue) * K4)*/
    rgb_matrix[1] = vpe_fixpt_mul(grph_sat, rgb_matrix[1]);
    /* (LumaG + GrphSat * (Cos(GrphHue) * K3 + Sin(GrphHue) * K4))*/
    rgb_matrix[1] = vpe_fixpt_add(luma_g, rgb_matrix[1]);
    /* GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K3 + Sin(GrphHue)**/
    /* K4))*/
    rgb_matrix[1] = vpe_fixpt_mul(grph_cont, rgb_matrix[1]);

    /* COEF_1_3 = GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K5 +*/
    /* Sin(GrphHue) * K6))*/
    /* (Cos(GrphHue) * K5 + Sin(GrphHue) * K6)*/
    rgb_matrix[2] = vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k5), vpe_fixpt_mul(sin_grph_hue, k6));
    /* GrphSat * (Cos(GrphHue) * K5 + Sin(GrphHue) * K6)*/
    rgb_matrix[2] = vpe_fixpt_mul(grph_sat, rgb_matrix[2]);
    /* LumaB + GrphSat * (Cos(GrphHue) * K5 + Sin(GrphHue) * K6)*/
    rgb_matrix[2] = vpe_fixpt_add(luma_b, rgb_matrix[2]);
    /* GrphCont  * (LumaB + GrphSat * (Cos(GrphHue) * K5 + Sin(GrphHue)**/
    /* K6))*/
    rgb_matrix[2] = vpe_fixpt_mul(grph_cont, rgb_matrix[2]);

    /* COEF_1_4 = GrphBright*/
    rgb_matrix[3] = grph_bright;

    /* COEF_2_1 = GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K7 +*/
    /* Sin(GrphHue) * K8))*/
    /* (Cos(GrphHue) * K7 + Sin(GrphHue) * K8)*/
    rgb_matrix[4] = vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k7), vpe_fixpt_mul(sin_grph_hue, k8));
    /* GrphSat * (Cos(GrphHue) * K7 + Sin(GrphHue) * K8)*/
    rgb_matrix[4] = vpe_fixpt_mul(grph_sat, rgb_matrix[4]);
    /* (LumaR + GrphSat * (Cos(GrphHue) * K7 + Sin(GrphHue) * K8))*/
    rgb_matrix[4] = vpe_fixpt_add(luma_r, rgb_matrix[4]);
    /* GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K7 + Sin(GrphHue)**/
    /* K8))*/
    rgb_matrix[4] = vpe_fixpt_mul(grph_cont, rgb_matrix[4]);

    /* COEF_2_2 = GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K9 +*/
    /* Sin(GrphHue) * K10))*/
    /* (Cos(GrphHue) * K9 + Sin(GrphHue) * K10))*/
    rgb_matrix[5] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k9), vpe_fixpt_mul(sin_grph_hue, k10));
    /* GrphSat * (Cos(GrphHue) * K9 + Sin(GrphHue) * K10))*/
    rgb_matrix[5] = vpe_fixpt_mul(grph_sat, rgb_matrix[5]);
    /* (LumaG + GrphSat * (Cos(GrphHue) * K9 + Sin(GrphHue) * K10))*/
    rgb_matrix[5] = vpe_fixpt_add(luma_g, rgb_matrix[5]);
    /* GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K9 + Sin(GrphHue)**/
    /* K10))*/
    rgb_matrix[5] = vpe_fixpt_mul(grph_cont, rgb_matrix[5]);

    /* COEF_2_3 = GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K11 +*/
    /* Sin(GrphHue) * K12))*/
    /* (Cos(GrphHue) * K11 + Sin(GrphHue) * K12))*/
    rgb_matrix[6] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k11), vpe_fixpt_mul(sin_grph_hue, k12));
    /* GrphSat * (Cos(GrphHue) * K11 + Sin(GrphHue) * K12))*/
    rgb_matrix[6] = vpe_fixpt_mul(grph_sat, rgb_matrix[6]);
    /* (LumaB + GrphSat * (Cos(GrphHue) * K11 + Sin(GrphHue) * K12))*/
    rgb_matrix[6] = vpe_fixpt_add(luma_b, rgb_matrix[6]);
    /* GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K11 + Sin(GrphHue)**/
    /* K12))*/
    rgb_matrix[6] = vpe_fixpt_mul(grph_cont, rgb_matrix[6]);

    /* COEF_2_4 = GrphBright*/
    rgb_matrix[7] = grph_bright;

    /* COEF_3_1 = GrphCont  * (LumaR + GrphSat * (Cos(GrphHue) * K13 +*/
    /* Sin(GrphHue) * K14))*/
    /* (Cos(GrphHue) * K13 + Sin(GrphHue) * K14)) */
    rgb_matrix[8] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k13), vpe_fixpt_mul(sin_grph_hue, k14));
    /* GrphSat * (Cos(GrphHue) * K13 + Sin(GrphHue) * K14)) */
    rgb_matrix[8] = vpe_fixpt_mul(grph_sat, rgb_matrix[8]);
    /* (LumaR + GrphSat * (Cos(GrphHue) * K13 + Sin(GrphHue) * K14)) */
    rgb_matrix[8] = vpe_fixpt_add(luma_r, rgb_matrix[8]);
    /* GrphCont  * (LumaR + GrphSat * (Cos(GrphHue) * K13 + Sin(GrphHue)**/
    /* K14)) */
    rgb_matrix[8] = vpe_fixpt_mul(grph_cont, rgb_matrix[8]);

    /* COEF_3_2    = GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K15 +*/
    /* Sin(GrphHue) * K16)) */
    /* GrphSat * (Cos(GrphHue) * K15 + Sin(GrphHue) * K16) */
    rgb_matrix[9] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k15), vpe_fixpt_mul(sin_grph_hue, k16));
    /* (LumaG + GrphSat * (Cos(GrphHue) * K15 + Sin(GrphHue) * K16)) */
    rgb_matrix[9] = vpe_fixpt_mul(grph_sat, rgb_matrix[9]);
    /* (LumaG + GrphSat * (Cos(GrphHue) * K15 + Sin(GrphHue) * K16)) */
    rgb_matrix[9] = vpe_fixpt_add(luma_g, rgb_matrix[9]);
    /* GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K15 + Sin(GrphHue)**/
    /* K16)) */
    rgb_matrix[9] = vpe_fixpt_mul(grph_cont, rgb_matrix[9]);

    /*  COEF_3_3 = GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K17 +*/
    /* Sin(GrphHue) * K18)) */
    /* (Cos(GrphHue) * K17 + Sin(GrphHue) * K18)) */
    rgb_matrix[10] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k17), vpe_fixpt_mul(sin_grph_hue, k18));
    /*  GrphSat * (Cos(GrphHue) * K17 + Sin(GrphHue) * K18)) */
    rgb_matrix[10] = vpe_fixpt_mul(grph_sat, rgb_matrix[10]);
    /* (LumaB + GrphSat * (Cos(GrphHue) * K17 + Sin(GrphHue) * K18)) */
    rgb_matrix[10] = vpe_fixpt_add(luma_b, rgb_matrix[10]);
    /* GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K17 + Sin(GrphHue)**/
    /* K18)) */
    rgb_matrix[10] = vpe_fixpt_mul(grph_cont, rgb_matrix[10]);

    /* COEF_3_4 = GrphBright */
    rgb_matrix[11] = grph_bright;
}

static void calculate_rgb_limited_range_matrix_legacy(
    struct vpe_color_adjustments *vpe_adjust, struct fixed31_32 *rgb_matrix)
{
    const struct fixed31_32 k1  = vpe_fixpt_from_fraction(701000, 1000000);
    const struct fixed31_32 k2  = vpe_fixpt_from_fraction(236568, 1000000);
    const struct fixed31_32 k3  = vpe_fixpt_from_fraction(-587000, 1000000);
    const struct fixed31_32 k4  = vpe_fixpt_from_fraction(464432, 1000000);
    const struct fixed31_32 k5  = vpe_fixpt_from_fraction(-114000, 1000000);
    const struct fixed31_32 k6  = vpe_fixpt_from_fraction(-701000, 1000000);
    const struct fixed31_32 k7  = vpe_fixpt_from_fraction(-299000, 1000000);
    const struct fixed31_32 k8  = vpe_fixpt_from_fraction(-292569, 1000000);
    const struct fixed31_32 k9  = vpe_fixpt_from_fraction(413000, 1000000);
    const struct fixed31_32 k10 = vpe_fixpt_from_fraction(-92482, 1000000);
    const struct fixed31_32 k11 = vpe_fixpt_from_fraction(-114000, 1000000);
    const struct fixed31_32 k12 = vpe_fixpt_from_fraction(385051, 1000000);
    const struct fixed31_32 k13 = vpe_fixpt_from_fraction(-299000, 1000000);
    const struct fixed31_32 k14 = vpe_fixpt_from_fraction(886000, 1000000);
    const struct fixed31_32 k15 = vpe_fixpt_from_fraction(-587000, 1000000);
    const struct fixed31_32 k16 = vpe_fixpt_from_fraction(-741914, 1000000);
    const struct fixed31_32 k17 = vpe_fixpt_from_fraction(886000, 1000000);
    const struct fixed31_32 k18 = vpe_fixpt_from_fraction(-144086, 1000000);

    const struct fixed31_32 luma_r = vpe_fixpt_from_fraction(299, 1000);
    const struct fixed31_32 luma_g = vpe_fixpt_from_fraction(587, 1000);
    const struct fixed31_32 luma_b = vpe_fixpt_from_fraction(114, 1000);
    /*onst struct fixed31_32 luma_scale =
        vpe_fixpt_from_fraction(875855, 1000000);*/

    const struct fixed31_32 rgb_scale = vpe_fixpt_from_fraction(85546875, 100000000);
    const struct fixed31_32 rgb_bias  = vpe_fixpt_from_fraction(625, 10000);

    struct fixed31_32 grph_cont;
    struct fixed31_32 grph_sat;
    struct fixed31_32 grph_bright;
    struct fixed31_32 sin_grph_hue;
    struct fixed31_32 cos_grph_hue;

    color_adjustments_to_fixed_point(
        vpe_adjust, true, &grph_cont, &grph_sat, &grph_bright, &sin_grph_hue, &cos_grph_hue);

    /* COEF_1_1 = GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K1 +*/
    /* Sin(GrphHue) * K2))*/
    /* (Cos(GrphHue) * K1 + Sin(GrphHue) * K2)*/
    rgb_matrix[0] = vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k1), vpe_fixpt_mul(sin_grph_hue, k2));
    /* GrphSat * (Cos(GrphHue) * K1 + Sin(GrphHue) * K2 */
    rgb_matrix[0] = vpe_fixpt_mul(grph_sat, rgb_matrix[0]);
    /* (LumaR + GrphSat * (Cos(GrphHue) * K1 + Sin(GrphHue) * K2))*/
    rgb_matrix[0] = vpe_fixpt_add(luma_r, rgb_matrix[0]);
    /* GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K1 + Sin(GrphHue)**/
    /* K2))*/
    rgb_matrix[0] = vpe_fixpt_mul(grph_cont, rgb_matrix[0]);
    /* LumaScale * GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K1 + */
    /* Sin(GrphHue) * K2))*/
    rgb_matrix[0] = vpe_fixpt_mul(rgb_scale, rgb_matrix[0]);

    /* COEF_1_2 = GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K3 +*/
    /* Sin(GrphHue) * K4))*/
    /* (Cos(GrphHue) * K3 + Sin(GrphHue) * K4)*/
    rgb_matrix[1] = vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k3), vpe_fixpt_mul(sin_grph_hue, k4));
    /* GrphSat * (Cos(GrphHue) * K3 + Sin(GrphHue) * K4)*/
    rgb_matrix[1] = vpe_fixpt_mul(grph_sat, rgb_matrix[1]);
    /* (LumaG + GrphSat * (Cos(GrphHue) * K3 + Sin(GrphHue) * K4))*/
    rgb_matrix[1] = vpe_fixpt_add(luma_g, rgb_matrix[1]);
    /* GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K3 + Sin(GrphHue)**/
    /* K4))*/
    rgb_matrix[1] = vpe_fixpt_mul(grph_cont, rgb_matrix[1]);
    /* LumaScale * GrphCont * (LumaG + GrphSat *(Cos(GrphHue) * K3 + */
    /* Sin(GrphHue) * K4))*/
    rgb_matrix[1] = vpe_fixpt_mul(rgb_scale, rgb_matrix[1]);

    /* COEF_1_3 = GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K5 +*/
    /* Sin(GrphHue) * K6))*/
    /* (Cos(GrphHue) * K5 + Sin(GrphHue) * K6)*/
    rgb_matrix[2] = vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k5), vpe_fixpt_mul(sin_grph_hue, k6));
    /* GrphSat * (Cos(GrphHue) * K5 + Sin(GrphHue) * K6)*/
    rgb_matrix[2] = vpe_fixpt_mul(grph_sat, rgb_matrix[2]);
    /* LumaB + GrphSat * (Cos(GrphHue) * K5 + Sin(GrphHue) * K6)*/
    rgb_matrix[2] = vpe_fixpt_add(luma_b, rgb_matrix[2]);
    /* GrphCont  * (LumaB + GrphSat * (Cos(GrphHue) * K5 + Sin(GrphHue)**/
    /* K6))*/
    rgb_matrix[2] = vpe_fixpt_mul(grph_cont, rgb_matrix[2]);
    /* LumaScale * GrphCont  * (LumaB + GrphSat *(Cos(GrphHue) * K5 + */
    /* Sin(GrphHue) * K6))*/
    rgb_matrix[2] = vpe_fixpt_mul(rgb_scale, rgb_matrix[2]);

    /* COEF_1_4 = RGBBias + RGBScale * GrphBright*/
    rgb_matrix[3] = vpe_fixpt_add(rgb_bias, vpe_fixpt_mul(rgb_scale, grph_bright));

    /* COEF_2_1 = GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K7 +*/
    /* Sin(GrphHue) * K8))*/
    /* (Cos(GrphHue) * K7 + Sin(GrphHue) * K8)*/
    rgb_matrix[4] = vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k7), vpe_fixpt_mul(sin_grph_hue, k8));
    /* GrphSat * (Cos(GrphHue) * K7 + Sin(GrphHue) * K8)*/
    rgb_matrix[4] = vpe_fixpt_mul(grph_sat, rgb_matrix[4]);
    /* (LumaR + GrphSat * (Cos(GrphHue) * K7 + Sin(GrphHue) * K8))*/
    rgb_matrix[4] = vpe_fixpt_add(luma_r, rgb_matrix[4]);
    /* GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K7 + Sin(GrphHue)**/
    /* K8))*/
    rgb_matrix[4] = vpe_fixpt_mul(grph_cont, rgb_matrix[4]);
    /* LumaScale * GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K7 + */
    /* Sin(GrphHue) * K8))*/
    rgb_matrix[4] = vpe_fixpt_mul(rgb_scale, rgb_matrix[4]);

    /* COEF_2_2 = GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K9 +*/
    /* Sin(GrphHue) * K10))*/
    /* (Cos(GrphHue) * K9 + Sin(GrphHue) * K10))*/
    rgb_matrix[5] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k9), vpe_fixpt_mul(sin_grph_hue, k10));
    /* GrphSat * (Cos(GrphHue) * K9 + Sin(GrphHue) * K10))*/
    rgb_matrix[5] = vpe_fixpt_mul(grph_sat, rgb_matrix[5]);
    /* (LumaG + GrphSat * (Cos(GrphHue) * K9 + Sin(GrphHue) * K10))*/
    rgb_matrix[5] = vpe_fixpt_add(luma_g, rgb_matrix[5]);
    /* GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K9 + Sin(GrphHue)**/
    /* K10))*/
    rgb_matrix[5] = vpe_fixpt_mul(grph_cont, rgb_matrix[5]);
    /* LumaScale * GrphCont * (LumaG + GrphSat *(Cos(GrphHue) * K9 + */
    /* Sin(GrphHue) * K10))*/
    rgb_matrix[5] = vpe_fixpt_mul(rgb_scale, rgb_matrix[5]);

    /* COEF_2_3 = GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K11 +*/
    /* Sin(GrphHue) * K12))*/
    /* (Cos(GrphHue) * K11 + Sin(GrphHue) * K12))*/
    rgb_matrix[6] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k11), vpe_fixpt_mul(sin_grph_hue, k12));
    /* GrphSat * (Cos(GrphHue) * K11 + Sin(GrphHue) * K12))*/
    rgb_matrix[6] = vpe_fixpt_mul(grph_sat, rgb_matrix[6]);
    /* (LumaB + GrphSat * (Cos(GrphHue) * K11 + Sin(GrphHue) * K12))*/
    rgb_matrix[6] = vpe_fixpt_add(luma_b, rgb_matrix[6]);
    /* GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K11 + Sin(GrphHue)**/
    /* K12))*/
    rgb_matrix[6] = vpe_fixpt_mul(grph_cont, rgb_matrix[6]);
    /* LumaScale * GrphCont  * (LumaB + GrphSat *(Cos(GrphHue) * K11 +*/
    /* Sin(GrphHue) * K12)) */
    rgb_matrix[6] = vpe_fixpt_mul(rgb_scale, rgb_matrix[6]);

    /* COEF_2_4 = RGBBias + RGBScale * GrphBright*/
    rgb_matrix[7] = vpe_fixpt_add(rgb_bias, vpe_fixpt_mul(rgb_scale, grph_bright));

    /* COEF_3_1 = GrphCont  * (LumaR + GrphSat * (Cos(GrphHue) * K13 +*/
    /* Sin(GrphHue) * K14))*/
    /* (Cos(GrphHue) * K13 + Sin(GrphHue) * K14)) */
    rgb_matrix[8] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k13), vpe_fixpt_mul(sin_grph_hue, k14));
    /* GrphSat * (Cos(GrphHue) * K13 + Sin(GrphHue) * K14)) */
    rgb_matrix[8] = vpe_fixpt_mul(grph_sat, rgb_matrix[8]);
    /* (LumaR + GrphSat * (Cos(GrphHue) * K13 + Sin(GrphHue) * K14)) */
    rgb_matrix[8] = vpe_fixpt_add(luma_r, rgb_matrix[8]);
    /* GrphCont  * (LumaR + GrphSat * (Cos(GrphHue) * K13 + Sin(GrphHue)**/
    /* K14)) */
    rgb_matrix[8] = vpe_fixpt_mul(grph_cont, rgb_matrix[8]);
    /* LumaScale * GrphCont * (LumaR + GrphSat * (Cos(GrphHue) * K13 +*/
    /* Sin(GrphHue) * K14))*/
    rgb_matrix[8] = vpe_fixpt_mul(rgb_scale, rgb_matrix[8]);

    /* COEF_3_2    = GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K15 +*/
    /* Sin(GrphHue) * K16)) */
    /* GrphSat * (Cos(GrphHue) * K15 + Sin(GrphHue) * K16) */
    rgb_matrix[9] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k15), vpe_fixpt_mul(sin_grph_hue, k16));
    /* (LumaG + GrphSat * (Cos(GrphHue) * K15 + Sin(GrphHue) * K16)) */
    rgb_matrix[9] = vpe_fixpt_mul(grph_sat, rgb_matrix[9]);
    /* (LumaG + GrphSat * (Cos(GrphHue) * K15 + Sin(GrphHue) * K16)) */
    rgb_matrix[9] = vpe_fixpt_add(luma_g, rgb_matrix[9]);
    /* GrphCont * (LumaG + GrphSat * (Cos(GrphHue) * K15 + Sin(GrphHue)**/
    /* K16)) */
    rgb_matrix[9] = vpe_fixpt_mul(grph_cont, rgb_matrix[9]);
    /* LumaScale * GrphCont * (LumaG + GrphSat *(Cos(GrphHue) * K15 + */
    /* Sin(GrphHue) * K16))*/
    rgb_matrix[9] = vpe_fixpt_mul(rgb_scale, rgb_matrix[9]);

    /*  COEF_3_3 = GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K17 +*/
    /* Sin(GrphHue) * K18)) */
    /* (Cos(GrphHue) * K17 + Sin(GrphHue) * K18)) */
    rgb_matrix[10] =
        vpe_fixpt_add(vpe_fixpt_mul(cos_grph_hue, k17), vpe_fixpt_mul(sin_grph_hue, k18));
    /*  GrphSat * (Cos(GrphHue) * K17 + Sin(GrphHue) * K18)) */
    rgb_matrix[10] = vpe_fixpt_mul(grph_sat, rgb_matrix[10]);
    /* (LumaB + GrphSat * (Cos(GrphHue) * K17 + Sin(GrphHue) * K18)) */
    rgb_matrix[10] = vpe_fixpt_add(luma_b, rgb_matrix[10]);
    /* GrphCont * (LumaB + GrphSat * (Cos(GrphHue) * K17 + Sin(GrphHue)**/
    /* K18)) */
    rgb_matrix[10] = vpe_fixpt_mul(grph_cont, rgb_matrix[10]);
    /* LumaScale * GrphCont * (LumaB + GrphSat *(Cos(GrphHue) * */
    /* K17 + Sin(GrphHue) * K18))*/
    rgb_matrix[10] = vpe_fixpt_mul(rgb_scale, rgb_matrix[10]);

    /* COEF_3_4 = RGBBias + RGBScale * GrphBright */
    rgb_matrix[11] = vpe_fixpt_add(rgb_bias, vpe_fixpt_mul(rgb_scale, grph_bright));
}

/* this function scales the matrix coefficients to fit a maximum integer bit range*/
static bool vpe_scale_csc_matrix(struct fixed31_32 *matrix, unsigned int matrixLength,
    unsigned int maxIntegerBits, struct fixed31_32 *scalingFactor)
{
    bool              ret            = false;
    unsigned int      index          = 0;
    long long         maxIntegerVal  = ((long long)1 << maxIntegerBits);
    long long         maxMatrixVal   = 0;
    unsigned int      crtIntPart     = 0;
    struct fixed31_32 divisionFactor = vpe_fixpt_one;
    long long         crtValue       = 0;
    unsigned int      posLargestBit  = 0;
    (*scalingFactor)                 = vpe_fixpt_one; // by default this is initialized to one
    for (index = 0; index < matrixLength; index++) {
        crtValue = matrix[index].value;
        if (crtValue < 0) {
            crtValue = -crtValue;
        }
        crtIntPart = (crtValue >> FIXED31_32_BITS_PER_FRACTIONAL_PART);
        if (maxMatrixVal < crtIntPart) {
            maxMatrixVal = crtIntPart;
        }
    }
    if ((maxMatrixVal >= maxIntegerVal) && (maxIntegerVal > 0)) {
        for (index = 0; index < (FIXED31_32_BITS_PER_FRACTIONAL_PART - 1); index++) {
            if (maxMatrixVal & ((long long)1 << index)) { // scan all the bits
                posLargestBit = index;
            }
        }
        divisionFactor.value = (long long)1 << (posLargestBit - maxIntegerBits + 1);
        divisionFactor.value <<= FIXED31_32_BITS_PER_FRACTIONAL_PART;
        (*scalingFactor) = divisionFactor;
        for (index = 0; index < matrixLength; index++) {
            matrix[index] = vpe_fixpt_div(matrix[index], divisionFactor);
        }
        ret = true;
    }
    return ret;
}

static void calculate_yuv_matrix(struct vpe_color_adjustments *vpe_adjust,
    enum color_space color_space, struct vpe_csc_matrix *input_cs, struct fixed31_32 *yuv_matrix)
{
    struct fixed31_32 initialMatrix[12];
    uint32_t          i = 0;
    bool ovl = true; // if we ever have Output CSC case, we can reuse this function with ovl passed
                     // in as param
    struct fixed31_32 grph_cont;
    struct fixed31_32 grph_sat;
    struct fixed31_32 grph_bright;
    struct fixed31_32 sin_grph_hue;
    struct fixed31_32 cos_grph_hue;
    struct fixed31_32 multiplier;
    struct fixed31_32 chromaOffset = vpe_fixpt_sub(vpe_fixpt_half, vpe_fixpt_one); // = -0.5
    struct fixed31_32 lumaOffset   = {
        0x10101010LL}; //=16/255.0 This is an offset applied in the shader, not clear why
                         // to maintain compatibility this offset is still applied in VPE

    /* The input YCbCr to RGB matrix is modified to embed the color adjustments as follows:
        A = initial YCbCr to RGB conversion matrix
        s = saturation , h = hue, c = contrast, b = brightness

                | c*s*(a11*cos(h)+a13*sin(h))   a12*c   c*s(a13*cos(h)-a11*sin(h))  |
        |R|     |                                                                   |   |Y+b   |
        |G|=    | c*s*(a21*cos(h)+a23*sin(h))   a22*c   c*s(a23*cos(h)-a21*sin(h))  | * |Cb-0.5|
        |B|     |                                                                   |   |Cr-0.5|
                | c*s*(a31*cos(h)+a33*sin(h))   a32*c   c*s(a33*cos(h)-a31*sin(h))  |
    */

    for (i = 0; i < 12; i++) {
        initialMatrix[i] = vpe_convfix31_32(input_cs->regval[i]); // convert from s.2.13 to s.31.32
    }
    color_adjustments_to_fixed_point(
        vpe_adjust, ovl, &grph_cont, &grph_sat, &grph_bright, &sin_grph_hue, &cos_grph_hue);
    grph_bright = vpe_fixpt_sub(grph_bright, lumaOffset);
    multiplier  = vpe_fixpt_mul(grph_cont, grph_sat); // contSat

    yuv_matrix[0] =
        vpe_fixpt_mul(multiplier, vpe_fixpt_add(vpe_fixpt_mul(initialMatrix[0], cos_grph_hue),
                                      vpe_fixpt_mul(initialMatrix[2], sin_grph_hue)));

    yuv_matrix[1] = vpe_fixpt_mul(initialMatrix[1], grph_cont);

    yuv_matrix[2] =
        vpe_fixpt_mul(multiplier, vpe_fixpt_sub(vpe_fixpt_mul(initialMatrix[2], cos_grph_hue),
                                      vpe_fixpt_mul(initialMatrix[0], sin_grph_hue)));

    yuv_matrix[3] = initialMatrix[3];

    yuv_matrix[4] =
        vpe_fixpt_mul(multiplier, vpe_fixpt_add(vpe_fixpt_mul(initialMatrix[4], cos_grph_hue),
                                      vpe_fixpt_mul(initialMatrix[6], sin_grph_hue)));

    yuv_matrix[5] = vpe_fixpt_mul(initialMatrix[5], grph_cont);

    yuv_matrix[6] =
        vpe_fixpt_mul(multiplier, vpe_fixpt_sub(vpe_fixpt_mul(initialMatrix[6], cos_grph_hue),
                                      vpe_fixpt_mul(initialMatrix[4], sin_grph_hue)));

    yuv_matrix[7] = initialMatrix[7];

    yuv_matrix[8] =
        vpe_fixpt_mul(multiplier, vpe_fixpt_add(vpe_fixpt_mul(initialMatrix[8], cos_grph_hue),
                                      vpe_fixpt_mul(initialMatrix[10], sin_grph_hue)));

    yuv_matrix[9] = vpe_fixpt_mul(initialMatrix[9], grph_cont);

    yuv_matrix[10] =
        vpe_fixpt_mul(multiplier, vpe_fixpt_sub(vpe_fixpt_mul(initialMatrix[10], cos_grph_hue),
                                      vpe_fixpt_mul(initialMatrix[8], sin_grph_hue)));

    yuv_matrix[3]  = vpe_fixpt_add(vpe_fixpt_mul(grph_bright, yuv_matrix[1]),
         vpe_fixpt_add(vpe_fixpt_mul(chromaOffset, yuv_matrix[0]),
             vpe_fixpt_mul(chromaOffset, yuv_matrix[2])));
    yuv_matrix[7]  = vpe_fixpt_add(vpe_fixpt_mul(grph_bright, yuv_matrix[5]),
         vpe_fixpt_add(vpe_fixpt_mul(chromaOffset, yuv_matrix[4]),
             vpe_fixpt_mul(chromaOffset, yuv_matrix[6])));
    yuv_matrix[11] = vpe_fixpt_add(vpe_fixpt_mul(grph_bright, yuv_matrix[9]),
        vpe_fixpt_add(vpe_fixpt_mul(chromaOffset, yuv_matrix[8]),
            vpe_fixpt_mul(chromaOffset, yuv_matrix[10])));
}

static void convert_float_matrix(uint16_t *matrix, struct fixed31_32 *flt, uint32_t buffer_size)
{
    const struct fixed31_32 min_2_13 = vpe_fixpt_from_fraction(S2D13_MIN, DIVIDER);
    const struct fixed31_32 max_2_13 = vpe_fixpt_from_fraction(S2D13_MAX, DIVIDER);
    uint32_t                i;
    uint16_t                temp_matrix[12];

    for (i = 0; i < 12; i++)
        temp_matrix[i] = 0;

    for (i = 0; i < buffer_size; ++i) {
        uint32_t reg_value =
            conv_fixed_point_to_int_frac(vpe_fixpt_clamp(flt[i], min_2_13, max_2_13), 2, 13);

        temp_matrix[i] = (uint16_t)reg_value;
    }

    matrix[4] = temp_matrix[0];
    matrix[5] = temp_matrix[1];
    matrix[6] = temp_matrix[2];
    matrix[7] = temp_matrix[3];

    matrix[8]  = temp_matrix[4];
    matrix[9]  = temp_matrix[5];
    matrix[10] = temp_matrix[6];
    matrix[11] = temp_matrix[7];

    matrix[0] = temp_matrix[8];
    matrix[1] = temp_matrix[9];
    matrix[2] = temp_matrix[10];
    matrix[3] = temp_matrix[11];
}

bool vpe_color_calculate_input_cs(struct vpe_priv *vpe_priv, enum color_space in_cs,
    const struct vpe_color_adjust *vpe_blt_adjust, struct vpe_csc_matrix *input_cs,
    struct fixed31_32 *matrix_scaling_factor)
{
    struct fixed31_32 fixed_csc_matrix[12];

    struct vpe_color_adjustments vpe_adjust = {0};

    if (vpe_blt_adjust) {
        translate_blt_to_internal_adjustments(vpe_blt_adjust, &vpe_adjust);
    }

    switch (in_cs) {
    case COLOR_SPACE_SRGB:
    case COLOR_SPACE_2020_RGB_FULLRANGE:
    case COLOR_SPACE_MSREF_SCRGB:
    case COLOR_SPACE_SRGB_LIMITED:
    case COLOR_SPACE_2020_RGB_LIMITEDRANGE:
        calculate_rgb_matrix_legacy(&vpe_adjust, fixed_csc_matrix);
        break;

    case COLOR_SPACE_YCBCR601:
    case COLOR_SPACE_YCBCR709:
    case COLOR_SPACE_YCBCR601_LIMITED:
    case COLOR_SPACE_YCBCR709_LIMITED:
    case COLOR_SPACE_2020_YCBCR:
        calculate_yuv_matrix(&vpe_adjust, in_cs, input_cs, fixed_csc_matrix);
        if (vpe_priv->scale_yuv_matrix) { // in case the coefficitents are too large
            // they are scaled down to fit the n integer bits, m
            // fractional bits (for now 2.19)
            vpe_log("Scale down YUV -> RGB matrix");
            vpe_scale_csc_matrix(fixed_csc_matrix, 12, 2, matrix_scaling_factor);
        } else {
            vpe_log("No scaling on the yuv -> rgb matrix");
        }
        break;

    default:
        calculate_rgb_matrix_legacy(&vpe_adjust, fixed_csc_matrix);
        break;
    }
    conv_convert_float_matrix(&input_cs->regval[0], fixed_csc_matrix, 12);

    return true;
}
