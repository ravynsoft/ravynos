/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "dzn_private.h"

#include "vk_alloc.h"
#include "vk_debug_report.h"
#include "vk_format.h"
#include "vk_util.h"

void
dzn_image_align_extent(const struct dzn_image *image,
                       VkExtent3D *extent)
{
   enum pipe_format pfmt = vk_format_to_pipe_format(image->vk.format);
   uint32_t blkw = util_format_get_blockwidth(pfmt);
   uint32_t blkh = util_format_get_blockheight(pfmt);
   uint32_t blkd = util_format_get_blockdepth(pfmt);

   assert(util_is_power_of_two_nonzero(blkw) &&
          util_is_power_of_two_nonzero(blkh) &&
          util_is_power_of_two_nonzero(blkh));

   extent->width = ALIGN_POT(extent->width, blkw);
   extent->height = ALIGN_POT(extent->height, blkh);
   extent->depth = ALIGN_POT(extent->depth, blkd);
}

static void
dzn_image_destroy(struct dzn_image *image,
                  const VkAllocationCallbacks *pAllocator)
{
   if (!image)
      return;

   struct dzn_device *device = container_of(image->vk.base.device, struct dzn_device, vk);

   if (image->res)
      ID3D12Resource_Release(image->res);

   vk_image_finish(&image->vk);
   vk_free2(&device->vk.alloc, pAllocator, image);
}

static VkResult
dzn_image_create(struct dzn_device *device,
                 const VkImageCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *pAllocator,
                 VkImage *out)
{
   struct dzn_physical_device *pdev =
      container_of(device->vk.physical, struct dzn_physical_device, vk);
   VkFormat *compat_formats = NULL;
   uint32_t compat_format_count = 0;

   if (pdev->options12.RelaxedFormatCastingSupported) {
      VkResult ret =
         vk_image_create_get_format_list(&device->vk, pCreateInfo, pAllocator,
                                         &compat_formats, &compat_format_count);
      if (ret != VK_SUCCESS)
         return ret;
   }

   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct dzn_image, image, 1);
   VK_MULTIALLOC_DECL(&ma, DXGI_FORMAT, castable_formats, compat_format_count);

   if (!vk_multialloc_zalloc2(&ma, &device->vk.alloc, pAllocator, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT)) {
      vk_free2(&device->vk.alloc, pAllocator, compat_formats);
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

#if 0
    VkExternalMemoryHandleTypeFlags supported =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT |
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT |
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT |
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT |
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT |
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;

   if (create_info && (create_info->handleTypes & supported))
      return dzn_image_from_external(device, pCreateInfo, create_info,
                                     pAllocator, pImage);
#endif

#if 0
   const VkImageSwapchainCreateInfoKHR *swapchain_info = (const VkImageSwapchainCreateInfoKHR *)
      vk_find_struct_const(pCreateInfo->pNext, IMAGE_SWAPCHAIN_CREATE_INFO_KHR);
   if (swapchain_info && swapchain_info->swapchain != VK_NULL_HANDLE)
      return dzn_image_from_swapchain(device, pCreateInfo, swapchain_info,
                                      pAllocator, pImage);
#endif

   vk_image_init(&device->vk, &image->vk, pCreateInfo);
   enum pipe_format pfmt = vk_format_to_pipe_format(image->vk.format);

   VkImageUsageFlags usage = image->vk.usage | image->vk.stencil_usage;

   image->castable_formats = castable_formats;
   image->castable_format_count = 0;
   for (uint32_t i = 0; i < compat_format_count; i++) {
      castable_formats[image->castable_format_count] =
         dzn_image_get_dxgi_format(pdev, compat_formats[i], usage, 0);

      if (castable_formats[image->castable_format_count] != DXGI_FORMAT_UNKNOWN)
         image->castable_format_count++;
   }

   vk_free2(&device->vk.alloc, pAllocator, compat_formats);

   image->valid_access = D3D12_BARRIER_ACCESS_COPY_SOURCE | D3D12_BARRIER_ACCESS_COPY_DEST;

   if (image->vk.tiling == VK_IMAGE_TILING_LINEAR) {
      /* Treat linear images as buffers: they should only be used as copy
       * src/dest, and CopyTextureResource() can manipulate buffers.
       * We only support linear tiling on things strictly required by the spec:
       * "Images created with tiling equal to VK_IMAGE_TILING_LINEAR have
       * further restrictions on their limits and capabilities compared to
       * images created with tiling equal to VK_IMAGE_TILING_OPTIMAL. Creation
       * of images with tiling VK_IMAGE_TILING_LINEAR may not be supported
       * unless other parameters meet all of the constraints:
       * - imageType is VK_IMAGE_TYPE_2D
       * - format is not a depth/stencil format
       * - mipLevels is 1
       * - arrayLayers is 1
       * - samples is VK_SAMPLE_COUNT_1_BIT
       * - usage only includes VK_IMAGE_USAGE_TRANSFER_SRC_BIT and/or VK_IMAGE_USAGE_TRANSFER_DST_BIT
       * "
       */
      assert(!vk_format_is_depth_or_stencil(pCreateInfo->format));
      assert(pCreateInfo->mipLevels == 1);
      assert(pCreateInfo->arrayLayers == 1);
      assert(pCreateInfo->samples == 1);
      assert(pCreateInfo->imageType != VK_IMAGE_TYPE_3D);
      assert(!(usage & ~(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)));
      D3D12_RESOURCE_DESC tmp_desc = {
         .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
         .Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
         .Width = ALIGN_POT(image->vk.extent.width, util_format_get_blockwidth(pfmt)),
         .Height = (UINT)ALIGN_POT(image->vk.extent.height, util_format_get_blockheight(pfmt)),
         .DepthOrArraySize = 1,
         .MipLevels = 1,
         .Format =
            dzn_image_get_dxgi_format(pdev, pCreateInfo->format, usage, 0),
         .SampleDesc = { .Count = 1, .Quality = 0 },
         .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
         .Flags = D3D12_RESOURCE_FLAG_NONE
      };
      D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
      uint64_t size = 0;
      ID3D12Device1_GetCopyableFootprints(device->dev, &tmp_desc, 0, 1, 0, &footprint, NULL, NULL, &size);

      image->linear.row_stride = footprint.Footprint.RowPitch;
      image->linear.size = size;
      size *= pCreateInfo->arrayLayers;
      image->desc.Format = DXGI_FORMAT_UNKNOWN;
      image->desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      image->desc.Width = size;
      image->desc.Height = 1;
      image->desc.DepthOrArraySize = 1;
      image->desc.MipLevels = 1;
      image->desc.SampleDesc.Count = 1;
      image->desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      image->castable_formats = NULL;
      image->castable_format_count = 0;
   } else {
      image->desc.Format =
         dzn_image_get_dxgi_format(pdev, pCreateInfo->format,
                                   usage & ~VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                   0),
      image->desc.Dimension = (D3D12_RESOURCE_DIMENSION)(D3D12_RESOURCE_DIMENSION_TEXTURE1D + pCreateInfo->imageType);
      image->desc.Width = image->vk.extent.width;
      image->desc.Height = image->vk.extent.height;
      image->desc.DepthOrArraySize = pCreateInfo->imageType == VK_IMAGE_TYPE_3D ?
                                     image->vk.extent.depth :
                                     pCreateInfo->arrayLayers;
      image->desc.MipLevels = pCreateInfo->mipLevels;
      image->desc.SampleDesc.Count = pCreateInfo->samples;
      image->desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
      image->valid_access |= D3D12_BARRIER_ACCESS_RESOLVE_DEST |
         D3D12_BARRIER_ACCESS_SHADER_RESOURCE |
         (pCreateInfo->samples > 1 ? D3D12_BARRIER_ACCESS_RESOLVE_SOURCE : 0);
   }

   if ((image->vk.create_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) &&
       !pdev->options12.RelaxedFormatCastingSupported)
      image->desc.Format = dzn_get_typeless_dxgi_format(image->desc.Format);

   if (image->desc.SampleDesc.Count > 1)
      image->desc.Alignment = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
   else
      image->desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

   image->desc.SampleDesc.Quality = 0;

   image->desc.Flags = D3D12_RESOURCE_FLAG_NONE;

   if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
      image->desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
      image->valid_access |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
   }

   if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      image->desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
      image->valid_access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ |
                             D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;

      if (!(usage & (VK_IMAGE_USAGE_SAMPLED_BIT |
                               VK_IMAGE_USAGE_STORAGE_BIT |
                               VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT))) {
         image->desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
         image->valid_access &= ~D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
      }
   } else if (usage & VK_IMAGE_USAGE_STORAGE_BIT) {
      image->desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      image->valid_access |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
   }

   /* Images with TRANSFER_DST can be cleared or passed as a blit/resolve
    * destination. Both operations require the RT or DS cap flags.
    */
   if ((usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) &&
       image->vk.tiling == VK_IMAGE_TILING_OPTIMAL) {

      D3D12_FEATURE_DATA_FORMAT_SUPPORT dfmt_info =
         dzn_physical_device_get_format_support(pdev, pCreateInfo->format, pCreateInfo->flags);
      if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) {
         image->desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
         image->valid_access |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
      } else if ((dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) &&
                 (image->desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                                       D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)) == D3D12_RESOURCE_FLAG_NONE) {
         image->desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
         image->valid_access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
      } else if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) {
         image->desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
         image->valid_access |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
      }
   }

   if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT &&
       !(image->desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
      image->desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

   *out = dzn_image_to_handle(image);
   return VK_SUCCESS;
}

