/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * Improved cache implementation.
 *
 * Fixed size array with linear probing on collision and LRU eviction
 * on full.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#include "util/compiler.h"
#include "util/u_debug.h"

#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_cache.h"
#include "util/list.h"

struct util_cache_entry
{
   enum { EMPTY = 0, FILLED, DELETED } state;
   uint32_t hash;

   struct list_head list;

   void *key;
   void *value;

#ifdef DEBUG
   unsigned count;
#endif
};


struct util_cache
{
   /** Hash function */
   uint32_t (*hash)(const void *key);

   /** Compare two keys */
   int (*compare)(const void *key1, const void *key2);

   /** Destroy a (key, value) pair */
   void (*destroy)(void *key, void *value);

   /** Max entries in the cache */
   uint32_t size;

   /** Array [size] of entries */
   struct util_cache_entry *entries;

   /** Number of entries in the cache */
   unsigned count;

   /** Head of list, sorted from Least-recently used to Most-recently used */
   struct util_cache_entry lru;
};


static void
ensure_sanity(const struct util_cache *cache);

#define CACHE_DEFAULT_ALPHA 2

/**
 * Create a new cache with 'size' entries.  Also provide functions for
 * hashing keys, comparing keys and destroying (key,value) pairs.
 */
struct util_cache *
util_cache_create(uint32_t (*hash)(const void *key),
                  int (*compare)(const void *key1, const void *key2),
                  void (*destroy)(void *key, void *value),
                  uint32_t size)
{
   struct util_cache *cache;

   cache = CALLOC_STRUCT(util_cache);
   if (!cache)
      return NULL;

   cache->hash = hash;
   cache->compare = compare;
   cache->destroy = destroy;

   list_inithead(&cache->lru.list);

   size *= CACHE_DEFAULT_ALPHA;
   cache->size = size;

   cache->entries = CALLOC(size, sizeof(struct util_cache_entry));
   if (!cache->entries) {
      FREE(cache);
      return NULL;
   }

   ensure_sanity(cache);
   return cache;
}


/**
 * Try to find a cache entry, given the key and hash of the key.
 */
static struct util_cache_entry *
util_cache_entry_get(struct util_cache *cache,
                     uint32_t hash,
                     const void *key)
{
   struct util_cache_entry *first_unfilled = NULL;
   uint32_t index = hash % cache->size;
   uint32_t probe;

   /* Probe until we find either a matching FILLED entry or an EMPTY
    * slot (which has never been occupied).
    *
    * Deleted or non-matching slots are not indicative of completion
    * as a previous linear probe for the same key could have continued
    * past this point.
    */
   for (probe = 0; probe < cache->size; probe++) {
      uint32_t i = (index + probe) % cache->size;
      struct util_cache_entry *current = &cache->entries[i];

      if (current->state == FILLED) {
         if (current->hash == hash &&
             cache->compare(key, current->key) == 0)
            return current;
      }
      else {
         if (!first_unfilled)
            first_unfilled = current;

         if (current->state == EMPTY)
            return first_unfilled;
      }
   }

   return NULL;
}

static inline void
util_cache_entry_destroy(struct util_cache *cache,
                         struct util_cache_entry *entry)
{
   void *key = entry->key;
   void *value = entry->value;

   entry->key = NULL;
   entry->value = NULL;

   if (entry->state == FILLED) {
      list_del(&entry->list);
      cache->count--;

      if (cache->destroy)
         cache->destroy(key, value);

      entry->state = DELETED;
   }
}


/**
 * Insert an entry in the cache, given the key and value.
 */
