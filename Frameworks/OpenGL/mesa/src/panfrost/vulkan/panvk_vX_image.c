/*
 * Copyright © 2021 Collabora Ltd.
 *
 * Derived from tu_image.c which is:
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * Copyright © 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "genxml/gen_macros.h"
#include "panvk_private.h"

#include "drm-uapi/drm_fourcc.h"
#include "util/u_atomic.h"
#include "util/u_debug.h"
#include "vk_format.h"
#include "vk_object.h"
#include "vk_util.h"

static enum mali_texture_dimension
panvk_view_type_to_mali_tex_dim(VkImageViewType type)
{
   switch (type) {
   case VK_IMAGE_VIEW_TYPE_1D:
   case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
      return MALI_TEXTURE_DIMENSION_1D;
   case VK_IMAGE_VIEW_TYPE_2D:
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
      return MALI_TEXTURE_DIMENSION_2D;
   case VK_IMAGE_VIEW_TYPE_3D:
      return MALI_TEXTURE_DIMENSION_3D;
   case VK_IMAGE_VIEW_TYPE_CUBE:
   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
      return MALI_TEXTURE_DIMENSION_CUBE;
   default:
      unreachable("Invalid view type");
   }
}

static void
panvk_convert_swizzle(const VkComponentMapping *in, unsigned char *out)
{
   const VkComponentSwizzle *comp = &in->r;
   for (unsigned i = 0; i < 4; i++) {
      switch (comp[i]) {
      case VK_COMPONENT_SWIZZLE_ZERO:
         out[i] = PIPE_SWIZZLE_0;
         break;
      case VK_COMPONENT_SWIZZLE_ONE:
         out[i] = PIPE_SWIZZLE_1;
         break;
      case VK_COMPONENT_SWIZZLE_R:
         out[i] = PIPE_SWIZZLE_X;
         break;
      case VK_COMPONENT_SWIZZLE_G:
         out[i] = PIPE_SWIZZLE_Y;
         break;
      case VK_COMPONENT_SWIZZLE_B:
         out[i] = PIPE_SWIZZLE_Z;
         break;
      case VK_COMPONENT_SWIZZLE_A:
         out[i] = PIPE_SWIZZLE_W;
         break;
      default:
         unreachable("Invalid swizzle");
      }
   }
}

VkResult
panvk_per_arch(CreateImageView)(VkDevice _device,
                                const VkImageViewCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator,
                                VkImageView *pView)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_image, image, pCreateInfo->image);
   struct panvk_image_view *view;

   view = vk_image_view_create(&device->vk, false, pCreateInfo, pAllocator,
                               sizeof(*view));
   if (view == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   view->pview = (struct pan_image_view){
      .planes[0] = &image->pimage,
      .format = vk_format_to_pipe_format(view->vk.view_format),
      .dim = panvk_view_type_to_mali_tex_dim(view->vk.view_type),
      .nr_samples = image->pimage.layout.nr_samples,
      .first_level = view->vk.base_mip_level,
      .last_level = view->vk.base_mip_level + view->vk.level_count - 1,
      .first_layer = view->vk.base_array_layer,
      .last_layer = view->vk.base_array_layer + view->vk.layer_count - 1,
   };
   panvk_convert_swizzle(&view->vk.swizzle, view->pview.swizzle);

   struct panfrost_device *pdev = &device->physical_device->pdev;

   if (view->vk.usage &
       (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
      unsigned bo_size =
         GENX(panfrost_estimate_texture_payload_size)(&view->pview) +
         pan_size(TEXTURE);

      view->bo = panfrost_bo_create(pdev, bo_size, 0, "Texture descriptor");

      STATIC_ASSERT(sizeof(view->descs.tex) >= pan_size(TEXTURE));
      GENX(panfrost_new_texture)
      (pdev, &view->pview, &view->descs.tex, &view->bo->ptr);
   }

   if (view->vk.usage & VK_IMAGE_USAGE_STORAGE_BIT) {
      uint8_t *attrib_buf = (uint8_t *)view->descs.img_attrib_buf;
      bool is_3d = image->pimage.layout.dim == MALI_TEXTURE_DIMENSION_3D;
      unsigned offset = image->pimage.data.offset;
      offset +=
         panfrost_texture_offset(&image->pimage.layout, view->pview.first_level,
                                 is_3d ? 0 : view->pview.first_layer,
                                 is_3d ? view->pview.first_layer : 0);

      pan_pack(attrib_buf, ATTRIBUTE_BUFFER, cfg) {
         cfg.type = image->pimage.layout.modifier == DRM_FORMAT_MOD_LINEAR
                       ? MALI_ATTRIBUTE_TYPE_3D_LINEAR
                       : MALI_ATTRIBUTE_TYPE_3D_INTERLEAVED;
         cfg.pointer = image->pimage.data.bo->ptr.gpu + offset;
         cfg.stride = util_format_get_blocksize(view->pview.format);
         cfg.size = panfrost_bo_size(image->pimage.data.bo) - offset;
      }

      attrib_buf += pan_size(ATTRIBUTE_BUFFER);
      pan_pack(attrib_buf, ATTRIBUTE_BUFFER_CONTINUATION_3D, cfg) {
         unsigned level = view->pview.first_level;

         cfg.s_dimension = u_minify(image->pimage.layout.width, level);
         cfg.t_dimension = u_minify(image->pimage.layout.height, level);
         cfg.r_dimension =
            view->pview.dim == MALI_TEXTURE_DIMENSION_3D
               ? u_minify(image->pimage.layout.depth, level)
               : (view->pview.last_layer - view->pview.first_layer + 1);
         cfg.row_stride = image->pimage.layout.slices[level].row_stride;
         if (cfg.r_dimension > 1) {
            cfg.slice_stride =
               panfrost_get_layer_stride(&image->pimage.layout, level);
         }
      }
   }

   *pView = panvk_image_view_to_handle(view);
   return VK_SUCCESS;
}

VkResult
panvk_per_arch(CreateBufferView)(VkDevice _device,
                                 const VkBufferViewCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator,
                                 VkBufferView *pView)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_buffer, buffer, pCreateInfo->buffer);

   struct panvk_buffer_view *view = vk_object_zalloc(
      &device->vk, pAllocator, sizeof(*view), VK_OBJECT_TYPE_BUFFER_VIEW);

   if (!view)
      return vk_error(device->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   view->fmt = vk_format_to_pipe_format(pCreateInfo->format);

   struct panfrost_device *pdev = &device->physical_device->pdev;
   mali_ptr address = panvk_buffer_gpu_ptr(buffer, pCreateInfo->offset);
   unsigned size =
      panvk_buffer_range(buffer, pCreateInfo->offset, pCreateInfo->range);
   unsigned blksz = util_format_get_blocksize(view->fmt);
   view->elems = size / blksz;

   assert(!(address & 63));

   if (buffer->vk.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
      unsigned bo_size = pan_size(SURFACE_WITH_STRIDE);
      view->bo = panfrost_bo_create(pdev, bo_size, 0, "Texture descriptor");

      pan_pack(view->bo->ptr.cpu, SURFACE_WITH_STRIDE, cfg) {
         cfg.pointer = address;
      }

      pan_pack(view->descs.tex, TEXTURE, cfg) {
         cfg.dimension = MALI_TEXTURE_DIMENSION_1D;
         cfg.format = pdev->formats[view->fmt].hw;
         cfg.width = view->elems;
         cfg.depth = cfg.height = 1;
         cfg.swizzle = PAN_V6_SWIZZLE(R, G, B, A);
         cfg.texel_ordering = MALI_TEXTURE_LAYOUT_LINEAR;
         cfg.levels = 1;
         cfg.array_size = 1;
         cfg.surfaces = view->bo->ptr.gpu;
         cfg.maximum_lod = cfg.minimum_lod = 0;
      }
   }

   if (buffer->vk.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
      uint8_t *attrib_buf = (uint8_t *)view->descs.img_attrib_buf;

      pan_pack(attrib_buf, ATTRIBUTE_BUFFER, cfg) {
         cfg.type = MALI_ATTRIBUTE_TYPE_3D_LINEAR;
         cfg.pointer = address;
         cfg.stride = blksz;
         cfg.size = view->elems * blksz;
      }

      attrib_buf += pan_size(ATTRIBUTE_BUFFER);
      pan_pack(attrib_buf, ATTRIBUTE_BUFFER_CONTINUATION_3D, cfg) {
         cfg.s_dimension = view->elems;
         cfg.t_dimension = 1;
         cfg.r_dimension = 1;
         cfg.row_stride = view->elems * blksz;
      }
   }

   *pView = panvk_buffer_view_to_handle(view);
   return VK_SUCCESS;
}
