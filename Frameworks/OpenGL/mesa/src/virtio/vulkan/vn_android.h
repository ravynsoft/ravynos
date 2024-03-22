/*
 * Copyright 2021 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_ANDROID_H
#define VN_ANDROID_H

#include "vn_common.h"

#include <vulkan/vk_android_native_buffer.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

/* venus implements VK_ANDROID_native_buffer up to spec version 7 */
#define VN_ANDROID_NATIVE_BUFFER_SPEC_VERSION 7

#ifdef ANDROID

VkResult
vn_android_image_from_anb(struct vn_device *dev,
                          const VkImageCreateInfo *image_info,
                          const VkNativeBufferANDROID *anb_info,
                          const VkAllocationCallbacks *alloc,
                          struct vn_image **out_img);

bool
vn_android_get_drm_format_modifier_info(
   const VkPhysicalDeviceImageFormatInfo2 *format_info,
   VkPhysicalDeviceImageDrmFormatModifierInfoEXT *out_info);

const VkFormat *
vn_android_format_to_view_formats(VkFormat format, uint32_t *out_count);

uint64_t
vn_android_get_ahb_usage(const VkImageUsageFlags usage,
                         const VkImageCreateFlags flags);

VkResult
vn_android_device_import_ahb(
   struct vn_device *dev,
   struct vn_device_memory *mem,
   const struct VkMemoryDedicatedAllocateInfo *dedicated_info);

VkFormat
vn_android_drm_format_to_vk_format(uint32_t format);

uint32_t
vn_android_get_ahb_buffer_memory_type_bits(struct vn_device *dev);

uint64_t
vn_android_gralloc_get_shared_present_usage(void);

#else

static inline VkResult
vn_android_image_from_anb(UNUSED struct vn_device *dev,
                          UNUSED const VkImageCreateInfo *image_info,
                          UNUSED const VkNativeBufferANDROID *anb_info,
                          UNUSED const VkAllocationCallbacks *alloc,
                          UNUSED struct vn_image **out_img)
{
   return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static inline bool
vn_android_get_drm_format_modifier_info(
   UNUSED const VkPhysicalDeviceImageFormatInfo2 *format_info,
   UNUSED VkPhysicalDeviceImageDrmFormatModifierInfoEXT *out_info)
{
   return false;
}

static inline const VkFormat *
vn_android_format_to_view_formats(UNUSED VkFormat format,
                                  UNUSED uint32_t *out_count)
{
   return NULL;
}

static inline uint64_t
vn_android_get_ahb_usage(UNUSED const VkImageUsageFlags usage,
                         UNUSED const VkImageCreateFlags flags)
{
   return 0;
}

static inline VkResult
vn_android_device_import_ahb(
   UNUSED struct vn_device *dev,
   UNUSED struct vn_device_memory *mem,
   UNUSED const struct VkMemoryDedicatedAllocateInfo *dedicated_info)
{
   return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static inline VkFormat
vn_android_drm_format_to_vk_format(UNUSED uint32_t format)
{
   return VK_FORMAT_UNDEFINED;
}

static inline uint32_t
vn_android_get_ahb_buffer_memory_type_bits(UNUSED struct vn_device *dev)
{
   return 0;
}

static inline uint64_t
vn_android_gralloc_get_shared_present_usage(void)
{
   return 0;
}

#endif /* ANDROID */

#endif /* VN_ANDROID_H */
