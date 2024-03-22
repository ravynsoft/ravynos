/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_IMAGE_H
#define VN_IMAGE_H

#include "vn_common.h"

/* changing this to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR disables ownership
 * transfers and can be useful for debugging
 */
#define VN_PRESENT_SRC_INTERNAL_LAYOUT VK_IMAGE_LAYOUT_GENERAL

struct vn_image_memory_requirements {
   VkMemoryRequirements2 memory;
   VkMemoryDedicatedRequirements dedicated;
};

struct vn_image_reqs_cache_entry {
   struct vn_image_memory_requirements requirements[4];
   uint8_t plane_count;
   uint8_t key[SHA1_DIGEST_LENGTH];
   struct list_head head;
};

struct vn_image_reqs_cache {
   struct hash_table *ht;
   struct list_head lru;
   simple_mtx_t mutex;

   struct {
      uint32_t cache_hit_count;
      uint32_t cache_miss_count;
      uint32_t cache_skip_count;
   } debug;
};

struct vn_image_create_deferred_info {
   VkImageCreateInfo create;
   VkImageFormatListCreateInfo list;
   VkImageStencilUsageCreateInfo stencil;

   /* True if VkImageCreateInfo::format is translated from a non-zero
    * VkExternalFormatANDROID::externalFormat for the AHB image.
    */
   bool from_external_format;
   /* track whether vn_image_init_deferred succeeds */
   bool initialized;
};

struct vn_image {
   struct vn_image_base base;

   VkSharingMode sharing_mode;

   struct vn_image_memory_requirements requirements[4];

   /* For VK_ANDROID_external_memory_android_hardware_buffer, real image
    * creation is deferred until bind image memory.
    */
   struct vn_image_create_deferred_info *deferred_info;

   struct {
      /* True if this is a swapchain image and VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
       * is a valid layout.  A swapchain image can be created internally
       * (wsi_image_create_info) or externally (VkNativeBufferANDROID and
       * VkImageSwapchainCreateInfoKHR).
       */
      bool is_wsi;
      bool is_prime_blit_src;
      VkImageTiling tiling_override;
      /* valid when tiling is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT */
      uint64_t drm_format_modifier;

      struct vn_device_memory *memory;

      /* For VK_ANDROID_native_buffer, the WSI image owns the memory. */
      bool memory_owned;
   } wsi;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_image,
                               base.base.base,
                               VkImage,
                               VK_OBJECT_TYPE_IMAGE)

struct vn_image_view {
   struct vn_object_base base;

   const struct vn_image *image;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_image_view,
                               base.base,
                               VkImageView,
                               VK_OBJECT_TYPE_IMAGE_VIEW)

struct vn_sampler {
   struct vn_object_base base;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_sampler,
                               base.base,
                               VkSampler,
                               VK_OBJECT_TYPE_SAMPLER)

struct vn_sampler_ycbcr_conversion {
   struct vn_object_base base;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_sampler_ycbcr_conversion,
                               base.base,
                               VkSamplerYcbcrConversion,
                               VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION)

VkResult
vn_image_create(struct vn_device *dev,
                const VkImageCreateInfo *create_info,
                const VkAllocationCallbacks *alloc,
                struct vn_image **out_img);

VkResult
vn_image_init_deferred(struct vn_device *dev,
                       const VkImageCreateInfo *create_info,
                       struct vn_image *img);

void
vn_image_reqs_cache_init(struct vn_device *dev);

void
vn_image_reqs_cache_fini(struct vn_device *dev);

#endif /* VN_IMAGE_H */
