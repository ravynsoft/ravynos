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
#include "color_gamut.h"

#define DIVIDER 10000

struct gamut_space_entry {
    unsigned int redX;
    unsigned int redY;
    unsigned int greenX;
    unsigned int greenY;
    unsigned int blueX;
    unsigned int blueY;

    int a0;
    int a1;
    int a2;
    int a3;
    int gamma;
};

struct white_point_coodinates_entry {
    unsigned int temperature;
    unsigned int whiteX;
    unsigned int whiteY;
};

static const struct gamut_space_entry predefined_gamuts[] = {
    /*                         x_red y_red x_gr  y_gr  x_blue y_blue   a0        a1     a2  a3 gamma
     */
    [gamut_type_bt709]     = {6400, 3300, 3000, 6000, 1500, 600, 180000, 4500, 99, 99, 2222},
    [gamut_type_bt601]     = {6300, 3400, 3100, 5950, 1550, 700, 180000, 4500, 99, 99, 2200},
    [gamut_type_adobe_rgb] = {6400, 3300, 2100, 7100, 1500, 600, 180000, 4500, 99, 99, 2200},
    [gamut_type_srgb]      = {6400, 3300, 3000, 6000, 1500, 600, 31308, 12920, 55, 55, 2400},
    [gamut_type_bt2020]    = {7080, 2920, 1700, 7970, 1310, 460, 180000, 4500, 99, 99, 2200},
    [gamut_type_dcip3]     = {6800, 3200, 2650, 6900, 1500, 600, 0, 0, 0, 0, 2600}};

static const struct white_point_coodinates_entry predefined_white_points[] = {
    [white_point_type_5000k_horizon]   = {5000, 3473, 3561},
    [white_point_type_6500k_noon]      = {6500, 3127, 3290},
    [white_point_type_7500k_north_sky] = {7500, 3022, 3129},
    [white_point_type_9300k]           = {9300, 2866, 2950}};

struct gamut_src_dst_matrix {
    struct fixed31_32 rgbCoeffDst[9];
    struct fixed31_32 whiteCoeffDst[3];
    struct fixed31_32 rgbCoeffSrc[9];
    struct fixed31_32 whiteCoeffSrc[3];
    struct fixed31_32 xyzMatrix[9];
    struct fixed31_32 xyzOffset[3];
    struct fixed31_32 bradford[9];
};

struct gamut_calculation_matrix {
    struct fixed31_32 MTransposed[9];
    struct fixed31_32 XYZtoRGB_Custom[9];
    struct fixed31_32 XYZtoRGB_Ref[9];
    struct fixed31_32 RGBtoXYZ_Final[9];

    struct fixed31_32 MResult[9];
    struct fixed31_32 fXYZofWhiteRef[9];
    struct fixed31_32 fXYZofRGBRef[9];
    struct fixed31_32 fXYZofRGBRefCopy[9];
    struct fixed31_32 MResultOffset[3];
};

static void color_find_predefined_gamut(
    struct color_space_coordinates *out_gamut, enum predefined_gamut_type type)
{
    out_gamut->redX   = predefined_gamuts[type].redX;
    out_gamut->redY   = predefined_gamuts[type].redY;
    out_gamut->greenX = predefined_gamuts[type].greenX;
    out_gamut->greenY = predefined_gamuts[type].greenY;
    out_gamut->blueX  = predefined_gamuts[type].blueX;
    out_gamut->blueY  = predefined_gamuts[type].blueY;
}

static void color_find_predefined_white_point(
    struct color_space_coordinates *out_white_point, enum predefined_white_point_type type)
{
    out_white_point->whiteX = predefined_white_points[type].whiteX;
    out_white_point->whiteY = predefined_white_points[type].whiteY;
}

static void color_transpose_matrix(const struct fixed31_32 *M, unsigned int Rows, unsigned int Cols,
    struct fixed31_32 *MTransposed)
{
    unsigned int i, j;

    for (i = 0; i < Rows; i++) {
        for (j = 0; j < Cols; j++)
            MTransposed[(j * Rows) + i] = M[(i * Cols) + j];
    }
}

static void color_multiply_matrices(struct fixed31_32 *mResult, const struct fixed31_32 *M1,
    const struct fixed31_32 *M2, unsigned int Rows1, unsigned int Cols1, unsigned int Cols2)
{
    unsigned int i, j, k;

    for (i = 0; i < Rows1; i++) {
        for (j = 0; j < Cols2; j++) {
            mResult[(i * Cols2) + j] = vpe_fixpt_zero;
            for (k = 0; k < Cols1; k++)
                mResult[(i * Cols2) + j] = vpe_fixpt_add(mResult[(i * Cols2) + j],
                    vpe_fixpt_mul(M1[(i * Cols1) + k], M2[(k * Cols2) + j]));
        }
    }
}

