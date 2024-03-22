/**************************************************************************
 *
 * Copyright 2010-2021 VMware, Inc.
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

#include <limits.h>

#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_pointer.h"
#include "util/format/u_format.h"
#include "util/u_dump.h"
#include "util/u_string.h"
#include "util/os_time.h"
#include "pipe/p_shader_tokens.h"
#include "draw/draw_context.h"
#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_conv.h"
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_intr.h"
#include "gallivm/lp_bld_logic.h"
#include "gallivm/lp_bld_tgsi.h"
#include "gallivm/lp_bld_swizzle.h"
#include "gallivm/lp_bld_flow.h"
#include "gallivm/lp_bld_printf.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_nir.h"

#include "lp_bld_alpha.h"
#include "lp_bld_blend.h"
#include "lp_bld_depth.h"
#include "lp_bld_interp.h"
#include "lp_context.h"
#include "lp_debug.h"
#include "lp_perf.h"
#include "lp_screen.h"
#include "lp_setup.h"
#include "lp_state.h"
#include "lp_tex_sample.h"
#include "lp_flush.h"
#include "lp_state_fs.h"


/**
 * Sampler.
 */
struct linear_sampler
{
   struct lp_build_sampler_aos base;
   LLVMValueRef texels_ptrs[LP_MAX_LINEAR_TEXTURES];
   LLVMValueRef counter;
   unsigned instance;
};


/**
 * Provide texels to the TGSI translation.
 *
 * We don't actually do any texture sampling here, but simply hand the
 * precomputed row of texels.
 */
static LLVMValueRef
emit_fetch_texel_linear(const struct lp_build_sampler_aos *base,
                        struct lp_build_context *bld,
                        enum tgsi_texture_type target,
                        unsigned unit,
                        LLVMValueRef coords,
                        const struct lp_derivatives derivs,
                        enum lp_build_tex_modifier modifier)
{
   struct linear_sampler *sampler = (struct linear_sampler *)base;

   if (sampler->instance >= LP_MAX_LINEAR_TEXTURES) {
      assert(false);
      return bld->undef;
   }

   /* Pointer to a row of texels */
   LLVMValueRef texels_ptr = sampler->texels_ptrs[sampler->instance];

   LLVMValueRef texel = lp_build_pointer_get2(bld->gallivm->builder,
                                              bld->vec_type,
                                              texels_ptr, sampler->counter);
   assert(LLVMTypeOf(texel) == bld->vec_type);

   /*
    * We have a struct lp_linear_sampler instance per TEX instruction,
    * _not_ per unit, as each TEX instruction will need separate storage
    * for the texels.
    */
   (void)unit;
   ++sampler->instance;

   return texel;
}


/**
 * Generates the main body of the fragment shader
 * Supports generating code for 4 pixel blocks and individual pixels
 */
