/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#include "vn_command_buffer.h"

#include "venus-protocol/vn_protocol_driver_command_buffer.h"
#include "venus-protocol/vn_protocol_driver_command_pool.h"

#include "vn_descriptor_set.h"
#include "vn_device.h"
#include "vn_image.h"
#include "vn_query_pool.h"
#include "vn_render_pass.h"

static void
vn_cmd_submit(struct vn_command_buffer *cmd);

#define VN_CMD_ENQUEUE(cmd_name, commandBuffer, ...)                         \
   do {                                                                      \
      struct vn_command_buffer *_cmd =                                       \
         vn_command_buffer_from_handle(commandBuffer);                       \
      size_t _cmd_size = vn_sizeof_##cmd_name(commandBuffer, ##__VA_ARGS__); \
                                                                             \
      if (vn_cs_encoder_reserve(&_cmd->cs, _cmd_size))                       \
         vn_encode_##cmd_name(&_cmd->cs, 0, commandBuffer, ##__VA_ARGS__);   \
      else                                                                   \
         _cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;                      \
                                                                             \
      if (VN_PERF(NO_CMD_BATCHING))                                          \
         vn_cmd_submit(_cmd);                                                \
   } while (0)

static bool
vn_image_memory_barrier_has_present_src(
   const VkImageMemoryBarrier *img_barriers, uint32_t count)
{
   for (uint32_t i = 0; i < count; i++) {
      if (img_barriers[i].oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ||
          img_barriers[i].newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
         return true;
   }
   return false;
}

static bool
vn_dependency_info_has_present_src(uint32_t dep_count,
                                   const VkDependencyInfo *dep_infos)
{
   for (uint32_t i = 0; i < dep_count; i++) {
      for (uint32_t j = 0; j < dep_infos[i].imageMemoryBarrierCount; j++) {
         const VkImageMemoryBarrier2 *b =
            &dep_infos[i].pImageMemoryBarriers[j];
         if (b->oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ||
             b->newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            return true;
         }
      }
   }

   return false;
}

static void *
vn_cmd_get_tmp_data(struct vn_command_buffer *cmd, size_t size)
{
   struct vn_command_pool *pool = cmd->pool;
   /* avoid shrinking in case of non efficient reallocation implementation */
   if (size > pool->tmp.size) {
      void *data =
         vk_realloc(&pool->allocator, pool->tmp.data, size, VN_DEFAULT_ALIGN,
                    VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (!data)
         return NULL;

      pool->tmp.data = data;
      pool->tmp.size = size;
   }

   return pool->tmp.data;
}

static inline VkImageMemoryBarrier *
vn_cmd_get_image_memory_barriers(struct vn_command_buffer *cmd,
                                 uint32_t count)
{
   return vn_cmd_get_tmp_data(cmd, count * sizeof(VkImageMemoryBarrier));
}

/* About VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, the spec says
 *
 *    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR must only be used for presenting a
 *    presentable image for display. A swapchain's image must be transitioned
 *    to this layout before calling vkQueuePresentKHR, and must be
 *    transitioned away from this layout after calling vkAcquireNextImageKHR.
 *
 * That allows us to treat the layout internally as
 *
 *  - VK_IMAGE_LAYOUT_GENERAL
 *  - VK_QUEUE_FAMILY_FOREIGN_EXT has the ownership, if the image is not a
 *    prime blit source
 *
 * while staying performant.
 *
 * About queue family ownerships, the spec says
 *
 *    A queue family can take ownership of an image subresource or buffer
 *    range of a resource created with VK_SHARING_MODE_EXCLUSIVE, without an
 *    ownership transfer, in the same way as for a resource that was just
 *    created; however, taking ownership in this way has the effect that the
 *    contents of the image subresource or buffer range are undefined.
 *
 * It is unclear if that is applicable to external resources, which supposedly
 * have the same semantics
 *
 *    Binding a resource to a memory object shared between multiple Vulkan
 *    instances or other APIs does not change the ownership of the underlying
 *    memory. The first entity to access the resource implicitly acquires
 *    ownership. Accessing a resource backed by memory that is owned by a
 *    particular instance or API has the same semantics as accessing a
 *    VK_SHARING_MODE_EXCLUSIVE resource[...]
 *
 * We should get the spec clarified, or get rid of this completely broken code
 * (TODO).
 *
 * Assuming a queue family can acquire the ownership implicitly when the
 * contents are not needed, we do not need to worry about
 * VK_IMAGE_LAYOUT_UNDEFINED.  We can use VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as
 * the sole signal to trigger queue family ownership transfers.
 *
 * When the image has VK_SHARING_MODE_CONCURRENT, we can, and are required to,
 * use VK_QUEUE_FAMILY_IGNORED as the other queue family whether we are
 * transitioning to or from VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
 *
 * When the image has VK_SHARING_MODE_EXCLUSIVE, we have to work out who the
 * other queue family is.  It is easier when the barrier does not also define
 * a queue family ownership transfer (i.e., srcQueueFamilyIndex equals to
 * dstQueueFamilyIndex).  The other queue family must be the queue family the
 * command buffer was allocated for.
 *
 * When the barrier also defines a queue family ownership transfer, it is
 * submitted both to the source queue family to release the ownership and to
 * the destination queue family to acquire the ownership.  Depending on
 * whether the barrier transitions to or from VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
 * we are only interested in the ownership release or acquire respectively and
 * should be careful to avoid double releases/acquires.
 *
 * I haven't followed all transition paths mentally to verify the correctness.
 * I likely also violate some VUs or miss some cases below.  They are
 * hopefully fixable and are left as TODOs.
 */
static void
vn_cmd_fix_image_memory_barrier(const struct vn_command_buffer *cmd,
                                const VkImageMemoryBarrier *src_barrier,
                                VkImageMemoryBarrier *out_barrier)
{
   const struct vn_image *img = vn_image_from_handle(src_barrier->image);

   *out_barrier = *src_barrier;

   /* no fix needed */
   if (out_barrier->oldLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
       out_barrier->newLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
      return;

   assert(img->wsi.is_wsi);

   if (VN_PRESENT_SRC_INTERNAL_LAYOUT == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
      return;

   /* prime blit src or no layout transition */
   if (img->wsi.is_prime_blit_src ||
       out_barrier->oldLayout == out_barrier->newLayout) {
      if (out_barrier->oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
         out_barrier->oldLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;
      if (out_barrier->newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
         out_barrier->newLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;
      return;
   }

   if (out_barrier->oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
      out_barrier->oldLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;

      /* no availability operation needed */
      out_barrier->srcAccessMask = 0;

      const uint32_t dst_qfi = out_barrier->dstQueueFamilyIndex;
      if (img->sharing_mode == VK_SHARING_MODE_CONCURRENT) {
         out_barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
         out_barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      } else if (dst_qfi == out_barrier->srcQueueFamilyIndex ||
                 dst_qfi == cmd->pool->queue_family_index) {
         out_barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
         out_barrier->dstQueueFamilyIndex = cmd->pool->queue_family_index;
      } else {
         /* The barrier also defines a queue family ownership transfer, and
          * this is the one that gets submitted to the source queue family to
          * release the ownership.  Skip both the transfer and the transition.
          */
         out_barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         out_barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         out_barrier->newLayout = out_barrier->oldLayout;
      }
   } else {
      out_barrier->newLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;

      /* no visibility operation needed */
      out_barrier->dstAccessMask = 0;

      const uint32_t src_qfi = out_barrier->srcQueueFamilyIndex;
      if (img->sharing_mode == VK_SHARING_MODE_CONCURRENT) {
         out_barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         out_barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
      } else if (src_qfi == out_barrier->dstQueueFamilyIndex ||
                 src_qfi == cmd->pool->queue_family_index) {
         out_barrier->srcQueueFamilyIndex = cmd->pool->queue_family_index;
         out_barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
      } else {
         /* The barrier also defines a queue family ownership transfer, and
          * this is the one that gets submitted to the destination queue
          * family to acquire the ownership.  Skip both the transfer and the
          * transition.
          */
         out_barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         out_barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         out_barrier->oldLayout = out_barrier->newLayout;
      }
   }
}

/** See vn_cmd_fix_image_memory_barrier(). */
static void
vn_cmd_fix_image_memory_barrier2(const struct vn_command_buffer *cmd,
                                 VkImageMemoryBarrier2 *b)
{
   if (VN_PRESENT_SRC_INTERNAL_LAYOUT == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
      return;

   if (b->oldLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
       b->newLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
      return;

   const struct vn_image *img = vn_image_from_handle(b->image);
   assert(img->wsi.is_wsi);

   if (img->wsi.is_prime_blit_src || b->oldLayout == b->newLayout) {
      if (b->oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
         b->oldLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;
      if (b->newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
         b->newLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;
      return;
   }

   if (b->oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
      b->oldLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;

      /* no availability operation needed */
      b->srcStageMask = 0;
      b->srcAccessMask = 0;

      if (img->sharing_mode == VK_SHARING_MODE_CONCURRENT) {
         b->srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
         b->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      } else if (b->dstQueueFamilyIndex == b->srcQueueFamilyIndex ||
                 b->dstQueueFamilyIndex == cmd->pool->queue_family_index) {
         b->srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
         b->dstQueueFamilyIndex = cmd->pool->queue_family_index;
      } else {
         /* The barrier also defines a queue family ownership transfer, and
          * this is the one that gets submitted to the source queue family to
          * release the ownership.  Skip both the transfer and the transition.
          */
         b->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         b->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         b->newLayout = b->oldLayout;
      }
   } else {
      b->newLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;

      /* no visibility operation needed */
      b->dstStageMask = 0;
      b->dstAccessMask = 0;

      if (img->sharing_mode == VK_SHARING_MODE_CONCURRENT) {
         b->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         b->dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
      } else if (b->srcQueueFamilyIndex == b->dstQueueFamilyIndex ||
                 b->srcQueueFamilyIndex == cmd->pool->queue_family_index) {
         b->srcQueueFamilyIndex = cmd->pool->queue_family_index;
         b->dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
      } else {
         /* The barrier also defines a queue family ownership transfer, and
          * this is the one that gets submitted to the destination queue
          * family to acquire the ownership.  Skip both the transfer and the
          * transition.
          */
         b->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         b->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         b->oldLayout = b->newLayout;
      }
   }
}

static const VkImageMemoryBarrier *
vn_cmd_wait_events_fix_image_memory_barriers(
   struct vn_command_buffer *cmd,
   const VkImageMemoryBarrier *src_barriers,
   uint32_t count,
   uint32_t *out_transfer_count)
{
   *out_transfer_count = 0;

   if (cmd->builder.in_render_pass ||
       !vn_image_memory_barrier_has_present_src(src_barriers, count))
      return src_barriers;

   VkImageMemoryBarrier *img_barriers =
      vn_cmd_get_image_memory_barriers(cmd, count * 2);
   if (!img_barriers) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      return src_barriers;
   }

   /* vkCmdWaitEvents cannot be used for queue family ownership transfers.
    * Nothing appears to be said about the submission order of image memory
    * barriers in the same array.  We take the liberty to move queue family
    * ownership transfers to the tail.
    */
   VkImageMemoryBarrier *transfer_barriers = img_barriers + count;
   uint32_t transfer_count = 0;
   uint32_t valid_count = 0;
   for (uint32_t i = 0; i < count; i++) {
      VkImageMemoryBarrier *img_barrier = &img_barriers[valid_count];
      vn_cmd_fix_image_memory_barrier(cmd, &src_barriers[i], img_barrier);

      if (VN_PRESENT_SRC_INTERNAL_LAYOUT == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
         valid_count++;
         continue;
      }

      if (img_barrier->srcQueueFamilyIndex ==
          img_barrier->dstQueueFamilyIndex) {
         valid_count++;
      } else {
         transfer_barriers[transfer_count++] = *img_barrier;
      }
   }

   assert(valid_count + transfer_count == count);
   if (transfer_count) {
      /* copy back to the tail */
      memcpy(&img_barriers[valid_count], transfer_barriers,
             sizeof(*transfer_barriers) * transfer_count);
      *out_transfer_count = transfer_count;
   }

   return img_barriers;
}

static const VkImageMemoryBarrier *
vn_cmd_pipeline_barrier_fix_image_memory_barriers(
   struct vn_command_buffer *cmd,
   const VkImageMemoryBarrier *src_barriers,
   uint32_t count)
{
   if (cmd->builder.in_render_pass ||
       !vn_image_memory_barrier_has_present_src(src_barriers, count))
      return src_barriers;

   VkImageMemoryBarrier *img_barriers =
      vn_cmd_get_image_memory_barriers(cmd, count);
   if (!img_barriers) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      return src_barriers;
   }

   for (uint32_t i = 0; i < count; i++) {
      vn_cmd_fix_image_memory_barrier(cmd, &src_barriers[i],
                                      &img_barriers[i]);
   }

   return img_barriers;
}

static const VkDependencyInfo *
vn_cmd_fix_dependency_infos(struct vn_command_buffer *cmd,
                            uint32_t dep_count,
                            const VkDependencyInfo *dep_infos)
{
   if (cmd->builder.in_render_pass ||
       !vn_dependency_info_has_present_src(dep_count, dep_infos))
      return dep_infos;

   uint32_t total_barrier_count = 0;
   for (uint32_t i = 0; i < dep_count; i++)
      total_barrier_count += dep_infos[i].imageMemoryBarrierCount;

   size_t tmp_size = dep_count * sizeof(VkDependencyInfo) +
                     total_barrier_count * sizeof(VkImageMemoryBarrier2);
   void *tmp = vn_cmd_get_tmp_data(cmd, tmp_size);
   if (!tmp) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      return dep_infos;
   }

   VkDependencyInfo *new_dep_infos = tmp;
   tmp += dep_count * sizeof(VkDependencyInfo);
   memcpy(new_dep_infos, dep_infos, dep_count * sizeof(VkDependencyInfo));

   for (uint32_t i = 0; i < dep_count; i++) {
      uint32_t barrier_count = dep_infos[i].imageMemoryBarrierCount;

      VkImageMemoryBarrier2 *new_barriers = tmp;
      tmp += barrier_count * sizeof(VkImageMemoryBarrier2);

      memcpy(new_barriers, dep_infos[i].pImageMemoryBarriers,
             barrier_count * sizeof(VkImageMemoryBarrier2));
      new_dep_infos[i].pImageMemoryBarriers = new_barriers;

      for (uint32_t j = 0; j < barrier_count; j++) {
         vn_cmd_fix_image_memory_barrier2(cmd, &new_barriers[j]);
      }
   }

   return new_dep_infos;
}

static void
vn_cmd_encode_memory_barriers(struct vn_command_buffer *cmd,
                              VkPipelineStageFlags src_stage_mask,
                              VkPipelineStageFlags dst_stage_mask,
                              uint32_t buf_barrier_count,
                              const VkBufferMemoryBarrier *buf_barriers,
                              uint32_t img_barrier_count,
                              const VkImageMemoryBarrier *img_barriers)
{
   const VkCommandBuffer cmd_handle = vn_command_buffer_to_handle(cmd);

   VN_CMD_ENQUEUE(vkCmdPipelineBarrier, cmd_handle, src_stage_mask,
                  dst_stage_mask, 0, 0, NULL, buf_barrier_count, buf_barriers,
                  img_barrier_count, img_barriers);
}

static void
vn_present_src_attachment_to_image_memory_barrier(
   const struct vn_image *img,
   const struct vn_present_src_attachment *att,
   VkImageMemoryBarrier *img_barrier,
   bool acquire)
{
   *img_barrier = (VkImageMemoryBarrier)
   {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = att->src_access_mask,
      .dstAccessMask = att->dst_access_mask,
      .oldLayout = acquire ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                           : VN_PRESENT_SRC_INTERNAL_LAYOUT,
      .newLayout = acquire ? VN_PRESENT_SRC_INTERNAL_LAYOUT
                           : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .image = vn_image_to_handle((struct vn_image *)img),
      .subresourceRange = {
         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .levelCount = 1,
         .layerCount = 1,
      },
   };
}

static void
vn_cmd_transfer_present_src_images(
   struct vn_command_buffer *cmd,
   bool acquire,
   const struct vn_image *const *images,
   const struct vn_present_src_attachment *atts,
   uint32_t count)
{
   VkImageMemoryBarrier *img_barriers =
      vn_cmd_get_image_memory_barriers(cmd, count);
   if (!img_barriers) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      return;
   }

   VkPipelineStageFlags src_stage_mask = 0;
   VkPipelineStageFlags dst_stage_mask = 0;
   for (uint32_t i = 0; i < count; i++) {
      src_stage_mask |= atts[i].src_stage_mask;
      dst_stage_mask |= atts[i].dst_stage_mask;

      vn_present_src_attachment_to_image_memory_barrier(
         images[i], &atts[i], &img_barriers[i], acquire);
      vn_cmd_fix_image_memory_barrier(cmd, &img_barriers[i],
                                      &img_barriers[i]);
   }

   if (VN_PRESENT_SRC_INTERNAL_LAYOUT == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
      return;

   vn_cmd_encode_memory_barriers(cmd, src_stage_mask, dst_stage_mask, 0, NULL,
                                 count, img_barriers);
}

struct vn_feedback_query_batch *
vn_cmd_query_batch_alloc(struct vn_command_pool *pool,
                         struct vn_query_pool *query_pool,
                         uint32_t query,
                         uint32_t query_count,
                         bool copy)
{
   struct vn_feedback_query_batch *batch;
   if (list_is_empty(&pool->free_query_batches)) {
      batch = vk_alloc(&pool->allocator, sizeof(*batch), VN_DEFAULT_ALIGN,
                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (!batch)
         return NULL;
   } else {
      batch = list_first_entry(&pool->free_query_batches,
                               struct vn_feedback_query_batch, head);
      list_del(&batch->head);
   }

   batch->query_pool = query_pool;
   batch->query = query;
   batch->query_count = query_count;
   batch->copy = copy;

   return batch;
}

static inline void
vn_cmd_merge_batched_query_feedback(struct vn_command_buffer *primary_cmd,
                                    struct vn_command_buffer *secondary_cmd)
{
   list_for_each_entry_safe(struct vn_feedback_query_batch, secondary_batch,
                            &secondary_cmd->builder.query_batches, head) {

      struct vn_feedback_query_batch *batch = vn_cmd_query_batch_alloc(
         primary_cmd->pool, secondary_batch->query_pool,
         secondary_batch->query, secondary_batch->query_count,
         secondary_batch->copy);

      if (!batch) {
         primary_cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
         return;
      }

      list_addtail(&batch->head, &primary_cmd->builder.query_batches);
   }
}

static void
vn_cmd_begin_render_pass(struct vn_command_buffer *cmd,
                         const struct vn_render_pass *pass,
                         const struct vn_framebuffer *fb,
                         const VkRenderPassBeginInfo *begin_info)
{
   assert(begin_info);
   assert(cmd->level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   cmd->builder.render_pass = pass;
   cmd->builder.in_render_pass = true;
   cmd->builder.subpass_index = 0;
   cmd->builder.view_mask = vn_render_pass_get_subpass_view_mask(pass, 0);

   if (!pass->present_count)
      return;

   /* find fb attachments */
   const VkImageView *views;
   ASSERTED uint32_t view_count;
   if (fb->image_view_count) {
      views = fb->image_views;
      view_count = fb->image_view_count;
   } else {
      const VkRenderPassAttachmentBeginInfo *imageless_info =
         vk_find_struct_const(begin_info->pNext,
                              RENDER_PASS_ATTACHMENT_BEGIN_INFO);
      assert(imageless_info);
      views = imageless_info->pAttachments;
      view_count = imageless_info->attachmentCount;
   }

   const struct vn_image **images =
      vk_alloc(&cmd->pool->allocator, sizeof(*images) * pass->present_count,
               VN_DEFAULT_ALIGN, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!images) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      return;
   }

   for (uint32_t i = 0; i < pass->present_count; i++) {
      const uint32_t index = pass->present_attachments[i].index;
      assert(index < view_count);
      images[i] = vn_image_view_from_handle(views[index])->image;
   }

   if (pass->present_acquire_count) {
      vn_cmd_transfer_present_src_images(cmd, true, images,
                                         pass->present_acquire_attachments,
                                         pass->present_acquire_count);
   }

   cmd->builder.present_src_images = images;
}

static void
vn_cmd_end_render_pass(struct vn_command_buffer *cmd)
{
   const struct vn_render_pass *pass = cmd->builder.render_pass;
   const struct vn_image **images = cmd->builder.present_src_images;

   cmd->builder.render_pass = NULL;
   cmd->builder.present_src_images = NULL;
   cmd->builder.in_render_pass = false;
   cmd->builder.subpass_index = 0;
   cmd->builder.view_mask = 0;

   if (!pass->present_count || !images)
      return;

   if (pass->present_release_count) {
      vn_cmd_transfer_present_src_images(
         cmd, false, images + pass->present_acquire_count,
         pass->present_release_attachments, pass->present_release_count);
   }

   vk_free(&cmd->pool->allocator, images);
}

static inline void
vn_cmd_next_subpass(struct vn_command_buffer *cmd)
{
   cmd->builder.view_mask = vn_render_pass_get_subpass_view_mask(
      cmd->builder.render_pass, ++cmd->builder.subpass_index);
}

static inline void
vn_cmd_begin_rendering(struct vn_command_buffer *cmd,
                       const VkRenderingInfo *rendering_info)
{
   cmd->builder.in_render_pass = true;
   cmd->builder.view_mask = rendering_info->viewMask;
}

static inline void
vn_cmd_end_rendering(struct vn_command_buffer *cmd)
{
   cmd->builder.in_render_pass = false;
   cmd->builder.view_mask = 0;
}

/* command pool commands */

VkResult
vn_CreateCommandPool(VkDevice device,
                     const VkCommandPoolCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *pAllocator,
                     VkCommandPool *pCommandPool)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   struct vn_command_pool *pool =
      vk_zalloc(alloc, sizeof(*pool), VN_DEFAULT_ALIGN,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!pool)
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   vn_object_base_init(&pool->base, VK_OBJECT_TYPE_COMMAND_POOL, &dev->base);

   pool->allocator = *alloc;
   pool->device = dev;
   pool->queue_family_index = pCreateInfo->queueFamilyIndex;
   list_inithead(&pool->command_buffers);
   list_inithead(&pool->free_query_batches);
   list_inithead(&pool->free_query_feedback_cmds);

   VkCommandPool pool_handle = vn_command_pool_to_handle(pool);
   vn_async_vkCreateCommandPool(dev->primary_ring, device, pCreateInfo, NULL,
                                &pool_handle);

   vn_tls_set_async_pipeline_create();

   *pCommandPool = pool_handle;

   return VK_SUCCESS;
}

static inline void
vn_recycle_query_feedback_cmd(struct vn_command_buffer *cmd)
{
   vn_ResetCommandBuffer(
      vn_command_buffer_to_handle(cmd->linked_query_feedback_cmd), 0);
   list_add(&cmd->linked_query_feedback_cmd->feedback_head,
            &cmd->linked_query_feedback_cmd->pool->free_query_feedback_cmds);
   cmd->linked_query_feedback_cmd = NULL;
}

void
vn_DestroyCommandPool(VkDevice device,
                      VkCommandPool commandPool,
                      const VkAllocationCallbacks *pAllocator)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_command_pool *pool = vn_command_pool_from_handle(commandPool);
   const VkAllocationCallbacks *alloc;

   if (!pool)
      return;

   alloc = pAllocator ? pAllocator : &pool->allocator;

   /* We must emit vkDestroyCommandPool before freeing the command buffers in
    * pool->command_buffers.  Otherwise, another thread might reuse their
    * object ids while they still refer to the command buffers in the
    * renderer.
    */
   vn_async_vkDestroyCommandPool(dev->primary_ring, device, commandPool,
                                 NULL);

   list_for_each_entry_safe(struct vn_command_buffer, cmd,
                            &pool->command_buffers, head) {
      vn_cs_encoder_fini(&cmd->cs);
      vn_object_base_fini(&cmd->base);

      if (cmd->builder.present_src_images)
         vk_free(alloc, cmd->builder.present_src_images);

      list_for_each_entry_safe(struct vn_feedback_query_batch, batch,
                               &cmd->builder.query_batches, head)
         vk_free(alloc, batch);

      if (cmd->linked_query_feedback_cmd)
         vn_recycle_query_feedback_cmd(cmd);

      vk_free(alloc, cmd);
   }

   list_for_each_entry_safe(struct vn_feedback_query_batch, batch,
                            &pool->free_query_batches, head)
      vk_free(alloc, batch);

   if (pool->tmp.data)
      vk_free(alloc, pool->tmp.data);

   vn_object_base_fini(&pool->base);
   vk_free(alloc, pool);
}

static void
vn_cmd_reset(struct vn_command_buffer *cmd)
{
   vn_cs_encoder_reset(&cmd->cs);

   cmd->state = VN_COMMAND_BUFFER_STATE_INITIAL;
   cmd->draw_cmd_batched = 0;

   if (cmd->builder.present_src_images)
      vk_free(&cmd->pool->allocator, cmd->builder.present_src_images);

   list_for_each_entry_safe(struct vn_feedback_query_batch, batch,
                            &cmd->builder.query_batches, head)
      list_move_to(&batch->head, &cmd->pool->free_query_batches);

   if (cmd->linked_query_feedback_cmd)
      vn_recycle_query_feedback_cmd(cmd);

   memset(&cmd->builder, 0, sizeof(cmd->builder));

   list_inithead(&cmd->builder.query_batches);
}

VkResult
vn_ResetCommandPool(VkDevice device,
                    VkCommandPool commandPool,
                    VkCommandPoolResetFlags flags)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_command_pool *pool = vn_command_pool_from_handle(commandPool);

   list_for_each_entry_safe(struct vn_command_buffer, cmd,
                            &pool->command_buffers, head)
      vn_cmd_reset(cmd);

   vn_async_vkResetCommandPool(dev->primary_ring, device, commandPool, flags);

   return VK_SUCCESS;
}

void
vn_TrimCommandPool(VkDevice device,
                   VkCommandPool commandPool,
                   VkCommandPoolTrimFlags flags)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);

   vn_async_vkTrimCommandPool(dev->primary_ring, device, commandPool, flags);
}

/* command buffer commands */

VkResult
vn_AllocateCommandBuffers(VkDevice device,
                          const VkCommandBufferAllocateInfo *pAllocateInfo,
                          VkCommandBuffer *pCommandBuffers)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_command_pool *pool =
      vn_command_pool_from_handle(pAllocateInfo->commandPool);
   const VkAllocationCallbacks *alloc = &pool->allocator;

   for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++) {
      struct vn_command_buffer *cmd =
         vk_zalloc(alloc, sizeof(*cmd), VN_DEFAULT_ALIGN,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (!cmd) {
         for (uint32_t j = 0; j < i; j++) {
            cmd = vn_command_buffer_from_handle(pCommandBuffers[j]);
            vn_cs_encoder_fini(&cmd->cs);
            list_del(&cmd->head);
            vn_object_base_fini(&cmd->base);
            vk_free(alloc, cmd);
         }
         memset(pCommandBuffers, 0,
                sizeof(*pCommandBuffers) * pAllocateInfo->commandBufferCount);
         return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      vn_object_base_init(&cmd->base, VK_OBJECT_TYPE_COMMAND_BUFFER,
                          &dev->base);
      cmd->pool = pool;
      cmd->level = pAllocateInfo->level;
      cmd->state = VN_COMMAND_BUFFER_STATE_INITIAL;
      vn_cs_encoder_init(&cmd->cs, dev->instance,
                         VN_CS_ENCODER_STORAGE_SHMEM_POOL, 16 * 1024);

      list_inithead(&cmd->builder.query_batches);

      list_addtail(&cmd->head, &pool->command_buffers);

      VkCommandBuffer cmd_handle = vn_command_buffer_to_handle(cmd);
      pCommandBuffers[i] = cmd_handle;
   }

   vn_async_vkAllocateCommandBuffers(dev->primary_ring, device, pAllocateInfo,
                                     pCommandBuffers);

   return VK_SUCCESS;
}

void
vn_FreeCommandBuffers(VkDevice device,
                      VkCommandPool commandPool,
                      uint32_t commandBufferCount,
                      const VkCommandBuffer *pCommandBuffers)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_command_pool *pool = vn_command_pool_from_handle(commandPool);
   const VkAllocationCallbacks *alloc = &pool->allocator;

   vn_async_vkFreeCommandBuffers(dev->primary_ring, device, commandPool,
                                 commandBufferCount, pCommandBuffers);

   for (uint32_t i = 0; i < commandBufferCount; i++) {
      struct vn_command_buffer *cmd =
         vn_command_buffer_from_handle(pCommandBuffers[i]);

      if (!cmd)
         continue;

      vn_cs_encoder_fini(&cmd->cs);
      list_del(&cmd->head);

      if (cmd->builder.present_src_images)
         vk_free(alloc, cmd->builder.present_src_images);

      list_for_each_entry_safe(struct vn_feedback_query_batch, batch,
                               &cmd->builder.query_batches, head)
         list_move_to(&batch->head, &cmd->pool->free_query_batches);

      if (cmd->linked_query_feedback_cmd)
         vn_recycle_query_feedback_cmd(cmd);

      vn_object_base_fini(&cmd->base);
      vk_free(alloc, cmd);
   }
}

VkResult
vn_ResetCommandBuffer(VkCommandBuffer commandBuffer,
                      VkCommandBufferResetFlags flags)
{
   VN_TRACE_FUNC();
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);
   struct vn_ring *ring = cmd->pool->device->primary_ring;

   vn_cmd_reset(cmd);

   vn_async_vkResetCommandBuffer(ring, commandBuffer, flags);

   return VK_SUCCESS;
}

struct vn_command_buffer_begin_info {
   VkCommandBufferBeginInfo begin;
   VkCommandBufferInheritanceInfo inheritance;
   VkCommandBufferInheritanceConditionalRenderingInfoEXT conditional_rendering;

   bool has_inherited_pass;
   bool in_render_pass;
};

static const VkCommandBufferBeginInfo *
vn_fix_command_buffer_begin_info(struct vn_command_buffer *cmd,
                                 const VkCommandBufferBeginInfo *begin_info,
                                 struct vn_command_buffer_begin_info *local)
{
   local->has_inherited_pass = false;

   if (!begin_info->pInheritanceInfo)
      return begin_info;

   const bool is_cmd_secondary =
      cmd->level == VK_COMMAND_BUFFER_LEVEL_SECONDARY;
   const bool has_continue =
      begin_info->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
   const bool has_renderpass =
      is_cmd_secondary &&
      begin_info->pInheritanceInfo->renderPass != VK_NULL_HANDLE;

   /* Per spec 1.3.255: "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
    * specifies that a secondary command buffer is considered to be
    * entirely inside a render pass. If this is a primary command buffer,
    * then this bit is ignored."
    */
   local->in_render_pass = has_continue && is_cmd_secondary;

   /* Can early-return if dynamic rendering is used and no structures need to
    * be dropped from the pNext chain of VkCommandBufferInheritanceInfo.
    */
   if (is_cmd_secondary && has_continue && !has_renderpass)
      return begin_info;

   local->begin = *begin_info;

   if (!is_cmd_secondary) {
      local->begin.pInheritanceInfo = NULL;
      return &local->begin;
   }

   local->inheritance = *begin_info->pInheritanceInfo;
   local->begin.pInheritanceInfo = &local->inheritance;

   if (!has_continue) {
      local->inheritance.framebuffer = VK_NULL_HANDLE;
      local->inheritance.renderPass = VK_NULL_HANDLE;
      local->inheritance.subpass = 0;
   } else {
      /* With early-returns above, it must be an inherited pass. */
      local->has_inherited_pass = true;
   }

   /* Per spec, about VkCommandBufferInheritanceRenderingInfo:
    *
    * If VkCommandBufferInheritanceInfo::renderPass is not VK_NULL_HANDLE, or
    * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT is not specified in
    * VkCommandBufferBeginInfo::flags, parameters of this structure are
    * ignored.
    */
   VkBaseOutStructure *head = NULL;
   VkBaseOutStructure *tail = NULL;
   vk_foreach_struct_const(src, local->inheritance.pNext) {
      void *pnext = NULL;
      switch (src->sType) {
      case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT:
         memcpy(
            &local->conditional_rendering, src,
            sizeof(VkCommandBufferInheritanceConditionalRenderingInfoEXT));
         pnext = &local->conditional_rendering;
         break;
      case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO:
      default:
         break;
      }

      if (pnext) {
         if (!head)
            head = pnext;
         else
            tail->pNext = pnext;

         tail = pnext;
      }
   }
   local->inheritance.pNext = head;

   return &local->begin;
}

VkResult
vn_BeginCommandBuffer(VkCommandBuffer commandBuffer,
                      const VkCommandBufferBeginInfo *pBeginInfo)
{
   VN_TRACE_FUNC();
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);
   struct vn_instance *instance = cmd->pool->device->instance;
   size_t cmd_size;

   /* reset regardless of VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT */
   vn_cmd_reset(cmd);

   struct vn_command_buffer_begin_info local_begin_info;
   pBeginInfo =
      vn_fix_command_buffer_begin_info(cmd, pBeginInfo, &local_begin_info);

   cmd_size = vn_sizeof_vkBeginCommandBuffer(commandBuffer, pBeginInfo);
   if (!vn_cs_encoder_reserve(&cmd->cs, cmd_size)) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      return vn_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);
   }
   cmd->builder.is_simultaneous =
      pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

   vn_encode_vkBeginCommandBuffer(&cmd->cs, 0, commandBuffer, pBeginInfo);

   cmd->state = VN_COMMAND_BUFFER_STATE_RECORDING;

   const VkCommandBufferInheritanceInfo *inheritance_info =
      pBeginInfo->pInheritanceInfo;

   if (inheritance_info) {
      cmd->builder.in_render_pass = local_begin_info.in_render_pass;

      if (local_begin_info.has_inherited_pass) {
         /* Store the viewMask from the inherited render pass subpass for
          * query feedback.
          */
         cmd->builder.view_mask = vn_render_pass_get_subpass_view_mask(
            vn_render_pass_from_handle(inheritance_info->renderPass),
            inheritance_info->subpass);
      } else {
         /* Store the viewMask from the
          * VkCommandBufferInheritanceRenderingInfo.
          */
         const VkCommandBufferInheritanceRenderingInfo
            *inheritance_rendering_info = vk_find_struct_const(
               inheritance_info->pNext,
               COMMAND_BUFFER_INHERITANCE_RENDERING_INFO);
         if (inheritance_rendering_info)
            cmd->builder.view_mask = inheritance_rendering_info->viewMask;
      }
   }

   return VK_SUCCESS;
}

