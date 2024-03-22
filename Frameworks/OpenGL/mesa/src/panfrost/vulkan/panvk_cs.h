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

#ifndef PANVK_CS_H
#define PANVK_CS_H

#include "pan_encoder.h"

#include <vulkan/vulkan.h>

#include "compiler/shader_enums.h"
#include "pan_desc.h"
#include "panfrost-job.h"

#include "vk_util.h"

#include "panvk_private.h"

struct pan_blend_state;
struct pan_shader_info;
struct panfrost_ptr;
struct pan_pool;

union panvk_sysval_data;
struct panvk_framebuffer;
struct panvk_cmd_state;
struct panvk_compute_dim;
struct panvk_device;
struct panvk_batch;
struct panvk_varyings_info;
struct panvk_attrib_buf;
struct panvk_attribs_info;
struct panvk_pipeline;
struct panvk_draw_info;
struct panvk_descriptor_state;
struct panvk_subpass;
struct panvk_clear_value;

#ifdef PAN_ARCH
static inline enum mali_func
panvk_per_arch(translate_compare_func)(VkCompareOp comp)
{
   STATIC_ASSERT(VK_COMPARE_OP_NEVER == (VkCompareOp)MALI_FUNC_NEVER);
   STATIC_ASSERT(VK_COMPARE_OP_LESS == (VkCompareOp)MALI_FUNC_LESS);
   STATIC_ASSERT(VK_COMPARE_OP_EQUAL == (VkCompareOp)MALI_FUNC_EQUAL);
   STATIC_ASSERT(VK_COMPARE_OP_LESS_OR_EQUAL == (VkCompareOp)MALI_FUNC_LEQUAL);
   STATIC_ASSERT(VK_COMPARE_OP_GREATER == (VkCompareOp)MALI_FUNC_GREATER);
   STATIC_ASSERT(VK_COMPARE_OP_NOT_EQUAL == (VkCompareOp)MALI_FUNC_NOT_EQUAL);
   STATIC_ASSERT(VK_COMPARE_OP_GREATER_OR_EQUAL ==
                 (VkCompareOp)MALI_FUNC_GEQUAL);
   STATIC_ASSERT(VK_COMPARE_OP_ALWAYS == (VkCompareOp)MALI_FUNC_ALWAYS);

   return (enum mali_func)comp;
}

static inline enum mali_func
panvk_per_arch(translate_sampler_compare_func)(
   const VkSamplerCreateInfo *pCreateInfo)
{
   if (!pCreateInfo->compareEnable)
      return MALI_FUNC_NEVER;

   enum mali_func f =
      panvk_per_arch(translate_compare_func)(pCreateInfo->compareOp);
   return panfrost_flip_compare_func(f);
}
#endif

void panvk_sysval_upload_viewport_scale(const VkViewport *viewport,
                                        union panvk_sysval_vec4 *data);

void panvk_sysval_upload_viewport_offset(const VkViewport *viewport,
                                         union panvk_sysval_vec4 *data);

#endif
