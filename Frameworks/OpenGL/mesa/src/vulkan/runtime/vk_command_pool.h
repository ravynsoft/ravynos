/*
 * Copyright © 2022 Collabora, Ltd
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

#ifndef VK_COMMAND_POOL_H
#define VK_COMMAND_POOL_H

#include "vk_object.h"
#include "util/list.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Base object for implementin VkCommandPool */
struct vk_command_pool {
   struct vk_object_base base;

   /** VkCommandPoolCreateInfo::flags */
   VkCommandPoolCreateFlags flags;

   /** VkCommandPoolCreateInfo::queueFamilyIndex */
   uint32_t queue_family_index;

   /** Allocator passed to vkCreateCommandPool() */
   VkAllocationCallbacks alloc;

   /** Command buffer vtable for command buffers allocated from this pool */
   const struct vk_command_buffer_ops *command_buffer_ops;

   /** True if we should recycle command buffers */
   bool recycle_command_buffers;

   /** List of all command buffers */
   struct list_head command_buffers;

   /** List of freed command buffers for trimming. */
   struct list_head free_command_buffers;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(vk_command_pool, base, VkCommandPool,
                               VK_OBJECT_TYPE_COMMAND_POOL);

/** Initialize a vk_command_pool
 *
 * :param device:       |in|  The Vulkan device
 * :param pool:         |out| The command pool to initialize
 * :param pCreateInfo:  |in|  VkCommandPoolCreateInfo pointer passed to
 *                            `vkCreateCommandPool()`
 * :param pAllocator:   |in|   Allocation callbacks passed to
 *                            `vkCreateCommandPool()`
 */
VkResult MUST_CHECK
vk_command_pool_init(struct vk_device *device,
                     struct vk_command_pool *pool,
                     const VkCommandPoolCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *pAllocator);

/** Tear down a vk_command_pool
 *
 * :param pool:         |inout| The command pool to tear down
 */
void
vk_command_pool_finish(struct vk_command_pool *pool);

/** Trim a vk_command_pool
 *
 * This discards any resources that may be cached by the common
 * vk_command_pool code.  For driver-implemented command pools, drivers should
 * call this function inside their `vkTrimCommandPool()` implementation.  This
 * should be called before doing any driver-specific trimming in case it ends
 * up returning driver-internal resources to the pool.
 *
 * :param pool:         |inout| The command pool to trim
 * :param flags:        |in|    Flags controling the trim operation
 */
void
vk_command_pool_trim(struct vk_command_pool *pool,
                     VkCommandPoolTrimFlags flags);

#ifdef __cplusplus
}
#endif

#endif  /* VK_COMMAND_POOL_H */