void
util_cache_set(struct util_cache *cache,
               void *key,
               void *value)
{
   struct util_cache_entry *entry;
   uint32_t hash;

   assert(cache);
   if (!cache)
      return;
   hash = cache->hash(key);
   entry = util_cache_entry_get(cache, hash, key);
   if (!entry)
      entry = list_last_entry(&cache->lru.list, struct util_cache_entry, list);

   if (cache->count >= cache->size / CACHE_DEFAULT_ALPHA)
      util_cache_entry_destroy(cache, list_last_entry(&cache->lru.list, struct util_cache_entry, list));

   util_cache_entry_destroy(cache, entry);

#ifdef DEBUG
   ++entry->count;
#endif

   entry->key = key;
   entry->hash = hash;
   entry->value = value;
   entry->state = FILLED;
   list_add(&cache->lru.list, &entry->list);
   cache->count++;

   ensure_sanity(cache);
}


/**
 * Search the cache for an entry with the given key.  Return the corresponding
 * value or NULL if not found.
 */
void *
util_cache_get(struct util_cache *cache,
               const void *key)
{
   struct util_cache_entry *entry;
   uint32_t hash;

   assert(cache);
   if (!cache)
      return NULL;
   hash = cache->hash(key);
   entry = util_cache_entry_get(cache, hash, key);
   if (!entry)
      return NULL;

   if (entry->state == FILLED) {
      list_move_to(&cache->lru.list, &entry->list);
   }

   return entry->value;
}


/**
 * Remove all entries from the cache.  The 'destroy' function will be called
 * for each entry's (key, value).
 */
void
util_cache_clear(struct util_cache *cache)
{
   uint32_t i;

   assert(cache);
   if (!cache)
      return;

   for (i = 0; i < cache->size; ++i) {
      util_cache_entry_destroy(cache, &cache->entries[i]);
      cache->entries[i].state = EMPTY;
   }

   assert(cache->count == 0);
   assert(list_is_empty(&cache->lru.list));
   ensure_sanity(cache);
}


/**
 * Destroy the cache and all entries.
 */
void
util_cache_destroy(struct util_cache *cache)
{
   assert(cache);
   if (!cache)
      return;

#ifdef DEBUG
   if (cache->count >= 20*cache->size) {
      /* Normal approximation of the Poisson distribution */
      double mean = (double)cache->count/(double)cache->size;
      double stddev = sqrt(mean);
      unsigned i;
      for (i = 0; i < cache->size; ++i) {
         double z = fabs(cache->entries[i].count - mean)/stddev;
         /* This assert should not fail 99.9999998027% of the times, unless
          * the hash function is a poor one */
         assert(z <= 6.0);
      }
   }
#endif

   util_cache_clear(cache);

   FREE(cache->entries);
   FREE(cache);
}


/**
 * Remove the cache entry which matches the given key.
 */
void
util_cache_remove(struct util_cache *cache,
                  const void *key)
{
   struct util_cache_entry *entry;
   uint32_t hash;

   assert(cache);
   if (!cache)
      return;

   hash = cache->hash(key);

   entry = util_cache_entry_get(cache, hash, key);
   if (!entry)
      return;

   if (entry->state == FILLED)
      util_cache_entry_destroy(cache, entry);

   ensure_sanity(cache);
}


static void
ensure_sanity(const struct util_cache *cache)
{
#ifdef DEBUG
   unsigned i, cnt = 0;

   assert(cache);
   for (i = 0; i < cache->size; i++) {
      struct util_cache_entry *header = &cache->entries[i];

      assert(header);
      assert(header->state == FILLED ||
             header->state == EMPTY ||
             header->state == DELETED);
      if (header->state == FILLED) {
         cnt++;
         assert(header->hash == cache->hash(header->key));
      }
   }

   assert(cnt == cache->count);
   assert(cache->size >= cnt);

   if (cache->count == 0) {
      assert (list_is_empty(&cache->lru.list));
   }
   else {
      struct util_cache_entry *header =
         list_entry(&cache->lru, struct util_cache_entry, list);

      assert (header);
      assert (!list_is_empty(&cache->lru.list));

      for (i = 0; i < cache->count; i++)
         header = list_entry(&header, struct util_cache_entry, list);

      assert(header == &cache->lru);
   }
#endif

   (void)cache;
}
