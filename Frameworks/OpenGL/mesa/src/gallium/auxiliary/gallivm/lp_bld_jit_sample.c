/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * All rights reserved.
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

#include "gallivm/lp_bld_sample.h"
#include "gallivm/lp_bld_limits.h"
#include "gallivm/lp_bld_tgsi.h"
#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_sample.h"
#include "gallivm/lp_bld_jit_types.h"
#include "gallivm/lp_bld_jit_sample.h"
#include "gallivm/lp_bld_flow.h"

struct lp_bld_sampler_dynamic_state
{
   struct lp_sampler_dynamic_state base;

   const struct lp_sampler_static_state *static_state;
};

struct lp_bld_llvm_sampler_soa
{
   struct lp_build_sampler_soa base;

   struct lp_bld_sampler_dynamic_state dynamic_state;
   unsigned nr_samplers;
};


struct lp_bld_image_dynamic_state
{
   struct lp_sampler_dynamic_state base;

   const struct lp_image_static_state *static_state;
};

struct lp_bld_llvm_image_soa
{
   struct lp_build_image_soa base;

   struct lp_bld_image_dynamic_state dynamic_state;
   unsigned nr_images;
};

static LLVMValueRef
load_texture_functions_ptr(struct gallivm_state *gallivm, LLVMValueRef descriptor,
                           uint32_t offset1, uint32_t offset2)
{
   LLVMBuilderRef builder = gallivm->builder;

   LLVMValueRef texture_base_offset = lp_build_const_int64(gallivm, offset1);
   LLVMValueRef texture_base_ptr = LLVMBuildAdd(builder, descriptor, texture_base_offset, "");

   LLVMTypeRef texture_base_type = LLVMInt64TypeInContext(gallivm->context);
   LLVMTypeRef texture_base_ptr_type = LLVMPointerType(texture_base_type, 0);

   texture_base_ptr = LLVMBuildIntToPtr(builder, texture_base_ptr, texture_base_ptr_type, "");
   /* struct lp_texture_functions * */
   LLVMValueRef texture_base = LLVMBuildLoad2(builder, texture_base_type, texture_base_ptr, "");

   LLVMValueRef functions_offset = lp_build_const_int64(gallivm, offset2);
   return LLVMBuildAdd(builder, texture_base, functions_offset, "");
}

static LLVMValueRef
widen_to_simd_width(struct gallivm_state *gallivm, LLVMValueRef value)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMTypeRef type = LLVMTypeOf(value);

   if (LLVMGetTypeKind(type) == LLVMVectorTypeKind) {
      LLVMTypeRef element_type = LLVMGetElementType(type);
      uint32_t element_count = LLVMGetVectorSize(type);
      LLVMValueRef elements[8] = { 0 };
      for (uint32_t i = 0; i < lp_native_vector_width / 32; i++) {
         if (i < element_count)
            elements[i] = LLVMBuildExtractElement(builder, value, lp_build_const_int32(gallivm, i), "");
         else
            elements[i] = LLVMConstNull(element_type);
      }

      LLVMTypeRef result_type = LLVMVectorType(element_type, lp_native_vector_width / 32);
      LLVMValueRef result = LLVMGetUndef(result_type);
      for (unsigned i = 0; i < lp_native_vector_width / 32; i++)
         result = LLVMBuildInsertElement(builder, result, elements[i], lp_build_const_int32(gallivm, i), "");

      return result;
   }

   return value;
}

static LLVMValueRef
truncate_to_type_width(struct gallivm_state *gallivm, LLVMValueRef value, struct lp_type target_type)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMTypeRef type = LLVMTypeOf(value);

   if (LLVMGetTypeKind(type) == LLVMVectorTypeKind) {
      LLVMTypeRef element_type = LLVMGetElementType(type);

      LLVMValueRef elements[8];
      for (uint32_t i = 0; i < target_type.length; i++)
         elements[i] = LLVMBuildExtractElement(builder, value, lp_build_const_int32(gallivm, i), "");

      LLVMTypeRef result_type = LLVMVectorType(element_type, target_type.length);
      LLVMValueRef result = LLVMGetUndef(result_type);
      for (unsigned i = 0; i < target_type.length; i++)
         result = LLVMBuildInsertElement(builder, result, elements[i], lp_build_const_int32(gallivm, i), "");

      return result;
   }

   return value;
}

