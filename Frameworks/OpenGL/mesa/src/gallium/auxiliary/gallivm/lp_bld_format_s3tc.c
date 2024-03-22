/**************************************************************************
 *
 * Copyright 2010-2018 VMware, Inc.
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


/**
 * @file
 * s3tc pixel format manipulation.
 *
 * @author Roland Scheidegger <sroland@vmware.com>
 */


#include <llvm/Config/llvm-config.h>

#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_string.h"
#include "util/u_cpu_detect.h"
#include "util/u_debug.h"

#include "lp_bld_arit.h"
#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_conv.h"
#include "lp_bld_gather.h"
#include "lp_bld_format.h"
#include "lp_bld_logic.h"
#include "lp_bld_pack.h"
#include "lp_bld_flow.h"
#include "lp_bld_printf.h"
#include "lp_bld_struct.h"
#include "lp_bld_swizzle.h"
#include "lp_bld_init.h"
#include "lp_bld_debug.h"
#include "lp_bld_intr.h"


/**
 * Reverse an interleave2_half
 * (ie. pick every second element, independent lower/upper halfs)
 * sse2 can only do that with 32bit (shufps) or larger elements
 * natively. (Otherwise, and/pack (even) or shift/pack (odd)
 * could be used, ideally llvm would do that for us.)
 * XXX: Unfortunately, this does NOT translate to a shufps if those
 * are int vectors (and casting will not help, llvm needs to recognize it
 * as "real" float). Instead, llvm will use a pshufd/pshufd/punpcklqdq
 * sequence which I'm pretty sure is a lot worse despite domain transition
 * penalties with shufps (except maybe on Nehalem).
 */
static LLVMValueRef
lp_build_uninterleave2_half(struct gallivm_state *gallivm,
                            struct lp_type type,
                            LLVMValueRef a,
                            LLVMValueRef b,
                            unsigned lo_hi)
{
   LLVMValueRef shuffle, elems[LP_MAX_VECTOR_LENGTH];
   unsigned i;

   assert(type.length <= LP_MAX_VECTOR_LENGTH);
   assert(lo_hi < 2);

   if (type.length * type.width == 256) {
      assert(type.length == 8);
      assert(type.width == 32);
      static const unsigned shufvals[8] = {0, 2, 8, 10, 4, 6, 12, 14};
      for (i = 0; i < type.length; ++i) {
         elems[i] = lp_build_const_int32(gallivm, shufvals[i] + lo_hi);
      }
   } else {
      for (i = 0; i < type.length; ++i) {
         elems[i] = lp_build_const_int32(gallivm, 2*i + lo_hi);
      }
   }

   shuffle = LLVMConstVector(elems, type.length);

   return LLVMBuildShuffleVector(gallivm->builder, a, b, shuffle, "");

}


/**
 * Build shuffle for extending vectors.
 */
static LLVMValueRef
lp_build_const_extend_shuffle(struct gallivm_state *gallivm,
                              unsigned n, unsigned length)
{
   LLVMValueRef elems[LP_MAX_VECTOR_LENGTH];
   unsigned i;

   assert(n <= length);
   assert(length <= LP_MAX_VECTOR_LENGTH);

   /* TODO: cache results in a static table */

   for(i = 0; i < n; i++) {
      elems[i] = lp_build_const_int32(gallivm, i);
   }
   for (i = n; i < length; i++) {
      elems[i] = LLVMGetUndef(LLVMInt32TypeInContext(gallivm->context));
   }

   return LLVMConstVector(elems, length);
}

static LLVMValueRef
lp_build_const_unpackx2_shuffle(struct gallivm_state *gallivm, unsigned n)
{
   LLVMValueRef elems[LP_MAX_VECTOR_LENGTH];
   unsigned i, j;

   assert(n <= LP_MAX_VECTOR_LENGTH);

   /* TODO: cache results in a static table */

   for(i = 0, j = 0; i < n; i += 2, ++j) {
      elems[i + 0] = lp_build_const_int32(gallivm, 0 + j);
      elems[i + 1] = lp_build_const_int32(gallivm, n + j);
      elems[n + i + 0] = lp_build_const_int32(gallivm, 0 + n/2 + j);
      elems[n + i + 1] = lp_build_const_int32(gallivm, n + n/2 + j);
   }

   return LLVMConstVector(elems, n * 2);
}

/*
 * broadcast 1 element to all elements
 */
static LLVMValueRef
lp_build_const_shuffle1(struct gallivm_state *gallivm,
                        unsigned index, unsigned n)
{
   LLVMValueRef elems[LP_MAX_VECTOR_LENGTH];
   unsigned i;

   assert(n <= LP_MAX_VECTOR_LENGTH);

   /* TODO: cache results in a static table */

   for (i = 0; i < n; i++) {
      elems[i] = lp_build_const_int32(gallivm, index);
   }

   return LLVMConstVector(elems, n);
}

/*
 * move 1 element to pos 0, rest undef
 */
static LLVMValueRef
lp_build_shuffle1undef(struct gallivm_state *gallivm,
                       LLVMValueRef a, unsigned index, unsigned n)
{
   LLVMValueRef elems[LP_MAX_VECTOR_LENGTH], shuf;
   unsigned i;

   assert(n <= LP_MAX_VECTOR_LENGTH);

   elems[0] = lp_build_const_int32(gallivm, index);

   for (i = 1; i < n; i++) {
      elems[i] = LLVMGetUndef(LLVMInt32TypeInContext(gallivm->context));
   }
   shuf = LLVMConstVector(elems, n);

   return LLVMBuildShuffleVector(gallivm->builder, a, a, shuf, "");
}

static bool
format_dxt1_variant(enum pipe_format format)
{
  return format == PIPE_FORMAT_DXT1_RGB ||
         format == PIPE_FORMAT_DXT1_RGBA ||
         format == PIPE_FORMAT_DXT1_SRGB ||
         format == PIPE_FORMAT_DXT1_SRGBA;

}

/**
 * Gather elements from scatter positions in memory into vectors.
 * This is customised for fetching texels from s3tc textures.
 * For SSE, typical value is length=4.
 *
 * @param length length of the offsets
 * @param colors the stored colors of the blocks will be extracted into this.
 * @param codewords the codewords of the blocks will be extracted into this.
 * @param alpha_lo used for storing lower 32bit of alpha components for dxt3/5
 * @param alpha_hi used for storing higher 32bit of alpha components for dxt3/5
 * @param base_ptr base pointer, should be a i8 pointer type.
 * @param offsets vector with offsets
 */
static void
lp_build_gather_s3tc(struct gallivm_state *gallivm,
                     unsigned length,
                     const struct util_format_description *format_desc,
                     LLVMValueRef *colors,
                     LLVMValueRef *codewords,
                     LLVMValueRef *alpha_lo,
                     LLVMValueRef *alpha_hi,
                     LLVMValueRef base_ptr,
                     LLVMValueRef offsets)
{
   LLVMBuilderRef builder = gallivm->builder;
   unsigned block_bits = format_desc->block.bits;
   unsigned i;
   LLVMValueRef elems[8];
   LLVMTypeRef type32 = LLVMInt32TypeInContext(gallivm->context);
   LLVMTypeRef type64 = LLVMInt64TypeInContext(gallivm->context);
   LLVMTypeRef type32dxt;
   struct lp_type lp_type32dxt;

   memset(&lp_type32dxt, 0, sizeof lp_type32dxt);
   lp_type32dxt.width = 32;
   lp_type32dxt.length = block_bits / 32;
   type32dxt = lp_build_vec_type(gallivm, lp_type32dxt);

   assert(block_bits == 64 || block_bits == 128);
   assert(length == 1 || length == 4 || length == 8);

   for (i = 0; i < length; ++i) {
      elems[i] = lp_build_gather_elem(gallivm, length,
                                      block_bits, block_bits, true,
                                      base_ptr, offsets, i, false);
      elems[i] = LLVMBuildBitCast(builder, elems[i], type32dxt, "");
   }
   if (length == 1) {
      LLVMValueRef elem = elems[0];
      if (block_bits == 128) {
         *alpha_lo = LLVMBuildExtractElement(builder, elem,
                                             lp_build_const_int32(gallivm, 0), "");
         *alpha_hi = LLVMBuildExtractElement(builder, elem,
                                             lp_build_const_int32(gallivm, 1), "");
         *colors = LLVMBuildExtractElement(builder, elem,
                                           lp_build_const_int32(gallivm, 2), "");
         *codewords = LLVMBuildExtractElement(builder, elem,
                                              lp_build_const_int32(gallivm, 3), "");
      }
      else {
         *alpha_lo = LLVMGetUndef(type32);
         *alpha_hi = LLVMGetUndef(type32);
         *colors = LLVMBuildExtractElement(builder, elem,
                                           lp_build_const_int32(gallivm, 0), "");
         *codewords = LLVMBuildExtractElement(builder, elem,
                                              lp_build_const_int32(gallivm, 1), "");
      }
   }
   else {
      LLVMValueRef tmp[4], cc01, cc23;
      struct lp_type lp_type32, lp_type64;
      memset(&lp_type32, 0, sizeof lp_type32);
      lp_type32.width = 32;
      lp_type32.length = length;
      memset(&lp_type64, 0, sizeof lp_type64);
      lp_type64.width = 64;
      lp_type64.length = length/2;

      if (block_bits == 128) {
         if (length == 8) {
            for (i = 0; i < 4; ++i) {
               tmp[0] = elems[i];
               tmp[1] = elems[i+4];
               elems[i] = lp_build_concat(gallivm, tmp, lp_type32dxt, 2);
            }
         }
         lp_build_transpose_aos(gallivm, lp_type32, elems, tmp);
         *colors = tmp[2];
         *codewords = tmp[3];
         *alpha_lo = tmp[0];
         *alpha_hi = tmp[1];
      } else {
         LLVMTypeRef type64_vec = LLVMVectorType(type64, length/2);
         LLVMTypeRef type32_vec = LLVMVectorType(type32, length);

         for (i = 0; i < length; ++i) {
            /* no-op shuffle */
            elems[i] = LLVMBuildShuffleVector(builder, elems[i],
                                              LLVMGetUndef(type32dxt),
                                              lp_build_const_extend_shuffle(gallivm, 2, 4), "");
         }
         if (length == 8) {
            struct lp_type lp_type32_4 = {0};
            lp_type32_4.width = 32;
            lp_type32_4.length = 4;
            for (i = 0; i < 4; ++i) {
               tmp[0] = elems[i];
               tmp[1] = elems[i+4];
               elems[i] = lp_build_concat(gallivm, tmp, lp_type32_4, 2);
            }
         }
         cc01 = lp_build_interleave2_half(gallivm, lp_type32, elems[0], elems[1], 0);
         cc23 = lp_build_interleave2_half(gallivm, lp_type32, elems[2], elems[3], 0);
         cc01 = LLVMBuildBitCast(builder, cc01, type64_vec, "");
         cc23 = LLVMBuildBitCast(builder, cc23, type64_vec, "");
         *colors = lp_build_interleave2_half(gallivm, lp_type64, cc01, cc23, 0);
         *codewords = lp_build_interleave2_half(gallivm, lp_type64, cc01, cc23, 1);
         *colors = LLVMBuildBitCast(builder, *colors, type32_vec, "");
         *codewords = LLVMBuildBitCast(builder, *codewords, type32_vec, "");
      }
   }
}

/** Convert from <n x i32> containing 2 x n rgb565 colors
 * to 2 <n x i32> rgba8888 colors
 * This is the most optimized version I can think of
 * should be nearly as fast as decoding only one color
 * NOTE: alpha channel will be set to 0
 * @param colors  is a <n x i32> vector containing the rgb565 colors
 */
static void
color_expand2_565_to_8888(struct gallivm_state *gallivm,
                          unsigned n,
                          LLVMValueRef colors,
                          LLVMValueRef *color0,
                          LLVMValueRef *color1)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef r, g, b, rblo, glo;
   LLVMValueRef rgblomask, rb, rgb0, rgb1;
   struct lp_type type, type16, type8;

   assert(n > 1);

   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;

   memset(&type16, 0, sizeof type16);
   type16.width = 16;
   type16.length = 2 * n;

   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = 4 * n;

   rgblomask = lp_build_const_int_vec(gallivm, type16, 0x0707);
   colors = LLVMBuildBitCast(builder, colors,
                             lp_build_vec_type(gallivm, type16), "");
   /* move r into low 8 bits, b into high 8 bits, g into another reg (low bits)
    * make sure low bits of r are zero - could use AND but requires constant */
   r = LLVMBuildLShr(builder, colors, lp_build_const_int_vec(gallivm, type16, 11), "");
   r = LLVMBuildShl(builder, r, lp_build_const_int_vec(gallivm, type16, 3), "");
   b = LLVMBuildShl(builder, colors, lp_build_const_int_vec(gallivm, type16, 11), "");
   rb = LLVMBuildOr(builder, r, b, "");
   rblo = LLVMBuildLShr(builder, rb, lp_build_const_int_vec(gallivm, type16, 5), "");
   /* don't have byte shift hence need mask */
   rblo = LLVMBuildAnd(builder, rblo, rgblomask, "");
   rb = LLVMBuildOr(builder, rb, rblo, "");

   /* make sure low bits of g are zero */
   g = LLVMBuildAnd(builder, colors, lp_build_const_int_vec(gallivm, type16, 0x07e0), "");
   g = LLVMBuildLShr(builder, g, lp_build_const_int_vec(gallivm, type16, 3), "");
   glo = LLVMBuildLShr(builder, g, lp_build_const_int_vec(gallivm, type16, 6), "");
   g = LLVMBuildOr(builder, g, glo, "");

   rb = LLVMBuildBitCast(builder, rb, lp_build_vec_type(gallivm, type8), "");
   g = LLVMBuildBitCast(builder, g, lp_build_vec_type(gallivm, type8), "");
   rgb0 = lp_build_interleave2_half(gallivm, type8, rb, g, 0);
   rgb1 = lp_build_interleave2_half(gallivm, type8, rb, g, 1);

   rgb0 = LLVMBuildBitCast(builder, rgb0, lp_build_vec_type(gallivm, type), "");
   rgb1 = LLVMBuildBitCast(builder, rgb1, lp_build_vec_type(gallivm, type), "");

   /* rgb0 is rgb00, rgb01, rgb10, rgb11
    * instead of rgb00, rgb10, rgb20, rgb30 hence need reshuffle
    * on x86 this _should_ just generate one shufps...
    */
   *color0 = lp_build_uninterleave2_half(gallivm, type, rgb0, rgb1, 0);
   *color1 = lp_build_uninterleave2_half(gallivm, type, rgb0, rgb1, 1);
}


/** Convert from <n x i32> containing rgb565 colors
 * (in first 16 bits) to <n x i32> rgba8888 colors
 * bits 16-31 MBZ
 * NOTE: alpha channel will be set to 0
 * @param colors  is a <n x i32> vector containing the rgb565 colors
 */
