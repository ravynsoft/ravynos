/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "draw_llvm.h"

#include "draw_context.h"
#include "draw_vs.h"
#include "draw_gs.h"

#include "gallivm/lp_bld_arit.h"
#include "gallivm/lp_bld_arit_overflow.h"
#include "gallivm/lp_bld_bitarit.h"
#include "gallivm/lp_bld_gather.h"
#include "gallivm/lp_bld_logic.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_coro.h"
#include "gallivm/lp_bld_swizzle.h"
#include "gallivm/lp_bld_struct.h"
#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_flow.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_tgsi.h"
#include "gallivm/lp_bld_nir.h"
#include "gallivm/lp_bld_printf.h"
#include "gallivm/lp_bld_intr.h"
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_pack.h"
#include "gallivm/lp_bld_format.h"
#include "gallivm/lp_bld_misc.h"
#include "gallivm/lp_bld_jit_sample.h"
#include "tgsi/tgsi_exec.h"
#include "tgsi/tgsi_dump.h"

#include "util/u_math.h"
#include "util/u_pointer.h"
#include "util/u_string.h"
#include "nir_serialize.h"
#include "util/mesa-sha1.h"
#define DEBUG_STORE 0


static void
draw_llvm_generate(struct draw_llvm *llvm, struct draw_llvm_variant *var);


struct draw_gs_llvm_iface {
   struct lp_build_gs_iface base;

   struct draw_gs_llvm_variant *variant;
   LLVMValueRef input;
};


static inline const struct draw_gs_llvm_iface *
draw_gs_llvm_iface(const struct lp_build_gs_iface *iface)
{
   return (const struct draw_gs_llvm_iface *)iface;
}


struct draw_tcs_llvm_iface {
   struct lp_build_tcs_iface base;

   struct draw_tcs_llvm_variant *variant;
   LLVMValueRef input;
   LLVMValueRef output;
};


static inline const struct draw_tcs_llvm_iface *
draw_tcs_llvm_iface(const struct lp_build_tcs_iface *iface)
{
   return (const struct draw_tcs_llvm_iface *)iface;
}


struct draw_tes_llvm_iface {
   struct lp_build_tes_iface base;

   struct draw_tes_llvm_variant *variant;
   LLVMValueRef input;
};


static inline const struct draw_tes_llvm_iface *
draw_tes_llvm_iface(const struct lp_build_tes_iface *iface)
{
   return (const struct draw_tes_llvm_iface *)iface;
}


/**
 * Create LLVM type for draw_vertex_buffer.
 */
static LLVMTypeRef
create_jit_dvbuffer_type(struct gallivm_state *gallivm,
                         const char *struct_name)
{
   LLVMTargetDataRef target = gallivm->target;
   LLVMTypeRef dvbuffer_type;
   LLVMTypeRef elem_types[DRAW_JIT_DVBUFFER_NUM_FIELDS];
   LLVMTypeRef int32_type = LLVMInt32TypeInContext(gallivm->context);

   elem_types[DRAW_JIT_DVBUFFER_MAP] =
      LLVMPointerType(LLVMIntTypeInContext(gallivm->context, 8), 0);
   elem_types[DRAW_JIT_DVBUFFER_SIZE] = int32_type;

   dvbuffer_type = LLVMStructTypeInContext(gallivm->context, elem_types,
                                           ARRAY_SIZE(elem_types), 0);

   (void) target; /* silence unused var warning for non-debug build */
   LP_CHECK_MEMBER_OFFSET(struct draw_vertex_buffer, map,
                          target, dvbuffer_type,
                          DRAW_JIT_DVBUFFER_MAP);
   LP_CHECK_MEMBER_OFFSET(struct draw_vertex_buffer, size,
                          target, dvbuffer_type,
                          DRAW_JIT_DVBUFFER_SIZE);

   return dvbuffer_type;
}

/**
 * Create LLVM type for struct draw_jit_context
 */
static LLVMTypeRef
create_vs_jit_context_type(struct gallivm_state *gallivm, const char *struct_name)
{
   LLVMTargetDataRef target = gallivm->target;
   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);
   LLVMTypeRef elem_types[DRAW_VS_JIT_CTX_NUM_FIELDS];

   elem_types[DRAW_VS_JIT_CTX_PLANES] = LLVMPointerType(LLVMArrayType(LLVMArrayType(float_type, 4), DRAW_TOTAL_CLIP_PLANES), 0);
   elem_types[DRAW_VS_JIT_CTX_VIEWPORT] = LLVMPointerType(float_type, 0);

   LLVMTypeRef context_type = LLVMStructTypeInContext(gallivm->context, elem_types, ARRAY_SIZE(elem_types), 0);

   (void) target; /* silence unused var warning for non-debug build */
   LP_CHECK_MEMBER_OFFSET(struct draw_vs_jit_context, planes,
                          target, context_type, DRAW_VS_JIT_CTX_PLANES);
   LP_CHECK_MEMBER_OFFSET(struct draw_vs_jit_context, viewports,
                          target, context_type, DRAW_VS_JIT_CTX_VIEWPORT);
   LP_CHECK_STRUCT_SIZE(struct draw_vs_jit_context,
                        target, context_type);

   return context_type;
}


/**
 * Create LLVM type for struct draw_gs_jit_context
 */
static LLVMTypeRef
create_gs_jit_context_type(struct gallivm_state *gallivm,
                           unsigned vector_length,
                           const char *struct_name)
{
   LLVMTargetDataRef target = gallivm->target;
   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);
   LLVMTypeRef int_type = LLVMInt32TypeInContext(gallivm->context);
   LLVMTypeRef elem_types[DRAW_GS_JIT_CTX_NUM_FIELDS];
   LLVMTypeRef context_type;

   elem_types[DRAW_GS_JIT_CTX_PLANES] = LLVMPointerType(LLVMArrayType(LLVMArrayType(float_type, 4),
                                                                      DRAW_TOTAL_CLIP_PLANES), 0);
   elem_types[DRAW_GS_JIT_CTX_VIEWPORT] = LLVMPointerType(float_type, 0); /* viewports */


   elem_types[DRAW_GS_JIT_CTX_PRIM_LENGTHS] = LLVMPointerType(LLVMPointerType(int_type, 0), 0);
   elem_types[DRAW_GS_JIT_CTX_EMITTED_VERTICES] = LLVMPointerType(LLVMVectorType(int_type,
                                                                                 vector_length), 0);
   elem_types[DRAW_GS_JIT_CTX_EMITTED_PRIMS] = LLVMPointerType(LLVMVectorType(int_type,
                                                                              vector_length), 0);

   context_type = LLVMStructTypeInContext(gallivm->context, elem_types,
                                          ARRAY_SIZE(elem_types), 0);

   (void) target; /* silence unused var warning for non-debug build */
   LP_CHECK_MEMBER_OFFSET(struct draw_gs_jit_context, planes,
                          target, context_type, DRAW_GS_JIT_CTX_PLANES);
   LP_CHECK_MEMBER_OFFSET(struct draw_gs_jit_context, viewports,
                          target, context_type, DRAW_GS_JIT_CTX_VIEWPORT);
   LP_CHECK_MEMBER_OFFSET(struct draw_gs_jit_context, prim_lengths,
                          target, context_type,
                          DRAW_GS_JIT_CTX_PRIM_LENGTHS);
   LP_CHECK_MEMBER_OFFSET(struct draw_gs_jit_context, emitted_vertices,
                          target, context_type,
                          DRAW_GS_JIT_CTX_EMITTED_VERTICES);
   LP_CHECK_MEMBER_OFFSET(struct draw_gs_jit_context, emitted_prims,
                          target, context_type,
                          DRAW_GS_JIT_CTX_EMITTED_PRIMS);
   LP_CHECK_STRUCT_SIZE(struct draw_gs_jit_context,
                        target, context_type);
   return context_type;
}


static LLVMTypeRef
create_gs_jit_input_type_deref(struct gallivm_state *gallivm)
{
   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);
   LLVMTypeRef input_array;

   input_array = LLVMVectorType(float_type, TGSI_NUM_CHANNELS); /* num primitives */
   input_array = LLVMArrayType(input_array, TGSI_NUM_CHANNELS); /* num channels */
   input_array = LLVMArrayType(input_array, PIPE_MAX_SHADER_INPUTS); /* num attrs per vertex */
   return input_array;
}


static LLVMTypeRef
create_gs_jit_input_type(struct gallivm_state *gallivm)
{
   return LLVMPointerType(create_gs_jit_input_type_deref(gallivm), 0); /* num vertices per prim */
}


/**
 * Create LLVM type for struct pipe_vertex_buffer
 */
static LLVMTypeRef
create_jit_vertex_buffer_type(struct gallivm_state *gallivm,
                              const char *struct_name)
{
   LLVMTargetDataRef target = gallivm->target;
   LLVMTypeRef elem_types[3];
   LLVMTypeRef vb_type;

   elem_types[0] = LLVMInt8TypeInContext(gallivm->context);
   elem_types[1] = LLVMInt32TypeInContext(gallivm->context);
   elem_types[2] = LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0);

   vb_type = LLVMStructTypeInContext(gallivm->context, elem_types,
                                     ARRAY_SIZE(elem_types), 0);

   (void) target; /* silence unused var warning for non-debug build */
   LP_CHECK_MEMBER_OFFSET(struct pipe_vertex_buffer, is_user_buffer,
                          target, vb_type, 0);
   LP_CHECK_MEMBER_OFFSET(struct pipe_vertex_buffer, buffer_offset,
                          target, vb_type, 1);
   LP_CHECK_MEMBER_OFFSET(struct pipe_vertex_buffer, buffer.resource,
                          target, vb_type, 2);

   LP_CHECK_STRUCT_SIZE(struct pipe_vertex_buffer, target, vb_type);

   return vb_type;
}


static LLVMTypeRef
create_tcs_jit_input_type_deref(struct gallivm_state *gallivm)
{
   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);
   LLVMTypeRef input_array;

   input_array = LLVMArrayType(float_type, TGSI_NUM_CHANNELS); /* num channels */
   input_array = LLVMArrayType(input_array, NUM_TCS_INPUTS); /* num attrs per vertex */
   return input_array;
}


static LLVMTypeRef
create_tcs_jit_input_type(struct gallivm_state *gallivm)
{
   return LLVMPointerType(create_tcs_jit_input_type_deref(gallivm), 0); /* num vertices per prim */
}


static LLVMTypeRef
create_tcs_jit_output_type_deref(struct gallivm_state *gallivm)
{
   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);
   LLVMTypeRef output_array;

   output_array = LLVMArrayType(float_type, TGSI_NUM_CHANNELS); /* num channels */
   output_array = LLVMArrayType(output_array, PIPE_MAX_SHADER_INPUTS); /* num attrs per vertex */
   return output_array;
}


static LLVMTypeRef
create_tcs_jit_output_type(struct gallivm_state *gallivm)
{
   return LLVMPointerType(create_tcs_jit_output_type_deref(gallivm), 0); /* num vertices per prim */
}


static LLVMTypeRef
create_tes_jit_input_deref_type(struct gallivm_state *gallivm)
{
   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);
   LLVMTypeRef input_array;

   input_array = LLVMArrayType(float_type, TGSI_NUM_CHANNELS); /* num channels */
   input_array = LLVMArrayType(input_array, PIPE_MAX_SHADER_INPUTS); /* num attrs per vertex */

   return input_array;
}


/**
 * Create LLVM types for various structures.
 */
static void
create_vs_jit_types(struct draw_llvm_variant *variant)
{
   struct gallivm_state *gallivm = variant->gallivm;

   variant->context_type = create_vs_jit_context_type(gallivm, "draw_vs_jit_context");
   variant->context_ptr_type = LLVMPointerType(variant->context_type, 0);

   variant->resources_type = lp_build_jit_resources_type(gallivm);
   variant->resources_ptr_type = LLVMPointerType(variant->resources_type, 0);

   variant->buffer_type = create_jit_dvbuffer_type(gallivm, "draw_vertex_buffer");
   variant->buffer_ptr_type = LLVMPointerType(variant->buffer_type, 0);

   variant->vb_type = create_jit_vertex_buffer_type(gallivm, "pipe_vertex_buffer");
   variant->vb_ptr_type = LLVMPointerType(variant->vb_type, 0);
}


static LLVMTypeRef
get_context_ptr_type(struct draw_llvm_variant *variant)
{
   if (!variant->context_ptr_type)
      create_vs_jit_types(variant);
   return variant->context_ptr_type;
}


static LLVMTypeRef
get_buffer_ptr_type(struct draw_llvm_variant *variant)
{
   if (!variant->buffer_ptr_type)
      create_vs_jit_types(variant);
   return variant->buffer_ptr_type;
}


static LLVMTypeRef
get_vb_ptr_type(struct draw_llvm_variant *variant)
{
   if (!variant->vb_ptr_type)
      create_vs_jit_types(variant);
   return variant->vb_ptr_type;
}

static LLVMTypeRef
get_vertex_header_ptr_type(struct draw_llvm_variant *variant)
{
   assert(variant->vertex_header_ptr_type);
   return variant->vertex_header_ptr_type;
}


/**
 * Create per-context LLVM info.
 */
struct draw_llvm *
draw_llvm_create(struct draw_context *draw, LLVMContextRef context)
{
   struct draw_llvm *llvm;

   if (!lp_build_init())
      return NULL;

   llvm = CALLOC_STRUCT(draw_llvm);
   if (!llvm)
      return NULL;

   llvm->draw = draw;

   llvm->context = context;
   if (!llvm->context) {
      llvm->context = LLVMContextCreate();

#if LLVM_VERSION_MAJOR == 15
      LLVMContextSetOpaquePointers(llvm->context, false);
#endif

      llvm->context_owned = true;
   }
   if (!llvm->context)
      goto fail;

   llvm->nr_variants = 0;
   list_inithead(&llvm->vs_variants_list.list);

   llvm->nr_gs_variants = 0;
   list_inithead(&llvm->gs_variants_list.list);

   llvm->nr_tcs_variants = 0;
   list_inithead(&llvm->tcs_variants_list.list);

   llvm->nr_tes_variants = 0;
   list_inithead(&llvm->tes_variants_list.list);

   return llvm;

fail:
   draw_llvm_destroy(llvm);
   return NULL;
}


/**
 * Free per-context LLVM info.
 */
void
draw_llvm_destroy(struct draw_llvm *llvm)
{
   if (llvm->context_owned)
      LLVMContextDispose(llvm->context);
   llvm->context = NULL;

   /* XXX free other draw_llvm data? */
   FREE(llvm);
}


static void
draw_get_ir_cache_key(struct nir_shader *nir,
                      const void *key, size_t key_size,
                      uint32_t val_32bit,
                      unsigned char ir_sha1_cache_key[20])
{
   struct blob blob = { 0 };
   unsigned ir_size;
   void *ir_binary;

   blob_init(&blob);
   nir_serialize(&blob, nir, true);
   ir_binary = blob.data;
   ir_size = blob.size;

   struct mesa_sha1 ctx;
   _mesa_sha1_init(&ctx);
   _mesa_sha1_update(&ctx, key, key_size);
   _mesa_sha1_update(&ctx, ir_binary, ir_size);
   _mesa_sha1_update(&ctx, &val_32bit, 4);
   _mesa_sha1_final(&ctx, ir_sha1_cache_key);

   blob_finish(&blob);
}


/**
 * Create LLVM-generated code for a vertex shader.
 */
struct draw_llvm_variant *
draw_llvm_create_variant(struct draw_llvm *llvm,
                         unsigned num_inputs,
                         const struct draw_llvm_variant_key *key)
{
   struct draw_llvm_variant *variant;
   struct llvm_vertex_shader *shader =
      llvm_vertex_shader(llvm->draw->vs.vertex_shader);
   char module_name[64];
   unsigned char ir_sha1_cache_key[20];
   struct lp_cached_code cached = { 0 };
   bool needs_caching = false;
   variant = MALLOC(sizeof *variant +
                    shader->variant_key_size -
                    sizeof variant->key);
   if (!variant)
      return NULL;

   variant->llvm = llvm;
   variant->shader = shader;
   memcpy(&variant->key, key, shader->variant_key_size);

   snprintf(module_name, sizeof(module_name), "draw_llvm_vs_variant%u",
            variant->shader->variants_cached);

   if (shader->base.state.ir.nir && llvm->draw->disk_cache_cookie) {
      draw_get_ir_cache_key(shader->base.state.ir.nir,
                            key,
                            shader->variant_key_size,
                            num_inputs,
                            ir_sha1_cache_key);

      llvm->draw->disk_cache_find_shader(llvm->draw->disk_cache_cookie,
                                         &cached,
                                         ir_sha1_cache_key);
      if (!cached.data_size)
         needs_caching = true;
   }
   variant->gallivm = gallivm_create(module_name, llvm->context, &cached);

   create_vs_jit_types(variant);

   if (gallivm_debug & (GALLIVM_DEBUG_TGSI | GALLIVM_DEBUG_IR)) {
      if (llvm->draw->vs.vertex_shader->state.type == PIPE_SHADER_IR_TGSI)
         tgsi_dump(llvm->draw->vs.vertex_shader->state.tokens, 0);
      else
         nir_print_shader(llvm->draw->vs.vertex_shader->state.ir.nir, stderr);
      draw_llvm_dump_variant_key(&variant->key);
   }

   variant->vertex_header_type = lp_build_create_jit_vertex_header_type(variant->gallivm, num_inputs);
   variant->vertex_header_ptr_type = LLVMPointerType(variant->vertex_header_type, 0);

   draw_llvm_generate(llvm, variant);

   gallivm_compile_module(variant->gallivm);

   variant->jit_func = (draw_jit_vert_func)
         gallivm_jit_function(variant->gallivm, variant->function);

   if (needs_caching)
      llvm->draw->disk_cache_insert_shader(llvm->draw->disk_cache_cookie,
                                           &cached,
                                           ir_sha1_cache_key);
   gallivm_free_ir(variant->gallivm);

   variant->list_item_global.base = variant;
   variant->list_item_local.base = variant;
   /*variant->no = */shader->variants_created++;
   variant->list_item_global.base = variant;

   return variant;
}


static void
do_clamp_vertex_color(struct gallivm_state *gallivm,
                      struct lp_type type,
                      const struct tgsi_shader_info *info,
                      LLVMValueRef (*outputs)[TGSI_NUM_CHANNELS])
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef out;
   unsigned chan, attrib;
   struct lp_build_context bld;
   lp_build_context_init(&bld, gallivm, type);

   for (attrib = 0; attrib < info->num_outputs; ++attrib) {
      for (chan = 0; chan < TGSI_NUM_CHANNELS; ++chan) {
         if (outputs[attrib][chan]) {
            switch (info->output_semantic_name[attrib]) {
            case TGSI_SEMANTIC_COLOR:
            case TGSI_SEMANTIC_BCOLOR:
               out = LLVMBuildLoad2(builder, LLVMTypeOf(bld.zero), outputs[attrib][chan], "");
               out = lp_build_clamp(&bld, out, bld.zero, bld.one);
               LLVMBuildStore(builder, out, outputs[attrib][chan]);
               break;
            }
         }
      }
   }
}


static void
generate_vs(struct draw_llvm_variant *variant,
            LLVMBuilderRef builder,
            struct lp_type vs_type,
            LLVMValueRef (*outputs)[TGSI_NUM_CHANNELS],
            const LLVMValueRef (*inputs)[TGSI_NUM_CHANNELS],
            const struct lp_bld_tgsi_system_values *system_values,
            LLVMValueRef context_ptr,
            LLVMValueRef resources_ptr,
            const struct lp_build_sampler_soa *draw_sampler,
            const struct lp_build_image_soa *draw_image,
            bool clamp_vertex_color,
            struct lp_build_mask_context *bld_mask)
{
   struct draw_llvm *llvm = variant->llvm;
   const struct tgsi_token *tokens = llvm->draw->vs.vertex_shader->state.tokens;
   LLVMValueRef consts_ptr =
      lp_jit_resources_constants(variant->gallivm, variant->resources_type, resources_ptr);
   LLVMValueRef ssbos_ptr =
      lp_jit_resources_ssbos(variant->gallivm, variant->resources_type, resources_ptr);
   struct draw_llvm_variant_key *key = &variant->key;

