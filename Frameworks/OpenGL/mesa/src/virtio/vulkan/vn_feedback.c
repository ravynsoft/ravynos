/*
 * Copyright 2022 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "vn_feedback.h"

#include "vn_command_buffer.h"
#include "vn_device.h"
#include "vn_physical_device.h"
#include "vn_query_pool.h"
#include "vn_queue.h"

static uint32_t
vn_get_memory_type_index(const VkPhysicalDeviceMemoryProperties *mem_props,
                         uint32_t mem_type_bits,
                         VkMemoryPropertyFlags required_mem_flags)
{
   u_foreach_bit(mem_type_index, mem_type_bits)
   {
      assert(mem_type_index < mem_props->memoryTypeCount);
      if ((mem_props->memoryTypes[mem_type_index].propertyFlags &
           required_mem_flags) == required_mem_flags)
         return mem_type_index;
   }

   return UINT32_MAX;
}

VkResult
vn_feedback_buffer_create(struct vn_device *dev,
                          uint32_t size,
                          const VkAllocationCallbacks *alloc,
                          struct vn_feedback_buffer **out_feedback_buf)
{
   const bool exclusive = dev->queue_family_count == 1;
   const VkPhysicalDeviceMemoryProperties *mem_props =
      &dev->physical_device->memory_properties;
   VkDevice dev_handle = vn_device_to_handle(dev);
   struct vn_feedback_buffer *feedback_buf;
   VkResult result;

   feedback_buf = vk_zalloc(alloc, sizeof(*feedback_buf), VN_DEFAULT_ALIGN,
                            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!feedback_buf)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   /* use concurrent to avoid explicit queue family ownership transfer for
    * device created with queues from multiple queue families
    */
   const VkBufferCreateInfo buf_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      /* Feedback for fences and timeline semaphores will write to this buffer
       * as a DST when signalling. Timeline semaphore feedback will also read
       * from this buffer as a SRC to retrieve the counter value to signal.
       */
      .usage =
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      .sharingMode =
         exclusive ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
      /* below favors the current venus protocol */
      .queueFamilyIndexCount = exclusive ? 0 : dev->queue_family_count,
      .pQueueFamilyIndices = exclusive ? NULL : dev->queue_families,
   };
   result = vn_CreateBuffer(dev_handle, &buf_create_info, alloc,
                            &feedback_buf->buffer);
   if (result != VK_SUCCESS)
      goto out_free_feedback_buf;

   struct vn_buffer *buf = vn_buffer_from_handle(feedback_buf->buffer);
   const VkMemoryRequirements *mem_req =
      &buf->requirements.memory.memoryRequirements;
   const uint32_t mem_type_index =
      vn_get_memory_type_index(mem_props, mem_req->memoryTypeBits,
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
   if (mem_type_index >= mem_props->memoryTypeCount) {
      result = VK_ERROR_INITIALIZATION_FAILED;
      goto out_destroy_buffer;
   }

   const VkMemoryAllocateInfo mem_alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = mem_req->size,
      .memoryTypeIndex = mem_type_index,
   };
   result = vn_AllocateMemory(dev_handle, &mem_alloc_info, alloc,
                              &feedback_buf->memory);
   if (result != VK_SUCCESS)
      goto out_destroy_buffer;

   const VkBindBufferMemoryInfo bind_info = {
      .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
      .buffer = feedback_buf->buffer,
      .memory = feedback_buf->memory,
      .memoryOffset = 0,
   };
   result = vn_BindBufferMemory2(dev_handle, 1, &bind_info);
   if (result != VK_SUCCESS)
      goto out_free_memory;

   result = vn_MapMemory(dev_handle, feedback_buf->memory, 0, VK_WHOLE_SIZE,
                         0, &feedback_buf->data);
   if (result != VK_SUCCESS)
      goto out_free_memory;

   *out_feedback_buf = feedback_buf;

   return VK_SUCCESS;

out_free_memory:
   vn_FreeMemory(dev_handle, feedback_buf->memory, alloc);

out_destroy_buffer:
   vn_DestroyBuffer(dev_handle, feedback_buf->buffer, alloc);

out_free_feedback_buf:
   vk_free(alloc, feedback_buf);

   return result;
}

