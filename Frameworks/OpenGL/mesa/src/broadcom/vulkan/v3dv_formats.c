/*
 * Copyright © 2019 Raspberry Pi Ltd
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

#include "v3dv_private.h"
#ifdef ANDROID
#include "vk_android.h"
#endif
#include "vk_enum_defines.h"
#include "vk_util.h"

#include "drm-uapi/drm_fourcc.h"
#include "util/format/u_format.h"
#include "vulkan/wsi/wsi_common.h"

#include <vulkan/vulkan_android.h>

const uint8_t *
v3dv_get_format_swizzle(struct v3dv_device *device, VkFormat f, uint8_t plane)
{
   const struct v3dv_format *vf = v3dv_X(device, get_format)(f);
   static const uint8_t fallback[] = {0, 1, 2, 3};

   if (!vf)
      return fallback;

   return vf->planes[plane].swizzle;
}

bool
v3dv_format_swizzle_needs_rb_swap(const uint8_t *swizzle)
{
   /* Normal case */
   if (swizzle[0] == PIPE_SWIZZLE_Z)
      return swizzle[2] == PIPE_SWIZZLE_X;

   /* Format uses reverse flag */
   if (swizzle[0] == PIPE_SWIZZLE_Y)
      return swizzle[2] == PIPE_SWIZZLE_W;

   return false;
}

bool
v3dv_format_swizzle_needs_reverse(const uint8_t *swizzle)
{
   /* Normal case */
   if (swizzle[0] == PIPE_SWIZZLE_W &&
       swizzle[1] == PIPE_SWIZZLE_Z &&
       swizzle[2] == PIPE_SWIZZLE_Y &&
       swizzle[3] == PIPE_SWIZZLE_X) {
      return true;
   }

   /* Format uses RB swap flag */
   if (swizzle[0] == PIPE_SWIZZLE_Y &&
       swizzle[1] == PIPE_SWIZZLE_Z &&
       swizzle[2] == PIPE_SWIZZLE_W &&
       swizzle[3] == PIPE_SWIZZLE_X) {
      return true;
   }

   return false;
}

/* Some cases of transfer operations are raw data copies that don't depend
 * on the semantics of the pixel format (no pixel format conversions are
 * involved). In these cases, it is safe to choose any format supported by
 * the TFU so long as it has the same texel size, which allows us to use the
 * TFU paths with formats that are not TFU supported otherwise.
 *
 * Even when copying multi-plane images, we are copying per-plane, so the
 * compatible TFU format will be single-plane.
 */
const struct v3dv_format *
v3dv_get_compatible_tfu_format(struct v3dv_device *device,
                               uint32_t bpp,
                               VkFormat *out_vk_format)
{
   VkFormat vk_format;
   switch (bpp) {
   case 16: vk_format = VK_FORMAT_R32G32B32A32_SFLOAT;  break;
   case 8:  vk_format = VK_FORMAT_R16G16B16A16_SFLOAT;  break;
   case 4:  vk_format = VK_FORMAT_R32_SFLOAT;           break;
   case 2:  vk_format = VK_FORMAT_R16_SFLOAT;           break;
   case 1:  vk_format = VK_FORMAT_R8_UNORM;             break;
   default: unreachable("unsupported format bit-size"); break;
   };

   if (out_vk_format)
      *out_vk_format = vk_format;

   const struct v3dv_format *format = v3dv_X(device, get_format)(vk_format);
   assert(format->plane_count == 1);
   assert(v3dv_X(device, tfu_supports_tex_format)(format->planes[0].tex_type));

   return format;
}

