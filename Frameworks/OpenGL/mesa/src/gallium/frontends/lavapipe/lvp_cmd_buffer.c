/*
 * Copyright © 2019 Red Hat.
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

#include "lvp_private.h"
#include "pipe/p_context.h"
#include "vk_util.h"

#include "vk_common_entrypoints.h"

static void
lvp_cmd_buffer_destroy(struct vk_command_buffer *cmd_buffer)
{
   vk_command_buffer_finish(cmd_buffer);
   vk_free(&cmd_buffer->pool->alloc, cmd_buffer);
}

static VkResult
lvp_create_cmd_buffer(struct vk_command_pool *pool,
                      struct vk_command_buffer **cmd_buffer_out)
{
   struct lvp_device *device =
      container_of(pool->base.device, struct lvp_device, vk);
   struct lvp_cmd_buffer *cmd_buffer;

   cmd_buffer = vk_alloc(&pool->alloc, sizeof(*cmd_buffer), 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (cmd_buffer == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   VkResult result = vk_command_buffer_init(pool, &cmd_buffer->vk,
                                            &lvp_cmd_buffer_ops, 0);
   if (result != VK_SUCCESS) {
      vk_free(&pool->alloc, cmd_buffer);
      return result;
   }

   cmd_buffer->device = device;

   *cmd_buffer_out = &cmd_buffer->vk;

   return VK_SUCCESS;
}

static void
lvp_reset_cmd_buffer(struct vk_command_buffer *vk_cmd_buffer,
                     UNUSED VkCommandBufferResetFlags flags)
{
   vk_command_buffer_reset(vk_cmd_buffer);
}

const struct vk_command_buffer_ops lvp_cmd_buffer_ops = {
   .create = lvp_create_cmd_buffer,
   .reset = lvp_reset_cmd_buffer,
   .destroy = lvp_cmd_buffer_destroy,
};

VKAPI_ATTR VkResult VKAPI_CALL lvp_BeginCommandBuffer(
   VkCommandBuffer                             commandBuffer,
   const VkCommandBufferBeginInfo*             pBeginInfo)
{
   LVP_FROM_HANDLE(lvp_cmd_buffer, cmd_buffer, commandBuffer);

   vk_command_buffer_begin(&cmd_buffer->vk, pBeginInfo);

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_EndCommandBuffer(
   VkCommandBuffer                             commandBuffer)
{
   LVP_FROM_HANDLE(lvp_cmd_buffer, cmd_buffer, commandBuffer);

   return vk_command_buffer_end(&cmd_buffer->vk);
}

static void
lvp_free_CmdPushDescriptorSetWithTemplate2KHR(struct vk_cmd_queue *queue, struct vk_cmd_queue_entry *cmd)
{
   struct lvp_device *device = cmd->driver_data;
   LVP_FROM_HANDLE(lvp_descriptor_update_template, templ, cmd->u.push_descriptor_set_with_template2_khr.push_descriptor_set_with_template_info->descriptorUpdateTemplate);
   lvp_descriptor_template_templ_unref(device, templ);
}

VKAPI_ATTR void VKAPI_CALL lvp_CmdPushDescriptorSetWithTemplate2KHR(
   VkCommandBuffer                             commandBuffer,
   const VkPushDescriptorSetWithTemplateInfoKHR* pPushDescriptorSetWithTemplateInfo)
{
   LVP_FROM_HANDLE(lvp_cmd_buffer, cmd_buffer, commandBuffer);
   LVP_FROM_HANDLE(lvp_descriptor_update_template, templ, pPushDescriptorSetWithTemplateInfo->descriptorUpdateTemplate);
   size_t info_size = 0;
   struct vk_cmd_queue_entry *cmd = vk_zalloc(cmd_buffer->vk.cmd_queue.alloc,
                                              vk_cmd_queue_type_sizes[VK_CMD_PUSH_DESCRIPTOR_SET_WITH_TEMPLATE2_KHR], 8,
                                              VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!cmd)
      return;

   cmd->type = VK_CMD_PUSH_DESCRIPTOR_SET_WITH_TEMPLATE2_KHR;

   list_addtail(&cmd->cmd_link, &cmd_buffer->vk.cmd_queue.cmds);
   cmd->driver_free_cb = lvp_free_CmdPushDescriptorSetWithTemplate2KHR;
   cmd->driver_data = cmd_buffer->device;
   lvp_descriptor_template_templ_ref(templ);
   cmd->u.push_descriptor_set_with_template2_khr.push_descriptor_set_with_template_info = vk_zalloc(cmd_buffer->vk.cmd_queue.alloc, sizeof(VkPushDescriptorSetWithTemplateInfoKHR), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   memcpy(cmd->u.push_descriptor_set_with_template2_khr.push_descriptor_set_with_template_info, pPushDescriptorSetWithTemplateInfo, sizeof(VkPushDescriptorSetWithTemplateInfoKHR));

   for (unsigned i = 0; i < templ->entry_count; i++) {
      VkDescriptorUpdateTemplateEntry *entry = &templ->entry[i];

      switch (entry->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         info_size += sizeof(VkDescriptorImageInfo) * entry->descriptorCount;
         break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         info_size += sizeof(VkBufferView) * entry->descriptorCount;
         break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      default:
         info_size += sizeof(VkDescriptorBufferInfo) * entry->descriptorCount;
         break;
      }
   }

   cmd->u.push_descriptor_set_with_template2_khr.push_descriptor_set_with_template_info->pData = vk_zalloc(cmd_buffer->vk.cmd_queue.alloc, info_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);

   uint64_t offset = 0;
   for (unsigned i = 0; i < templ->entry_count; i++) {
      VkDescriptorUpdateTemplateEntry *entry = &templ->entry[i];

      unsigned size = lvp_descriptor_update_template_entry_size(entry->descriptorType);

      for (unsigned i = 0; i < entry->descriptorCount; i++) {
         memcpy((uint8_t*)cmd->u.push_descriptor_set_with_template2_khr.push_descriptor_set_with_template_info->pData + offset, (const uint8_t*)pPushDescriptorSetWithTemplateInfo->pData + entry->offset + i * entry->stride, size);
         offset += size;
      }
   }
}


static void
vk_free_cmd_push_constants2_khr(struct vk_cmd_queue *queue,
                                struct vk_cmd_queue_entry *cmd)
{
   vk_free(queue->alloc, (void*)cmd->u.push_constants2_khr.push_constants_info->pValues);
   vk_free(queue->alloc, (VkPushConstantsInfoKHR*)cmd->u.push_constants2_khr.push_constants_info);
   vk_free(queue->alloc, cmd);
}

VKAPI_ATTR void VKAPI_CALL lvp_CmdPushConstants2KHR(
   VkCommandBuffer                             commandBuffer,
   const VkPushConstantsInfoKHR* pPushConstantsInfo)
{
   LVP_FROM_HANDLE(lvp_cmd_buffer, cmd_buffer, commandBuffer);
   struct vk_cmd_queue_entry *cmd = vk_zalloc(cmd_buffer->vk.cmd_queue.alloc, vk_cmd_queue_type_sizes[VK_CMD_PUSH_CONSTANTS2_KHR], 8,
                                              VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd)
      return;

   cmd->type = VK_CMD_PUSH_CONSTANTS2_KHR;
      
   cmd->u.push_constants2_khr.push_constants_info = vk_zalloc(cmd_buffer->vk.cmd_queue.alloc, sizeof(VkPushConstantsInfoKHR), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   memcpy((void*)cmd->u.push_constants2_khr.push_constants_info, pPushConstantsInfo, sizeof(VkPushConstantsInfoKHR));

   cmd->u.push_constants2_khr.push_constants_info->pValues = vk_zalloc(cmd_buffer->vk.cmd_queue.alloc, pPushConstantsInfo->size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   memcpy((void*)cmd->u.push_constants2_khr.push_constants_info->pValues, pPushConstantsInfo->pValues, pPushConstantsInfo->size);

   list_addtail(&cmd->cmd_link, &cmd_buffer->vk.cmd_queue.cmds);
}


static void
lvp_free_cmd_push_descriptor_set2_khr(struct vk_cmd_queue *queue,
                                     struct vk_cmd_queue_entry *cmd)
{
   ralloc_free(cmd->driver_data);
}

VKAPI_ATTR void VKAPI_CALL lvp_CmdPushDescriptorSet2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetInfoKHR*           pPushDescriptorSetInfo)
{
   LVP_FROM_HANDLE(lvp_cmd_buffer, cmd_buffer, commandBuffer);
   struct vk_cmd_queue_entry *cmd = vk_zalloc(cmd_buffer->vk.cmd_queue.alloc, vk_cmd_queue_type_sizes[VK_CMD_PUSH_DESCRIPTOR_SET2_KHR], 8,
                                              VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

   cmd->type = VK_CMD_PUSH_DESCRIPTOR_SET2_KHR;
   cmd->driver_free_cb = lvp_free_cmd_push_descriptor_set2_khr;

   void *ctx = cmd->driver_data = ralloc_context(NULL);
   if (pPushDescriptorSetInfo) {
      cmd->u.push_descriptor_set2_khr.push_descriptor_set_info = vk_zalloc(cmd_buffer->vk.cmd_queue.alloc, sizeof(VkPushDescriptorSetInfoKHR), 8,
                                                                           VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      memcpy((void*)cmd->u.push_descriptor_set2_khr.push_descriptor_set_info, pPushDescriptorSetInfo, sizeof(VkPushDescriptorSetInfoKHR));
      VkPushDescriptorSetInfoKHR *tmp_dst1 = (void *) cmd->u.push_descriptor_set2_khr.push_descriptor_set_info; (void) tmp_dst1;
      VkPushDescriptorSetInfoKHR *tmp_src1 = (void *) pPushDescriptorSetInfo; (void) tmp_src1;

      const VkBaseInStructure *pnext = tmp_dst1->pNext;
      if (pnext) {
         switch ((int32_t)pnext->sType) {
         case VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO:
            if (pnext) {
               tmp_dst1->pNext = rzalloc(ctx, VkPipelineLayoutCreateInfo);

               memcpy((void*)tmp_dst1->pNext, pnext, sizeof(VkPipelineLayoutCreateInfo));
               VkPipelineLayoutCreateInfo *tmp_dst2 = (void *) tmp_dst1->pNext; (void) tmp_dst2;
               VkPipelineLayoutCreateInfo *tmp_src2 = (void *) pnext; (void) tmp_src2;
               if (tmp_src2->pSetLayouts) {
                  tmp_dst2->pSetLayouts = rzalloc_array_size(ctx, sizeof(*tmp_dst2->pSetLayouts), tmp_dst2->setLayoutCount);

                  memcpy((void*)tmp_dst2->pSetLayouts, tmp_src2->pSetLayouts, sizeof(*tmp_dst2->pSetLayouts) * tmp_dst2->setLayoutCount);
               }
               if (tmp_src2->pPushConstantRanges) {
                  tmp_dst2->pPushConstantRanges = rzalloc_array_size(ctx, sizeof(*tmp_dst2->pPushConstantRanges), tmp_dst2->pushConstantRangeCount);

                  memcpy((void*)tmp_dst2->pPushConstantRanges, tmp_src2->pPushConstantRanges, sizeof(*tmp_dst2->pPushConstantRanges) * tmp_dst2->pushConstantRangeCount);
               }

            } else {
               tmp_dst1->pNext = NULL;
            }
            break;
         }
      }
      if (tmp_src1->pDescriptorWrites) {
         tmp_dst1->pDescriptorWrites = vk_zalloc(cmd_buffer->vk.cmd_queue.alloc, sizeof(*tmp_dst1->pDescriptorWrites) * tmp_dst1->descriptorWriteCount, 8,
                                                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

         memcpy((void*)tmp_dst1->pDescriptorWrites, tmp_src1->pDescriptorWrites, sizeof(*tmp_dst1->pDescriptorWrites) * tmp_dst1->descriptorWriteCount);
         for (unsigned i = 0; i < tmp_src1->descriptorWriteCount; i++) {
            VkWriteDescriptorSet *dstwrite = (void*)&tmp_dst1->pDescriptorWrites[i];
            const VkWriteDescriptorSet *write = &tmp_src1->pDescriptorWrites[i];
            switch (write->descriptorType) {
            case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
               const VkWriteDescriptorSetInlineUniformBlock *uniform_data = vk_find_struct_const(write->pNext, WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK);
               assert(uniform_data);
               VkWriteDescriptorSetInlineUniformBlock *dst = rzalloc(ctx, VkWriteDescriptorSetInlineUniformBlock);
               memcpy((void*)dst, uniform_data, sizeof(*uniform_data));
               dst->pData = ralloc_size(ctx, uniform_data->dataSize);
               memcpy((void*)dst->pData, uniform_data->pData, uniform_data->dataSize);
               dstwrite->pNext = dst;
               break;
            }

            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
               dstwrite->pImageInfo = rzalloc_array(ctx, VkDescriptorImageInfo, write->descriptorCount);
               {
                  VkDescriptorImageInfo *arr = (void*)dstwrite->pImageInfo;
                  typed_memcpy(arr, write->pImageInfo, write->descriptorCount);
               }
               break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
               dstwrite->pTexelBufferView = rzalloc_array(ctx, VkBufferView, write->descriptorCount);
               {
                  VkBufferView *arr = (void*)dstwrite->pTexelBufferView;
                  typed_memcpy(arr, write->pTexelBufferView, write->descriptorCount);
               }
               break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
               dstwrite->pBufferInfo = rzalloc_array(ctx, VkDescriptorBufferInfo, write->descriptorCount);
               {
                  VkDescriptorBufferInfo *arr = (void*)dstwrite->pBufferInfo;
                  typed_memcpy(arr, write->pBufferInfo, write->descriptorCount);
               }
               break;

            default:
               break;
            }
         }
      }

   } else {
      cmd->u.push_descriptor_set2_khr.push_descriptor_set_info = NULL;
   }

   list_addtail(&cmd->cmd_link, &cmd_buffer->vk.cmd_queue.cmds);
}
