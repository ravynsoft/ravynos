/*
 * Copyright © 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * NOTE: The header can be included multiple times, from the same file.
 */

/*
 * Gen-specific function declarations.  This header must *not* be included
 * directly.  Instead, it is included multiple times by isl_priv.h
 *
 * In this header file, the usual isl_genX() macro is available.
 */

void
isl_genX(surf_fill_state_s)(const struct isl_device *dev, void *state,
                            const struct isl_surf_fill_state_info *restrict info);

void
isl_genX(buffer_fill_state_s)(const struct isl_device *dev, void *state,
                              const struct isl_buffer_fill_state_info *restrict info);

void
isl_genX(emit_depth_stencil_hiz_s)(const struct isl_device *dev, void *batch,
                                   const struct isl_depth_stencil_hiz_emit_info *restrict info);

void
isl_genX(null_fill_state_s)(const struct isl_device *dev, void *state,
                            const struct isl_null_fill_state_info *restrict info);

void
isl_genX(emit_cpb_control_s)(const struct isl_device *dev, void *batch,
                             const struct isl_cpb_emit_info *restrict info);
