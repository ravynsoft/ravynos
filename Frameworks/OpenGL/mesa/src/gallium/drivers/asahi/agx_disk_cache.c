/*
 * Copyright 2023 Rose Hudson
 * Copyright 2022 Amazon.com, Inc. or its affiliates.
 * Copyright 2018 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>

#include "asahi/compiler/agx_debug.h"
#include "compiler/shader_enums.h"
#include "util/blob.h"
#include "util/build_id.h"
#include "util/disk_cache.h"
#include "util/mesa-sha1.h"
#include "agx_bo.h"
#include "agx_disk_cache.h"
#include "agx_state.h"

/* Flags that are allowed and do not disable the disk cache */
#define ALLOWED_FLAGS (AGX_DBG_NO16)

/**
 * Compute a disk cache key for the given uncompiled shader and shader key.
 */
static void
agx_disk_cache_compute_key(struct disk_cache *cache,
                           const struct agx_uncompiled_shader *uncompiled,
                           const union asahi_shader_key *shader_key,
                           cache_key cache_key)
{
   uint8_t data[sizeof(uncompiled->nir_sha1) + sizeof(*shader_key)];
   int hash_size = sizeof(uncompiled->nir_sha1);
   int key_size;
   if (uncompiled->type == PIPE_SHADER_VERTEX)
      key_size = sizeof(shader_key->vs);
   else if (uncompiled->type == PIPE_SHADER_GEOMETRY)
      key_size = sizeof(shader_key->gs);
   else if (uncompiled->type == PIPE_SHADER_FRAGMENT)
      key_size = sizeof(shader_key->fs);
   else if (uncompiled->type == PIPE_SHADER_COMPUTE)
      key_size = 0;
   else
      unreachable("Unsupported shader stage");

   memcpy(data, uncompiled->nir_sha1, hash_size);

   if (key_size)
      memcpy(data + hash_size, shader_key, key_size);

   disk_cache_compute_key(cache, data, hash_size + key_size, cache_key);
}

/**
 * Store the given compiled shader in the disk cache.
 *
 * This should only be called on newly compiled shaders.  No checking is
 * done to prevent repeated stores of the same shader.
 */
void
agx_disk_cache_store(struct disk_cache *cache,
                     const struct agx_uncompiled_shader *uncompiled,
                     const union asahi_shader_key *key,
                     const struct agx_compiled_shader *binary)
{
#ifdef ENABLE_SHADER_CACHE
   if (!cache)
      return;

   /* TODO: Support caching GS */
   if (uncompiled->type == PIPE_SHADER_GEOMETRY)
      return;

   assert(binary->bo->ptr.cpu != NULL && "shaders must be CPU mapped");

   cache_key cache_key;
   agx_disk_cache_compute_key(cache, uncompiled, key, cache_key);

   struct blob blob;
   blob_init(&blob);

   uint32_t shader_size = binary->bo->size;
   blob_write_uint32(&blob, shader_size);
   blob_write_bytes(&blob, binary->bo->ptr.cpu, shader_size);
   blob_write_bytes(&blob, &binary->info, sizeof(binary->info));
   blob_write_uint32(&blob, binary->push_range_count);
   blob_write_bytes(&blob, binary->push,
                    sizeof(binary->push[0]) * binary->push_range_count);

   disk_cache_put(cache, cache_key, blob.data, blob.size, NULL);
   blob_finish(&blob);
#endif
}

/**
 * Search for a compiled shader in the disk cache.
 */
struct agx_compiled_shader *
agx_disk_cache_retrieve(struct agx_screen *screen,
                        const struct agx_uncompiled_shader *uncompiled,
                        const union asahi_shader_key *key)
{
#ifdef ENABLE_SHADER_CACHE
   struct disk_cache *cache = screen->disk_cache;
   if (!cache)
      return NULL;

   /* TODO: Support caching GS */
   if (uncompiled->type == PIPE_SHADER_GEOMETRY)
      return NULL;

   cache_key cache_key;
   agx_disk_cache_compute_key(cache, uncompiled, key, cache_key);

   size_t size;
   void *buffer = disk_cache_get(cache, cache_key, &size);
   if (!buffer)
      return NULL;

   struct agx_compiled_shader *binary = CALLOC_STRUCT(agx_compiled_shader);

   struct blob_reader blob;
   blob_reader_init(&blob, buffer, size);

   uint32_t binary_size = blob_read_uint32(&blob);
   binary->bo = agx_bo_create(&screen->dev, binary_size,
                              AGX_BO_EXEC | AGX_BO_LOW_VA, "Executable");
   blob_copy_bytes(&blob, binary->bo->ptr.cpu, binary_size);

   blob_copy_bytes(&blob, &binary->info, sizeof(binary->info));
   binary->push_range_count = blob_read_uint32(&blob);
   blob_copy_bytes(&blob, binary->push,
                   sizeof(binary->push[0]) * binary->push_range_count);

   free(buffer);
   return binary;
#else
   return NULL;
#endif
}

/**
 * Initialise the on-disk shader cache.
 */
void
agx_disk_cache_init(struct agx_screen *screen)
{
#ifdef ENABLE_SHADER_CACHE
   if (agx_get_compiler_debug() || (screen->dev.debug & ~ALLOWED_FLAGS))
      return;

   const char *renderer = screen->pscreen.get_name(&screen->pscreen);

   const struct build_id_note *note =
      build_id_find_nhdr_for_addr(agx_disk_cache_init);
   assert(note && build_id_length(note) == 20);

   const uint8_t *id_sha1 = build_id_data(note);
   assert(id_sha1);

   char timestamp[41];
   _mesa_sha1_format(timestamp, id_sha1);

   uint64_t driver_flags = screen->dev.debug;
   screen->disk_cache = disk_cache_create(renderer, timestamp, driver_flags);
#endif
}
