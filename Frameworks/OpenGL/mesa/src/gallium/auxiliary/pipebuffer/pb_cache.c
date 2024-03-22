/**************************************************************************
 *
 * Copyright 2007-2008 VMware, Inc.
 * Copyright 2015 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "pb_cache.h"
#include "util/u_memory.h"
#include "util/os_time.h"

/*
 * Helper function for detecting time outs, taking in account overflow.
 *
 * Returns true if the current time has elapsed beyond the specified interval.
 */
static inline bool
time_timeout_ms(unsigned start, unsigned interval, unsigned curr)
{
   unsigned end = start + interval;

   if (start <= end)
      return !(start <= curr && curr < end);
   else
      return !((start <= curr) || (curr < end));
}

static unsigned
time_get_ms(struct pb_cache *mgr)
{
   /* Return the time relative to msecs_base_time. */
   return os_time_get() / 1000 - mgr->msecs_base_time;
}

static struct pb_buffer_lean *
get_buffer(struct pb_cache *mgr, struct pb_cache_entry *entry)
{
   return (struct pb_buffer_lean*)((char*)entry - mgr->offsetof_pb_cache_entry);
}

/**
 * Actually destroy the buffer.
 */
static void
destroy_buffer_locked(struct pb_cache *mgr, struct pb_cache_entry *entry)
{
   struct pb_buffer_lean *buf = get_buffer(mgr, entry);

   assert(!pipe_is_referenced(&buf->reference));
   if (list_is_linked(&entry->head)) {
      list_del(&entry->head);
      assert(mgr->num_buffers);
      --mgr->num_buffers;
      mgr->cache_size -= buf->size;
   }
   mgr->destroy_buffer(mgr->winsys, buf);
}

/**
 * Free as many cache buffers from the list head as possible.
 */
static void
release_expired_buffers_locked(struct pb_cache *mgr, struct list_head *cache,
                               unsigned current_time_ms)
{
   struct list_head *curr, *next;
   struct pb_cache_entry *entry;

   curr = cache->next;
   next = curr->next;
   while (curr != cache) {
      entry = list_entry(curr, struct pb_cache_entry, head);

      if (!time_timeout_ms(entry->start_ms, mgr->msecs,
                           current_time_ms))
         break;

      destroy_buffer_locked(mgr, entry);

      curr = next;
      next = curr->next;
   }
}

/**
 * Add a buffer to the cache. This is typically done when the buffer is
 * being released.
 */
void
pb_cache_add_buffer(struct pb_cache *mgr, struct pb_cache_entry *entry)
{
   struct list_head *cache = &mgr->buckets[entry->bucket_index];
   struct pb_buffer_lean *buf = get_buffer(mgr, entry);
   unsigned i;

   simple_mtx_lock(&mgr->mutex);
   assert(!pipe_is_referenced(&buf->reference));

   unsigned current_time_ms = time_get_ms(mgr);

   for (i = 0; i < mgr->num_heaps; i++)
      release_expired_buffers_locked(mgr, &mgr->buckets[i], current_time_ms);

   /* Directly release any buffer that exceeds the limit. */
   if (mgr->cache_size + buf->size > mgr->max_cache_size) {
      mgr->destroy_buffer(mgr->winsys, buf);
      simple_mtx_unlock(&mgr->mutex);
      return;
   }

   entry->start_ms = time_get_ms(mgr);
   list_addtail(&entry->head, cache);
   ++mgr->num_buffers;
   mgr->cache_size += buf->size;
   simple_mtx_unlock(&mgr->mutex);
}

/**
 * \return 1   if compatible and can be reclaimed
 *         0   if incompatible
 *        -1   if compatible and can't be reclaimed
 */
static int
pb_cache_is_buffer_compat(struct pb_cache *mgr, struct pb_cache_entry *entry,
                          pb_size size, unsigned alignment, unsigned usage)
{
   struct pb_buffer_lean *buf = get_buffer(mgr, entry);

   if (!pb_check_usage(usage, buf->usage))
      return 0;

   /* be lenient with size */
   if (buf->size < size ||
       buf->size > (unsigned) (mgr->size_factor * size))
      return 0;

   if (usage & mgr->bypass_usage)
      return 0;

   if (!pb_check_alignment(alignment, 1u << buf->alignment_log2))
      return 0;

   return mgr->can_reclaim(mgr->winsys, buf) ? 1 : -1;
}

/**
 * Find a compatible buffer in the cache, return it, and remove it
 * from the cache.
 */
