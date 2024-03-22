/*
 * Copyright © 2023 Valve Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "lp_context.h"
#include "lp_texture_handle.h"
#include "lp_screen.h"

#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_nir.h"

#include "nir.h"
#include "nir_builder.h"

#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/mesa-sha1.h"

static const char *image_function_base_hash = "8ca89d7a4ab5830be6a1ba1140844081235b01164a8fce8316ca6a2f81f1a899";
static const char *sample_function_base_hash = "0789b032c4a1ddba086e07496fe2a992b1ee08f78c0884a2923564b1ed52b9cc";
static const char *size_function_base_hash = "6d249ab9c1106c68b87ec9fdb5ade28368171d27f221c687f32ae1544231d2fe";

static void
llvmpipe_register_texture(struct llvmpipe_context *ctx, struct lp_static_texture_state *state, bool sampled);

static void
llvmpipe_register_sampler(struct llvmpipe_context *ctx, struct lp_static_sampler_state *state);

static uint64_t
llvmpipe_create_texture_handle(struct pipe_context *pctx, struct pipe_sampler_view *view, const struct pipe_sampler_state *sampler)
{
   struct llvmpipe_context *ctx = llvmpipe_context(pctx);
   struct lp_sampler_matrix *matrix = &ctx->sampler_matrix;

   struct lp_texture_handle *handle = calloc(1, sizeof(struct lp_texture_handle));

   if (view) {
      struct lp_static_texture_state state;
      lp_sampler_static_texture_state(&state, view);

      /* Trade a bit of performance for potentially less sampler/texture combinations. */
      state.pot_width = false;
      state.pot_height = false;
      state.pot_depth = false;

      llvmpipe_register_texture(ctx, &state, true);

      bool found = false;
      for (uint32_t i = 0; i < matrix->texture_count; i++) {
         if (!memcmp(&matrix->textures[i]->state, &state, sizeof(struct lp_static_texture_state))) {
            handle->functions = matrix->textures[i];
            found = true;
            break;
         }
      }
      assert(found);
   }

   if (sampler) {
      struct lp_static_sampler_state state;
      lp_sampler_static_sampler_state(&state, sampler);

      llvmpipe_register_sampler(ctx, &state);

      bool found = false;
      for (uint32_t i = 0; i < matrix->sampler_count; i++) {
         if (!memcmp(matrix->samplers + i, &state, sizeof(struct lp_static_sampler_state))) {
            handle->sampler_index = i;
            found = true;
            break;
         }
      }
      assert(found);
   }

   return (uint64_t)(uintptr_t)handle;
}

static void
llvmpipe_delete_texture_handle(struct pipe_context *pctx, uint64_t _handle)
{
   if (!_handle)
      return;

   struct lp_texture_handle *handle = (void *)(uintptr_t)_handle;

   struct lp_texture_functions *functions = handle->functions;
   if (functions) {
      assert(functions->ref_count);
      functions->ref_count--;
   }

   free(handle);
}

static uint64_t
llvmpipe_create_image_handle(struct pipe_context *pctx, const struct pipe_image_view *view)
{
   struct llvmpipe_context *ctx = llvmpipe_context(pctx);
   struct lp_sampler_matrix *matrix = &ctx->sampler_matrix;

   struct lp_texture_handle *handle = calloc(1, sizeof(struct lp_texture_handle));

   struct lp_static_texture_state state;
   lp_sampler_static_texture_state_image(&state, view);

   /* Trade a bit of performance for potentially less sampler/texture combinations. */
   state.pot_width = false;
   state.pot_height = false;
   state.pot_depth = false;

   if (view->u.tex.first_layer == view->u.tex.last_layer) {
      if (state.target == PIPE_TEXTURE_1D_ARRAY)
         state.target = PIPE_TEXTURE_1D;
      else if (state.target == PIPE_TEXTURE_2D_ARRAY || state.target == PIPE_TEXTURE_3D)
         state.target = PIPE_TEXTURE_2D;
      else if (state.target == PIPE_TEXTURE_CUBE_ARRAY)
         state.target = PIPE_TEXTURE_CUBE;
   }

   llvmpipe_register_texture(ctx, &state, false);

   bool found = false;
   for (uint32_t i = 0; i < matrix->texture_count; i++) {
      if (!memcmp(&matrix->textures[i]->state, &state, sizeof(struct lp_static_texture_state))) {
         handle->functions = matrix->textures[i];
         found = true;
         break;
      }
   }
   assert(found);

   return (uint64_t)(uintptr_t)handle;
}

