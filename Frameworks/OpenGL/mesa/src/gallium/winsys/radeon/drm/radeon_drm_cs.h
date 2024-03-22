/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RADEON_DRM_CS_H
#define RADEON_DRM_CS_H

#include "radeon_drm_bo.h"

struct radeon_ctx {
   struct radeon_drm_winsys *ws;
   uint32_t gpu_reset_counter;
};

struct radeon_bo_item {
   struct radeon_bo    *bo;
   union {
      struct {
         uint32_t    priority_usage;
      } real;
      struct {
         unsigned    real_idx;
      } slab;
   } u;
};

struct radeon_cs_context {
   uint32_t                    buf[16 * 1024];

   int                         fd;
   struct drm_radeon_cs        cs;
   struct drm_radeon_cs_chunk  chunks[3];
   uint64_t                    chunk_array[3];
   uint32_t                    flags[2];

   /* Buffers. */
   unsigned                    max_relocs;
   unsigned                    num_relocs;
   unsigned                    num_validated_relocs;
   struct radeon_bo_item       *relocs_bo;
   struct drm_radeon_cs_reloc  *relocs;

   unsigned                    num_slab_buffers;
   unsigned                    max_slab_buffers;
   struct radeon_bo_item       *slab_buffers;

   int                         reloc_indices_hashlist[4096];
};

struct radeon_drm_cs {
   enum amd_ip_type          ip_type;

   /* We flip between these two CS. While one is being consumed
    * by the kernel in another thread, the other one is being filled
    * by the pipe driver. */
   struct radeon_cs_context csc1;
   struct radeon_cs_context csc2;
   /* The currently-used CS. */
   struct radeon_cs_context *csc;
   /* The CS being currently-owned by the other thread. */
   struct radeon_cs_context *cst;

   /* The winsys. */
   struct radeon_drm_winsys *ws;

   /* Flush CS. */
   void (*flush_cs)(void *ctx, unsigned flags, struct pipe_fence_handle **fence);
   void *flush_data;

   struct util_queue_fence flush_completed;
   struct pipe_fence_handle *next_fence;
};

int radeon_lookup_buffer(struct radeon_winsys *rws, struct radeon_cs_context *csc,
                         struct radeon_bo *bo);

static inline struct radeon_drm_cs *
radeon_drm_cs(struct radeon_cmdbuf *rcs)
{
   return (struct radeon_drm_cs*)rcs->priv;
}

static inline bool
radeon_bo_is_referenced_by_cs(struct radeon_drm_cs *cs,
                              struct radeon_bo *bo)
{
   int num_refs = bo->num_cs_references;
   return num_refs == bo->rws->num_cs ||
         (num_refs && radeon_lookup_buffer(&cs->ws->base, cs->csc, bo) != -1);
}

static inline bool
radeon_bo_is_referenced_by_cs_for_write(struct radeon_drm_cs *cs,
                                        struct radeon_bo *bo)
{
   int index;

   if (!bo->num_cs_references)
      return false;

   index = radeon_lookup_buffer(&cs->ws->base, cs->csc, bo);
   if (index == -1)
      return false;

   if (!bo->handle)
      index = cs->csc->slab_buffers[index].u.slab.real_idx;

   return cs->csc->relocs[index].write_domain != 0;
}

static inline bool
radeon_bo_is_referenced_by_any_cs(struct radeon_bo *bo)
{
   return bo->num_cs_references != 0;
}

void radeon_drm_cs_sync_flush(struct radeon_cmdbuf *rcs);
void radeon_drm_cs_init_functions(struct radeon_drm_winsys *ws);
void radeon_drm_cs_emit_ioctl_oneshot(void *job, void *gdata, int thread_index);

#endif
