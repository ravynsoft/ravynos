/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "util/u_atomic.h"
#include "util/u_debug.h"
#include "vulkan/util/vk_format.h"
#include "ac_drm_fourcc.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "radv_radeon_winsys.h"
#include "sid.h"
#include "vk_format.h"
#include "vk_render_pass.h"
#include "vk_util.h"

#include "gfx10_format_table.h"

static unsigned
radv_choose_tiling(struct radv_device *device, const VkImageCreateInfo *pCreateInfo, VkFormat format)
{
   if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR) {
      assert(pCreateInfo->samples <= 1);
      return RADEON_SURF_MODE_LINEAR_ALIGNED;
   }

   if (pCreateInfo->usage & (VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR))
      return RADEON_SURF_MODE_LINEAR_ALIGNED;

   /* MSAA resources must be 2D tiled. */
   if (pCreateInfo->samples > 1)
      return RADEON_SURF_MODE_2D;

   if (!vk_format_is_compressed(format) && !vk_format_is_depth_or_stencil(format) &&
       device->physical_device->rad_info.gfx_level <= GFX8) {
      /* this causes hangs in some VK CTS tests on GFX9. */
      /* Textures with a very small height are recommended to be linear. */
      if (pCreateInfo->imageType == VK_IMAGE_TYPE_1D ||
          /* Only very thin and long 2D textures should benefit from
           * linear_aligned. */
          (pCreateInfo->extent.width > 8 && pCreateInfo->extent.height <= 2))
         return RADEON_SURF_MODE_LINEAR_ALIGNED;
   }

   return RADEON_SURF_MODE_2D;
}

static bool
radv_use_tc_compat_htile_for_image(struct radv_device *device, const VkImageCreateInfo *pCreateInfo, VkFormat format)
{
   /* TC-compat HTILE is only available for GFX8+. */
   if (device->physical_device->rad_info.gfx_level < GFX8)
      return false;

   /* TC-compat HTILE looks broken on Tonga (and Iceland is the same design) and the documented bug
    * workarounds don't help.
    */
   if (device->physical_device->rad_info.family == CHIP_TONGA ||
       device->physical_device->rad_info.family == CHIP_ICELAND)
      return false;

   if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR)
      return false;

   /* Do not enable TC-compatible HTILE if the image isn't readable by a
    * shader because no texture fetches will happen.
    */
   if (!(pCreateInfo->usage &
         (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)))
      return false;

   if (device->physical_device->rad_info.gfx_level < GFX9) {
      /* TC-compat HTILE for MSAA depth/stencil images is broken
       * on GFX8 because the tiling doesn't match.
       */
      if (pCreateInfo->samples >= 2 && format == VK_FORMAT_D32_SFLOAT_S8_UINT)
         return false;

      /* GFX9+ supports compression for both 32-bit and 16-bit depth
       * surfaces, while GFX8 only supports 32-bit natively. Though,
       * the driver allows TC-compat HTILE for 16-bit depth surfaces
       * with no Z planes compression.
       */
      if (format != VK_FORMAT_D32_SFLOAT_S8_UINT && format != VK_FORMAT_D32_SFLOAT && format != VK_FORMAT_D16_UNORM)
         return false;

      /* TC-compat HTILE for layered images can have interleaved slices (see sliceInterleaved flag
       * in addrlib).  radv_clear_htile does not work.
       */
      if (pCreateInfo->arrayLayers > 1)
         return false;
   }

   /* GFX9 has issues when the sample count is 4 and the format is D16 */
   if (device->physical_device->rad_info.gfx_level == GFX9 && pCreateInfo->samples == 4 &&
       format == VK_FORMAT_D16_UNORM)
      return false;

   return true;
}

static bool
radv_surface_has_scanout(struct radv_device *device, const struct radv_image_create_info *info)
{
   if (info->bo_metadata) {
      if (device->physical_device->rad_info.gfx_level >= GFX9)
         return info->bo_metadata->u.gfx9.scanout;
      else
         return info->bo_metadata->u.legacy.scanout;
   }

   return info->scanout;
}

static bool
radv_image_use_fast_clear_for_image_early(const struct radv_device *device, const struct radv_image *image)
{
   if (device->instance->debug_flags & RADV_DEBUG_FORCE_COMPRESS)
      return true;

   if (image->vk.samples <= 1 && image->vk.extent.width * image->vk.extent.height <= 512 * 512) {
      /* Do not enable CMASK or DCC for small surfaces where the cost
       * of the eliminate pass can be higher than the benefit of fast
       * clear. RadeonSI does this, but the image threshold is
       * different.
       */
      return false;
   }

   return !!(image->vk.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
}

static bool
radv_image_use_fast_clear_for_image(const struct radv_device *device, const struct radv_image *image)
{
   if (device->instance->debug_flags & RADV_DEBUG_FORCE_COMPRESS)
      return true;

   return radv_image_use_fast_clear_for_image_early(device, image) && (image->exclusive ||
                                                                       /* Enable DCC for concurrent images if stores are
                                                                        * supported because that means we can keep DCC
                                                                        * compressed on all layouts/queues.
                                                                        */
                                                                       radv_image_use_dcc_image_stores(device, image));
}

bool
radv_are_formats_dcc_compatible(const struct radv_physical_device *pdev, const void *pNext, VkFormat format,
                                VkImageCreateFlags flags, bool *sign_reinterpret)
{
   bool blendable;

   if (!radv_is_colorbuffer_format_supported(pdev, format, &blendable))
      return false;

   if (sign_reinterpret != NULL)
      *sign_reinterpret = false;

   /* All formats are compatible on GFX11. */
   if ((flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) && pdev->rad_info.gfx_level < GFX11) {
      const struct VkImageFormatListCreateInfo *format_list =
         (const struct VkImageFormatListCreateInfo *)vk_find_struct_const(pNext, IMAGE_FORMAT_LIST_CREATE_INFO);

      /* We have to ignore the existence of the list if viewFormatCount = 0 */
      if (format_list && format_list->viewFormatCount) {
         /* compatibility is transitive, so we only need to check
          * one format with everything else. */
         for (unsigned i = 0; i < format_list->viewFormatCount; ++i) {
            if (format_list->pViewFormats[i] == VK_FORMAT_UNDEFINED)
               continue;

            if (!radv_dcc_formats_compatible(pdev->rad_info.gfx_level, format, format_list->pViewFormats[i],
                                             sign_reinterpret))
               return false;
         }
      } else {
         return false;
      }
   }

   return true;
}

static bool
radv_format_is_atomic_allowed(struct radv_device *device, VkFormat format)
{
   if (format == VK_FORMAT_R32_SFLOAT && !device->image_float32_atomics)
      return false;

   return radv_is_atomic_format_supported(format);
}

static bool
radv_formats_is_atomic_allowed(struct radv_device *device, const void *pNext, VkFormat format, VkImageCreateFlags flags)
{
   if (radv_format_is_atomic_allowed(device, format))
      return true;

   if (flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) {
      const struct VkImageFormatListCreateInfo *format_list =
         (const struct VkImageFormatListCreateInfo *)vk_find_struct_const(pNext, IMAGE_FORMAT_LIST_CREATE_INFO);

      /* We have to ignore the existence of the list if viewFormatCount = 0 */
      if (format_list && format_list->viewFormatCount) {
         for (unsigned i = 0; i < format_list->viewFormatCount; ++i) {
            if (radv_format_is_atomic_allowed(device, format_list->pViewFormats[i]))
               return true;
         }
      }
   }

   return false;
}

static bool
radv_use_dcc_for_image_early(struct radv_device *device, struct radv_image *image, const VkImageCreateInfo *pCreateInfo,
                             VkFormat format, bool *sign_reinterpret)
{
   /* DCC (Delta Color Compression) is only available for GFX8+. */
   if (device->physical_device->rad_info.gfx_level < GFX8)
      return false;

   const VkImageCompressionControlEXT *compression =
      vk_find_struct_const(pCreateInfo->pNext, IMAGE_COMPRESSION_CONTROL_EXT);

   if (device->instance->debug_flags & RADV_DEBUG_NO_DCC ||
       (compression && compression->flags == VK_IMAGE_COMPRESSION_DISABLED_EXT)) {
      return false;
   }

   if (image->shareable && image->vk.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)
      return false;

   /*
    * TODO: Enable DCC for storage images on GFX9 and earlier.
    *
    * Also disable DCC with atomics because even when DCC stores are
    * supported atomics will always decompress. So if we are
    * decompressing a lot anyway we might as well not have DCC.
    */
   if ((pCreateInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT) &&
       (device->physical_device->rad_info.gfx_level < GFX10 ||
        radv_formats_is_atomic_allowed(device, pCreateInfo->pNext, format, pCreateInfo->flags)))
      return false;

   if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR)
      return false;

   if (vk_format_is_subsampled(format) || vk_format_get_plane_count(format) > 1)
      return false;

   if (!radv_image_use_fast_clear_for_image_early(device, image) &&
       image->vk.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)
      return false;

   /* Do not enable DCC for mipmapped arrays because performance is worse. */
   if (pCreateInfo->arrayLayers > 1 && pCreateInfo->mipLevels > 1)
      return false;

   if (device->physical_device->rad_info.gfx_level < GFX10) {
      /* TODO: Add support for DCC MSAA on GFX8-9. */
      if (pCreateInfo->samples > 1 && !device->physical_device->dcc_msaa_allowed)
         return false;

      /* TODO: Add support for DCC layers/mipmaps on GFX9. */
      if ((pCreateInfo->arrayLayers > 1 || pCreateInfo->mipLevels > 1) &&
          device->physical_device->rad_info.gfx_level == GFX9)
         return false;
   }

   /* DCC MSAA can't work on GFX10.3 and earlier without FMASK. */
   if (pCreateInfo->samples > 1 && device->physical_device->rad_info.gfx_level < GFX11 &&
       (device->instance->debug_flags & RADV_DEBUG_NO_FMASK))
      return false;

   return radv_are_formats_dcc_compatible(device->physical_device, pCreateInfo->pNext, format, pCreateInfo->flags,
                                          sign_reinterpret);
}

