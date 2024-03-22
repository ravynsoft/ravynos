/*
 * Copyright © 2017 Intel Corporation
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
 * @file crocus_program_cache.c
 *
 * The in-memory program cache.  This is basically a hash table mapping
 * API-specified shaders and a state key to a compiled variant.  It also
 * takes care of uploading shader assembly into a BO for use on the GPU.
 */

#include <stdio.h>
#include <errno.h>
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_atomic.h"
#include "util/u_upload_mgr.h"
#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "intel/compiler/brw_compiler.h"
#include "intel/compiler/brw_eu.h"
#include "intel/compiler/brw_nir.h"
#include "crocus_context.h"
#include "crocus_resource.h"

struct keybox {
   uint16_t size;
   enum crocus_program_cache_id cache_id;
   uint8_t data[0];
};

static struct keybox *
make_keybox(void *mem_ctx, enum crocus_program_cache_id cache_id,
            const void *key, uint32_t key_size)
{
   struct keybox *keybox =
      ralloc_size(mem_ctx, sizeof(struct keybox) + key_size);

   keybox->cache_id = cache_id;
   keybox->size = key_size;
   memcpy(keybox->data, key, key_size);

   return keybox;
}

static uint32_t
keybox_hash(const void *void_key)
{
   const struct keybox *key = void_key;
   return _mesa_hash_data(&key->cache_id, key->size + sizeof(key->cache_id));
}

static bool
keybox_equals(const void *void_a, const void *void_b)
{
   const struct keybox *a = void_a, *b = void_b;
   if (a->size != b->size)
      return false;

   return memcmp(a->data, b->data, a->size) == 0;
}

struct crocus_compiled_shader *
crocus_find_cached_shader(struct crocus_context *ice,
                          enum crocus_program_cache_id cache_id,
                          uint32_t key_size, const void *key)
{
   struct keybox *keybox = make_keybox(NULL, cache_id, key, key_size);
   struct hash_entry *entry =
      _mesa_hash_table_search(ice->shaders.cache, keybox);

   ralloc_free(keybox);

   return entry ? entry->data : NULL;
}

const void *
crocus_find_previous_compile(const struct crocus_context *ice,
                             enum crocus_program_cache_id cache_id,
                             unsigned program_string_id)
{
   hash_table_foreach(ice->shaders.cache, entry) {
      const struct keybox *keybox = entry->key;
      const struct brw_base_prog_key *key = (const void *)keybox->data;
      if (keybox->cache_id == cache_id &&
          key->program_string_id == program_string_id) {
         return keybox->data;
      }
   }

   return NULL;
}

/**
 * Look for an existing entry in the cache that has identical assembly code.
 *
 * This is useful for programs generating shaders at runtime, where multiple
 * distinct shaders (from an API perspective) may compile to the same assembly
 * in our backend.  This saves space in the program cache buffer.
 */
static const struct crocus_compiled_shader *
find_existing_assembly(struct hash_table *cache, void *map,
                       const void *assembly, unsigned assembly_size)
{
   hash_table_foreach (cache, entry) {
      const struct crocus_compiled_shader *existing = entry->data;

      if (existing->map_size != assembly_size)
         continue;

      if (memcmp(map + existing->offset, assembly, assembly_size) == 0)
         return existing;
   }
   return NULL;
}

static void
crocus_cache_new_bo(struct crocus_context *ice,
                    uint32_t new_size)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   struct crocus_bo *new_bo;
   new_bo = crocus_bo_alloc(screen->bufmgr, "program cache", new_size);

   void *map = crocus_bo_map(NULL, new_bo, MAP_READ | MAP_WRITE |
                             MAP_ASYNC | MAP_PERSISTENT);

   if (ice->shaders.cache_next_offset != 0) {
      memcpy(map, ice->shaders.cache_bo_map, ice->shaders.cache_next_offset);
   }

   crocus_bo_unmap(ice->shaders.cache_bo);
   crocus_bo_unreference(ice->shaders.cache_bo);
   ice->shaders.cache_bo = new_bo;
   ice->shaders.cache_bo_map = map;

   if (screen->devinfo.ver <= 5) {
      /* reemit all shaders on GEN4 only. */
      ice->state.dirty |= CROCUS_DIRTY_CLIP | CROCUS_DIRTY_RASTER |
         CROCUS_DIRTY_WM;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_VS;
   }
   ice->batches[CROCUS_BATCH_RENDER].state_base_address_emitted = false;
   ice->batches[CROCUS_BATCH_COMPUTE].state_base_address_emitted = false;
   /* unset state base address */
}

static uint32_t
crocus_alloc_item_data(struct crocus_context *ice, uint32_t size)
{
   if (ice->shaders.cache_next_offset + size > ice->shaders.cache_bo->size) {
      uint32_t new_size = ice->shaders.cache_bo->size * 2;
      while (ice->shaders.cache_next_offset + size > new_size)
         new_size *= 2;

      crocus_cache_new_bo(ice, new_size);
   }
   uint32_t offset = ice->shaders.cache_next_offset;

   /* Programs are always 64-byte aligned, so set up the next one now */
   ice->shaders.cache_next_offset = ALIGN(offset + size, 64);
   return offset;
}

