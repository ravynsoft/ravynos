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

#include "vk_sync_dummy.h"

static VkResult
vk_sync_dummy_init(struct vk_device *device,
                   struct vk_sync *sync,
                   uint64_t initial_value)
{
   return VK_SUCCESS;
}

static void
vk_sync_dummy_finish(struct vk_device *device,
                     struct vk_sync *sync)
{ }

static VkResult
vk_sync_dummy_wait_many(struct vk_device *device,
                        uint32_t wait_count,
                        const struct vk_sync_wait *waits,
                        enum vk_sync_wait_flags wait_flags,
                        uint64_t abs_timeout_ns)
{
   return VK_SUCCESS;
}

const struct vk_sync_type vk_sync_dummy_type = {
   .size = sizeof(struct vk_sync),
   .features = VK_SYNC_FEATURE_BINARY |
               VK_SYNC_FEATURE_GPU_WAIT |
               VK_SYNC_FEATURE_CPU_WAIT |
               VK_SYNC_FEATURE_WAIT_ANY |
               VK_SYNC_FEATURE_WAIT_PENDING,
   .init = vk_sync_dummy_init,
   .finish = vk_sync_dummy_finish,
   .wait_many = vk_sync_dummy_wait_many,
};