static bool
radv_use_dcc_for_image_late(struct radv_device *device, struct radv_image *image)
{
   if (!radv_image_has_dcc(image))
      return false;

   if (image->vk.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)
      return true;

   if (!radv_image_use_fast_clear_for_image(device, image))
      return false;

   /* TODO: Fix storage images with DCC without DCC image stores.
    * Disabling it for now. */
   if ((image->vk.usage & VK_IMAGE_USAGE_STORAGE_BIT) && !radv_image_use_dcc_image_stores(device, image))
      return false;

   return true;
}

/*
 * Whether to enable image stores with DCC compression for this image. If
 * this function returns false the image subresource should be decompressed
 * before using it with image stores.
 *
 * Note that this can have mixed performance implications, see
 * https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/6796#note_643299
 *
 * This function assumes the image uses DCC compression.
 */
bool
radv_image_use_dcc_image_stores(const struct radv_device *device, const struct radv_image *image)
{
   return ac_surface_supports_dcc_image_stores(device->physical_device->rad_info.gfx_level, &image->planes[0].surface);
}

/*
 * Whether to use a predicate to determine whether DCC is in a compressed
 * state. This can be used to avoid decompressing an image multiple times.
 */
bool
radv_image_use_dcc_predication(const struct radv_device *device, const struct radv_image *image)
{
   return radv_image_has_dcc(image) && !radv_image_use_dcc_image_stores(device, image);
}

static inline bool
radv_use_fmask_for_image(const struct radv_device *device, const struct radv_image *image)
{
   if (device->physical_device->rad_info.gfx_level == GFX9 && image->vk.array_layers > 1) {
      /* On GFX9, FMASK can be interleaved with layers and this isn't properly supported. */
      return false;
   }

   return device->physical_device->use_fmask && image->vk.samples > 1 &&
          ((image->vk.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ||
           (device->instance->debug_flags & RADV_DEBUG_FORCE_COMPRESS));
}

static inline bool
radv_use_htile_for_image(const struct radv_device *device, const struct radv_image *image,
                         const VkImageCreateInfo *pCreateInfo)
{
   const enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;

   const VkImageCompressionControlEXT *compression =
      vk_find_struct_const(pCreateInfo->pNext, IMAGE_COMPRESSION_CONTROL_EXT);

   if (device->instance->debug_flags & RADV_DEBUG_NO_HIZ ||
       (compression && compression->flags == VK_IMAGE_COMPRESSION_DISABLED_EXT))
      return false;

   /* TODO:
    * - Investigate about mips+layers.
    * - Enable on other gens.
    */
   bool use_htile_for_mips = image->vk.array_layers == 1 && device->physical_device->rad_info.gfx_level >= GFX10;

   /* Stencil texturing with HTILE doesn't work with mipmapping on Navi10-14. */
   if (device->physical_device->rad_info.gfx_level == GFX10 && image->vk.format == VK_FORMAT_D32_SFLOAT_S8_UINT &&
       image->vk.mip_levels > 1)
      return false;

   /* Do not enable HTILE for very small images because it seems less performant but make sure it's
    * allowed with VRS attachments because we need HTILE on GFX10.3.
    */
   if (image->vk.extent.width * image->vk.extent.height < 8 * 8 &&
       !(device->instance->debug_flags & RADV_DEBUG_FORCE_COMPRESS) &&
       !(gfx_level == GFX10_3 && device->attachment_vrs_enabled))
      return false;

   return (image->vk.mip_levels == 1 || use_htile_for_mips) && !image->shareable;
}

static bool
radv_use_tc_compat_cmask_for_image(struct radv_device *device, struct radv_image *image)
{
   /* TC-compat CMASK is only available for GFX8+. */
   if (device->physical_device->rad_info.gfx_level < GFX8)
      return false;

   /* GFX9 has issues when sample count is greater than 2 */
   if (device->physical_device->rad_info.gfx_level == GFX9 && image->vk.samples > 2)
      return false;

   if (device->instance->debug_flags & RADV_DEBUG_NO_TC_COMPAT_CMASK)
      return false;

   /* TC-compat CMASK with storage images is supported on GFX10+. */
   if ((image->vk.usage & VK_IMAGE_USAGE_STORAGE_BIT) && device->physical_device->rad_info.gfx_level < GFX10)
      return false;

   /* Do not enable TC-compatible if the image isn't readable by a shader
    * because no texture fetches will happen.
    */
   if (!(image->vk.usage &
         (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)))
      return false;

   /* If the image doesn't have FMASK, it can't be fetchable. */
   if (!radv_image_has_fmask(image))
      return false;

   return true;
}

static uint32_t
radv_get_bo_metadata_word1(const struct radv_device *device)
{
   return (ATI_VENDOR_ID << 16) | device->physical_device->rad_info.pci_id;
}

static bool
radv_is_valid_opaque_metadata(const struct radv_device *device, const struct radeon_bo_metadata *md)
{
   if (md->metadata[0] != 1 || md->metadata[1] != radv_get_bo_metadata_word1(device))
      return false;

   if (md->size_metadata < 40)
      return false;

   return true;
}

static void
radv_patch_surface_from_metadata(struct radv_device *device, struct radeon_surf *surface,
                                 const struct radeon_bo_metadata *md)
{
   surface->flags = RADEON_SURF_CLR(surface->flags, MODE);

   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      if (md->u.gfx9.swizzle_mode > 0)
         surface->flags |= RADEON_SURF_SET(RADEON_SURF_MODE_2D, MODE);
      else
         surface->flags |= RADEON_SURF_SET(RADEON_SURF_MODE_LINEAR_ALIGNED, MODE);

      surface->u.gfx9.swizzle_mode = md->u.gfx9.swizzle_mode;
   } else {
      surface->u.legacy.pipe_config = md->u.legacy.pipe_config;
      surface->u.legacy.bankw = md->u.legacy.bankw;
      surface->u.legacy.bankh = md->u.legacy.bankh;
      surface->u.legacy.tile_split = md->u.legacy.tile_split;
      surface->u.legacy.mtilea = md->u.legacy.mtilea;
      surface->u.legacy.num_banks = md->u.legacy.num_banks;

      if (md->u.legacy.macrotile == RADEON_LAYOUT_TILED)
         surface->flags |= RADEON_SURF_SET(RADEON_SURF_MODE_2D, MODE);
      else if (md->u.legacy.microtile == RADEON_LAYOUT_TILED)
         surface->flags |= RADEON_SURF_SET(RADEON_SURF_MODE_1D, MODE);
      else
         surface->flags |= RADEON_SURF_SET(RADEON_SURF_MODE_LINEAR_ALIGNED, MODE);
   }
}

