/*
 * Copyright 2022 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VN_FEEDBACK_H
#define VN_FEEDBACK_H

#include "vn_common.h"

struct vn_feedback_pool {
   /* single lock for simplicity though free_slots can use another */
   simple_mtx_t mutex;

   struct vn_device *device;
   const VkAllocationCallbacks *alloc;

   /* size in bytes of the feedback buffer */
   uint32_t size;
   /* size in bytes used of the active feedback buffer */
   uint32_t used;
   /* alignment in bytes for slot suballocation from the feedback buffer */
   uint32_t alignment;

   /* first entry is the active feedback buffer */
   struct list_head feedback_buffers;

   /* cache for returned feedback slots */
   struct list_head free_slots;
};

enum vn_feedback_type {
   VN_FEEDBACK_TYPE_FENCE,
   VN_FEEDBACK_TYPE_TIMELINE_SEMAPHORE,
   VN_FEEDBACK_TYPE_EVENT,
};

struct vn_feedback_slot {
   enum vn_feedback_type type;
   uint32_t offset;
   VkBuffer buffer;

   union {
      void *data;
      VkResult *status;
      uint64_t *counter;
   };

   struct list_head head;
};

struct vn_feedback_cmd_pool {
   simple_mtx_t mutex;

   VkCommandPool pool;
};

/* coherent buffer with bound and mapped memory */
struct vn_feedback_buffer {
   VkBuffer buffer;
   VkDeviceMemory memory;
   void *data;

   struct list_head head;
};

/* query feedback batch for deferred recording */
struct vn_feedback_query_batch {
   struct vn_query_pool *query_pool;
   uint32_t query;
   uint32_t query_count;
   bool copy;

   struct list_head head;
};

VkResult
vn_feedback_pool_init(struct vn_device *dev,
                      struct vn_feedback_pool *pool,
                      uint32_t size,
                      const VkAllocationCallbacks *alloc);

void
vn_feedback_pool_fini(struct vn_feedback_pool *pool);

struct vn_feedback_slot *
vn_feedback_pool_alloc(struct vn_feedback_pool *pool,
                       enum vn_feedback_type type);

void
vn_feedback_pool_free(struct vn_feedback_pool *pool,
                      struct vn_feedback_slot *slot);

static inline VkResult
vn_feedback_get_status(struct vn_feedback_slot *slot)
{
   return *slot->status;
}

static inline void
vn_feedback_reset_status(struct vn_feedback_slot *slot)
{
   assert(slot->type == VN_FEEDBACK_TYPE_FENCE ||
          slot->type == VN_FEEDBACK_TYPE_EVENT);
   *slot->status =
      slot->type == VN_FEEDBACK_TYPE_FENCE ? VK_NOT_READY : VK_EVENT_RESET;
}

static inline void
vn_feedback_set_status(struct vn_feedback_slot *slot, VkResult status)
{
   assert(slot->type == VN_FEEDBACK_TYPE_FENCE ||
          slot->type == VN_FEEDBACK_TYPE_EVENT);
   *slot->status = status;
}

static inline uint64_t
vn_feedback_get_counter(struct vn_feedback_slot *slot)
{
   assert(slot->type == VN_FEEDBACK_TYPE_TIMELINE_SEMAPHORE);
   return *slot->counter;
}

static inline void
vn_feedback_set_counter(struct vn_feedback_slot *slot, uint64_t counter)
{
   assert(slot->type == VN_FEEDBACK_TYPE_TIMELINE_SEMAPHORE);
   *slot->counter = counter;
}

VkResult
vn_feedback_buffer_create(struct vn_device *dev,
                          uint32_t size,
                          const VkAllocationCallbacks *alloc,
                          struct vn_feedback_buffer **out_feedback_buf);

void
vn_feedback_buffer_destroy(struct vn_device *dev,
                           struct vn_feedback_buffer *feedback_buf,
                           const VkAllocationCallbacks *alloc);

void
vn_feedback_event_cmd_record(VkCommandBuffer cmd_handle,
                             VkEvent ev_handle,
                             VkPipelineStageFlags2 src_stage_mask,
                             VkResult status,
                             bool sync2);

VkResult
vn_feedback_query_batch_record(VkDevice dev_handle,
                               struct vn_feedback_cmd_pool *feedback_pool,
                               struct list_head *combined_query_batches,
                               VkCommandBuffer *out_cmd_handle);

VkResult
vn_feedback_cmd_alloc(VkDevice dev_handle,
                      struct vn_feedback_cmd_pool *pool,
                      struct vn_feedback_slot *dst_slot,
                      struct vn_feedback_slot *src_slot,
                      VkCommandBuffer *out_cmd_handle);
void
vn_feedback_cmd_free(VkDevice dev_handle,
                     struct vn_feedback_cmd_pool *pool,
                     VkCommandBuffer cmd_handle);

VkResult
vn_feedback_cmd_pools_init(struct vn_device *dev);

void
vn_feedback_cmd_pools_fini(struct vn_device *dev);

#endif /* VN_FEEDBACK_H */
