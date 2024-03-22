/*
 * Copyright (c) 2017-2019 Lima Project
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
 */

#include "util/u_memory.h"
#include "util/ralloc.h"
#include "util/u_debug.h"

#include "tgsi/tgsi_dump.h"
#include "compiler/nir/nir.h"
#include "compiler/nir/nir_serialize.h"
#include "nir/tgsi_to_nir.h"
#include "nir_legacy.h"

#include "pipe/p_state.h"

#include "lima_screen.h"
#include "lima_context.h"
#include "lima_job.h"
#include "lima_program.h"
#include "lima_bo.h"
#include "lima_disk_cache.h"

#include "ir/lima_ir.h"

static const nir_shader_compiler_options vs_nir_options = {
   .lower_ffma16 = true,
   .lower_ffma32 = true,
   .lower_ffma64 = true,
   .lower_fpow = true,
   .lower_ffract = true,
   .lower_fdiv = true,
   .lower_fmod = true,
   .lower_fsqrt = true,
   .lower_flrp32 = true,
   .lower_flrp64 = true,
   /* could be implemented by clamp */
   .lower_fsat = true,
   .lower_bitops = true,
   .lower_sincos = true,
   .lower_fceil = true,
   .lower_insert_byte = true,
   .lower_insert_word = true,
   .force_indirect_unrolling = nir_var_all,
   .force_indirect_unrolling_sampler = true,
   .lower_varying_from_uniform = true,
   .max_unroll_iterations = 32,
};

static const nir_shader_compiler_options fs_nir_options = {
   .lower_ffma16 = true,
   .lower_ffma32 = true,
   .lower_ffma64 = true,
   .lower_fpow = true,
   .lower_fdiv = true,
   .lower_fmod = true,
   .lower_flrp32 = true,
   .lower_flrp64 = true,
   .lower_fsign = true,
   .lower_fdot = true,
   .lower_fdph = true,
   .lower_insert_byte = true,
   .lower_insert_word = true,
   .lower_bitops = true,
   .lower_vector_cmp = true,
   .force_indirect_unrolling = (nir_var_shader_out | nir_var_function_temp),
   .force_indirect_unrolling_sampler = true,
   .lower_varying_from_uniform = true,
   .max_unroll_iterations = 32,
};

const void *
lima_program_get_compiler_options(enum pipe_shader_type shader)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX:
      return &vs_nir_options;
   case PIPE_SHADER_FRAGMENT:
      return &fs_nir_options;
   default:
      return NULL;
   }
}

static int
type_size(const struct glsl_type *type, bool bindless)
{
   return glsl_count_attribute_slots(type, false);
}

void
lima_program_optimize_vs_nir(struct nir_shader *s)
{
   bool progress;

   NIR_PASS_V(s, nir_lower_viewport_transform);
   NIR_PASS_V(s, nir_lower_point_size, 1.0f, 100.0f);
   NIR_PASS_V(s, nir_lower_io,
	      nir_var_shader_in | nir_var_shader_out, type_size, 0);
   NIR_PASS_V(s, nir_lower_load_const_to_scalar);
   NIR_PASS_V(s, lima_nir_lower_uniform_to_scalar);
   NIR_PASS_V(s, nir_lower_io_to_scalar,
              nir_var_shader_in|nir_var_shader_out, NULL, NULL);

   do {
      progress = false;

      NIR_PASS_V(s, nir_lower_vars_to_ssa);
      NIR_PASS(progress, s, nir_lower_alu_to_scalar, NULL, NULL);
      NIR_PASS(progress, s, nir_lower_phis_to_scalar, false);
      NIR_PASS(progress, s, nir_copy_prop);
      NIR_PASS(progress, s, nir_opt_remove_phis);
      NIR_PASS(progress, s, nir_opt_dce);
      NIR_PASS(progress, s, nir_opt_dead_cf);
      NIR_PASS(progress, s, nir_opt_cse);
      NIR_PASS(progress, s, nir_opt_peephole_select, 8, true, true);
      NIR_PASS(progress, s, nir_opt_algebraic);
      NIR_PASS(progress, s, lima_nir_lower_ftrunc);
      NIR_PASS(progress, s, nir_opt_constant_folding);
      NIR_PASS(progress, s, nir_opt_undef);
      NIR_PASS(progress, s, nir_lower_undef_to_zero);
      NIR_PASS(progress, s, nir_opt_loop_unroll);
      NIR_PASS(progress, s, nir_lower_undef_to_zero);
   } while (progress);

   NIR_PASS_V(s, nir_lower_int_to_float);
   /* int_to_float pass generates ftrunc, so lower it */
   NIR_PASS(progress, s, lima_nir_lower_ftrunc);
   NIR_PASS_V(s, nir_lower_bool_to_float, true);

   NIR_PASS_V(s, nir_copy_prop);
   NIR_PASS_V(s, nir_opt_dce);
   NIR_PASS_V(s, lima_nir_split_loads);
   NIR_PASS_V(s, nir_convert_from_ssa, true);
   NIR_PASS_V(s, nir_opt_dce);
   NIR_PASS_V(s, nir_remove_dead_variables, nir_var_function_temp, NULL);
   nir_sweep(s);
}

