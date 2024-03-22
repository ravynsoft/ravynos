/**************************************************************************
 *
 * Copyright 2019 Red Hat.
 * All Rights Reserved.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **************************************************************************/

#include "lp_bld_nir.h"
#include "lp_bld_arit.h"
#include "lp_bld_bitarit.h"
#include "lp_bld_const.h"
#include "lp_bld_conv.h"
#include "lp_bld_gather.h"
#include "lp_bld_logic.h"
#include "lp_bld_quad.h"
#include "lp_bld_flow.h"
#include "lp_bld_intr.h"
#include "lp_bld_struct.h"
#include "lp_bld_debug.h"
#include "lp_bld_printf.h"
#include "nir.h"
#include "nir_deref.h"
#include "nir_search_helpers.h"


// Doing AOS (and linear) codegen?
static bool
is_aos(const struct lp_build_nir_context *bld_base)
{
   // AOS is used for vectors of uint8[16]
   return bld_base->base.type.length == 16 && bld_base->base.type.width == 8;
}


static void
visit_cf_list(struct lp_build_nir_context *bld_base,
              struct exec_list *list);


static LLVMValueRef
cast_type(struct lp_build_nir_context *bld_base, LLVMValueRef val,
          nir_alu_type alu_type, unsigned bit_size)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   switch (alu_type) {
   case nir_type_float:
      switch (bit_size) {
      case 16:
         return LLVMBuildBitCast(builder, val, bld_base->half_bld.vec_type, "");
      case 32:
         return LLVMBuildBitCast(builder, val, bld_base->base.vec_type, "");
      case 64:
         return LLVMBuildBitCast(builder, val, bld_base->dbl_bld.vec_type, "");
      default:
         assert(0);
         break;
      }
      break;
   case nir_type_int:
      switch (bit_size) {
      case 8:
         return LLVMBuildBitCast(builder, val, bld_base->int8_bld.vec_type, "");
      case 16:
         return LLVMBuildBitCast(builder, val, bld_base->int16_bld.vec_type, "");
      case 32:
         return LLVMBuildBitCast(builder, val, bld_base->int_bld.vec_type, "");
      case 64:
         return LLVMBuildBitCast(builder, val, bld_base->int64_bld.vec_type, "");
      default:
         assert(0);
         break;
      }
      break;
   case nir_type_uint:
      switch (bit_size) {
      case 8:
         return LLVMBuildBitCast(builder, val, bld_base->uint8_bld.vec_type, "");
      case 16:
         return LLVMBuildBitCast(builder, val, bld_base->uint16_bld.vec_type, "");
      case 1:
      case 32:
         return LLVMBuildBitCast(builder, val, bld_base->uint_bld.vec_type, "");
      case 64:
         return LLVMBuildBitCast(builder, val, bld_base->uint64_bld.vec_type, "");
      default:
         assert(0);
         break;
      }
      break;
   case nir_type_uint32:
      return LLVMBuildBitCast(builder, val, bld_base->uint_bld.vec_type, "");
   default:
      return val;
   }
   return NULL;
}


static unsigned
glsl_sampler_to_pipe(int sampler_dim, bool is_array)
{
   unsigned pipe_target = PIPE_BUFFER;
   switch (sampler_dim) {
   case GLSL_SAMPLER_DIM_1D:
      pipe_target = is_array ? PIPE_TEXTURE_1D_ARRAY : PIPE_TEXTURE_1D;
      break;
   case GLSL_SAMPLER_DIM_2D:
      pipe_target = is_array ? PIPE_TEXTURE_2D_ARRAY : PIPE_TEXTURE_2D;
      break;
   case GLSL_SAMPLER_DIM_SUBPASS:
   case GLSL_SAMPLER_DIM_SUBPASS_MS:
      pipe_target = PIPE_TEXTURE_2D_ARRAY;
      break;
   case GLSL_SAMPLER_DIM_3D:
      pipe_target = PIPE_TEXTURE_3D;
      break;
   case GLSL_SAMPLER_DIM_MS:
      pipe_target = is_array ? PIPE_TEXTURE_2D_ARRAY : PIPE_TEXTURE_2D;
      break;
   case GLSL_SAMPLER_DIM_CUBE:
      pipe_target = is_array ? PIPE_TEXTURE_CUBE_ARRAY : PIPE_TEXTURE_CUBE;
      break;
   case GLSL_SAMPLER_DIM_RECT:
      pipe_target = PIPE_TEXTURE_RECT;
      break;
   case GLSL_SAMPLER_DIM_BUF:
      pipe_target = PIPE_BUFFER;
      break;
   default:
      break;
   }
   return pipe_target;
}


static LLVMValueRef
get_src(struct lp_build_nir_context *bld_base, nir_src src)
{
   return bld_base->ssa_defs[src.ssa->index];
}


static void
assign_ssa(struct lp_build_nir_context *bld_base, int idx, LLVMValueRef ptr)
{
   bld_base->ssa_defs[idx] = ptr;
}


static void
assign_ssa_dest(struct lp_build_nir_context *bld_base, const nir_def *ssa,
                LLVMValueRef vals[NIR_MAX_VEC_COMPONENTS])
{
   if ((ssa->num_components == 1 || is_aos(bld_base))) {
      assign_ssa(bld_base, ssa->index, vals[0]);
   } else {
      assign_ssa(bld_base, ssa->index,
             lp_nir_array_build_gather_values(bld_base->base.gallivm->builder,
                                              vals, ssa->num_components));
   }
}


static LLVMValueRef
fcmp32(struct lp_build_nir_context *bld_base,
       enum pipe_compare_func compare,
       uint32_t src_bit_size,
       LLVMValueRef src[NIR_MAX_VEC_COMPONENTS])
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   struct lp_build_context *flt_bld = get_flt_bld(bld_base, src_bit_size);
   LLVMValueRef result;

   if (compare != PIPE_FUNC_NOTEQUAL)
      result = lp_build_cmp_ordered(flt_bld, compare, src[0], src[1]);
   else
      result = lp_build_cmp(flt_bld, compare, src[0], src[1]);
   if (src_bit_size == 64)
      result = LLVMBuildTrunc(builder, result, bld_base->int_bld.vec_type, "");
   else if (src_bit_size == 16)
      result = LLVMBuildSExt(builder, result, bld_base->int_bld.vec_type, "");
   return result;
}


static LLVMValueRef
icmp32(struct lp_build_nir_context *bld_base,
       enum pipe_compare_func compare,
       bool is_unsigned,
       uint32_t src_bit_size,
       LLVMValueRef src[NIR_MAX_VEC_COMPONENTS])
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   struct lp_build_context *i_bld =
      get_int_bld(bld_base, is_unsigned, src_bit_size);
   LLVMValueRef result = lp_build_cmp(i_bld, compare, src[0], src[1]);
   if (src_bit_size < 32)
      result = LLVMBuildSExt(builder, result, bld_base->int_bld.vec_type, "");
   else if (src_bit_size == 64)
      result = LLVMBuildTrunc(builder, result, bld_base->int_bld.vec_type, "");
   return result;
}


/**
 * Get a source register value for an ALU instruction.
 * This is where swizzles are handled.  There should be no negation
 * or absolute value modifiers.
 * num_components indicates the number of components needed in the
 * returned array or vector.
 */
static LLVMValueRef
get_alu_src(struct lp_build_nir_context *bld_base,
            nir_alu_src src,
            unsigned num_components)
{
   assert(num_components >= 1);
   assert(num_components <= 4);

   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   const unsigned src_components = nir_src_num_components(src.src);
   assert(src_components > 0);
   LLVMValueRef value = get_src(bld_base, src.src);
   assert(value);

   /* check if swizzling needed for the src vector */
   bool need_swizzle = false;
   for (unsigned i = 0; i < src_components; ++i) {
      if (src.swizzle[i] != i) {
         need_swizzle = true;
         break;
      }
   }

   if (is_aos(bld_base) && !need_swizzle) {
      return value;
   }

   if (need_swizzle || num_components != src_components) {
      if (is_aos(bld_base) && need_swizzle) {
         // Handle swizzle for AOS
         assert(LLVMGetTypeKind(LLVMTypeOf(value)) == LLVMVectorTypeKind);

         // swizzle vector of ((r,g,b,a), (r,g,b,a), (r,g,b,a), (r,g,b,a))
         assert(bld_base->base.type.width == 8);
         assert(bld_base->base.type.length == 16);

         // Do our own swizzle here since lp_build_swizzle_aos_n() does
         // not do what we want.
         // Ex: value = {r0,g0,b0,a0, r1,g1,b1,a1, r2,g2,b2,a2, r3,g3,b3,a3}.
         // aos swizzle = {2,1,0,3}  // swap red/blue
         // shuffles = {2,1,0,3, 6,5,4,7, 10,9,8,11, 14,13,12,15}
         // result = {b0,g0,r0,a0, b1,g1,r1,a1, b2,g2,r2,a2, b3,g3,r3,a3}.
         LLVMValueRef shuffles[LP_MAX_VECTOR_WIDTH];
         for (unsigned i = 0; i < 16; i++) {
            unsigned chan = i % 4;
            /* apply src register swizzle */
            if (chan < num_components) {
               chan = src.swizzle[chan];
            } else {
               chan = src.swizzle[0];
            }
            /* apply aos swizzle */
            chan = lp_nir_aos_swizzle(bld_base, chan);
            shuffles[i] = lp_build_const_int32(gallivm, (i & ~3) + chan);
         }
         value = LLVMBuildShuffleVector(builder, value,
                                        LLVMGetUndef(LLVMTypeOf(value)),
                                        LLVMConstVector(shuffles, 16), "");
      } else if (src_components > 1 && num_components == 1) {
         value = LLVMBuildExtractValue(gallivm->builder, value,
                                       src.swizzle[0], "");
      } else if (src_components == 1 && num_components > 1) {
         LLVMValueRef values[] = {value, value, value, value,
                                  value, value, value, value,
                                  value, value, value, value,
                                  value, value, value, value};
         value = lp_nir_array_build_gather_values(builder, values, num_components);
      } else {
         LLVMValueRef arr = LLVMGetUndef(LLVMArrayType(LLVMTypeOf(LLVMBuildExtractValue(builder, value, 0, "")), num_components));
         for (unsigned i = 0; i < num_components; i++)
            arr = LLVMBuildInsertValue(builder, arr, LLVMBuildExtractValue(builder, value, src.swizzle[i], ""), i, "");
         value = arr;
      }
   }

   return value;
}


static LLVMValueRef
emit_b2f(struct lp_build_nir_context *bld_base,
         LLVMValueRef src0,
         unsigned bitsize)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef result =
      LLVMBuildAnd(builder, cast_type(bld_base, src0, nir_type_int, 32),
                   LLVMBuildBitCast(builder,
                                    lp_build_const_vec(bld_base->base.gallivm,
                                                       bld_base->base.type,
                                                       1.0),
                                    bld_base->int_bld.vec_type, ""),
                   "");
   result = LLVMBuildBitCast(builder, result, bld_base->base.vec_type, "");
   switch (bitsize) {
   case 16:
      result = LLVMBuildFPTrunc(builder, result,
                                bld_base->half_bld.vec_type, "");
      break;
   case 32:
      break;
   case 64:
      result = LLVMBuildFPExt(builder, result,
                              bld_base->dbl_bld.vec_type, "");
      break;
   default:
      unreachable("unsupported bit size.");
   }
   return result;
}


static LLVMValueRef
emit_b2i(struct lp_build_nir_context *bld_base,
         LLVMValueRef src0,
         unsigned bitsize)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef result = LLVMBuildAnd(builder,
                          cast_type(bld_base, src0, nir_type_int, 32),
                          lp_build_const_int_vec(bld_base->base.gallivm,
                                                 bld_base->base.type, 1), "");
   switch (bitsize) {
   case 8:
      return LLVMBuildTrunc(builder, result, bld_base->int8_bld.vec_type, "");
   case 16:
      return LLVMBuildTrunc(builder, result, bld_base->int16_bld.vec_type, "");
   case 32:
      return result;
   case 64:
      return LLVMBuildZExt(builder, result, bld_base->int64_bld.vec_type, "");
   default:
      unreachable("unsupported bit size.");
   }
}


static LLVMValueRef
emit_b32csel(struct lp_build_nir_context *bld_base,
             unsigned src_bit_size[NIR_MAX_VEC_COMPONENTS],
             LLVMValueRef src[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef sel = cast_type(bld_base, src[0], nir_type_int, 32);
   LLVMValueRef v = lp_build_compare(bld_base->base.gallivm, bld_base->int_bld.type, PIPE_FUNC_NOTEQUAL, sel, bld_base->int_bld.zero);
   struct lp_build_context *bld = get_int_bld(bld_base, false, src_bit_size[1]);
   return lp_build_select(bld, v, src[1], src[2]);
}


static LLVMValueRef
split_64bit(struct lp_build_nir_context *bld_base,
            LLVMValueRef src,
            bool hi)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMValueRef shuffles[LP_MAX_VECTOR_WIDTH/32];
   LLVMValueRef shuffles2[LP_MAX_VECTOR_WIDTH/32];
   int len = bld_base->base.type.length * 2;
   for (unsigned i = 0; i < bld_base->base.type.length; i++) {
#if UTIL_ARCH_LITTLE_ENDIAN
      shuffles[i] = lp_build_const_int32(gallivm, i * 2);
      shuffles2[i] = lp_build_const_int32(gallivm, (i * 2) + 1);
#else
      shuffles[i] = lp_build_const_int32(gallivm, (i * 2) + 1);
      shuffles2[i] = lp_build_const_int32(gallivm, (i * 2));
#endif
   }

   src = LLVMBuildBitCast(gallivm->builder, src,
           LLVMVectorType(LLVMInt32TypeInContext(gallivm->context), len), "");
   return LLVMBuildShuffleVector(gallivm->builder, src,
                                 LLVMGetUndef(LLVMTypeOf(src)),
                                 LLVMConstVector(hi ? shuffles2 : shuffles,
                                                 bld_base->base.type.length),
                                 "");
}


