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
#ifndef VK_FENCE_H
#define VK_FENCE_H

#include "vk_object.h"
#include "vk_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_sync;

struct vk_fence {
   struct vk_object_base base;

   /* Temporary fence state.
    *
    * A fence *may* have temporary state.  That state is added to the fence by
    * an import operation and is reset back to NULL when the fence is reset.
    * A fence with temporary state cannot be signaled because the fence must
    * already be signaled before the temporary state can be exported from the
    * fence in the other process and imported here.
    */
   struct vk_sync *temporary;

   /** Permanent fence state.
    *
    * Every fence has some form of permanent state.
    *
    * This field must be last
    */
   alignas(8) struct vk_sync permanent;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(vk_fence, base, VkFence,
                               VK_OBJECT_TYPE_FENCE);

VkResult vk_fence_create(struct vk_device *device,
                         const VkFenceCreateInfo *pCreateInfo,
                         const VkAllocationCallbacks *pAllocator,
                         struct vk_fence **fence_out);

void vk_fence_destroy(struct vk_device *device,
                      struct vk_fence *fence,
                      const VkAllocationCallbacks *pAllocator);

void vk_fence_reset_temporary(struct vk_device *device,
                              struct vk_fence *fence);

static inline struct vk_sync *
vk_fence_get_active_sync(struct vk_fence *fence)
{
   return fence->temporary ? fence->temporary : &fence->permanent;
}

#ifdef __cplusplus
}
#endif

#endif /* VK_FENCE_H */