static bool
lima_alu_to_scalar_filter_cb(const nir_instr *instr, const void *data)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);
   switch (alu->op) {
   case nir_op_frcp:
   /* nir_op_idiv is lowered to frcp by lower_int_to_floats which
    * will be run later, so lower idiv here
    */
   case nir_op_idiv:
   case nir_op_frsq:
   case nir_op_flog2:
   case nir_op_fexp2:
   case nir_op_fsqrt:
   case nir_op_fsin:
   case nir_op_fcos:
      return true;
   default:
      break;
   }

   /* nir vec4 fcsel assumes that each component of the condition will be
    * used to select the same component from the two options, but Utgard PP
    * has only 1 component condition. If all condition components are not the
    * same we need to lower it to scalar.
    */
   switch (alu->op) {
   case nir_op_bcsel:
   case nir_op_fcsel:
      break;
   default:
      return false;
   }

   int num_components = alu->def.num_components;

   uint8_t swizzle = alu->src[0].swizzle[0];

   for (int i = 1; i < num_components; i++)
      if (alu->src[0].swizzle[i] != swizzle)
         return true;

   return false;
}

static bool
lima_vec_to_regs_filter_cb(const nir_instr *instr, unsigned writemask,
                           const void *data)
{
   assert(writemask > 0);
   if (util_bitcount(writemask) == 1)
      return true;

   return !lima_alu_to_scalar_filter_cb(instr, data);
}

void
lima_program_optimize_fs_nir(struct nir_shader *s,
                             struct nir_lower_tex_options *tex_options)
{
   bool progress;

   NIR_PASS_V(s, nir_lower_fragcoord_wtrans);
   NIR_PASS_V(s, nir_lower_io,
	      nir_var_shader_in | nir_var_shader_out, type_size, 0);
   NIR_PASS_V(s, nir_lower_tex, tex_options);
   NIR_PASS_V(s, lima_nir_lower_txp);

   do {
      progress = false;
      NIR_PASS(progress, s, nir_opt_vectorize, NULL, NULL);
   } while (progress);

   do {
      progress = false;

      NIR_PASS_V(s, nir_lower_vars_to_ssa);
      NIR_PASS(progress, s, nir_lower_alu_to_scalar, lima_alu_to_scalar_filter_cb, NULL);
      NIR_PASS(progress, s, nir_copy_prop);
      NIR_PASS(progress, s, nir_opt_remove_phis);
      NIR_PASS(progress, s, nir_opt_dce);
      NIR_PASS(progress, s, nir_opt_dead_cf);
      NIR_PASS(progress, s, nir_opt_cse);
      NIR_PASS(progress, s, nir_opt_peephole_select, 8, true, true);
      NIR_PASS(progress, s, nir_opt_algebraic);
      NIR_PASS(progress, s, nir_opt_constant_folding);
      NIR_PASS(progress, s, nir_opt_undef);
      NIR_PASS(progress, s, nir_opt_loop_unroll);
      NIR_PASS(progress, s, lima_nir_split_load_input);
   } while (progress);

   NIR_PASS_V(s, nir_lower_int_to_float);
   NIR_PASS_V(s, nir_lower_bool_to_float, true);

   /* Some ops must be lowered after being converted from int ops,
    * so re-run nir_opt_algebraic after int lowering. */
   do {
      progress = false;
      NIR_PASS(progress, s, nir_opt_algebraic);
   } while (progress);

