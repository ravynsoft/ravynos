/*
 * Copyright © 2021 Intel Corporation
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

#include "vk_image.h"

#ifndef _WIN32
#include <drm-uapi/drm_fourcc.h>
#endif

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_format.h"
#include "vk_format_info.h"
#include "vk_log.h"
#include "vk_physical_device.h"
#include "vk_render_pass.h"
#include "vk_util.h"
#include "vulkan/wsi/wsi_common.h"

#ifdef ANDROID
#include "vk_android.h"
#include <vulkan/vulkan_android.h>
#endif

void
vk_image_init(struct vk_device *device,
              struct vk_image *image,
              const VkImageCreateInfo *pCreateInfo)
{
   vk_object_base_init(device, &image->base, VK_OBJECT_TYPE_IMAGE);

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
   assert(pCreateInfo->mipLevels > 0);
   assert(pCreateInfo->arrayLayers > 0);
   assert(pCreateInfo->samples > 0);
   assert(pCreateInfo->extent.width > 0);
   assert(pCreateInfo->extent.height > 0);
   assert(pCreateInfo->extent.depth > 0);

   if (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
      assert(pCreateInfo->imageType == VK_IMAGE_TYPE_2D);
   if (pCreateInfo->flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT)
      assert(pCreateInfo->imageType == VK_IMAGE_TYPE_3D);

   image->create_flags = pCreateInfo->flags;
   image->image_type = pCreateInfo->imageType;
   vk_image_set_format(image, pCreateInfo->format);
   image->extent = vk_image_sanitize_extent(image, pCreateInfo->extent);
   image->mip_levels = pCreateInfo->mipLevels;
   image->array_layers = pCreateInfo->arrayLayers;
   image->samples = pCreateInfo->samples;
   image->tiling = pCreateInfo->tiling;
   image->usage = pCreateInfo->usage;

   if (image->aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      const VkImageStencilUsageCreateInfo *stencil_usage_info =
         vk_find_struct_const(pCreateInfo->pNext,
                              IMAGE_STENCIL_USAGE_CREATE_INFO);
      image->stencil_usage =
         stencil_usage_info ? stencil_usage_info->stencilUsage :
                              pCreateInfo->usage;
   } else {
      image->stencil_usage = 0;
   }

   const VkExternalMemoryImageCreateInfo *ext_mem_info =
      vk_find_struct_const(pCreateInfo->pNext, EXTERNAL_MEMORY_IMAGE_CREATE_INFO);
   if (ext_mem_info)
      image->external_handle_types = ext_mem_info->handleTypes;
   else
      image->external_handle_types = 0;

   const struct wsi_image_create_info *wsi_info =
      vk_find_struct_const(pCreateInfo->pNext, WSI_IMAGE_CREATE_INFO_MESA);
   image->wsi_legacy_scanout = wsi_info && wsi_info->scanout;

#ifndef _WIN32
   image->drm_format_mod = ((1ULL << 56) - 1) /* DRM_FORMAT_MOD_INVALID */;
#endif

#ifdef ANDROID
   const VkExternalFormatANDROID *ext_format =
      vk_find_struct_const(pCreateInfo->pNext, EXTERNAL_FORMAT_ANDROID);
   if (ext_format && ext_format->externalFormat != 0) {
      assert(image->format == VK_FORMAT_UNDEFINED);
      assert(image->external_handle_types &
             VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID);
      vk_image_set_format(image, (VkFormat)ext_format->externalFormat);
   }

   image->ahb_format = vk_image_format_to_ahb_format(image->format);
#endif
}

