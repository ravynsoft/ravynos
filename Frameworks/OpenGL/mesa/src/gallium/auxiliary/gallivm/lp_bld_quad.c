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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/


#include "lp_bld_type.h"
#include "lp_bld_arit.h"
#include "lp_bld_const.h"
#include "lp_bld_swizzle.h"
#include "lp_bld_quad.h"
#include "lp_bld_pack.h"


static const unsigned char
swizzle_left[4] = {
   LP_BLD_QUAD_TOP_LEFT,     LP_BLD_QUAD_TOP_LEFT,
   LP_BLD_QUAD_BOTTOM_LEFT,  LP_BLD_QUAD_BOTTOM_LEFT
};

static const unsigned char
swizzle_right[4] = {
   LP_BLD_QUAD_TOP_RIGHT,    LP_BLD_QUAD_TOP_RIGHT,
   LP_BLD_QUAD_BOTTOM_RIGHT, LP_BLD_QUAD_BOTTOM_RIGHT
};

static const unsigned char
swizzle_top[4] = {
   LP_BLD_QUAD_TOP_LEFT,     LP_BLD_QUAD_TOP_RIGHT,
   LP_BLD_QUAD_TOP_LEFT,     LP_BLD_QUAD_TOP_RIGHT
};

static const unsigned char
swizzle_bottom[4] = {
   LP_BLD_QUAD_BOTTOM_LEFT,  LP_BLD_QUAD_BOTTOM_RIGHT,
   LP_BLD_QUAD_BOTTOM_LEFT,  LP_BLD_QUAD_BOTTOM_RIGHT
};


LLVMValueRef
lp_build_ddx(struct lp_build_context *bld,
             LLVMValueRef a)
{
   LLVMValueRef a_left  = lp_build_swizzle_aos(bld, a, swizzle_left);
   LLVMValueRef a_right = lp_build_swizzle_aos(bld, a, swizzle_right);
   return lp_build_sub(bld, a_right, a_left);
}


LLVMValueRef
lp_build_ddy(struct lp_build_context *bld,
             LLVMValueRef a)
{
   LLVMValueRef a_top    = lp_build_swizzle_aos(bld, a, swizzle_top);
   LLVMValueRef a_bottom = lp_build_swizzle_aos(bld, a, swizzle_bottom);
   return lp_build_sub(bld, a_bottom, a_top);
}

/*
 * Helper for building packed ddx/ddy vector for one coord (scalar per quad
 * values). The vector will look like this (8-wide):
 * dr1dx _____ -dr1dy _____ dr2dx _____ -dr2dy _____
 * This only requires one shuffle instead of two for more straightforward packing.
 */
LLVMValueRef
lp_build_packed_ddx_ddy_onecoord(struct lp_build_context *bld,
                                 LLVMValueRef a)
{
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef vec1, vec2;

   /* use aos swizzle helper */

   static const unsigned char swizzle1[] = { /* no-op swizzle */
      LP_BLD_QUAD_TOP_LEFT, LP_BLD_SWIZZLE_DONTCARE,
      LP_BLD_QUAD_BOTTOM_LEFT, LP_BLD_SWIZZLE_DONTCARE
   };
   static const unsigned char swizzle2[] = {
      LP_BLD_QUAD_TOP_RIGHT, LP_BLD_SWIZZLE_DONTCARE,
      LP_BLD_QUAD_TOP_LEFT, LP_BLD_SWIZZLE_DONTCARE
   };

   vec1 = lp_build_swizzle_aos(bld, a, swizzle1);
   vec2 = lp_build_swizzle_aos(bld, a, swizzle2);

   if (bld->type.floating)
      return LLVMBuildFSub(builder, vec2, vec1, "ddxddy");
   else
      return LLVMBuildSub(builder, vec2, vec1, "ddxddy");
}


/*
 * Helper for building packed ddx/ddy vector for one coord (scalar per quad
 * values). The vector will look like this (8-wide):
 * ds1dx ds1dy dt1dx dt1dy ds2dx ds2dy dt2dx dt2dy
 * This only needs 2 (v)shufps.
 */