static VkResult
radv_patch_image_dimensions(struct radv_device *device, struct radv_image *image,
                            const struct radv_image_create_info *create_info, struct ac_surf_info *image_info)
{
   unsigned width = image->vk.extent.width;
   unsigned height = image->vk.extent.height;

   /*
    * minigbm sometimes allocates bigger images which is going to result in
    * weird strides and other properties. Lets be lenient where possible and
    * fail it on GFX10 (as we cannot cope there).
    *
    * Example hack: https://chromium-review.googlesource.com/c/chromiumos/platform/minigbm/+/1457777/
    */
   if (create_info->bo_metadata && radv_is_valid_opaque_metadata(device, create_info->bo_metadata)) {
      const struct radeon_bo_metadata *md = create_info->bo_metadata;

      if (device->physical_device->rad_info.gfx_level >= GFX10) {
         width = G_00A004_WIDTH_LO(md->metadata[3]) + (G_00A008_WIDTH_HI(md->metadata[4]) << 2) + 1;
         height = G_00A008_HEIGHT(md->metadata[4]) + 1;
      } else {
         width = G_008F18_WIDTH(md->metadata[4]) + 1;
         height = G_008F18_HEIGHT(md->metadata[4]) + 1;
      }
   }

   if (image->vk.extent.width == width && image->vk.extent.height == height)
      return VK_SUCCESS;

   if (width < image->vk.extent.width || height < image->vk.extent.height) {
      fprintf(stderr,
              "The imported image has smaller dimensions than the internal\n"
              "dimensions. Using it is going to fail badly, so we reject\n"
              "this import.\n"
              "(internal dimensions: %d x %d, external dimensions: %d x %d)\n",
              image->vk.extent.width, image->vk.extent.height, width, height);
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   } else if (device->physical_device->rad_info.gfx_level >= GFX10) {
      fprintf(stderr,
              "Tried to import an image with inconsistent width on GFX10.\n"
              "As GFX10 has no separate stride fields we cannot cope with\n"
              "an inconsistency in width and will fail this import.\n"
              "(internal dimensions: %d x %d, external dimensions: %d x %d)\n",
              image->vk.extent.width, image->vk.extent.height, width, height);
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   } else {
      fprintf(stderr,
              "Tried to import an image with inconsistent width on pre-GFX10.\n"
              "As GFX10 has no separate stride fields we cannot cope with\n"
              "an inconsistency and would fail on GFX10.\n"
              "(internal dimensions: %d x %d, external dimensions: %d x %d)\n",
              image->vk.extent.width, image->vk.extent.height, width, height);
   }
   image_info->width = width;
   image_info->height = height;

   return VK_SUCCESS;
}

static VkResult
radv_patch_image_from_extra_info(struct radv_device *device, struct radv_image *image,
                                 const struct radv_image_create_info *create_info, struct ac_surf_info *image_info)
{
   VkResult result = radv_patch_image_dimensions(device, image, create_info, image_info);
   if (result != VK_SUCCESS)
      return result;

   for (unsigned plane = 0; plane < image->plane_count; ++plane) {
      if (create_info->bo_metadata) {
         radv_patch_surface_from_metadata(device, &image->planes[plane].surface, create_info->bo_metadata);
      }

      if (radv_surface_has_scanout(device, create_info)) {
         image->planes[plane].surface.flags |= RADEON_SURF_SCANOUT;
         if (device->instance->debug_flags & RADV_DEBUG_NO_DISPLAY_DCC)
            image->planes[plane].surface.flags |= RADEON_SURF_DISABLE_DCC;

         image_info->surf_index = NULL;
      }

      if (create_info->prime_blit_src && !device->physical_device->rad_info.sdma_supports_compression) {
         /* Older SDMA hw can't handle DCC */
         image->planes[plane].surface.flags |= RADEON_SURF_DISABLE_DCC;
      }
   }
   return VK_SUCCESS;
}

static VkFormat
radv_image_get_plane_format(const struct radv_physical_device *pdev, const struct radv_image *image, unsigned plane)
{
   if (radv_is_format_emulated(pdev, image->vk.format)) {
      if (plane == 0)
         return image->vk.format;
      if (vk_format_description(image->vk.format)->layout == UTIL_FORMAT_LAYOUT_ASTC)
         return vk_texcompress_astc_emulation_format(image->vk.format);
      else
         return vk_texcompress_etc2_emulation_format(image->vk.format);
   }

   return vk_format_get_plane_format(image->vk.format, plane);
}

static uint64_t
radv_get_surface_flags(struct radv_device *device, struct radv_image *image, unsigned plane_id,
                       const VkImageCreateInfo *pCreateInfo, VkFormat image_format)
{
   uint64_t flags;
   unsigned array_mode = radv_choose_tiling(device, pCreateInfo, image_format);
   VkFormat format = radv_image_get_plane_format(device->physical_device, image, plane_id);
   const struct util_format_description *desc = vk_format_description(format);
   bool is_depth, is_stencil;

   is_depth = util_format_has_depth(desc);
   is_stencil = util_format_has_stencil(desc);

   flags = RADEON_SURF_SET(array_mode, MODE);

   switch (pCreateInfo->imageType) {
   case VK_IMAGE_TYPE_1D:
      if (pCreateInfo->arrayLayers > 1)
         flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_1D_ARRAY, TYPE);
      else
         flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_1D, TYPE);
      break;
   case VK_IMAGE_TYPE_2D:
      if (pCreateInfo->arrayLayers > 1)
         flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_2D_ARRAY, TYPE);
      else
         flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_2D, TYPE);
      break;
   case VK_IMAGE_TYPE_3D:
      flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_3D, TYPE);
      break;
   default:
      unreachable("unhandled image type");
   }

   /* Required for clearing/initializing a specific layer on GFX8. */
   flags |= RADEON_SURF_CONTIGUOUS_DCC_LAYERS;

   if (is_depth) {
      flags |= RADEON_SURF_ZBUFFER;

      if (is_depth && is_stencil && device->physical_device->rad_info.gfx_level <= GFX8) {
         if (!(pCreateInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
            flags |= RADEON_SURF_NO_RENDER_TARGET;

         /* RADV doesn't support stencil pitch adjustment. As a result there are some spec gaps that
          * are not covered by CTS.
          *
          * For D+S images with pitch constraints due to rendertarget usage it can happen that
          * sampling from mipmaps beyond the base level of the descriptor is broken as the pitch
          * adjustment can't be applied to anything beyond the first level.
          */
         flags |= RADEON_SURF_NO_STENCIL_ADJUST;
      }

      if (radv_use_htile_for_image(device, image, pCreateInfo) && !(flags & RADEON_SURF_NO_RENDER_TARGET)) {
         if (radv_use_tc_compat_htile_for_image(device, pCreateInfo, image_format))
            flags |= RADEON_SURF_TC_COMPATIBLE_HTILE;
      } else {
         flags |= RADEON_SURF_NO_HTILE;
      }
   }

   if (is_stencil)
      flags |= RADEON_SURF_SBUFFER;

   if (device->physical_device->rad_info.gfx_level >= GFX9 && pCreateInfo->imageType == VK_IMAGE_TYPE_3D &&
       vk_format_get_blocksizebits(image_format) == 128 && vk_format_is_compressed(image_format))
      flags |= RADEON_SURF_NO_RENDER_TARGET;

   if (!radv_use_dcc_for_image_early(device, image, pCreateInfo, image_format, &image->dcc_sign_reinterpret))
      flags |= RADEON_SURF_DISABLE_DCC;

   if (!radv_use_fmask_for_image(device, image))
      flags |= RADEON_SURF_NO_FMASK;

   if (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
      flags |= RADEON_SURF_PRT | RADEON_SURF_NO_FMASK | RADEON_SURF_NO_HTILE | RADEON_SURF_DISABLE_DCC;
   }

   if (image->queue_family_mask & BITFIELD_BIT(RADV_QUEUE_TRANSFER)) {
      if (!device->physical_device->rad_info.sdma_supports_compression)
         flags |= RADEON_SURF_DISABLE_DCC | RADEON_SURF_NO_HTILE;
   }

   /* Disable DCC for VRS rate images because the hw can't handle compression. */
   if (pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)
      flags |= RADEON_SURF_VRS_RATE | RADEON_SURF_DISABLE_DCC;
   if (!(pCreateInfo->usage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)))
      flags |= RADEON_SURF_NO_TEXTURE;

   return flags;
}

unsigned
radv_map_swizzle(unsigned swizzle)
{
   switch (swizzle) {
   case PIPE_SWIZZLE_Y:
      return V_008F0C_SQ_SEL_Y;
   case PIPE_SWIZZLE_Z:
      return V_008F0C_SQ_SEL_Z;
   case PIPE_SWIZZLE_W:
      return V_008F0C_SQ_SEL_W;
   case PIPE_SWIZZLE_0:
      return V_008F0C_SQ_SEL_0;
   case PIPE_SWIZZLE_1:
      return V_008F0C_SQ_SEL_1;
   default: /* PIPE_SWIZZLE_X */
      return V_008F0C_SQ_SEL_X;
   }
}

void
radv_compose_swizzle(const struct util_format_description *desc, const VkComponentMapping *mapping,
                     enum pipe_swizzle swizzle[4])
{
   if (desc->format == PIPE_FORMAT_R64_UINT || desc->format == PIPE_FORMAT_R64_SINT) {
      /* 64-bit formats only support storage images and storage images
       * require identity component mappings. We use 32-bit
       * instructions to access 64-bit images, so we need a special
       * case here.
       *
       * The zw components are 1,0 so that they can be easily be used
       * by loads to create the w component, which has to be 0 for
       * NULL descriptors.
       */
      swizzle[0] = PIPE_SWIZZLE_X;
      swizzle[1] = PIPE_SWIZZLE_Y;
      swizzle[2] = PIPE_SWIZZLE_1;
      swizzle[3] = PIPE_SWIZZLE_0;
   } else if (!mapping) {
      for (unsigned i = 0; i < 4; i++)
         swizzle[i] = desc->swizzle[i];
   } else if (desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS) {
      const unsigned char swizzle_xxxx[4] = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_0, PIPE_SWIZZLE_0, PIPE_SWIZZLE_1};
      vk_format_compose_swizzles(mapping, swizzle_xxxx, swizzle);
   } else {
      vk_format_compose_swizzles(mapping, desc->swizzle, swizzle);
   }
}

