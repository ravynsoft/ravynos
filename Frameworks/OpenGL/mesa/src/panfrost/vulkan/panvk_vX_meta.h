/*
 * Copyright (C) 2021 Collabora Ltd.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef PANVK_PRIVATE_H
#error "Must be included from panvk_private.h"
#endif

#ifndef PAN_ARCH
#error "no arch"
#endif

void panvk_per_arch(meta_init)(struct panvk_physical_device *dev);

void panvk_per_arch(meta_cleanup)(struct panvk_physical_device *dev);

mali_ptr panvk_per_arch(meta_emit_viewport)(struct pan_pool *pool,
                                            uint16_t minx, uint16_t miny,
                                            uint16_t maxx, uint16_t maxy);

void panvk_per_arch(meta_clear_init)(struct panvk_physical_device *dev);

void panvk_per_arch(meta_blit_init)(struct panvk_physical_device *dev);

void panvk_per_arch(meta_blit_cleanup)(struct panvk_physical_device *dev);

void panvk_per_arch(meta_copy_init)(struct panvk_physical_device *dev);