   struct lp_build_tgsi_params params;
   memset(&params, 0, sizeof(params));

   params.type = vs_type;
   params.mask = bld_mask;
   params.consts_ptr = consts_ptr;
   params.system_values = system_values;
   params.inputs = inputs;
   params.num_inputs = key->nr_vertex_elements;
   params.context_type = variant->context_type;
   params.context_ptr = context_ptr;
   params.resources_type = variant->resources_type;
   params.resources_ptr = resources_ptr;
   params.sampler = draw_sampler;
   params.info = &llvm->draw->vs.vertex_shader->info;
   params.ssbo_ptr = ssbos_ptr;
   params.image = draw_image;
   params.aniso_filter_table = lp_jit_resources_aniso_filter_table(variant->gallivm,
                                                                   variant->resources_type,
                                                                   resources_ptr);

   if (llvm->draw->vs.vertex_shader->state.ir.nir &&
       llvm->draw->vs.vertex_shader->state.type == PIPE_SHADER_IR_NIR) {
      lp_build_nir_soa(variant->gallivm,
                       llvm->draw->vs.vertex_shader->state.ir.nir,
                       &params,
                       outputs);
   } else {
      lp_build_tgsi_soa(variant->gallivm,
                        tokens,
                        &params,
                        outputs);
   }

   if (clamp_vertex_color) {
      const struct tgsi_shader_info *info = &llvm->draw->vs.vertex_shader->info;
      do_clamp_vertex_color(variant->gallivm,
                            vs_type, info,
                            outputs);
   }
}


static void
fetch_instanced(struct gallivm_state *gallivm,
                const struct util_format_description *format_desc,
                struct lp_type vs_type,
                LLVMValueRef vb_stride,
                LLVMValueRef map_ptr,
                LLVMValueRef buffer_size_adj,
                LLVMValueRef *inputs,
                LLVMValueRef index)
{
   LLVMTypeRef i32_t = LLVMInt32TypeInContext(gallivm->context);
   LLVMTypeRef aosf_t, aosi_t;
   LLVMValueRef zero = LLVMConstNull(i32_t);
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef stride, buffer_overflowed, aos, index_valid;
   unsigned i;

   aosf_t = lp_build_vec_type(gallivm, lp_float32_vec4_type());
   aosi_t = lp_build_vec_type(gallivm, lp_int32_vec4_type());

   /* This mul can overflow. Wraparound is ok. */
   stride = LLVMBuildMul(builder, vb_stride, index, "");

   buffer_overflowed = LLVMBuildICmp(builder, LLVMIntUGE,
                                     stride, buffer_size_adj,
                                     "buffer_overflowed");

   if (0) {
      lp_build_print_value(gallivm, "   instance index = ", index);
      lp_build_print_value(gallivm, "   buffer overflowed = ", buffer_overflowed);
   }

   index_valid = LLVMBuildNot(builder, buffer_overflowed, "");
   index_valid = LLVMBuildSExt(builder, index_valid, i32_t, "");
   stride = LLVMBuildAnd(builder, stride, index_valid, "");

   aos = lp_build_fetch_rgba_aos(gallivm,
                                 format_desc,
                                 lp_float32_vec4_type(),
                                 false,
                                 map_ptr,
                                 stride, zero, zero,
                                 NULL);

   index_valid = lp_build_broadcast(gallivm, aosi_t, index_valid);
   aos = LLVMBuildBitCast(builder, aos, aosi_t, "");
   aos = LLVMBuildAnd(builder, aos, index_valid, "");
   aos = LLVMBuildBitCast(builder, aos, aosf_t, "");

   for (i = 0; i < TGSI_NUM_CHANNELS; i++) {
      LLVMValueRef index = lp_build_const_int32(gallivm, i);
      inputs[i] = lp_build_extract_broadcast(gallivm,
                                             lp_float32_vec4_type(),
                                             vs_type, aos, index);
   }
}


static void
fetch_vector(struct gallivm_state *gallivm,
             const struct util_format_description *format_desc,
             struct lp_type vs_type,
             LLVMValueRef vb_stride,
             LLVMValueRef map_ptr,
             LLVMValueRef buffer_size_adj,
             LLVMValueRef *inputs,
             LLVMValueRef indices)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context blduivec;
   struct lp_type fetch_type = vs_type;
   LLVMValueRef offset, valid_mask;
   unsigned i;

   lp_build_context_init(&blduivec, gallivm, lp_uint_type(vs_type));

   vb_stride = lp_build_broadcast_scalar(&blduivec, vb_stride);
   buffer_size_adj = lp_build_broadcast_scalar(&blduivec, buffer_size_adj);

   /* This mul can overflow. Wraparound is ok. */
   offset = lp_build_mul(&blduivec, vb_stride, indices);

   valid_mask = lp_build_compare(gallivm, blduivec.type,
                                 PIPE_FUNC_LESS, offset, buffer_size_adj);

   /* not valid elements use offset 0 */
   offset = LLVMBuildAnd(builder, offset, valid_mask, "");

   if (0) {
      lp_build_print_value(gallivm, "   indices = ", indices);
      lp_build_print_value(gallivm, "   offsets = ", offset);
      lp_build_print_value(gallivm, "   valid_mask = ", valid_mask);
   }

   /*
    * Unlike fetch_instanced, use SoA fetch instead of multiple AoS fetches.
    * This should always produce better code.
    */

   /* The type handling is annoying here... */
   if (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB &&
       format_desc->channel[0].pure_integer) {
      if (format_desc->channel[0].type == UTIL_FORMAT_TYPE_SIGNED) {
         fetch_type = lp_type_int_vec(vs_type.width, vs_type.width * vs_type.length);
      } else if (format_desc->channel[0].type == UTIL_FORMAT_TYPE_UNSIGNED) {
         fetch_type = lp_type_uint_vec(vs_type.width, vs_type.width * vs_type.length);
      }
   }

   lp_build_fetch_rgba_soa(gallivm, format_desc,
                           fetch_type, false, map_ptr, offset,
                           blduivec.zero, blduivec.zero,
                           NULL, inputs);

   for (i = 0; i < TGSI_NUM_CHANNELS; i++) {
      inputs[i] = LLVMBuildBitCast(builder, inputs[i],
                                   lp_build_vec_type(gallivm, vs_type), "");
   }

   /* out-of-bound fetches return all zeros */
   for (i = 0; i < format_desc->nr_channels; i++) {
      inputs[i] = LLVMBuildBitCast(builder, inputs[i], blduivec.vec_type, "");
      inputs[i] = LLVMBuildAnd(builder, inputs[i], valid_mask, "");
      inputs[i] = LLVMBuildBitCast(builder, inputs[i],
                                   lp_build_vec_type(gallivm, vs_type), "");
   }
}


static void
store_aos(struct gallivm_state *gallivm,
          bool is_per_prim,
          LLVMTypeRef io_type,
          LLVMValueRef io_ptr,
          LLVMValueRef index,
          LLVMValueRef value)
{
   LLVMTypeRef data_ptr_type = LLVMPointerType(lp_build_vec_type(gallivm, lp_float32_vec4_type()), 0);
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef data_ptr;
   LLVMTypeRef data_type;
   LLVMValueRef indices[3];

   indices[0] = lp_build_const_int32(gallivm, 0);
   indices[1] = index;
   indices[2] = lp_build_const_int32(gallivm, 0);

   if (!is_per_prim) {
      data_ptr = lp_jit_vertex_header_data(gallivm, io_type, io_ptr);
      data_type = LLVMStructGetTypeAtIndex(io_type, LP_JIT_VERTEX_HEADER_DATA);
   } else {
      data_ptr = io_ptr;
      data_type = io_type;
   }

   data_ptr = LLVMBuildGEP2(builder, data_type, data_ptr, indices, 3, "");
   data_ptr = LLVMBuildPointerCast(builder, data_ptr, data_ptr_type, "");

#if DEBUG_STORE
   if (is_per_prim)
      lp_build_printf(gallivm, "    ---- %p storing prim attribute %d (io = %p)\n", data_ptr, index, io_ptr);
   else
      lp_build_printf(gallivm, "    ---- %p storing attribute %d (io = %p)\n", data_ptr, index, io_ptr);
#endif

   /* Unaligned store due to the vertex header */
   LLVMSetAlignment(LLVMBuildStore(builder, value, data_ptr), sizeof(float));
}


/**
 * Adjust the mask to architecture endianess. The mask will the store in struct:
 *
 * struct vertex_header {
 *    unsigned clipmask:DRAW_TOTAL_CLIP_PLANES;
 *    unsigned edgeflag:1;
 *    unsigned pad:1;
 *    unsigned vertex_id:16;
 *    [...]
 * }
 *
 * On little-endian machine nothing needs to done, however on bit-endian machine
 * the mask's fields need to be adjusted with the algorithm:
 *
 * uint32_t reverse (uint32_t x)
 * {
 *   return (x >> 16) |              // vertex_id
 *          ((x & 0x3fff) << 18) |   // clipmask
 *          ((x & 0x4000) << 3) |    // edgeflag
 *          ((x & 0x8000) << 1);     // pad
 * }
 */
static LLVMValueRef
adjust_mask(struct gallivm_state *gallivm,
            LLVMValueRef mask)
{
#if UTIL_ARCH_BIG_ENDIAN
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef vertex_id;
   LLVMValueRef clipmask;
   LLVMValueRef pad;
   LLVMValueRef edgeflag;

   vertex_id = LLVMBuildLShr(builder, mask, lp_build_const_int32(gallivm, 16), "");
   clipmask  = LLVMBuildAnd(builder, mask, lp_build_const_int32(gallivm, 0x3fff), "");
   clipmask  = LLVMBuildShl(builder, clipmask, lp_build_const_int32(gallivm, 18), "");
   if (0) {
      pad = LLVMBuildAnd(builder, mask, lp_build_const_int32(gallivm, 0x8000), "");
      pad = LLVMBuildShl(builder, pad, lp_build_const_int32(gallivm, 1), "");
   }
   edgeflag = LLVMBuildAnd(builder, mask, lp_build_const_int32(gallivm, 0x4000), "");
   edgeflag = LLVMBuildShl(builder, edgeflag, lp_build_const_int32(gallivm, 3), "");

   mask = LLVMBuildOr(builder, vertex_id, clipmask, "");
   if (0) {
      mask = LLVMBuildOr(builder, mask, pad, "");
   }
   mask = LLVMBuildOr(builder, mask, edgeflag, "");
#endif
   return mask;
}


void
draw_store_aos_array(struct gallivm_state *gallivm,
                     struct lp_type soa_type,
                     LLVMTypeRef io_type,
                     LLVMValueRef io_ptr,
                     LLVMValueRef *indices,
                     LLVMValueRef* aos,
                     int attrib,
                     LLVMValueRef clipmask,
                     bool need_edgeflag, bool is_per_prim)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef attr_index = lp_build_const_int32(gallivm, attrib);
   LLVMValueRef inds[LP_MAX_VECTOR_WIDTH / 32];
   LLVMValueRef linear_inds[LP_MAX_VECTOR_WIDTH / 32];
   LLVMValueRef io_ptrs[LP_MAX_VECTOR_WIDTH / 32];
   int vector_length = soa_type.length;

   assert(TGSI_NUM_CHANNELS == 4);

   for (int i = 0; i < vector_length; i++) {
      linear_inds[i] = lp_build_const_int32(gallivm, i);
      if (indices) {
         inds[i] = indices[i];
      } else {
         inds[i] = linear_inds[i];
      }
      io_ptrs[i] = LLVMBuildGEP2(builder, io_type, io_ptr, &inds[i], 1, "");
   }

   if (attrib == 0 && !is_per_prim) {
      /* store vertex header for each of the n vertices */
      LLVMValueRef val, cliptmp;
      int vertex_id_pad_edgeflag;

      /* If this assertion fails, it means we need to update the bit twidding
       * code here.  See struct vertex_header in draw_private.h.
       */
      assert(DRAW_TOTAL_CLIP_PLANES==14);
      /* initialize vertex id:16 = 0xffff, pad:1 = 0, edgeflag:1 = 1 */
      if (!need_edgeflag) {
         vertex_id_pad_edgeflag = (0xffff << 16) | (1 << DRAW_TOTAL_CLIP_PLANES);
      } else {
         vertex_id_pad_edgeflag = (0xffff << 16);
      }
      if (vector_length == 1)
         val = lp_build_const_int32(gallivm, vertex_id_pad_edgeflag);
      else
         val = lp_build_const_int_vec(gallivm, lp_int_type(soa_type),
                                      vertex_id_pad_edgeflag);

      /* OR with the clipmask */
      cliptmp = LLVMBuildOr(builder, val, clipmask, "");
      for (unsigned i = 0; i < vector_length; i++) {
         LLVMValueRef id_ptr = lp_jit_vertex_header_id(gallivm, io_type, io_ptrs[i]);
         if (vector_length > 1)
            val = LLVMBuildExtractElement(builder, cliptmp, linear_inds[i], "");
         else
            val = cliptmp;
         val = adjust_mask(gallivm, val);
#if DEBUG_STORE
         lp_build_printf(gallivm, "io = %p, index %d, clipmask = %x\n",
                         io_ptrs[i], inds[i], val);
#endif
         LLVMBuildStore(builder, val, id_ptr);
      }
   }

   /* store for each of the n vertices */
   for (int i = 0; i < vector_length; i++) {
      store_aos(gallivm, is_per_prim, io_type, io_ptrs[i], attr_index, aos[i]);
   }
}


static void
convert_to_aos(struct gallivm_state *gallivm,
               LLVMTypeRef io_type,
               LLVMValueRef io,
               LLVMValueRef *indices,
               LLVMValueRef (*outputs)[TGSI_NUM_CHANNELS],
               LLVMValueRef clipmask,
               int num_outputs,
               struct lp_type soa_type,
               int primid_slot,
               bool need_edgeflag)
{
   LLVMBuilderRef builder = gallivm->builder;

#if DEBUG_STORE
   lp_build_printf(gallivm, "   # storing begin\n");
#endif
   for (unsigned attrib = 0; attrib < num_outputs; ++attrib) {
      LLVMValueRef soa[TGSI_NUM_CHANNELS];
      LLVMValueRef aos[LP_MAX_VECTOR_WIDTH / 32];
      for (unsigned chan = 0; chan < TGSI_NUM_CHANNELS; ++chan) {
         if (outputs[attrib][chan]) {
            LLVMTypeRef single_type = (attrib == primid_slot) ? lp_build_int_vec_type(gallivm, soa_type) : lp_build_vec_type(gallivm, soa_type);
            LLVMValueRef out = LLVMBuildLoad2(builder, single_type, outputs[attrib][chan], "");
            lp_build_name(out, "output%u.%c", attrib, "xyzw"[chan]);
#if DEBUG_STORE
            lp_build_printf(gallivm, "output %d : %d ",
                            LLVMConstInt(LLVMInt32TypeInContext(gallivm->context),
                                         attrib, 0),
                            LLVMConstInt(LLVMInt32TypeInContext(gallivm->context),
                                         chan, 0));
            lp_build_print_value(gallivm, "val = ", out);
            {
               LLVMValueRef iv =
                  LLVMBuildBitCast(builder, out, lp_build_int_vec_type(gallivm, soa_type), "");

               lp_build_print_value(gallivm, "  ival = ", iv);
            }
#endif
            soa[chan] = out;
         } else {
            soa[chan] = 0;
         }
      }


      if (soa_type.length == TGSI_NUM_CHANNELS) {
         lp_build_transpose_aos(gallivm, soa_type, soa, aos);
      } else {
         lp_build_transpose_aos(gallivm, soa_type, soa, soa);

         for (unsigned i = 0; i < soa_type.length; ++i) {
            aos[i] = lp_build_extract_range(gallivm,
                                            soa[i % TGSI_NUM_CHANNELS],
                                            (i / TGSI_NUM_CHANNELS) * TGSI_NUM_CHANNELS,
                                            TGSI_NUM_CHANNELS);
         }
      }

      draw_store_aos_array(gallivm,
                           soa_type,
                           io_type,
                           io,
                           indices,
                           aos,
                           attrib,
                           clipmask,
                           need_edgeflag, false);
   }
#if DEBUG_STORE
   lp_build_printf(gallivm, "   # storing end\n");
#endif
}


/**
 * Stores original vertex positions in clip coordinates
 */
static void
store_clip(struct gallivm_state *gallivm,
           const struct lp_type vs_type,
           LLVMTypeRef io_type,
           LLVMValueRef io_ptr,
           LLVMValueRef (*outputs)[TGSI_NUM_CHANNELS],
           int idx)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef soa[4];
   LLVMValueRef aos[LP_MAX_VECTOR_LENGTH];
   LLVMValueRef io_ptrs[LP_MAX_VECTOR_WIDTH / 32];
   LLVMValueRef inds[LP_MAX_VECTOR_WIDTH / 32];
   LLVMValueRef clip_ptrs[LP_MAX_VECTOR_WIDTH / 32];
   LLVMTypeRef clip_ptr_type =
      LLVMPointerType(LLVMVectorType(LLVMFloatTypeInContext(gallivm->context),
                                     4), 0);

   for (int i = 0; i < vs_type.length; i++) {
      inds[i] = lp_build_const_int32(gallivm, i);
      io_ptrs[i] = LLVMBuildGEP2(builder, io_type, io_ptr, &inds[i], 1, "");
   }

   LLVMTypeRef single_type = lp_build_vec_type(gallivm, vs_type);
   soa[0] = LLVMBuildLoad2(builder, single_type, outputs[idx][0], ""); /*x0 x1 .. xn*/
   soa[1] = LLVMBuildLoad2(builder, single_type, outputs[idx][1], ""); /*y0 y1 .. yn*/
   soa[2] = LLVMBuildLoad2(builder, single_type, outputs[idx][2], ""); /*z0 z1 .. zn*/
   soa[3] = LLVMBuildLoad2(builder, single_type, outputs[idx][3], ""); /*w0 w1 .. wn*/

   for (int i = 0; i < vs_type.length; i++) {
      clip_ptrs[i] = lp_jit_vertex_header_clip_pos(gallivm, io_type, io_ptrs[i]);
   }

   lp_build_transpose_aos(gallivm, vs_type, soa, soa);
   for (int i = 0; i < vs_type.length; ++i) {
      aos[i] = lp_build_extract_range(gallivm,
                                      soa[i % TGSI_NUM_CHANNELS],
                                      (i / TGSI_NUM_CHANNELS) * TGSI_NUM_CHANNELS,
                                      TGSI_NUM_CHANNELS);
   }

   for (int j = 0; j < vs_type.length; j++) {
      LLVMValueRef clip_ptr;

      clip_ptr = LLVMBuildPointerCast(builder, clip_ptrs[j], clip_ptr_type, "");

      /* Unaligned store */
      LLVMSetAlignment(LLVMBuildStore(builder, aos[j], clip_ptr), sizeof(float));
   }
}


/**
 * Transforms the outputs for viewport mapping
 */