static LLVMValueRef
merge_64bit(struct lp_build_nir_context *bld_base,
            LLVMValueRef input,
            LLVMValueRef input2)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   int i;
   LLVMValueRef shuffles[2 * (LP_MAX_VECTOR_WIDTH/32)];
   int len = bld_base->base.type.length * 2;
   assert(len <= (2 * (LP_MAX_VECTOR_WIDTH/32)));

   for (i = 0; i < bld_base->base.type.length * 2; i+=2) {
#if UTIL_ARCH_LITTLE_ENDIAN
      shuffles[i] = lp_build_const_int32(gallivm, i / 2);
      shuffles[i + 1] = lp_build_const_int32(gallivm, i / 2 + bld_base->base.type.length);
#else
      shuffles[i] = lp_build_const_int32(gallivm, i / 2 + bld_base->base.type.length);
      shuffles[i + 1] = lp_build_const_int32(gallivm, i / 2);
#endif
   }
   return LLVMBuildShuffleVector(builder, input, input2, LLVMConstVector(shuffles, len), "");
}


static LLVMValueRef
split_16bit(struct lp_build_nir_context *bld_base,
            LLVMValueRef src,
            bool hi)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMValueRef shuffles[LP_MAX_VECTOR_WIDTH/32];
   LLVMValueRef shuffles2[LP_MAX_VECTOR_WIDTH/32];
   int len = bld_base->base.type.length * 2;
   for (unsigned i = 0; i < bld_base->base.type.length; i++) {
#if UTIL_ARCH_LITTLE_ENDIAN
      shuffles[i] = lp_build_const_int32(gallivm, i * 2);
      shuffles2[i] = lp_build_const_int32(gallivm, (i * 2) + 1);
#else
      shuffles[i] = lp_build_const_int32(gallivm, (i * 2) + 1);
      shuffles2[i] = lp_build_const_int32(gallivm, (i * 2));
#endif
   }

   src = LLVMBuildBitCast(gallivm->builder, src, LLVMVectorType(LLVMInt16TypeInContext(gallivm->context), len), "");
   return LLVMBuildShuffleVector(gallivm->builder, src,
                                 LLVMGetUndef(LLVMTypeOf(src)),
                                 LLVMConstVector(hi ? shuffles2 : shuffles,
                                                 bld_base->base.type.length),
                                 "");
}


static LLVMValueRef
merge_16bit(struct lp_build_nir_context *bld_base,
            LLVMValueRef input,
            LLVMValueRef input2)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   int i;
   LLVMValueRef shuffles[2 * (LP_MAX_VECTOR_WIDTH/32)];
   int len = bld_base->int16_bld.type.length * 2;
   assert(len <= (2 * (LP_MAX_VECTOR_WIDTH/32)));

   for (i = 0; i < bld_base->int_bld.type.length * 2; i+=2) {
#if UTIL_ARCH_LITTLE_ENDIAN
      shuffles[i] = lp_build_const_int32(gallivm, i / 2);
      shuffles[i + 1] = lp_build_const_int32(gallivm, i / 2 + bld_base->base.type.length);
#else
      shuffles[i] = lp_build_const_int32(gallivm, i / 2 + bld_base->base.type.length);
      shuffles[i + 1] = lp_build_const_int32(gallivm, i / 2);
#endif
   }
   return LLVMBuildShuffleVector(builder, input, input2, LLVMConstVector(shuffles, len), "");
}


static LLVMValueRef
get_signed_divisor(struct gallivm_state *gallivm,
                   struct lp_build_context *int_bld,
                   struct lp_build_context *mask_bld,
                   int src_bit_size,
                   LLVMValueRef src, LLVMValueRef divisor)
{
   LLVMBuilderRef builder = gallivm->builder;
   /* However for signed divides SIGFPE can occur if the numerator is INT_MIN
      and divisor is -1. */
   /* set mask if numerator == INT_MIN */
   long long min_val;
   switch (src_bit_size) {
   case 8:
      min_val = INT8_MIN;
      break;
   case 16:
      min_val = INT16_MIN;
      break;
   default:
   case 32:
      min_val = INT_MIN;
      break;
   case 64:
      min_val = INT64_MIN;
      break;
   }
   LLVMValueRef div_mask2 = lp_build_cmp(mask_bld, PIPE_FUNC_EQUAL, src,
                                         lp_build_const_int_vec(gallivm, int_bld->type, min_val));
   /* set another mask if divisor is - 1 */
   LLVMValueRef div_mask3 = lp_build_cmp(mask_bld, PIPE_FUNC_EQUAL, divisor,
                                         lp_build_const_int_vec(gallivm, int_bld->type, -1));
   div_mask2 = LLVMBuildAnd(builder, div_mask2, div_mask3, "");

   divisor = lp_build_select(mask_bld, div_mask2, int_bld->one, divisor);
   return divisor;
}


static LLVMValueRef
do_int_divide(struct lp_build_nir_context *bld_base,
              bool is_unsigned, unsigned src_bit_size,
              LLVMValueRef src, LLVMValueRef src2)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *int_bld = get_int_bld(bld_base, is_unsigned, src_bit_size);
   struct lp_build_context *mask_bld = get_int_bld(bld_base, true, src_bit_size);

   /* avoid divide by 0. Converted divisor from 0 to -1 */
   LLVMValueRef div_mask = lp_build_cmp(mask_bld, PIPE_FUNC_EQUAL, src2,
                                        mask_bld->zero);

   LLVMValueRef divisor = LLVMBuildOr(builder, div_mask, src2, "");
   if (!is_unsigned) {
      divisor = get_signed_divisor(gallivm, int_bld, mask_bld,
                                   src_bit_size, src, divisor);
   }
   LLVMValueRef result = lp_build_div(int_bld, src, divisor);

   if (!is_unsigned) {
      LLVMValueRef not_div_mask = LLVMBuildNot(builder, div_mask, "");
      return LLVMBuildAnd(builder, not_div_mask, result, "");
   } else
      /* udiv by zero is guaranteed to return 0xffffffff at least with d3d10
       * may as well do same for idiv */
      return LLVMBuildOr(builder, div_mask, result, "");
}


static LLVMValueRef
do_int_mod(struct lp_build_nir_context *bld_base,
           bool is_unsigned, unsigned src_bit_size,
           LLVMValueRef src, LLVMValueRef src2)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *int_bld = get_int_bld(bld_base, is_unsigned, src_bit_size);
   struct lp_build_context *mask_bld = get_int_bld(bld_base, true, src_bit_size);
   LLVMValueRef div_mask = lp_build_cmp(mask_bld, PIPE_FUNC_EQUAL, src2,
                                        mask_bld->zero);
   LLVMValueRef divisor = LLVMBuildOr(builder,
                                      div_mask,
                                      src2, "");
   if (!is_unsigned) {
      divisor = get_signed_divisor(gallivm, int_bld, mask_bld,
                                   src_bit_size, src, divisor);
   }
   LLVMValueRef result = lp_build_mod(int_bld, src, divisor);
   return LLVMBuildOr(builder, div_mask, result, "");
}

