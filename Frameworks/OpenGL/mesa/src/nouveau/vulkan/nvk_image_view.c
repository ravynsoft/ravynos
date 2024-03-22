/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_image_view.h"

#include "nvk_device.h"
#include "nvk_entrypoints.h"
#include "nvk_format.h"
#include "nvk_image.h"
#include "nvk_physical_device.h"

#include "vk_format.h"

static enum nil_view_type
vk_image_view_type_to_nil_view_type(VkImageViewType view_type)
{
   switch (view_type) {
   case VK_IMAGE_VIEW_TYPE_1D:         return NIL_VIEW_TYPE_1D;
   case VK_IMAGE_VIEW_TYPE_2D:         return NIL_VIEW_TYPE_2D;
   case VK_IMAGE_VIEW_TYPE_3D:         return NIL_VIEW_TYPE_3D;
   case VK_IMAGE_VIEW_TYPE_CUBE:       return NIL_VIEW_TYPE_CUBE;
   case VK_IMAGE_VIEW_TYPE_1D_ARRAY:   return NIL_VIEW_TYPE_1D_ARRAY;
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY:   return NIL_VIEW_TYPE_2D_ARRAY;
   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY: return NIL_VIEW_TYPE_CUBE_ARRAY;
   default:
      unreachable("Invalid image view type");
   }
}

static enum pipe_swizzle
vk_swizzle_to_pipe(VkComponentSwizzle swizzle)
{
   switch (swizzle) {
   case VK_COMPONENT_SWIZZLE_R:     return PIPE_SWIZZLE_X;
   case VK_COMPONENT_SWIZZLE_G:     return PIPE_SWIZZLE_Y;
   case VK_COMPONENT_SWIZZLE_B:     return PIPE_SWIZZLE_Z;
   case VK_COMPONENT_SWIZZLE_A:     return PIPE_SWIZZLE_W;
   case VK_COMPONENT_SWIZZLE_ONE:   return PIPE_SWIZZLE_1;
   case VK_COMPONENT_SWIZZLE_ZERO:  return PIPE_SWIZZLE_0;
   default:
      unreachable("Invalid component swizzle");
   }
}

static void
image_uncompressed_view(struct nil_image *image,
                        struct nil_view *view,
                        uint64_t *base_addr)
{
   assert(view->num_levels == 1);

   uint64_t offset_B;
   nil_image_level_as_uncompressed(image, view->base_level, image, &offset_B);
   *base_addr += offset_B;
   view->base_level = 0;
}

static void
image_3d_view_as_2d_array(struct nil_image *image,
                          struct nil_view *view,
                          uint64_t *base_addr)
{
   assert(view->type == NIL_VIEW_TYPE_2D ||
          view->type == NIL_VIEW_TYPE_2D_ARRAY);
   assert(view->num_levels == 1);

   uint64_t offset_B;
   nil_image_3d_level_as_2d_array(image, view->base_level, image, &offset_B);
   *base_addr += offset_B;
   view->base_level = 0;
}

static enum pipe_format
get_stencil_format(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_S8_UINT:              return PIPE_FORMAT_S8_UINT;
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:    return PIPE_FORMAT_X24S8_UINT;
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:    return PIPE_FORMAT_S8X24_UINT;
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT: return PIPE_FORMAT_X32_S8X24_UINT;
   default: unreachable("Unsupported depth/stencil format");
   }
}

