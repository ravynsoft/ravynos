/*
 * Copyright 2021 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VN_RENDERER_INTERNAL_H
#define VN_RENDERER_INTERNAL_H

#include "vn_renderer.h"

typedef void (*vn_renderer_shmem_cache_destroy_func)(
   struct vn_renderer *renderer, struct vn_renderer_shmem *shmem);

struct vn_renderer_shmem_cache {
   bool initialized;

   struct vn_renderer *renderer;
   vn_renderer_shmem_cache_destroy_func destroy_func;

   simple_mtx_t mutex;

   /* cache shmems up to 2^26 in size (see choose_bucket) */
   struct vn_renderer_shmem_bucket {
      struct list_head shmems;
   } buckets[27];

   /* which buckets have shmems */
   uint32_t bucket_mask;

   struct {
      uint32_t cache_skip_count;
      uint32_t cache_hit_count;
      uint32_t cache_miss_count;
   } debug;
};

void
vn_renderer_shmem_cache_init(
   struct vn_renderer_shmem_cache *cache,
   struct vn_renderer *renderer,
   vn_renderer_shmem_cache_destroy_func destroy_func);

void
vn_renderer_shmem_cache_fini(struct vn_renderer_shmem_cache *cache);

bool
vn_renderer_shmem_cache_add(struct vn_renderer_shmem_cache *cache,
                            struct vn_renderer_shmem *shmem);

struct vn_renderer_shmem *
vn_renderer_shmem_cache_get(struct vn_renderer_shmem_cache *cache,
                            size_t size);

#endif /* VN_RENDERER_INTERNAL_H */