static enum predefined_gamut_type color_space_to_predefined_gamut_types(
    enum color_space color_space)
{
    switch (color_space) {
    case COLOR_SPACE_JFIF:
    case COLOR_SPACE_YCBCR709:
    case COLOR_SPACE_YCBCR709_LIMITED:
        return gamut_type_bt709;
    case COLOR_SPACE_YCBCR601:
    case COLOR_SPACE_YCBCR601_LIMITED:
        return gamut_type_bt601;
    case COLOR_SPACE_SRGB:
    case COLOR_SPACE_SRGB_LIMITED:
    case COLOR_SPACE_MSREF_SCRGB:
        return gamut_type_srgb;
    case COLOR_SPACE_2020_RGB_FULLRANGE:
    case COLOR_SPACE_2020_RGB_LIMITEDRANGE:
    case COLOR_SPACE_2020_YCBCR:
        return gamut_type_bt2020;
    default:
        VPE_ASSERT(0);
        return gamut_type_unknown;
    }
}

static enum vpe_status find_predefined_gamut_and_white_point(
    struct vpe_priv *vpe_priv, struct color_gamut_data *gamut, enum color_space color_space)
{
    enum predefined_gamut_type gamut_type;

    gamut->color_space = color_space;

    gamut_type = color_space_to_predefined_gamut_types(color_space);
    if (gamut_type == gamut_type_unknown) {
        vpe_log("err: color space not supported! %d %d\n", (int)color_space, (int)gamut_type);
        return VPE_STATUS_COLOR_SPACE_VALUE_NOT_SUPPORTED;
    }

    color_find_predefined_gamut(&gamut->gamut, gamut_type);
    gamut->white_point = color_white_point_type_6500k_noon;
    color_find_predefined_white_point(&gamut->gamut, white_point_type_6500k_noon);

    return VPE_STATUS_OK;
}

static bool build_gamut_remap_matrix(struct color_space_coordinates gamut_description,
    struct fixed31_32 *rgb_matrix, struct fixed31_32 *white_point_matrix)
{
    struct fixed31_32 fixed_blueX  = vpe_fixpt_from_fraction(gamut_description.blueX, DIVIDER);
    struct fixed31_32 fixed_blueY  = vpe_fixpt_from_fraction(gamut_description.blueY, DIVIDER);
    struct fixed31_32 fixed_greenX = vpe_fixpt_from_fraction(gamut_description.greenX, DIVIDER);
    struct fixed31_32 fixed_greenY = vpe_fixpt_from_fraction(gamut_description.greenY, DIVIDER);
    struct fixed31_32 fixed_redX   = vpe_fixpt_from_fraction(gamut_description.redX, DIVIDER);
    struct fixed31_32 fixed_redY   = vpe_fixpt_from_fraction(gamut_description.redY, DIVIDER);
    struct fixed31_32 fixed_whiteX = vpe_fixpt_from_fraction(gamut_description.whiteX, DIVIDER);
    struct fixed31_32 fixed_whiteY = vpe_fixpt_from_fraction(gamut_description.whiteY, DIVIDER);

    rgb_matrix[0] = vpe_fixpt_div(fixed_redX, fixed_redY);
    rgb_matrix[1] = vpe_fixpt_one;
    rgb_matrix[2] = vpe_fixpt_div(
        vpe_fixpt_sub(vpe_fixpt_sub(vpe_fixpt_one, fixed_redX), fixed_redY), fixed_redY);

    rgb_matrix[3] = vpe_fixpt_div(fixed_greenX, fixed_greenY);
    rgb_matrix[4] = vpe_fixpt_one;
    rgb_matrix[5] = vpe_fixpt_div(
        vpe_fixpt_sub(vpe_fixpt_sub(vpe_fixpt_one, fixed_greenX), fixed_greenY), fixed_greenY);

    rgb_matrix[6] = vpe_fixpt_div(fixed_blueX, fixed_blueY);
    rgb_matrix[7] = vpe_fixpt_one;
    rgb_matrix[8] = vpe_fixpt_div(
        vpe_fixpt_sub(vpe_fixpt_sub(vpe_fixpt_one, fixed_blueX), fixed_blueY), fixed_blueY);

