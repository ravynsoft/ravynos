/*
 * Copyright 2018 Collabora Ltd.
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

#include "zink_program.h"

#include "zink_compiler.h"
#include "zink_context.h"
#include "zink_descriptors.h"
#include "zink_helpers.h"
#include "zink_pipeline.h"
#include "zink_render_pass.h"
#include "zink_resource.h"
#include "zink_screen.h"
#include "zink_state.h"
#include "zink_inlines.h"

#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "nir_serialize.h"
#include "nir/nir_draw_helpers.h"

/* for pipeline cache */
#define XXH_INLINE_ALL
#include "util/xxhash.h"

static void
precompile_job(void *data, void *gdata, int thread_index);
struct zink_gfx_program *
create_gfx_program_separable(struct zink_context *ctx, struct zink_shader **stages, unsigned vertices_per_patch);

void
debug_describe_zink_gfx_program(char *buf, const struct zink_gfx_program *ptr)
{
   sprintf(buf, "zink_gfx_program");
}

void
debug_describe_zink_compute_program(char *buf, const struct zink_compute_program *ptr)
{
   sprintf(buf, "zink_compute_program");
}

ALWAYS_INLINE static bool
shader_key_matches_tcs_nongenerated(const struct zink_shader_module *zm, const struct zink_shader_key *key, unsigned num_uniforms)
{
   if (zm->num_uniforms != num_uniforms || zm->has_nonseamless != !!key->base.nonseamless_cube_mask ||
       zm->needs_zs_shader_swizzle != key->base.needs_zs_shader_swizzle)
      return false;
   const uint32_t nonseamless_size = zm->has_nonseamless ? sizeof(uint32_t) : 0;
   return (!nonseamless_size || !memcmp(zm->key + zm->key_size, &key->base.nonseamless_cube_mask, nonseamless_size)) &&
          (!num_uniforms || !memcmp(zm->key + zm->key_size + nonseamless_size,
                                    key->base.inlined_uniform_values, zm->num_uniforms * sizeof(uint32_t)));
}

ALWAYS_INLINE static bool
shader_key_matches(const struct zink_shader_module *zm,
                   const struct zink_shader_key *key, unsigned num_uniforms,
                   bool has_inline, bool has_nonseamless)
{
   const uint32_t nonseamless_size = !has_nonseamless && zm->has_nonseamless ? sizeof(uint32_t) : 0;
   if (has_inline) {
      if (zm->num_uniforms != num_uniforms ||
          (num_uniforms &&
           memcmp(zm->key + zm->key_size + nonseamless_size,
                  key->base.inlined_uniform_values, zm->num_uniforms * sizeof(uint32_t))))
         return false;
   }
   if (!has_nonseamless) {
      if (zm->has_nonseamless != !!key->base.nonseamless_cube_mask ||
          (nonseamless_size && memcmp(zm->key + zm->key_size, &key->base.nonseamless_cube_mask, nonseamless_size)))
         return false;
   }
   if (zm->needs_zs_shader_swizzle != key->base.needs_zs_shader_swizzle)
      return false;
   return !memcmp(zm->key, key, zm->key_size);
}

static uint32_t
shader_module_hash(const struct zink_shader_module *zm)
{
   const uint32_t nonseamless_size = zm->has_nonseamless ? sizeof(uint32_t) : 0;
   unsigned key_size = zm->key_size + nonseamless_size + zm->num_uniforms * sizeof(uint32_t);
   return _mesa_hash_data(zm->key, key_size);
}

ALWAYS_INLINE static void
gather_shader_module_info(struct zink_context *ctx, struct zink_screen *screen,
                          struct zink_shader *zs, struct zink_gfx_program *prog,
                          struct zink_gfx_pipeline_state *state,
                          bool has_inline, //is inlining enabled?
                          bool has_nonseamless, //is nonseamless ext present?
                          unsigned *inline_size, unsigned *nonseamless_size)
{
   gl_shader_stage stage = zs->info.stage;
   struct zink_shader_key *key = &state->shader_keys.key[stage];
   if (has_inline && ctx && zs->info.num_inlinable_uniforms &&
       ctx->inlinable_uniforms_valid_mask & BITFIELD64_BIT(stage)) {
      if (zs->can_inline && (screen->is_cpu || prog->inlined_variant_count[stage] < ZINK_MAX_INLINED_VARIANTS))
         *inline_size = zs->info.num_inlinable_uniforms;
      else
         key->inline_uniforms = false;
   }
   if (!has_nonseamless && key->base.nonseamless_cube_mask)
      *nonseamless_size = sizeof(uint32_t);
}

ALWAYS_INLINE static struct zink_shader_module *
create_shader_module_for_stage(struct zink_context *ctx, struct zink_screen *screen,
                               struct zink_shader *zs, struct zink_gfx_program *prog,
                               gl_shader_stage stage,
                               struct zink_gfx_pipeline_state *state,
                               unsigned inline_size, unsigned nonseamless_size,
                               bool has_inline, //is inlining enabled?
                               bool has_nonseamless) //is nonseamless ext present?
{
   struct zink_shader_module *zm;
   const struct zink_shader_key *key = &state->shader_keys.key[stage];
   /* non-generated tcs won't use the shader key */
   const bool is_nongenerated_tcs = stage == MESA_SHADER_TESS_CTRL && !zs->non_fs.is_generated;
   const bool shadow_needs_shader_swizzle = key->base.needs_zs_shader_swizzle ||
                                            (stage == MESA_SHADER_FRAGMENT && key->key.fs.base.shadow_needs_shader_swizzle);
   zm = malloc(sizeof(struct zink_shader_module) + key->size +
               (!has_nonseamless ? nonseamless_size : 0) + inline_size * sizeof(uint32_t) +
               (shadow_needs_shader_swizzle ? sizeof(struct zink_zs_swizzle_key) : 0));
   if (!zm) {
      return NULL;
   }
   unsigned patch_vertices = state->shader_keys.key[MESA_SHADER_TESS_CTRL].key.tcs.patch_vertices;
   if (stage == MESA_SHADER_TESS_CTRL && zs->non_fs.is_generated && zs->spirv) {
      assert(ctx); //TODO async
      zm->obj = zink_shader_tcs_compile(screen, zs, patch_vertices, prog->base.uses_shobj, &prog->base);
   } else {
      zm->obj = zink_shader_compile(screen, prog->base.uses_shobj, zs, zink_shader_blob_deserialize(screen, &prog->blobs[stage]), key, &ctx->di.zs_swizzle[stage], &prog->base);
   }
   if (!zm->obj.mod) {
      FREE(zm);
      return NULL;
   }
   zm->shobj = prog->base.uses_shobj;
   zm->num_uniforms = inline_size;
   if (!is_nongenerated_tcs) {
      zm->key_size = key->size;
      memcpy(zm->key, key, key->size);
   } else {
      zm->key_size = 0;
      memset(zm->key, 0, key->size);
   }
   if (!has_nonseamless && nonseamless_size) {
      /* nonseamless mask gets added to base key if it exists */
      memcpy(zm->key + key->size, &key->base.nonseamless_cube_mask, nonseamless_size);
   }
   zm->needs_zs_shader_swizzle = shadow_needs_shader_swizzle;
   zm->has_nonseamless = has_nonseamless ? 0 : !!nonseamless_size;
   if (inline_size)
      memcpy(zm->key + key->size + nonseamless_size, key->base.inlined_uniform_values, inline_size * sizeof(uint32_t));
   if (stage == MESA_SHADER_TESS_CTRL && zs->non_fs.is_generated)
      zm->hash = patch_vertices;
   else
      zm->hash = shader_module_hash(zm);
   if (unlikely(shadow_needs_shader_swizzle)) {
      memcpy(zm->key + key->size + nonseamless_size + inline_size * sizeof(uint32_t), &ctx->di.zs_swizzle[stage], sizeof(struct zink_zs_swizzle_key));
      zm->hash ^= _mesa_hash_data(&ctx->di.zs_swizzle[stage], sizeof(struct zink_zs_swizzle_key));
   }
   zm->default_variant = !shadow_needs_shader_swizzle && !inline_size && !util_dynarray_contains(&prog->shader_cache[stage][0][0], void*);
   if (inline_size)
      prog->inlined_variant_count[stage]++;
   util_dynarray_append(&prog->shader_cache[stage][has_nonseamless ? 0 : !!nonseamless_size][!!inline_size], void*, zm);
   return zm;
}

ALWAYS_INLINE static struct zink_shader_module *
get_shader_module_for_stage(struct zink_context *ctx, struct zink_screen *screen,
                            struct zink_shader *zs, struct zink_gfx_program *prog,
                            gl_shader_stage stage,
                            struct zink_gfx_pipeline_state *state,
                            unsigned inline_size, unsigned nonseamless_size,
                            bool has_inline, //is inlining enabled?
                            bool has_nonseamless) //is nonseamless ext present?
{
   const struct zink_shader_key *key = &state->shader_keys.key[stage];
   /* non-generated tcs won't use the shader key */
   const bool is_nongenerated_tcs = stage == MESA_SHADER_TESS_CTRL && !zs->non_fs.is_generated;
   const bool shadow_needs_shader_swizzle = unlikely(key->base.needs_zs_shader_swizzle) ||
                                            (stage == MESA_SHADER_FRAGMENT && unlikely(key->key.fs.base.shadow_needs_shader_swizzle));

   struct util_dynarray *shader_cache = &prog->shader_cache[stage][!has_nonseamless ? !!nonseamless_size : 0][has_inline ? !!inline_size : 0];
   unsigned count = util_dynarray_num_elements(shader_cache, struct zink_shader_module *);
   struct zink_shader_module **pzm = shader_cache->data;
   for (unsigned i = 0; i < count; i++) {
      struct zink_shader_module *iter = pzm[i];
      if (is_nongenerated_tcs) {
         if (!shader_key_matches_tcs_nongenerated(iter, key, has_inline ? !!inline_size : 0))
            continue;
      } else {
         if (stage == MESA_SHADER_VERTEX && iter->key_size != key->size)
            continue;
         if (!shader_key_matches(iter, key, inline_size, has_inline, has_nonseamless))
            continue;
         if (unlikely(shadow_needs_shader_swizzle)) {
            /* shadow swizzle data needs a manual compare since it's so fat */
            if (memcmp(iter->key + iter->key_size + nonseamless_size + iter->num_uniforms * sizeof(uint32_t),
                       &ctx->di.zs_swizzle[stage], sizeof(struct zink_zs_swizzle_key)))
               continue;
         }
      }
      if (i > 0) {
         struct zink_shader_module *zero = pzm[0];
         pzm[0] = iter;
         pzm[i] = zero;
      }
      return iter;
   }

   return NULL;
}

ALWAYS_INLINE static struct zink_shader_module *
create_shader_module_for_stage_optimal(struct zink_context *ctx, struct zink_screen *screen,
                                       struct zink_shader *zs, struct zink_gfx_program *prog,
                                       gl_shader_stage stage,
                                       struct zink_gfx_pipeline_state *state)
{
   struct zink_shader_module *zm;
   uint16_t *key;
   unsigned mask = stage == MESA_SHADER_FRAGMENT ? BITFIELD_MASK(16) : BITFIELD_MASK(8);
   bool shadow_needs_shader_swizzle = false;
   if (zs == prog->last_vertex_stage) {
      key = (uint16_t*)&state->shader_keys_optimal.key.vs_base;
   } else if (stage == MESA_SHADER_FRAGMENT) {
      key = (uint16_t*)&state->shader_keys_optimal.key.fs;
      shadow_needs_shader_swizzle = ctx ? ctx->gfx_pipeline_state.shader_keys_optimal.key.fs.shadow_needs_shader_swizzle : false;
   } else if (stage == MESA_SHADER_TESS_CTRL && zs->non_fs.is_generated) {
      key = (uint16_t*)&state->shader_keys_optimal.key.tcs;
   } else {
      key = NULL;
   }
   size_t key_size = sizeof(uint16_t);
   zm = calloc(1, sizeof(struct zink_shader_module) + (key ? key_size : 0) + (unlikely(shadow_needs_shader_swizzle) ? sizeof(struct zink_zs_swizzle_key) : 0));
   if (!zm) {
      return NULL;
   }
   if (stage == MESA_SHADER_TESS_CTRL && zs->non_fs.is_generated && zs->spirv) {
      assert(ctx || screen->info.dynamic_state2_feats.extendedDynamicState2PatchControlPoints);
      unsigned patch_vertices = 3;
      if (ctx) {
         struct zink_tcs_key *tcs = (struct zink_tcs_key*)key;
         patch_vertices = tcs->patch_vertices;
      }
      zm->obj = zink_shader_tcs_compile(screen, zs, patch_vertices, prog->base.uses_shobj, &prog->base);
   } else {
      zm->obj = zink_shader_compile(screen, prog->base.uses_shobj, zs, zink_shader_blob_deserialize(screen, &prog->blobs[stage]),
                                    (struct zink_shader_key*)key, shadow_needs_shader_swizzle ? &ctx->di.zs_swizzle[stage] : NULL, &prog->base);
   }
   if (!zm->obj.mod) {
      FREE(zm);
      return NULL;
   }
   zm->shobj = prog->base.uses_shobj;
   /* non-generated tcs won't use the shader key */
   const bool is_nongenerated_tcs = stage == MESA_SHADER_TESS_CTRL && !zs->non_fs.is_generated;
   if (key && !is_nongenerated_tcs) {
      zm->key_size = key_size;
      uint16_t *data = (uint16_t*)zm->key;
      /* sanitize actual key bits */
      *data = (*key) & mask;
      if (unlikely(shadow_needs_shader_swizzle))
         memcpy(&data[1], &ctx->di.zs_swizzle[stage], sizeof(struct zink_zs_swizzle_key));
   }
   zm->default_variant = !util_dynarray_contains(&prog->shader_cache[stage][0][0], void*);
   util_dynarray_append(&prog->shader_cache[stage][0][0], void*, zm);
   return zm;
}