static void
llvmpipe_delete_image_handle(struct pipe_context *pctx, uint64_t handle)
{
   free((void *)(uintptr_t)handle);
}

void
llvmpipe_init_sampler_matrix(struct llvmpipe_context *ctx)
{
   ctx->pipe.create_texture_handle = llvmpipe_create_texture_handle;
   ctx->pipe.delete_texture_handle = llvmpipe_delete_texture_handle;
   ctx->pipe.create_image_handle = llvmpipe_create_image_handle;
   ctx->pipe.delete_image_handle = llvmpipe_delete_image_handle;

   util_dynarray_init(&ctx->sampler_matrix.gallivms, NULL);
}

void
llvmpipe_sampler_matrix_destroy(struct llvmpipe_context *ctx)
{
   struct lp_sampler_matrix *matrix = &ctx->sampler_matrix;

   free(matrix->samplers);

   for (uint32_t texture_index = 0; texture_index < matrix->texture_count; texture_index++) {
      struct lp_texture_functions *texture = matrix->textures[texture_index];

      uint32_t sampler_count = texture->sampler_count;
      if (texture->state.format == PIPE_FORMAT_NONE)
         sampler_count = MIN2(sampler_count, 1);

      for (uint32_t sampler_index = 0; sampler_index < sampler_count; sampler_index++)
         free(texture->sample_functions[sampler_index]);

      free(texture->sample_functions);
      free(texture->fetch_functions);
      free(texture->image_functions);
      free(texture);
   }
   free(matrix->textures);

   util_dynarray_foreach (&ctx->sampler_matrix.gallivms, struct gallivm_state *, gallivm)
      gallivm_destroy(*gallivm);

   util_dynarray_fini(&ctx->sampler_matrix.gallivms);
}

static void *
compile_function(struct llvmpipe_context *ctx, struct gallivm_state *gallivm, LLVMValueRef function,
                 bool needs_caching,
                 uint8_t cache_key[SHA1_DIGEST_LENGTH])
{
   gallivm_verify_function(gallivm, function);
   gallivm_compile_module(gallivm);

   void *function_ptr = func_to_pointer(gallivm_jit_function(gallivm, function));

   if (needs_caching)
      lp_disk_cache_insert_shader(llvmpipe_screen(ctx->pipe.screen), gallivm->cache, cache_key);

   gallivm_free_ir(gallivm);

   util_dynarray_append(&ctx->sampler_matrix.gallivms, struct gallivm_state *, gallivm);

   return function_ptr;
}

