/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_COMMAND_BUFFER_H
#define VN_COMMAND_BUFFER_H

#include "vn_common.h"

#include "vn_cs.h"
#include "vn_feedback.h"

struct vn_command_pool {
   struct vn_object_base base;

   VkAllocationCallbacks allocator;
   struct vn_device *device;
   uint32_t queue_family_index;

   struct list_head command_buffers;

   struct list_head free_query_batches;
   struct list_head free_query_feedback_cmds;

   /* Temporary storage for scrubbing VK_IMAGE_LAYOUT_PRESENT_SRC_KHR. The
    * storage's lifetime is the command pool's lifetime. We increase the
    * storage as needed, but never shrink it. Upon used by the cmd buffer, the
    * storage must fit within command scope to avoid locking or suballocation.
    */
   struct {
      void *data;
      size_t size;
   } tmp;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_command_pool,
                               base.base,
                               VkCommandPool,
                               VK_OBJECT_TYPE_COMMAND_POOL)

enum vn_command_buffer_state {
   VN_COMMAND_BUFFER_STATE_INITIAL,
   VN_COMMAND_BUFFER_STATE_RECORDING,
   VN_COMMAND_BUFFER_STATE_EXECUTABLE,
   VN_COMMAND_BUFFER_STATE_INVALID,
};

/* command buffer builder to:
 * - fix wsi image ownership and layout transitions
 * - scrub ignored bits in VkCommandBufferBeginInfo
 * - support asynchronization query optimization (query feedback)
 */
struct vn_command_buffer_builder {
   /* track the active legacy render pass */
   const struct vn_render_pass *render_pass;
   /* track the wsi images requiring layout fixes */
   const struct vn_image **present_src_images;
   /* track if inside a render pass instance */
   bool in_render_pass;
   /* track the active subpass for view mask used in the subpass */
   uint32_t subpass_index;
   /* track the active view mask inside a render pass instance */
   uint32_t view_mask;
   /* track if VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT was set */
   bool is_simultaneous;
   /* track the query feedbacks deferred outside the render pass instance */
   struct list_head query_batches;
};

struct vn_command_buffer {
   struct vn_object_base base;

   struct vn_command_pool *pool;
   VkCommandBufferLevel level;
   enum vn_command_buffer_state state;
   struct vn_cs_encoder cs;

   uint32_t draw_cmd_batched;

   struct vn_command_buffer_builder builder;

   struct vn_command_buffer *linked_query_feedback_cmd;

   struct list_head head;

   struct list_head feedback_head;
};
VK_DEFINE_HANDLE_CASTS(vn_command_buffer,
                       base.base,
                       VkCommandBuffer,
                       VK_OBJECT_TYPE_COMMAND_BUFFER)

struct vn_feedback_query_batch *
vn_cmd_query_batch_alloc(struct vn_command_pool *pool,
                         struct vn_query_pool *query_pool,
                         uint32_t query,
                         uint32_t query_count,
                         bool copy);
#endif /* VN_COMMAND_BUFFER_H */
