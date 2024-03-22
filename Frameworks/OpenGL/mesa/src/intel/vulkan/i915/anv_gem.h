/*
 * Copyright © 2023 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <stdint.h>

#include "vulkan/vulkan_core.h"

struct anv_bo;
struct anv_device;
enum anv_bo_alloc_flags;

int anv_i915_gem_get_tiling(struct anv_device *device, uint32_t gem_handle);
int anv_i915_gem_set_tiling(struct anv_device *device, uint32_t gem_handle,
                            uint32_t stride, uint32_t tiling);

int anv_i915_gem_wait(struct anv_device *device, uint32_t gem_handle,
                      int64_t *timeout_ns);

VkResult anv_i915_gem_import_bo_alloc_flags_to_bo_flags(struct anv_device *device,
                                                        struct anv_bo *bo,
                                                        enum anv_bo_alloc_flags alloc_flags,
                                                        uint32_t *out_bo_flags);
