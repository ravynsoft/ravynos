/*
 * Copyright 2021 Alyssa Rosenzweig
 * Copyright 2019 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "agx_bo.h"
#include <inttypes.h>
#include "agx_device.h"
#include "decode.h"

/* Helper to calculate the bucket index of a BO */
static unsigned
agx_bucket_index(unsigned size)
{
   /* Round down to POT to compute a bucket index */
   unsigned bucket_index = util_logbase2(size);

   /* Clamp to supported buckets. Huge allocations use the largest bucket */
   bucket_index = CLAMP(bucket_index, MIN_BO_CACHE_BUCKET, MAX_BO_CACHE_BUCKET);

   /* Reindex from 0 */
   return (bucket_index - MIN_BO_CACHE_BUCKET);
}

static struct list_head *
agx_bucket(struct agx_device *dev, unsigned size)
{
   return &dev->bo_cache.buckets[agx_bucket_index(size)];
}

static bool
agx_bo_wait(struct agx_bo *bo, int64_t timeout_ns)
{
   /* TODO: When we allow parallelism we'll need to implement this for real */
   return true;
}

static void
agx_bo_cache_remove_locked(struct agx_device *dev, struct agx_bo *bo)
{
   simple_mtx_assert_locked(&dev->bo_cache.lock);
   list_del(&bo->bucket_link);
   list_del(&bo->lru_link);
   dev->bo_cache.size -= bo->size;
}

/* Tries to fetch a BO of sufficient size with the appropriate flags from the
 * BO cache. If it succeeds, it returns that BO and removes the BO from the
 * cache. If it fails, it returns NULL signaling the caller to allocate a new
 * BO. */

struct agx_bo *
agx_bo_cache_fetch(struct agx_device *dev, size_t size, size_t align,
                   uint32_t flags, const bool dontwait)
{
   simple_mtx_lock(&dev->bo_cache.lock);
   struct list_head *bucket = agx_bucket(dev, size);
   struct agx_bo *bo = NULL;

   /* Iterate the bucket looking for something suitable */
   list_for_each_entry_safe(struct agx_bo, entry, bucket, bucket_link) {
      if (entry->size < size || entry->flags != flags)
         continue;

      /* Do not return more than 2x oversized BOs. */
      if (entry->size > 2 * size)
         continue;

      if (align > entry->align)
         continue;

      /* If the oldest BO in the cache is busy, likely so is
       * everything newer, so bail. */
      if (!agx_bo_wait(entry, dontwait ? 0 : INT64_MAX))
         break;

      /* This one works, use it */
      agx_bo_cache_remove_locked(dev, entry);
      bo = entry;
      break;
   }
   simple_mtx_unlock(&dev->bo_cache.lock);

   return bo;
}

static void
agx_bo_cache_evict_stale_bos(struct agx_device *dev, unsigned tv_sec)
{
   struct timespec time;

   clock_gettime(CLOCK_MONOTONIC, &time);
   list_for_each_entry_safe(struct agx_bo, entry, &dev->bo_cache.lru,
                            lru_link) {
      /* We want all entries that have been used more than 1 sec ago to be
       * dropped, others can be kept.  Note the <= 2 check and not <= 1. It's
       * here to account for the fact that we're only testing ->tv_sec, not
       * ->tv_nsec.  That means we might keep entries that are between 1 and 2
       * seconds old, but we don't really care, as long as unused BOs are
       * dropped at some point.
       */
      if (time.tv_sec - entry->last_used <= 2)
         break;

      agx_bo_cache_remove_locked(dev, entry);
      agx_bo_free(dev, entry);
   }
}