void *
vk_image_create(struct vk_device *device,
                const VkImageCreateInfo *pCreateInfo,
                const VkAllocationCallbacks *alloc,
                size_t size)
{
   struct vk_image *image =
      vk_zalloc2(&device->alloc, alloc, size, 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (image == NULL)
      return NULL;

   vk_image_init(device, image, pCreateInfo);

   return image;
}

void
vk_image_finish(struct vk_image *image)
{
   vk_object_base_finish(&image->base);
}

void
vk_image_destroy(struct vk_device *device,
                 const VkAllocationCallbacks *alloc,
                 struct vk_image *image)
{
   vk_object_free(device, alloc, image);
}

#ifndef _WIN32
VKAPI_ATTR VkResult VKAPI_CALL
vk_common_GetImageDrmFormatModifierPropertiesEXT(UNUSED VkDevice device,
                                                 VkImage _image,
                                                 VkImageDrmFormatModifierPropertiesEXT *pProperties)
{
   VK_FROM_HANDLE(vk_image, image, _image);

   assert(pProperties->sType ==
          VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT);

   assert(image->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT);
   pProperties->drmFormatModifier = image->drm_format_mod;

   return VK_SUCCESS;
}
#endif

VKAPI_ATTR void VKAPI_CALL
vk_common_GetImageSubresourceLayout(VkDevice _device, VkImage _image,
                                    const VkImageSubresource *pSubresource,
                                    VkSubresourceLayout *pLayout)
{
   VK_FROM_HANDLE(vk_device, device, _device);

   const VkImageSubresource2KHR subresource = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_SUBRESOURCE_2_KHR,
      .imageSubresource = *pSubresource,
   };

   VkSubresourceLayout2KHR layout = {
      .sType = VK_STRUCTURE_TYPE_SUBRESOURCE_LAYOUT_2_KHR
   };

   device->dispatch_table.GetImageSubresourceLayout2KHR(_device, _image,
                                                        &subresource, &layout);

   *pLayout = layout.subresourceLayout;
}

void
vk_image_set_format(struct vk_image *image, VkFormat format)
{
   image->format = format;
   image->aspects = vk_format_aspects(format);
}

VkImageUsageFlags
vk_image_usage(const struct vk_image *image,
               VkImageAspectFlags aspect_mask)
{
   /* From the Vulkan 1.2.131 spec:
    *
    *    "If the image was has a depth-stencil format and was created with
    *    a VkImageStencilUsageCreateInfo structure included in the pNext
    *    chain of VkImageCreateInfo, the usage is calculated based on the
    *    subresource.aspectMask provided:
    *
    *     - If aspectMask includes only VK_IMAGE_ASPECT_STENCIL_BIT, the
    *       implicit usage is equal to
    *       VkImageStencilUsageCreateInfo::stencilUsage.
    *
    *     - If aspectMask includes only VK_IMAGE_ASPECT_DEPTH_BIT, the
    *       implicit usage is equal to VkImageCreateInfo::usage.
    *
    *     - If both aspects are included in aspectMask, the implicit usage
    *       is equal to the intersection of VkImageCreateInfo::usage and
    *       VkImageStencilUsageCreateInfo::stencilUsage.
    */
   if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
      return image->stencil_usage;
   } else if (aspect_mask == (VK_IMAGE_ASPECT_DEPTH_BIT |
                              VK_IMAGE_ASPECT_STENCIL_BIT)) {
      return image->usage & image->stencil_usage;
   } else {
      /* This also handles the color case */
      return image->usage;
   }
}

#define VK_IMAGE_ASPECT_ANY_COLOR_MASK_MESA ( \
   VK_IMAGE_ASPECT_COLOR_BIT | \
   VK_IMAGE_ASPECT_PLANE_0_BIT | \
   VK_IMAGE_ASPECT_PLANE_1_BIT | \
   VK_IMAGE_ASPECT_PLANE_2_BIT)

/** Expands the given aspect mask relative to the image
 *
 * If the image has color plane aspects VK_IMAGE_ASPECT_COLOR_BIT has been
 * requested, this returns the aspects of the underlying image.
 *
 * For example,
 *
 *    VK_IMAGE_ASPECT_COLOR_BIT
 *
 * will be converted to
 *
 *    VK_IMAGE_ASPECT_PLANE_0_BIT |
 *    VK_IMAGE_ASPECT_PLANE_1_BIT |
 *    VK_IMAGE_ASPECT_PLANE_2_BIT
 *
 * for an image of format VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM.
 */
VkImageAspectFlags
vk_image_expand_aspect_mask(const struct vk_image *image,
                            VkImageAspectFlags aspect_mask)
{
   if (aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT) {
      assert(image->aspects & VK_IMAGE_ASPECT_ANY_COLOR_MASK_MESA);
      return image->aspects;
   } else {
      assert(aspect_mask && !(aspect_mask & ~image->aspects));
      return aspect_mask;
   }
}