/**
 * Fetch filtered values from texture.
 * The 'texel' parameter returns four vectors corresponding to R, G, B, A.
 */
static void
lp_bld_llvm_sampler_soa_emit_fetch_texel(const struct lp_build_sampler_soa *base,
                                         struct gallivm_state *gallivm,
                                         const struct lp_sampler_params *params)
{
   struct lp_bld_llvm_sampler_soa *sampler = (struct lp_bld_llvm_sampler_soa *)base;
   LLVMBuilderRef builder = gallivm->builder;

   if (params->texture_resource) {
      LLVMTypeRef out_data_type = lp_build_vec_type(gallivm, params->type);

      LLVMValueRef out_data[4];
      for (uint32_t i = 0; i < 4; i++) {
         out_data[i] = lp_build_alloca(gallivm, out_data_type, "");
         LLVMBuildStore(builder, lp_build_const_vec(gallivm, params->type, 0), out_data[i]);
      }

      struct lp_type uint_type = lp_uint_type(params->type);
      LLVMValueRef uint_zero = lp_build_const_int_vec(gallivm, uint_type, 0);

      LLVMValueRef bitmask = LLVMBuildICmp(builder, LLVMIntNE, params->exec_mask, uint_zero, "exec_bitvec");

      LLVMTypeRef bitmask_type = LLVMIntTypeInContext(gallivm->context, uint_type.length);
      bitmask = LLVMBuildBitCast(builder, bitmask, bitmask_type, "exec_bitmask");

      LLVMValueRef any_active = LLVMBuildICmp(builder, LLVMIntNE, bitmask, LLVMConstInt(bitmask_type, 0, false), "any_active");

      struct lp_build_if_state if_state;
      lp_build_if(&if_state, gallivm, any_active);

      LLVMValueRef consts = lp_jit_resources_constants(gallivm, params->resources_type, params->resources_ptr);

      LLVMValueRef texture_descriptor = lp_llvm_descriptor_base(gallivm, consts, params->texture_resource, LP_MAX_TGSI_CONST_BUFFERS);

      enum lp_sampler_op_type op_type = (params->sample_key & LP_SAMPLER_OP_TYPE_MASK) >> LP_SAMPLER_OP_TYPE_SHIFT;
      uint32_t functions_offset = op_type == LP_SAMPLER_OP_FETCH ? offsetof(struct lp_texture_functions, fetch_functions)
                                                                 : offsetof(struct lp_texture_functions, sample_functions);

      LLVMValueRef texture_base_ptr = load_texture_functions_ptr(
         gallivm, texture_descriptor, offsetof(struct lp_descriptor, functions), functions_offset);

      LLVMTypeRef texture_function_type = lp_build_sample_function_type(gallivm, params->sample_key);
      LLVMTypeRef texture_function_ptr_type = LLVMPointerType(texture_function_type, 0);
      LLVMTypeRef texture_functions_type = LLVMPointerType(texture_function_ptr_type, 0);
      LLVMTypeRef texture_base_type = LLVMPointerType(texture_functions_type, 0);
      LLVMTypeRef texture_base_ptr_type = LLVMPointerType(texture_base_type, 0);

      texture_base_ptr = LLVMBuildIntToPtr(builder, texture_base_ptr, texture_base_ptr_type, "");
      LLVMValueRef texture_base = LLVMBuildLoad2(builder, texture_base_type, texture_base_ptr, "");

      LLVMValueRef texture_functions;
      LLVMValueRef sampler_desc_ptr;
      if (op_type == LP_SAMPLER_OP_FETCH) {
         texture_functions = texture_base;
         sampler_desc_ptr = LLVMGetUndef(LLVMInt64TypeInContext(gallivm->context));
      } else {
         sampler_desc_ptr = lp_llvm_descriptor_base(gallivm, consts, params->sampler_resource, LP_MAX_TGSI_CONST_BUFFERS);

         LLVMValueRef sampler_index_offset = lp_build_const_int64(gallivm, offsetof(struct lp_descriptor, texture.sampler_index));
         LLVMValueRef sampler_index_ptr = LLVMBuildAdd(builder, sampler_desc_ptr, sampler_index_offset, "");

         LLVMTypeRef sampler_index_type = LLVMInt32TypeInContext(gallivm->context);
         LLVMTypeRef sampler_index_ptr_type = LLVMPointerType(sampler_index_type, 0);

         sampler_index_ptr = LLVMBuildIntToPtr(builder, sampler_index_ptr, sampler_index_ptr_type, "");
         LLVMValueRef sampler_index = LLVMBuildLoad2(builder, sampler_index_type, sampler_index_ptr, "");

         LLVMValueRef texture_functions_ptr = LLVMBuildGEP2(builder, texture_functions_type, texture_base, &sampler_index, 1, "");
         texture_functions = LLVMBuildLoad2(builder, texture_functions_type, texture_functions_ptr, "");
      }

      LLVMValueRef sample_key = lp_build_const_int32(gallivm, params->sample_key);
      LLVMValueRef texture_function_ptr = LLVMBuildGEP2(builder, texture_function_ptr_type, texture_functions, &sample_key, 1, "");
      LLVMValueRef texture_function = LLVMBuildLoad2(builder, texture_function_ptr_type, texture_function_ptr, "");

      LLVMValueRef args[LP_MAX_TEX_FUNC_ARGS];
      uint32_t num_args = 0;

      args[num_args++] = texture_descriptor;
      args[num_args++] = sampler_desc_ptr;

      args[num_args++] = params->aniso_filter_table;

      LLVMTypeRef coord_type;
      if (op_type == LP_SAMPLER_OP_FETCH)
         coord_type = lp_build_int_vec_type(gallivm, params->type);
      else
         coord_type = lp_build_vec_type(gallivm, params->type);

      for (uint32_t i = 0; i < 4; i++) {
         if (LLVMIsUndef(params->coords[i]))
            args[num_args++] = LLVMGetUndef(coord_type);
         else
            args[num_args++] = params->coords[i];
      }

      if (params->sample_key & LP_SAMPLER_SHADOW)
         args[num_args++] = params->coords[4];

      if (params->sample_key & LP_SAMPLER_FETCH_MS)
         args[num_args++] = params->ms_index;

      if (params->sample_key & LP_SAMPLER_OFFSETS) {
         for (uint32_t i = 0; i < 3; i++) {
            if (params->offsets[i])
               args[num_args++] = params->offsets[i];
            else
               args[num_args++] = LLVMGetUndef(lp_build_int_vec_type(gallivm, params->type));
         }
      }

      enum lp_sampler_lod_control lod_control = (params->sample_key & LP_SAMPLER_LOD_CONTROL_MASK) >> LP_SAMPLER_LOD_CONTROL_SHIFT;
      if (lod_control == LP_SAMPLER_LOD_BIAS || lod_control == LP_SAMPLER_LOD_EXPLICIT)
         args[num_args++] = params->lod;

      if (params->type.length != lp_native_vector_width / 32)
         for (uint32_t i = 0; i < num_args; i++)
            args[i] = widen_to_simd_width(gallivm, args[i]);

      LLVMValueRef result = LLVMBuildCall2(builder, texture_function_type, texture_function, args, num_args, "");

      for (unsigned i = 0; i < 4; i++) {
         params->texel[i] = LLVMBuildExtractValue(gallivm->builder, result, i, "");

         if (params->type.length != lp_native_vector_width / 32)
            params->texel[i] = truncate_to_type_width(gallivm, params->texel[i], params->type);

         LLVMBuildStore(builder, params->texel[i], out_data[i]);
      }

      lp_build_endif(&if_state);

      for (unsigned i = 0; i < 4; i++)
         params->texel[i] = LLVMBuildLoad2(gallivm->builder, out_data_type, out_data[i], "");

      return;
   }

   const unsigned texture_index = params->texture_index;
   const unsigned sampler_index = params->sampler_index;

   assert(sampler_index < PIPE_MAX_SAMPLERS);
   assert(texture_index < PIPE_MAX_SHADER_SAMPLER_VIEWS);
#if 0
   if (LP_PERF & PERF_NO_TEX) {
      lp_build_sample_nop(gallivm, params->type, params->coords, params->texel);
      return;
   }
#endif

   if (params->texture_index_offset) {
      LLVMValueRef unit =
         LLVMBuildAdd(gallivm->builder, params->texture_index_offset,
                      lp_build_const_int32(gallivm, texture_index), "");

      struct lp_build_sample_array_switch switch_info;
      memset(&switch_info, 0, sizeof(switch_info));
      lp_build_sample_array_init_soa(&switch_info, gallivm, params, unit,
                                     0, sampler->nr_samplers);
      // build the switch cases
      for (unsigned i = 0; i < sampler->nr_samplers; i++) {
         lp_build_sample_array_case_soa(&switch_info, i,
                                        &sampler->dynamic_state.static_state[i].texture_state,
                                        &sampler->dynamic_state.static_state[i].sampler_state,
                                        &sampler->dynamic_state.base);
      }
      lp_build_sample_array_fini_soa(&switch_info);
   } else {
      lp_build_sample_soa(&sampler->dynamic_state.static_state[texture_index].texture_state,
                          &sampler->dynamic_state.static_state[sampler_index].sampler_state,
                          &sampler->dynamic_state.base,
                          gallivm, params);
   }
}