void
vn_feedback_buffer_destroy(struct vn_device *dev,
                           struct vn_feedback_buffer *feedback_buf,
                           const VkAllocationCallbacks *alloc)
{
   VkDevice dev_handle = vn_device_to_handle(dev);

   vn_UnmapMemory(dev_handle, feedback_buf->memory);
   vn_FreeMemory(dev_handle, feedback_buf->memory, alloc);
   vn_DestroyBuffer(dev_handle, feedback_buf->buffer, alloc);
   vk_free(alloc, feedback_buf);
}

static inline uint32_t
vn_get_feedback_buffer_alignment(struct vn_feedback_buffer *feedback_buf)
{
   struct vn_buffer *buf = vn_buffer_from_handle(feedback_buf->buffer);
   return buf->requirements.memory.memoryRequirements.alignment;
}

static VkResult
vn_feedback_pool_grow_locked(struct vn_feedback_pool *pool)
{
   VN_TRACE_FUNC();
   struct vn_feedback_buffer *feedback_buf = NULL;
   VkResult result;

   result = vn_feedback_buffer_create(pool->device, pool->size, pool->alloc,
                                      &feedback_buf);
   if (result != VK_SUCCESS)
      return result;

   pool->used = 0;
   pool->alignment = vn_get_feedback_buffer_alignment(feedback_buf);

   list_add(&feedback_buf->head, &pool->feedback_buffers);

   return VK_SUCCESS;
}

VkResult
vn_feedback_pool_init(struct vn_device *dev,
                      struct vn_feedback_pool *pool,
                      uint32_t size,
                      const VkAllocationCallbacks *alloc)
{
   simple_mtx_init(&pool->mutex, mtx_plain);

   pool->device = dev;
   pool->alloc = alloc;
   pool->size = size;
   pool->used = size;
   pool->alignment = 1;
   list_inithead(&pool->feedback_buffers);
   list_inithead(&pool->free_slots);

   return VK_SUCCESS;
}

void
vn_feedback_pool_fini(struct vn_feedback_pool *pool)
{
   list_for_each_entry_safe(struct vn_feedback_slot, slot, &pool->free_slots,
                            head)
      vk_free(pool->alloc, slot);

   list_for_each_entry_safe(struct vn_feedback_buffer, feedback_buf,
                            &pool->feedback_buffers, head)
      vn_feedback_buffer_destroy(pool->device, feedback_buf, pool->alloc);

   simple_mtx_destroy(&pool->mutex);
}

static struct vn_feedback_buffer *
vn_feedback_pool_alloc_locked(struct vn_feedback_pool *pool,
                              uint32_t size,
                              uint32_t *out_offset)
{
   VN_TRACE_FUNC();

   /* Default values of pool->used and pool->alignment are used to trigger the
    * initial pool grow, and will be properly initialized after that.
    */
   if (unlikely(align(size, pool->alignment) > pool->size - pool->used)) {
      VkResult result = vn_feedback_pool_grow_locked(pool);
      if (result != VK_SUCCESS)
         return NULL;

      assert(align(size, pool->alignment) <= pool->size - pool->used);
   }

   *out_offset = pool->used;
   pool->used += align(size, pool->alignment);

   return list_first_entry(&pool->feedback_buffers, struct vn_feedback_buffer,
                           head);
}

struct vn_feedback_slot *
vn_feedback_pool_alloc(struct vn_feedback_pool *pool,
                       enum vn_feedback_type type)
{
   /* TODO Make slot size variable for VkQueryPool feedback. Currently it's
    * MAX2(sizeof(VkResult), sizeof(uint64_t)).
    */
   static const uint32_t slot_size = 8;
   struct vn_feedback_buffer *feedback_buf;
   uint32_t offset;
   struct vn_feedback_slot *slot;

   simple_mtx_lock(&pool->mutex);
   if (!list_is_empty(&pool->free_slots)) {
      slot =
         list_first_entry(&pool->free_slots, struct vn_feedback_slot, head);
      list_del(&slot->head);
      simple_mtx_unlock(&pool->mutex);

      slot->type = type;
      return slot;
   }

   slot = vk_alloc(pool->alloc, sizeof(*slot), VN_DEFAULT_ALIGN,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!slot) {
      simple_mtx_unlock(&pool->mutex);
      return NULL;
   }

   feedback_buf = vn_feedback_pool_alloc_locked(pool, slot_size, &offset);
   simple_mtx_unlock(&pool->mutex);

   if (!feedback_buf) {
      vk_free(pool->alloc, slot);
      return NULL;
   }

   slot->type = type;
   slot->offset = offset;
   slot->buffer = feedback_buf->buffer;
   slot->data = feedback_buf->data + offset;

   return slot;
}