static VkFormatFeatureFlags2
image_format_plane_features(struct v3dv_physical_device *pdevice,
                            VkFormat vk_format,
                            const struct v3dv_format_plane *v3dv_format,
                            VkImageTiling tiling)
{
   const VkImageAspectFlags aspects = vk_format_aspects(vk_format);

   const VkImageAspectFlags zs_aspects = VK_IMAGE_ASPECT_DEPTH_BIT |
                                         VK_IMAGE_ASPECT_STENCIL_BIT;
   const VkImageAspectFlags supported_aspects = VK_IMAGE_ASPECT_COLOR_BIT |
                                                zs_aspects;
   if ((aspects & supported_aspects) != aspects)
      return 0;

   /* FIXME: We don't support separate stencil yet */
   if ((aspects & zs_aspects) == VK_IMAGE_ASPECT_STENCIL_BIT)
      return 0;

   if (v3dv_format->tex_type == TEXTURE_DATA_FORMAT_NO &&
       v3dv_format->rt_type == V3D_OUTPUT_IMAGE_FORMAT_NO) {
      return 0;
   }

   VkFormatFeatureFlags2 flags = 0;

   /* Raster format is only supported for 1D textures, so let's just
    * always require optimal tiling for anything that requires sampling.
    * Note: even if the user requests optimal for a 1D image, we will still
    * use raster format since that is what the HW requires.
    */
   if (v3dv_format->tex_type != TEXTURE_DATA_FORMAT_NO &&
       tiling == VK_IMAGE_TILING_OPTIMAL) {
      flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT |
               VK_FORMAT_FEATURE_2_BLIT_SRC_BIT;

   }

   if (v3dv_format->rt_type != V3D_OUTPUT_IMAGE_FORMAT_NO) {
      if (aspects & VK_IMAGE_ASPECT_COLOR_BIT) {
         flags |= VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT |
                  VK_FORMAT_FEATURE_2_BLIT_DST_BIT;
      } else if (aspects & zs_aspects) {
         flags |= VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT |
                  VK_FORMAT_FEATURE_2_BLIT_DST_BIT;
      }
   }

   const struct util_format_description *desc =
      vk_format_description(vk_format);

   if (tiling != VK_IMAGE_TILING_LINEAR) {
      if (desc->layout == UTIL_FORMAT_LAYOUT_PLAIN && desc->is_array) {
         flags |= VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT |
                  VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT;
         if (desc->nr_channels == 1 && vk_format_is_int(vk_format))
            flags |= VK_FORMAT_FEATURE_2_STORAGE_IMAGE_ATOMIC_BIT;
      } else if (vk_format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 ||
                 vk_format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 ||
                 vk_format == VK_FORMAT_A2B10G10R10_UINT_PACK32 ||
                 vk_format == VK_FORMAT_B10G11R11_UFLOAT_PACK32) {
         /* To comply with shaderStorageImageExtendedFormats */
         flags |= VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT |
                  VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT;
      }
   }

   /* All our depth formats support shadow comparisons. */
   if (vk_format_has_depth(vk_format) &&
       (flags & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT)) {
      flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT;
   }

   if (flags) {
      flags |= VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT |
               VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT;
   }

   return flags;
}

static VkFormatFeatureFlags2
image_format_features(struct v3dv_physical_device *pdevice,
                       VkFormat vk_format,
                       const struct v3dv_format *v3dv_format,
                       VkImageTiling tiling)
{
   if (!v3dv_format || !v3dv_format->plane_count)
      return 0;

   VkFormatFeatureFlags2 flags = ~0ull;
   for (uint8_t plane = 0;
        flags && plane < v3dv_format->plane_count;
        plane++) {
      VkFormat plane_format = vk_format_get_plane_format(vk_format, plane);

      flags &= image_format_plane_features(pdevice,
                                           plane_format,
                                           &v3dv_format->planes[plane],
                                           tiling);
   }

   const struct vk_format_ycbcr_info *ycbcr_info =
      vk_format_get_ycbcr_info(vk_format);

   if (ycbcr_info) {
      assert(v3dv_format->plane_count == ycbcr_info->n_planes);

      flags |= VK_FORMAT_FEATURE_2_DISJOINT_BIT;

      if (flags & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT) {
         flags |= VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT;
         for (unsigned p = 0; p < ycbcr_info->n_planes; p++) {
            if (ycbcr_info->planes[p].denominator_scales[0] > 1 ||
                ycbcr_info->planes[p].denominator_scales[1] > 1) {
               flags |= VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
               break;
            }
         }
      }

      /* FIXME: in the future we should be able to support BLIT_SRC via the
       * blit_shader path
       */
      const VkFormatFeatureFlags2 disallowed_ycbcr_image_features =
         VK_FORMAT_FEATURE_2_BLIT_SRC_BIT |
         VK_FORMAT_FEATURE_2_BLIT_DST_BIT |
         VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT |
         VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT |
         VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT;

      flags &= ~disallowed_ycbcr_image_features;
   }

   if (flags & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT &&
       v3dv_format->supports_filtering) {
      flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
   }

   if (flags & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT &&
       v3dv_X(pdevice, format_supports_blending)(v3dv_format)) {
      flags |= VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT;
   }

   return flags;
}