static void *
compile_image_function(struct llvmpipe_context *ctx, struct lp_static_texture_state *texture, uint32_t op)
{
   const struct util_format_description *desc = util_format_description(texture->format);
   if (desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS && !lp_storage_render_image_format_supported(texture->format))
      return NULL;
   
   bool ms = op >= LP_TOTAL_IMAGE_OP_COUNT / 2;
   if (ms)
      op -= LP_TOTAL_IMAGE_OP_COUNT / 2;

   struct lp_img_params params = { 0 };

   params.img_op = op;
   if (op >= LP_IMG_OP_COUNT - 1) {
      params.img_op = LP_IMG_ATOMIC;
      params.op = op - (LP_IMG_OP_COUNT - 1);
   } else if (op != LP_IMG_LOAD && op != LP_IMG_STORE) {
      params.img_op = LP_IMG_ATOMIC_CAS;
   }

   /* Loads need to support a wider range of formats for input attachments. */
   if (params.img_op != LP_IMG_LOAD)
      if (texture->format != PIPE_FORMAT_NONE && !lp_storage_image_format_supported(texture->format))
         return NULL;

   uint8_t cache_key[SHA1_DIGEST_LENGTH];
   struct mesa_sha1 hash_ctx;
   _mesa_sha1_init(&hash_ctx);
   _mesa_sha1_update(&hash_ctx, image_function_base_hash, strlen(image_function_base_hash));
   _mesa_sha1_update(&hash_ctx, texture, sizeof(*texture));
   _mesa_sha1_update(&hash_ctx, &op, sizeof(op));
   _mesa_sha1_update(&hash_ctx, &ms, sizeof(ms));
   _mesa_sha1_final(&hash_ctx, cache_key);

   struct lp_cached_code cached = { 0 };
   lp_disk_cache_find_shader(llvmpipe_screen(ctx->pipe.screen), &cached, cache_key);
   bool needs_caching = !cached.data_size;

   struct gallivm_state *gallivm = gallivm_create("sample_function", ctx->context, &cached);

   struct lp_image_static_state state = {
      .image_state = *texture,
   };
   struct lp_build_image_soa *image_soa = lp_bld_llvm_image_soa_create(&state, 1);

   struct lp_type type;
   memset(&type, 0, sizeof type);
   type.floating = true;      /* floating point values */
   type.sign = true;          /* values are signed */
   type.norm = false;         /* values are not limited to [0,1] or [-1,1] */
   type.width = 32;           /* 32-bit float */
   type.length = MIN2(lp_native_vector_width / 32, 16); /* n*4 elements per vector */

   struct lp_compute_shader_variant cs = { .gallivm = gallivm };
   lp_jit_init_cs_types(&cs);

   params.type = type;
   params.target = texture->target;
   params.resources_type = cs.jit_resources_type;
   params.format = texture->format;

   LLVMTypeRef function_type = lp_build_image_function_type(gallivm, &params, ms);
   if (!function_type) {
      free(image_soa);
      gallivm_destroy(gallivm);
      return NULL;
   }

   LLVMValueRef function = LLVMAddFunction(gallivm->module, "image", function_type);

   uint32_t arg_index = 0;

   gallivm->texture_descriptor = LLVMGetParam(function, arg_index++);

   if (params.img_op != LP_IMG_LOAD)
      params.exec_mask = LLVMGetParam(function, arg_index++);

   LLVMValueRef coords[3];
   params.coords = coords;
   for (uint32_t i = 0; i < 3; i++)
      coords[i] = LLVMGetParam(function, arg_index++);

   if (ms)
      params.ms_index = LLVMGetParam(function, arg_index++);

   if (params.img_op != LP_IMG_LOAD)
      for (uint32_t i = 0; i < 4; i++)
         params.indata[i] = LLVMGetParam(function, arg_index++);

   if (params.img_op == LP_IMG_ATOMIC_CAS)
      for (uint32_t i = 0; i < 4; i++)
         params.indata2[i] = LLVMGetParam(function, arg_index++);

   LLVMBuilderRef old_builder = gallivm->builder;
   LLVMBasicBlockRef block = LLVMAppendBasicBlockInContext(gallivm->context, function, "entry");
   gallivm->builder = LLVMCreateBuilderInContext(gallivm->context);
   LLVMPositionBuilderAtEnd(gallivm->builder, block);

   LLVMValueRef outdata[4] = { 0 };
   lp_build_img_op_soa(texture, lp_build_image_soa_dynamic_state(image_soa), gallivm, &params, outdata);

   for (uint32_t i = 1; i < 4; i++)
      if (!outdata[i])
         outdata[i] = outdata[0];

   if (params.img_op != LP_IMG_STORE)
      LLVMBuildAggregateRet(gallivm->builder, outdata, 4);
   else
      LLVMBuildRetVoid(gallivm->builder);

   LLVMDisposeBuilder(gallivm->builder);
   gallivm->builder = old_builder;

   free(image_soa);

   return compile_function(ctx, gallivm, function, needs_caching, cache_key);
}

static void *
compile_sample_function(struct llvmpipe_context *ctx, struct lp_static_texture_state *texture,
                        struct lp_static_sampler_state *sampler, uint32_t sample_key)
{
   enum lp_sampler_lod_control lod_control = (sample_key & LP_SAMPLER_LOD_CONTROL_MASK) >> LP_SAMPLER_LOD_CONTROL_SHIFT;

