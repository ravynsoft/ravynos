/*
 * Copyright 2021 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "vn_renderer_internal.h"

/* 3 seconds */
#define VN_RENDERER_SHMEM_CACHE_EXPIRACY (3ll * 1000 * 1000)

static void
vn_renderer_shmem_cache_dump(struct vn_renderer_shmem_cache *cache)
{
   simple_mtx_lock(&cache->mutex);

   vn_log(NULL, "dumping renderer shmem cache");
   vn_log(NULL, "  cache skip: %d", cache->debug.cache_skip_count);
   vn_log(NULL, "  cache hit: %d", cache->debug.cache_hit_count);
   vn_log(NULL, "  cache miss: %d", cache->debug.cache_miss_count);

   uint32_t bucket_mask = cache->bucket_mask;
   while (bucket_mask) {
      const int idx = u_bit_scan(&bucket_mask);
      const struct vn_renderer_shmem_bucket *bucket = &cache->buckets[idx];
      uint32_t count = 0;
      list_for_each_entry(struct vn_renderer_shmem, shmem, &bucket->shmems,
                          cache_head)
         count++;
      if (count)
         vn_log(NULL, "  buckets[%d]: %d shmems", idx, count);
   }

   simple_mtx_unlock(&cache->mutex);
}

void
vn_renderer_shmem_cache_init(struct vn_renderer_shmem_cache *cache,
                             struct vn_renderer *renderer,
                             vn_renderer_shmem_cache_destroy_func destroy_func)
{
   /* cache->bucket_mask is 32-bit and u_bit_scan is used */
   static_assert(ARRAY_SIZE(cache->buckets) <= 32, "");

   cache->renderer = renderer;
   cache->destroy_func = destroy_func;

   simple_mtx_init(&cache->mutex, mtx_plain);

   for (uint32_t i = 0; i < ARRAY_SIZE(cache->buckets); i++) {
      struct vn_renderer_shmem_bucket *bucket = &cache->buckets[i];
      list_inithead(&bucket->shmems);
   }

   cache->initialized = true;
}

void
vn_renderer_shmem_cache_fini(struct vn_renderer_shmem_cache *cache)
{
   if (!cache->initialized)
      return;

   if (VN_DEBUG(CACHE))
      vn_renderer_shmem_cache_dump(cache);

   while (cache->bucket_mask) {
      const int idx = u_bit_scan(&cache->bucket_mask);
      struct vn_renderer_shmem_bucket *bucket = &cache->buckets[idx];

      list_for_each_entry_safe(struct vn_renderer_shmem, shmem,
                               &bucket->shmems, cache_head)
         cache->destroy_func(cache->renderer, shmem);
   }

   simple_mtx_destroy(&cache->mutex);
}

static struct vn_renderer_shmem_bucket *
choose_bucket(struct vn_renderer_shmem_cache *cache,
              size_t size,
              int *out_idx)
{
   assert(size);
   if (unlikely(!util_is_power_of_two_or_zero64(size)))
      return NULL;

   const uint32_t idx = ffsll(size) - 1;
   if (unlikely(idx >= ARRAY_SIZE(cache->buckets)))
      return NULL;

   *out_idx = idx;
   return &cache->buckets[idx];
}

static void
vn_renderer_shmem_cache_remove_expired_locked(
   struct vn_renderer_shmem_cache *cache, int64_t now)
{
   uint32_t bucket_mask = cache->bucket_mask;
   while (bucket_mask) {
      const int idx = u_bit_scan(&bucket_mask);
      struct vn_renderer_shmem_bucket *bucket = &cache->buckets[idx];

      assert(!list_is_empty(&bucket->shmems));
      const struct vn_renderer_shmem *last_shmem = list_last_entry(
         &bucket->shmems, struct vn_renderer_shmem, cache_head);

      /* remove expired shmems but keep at least the last one */
      list_for_each_entry_safe(struct vn_renderer_shmem, shmem,
                               &bucket->shmems, cache_head) {
         if (shmem == last_shmem ||
             now - shmem->cache_timestamp < VN_RENDERER_SHMEM_CACHE_EXPIRACY)
            break;

         list_del(&shmem->cache_head);
         cache->destroy_func(cache->renderer, shmem);
      }
   }
}

bool
vn_renderer_shmem_cache_add(struct vn_renderer_shmem_cache *cache,
                            struct vn_renderer_shmem *shmem)
{
   assert(!vn_refcount_is_valid(&shmem->refcount));

   int idx;
   struct vn_renderer_shmem_bucket *bucket =
      choose_bucket(cache, shmem->mmap_size, &idx);
   if (!bucket)
      return false;

   const int64_t now = os_time_get();
   shmem->cache_timestamp = now;

   simple_mtx_lock(&cache->mutex);

   vn_renderer_shmem_cache_remove_expired_locked(cache, now);

   list_addtail(&shmem->cache_head, &bucket->shmems);
   cache->bucket_mask |= 1 << idx;

   simple_mtx_unlock(&cache->mutex);

   return true;
}

struct vn_renderer_shmem *
vn_renderer_shmem_cache_get(struct vn_renderer_shmem_cache *cache,
                            size_t size)
{
   int idx;
   struct vn_renderer_shmem_bucket *bucket = choose_bucket(cache, size, &idx);
   if (!bucket) {
      VN_TRACE_SCOPE("shmem cache skip");
      simple_mtx_lock(&cache->mutex);
      cache->debug.cache_skip_count++;
      simple_mtx_unlock(&cache->mutex);
      return NULL;
   }

   struct vn_renderer_shmem *shmem = NULL;

   simple_mtx_lock(&cache->mutex);
   if (cache->bucket_mask & (1 << idx)) {
      assert(!list_is_empty(&bucket->shmems));
      shmem = list_first_entry(&bucket->shmems, struct vn_renderer_shmem,
                               cache_head);
      list_del(&shmem->cache_head);

      if (list_is_empty(&bucket->shmems))
         cache->bucket_mask &= ~(1 << idx);

      cache->debug.cache_hit_count++;
   } else {
      VN_TRACE_SCOPE("shmem cache miss");
      cache->debug.cache_miss_count++;
   }
   simple_mtx_unlock(&cache->mutex);

   return shmem;
}