static VkFormatFeatureFlags2
buffer_format_features(VkFormat vk_format, const struct v3dv_format *v3dv_format)
{
   if (!v3dv_format)
      return 0;

   if (v3dv_format->plane_count != 1)
      return 0;

   /* We probably only want to support buffer formats that have a
    * color format specification.
    */
   if (!vk_format_is_color(vk_format))
      return 0;

   const struct util_format_description *desc =
      vk_format_description(vk_format);

   VkFormatFeatureFlags2 flags = 0;
   if (desc->layout == UTIL_FORMAT_LAYOUT_PLAIN &&
       desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB &&
       desc->is_array) {
      flags |=  VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT;
      if (v3dv_format->planes[0].tex_type != TEXTURE_DATA_FORMAT_NO) {
         /* STORAGE_READ_WITHOUT_FORMAT can also be applied for buffers. From spec:
          *   "VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT specifies
          *    that image views or buffer views created with this format can
          *    be used as storage images for read operations without
          *    specifying a format."
          */
         flags |= VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT |
                  VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT |
                  VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT;
      }
   } else if (vk_format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 ||
              vk_format == VK_FORMAT_A2R10G10B10_UNORM_PACK32) {
      flags |= VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT |
               VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT |
               VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT;
   } else if (vk_format == VK_FORMAT_A2B10G10R10_UINT_PACK32 ||
              vk_format == VK_FORMAT_B10G11R11_UFLOAT_PACK32) {
      flags |= VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT |
               VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT;
   }

   if (desc->layout == UTIL_FORMAT_LAYOUT_PLAIN &&
       desc->is_array &&
       desc->nr_channels == 1 &&
       vk_format_is_int(vk_format)) {
      flags |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_ATOMIC_BIT;
   }

   return flags;
}

