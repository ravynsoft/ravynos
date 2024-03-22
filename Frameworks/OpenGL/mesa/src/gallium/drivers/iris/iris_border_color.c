/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iris_border_color.c
 *
 * Each SAMPLER_STATE points to a SAMPLER_BORDER_COLOR_STATE entry,
 * describing the color to return when sampling outside the texture
 * when using CLAMP_TO_BORDER wrap modes.
 *
 * These must be stored relative to Dynamic State Base Address.
 * Unfortunately, the hardware designers only gave us a 24-bit pointer
 * rather than an actual graphics address, so it must be stored in the
 * bottom 16MB of that memory zone.  This means we can't simply use
 * u_upload_mgr like we do for most state.
 *
 * To work around this, we maintain a single "border color pool" BO
 * which we pin at the base of IRIS_MEMZONE_DYNAMIC.  Since most border
 * colors are the same (typically black or white), we maintain a hash
 * table of known colors, and reuse the same entries.  This avoids
 * wasting a lot of space in the pool.
 *
 * If it ever does fill up, we simply return the black border. We
 * can't simply flush since the BO is shared by every context. If we
 * ever need we may choose to have multiple BOs, refcount them and
 * then recycle when unused.
 */

#include <stdlib.h>
#include "util/u_math.h"
#include "iris_binder.h"
#include "iris_bufmgr.h"
#include "iris_context.h"

#define BC_ALIGNMENT 64
#define BC_BLACK 64

static bool
color_equals(const void *a, const void *b)
{
   return memcmp(a, b, sizeof(union pipe_color_union)) == 0;
}

static uint32_t
color_hash(const void *key)
{
   return _mesa_hash_data(key, sizeof(union pipe_color_union));
}

void
iris_init_border_color_pool(struct iris_bufmgr *bufmgr,
                            struct iris_border_color_pool *pool)
{
   simple_mtx_init(&pool->lock, mtx_plain);

   pool->ht = _mesa_hash_table_create(NULL, color_hash, color_equals);

   pool->bo = iris_bo_alloc(bufmgr, "border colors",
                            IRIS_BORDER_COLOR_POOL_SIZE, 64,
                            IRIS_MEMZONE_BORDER_COLOR_POOL, BO_ALLOC_PLAIN);
   pool->map = iris_bo_map(NULL, pool->bo, MAP_WRITE);

   /* Don't make 0 a valid offset - tools treat that as a NULL pointer. */
   pool->insert_point = BC_ALIGNMENT;

   union pipe_color_union black = {.f = { 0.0, 0.0, 0.0, 1.0 }};
   ASSERTED uint32_t black_offset = iris_upload_border_color(pool, &black);
   assert(black_offset == BC_BLACK);
}

void
iris_destroy_border_color_pool(struct iris_border_color_pool *pool)
{
   iris_bo_unreference(pool->bo);
   ralloc_free(pool->ht);
   simple_mtx_destroy(&pool->lock);
}

/**
 * Upload a border color (or use a cached version).
 *
 * Returns the offset into the border color pool BO.  Note that you must
 * reserve space ahead of time by calling iris_border_color_pool_reserve().
 */
uint32_t
iris_upload_border_color(struct iris_border_color_pool *pool,
                         union pipe_color_union *color)
{
   uint32_t offset;
   uint32_t hash = color_hash(color);

   simple_mtx_lock(&pool->lock);

   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(pool->ht, hash, color);
   if (entry) {
      offset = (uintptr_t) entry->data;
      goto out;
   }

   if (pool->insert_point + BC_ALIGNMENT > IRIS_BORDER_COLOR_POOL_SIZE) {
      static bool warned = false;
      if (!warned) {
         fprintf(stderr, "Border color pool is full. Using black instead.\n");
         warned = true;
      }
      offset = BC_BLACK;
      goto out;
   }

   offset = pool->insert_point;
   memcpy(pool->map + offset, color, sizeof(*color));
   pool->insert_point += BC_ALIGNMENT;

   _mesa_hash_table_insert_pre_hashed(pool->ht, hash, pool->map + offset,
                                      (void *) (uintptr_t) offset);
out:
   simple_mtx_unlock(&pool->lock);
   return offset;
}

