/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * based in part on tu driver which is:
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_BO_H
#define PVR_BO_H

#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "pvr_types.h"
#include "pvr_winsys.h"
#include "util/list.h"
#include "util/macros.h"
#include "util/simple_mtx.h"

struct pvr_device;
struct pvr_dump_ctx;
struct pvr_winsys_bo;
struct pvr_winsys_vma;
struct pvr_winsys_heap;

struct pvr_bo {
   /* Since multiple components (csb, caching logic, etc.) can make use of
    * linking buffers in a list, we add 'link' in pvr_bo to avoid an extra
    * level of structure inheritance. It's the responsibility of the buffer
    * user to manage the list and remove the buffer from the list before
    * freeing it.
    */
   struct list_head link;

   struct pvr_winsys_bo *bo;
   struct pvr_winsys_vma *vma;
   uint32_t ref_count;
};

struct pvr_suballocator {
   /* Pointer to the pvr_device this allocator is associated with */
   struct pvr_device *device;
   /* Pointer to one heap type (e.g. general, pds or usc) */
   struct pvr_winsys_heap *heap;
   /* Minimum size of the pvr_bo shared across multiple sub-allocations */
   uint32_t default_size;

   /* Mutex to protect access to all of the members below this point */
   simple_mtx_t mtx;

   /* Current buffer object where sub-allocations are made from */
   struct pvr_bo *bo;
   /* Previous buffer that can be used when a new buffer object is needed */
   struct pvr_bo *bo_cached;
   /* Track from where to start the next sub-allocation */
   uint32_t next_offset;
};

struct pvr_suballoc_bo {
   /* Since multiple components (command buffer, clear, descriptor sets,
    * pipeline, SPM, etc.) can make use of linking sub-allocated bo(s), we
    * add 'link' in pvr_suballoc_bo and avoid one extra level of structure
    * inheritance. It is users' responsibility to manage the linked list,
    * to remove sub-allocations before freeing it.
    */
   struct list_head link;

   struct pvr_suballocator *allocator;
   struct pvr_bo *bo;
   pvr_dev_addr_t dev_addr;
   uint64_t offset;
   uint32_t size;
};

/**
 * \brief Flag passed to #pvr_bo_alloc() to indicate that the buffer should be
 * CPU accessible. This is required in order to map a buffer with
 * #pvr_bo_cpu_map().
 */
#define PVR_BO_ALLOC_FLAG_CPU_ACCESS BITFIELD_BIT(0U)
/**
 * \brief Flag passed to #pvr_bo_alloc() to indicate that the buffer should
 * be mapped to the CPU. Implies #PVR_BO_ALLOC_FLAG_CPU_ACCESS.
 */
#define PVR_BO_ALLOC_FLAG_CPU_MAPPED BITFIELD_BIT(1U)
/**
 * \brief Flag passed to #pvr_bo_alloc() to indicate that the buffer should be
 * mapped to the GPU as uncached.
 */
#define PVR_BO_ALLOC_FLAG_GPU_UNCACHED BITFIELD_BIT(2U)
/**
 * \brief Flag passed to #pvr_bo_alloc() to indicate that the buffer GPU mapping
 * should be restricted to only allow access to the Parameter Manager unit and
 * firmware processor.
 */
#define PVR_BO_ALLOC_FLAG_PM_FW_PROTECT BITFIELD_BIT(3U)

VkResult pvr_bo_alloc(struct pvr_device *device,
                      struct pvr_winsys_heap *heap,
                      uint64_t size,
                      uint64_t alignment,
                      uint64_t flags,
                      struct pvr_bo **const bo_out);
VkResult pvr_bo_cpu_map(struct pvr_device *device, struct pvr_bo *bo);
void pvr_bo_cpu_unmap(struct pvr_device *device, struct pvr_bo *bo);
void pvr_bo_free(struct pvr_device *device, struct pvr_bo *bo);

void pvr_bo_suballocator_init(struct pvr_suballocator *allocator,
                              struct pvr_winsys_heap *heap,
                              struct pvr_device *device,
                              uint32_t default_size);
void pvr_bo_suballocator_fini(struct pvr_suballocator *suballoc);
VkResult pvr_bo_suballoc(struct pvr_suballocator *allocator,
                         uint32_t size,
                         uint32_t alignment,
                         bool zero_on_alloc,
                         struct pvr_suballoc_bo **const suballoc_bo_out);
void pvr_bo_suballoc_free(struct pvr_suballoc_bo *suballoc_bo);
void *pvr_bo_suballoc_get_map_addr(const struct pvr_suballoc_bo *suballoc_bo);

#if defined(HAVE_VALGRIND)
VkResult pvr_bo_cpu_map_unchanged(struct pvr_device *device,
                                  struct pvr_bo *pvr_bo);
#else /* defined(HAVE_VALGRIND) */
static ALWAYS_INLINE VkResult
pvr_bo_cpu_map_unchanged(struct pvr_device *device, struct pvr_bo *pvr_bo)
{
   return pvr_bo_cpu_map(device, pvr_bo);
}
#endif /* defined(HAVE_VALGRIND) */

struct pvr_bo_store;

VkResult pvr_bo_store_create(struct pvr_device *device);
void pvr_bo_store_destroy(struct pvr_device *device);
struct pvr_bo *pvr_bo_store_lookup(struct pvr_device *device,
                                   pvr_dev_addr_t addr);
bool pvr_bo_store_dump(struct pvr_device *device);

void pvr_bo_list_dump(struct pvr_dump_ctx *ctx,
                      const struct list_head *bo_list,
                      uint32_t bo_size);

#endif /* PVR_BO_H */
