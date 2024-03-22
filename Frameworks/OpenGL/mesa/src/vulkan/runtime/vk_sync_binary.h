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
#ifndef VK_SYNC_BINARY_H
#define VK_SYNC_BINARY_H

#include "vk_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_sync_binary_type {
   struct vk_sync_type sync;
   const struct vk_sync_type *timeline_type;
};

struct vk_sync_binary_type
vk_sync_binary_get_type(const struct vk_sync_type *timeline_type);

/** Implements a binary vk_sync type on top of a timeline vk_sync
 *
 * This is useful when targeting Windows APIs such as D3D12 which only have
 * timelines and have no concept of a binary synchronization object.  Because
 * binary vk_sync emulation requires tracking additional state (the next time
 * point), fences and semaphores created from this type cannot support any of
 * the sharing APIs.
 */
struct vk_sync_binary {
   struct vk_sync sync;

   uint64_t next_point;

   struct vk_sync timeline;
};

VkResult vk_sync_binary_init(struct vk_device *device,
                             struct vk_sync *sync,
                             uint64_t initial_value);

static inline bool
vk_sync_type_is_vk_sync_binary(const struct vk_sync_type *type)
{
   return type->init == vk_sync_binary_init;
}

static inline struct vk_sync_binary *
vk_sync_as_binary(struct vk_sync *sync)
{
   if (!vk_sync_type_is_vk_sync_binary(sync->type))
      return NULL;

   return container_of(sync, struct vk_sync_binary, sync);
}

#ifdef __cplusplus
}
#endif

#endif /* VK_TIMELINE_H */