static LLVMValueRef
do_alu_action(struct lp_build_nir_context *bld_base,
              const nir_alu_instr *instr,
              unsigned src_bit_size[NIR_MAX_VEC_COMPONENTS],
              LLVMValueRef src[NIR_MAX_VEC_COMPONENTS])
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef result;

   switch (instr->op) {
   case nir_op_b2f16:
      result = emit_b2f(bld_base, src[0], 16);
      break;
   case nir_op_b2f32:
      result = emit_b2f(bld_base, src[0], 32);
      break;
   case nir_op_b2f64:
      result = emit_b2f(bld_base, src[0], 64);
      break;
   case nir_op_b2i8:
      result = emit_b2i(bld_base, src[0], 8);
      break;
   case nir_op_b2i16:
      result = emit_b2i(bld_base, src[0], 16);
      break;
   case nir_op_b2i32:
      result = emit_b2i(bld_base, src[0], 32);
      break;
   case nir_op_b2i64:
      result = emit_b2i(bld_base, src[0], 64);
      break;
   case nir_op_b32csel:
      result = emit_b32csel(bld_base, src_bit_size, src);
      break;
   case nir_op_bit_count:
      result = lp_build_popcount(get_int_bld(bld_base, false, src_bit_size[0]), src[0]);
      if (src_bit_size[0] < 32)
         result = LLVMBuildZExt(builder, result, bld_base->int_bld.vec_type, "");
      else if (src_bit_size[0] > 32)
         result = LLVMBuildTrunc(builder, result, bld_base->int_bld.vec_type, "");
      break;
   case nir_op_bitfield_select:
      result = lp_build_xor(&bld_base->uint_bld, src[2], lp_build_and(&bld_base->uint_bld, src[0], lp_build_xor(&bld_base->uint_bld, src[1], src[2])));
      break;
   case nir_op_bitfield_reverse:
      result = lp_build_bitfield_reverse(get_int_bld(bld_base, false, src_bit_size[0]), src[0]);
      break;
   case nir_op_f2f16:
      if (src_bit_size[0] == 64)
         src[0] = LLVMBuildFPTrunc(builder, src[0],
                                   bld_base->base.vec_type, "");
      result = LLVMBuildFPTrunc(builder, src[0],
                                bld_base->half_bld.vec_type, "");
      break;
   case nir_op_f2f32:
      if (src_bit_size[0] < 32)
         result = LLVMBuildFPExt(builder, src[0],
                                 bld_base->base.vec_type, "");
      else
         result = LLVMBuildFPTrunc(builder, src[0],
                                   bld_base->base.vec_type, "");
      break;
   case nir_op_f2f64:
      result = LLVMBuildFPExt(builder, src[0],
                              bld_base->dbl_bld.vec_type, "");
      break;
   case nir_op_f2i8:
      result = LLVMBuildFPToSI(builder,
                               src[0],
                               bld_base->uint8_bld.vec_type, "");
      break;
   case nir_op_f2i16:
      result = LLVMBuildFPToSI(builder,
                               src[0],
                               bld_base->uint16_bld.vec_type, "");
      break;
   case nir_op_f2i32:
      result = LLVMBuildFPToSI(builder, src[0], bld_base->base.int_vec_type, "");
      break;
   case nir_op_f2u8:
      result = LLVMBuildFPToUI(builder,
                               src[0],
                               bld_base->uint8_bld.vec_type, "");
      break;
   case nir_op_f2u16:
      result = LLVMBuildFPToUI(builder,
                               src[0],
                               bld_base->uint16_bld.vec_type, "");
      break;
   case nir_op_f2u32:
      result = LLVMBuildFPToUI(builder,
                               src[0],
                               bld_base->base.int_vec_type, "");
      break;
   case nir_op_f2i64:
      result = LLVMBuildFPToSI(builder,
                               src[0],
                               bld_base->int64_bld.vec_type, "");
      break;
   case nir_op_f2u64:
      result = LLVMBuildFPToUI(builder,
                               src[0],
                               bld_base->uint64_bld.vec_type, "");
      break;
   case nir_op_fabs:
      result = lp_build_abs(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fadd:
      result = lp_build_add(get_flt_bld(bld_base, src_bit_size[0]),
                            src[0], src[1]);
      break;
   case nir_op_fceil:
      result = lp_build_ceil(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fcos:
      result = lp_build_cos(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fddx:
   case nir_op_fddx_coarse:
   case nir_op_fddx_fine:
      result = lp_build_ddx(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fddy:
   case nir_op_fddy_coarse:
   case nir_op_fddy_fine:
      result = lp_build_ddy(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fdiv:
      result = lp_build_div(get_flt_bld(bld_base, src_bit_size[0]),
                            src[0], src[1]);
      break;
   case nir_op_feq32:
      result = fcmp32(bld_base, PIPE_FUNC_EQUAL, src_bit_size[0], src);
      break;
   case nir_op_fexp2:
      result = lp_build_exp2(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_ffloor:
      result = lp_build_floor(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_ffma:
      result = lp_build_fmuladd(builder, src[0], src[1], src[2]);
      break;
   case nir_op_ffract: {
      struct lp_build_context *flt_bld = get_flt_bld(bld_base, src_bit_size[0]);
      LLVMValueRef tmp = lp_build_floor(flt_bld, src[0]);
      result = lp_build_sub(flt_bld, src[0], tmp);
      break;
   }
   case nir_op_fge:
   case nir_op_fge32:
      result = fcmp32(bld_base, PIPE_FUNC_GEQUAL, src_bit_size[0], src);
      break;
   case nir_op_find_lsb: {
      struct lp_build_context *int_bld = get_int_bld(bld_base, false, src_bit_size[0]);
      result = lp_build_cttz(int_bld, src[0]);
      if (src_bit_size[0] < 32)
         result = LLVMBuildZExt(builder, result, bld_base->uint_bld.vec_type, "");
      else if (src_bit_size[0] > 32)
         result = LLVMBuildTrunc(builder, result, bld_base->uint_bld.vec_type, "");
      break;
   }
   case nir_op_fisfinite32:
      unreachable("Should have been lowered in nir_opt_algebraic_late.");
   case nir_op_flog2:
      result = lp_build_log2_safe(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_flt:
   case nir_op_flt32:
      result = fcmp32(bld_base, PIPE_FUNC_LESS, src_bit_size[0], src);
      break;
   case nir_op_fmax:
   case nir_op_fmin: {
      enum gallivm_nan_behavior minmax_nan;
      int first = 0;

      /* If one of the sources is known to be a number (i.e., not NaN), then
       * better code can be generated by passing that information along.
       */
      if (is_a_number(bld_base->range_ht, instr, 1,
                      0 /* unused num_components */,
                      NULL /* unused swizzle */)) {
         minmax_nan = GALLIVM_NAN_RETURN_OTHER_SECOND_NONNAN;
      } else if (is_a_number(bld_base->range_ht, instr, 0,
                             0 /* unused num_components */,
                             NULL /* unused swizzle */)) {
         first = 1;
         minmax_nan = GALLIVM_NAN_RETURN_OTHER_SECOND_NONNAN;
      } else {
         minmax_nan = GALLIVM_NAN_RETURN_OTHER;
      }

      if (instr->op == nir_op_fmin) {
         result = lp_build_min_ext(get_flt_bld(bld_base, src_bit_size[0]),
                                   src[first], src[1 - first], minmax_nan);
      } else {
         result = lp_build_max_ext(get_flt_bld(bld_base, src_bit_size[0]),
                                   src[first], src[1 - first], minmax_nan);
      }
      break;
   }
   case nir_op_fmod: {
      struct lp_build_context *flt_bld = get_flt_bld(bld_base, src_bit_size[0]);
      result = lp_build_div(flt_bld, src[0], src[1]);
      result = lp_build_floor(flt_bld, result);
      result = lp_build_mul(flt_bld, src[1], result);
      result = lp_build_sub(flt_bld, src[0], result);
      break;
   }
   case nir_op_fmul:
      result = lp_build_mul(get_flt_bld(bld_base, src_bit_size[0]),
                            src[0], src[1]);
      break;
   case nir_op_fneu32:
      result = fcmp32(bld_base, PIPE_FUNC_NOTEQUAL, src_bit_size[0], src);
      break;
   case nir_op_fneg:
      result = lp_build_negate(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fpow:
      result = lp_build_pow(get_flt_bld(bld_base, src_bit_size[0]), src[0], src[1]);
      break;
   case nir_op_frcp:
      result = lp_build_rcp(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fround_even:
      if (src_bit_size[0] == 16) {
         struct lp_build_context *bld = get_flt_bld(bld_base, 16);
         char intrinsic[64];
         lp_format_intrinsic(intrinsic, 64, "llvm.roundeven", bld->vec_type);
         result = lp_build_intrinsic_unary(builder, intrinsic, bld->vec_type, src[0]);
      } else {
         result = lp_build_round(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      }
      break;
   case nir_op_frsq:
      result = lp_build_rsqrt(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fsat:
      result = lp_build_clamp_zero_one_nanzero(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fsign:
      result = lp_build_sgn(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fsin:
      result = lp_build_sin(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_fsqrt:
      result = lp_build_sqrt(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_ftrunc:
      result = lp_build_trunc(get_flt_bld(bld_base, src_bit_size[0]), src[0]);
      break;
   case nir_op_i2f16:
      result = LLVMBuildSIToFP(builder, src[0],
                               bld_base->half_bld.vec_type, "");
      break;
   case nir_op_i2f32:
      result = lp_build_int_to_float(&bld_base->base, src[0]);
      break;
   case nir_op_i2f64:
      result = lp_build_int_to_float(&bld_base->dbl_bld, src[0]);
      break;
   case nir_op_i2i8:
      result = LLVMBuildTrunc(builder, src[0], bld_base->int8_bld.vec_type, "");
      break;
   case nir_op_i2i16:
      if (src_bit_size[0] < 16)
         result = LLVMBuildSExt(builder, src[0], bld_base->int16_bld.vec_type, "");
      else
         result = LLVMBuildTrunc(builder, src[0], bld_base->int16_bld.vec_type, "");
      break;
   case nir_op_i2i32:
      if (src_bit_size[0] < 32)
         result = LLVMBuildSExt(builder, src[0], bld_base->int_bld.vec_type, "");
      else
         result = LLVMBuildTrunc(builder, src[0], bld_base->int_bld.vec_type, "");
      break;
   case nir_op_i2i64:
      result = LLVMBuildSExt(builder, src[0], bld_base->int64_bld.vec_type, "");
      break;
   case nir_op_iabs:
      result = lp_build_abs(get_int_bld(bld_base, false, src_bit_size[0]), src[0]);
      break;
   case nir_op_iadd:
      result = lp_build_add(get_int_bld(bld_base, false, src_bit_size[0]),
                            src[0], src[1]);
      break;
   case nir_op_iand:
      result = lp_build_and(get_int_bld(bld_base, false, src_bit_size[0]),
                            src[0], src[1]);
      break;
   case nir_op_idiv:
      result = do_int_divide(bld_base, false, src_bit_size[0], src[0], src[1]);
      break;
   case nir_op_ieq32:
      result = icmp32(bld_base, PIPE_FUNC_EQUAL, false, src_bit_size[0], src);
      break;
   case nir_op_ige32:
      result = icmp32(bld_base, PIPE_FUNC_GEQUAL, false, src_bit_size[0], src);
      break;
   case nir_op_ilt32:
      result = icmp32(bld_base, PIPE_FUNC_LESS, false, src_bit_size[0], src);
      break;
   case nir_op_imax:
      result = lp_build_max(get_int_bld(bld_base, false, src_bit_size[0]), src[0], src[1]);
      break;
   case nir_op_imin:
      result = lp_build_min(get_int_bld(bld_base, false, src_bit_size[0]), src[0], src[1]);
      break;
   case nir_op_imul:
   case nir_op_imul24:
      result = lp_build_mul(get_int_bld(bld_base, false, src_bit_size[0]),
                            src[0], src[1]);
      break;
   case nir_op_imul_high: {
      LLVMValueRef hi_bits;
      lp_build_mul_32_lohi(get_int_bld(bld_base, false, src_bit_size[0]), src[0], src[1], &hi_bits);
      result = hi_bits;
      break;
   }
   case nir_op_ine32:
      result = icmp32(bld_base, PIPE_FUNC_NOTEQUAL, false, src_bit_size[0], src);
      break;
   case nir_op_ineg:
      result = lp_build_negate(get_int_bld(bld_base, false, src_bit_size[0]), src[0]);
      break;
   case nir_op_inot:
      result = lp_build_not(get_int_bld(bld_base, false, src_bit_size[0]), src[0]);
      break;
   case nir_op_ior:
      result = lp_build_or(get_int_bld(bld_base, false, src_bit_size[0]),
                           src[0], src[1]);
      break;
   case nir_op_imod:
   case nir_op_irem:
      result = do_int_mod(bld_base, false, src_bit_size[0], src[0], src[1]);
      break;
   case nir_op_ishl: {
      struct lp_build_context *uint_bld = get_int_bld(bld_base, true, src_bit_size[0]);
      struct lp_build_context *int_bld = get_int_bld(bld_base, false, src_bit_size[0]);
      if (src_bit_size[0] == 64)
         src[1] = LLVMBuildZExt(builder, src[1], uint_bld->vec_type, "");
      if (src_bit_size[0] < 32)
         src[1] = LLVMBuildTrunc(builder, src[1], uint_bld->vec_type, "");
      src[1] = lp_build_and(uint_bld, src[1], lp_build_const_int_vec(gallivm, uint_bld->type, (src_bit_size[0] - 1)));
      result = lp_build_shl(int_bld, src[0], src[1]);
      break;
   }
   case nir_op_ishr: {
      struct lp_build_context *uint_bld = get_int_bld(bld_base, true, src_bit_size[0]);
      struct lp_build_context *int_bld = get_int_bld(bld_base, false, src_bit_size[0]);
      if (src_bit_size[0] == 64)
         src[1] = LLVMBuildZExt(builder, src[1], uint_bld->vec_type, "");
      if (src_bit_size[0] < 32)
         src[1] = LLVMBuildTrunc(builder, src[1], uint_bld->vec_type, "");
      src[1] = lp_build_and(uint_bld, src[1], lp_build_const_int_vec(gallivm, uint_bld->type, (src_bit_size[0] - 1)));
      result = lp_build_shr(int_bld, src[0], src[1]);
      break;
   }
   case nir_op_isign:
      result = lp_build_sgn(get_int_bld(bld_base, false, src_bit_size[0]), src[0]);
      break;
   case nir_op_isub:
      result = lp_build_sub(get_int_bld(bld_base, false, src_bit_size[0]),
                            src[0], src[1]);
      break;
   case nir_op_ixor:
      result = lp_build_xor(get_int_bld(bld_base, false, src_bit_size[0]),
                            src[0], src[1]);
      break;
   case nir_op_mov:
      result = src[0];
      break;
   case nir_op_unpack_64_2x32_split_x:
      result = split_64bit(bld_base, src[0], false);
      break;
   case nir_op_unpack_64_2x32_split_y:
      result = split_64bit(bld_base, src[0], true);
      break;

   case nir_op_pack_32_2x16_split: {
      LLVMValueRef tmp = merge_16bit(bld_base, src[0], src[1]);
      result = LLVMBuildBitCast(builder, tmp, bld_base->base.vec_type, "");
      break;
   }
   case nir_op_unpack_32_2x16_split_x:
      result = split_16bit(bld_base, src[0], false);
      break;
   case nir_op_unpack_32_2x16_split_y:
      result = split_16bit(bld_base, src[0], true);
      break;
   case nir_op_pack_64_2x32_split: {
      LLVMValueRef tmp = merge_64bit(bld_base, src[0], src[1]);
      result = LLVMBuildBitCast(builder, tmp, bld_base->uint64_bld.vec_type, "");
      break;
   }
   case nir_op_pack_32_4x8_split: {
      LLVMValueRef tmp1 = merge_16bit(bld_base, src[0], src[1]);
      LLVMValueRef tmp2 = merge_16bit(bld_base, src[2], src[3]);
      tmp1 = LLVMBuildBitCast(builder, tmp1, bld_base->uint16_bld.vec_type, "");
      tmp2 = LLVMBuildBitCast(builder, tmp2, bld_base->uint16_bld.vec_type, "");
      LLVMValueRef tmp = merge_16bit(bld_base, tmp1, tmp2);
      result = LLVMBuildBitCast(builder, tmp, bld_base->uint_bld.vec_type, "");
      break;
   }
   case nir_op_u2f16:
      result = LLVMBuildUIToFP(builder, src[0],
                               bld_base->half_bld.vec_type, "");
      break;
   case nir_op_u2f32:
      result = LLVMBuildUIToFP(builder, src[0], bld_base->base.vec_type, "");
      break;
   case nir_op_u2f64:
      result = LLVMBuildUIToFP(builder, src[0], bld_base->dbl_bld.vec_type, "");
      break;
   case nir_op_u2u8:
      result = LLVMBuildTrunc(builder, src[0], bld_base->uint8_bld.vec_type, "");
      break;
   case nir_op_u2u16:
      if (src_bit_size[0] < 16)
         result = LLVMBuildZExt(builder, src[0], bld_base->uint16_bld.vec_type, "");
      else
         result = LLVMBuildTrunc(builder, src[0], bld_base->uint16_bld.vec_type, "");
      break;
   case nir_op_u2u32:
      if (src_bit_size[0] < 32)
         result = LLVMBuildZExt(builder, src[0], bld_base->uint_bld.vec_type, "");
      else
         result = LLVMBuildTrunc(builder, src[0], bld_base->uint_bld.vec_type, "");
      break;
   case nir_op_u2u64:
      result = LLVMBuildZExt(builder, src[0], bld_base->uint64_bld.vec_type, "");
      break;
   case nir_op_udiv:
      result = do_int_divide(bld_base, true, src_bit_size[0], src[0], src[1]);
      break;
   case nir_op_ufind_msb: {
      struct lp_build_context *uint_bld = get_int_bld(bld_base, true, src_bit_size[0]);
      result = lp_build_ctlz(uint_bld, src[0]);
      result = lp_build_sub(uint_bld, lp_build_const_int_vec(gallivm, uint_bld->type, src_bit_size[0] - 1), result);
      if (src_bit_size[0] < 32)
         result = LLVMBuildZExt(builder, result, bld_base->uint_bld.vec_type, "");
      else
         result = LLVMBuildTrunc(builder, result, bld_base->uint_bld.vec_type, "");
      break;
   }
   case nir_op_uge32:
      result = icmp32(bld_base, PIPE_FUNC_GEQUAL, true, src_bit_size[0], src);
      break;
   case nir_op_ult32:
      result = icmp32(bld_base, PIPE_FUNC_LESS, true, src_bit_size[0], src);
      break;
   case nir_op_umax:
      result = lp_build_max(get_int_bld(bld_base, true, src_bit_size[0]), src[0], src[1]);
      break;
   case nir_op_umin:
      result = lp_build_min(get_int_bld(bld_base, true, src_bit_size[0]), src[0], src[1]);
      break;
   case nir_op_umod:
      result = do_int_mod(bld_base, true, src_bit_size[0], src[0], src[1]);
      break;
   case nir_op_umul_high: {
      LLVMValueRef hi_bits;
      lp_build_mul_32_lohi(get_int_bld(bld_base, true, src_bit_size[0]), src[0], src[1], &hi_bits);
      result = hi_bits;
      break;
   }
   case nir_op_ushr: {
      struct lp_build_context *uint_bld = get_int_bld(bld_base, true, src_bit_size[0]);
      if (src_bit_size[0] == 64)
         src[1] = LLVMBuildZExt(builder, src[1], uint_bld->vec_type, "");
      if (src_bit_size[0] < 32)
         src[1] = LLVMBuildTrunc(builder, src[1], uint_bld->vec_type, "");
      src[1] = lp_build_and(uint_bld, src[1], lp_build_const_int_vec(gallivm, uint_bld->type, (src_bit_size[0] - 1)));
      result = lp_build_shr(uint_bld, src[0], src[1]);
      break;
   }
   case nir_op_bcsel: {
      LLVMTypeRef src1_type = LLVMTypeOf(src[1]);
      LLVMTypeRef src2_type = LLVMTypeOf(src[2]);

      if (LLVMGetTypeKind(src1_type) == LLVMPointerTypeKind &&
          LLVMGetTypeKind(src2_type) != LLVMPointerTypeKind) {
         src[2] = LLVMBuildIntToPtr(builder, src[2], src1_type, "");
      } else if (LLVMGetTypeKind(src2_type) == LLVMPointerTypeKind &&
                 LLVMGetTypeKind(src1_type) != LLVMPointerTypeKind) {
         src[1] = LLVMBuildIntToPtr(builder, src[1], src2_type, "");
      }

      for (int i = 1; i <= 2; i++) {
         LLVMTypeRef type = LLVMTypeOf(src[i]);
         if (LLVMGetTypeKind(type) == LLVMPointerTypeKind)
            break;
         src[i] = LLVMBuildBitCast(builder, src[i], get_int_bld(bld_base, true, src_bit_size[i])->vec_type, "");
      }
      return LLVMBuildSelect(builder, src[0], src[1], src[2], "");
   }
   default:
      assert(0);
      break;
   }
   return result;
}


static void
visit_alu(struct lp_build_nir_context *bld_base,
          const nir_alu_instr *instr)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMValueRef src[NIR_MAX_VEC_COMPONENTS];
   unsigned src_bit_size[NIR_MAX_VEC_COMPONENTS];
   const unsigned num_components = instr->def.num_components;
   unsigned src_components;

   switch (instr->op) {
   case nir_op_vec2:
   case nir_op_vec3:
   case nir_op_vec4:
   case nir_op_vec8:
   case nir_op_vec16:
      src_components = 1;
      break;
   case nir_op_pack_half_2x16:
      src_components = 2;
      break;
   case nir_op_unpack_half_2x16:
      src_components = 1;
      break;
   case nir_op_cube_amd:
      src_components = 3;
      break;
   case nir_op_fsum2:
   case nir_op_fsum3:
   case nir_op_fsum4:
      src_components = nir_op_infos[instr->op].input_sizes[0];
      break;
   default:
      src_components = num_components;
      break;
   }

   for (unsigned i = 0; i < nir_op_infos[instr->op].num_inputs; i++) {
      src[i] = get_alu_src(bld_base, instr->src[i], src_components);
      src_bit_size[i] = nir_src_bit_size(instr->src[i].src);
   }

   LLVMValueRef result[NIR_MAX_VEC_COMPONENTS];
   if (instr->op == nir_op_vec4 ||
       instr->op == nir_op_vec3 ||
       instr->op == nir_op_vec2 ||
       instr->op == nir_op_vec8 ||
       instr->op == nir_op_vec16) {
      for (unsigned i = 0; i < nir_op_infos[instr->op].num_inputs; i++) {
         result[i] = cast_type(bld_base, src[i],
                               nir_op_infos[instr->op].input_types[i],
                               src_bit_size[i]);
      }
   } else if (instr->op == nir_op_fsum4 ||
              instr->op == nir_op_fsum3 ||
              instr->op == nir_op_fsum2) {
      for (unsigned c = 0; c < nir_op_infos[instr->op].input_sizes[0]; c++) {
         LLVMValueRef temp_chan = LLVMBuildExtractValue(gallivm->builder,
                                                          src[0], c, "");
         temp_chan = cast_type(bld_base, temp_chan,
                               nir_op_infos[instr->op].input_types[0],
                               src_bit_size[0]);
         result[0] = (c == 0) ? temp_chan
            : lp_build_add(get_flt_bld(bld_base, src_bit_size[0]),
                           result[0], temp_chan);
      }
   } else if (is_aos(bld_base)) {
      result[0] = do_alu_action(bld_base, instr, src_bit_size, src);
   } else {
      /* Loop for R,G,B,A channels */
      for (unsigned c = 0; c < num_components; c++) {
         LLVMValueRef src_chan[NIR_MAX_VEC_COMPONENTS];

         /* Loop over instruction operands */
         for (unsigned i = 0; i < nir_op_infos[instr->op].num_inputs; i++) {
            if (num_components > 1) {
               src_chan[i] = LLVMBuildExtractValue(gallivm->builder,
                                                     src[i], c, "");
            } else {
               src_chan[i] = src[i];
            }
            src_chan[i] = cast_type(bld_base, src_chan[i],
                                    nir_op_infos[instr->op].input_types[i],
                                    src_bit_size[i]);
         }
         result[c] = do_alu_action(bld_base, instr, src_bit_size, src_chan);
         result[c] = cast_type(bld_base, result[c],
                               nir_op_infos[instr->op].output_type,
                               instr->def.bit_size);
      }
   }
   assign_ssa_dest(bld_base, &instr->def, result);
}


static void
visit_load_const(struct lp_build_nir_context *bld_base,
                 const nir_load_const_instr *instr)
{
   LLVMValueRef result[NIR_MAX_VEC_COMPONENTS];
   bld_base->load_const(bld_base, instr, result);
   assign_ssa_dest(bld_base, &instr->def, result);
}


static void
get_deref_offset(struct lp_build_nir_context *bld_base, nir_deref_instr *instr,
                 bool vs_in, unsigned *vertex_index_out,
                 LLVMValueRef *vertex_index_ref,
                 unsigned *const_out, LLVMValueRef *indir_out)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   nir_variable *var = nir_deref_instr_get_variable(instr);
   nir_deref_path path;
   unsigned idx_lvl = 1;

   nir_deref_path_init(&path, instr, NULL);

   if (vertex_index_out != NULL || vertex_index_ref != NULL) {
      if (vertex_index_ref) {
         *vertex_index_ref = get_src(bld_base, path.path[idx_lvl]->arr.index);
         if (vertex_index_out)
            *vertex_index_out = 0;
      } else {
         *vertex_index_out = nir_src_as_uint(path.path[idx_lvl]->arr.index);
      }
      ++idx_lvl;
   }

   uint32_t const_offset = 0;
   LLVMValueRef offset = NULL;

   if (var->data.compact && nir_src_is_const(instr->arr.index)) {
      assert(instr->deref_type == nir_deref_type_array);
      const_offset = nir_src_as_uint(instr->arr.index);
      goto out;
   }

   for (; path.path[idx_lvl]; ++idx_lvl) {
      const struct glsl_type *parent_type = path.path[idx_lvl - 1]->type;
      if (path.path[idx_lvl]->deref_type == nir_deref_type_struct) {
         unsigned index = path.path[idx_lvl]->strct.index;

         for (unsigned i = 0; i < index; i++) {
            const struct glsl_type *ft = glsl_get_struct_field(parent_type, i);
            const_offset += glsl_count_attribute_slots(ft, vs_in);
         }
      } else if (path.path[idx_lvl]->deref_type == nir_deref_type_array) {
         unsigned size = glsl_count_attribute_slots(path.path[idx_lvl]->type, vs_in);
         if (nir_src_is_const(path.path[idx_lvl]->arr.index)) {
            const_offset += nir_src_comp_as_int(path.path[idx_lvl]->arr.index, 0) * size;
         } else {
            LLVMValueRef idx_src = get_src(bld_base, path.path[idx_lvl]->arr.index);
            idx_src = cast_type(bld_base, idx_src, nir_type_uint, 32);
            LLVMValueRef array_off = lp_build_mul(&bld_base->uint_bld, lp_build_const_int_vec(bld_base->base.gallivm, bld_base->base.type, size),
                                                  idx_src);
            if (offset)
               offset = lp_build_add(&bld_base->uint_bld, offset, array_off);
            else
               offset = array_off;
         }
      } else
         unreachable("Uhandled deref type in get_deref_instr_offset");
   }

out:
   nir_deref_path_finish(&path);

   if (const_offset && offset)
      offset = LLVMBuildAdd(builder, offset,
                            lp_build_const_int_vec(bld_base->base.gallivm, bld_base->uint_bld.type, const_offset),
                            "");
   *const_out = const_offset;
   *indir_out = offset;
}


static void
visit_load_input(struct lp_build_nir_context *bld_base,
                 nir_intrinsic_instr *instr,
                 LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   nir_variable var = {0};
   var.data.location = nir_intrinsic_io_semantics(instr).location;
   var.data.driver_location = nir_intrinsic_base(instr);
   var.data.location_frac = nir_intrinsic_component(instr);

   unsigned nc = instr->def.num_components;
   unsigned bit_size = instr->def.bit_size;

   nir_src offset = *nir_get_io_offset_src(instr);
   bool indirect = !nir_src_is_const(offset);
   if (!indirect)
      assert(nir_src_as_uint(offset) == 0);
   LLVMValueRef indir_index = indirect ? get_src(bld_base, offset) : NULL;

   bld_base->load_var(bld_base, nir_var_shader_in, nc, bit_size, &var, 0, NULL, 0, indir_index, result);
}


static void
visit_store_output(struct lp_build_nir_context *bld_base,
                   nir_intrinsic_instr *instr)
{
   nir_variable var = {0};
   var.data.location = nir_intrinsic_io_semantics(instr).location;
   var.data.driver_location = nir_intrinsic_base(instr);
   var.data.location_frac = nir_intrinsic_component(instr);

   unsigned mask = nir_intrinsic_write_mask(instr);

   unsigned bit_size = nir_src_bit_size(instr->src[0]);
   LLVMValueRef src = get_src(bld_base, instr->src[0]);

   nir_src offset = *nir_get_io_offset_src(instr);
   bool indirect = !nir_src_is_const(offset);
   if (!indirect)
      assert(nir_src_as_uint(offset) == 0);
   LLVMValueRef indir_index = indirect ? get_src(bld_base, offset) : NULL;

   if (mask == 0x1 && LLVMGetTypeKind(LLVMTypeOf(src)) == LLVMArrayTypeKind) {
      src = LLVMBuildExtractValue(bld_base->base.gallivm->builder,
                                  src, 0, "");
   }

   bld_base->store_var(bld_base, nir_var_shader_out, util_last_bit(mask),
                       bit_size, &var, mask, NULL, 0, indir_index, src);
}


static void
visit_load_reg(struct lp_build_nir_context *bld_base,
               nir_intrinsic_instr *instr,
               LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;

   nir_intrinsic_instr *decl = nir_reg_get_decl(instr->src[0].ssa);
   unsigned base = nir_intrinsic_base(instr);

   struct hash_entry *entry = _mesa_hash_table_search(bld_base->regs, decl);
   LLVMValueRef reg_storage = (LLVMValueRef)entry->data;

   unsigned bit_size = nir_intrinsic_bit_size(decl);
   struct lp_build_context *reg_bld = get_int_bld(bld_base, true, bit_size);

   LLVMValueRef indir_src = NULL;
   if (instr->intrinsic == nir_intrinsic_load_reg_indirect) {
      indir_src = cast_type(bld_base, get_src(bld_base, instr->src[1]),
                            nir_type_uint, 32);
   }

   LLVMValueRef val = bld_base->load_reg(bld_base, reg_bld, decl, base, indir_src, reg_storage);

   if (!is_aos(bld_base) && instr->def.num_components > 1) {
      for (unsigned i = 0; i < instr->def.num_components; i++)
         result[i] = LLVMBuildExtractValue(builder, val, i, "");
   } else {
      result[0] = val;
   }
}


static void
visit_store_reg(struct lp_build_nir_context *bld_base,
                nir_intrinsic_instr *instr)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;

   nir_intrinsic_instr *decl = nir_reg_get_decl(instr->src[1].ssa);
   unsigned base = nir_intrinsic_base(instr);
   unsigned write_mask = nir_intrinsic_write_mask(instr);
   assert(write_mask != 0x0);

   LLVMValueRef val = get_src(bld_base, instr->src[0]);
   LLVMValueRef vals[NIR_MAX_VEC_COMPONENTS] = { NULL };
   if (!is_aos(bld_base) && nir_src_num_components(instr->src[0]) > 1) {
      for (unsigned i = 0; i < nir_src_num_components(instr->src[0]); i++)
         vals[i] = LLVMBuildExtractValue(builder, val, i, "");
   } else {
      vals[0] = val;
   }

   struct hash_entry *entry = _mesa_hash_table_search(bld_base->regs, decl);
   LLVMValueRef reg_storage = (LLVMValueRef)entry->data;

   unsigned bit_size = nir_intrinsic_bit_size(decl);
   struct lp_build_context *reg_bld = get_int_bld(bld_base, true, bit_size);

   LLVMValueRef indir_src = NULL;
   if (instr->intrinsic == nir_intrinsic_store_reg_indirect) {
      indir_src = cast_type(bld_base, get_src(bld_base, instr->src[2]),
                            nir_type_uint, 32);
   }

   bld_base->store_reg(bld_base, reg_bld, decl, write_mask, base,
                       indir_src, reg_storage, vals);
}


static bool
compact_array_index_oob(struct lp_build_nir_context *bld_base, nir_variable *var, const uint32_t index)
{
   const struct glsl_type *type = var->type;
   if (nir_is_arrayed_io(var, bld_base->shader->info.stage)) {
      assert(glsl_type_is_array(type));
      type = glsl_get_array_element(type);
   }
   return index >= glsl_get_length(type);
}

static void
visit_load_var(struct lp_build_nir_context *bld_base,
               nir_intrinsic_instr *instr,
               LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   nir_deref_instr *deref = nir_instr_as_deref(instr->src[0].ssa->parent_instr);
   nir_variable *var = nir_deref_instr_get_variable(deref);
   assert(util_bitcount(deref->modes) == 1);
   nir_variable_mode mode = deref->modes;
   unsigned const_index = 0;
   LLVMValueRef indir_index = NULL;
   LLVMValueRef indir_vertex_index = NULL;
   unsigned vertex_index = 0;
   unsigned nc = instr->def.num_components;
   unsigned bit_size = instr->def.bit_size;
   if (var) {
      bool vs_in = bld_base->shader->info.stage == MESA_SHADER_VERTEX &&
         var->data.mode == nir_var_shader_in;
      bool gs_in = bld_base->shader->info.stage == MESA_SHADER_GEOMETRY &&
         var->data.mode == nir_var_shader_in;
      bool tcs_in = bld_base->shader->info.stage == MESA_SHADER_TESS_CTRL &&
         var->data.mode == nir_var_shader_in;
      bool tcs_out = bld_base->shader->info.stage == MESA_SHADER_TESS_CTRL &&
         var->data.mode == nir_var_shader_out && !var->data.patch;
      bool tes_in = bld_base->shader->info.stage == MESA_SHADER_TESS_EVAL &&
         var->data.mode == nir_var_shader_in && !var->data.patch;

      mode = var->data.mode;

      get_deref_offset(bld_base, deref, vs_in,
                   gs_in ? &vertex_index : NULL,
                   (tcs_in || tcs_out || tes_in) ? &indir_vertex_index : NULL,
                   &const_index, &indir_index);

      /* Return undef for loads definitely outside of the array bounds
       * (tcs-tes-levels-out-of-bounds-read.shader_test).
       */
      if (var->data.compact && compact_array_index_oob(bld_base, var, const_index)) {
         struct lp_build_context *undef_bld = get_int_bld(bld_base, true,
                                                          instr->def.bit_size);
         for (int i = 0; i < instr->def.num_components; i++)
            result[i] = LLVMGetUndef(undef_bld->vec_type);
         return;
      }
   }
   bld_base->load_var(bld_base, mode, nc, bit_size, var, vertex_index,
                      indir_vertex_index, const_index, indir_index, result);
}


static void
visit_store_var(struct lp_build_nir_context *bld_base,
                nir_intrinsic_instr *instr)
{
   nir_deref_instr *deref = nir_instr_as_deref(instr->src[0].ssa->parent_instr);
   nir_variable *var = nir_deref_instr_get_variable(deref);
   assert(util_bitcount(deref->modes) == 1);
   nir_variable_mode mode = deref->modes;
   int writemask = instr->const_index[0];
   unsigned bit_size = nir_src_bit_size(instr->src[1]);
   LLVMValueRef src = get_src(bld_base, instr->src[1]);
   unsigned const_index = 0;
   LLVMValueRef indir_index = NULL, indir_vertex_index = NULL;
   if (var) {
      bool tcs_out = bld_base->shader->info.stage == MESA_SHADER_TESS_CTRL &&
         var->data.mode == nir_var_shader_out && !var->data.patch;
      bool mesh_out = bld_base->shader->info.stage == MESA_SHADER_MESH &&
         var->data.mode == nir_var_shader_out;
      get_deref_offset(bld_base, deref, false, NULL,
                       (tcs_out || mesh_out) ? &indir_vertex_index : NULL,
                       &const_index, &indir_index);

      /* Skip stores definitely outside of the array bounds
       * (tcs-tes-levels-out-of-bounds-write.shader_test).
       */
      if (var->data.compact && compact_array_index_oob(bld_base, var, const_index))
         return;
   }
   bld_base->store_var(bld_base, mode, instr->num_components, bit_size,
                       var, writemask, indir_vertex_index, const_index,
                       indir_index, src);
}


static void
visit_load_ubo(struct lp_build_nir_context *bld_base,
               nir_intrinsic_instr *instr,
               LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef idx = get_src(bld_base, instr->src[0]);
   LLVMValueRef offset = get_src(bld_base, instr->src[1]);

   bool offset_is_uniform = nir_src_is_always_uniform(instr->src[1]);

   if (nir_src_num_components(instr->src[0]) == 1)
      idx = LLVMBuildExtractElement(builder, idx, lp_build_const_int32(gallivm, 0), "");

   bld_base->load_ubo(bld_base, instr->def.num_components,
                      instr->def.bit_size,
                      offset_is_uniform, idx, offset, result);
}


static void
visit_load_push_constant(struct lp_build_nir_context *bld_base,
                         nir_intrinsic_instr *instr,
                         LLVMValueRef result[4])
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMValueRef offset = get_src(bld_base, instr->src[0]);
   LLVMValueRef idx = lp_build_const_int32(gallivm, 0);
   bool offset_is_uniform = nir_src_is_always_uniform(instr->src[0]);

   bld_base->load_ubo(bld_base, instr->def.num_components,
                      instr->def.bit_size,
                      offset_is_uniform, idx, offset, result);
}


static void
visit_load_ssbo(struct lp_build_nir_context *bld_base,
                nir_intrinsic_instr *instr,
                LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef idx = get_src(bld_base, instr->src[0]);
   if (nir_src_num_components(instr->src[0]) == 1)
      idx = cast_type(bld_base, idx, nir_type_uint, 32);

   LLVMValueRef offset = get_src(bld_base, instr->src[1]);
   bool index_and_offset_are_uniform =
      nir_src_is_always_uniform(instr->src[0]) &&
      nir_src_is_always_uniform(instr->src[1]);
   bld_base->load_mem(bld_base, instr->def.num_components,
                      instr->def.bit_size,
                      index_and_offset_are_uniform, false, idx, offset, result);
}


static void
visit_store_ssbo(struct lp_build_nir_context *bld_base,
                 nir_intrinsic_instr *instr)
{
   LLVMValueRef val = get_src(bld_base, instr->src[0]);

   LLVMValueRef idx = get_src(bld_base, instr->src[1]);
   if (nir_src_num_components(instr->src[1]) == 1)
      idx = cast_type(bld_base, idx, nir_type_uint, 32);

   LLVMValueRef offset = get_src(bld_base, instr->src[2]);
   bool index_and_offset_are_uniform =
      nir_src_is_always_uniform(instr->src[1]) &&
      nir_src_is_always_uniform(instr->src[2]);
   int writemask = instr->const_index[0];
   int nc = nir_src_num_components(instr->src[0]);
   int bitsize = nir_src_bit_size(instr->src[0]);
   bld_base->store_mem(bld_base, writemask, nc, bitsize,
                       index_and_offset_are_uniform, false, idx, offset, val);
}


static void
visit_get_ssbo_size(struct lp_build_nir_context *bld_base,
                    nir_intrinsic_instr *instr,
                    LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef idx = get_src(bld_base, instr->src[0]);
   if (nir_src_num_components(instr->src[0]) == 1)
      idx = cast_type(bld_base, idx, nir_type_uint, 32);

   result[0] = bld_base->get_ssbo_size(bld_base, idx);
}


static void
visit_ssbo_atomic(struct lp_build_nir_context *bld_base,
                  nir_intrinsic_instr *instr,
                  LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef idx = get_src(bld_base, instr->src[0]);
   if (nir_src_num_components(instr->src[0]) == 1)
      idx = cast_type(bld_base, idx, nir_type_uint, 32);

   LLVMValueRef offset = get_src(bld_base, instr->src[1]);
   LLVMValueRef val = get_src(bld_base, instr->src[2]);
   LLVMValueRef val2 = NULL;
   int bitsize = nir_src_bit_size(instr->src[2]);
   if (instr->intrinsic == nir_intrinsic_ssbo_atomic_swap)
      val2 = get_src(bld_base, instr->src[3]);

   bld_base->atomic_mem(bld_base, nir_intrinsic_atomic_op(instr), bitsize, false, idx,
                        offset, val, val2, &result[0]);
}

static void
img_params_init_resource(struct lp_build_nir_context *bld_base, struct lp_img_params *params, nir_src src)
{
   if (nir_src_num_components(src) == 1) {
      if (nir_src_is_const(src))
         params->image_index = nir_src_as_int(src);
      else
         params->image_index_offset = get_src(bld_base, src);
   
      return;
   }

   params->resource = get_src(bld_base, src);
}

static void
sampler_size_params_init_resource(struct lp_build_nir_context *bld_base, struct lp_sampler_size_query_params *params, nir_src src)
{
   if (nir_src_num_components(src) == 1) {
      if (nir_src_is_const(src))
         params->texture_unit = nir_src_as_int(src);
      else
         params->texture_unit_offset = get_src(bld_base, src);
   
      return;
   }

   params->resource = get_src(bld_base, src);
}

static void
visit_load_image(struct lp_build_nir_context *bld_base,
                 nir_intrinsic_instr *instr,
                 LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef coord_val = get_src(bld_base, instr->src[1]);
   LLVMValueRef coords[5];
   struct lp_img_params params = { 0 };

   params.target = glsl_sampler_to_pipe(nir_intrinsic_image_dim(instr),
                                        nir_intrinsic_image_array(instr));
   for (unsigned i = 0; i < 4; i++)
      coords[i] = LLVMBuildExtractValue(builder, coord_val, i, "");
   if (params.target == PIPE_TEXTURE_1D_ARRAY)
      coords[2] = coords[1];

   params.coords = coords;
   params.outdata = result;
   params.img_op = LP_IMG_LOAD;
   if (nir_intrinsic_image_dim(instr) == GLSL_SAMPLER_DIM_MS ||
       nir_intrinsic_image_dim(instr) == GLSL_SAMPLER_DIM_SUBPASS_MS)
      params.ms_index = cast_type(bld_base, get_src(bld_base, instr->src[2]),
                                  nir_type_uint, 32);

   img_params_init_resource(bld_base, &params, instr->src[0]);
   params.format = nir_intrinsic_format(instr);

   bld_base->image_op(bld_base, &params);
}


static void
visit_store_image(struct lp_build_nir_context *bld_base,
                  nir_intrinsic_instr *instr)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef coord_val = get_src(bld_base, instr->src[1]);
   LLVMValueRef in_val = get_src(bld_base, instr->src[3]);
   LLVMValueRef coords[5];
   struct lp_img_params params = { 0 };

   params.target = glsl_sampler_to_pipe(nir_intrinsic_image_dim(instr), nir_intrinsic_image_array(instr));
   for (unsigned i = 0; i < 4; i++)
      coords[i] = LLVMBuildExtractValue(builder, coord_val, i, "");
   if (params.target == PIPE_TEXTURE_1D_ARRAY)
      coords[2] = coords[1];
   params.coords = coords;

   params.format = nir_intrinsic_format(instr);

   const struct util_format_description *desc = util_format_description(params.format);
   bool integer = desc->channel[util_format_get_first_non_void_channel(params.format)].pure_integer;

   for (unsigned i = 0; i < 4; i++) {
      params.indata[i] = LLVMBuildExtractValue(builder, in_val, i, "");

      if (integer)
         params.indata[i] = LLVMBuildBitCast(builder, params.indata[i], bld_base->int_bld.vec_type, "");
      else
         params.indata[i] = LLVMBuildBitCast(builder, params.indata[i], bld_base->base.vec_type, "");
   }
   if (nir_intrinsic_image_dim(instr) == GLSL_SAMPLER_DIM_MS)
      params.ms_index = get_src(bld_base, instr->src[2]);
   params.img_op = LP_IMG_STORE;

   img_params_init_resource(bld_base, &params, instr->src[0]);

   if (params.target == PIPE_TEXTURE_1D_ARRAY)
      coords[2] = coords[1];
   bld_base->image_op(bld_base, &params);
}

LLVMAtomicRMWBinOp
lp_translate_atomic_op(nir_atomic_op op)
{
   switch (op) {
   case nir_atomic_op_iadd: return LLVMAtomicRMWBinOpAdd;
   case nir_atomic_op_xchg: return LLVMAtomicRMWBinOpXchg;
   case nir_atomic_op_iand: return LLVMAtomicRMWBinOpAnd;
   case nir_atomic_op_ior:  return LLVMAtomicRMWBinOpOr;
   case nir_atomic_op_ixor: return LLVMAtomicRMWBinOpXor;
   case nir_atomic_op_umin: return LLVMAtomicRMWBinOpUMin;
   case nir_atomic_op_umax: return LLVMAtomicRMWBinOpUMax;
   case nir_atomic_op_imin: return LLVMAtomicRMWBinOpMin;
   case nir_atomic_op_imax: return LLVMAtomicRMWBinOpMax;
   case nir_atomic_op_fadd: return LLVMAtomicRMWBinOpFAdd;
#if LLVM_VERSION_MAJOR >= 15
   case nir_atomic_op_fmin: return LLVMAtomicRMWBinOpFMin;
   case nir_atomic_op_fmax: return LLVMAtomicRMWBinOpFMax;
#endif
   default:          unreachable("Unexpected atomic");
   }
}

void
lp_img_op_from_intrinsic(struct lp_img_params *params, nir_intrinsic_instr *instr)
{
   if (instr->intrinsic == nir_intrinsic_image_load ||
       instr->intrinsic == nir_intrinsic_bindless_image_load) {
      params->img_op = LP_IMG_LOAD;
      return;
   }

   if (instr->intrinsic == nir_intrinsic_image_store ||
       instr->intrinsic == nir_intrinsic_bindless_image_store) {
      params->img_op = LP_IMG_STORE;
      return;
   }

   if (instr->intrinsic == nir_intrinsic_image_atomic_swap ||
       instr->intrinsic == nir_intrinsic_bindless_image_atomic_swap) {
      params->img_op = LP_IMG_ATOMIC_CAS;
      return;
   }

   if (instr->intrinsic == nir_intrinsic_image_atomic ||
       instr->intrinsic == nir_intrinsic_bindless_image_atomic) {
      params->img_op = LP_IMG_ATOMIC;
      params->op = lp_translate_atomic_op(nir_intrinsic_atomic_op(instr));
   } else {
      params->img_op = -1;
   }
}


static void
visit_atomic_image(struct lp_build_nir_context *bld_base,
                   nir_intrinsic_instr *instr,
                   LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_img_params params = { 0 };
   LLVMValueRef coord_val = get_src(bld_base, instr->src[1]);
   LLVMValueRef in_val = get_src(bld_base, instr->src[3]);
   LLVMValueRef coords[5];

   params.target = glsl_sampler_to_pipe(nir_intrinsic_image_dim(instr),
                                        nir_intrinsic_image_array(instr));
   for (unsigned i = 0; i < 4; i++) {
      coords[i] = LLVMBuildExtractValue(builder, coord_val, i, "");
   }
   if (params.target == PIPE_TEXTURE_1D_ARRAY) {
      coords[2] = coords[1];
   }

   params.coords = coords;

   params.format = nir_intrinsic_format(instr);

   const struct util_format_description *desc = util_format_description(params.format);
   bool integer = desc->channel[util_format_get_first_non_void_channel(params.format)].pure_integer;

   if (nir_intrinsic_image_dim(instr) == GLSL_SAMPLER_DIM_MS)
      params.ms_index = get_src(bld_base, instr->src[2]);

   if (instr->intrinsic == nir_intrinsic_image_atomic_swap ||
       instr->intrinsic == nir_intrinsic_bindless_image_atomic_swap) {
      LLVMValueRef cas_val = get_src(bld_base, instr->src[4]);
      params.indata[0] = in_val;
      params.indata2[0] = cas_val;

      if (integer)
         params.indata2[0] = LLVMBuildBitCast(builder, params.indata2[0], bld_base->int_bld.vec_type, "");
      else
         params.indata2[0] = LLVMBuildBitCast(builder, params.indata2[0], bld_base->base.vec_type, "");
   } else {
      params.indata[0] = in_val;
   }

   if (integer)
      params.indata[0] = LLVMBuildBitCast(builder, params.indata[0], bld_base->int_bld.vec_type, "");
   else
      params.indata[0] = LLVMBuildBitCast(builder, params.indata[0], bld_base->base.vec_type, "");

   params.outdata = result;

   lp_img_op_from_intrinsic(&params, instr);

   img_params_init_resource(bld_base, &params, instr->src[0]);

   bld_base->image_op(bld_base, &params);
}


static void
visit_image_size(struct lp_build_nir_context *bld_base,
                 nir_intrinsic_instr *instr,
                 LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   struct lp_sampler_size_query_params params = { 0 };

   sampler_size_params_init_resource(bld_base, &params, instr->src[0]);

   params.target = glsl_sampler_to_pipe(nir_intrinsic_image_dim(instr),
                                        nir_intrinsic_image_array(instr));
   params.sizes_out = result;
   params.ms = nir_intrinsic_image_dim(instr) == GLSL_SAMPLER_DIM_MS ||
      nir_intrinsic_image_dim(instr) == GLSL_SAMPLER_DIM_SUBPASS_MS;
   params.format = nir_intrinsic_format(instr);

   bld_base->image_size(bld_base, &params);
}


static void
visit_image_samples(struct lp_build_nir_context *bld_base,
                    nir_intrinsic_instr *instr,
                    LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   struct lp_sampler_size_query_params params = { 0 };

   sampler_size_params_init_resource(bld_base, &params, instr->src[0]);

   params.target = glsl_sampler_to_pipe(nir_intrinsic_image_dim(instr),
                                        nir_intrinsic_image_array(instr));
   params.sizes_out = result;
   params.ms = nir_intrinsic_image_dim(instr) == GLSL_SAMPLER_DIM_MS ||
      nir_intrinsic_image_dim(instr) == GLSL_SAMPLER_DIM_SUBPASS_MS;
   params.samples_only = true;

   params.format = nir_intrinsic_format(instr);

   bld_base->image_size(bld_base, &params);
}


static void
visit_shared_load(struct lp_build_nir_context *bld_base,
                  nir_intrinsic_instr *instr,
                  LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef offset = get_src(bld_base, instr->src[0]);
   bool offset_is_uniform = nir_src_is_always_uniform(instr->src[0]);
   bld_base->load_mem(bld_base, instr->def.num_components,
                      instr->def.bit_size,
                      offset_is_uniform, false, NULL, offset, result);
}


static void
visit_shared_store(struct lp_build_nir_context *bld_base,
                   nir_intrinsic_instr *instr)
{
   LLVMValueRef val = get_src(bld_base, instr->src[0]);
   LLVMValueRef offset = get_src(bld_base, instr->src[1]);
   bool offset_is_uniform = nir_src_is_always_uniform(instr->src[1]);
   int writemask = instr->const_index[1];
   int nc = nir_src_num_components(instr->src[0]);
   int bitsize = nir_src_bit_size(instr->src[0]);
   bld_base->store_mem(bld_base, writemask, nc, bitsize,
                       offset_is_uniform, false, NULL, offset, val);
}


static void
visit_shared_atomic(struct lp_build_nir_context *bld_base,
                    nir_intrinsic_instr *instr,
                    LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef offset = get_src(bld_base, instr->src[0]);
   LLVMValueRef val = get_src(bld_base, instr->src[1]);
   LLVMValueRef val2 = NULL;
   int bitsize = nir_src_bit_size(instr->src[1]);
   if (instr->intrinsic == nir_intrinsic_shared_atomic_swap)
      val2 = get_src(bld_base, instr->src[2]);

   bld_base->atomic_mem(bld_base, nir_intrinsic_atomic_op(instr), bitsize, false, NULL,
                        offset, val, val2, &result[0]);
}


static void
visit_barrier(struct lp_build_nir_context *bld_base,
              nir_intrinsic_instr *instr)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   mesa_scope exec_scope = nir_intrinsic_execution_scope(instr);
   unsigned nir_semantics = nir_intrinsic_memory_semantics(instr);

   if (nir_semantics) {
      LLVMAtomicOrdering ordering = LLVMAtomicOrderingSequentiallyConsistent;
      LLVMBuildFence(builder, ordering, false, "");
   }
   if (exec_scope != SCOPE_NONE)
      bld_base->barrier(bld_base);
}


static void
visit_discard(struct lp_build_nir_context *bld_base,
              nir_intrinsic_instr *instr)
{
   LLVMValueRef cond = NULL;
   if (instr->intrinsic == nir_intrinsic_discard_if) {
      cond = get_src(bld_base, instr->src[0]);
      cond = cast_type(bld_base, cond, nir_type_int, 32);
   }
   bld_base->discard(bld_base, cond);
}


static void
visit_load_kernel_input(struct lp_build_nir_context *bld_base,
                        nir_intrinsic_instr *instr,
                        LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef offset = get_src(bld_base, instr->src[0]);

   bool offset_is_uniform = nir_src_is_always_uniform(instr->src[0]);
   bld_base->load_kernel_arg(bld_base, instr->def.num_components,
                             instr->def.bit_size,
                             nir_src_bit_size(instr->src[0]),
                             offset_is_uniform, offset, result);
}


static void
visit_load_global(struct lp_build_nir_context *bld_base,
                  nir_intrinsic_instr *instr,
                  LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef addr = get_src(bld_base, instr->src[0]);
   bool offset_is_uniform = nir_src_is_always_uniform(instr->src[0]);
   bld_base->load_global(bld_base, instr->def.num_components,
                         instr->def.bit_size,
                         nir_src_bit_size(instr->src[0]),
                         offset_is_uniform, addr, result);
}


static void
visit_store_global(struct lp_build_nir_context *bld_base,
                   nir_intrinsic_instr *instr)
{
   LLVMValueRef val = get_src(bld_base, instr->src[0]);
   int nc = nir_src_num_components(instr->src[0]);
   int bitsize = nir_src_bit_size(instr->src[0]);
   LLVMValueRef addr = get_src(bld_base, instr->src[1]);
   int addr_bitsize = nir_src_bit_size(instr->src[1]);
   int writemask = instr->const_index[0];
   bld_base->store_global(bld_base, writemask, nc, bitsize,
                          addr_bitsize, addr, val);
}


static void
visit_global_atomic(struct lp_build_nir_context *bld_base,
                    nir_intrinsic_instr *instr,
                    LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef addr = get_src(bld_base, instr->src[0]);
   LLVMValueRef val = get_src(bld_base, instr->src[1]);
   LLVMValueRef val2 = NULL;
   int addr_bitsize = nir_src_bit_size(instr->src[0]);
   int val_bitsize = nir_src_bit_size(instr->src[1]);
   if (instr->intrinsic == nir_intrinsic_global_atomic_swap)
      val2 = get_src(bld_base, instr->src[2]);

   bld_base->atomic_global(bld_base, nir_intrinsic_atomic_op(instr),
                           addr_bitsize, val_bitsize, addr, val, val2,
                           &result[0]);
}

#if LLVM_VERSION_MAJOR >= 10
static void visit_shuffle(struct lp_build_nir_context *bld_base,
                          nir_intrinsic_instr *instr,
                          LLVMValueRef dst[4])
{
   LLVMValueRef src = get_src(bld_base, instr->src[0]);
   src = cast_type(bld_base, src, nir_type_int,
                   nir_src_bit_size(instr->src[0]));
   LLVMValueRef index = get_src(bld_base, instr->src[1]);
   index = cast_type(bld_base, index, nir_type_uint,
                     nir_src_bit_size(instr->src[1]));

   bld_base->shuffle(bld_base, src, index, instr, dst);
}
#endif


static void
visit_interp(struct lp_build_nir_context *bld_base,
             nir_intrinsic_instr *instr,
             LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   nir_deref_instr *deref = nir_instr_as_deref(instr->src[0].ssa->parent_instr);
   unsigned num_components = instr->def.num_components;
   nir_variable *var = nir_deref_instr_get_variable(deref);
   unsigned const_index;
   LLVMValueRef indir_index;
   LLVMValueRef offsets[2] = { NULL, NULL };
   get_deref_offset(bld_base, deref, false, NULL, NULL,
                    &const_index, &indir_index);
   bool centroid = instr->intrinsic == nir_intrinsic_interp_deref_at_centroid;
   bool sample = false;
   if (instr->intrinsic == nir_intrinsic_interp_deref_at_offset) {
      for (unsigned i = 0; i < 2; i++) {
         offsets[i] = LLVMBuildExtractValue(builder, get_src(bld_base, instr->src[1]), i, "");
         offsets[i] = cast_type(bld_base, offsets[i], nir_type_float, 32);
      }
   } else if (instr->intrinsic == nir_intrinsic_interp_deref_at_sample) {
      offsets[0] = get_src(bld_base, instr->src[1]);
      offsets[0] = cast_type(bld_base, offsets[0], nir_type_int, 32);
      sample = true;
   }
   bld_base->interp_at(bld_base, num_components, var, centroid, sample,
                       const_index, indir_index, offsets, result);
}


static void
visit_load_scratch(struct lp_build_nir_context *bld_base,
                   nir_intrinsic_instr *instr,
                   LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef offset = get_src(bld_base, instr->src[0]);

   bld_base->load_scratch(bld_base, instr->def.num_components,
                          instr->def.bit_size, offset, result);
}


static void
visit_store_scratch(struct lp_build_nir_context *bld_base,
                    nir_intrinsic_instr *instr)
{
   LLVMValueRef val = get_src(bld_base, instr->src[0]);
   LLVMValueRef offset = get_src(bld_base, instr->src[1]);
   int writemask = instr->const_index[2];
   int nc = nir_src_num_components(instr->src[0]);
   int bitsize = nir_src_bit_size(instr->src[0]);
   bld_base->store_scratch(bld_base, writemask, nc, bitsize, offset, val);
}

static void
visit_payload_load(struct lp_build_nir_context *bld_base,
                  nir_intrinsic_instr *instr,
                  LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef offset = get_src(bld_base, instr->src[0]);
   bool offset_is_uniform = nir_src_is_always_uniform(instr->src[0]);
   bld_base->load_mem(bld_base, instr->def.num_components,
                      instr->def.bit_size,
                      offset_is_uniform, true, NULL, offset, result);
}

static void
visit_payload_store(struct lp_build_nir_context *bld_base,
                    nir_intrinsic_instr *instr)
{
   LLVMValueRef val = get_src(bld_base, instr->src[0]);
   LLVMValueRef offset = get_src(bld_base, instr->src[1]);
   bool offset_is_uniform = nir_src_is_always_uniform(instr->src[1]);
   int writemask = instr->const_index[1];
   int nc = nir_src_num_components(instr->src[0]);
   int bitsize = nir_src_bit_size(instr->src[0]);
   bld_base->store_mem(bld_base, writemask, nc, bitsize,
                       offset_is_uniform, true, NULL, offset, val);
}

static void
visit_payload_atomic(struct lp_build_nir_context *bld_base,
                     nir_intrinsic_instr *instr,
                     LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef offset = get_src(bld_base, instr->src[0]);
   LLVMValueRef val = get_src(bld_base, instr->src[1]);
   LLVMValueRef val2 = NULL;
   int bitsize = nir_src_bit_size(instr->src[1]);
   if (instr->intrinsic == nir_intrinsic_task_payload_atomic_swap)
      val2 = get_src(bld_base, instr->src[2]);

   bld_base->atomic_mem(bld_base, nir_intrinsic_atomic_op(instr), bitsize, true, NULL,
                        offset, val, val2, &result[0]);
}

static void visit_load_param(struct lp_build_nir_context *bld_base,
                             nir_intrinsic_instr *instr,
                             LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   LLVMValueRef param = LLVMGetParam(bld_base->func, nir_intrinsic_param_idx(instr) + LP_RESV_FUNC_ARGS);
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   if (instr->num_components == 1)
      result[0] = param;
   else {
      for (unsigned i = 0; i < instr->num_components; i++)
         result[i] = LLVMBuildExtractValue(gallivm->builder, param, i, "");
   }
}

static void
visit_intrinsic(struct lp_build_nir_context *bld_base,
                nir_intrinsic_instr *instr)
{
   LLVMValueRef result[NIR_MAX_VEC_COMPONENTS] = {0};
   switch (instr->intrinsic) {
   case nir_intrinsic_decl_reg:
      /* already handled */
      break;
   case nir_intrinsic_load_reg:
   case nir_intrinsic_load_reg_indirect:
      visit_load_reg(bld_base, instr, result);
      break;
   case nir_intrinsic_store_reg:
   case nir_intrinsic_store_reg_indirect:
      visit_store_reg(bld_base, instr);
      break;
   case nir_intrinsic_load_input:
      visit_load_input(bld_base, instr, result);
      break;
   case nir_intrinsic_store_output:
      visit_store_output(bld_base, instr);
      break;
   case nir_intrinsic_load_deref:
      visit_load_var(bld_base, instr, result);
      break;
   case nir_intrinsic_store_deref:
      visit_store_var(bld_base, instr);
      break;
   case nir_intrinsic_load_ubo:
      visit_load_ubo(bld_base, instr, result);
      break;
   case nir_intrinsic_load_push_constant:
      visit_load_push_constant(bld_base, instr, result);
      break;
   case nir_intrinsic_load_ssbo:
      visit_load_ssbo(bld_base, instr, result);
      break;
   case nir_intrinsic_store_ssbo:
      visit_store_ssbo(bld_base, instr);
      break;
   case nir_intrinsic_get_ssbo_size:
      visit_get_ssbo_size(bld_base, instr, result);
      break;
   case nir_intrinsic_load_vertex_id:
   case nir_intrinsic_load_primitive_id:
   case nir_intrinsic_load_instance_id:
   case nir_intrinsic_load_base_instance:
   case nir_intrinsic_load_base_vertex:
   case nir_intrinsic_load_first_vertex:
   case nir_intrinsic_load_workgroup_id:
   case nir_intrinsic_load_local_invocation_id:
   case nir_intrinsic_load_local_invocation_index:
   case nir_intrinsic_load_num_workgroups:
   case nir_intrinsic_load_invocation_id:
   case nir_intrinsic_load_front_face:
   case nir_intrinsic_load_draw_id:
   case nir_intrinsic_load_workgroup_size:
   case nir_intrinsic_load_work_dim:
   case nir_intrinsic_load_tess_coord:
   case nir_intrinsic_load_tess_level_outer:
   case nir_intrinsic_load_tess_level_inner:
   case nir_intrinsic_load_patch_vertices_in:
   case nir_intrinsic_load_sample_id:
   case nir_intrinsic_load_sample_pos:
   case nir_intrinsic_load_sample_mask_in:
   case nir_intrinsic_load_view_index:
   case nir_intrinsic_load_subgroup_invocation:
   case nir_intrinsic_load_subgroup_id:
   case nir_intrinsic_load_num_subgroups:
      bld_base->sysval_intrin(bld_base, instr, result);
      break;
   case nir_intrinsic_load_helper_invocation:
      bld_base->helper_invocation(bld_base, &result[0]);
      break;
   case nir_intrinsic_discard_if:
   case nir_intrinsic_discard:
      visit_discard(bld_base, instr);
      break;
   case nir_intrinsic_emit_vertex:
      bld_base->emit_vertex(bld_base, nir_intrinsic_stream_id(instr));
      break;
   case nir_intrinsic_end_primitive:
      bld_base->end_primitive(bld_base, nir_intrinsic_stream_id(instr));
      break;
   case nir_intrinsic_ssbo_atomic:
   case nir_intrinsic_ssbo_atomic_swap:
      visit_ssbo_atomic(bld_base, instr, result);
      break;
   case nir_intrinsic_image_load:
   case nir_intrinsic_bindless_image_load:
      visit_load_image(bld_base, instr, result);
      break;
   case nir_intrinsic_image_store:
   case nir_intrinsic_bindless_image_store:
      visit_store_image(bld_base, instr);
      break;
   case nir_intrinsic_image_atomic:
   case nir_intrinsic_image_atomic_swap:
   case nir_intrinsic_bindless_image_atomic:
   case nir_intrinsic_bindless_image_atomic_swap:
      visit_atomic_image(bld_base, instr, result);
      break;
   case nir_intrinsic_image_size:
   case nir_intrinsic_bindless_image_size:
      visit_image_size(bld_base, instr, result);
      break;
   case nir_intrinsic_image_samples:
   case nir_intrinsic_bindless_image_samples:
      visit_image_samples(bld_base, instr, result);
      break;
   case nir_intrinsic_load_shared:
      visit_shared_load(bld_base, instr, result);
      break;
   case nir_intrinsic_store_shared:
      visit_shared_store(bld_base, instr);
      break;
   case nir_intrinsic_shared_atomic:
   case nir_intrinsic_shared_atomic_swap:
      visit_shared_atomic(bld_base, instr, result);
      break;
   case nir_intrinsic_barrier:
      visit_barrier(bld_base, instr);
      break;
   case nir_intrinsic_load_kernel_input:
      visit_load_kernel_input(bld_base, instr, result);
     break;
   case nir_intrinsic_load_global:
   case nir_intrinsic_load_global_constant:
      visit_load_global(bld_base, instr, result);
      break;
   case nir_intrinsic_store_global:
      visit_store_global(bld_base, instr);
      break;
   case nir_intrinsic_global_atomic:
   case nir_intrinsic_global_atomic_swap:
      visit_global_atomic(bld_base, instr, result);
      break;
   case nir_intrinsic_vote_all:
   case nir_intrinsic_vote_any:
   case nir_intrinsic_vote_ieq:
   case nir_intrinsic_vote_feq:
      bld_base->vote(bld_base, cast_type(bld_base, get_src(bld_base, instr->src[0]), nir_type_int, nir_src_bit_size(instr->src[0])), instr, result);
      break;
   case nir_intrinsic_elect:
      bld_base->elect(bld_base, result);
      break;
   case nir_intrinsic_reduce:
   case nir_intrinsic_inclusive_scan:
   case nir_intrinsic_exclusive_scan:
      bld_base->reduce(bld_base, cast_type(bld_base, get_src(bld_base, instr->src[0]), nir_type_int, nir_src_bit_size(instr->src[0])), instr, result);
      break;
   case nir_intrinsic_ballot:
      bld_base->ballot(bld_base, cast_type(bld_base, get_src(bld_base, instr->src[0]), nir_type_int, 32), instr, result);
      break;
#if LLVM_VERSION_MAJOR >= 10
   case nir_intrinsic_shuffle:
      visit_shuffle(bld_base, instr, result);
      break;
#endif
   case nir_intrinsic_read_invocation:
   case nir_intrinsic_read_first_invocation: {
      LLVMValueRef src0 = get_src(bld_base, instr->src[0]);
      src0 = cast_type(bld_base, src0, nir_type_int, nir_src_bit_size(instr->src[0]));

      LLVMValueRef src1 = NULL;
      if (instr->intrinsic == nir_intrinsic_read_invocation)
         src1 = cast_type(bld_base, get_src(bld_base, instr->src[1]), nir_type_int, 32);

      bld_base->read_invocation(bld_base, src0, nir_src_bit_size(instr->src[0]), src1, result);
      break;
   }
   case nir_intrinsic_interp_deref_at_offset:
   case nir_intrinsic_interp_deref_at_centroid:
   case nir_intrinsic_interp_deref_at_sample:
      visit_interp(bld_base, instr, result);
      break;
   case nir_intrinsic_load_scratch:
      visit_load_scratch(bld_base, instr, result);
      break;
   case nir_intrinsic_store_scratch:
      visit_store_scratch(bld_base, instr);
      break;
   case nir_intrinsic_shader_clock:
      bld_base->clock(bld_base, result);
      break;
   case nir_intrinsic_launch_mesh_workgroups:
      bld_base->launch_mesh_workgroups(bld_base,
                                       get_src(bld_base, instr->src[0]));
      break;
   case nir_intrinsic_load_task_payload:
      visit_payload_load(bld_base, instr, result);
      break;
   case nir_intrinsic_store_task_payload:
      visit_payload_store(bld_base, instr);
      break;
   case nir_intrinsic_task_payload_atomic:
   case nir_intrinsic_task_payload_atomic_swap:
      visit_payload_atomic(bld_base, instr, result);
      break;
   case nir_intrinsic_set_vertex_and_primitive_count:
      bld_base->set_vertex_and_primitive_count(bld_base,
                                               get_src(bld_base, instr->src[0]),
                                               get_src(bld_base, instr->src[1]));
      break;
   case nir_intrinsic_load_param:
      visit_load_param(bld_base, instr, result);
      break;
   default:
      fprintf(stderr, "Unsupported intrinsic: ");
      nir_print_instr(&instr->instr, stderr);
      fprintf(stderr, "\n");
      assert(0);
      break;
   }
   if (result[0]) {
      assign_ssa_dest(bld_base, &instr->def, result);
   }
}


static void
visit_txs(struct lp_build_nir_context *bld_base, nir_tex_instr *instr)
{
   struct lp_sampler_size_query_params params = { 0 };
   LLVMValueRef sizes_out[NIR_MAX_VEC_COMPONENTS];
   LLVMValueRef explicit_lod = NULL;
   LLVMValueRef texture_unit_offset = NULL;
   LLVMValueRef resource = NULL;

   for (unsigned i = 0; i < instr->num_srcs; i++) {
      switch (instr->src[i].src_type) {
      case nir_tex_src_lod:
         explicit_lod = cast_type(bld_base,
                                  get_src(bld_base, instr->src[i].src),
                                  nir_type_int, 32);
         break;
      case nir_tex_src_texture_offset:
         texture_unit_offset = get_src(bld_base, instr->src[i].src);
         break;
      case nir_tex_src_texture_handle:
         resource = get_src(bld_base, instr->src[i].src);
         break;
      default:
         break;
      }
   }

   params.target = glsl_sampler_to_pipe(instr->sampler_dim, instr->is_array);
   params.texture_unit = instr->texture_index;
   params.explicit_lod = explicit_lod;
   params.is_sviewinfo = true;
   params.sizes_out = sizes_out;
   params.samples_only = (instr->op == nir_texop_texture_samples);
   params.texture_unit_offset = texture_unit_offset;
   params.ms = instr->sampler_dim == GLSL_SAMPLER_DIM_MS ||
      instr->sampler_dim == GLSL_SAMPLER_DIM_SUBPASS_MS;

   if (instr->op == nir_texop_query_levels)
      params.explicit_lod = bld_base->uint_bld.zero;

   params.resource = resource;

   bld_base->tex_size(bld_base, &params);
   assign_ssa_dest(bld_base, &instr->def,
                   &sizes_out[instr->op == nir_texop_query_levels ? 3 : 0]);
}


static enum lp_sampler_lod_property
lp_build_nir_lod_property(gl_shader_stage stage, nir_src lod_src)
{
   enum lp_sampler_lod_property lod_property;

   if (nir_src_is_always_uniform(lod_src)) {
      lod_property = LP_SAMPLER_LOD_SCALAR;
   } else if (stage == MESA_SHADER_FRAGMENT) {
      if (gallivm_perf & GALLIVM_PERF_NO_QUAD_LOD)
         lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
      else
         lod_property = LP_SAMPLER_LOD_PER_QUAD;
   } else {
      lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
   }
   return lod_property;
}


uint32_t
lp_build_nir_sample_key(gl_shader_stage stage, nir_tex_instr *instr)
{
   uint32_t sample_key = 0;

   if (instr->op == nir_texop_txf ||
       instr->op == nir_texop_txf_ms) {
      sample_key |= LP_SAMPLER_OP_FETCH << LP_SAMPLER_OP_TYPE_SHIFT;
   } else if (instr->op == nir_texop_tg4) {
      sample_key |= LP_SAMPLER_OP_GATHER << LP_SAMPLER_OP_TYPE_SHIFT;
      sample_key |= (instr->component << LP_SAMPLER_GATHER_COMP_SHIFT);
   } else if (instr->op == nir_texop_lod) {
      sample_key |= LP_SAMPLER_OP_LODQ << LP_SAMPLER_OP_TYPE_SHIFT;
   }

   bool explicit_lod = false;
   uint32_t lod_src = 0;

   for (unsigned i = 0; i < instr->num_srcs; i++) {
      switch (instr->src[i].src_type) {
      case nir_tex_src_comparator:
         sample_key |= LP_SAMPLER_SHADOW;
         break;
      case nir_tex_src_bias:
         sample_key |= LP_SAMPLER_LOD_BIAS << LP_SAMPLER_LOD_CONTROL_SHIFT;
         explicit_lod = true;
         lod_src = i;
         break;
      case nir_tex_src_lod:
         sample_key |= LP_SAMPLER_LOD_EXPLICIT << LP_SAMPLER_LOD_CONTROL_SHIFT;
         explicit_lod = true;
         lod_src = i;
         break;
      case nir_tex_src_offset:
         sample_key |= LP_SAMPLER_OFFSETS;
         break;
      case nir_tex_src_ms_index:
         sample_key |= LP_SAMPLER_FETCH_MS;
         break;
      default:
         break;
      }
   }

   enum lp_sampler_lod_property lod_property = LP_SAMPLER_LOD_SCALAR;
   if (explicit_lod)
      lod_property = lp_build_nir_lod_property(stage, instr->src[lod_src].src);

   if (instr->op == nir_texop_txd) {
      sample_key |= LP_SAMPLER_LOD_DERIVATIVES << LP_SAMPLER_LOD_CONTROL_SHIFT;

      if (stage == MESA_SHADER_FRAGMENT) {
         if (gallivm_perf & GALLIVM_PERF_NO_QUAD_LOD)
            lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
         else
            lod_property = LP_SAMPLER_LOD_PER_QUAD;
      } else
         lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
   }

   sample_key |= lod_property << LP_SAMPLER_LOD_PROPERTY_SHIFT;

   return sample_key;
}


static void
visit_tex(struct lp_build_nir_context *bld_base, nir_tex_instr *instr)
{
   if (instr->op == nir_texop_txs ||
       instr->op == nir_texop_query_levels ||
       instr->op == nir_texop_texture_samples) {
      visit_txs(bld_base, instr);
      return;
   }

   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef coords[5];
   LLVMValueRef offsets[3] = { NULL };
   LLVMValueRef explicit_lod = NULL, ms_index = NULL;
   struct lp_sampler_params params = { 0 };
   struct lp_derivatives derivs;
   nir_deref_instr *texture_deref_instr = NULL;
   nir_deref_instr *sampler_deref_instr = NULL;
   LLVMValueRef texture_unit_offset = NULL;
   LLVMValueRef texel[NIR_MAX_VEC_COMPONENTS];
   LLVMValueRef coord_undef = LLVMGetUndef(bld_base->base.vec_type);
   unsigned coord_vals = is_aos(bld_base) ? 1 : instr->coord_components;

   LLVMValueRef texture_resource = NULL;
   LLVMValueRef sampler_resource = NULL;

   for (unsigned i = 0; i < instr->num_srcs; i++) {
      switch (instr->src[i].src_type) {
      case nir_tex_src_coord: {
         LLVMValueRef coord = get_src(bld_base, instr->src[i].src);
         if (coord_vals == 1) {
            coords[0] = coord;
         } else {
            for (unsigned chan = 0; chan < instr->coord_components; ++chan)
               coords[chan] = LLVMBuildExtractValue(builder, coord,
                                                    chan, "");
         }
         for (unsigned chan = coord_vals; chan < 5; chan++) {
            coords[chan] = coord_undef;
         }
         break;
      }
      case nir_tex_src_texture_deref:
         texture_deref_instr = nir_src_as_deref(instr->src[i].src);
         break;
      case nir_tex_src_sampler_deref:
         sampler_deref_instr = nir_src_as_deref(instr->src[i].src);
         break;
      case nir_tex_src_comparator:
         coords[4] = get_src(bld_base, instr->src[i].src);
         coords[4] = cast_type(bld_base, coords[4], nir_type_float, 32);
         break;
      case nir_tex_src_bias:
         explicit_lod = cast_type(bld_base, get_src(bld_base, instr->src[i].src), nir_type_float, 32);
         break;
      case nir_tex_src_lod:
         if (instr->op == nir_texop_txf)
            explicit_lod = cast_type(bld_base, get_src(bld_base, instr->src[i].src), nir_type_int, 32);
         else
            explicit_lod = cast_type(bld_base, get_src(bld_base, instr->src[i].src), nir_type_float, 32);
         break;
      case nir_tex_src_ddx: {
         int deriv_cnt = instr->coord_components;
         if (instr->is_array)
            deriv_cnt--;
         LLVMValueRef deriv_val = get_src(bld_base, instr->src[i].src);
         if (deriv_cnt == 1)
            derivs.ddx[0] = deriv_val;
         else
            for (unsigned chan = 0; chan < deriv_cnt; ++chan)
               derivs.ddx[chan] = LLVMBuildExtractValue(builder, deriv_val,
                                                        chan, "");
         for (unsigned chan = 0; chan < deriv_cnt; ++chan)
            derivs.ddx[chan] = cast_type(bld_base, derivs.ddx[chan], nir_type_float, 32);
         break;
      }
      case nir_tex_src_ddy: {
         int deriv_cnt = instr->coord_components;
         if (instr->is_array)
            deriv_cnt--;
         LLVMValueRef deriv_val = get_src(bld_base, instr->src[i].src);
         if (deriv_cnt == 1)
            derivs.ddy[0] = deriv_val;
         else
            for (unsigned chan = 0; chan < deriv_cnt; ++chan)
               derivs.ddy[chan] = LLVMBuildExtractValue(builder, deriv_val,
                                                        chan, "");
         for (unsigned chan = 0; chan < deriv_cnt; ++chan)
            derivs.ddy[chan] = cast_type(bld_base, derivs.ddy[chan], nir_type_float, 32);
         break;
      }
      case nir_tex_src_offset: {
         int offset_cnt = instr->coord_components;
         if (instr->is_array)
            offset_cnt--;
         LLVMValueRef offset_val = get_src(bld_base, instr->src[i].src);
         if (offset_cnt == 1)
            offsets[0] = cast_type(bld_base, offset_val, nir_type_int, 32);
         else {
            for (unsigned chan = 0; chan < offset_cnt; ++chan) {
               offsets[chan] = LLVMBuildExtractValue(builder, offset_val,
                                                     chan, "");
               offsets[chan] = cast_type(bld_base, offsets[chan], nir_type_int, 32);
            }
         }
         break;
      }
      case nir_tex_src_ms_index:
         ms_index = cast_type(bld_base, get_src(bld_base, instr->src[i].src), nir_type_int, 32);
         break;

      case nir_tex_src_texture_offset:
         texture_unit_offset = get_src(bld_base, instr->src[i].src);
         break;
      case nir_tex_src_sampler_offset:
         break;
      case nir_tex_src_texture_handle:
         texture_resource = get_src(bld_base, instr->src[i].src);
         break;
      case nir_tex_src_sampler_handle:
         sampler_resource = get_src(bld_base, instr->src[i].src);
         break;
      case nir_tex_src_plane:
         assert(nir_src_is_const(instr->src[i].src) && !nir_src_as_uint(instr->src[i].src));
         break;
      default:
         assert(0);
         break;
      }
   }
   if (!sampler_deref_instr)
      sampler_deref_instr = texture_deref_instr;

   if (!sampler_resource)
      sampler_resource = texture_resource;

   switch (instr->op) {
   case nir_texop_tex:
   case nir_texop_tg4:
   case nir_texop_txb:
   case nir_texop_txl:
   case nir_texop_txd:
   case nir_texop_lod:
      for (unsigned chan = 0; chan < coord_vals; ++chan)
         coords[chan] = cast_type(bld_base, coords[chan], nir_type_float, 32);
      break;
   case nir_texop_txf:
   case nir_texop_txf_ms:
      for (unsigned chan = 0; chan < instr->coord_components; ++chan)
         coords[chan] = cast_type(bld_base, coords[chan], nir_type_int, 32);
      break;
   default:
      ;
   }

   if (instr->is_array && instr->sampler_dim == GLSL_SAMPLER_DIM_1D) {
      /* move layer coord for 1d arrays. */
      coords[2] = coords[1];
      coords[1] = coord_undef;
   }

   uint32_t samp_base_index = 0, tex_base_index = 0;
   if (!sampler_deref_instr) {
      int samp_src_index = nir_tex_instr_src_index(instr, nir_tex_src_sampler_handle);
      if (samp_src_index == -1) {
         samp_base_index = instr->sampler_index;
      }
   }
   if (!texture_deref_instr) {
      int tex_src_index = nir_tex_instr_src_index(instr, nir_tex_src_texture_handle);
      if (tex_src_index == -1) {
         tex_base_index = instr->texture_index;
      }
   }

   if (instr->op == nir_texop_txd)
      params.derivs = &derivs;

   params.sample_key = lp_build_nir_sample_key(bld_base->shader->info.stage, instr);
   params.offsets = offsets;
   params.texture_index = tex_base_index;
   params.texture_index_offset = texture_unit_offset;
   params.sampler_index = samp_base_index;
   params.coords = coords;
   params.texel = texel;
   params.lod = explicit_lod;
   params.ms_index = ms_index;
   params.aniso_filter_table = bld_base->aniso_filter_table;
   params.texture_resource = texture_resource;
   params.sampler_resource = sampler_resource;
   bld_base->tex(bld_base, &params);

   if (instr->def.bit_size != 32) {
      assert(instr->def.bit_size == 16);
      LLVMTypeRef vec_type = NULL;
      bool is_float = false;
      switch (nir_alu_type_get_base_type(instr->dest_type)) {
      case nir_type_float:
         is_float = true;
         break;
      case nir_type_int:
         vec_type = bld_base->int16_bld.vec_type;
         break;
      case nir_type_uint:
         vec_type = bld_base->uint16_bld.vec_type;
         break;
      default:
         unreachable("unexpected alu type");
      }
      for (int i = 0; i < instr->def.num_components; ++i) {
         if (is_float) {
            texel[i] = lp_build_float_to_half(gallivm, texel[i]);
         } else {
            texel[i] = LLVMBuildBitCast(builder, texel[i], bld_base->int_bld.vec_type, "");
            texel[i] = LLVMBuildTrunc(builder, texel[i], vec_type, "");
         }
      }
   }

   assign_ssa_dest(bld_base, &instr->def, texel);
}


static void
visit_ssa_undef(struct lp_build_nir_context *bld_base,
                const nir_undef_instr *instr)
{
   unsigned num_components = instr->def.num_components;
   LLVMValueRef undef[NIR_MAX_VEC_COMPONENTS];
   struct lp_build_context *undef_bld = get_int_bld(bld_base, true,
                                                    instr->def.bit_size);
   for (unsigned i = 0; i < num_components; i++)
      undef[i] = LLVMGetUndef(undef_bld->vec_type);
   memset(&undef[num_components], 0, NIR_MAX_VEC_COMPONENTS - num_components);
   assign_ssa_dest(bld_base, &instr->def, undef);
}


static void
visit_jump(struct lp_build_nir_context *bld_base,
           const nir_jump_instr *instr)
{
   switch (instr->type) {
   case nir_jump_break:
      bld_base->break_stmt(bld_base);
      break;
   case nir_jump_continue:
      bld_base->continue_stmt(bld_base);
      break;
   default:
      unreachable("Unknown jump instr\n");
   }
}


static void
visit_deref(struct lp_build_nir_context *bld_base,
            nir_deref_instr *instr)
{
   if (!nir_deref_mode_is_one_of(instr, nir_var_mem_shared |
                                        nir_var_mem_global)) {
      return;
   }

   LLVMValueRef result = NULL;
   switch(instr->deref_type) {
   case nir_deref_type_var: {
      struct hash_entry *entry =
         _mesa_hash_table_search(bld_base->vars, instr->var);
      result = entry->data;
      break;
   }
   default:
      unreachable("Unhandled deref_instr deref type");
   }

   assign_ssa(bld_base, instr->def.index, result);
}

static void
visit_call(struct lp_build_nir_context *bld_base,
           nir_call_instr *instr)
{
   LLVMValueRef *args;
   struct hash_entry *entry = _mesa_hash_table_search(bld_base->fns, instr->callee);
   struct lp_build_fn *fn = entry->data;
   args = calloc(instr->num_params + LP_RESV_FUNC_ARGS, sizeof(LLVMValueRef));

   assert(args);

   args[0] = 0;
   for (unsigned i = 0; i < instr->num_params; i++) {
      LLVMValueRef arg = get_src(bld_base, instr->params[i]);

      if (nir_src_bit_size(instr->params[i]) == 32 && LLVMTypeOf(arg) == bld_base->base.vec_type)
         arg = cast_type(bld_base, arg, nir_type_int, 32);
      args[i + LP_RESV_FUNC_ARGS] = arg;
   }

   bld_base->call(bld_base, fn, instr->num_params + LP_RESV_FUNC_ARGS, args);
   free(args);
}

static void
visit_block(struct lp_build_nir_context *bld_base, nir_block *block)
{
   nir_foreach_instr(instr, block)
   {
      switch (instr->type) {
      case nir_instr_type_alu:
         visit_alu(bld_base, nir_instr_as_alu(instr));
         break;
      case nir_instr_type_load_const:
         visit_load_const(bld_base, nir_instr_as_load_const(instr));
         break;
      case nir_instr_type_intrinsic:
         visit_intrinsic(bld_base, nir_instr_as_intrinsic(instr));
         break;
      case nir_instr_type_tex:
         visit_tex(bld_base, nir_instr_as_tex(instr));
         break;
      case nir_instr_type_phi:
         assert(0);
         break;
      case nir_instr_type_undef:
         visit_ssa_undef(bld_base, nir_instr_as_undef(instr));
         break;
      case nir_instr_type_jump:
         visit_jump(bld_base, nir_instr_as_jump(instr));
         break;
      case nir_instr_type_deref:
         visit_deref(bld_base, nir_instr_as_deref(instr));
         break;
      case nir_instr_type_call:
         visit_call(bld_base, nir_instr_as_call(instr));
         break;
      default:
         fprintf(stderr, "Unknown NIR instr type: ");
         nir_print_instr(instr, stderr);
         fprintf(stderr, "\n");
         abort();
      }
   }
}


static void
visit_if(struct lp_build_nir_context *bld_base, nir_if *if_stmt)
{
   LLVMValueRef cond = get_src(bld_base, if_stmt->condition);

   bld_base->if_cond(bld_base, cond);
   visit_cf_list(bld_base, &if_stmt->then_list);

   if (!exec_list_is_empty(&if_stmt->else_list)) {
      bld_base->else_stmt(bld_base);
      visit_cf_list(bld_base, &if_stmt->else_list);
   }
   bld_base->endif_stmt(bld_base);
}


static void
visit_loop(struct lp_build_nir_context *bld_base, nir_loop *loop)
{
   assert(!nir_loop_has_continue_construct(loop));
   bld_base->bgnloop(bld_base);
   visit_cf_list(bld_base, &loop->body);
   bld_base->endloop(bld_base);
}


static void
visit_cf_list(struct lp_build_nir_context *bld_base,
              struct exec_list *list)
{
   foreach_list_typed(nir_cf_node, node, node, list)
   {
      switch (node->type) {
      case nir_cf_node_block:
         visit_block(bld_base, nir_cf_node_as_block(node));
         break;
      case nir_cf_node_if:
         visit_if(bld_base, nir_cf_node_as_if(node));
         break;
      case nir_cf_node_loop:
         visit_loop(bld_base, nir_cf_node_as_loop(node));
         break;
      default:
         assert(0);
      }
   }
}


static void
handle_shader_output_decl(struct lp_build_nir_context *bld_base,
                          struct nir_shader *nir,
                          struct nir_variable *variable)
{
   bld_base->emit_var_decl(bld_base, variable);
}


/* vector registers are stored as arrays in LLVM side,
   so we can use GEP on them, as to do exec mask stores
   we need to operate on a single components.
   arrays are:
   0.x, 1.x, 2.x, 3.x
   0.y, 1.y, 2.y, 3.y
   ....
*/
static LLVMTypeRef
get_register_type(struct lp_build_nir_context *bld_base,
                  nir_intrinsic_instr *reg)
{
   if (is_aos(bld_base))
      return bld_base->base.int_vec_type;

   unsigned num_array_elems = nir_intrinsic_num_array_elems(reg);
   unsigned bit_size = nir_intrinsic_bit_size(reg);
   unsigned num_components = nir_intrinsic_num_components(reg);

   struct lp_build_context *int_bld =
      get_int_bld(bld_base, true, bit_size == 1 ? 32 : bit_size);

   LLVMTypeRef type = int_bld->vec_type;
   if (num_components > 1)
      type = LLVMArrayType(type, num_components);
   if (num_array_elems)
      type = LLVMArrayType(type, num_array_elems);

   return type;
}

void
lp_build_nir_prepasses(struct nir_shader *nir)
{
   NIR_PASS_V(nir, nir_convert_to_lcssa, true, true);
   NIR_PASS_V(nir, nir_convert_from_ssa, true);
   NIR_PASS_V(nir, nir_lower_locals_to_regs, 32);
   NIR_PASS_V(nir, nir_remove_dead_derefs);
   NIR_PASS_V(nir, nir_remove_dead_variables, nir_var_function_temp, NULL);
}

bool lp_build_nir_llvm(struct lp_build_nir_context *bld_base,
                       struct nir_shader *nir,
                       nir_function_impl *impl)
{
   nir_foreach_shader_out_variable(variable, nir)
      handle_shader_output_decl(bld_base, nir, variable);

   if (nir->info.io_lowered) {
      uint64_t outputs_written = nir->info.outputs_written;

      while (outputs_written) {
         unsigned location = u_bit_scan64(&outputs_written);
         nir_variable var = {0};

         var.type = glsl_vec4_type();
         var.data.mode = nir_var_shader_out;
         var.data.location = location;
         var.data.driver_location = util_bitcount64(nir->info.outputs_written &
                                                    BITFIELD64_MASK(location));
         bld_base->emit_var_decl(bld_base, &var);
      }
   }

   bld_base->regs = _mesa_hash_table_create(NULL, _mesa_hash_pointer,
                                            _mesa_key_pointer_equal);
   bld_base->vars = _mesa_hash_table_create(NULL, _mesa_hash_pointer,
                                            _mesa_key_pointer_equal);
   bld_base->range_ht = _mesa_pointer_hash_table_create(NULL);

   nir_foreach_reg_decl(reg, impl) {
      LLVMTypeRef type = get_register_type(bld_base, reg);
      LLVMValueRef reg_alloc = lp_build_alloca(bld_base->base.gallivm,
                                               type, "reg");
      _mesa_hash_table_insert(bld_base->regs, reg, reg_alloc);
   }
   nir_index_ssa_defs(impl);
   bld_base->ssa_defs = calloc(impl->ssa_alloc, sizeof(LLVMValueRef));
   visit_cf_list(bld_base, &impl->body);

   free(bld_base->ssa_defs);
   ralloc_free(bld_base->vars);
   ralloc_free(bld_base->regs);
   ralloc_free(bld_base->range_ht);
   return true;
}


/* do some basic opts to remove some things we don't want to see. */
void
lp_build_opt_nir(struct nir_shader *nir)
{
   bool progress;

   static const struct nir_lower_tex_options lower_tex_options = {
      .lower_tg4_offsets = true,
      .lower_txp = ~0u,
      .lower_invalid_implicit_lod = true,
   };
   NIR_PASS_V(nir, nir_lower_tex, &lower_tex_options);
   NIR_PASS_V(nir, nir_lower_frexp);

   if (nir->info.stage == MESA_SHADER_TASK) {
      nir_lower_task_shader_options ts_opts = { 0 };
      NIR_PASS_V(nir, nir_lower_task_shader, ts_opts);
   }

   NIR_PASS_V(nir, nir_lower_flrp, 16|32|64, true);
   NIR_PASS_V(nir, nir_lower_fp16_casts, nir_lower_fp16_all | nir_lower_fp16_split_fp64);
   do {
      progress = false;
      NIR_PASS(progress, nir, nir_opt_constant_folding);
      NIR_PASS(progress, nir, nir_opt_algebraic);
      NIR_PASS(progress, nir, nir_lower_pack);

      nir_lower_tex_options options = { .lower_invalid_implicit_lod = true, };
      NIR_PASS_V(nir, nir_lower_tex, &options);

      const nir_lower_subgroups_options subgroups_options = {
         .subgroup_size = lp_native_vector_width / 32,
         .ballot_bit_size = 32,
         .ballot_components = 1,
         .lower_to_scalar = true,
         .lower_subgroup_masks = true,
         .lower_relative_shuffle = true,
         .lower_inverse_ballot = true,
      };
      NIR_PASS(progress, nir, nir_lower_subgroups, &subgroups_options);
   } while (progress);

   do {
      progress = false;
      NIR_PASS(progress, nir, nir_opt_algebraic_late);
      if (progress) {
         NIR_PASS_V(nir, nir_copy_prop);
         NIR_PASS_V(nir, nir_opt_dce);
         NIR_PASS_V(nir, nir_opt_cse);
      }
   } while (progress);

   if (nir_lower_bool_to_int32(nir)) {
      NIR_PASS_V(nir, nir_copy_prop);
      NIR_PASS_V(nir, nir_opt_dce);
   }
}
