/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
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

/**
 * @file
 * Helper functions for logical operations.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#include <llvm/Config/llvm-config.h>

#include "util/u_cpu_detect.h"
#include "util/u_memory.h"
#include "util/u_debug.h"

#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_swizzle.h"
#include "lp_bld_init.h"
#include "lp_bld_intr.h"
#include "lp_bld_debug.h"
#include "lp_bld_logic.h"


/*
 * XXX
 *
 * Selection with vector conditional like
 *
 *    select <4 x i1> %C, %A, %B
 *
 * is valid IR (e.g. llvm/test/Assembler/vector-select.ll), but it is only
 * supported on some backends (x86) starting with llvm 3.1.
 *
 * Expanding the boolean vector to full SIMD register width, as in
 *
 *    sext <4 x i1> %C to <4 x i32>
 *
 * is valid and supported (e.g., llvm/test/CodeGen/X86/vec_compare.ll), but
 * it causes assertion failures in LLVM 2.6. It appears to work correctly on
 * LLVM 2.7.
 */


/**
 * Build code to compare two values 'a' and 'b' of 'type' using the given func.
 * \param func  one of PIPE_FUNC_x
 * If the ordered argument is true the function will use LLVM's ordered
 * comparisons, otherwise unordered comparisons will be used.
 * The result values will be 0 for false or ~0 for true.
 */
static LLVMValueRef
lp_build_compare_ext(struct gallivm_state *gallivm,
                     const struct lp_type type,
                     enum pipe_compare_func func,
                     LLVMValueRef a,
                     LLVMValueRef b,
                     bool ordered)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMTypeRef int_vec_type = lp_build_int_vec_type(gallivm, type);
   LLVMValueRef zeros = LLVMConstNull(int_vec_type);
   LLVMValueRef ones = LLVMConstAllOnes(int_vec_type);
   LLVMValueRef cond;
   LLVMValueRef res;

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   if (func == PIPE_FUNC_NEVER)
      return zeros;
   if (func == PIPE_FUNC_ALWAYS)
      return ones;

   assert(func > PIPE_FUNC_NEVER);
   assert(func < PIPE_FUNC_ALWAYS);

   if (type.floating) {
      LLVMRealPredicate op;
      switch(func) {
      case PIPE_FUNC_EQUAL:
         op = ordered ? LLVMRealOEQ : LLVMRealUEQ;
         break;
      case PIPE_FUNC_NOTEQUAL:
         op = ordered ? LLVMRealONE : LLVMRealUNE;
         break;
      case PIPE_FUNC_LESS:
         op = ordered ? LLVMRealOLT : LLVMRealULT;
         break;
      case PIPE_FUNC_LEQUAL:
         op = ordered ? LLVMRealOLE : LLVMRealULE;
         break;
      case PIPE_FUNC_GREATER:
         op = ordered ? LLVMRealOGT : LLVMRealUGT;
         break;
      case PIPE_FUNC_GEQUAL:
         op = ordered ? LLVMRealOGE : LLVMRealUGE;
         break;
      default:
         assert(0);
         return lp_build_undef(gallivm, type);
      }

      cond = LLVMBuildFCmp(builder, op, a, b, "");
      res = LLVMBuildSExt(builder, cond, int_vec_type, "");
   }
   else {
      LLVMIntPredicate op;
      switch(func) {
      case PIPE_FUNC_EQUAL:
         op = LLVMIntEQ;
         break;
      case PIPE_FUNC_NOTEQUAL:
         op = LLVMIntNE;
         break;
      case PIPE_FUNC_LESS:
         op = type.sign ? LLVMIntSLT : LLVMIntULT;
         break;
      case PIPE_FUNC_LEQUAL:
         op = type.sign ? LLVMIntSLE : LLVMIntULE;
         break;
      case PIPE_FUNC_GREATER:
         op = type.sign ? LLVMIntSGT : LLVMIntUGT;
         break;
      case PIPE_FUNC_GEQUAL:
         op = type.sign ? LLVMIntSGE : LLVMIntUGE;
         break;
      default:
         assert(0);
         return lp_build_undef(gallivm, type);
      }

      cond = LLVMBuildICmp(builder, op, a, b, "");
      res = LLVMBuildSExt(builder, cond, int_vec_type, "");
   }

   return res;
}