   bool supported = true;
   if (texture->format != PIPE_FORMAT_NONE) {
      enum lp_sampler_op_type op_type = (sample_key & LP_SAMPLER_OP_TYPE_MASK) >> LP_SAMPLER_OP_TYPE_SHIFT;
      if (op_type != LP_SAMPLER_OP_LODQ)
         if ((sampler->compare_mode == PIPE_TEX_COMPARE_NONE) == !!(sample_key & LP_SAMPLER_SHADOW))
            supported = false;

      /* Skip integer formats which would cause a type mismatch in the compare function. */
      const struct util_format_description *desc = util_format_description(texture->format);
      struct lp_type texel_type = {
         .floating = true,
         .width = 32,
         .length = 1,
      };
      texel_type = lp_build_texel_type(texel_type, desc);
      if ((sample_key & LP_SAMPLER_SHADOW) && !texel_type.floating)
         supported = false;

      if (texture_dims(texture->target) != 2 && op_type == LP_SAMPLER_OP_GATHER)
         supported = false;

      if (op_type != LP_SAMPLER_OP_FETCH) {
         if (!sampler->normalized_coords) {
            if (texture->target != PIPE_TEXTURE_1D && texture->target != PIPE_TEXTURE_2D &&
                texture->target != PIPE_TEXTURE_1D_ARRAY && texture->target != PIPE_TEXTURE_2D_ARRAY)
               supported = false;

            if (!texture->level_zero_only)
               supported = false;
         }
      }

      if (util_format_is_pure_integer(texture->format) &&
          (sampler->min_img_filter == PIPE_TEX_FILTER_LINEAR ||
           sampler->min_mip_filter == PIPE_TEX_MIPFILTER_LINEAR ||
           sampler->mag_img_filter == PIPE_TEX_FILTER_LINEAR))
         supported = false;

      if (sampler->aniso) {
         if (texture_dims(texture->target) != 2)
            supported = false;

         if (util_format_is_pure_integer(texture->format))
            supported = false;
      }

      if (util_format_get_num_planes(texture->format) > 1)
         return NULL;

      uint32_t bind = op_type == LP_SAMPLER_OP_FETCH ? PIPE_BIND_CONSTANT_BUFFER : PIPE_BIND_SAMPLER_VIEW;
      if (!ctx->pipe.screen->is_format_supported(ctx->pipe.screen, texture->format, texture->target, 0, 0, bind))
         supported = false;
   }

   uint8_t cache_key[SHA1_DIGEST_LENGTH];
   struct mesa_sha1 hash_ctx;
   _mesa_sha1_init(&hash_ctx);
   _mesa_sha1_update(&hash_ctx, sample_function_base_hash, strlen(sample_function_base_hash));
   _mesa_sha1_update(&hash_ctx, texture, sizeof(*texture));
   _mesa_sha1_update(&hash_ctx, sampler, sizeof(*sampler));
   _mesa_sha1_update(&hash_ctx, &sample_key, sizeof(sample_key));
   _mesa_sha1_final(&hash_ctx, cache_key);

   struct lp_cached_code cached = { 0 };
   lp_disk_cache_find_shader(llvmpipe_screen(ctx->pipe.screen), &cached, cache_key);
   bool needs_caching = !cached.data_size;

   struct gallivm_state *gallivm = gallivm_create("sample_function", ctx->context, &cached);

   struct lp_sampler_static_state state = {
      .texture_state = *texture,
      .sampler_state = *sampler,
   };
   struct lp_build_sampler_soa *sampler_soa = lp_llvm_sampler_soa_create(&state, 1);

   struct lp_type type;
   memset(&type, 0, sizeof type);
   type.floating = true;      /* floating point values */
   type.sign = true;          /* values are signed */
   type.norm = false;         /* values are not limited to [0,1] or [-1,1] */
   type.width = 32;           /* 32-bit float */
   type.length = MIN2(lp_native_vector_width / 32, 16); /* n*4 elements per vector */

   struct lp_compute_shader_variant cs = { .gallivm = gallivm };
   lp_jit_init_cs_types(&cs);

