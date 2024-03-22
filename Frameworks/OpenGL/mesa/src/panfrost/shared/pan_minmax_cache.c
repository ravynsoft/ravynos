/*
 * Copyright (c) 2020 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors (Collabora):
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

/* Index buffer min/max cache. We need to calculate the min/max for arbitrary
 * slices (start, start + count) of the index buffer at drawtime. As this can
 * be quite expensive, we cache. Conceptually, we just use a hash table mapping
 * the key (start, count) to the value (min, max). In practice, mesa's hash
 * table implementation is higher overhead than we would like and makes
 * handling memory usage a little complicated. So we use this data structure
 * instead. Searching is O(n) to the size, but the size is capped at the
 * PANFROST_MINMAX_SIZE constant (so this is a tradeoff between cache hit/miss
 * ratio and cache search speed). Note that keys are adjacent so we get cache
 * line alignment benefits. Insertion is O(1) and in-order until the cache
 * fills up, after that it evicts the oldest cached value in a ring facilitated
 * by index.
 */

#include "pan_minmax_cache.h"

bool
panfrost_minmax_cache_get(struct panfrost_minmax_cache *cache, unsigned start,
                          unsigned count, unsigned *min_index,
                          unsigned *max_index)
{
   uint64_t ht_key = (((uint64_t)count) << 32) | start;
   bool found = false;

   if (!cache)
      return false;

   for (unsigned i = 0; i < cache->size; ++i) {
      if (cache->keys[i] == ht_key) {
         uint64_t hit = cache->values[i];

         *min_index = hit & 0xffffffff;
         *max_index = hit >> 32;
         found = true;
         break;
      }
   }

   return found;
}

void
panfrost_minmax_cache_add(struct panfrost_minmax_cache *cache, unsigned start,
                          unsigned count, unsigned min_index,
                          unsigned max_index)
{
   uint64_t ht_key = (((uint64_t)count) << 32) | start;
   uint64_t value = min_index | (((uint64_t)max_index) << 32);
   unsigned index = 0;

   if (!cache)
      return;

   if (cache->size == PANFROST_MINMAX_SIZE) {
      index = cache->index++;
      cache->index = cache->index % PANFROST_MINMAX_SIZE;
   } else {
      index = cache->size++;
   }

   cache->keys[index] = ht_key;
   cache->values[index] = value;
}

/* If we've been caching min/max indices and we update the index
 * buffer, that may invalidate the min/max. Check what's been cached vs
 * what we've written, and throw out invalid entries. */

void
panfrost_minmax_cache_invalidate(struct panfrost_minmax_cache *cache,
                                 struct pipe_transfer *transfer)
{
   /* Ensure there is a cache to invalidate and a write */
   if (!cache)
      return;

   if (!(transfer->usage & PIPE_MAP_WRITE))
      return;

   unsigned valid_count = 0;

   for (unsigned i = 0; i < cache->size; ++i) {
      uint64_t key = cache->keys[i];

      uint32_t start = key & 0xffffffff;
      uint32_t count = key >> 32;

      /* 1D range intersection */
      bool invalid = MAX2(transfer->box.x, start) <
                     MIN2(transfer->box.x + transfer->box.width, start + count);
      if (!invalid) {
         cache->keys[valid_count] = key;
         cache->values[valid_count] = cache->values[i];
         valid_count++;
      }
   }

   cache->size = valid_count;
   cache->index = 0;
}
