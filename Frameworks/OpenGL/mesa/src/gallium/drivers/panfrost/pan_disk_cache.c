/*
 * Copyright (c) 2022 Amazon.com, Inc. or its affiliates.
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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "compiler/nir/nir.h"
#include "util/blob.h"
#include "util/build_id.h"
#include "util/disk_cache.h"
#include "util/mesa-sha1.h"

#include "pan_context.h"

static bool debug = false;

extern int midgard_debug;
extern int bifrost_debug;

/**
 * Compute a disk cache key for the given uncompiled shader and shader key.
 */
static void
panfrost_disk_cache_compute_key(
   struct disk_cache *cache,
   const struct panfrost_uncompiled_shader *uncompiled,
   const struct panfrost_shader_key *shader_key, cache_key cache_key)
{
   uint8_t data[sizeof(uncompiled->nir_sha1) + sizeof(*shader_key)];

   memcpy(data, uncompiled->nir_sha1, sizeof(uncompiled->nir_sha1));
   memcpy(data + sizeof(uncompiled->nir_sha1), shader_key, sizeof(*shader_key));

   disk_cache_compute_key(cache, data, sizeof(data), cache_key);
}

/**
 * Store the given compiled shader in the disk cache.
 *
 * This should only be called on newly compiled shaders.  No checking is
 * done to prevent repeated stores of the same shader.
 */
void
panfrost_disk_cache_store(struct disk_cache *cache,
                          const struct panfrost_uncompiled_shader *uncompiled,
                          const struct panfrost_shader_key *key,
                          const struct panfrost_shader_binary *binary)
{
#ifdef ENABLE_SHADER_CACHE
   if (!cache)
      return;

   cache_key cache_key;
   panfrost_disk_cache_compute_key(cache, uncompiled, key, cache_key);

   if (debug) {
      char sha1[41];
      _mesa_sha1_format(sha1, cache_key);
      fprintf(stderr, "[mesa disk cache] storing %s\n", sha1);
   }

   struct blob blob;
   blob_init(&blob);

   /* We write the following data to the cache blob:
    *
    * 1. Size of program binary
    * 2. Program binary
    * 3. Shader info
    * 4. System values
    */
   blob_write_uint32(&blob, binary->binary.size);
   blob_write_bytes(&blob, binary->binary.data, binary->binary.size);
   blob_write_bytes(&blob, &binary->info, sizeof(binary->info));
   blob_write_bytes(&blob, &binary->sysvals, sizeof(binary->sysvals));

   disk_cache_put(cache, cache_key, blob.data, blob.size, NULL);
   blob_finish(&blob);
#endif
}

/**
 * Search for a compiled shader in the disk cache.
 */
bool
panfrost_disk_cache_retrieve(struct disk_cache *cache,
                             const struct panfrost_uncompiled_shader *uncompiled,
                             const struct panfrost_shader_key *key,
                             struct panfrost_shader_binary *binary)
{
#ifdef ENABLE_SHADER_CACHE
   if (!cache)
      return false;

   cache_key cache_key;
   panfrost_disk_cache_compute_key(cache, uncompiled, key, cache_key);

   if (debug) {
      char sha1[41];
      _mesa_sha1_format(sha1, cache_key);
      fprintf(stderr, "[mesa disk cache] retrieving %s: ", sha1);
   }

   size_t size;
   void *buffer = disk_cache_get(cache, cache_key, &size);

   if (debug)
      fprintf(stderr, "%s\n", buffer ? "found" : "missing");

   if (!buffer)
      return false;

   struct blob_reader blob;
   blob_reader_init(&blob, buffer, size);

   util_dynarray_init(&binary->binary, NULL);

   uint32_t binary_size = blob_read_uint32(&blob);
   void *ptr = util_dynarray_resize_bytes(&binary->binary, binary_size, 1);

   blob_copy_bytes(&blob, ptr, binary_size);
   blob_copy_bytes(&blob, &binary->info, sizeof(binary->info));
   blob_copy_bytes(&blob, &binary->sysvals, sizeof(binary->sysvals));

   free(buffer);

   return true;
#else
   return false;
#endif
}

/**
 * Initialize the on-disk shader cache.
 */
void
panfrost_disk_cache_init(struct panfrost_screen *screen)
{
#ifdef ENABLE_SHADER_CACHE
   const char *renderer = screen->base.get_name(&screen->base);

   const struct build_id_note *note =
      build_id_find_nhdr_for_addr(panfrost_disk_cache_init);
   assert(note && build_id_length(note) == 20); /* sha1 */

   const uint8_t *id_sha1 = build_id_data(note);
   assert(id_sha1);

   char timestamp[41];
   _mesa_sha1_format(timestamp, id_sha1);

   /* Consider any flags affecting the compile when caching */
   uint64_t driver_flags = screen->dev.debug;
   driver_flags |= ((uint64_t)(midgard_debug | bifrost_debug) << 32);

   screen->disk_cache = disk_cache_create(renderer, timestamp, driver_flags);
#endif
}
