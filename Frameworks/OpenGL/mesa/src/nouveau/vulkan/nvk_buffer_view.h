/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_BUFFER_VIEW_H
#define NVK_BUFFER_VIEW_H 1

#include "nvk_private.h"

#include "vulkan/runtime/vk_buffer_view.h"

struct nvk_physical_device;

VkFormatFeatureFlags2
nvk_get_buffer_format_features(struct nvk_physical_device *pdevice,
                               VkFormat format);

struct nvk_buffer_view {
   struct vk_buffer_view vk;

   /** Index in the image descriptor table */
   uint32_t desc_index;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_buffer_view, vk.base, VkBufferView,
                               VK_OBJECT_TYPE_BUFFER_VIEW)

#endif
