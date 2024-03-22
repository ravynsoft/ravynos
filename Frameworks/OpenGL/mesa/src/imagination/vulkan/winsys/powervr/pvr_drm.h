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

#ifndef PVR_DRM_H
#define PVR_DRM_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "pvr_winsys.h"
#include "util/macros.h"

struct pvr_drm_winsys_heap {
   struct pvr_winsys_heap base;
};

struct pvr_drm_winsys {
   struct pvr_winsys base;

   /* Packed bvnc */
   uint64_t bvnc;

   /* Required heaps */
   struct pvr_drm_winsys_heap general_heap;
   struct pvr_drm_winsys_heap pds_heap;
   struct pvr_drm_winsys_heap usc_heap;
   struct pvr_drm_winsys_heap vis_test_heap;
   struct pvr_drm_winsys_heap transfer_frag_heap;

   /* Optional heaps */
   bool rgn_hdr_heap_present;
   struct pvr_drm_winsys_heap rgn_hdr_heap;

   /* vma's for carveout memory regions */
   struct pvr_winsys_vma *pds_vma;
   struct pvr_winsys_vma *usc_vma;
   struct pvr_winsys_vma *general_vma;

   uint32_t vm_context;
};

/*******************************************
    helper macros
 *******************************************/

#define to_pvr_drm_winsys(ws) container_of((ws), struct pvr_drm_winsys, base)
#define to_pvr_drm_winsys_heap(heap) \
   container_of((heap), struct pvr_drm_winsys_heap, base)

#endif /* PVR_DRM_H */