/**
 * Fetch the texture size.
 */
static void
lp_bld_llvm_sampler_soa_emit_size_query(const struct lp_build_sampler_soa *base,
                                        struct gallivm_state *gallivm,
                                        const struct lp_sampler_size_query_params *params)
{
   LLVMBuilderRef builder = gallivm->builder;

   if (params->resource) {
      LLVMTypeRef out_data_type = lp_build_vec_type(gallivm, params->int_type);

      LLVMValueRef out_data[4];
      for (uint32_t i = 0; i < 4; i++) {
         out_data[i] = lp_build_alloca(gallivm, out_data_type, "");
         LLVMBuildStore(builder, lp_build_const_vec(gallivm, params->int_type, 0), out_data[i]);
      }

      struct lp_type uint_type = lp_uint_type(params->int_type);
      LLVMValueRef uint_zero = lp_build_const_int_vec(gallivm, uint_type, 0);

      LLVMValueRef bitmask = LLVMBuildICmp(builder, LLVMIntNE, params->exec_mask, uint_zero, "exec_bitvec");

      LLVMTypeRef bitmask_type = LLVMIntTypeInContext(gallivm->context, uint_type.length);
      bitmask = LLVMBuildBitCast(builder, bitmask, bitmask_type, "exec_bitmask");

      LLVMValueRef any_active = LLVMBuildICmp(builder, LLVMIntNE, bitmask, LLVMConstInt(bitmask_type, 0, false), "any_active");

      struct lp_build_if_state if_state;
      lp_build_if(&if_state, gallivm, any_active);

      LLVMValueRef consts = lp_jit_resources_constants(gallivm, params->resources_type, params->resources_ptr);

      LLVMValueRef texture_descriptor = lp_llvm_descriptor_base(gallivm, consts, params->resource, LP_MAX_TGSI_CONST_BUFFERS);

      uint32_t functions_offset = params->samples_only ? offsetof(struct lp_texture_functions, samples_function)
                                                       : offsetof(struct lp_texture_functions, size_function);

      LLVMValueRef texture_base_ptr = load_texture_functions_ptr(
         gallivm, texture_descriptor, offsetof(struct lp_descriptor, functions), functions_offset);

      LLVMTypeRef texture_function_type = lp_build_size_function_type(gallivm, params);
      LLVMTypeRef texture_function_ptr_type = LLVMPointerType(texture_function_type, 0);
      LLVMTypeRef texture_function_ptr_ptr_type = LLVMPointerType(texture_function_ptr_type, 0);

      texture_base_ptr = LLVMBuildIntToPtr(builder, texture_base_ptr, texture_function_ptr_ptr_type, "");
      LLVMValueRef texture_function = LLVMBuildLoad2(builder, texture_function_ptr_type, texture_base_ptr, "");

      LLVMValueRef args[LP_MAX_TEX_FUNC_ARGS];
      uint32_t num_args = 0;

      args[num_args++] = texture_descriptor;

      if (!params->samples_only)
         args[num_args++] = params->explicit_lod;

      if (params->int_type.length != lp_native_vector_width / 32)
         for (uint32_t i = 0; i < num_args; i++)
            args[i] = widen_to_simd_width(gallivm, args[i]);

      LLVMValueRef result = LLVMBuildCall2(builder, texture_function_type, texture_function, args, num_args, "");

      for (unsigned i = 0; i < 4; i++) {
         params->sizes_out[i] = LLVMBuildExtractValue(gallivm->builder, result, i, "");

         if (params->int_type.length != lp_native_vector_width / 32)
            params->sizes_out[i] = truncate_to_type_width(gallivm, params->sizes_out[i], params->int_type);

         LLVMBuildStore(builder, params->sizes_out[i], out_data[i]);
      }

      lp_build_endif(&if_state);

      for (unsigned i = 0; i < 4; i++)
         params->sizes_out[i] = LLVMBuildLoad2(gallivm->builder, out_data_type, out_data[i], "");

      return;
   }

   struct lp_bld_llvm_sampler_soa *sampler = (struct lp_bld_llvm_sampler_soa *)base;

   assert(params->texture_unit < PIPE_MAX_SHADER_SAMPLER_VIEWS);

   lp_build_size_query_soa(gallivm,
                           &sampler->dynamic_state.static_state[params->texture_unit].texture_state,
                           &sampler->dynamic_state.base,
                           params);
}