static LLVMValueRef
color_expand_565_to_8888(struct gallivm_state *gallivm,
                         unsigned n,
                         LLVMValueRef colors)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef rgba, r, g, b, rgblo, glo;
   LLVMValueRef rbhimask, g6mask, rgblomask;
   struct lp_type type;
   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;

   /* color expansion:
    * first extract and shift colors into their final locations
    * (high bits - low bits zero at this point)
    * then replicate highest bits to the lowest bits
    * note rb replication can be done in parallel but not g
    * (different shift)
    * r5mask = 0xf800, g6mask = 0x07e0, b5mask = 0x001f
    * rhigh = 8, ghigh = 5, bhigh = 19
    * rblow = 5, glow = 6
    * rgblowmask = 0x00070307
    * r = colors >> rhigh
    * b = colors << bhigh
    * g = (colors & g6mask) << ghigh
    * rb = (r | b) rbhimask
    * rbtmp = rb >> rblow
    * gtmp = rb >> glow
    * rbtmp = rbtmp | gtmp
    * rbtmp = rbtmp & rgblowmask
    * rgb = rb | g | rbtmp
    */
   g6mask = lp_build_const_int_vec(gallivm, type, 0x07e0);
   rbhimask = lp_build_const_int_vec(gallivm, type, 0x00f800f8);
   rgblomask = lp_build_const_int_vec(gallivm, type, 0x00070307);

   r = LLVMBuildLShr(builder, colors, lp_build_const_int_vec(gallivm, type, 8), "");
   b = LLVMBuildShl(builder, colors, lp_build_const_int_vec(gallivm, type, 19), "");
   g = LLVMBuildAnd(builder, colors, g6mask, "");
   g = LLVMBuildShl(builder, g, lp_build_const_int_vec(gallivm, type, 5), "");
   rgba = LLVMBuildOr(builder, r, b, "");
   rgba = LLVMBuildAnd(builder, rgba, rbhimask, "");
   rgblo = LLVMBuildLShr(builder, rgba, lp_build_const_int_vec(gallivm, type, 5), "");
   glo = LLVMBuildLShr(builder, g, lp_build_const_int_vec(gallivm, type, 6), "");
   rgblo = LLVMBuildOr(builder, rgblo, glo, "");
   rgblo = LLVMBuildAnd(builder, rgblo, rgblomask, "");
   rgba = LLVMBuildOr(builder, rgba, g, "");
   rgba = LLVMBuildOr(builder, rgba, rgblo, "");

   return rgba;
}


/*
 * Average two byte vectors. (Will always round up.)
 */
static LLVMValueRef
lp_build_pavgb(struct lp_build_context *bld8,
               LLVMValueRef v0,
               LLVMValueRef v1)
{
   struct gallivm_state *gallivm = bld8->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   assert(bld8->type.width == 8);
   assert(bld8->type.length == 16 || bld8->type.length == 32);
   if (LLVM_VERSION_MAJOR < 6) {
      LLVMValueRef intrargs[2];
      char *intr_name = bld8->type.length == 32 ? "llvm.x86.avx2.pavg.b" :
                                                  "llvm.x86.sse2.pavg.b";
      intrargs[0] = v0;
      intrargs[1] = v1;
      return lp_build_intrinsic(builder, intr_name,
                                bld8->vec_type, intrargs, 2, 0);
   } else {
      /*
       * Must match llvm's autoupgrade of pavg.b intrinsic to be useful.
       * You better hope the backend code manages to detect the pattern, and
       * the pattern doesn't change there...
       */
      struct lp_type type_ext = bld8->type;
      LLVMTypeRef vec_type_ext;
      LLVMValueRef res;
      LLVMValueRef ext_one;
      type_ext.width = 16;
      vec_type_ext = lp_build_vec_type(gallivm, type_ext);
      ext_one = lp_build_const_vec(gallivm, type_ext, 1);

      v0 = LLVMBuildZExt(builder, v0, vec_type_ext, "");
      v1 = LLVMBuildZExt(builder, v1, vec_type_ext, "");
      res = LLVMBuildAdd(builder, v0, v1, "");
      res = LLVMBuildAdd(builder, res, ext_one, "");
      res = LLVMBuildLShr(builder, res, ext_one, "");
      res = LLVMBuildTrunc(builder, res, bld8->vec_type, "");
      return res;
   }
}

/**
 * Calculate 1/3(v1-v0) + v0
 * and 2*1/3(v1-v0) + v0
 */
static void
lp_build_lerp23(struct lp_build_context *bld,
                LLVMValueRef v0,
                LLVMValueRef v1,
                LLVMValueRef *res0,
                LLVMValueRef *res1)
{
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMValueRef x, x_lo, x_hi, delta_lo, delta_hi;
   LLVMValueRef mul_lo, mul_hi, v0_lo, v0_hi, v1_lo, v1_hi, tmp;
   const struct lp_type type = bld->type;
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_type i16_type = lp_wider_type(type);
   struct lp_build_context bld2;

   assert(lp_check_value(type, v0));
   assert(lp_check_value(type, v1));
   assert(!type.floating && !type.fixed && !type.norm && type.width == 8);

   lp_build_context_init(&bld2, gallivm, i16_type);
   bld2.type.sign = true;
   x = lp_build_const_int_vec(gallivm, bld->type, 255*1/3);

   /* FIXME: use native avx256 unpack/pack */
   lp_build_unpack2(gallivm, type, i16_type, x, &x_lo, &x_hi);
   lp_build_unpack2(gallivm, type, i16_type, v0, &v0_lo, &v0_hi);
   lp_build_unpack2(gallivm, type, i16_type, v1, &v1_lo, &v1_hi);
   delta_lo = lp_build_sub(&bld2, v1_lo, v0_lo);
   delta_hi = lp_build_sub(&bld2, v1_hi, v0_hi);

   mul_lo = LLVMBuildMul(builder, x_lo, delta_lo, "");
   mul_hi = LLVMBuildMul(builder, x_hi, delta_hi, "");

   x_lo = LLVMBuildLShr(builder, mul_lo, lp_build_const_int_vec(gallivm, i16_type, 8), "");
   x_hi = LLVMBuildLShr(builder, mul_hi, lp_build_const_int_vec(gallivm, i16_type, 8), "");
   /* lerp optimization: pack now, do add afterwards */
   tmp = lp_build_pack2(gallivm, i16_type, type, x_lo, x_hi);
   *res0 = lp_build_add(bld, tmp, v0);

   x_lo = LLVMBuildLShr(builder, mul_lo, lp_build_const_int_vec(gallivm, i16_type, 7), "");
   x_hi = LLVMBuildLShr(builder, mul_hi, lp_build_const_int_vec(gallivm, i16_type, 7), "");
   /* unlike above still need mask (but add still afterwards). */
   x_lo = LLVMBuildAnd(builder, x_lo, lp_build_const_int_vec(gallivm, i16_type, 0xff), "");
   x_hi = LLVMBuildAnd(builder, x_hi, lp_build_const_int_vec(gallivm, i16_type, 0xff), "");
   tmp = lp_build_pack2(gallivm, i16_type, type, x_lo, x_hi);
   *res1 = lp_build_add(bld, tmp, v0);
}

/**
 * Convert from <n x i64> s3tc dxt1 to <4n x i8> RGBA AoS
 * @param colors  is a <n x i32> vector with n x 2x16bit colors
 * @param codewords  is a <n x i32> vector containing the codewords
 * @param i  is a <n x i32> vector with the x pixel coordinate (0 to 3)
 * @param j  is a <n x i32> vector with the y pixel coordinate (0 to 3)
 */
static LLVMValueRef
s3tc_dxt1_full_to_rgba_aos(struct gallivm_state *gallivm,
                           unsigned n,
                           enum pipe_format format,
                           LLVMValueRef colors,
                           LLVMValueRef codewords,
                           LLVMValueRef i,
                           LLVMValueRef j)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef color0, color1, color2, color3, color2_2, color3_2;
   LLVMValueRef rgba, a, colors0, colors1, col0, col1, const2;
   LLVMValueRef bit_pos, sel_mask, sel_lo, sel_hi, indices;
   struct lp_type type, type8;
   struct lp_build_context bld8, bld32;
   bool is_dxt1_variant = format_dxt1_variant(format);

   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;

   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = 4*n;

   assert(lp_check_value(type, i));
   assert(lp_check_value(type, j));

   a = lp_build_const_int_vec(gallivm, type, 0xff000000);

   lp_build_context_init(&bld32, gallivm, type);
   lp_build_context_init(&bld8, gallivm, type8);

   /*
    * works as follows:
    * - expand color0/color1 to rgba8888
    * - calculate color2/3 (interpolation) according to color0 < color1 rules
    * - calculate color2/3 according to color0 >= color1 rules
    * - do selection of color2/3 according to comparison of color0/1
    * - extract indices (vector shift).
    * - use compare/select to select the correct color. Since we have 2bit
    *   indices (and 4 colors), needs at least three compare/selects.
    */
   /*
    * expand the two colors
    */
   col0 = LLVMBuildAnd(builder, colors, lp_build_const_int_vec(gallivm, type, 0x0000ffff), "");
   col1 = LLVMBuildLShr(builder, colors, lp_build_const_int_vec(gallivm, type, 16), "");
   if (n > 1) {
      color_expand2_565_to_8888(gallivm, n, colors, &color0, &color1);
   }
   else {
      color0 = color_expand_565_to_8888(gallivm, n, col0);
      color1 = color_expand_565_to_8888(gallivm, n, col1);
   }

   /*
    * interpolate colors
    * color2_1 is 2/3 color0 + 1/3 color1
    * color3_1 is 1/3 color0 + 2/3 color1
    * color2_2 is 1/2 color0 + 1/2 color1
    * color3_2 is 0
    */

   colors0 = LLVMBuildBitCast(builder, color0, bld8.vec_type, "");
   colors1 = LLVMBuildBitCast(builder, color1, bld8.vec_type, "");
   /* can combine 2 lerps into one mostly - still looks expensive enough. */
   lp_build_lerp23(&bld8, colors0, colors1, &color2, &color3);
   color2 = LLVMBuildBitCast(builder, color2, bld32.vec_type, "");
   color3 = LLVMBuildBitCast(builder, color3, bld32.vec_type, "");

   /* dxt3/5 always use 4-color encoding */
   if (is_dxt1_variant) {
      /* fix up alpha */
      if (format == PIPE_FORMAT_DXT1_RGBA ||
          format == PIPE_FORMAT_DXT1_SRGBA) {
         color0 = LLVMBuildOr(builder, color0, a, "");
         color1 = LLVMBuildOr(builder, color1, a, "");
         color3 = LLVMBuildOr(builder, color3, a, "");
      }
      /*
       * XXX with sse2 and 16x8 vectors, should use pavgb even when n == 1.
       * Much cheaper (but we don't care that much if n == 1).
       */
      if ((util_get_cpu_caps()->has_sse2 && n == 4) ||
          (util_get_cpu_caps()->has_avx2 && n == 8)) {
         color2_2 = lp_build_pavgb(&bld8, colors0, colors1);
         color2_2 = LLVMBuildBitCast(builder, color2_2, bld32.vec_type, "");
      }
      else {
         struct lp_type i16_type = lp_wider_type(type8);
         struct lp_build_context bld2;
         LLVMValueRef v0_lo, v0_hi, v1_lo, v1_hi, addlo, addhi;

         lp_build_context_init(&bld2, gallivm, i16_type);
         bld2.type.sign = true;

         /*
          * This isn't as expensive as it looks (the unpack is the same as
          * for lerp23), with correct rounding.
          * (Note that while rounding is correct, this will always round down,
          * whereas pavgb will always round up.)
          */
         /* FIXME: use native avx256 unpack/pack */
         lp_build_unpack2(gallivm, type8, i16_type, colors0, &v0_lo, &v0_hi);
         lp_build_unpack2(gallivm, type8, i16_type, colors1, &v1_lo, &v1_hi);

         addlo = lp_build_add(&bld2, v0_lo, v1_lo);
         addhi = lp_build_add(&bld2, v0_hi, v1_hi);
         addlo = LLVMBuildLShr(builder, addlo,
                               lp_build_const_int_vec(gallivm, i16_type, 1), "");
         addhi = LLVMBuildLShr(builder, addhi,
                               lp_build_const_int_vec(gallivm, i16_type, 1), "");
         color2_2 = lp_build_pack2(gallivm, i16_type, type8, addlo, addhi);
         color2_2 = LLVMBuildBitCast(builder, color2_2, bld32.vec_type, "");
      }
      color3_2 = lp_build_const_int_vec(gallivm, type, 0);

      /* select between colors2/3 */
      /* signed compare is faster saves some xors */
      type.sign = true;
      sel_mask = lp_build_compare(gallivm, type, PIPE_FUNC_GREATER, col0, col1);
      color2 = lp_build_select(&bld32, sel_mask, color2, color2_2);
      color3 = lp_build_select(&bld32, sel_mask, color3, color3_2);
      type.sign = false;

      if (format == PIPE_FORMAT_DXT1_RGBA ||
          format == PIPE_FORMAT_DXT1_SRGBA) {
         color2 = LLVMBuildOr(builder, color2, a, "");
      }
   }

   const2 = lp_build_const_int_vec(gallivm, type, 2);
   /* extract 2-bit index values */
   bit_pos = LLVMBuildShl(builder, j, const2, "");
   bit_pos = LLVMBuildAdd(builder, bit_pos, i, "");
   bit_pos = LLVMBuildAdd(builder, bit_pos, bit_pos, "");
   /*
    * NOTE: This innocent looking shift is very expensive with x86/ssex.
    * Shifts with per-elemnent shift count get roughly translated to
    * extract (count), extract (value), shift, move (back to xmm), unpack
    * per element!
    * So about 20 instructions here for 4xi32.
    * Newer llvm versions (3.7+) will not do extract/insert but use a
    * a couple constant count vector shifts plus shuffles. About same
    * amount of instructions unfortunately...
    * Would get much worse with 8xi16 even...
    * We could actually do better here:
    * - subtract bit_pos from 128+30, shl 23, convert float to int...
    * - now do mul with codewords followed by shr 30...
    * But requires 32bit->32bit mul, sse41 only (well that's emulatable
    * with 2 32bit->64bit muls...) and not exactly cheap
    * AVX2, of course, fixes this nonsense.
    */
   indices = LLVMBuildLShr(builder, codewords, bit_pos, "");

   /* finally select the colors */
   sel_lo = LLVMBuildAnd(builder, indices, bld32.one, "");
   sel_lo = lp_build_compare(gallivm, type, PIPE_FUNC_EQUAL, sel_lo, bld32.one);
   color0 = lp_build_select(&bld32, sel_lo, color1, color0);
   color2 = lp_build_select(&bld32, sel_lo, color3, color2);
   sel_hi = LLVMBuildAnd(builder, indices, const2, "");
   sel_hi = lp_build_compare(gallivm, type, PIPE_FUNC_EQUAL, sel_hi, const2);
   rgba = lp_build_select(&bld32, sel_hi, color2, color0);

   /* fix up alpha */
   if (format == PIPE_FORMAT_DXT1_RGB ||
       format == PIPE_FORMAT_DXT1_SRGB) {
      rgba = LLVMBuildOr(builder, rgba, a, "");
   }
   return LLVMBuildBitCast(builder, rgba, bld8.vec_type, "");
}