   LLVMTypeRef function_type = lp_build_sample_function_type(gallivm, sample_key);
   LLVMValueRef function = LLVMAddFunction(gallivm->module, "sample", function_type);

   uint32_t arg_index = 0;

   gallivm->texture_descriptor = LLVMGetParam(function, arg_index++);
   gallivm->sampler_descriptor = LLVMGetParam(function, arg_index++);

   LLVMValueRef aniso_filter_table = LLVMGetParam(function, arg_index++);

   LLVMValueRef coords[5];
   for (unsigned i = 0; i < 4; i++)
      coords[i] = LLVMGetParam(function, arg_index++);

   if (sample_key & LP_SAMPLER_SHADOW)
      coords[4] = LLVMGetParam(function, arg_index++);
   else
      coords[4] = lp_build_undef(gallivm, type);

   LLVMValueRef ms_index = NULL;
   if (sample_key & LP_SAMPLER_FETCH_MS)
      ms_index = LLVMGetParam(function, arg_index++);

   LLVMValueRef offsets[3] = { 0 };
   if (sample_key & LP_SAMPLER_OFFSETS)
      for (unsigned i = 0; i < 3; i++)
         offsets[i] = LLVMGetParam(function, arg_index++);

   LLVMValueRef lod = NULL;
   if (lod_control == LP_SAMPLER_LOD_BIAS || lod_control == LP_SAMPLER_LOD_EXPLICIT)
      lod = LLVMGetParam(function, arg_index++);

   LLVMBuilderRef old_builder = gallivm->builder;
   LLVMBasicBlockRef block = LLVMAppendBasicBlockInContext(gallivm->context, function, "entry");
   gallivm->builder = LLVMCreateBuilderInContext(gallivm->context);
   LLVMPositionBuilderAtEnd(gallivm->builder, block);

   LLVMValueRef texel_out[4] = { 0 };
   if (supported) {
      lp_build_sample_soa_code(gallivm, texture, sampler, lp_build_sampler_soa_dynamic_state(sampler_soa),
                               type, sample_key, 0, 0, cs.jit_resources_type, NULL, cs.jit_cs_thread_data_type,
                               NULL, coords, offsets, NULL, lod, ms_index, aniso_filter_table, texel_out);
   } else {
      lp_build_sample_nop(gallivm, lp_build_texel_type(type, util_format_description(texture->format)), coords, texel_out);
   }

   LLVMBuildAggregateRet(gallivm->builder, texel_out, 4);

   LLVMDisposeBuilder(gallivm->builder);
   gallivm->builder = old_builder;

   free(sampler_soa);

   return compile_function(ctx, gallivm, function, needs_caching, cache_key);
}

