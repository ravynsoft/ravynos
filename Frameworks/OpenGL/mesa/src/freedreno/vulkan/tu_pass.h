/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_PASS_H
#define TU_PASS_H

#include "tu_common.h"

enum tu_gmem_layout
{
   /* Use all of GMEM for attachments */
   TU_GMEM_LAYOUT_FULL,
   /* Avoid using the region of GMEM that the CCU needs */
   TU_GMEM_LAYOUT_AVOID_CCU,
   /* Number of layouts we have, also the value set when we don't know the layout in a secondary. */
   TU_GMEM_LAYOUT_COUNT,
};

struct tu_subpass_barrier {
   VkPipelineStageFlags2 src_stage_mask;
   VkPipelineStageFlags2 dst_stage_mask;
   VkAccessFlags2 src_access_mask;
   VkAccessFlags2 dst_access_mask;
   bool incoherent_ccu_color, incoherent_ccu_depth;
};

struct tu_subpass_attachment
{
   uint32_t attachment;

   /* For input attachments, true if it needs to be patched to refer to GMEM
    * in GMEM mode. This is false if it hasn't already been written as an
    * attachment.
    */
   bool patch_input_gmem;
};

struct tu_subpass
{
   uint32_t input_count;
   uint32_t color_count;
   uint32_t resolve_count;
   bool resolve_depth_stencil;

   bool feedback_loop_color;
   bool feedback_loop_ds;

   /* True if we must invalidate UCHE thanks to a feedback loop. */
   bool feedback_invalidate;

   /* In other words - framebuffer fetch support */
   bool raster_order_attachment_access;

   struct tu_subpass_attachment *input_attachments;
   struct tu_subpass_attachment *color_attachments;
   struct tu_subpass_attachment *resolve_attachments;
   struct tu_subpass_attachment depth_stencil_attachment;

   VkSampleCountFlagBits samples;

   uint32_t srgb_cntl;
   uint32_t multiview_mask;

   struct tu_subpass_barrier start_barrier;
};

struct tu_render_pass_attachment
{
   VkFormat format;
   VkSampleCountFlagBits samples;
   uint32_t cpp;
   VkImageAspectFlags clear_mask;
   uint32_t clear_views;
   bool load;
   bool store;
   bool gmem;
   int32_t gmem_offset[TU_GMEM_LAYOUT_COUNT];
   bool will_be_resolved;
   /* for D32S8 separate stencil: */
   bool load_stencil;
   bool store_stencil;

   bool cond_load_allowed;
   bool cond_store_allowed;

   int32_t gmem_offset_stencil[TU_GMEM_LAYOUT_COUNT];

   /* The subpass id in which the attachment will be used first/last. */
   uint32_t first_subpass_idx;
   uint32_t last_subpass_idx;
};

struct tu_render_pass
{
   struct vk_object_base base;

   uint32_t attachment_count;
   uint32_t subpass_count;
   uint32_t gmem_pixels[TU_GMEM_LAYOUT_COUNT];
   uint32_t tile_align_w;
   uint32_t min_cpp;
   uint64_t autotune_hash;

   /* memory bandwidth costs (in bytes) for gmem / sysmem rendering */
   uint32_t gmem_bandwidth_per_pixel;
   uint32_t sysmem_bandwidth_per_pixel;

   unsigned num_views;

   struct tu_subpass_attachment fragment_density_map;

   struct tu_subpass_attachment *subpass_attachments;

   struct tu_render_pass_attachment *attachments;
   bool has_cond_load_store;
   bool has_fdm;

   struct tu_subpass_barrier end_barrier;
   struct tu_subpass subpasses[0];
};

VK_DEFINE_NONDISP_HANDLE_CASTS(tu_render_pass, base, VkRenderPass,
                               VK_OBJECT_TYPE_RENDER_PASS)

void tu_setup_dynamic_render_pass(struct tu_cmd_buffer *cmd_buffer,
                                  const VkRenderingInfo *pRenderingInfo);

void tu_setup_dynamic_inheritance(struct tu_cmd_buffer *cmd_buffer,
                                  const VkCommandBufferInheritanceRenderingInfo *info);

uint32_t
tu_subpass_get_attachment_to_resolve(const struct tu_subpass *subpass, uint32_t index);

#endif /* TU_PASS_H */