static LLVMValueRef
llvm_fragment_body(struct lp_build_context *bld,
                   struct lp_fragment_shader *shader,
                   struct lp_fragment_shader_variant *variant,
                   struct linear_sampler* sampler,
                   LLVMValueRef *inputs_ptrs,
                   LLVMValueRef consts_ptr,
                   LLVMValueRef blend_color,
                   LLVMValueRef alpha_ref,
                   struct lp_type fs_type,
                   LLVMValueRef dst)
{
   static const unsigned char bgra_swizzles[4] = {2, 1, 0, 3};
   static const unsigned char rgba_swizzles[4] = {0, 1, 2, 3};
   LLVMValueRef inputs[PIPE_MAX_SHADER_INPUTS];
   LLVMValueRef outputs[PIPE_MAX_SHADER_OUTPUTS];
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMValueRef result = NULL;
   bool rgba_order = (variant->key.cbuf_format[0] == PIPE_FORMAT_R8G8B8A8_UNORM ||
                      variant->key.cbuf_format[0] == PIPE_FORMAT_R8G8B8X8_UNORM);
   struct nir_shader *nir = shader->base.ir.nir;
   sampler->instance = 0;

   /*
    * Advance inputs
    */
   unsigned i;
   for (i = 0; i < util_bitcount64(nir->info.inputs_read); ++i) {
      inputs[i] =
         lp_build_pointer_get2(builder, bld->vec_type, inputs_ptrs[i], sampler->counter);
      assert(LLVMTypeOf(inputs[i]) == bld->vec_type);
   }
   for ( ; i < PIPE_MAX_SHADER_INPUTS; ++i) {
      inputs[i] = bld->undef;
   }

   for (i = 0; i < PIPE_MAX_SHADER_OUTPUTS; ++i) {
      outputs[i] = bld->undef;
   }

   nir_shader *clone = nir_shader_clone(NULL, nir);
   lp_build_nir_aos(gallivm, clone, fs_type,
                    rgba_order ? rgba_swizzles : bgra_swizzles,
                    consts_ptr, inputs, outputs,
                    &sampler->base);
   ralloc_free(clone);

   /*
    * Blend output color
    */
   nir_foreach_shader_out_variable(var, nir) {
      unsigned slots = nir_variable_count_slots(var, var->type);

      for (unsigned s = 0; s < slots; s++) {
         unsigned idx = var->data.driver_location + s;
         if (!outputs[idx])
            continue;

         LLVMValueRef output = LLVMBuildLoad2(builder, bld->vec_type, outputs[idx], "");
         lp_build_name(output, "output%u", i);

         unsigned cbuf = var->data.location - FRAG_RESULT_DATA0 + s;
         lp_build_name(output, "cbuf%u", cbuf);

         if (var->data.location < FRAG_RESULT_DATA0 || s > 0)
            continue;

         /* Perform alpha test if necessary */
         LLVMValueRef mask = NULL;
         if (variant->key.alpha.enabled) {
            LLVMTypeRef vec_type = lp_build_vec_type(gallivm, fs_type);
            LLVMValueRef broadcast_alpha = lp_build_broadcast(gallivm, vec_type,
                                                              alpha_ref);

            mask = lp_build_cmp(bld, variant->key.alpha.func, output,
                                broadcast_alpha);
            /* XXX is 4 correct? */
            mask = lp_build_swizzle_scalar_aos(bld, mask, bgra_swizzles[3], 4);

            lp_build_name(mask, "alpha_test_mask");
         }

         LLVMValueRef src1 = lp_build_zero(gallivm, fs_type);

         result = lp_build_blend_aos(gallivm,
                                     &variant->key.blend,
                                     variant->key.cbuf_format[idx],
                                     fs_type,
                                     cbuf,   /* rt */
                                     output, /* src */
                                     NULL,   /* src_alpha */
                                     src1,   /* src1 */
                                     NULL,   /* src1_alpha */
                                     dst,
                                     mask,
                                     blend_color,  /* const_ */
                                     NULL,         /* const_alpha */
                                     rgba_order ? rgba_swizzles : bgra_swizzles,
                                     4);
      }
   }

   return result;
}


/**
 * Generate a function that executes the fragment shader in a linear fashion.
 * The shader operates on unorm8[16] vectors.
 * See lp_state_fs_analysis for the "linear" conditions.
 */
void
llvmpipe_fs_variant_linear_llvm(struct llvmpipe_context *lp,
                                struct lp_fragment_shader *shader,
                                struct lp_fragment_shader_variant *variant)
{
   assert(shader->kind == LP_FS_KIND_BLIT_RGBA ||
          shader->kind == LP_FS_KIND_BLIT_RGB1 ||
          shader->kind == LP_FS_KIND_LLVM_LINEAR);

   struct nir_shader *nir = shader->base.ir.nir;
   struct gallivm_state *gallivm = variant->gallivm;
   LLVMTypeRef int8t = LLVMInt8TypeInContext(gallivm->context);
   LLVMTypeRef int32t = LLVMInt32TypeInContext(gallivm->context);
   LLVMTypeRef pint8t = LLVMPointerType(int8t, 0);
   LLVMTypeRef pixelt = LLVMVectorType(int32t, 4);

   // unorm8[16] vector type
   struct lp_type fs_type;
   memset(&fs_type, 0, sizeof fs_type);
   fs_type.floating = false;
   fs_type.sign = false;
   fs_type.norm = true;
   fs_type.width = 8;
   fs_type.length = 16;

