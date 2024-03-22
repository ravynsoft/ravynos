/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_RENDER_PASS_H
#define VN_RENDER_PASS_H

#include "vn_common.h"

struct vn_present_src_attachment {
   uint32_t index;

   VkPipelineStageFlags src_stage_mask;
   VkAccessFlags src_access_mask;

   VkPipelineStageFlags dst_stage_mask;
   VkAccessFlags dst_access_mask;
};

struct vn_subpass {
   VkImageAspectFlags attachment_aspects;
   uint32_t view_mask;
};

struct vn_render_pass {
   struct vn_object_base base;

   VkExtent2D granularity;

   uint32_t present_count;
   uint32_t present_acquire_count;
   uint32_t present_release_count;
   uint32_t subpass_count;

   /* Attachments where initialLayout or finalLayout was
    * VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
    */
   struct vn_present_src_attachment *present_attachments;

   /* Slice of present_attachments where initialLayout was
    * VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
    */
   struct vn_present_src_attachment *present_acquire_attachments;

   /* Slice of present_attachments where finalLayout was
    * VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
    */
   struct vn_present_src_attachment *present_release_attachments;

   struct vn_subpass *subpasses;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(vn_render_pass,
                               base.base,
                               VkRenderPass,
                               VK_OBJECT_TYPE_RENDER_PASS)

struct vn_framebuffer {
   struct vn_object_base base;

   uint32_t image_view_count;
   VkImageView image_views[];
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_framebuffer,
                               base.base,
                               VkFramebuffer,
                               VK_OBJECT_TYPE_FRAMEBUFFER)

static inline uint32_t
vn_render_pass_get_subpass_view_mask(const struct vn_render_pass *render_pass,
                                     uint32_t subpass_index)
{
   assert(subpass_index < render_pass->subpass_count);
   return render_pass->subpasses[subpass_index].view_mask;
}

#endif /* VN_RENDER_PASS_H */
