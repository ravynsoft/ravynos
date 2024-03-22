/*
 * Mesa 3-D graphics library
 *
 * Copyright 2003 VMware, Inc.
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 * Copyright (C) 2016 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "util/glheader.h"
#include "util/u_cpu_detect.h"
#include "main/context.h"
#include "main/varray.h"
#include "main/macros.h"
#include "main/sse_minmax.h"
#include "util/hash_table.h"
#include "util/u_memory.h"
#include "pipe/p_state.h"

struct minmax_cache_key {
   GLintptr offset;
   GLuint count;
   unsigned index_size;
};


struct minmax_cache_entry {
   struct minmax_cache_key key;
   GLuint min;
   GLuint max;
};


static uint32_t
vbo_minmax_cache_hash(const struct minmax_cache_key *key)
{
   return _mesa_hash_data(key, sizeof(*key));
}


static bool
vbo_minmax_cache_key_equal(const struct minmax_cache_key *a,
                           const struct minmax_cache_key *b)
{
   return (a->offset == b->offset) && (a->count == b->count) &&
          (a->index_size == b->index_size);
}


static void
vbo_minmax_cache_delete_entry(struct hash_entry *entry)
{
   free(entry->data);
}


static GLboolean
vbo_use_minmax_cache(struct gl_buffer_object *bufferObj)
{
   if (bufferObj->UsageHistory & (USAGE_TEXTURE_BUFFER |
                                  USAGE_ATOMIC_COUNTER_BUFFER |
                                  USAGE_SHADER_STORAGE_BUFFER |
                                  USAGE_TRANSFORM_FEEDBACK_BUFFER |
                                  USAGE_PIXEL_PACK_BUFFER |
                                  USAGE_DISABLE_MINMAX_CACHE))
      return GL_FALSE;

   if ((bufferObj->Mappings[MAP_USER].AccessFlags &
        (GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT)) ==
       (GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT))
      return GL_FALSE;

   return GL_TRUE;
}


void
vbo_delete_minmax_cache(struct gl_buffer_object *bufferObj)
{
   _mesa_hash_table_destroy(bufferObj->MinMaxCache, vbo_minmax_cache_delete_entry);
   bufferObj->MinMaxCache = NULL;
}


static GLboolean
vbo_get_minmax_cached(struct gl_buffer_object *bufferObj,
                      unsigned index_size, GLintptr offset, GLuint count,
                      GLuint *min_index, GLuint *max_index)
{
   GLboolean found = GL_FALSE;
   struct minmax_cache_key key;
   uint32_t hash;
   struct hash_entry *result;

   if (!bufferObj->MinMaxCache)
      return GL_FALSE;
   if (!vbo_use_minmax_cache(bufferObj))
      return GL_FALSE;

   simple_mtx_lock(&bufferObj->MinMaxCacheMutex);

   if (bufferObj->MinMaxCacheDirty) {
      /* Disable the cache permanently for this BO if the number of hits
       * is asymptotically less than the number of misses. This happens when
       * applications use the BO for streaming.
       *
       * However, some initial optimism allows applications that interleave
       * draw calls with glBufferSubData during warmup.
       */
      unsigned optimism = bufferObj->Size;
      if (bufferObj->MinMaxCacheMissIndices > optimism &&
          bufferObj->MinMaxCacheHitIndices < bufferObj->MinMaxCacheMissIndices - optimism) {
         bufferObj->UsageHistory |= USAGE_DISABLE_MINMAX_CACHE;
         vbo_delete_minmax_cache(bufferObj);
         goto out_disable;
      }

      _mesa_hash_table_clear(bufferObj->MinMaxCache, vbo_minmax_cache_delete_entry);
      bufferObj->MinMaxCacheDirty = false;
      goto out_invalidate;
   }

   key.index_size = index_size;
   key.offset = offset;
   key.count = count;
   hash = vbo_minmax_cache_hash(&key);
   result = _mesa_hash_table_search_pre_hashed(bufferObj->MinMaxCache, hash, &key);
   if (result) {
      struct minmax_cache_entry *entry = result->data;
      *min_index = entry->min;
      *max_index = entry->max;
      found = GL_TRUE;
   }

