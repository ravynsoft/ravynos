/**************************************************************************
 *
 * Copyright 2012 VMware, Inc.
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

#include "pipe/p_state.h"
#include "util/u_debug.h"

#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_arit.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_logic.h"
#include "gallivm/lp_bld_swizzle.h"
#include "gallivm/lp_bld_flow.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_pack.h"

#include "lp_bld_blend.h"

/**
 * Is (a OP b) == (b OP a)?
 */
bool
lp_build_blend_func_commutative(enum pipe_blend_func func)
{
   switch (func) {
   case PIPE_BLEND_ADD:
   case PIPE_BLEND_MIN:
   case PIPE_BLEND_MAX:
      return true;
   case PIPE_BLEND_SUBTRACT:
   case PIPE_BLEND_REVERSE_SUBTRACT:
      return false;
   default:
      assert(0);
      return true;
   }
}


/**
 * Whether the blending functions are the reverse of each other.
 */
bool
lp_build_blend_func_reverse(enum pipe_blend_func rgb_func,
                            enum pipe_blend_func alpha_func)
{
   if (rgb_func == alpha_func)
      return false;
   if (rgb_func == PIPE_BLEND_SUBTRACT && alpha_func == PIPE_BLEND_REVERSE_SUBTRACT)
      return true;
   if (rgb_func == PIPE_BLEND_REVERSE_SUBTRACT && alpha_func == PIPE_BLEND_SUBTRACT)
      return true;
   return false;
}


/**
 * Whether the blending factors are complementary of each other.
 */
static inline bool
lp_build_blend_factor_complementary(unsigned src_factor, unsigned dst_factor)
{
   STATIC_ASSERT((PIPE_BLENDFACTOR_ZERO ^ 0x10) == PIPE_BLENDFACTOR_ONE);
   STATIC_ASSERT((PIPE_BLENDFACTOR_CONST_COLOR ^ 0x10) ==
                 PIPE_BLENDFACTOR_INV_CONST_COLOR);
   return dst_factor == (src_factor ^ 0x10);
}


/**
 * Whether this is a inverse blend factor
 */
static inline bool
is_inverse_factor(unsigned factor)
{
   STATIC_ASSERT(PIPE_BLENDFACTOR_ZERO == 0x11);
   return factor > 0x11;
}


/**
 * Calculates the (expanded to wider type) multiplication
 * of 2 normalized numbers.
 */
static void
lp_build_mul_norm_expand(struct lp_build_context *bld,
                         LLVMValueRef a, LLVMValueRef b,
                         LLVMValueRef *resl, LLVMValueRef *resh,
                         bool signedness_differs)
{
   const struct lp_type type = bld->type;
   struct lp_type wide_type = lp_wider_type(type);
   struct lp_type wide_type2 = wide_type;
   struct lp_type type2 = type;
   LLVMValueRef al, ah, bl, bh;

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));
   assert(!type.floating && !type.fixed && type.norm);

   if (a == bld->zero || b == bld->zero) {
      LLVMValueRef zero = LLVMConstNull(lp_build_vec_type(bld->gallivm, wide_type));
      *resl = zero;
      *resh = zero;
      return;
   }

   if (signedness_differs) {
      type2.sign = !type.sign;
      wide_type2.sign = !wide_type2.sign;
   }

   lp_build_unpack2_native(bld->gallivm, type, wide_type, a, &al, &ah);
   lp_build_unpack2_native(bld->gallivm, type2, wide_type2, b, &bl, &bh);

   *resl = lp_build_mul_norm(bld->gallivm, wide_type, al, bl);
   *resh = lp_build_mul_norm(bld->gallivm, wide_type, ah, bh);
}


/**
 * @sa http://www.opengl.org/sdk/docs/man/xhtml/glBlendEquationSeparate.xml
 */
LLVMValueRef
lp_build_blend_func(struct lp_build_context *bld,
                    enum pipe_blend_func func,
                    LLVMValueRef term1,
                    LLVMValueRef term2)
{
   switch (func) {
   case PIPE_BLEND_ADD:
      return lp_build_add(bld, term1, term2);
   case PIPE_BLEND_SUBTRACT:
      return lp_build_sub(bld, term1, term2);
   case PIPE_BLEND_REVERSE_SUBTRACT:
      return lp_build_sub(bld, term2, term1);
   case PIPE_BLEND_MIN:
      return lp_build_min(bld, term1, term2);
   case PIPE_BLEND_MAX:
      return lp_build_max(bld, term1, term2);
   default:
      assert(0);
      return bld->zero;
   }
}


/**
 * Performs optimizations and blending independent of SoA/AoS
 *
 * @param func                   the blend function
 * @param factor_src             PIPE_BLENDFACTOR_xxx
 * @param factor_dst             PIPE_BLENDFACTOR_xxx
 * @param src                    source rgba
 * @param dst                    dest rgba
 * @param src_factor             src factor computed value
 * @param dst_factor             dst factor computed value
 * @param not_alpha_dependent    same factors accross all channels of src/dst
 *
 * not_alpha_dependent should be:
 *  SoA: always true as it is only one channel at a time
 *  AoS: rgb_src_factor == alpha_src_factor && rgb_dst_factor == alpha_dst_factor
 *
 * Note that pretty much every possible optimisation can only be done on non-unorm targets
 * due to unorm values not going above 1.0 meaning factorisation can change results.
 * e.g. (0.9 * 0.9) + (0.9 * 0.9) != 0.9 * (0.9 + 0.9) as result of + is always <= 1.
 */