ALWAYS_INLINE static struct zink_shader_module *
get_shader_module_for_stage_optimal(struct zink_context *ctx, struct zink_screen *screen,
                                    struct zink_shader *zs, struct zink_gfx_program *prog,
                                    gl_shader_stage stage,
                                    struct zink_gfx_pipeline_state *state)
{
   /* non-generated tcs won't use the shader key */
   const bool is_nongenerated_tcs = stage == MESA_SHADER_TESS_CTRL && !zs->non_fs.is_generated;
   bool shadow_needs_shader_swizzle = false;
   uint16_t *key;
   unsigned mask = stage == MESA_SHADER_FRAGMENT ? BITFIELD_MASK(16) : BITFIELD_MASK(8);
   if (zs == prog->last_vertex_stage) {
      key = (uint16_t*)&ctx->gfx_pipeline_state.shader_keys_optimal.key.vs_base;
   } else if (stage == MESA_SHADER_FRAGMENT) {
      key = (uint16_t*)&ctx->gfx_pipeline_state.shader_keys_optimal.key.fs;
      shadow_needs_shader_swizzle = ctx->gfx_pipeline_state.shader_keys_optimal.key.fs.shadow_needs_shader_swizzle;
   } else if (stage == MESA_SHADER_TESS_CTRL && zs->non_fs.is_generated) {
      key = (uint16_t*)&ctx->gfx_pipeline_state.shader_keys_optimal.key.tcs;
   } else {
      key = NULL;
   }
   struct util_dynarray *shader_cache = &prog->shader_cache[stage][0][0];
   unsigned count = util_dynarray_num_elements(shader_cache, struct zink_shader_module *);
   struct zink_shader_module **pzm = shader_cache->data;
   for (unsigned i = 0; i < count; i++) {
      struct zink_shader_module *iter = pzm[i];
      if (is_nongenerated_tcs) {
         /* always match */
      } else if (key) {
         uint16_t val = (*key) & mask;
         /* no key is bigger than uint16_t */
         if (memcmp(iter->key, &val, sizeof(uint16_t)))
            continue;
         if (unlikely(shadow_needs_shader_swizzle)) {
            /* shadow swizzle data needs a manual compare since it's so fat */
            if (memcmp(iter->key + sizeof(uint16_t), &ctx->di.zs_swizzle[stage], sizeof(struct zink_zs_swizzle_key)))
               continue;
         }
      }
      if (i > 0) {
         struct zink_shader_module *zero = pzm[0];
         pzm[0] = iter;
         pzm[i] = zero;
      }
      return iter;
   }

   return NULL;
}

static void
zink_destroy_shader_module(struct zink_screen *screen, struct zink_shader_module *zm)
{
   if (zm->shobj)
      VKSCR(DestroyShaderEXT)(screen->dev, zm->obj.obj, NULL);
   else
      VKSCR(DestroyShaderModule)(screen->dev, zm->obj.mod, NULL);
   ralloc_free(zm->obj.spirv);
   free(zm);
}

static void
destroy_shader_cache(struct zink_screen *screen, struct util_dynarray *sc)
{
   while (util_dynarray_contains(sc, void*)) {
      struct zink_shader_module *zm = util_dynarray_pop(sc, struct zink_shader_module*);
      zink_destroy_shader_module(screen, zm);
   }
}

ALWAYS_INLINE static void
update_gfx_shader_modules(struct zink_context *ctx,
                      struct zink_screen *screen,
                      struct zink_gfx_program *prog, uint32_t mask,
                      struct zink_gfx_pipeline_state *state,
                      bool has_inline, //is inlining enabled?
                      bool has_nonseamless) //is nonseamless ext present?
{
   bool hash_changed = false;
   bool default_variants = true;
   assert(prog->objs[MESA_SHADER_VERTEX].mod);
   uint32_t variant_hash = prog->last_variant_hash;
   prog->has_edgeflags = prog->shaders[MESA_SHADER_VERTEX]->has_edgeflags;
   for (unsigned i = 0; i < MESA_SHADER_COMPUTE; i++) {
      if (!(mask & BITFIELD_BIT(i)))
         continue;

      assert(prog->shaders[i]);

      unsigned inline_size = 0, nonseamless_size = 0;
      gather_shader_module_info(ctx, screen, prog->shaders[i], prog, state, has_inline, has_nonseamless, &inline_size, &nonseamless_size);
      struct zink_shader_module *zm = get_shader_module_for_stage(ctx, screen, prog->shaders[i], prog, i, state,
                                                                  inline_size, nonseamless_size, has_inline, has_nonseamless);
      if (!zm)
         zm = create_shader_module_for_stage(ctx, screen, prog->shaders[i], prog, i, state,
                                             inline_size, nonseamless_size, has_inline, has_nonseamless);
      state->modules[i] = zm->obj.mod;
      if (prog->objs[i].mod == zm->obj.mod)
         continue;
      prog->optimal_keys &= !prog->shaders[i]->non_fs.is_generated;
      variant_hash ^= prog->module_hash[i];
      hash_changed = true;
      default_variants &= zm->default_variant;
      prog->objs[i] = zm->obj;
      prog->objects[i] = zm->obj.obj;
      prog->module_hash[i] = zm->hash;
      if (has_inline) {
         if (zm->num_uniforms)
            prog->inline_variants |= BITFIELD_BIT(i);
         else
            prog->inline_variants &= ~BITFIELD_BIT(i);
      }
      variant_hash ^= prog->module_hash[i];
   }

   if (hash_changed && state) {
      if (default_variants)
         prog->last_variant_hash = prog->default_variant_hash;
      else
         prog->last_variant_hash = variant_hash;

      state->modules_changed = true;
   }
}

static void
generate_gfx_program_modules(struct zink_context *ctx, struct zink_screen *screen, struct zink_gfx_program *prog, struct zink_gfx_pipeline_state *state)
{
   assert(!prog->objs[MESA_SHADER_VERTEX].mod);
   uint32_t variant_hash = 0;
   bool default_variants = true;
   for (unsigned i = 0; i < MESA_SHADER_COMPUTE; i++) {
      if (!(prog->stages_present & BITFIELD_BIT(i)))
         continue;

      assert(prog->shaders[i]);

      unsigned inline_size = 0, nonseamless_size = 0;
      gather_shader_module_info(ctx, screen, prog->shaders[i], prog, state,
                                screen->driconf.inline_uniforms, screen->info.have_EXT_non_seamless_cube_map,
                                &inline_size, &nonseamless_size);
      struct zink_shader_module *zm = create_shader_module_for_stage(ctx, screen, prog->shaders[i], prog, i, state,
                                                                     inline_size, nonseamless_size,
                                                                     screen->driconf.inline_uniforms, screen->info.have_EXT_non_seamless_cube_map);
      state->modules[i] = zm->obj.mod;
      prog->objs[i] = zm->obj;
      prog->objects[i] = zm->obj.obj;
      prog->module_hash[i] = zm->hash;
      if (zm->num_uniforms)
         prog->inline_variants |= BITFIELD_BIT(i);
      default_variants &= zm->default_variant;
      variant_hash ^= prog->module_hash[i];
   }

   state->modules_changed = true;

   prog->last_variant_hash = variant_hash;
   if (default_variants)
      prog->default_variant_hash = prog->last_variant_hash;
}

static void
generate_gfx_program_modules_optimal(struct zink_context *ctx, struct zink_screen *screen, struct zink_gfx_program *prog, struct zink_gfx_pipeline_state *state)
{
   assert(!prog->objs[MESA_SHADER_VERTEX].mod);
   for (unsigned i = 0; i < MESA_SHADER_COMPUTE; i++) {
      if (!(prog->stages_present & BITFIELD_BIT(i)))
         continue;

      assert(prog->shaders[i]);

      struct zink_shader_module *zm = create_shader_module_for_stage_optimal(ctx, screen, prog->shaders[i], prog, i, state);
      prog->objs[i] = zm->obj;
      prog->objects[i] = zm->obj.obj;
   }

   state->modules_changed = true;
   prog->last_variant_hash = state->optimal_key;
}

static uint32_t
hash_pipeline_lib_generated_tcs(const void *key)
{
   const struct zink_gfx_library_key *gkey = key;
   return gkey->optimal_key;
}


static bool
equals_pipeline_lib_generated_tcs(const void *a, const void *b)
{
   return !memcmp(a, b, sizeof(uint32_t));
}

static uint32_t
hash_pipeline_lib(const void *key)
{
   const struct zink_gfx_library_key *gkey = key;
   /* remove generated tcs bits */
   return zink_shader_key_optimal_no_tcs(gkey->optimal_key);
}

static bool
equals_pipeline_lib(const void *a, const void *b)
{
   const struct zink_gfx_library_key *ak = a;
   const struct zink_gfx_library_key *bk = b;
   /* remove generated tcs bits */
   uint32_t val_a = zink_shader_key_optimal_no_tcs(ak->optimal_key);
   uint32_t val_b = zink_shader_key_optimal_no_tcs(bk->optimal_key);
   return val_a == val_b;
}

uint32_t
hash_gfx_input_dynamic(const void *key)
{
   const struct zink_gfx_input_key *ikey = key;
   return ikey->idx;
}

static bool
equals_gfx_input_dynamic(const void *a, const void *b)
{
   const struct zink_gfx_input_key *ikey_a = a;
   const struct zink_gfx_input_key *ikey_b = b;
   return ikey_a->idx == ikey_b->idx;
}

uint32_t
hash_gfx_input(const void *key)
{
   const struct zink_gfx_input_key *ikey = key;
   if (ikey->uses_dynamic_stride)
      return ikey->input;
   return _mesa_hash_data(key, offsetof(struct zink_gfx_input_key, pipeline));
}

static bool
equals_gfx_input(const void *a, const void *b)
{
   const struct zink_gfx_input_key *ikey_a = a;
   const struct zink_gfx_input_key *ikey_b = b;
   if (ikey_a->uses_dynamic_stride)
      return ikey_a->element_state == ikey_b->element_state &&
             !memcmp(a, b, offsetof(struct zink_gfx_input_key, vertex_buffers_enabled_mask));
   return !memcmp(a, b, offsetof(struct zink_gfx_input_key, pipeline));
}

uint32_t
hash_gfx_output_ds3(const void *key)
{
   const uint8_t *data = key;
   return _mesa_hash_data(data, sizeof(uint32_t));
}

static bool
equals_gfx_output_ds3(const void *a, const void *b)
{
   const uint8_t *da = a;
   const uint8_t *db = b;
   return !memcmp(da, db, sizeof(uint32_t));
}

uint32_t
hash_gfx_output(const void *key)
{
   const uint8_t *data = key;
   return _mesa_hash_data(data, offsetof(struct zink_gfx_output_key, pipeline));
}

static bool
equals_gfx_output(const void *a, const void *b)
{
   const uint8_t *da = a;
   const uint8_t *db = b;
   return !memcmp(da, db, offsetof(struct zink_gfx_output_key, pipeline));
}

ALWAYS_INLINE static void
update_gfx_program_nonseamless(struct zink_context *ctx, struct zink_gfx_program *prog, bool has_nonseamless)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (screen->driconf.inline_uniforms || prog->needs_inlining)
      update_gfx_shader_modules(ctx, screen, prog,
                                ctx->dirty_gfx_stages & prog->stages_present, &ctx->gfx_pipeline_state,
                                true, has_nonseamless);
   else
      update_gfx_shader_modules(ctx, screen, prog,
                                ctx->dirty_gfx_stages & prog->stages_present, &ctx->gfx_pipeline_state,
                                false, has_nonseamless);
}

static void
update_gfx_program(struct zink_context *ctx, struct zink_gfx_program *prog)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (screen->info.have_EXT_non_seamless_cube_map)
      update_gfx_program_nonseamless(ctx, prog, true);
   else
      update_gfx_program_nonseamless(ctx, prog, false);
}

void
zink_gfx_program_update(struct zink_context *ctx)
{
   if (ctx->last_vertex_stage_dirty) {
      gl_shader_stage pstage = ctx->last_vertex_stage->info.stage;
      ctx->dirty_gfx_stages |= BITFIELD_BIT(pstage);
      memcpy(&ctx->gfx_pipeline_state.shader_keys.key[pstage].key.vs_base,
             &ctx->gfx_pipeline_state.shader_keys.last_vertex.key.vs_base,
             sizeof(struct zink_vs_key_base));
      ctx->last_vertex_stage_dirty = false;
   }
   if (ctx->gfx_dirty) {
      struct zink_gfx_program *prog = NULL;

      simple_mtx_lock(&ctx->program_lock[zink_program_cache_stages(ctx->shader_stages)]);
      struct hash_table *ht = &ctx->program_cache[zink_program_cache_stages(ctx->shader_stages)];
      const uint32_t hash = ctx->gfx_hash;
      struct hash_entry *entry = _mesa_hash_table_search_pre_hashed(ht, hash, ctx->gfx_stages);
      /* this must be done before prog is updated */
      if (ctx->curr_program)
         ctx->gfx_pipeline_state.final_hash ^= ctx->curr_program->last_variant_hash;
      if (entry) {
         prog = (struct zink_gfx_program*)entry->data;
         for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++) {
            if (prog->stages_present & ~ctx->dirty_gfx_stages & BITFIELD_BIT(i))
               ctx->gfx_pipeline_state.modules[i] = prog->objs[i].mod;
         }
         /* ensure variants are always updated if keys have changed since last use */
         ctx->dirty_gfx_stages |= prog->stages_present;
         update_gfx_program(ctx, prog);
      } else {
         ctx->dirty_gfx_stages |= ctx->shader_stages;
         prog = zink_create_gfx_program(ctx, ctx->gfx_stages, ctx->gfx_pipeline_state.dyn_state2.vertices_per_patch, hash);
         zink_screen_get_pipeline_cache(zink_screen(ctx->base.screen), &prog->base, false);
         _mesa_hash_table_insert_pre_hashed(ht, hash, prog->shaders, prog);
         prog->base.removed = false;
         generate_gfx_program_modules(ctx, zink_screen(ctx->base.screen), prog, &ctx->gfx_pipeline_state);
      }
      simple_mtx_unlock(&ctx->program_lock[zink_program_cache_stages(ctx->shader_stages)]);
      if (prog && prog != ctx->curr_program)
         zink_batch_reference_program(&ctx->batch, &prog->base);
      ctx->curr_program = prog;
      ctx->gfx_pipeline_state.final_hash ^= ctx->curr_program->last_variant_hash;
      ctx->gfx_dirty = false;
   } else if (ctx->dirty_gfx_stages) {
      /* remove old hash */
      ctx->gfx_pipeline_state.final_hash ^= ctx->curr_program->last_variant_hash;
      update_gfx_program(ctx, ctx->curr_program);
      /* apply new hash */
      ctx->gfx_pipeline_state.final_hash ^= ctx->curr_program->last_variant_hash;
   }
   ctx->dirty_gfx_stages = 0;
}

