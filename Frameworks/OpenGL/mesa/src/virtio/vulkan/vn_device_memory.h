/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_DEVICE_MEMORY_H
#define VN_DEVICE_MEMORY_H

#include "vn_common.h"

struct vn_device_memory_pool {
   mtx_t mutex;
   struct vn_device_memory *memory;
   VkDeviceSize used;
};

struct vn_device_memory {
   struct vn_device_memory_base base;

   /* non-NULL when suballocated */
   struct vn_device_memory *base_memory;
   /* non-NULL when mappable or external */
   struct vn_renderer_bo *base_bo;

   /* ensure renderer side resource create is called after vkAllocateMemory
    *
    * 1. driver submits vkAllocateMemory (alloc) via ring for a ring seqno
    * 2. driver submits via vq to wait for above ring to reach the seqno
    * 3. driver creates virtgpu bo from renderer VkDeviceMemory
    *
    * ensure renderer side resource destroy is called after vkAllocateMemory
    *
    * 1. driver submits vkAllocateMemory (import) via ring for a ring seqno
    * 2. driver submits via vq to wait for above ring to reach the seqno
    * 3. driver destroys virtgpu bo
    */
   bool bo_ring_seqno_valid;
   uint32_t bo_ring_seqno;

   /* ensure renderer side vkFreeMemory is called after vkGetMemoryFdKHR
    *
    * 1. driver creates virtgpu bo from renderer VkDeviceMemory
    * 2. driver submits via vq to update the vq seqno
    * 3, driver submits via ring to wait for vq reaching above seqno
    * 4. driver submits vkFreeMemory via ring
    *
    * To be noted: a successful virtgpu mmap implies a roundtrip, so
    * vn_FreeMemory after that no longer has to wait.
    */
   bool bo_roundtrip_seqno_valid;
   uint64_t bo_roundtrip_seqno;

   VkDeviceSize base_offset;

   VkDeviceSize map_end;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_device_memory,
                               base.base.base,
                               VkDeviceMemory,
                               VK_OBJECT_TYPE_DEVICE_MEMORY)

void
vn_device_memory_pool_fini(struct vn_device *dev, uint32_t mem_type_index);

VkResult
vn_device_memory_import_dma_buf(struct vn_device *dev,
                                struct vn_device_memory *mem,
                                const VkMemoryAllocateInfo *alloc_info,
                                bool force_unmappable,
                                int fd);

VkResult
vn_get_memory_dma_buf_properties(struct vn_device *dev,
                                 int fd,
                                 uint64_t *out_alloc_size,
                                 uint32_t *out_mem_type_bits);

#endif /* VN_DEVICE_MEMORY_H */