static void
generate_viewport(struct draw_llvm_variant *variant,
                  LLVMBuilderRef builder,
                  struct lp_type vs_type,
                  LLVMValueRef (*outputs)[TGSI_NUM_CHANNELS],
                  LLVMValueRef context_ptr)
{
   struct gallivm_state *gallivm = variant->gallivm;
   struct lp_type f32_type = vs_type;
   const unsigned pos = variant->llvm->draw->vs.position_output;
   LLVMTypeRef vs_type_llvm = lp_build_vec_type(gallivm, vs_type);
   LLVMValueRef out3 = LLVMBuildLoad2(builder, vs_type_llvm, outputs[pos][3], ""); /*w0 w1 .. wn*/
   LLVMValueRef const1 = lp_build_const_vec(gallivm, f32_type, 1.0);       /*1.0 1.0 1.0 1.0*/
   LLVMValueRef vp_ptr = draw_vs_jit_context_viewports(variant, context_ptr);

   /* We treat pipe_viewport_state as a float array */
   const int scale_index_offset = offsetof(struct pipe_viewport_state, scale) / sizeof(float);
   const int trans_index_offset = offsetof(struct pipe_viewport_state, translate) / sizeof(float);

   /* for 1/w convention*/
   out3 = LLVMBuildFDiv(builder, const1, out3, "");
   LLVMBuildStore(builder, out3, outputs[pos][3]);

   LLVMTypeRef elem_type = lp_build_elem_type(gallivm, vs_type);

   /* Viewport Mapping */
   for (unsigned i = 0; i < 3; i++) {
      LLVMValueRef out = LLVMBuildLoad2(builder, vs_type_llvm, outputs[pos][i], ""); /*x0 x1 .. xn*/
      LLVMValueRef scale;
      LLVMValueRef trans;
      LLVMValueRef scale_i;
      LLVMValueRef trans_i;
      LLVMValueRef index;

      index = lp_build_const_int32(gallivm, i + scale_index_offset);
      scale_i = LLVMBuildGEP2(builder, elem_type, vp_ptr, &index, 1, "");

      index = lp_build_const_int32(gallivm, i + trans_index_offset);
      trans_i = LLVMBuildGEP2(builder, elem_type, vp_ptr, &index, 1, "");

      scale = lp_build_broadcast(gallivm, vs_type_llvm,
                                 LLVMBuildLoad2(builder, elem_type, scale_i, "scale"));
      trans = lp_build_broadcast(gallivm, vs_type_llvm,
                                 LLVMBuildLoad2(builder, elem_type, trans_i, "trans"));

      /* divide by w */
      out = LLVMBuildFMul(builder, out, out3, "");
      /* mult by scale, add translation */
      out = lp_build_fmuladd(builder, out, scale, trans);

      /* store transformed outputs */
      LLVMBuildStore(builder, out, outputs[pos][i]);
   }

}


/**
 * Returns clipmask as nxi32 bitmask for the n vertices
 */
static LLVMValueRef
generate_clipmask(struct draw_llvm *llvm,
                  struct gallivm_state *gallivm,
                  struct lp_type vs_type,
                  LLVMValueRef (*outputs)[TGSI_NUM_CHANNELS],
                  struct draw_llvm_variant_key *key,
                  LLVMTypeRef context_type,
                  LLVMValueRef context_ptr,
                  bool *have_clipdist)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef mask; /* stores the <nxi32> clipmasks */
   LLVMValueRef test, temp;
   LLVMValueRef zero, shift;
   LLVMValueRef pos_x, pos_y, pos_z, pos_w;
   LLVMValueRef cv_x, cv_y, cv_z, cv_w;
   LLVMValueRef plane1, planes, plane_ptr;
   struct lp_type f32_type = vs_type;
   struct lp_type i32_type = lp_int_type(vs_type);
   const unsigned pos = llvm->draw->vs.position_output;
   const unsigned cv = llvm->draw->vs.clipvertex_output;
   int num_written_clipdistance = llvm->draw->vs.vertex_shader->info.num_written_clipdistance;
   bool have_cd = false;
   bool clip_user = key->clip_user;
   unsigned ucp_enable = key->ucp_enable;
   unsigned cd[2];

   cd[0] = llvm->draw->vs.ccdistance_output[0];
   cd[1] = llvm->draw->vs.ccdistance_output[1];

   if (cd[0] != pos || cd[1] != pos)
      have_cd = true;

   if (num_written_clipdistance && !clip_user) {
      clip_user = true;
      ucp_enable = (1 << num_written_clipdistance) - 1;
   }

   mask = lp_build_const_int_vec(gallivm, i32_type, 0);
   temp = lp_build_const_int_vec(gallivm, i32_type, 0);
   zero = lp_build_const_vec(gallivm, f32_type, 0);         /* 0.0f 0.0f 0.0f 0.0f */
   shift = lp_build_const_int_vec(gallivm, i32_type, 1);    /* 1 1 1 1 */

   LLVMTypeRef vec_type = LLVMTypeOf(zero);

   /*
    * load clipvertex and position from correct locations.
    * if they are the same just load them once.
    */
   pos_x = LLVMBuildLoad2(builder, vec_type, outputs[pos][0], ""); /*x0 x1 .. xn */
   pos_y = LLVMBuildLoad2(builder, vec_type, outputs[pos][1], ""); /*y0 y1 .. yn */
   pos_z = LLVMBuildLoad2(builder, vec_type, outputs[pos][2], ""); /*z0 z1 .. zn */
   pos_w = LLVMBuildLoad2(builder, vec_type, outputs[pos][3], ""); /*w0 w1 .. wn */

   if (clip_user && cv != pos) {
      cv_x = LLVMBuildLoad2(builder, vec_type, outputs[cv][0], ""); /*x0 x1 .. xn */
      cv_y = LLVMBuildLoad2(builder, vec_type, outputs[cv][1], ""); /*y0 y1 .. yn */
      cv_z = LLVMBuildLoad2(builder, vec_type, outputs[cv][2], ""); /*z0 z1 .. zn */
      cv_w = LLVMBuildLoad2(builder, vec_type, outputs[cv][3], ""); /*w0 w1 .. wn */
   } else {
      cv_x = pos_x;
      cv_y = pos_y;
      cv_z = pos_z;
      cv_w = pos_w;
   }

   /*
    * Be careful with the comparisons and NaNs (using llvm's unordered
    * comparisons here).
    */
   /* Cliptest, for hardwired planes */
   /*
    * XXX should take guardband into account (currently not in key).
    * Otherwise might run the draw pipeline stages for nothing.
    */
   if (key->clip_xy) {
      /* plane 1 */
      test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_GREATER, pos_x , pos_w);
      temp = shift;
      test = LLVMBuildAnd(builder, test, temp, "");
      mask = test;

      /* plane 2 */
      test = LLVMBuildFAdd(builder, pos_x, pos_w, "");
      test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_GREATER, zero, test);
      temp = LLVMBuildShl(builder, temp, shift, "");
      test = LLVMBuildAnd(builder, test, temp, "");
      mask = LLVMBuildOr(builder, mask, test, "");

      /* plane 3 */
      test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_GREATER, pos_y, pos_w);
      temp = LLVMBuildShl(builder, temp, shift, "");
      test = LLVMBuildAnd(builder, test, temp, "");
      mask = LLVMBuildOr(builder, mask, test, "");

      /* plane 4 */
      test = LLVMBuildFAdd(builder, pos_y, pos_w, "");
      test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_GREATER, zero, test);
      temp = LLVMBuildShl(builder, temp, shift, "");
      test = LLVMBuildAnd(builder, test, temp, "");
      mask = LLVMBuildOr(builder, mask, test, "");
   }

   if (key->clip_z) {
      temp = lp_build_const_int_vec(gallivm, i32_type, 16);
      if (key->clip_halfz) {
         /* plane 5 */
         test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_GREATER, zero, pos_z);
         test = LLVMBuildAnd(builder, test, temp, "");
         mask = LLVMBuildOr(builder, mask, test, "");
      } else {
         /* plane 5 */
         test = LLVMBuildFAdd(builder, pos_z, pos_w, "");
         test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_GREATER, zero, test);
         test = LLVMBuildAnd(builder, test, temp, "");
         mask = LLVMBuildOr(builder, mask, test, "");
      }
      /* plane 6 */
      test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_GREATER, pos_z, pos_w);
      temp = LLVMBuildShl(builder, temp, shift, "");
      test = LLVMBuildAnd(builder, test, temp, "");
      mask = LLVMBuildOr(builder, mask, test, "");
   }

   if (clip_user) {
      LLVMValueRef planes_ptr = draw_vs_jit_context_planes(gallivm, context_type, context_ptr);
      LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);
      LLVMTypeRef planes_type = LLVMArrayType(LLVMArrayType(float_type, 4), DRAW_TOTAL_CLIP_PLANES);
      LLVMValueRef indices[3];
      LLVMValueRef is_nan_or_inf;

      /* userclip planes */
      while (ucp_enable) {
         unsigned plane_idx = ffs(ucp_enable)-1;
         ucp_enable &= ~(1 << plane_idx);
         plane_idx += 6;

         if (have_cd && num_written_clipdistance) {
            LLVMValueRef clipdist;
            int i;
            i = plane_idx - 6;

            *have_clipdist = true;
            if (i < 4) {
               clipdist = LLVMBuildLoad2(builder, vec_type, outputs[cd[0]][i], "");
            } else {
               clipdist = LLVMBuildLoad2(builder, vec_type, outputs[cd[1]][i-4], "");
            }
            test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_GREATER, zero, clipdist);
            is_nan_or_inf = lp_build_is_inf_or_nan(gallivm, vs_type, clipdist);
            test = LLVMBuildOr(builder, test, is_nan_or_inf, "");
            temp = lp_build_const_int_vec(gallivm, i32_type, 1LL << plane_idx);
            test = LLVMBuildAnd(builder, test, temp, "");
            mask = LLVMBuildOr(builder, mask, test, "");
         } else {
            LLVMTypeRef vs_elem_type = lp_build_elem_type(gallivm, vs_type);
            LLVMTypeRef vs_type_llvm = lp_build_vec_type(gallivm, vs_type);
            LLVMValueRef sum = NULL;
            indices[0] = lp_build_const_int32(gallivm, 0);
            indices[1] = lp_build_const_int32(gallivm, plane_idx);

            for (int i = 0; i < 4; ++i) {
               indices[2] = lp_build_const_int32(gallivm, i);
               plane_ptr = LLVMBuildGEP2(builder, planes_type, planes_ptr, indices, 3, "");
               plane1 = LLVMBuildLoad2(builder, vs_elem_type, plane_ptr,
                                       (const char *[]){"plane_x", "plane_y", "plane_z", "plane_w"}[i]);
               planes = lp_build_broadcast(gallivm, vs_type_llvm, plane1);
               if (i == 0) {
                  sum = LLVMBuildFMul(builder, planes, cv_x, "");
               } else {
                  sum = lp_build_fmuladd(builder, planes,
                                         (LLVMValueRef[]){cv_x, cv_y, cv_z, cv_w}[i], sum);
               }
            }

            test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_GREATER, zero, sum);
            temp = lp_build_const_int_vec(gallivm, i32_type, 1LL << plane_idx);
            test = LLVMBuildAnd(builder, test, temp, "");
            mask = LLVMBuildOr(builder, mask, test, "");
         }
      }
   }
   if (key->need_edgeflags) {
      /*
       * This isn't really part of clipmask but stored the same in vertex
       * header later, so do it here.
       */
      unsigned edge_attr = llvm->draw->vs.edgeflag_output;
      LLVMValueRef one = lp_build_const_vec(gallivm, f32_type, 1.0);
      LLVMValueRef edgeflag = LLVMBuildLoad2(builder, vec_type, outputs[edge_attr][0], "");
      test = lp_build_compare(gallivm, f32_type, PIPE_FUNC_EQUAL, one, edgeflag);
      temp = lp_build_const_int_vec(gallivm, i32_type,
                                    1LL << DRAW_TOTAL_CLIP_PLANES);
      test = LLVMBuildAnd(builder, test, temp, "");
      mask = LLVMBuildOr(builder, mask, test, "");
   }
   return mask;
}


/**
 * Returns boolean if any clipping has occurred
 * Used zero/one i8 value to represent boolean
 */
static LLVMValueRef
clipmask_booli8(struct gallivm_state *gallivm,
                const struct lp_type vs_type,
                LLVMTypeRef clipmask_bool_type,
                LLVMValueRef clipmask_bool_ptr,
                bool edgeflag_in_clipmask)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMTypeRef int8_type = LLVMInt8TypeInContext(gallivm->context);
   LLVMValueRef clipmask_bool = LLVMBuildLoad2(builder, clipmask_bool_type, clipmask_bool_ptr, "");
   LLVMValueRef ret;
   struct lp_build_context bldivec;

   lp_build_context_init(&bldivec, gallivm, lp_int_type(vs_type));

   /*
    * We need to invert the edgeflag bit from the clipmask here
    * (because the result is really if we want to run the pipeline or not
    * and we (may) need it if edgeflag was 0).
    */
   if (edgeflag_in_clipmask) {
      LLVMValueRef edge = lp_build_const_int_vec(gallivm, bldivec.type,
                                                 1LL << DRAW_TOTAL_CLIP_PLANES);
      clipmask_bool = LLVMBuildXor(builder, clipmask_bool, edge, "");
   }

   /*
    * XXX: probably should mask off bits from the mask which come from
    * vertices which were beyond the count (i.e. indices_valid for
    * linear fetches, for elts ones we don't have the correct mask
    * right now). Otherwise might run the pipeline for nothing,
    * though everything should still work.
    */
   ret = lp_build_any_true_range(&bldivec, vs_type.length, clipmask_bool);
   ret = LLVMBuildZExt(builder, ret, int8_type, "");
   return ret;
}


static LLVMValueRef
draw_gs_llvm_fetch_input(const struct lp_build_gs_iface *gs_iface,
                         struct lp_build_context * bld,
                         bool is_vindex_indirect,
                         LLVMValueRef vertex_index,
                         bool is_aindex_indirect,
                         LLVMValueRef attrib_index,
                         LLVMValueRef swizzle_index)
{
   const struct draw_gs_llvm_iface *gs = draw_gs_llvm_iface(gs_iface);
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef indices[3];
   LLVMValueRef res;
   struct lp_type type = bld->type;

   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);
   LLVMTypeRef channel_vec_type = LLVMVectorType(float_type, TGSI_NUM_CHANNELS);
   LLVMTypeRef input_array_type = create_gs_jit_input_type_deref(gallivm);

   if (is_vindex_indirect || is_aindex_indirect) {
      res = bld->zero;
      for (int i = 0; i < type.length; ++i) {
         LLVMValueRef idx = lp_build_const_int32(gallivm, i);
         LLVMValueRef vert_chan_index = vertex_index;
         LLVMValueRef attr_chan_index = attrib_index;
         LLVMValueRef channel_vec, value;

         if (is_vindex_indirect) {
            vert_chan_index = LLVMBuildExtractElement(builder,
                                                      vertex_index, idx, "");
         }
         if (is_aindex_indirect) {
            attr_chan_index = LLVMBuildExtractElement(builder,
                                                      attrib_index, idx, "");
         }

         indices[0] = vert_chan_index;
         indices[1] = attr_chan_index;
         indices[2] = swizzle_index;

         channel_vec = LLVMBuildGEP2(builder, input_array_type, gs->input, indices, 3, "");
         channel_vec = LLVMBuildLoad2(builder, channel_vec_type, channel_vec, "");
         value = LLVMBuildExtractElement(builder, channel_vec, idx, "");

         res = LLVMBuildInsertElement(builder, res, value, idx, "");
      }
   } else {
      indices[0] = vertex_index;
      indices[1] = attrib_index;
      indices[2] = swizzle_index;

      res = LLVMBuildGEP2(builder, input_array_type, gs->input, indices, 3, "");
      res = LLVMBuildLoad2(builder, channel_vec_type, res, "");
   }

   return res;
}


static void
draw_gs_llvm_emit_vertex(const struct lp_build_gs_iface *gs_base,
                         struct lp_build_context * bld,
                         LLVMValueRef (*outputs)[4],
                         LLVMValueRef emitted_vertices_vec,
                         LLVMValueRef mask_vec, LLVMValueRef stream_id)
{
   const struct draw_gs_llvm_iface *gs_iface = draw_gs_llvm_iface(gs_base);
   struct draw_gs_llvm_variant *variant = gs_iface->variant;
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type gs_type = bld->type;
   LLVMValueRef clipmask = lp_build_const_int_vec(gallivm,
                                                  lp_int_type(gs_type), 0);
   LLVMValueRef indices[LP_MAX_VECTOR_LENGTH];
   LLVMValueRef next_prim_offset =
      lp_build_const_int32(gallivm, variant->shader->base.primitive_boundary);
   LLVMValueRef io = variant->io_ptr;
   const struct tgsi_shader_info *gs_info = &variant->shader->base.info;

   LLVMValueRef cond = LLVMBuildICmp(gallivm->builder, LLVMIntNE, mask_vec, lp_build_const_int_vec(gallivm, bld->type, 0), "");
   for (unsigned i = 0; i < gs_type.length; ++i) {
      LLVMValueRef ind = lp_build_const_int32(gallivm, i);
      LLVMValueRef currently_emitted =
         LLVMBuildExtractElement(builder, emitted_vertices_vec, ind, "");
      indices[i] = LLVMBuildMul(builder, ind, next_prim_offset, "");
      indices[i] = LLVMBuildAdd(builder, indices[i], currently_emitted, "");
      indices[i] = LLVMBuildSelect(builder, LLVMBuildExtractElement(builder, cond, ind, ""), indices[i],
                                   lp_build_const_int32(gallivm, variant->shader->base.primitive_boundary - 1), "");
   }

   LLVMValueRef stream_idx = LLVMBuildExtractElement(builder, stream_id, lp_build_const_int32(gallivm, 0), "");
   LLVMValueRef cnd = LLVMBuildICmp(builder, LLVMIntULT, stream_idx, lp_build_const_int32(gallivm, variant->shader->base.num_vertex_streams), "");
   struct lp_build_if_state if_ctx;
   lp_build_if(&if_ctx, gallivm, cnd);
   io = lp_build_pointer_get2(builder, variant->vertex_header_ptr_type,
                              io, LLVMBuildExtractElement(builder, stream_id, lp_build_const_int32(gallivm, 0), ""));

   if (variant->key.clamp_vertex_color) {
      do_clamp_vertex_color(gallivm, gs_type,
                            gs_info, outputs);
   }
   convert_to_aos(gallivm, variant->vertex_header_type,
                  io, indices,
                  outputs, clipmask,
                  gs_info->num_outputs, gs_type,
                  -1,
                  false);
   lp_build_endif(&if_ctx);
}


static void
draw_gs_llvm_end_primitive(const struct lp_build_gs_iface *gs_base,
                           struct lp_build_context * bld,
                           LLVMValueRef total_emitted_vertices_vec_ptr,
                           LLVMValueRef verts_per_prim_vec,
                           LLVMValueRef emitted_prims_vec,
                           LLVMValueRef mask_vec, unsigned stream)
{
   const struct draw_gs_llvm_iface *gs_iface = draw_gs_llvm_iface(gs_base);
   struct draw_gs_llvm_variant *variant = gs_iface->variant;
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef prim_lengts_ptr =
      draw_gs_jit_prim_lengths(variant, variant->context_ptr);

   LLVMValueRef cond = LLVMBuildICmp(gallivm->builder, LLVMIntNE, mask_vec, lp_build_const_int_vec(gallivm, bld->type, 0), "");
   for (unsigned i = 0; i < bld->type.length; ++i) {
      LLVMValueRef ind = lp_build_const_int32(gallivm, i);
      LLVMValueRef prims_emitted =
         LLVMBuildExtractElement(builder, emitted_prims_vec, ind, "");
      LLVMValueRef store_ptr;
      LLVMValueRef num_vertices =
         LLVMBuildExtractElement(builder, verts_per_prim_vec, ind, "");

      LLVMValueRef this_cond = LLVMBuildExtractElement(gallivm->builder, cond, ind, "");
      struct lp_build_if_state ifthen;
      lp_build_if(&ifthen, gallivm, this_cond);
      prims_emitted = LLVMBuildMul(gallivm->builder, prims_emitted, lp_build_const_int32(gallivm, variant->shader->base.num_vertex_streams), "");
      prims_emitted = LLVMBuildAdd(gallivm->builder, prims_emitted, lp_build_const_int32(gallivm, stream), "");
      LLVMTypeRef int_type = LLVMInt32TypeInContext(gallivm->context);
      LLVMTypeRef prim_lengths_type = LLVMPointerType(int_type, 0);
      store_ptr = LLVMBuildGEP2(builder, prim_lengths_type, prim_lengts_ptr, &prims_emitted, 1, "");
      store_ptr = LLVMBuildLoad2(builder, prim_lengths_type, store_ptr, "");
      store_ptr = LLVMBuildGEP2(builder, int_type, store_ptr, &ind, 1, "");
      LLVMBuildStore(builder, num_vertices, store_ptr);
      lp_build_endif(&ifthen);
   }
}