/**
 * Build code to compare two values 'a' and 'b' of 'type' using the given func.
 * \param func  one of PIPE_FUNC_x
 * The result values will be 0 for false or ~0 for true.
 */
LLVMValueRef
lp_build_compare(struct gallivm_state *gallivm,
                 const struct lp_type type,
                 enum pipe_compare_func func,
                 LLVMValueRef a,
                 LLVMValueRef b)
{
   LLVMTypeRef int_vec_type = lp_build_int_vec_type(gallivm, type);
   LLVMValueRef zeros = LLVMConstNull(int_vec_type);
   LLVMValueRef ones = LLVMConstAllOnes(int_vec_type);

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   if (func == PIPE_FUNC_NEVER)
      return zeros;
   if (func == PIPE_FUNC_ALWAYS)
      return ones;

   assert(func > PIPE_FUNC_NEVER);
   assert(func < PIPE_FUNC_ALWAYS);

#if DETECT_ARCH_X86 || DETECT_ARCH_X86_64
   /*
    * There are no unsigned integer comparison instructions in SSE.
    */

   if (!type.floating && !type.sign &&
       type.width * type.length == 128 &&
       util_get_cpu_caps()->has_sse2 &&
       (func == PIPE_FUNC_LESS ||
        func == PIPE_FUNC_LEQUAL ||
        func == PIPE_FUNC_GREATER ||
        func == PIPE_FUNC_GEQUAL) &&
       (gallivm_debug & GALLIVM_DEBUG_PERF)) {
         debug_printf("%s: inefficient <%u x i%u> unsigned comparison\n",
                      __func__, type.length, type.width);
   }
#endif

   return lp_build_compare_ext(gallivm, type, func, a, b, false);
}

/**
 * Build code to compare two values 'a' and 'b' using the given func.
 * \param func  one of PIPE_FUNC_x
 * If the operands are floating point numbers, the function will use
 * ordered comparison which means that it will return true if both
 * operands are not a NaN and the specified condition evaluates to true.
 * The result values will be 0 for false or ~0 for true.
 */
LLVMValueRef
lp_build_cmp_ordered(struct lp_build_context *bld,
                     enum pipe_compare_func func,
                     LLVMValueRef a,
                     LLVMValueRef b)
{
   return lp_build_compare_ext(bld->gallivm, bld->type, func, a, b, true);
}

/**
 * Build code to compare two values 'a' and 'b' using the given func.
 * \param func  one of PIPE_FUNC_x
 * If the operands are floating point numbers, the function will use
 * unordered comparison which means that it will return true if either
 * operand is a NaN or the specified condition evaluates to true.
 * The result values will be 0 for false or ~0 for true.
 */
LLVMValueRef
lp_build_cmp(struct lp_build_context *bld,
             enum pipe_compare_func func,
             LLVMValueRef a,
             LLVMValueRef b)
{
   return lp_build_compare(bld->gallivm, bld->type, func, a, b);
}


/**
 * Return (mask & a) | (~mask & b);
 */
LLVMValueRef
lp_build_select_bitwise(struct lp_build_context *bld,
                        LLVMValueRef mask,
                        LLVMValueRef a,
                        LLVMValueRef b)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_type type = bld->type;
   LLVMValueRef res;
   LLVMTypeRef int_vec_type = lp_build_int_vec_type(bld->gallivm, type);

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   if (a == b) {
      return a;
   }

   if (type.floating) {
      a = LLVMBuildBitCast(builder, a, int_vec_type, "");
      b = LLVMBuildBitCast(builder, b, int_vec_type, "");
   }

   if (type.width > 32)
      mask = LLVMBuildSExt(builder, mask, int_vec_type, "");
   a = LLVMBuildAnd(builder, a, mask, "");

   /* This often gets translated to PANDN, but sometimes the NOT is
    * pre-computed and stored in another constant. The best strategy depends
    * on available registers, so it is not a big deal -- hopefully LLVM does
    * the right decision attending the rest of the program.
    */
   b = LLVMBuildAnd(builder, b, LLVMBuildNot(builder, mask, ""), "");

   res = LLVMBuildOr(builder, a, b, "");

   if (type.floating) {
      LLVMTypeRef vec_type = lp_build_vec_type(bld->gallivm, type);
      res = LLVMBuildBitCast(builder, res, vec_type, "");
   }

   return res;
}