ALWAYS_INLINE static bool
update_gfx_shader_module_optimal(struct zink_context *ctx, struct zink_gfx_program *prog, gl_shader_stage pstage)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (screen->info.have_EXT_graphics_pipeline_library)
      util_queue_fence_wait(&prog->base.cache_fence);
   struct zink_shader_module *zm = get_shader_module_for_stage_optimal(ctx, screen, prog->shaders[pstage], prog, pstage, &ctx->gfx_pipeline_state);
   if (!zm) {
      zm = create_shader_module_for_stage_optimal(ctx, screen, prog->shaders[pstage], prog, pstage, &ctx->gfx_pipeline_state);
      perf_debug(ctx, "zink[gfx_compile]: %s shader variant required\n", _mesa_shader_stage_to_string(pstage));
   }

   bool changed = prog->objs[pstage].mod != zm->obj.mod;
   prog->objs[pstage] = zm->obj;
   prog->objects[pstage] = zm->obj.obj;
   return changed;
}

static void
update_gfx_program_optimal(struct zink_context *ctx, struct zink_gfx_program *prog)
{
   const union zink_shader_key_optimal *optimal_key = (union zink_shader_key_optimal*)&prog->last_variant_hash;
   if (ctx->gfx_pipeline_state.shader_keys_optimal.key.vs_bits != optimal_key->vs_bits) {
      assert(!prog->is_separable);
      bool changed = update_gfx_shader_module_optimal(ctx, prog, ctx->last_vertex_stage->info.stage);
      ctx->gfx_pipeline_state.modules_changed |= changed;
   }
   const bool shadow_needs_shader_swizzle = optimal_key->fs.shadow_needs_shader_swizzle && (ctx->dirty_gfx_stages & BITFIELD_BIT(MESA_SHADER_FRAGMENT));
   if (ctx->gfx_pipeline_state.shader_keys_optimal.key.fs_bits != optimal_key->fs_bits ||
       /* always recheck shadow swizzles since they aren't directly part of the key */
       unlikely(shadow_needs_shader_swizzle)) {
      assert(!prog->is_separable);
      bool changed = update_gfx_shader_module_optimal(ctx, prog, MESA_SHADER_FRAGMENT);
      ctx->gfx_pipeline_state.modules_changed |= changed;
      if (unlikely(shadow_needs_shader_swizzle)) {
         struct zink_shader_module **pzm = prog->shader_cache[MESA_SHADER_FRAGMENT][0][0].data;
         ctx->gfx_pipeline_state.shadow = (struct zink_zs_swizzle_key*)pzm[0]->key + sizeof(uint16_t);
      }
   }
   if (prog->shaders[MESA_SHADER_TESS_CTRL] && prog->shaders[MESA_SHADER_TESS_CTRL]->non_fs.is_generated &&
       ctx->gfx_pipeline_state.shader_keys_optimal.key.tcs_bits != optimal_key->tcs_bits) {
      assert(!prog->is_separable);
      bool changed = update_gfx_shader_module_optimal(ctx, prog, MESA_SHADER_TESS_CTRL);
      ctx->gfx_pipeline_state.modules_changed |= changed;
   }
   prog->last_variant_hash = ctx->gfx_pipeline_state.optimal_key;
}

static struct zink_gfx_program *
replace_separable_prog(struct zink_screen *screen, struct hash_entry *entry, struct zink_gfx_program *prog)
{
   struct zink_gfx_program *real = prog->full_prog;
   entry->data = real;
   entry->key = real->shaders;
   real->base.removed = false;
   zink_gfx_program_reference(screen, &prog->full_prog, NULL);
   prog->base.removed = true;
   return real;
}

void
zink_gfx_program_update_optimal(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (ctx->gfx_dirty) {
      struct zink_gfx_program *prog = NULL;
      ctx->gfx_pipeline_state.optimal_key = zink_sanitize_optimal_key(ctx->gfx_stages, ctx->gfx_pipeline_state.shader_keys_optimal.key.val);
      struct hash_table *ht = &ctx->program_cache[zink_program_cache_stages(ctx->shader_stages)];
      const uint32_t hash = ctx->gfx_hash;
      simple_mtx_lock(&ctx->program_lock[zink_program_cache_stages(ctx->shader_stages)]);
      struct hash_entry *entry = _mesa_hash_table_search_pre_hashed(ht, hash, ctx->gfx_stages);

      if (ctx->curr_program)
         ctx->gfx_pipeline_state.final_hash ^= ctx->curr_program->last_variant_hash;
      if (entry) {
         prog = (struct zink_gfx_program*)entry->data;
         if (prog->is_separable && !(zink_debug & ZINK_DEBUG_NOOPT)) {
            /* shader variants can't be handled by separable programs: sync and compile */
            if (!ZINK_SHADER_KEY_OPTIMAL_IS_DEFAULT(ctx->gfx_pipeline_state.optimal_key))
               util_queue_fence_wait(&prog->base.cache_fence);
            /* If the optimized linked pipeline is done compiling, swap it into place. */
            if (util_queue_fence_is_signalled(&prog->base.cache_fence)) {
               prog = replace_separable_prog(screen, entry, prog);
            }
         }
         update_gfx_program_optimal(ctx, prog);
      } else {
         ctx->dirty_gfx_stages |= ctx->shader_stages;
         prog = create_gfx_program_separable(ctx, ctx->gfx_stages, ctx->gfx_pipeline_state.dyn_state2.vertices_per_patch);
         prog->base.removed = false;
         _mesa_hash_table_insert_pre_hashed(ht, hash, prog->shaders, prog);
         if (!prog->is_separable) {
            zink_screen_get_pipeline_cache(screen, &prog->base, false);
            perf_debug(ctx, "zink[gfx_compile]: new program created (probably legacy GL features in use)\n");
            generate_gfx_program_modules_optimal(ctx, screen, prog, &ctx->gfx_pipeline_state);
         }
      }
      simple_mtx_unlock(&ctx->program_lock[zink_program_cache_stages(ctx->shader_stages)]);
      if (prog && prog != ctx->curr_program)
         zink_batch_reference_program(&ctx->batch, &prog->base);
      ctx->curr_program = prog;
      ctx->gfx_pipeline_state.final_hash ^= ctx->curr_program->last_variant_hash;
   } else if (ctx->dirty_gfx_stages) {
      /* remove old hash */
      ctx->gfx_pipeline_state.optimal_key = zink_sanitize_optimal_key(ctx->gfx_stages, ctx->gfx_pipeline_state.shader_keys_optimal.key.val);
      ctx->gfx_pipeline_state.final_hash ^= ctx->curr_program->last_variant_hash;
      if (ctx->curr_program->is_separable && !(zink_debug & ZINK_DEBUG_NOOPT)) {
         struct zink_gfx_program *prog = ctx->curr_program;
         if (!ZINK_SHADER_KEY_OPTIMAL_IS_DEFAULT(ctx->gfx_pipeline_state.optimal_key)) {
            util_queue_fence_wait(&prog->base.cache_fence);
            /* shader variants can't be handled by separable programs: sync and compile */
            perf_debug(ctx, "zink[gfx_compile]: non-default shader variant required with separate shader object program\n");
            struct hash_table *ht = &ctx->program_cache[zink_program_cache_stages(ctx->shader_stages)];
            const uint32_t hash = ctx->gfx_hash;
            simple_mtx_lock(&ctx->program_lock[zink_program_cache_stages(ctx->shader_stages)]);
            struct hash_entry *entry = _mesa_hash_table_search_pre_hashed(ht, hash, ctx->gfx_stages);
            ctx->curr_program = replace_separable_prog(screen, entry, prog);
            simple_mtx_unlock(&ctx->program_lock[zink_program_cache_stages(ctx->shader_stages)]);
         }
      }
      update_gfx_program_optimal(ctx, ctx->curr_program);
      /* apply new hash */
      ctx->gfx_pipeline_state.final_hash ^= ctx->curr_program->last_variant_hash;
   }
   ctx->dirty_gfx_stages = 0;
   ctx->gfx_dirty = false;
   ctx->last_vertex_stage_dirty = false;
}

static void
optimized_compile_job(void *data, void *gdata, int thread_index)
{
   struct zink_gfx_pipeline_cache_entry *pc_entry = data;
   struct zink_screen *screen = gdata;
   VkPipeline pipeline;
   if (pc_entry->gpl.gkey)
      pipeline = zink_create_gfx_pipeline_combined(screen, pc_entry->prog, pc_entry->gpl.ikey->pipeline, &pc_entry->gpl.gkey->pipeline, 1, pc_entry->gpl.okey->pipeline, true, false);
   else
      pipeline = zink_create_gfx_pipeline(screen, pc_entry->prog, pc_entry->prog->objs, &pc_entry->state, pc_entry->state.element_state->binding_map, zink_primitive_topology(pc_entry->state.gfx_prim_mode), true, NULL);
   if (pipeline) {
      pc_entry->gpl.unoptimized_pipeline = pc_entry->pipeline;
      pc_entry->pipeline = pipeline;
   }
}

static void
optimized_shobj_compile_job(void *data, void *gdata, int thread_index)
{
   struct zink_gfx_pipeline_cache_entry *pc_entry = data;
   struct zink_screen *screen = gdata;

   struct zink_shader_object objs[ZINK_GFX_SHADER_COUNT];
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++) {
      objs[i].mod = VK_NULL_HANDLE;
      objs[i].spirv = pc_entry->shobjs[i].spirv;
   }
   pc_entry->pipeline = zink_create_gfx_pipeline(screen, pc_entry->prog, objs, &pc_entry->state, NULL, zink_primitive_topology(pc_entry->state.gfx_prim_mode), true, NULL);
   /* no unoptimized_pipeline dance */
}

void
zink_gfx_program_compile_queue(struct zink_context *ctx, struct zink_gfx_pipeline_cache_entry *pc_entry)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (screen->driver_workarounds.disable_optimized_compile)
      return;
   if (zink_debug & ZINK_DEBUG_NOBGC) {
      if (pc_entry->prog->base.uses_shobj)
         optimized_shobj_compile_job(pc_entry, screen, 0);
      else
         optimized_compile_job(pc_entry, screen, 0);
   } else {
      util_queue_add_job(&screen->cache_get_thread, pc_entry, &pc_entry->fence,
                         pc_entry->prog->base.uses_shobj ? optimized_shobj_compile_job : optimized_compile_job, NULL, 0);
   }
}

static void
update_cs_shader_module(struct zink_context *ctx, struct zink_compute_program *comp)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_shader *zs = comp->shader;
   struct zink_shader_module *zm = NULL;
   unsigned inline_size = 0, nonseamless_size = 0, zs_swizzle_size = 0;
   struct zink_shader_key *key = &ctx->compute_pipeline_state.key;
   ASSERTED bool check_robustness = screen->driver_workarounds.lower_robustImageAccess2 && (ctx->flags & PIPE_CONTEXT_ROBUST_BUFFER_ACCESS);
   assert(zink_cs_key(key)->robust_access == check_robustness);

   if (ctx && zs->info.num_inlinable_uniforms &&
       ctx->inlinable_uniforms_valid_mask & BITFIELD64_BIT(MESA_SHADER_COMPUTE)) {
      if (screen->is_cpu || comp->inlined_variant_count < ZINK_MAX_INLINED_VARIANTS)
         inline_size = zs->info.num_inlinable_uniforms;
      else
         key->inline_uniforms = false;
   }
   if (key->base.nonseamless_cube_mask)
      nonseamless_size = sizeof(uint32_t);
   if (key->base.needs_zs_shader_swizzle)
      zs_swizzle_size = sizeof(struct zink_zs_swizzle_key);

   if (inline_size || nonseamless_size || zink_cs_key(key)->robust_access || zs_swizzle_size) {
      struct util_dynarray *shader_cache = &comp->shader_cache[!!nonseamless_size];
      unsigned count = util_dynarray_num_elements(shader_cache, struct zink_shader_module *);
      struct zink_shader_module **pzm = shader_cache->data;
      for (unsigned i = 0; i < count; i++) {
         struct zink_shader_module *iter = pzm[i];
         if (!shader_key_matches(iter, key, inline_size,
                                 screen->driconf.inline_uniforms,
                                 screen->info.have_EXT_non_seamless_cube_map))
            continue;
         if (unlikely(zs_swizzle_size)) {
            /* zs swizzle data needs a manual compare since it's so fat */
            if (memcmp(iter->key + iter->key_size + nonseamless_size + inline_size * sizeof(uint32_t),
                       &ctx->di.zs_swizzle[MESA_SHADER_COMPUTE], zs_swizzle_size))
               continue;
         }
         if (i > 0) {
            struct zink_shader_module *zero = pzm[0];
            pzm[0] = iter;
            pzm[i] = zero;
         }
         zm = iter;
      }
   } else {
      zm = comp->module;
   }

   if (!zm) {
      zm = malloc(sizeof(struct zink_shader_module) + nonseamless_size + inline_size * sizeof(uint32_t) + zs_swizzle_size);
      if (!zm) {
         return;
      }
      zm->shobj = false;
      zm->obj = zink_shader_compile(screen, false, zs, zink_shader_blob_deserialize(screen, &comp->shader->blob), key, zs_swizzle_size ? &ctx->di.zs_swizzle[MESA_SHADER_COMPUTE] : NULL, &comp->base);
      if (!zm->obj.spirv) {
         FREE(zm);
         return;
      }
      zm->num_uniforms = inline_size;
      zm->key_size = key->size;
      memcpy(zm->key, key, key->size);
      zm->has_nonseamless = !!nonseamless_size;
      zm->needs_zs_shader_swizzle = !!zs_swizzle_size;
      assert(nonseamless_size || inline_size || zink_cs_key(key)->robust_access || zs_swizzle_size);
      if (nonseamless_size)
         memcpy(zm->key + zm->key_size, &key->base.nonseamless_cube_mask, nonseamless_size);
      if (inline_size)
         memcpy(zm->key + zm->key_size + nonseamless_size, key->base.inlined_uniform_values, inline_size * sizeof(uint32_t));
      if (zs_swizzle_size)
         memcpy(zm->key + zm->key_size + nonseamless_size + inline_size * sizeof(uint32_t), &ctx->di.zs_swizzle[MESA_SHADER_COMPUTE], zs_swizzle_size);

      zm->hash = shader_module_hash(zm);
      zm->default_variant = false;
      if (inline_size)
         comp->inlined_variant_count++;

      /* this is otherwise the default variant, which is stored as comp->module */
      if (zm->num_uniforms || nonseamless_size || zink_cs_key(key)->robust_access || zs_swizzle_size)
         util_dynarray_append(&comp->shader_cache[!!nonseamless_size], void*, zm);
   }
   if (comp->curr == zm)
      return;
   ctx->compute_pipeline_state.final_hash ^= ctx->compute_pipeline_state.module_hash;
   comp->curr = zm;
   ctx->compute_pipeline_state.module_hash = zm->hash;
   ctx->compute_pipeline_state.final_hash ^= ctx->compute_pipeline_state.module_hash;
   ctx->compute_pipeline_state.module_changed = true;
}