static void
draw_gs_llvm_epilogue(const struct lp_build_gs_iface *gs_base,
                      LLVMValueRef total_emitted_vertices_vec,
                      LLVMValueRef emitted_prims_vec, unsigned stream)
{
   const struct draw_gs_llvm_iface *gs_iface = draw_gs_llvm_iface(gs_base);
   struct draw_gs_llvm_variant *variant = gs_iface->variant;
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef emitted_verts_ptr =
      draw_gs_jit_emitted_vertices(variant, variant->context_ptr);
   LLVMValueRef emitted_prims_ptr =
      draw_gs_jit_emitted_prims(variant, variant->context_ptr);
   LLVMValueRef stream_val = lp_build_const_int32(gallivm, stream);

   emitted_verts_ptr = LLVMBuildGEP2(builder, LLVMTypeOf(total_emitted_vertices_vec), emitted_verts_ptr, &stream_val, 1, "");
   emitted_prims_ptr = LLVMBuildGEP2(builder, LLVMTypeOf(emitted_prims_vec), emitted_prims_ptr, &stream_val, 1, "");

   LLVMBuildStore(builder, total_emitted_vertices_vec, emitted_verts_ptr);
   LLVMBuildStore(builder, emitted_prims_vec, emitted_prims_ptr);
}


static void
draw_llvm_generate(struct draw_llvm *llvm, struct draw_llvm_variant *variant)
{
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMContextRef context = gallivm->context;
   LLVMTypeRef int32_type = LLVMInt32TypeInContext(context);
   LLVMTypeRef arg_types[14];
   unsigned num_arg_types = ARRAY_SIZE(arg_types);
   LLVMTypeRef func_type;
   LLVMValueRef context_ptr;
   LLVMValueRef resources_ptr;
   LLVMBasicBlockRef block;
   LLVMBuilderRef builder;
   char func_name[64];
   struct lp_type vs_type;
   LLVMValueRef count, fetch_elts, start;
   LLVMValueRef vertex_id_offset;
   LLVMValueRef stride, step, io_itr;
   LLVMValueRef ind_vec, start_vec, have_elts, fetch_max, tmp;
   LLVMValueRef io_ptr, vbuffers_ptr, vb_ptr;
   LLVMValueRef vb_stride[PIPE_MAX_ATTRIBS];
   LLVMValueRef map_ptr[PIPE_MAX_ATTRIBS];
   LLVMValueRef buffer_size_adj[PIPE_MAX_ATTRIBS];
   LLVMValueRef instance_index[PIPE_MAX_ATTRIBS];
   LLVMValueRef fake_buf_ptr, fake_buf;

   struct draw_context *draw = llvm->draw;
   const struct tgsi_shader_info *vs_info = &draw->vs.vertex_shader->info;
   unsigned i, j;
   struct lp_build_context bld, blduivec;
   struct lp_build_loop_state lp_loop;
   struct lp_build_if_state if_ctx;
   const int vector_length = lp_native_vector_width / 32;
   LLVMValueRef outputs[PIPE_MAX_SHADER_OUTPUTS][TGSI_NUM_CHANNELS];
   struct lp_build_sampler_soa *sampler = 0;
   struct lp_build_image_soa *image = NULL;
   LLVMValueRef ret, clipmask_bool_ptr;
   struct draw_llvm_variant_key *key = &variant->key;
   /* If geometry shader is present we need to skip both the viewport
    * transformation and clipping otherwise the inputs to the geometry
    * shader will be incorrect.
    * The code can't handle vp transform when vs writes vp index neither
    * (though this would be fixable here, but couldn't just broadcast
    * the values).
    */
   const bool bypass_viewport = key->has_gs_or_tes || key->bypass_viewport ||
                                vs_info->writes_viewport_index;
   const bool enable_cliptest = !key->has_gs_or_tes && (key->clip_xy ||
                                                    key->clip_z ||
                                                    key->clip_user ||
                                                    key->need_edgeflags);
   LLVMValueRef variant_func;
   const unsigned pos = draw->vs.position_output;
   const unsigned cv = draw->vs.clipvertex_output;
   bool have_clipdist = false;
   struct lp_bld_tgsi_system_values system_values;

   memset(&system_values, 0, sizeof(system_values));
   memset(&outputs, 0, sizeof(outputs));
   snprintf(func_name, sizeof(func_name), "draw_llvm_vs_variant");

   i = 0;
   arg_types[i++] = get_context_ptr_type(variant);       /* context */
   arg_types[i++] = variant->resources_ptr_type;       /* context */
   arg_types[i++] = get_vertex_header_ptr_type(variant); /* vertex_header */
   arg_types[i++] = get_buffer_ptr_type(variant);        /* vbuffers */
   arg_types[i++] = int32_type;                          /* count */
   arg_types[i++] = int32_type;                          /* start/fetch_elt_max */
   arg_types[i++] = int32_type;                          /* stride */
   arg_types[i++] = get_vb_ptr_type(variant);            /* pipe_vertex_buffer's */
   arg_types[i++] = int32_type;                          /* instance_id */
   arg_types[i++] = int32_type;                          /* vertex_id_offset */
   arg_types[i++] = int32_type;                          /* start_instance */
   arg_types[i++] = LLVMPointerType(int32_type, 0);      /* fetch_elts  */
   arg_types[i++] = int32_type;                          /* draw_id */
   arg_types[i++] = int32_type;                          /* view_id */
   assert(i == ARRAY_SIZE(arg_types));

   func_type = LLVMFunctionType(LLVMInt8TypeInContext(context),
                                arg_types, num_arg_types, 0);

   variant_func = LLVMAddFunction(gallivm->module, func_name, func_type);
   variant->function = variant_func;

   LLVMSetFunctionCallConv(variant_func, LLVMCCallConv);
   for (i = 0; i < num_arg_types; ++i)
      if (LLVMGetTypeKind(arg_types[i]) == LLVMPointerTypeKind)
         lp_add_function_attr(variant_func, i + 1, LP_FUNC_ATTR_NOALIAS);

   if (gallivm->cache && gallivm->cache->data_size)
      return;

   context_ptr               = LLVMGetParam(variant_func, 0);
   resources_ptr             = LLVMGetParam(variant_func, 1);
   io_ptr                    = LLVMGetParam(variant_func, 2);
   vbuffers_ptr              = LLVMGetParam(variant_func, 3);
   count                     = LLVMGetParam(variant_func, 4);
   start                     = LLVMGetParam(variant_func, 5);
   /*
    * XXX: stride is actually unused. The stride we use is strictly calculated
    * from the number of outputs (including the draw_extra outputs).
    * Should probably fix some day (we need a new vs just because of extra
    * outputs which the generated vs won't touch).
    */
   stride                    = LLVMGetParam(variant_func, 6);
   vb_ptr                    = LLVMGetParam(variant_func, 7);
   system_values.instance_id = LLVMGetParam(variant_func, 8);
   vertex_id_offset          = LLVMGetParam(variant_func, 9);
   system_values.base_instance = LLVMGetParam(variant_func, 10);
   fetch_elts                = LLVMGetParam(variant_func, 11);
   system_values.draw_id     = LLVMGetParam(variant_func, 12);
   system_values.view_index  = LLVMGetParam(variant_func, 13);

   lp_build_name(context_ptr, "context");
   lp_build_name(resources_ptr, "resources");
   lp_build_name(io_ptr, "io");
   lp_build_name(vbuffers_ptr, "vbuffers");
   lp_build_name(count, "count");
   lp_build_name(start, "start");
   lp_build_name(stride, "stride");
   lp_build_name(vb_ptr, "vb");
   lp_build_name(system_values.instance_id, "instance_id");
   lp_build_name(vertex_id_offset, "vertex_id_offset");
   lp_build_name(system_values.base_instance, "start_instance");
   lp_build_name(fetch_elts, "fetch_elts");
   lp_build_name(system_values.draw_id, "draw_id");

   /*
    * Function body
    */

   block = LLVMAppendBasicBlockInContext(gallivm->context, variant_func, "entry");
   builder = gallivm->builder;
   LLVMPositionBuilderAtEnd(builder, block);

   memset(&vs_type, 0, sizeof vs_type);
   vs_type.floating = true; /* floating point values */
   vs_type.sign = true;     /* values are signed */
   vs_type.norm = false;    /* values are not limited to [0,1] or [-1,1] */
   vs_type.width = 32;      /* 32-bit float */
   vs_type.length = vector_length;

   lp_build_context_init(&bld, gallivm, lp_type_uint(32));
   lp_build_context_init(&blduivec, gallivm, lp_uint_type(vs_type));

   /* hold temporary "bool" clipmask */
   clipmask_bool_ptr = lp_build_alloca(gallivm, blduivec.vec_type, "");

   fake_buf = lp_build_alloca_undef(gallivm,
                 LLVMVectorType(LLVMInt64TypeInContext(context), 4), "");
   fake_buf = LLVMBuildBitCast(builder, fake_buf,
                 LLVMPointerType(LLVMInt8TypeInContext(context), 0), "");
   fake_buf_ptr = LLVMBuildGEP2(builder, LLVMInt8TypeInContext(context), fake_buf, &bld.zero, 1, "");

   /* code generated texture sampling */
   sampler = lp_bld_llvm_sampler_soa_create(draw_llvm_variant_key_samplers(key),
                                          MAX2(key->nr_samplers,
                                               key->nr_sampler_views));
   image = lp_bld_llvm_image_soa_create(draw_llvm_variant_key_images(key),
                                      key->nr_images);

   step = lp_build_const_int32(gallivm, vector_length);

   ind_vec = blduivec.undef;
   for (i = 0; i < vs_type.length; i++) {
      LLVMValueRef index = lp_build_const_int32(gallivm, i);
      ind_vec = LLVMBuildInsertElement(builder, ind_vec, index, index, "");
   }

   have_elts = LLVMBuildICmp(builder, LLVMIntNE,
                             LLVMConstPointerNull(arg_types[11]), fetch_elts, "");

   fetch_max = LLVMBuildSub(builder, count, bld.one, "fetch_max");
   fetch_max = lp_build_broadcast_scalar(&blduivec, fetch_max);
   /*
    * Only needed for non-indexed path.
    */
   start_vec = lp_build_broadcast_scalar(&blduivec, start);

   /*
    * Pre-calculate everything which is constant per shader invocation.
    */
   for (j = 0; j < key->nr_vertex_elements; ++j) {
      LLVMValueRef vb_buffer_offset, buffer_size, temp_ptr;
      LLVMValueRef vb_info, vbuffer_ptr, buf_offset, ofbit;
      struct pipe_vertex_element *velem = &key->vertex_element[j];
      LLVMValueRef vb_index =
         lp_build_const_int32(gallivm, velem->vertex_buffer_index);
      LLVMValueRef bsize = lp_build_const_int32(gallivm,
                                                util_format_get_blocksize(velem->src_format));
      LLVMValueRef src_offset = lp_build_const_int32(gallivm,
                                                     velem->src_offset);
      LLVMValueRef src_stride = lp_build_const_int32(gallivm,
                                                     velem->src_stride);
      struct lp_build_if_state if_ctx;

      if (velem->src_format != PIPE_FORMAT_NONE) {
         vbuffer_ptr = LLVMBuildGEP2(builder, variant->buffer_type, vbuffers_ptr, &vb_index, 1, "");
         vb_info = LLVMBuildGEP2(builder, variant->vb_type, vb_ptr, &vb_index, 1, "");
         vb_stride[j] = src_stride;
         vb_buffer_offset = draw_jit_vbuffer_offset(gallivm, variant->vb_type, vb_info);
         map_ptr[j] = draw_jit_dvbuffer_map(gallivm, variant->buffer_type, vbuffer_ptr);
         buffer_size = draw_jit_dvbuffer_size(gallivm, variant->buffer_type, vbuffer_ptr);

         ofbit = NULL;
         /*
          * We'll set buffer_size_adj to zero if we have of, so it will
          * always overflow later automatically without having to keep ofbit.
          * Overflows (with normal wraparound) doing the actual offset
          * calculation should be ok, just not for the buffer size calc.
          * It would also be possible to detect such overflows and return
          * zeros if that happens, but this would be more complex.
          */
         buf_offset = lp_build_add(&bld, vb_buffer_offset, src_offset);
         tmp = lp_build_sub(&bld, bsize, bld.one);
         buffer_size_adj[j] = lp_build_usub_overflow(gallivm, buffer_size, tmp,
                                                     &ofbit);
         buffer_size_adj[j] = lp_build_usub_overflow(gallivm, buffer_size_adj[j],
                                                     buf_offset, &ofbit);

         /*
          * We can't easily set fake vertex buffers outside the generated code.
          * Hence, set fake vertex buffers here instead basically, so fetch
          * code can always fetch using offset 0, eliminating all control flow
          * inside the main loop.
          * (Alternatively, could have control flow per vector skipping fetch
          * if ofbit is true.)
          */
         if (velem->instance_divisor) {
            /*
             * Index is equal to the start instance plus the number of current
             * instance divided by the divisor. In this case we compute it as:
             * index = start_instance + (instance_id  / divisor).
             * Note we could actually do the fetch here, outside the loop -
             * it's all constant, hopefully llvm recognizes this.
             */
            LLVMValueRef current_instance;
            current_instance = LLVMBuildUDiv(builder, system_values.instance_id,
                                             lp_build_const_int32(gallivm,
                                                                  velem->instance_divisor),
                                             "instance_divisor");
            instance_index[j] = lp_build_uadd_overflow(gallivm, system_values.base_instance,
                                                       current_instance, &ofbit);
         }

         buffer_size_adj[j] = LLVMBuildSelect(builder, ofbit, bld.zero,
                                              buffer_size_adj[j], "");

         LLVMTypeRef byte_type = LLVMInt8TypeInContext(context);
         LLVMTypeRef byte_ptr_type = LLVMPointerType(byte_type, 0);
         temp_ptr = lp_build_alloca_undef(gallivm, byte_ptr_type, "");

         lp_build_if(&if_ctx, gallivm, ofbit);
         {
            LLVMBuildStore(builder, fake_buf_ptr, temp_ptr);
         }
         lp_build_else(&if_ctx);
         {
            map_ptr[j] = LLVMBuildGEP2(builder, byte_type, map_ptr[j], &buf_offset, 1, "");
            LLVMBuildStore(builder, map_ptr[j], temp_ptr);
         }
         lp_build_endif(&if_ctx);
         map_ptr[j] = LLVMBuildLoad2(builder, byte_ptr_type, temp_ptr, "map_ptr");

         if (0) {
            lp_build_printf(gallivm, "velem %d, vbuf index = %u, vb_stride = %u\n",
                            lp_build_const_int32(gallivm, j),
                            vb_index, vb_stride[j]);
            lp_build_printf(gallivm,
                            "   vb_buffer_offset = %u, src_offset = %u, buf_offset = %u\n",
                            vb_buffer_offset, src_offset, buf_offset);
            lp_build_printf(gallivm, "   buffer size = %u, blocksize = %u\n",
                            buffer_size, bsize);
            lp_build_printf(gallivm, "   instance_id = %u\n", system_values.instance_id);
         }
      }
   }

   lp_build_loop_begin(&lp_loop, gallivm, bld.zero);
   {
      LLVMValueRef inputs[PIPE_MAX_SHADER_INPUTS][TGSI_NUM_CHANNELS];
      LLVMValueRef io;
      LLVMValueRef clipmask;   /* holds the clipmask value */
      LLVMValueRef true_index_array, index_store;
      const LLVMValueRef (*ptr_aos)[TGSI_NUM_CHANNELS];

      io_itr = lp_loop.counter;

      io = LLVMBuildGEP2(builder, variant->vertex_header_type, io_ptr, &io_itr, 1, "");
#if DEBUG_STORE
      lp_build_printf(gallivm, " --- io %d = %p, loop counter %d\n",
                      io_itr, io, lp_loop.counter);
#endif

      true_index_array = lp_build_broadcast_scalar(&blduivec, lp_loop.counter);
      true_index_array = LLVMBuildAdd(builder, true_index_array, ind_vec, "");

      LLVMValueRef exec_mask = lp_build_cmp(&blduivec, PIPE_FUNC_LEQUAL, true_index_array, fetch_max);
      /*
       * Limit indices to fetch_max, otherwise might try to access indices
       * beyond index buffer (or rather vsplit elt buffer) size.
       * Could probably safely (?) skip this for non-indexed draws and
       * simplify things minimally (by removing it could combine the ind_vec
       * and start_vec adds). I think the only effect for non-indexed draws will
       * be that for the invalid elements they will be all fetched from the
       * same location as the last valid one, but noone should really care.
       */
      true_index_array = lp_build_min(&blduivec, true_index_array, fetch_max);

      index_store = lp_build_alloca_undef(gallivm, blduivec.vec_type, "index_store");

      lp_build_if(&if_ctx, gallivm, have_elts);
      {
         /*
          * Note: you'd expect some comparison/clamp against fetch_elt_max
          * here.
          * There used to be one here but it was incorrect: overflow was
          * detected if index > fetch_elt_max - but the correct condition
          * would be index >= fetch_elt_max (since this is just size of elts
          * buffer / element size).
          * Using the correct condition however will cause failures - due to
          * vsplit/vcache code which rebases indices. So, as an example, if
          * fetch_elt_max is just 1 and fetch_count 2, vsplit cache will
          * replace all invalid indices with 0 - which in case of elt_bias
          * not being zero will get a different fetch index than the valid
          * index 0. So, just rely on vsplit code preventing out-of-bounds
          * fetches. This is also why it's safe to do elts fetch even if there
          * was no index buffer bound - the real buffer is never seen here, at
          * least not if there are index buffer overflows...
          */

         /*
          * XXX should not have to do this, as scale can be handled
          * natively by loads (hits asserts though).
          */
         tmp = lp_build_shl_imm(&blduivec, true_index_array, 2);
         fetch_elts = LLVMBuildBitCast(builder, fetch_elts,
                                       LLVMPointerType(LLVMInt8TypeInContext(context),
                                                       0), "");
         tmp = lp_build_gather(gallivm, vs_type.length,
                               32, bld.type, true,
                               fetch_elts, tmp, false);
         LLVMBuildStore(builder, tmp, index_store);
      }
      lp_build_else(&if_ctx);
      {
         tmp = LLVMBuildAdd(builder, true_index_array, start_vec, "");
         LLVMBuildStore(builder, tmp, index_store);
      }
      lp_build_endif(&if_ctx);

      true_index_array = LLVMBuildLoad2(builder, blduivec.vec_type, index_store, "");

      for (j = 0; j < key->nr_vertex_elements; ++j) {
         struct pipe_vertex_element *velem = &key->vertex_element[j];
         const struct util_format_description *format_desc =
            util_format_description(velem->src_format);

         if (format_desc->format == PIPE_FORMAT_NONE) {
            for (i = 0; i < TGSI_NUM_CHANNELS; i++) {
               inputs[j][i] = lp_build_zero(gallivm, vs_type);
            }
         } else if (velem->instance_divisor) {
            fetch_instanced(gallivm, format_desc, vs_type,
                            vb_stride[j], map_ptr[j],
                            buffer_size_adj[j],
                            inputs[j], instance_index[j]);
         } else {
            fetch_vector(gallivm, format_desc, vs_type,
                         vb_stride[j], map_ptr[j],
                         buffer_size_adj[j],
                         inputs[j], true_index_array);
         }
      }

      struct lp_build_mask_context mask;

      lp_build_mask_begin(&mask, gallivm, vs_type, exec_mask);
      /* In the paths with elts vertex id has to be unaffected by the
       * index bias and because indices inside our elements array have
       * already had index bias applied we need to subtract it here to
       * get back to the original index.
       * In the linear paths vertex id has to be unaffected by the
       * original start index and because we abuse the 'start' variable
       * to either represent the actual start index or the index at which
       * the primitive was split (we split rendering into chunks of at
       * most 4095-vertices) we need to back out the original start
       * index out of our vertex id here.
       * for ARB_shader_draw_parameters, base_vertex should be 0 for
       * non-indexed draws.
       */
      LLVMValueRef base_vertex = lp_build_select(&bld, have_elts, vertex_id_offset, lp_build_const_int32(gallivm, 0));
      system_values.basevertex = lp_build_broadcast_scalar(&blduivec, base_vertex);

      /* first vertex is for Vulkan base vertex support */
      LLVMValueRef first_vertex = vertex_id_offset;
      system_values.firstvertex = lp_build_broadcast_scalar(&blduivec, first_vertex);

      system_values.vertex_id = true_index_array;
      system_values.vertex_id_nobase = LLVMBuildSub(builder, true_index_array,
                                                    lp_build_broadcast_scalar(&blduivec, vertex_id_offset), "");

      ptr_aos = (const LLVMValueRef (*)[TGSI_NUM_CHANNELS]) inputs;
      generate_vs(variant,
                  builder,
                  vs_type,
                  outputs,
                  ptr_aos,
                  &system_values,
                  context_ptr,
                  resources_ptr,
                  sampler,
                  image,
                  key->clamp_vertex_color,
                  &mask);

      lp_build_mask_end(&mask);
      if (pos != -1 && cv != -1) {
         /* store original positions in clip before further manipulation */
         store_clip(gallivm, vs_type, variant->vertex_header_type, io, outputs, pos);

         /* do cliptest */
         if (enable_cliptest) {
            LLVMValueRef temp = LLVMBuildLoad2(builder, blduivec.vec_type, clipmask_bool_ptr, "");
            /* allocate clipmask, assign it integer type */
            clipmask = generate_clipmask(llvm,
                                         gallivm,
                                         vs_type,
                                         outputs,
                                         key,
                                         variant->context_type,
                                         context_ptr, &have_clipdist);
            temp = LLVMBuildOr(builder, clipmask, temp, "");
            /* store temporary clipping boolean value */
            LLVMBuildStore(builder, temp, clipmask_bool_ptr);
         } else {
            clipmask = blduivec.zero;
         }

         /* do viewport mapping */
         if (!bypass_viewport) {
            generate_viewport(variant, builder, vs_type, outputs, context_ptr);
         }
      } else {
         clipmask = blduivec.zero;
      }

      /* store clipmask in vertex header,
       * original positions in clip
       * and transformed positions in data
       */
      convert_to_aos(gallivm, variant->vertex_header_type, io, NULL, outputs, clipmask,
                     vs_info->num_outputs, vs_type, -1,
                     enable_cliptest && key->need_edgeflags);
   }
   lp_build_loop_end_cond(&lp_loop, count, step, LLVMIntUGE);

   lp_bld_llvm_sampler_soa_destroy(sampler);
   lp_bld_llvm_image_soa_destroy(image);

   /* return clipping boolean value for function */
   ret = clipmask_booli8(gallivm, vs_type, blduivec.vec_type, clipmask_bool_ptr,
                         enable_cliptest && key->need_edgeflags);

   LLVMBuildRet(builder, ret);

   gallivm_verify_function(gallivm, variant_func);
}


