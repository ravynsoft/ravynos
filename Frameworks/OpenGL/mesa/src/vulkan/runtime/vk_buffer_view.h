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
#ifndef VK_BUFFER_VIEW_H
#define VK_BUFFER_VIEW_H

#include "vk_object.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_buffer_view {
   struct vk_object_base base;

   /** VkBufferViewCreateInfo::buffer */
   struct vk_buffer *buffer;

   /** VkBufferViewCreateInfo::format */
   VkFormat format;

   /** VkBufferViewCreateInfo::offset */
   VkDeviceSize offset;

   /** VkBufferViewCreateInfo::range
    *
    * This is asserted to be in-range for the attached buffer and will never
    * be VK_WHOLE_SIZE.
    */
   VkDeviceSize range;

   /* Number of elements in the buffer.  This is range divided by the size of
    * format, rounded down.
    */
   VkDeviceSize elements;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vk_buffer_view, base, VkBufferView,
                               VK_OBJECT_TYPE_BUFFER_VIEW);

void vk_buffer_view_init(struct vk_device *device,
                         struct vk_buffer_view *buffer_view,
                         const VkBufferViewCreateInfo *pCreateInfo);
void vk_buffer_view_finish(struct vk_buffer_view *buffer_view);
void *vk_buffer_view_create(struct vk_device *device,
                            const VkBufferViewCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *alloc,
                            size_t size);
void vk_buffer_view_destroy(struct vk_device *device,
                            const VkAllocationCallbacks *alloc,
                            struct vk_buffer_view *buffer_view);

#ifdef __cplusplus
}
#endif

#endif /* VK_BUFFER_VIEW_H */