void
zink_update_compute_program(struct zink_context *ctx)
{
   util_queue_fence_wait(&ctx->curr_compute->base.cache_fence);
   update_cs_shader_module(ctx, ctx->curr_compute);
}

VkPipelineLayout
zink_pipeline_layout_create(struct zink_screen *screen, VkDescriptorSetLayout *dsl, unsigned num_dsl, bool is_compute, VkPipelineLayoutCreateFlags flags)
{
   VkPipelineLayoutCreateInfo plci = {0};
   plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   plci.flags = flags;

   plci.pSetLayouts = dsl;
   plci.setLayoutCount = num_dsl;

   VkPushConstantRange pcr;
   if (!is_compute) {
      pcr.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
      pcr.offset = 0;
      pcr.size = sizeof(struct zink_gfx_push_constant);
      plci.pushConstantRangeCount = 1;
      plci.pPushConstantRanges = &pcr;
   }

   VkPipelineLayout layout;
   VkResult result = VKSCR(CreatePipelineLayout)(screen->dev, &plci, NULL, &layout);
   if (result != VK_SUCCESS) {
      mesa_loge("vkCreatePipelineLayout failed (%s)", vk_Result_to_str(result));
      return VK_NULL_HANDLE;
   }

   return layout;
}

static void *
create_program(struct zink_context *ctx, bool is_compute)
{
   struct zink_program *pg = rzalloc_size(NULL, is_compute ? sizeof(struct zink_compute_program) : sizeof(struct zink_gfx_program));
   if (!pg)
      return NULL;

   pipe_reference_init(&pg->reference, 1);
   u_rwlock_init(&pg->pipeline_cache_lock);
   util_queue_fence_init(&pg->cache_fence);
   pg->is_compute = is_compute;
   pg->ctx = ctx;
   return (void*)pg;
}

static void
assign_io(struct zink_screen *screen,
          nir_shader *shaders[ZINK_GFX_SHADER_COUNT])
{
   for (unsigned i = 0; i < MESA_SHADER_FRAGMENT;) {
      nir_shader *producer = shaders[i];
      for (unsigned j = i + 1; j < ZINK_GFX_SHADER_COUNT; i++, j++) {
         nir_shader *consumer = shaders[j];
         if (!consumer)
            continue;
         zink_compiler_assign_io(screen, producer, consumer);
         i = j;
         break;
      }
   }
}

void
zink_gfx_lib_cache_unref(struct zink_screen *screen, struct zink_gfx_lib_cache *libs)
{
   if (!p_atomic_dec_zero(&libs->refcount))
      return;

   simple_mtx_destroy(&libs->lock);
   set_foreach_remove(&libs->libs, he) {
      struct zink_gfx_library_key *gkey = (void*)he->key;
      VKSCR(DestroyPipeline)(screen->dev, gkey->pipeline, NULL);
      FREE(gkey);
   }
   ralloc_free(libs->libs.table);
   FREE(libs);
}

static struct zink_gfx_lib_cache *
create_lib_cache(struct zink_gfx_program *prog, bool generated_tcs)
{
   struct zink_gfx_lib_cache *libs = CALLOC_STRUCT(zink_gfx_lib_cache);
   libs->stages_present = prog->stages_present;
   if (generated_tcs)
      libs->stages_present &= ~BITFIELD_BIT(MESA_SHADER_TESS_CTRL);
   simple_mtx_init(&libs->lock, mtx_plain);
   if (generated_tcs)
      _mesa_set_init(&libs->libs, NULL, hash_pipeline_lib_generated_tcs, equals_pipeline_lib_generated_tcs);
   else
      _mesa_set_init(&libs->libs, NULL, hash_pipeline_lib, equals_pipeline_lib);
   return libs;
}

static struct zink_gfx_lib_cache *
find_or_create_lib_cache(struct zink_screen *screen, struct zink_gfx_program *prog)
{
   unsigned stages_present = prog->stages_present;
   bool generated_tcs = prog->shaders[MESA_SHADER_TESS_CTRL] && prog->shaders[MESA_SHADER_TESS_CTRL]->non_fs.is_generated;
   if (generated_tcs)
      stages_present &= ~BITFIELD_BIT(MESA_SHADER_TESS_CTRL);
   unsigned idx = zink_program_cache_stages(stages_present);
   struct set *ht = &screen->pipeline_libs[idx];
   const uint32_t hash = prog->gfx_hash;

   simple_mtx_lock(&screen->pipeline_libs_lock[idx]);
   bool found = false;
   struct set_entry *entry = _mesa_set_search_or_add_pre_hashed(ht, hash, prog->shaders, &found);
   struct zink_gfx_lib_cache *libs;
   if (found) {
      libs = (void*)entry->key;
   } else {
      libs = create_lib_cache(prog, generated_tcs);
      memcpy(libs->shaders, prog->shaders, sizeof(prog->shaders));
      entry->key = libs;
      unsigned refs = 0;
      for (unsigned i = 0; i < MESA_SHADER_COMPUTE; i++) {
         if (prog->shaders[i] && (!generated_tcs || i != MESA_SHADER_TESS_CTRL)) {
            simple_mtx_lock(&prog->shaders[i]->lock);
            util_dynarray_append(&prog->shaders[i]->pipeline_libs, struct zink_gfx_lib_cache*, libs);
            simple_mtx_unlock(&prog->shaders[i]->lock);
            refs++;
         }
      }
      p_atomic_set(&libs->refcount, refs);
   }
   simple_mtx_unlock(&screen->pipeline_libs_lock[idx]);
   return libs;
}

struct zink_gfx_program *
zink_create_gfx_program(struct zink_context *ctx,
                        struct zink_shader **stages,
                        unsigned vertices_per_patch,
                        uint32_t gfx_hash)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_gfx_program *prog = create_program(ctx, false);
   if (!prog)
      goto fail;

   prog->ctx = ctx;
   prog->gfx_hash = gfx_hash;
   prog->base.removed = true;
   prog->optimal_keys = screen->optimal_keys;

   nir_shader *nir[ZINK_GFX_SHADER_COUNT];

   prog->has_edgeflags = prog->shaders[MESA_SHADER_VERTEX] &&
                         prog->shaders[MESA_SHADER_VERTEX]->has_edgeflags;
   for (int i = 0; i < ZINK_GFX_SHADER_COUNT; ++i) {
      util_dynarray_init(&prog->shader_cache[i][0][0], prog);
      util_dynarray_init(&prog->shader_cache[i][0][1], prog);
      util_dynarray_init(&prog->shader_cache[i][1][0], prog);
      util_dynarray_init(&prog->shader_cache[i][1][1], prog);
      if (stages[i]) {
         prog->shaders[i] = stages[i];
         prog->stages_present |= BITFIELD_BIT(i);
         if (i != MESA_SHADER_FRAGMENT)
            prog->optimal_keys &= !prog->shaders[i]->non_fs.is_generated;
         prog->needs_inlining |= prog->shaders[i]->needs_inlining;
         nir[i] = zink_shader_deserialize(screen, stages[i]);
      } else {
         nir[i] = NULL;
      }
   }
   if (stages[MESA_SHADER_TESS_EVAL] && !stages[MESA_SHADER_TESS_CTRL]) {
      prog->shaders[MESA_SHADER_TESS_EVAL]->non_fs.generated_tcs =
      prog->shaders[MESA_SHADER_TESS_CTRL] =
        zink_shader_tcs_create(screen, nir[MESA_SHADER_TESS_EVAL], vertices_per_patch, &nir[MESA_SHADER_TESS_CTRL]);
      prog->stages_present |= BITFIELD_BIT(MESA_SHADER_TESS_CTRL);
   }
   prog->stages_remaining = prog->stages_present;

   assign_io(screen, nir);
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++) {
      if (nir[i])
         zink_shader_serialize_blob(nir[i], &prog->blobs[i]);
      ralloc_free(nir[i]);
   }


   if (stages[MESA_SHADER_GEOMETRY])
      prog->last_vertex_stage = stages[MESA_SHADER_GEOMETRY];
   else if (stages[MESA_SHADER_TESS_EVAL])
      prog->last_vertex_stage = stages[MESA_SHADER_TESS_EVAL];
   else
      prog->last_vertex_stage = stages[MESA_SHADER_VERTEX];

   for (int r = 0; r < ARRAY_SIZE(prog->pipelines); ++r) {
      for (int i = 0; i < ARRAY_SIZE(prog->pipelines[0]); ++i) {
         _mesa_hash_table_init(&prog->pipelines[r][i], prog, NULL, zink_get_gfx_pipeline_eq_func(screen, prog));
         /* only need first 3/4 for point/line/tri/patch */
         if (screen->info.have_EXT_extended_dynamic_state &&
             i == (prog->last_vertex_stage->info.stage == MESA_SHADER_TESS_EVAL ? 4 : 3))
            break;
      }
   }

   if (screen->optimal_keys)
      prog->libs = find_or_create_lib_cache(screen, prog);
   if (prog->libs)
      p_atomic_inc(&prog->libs->refcount);

   struct mesa_sha1 sctx;
   _mesa_sha1_init(&sctx);
   for (int i = 0; i < ZINK_GFX_SHADER_COUNT; ++i) {
      if (prog->shaders[i]) {
         simple_mtx_lock(&prog->shaders[i]->lock);
         _mesa_set_add(prog->shaders[i]->programs, prog);
         simple_mtx_unlock(&prog->shaders[i]->lock);
         zink_gfx_program_reference(screen, NULL, prog);
         _mesa_sha1_update(&sctx, prog->shaders[i]->base.sha1, sizeof(prog->shaders[i]->base.sha1));
      }
   }
   _mesa_sha1_final(&sctx, prog->base.sha1);
   p_atomic_dec(&prog->base.reference.count);

   if (!zink_descriptor_program_init(ctx, &prog->base))
      goto fail;

   return prog;

fail:
   if (prog)
      zink_destroy_gfx_program(screen, prog);
   return NULL;
}

/* Creates a replacement, optimized zink_gfx_program for this set of separate shaders, which will
 * be swapped in in place of the fast-linked separable program once it's done compiling.
 */
static void
create_linked_separable_job(void *data, void *gdata, int thread_index)
{
   struct zink_gfx_program *prog = data;
   prog->full_prog = zink_create_gfx_program(prog->ctx, prog->shaders, 0, prog->gfx_hash);
   /* add an ownership ref */
   zink_gfx_program_reference(zink_screen(prog->ctx->base.screen), NULL, prog->full_prog);
   precompile_job(prog->full_prog, gdata, thread_index);
}

struct zink_gfx_program *
create_gfx_program_separable(struct zink_context *ctx, struct zink_shader **stages, unsigned vertices_per_patch)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   bool is_separate = true;
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++)
      is_separate &= !stages[i] || stages[i]->info.separate_shader;
   /* filter cases that need real pipelines */
   if (!is_separate ||
       /* TODO: maybe try variants? grimace */
       !ZINK_SHADER_KEY_OPTIMAL_IS_DEFAULT(ctx->gfx_pipeline_state.optimal_key) ||
       !zink_can_use_pipeline_libs(ctx))
      return zink_create_gfx_program(ctx, stages, vertices_per_patch, ctx->gfx_hash);
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++) {
      /* ensure async shader creation is done */
      if (stages[i]) {
         util_queue_fence_wait(&stages[i]->precompile.fence);
         if (!stages[i]->precompile.obj.mod)
            return zink_create_gfx_program(ctx, stages, vertices_per_patch, ctx->gfx_hash);
      }
   }

   struct zink_gfx_program *prog = create_program(ctx, false);
   if (!prog)
      goto fail;

   prog->ctx = ctx;
   prog->is_separable = true;
   prog->gfx_hash = ctx->gfx_hash;
   prog->base.uses_shobj = screen->info.have_EXT_shader_object;

   prog->stages_remaining = prog->stages_present = ctx->shader_stages;
   memcpy(prog->shaders, stages, sizeof(prog->shaders));
   prog->last_vertex_stage = ctx->last_vertex_stage;

   if (stages[MESA_SHADER_TESS_EVAL] && !stages[MESA_SHADER_TESS_CTRL]) {
      prog->shaders[MESA_SHADER_TESS_CTRL] = stages[MESA_SHADER_TESS_EVAL]->non_fs.generated_tcs;
      prog->stages_present |= BITFIELD_BIT(MESA_SHADER_TESS_CTRL);
   }

   if (!screen->info.have_EXT_shader_object) {
      prog->libs = create_lib_cache(prog, false);
      /* this libs cache is owned by the program */
      p_atomic_set(&prog->libs->refcount, 1);
   }

   unsigned refs = 0;
   for (int i = 0; i < ZINK_GFX_SHADER_COUNT; ++i) {
      if (prog->shaders[i]) {
         simple_mtx_lock(&prog->shaders[i]->lock);
         _mesa_set_add(prog->shaders[i]->programs, prog);
         simple_mtx_unlock(&prog->shaders[i]->lock);
         if (screen->info.have_EXT_shader_object) {
            if (!prog->objects[i])
               prog->objects[i] = prog->shaders[i]->precompile.obj.obj;
         }
         refs++;
      }
   }
   /* We can do this add after the _mesa_set_adds above because we know the prog->shaders[] are 
   * referenced by the draw state and zink_gfx_shader_free() can't be called on them while we're in here.
   */
   p_atomic_add(&prog->base.reference.count, refs - 1);

   for (int r = 0; r < ARRAY_SIZE(prog->pipelines); ++r) {
      for (int i = 0; i < ARRAY_SIZE(prog->pipelines[0]); ++i) {
         _mesa_hash_table_init(&prog->pipelines[r][i], prog, NULL, zink_get_gfx_pipeline_eq_func(screen, prog));
         /* only need first 3/4 for point/line/tri/patch */
         if (screen->info.have_EXT_extended_dynamic_state &&
             i == (prog->last_vertex_stage->info.stage == MESA_SHADER_TESS_EVAL ? 4 : 3))
            break;
      }
   }

   for (int i = 0; i < ZINK_GFX_SHADER_COUNT; ++i) {
      if (!prog->shaders[i] || !prog->shaders[i]->precompile.dsl)
         continue;
      int idx = !i ? 0 : screen->info.have_EXT_shader_object ? i : 1;
      prog->base.dd.binding_usage |= BITFIELD_BIT(idx);
      prog->base.dsl[idx] = prog->shaders[i]->precompile.dsl;
      /* guarantee a null dsl if previous stages don't have descriptors */
      if (prog->shaders[i]->precompile.dsl)
         prog->base.num_dsl = idx + 1;
      prog->base.dd.bindless |= prog->shaders[i]->bindless;
   }
   if (prog->base.dd.bindless) {
      prog->base.num_dsl = screen->compact_descriptors ? ZINK_DESCRIPTOR_ALL_TYPES - ZINK_DESCRIPTOR_COMPACT : ZINK_DESCRIPTOR_ALL_TYPES;
      prog->base.dsl[screen->desc_set_id[ZINK_DESCRIPTOR_BINDLESS]] = screen->bindless_layout;
   }
   prog->base.layout = zink_pipeline_layout_create(screen, prog->base.dsl, prog->base.num_dsl, false, VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

   prog->last_variant_hash = ctx->gfx_pipeline_state.optimal_key;

   if (!screen->info.have_EXT_shader_object) {
      VkPipeline libs[] = {stages[MESA_SHADER_VERTEX]->precompile.gpl, stages[MESA_SHADER_FRAGMENT]->precompile.gpl};
      struct zink_gfx_library_key *gkey = CALLOC_STRUCT(zink_gfx_library_key);
      if (!gkey) {
         mesa_loge("ZINK: failed to allocate gkey!");
         goto fail;
      }
      gkey->optimal_key = prog->last_variant_hash;
      assert(gkey->optimal_key);
      gkey->pipeline = zink_create_gfx_pipeline_combined(screen, prog, VK_NULL_HANDLE, libs, 2, VK_NULL_HANDLE, false, false);
      _mesa_set_add(&prog->libs->libs, gkey);
   }

   if (!(zink_debug & ZINK_DEBUG_NOOPT))
      util_queue_add_job(&screen->cache_get_thread, prog, &prog->base.cache_fence, create_linked_separable_job, NULL, 0);

   return prog;
fail:
   if (prog)
      zink_destroy_gfx_program(screen, prog);
   return NULL;
}