static LLVMValueRef
s3tc_dxt1_to_rgba_aos(struct gallivm_state *gallivm,
                      unsigned n,
                      enum pipe_format format,
                      LLVMValueRef colors,
                      LLVMValueRef codewords,
                      LLVMValueRef i,
                      LLVMValueRef j)
{
   return s3tc_dxt1_full_to_rgba_aos(gallivm, n, format,
                                     colors, codewords, i, j);
}


/**
 * Convert from <n x i128> s3tc dxt3 to <4n x i8> RGBA AoS
 * @param colors  is a <n x i32> vector with n x 2x16bit colors
 * @param codewords  is a <n x i32> vector containing the codewords
 * @param alphas  is a <n x i64> vector containing the alpha values
 * @param i  is a <n x i32> vector with the x pixel coordinate (0 to 3)
 * @param j  is a <n x i32> vector with the y pixel coordinate (0 to 3)
 */
static LLVMValueRef
s3tc_dxt3_to_rgba_aos(struct gallivm_state *gallivm,
                      unsigned n,
                      enum pipe_format format,
                      LLVMValueRef colors,
                      LLVMValueRef codewords,
                      LLVMValueRef alpha_low,
                      LLVMValueRef alpha_hi,
                      LLVMValueRef i,
                      LLVMValueRef j)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef rgba, tmp, tmp2;
   LLVMValueRef bit_pos, sel_mask;
   struct lp_type type, type8;
   struct lp_build_context bld;

   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;

   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = n*4;

   assert(lp_check_value(type, i));
   assert(lp_check_value(type, j));

   lp_build_context_init(&bld, gallivm, type);

   rgba = s3tc_dxt1_to_rgba_aos(gallivm, n, format,
                                colors, codewords, i, j);

   rgba = LLVMBuildBitCast(builder, rgba, bld.vec_type, "");

   /*
    * Extract alpha values. Since we now need to select from
    * which 32bit vector values are fetched, construct selection
    * mask from highest bit of bit_pos, and use select, then shift
    * according to the bit_pos (without the highest bit).
    * Note this is pointless for n == 1 case. Could just
    * directly use 64bit arithmetic if we'd extract 64bit
    * alpha value instead of 2x32...
    */
   /* pos = 4*(4j+i) */
   bit_pos = LLVMBuildShl(builder, j, lp_build_const_int_vec(gallivm, type, 2), "");
   bit_pos = LLVMBuildAdd(builder, bit_pos, i, "");
   bit_pos = LLVMBuildShl(builder, bit_pos,
                          lp_build_const_int_vec(gallivm, type, 2), "");
   sel_mask = LLVMBuildLShr(builder, bit_pos,
                            lp_build_const_int_vec(gallivm, type, 5), "");
   sel_mask = LLVMBuildSub(builder, sel_mask, bld.one, "");
   tmp = lp_build_select(&bld, sel_mask, alpha_low, alpha_hi);
   bit_pos = LLVMBuildAnd(builder, bit_pos,
                          lp_build_const_int_vec(gallivm, type, 0xffffffdf), "");
   /* Warning: slow shift with per element count (without avx2) */
   /*
    * Could do pshufb here as well - just use appropriate 2 bits in bit_pos
    * to select the right byte with pshufb. Then for the remaining one bit
    * just do shift/select.
    */
   tmp = LLVMBuildLShr(builder, tmp, bit_pos, "");

   /* combined expand from a4 to a8 and shift into position */
   tmp = LLVMBuildShl(builder, tmp, lp_build_const_int_vec(gallivm, type, 28), "");
   tmp2 = LLVMBuildLShr(builder, tmp, lp_build_const_int_vec(gallivm, type, 4), "");
   tmp = LLVMBuildOr(builder, tmp, tmp2, "");

   rgba = LLVMBuildOr(builder, tmp, rgba, "");

   return LLVMBuildBitCast(builder, rgba, lp_build_vec_type(gallivm, type8), "");
}

static LLVMValueRef
lp_build_lerpdxta(struct gallivm_state *gallivm,
                  LLVMValueRef alpha0,
                  LLVMValueRef alpha1,
                  LLVMValueRef code,
                  LLVMValueRef sel_mask,
                  unsigned n)
{
   /*
    * note we're doing lerp in 16bit since 32bit pmulld is only available in sse41
    * (plus pmullw is actually faster...)
    * we just pretend our 32bit values (which are really only 8bit) are 16bits.
    * Note that this is obviously a disaster for the scalar case.
    */
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef delta, ainterp;
   LLVMValueRef weight5, weight7, weight;
   struct lp_type type32, type16, type8;
   struct lp_build_context bld16;

   memset(&type32, 0, sizeof type32);
   type32.width = 32;
   type32.length = n;
   memset(&type16, 0, sizeof type16);
   type16.width = 16;
   type16.length = 2*n;
   type16.sign = true;
   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = 4*n;

   lp_build_context_init(&bld16, gallivm, type16);
   /* 255/7 is a bit off - increase accuracy at the expense of shift later */
   sel_mask = LLVMBuildBitCast(builder, sel_mask, bld16.vec_type, "");
   weight5 = lp_build_const_int_vec(gallivm, type16, 255*64/5);
   weight7 = lp_build_const_int_vec(gallivm, type16, 255*64/7);
   weight = lp_build_select(&bld16, sel_mask, weight7, weight5);

   alpha0 = LLVMBuildBitCast(builder, alpha0, bld16.vec_type, "");
   alpha1 = LLVMBuildBitCast(builder, alpha1, bld16.vec_type, "");
   code = LLVMBuildBitCast(builder, code, bld16.vec_type, "");
   /* we'll get garbage in the elements which had code 0 (or larger than 5 or 7)
      but we don't care */
   code = LLVMBuildSub(builder, code, bld16.one, "");

   weight = LLVMBuildMul(builder, weight, code, "");
   weight = LLVMBuildLShr(builder, weight,
                          lp_build_const_int_vec(gallivm, type16, 6), "");

   delta = LLVMBuildSub(builder, alpha1, alpha0, "");

   ainterp = LLVMBuildMul(builder, delta, weight, "");
   ainterp = LLVMBuildLShr(builder, ainterp,
                           lp_build_const_int_vec(gallivm, type16, 8), "");

   ainterp = LLVMBuildBitCast(builder, ainterp, lp_build_vec_type(gallivm, type8), "");
   alpha0 = LLVMBuildBitCast(builder, alpha0, lp_build_vec_type(gallivm, type8), "");
   ainterp = LLVMBuildAdd(builder, alpha0, ainterp, "");
   ainterp = LLVMBuildBitCast(builder, ainterp, lp_build_vec_type(gallivm, type32), "");

   return ainterp;
}

static LLVMValueRef
s3tc_dxt5_alpha_channel(struct gallivm_state *gallivm,
                        bool is_signed,
                        unsigned n,
                        LLVMValueRef alpha_hi, LLVMValueRef alpha_lo,
                        LLVMValueRef i, LLVMValueRef j)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type type, type8;
   LLVMValueRef tmp, alpha0, alpha1, alphac, alphac0, bit_pos, shift;
   LLVMValueRef sel_mask, tmp_mask, alpha, alpha64, code_s;
   LLVMValueRef mask6, mask7, ainterp;
   LLVMTypeRef i64t = LLVMInt64TypeInContext(gallivm->context);
   LLVMTypeRef i32t = LLVMInt32TypeInContext(gallivm->context);
   struct lp_build_context bld32;

   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;

   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = n;
   type8.sign = is_signed;

   lp_build_context_init(&bld32, gallivm, type);
   /* this looks pretty complex for vectorization:
    * extract a0/a1 values
    * extract code
    * select weights for interpolation depending on a0 > a1
    * mul weights by code - 1
    * lerp a0/a1/weights
    * use selects for getting either a0, a1, interp a, interp a/0.0, interp a/1.0
    */

   alpha0 = LLVMBuildAnd(builder, alpha_lo,
                         lp_build_const_int_vec(gallivm, type, 0xff), "");
   if (is_signed) {
      alpha0 = LLVMBuildTrunc(builder, alpha0, lp_build_vec_type(gallivm, type8), "");
      alpha0 = LLVMBuildSExt(builder, alpha0, lp_build_vec_type(gallivm, type), "");
   }

   alpha1 = LLVMBuildLShr(builder, alpha_lo,
                          lp_build_const_int_vec(gallivm, type, 8), "");
   alpha1 = LLVMBuildAnd(builder, alpha1,
                         lp_build_const_int_vec(gallivm, type, 0xff), "");
   if (is_signed) {
      alpha1 = LLVMBuildTrunc(builder, alpha1, lp_build_vec_type(gallivm, type8), "");
      alpha1 = LLVMBuildSExt(builder, alpha1, lp_build_vec_type(gallivm, type), "");
   }

   /* pos = 3*(4j+i) */
   bit_pos = LLVMBuildShl(builder, j, lp_build_const_int_vec(gallivm, type, 2), "");
   bit_pos = LLVMBuildAdd(builder, bit_pos, i, "");
   tmp = LLVMBuildAdd(builder, bit_pos, bit_pos, "");
   bit_pos = LLVMBuildAdd(builder, bit_pos, tmp, "");
   /* get rid of first 2 bytes - saves shifts of alpha_lo/hi */
   bit_pos = LLVMBuildAdd(builder, bit_pos,
                          lp_build_const_int_vec(gallivm, type, 16), "");

   if (n == 1) {
      struct lp_type type64;
      memset(&type64, 0, sizeof type64);
      type64.width = 64;
      type64.length = 1;
      /* This is pretty pointless could avoid by just directly extracting
         64bit in the first place but makes it more complicated elsewhere */
      alpha_lo = LLVMBuildZExt(builder, alpha_lo, i64t, "");
      alpha_hi = LLVMBuildZExt(builder, alpha_hi, i64t, "");
      alphac0 = LLVMBuildShl(builder, alpha_hi,
                             lp_build_const_int_vec(gallivm, type64, 32), "");
      alphac0 = LLVMBuildOr(builder, alpha_lo, alphac0, "");

      shift = LLVMBuildZExt(builder, bit_pos, i64t, "");
      alphac0 = LLVMBuildLShr(builder, alphac0, shift, "");
      alphac0 = LLVMBuildTrunc(builder, alphac0, i32t, "");
      alphac = LLVMBuildAnd(builder, alphac0,
                            lp_build_const_int_vec(gallivm, type, 0x7), "");
   }
   else {
      /*
       * Using non-native vector length here (actually, with avx2 and
       * n == 4 llvm will indeed expand to ymm regs...)
       * At least newer llvm versions handle that ok.
       * llvm 3.7+ will even handle the emulated 64bit shift with variable
       * shift count without extraction (and it's actually easier to
       * emulate than the 32bit one).
       */
      alpha64 = LLVMBuildShuffleVector(builder, alpha_lo, alpha_hi,
                                       lp_build_const_unpackx2_shuffle(gallivm, n), "");

      alpha64 = LLVMBuildBitCast(builder, alpha64, LLVMVectorType(i64t, n), "");
      shift = LLVMBuildZExt(builder, bit_pos, LLVMVectorType(i64t, n), "");
      alphac = LLVMBuildLShr(builder, alpha64, shift, "");
      alphac = LLVMBuildTrunc(builder, alphac, bld32.vec_type, "");

      alphac = LLVMBuildAnd(builder, alphac,
                            lp_build_const_int_vec(gallivm, type, 0x7), "");
   }

   /* signed compare is faster saves some xors */
   type.sign = true;
   /* alpha0 > alpha1 selection */
   sel_mask = lp_build_compare(gallivm, type, PIPE_FUNC_GREATER,
                               alpha0, alpha1);
   ainterp = lp_build_lerpdxta(gallivm, alpha0, alpha1, alphac, sel_mask, n);

   /*
    * if a0 > a1 then we select a0 for case 0, a1 for case 1, interp otherwise.
    * else we select a0 for case 0, a1 for case 1,
    * interp for case 2-5, 00 for 6 and 0xff(ffffff) for 7
    * a = (c == 0) ? a0 : a1
    * a = (c > 1) ? ainterp : a
    * Finally handle case 6/7 for !(a0 > a1)
    * a = (!(a0 > a1) && c == 6) ? 0 : a (andnot with mask)
    * a = (!(a0 > a1) && c == 7) ? 0xffffffff : a (or with mask)
    */
   tmp_mask = lp_build_compare(gallivm, type, PIPE_FUNC_EQUAL,
                               alphac, bld32.zero);
   alpha = lp_build_select(&bld32, tmp_mask, alpha0, alpha1);
   tmp_mask = lp_build_compare(gallivm, type, PIPE_FUNC_GREATER,
                               alphac, bld32.one);
   alpha = lp_build_select(&bld32, tmp_mask, ainterp, alpha);

   code_s = LLVMBuildAnd(builder, alphac,
                         LLVMBuildNot(builder, sel_mask, ""), "");
   mask6 = lp_build_compare(gallivm, type, PIPE_FUNC_EQUAL,
                            code_s, lp_build_const_int_vec(gallivm, type, 6));
   mask7 = lp_build_compare(gallivm, type, PIPE_FUNC_EQUAL,
                            code_s, lp_build_const_int_vec(gallivm, type, 7));
   if (is_signed) {
      alpha = lp_build_select(&bld32, mask6, lp_build_const_int_vec(gallivm, type, -127), alpha);
      alpha = lp_build_select(&bld32, mask7, lp_build_const_int_vec(gallivm, type, 127), alpha);
   } else {
      alpha = LLVMBuildAnd(builder, alpha, LLVMBuildNot(builder, mask6, ""), "");
      alpha = LLVMBuildOr(builder, alpha, mask7, "");
   }
   /* There can be garbage in upper bits, mask them off for rgtc formats */
   alpha = LLVMBuildAnd(builder, alpha, lp_build_const_int_vec(gallivm, type, 0xff), "");

   return alpha;
}

/**
 * Convert from <n x i128> s3tc dxt5 to <4n x i8> RGBA AoS
 * @param colors  is a <n x i32> vector with n x 2x16bit colors
 * @param codewords  is a <n x i32> vector containing the codewords
 * @param alphas  is a <n x i64> vector containing the alpha values
 * @param i  is a <n x i32> vector with the x pixel coordinate (0 to 3)
 * @param j  is a <n x i32> vector with the y pixel coordinate (0 to 3)
 */