struct pb_buffer_lean *
pb_cache_reclaim_buffer(struct pb_cache *mgr, pb_size size,
                        unsigned alignment, unsigned usage,
                        unsigned bucket_index)
{
   struct pb_cache_entry *entry;
   struct pb_cache_entry *cur_entry;
   struct list_head *cur, *next;
   int ret = 0;

   assert(bucket_index < mgr->num_heaps);
   struct list_head *cache = &mgr->buckets[bucket_index];

   simple_mtx_lock(&mgr->mutex);

   entry = NULL;
   cur = cache->next;
   next = cur->next;

   /* search in the expired buffers, freeing them in the process */
   unsigned now = time_get_ms(mgr);
   while (cur != cache) {
      cur_entry = list_entry(cur, struct pb_cache_entry, head);

      if (!entry && (ret = pb_cache_is_buffer_compat(mgr, cur_entry, size,
                                                     alignment, usage)) > 0)
         entry = cur_entry;
      else if (time_timeout_ms(cur_entry->start_ms, mgr->msecs, now))
         destroy_buffer_locked(mgr, cur_entry);
      else
         /* This buffer (and all hereafter) are still hot in cache */
         break;

      /* the buffer is busy (and probably all remaining ones too) */
      if (ret == -1)
         break;

      cur = next;
      next = cur->next;
   }

   /* keep searching in the hot buffers */
   if (!entry && ret != -1) {
      while (cur != cache) {
         cur_entry = list_entry(cur, struct pb_cache_entry, head);
         ret = pb_cache_is_buffer_compat(mgr, cur_entry, size, alignment, usage);

         if (ret > 0) {
            entry = cur_entry;
            break;
         }
         if (ret == -1)
            break;
         /* no need to check the timeout here */
         cur = next;
         next = cur->next;
      }
   }

   /* found a compatible buffer, return it */
   if (entry) {
      struct pb_buffer_lean *buf = get_buffer(mgr, entry);

      mgr->cache_size -= buf->size;
      list_del(&entry->head);
      --mgr->num_buffers;
      simple_mtx_unlock(&mgr->mutex);
      /* Increase refcount */
      pipe_reference_init(&buf->reference, 1);
      return buf;
   }

   simple_mtx_unlock(&mgr->mutex);
   return NULL;
}

/**
 * Empty the cache. Useful when there is not enough memory.
 */
unsigned
pb_cache_release_all_buffers(struct pb_cache *mgr)
{
   struct list_head *curr, *next;
   struct pb_cache_entry *buf;
   unsigned num_reclaims = 0;
   unsigned i;

   simple_mtx_lock(&mgr->mutex);
   for (i = 0; i < mgr->num_heaps; i++) {
      struct list_head *cache = &mgr->buckets[i];

      curr = cache->next;
      next = curr->next;
      while (curr != cache) {
         buf = list_entry(curr, struct pb_cache_entry, head);
         destroy_buffer_locked(mgr, buf);
         num_reclaims++;
         curr = next;
         next = curr->next;
      }
   }
   simple_mtx_unlock(&mgr->mutex);
   return num_reclaims;
}

void
pb_cache_init_entry(struct pb_cache *mgr, struct pb_cache_entry *entry,
                    struct pb_buffer_lean *buf, unsigned bucket_index)
{
   assert(bucket_index < mgr->num_heaps);

   memset(entry, 0, sizeof(*entry));
   entry->bucket_index = bucket_index;
}

/**
 * Initialize a caching buffer manager.
 *
 * @param mgr     The cache buffer manager
 * @param num_heaps  Number of separate caches/buckets indexed by bucket_index
 *                   for faster buffer matching (alternative to slower
 *                   "usage"-based matching).
 * @param usecs   Unused buffers may be released from the cache after this
 *                time
 * @param size_factor  Declare buffers that are size_factor times bigger than
 *                     the requested size as cache hits.
 * @param bypass_usage  Bitmask. If (requested usage & bypass_usage) != 0,
 *                      buffer allocation requests are rejected.
 * @param maximum_cache_size  Maximum size of all unused buffers the cache can
 *                            hold.
 * @param offsetof_pb_cache_entry  offsetof(driver_bo, pb_cache_entry)
 * @param destroy_buffer  Function that destroys a buffer for good.
 * @param can_reclaim     Whether a buffer can be reclaimed (e.g. is not busy)
 */
void
pb_cache_init(struct pb_cache *mgr, unsigned num_heaps,
              unsigned usecs, float size_factor,
              unsigned bypass_usage, uint64_t maximum_cache_size,
              unsigned offsetof_pb_cache_entry, void *winsys,
              void (*destroy_buffer)(void *winsys, struct pb_buffer_lean *buf),
              bool (*can_reclaim)(void *winsys, struct pb_buffer_lean *buf))
{
   unsigned i;

   mgr->buckets = CALLOC(num_heaps, sizeof(struct list_head));
   if (!mgr->buckets)
      return;

   for (i = 0; i < num_heaps; i++)
      list_inithead(&mgr->buckets[i]);

   (void) simple_mtx_init(&mgr->mutex, mtx_plain);
   mgr->winsys = winsys;
   mgr->cache_size = 0;
   mgr->max_cache_size = maximum_cache_size;
   mgr->num_heaps = num_heaps;
   mgr->msecs = usecs / 1000;
   mgr->msecs_base_time = os_time_get() / 1000;
   mgr->num_buffers = 0;
   mgr->bypass_usage = bypass_usage;
   mgr->size_factor = size_factor;
   mgr->offsetof_pb_cache_entry = offsetof_pb_cache_entry;
   mgr->destroy_buffer = destroy_buffer;
   mgr->can_reclaim = can_reclaim;
}

/**
 * Deinitialize the manager completely.
 */
void
pb_cache_deinit(struct pb_cache *mgr)
{
   pb_cache_release_all_buffers(mgr);
   simple_mtx_destroy(&mgr->mutex);
   FREE(mgr->buckets);
   mgr->buckets = NULL;
}