VkExtent3D
vk_image_extent_to_elements(const struct vk_image *image, VkExtent3D extent)
{
   const struct util_format_description *fmt =
      vk_format_description(image->format);

   extent = vk_image_sanitize_extent(image, extent);
   extent.width = DIV_ROUND_UP(extent.width, fmt->block.width);
   extent.height = DIV_ROUND_UP(extent.height, fmt->block.height);
   extent.depth = DIV_ROUND_UP(extent.depth, fmt->block.depth);

   return extent;
}

VkOffset3D
vk_image_offset_to_elements(const struct vk_image *image, VkOffset3D offset)
{
   const struct util_format_description *fmt =
      vk_format_description(image->format);

   offset = vk_image_sanitize_offset(image, offset);

   assert(offset.x % fmt->block.width == 0);
   assert(offset.y % fmt->block.height == 0);
   assert(offset.z % fmt->block.depth == 0);

   offset.x /= fmt->block.width;
   offset.y /= fmt->block.height;
   offset.z /= fmt->block.depth;

   return offset;
}

struct vk_image_buffer_layout
vk_image_buffer_copy_layout(const struct vk_image *image,
                            const VkBufferImageCopy2* region)
{
   VkExtent3D extent = vk_image_sanitize_extent(image, region->imageExtent);

   const uint32_t row_length = region->bufferRowLength ?
                               region->bufferRowLength : extent.width;
   const uint32_t image_height = region->bufferImageHeight ?
                                 region->bufferImageHeight : extent.height;

   const VkImageAspectFlags aspect = region->imageSubresource.aspectMask;
   VkFormat format = vk_format_get_aspect_format(image->format, aspect);
   const struct util_format_description *fmt = vk_format_description(format);

   assert(fmt->block.bits % 8 == 0);
   const uint32_t element_size_B = fmt->block.bits / 8;

   const uint32_t row_stride_B =
      DIV_ROUND_UP(row_length, fmt->block.width) * element_size_B;
   const uint64_t image_stride_B =
      DIV_ROUND_UP(image_height, fmt->block.height) * (uint64_t)row_stride_B;

   return (struct vk_image_buffer_layout) {
      .row_length = row_length,
      .image_height = image_height,
      .element_size_B = element_size_B,
      .row_stride_B = row_stride_B,
      .image_stride_B = image_stride_B,
   };
}

struct vk_image_buffer_layout
vk_memory_to_image_copy_layout(const struct vk_image *image,
                               const VkMemoryToImageCopyEXT* region)
{
   const VkBufferImageCopy2 bic = {
      .bufferOffset = 0,
      .bufferRowLength = region->memoryRowLength,
      .bufferImageHeight = region->memoryImageHeight,
      .imageSubresource = region->imageSubresource,
      .imageOffset = region->imageOffset,
      .imageExtent = region->imageExtent,
   };
   return vk_image_buffer_copy_layout(image, &bic);
}

struct vk_image_buffer_layout
vk_image_to_memory_copy_layout(const struct vk_image *image,
                               const VkImageToMemoryCopyEXT* region)
{
   const VkBufferImageCopy2 bic = {
      .bufferOffset = 0,
      .bufferRowLength = region->memoryRowLength,
      .bufferImageHeight = region->memoryImageHeight,
      .imageSubresource = region->imageSubresource,
      .imageOffset = region->imageOffset,
      .imageExtent = region->imageExtent,
   };
   return vk_image_buffer_copy_layout(image, &bic);
}

static VkComponentSwizzle
remap_swizzle(VkComponentSwizzle swizzle, VkComponentSwizzle component)
{
   return swizzle == VK_COMPONENT_SWIZZLE_IDENTITY ? component : swizzle;
}

