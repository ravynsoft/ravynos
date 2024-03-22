/*
 * Copyright © 2018 Google, Inc.
 * Copyright © 2015 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#ifndef TU_KNL_DRM_H
#define TU_KNL_DRM_H

#include "tu_knl.h"
#include "drm-uapi/msm_drm.h"

#include "vk_util.h"

#include "util/timespec.h"

enum tu_mem_sync_op {
   TU_MEM_SYNC_CACHE_TO_GPU,
   TU_MEM_SYNC_CACHE_FROM_GPU,
};

void
tu_sync_cache_bo(struct tu_device *dev,
                 struct tu_bo *bo,
                 VkDeviceSize offset,
                 VkDeviceSize size,
                 enum tu_mem_sync_op op);

VkResult tu_allocate_userspace_iova(struct tu_device *dev,
                                    uint64_t size,
                                    uint64_t client_iova,
                                    enum tu_bo_alloc_flags flags,
                                    uint64_t *iova);
int tu_drm_export_dmabuf(struct tu_device *dev, struct tu_bo *bo);
void tu_drm_bo_finish(struct tu_device *dev, struct tu_bo *bo);

static inline void
get_abs_timeout(struct drm_msm_timespec *tv, uint64_t ns)
{
   struct timespec t;
   clock_gettime(CLOCK_MONOTONIC, &t);
   tv->tv_sec = t.tv_sec + ns / 1000000000;
   tv->tv_nsec = t.tv_nsec + ns % 1000000000;
}

static inline bool
fence_before(uint32_t a, uint32_t b)
{
   return (int32_t)(a - b) < 0;
}

extern const struct vk_sync_type tu_timeline_sync_type;

static inline bool
vk_sync_is_tu_timeline_sync(const struct vk_sync *sync)
{
   return sync->type == &tu_timeline_sync_type;
}

static inline struct tu_timeline_sync *
to_tu_timeline_sync(struct vk_sync *sync)
{
   assert(sync->type == &tu_timeline_sync_type);
   return container_of(sync, struct tu_timeline_sync, base);
}

uint32_t tu_syncobj_from_vk_sync(struct vk_sync *sync);

#endif