struct crocus_compiled_shader *
crocus_upload_shader(struct crocus_context *ice,
                     enum crocus_program_cache_id cache_id, uint32_t key_size,
                     const void *key, const void *assembly, uint32_t asm_size,
                     struct brw_stage_prog_data *prog_data,
                     uint32_t prog_data_size, uint32_t *streamout,
                     enum brw_param_builtin *system_values,
                     unsigned num_system_values, unsigned num_cbufs,
                     const struct crocus_binding_table *bt)
{
   struct hash_table *cache = ice->shaders.cache;
   struct crocus_compiled_shader *shader =
      rzalloc_size(cache, sizeof(struct crocus_compiled_shader));
   const struct crocus_compiled_shader *existing = find_existing_assembly(
      cache, ice->shaders.cache_bo_map, assembly, asm_size);

   /* If we can find a matching prog in the cache already, then reuse the
    * existing stuff without creating new copy into the underlying buffer
    * object.  This is notably useful for programs generating shaders at
    * runtime, where multiple shaders may compile to the same thing in our
    * backend.
    */
   if (existing) {
      shader->offset = existing->offset;
      shader->map_size = existing->map_size;
   } else {
      shader->offset = crocus_alloc_item_data(ice, asm_size);
      shader->map_size = asm_size;

      memcpy(ice->shaders.cache_bo_map + shader->offset, assembly, asm_size);
   }

   shader->prog_data = prog_data;
   shader->prog_data_size = prog_data_size;
   shader->streamout = streamout;
   shader->system_values = system_values;
   shader->num_system_values = num_system_values;
   shader->num_cbufs = num_cbufs;
   shader->bt = *bt;

   ralloc_steal(shader, shader->prog_data);
   if (prog_data_size > 16)
      ralloc_steal(shader->prog_data, prog_data->param);
   ralloc_steal(shader, shader->streamout);
   ralloc_steal(shader, shader->system_values);

   struct keybox *keybox = make_keybox(shader, cache_id, key, key_size);
   _mesa_hash_table_insert(ice->shaders.cache, keybox, shader);

   return shader;
}

bool
crocus_blorp_lookup_shader(struct blorp_batch *blorp_batch, const void *key,
                           uint32_t key_size, uint32_t *kernel_out,
                           void *prog_data_out)
{
   struct blorp_context *blorp = blorp_batch->blorp;
   struct crocus_context *ice = blorp->driver_ctx;
   struct crocus_compiled_shader *shader =
      crocus_find_cached_shader(ice, CROCUS_CACHE_BLORP, key_size, key);

   if (!shader)
      return false;

   *kernel_out = shader->offset;
   *((void **)prog_data_out) = shader->prog_data;

   return true;
}

bool
crocus_blorp_upload_shader(struct blorp_batch *blorp_batch, uint32_t stage,
                           const void *key, uint32_t key_size,
                           const void *kernel, uint32_t kernel_size,
                           const struct brw_stage_prog_data *prog_data_templ,
                           uint32_t prog_data_size, uint32_t *kernel_out,
                           void *prog_data_out)
{
   struct blorp_context *blorp = blorp_batch->blorp;
   struct crocus_context *ice = blorp->driver_ctx;

   struct brw_stage_prog_data *prog_data = ralloc_size(NULL, prog_data_size);
   memcpy(prog_data, prog_data_templ, prog_data_size);

   struct crocus_binding_table bt;
   memset(&bt, 0, sizeof(bt));

   struct crocus_compiled_shader *shader = crocus_upload_shader(
      ice, CROCUS_CACHE_BLORP, key_size, key, kernel, kernel_size, prog_data,
      prog_data_size, NULL, NULL, 0, 0, &bt);

   *kernel_out = shader->offset;
   *((void **)prog_data_out) = shader->prog_data;

   return true;
}

void
crocus_init_program_cache(struct crocus_context *ice)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   ice->shaders.cache =
      _mesa_hash_table_create(ice, keybox_hash, keybox_equals);

   ice->shaders.cache_bo =
      crocus_bo_alloc(screen->bufmgr, "program_cache", 16384);
   ice->shaders.cache_bo_map =
      crocus_bo_map(NULL, ice->shaders.cache_bo,
                    MAP_READ | MAP_WRITE | MAP_ASYNC | MAP_PERSISTENT);
}

void
crocus_destroy_program_cache(struct crocus_context *ice)
{
   for (int i = 0; i < MESA_SHADER_STAGES; i++) {
      ice->shaders.prog[i] = NULL;
   }

   if (ice->shaders.cache_bo) {
      crocus_bo_unmap(ice->shaders.cache_bo);
      crocus_bo_unreference(ice->shaders.cache_bo);
      ice->shaders.cache_bo_map = NULL;
      ice->shaders.cache_bo = NULL;
   }

   ralloc_free(ice->shaders.cache);
}

static const char *
cache_name(enum crocus_program_cache_id cache_id)
{
   if (cache_id == CROCUS_CACHE_BLORP)
      return "BLORP";

   if (cache_id == CROCUS_CACHE_SF)
      return "SF";

   if (cache_id == CROCUS_CACHE_CLIP)
      return "CLIP";

   if (cache_id == CROCUS_CACHE_FF_GS)
      return "FF_GS";

   return _mesa_shader_stage_to_string(cache_id);
}

void
crocus_print_program_cache(struct crocus_context *ice)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct brw_isa_info *isa = &screen->compiler->isa;

   hash_table_foreach(ice->shaders.cache, entry) {
      const struct keybox *keybox = entry->key;
      struct crocus_compiled_shader *shader = entry->data;
      fprintf(stderr, "%s:\n", cache_name(keybox->cache_id));
      brw_disassemble(isa, ice->shaders.cache_bo_map + shader->offset, 0,
                      shader->prog_data->program_size, NULL, stderr);
   }
}
