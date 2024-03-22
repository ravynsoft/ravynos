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
 * Helper functions for swizzling/shuffling.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#include <inttypes.h>  /* for PRIx64 macro */
#include "util/compiler.h"
#include "util/u_debug.h"

#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_init.h"
#include "lp_bld_logic.h"
#include "lp_bld_swizzle.h"
#include "lp_bld_pack.h"


LLVMValueRef
lp_build_broadcast(struct gallivm_state *gallivm,
                   LLVMTypeRef vec_type,
                   LLVMValueRef scalar)
{
   LLVMValueRef res;

   if (LLVMGetTypeKind(vec_type) != LLVMVectorTypeKind) {
      /* scalar */
      assert(vec_type == LLVMTypeOf(scalar));
      res = scalar;
   } else {
      LLVMBuilderRef builder = gallivm->builder;
      const unsigned length = LLVMGetVectorSize(vec_type);
      LLVMValueRef undef = LLVMGetUndef(vec_type);
      /* The shuffle vector is always made of int32 elements */
      LLVMTypeRef i32_type = LLVMInt32TypeInContext(gallivm->context);
      LLVMTypeRef i32_vec_type = LLVMVectorType(i32_type, length);

      assert(LLVM_VERSION_MAJOR >= 15 || LLVMGetElementType(vec_type) == LLVMTypeOf(scalar));

      res = LLVMBuildInsertElement(builder, undef, scalar, LLVMConstNull(i32_type), "");
      res = LLVMBuildShuffleVector(builder, res, undef, LLVMConstNull(i32_vec_type), "");
   }

   return res;
}


/**
 * Broadcast
 */
LLVMValueRef
lp_build_broadcast_scalar(struct lp_build_context *bld,
                          LLVMValueRef scalar)
{
   assert(lp_check_elem_type(bld->type, LLVMTypeOf(scalar)));

   return lp_build_broadcast(bld->gallivm, bld->vec_type, scalar);
}


/**
 * Combined extract and broadcast (mere shuffle in most cases)
 */
LLVMValueRef
lp_build_extract_broadcast(struct gallivm_state *gallivm,
                           struct lp_type src_type,
                           struct lp_type dst_type,
                           LLVMValueRef vector,
                           LLVMValueRef index)
{
   LLVMTypeRef i32t = LLVMInt32TypeInContext(gallivm->context);
   LLVMValueRef res;

   assert(src_type.floating == dst_type.floating);
   assert(src_type.width    == dst_type.width);

   assert(lp_check_value(src_type, vector));
   assert(LLVMTypeOf(index) == i32t);

   if (src_type.length == 1) {
      if (dst_type.length == 1) {
         /*
          * Trivial scalar -> scalar.
          */
         res = vector;
      } else {
         /*
          * Broadcast scalar -> vector.
          */
         res = lp_build_broadcast(gallivm,
                                  lp_build_vec_type(gallivm, dst_type),
                                  vector);
      }
   } else {
      if (dst_type.length > 1) {
         /*
          * shuffle - result can be of different length.
          */
         LLVMValueRef shuffle;
         shuffle = lp_build_broadcast(gallivm,
                                      LLVMVectorType(i32t, dst_type.length),
                                      index);
         res = LLVMBuildShuffleVector(gallivm->builder, vector,
                                      LLVMGetUndef(lp_build_vec_type(gallivm, src_type)),
                                      shuffle, "");
      } else {
         /*
          * Trivial extract scalar from vector.
          */
          res = LLVMBuildExtractElement(gallivm->builder, vector, index, "");
      }
   }

   return res;
}


/**
 * Swizzle one channel into other channels.
 */