void
vn_feedback_pool_free(struct vn_feedback_pool *pool,
                      struct vn_feedback_slot *slot)
{
   simple_mtx_lock(&pool->mutex);
   list_add(&slot->head, &pool->free_slots);
   simple_mtx_unlock(&pool->mutex);
}

static inline bool
mask_is_32bit(uint64_t x)
{
   return (x & 0xffffffff00000000) == 0;
}

static void
vn_build_buffer_memory_barrier(const VkDependencyInfo *dep_info,
                               VkBufferMemoryBarrier *barrier1,
                               VkPipelineStageFlags *src_stage_mask,
                               VkPipelineStageFlags *dst_stage_mask)
{

   assert(dep_info->pNext == NULL);
   assert(dep_info->memoryBarrierCount == 0);
   assert(dep_info->bufferMemoryBarrierCount == 1);
   assert(dep_info->imageMemoryBarrierCount == 0);

   const VkBufferMemoryBarrier2 *barrier2 =
      &dep_info->pBufferMemoryBarriers[0];
   assert(barrier2->pNext == NULL);
   assert(mask_is_32bit(barrier2->srcStageMask));
   assert(mask_is_32bit(barrier2->srcAccessMask));
   assert(mask_is_32bit(barrier2->dstStageMask));
   assert(mask_is_32bit(barrier2->dstAccessMask));

   *barrier1 = (VkBufferMemoryBarrier){
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .pNext = NULL,
      .srcAccessMask = barrier2->srcAccessMask,
      .dstAccessMask = barrier2->dstAccessMask,
      .srcQueueFamilyIndex = barrier2->srcQueueFamilyIndex,
      .dstQueueFamilyIndex = barrier2->dstQueueFamilyIndex,
      .buffer = barrier2->buffer,
      .offset = barrier2->offset,
      .size = barrier2->size,
   };

   *src_stage_mask = barrier2->srcStageMask;
   *dst_stage_mask = barrier2->dstStageMask;
}

static void
vn_cmd_buffer_memory_barrier(VkCommandBuffer cmd_handle,
                             const VkDependencyInfo *dep_info,
                             bool sync2)
{
   if (sync2)
      vn_CmdPipelineBarrier2(cmd_handle, dep_info);
   else {
      VkBufferMemoryBarrier barrier1;
      VkPipelineStageFlags src_stage_mask;
      VkPipelineStageFlags dst_stage_mask;

      vn_build_buffer_memory_barrier(dep_info, &barrier1, &src_stage_mask,
                                     &dst_stage_mask);
      vn_CmdPipelineBarrier(cmd_handle, src_stage_mask, dst_stage_mask,
                            dep_info->dependencyFlags, 0, NULL, 1, &barrier1,
                            0, NULL);
   }
}

void
vn_feedback_event_cmd_record(VkCommandBuffer cmd_handle,
                             VkEvent ev_handle,
                             VkPipelineStageFlags2 src_stage_mask,
                             VkResult status,
                             bool sync2)
{
   /* For vkCmdSetEvent and vkCmdResetEvent feedback interception.
    *
    * The injection point is after the event call to avoid introducing
    * unexpected src stage waiting for VK_PIPELINE_STAGE_HOST_BIT and
    * VK_PIPELINE_STAGE_TRANSFER_BIT if they are not already being waited by
    * vkCmdSetEvent or vkCmdResetEvent. On the other hand, the delay in the
    * feedback signal is acceptable for the nature of VkEvent, and the event
    * feedback cmds lifecycle is guarded by the intercepted command buffer.
    */
   struct vn_event *ev = vn_event_from_handle(ev_handle);
   struct vn_feedback_slot *slot = ev->feedback_slot;

   if (!slot)
      return;

   STATIC_ASSERT(sizeof(*slot->status) == 4);

   const VkDependencyInfo dep_before = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .dependencyFlags = 0,
      .bufferMemoryBarrierCount = 1,
      .pBufferMemoryBarriers =
         (VkBufferMemoryBarrier2[]){
            {
               .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
               .srcStageMask = src_stage_mask | VK_PIPELINE_STAGE_HOST_BIT |
                               VK_PIPELINE_STAGE_TRANSFER_BIT,
               .srcAccessMask =
                  VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
               .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
               .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
               .buffer = slot->buffer,
               .offset = slot->offset,
               .size = 4,
            },
         },
   };
   vn_cmd_buffer_memory_barrier(cmd_handle, &dep_before, sync2);

   vn_CmdFillBuffer(cmd_handle, slot->buffer, slot->offset, 4, status);

   const VkDependencyInfo dep_after = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .dependencyFlags = 0,
      .bufferMemoryBarrierCount = 1,
      .pBufferMemoryBarriers =
         (VkBufferMemoryBarrier2[]){
            {
               .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
               .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
               .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
               .dstStageMask = VK_PIPELINE_STAGE_HOST_BIT,
               .dstAccessMask =
                  VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT,
               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
               .buffer = slot->buffer,
               .offset = slot->offset,
               .size = 4,
            },
         },
   };
   vn_cmd_buffer_memory_barrier(cmd_handle, &dep_after, sync2);
}