struct draw_llvm_variant_key *
draw_llvm_make_variant_key(struct draw_llvm *llvm, char *store)
{
   struct draw_llvm_variant_key *key;
   struct lp_sampler_static_state *draw_sampler;
   struct lp_image_static_state *draw_image;

   key = (struct draw_llvm_variant_key *)store;

   memset(key, 0, offsetof(struct draw_llvm_variant_key, vertex_element[0]));


   /* will have to rig this up properly later */
   key->clip_xy = llvm->draw->clip_xy;
   key->clip_z = llvm->draw->clip_z;
   key->clip_user = llvm->draw->clip_user;
   key->bypass_viewport = llvm->draw->bypass_viewport;
   key->clip_halfz = llvm->draw->rasterizer->clip_halfz;
   /* XXX assumes edgeflag output not at 0 */
   key->need_edgeflags = (llvm->draw->vs.edgeflag_output ? true : false);
   key->ucp_enable = llvm->draw->rasterizer->clip_plane_enable;
   key->has_gs_or_tes = llvm->draw->gs.geometry_shader != NULL || llvm->draw->tes.tess_eval_shader != NULL;
   key->num_outputs = draw_total_vs_outputs(llvm->draw);

   key->clamp_vertex_color = !key->has_gs_or_tes &&
      llvm->draw->rasterizer->clamp_vertex_color;

   /* All variants of this shader will have the same value for
    * nr_samplers.  Not yet trying to compact away holes in the
    * sampler array.
    */
   key->nr_samplers = llvm->draw->vs.vertex_shader->info.file_max[TGSI_FILE_SAMPLER] + 1;
   if (llvm->draw->vs.vertex_shader->info.file_max[TGSI_FILE_SAMPLER_VIEW] != -1) {
      key->nr_sampler_views =
         llvm->draw->vs.vertex_shader->info.file_max[TGSI_FILE_SAMPLER_VIEW] + 1;
   } else {
      key->nr_sampler_views = key->nr_samplers;
   }

   key->nr_images = llvm->draw->vs.vertex_shader->info.file_max[TGSI_FILE_IMAGE] + 1;

   /* Presumably all variants of the shader should have the same
    * number of vertex elements - ie the number of shader inputs.
    * NOTE: we NEED to store the needed number of needed inputs
    * here, not the number of provided elements to match keysize
    * (and the offset of sampler state in the key).
    * If we have excess number of vertex elements, this is valid,
    * but the excess ones don't matter.
    * If we don't have enough vertex elements (which looks not really
    * valid but we'll handle it gracefully) fill out missing ones with
    * zero (we'll recognize these later by PIPE_FORMAT_NONE).
    */
   key->nr_vertex_elements =
      llvm->draw->vs.vertex_shader->info.file_max[TGSI_FILE_INPUT] + 1;

   if (llvm->draw->pt.nr_vertex_elements < key->nr_vertex_elements) {
      debug_printf("draw: vs with %d inputs but only have %d vertex elements\n",
                   key->nr_vertex_elements, llvm->draw->pt.nr_vertex_elements);
      memset(key->vertex_element, 0,
             sizeof(struct pipe_vertex_element) * key->nr_vertex_elements);
   }
   memcpy(key->vertex_element,
          llvm->draw->pt.vertex_element,
          sizeof(struct pipe_vertex_element) *
             MIN2(key->nr_vertex_elements, llvm->draw->pt.nr_vertex_elements));

   draw_sampler = draw_llvm_variant_key_samplers(key);
   memset(draw_sampler, 0,
          MAX2(key->nr_samplers, key->nr_sampler_views) * sizeof *draw_sampler);

   for (unsigned i = 0 ; i < key->nr_samplers; i++) {
      lp_sampler_static_sampler_state(&draw_sampler[i].sampler_state,
                                      llvm->draw->samplers[PIPE_SHADER_VERTEX][i]);
   }
   for (unsigned i = 0 ; i < key->nr_sampler_views; i++) {
      lp_sampler_static_texture_state(&draw_sampler[i].texture_state,
                                      llvm->draw->sampler_views[PIPE_SHADER_VERTEX][i]);
   }

   draw_image = draw_llvm_variant_key_images(key);
   memset(draw_image, 0,
          key->nr_images * sizeof *draw_image);
   for (unsigned i = 0; i < key->nr_images; i++) {
      lp_sampler_static_texture_state_image(&draw_image[i].image_state,
                                            llvm->draw->images[PIPE_SHADER_VERTEX][i]);
   }
   return key;
}


void
draw_llvm_dump_variant_key(struct draw_llvm_variant_key *key)
{
   struct lp_sampler_static_state *sampler = draw_llvm_variant_key_samplers(key);
   struct lp_image_static_state *image = draw_llvm_variant_key_images(key);
   debug_printf("clamp_vertex_color = %u\n", key->clamp_vertex_color);
   debug_printf("clip_xy = %u\n", key->clip_xy);
   debug_printf("clip_z = %u\n", key->clip_z);
   debug_printf("clip_user = %u\n", key->clip_user);
   debug_printf("bypass_viewport = %u\n", key->bypass_viewport);
   debug_printf("clip_halfz = %u\n", key->clip_halfz);
   debug_printf("need_edgeflags = %u\n", key->need_edgeflags);
   debug_printf("has_gs_or_tes = %u\n", key->has_gs_or_tes);
   debug_printf("ucp_enable = %u\n", key->ucp_enable);

   for (unsigned i = 0 ; i < key->nr_vertex_elements; i++) {
      debug_printf("vertex_element[%i].src_offset = %u\n", i, key->vertex_element[i].src_offset);
      debug_printf("vertex_element[%i].instance_divisor = %u\n", i, key->vertex_element[i].instance_divisor);
      debug_printf("vertex_element[%i].vertex_buffer_index = %u\n", i, key->vertex_element[i].vertex_buffer_index);
      debug_printf("vertex_element[%i].src_format = %s\n", i, util_format_name(key->vertex_element[i].src_format));
   }

   for (unsigned i = 0 ; i < key->nr_sampler_views; i++) {
      debug_printf("sampler[%i].src_format = %s\n", i, util_format_name(sampler[i].texture_state.format));
   }

   for (unsigned i = 0 ; i < key->nr_images; i++)
      debug_printf("images[%i].format = %s\n", i, util_format_name(image[i].image_state.format));
}


void
draw_llvm_set_mapped_texture(struct draw_context *draw,
                             enum pipe_shader_type shader_stage,
                             unsigned sview_idx,
                             uint32_t width, uint32_t height, uint32_t depth,
                             uint32_t first_level, uint32_t last_level,
                             uint32_t num_samples,
                             uint32_t sample_stride,
                             const void *base_ptr,
                             uint32_t row_stride[PIPE_MAX_TEXTURE_LEVELS],
                             uint32_t img_stride[PIPE_MAX_TEXTURE_LEVELS],
                             uint32_t mip_offsets[PIPE_MAX_TEXTURE_LEVELS])
{
   struct lp_jit_texture *jit_tex;

   assert(shader_stage < DRAW_MAX_SHADER_STAGE);
   assert(sview_idx < ARRAY_SIZE(draw->llvm->jit_resources[shader_stage].textures));

   jit_tex = &draw->llvm->jit_resources[shader_stage].textures[sview_idx];
   jit_tex->width = width;
   jit_tex->height = height;
   jit_tex->depth = depth;
   jit_tex->first_level = first_level;
   jit_tex->last_level = last_level;
   jit_tex->mip_offsets[0] = 0;
   jit_tex->base = base_ptr;
   if (num_samples > 1) {
      jit_tex->mip_offsets[LP_JIT_TEXTURE_SAMPLE_STRIDE] = sample_stride;
      jit_tex->row_stride[0] = row_stride[0];
      jit_tex->img_stride[0] = img_stride[0];
      jit_tex->last_level = num_samples;
   } else {
      for (unsigned j = first_level; j <= last_level; j++) {
         jit_tex->mip_offsets[j] = mip_offsets[j];
         jit_tex->row_stride[j] = row_stride[j];
         jit_tex->img_stride[j] = img_stride[j];
      }
   }
}


void
draw_llvm_set_mapped_image(struct draw_context *draw,
                           enum pipe_shader_type shader_stage,
                           unsigned idx,
                           uint32_t width, uint32_t height, uint32_t depth,
                           const void *base_ptr,
                           uint32_t row_stride,
                           uint32_t img_stride,
                           uint32_t num_samples,
                           uint32_t sample_stride)
{
   struct lp_jit_image *jit_image;

   assert(shader_stage < DRAW_MAX_SHADER_STAGE);
   assert(idx < ARRAY_SIZE(draw->llvm->jit_resources[shader_stage].images));

   jit_image = &draw->llvm->jit_resources[shader_stage].images[idx];

   jit_image->width = width;
   jit_image->height = height;
   jit_image->depth = depth;
   jit_image->base = base_ptr;

   jit_image->row_stride = row_stride;
   jit_image->img_stride = img_stride;
   jit_image->num_samples = num_samples;
   jit_image->sample_stride = sample_stride;
}


void
draw_llvm_set_sampler_state(struct draw_context *draw,
                            enum pipe_shader_type shader_type)
{
   assert(shader_type < DRAW_MAX_SHADER_STAGE);
   for (unsigned i = 0; i < draw->num_samplers[shader_type]; i++) {
      struct lp_jit_sampler *jit_sam = &draw->llvm->jit_resources[shader_type].samplers[i];

      if (draw->samplers[shader_type][i]) {
         const struct pipe_sampler_state *s
            = draw->samplers[shader_type][i];
         jit_sam->min_lod = s->min_lod;
         jit_sam->max_lod = s->max_lod;
         jit_sam->lod_bias = s->lod_bias;
         jit_sam->max_aniso = s->max_anisotropy;
         COPY_4V(jit_sam->border_color, s->border_color.f);
      }
   }
}


void
draw_llvm_destroy_variant(struct draw_llvm_variant *variant)
{
   struct draw_llvm *llvm = variant->llvm;

   if (gallivm_debug & (GALLIVM_DEBUG_TGSI | GALLIVM_DEBUG_IR)) {
      debug_printf("Deleting VS variant: %u vs variants,\t%u total variants\n",
                    variant->shader->variants_cached, llvm->nr_variants);
   }

   gallivm_destroy(variant->gallivm);

   list_del(&variant->list_item_local.list);
   variant->shader->variants_cached--;
   list_del(&variant->list_item_global.list);
   llvm->nr_variants--;
   FREE(variant);
}


/**
 * Create LLVM types for various structures.
 */
static void
create_gs_jit_types(struct draw_gs_llvm_variant *var)
{
   struct gallivm_state *gallivm = var->gallivm;

   var->context_type = create_gs_jit_context_type(gallivm,
                                             var->shader->base.vector_length,
                                             "draw_gs_jit_context");
   var->context_ptr_type = LLVMPointerType(var->context_type, 0);

   var->resources_type = lp_build_jit_resources_type(gallivm);
   var->resources_ptr_type = LLVMPointerType(var->resources_type, 0);
   var->input_array_type = create_gs_jit_input_type(gallivm);
}


static LLVMTypeRef
get_gs_context_ptr_type(struct draw_gs_llvm_variant *variant)
{
   if (!variant->context_ptr_type)
      create_gs_jit_types(variant);
   return variant->context_ptr_type;
}


static LLVMValueRef
generate_mask_value(struct draw_gs_llvm_variant *variant,
                    struct lp_type gs_type)
{
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type mask_type = lp_int_type(gs_type);
   LLVMValueRef num_prims;
   LLVMValueRef mask_val = lp_build_const_vec(gallivm, mask_type, 0);

   num_prims = lp_build_broadcast(gallivm, lp_build_vec_type(gallivm, mask_type),
                                  variant->num_prims);
   for (unsigned i = 0; i < gs_type.length; i++) {
      LLVMValueRef idx = lp_build_const_int32(gallivm, i);
      mask_val = LLVMBuildInsertElement(builder, mask_val, idx, idx, "");
   }
   mask_val = lp_build_compare(gallivm, mask_type,
                               PIPE_FUNC_GREATER, num_prims, mask_val);

   return mask_val;
}


static void
draw_gs_llvm_generate(struct draw_llvm *llvm,
                      struct draw_gs_llvm_variant *variant)
{
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMContextRef context = gallivm->context;
   LLVMTypeRef int32_type = LLVMInt32TypeInContext(context);
   LLVMTypeRef arg_types[9];
   LLVMTypeRef func_type;
   LLVMValueRef variant_func;
   LLVMValueRef context_ptr;
   LLVMValueRef resources_ptr;
   LLVMValueRef prim_id_ptr;
   LLVMBasicBlockRef block;
   LLVMBuilderRef builder;
   LLVMValueRef io_ptr, input_array, num_prims, mask_val;
   struct lp_build_sampler_soa *sampler = 0;
   struct lp_build_image_soa *image = NULL;
   struct lp_build_context bld;
   struct lp_bld_tgsi_system_values system_values;
   char func_name[64];
   struct lp_type gs_type;
   struct draw_gs_llvm_iface gs_iface;
   const struct tgsi_token *tokens = variant->shader->base.state.tokens;
   LLVMValueRef consts_ptr;
   LLVMValueRef ssbos_ptr;
   LLVMValueRef outputs[PIPE_MAX_SHADER_OUTPUTS][TGSI_NUM_CHANNELS];
   struct lp_build_mask_context mask;
   const struct tgsi_shader_info *gs_info = &variant->shader->base.info;
   unsigned vector_length = variant->shader->base.vector_length;

   memset(&system_values, 0, sizeof(system_values));
   memset(&outputs, 0, sizeof(outputs));

   snprintf(func_name, sizeof(func_name), "draw_llvm_gs_variant");

   assert(variant->vertex_header_ptr_type);

   LLVMTypeRef prim_id_type = LLVMVectorType(int32_type, vector_length);
   arg_types[0] = get_gs_context_ptr_type(variant);    /* context */
   arg_types[1] = variant->resources_ptr_type;
   arg_types[2] = variant->input_array_type;           /* input */
   arg_types[3] = LLVMPointerType(variant->vertex_header_ptr_type, 0);     /* vertex_header */
   arg_types[4] = int32_type;                          /* num_prims */
   arg_types[5] = int32_type;                          /* instance_id */
   arg_types[6] = LLVMPointerType(prim_id_type, 0);    /* prim_id_ptr */
   arg_types[7] = int32_type;
   arg_types[8] = int32_type;

   func_type = LLVMFunctionType(int32_type, arg_types, ARRAY_SIZE(arg_types), 0);

   variant_func = LLVMAddFunction(gallivm->module, func_name, func_type);

   variant->function = variant_func;

   LLVMSetFunctionCallConv(variant_func, LLVMCCallConv);

   for (unsigned i = 0; i < ARRAY_SIZE(arg_types); ++i)
      if (LLVMGetTypeKind(arg_types[i]) == LLVMPointerTypeKind)
         lp_add_function_attr(variant_func, i + 1, LP_FUNC_ATTR_NOALIAS);

   if (gallivm->cache && gallivm->cache->data_size)
      return;
   context_ptr               = LLVMGetParam(variant_func, 0);
   resources_ptr             = LLVMGetParam(variant_func, 1);
   input_array               = LLVMGetParam(variant_func, 2);
   io_ptr                    = LLVMGetParam(variant_func, 3);
   num_prims                 = LLVMGetParam(variant_func, 4);
   system_values.instance_id = LLVMGetParam(variant_func, 5);
   prim_id_ptr               = LLVMGetParam(variant_func, 6);
   system_values.invocation_id = LLVMGetParam(variant_func, 7);
   system_values.view_index  = LLVMGetParam(variant_func, 8);

   lp_build_name(context_ptr, "context");
   lp_build_name(resources_ptr, "resources");
   lp_build_name(input_array, "input");
   lp_build_name(io_ptr, "io");
   lp_build_name(num_prims, "num_prims");
   lp_build_name(system_values.instance_id, "instance_id");
   lp_build_name(prim_id_ptr, "prim_id_ptr");
   lp_build_name(system_values.invocation_id, "invocation_id");
   lp_build_name(system_values.view_index, "view_index");

   variant->context_ptr = context_ptr;
   variant->io_ptr = io_ptr;
   variant->num_prims = num_prims;

   gs_iface.base.fetch_input = draw_gs_llvm_fetch_input;
   gs_iface.base.emit_vertex = draw_gs_llvm_emit_vertex;
   gs_iface.base.end_primitive = draw_gs_llvm_end_primitive;
   gs_iface.base.gs_epilogue = draw_gs_llvm_epilogue;
   gs_iface.input = input_array;
   gs_iface.variant = variant;

   /*
    * Function body
    */

   block = LLVMAppendBasicBlockInContext(gallivm->context, variant_func, "entry");
   builder = gallivm->builder;
   LLVMPositionBuilderAtEnd(builder, block);

   lp_build_context_init(&bld, gallivm, lp_type_int(32));

