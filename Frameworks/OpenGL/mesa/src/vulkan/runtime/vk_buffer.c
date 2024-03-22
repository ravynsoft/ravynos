/*
 * Copyright © 2022 Collabora, Ltd.
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

#include "vk_buffer.h"

#include "vk_common_entrypoints.h"
#include "vk_alloc.h"
#include "vk_device.h"
#include "vk_util.h"

void
vk_buffer_init(struct vk_device *device,
               struct vk_buffer *buffer,
               const VkBufferCreateInfo *pCreateInfo)
{
   vk_object_base_init(device, &buffer->base, VK_OBJECT_TYPE_BUFFER);

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
   assert(pCreateInfo->size > 0);

   buffer->create_flags = pCreateInfo->flags;
   buffer->size = pCreateInfo->size;
   buffer->usage = pCreateInfo->usage;

   const VkBufferUsageFlags2CreateInfoKHR *usage2_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           BUFFER_USAGE_FLAGS_2_CREATE_INFO_KHR);
   if (usage2_info != NULL)
      buffer->usage = usage2_info->usage;
}

void *
vk_buffer_create(struct vk_device *device,
                 const VkBufferCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *alloc,
                 size_t size)
{
   struct vk_buffer *buffer =
      vk_zalloc2(&device->alloc, alloc, size, 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (buffer == NULL)
      return NULL;

   vk_buffer_init(device, buffer, pCreateInfo);

   return buffer;
}

void
vk_buffer_finish(struct vk_buffer *buffer)
{
   vk_object_base_finish(&buffer->base);
}

void
vk_buffer_destroy(struct vk_device *device,
                  const VkAllocationCallbacks *alloc,
                  struct vk_buffer *buffer)
{
   vk_object_free(device, alloc, buffer);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetBufferMemoryRequirements(VkDevice _device,
                                      VkBuffer buffer,
                                      VkMemoryRequirements *pMemoryRequirements)
{
   VK_FROM_HANDLE(vk_device, device, _device);

   VkBufferMemoryRequirementsInfo2 info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
      .buffer = buffer,
   };
   VkMemoryRequirements2 reqs = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
   };
   device->dispatch_table.GetBufferMemoryRequirements2(_device, &info, &reqs);

   *pMemoryRequirements = reqs.memoryRequirements;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetBufferMemoryRequirements2(VkDevice _device,
                                       const VkBufferMemoryRequirementsInfo2 *pInfo,
                                       VkMemoryRequirements2 *pMemoryRequirements)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_buffer, buffer, pInfo->buffer);

   VkBufferCreateInfo pCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = NULL,
      .usage = buffer->usage,
      .size = buffer->size,
      .flags = buffer->create_flags,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = NULL,
   };
   VkDeviceBufferMemoryRequirements info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
      .pNext = NULL,
      .pCreateInfo = &pCreateInfo,
   };
   
   device->dispatch_table.GetDeviceBufferMemoryRequirements(_device, &info, pMemoryRequirements);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_BindBufferMemory(VkDevice _device,
                           VkBuffer buffer,
                           VkDeviceMemory memory,
                           VkDeviceSize memoryOffset)
{
   VK_FROM_HANDLE(vk_device, device, _device);

   VkBindBufferMemoryInfo bind = {
      .sType         = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
      .buffer        = buffer,
      .memory        = memory,
      .memoryOffset  = memoryOffset,
   };

   return device->dispatch_table.BindBufferMemory2(_device, 1, &bind);
}