static inline void
vn_feedback_cmd_record_flush_barrier(VkCommandBuffer cmd_handle,
                                     VkBuffer buffer,
                                     VkDeviceSize offset,
                                     VkDeviceSize size)
{
   const VkBufferMemoryBarrier buf_flush_barrier = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .pNext = NULL,
      .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = buffer,
      .offset = offset,
      .size = size,
   };
   vn_CmdPipelineBarrier(cmd_handle, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_HOST_BIT, 0, 0, NULL, 1,
                         &buf_flush_barrier, 0, NULL);
}

static VkResult
vn_feedback_cmd_record(VkCommandBuffer cmd_handle,
                       struct vn_feedback_slot *dst_slot,
                       struct vn_feedback_slot *src_slot)
{
   STATIC_ASSERT(sizeof(*dst_slot->status) == 4);
   STATIC_ASSERT(sizeof(*dst_slot->counter) == 8);
   STATIC_ASSERT(sizeof(*src_slot->counter) == 8);

   /* slot size is 8 bytes for timeline semaphore and 4 bytes fence.
    * src slot is non-null for timeline semaphore.
    */
   const VkDeviceSize buf_size = src_slot ? 8 : 4;

   static const VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = NULL,
      .flags = 0,
      .pInheritanceInfo = NULL,
   };
   VkResult result = vn_BeginCommandBuffer(cmd_handle, &begin_info);
   if (result != VK_SUCCESS)
      return result;

   static const VkMemoryBarrier mem_barrier_before = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
      .pNext = NULL,
      /* make pending writes available to stay close to signal op */
      .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
      /* no need to make all memory visible for feedback update */
      .dstAccessMask = 0,
   };

   const VkBufferMemoryBarrier buf_barrier_before = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .pNext = NULL,
      /* slot memory has been made available via mem_barrier_before */
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = dst_slot->buffer,
      .offset = dst_slot->offset,
      .size = buf_size,
   };

   /* host writes for src_slots should implicitly be made visible upon
    * QueueSubmit call */
   vn_CmdPipelineBarrier(cmd_handle, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                         &mem_barrier_before, 1, &buf_barrier_before, 0,
                         NULL);

   /* If passed a src_slot, timeline semaphore feedback records a
    * cmd to copy the counter value from the src slot to the dst slot.
    * If src_slot is NULL, then fence feedback records a cmd to fill
    * the dst slot with VK_SUCCESS.
    */
   if (src_slot) {
      assert(src_slot->type == VN_FEEDBACK_TYPE_TIMELINE_SEMAPHORE);
      assert(dst_slot->type == VN_FEEDBACK_TYPE_TIMELINE_SEMAPHORE);

      const VkBufferCopy buffer_copy = {
         .srcOffset = src_slot->offset,
         .dstOffset = dst_slot->offset,
         .size = buf_size,
      };
      vn_CmdCopyBuffer(cmd_handle, src_slot->buffer, dst_slot->buffer, 1,
                       &buffer_copy);
   } else {
      assert(dst_slot->type == VN_FEEDBACK_TYPE_FENCE);

      vn_CmdFillBuffer(cmd_handle, dst_slot->buffer, dst_slot->offset,
                       buf_size, VK_SUCCESS);
   }

   vn_feedback_cmd_record_flush_barrier(cmd_handle, dst_slot->buffer,
                                        dst_slot->offset, buf_size);

   return vn_EndCommandBuffer(cmd_handle);
}