DXGI_FORMAT
dzn_image_get_dxgi_format(const struct dzn_physical_device *pdev,
                          VkFormat format,
                          VkImageUsageFlags usage,
                          VkImageAspectFlags aspects)
{
   enum pipe_format pfmt = vk_format_to_pipe_format(format);

   if (pdev && !pdev->support_a4b4g4r4) {
      if (pfmt == PIPE_FORMAT_A4R4G4B4_UNORM)
         return DXGI_FORMAT_B4G4R4A4_UNORM;
      if (pfmt == PIPE_FORMAT_A4B4G4R4_UNORM)
         return DXGI_FORMAT_UNKNOWN;
   }

   if (!vk_format_is_depth_or_stencil(format))
      return dzn_pipe_to_dxgi_format(pfmt);

   switch (pfmt) {
   case PIPE_FORMAT_Z16_UNORM:
      return usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ?
             DXGI_FORMAT_D16_UNORM : DXGI_FORMAT_R16_UNORM;

   case PIPE_FORMAT_Z32_FLOAT:
      return usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ?
             DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_R32_FLOAT;

   case PIPE_FORMAT_Z24X8_UNORM:
      if (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
         return DXGI_FORMAT_D24_UNORM_S8_UINT;
      if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT)
         return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
      return DXGI_FORMAT_R24G8_TYPELESS;

   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      if (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
         return DXGI_FORMAT_D24_UNORM_S8_UINT;

      if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT)
         return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
      else if (aspects & VK_IMAGE_ASPECT_STENCIL_BIT)
         return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
      else
         return DXGI_FORMAT_R24G8_TYPELESS;

   case PIPE_FORMAT_X24S8_UINT:
      if (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
         return DXGI_FORMAT_D24_UNORM_S8_UINT;
      if (aspects & VK_IMAGE_ASPECT_STENCIL_BIT)
         return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
      return DXGI_FORMAT_R24G8_TYPELESS;

   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      if (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
         return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

      if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT)
         return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
      else if (aspects & VK_IMAGE_ASPECT_STENCIL_BIT)
         return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
      else
         return DXGI_FORMAT_R32G8X24_TYPELESS;

   default:
      return dzn_pipe_to_dxgi_format(pfmt);
   }
}

DXGI_FORMAT
dzn_image_get_placed_footprint_format(const struct dzn_physical_device *pdev,
                                      VkFormat format,
                                      VkImageAspectFlags aspect)
{
   DXGI_FORMAT out =
      dzn_image_get_dxgi_format(pdev, format,
                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                aspect);

   switch (out) {
   case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
   case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
      return DXGI_FORMAT_R32_TYPELESS;
   case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
   case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
      return DXGI_FORMAT_R8_TYPELESS;
   default:
      return out;
   }
}

VkFormat
dzn_image_get_plane_format(VkFormat format,
                           VkImageAspectFlags aspectMask)
{
   if (aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT)
      return vk_format_stencil_only(format);
   else if (aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT)
      return vk_format_depth_only(format);
   else
      return format;
}

uint32_t
dzn_image_layers_get_subresource_index(const struct dzn_image *image,
                                       const VkImageSubresourceLayers *subres,
                                       VkImageAspectFlagBits aspect,
                                       uint32_t layer)
{
   int planeSlice =
      aspect == VK_IMAGE_ASPECT_STENCIL_BIT ? 1 : 0;

   return subres->mipLevel +
          ((subres->baseArrayLayer + layer) * image->desc.MipLevels) +
          (planeSlice * image->desc.MipLevels * image->desc.DepthOrArraySize);
}

uint32_t
dzn_image_range_get_subresource_index(const struct dzn_image *image,
                                      const VkImageSubresourceRange *subres,
                                      VkImageAspectFlagBits aspect,
                                      uint32_t level, uint32_t layer)
{
   int planeSlice =
      aspect == VK_IMAGE_ASPECT_STENCIL_BIT ? 1 : 0;

   return subres->baseMipLevel + level +
          ((subres->baseArrayLayer + layer) * image->desc.MipLevels) +
          (planeSlice * image->desc.MipLevels * image->desc.DepthOrArraySize);
}

static uint32_t
dzn_image_get_subresource_index(const struct dzn_image *image,
                                const VkImageSubresource *subres,
                                VkImageAspectFlagBits aspect)
{
   int planeSlice =
      aspect == VK_IMAGE_ASPECT_STENCIL_BIT ? 1 : 0;

   return subres->mipLevel +
          (subres->arrayLayer * image->desc.MipLevels) +
          (planeSlice * image->desc.MipLevels * image->desc.DepthOrArraySize);
}

D3D12_TEXTURE_COPY_LOCATION
dzn_image_get_copy_loc(const struct dzn_image *image,
                       const VkImageSubresourceLayers *subres,
                       VkImageAspectFlagBits aspect,
                       uint32_t layer)
{
   struct dzn_physical_device *pdev =
      container_of(image->vk.base.device->physical, struct dzn_physical_device, vk);
   D3D12_TEXTURE_COPY_LOCATION loc = {
      .pResource = image->res,
   };

   assert((subres->aspectMask & aspect) != 0);

   if (image->desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
      assert((subres->baseArrayLayer + layer) == 0);
      assert(subres->mipLevel == 0);
      enum pipe_format pfmt = vk_format_to_pipe_format(image->vk.format);
      uint32_t blkw = util_format_get_blockwidth(pfmt);
      uint32_t blkh = util_format_get_blockheight(pfmt);
      uint32_t blkd = util_format_get_blockdepth(pfmt);
      loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
      loc.PlacedFootprint.Offset = 0;
      loc.PlacedFootprint.Footprint.Format =
         dzn_image_get_placed_footprint_format(pdev, image->vk.format, aspect);
      loc.PlacedFootprint.Footprint.Width = ALIGN_POT(image->vk.extent.width, blkw);
      loc.PlacedFootprint.Footprint.Height = ALIGN_POT(image->vk.extent.height, blkh);
      loc.PlacedFootprint.Footprint.Depth = ALIGN_POT(image->vk.extent.depth, blkd);
      loc.PlacedFootprint.Footprint.RowPitch = image->linear.row_stride;
   } else {
      loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
      loc.SubresourceIndex = dzn_image_layers_get_subresource_index(image, subres, aspect, layer);
   }

   return loc;
}

D3D12_DEPTH_STENCIL_VIEW_DESC
dzn_image_get_dsv_desc(const struct dzn_image *image,
                       const VkImageSubresourceRange *range,
                       uint32_t level)
{
   struct dzn_physical_device *pdev =
      container_of(image->vk.base.device->physical, struct dzn_physical_device, vk);
   uint32_t layer_count = dzn_get_layer_count(image, range);
   D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
      .Format =
         dzn_image_get_dxgi_format(pdev, image->vk.format,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                   range->aspectMask),
   };

   switch (image->vk.image_type) {
   case VK_IMAGE_TYPE_1D:
      dsv_desc.ViewDimension =
         image->vk.array_layers > 1 ?
         D3D12_DSV_DIMENSION_TEXTURE1DARRAY :
         D3D12_DSV_DIMENSION_TEXTURE1D;
      break;
   case VK_IMAGE_TYPE_2D:
      if (image->vk.array_layers > 1) {
         dsv_desc.ViewDimension =
            image->vk.samples > 1 ?
            D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY :
            D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
      } else {
         dsv_desc.ViewDimension =
            image->vk.samples > 1 ?
            D3D12_DSV_DIMENSION_TEXTURE2DMS :
            D3D12_DSV_DIMENSION_TEXTURE2D;
      }
      break;
   default:
      unreachable("Invalid image type");
   }

   switch (dsv_desc.ViewDimension) {
   case D3D12_DSV_DIMENSION_TEXTURE1D:
      dsv_desc.Texture1D.MipSlice = range->baseMipLevel + level;
      break;
   case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
      dsv_desc.Texture1DArray.MipSlice = range->baseMipLevel + level;
      dsv_desc.Texture1DArray.FirstArraySlice = range->baseArrayLayer;
      dsv_desc.Texture1DArray.ArraySize = layer_count;
      break;
   case D3D12_DSV_DIMENSION_TEXTURE2D:
      dsv_desc.Texture2D.MipSlice = range->baseMipLevel + level;
      break;
   case D3D12_DSV_DIMENSION_TEXTURE2DMS:
      break;
   case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
      dsv_desc.Texture2DArray.MipSlice = range->baseMipLevel + level;
      dsv_desc.Texture2DArray.FirstArraySlice = range->baseArrayLayer;
      dsv_desc.Texture2DArray.ArraySize = layer_count;
      break;
   case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
      dsv_desc.Texture2DMSArray.FirstArraySlice = range->baseArrayLayer;
      dsv_desc.Texture2DMSArray.ArraySize = layer_count;
      break;
   default:
      unreachable("Invalid view dimension");
   }

   return dsv_desc;
}