   if (LP_DEBUG & DEBUG_TGSI) {
      if (shader->base.ir.nir) {
         nir_print_shader(shader->base.ir.nir, stderr);
      }
   }

   /*
    * Generate the function prototype. Any change here must be reflected in
    * lp_jit.h's lp_jit_frag_func function pointer type, and vice-versa.
    */

   char func_name[256];
   snprintf(func_name, sizeof(func_name), "fs_variant_linear2");

   LLVMTypeRef ret_type = pint8t;
   LLVMTypeRef arg_types[4];
   arg_types[0] = variant->jit_linear_context_ptr_type; /* context */
   arg_types[1] = int32t;                               /* x */
   arg_types[2] = int32t;                               /* y */
   arg_types[3] = int32t;                               /* width */

   LLVMTypeRef func_type =
      LLVMFunctionType(ret_type, arg_types, ARRAY_SIZE(arg_types), 0);

   LLVMValueRef function =
      LLVMAddFunction(gallivm->module, func_name, func_type);
   LLVMSetFunctionCallConv(function, LLVMCCallConv);

   variant->linear_function = function;

   /* XXX: need to propagate noalias down into color param now we are
    * passing a pointer-to-pointer?
    */
   for (unsigned i = 0; i < ARRAY_SIZE(arg_types); ++i) {
      if (LLVMGetTypeKind(arg_types[i]) == LLVMPointerTypeKind) {
         lp_add_function_attr(function, i + 1, LP_FUNC_ATTR_NOALIAS);
      }
   }

   if (variant->gallivm->cache->data_size)
      return;

   LLVMValueRef context_ptr = LLVMGetParam(function, 0);
   LLVMValueRef x = LLVMGetParam(function, 1);
   LLVMValueRef y = LLVMGetParam(function, 2);
   LLVMValueRef width = LLVMGetParam(function, 3);

   lp_build_name(context_ptr, "context");
   lp_build_name(x, "x");
   lp_build_name(y, "y");
   lp_build_name(width, "width");

   /*
    * Function body
    */

   LLVMBasicBlockRef block =
      LLVMAppendBasicBlockInContext(gallivm->context, function, "entry");
   LLVMBuilderRef builder = gallivm->builder;

   LLVMPositionBuilderAtEnd(builder, block);

   struct lp_build_context bld;
   lp_build_context_init(&bld, gallivm, fs_type);

   /*
    * Get context data
    */
   LLVMValueRef consts_ptr =
      lp_jit_linear_context_constants(gallivm,
                                      variant->jit_linear_context_type,
                                      context_ptr);
   LLVMValueRef interpolators_ptr =
      lp_jit_linear_context_inputs(gallivm,
                                   variant->jit_linear_context_type,
                                   context_ptr);
   LLVMValueRef samplers_ptr =
      lp_jit_linear_context_tex(gallivm,
                                variant->jit_linear_context_type,
                                context_ptr);

