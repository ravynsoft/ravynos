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

#ifndef VIRGL_RESOURCE_CACHE_H
#define VIRGL_RESOURCE_CACHE_H

#include <stdint.h>

#include "util/list.h"
#include "gallium/include/pipe/p_defines.h"

struct virgl_resource_params {
   uint32_t size;
   uint32_t bind;
   uint32_t format;
   uint32_t flags;
   uint32_t nr_samples;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t array_size;
   uint32_t last_level;
   enum pipe_texture_target target;
};

struct virgl_resource_cache_entry {
   struct list_head head;
   int64_t timeout_start;
   int64_t timeout_end;
   struct virgl_resource_params params;
};

/* Pointer to a function that returns whether the resource represented by
 * the specified cache entry is busy.
 */
typedef bool (*virgl_resource_cache_entry_is_busy_func) (
   struct virgl_resource_cache_entry *entry, void *user_data);

/* Pointer to a function that destroys the resource represented by
 * the specified cache entry.
 */
typedef void (*virgl_resource_cache_entry_release_func) (
   struct virgl_resource_cache_entry *entry, void *user_data);

struct virgl_resource_cache {
   struct list_head resources;
   unsigned timeout_usecs;
   virgl_resource_cache_entry_is_busy_func entry_is_busy_func;
   virgl_resource_cache_entry_release_func entry_release_func;
   void *user_data;
};

void
virgl_resource_cache_init(struct virgl_resource_cache *cache,
                          unsigned timeout_usecs,
                          virgl_resource_cache_entry_is_busy_func is_busy_func,
                          virgl_resource_cache_entry_release_func destroy_func,
                          void *user_data);

/** Adds a resource to the cache.
 *
 *  Adding a resource that's already present in the cache leads to undefined
 *  behavior.
 */
void
virgl_resource_cache_add(struct virgl_resource_cache *cache,
                         struct virgl_resource_cache_entry *entry);

/** Finds and removes a cached resource compatible with size, bind and format.
 *
 *  Returns a pointer to the cache entry of the compatible resource, or NULL if
 *  no such resource was found.
 */
struct virgl_resource_cache_entry *
virgl_resource_cache_remove_compatible(struct virgl_resource_cache *cache,
                                       struct virgl_resource_params params);

/** Empties the resource cache. */
void
virgl_resource_cache_flush(struct virgl_resource_cache *cache);

static inline void
virgl_resource_cache_entry_init(struct virgl_resource_cache_entry *entry,
                                struct virgl_resource_params params)
{
   entry->params = params;
}

#endif