static LLVMValueRef
s3tc_dxt5_full_to_rgba_aos(struct gallivm_state *gallivm,
                           unsigned n,
                           enum pipe_format format,
                           LLVMValueRef colors,
                           LLVMValueRef codewords,
                           LLVMValueRef alpha_lo,
                           LLVMValueRef alpha_hi,
                           LLVMValueRef i,
                           LLVMValueRef j)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef rgba, alpha;
   struct lp_type type, type8;
   struct lp_build_context bld32;

   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;

   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = n*4;

   assert(lp_check_value(type, i));
   assert(lp_check_value(type, j));

   lp_build_context_init(&bld32, gallivm, type);

   assert(lp_check_value(type, i));
   assert(lp_check_value(type, j));

   rgba = s3tc_dxt1_to_rgba_aos(gallivm, n, format,
                                colors, codewords, i, j);

   rgba = LLVMBuildBitCast(builder, rgba, bld32.vec_type, "");

   alpha = s3tc_dxt5_alpha_channel(gallivm, false, n, alpha_hi, alpha_lo, i, j);
   alpha = LLVMBuildShl(builder, alpha, lp_build_const_int_vec(gallivm, type, 24), "");
   rgba = LLVMBuildOr(builder, alpha, rgba, "");

   return LLVMBuildBitCast(builder, rgba, lp_build_vec_type(gallivm, type8), "");
}


static void
lp_build_gather_s3tc_simple_scalar(struct gallivm_state *gallivm,
                                   const struct util_format_description *format_desc,
                                   LLVMValueRef *dxt_block,
                                   LLVMValueRef ptr)
{
   LLVMBuilderRef builder = gallivm->builder;
   unsigned block_bits = format_desc->block.bits;
   LLVMValueRef elem, shuf;
   LLVMTypeRef type32 = LLVMIntTypeInContext(gallivm->context, 32);
   LLVMTypeRef src_type = LLVMIntTypeInContext(gallivm->context, block_bits);
   LLVMTypeRef type32_4 = LLVMVectorType(type32, 4);

   assert(block_bits == 64 || block_bits == 128);

   ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(src_type, 0), "");
   elem = LLVMBuildLoad2(builder, src_type, ptr, "");

   if (block_bits == 128) {
      /* just return block as is */
      *dxt_block = LLVMBuildBitCast(builder, elem, type32_4, "");
   }
   else {
      LLVMTypeRef type32_2 = LLVMVectorType(type32, 2);
      shuf = lp_build_const_extend_shuffle(gallivm, 2, 4);
      elem = LLVMBuildBitCast(builder, elem, type32_2, "");
      *dxt_block = LLVMBuildShuffleVector(builder, elem,
                                          LLVMGetUndef(type32_2), shuf, "");
   }
}


static void
s3tc_store_cached_block(struct gallivm_state *gallivm,
                        LLVMValueRef *col,
                        LLVMValueRef tag_value,
                        LLVMValueRef hash_index,
                        LLVMValueRef cache)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef ptr, indices[3];
   LLVMTypeRef type_ptr4x32;
   unsigned count;

   type_ptr4x32 = LLVMPointerType(LLVMVectorType(LLVMInt32TypeInContext(gallivm->context), 4), 0);
   indices[0] = lp_build_const_int32(gallivm, 0);
   indices[1] = lp_build_const_int32(gallivm, LP_BUILD_FORMAT_CACHE_MEMBER_TAGS);
   indices[2] = hash_index;
   LLVMTypeRef cache_type = lp_build_format_cache_type(gallivm);
   ptr = LLVMBuildGEP2(builder, cache_type, cache, indices, ARRAY_SIZE(indices), "");
   LLVMBuildStore(builder, tag_value, ptr);

   indices[1] = lp_build_const_int32(gallivm, LP_BUILD_FORMAT_CACHE_MEMBER_DATA);
   hash_index = LLVMBuildMul(builder, hash_index, lp_build_const_int32(gallivm, 16), "");
   for (count = 0; count < 4; count++) {
      indices[2] = hash_index;
      ptr = LLVMBuildGEP2(builder, cache_type, cache, indices, ARRAY_SIZE(indices), "");
      ptr = LLVMBuildBitCast(builder, ptr, type_ptr4x32, "");
      LLVMBuildStore(builder, col[count], ptr);
      hash_index = LLVMBuildAdd(builder, hash_index, lp_build_const_int32(gallivm, 4), "");
   }
}

static LLVMValueRef
lookup_cache_member(struct gallivm_state *gallivm, LLVMValueRef cache, enum cache_member member, LLVMValueRef index) {
   assert(member == LP_BUILD_FORMAT_CACHE_MEMBER_DATA || member == LP_BUILD_FORMAT_CACHE_MEMBER_TAGS);
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef member_ptr, indices[3];

   indices[0] = lp_build_const_int32(gallivm, 0);
   indices[1] = lp_build_const_int32(gallivm, member);
   indices[2] = index;

   const char *name =
         member == LP_BUILD_FORMAT_CACHE_MEMBER_DATA ? "cache_data" :
         member == LP_BUILD_FORMAT_CACHE_MEMBER_TAGS ? "tag_data" : "";

   member_ptr = LLVMBuildGEP2(builder, lp_build_format_cache_type(gallivm),
                              cache, indices, ARRAY_SIZE(indices), "cache_gep");

   return LLVMBuildLoad2(builder, lp_build_format_cache_elem_type(gallivm, member), member_ptr, name);
}

static LLVMValueRef
s3tc_lookup_cached_pixel(struct gallivm_state *gallivm,
                         LLVMValueRef cache,
                         LLVMValueRef index)
{
   return lookup_cache_member(gallivm, cache, LP_BUILD_FORMAT_CACHE_MEMBER_DATA, index);
}

static LLVMValueRef
s3tc_lookup_tag_data(struct gallivm_state *gallivm,
                     LLVMValueRef cache,
                     LLVMValueRef index)
{
   return lookup_cache_member(gallivm, cache, LP_BUILD_FORMAT_CACHE_MEMBER_TAGS, index);
}

#if LP_BUILD_FORMAT_CACHE_DEBUG
static void
s3tc_update_cache_access(struct gallivm_state *gallivm,
                         LLVMValueRef ptr,
                         unsigned count,
                         unsigned index)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef member_ptr, cache_access;

   assert(index == LP_BUILD_FORMAT_CACHE_MEMBER_ACCESS_TOTAL ||
          index == LP_BUILD_FORMAT_CACHE_MEMBER_ACCESS_MISS);
   LLVMTypeRef cache_type = lp_build_format_cache_type(gallivm);
   member_ptr = lp_build_struct_get_ptr2(gallivm, cache_type, ptr, index, "");
   cache_access = LLVMBuildLoad2(builder, LLVMInt64TypeInContext(gallivm->context), member_ptr, "cache_access");
   cache_access = LLVMBuildAdd(builder, cache_access,
                               LLVMConstInt(LLVMInt64TypeInContext(gallivm->context), count, 0), "");
   LLVMBuildStore(builder, cache_access, member_ptr);
}
#endif

/** 
 * Calculate 1/3(v1-v0) + v0 and 2*1/3(v1-v0) + v0.
 * The lerp is performed between the first 2 32bit colors
 * in the source vector, both results are returned packed in result vector.
 */
static LLVMValueRef
lp_build_lerp23_single(struct lp_build_context *bld,
                       LLVMValueRef v01)
{
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMValueRef x, mul, delta, res, v0, v1, elems[8];
   const struct lp_type type = bld->type;
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_type i16_type = lp_wider_type(type);
   struct lp_type i32_type = lp_wider_type(i16_type);
   struct lp_build_context bld2;

   assert(!type.floating && !type.fixed && !type.norm && type.width == 8);

   lp_build_context_init(&bld2, gallivm, i16_type);
   bld2.type.sign = true;

   /* weights 256/3, 256*2/3, with correct rounding */
   elems[0] = elems[1] = elems[2] = elems[3] =
      lp_build_const_elem(gallivm, i16_type, 255*1/3);
   elems[4] = elems[5] = elems[6] = elems[7] =
      lp_build_const_elem(gallivm, i16_type, 171);
   x = LLVMConstVector(elems, 8);

   /*
    * v01 has col0 in 32bit elem 0, col1 in elem 1.
    * Interleave/unpack will give us separate v0/v1 vectors.
    */
   v01 = lp_build_interleave2(gallivm, i32_type, v01, v01, 0);
   v01 = LLVMBuildBitCast(builder, v01, bld->vec_type, "");

   lp_build_unpack2(gallivm, type, i16_type, v01, &v0, &v1);
   delta = lp_build_sub(&bld2, v1, v0);

   mul = LLVMBuildMul(builder, x, delta, "");

   mul = LLVMBuildLShr(builder, mul, lp_build_const_int_vec(gallivm, i16_type, 8), "");
   /* lerp optimization: pack now, do add afterwards */
   res = lp_build_pack2(gallivm, i16_type, type, mul, bld2.undef);
   /* only lower 2 elems are valid - for these v0 is really v0 */
   return lp_build_add(bld, res, v01);
}

/*
 * decode one dxt1 block.
 */
static void
s3tc_decode_block_dxt1(struct gallivm_state *gallivm,
                       enum pipe_format format,
                       LLVMValueRef dxt_block,
                       LLVMValueRef *col)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef color01, color23, color01_16, color0123;
   LLVMValueRef rgba, tmp, a, sel_mask, indices, code, const2;
   struct lp_type type8, type32, type16, type64;
   struct lp_build_context bld8, bld32, bld16, bld64;
   unsigned i;
   bool is_dxt1_variant = format_dxt1_variant(format);

   memset(&type32, 0, sizeof type32);
   type32.width = 32;
   type32.length = 4;
   type32.sign = true;

   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = 16;

   memset(&type16, 0, sizeof type16);
   type16.width = 16;
   type16.length = 8;

   memset(&type64, 0, sizeof type64);
   type64.width = 64;
   type64.length = 2;

   a = lp_build_const_int_vec(gallivm, type32, 0xff000000);
   const2 = lp_build_const_int_vec(gallivm, type32, 2);

   lp_build_context_init(&bld32, gallivm, type32);
   lp_build_context_init(&bld16, gallivm, type16);
   lp_build_context_init(&bld8, gallivm, type8);
   lp_build_context_init(&bld64, gallivm, type64);

   if (is_dxt1_variant) {
      color01 = lp_build_shuffle1undef(gallivm, dxt_block, 0, 4);
      code = lp_build_shuffle1undef(gallivm, dxt_block, 1, 4);
   } else {
      color01 = lp_build_shuffle1undef(gallivm, dxt_block, 2, 4);
      code = lp_build_shuffle1undef(gallivm, dxt_block, 3, 4);
   }
   code = LLVMBuildBitCast(builder, code, bld8.vec_type, "");
   /* expand bytes to dwords */
   code = lp_build_interleave2(gallivm, type8, code, code, 0);
   code = lp_build_interleave2(gallivm, type8, code, code, 0);


   /*
    * works as follows:
    * - expand color0/color1 to rgba8888
    * - calculate color2/3 (interpolation) according to color0 < color1 rules
    * - calculate color2/3 according to color0 >= color1 rules
    * - do selection of color2/3 according to comparison of color0/1
    * - extract indices.
    * - use compare/select to select the correct color. Since we have 2bit
    *   indices (and 4 colors), needs at least three compare/selects.
    */

   /*
    * expand the two colors
    */
   color01 = LLVMBuildBitCast(builder, color01, bld16.vec_type, "");
   color01 = lp_build_interleave2(gallivm, type16, color01,
                                  bld16.zero, 0);
   color01_16 = LLVMBuildBitCast(builder, color01, bld32.vec_type, "");
   color01 = color_expand_565_to_8888(gallivm, 4, color01_16);

   /*
    * interpolate colors
    * color2_1 is 2/3 color0 + 1/3 color1
    * color3_1 is 1/3 color0 + 2/3 color1
    * color2_2 is 1/2 color0 + 1/2 color1
    * color3_2 is 0
    */

   /* TODO: since this is now always scalar, should
    * probably just use control flow here instead of calculating
    * both cases and then selection
    */
   if (format == PIPE_FORMAT_DXT1_RGBA ||
       format == PIPE_FORMAT_DXT1_SRGBA) {
      color01 = LLVMBuildOr(builder, color01, a, "");
   }
   /* can combine 2 lerps into one mostly */
   color23 = lp_build_lerp23_single(&bld8, color01);
   color23 = LLVMBuildBitCast(builder, color23, bld32.vec_type, "");

   /* dxt3/5 always use 4-color encoding */
   if (is_dxt1_variant) {
      LLVMValueRef color23_2, color2_2;

      if (util_get_cpu_caps()->has_sse2) {
         LLVMValueRef intrargs[2];
         intrargs[0] = LLVMBuildBitCast(builder, color01, bld8.vec_type, "");
         /* same interleave as for lerp23 - correct result in 2nd element */
         intrargs[1] = lp_build_interleave2(gallivm, type32, color01, color01, 0);
         intrargs[1] = LLVMBuildBitCast(builder, intrargs[1], bld8.vec_type, "");
         color2_2 = lp_build_pavgb(&bld8, intrargs[0], intrargs[1]);
      }
      else {
         LLVMValueRef v01, v0, v1, vhalf;
         /*
          * This isn't as expensive as it looks (the unpack is the same as
          * for lerp23, which is the reason why we do the pointless
          * interleave2 too), with correct rounding (the two lower elements
          * will be the same).
          */
         v01 = lp_build_interleave2(gallivm, type32, color01, color01, 0);
         v01 = LLVMBuildBitCast(builder, v01, bld8.vec_type, "");
         lp_build_unpack2(gallivm, type8, type16, v01, &v0, &v1);
         vhalf = lp_build_add(&bld16, v0, v1);
         vhalf = LLVMBuildLShr(builder, vhalf, bld16.one, "");
         color2_2 = lp_build_pack2(gallivm, type16, type8, vhalf, bld16.undef);
      }
      /* shuffle in color 3 as elem 2 zero, color 2 elem 1 */
      color23_2 = LLVMBuildBitCast(builder, color2_2, bld64.vec_type, "");
      color23_2 = LLVMBuildLShr(builder, color23_2,
                                lp_build_const_int_vec(gallivm, type64, 32), "");
      color23_2 = LLVMBuildBitCast(builder, color23_2, bld32.vec_type, "");

      tmp = LLVMBuildBitCast(builder, color01_16, bld64.vec_type, "");
      tmp = LLVMBuildLShr(builder, tmp,
                          lp_build_const_int_vec(gallivm, type64, 32), "");
      tmp = LLVMBuildBitCast(builder, tmp, bld32.vec_type, "");
      sel_mask = lp_build_compare(gallivm, type32, PIPE_FUNC_GREATER,
                                  color01_16, tmp);
      sel_mask = lp_build_interleave2(gallivm, type32, sel_mask, sel_mask, 0);
      color23 = lp_build_select(&bld32, sel_mask, color23, color23_2);
   }

   if (util_get_cpu_caps()->has_ssse3) {
      /*
       * Use pshufb as mini-lut. (Only doable with intrinsics as the
       * final shuffles are non-constant. pshufb is awesome!)
       */
      LLVMValueRef shuf[16], low2mask;
      LLVMValueRef intrargs[2], lut_ind, lut_adj;

      color01 = LLVMBuildBitCast(builder, color01, bld64.vec_type, "");
      color23 = LLVMBuildBitCast(builder, color23, bld64.vec_type, "");
      color0123 = lp_build_interleave2(gallivm, type64, color01, color23, 0);
      color0123 = LLVMBuildBitCast(builder, color0123, bld32.vec_type, "");

      if (format == PIPE_FORMAT_DXT1_RGB ||
          format == PIPE_FORMAT_DXT1_SRGB) {
         color0123 = LLVMBuildOr(builder, color0123, a, "");
      }

      /* shuffle as r0r1r2r3g0g1... */
      for (i = 0; i < 4; i++) {
         shuf[4*i] = lp_build_const_int32(gallivm, 0 + i);
         shuf[4*i+1] = lp_build_const_int32(gallivm, 4 + i);
         shuf[4*i+2] = lp_build_const_int32(gallivm, 8 + i);
         shuf[4*i+3] = lp_build_const_int32(gallivm, 12 + i);
      }
      color0123 = LLVMBuildBitCast(builder, color0123, bld8.vec_type, "");
      color0123 = LLVMBuildShuffleVector(builder, color0123, bld8.undef,
                                         LLVMConstVector(shuf, 16), "");

      /* lowest 2 bits of each 8 bit value contain index into "LUT" */
      low2mask = lp_build_const_int_vec(gallivm, type8, 3);
      /* add 0/4/8/12 for r/g/b/a */
      lut_adj = lp_build_const_int_vec(gallivm, type32, 0x0c080400);
      lut_adj = LLVMBuildBitCast(builder, lut_adj, bld8.vec_type, "");
      intrargs[0] = color0123;
      for (i = 0; i < 4; i++) {
         lut_ind = LLVMBuildAnd(builder, code, low2mask, "");
         lut_ind = LLVMBuildOr(builder, lut_ind, lut_adj, "");
         intrargs[1] = lut_ind;
         col[i] = lp_build_intrinsic(builder, "llvm.x86.ssse3.pshuf.b.128",
                                     bld8.vec_type, intrargs, 2, 0);
         col[i] = LLVMBuildBitCast(builder, col[i], bld32.vec_type, "");
         code = LLVMBuildBitCast(builder, code, bld32.vec_type, "");
         code = LLVMBuildLShr(builder, code, const2, "");
         code = LLVMBuildBitCast(builder, code, bld8.vec_type, "");
      }
   }
   else {
      /* Thanks to vectorization can do 4 texels in parallel */
      LLVMValueRef color0, color1, color2, color3;
      if (format == PIPE_FORMAT_DXT1_RGB ||
          format == PIPE_FORMAT_DXT1_SRGB) {
         color01 = LLVMBuildOr(builder, color01, a, "");
         color23 = LLVMBuildOr(builder, color23, a, "");
      }
      color0 = LLVMBuildShuffleVector(builder, color01, bld32.undef,
                                      lp_build_const_shuffle1(gallivm, 0, 4), "");
      color1 = LLVMBuildShuffleVector(builder, color01, bld32.undef,
                                      lp_build_const_shuffle1(gallivm, 1, 4), "");
      color2 = LLVMBuildShuffleVector(builder, color23, bld32.undef,
                                      lp_build_const_shuffle1(gallivm, 0, 4), "");
      color3 = LLVMBuildShuffleVector(builder, color23, bld32.undef,
                                      lp_build_const_shuffle1(gallivm, 1, 4), "");
      code = LLVMBuildBitCast(builder, code, bld32.vec_type, "");

      for (i = 0; i < 4; i++) {
         /* select the colors */
         LLVMValueRef selmasklo, rgba01, rgba23, bitlo;
         bitlo = bld32.one;
         indices = LLVMBuildAnd(builder, code, bitlo, "");
         selmasklo = lp_build_compare(gallivm, type32, PIPE_FUNC_EQUAL,
                                      indices, bitlo);
         rgba01 = lp_build_select(&bld32, selmasklo, color1, color0);

         LLVMValueRef selmaskhi;
         indices = LLVMBuildAnd(builder, code, const2, "");
         selmaskhi = lp_build_compare(gallivm, type32, PIPE_FUNC_EQUAL,
                                      indices, const2);
         rgba23 = lp_build_select(&bld32, selmasklo, color3, color2);
         rgba = lp_build_select(&bld32, selmaskhi, rgba23, rgba01);

         /*
          * Note that this will give "wrong" order.
          * col0 will be rgba0, rgba4, rgba8, rgba12, col1 rgba1, rgba5, ...
          * This would be easily fixable by using different shuffle, bitlo/hi
          * vectors above (and different shift), but seems slightly easier to
          * deal with for dxt3/dxt5 alpha too. So instead change lookup.
          */
         col[i] = rgba;
         code = LLVMBuildLShr(builder, code, const2, "");
      }
   }
}

