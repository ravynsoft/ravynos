/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/crc32.h"

#include "svga_debug.h"
#include "svga_format.h"
#include "svga_winsys.h"
#include "svga_screen.h"
#include "svga_screen_cache.h"
#include "svga_context.h"
#include "svga_cmd.h"

#define SVGA_SURFACE_CACHE_ENABLED 1


/**
 * Return the size of the surface described by the key (in bytes).
 */
unsigned
svga_surface_size(const struct svga_host_surface_cache_key *key)
{
   unsigned bw, bh, bpb, total_size, i;

   assert(key->numMipLevels > 0);
   assert(key->numFaces > 0);
   assert(key->arraySize > 0);

   if (key->format == SVGA3D_BUFFER) {
      /* Special case: we don't want to count vertex/index buffers
       * against the cache size limit, so view them as zero-sized.
       */
      return 0;
   }

   svga_format_size(key->format, &bw, &bh, &bpb);

   total_size = 0;

   for (i = 0; i < key->numMipLevels; i++) {
      unsigned w = u_minify(key->size.width, i);
      unsigned h = u_minify(key->size.height, i);
      unsigned d = u_minify(key->size.depth, i);
      unsigned img_size = ((w + bw - 1) / bw) * ((h + bh - 1) / bh) * d * bpb;
      total_size += img_size;
   }

   total_size *= key->numFaces * key->arraySize * MAX2(1, key->sampleCount);

   return total_size;
}


/**
 * Compute the bucket for this key.
 */
static inline unsigned
svga_screen_cache_bucket(const struct svga_host_surface_cache_key *key)
{
   return util_hash_crc32(key, sizeof *key) % SVGA_HOST_SURFACE_CACHE_BUCKETS;
}


/**
 * Search the cache for a surface that matches the key.  If a match is
 * found, remove it from the cache and return the surface pointer.
 * Return NULL otherwise.
 */
static struct svga_winsys_surface *
svga_screen_cache_lookup(struct svga_screen *svgascreen,
                         const struct svga_host_surface_cache_key *key)
{
   struct svga_host_surface_cache *cache = &svgascreen->cache;
   struct svga_winsys_screen *sws = svgascreen->sws;
   struct svga_host_surface_cache_entry *entry;
   struct svga_winsys_surface *handle = NULL;
   struct list_head *curr, *next;
   unsigned bucket;
   unsigned tries = 0;

   assert(key->cachable);

   bucket = svga_screen_cache_bucket(key);

   mtx_lock(&cache->mutex);

   curr = cache->bucket[bucket].next;
   next = curr->next;
   while (curr != &cache->bucket[bucket]) {
      ++tries;

      entry = list_entry(curr, struct svga_host_surface_cache_entry, bucket_head);

      assert(entry->handle);

      /* If the key matches and the fence is signalled (the surface is no
       * longer needed) the lookup was successful.  We found a surface that
       * can be reused.
       * We unlink the surface from the cache entry and we add the entry to
       * the 'empty' list.
       */
      if (memcmp(&entry->key, key, sizeof *key) == 0 &&
          sws->fence_signalled(sws, entry->fence, 0) == 0) {
         unsigned surf_size;

         assert(sws->surface_is_flushed(sws, entry->handle));

         handle = entry->handle; /* Reference is transfered here. */
         entry->handle = NULL;

         /* Remove from hash table */
         list_del(&entry->bucket_head);

         /* remove from LRU list */
         list_del(&entry->head);

         /* Add the cache entry (but not the surface!) to the empty list */
         list_add(&entry->head, &cache->empty);

         /* update the cache size */
         surf_size = svga_surface_size(&entry->key);
         assert(surf_size <= cache->total_size);
         if (surf_size > cache->total_size)
            cache->total_size = 0; /* should never happen, but be safe */
         else
            cache->total_size -= surf_size;

         break;
      }

      curr = next;
      next = curr->next;
   }

   mtx_unlock(&cache->mutex);

   if (SVGA_DEBUG & DEBUG_DMA)
      debug_printf("%s: cache %s after %u tries (bucket %d)\n", __func__,
                   handle ? "hit" : "miss", tries, bucket);

   return handle;
}