static void
vn_cmd_submit(struct vn_command_buffer *cmd)
{
   struct vn_ring *ring = cmd->pool->device->primary_ring;

   if (cmd->state != VN_COMMAND_BUFFER_STATE_RECORDING)
      return;

   vn_cs_encoder_commit(&cmd->cs);
   if (vn_cs_encoder_get_fatal(&cmd->cs)) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      vn_cs_encoder_reset(&cmd->cs);
      return;
   }

   if (vn_ring_submit_command_simple(ring, &cmd->cs) != VK_SUCCESS) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      return;
   }

   vn_cs_encoder_reset(&cmd->cs);
   cmd->draw_cmd_batched = 0;
}

static inline void
vn_cmd_count_draw_and_submit_on_batch_limit(struct vn_command_buffer *cmd)
{
   if (++cmd->draw_cmd_batched >= vn_env.draw_cmd_batch_limit)
      vn_cmd_submit(cmd);
}

VkResult
vn_EndCommandBuffer(VkCommandBuffer commandBuffer)
{
   VN_TRACE_FUNC();
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);
   struct vn_instance *instance = cmd->pool->device->instance;
   size_t cmd_size;

   if (cmd->state != VN_COMMAND_BUFFER_STATE_RECORDING)
      return vn_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   cmd_size = vn_sizeof_vkEndCommandBuffer(commandBuffer);
   if (!vn_cs_encoder_reserve(&cmd->cs, cmd_size)) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      return vn_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   vn_encode_vkEndCommandBuffer(&cmd->cs, 0, commandBuffer);

   vn_cmd_submit(cmd);
   if (cmd->state == VN_COMMAND_BUFFER_STATE_INVALID)
      return vn_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   cmd->state = VN_COMMAND_BUFFER_STATE_EXECUTABLE;

   return VK_SUCCESS;
}