static void *
compile_size_function(struct llvmpipe_context *ctx, struct lp_static_texture_state *texture, bool samples)
{
   uint8_t cache_key[SHA1_DIGEST_LENGTH];
   struct mesa_sha1 hash_ctx;
   _mesa_sha1_init(&hash_ctx);
   _mesa_sha1_update(&hash_ctx, size_function_base_hash, strlen(size_function_base_hash));
   _mesa_sha1_update(&hash_ctx, texture, sizeof(*texture));
   _mesa_sha1_update(&hash_ctx, &samples, sizeof(samples));
   _mesa_sha1_final(&hash_ctx, cache_key);

   struct lp_cached_code cached = { 0 };
   lp_disk_cache_find_shader(llvmpipe_screen(ctx->pipe.screen), &cached, cache_key);
   bool needs_caching = !cached.data_size;

   struct gallivm_state *gallivm = gallivm_create("sample_function", ctx->context, &cached);

   struct lp_sampler_static_state state = {
      .texture_state = *texture,
   };
   struct lp_build_sampler_soa *sampler_soa = lp_llvm_sampler_soa_create(&state, 1);

   struct lp_type type;
   memset(&type, 0, sizeof type);
   type.floating = true;      /* floating point values */
   type.sign = true;          /* values are signed */
   type.norm = false;         /* values are not limited to [0,1] or [-1,1] */
   type.width = 32;           /* 32-bit float */
   type.length = MIN2(lp_native_vector_width / 32, 16); /* n*4 elements per vector */

   struct lp_compute_shader_variant cs = { .gallivm = gallivm };
   lp_jit_init_cs_types(&cs);

   struct lp_sampler_size_query_params params = {
      .int_type = lp_int_type(type),
      .target = texture->target,
      .resources_type = cs.jit_resources_type,
      .is_sviewinfo = true,
      .samples_only = samples,
      .ms = samples,
   };

   if (params.target == PIPE_TEXTURE_1D)
      params.target = PIPE_TEXTURE_1D_ARRAY;
   else if (params.target == PIPE_TEXTURE_2D)
      params.target = PIPE_TEXTURE_2D_ARRAY;
   else if (params.target == PIPE_TEXTURE_CUBE)
      params.target = PIPE_TEXTURE_CUBE_ARRAY;

   LLVMTypeRef function_type = lp_build_size_function_type(gallivm, &params);
   LLVMValueRef function = LLVMAddFunction(gallivm->module, "size", function_type);

   uint32_t arg_index = 0;

   gallivm->texture_descriptor = LLVMGetParam(function, arg_index++);

   if (!samples)
      params.explicit_lod = LLVMGetParam(function, arg_index++);

   LLVMBuilderRef old_builder = gallivm->builder;
   LLVMBasicBlockRef block = LLVMAppendBasicBlockInContext(gallivm->context, function, "entry");
   gallivm->builder = LLVMCreateBuilderInContext(gallivm->context);
   LLVMPositionBuilderAtEnd(gallivm->builder, block);

   LLVMValueRef out_sizes[4] = { 0 };
   params.sizes_out = out_sizes;
   lp_build_size_query_soa(gallivm, texture, lp_build_sampler_soa_dynamic_state(sampler_soa), &params);

   for (uint32_t i = 0; i < 4; i++)
      if (!out_sizes[i])
         out_sizes[i] = lp_build_const_int_vec(gallivm, params.int_type, 0);

   LLVMBuildAggregateRet(gallivm->builder, out_sizes, 4);

   LLVMDisposeBuilder(gallivm->builder);
   gallivm->builder = old_builder;

   free(sampler_soa);

   return compile_function(ctx, gallivm, function, needs_caching, cache_key);
}

static void
compile_sample_functions(struct llvmpipe_context *ctx, struct lp_static_texture_state *texture,
                        struct lp_static_sampler_state *sampler, void ***dst)
{
   void **functions;
   if (*dst) {
      functions = *dst;
   } else {
      functions = calloc(LP_SAMPLE_KEY_COUNT, sizeof(void *));
      *dst = functions;
   }

   bool has_sampler = !!sampler;

   struct lp_static_sampler_state dummy_sampler = { 0 };
   if (!sampler)
      sampler = &dummy_sampler;

   struct lp_sampler_matrix *matrix = &ctx->sampler_matrix;
   for (uint32_t sample_key = 0; sample_key < LP_SAMPLE_KEY_COUNT; sample_key++) {
      if (!matrix->sampler_keys[sample_key])
         continue;

      enum lp_sampler_op_type op_type = (sample_key & LP_SAMPLER_OP_TYPE_MASK) >> LP_SAMPLER_OP_TYPE_SHIFT;
      if (has_sampler && op_type == LP_SAMPLER_OP_FETCH)
         continue;

      if (!functions[sample_key])
         functions[sample_key] = compile_sample_function(ctx, texture, sampler, sample_key);
   }
}