void
vk_image_view_init(struct vk_device *device,
                   struct vk_image_view *image_view,
                   bool driver_internal,
                   const VkImageViewCreateInfo *pCreateInfo)
{
   vk_object_base_init(device, &image_view->base, VK_OBJECT_TYPE_IMAGE_VIEW);

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
   VK_FROM_HANDLE(vk_image, image, pCreateInfo->image);

   image_view->create_flags = pCreateInfo->flags;
   image_view->image = image;
   image_view->view_type = pCreateInfo->viewType;

   image_view->format = pCreateInfo->format;
   if (image_view->format == VK_FORMAT_UNDEFINED)
      image_view->format = image->format;

   if (!driver_internal) {
      switch (image_view->view_type) {
      case VK_IMAGE_VIEW_TYPE_1D:
      case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
         assert(image->image_type == VK_IMAGE_TYPE_1D);
         break;
      case VK_IMAGE_VIEW_TYPE_2D:
      case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
         if (image->create_flags & (VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT |
                                    VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT))
            assert(image->image_type == VK_IMAGE_TYPE_3D);
         else
            assert(image->image_type == VK_IMAGE_TYPE_2D);
         break;
      case VK_IMAGE_VIEW_TYPE_3D:
         assert(image->image_type == VK_IMAGE_TYPE_3D);
         break;
      case VK_IMAGE_VIEW_TYPE_CUBE:
      case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
         assert(image->image_type == VK_IMAGE_TYPE_2D);
         assert(image->create_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
         break;
      default:
         unreachable("Invalid image view type");
      }
   }

   const VkImageSubresourceRange *range = &pCreateInfo->subresourceRange;

   if (driver_internal) {
      image_view->aspects = range->aspectMask;
      image_view->view_format = image_view->format;
   } else {
      image_view->aspects =
         vk_image_expand_aspect_mask(image, range->aspectMask);

      assert(!(image_view->aspects & ~image->aspects));

      /* From the Vulkan 1.2.184 spec:
       *
       *    "If the image has a multi-planar format and
       *    subresourceRange.aspectMask is VK_IMAGE_ASPECT_COLOR_BIT, and image
       *    has been created with a usage value not containing any of the
       *    VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
       *    VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
       *    VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
       *    VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR,
       *    VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR, and
       *    VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR flags, then the format must
       *    be identical to the image format, and the sampler to be used with the
       *    image view must enable sampler Y′CBCR conversion."
       *
       * Since no one implements video yet, we can ignore the bits about video
       * create flags and assume YCbCr formats match.
       */
      if ((image->aspects & VK_IMAGE_ASPECT_PLANE_1_BIT) &&
          (range->aspectMask == VK_IMAGE_ASPECT_COLOR_BIT))
         assert(image_view->format == image->format);

      /* From the Vulkan 1.2.184 spec:
       *
       *    "Each depth/stencil format is only compatible with itself."
       */
      if (image_view->aspects & (VK_IMAGE_ASPECT_DEPTH_BIT |
                                 VK_IMAGE_ASPECT_STENCIL_BIT))
         assert(image_view->format == image->format);

      if (!(image->create_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT))
         assert(image_view->format == image->format);

      /* Restrict the format to only the planes chosen.
       *
       * For combined depth and stencil images, this means the depth-only or
       * stencil-only format if only one aspect is chosen and the full
       * combined format if both aspects are chosen.
       *
       * For single-plane color images, we just take the format as-is.  For
       * multi-plane views of multi-plane images, this means we want the full
       * multi-plane format.  For single-plane views of multi-plane images, we
       * want a format compatible with the one plane.  Fortunately, this is
       * already what the client gives us.  The Vulkan 1.2.184 spec says:
       *
       *    "If image was created with the VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT
       *    and the image has a multi-planar format, and if
       *    subresourceRange.aspectMask is VK_IMAGE_ASPECT_PLANE_0_BIT,
       *    VK_IMAGE_ASPECT_PLANE_1_BIT, or VK_IMAGE_ASPECT_PLANE_2_BIT,
       *    format must be compatible with the corresponding plane of the
       *    image, and the sampler to be used with the image view must not
       *    enable sampler Y′CBCR conversion."
       */
      if (image_view->aspects == VK_IMAGE_ASPECT_STENCIL_BIT) {
         image_view->view_format = vk_format_stencil_only(image_view->format);
      } else if (image_view->aspects == VK_IMAGE_ASPECT_DEPTH_BIT) {
         image_view->view_format = vk_format_depth_only(image_view->format);
      } else {
         image_view->view_format = image_view->format;
      }
   }

   image_view->swizzle = (VkComponentMapping) {
      .r = remap_swizzle(pCreateInfo->components.r, VK_COMPONENT_SWIZZLE_R),
      .g = remap_swizzle(pCreateInfo->components.g, VK_COMPONENT_SWIZZLE_G),
      .b = remap_swizzle(pCreateInfo->components.b, VK_COMPONENT_SWIZZLE_B),
      .a = remap_swizzle(pCreateInfo->components.a, VK_COMPONENT_SWIZZLE_A),
   };

   assert(range->layerCount > 0);
   assert(range->baseMipLevel < image->mip_levels);

   image_view->base_mip_level = range->baseMipLevel;
   image_view->level_count = vk_image_subresource_level_count(image, range);
   image_view->base_array_layer = range->baseArrayLayer;
   image_view->layer_count = vk_image_subresource_layer_count(image, range);

   const VkImageViewMinLodCreateInfoEXT *min_lod_info =
      vk_find_struct_const(pCreateInfo, IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT);
   image_view->min_lod = min_lod_info ? min_lod_info->minLod : 0.0f;

   /* From the Vulkan 1.3.215 spec:
    *
    *    VUID-VkImageViewMinLodCreateInfoEXT-minLod-06456
    *
    *    "minLod must be less or equal to the index of the last mipmap level
    *    accessible to the view."
    */
   assert(image_view->min_lod <= image_view->base_mip_level +
                                 image_view->level_count - 1);

   image_view->extent =
      vk_image_mip_level_extent(image, image_view->base_mip_level);

   /* By default storage uses the same as the image properties, but it can be
    * overriden with VkImageViewSlicedCreateInfoEXT.
    */
   image_view->storage.z_slice_offset = 0;
   image_view->storage.z_slice_count = image_view->extent.depth;

   const VkImageViewSlicedCreateInfoEXT *sliced_info =
      vk_find_struct_const(pCreateInfo, IMAGE_VIEW_SLICED_CREATE_INFO_EXT);
   assert(image_view->base_mip_level + image_view->level_count
          <= image->mip_levels);
   switch (image->image_type) {
   default:
      unreachable("bad VkImageType");
   case VK_IMAGE_TYPE_1D:
   case VK_IMAGE_TYPE_2D:
      assert(image_view->base_array_layer + image_view->layer_count
             <= image->array_layers);
      break;
   case VK_IMAGE_TYPE_3D:
      if (sliced_info && image_view->view_type == VK_IMAGE_VIEW_TYPE_3D) {
         unsigned total = image_view->extent.depth;
         image_view->storage.z_slice_offset = sliced_info->sliceOffset;
         assert(image_view->storage.z_slice_offset < total);
         if (sliced_info->sliceCount == VK_REMAINING_3D_SLICES_EXT) {
            image_view->storage.z_slice_count = total - image_view->storage.z_slice_offset;
         } else {
            image_view->storage.z_slice_count = sliced_info->sliceCount;
         }
      } else if (image_view->view_type != VK_IMAGE_VIEW_TYPE_3D) {
         image_view->storage.z_slice_offset = image_view->base_array_layer;
         image_view->storage.z_slice_count = image_view->layer_count;
      }
      assert(image_view->storage.z_slice_offset + image_view->storage.z_slice_count
             <= image->extent.depth);
      assert(image_view->base_array_layer + image_view->layer_count
             <= image_view->extent.depth);
      break;
   }

   /* If we are creating a color view from a depth/stencil image we compute
    * usage from the underlying depth/stencil aspects.
    */
   const VkImageUsageFlags image_usage =
      vk_image_usage(image, image_view->aspects);
   const VkImageViewUsageCreateInfo *usage_info =
      vk_find_struct_const(pCreateInfo, IMAGE_VIEW_USAGE_CREATE_INFO);
   image_view->usage = usage_info ? usage_info->usage : image_usage;
   assert(driver_internal || !(image_view->usage & ~image_usage));
}

void
vk_image_view_finish(struct vk_image_view *image_view)
{
   vk_object_base_finish(&image_view->base);
}

void *
vk_image_view_create(struct vk_device *device,
                     bool driver_internal,
                     const VkImageViewCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *alloc,
                     size_t size)
{
   struct vk_image_view *image_view =
      vk_zalloc2(&device->alloc, alloc, size, 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (image_view == NULL)
      return NULL;

   vk_image_view_init(device, image_view, driver_internal, pCreateInfo);

   return image_view;
}

void
vk_image_view_destroy(struct vk_device *device,
                      const VkAllocationCallbacks *alloc,
                      struct vk_image_view *image_view)
{
   vk_object_free(device, alloc, image_view);
}

bool
vk_image_layout_is_read_only(VkImageLayout layout,
                             VkImageAspectFlagBits aspect)
{
   assert(util_bitcount(aspect) == 1);

   switch (layout) {
   case VK_IMAGE_LAYOUT_UNDEFINED:
   case VK_IMAGE_LAYOUT_PREINITIALIZED:
      return true; /* These are only used for layout transitions */

   case VK_IMAGE_LAYOUT_GENERAL:
   case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
   case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
      return false;

   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
   case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
   case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
   case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
   case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
      return true;

   case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
      return aspect == VK_IMAGE_ASPECT_DEPTH_BIT;

   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
      return aspect == VK_IMAGE_ASPECT_STENCIL_BIT;

   case VK_IMAGE_LAYOUT_MAX_ENUM:
   case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
   case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
   case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
   case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
   case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
   case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
      unreachable("Invalid image layout.");
   }

   unreachable("Invalid image layout.");
}

bool
vk_image_layout_is_depth_only(VkImageLayout layout)
{
   switch (layout) {
   case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
      return true;

   default:
      return false;
   }
}

static VkResult
vk_image_create_get_format_list_uncompressed(struct vk_device *device,
                                             const VkImageCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkFormat **formats,
                                             uint32_t *format_count)
{
   const struct vk_format_class_info *class =
      vk_format_get_class_info(pCreateInfo->format);

   *formats = NULL;
   *format_count = 0;

   if (class->format_count < 2)
      return VK_SUCCESS;

   *formats = vk_alloc2(&device->alloc, pAllocator,
                        sizeof(VkFormat) * class->format_count,
                        alignof(VkFormat), VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (*formats == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   memcpy(*formats, class->formats, sizeof(VkFormat) * class->format_count);
   *format_count = class->format_count;

   return VK_SUCCESS;
}

static VkResult
vk_image_create_get_format_list_compressed(struct vk_device *device,
                                           const VkImageCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator,
                                           VkFormat **formats,
                                           uint32_t *format_count)
{
   if ((pCreateInfo->flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) == 0) {
      return vk_image_create_get_format_list_uncompressed(device,
                                                          pCreateInfo,
                                                          pAllocator,
                                                          formats,
                                                          format_count);
   }

   const struct vk_format_class_info *class =
      vk_format_get_class_info(pCreateInfo->format);
   const struct vk_format_class_info *uncompr_class = NULL;

   switch (vk_format_get_blocksizebits(pCreateInfo->format)) {
   case 64:
      uncompr_class = vk_format_class_get_info(MESA_VK_FORMAT_CLASS_64_BIT);
      break;
   case 128:
      uncompr_class = vk_format_class_get_info(MESA_VK_FORMAT_CLASS_128_BIT);
      break;
   }

   if (!uncompr_class)
      return vk_error(device, VK_ERROR_FORMAT_NOT_SUPPORTED);

   uint32_t fmt_count = class->format_count + uncompr_class->format_count;

   *formats = vk_alloc2(&device->alloc, pAllocator,
                        sizeof(VkFormat) * fmt_count,
                        alignof(VkFormat), VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (*formats == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   memcpy(*formats, class->formats, sizeof(VkFormat) * class->format_count);
   memcpy(*formats + class->format_count, uncompr_class->formats,
          sizeof(VkFormat) * uncompr_class->format_count);
   *format_count = class->format_count + uncompr_class->format_count;

   return VK_SUCCESS;
}

/* Get a list of compatible formats when VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT
 * or VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT is set. This list is
 * either retrieved from a VkImageFormatListCreateInfo passed to the creation
 * chain, or forged from the default compatible list specified in the
 * "formats-compatibility-classes" section of the spec.
 *
 * The value returned in *formats must be freed with
 * vk_free2(&device->alloc, pAllocator), and should not live past the
 * vkCreateImage() call (allocated in the COMMAND scope).
 */
VkResult
vk_image_create_get_format_list(struct vk_device *device,
                                const VkImageCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator,
                                VkFormat **formats,
                                uint32_t *format_count)
{
   *formats = NULL;
   *format_count = 0;

   if (!(pCreateInfo->flags &
         (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT |
          VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT))) {
      return VK_SUCCESS;
   }

   /* "Each depth/stencil format is only compatible with itself." */
   if (vk_format_is_depth_or_stencil(pCreateInfo->format))
      return VK_SUCCESS;

   const VkImageFormatListCreateInfo *format_list = (const VkImageFormatListCreateInfo *)
      vk_find_struct_const(pCreateInfo->pNext, IMAGE_FORMAT_LIST_CREATE_INFO);

   if (format_list) {
      if (!format_list->viewFormatCount)
         return VK_SUCCESS;

      *formats = vk_alloc2(&device->alloc, pAllocator,
                           sizeof(VkFormat) * format_list->viewFormatCount,
                           alignof(VkFormat), VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
      if (*formats == NULL)
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

      memcpy(*formats, format_list->pViewFormats, sizeof(VkFormat) * format_list->viewFormatCount);
      *format_count = format_list->viewFormatCount;
      return VK_SUCCESS;
   }

   if (vk_format_is_compressed(pCreateInfo->format))
      return vk_image_create_get_format_list_compressed(device,
                                                        pCreateInfo,
                                                        pAllocator,
                                                        formats,
                                                        format_count);

   return vk_image_create_get_format_list_uncompressed(device,
                                                       pCreateInfo,
                                                       pAllocator,
                                                       formats,
                                                       format_count);
}

/* From the Vulkan Specification 1.2.166 - VkAttachmentReference2:
 *
 *   "If layout only specifies the layout of the depth aspect of the
 *    attachment, the layout of the stencil aspect is specified by the
 *    stencilLayout member of a VkAttachmentReferenceStencilLayout structure
 *    included in the pNext chain. Otherwise, layout describes the layout for
 *    all relevant image aspects."
 */
VkImageLayout
vk_att_ref_stencil_layout(const VkAttachmentReference2 *att_ref,
                          const VkAttachmentDescription2 *attachments)
{
   /* From VUID-VkAttachmentReference2-attachment-04755:
    *  "If attachment is not VK_ATTACHMENT_UNUSED, and the format of the
    *   referenced attachment is a depth/stencil format which includes both
    *   depth and stencil aspects [...]
    */
   if (att_ref->attachment == VK_ATTACHMENT_UNUSED ||
       !vk_format_has_stencil(attachments[att_ref->attachment].format))
      return VK_IMAGE_LAYOUT_UNDEFINED;

   const VkAttachmentReferenceStencilLayout *stencil_ref =
      vk_find_struct_const(att_ref->pNext, ATTACHMENT_REFERENCE_STENCIL_LAYOUT);

   if (stencil_ref)
      return stencil_ref->stencilLayout;

   /* From VUID-VkAttachmentReference2-attachment-04755:
    *  "If attachment is not VK_ATTACHMENT_UNUSED, and the format of the
    *   referenced attachment is a depth/stencil format which includes both
    *   depth and stencil aspects, and layout is
    *   VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or
    *   VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, the pNext chain must include
    *   a VkAttachmentReferenceStencilLayout structure."
    */
   assert(!vk_image_layout_is_depth_only(att_ref->layout));

   return att_ref->layout;
}

/* From the Vulkan Specification 1.2.184:
 *
 *   "If the pNext chain includes a VkAttachmentDescriptionStencilLayout
 *    structure, then the stencilInitialLayout and stencilFinalLayout members
 *    specify the initial and final layouts of the stencil aspect of a
 *    depth/stencil format, and initialLayout and finalLayout only apply to the
 *    depth aspect. For depth-only formats, the
 *    VkAttachmentDescriptionStencilLayout structure is ignored. For
 *    stencil-only formats, the initial and final layouts of the stencil aspect
 *    are taken from the VkAttachmentDescriptionStencilLayout structure if
 *    present, or initialLayout and finalLayout if not present."
 *
 *   "If format is a depth/stencil format, and either initialLayout or
 *    finalLayout does not specify a layout for the stencil aspect, then the
 *    application must specify the initial and final layouts of the stencil
 *    aspect by including a VkAttachmentDescriptionStencilLayout structure in
 *    the pNext chain."
 */
VkImageLayout
vk_att_desc_stencil_layout(const VkAttachmentDescription2 *att_desc, bool final)
{
   if (!vk_format_has_stencil(att_desc->format))
      return VK_IMAGE_LAYOUT_UNDEFINED;

   const VkAttachmentDescriptionStencilLayout *stencil_desc =
      vk_find_struct_const(att_desc->pNext, ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT);

   if (stencil_desc) {
      return final ?
         stencil_desc->stencilFinalLayout :
         stencil_desc->stencilInitialLayout;
   }

   const VkImageLayout main_layout =
      final ? att_desc->finalLayout : att_desc->initialLayout;

   /* From VUID-VkAttachmentDescription2-format-03302/03303:
    *  "If format is a depth/stencil format which includes both depth and
    *   stencil aspects, and initial/finalLayout is
    *   VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or
    *   VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, the pNext chain must include
    *   a VkAttachmentDescriptionStencilLayout structure."
    */
   assert(!vk_image_layout_is_depth_only(main_layout));

   return main_layout;
}

VkImageUsageFlags
vk_image_layout_to_usage_flags(VkImageLayout layout,
                               VkImageAspectFlagBits aspect)
{
   assert(util_bitcount(aspect) == 1);

   switch (layout) {
   case VK_IMAGE_LAYOUT_UNDEFINED:
   case VK_IMAGE_LAYOUT_PREINITIALIZED:
      return 0u;

   case VK_IMAGE_LAYOUT_GENERAL:
      return ~0u;

   case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      assert(aspect & VK_IMAGE_ASPECT_ANY_COLOR_MASK_MESA);
      return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      assert(aspect & (VK_IMAGE_ASPECT_DEPTH_BIT |
                       VK_IMAGE_ASPECT_STENCIL_BIT));
      return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
      assert(aspect & VK_IMAGE_ASPECT_DEPTH_BIT);
      return vk_image_layout_to_usage_flags(
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, aspect);

   case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
      assert(aspect & VK_IMAGE_ASPECT_STENCIL_BIT);
      return vk_image_layout_to_usage_flags(
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, aspect);

   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
      assert(aspect & (VK_IMAGE_ASPECT_DEPTH_BIT |
                       VK_IMAGE_ASPECT_STENCIL_BIT));
      return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
             VK_IMAGE_USAGE_SAMPLED_BIT |
             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

   case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
      assert(aspect & VK_IMAGE_ASPECT_DEPTH_BIT);
      return vk_image_layout_to_usage_flags(
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, aspect);

   case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
      assert(aspect & VK_IMAGE_ASPECT_STENCIL_BIT);
      return vk_image_layout_to_usage_flags(
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, aspect);

   case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      return VK_IMAGE_USAGE_SAMPLED_BIT |
             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

   case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

   case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      return VK_IMAGE_USAGE_TRANSFER_DST_BIT;

   case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
      if (aspect == VK_IMAGE_ASPECT_DEPTH_BIT) {
         return vk_image_layout_to_usage_flags(
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, aspect);
      } else if (aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
         return vk_image_layout_to_usage_flags(
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, aspect);
      } else {
         assert(!"Must be a depth/stencil aspect");
         return 0;
      }

   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
      if (aspect == VK_IMAGE_ASPECT_DEPTH_BIT) {
         return vk_image_layout_to_usage_flags(
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, aspect);
      } else if (aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
         return vk_image_layout_to_usage_flags(
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, aspect);
      } else {
         assert(!"Must be a depth/stencil aspect");
         return 0;
      }

   case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT);
      /* This needs to be handled specially by the caller */
      return 0;

   case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
      assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT);
      return vk_image_layout_to_usage_flags(VK_IMAGE_LAYOUT_GENERAL, aspect);

   case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
      assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT);
      return VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;

   case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
      assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT);
      return VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;

   case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
      if (aspect == VK_IMAGE_ASPECT_DEPTH_BIT ||
          aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
         return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      } else {
         assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT);
         return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      }

   case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
      return VK_IMAGE_USAGE_SAMPLED_BIT |
             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

   case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
      if (aspect == VK_IMAGE_ASPECT_DEPTH_BIT ||
          aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
         return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT |
                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
      } else {
         assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT);
         return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT |
                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
      }

   case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
      return VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
   case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
      return VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
   case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
      return VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
   case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
      return VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
   case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
      return VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
   case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
      return VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
   case VK_IMAGE_LAYOUT_MAX_ENUM:
      unreachable("Invalid image layout.");
   }

   unreachable("Invalid image layout.");
}
