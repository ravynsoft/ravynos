/*
 * Copyright © 2020 Google, Inc.
 * Copyright (c) 2020 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "etnaviv_debug.h"
#include "etnaviv_disk_cache.h"
#include "nir_serialize.h"

#define debug 0

void
etna_disk_cache_init(struct etna_compiler *compiler, const char *renderer)
{
   if (DBG_ENABLED(ETNA_DBG_NOCACHE))
      return;

   const struct build_id_note *note =
         build_id_find_nhdr_for_addr(etna_disk_cache_init);
   assert(note && build_id_length(note) == 20); /* sha1 */

   const uint8_t *id_sha1 = build_id_data(note);
   assert(id_sha1);

   char timestamp[41];
   _mesa_sha1_format(timestamp, id_sha1);

   compiler->disk_cache = disk_cache_create(renderer, timestamp, etna_mesa_debug);
}

void
etna_disk_cache_init_shader_key(struct etna_compiler *compiler, struct etna_shader *shader)
{
   if (!compiler->disk_cache)
      return;

   struct mesa_sha1 ctx;

   _mesa_sha1_init(&ctx);

   /* Serialize the NIR to a binary blob that we can hash for the disk
    * cache.  Drop unnecessary information (like variable names)
    * so the serialized NIR is smaller, and also to let us detect more
    * isomorphic shaders when hashing, increasing cache hits.
    */
   struct blob blob;

   blob_init(&blob);
   nir_serialize(&blob, shader->nir, true);
   _mesa_sha1_update(&ctx, blob.data, blob.size);
   blob_finish(&blob);

   _mesa_sha1_final(&ctx, shader->cache_key);
}

static void
compute_variant_key(struct etna_compiler *compiler, struct etna_shader_variant *v,
                    cache_key cache_key)
{
   struct blob blob;

   blob_init(&blob);

   blob_write_bytes(&blob, &v->shader->cache_key, sizeof(v->shader->cache_key));
   blob_write_bytes(&blob, &v->key, sizeof(v->key));

   disk_cache_compute_key(compiler->disk_cache, blob.data, blob.size, cache_key);

   blob_finish(&blob);
}

static void
retrieve_variant(struct blob_reader *blob, struct etna_shader_variant *v)
{
   blob_copy_bytes(blob, VARIANT_CACHE_PTR(v), VARIANT_CACHE_SIZE);

   v->code = malloc(4 * v->code_size);
   blob_copy_bytes(blob, v->code, 4 * v->code_size);

   blob_copy_bytes(blob, &v->uniforms.count, sizeof(v->uniforms.count));
   v->uniforms.contents = malloc(v->uniforms.count * sizeof(*v->uniforms.contents));
   v->uniforms.data = malloc(v->uniforms.count * sizeof(*v->uniforms.data));

   blob_copy_bytes(blob, v->uniforms.contents, v->uniforms.count * sizeof(*v->uniforms.contents));
   blob_copy_bytes(blob, v->uniforms.data, v->uniforms.count * sizeof(*v->uniforms.data));
}

static void
store_variant(struct blob *blob, const struct etna_shader_variant *v)
{
   const uint32_t imm_count = v->uniforms.count;

   blob_write_bytes(blob, VARIANT_CACHE_PTR(v), VARIANT_CACHE_SIZE);
   blob_write_bytes(blob, v->code, 4 * v->code_size);

   blob_write_bytes(blob, &v->uniforms.count, sizeof(v->uniforms.count));
   blob_write_bytes(blob, v->uniforms.contents, imm_count * sizeof(*v->uniforms.contents));
   blob_write_bytes(blob, v->uniforms.data, imm_count * sizeof(*v->uniforms.data));
}

bool
etna_disk_cache_retrieve(struct etna_compiler *compiler, struct etna_shader_variant *v)
{
   if (!compiler->disk_cache)
      return false;

   cache_key cache_key;

   compute_variant_key(compiler, v, cache_key);

   if (debug) {
      char sha1[41];

      _mesa_sha1_format(sha1, cache_key);
      fprintf(stderr, "[mesa disk cache] retrieving variant %s: ", sha1);
   }

   size_t size;
   void *buffer = disk_cache_get(compiler->disk_cache, cache_key, &size);

   if (debug)
      fprintf(stderr, "%s\n", buffer ? "found" : "missing");

   if (!buffer)
      return false;

   struct blob_reader blob;
   blob_reader_init(&blob, buffer, size);

   retrieve_variant(&blob, v);

   free(buffer);

   return true;
}

void
etna_disk_cache_store(struct etna_compiler *compiler, struct etna_shader_variant *v)
{
   if (!compiler->disk_cache)
      return;

   cache_key cache_key;

   compute_variant_key(compiler, v, cache_key);

   if (debug) {
      char sha1[41];

      _mesa_sha1_format(sha1, cache_key);
      fprintf(stderr, "[mesa disk cache] storing variant %s\n", sha1);
   }

   struct blob blob;
   blob_init(&blob);

   store_variant(&blob, v);

   disk_cache_put(compiler->disk_cache, cache_key, blob.data, blob.size, NULL);
   blob_finish(&blob);
}
