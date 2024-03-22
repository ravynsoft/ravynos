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

#include "vpe_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cdc;
struct vpe_priv;

/** note: all program_* functions shall return number of config packet created */

struct cdc_funcs {
    bool (*check_input_format)(struct cdc *cdc, enum vpe_surface_pixel_format format);

    bool (*check_output_format)(struct cdc *cdc, enum vpe_surface_pixel_format format);

    /** non segment specific */
    void (*program_surface_config)(struct cdc *cdc, enum vpe_surface_pixel_format format,
        enum vpe_rotation_angle rotation, bool horizontal_mirror,
        enum vpe_swizzle_mode_values swizzle);

    void (*program_crossbar_config)(struct cdc *cdc, enum vpe_surface_pixel_format format);

    void (*program_global_sync)(
        struct cdc *cdc, uint32_t vupdate_offset, uint32_t vupdate_width, uint32_t vready_offset);

    void (*program_p2b_config)(struct cdc *cdc, enum vpe_surface_pixel_format format);

    /** segment specific */
    void (*program_viewport)(
        struct cdc *cdc, const struct vpe_rect *viewport, const struct vpe_rect *viewport_c);
};

struct cdc {
    struct vpe_priv  *vpe_priv;
    struct cdc_funcs *funcs;
};

#ifdef __cplusplus
}
#endif