struct lp_build_sampler_soa *
lp_bld_llvm_sampler_soa_create(const struct lp_sampler_static_state *static_state,
                               unsigned nr_samplers)
{
   assert(static_state);

   struct lp_bld_llvm_sampler_soa *sampler = CALLOC_STRUCT(lp_bld_llvm_sampler_soa);
   if (!sampler)
      return NULL;

   sampler->base.emit_tex_sample = lp_bld_llvm_sampler_soa_emit_fetch_texel;
   sampler->base.emit_size_query = lp_bld_llvm_sampler_soa_emit_size_query;

   lp_build_jit_fill_sampler_dynamic_state(&sampler->dynamic_state.base);

   sampler->dynamic_state.static_state = static_state;

   sampler->nr_samplers = nr_samplers;
   return &sampler->base;
}


static void
lp_bld_llvm_image_soa_emit_op(const struct lp_build_image_soa *base,
                              struct gallivm_state *gallivm,
                              const struct lp_img_params *params)
{
   LLVMBuilderRef builder = gallivm->builder;

   if (params->resource) {
      const struct util_format_description *desc = util_format_description(params->format);
      LLVMTypeRef out_data_type = lp_build_vec_type(gallivm, lp_build_texel_type(params->type, desc));

      LLVMValueRef out_data[4];
      for (uint32_t i = 0; i < 4; i++) {
         out_data[i] = lp_build_alloca(gallivm, out_data_type, "");
         LLVMBuildStore(builder, lp_build_const_vec(gallivm, lp_build_texel_type(params->type, desc), 0), out_data[i]);
      }

      struct lp_type uint_type = lp_uint_type(params->type);
      LLVMValueRef uint_zero = lp_build_const_int_vec(gallivm, uint_type, 0);

      LLVMValueRef bitmask = LLVMBuildICmp(builder, LLVMIntNE, params->exec_mask, uint_zero, "exec_bitvec");

      LLVMTypeRef bitmask_type = LLVMIntTypeInContext(gallivm->context, uint_type.length);
      bitmask = LLVMBuildBitCast(builder, bitmask, bitmask_type, "exec_bitmask");

      LLVMValueRef any_active = LLVMBuildICmp(builder, LLVMIntNE, bitmask, LLVMConstInt(bitmask_type, 0, false), "any_active");

      LLVMValueRef binding_index = LLVMBuildExtractValue(builder, params->resource, 1, "");
      LLVMValueRef inbounds = LLVMBuildICmp(builder, LLVMIntSGE, binding_index, lp_build_const_int32(gallivm, 0), "inbounds");

      struct lp_build_if_state if_state;
      lp_build_if(&if_state, gallivm, LLVMBuildAnd(builder, any_active, inbounds, ""));

      LLVMValueRef consts = lp_jit_resources_constants(gallivm, params->resources_type, params->resources_ptr);

      LLVMValueRef image_descriptor = lp_llvm_descriptor_base(gallivm, consts, params->resource, LP_MAX_TGSI_CONST_BUFFERS);

      LLVMValueRef image_base_ptr = load_texture_functions_ptr(
         gallivm, image_descriptor, offsetof(struct lp_descriptor, functions),
         offsetof(struct lp_texture_functions, image_functions));

      LLVMTypeRef image_function_type = lp_build_image_function_type(gallivm, params, params->ms_index);
      LLVMTypeRef image_function_ptr_type = LLVMPointerType(image_function_type, 0);
      LLVMTypeRef image_functions_type = LLVMPointerType(image_function_ptr_type, 0);
      LLVMTypeRef image_base_type = LLVMPointerType(image_functions_type, 0);

      image_base_ptr = LLVMBuildIntToPtr(builder, image_base_ptr, image_base_type, "");
      LLVMValueRef image_functions = LLVMBuildLoad2(builder, image_functions_type, image_base_ptr, "");

      uint32_t op = params->img_op;
      if (op == LP_IMG_ATOMIC_CAS)
         op--;
      else if (op == LP_IMG_ATOMIC)
         op = params->op + (LP_IMG_OP_COUNT - 1);

      if (params->ms_index)
         op += LP_TOTAL_IMAGE_OP_COUNT / 2;

      LLVMValueRef function_index = lp_build_const_int32(gallivm, op);

      LLVMValueRef image_function_ptr = LLVMBuildGEP2(builder, image_function_ptr_type, image_functions, &function_index, 1, "");
      LLVMValueRef image_function = LLVMBuildLoad2(builder, image_function_ptr_type, image_function_ptr, "");

      LLVMValueRef args[LP_MAX_TEX_FUNC_ARGS] = { 0 };
      uint32_t num_args = 0;

      args[num_args++] = image_descriptor;

      if (params->img_op != LP_IMG_LOAD)
         args[num_args++] = params->exec_mask;

      for (uint32_t i = 0; i < 3; i++)
         args[num_args++] = params->coords[i];

      if (params->ms_index)
         args[num_args++] = params->ms_index;

      if (params->img_op != LP_IMG_LOAD)
         for (uint32_t i = 0; i < 4; i++)
            args[num_args++] = params->indata[i];

      if (params->img_op == LP_IMG_ATOMIC_CAS)
         for (uint32_t i = 0; i < 4; i++)
            args[num_args++] = params->indata2[i];

      assert(num_args == LLVMCountParamTypes(image_function_type));

      LLVMTypeRef param_types[LP_MAX_TEX_FUNC_ARGS];
      LLVMGetParamTypes(image_function_type, param_types);
      for (uint32_t i = 0; i < num_args; i++)
         if (!args[i])
            args[i] = LLVMGetUndef(param_types[i]);

      if (params->type.length != lp_native_vector_width / 32)
         for (uint32_t i = 0; i < num_args; i++)
            args[i] = widen_to_simd_width(gallivm, args[i]);

      LLVMValueRef result = LLVMBuildCall2(builder, image_function_type, image_function, args, num_args, "");

      if (params->img_op != LP_IMG_STORE) {
         for (unsigned i = 0; i < 4; i++) {
            LLVMValueRef channel = LLVMBuildExtractValue(gallivm->builder, result, i, "");
            if (params->type.length != lp_native_vector_width / 32)
               channel = truncate_to_type_width(gallivm, channel, params->type);

            LLVMBuildStore(builder, channel, out_data[i]);
         }
      }

      lp_build_endif(&if_state);

      if (params->img_op != LP_IMG_STORE) {
         for (unsigned i = 0; i < 4; i++) {
            params->outdata[i] = LLVMBuildLoad2(gallivm->builder, out_data_type, out_data[i], "");
         }
      }

      return;
   }

   struct lp_bld_llvm_image_soa *image = (struct lp_bld_llvm_image_soa *)base;
   const unsigned image_index = params->image_index;
   assert(image_index < PIPE_MAX_SHADER_IMAGES);

   if (params->image_index_offset) {
      struct lp_build_img_op_array_switch switch_info;
      memset(&switch_info, 0, sizeof(switch_info));
      LLVMValueRef unit = LLVMBuildAdd(gallivm->builder,
                                       params->image_index_offset,
                                       lp_build_const_int32(gallivm,
                                                            image_index), "");

      lp_build_image_op_switch_soa(&switch_info, gallivm, params,
                                   unit, 0, image->nr_images);

      for (unsigned i = 0; i < image->nr_images; i++) {
         lp_build_image_op_array_case(&switch_info, i,
                                      &image->dynamic_state.static_state[i].image_state,
                                      &image->dynamic_state.base);
      }
      lp_build_image_op_array_fini_soa(&switch_info);
   } else {
      lp_build_img_op_soa(&image->dynamic_state.static_state[image_index].image_state,
                          &image->dynamic_state.base,
                          gallivm, params, params->outdata);
   }
}