   memset(&gs_type, 0, sizeof gs_type);
   gs_type.floating = true; /* floating point values */
   gs_type.sign = true;     /* values are signed */
   gs_type.norm = false;    /* values are not limited to [0,1] or [-1,1] */
   gs_type.width = 32;      /* 32-bit float */
   gs_type.length = vector_length;

   consts_ptr = lp_jit_resources_constants(gallivm, variant->resources_type, resources_ptr);

   ssbos_ptr = lp_jit_resources_ssbos(gallivm, variant->resources_type, resources_ptr);

   /* code generated texture sampling */
   sampler = lp_bld_llvm_sampler_soa_create(variant->key.samplers,
                                          MAX2(variant->key.nr_samplers,
                                               variant->key.nr_sampler_views));
   image = lp_bld_llvm_image_soa_create(draw_gs_llvm_variant_key_images(&variant->key),
                                      variant->key.nr_images);
   mask_val = generate_mask_value(variant, gs_type);
   lp_build_mask_begin(&mask, gallivm, gs_type, mask_val);

   if (gs_info->uses_primid) {
      system_values.prim_id = LLVMBuildLoad2(builder, prim_id_type, prim_id_ptr, "prim_id");
   }

   if (gallivm_debug & (GALLIVM_DEBUG_TGSI | GALLIVM_DEBUG_IR)) {
      if (llvm->draw->gs.geometry_shader->state.type == PIPE_SHADER_IR_TGSI)
         tgsi_dump(tokens, 0);
      else
         nir_print_shader(llvm->draw->gs.geometry_shader->state.ir.nir, stderr);
      draw_gs_llvm_dump_variant_key(&variant->key);
   }

   struct lp_build_tgsi_params params;
   memset(&params, 0, sizeof(params));

   params.type = gs_type;
   params.mask = &mask;
   params.consts_ptr = consts_ptr;
   params.system_values = &system_values;
   params.context_type = variant->context_type;
   params.context_ptr = context_ptr;
   params.resources_type = variant->resources_type;
   params.resources_ptr = resources_ptr;
   params.sampler = sampler;
   params.info = &llvm->draw->gs.geometry_shader->info;
   params.gs_iface = (const struct lp_build_gs_iface *)&gs_iface;
   params.ssbo_ptr = ssbos_ptr;
   params.image = image;
   params.gs_vertex_streams = variant->shader->base.num_vertex_streams;
   params.aniso_filter_table = lp_jit_resources_aniso_filter_table(gallivm,
                                                                   variant->resources_type,
                                                                   resources_ptr);


   if (llvm->draw->gs.geometry_shader->state.type == PIPE_SHADER_IR_TGSI)
      lp_build_tgsi_soa(variant->gallivm,
                        tokens,
                        &params,
                        outputs);
   else
      lp_build_nir_soa(variant->gallivm,
                       llvm->draw->gs.geometry_shader->state.ir.nir,
                       &params,
                       outputs);

   lp_bld_llvm_sampler_soa_destroy(sampler);
   lp_bld_llvm_image_soa_destroy(image);

   lp_build_mask_end(&mask);

   LLVMBuildRet(builder, lp_build_zero(gallivm, lp_type_uint(32)));

   gallivm_verify_function(gallivm, variant_func);
}


struct draw_gs_llvm_variant *
draw_gs_llvm_create_variant(struct draw_llvm *llvm,
                            unsigned num_outputs,
                            const struct draw_gs_llvm_variant_key *key)
{
   struct draw_gs_llvm_variant *variant;
   struct llvm_geometry_shader *shader =
      llvm_geometry_shader(llvm->draw->gs.geometry_shader);
   char module_name[64];
   unsigned char ir_sha1_cache_key[20];
   struct lp_cached_code cached = { 0 };
   bool needs_caching = false;

   variant = MALLOC(sizeof *variant +
                    shader->variant_key_size -
                    sizeof variant->key);
   if (!variant)
      return NULL;

   variant->llvm = llvm;
   variant->shader = shader;

   snprintf(module_name, sizeof(module_name), "draw_llvm_gs_variant%u",
            variant->shader->variants_cached);

   memcpy(&variant->key, key, shader->variant_key_size);

   if (shader->base.state.ir.nir && llvm->draw->disk_cache_cookie) {
      draw_get_ir_cache_key(shader->base.state.ir.nir,
                            key,
                            shader->variant_key_size,
                            num_outputs,
                            ir_sha1_cache_key);

      llvm->draw->disk_cache_find_shader(llvm->draw->disk_cache_cookie,
                                         &cached,
                                         ir_sha1_cache_key);
      if (!cached.data_size)
         needs_caching = true;
   }
   variant->gallivm = gallivm_create(module_name, llvm->context, &cached);

   create_gs_jit_types(variant);

   variant->vertex_header_type = lp_build_create_jit_vertex_header_type(variant->gallivm, num_outputs);
   variant->vertex_header_ptr_type = LLVMPointerType(variant->vertex_header_type, 0);

   draw_gs_llvm_generate(llvm, variant);

   gallivm_compile_module(variant->gallivm);

   variant->jit_func = (draw_gs_jit_func)
         gallivm_jit_function(variant->gallivm, variant->function);

   if (needs_caching)
      llvm->draw->disk_cache_insert_shader(llvm->draw->disk_cache_cookie,
                                           &cached,
                                           ir_sha1_cache_key);
   gallivm_free_ir(variant->gallivm);

   variant->list_item_global.base = variant;
   variant->list_item_local.base = variant;
   /*variant->no = */shader->variants_created++;
   variant->list_item_global.base = variant;

   return variant;
}


void
draw_gs_llvm_destroy_variant(struct draw_gs_llvm_variant *variant)
{
   struct draw_llvm *llvm = variant->llvm;

   if (gallivm_debug & (GALLIVM_DEBUG_TGSI | GALLIVM_DEBUG_IR)) {
      debug_printf("Deleting GS variant: %u gs variants,\t%u total variants\n",
                    variant->shader->variants_cached, llvm->nr_gs_variants);
   }

   gallivm_destroy(variant->gallivm);

   list_del(&variant->list_item_local.list);
   variant->shader->variants_cached--;
   list_del(&variant->list_item_global.list);
   llvm->nr_gs_variants--;
   FREE(variant);
}


struct draw_gs_llvm_variant_key *
draw_gs_llvm_make_variant_key(struct draw_llvm *llvm, char *store)
{
   struct draw_gs_llvm_variant_key *key;
   struct lp_sampler_static_state *draw_sampler;
   struct lp_image_static_state *draw_image;

   key = (struct draw_gs_llvm_variant_key *)store;

   memset(key, 0, offsetof(struct draw_gs_llvm_variant_key, samplers[0]));

   key->num_outputs = draw_total_gs_outputs(llvm->draw);

   key->clamp_vertex_color = llvm->draw->rasterizer->clamp_vertex_color;

   /* All variants of this shader will have the same value for
    * nr_samplers.  Not yet trying to compact away holes in the
    * sampler array.
    */
   key->nr_samplers = llvm->draw->gs.geometry_shader->info.file_max[TGSI_FILE_SAMPLER] + 1;
   if (llvm->draw->gs.geometry_shader->info.file_max[TGSI_FILE_SAMPLER_VIEW] != -1) {
      key->nr_sampler_views =
         llvm->draw->gs.geometry_shader->info.file_max[TGSI_FILE_SAMPLER_VIEW] + 1;
   } else {
      key->nr_sampler_views = key->nr_samplers;
   }

   key->nr_images = llvm->draw->gs.geometry_shader->info.file_max[TGSI_FILE_IMAGE] + 1;

   draw_sampler = key->samplers;

   memset(draw_sampler, 0, MAX2(key->nr_samplers, key->nr_sampler_views) * sizeof *draw_sampler);

   for (unsigned i = 0 ; i < key->nr_samplers; i++) {
      lp_sampler_static_sampler_state(&draw_sampler[i].sampler_state,
                                      llvm->draw->samplers[PIPE_SHADER_GEOMETRY][i]);
   }
   for (unsigned i = 0 ; i < key->nr_sampler_views; i++) {
      lp_sampler_static_texture_state(&draw_sampler[i].texture_state,
                                      llvm->draw->sampler_views[PIPE_SHADER_GEOMETRY][i]);
   }

   draw_image = draw_gs_llvm_variant_key_images(key);
   memset(draw_image, 0,
          key->nr_images * sizeof *draw_image);
   for (unsigned i = 0; i < key->nr_images; i++) {
      lp_sampler_static_texture_state_image(&draw_image[i].image_state,
                                            llvm->draw->images[PIPE_SHADER_GEOMETRY][i]);
   }
   return key;
}


void
draw_gs_llvm_dump_variant_key(struct draw_gs_llvm_variant_key *key)
{
   struct lp_sampler_static_state *sampler = key->samplers;
   struct lp_image_static_state *image = draw_gs_llvm_variant_key_images(key);

   debug_printf("clamp_vertex_color = %u\n", key->clamp_vertex_color);
   for (unsigned i = 0 ; i < key->nr_sampler_views; i++) {
      debug_printf("sampler[%i].src_format = %s\n", i,
                   util_format_name(sampler[i].texture_state.format));
   }

   for (unsigned i = 0 ; i < key->nr_images; i++)
      debug_printf("images[%i].format = %s\n", i, util_format_name(image[i].image_state.format));

}


static void
create_tcs_jit_types(struct draw_tcs_llvm_variant *var)
{
   struct gallivm_state *gallivm = var->gallivm;

   var->resources_type = lp_build_jit_resources_type(gallivm);
   var->resources_ptr_type = LLVMPointerType(var->resources_type, 0);
   var->input_array_type = create_tcs_jit_input_type(gallivm);
   var->output_array_type = create_tcs_jit_output_type(gallivm);
}


static LLVMTypeRef
get_tcs_resources_ptr_type(struct draw_tcs_llvm_variant *variant)
{
   if (!variant->resources_ptr_type)
      create_tcs_jit_types(variant);
   return variant->resources_ptr_type;
}


static LLVMValueRef
draw_tcs_llvm_emit_fetch_input(const struct lp_build_tcs_iface *tes_iface,
                               struct lp_build_context *bld,
                               bool is_vindex_indirect,
                               LLVMValueRef vertex_index,
                               bool is_aindex_indirect,
                               LLVMValueRef attrib_index,
                               bool is_sindex_indirect,
                               LLVMValueRef swizzle_index)
{
   const struct draw_tcs_llvm_iface *tcs = draw_tcs_llvm_iface(tes_iface);
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef indices[3];
   LLVMValueRef res;
   struct lp_type type = bld->type;
   LLVMTypeRef input_type = create_tcs_jit_input_type_deref(gallivm);
   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);

   if (is_vindex_indirect || is_aindex_indirect || is_sindex_indirect) {
      res = bld->zero;
      for (int i = 0; i < type.length; ++i) {
         LLVMValueRef idx = lp_build_const_int32(gallivm, i);
         LLVMValueRef vert_chan_index = vertex_index;
         LLVMValueRef attr_chan_index = attrib_index;
         LLVMValueRef swiz_chan_index = swizzle_index;
         LLVMValueRef channel_vec;

         if (is_vindex_indirect) {
            vert_chan_index = LLVMBuildExtractElement(builder,
                                                      vertex_index, idx, "");
         }
         if (is_aindex_indirect) {
            attr_chan_index = LLVMBuildExtractElement(builder,
                                                      attrib_index, idx, "");
         }
         if (is_sindex_indirect) {
            swiz_chan_index = LLVMBuildExtractElement(builder,
                                                      swizzle_index, idx, "");
         }

         indices[0] = vert_chan_index;
         indices[1] = attr_chan_index;
         indices[2] = swiz_chan_index;

         channel_vec = LLVMBuildGEP2(builder, input_type, tcs->input, indices, 3, "");
         channel_vec = LLVMBuildLoad2(builder, float_type, channel_vec, "");
         res = LLVMBuildInsertElement(builder, res, channel_vec, idx, "");
      }
   } else {
      indices[0] = vertex_index;
      indices[1] = attrib_index;
      indices[2] = swizzle_index;
      res = LLVMBuildGEP2(builder, input_type, tcs->input, indices, 3, "");
      res = LLVMBuildLoad2(builder, float_type, res, "");
      res = lp_build_broadcast_scalar(bld, res);
   }
   return res;
}


static LLVMValueRef
draw_tcs_llvm_emit_fetch_output(const struct lp_build_tcs_iface *tes_iface,
                                struct lp_build_context *bld,
                                bool is_vindex_indirect,
                                LLVMValueRef vertex_index,
                                bool is_aindex_indirect,
                                LLVMValueRef attrib_index,
                                bool is_sindex_indirect,
                                LLVMValueRef swizzle_index,
                                uint32_t name)
{
   const struct draw_tcs_llvm_iface *tcs = draw_tcs_llvm_iface(tes_iface);
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef indices[3];
   LLVMValueRef res;
   struct lp_type type = bld->type;
   LLVMTypeRef output_type = create_tcs_jit_output_type_deref(gallivm);
   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);

   if (is_vindex_indirect || is_aindex_indirect || is_sindex_indirect) {
      res = bld->zero;
      for (int i = 0; i < type.length; ++i) {
         LLVMValueRef idx = lp_build_const_int32(gallivm, i);
         LLVMValueRef vert_chan_index = vertex_index;
         LLVMValueRef attr_chan_index = attrib_index;
         LLVMValueRef swiz_chan_index = swizzle_index;
         LLVMValueRef channel_vec;

         if (is_vindex_indirect) {
            vert_chan_index = LLVMBuildExtractElement(builder,
                                                      vertex_index, idx, "");
         }
         if (is_aindex_indirect) {
            attr_chan_index = LLVMBuildExtractElement(builder,
                                                      attrib_index, idx, "");
         }
         if (is_sindex_indirect) {
            swiz_chan_index = LLVMBuildExtractElement(builder,
                                                      swizzle_index, idx, "");
         }

         indices[0] = vert_chan_index;
         indices[1] = attr_chan_index;
         indices[2] = swiz_chan_index;

         channel_vec = LLVMBuildGEP2(builder, output_type, tcs->output, indices, 3, "");
         channel_vec = LLVMBuildLoad2(builder, float_type, channel_vec, "");

         res = LLVMBuildInsertElement(builder, res, channel_vec, idx, "");
      }
   } else {
      indices[0] = vertex_index ? vertex_index : lp_build_const_int32(gallivm, 0);
      indices[1] = attrib_index;
      indices[2] = swizzle_index;

      res = LLVMBuildGEP2(builder, output_type, tcs->output, indices, 3, "");
      res = LLVMBuildLoad2(builder, float_type, res, "");
      res = lp_build_broadcast_scalar(bld, res);
   }
   return res;
}


static void
draw_tcs_llvm_emit_store_output(const struct lp_build_tcs_iface *tes_iface,
                                struct lp_build_context *bld,
                                unsigned name,
                                bool is_vindex_indirect,
                                LLVMValueRef vertex_index,
                                bool is_aindex_indirect,
                                LLVMValueRef attrib_index,
                                bool is_sindex_indirect,
                                LLVMValueRef swizzle_index,
                                LLVMValueRef value,
                                LLVMValueRef mask_vec)
{
   const struct draw_tcs_llvm_iface *tcs = draw_tcs_llvm_iface(tes_iface);
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef indices[3];
   LLVMValueRef res;
   struct lp_type type = bld->type;
   LLVMTypeRef output_type = create_tcs_jit_output_type_deref(gallivm);

   if (is_vindex_indirect || is_aindex_indirect || is_sindex_indirect) {
      for (int i = 0; i < type.length; ++i) {
         LLVMValueRef idx = lp_build_const_int32(gallivm, i);
         LLVMValueRef vert_chan_index = vertex_index ? vertex_index : lp_build_const_int32(gallivm, 0);
         LLVMValueRef attr_chan_index = attrib_index;
         LLVMValueRef swiz_chan_index = swizzle_index;
         LLVMValueRef channel_vec;

         if (is_vindex_indirect) {
            vert_chan_index = LLVMBuildExtractElement(builder,
                                                      vertex_index, idx, "");
         }
         if (is_aindex_indirect) {
            attr_chan_index = LLVMBuildExtractElement(builder,
                                                      attrib_index, idx, "");
         }

         if (is_sindex_indirect) {
            swiz_chan_index = LLVMBuildExtractElement(builder,
                                                      swizzle_index, idx, "");
         }

         indices[0] = vert_chan_index;
         indices[1] = attr_chan_index;
         indices[2] = swiz_chan_index;

         channel_vec = LLVMBuildGEP2(builder, output_type, tcs->output, indices, 3, "");

         res = LLVMBuildExtractElement(builder, value, idx, "");

         struct lp_build_if_state ifthen;
         LLVMValueRef cond = LLVMBuildICmp(gallivm->builder, LLVMIntNE, mask_vec, lp_build_const_int_vec(gallivm, bld->type, 0), "");
         cond = LLVMBuildExtractElement(gallivm->builder, cond, idx, "");
         lp_build_if(&ifthen, gallivm, cond);
         LLVMBuildStore(builder, res, channel_vec);
         lp_build_endif(&ifthen);
      }
   } else {
      indices[0] = vertex_index ? vertex_index : lp_build_const_int32(gallivm, 0);
      indices[1] = attrib_index;
      indices[2] = swizzle_index;

      res = LLVMBuildGEP2(builder, output_type, tcs->output, indices, 3, "");
      for (unsigned i = 0; i < type.length; ++i) {
         LLVMValueRef idx = lp_build_const_int32(gallivm, i);
         LLVMValueRef val = LLVMBuildExtractElement(builder, value, idx, "");

         struct lp_build_if_state ifthen;
         LLVMValueRef cond = LLVMBuildICmp(gallivm->builder, LLVMIntNE, mask_vec, lp_build_const_int_vec(gallivm, bld->type, 0), "");
         cond = LLVMBuildExtractElement(gallivm->builder, cond, idx, "");
         lp_build_if(&ifthen, gallivm, cond);
         LLVMBuildStore(builder, val, res);
         lp_build_endif(&ifthen);
      }
   }
}


static LLVMValueRef
generate_tcs_mask_value(struct draw_tcs_llvm_variant *variant,
                        struct lp_type tcs_type, LLVMValueRef limit, LLVMValueRef loop_counter)
{
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type mask_type = lp_int_type(tcs_type);
   LLVMValueRef num_vecs;
   LLVMValueRef mask_val = lp_build_const_vec(gallivm, mask_type, 0);

   num_vecs = lp_build_broadcast(gallivm, lp_build_vec_type(gallivm, mask_type), limit);
   for (unsigned i = 0; i < tcs_type.length; i++) {
      LLVMValueRef idx = lp_build_const_int32(gallivm, i);
      mask_val = LLVMBuildInsertElement(builder, mask_val, LLVMBuildAdd(builder, loop_counter, idx, ""), idx, "");
   }
   mask_val = lp_build_compare(gallivm, mask_type,
                               PIPE_FUNC_GREATER, num_vecs, mask_val);

   return mask_val;
}


static void
draw_tcs_llvm_generate(struct draw_llvm *llvm,
                       struct draw_tcs_llvm_variant *variant)
{
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMContextRef context = gallivm->context;
   LLVMTypeRef int32_type = LLVMInt32TypeInContext(context);
   LLVMTypeRef arg_types[7];
   LLVMTypeRef func_type, coro_func_type;
   LLVMValueRef variant_func, variant_coro;
   LLVMValueRef resources_ptr;
   LLVMValueRef view_index;
   LLVMValueRef input_array, output_array, prim_id, patch_vertices_in;
   LLVMValueRef mask_val;
   LLVMBasicBlockRef block;
   LLVMBuilderRef builder;
   struct lp_build_context bld, bldvec;
   struct lp_build_sampler_soa *sampler = 0;
   struct lp_build_image_soa *image = NULL;
   struct lp_bld_tgsi_system_values system_values;
   char func_name[64], func_name_coro[64];
   struct draw_tcs_llvm_iface tcs_iface;
   struct lp_build_mask_context mask;
   LLVMValueRef consts_ptr;
   LLVMValueRef ssbos_ptr;
   struct lp_type tcs_type;
   unsigned vector_length = variant->shader->base.vector_length;

