/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_event.h"

#include "nvk_cmd_buffer.h"
#include "nvk_device.h"
#include "nvk_entrypoints.h"
#include "nvk_mme.h"

#include "nvk_cl906f.h"
#include "nvk_cl9097.h"

#define NVK_EVENT_MEM_SIZE sizeof(VkResult)

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateEvent(VkDevice device,
                const VkEventCreateInfo *pCreateInfo,
                const VkAllocationCallbacks *pAllocator,
                VkEvent *pEvent)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   struct nvk_event *event;
   VkResult result;

   event = vk_object_zalloc(&dev->vk, pAllocator, sizeof(*event),
                            VK_OBJECT_TYPE_EVENT);
   if (!event)
      return vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = nvk_heap_alloc(dev, &dev->event_heap,
                           NVK_EVENT_MEM_SIZE, NVK_EVENT_MEM_SIZE,
                           &event->addr, (void **)&event->status);
   if (result != VK_SUCCESS) {
      vk_object_free(&dev->vk, pAllocator, event);
      return result;
   }

   *event->status = VK_EVENT_RESET;

   *pEvent = nvk_event_to_handle(event);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
nvk_DestroyEvent(VkDevice device,
                 VkEvent _event,
                 const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   VK_FROM_HANDLE(nvk_event, event, _event);

   if (!event)
      return;

   nvk_heap_free(dev, &dev->event_heap, event->addr, NVK_EVENT_MEM_SIZE);

   vk_object_free(&dev->vk, pAllocator, event);
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_GetEventStatus(VkDevice device,
                   VkEvent _event)
{
   VK_FROM_HANDLE(nvk_event, event, _event);

   return *event->status;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_SetEvent(VkDevice device,
             VkEvent _event)
{
   VK_FROM_HANDLE(nvk_event, event, _event);

   *event->status = VK_EVENT_SET;

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_ResetEvent(VkDevice device,
               VkEvent _event)
{
   VK_FROM_HANDLE(nvk_event, event, _event);

   *event->status = VK_EVENT_RESET;

   return VK_SUCCESS;
}

static bool
clear_bits64(uint64_t *bitfield, uint64_t bits)
{
   bool has_bits = (*bitfield & bits) != 0;
   *bitfield &= ~bits;
   return has_bits;
}

uint32_t
vk_stage_flags_to_nv9097_pipeline_location(VkPipelineStageFlags2 flags)
{
   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT |
                            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT |
                            VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT |
                            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT|
                            VK_PIPELINE_STAGE_2_COPY_BIT|
                            VK_PIPELINE_STAGE_2_RESOLVE_BIT|
                            VK_PIPELINE_STAGE_2_BLIT_BIT|
                            VK_PIPELINE_STAGE_2_CLEAR_BIT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_ALL;

   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_DEPTH_TEST;

   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_PIXEL_SHADER;

   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_ZCULL;

   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_STREAMING_OUTPUT;

   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT |
                            VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_GEOMETRY_SHADER;

   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_TESSELATION_SHADER;

   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_TESSELATION_INIT_SHADER;

   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_VERTEX_SHADER;

   if (clear_bits64(&flags, VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT |
                            VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT |
                            VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT))
      return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_DATA_ASSEMBLER;

   clear_bits64(&flags, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT |
                        VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT |
                        VK_PIPELINE_STAGE_2_HOST_BIT |
                        VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT);

   /* TODO: Doing this on 3D will likely cause a WFI which is probably ok but,
    * if we tracked which subchannel we've used most recently, we can probably
    * do better than that.
    */
   clear_bits64(&flags, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

   assert(flags == 0);

   return NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_NONE;
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdSetEvent2(VkCommandBuffer commandBuffer,
                 VkEvent _event,
                 const VkDependencyInfo *pDependencyInfo)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_event, event, _event);

   nvk_cmd_flush_wait_dep(cmd, pDependencyInfo, false);

   VkPipelineStageFlags2 stages = 0;
   for (uint32_t i = 0; i < pDependencyInfo->memoryBarrierCount; i++)
      stages |= pDependencyInfo->pMemoryBarriers[i].srcStageMask;
   for (uint32_t i = 0; i < pDependencyInfo->bufferMemoryBarrierCount; i++)
      stages |= pDependencyInfo->pBufferMemoryBarriers[i].srcStageMask;
   for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; i++)
      stages |= pDependencyInfo->pImageMemoryBarriers[i].srcStageMask;

   struct nv_push *p = nvk_cmd_buffer_push(cmd, 5);
   P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
   P_NV9097_SET_REPORT_SEMAPHORE_A(p, event->addr >> 32);
   P_NV9097_SET_REPORT_SEMAPHORE_B(p, event->addr);
   P_NV9097_SET_REPORT_SEMAPHORE_C(p, VK_EVENT_SET);
   P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
      .operation = OPERATION_RELEASE,
      .release = RELEASE_AFTER_ALL_PRECEEDING_WRITES_COMPLETE,
      .pipeline_location = vk_stage_flags_to_nv9097_pipeline_location(stages),
      .structure_size = STRUCTURE_SIZE_ONE_WORD,
   });
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdResetEvent2(VkCommandBuffer commandBuffer,
                   VkEvent _event,
                   VkPipelineStageFlags2 stageMask)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_event, event, _event);

   struct nv_push *p = nvk_cmd_buffer_push(cmd, 5);
   P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
   P_NV9097_SET_REPORT_SEMAPHORE_A(p, event->addr >> 32);
   P_NV9097_SET_REPORT_SEMAPHORE_B(p, event->addr);
   P_NV9097_SET_REPORT_SEMAPHORE_C(p, VK_EVENT_RESET);
   P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
      .operation = OPERATION_RELEASE,
      .release = RELEASE_AFTER_ALL_PRECEEDING_WRITES_COMPLETE,
      .pipeline_location =
         vk_stage_flags_to_nv9097_pipeline_location(stageMask),
      .structure_size = STRUCTURE_SIZE_ONE_WORD,
   });
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdWaitEvents2(VkCommandBuffer commandBuffer,
                   uint32_t eventCount,
                   const VkEvent *pEvents,
                   const VkDependencyInfo *pDependencyInfos)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);

   for (uint32_t i = 0; i < eventCount; i++) {
      VK_FROM_HANDLE(nvk_event, event, pEvents[i]);

      struct nv_push *p = nvk_cmd_buffer_push(cmd, 5);
      __push_mthd(p, SUBC_NV9097, NV906F_SEMAPHOREA);
      P_NV906F_SEMAPHOREA(p, event->addr >> 32);
      P_NV906F_SEMAPHOREB(p, (event->addr & UINT32_MAX) >> 2);
      P_NV906F_SEMAPHOREC(p, VK_EVENT_SET);
      P_NV906F_SEMAPHORED(p, {
         .operation = OPERATION_ACQUIRE,
         .acquire_switch = ACQUIRE_SWITCH_ENABLED,
         .release_size = RELEASE_SIZE_4BYTE,
      });
   }

   nvk_cmd_invalidate_deps(cmd, eventCount, pDependencyInfos);
}
