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
 * @file crocus_disk_cache.c
 *
 * Functions for interacting with the on-disk shader cache.
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "compiler/nir/nir.h"
#include "util/blob.h"
#include "util/build_id.h"
#include "util/disk_cache.h"
#include "util/mesa-sha1.h"

#include "crocus_context.h"

static bool debug = false;

/**
 * Compute a disk cache key for the given uncompiled shader and NOS key.
 */
static void
crocus_disk_cache_compute_key(struct disk_cache *cache,
                              const struct crocus_uncompiled_shader *ish,
                              const void *orig_prog_key,
                              uint32_t prog_key_size,
                              cache_key cache_key)
{
   /* Create a copy of the program key with program_string_id zeroed out.
    * It's essentially random data which we don't want to include in our
    * hashing and comparisons.  We'll set a proper value on a cache hit.
    */
   union brw_any_prog_key prog_key;
   memcpy(&prog_key, orig_prog_key, prog_key_size);
   prog_key.base.program_string_id = 0;

   uint8_t data[sizeof(prog_key) + sizeof(ish->nir_sha1)];
   uint32_t data_size = prog_key_size + sizeof(ish->nir_sha1);

   memcpy(data, ish->nir_sha1, sizeof(ish->nir_sha1));
   memcpy(data + sizeof(ish->nir_sha1), &prog_key, prog_key_size);

   disk_cache_compute_key(cache, data, data_size, cache_key);
}

/**
 * Store the given compiled shader in the disk cache.
 *
 * This should only be called on newly compiled shaders.  No checking is
 * done to prevent repeated stores of the same shader.
 */
void
crocus_disk_cache_store(struct disk_cache *cache,
                        const struct crocus_uncompiled_shader *ish,
                        const struct crocus_compiled_shader *shader,
                        void *map,
                        const void *prog_key,
                        uint32_t prog_key_size)
{
#ifdef ENABLE_SHADER_CACHE
   if (!cache)
      return;

   gl_shader_stage stage = ish->nir->info.stage;
   const struct brw_stage_prog_data *prog_data = shader->prog_data;

   cache_key cache_key;
   crocus_disk_cache_compute_key(cache, ish, prog_key, prog_key_size, cache_key);

   if (debug) {
      char sha1[41];
      _mesa_sha1_format(sha1, cache_key);
      fprintf(stderr, "[mesa disk cache] storing %s\n", sha1);
   }

   struct blob blob;
   blob_init(&blob);

   /* We write the following data to the cache blob:
    *
    * 1. Prog data (must come first because it has the assembly size)
    * 2. Assembly code
    * 3. Number of entries in the system value array
    * 4. System value array
    * 5. Legacy param array (only used for compute workgroup ID)
    * 6. Binding table
    */
   blob_write_bytes(&blob, shader->prog_data, brw_prog_data_size(stage));
   blob_write_bytes(&blob, map + shader->offset, shader->prog_data->program_size);
   blob_write_bytes(&blob, &shader->num_system_values, sizeof(unsigned));
   blob_write_bytes(&blob, shader->system_values,
                    shader->num_system_values * sizeof(enum brw_param_builtin));
   blob_write_bytes(&blob, prog_data->param,
                    prog_data->nr_params * sizeof(uint32_t));
   blob_write_bytes(&blob, &shader->bt, sizeof(shader->bt));

   disk_cache_put(cache, cache_key, blob.data, blob.size, NULL);
   blob_finish(&blob);
#endif
}

/**
 * Search for a compiled shader in the disk cache.  If found, upload it
 * to the in-memory program cache so we can use it.
 */