static void
llvmpipe_register_texture(struct llvmpipe_context *ctx, struct lp_static_texture_state *state, bool sampled)
{
   struct lp_sampler_matrix *matrix = &ctx->sampler_matrix;

   bool packed = true;
   uint32_t dst_index = matrix->texture_count;
   for (uint32_t i = 0; i < matrix->texture_count; i++) {
      if (memcmp(&matrix->textures[i]->state, state, sizeof(struct lp_static_texture_state)))
         continue;

      bool has_functions = sampled ? matrix->textures[i]->sampled : matrix->textures[i]->storage;

      uint32_t prev_ref_count = matrix->textures[i]->ref_count++;
      if (has_functions && prev_ref_count)
         return;

      packed = false;
      dst_index = i;
      break;
   }

   struct lp_texture_functions *entry;
   if (packed) {
      matrix->texture_count++;
      matrix->textures = realloc(matrix->textures, matrix->texture_count * sizeof(struct lp_texture_functions *));

      entry = calloc(1, sizeof(struct lp_texture_functions));
      matrix->textures[dst_index] = entry;

      entry->ref_count = 1;
      entry->state = *state;
      entry->image_functions = calloc(LP_TOTAL_IMAGE_OP_COUNT, sizeof(void **));
   } else {
      entry = matrix->textures[dst_index];
   }

   if (sampled)
      entry->sampled = true;
   else
      entry->storage = true;

   if (entry->sampled) {
      if (entry->sample_functions) {
         entry->sample_functions = realloc(entry->sample_functions, matrix->sampler_count * sizeof(void **));
         memset(entry->sample_functions + entry->sampler_count, 0, (matrix->sampler_count - entry->sampler_count) * sizeof(void **));
      } else {
         entry->sample_functions = calloc(matrix->sampler_count, sizeof(void **));
      }
      entry->sampler_count = matrix->sampler_count;

      if (state->format == PIPE_FORMAT_NONE) {
         if (matrix->sampler_count)
            compile_sample_functions(ctx, state, NULL, entry->sample_functions);
         for (uint32_t i = 1; i < matrix->sampler_count; i++)
            entry->sample_functions[i] = entry->sample_functions[0];
      } else {
         for (uint32_t i = 0; i < matrix->sampler_count; i++)
            compile_sample_functions(ctx, state, matrix->samplers + i, entry->sample_functions + i);
      }

      compile_sample_functions(ctx, state, NULL, &entry->fetch_functions);

      if (!entry->size_function)
         entry->size_function = compile_size_function(ctx, state, false);

      if (!entry->samples_function)
         entry->samples_function = compile_size_function(ctx, state, true);
   }

   if (entry->storage) {
      uint32_t image_op;
      BITSET_FOREACH_SET (image_op, matrix->image_ops, LP_TOTAL_IMAGE_OP_COUNT)
         if (!entry->image_functions[image_op])
            entry->image_functions[image_op] = compile_image_function(ctx, state, image_op);
   }
}

static void
llvmpipe_register_sampler(struct llvmpipe_context *ctx, struct lp_static_sampler_state *state)
{
   struct lp_sampler_matrix *matrix = &ctx->sampler_matrix;
   for (uint32_t i = 0; i < matrix->sampler_count; i++)
      if (!memcmp(matrix->samplers + i, state, sizeof(struct lp_static_sampler_state)))
         return;

   matrix->sampler_count++;
   matrix->samplers = realloc(matrix->samplers, matrix->sampler_count * sizeof(struct lp_static_sampler_state));

   matrix->samplers[matrix->sampler_count - 1] = *state;

   for (uint32_t i = 0; i < matrix->texture_count; i++) {
      struct lp_texture_functions *texture = matrix->textures[i];
      if (!texture->ref_count || !texture->sampled)
         continue;

      texture->sampler_count = matrix->sampler_count;
      texture->sample_functions = realloc(texture->sample_functions, matrix->sampler_count * sizeof(void **));

      void ***dst = texture->sample_functions + (matrix->sampler_count - 1);

      if (texture->state.format == PIPE_FORMAT_NONE)  {
         if (matrix->sampler_count == 1) {
            *dst = NULL;
            compile_sample_functions(ctx, &texture->state, NULL, dst);
         } else {
            *dst = texture->sample_functions[0];
         }

         continue;
      }

      *dst = NULL;
      compile_sample_functions(ctx, &texture->state, state, dst);
   }
}