LLVMValueRef
lp_build_swizzle_scalar_aos(struct lp_build_context *bld,
                            LLVMValueRef a,
                            unsigned channel,
                            unsigned num_channels)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   const unsigned n = type.length;

   if (a == bld->undef || a == bld->zero || a == bld->one || num_channels == 1)
      return a;

   assert(num_channels == 2 || num_channels == 4);

   /* XXX: SSE3 has PSHUFB which should be better than bitmasks, but forcing
    * using shuffles here actually causes worst results. More investigation is
    * needed. */
   if (LLVMIsConstant(a) || type.width >= 16) {
      /*
       * Shuffle.
       */
      LLVMTypeRef elem_type = LLVMInt32TypeInContext(bld->gallivm->context);
      LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH];

      for (unsigned j = 0; j < n; j += num_channels)
         for (unsigned i = 0; i < num_channels; ++i)
            shuffles[j + i] = LLVMConstInt(elem_type, j + channel, 0);

      return LLVMBuildShuffleVector(builder, a, bld->undef, LLVMConstVector(shuffles, n), "");
   } else if (num_channels == 2) {
      /*
       * Bit mask and shifts
       *
       *   XY XY .... XY  <= input
       *   0Y 0Y .... 0Y
       *   YY YY .... YY
       *   YY YY .... YY  <= output
       */
      struct lp_type type2;
      LLVMValueRef tmp = NULL;
      int shift;

      a = LLVMBuildAnd(builder, a,
                       lp_build_const_mask_aos(bld->gallivm,
                                               type, 1 << channel, num_channels), "");

      type2 = type;
      type2.floating = false;
      type2.width *= 2;
      type2.length /= 2;

      a = LLVMBuildBitCast(builder, a, lp_build_vec_type(bld->gallivm, type2), "");

      /*
       * Vector element 0 is always channel X.
       *
       *                        76 54 32 10 (array numbering)
       * Little endian reg in:  YX YX YX YX
       * Little endian reg out: YY YY YY YY if shift right (shift == -1)
       *                        XX XX XX XX if shift left (shift == 1)
       *
       *                        01 23 45 67 (array numbering)
       * Big endian reg in:     XY XY XY XY
       * Big endian reg out:    YY YY YY YY if shift left (shift == 1)
       *                        XX XX XX XX if shift right (shift == -1)
       *
       */
#if UTIL_ARCH_LITTLE_ENDIAN
      shift = channel == 0 ? 1 : -1;
#else
      shift = channel == 0 ? -1 : 1;
#endif

      if (shift > 0) {
         tmp = LLVMBuildShl(builder, a, lp_build_const_int_vec(bld->gallivm, type2, shift * type.width), "");
      } else if (shift < 0) {
         tmp = LLVMBuildLShr(builder, a, lp_build_const_int_vec(bld->gallivm, type2, -shift * type.width), "");
      }

      assert(tmp);
      if (tmp) {
         a = LLVMBuildOr(builder, a, tmp, "");
      }

      return LLVMBuildBitCast(builder, a, lp_build_vec_type(bld->gallivm, type), "");
   } else {
      /*
       * Bit mask and recursive shifts
       *
       * Little-endian registers:
       *
       *   7654 3210
       *   WZYX WZYX .... WZYX  <= input
       *   00Y0 00Y0 .... 00Y0  <= mask
       *   00YY 00YY .... 00YY  <= shift right 1 (shift amount -1)
       *   YYYY YYYY .... YYYY  <= shift left 2 (shift amount 2)
       *
       * Big-endian registers:
       *
       *   0123 4567
       *   XYZW XYZW .... XYZW  <= input
       *   0Y00 0Y00 .... 0Y00  <= mask
       *   YY00 YY00 .... YY00  <= shift left 1 (shift amount 1)
       *   YYYY YYYY .... YYYY  <= shift right 2 (shift amount -2)
       *
       * shifts[] gives little-endian shift amounts; we need to negate for big-endian.
       */
      static const int shifts[4][2] = {
         { 1,  2},
         {-1,  2},
         { 1, -2},
         {-1, -2}
      };

      a = LLVMBuildAnd(builder, a,
                       lp_build_const_mask_aos(bld->gallivm,
                                               type, 1 << channel, 4), "");

      /*
       * Build a type where each element is an integer that cover the four
       * channels.
       */

      struct lp_type type4 = type;
      type4.floating = false;
      type4.width *= 4;
      type4.length /= 4;

      a = LLVMBuildBitCast(builder, a, lp_build_vec_type(bld->gallivm, type4), "");

      for (unsigned i = 0; i < 2; ++i) {
         LLVMValueRef tmp = NULL;
         int shift = shifts[channel][i];

         /* See endianness diagram above */
#if UTIL_ARCH_BIG_ENDIAN
         shift = -shift;
#endif

         if (shift > 0)
            tmp = LLVMBuildShl(builder, a, lp_build_const_int_vec(bld->gallivm, type4, shift*type.width), "");
         if (shift < 0)
            tmp = LLVMBuildLShr(builder, a, lp_build_const_int_vec(bld->gallivm, type4, -shift*type.width), "");

         assert(tmp);
         if (tmp)
            a = LLVMBuildOr(builder, a, tmp, "");
      }

      return LLVMBuildBitCast(builder, a, lp_build_vec_type(bld->gallivm, type), "");
   }
}


