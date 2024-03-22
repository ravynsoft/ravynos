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
 * Blend LLVM IR generation -- AoS layout.
 *
 * AoS blending is in general much slower than SoA, but there are some cases
 * where it might be faster. In particular, if a pixel is rendered only once
 * then the overhead of tiling and untiling will dominate over the speedup that
 * SoA gives. So we might want to detect such cases and fallback to AoS in the
 * future, but for now this function is here for historical/benchmarking
 * purposes.
 *
 * Run lp_blend_test after any change to this file.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#include "pipe/p_state.h"
#include "util/u_debug.h"
#include "util/format/u_format.h"

#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_arit.h"
#include "gallivm/lp_bld_logic.h"
#include "gallivm/lp_bld_swizzle.h"
#include "gallivm/lp_bld_bitarit.h"
#include "gallivm/lp_bld_debug.h"

#include "lp_bld_blend.h"


/**
 * We may the same values several times, so we keep them here to avoid
 * recomputing them. Also reusing the values allows us to do simplifications
 * that LLVM optimization passes wouldn't normally be able to do.
 */
struct lp_build_blend_aos_context
{
   struct lp_build_context base;

   LLVMValueRef src;
   LLVMValueRef src_alpha;
   LLVMValueRef src1;
   LLVMValueRef src1_alpha;
   LLVMValueRef dst;
   LLVMValueRef const_;
   LLVMValueRef const_alpha;
   bool has_dst_alpha;

   LLVMValueRef inv_src;
   LLVMValueRef inv_src_alpha;
   LLVMValueRef inv_dst;
   LLVMValueRef inv_const;
   LLVMValueRef inv_const_alpha;
   LLVMValueRef saturate;

   LLVMValueRef rgb_src_factor;
   LLVMValueRef alpha_src_factor;
   LLVMValueRef rgb_dst_factor;
   LLVMValueRef alpha_dst_factor;
};


static LLVMValueRef
lp_build_blend_factor_unswizzled(struct lp_build_blend_aos_context *bld,
                                 unsigned factor,
                                 bool alpha)
{
   LLVMValueRef src_alpha = bld->src_alpha ? bld->src_alpha : bld->src;
   LLVMValueRef src1_alpha = bld->src1_alpha ? bld->src1_alpha : bld->src1;
   LLVMValueRef const_alpha = bld->const_alpha ? bld->const_alpha : bld->const_;

   switch (factor) {
   case PIPE_BLENDFACTOR_ZERO:
      return bld->base.zero;
   case PIPE_BLENDFACTOR_ONE:
      return bld->base.one;
   case PIPE_BLENDFACTOR_SRC_COLOR:
      return bld->src;
   case PIPE_BLENDFACTOR_SRC_ALPHA:
      return src_alpha;
   case PIPE_BLENDFACTOR_DST_COLOR:
   case PIPE_BLENDFACTOR_DST_ALPHA:
      return bld->dst;
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      if (alpha) {
         return bld->base.one;
      } else {
         /*
          * If there's no dst alpha the complement is zero but for unclamped
          * float inputs (or snorm inputs) min can be non-zero (negative).
          */
         if (!bld->saturate) {
            if (!bld->has_dst_alpha) {
               bld->saturate = lp_build_min(&bld->base, src_alpha, bld->base.zero);
            }
            else if (bld->base.type.norm && bld->base.type.sign) {
               /*
                * The complement/min totally doesn't work, since
                * the complement is in range [0,2] but the other
                * min input is [-1,1]. However, we can just clamp to 0
                * before doing the complement...
                */
               LLVMValueRef inv_dst;
               inv_dst = lp_build_max(&bld->base, bld->base.zero, bld->dst);
               inv_dst = lp_build_comp(&bld->base, inv_dst);
               bld->saturate = lp_build_min(&bld->base, src_alpha, inv_dst);
            } else {
               if (!bld->inv_dst) {
                  bld->inv_dst = lp_build_comp(&bld->base, bld->dst);
               }
               bld->saturate = lp_build_min(&bld->base, src_alpha, bld->inv_dst);
            }
         }
         return bld->saturate;
      }
   case PIPE_BLENDFACTOR_CONST_COLOR:
      return bld->const_;
   case PIPE_BLENDFACTOR_CONST_ALPHA:
      return const_alpha;
   case PIPE_BLENDFACTOR_SRC1_COLOR:
      return bld->src1;
   case PIPE_BLENDFACTOR_SRC1_ALPHA:
      return src1_alpha;
   case PIPE_BLENDFACTOR_INV_SRC_COLOR:
      if (!bld->inv_src)
         bld->inv_src = lp_build_comp(&bld->base, bld->src);
      return bld->inv_src;
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
      if (!bld->inv_src_alpha)
         bld->inv_src_alpha = lp_build_comp(&bld->base, src_alpha);
      return bld->inv_src_alpha;
   case PIPE_BLENDFACTOR_INV_DST_COLOR:
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
      if (!bld->inv_dst)
         bld->inv_dst = lp_build_comp(&bld->base, bld->dst);
      return bld->inv_dst;
   case PIPE_BLENDFACTOR_INV_CONST_COLOR:
      if (!bld->inv_const)
         bld->inv_const = lp_build_comp(&bld->base, bld->const_);
      return bld->inv_const;
   case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
      if (!bld->inv_const_alpha)
         bld->inv_const_alpha = lp_build_comp(&bld->base, const_alpha);
      return bld->inv_const_alpha;
   case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
      return lp_build_comp(&bld->base, bld->src1);
   case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
      return lp_build_comp(&bld->base, src1_alpha);
   default:
      assert(0);
      return bld->base.zero;
   }
}


