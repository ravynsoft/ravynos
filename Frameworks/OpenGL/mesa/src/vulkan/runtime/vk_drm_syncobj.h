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
#ifndef VK_DRM_SYNCOBJ_H
#define VK_DRM_SYNCOBJ_H

#include "vk_sync.h"

#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_drm_syncobj {
   struct vk_sync base;
   uint32_t syncobj;
};

void vk_drm_syncobj_finish(struct vk_device *device,
                           struct vk_sync *sync);

static inline bool
vk_sync_type_is_drm_syncobj(const struct vk_sync_type *type)
{
   return type->finish == vk_drm_syncobj_finish;
}

static inline struct vk_drm_syncobj *
vk_sync_as_drm_syncobj(struct vk_sync *sync)
{
   if (!vk_sync_type_is_drm_syncobj(sync->type))
      return NULL;

   return container_of(sync, struct vk_drm_syncobj, base);
}

struct vk_sync_type vk_drm_syncobj_get_type(int drm_fd);

#ifdef __cplusplus
}
#endif

#endif /* VK_DRM_SYNCOBJ_H */