/**
 * Swizzle a vector consisting of an array of XYZW structs.
 *
 * This fills a vector of dst_len length with the swizzled channels from src.
 *
 * e.g. with swizzles = { 2, 1, 0 } and swizzle_count = 6 results in
 *      RGBA RGBA = BGR BGR BG
 *
 * @param swizzles        the swizzle array
 * @param num_swizzles    the number of elements in swizzles
 * @param dst_len         the length of the result
 */
LLVMValueRef
lp_build_swizzle_aos_n(struct gallivm_state* gallivm,
                       LLVMValueRef src,
                       const unsigned char* swizzles,
                       unsigned num_swizzles,
                       unsigned dst_len)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef shuffles[LP_MAX_VECTOR_WIDTH];

   assert(dst_len < LP_MAX_VECTOR_WIDTH);

   for (unsigned i = 0; i < dst_len; ++i) {
      int swizzle = swizzles[i % num_swizzles];

      if (swizzle == LP_BLD_SWIZZLE_DONTCARE) {
         shuffles[i] = LLVMGetUndef(LLVMInt32TypeInContext(gallivm->context));
      } else {
         shuffles[i] = lp_build_const_int32(gallivm, swizzle);
      }
   }

   return LLVMBuildShuffleVector(builder, src,
                                 LLVMGetUndef(LLVMTypeOf(src)),
                                 LLVMConstVector(shuffles, dst_len), "");
}