bool
vi_alpha_is_on_msb(const struct radv_device *device, const VkFormat format)
{
   if (device->physical_device->rad_info.gfx_level >= GFX11)
      return false;

   const struct util_format_description *desc = vk_format_description(format);

   if (device->physical_device->rad_info.gfx_level >= GFX10 && desc->nr_channels == 1)
      return desc->swizzle[3] == PIPE_SWIZZLE_X;

   return radv_translate_colorswap(format, false) <= 1;
}

static void
radv_query_opaque_metadata(struct radv_device *device, struct radv_image *image, unsigned plane_id,
                           struct radeon_bo_metadata *md)
{
   static const VkComponentMapping fixedmapping;
   const VkFormat plane_format = radv_image_get_plane_format(device->physical_device, image, plane_id);
   const unsigned plane_width = vk_format_get_plane_width(image->vk.format, plane_id, image->vk.extent.width);
   const unsigned plane_height = vk_format_get_plane_height(image->vk.format, plane_id, image->vk.extent.height);
   struct radeon_surf *surface = &image->planes[plane_id].surface;
   const struct legacy_surf_level *base_level_info =
      device->physical_device->rad_info.gfx_level <= GFX8 ? &surface->u.legacy.level[0] : NULL;
   uint32_t desc[8];

   radv_make_texture_descriptor(device, image, false, (VkImageViewType)image->vk.image_type, plane_format,
                                &fixedmapping, 0, image->vk.mip_levels - 1, 0, image->vk.array_layers - 1, plane_width,
                                plane_height, image->vk.extent.depth, 0.0f, desc, NULL, 0, NULL, NULL);

   radv_set_mutable_tex_desc_fields(device, image, base_level_info, plane_id, 0, 0, surface->blk_w, false, false, false,
                                    false, desc, NULL);

   ac_surface_compute_umd_metadata(&device->physical_device->rad_info, surface, image->vk.mip_levels, desc,
                                   &md->size_metadata, md->metadata,
                                   device->instance->debug_flags & RADV_DEBUG_EXTRA_MD);
}

void
radv_init_metadata(struct radv_device *device, struct radv_image *image, struct radeon_bo_metadata *metadata)
{
   /* use plane 0, even when there are multiple planes, to follow radeonsi */
   const unsigned plane_id = 0;
   struct radeon_surf *surface = &image->planes[plane_id].surface;

   memset(metadata, 0, sizeof(*metadata));

   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      uint64_t dcc_offset =
         image->bindings[0].offset + (surface->display_dcc_offset ? surface->display_dcc_offset : surface->meta_offset);
      metadata->u.gfx9.swizzle_mode = surface->u.gfx9.swizzle_mode;
      metadata->u.gfx9.dcc_offset_256b = dcc_offset >> 8;
      metadata->u.gfx9.dcc_pitch_max = surface->u.gfx9.color.display_dcc_pitch_max;
      metadata->u.gfx9.dcc_independent_64b_blocks = surface->u.gfx9.color.dcc.independent_64B_blocks;
      metadata->u.gfx9.dcc_independent_128b_blocks = surface->u.gfx9.color.dcc.independent_128B_blocks;
      metadata->u.gfx9.dcc_max_compressed_block_size = surface->u.gfx9.color.dcc.max_compressed_block_size;
      metadata->u.gfx9.scanout = (surface->flags & RADEON_SURF_SCANOUT) != 0;
   } else {
      metadata->u.legacy.microtile =
         surface->u.legacy.level[0].mode >= RADEON_SURF_MODE_1D ? RADEON_LAYOUT_TILED : RADEON_LAYOUT_LINEAR;
      metadata->u.legacy.macrotile =
         surface->u.legacy.level[0].mode >= RADEON_SURF_MODE_2D ? RADEON_LAYOUT_TILED : RADEON_LAYOUT_LINEAR;
      metadata->u.legacy.pipe_config = surface->u.legacy.pipe_config;
      metadata->u.legacy.bankw = surface->u.legacy.bankw;
      metadata->u.legacy.bankh = surface->u.legacy.bankh;
      metadata->u.legacy.tile_split = surface->u.legacy.tile_split;
      metadata->u.legacy.mtilea = surface->u.legacy.mtilea;
      metadata->u.legacy.num_banks = surface->u.legacy.num_banks;
      metadata->u.legacy.stride = surface->u.legacy.level[0].nblk_x * surface->bpe;
      metadata->u.legacy.scanout = (surface->flags & RADEON_SURF_SCANOUT) != 0;
   }
   radv_query_opaque_metadata(device, image, plane_id, metadata);
}

void
radv_image_override_offset_stride(struct radv_device *device, struct radv_image *image, uint64_t offset,
                                  uint32_t stride)
{
   ac_surface_override_offset_stride(&device->physical_device->rad_info, &image->planes[0].surface,
                                     image->vk.array_layers, image->vk.mip_levels, offset, stride);
}

static void
radv_image_alloc_single_sample_cmask(const struct radv_device *device, const struct radv_image *image,
                                     struct radeon_surf *surf)
{
   if (!surf->cmask_size || surf->cmask_offset || surf->bpe > 8 || image->vk.mip_levels > 1 ||
       image->vk.extent.depth > 1 || radv_image_has_dcc(image) || !radv_image_use_fast_clear_for_image(device, image) ||
       (image->vk.create_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT))
      return;

   assert(image->vk.samples == 1);

   surf->cmask_offset = align64(surf->total_size, 1ull << surf->cmask_alignment_log2);
   surf->total_size = surf->cmask_offset + surf->cmask_size;
   surf->alignment_log2 = MAX2(surf->alignment_log2, surf->cmask_alignment_log2);
}

static void
radv_image_alloc_values(const struct radv_device *device, struct radv_image *image)
{
   /* images with modifiers can be potentially imported */
   if (image->vk.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)
      return;

   if (radv_image_has_cmask(image) || (radv_image_has_dcc(image) && !image->support_comp_to_single)) {
      image->fce_pred_offset = image->size;
      image->size += 8 * image->vk.mip_levels;
   }

   if (radv_image_use_dcc_predication(device, image)) {
      image->dcc_pred_offset = image->size;
      image->size += 8 * image->vk.mip_levels;
   }

   if ((radv_image_has_dcc(image) && !image->support_comp_to_single) || radv_image_has_cmask(image) ||
       radv_image_has_htile(image)) {
      image->clear_value_offset = image->size;
      image->size += 8 * image->vk.mip_levels;
   }

   if (radv_image_is_tc_compat_htile(image) && device->physical_device->rad_info.has_tc_compat_zrange_bug) {
      /* Metadata for the TC-compatible HTILE hardware bug which
       * have to be fixed by updating ZRANGE_PRECISION when doing
       * fast depth clears to 0.0f.
       */
      image->tc_compat_zrange_offset = image->size;
      image->size += image->vk.mip_levels * 4;
   }
}

/* Determine if the image is affected by the pipe misaligned metadata issue
 * which requires to invalidate L2.
 */
static bool
radv_image_is_pipe_misaligned(const struct radv_device *device, const struct radv_image *image)
{
   const struct radeon_info *rad_info = &device->physical_device->rad_info;
   int log2_samples = util_logbase2(image->vk.samples);

   assert(rad_info->gfx_level >= GFX10);

   for (unsigned i = 0; i < image->plane_count; ++i) {
      VkFormat fmt = radv_image_get_plane_format(device->physical_device, image, i);
      int log2_bpp = util_logbase2(vk_format_get_blocksize(fmt));
      int log2_bpp_and_samples;

      if (rad_info->gfx_level >= GFX10_3) {
         log2_bpp_and_samples = log2_bpp + log2_samples;
      } else {
         if (vk_format_has_depth(image->vk.format) && image->vk.array_layers >= 8) {
            log2_bpp = 2;
         }

         log2_bpp_and_samples = MIN2(6, log2_bpp + log2_samples);
      }

      int num_pipes = G_0098F8_NUM_PIPES(rad_info->gb_addr_config);
      int overlap = MAX2(0, log2_bpp_and_samples + num_pipes - 8);

      if (vk_format_has_depth(image->vk.format)) {
         if (radv_image_is_tc_compat_htile(image) && overlap) {
            return true;
         }
      } else {
         int max_compressed_frags = G_0098F8_MAX_COMPRESSED_FRAGS(rad_info->gb_addr_config);
         int log2_samples_frag_diff = MAX2(0, log2_samples - max_compressed_frags);
         int samples_overlap = MIN2(log2_samples, overlap);

         /* TODO: It shouldn't be necessary if the image has DCC but
          * not readable by shader.
          */
         if ((radv_image_has_dcc(image) || radv_image_is_tc_compat_cmask(image)) &&
             (samples_overlap > log2_samples_frag_diff)) {
            return true;
         }
      }
   }

   return false;
}

