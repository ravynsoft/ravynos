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

#include "vk_buffer_view.h"

#include "vk_alloc.h"
#include "vk_buffer.h"
#include "vk_device.h"
#include "vk_format.h"

void
vk_buffer_view_init(struct vk_device *device,
                    struct vk_buffer_view *buffer_view,
                    const VkBufferViewCreateInfo *pCreateInfo)
{
   VK_FROM_HANDLE(vk_buffer, buffer, pCreateInfo->buffer);

   vk_object_base_init(device, &buffer_view->base, VK_OBJECT_TYPE_BUFFER_VIEW);

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO);
   assert(pCreateInfo->flags == 0);
   assert(pCreateInfo->range > 0);

   buffer_view->buffer = buffer;
   buffer_view->format = pCreateInfo->format;
   buffer_view->offset = pCreateInfo->offset;
   buffer_view->range = vk_buffer_range(buffer, pCreateInfo->offset,
                                        pCreateInfo->range);
   buffer_view->elements = buffer_view->range /
                           vk_format_get_blocksize(buffer_view->format);
}

void *
vk_buffer_view_create(struct vk_device *device,
                      const VkBufferViewCreateInfo *pCreateInfo,
                      const VkAllocationCallbacks *alloc,
                      size_t size)
{
   struct vk_buffer_view *buffer_view;

   buffer_view = vk_zalloc2(&device->alloc, alloc, size, 8,
                            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!buffer_view)
      return NULL;

   vk_buffer_view_init(device, buffer_view, pCreateInfo);

   return buffer_view;
}

void
vk_buffer_view_finish(struct vk_buffer_view *buffer_view)
{
   vk_object_base_finish(&buffer_view->base);
}

void
vk_buffer_view_destroy(struct vk_device *device,
                       const VkAllocationCallbacks *alloc,
                       struct vk_buffer_view *buffer_view)
{
   vk_object_free(device, alloc, buffer_view);
}
