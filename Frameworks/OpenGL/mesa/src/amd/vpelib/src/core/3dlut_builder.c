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

#include "3dlut_builder.h"

static void convert_3dlut_to_tetrahedral_params(
    struct vpe_rgb *rgb, bool is_17x17x17, bool is_12_bits, struct tetrahedral_params *params)
{
    struct vpe_rgb *lut0;
    struct vpe_rgb *lut1;
    struct vpe_rgb *lut2;
    struct vpe_rgb *lut3;
    int             i, lut_i;

    int num_values;

    if (is_17x17x17 == false) {
        lut0       = params->tetrahedral_9.lut0;
        lut1       = params->tetrahedral_9.lut1;
        lut2       = params->tetrahedral_9.lut2;
        lut3       = params->tetrahedral_9.lut3;
        num_values = LUT3D_SIZE_9x9x9;
    } else {
        lut0       = params->tetrahedral_17.lut0;
        lut1       = params->tetrahedral_17.lut1;
        lut2       = params->tetrahedral_17.lut2;
        lut3       = params->tetrahedral_17.lut3;
        num_values = LUT3D_SIZE_17x17x17;
    }

    for (lut_i = 0, i = 0; i < num_values - 4; lut_i++, i += 4) {
        lut0[lut_i].red   = rgb[i].red;
        lut0[lut_i].green = rgb[i].green;
        lut0[lut_i].blue  = rgb[i].blue;

        lut1[lut_i].red   = rgb[i + 1].red;
        lut1[lut_i].green = rgb[i + 1].green;
        lut1[lut_i].blue  = rgb[i + 1].blue;

        lut2[lut_i].red   = rgb[i + 2].red;
        lut2[lut_i].green = rgb[i + 2].green;
        lut2[lut_i].blue  = rgb[i + 2].blue;

        lut3[lut_i].red   = rgb[i + 3].red;
        lut3[lut_i].green = rgb[i + 3].green;
        lut3[lut_i].blue  = rgb[i + 3].blue;
    }
    lut0[lut_i].red   = rgb[i].red;
    lut0[lut_i].green = rgb[i].green;
    lut0[lut_i].blue  = rgb[i].blue;

    params->use_12bits        = is_12_bits;
    params->use_tetrahedral_9 = !is_17x17x17;
}

bool convert_to_tetrahedral(struct vpe_priv *vpe_priv, uint16_t rgb_lib[17 * 17 * 17 * 3],
    struct vpe_3dlut *params, bool enable_3dlut)
{

    if (!enable_3dlut) {
        params->state.bits.initialized = 0;
        return true;
    }

    bool            ret      = false;
    struct vpe_rgb *rgb_area = NULL;
    int             ind      = 0;
    int             ind_lut  = 0;
    int             nir, nig, nib;

    rgb_area = (struct vpe_rgb *)vpe_zalloc(sizeof(struct vpe_rgb) * 17 * 17 * 17);
    if (rgb_area == NULL)
        goto release;

    memset(rgb_area, 0, 17 * 17 * 17 * sizeof(struct vpe_rgb));

    for (nib = 0; nib < 17; nib++) {
        for (nig = 0; nig < 17; nig++) {
            for (nir = 0; nir < 17; nir++) {
                ind_lut = 3 * (nib + 17 * nig + 289 * nir);

                rgb_area[ind].red   = rgb_lib[ind_lut + 0];
                rgb_area[ind].green = rgb_lib[ind_lut + 1];
                rgb_area[ind].blue  = rgb_lib[ind_lut + 2];
                ind++;
            }
        }
    }
    convert_3dlut_to_tetrahedral_params(rgb_area, true, true, &params->lut_3d);
    params->state.bits.initialized = 1;

    vpe_free(rgb_area);
    ret = true;
release:
    return ret;
}