/**
 * Return mask ? a : b;
 *
 * mask is a bitwise mask, composed of 0 or ~0 for each element. Any other value
 * will yield unpredictable results.
 */
LLVMValueRef
lp_build_select(struct lp_build_context *bld,
                LLVMValueRef mask,
                LLVMValueRef a,
                LLVMValueRef b)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMContextRef lc = bld->gallivm->context;
   struct lp_type type = bld->type;
   LLVMValueRef res;

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   if (a == b)
      return a;

   if (type.length == 1) {
      mask = LLVMBuildTrunc(builder, mask, LLVMInt1TypeInContext(lc), "");
      res = LLVMBuildSelect(builder, mask, a, b, "");
   }
   else if (LLVMIsConstant(mask) ||
            LLVMGetInstructionOpcode(mask) == LLVMSExt) {
      /* Generate a vector select.
       *
       * Using vector selects should avoid emitting intrinsics hence avoid
       * hindering optimization passes, but vector selects weren't properly
       * supported yet for a long time, and LLVM will generate poor code when
       * the mask is not the result of a comparison.
       * XXX: Even if the instruction was an SExt, this may still produce
       * terrible code. Try piglit stencil-twoside.
       */

      /* Convert the mask to a vector of booleans.
       *
       * XXX: In x86 the mask is controlled by the MSB, so if we shifted the
       * mask by `type.width - 1`, LLVM should realize the mask is ready.  Alas
       * what really happens is that LLVM will emit two shifts back to back.
       */
      if (0) {
         LLVMValueRef shift =
            LLVMConstInt(bld->int_elem_type, bld->type.width - 1, 0);
         shift = lp_build_broadcast(bld->gallivm, bld->int_vec_type, shift);
         mask = LLVMBuildLShr(builder, mask, shift, "");
      }
      LLVMTypeRef bool_vec_type =
         LLVMVectorType(LLVMInt1TypeInContext(lc), type.length);
      mask = LLVMBuildTrunc(builder, mask, bool_vec_type, "");

      res = LLVMBuildSelect(builder, mask, a, b, "");
   }
   else if (((util_get_cpu_caps()->has_sse4_1 &&
              type.width * type.length == 128) ||
             (util_get_cpu_caps()->has_avx &&
              type.width * type.length == 256 && type.width >= 32) ||
             (util_get_cpu_caps()->has_avx2 &&
              type.width * type.length == 256)) &&
            !LLVMIsConstant(a) &&
            !LLVMIsConstant(b) &&
            !LLVMIsConstant(mask)) {
      const char *intrinsic;
      LLVMTypeRef arg_type;
      LLVMValueRef args[3];

      LLVMTypeRef mask_type = LLVMGetElementType(LLVMTypeOf(mask));
      if (LLVMGetIntTypeWidth(mask_type) != type.width) {
         LLVMTypeRef int_vec_type =
            LLVMVectorType(LLVMIntTypeInContext(lc, type.width), type.length);
         mask = LLVMBuildSExt(builder, mask, int_vec_type, "");
      }
      /*
       *  There's only float blend in AVX but can just cast i32/i64
       *  to float.
       */
      if (type.width * type.length == 256) {
         if (type.width == 64) {
           intrinsic = "llvm.x86.avx.blendv.pd.256";
           arg_type = LLVMVectorType(LLVMDoubleTypeInContext(lc), 4);
         }
         else if (type.width == 32) {
            intrinsic = "llvm.x86.avx.blendv.ps.256";
            arg_type = LLVMVectorType(LLVMFloatTypeInContext(lc), 8);
         } else {
            assert(util_get_cpu_caps()->has_avx2);
            intrinsic = "llvm.x86.avx2.pblendvb";
            arg_type = LLVMVectorType(LLVMInt8TypeInContext(lc), 32);
         }
      }
      else if (type.floating &&
               type.width == 64) {
         intrinsic = "llvm.x86.sse41.blendvpd";
         arg_type = LLVMVectorType(LLVMDoubleTypeInContext(lc), 2);
      } else if (type.floating &&
                 type.width == 32) {
         intrinsic = "llvm.x86.sse41.blendvps";
         arg_type = LLVMVectorType(LLVMFloatTypeInContext(lc), 4);
      } else {
         intrinsic = "llvm.x86.sse41.pblendvb";
         arg_type = LLVMVectorType(LLVMInt8TypeInContext(lc), 16);
      }

      if (arg_type != bld->int_vec_type) {
         mask = LLVMBuildBitCast(builder, mask, arg_type, "");
      }

      if (arg_type != bld->vec_type) {
         a = LLVMBuildBitCast(builder, a, arg_type, "");
         b = LLVMBuildBitCast(builder, b, arg_type, "");
      }

      args[0] = b;
      args[1] = a;
      args[2] = mask;

      res = lp_build_intrinsic(builder, intrinsic,
                               arg_type, args, ARRAY_SIZE(args), 0);

      if (arg_type != bld->vec_type) {
         res = LLVMBuildBitCast(builder, res, bld->vec_type, "");
      }
   }
   else {
      res = lp_build_select_bitwise(bld, mask, a, b);
   }

   return res;
}