D3D12_RENDER_TARGET_VIEW_DESC
dzn_image_get_rtv_desc(const struct dzn_image *image,
                       const VkImageSubresourceRange *range,
                       uint32_t level)
{
   struct dzn_physical_device *pdev =
      container_of(image->vk.base.device->physical, struct dzn_physical_device, vk);
   uint32_t layer_count = dzn_get_layer_count(image, range);
   D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {
      .Format =
         dzn_image_get_dxgi_format(pdev, image->vk.format,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                   VK_IMAGE_ASPECT_COLOR_BIT),
   };

   switch (image->vk.image_type) {
   case VK_IMAGE_TYPE_1D:
      rtv_desc.ViewDimension =
         image->vk.array_layers > 1 ?
         D3D12_RTV_DIMENSION_TEXTURE1DARRAY : D3D12_RTV_DIMENSION_TEXTURE1D;
      break;
   case VK_IMAGE_TYPE_2D:
      if (image->vk.array_layers > 1) {
         rtv_desc.ViewDimension =
            image->vk.samples > 1 ?
            D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY :
            D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      } else {
         rtv_desc.ViewDimension =
            image->vk.samples > 1 ?
            D3D12_RTV_DIMENSION_TEXTURE2DMS :
            D3D12_RTV_DIMENSION_TEXTURE2D;
      }
      break;
   case VK_IMAGE_TYPE_3D:
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
      break;
   default: unreachable("Invalid image type\n");
   }

   switch (rtv_desc.ViewDimension) {
   case D3D12_RTV_DIMENSION_TEXTURE1D:
      rtv_desc.Texture1D.MipSlice = range->baseMipLevel + level;
      break;
   case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
      rtv_desc.Texture1DArray.MipSlice = range->baseMipLevel + level;
      rtv_desc.Texture1DArray.FirstArraySlice = range->baseArrayLayer;
      rtv_desc.Texture1DArray.ArraySize = layer_count;
      break;
   case D3D12_RTV_DIMENSION_TEXTURE2D:
      rtv_desc.Texture2D.MipSlice = range->baseMipLevel + level;
      if (range->aspectMask & VK_IMAGE_ASPECT_PLANE_1_BIT)
         rtv_desc.Texture2D.PlaneSlice = 1;
      else if (range->aspectMask & VK_IMAGE_ASPECT_PLANE_2_BIT)
         rtv_desc.Texture2D.PlaneSlice = 2;
      else
         rtv_desc.Texture2D.PlaneSlice = 0;
      break;
   case D3D12_RTV_DIMENSION_TEXTURE2DMS:
      break;
   case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
      rtv_desc.Texture2DArray.MipSlice = range->baseMipLevel + level;
      rtv_desc.Texture2DArray.FirstArraySlice = range->baseArrayLayer;
      rtv_desc.Texture2DArray.ArraySize = layer_count;
      if (range->aspectMask & VK_IMAGE_ASPECT_PLANE_1_BIT)
         rtv_desc.Texture2DArray.PlaneSlice = 1;
      else if (range->aspectMask & VK_IMAGE_ASPECT_PLANE_2_BIT)
         rtv_desc.Texture2DArray.PlaneSlice = 2;
      else
         rtv_desc.Texture2DArray.PlaneSlice = 0;
      break;
   case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
      rtv_desc.Texture2DMSArray.FirstArraySlice = range->baseArrayLayer;
      rtv_desc.Texture2DMSArray.ArraySize = layer_count;
      break;
   case D3D12_RTV_DIMENSION_TEXTURE3D:
      rtv_desc.Texture3D.MipSlice = range->baseMipLevel + level;
      rtv_desc.Texture3D.FirstWSlice = range->baseArrayLayer;
      rtv_desc.Texture3D.WSize =
         range->layerCount == VK_REMAINING_ARRAY_LAYERS ? -1 : layer_count;
      break;
   default:
      unreachable("Invalid ViewDimension");
   }

   return rtv_desc;
}