static bool
radv_image_is_l2_coherent(const struct radv_device *device, const struct radv_image *image)
{
   if (device->physical_device->rad_info.gfx_level >= GFX10) {
      return !device->physical_device->rad_info.tcc_rb_non_coherent && !radv_image_is_pipe_misaligned(device, image);
   } else if (device->physical_device->rad_info.gfx_level == GFX9) {
      if (image->vk.samples == 1 &&
          (image->vk.usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
          !vk_format_has_stencil(image->vk.format)) {
         /* Single-sample color and single-sample depth
          * (not stencil) are coherent with shaders on
          * GFX9.
          */
         return true;
      }
   }

   return false;
}

/**
 * Determine if the given image can be fast cleared.
 */
bool
radv_image_can_fast_clear(const struct radv_device *device, const struct radv_image *image)
{
   if (device->instance->debug_flags & RADV_DEBUG_NO_FAST_CLEARS)
      return false;

   if (vk_format_is_color(image->vk.format)) {
      if (!radv_image_has_cmask(image) && !radv_image_has_dcc(image))
         return false;

      /* RB+ doesn't work with CMASK fast clear on Stoney. */
      if (!radv_image_has_dcc(image) && device->physical_device->rad_info.family == CHIP_STONEY)
         return false;

      /* Fast-clears with CMASK aren't supported for 128-bit formats. */
      if (radv_image_has_cmask(image) && vk_format_get_blocksizebits(image->vk.format) > 64)
         return false;
   } else {
      if (!radv_image_has_htile(image))
         return false;
   }

   /* Do not fast clears 3D images. */
   if (image->vk.image_type == VK_IMAGE_TYPE_3D)
      return false;

   return true;
}

/**
 * Determine if the given image can be fast cleared using comp-to-single.
 */
static bool
radv_image_use_comp_to_single(const struct radv_device *device, const struct radv_image *image)
{
   /* comp-to-single is only available for GFX10+. */
   if (device->physical_device->rad_info.gfx_level < GFX10)
      return false;

   /* If the image can't be fast cleared, comp-to-single can't be used. */
   if (!radv_image_can_fast_clear(device, image))
      return false;

   /* If the image doesn't have DCC, it can't be fast cleared using comp-to-single */
   if (!radv_image_has_dcc(image))
      return false;

   /* It seems 8bpp and 16bpp require RB+ to work. */
   unsigned bytes_per_pixel = vk_format_get_blocksize(image->vk.format);
   if (bytes_per_pixel <= 2 && !device->physical_device->rad_info.rbplus_allowed)
      return false;

   return true;
}

static unsigned
radv_get_internal_plane_count(const struct radv_physical_device *pdev, VkFormat fmt)
{
   if (radv_is_format_emulated(pdev, fmt))
      return 2;
   return vk_format_get_plane_count(fmt);
}

static void
radv_image_reset_layout(const struct radv_physical_device *pdev, struct radv_image *image)
{
   image->size = 0;
   image->alignment = 1;

   image->tc_compatible_cmask = 0;
   image->fce_pred_offset = image->dcc_pred_offset = 0;
   image->clear_value_offset = image->tc_compat_zrange_offset = 0;

   unsigned plane_count = radv_get_internal_plane_count(pdev, image->vk.format);
   for (unsigned i = 0; i < plane_count; ++i) {
      VkFormat format = radv_image_get_plane_format(pdev, image, i);
      if (vk_format_has_depth(format))
         format = vk_format_depth_only(format);

      uint64_t flags = image->planes[i].surface.flags;
      uint64_t modifier = image->planes[i].surface.modifier;
      memset(image->planes + i, 0, sizeof(image->planes[i]));

      image->planes[i].surface.flags = flags;
      image->planes[i].surface.modifier = modifier;
      image->planes[i].surface.blk_w = vk_format_get_blockwidth(format);
      image->planes[i].surface.blk_h = vk_format_get_blockheight(format);
      image->planes[i].surface.bpe = vk_format_get_blocksize(format);

      /* align byte per element on dword */
      if (image->planes[i].surface.bpe == 3) {
         image->planes[i].surface.bpe = 4;
      }
   }
}

struct ac_surf_info
radv_get_ac_surf_info(struct radv_device *device, const struct radv_image *image)
{
   struct ac_surf_info info;

   memset(&info, 0, sizeof(info));

   info.width = image->vk.extent.width;
   info.height = image->vk.extent.height;
   info.depth = image->vk.extent.depth;
   info.samples = image->vk.samples;
   info.storage_samples = image->vk.samples;
   info.array_size = image->vk.array_layers;
   info.levels = image->vk.mip_levels;
   info.num_channels = vk_format_get_nr_components(image->vk.format);

   if (!vk_format_is_depth_or_stencil(image->vk.format) && !image->shareable &&
       !(image->vk.create_flags & (VK_IMAGE_CREATE_SPARSE_ALIASED_BIT | VK_IMAGE_CREATE_ALIAS_BIT)) &&
       image->vk.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
      info.surf_index = &device->image_mrt_offset_counter;
   }

   return info;
}

VkResult
radv_image_create_layout(struct radv_device *device, struct radv_image_create_info create_info,
                         const struct VkImageDrmFormatModifierExplicitCreateInfoEXT *mod_info,
                         const struct VkVideoProfileListInfoKHR *profile_list, struct radv_image *image)
{
   /* Clear the pCreateInfo pointer so we catch issues in the delayed case when we test in the
    * common internal case. */
   create_info.vk_info = NULL;

   struct ac_surf_info image_info = radv_get_ac_surf_info(device, image);
   VkResult result = radv_patch_image_from_extra_info(device, image, &create_info, &image_info);
   if (result != VK_SUCCESS)
      return result;

   assert(!mod_info || mod_info->drmFormatModifierPlaneCount >= image->plane_count);

   radv_image_reset_layout(device->physical_device, image);

   /*
    * Due to how the decoder works, the user can't supply an oversized image, because if it attempts
    * to sample it later with a linear filter, it will get garbage after the height it wants,
    * so we let the user specify the width/height unaligned, and align them preallocation.
    */
   if (image->vk.usage & (VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR)) {
      assert(profile_list);
      uint32_t width_align, height_align;
      radv_video_get_profile_alignments(device->physical_device, profile_list, &width_align, &height_align);
      image_info.width = align(image_info.width, width_align);
      image_info.height = align(image_info.height, height_align);

      if (radv_has_uvd(device->physical_device) && image->vk.usage & VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) {
         /* UVD and kernel demand a full DPB allocation. */
         image_info.array_size = MIN2(16, image_info.array_size);
      }
   }

   unsigned plane_count = radv_get_internal_plane_count(device->physical_device, image->vk.format);
   for (unsigned plane = 0; plane < plane_count; ++plane) {
      struct ac_surf_info info = image_info;
      uint64_t offset;
      unsigned stride;

      info.width = vk_format_get_plane_width(image->vk.format, plane, info.width);
      info.height = vk_format_get_plane_height(image->vk.format, plane, info.height);

      if (create_info.no_metadata_planes || plane_count > 1) {
         image->planes[plane].surface.flags |= RADEON_SURF_DISABLE_DCC | RADEON_SURF_NO_FMASK | RADEON_SURF_NO_HTILE;
      }

      device->ws->surface_init(device->ws, &info, &image->planes[plane].surface);

      if (plane == 0) {
         if (!radv_use_dcc_for_image_late(device, image))
            ac_surface_zero_dcc_fields(&image->planes[0].surface);
      }

      if (create_info.bo_metadata && !mod_info &&
          !ac_surface_apply_umd_metadata(&device->physical_device->rad_info, &image->planes[plane].surface,
                                         image->vk.samples, image->vk.mip_levels,
                                         create_info.bo_metadata->size_metadata, create_info.bo_metadata->metadata))
         return VK_ERROR_INVALID_EXTERNAL_HANDLE;

      if (!create_info.no_metadata_planes && !create_info.bo_metadata && plane_count == 1 && !mod_info)
         radv_image_alloc_single_sample_cmask(device, image, &image->planes[plane].surface);

      if (mod_info) {
         if (mod_info->pPlaneLayouts[plane].rowPitch % image->planes[plane].surface.bpe ||
             !mod_info->pPlaneLayouts[plane].rowPitch)
            return VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT;

         offset = mod_info->pPlaneLayouts[plane].offset;
         stride = mod_info->pPlaneLayouts[plane].rowPitch / image->planes[plane].surface.bpe;
      } else {
         offset = image->disjoint ? 0 : align64(image->size, 1ull << image->planes[plane].surface.alignment_log2);
         stride = 0; /* 0 means no override */
      }

      if (!ac_surface_override_offset_stride(&device->physical_device->rad_info, &image->planes[plane].surface,
                                             image->vk.array_layers, image->vk.mip_levels, offset, stride))
         return VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT;

      /* Validate DCC offsets in modifier layout. */
      if (plane_count == 1 && mod_info) {
         unsigned mem_planes = ac_surface_get_nplanes(&image->planes[plane].surface);
         if (mod_info->drmFormatModifierPlaneCount != mem_planes)
            return VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT;

         for (unsigned i = 1; i < mem_planes; ++i) {
            if (ac_surface_get_plane_offset(device->physical_device->rad_info.gfx_level, &image->planes[plane].surface,
                                            i, 0) != mod_info->pPlaneLayouts[i].offset)
               return VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT;
         }
      }

      image->size = MAX2(image->size, offset + image->planes[plane].surface.total_size);
      image->alignment = MAX2(image->alignment, 1 << image->planes[plane].surface.alignment_log2);

      image->planes[plane].format = radv_image_get_plane_format(device->physical_device, image, plane);
   }

   image->tc_compatible_cmask = radv_image_has_cmask(image) && radv_use_tc_compat_cmask_for_image(device, image);

   image->l2_coherent = radv_image_is_l2_coherent(device, image);

   image->support_comp_to_single = radv_image_use_comp_to_single(device, image);

   radv_image_alloc_values(device, image);

   assert(image->planes[0].surface.surf_size);
   assert(image->planes[0].surface.modifier == DRM_FORMAT_MOD_INVALID ||
          ac_modifier_has_dcc(image->planes[0].surface.modifier) == radv_image_has_dcc(image));
   return VK_SUCCESS;
}

static void
radv_destroy_image(struct radv_device *device, const VkAllocationCallbacks *pAllocator, struct radv_image *image)
{
   if ((image->vk.create_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) && image->bindings[0].bo) {
      radv_rmv_log_bo_destroy(device, image->bindings[0].bo);
      device->ws->buffer_destroy(device->ws, image->bindings[0].bo);
   }

   if (image->owned_memory != VK_NULL_HANDLE) {
      RADV_FROM_HANDLE(radv_device_memory, mem, image->owned_memory);
      radv_free_memory(device, pAllocator, mem);
   }

   radv_rmv_log_resource_destroy(device, (uint64_t)radv_image_to_handle(image));
   vk_image_finish(&image->vk);
   vk_free2(&device->vk.alloc, pAllocator, image);
}

static void
radv_image_print_info(struct radv_device *device, struct radv_image *image)
{
   fprintf(stderr, "Image:\n");
   fprintf(stderr,
           "  Info: size=%" PRIu64 ", alignment=%" PRIu32 ", "
           "width=%" PRIu32 ", height=%" PRIu32 ", depth=%" PRIu32 ", "
           "array_size=%" PRIu32 ", levels=%" PRIu32 "\n",
           image->size, image->alignment, image->vk.extent.width, image->vk.extent.height, image->vk.extent.depth,
           image->vk.array_layers, image->vk.mip_levels);
   for (unsigned i = 0; i < image->plane_count; ++i) {
      const struct radv_image_plane *plane = &image->planes[i];
      const struct radeon_surf *surf = &plane->surface;
      const struct util_format_description *desc = vk_format_description(plane->format);
      uint64_t offset = ac_surface_get_plane_offset(device->physical_device->rad_info.gfx_level, &plane->surface, 0, 0);

      fprintf(stderr, "  Plane[%u]: vkformat=%s, offset=%" PRIu64 "\n", i, desc->name, offset);

      ac_surface_print_info(stderr, &device->physical_device->rad_info, surf);
   }
}

static uint64_t
radv_select_modifier(const struct radv_device *dev, VkFormat format,
                     const struct VkImageDrmFormatModifierListCreateInfoEXT *mod_list)
{
   const struct radv_physical_device *pdev = dev->physical_device;
   unsigned mod_count;

   assert(mod_list->drmFormatModifierCount);

   /* We can allow everything here as it does not affect order and the application
    * is only allowed to specify modifiers that we support. */
   const struct ac_modifier_options modifier_options = {
      .dcc = true,
      .dcc_retile = true,
   };

   ac_get_supported_modifiers(&pdev->rad_info, &modifier_options, vk_format_to_pipe_format(format), &mod_count, NULL);

   uint64_t *mods = calloc(mod_count, sizeof(*mods));

   /* If allocations fail, fall back to a dumber solution. */
   if (!mods)
      return mod_list->pDrmFormatModifiers[0];

   ac_get_supported_modifiers(&pdev->rad_info, &modifier_options, vk_format_to_pipe_format(format), &mod_count, mods);

   for (unsigned i = 0; i < mod_count; ++i) {
      for (uint32_t j = 0; j < mod_list->drmFormatModifierCount; ++j) {
         if (mods[i] == mod_list->pDrmFormatModifiers[j]) {
            free(mods);
            return mod_list->pDrmFormatModifiers[j];
         }
      }
   }
   unreachable("App specified an invalid modifier");
}

VkResult
radv_image_create(VkDevice _device, const struct radv_image_create_info *create_info,
                  const VkAllocationCallbacks *alloc, VkImage *pImage, bool is_internal)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   const VkImageCreateInfo *pCreateInfo = create_info->vk_info;
   uint64_t modifier = DRM_FORMAT_MOD_INVALID;
   struct radv_image *image = NULL;
   VkFormat format = radv_select_android_external_format(pCreateInfo->pNext, pCreateInfo->format);
   const struct VkImageDrmFormatModifierListCreateInfoEXT *mod_list =
      vk_find_struct_const(pCreateInfo->pNext, IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT);
   const struct VkImageDrmFormatModifierExplicitCreateInfoEXT *explicit_mod =
      vk_find_struct_const(pCreateInfo->pNext, IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT);
   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
   const struct VkVideoProfileListInfoKHR *profile_list =
      vk_find_struct_const(pCreateInfo->pNext, VIDEO_PROFILE_LIST_INFO_KHR);

   unsigned plane_count = radv_get_internal_plane_count(device->physical_device, format);

   const size_t image_struct_size = sizeof(*image) + sizeof(struct radv_image_plane) * plane_count;

   image = vk_zalloc2(&device->vk.alloc, alloc, image_struct_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!image)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_image_init(&device->vk, &image->vk, pCreateInfo);

   image->plane_count = vk_format_get_plane_count(format);
   image->disjoint = image->plane_count > 1 && pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT;

   image->exclusive = pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE;
   if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
      for (uint32_t i = 0; i < pCreateInfo->queueFamilyIndexCount; ++i)
         if (pCreateInfo->pQueueFamilyIndices[i] == VK_QUEUE_FAMILY_EXTERNAL ||
             pCreateInfo->pQueueFamilyIndices[i] == VK_QUEUE_FAMILY_FOREIGN_EXT)
            image->queue_family_mask |= (1u << RADV_MAX_QUEUE_FAMILIES) - 1u;
         else
            image->queue_family_mask |=
               1u << vk_queue_to_radv(device->physical_device, pCreateInfo->pQueueFamilyIndices[i]);

      /* This queue never really accesses the image. */
      image->queue_family_mask &= ~(1u << RADV_QUEUE_SPARSE);
   }

   const VkExternalMemoryImageCreateInfo *external_info =
      vk_find_struct_const(pCreateInfo->pNext, EXTERNAL_MEMORY_IMAGE_CREATE_INFO);

   image->shareable = external_info;

   if (mod_list)
      modifier = radv_select_modifier(device, format, mod_list);
   else if (explicit_mod)
      modifier = explicit_mod->drmFormatModifier;

   for (unsigned plane = 0; plane < plane_count; ++plane) {
      image->planes[plane].surface.flags = radv_get_surface_flags(device, image, plane, pCreateInfo, format);
      image->planes[plane].surface.modifier = modifier;
   }

   if (image->vk.external_handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) {
#ifdef ANDROID
      image->vk.ahb_format = radv_ahb_format_for_vk_format(image->vk.format);
#endif

      *pImage = radv_image_to_handle(image);
      assert(!(image->vk.create_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT));
      return VK_SUCCESS;
   }

   VkResult result = radv_image_create_layout(device, *create_info, explicit_mod, profile_list, image);
   if (result != VK_SUCCESS) {
      radv_destroy_image(device, alloc, image);
      return result;
   }

   if (image->vk.create_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) {
      image->alignment = MAX2(image->alignment, 4096);
      image->size = align64(image->size, image->alignment);
      image->bindings[0].offset = 0;

      result = device->ws->buffer_create(device->ws, image->size, image->alignment, 0, RADEON_FLAG_VIRTUAL,
                                         RADV_BO_PRIORITY_VIRTUAL, 0, &image->bindings[0].bo);
      if (result != VK_SUCCESS) {
         radv_destroy_image(device, alloc, image);
         return vk_error(device, result);
      }
      radv_rmv_log_bo_allocate(device, image->bindings[0].bo, image->size, true);
   }

   if (device->instance->debug_flags & RADV_DEBUG_IMG) {
      radv_image_print_info(device, image);
   }

   *pImage = radv_image_to_handle(image);

   radv_rmv_log_image_create(device, pCreateInfo, is_internal, *pImage);
   if (image->bindings[0].bo)
      radv_rmv_log_image_bind(device, *pImage);
   return VK_SUCCESS;
}