/**
 * Free the least recently used entries in the surface cache until the
 * cache size is <= the target size OR there are no unused entries left
 * to discard.  We don't do any flushing to try to free up additional
 * surfaces.
 */
static void
svga_screen_cache_shrink(struct svga_screen *svgascreen,
                         unsigned target_size)
{
   struct svga_host_surface_cache *cache = &svgascreen->cache;
   struct svga_winsys_screen *sws = svgascreen->sws;
   struct svga_host_surface_cache_entry *entry = NULL, *next_entry;

   /* Walk over the list of unused buffers in reverse order: from oldest
    * to newest.
    */
   LIST_FOR_EACH_ENTRY_SAFE_REV(entry, next_entry, &cache->unused, head) {
      if (entry->key.format != SVGA3D_BUFFER) {
         /* we don't want to discard vertex/index buffers */

         cache->total_size -= svga_surface_size(&entry->key);

         assert(entry->handle);
         sws->surface_reference(sws, &entry->handle, NULL);

         list_del(&entry->bucket_head);
         list_del(&entry->head);
         list_add(&entry->head, &cache->empty);

         if (cache->total_size <= target_size) {
            /* all done */
            break;
         }
      }
   }
}


/**
 * Add a surface to the cache.  This is done when the driver deletes
 * the surface.  Note: transfers a handle reference.
 */
static void
svga_screen_cache_add(struct svga_screen *svgascreen,
                      const struct svga_host_surface_cache_key *key,
                      bool to_invalidate,
                      struct svga_winsys_surface **p_handle)
{
   struct svga_host_surface_cache *cache = &svgascreen->cache;
   struct svga_winsys_screen *sws = svgascreen->sws;
   struct svga_host_surface_cache_entry *entry = NULL;
   struct svga_winsys_surface *handle = *p_handle;
   unsigned surf_size;

   assert(key->cachable);

   if (!handle)
      return;

   surf_size = svga_surface_size(key);

   *p_handle = NULL;
   mtx_lock(&cache->mutex);

   if (surf_size >= SVGA_HOST_SURFACE_CACHE_BYTES) {
      /* this surface is too large to cache, just free it */
      sws->surface_reference(sws, &handle, NULL);
      mtx_unlock(&cache->mutex);
      return;
   }

   if (cache->total_size + surf_size > SVGA_HOST_SURFACE_CACHE_BYTES) {
      /* Adding this surface would exceed the cache size.
       * Try to discard least recently used entries until we hit the
       * new target cache size.
       */
      unsigned target_size = SVGA_HOST_SURFACE_CACHE_BYTES - surf_size;

      svga_screen_cache_shrink(svgascreen, target_size);

      if (cache->total_size > target_size) {
         /* we weren't able to shrink the cache as much as we wanted so
          * just discard this surface.
          */
         sws->surface_reference(sws, &handle, NULL);
         mtx_unlock(&cache->mutex);
         return;
      }
   }

   if (!list_is_empty(&cache->empty)) {
      /* An empty entry has no surface associated with it.
       * Use the first empty entry.
       */
      entry = list_entry(cache->empty.next,
                         struct svga_host_surface_cache_entry,
                         head);

      /* Remove from LRU list */
      list_del(&entry->head);
   }
   else if (!list_is_empty(&cache->unused)) {
      /* free the last used buffer and reuse its entry */
      entry = list_entry(cache->unused.prev,
                         struct svga_host_surface_cache_entry,
                         head);
      SVGA_DBG(DEBUG_CACHE|DEBUG_DMA,
               "unref sid %p (make space)\n", entry->handle);

      cache->total_size -= svga_surface_size(&entry->key);

      sws->surface_reference(sws, &entry->handle, NULL);

      /* Remove from hash table */
      list_del(&entry->bucket_head);

      /* Remove from LRU list */
      list_del(&entry->head);
   }

