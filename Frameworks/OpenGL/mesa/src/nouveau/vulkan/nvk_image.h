/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_IMAGE_H
#define NVK_IMAGE_H 1

#include "nvk_private.h"

#include "vk_image.h"

#include "nil_image.h"

struct nvk_device_memory;
struct nvk_physical_device;

static VkFormatFeatureFlags2
nvk_get_image_plane_format_features(struct nvk_physical_device *pdev,
                                    VkFormat vk_format, VkImageTiling tiling);

VkFormatFeatureFlags2
nvk_get_image_format_features(struct nvk_physical_device *pdevice,
                              VkFormat format, VkImageTiling tiling);

uint32_t
nvk_image_max_dimension(const struct nv_device_info *info,
                        VkImageType image_type);

struct nvk_image_plane {
   struct nil_image nil;
   uint64_t addr;

   /** Size of the reserved VMA range for sparse images, zero otherwise. */
   uint64_t vma_size_B;
};

struct nvk_image {
   struct vk_image vk;

   /** True if the planes are bound separately
    *
    * This is set based on VK_IMAGE_CREATE_DISJOINT_BIT
    */
   bool disjoint;

   uint8_t plane_count;
   struct nvk_image_plane planes[3];

   /* In order to support D32_SFLOAT_S8_UINT, a temp area is
    * needed. The stencil plane can't be a copied using the DMA
    * engine in a single pass since it would need 8 components support.
    * Instead we allocate a 16-bit temp, that gets copied into, then
    * copied again down to the 8-bit result.
    */
   struct nvk_image_plane stencil_copy_temp;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_image, vk.base, VkImage, VK_OBJECT_TYPE_IMAGE)

static inline uint64_t
nvk_image_plane_base_address(const struct nvk_image_plane *plane)
{
   return plane->addr;
}

static inline uint64_t
nvk_image_base_address(const struct nvk_image *image, uint8_t plane)
{
   return nvk_image_plane_base_address(&image->planes[plane]);
}

static inline uint8_t
nvk_image_aspects_to_plane(ASSERTED const struct nvk_image *image,
                           VkImageAspectFlags aspectMask)
{
   /* Verify that the aspects are actually in the image */
   assert(!(aspectMask & ~image->vk.aspects));

   /* Must only be one aspect unless it's depth/stencil */
   assert(aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT |
                         VK_IMAGE_ASPECT_STENCIL_BIT) ||
          util_bitcount(aspectMask) == 1);

   switch(aspectMask) {
   case VK_IMAGE_ASPECT_PLANE_1_BIT: return 1;
   case VK_IMAGE_ASPECT_PLANE_2_BIT: return 2;
   default: return 0;
   }
}

#endif