static void
agx_bo_cache_put_locked(struct agx_bo *bo)
{
   struct agx_device *dev = bo->dev;
   struct list_head *bucket = agx_bucket(dev, bo->size);
   struct timespec time;

   /* Add us to the bucket */
   list_addtail(&bo->bucket_link, bucket);

   /* Add us to the LRU list and update the last_used field. */
   list_addtail(&bo->lru_link, &dev->bo_cache.lru);
   clock_gettime(CLOCK_MONOTONIC, &time);
   bo->last_used = time.tv_sec;

   /* Update statistics */
   dev->bo_cache.size += bo->size;

   if (0) {
      printf("BO cache: %zu KiB (+%zu KiB from %s, hit/miss %" PRIu64
             "/%" PRIu64 ")\n",
             DIV_ROUND_UP(dev->bo_cache.size, 1024),
             DIV_ROUND_UP(bo->size, 1024), bo->label,
             p_atomic_read(&dev->bo_cache.hits),
             p_atomic_read(&dev->bo_cache.misses));
   }

   /* Update label for debug */
   bo->label = "Unused (BO cache)";

   /* Let's do some cleanup in the BO cache while we hold the lock. */
   agx_bo_cache_evict_stale_bos(dev, time.tv_sec);
}

/* Tries to add a BO to the cache. Returns if it was successful */
static bool
agx_bo_cache_put(struct agx_bo *bo)
{
   struct agx_device *dev = bo->dev;

   if (bo->flags & AGX_BO_SHARED) {
      return false;
   } else {
      simple_mtx_lock(&dev->bo_cache.lock);
      agx_bo_cache_put_locked(bo);
      simple_mtx_unlock(&dev->bo_cache.lock);

      return true;
   }
}

void
agx_bo_cache_evict_all(struct agx_device *dev)
{
   simple_mtx_lock(&dev->bo_cache.lock);
   for (unsigned i = 0; i < ARRAY_SIZE(dev->bo_cache.buckets); ++i) {
      struct list_head *bucket = &dev->bo_cache.buckets[i];

      list_for_each_entry_safe(struct agx_bo, entry, bucket, bucket_link) {
         agx_bo_cache_remove_locked(dev, entry);
         agx_bo_free(dev, entry);
      }
   }
   simple_mtx_unlock(&dev->bo_cache.lock);
}

void
agx_bo_reference(struct agx_bo *bo)
{
   if (bo) {
      ASSERTED int count = p_atomic_inc_return(&bo->refcnt);
      assert(count != 1);
   }
}

void
agx_bo_unreference(struct agx_bo *bo)
{
   if (!bo)
      return;

   /* Don't return to cache if there are still references */
   if (p_atomic_dec_return(&bo->refcnt))
      return;

   struct agx_device *dev = bo->dev;

   pthread_mutex_lock(&dev->bo_map_lock);

   /* Someone might have imported this BO while we were waiting for the
    * lock, let's make sure it's still not referenced before freeing it.
    */
   if (p_atomic_read(&bo->refcnt) == 0) {
      assert(!p_atomic_read_relaxed(&bo->writer_syncobj));

      if (dev->debug & AGX_DBG_TRACE)
         agxdecode_track_free(bo);

      if (!agx_bo_cache_put(bo))
         agx_bo_free(dev, bo);
   }

   pthread_mutex_unlock(&dev->bo_map_lock);
}

struct agx_bo *
agx_bo_create_aligned(struct agx_device *dev, unsigned size, unsigned align,
                      enum agx_bo_flags flags, const char *label)
{
   struct agx_bo *bo;
   assert(size > 0);

   /* To maximize BO cache usage, don't allocate tiny BOs */
   size = ALIGN_POT(size, 16384);

   /* See if we have a BO already in the cache */
   bo = agx_bo_cache_fetch(dev, size, align, flags, true);

   /* Update stats based on the first attempt to fetch */
   if (bo != NULL)
      p_atomic_inc(&dev->bo_cache.hits);
   else
      p_atomic_inc(&dev->bo_cache.misses);

   /* Otherwise, allocate a fresh BO. If allocation fails, we can try waiting
    * for something in the cache. But if there's no nothing suitable, we should
    * flush the cache to make space for the new allocation.
    */
   if (!bo)
      bo = agx_bo_alloc(dev, size, align, flags);
   if (!bo)
      bo = agx_bo_cache_fetch(dev, size, align, flags, false);
   if (!bo) {
      agx_bo_cache_evict_all(dev);
      bo = agx_bo_alloc(dev, size, align, flags);
   }

   if (!bo) {
      fprintf(stderr, "BO creation failed\n");
      return NULL;
   }

   bo->label = label;
   p_atomic_set(&bo->refcnt, 1);

   if (dev->debug & AGX_DBG_TRACE)
      agxdecode_track_alloc(bo);

   return bo;
}