LLVMValueRef
lp_build_swizzle_aos(struct lp_build_context *bld,
                     LLVMValueRef a,
                     const unsigned char swizzles[4])
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   const unsigned n = type.length;

   if (swizzles[0] == PIPE_SWIZZLE_X &&
       swizzles[1] == PIPE_SWIZZLE_Y &&
       swizzles[2] == PIPE_SWIZZLE_Z &&
       swizzles[3] == PIPE_SWIZZLE_W) {
      return a;
   }

   if (swizzles[0] == swizzles[1] &&
       swizzles[1] == swizzles[2] &&
       swizzles[2] == swizzles[3]) {
      switch (swizzles[0]) {
      case PIPE_SWIZZLE_X:
      case PIPE_SWIZZLE_Y:
      case PIPE_SWIZZLE_Z:
      case PIPE_SWIZZLE_W:
         return lp_build_swizzle_scalar_aos(bld, a, swizzles[0], 4);
      case PIPE_SWIZZLE_0:
         return bld->zero;
      case PIPE_SWIZZLE_1:
         return bld->one;
      case LP_BLD_SWIZZLE_DONTCARE:
         return bld->undef;
      default:
         assert(0);
         return bld->undef;
      }
   }

   if (LLVMIsConstant(a) ||
       type.width >= 16) {
      /*
       * Shuffle.
       */
      LLVMValueRef undef = LLVMGetUndef(lp_build_elem_type(bld->gallivm, type));
      LLVMTypeRef i32t = LLVMInt32TypeInContext(bld->gallivm->context);
      LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH];
      LLVMValueRef aux[LP_MAX_VECTOR_LENGTH];

      memset(aux, 0, sizeof aux);

      for (unsigned j = 0; j < n; j += 4) {
         for (unsigned i = 0; i < 4; ++i) {
            unsigned shuffle;
            switch (swizzles[i]) {
            default:
               unreachable("Unsupported swizzle");
            case PIPE_SWIZZLE_X:
            case PIPE_SWIZZLE_Y:
            case PIPE_SWIZZLE_Z:
            case PIPE_SWIZZLE_W:
               shuffle = j + swizzles[i];
               shuffles[j + i] = LLVMConstInt(i32t, shuffle, 0);
               break;
            case PIPE_SWIZZLE_0:
               shuffle = type.length + 0;
               shuffles[j + i] = LLVMConstInt(i32t, shuffle, 0);
               if (!aux[0]) {
                  aux[0] = lp_build_const_elem(bld->gallivm, type, 0.0);
               }
               break;
            case PIPE_SWIZZLE_1:
               shuffle = type.length + 1;
               shuffles[j + i] = LLVMConstInt(i32t, shuffle, 0);
               if (!aux[1]) {
                  aux[1] = lp_build_const_elem(bld->gallivm, type, 1.0);
               }
               break;
            case LP_BLD_SWIZZLE_DONTCARE:
               shuffles[j + i] = LLVMGetUndef(i32t);
               break;
            }
         }
      }

      for (unsigned i = 0; i < n; ++i) {
         if (!aux[i]) {
            aux[i] = undef;
         }
      }

      return LLVMBuildShuffleVector(builder, a,
                                    LLVMConstVector(aux, n),
                                    LLVMConstVector(shuffles, n), "");
   } else {
      /*
       * Bit mask and shifts.
       *
       * For example, this will convert BGRA to RGBA by doing
       *
       * Little endian:
       *   rgba = (bgra & 0x00ff0000) >> 16
       *        | (bgra & 0xff00ff00)
       *        | (bgra & 0x000000ff) << 16
       *
       * Big endian:A
       *   rgba = (bgra & 0x0000ff00) << 16
       *        | (bgra & 0x00ff00ff)
       *        | (bgra & 0xff000000) >> 16
       *
       * This is necessary not only for faster cause, but because X86 backend
       * will refuse shuffles of <4 x i8> vectors
       */

      /*
       * Start with a mixture of 1 and 0.
       */
      unsigned cond = 0;
      for (unsigned chan = 0; chan < 4; ++chan) {
         if (swizzles[chan] == PIPE_SWIZZLE_1) {
            cond |= 1 << chan;
         }
      }
      LLVMValueRef res =
         lp_build_select_aos(bld, cond, bld->one, bld->zero, 4);

      /*
       * Build a type where each element is an integer that cover the four
       * channels.
       */
      struct lp_type type4 = type;
      type4.floating = false;
      type4.width *= 4;
      type4.length /= 4;

      a = LLVMBuildBitCast(builder, a, lp_build_vec_type(bld->gallivm, type4), "");
      res = LLVMBuildBitCast(builder, res, lp_build_vec_type(bld->gallivm, type4), "");

      /*
       * Mask and shift the channels, trying to group as many channels in the
       * same shift as possible.  The shift amount is positive for shifts left
       * and negative for shifts right.
       */
      for (int shift = -3; shift <= 3; ++shift) {
         uint64_t mask = 0;

         assert(type4.width <= sizeof(mask)*8);

         /*
          * Vector element numbers follow the XYZW order, so 0 is always X,
          * etc.  After widening 4 times we have:
          *
          *                                3210
          * Little-endian register layout: WZYX
          *
          *                                0123
          * Big-endian register layout:    XYZW
          *
          * For little-endian, higher-numbered channels are obtained by a
          * shift right (negative shift amount) and lower-numbered channels by
          * a shift left (positive shift amount).  The opposite is true for
          * big-endian.
          */
         for (unsigned chan = 0; chan < 4; ++chan) {
            if (swizzles[chan] < 4) {
               /* We need to move channel swizzles[chan] into channel chan */
#if UTIL_ARCH_LITTLE_ENDIAN
               if (swizzles[chan] - chan == -shift) {
                  mask |= ((1ULL << type.width) - 1) << (swizzles[chan] * type.width);
               }
#else
               if (swizzles[chan] - chan == shift) {
                  mask |= ((1ULL << type.width) - 1) << (type4.width - type.width) >> (swizzles[chan] * type.width);
               }
#endif
            }
         }

         if (mask) {
            LLVMValueRef masked;
            LLVMValueRef shifted;
            if (0)
               debug_printf("shift = %i, mask = %" PRIx64 "\n", shift, mask);

            masked = LLVMBuildAnd(builder, a,
                                  lp_build_const_int_vec(bld->gallivm, type4, mask), "");
            if (shift > 0) {
               shifted = LLVMBuildShl(builder, masked,
                                      lp_build_const_int_vec(bld->gallivm, type4, shift*type.width), "");
            } else if (shift < 0) {
               shifted = LLVMBuildLShr(builder, masked,
                                       lp_build_const_int_vec(bld->gallivm, type4, -shift*type.width), "");
            } else {
               shifted = masked;
            }

            res = LLVMBuildOr(builder, res, shifted, "");
         }
      }

      return LLVMBuildBitCast(builder, res,
                              lp_build_vec_type(bld->gallivm, type), "");
   }
}


/**
 * Extended swizzle of a single channel of a SoA vector.
 *
 * @param bld         building context
 * @param unswizzled  array with the 4 unswizzled values
 * @param swizzle     one of the PIPE_SWIZZLE_*
 *
 * @return  the swizzled value.
 */