unsigned
radv_plane_from_aspect(VkImageAspectFlags mask)
{
   switch (mask) {
   case VK_IMAGE_ASPECT_PLANE_1_BIT:
   case VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT:
      return 1;
   case VK_IMAGE_ASPECT_PLANE_2_BIT:
   case VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT:
      return 2;
   case VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT:
      return 3;
   default:
      return 0;
   }
}

VkFormat
radv_get_aspect_format(struct radv_image *image, VkImageAspectFlags mask)
{
   switch (mask) {
   case VK_IMAGE_ASPECT_PLANE_0_BIT:
      return image->planes[0].format;
   case VK_IMAGE_ASPECT_PLANE_1_BIT:
      return image->planes[1].format;
   case VK_IMAGE_ASPECT_PLANE_2_BIT:
      return image->planes[2].format;
   case VK_IMAGE_ASPECT_STENCIL_BIT:
      return vk_format_stencil_only(image->vk.format);
   case VK_IMAGE_ASPECT_DEPTH_BIT:
      return vk_format_depth_only(image->vk.format);
   case VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT:
      return vk_format_depth_only(image->vk.format);
   default:
      return image->vk.format;
   }
}

bool
radv_layout_is_htile_compressed(const struct radv_device *device, const struct radv_image *image, VkImageLayout layout,
                                unsigned queue_mask)
{
   /* Don't compress exclusive images used on transfer queues when SDMA doesn't support HTILE.
    * Note that HTILE is already disabled on concurrent images when not supported.
    */
   if (queue_mask == BITFIELD_BIT(RADV_QUEUE_TRANSFER) && !device->physical_device->rad_info.sdma_supports_compression)
      return false;

   switch (layout) {
   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
   case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
      return radv_image_has_htile(image);
   case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      return radv_image_is_tc_compat_htile(image) ||
             (radv_image_has_htile(image) && queue_mask == (1u << RADV_QUEUE_GENERAL));
   case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
   case VK_IMAGE_LAYOUT_GENERAL:
      /* It should be safe to enable TC-compat HTILE with
       * VK_IMAGE_LAYOUT_GENERAL if we are not in a render loop and
       * if the image doesn't have the storage bit set. This
       * improves performance for apps that use GENERAL for the main
       * depth pass because this allows compression and this reduces
       * the number of decompressions from/to GENERAL.
       */
      if (radv_image_is_tc_compat_htile(image) && queue_mask & (1u << RADV_QUEUE_GENERAL) &&
          !device->instance->drirc.disable_tc_compat_htile_in_general) {
         return true;
      } else {
         return false;
      }
   case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
      /* Do not compress HTILE with feedback loops because we can't read&write it without
       * introducing corruption.
       */
      return false;
   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
      if (radv_image_is_tc_compat_htile(image) ||
          (radv_image_has_htile(image) &&
           !(image->vk.usage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)))) {
         /* Keep HTILE compressed if the image is only going to
          * be used as a depth/stencil read-only attachment.
          */
         return true;
      } else {
         return false;
      }
      break;
   default:
      return radv_image_is_tc_compat_htile(image);
   }
}

