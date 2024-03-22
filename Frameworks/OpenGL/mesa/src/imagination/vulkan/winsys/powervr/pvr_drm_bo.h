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

#ifndef PVR_DRM_BO_H
#define PVR_DRM_BO_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "pvr_drm.h"
#include "pvr_winsys.h"
#include "util/macros.h"

/*******************************************
   struct definitions
 *******************************************/

struct pvr_drm_winsys_bo {
   struct pvr_winsys_bo base;
   uint32_t ref_count;

   uint32_t handle;
   uint64_t flags;
};

struct pvr_drm_winsys_vma {
   struct pvr_winsys_vma base;
};

/*******************************************
   function prototypes
 *******************************************/

VkResult pvr_drm_winsys_buffer_create(struct pvr_winsys *ws,
                                      uint64_t size,
                                      uint64_t alignment,
                                      enum pvr_winsys_bo_type type,
                                      uint32_t ws_flags,
                                      struct pvr_winsys_bo **const bo_out);
VkResult
pvr_drm_winsys_buffer_create_from_fd(struct pvr_winsys *ws,
                                     int fd,
                                     struct pvr_winsys_bo **const bo_out);
void pvr_drm_winsys_buffer_destroy(struct pvr_winsys_bo *bo);

VkResult pvr_drm_winsys_buffer_get_fd(struct pvr_winsys_bo *bo,
                                      int *const fd_out);

VkResult pvr_drm_winsys_buffer_map(struct pvr_winsys_bo *bo);
void pvr_drm_winsys_buffer_unmap(struct pvr_winsys_bo *bo);

VkResult pvr_drm_heap_alloc_carveout(struct pvr_winsys_heap *const heap,
                                     const pvr_dev_addr_t carveout_dev_addr,
                                     uint64_t size,
                                     uint64_t alignment,
                                     struct pvr_winsys_vma **vma_out);
VkResult pvr_drm_winsys_heap_alloc(struct pvr_winsys_heap *heap,
                                   uint64_t size,
                                   uint64_t alignment,
                                   struct pvr_winsys_vma **vma_out);
void pvr_drm_winsys_heap_free(struct pvr_winsys_vma *vma);

VkResult pvr_drm_winsys_vma_map(struct pvr_winsys_vma *vma,
                                struct pvr_winsys_bo *bo,
                                uint64_t offset,
                                uint64_t size,
                                pvr_dev_addr_t *dev_addr_out);
void pvr_drm_winsys_vma_unmap(struct pvr_winsys_vma *vma);

/*******************************************
   helper macros
 *******************************************/

#define to_pvr_drm_winsys_bo(bo) \
   container_of((bo), struct pvr_drm_winsys_bo, base)
#define to_pvr_drm_winsys_vma(vma) \
   container_of((vma), struct pvr_drm_winsys_vma, base)

#endif /* PVR_DRM_BO_H */
