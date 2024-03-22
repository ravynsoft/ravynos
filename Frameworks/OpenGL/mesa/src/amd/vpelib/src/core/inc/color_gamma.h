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

struct calculate_buffer {
    int               buffer_index;
    struct fixed31_32 buffer[NUM_PTS_IN_REGION];
    struct fixed31_32 gamma_of_2;
};

struct translate_from_linear_space_args {
    struct fixed31_32        arg;
    struct fixed31_32        a0;
    struct fixed31_32        a1;
    struct fixed31_32        a2;
    struct fixed31_32        a3;
    struct fixed31_32        gamma;
    struct calculate_buffer *cal_buffer;
};

void vpe_color_setup_x_points_distribution(void);

void vpe_color_setup_x_points_distribution_degamma(void);

bool vpe_color_calculate_regamma_params(struct vpe_priv *vpe_priv, struct transfer_func *output_tf,
    struct calculate_buffer *cal_buffer);

bool vpe_color_calculate_degamma_params(struct vpe_priv *vpe_priv, struct fixed31_32 x_scale,
    struct fixed31_32 y_scale, struct transfer_func *input_tf);

void vpe_compute_pq(struct fixed31_32 in_x, struct fixed31_32 *out_y);

#ifdef __cplusplus
}
#endif