void
vn_CmdBindPipeline(VkCommandBuffer commandBuffer,
                   VkPipelineBindPoint pipelineBindPoint,
                   VkPipeline pipeline)
{
   VN_CMD_ENQUEUE(vkCmdBindPipeline, commandBuffer, pipelineBindPoint,
                  pipeline);
}

void
vn_CmdSetViewport(VkCommandBuffer commandBuffer,
                  uint32_t firstViewport,
                  uint32_t viewportCount,
                  const VkViewport *pViewports)
{
   VN_CMD_ENQUEUE(vkCmdSetViewport, commandBuffer, firstViewport,
                  viewportCount, pViewports);
}

void
vn_CmdSetScissor(VkCommandBuffer commandBuffer,
                 uint32_t firstScissor,
                 uint32_t scissorCount,
                 const VkRect2D *pScissors)
{
   VN_CMD_ENQUEUE(vkCmdSetScissor, commandBuffer, firstScissor, scissorCount,
                  pScissors);
}

void
vn_CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
   VN_CMD_ENQUEUE(vkCmdSetLineWidth, commandBuffer, lineWidth);
}

void
vn_CmdSetDepthBias(VkCommandBuffer commandBuffer,
                   float depthBiasConstantFactor,
                   float depthBiasClamp,
                   float depthBiasSlopeFactor)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthBias, commandBuffer, depthBiasConstantFactor,
                  depthBiasClamp, depthBiasSlopeFactor);
}

