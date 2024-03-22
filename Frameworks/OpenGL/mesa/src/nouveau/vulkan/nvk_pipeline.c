/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_pipeline.h"

#include "nvk_device.h"
#include "nvk_entrypoints.h"

#include "vk_pipeline_cache.h"

struct nvk_pipeline *
nvk_pipeline_zalloc(struct nvk_device *dev,
                    enum nvk_pipeline_type type, size_t size,
                    const VkAllocationCallbacks *pAllocator)
{
   struct nvk_pipeline *pipeline;

   assert(size >= sizeof(*pipeline));
   pipeline = vk_object_zalloc(&dev->vk, pAllocator, size,
                               VK_OBJECT_TYPE_PIPELINE);
   if (pipeline == NULL)
      return NULL;

   pipeline->type = type;

   return pipeline;
}

void
nvk_pipeline_free(struct nvk_device *dev,
                  struct nvk_pipeline *pipeline,
                  const VkAllocationCallbacks *pAllocator)
{
   for (uint32_t s = 0; s < ARRAY_SIZE(pipeline->shaders); s++) {
      if (pipeline->shaders[s] != NULL)
         vk_pipeline_cache_object_unref(&dev->vk, &pipeline->shaders[s]->base);
   }

   vk_object_free(&dev->vk, pAllocator, pipeline);
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateGraphicsPipelines(VkDevice _device,
                            VkPipelineCache pipelineCache,
                            uint32_t createInfoCount,
                            const VkGraphicsPipelineCreateInfo *pCreateInfos,
                            const VkAllocationCallbacks *pAllocator,
                            VkPipeline *pPipelines)
{
   VK_FROM_HANDLE(nvk_device, dev, _device);
   VK_FROM_HANDLE(vk_pipeline_cache, cache, pipelineCache);
   VkResult result = VK_SUCCESS;

   unsigned i = 0;
   for (; i < createInfoCount; i++) {
      VkResult r = nvk_graphics_pipeline_create(dev, cache, &pCreateInfos[i],
                                                pAllocator, &pPipelines[i]);
      if (r == VK_SUCCESS)
         continue;

      result = r;
      pPipelines[i] = VK_NULL_HANDLE;
      if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT)
         break;
   }

   for (; i < createInfoCount; i++)
      pPipelines[i] = VK_NULL_HANDLE;

   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateComputePipelines(VkDevice _device,
                           VkPipelineCache pipelineCache,
                           uint32_t createInfoCount,
                           const VkComputePipelineCreateInfo *pCreateInfos,
                           const VkAllocationCallbacks *pAllocator,
                           VkPipeline *pPipelines)
{
   VK_FROM_HANDLE(nvk_device, dev, _device);
   VK_FROM_HANDLE(vk_pipeline_cache, cache, pipelineCache);
   VkResult result = VK_SUCCESS;

   unsigned i = 0;
   for (; i < createInfoCount; i++) {
      VkResult r = nvk_compute_pipeline_create(dev, cache, &pCreateInfos[i],
                                               pAllocator, &pPipelines[i]);
      if (r == VK_SUCCESS)
         continue;

      result = r;
      pPipelines[i] = VK_NULL_HANDLE;
      if (pCreateInfos[i].flags & VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT)
         break;
   }

   for (; i < createInfoCount; i++)
      pPipelines[i] = VK_NULL_HANDLE;

   return result;
}

VKAPI_ATTR void VKAPI_CALL
nvk_DestroyPipeline(VkDevice _device, VkPipeline _pipeline,
                    const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(nvk_device, dev, _device);
   VK_FROM_HANDLE(nvk_pipeline, pipeline, _pipeline);

   if (!pipeline)
      return;

   nvk_pipeline_free(dev, pipeline, pAllocator);
}

#define WRITE_STR(field, ...) ({                               \
   memset(field, 0, sizeof(field));                            \
   UNUSED int i = snprintf(field, sizeof(field), __VA_ARGS__); \
   assert(i > 0 && i < sizeof(field));                         \
})

