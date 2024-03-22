/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_format.h"

#include "nvk_buffer_view.h"
#include "nvk_entrypoints.h"
#include "nvk_image.h"
#include "nvk_physical_device.h"

#include "vk_enum_defines.h"
#include "vk_format.h"

#include "nvtypes.h"
#include "cl902d.h"
#include "cl9097.h"
#include "cl90c0.h"

#define VA_FMT(vk_fmt, widths, swap_rb, type) \
   [VK_FORMAT_##vk_fmt] = \
   { NV9097_SET_VERTEX_ATTRIBUTE_A_COMPONENT_BIT_WIDTHS_##widths, \
     NV9097_SET_VERTEX_ATTRIBUTE_A_SWAP_R_AND_B_##swap_rb, \
     NV9097_SET_VERTEX_ATTRIBUTE_A_NUMERICAL_TYPE_NUM_##type }

static const struct nvk_va_format nvk_vf_formats[] = {
   VA_FMT(R8_UNORM,                    R8,               FALSE,   UNORM),
   VA_FMT(R8_SNORM,                    R8,               FALSE,   SNORM),
   VA_FMT(R8_USCALED,                  R8,               FALSE,   USCALED),
   VA_FMT(R8_SSCALED,                  R8,               FALSE,   SSCALED),
   VA_FMT(R8_UINT,                     R8,               FALSE,   UINT),
   VA_FMT(R8_SINT,                     R8,               FALSE,   SINT),

   VA_FMT(R8G8_UNORM,                  R8_G8,            FALSE,   UNORM),
   VA_FMT(R8G8_SNORM,                  R8_G8,            FALSE,   SNORM),
   VA_FMT(R8G8_USCALED,                R8_G8,            FALSE,   USCALED),
   VA_FMT(R8G8_SSCALED,                R8_G8,            FALSE,   SSCALED),
   VA_FMT(R8G8_UINT,                   R8_G8,            FALSE,   UINT),
   VA_FMT(R8G8_SINT,                   R8_G8,            FALSE,   SINT),

   VA_FMT(R8G8B8_UNORM,                R8_G8_B8,         FALSE,   UNORM),
   VA_FMT(R8G8B8_SNORM,                R8_G8_B8,         FALSE,   SNORM),
   VA_FMT(R8G8B8_USCALED,              R8_G8_B8,         FALSE,   USCALED),
   VA_FMT(R8G8B8_SSCALED,              R8_G8_B8,         FALSE,   SSCALED),
   VA_FMT(R8G8B8_UINT,                 R8_G8_B8,         FALSE,   UINT),
   VA_FMT(R8G8B8_SINT,                 R8_G8_B8,         FALSE,   SINT),

   VA_FMT(B8G8R8_UNORM,                R8_G8_B8,         TRUE,    UNORM),
   VA_FMT(B8G8R8_SNORM,                R8_G8_B8,         TRUE,    SNORM),
   VA_FMT(B8G8R8_USCALED,              R8_G8_B8,         TRUE,    USCALED),
   VA_FMT(B8G8R8_SSCALED,              R8_G8_B8,         TRUE,    SSCALED),
   VA_FMT(B8G8R8_UINT,                 R8_G8_B8,         TRUE,    UINT),
   VA_FMT(B8G8R8_SINT,                 R8_G8_B8,         TRUE,    SINT),

   VA_FMT(R8G8B8A8_UNORM,              R8_G8_B8_A8,      FALSE,   UNORM),
   VA_FMT(R8G8B8A8_SNORM,              R8_G8_B8_A8,      FALSE,   SNORM),
   VA_FMT(R8G8B8A8_USCALED,            R8_G8_B8_A8,      FALSE,   USCALED),
   VA_FMT(R8G8B8A8_SSCALED,            R8_G8_B8_A8,      FALSE,   SSCALED),
   VA_FMT(R8G8B8A8_UINT,               R8_G8_B8_A8,      FALSE,   UINT),
   VA_FMT(R8G8B8A8_SINT,               R8_G8_B8_A8,      FALSE,   SINT),

   VA_FMT(B8G8R8A8_UNORM,              R8_G8_B8_A8,      TRUE,   UNORM),
   VA_FMT(B8G8R8A8_SNORM,              R8_G8_B8_A8,      TRUE,   SNORM),
   VA_FMT(B8G8R8A8_USCALED,            R8_G8_B8_A8,      TRUE,   USCALED),
   VA_FMT(B8G8R8A8_SSCALED,            R8_G8_B8_A8,      TRUE,   SSCALED),
   VA_FMT(B8G8R8A8_UINT,               R8_G8_B8_A8,      TRUE,   UINT),
   VA_FMT(B8G8R8A8_SINT,               R8_G8_B8_A8,      TRUE,   SINT),

   VA_FMT(A8B8G8R8_UNORM_PACK32,       R8_G8_B8_A8,      FALSE,  UNORM),
   VA_FMT(A8B8G8R8_SNORM_PACK32,       R8_G8_B8_A8,      FALSE,  SNORM),
   VA_FMT(A8B8G8R8_USCALED_PACK32,     R8_G8_B8_A8,      FALSE,  USCALED),
   VA_FMT(A8B8G8R8_SSCALED_PACK32,     R8_G8_B8_A8,      FALSE,  SSCALED),
   VA_FMT(A8B8G8R8_UINT_PACK32,        R8_G8_B8_A8,      FALSE,  UINT),
   VA_FMT(A8B8G8R8_SINT_PACK32,        R8_G8_B8_A8,      FALSE,  SINT),

   VA_FMT(A2R10G10B10_UNORM_PACK32,    A2B10G10R10,      TRUE,    UNORM),
   VA_FMT(A2R10G10B10_SNORM_PACK32,    A2B10G10R10,      TRUE,    SNORM),
   VA_FMT(A2R10G10B10_USCALED_PACK32,  A2B10G10R10,      TRUE,    USCALED),
   VA_FMT(A2R10G10B10_SSCALED_PACK32,  A2B10G10R10,      TRUE,    SSCALED),
   VA_FMT(A2R10G10B10_UINT_PACK32,     A2B10G10R10,      TRUE,    UINT),
   VA_FMT(A2R10G10B10_SINT_PACK32,     A2B10G10R10,      TRUE,    SINT),

   VA_FMT(A2B10G10R10_UNORM_PACK32,    A2B10G10R10,      FALSE,   UNORM),
   VA_FMT(A2B10G10R10_SNORM_PACK32,    A2B10G10R10,      FALSE,   SNORM),
   VA_FMT(A2B10G10R10_USCALED_PACK32,  A2B10G10R10,      FALSE,   USCALED),
   VA_FMT(A2B10G10R10_SSCALED_PACK32,  A2B10G10R10,      FALSE,   SSCALED),
   VA_FMT(A2B10G10R10_UINT_PACK32,     A2B10G10R10,      FALSE,   UINT),
   VA_FMT(A2B10G10R10_SINT_PACK32,     A2B10G10R10,      FALSE,   SINT),

   VA_FMT(B10G11R11_UFLOAT_PACK32,     B10G11R11,        FALSE,   FLOAT),

   VA_FMT(R16_UNORM,                   R16,              FALSE,   UNORM),
   VA_FMT(R16_SNORM,                   R16,              FALSE,   SNORM),
   VA_FMT(R16_USCALED,                 R16,              FALSE,   USCALED),
   VA_FMT(R16_SSCALED,                 R16,              FALSE,   SSCALED),
   VA_FMT(R16_UINT,                    R16,              FALSE,   UINT),
   VA_FMT(R16_SINT,                    R16,              FALSE,   SINT),
   VA_FMT(R16_SFLOAT,                  R16,              FALSE,   FLOAT),

   VA_FMT(R16G16_UNORM,                R16_G16,          FALSE,   UNORM),
   VA_FMT(R16G16_SNORM,                R16_G16,          FALSE,   SNORM),
   VA_FMT(R16G16_USCALED,              R16_G16,          FALSE,   USCALED),
   VA_FMT(R16G16_SSCALED,              R16_G16,          FALSE,   SSCALED),
   VA_FMT(R16G16_UINT,                 R16_G16,          FALSE,   UINT),
   VA_FMT(R16G16_SINT,                 R16_G16,          FALSE,   SINT),
   VA_FMT(R16G16_SFLOAT,               R16_G16,          FALSE,   FLOAT),

   VA_FMT(R16G16B16_UNORM,             R16_G16_B16,      FALSE,   UNORM),
   VA_FMT(R16G16B16_SNORM,             R16_G16_B16,      FALSE,   SNORM),
   VA_FMT(R16G16B16_USCALED,           R16_G16_B16,      FALSE,   USCALED),
   VA_FMT(R16G16B16_SSCALED,           R16_G16_B16,      FALSE,   SSCALED),
   VA_FMT(R16G16B16_UINT,              R16_G16_B16,      FALSE,   UINT),
   VA_FMT(R16G16B16_SINT,              R16_G16_B16,      FALSE,   SINT),
   VA_FMT(R16G16B16_SFLOAT,            R16_G16_B16,      FALSE,   FLOAT),

   VA_FMT(R16G16B16A16_UNORM,          R16_G16_B16_A16,  FALSE,   UNORM),
   VA_FMT(R16G16B16A16_SNORM,          R16_G16_B16_A16,  FALSE,   SNORM),
   VA_FMT(R16G16B16A16_USCALED,        R16_G16_B16_A16,  FALSE,   USCALED),
   VA_FMT(R16G16B16A16_SSCALED,        R16_G16_B16_A16,  FALSE,   SSCALED),
   VA_FMT(R16G16B16A16_UINT,           R16_G16_B16_A16,  FALSE,   UINT),
   VA_FMT(R16G16B16A16_SINT,           R16_G16_B16_A16,  FALSE,   SINT),
   VA_FMT(R16G16B16A16_SFLOAT,         R16_G16_B16_A16,  FALSE,   FLOAT),

   VA_FMT(R32_UINT,                    R32,              FALSE,   UINT),
   VA_FMT(R32_SINT,                    R32,              FALSE,   SINT),
   VA_FMT(R32_SFLOAT,                  R32,              FALSE,   FLOAT),

   VA_FMT(R32G32_UINT,                 R32_G32,          FALSE,   UINT),
   VA_FMT(R32G32_SINT,                 R32_G32,          FALSE,   SINT),
   VA_FMT(R32G32_SFLOAT,               R32_G32,          FALSE,   FLOAT),

   VA_FMT(R32G32B32_UINT,              R32_G32_B32,      FALSE,   UINT),
   VA_FMT(R32G32B32_SINT,              R32_G32_B32,      FALSE,   SINT),
   VA_FMT(R32G32B32_SFLOAT,            R32_G32_B32,      FALSE,   FLOAT),

   VA_FMT(R32G32B32A32_UINT,           R32_G32_B32_A32,  FALSE,   UINT),
   VA_FMT(R32G32B32A32_SINT,           R32_G32_B32_A32,  FALSE,   SINT),
   VA_FMT(R32G32B32A32_SFLOAT,         R32_G32_B32_A32,  FALSE,   FLOAT),
};

#undef VA_FMT

const struct nvk_va_format *
nvk_get_va_format(const struct nvk_physical_device *pdev, VkFormat format)
{
   if (format >= ARRAY_SIZE(nvk_vf_formats))
      return NULL;

   if (nvk_vf_formats[format].bit_widths == 0)
      return NULL;

   return &nvk_vf_formats[format];
}

VKAPI_ATTR void VKAPI_CALL
nvk_GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice,
                                       VkFormat format,
                                       VkFormatProperties2 *pFormatProperties)
{
   VK_FROM_HANDLE(nvk_physical_device, pdevice, physicalDevice);

   VkFormatFeatureFlags2 linear2, optimal2, buffer2;
   linear2 = nvk_get_image_format_features(pdevice, format,
                                           VK_IMAGE_TILING_LINEAR);
   optimal2 = nvk_get_image_format_features(pdevice, format,
                                            VK_IMAGE_TILING_OPTIMAL);
   buffer2 = nvk_get_buffer_format_features(pdevice, format);

   pFormatProperties->formatProperties = (VkFormatProperties) {
      .linearTilingFeatures = vk_format_features2_to_features(linear2),
      .optimalTilingFeatures = vk_format_features2_to_features(optimal2),
      .bufferFeatures = vk_format_features2_to_features(buffer2),
   };

   vk_foreach_struct(ext, pFormatProperties->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3: {
         VkFormatProperties3 *p = (void *)ext;
         p->linearTilingFeatures = linear2;
         p->optimalTilingFeatures = optimal2;
         p->bufferFeatures = buffer2;
         break;
      }

      default:
         nvk_debug_ignored_stype(ext->sType);
         break;
      }
   }
}