/*
 * decode one dxt3 block.
 */
static void
s3tc_decode_block_dxt3(struct gallivm_state *gallivm,
                       enum pipe_format format,
                       LLVMValueRef dxt_block,
                       LLVMValueRef *col)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef alpha, alphas0, alphas1, shift4_16, a[4], mask8hi;
   struct lp_type type32, type8, type16;
   unsigned i;

   memset(&type32, 0, sizeof type32);
   type32.width = 32;
   type32.length = 4;

   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = 16;

   memset(&type16, 0, sizeof type16);
   type16.width = 16;
   type16.length = 8;

   s3tc_decode_block_dxt1(gallivm, format, dxt_block, col);

   shift4_16 = lp_build_const_int_vec(gallivm, type16, 4);
   mask8hi = lp_build_const_int_vec(gallivm, type32, 0xff000000);

   alpha = LLVMBuildBitCast(builder, dxt_block,
                            lp_build_vec_type(gallivm, type8), "");
   alpha = lp_build_interleave2(gallivm, type8, alpha, alpha, 0);
   alpha = LLVMBuildBitCast(builder, alpha,
                            lp_build_vec_type(gallivm, type16), "");
   alpha = LLVMBuildAnd(builder, alpha,
                        lp_build_const_int_vec(gallivm, type16, 0xf00f), "");
   alphas0 = LLVMBuildLShr(builder, alpha, shift4_16, "");
   alphas1 = LLVMBuildShl(builder, alpha, shift4_16, "");
   alpha = LLVMBuildOr(builder, alphas0, alpha, "");
   alpha = LLVMBuildOr(builder, alphas1, alpha, "");
   alpha = LLVMBuildBitCast(builder, alpha,
                            lp_build_vec_type(gallivm, type32), "");
   /*
    * alpha now contains elems 0,1,2,3,... (ubytes)
    * we need 0,4,8,12, 1,5,9,13 etc. in dwords to match color (which
    * is just as easy as "natural" order - 3 shift/and instead of 6 unpack).
    */
   a[0] = LLVMBuildShl(builder, alpha,
                       lp_build_const_int_vec(gallivm, type32, 24), "");
   a[1] = LLVMBuildShl(builder, alpha,
                       lp_build_const_int_vec(gallivm, type32, 16), "");
   a[1] = LLVMBuildAnd(builder, a[1], mask8hi, "");
   a[2] = LLVMBuildShl(builder, alpha,
                       lp_build_const_int_vec(gallivm, type32, 8), "");
   a[2] = LLVMBuildAnd(builder, a[2], mask8hi, "");
   a[3] = LLVMBuildAnd(builder, alpha, mask8hi, "");

   for (i = 0; i < 4; i++) {
      col[i] = LLVMBuildOr(builder, col[i], a[i], "");
   }
}


static LLVMValueRef
lp_build_lerpdxta_block(struct gallivm_state *gallivm,
                        LLVMValueRef alpha0,
                        LLVMValueRef alpha1,
                        LLVMValueRef code,
                        LLVMValueRef sel_mask)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef delta, ainterp;
   LLVMValueRef weight5, weight7, weight;
   struct lp_type type16;
   struct lp_build_context bld;

   memset(&type16, 0, sizeof type16);
   type16.width = 16;
   type16.length = 8;
   type16.sign = true;

   lp_build_context_init(&bld, gallivm, type16);
   /*
    * 256/7 is only 36.57 so we'd lose quite some precision. Since it would
    * actually be desirable to do this here with even higher accuracy than
    * even 8 bit (more or less required for rgtc, albeit that's not handled
    * here right now), shift the weights after multiplication by code.
    */
   weight5 = lp_build_const_int_vec(gallivm, type16, 256*64/5);
   weight7 = lp_build_const_int_vec(gallivm, type16, 256*64/7);
   weight = lp_build_select(&bld, sel_mask, weight7, weight5);

   /*
    * we'll get garbage in the elements which had code 0 (or larger than
    * 5 or 7) but we don't care (or rather, need to fix up anyway).
    */
   code = LLVMBuildSub(builder, code, bld.one, "");

   weight = LLVMBuildMul(builder, weight, code, "");
   weight = LLVMBuildLShr(builder, weight,
                          lp_build_const_int_vec(gallivm, type16, 6), "");

   delta = LLVMBuildSub(builder, alpha1, alpha0, "");

   ainterp = LLVMBuildMul(builder, delta, weight, "");
   ainterp = LLVMBuildLShr(builder, ainterp,
                           lp_build_const_int_vec(gallivm, type16, 8), "");

   /* lerp is done later (with packed values) */

   return ainterp;
}


/*
 * decode one dxt5 block.
 */