    white_point_matrix[0] = vpe_fixpt_div(fixed_whiteX, fixed_whiteY);
    white_point_matrix[1] = vpe_fixpt_one;
    white_point_matrix[2] = vpe_fixpt_div(
        vpe_fixpt_sub(vpe_fixpt_sub(vpe_fixpt_one, fixed_whiteX), fixed_whiteY), fixed_whiteY);

    return true;
}

static struct fixed31_32 find_3X3_det(const struct fixed31_32 *m)
{
    struct fixed31_32 det, A1, A2, A3;

    A1  = vpe_fixpt_mul(m[0], vpe_fixpt_sub(vpe_fixpt_mul(m[4], m[8]), vpe_fixpt_mul(m[5], m[7])));
    A2  = vpe_fixpt_mul(m[1], vpe_fixpt_sub(vpe_fixpt_mul(m[3], m[8]), vpe_fixpt_mul(m[5], m[6])));
    A3  = vpe_fixpt_mul(m[2], vpe_fixpt_sub(vpe_fixpt_mul(m[3], m[7]), vpe_fixpt_mul(m[4], m[6])));
    det = vpe_fixpt_add(vpe_fixpt_sub(A1, A2), A3);
    return det;
}

static bool compute_inverse_matrix_3x3(const struct fixed31_32 *m, struct fixed31_32 *im)
{
    struct fixed31_32 determinant = find_3X3_det(m);

    if (vpe_fixpt_eq(determinant, vpe_fixpt_zero) == false) {
        im[0] = vpe_fixpt_div(
            vpe_fixpt_sub(vpe_fixpt_mul(m[4], m[8]), vpe_fixpt_mul(m[5], m[7])), determinant);
        im[1] = vpe_fixpt_neg(vpe_fixpt_div(
            vpe_fixpt_sub(vpe_fixpt_mul(m[1], m[8]), vpe_fixpt_mul(m[2], m[7])), determinant));
        im[2] = vpe_fixpt_div(
            vpe_fixpt_sub(vpe_fixpt_mul(m[1], m[5]), vpe_fixpt_mul(m[2], m[4])), determinant);
        im[3] = vpe_fixpt_neg(vpe_fixpt_div(
            vpe_fixpt_sub(vpe_fixpt_mul(m[3], m[8]), vpe_fixpt_mul(m[5], m[6])), determinant));
        im[4] = vpe_fixpt_div(
            vpe_fixpt_sub(vpe_fixpt_mul(m[0], m[8]), vpe_fixpt_mul(m[2], m[6])), determinant);
        im[5] = vpe_fixpt_neg(vpe_fixpt_div(
            vpe_fixpt_sub(vpe_fixpt_mul(m[0], m[5]), vpe_fixpt_mul(m[2], m[3])), determinant));
        im[6] = vpe_fixpt_div(
            vpe_fixpt_sub(vpe_fixpt_mul(m[3], m[7]), vpe_fixpt_mul(m[4], m[6])), determinant);
        im[7] = vpe_fixpt_neg(vpe_fixpt_div(
            vpe_fixpt_sub(vpe_fixpt_mul(m[0], m[7]), vpe_fixpt_mul(m[1], m[6])), determinant));
        im[8] = vpe_fixpt_div(
            vpe_fixpt_sub(vpe_fixpt_mul(m[0], m[4]), vpe_fixpt_mul(m[1], m[3])), determinant);
        return true;
    }
    return false;
}

static bool calculate_XYZ_to_RGB_3x3(const struct fixed31_32 *XYZofRGB,
    const struct fixed31_32 *XYZofWhite, struct fixed31_32 *XYZtoRGB)
{

    struct fixed31_32 MInversed[9];
    struct fixed31_32 SVector[3];

    /*1. Find Inverse matrix 3x3 of MTransposed*/
    if (!compute_inverse_matrix_3x3(XYZofRGB, MInversed))
        return false;

    /*2. Calculate vector: |Sr Sg Sb| = [MInversed] * |Wx Wy Wz|*/
    color_multiply_matrices(SVector, MInversed, XYZofWhite, 3, 3, 1);

    /*3. Calculate matrix XYZtoRGB 3x3*/
    XYZtoRGB[0] = vpe_fixpt_mul(XYZofRGB[0], SVector[0]);
    XYZtoRGB[1] = vpe_fixpt_mul(XYZofRGB[1], SVector[1]);
    XYZtoRGB[2] = vpe_fixpt_mul(XYZofRGB[2], SVector[2]);

    XYZtoRGB[3] = vpe_fixpt_mul(XYZofRGB[3], SVector[0]);
    XYZtoRGB[4] = vpe_fixpt_mul(XYZofRGB[4], SVector[1]);
    XYZtoRGB[5] = vpe_fixpt_mul(XYZofRGB[5], SVector[2]);

    XYZtoRGB[6] = vpe_fixpt_mul(XYZofRGB[6], SVector[0]);
    XYZtoRGB[7] = vpe_fixpt_mul(XYZofRGB[7], SVector[1]);
    XYZtoRGB[8] = vpe_fixpt_mul(XYZofRGB[8], SVector[2]);

    return true;
}