static uint32_t
hash_compute_pipeline_state_local_size(const void *key)
{
   const struct zink_compute_pipeline_state *state = key;
   uint32_t hash = _mesa_hash_data(state, offsetof(struct zink_compute_pipeline_state, hash));
   hash = XXH32(&state->local_size[0], sizeof(state->local_size), hash);
   return hash;
}

static uint32_t
hash_compute_pipeline_state(const void *key)
{
   const struct zink_compute_pipeline_state *state = key;
   return _mesa_hash_data(state, offsetof(struct zink_compute_pipeline_state, hash));
}

void
zink_program_update_compute_pipeline_state(struct zink_context *ctx, struct zink_compute_program *comp, const struct pipe_grid_info *info)
{
   if (comp->use_local_size) {
      for (int i = 0; i < ARRAY_SIZE(ctx->compute_pipeline_state.local_size); i++) {
         if (ctx->compute_pipeline_state.local_size[i] != info->block[i])
            ctx->compute_pipeline_state.dirty = true;
         ctx->compute_pipeline_state.local_size[i] = info->block[i];
      }
   }
   if (ctx->compute_pipeline_state.variable_shared_mem != info->variable_shared_mem) {
      ctx->compute_pipeline_state.dirty = true;
      ctx->compute_pipeline_state.variable_shared_mem = info->variable_shared_mem;
   }
}

static bool
equals_compute_pipeline_state(const void *a, const void *b)
{
   const struct zink_compute_pipeline_state *sa = a;
   const struct zink_compute_pipeline_state *sb = b;
   return !memcmp(a, b, offsetof(struct zink_compute_pipeline_state, hash)) &&
          sa->module == sb->module;
}

static bool
equals_compute_pipeline_state_local_size(const void *a, const void *b)
{
   const struct zink_compute_pipeline_state *sa = a;
   const struct zink_compute_pipeline_state *sb = b;
   return !memcmp(a, b, offsetof(struct zink_compute_pipeline_state, hash)) &&
          !memcmp(sa->local_size, sb->local_size, sizeof(sa->local_size)) &&
          sa->module == sb->module;
}

static void
precompile_compute_job(void *data, void *gdata, int thread_index)
{
   struct zink_compute_program *comp = data;
   struct zink_screen *screen = gdata;

   comp->shader = zink_shader_create(screen, comp->nir);
   comp->curr = comp->module = CALLOC_STRUCT(zink_shader_module);
   assert(comp->module);
   comp->module->shobj = false;
   comp->module->obj = zink_shader_compile(screen, false, comp->shader, comp->nir, NULL, NULL, &comp->base);
   /* comp->nir will be freed by zink_shader_compile */
   comp->nir = NULL;
   assert(comp->module->obj.spirv);
   util_dynarray_init(&comp->shader_cache[0], comp);
   util_dynarray_init(&comp->shader_cache[1], comp);

   struct mesa_sha1 sha1_ctx;
   _mesa_sha1_init(&sha1_ctx);
   _mesa_sha1_update(&sha1_ctx, comp->shader->blob.data, comp->shader->blob.size);
   _mesa_sha1_final(&sha1_ctx, comp->base.sha1);

   zink_descriptor_program_init(comp->base.ctx, &comp->base);

   zink_screen_get_pipeline_cache(screen, &comp->base, true);
   if (comp->base.can_precompile)
      comp->base_pipeline = zink_create_compute_pipeline(screen, comp, NULL);
   if (comp->base_pipeline)
      zink_screen_update_pipeline_cache(screen, &comp->base, true);
}

static struct zink_compute_program *
create_compute_program(struct zink_context *ctx, nir_shader *nir)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_compute_program *comp = create_program(ctx, true);
   if (!comp)
      return NULL;
   simple_mtx_init(&comp->cache_lock, mtx_plain);
   comp->scratch_size = nir->scratch_size;
   comp->nir = nir;
   comp->num_inlinable_uniforms = nir->info.num_inlinable_uniforms;

   comp->use_local_size = !(nir->info.workgroup_size[0] ||
                            nir->info.workgroup_size[1] ||
                            nir->info.workgroup_size[2]);
   comp->has_variable_shared_mem = nir->info.cs.has_variable_shared_mem;
   comp->base.can_precompile = !comp->use_local_size &&
                               (screen->info.have_EXT_non_seamless_cube_map || !zink_shader_has_cubes(nir)) &&
                               (screen->info.rb2_feats.robustImageAccess2 || !(ctx->flags & PIPE_CONTEXT_ROBUST_BUFFER_ACCESS));
   _mesa_hash_table_init(&comp->pipelines, comp, NULL, comp->use_local_size ?
                                                       equals_compute_pipeline_state_local_size :
                                                       equals_compute_pipeline_state);
   if (zink_debug & ZINK_DEBUG_NOBGC)
      precompile_compute_job(comp, screen, 0);
   else
      util_queue_add_job(&screen->cache_get_thread, comp, &comp->base.cache_fence,
                        precompile_compute_job, NULL, 0);
   return comp;
}

uint32_t
zink_program_get_descriptor_usage(struct zink_context *ctx, gl_shader_stage stage, enum zink_descriptor_type type)
{
   struct zink_shader *zs = NULL;
   switch (stage) {
   case MESA_SHADER_VERTEX:
   case MESA_SHADER_TESS_CTRL:
   case MESA_SHADER_TESS_EVAL:
   case MESA_SHADER_GEOMETRY:
   case MESA_SHADER_FRAGMENT:
      zs = ctx->gfx_stages[stage];
      break;
   case MESA_SHADER_COMPUTE: {
      zs = ctx->curr_compute->shader;
      break;
   }
   default:
      unreachable("unknown shader type");
   }
   if (!zs)
      return 0;
   switch (type) {
   case ZINK_DESCRIPTOR_TYPE_UBO:
      return zs->ubos_used;
   case ZINK_DESCRIPTOR_TYPE_SSBO:
      return zs->ssbos_used;
   case ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW:
      return BITSET_TEST_RANGE(zs->info.textures_used, 0, PIPE_MAX_SAMPLERS - 1);
   case ZINK_DESCRIPTOR_TYPE_IMAGE:
      return BITSET_TEST_RANGE(zs->info.images_used, 0, PIPE_MAX_SAMPLERS - 1);
   default:
      unreachable("unknown descriptor type!");
   }
   return 0;
}

bool
zink_program_descriptor_is_buffer(struct zink_context *ctx, gl_shader_stage stage, enum zink_descriptor_type type, unsigned i)
{
   struct zink_shader *zs = NULL;
   switch (stage) {
   case MESA_SHADER_VERTEX:
   case MESA_SHADER_TESS_CTRL:
   case MESA_SHADER_TESS_EVAL:
   case MESA_SHADER_GEOMETRY:
   case MESA_SHADER_FRAGMENT:
      zs = ctx->gfx_stages[stage];
      break;
   case MESA_SHADER_COMPUTE: {
      zs = ctx->curr_compute->shader;
      break;
   }
   default:
      unreachable("unknown shader type");
   }
   if (!zs)
      return false;
   return zink_shader_descriptor_is_buffer(zs, type, i);
}

static unsigned
get_num_bindings(struct zink_shader *zs, enum zink_descriptor_type type)
{
   switch (type) {
   case ZINK_DESCRIPTOR_TYPE_UNIFORMS:
      return !!zs->has_uniforms;
   case ZINK_DESCRIPTOR_TYPE_UBO:
   case ZINK_DESCRIPTOR_TYPE_SSBO:
      return zs->num_bindings[type];
   default:
      break;
   }
   unsigned num_bindings = 0;
   for (int i = 0; i < zs->num_bindings[type]; i++)
      num_bindings += zs->bindings[type][i].size;
   return num_bindings;
}

unsigned
zink_program_num_bindings_typed(const struct zink_program *pg, enum zink_descriptor_type type)
{
   unsigned num_bindings = 0;
   if (pg->is_compute) {
      struct zink_compute_program *comp = (void*)pg;
      return get_num_bindings(comp->shader, type);
   }
   struct zink_gfx_program *prog = (void*)pg;
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++) {
      if (prog->shaders[i])
         num_bindings += get_num_bindings(prog->shaders[i], type);
   }
   return num_bindings;
}

unsigned
zink_program_num_bindings(const struct zink_program *pg)
{
   unsigned num_bindings = 0;
   for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++)
      num_bindings += zink_program_num_bindings_typed(pg, i);
   return num_bindings;
}

static void
deinit_program(struct zink_screen *screen, struct zink_program *pg)
{
   util_queue_fence_wait(&pg->cache_fence);
   if (pg->layout)
      VKSCR(DestroyPipelineLayout)(screen->dev, pg->layout, NULL);

   if (pg->pipeline_cache)
      VKSCR(DestroyPipelineCache)(screen->dev, pg->pipeline_cache, NULL);
   u_rwlock_destroy(&pg->pipeline_cache_lock);
   zink_descriptor_program_deinit(screen, pg);
}

void
zink_destroy_gfx_program(struct zink_screen *screen,
                         struct zink_gfx_program *prog)
{
   unsigned max_idx = ARRAY_SIZE(prog->pipelines[0]);
   if (screen->info.have_EXT_extended_dynamic_state) {
      /* only need first 3/4 for point/line/tri/patch */
      if ((prog->stages_present &
          (BITFIELD_BIT(MESA_SHADER_TESS_EVAL) | BITFIELD_BIT(MESA_SHADER_GEOMETRY))) ==
          BITFIELD_BIT(MESA_SHADER_TESS_EVAL))
         max_idx = 4;
      else
         max_idx = 3;
      max_idx++;
   }

   if (prog->is_separable)
      zink_gfx_program_reference(screen, &prog->full_prog, NULL);
   for (unsigned r = 0; r < ARRAY_SIZE(prog->pipelines); r++) {
      for (int i = 0; i < max_idx; ++i) {
         hash_table_foreach(&prog->pipelines[r][i], entry) {
            struct zink_gfx_pipeline_cache_entry *pc_entry = entry->data;

            util_queue_fence_wait(&pc_entry->fence);
            VKSCR(DestroyPipeline)(screen->dev, pc_entry->pipeline, NULL);
            VKSCR(DestroyPipeline)(screen->dev, pc_entry->gpl.unoptimized_pipeline, NULL);
            free(pc_entry);
         }
      }
   }

   deinit_program(screen, &prog->base);

   for (int i = 0; i < ZINK_GFX_SHADER_COUNT; ++i) {
      if (prog->shaders[i]) {
         _mesa_set_remove_key(prog->shaders[i]->programs, prog);
         prog->shaders[i] = NULL;
      }
      if (!prog->is_separable) {
         destroy_shader_cache(screen, &prog->shader_cache[i][0][0]);
         destroy_shader_cache(screen, &prog->shader_cache[i][0][1]);
         destroy_shader_cache(screen, &prog->shader_cache[i][1][0]);
         destroy_shader_cache(screen, &prog->shader_cache[i][1][1]);
         blob_finish(&prog->blobs[i]);
      }
   }
   if (prog->libs)
      zink_gfx_lib_cache_unref(screen, prog->libs);

   ralloc_free(prog);
}

void
zink_destroy_compute_program(struct zink_screen *screen,
                             struct zink_compute_program *comp)
{
   deinit_program(screen, &comp->base);

   assert(comp->shader);
   assert(!comp->shader->spirv);

   zink_shader_free(screen, comp->shader);

   destroy_shader_cache(screen, &comp->shader_cache[0]);
   destroy_shader_cache(screen, &comp->shader_cache[1]);

   hash_table_foreach(&comp->pipelines, entry) {
      struct compute_pipeline_cache_entry *pc_entry = entry->data;

      VKSCR(DestroyPipeline)(screen->dev, pc_entry->pipeline, NULL);
      free(pc_entry);
   }
   VKSCR(DestroyPipeline)(screen->dev, comp->base_pipeline, NULL);
   zink_destroy_shader_module(screen, comp->module);

   ralloc_free(comp);
}

ALWAYS_INLINE static bool
compute_can_shortcut(const struct zink_compute_program *comp)
{
   return !comp->use_local_size && !comp->curr->num_uniforms && !comp->curr->has_nonseamless;
}

VkPipeline
zink_get_compute_pipeline(struct zink_screen *screen,
                      struct zink_compute_program *comp,
                      struct zink_compute_pipeline_state *state)
{
   struct hash_entry *entry = NULL;
   struct compute_pipeline_cache_entry *cache_entry;

   if (!state->dirty && !state->module_changed)
      return state->pipeline;
   if (state->dirty) {
      if (state->pipeline) //avoid on first hash
         state->final_hash ^= state->hash;
      if (comp->use_local_size)
         state->hash = hash_compute_pipeline_state_local_size(state);
      else
         state->hash = hash_compute_pipeline_state(state);
      state->dirty = false;
      state->final_hash ^= state->hash;
   }