bool
v3dv_buffer_format_supports_features(struct v3dv_device *device,
                                     VkFormat vk_format,
                                     VkFormatFeatureFlags2 features)
{
   const struct v3dv_format *v3dv_format = v3dv_X(device, get_format)(vk_format);
   const VkFormatFeatureFlags2 supported =
      buffer_format_features(vk_format, v3dv_format);
   return (supported & features) == features;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice,
                                        VkFormat format,
                                        VkFormatProperties2 *pFormatProperties)
{
   V3DV_FROM_HANDLE(v3dv_physical_device, pdevice, physicalDevice);
   const struct v3dv_format *v3dv_format = v3dv_X(pdevice, get_format)(format);

   VkFormatFeatureFlags2 linear2, optimal2, buffer2;
   linear2 = image_format_features(pdevice, format, v3dv_format,
                                   VK_IMAGE_TILING_LINEAR);
   optimal2 = image_format_features(pdevice, format, v3dv_format,
                                    VK_IMAGE_TILING_OPTIMAL);
   buffer2 = buffer_format_features(format, v3dv_format);
   pFormatProperties->formatProperties = (VkFormatProperties) {
      .linearTilingFeatures = vk_format_features2_to_features(linear2),
      .optimalTilingFeatures = vk_format_features2_to_features(optimal2),
      .bufferFeatures = vk_format_features2_to_features(buffer2),
   };

   vk_foreach_struct(ext, pFormatProperties->pNext) {
      switch ((unsigned)ext->sType) {
      case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT: {
         struct VkDrmFormatModifierPropertiesListEXT *list = (void *)ext;
         VK_OUTARRAY_MAKE_TYPED(VkDrmFormatModifierPropertiesEXT, out,
                                list->pDrmFormatModifierProperties,
                                &list->drmFormatModifierCount);
         if (pFormatProperties->formatProperties.linearTilingFeatures) {
            vk_outarray_append_typed(VkDrmFormatModifierPropertiesEXT,
                                     &out, mod_props) {
               mod_props->drmFormatModifier = DRM_FORMAT_MOD_LINEAR;
               mod_props->drmFormatModifierPlaneCount = 1;
               mod_props->drmFormatModifierTilingFeatures =
                  pFormatProperties->formatProperties.linearTilingFeatures;
            }
         }
         if (pFormatProperties->formatProperties.optimalTilingFeatures) {
            vk_outarray_append_typed(VkDrmFormatModifierPropertiesEXT,
                                     &out, mod_props) {
               mod_props->drmFormatModifier = DRM_FORMAT_MOD_BROADCOM_UIF;
               mod_props->drmFormatModifierPlaneCount = 1;
               mod_props->drmFormatModifierTilingFeatures =
                  pFormatProperties->formatProperties.optimalTilingFeatures;
            }
         }
         break;
      }
      case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT: {
         struct VkDrmFormatModifierPropertiesList2EXT *list = (void *)ext;
         VK_OUTARRAY_MAKE_TYPED(VkDrmFormatModifierProperties2EXT, out,
                                list->pDrmFormatModifierProperties,
                                &list->drmFormatModifierCount);
         if (linear2) {
            vk_outarray_append_typed(VkDrmFormatModifierProperties2EXT,
                                     &out, mod_props) {
               mod_props->drmFormatModifier = DRM_FORMAT_MOD_LINEAR;
               mod_props->drmFormatModifierPlaneCount = 1;
               mod_props->drmFormatModifierTilingFeatures = linear2;
            }
         }
         if (optimal2) {
            vk_outarray_append_typed(VkDrmFormatModifierProperties2EXT,
                                     &out, mod_props) {
               mod_props->drmFormatModifier = DRM_FORMAT_MOD_BROADCOM_UIF;
               mod_props->drmFormatModifierPlaneCount = 1;
               mod_props->drmFormatModifierTilingFeatures = optimal2;
            }
         }
         break;
      }
      case VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3: {
         VkFormatProperties3 *props = (VkFormatProperties3 *)ext;
         props->linearTilingFeatures = linear2;
         props->optimalTilingFeatures = optimal2;
         props->bufferFeatures = buffer2;
         break;
      }
      default:
         v3dv_debug_ignored_stype(ext->sType);
         break;
      }
   }
}