   memset(&system_values, 0, sizeof(system_values));

   snprintf(func_name, sizeof(func_name), "draw_llvm_tcs_variant");

   snprintf(func_name_coro, sizeof(func_name_coro), "draw_llvm_tcs_coro_variant");

   arg_types[0] = get_tcs_resources_ptr_type(variant);    /* context */
   arg_types[1] = variant->input_array_type;           /* input */
   arg_types[2] = variant->output_array_type;
   arg_types[3] = int32_type;
   arg_types[4] = int32_type;
   arg_types[5] = int32_type;
   arg_types[6] = int32_type; /* coroutine only */

   func_type = LLVMFunctionType(int32_type, arg_types, ARRAY_SIZE(arg_types) - 1, 0);

   coro_func_type = LLVMFunctionType(LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0), arg_types, ARRAY_SIZE(arg_types), 0);

   variant_func = LLVMAddFunction(gallivm->module, func_name, func_type);

   variant_coro = LLVMAddFunction(gallivm->module, func_name_coro, coro_func_type);

   variant->function = variant_func;
   LLVMSetFunctionCallConv(variant_func, LLVMCCallConv);

   LLVMSetFunctionCallConv(variant_coro, LLVMCCallConv);

   lp_build_coro_add_presplit(variant_coro);

   for (unsigned i = 0; i < ARRAY_SIZE(arg_types); ++i) {
      if (LLVMGetTypeKind(arg_types[i]) == LLVMPointerTypeKind) {
         lp_add_function_attr(variant_coro, i + 1, LP_FUNC_ATTR_NOALIAS);
         lp_add_function_attr(variant_func, i + 1, LP_FUNC_ATTR_NOALIAS);
      }
   }

   if (gallivm->cache && gallivm->cache->data_size)
      return;
   resources_ptr               = LLVMGetParam(variant_func, 0);
   input_array               = LLVMGetParam(variant_func, 1);
   output_array              = LLVMGetParam(variant_func, 2);
   prim_id                   = LLVMGetParam(variant_func, 3);
   patch_vertices_in         = LLVMGetParam(variant_func, 4);
   view_index                = LLVMGetParam(variant_func, 5);

   lp_build_name(resources_ptr, "resources");
   lp_build_name(input_array, "input");
   lp_build_name(output_array, "output");
   lp_build_name(prim_id, "prim_id");
   lp_build_name(patch_vertices_in, "patch_vertices_in");
   lp_build_name(view_index, "view_index");

   block = LLVMAppendBasicBlockInContext(gallivm->context, variant_func, "entry");
   builder = gallivm->builder;
   LLVMPositionBuilderAtEnd(builder, block);

   lp_build_context_init(&bld, gallivm, lp_type_int(32));

   memset(&tcs_type, 0, sizeof tcs_type);
   tcs_type.floating = true; /* floating point values */
   tcs_type.sign = true;     /* values are signed */
   tcs_type.norm = false;    /* values are not limited to [0,1] or [-1,1] */
   tcs_type.width = 32;      /* 32-bit float */
   tcs_type.length = vector_length;

   lp_build_context_init(&bldvec, variant->gallivm, lp_int_type(tcs_type));

   LLVMValueRef count = lp_build_const_int32(gallivm, variant->shader->base.vertices_out);
   LLVMValueRef step = lp_build_const_int32(gallivm, vector_length);

   struct lp_build_loop_state loop_state[2];
   LLVMValueRef num_inner_loop;
   unsigned count_align = util_align_npot(variant->shader->base.vertices_out, tcs_type.length);
   num_inner_loop = lp_build_const_int32(gallivm, count_align / tcs_type.length);
   LLVMTypeRef hdl_ptr_type = LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0);
   LLVMValueRef coro_hdls = LLVMBuildArrayAlloca(gallivm->builder, hdl_ptr_type, num_inner_loop, "coro_hdls");
   unsigned end_coroutine = INT_MAX;
   lp_build_loop_begin(&loop_state[1], gallivm,
                       lp_build_const_int32(gallivm, 0)); /* coroutine reentry loop */
   lp_build_loop_begin(&loop_state[0], gallivm,
                       lp_build_const_int32(gallivm, 0)); /* inner loop */
   {
      LLVMValueRef args[7];
      args[0] = resources_ptr;
      args[1] = input_array;
      args[2] = output_array;
      args[3] = prim_id;
      args[4] = patch_vertices_in;
      args[5] = view_index;
      args[6] = loop_state[0].counter;
      LLVMValueRef coro_entry = LLVMBuildGEP2(builder, hdl_ptr_type, coro_hdls, &loop_state[0].counter, 1, "");
      LLVMValueRef coro_hdl = LLVMBuildLoad2(builder, hdl_ptr_type, coro_entry, "coro_hdl");

      struct lp_build_if_state ifstate;
      LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntEQ, loop_state[1].counter,
                                       lp_build_const_int32(gallivm, 0), "");
      /* first time here - call the coroutine function entry point */
      lp_build_if(&ifstate, gallivm, cmp);
      LLVMValueRef coro_ret = LLVMBuildCall2(builder, coro_func_type, variant_coro, args, 7, "");
      LLVMBuildStore(builder, coro_ret, coro_entry);
      lp_build_else(&ifstate);
      /* subsequent calls for this invocation - check if done. */
      LLVMValueRef coro_done = lp_build_coro_done(gallivm, coro_hdl);
      struct lp_build_if_state ifstate2;
      lp_build_if(&ifstate2, gallivm, coro_done);
      /* if done destroy and force loop exit */
      lp_build_coro_destroy(gallivm, coro_hdl);
      lp_build_loop_force_set_counter(&loop_state[1], lp_build_const_int32(gallivm, end_coroutine - 1));
      lp_build_else(&ifstate2);
      /* otherwise resume the coroutine */
      lp_build_coro_resume(gallivm, coro_hdl);
      lp_build_endif(&ifstate2);
      lp_build_endif(&ifstate);
      lp_build_loop_force_reload_counter(&loop_state[1]);
   }
   lp_build_loop_end_cond(&loop_state[0],
                          num_inner_loop,
                          NULL,  LLVMIntUGE);
   lp_build_loop_end_cond(&loop_state[1],
                          lp_build_const_int32(gallivm, end_coroutine),
                          NULL, LLVMIntEQ);
   LLVMBuildRet(builder, lp_build_zero(gallivm, lp_type_uint(32)));

   block = LLVMAppendBasicBlockInContext(gallivm->context, variant_coro, "entry");
   LLVMPositionBuilderAtEnd(builder, block);

   resources_ptr = LLVMGetParam(variant_coro, 0);
   input_array = LLVMGetParam(variant_coro, 1);
   output_array = LLVMGetParam(variant_coro, 2);
   prim_id = LLVMGetParam(variant_coro, 3);
   patch_vertices_in = LLVMGetParam(variant_coro, 4);
   view_index = LLVMGetParam(variant_coro, 5);

   consts_ptr = lp_jit_resources_constants(gallivm, variant->resources_type, resources_ptr);

   ssbos_ptr = lp_jit_resources_ssbos(gallivm, variant->resources_type, resources_ptr);
   sampler = lp_bld_llvm_sampler_soa_create(variant->key.samplers,
                                          MAX2(variant->key.nr_samplers,
                                               variant->key.nr_sampler_views));
   image = lp_bld_llvm_image_soa_create(draw_tcs_llvm_variant_key_images(&variant->key),
                                      variant->key.nr_images);

   LLVMValueRef counter = LLVMGetParam(variant_coro, 6);
   LLVMValueRef invocvec = LLVMGetUndef(LLVMVectorType(int32_type, vector_length));
   for (unsigned i = 0; i < vector_length; i++) {
      LLVMValueRef loop_iter = lp_build_const_int32(gallivm, i);
      LLVMValueRef idx = LLVMBuildAdd(builder, LLVMBuildMul(builder, counter, step, ""), loop_iter, "");
      invocvec = LLVMBuildInsertElement(builder, invocvec, idx, loop_iter, "");
   }

   system_values.invocation_id = invocvec;
   system_values.prim_id = lp_build_broadcast_scalar(&bldvec, prim_id);
   system_values.view_index = view_index;
   system_values.vertices_in = lp_build_broadcast_scalar(&bldvec, patch_vertices_in);
   tcs_iface.input = input_array;
   tcs_iface.output = output_array;
   tcs_iface.base.emit_fetch_input = draw_tcs_llvm_emit_fetch_input;
   tcs_iface.base.emit_fetch_output = draw_tcs_llvm_emit_fetch_output;
   tcs_iface.base.emit_store_output = draw_tcs_llvm_emit_store_output;


   {
      LLVMValueRef coro_id = lp_build_coro_id(gallivm);
      LLVMValueRef coro_hdl = lp_build_coro_begin_alloc_mem(gallivm, coro_id);

      mask_val = generate_tcs_mask_value(variant, tcs_type, count, LLVMBuildMul(builder, counter, step, ""));
      lp_build_mask_begin(&mask, gallivm, tcs_type, mask_val);

      struct lp_build_coro_suspend_info coro_info;

      LLVMBasicBlockRef sus_block = LLVMAppendBasicBlockInContext(gallivm->context, variant_coro, "suspend");
      LLVMBasicBlockRef clean_block = LLVMAppendBasicBlockInContext(gallivm->context, variant_coro, "cleanup");

      coro_info.suspend = sus_block;
      coro_info.cleanup = clean_block;

      struct lp_build_tgsi_params params;
      memset(&params, 0, sizeof(params));

      params.type = tcs_type;
      params.mask = &mask;
      params.consts_ptr = consts_ptr;
      params.system_values = &system_values;
      params.resources_type = variant->resources_type;
      params.resources_ptr = resources_ptr;
      params.sampler = sampler;
      params.info = &llvm->draw->tcs.tess_ctrl_shader->info;
      params.ssbo_ptr = ssbos_ptr;
      params.image = image;
      params.coro = &coro_info;
      params.tcs_iface = &tcs_iface.base;
      params.aniso_filter_table = lp_jit_resources_aniso_filter_table(gallivm,
                                                                      variant->resources_type,
                                                                      resources_ptr);

      lp_build_nir_soa(variant->gallivm,
                       llvm->draw->tcs.tess_ctrl_shader->state.ir.nir,
                       &params, NULL);

      lp_build_mask_end(&mask);

      lp_build_coro_suspend_switch(gallivm, &coro_info, NULL, true);
      LLVMPositionBuilderAtEnd(builder, clean_block);

      lp_build_coro_free_mem(gallivm, coro_id, coro_hdl);

      LLVMBuildBr(builder, sus_block);
      LLVMPositionBuilderAtEnd(builder, sus_block);

      lp_build_coro_end(gallivm, coro_hdl);
      LLVMBuildRet(builder, coro_hdl);
   }

   lp_bld_llvm_sampler_soa_destroy(sampler);
   lp_bld_llvm_image_soa_destroy(image);
   gallivm_verify_function(gallivm, variant_func);
   gallivm_verify_function(gallivm, variant_coro);
}


struct draw_tcs_llvm_variant *
draw_tcs_llvm_create_variant(struct draw_llvm *llvm,
                             unsigned num_outputs,
                             const struct draw_tcs_llvm_variant_key *key)
{
   struct draw_tcs_llvm_variant *variant;
   struct llvm_tess_ctrl_shader *shader = llvm_tess_ctrl_shader(llvm->draw->tcs.tess_ctrl_shader);
   char module_name[64];
   unsigned char ir_sha1_cache_key[20];
   struct lp_cached_code cached = { 0 };
   bool needs_caching = false;

   variant = MALLOC(sizeof *variant +
                    shader->variant_key_size - sizeof variant->key);
   if (!variant)
      return NULL;

   variant->llvm = llvm;
   variant->shader = shader;

   snprintf(module_name, sizeof(module_name), "draw_llvm_tcs_variant%u",
            variant->shader->variants_cached);

   memcpy(&variant->key, key, shader->variant_key_size);

   if (shader->base.state.ir.nir && llvm->draw->disk_cache_cookie) {
      draw_get_ir_cache_key(shader->base.state.ir.nir,
                            key,
                            shader->variant_key_size,
                            num_outputs,
                            ir_sha1_cache_key);

      llvm->draw->disk_cache_find_shader(llvm->draw->disk_cache_cookie,
                                         &cached,
                                         ir_sha1_cache_key);
      if (!cached.data_size)
         needs_caching = true;
   }

   variant->gallivm = gallivm_create(module_name, llvm->context, &cached);

   create_tcs_jit_types(variant);

   if (gallivm_debug & (GALLIVM_DEBUG_TGSI | GALLIVM_DEBUG_IR)) {
      nir_print_shader(llvm->draw->tcs.tess_ctrl_shader->state.ir.nir, stderr);
      draw_tcs_llvm_dump_variant_key(&variant->key);
   }

   draw_tcs_llvm_generate(llvm, variant);

   gallivm_compile_module(variant->gallivm);

   variant->jit_func = (draw_tcs_jit_func)
      gallivm_jit_function(variant->gallivm, variant->function);

   if (needs_caching)
      llvm->draw->disk_cache_insert_shader(llvm->draw->disk_cache_cookie,
                                           &cached,
                                           ir_sha1_cache_key);
   gallivm_free_ir(variant->gallivm);

   variant->list_item_global.base = variant;
   variant->list_item_local.base = variant;
   /*variant->no = */shader->variants_created++;
   variant->list_item_global.base = variant;

   return variant;
}


void
draw_tcs_llvm_destroy_variant(struct draw_tcs_llvm_variant *variant)
{
   struct draw_llvm *llvm = variant->llvm;

   if (gallivm_debug & (GALLIVM_DEBUG_TGSI | GALLIVM_DEBUG_IR)) {
      debug_printf("Deleting TCS variant: %u tcs variants,\t%u total variants\n",
                    variant->shader->variants_cached, llvm->nr_tcs_variants);
   }

   gallivm_destroy(variant->gallivm);

   list_del(&variant->list_item_local.list);
   variant->shader->variants_cached--;
   list_del(&variant->list_item_global.list);
   llvm->nr_tcs_variants--;
   FREE(variant);
}


struct draw_tcs_llvm_variant_key *
draw_tcs_llvm_make_variant_key(struct draw_llvm *llvm, char *store)
{
   unsigned i;
   struct draw_tcs_llvm_variant_key *key;
   struct lp_sampler_static_state *draw_sampler;
   struct lp_image_static_state *draw_image;

   key = (struct draw_tcs_llvm_variant_key *)store;

   memset(key, 0, offsetof(struct draw_tcs_llvm_variant_key, samplers[0]));

   /* All variants of this shader will have the same value for
    * nr_samplers.  Not yet trying to compact away holes in the
    * sampler array.
    */
   key->nr_samplers = llvm->draw->tcs.tess_ctrl_shader->info.file_max[TGSI_FILE_SAMPLER] + 1;
   if (llvm->draw->tcs.tess_ctrl_shader->info.file_max[TGSI_FILE_SAMPLER_VIEW] != -1) {
      key->nr_sampler_views =
         llvm->draw->tcs.tess_ctrl_shader->info.file_max[TGSI_FILE_SAMPLER_VIEW] + 1;
   } else {
      key->nr_sampler_views = key->nr_samplers;
   }

   key->nr_images = llvm->draw->tcs.tess_ctrl_shader->info.file_max[TGSI_FILE_IMAGE] + 1;

   draw_sampler = key->samplers;

   memset(draw_sampler, 0, MAX2(key->nr_samplers, key->nr_sampler_views) * sizeof *draw_sampler);

   for (i = 0 ; i < key->nr_samplers; i++) {
      lp_sampler_static_sampler_state(&draw_sampler[i].sampler_state,
                                      llvm->draw->samplers[PIPE_SHADER_TESS_CTRL][i]);
   }
   for (i = 0 ; i < key->nr_sampler_views; i++) {
      lp_sampler_static_texture_state(&draw_sampler[i].texture_state,
                                      llvm->draw->sampler_views[PIPE_SHADER_TESS_CTRL][i]);
   }

   draw_image = draw_tcs_llvm_variant_key_images(key);
   memset(draw_image, 0,
          key->nr_images * sizeof *draw_image);
   for (i = 0; i < key->nr_images; i++) {
      lp_sampler_static_texture_state_image(&draw_image[i].image_state,
                                            llvm->draw->images[PIPE_SHADER_TESS_CTRL][i]);
   }
   return key;
}


void
draw_tcs_llvm_dump_variant_key(struct draw_tcs_llvm_variant_key *key)
{
   struct lp_sampler_static_state *sampler = key->samplers;
   struct lp_image_static_state *image = draw_tcs_llvm_variant_key_images(key);
   for (unsigned i = 0 ; i < key->nr_sampler_views; i++) {
      debug_printf("sampler[%i].src_format = %s\n", i,
                   util_format_name(sampler[i].texture_state.format));
   }

   for (unsigned i = 0 ; i < key->nr_images; i++)
      debug_printf("images[%i].format = %s\n", i, util_format_name(image[i].image_state.format));
}


static void
create_tes_jit_types(struct draw_tes_llvm_variant *var)
{
   struct gallivm_state *gallivm = var->gallivm;

   var->resources_type = lp_build_jit_resources_type(gallivm);
   var->resources_ptr_type = LLVMPointerType(var->resources_type, 0);
   var->input_array_deref_type = create_tes_jit_input_deref_type(gallivm);
   var->input_array_type = LLVMPointerType(var->input_array_deref_type, 0); /* num vertices per prim */
}


static LLVMTypeRef
get_tes_resources_ptr_type(struct draw_tes_llvm_variant *variant)
{
   if (!variant->resources_ptr_type)
      create_tes_jit_types(variant);
   return variant->resources_ptr_type;
}


static LLVMValueRef
generate_tes_mask_value(struct draw_tes_llvm_variant *variant,
                        struct lp_type tes_type, LLVMValueRef limit, LLVMValueRef loop_counter)
{
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type mask_type = lp_int_type(tes_type);
   LLVMValueRef num_prims;
   LLVMValueRef mask_val = lp_build_const_vec(gallivm, mask_type, 0);
   unsigned i;

   num_prims = lp_build_broadcast(gallivm, lp_build_vec_type(gallivm, mask_type), limit);
   for (i = 0; i < tes_type.length; i++) {
      LLVMValueRef idx = lp_build_const_int32(gallivm, i);
      mask_val = LLVMBuildInsertElement(builder, mask_val, LLVMBuildAdd(builder, loop_counter, idx, ""), idx, "");
   }
   mask_val = lp_build_compare(gallivm, mask_type,
                               PIPE_FUNC_GREATER, num_prims, mask_val);

   return mask_val;
}


static LLVMValueRef
draw_tes_llvm_fetch_vertex_input(const struct lp_build_tes_iface *tes_iface,
                                 struct lp_build_context *bld,
                                 bool is_vindex_indirect,
                                 LLVMValueRef vertex_index,
                                 bool is_aindex_indirect,
                                 LLVMValueRef attrib_index,
                                 bool is_sindex_indirect,
                                 LLVMValueRef swizzle_index)
{
   const struct draw_tes_llvm_iface *tes = draw_tes_llvm_iface(tes_iface);
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef indices[3];
   LLVMValueRef res;
   struct lp_type type = bld->type;

   if (is_vindex_indirect || is_aindex_indirect || is_sindex_indirect) {
      res = bld->zero;

      for (int i = 0; i < type.length; ++i) {
         LLVMValueRef idx = lp_build_const_int32(gallivm, i);
         LLVMValueRef vert_chan_index = vertex_index;
         LLVMValueRef attr_chan_index = attrib_index;
         LLVMValueRef swiz_chan_index = swizzle_index;
         LLVMValueRef channel_vec;

         if (is_vindex_indirect) {
            vert_chan_index = LLVMBuildExtractElement(builder,
                                                      vertex_index, idx, "");
         }
         if (is_aindex_indirect) {
            attr_chan_index = LLVMBuildExtractElement(builder,
                                                      attrib_index, idx, "");
         }
         if (is_sindex_indirect) {
            swiz_chan_index = LLVMBuildExtractElement(builder,
                                                      swizzle_index, idx, "");
         }

         indices[0] = vert_chan_index;
         indices[1] = attr_chan_index;
         indices[2] = swiz_chan_index;

         channel_vec = LLVMBuildGEP2(builder, tes->variant->input_array_deref_type, tes->input, indices, 3, "");
         channel_vec = LLVMBuildLoad2(builder, LLVMFloatTypeInContext(gallivm->context), channel_vec, "");

         res = LLVMBuildInsertElement(builder, res, channel_vec, idx, "");
      }
   } else {
      indices[0] = vertex_index;
      indices[1] = attrib_index;
      indices[2] = swizzle_index;

      res = LLVMBuildGEP2(builder, tes->variant->input_array_deref_type, tes->input, indices, 3, "");
      res = LLVMBuildLoad2(builder, LLVMFloatTypeInContext(gallivm->context), res, "");
      res = lp_build_broadcast_scalar(bld, res);
   }
   return res;
}


