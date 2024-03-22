/*
 * Copyright 2021 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "vn_renderer_util.h"

void
vn_renderer_shmem_pool_init(UNUSED struct vn_renderer *renderer,
                            struct vn_renderer_shmem_pool *pool,
                            size_t min_alloc_size)
{
   *pool = (struct vn_renderer_shmem_pool){
      /* power-of-two to hit shmem cache */
      .min_alloc_size = util_next_power_of_two(min_alloc_size),
   };
   mtx_init(&pool->mutex, mtx_plain);
}

void
vn_renderer_shmem_pool_fini(struct vn_renderer *renderer,
                            struct vn_renderer_shmem_pool *pool)
{
   if (pool->shmem)
      vn_renderer_shmem_unref(renderer, pool->shmem);
   mtx_destroy(&pool->mutex);
}

static bool
vn_renderer_shmem_pool_grow_locked(struct vn_renderer *renderer,
                                   struct vn_renderer_shmem_pool *pool,
                                   size_t size)
{
   VN_TRACE_FUNC();
   /* power-of-two to hit shmem cache */
   size_t alloc_size = pool->min_alloc_size;
   while (alloc_size < size) {
      alloc_size <<= 1;
      if (!alloc_size)
         return false;
   }

   struct vn_renderer_shmem *shmem =
      vn_renderer_shmem_create(renderer, alloc_size);
   if (!shmem)
      return false;

   if (pool->shmem)
      vn_renderer_shmem_unref(renderer, pool->shmem);

   pool->shmem = shmem;
   pool->size = alloc_size;
   pool->used = 0;

   return true;
}

struct vn_renderer_shmem *
vn_renderer_shmem_pool_alloc(struct vn_renderer *renderer,
                             struct vn_renderer_shmem_pool *pool,
                             size_t size,
                             size_t *out_offset)
{
   mtx_lock(&pool->mutex);
   if (unlikely(size > pool->size - pool->used)) {
      if (!vn_renderer_shmem_pool_grow_locked(renderer, pool, size)) {
         mtx_unlock(&pool->mutex);
         return NULL;
      }

      assert(size <= pool->size - pool->used);
   }

   struct vn_renderer_shmem *shmem =
      vn_renderer_shmem_ref(renderer, pool->shmem);
   *out_offset = pool->used;
   pool->used += size;
   mtx_unlock(&pool->mutex);

   return shmem;
}