   if (entry) {
      assert(entry->handle == NULL);
      entry->handle = handle;
      memcpy(&entry->key, key, sizeof entry->key);

      SVGA_DBG(DEBUG_CACHE|DEBUG_DMA,
               "cache sid %p\n", entry->handle);

      /* If we don't have gb objects, we don't need to invalidate. */
      if (sws->have_gb_objects) {
         if (to_invalidate)
            list_add(&entry->head, &cache->validated);
         else
            list_add(&entry->head, &cache->invalidated);
      }
      else
         list_add(&entry->head, &cache->invalidated);

      cache->total_size += surf_size;
   }
   else {
      /* Couldn't cache the buffer -- this really shouldn't happen */
      SVGA_DBG(DEBUG_CACHE|DEBUG_DMA,
               "unref sid %p (couldn't find space)\n", handle);
      sws->surface_reference(sws, &handle, NULL);
   }

   mtx_unlock(&cache->mutex);
}


/* Maximum number of invalidate surface commands in a command buffer */
# define SVGA_MAX_SURFACE_TO_INVALIDATE 1000

/**
 * Called during the screen flush to move all buffers not in a validate list
 * into the unused list.
 */
void
svga_screen_cache_flush(struct svga_screen *svgascreen,
                        struct svga_context *svga,
                        struct pipe_fence_handle *fence)
{
   struct svga_host_surface_cache *cache = &svgascreen->cache;
   struct svga_winsys_screen *sws = svgascreen->sws;
   struct svga_host_surface_cache_entry *entry;
   struct list_head *curr, *next;
   unsigned bucket;

   mtx_lock(&cache->mutex);

   /* Loop over entries in the invalidated list */
   curr = cache->invalidated.next;
   next = curr->next;
   while (curr != &cache->invalidated) {
      entry = list_entry(curr, struct svga_host_surface_cache_entry, head);

      assert(entry->handle);

      if (sws->surface_is_flushed(sws, entry->handle)) {
         /* remove entry from the invalidated list */
         list_del(&entry->head);

         sws->fence_reference(sws, &entry->fence, fence);

         /* Add entry to the unused list */
         list_add(&entry->head, &cache->unused);

         /* Add entry to the hash table bucket */
         bucket = svga_screen_cache_bucket(&entry->key);
         list_add(&entry->bucket_head, &cache->bucket[bucket]);
      }

      curr = next;
      next = curr->next;
   }

   unsigned nsurf = 0;
   curr = cache->validated.next;
   next = curr->next;
   while (curr != &cache->validated) {
      entry = list_entry(curr, struct svga_host_surface_cache_entry, head);

      assert(entry->handle);
      assert(svga_have_gb_objects(svga));

      if (sws->surface_is_flushed(sws, entry->handle)) {
         /* remove entry from the validated list */
         list_del(&entry->head);

         /* It is now safe to invalidate the surface content.
          * It will be done using the current context.
          */
         if (SVGA_TRY(SVGA3D_InvalidateGBSurface(svga->swc, entry->handle))
             != PIPE_OK) {
            ASSERTED enum pipe_error ret;

            /* Even though surface invalidation here is done after the command
             * buffer is flushed, it is still possible that it will
             * fail because there might be just enough of this command that is
             * filling up the command buffer, so in this case we will call
             * the winsys flush directly to flush the buffer.
             * Note, we don't want to call svga_context_flush() here because
             * this function itself is called inside svga_context_flush().
             */
            svga_retry_enter(svga);
            svga->swc->flush(svga->swc, NULL);
            nsurf = 0;
            ret = SVGA3D_InvalidateGBSurface(svga->swc, entry->handle);
            svga_retry_exit(svga);
            assert(ret == PIPE_OK);
         }

         /* add the entry to the invalidated list */

         list_add(&entry->head, &cache->invalidated);
         nsurf++;
      }

      curr = next;
      next = curr->next;
   }

   mtx_unlock(&cache->mutex);

   /**
    * In some rare cases (when running ARK survival), we hit the max number
    * of surface relocations with invalidated surfaces during context flush.
    * So if the number of invalidated surface exceeds a certain limit (1000),
    * we'll do another winsys flush.
    */
   if (nsurf > SVGA_MAX_SURFACE_TO_INVALIDATE) {
      svga->swc->flush(svga->swc, NULL);
   }
}


/**
 * Free all the surfaces in the cache.
 * Called when destroying the svga screen object.
 */