   /* Must be run after optimization loop */
   NIR_PASS_V(s, lima_nir_scale_trig);

   NIR_PASS_V(s, nir_copy_prop);
   NIR_PASS_V(s, nir_opt_dce);

   NIR_PASS_V(s, nir_convert_from_ssa, true);
   NIR_PASS_V(s, nir_remove_dead_variables, nir_var_function_temp, NULL);

   NIR_PASS_V(s, nir_move_vec_src_uses_to_dest, false);
   NIR_PASS_V(s, nir_lower_vec_to_regs, lima_vec_to_regs_filter_cb, NULL);

   NIR_PASS_V(s, nir_opt_dce); /* clean up any new dead code from vec to movs */

   NIR_PASS_V(s, lima_nir_duplicate_load_uniforms);
   NIR_PASS_V(s, lima_nir_duplicate_load_inputs);
   NIR_PASS_V(s, lima_nir_duplicate_load_consts);

   NIR_PASS_V(s, nir_legacy_trivialize, true);

   nir_sweep(s);
}

static bool
lima_fs_compile_shader(struct lima_context *ctx,
                       struct lima_fs_key *key,
                       struct lima_fs_uncompiled_shader *ufs,
                       struct lima_fs_compiled_shader *fs)
{
   struct lima_screen *screen = lima_screen(ctx->base.screen);
   nir_shader *nir = nir_shader_clone(fs, ufs->base.ir.nir);

   struct nir_lower_tex_options tex_options = {
      .swizzle_result = ~0u,
      .lower_invalid_implicit_lod = true,
   };

   for (int i = 0; i < ARRAY_SIZE(key->tex); i++) {
      for (int j = 0; j < 4; j++)
         tex_options.swizzles[i][j] = key->tex[i].swizzle[j];
   }

   lima_program_optimize_fs_nir(nir, &tex_options);

   if (lima_debug & LIMA_DEBUG_PP)
      nir_print_shader(nir, stdout);

   if (!ppir_compile_nir(fs, nir, screen->pp_ra, &ctx->base.debug)) {
      ralloc_free(nir);
      return false;
   }

   fs->state.uses_discard = nir->info.fs.uses_discard;
   ralloc_free(nir);

   return true;
}

static bool
lima_fs_upload_shader(struct lima_context *ctx,
                      struct lima_fs_compiled_shader *fs)
{
   static const uint32_t pp_clear_program[] = {
      PP_CLEAR_PROGRAM
   };
   int shader_size = sizeof(pp_clear_program);
   void *shader = (void *)pp_clear_program;
   struct lima_screen *screen = lima_screen(ctx->base.screen);

   if (fs->state.shader_size) {
      shader_size = fs->state.shader_size;
      shader = fs->shader;
   }

   fs->bo = lima_bo_create(screen, shader_size, 0);
   if (!fs->bo) {
      fprintf(stderr, "lima: create fs shader bo fail\n");
      return false;
   }

   memcpy(lima_bo_map(fs->bo), shader, shader_size);

   return true;
}

static struct lima_fs_compiled_shader *
lima_get_compiled_fs(struct lima_context *ctx,
                     struct lima_fs_uncompiled_shader *ufs,
                     struct lima_fs_key *key)
{
   struct lima_screen *screen = lima_screen(ctx->base.screen);
   struct hash_table *ht;
   uint32_t key_size;

   ht = ctx->fs_cache;
   key_size = sizeof(struct lima_fs_key);

   struct hash_entry *entry = _mesa_hash_table_search(ht, key);
   if (entry)
      return entry->data;

   /* Not on memory cache, try disk cache */
   struct lima_fs_compiled_shader *fs =
      lima_fs_disk_cache_retrieve(screen->disk_cache, key);

   if (!fs) {
      /* Not on disk cache, compile and insert into disk cache*/
      fs = rzalloc(NULL, struct lima_fs_compiled_shader);
      if (!fs)
         return NULL;

      if (!lima_fs_compile_shader(ctx, key, ufs, fs))
         goto err;

      lima_fs_disk_cache_store(screen->disk_cache, key, fs);
   }

   if (!lima_fs_upload_shader(ctx, fs))
      goto err;

   ralloc_free(fs->shader);
   fs->shader = NULL;

