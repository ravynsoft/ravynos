/*
 * Copyright © 2021 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef VK_SYNC_TIMELINE_H
#define VK_SYNC_TIMELINE_H

#include "c11/threads.h"
#include "util/list.h"
#include "util/macros.h"

#include "vk_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_sync_timeline_type {
   struct vk_sync_type sync;

   /* Type of each individual time point */
   const struct vk_sync_type *point_sync_type;
};

struct vk_sync_timeline_type
vk_sync_timeline_get_type(const struct vk_sync_type *point_sync_type);

struct vk_sync_timeline_point {
   struct vk_sync_timeline *timeline;

   struct list_head link;

   uint64_t value;

   int refcount;
   bool pending;

   struct vk_sync sync;
};

/** Implements a timeline vk_sync type on top of a binary vk_sync
 *
 * This is used for emulating VK_KHR_timeline_semaphores for implementations
 * whose kernel driver do not yet support timeline syncobj.  Since it's a
 * requirement for Vulkan 1.2, it's useful to have an emulation like this.
 *
 * The driver should never see a vk_sync_timeline object.  Instead, converting
 * from vk_sync_timeline to a binary vk_sync for a particular time point is
 * handled by common code.  All a driver needs to do is declare its preferred
 * binary vk_sync_type for emulation as follows:
 *
 *    const struct vk_sync_type anv_bo_sync_type = {
 *       ...
 *    };
 *    VK_DECL_TIMELINE_TYPE(anv_bo_timeline_sync_type, &anv_bo_sync_type);
 *
 * and then anv_bo_timeline_sync_type.sync can be used as a sync type to
 * provide timelines.
 */
struct vk_sync_timeline {
   struct vk_sync sync;

   mtx_t mutex;
   cnd_t cond;

   uint64_t highest_past;
   uint64_t highest_pending;

   struct list_head pending_points;
   struct list_head free_points;
};

VkResult vk_sync_timeline_init(struct vk_device *device,
                               struct vk_sync *sync,
                               uint64_t initial_value);

VkResult vk_sync_timeline_alloc_point(struct vk_device *device,
                                      struct vk_sync_timeline *timeline,
                                      uint64_t value,
                                      struct vk_sync_timeline_point **point_out);

void vk_sync_timeline_point_free(struct vk_device *device,
                                 struct vk_sync_timeline_point *point);

VkResult vk_sync_timeline_point_install(struct vk_device *device,
                                        struct vk_sync_timeline_point *point);

VkResult vk_sync_timeline_get_point(struct vk_device *device,
                                    struct vk_sync_timeline *timeline,
                                    uint64_t wait_value,
                                    struct vk_sync_timeline_point **point_out);

void vk_sync_timeline_point_release(struct vk_device *device,
                                    struct vk_sync_timeline_point *point);

static inline bool
vk_sync_type_is_vk_sync_timeline(const struct vk_sync_type *type)
{
   return type->init == vk_sync_timeline_init;
}

static inline struct vk_sync_timeline *
vk_sync_as_timeline(struct vk_sync *sync)
{
   if (!vk_sync_type_is_vk_sync_timeline(sync->type))
      return NULL;

   return container_of(sync, struct vk_sync_timeline, sync);
}

#ifdef __cplusplus
}
#endif

#endif /* VK_SYNC_TIMELINE_H */
