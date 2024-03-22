/*
 * Copyright © 2022 Collabora, LTD
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

#ifndef VK_PIPELINE_H
#define VK_PIPELINE_H

#include "vulkan/vulkan_core.h"
#include "vk_util.h"

#include <stdbool.h>

struct nir_shader;
struct nir_shader_compiler_options;
struct spirv_to_nir_options;
struct vk_device;

#ifdef __cplusplus
extern "C" {
#endif

#define VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NIR_CREATE_INFO_MESA \
   (VkStructureType)1000290001

#define VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NIR_CREATE_INFO_MESA_cast \
   VkPipelineShaderStageNirCreateInfoMESA

typedef struct VkPipelineShaderStageNirCreateInfoMESA {
   VkStructureType sType;
   const void *pNext;
   struct nir_shader *nir;
} VkPipelineShaderStageNirCreateInfoMESA;

bool
vk_pipeline_shader_stage_is_null(const VkPipelineShaderStageCreateInfo *info);

VkResult
vk_pipeline_shader_stage_to_nir(struct vk_device *device,
                                const VkPipelineShaderStageCreateInfo *info,
                                const struct spirv_to_nir_options *spirv_options,
                                const struct nir_shader_compiler_options *nir_options,
                                void *mem_ctx, struct nir_shader **nir_out);

struct vk_pipeline_robustness_state {
   VkPipelineRobustnessBufferBehaviorEXT storage_buffers;
   VkPipelineRobustnessBufferBehaviorEXT uniform_buffers;
   VkPipelineRobustnessBufferBehaviorEXT vertex_inputs;
   VkPipelineRobustnessImageBehaviorEXT images;
};

/** Hash VkPipelineShaderStageCreateInfo info
 *
 * Returns the hash of a VkPipelineShaderStageCreateInfo:
 *    SHA1(info->module->sha1,
 *         info->pName,
 *         vk_stage_to_mesa_stage(info->stage),
 *         info->pSpecializationInfo)
 *
 * Can only be used if VkPipelineShaderStageCreateInfo::module is a
 * vk_shader_module object.
 */
void
vk_pipeline_hash_shader_stage(const VkPipelineShaderStageCreateInfo *info,
                              const struct vk_pipeline_robustness_state *rstate,
                              unsigned char *stage_sha1);

void
vk_pipeline_robustness_state_fill(const struct vk_device *device,
                                  struct vk_pipeline_robustness_state *rs,
                                  const void *pipeline_pNext,
                                  const void *shader_stage_pNext);

static inline VkPipelineCreateFlags2KHR
vk_compute_pipeline_create_flags(const VkComputePipelineCreateInfo *info)
{
   const VkPipelineCreateFlags2CreateInfoKHR *flags2 =
      vk_find_struct_const(info->pNext,
                           PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR);
   if (flags2)
      return flags2->flags;
   else
      return info->flags;
}

static inline VkPipelineCreateFlags2KHR
vk_graphics_pipeline_create_flags(const VkGraphicsPipelineCreateInfo *info)
{
   const VkPipelineCreateFlags2CreateInfoKHR *flags2 =
      vk_find_struct_const(info->pNext,
                           PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR);
   if (flags2)
      return flags2->flags;
   else
      return info->flags;
}

static inline VkPipelineCreateFlags2KHR
vk_rt_pipeline_create_flags(const VkRayTracingPipelineCreateInfoKHR *info)
{
   const VkPipelineCreateFlags2CreateInfoKHR *flags2 =
      vk_find_struct_const(info->pNext,
                           PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR);
   if (flags2)
      return flags2->flags;
   else
      return info->flags;
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
static inline VkPipelineCreateFlags2KHR
vk_graph_pipeline_create_flags(const VkExecutionGraphPipelineCreateInfoAMDX *info)
{
   const VkPipelineCreateFlags2CreateInfoKHR *flags2 =
      vk_find_struct_const(info->pNext,
                           PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR);
   if (flags2)
      return flags2->flags;
   else
      return info->flags;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* VK_PIPELINE_H */