   util_queue_fence_wait(&comp->base.cache_fence);
   if (comp->base_pipeline && compute_can_shortcut(comp)) {
      state->pipeline = comp->base_pipeline;
      return state->pipeline;
   }
   entry = _mesa_hash_table_search_pre_hashed(&comp->pipelines, state->final_hash, state);

   if (!entry) {
      simple_mtx_lock(&comp->cache_lock);
      entry = _mesa_hash_table_search_pre_hashed(&comp->pipelines, state->final_hash, state);
      if (entry) {
         simple_mtx_unlock(&comp->cache_lock);
         goto out;
      }
      VkPipeline pipeline = zink_create_compute_pipeline(screen, comp, state);

      if (pipeline == VK_NULL_HANDLE) {
         simple_mtx_unlock(&comp->cache_lock);
         return VK_NULL_HANDLE;
      }

      zink_screen_update_pipeline_cache(screen, &comp->base, false);
      if (compute_can_shortcut(comp)) {
         simple_mtx_unlock(&comp->cache_lock);
         /* don't add base pipeline to cache */
         state->pipeline = comp->base_pipeline = pipeline;
         return state->pipeline;
      }

      struct compute_pipeline_cache_entry *pc_entry = CALLOC_STRUCT(compute_pipeline_cache_entry);
      if (!pc_entry) {
         simple_mtx_unlock(&comp->cache_lock);
         return VK_NULL_HANDLE;
      }

      memcpy(&pc_entry->state, state, sizeof(*state));
      pc_entry->pipeline = pipeline;

      entry = _mesa_hash_table_insert_pre_hashed(&comp->pipelines, state->final_hash, pc_entry, pc_entry);
      assert(entry);
      simple_mtx_unlock(&comp->cache_lock);
   }
out:
   cache_entry = entry->data;
   state->pipeline = cache_entry->pipeline;
   return state->pipeline;
}

static void
bind_gfx_stage(struct zink_context *ctx, gl_shader_stage stage, struct zink_shader *shader)
{
   /* RADV doesn't support binding pipelines in DGC */
   if (zink_screen(ctx->base.screen)->info.nv_dgc_props.maxGraphicsShaderGroupCount == 0)
      zink_flush_dgc_if_enabled(ctx);
   if (shader && shader->info.num_inlinable_uniforms)
      ctx->shader_has_inlinable_uniforms_mask |= 1 << stage;
   else
      ctx->shader_has_inlinable_uniforms_mask &= ~(1 << stage);

   if (ctx->gfx_stages[stage])
      ctx->gfx_hash ^= ctx->gfx_stages[stage]->hash;

   if (stage == MESA_SHADER_GEOMETRY && ctx->is_generated_gs_bound && (!shader || !shader->non_fs.parent)) {
      ctx->inlinable_uniforms_valid_mask &= ~BITFIELD64_BIT(MESA_SHADER_GEOMETRY);
      ctx->is_generated_gs_bound = false;
   }

   ctx->gfx_stages[stage] = shader;
   ctx->gfx_dirty = ctx->gfx_stages[MESA_SHADER_FRAGMENT] && ctx->gfx_stages[MESA_SHADER_VERTEX];
   ctx->gfx_pipeline_state.modules_changed = true;
   if (shader) {
      ctx->shader_stages |= BITFIELD_BIT(stage);
      ctx->gfx_hash ^= ctx->gfx_stages[stage]->hash;
   } else {
      ctx->gfx_pipeline_state.modules[stage] = VK_NULL_HANDLE;
      if (ctx->curr_program)
         ctx->gfx_pipeline_state.final_hash ^= ctx->curr_program->last_variant_hash;
      ctx->curr_program = NULL;
      ctx->shader_stages &= ~BITFIELD_BIT(stage);
   }
}

static enum mesa_prim
gs_output_to_reduced_prim_type(struct shader_info *info)
{
   switch (info->gs.output_primitive) {
   case MESA_PRIM_POINTS:
      return MESA_PRIM_POINTS;

   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_LOOP:
   case MESA_PRIM_LINE_STRIP:
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
      return MESA_PRIM_LINES;

   case MESA_PRIM_TRIANGLES:
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_TRIANGLE_FAN:
   case MESA_PRIM_TRIANGLES_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      return MESA_PRIM_TRIANGLES;

   default:
      unreachable("unexpected output primitive type");
   }
}

static enum mesa_prim
update_rast_prim(struct zink_shader *shader)
{
   struct shader_info *info = &shader->info;
   if (info->stage == MESA_SHADER_GEOMETRY)
      return gs_output_to_reduced_prim_type(info);
   else if (info->stage == MESA_SHADER_TESS_EVAL) {
      if (info->tess.point_mode)
         return MESA_PRIM_POINTS;
      else {
         switch (info->tess._primitive_mode) {
         case TESS_PRIMITIVE_ISOLINES:
            return MESA_PRIM_LINES;
         case TESS_PRIMITIVE_TRIANGLES:
         case TESS_PRIMITIVE_QUADS:
            return MESA_PRIM_TRIANGLES;
         default:
            return MESA_PRIM_COUNT;
         }
      }
   }
   return MESA_PRIM_COUNT;
}

static void
unbind_generated_gs(struct zink_context *ctx, gl_shader_stage stage, struct zink_shader *prev_shader)
{
   if (prev_shader->non_fs.is_generated)
      ctx->inlinable_uniforms_valid_mask &= ~BITFIELD64_BIT(MESA_SHADER_GEOMETRY);

   if (ctx->gfx_stages[MESA_SHADER_GEOMETRY] &&
       ctx->gfx_stages[MESA_SHADER_GEOMETRY]->non_fs.parent ==
       prev_shader) {
      bind_gfx_stage(ctx, MESA_SHADER_GEOMETRY, NULL);
   }
}

static void
bind_last_vertex_stage(struct zink_context *ctx, gl_shader_stage stage, struct zink_shader *prev_shader)
{
   if (prev_shader && stage < MESA_SHADER_GEOMETRY)
      unbind_generated_gs(ctx, stage, prev_shader);

   gl_shader_stage old = ctx->last_vertex_stage ? ctx->last_vertex_stage->info.stage : MESA_SHADER_STAGES;
   if (ctx->gfx_stages[MESA_SHADER_GEOMETRY])
      ctx->last_vertex_stage = ctx->gfx_stages[MESA_SHADER_GEOMETRY];
   else if (ctx->gfx_stages[MESA_SHADER_TESS_EVAL])
      ctx->last_vertex_stage = ctx->gfx_stages[MESA_SHADER_TESS_EVAL];
   else
      ctx->last_vertex_stage = ctx->gfx_stages[MESA_SHADER_VERTEX];
   gl_shader_stage current = ctx->last_vertex_stage ? ctx->last_vertex_stage->info.stage : MESA_SHADER_VERTEX;

   /* update rast_prim */
   ctx->gfx_pipeline_state.shader_rast_prim =
      ctx->last_vertex_stage ? update_rast_prim(ctx->last_vertex_stage) :
                               MESA_PRIM_COUNT;

   if (old != current) {
      if (!zink_screen(ctx->base.screen)->optimal_keys) {
         if (old != MESA_SHADER_STAGES) {
            memset(&ctx->gfx_pipeline_state.shader_keys.key[old].key.vs_base, 0, sizeof(struct zink_vs_key_base));
            ctx->dirty_gfx_stages |= BITFIELD_BIT(old);
         } else {
            /* always unset vertex shader values when changing to a non-vs last stage */
            memset(&ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_VERTEX].key.vs_base, 0, sizeof(struct zink_vs_key_base));
         }
      }

      unsigned num_viewports = ctx->vp_state.num_viewports;
      struct zink_screen *screen = zink_screen(ctx->base.screen);
      /* number of enabled viewports is based on whether last vertex stage writes viewport index */
      if (ctx->last_vertex_stage) {
         if (ctx->last_vertex_stage->info.outputs_written & (VARYING_BIT_VIEWPORT | VARYING_BIT_VIEWPORT_MASK))
            ctx->vp_state.num_viewports = MIN2(screen->info.props.limits.maxViewports, PIPE_MAX_VIEWPORTS);
         else
            ctx->vp_state.num_viewports = 1;
      } else {
         ctx->vp_state.num_viewports = 1;
      }
      ctx->vp_state_changed |= num_viewports != ctx->vp_state.num_viewports;
      if (!screen->info.have_EXT_extended_dynamic_state) {
         if (ctx->gfx_pipeline_state.dyn_state1.num_viewports != ctx->vp_state.num_viewports)
            ctx->gfx_pipeline_state.dirty = true;
         ctx->gfx_pipeline_state.dyn_state1.num_viewports = ctx->vp_state.num_viewports;
      }
      ctx->last_vertex_stage_dirty = true;
   }
}

static void
zink_bind_vs_state(struct pipe_context *pctx,
                   void *cso)
{
   struct zink_context *ctx = zink_context(pctx);
   if (!cso && !ctx->gfx_stages[MESA_SHADER_VERTEX])
      return;
   struct zink_shader *prev_shader = ctx->gfx_stages[MESA_SHADER_VERTEX];
   bind_gfx_stage(ctx, MESA_SHADER_VERTEX, cso);
   bind_last_vertex_stage(ctx, MESA_SHADER_VERTEX, prev_shader);
   if (cso) {
      struct zink_shader *zs = cso;
      ctx->shader_reads_drawid = BITSET_TEST(zs->info.system_values_read, SYSTEM_VALUE_DRAW_ID);
      ctx->shader_reads_basevertex = BITSET_TEST(zs->info.system_values_read, SYSTEM_VALUE_BASE_VERTEX);
   } else {
      ctx->shader_reads_drawid = false;
      ctx->shader_reads_basevertex = false;
   }
}

/* if gl_SampleMask[] is written to, we have to ensure that we get a shader with the same sample count:
 * in GL, samples==1 means ignore gl_SampleMask[]
 * in VK, gl_SampleMask[] is never ignored
 */
void
zink_update_fs_key_samples(struct zink_context *ctx)
{
   if (!ctx->gfx_stages[MESA_SHADER_FRAGMENT])
      return;
   shader_info *info = &ctx->gfx_stages[MESA_SHADER_FRAGMENT]->info;
   if (info->outputs_written & (1 << FRAG_RESULT_SAMPLE_MASK)) {
      bool samples = zink_get_fs_base_key(ctx)->samples;
      if (samples != (ctx->fb_state.samples > 1))
         zink_set_fs_base_key(ctx)->samples = ctx->fb_state.samples > 1;
   }
}

void zink_update_gs_key_rectangular_line(struct zink_context *ctx)
{
   bool line_rectangular = zink_get_gs_key(ctx)->line_rectangular;
   if (line_rectangular != ctx->rast_state->base.line_rectangular)
      zink_set_gs_key(ctx)->line_rectangular = ctx->rast_state->base.line_rectangular;
}

static void
zink_bind_fs_state(struct pipe_context *pctx,
                   void *cso)
{
   struct zink_context *ctx = zink_context(pctx);
   if (!cso && !ctx->gfx_stages[MESA_SHADER_FRAGMENT])
      return;
   if (ctx->disable_fs && !ctx->disable_color_writes && cso != ctx->null_fs) {
      ctx->saved_fs = cso;
      zink_set_null_fs(ctx);
      return;
   }
   bool writes_cbuf0 = ctx->gfx_stages[MESA_SHADER_FRAGMENT] ? (ctx->gfx_stages[MESA_SHADER_FRAGMENT]->info.outputs_written & BITFIELD_BIT(FRAG_RESULT_DATA0)) > 0 : true;
   unsigned shadow_mask = ctx->gfx_stages[MESA_SHADER_FRAGMENT] ? ctx->gfx_stages[MESA_SHADER_FRAGMENT]->fs.legacy_shadow_mask : 0;
   bind_gfx_stage(ctx, MESA_SHADER_FRAGMENT, cso);
   ctx->fbfetch_outputs = 0;
   if (cso) {
      shader_info *info = &ctx->gfx_stages[MESA_SHADER_FRAGMENT]->info;
      bool new_writes_cbuf0 = (info->outputs_written & BITFIELD_BIT(FRAG_RESULT_DATA0)) > 0;
      if (ctx->gfx_pipeline_state.blend_state && ctx->gfx_pipeline_state.blend_state->alpha_to_coverage &&
          writes_cbuf0 != new_writes_cbuf0 && zink_screen(pctx->screen)->info.have_EXT_extended_dynamic_state3) {
         ctx->blend_state_changed = true;
         ctx->ds3_states |= BITFIELD_BIT(ZINK_DS3_BLEND_A2C);
      }
      if (info->fs.uses_fbfetch_output) {
         if (info->outputs_read & (BITFIELD_BIT(FRAG_RESULT_DEPTH) | BITFIELD_BIT(FRAG_RESULT_STENCIL)))
            ctx->fbfetch_outputs |= BITFIELD_BIT(PIPE_MAX_COLOR_BUFS);
         ctx->fbfetch_outputs |= info->outputs_read >> FRAG_RESULT_DATA0;
      }
      zink_update_fs_key_samples(ctx);
      if (zink_screen(pctx->screen)->info.have_EXT_rasterization_order_attachment_access) {
         if (ctx->gfx_pipeline_state.rast_attachment_order != info->fs.uses_fbfetch_output)
            ctx->gfx_pipeline_state.dirty = true;
         ctx->gfx_pipeline_state.rast_attachment_order = info->fs.uses_fbfetch_output;
      }
      zink_set_zs_needs_shader_swizzle_key(ctx, MESA_SHADER_FRAGMENT, false);
      if (shadow_mask != ctx->gfx_stages[MESA_SHADER_FRAGMENT]->fs.legacy_shadow_mask &&
          !zink_screen(pctx->screen)->driver_workarounds.needs_zs_shader_swizzle)
         zink_update_shadow_samplerviews(ctx, shadow_mask | ctx->gfx_stages[MESA_SHADER_FRAGMENT]->fs.legacy_shadow_mask);
      if (!ctx->track_renderpasses && !ctx->blitting)
         ctx->rp_tc_info_updated = true;
   }
   zink_update_fbfetch(ctx);
}

static void
zink_bind_gs_state(struct pipe_context *pctx,
                   void *cso)
{
   struct zink_context *ctx = zink_context(pctx);
   if (!cso && !ctx->gfx_stages[MESA_SHADER_GEOMETRY])
      return;
   bind_gfx_stage(ctx, MESA_SHADER_GEOMETRY, cso);
   bind_last_vertex_stage(ctx, MESA_SHADER_GEOMETRY, NULL);
}

