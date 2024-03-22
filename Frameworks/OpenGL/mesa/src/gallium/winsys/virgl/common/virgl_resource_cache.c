/*
 * Copyright 2019 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "virgl_resource_cache.h"
#include "util/os_time.h"

/* Checks whether the resource represented by a cache entry is able to hold
 * data of the specified size, bind and format.
 */
static bool
virgl_resource_cache_entry_is_compatible(struct virgl_resource_cache_entry *entry, struct virgl_resource_params params)
{
   if (entry->params.target == PIPE_BUFFER) {
         return (entry->params.bind == params.bind &&
                 entry->params.format == params.format &&
                 entry->params.size >= params.size &&
                 entry->params.flags == params.flags &&
                 /* We don't want to waste space, so don't reuse resource storage to
                 * hold much smaller (< 50%) sizes.
                 */
                 entry->params.size <= params.size * 2 &&
                 entry->params.width >= params.width &&
                 entry->params.target == params.target);
   } else {
      return memcmp(&entry->params, &params, sizeof(params)) == 0;
   }
}

static void
virgl_resource_cache_entry_release(struct virgl_resource_cache *cache,
                                   struct virgl_resource_cache_entry *entry)
{
      list_del(&entry->head);
      cache->entry_release_func(entry, cache->user_data);
}

static void
virgl_resource_cache_destroy_expired(struct virgl_resource_cache *cache, int64_t now)
{
   list_for_each_entry_safe(struct virgl_resource_cache_entry,
                            entry, &cache->resources, head) {
      /* Entries are in non-decreasing timeout order, so we can stop
       * at the first entry which hasn't expired.
       */
      if (!os_time_timeout(entry->timeout_start, entry->timeout_end, now))
         break;
      virgl_resource_cache_entry_release(cache, entry);
   }
}

void
virgl_resource_cache_init(struct virgl_resource_cache *cache,
                          unsigned timeout_usecs,
                          virgl_resource_cache_entry_is_busy_func is_busy_func,
                          virgl_resource_cache_entry_release_func destroy_func,
                          void *user_data)
{
   list_inithead(&cache->resources);
   cache->timeout_usecs = timeout_usecs;
   cache->entry_is_busy_func = is_busy_func;
   cache->entry_release_func = destroy_func;
   cache->user_data = user_data;
}

void
virgl_resource_cache_add(struct virgl_resource_cache *cache,
                         struct virgl_resource_cache_entry *entry)
{
   const int64_t now = os_time_get();

   /* Entry should not already be in the cache. */
   assert(entry->head.next == NULL);
   assert(entry->head.prev == NULL);

   virgl_resource_cache_destroy_expired(cache, now);

   entry->timeout_start = now;
   entry->timeout_end = entry->timeout_start + cache->timeout_usecs;
   list_addtail(&entry->head, &cache->resources);
}

struct virgl_resource_cache_entry *
virgl_resource_cache_remove_compatible(struct virgl_resource_cache *cache,
                                       struct virgl_resource_params params)
{
   const int64_t now = os_time_get();
   struct virgl_resource_cache_entry *compat_entry = NULL;
   bool check_expired = true;

   /* Iterate through the cache to find a compatible resource, while also
    * destroying any expired resources we come across.
    */
   list_for_each_entry_safe(struct virgl_resource_cache_entry,
                            entry, &cache->resources, head) {
      const bool compatible =
         virgl_resource_cache_entry_is_compatible(entry, params);

      if (compatible) {
         if (!cache->entry_is_busy_func(entry, cache->user_data))
            compat_entry = entry;

         /* We either have found a compatible resource, in which case we are
          * done, or the resource is busy, which means resources later in
          * the cache list will also be busy, so there is no point in
          * searching further.
          */
         break;
      }

      /* If we aren't using this resource, check to see if it has expired.
       * Once we have found the first non-expired resource, we can stop checking
       * since the cache holds resources in non-decreasing timeout order.
       */
      if (check_expired) {
         if (os_time_timeout(entry->timeout_start, entry->timeout_end, now))
            virgl_resource_cache_entry_release(cache, entry);
         else
            check_expired = false;
      }
   }

   if (compat_entry)
      list_del(&compat_entry->head);

   return compat_entry;
}

void
virgl_resource_cache_flush(struct virgl_resource_cache *cache)
{
   list_for_each_entry_safe(struct virgl_resource_cache_entry,
                            entry, &cache->resources, head) {
      virgl_resource_cache_entry_release(cache, entry);
   }
}