static void
s3tc_decode_block_dxt5(struct gallivm_state *gallivm,
                       enum pipe_format format,
                       LLVMValueRef dxt_block,
                       LLVMValueRef *col)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef alpha, alpha0, alpha1, ares;
   LLVMValueRef ainterp, ainterp0, ainterp1, shuffle1, sel_mask, sel_mask2;
   LLVMValueRef a[4], acode, tmp0, tmp1;
   LLVMTypeRef i64t, i32t;
   struct lp_type type32, type64, type8, type16;
   struct lp_build_context bld16, bld8;
   unsigned i;

   memset(&type32, 0, sizeof type32);
   type32.width = 32;
   type32.length = 4;

   memset(&type64, 0, sizeof type64);
   type64.width = 64;
   type64.length = 2;

   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = 16;

   memset(&type16, 0, sizeof type16);
   type16.width = 16;
   type16.length = 8;

   lp_build_context_init(&bld16, gallivm, type16);
   lp_build_context_init(&bld8, gallivm, type8);

   i64t = lp_build_vec_type(gallivm, type64);
   i32t = lp_build_vec_type(gallivm, type32);

   s3tc_decode_block_dxt1(gallivm, format, dxt_block, col);

   /*
    * three possible strategies for vectorizing alpha:
    * 1) compute all 8 values then use scalar extraction
    *    (i.e. have all 8 alpha values packed in one 64bit scalar
    *    and do something like ax = vals >> (codex * 8) followed
    *    by inserting these values back into color)
    * 2) same as 8 but just use pshufb as a mini-LUT for selection.
    *    (without pshufb would need boatloads of cmp/selects trying to
    *    keep things vectorized for essentially scalar selection).
    * 3) do something similar to the uncached case
    *    needs more calculations (need to calc 16 values instead of 8 though
    *    that's only an issue for the lerp which we need to do twice otherwise
    *    everything still fits into 128bit) but keeps things vectorized mostly.
    * Trying 3) here though not sure it's really faster...
    * With pshufb, we try 2) (cheaper and more accurate)
    */

   /*
    * Ideally, we'd use 2 variable 16bit shifts here (byte shifts wouldn't
    * help since code crosses 8bit boundaries). But variable shifts are
    * AVX2 only, and even then only dword/quadword (intel _really_ hates
    * shifts!). Instead, emulate by 16bit muls.
    * Also, the required byte shuffles are essentially non-emulatable, so
    * require ssse3 (albeit other archs might do them fine).
    * This is not directly tied to ssse3 - just need sane byte shuffles.
    * But ordering is going to be different below so use same condition.
    */


   /* vectorize alpha */
   alpha = LLVMBuildBitCast(builder, dxt_block, i64t, "");
   alpha0 = LLVMBuildAnd(builder, alpha,
                         lp_build_const_int_vec(gallivm, type64, 0xff), "");
   alpha0 = LLVMBuildBitCast(builder, alpha0, bld16.vec_type, "");
   alpha = LLVMBuildBitCast(builder, alpha, bld16.vec_type, "");
   alpha1 = LLVMBuildLShr(builder, alpha,
                          lp_build_const_int_vec(gallivm, type16, 8), "");
   alpha = LLVMBuildBitCast(builder, alpha,  i64t, "");
   shuffle1 = lp_build_const_shuffle1(gallivm, 0, 8);
   alpha0 = LLVMBuildShuffleVector(builder, alpha0, alpha0, shuffle1, "");
   alpha1 = LLVMBuildShuffleVector(builder, alpha1, alpha1, shuffle1, "");

   type16.sign = true;
   sel_mask = lp_build_compare(gallivm, type16, PIPE_FUNC_GREATER,
                               alpha0, alpha1);
   type16.sign = false;
   sel_mask = LLVMBuildBitCast(builder, sel_mask, bld8.vec_type, "");

   if (!util_get_cpu_caps()->has_ssse3) {
      LLVMValueRef acodeg, mask1, acode0, acode1;

      /* extraction of the 3 bit values into something more useful is HARD */
      /* first steps are actually scalar */
      acode = LLVMBuildLShr(builder, alpha,
                            lp_build_const_int_vec(gallivm, type64, 16), "");
      tmp0 = LLVMBuildAnd(builder, acode,
                          lp_build_const_int_vec(gallivm, type64, 0xffffff), "");
      tmp1 =  LLVMBuildLShr(builder, acode,
                            lp_build_const_int_vec(gallivm, type64, 24), "");
      tmp0 = LLVMBuildBitCast(builder, tmp0, i32t, "");
      tmp1 = LLVMBuildBitCast(builder, tmp1, i32t, "");
      acode = lp_build_interleave2(gallivm, type32, tmp0, tmp1, 0);
      /* now have 2x24bit in 4x32bit, order 01234567, 89..., undef, undef */
      tmp0 = LLVMBuildAnd(builder, acode,
                          lp_build_const_int_vec(gallivm, type32, 0xfff), "");
      tmp1 =  LLVMBuildLShr(builder, acode,
                            lp_build_const_int_vec(gallivm, type32, 12), "");
      acode = lp_build_interleave2(gallivm, type32, tmp0, tmp1, 0);
      /* now have 4x12bit in 4x32bit, order 0123, 4567, ,,, */
      tmp0 = LLVMBuildAnd(builder, acode,
                          lp_build_const_int_vec(gallivm, type32, 0x3f), "");
      tmp1 =  LLVMBuildLShr(builder, acode,
                            lp_build_const_int_vec(gallivm, type32, 6), "");
      /* use signed pack doesn't matter and otherwise need sse41 */
      type32.sign = type16.sign = true;
      acode = lp_build_pack2(gallivm, type32, type16, tmp0, tmp1);
      type32.sign = type16.sign = false;
      /* now have 8x6bit in 8x16bit, 01, 45, 89, ..., 23, 67, ... */
      acode0 = LLVMBuildAnd(builder, acode,
                            lp_build_const_int_vec(gallivm, type16, 0x7), "");
      acode1 =  LLVMBuildLShr(builder, acode,
                              lp_build_const_int_vec(gallivm, type16, 3), "");
      acode = lp_build_pack2(gallivm, type16, type8, acode0, acode1);
      /* acode0 contains elems 0,4,8,12,2,6,10,14, acode1 1,5,9,... */

      acodeg = LLVMBuildAnd(builder, acode,
                            LLVMBuildNot(builder, sel_mask, ""), "");
      mask1 = lp_build_compare(gallivm, type8, PIPE_FUNC_EQUAL,
                               acode, bld8.one);

      sel_mask = LLVMBuildBitCast(builder, sel_mask, bld16.vec_type, "");
      ainterp0 = lp_build_lerpdxta_block(gallivm, alpha0, alpha1, acode0, sel_mask);
      ainterp1 = lp_build_lerpdxta_block(gallivm, alpha0, alpha1, acode1, sel_mask);
      sel_mask = LLVMBuildBitCast(builder, sel_mask, bld8.vec_type, "");
      ainterp = lp_build_pack2(gallivm, type16, type8, ainterp0, ainterp1);
      alpha0 = lp_build_pack2(gallivm, type16, type8, alpha0, alpha0);
      alpha1 = lp_build_pack2(gallivm, type16, type8, alpha1, alpha1);
      ainterp = LLVMBuildAdd(builder, ainterp, alpha0, "");
      /* Fix up val01 */
      sel_mask2 = lp_build_compare(gallivm, type8, PIPE_FUNC_EQUAL,
                                   acode, bld8.zero);
      ainterp = lp_build_select(&bld8, sel_mask2, alpha0, ainterp);
      ainterp = lp_build_select(&bld8, mask1, alpha1, ainterp);

      /* fix up val67 if a0 <= a1 */
      sel_mask2 = lp_build_compare(gallivm, type8, PIPE_FUNC_EQUAL,
                                   acodeg, lp_build_const_int_vec(gallivm, type8, 6));
      ares = LLVMBuildAnd(builder, ainterp, LLVMBuildNot(builder, sel_mask2, ""), "");
      sel_mask2 = lp_build_compare(gallivm, type8, PIPE_FUNC_EQUAL,
                                   acodeg, lp_build_const_int_vec(gallivm, type8, 7));
      ares = LLVMBuildOr(builder, ares, sel_mask2, "");

      /* unpack in right order (0,4,8,12,1,5,..) */
      /* this gives us zero, a0, zero, a4, zero, a8, ... for tmp0 */
      tmp0 = lp_build_interleave2(gallivm, type8, bld8.zero, ares, 0);
      tmp1 = lp_build_interleave2(gallivm, type8, bld8.zero, ares, 1);
      tmp0 = LLVMBuildBitCast(builder, tmp0, bld16.vec_type, "");
      tmp1 = LLVMBuildBitCast(builder, tmp1, bld16.vec_type, "");

      a[0] = lp_build_interleave2(gallivm, type16, bld16.zero, tmp0, 0);
      a[1] = lp_build_interleave2(gallivm, type16, bld16.zero, tmp1, 0);
      a[2] = lp_build_interleave2(gallivm, type16, bld16.zero, tmp0, 1);
      a[3] = lp_build_interleave2(gallivm, type16, bld16.zero, tmp1, 1);
   }
   else {
      LLVMValueRef elems[16], intrargs[2], shufa, mulclo, mulchi, mask8hi;
      LLVMTypeRef type16s = LLVMInt16TypeInContext(gallivm->context);
      LLVMTypeRef type8s = LLVMInt8TypeInContext(gallivm->context);
      unsigned i, j;
      /*
       * Ideally, we'd use 2 variable 16bit shifts here (byte shifts wouldn't
       * help since code crosses 8bit boundaries). But variable shifts are
       * AVX2 only, and even then only dword/quadword (intel _really_ hates
       * shifts!). Instead, emulate by 16bit muls.
       * Also, the required byte shuffles are essentially non-emulatable, so
       * require ssse3 (albeit other archs might do them fine, but the
       * complete path is ssse3 only for now).
       */
      for (i = 0, j = 0; i < 16; i += 8, j += 3) {
         elems[i+0] = elems[i+1] = elems[i+2] = lp_build_const_int32(gallivm, j+2);
         elems[i+3] = elems[i+4] = lp_build_const_int32(gallivm, j+3);
         elems[i+5] = elems[i+6] = elems[i+7] = lp_build_const_int32(gallivm, j+4);
      }
      shufa = LLVMConstVector(elems, 16);
      alpha = LLVMBuildBitCast(builder, alpha, bld8.vec_type, "");
      acode = LLVMBuildShuffleVector(builder, alpha, bld8.undef, shufa, "");
      acode = LLVMBuildBitCast(builder, acode, bld16.vec_type, "");
      /*
       * Put 0/2/4/6 into high 3 bits of 16 bits (save AND mask)
       * Do the same for 1/3/5/7 (albeit still need mask there - ideally
       * we'd place them into bits 4-7 so could save shift but impossible.)
       */
      for (i = 0; i < 8; i += 4) {
         elems[i+0] = LLVMConstInt(type16s, 1 << (13-0), 0);
         elems[i+1] = LLVMConstInt(type16s, 1 << (13-6), 0);
         elems[i+2] = LLVMConstInt(type16s, 1 << (13-4), 0);
         elems[i+3] = LLVMConstInt(type16s, 1 << (13-2), 0);
      }
      mulclo = LLVMConstVector(elems, 8);
      for (i = 0; i < 8; i += 4) {
         elems[i+0] = LLVMConstInt(type16s, 1 << (13-3), 0);
         elems[i+1] = LLVMConstInt(type16s, 1 << (13-9), 0);
         elems[i+2] = LLVMConstInt(type16s, 1 << (13-7), 0);
         elems[i+3] = LLVMConstInt(type16s, 1 << (13-5), 0);
      }
      mulchi = LLVMConstVector(elems, 8);

      tmp0 = LLVMBuildMul(builder, acode, mulclo, "");
      tmp1 = LLVMBuildMul(builder, acode, mulchi, "");
      tmp0 = LLVMBuildLShr(builder, tmp0,
                           lp_build_const_int_vec(gallivm, type16, 13), "");
      tmp1 = LLVMBuildLShr(builder, tmp1,
                           lp_build_const_int_vec(gallivm, type16, 5), "");
      tmp1 = LLVMBuildAnd(builder, tmp1,
                          lp_build_const_int_vec(gallivm, type16, 0x700), "");
      acode = LLVMBuildOr(builder, tmp0, tmp1, "");
      acode = LLVMBuildBitCast(builder, acode, bld8.vec_type, "");

      /*
       * Note that ordering is different here to non-ssse3 path:
       * 0/1/2/3/4/5...
       */

      LLVMValueRef weight0, weight1, weight, delta;
      LLVMValueRef constff_elem7, const0_elem6;
      /* weights, correctly rounded (round(256*x/7)) */
      elems[0] = LLVMConstInt(type16s, 256, 0);
      elems[1] = LLVMConstInt(type16s, 0, 0);
      elems[2] = LLVMConstInt(type16s, 219, 0);
      elems[3] =  LLVMConstInt(type16s, 183, 0);
      elems[4] =  LLVMConstInt(type16s, 146, 0);
      elems[5] =  LLVMConstInt(type16s, 110, 0);
      elems[6] =  LLVMConstInt(type16s, 73, 0);
      elems[7] =  LLVMConstInt(type16s, 37, 0);
      weight0 = LLVMConstVector(elems, 8);

      elems[0] = LLVMConstInt(type16s, 256, 0);
      elems[1] = LLVMConstInt(type16s, 0, 0);
      elems[2] = LLVMConstInt(type16s, 205, 0);
      elems[3] =  LLVMConstInt(type16s, 154, 0);
      elems[4] =  LLVMConstInt(type16s, 102, 0);
      elems[5] =  LLVMConstInt(type16s, 51, 0);
      elems[6] =  LLVMConstInt(type16s, 0, 0);
      elems[7] =  LLVMConstInt(type16s, 0, 0);
      weight1 = LLVMConstVector(elems, 8);

      weight0 = LLVMBuildBitCast(builder, weight0, bld8.vec_type, "");
      weight1 = LLVMBuildBitCast(builder, weight1, bld8.vec_type, "");
      weight = lp_build_select(&bld8, sel_mask, weight0, weight1);
      weight = LLVMBuildBitCast(builder, weight, bld16.vec_type, "");

      for (i = 0; i < 16; i++) {
         elems[i] = LLVMConstNull(type8s);
      }
      elems[7] = LLVMConstInt(type8s, 255, 0);
      constff_elem7 = LLVMConstVector(elems, 16);

      for (i = 0; i < 16; i++) {
         elems[i] = LLVMConstInt(type8s, 255, 0);
      }
      elems[6] = LLVMConstInt(type8s, 0, 0);
      const0_elem6 = LLVMConstVector(elems, 16);

      /* standard simple lerp - but the version we need isn't available */
      delta = LLVMBuildSub(builder, alpha0, alpha1, "");
      ainterp = LLVMBuildMul(builder, delta, weight, "");
      ainterp = LLVMBuildLShr(builder, ainterp,
                              lp_build_const_int_vec(gallivm, type16, 8), "");
      ainterp = LLVMBuildBitCast(builder, ainterp, bld8.vec_type, "");
      alpha1 = LLVMBuildBitCast(builder, alpha1, bld8.vec_type, "");
      ainterp = LLVMBuildAdd(builder, ainterp, alpha1, "");
      ainterp = LLVMBuildBitCast(builder, ainterp, bld16.vec_type, "");
      ainterp = lp_build_pack2(gallivm, type16, type8, ainterp, bld16.undef);

      /* fixing 0/0xff case is slightly more complex */
      constff_elem7 = LLVMBuildAnd(builder, constff_elem7,
                                   LLVMBuildNot(builder, sel_mask, ""), "");
      const0_elem6 = LLVMBuildOr(builder, const0_elem6, sel_mask, "");
      ainterp = LLVMBuildOr(builder, ainterp, constff_elem7, "");
      ainterp = LLVMBuildAnd(builder, ainterp, const0_elem6, "");

      /* now pick all 16 elements at once! */
      intrargs[0] = ainterp;
      intrargs[1] = acode;
      ares = lp_build_intrinsic(builder, "llvm.x86.ssse3.pshuf.b.128",
                                bld8.vec_type, intrargs, 2, 0);

      ares = LLVMBuildBitCast(builder, ares, i32t, "");
      mask8hi = lp_build_const_int_vec(gallivm, type32, 0xff000000);
      a[0] = LLVMBuildShl(builder, ares,
                          lp_build_const_int_vec(gallivm, type32, 24), "");
      a[1] = LLVMBuildShl(builder, ares,
                          lp_build_const_int_vec(gallivm, type32, 16), "");
      a[1] = LLVMBuildAnd(builder, a[1], mask8hi, "");
      a[2] = LLVMBuildShl(builder, ares,
                          lp_build_const_int_vec(gallivm, type32, 8), "");
      a[2] = LLVMBuildAnd(builder, a[2], mask8hi, "");
      a[3] = LLVMBuildAnd(builder, ares, mask8hi, "");
   }

   for (i = 0; i < 4; i++) {
      a[i] = LLVMBuildBitCast(builder, a[i], i32t, "");
      col[i] = LLVMBuildOr(builder, col[i], a[i], "");
   }
}


static void
generate_update_cache_one_block(struct gallivm_state *gallivm,
                                LLVMValueRef function,
                                const struct util_format_description *format_desc)
{
   LLVMBasicBlockRef block;
   LLVMBuilderRef old_builder;
   LLVMValueRef ptr_addr;
   LLVMValueRef hash_index;
   LLVMValueRef cache;
   LLVMValueRef dxt_block, tag_value;
   LLVMValueRef col[LP_MAX_VECTOR_LENGTH];

   ptr_addr     = LLVMGetParam(function, 0);
   hash_index   = LLVMGetParam(function, 1);
   cache        = LLVMGetParam(function, 2);

   lp_build_name(ptr_addr,   "ptr_addr"  );
   lp_build_name(hash_index, "hash_index");
   lp_build_name(cache,      "cache_addr");

   /*
    * Function body
    */

   old_builder = gallivm->builder;
   block = LLVMAppendBasicBlockInContext(gallivm->context, function, "entry");
   gallivm->builder = LLVMCreateBuilderInContext(gallivm->context);
   LLVMPositionBuilderAtEnd(gallivm->builder, block);

   lp_build_gather_s3tc_simple_scalar(gallivm, format_desc, &dxt_block,
                                      ptr_addr);

   switch (format_desc->format) {
   case PIPE_FORMAT_DXT1_RGB:
   case PIPE_FORMAT_DXT1_RGBA:
   case PIPE_FORMAT_DXT1_SRGB:
   case PIPE_FORMAT_DXT1_SRGBA:
      s3tc_decode_block_dxt1(gallivm, format_desc->format, dxt_block, col);
      break;
   case PIPE_FORMAT_DXT3_RGBA:
   case PIPE_FORMAT_DXT3_SRGBA:
      s3tc_decode_block_dxt3(gallivm, format_desc->format, dxt_block, col);
      break;
   case PIPE_FORMAT_DXT5_RGBA:
   case PIPE_FORMAT_DXT5_SRGBA:
      s3tc_decode_block_dxt5(gallivm, format_desc->format, dxt_block, col);
      break;
   default:
      assert(0);
      s3tc_decode_block_dxt1(gallivm, format_desc->format, dxt_block, col);
      break;
   }

   tag_value = LLVMBuildPtrToInt(gallivm->builder, ptr_addr,
                                 LLVMInt64TypeInContext(gallivm->context), "");
   s3tc_store_cached_block(gallivm, col, tag_value, hash_index, cache);

   LLVMBuildRetVoid(gallivm->builder);

   LLVMDisposeBuilder(gallivm->builder);
   gallivm->builder = old_builder;

   gallivm_verify_function(gallivm, function);
}


static void
update_cached_block(struct gallivm_state *gallivm,
                    const struct util_format_description *format_desc,
                    LLVMValueRef ptr_addr,
                    LLVMValueRef hash_index,
                    LLVMValueRef cache)