out_invalidate:
   if (found) {
      /* The hit counter saturates so that we don't accidently disable the
       * cache in a long-running program.
       */
      unsigned new_hit_count = bufferObj->MinMaxCacheHitIndices + count;

      if (new_hit_count >= bufferObj->MinMaxCacheHitIndices)
         bufferObj->MinMaxCacheHitIndices = new_hit_count;
      else
         bufferObj->MinMaxCacheHitIndices = ~(unsigned)0;
   } else {
      bufferObj->MinMaxCacheMissIndices += count;
   }

out_disable:
   simple_mtx_unlock(&bufferObj->MinMaxCacheMutex);
   return found;
}


static void
vbo_minmax_cache_store(struct gl_context *ctx,
                       struct gl_buffer_object *bufferObj,
                       unsigned index_size, GLintptr offset, GLuint count,
                       GLuint min, GLuint max)
{
   struct minmax_cache_entry *entry;
   struct hash_entry *table_entry;
   uint32_t hash;

   if (!vbo_use_minmax_cache(bufferObj))
      return;

   simple_mtx_lock(&bufferObj->MinMaxCacheMutex);

   if (!bufferObj->MinMaxCache) {
      bufferObj->MinMaxCache =
         _mesa_hash_table_create(NULL,
                                 (uint32_t (*)(const void *))vbo_minmax_cache_hash,
                                 (bool (*)(const void *, const void *))vbo_minmax_cache_key_equal);
      if (!bufferObj->MinMaxCache)
         goto out;
   }

   entry = MALLOC_STRUCT(minmax_cache_entry);
   if (!entry)
      goto out;

   entry->key.offset = offset;
   entry->key.count = count;
   entry->key.index_size = index_size;
   entry->min = min;
   entry->max = max;
   hash = vbo_minmax_cache_hash(&entry->key);

   table_entry = _mesa_hash_table_search_pre_hashed(bufferObj->MinMaxCache,
                                                    hash, &entry->key);
   if (table_entry) {
      /* It seems like this could happen when two contexts are rendering using
       * the same buffer object from multiple threads.
       */
      _mesa_debug(ctx, "duplicate entry in minmax cache\n");
      free(entry);
      goto out;
   }

   table_entry = _mesa_hash_table_insert_pre_hashed(bufferObj->MinMaxCache,
                                                    hash, &entry->key, entry);
   if (!table_entry)
      free(entry);

out:
   simple_mtx_unlock(&bufferObj->MinMaxCacheMutex);
}


void
vbo_get_minmax_index_mapped(unsigned count, unsigned index_size,
                            unsigned restartIndex, bool restart,
                            const void *indices,
                            unsigned *min_index, unsigned *max_index)
{
   switch (index_size) {
   case 4: {
      const GLuint *ui_indices = (const GLuint *)indices;
      GLuint max_ui = 0;
      GLuint min_ui = ~0U;
      if (restart) {
         for (unsigned i = 0; i < count; i++) {
            if (ui_indices[i] != restartIndex) {
               if (ui_indices[i] > max_ui) max_ui = ui_indices[i];
               if (ui_indices[i] < min_ui) min_ui = ui_indices[i];
            }
         }
      }
      else {
#if defined(USE_SSE41)
         if (util_get_cpu_caps()->has_sse4_1) {
            _mesa_uint_array_min_max(ui_indices, &min_ui, &max_ui, count);
         }
         else
#endif
            for (unsigned i = 0; i < count; i++) {
               if (ui_indices[i] > max_ui) max_ui = ui_indices[i];
               if (ui_indices[i] < min_ui) min_ui = ui_indices[i];
            }
      }
      *min_index = min_ui;
      *max_index = max_ui;
      break;
   }
   case 2: {
      const GLushort *us_indices = (const GLushort *)indices;
      GLuint max_us = 0;
      GLuint min_us = ~0U;
      if (restart) {
         for (unsigned i = 0; i < count; i++) {
            if (us_indices[i] != restartIndex) {
               if (us_indices[i] > max_us) max_us = us_indices[i];
               if (us_indices[i] < min_us) min_us = us_indices[i];
            }
         }
      }
      else {
         for (unsigned i = 0; i < count; i++) {
            if (us_indices[i] > max_us) max_us = us_indices[i];
            if (us_indices[i] < min_us) min_us = us_indices[i];
         }
      }
      *min_index = min_us;
      *max_index = max_us;
      break;
   }
   case 1: {
      const GLubyte *ub_indices = (const GLubyte *)indices;
      GLuint max_ub = 0;
      GLuint min_ub = ~0U;
      if (restart) {
         for (unsigned i = 0; i < count; i++) {
            if (ub_indices[i] != restartIndex) {
               if (ub_indices[i] > max_ub) max_ub = ub_indices[i];
               if (ub_indices[i] < min_ub) min_ub = ub_indices[i];
            }
         }
      }
      else {
         for (unsigned i = 0; i < count; i++) {
            if (ub_indices[i] > max_ub) max_ub = ub_indices[i];
            if (ub_indices[i] < min_ub) min_ub = ub_indices[i];
         }
      }
      *min_index = min_ub;
      *max_index = max_ub;
      break;
   }
   default:
      unreachable("not reached");
   }
}


