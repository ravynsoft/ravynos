/*
 * Copyright © 2008 Jérôme Glisse
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RADEON_DRM_BO_H
#define RADEON_DRM_BO_H

#include "radeon_drm_winsys.h"
#include "util/u_thread.h"
#include "pipebuffer/pb_slab.h"

struct radeon_bo {
   struct pb_buffer_lean base;
   union {
      struct {
         struct pb_cache_entry cache_entry;

         void *ptr;
         mtx_t map_mutex;
         unsigned map_count;
         bool use_reusable_pool;
      } real;
      struct {
         struct pb_slab_entry entry;
         struct radeon_bo *real;

         unsigned num_fences;
         unsigned max_fences;
         struct radeon_bo **fences;
      } slab;
   } u;

   struct radeon_drm_winsys *rws;
   void *user_ptr; /* from buffer_from_ptr */

   uint32_t handle; /* 0 for slab entries */
   uint32_t flink_name;
   uint64_t va;
   uint32_t hash;
   enum radeon_bo_domain initial_domain;

   /* how many command streams is this bo referenced in? */
   int num_cs_references;

   /* how many command streams, which are being emitted in a separate
    * thread, is this bo referenced in? */
   int num_active_ioctls;
};

struct radeon_slab {
   struct pb_slab base;
   struct radeon_bo *buffer;
   struct radeon_bo *entries;
};

void radeon_bo_destroy(void *winsys, struct pb_buffer_lean *_buf);
bool radeon_bo_can_reclaim(void *winsys, struct pb_buffer_lean *_buf);
void radeon_drm_bo_init_functions(struct radeon_drm_winsys *ws);

bool radeon_bo_can_reclaim_slab(void *priv, struct pb_slab_entry *entry);
struct pb_slab *radeon_bo_slab_alloc(void *priv, unsigned heap,
                                     unsigned entry_size,
                                     unsigned group_index);
void radeon_bo_slab_free(void *priv, struct pb_slab *slab);

static inline void
radeon_ws_bo_reference(struct radeon_winsys *rws, struct radeon_bo **dst,
                       struct radeon_bo *src)
{
   radeon_bo_reference(rws, (struct pb_buffer_lean**)dst, (struct pb_buffer_lean*)src);
}

void *radeon_bo_do_map(struct radeon_bo *bo);

#endif