LLVMValueRef
lp_build_swizzle_soa_channel(struct lp_build_context *bld,
                             const LLVMValueRef *unswizzled,
                             enum pipe_swizzle swizzle)
{
   switch (swizzle) {
   case PIPE_SWIZZLE_X:
   case PIPE_SWIZZLE_Y:
   case PIPE_SWIZZLE_Z:
   case PIPE_SWIZZLE_W:
      return unswizzled[swizzle];
   case PIPE_SWIZZLE_0:
      return bld->zero;
   case PIPE_SWIZZLE_1:
      return bld->one;
   default:
      assert(0);
      return bld->undef;
   }
}


/**
 * Extended swizzle of a SoA vector.
 *
 * @param bld         building context
 * @param unswizzled  array with the 4 unswizzled values
 * @param swizzles    array of PIPE_SWIZZLE_*
 * @param swizzled    output swizzled values
 */
void
lp_build_swizzle_soa(struct lp_build_context *bld,
                     const LLVMValueRef *unswizzled,
                     const unsigned char swizzles[4],
                     LLVMValueRef *swizzled)
{
   for (unsigned chan = 0; chan < 4; ++chan) {
      swizzled[chan] = lp_build_swizzle_soa_channel(bld, unswizzled,
                                                    swizzles[chan]);
   }
}


/**
 * Do an extended swizzle of a SoA vector inplace.
 *
 * @param bld         building context
 * @param values      intput/output array with the 4 values
 * @param swizzles    array of PIPE_SWIZZLE_*
 */
void
lp_build_swizzle_soa_inplace(struct lp_build_context *bld,
                             LLVMValueRef *values,
                             const unsigned char swizzles[4])
{
   LLVMValueRef unswizzled[4];

   for (unsigned chan = 0; chan < 4; ++chan) {
      unswizzled[chan] = values[chan];
   }

   lp_build_swizzle_soa(bld, unswizzled, swizzles, values);
}


/**
 * Transpose from AOS <-> SOA
 *
 * @param single_type_lp   type of pixels
 * @param src              the 4 * n pixel input
 * @param dst              the 4 * n pixel output
 */
void
lp_build_transpose_aos(struct gallivm_state *gallivm,
                       struct lp_type single_type_lp,
                       const LLVMValueRef src[4],
                       LLVMValueRef dst[4])
{
   struct lp_type double_type_lp = single_type_lp;
   double_type_lp.length >>= 1;
   double_type_lp.width  <<= 1;

   LLVMTypeRef double_type = lp_build_vec_type(gallivm, double_type_lp);
   LLVMTypeRef single_type = lp_build_vec_type(gallivm, single_type_lp);

   LLVMValueRef double_type_zero = LLVMConstNull(double_type);
   LLVMValueRef t0 = NULL, t1 = NULL, t2 = NULL, t3 = NULL;

   /* Interleave x, y, z, w -> xy and zw */
   if (src[0] || src[1]) {
      LLVMValueRef src0 = src[0];
      LLVMValueRef src1 = src[1];
      if (!src0)
         src0 = LLVMConstNull(single_type);
      if (!src1)
         src1 = LLVMConstNull(single_type);
      t0 = lp_build_interleave2_half(gallivm, single_type_lp, src0, src1, 0);
      t2 = lp_build_interleave2_half(gallivm, single_type_lp, src0, src1, 1);

      /* Cast to double width type for second interleave */
      t0 = LLVMBuildBitCast(gallivm->builder, t0, double_type, "t0");
      t2 = LLVMBuildBitCast(gallivm->builder, t2, double_type, "t2");
   }
   if (src[2] || src[3]) {
      LLVMValueRef src2 = src[2];
      LLVMValueRef src3 = src[3];
      if (!src2)
         src2 = LLVMConstNull(single_type);
      if (!src3)
         src3 = LLVMConstNull(single_type);
      t1 = lp_build_interleave2_half(gallivm, single_type_lp, src2, src3, 0);
      t3 = lp_build_interleave2_half(gallivm, single_type_lp, src2, src3, 1);

      /* Cast to double width type for second interleave */
      t1 = LLVMBuildBitCast(gallivm->builder, t1, double_type, "t1");
      t3 = LLVMBuildBitCast(gallivm->builder, t3, double_type, "t3");
   }

   if (!t0)
      t0 = double_type_zero;
   if (!t1)
      t1 = double_type_zero;
   if (!t2)
      t2 = double_type_zero;
   if (!t3)
      t3 = double_type_zero;

   /* Interleave xy, zw -> xyzw */
   dst[0] = lp_build_interleave2_half(gallivm, double_type_lp, t0, t1, 0);
   dst[1] = lp_build_interleave2_half(gallivm, double_type_lp, t0, t1, 1);
   dst[2] = lp_build_interleave2_half(gallivm, double_type_lp, t2, t3, 0);
   dst[3] = lp_build_interleave2_half(gallivm, double_type_lp, t2, t3, 1);

   /* Cast back to original single width type */
   dst[0] = LLVMBuildBitCast(gallivm->builder, dst[0], single_type, "dst0");
   dst[1] = LLVMBuildBitCast(gallivm->builder, dst[1], single_type, "dst1");
   dst[2] = LLVMBuildBitCast(gallivm->builder, dst[2], single_type, "dst2");
   dst[3] = LLVMBuildBitCast(gallivm->builder, dst[3], single_type, "dst3");
}