   /* Insert into memory cache */
   struct lima_key *dup_key;
   dup_key = rzalloc_size(fs, key_size);
   memcpy(dup_key, key, key_size);
   _mesa_hash_table_insert(ht, dup_key, fs);

   return fs;

err:
   ralloc_free(fs);
   return NULL;
}

static void *
lima_create_fs_state(struct pipe_context *pctx,
                     const struct pipe_shader_state *cso)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_fs_uncompiled_shader *so = rzalloc(NULL, struct lima_fs_uncompiled_shader);

   if (!so)
      return NULL;

   nir_shader *nir;
   if (cso->type == PIPE_SHADER_IR_NIR)
      /* The backend takes ownership of the NIR shader on state
       * creation. */
      nir = cso->ir.nir;
   else {
      assert(cso->type == PIPE_SHADER_IR_TGSI);

      nir = tgsi_to_nir(cso->tokens, pctx->screen, false);
   }

   so->base.type = PIPE_SHADER_IR_NIR;
   so->base.ir.nir = nir;

   /* Serialize the NIR to a binary blob that we can hash for the disk
    * cache.  Drop unnecessary information (like variable names)
    * so the serialized NIR is smaller, and also to let us detect more
    * isomorphic shaders when hashing, increasing cache hits.
    */
   struct blob blob;
   blob_init(&blob);
   nir_serialize(&blob, nir, true);
   _mesa_sha1_compute(blob.data, blob.size, so->nir_sha1);
   blob_finish(&blob);

   if (lima_debug & LIMA_DEBUG_PRECOMPILE) {
      /* Trigger initial compilation with default settings */
      struct lima_fs_key key;
      memset(&key, 0, sizeof(key));
      memcpy(key.nir_sha1, so->nir_sha1, sizeof(so->nir_sha1));
      for (int i = 0; i < ARRAY_SIZE(key.tex); i++) {
         for (int j = 0; j < 4; j++)
            key.tex[i].swizzle[j] = j;
      }
      lima_get_compiled_fs(ctx, so, &key);
   }

   return so;
}

static void
lima_bind_fs_state(struct pipe_context *pctx, void *hwcso)
{
   struct lima_context *ctx = lima_context(pctx);

   ctx->uncomp_fs = hwcso;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_UNCOMPILED_FS;
}

static void
lima_delete_fs_state(struct pipe_context *pctx, void *hwcso)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_fs_uncompiled_shader *so = hwcso;

   hash_table_foreach(ctx->fs_cache, entry) {
      const struct lima_fs_key *key = entry->key;
      if (!memcmp(key->nir_sha1, so->nir_sha1, sizeof(so->nir_sha1))) {
         struct lima_fs_compiled_shader *fs = entry->data;
         _mesa_hash_table_remove(ctx->fs_cache, entry);
         if (fs->bo)
            lima_bo_unreference(fs->bo);

         if (fs == ctx->fs)
            ctx->fs = NULL;

         ralloc_free(fs);
      }
   }

   ralloc_free(so->base.ir.nir);
   ralloc_free(so);
}

static bool
lima_vs_compile_shader(struct lima_context *ctx,
                       struct lima_vs_key *key,
                       struct lima_vs_uncompiled_shader *uvs,
                       struct lima_vs_compiled_shader *vs)
{
   nir_shader *nir = nir_shader_clone(vs, uvs->base.ir.nir);

   lima_program_optimize_vs_nir(nir);

   if (lima_debug & LIMA_DEBUG_GP)
      nir_print_shader(nir, stdout);

   if (!gpir_compile_nir(vs, nir, &ctx->base.debug)) {
      ralloc_free(nir);
      return false;
   }

   ralloc_free(nir);

   return true;
}

static bool
lima_vs_upload_shader(struct lima_context *ctx,
                      struct lima_vs_compiled_shader *vs)
{
   struct lima_screen *screen = lima_screen(ctx->base.screen);
   vs->bo = lima_bo_create(screen, vs->state.shader_size, 0);
   if (!vs->bo) {
      fprintf(stderr, "lima: create vs shader bo fail\n");
      return false;
   }

   memcpy(lima_bo_map(vs->bo), vs->shader, vs->state.shader_size);

   return true;
}