LLVMValueRef
lp_build_blend(struct lp_build_context *bld,
               enum pipe_blend_func func,
               enum pipe_blendfactor factor_src,
               enum pipe_blendfactor factor_dst,
               LLVMValueRef src,
               LLVMValueRef dst,
               LLVMValueRef src_factor,
               LLVMValueRef dst_factor,
               bool not_alpha_dependent,
               bool optimise_only)
{
   LLVMValueRef result, src_term, dst_term;

   /* If we are not alpha dependent we can mess with the src/dst factors */
   if (not_alpha_dependent) {
      if (lp_build_blend_factor_complementary(factor_src, factor_dst)) {
         if (func == PIPE_BLEND_ADD) {
            if (factor_src < factor_dst) {
               return lp_build_lerp(bld, src_factor, dst, src, 0);
            } else {
               return lp_build_lerp(bld, dst_factor, src, dst, 0);
            }
         } else if (bld->type.floating && func == PIPE_BLEND_SUBTRACT) {
            result = lp_build_add(bld, src, dst);

            if (factor_src < factor_dst) {
               result = lp_build_mul(bld, result, src_factor);
               return lp_build_sub(bld, result, dst);
            } else {
               result = lp_build_mul(bld, result, dst_factor);
               return lp_build_sub(bld, src, result);
            }
         } else if (bld->type.floating && func == PIPE_BLEND_REVERSE_SUBTRACT) {
            result = lp_build_add(bld, src, dst);

            if (factor_src < factor_dst) {
               result = lp_build_mul(bld, result, src_factor);
               return lp_build_sub(bld, dst, result);
            } else {
               result = lp_build_mul(bld, result, dst_factor);
               return lp_build_sub(bld, result, src);
            }
         }
      }

      if (bld->type.floating && factor_src == factor_dst) {
         if (func == PIPE_BLEND_ADD ||
             func == PIPE_BLEND_SUBTRACT ||
             func == PIPE_BLEND_REVERSE_SUBTRACT) {
            LLVMValueRef result;
            result = lp_build_blend_func(bld, func, src, dst);
            return lp_build_mul(bld, result, src_factor);
         }
      }
   }

   if (optimise_only)
      return NULL;

   if ((bld->type.norm && bld->type.sign) &&
       (is_inverse_factor(factor_src) || is_inverse_factor(factor_dst))) {
      /*
       * With snorm blending, the inverse blend factors range from [0,2]
       * instead of [-1,1], so the ordinary signed normalized arithmetic
       * doesn't quite work. Unpack must be unsigned, and the add/sub
       * must be done with wider type.
       * (Note that it's not quite obvious what the blend equation wrt to
       * clamping should actually be based on GL spec in this case, but
       * really the incoming src values are clamped to [-1,1] (the dst is
       * always clamped already), and then NO further clamping occurs until
       * the end.)
       */
      struct lp_build_context bldw;
      struct lp_type wide_type = lp_wider_type(bld->type);
      LLVMValueRef src_terml, src_termh, dst_terml, dst_termh;
      LLVMValueRef resl, resh;

      /*
       * We don't need saturate math for the sub/add, since we have
       * x+1 bit numbers in x*2 wide type (result is x+2 bits).
       * (Doesn't really matter on x86 sse2 though as we use saturated
       * intrinsics.)
       */
      wide_type.norm = 0;
      lp_build_context_init(&bldw, bld->gallivm, wide_type);

      /*
       * XXX This is a bit hackish. Note that -128 really should
       * be -1.0, the same as -127. However, we did not actually clamp
       * things anywhere (relying on pack intrinsics instead) therefore
       * we will get -128, and the inverted factor then 255. But the mul
       * can overflow in this case (rather the rounding fixups for the mul,
       * -128*255 will be positive).
       * So we clamp the src and dst up here but only when necessary (we
       * should do this before calculating blend factors but it's enough
       * for avoiding overflow).
       */
      if (is_inverse_factor(factor_src)) {
         src = lp_build_max(bld, src,
                            lp_build_const_vec(bld->gallivm, bld->type, -1.0));
      }
      if (is_inverse_factor(factor_dst)) {
         dst = lp_build_max(bld, dst,
                            lp_build_const_vec(bld->gallivm, bld->type, -1.0));
      }

      lp_build_mul_norm_expand(bld, src, src_factor, &src_terml, &src_termh,
                               is_inverse_factor(factor_src) ? true : false);
      lp_build_mul_norm_expand(bld, dst, dst_factor, &dst_terml, &dst_termh,
                               is_inverse_factor(factor_dst) ? true : false);
      resl = lp_build_blend_func(&bldw, func, src_terml, dst_terml);
      resh = lp_build_blend_func(&bldw, func, src_termh, dst_termh);

      /*
       * XXX pack2_native is not ok because the values have to be in dst
       * range. We need native pack though for the correct order on avx2.
       * Will break on everything not implementing clamping pack intrinsics
       * (i.e. everything but sse2 and altivec).
       */
      return lp_build_pack2_native(bld->gallivm, wide_type, bld->type, resl, resh);
   } else {
      src_term = lp_build_mul(bld, src, src_factor);
      dst_term = lp_build_mul(bld, dst, dst_factor);
      return lp_build_blend_func(bld, func, src_term, dst_term);
   }
}

void
lp_build_alpha_to_coverage(struct gallivm_state *gallivm,
                           struct lp_type type,
                           struct lp_build_mask_context *mask,
                           LLVMValueRef alpha,
                           bool do_branch)
{
   struct lp_build_context bld;
   LLVMValueRef test;
   LLVMValueRef alpha_ref_value;

   lp_build_context_init(&bld, gallivm, type);

   alpha_ref_value = lp_build_const_vec(gallivm, type, 0.5);

   test = lp_build_cmp(&bld, PIPE_FUNC_GREATER, alpha, alpha_ref_value);

   lp_build_name(test, "alpha_to_coverage");

   lp_build_mask_update(mask, test);

   if (do_branch)
      lp_build_mask_check(mask);
}