void
vn_CmdSetBlendConstants(VkCommandBuffer commandBuffer,
                        const float blendConstants[4])
{
   VN_CMD_ENQUEUE(vkCmdSetBlendConstants, commandBuffer, blendConstants);
}

void
vn_CmdSetDepthBounds(VkCommandBuffer commandBuffer,
                     float minDepthBounds,
                     float maxDepthBounds)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthBounds, commandBuffer, minDepthBounds,
                  maxDepthBounds);
}

void
vn_CmdSetStencilCompareMask(VkCommandBuffer commandBuffer,
                            VkStencilFaceFlags faceMask,
                            uint32_t compareMask)
{
   VN_CMD_ENQUEUE(vkCmdSetStencilCompareMask, commandBuffer, faceMask,
                  compareMask);
}

void
vn_CmdSetStencilWriteMask(VkCommandBuffer commandBuffer,
                          VkStencilFaceFlags faceMask,
                          uint32_t writeMask)
{
   VN_CMD_ENQUEUE(vkCmdSetStencilWriteMask, commandBuffer, faceMask,
                  writeMask);
}

void
vn_CmdSetStencilReference(VkCommandBuffer commandBuffer,
                          VkStencilFaceFlags faceMask,
                          uint32_t reference)
{
   VN_CMD_ENQUEUE(vkCmdSetStencilReference, commandBuffer, faceMask,
                  reference);
}

void
vn_CmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                         VkPipelineBindPoint pipelineBindPoint,
                         VkPipelineLayout layout,
                         uint32_t firstSet,
                         uint32_t descriptorSetCount,
                         const VkDescriptorSet *pDescriptorSets,
                         uint32_t dynamicOffsetCount,
                         const uint32_t *pDynamicOffsets)
{
   VN_CMD_ENQUEUE(vkCmdBindDescriptorSets, commandBuffer, pipelineBindPoint,
                  layout, firstSet, descriptorSetCount, pDescriptorSets,
                  dynamicOffsetCount, pDynamicOffsets);
}

void
vn_CmdBindIndexBuffer(VkCommandBuffer commandBuffer,
                      VkBuffer buffer,
                      VkDeviceSize offset,
                      VkIndexType indexType)
{
   VN_CMD_ENQUEUE(vkCmdBindIndexBuffer, commandBuffer, buffer, offset,
                  indexType);
}

void
vn_CmdBindVertexBuffers(VkCommandBuffer commandBuffer,
                        uint32_t firstBinding,
                        uint32_t bindingCount,
                        const VkBuffer *pBuffers,
                        const VkDeviceSize *pOffsets)
{
   VN_CMD_ENQUEUE(vkCmdBindVertexBuffers, commandBuffer, firstBinding,
                  bindingCount, pBuffers, pOffsets);
}