enum lp_build_blend_swizzle {
   LP_BUILD_BLEND_SWIZZLE_RGBA = 0,
   LP_BUILD_BLEND_SWIZZLE_AAAA = 1
};


/**
 * How should we shuffle the base factor.
 */
static enum lp_build_blend_swizzle
lp_build_blend_factor_swizzle(unsigned factor)
{
   switch (factor) {
   case PIPE_BLENDFACTOR_ONE:
   case PIPE_BLENDFACTOR_ZERO:
   case PIPE_BLENDFACTOR_SRC_COLOR:
   case PIPE_BLENDFACTOR_DST_COLOR:
   case PIPE_BLENDFACTOR_CONST_COLOR:
   case PIPE_BLENDFACTOR_SRC1_COLOR:
   case PIPE_BLENDFACTOR_INV_SRC_COLOR:
   case PIPE_BLENDFACTOR_INV_DST_COLOR:
   case PIPE_BLENDFACTOR_INV_CONST_COLOR:
   case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
      return LP_BUILD_BLEND_SWIZZLE_RGBA;
   case PIPE_BLENDFACTOR_SRC_ALPHA:
   case PIPE_BLENDFACTOR_DST_ALPHA:
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
   case PIPE_BLENDFACTOR_SRC1_ALPHA:
   case PIPE_BLENDFACTOR_CONST_ALPHA:
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
   case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
   case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
      return LP_BUILD_BLEND_SWIZZLE_AAAA;
   default:
      assert(0);
      return LP_BUILD_BLEND_SWIZZLE_RGBA;
   }
}


static LLVMValueRef
lp_build_blend_swizzle(struct lp_build_blend_aos_context *bld,
                       LLVMValueRef rgb,
                       LLVMValueRef alpha,
                       enum lp_build_blend_swizzle rgb_swizzle,
                       unsigned alpha_swizzle,
                       unsigned num_channels)
{
   LLVMValueRef swizzled_rgb;

   switch (rgb_swizzle) {
   case LP_BUILD_BLEND_SWIZZLE_RGBA:
      swizzled_rgb = rgb;
      break;
   case LP_BUILD_BLEND_SWIZZLE_AAAA:
      swizzled_rgb = lp_build_swizzle_scalar_aos(&bld->base, rgb,
                                                 alpha_swizzle, num_channels);
      break;
   default:
      assert(0);
      swizzled_rgb = bld->base.undef;
   }

   if (rgb != alpha) {
      swizzled_rgb = lp_build_select_aos(&bld->base, 1 << alpha_swizzle,
                                         alpha, swizzled_rgb,
                                         num_channels);
   }

   return swizzled_rgb;
}


/**
 * @sa http://www.opengl.org/sdk/docs/man/xhtml/glBlendFuncSeparate.xml
 */
static LLVMValueRef
lp_build_blend_factor(struct lp_build_blend_aos_context *bld,
                      unsigned rgb_factor,
                      unsigned alpha_factor,
                      unsigned alpha_swizzle,
                      unsigned num_channels)
{
   LLVMValueRef rgb_factor_, alpha_factor_;
   enum lp_build_blend_swizzle rgb_swizzle;

   if (alpha_swizzle == PIPE_SWIZZLE_X && num_channels == 1) {
      return lp_build_blend_factor_unswizzled(bld, alpha_factor, true);
   }

   rgb_factor_ = lp_build_blend_factor_unswizzled(bld, rgb_factor, false);

   if (alpha_swizzle != PIPE_SWIZZLE_NONE) {
      rgb_swizzle   = lp_build_blend_factor_swizzle(rgb_factor);
      alpha_factor_ = lp_build_blend_factor_unswizzled(bld, alpha_factor, true);
      return lp_build_blend_swizzle(bld, rgb_factor_, alpha_factor_,
                                    rgb_swizzle, alpha_swizzle, num_channels);
   } else {
      return rgb_factor_;
   }
}


/**
 * Performs blending of src and dst pixels
 *
 * @param blend         the blend state of the shader variant
 * @param cbuf_format   format of the colour buffer
 * @param type          data type of the pixel vector
 * @param rt            render target index
 * @param src           blend src
 * @param src_alpha     blend src alpha (if not included in src)
 * @param src1          second blend src (for dual source blend)
 * @param src1_alpha    second blend src alpha (if not included in src1)
 * @param dst           blend dst
 * @param mask          optional mask to apply to the blending result
 * @param const_        const blend color
 * @param const_alpha   const blend color alpha (if not included in const_)
 * @param swizzle       swizzle values for RGBA
 *
 * @return the result of blending src and dst
 */
