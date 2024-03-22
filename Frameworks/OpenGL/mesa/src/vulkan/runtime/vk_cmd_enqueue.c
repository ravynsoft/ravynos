/*
 * Copyright © 2019 Red Hat.
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

#include "vk_alloc.h"
#include "vk_cmd_enqueue_entrypoints.h"
#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_pipeline_layout.h"
#include "vk_util.h"

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_CmdDrawMultiEXT(VkCommandBuffer commandBuffer,
                               uint32_t drawCount,
                               const VkMultiDrawInfoEXT *pVertexInfo,
                               uint32_t instanceCount,
                               uint32_t firstInstance,
                               uint32_t stride)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);

   struct vk_cmd_queue_entry *cmd =
      vk_zalloc(cmd_buffer->cmd_queue.alloc, sizeof(*cmd), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd)
      return;

   cmd->type = VK_CMD_DRAW_MULTI_EXT;
   list_addtail(&cmd->cmd_link, &cmd_buffer->cmd_queue.cmds);

   cmd->u.draw_multi_ext.draw_count = drawCount;
   if (pVertexInfo) {
      unsigned i = 0;
      cmd->u.draw_multi_ext.vertex_info =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.draw_multi_ext.vertex_info) * drawCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      vk_foreach_multi_draw(draw, i, pVertexInfo, drawCount, stride) {
         memcpy(&cmd->u.draw_multi_ext.vertex_info[i], draw,
                sizeof(*cmd->u.draw_multi_ext.vertex_info));
      }
   }
   cmd->u.draw_multi_ext.instance_count = instanceCount;
   cmd->u.draw_multi_ext.first_instance = firstInstance;
   cmd->u.draw_multi_ext.stride = stride;
}

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_CmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer,
                                      uint32_t drawCount,
                                      const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                                      uint32_t instanceCount,
                                      uint32_t firstInstance,
                                      uint32_t stride,
                                      const int32_t *pVertexOffset)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);

   struct vk_cmd_queue_entry *cmd =
      vk_zalloc(cmd_buffer->cmd_queue.alloc, sizeof(*cmd), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd)
      return;

   cmd->type = VK_CMD_DRAW_MULTI_INDEXED_EXT;
   list_addtail(&cmd->cmd_link, &cmd_buffer->cmd_queue.cmds);

   cmd->u.draw_multi_indexed_ext.draw_count = drawCount;

   if (pIndexInfo) {
      unsigned i = 0;
      cmd->u.draw_multi_indexed_ext.index_info =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.draw_multi_indexed_ext.index_info) * drawCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      vk_foreach_multi_draw_indexed(draw, i, pIndexInfo, drawCount, stride) {
         cmd->u.draw_multi_indexed_ext.index_info[i].firstIndex = draw->firstIndex;
         cmd->u.draw_multi_indexed_ext.index_info[i].indexCount = draw->indexCount;
         if (pVertexOffset == NULL)
            cmd->u.draw_multi_indexed_ext.index_info[i].vertexOffset = draw->vertexOffset;
      }
   }

   cmd->u.draw_multi_indexed_ext.instance_count = instanceCount;
   cmd->u.draw_multi_indexed_ext.first_instance = firstInstance;
   cmd->u.draw_multi_indexed_ext.stride = stride;

   if (pVertexOffset) {
      cmd->u.draw_multi_indexed_ext.vertex_offset =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.draw_multi_indexed_ext.vertex_offset), 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      memcpy(cmd->u.draw_multi_indexed_ext.vertex_offset, pVertexOffset,
             sizeof(*cmd->u.draw_multi_indexed_ext.vertex_offset));
   }
}

static void
push_descriptors_set_free(struct vk_cmd_queue *queue,
                          struct vk_cmd_queue_entry *cmd)
{
  struct vk_cmd_push_descriptor_set_khr *pds = &cmd->u.push_descriptor_set_khr;
  for (unsigned i = 0; i < pds->descriptor_write_count; i++) {
    VkWriteDescriptorSet *entry = &pds->descriptor_writes[i];
    switch (entry->descriptorType) {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
       vk_free(queue->alloc, (void *)entry->pImageInfo);
       break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
       vk_free(queue->alloc, (void *)entry->pTexelBufferView);
       break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
    default:
       vk_free(queue->alloc, (void *)entry->pBufferInfo);
       break;
    }
  }
}

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                       VkPipelineBindPoint pipelineBindPoint,
                                       VkPipelineLayout layout,
                                       uint32_t set,
                                       uint32_t descriptorWriteCount,
                                       const VkWriteDescriptorSet *pDescriptorWrites)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   struct vk_cmd_push_descriptor_set_khr *pds;

   struct vk_cmd_queue_entry *cmd =
      vk_zalloc(cmd_buffer->cmd_queue.alloc, sizeof(*cmd), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd)
      return;

   pds = &cmd->u.push_descriptor_set_khr;

   cmd->type = VK_CMD_PUSH_DESCRIPTOR_SET_KHR;
   cmd->driver_free_cb = push_descriptors_set_free;
   list_addtail(&cmd->cmd_link, &cmd_buffer->cmd_queue.cmds);

   pds->pipeline_bind_point = pipelineBindPoint;
   pds->layout = layout;
   pds->set = set;
   pds->descriptor_write_count = descriptorWriteCount;

   if (pDescriptorWrites) {
      pds->descriptor_writes =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*pds->descriptor_writes) * descriptorWriteCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      memcpy(pds->descriptor_writes,
             pDescriptorWrites,
             sizeof(*pds->descriptor_writes) * descriptorWriteCount);

      for (unsigned i = 0; i < descriptorWriteCount; i++) {
         switch (pds->descriptor_writes[i].descriptorType) {
         case VK_DESCRIPTOR_TYPE_SAMPLER:
         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
         case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            pds->descriptor_writes[i].pImageInfo =
               vk_zalloc(cmd_buffer->cmd_queue.alloc,
                         sizeof(VkDescriptorImageInfo) * pds->descriptor_writes[i].descriptorCount, 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            memcpy((VkDescriptorImageInfo *)pds->descriptor_writes[i].pImageInfo,
                   pDescriptorWrites[i].pImageInfo,
                   sizeof(VkDescriptorImageInfo) * pds->descriptor_writes[i].descriptorCount);
            break;
         case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            pds->descriptor_writes[i].pTexelBufferView =
               vk_zalloc(cmd_buffer->cmd_queue.alloc,
                         sizeof(VkBufferView) * pds->descriptor_writes[i].descriptorCount, 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            memcpy((VkBufferView *)pds->descriptor_writes[i].pTexelBufferView,
                   pDescriptorWrites[i].pTexelBufferView,
                   sizeof(VkBufferView) * pds->descriptor_writes[i].descriptorCount);
            break;
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         default:
            pds->descriptor_writes[i].pBufferInfo =
               vk_zalloc(cmd_buffer->cmd_queue.alloc,
                         sizeof(VkDescriptorBufferInfo) * pds->descriptor_writes[i].descriptorCount, 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            memcpy((VkDescriptorBufferInfo *)pds->descriptor_writes[i].pBufferInfo,
                   pDescriptorWrites[i].pBufferInfo,
                   sizeof(VkDescriptorBufferInfo) * pds->descriptor_writes[i].descriptorCount);
            break;
         }
      }
   }
}

static void
unref_pipeline_layout(struct vk_cmd_queue *queue,
                      struct vk_cmd_queue_entry *cmd)
{
   struct vk_command_buffer *cmd_buffer =
      container_of(queue, struct vk_command_buffer, cmd_queue);
   VK_FROM_HANDLE(vk_pipeline_layout, layout,
                  cmd->u.bind_descriptor_sets.layout);

   assert(cmd->type == VK_CMD_BIND_DESCRIPTOR_SETS);

   vk_pipeline_layout_unref(cmd_buffer->base.device, layout);
}

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_CmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                     VkPipelineBindPoint pipelineBindPoint,
                                     VkPipelineLayout layout,
                                     uint32_t firstSet,
                                     uint32_t descriptorSetCount,
                                     const VkDescriptorSet* pDescriptorSets,
                                     uint32_t dynamicOffsetCount,
                                     const uint32_t *pDynamicOffsets)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);

   struct vk_cmd_queue_entry *cmd =
      vk_zalloc(cmd_buffer->cmd_queue.alloc, sizeof(*cmd), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd)
      return;

   cmd->type = VK_CMD_BIND_DESCRIPTOR_SETS;
   list_addtail(&cmd->cmd_link, &cmd_buffer->cmd_queue.cmds);

   /* We need to hold a reference to the descriptor set as long as this
    * command is in the queue.  Otherwise, it may get deleted out from under
    * us before the command is replayed.
    */
   vk_pipeline_layout_ref(vk_pipeline_layout_from_handle(layout));
   cmd->u.bind_descriptor_sets.layout = layout;
   cmd->driver_free_cb = unref_pipeline_layout;

   cmd->u.bind_descriptor_sets.pipeline_bind_point = pipelineBindPoint;
   cmd->u.bind_descriptor_sets.first_set = firstSet;
   cmd->u.bind_descriptor_sets.descriptor_set_count = descriptorSetCount;
   if (pDescriptorSets) {
      cmd->u.bind_descriptor_sets.descriptor_sets =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.bind_descriptor_sets.descriptor_sets) * descriptorSetCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      memcpy(cmd->u.bind_descriptor_sets.descriptor_sets, pDescriptorSets,
             sizeof(*cmd->u.bind_descriptor_sets.descriptor_sets) * descriptorSetCount);
   }
   cmd->u.bind_descriptor_sets.dynamic_offset_count = dynamicOffsetCount;
   if (pDynamicOffsets) {
      cmd->u.bind_descriptor_sets.dynamic_offsets =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.bind_descriptor_sets.dynamic_offsets) * dynamicOffsetCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      memcpy(cmd->u.bind_descriptor_sets.dynamic_offsets, pDynamicOffsets,
             sizeof(*cmd->u.bind_descriptor_sets.dynamic_offsets) * dynamicOffsetCount);
   }
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
static void
dispatch_graph_amdx_free(struct vk_cmd_queue *queue, struct vk_cmd_queue_entry *cmd)
{
   VkDispatchGraphCountInfoAMDX *count_info = cmd->u.dispatch_graph_amdx.count_info;
   void *infos = (void *)count_info->infos.hostAddress;

   for (uint32_t i = 0; i < count_info->count; i++) {
      VkDispatchGraphInfoAMDX *info = (void *)((const uint8_t *)infos + i * count_info->stride);
      vk_free(queue->alloc, (void *)info->payloads.hostAddress);
   }

   vk_free(queue->alloc, infos);
}

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_CmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                    const VkDispatchGraphCountInfoAMDX *pCountInfo)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);

   if (vk_command_buffer_has_error(cmd_buffer))
      return;

   VkResult result = VK_SUCCESS;
   const VkAllocationCallbacks *alloc = cmd_buffer->cmd_queue.alloc;

   struct vk_cmd_queue_entry *cmd =
      vk_zalloc(alloc, sizeof(struct vk_cmd_queue_entry), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto err;
   }

   cmd->type = VK_CMD_DISPATCH_GRAPH_AMDX;
   cmd->driver_free_cb = dispatch_graph_amdx_free;

   cmd->u.dispatch_graph_amdx.scratch = scratch;

   cmd->u.dispatch_graph_amdx.count_info =
      vk_zalloc(alloc, sizeof(VkDispatchGraphCountInfoAMDX), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (cmd->u.dispatch_graph_amdx.count_info == NULL)
      goto err;

   memcpy((void *)cmd->u.dispatch_graph_amdx.count_info, pCountInfo,
          sizeof(VkDispatchGraphCountInfoAMDX));

   uint32_t infos_size = pCountInfo->count * pCountInfo->stride;
   void *infos = vk_zalloc(alloc, infos_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   cmd->u.dispatch_graph_amdx.count_info->infos.hostAddress = infos;
   memcpy(infos, pCountInfo->infos.hostAddress, infos_size);

   for (uint32_t i = 0; i < pCountInfo->count; i++) {
      VkDispatchGraphInfoAMDX *info = (void *)((const uint8_t *)infos + i * pCountInfo->stride);

      uint32_t payloads_size = info->payloadCount * info->payloadStride;
      void *dst_payload = vk_zalloc(alloc, payloads_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      memcpy(dst_payload, info->payloads.hostAddress, payloads_size);
      info->payloads.hostAddress = dst_payload;
   }

   list_addtail(&cmd->cmd_link, &cmd_buffer->cmd_queue.cmds);
   goto finish;
err:
   if (cmd) {
      vk_free(alloc, cmd);
      dispatch_graph_amdx_free(&cmd_buffer->cmd_queue, cmd);
   }

finish:
   if (unlikely(result != VK_SUCCESS))
      vk_command_buffer_set_error(cmd_buffer, result);
}
#endif