{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMModuleRef module = gallivm->module;
   char name[256];
   LLVMTypeRef i8t = LLVMInt8TypeInContext(gallivm->context);
   LLVMTypeRef pi8t = LLVMPointerType(i8t, 0);
   LLVMValueRef function, inst;
   LLVMBasicBlockRef bb;
   LLVMValueRef args[3];

   snprintf(name, sizeof name, "%s_update_cache_one_block",
            format_desc->short_name);
   function = LLVMGetNamedFunction(module, name);

   LLVMTypeRef ret_type = LLVMVoidTypeInContext(gallivm->context);
   LLVMTypeRef arg_types[3];
   arg_types[0] = pi8t;
   arg_types[1] = LLVMInt32TypeInContext(gallivm->context);
   arg_types[2] = LLVMTypeOf(cache); // XXX: put right type here
   LLVMTypeRef function_type = LLVMFunctionType(ret_type, arg_types, ARRAY_SIZE(arg_types), 0);

   if (!function) {
      function = LLVMAddFunction(module, name, function_type);

      for (unsigned arg = 0; arg < ARRAY_SIZE(arg_types); ++arg)
         if (LLVMGetTypeKind(arg_types[arg]) == LLVMPointerTypeKind)
            lp_add_function_attr(function, arg + 1, LP_FUNC_ATTR_NOALIAS);

      LLVMSetFunctionCallConv(function, LLVMFastCallConv);
      LLVMSetVisibility(function, LLVMHiddenVisibility);
      generate_update_cache_one_block(gallivm, function, format_desc);
   }

   args[0] = ptr_addr;
   args[1] = hash_index;
   args[2] = cache;
 
   LLVMBuildCall2(builder, function_type, function, args, ARRAY_SIZE(args), "");
   bb = LLVMGetInsertBlock(builder);
   inst = LLVMGetLastInstruction(bb);
   LLVMSetInstructionCallConv(inst, LLVMFastCallConv);
}

/*
 * cached lookup
 */
static LLVMValueRef
compressed_fetch_cached(struct gallivm_state *gallivm,
                        const struct util_format_description *format_desc,
                        unsigned n,
                        LLVMValueRef base_ptr,
                        LLVMValueRef offset,
                        LLVMValueRef i,
                        LLVMValueRef j,
                        LLVMValueRef cache)

{
   LLVMBuilderRef builder = gallivm->builder;
   unsigned count, low_bit, log2size;
   LLVMValueRef color, offset_stored, addr, ptr_addrtrunc, tmp;
   LLVMValueRef ij_index, hash_index, hash_mask, block_index;
   LLVMTypeRef i8t = LLVMInt8TypeInContext(gallivm->context);
   LLVMTypeRef i32t = LLVMInt32TypeInContext(gallivm->context);
   LLVMTypeRef i64t = LLVMInt64TypeInContext(gallivm->context);
   struct lp_type type;
   struct lp_build_context bld32;
   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;

   lp_build_context_init(&bld32, gallivm, type);

   /*
    * compute hash - we use direct mapped cache, the hash function could
    *                be better but it needs to be simple
    * per-element:
    *    compare offset with offset stored at tag (hash)
    *    if not equal extract block, store block, update tag
    *    extract color from cache
    *    assemble colors
    */

   low_bit = util_logbase2(format_desc->block.bits / 8);
   log2size = util_logbase2(LP_BUILD_FORMAT_CACHE_SIZE);
   addr = LLVMBuildPtrToInt(builder, base_ptr, i64t, "");
   ptr_addrtrunc = LLVMBuildPtrToInt(builder, base_ptr, i32t, "");
   ptr_addrtrunc = lp_build_broadcast_scalar(&bld32, ptr_addrtrunc);
   /* For the hash function, first mask off the unused lowest bits. Then just
      do some xor with address bits - only use lower 32bits */
   ptr_addrtrunc = LLVMBuildAdd(builder, offset, ptr_addrtrunc, "");
   ptr_addrtrunc = LLVMBuildLShr(builder, ptr_addrtrunc,
                                 lp_build_const_int_vec(gallivm, type, low_bit), "");
   /* This only really makes sense for size 64,128,256 */
   hash_index = ptr_addrtrunc;
   ptr_addrtrunc = LLVMBuildLShr(builder, ptr_addrtrunc,
                                 lp_build_const_int_vec(gallivm, type, 2*log2size), "");
   hash_index = LLVMBuildXor(builder, ptr_addrtrunc, hash_index, "");
   tmp = LLVMBuildLShr(builder, hash_index,
                       lp_build_const_int_vec(gallivm, type, log2size), "");
   hash_index = LLVMBuildXor(builder, hash_index, tmp, "");

   hash_mask = lp_build_const_int_vec(gallivm, type, LP_BUILD_FORMAT_CACHE_SIZE - 1);
   hash_index = LLVMBuildAnd(builder, hash_index, hash_mask, "");
   ij_index = LLVMBuildShl(builder, i, lp_build_const_int_vec(gallivm, type, 2), "");
   ij_index = LLVMBuildAdd(builder, ij_index, j, "");
   block_index = LLVMBuildShl(builder, hash_index,
                              lp_build_const_int_vec(gallivm, type, 4), "");
   block_index = LLVMBuildAdd(builder, ij_index, block_index, "");

   if (n > 1) {
      color = bld32.undef;
      for (count = 0; count < n; count++) {
         LLVMValueRef index, cond, colorx;
         LLVMValueRef block_indexx, hash_indexx, addrx, offsetx, ptr_addrx;
         struct lp_build_if_state if_ctx;

         index = lp_build_const_int32(gallivm, count);
         offsetx = LLVMBuildExtractElement(builder, offset, index, "");
         addrx = LLVMBuildZExt(builder, offsetx, i64t, "");
         addrx = LLVMBuildAdd(builder, addrx, addr, "");
         block_indexx = LLVMBuildExtractElement(builder, block_index, index, "");
         hash_indexx = LLVMBuildLShr(builder, block_indexx,
                                     lp_build_const_int32(gallivm, 4), "");
         offset_stored = s3tc_lookup_tag_data(gallivm, cache, hash_indexx);
         cond = LLVMBuildICmp(builder, LLVMIntNE, offset_stored, addrx, "");

         lp_build_if(&if_ctx, gallivm, cond);
         {
            ptr_addrx = LLVMBuildIntToPtr(builder, addrx,
                                          LLVMPointerType(i8t, 0), "");
            update_cached_block(gallivm, format_desc, ptr_addrx, hash_indexx, cache);
#if LP_BUILD_FORMAT_CACHE_DEBUG
            s3tc_update_cache_access(gallivm, cache, 1,
                                     LP_BUILD_FORMAT_CACHE_MEMBER_ACCESS_MISS);
#endif
         }
         lp_build_endif(&if_ctx);

         colorx = s3tc_lookup_cached_pixel(gallivm, cache, block_indexx);

         color = LLVMBuildInsertElement(builder, color, colorx,
                                        lp_build_const_int32(gallivm, count), "");
      }
   }
   else {
      LLVMValueRef cond;
      struct lp_build_if_state if_ctx;

      tmp = LLVMBuildZExt(builder, offset, i64t, "");
      addr = LLVMBuildAdd(builder, tmp, addr, "");
      offset_stored = s3tc_lookup_tag_data(gallivm, cache, hash_index);
      cond = LLVMBuildICmp(builder, LLVMIntNE, offset_stored, addr, "");

      lp_build_if(&if_ctx, gallivm, cond);
      {
         tmp = LLVMBuildIntToPtr(builder, addr, LLVMPointerType(i8t, 0), "");
         update_cached_block(gallivm, format_desc, tmp, hash_index, cache);
#if LP_BUILD_FORMAT_CACHE_DEBUG
         s3tc_update_cache_access(gallivm, cache, 1,
                                  LP_BUILD_FORMAT_CACHE_MEMBER_ACCESS_MISS);
#endif
      }
      lp_build_endif(&if_ctx);

      color = s3tc_lookup_cached_pixel(gallivm, cache, block_index);
   }
#if LP_BUILD_FORMAT_CACHE_DEBUG
   s3tc_update_cache_access(gallivm, cache, n,
                            LP_BUILD_FORMAT_CACHE_MEMBER_ACCESS_TOTAL);
#endif
   return LLVMBuildBitCast(builder, color, LLVMVectorType(i8t, n * 4), "");
}


static LLVMValueRef
s3tc_dxt5_to_rgba_aos(struct gallivm_state *gallivm,
                      unsigned n,
                      enum pipe_format format,
                      LLVMValueRef colors,
                      LLVMValueRef codewords,
                      LLVMValueRef alpha_lo,
                      LLVMValueRef alpha_hi,
                      LLVMValueRef i,
                      LLVMValueRef j)
{
   return s3tc_dxt5_full_to_rgba_aos(gallivm, n, format, colors,
                                     codewords, alpha_lo, alpha_hi, i, j);
}


/**
 * @param n  number of pixels processed (usually n=4, but it should also work with n=1
 *           and multiples of 4)
 * @param base_ptr  base pointer (32bit or 64bit pointer depending on the architecture)
 * @param offset <n x i32> vector with the relative offsets of the S3TC blocks
 * @param i  is a <n x i32> vector with the x subpixel coordinate (0..3)
 * @param j  is a <n x i32> vector with the y subpixel coordinate (0..3)
 * @return  a <4*n x i8> vector with the pixel RGBA values in AoS
 */
LLVMValueRef
lp_build_fetch_s3tc_rgba_aos(struct gallivm_state *gallivm,
                             const struct util_format_description *format_desc,
                             unsigned n,
                             LLVMValueRef base_ptr,
                             LLVMValueRef offset,
                             LLVMValueRef i,
                             LLVMValueRef j,
                             LLVMValueRef cache)
{
   LLVMValueRef rgba;
   LLVMTypeRef i8t = LLVMInt8TypeInContext(gallivm->context);
   LLVMBuilderRef builder = gallivm->builder;

   assert(format_desc->layout == UTIL_FORMAT_LAYOUT_S3TC);
   assert(format_desc->block.width == 4);
   assert(format_desc->block.height == 4);

   assert((n == 1) || (n % 4 == 0));

/*   debug_printf("format = %d\n", format_desc->format);*/
   if (cache) {
      rgba = compressed_fetch_cached(gallivm, format_desc, n,
                                     base_ptr, offset, i, j, cache);
      return rgba;
   }

   /*
    * Could use n > 8 here with avx2, but doesn't seem faster.
    */
   if (n > 4) {
      unsigned count;
      LLVMTypeRef i8_vectype = LLVMVectorType(i8t, 4 * n);
      LLVMTypeRef i128_type = LLVMIntTypeInContext(gallivm->context, 128);
      LLVMTypeRef i128_vectype =  LLVMVectorType(i128_type, n / 4);
      LLVMTypeRef i324_vectype = LLVMVectorType(LLVMInt32TypeInContext(
                                                gallivm->context), 4);
      LLVMValueRef offset4, i4, j4, rgba4[LP_MAX_VECTOR_LENGTH/16];
      struct lp_type lp_324_vectype = lp_type_uint_vec(32, 128);

      assert(n / 4 <= ARRAY_SIZE(rgba4));

      rgba = LLVMGetUndef(i128_vectype);

      for (count = 0; count < n / 4; count++) {
         LLVMValueRef colors, codewords, alpha_lo = NULL, alpha_hi = NULL;

         i4 = lp_build_extract_range(gallivm, i, count * 4, 4);
         j4 = lp_build_extract_range(gallivm, j, count * 4, 4);
         offset4 = lp_build_extract_range(gallivm, offset, count * 4, 4);

         lp_build_gather_s3tc(gallivm, 4, format_desc, &colors, &codewords,
                              &alpha_lo, &alpha_hi, base_ptr, offset4);

         switch (format_desc->format) {
         case PIPE_FORMAT_DXT1_RGB:
         case PIPE_FORMAT_DXT1_RGBA:
         case PIPE_FORMAT_DXT1_SRGB:
         case PIPE_FORMAT_DXT1_SRGBA:
            rgba4[count] = s3tc_dxt1_to_rgba_aos(gallivm, 4, format_desc->format,
                                                 colors, codewords, i4, j4);
            break;
         case PIPE_FORMAT_DXT3_RGBA:
         case PIPE_FORMAT_DXT3_SRGBA:
            rgba4[count] = s3tc_dxt3_to_rgba_aos(gallivm, 4, format_desc->format, colors,
                                                 codewords, alpha_lo, alpha_hi, i4, j4);
            break;
         case PIPE_FORMAT_DXT5_RGBA:
         case PIPE_FORMAT_DXT5_SRGBA:
            rgba4[count] = s3tc_dxt5_to_rgba_aos(gallivm, 4, format_desc->format, colors,
                                                 codewords, alpha_lo, alpha_hi, i4, j4);
            break;
         default:
            assert(0);
            rgba4[count] = LLVMGetUndef(LLVMVectorType(i8t, 4));
            break;
         }
         /* shuffles typically give best results with dword elements...*/
         rgba4[count] = LLVMBuildBitCast(builder, rgba4[count], i324_vectype, "");
      }
      rgba = lp_build_concat(gallivm, rgba4, lp_324_vectype, n / 4);
      rgba = LLVMBuildBitCast(builder, rgba, i8_vectype, "");
   }
   else {
      LLVMValueRef colors, codewords, alpha_lo = NULL, alpha_hi = NULL;

      lp_build_gather_s3tc(gallivm, n, format_desc, &colors, &codewords,
                           &alpha_lo, &alpha_hi, base_ptr, offset);

      switch (format_desc->format) {
      case PIPE_FORMAT_DXT1_RGB:
      case PIPE_FORMAT_DXT1_RGBA:
      case PIPE_FORMAT_DXT1_SRGB:
      case PIPE_FORMAT_DXT1_SRGBA:
         rgba = s3tc_dxt1_to_rgba_aos(gallivm, n, format_desc->format,
                                      colors, codewords, i, j);
         break;
      case PIPE_FORMAT_DXT3_RGBA:
      case PIPE_FORMAT_DXT3_SRGBA:
         rgba = s3tc_dxt3_to_rgba_aos(gallivm, n, format_desc->format, colors,
                                      codewords, alpha_lo, alpha_hi, i, j);
         break;
      case PIPE_FORMAT_DXT5_RGBA:
      case PIPE_FORMAT_DXT5_SRGBA:
         rgba = s3tc_dxt5_to_rgba_aos(gallivm, n, format_desc->format, colors,
                                      codewords, alpha_lo, alpha_hi, i, j);
         break;
      default:
         assert(0);
         rgba = LLVMGetUndef(LLVMVectorType(i8t, 4*n));
         break;
      }
   }

   /* always return just decompressed values - srgb conversion is done later */

   return rgba;
}

/**
 * Gather elements from scatter positions in memory into vectors.
 * This is customised for fetching texels from s3tc textures.
 * For SSE, typical value is length=4.
 *
 * @param length length of the offsets
 * @param colors the stored colors of the blocks will be extracted into this.
 * @param codewords the codewords of the blocks will be extracted into this.
 * @param alpha_lo used for storing lower 32bit of alpha components for dxt3/5
 * @param alpha_hi used for storing higher 32bit of alpha components for dxt3/5
 * @param base_ptr base pointer, should be a i8 pointer type.
 * @param offsets vector with offsets
 */
