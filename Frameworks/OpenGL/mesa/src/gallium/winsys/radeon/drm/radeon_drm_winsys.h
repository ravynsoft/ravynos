/*
 * Copyright © 2009 Corbin Simpson
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RADEON_DRM_WINSYS_H
#define RADEON_DRM_WINSYS_H

#include "winsys/radeon_winsys.h"
#include "pipebuffer/pb_cache.h"
#include "pipebuffer/pb_slab.h"
#include "util/u_queue.h"
#include "util/list.h"
#include <radeon_drm.h>

struct radeon_drm_cs;

enum radeon_generation {
   DRV_R300,
   DRV_R600,
   DRV_SI
};

#define RADEON_SLAB_MIN_SIZE_LOG2 9
#define RADEON_SLAB_MAX_SIZE_LOG2 14

struct radeon_vm_heap {
   mtx_t mutex;
   uint64_t start;
   uint64_t end;
   struct list_head holes;
};

struct radeon_drm_winsys {
   struct radeon_winsys base;
   struct pipe_reference reference;
   struct pb_cache bo_cache;
   struct pb_slabs bo_slabs;

   int fd; /* DRM file descriptor */
   int num_cs; /* The number of command streams created. */
   uint64_t allocated_vram;
   uint64_t allocated_gtt;
   uint64_t mapped_vram;
   uint64_t mapped_gtt;
   uint64_t buffer_wait_time; /* time spent in buffer_wait in ns */
   uint64_t num_gfx_IBs;
   uint64_t num_sdma_IBs;
   uint64_t num_mapped_buffers;
   uint32_t next_bo_hash;

   enum radeon_generation gen;
   struct radeon_info info;
   uint32_t va_start;
   uint32_t va_unmap_working;
   uint32_t accel_working2;

   /* List of buffer GEM names. Protected by bo_handles_mutex. */
   struct hash_table *bo_names;
   /* List of buffer handles. Protected by bo_handles_mutex. */
   struct hash_table *bo_handles;
   /* List of buffer virtual memory ranges. Protected by bo_handles_mutex. */
   struct hash_table_u64 *bo_vas;
   mtx_t bo_handles_mutex;
   mtx_t bo_fence_lock;

   struct radeon_vm_heap vm32;
   struct radeon_vm_heap vm64;

   bool check_vm;
   bool noop_cs;

   struct radeon_surface_manager *surf_man;

   uint32_t num_cpus;      /* Number of CPUs. */

   struct radeon_drm_cs *hyperz_owner;
   mtx_t hyperz_owner_mutex;
   struct radeon_drm_cs *cmask_owner;
   mtx_t cmask_owner_mutex;

   /* multithreaded command submission */
   struct util_queue cs_queue;
};

static inline struct radeon_drm_winsys *radeon_drm_winsys(struct radeon_winsys *base)
{
   return (struct radeon_drm_winsys*)base;
}

uint32_t radeon_drm_get_gpu_reset_counter(struct radeon_drm_winsys *ws);
void radeon_surface_init_functions(struct radeon_drm_winsys *ws);

#endif