static VkResult
get_image_format_properties(
   struct v3dv_physical_device *physical_device,
   const VkPhysicalDeviceImageFormatInfo2 *info,
   VkImageTiling tiling,
   VkImageFormatProperties *pImageFormatProperties,
   VkSamplerYcbcrConversionImageFormatProperties *pYcbcrImageFormatProperties)
{
   const struct v3dv_format *v3dv_format = v3dv_X(physical_device, get_format)(info->format);
   VkFormatFeatureFlags2 format_feature_flags =
      image_format_features(physical_device, info->format, v3dv_format, tiling);
   if (!format_feature_flags)
      goto unsupported;

   /* This allows users to create uncompressed views of compressed images,
    * however this is not something the hardware supports naturally and requires
    * the driver to lie when programming the texture state to make the hardware
    * sample with the uncompressed view correctly, and even then, there are
    * issues when running on real hardware.
    *
    * See https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/11336
    * for details.
    */
   if (info->flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT)
      goto unsupported;

   const VkImageStencilUsageCreateInfo *stencil_usage_info =
      vk_find_struct_const(info->pNext, IMAGE_STENCIL_USAGE_CREATE_INFO);

   VkImageUsageFlags image_usage =
      info->usage | (stencil_usage_info ? stencil_usage_info->stencilUsage : 0);

   /* If VK_IMAGE_CREATE_EXTENDED_USAGE_BIT is set it means the usage flags may
    * not be be supported for the image format but are supported for at least
    * one compatible format from which an image view can be created for the
    * image. This means we should not report the format as unsupported based
    * on the usage flags when usage refers to how an image view may be used
    * (i.e. as a framebuffer attachment, for sampling, etc).
    */
   VkImageUsageFlags view_usage =
      info->flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT ? 0 : image_usage;

   if (image_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
      if (!(format_feature_flags & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT)) {
         goto unsupported;
      }

      /* Sampling of raster depth/stencil images is not supported. Since 1D
       * images are always raster, even if the user requested optimal tiling,
       * we can't have them be used as transfer sources, since that includes
       * using them for blit sources, which might require sampling.
       */
      if (info->type == VK_IMAGE_TYPE_1D &&
          vk_format_is_depth_or_stencil(info->format)) {
         goto unsupported;
      }
   }

   if (image_usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
      if (!(format_feature_flags & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT)) {
         goto unsupported;
      }
   }

   if (view_usage & (VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
      if (!(format_feature_flags & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT))
         goto unsupported;

      /* Sampling of raster depth/stencil images is not supported. Since 1D
       * images are always raster, even if the user requested optimal tiling,
       * we can't allow sampling if the format is depth/stencil.
       */
      if (info->type == VK_IMAGE_TYPE_1D &&
          vk_format_is_depth_or_stencil(info->format)) {
         goto unsupported;
      }
   }

   if (view_usage & VK_IMAGE_USAGE_STORAGE_BIT) {
      if (!(format_feature_flags & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT)) {
         goto unsupported;
      }
   }

   if (view_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
      if (!(format_feature_flags & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT)) {
         goto unsupported;
      }
   }

   if (view_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      if (!(format_feature_flags &
            VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT)) {
         goto unsupported;
      }
   }

   switch (info->type) {
   case VK_IMAGE_TYPE_1D:
      pImageFormatProperties->maxExtent.width = V3D_MAX_IMAGE_DIMENSION;
      pImageFormatProperties->maxExtent.height = 1;
      pImageFormatProperties->maxExtent.depth = 1;
      pImageFormatProperties->maxArrayLayers = V3D_MAX_ARRAY_LAYERS;
      pImageFormatProperties->maxMipLevels = V3D_MAX_MIP_LEVELS;
      break;
   case VK_IMAGE_TYPE_2D:
      pImageFormatProperties->maxExtent.width = V3D_MAX_IMAGE_DIMENSION;
      pImageFormatProperties->maxExtent.height = V3D_MAX_IMAGE_DIMENSION;
      pImageFormatProperties->maxExtent.depth = 1;
      pImageFormatProperties->maxArrayLayers =
         v3dv_format->plane_count == 1 ? V3D_MAX_ARRAY_LAYERS : 1;
      pImageFormatProperties->maxMipLevels = V3D_MAX_MIP_LEVELS;
      break;
   case VK_IMAGE_TYPE_3D:
      pImageFormatProperties->maxExtent.width = V3D_MAX_IMAGE_DIMENSION;
      pImageFormatProperties->maxExtent.height = V3D_MAX_IMAGE_DIMENSION;
      pImageFormatProperties->maxExtent.depth = V3D_MAX_IMAGE_DIMENSION;
      pImageFormatProperties->maxArrayLayers = 1;
      pImageFormatProperties->maxMipLevels = V3D_MAX_MIP_LEVELS;
      break;
   default:
      unreachable("bad VkImageType");
   }

   /* Our hw doesn't support 1D compressed textures. */
   if (info->type == VK_IMAGE_TYPE_1D &&
       vk_format_is_compressed(info->format)) {
       goto unsupported;
   }

   /* From the Vulkan 1.0 spec, section 34.1.1. Supported Sample Counts:
    *
    * sampleCounts will be set to VK_SAMPLE_COUNT_1_BIT if at least one of the
    * following conditions is true:
    *
    *   - tiling is VK_IMAGE_TILING_LINEAR
    *   - type is not VK_IMAGE_TYPE_2D
    *   - flags contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
    *   - neither the VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT flag nor the
    *     VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT flag in
    *     VkFormatProperties::optimalTilingFeatures returned by
    *     vkGetPhysicalDeviceFormatProperties is set.
    */
   pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
   if (tiling != VK_IMAGE_TILING_LINEAR &&
       info->type == VK_IMAGE_TYPE_2D &&
       !(info->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) &&
       (format_feature_flags & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT ||
        format_feature_flags & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT)) {
      pImageFormatProperties->sampleCounts |= VK_SAMPLE_COUNT_4_BIT;
   }

   if (tiling == VK_IMAGE_TILING_LINEAR)
      pImageFormatProperties->maxMipLevels = 1;

   /* From the Vulkan 1.2 spec, section 12.3. Images, VkImageCreateInfo structure:
    *
    *   "Images created with one of the formats that require a sampler Y′CBCR
    *    conversion, have further restrictions on their limits and
    *    capabilities compared to images created with other formats. Creation
    *    of images with a format requiring Y′CBCR conversion may not be
    *    supported unless other parameters meet all of the constraints:
    *
    *    * imageType is VK_IMAGE_TYPE_2D
    *    * mipLevels is 1
    *    * arrayLayers is 1, unless the ycbcrImageArrays feature is enabled, or
    *      otherwise indicated by VkImageFormatProperties::maxArrayLayers, as
    *      returned by vkGetPhysicalDeviceImageFormatProperties
    *    * samples is VK_SAMPLE_COUNT_1_BIT
    *
    * Implementations may support additional limits and capabilities beyond
    * those listed above."
    *
    * We don't provide such additional limits, so we set those limits, or just
    * return unsupported.
    */
   if (vk_format_get_plane_count(info->format) > 1) {
      if (info->type != VK_IMAGE_TYPE_2D)
         goto unsupported;
      pImageFormatProperties->maxMipLevels = 1;
      pImageFormatProperties->maxArrayLayers = 1;
      pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
   }

   pImageFormatProperties->maxResourceSize = 0xffffffff; /* 32-bit allocation */

   if (pYcbcrImageFormatProperties) {
      pYcbcrImageFormatProperties->combinedImageSamplerDescriptorCount =
          vk_format_get_plane_count(info->format);
   }

   return VK_SUCCESS;

unsupported:
   *pImageFormatProperties = (VkImageFormatProperties) {
      .maxExtent = { 0, 0, 0 },
      .maxMipLevels = 0,
      .maxArrayLayers = 0,
      .sampleCounts = 0,
      .maxResourceSize = 0,
   };

   return VK_ERROR_FORMAT_NOT_SUPPORTED;
}

static const VkExternalMemoryProperties prime_fd_props = {
   .externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT |
                             VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT,
   .exportFromImportedHandleTypes =
      VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT |
      VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
   .compatibleHandleTypes =
      VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT |
      VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
};

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_GetPhysicalDeviceImageFormatProperties(
   VkPhysicalDevice physicalDevice,
   VkFormat format,
   VkImageType type,
   VkImageTiling tiling,
   VkImageUsageFlags usage,
   VkImageCreateFlags createFlags,
   VkImageFormatProperties *pImageFormatProperties)
{
   V3DV_FROM_HANDLE(v3dv_physical_device, physical_device, physicalDevice);

   const VkPhysicalDeviceImageFormatInfo2 info = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
      .pNext = NULL,
      .format = format,
      .type = type,
      .tiling = tiling,
      .usage = usage,
      .flags = createFlags,
   };

   return get_image_format_properties(physical_device, &info, tiling,
                                      pImageFormatProperties, NULL);
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                             const VkPhysicalDeviceImageFormatInfo2 *base_info,
                                             VkImageFormatProperties2 *base_props)
{
   V3DV_FROM_HANDLE(v3dv_physical_device, physical_device, physicalDevice);
   const VkPhysicalDeviceExternalImageFormatInfo *external_info = NULL;
   const VkPhysicalDeviceImageDrmFormatModifierInfoEXT *drm_format_mod_info = NULL;
   VkExternalImageFormatProperties *external_props = NULL;
   UNUSED VkAndroidHardwareBufferUsageANDROID *android_usage = NULL;
   VkSamplerYcbcrConversionImageFormatProperties *ycbcr_props = NULL;
   VkImageTiling tiling = base_info->tiling;

   /* Extract input structs */
   vk_foreach_struct_const(s, base_info->pNext) {
      switch (s->sType) {
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO:
         external_info = (const void *) s;
         break;
      case VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO:
         /* Do nothing, get_image_format_properties() below will handle it */;
         break;
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT:
         drm_format_mod_info = (const void *) s;
         switch (drm_format_mod_info->drmFormatModifier) {
         case DRM_FORMAT_MOD_LINEAR:
            tiling = VK_IMAGE_TILING_LINEAR;
            break;
         case DRM_FORMAT_MOD_BROADCOM_UIF:
            tiling = VK_IMAGE_TILING_OPTIMAL;
            break;
         default:
            assert("Unknown DRM format modifier");
         }
         break;
      default:
         v3dv_debug_ignored_stype(s->sType);
         break;
      }
   }

   assert(tiling == VK_IMAGE_TILING_OPTIMAL ||
          tiling == VK_IMAGE_TILING_LINEAR);

   /* Extract output structs */
   vk_foreach_struct(s, base_props->pNext) {
      switch (s->sType) {
      case VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES:
         external_props = (void *) s;
         break;
      case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID:
         android_usage = (void *)s;
         break;
      case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES:
         ycbcr_props = (void *) s;
         break;
      default:
         v3dv_debug_ignored_stype(s->sType);
         break;
      }
   }

   VkResult result =
      get_image_format_properties(physical_device, base_info, tiling,
                                  &base_props->imageFormatProperties,
                                  ycbcr_props);
   if (result != VK_SUCCESS)
      goto done;

   if (external_info && external_info->handleType != 0) {
      switch (external_info->handleType) {
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT:
         if (external_props)
            external_props->externalMemoryProperties = prime_fd_props;
         break;
#ifdef ANDROID
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID:
         if (external_props) {
            external_props->externalMemoryProperties.exportFromImportedHandleTypes = 0;
            external_props->externalMemoryProperties.compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
            external_props->externalMemoryProperties.externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
         }
         break;
#endif
      default:
         result = VK_ERROR_FORMAT_NOT_SUPPORTED;
         break;
      }
   }

   if (android_usage) {
#ifdef ANDROID
      android_usage->androidHardwareBufferUsage =
         vk_image_usage_to_ahb_usage(base_info->flags, base_info->usage);
#endif
   }

done:
   return result;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_GetPhysicalDeviceSparseImageFormatProperties(
   VkPhysicalDevice physicalDevice,
   VkFormat format,
   VkImageType type,
   VkSampleCountFlagBits samples,
   VkImageUsageFlags usage,
   VkImageTiling tiling,
   uint32_t *pPropertyCount,
   VkSparseImageFormatProperties *pProperties)
{
   *pPropertyCount = 0;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_GetPhysicalDeviceSparseImageFormatProperties2(
   VkPhysicalDevice physicalDevice,
   const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
   uint32_t *pPropertyCount,
   VkSparseImageFormatProperties2 *pProperties)
{
   *pPropertyCount = 0;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_GetPhysicalDeviceExternalBufferProperties(
   VkPhysicalDevice physicalDevice,
   const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
   VkExternalBufferProperties *pExternalBufferProperties)
{
   switch (pExternalBufferInfo->handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT:
      pExternalBufferProperties->externalMemoryProperties = prime_fd_props;
      return;
   default: /* Unsupported */
      pExternalBufferProperties->externalMemoryProperties =
         (VkExternalMemoryProperties) {
            .compatibleHandleTypes = pExternalBufferInfo->handleType,
         };
      break;
   }
}