void
vn_CmdDraw(VkCommandBuffer commandBuffer,
           uint32_t vertexCount,
           uint32_t instanceCount,
           uint32_t firstVertex,
           uint32_t firstInstance)
{
   VN_CMD_ENQUEUE(vkCmdDraw, commandBuffer, vertexCount, instanceCount,
                  firstVertex, firstInstance);

   vn_cmd_count_draw_and_submit_on_batch_limit(
      vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdBeginRendering(VkCommandBuffer commandBuffer,
                     const VkRenderingInfo *pRenderingInfo)
{
   vn_cmd_begin_rendering(vn_command_buffer_from_handle(commandBuffer),
                          pRenderingInfo);

   VN_CMD_ENQUEUE(vkCmdBeginRendering, commandBuffer, pRenderingInfo);
}

void
vn_CmdEndRendering(VkCommandBuffer commandBuffer)
{
   VN_CMD_ENQUEUE(vkCmdEndRendering, commandBuffer);

   vn_cmd_end_rendering(vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdDrawIndexed(VkCommandBuffer commandBuffer,
                  uint32_t indexCount,
                  uint32_t instanceCount,
                  uint32_t firstIndex,
                  int32_t vertexOffset,
                  uint32_t firstInstance)
{
   VN_CMD_ENQUEUE(vkCmdDrawIndexed, commandBuffer, indexCount, instanceCount,
                  firstIndex, vertexOffset, firstInstance);

   vn_cmd_count_draw_and_submit_on_batch_limit(
      vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdDrawIndirect(VkCommandBuffer commandBuffer,
                   VkBuffer buffer,
                   VkDeviceSize offset,
                   uint32_t drawCount,
                   uint32_t stride)
{
   VN_CMD_ENQUEUE(vkCmdDrawIndirect, commandBuffer, buffer, offset, drawCount,
                  stride);

   vn_cmd_count_draw_and_submit_on_batch_limit(
      vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer,
                          VkBuffer buffer,
                          VkDeviceSize offset,
                          uint32_t drawCount,
                          uint32_t stride)
{
   VN_CMD_ENQUEUE(vkCmdDrawIndexedIndirect, commandBuffer, buffer, offset,
                  drawCount, stride);

   vn_cmd_count_draw_and_submit_on_batch_limit(
      vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdDrawIndirectCount(VkCommandBuffer commandBuffer,
                        VkBuffer buffer,
                        VkDeviceSize offset,
                        VkBuffer countBuffer,
                        VkDeviceSize countBufferOffset,
                        uint32_t maxDrawCount,
                        uint32_t stride)
{
   VN_CMD_ENQUEUE(vkCmdDrawIndirectCount, commandBuffer, buffer, offset,
                  countBuffer, countBufferOffset, maxDrawCount, stride);

   vn_cmd_count_draw_and_submit_on_batch_limit(
      vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer,
                               VkBuffer buffer,
                               VkDeviceSize offset,
                               VkBuffer countBuffer,
                               VkDeviceSize countBufferOffset,
                               uint32_t maxDrawCount,
                               uint32_t stride)
{
   VN_CMD_ENQUEUE(vkCmdDrawIndexedIndirectCount, commandBuffer, buffer,
                  offset, countBuffer, countBufferOffset, maxDrawCount,
                  stride);

   vn_cmd_count_draw_and_submit_on_batch_limit(
      vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdDispatch(VkCommandBuffer commandBuffer,
               uint32_t groupCountX,
               uint32_t groupCountY,
               uint32_t groupCountZ)
{
   VN_CMD_ENQUEUE(vkCmdDispatch, commandBuffer, groupCountX, groupCountY,
                  groupCountZ);
}

void
vn_CmdDispatchIndirect(VkCommandBuffer commandBuffer,
                       VkBuffer buffer,
                       VkDeviceSize offset)
{
   VN_CMD_ENQUEUE(vkCmdDispatchIndirect, commandBuffer, buffer, offset);
}

void
vn_CmdCopyBuffer(VkCommandBuffer commandBuffer,
                 VkBuffer srcBuffer,
                 VkBuffer dstBuffer,
                 uint32_t regionCount,
                 const VkBufferCopy *pRegions)
{
   VN_CMD_ENQUEUE(vkCmdCopyBuffer, commandBuffer, srcBuffer, dstBuffer,
                  regionCount, pRegions);
}

void
vn_CmdCopyBuffer2(VkCommandBuffer commandBuffer,
                  const VkCopyBufferInfo2 *pCopyBufferInfo)
{
   VN_CMD_ENQUEUE(vkCmdCopyBuffer2, commandBuffer, pCopyBufferInfo);
}

void
vn_CmdCopyImage(VkCommandBuffer commandBuffer,
                VkImage srcImage,
                VkImageLayout srcImageLayout,
                VkImage dstImage,
                VkImageLayout dstImageLayout,
                uint32_t regionCount,
                const VkImageCopy *pRegions)
{
   VN_CMD_ENQUEUE(vkCmdCopyImage, commandBuffer, srcImage, srcImageLayout,
                  dstImage, dstImageLayout, regionCount, pRegions);
}

void
vn_CmdCopyImage2(VkCommandBuffer commandBuffer,
                 const VkCopyImageInfo2 *pCopyImageInfo)
{
   VN_CMD_ENQUEUE(vkCmdCopyImage2, commandBuffer, pCopyImageInfo);
}

void
vn_CmdBlitImage(VkCommandBuffer commandBuffer,
                VkImage srcImage,
                VkImageLayout srcImageLayout,
                VkImage dstImage,
                VkImageLayout dstImageLayout,
                uint32_t regionCount,
                const VkImageBlit *pRegions,
                VkFilter filter)
{
   VN_CMD_ENQUEUE(vkCmdBlitImage, commandBuffer, srcImage, srcImageLayout,
                  dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void
vn_CmdBlitImage2(VkCommandBuffer commandBuffer,
                 const VkBlitImageInfo2 *pBlitImageInfo)
{
   VN_CMD_ENQUEUE(vkCmdBlitImage2, commandBuffer, pBlitImageInfo);
}

void
vn_CmdCopyBufferToImage(VkCommandBuffer commandBuffer,
                        VkBuffer srcBuffer,
                        VkImage dstImage,
                        VkImageLayout dstImageLayout,
                        uint32_t regionCount,
                        const VkBufferImageCopy *pRegions)
{
   VN_CMD_ENQUEUE(vkCmdCopyBufferToImage, commandBuffer, srcBuffer, dstImage,
                  dstImageLayout, regionCount, pRegions);
}

void
vn_CmdCopyBufferToImage2(
   VkCommandBuffer commandBuffer,
   const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo)
{
   VN_CMD_ENQUEUE(vkCmdCopyBufferToImage2, commandBuffer,
                  pCopyBufferToImageInfo);
}

static bool
vn_needs_prime_blit(VkImage src_image, VkImageLayout src_image_layout)
{
   if (src_image_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
       VN_PRESENT_SRC_INTERNAL_LAYOUT != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {

      /* sanity check */
      ASSERTED const struct vn_image *img = vn_image_from_handle(src_image);
      assert(img->wsi.is_wsi && img->wsi.is_prime_blit_src);
      return true;
   }

   return false;
}

static void
vn_transition_prime_layout(struct vn_command_buffer *cmd, VkBuffer dst_buffer)
{
   const VkBufferMemoryBarrier buf_barrier = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .srcQueueFamilyIndex = cmd->pool->queue_family_index,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT,
      .buffer = dst_buffer,
      .size = VK_WHOLE_SIZE,
   };
   vn_cmd_encode_memory_barriers(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 1,
                                 &buf_barrier, 0, NULL);
}

void
vn_CmdCopyImageToBuffer(VkCommandBuffer commandBuffer,
                        VkImage srcImage,
                        VkImageLayout srcImageLayout,
                        VkBuffer dstBuffer,
                        uint32_t regionCount,
                        const VkBufferImageCopy *pRegions)
{
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);

   bool prime_blit = vn_needs_prime_blit(srcImage, srcImageLayout);
   if (prime_blit)
      srcImageLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;

   VN_CMD_ENQUEUE(vkCmdCopyImageToBuffer, commandBuffer, srcImage,
                  srcImageLayout, dstBuffer, regionCount, pRegions);

   if (prime_blit)
      vn_transition_prime_layout(cmd, dstBuffer);
}

void
vn_CmdCopyImageToBuffer2(
   VkCommandBuffer commandBuffer,
   const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo)
{
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);
   struct VkCopyImageToBufferInfo2 copy_info = *pCopyImageToBufferInfo;

   bool prime_blit =
      vn_needs_prime_blit(copy_info.srcImage, copy_info.srcImageLayout);
   if (prime_blit)
      copy_info.srcImageLayout = VN_PRESENT_SRC_INTERNAL_LAYOUT;

   VN_CMD_ENQUEUE(vkCmdCopyImageToBuffer2, commandBuffer, &copy_info);

   if (prime_blit)
      vn_transition_prime_layout(cmd, copy_info.dstBuffer);
}

void
vn_CmdUpdateBuffer(VkCommandBuffer commandBuffer,
                   VkBuffer dstBuffer,
                   VkDeviceSize dstOffset,
                   VkDeviceSize dataSize,
                   const void *pData)
{
   VN_CMD_ENQUEUE(vkCmdUpdateBuffer, commandBuffer, dstBuffer, dstOffset,
                  dataSize, pData);
}

void
vn_CmdFillBuffer(VkCommandBuffer commandBuffer,
                 VkBuffer dstBuffer,
                 VkDeviceSize dstOffset,
                 VkDeviceSize size,
                 uint32_t data)
{
   VN_CMD_ENQUEUE(vkCmdFillBuffer, commandBuffer, dstBuffer, dstOffset, size,
                  data);
}

void
vn_CmdClearColorImage(VkCommandBuffer commandBuffer,
                      VkImage image,
                      VkImageLayout imageLayout,
                      const VkClearColorValue *pColor,
                      uint32_t rangeCount,
                      const VkImageSubresourceRange *pRanges)
{
   VN_CMD_ENQUEUE(vkCmdClearColorImage, commandBuffer, image, imageLayout,
                  pColor, rangeCount, pRanges);
}

void
vn_CmdClearDepthStencilImage(VkCommandBuffer commandBuffer,
                             VkImage image,
                             VkImageLayout imageLayout,
                             const VkClearDepthStencilValue *pDepthStencil,
                             uint32_t rangeCount,
                             const VkImageSubresourceRange *pRanges)
{
   VN_CMD_ENQUEUE(vkCmdClearDepthStencilImage, commandBuffer, image,
                  imageLayout, pDepthStencil, rangeCount, pRanges);
}

void
vn_CmdClearAttachments(VkCommandBuffer commandBuffer,
                       uint32_t attachmentCount,
                       const VkClearAttachment *pAttachments,
                       uint32_t rectCount,
                       const VkClearRect *pRects)
{
   VN_CMD_ENQUEUE(vkCmdClearAttachments, commandBuffer, attachmentCount,
                  pAttachments, rectCount, pRects);
}

void
vn_CmdResolveImage(VkCommandBuffer commandBuffer,
                   VkImage srcImage,
                   VkImageLayout srcImageLayout,
                   VkImage dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageResolve *pRegions)
{
   VN_CMD_ENQUEUE(vkCmdResolveImage, commandBuffer, srcImage, srcImageLayout,
                  dstImage, dstImageLayout, regionCount, pRegions);
}

void
vn_CmdResolveImage2(VkCommandBuffer commandBuffer,
                    const VkResolveImageInfo2 *pResolveImageInfo)
{
   VN_CMD_ENQUEUE(vkCmdResolveImage2, commandBuffer, pResolveImageInfo);
}

void
vn_CmdSetEvent(VkCommandBuffer commandBuffer,
               VkEvent event,
               VkPipelineStageFlags stageMask)
{
   VN_CMD_ENQUEUE(vkCmdSetEvent, commandBuffer, event, stageMask);

   vn_feedback_event_cmd_record(commandBuffer, event, stageMask, VK_EVENT_SET,
                                false);
}

static VkPipelineStageFlags2
vn_dependency_info_collect_src_stage_mask(const VkDependencyInfo *dep_info)
{
   VkPipelineStageFlags2 mask = 0;

   for (uint32_t i = 0; i < dep_info->memoryBarrierCount; i++)
      mask |= dep_info->pMemoryBarriers[i].srcStageMask;

   for (uint32_t i = 0; i < dep_info->bufferMemoryBarrierCount; i++)
      mask |= dep_info->pBufferMemoryBarriers[i].srcStageMask;

   for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; i++)
      mask |= dep_info->pImageMemoryBarriers[i].srcStageMask;

   return mask;
}

void
vn_CmdSetEvent2(VkCommandBuffer commandBuffer,
                VkEvent event,
                const VkDependencyInfo *pDependencyInfo)

{
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);

   pDependencyInfo = vn_cmd_fix_dependency_infos(cmd, 1, pDependencyInfo);

   VN_CMD_ENQUEUE(vkCmdSetEvent2, commandBuffer, event, pDependencyInfo);

   VkPipelineStageFlags2 src_stage_mask =
      vn_dependency_info_collect_src_stage_mask(pDependencyInfo);

   vn_feedback_event_cmd_record(commandBuffer, event, src_stage_mask,
                                VK_EVENT_SET, true);
}

void
vn_CmdResetEvent(VkCommandBuffer commandBuffer,
                 VkEvent event,
                 VkPipelineStageFlags stageMask)
{
   VN_CMD_ENQUEUE(vkCmdResetEvent, commandBuffer, event, stageMask);

   vn_feedback_event_cmd_record(commandBuffer, event, stageMask,
                                VK_EVENT_RESET, false);
}

void
vn_CmdResetEvent2(VkCommandBuffer commandBuffer,
                  VkEvent event,
                  VkPipelineStageFlags2 stageMask)
{
   VN_CMD_ENQUEUE(vkCmdResetEvent2, commandBuffer, event, stageMask);
   vn_feedback_event_cmd_record(commandBuffer, event, stageMask,
                                VK_EVENT_RESET, true);
}

void
vn_CmdWaitEvents(VkCommandBuffer commandBuffer,
                 uint32_t eventCount,
                 const VkEvent *pEvents,
                 VkPipelineStageFlags srcStageMask,
                 VkPipelineStageFlags dstStageMask,
                 uint32_t memoryBarrierCount,
                 const VkMemoryBarrier *pMemoryBarriers,
                 uint32_t bufferMemoryBarrierCount,
                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                 uint32_t imageMemoryBarrierCount,
                 const VkImageMemoryBarrier *pImageMemoryBarriers)
{
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);
   uint32_t transfer_count;

   pImageMemoryBarriers = vn_cmd_wait_events_fix_image_memory_barriers(
      cmd, pImageMemoryBarriers, imageMemoryBarrierCount, &transfer_count);
   imageMemoryBarrierCount -= transfer_count;

   VN_CMD_ENQUEUE(vkCmdWaitEvents, commandBuffer, eventCount, pEvents,
                  srcStageMask, dstStageMask, memoryBarrierCount,
                  pMemoryBarriers, bufferMemoryBarrierCount,
                  pBufferMemoryBarriers, imageMemoryBarrierCount,
                  pImageMemoryBarriers);

   if (transfer_count) {
      pImageMemoryBarriers += imageMemoryBarrierCount;
      vn_cmd_encode_memory_barriers(cmd, srcStageMask, dstStageMask, 0, NULL,
                                    transfer_count, pImageMemoryBarriers);
   }
}

void
vn_CmdWaitEvents2(VkCommandBuffer commandBuffer,
                  uint32_t eventCount,
                  const VkEvent *pEvents,
                  const VkDependencyInfo *pDependencyInfos)
{
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);

   pDependencyInfos =
      vn_cmd_fix_dependency_infos(cmd, eventCount, pDependencyInfos);

   VN_CMD_ENQUEUE(vkCmdWaitEvents2, commandBuffer, eventCount, pEvents,
                  pDependencyInfos);
}

void
vn_CmdPipelineBarrier(VkCommandBuffer commandBuffer,
                      VkPipelineStageFlags srcStageMask,
                      VkPipelineStageFlags dstStageMask,
                      VkDependencyFlags dependencyFlags,
                      uint32_t memoryBarrierCount,
                      const VkMemoryBarrier *pMemoryBarriers,
                      uint32_t bufferMemoryBarrierCount,
                      const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                      uint32_t imageMemoryBarrierCount,
                      const VkImageMemoryBarrier *pImageMemoryBarriers)
{
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);

   pImageMemoryBarriers = vn_cmd_pipeline_barrier_fix_image_memory_barriers(
      cmd, pImageMemoryBarriers, imageMemoryBarrierCount);

   VN_CMD_ENQUEUE(vkCmdPipelineBarrier, commandBuffer, srcStageMask,
                  dstStageMask, dependencyFlags, memoryBarrierCount,
                  pMemoryBarriers, bufferMemoryBarrierCount,
                  pBufferMemoryBarriers, imageMemoryBarrierCount,
                  pImageMemoryBarriers);
}

void
vn_CmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                       const VkDependencyInfo *pDependencyInfo)
{
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);

   pDependencyInfo = vn_cmd_fix_dependency_infos(cmd, 1, pDependencyInfo);

   VN_CMD_ENQUEUE(vkCmdPipelineBarrier2, commandBuffer, pDependencyInfo);
}

void
vn_CmdBeginQuery(VkCommandBuffer commandBuffer,
                 VkQueryPool queryPool,
                 uint32_t query,
                 VkQueryControlFlags flags)
{
   VN_CMD_ENQUEUE(vkCmdBeginQuery, commandBuffer, queryPool, query, flags);
}

static inline void
vn_cmd_add_query_feedback(VkCommandBuffer cmd_handle,
                          VkQueryPool pool_handle,
                          uint32_t query)
{
   struct vn_command_buffer *cmd = vn_command_buffer_from_handle(cmd_handle);
   struct vn_query_pool *query_pool = vn_query_pool_from_handle(pool_handle);

   if (!query_pool->feedback)
      return;

   /* Per 1.3.255 spec "If queries are used while executing a render pass
    * instance that has multiview enabled, the query uses N consecutive
    * query indices in the query pool (starting at query) where N is the
    * number of bits set in the view mask in the subpass the query is used
    * in."
    */
   uint32_t query_count =
      (cmd->builder.in_render_pass && cmd->builder.view_mask)
         ? util_bitcount(cmd->builder.view_mask)
         : 1;

   struct vn_feedback_query_batch *batch = vn_cmd_query_batch_alloc(
      cmd->pool, query_pool, query, query_count, true);
   if (!batch) {
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
      return;
   }

   list_addtail(&batch->head, &cmd->builder.query_batches);
}

static inline void
vn_cmd_add_query_reset_feedback(VkCommandBuffer cmd_handle,
                                VkQueryPool pool_handle,
                                uint32_t query,
                                uint32_t query_count)
{
   struct vn_command_buffer *cmd = vn_command_buffer_from_handle(cmd_handle);
   struct vn_query_pool *query_pool = vn_query_pool_from_handle(pool_handle);

   if (!query_pool->feedback)
      return;

   struct vn_feedback_query_batch *batch = vn_cmd_query_batch_alloc(
      cmd->pool, query_pool, query, query_count, false);
   if (!batch)
      cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;

   list_addtail(&batch->head, &cmd->builder.query_batches);
}

void
vn_CmdEndQuery(VkCommandBuffer commandBuffer,
               VkQueryPool queryPool,
               uint32_t query)
{
   VN_CMD_ENQUEUE(vkCmdEndQuery, commandBuffer, queryPool, query);

   vn_cmd_add_query_feedback(commandBuffer, queryPool, query);
}

void
vn_CmdResetQueryPool(VkCommandBuffer commandBuffer,
                     VkQueryPool queryPool,
                     uint32_t firstQuery,
                     uint32_t queryCount)
{
   VN_CMD_ENQUEUE(vkCmdResetQueryPool, commandBuffer, queryPool, firstQuery,
                  queryCount);

   vn_cmd_add_query_reset_feedback(commandBuffer, queryPool, firstQuery,
                                   queryCount);
}

void
vn_CmdWriteTimestamp(VkCommandBuffer commandBuffer,
                     VkPipelineStageFlagBits pipelineStage,
                     VkQueryPool queryPool,
                     uint32_t query)
{
   VN_CMD_ENQUEUE(vkCmdWriteTimestamp, commandBuffer, pipelineStage,
                  queryPool, query);

   vn_cmd_add_query_feedback(commandBuffer, queryPool, query);
}

void
vn_CmdWriteTimestamp2(VkCommandBuffer commandBuffer,
                      VkPipelineStageFlagBits2 stage,
                      VkQueryPool queryPool,
                      uint32_t query)
{
   VN_CMD_ENQUEUE(vkCmdWriteTimestamp2, commandBuffer, stage, queryPool,
                  query);

   vn_cmd_add_query_feedback(commandBuffer, queryPool, query);
}

void
vn_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer,
                           VkQueryPool queryPool,
                           uint32_t firstQuery,
                           uint32_t queryCount,
                           VkBuffer dstBuffer,
                           VkDeviceSize dstOffset,
                           VkDeviceSize stride,
                           VkQueryResultFlags flags)
{
   VN_CMD_ENQUEUE(vkCmdCopyQueryPoolResults, commandBuffer, queryPool,
                  firstQuery, queryCount, dstBuffer, dstOffset, stride,
                  flags);
}

void
vn_CmdPushConstants(VkCommandBuffer commandBuffer,
                    VkPipelineLayout layout,
                    VkShaderStageFlags stageFlags,
                    uint32_t offset,
                    uint32_t size,
                    const void *pValues)
{
   VN_CMD_ENQUEUE(vkCmdPushConstants, commandBuffer, layout, stageFlags,
                  offset, size, pValues);
}

void
vn_CmdBeginRenderPass(VkCommandBuffer commandBuffer,
                      const VkRenderPassBeginInfo *pRenderPassBegin,
                      VkSubpassContents contents)
{
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);

   vn_cmd_begin_render_pass(
      cmd, vn_render_pass_from_handle(pRenderPassBegin->renderPass),
      vn_framebuffer_from_handle(pRenderPassBegin->framebuffer),
      pRenderPassBegin);

   VN_CMD_ENQUEUE(vkCmdBeginRenderPass, commandBuffer, pRenderPassBegin,
                  contents);
}

void
vn_CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
   vn_cmd_next_subpass(vn_command_buffer_from_handle(commandBuffer));

   VN_CMD_ENQUEUE(vkCmdNextSubpass, commandBuffer, contents);
}

void
vn_CmdEndRenderPass(VkCommandBuffer commandBuffer)
{
   VN_CMD_ENQUEUE(vkCmdEndRenderPass, commandBuffer);

   vn_cmd_end_render_pass(vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                       const VkRenderPassBeginInfo *pRenderPassBegin,
                       const VkSubpassBeginInfo *pSubpassBeginInfo)
{
   struct vn_command_buffer *cmd =
      vn_command_buffer_from_handle(commandBuffer);

   vn_cmd_begin_render_pass(
      cmd, vn_render_pass_from_handle(pRenderPassBegin->renderPass),
      vn_framebuffer_from_handle(pRenderPassBegin->framebuffer),
      pRenderPassBegin);

   VN_CMD_ENQUEUE(vkCmdBeginRenderPass2, commandBuffer, pRenderPassBegin,
                  pSubpassBeginInfo);
}

void
vn_CmdNextSubpass2(VkCommandBuffer commandBuffer,
                   const VkSubpassBeginInfo *pSubpassBeginInfo,
                   const VkSubpassEndInfo *pSubpassEndInfo)
{
   vn_cmd_next_subpass(vn_command_buffer_from_handle(commandBuffer));

   VN_CMD_ENQUEUE(vkCmdNextSubpass2, commandBuffer, pSubpassBeginInfo,
                  pSubpassEndInfo);
}

void
vn_CmdEndRenderPass2(VkCommandBuffer commandBuffer,
                     const VkSubpassEndInfo *pSubpassEndInfo)
{
   VN_CMD_ENQUEUE(vkCmdEndRenderPass2, commandBuffer, pSubpassEndInfo);

   vn_cmd_end_render_pass(vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdExecuteCommands(VkCommandBuffer commandBuffer,
                      uint32_t commandBufferCount,
                      const VkCommandBuffer *pCommandBuffers)
{
   VN_CMD_ENQUEUE(vkCmdExecuteCommands, commandBuffer, commandBufferCount,
                  pCommandBuffers);

   struct vn_command_buffer *primary_cmd =
      vn_command_buffer_from_handle(commandBuffer);
   for (uint32_t i = 0; i < commandBufferCount; i++) {
      struct vn_command_buffer *secondary_cmd =
         vn_command_buffer_from_handle(pCommandBuffers[i]);
      vn_cmd_merge_batched_query_feedback(primary_cmd, secondary_cmd);
   }
}

void
vn_CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
   VN_CMD_ENQUEUE(vkCmdSetDeviceMask, commandBuffer, deviceMask);
}

void
vn_CmdDispatchBase(VkCommandBuffer commandBuffer,
                   uint32_t baseGroupX,
                   uint32_t baseGroupY,
                   uint32_t baseGroupZ,
                   uint32_t groupCountX,
                   uint32_t groupCountY,
                   uint32_t groupCountZ)
{
   VN_CMD_ENQUEUE(vkCmdDispatchBase, commandBuffer, baseGroupX, baseGroupY,
                  baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

void
vn_CmdSetLineStippleEXT(VkCommandBuffer commandBuffer,
                        uint32_t lineStippleFactor,
                        uint16_t lineStipplePattern)
{
   VN_CMD_ENQUEUE(vkCmdSetLineStippleEXT, commandBuffer, lineStippleFactor,
                  lineStipplePattern);
}

void
vn_CmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer,
                           VkQueryPool queryPool,
                           uint32_t query,
                           VkQueryControlFlags flags,
                           uint32_t index)
{
   VN_CMD_ENQUEUE(vkCmdBeginQueryIndexedEXT, commandBuffer, queryPool, query,
                  flags, index);
}

void
vn_CmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer,
                         VkQueryPool queryPool,
                         uint32_t query,
                         uint32_t index)
{
   VN_CMD_ENQUEUE(vkCmdEndQueryIndexedEXT, commandBuffer, queryPool, query,
                  index);

   vn_cmd_add_query_feedback(commandBuffer, queryPool, query);
}

void
vn_CmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer,
                                      uint32_t firstBinding,
                                      uint32_t bindingCount,
                                      const VkBuffer *pBuffers,
                                      const VkDeviceSize *pOffsets,
                                      const VkDeviceSize *pSizes)
{
   VN_CMD_ENQUEUE(vkCmdBindTransformFeedbackBuffersEXT, commandBuffer,
                  firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}

void
vn_CmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                                uint32_t firstCounterBuffer,
                                uint32_t counterBufferCount,
                                const VkBuffer *pCounterBuffers,
                                const VkDeviceSize *pCounterBufferOffsets)
{
   VN_CMD_ENQUEUE(vkCmdBeginTransformFeedbackEXT, commandBuffer,
                  firstCounterBuffer, counterBufferCount, pCounterBuffers,
                  pCounterBufferOffsets);
}

void
vn_CmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                              uint32_t firstCounterBuffer,
                              uint32_t counterBufferCount,
                              const VkBuffer *pCounterBuffers,
                              const VkDeviceSize *pCounterBufferOffsets)
{
   VN_CMD_ENQUEUE(vkCmdEndTransformFeedbackEXT, commandBuffer,
                  firstCounterBuffer, counterBufferCount, pCounterBuffers,
                  pCounterBufferOffsets);
}

void
vn_CmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer,
                               uint32_t instanceCount,
                               uint32_t firstInstance,
                               VkBuffer counterBuffer,
                               VkDeviceSize counterBufferOffset,
                               uint32_t counterOffset,
                               uint32_t vertexStride)
{
   VN_CMD_ENQUEUE(vkCmdDrawIndirectByteCountEXT, commandBuffer, instanceCount,
                  firstInstance, counterBuffer, counterBufferOffset,
                  counterOffset, vertexStride);

   vn_cmd_count_draw_and_submit_on_batch_limit(
      vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdBindVertexBuffers2(VkCommandBuffer commandBuffer,
                         uint32_t firstBinding,
                         uint32_t bindingCount,
                         const VkBuffer *pBuffers,
                         const VkDeviceSize *pOffsets,
                         const VkDeviceSize *pSizes,
                         const VkDeviceSize *pStrides)
{
   VN_CMD_ENQUEUE(vkCmdBindVertexBuffers2, commandBuffer, firstBinding,
                  bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

void
vn_CmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode)
{
   VN_CMD_ENQUEUE(vkCmdSetCullMode, commandBuffer, cullMode);
}

void
vn_CmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer,
                               VkBool32 depthBoundsTestEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthBoundsTestEnable, commandBuffer,
                  depthBoundsTestEnable);
}

void
vn_CmdSetDepthCompareOp(VkCommandBuffer commandBuffer,
                        VkCompareOp depthCompareOp)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthCompareOp, commandBuffer, depthCompareOp);
}