static void
zink_bind_tcs_state(struct pipe_context *pctx,
                   void *cso)
{
   bind_gfx_stage(zink_context(pctx), MESA_SHADER_TESS_CTRL, cso);
}

static void
zink_bind_tes_state(struct pipe_context *pctx,
                   void *cso)
{
   struct zink_context *ctx = zink_context(pctx);
   if (!cso && !ctx->gfx_stages[MESA_SHADER_TESS_EVAL])
      return;
   if (!!ctx->gfx_stages[MESA_SHADER_TESS_EVAL] != !!cso) {
      if (!cso) {
         /* if unsetting a TESS that uses a generated TCS, ensure the TCS is unset */
         if (ctx->gfx_stages[MESA_SHADER_TESS_CTRL] == ctx->gfx_stages[MESA_SHADER_TESS_EVAL]->non_fs.generated_tcs)
            ctx->gfx_stages[MESA_SHADER_TESS_CTRL] = NULL;
      }
   }
   struct zink_shader *prev_shader = ctx->gfx_stages[MESA_SHADER_TESS_EVAL];
   bind_gfx_stage(ctx, MESA_SHADER_TESS_EVAL, cso);
   bind_last_vertex_stage(ctx, MESA_SHADER_TESS_EVAL, prev_shader);
}

static void *
zink_create_cs_state(struct pipe_context *pctx,
                     const struct pipe_compute_state *shader)
{
   struct nir_shader *nir;
   if (shader->ir_type != PIPE_SHADER_IR_NIR)
      nir = zink_tgsi_to_nir(pctx->screen, shader->prog);
   else
      nir = (struct nir_shader *)shader->prog;

   if (nir->info.uses_bindless)
      zink_descriptors_init_bindless(zink_context(pctx));

   return create_compute_program(zink_context(pctx), nir);
}

static void
zink_bind_cs_state(struct pipe_context *pctx,
                   void *cso)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_compute_program *comp = cso;
   if (comp && comp->num_inlinable_uniforms)
      ctx->shader_has_inlinable_uniforms_mask |= 1 << MESA_SHADER_COMPUTE;
   else
      ctx->shader_has_inlinable_uniforms_mask &= ~(1 << MESA_SHADER_COMPUTE);

   if (ctx->curr_compute) {
      zink_batch_reference_program(&ctx->batch, &ctx->curr_compute->base);
      ctx->compute_pipeline_state.final_hash ^= ctx->compute_pipeline_state.module_hash;
      ctx->compute_pipeline_state.module = VK_NULL_HANDLE;
      ctx->compute_pipeline_state.module_hash = 0;
   }
   ctx->compute_pipeline_state.dirty = true;
   ctx->curr_compute = comp;
   if (comp && comp != ctx->curr_compute) {
      ctx->compute_pipeline_state.module_hash = ctx->curr_compute->curr->hash;
      if (util_queue_fence_is_signalled(&comp->base.cache_fence))
         ctx->compute_pipeline_state.module = ctx->curr_compute->curr->obj.mod;
      ctx->compute_pipeline_state.final_hash ^= ctx->compute_pipeline_state.module_hash;
      if (ctx->compute_pipeline_state.key.base.nonseamless_cube_mask)
         ctx->compute_dirty = true;
   }
   zink_select_launch_grid(ctx);
}

static void
zink_get_compute_state_info(struct pipe_context *pctx, void *cso, struct pipe_compute_state_object_info *info)
{
   struct zink_compute_program *comp = cso;
   struct zink_screen *screen = zink_screen(pctx->screen);

   info->max_threads = screen->info.props.limits.maxComputeWorkGroupInvocations;
   info->private_memory = comp->scratch_size;
   if (screen->info.props11.subgroupSize) {
      info->preferred_simd_size = screen->info.props11.subgroupSize;
      info->simd_sizes = info->preferred_simd_size;
   } else {
      // just guess it
      info->preferred_simd_size = 64;
      // only used for actual subgroup support
      info->simd_sizes = 0;
   }
}

static void
zink_delete_cs_shader_state(struct pipe_context *pctx, void *cso)
{
   struct zink_compute_program *comp = cso;
   zink_compute_program_reference(zink_screen(pctx->screen), &comp, NULL);
}

/* caller must lock prog->libs->lock */
struct zink_gfx_library_key *
zink_create_pipeline_lib(struct zink_screen *screen, struct zink_gfx_program *prog, struct zink_gfx_pipeline_state *state)
{
   struct zink_gfx_library_key *gkey = CALLOC_STRUCT(zink_gfx_library_key);
   if (!gkey) {
      mesa_loge("ZINK: failed to allocate gkey!");
      return NULL;
   }
      
   gkey->optimal_key = state->optimal_key;
   assert(gkey->optimal_key);
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++)
      gkey->modules[i] = prog->objs[i].mod;
   gkey->pipeline = zink_create_gfx_pipeline_library(screen, prog);
   _mesa_set_add(&prog->libs->libs, gkey);
   return gkey;
}

static const char *
print_exe_stages(VkShaderStageFlags stages)
{
   if (stages == VK_SHADER_STAGE_VERTEX_BIT)
      return "VS";
   if (stages == (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT))
      return "VS+GS";
   if (stages == (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))
      return "VS+TCS+TES";
   if (stages == (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT))
      return "VS+TCS+TES+GS";
   if (stages == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
      return "TCS";
   if (stages == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
      return "TES";
   if (stages == VK_SHADER_STAGE_GEOMETRY_BIT)
      return "GS";
   if (stages == VK_SHADER_STAGE_FRAGMENT_BIT)
      return "FS";
   if (stages == VK_SHADER_STAGE_COMPUTE_BIT)
      return "CS";
   unreachable("unhandled combination of stages!");
}

static void
print_pipeline_stats(struct zink_screen *screen, VkPipeline pipeline)
{
   VkPipelineInfoKHR pinfo = {
     VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR,
     NULL,
     pipeline 
   };
   unsigned exe_count = 0;
   VkPipelineExecutablePropertiesKHR props[10] = {0};
   for (unsigned i = 0; i < ARRAY_SIZE(props); i++) {
      props[i].sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR;
      props[i].pNext = NULL;
   }
   VKSCR(GetPipelineExecutablePropertiesKHR)(screen->dev, &pinfo, &exe_count, NULL);
   VKSCR(GetPipelineExecutablePropertiesKHR)(screen->dev, &pinfo, &exe_count, props);
   printf("PIPELINE STATISTICS:");
   for (unsigned e = 0; e < exe_count; e++) {
      VkPipelineExecutableInfoKHR info = {
         VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR,
         NULL,
         pipeline,
         e
      };
      unsigned count = 0;
      printf("\n\t%s (%s): ", print_exe_stages(props[e].stages), props[e].name);
      VkPipelineExecutableStatisticKHR *stats = NULL;
      VKSCR(GetPipelineExecutableStatisticsKHR)(screen->dev, &info, &count, NULL);
      stats = calloc(count, sizeof(VkPipelineExecutableStatisticKHR));
      if (!stats) {
         mesa_loge("ZINK: failed to allocate stats!");
         return;
      }
         
      for (unsigned i = 0; i < count; i++)
         stats[i].sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR;
      VKSCR(GetPipelineExecutableStatisticsKHR)(screen->dev, &info, &count, stats);

      for (unsigned i = 0; i < count; i++) {
         if (i)
            printf(", ");
         switch (stats[i].format) {
         case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR:
            printf("%s: %u", stats[i].name, stats[i].value.b32);
            break;
         case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR:
            printf("%s: %" PRIi64, stats[i].name, stats[i].value.i64);
            break;
         case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR:
            printf("%s: %" PRIu64, stats[i].name, stats[i].value.u64);
            break;
         case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR:
            printf("%s: %g", stats[i].name, stats[i].value.f64);
            break;
         default:
            unreachable("unknown statistic");
         }
      }
   }
   printf("\n");
}

static void
precompile_job(void *data, void *gdata, int thread_index)
{
   struct zink_screen *screen = gdata;
   struct zink_gfx_program *prog = data;

   struct zink_gfx_pipeline_state state = {0};
   state.shader_keys_optimal.key.vs_base.last_vertex_stage = true;
   state.shader_keys_optimal.key.tcs.patch_vertices = 3; //random guess, generated tcs precompile is hard
   state.optimal_key = state.shader_keys_optimal.key.val;
   generate_gfx_program_modules_optimal(NULL, screen, prog, &state);
   zink_screen_get_pipeline_cache(screen, &prog->base, true);
   if (!screen->info.have_EXT_shader_object) {
      simple_mtx_lock(&prog->libs->lock);
      zink_create_pipeline_lib(screen, prog, &state);
      simple_mtx_unlock(&prog->libs->lock);
   }
   zink_screen_update_pipeline_cache(screen, &prog->base, true);
}

static void
precompile_separate_shader_job(void *data, void *gdata, int thread_index)
{
   struct zink_screen *screen = gdata;
   struct zink_shader *zs = data;

   zs->precompile.obj = zink_shader_compile_separate(screen, zs);
   if (!screen->info.have_EXT_shader_object) {
      struct zink_shader_object objs[ZINK_GFX_SHADER_COUNT] = {0};
      objs[zs->info.stage].mod = zs->precompile.obj.mod;
      zs->precompile.gpl = zink_create_gfx_pipeline_separate(screen, objs, zs->precompile.layout, zs->info.stage);
   }
}

static void
zink_link_gfx_shader(struct pipe_context *pctx, void **shaders)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_shader **zshaders = (struct zink_shader **)shaders;
   if (shaders[MESA_SHADER_COMPUTE])
      return;
   /* explicitly block sample shading: this needs full pipelines always */
   if (zshaders[MESA_SHADER_FRAGMENT] && zshaders[MESA_SHADER_FRAGMENT]->info.fs.uses_sample_shading)
      return;
   /* can't precompile fixedfunc */
   if (!shaders[MESA_SHADER_VERTEX] || !shaders[MESA_SHADER_FRAGMENT]) {
      /* handled directly from shader create */
      return;
   }
   unsigned hash = 0;
   unsigned shader_stages = 0;
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++) {
      if (zshaders[i]) {
         hash ^= zshaders[i]->hash;
         shader_stages |= BITFIELD_BIT(i);
      }
   }
   unsigned tess_stages = BITFIELD_BIT(MESA_SHADER_TESS_CTRL) | BITFIELD_BIT(MESA_SHADER_TESS_EVAL);
   unsigned tess = shader_stages & tess_stages;
   /* can't do fixedfunc tes either */
   if (tess && !shaders[MESA_SHADER_TESS_EVAL])
      return;
   struct hash_table *ht = &ctx->program_cache[zink_program_cache_stages(shader_stages)];
   simple_mtx_lock(&ctx->program_lock[zink_program_cache_stages(shader_stages)]);
   /* link can be called repeatedly with the same shaders: ignore */
   if (_mesa_hash_table_search_pre_hashed(ht, hash, shaders)) {
      simple_mtx_unlock(&ctx->program_lock[zink_program_cache_stages(shader_stages)]);
      return;
   }
   struct zink_gfx_program *prog = zink_create_gfx_program(ctx, zshaders, 3, hash);
   u_foreach_bit(i, shader_stages)
      assert(prog->shaders[i]);
   _mesa_hash_table_insert_pre_hashed(ht, hash, prog->shaders, prog);
   prog->base.removed = false;
   simple_mtx_unlock(&ctx->program_lock[zink_program_cache_stages(shader_stages)]);
   if (zink_debug & ZINK_DEBUG_SHADERDB) {
      struct zink_screen *screen = zink_screen(pctx->screen);
      if (screen->optimal_keys)
         generate_gfx_program_modules_optimal(ctx, screen,  prog, &ctx->gfx_pipeline_state);
      else
         generate_gfx_program_modules(ctx, screen,  prog, &ctx->gfx_pipeline_state);
      VkPipeline pipeline = zink_create_gfx_pipeline(screen, prog, prog->objs, &ctx->gfx_pipeline_state,
                                                     ctx->gfx_pipeline_state.element_state->binding_map,
                                                     shaders[MESA_SHADER_TESS_EVAL] ? VK_PRIMITIVE_TOPOLOGY_PATCH_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, true, NULL);
      print_pipeline_stats(screen, pipeline);
   } else {
      if (zink_screen(pctx->screen)->info.have_EXT_shader_object)
         prog->base.uses_shobj = !BITSET_TEST(zshaders[MESA_SHADER_FRAGMENT]->info.system_values_read, SYSTEM_VALUE_SAMPLE_MASK_IN);
      if (zink_debug & ZINK_DEBUG_NOBGC)
         precompile_job(prog, pctx->screen, 0);
      else
         util_queue_add_job(&zink_screen(pctx->screen)->cache_get_thread, prog, &prog->base.cache_fence, precompile_job, NULL, 0);
   }
}

void
zink_delete_shader_state(struct pipe_context *pctx, void *cso)
{
   zink_gfx_shader_free(zink_screen(pctx->screen), cso);
}

void *
zink_create_gfx_shader_state(struct pipe_context *pctx, const struct pipe_shader_state *shader)
{
   struct zink_screen *screen = zink_screen(pctx->screen);
   nir_shader *nir;
   if (shader->type != PIPE_SHADER_IR_NIR)
      nir = zink_tgsi_to_nir(pctx->screen, shader->tokens);
   else
      nir = (struct nir_shader *)shader->ir.nir;

   if (nir->info.stage == MESA_SHADER_FRAGMENT && nir->info.fs.uses_fbfetch_output)
      zink_descriptor_util_init_fbfetch(zink_context(pctx));
   if (nir->info.uses_bindless)
      zink_descriptors_init_bindless(zink_context(pctx));

   void *ret = zink_shader_create(zink_screen(pctx->screen), nir);

   if (nir->info.separate_shader && zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB &&
       (screen->info.have_EXT_shader_object ||
       (screen->info.have_EXT_graphics_pipeline_library && (nir->info.stage == MESA_SHADER_FRAGMENT || nir->info.stage == MESA_SHADER_VERTEX)))) {
      struct zink_shader *zs = ret;
      /* sample shading can't precompile */
      if (nir->info.stage != MESA_SHADER_FRAGMENT || !nir->info.fs.uses_sample_shading) {
         if (zink_debug & ZINK_DEBUG_NOBGC)
            precompile_separate_shader_job(zs, screen, 0);
         else
            util_queue_add_job(&screen->cache_get_thread, zs, &zs->precompile.fence, precompile_separate_shader_job, NULL, 0);
      }
   }
   ralloc_free(nir);

   return ret;
}