struct crocus_compiled_shader *
crocus_disk_cache_retrieve(struct crocus_context *ice,
                           const struct crocus_uncompiled_shader *ish,
                           const void *prog_key,
                           uint32_t key_size)
{
#ifdef ENABLE_SHADER_CACHE
   struct crocus_screen *screen = (void *) ice->ctx.screen;
   struct disk_cache *cache = screen->disk_cache;
   gl_shader_stage stage = ish->nir->info.stage;

   if (!cache)
      return NULL;

   cache_key cache_key;
   crocus_disk_cache_compute_key(cache, ish, prog_key, key_size, cache_key);

   if (debug) {
      char sha1[41];
      _mesa_sha1_format(sha1, cache_key);
      fprintf(stderr, "[mesa disk cache] retrieving %s: ", sha1);
   }

   size_t size;
   void *buffer = disk_cache_get(screen->disk_cache, cache_key, &size);

   if (debug)
      fprintf(stderr, "%s\n", buffer ? "found" : "missing");

   if (!buffer)
      return NULL;

   const uint32_t prog_data_size = brw_prog_data_size(stage);

   struct brw_stage_prog_data *prog_data = ralloc_size(NULL, prog_data_size);
   const void *assembly;
   uint32_t num_system_values;
   uint32_t *system_values = NULL;
   uint32_t *so_decls = NULL;

   struct blob_reader blob;
   blob_reader_init(&blob, buffer, size);
   blob_copy_bytes(&blob, prog_data, prog_data_size);
   assembly = blob_read_bytes(&blob, prog_data->program_size);
   num_system_values = blob_read_uint32(&blob);
   if (num_system_values) {
      system_values =
         ralloc_array(NULL, enum brw_param_builtin, num_system_values);
      blob_copy_bytes(&blob, system_values,
                      num_system_values * sizeof(enum brw_param_builtin));
   }

   prog_data->param = NULL;
   if (prog_data->nr_params) {
      prog_data->param = ralloc_array(NULL, uint32_t, prog_data->nr_params);
      blob_copy_bytes(&blob, prog_data->param,
                      prog_data->nr_params * sizeof(uint32_t));
   }

   struct crocus_binding_table bt;
   blob_copy_bytes(&blob, &bt, sizeof(bt));

   if ((stage == MESA_SHADER_VERTEX ||
        stage == MESA_SHADER_TESS_EVAL ||
        stage == MESA_SHADER_GEOMETRY) && screen->devinfo.ver > 6) {
      struct brw_vue_prog_data *vue_prog_data = (void *) prog_data;
      so_decls = screen->vtbl.create_so_decl_list(&ish->stream_output,
                                                  &vue_prog_data->vue_map);
   }

   /* System values and uniforms are stored in constant buffer 0, the
    * user-facing UBOs are indexed by one.  So if any constant buffer is
    * needed, the constant buffer 0 will be needed, so account for it.
    */
   unsigned num_cbufs = ish->nir->info.num_ubos;

   if (num_cbufs || ish->nir->num_uniforms)
      num_cbufs++;

   if (num_system_values)
      num_cbufs++;

   /* Upload our newly read shader to the in-memory program cache and
    * return it to the caller.
    */
   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, stage, key_size, prog_key, assembly,
                           prog_data->program_size,
                           prog_data, prog_data_size, so_decls, system_values,
                           num_system_values, num_cbufs, &bt);

   free(buffer);

   return shader;
#else
   return NULL;
#endif
}

/**
 * Initialize the on-disk shader cache.
 */
void
crocus_disk_cache_init(struct crocus_screen *screen)
{
#ifdef ENABLE_SHADER_CACHE
   if (INTEL_DEBUG(DEBUG_DISK_CACHE_DISABLE_MASK))
      return;

   /* array length = print length + nul char + 1 extra to verify it's unused */
   char renderer[13];
   UNUSED int len =
      snprintf(renderer, sizeof(renderer), "crocus_%04x", screen->pci_id);
   assert(len == sizeof(renderer) - 2);

   const struct build_id_note *note =
      build_id_find_nhdr_for_addr(crocus_disk_cache_init);
   assert(note && build_id_length(note) == 20); /* sha1 */

   const uint8_t *id_sha1 = build_id_data(note);
   assert(id_sha1);

   char timestamp[41];
   _mesa_sha1_format(timestamp, id_sha1);

   const uint64_t driver_flags =
      brw_get_compiler_config_value(screen->compiler);
   screen->disk_cache = disk_cache_create(renderer, timestamp, driver_flags);
#endif
}