LLVMValueRef
lp_build_packed_ddx_ddy_twocoord(struct lp_build_context *bld,
                                 LLVMValueRef a, LLVMValueRef b)
{
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef shuffles1[LP_MAX_VECTOR_LENGTH/4];
   LLVMValueRef shuffles2[LP_MAX_VECTOR_LENGTH/4];
   LLVMValueRef vec1, vec2;

   /* XXX: do hsub version */
   const unsigned length = bld->type.length;
   const unsigned num_quads = length / 4;
   for (unsigned i = 0; i < num_quads; i++) {
      unsigned s1 = 4 * i;
      unsigned s2 = 4 * i + length;
      shuffles1[4*i + 0] = lp_build_const_int32(gallivm, LP_BLD_QUAD_TOP_LEFT + s1);
      shuffles1[4*i + 1] = lp_build_const_int32(gallivm, LP_BLD_QUAD_TOP_LEFT + s1);
      shuffles1[4*i + 2] = lp_build_const_int32(gallivm, LP_BLD_QUAD_TOP_LEFT + s2);
      shuffles1[4*i + 3] = lp_build_const_int32(gallivm, LP_BLD_QUAD_TOP_LEFT + s2);
      shuffles2[4*i + 0] = lp_build_const_int32(gallivm, LP_BLD_QUAD_TOP_RIGHT + s1);
      shuffles2[4*i + 1] = lp_build_const_int32(gallivm, LP_BLD_QUAD_BOTTOM_LEFT + s1);
      shuffles2[4*i + 2] = lp_build_const_int32(gallivm, LP_BLD_QUAD_TOP_RIGHT + s2);
      shuffles2[4*i + 3] = lp_build_const_int32(gallivm, LP_BLD_QUAD_BOTTOM_LEFT + s2);
   }
   vec1 = LLVMBuildShuffleVector(builder, a, b,
                                 LLVMConstVector(shuffles1, length), "");
   vec2 = LLVMBuildShuffleVector(builder, a, b,
                                 LLVMConstVector(shuffles2, length), "");
   if (bld->type.floating)
      return LLVMBuildFSub(builder, vec2, vec1, "ddxddyddxddy");
   else
      return LLVMBuildSub(builder, vec2, vec1, "ddxddyddxddy");
}


/**
 * Twiddle from quad format to row format
 *
 *   src0      src1
 * ######### #########      #################
 * # 0 | 1 # # 4 | 5 #      # 0 | 1 | 4 | 5 # src0
 * #---+---# #---+---#  ->  #################
 * # 2 | 3 # # 6 | 7 #      # 2 | 3 | 6 | 7 # src1
 * ######### #########      #################
 *
 */
void
lp_bld_quad_twiddle(struct gallivm_state *gallivm,
                    struct lp_type lp_dst_type,
                    const LLVMValueRef* src,
                    unsigned src_count,
                    LLVMValueRef* dst)
{
   assert((src_count % 2) == 0);

   /* Create a type with only 2 elements */
   struct lp_type type2 = lp_dst_type;
   type2.width = (lp_dst_type.width * lp_dst_type.length) / 2;
   type2.length = 2;
   type2.floating = 0;

   LLVMTypeRef type2_ref = lp_build_vec_type(gallivm, type2);
   LLVMTypeRef dst_type_ref = lp_build_vec_type(gallivm, lp_dst_type);
   LLVMBuilderRef builder = gallivm->builder;

   for (unsigned i = 0; i < src_count; i += 2) {
      LLVMValueRef src0, src1;

      src0 = LLVMBuildBitCast(builder, src[i + 0], type2_ref, "");
      src1 = LLVMBuildBitCast(builder, src[i + 1], type2_ref, "");

      dst[i + 0] = lp_build_interleave2(gallivm, type2, src0, src1, 0);
      dst[i + 1] = lp_build_interleave2(gallivm, type2, src0, src1, 1);

      dst[i + 0] = LLVMBuildBitCast(builder, dst[i + 0], dst_type_ref, "");
      dst[i + 1] = LLVMBuildBitCast(builder, dst[i + 1], dst_type_ref, "");
   }
}