static struct lima_vs_compiled_shader *
lima_get_compiled_vs(struct lima_context *ctx,
                     struct lima_vs_uncompiled_shader *uvs,
                     struct lima_vs_key *key)
{
   struct lima_screen *screen = lima_screen(ctx->base.screen);
   struct hash_table *ht;
   uint32_t key_size;

   ht = ctx->vs_cache;
   key_size = sizeof(struct lima_vs_key);

   struct hash_entry *entry = _mesa_hash_table_search(ht, key);
   if (entry)
      return entry->data;

   /* Not on memory cache, try disk cache */
   struct lima_vs_compiled_shader *vs =
      lima_vs_disk_cache_retrieve(screen->disk_cache, key);

   if (!vs) {
      /* Not on disk cache, compile and insert into disk cache */
      vs = rzalloc(NULL, struct lima_vs_compiled_shader);
      if (!vs)
         return NULL;
      if (!lima_vs_compile_shader(ctx, key, uvs, vs))
         goto err;

      lima_vs_disk_cache_store(screen->disk_cache, key, vs);
   }

   if (!lima_vs_upload_shader(ctx, vs))
      goto err;

   ralloc_free(vs->shader);
   vs->shader = NULL;

   struct lima_key *dup_key;
   dup_key = rzalloc_size(vs, key_size);
   memcpy(dup_key, key, key_size);
   _mesa_hash_table_insert(ht, dup_key, vs);

   return vs;

err:
   ralloc_free(vs);
   return NULL;
}

bool
lima_update_vs_state(struct lima_context *ctx)
{
   if (!(ctx->dirty & LIMA_CONTEXT_DIRTY_UNCOMPILED_VS)) {
      return true;
   }

   struct lima_vs_key local_key;
   struct lima_vs_key *key = &local_key;
   memset(key, 0, sizeof(*key));
   memcpy(key->nir_sha1, ctx->uncomp_vs->nir_sha1,
          sizeof(ctx->uncomp_vs->nir_sha1));

   struct lima_vs_compiled_shader *old_vs = ctx->vs;
   struct lima_vs_compiled_shader *vs = lima_get_compiled_vs(ctx,
                                                             ctx->uncomp_vs,
                                                             key);
   if (!vs)
      return false;

   ctx->vs = vs;

   if (ctx->vs != old_vs)
      ctx->dirty |= LIMA_CONTEXT_DIRTY_COMPILED_VS;

   return true;
}

bool
lima_update_fs_state(struct lima_context *ctx)
{
   if (!(ctx->dirty & (LIMA_CONTEXT_DIRTY_UNCOMPILED_FS |
                       LIMA_CONTEXT_DIRTY_TEXTURES))) {
      return true;
   }

   struct lima_texture_stateobj *lima_tex = &ctx->tex_stateobj;
   struct lima_fs_key local_key;
   struct lima_fs_key *key = &local_key;
   memset(key, 0, sizeof(*key));
   memcpy(key->nir_sha1, ctx->uncomp_fs->nir_sha1,
          sizeof(ctx->uncomp_fs->nir_sha1));

   uint8_t identity[4] = { PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y,
                           PIPE_SWIZZLE_Z, PIPE_SWIZZLE_W };
   for (int i = 0; i < lima_tex->num_textures; i++) {
      struct lima_sampler_view *sampler = lima_sampler_view(lima_tex->textures[i]);
      if (!sampler) {
         memcpy(key->tex[i].swizzle, identity, 4);
         continue;
      }
      for (int j = 0; j < 4; j++)
         key->tex[i].swizzle[j] = sampler->swizzle[j];
   }

   /* Fill rest with identity swizzle */
   for (int i = lima_tex->num_textures; i < ARRAY_SIZE(key->tex); i++)
      memcpy(key->tex[i].swizzle, identity, 4);

   struct lima_fs_compiled_shader *old_fs = ctx->fs;

   struct lima_fs_compiled_shader *fs = lima_get_compiled_fs(ctx,
                                                             ctx->uncomp_fs,
                                                             key);
   if (!fs)
      return false;

   ctx->fs = fs;

   if (ctx->fs != old_fs)
      ctx->dirty |= LIMA_CONTEXT_DIRTY_COMPILED_FS;

   return true;
}