static void
lp_build_gather_rgtc(struct gallivm_state *gallivm,
                     unsigned length,
                     const struct util_format_description *format_desc,
                     LLVMValueRef *red_lo, LLVMValueRef *red_hi,
                     LLVMValueRef *green_lo, LLVMValueRef *green_hi,
                     LLVMValueRef base_ptr,
                     LLVMValueRef offsets)
{
   LLVMBuilderRef builder = gallivm->builder;
   unsigned block_bits = format_desc->block.bits;
   unsigned i;
   LLVMValueRef elems[8];
   LLVMTypeRef type32 = LLVMInt32TypeInContext(gallivm->context);
   LLVMTypeRef type64 = LLVMInt64TypeInContext(gallivm->context);
   LLVMTypeRef type32dxt;
   struct lp_type lp_type32dxt;

   memset(&lp_type32dxt, 0, sizeof lp_type32dxt);
   lp_type32dxt.width = 32;
   lp_type32dxt.length = block_bits / 32;
   type32dxt = lp_build_vec_type(gallivm, lp_type32dxt);

   assert(block_bits == 64 || block_bits == 128);
   assert(length == 1 || length == 4 || length == 8);

   for (i = 0; i < length; ++i) {
      elems[i] = lp_build_gather_elem(gallivm, length,
                                      block_bits, block_bits, true,
                                      base_ptr, offsets, i, false);
      elems[i] = LLVMBuildBitCast(builder, elems[i], type32dxt, "");
   }
   if (length == 1) {
      LLVMValueRef elem = elems[0];

      *red_lo = LLVMBuildExtractElement(builder, elem,
                                        lp_build_const_int32(gallivm, 0), "");
      *red_hi = LLVMBuildExtractElement(builder, elem,
                                        lp_build_const_int32(gallivm, 1), "");

      if (block_bits == 128) {
         *green_lo = LLVMBuildExtractElement(builder, elem,
                                             lp_build_const_int32(gallivm, 2), "");
         *green_hi = LLVMBuildExtractElement(builder, elem,
                                             lp_build_const_int32(gallivm, 3), "");
      } else {
         *green_lo = NULL;
         *green_hi = NULL;
      }
   } else {
      LLVMValueRef tmp[4];
      struct lp_type lp_type32, lp_type64;
      memset(&lp_type32, 0, sizeof lp_type32);
      lp_type32.width = 32;
      lp_type32.length = length;
      lp_type32.sign = lp_type32dxt.sign;
      memset(&lp_type64, 0, sizeof lp_type64);
      lp_type64.width = 64;
      lp_type64.length = length/2;
      if (block_bits == 128) {
         if (length == 8) {
            for (i = 0; i < 4; ++i) {
               tmp[0] = elems[i];
               tmp[1] = elems[i+4];
               elems[i] = lp_build_concat(gallivm, tmp, lp_type32dxt, 2);
            }
         }
         lp_build_transpose_aos(gallivm, lp_type32, elems, tmp);
         *green_lo = tmp[2];
         *green_hi = tmp[3];
         *red_lo = tmp[0];
         *red_hi = tmp[1];
      } else {
         LLVMValueRef red01, red23;
         LLVMTypeRef type64_vec = LLVMVectorType(type64, length/2);
         LLVMTypeRef type32_vec = LLVMVectorType(type32, length);

         for (i = 0; i < length; ++i) {
            /* no-op shuffle */
            elems[i] = LLVMBuildShuffleVector(builder, elems[i],
                                              LLVMGetUndef(type32dxt),
                                              lp_build_const_extend_shuffle(gallivm, 2, 4), "");
         }
         if (length == 8) {
            struct lp_type lp_type32_4 = {0};
            lp_type32_4.width = 32;
            lp_type32_4.length = 4;
            for (i = 0; i < 4; ++i) {
               tmp[0] = elems[i];
               tmp[1] = elems[i+4];
               elems[i] = lp_build_concat(gallivm, tmp, lp_type32_4, 2);
            }
         }
         red01 = lp_build_interleave2_half(gallivm, lp_type32, elems[0], elems[1], 0);
         red23 = lp_build_interleave2_half(gallivm, lp_type32, elems[2], elems[3], 0);
         red01 = LLVMBuildBitCast(builder, red01, type64_vec, "");
         red23 = LLVMBuildBitCast(builder, red23, type64_vec, "");
         *red_lo = lp_build_interleave2_half(gallivm, lp_type64, red01, red23, 0);
         *red_hi = lp_build_interleave2_half(gallivm, lp_type64, red01, red23, 1);
         *red_lo = LLVMBuildBitCast(builder, *red_lo, type32_vec, "");
         *red_hi = LLVMBuildBitCast(builder, *red_hi, type32_vec, "");
         *green_lo = NULL;
         *green_hi = NULL;
      }
   }
}

static LLVMValueRef
rgtc1_to_rgba_aos(struct gallivm_state *gallivm,
                  unsigned n,
                  enum pipe_format format,
                  LLVMValueRef red_lo,
                  LLVMValueRef red_hi,
                  LLVMValueRef i,
                  LLVMValueRef j)
{
   LLVMBuilderRef builder = gallivm->builder;
   bool is_signed = (format == PIPE_FORMAT_RGTC1_SNORM);
   LLVMValueRef red = s3tc_dxt5_alpha_channel(gallivm, is_signed, n, red_hi, red_lo, i, j);
   LLVMValueRef rgba;
   struct lp_type type, type8;
   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;
   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = n*4;
   rgba = lp_build_const_int_vec(gallivm, type, is_signed ? (0x7f << 24) : (0xffu << 24));
   rgba = LLVMBuildOr(builder, rgba, red, "");
   return LLVMBuildBitCast(builder, rgba, lp_build_vec_type(gallivm, type8), "");
}

static LLVMValueRef
rgtc2_to_rgba_aos(struct gallivm_state *gallivm,
                  unsigned n,
                  enum pipe_format format,
                  LLVMValueRef red_lo,
                  LLVMValueRef red_hi,
                  LLVMValueRef green_lo,
                  LLVMValueRef green_hi,
                  LLVMValueRef i,
                  LLVMValueRef j)
{
   LLVMBuilderRef builder = gallivm->builder;
   bool is_signed = (format == PIPE_FORMAT_RGTC2_SNORM);
   LLVMValueRef red = s3tc_dxt5_alpha_channel(gallivm, is_signed, n, red_hi, red_lo, i, j);
   LLVMValueRef green = s3tc_dxt5_alpha_channel(gallivm, is_signed, n, green_hi, green_lo, i, j);
   LLVMValueRef rgba;
   struct lp_type type, type8;
   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;
   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = n*4;
   rgba = lp_build_const_int_vec(gallivm, type, is_signed ? (0x7f << 24) : (0xffu << 24));
   rgba = LLVMBuildOr(builder, rgba, red, "");
   green = LLVMBuildShl(builder, green, lp_build_const_int_vec(gallivm, type, 8), "");
   rgba = LLVMBuildOr(builder, rgba, green, "");
   return LLVMBuildBitCast(builder, rgba, lp_build_vec_type(gallivm, type8), "");
}

static LLVMValueRef
latc1_to_rgba_aos(struct gallivm_state *gallivm,
                  unsigned n,
                  enum pipe_format format,
                  LLVMValueRef red_lo,
                  LLVMValueRef red_hi,
                  LLVMValueRef i,
                  LLVMValueRef j)
{
   LLVMBuilderRef builder = gallivm->builder;
   bool is_signed = (format == PIPE_FORMAT_LATC1_SNORM);
   LLVMValueRef red = s3tc_dxt5_alpha_channel(gallivm, is_signed, n, red_hi, red_lo, i, j);
   LLVMValueRef rgba, temp;
   struct lp_type type, type8;
   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;
   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = n*4;
   rgba = lp_build_const_int_vec(gallivm, type, is_signed ? (0x7f << 24) : (0xffu << 24));
   rgba = LLVMBuildOr(builder, rgba, red, "");
   temp = LLVMBuildShl(builder, red, lp_build_const_int_vec(gallivm, type, 8), "");
   rgba = LLVMBuildOr(builder, rgba, temp, "");
   temp = LLVMBuildShl(builder, red, lp_build_const_int_vec(gallivm, type, 16), "");
   rgba = LLVMBuildOr(builder, rgba, temp, "");
   return LLVMBuildBitCast(builder, rgba, lp_build_vec_type(gallivm, type8), "");
}

static LLVMValueRef
latc2_to_rgba_aos(struct gallivm_state *gallivm,
                  unsigned n,
                  enum pipe_format format,
                  LLVMValueRef red_lo,
                  LLVMValueRef red_hi,
                  LLVMValueRef green_lo,
                  LLVMValueRef green_hi,
                  LLVMValueRef i,
                  LLVMValueRef j)
{
   LLVMBuilderRef builder = gallivm->builder;
   bool is_signed = (format == PIPE_FORMAT_LATC2_SNORM);
   LLVMValueRef red = s3tc_dxt5_alpha_channel(gallivm, is_signed, n, red_hi, red_lo, i, j);
   LLVMValueRef green = s3tc_dxt5_alpha_channel(gallivm, is_signed, n, green_hi, green_lo, i, j);
   LLVMValueRef rgba, temp;
   struct lp_type type, type8;
   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;
   memset(&type8, 0, sizeof type8);
   type8.width = 8;
   type8.length = n*4;

   temp = LLVMBuildShl(builder, red, lp_build_const_int_vec(gallivm, type, 8), "");
   rgba = LLVMBuildOr(builder, red, temp, "");
   temp = LLVMBuildShl(builder, red, lp_build_const_int_vec(gallivm, type, 16), "");
   rgba = LLVMBuildOr(builder, rgba, temp, "");
   temp = LLVMBuildShl(builder, green, lp_build_const_int_vec(gallivm, type, 24), "");
   rgba = LLVMBuildOr(builder, rgba, temp, "");
   return LLVMBuildBitCast(builder, rgba, lp_build_vec_type(gallivm, type8), "");
}

/**
 * @param n  number of pixels processed (usually n=4, but it should also work with n=1
 *           and multiples of 4)
 * @param base_ptr  base pointer (32bit or 64bit pointer depending on the architecture)
 * @param offset <n x i32> vector with the relative offsets of the S3TC blocks
 * @param i  is a <n x i32> vector with the x subpixel coordinate (0..3)
 * @param j  is a <n x i32> vector with the y subpixel coordinate (0..3)
 * @return  a <4*n x i8> vector with the pixel RGBA values in AoS
 */
LLVMValueRef
lp_build_fetch_rgtc_rgba_aos(struct gallivm_state *gallivm,
                             const struct util_format_description *format_desc,
                             unsigned n,
                             LLVMValueRef base_ptr,
                             LLVMValueRef offset,
                             LLVMValueRef i,
                             LLVMValueRef j,
                             LLVMValueRef cache)
{
   LLVMValueRef rgba;
   LLVMTypeRef i8t = LLVMInt8TypeInContext(gallivm->context);
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef red_lo, red_hi, green_lo, green_hi;
   assert(format_desc->layout == UTIL_FORMAT_LAYOUT_RGTC);
   assert(format_desc->block.width == 4);
   assert(format_desc->block.height == 4);

   assert((n == 1) || (n % 4 == 0));

   if (n > 4) {
      unsigned count;
      LLVMTypeRef i128_type = LLVMIntTypeInContext(gallivm->context, 128);
      LLVMTypeRef i128_vectype =  LLVMVectorType(i128_type, n / 4);
      LLVMTypeRef i8_vectype = LLVMVectorType(i8t, 4 * n);
      LLVMTypeRef i324_vectype = LLVMVectorType(LLVMInt32TypeInContext(
                                                   gallivm->context), 4);
      LLVMValueRef offset4, i4, j4, rgba4[LP_MAX_VECTOR_LENGTH/16];
      struct lp_type lp_324_vectype = lp_type_uint_vec(32, 128);

      rgba = LLVMGetUndef(i128_vectype);

      for (count = 0; count < n / 4; count++) {

         i4 = lp_build_extract_range(gallivm, i, count * 4, 4);
         j4 = lp_build_extract_range(gallivm, j, count * 4, 4);
         offset4 = lp_build_extract_range(gallivm, offset, count * 4, 4);

         lp_build_gather_rgtc(gallivm, 4, format_desc, &red_lo, &red_hi,
                              &green_lo, &green_hi, base_ptr, offset4);

         switch (format_desc->format) {
         case PIPE_FORMAT_RGTC1_UNORM:
         case PIPE_FORMAT_RGTC1_SNORM:
            rgba4[count] = rgtc1_to_rgba_aos(gallivm, 4, format_desc->format,
                                             red_lo, red_hi, i4, j4);
            break;
         case PIPE_FORMAT_RGTC2_UNORM:
         case PIPE_FORMAT_RGTC2_SNORM:
            rgba4[count] = rgtc2_to_rgba_aos(gallivm, 4, format_desc->format,
                                             red_lo, red_hi, green_lo, green_hi, i4, j4);
            break;
         case PIPE_FORMAT_LATC1_UNORM:
         case PIPE_FORMAT_LATC1_SNORM:
            rgba4[count] = latc1_to_rgba_aos(gallivm, 4, format_desc->format,
                                             red_lo, red_hi, i4, j4);
            break;
         case PIPE_FORMAT_LATC2_UNORM:
         case PIPE_FORMAT_LATC2_SNORM:
            rgba4[count] = latc2_to_rgba_aos(gallivm, 4, format_desc->format,
                                             red_lo, red_hi, green_lo, green_hi, i4, j4);
            break;
         default:
            assert(0);
            rgba4[count] = LLVMGetUndef(LLVMVectorType(i8t, 4));
            break;
         }
         /* shuffles typically give best results with dword elements...*/
         rgba4[count] = LLVMBuildBitCast(builder, rgba4[count], i324_vectype, "");
      }
      rgba = lp_build_concat(gallivm, rgba4, lp_324_vectype, n / 4);
      rgba = LLVMBuildBitCast(builder, rgba, i8_vectype, "");
   } else {
      LLVMValueRef red_lo, red_hi, green_lo, green_hi;

      lp_build_gather_rgtc(gallivm, n, format_desc, &red_lo, &red_hi,
                           &green_lo, &green_hi, base_ptr, offset);

      switch (format_desc->format) {
      case PIPE_FORMAT_RGTC1_UNORM:
      case PIPE_FORMAT_RGTC1_SNORM:
         rgba = rgtc1_to_rgba_aos(gallivm, n, format_desc->format,
                                  red_lo, red_hi, i, j);
         break;
      case PIPE_FORMAT_RGTC2_UNORM:
      case PIPE_FORMAT_RGTC2_SNORM:
         rgba = rgtc2_to_rgba_aos(gallivm, n, format_desc->format,
                                  red_lo, red_hi, green_lo, green_hi, i, j);
         break;
      case PIPE_FORMAT_LATC1_UNORM:
      case PIPE_FORMAT_LATC1_SNORM:
         rgba = latc1_to_rgba_aos(gallivm, n, format_desc->format,
                                  red_lo, red_hi, i, j);
         break;
      case PIPE_FORMAT_LATC2_UNORM:
      case PIPE_FORMAT_LATC2_SNORM:
         rgba = latc2_to_rgba_aos(gallivm, n, format_desc->format,
                                  red_lo, red_hi, green_lo, green_hi, i, j);
         break;
      default:
         assert(0);
         rgba = LLVMGetUndef(LLVMVectorType(i8t, 4*n));
         break;
      }
   }
   return rgba;
}