VKAPI_ATTR VkResult VKAPI_CALL
nvk_GetPipelineExecutablePropertiesKHR(
   VkDevice device,
   const VkPipelineInfoKHR *pPipelineInfo,
   uint32_t *pExecutableCount,
   VkPipelineExecutablePropertiesKHR *pProperties)
{
   VK_FROM_HANDLE(nvk_pipeline, pipeline, pPipelineInfo->pipeline);
   VK_OUTARRAY_MAKE_TYPED(VkPipelineExecutablePropertiesKHR, out,
                          pProperties, pExecutableCount);

   for (gl_shader_stage stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      const struct nvk_shader *shader = pipeline->shaders[stage];
      if (!shader || shader->code_size == 0)
         continue;

      vk_outarray_append_typed(VkPipelineExecutablePropertiesKHR, &out, props) {
         props->stages = mesa_to_vk_shader_stage(stage);
         props->subgroupSize = 32;
         WRITE_STR(props->name, "%s", _mesa_shader_stage_to_string(stage));
         WRITE_STR(props->description, "%s shader",
                   _mesa_shader_stage_to_string(stage));
      }
   }

   return vk_outarray_status(&out);
}

static struct nvk_shader *
shader_for_exe_idx(struct nvk_pipeline *pipeline, uint32_t idx)
{
   for (gl_shader_stage stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      const struct nvk_shader *shader = pipeline->shaders[stage];
      if (!shader || shader->code_size == 0)
         continue;

      if (idx == 0)
         return pipeline->shaders[stage];

      idx--;
   }

   return NULL;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_GetPipelineExecutableStatisticsKHR(
    VkDevice device,
    const VkPipelineExecutableInfoKHR *pExecutableInfo,
    uint32_t *pStatisticCount,
    VkPipelineExecutableStatisticKHR *pStatistics)
{
   VK_FROM_HANDLE(nvk_pipeline, pipeline, pExecutableInfo->pipeline);
   VK_OUTARRAY_MAKE_TYPED(VkPipelineExecutableStatisticKHR, out,
                          pStatistics, pStatisticCount);

   struct nvk_shader *shader =
      shader_for_exe_idx(pipeline, pExecutableInfo->executableIndex);

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Code Size");
      WRITE_STR(stat->description,
                "Size of the compiled shader binary, in bytes");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = shader->code_size;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Number of GPRs");
      WRITE_STR(stat->description, "Number of GPRs used by this pipeline");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = shader->info.num_gprs;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "SLM Size");
      WRITE_STR(stat->description,
                "Size of shader local (scratch) memory, in bytes");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = shader->info.slm_size;
   }

   return vk_outarray_status(&out);
}

static bool
write_ir_text(VkPipelineExecutableInternalRepresentationKHR* ir,
              const char *data)
{
   ir->isText = VK_TRUE;

   size_t data_len = strlen(data) + 1;

   if (ir->pData == NULL) {
      ir->dataSize = data_len;
      return true;
   }

   strncpy(ir->pData, data, ir->dataSize);
   if (ir->dataSize < data_len)
      return false;

   ir->dataSize = data_len;
   return true;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_GetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device,
    const VkPipelineExecutableInfoKHR *pExecutableInfo,
    uint32_t *pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations)
{
   VK_FROM_HANDLE(nvk_pipeline, pipeline, pExecutableInfo->pipeline);
   VK_OUTARRAY_MAKE_TYPED(VkPipelineExecutableInternalRepresentationKHR, out,
                          pInternalRepresentations,
                          pInternalRepresentationCount);
   bool incomplete_text = false;

   struct nvk_shader *shader =
      shader_for_exe_idx(pipeline, pExecutableInfo->executableIndex);

   if (shader->nak != NULL && shader->nak->asm_str != NULL) {
      vk_outarray_append_typed(VkPipelineExecutableInternalRepresentationKHR, &out, ir) {
         WRITE_STR(ir->name, "NAK assembly");
         WRITE_STR(ir->description, "NAK assembly");
         if (!write_ir_text(ir, shader->nak->asm_str))
            incomplete_text = true;
      }
   }

   return incomplete_text ? VK_INCOMPLETE : vk_outarray_status(&out);
}