static void *
lima_create_vs_state(struct pipe_context *pctx,
                     const struct pipe_shader_state *cso)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_vs_uncompiled_shader *so = rzalloc(NULL, struct lima_vs_uncompiled_shader);

   if (!so)
      return NULL;

   nir_shader *nir;
   if (cso->type == PIPE_SHADER_IR_NIR)
      /* The backend takes ownership of the NIR shader on state
       * creation. */
      nir = cso->ir.nir;
   else {
      assert(cso->type == PIPE_SHADER_IR_TGSI);

      nir = tgsi_to_nir(cso->tokens, pctx->screen, false);
   }

   so->base.type = PIPE_SHADER_IR_NIR;
   so->base.ir.nir = nir;

   /* Serialize the NIR to a binary blob that we can hash for the disk
    * cache.  Drop unnecessary information (like variable names)
    * so the serialized NIR is smaller, and also to let us detect more
    * isomorphic shaders when hashing, increasing cache hits.
    */
   struct blob blob;
   blob_init(&blob);
   nir_serialize(&blob, nir, true);
   _mesa_sha1_compute(blob.data, blob.size, so->nir_sha1);
   blob_finish(&blob);

   if (lima_debug & LIMA_DEBUG_PRECOMPILE) {
      /* Trigger initial compilation with default settings */
      struct lima_vs_key key;
      memset(&key, 0, sizeof(key));
      memcpy(key.nir_sha1, so->nir_sha1, sizeof(so->nir_sha1));
      lima_get_compiled_vs(ctx, so, &key);
   }

   return so;
}

static void
lima_bind_vs_state(struct pipe_context *pctx, void *hwcso)
{
   struct lima_context *ctx = lima_context(pctx);

   ctx->uncomp_vs = hwcso;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_UNCOMPILED_VS;
}

static void
lima_delete_vs_state(struct pipe_context *pctx, void *hwcso)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_vs_uncompiled_shader *so = hwcso;

   hash_table_foreach(ctx->vs_cache, entry) {
      const struct lima_vs_key *key = entry->key;
      if (!memcmp(key->nir_sha1, so->nir_sha1, sizeof(so->nir_sha1))) {
         struct lima_vs_compiled_shader *vs = entry->data;
         _mesa_hash_table_remove(ctx->vs_cache, entry);
         if (vs->bo)
            lima_bo_unreference(vs->bo);

         if (vs == ctx->vs)
            ctx->vs = NULL;

         ralloc_free(vs);
      }
   }

   ralloc_free(so->base.ir.nir);
   ralloc_free(so);
}

static uint32_t
lima_fs_cache_hash(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct lima_fs_key));
}

static uint32_t
lima_vs_cache_hash(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct lima_vs_key));
}

static bool
lima_fs_cache_compare(const void *key1, const void *key2)
{
   return memcmp(key1, key2, sizeof(struct lima_fs_key)) == 0;
}

static bool
lima_vs_cache_compare(const void *key1, const void *key2)
{
   return memcmp(key1, key2, sizeof(struct lima_vs_key)) == 0;
}

void
lima_program_init(struct lima_context *ctx)
{
   ctx->base.create_fs_state = lima_create_fs_state;
   ctx->base.bind_fs_state = lima_bind_fs_state;
   ctx->base.delete_fs_state = lima_delete_fs_state;

   ctx->base.create_vs_state = lima_create_vs_state;
   ctx->base.bind_vs_state = lima_bind_vs_state;
   ctx->base.delete_vs_state = lima_delete_vs_state;

   ctx->fs_cache = _mesa_hash_table_create(ctx, lima_fs_cache_hash,
                                           lima_fs_cache_compare);
   ctx->vs_cache = _mesa_hash_table_create(ctx, lima_vs_cache_hash,
                                           lima_vs_cache_compare);
}

void
lima_program_fini(struct lima_context *ctx)
{
   hash_table_foreach(ctx->vs_cache, entry) {
      struct lima_vs_compiled_shader *vs = entry->data;
      if (vs->bo)
         lima_bo_unreference(vs->bo);
      ralloc_free(vs);
      _mesa_hash_table_remove(ctx->vs_cache, entry);
   }

   hash_table_foreach(ctx->fs_cache, entry) {
      struct lima_fs_compiled_shader *fs = entry->data;
      if (fs->bo)
         lima_bo_unreference(fs->bo);
      ralloc_free(fs);
      _mesa_hash_table_remove(ctx->fs_cache, entry);
   }
}
