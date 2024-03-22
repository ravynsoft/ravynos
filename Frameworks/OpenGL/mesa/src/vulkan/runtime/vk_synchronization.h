/*
 * Copyright © 2023 Collabora, Ltd
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
#ifndef VK_SYNCHRONIZATION_H
#define VK_SYNCHRONIZATION_H

#include <vulkan/vulkan_core.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline bool
vk_pipeline_stage_flags2_has_graphics_shader(VkPipelineStageFlags2 stages)
{
   return stages & (VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT |
                    VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT |
                    VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT |
                    VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT |
                    VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT |
                    VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT);
}

static inline bool
vk_pipeline_stage_flags2_has_compute_shader(VkPipelineStageFlags2 stages)
{
   return stages & (VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT |
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT |
                    VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
}

/** Expands pipeline stage group flags
 *
 * Some stages like VK_PIPELINE_SHADER_STAGE_2_ALL_GRAPHICS_BIT represent more
 * than one stage.  This helper expands any such bits out to the full set of
 * individual stages bits they represent.
 *
 * Note: This helper does not handle BOTTOM/TOP_OF_PIPE.  You probably want to
 * use vk_expand_src/dst_stage_flags2() instead.
 */
VkPipelineStageFlags2
vk_expand_pipeline_stage_flags2(VkPipelineStageFlags2 stages);

static inline VkPipelineStageFlags2
vk_expand_src_stage_flags2(VkPipelineStageFlags2 stages)
{
   if (stages & VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT)
      stages |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

   return vk_expand_pipeline_stage_flags2(stages);
}

static inline VkPipelineStageFlags2
vk_expand_dst_stage_flags2(VkPipelineStageFlags2 stages)
{
   if (stages & VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT)
      stages |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

   return vk_expand_pipeline_stage_flags2(stages);
}

/** Returns the set of read accesses allowed in the given stages */
VkAccessFlags2
vk_read_access2_for_pipeline_stage_flags2(VkPipelineStageFlags2 stages);

/** Returns the set of write accesses allowed in the given stages */
VkAccessFlags2
vk_write_access2_for_pipeline_stage_flags2(VkPipelineStageFlags2 stages);

VkAccessFlags2
vk_filter_src_access_flags2(VkPipelineStageFlags2 stages,
                            VkAccessFlags2 access);

VkAccessFlags2
vk_filter_dst_access_flags2(VkPipelineStageFlags2 stages,
                            VkAccessFlags2 access);

#ifdef __cplusplus
}
#endif

#endif /* VK_SYNCHRONIZATION_H */
