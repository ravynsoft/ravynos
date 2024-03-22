/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_MEMORY_H
#define NVK_MEMORY_H 1

#include "nvk_private.h"

#include "vk_device_memory.h"

#include "util/list.h"

struct nvk_device;
struct nvk_image_plane;

struct nvk_device_memory {
   struct vk_device_memory vk;

   struct nouveau_ws_bo *bo;

   void *map;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_device_memory, vk.base, VkDeviceMemory, VK_OBJECT_TYPE_DEVICE_MEMORY)

extern const VkExternalMemoryProperties nvk_opaque_fd_mem_props;
extern const VkExternalMemoryProperties nvk_dma_buf_mem_props;

#endif