/**
 * Return mask ? a : b;
 *
 * mask is a TGSI_WRITEMASK_xxx.
 */
LLVMValueRef
lp_build_select_aos(struct lp_build_context *bld,
                    unsigned mask,
                    LLVMValueRef a,
                    LLVMValueRef b,
                    unsigned num_channels)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   const unsigned n = type.length;

   assert((mask & ~0xf) == 0);
   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   if (a == b)
      return a;
   if ((mask & 0xf) == 0xf)
      return a;
   if ((mask & 0xf) == 0x0)
      return b;
   if (a == bld->undef || b == bld->undef)
      return bld->undef;

   /*
    * There are two major ways of accomplishing this:
    * - with a shuffle
    * - with a select
    *
    * The flip between these is empirical and might need to be adjusted.
    */
   if (n <= 4) {
      /*
       * Shuffle.
       */
      LLVMTypeRef elem_type = LLVMInt32TypeInContext(bld->gallivm->context);
      LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH];

      for (unsigned j = 0; j < n; j += num_channels)
         for (unsigned i = 0; i < num_channels; ++i)
            shuffles[j + i] = LLVMConstInt(elem_type,
                                           (mask & (1 << i) ? 0 : n) + j + i,
                                           0);

      return LLVMBuildShuffleVector(builder, a, b,
                                    LLVMConstVector(shuffles, n), "");
   }
   else {
      LLVMValueRef mask_vec = lp_build_const_mask_aos(bld->gallivm,
                                                      type, mask, num_channels);
      return lp_build_select(bld, mask_vec, a, b);
   }
}


/**
 * Return (scalar-cast)val ? true : false;
 */
LLVMValueRef
lp_build_any_true_range(struct lp_build_context *bld,
                        unsigned real_length,
                        LLVMValueRef val)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMTypeRef scalar_type;
   LLVMTypeRef true_type;

   assert(real_length <= bld->type.length);

   true_type = LLVMIntTypeInContext(bld->gallivm->context,
                                    bld->type.width * real_length);
   scalar_type = LLVMIntTypeInContext(bld->gallivm->context,
                                      bld->type.width * bld->type.length);
   val = LLVMBuildBitCast(builder, val, scalar_type, "");
   /*
    * We're using always native types so we can use intrinsics.
    * However, if we don't do per-element calculations, we must ensure
    * the excess elements aren't used since they may contain garbage.
    */
   if (real_length < bld->type.length) {
      val = LLVMBuildTrunc(builder, val, true_type, "");
   }
   return LLVMBuildICmp(builder, LLVMIntNE,
                        val, LLVMConstNull(true_type), "");
}
