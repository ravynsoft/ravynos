/*
 * Copyright © 2022 Imagination Technologies Ltd.
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

#ifndef PVR_SRV_BO_H
#define PVR_SRV_BO_H

#include <stdint.h>

#include "pvr_private.h"
#include "pvr_srv.h"
#include "pvr_types.h"
#include "pvr_winsys.h"
#include "util/macros.h"

/*******************************************
   MemAlloc flags
 *******************************************/

/* TODO: remove unused and redundant flags */
#define PVR_SRV_MEMALLOCFLAG_DEVICE_FLAGS_OFFSET 26U
#define PVR_SRV_MEMALLOCFLAG_DEVICE_FLAGS_MASK \
   (0x3ULL << PVR_SRV_MEMALLOCFLAG_DEVICE_FLAGS_OFFSET)
#define PVR_SRV_MEMALLOCFLAG_CPU_CACHE_CLEAN BITFIELD_BIT(19U)
#define PVR_SRV_MEMALLOCFLAG_KERNEL_CPU_MAPPABLE BITFIELD_BIT(14U)
#define PVR_SRV_MEMALLOCFLAG_ZERO_ON_ALLOC BITFIELD_BIT(31U)
#define PVR_SRV_MEMALLOCFLAG_SVM_ALLOC BITFIELD_BIT(17U)
#define PVR_SRV_MEMALLOCFLAG_POISON_ON_ALLOC BITFIELD_BIT(30U)
#define PVR_SRV_MEMALLOCFLAG_POISON_ON_FREE BITFIELD_BIT(29U)
#define PVR_SRV_MEMALLOCFLAG_GPU_READABLE BITFIELD_BIT(0U)
#define PVR_SRV_MEMALLOCFLAG_GPU_WRITEABLE BITFIELD_BIT(1U)
#define PVR_SRV_MEMALLOCFLAG_GPU_CACHE_MODE_MASK (7ULL << 8U)
#define PVR_SRV_MEMALLOCFLAGS_GPU_MMUFLAGSMASK                               \
   (PVR_SRV_MEMALLOCFLAG_GPU_READABLE | PVR_SRV_MEMALLOCFLAG_GPU_WRITEABLE | \
    PVR_SRV_MEMALLOCFLAG_GPU_CACHE_MODE_MASK)
#define PVR_SRV_MEMALLOCFLAG_CPU_READABLE BITFIELD_BIT(4U)
#define PVR_SRV_MEMALLOCFLAG_CPU_WRITEABLE BITFIELD_BIT(5U)
#define PVR_SRV_MEMALLOCFLAG_CPU_CACHE_MODE_MASK (7ULL << 11U)
#define PVR_SRV_MEMALLOCFLAG_CPU_CACHE_INCOHERENT (3ULL << 11U)
#define PVR_SRV_MEMALLOCFLAGS_CPU_MMUFLAGSMASK                               \
   (PVR_SRV_MEMALLOCFLAG_CPU_READABLE | PVR_SRV_MEMALLOCFLAG_CPU_WRITEABLE | \
    PVR_SRV_MEMALLOCFLAG_CPU_CACHE_MODE_MASK)
#define PVR_SRV_MEMALLOCFLAG_NO_OSPAGES_ON_ALLOC BITFIELD_BIT(15U)
#define PVR_SRV_MEMALLOCFLAG_SPARSE_NO_DUMMY_BACKING BITFIELD_BIT(18U)
#define PVR_SRV_MEMALLOCFLAG_SPARSE_ZERO_BACKING BITFIELD_BIT(20U)
#define PVR_SRV_MEMALLOCFLAG_FW_ALLOC_OSID_MASK (7ULL << 23U)
#define PVR_SRV_MEMALLOCFLAG_VAL_SECURE_BUFFER BITFIELD64_BIT(34U)
#define PVR_SRV_MEMALLOCFLAG_VAL_SHARED_BUFFER BITFIELD64_BIT(35U)
#define PVR_SRV_PHYS_HEAP_HINT_SHIFT (60U)
#define PVR_SRV_PHYS_HEAP_HINT_MASK (0xFULL << PVR_SRV_PHYS_HEAP_HINT_SHIFT)
#define PVR_SRV_MEMALLOCFLAG_GPU_UNCACHED BITFIELD_BIT(8U)
#define PVR_SRV_MEMALLOCFLAG_GPU_CACHE_INCOHERENT (3ULL << 8U)
#define PVR_SRV_MEMALLOCFLAG_CPU_UNCACHED_WC (0ULL << 11U)
#define PVR_SRV_MEMALLOCFLAG_GPU_READ_PERMITTED BITFIELD_BIT(2U)
#define PVR_SRV_MEMALLOCFLAG_GPU_WRITE_PERMITTED BITFIELD_BIT(3U)
#define PVR_SRV_MEMALLOCFLAG_CPU_READ_PERMITTED BITFIELD_BIT(6U)
#define PVR_SRV_MEMALLOCFLAG_CPU_WRITE_PERMITTED BITFIELD_BIT(7U)