static bool gamut_to_color_matrix(struct vpe_priv *vpe_priv,
    const struct gamut_src_dst_matrix *matrices, bool invert, struct fixed31_32 *tempMatrix3X3,
    struct fixed31_32 *tempOffset)
{
    int                              i      = 0;
    struct gamut_calculation_matrix *matrix = vpe_zalloc(sizeof(struct gamut_calculation_matrix));

    const struct fixed31_32 *pXYZofRGB    = matrices->rgbCoeffDst;   /*destination gamut*/
    const struct fixed31_32 *pXYZofWhite  = matrices->whiteCoeffDst; /*destination of white point*/
    const struct fixed31_32 *pRefXYZofRGB = matrices->rgbCoeffSrc;   /*source gamut*/
    const struct fixed31_32 *pRefXYZofWhite     = matrices->whiteCoeffSrc; /*source of white point*/
    const struct fixed31_32 *pColorTransformXYZ = matrices->xyzMatrix; /*additional XYZ->XYZ tfm*/
    const struct fixed31_32 *pColorTransformXYZOffset = matrices->xyzOffset; /*XYZ tfm offset*/
    const struct fixed31_32 *pBradford = matrices->bradford; /*Bradford chromatic adaptation*/

    struct fixed31_32 *pXYZtoRGB_Temp;
    struct fixed31_32 *pXYZtoRGB_Final;

    if (!matrix)
        return false;

    matrix->fXYZofWhiteRef[0] = pRefXYZofWhite[0];
    matrix->fXYZofWhiteRef[1] = pRefXYZofWhite[1];
    matrix->fXYZofWhiteRef[2] = pRefXYZofWhite[2];

    matrix->fXYZofRGBRef[0] = pRefXYZofRGB[0];
    matrix->fXYZofRGBRef[1] = pRefXYZofRGB[1];
    matrix->fXYZofRGBRef[2] = pRefXYZofRGB[2];

    matrix->fXYZofRGBRef[3] = pRefXYZofRGB[3];
    matrix->fXYZofRGBRef[4] = pRefXYZofRGB[4];
    matrix->fXYZofRGBRef[5] = pRefXYZofRGB[5];

    matrix->fXYZofRGBRef[6] = pRefXYZofRGB[6];
    matrix->fXYZofRGBRef[7] = pRefXYZofRGB[7];
    matrix->fXYZofRGBRef[8] = pRefXYZofRGB[8];

    /*default values -  unity matrix*/
    while (i < 9) {
        if (i == 0 || i == 4 || i == 8)
            tempMatrix3X3[i] = vpe_fixpt_one;
        else
            tempMatrix3X3[i] = vpe_fixpt_zero;
        i++;
    }

    /*1. Decide about the order of calculation.
     * bInvert == FALSE --> RGBtoXYZ_Ref * XYZtoRGB_Custom
     * bInvert == TRUE  --> RGBtoXYZ_Custom * XYZtoRGB_Ref */
    if (invert) {
        pXYZtoRGB_Temp  = matrix->XYZtoRGB_Custom;
        pXYZtoRGB_Final = matrix->XYZtoRGB_Ref;
    } else {
        pXYZtoRGB_Temp  = matrix->XYZtoRGB_Ref;
        pXYZtoRGB_Final = matrix->XYZtoRGB_Custom;
    }

    /*2. Calculate XYZtoRGB_Ref*/
    color_transpose_matrix(matrix->fXYZofRGBRef, 3, 3, matrix->MTransposed);

    if (!calculate_XYZ_to_RGB_3x3(
            matrix->MTransposed, matrix->fXYZofWhiteRef, matrix->XYZtoRGB_Ref))
        goto function_fail;

    /*3. Calculate XYZtoRGB_Custom*/
    color_transpose_matrix(pXYZofRGB, 3, 3, matrix->MTransposed);

