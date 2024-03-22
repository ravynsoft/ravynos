/*
 * Copyright © 2015 Intel Corporation
 * Copyright © 2022 Collabora, Ltd
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

#include "vk_command_pool.h"

#include "vk_alloc.h"
#include "vk_command_buffer.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_log.h"

static bool
should_recycle_command_buffers(struct vk_device *device)
{
   /* They have to be using the common allocation implementation, otherwise
    * the recycled command buffers will never actually get re-used
    */
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   if (disp->AllocateCommandBuffers != vk_common_AllocateCommandBuffers)
      return false;

   /* We need to be able to reset command buffers */
   if (device->command_buffer_ops->reset == NULL)
      return false;

   return true;
}

VkResult MUST_CHECK
vk_command_pool_init(struct vk_device *device,
                     struct vk_command_pool *pool,
                     const VkCommandPoolCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *pAllocator)
{
   memset(pool, 0, sizeof(*pool));
   vk_object_base_init(device, &pool->base,
                       VK_OBJECT_TYPE_COMMAND_POOL);

   pool->flags = pCreateInfo->flags;
   pool->queue_family_index = pCreateInfo->queueFamilyIndex;
   pool->alloc = pAllocator ? *pAllocator : device->alloc;
   pool->command_buffer_ops = device->command_buffer_ops;
   pool->recycle_command_buffers = should_recycle_command_buffers(device);
   list_inithead(&pool->command_buffers);
   list_inithead(&pool->free_command_buffers);

   return VK_SUCCESS;
}

void
vk_command_pool_finish(struct vk_command_pool *pool)
{
   list_for_each_entry_safe(struct vk_command_buffer, cmd_buffer,
                            &pool->command_buffers, pool_link) {
      cmd_buffer->ops->destroy(cmd_buffer);
   }
   assert(list_is_empty(&pool->command_buffers));

   list_for_each_entry_safe(struct vk_command_buffer, cmd_buffer,
                            &pool->free_command_buffers, pool_link) {
      cmd_buffer->ops->destroy(cmd_buffer);
   }
   assert(list_is_empty(&pool->free_command_buffers));

   vk_object_base_finish(&pool->base);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_CreateCommandPool(VkDevice _device,
                            const VkCommandPoolCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator,
                            VkCommandPool *pCommandPool)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct vk_command_pool *pool;
   VkResult result;

   pool = vk_alloc2(&device->alloc, pAllocator, sizeof(*pool), 8,
                    VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (pool == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = vk_command_pool_init(device, pool, pCreateInfo, pAllocator);
   if (unlikely(result != VK_SUCCESS)) {
      vk_free2(&device->alloc, pAllocator, pool);
      return result;
   }

   *pCommandPool = vk_command_pool_to_handle(pool);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_DestroyCommandPool(VkDevice _device,
                             VkCommandPool commandPool,
                             const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_command_pool, pool, commandPool);

   if (pool == NULL)
      return;

   vk_command_pool_finish(pool);
   vk_free2(&device->alloc, pAllocator, pool);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_ResetCommandPool(VkDevice device,
                           VkCommandPool commandPool,
                           VkCommandPoolResetFlags flags)
{
   VK_FROM_HANDLE(vk_command_pool, pool, commandPool);
   const struct vk_device_dispatch_table *disp =
      &pool->base.device->dispatch_table;

#define COPY_FLAG(flag) \
   if (flags & VK_COMMAND_POOL_RESET_##flag) \
      cb_flags |= VK_COMMAND_BUFFER_RESET_##flag

   VkCommandBufferResetFlags cb_flags = 0;
   COPY_FLAG(RELEASE_RESOURCES_BIT);

#undef COPY_FLAG

   list_for_each_entry_safe(struct vk_command_buffer, cmd_buffer,
                            &pool->command_buffers, pool_link) {
      VkResult result =
         disp->ResetCommandBuffer(vk_command_buffer_to_handle(cmd_buffer),
                                  cb_flags);
      if (result != VK_SUCCESS)
         return result;
   }

   return VK_SUCCESS;
}

static void
vk_command_buffer_recycle_or_destroy(struct vk_command_pool *pool,
                                     struct vk_command_buffer *cmd_buffer)
{
   assert(pool == cmd_buffer->pool);

   if (pool->recycle_command_buffers) {
      vk_command_buffer_recycle(cmd_buffer);

      list_del(&cmd_buffer->pool_link);
      list_add(&cmd_buffer->pool_link, &pool->free_command_buffers);
   } else {
      cmd_buffer->ops->destroy(cmd_buffer);
   }
}

static struct vk_command_buffer *
vk_command_pool_find_free(struct vk_command_pool *pool)
{
   if (list_is_empty(&pool->free_command_buffers))
      return NULL;

   struct vk_command_buffer *cmd_buffer =
      list_first_entry(&pool->free_command_buffers,
                       struct vk_command_buffer, pool_link);

   list_del(&cmd_buffer->pool_link);
   list_addtail(&cmd_buffer->pool_link, &pool->command_buffers);

   return cmd_buffer;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_AllocateCommandBuffers(VkDevice device,
                                 const VkCommandBufferAllocateInfo *pAllocateInfo,
                                 VkCommandBuffer *pCommandBuffers)
{
   VK_FROM_HANDLE(vk_command_pool, pool, pAllocateInfo->commandPool);
   VkResult result;
   uint32_t i;

   assert(device == vk_device_to_handle(pool->base.device));

   for (i = 0; i < pAllocateInfo->commandBufferCount; i++) {
      struct vk_command_buffer *cmd_buffer = vk_command_pool_find_free(pool);
      if (cmd_buffer == NULL) {
         result = pool->command_buffer_ops->create(pool, &cmd_buffer);
         if (unlikely(result != VK_SUCCESS))
            goto fail;
      }

      cmd_buffer->level = pAllocateInfo->level;

      pCommandBuffers[i] = vk_command_buffer_to_handle(cmd_buffer);
   }

   return VK_SUCCESS;

fail:
   while (i--) {
      VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, pCommandBuffers[i]);
      vk_command_buffer_recycle_or_destroy(pool, cmd_buffer);
   }
   for (i = 0; i < pAllocateInfo->commandBufferCount; i++)
      pCommandBuffers[i] = VK_NULL_HANDLE;

   return result;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_FreeCommandBuffers(VkDevice device,
                             VkCommandPool commandPool,
                             uint32_t commandBufferCount,
                             const VkCommandBuffer *pCommandBuffers)
{
   VK_FROM_HANDLE(vk_command_pool, pool, commandPool);

   for (uint32_t i = 0; i < commandBufferCount; i++) {
      VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, pCommandBuffers[i]);

      if (cmd_buffer == NULL)
         continue;

      vk_command_buffer_recycle_or_destroy(pool, cmd_buffer);
   }
}

void
vk_command_pool_trim(struct vk_command_pool *pool,
                     VkCommandPoolTrimFlags flags)
{
   list_for_each_entry_safe(struct vk_command_buffer, cmd_buffer,
                            &pool->free_command_buffers, pool_link) {
      cmd_buffer->ops->destroy(cmd_buffer);
   }
   assert(list_is_empty(&pool->free_command_buffers));
}

VKAPI_ATTR void VKAPI_CALL
vk_common_TrimCommandPool(VkDevice device,
                          VkCommandPool commandPool,
                          VkCommandPoolTrimFlags flags)
{
   VK_FROM_HANDLE(vk_command_pool, pool, commandPool);

   vk_command_pool_trim(pool, flags);
}