#define PVR_SRV_MEMALLOCFLAGS_PMRFLAGSMASK                                \
   (PVR_SRV_MEMALLOCFLAG_DEVICE_FLAGS_MASK |                              \
    PVR_SRV_MEMALLOCFLAG_CPU_CACHE_CLEAN |                                \
    PVR_SRV_MEMALLOCFLAG_KERNEL_CPU_MAPPABLE |                            \
    PVR_SRV_MEMALLOCFLAG_ZERO_ON_ALLOC | PVR_SRV_MEMALLOCFLAG_SVM_ALLOC | \
    PVR_SRV_MEMALLOCFLAG_POISON_ON_ALLOC |                                \
    PVR_SRV_MEMALLOCFLAG_POISON_ON_FREE |                                 \
    PVR_SRV_MEMALLOCFLAGS_GPU_MMUFLAGSMASK |                              \
    PVR_SRV_MEMALLOCFLAGS_CPU_MMUFLAGSMASK |                              \
    PVR_SRV_MEMALLOCFLAG_NO_OSPAGES_ON_ALLOC |                            \
    PVR_SRV_MEMALLOCFLAG_SPARSE_NO_DUMMY_BACKING |                        \
    PVR_SRV_MEMALLOCFLAG_SPARSE_ZERO_BACKING |                            \
    PVR_SRV_MEMALLOCFLAG_FW_ALLOC_OSID_MASK |                             \
    PVR_SRV_MEMALLOCFLAG_VAL_SECURE_BUFFER |                              \
    PVR_SRV_MEMALLOCFLAG_VAL_SHARED_BUFFER | PVR_SRV_PHYS_HEAP_HINT_MASK)

#define PVR_SRV_MEMALLOCFLAGS_PHYSICAL_MASK    \
   (PVR_SRV_MEMALLOCFLAGS_CPU_MMUFLAGSMASK |   \
    PVR_SRV_MEMALLOCFLAG_GPU_CACHE_MODE_MASK | \
    PVR_SRV_MEMALLOCFLAG_CPU_READ_PERMITTED |  \
    PVR_SRV_MEMALLOCFLAG_CPU_WRITE_PERMITTED | \
    PVR_SRV_MEMALLOCFLAG_CPU_CACHE_CLEAN |     \
    PVR_SRV_MEMALLOCFLAG_ZERO_ON_ALLOC |       \
    PVR_SRV_MEMALLOCFLAG_POISON_ON_ALLOC |     \
    PVR_SRV_MEMALLOCFLAG_POISON_ON_FREE | PVR_SRV_PHYS_HEAP_HINT_MASK)

#define PVR_SRV_MEMALLOCFLAGS_VIRTUAL_MASK    \
   (PVR_SRV_MEMALLOCFLAGS_GPU_MMUFLAGSMASK |  \
    PVR_SRV_MEMALLOCFLAG_GPU_READ_PERMITTED | \
    PVR_SRV_MEMALLOCFLAG_GPU_WRITE_PERMITTED)

/* Device specific MMU flags. */
/*!< Memory that only the PM and Firmware can access */
#define PM_FW_PROTECT BITFIELD_BIT(0U)

/* Helper macro for setting device specific MMU flags. */
#define PVR_SRV_MEMALLOCFLAG_DEVICE_FLAG(n)             \
   (((n) << PVR_SRV_MEMALLOCFLAG_DEVICE_FLAGS_OFFSET) & \
    PVR_SRV_MEMALLOCFLAG_DEVICE_FLAGS_MASK)

/*******************************************
   struct definitions
 *******************************************/

struct pvr_srv_winsys_bo {
   struct pvr_winsys_bo base;
   uint32_t ref_count;
   void *pmr;

   bool is_display_buffer;
   uint32_t handle;
   uint64_t flags;
};

struct pvr_srv_winsys_vma {
   struct pvr_winsys_vma base;
   void *reservation;

   /* Required when mapping whole PMR, used for display buffers mapping. */
   void *mapping;
};

/*******************************************
   function prototypes
 *******************************************/

VkResult pvr_srv_winsys_buffer_create(struct pvr_winsys *ws,
                                      uint64_t size,
                                      uint64_t alignment,
                                      enum pvr_winsys_bo_type type,
                                      uint32_t ws_flags,
                                      struct pvr_winsys_bo **const bo_out);
VkResult
pvr_srv_winsys_buffer_create_from_fd(struct pvr_winsys *ws,
                                     int fd,
                                     struct pvr_winsys_bo **const bo_out);
void pvr_srv_winsys_buffer_destroy(struct pvr_winsys_bo *bo);

VkResult pvr_srv_winsys_buffer_get_fd(struct pvr_winsys_bo *bo,
                                      int *const fd_out);

VkResult pvr_srv_winsys_buffer_map(struct pvr_winsys_bo *bo);
void pvr_srv_winsys_buffer_unmap(struct pvr_winsys_bo *bo);

VkResult pvr_srv_heap_alloc_carveout(struct pvr_winsys_heap *heap,
                                     const pvr_dev_addr_t carveout_dev_addr,
                                     uint64_t size,
                                     uint64_t alignment,
                                     struct pvr_winsys_vma **vma_out);
VkResult pvr_srv_winsys_heap_alloc(struct pvr_winsys_heap *heap,
                                   uint64_t size,
                                   uint64_t alignment,
                                   struct pvr_winsys_vma **vma_out);
void pvr_srv_winsys_heap_free(struct pvr_winsys_vma *vma);

VkResult pvr_srv_winsys_vma_map(struct pvr_winsys_vma *vma,
                                struct pvr_winsys_bo *bo,
                                uint64_t offset,
                                uint64_t size,
                                pvr_dev_addr_t *dev_addr_out);
void pvr_srv_winsys_vma_unmap(struct pvr_winsys_vma *vma);

/*******************************************
   helper macros
 *******************************************/

#define to_pvr_srv_winsys_bo(bo) \
   container_of((bo), struct pvr_srv_winsys_bo, base)
#define to_pvr_srv_winsys_vma(vma) \
   container_of((vma), struct pvr_srv_winsys_vma, base)

#endif /* PVR_SRV_BO_H */
