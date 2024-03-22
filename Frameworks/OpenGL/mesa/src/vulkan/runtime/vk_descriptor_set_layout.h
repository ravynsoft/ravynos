/*
 * Copyright © 2022 Collabora Ltd
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
#ifndef VK_DESCRIPTOR_SET_LAYOUT_H
#define VK_DESCRIPTOR_SET_LAYOUT_H

#include "vk_object.h"

#include "util/u_atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_descriptor_set_layout {
   struct vk_object_base base;

   void (*destroy)(struct vk_device *device,
                   struct vk_descriptor_set_layout *layout);

   /** Reference count
    *
    * It's often necessary to store a pointer to the descriptor set layout in
    * the descriptor so that any entrypoint which has access to a descriptor
    * set also has the layout.  While layouts are often passed into various
    * entrypoints, they're notably missing from vkUpdateDescriptorSets().  In
    * order to implement descriptor writes, you either need to stash a pointer
    * to the descriptor set layout in the descriptor set or you need to copy
    * all of the relevant information.  Storing a pointer is a lot cheaper.
    *
    * Because descriptor set layout lifetimes and descriptor set lifetimes are
    * not guaranteed to coincide, we have to reference count if we're going to
    * do this.
    */
   uint32_t ref_cnt;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(vk_descriptor_set_layout, base,
                               VkDescriptorSetLayout,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);

void *vk_descriptor_set_layout_zalloc(struct vk_device *device, size_t size);

void *vk_descriptor_set_layout_multizalloc(struct vk_device *device,
                                           struct vk_multialloc *ma);

void vk_descriptor_set_layout_destroy(struct vk_device *device,
                                      struct vk_descriptor_set_layout *layout);

static inline struct vk_descriptor_set_layout *
vk_descriptor_set_layout_ref(struct vk_descriptor_set_layout *layout)
{
   assert(layout && layout->ref_cnt >= 1);
   p_atomic_inc(&layout->ref_cnt);
   return layout;
}

static inline void
vk_descriptor_set_layout_unref(struct vk_device *device,
                               struct vk_descriptor_set_layout *layout)
{
   assert(layout && layout->ref_cnt >= 1);
   if (p_atomic_dec_zero(&layout->ref_cnt))
      layout->destroy(device, layout);
}

#ifdef __cplusplus
}
#endif

#endif /* VK_DESCRIPTOR_SET_LAYOUT_H */