LLVMValueRef
lp_build_blend_aos(struct gallivm_state *gallivm,
                   const struct pipe_blend_state *blend,
                   enum pipe_format cbuf_format,
                   struct lp_type type,
                   unsigned rt,
                   LLVMValueRef src,
                   LLVMValueRef src_alpha,
                   LLVMValueRef src1,
                   LLVMValueRef src1_alpha,
                   LLVMValueRef dst,
                   LLVMValueRef mask,
                   LLVMValueRef const_,
                   LLVMValueRef const_alpha,
                   const unsigned char swizzle[4],
                   int nr_channels)
{
   const struct pipe_rt_blend_state *state = &blend->rt[rt];
   const struct util_format_description *desc =
      util_format_description(cbuf_format);
   struct lp_build_blend_aos_context bld;
   LLVMValueRef result;

   /* Setup build context */
   memset(&bld, 0, sizeof bld);
   lp_build_context_init(&bld.base, gallivm, type);
   bld.src = src;
   bld.src1 = src1;
   bld.dst = dst;
   bld.const_ = const_;
   bld.src_alpha = src_alpha;
   bld.src1_alpha = src1_alpha;
   bld.const_alpha = const_alpha;
   bld.has_dst_alpha = false;

   /* Find the alpha channel if not provided separately */
   unsigned alpha_swizzle = PIPE_SWIZZLE_NONE;
   if (!src_alpha) {
      for (unsigned i = 0; i < 4; ++i) {
         if (swizzle[i] == 3) {
            alpha_swizzle = i;
         }
      }
      /*
       * Note that we may get src_alpha included from source (and 4 channels)
       * even if the destination doesn't have an alpha channel (for rgbx
       * formats). Generally this shouldn't make much of a difference (we're
       * relying on blend factors being sanitized already if there's no
       * dst alpha).
       */
      bld.has_dst_alpha = desc->swizzle[3] <= PIPE_SWIZZLE_W;
   }

   if (blend->logicop_enable) {
      if (!type.floating) {
         result = lp_build_logicop(gallivm->builder, blend->logicop_func,
                                   src, dst);
      } else {
         result = src;
      }
   } else if (!state->blend_enable) {
      result = src;
   } else {
      bool rgb_alpha_same =
         (state->rgb_src_factor == state->rgb_dst_factor &&
          state->alpha_src_factor == state->alpha_dst_factor) ||
         nr_channels == 1;
      bool alpha_only = nr_channels == 1 && alpha_swizzle == PIPE_SWIZZLE_X;
      LLVMValueRef src_factor, dst_factor;

      src_factor = lp_build_blend_factor(&bld, state->rgb_src_factor,
                                         state->alpha_src_factor,
                                         alpha_swizzle,
                                         nr_channels);

      dst_factor = lp_build_blend_factor(&bld, state->rgb_dst_factor,
                                         state->alpha_dst_factor,
                                         alpha_swizzle,
                                         nr_channels);

      result = lp_build_blend(&bld.base,
                              state->rgb_func,
                              alpha_only ? state->alpha_src_factor : state->rgb_src_factor,
                              alpha_only ? state->alpha_dst_factor : state->rgb_dst_factor,
                              src,
                              dst,
                              src_factor,
                              dst_factor,
                              rgb_alpha_same,
                              false);

      if (state->rgb_func != state->alpha_func && nr_channels > 1 &&
          alpha_swizzle != PIPE_SWIZZLE_NONE) {
         LLVMValueRef alpha;

         alpha = lp_build_blend(&bld.base,
                                state->alpha_func,
                                state->alpha_src_factor,
                                state->alpha_dst_factor,
                                src,
                                dst,
                                src_factor,
                                dst_factor,
                                rgb_alpha_same,
                                false);

         result = lp_build_blend_swizzle(&bld,
                                         result,
                                         alpha,
                                         LP_BUILD_BLEND_SWIZZLE_RGBA,
                                         alpha_swizzle,
                                         nr_channels);
      }
   }

   /* Check if color mask is necessary */
   if (!util_format_colormask_full(desc, state->colormask)) {
      LLVMValueRef color_mask =
         lp_build_const_mask_aos_swizzled(gallivm, bld.base.type,
                                          state->colormask, nr_channels,
                                          swizzle);
      lp_build_name(color_mask, "color_mask");

      /* Combine with input mask if necessary */
      if (mask) {
         /* We can be blending floating values but masks are always integer... */
         unsigned floating = bld.base.type.floating;
         bld.base.type.floating = 0;

         mask = lp_build_and(&bld.base, color_mask, mask);

         bld.base.type.floating = floating;
      } else {
         mask = color_mask;
      }
   }

   /* Apply mask, if one exists */
   if (mask) {
      result = lp_build_select(&bld.base, mask, result, dst);
   }

   return result;
}
