/*
 * Copyright (c) 2018 Intel Corporation
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#ifndef ISL_GFX12_H
#define ISL_GFX12_H

#include "isl.h"

#ifdef __cplusplus
extern "C" {
#endif

void
isl_gfx125_filter_tiling(const struct isl_device *dev,
                         const struct isl_surf_init_info *restrict info,
                         isl_tiling_flags_t *flags);

void
isl_gfx125_choose_image_alignment_el(const struct isl_device *dev,
                                     const struct isl_surf_init_info *restrict info,
                                     enum isl_tiling tiling,
                                     enum isl_dim_layout dim_layout,
                                     enum isl_msaa_layout msaa_layout,
                                     struct isl_extent3d *image_align_el);

void
isl_gfx12_choose_image_alignment_el(const struct isl_device *dev,
                                    const struct isl_surf_init_info *restrict info,
                                    enum isl_tiling tiling,
                                    enum isl_dim_layout dim_layout,
                                    enum isl_msaa_layout msaa_layout,
                                    struct isl_extent3d *image_align_el);

#ifdef __cplusplus
}
#endif

#endif /* ISL_GFX12_H */