   LLVMValueRef color0_ptr =
      lp_jit_linear_context_color0(gallivm,
                                   variant->jit_linear_context_type,
                                   context_ptr);
   color0_ptr = LLVMBuildLoad2(builder, LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0),
                               color0_ptr, "");
   color0_ptr = LLVMBuildBitCast(builder, color0_ptr,
                                 LLVMPointerType(bld.vec_type, 0), "");

   LLVMValueRef blend_color =
      lp_jit_linear_context_blend_color(gallivm,
                                        variant->jit_linear_context_type,
                                        context_ptr);
   blend_color = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(gallivm->context),
                                blend_color, "");
   blend_color = lp_build_broadcast(gallivm, LLVMVectorType(int32t, 4),
                                    blend_color);
   blend_color = LLVMBuildBitCast(builder, blend_color,
                                  LLVMVectorType(int8t, 16), "");

   LLVMValueRef alpha_ref =
      lp_jit_linear_context_alpha_ref(gallivm,
                                      variant->jit_linear_context_type,
                                      context_ptr);
   alpha_ref = LLVMBuildLoad2(builder, LLVMInt8TypeInContext(gallivm->context),
                              alpha_ref, "");

   /*
    * Invoke the input interpolators
    */
   LLVMValueRef inputs_ptrs[LP_MAX_LINEAR_INPUTS];

   nir_foreach_shader_in_variable(var, nir) {
      unsigned slots = nir_variable_count_slots(var, var->type);

      for (unsigned s = 0; s < slots; s++) {
         unsigned attrib = var->data.driver_location + s;
         assert(attrib < LP_MAX_LINEAR_INPUTS);
         if (attrib >= LP_MAX_LINEAR_INPUTS) {
            break;
         }

         LLVMValueRef index = LLVMConstInt(int32t, attrib, 0);

         LLVMTypeRef input_type = variant->jit_linear_inputs_type;
         LLVMValueRef elem =
            lp_build_array_get2(bld.gallivm, input_type, interpolators_ptr, index);
         assert(LLVMGetTypeKind(LLVMTypeOf(elem)) == LLVMPointerTypeKind);

         LLVMTypeRef fetch_type = LLVMPointerType(variant->jit_linear_func_type, 0);
         LLVMValueRef fetch_ptr = lp_build_pointer_get2(builder, fetch_type, elem,
                                                        LLVMConstInt(int32t, 0, 0));
         assert(LLVMGetTypeKind(LLVMTypeOf(fetch_ptr)) == LLVMPointerTypeKind);

         /* Pointer to a row of interpolated inputs */
         LLVMTypeRef call_type = variant->jit_linear_func_type;
         elem = LLVMBuildBitCast(builder, elem, pint8t, "");
         LLVMValueRef inputs_ptr = LLVMBuildCall2(builder, call_type, fetch_ptr, &elem, 1, "");
         assert(LLVMGetTypeKind(LLVMTypeOf(inputs_ptr)) == LLVMPointerTypeKind);

         lp_add_function_attr(inputs_ptr, -1, LP_FUNC_ATTR_NOUNWIND);

         lp_build_name(inputs_ptr, "input%u_ptr", attrib);

         inputs_ptrs[attrib] = inputs_ptr;
      }
   }

   /*
    * Invoke and hook up the texture samplers.
    */

   struct linear_sampler sampler;
   memset(&sampler, 0, sizeof sampler);
   sampler.base.emit_fetch_texel = &emit_fetch_texel_linear;

   for (unsigned attrib = 0; attrib < shader->info.num_texs; ++attrib) {
      assert(attrib < LP_MAX_LINEAR_TEXTURES);
      if (attrib >= LP_MAX_LINEAR_TEXTURES) {
         break;
      }

      LLVMValueRef index = LLVMConstInt(int32t, attrib, 0);
      LLVMTypeRef samp_type = variant->jit_linear_textures_type;
      LLVMValueRef elem = lp_build_array_get2(bld.gallivm, samp_type, samplers_ptr, index);
      assert(LLVMGetTypeKind(LLVMTypeOf(elem)) == LLVMPointerTypeKind);

      LLVMTypeRef fetch_type = LLVMPointerType(variant->jit_linear_func_type, 0);
      LLVMValueRef fetch_ptr =
         lp_build_pointer_get2(builder, fetch_type,
                               elem, LLVMConstInt(int32t, 0, 0));
      assert(LLVMGetTypeKind(LLVMTypeOf(fetch_ptr)) == LLVMPointerTypeKind);

      /* Pointer to a row of texels */
      LLVMTypeRef call_type = variant->jit_linear_func_type;
      elem = LLVMBuildBitCast(builder, elem, pint8t, "");
      LLVMValueRef texels_ptr = LLVMBuildCall2(builder, call_type, fetch_ptr, &elem, 1, "");
      assert(LLVMGetTypeKind(LLVMTypeOf(texels_ptr)) == LLVMPointerTypeKind);

      lp_add_function_attr(texels_ptr, -1, LP_FUNC_ATTR_NOUNWIND);

      lp_build_name(texels_ptr, "tex%u_ptr", attrib);

      sampler.texels_ptrs[attrib] = texels_ptr;
   }

   /* excess = width & 0x3 */
   LLVMValueRef excess =
      LLVMBuildAnd(builder, width, LLVMConstInt(int32t, 3, 0), "");
   /* width *= 4 */
   width = LLVMBuildLShr(builder, width, LLVMConstInt(int32t, 2, 0), "");

   /* Loop over blocks of 4 pixels */
   /* for loop.counter = 0; loop.counter < width; loop.counter++) { */
   struct lp_build_for_loop_state loop;
   lp_build_for_loop_begin(&loop, gallivm, LLVMConstInt(int32t, 0, 0),
                           LLVMIntULT, width, LLVMConstInt(int32t, 1, 0));
   {
      LLVMValueRef value;
      sampler.counter = loop.counter;

      /* Read 4 pixels */
      value = lp_build_pointer_get_unaligned2(builder,
                                              bld.vec_type,
                                              color0_ptr,
                                              loop.counter, 4);

      /* Perform fragment shader body */
      value = llvm_fragment_body(&bld, shader, variant, &sampler, inputs_ptrs,
                                 consts_ptr, blend_color, alpha_ref, fs_type,
                                 value);

      /* Write 4 pixels */
      lp_build_pointer_set_unaligned(builder, color0_ptr, loop.counter,
                                     value, 4);
   }
   lp_build_for_loop_end(&loop);

   /* Compute the edge pixels (width % 4) */
   struct lp_build_if_state ifstate;
   lp_build_if(&ifstate, gallivm, LLVMBuildICmp(builder, LLVMIntNE, excess,
                                            LLVMConstInt(int32t, 0, 0), ""));
   {
      struct lp_build_loop_state loop_read, loop_write;
      LLVMValueRef buf, elem, result, pixel_ptr;
      LLVMValueRef buf_ptr = lp_build_alloca(gallivm, pixelt, "");

      sampler.counter = width;

      /* Get the i32* pixel pointer from the <i16x8>* element pointer */
      pixel_ptr = LLVMBuildGEP2(gallivm->builder, bld.vec_type,
                                color0_ptr, &width, 1, "");
      pixel_ptr = LLVMBuildBitCast(gallivm->builder, pixel_ptr,
                                   LLVMPointerType(int32t, 0), "");

      /* Copy individual pixels from memory to local buffer */
      lp_build_loop_begin(&loop_read, gallivm, LLVMConstInt(int32t, 0, 0));
      {
         elem = lp_build_pointer_get2(gallivm->builder,
                                      int32t,
                                      pixel_ptr, loop_read.counter);

         buf = LLVMBuildLoad2(gallivm->builder, pixelt, buf_ptr, "");
         buf = LLVMBuildInsertElement(builder, buf, elem,
                                      loop_read.counter, "");
         LLVMBuildStore(builder, buf, buf_ptr);
      }
      lp_build_loop_end_cond(&loop_read, excess,
                             LLVMConstInt(int32t, 1, 0), LLVMIntUGE);

      /* Perform fragment shader body */
      buf = LLVMBuildLoad2(gallivm->builder, pixelt, buf_ptr, "");
      buf = LLVMBuildBitCast(builder, buf, bld.vec_type, "");

      result = llvm_fragment_body(&bld, shader, variant, &sampler,
                                  inputs_ptrs, consts_ptr, blend_color,
                                  alpha_ref, fs_type, buf);
      result = LLVMBuildBitCast(builder, result, pixelt, "");

      /* Write individual pixels from local buffer to the memory */
      lp_build_loop_begin(&loop_write, gallivm, LLVMConstInt(int32t, 0, 0));
      {
         elem = LLVMBuildExtractElement(builder, result,
                                        loop_write.counter, "");

         lp_build_pointer_set(gallivm->builder, pixel_ptr,
                              loop_write.counter, elem);
      }
      lp_build_loop_end_cond(&loop_write, excess,
                             LLVMConstInt(int32t, 1, 0), LLVMIntUGE);
   }
   lp_build_endif(&ifstate);

   color0_ptr = LLVMBuildBitCast(builder, color0_ptr, pint8t, "");

   LLVMBuildRet(builder, color0_ptr);

   gallivm_verify_function(gallivm, function);
}
