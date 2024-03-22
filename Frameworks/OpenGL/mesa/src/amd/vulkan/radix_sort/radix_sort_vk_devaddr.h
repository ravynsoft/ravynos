// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_INCLUDE_RADIX_SORT_PLATFORMS_VK_RADIX_SORT_VK_DEVADDR_H_
#define SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_INCLUDE_RADIX_SORT_PLATFORMS_VK_RADIX_SORT_VK_DEVADDR_H_

//
//
//

#include "radix_sort_vk.h"

//
//
//

#ifdef __cplusplus
extern "C" {
#endif

//
// Structure that enables integration with Vulkan drivers.
//
typedef struct radix_sort_vk_buffer_info
{
  VkBuffer        buffer;
  VkDeviceSize    offset;
  VkDeviceAddress devaddr;
} radix_sort_vk_buffer_info_t;

//
// Function prototypes
//
typedef void (*radix_sort_vk_fill_buffer_pfn)(VkCommandBuffer                     cb,
                                              radix_sort_vk_buffer_info_t const * buffer_info,
                                              VkDeviceSize                        offset,
                                              VkDeviceSize                        size,
                                              uint32_t                            data);

typedef void (*radix_sort_vk_dispatch_indirect_pfn)(VkCommandBuffer                     cb,
                                                    radix_sort_vk_buffer_info_t const * buffer_info,
                                                    VkDeviceSize                        offset);

//
// Direct dispatch sorting using buffer device addresses
// -----------------------------------------------------
//
typedef struct radix_sort_vk_sort_devaddr_info
{
  void *                        ext;
  uint32_t                      key_bits;
  uint32_t                      count;
  radix_sort_vk_buffer_info_t   keyvals_even;
  VkDeviceAddress               keyvals_odd;
  radix_sort_vk_buffer_info_t   internal;
  radix_sort_vk_fill_buffer_pfn fill_buffer;
} radix_sort_vk_sort_devaddr_info_t;

void
radix_sort_vk_sort_devaddr(radix_sort_vk_t const *                   rs,
                           radix_sort_vk_sort_devaddr_info_t const * info,
                           VkDevice                                  device,
                           VkCommandBuffer                           cb,
                           VkDeviceAddress *                         keyvals_sorted);

//
// Indirect dispatch sorting using buffer device addresses
// -------------------------------------------------------
//
// clang-format off
//
typedef struct radix_sort_vk_sort_indirect_devaddr_info
{
  void *                              ext;
  uint32_t                            key_bits;
  VkDeviceAddress                     count;
  VkDeviceAddress                     keyvals_even;
  VkDeviceAddress                     keyvals_odd;
  VkDeviceAddress                     internal;
  radix_sort_vk_buffer_info_t         indirect;
  radix_sort_vk_dispatch_indirect_pfn dispatch_indirect;
} radix_sort_vk_sort_indirect_devaddr_info_t;

void
radix_sort_vk_sort_indirect_devaddr(radix_sort_vk_t const *                            rs,
                                    radix_sort_vk_sort_indirect_devaddr_info_t const * info,
                                    VkDevice                                           device,
                                    VkCommandBuffer                                    cb,
                                    VkDeviceAddress *                                  keyvals_sorted);

//
// clang-format on
//

#ifdef __cplusplus
}
#endif

//
//
//

#endif  // SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_INCLUDE_RADIX_SORT_PLATFORMS_VK_RADIX_SORT_VK_DEVADDR_H_
