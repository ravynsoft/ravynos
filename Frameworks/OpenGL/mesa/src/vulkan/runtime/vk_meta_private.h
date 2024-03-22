/*
 * Copyright © 2022 Collabora Ltd
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
#ifndef VK_META_PRIVATE_H
#define VK_META_PRIVATE_H

#include "vk_image.h"
#include "vk_meta.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const VkPipelineVertexInputStateCreateInfo vk_meta_draw_rects_vi_state;
extern const VkPipelineInputAssemblyStateCreateInfo vk_meta_draw_rects_ia_state;
extern const VkPipelineViewportStateCreateInfo vk_meta_draw_rects_vs_state;

struct nir_shader *
vk_meta_draw_rects_vs_nir(struct vk_meta_device *device, bool use_gs);

struct nir_shader *
vk_meta_draw_rects_gs_nir(struct vk_meta_device *device);

static inline void
vk_meta_rendering_info_copy(struct vk_meta_rendering_info *dst,
                            const struct vk_meta_rendering_info *src)
{
   dst->view_mask = src->view_mask;
   dst->samples = src->samples;
   dst->color_attachment_count = src->color_attachment_count;
   for (uint32_t a = 0; a < src->color_attachment_count; a++)
      dst->color_attachment_formats[a] = src->color_attachment_formats[a];
   dst->depth_attachment_format = src->depth_attachment_format;
   dst->stencil_attachment_format = src->stencil_attachment_format;
}

static inline VkImageViewType
vk_image_sampled_view_type(const struct vk_image *image)
{
   switch (image->image_type) {
   case VK_IMAGE_TYPE_1D: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
   case VK_IMAGE_TYPE_2D: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
   case VK_IMAGE_TYPE_3D: return VK_IMAGE_VIEW_TYPE_3D;
   default: unreachable("Invalid image type");
   }
}

static inline VkImageViewType
vk_image_render_view_type(const struct vk_image *image, uint32_t layer_count)
{
   switch (image->image_type) {
   case VK_IMAGE_TYPE_1D:
      return layer_count == 1 ? VK_IMAGE_VIEW_TYPE_1D :
                                VK_IMAGE_VIEW_TYPE_1D_ARRAY;
   case VK_IMAGE_TYPE_2D:
   case VK_IMAGE_TYPE_3D:
      return layer_count == 1 ? VK_IMAGE_VIEW_TYPE_2D :
                                VK_IMAGE_VIEW_TYPE_2D_ARRAY;
   default:
      unreachable("Invalid image type");
   }
}

#ifdef __cplusplus
}
#endif

#endif /* VK_META_PRIVATE_H */