void
vn_CmdSetDepthTestEnable(VkCommandBuffer commandBuffer,
                         VkBool32 depthTestEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthTestEnable, commandBuffer, depthTestEnable);
}

void
vn_CmdSetDepthWriteEnable(VkCommandBuffer commandBuffer,
                          VkBool32 depthWriteEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthWriteEnable, commandBuffer, depthWriteEnable);
}

void
vn_CmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace)
{
   VN_CMD_ENQUEUE(vkCmdSetFrontFace, commandBuffer, frontFace);
}

void
vn_CmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                           VkPrimitiveTopology primitiveTopology)
{
   VN_CMD_ENQUEUE(vkCmdSetPrimitiveTopology, commandBuffer,
                  primitiveTopology);
}

void
vn_CmdSetScissorWithCount(VkCommandBuffer commandBuffer,
                          uint32_t scissorCount,
                          const VkRect2D *pScissors)
{
   VN_CMD_ENQUEUE(vkCmdSetScissorWithCount, commandBuffer, scissorCount,
                  pScissors);
}

void
vn_CmdSetStencilOp(VkCommandBuffer commandBuffer,
                   VkStencilFaceFlags faceMask,
                   VkStencilOp failOp,
                   VkStencilOp passOp,
                   VkStencilOp depthFailOp,
                   VkCompareOp compareOp)
{
   VN_CMD_ENQUEUE(vkCmdSetStencilOp, commandBuffer, faceMask, failOp, passOp,
                  depthFailOp, compareOp);
}

