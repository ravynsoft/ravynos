/*
 * Copyright 2019 Advanced Micro Devices, Inc.
 * All Rights Reserved.
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

#include "util/u_live_shader_cache.h"

#include "util/u_inlines.h"
#include "tgsi/tgsi_from_mesa.h"
#include "tgsi/tgsi_parse.h"

#include "compiler/nir/nir_serialize.h"

#include "util/blob.h"
#include "util/hash_table.h"
#include "util/mesa-sha1.h"

static uint32_t key_hash(const void *key)
{
   /* Take the first dword of SHA1. */
   return *(uint32_t*)key;
}

static bool key_equals(const void *a, const void *b)
{
   /* Compare SHA1s. */
   return memcmp(a, b, 20) == 0;
}

void
util_live_shader_cache_init(struct util_live_shader_cache *cache,
                            void *(*create_shader)(struct pipe_context *,
                                                   const struct pipe_shader_state *state),
                            void (*destroy_shader)(struct pipe_context *, void *))
{
   simple_mtx_init(&cache->lock, mtx_plain);
   cache->hashtable = _mesa_hash_table_create(NULL, key_hash, key_equals);
   cache->create_shader = create_shader;
   cache->destroy_shader = destroy_shader;
}

void
util_live_shader_cache_deinit(struct util_live_shader_cache *cache)
{
   if (cache->hashtable) {
      /* The hash table should be empty at this point. */
      _mesa_hash_table_destroy(cache->hashtable, NULL);
      simple_mtx_destroy(&cache->lock);
   }
}

void *
util_live_shader_cache_get(struct pipe_context *ctx,
                           struct util_live_shader_cache *cache,
                           const struct pipe_shader_state *state,
                           bool* cache_hit)
{
   struct blob blob = {0};
   unsigned ir_size;
   const void *ir_binary;
   enum pipe_shader_type stage;

   /* Get the shader binary and shader stage. */
   if (state->type == PIPE_SHADER_IR_TGSI) {
      ir_binary = state->tokens;
      ir_size = tgsi_num_tokens(state->tokens) *
                sizeof(struct tgsi_token);
      stage = tgsi_get_processor_type(state->tokens);
   } else if (state->type == PIPE_SHADER_IR_NIR) {
      blob_init(&blob);
      nir_serialize(&blob, state->ir.nir, true);
      ir_binary = blob.data;
      ir_size = blob.size;
      stage = pipe_shader_type_from_mesa(((nir_shader*)state->ir.nir)->info.stage);
   } else {
      assert(0);
      return NULL;
   }

   /* Compute SHA1 of pipe_shader_state. */
   struct mesa_sha1 sha1_ctx;
   unsigned char sha1[20];
   _mesa_sha1_init(&sha1_ctx);
   _mesa_sha1_update(&sha1_ctx, ir_binary, ir_size);
   if ((stage == PIPE_SHADER_VERTEX ||
        stage == PIPE_SHADER_TESS_EVAL ||
        stage == PIPE_SHADER_GEOMETRY) &&
       state->stream_output.num_outputs) {
      _mesa_sha1_update(&sha1_ctx, &state->stream_output,
                        sizeof(state->stream_output));
   }
   _mesa_sha1_final(&sha1_ctx, sha1);

   if (ir_binary == blob.data)
      blob_finish(&blob);

   /* Find the shader in the live cache. */
   simple_mtx_lock(&cache->lock);
   struct hash_entry *entry = _mesa_hash_table_search(cache->hashtable, sha1);
   struct util_live_shader *shader = entry ? entry->data : NULL;

   /* Increase the refcount. */
   if (shader) {
      pipe_reference(NULL, &shader->reference);
      cache->hits++;
   }
   simple_mtx_unlock(&cache->lock);

   if (cache_hit)
      *cache_hit = (shader != NULL);

   /* Return if the shader already exists. */
   if (shader) {
      if (state->type == PIPE_SHADER_IR_NIR)
          ralloc_free(state->ir.nir);
      return shader;
   }

   /* The cache mutex is unlocked to allow multiple create_shader
    * invocations to run simultaneously.
    */
   shader = (struct util_live_shader*)cache->create_shader(ctx, state);
   pipe_reference_init(&shader->reference, 1);
   memcpy(shader->sha1, sha1, sizeof(sha1));

   simple_mtx_lock(&cache->lock);
   /* The same shader might have been created in parallel. This is rare.
    * If so, keep the one already in cache.
    */
   struct hash_entry *entry2 = _mesa_hash_table_search(cache->hashtable, sha1);
   struct util_live_shader *shader2 = entry2 ? entry2->data : NULL;

   if (shader2) {
      cache->destroy_shader(ctx, shader);
      shader = shader2;
      /* Increase the refcount. */
      pipe_reference(NULL, &shader->reference);
   } else {
      _mesa_hash_table_insert(cache->hashtable, shader->sha1, shader);
   }
   cache->misses++;
   simple_mtx_unlock(&cache->lock);

   return shader;
}

void
util_shader_reference(struct pipe_context *ctx,
                      struct util_live_shader_cache *cache,
                      void **dst, void *src)
{
   if (*dst == src)
      return;

   struct util_live_shader *dst_shader = (struct util_live_shader*)*dst;
   struct util_live_shader *src_shader = (struct util_live_shader*)src;

   simple_mtx_lock(&cache->lock);
   bool destroy = pipe_reference(&dst_shader->reference, &src_shader->reference);
   if (destroy) {
      struct hash_entry *entry = _mesa_hash_table_search(cache->hashtable,
                                                         dst_shader->sha1);
      assert(entry);
      _mesa_hash_table_remove(cache->hashtable, entry);
   }
   simple_mtx_unlock(&cache->lock);

   if (destroy)
      cache->destroy_shader(ctx, dst_shader);

   *dst = src;
}