/**
 * Compute min and max elements by scanning the index buffer for
 * glDraw[Range]Elements() calls.
 * If primitive restart is enabled, we need to ignore restart
 * indexes when computing min/max.
 */
void
vbo_get_minmax_index(struct gl_context *ctx, struct gl_buffer_object *obj,
                     const void *ptr, GLintptr offset, unsigned count,
                     unsigned index_size, bool primitive_restart,
                     unsigned restart_index, GLuint *min_index,
                     GLuint *max_index)
{
   const char *indices;

   if (!obj) {
      indices = (const char *)ptr + offset;
   } else {
      GLsizeiptr size = MIN2((GLsizeiptr)count * index_size, obj->Size);

      if (vbo_get_minmax_cached(obj, index_size, offset, count, min_index,
                                max_index))
         return;

      indices = _mesa_bufferobj_map_range(ctx, offset, size, GL_MAP_READ_BIT,
                                          obj, MAP_INTERNAL);
   }

   vbo_get_minmax_index_mapped(count, index_size, restart_index,
                               primitive_restart, indices,
                               min_index, max_index);

   if (obj) {
      vbo_minmax_cache_store(ctx, obj, index_size, offset, count, *min_index,
                             *max_index);
      _mesa_bufferobj_unmap(ctx, obj, MAP_INTERNAL);
   }
}

/**
 * Same as vbo_get_minmax_index, but using gallium draw structures.
 */
bool
vbo_get_minmax_indices_gallium(struct gl_context *ctx,
                               struct pipe_draw_info *info,
                               const struct pipe_draw_start_count_bias *draws,
                               unsigned num_draws)
{
   info->min_index = ~0;
   info->max_index = 0;

   struct gl_buffer_object *buf =
      info->has_user_indices ? NULL : ctx->Array.VAO->IndexBufferObj;

   for (unsigned i = 0; i < num_draws; i++) {
      struct pipe_draw_start_count_bias draw = draws[i];

      /* Do combination if possible to reduce map/unmap count */
      while ((i + 1 < num_draws) &&
             (draws[i].start + draws[i].count == draws[i+1].start)) {
         draw.count += draws[i+1].count;
         i++;
      }

      if (!draw.count)
         continue;

      unsigned tmp_min, tmp_max;
      vbo_get_minmax_index(ctx, buf,
                           info->index.user,
                           (GLintptr)draw.start * info->index_size,
                           draw.count, info->index_size,
                           info->primitive_restart, info->restart_index,
                           &tmp_min, &tmp_max);
      info->min_index = MIN2(info->min_index, tmp_min);
      info->max_index = MAX2(info->max_index, tmp_max);
   }

   return info->min_index <= info->max_index;
}