void
svga_screen_cache_cleanup(struct svga_screen *svgascreen)
{
   struct svga_host_surface_cache *cache = &svgascreen->cache;
   struct svga_winsys_screen *sws = svgascreen->sws;
   unsigned i;

   for (i = 0; i < SVGA_HOST_SURFACE_CACHE_SIZE; ++i) {
      if (cache->entries[i].handle) {
	 SVGA_DBG(DEBUG_CACHE|DEBUG_DMA,
                  "unref sid %p (shutdown)\n", cache->entries[i].handle);
	 sws->surface_reference(sws, &cache->entries[i].handle, NULL);

         cache->total_size -= svga_surface_size(&cache->entries[i].key);
      }

      if (cache->entries[i].fence)
         sws->fence_reference(sws, &cache->entries[i].fence, NULL);
   }

   mtx_destroy(&cache->mutex);
}


enum pipe_error
svga_screen_cache_init(struct svga_screen *svgascreen)
{
   struct svga_host_surface_cache *cache = &svgascreen->cache;
   unsigned i;

   assert(cache->total_size == 0);

   (void) mtx_init(&cache->mutex, mtx_plain);

   for (i = 0; i < SVGA_HOST_SURFACE_CACHE_BUCKETS; ++i)
      list_inithead(&cache->bucket[i]);

   list_inithead(&cache->unused);

   list_inithead(&cache->validated);

   list_inithead(&cache->invalidated);

   list_inithead(&cache->empty);
   for (i = 0; i < SVGA_HOST_SURFACE_CACHE_SIZE; ++i)
      list_addtail(&cache->entries[i].head, &cache->empty);

   return PIPE_OK;
}


/**
 * Allocate a new host-side surface.  If the surface is marked as cachable,
 * first try re-using a surface in the cache of freed surfaces.  Otherwise,
 * allocate a new surface.
 * \param bind_flags  bitmask of PIPE_BIND_x flags
 * \param usage  one of PIPE_USAGE_x values
 * \param validated return True if the surface is a reused surface
 */
struct svga_winsys_surface *
svga_screen_surface_create(struct svga_screen *svgascreen,
                           unsigned bind_flags, enum pipe_resource_usage usage,
                           bool *validated,
                           struct svga_host_surface_cache_key *key)
{
   struct svga_winsys_screen *sws = svgascreen->sws;
   struct svga_winsys_surface *handle = NULL;
   bool cachable = SVGA_SURFACE_CACHE_ENABLED && key->cachable;

   SVGA_DBG(DEBUG_CACHE|DEBUG_DMA,
            "%s sz %dx%dx%d mips %d faces %d arraySize %d cachable %d\n",
            __func__,
            key->size.width,
            key->size.height,
            key->size.depth,
            key->numMipLevels,
            key->numFaces,
            key->arraySize,
            key->cachable);

   if (cachable) {
      /* Try to re-cycle a previously freed, cached surface */
      if (key->format == SVGA3D_BUFFER) {
         SVGA3dSurfaceAllFlags hint_flag;

         /* For buffers, round the buffer size up to the nearest power
          * of two to increase the probability of cache hits.  Keep
          * texture surface dimensions unchanged.
          */
         uint32_t size = 1;
         while (size < key->size.width)
            size <<= 1;
         key->size.width = size;

         /* Determine whether the buffer is static or dynamic.
          * This is a bit of a heuristic which can be tuned as needed.
          */
         if (usage == PIPE_USAGE_DEFAULT ||
             usage == PIPE_USAGE_IMMUTABLE) {
            hint_flag = SVGA3D_SURFACE_HINT_STATIC;
         }
         else if (bind_flags & PIPE_BIND_INDEX_BUFFER) {
            /* Index buffers don't change too often.  Mark them as static.
             */
            hint_flag = SVGA3D_SURFACE_HINT_STATIC;
         }
         else {
            /* Since we're reusing buffers we're effectively transforming all
             * of them into dynamic buffers.
             *
             * It would be nice to not cache long lived static buffers. But there
             * is no way to detect the long lived from short lived ones yet. A
             * good heuristic would be buffer size.
             */
            hint_flag = SVGA3D_SURFACE_HINT_DYNAMIC;
         }

         key->flags &= ~(SVGA3D_SURFACE_HINT_STATIC |
                         SVGA3D_SURFACE_HINT_DYNAMIC);
         key->flags |= hint_flag;
      }

      handle = svga_screen_cache_lookup(svgascreen, key);
      if (handle) {
         if (key->format == SVGA3D_BUFFER)
            SVGA_DBG(DEBUG_CACHE|DEBUG_DMA,
                     "reuse sid %p sz %d (buffer)\n", handle,
                     key->size.width);
         else
            SVGA_DBG(DEBUG_CACHE|DEBUG_DMA,
                     "reuse sid %p sz %dx%dx%d mips %d faces %d arraySize %d\n", handle,
                     key->size.width,
                     key->size.height,
                     key->size.depth,
                     key->numMipLevels,
                     key->numFaces,
                     key->arraySize);
         *validated = true;
      }
   }