D3D12_RESOURCE_STATES
dzn_image_layout_to_state(const struct dzn_image *image,
                          VkImageLayout layout,
                          VkImageAspectFlagBits aspect,
                          D3D12_COMMAND_LIST_TYPE type)
{
   D3D12_RESOURCE_STATES shaders_access =
      (image->desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) ?
      0 : (type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE :
           D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

   switch (layout) {
   case VK_IMAGE_LAYOUT_PREINITIALIZED:
   case VK_IMAGE_LAYOUT_UNDEFINED:
   case VK_IMAGE_LAYOUT_GENERAL:
      /* YOLO! */
   case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      return D3D12_RESOURCE_STATE_COMMON;

   case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      return D3D12_RESOURCE_STATE_COPY_DEST;

   case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      return D3D12_RESOURCE_STATE_COPY_SOURCE;

   case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      return type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
         D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_COMMON;

   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
      return D3D12_RESOURCE_STATE_DEPTH_WRITE;

   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
      return D3D12_RESOURCE_STATE_DEPTH_READ | shaders_access;

   case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
      return aspect == VK_IMAGE_ASPECT_STENCIL_BIT ?
             D3D12_RESOURCE_STATE_DEPTH_WRITE :
             (D3D12_RESOURCE_STATE_DEPTH_READ | shaders_access);

   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
      return aspect == VK_IMAGE_ASPECT_STENCIL_BIT ?
             (D3D12_RESOURCE_STATE_DEPTH_READ | shaders_access) :
             D3D12_RESOURCE_STATE_DEPTH_WRITE;

   case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      return shaders_access;

   case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
      return D3D12_RESOURCE_STATE_COMMON;

   default:
      unreachable("not implemented");
   }
}

D3D12_BARRIER_LAYOUT
dzn_vk_layout_to_d3d_layout(VkImageLayout layout,
                            D3D12_COMMAND_LIST_TYPE type,
                            VkImageAspectFlags aspect)
{
   if (type == D3D12_COMMAND_LIST_TYPE_COPY)
      return D3D12_BARRIER_LAYOUT_COMMON;

   switch (layout) {
   case VK_IMAGE_LAYOUT_UNDEFINED:
      return D3D12_BARRIER_LAYOUT_UNDEFINED;
   case VK_IMAGE_LAYOUT_PREINITIALIZED:
      return D3D12_BARRIER_LAYOUT_COMMON;
   case VK_IMAGE_LAYOUT_GENERAL:
   case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
   case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
      switch (type) {
      case D3D12_COMMAND_LIST_TYPE_DIRECT: return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON;
      case D3D12_COMMAND_LIST_TYPE_COMPUTE: return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON;
      default: return D3D12_BARRIER_LAYOUT_COMMON;
      }
   case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
   case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      switch (type) {
      case D3D12_COMMAND_LIST_TYPE_DIRECT: return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ;
      case D3D12_COMMAND_LIST_TYPE_COMPUTE: return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_GENERIC_READ;
      default: return D3D12_BARRIER_LAYOUT_GENERIC_READ;
      }
   case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
      return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
   case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
      return aspect == VK_IMAGE_ASPECT_DEPTH_BIT ?
         dzn_vk_layout_to_d3d_layout(VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, type, aspect) :
         D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
      return aspect == VK_IMAGE_ASPECT_DEPTH_BIT ?
         D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE :
         dzn_vk_layout_to_d3d_layout(VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, type, aspect);
   case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
      return aspect == VK_IMAGE_ASPECT_COLOR_BIT ?
         D3D12_BARRIER_LAYOUT_RENDER_TARGET : D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
   case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
   case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
      return D3D12_BARRIER_LAYOUT_PRESENT;
   case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
      return D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE;
   default:
      assert(!"Unexpected layout");
      return D3D12_BARRIER_LAYOUT_COMMON;
   }
}

bool
dzn_image_formats_are_compatible(const struct dzn_device *device,
                                 VkFormat orig_fmt, VkFormat new_fmt,
                                 VkImageUsageFlags usage,
                                 VkImageAspectFlagBits aspect)
{
   const struct dzn_physical_device *pdev =
      container_of(device->vk.physical, struct dzn_physical_device, vk);
   DXGI_FORMAT orig_dxgi = dzn_image_get_dxgi_format(pdev, orig_fmt, usage, aspect);
   DXGI_FORMAT new_dxgi = dzn_image_get_dxgi_format(pdev, new_fmt, usage, aspect);

   if (orig_dxgi == new_dxgi)
      return true;

   DXGI_FORMAT typeless_orig = dzn_get_typeless_dxgi_format(orig_dxgi);
   DXGI_FORMAT typeless_new = dzn_get_typeless_dxgi_format(new_dxgi);

   if (!(usage & VK_IMAGE_USAGE_SAMPLED_BIT))
      return typeless_orig == typeless_new;

   if (pdev->options3.CastingFullyTypedFormatSupported) {
      enum pipe_format orig_pfmt = vk_format_to_pipe_format(orig_fmt);
      enum pipe_format new_pfmt = vk_format_to_pipe_format(new_fmt);

      /* Types don't belong to the same group, they're incompatible. */
      if (typeless_orig != typeless_new)
         return false;

      /* FLOAT <-> non-FLOAT casting is disallowed. */
      if (util_format_is_float(orig_pfmt) != util_format_is_float(new_pfmt))
         return false;

      /* UNORM <-> SNORM casting is disallowed. */
      bool orig_is_norm =
         util_format_is_unorm(orig_pfmt) || util_format_is_snorm(orig_pfmt);
      bool new_is_norm =
         util_format_is_unorm(new_pfmt) || util_format_is_snorm(new_pfmt);
      if (orig_is_norm && new_is_norm &&
          util_format_is_unorm(orig_pfmt) != util_format_is_unorm(new_pfmt))
         return false;

      return true;
   }

   return false;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_CreateImage(VkDevice device,
                const VkImageCreateInfo *pCreateInfo,
                const VkAllocationCallbacks *pAllocator,
                VkImage *pImage)
{
   return dzn_image_create(dzn_device_from_handle(device),
                           pCreateInfo, pAllocator, pImage);
}

VKAPI_ATTR void VKAPI_CALL
dzn_DestroyImage(VkDevice device, VkImage image,
                 const VkAllocationCallbacks *pAllocator)
{
   dzn_image_destroy(dzn_image_from_handle(image), pAllocator);
}

static struct dzn_image *
dzn_swapchain_get_image(struct dzn_device *device,
                        VkSwapchainKHR swapchain,
                        uint32_t index)
{
   uint32_t n_images = index + 1;
   STACK_ARRAY(VkImage, images, n_images);
   struct dzn_image *image = NULL;

   VkResult result = wsi_common_get_images(swapchain, &n_images, images);

   if (result == VK_SUCCESS || result == VK_INCOMPLETE)
      image = dzn_image_from_handle(images[index]);

   STACK_ARRAY_FINISH(images);
   return image;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_BindImageMemory2(VkDevice dev,
                     uint32_t bindInfoCount,
                     const VkBindImageMemoryInfo *pBindInfos)
{
   VK_FROM_HANDLE(dzn_device, device, dev);

   for (uint32_t i = 0; i < bindInfoCount; i++) {
      const VkBindImageMemoryInfo *bind_info = &pBindInfos[i];
      VK_FROM_HANDLE(dzn_device_memory, mem, bind_info->memory);
      VK_FROM_HANDLE(dzn_image, image, bind_info->image);

      vk_foreach_struct_const(s, bind_info->pNext) {
         dzn_debug_ignored_stype(s->sType);
      }

      image->mem = mem;

      HRESULT hres = S_OK;

      if (mem->dedicated_res) {
         assert(pBindInfos[i].memoryOffset == 0);
         image->res = mem->dedicated_res;
         ID3D12Resource_AddRef(image->res);
      } else if (device->dev10 && image->castable_format_count > 0) {
         D3D12_RESOURCE_DESC1 desc = {
            .Dimension = image->desc.Dimension,
            .Alignment = image->desc.Alignment,
            .Width = image->desc.Width,
            .Height = image->desc.Height,
            .DepthOrArraySize = image->desc.DepthOrArraySize,
            .MipLevels = image->desc.MipLevels,
            .Format = image->desc.Format,
            .SampleDesc = image->desc.SampleDesc,
            .Layout = image->desc.Layout,
            .Flags = image->desc.Flags | mem->res_flags,
         };

         hres = ID3D12Device10_CreatePlacedResource2(device->dev10, mem->heap,
                                                     bind_info->memoryOffset,
                                                     &desc,
                                                     D3D12_BARRIER_LAYOUT_COMMON,
                                                     NULL,
                                                     image->castable_format_count,
                                                     image->castable_formats,
                                                     &IID_ID3D12Resource,
                                                     (void **)&image->res);
      } else {
         D3D12_RESOURCE_DESC desc = image->desc;
         desc.Flags |= mem->res_flags;
         hres = ID3D12Device1_CreatePlacedResource(device->dev, mem->heap,
                                                   bind_info->memoryOffset,
                                                   &desc,
                                                   D3D12_RESOURCE_STATE_COMMON,
                                                   NULL,
                                                   &IID_ID3D12Resource,
                                                   (void **)&image->res);
      }
      if (FAILED(hres))
         return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   }

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetImageMemoryRequirements2(VkDevice _device,
                                const VkImageMemoryRequirementsInfo2 *pInfo,
                                VkMemoryRequirements2 *pMemoryRequirements)
{
   VK_FROM_HANDLE(dzn_device, device, _device);
   VK_FROM_HANDLE(dzn_image, image, pInfo->image);
   struct dzn_physical_device *pdev =
      container_of(device->vk.physical, struct dzn_physical_device, vk);

   vk_foreach_struct_const(ext, pInfo->pNext) {
      dzn_debug_ignored_stype(ext->sType);
   }

   vk_foreach_struct(ext, pMemoryRequirements->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS: {
         VkMemoryDedicatedRequirements *requirements =
            (VkMemoryDedicatedRequirements *)ext;
         requirements->requiresDedicatedAllocation = image->vk.external_handle_types != 0;
         requirements->prefersDedicatedAllocation = requirements->requiresDedicatedAllocation ||
            image->vk.tiling == VK_IMAGE_TILING_OPTIMAL;
         break;
      }

      default:
         dzn_debug_ignored_stype(ext->sType);
         break;
      }
   }

   D3D12_RESOURCE_ALLOCATION_INFO info;
   if (device->dev12 && image->castable_format_count > 0) {
      D3D12_RESOURCE_DESC1 desc1;
      memcpy(&desc1, &image->desc, sizeof(image->desc));
      memset(&desc1.SamplerFeedbackMipRegion, 0, sizeof(desc1.SamplerFeedbackMipRegion));
      info = dzn_ID3D12Device12_GetResourceAllocationInfo3(device->dev12, 0, 1, &desc1,
                                                           &image->castable_format_count,
                                                           (const DXGI_FORMAT *const *) &image->castable_formats,
                                                           NULL);
   } else {
      info = dzn_ID3D12Device4_GetResourceAllocationInfo(device->dev, 0, 1, &image->desc);
   }

   pMemoryRequirements->memoryRequirements = (VkMemoryRequirements) {
      .size = info.SizeInBytes,
      .alignment = info.Alignment,
      .memoryTypeBits =
         dzn_physical_device_get_mem_type_mask_for_resource(pdev, &image->desc,
                                                            image->vk.external_handle_types != 0),
   };

   /*
    * MSAA images need memory to be aligned on
    * D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT (4MB), but the memory
    * allocation function doesn't know what the memory will be used for,
    * and forcing all allocations to be 4MB-aligned has a cost, so let's
    * force MSAA resources to be at least 4MB, such that the allocation
    * logic can consider sub-4MB allocations to not require this 4MB alignment.
    */
   if (image->vk.samples > 1 &&
       pMemoryRequirements->memoryRequirements.size < D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT)
      pMemoryRequirements->memoryRequirements.size = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetImageSubresourceLayout(VkDevice _device,
                              VkImage _image,
                              const VkImageSubresource *subresource,
                              VkSubresourceLayout *layout)
{
   VK_FROM_HANDLE(dzn_device, device, _device);
   VK_FROM_HANDLE(dzn_image, image, _image);

   if (image->desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
      assert(subresource->arrayLayer == 0);
      assert(subresource->mipLevel == 0);
      layout->offset = 0;
      layout->rowPitch = image->linear.row_stride;
      layout->depthPitch = 0;
      layout->arrayPitch = 0;
      layout->size = image->linear.size;
   } else {
      UINT subres_index =
         dzn_image_get_subresource_index(image, subresource,
                                         (VkImageAspectFlagBits)subresource->aspectMask);
      D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
      UINT num_rows;
      UINT64 row_size, total_size;
      ID3D12Device1_GetCopyableFootprints(device->dev, &image->desc,
                                         subres_index, 1,
                                         0, // base-offset?
                                         &footprint,
                                         &num_rows, &row_size,
                                         &total_size);

      layout->offset = footprint.Offset;
      layout->rowPitch = footprint.Footprint.RowPitch;
      layout->depthPitch = layout->rowPitch * footprint.Footprint.Height;
      layout->arrayPitch = layout->depthPitch; // uuuh... why is this even here?
      layout->size = total_size;
   }
}

static D3D12_SHADER_COMPONENT_MAPPING
translate_swizzle(VkComponentSwizzle in, uint32_t comp)
{
   switch (in) {
   case VK_COMPONENT_SWIZZLE_IDENTITY:
      return (D3D12_SHADER_COMPONENT_MAPPING)
             (comp + D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0);
   case VK_COMPONENT_SWIZZLE_ZERO:
      return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0;
   case VK_COMPONENT_SWIZZLE_ONE:
      return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1;
   case VK_COMPONENT_SWIZZLE_R:
      return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;
   case VK_COMPONENT_SWIZZLE_G:
      return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1;
   case VK_COMPONENT_SWIZZLE_B:
      return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2;
   case VK_COMPONENT_SWIZZLE_A:
      return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3;
   default: unreachable("Invalid swizzle");
   }
}

static void
dzn_image_view_prepare_srv_desc(struct dzn_image_view *iview)
{
   struct dzn_physical_device *pdev =
      container_of(iview->vk.base.device->physical, struct dzn_physical_device, vk);
   uint32_t plane_slice = (iview->vk.aspects & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) ==
      VK_IMAGE_ASPECT_STENCIL_BIT ? 1 : 0;
   bool ms = iview->vk.image->samples > 1;
   uint32_t layers_per_elem =
      (iview->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE ||
       iview->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) ?
      6 : 1;
   bool from_3d_image = iview->vk.image->image_type == VK_IMAGE_TYPE_3D;
   bool use_array = iview->vk.base_array_layer > 0 ||
                    (iview->vk.layer_count / layers_per_elem) > 1;

   iview->srv_desc = (D3D12_SHADER_RESOURCE_VIEW_DESC) {
      .Format =
         dzn_image_get_dxgi_format(pdev, iview->vk.format,
                                   iview->vk.usage & ~VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                   iview->vk.aspects),
   };

   D3D12_SHADER_COMPONENT_MAPPING swz[] = {
      translate_swizzle(iview->vk.swizzle.r, 0),
      translate_swizzle(iview->vk.swizzle.g, 1),
      translate_swizzle(iview->vk.swizzle.b, 2),
      translate_swizzle(iview->vk.swizzle.a, 3),
   };

   /* Swap components to fake B4G4R4A4 support. */
   if (iview->vk.format == VK_FORMAT_B4G4R4A4_UNORM_PACK16) {
      if (pdev->support_a4b4g4r4) {
         static const D3D12_SHADER_COMPONENT_MAPPING bgra4_remap[] = {
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2,
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1,
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0,
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3,
            D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0,
            D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1,
         };

         for (uint32_t i = 0; i < ARRAY_SIZE(swz); i++)
            swz[i] = bgra4_remap[swz[i]];
      } else {
         static const D3D12_SHADER_COMPONENT_MAPPING bgra4_remap[] = {
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1,
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0,
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3,
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2,
            D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0,
            D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1,
         };

         for (uint32_t i = 0; i < ARRAY_SIZE(swz); i++)
            swz[i] = bgra4_remap[swz[i]];
      }
   } else if (iview->vk.aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      /* D3D puts stencil in G, not R. Requests for R should be routed to G and vice versa. */
      for (uint32_t i = 0; i < ARRAY_SIZE(swz); i++) {
         if (swz[i] == D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0)
            swz[i] = D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1;
         else if (swz[i] == D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1)
            swz[i] = D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;
      }
   } else if (iview->vk.view_format == VK_FORMAT_BC1_RGB_SRGB_BLOCK ||
              iview->vk.view_format == VK_FORMAT_BC1_RGB_UNORM_BLOCK) {
      /* D3D has no opaque version of these; force alpha to 1 */
      for (uint32_t i = 0; i < ARRAY_SIZE(swz); i++) {
         if (swz[i] == D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3)
            swz[i] = D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1;
      }
   }

   iview->srv_desc.Shader4ComponentMapping =
      D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(swz[0], swz[1], swz[2], swz[3]);

   switch (iview->vk.view_type) {
   case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
   case VK_IMAGE_VIEW_TYPE_1D:
      if (use_array) {
         iview->srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
         iview->srv_desc.Texture1DArray.MostDetailedMip = iview->vk.base_mip_level;
         iview->srv_desc.Texture1DArray.MipLevels = iview->vk.level_count;
         iview->srv_desc.Texture1DArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->srv_desc.Texture1DArray.ArraySize = iview->vk.layer_count;
         iview->srv_desc.Texture1DArray.ResourceMinLODClamp = 0.0f;
      } else {
         iview->srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
         iview->srv_desc.Texture1D.MostDetailedMip = iview->vk.base_mip_level;
         iview->srv_desc.Texture1D.MipLevels = iview->vk.level_count;
         iview->srv_desc.Texture1D.ResourceMinLODClamp = 0.0f;
      }
      break;

   case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
   case VK_IMAGE_VIEW_TYPE_2D:
      if (from_3d_image) {
         iview->srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
         iview->srv_desc.Texture3D.MostDetailedMip = iview->vk.base_mip_level;
         iview->srv_desc.Texture3D.MipLevels = iview->vk.level_count;
         iview->srv_desc.Texture3D.ResourceMinLODClamp = 0.0f;
      } else if (use_array && ms) {
         iview->srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
         iview->srv_desc.Texture2DMSArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->srv_desc.Texture2DMSArray.ArraySize = iview->vk.layer_count;
      } else if (use_array && !ms) {
         iview->srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
         iview->srv_desc.Texture2DArray.MostDetailedMip = iview->vk.base_mip_level;
         iview->srv_desc.Texture2DArray.MipLevels = iview->vk.level_count;
         iview->srv_desc.Texture2DArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->srv_desc.Texture2DArray.ArraySize = iview->vk.layer_count;
         iview->srv_desc.Texture2DArray.PlaneSlice = plane_slice;
         iview->srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
      } else if (!use_array && ms) {
         iview->srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
      } else {
         iview->srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
         iview->srv_desc.Texture2D.MostDetailedMip = iview->vk.base_mip_level;
         iview->srv_desc.Texture2D.MipLevels = iview->vk.level_count;
         iview->srv_desc.Texture2D.PlaneSlice = plane_slice;
         iview->srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
      }
      break;

   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
   case VK_IMAGE_VIEW_TYPE_CUBE:
      if (use_array) {
         iview->srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
         iview->srv_desc.TextureCubeArray.MostDetailedMip = iview->vk.base_mip_level;
         iview->srv_desc.TextureCubeArray.MipLevels = iview->vk.level_count;
         iview->srv_desc.TextureCubeArray.First2DArrayFace = iview->vk.base_array_layer;
         iview->srv_desc.TextureCubeArray.NumCubes = iview->vk.layer_count / 6;
         iview->srv_desc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
      } else {
         iview->srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
         iview->srv_desc.TextureCube.MostDetailedMip = iview->vk.base_mip_level;
         iview->srv_desc.TextureCube.MipLevels = iview->vk.level_count;
         iview->srv_desc.TextureCube.ResourceMinLODClamp = 0.0f;
      }
      break;

   case VK_IMAGE_VIEW_TYPE_3D:
      iview->srv_desc.ViewDimension =  D3D12_SRV_DIMENSION_TEXTURE3D;
      iview->srv_desc.Texture3D.MostDetailedMip = iview->vk.base_mip_level;
      iview->srv_desc.Texture3D.MipLevels = iview->vk.level_count;
      iview->srv_desc.Texture3D.ResourceMinLODClamp = 0.0f;
      break;

   default: unreachable("Invalid view type");
   }
}

static void
dzn_image_view_prepare_uav_desc(struct dzn_image_view *iview)
{
   struct dzn_physical_device *pdev =
      container_of(iview->vk.base.device->physical, struct dzn_physical_device, vk);
   bool use_array = iview->vk.base_array_layer > 0 || iview->vk.layer_count > 1;

   assert(iview->vk.image->samples == 1);

   iview->uav_desc = (D3D12_UNORDERED_ACCESS_VIEW_DESC) {
      .Format =
         dzn_image_get_dxgi_format(pdev, iview->vk.format,
                                   VK_IMAGE_USAGE_STORAGE_BIT,
                                   iview->vk.aspects),
   };

   switch (iview->vk.view_type) {
   case VK_IMAGE_VIEW_TYPE_1D:
   case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
      if (use_array) {
         iview->uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
         iview->uav_desc.Texture1DArray.MipSlice = iview->vk.base_mip_level;
         iview->uav_desc.Texture1DArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->uav_desc.Texture1DArray.ArraySize = iview->vk.layer_count;
      } else {
         iview->uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
         iview->uav_desc.Texture1D.MipSlice = iview->vk.base_mip_level;
      }
      break;

   case VK_IMAGE_VIEW_TYPE_2D:
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
   case VK_IMAGE_VIEW_TYPE_CUBE:
   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
      if (use_array) {
         iview->uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
         iview->uav_desc.Texture2DArray.PlaneSlice = 0;
         iview->uav_desc.Texture2DArray.MipSlice = iview->vk.base_mip_level;
         iview->uav_desc.Texture2DArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->uav_desc.Texture2DArray.ArraySize = iview->vk.layer_count;
      } else {
         iview->uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
         iview->uav_desc.Texture2D.MipSlice = iview->vk.base_mip_level;
         iview->uav_desc.Texture2D.PlaneSlice = 0;
      }
      break;
   case VK_IMAGE_VIEW_TYPE_3D:
      iview->uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
      iview->uav_desc.Texture3D.MipSlice = iview->vk.base_mip_level;
      iview->uav_desc.Texture3D.FirstWSlice = 0;
      iview->uav_desc.Texture3D.WSize = iview->vk.extent.depth;
      break;
   default: unreachable("Invalid type");
   }
}

static void
dzn_image_view_prepare_rtv_desc(struct dzn_image_view *iview)
{
   struct dzn_physical_device *pdev =
      container_of(iview->vk.base.device->physical, struct dzn_physical_device, vk);
   bool use_array = iview->vk.base_array_layer > 0 || iview->vk.layer_count > 1;
   bool from_3d_image = iview->vk.image->image_type == VK_IMAGE_TYPE_3D;
   bool ms = iview->vk.image->samples > 1;
   uint32_t plane_slice =
      (iview->vk.aspects & VK_IMAGE_ASPECT_PLANE_2_BIT) ? 2 :
      (iview->vk.aspects & VK_IMAGE_ASPECT_PLANE_1_BIT) ? 1 : 0;

   iview->rtv_desc = (D3D12_RENDER_TARGET_VIEW_DESC) {
      .Format =
         dzn_image_get_dxgi_format(pdev, iview->vk.format,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                   iview->vk.aspects),
   };

   switch (iview->vk.view_type) {
   case VK_IMAGE_VIEW_TYPE_1D:
   case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
      if (use_array) {
         iview->rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
         iview->rtv_desc.Texture1DArray.MipSlice = iview->vk.base_mip_level;
         iview->rtv_desc.Texture1DArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->rtv_desc.Texture1DArray.ArraySize = iview->vk.layer_count;
      } else {
         iview->rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
         iview->rtv_desc.Texture1D.MipSlice = iview->vk.base_mip_level;
      }
      break;

   case VK_IMAGE_VIEW_TYPE_2D:
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
   case VK_IMAGE_VIEW_TYPE_CUBE:
   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
      if (from_3d_image) {
         iview->rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
         iview->rtv_desc.Texture3D.MipSlice = iview->vk.base_mip_level;
         iview->rtv_desc.Texture3D.FirstWSlice = iview->vk.base_array_layer;
         iview->rtv_desc.Texture3D.WSize = iview->vk.layer_count;
      } else if (use_array && ms) {
         iview->rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
         iview->rtv_desc.Texture2DMSArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->rtv_desc.Texture2DMSArray.ArraySize = iview->vk.layer_count;
      } else if (use_array && !ms) {
         iview->rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
         iview->rtv_desc.Texture2DArray.MipSlice = iview->vk.base_mip_level;
         iview->rtv_desc.Texture2DArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->rtv_desc.Texture2DArray.ArraySize = iview->vk.layer_count;
         iview->rtv_desc.Texture2DArray.PlaneSlice = plane_slice;
      } else if (!use_array && ms) {
         iview->rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
      } else {
         iview->rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
         iview->rtv_desc.Texture2D.MipSlice = iview->vk.base_mip_level;
         iview->rtv_desc.Texture2D.PlaneSlice = plane_slice;
      }
      break;

   case VK_IMAGE_VIEW_TYPE_3D:
      iview->rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
      iview->rtv_desc.Texture3D.MipSlice = iview->vk.base_mip_level;
      iview->rtv_desc.Texture3D.FirstWSlice = 0;
      iview->rtv_desc.Texture3D.WSize = iview->vk.extent.depth;
      break;

   default: unreachable("Invalid view type");
   }
}

static void
dzn_image_view_prepare_dsv_desc(struct dzn_image_view *iview)
{
   struct dzn_physical_device *pdev =
      container_of(iview->vk.base.device->physical, struct dzn_physical_device, vk);
   bool use_array = iview->vk.base_array_layer > 0 || iview->vk.layer_count > 1;
   bool ms = iview->vk.image->samples > 1;

   iview->dsv_desc = (D3D12_DEPTH_STENCIL_VIEW_DESC) {
      .Format =
         dzn_image_get_dxgi_format(pdev, iview->vk.format,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                   iview->vk.aspects),
   };

   switch (iview->vk.view_type) {
   case VK_IMAGE_VIEW_TYPE_1D:
   case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
      if (use_array) {
         iview->dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
         iview->dsv_desc.Texture1DArray.MipSlice = iview->vk.base_mip_level;
         iview->dsv_desc.Texture1DArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->dsv_desc.Texture1DArray.ArraySize = iview->vk.layer_count;
      } else {
         iview->dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
         iview->dsv_desc.Texture1D.MipSlice = iview->vk.base_mip_level;
      }
      break;

   case VK_IMAGE_VIEW_TYPE_2D:
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
   case VK_IMAGE_VIEW_TYPE_CUBE:
   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
      if (use_array && ms) {
         iview->dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
         iview->dsv_desc.Texture2DMSArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->dsv_desc.Texture2DMSArray.ArraySize = iview->vk.layer_count;
      } else if (use_array && !ms) {
         iview->dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
         iview->dsv_desc.Texture2DArray.MipSlice = iview->vk.base_mip_level;
         iview->dsv_desc.Texture2DArray.FirstArraySlice = iview->vk.base_array_layer;
         iview->dsv_desc.Texture2DArray.ArraySize = iview->vk.layer_count;
      } else if (!use_array && ms) {
         iview->dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
      } else {
         iview->dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
         iview->dsv_desc.Texture2D.MipSlice = iview->vk.base_mip_level;
      }
      break;

   default: unreachable("Invalid view type");
   }
}

void
dzn_image_view_finish(struct dzn_image_view *iview)
{
   vk_image_view_finish(&iview->vk);
}

void
dzn_image_view_init(struct dzn_device *device,
                    struct dzn_image_view *iview,
                    const VkImageViewCreateInfo *pCreateInfo)
{
   VK_FROM_HANDLE(dzn_image, image, pCreateInfo->image);

   const VkImageSubresourceRange *range = &pCreateInfo->subresourceRange;
   ASSERTED uint32_t layer_count = dzn_get_layer_count(image, range);

   vk_image_view_init(&device->vk, &iview->vk, false, pCreateInfo);

   assert(layer_count > 0);
   assert(range->baseMipLevel < image->vk.mip_levels);

   /* View usage should be a subset of image usage */
   assert(iview->vk.usage & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                             VK_IMAGE_USAGE_SAMPLED_BIT |
                             VK_IMAGE_USAGE_STORAGE_BIT |
                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));

   /* We remove this bit on depth textures, so skip creating a UAV for those */
   if ((iview->vk.usage & VK_IMAGE_USAGE_STORAGE_BIT) &&
       !(image->desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
      iview->vk.usage &= ~VK_IMAGE_USAGE_STORAGE_BIT;

   switch (image->vk.image_type) {
   default:
      unreachable("bad VkImageType");
   case VK_IMAGE_TYPE_1D:
   case VK_IMAGE_TYPE_2D:
      assert(range->baseArrayLayer + dzn_get_layer_count(image, range) - 1 <= image->vk.array_layers);
      break;
   case VK_IMAGE_TYPE_3D:
      assert(range->baseArrayLayer + dzn_get_layer_count(image, range) - 1
             <= u_minify(image->vk.extent.depth, range->baseMipLevel));
      break;
   }

   dzn_image_view_prepare_srv_desc(iview);

   if (iview->vk.usage & VK_IMAGE_USAGE_STORAGE_BIT)
      dzn_image_view_prepare_uav_desc(iview);

   if (iview->vk.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
      dzn_image_view_prepare_rtv_desc(iview);

   if (iview->vk.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      dzn_image_view_prepare_dsv_desc(iview);
}

static void
dzn_image_view_destroy(struct dzn_image_view *iview,
                      const VkAllocationCallbacks *pAllocator)
{
   if (!iview)
      return;

   struct dzn_device *device = container_of(iview->vk.base.device, struct dzn_device, vk);

   dzn_device_descriptor_heap_free_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, iview->srv_bindless_slot);
   dzn_device_descriptor_heap_free_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, iview->uav_bindless_slot);

   vk_image_view_finish(&iview->vk);
   vk_free2(&device->vk.alloc, pAllocator, iview);
}

static VkResult
dzn_image_view_create(struct dzn_device *device,
                      const VkImageViewCreateInfo *pCreateInfo,
                      const VkAllocationCallbacks *pAllocator,
                      VkImageView *out)
{
   VK_FROM_HANDLE(dzn_image, image, pCreateInfo->image);
   struct dzn_image_view *iview =
      vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*iview), 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!iview)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   dzn_image_view_init(device, iview, pCreateInfo);

   iview->srv_bindless_slot = iview->uav_bindless_slot = -1;
   if (device->bindless) {
      if (!(image->desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
         iview->srv_bindless_slot = dzn_device_descriptor_heap_alloc_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
         if (iview->srv_bindless_slot < 0) {
            dzn_image_view_destroy(iview, pAllocator);
            return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         }

         dzn_descriptor_heap_write_image_view_desc(device,
                                                   &device->device_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].heap,
                                                   iview->srv_bindless_slot,
                                                   false, false,
                                                   iview);
      }
      if (iview->vk.usage & VK_IMAGE_USAGE_STORAGE_BIT) {
         iview->uav_bindless_slot = dzn_device_descriptor_heap_alloc_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
         if (iview->uav_bindless_slot < 0) {
            dzn_image_view_destroy(iview, pAllocator);
            return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         }

         dzn_descriptor_heap_write_image_view_desc(device,
                                                   &device->device_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].heap,
                                                   iview->uav_bindless_slot,
                                                   true, true,
                                                   iview);
      }
   }

   *out = dzn_image_view_to_handle(iview);
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_CreateImageView(VkDevice device,
                    const VkImageViewCreateInfo *pCreateInfo,
                    const VkAllocationCallbacks *pAllocator,
                    VkImageView *pView)
{
   return dzn_image_view_create(dzn_device_from_handle(device), pCreateInfo,
                                pAllocator, pView);
}

VKAPI_ATTR void VKAPI_CALL
dzn_DestroyImageView(VkDevice device,
                     VkImageView imageView,
                     const VkAllocationCallbacks *pAllocator)
{
   dzn_image_view_destroy(dzn_image_view_from_handle(imageView), pAllocator);
}

static void
dzn_buffer_view_destroy(struct dzn_buffer_view *bview,
                        const VkAllocationCallbacks *pAllocator)
{
   if (!bview)
      return;

   struct dzn_device *device = container_of(bview->base.device, struct dzn_device, vk);

   dzn_device_descriptor_heap_free_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, bview->srv_bindless_slot);
   dzn_device_descriptor_heap_free_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, bview->uav_bindless_slot);

   vk_object_base_finish(&bview->base);
   vk_free2(&device->vk.alloc, pAllocator, bview);
}

static VkResult
dzn_buffer_view_create(struct dzn_device *device,
                       const VkBufferViewCreateInfo *pCreateInfo,
                       const VkAllocationCallbacks *pAllocator,
                       VkBufferView *out)
{
   VK_FROM_HANDLE(dzn_buffer, buf, pCreateInfo->buffer);

   struct dzn_buffer_view *bview =
      vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*bview), 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!bview)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &bview->base, VK_OBJECT_TYPE_BUFFER_VIEW);

   enum pipe_format pfmt = vk_format_to_pipe_format(pCreateInfo->format);
   unsigned blksz = util_format_get_blocksize(pfmt);
   VkDeviceSize size =
      pCreateInfo->range == VK_WHOLE_SIZE ?
      buf->size - pCreateInfo->offset : pCreateInfo->range;

   bview->buffer = buf;
   bview->srv_bindless_slot = bview->uav_bindless_slot = -1;
   if (buf->usage &
       (VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)) {
      bview->srv_desc = (D3D12_SHADER_RESOURCE_VIEW_DESC) {
         .Format = dzn_buffer_get_dxgi_format(pCreateInfo->format),
         .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
         .Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
         .Buffer = {
            .FirstElement = pCreateInfo->offset / blksz,
            .NumElements = (UINT)(size / blksz),
            .Flags = D3D12_BUFFER_SRV_FLAG_NONE,
         },
      };

      if (device->bindless) {
         bview->srv_bindless_slot = dzn_device_descriptor_heap_alloc_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
         if (bview->srv_bindless_slot < 0) {
            dzn_buffer_view_destroy(bview, pAllocator);
            return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         }
         dzn_descriptor_heap_write_buffer_view_desc(device, &device->device_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].heap,
                                                    bview->srv_bindless_slot, false, bview);
      }
   }

   if (buf->usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
      bview->uav_desc = (D3D12_UNORDERED_ACCESS_VIEW_DESC) {
         .Format = dzn_buffer_get_dxgi_format(pCreateInfo->format),
         .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
         .Buffer = {
            .FirstElement = pCreateInfo->offset / blksz,
            .NumElements = (UINT)(size / blksz),
            .Flags = D3D12_BUFFER_UAV_FLAG_NONE,
         },
      };

      if (device->bindless) {
         bview->uav_bindless_slot = dzn_device_descriptor_heap_alloc_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
         if (bview->uav_bindless_slot < 0) {
            dzn_buffer_view_destroy(bview, pAllocator);
            return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         }
         dzn_descriptor_heap_write_buffer_view_desc(device, &device->device_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].heap,
                                                    bview->uav_bindless_slot, true, bview);
      }
   }

   *out = dzn_buffer_view_to_handle(bview);
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_CreateBufferView(VkDevice device,
                     const VkBufferViewCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *pAllocator,
                     VkBufferView *pView)
{
   return dzn_buffer_view_create(dzn_device_from_handle(device),
                                 pCreateInfo, pAllocator, pView);
}

VKAPI_ATTR void VKAPI_CALL
dzn_DestroyBufferView(VkDevice device,
                      VkBufferView bufferView,
                      const VkAllocationCallbacks *pAllocator)
{
   dzn_buffer_view_destroy(dzn_buffer_view_from_handle(bufferView), pAllocator);
}