static LLVMValueRef
draw_tes_llvm_fetch_patch_input(const struct lp_build_tes_iface *tes_iface,
                                struct lp_build_context *bld,
                                bool is_aindex_indirect,
                                LLVMValueRef attrib_index,
                                LLVMValueRef swizzle_index)
{
   const struct draw_tes_llvm_iface *tes = draw_tes_llvm_iface(tes_iface);
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef indices[3];
   LLVMValueRef res;
   struct lp_type type = bld->type;

   if (is_aindex_indirect) {
      res = bld->zero;

      for (int i = 0; i < type.length; ++i) {
         LLVMValueRef idx = lp_build_const_int32(gallivm, i);
         LLVMValueRef attr_chan_index = attrib_index;
         LLVMValueRef channel_vec;

         if (is_aindex_indirect) {
            attr_chan_index = LLVMBuildExtractElement(builder,
                                                      attrib_index, idx, "");
         }

         indices[0] = lp_build_const_int32(gallivm, 0);
         indices[1] = attr_chan_index;
         indices[2] = swizzle_index;

         channel_vec = LLVMBuildGEP2(builder, tes->variant->input_array_deref_type, tes->input, indices, 3, "");
         channel_vec = LLVMBuildLoad2(builder, LLVMFloatTypeInContext(gallivm->context), channel_vec, "");

         res = LLVMBuildInsertElement(builder, res, channel_vec, idx, "");
      }
   } else {
      indices[0] = lp_build_const_int32(gallivm, 0);
      indices[1] = attrib_index;
      indices[2] = swizzle_index;

      res = LLVMBuildGEP2(builder, tes->variant->input_array_deref_type, tes->input, indices, 3, "");
      res = LLVMBuildLoad2(builder, LLVMFloatTypeInContext(gallivm->context), res, "");
      res = lp_build_broadcast_scalar(bld, res);
   }
   return res;
}


static void
draw_tes_llvm_generate(struct draw_llvm *llvm,
                       struct draw_tes_llvm_variant *variant)
{
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMContextRef context = gallivm->context;
   LLVMTypeRef int32_type = LLVMInt32TypeInContext(context);
   LLVMTypeRef flt_type = LLVMFloatTypeInContext(context);
   LLVMTypeRef arg_types[11];
   LLVMTypeRef func_type;
   LLVMValueRef variant_func;
   LLVMValueRef resources_ptr;
   LLVMValueRef tess_coord[2], io_ptr, input_array, num_tess_coord;
   LLVMValueRef view_index;
   LLVMValueRef tess_inner, tess_outer, prim_id, patch_vertices_in;
   LLVMBasicBlockRef block;
   LLVMBuilderRef builder;
   LLVMValueRef mask_val;
   struct lp_build_context bld, bldvec;
   struct lp_build_sampler_soa *sampler = 0;
   struct lp_build_image_soa *image = NULL;
   struct lp_bld_tgsi_system_values system_values;
   char func_name[64];
   unsigned i;
   struct draw_tes_llvm_iface tes_iface;
   LLVMValueRef outputs[PIPE_MAX_SHADER_OUTPUTS][TGSI_NUM_CHANNELS];
   struct lp_build_mask_context mask;
   LLVMValueRef consts_ptr;
   LLVMValueRef ssbos_ptr;
   LLVMValueRef step;
   struct lp_type tes_type;
   unsigned vector_length = variant->shader->base.vector_length;
   int primid_slot = -1;

   memset(&system_values, 0, sizeof(system_values));
   memset(&outputs, 0, sizeof(outputs));

   snprintf(func_name, sizeof(func_name), "draw_llvm_tes_variant");

   LLVMTypeRef tess_outer_deref_type = LLVMArrayType(flt_type, 4);
   LLVMTypeRef tess_inner_deref_type = LLVMArrayType(flt_type, 2);

   arg_types[0] = get_tes_resources_ptr_type(variant);    /* context */
   arg_types[1] = variant->input_array_type;           /* input */
   arg_types[2] = variant->vertex_header_ptr_type;
   arg_types[3] = int32_type;
   arg_types[4] = int32_type;
   arg_types[5] = LLVMPointerType(flt_type, 0);
   arg_types[6] = LLVMPointerType(flt_type, 0);
   arg_types[7] = LLVMPointerType(tess_outer_deref_type, 0);
   arg_types[8] = LLVMPointerType(tess_inner_deref_type, 0);
   arg_types[9] = int32_type;
   arg_types[10] = int32_type;

   func_type = LLVMFunctionType(int32_type, arg_types, ARRAY_SIZE(arg_types), 0);
   variant_func = LLVMAddFunction(gallivm->module, func_name, func_type);

   variant->function = variant_func;
   LLVMSetFunctionCallConv(variant_func, LLVMCCallConv);

   for (i = 0; i < ARRAY_SIZE(arg_types); ++i)
      if (LLVMGetTypeKind(arg_types[i]) == LLVMPointerTypeKind)
         lp_add_function_attr(variant_func, i + 1, LP_FUNC_ATTR_NOALIAS);

   if (gallivm->cache && gallivm->cache->data_size)
      return;
   resources_ptr               = LLVMGetParam(variant_func, 0);
   input_array               = LLVMGetParam(variant_func, 1);
   io_ptr                    = LLVMGetParam(variant_func, 2);
   prim_id                   = LLVMGetParam(variant_func, 3);
   num_tess_coord            = LLVMGetParam(variant_func, 4);
   tess_coord[0]             = LLVMGetParam(variant_func, 5);
   tess_coord[1]             = LLVMGetParam(variant_func, 6);
   tess_outer                = LLVMGetParam(variant_func, 7);
   tess_inner                = LLVMGetParam(variant_func, 8);
   patch_vertices_in         = LLVMGetParam(variant_func, 9);
   view_index                = LLVMGetParam(variant_func, 10);

   lp_build_name(resources_ptr, "resources");
   lp_build_name(input_array, "input");
   lp_build_name(io_ptr, "io");
   lp_build_name(prim_id, "prim_id");
   lp_build_name(num_tess_coord, "num_tess_coord");
   lp_build_name(tess_coord[0], "tess_coord[0]");
   lp_build_name(tess_coord[1], "tess_coord[1]");
   lp_build_name(tess_outer, "tess_outer");
   lp_build_name(tess_inner, "tess_inner");
   lp_build_name(patch_vertices_in, "patch_vertices_in");
   lp_build_name(view_index, "view_index");

   tes_iface.base.fetch_vertex_input = draw_tes_llvm_fetch_vertex_input;
   tes_iface.base.fetch_patch_input = draw_tes_llvm_fetch_patch_input;
   tes_iface.input = input_array;
   tes_iface.variant = variant;

   block = LLVMAppendBasicBlockInContext(gallivm->context, variant_func, "entry");
   builder = gallivm->builder;
   LLVMPositionBuilderAtEnd(builder, block);

   lp_build_context_init(&bld, gallivm, lp_type_int(32));

   memset(&tes_type, 0, sizeof tes_type);
   tes_type.floating = true; /* floating point values */
   tes_type.sign = true;     /* values are signed */
   tes_type.norm = false;    /* values are not limited to [0,1] or [-1,1] */
   tes_type.width = 32;      /* 32-bit float */
   tes_type.length = vector_length;

   lp_build_context_init(&bldvec, variant->gallivm, lp_int_type(tes_type));
   consts_ptr = lp_jit_resources_constants(gallivm, variant->resources_type, resources_ptr);

   ssbos_ptr = lp_jit_resources_ssbos(gallivm, variant->resources_type, resources_ptr);

   sampler = lp_bld_llvm_sampler_soa_create(variant->key.samplers,
                                          MAX2(variant->key.nr_samplers,
                                               variant->key.nr_sampler_views));
   image = lp_bld_llvm_image_soa_create(draw_tes_llvm_variant_key_images(&variant->key),
                                      variant->key.nr_images);
   step = lp_build_const_int32(gallivm, vector_length);

   system_values.tess_outer = LLVMBuildLoad2(builder, tess_outer_deref_type, tess_outer, "");
   system_values.tess_inner = LLVMBuildLoad2(builder, tess_inner_deref_type, tess_inner, "");

   system_values.prim_id = lp_build_broadcast_scalar(&bldvec, prim_id);

   system_values.view_index = view_index;

   system_values.vertices_in = lp_build_broadcast_scalar(&bldvec, patch_vertices_in);

   if (variant->key.primid_needed) {
      int slot = variant->key.primid_output;
      for (unsigned i = 0; i < 4; i++) {
         outputs[slot][i] = lp_build_alloca(gallivm, lp_build_int_vec_type(gallivm, tes_type), "primid");
         LLVMBuildStore(builder, system_values.prim_id, outputs[slot][i]);
      }
      primid_slot = slot;
   }
   struct lp_build_loop_state lp_loop;
   lp_build_loop_begin(&lp_loop, gallivm, bld.zero);
   {
      LLVMValueRef io;

      io = LLVMBuildGEP2(builder, variant->vertex_header_type, io_ptr, &lp_loop.counter, 1, "");
      mask_val = generate_tes_mask_value(variant, tes_type, num_tess_coord, lp_loop.counter);
      lp_build_mask_begin(&mask, gallivm, tes_type, mask_val);

      system_values.tess_coord = LLVMGetUndef(LLVMArrayType(LLVMVectorType(flt_type, vector_length), 3));
      for (i = 0; i < 3; i++) {
         LLVMValueRef tess_coord_chan = LLVMGetUndef(LLVMVectorType(flt_type, vector_length));
         for (unsigned j = 0; j < vector_length; j++) {
            LLVMValueRef idx = LLVMBuildAdd(builder, lp_loop.counter, lp_build_const_int32(gallivm, j), "");
            LLVMValueRef tc_val;
            if (i == 2) {
               if (variant->shader->base.prim_mode == MESA_PRIM_TRIANGLES) {
                  tc_val = lp_build_const_float(gallivm, 1.0);
                  tc_val = LLVMBuildFSub(builder, tc_val, lp_build_pointer_get2(builder, flt_type, tess_coord[0], idx), "");
                  tc_val = LLVMBuildFSub(builder, tc_val, lp_build_pointer_get2(builder, flt_type, tess_coord[1], idx), "");
               } else
                  tc_val = lp_build_const_float(gallivm, 0.0);
            } else
               tc_val = lp_build_pointer_get2(builder, flt_type, tess_coord[i], idx);

            tess_coord_chan = LLVMBuildInsertElement(builder, tess_coord_chan, tc_val, lp_build_const_int32(gallivm, j), "");
         }
         system_values.tess_coord = LLVMBuildInsertValue(builder, system_values.tess_coord, tess_coord_chan, i, "");
      }

      struct lp_build_tgsi_params params;
      memset(&params, 0, sizeof(params));

      params.type = tes_type;
      params.mask = &mask;
      params.consts_ptr = consts_ptr;
      params.system_values = &system_values;
      params.resources_type = variant->resources_type;
      params.resources_ptr = resources_ptr;
      params.sampler = sampler;
      params.info = &llvm->draw->tes.tess_eval_shader->info;
      params.ssbo_ptr = ssbos_ptr;
      params.image = image;
      params.tes_iface = &tes_iface.base;
      params.aniso_filter_table = lp_jit_resources_aniso_filter_table(gallivm, variant->resources_type, resources_ptr);

      lp_build_nir_soa(variant->gallivm,
                       llvm->draw->tes.tess_eval_shader->state.ir.nir,
                       &params,
                       outputs);

      lp_build_mask_end(&mask);

      if (variant->key.clamp_vertex_color) {
         const struct tgsi_shader_info *info = &llvm->draw->tes.tess_eval_shader->info;
         do_clamp_vertex_color(variant->gallivm,
                               tes_type, info,
                               outputs);
      }
      LLVMValueRef clipmask = lp_build_const_int_vec(gallivm,
                                                     lp_int_type(tes_type), 0);

      convert_to_aos(gallivm, variant->vertex_header_type, io, NULL, outputs, clipmask,
                     draw_total_tes_outputs(llvm->draw), tes_type, primid_slot, false);
   }
   lp_build_loop_end_cond(&lp_loop, num_tess_coord, step, LLVMIntUGE);
   lp_bld_llvm_sampler_soa_destroy(sampler);
   lp_bld_llvm_image_soa_destroy(image);

   LLVMBuildRet(builder, lp_build_zero(gallivm, lp_type_uint(32)));
   gallivm_verify_function(gallivm, variant_func);
}


struct draw_tes_llvm_variant *
draw_tes_llvm_create_variant(struct draw_llvm *llvm,
                             unsigned num_outputs,
                             const struct draw_tes_llvm_variant_key *key)
{
   struct draw_tes_llvm_variant *variant;
   struct llvm_tess_eval_shader *shader = llvm_tess_eval_shader(llvm->draw->tes.tess_eval_shader);
   char module_name[64];
   unsigned char ir_sha1_cache_key[20];
   struct lp_cached_code cached = { 0 };
   bool needs_caching = false;

   variant = MALLOC(sizeof *variant +
                    shader->variant_key_size - sizeof variant->key);
   if (!variant)
      return NULL;

   variant->llvm = llvm;
   variant->shader = shader;

   snprintf(module_name, sizeof(module_name), "draw_llvm_tes_variant%u",
            variant->shader->variants_cached);

   memcpy(&variant->key, key, shader->variant_key_size);
   if (shader->base.state.ir.nir && llvm->draw->disk_cache_cookie) {
      draw_get_ir_cache_key(shader->base.state.ir.nir,
                            key,
                            shader->variant_key_size,
                            num_outputs,
                            ir_sha1_cache_key);

      llvm->draw->disk_cache_find_shader(llvm->draw->disk_cache_cookie,
                                         &cached,
                                         ir_sha1_cache_key);
      if (!cached.data_size)
         needs_caching = true;
   }
   variant->gallivm = gallivm_create(module_name, llvm->context, &cached);

   create_tes_jit_types(variant);

   variant->vertex_header_type = lp_build_create_jit_vertex_header_type(variant->gallivm, num_outputs);
   variant->vertex_header_ptr_type = LLVMPointerType(variant->vertex_header_type, 0);

   if (gallivm_debug & (GALLIVM_DEBUG_TGSI | GALLIVM_DEBUG_IR)) {
      nir_print_shader(llvm->draw->tes.tess_eval_shader->state.ir.nir, stderr);
      draw_tes_llvm_dump_variant_key(&variant->key);
   }

   draw_tes_llvm_generate(llvm, variant);

   gallivm_compile_module(variant->gallivm);

   variant->jit_func = (draw_tes_jit_func)
      gallivm_jit_function(variant->gallivm, variant->function);

   if (needs_caching)
      llvm->draw->disk_cache_insert_shader(llvm->draw->disk_cache_cookie,
                                           &cached,
                                           ir_sha1_cache_key);
   gallivm_free_ir(variant->gallivm);

   variant->list_item_global.base = variant;
   variant->list_item_local.base = variant;
   /*variant->no = */shader->variants_created++;
   variant->list_item_global.base = variant;

   return variant;
}


void
draw_tes_llvm_destroy_variant(struct draw_tes_llvm_variant *variant)
{
   struct draw_llvm *llvm = variant->llvm;

   if (gallivm_debug & (GALLIVM_DEBUG_TGSI | GALLIVM_DEBUG_IR)) {
      debug_printf("Deleting TES variant: %u tes variants,\t%u total variants\n",
                    variant->shader->variants_cached, llvm->nr_tes_variants);
   }

   gallivm_destroy(variant->gallivm);

   list_del(&variant->list_item_local.list);
   variant->shader->variants_cached--;
   list_del(&variant->list_item_global.list);
   llvm->nr_tes_variants--;
   FREE(variant);
}


struct draw_tes_llvm_variant_key *
draw_tes_llvm_make_variant_key(struct draw_llvm *llvm, char *store)
{
   struct draw_tes_llvm_variant_key *key;
   struct lp_sampler_static_state *draw_sampler;
   struct lp_image_static_state *draw_image;

   key = (struct draw_tes_llvm_variant_key *)store;

   memset(key, 0, offsetof(struct draw_tes_llvm_variant_key, samplers[0]));

   int primid_output = draw_find_shader_output(llvm->draw, TGSI_SEMANTIC_PRIMID, 0);
   if (primid_output >= 0) {
      key->primid_output = primid_output;
      key->primid_needed = true;
   }

   key->clamp_vertex_color = llvm->draw->rasterizer->clamp_vertex_color &&
      llvm->draw->gs.geometry_shader == NULL;

   /* All variants of this shader will have the same value for
    * nr_samplers.  Not yet trying to compact away holes in the
    * sampler array.
    */
   key->nr_samplers = llvm->draw->tes.tess_eval_shader->info.file_max[TGSI_FILE_SAMPLER] + 1;
   if (llvm->draw->tes.tess_eval_shader->info.file_max[TGSI_FILE_SAMPLER_VIEW] != -1) {
      key->nr_sampler_views =
         llvm->draw->tes.tess_eval_shader->info.file_max[TGSI_FILE_SAMPLER_VIEW] + 1;
   } else {
      key->nr_sampler_views = key->nr_samplers;
   }

   key->nr_images = llvm->draw->tes.tess_eval_shader->info.file_max[TGSI_FILE_IMAGE] + 1;

   draw_sampler = key->samplers;

   memset(draw_sampler, 0, MAX2(key->nr_samplers, key->nr_sampler_views) * sizeof *draw_sampler);

   for (unsigned i = 0 ; i < key->nr_samplers; i++) {
      lp_sampler_static_sampler_state(&draw_sampler[i].sampler_state,
                                      llvm->draw->samplers[PIPE_SHADER_TESS_EVAL][i]);
   }
   for (unsigned i = 0 ; i < key->nr_sampler_views; i++) {
      lp_sampler_static_texture_state(&draw_sampler[i].texture_state,
                                      llvm->draw->sampler_views[PIPE_SHADER_TESS_EVAL][i]);
   }

   draw_image = draw_tes_llvm_variant_key_images(key);
   memset(draw_image, 0,
          key->nr_images * sizeof *draw_image);
   for (unsigned i = 0; i < key->nr_images; i++) {
      lp_sampler_static_texture_state_image(&draw_image[i].image_state,
                                            llvm->draw->images[PIPE_SHADER_TESS_EVAL][i]);
   }
   return key;
}


void
draw_tes_llvm_dump_variant_key(struct draw_tes_llvm_variant_key *key)
{
   struct lp_sampler_static_state *sampler = key->samplers;
   struct lp_image_static_state *image = draw_tes_llvm_variant_key_images(key);

   if (key->primid_needed)
      debug_printf("prim id output %d\n", key->primid_output);
   debug_printf("clamp_vertex_color = %u\n", key->clamp_vertex_color);
   for (unsigned i = 0 ; i < key->nr_sampler_views; i++) {
      debug_printf("sampler[%i].src_format = %s\n", i,
                   util_format_name(sampler[i].texture_state.format));
   }

   for (unsigned i = 0 ; i < key->nr_images; i++)
      debug_printf("images[%i].format = %s\n", i, util_format_name(image[i].image_state.format));

}
