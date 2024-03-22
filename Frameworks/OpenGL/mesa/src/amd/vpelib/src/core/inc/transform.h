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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "vpe_hw_types.h"
#include "fixed31_32.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VPE_GAMUT_REMAP_MATRIX_SIZE 12

enum gamut_adjust_type {
    GAMUT_ADJUST_TYPE_BYPASS = 0,
    GAMUT_ADJUST_TYPE_SW /* use adjustments */
};

struct gamut_remap_matrix {
    struct fixed31_32      matrix[VPE_GAMUT_REMAP_MATRIX_SIZE];
    enum gamut_adjust_type adjust_type;
};

enum lb_memory_config {
    /* Enable all 3 pieces of memory */
    LB_MEMORY_CONFIG_0 = 0,

    /* Enable only the first piece of memory */
    LB_MEMORY_CONFIG_1 = 1,

    /* Enable only the second piece of memory */
    LB_MEMORY_CONFIG_2 = 2,

    /* Only applicable in 4:2:0 mode, enable all 3 pieces of memory and the
     * last piece of chroma memory used for the luma storage
     */
    LB_MEMORY_CONFIG_3 = 3
};

struct scaling_ratios {
    struct fixed31_32 horz;
    struct fixed31_32 vert;
    struct fixed31_32 horz_c;
    struct fixed31_32 vert_c;
};

struct sharpness_adj {
    int horz;
    int vert;
};

struct line_buffer_params {
    bool alpha_en;
};

struct scl_inits {
    struct fixed31_32 h;
    struct fixed31_32 h_c;
    struct fixed31_32 v;
    struct fixed31_32 v_c;
};

struct scaler_data {
    uint32_t                          h_active;
    uint32_t                          v_active;
    struct vpe_scaling_taps           taps;
    struct vpe_rect                   viewport;
    struct vpe_rect                   viewport_c;
    struct vpe_rect                   dst_viewport;
    struct vpe_rect                   dst_viewport_c;
    struct vpe_rect                   recout;
    struct scaling_ratios             ratios;
    struct scl_inits                  inits;
    struct sharpness_adj              sharpness;
    enum vpe_surface_pixel_format     format;
    struct line_buffer_params         lb_params;
    struct vpe_scaling_filter_coeffs *polyphase_filter_coeffs;
};

const uint16_t *vpe_get_filter_2tap_64p(void);
const uint16_t *vpe_get_2tap_bilinear_64p(void);
const uint16_t *vpe_get_filter_4tap_64p(struct fixed31_32 ratio);
const uint16_t *vpe_get_filter_6tap_64p(struct fixed31_32 ratio);
const uint16_t *vpe_get_filter_8tap_64p(struct fixed31_32 ratio);

#ifdef __cplusplus
}
#endif
