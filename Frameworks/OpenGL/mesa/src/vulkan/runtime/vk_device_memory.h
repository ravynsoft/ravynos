/*
 * Copyright © 2023 Collabora, Ltd.
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
#ifndef VK_DEVICE_MEMORY_H
#define VK_DEVICE_MEMORY_H

#include "vk_object.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AHardwareBuffer;

struct vk_device_memory {
   struct vk_object_base base;

   /* VkMemoryAllocateFlagsInfo::flags */
   VkMemoryAllocateFlags alloc_flags;

   /* VkMemoryAllocateInfo::allocationSize */
   VkDeviceSize size;

   /* VkMemoryAllocateInfo::memoryTypeIndex */
   uint32_t memory_type_index;

   /* Import handle type (if any) */
   VkExternalMemoryHandleTypeFlags import_handle_type;

   /* VkExportMemoryAllocateInfo::handleTypes */
   VkExternalMemoryHandleTypeFlags export_handle_types;

   /* VkImportMemoryHostPointerInfoEXT::pHostPointer */
   void *host_ptr;

   /* VkImportAndroidHardwareBufferInfoANDROID::buffer */
   struct AHardwareBuffer *ahardware_buffer;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vk_device_memory, base, VkDeviceMemory,
                               VK_OBJECT_TYPE_DEVICE_MEMORY);

void *vk_device_memory_create(struct vk_device *device,
                              const VkMemoryAllocateInfo *pAllocateInfo,
                              const VkAllocationCallbacks *alloc,
                              size_t size);
void vk_device_memory_destroy(struct vk_device *device,
                              const VkAllocationCallbacks *alloc,
                              struct vk_device_memory *mem);

static inline uint64_t
vk_device_memory_range(const struct vk_device_memory *mem,
                       uint64_t offset, uint64_t range)
{
   assert(offset <= mem->size);
   if (range == VK_WHOLE_SIZE) {
      return mem->size - offset;
   } else {
      assert(range + offset >= range);
      assert(range + offset <= mem->size);
      return range;
   }
}

#ifdef __cplusplus
}
#endif

#endif /* VK_DEVICE_MEMORY_H */
