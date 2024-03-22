/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_PHYSICAL_DEVICE_H
#define NVK_PHYSICAL_DEVICE_H 1

#include "nvk_private.h"

#include "nouveau_device.h"
#include "nv_device_info.h"

#include "vulkan/runtime/vk_physical_device.h"
#include "vulkan/runtime/vk_sync.h"

#include "wsi_common.h"

#include <sys/types.h>

struct nak_compiler;
struct nvk_instance;

struct nvk_physical_device {
   struct vk_physical_device vk;
   struct nv_device_info info;
   enum nvk_debug debug_flags;
   dev_t render_dev;
   dev_t primary_dev;
   struct nak_compiler *nak;
   struct wsi_device wsi_device;

   uint8_t device_uuid[VK_UUID_SIZE];

   // TODO: add mapable VRAM heap if possible
   VkMemoryHeap mem_heaps[2];
   VkMemoryType mem_types[2];
   uint8_t mem_heap_count;
   uint8_t mem_type_count;

   struct vk_sync_type syncobj_sync_type;
   const struct vk_sync_type *sync_types[2];
};

uint32_t nvk_min_cbuf_alignment(const struct nv_device_info *info);

VK_DEFINE_HANDLE_CASTS(nvk_physical_device,
   vk.base,
   VkPhysicalDevice,
   VK_OBJECT_TYPE_PHYSICAL_DEVICE)

static inline struct nvk_instance *
nvk_physical_device_instance(struct nvk_physical_device *pdev)
{
   return (struct nvk_instance *)pdev->vk.instance;
}

VkResult nvk_create_drm_physical_device(struct vk_instance *vk_instance,
                                        struct _drmDevice *drm_device,
                                        struct vk_physical_device **pdev_out);

void nvk_physical_device_destroy(struct vk_physical_device *vk_device);

#if defined(VK_USE_PLATFORM_WAYLAND_KHR) || \
    defined(VK_USE_PLATFORM_XCB_KHR) || \
    defined(VK_USE_PLATFORM_XLIB_KHR) || \
    defined(VK_USE_PLATFORM_DISPLAY_KHR)
#define NVK_USE_WSI_PLATFORM
#endif

#endif