/**
 * Transpose from AOS <-> SOA for num_srcs
 */
void
lp_build_transpose_aos_n(struct gallivm_state *gallivm,
                         struct lp_type type,
                         const LLVMValueRef* src,
                         unsigned num_srcs,
                         LLVMValueRef* dst)
{
   switch (num_srcs) {
   case 1:
      dst[0] = src[0];
      break;
   case 2:
   {
      /* Note: we must use a temporary incase src == dst */
      LLVMValueRef lo, hi;

      lo = lp_build_interleave2_half(gallivm, type, src[0], src[1], 0);
      hi = lp_build_interleave2_half(gallivm, type, src[0], src[1], 1);

      dst[0] = lo;
      dst[1] = hi;
      break;
   }
   case 4:
      lp_build_transpose_aos(gallivm, type, src, dst);
      break;
   default:
      assert(0);
   }
}


/**
 * Pack n-th element of aos values,
 * pad out to destination size.
 * i.e. x1 y1 _ _ x2 y2 _ _ will become x1 x2 _ _
 */
LLVMValueRef
lp_build_pack_aos_scalars(struct gallivm_state *gallivm,
                          struct lp_type src_type,
                          struct lp_type dst_type,
                          const LLVMValueRef src,
                          unsigned channel)
{
   LLVMTypeRef i32t = LLVMInt32TypeInContext(gallivm->context);
   LLVMValueRef undef = LLVMGetUndef(i32t);
   LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH];
   unsigned num_src = src_type.length / 4;
   unsigned num_dst = dst_type.length;

   assert(num_src <= num_dst);

   for (unsigned i = 0; i < num_src; i++) {
      shuffles[i] = LLVMConstInt(i32t, i * 4 + channel, 0);
   }
   for (unsigned i = num_src; i < num_dst; i++) {
      shuffles[i] = undef;
   }

   if (num_dst == 1) {
      return LLVMBuildExtractElement(gallivm->builder, src, shuffles[0], "");
   }
   else {
      return LLVMBuildShuffleVector(gallivm->builder, src, src,
                                    LLVMConstVector(shuffles, num_dst), "");
   }
}


/**
 * Unpack and broadcast packed aos values consisting of only the
 * first value, i.e. x1 x2 _ _ will become x1 x1 x1 x1 x2 x2 x2 x2
 */
LLVMValueRef
lp_build_unpack_broadcast_aos_scalars(struct gallivm_state *gallivm,
                                      struct lp_type src_type,
                                      struct lp_type dst_type,
                                      const LLVMValueRef src)
{
   LLVMTypeRef i32t = LLVMInt32TypeInContext(gallivm->context);
   LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH];
   unsigned num_dst = dst_type.length;
   unsigned num_src = dst_type.length / 4;

   assert(num_dst / 4 <= src_type.length);

   for (unsigned i = 0; i < num_src; i++) {
      shuffles[i*4] = LLVMConstInt(i32t, i, 0);
      shuffles[i*4+1] = LLVMConstInt(i32t, i, 0);
      shuffles[i*4+2] = LLVMConstInt(i32t, i, 0);
      shuffles[i*4+3] = LLVMConstInt(i32t, i, 0);
   }

   if (num_src == 1) {
      return lp_build_extract_broadcast(gallivm, src_type, dst_type,
                                        src, shuffles[0]);
   } else {
      return LLVMBuildShuffleVector(gallivm->builder, src, src,
                                    LLVMConstVector(shuffles, num_dst), "");
   }
}