static void
vn_feedback_query_cmd_record(VkCommandBuffer cmd_handle,
                             VkQueryPool pool_handle,
                             uint32_t query,
                             uint32_t count,
                             bool copy)
{
   struct vn_query_pool *pool = vn_query_pool_from_handle(pool_handle);
   if (!pool->feedback)
      return;

   /* Results are always 64 bit and include availability bit (also 64 bit) */
   const VkDeviceSize slot_size = (pool->result_array_size * 8) + 8;
   const VkDeviceSize offset = slot_size * query;
   const VkDeviceSize buf_size = slot_size * count;

   /* The first synchronization scope of vkCmdCopyQueryPoolResults does not
    * include the query feedback buffer. Insert a barrier to ensure ordering
    * against feedback buffer fill cmd injected in vkCmdResetQueryPool.
    *
    * The second synchronization scope of vkCmdResetQueryPool does not include
    * the query feedback buffer. Insert a barrer to ensure ordering against
    * prior cmds referencing the queries.
    *
    * For srcAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT is sufficient since the
    * gpu cache invalidation for feedback buffer fill in vkResetQueryPool is
    * done implicitly via queue submission.
    */
   const VkPipelineStageFlags src_stage_mask =
      copy ? VK_PIPELINE_STAGE_TRANSFER_BIT
           : VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

   const VkBufferMemoryBarrier buf_barrier_before = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .pNext = NULL,
      .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = pool->feedback->buffer,
      .offset = offset,
      .size = buf_size,
   };
   vn_CmdPipelineBarrier(cmd_handle, src_stage_mask,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 1,
                         &buf_barrier_before, 0, NULL);

   if (copy) {
      /* Per spec: "The first synchronization scope includes all commands
       * which reference the queries in queryPool indicated by query that
       * occur earlier in submission order. If flags does not include
       * VK_QUERY_RESULT_WAIT_BIT, vkCmdEndQueryIndexedEXT,
       * vkCmdWriteTimestamp2, vkCmdEndQuery, and vkCmdWriteTimestamp are
       * excluded from this scope."
       *
       * Set VK_QUERY_RESULT_WAIT_BIT to ensure ordering after
       * vkCmdEndQuery or vkCmdWriteTimestamp makes the query available.
       *
       * Set VK_QUERY_RESULT_64_BIT as we can convert it to 32 bit if app
       * requested that.
       *
       * Per spec: "vkCmdCopyQueryPoolResults is considered to be a transfer
       * operation, and its writes to buffer memory must be synchronized using
       * VK_PIPELINE_STAGE_TRANSFER_BIT and VK_ACCESS_TRANSFER_WRITE_BIT
       * before using the results."
       *
       * So we can reuse the flush barrier after this copy cmd.
       */
      vn_CmdCopyQueryPoolResults(cmd_handle, pool_handle, query, count,
                                 pool->feedback->buffer, offset, slot_size,
                                 VK_QUERY_RESULT_WITH_AVAILABILITY_BIT |
                                    VK_QUERY_RESULT_64_BIT |
                                    VK_QUERY_RESULT_WAIT_BIT);
   } else {
      vn_CmdFillBuffer(cmd_handle, pool->feedback->buffer, offset, buf_size,
                       0);
   }

   vn_feedback_cmd_record_flush_barrier(cmd_handle, pool->feedback->buffer,
                                        offset, buf_size);
}

static void
vn_cmd_record_batched_query_feedback(VkCommandBuffer *cmd_handle,
                                     struct list_head *combined_query_batches)
{
   list_for_each_entry_safe(struct vn_feedback_query_batch, batch,
                            combined_query_batches, head) {
      vn_feedback_query_cmd_record(
         *cmd_handle, vn_query_pool_to_handle(batch->query_pool),
         batch->query, batch->query_count, batch->copy);
   }
}

VkResult
vn_feedback_query_batch_record(VkDevice dev_handle,
                               struct vn_feedback_cmd_pool *feedback_pool,
                               struct list_head *combined_query_batches,
                               VkCommandBuffer *out_cmd_handle)
{
   const VkCommandBufferAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = NULL,
      .commandPool = feedback_pool->pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
   };
   struct vn_command_pool *cmd_pool =
      vn_command_pool_from_handle(feedback_pool->pool);
   VkCommandBuffer feedback_cmd_handle;
   VkResult result;

   simple_mtx_lock(&feedback_pool->mutex);

   if (!list_is_empty(&cmd_pool->free_query_feedback_cmds)) {
      struct vn_command_buffer *free_cmd =
         list_first_entry(&cmd_pool->free_query_feedback_cmds,
                          struct vn_command_buffer, feedback_head);
      feedback_cmd_handle = vn_command_buffer_to_handle(free_cmd);
      list_del(&free_cmd->feedback_head);
   } else {
      result =
         vn_AllocateCommandBuffers(dev_handle, &info, &feedback_cmd_handle);
      if (result != VK_SUCCESS)
         goto out_unlock;
   }

   static const VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
   };

   result = vn_BeginCommandBuffer(feedback_cmd_handle, &begin_info);
   if (result != VK_SUCCESS) {
      vn_FreeCommandBuffers(dev_handle, feedback_pool->pool, 1,
                            &feedback_cmd_handle);
      goto out_unlock;
   }

   vn_cmd_record_batched_query_feedback(&feedback_cmd_handle,
                                        combined_query_batches);

   result = vn_EndCommandBuffer(feedback_cmd_handle);
   if (result != VK_SUCCESS) {
      vn_FreeCommandBuffers(dev_handle, feedback_pool->pool, 1,
                            &feedback_cmd_handle);
      goto out_unlock;
   }

   *out_cmd_handle = feedback_cmd_handle;