bool
radv_layout_can_fast_clear(const struct radv_device *device, const struct radv_image *image, unsigned level,
                           VkImageLayout layout, unsigned queue_mask)
{
   if (radv_dcc_enabled(image, level) && !radv_layout_dcc_compressed(device, image, level, layout, queue_mask))
      return false;

   if (!(image->vk.usage & RADV_IMAGE_USAGE_WRITE_BITS))
      return false;

   if (layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && layout != VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
      return false;

   /* Exclusive images with CMASK or DCC can always be fast-cleared on the gfx queue. Concurrent
    * images can only be fast-cleared if comp-to-single is supported because we don't yet support
    * FCE on the compute queue.
    */
   return queue_mask == (1u << RADV_QUEUE_GENERAL) || radv_image_use_comp_to_single(device, image);
}

bool
radv_layout_dcc_compressed(const struct radv_device *device, const struct radv_image *image, unsigned level,
                           VkImageLayout layout, unsigned queue_mask)
{
   if (!radv_dcc_enabled(image, level))
      return false;

   if (image->vk.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT && queue_mask & (1u << RADV_QUEUE_FOREIGN))
      return true;

   /* If the image is read-only, we can always just keep it compressed */
   if (!(image->vk.usage & RADV_IMAGE_USAGE_WRITE_BITS))
      return true;

   /* Don't compress compute transfer dst when image stores are not supported. */
   if ((layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || layout == VK_IMAGE_LAYOUT_GENERAL) &&
       (queue_mask & (1u << RADV_QUEUE_COMPUTE)) && !radv_image_use_dcc_image_stores(device, image))
      return false;

   /* Don't compress exclusive images used on transfer queues when SDMA doesn't support DCC.
    * Note that DCC is already disabled on concurrent images when not supported.
    */
   if (queue_mask == BITFIELD_BIT(RADV_QUEUE_TRANSFER) && !device->physical_device->rad_info.sdma_supports_compression)
      return false;

   if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
      /* Do not compress DCC with feedback loops because we can't read&write it without introducing
       * corruption.
       */
      return false;
   }

   return device->physical_device->rad_info.gfx_level >= GFX10 || layout != VK_IMAGE_LAYOUT_GENERAL;
}

enum radv_fmask_compression
radv_layout_fmask_compression(const struct radv_device *device, const struct radv_image *image, VkImageLayout layout,
                              unsigned queue_mask)
{
   if (!radv_image_has_fmask(image))
      return RADV_FMASK_COMPRESSION_NONE;

   if (layout == VK_IMAGE_LAYOUT_GENERAL)
      return RADV_FMASK_COMPRESSION_NONE;

   /* Don't compress compute transfer dst because image stores ignore FMASK and it needs to be
    * expanded before.
    */
   if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && (queue_mask & (1u << RADV_QUEUE_COMPUTE)))
      return RADV_FMASK_COMPRESSION_NONE;

   /* Compress images if TC-compat CMASK is enabled. */
   if (radv_image_is_tc_compat_cmask(image))
      return RADV_FMASK_COMPRESSION_FULL;

   switch (layout) {
   case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
   case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      /* Don't compress images but no need to expand FMASK. */
      return RADV_FMASK_COMPRESSION_PARTIAL;
   case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
      /* Don't compress images that are in feedback loops. */
      return RADV_FMASK_COMPRESSION_NONE;
   default:
      /* Don't compress images that are concurrent. */
      return queue_mask == (1u << RADV_QUEUE_GENERAL) ? RADV_FMASK_COMPRESSION_FULL : RADV_FMASK_COMPRESSION_NONE;
   }
}

unsigned
radv_image_queue_family_mask(const struct radv_image *image, enum radv_queue_family family,
                             enum radv_queue_family queue_family)
{
   if (!image->exclusive)
      return image->queue_family_mask;
   if (family == RADV_QUEUE_FOREIGN)
      return ((1u << RADV_MAX_QUEUE_FAMILIES) - 1u) | (1u << RADV_QUEUE_FOREIGN);
   if (family == RADV_QUEUE_IGNORED)
      return 1u << queue_family;
   return 1u << family;
}

bool
radv_image_is_renderable(const struct radv_device *device, const struct radv_image *image)
{
   if (image->vk.format == VK_FORMAT_R32G32B32_UINT || image->vk.format == VK_FORMAT_R32G32B32_SINT ||
       image->vk.format == VK_FORMAT_R32G32B32_SFLOAT)
      return false;

   if (device->physical_device->rad_info.gfx_level >= GFX9 && image->vk.image_type == VK_IMAGE_TYPE_3D &&
       vk_format_get_blocksizebits(image->vk.format) == 128 && vk_format_is_compressed(image->vk.format))
      return false;

   if (image->planes[0].surface.flags & RADEON_SURF_NO_RENDER_TARGET)
      return false;

   return true;
}