    if (!calculate_XYZ_to_RGB_3x3(matrix->MTransposed, pXYZofWhite, matrix->XYZtoRGB_Custom))
        goto function_fail;

    /*4. Calculate RGBtoXYZ -
     * inverse matrix 3x3 of XYZtoRGB_Ref or XYZtoRGB_Custom*/
    if (!compute_inverse_matrix_3x3(pXYZtoRGB_Temp, matrix->RGBtoXYZ_Final))
        goto function_fail;

    /* The naming is a bit confusing here (and earlier as well if you're
     * trying to follow RP 177-1993...), so in short:
     *      S - source->XYZ
     *      D - dest->XYZ
     *      At this point:
     *      D^-1 = RGBtoXYZ_Final
     *      S = XYZtoRGB_Ref == pXYZtoRGB_Final
     */

    /*5. Calculate M(3x3) = RGBtoXYZ * XYZtoRGB*/
    color_multiply_matrices(matrix->MResult, matrix->RGBtoXYZ_Final, pXYZtoRGB_Final, 3, 3, 3);

    /*7. Calculate offsets */
    for (i = 0; i < 9; i++)
        tempMatrix3X3[i] = matrix->MResult[i];

    for (i = 0; i < 3; i++)
        tempOffset[i] = vpe_fixpt_zero;

    vpe_free(matrix);
    return true;

function_fail:
    vpe_free(matrix);
    return false;
}

static bool color_build_gamut_remap_matrix(struct vpe_priv *vpe_priv,
    struct color_gamut_data *source_gamut, struct color_gamut_data *destination_gamut,
    struct colorspace_transform *gamut_remap_matrix)
{
    struct gamut_src_dst_matrix *matrix = NULL;
    struct fixed31_32            gamut_result[12];
    struct fixed31_32            temp_matrix[9];
    struct fixed31_32            temp_offset[3];
    int                          j;

    matrix = vpe_zalloc(sizeof(struct gamut_src_dst_matrix));
    if (matrix == NULL)
        return false;

    build_gamut_remap_matrix(source_gamut->gamut, matrix->rgbCoeffSrc, matrix->whiteCoeffSrc);
    build_gamut_remap_matrix(destination_gamut->gamut, matrix->rgbCoeffDst, matrix->whiteCoeffDst);

    if (!gamut_to_color_matrix(vpe_priv, matrix, true, temp_matrix, temp_offset))
        goto function_fail;

    gamut_result[0]  = temp_matrix[0];
    gamut_result[1]  = temp_matrix[1];
    gamut_result[2]  = temp_matrix[2];
    gamut_result[4]  = temp_matrix[3];
    gamut_result[5]  = temp_matrix[4];
    gamut_result[6]  = temp_matrix[5];
    gamut_result[8]  = temp_matrix[6];
    gamut_result[9]  = temp_matrix[7];
    gamut_result[10] = temp_matrix[8];

    gamut_result[3]  = temp_offset[0];
    gamut_result[7]  = temp_offset[1];
    gamut_result[11] = temp_offset[2];

    gamut_remap_matrix->enable_remap = true;

    for (j = 0; j < 12; j++)
        gamut_remap_matrix->matrix[j] = gamut_result[j];

    vpe_free(matrix);
    return true;

function_fail:
    vpe_free(matrix);
    vpe_log("err: build gamut remap fails!\n");
    return false;
}

enum vpe_status vpe_color_update_gamut(struct vpe_priv *vpe_priv, enum color_space in_color,
    enum color_space outColor, struct colorspace_transform *gamut_remap, bool bypass_remap)
{
    struct output_ctx      *output_ctx = &vpe_priv->output_ctx;
    struct color_gamut_data src_gamut;
    struct color_gamut_data dst_gamut;
    enum vpe_status         status;

    if (bypass_remap || in_color == outColor) {
        gamut_remap->enable_remap = false;
        return VPE_STATUS_OK;
    }

    status = find_predefined_gamut_and_white_point(vpe_priv, &src_gamut, in_color);
    if (status != VPE_STATUS_OK)
        return status;

    status = find_predefined_gamut_and_white_point(vpe_priv, &dst_gamut, outColor);
    if (status != VPE_STATUS_OK)
        return status;

    if (!color_build_gamut_remap_matrix(vpe_priv, &src_gamut, &dst_gamut, gamut_remap)) {
        vpe_log("err: build gamut remap failure!");
        VPE_ASSERT(0);
        return VPE_STATUS_ERROR;
    }

    return VPE_STATUS_OK;
}