out_unlock:
   simple_mtx_unlock(&feedback_pool->mutex);

   return result;
}

VkResult
vn_feedback_cmd_alloc(VkDevice dev_handle,
                      struct vn_feedback_cmd_pool *pool,
                      struct vn_feedback_slot *dst_slot,
                      struct vn_feedback_slot *src_slot,
                      VkCommandBuffer *out_cmd_handle)
{
   const VkCommandBufferAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = NULL,
      .commandPool = pool->pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
   };
   VkCommandBuffer cmd_handle;
   VkResult result;

   simple_mtx_lock(&pool->mutex);
   result = vn_AllocateCommandBuffers(dev_handle, &info, &cmd_handle);
   if (result != VK_SUCCESS)
      goto out_unlock;

   result = vn_feedback_cmd_record(cmd_handle, dst_slot, src_slot);
   if (result != VK_SUCCESS) {
      vn_FreeCommandBuffers(dev_handle, pool->pool, 1, &cmd_handle);
      goto out_unlock;
   }

   *out_cmd_handle = cmd_handle;

out_unlock:
   simple_mtx_unlock(&pool->mutex);

   return result;
}

void
vn_feedback_cmd_free(VkDevice dev_handle,
                     struct vn_feedback_cmd_pool *pool,
                     VkCommandBuffer cmd_handle)
{
   simple_mtx_lock(&pool->mutex);
   vn_FreeCommandBuffers(dev_handle, pool->pool, 1, &cmd_handle);
   simple_mtx_unlock(&pool->mutex);
}

VkResult
vn_feedback_cmd_pools_init(struct vn_device *dev)
{
   const VkAllocationCallbacks *alloc = &dev->base.base.alloc;
   VkDevice dev_handle = vn_device_to_handle(dev);
   struct vn_feedback_cmd_pool *pools;
   VkCommandPoolCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = NULL,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
   };

   if (VN_PERF(NO_FENCE_FEEDBACK) && VN_PERF(NO_TIMELINE_SEM_FEEDBACK) &&
       VN_PERF(NO_QUERY_FEEDBACK))
      return VK_SUCCESS;

   assert(dev->queue_family_count);

   pools = vk_zalloc(alloc, sizeof(*pools) * dev->queue_family_count,
                     VN_DEFAULT_ALIGN, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!pools)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   for (uint32_t i = 0; i < dev->queue_family_count; i++) {
      VkResult result;

      info.queueFamilyIndex = dev->queue_families[i];
      result = vn_CreateCommandPool(dev_handle, &info, alloc, &pools[i].pool);
      if (result != VK_SUCCESS) {
         for (uint32_t j = 0; j < i; j++) {
            vn_DestroyCommandPool(dev_handle, pools[j].pool, alloc);
            simple_mtx_destroy(&pools[j].mutex);
         }

         vk_free(alloc, pools);
         return result;
      }

      simple_mtx_init(&pools[i].mutex, mtx_plain);
   }

   dev->cmd_pools = pools;

   return VK_SUCCESS;
}

void
vn_feedback_cmd_pools_fini(struct vn_device *dev)
{
   const VkAllocationCallbacks *alloc = &dev->base.base.alloc;
   VkDevice dev_handle = vn_device_to_handle(dev);

   if (!dev->cmd_pools)
      return;

   for (uint32_t i = 0; i < dev->queue_family_count; i++) {
      vn_DestroyCommandPool(dev_handle, dev->cmd_pools[i].pool, alloc);
      simple_mtx_destroy(&dev->cmd_pools[i].mutex);
   }

   vk_free(alloc, dev->cmd_pools);
}
