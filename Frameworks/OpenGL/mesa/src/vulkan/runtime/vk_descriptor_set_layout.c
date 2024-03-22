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

#include "vk_descriptor_set_layout.h"

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"

static void
vk_descriptor_set_layout_init(struct vk_device *device,
                              struct vk_descriptor_set_layout *layout)
{
   vk_object_base_init(device, &layout->base,
                       VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);

   layout->ref_cnt = 1;
   layout->destroy = vk_descriptor_set_layout_destroy;
}

void *
vk_descriptor_set_layout_zalloc(struct vk_device *device, size_t size)
{
   /* Because we're reference counting and lifetimes may not be what the
    * client expects, these have to be allocated off the device and not as
    * their own object.
    */
   struct vk_descriptor_set_layout *layout =
      vk_zalloc(&device->alloc, size, 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!layout)
      return NULL;

   vk_descriptor_set_layout_init(device, layout);

   return layout;
}

void *
vk_descriptor_set_layout_multizalloc(struct vk_device *device,
                                     struct vk_multialloc *ma)
{
   /* Because we're reference counting and lifetimes may not be what the
    * client expects, these have to be allocated off the device and not as
    * their own object.
    */
   struct vk_descriptor_set_layout *layout =
      vk_multialloc_zalloc(ma, &device->alloc,
                           VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!layout)
      return NULL;

   vk_descriptor_set_layout_init(device, layout);

   return layout;
}

void
vk_descriptor_set_layout_destroy(struct vk_device *device,
                                 struct vk_descriptor_set_layout *layout)
{
   vk_object_free(device, NULL, layout);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_DestroyDescriptorSetLayout(VkDevice _device,
                                     VkDescriptorSetLayout descriptorSetLayout,
                                     UNUSED const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_descriptor_set_layout, layout, descriptorSetLayout);

   if (layout == NULL)
      return;

   vk_descriptor_set_layout_unref(device, layout);
}