static void
register_sample_key(struct llvmpipe_context *ctx, uint32_t sample_key)
{
   struct lp_sampler_matrix *matrix = &ctx->sampler_matrix;

   uint32_t prev_ref_count = matrix->sampler_keys[sample_key]++;
   if (prev_ref_count)
      return;

   for (uint32_t texture_index = 0; texture_index < matrix->texture_count; texture_index++) {
      struct lp_texture_functions *texture = matrix->textures[texture_index];
      if (!texture->ref_count || !texture->sampled)
         continue;

      enum lp_sampler_op_type op_type = (sample_key & LP_SAMPLER_OP_TYPE_MASK) >> LP_SAMPLER_OP_TYPE_SHIFT;
      if (op_type == LP_SAMPLER_OP_FETCH) {
         if (!texture->fetch_functions[sample_key]) {
            struct lp_static_sampler_state dummy_sampler = { 0 };
            texture->fetch_functions[sample_key] = compile_sample_function(ctx, &texture->state, &dummy_sampler, sample_key);
         }
         continue;
      }

      if (texture->state.format == PIPE_FORMAT_NONE) {
         if (matrix->sampler_count && !texture->sample_functions[0][sample_key]) {
            struct lp_static_sampler_state dummy_sampler = { 0 };
            texture->sample_functions[0][sample_key] = compile_sample_function(ctx, &texture->state, &dummy_sampler, sample_key);
         }
         continue;
      }

      for (uint32_t sampler_index = 0; sampler_index < matrix->sampler_count; sampler_index++) {
         if (!texture->sample_functions[sampler_index][sample_key]) {
            texture->sample_functions[sampler_index][sample_key] = compile_sample_function(
               ctx, &texture->state, matrix->samplers + sampler_index, sample_key);
         }
      }
   }
}

static void
unregister_sample_key(struct llvmpipe_context *ctx, uint32_t sample_key)
{
   struct lp_sampler_matrix *matrix = &ctx->sampler_matrix;

   assert(matrix->sampler_keys[sample_key]);
   matrix->sampler_keys[sample_key]--;
}

static void
register_image_op(struct llvmpipe_context *ctx, uint32_t op)
{
   struct lp_sampler_matrix *matrix = &ctx->sampler_matrix;
   if (BITSET_TEST(matrix->image_ops, op))
      return;

   BITSET_SET(matrix->image_ops, op);

   for (uint32_t texture_index = 0; texture_index < matrix->texture_count; texture_index++) {
      struct lp_texture_functions *texture = matrix->textures[texture_index];
      if (texture->ref_count && texture->storage)
         texture->image_functions[op] = compile_image_function(ctx, &texture->state, op);
   }
}

struct register_shader_state {
   struct llvmpipe_context *ctx;
   bool unregister;
};

static bool
register_instr(nir_builder *b, nir_instr *instr, void *_state)
{
   struct register_shader_state *state = _state;

   if (instr->type == nir_instr_type_tex) {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      uint32_t sample_key = lp_build_nir_sample_key(b->shader->info.stage, tex);

      if (state->unregister)
         unregister_sample_key(state->ctx, sample_key);
      else
         register_sample_key(state->ctx, sample_key);
   } else if (instr->type == nir_instr_type_intrinsic) {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

      struct lp_img_params params;
      lp_img_op_from_intrinsic(&params, intrin);

      if (params.img_op == -1)
         return false;

      uint32_t op = params.img_op;
      if (op == LP_IMG_ATOMIC_CAS)
         op--;
      else if (op == LP_IMG_ATOMIC)
         op = params.op + (LP_IMG_OP_COUNT - 1);

      if (nir_intrinsic_image_dim(intrin) == GLSL_SAMPLER_DIM_MS ||
          nir_intrinsic_image_dim(intrin) == GLSL_SAMPLER_DIM_SUBPASS_MS)
         op += LP_TOTAL_IMAGE_OP_COUNT / 2;

      register_image_op(state->ctx, op);
   }

   return false;
}

void
llvmpipe_register_shader(struct pipe_context *ctx, const struct pipe_shader_state *shader, bool unregister)
{
   if (shader->type != PIPE_SHADER_IR_NIR)
      return;

   struct register_shader_state state = {
      .ctx = llvmpipe_context(ctx),
      .unregister = unregister,
   };
   nir_shader_instructions_pass(shader->ir.nir, register_instr, nir_metadata_all, &state);
}
