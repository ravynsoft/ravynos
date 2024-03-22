/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * based on amdgpu winsys.
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Advanced Micro Devices, Inc.
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

#ifndef RADV_AMDGPU_WINSYS_H
#define RADV_AMDGPU_WINSYS_H

#include <amdgpu.h>
#include <pthread.h>
#include "util/list.h"
#include "util/rwlock.h"
#include "ac_gpu_info.h"
#include "radv_radeon_winsys.h"

#include "vk_sync.h"
#include "vk_sync_timeline.h"

struct radv_amdgpu_winsys {
   struct radeon_winsys base;
   amdgpu_device_handle dev;

   struct radeon_info info;
   struct ac_addrlib *addrlib;

   bool debug_all_bos;
   bool debug_log_bos;
   bool use_ib_bos;
   bool zero_all_vram_allocs;
   bool reserve_vmid;
   uint64_t perftest;

   alignas(8) uint64_t allocated_vram;
   alignas(8) uint64_t allocated_vram_vis;
   alignas(8) uint64_t allocated_gtt;

   /* Global BO list */
   struct {
      struct radv_amdgpu_winsys_bo **bos;
      uint32_t count;
      uint32_t capacity;
      struct u_rwlock lock;
   } global_bo_list;

   /* BO log */
   struct u_rwlock log_bo_list_lock;
   struct list_head log_bo_list;

   const struct vk_sync_type *sync_types[3];
   struct vk_sync_type syncobj_sync_type;
   struct vk_sync_timeline_type emulated_timeline_sync_type;

   uint32_t refcount;
};

static inline struct radv_amdgpu_winsys *
radv_amdgpu_winsys(struct radeon_winsys *base)
{
   return (struct radv_amdgpu_winsys *)base;
}

#endif /* RADV_AMDGPU_WINSYS_H */