/**
 * Fetch the texture size.
 */
static void
lp_bld_llvm_image_soa_emit_size_query(const struct lp_build_image_soa *base,
                                      struct gallivm_state *gallivm,
                                      const struct lp_sampler_size_query_params *params)
{
   struct lp_bld_llvm_image_soa *image = (struct lp_bld_llvm_image_soa *)base;

   if (params->resource) {
      LLVMValueRef old_texture = gallivm->texture_descriptor;

      LLVMValueRef consts = lp_jit_resources_constants(gallivm, params->resources_type, params->resources_ptr);
      gallivm->texture_descriptor = lp_llvm_descriptor_base(gallivm, consts, params->resource, LP_MAX_TGSI_CONST_BUFFERS);

      enum pipe_format format = params->format;
      if (format == PIPE_FORMAT_NONE)
         format = PIPE_FORMAT_R8G8B8A8_UNORM;

      struct lp_static_texture_state state = {
         .format = format,
         .res_format = format,
         .target = params->target,
         .level_zero_only = params->ms,
      };
      
      lp_build_size_query_soa(gallivm, &state, &image->dynamic_state.base, params);

      gallivm->texture_descriptor = old_texture;

      return;
   }

   assert(params->texture_unit < PIPE_MAX_SHADER_IMAGES);

   lp_build_size_query_soa(gallivm,
                           &image->dynamic_state.static_state[params->texture_unit].image_state,
                           &image->dynamic_state.base,
                           params);
}


struct lp_build_image_soa *
lp_bld_llvm_image_soa_create(const struct lp_image_static_state *static_state,
                             unsigned nr_images)
{
   struct lp_bld_llvm_image_soa *image = CALLOC_STRUCT(lp_bld_llvm_image_soa);
   if (!image)
      return NULL;

   image->base.emit_op = lp_bld_llvm_image_soa_emit_op;
   image->base.emit_size_query = lp_bld_llvm_image_soa_emit_size_query;

   lp_build_jit_fill_image_dynamic_state(&image->dynamic_state.base);
   image->dynamic_state.static_state = static_state;

   image->nr_images = nr_images;
   return &image->base;
}

struct lp_sampler_dynamic_state *
lp_build_sampler_soa_dynamic_state(struct lp_build_sampler_soa *_sampler)
{
   struct lp_bld_llvm_sampler_soa *sampler = (struct lp_bld_llvm_sampler_soa *)_sampler;
   return &sampler->dynamic_state.base;
}

struct lp_sampler_dynamic_state *
lp_build_image_soa_dynamic_state(struct lp_build_image_soa *_image)
{
   struct lp_bld_llvm_image_soa *image = (struct lp_bld_llvm_image_soa *)_image;
   return &image->dynamic_state.base;
}