unsigned
radv_tile_mode_index(const struct radv_image_plane *plane, unsigned level, bool stencil)
{
   if (stencil)
      return plane->surface.u.legacy.zs.stencil_tiling_index[level];
   else
      return plane->surface.u.legacy.tiling_index[level];
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CreateImage(VkDevice _device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                 VkImage *pImage)
{
#ifdef ANDROID
   const VkNativeBufferANDROID *gralloc_info = vk_find_struct_const(pCreateInfo->pNext, NATIVE_BUFFER_ANDROID);

   if (gralloc_info)
      return radv_image_from_gralloc(_device, pCreateInfo, gralloc_info, pAllocator, pImage);
#endif

#ifdef RADV_USE_WSI_PLATFORM
   /* Ignore swapchain creation info on Android. Since we don't have an implementation in Mesa,
    * we're guaranteed to access an Android object incorrectly.
    */
   RADV_FROM_HANDLE(radv_device, device, _device);
   const VkImageSwapchainCreateInfoKHR *swapchain_info =
      vk_find_struct_const(pCreateInfo->pNext, IMAGE_SWAPCHAIN_CREATE_INFO_KHR);
   if (swapchain_info && swapchain_info->swapchain != VK_NULL_HANDLE) {
      return wsi_common_create_swapchain_image(device->physical_device->vk.wsi_device, pCreateInfo,
                                               swapchain_info->swapchain, pImage);
   }
#endif

   const struct wsi_image_create_info *wsi_info = vk_find_struct_const(pCreateInfo->pNext, WSI_IMAGE_CREATE_INFO_MESA);
   bool scanout = wsi_info && wsi_info->scanout;
   bool prime_blit_src = wsi_info && wsi_info->blit_src;

   return radv_image_create(_device,
                            &(struct radv_image_create_info){
                               .vk_info = pCreateInfo,
                               .scanout = scanout,
                               .prime_blit_src = prime_blit_src,
                            },
                            pAllocator, pImage, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_DestroyImage(VkDevice _device, VkImage _image, const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_image, image, _image);

   if (!image)
      return;

   radv_destroy_image(device, pAllocator, image);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_BindImageMemory2(VkDevice _device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos)
{
   RADV_FROM_HANDLE(radv_device, device, _device);

   for (uint32_t i = 0; i < bindInfoCount; ++i) {
      RADV_FROM_HANDLE(radv_device_memory, mem, pBindInfos[i].memory);
      RADV_FROM_HANDLE(radv_image, image, pBindInfos[i].image);
      VkBindMemoryStatusKHR *status = (void *)vk_find_struct_const(&pBindInfos[i], BIND_MEMORY_STATUS_KHR);

      if (status)
         *status->pResult = VK_SUCCESS;

         /* Ignore this struct on Android, we cannot access swapchain structures there. */
#ifdef RADV_USE_WSI_PLATFORM
      const VkBindImageMemorySwapchainInfoKHR *swapchain_info =
         vk_find_struct_const(pBindInfos[i].pNext, BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR);

      if (swapchain_info && swapchain_info->swapchain != VK_NULL_HANDLE) {
         struct radv_image *swapchain_img =
            radv_image_from_handle(wsi_common_get_image(swapchain_info->swapchain, swapchain_info->imageIndex));

         image->bindings[0].bo = swapchain_img->bindings[0].bo;
         image->bindings[0].offset = swapchain_img->bindings[0].offset;
         continue;
      }
#endif

      if (mem->alloc_size) {
         VkImageMemoryRequirementsInfo2 info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
            .image = pBindInfos[i].image,
         };
         VkMemoryRequirements2 reqs = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
         };

         radv_GetImageMemoryRequirements2(_device, &info, &reqs);

         if (pBindInfos[i].memoryOffset + reqs.memoryRequirements.size > mem->alloc_size) {
            if (status)
               *status->pResult = VK_ERROR_UNKNOWN;
            return vk_errorf(device, VK_ERROR_UNKNOWN, "Device memory object too small for the image.\n");
         }
      }

      if (image->disjoint) {
         const VkBindImagePlaneMemoryInfo *plane_info =
            vk_find_struct_const(pBindInfos[i].pNext, BIND_IMAGE_PLANE_MEMORY_INFO);

         switch (plane_info->planeAspect) {
         case VK_IMAGE_ASPECT_PLANE_0_BIT:
            image->bindings[0].bo = mem->bo;
            image->bindings[0].offset = pBindInfos[i].memoryOffset;
            break;
         case VK_IMAGE_ASPECT_PLANE_1_BIT:
            image->bindings[1].bo = mem->bo;
            image->bindings[1].offset = pBindInfos[i].memoryOffset;
            break;
         case VK_IMAGE_ASPECT_PLANE_2_BIT:
            image->bindings[2].bo = mem->bo;
            image->bindings[2].offset = pBindInfos[i].memoryOffset;
            break;
         default:
            break;
         }
      } else {
         image->bindings[0].bo = mem->bo;
         image->bindings[0].offset = pBindInfos[i].memoryOffset;
      }
      radv_rmv_log_image_bind(device, pBindInfos[i].image);
   }
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
radv_GetImageSubresourceLayout2KHR(VkDevice _device, VkImage _image, const VkImageSubresource2KHR *pSubresource,
                                   VkSubresourceLayout2KHR *pLayout)
{
   RADV_FROM_HANDLE(radv_image, image, _image);
   RADV_FROM_HANDLE(radv_device, device, _device);
   int level = pSubresource->imageSubresource.mipLevel;
   int layer = pSubresource->imageSubresource.arrayLayer;

   const unsigned plane_count = vk_format_get_plane_count(image->vk.format);
   unsigned plane_id = 0;
   if (plane_count > 1)
      plane_id = radv_plane_from_aspect(pSubresource->imageSubresource.aspectMask);

   struct radv_image_plane *plane = &image->planes[plane_id];
   struct radeon_surf *surface = &plane->surface;

   if (image->vk.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT && plane_count == 1) {
      unsigned mem_plane_id = radv_plane_from_aspect(pSubresource->imageSubresource.aspectMask);

      assert(level == 0);
      assert(layer == 0);

      pLayout->subresourceLayout.offset =
         ac_surface_get_plane_offset(device->physical_device->rad_info.gfx_level, surface, mem_plane_id, 0);
      pLayout->subresourceLayout.rowPitch =
         ac_surface_get_plane_stride(device->physical_device->rad_info.gfx_level, surface, mem_plane_id, level);
      pLayout->subresourceLayout.arrayPitch = 0;
      pLayout->subresourceLayout.depthPitch = 0;
      pLayout->subresourceLayout.size = ac_surface_get_plane_size(surface, mem_plane_id);
   } else if (device->physical_device->rad_info.gfx_level >= GFX9) {
      uint64_t level_offset = surface->is_linear ? surface->u.gfx9.offset[level] : 0;

      pLayout->subresourceLayout.offset =
         ac_surface_get_plane_offset(device->physical_device->rad_info.gfx_level, &plane->surface, 0, layer) +
         level_offset;
      if (image->vk.format == VK_FORMAT_R32G32B32_UINT || image->vk.format == VK_FORMAT_R32G32B32_SINT ||
          image->vk.format == VK_FORMAT_R32G32B32_SFLOAT) {
         /* Adjust the number of bytes between each row because
          * the pitch is actually the number of components per
          * row.
          */
         pLayout->subresourceLayout.rowPitch = surface->u.gfx9.surf_pitch * surface->bpe / 3;
      } else {
         uint32_t pitch = surface->is_linear ? surface->u.gfx9.pitch[level] : surface->u.gfx9.surf_pitch;

         assert(util_is_power_of_two_nonzero(surface->bpe));
         pLayout->subresourceLayout.rowPitch = pitch * surface->bpe;
      }

      pLayout->subresourceLayout.arrayPitch = surface->u.gfx9.surf_slice_size;
      pLayout->subresourceLayout.depthPitch = surface->u.gfx9.surf_slice_size;
      pLayout->subresourceLayout.size = surface->u.gfx9.surf_slice_size;
      if (image->vk.image_type == VK_IMAGE_TYPE_3D)
         pLayout->subresourceLayout.size *= u_minify(image->vk.extent.depth, level);
   } else {
      pLayout->subresourceLayout.offset = (uint64_t)surface->u.legacy.level[level].offset_256B * 256 +
                                          (uint64_t)surface->u.legacy.level[level].slice_size_dw * 4 * layer;
      pLayout->subresourceLayout.rowPitch = surface->u.legacy.level[level].nblk_x * surface->bpe;
      pLayout->subresourceLayout.arrayPitch = (uint64_t)surface->u.legacy.level[level].slice_size_dw * 4;
      pLayout->subresourceLayout.depthPitch = (uint64_t)surface->u.legacy.level[level].slice_size_dw * 4;
      pLayout->subresourceLayout.size = (uint64_t)surface->u.legacy.level[level].slice_size_dw * 4;
      if (image->vk.image_type == VK_IMAGE_TYPE_3D)
         pLayout->subresourceLayout.size *= u_minify(image->vk.extent.depth, level);
   }

   VkImageCompressionPropertiesEXT *image_compression_props =
      vk_find_struct(pLayout->pNext, IMAGE_COMPRESSION_PROPERTIES_EXT);
   if (image_compression_props) {
      image_compression_props->imageCompressionFixedRateFlags = VK_IMAGE_COMPRESSION_FIXED_RATE_NONE_EXT;

      if (image->vk.aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
         image_compression_props->imageCompressionFlags =
            radv_image_has_htile(image) ? VK_IMAGE_COMPRESSION_DEFAULT_EXT : VK_IMAGE_COMPRESSION_DISABLED_EXT;
      } else {
         image_compression_props->imageCompressionFlags =
            radv_image_has_dcc(image) ? VK_IMAGE_COMPRESSION_DEFAULT_EXT : VK_IMAGE_COMPRESSION_DISABLED_EXT;
      }
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_GetImageDrmFormatModifierPropertiesEXT(VkDevice _device, VkImage _image,
                                            VkImageDrmFormatModifierPropertiesEXT *pProperties)
{
   RADV_FROM_HANDLE(radv_image, image, _image);

   pProperties->drmFormatModifier = image->planes[0].surface.modifier;
   return VK_SUCCESS;
}