void
vn_CmdSetStencilTestEnable(VkCommandBuffer commandBuffer,
                           VkBool32 stencilTestEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetStencilTestEnable, commandBuffer,
                  stencilTestEnable);
}

void
vn_CmdSetViewportWithCount(VkCommandBuffer commandBuffer,
                           uint32_t viewportCount,
                           const VkViewport *pViewports)
{
   VN_CMD_ENQUEUE(vkCmdSetViewportWithCount, commandBuffer, viewportCount,
                  pViewports);
}

void
vn_CmdSetDepthBiasEnable(VkCommandBuffer commandBuffer,
                         VkBool32 depthBiasEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthBiasEnable, commandBuffer, depthBiasEnable);
}

void
vn_CmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp)
{
   VN_CMD_ENQUEUE(vkCmdSetLogicOpEXT, commandBuffer, logicOp);
}

void
vn_CmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer,
                             uint32_t attachmentCount,
                             const VkBool32 *pColorWriteEnables)
{
   VN_CMD_ENQUEUE(vkCmdSetColorWriteEnableEXT, commandBuffer, attachmentCount,
                  pColorWriteEnables);
}

void
vn_CmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer,
                               uint32_t patchControlPoints)
{
   VN_CMD_ENQUEUE(vkCmdSetPatchControlPointsEXT, commandBuffer,
                  patchControlPoints);
}

