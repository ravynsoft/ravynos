/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_PIPELINE_H
#define NVK_PIPELINE_H 1

#include "nvk_private.h"
#include "nvk_shader.h"

#include "vk_graphics_state.h"
#include "vk_object.h"

struct vk_pipeline_cache;

enum nvk_pipeline_type {
   NVK_PIPELINE_GRAPHICS,
   NVK_PIPELINE_COMPUTE,
};

struct nvk_pipeline {
   struct vk_object_base base;

   enum nvk_pipeline_type type;

   struct nvk_shader *shaders[MESA_SHADER_STAGES];
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_pipeline, base, VkPipeline,
                               VK_OBJECT_TYPE_PIPELINE)

void
nvk_pipeline_free(struct nvk_device *dev,
                  struct nvk_pipeline *pipeline,
                  const VkAllocationCallbacks *pAllocator);
struct nvk_pipeline *
nvk_pipeline_zalloc(struct nvk_device *dev,
                    enum nvk_pipeline_type type, size_t size,
                    const VkAllocationCallbacks *pAllocator);

struct nvk_compute_pipeline {
   struct nvk_pipeline base;

   uint32_t qmd_template[64];
};

VkResult
nvk_compute_pipeline_create(struct nvk_device *dev,
                            struct vk_pipeline_cache *cache,
                            const VkComputePipelineCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator,
                            VkPipeline *pPipeline);

struct nvk_graphics_pipeline {
   struct nvk_pipeline base;

   uint32_t push_data[192];
   uint32_t push_dw_count;

   struct vk_vertex_input_state _dynamic_vi;
   struct vk_sample_locations_state _dynamic_sl;
   struct vk_dynamic_graphics_state dynamic;
};

VkResult
nvk_graphics_pipeline_create(struct nvk_device *dev,
                             struct vk_pipeline_cache *cache,
                             const VkGraphicsPipelineCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator,
                             VkPipeline *pPipeline);

#endif