VkResult
nvk_image_view_init(struct nvk_device *dev,
                    struct nvk_image_view *view,
                    bool driver_internal,
                    const VkImageViewCreateInfo *pCreateInfo)
{
   VK_FROM_HANDLE(nvk_image, image, pCreateInfo->image);
   VkResult result;

   memset(view, 0, sizeof(*view));

   vk_image_view_init(&dev->vk, &view->vk, driver_internal, pCreateInfo);

   /* First, figure out which image planes we need.
    * For depth/stencil, we only have plane so simply assert
    * and then map directly betweeen the image and view plane
    */
   if (image->vk.aspects & (VK_IMAGE_ASPECT_DEPTH_BIT |
                            VK_IMAGE_ASPECT_STENCIL_BIT)) {
      assert(image->plane_count == 1);
      assert(nvk_image_aspects_to_plane(image, view->vk.aspects) == 0);
      view->plane_count = 1;
      view->planes[0].image_plane = 0;
   } else {
      /* For other formats, retrieve the plane count from the aspect mask
       * and then walk through the aspect mask to map each image plane
       * to its corresponding view plane
       */
      assert(util_bitcount(view->vk.aspects) ==
             vk_format_get_plane_count(view->vk.format));
      view->plane_count = 0;
      u_foreach_bit(aspect_bit, view->vk.aspects) {
         uint8_t image_plane = nvk_image_aspects_to_plane(image, 1u << aspect_bit);
         view->planes[view->plane_count++].image_plane = image_plane;
      }
   }

   /* Finally, fill in each view plane separately */
   for (unsigned view_plane = 0; view_plane < view->plane_count; view_plane++) {
      const uint8_t image_plane = view->planes[view_plane].image_plane;
      struct nil_image nil_image = image->planes[image_plane].nil;
      uint64_t base_addr = nvk_image_base_address(image, image_plane);

      const struct vk_format_ycbcr_info *ycbcr_info =
         vk_format_get_ycbcr_info(view->vk.format);
      assert(ycbcr_info || view_plane == 0);
      VkFormat plane_format = ycbcr_info ?
         ycbcr_info->planes[view_plane].format : view->vk.format;
      enum pipe_format p_format = vk_format_to_pipe_format(plane_format);
      if (view->vk.aspects == VK_IMAGE_ASPECT_STENCIL_BIT)
         p_format = get_stencil_format(p_format);

      struct nil_view nil_view = {
         .type = vk_image_view_type_to_nil_view_type(view->vk.view_type),
         .format = p_format,
         .base_level = view->vk.base_mip_level,
         .num_levels = view->vk.level_count,
         .base_array_layer = view->vk.base_array_layer,
         .array_len = view->vk.layer_count,
         .swizzle = {
            vk_swizzle_to_pipe(view->vk.swizzle.r),
            vk_swizzle_to_pipe(view->vk.swizzle.g),
            vk_swizzle_to_pipe(view->vk.swizzle.b),
            vk_swizzle_to_pipe(view->vk.swizzle.a),
         },
         .min_lod_clamp = view->vk.min_lod,
      };

      if (util_format_is_compressed(nil_image.format) &&
         !util_format_is_compressed(nil_view.format))
         image_uncompressed_view(&nil_image, &nil_view, &base_addr);

      if (nil_image.dim == NIL_IMAGE_DIM_3D &&
         nil_view.type != NIL_VIEW_TYPE_3D)
         image_3d_view_as_2d_array(&nil_image, &nil_view, &base_addr);

      if (view->vk.usage & (VK_IMAGE_USAGE_SAMPLED_BIT |
                           VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
         uint32_t tic[8];
         nil_image_fill_tic(&nvk_device_physical(dev)->info,
                           &nil_image, &nil_view, base_addr, tic);

         result = nvk_descriptor_table_add(dev, &dev->images, tic, sizeof(tic),
                                          &view->planes[view_plane].sampled_desc_index);
         if (result != VK_SUCCESS) {
            nvk_image_view_finish(dev, view);
            return result;
         }
      }

      if (view->vk.usage & VK_IMAGE_USAGE_STORAGE_BIT) {
         /* For storage images, we can't have any cubes */
         if (view->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE ||
            view->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
            nil_view.type = NIL_VIEW_TYPE_2D_ARRAY;

         if (view->vk.view_type == VK_IMAGE_VIEW_TYPE_3D) {
            /* Without VK_AMD_shader_image_load_store_lod, the client can only
             * get at the first LOD from the shader anyway.
             */
            assert(view->vk.base_array_layer == 0);
            assert(view->vk.layer_count == 1);
            nil_view.type = NIL_VIEW_TYPE_2D_ARRAY;
            nil_view.num_levels = 1;
            nil_view.base_array_layer = view->vk.storage.z_slice_offset;
            nil_view.array_len = view->vk.storage.z_slice_count;
            image_3d_view_as_2d_array(&nil_image, &nil_view, &base_addr);
         }

         uint32_t tic[8];
         nil_image_fill_tic(&nvk_device_physical(dev)->info,
                           &nil_image, &nil_view, base_addr, tic);

         result = nvk_descriptor_table_add(dev, &dev->images, tic, sizeof(tic),
                                          &view->planes[view_plane].storage_desc_index);
         if (result != VK_SUCCESS) {
            nvk_image_view_finish(dev, view);
            return result;
         }
      }
   }

   return VK_SUCCESS;
}

void
nvk_image_view_finish(struct nvk_device *dev,
                      struct nvk_image_view *view)
{
   for (uint8_t plane = 0; plane < view->plane_count; plane++) {
      if (view->planes[plane].sampled_desc_index) {
      nvk_descriptor_table_remove(dev, &dev->images,
                                  view->planes[plane].sampled_desc_index);
      }

      if (view->planes[plane].storage_desc_index) {
         nvk_descriptor_table_remove(dev, &dev->images,
                                    view->planes[plane].storage_desc_index);
      }
   }

   vk_image_view_finish(&view->vk);
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateImageView(VkDevice _device,
                    const VkImageViewCreateInfo *pCreateInfo,
                    const VkAllocationCallbacks *pAllocator,
                    VkImageView *pView)
{
   VK_FROM_HANDLE(nvk_device, dev, _device);
   struct nvk_image_view *view;
   VkResult result;

   view = vk_alloc2(&dev->vk.alloc, pAllocator, sizeof(*view), 8,
                    VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!view)
      return vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = nvk_image_view_init(dev, view, false, pCreateInfo);
   if (result != VK_SUCCESS) {
      vk_free2(&dev->vk.alloc, pAllocator, view);
      return result;
   }

   *pView = nvk_image_view_to_handle(view);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
nvk_DestroyImageView(VkDevice _device,
                     VkImageView imageView,
                     const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(nvk_device, dev, _device);
   VK_FROM_HANDLE(nvk_image_view, view, imageView);

   if (!view)
      return;

   nvk_image_view_finish(dev, view);
   vk_free2(&dev->vk.alloc, pAllocator, view);
}