static void
zink_delete_cached_shader_state(struct pipe_context *pctx, void *cso)
{
   struct zink_screen *screen = zink_screen(pctx->screen);
   util_shader_reference(pctx, &screen->shaders, &cso, NULL);
}

static void *
zink_create_cached_shader_state(struct pipe_context *pctx, const struct pipe_shader_state *shader)
{
   bool cache_hit;
   struct zink_screen *screen = zink_screen(pctx->screen);
   return util_live_shader_cache_get(pctx, &screen->shaders, shader, &cache_hit);
}


void
zink_program_init(struct zink_context *ctx)
{
   ctx->base.create_vs_state = zink_create_cached_shader_state;
   ctx->base.bind_vs_state = zink_bind_vs_state;
   ctx->base.delete_vs_state = zink_delete_cached_shader_state;

   ctx->base.create_fs_state = zink_create_cached_shader_state;
   ctx->base.bind_fs_state = zink_bind_fs_state;
   ctx->base.delete_fs_state = zink_delete_cached_shader_state;

   ctx->base.create_gs_state = zink_create_cached_shader_state;
   ctx->base.bind_gs_state = zink_bind_gs_state;
   ctx->base.delete_gs_state = zink_delete_cached_shader_state;

   ctx->base.create_tcs_state = zink_create_cached_shader_state;
   ctx->base.bind_tcs_state = zink_bind_tcs_state;
   ctx->base.delete_tcs_state = zink_delete_cached_shader_state;

   ctx->base.create_tes_state = zink_create_cached_shader_state;
   ctx->base.bind_tes_state = zink_bind_tes_state;
   ctx->base.delete_tes_state = zink_delete_cached_shader_state;

   ctx->base.create_compute_state = zink_create_cs_state;
   ctx->base.bind_compute_state = zink_bind_cs_state;
   ctx->base.get_compute_state_info = zink_get_compute_state_info;
   ctx->base.delete_compute_state = zink_delete_cs_shader_state;

   if (zink_screen(ctx->base.screen)->info.have_EXT_vertex_input_dynamic_state)
      _mesa_set_init(&ctx->gfx_inputs, ctx, hash_gfx_input_dynamic, equals_gfx_input_dynamic);
   else
      _mesa_set_init(&ctx->gfx_inputs, ctx, hash_gfx_input, equals_gfx_input);
   if (zink_screen(ctx->base.screen)->have_full_ds3)
      _mesa_set_init(&ctx->gfx_outputs, ctx, hash_gfx_output_ds3, equals_gfx_output_ds3);
   else
      _mesa_set_init(&ctx->gfx_outputs, ctx, hash_gfx_output, equals_gfx_output);
   /* validate struct packing */
   STATIC_ASSERT(offsetof(struct zink_gfx_output_key, sample_mask) == sizeof(uint32_t));
   STATIC_ASSERT(offsetof(struct zink_gfx_pipeline_state, vertex_buffers_enabled_mask) - offsetof(struct zink_gfx_pipeline_state, input) ==
                 offsetof(struct zink_gfx_input_key, vertex_buffers_enabled_mask) - offsetof(struct zink_gfx_input_key, input));
   STATIC_ASSERT(offsetof(struct zink_gfx_pipeline_state, vertex_strides) - offsetof(struct zink_gfx_pipeline_state, input) ==
                 offsetof(struct zink_gfx_input_key, vertex_strides) - offsetof(struct zink_gfx_input_key, input));
   STATIC_ASSERT(offsetof(struct zink_gfx_pipeline_state, element_state) - offsetof(struct zink_gfx_pipeline_state, input) ==
                 offsetof(struct zink_gfx_input_key, element_state) - offsetof(struct zink_gfx_input_key, input));

   STATIC_ASSERT(sizeof(union zink_shader_key_optimal) == sizeof(uint32_t));

   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (screen->info.have_EXT_graphics_pipeline_library || screen->info.have_EXT_shader_object || zink_debug & ZINK_DEBUG_SHADERDB)
      ctx->base.link_shader = zink_link_gfx_shader;
}

bool
zink_set_rasterizer_discard(struct zink_context *ctx, bool disable)
{
   bool value = disable ? false : (ctx->rast_state ? ctx->rast_state->base.rasterizer_discard : false);
   bool changed = ctx->gfx_pipeline_state.dyn_state2.rasterizer_discard != value;
   ctx->gfx_pipeline_state.dyn_state2.rasterizer_discard = value;
   if (!changed)
      return false;
   if (!zink_screen(ctx->base.screen)->info.have_EXT_extended_dynamic_state2)
      ctx->gfx_pipeline_state.dirty |= true;
   ctx->rasterizer_discard_changed = true;
   return true;
}

void
zink_driver_thread_add_job(struct pipe_screen *pscreen, void *data,
                           struct util_queue_fence *fence,
                           pipe_driver_thread_func execute,
                           pipe_driver_thread_func cleanup,
                           const size_t job_size)
{
   struct zink_screen *screen = zink_screen(pscreen);
   util_queue_add_job(&screen->cache_get_thread, data, fence, execute, cleanup, job_size);
}

static bool
has_edge_flags(struct zink_context *ctx)
{
   switch(ctx->gfx_pipeline_state.gfx_prim_mode) {
   case MESA_PRIM_POINTS:
   case MESA_PRIM_LINE_STRIP:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_LOOP:
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_TRIANGLE_FAN:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
   case MESA_PRIM_QUAD_STRIP:
   case MESA_PRIM_PATCHES:
      return false;
   case MESA_PRIM_TRIANGLES:
   case MESA_PRIM_TRIANGLES_ADJACENCY:
   case MESA_PRIM_QUADS:
   case MESA_PRIM_POLYGON:
   case MESA_PRIM_COUNT:
   default:
      break;
   }
   return (ctx->gfx_pipeline_state.rast_prim == MESA_PRIM_LINES ||
           ctx->gfx_pipeline_state.rast_prim == MESA_PRIM_POINTS) &&
          ctx->gfx_stages[MESA_SHADER_VERTEX]->has_edgeflags;
}

static enum zink_rast_prim
zink_rast_prim_for_pipe(enum mesa_prim prim)
{
   switch (prim) {
   case MESA_PRIM_POINTS:
      return ZINK_PRIM_POINTS;
   case MESA_PRIM_LINES:
      return ZINK_PRIM_LINES;
   case MESA_PRIM_TRIANGLES:
   default:
      return ZINK_PRIM_TRIANGLES;
   }
}

static enum mesa_prim
zink_tess_prim_type(struct zink_shader *tess)
{
   if (tess->info.tess.point_mode)
      return MESA_PRIM_POINTS;
   else {
      switch (tess->info.tess._primitive_mode) {
      case TESS_PRIMITIVE_ISOLINES:
         return MESA_PRIM_LINES;
      case TESS_PRIMITIVE_TRIANGLES:
      case TESS_PRIMITIVE_QUADS:
         return MESA_PRIM_TRIANGLES;
      default:
         return MESA_PRIM_COUNT;
      }
   }
}

static inline void
zink_add_inline_uniform(nir_shader *shader, int offset)
{
   shader->info.inlinable_uniform_dw_offsets[shader->info.num_inlinable_uniforms] = offset;
   ++shader->info.num_inlinable_uniforms;
}

static unsigned
encode_lower_pv_mode(enum mesa_prim prim_type)
{
   switch (prim_type) {
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_QUAD_STRIP:
      return ZINK_PVE_PRIMITIVE_TRISTRIP;
   case MESA_PRIM_TRIANGLE_FAN:
      return ZINK_PVE_PRIMITIVE_FAN;
   default:
      return ZINK_PVE_PRIMITIVE_SIMPLE;
   }
}

void
zink_set_primitive_emulation_keys(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   bool lower_line_stipple = false, lower_line_smooth = false;
   unsigned lower_pv_mode = 0;
   if (!screen->optimal_keys) {
      lower_line_stipple = ctx->gfx_pipeline_state.rast_prim == MESA_PRIM_LINES &&
                                screen->driver_workarounds.no_linestipple &&
                                ctx->rast_state->base.line_stipple_enable &&
                                !ctx->num_so_targets;

      bool lower_point_smooth = ctx->gfx_pipeline_state.rast_prim == MESA_PRIM_POINTS &&
                                screen->driconf.emulate_point_smooth &&
                                ctx->rast_state->base.point_smooth;
      if (zink_get_fs_key(ctx)->lower_line_stipple != lower_line_stipple) {
         assert(zink_get_gs_key(ctx)->lower_line_stipple ==
                zink_get_fs_key(ctx)->lower_line_stipple);
         zink_set_fs_key(ctx)->lower_line_stipple = lower_line_stipple;
         zink_set_gs_key(ctx)->lower_line_stipple = lower_line_stipple;
      }

      lower_line_smooth = ctx->gfx_pipeline_state.rast_prim == MESA_PRIM_LINES &&
                          screen->driver_workarounds.no_linesmooth &&
                          ctx->rast_state->base.line_smooth &&
                          !ctx->num_so_targets;

      if (zink_get_fs_key(ctx)->lower_line_smooth != lower_line_smooth) {
         assert(zink_get_gs_key(ctx)->lower_line_smooth ==
                zink_get_fs_key(ctx)->lower_line_smooth);
         zink_set_fs_key(ctx)->lower_line_smooth = lower_line_smooth;
         zink_set_gs_key(ctx)->lower_line_smooth = lower_line_smooth;
      }

      if (zink_get_fs_key(ctx)->lower_point_smooth != lower_point_smooth) {
         zink_set_fs_key(ctx)->lower_point_smooth = lower_point_smooth;
      }

      lower_pv_mode = ctx->gfx_pipeline_state.dyn_state3.pv_last &&
                      !screen->info.have_EXT_provoking_vertex;
      if (lower_pv_mode)
         lower_pv_mode = encode_lower_pv_mode(ctx->gfx_pipeline_state.gfx_prim_mode);

      if (zink_get_gs_key(ctx)->lower_pv_mode != lower_pv_mode)
         zink_set_gs_key(ctx)->lower_pv_mode = lower_pv_mode;
   }

   bool lower_edge_flags = has_edge_flags(ctx);

   bool lower_quad_prim = ctx->gfx_pipeline_state.gfx_prim_mode == MESA_PRIM_QUADS;

   bool lower_filled_quad =  lower_quad_prim &&
      ctx->gfx_pipeline_state.rast_prim == MESA_PRIM_TRIANGLES;

   if (lower_line_stipple || lower_line_smooth ||
       lower_edge_flags || lower_quad_prim ||
       lower_pv_mode || zink_get_gs_key(ctx)->lower_gl_point) {
      enum pipe_shader_type prev_vertex_stage =
         ctx->gfx_stages[MESA_SHADER_TESS_EVAL] ?
            MESA_SHADER_TESS_EVAL : MESA_SHADER_VERTEX;
      enum zink_rast_prim zink_prim_type =
         zink_rast_prim_for_pipe(ctx->gfx_pipeline_state.rast_prim);

      //when using transform feedback primitives must be tessellated
      lower_filled_quad |= lower_quad_prim && ctx->gfx_stages[prev_vertex_stage]->info.has_transform_feedback_varyings;

      if (!ctx->gfx_stages[MESA_SHADER_GEOMETRY] || (ctx->gfx_stages[MESA_SHADER_GEOMETRY]->non_fs.is_generated &&
          ctx->gfx_stages[MESA_SHADER_GEOMETRY]->info.gs.input_primitive != ctx->gfx_pipeline_state.gfx_prim_mode)) {

         if (!ctx->gfx_stages[prev_vertex_stage]->non_fs.generated_gs[ctx->gfx_pipeline_state.gfx_prim_mode][zink_prim_type]) {
            nir_shader *prev_stage = zink_shader_deserialize(screen, ctx->gfx_stages[prev_vertex_stage]);
            nir_shader *nir;
            if (lower_filled_quad) {
               nir = zink_create_quads_emulation_gs(
                  &screen->nir_options,
                  prev_stage);
            } else {
               enum mesa_prim prim = ctx->gfx_pipeline_state.gfx_prim_mode;
               if (prev_vertex_stage == MESA_SHADER_TESS_EVAL)
                  prim = zink_tess_prim_type(ctx->gfx_stages[MESA_SHADER_TESS_EVAL]);
               nir = nir_create_passthrough_gs(
                  &screen->nir_options,
                  prev_stage,
                  prim,
                  ctx->gfx_pipeline_state.rast_prim,
                  lower_edge_flags,
                  lower_line_stipple || lower_quad_prim);
            }
            zink_lower_system_values_to_inlined_uniforms(nir);

            zink_add_inline_uniform(nir, ZINK_INLINE_VAL_FLAT_MASK);
            zink_add_inline_uniform(nir, ZINK_INLINE_VAL_FLAT_MASK+1);
            zink_add_inline_uniform(nir, ZINK_INLINE_VAL_PV_LAST_VERT);
            ralloc_free(prev_stage);
            struct zink_shader *shader = zink_shader_create(screen, nir);
            shader->needs_inlining = true;
            ctx->gfx_stages[prev_vertex_stage]->non_fs.generated_gs[ctx->gfx_pipeline_state.gfx_prim_mode][zink_prim_type] = shader;
            shader->non_fs.is_generated = true;
            shader->non_fs.parent = ctx->gfx_stages[prev_vertex_stage];
            shader->can_inline = true;
            memcpy(shader->sinfo.stride, ctx->gfx_stages[prev_vertex_stage]->sinfo.stride, sizeof(shader->sinfo.stride));
         }

         ctx->base.bind_gs_state(&ctx->base,
                                 ctx->gfx_stages[prev_vertex_stage]->non_fs.generated_gs[ctx->gfx_pipeline_state.gfx_prim_mode][zink_prim_type]);
         ctx->is_generated_gs_bound = true;
      }

      ctx->base.set_inlinable_constants(&ctx->base, MESA_SHADER_GEOMETRY, 3,
                                        (uint32_t []){ctx->gfx_stages[MESA_SHADER_FRAGMENT]->flat_flags,
                                                      ctx->gfx_stages[MESA_SHADER_FRAGMENT]->flat_flags >> 32,
                                                      ctx->gfx_pipeline_state.dyn_state3.pv_last});
   } else if (ctx->gfx_stages[MESA_SHADER_GEOMETRY] &&
              ctx->gfx_stages[MESA_SHADER_GEOMETRY]->non_fs.is_generated)
         ctx->base.bind_gs_state(&ctx->base, NULL);
}
