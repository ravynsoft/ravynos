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

#include "fixed31_32.h"
#include "color_table.h"

#ifdef __cplusplus
extern "C" {
#endif

struct color_range {
    int current;
    int min;
    int max;
};

struct vpe_color_adjustments {
    struct color_range contrast;
    struct color_range saturation;
    struct color_range brightness;
    struct color_range hue;
};

bool vpe_color_calculate_input_cs(struct vpe_priv *vpe_priv, enum color_space in_cs,
    const struct vpe_color_adjust *vpe_adjust, struct vpe_csc_matrix *input_cs,
    struct fixed31_32 *matrix_scaling_factor);

bool vpe_color_different_color_adjusts(
    const struct vpe_color_adjust *new_vpe_adjusts, struct vpe_color_adjust *crt_vpe_adjusts);

void vpe_color_set_adjustments_to_default(struct vpe_color_adjust *crt_vpe_adjusts);

#ifdef __cplusplus
}
#endif
