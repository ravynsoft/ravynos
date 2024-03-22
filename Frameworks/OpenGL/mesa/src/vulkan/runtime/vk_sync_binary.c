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

#include "vk_sync_binary.h"

#include "vk_util.h"

static struct vk_sync_binary *
to_vk_sync_binary(struct vk_sync *sync)
{
   assert(sync->type->init == vk_sync_binary_init);

   return container_of(sync, struct vk_sync_binary, sync);
}

VkResult
vk_sync_binary_init(struct vk_device *device,
                    struct vk_sync *sync,
                    uint64_t initial_value)
{
   struct vk_sync_binary *binary = to_vk_sync_binary(sync);

   const struct vk_sync_binary_type *btype =
      container_of(binary->sync.type, struct vk_sync_binary_type, sync);

   assert(!(sync->flags & VK_SYNC_IS_TIMELINE));
   assert(!(sync->flags & VK_SYNC_IS_SHAREABLE));

   binary->next_point = (initial_value == 0);

   return vk_sync_init(device, &binary->timeline, btype->timeline_type,
                       VK_SYNC_IS_TIMELINE, 0 /* initial_value */);
}

static void
vk_sync_binary_finish(struct vk_device *device,
                      struct vk_sync *sync)
{
   struct vk_sync_binary *binary = to_vk_sync_binary(sync);

   vk_sync_finish(device, &binary->timeline);
}

static VkResult
vk_sync_binary_reset(struct vk_device *device,
                     struct vk_sync *sync)
{
   struct vk_sync_binary *binary = to_vk_sync_binary(sync);

   binary->next_point++;

   return VK_SUCCESS;
}

static VkResult
vk_sync_binary_signal(struct vk_device *device,
                      struct vk_sync *sync,
                      uint64_t value)
{
   struct vk_sync_binary *binary = to_vk_sync_binary(sync);

   assert(value == 0);

   return vk_sync_signal(device, &binary->timeline, binary->next_point);
}

static VkResult
vk_sync_binary_wait_many(struct vk_device *device,
                         uint32_t wait_count,
                         const struct vk_sync_wait *waits,
                         enum vk_sync_wait_flags wait_flags,
                         uint64_t abs_timeout_ns)
{
   STACK_ARRAY(struct vk_sync_wait, timeline_waits, wait_count);

   for (uint32_t i = 0; i < wait_count; i++) {
      struct vk_sync_binary *binary = to_vk_sync_binary(waits[i].sync);

      timeline_waits[i] = (struct vk_sync_wait) {
         .sync = &binary->timeline,
         .stage_mask = waits[i].stage_mask,
         .wait_value = binary->next_point,
      };
   }

   VkResult result = vk_sync_wait_many(device, wait_count, timeline_waits, 
                                       wait_flags, abs_timeout_ns);

   STACK_ARRAY_FINISH(timeline_waits);

   return result;
}

struct vk_sync_binary_type
vk_sync_binary_get_type(const struct vk_sync_type *timeline_type)
{
   assert(timeline_type->features & VK_SYNC_FEATURE_TIMELINE);

   return (struct vk_sync_binary_type) {
      .sync = {
         .size = offsetof(struct vk_sync_binary, timeline) +
                 timeline_type->size,
         .features = VK_SYNC_FEATURE_BINARY |
                     VK_SYNC_FEATURE_GPU_WAIT |
                     VK_SYNC_FEATURE_CPU_WAIT |
                     VK_SYNC_FEATURE_CPU_RESET |
                     VK_SYNC_FEATURE_CPU_SIGNAL |
                     VK_SYNC_FEATURE_WAIT_ANY |
                     VK_SYNC_FEATURE_WAIT_PENDING,
         .init = vk_sync_binary_init,
         .finish = vk_sync_binary_finish,
         .reset = vk_sync_binary_reset,
         .signal = vk_sync_binary_signal,
         .wait_many = vk_sync_binary_wait_many,
      },
      .timeline_type = timeline_type,
   };
}