void
vn_CmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer,
                                VkBool32 primitiveRestartEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetPrimitiveRestartEnable, commandBuffer,
                  primitiveRestartEnable);
}

void
vn_CmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                                 VkBool32 rasterizerDiscardEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetRasterizerDiscardEnable, commandBuffer,
                  rasterizerDiscardEnable);
}

void
vn_CmdBeginConditionalRenderingEXT(
   VkCommandBuffer commandBuffer,
   const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin)
{
   VN_CMD_ENQUEUE(vkCmdBeginConditionalRenderingEXT, commandBuffer,
                  pConditionalRenderingBegin);
}

void
vn_CmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer)
{
   VN_CMD_ENQUEUE(vkCmdEndConditionalRenderingEXT, commandBuffer);
}

void
vn_CmdDrawMultiEXT(VkCommandBuffer commandBuffer,
                   uint32_t drawCount,
                   const VkMultiDrawInfoEXT *pVertexInfo,
                   uint32_t instanceCount,
                   uint32_t firstInstance,
                   uint32_t stride)
{
   VN_CMD_ENQUEUE(vkCmdDrawMultiEXT, commandBuffer, drawCount, pVertexInfo,
                  instanceCount, firstInstance, stride);

   vn_cmd_count_draw_and_submit_on_batch_limit(
      vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer,
                          uint32_t drawCount,
                          const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                          uint32_t instanceCount,
                          uint32_t firstInstance,
                          uint32_t stride,
                          const int32_t *pVertexOffset)
{
   VN_CMD_ENQUEUE(vkCmdDrawMultiIndexedEXT, commandBuffer, drawCount,
                  pIndexInfo, instanceCount, firstInstance, stride,
                  pVertexOffset);

   vn_cmd_count_draw_and_submit_on_batch_limit(
      vn_command_buffer_from_handle(commandBuffer));
}

void
vn_CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                           VkPipelineBindPoint pipelineBindPoint,
                           VkPipelineLayout layout,
                           uint32_t set,
                           uint32_t descriptorWriteCount,
                           const VkWriteDescriptorSet *pDescriptorWrites)
{
   if (vn_should_sanitize_descriptor_set_writes(descriptorWriteCount,
                                                pDescriptorWrites, layout)) {
      struct vn_command_buffer *cmd =
         vn_command_buffer_from_handle(commandBuffer);
      struct vn_update_descriptor_sets *update =
         vn_update_descriptor_sets_parse_writes(
            descriptorWriteCount, pDescriptorWrites, &cmd->pool->allocator,
            layout);
      if (!update) {
         cmd->state = VN_COMMAND_BUFFER_STATE_INVALID;
         return;
      }

      VN_CMD_ENQUEUE(vkCmdPushDescriptorSetKHR, commandBuffer,
                     pipelineBindPoint, layout, set, update->write_count,
                     update->writes);

      vk_free(&cmd->pool->allocator, update);
   } else {
      VN_CMD_ENQUEUE(vkCmdPushDescriptorSetKHR, commandBuffer,
                     pipelineBindPoint, layout, set, descriptorWriteCount,
                     pDescriptorWrites);
   }
}

void
vn_CmdPushDescriptorSetWithTemplateKHR(
   VkCommandBuffer commandBuffer,
   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
   VkPipelineLayout layout,
   uint32_t set,
   const void *pData)
{
   struct vn_descriptor_update_template *templ =
      vn_descriptor_update_template_from_handle(descriptorUpdateTemplate);

   mtx_lock(&templ->mutex);

   struct vn_update_descriptor_sets *update =
      vn_update_descriptor_set_with_template_locked(templ, VK_NULL_HANDLE,
                                                    pData);
   VN_CMD_ENQUEUE(vkCmdPushDescriptorSetKHR, commandBuffer,
                  templ->pipeline_bind_point, layout, set,
                  update->write_count, update->writes);

   mtx_unlock(&templ->mutex);
}

void
vn_CmdSetVertexInputEXT(
   VkCommandBuffer commandBuffer,
   uint32_t vertexBindingDescriptionCount,
   const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions,
   uint32_t vertexAttributeDescriptionCount,
   const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions)
{
   VN_CMD_ENQUEUE(vkCmdSetVertexInputEXT, commandBuffer,
                  vertexBindingDescriptionCount, pVertexBindingDescriptions,
                  vertexAttributeDescriptionCount,
                  pVertexAttributeDescriptions);
}

void
vn_CmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
                                  VkBool32 alphaToCoverageEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetAlphaToCoverageEnableEXT, commandBuffer,
                  alphaToCoverageEnable);
}

void
vn_CmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer,
                             VkBool32 alphaToOneEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetAlphaToOneEnableEXT, commandBuffer,
                  alphaToOneEnable);
}

void
vn_CmdSetColorBlendAdvancedEXT(
   VkCommandBuffer commandBuffer,
   uint32_t firstAttachment,
   uint32_t attachmentCount,
   const VkColorBlendAdvancedEXT *pColorBlendAdvanced)
{
   VN_CMD_ENQUEUE(vkCmdSetColorBlendAdvancedEXT, commandBuffer,
                  firstAttachment, attachmentCount, pColorBlendAdvanced);
}

void
vn_CmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer,
                             uint32_t firstAttachment,
                             uint32_t attachmentCount,
                             const VkBool32 *pColorBlendEnables)
{
   VN_CMD_ENQUEUE(vkCmdSetColorBlendEnableEXT, commandBuffer, firstAttachment,
                  attachmentCount, pColorBlendEnables);
}

void
vn_CmdSetColorBlendEquationEXT(
   VkCommandBuffer commandBuffer,
   uint32_t firstAttachment,
   uint32_t attachmentCount,
   const VkColorBlendEquationEXT *pColorBlendEquations)
{
   VN_CMD_ENQUEUE(vkCmdSetColorBlendEquationEXT, commandBuffer,
                  firstAttachment, attachmentCount, pColorBlendEquations);
}

void
vn_CmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer,
                           uint32_t firstAttachment,
                           uint32_t attachmentCount,
                           const VkColorComponentFlags *pColorWriteMasks)
{
   VN_CMD_ENQUEUE(vkCmdSetColorWriteMaskEXT, commandBuffer, firstAttachment,
                  attachmentCount, pColorWriteMasks);
}

void
vn_CmdSetConservativeRasterizationModeEXT(
   VkCommandBuffer commandBuffer,
   VkConservativeRasterizationModeEXT conservativeRasterizationMode)
{
   VN_CMD_ENQUEUE(vkCmdSetConservativeRasterizationModeEXT, commandBuffer,
                  conservativeRasterizationMode);
}

void
vn_CmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer,
                             VkBool32 depthClampEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthClampEnableEXT, commandBuffer,
                  depthClampEnable);
}

void
vn_CmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer,
                            VkBool32 depthClipEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthClipEnableEXT, commandBuffer, depthClipEnable);
}

void
vn_CmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer,
                                      VkBool32 negativeOneToOne)
{
   VN_CMD_ENQUEUE(vkCmdSetDepthClipNegativeOneToOneEXT, commandBuffer,
                  negativeOneToOne);
}

void
vn_CmdSetExtraPrimitiveOverestimationSizeEXT(
   VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize)
{
   VN_CMD_ENQUEUE(vkCmdSetExtraPrimitiveOverestimationSizeEXT, commandBuffer,
                  extraPrimitiveOverestimationSize);
}

void
vn_CmdSetLineRasterizationModeEXT(
   VkCommandBuffer commandBuffer,
   VkLineRasterizationModeEXT lineRasterizationMode)
{
   VN_CMD_ENQUEUE(vkCmdSetLineRasterizationModeEXT, commandBuffer,
                  lineRasterizationMode);
}

void
vn_CmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer,
                              VkBool32 stippledLineEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetLineStippleEnableEXT, commandBuffer,
                  stippledLineEnable);
}

void
vn_CmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer,
                          VkBool32 logicOpEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetLogicOpEnableEXT, commandBuffer, logicOpEnable);
}

void
vn_CmdSetPolygonModeEXT(VkCommandBuffer commandBuffer,
                        VkPolygonMode polygonMode)
{
   VN_CMD_ENQUEUE(vkCmdSetPolygonModeEXT, commandBuffer, polygonMode);
}

void
vn_CmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                VkProvokingVertexModeEXT provokingVertexMode)
{
   VN_CMD_ENQUEUE(vkCmdSetProvokingVertexModeEXT, commandBuffer,
                  provokingVertexMode);
}

void
vn_CmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                 VkSampleCountFlagBits rasterizationSamples)
{
   VN_CMD_ENQUEUE(vkCmdSetRasterizationSamplesEXT, commandBuffer,
                  rasterizationSamples);
}

void
vn_CmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer,
                                uint32_t rasterizationStream)
{
   VN_CMD_ENQUEUE(vkCmdSetRasterizationStreamEXT, commandBuffer,
                  rasterizationStream);
}

void
vn_CmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
                                  VkBool32 sampleLocationsEnable)
{
   VN_CMD_ENQUEUE(vkCmdSetSampleLocationsEnableEXT, commandBuffer,
                  sampleLocationsEnable);
}

void
vn_CmdSetSampleMaskEXT(VkCommandBuffer commandBuffer,
                       VkSampleCountFlagBits samples,
                       const VkSampleMask *pSampleMask)
{
   VN_CMD_ENQUEUE(vkCmdSetSampleMaskEXT, commandBuffer, samples, pSampleMask);
}

void
vn_CmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                     VkTessellationDomainOrigin domainOrigin)
{
   VN_CMD_ENQUEUE(vkCmdSetTessellationDomainOriginEXT, commandBuffer,
                  domainOrigin);
}
