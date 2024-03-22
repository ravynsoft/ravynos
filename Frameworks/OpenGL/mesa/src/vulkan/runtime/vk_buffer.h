/*
 * Copyright © 2022 Collabora, Ltd.
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
#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include "vk_object.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_buffer {
   struct vk_object_base base;

   /** VkBufferCreateInfo::flags */
   VkBufferCreateFlags create_flags;

   /** VkBufferCreateInfo::size */
   VkDeviceSize size;

   /** VkBufferCreateInfo::usage or VkBufferUsageFlags2CreateInfoKHR::usage */
   VkBufferUsageFlags2KHR usage;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vk_buffer, base, VkBuffer,
                               VK_OBJECT_TYPE_BUFFER);

void vk_buffer_init(struct vk_device *device,
                    struct vk_buffer *buffer,
                    const VkBufferCreateInfo *pCreateInfo);
void vk_buffer_finish(struct vk_buffer *buffer);

void *vk_buffer_create(struct vk_device *device,
                       const VkBufferCreateInfo *pCreateInfo,
                       const VkAllocationCallbacks *alloc,
                       size_t size);
void vk_buffer_destroy(struct vk_device *device,
                       const VkAllocationCallbacks *alloc,
                       struct vk_buffer *buffer);

static inline uint64_t
vk_buffer_range(const struct vk_buffer *buffer,
                uint64_t offset, uint64_t range)
{
   assert(offset <= buffer->size);
   if (range == VK_WHOLE_SIZE) {
      return buffer->size - offset;
   } else {
      assert(range + offset >= range);
      assert(range + offset <= buffer->size);
      return range;
   }
}

#ifdef __cplusplus
}
#endif

#endif /* VK_BUFFER_H */
