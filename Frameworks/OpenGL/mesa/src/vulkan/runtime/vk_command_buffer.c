/*
 * Copyright © 2021 Intel Corporation
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

#include "vk_command_buffer.h"

#include "vk_command_pool.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"

VkResult
vk_command_buffer_init(struct vk_command_pool *pool,
                       struct vk_command_buffer *command_buffer,
                       const struct vk_command_buffer_ops *ops,
                       VkCommandBufferLevel level)
{
   memset(command_buffer, 0, sizeof(*command_buffer));
   vk_object_base_init(pool->base.device, &command_buffer->base,
                       VK_OBJECT_TYPE_COMMAND_BUFFER);

   command_buffer->pool = pool;
   command_buffer->level = level;
   command_buffer->ops = ops;
   vk_dynamic_graphics_state_init(&command_buffer->dynamic_graphics_state);
   command_buffer->state = MESA_VK_COMMAND_BUFFER_STATE_INITIAL;
   command_buffer->record_result = VK_SUCCESS;
   vk_cmd_queue_init(&command_buffer->cmd_queue, &pool->alloc);
   vk_meta_object_list_init(&command_buffer->meta_objects);
   util_dynarray_init(&command_buffer->labels, NULL);
   command_buffer->region_begin = true;

   list_add(&command_buffer->pool_link, &pool->command_buffers);

   return VK_SUCCESS;
}

void
vk_command_buffer_reset(struct vk_command_buffer *command_buffer)
{
   vk_dynamic_graphics_state_clear(&command_buffer->dynamic_graphics_state);
   command_buffer->state = MESA_VK_COMMAND_BUFFER_STATE_INITIAL;
   command_buffer->record_result = VK_SUCCESS;
   vk_command_buffer_reset_render_pass(command_buffer);
   vk_cmd_queue_reset(&command_buffer->cmd_queue);
   vk_meta_object_list_reset(command_buffer->base.device,
                             &command_buffer->meta_objects);
   util_dynarray_clear(&command_buffer->labels);
   command_buffer->region_begin = true;
}

void
vk_command_buffer_begin(struct vk_command_buffer *command_buffer,
                        const VkCommandBufferBeginInfo *pBeginInfo)
{
   if (command_buffer->state != MESA_VK_COMMAND_BUFFER_STATE_INITIAL &&
       command_buffer->ops->reset != NULL)
      command_buffer->ops->reset(command_buffer, 0);

   command_buffer->state = MESA_VK_COMMAND_BUFFER_STATE_RECORDING;
}

VkResult
vk_command_buffer_end(struct vk_command_buffer *command_buffer)
{
   assert(command_buffer->state == MESA_VK_COMMAND_BUFFER_STATE_RECORDING);

   if (vk_command_buffer_has_error(command_buffer))
      command_buffer->state = MESA_VK_COMMAND_BUFFER_STATE_INVALID;
   else
      command_buffer->state = MESA_VK_COMMAND_BUFFER_STATE_EXECUTABLE;

   return vk_command_buffer_get_record_result(command_buffer);
}

void
vk_command_buffer_finish(struct vk_command_buffer *command_buffer)
{
   list_del(&command_buffer->pool_link);
   vk_command_buffer_reset_render_pass(command_buffer);
   vk_cmd_queue_finish(&command_buffer->cmd_queue);
   util_dynarray_fini(&command_buffer->labels);
   vk_meta_object_list_finish(command_buffer->base.device,
                              &command_buffer->meta_objects);
   vk_object_base_finish(&command_buffer->base);
}

void
vk_command_buffer_recycle(struct vk_command_buffer *cmd_buffer)
{
   /* Reset, returning resources to the pool.  The command buffer object
    * itself will be recycled but, if the driver supports returning other
    * resources such as batch buffers to the pool, it should do so so they're
    * not tied up in recycled command buffer objects.
    */
   cmd_buffer->ops->reset(cmd_buffer,
      VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

   vk_object_base_recycle(&cmd_buffer->base);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_ResetCommandBuffer(VkCommandBuffer commandBuffer,
                             VkCommandBufferResetFlags flags)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);

   if (cmd_buffer->state != MESA_VK_COMMAND_BUFFER_STATE_INITIAL)
      cmd_buffer->ops->reset(cmd_buffer, flags);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdExecuteCommands(VkCommandBuffer commandBuffer,
                             uint32_t commandBufferCount,
                             const VkCommandBuffer *pCommandBuffers)
{
   VK_FROM_HANDLE(vk_command_buffer, primary, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      primary->base.device->command_dispatch_table;

   for (uint32_t i = 0; i < commandBufferCount; i++) {
      VK_FROM_HANDLE(vk_command_buffer, secondary, pCommandBuffers[i]);

      vk_cmd_queue_execute(&secondary->cmd_queue, commandBuffer, disp);
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdBindVertexBuffers(VkCommandBuffer commandBuffer,
                               uint32_t firstBinding,
                               uint32_t bindingCount,
                               const VkBuffer *pBuffers,
                               const VkDeviceSize *pOffsets)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      &cmd_buffer->base.device->dispatch_table;

   disp->CmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount,
                               pBuffers, pOffsets, NULL, NULL);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      &cmd_buffer->base.device->dispatch_table;

   disp->CmdBindIndexBuffer2KHR(commandBuffer, buffer, offset,
                                VK_WHOLE_SIZE, indexType);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdDispatch(VkCommandBuffer commandBuffer,
                      uint32_t groupCountX,
                      uint32_t groupCountY,
                      uint32_t groupCountZ)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      &cmd_buffer->base.device->dispatch_table;

   disp->CmdDispatchBase(commandBuffer, 0, 0, 0,
                         groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
   /* Nothing to do here since we only support a single device */
   assert(deviceMask == 0x1);
}

VkShaderStageFlags
vk_shader_stages_from_bind_point(VkPipelineBindPoint pipelineBindPoint)
{
   switch (pipelineBindPoint) {
#ifdef VK_ENABLE_BETA_EXTENSIONS
    case VK_PIPELINE_BIND_POINT_EXECUTION_GRAPH_AMDX:
      return VK_SHADER_STAGE_COMPUTE_BIT | MESA_VK_SHADER_STAGE_WORKGRAPH_HACK_BIT_FIXME;
#endif
   case VK_PIPELINE_BIND_POINT_COMPUTE:
      return VK_SHADER_STAGE_COMPUTE_BIT;
   case VK_PIPELINE_BIND_POINT_GRAPHICS:
      return VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
   case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
      return VK_SHADER_STAGE_RAYGEN_BIT_KHR |
             VK_SHADER_STAGE_ANY_HIT_BIT_KHR |
             VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
             VK_SHADER_STAGE_MISS_BIT_KHR |
             VK_SHADER_STAGE_INTERSECTION_BIT_KHR |
             VK_SHADER_STAGE_CALLABLE_BIT_KHR;
   default:
      unreachable("unknown bind point!");
   }
   return 0;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets)
{
   const VkBindDescriptorSetsInfoKHR two = {
      .sType = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO_KHR,
      .stageFlags = vk_shader_stages_from_bind_point(pipelineBindPoint),
      .layout = layout,
      .firstSet = firstSet,
      .descriptorSetCount = descriptorSetCount,
      .pDescriptorSets = pDescriptorSets,
      .dynamicOffsetCount = dynamicOffsetCount,
      .pDynamicOffsets = pDynamicOffsets
   };

   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      &cmd_buffer->base.device->dispatch_table;

   disp->CmdBindDescriptorSets2KHR(commandBuffer, &two);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues)
{
   const VkPushConstantsInfoKHR two = {
      .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO_KHR,
      .layout = layout,
      .stageFlags = stageFlags,
      .offset = offset,
      .size = size,
      .pValues = pValues,
   };

   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      &cmd_buffer->base.device->dispatch_table;

   disp->CmdPushConstants2KHR(commandBuffer, &two);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites)
{
   const VkPushDescriptorSetInfoKHR two = {
      .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO_KHR,
      .stageFlags = vk_shader_stages_from_bind_point(pipelineBindPoint),
      .layout = layout,
      .set = set,
      .descriptorWriteCount = descriptorWriteCount,
      .pDescriptorWrites = pDescriptorWrites,
   };

   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      &cmd_buffer->base.device->dispatch_table;

   disp->CmdPushDescriptorSet2KHR(commandBuffer, &two);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData)
{
   const VkPushDescriptorSetWithTemplateInfoKHR two = {
      .sType = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_WITH_TEMPLATE_INFO_KHR,
      .descriptorUpdateTemplate = descriptorUpdateTemplate,
      .layout = layout,
      .set = set,
      .pData = pData,
   };

   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      &cmd_buffer->base.device->dispatch_table;

   disp->CmdPushDescriptorSetWithTemplate2KHR(commandBuffer, &two);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDescriptorBufferOffsetsEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    setCount,
    const uint32_t*                             pBufferIndices,
    const VkDeviceSize*                         pOffsets)
{
   const VkSetDescriptorBufferOffsetsInfoEXT two = {
      .sType = VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT,
      .stageFlags = vk_shader_stages_from_bind_point(pipelineBindPoint),
      .layout = layout,
      .firstSet = firstSet,
      .setCount = setCount,
      .pBufferIndices = pBufferIndices,
      .pOffsets = pOffsets
   };

   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      &cmd_buffer->base.device->dispatch_table;

   disp->CmdSetDescriptorBufferOffsets2EXT(commandBuffer, &two);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdBindDescriptorBufferEmbeddedSamplersEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set)
{
   const VkBindDescriptorBufferEmbeddedSamplersInfoEXT two = {
      .sType = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_BUFFER_EMBEDDED_SAMPLERS_INFO_EXT,
      .stageFlags = vk_shader_stages_from_bind_point(pipelineBindPoint),
      .layout = layout,
      .set = set
   };

   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   const struct vk_device_dispatch_table *disp =
      &cmd_buffer->base.device->dispatch_table;

   disp->CmdBindDescriptorBufferEmbeddedSamplers2EXT(commandBuffer, &two);
}
