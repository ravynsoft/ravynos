/*
 * Copyright 2021 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VN_RENDERER_UTIL_H
#define VN_RENDERER_UTIL_H

#include "vn_renderer.h"

/* for suballocations of short-lived shmems, thread-safe */
struct vn_renderer_shmem_pool {
   mtx_t mutex;
   size_t min_alloc_size;

   struct vn_renderer_shmem *shmem;
   size_t size;
   size_t used;
};

static inline VkResult
vn_renderer_submit_simple(struct vn_renderer *renderer,
                          const void *cs_data,
                          size_t cs_size)
{
   const struct vn_renderer_submit submit = {
      .batches =
         &(const struct vn_renderer_submit_batch){
            .cs_data = cs_data,
            .cs_size = cs_size,
            .ring_idx = 0, /* CPU ring */
         },
      .batch_count = 1,
   };
   return vn_renderer_submit(renderer, &submit);
}

void
vn_renderer_shmem_pool_init(struct vn_renderer *renderer,
                            struct vn_renderer_shmem_pool *pool,
                            size_t min_alloc_size);

void
vn_renderer_shmem_pool_fini(struct vn_renderer *renderer,
                            struct vn_renderer_shmem_pool *pool);

struct vn_renderer_shmem *
vn_renderer_shmem_pool_alloc(struct vn_renderer *renderer,
                             struct vn_renderer_shmem_pool *pool,
                             size_t size,
                             size_t *out_offset);

#endif /* VN_RENDERER_UTIL_H */
