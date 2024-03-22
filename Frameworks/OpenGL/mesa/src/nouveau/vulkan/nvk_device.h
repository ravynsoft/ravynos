/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_DEVICE_H
#define NVK_DEVICE_H 1

#include "nvk_private.h"

#include "nvk_descriptor_table.h"
#include "nvk_heap.h"
#include "nvk_queue.h"
#include "vk_device.h"
#include "vk_meta.h"
#include "vk_queue.h"

struct novueau_ws_context;
struct nvk_physical_device;
struct vk_pipeline_cache;

struct nvk_slm_area {
   simple_mtx_t mutex;
   struct nouveau_ws_bo *bo;
   uint32_t bytes_per_warp;
   uint32_t bytes_per_tpc;
};

struct nouveau_ws_bo *
nvk_slm_area_get_bo_ref(struct nvk_slm_area *area,
                        uint32_t *bytes_per_warp_out,
                        uint32_t *bytes_per_mp_out);

struct nvk_device {
   struct vk_device vk;
   struct nvk_physical_device *pdev;

   struct nouveau_ws_device *ws_dev;
   struct nouveau_ws_context *ws_ctx;

   struct nvk_descriptor_table images;
   struct nvk_descriptor_table samplers;
   struct nvk_heap shader_heap;
   struct nvk_heap event_heap;
   struct nvk_slm_area slm;
   struct nouveau_ws_bo *zero_page;
   struct nouveau_ws_bo *vab_memory;

   struct nvk_queue queue;

   struct vk_pipeline_cache *mem_cache;

   struct vk_meta_device meta;
};

VK_DEFINE_HANDLE_CASTS(nvk_device, vk.base, VkDevice, VK_OBJECT_TYPE_DEVICE)

VkResult nvk_device_ensure_slm(struct nvk_device *dev,
                               uint32_t bytes_per_thread);

static inline struct nvk_physical_device *
nvk_device_physical(struct nvk_device *dev)
{
   return (struct nvk_physical_device *)dev->vk.physical;
}

VkResult nvk_device_init_meta(struct nvk_device *dev);
void nvk_device_finish_meta(struct nvk_device *dev);

#endif