   if (!handle) {
      /* Unable to recycle surface, allocate a new one */
      unsigned usage = 0;

      /* mark the surface as shareable if the surface is not
       * cachable or the RENDER_TARGET bind flag is set.
       */
      if (!key->cachable ||
          ((bind_flags & PIPE_BIND_RENDER_TARGET) != 0))
         usage |= SVGA_SURFACE_USAGE_SHARED;
      if (key->scanout)
         usage |= SVGA_SURFACE_USAGE_SCANOUT;
      if (key->coherent)
         usage |= SVGA_SURFACE_USAGE_COHERENT;

      handle = sws->surface_create(sws,
                                   key->flags,
                                   key->format,
                                   usage,
                                   key->size,
                                   key->numFaces * key->arraySize,
                                   key->numMipLevels,
                                   key->sampleCount);
      if (handle)
         SVGA_DBG(DEBUG_CACHE|DEBUG_DMA,
                  "  CREATE sid %p sz %dx%dx%d\n",
                  handle,
                  key->size.width,
                  key->size.height,
                  key->size.depth);

      *validated = false;
   }

   return handle;
}


/**
 * Release a surface.  We don't actually free the surface- we put
 * it into the cache of freed surfaces (if it's cachable).
 */
void
svga_screen_surface_destroy(struct svga_screen *svgascreen,
                            const struct svga_host_surface_cache_key *key,
                            bool to_invalidate,
                            struct svga_winsys_surface **p_handle)
{
   struct svga_winsys_screen *sws = svgascreen->sws;

   /* We only set the cachable flag for surfaces of which we are the
    * exclusive owner.  So just hold onto our existing reference in
    * that case.
    */
   if (SVGA_SURFACE_CACHE_ENABLED && key->cachable) {
      svga_screen_cache_add(svgascreen, key, to_invalidate, p_handle);
   }
   else {
      SVGA_DBG(DEBUG_DMA,
               "unref sid %p (uncachable)\n", *p_handle);
      sws->surface_reference(sws, p_handle, NULL);
   }
}


/**
 * Print/dump the contents of the screen cache.  For debugging.
 */
void
svga_screen_cache_dump(const struct svga_screen *svgascreen)
{
   const struct svga_host_surface_cache *cache = &svgascreen->cache;
   unsigned bucket;
   unsigned count = 0;

   debug_printf("svga3d surface cache:\n");
   for (bucket = 0; bucket < SVGA_HOST_SURFACE_CACHE_BUCKETS; bucket++) {
      struct list_head *curr;
      curr = cache->bucket[bucket].next;
      while (curr && curr != &cache->bucket[bucket]) {
         struct svga_host_surface_cache_entry *entry =
            list_entry(curr, struct svga_host_surface_cache_entry,bucket_head);
         if (entry->key.format == SVGA3D_BUFFER) {
            debug_printf("  %p: buffer %u bytes\n",
                         entry->handle,
                         entry->key.size.width);
         }
         else {
            debug_printf("  %p: %u x %u x %u format %u\n",
                         entry->handle,
                         entry->key.size.width,
                         entry->key.size.height,
                         entry->key.size.depth,
                         entry->key.format);
         }
         curr = curr->next;
         count++;
      }
   }

   debug_printf("%u surfaces, %u bytes\n", count, cache->total_size);
}
