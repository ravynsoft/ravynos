/**************************************************************************
 *
 * Copyright 2009-2010 VMware, Inc.
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
 * Depth/stencil testing to LLVM IR translation.
 *
 * To be done accurately/efficiently the depth/stencil test must be done with
 * the same type/format of the depth/stencil buffer, which implies massaging
 * the incoming depths to fit into place. Using a more straightforward
 * type/format for depth/stencil values internally and only convert when
 * flushing would avoid this, but it would most likely result in depth fighting
 * artifacts.
 *
 * Since we're using linear layout for everything, but we need to deal with
 * 2x2 quads, we need to load/store multiple values and swizzle them into
 * place (we could avoid this by doing depth/stencil testing in linear format,
 * which would be easy for late depth/stencil test as we could do that after
 * the fragment shader loop just as we do for color buffers, but more tricky
 * for early depth test as we'd need both masks and interpolated depth in
 * linear format).
 *
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 * @author Brian Paul <jfonseca@vmware.com>
 */

#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/u_cpu_detect.h"

#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_arit.h"
#include "gallivm/lp_bld_bitarit.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_conv.h"
#include "gallivm/lp_bld_logic.h"
#include "gallivm/lp_bld_flow.h"
#include "gallivm/lp_bld_intr.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_swizzle.h"
#include "gallivm/lp_bld_pack.h"

#include "lp_bld_depth.h"
#include "lp_state_fs.h"


/** Used to select fields from pipe_stencil_state */
enum stencil_op {
   S_FAIL_OP,
   Z_FAIL_OP,
   Z_PASS_OP
};



/**
 * Do the stencil test comparison (compare FB stencil values against ref value).
 * This will be used twice when generating two-sided stencil code.
 * \param stencil  the front/back stencil state
 * \param stencilRef  the stencil reference value, replicated as a vector
 * \param stencilVals  vector of stencil values from framebuffer
 * \return vector mask of pass/fail values (~0 or 0)
 */
static LLVMValueRef
lp_build_stencil_test_single(struct lp_build_context *bld,
                             const struct pipe_stencil_state *stencil,
                             LLVMValueRef stencilRef,
                             LLVMValueRef stencilVals)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const unsigned stencilMax = 255; /* XXX fix */
   struct lp_type type = bld->type;

   /*
    * SSE2 has intrinsics for signed comparisons, but not unsigned ones. Values
    * are between 0..255 so ensure we generate the fastest comparisons for
    * wider elements.
    */
   if (type.width <= 8) {
      assert(!type.sign);
   } else {
      assert(type.sign);
   }

   assert(stencil->enabled);

   if (stencil->valuemask != stencilMax) {
      /* compute stencilRef = stencilRef & valuemask */
      LLVMValueRef valuemask = lp_build_const_int_vec(bld->gallivm, type, stencil->valuemask);
      stencilRef = LLVMBuildAnd(builder, stencilRef, valuemask, "");
      /* compute stencilVals = stencilVals & valuemask */
      stencilVals = LLVMBuildAnd(builder, stencilVals, valuemask, "");
   }

   LLVMValueRef res = lp_build_cmp(bld, stencil->func,
                                   stencilRef, stencilVals);
   return res;
}


/**
 * Do the one or two-sided stencil test comparison.
 * \sa lp_build_stencil_test_single
 * \param front_facing  an integer vector mask, indicating front (~0) or back
 *                      (0) facing polygon. If NULL, assume front-facing.
 */
static LLVMValueRef
lp_build_stencil_test(struct lp_build_context *bld,
                      const struct pipe_stencil_state stencil[2],
                      LLVMValueRef stencilRefs[2],
                      LLVMValueRef stencilVals,
                      LLVMValueRef front_facing)
{
   LLVMValueRef res;

   assert(stencil[0].enabled);

   /* do front face test */
   res = lp_build_stencil_test_single(bld, &stencil[0],
                                      stencilRefs[0], stencilVals);

   if (stencil[1].enabled && front_facing != NULL) {
      /* do back face test */
      LLVMValueRef back_res;

      back_res = lp_build_stencil_test_single(bld, &stencil[1],
                                              stencilRefs[1], stencilVals);

      res = lp_build_select(bld, front_facing, res, back_res);
   }

   return res;
}


/**
 * Apply the stencil operator (add/sub/keep/etc) to the given vector
 * of stencil values.
 * \return  new stencil values vector
 */
static LLVMValueRef
lp_build_stencil_op_single(struct lp_build_context *bld,
                           const struct pipe_stencil_state *stencil,
                           enum stencil_op op,
                           LLVMValueRef stencilRef,
                           LLVMValueRef stencilVals)

{
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_type type = bld->type;
   LLVMValueRef max = lp_build_const_int_vec(bld->gallivm, type, 0xff);

   assert(type.sign);

   unsigned stencil_op;
   switch (op) {
   case S_FAIL_OP:
      stencil_op = stencil->fail_op;
      break;
   case Z_FAIL_OP:
      stencil_op = stencil->zfail_op;
      break;
   case Z_PASS_OP:
      stencil_op = stencil->zpass_op;
      break;
   default:
      assert(0 && "Invalid stencil_op mode");
      stencil_op = PIPE_STENCIL_OP_KEEP;
   }

   LLVMValueRef res;
   switch (stencil_op) {
   case PIPE_STENCIL_OP_KEEP:
      res = stencilVals;
      /* we can return early for this case */
      return res;
   case PIPE_STENCIL_OP_ZERO:
      res = bld->zero;
      break;
   case PIPE_STENCIL_OP_REPLACE:
      res = stencilRef;
      break;
   case PIPE_STENCIL_OP_INCR:
      res = lp_build_add(bld, stencilVals, bld->one);
      res = lp_build_min(bld, res, max);
      break;
   case PIPE_STENCIL_OP_DECR:
      res = lp_build_sub(bld, stencilVals, bld->one);
      res = lp_build_max(bld, res, bld->zero);
      break;
   case PIPE_STENCIL_OP_INCR_WRAP:
      res = lp_build_add(bld, stencilVals, bld->one);
      res = LLVMBuildAnd(builder, res, max, "");
      break;
   case PIPE_STENCIL_OP_DECR_WRAP:
      res = lp_build_sub(bld, stencilVals, bld->one);
      res = LLVMBuildAnd(builder, res, max, "");
      break;
   case PIPE_STENCIL_OP_INVERT:
      res = LLVMBuildNot(builder, stencilVals, "");
      res = LLVMBuildAnd(builder, res, max, "");
      break;
   default:
      assert(0 && "bad stencil op mode");
      res = bld->undef;
   }

   return res;
}


/**
 * Do the one or two-sided stencil test op/update.
 */
static LLVMValueRef
lp_build_stencil_op(struct lp_build_context *bld,
                    const struct pipe_stencil_state stencil[2],
                    enum stencil_op op,
                    LLVMValueRef stencilRefs[2],
                    LLVMValueRef stencilVals,
                    LLVMValueRef mask,
                    LLVMValueRef front_facing)

{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef res;

   assert(stencil[0].enabled);

   /* do front face op */
   res = lp_build_stencil_op_single(bld, &stencil[0], op,
                                     stencilRefs[0], stencilVals);

   if (stencil[1].enabled && front_facing != NULL) {
      /* do back face op */
      LLVMValueRef back_res;

      back_res = lp_build_stencil_op_single(bld, &stencil[1], op,
                                            stencilRefs[1], stencilVals);

      res = lp_build_select(bld, front_facing, res, back_res);
   }

   if (stencil[0].writemask != 0xff ||
       (stencil[1].enabled && front_facing != NULL &&
        stencil[1].writemask != 0xff)) {
      /* mask &= stencil[0].writemask */
      LLVMValueRef writemask = lp_build_const_int_vec(bld->gallivm, bld->type,
                                                      stencil[0].writemask);
      if (stencil[1].enabled &&
          stencil[1].writemask != stencil[0].writemask &&
          front_facing != NULL) {
         LLVMValueRef back_writemask =
            lp_build_const_int_vec(bld->gallivm, bld->type,
                                   stencil[1].writemask);
         writemask = lp_build_select(bld, front_facing,
                                     writemask, back_writemask);
      }

      mask = LLVMBuildAnd(builder, mask, writemask, "");
      /* res = (res & mask) | (stencilVals & ~mask) */
      res = lp_build_select_bitwise(bld, mask, res, stencilVals);
   } else {
      /* res = mask ? res : stencilVals */
      res = lp_build_select(bld, mask, res, stencilVals);
   }

   return res;
}



/**
 * Return a type that matches the depth/stencil format.
 */
struct lp_type
lp_depth_type(const struct util_format_description *format_desc,
              unsigned length)
{
   struct lp_type type;
   unsigned z_swizzle;

   assert(format_desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS);
   assert(format_desc->block.width == 1);
   assert(format_desc->block.height == 1);

   memset(&type, 0, sizeof type);
   type.width = format_desc->block.bits;

   z_swizzle = format_desc->swizzle[0];
   if (z_swizzle < 4) {
      if (format_desc->channel[z_swizzle].type == UTIL_FORMAT_TYPE_FLOAT) {
         type.floating = true;
         assert(z_swizzle == 0);
         assert(format_desc->channel[z_swizzle].size == 32);
      }
      else if (format_desc->channel[z_swizzle].type == UTIL_FORMAT_TYPE_UNSIGNED) {
         assert(format_desc->block.bits <= 32);
         assert(format_desc->channel[z_swizzle].normalized);
         if (format_desc->channel[z_swizzle].size < format_desc->block.bits) {
            /* Prefer signed integers when possible, as SSE has less support
             * for unsigned comparison;
             */
            type.sign = true;
         }
      }
      else
         assert(0);
   }

   type.length = length;

   return type;
}


/**
 * Compute bitmask and bit shift to apply to the incoming fragment Z values
 * and the Z buffer values needed before doing the Z comparison.
 *
 * Note that we leave the Z bits in the position that we find them
 * in the Z buffer (typically 0xffffff00 or 0x00ffffff).  That lets us
 * get by with fewer bit twiddling steps.
 */
static bool
get_z_shift_and_mask(const struct util_format_description *format_desc,
                     unsigned *shift, unsigned *width, unsigned *mask)
{
   unsigned total_bits;
   unsigned z_swizzle;

   assert(format_desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS);
   assert(format_desc->block.width == 1);
   assert(format_desc->block.height == 1);

   /* 64bit d/s format is special already extracted 32 bits */
   total_bits = format_desc->block.bits > 32 ? 32 : format_desc->block.bits;

   z_swizzle = format_desc->swizzle[0];

   if (z_swizzle == PIPE_SWIZZLE_NONE)
      return false;

   *width = format_desc->channel[z_swizzle].size;
   /* & 31 is for the same reason as the 32-bit limit above */
   *shift = format_desc->channel[z_swizzle].shift & 31;

   if (*width == total_bits) {
      *mask = 0xffffffff;
   } else {
      *mask = ((1 << *width) - 1) << *shift;
   }

   return true;
}


/**
 * Compute bitmask and bit shift to apply to the framebuffer pixel values
 * to put the stencil bits in the least significant position.
 * (i.e. 0x000000ff)
 */
static bool
get_s_shift_and_mask(const struct util_format_description *format_desc,
                     unsigned *shift, unsigned *mask)
{
   const unsigned s_swizzle = format_desc->swizzle[1];

   if (s_swizzle == PIPE_SWIZZLE_NONE)
      return false;

   /* just special case 64bit d/s format */
   if (format_desc->block.bits > 32) {
      /* XXX big-endian? */
      assert(format_desc->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT);
      *shift = 0;
      *mask = 0xff;
      return true;
   }

   *shift = format_desc->channel[s_swizzle].shift;
   const unsigned sz = format_desc->channel[s_swizzle].size;
   *mask = (1U << sz) - 1U;

   return true;
}


/**
 * Perform the occlusion test and increase the counter.
 * Test the depth mask. Add the number of channel which has none zero mask
 * into the occlusion counter. e.g. maskvalue is {-1, -1, -1, -1}.
 * The counter will add 4.
 * TODO: could get that out of the fs loop.
 *
 * \param type holds element type of the mask vector.
 * \param maskvalue is the depth test mask.
 * \param counter is a pointer of the uint32 counter.
 */
void
lp_build_occlusion_count(struct gallivm_state *gallivm,
                         struct lp_type type,
                         LLVMValueRef maskvalue,
                         LLVMValueRef counter)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMContextRef context = gallivm->context;
   LLVMValueRef countmask = lp_build_const_int_vec(gallivm, type, 1);
   LLVMValueRef count, newcount;

   assert(type.length <= 16);
   assert(type.floating);

   if (util_get_cpu_caps()->has_sse && type.length == 4) {
      const char *movmskintr = "llvm.x86.sse.movmsk.ps";
      const char *popcntintr = "llvm.ctpop.i32";
      LLVMValueRef bits = LLVMBuildBitCast(builder, maskvalue,
                                           lp_build_vec_type(gallivm, type), "");
      bits = lp_build_intrinsic_unary(builder, movmskintr,
                                      LLVMInt32TypeInContext(context), bits);
      count = lp_build_intrinsic_unary(builder, popcntintr,
                                       LLVMInt32TypeInContext(context), bits);
      count = LLVMBuildZExt(builder, count, LLVMIntTypeInContext(context, 64), "");
   }
   else if (util_get_cpu_caps()->has_avx && type.length == 8) {
      const char *movmskintr = "llvm.x86.avx.movmsk.ps.256";
      const char *popcntintr = "llvm.ctpop.i32";
      LLVMValueRef bits = LLVMBuildBitCast(builder, maskvalue,
                                           lp_build_vec_type(gallivm, type), "");
      bits = lp_build_intrinsic_unary(builder, movmskintr,
                                      LLVMInt32TypeInContext(context), bits);
      count = lp_build_intrinsic_unary(builder, popcntintr,
                                       LLVMInt32TypeInContext(context), bits);
      count = LLVMBuildZExt(builder, count, LLVMIntTypeInContext(context, 64), "");
   } else {
      LLVMValueRef countv = LLVMBuildAnd(builder, maskvalue, countmask, "countv");
      LLVMTypeRef counttype = LLVMIntTypeInContext(context, type.length * 8);
      LLVMTypeRef i8vntype = LLVMVectorType(LLVMInt8TypeInContext(context), type.length * 4);
      LLVMValueRef shufflev, countd;
      LLVMValueRef shuffles[16];
      const char *popcntintr = NULL;

      countv = LLVMBuildBitCast(builder, countv, i8vntype, "");

       for (unsigned i = 0; i < type.length; i++) {
#if UTIL_ARCH_LITTLE_ENDIAN
          shuffles[i] = lp_build_const_int32(gallivm, 4*i);
#else
          shuffles[i] = lp_build_const_int32(gallivm, (4*i) + 3);
#endif
       }

       shufflev = LLVMConstVector(shuffles, type.length);
       countd = LLVMBuildShuffleVector(builder, countv, LLVMGetUndef(i8vntype), shufflev, "");
       countd = LLVMBuildBitCast(builder, countd, counttype, "countd");

       /*
        * XXX FIXME
        * this is bad on cpus without popcount (on x86 supported by intel
        * nehalem, amd barcelona, and up - not tied to sse42).
        * Would be much faster to just sum the 4 elements of the vector with
        * some horizontal add (shuffle/add/shuffle/add after the initial and).
        */
       switch (type.length) {
       case 4:
          popcntintr = "llvm.ctpop.i32";
          break;
       case 8:
          popcntintr = "llvm.ctpop.i64";
          break;
       case 16:
          popcntintr = "llvm.ctpop.i128";
          break;
       default:
          assert(0);
       }
       count = lp_build_intrinsic_unary(builder, popcntintr, counttype, countd);

       if (type.length > 8) {
          count = LLVMBuildTrunc(builder, count, LLVMIntTypeInContext(context, 64), "");
       }
       else if (type.length < 8) {
          count = LLVMBuildZExt(builder, count, LLVMIntTypeInContext(context, 64), "");
       }
   }
   newcount = LLVMBuildLoad2(builder, LLVMTypeOf(count), counter, "origcount");
   newcount = LLVMBuildAdd(builder, newcount, count, "newcount");
   LLVMBuildStore(builder, newcount, counter);
}


/**
 * Load depth/stencil values.
 * The stored values are linear, swizzle them.
 *
 * \param type  the data type of the fragment depth/stencil values
 * \param format_desc  description of the depth/stencil surface
 * \param is_1d  whether this resource has only one dimension
 * \param loop_counter  the current loop iteration
 * \param depth_ptr  pointer to the depth/stencil values of this 4x4 block
 * \param depth_stride  stride of the depth/stencil buffer
 * \param z_fb  contains z values loaded from fb (may include padding)
 * \param s_fb  contains s values loaded from fb (may include padding)
 */
void
lp_build_depth_stencil_load_swizzled(struct gallivm_state *gallivm,
                                     struct lp_type z_src_type,
                                     const struct util_format_description *format_desc,
                                     bool is_1d,
                                     LLVMValueRef depth_ptr,
                                     LLVMValueRef depth_stride,
                                     LLVMValueRef *z_fb,
                                     LLVMValueRef *s_fb,
                                     LLVMValueRef loop_counter)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH / 4];
   LLVMValueRef depth_offset1, depth_offset2;
   const unsigned depth_bytes = format_desc->block.bits / 8;
   struct lp_type zs_type = lp_depth_type(format_desc, z_src_type.length);

   struct lp_type zs_load_type = zs_type;
   zs_load_type.length = zs_load_type.length / 2;

   LLVMTypeRef zs_dst_type = lp_build_vec_type(gallivm, zs_load_type);

   if (z_src_type.length == 4) {
      LLVMValueRef looplsb = LLVMBuildAnd(builder, loop_counter,
                                          lp_build_const_int32(gallivm, 1), "");
      LLVMValueRef loopmsb = LLVMBuildAnd(builder, loop_counter,
                                          lp_build_const_int32(gallivm, 2), "");
      LLVMValueRef offset2 = LLVMBuildMul(builder, loopmsb,
                                          depth_stride, "");
      depth_offset1 = LLVMBuildMul(builder, looplsb,
                                   lp_build_const_int32(gallivm, depth_bytes * 2), "");
      depth_offset1 = LLVMBuildAdd(builder, depth_offset1, offset2, "");

      /* just concatenate the loaded 2x2 values into 4-wide vector */
      for (unsigned i = 0; i < 4; i++) {
         shuffles[i] = lp_build_const_int32(gallivm, i);
      }
   } else {
      unsigned i;
      LLVMValueRef loopx2 = LLVMBuildShl(builder, loop_counter,
                                         lp_build_const_int32(gallivm, 1), "");
      assert(z_src_type.length == 8);
      depth_offset1 = LLVMBuildMul(builder, loopx2, depth_stride, "");
      /*
       * We load 2x4 values, and need to swizzle them (order
       * 0,1,4,5,2,3,6,7) - not so hot with avx unfortunately.
       */
      for (i = 0; i < 8; i++) {
         shuffles[i] = lp_build_const_int32(gallivm, (i&1) + (i&2) * 2 + (i&4) / 2);
      }
   }

   depth_offset2 = LLVMBuildAdd(builder, depth_offset1, depth_stride, "");

   /* Load current z/stencil values from z/stencil buffer */
   LLVMTypeRef load_ptr_type = LLVMPointerType(zs_dst_type, 0);
   LLVMTypeRef int8_type = LLVMInt8TypeInContext(gallivm->context);
   LLVMValueRef zs_dst_ptr =
      LLVMBuildGEP2(builder, int8_type, depth_ptr, &depth_offset1, 1, "");
   zs_dst_ptr = LLVMBuildBitCast(builder, zs_dst_ptr, load_ptr_type, "");
   LLVMValueRef zs_dst1 = LLVMBuildLoad2(builder, zs_dst_type, zs_dst_ptr, "");
   LLVMValueRef zs_dst2;
   if (is_1d) {
      zs_dst2 = lp_build_undef(gallivm, zs_load_type);
   } else {
      zs_dst_ptr = LLVMBuildGEP2(builder, int8_type, depth_ptr, &depth_offset2, 1, "");
      zs_dst_ptr = LLVMBuildBitCast(builder, zs_dst_ptr, load_ptr_type, "");
      zs_dst2 = LLVMBuildLoad2(builder, zs_dst_type, zs_dst_ptr, "");
   }

   *z_fb = LLVMBuildShuffleVector(builder, zs_dst1, zs_dst2,
                                  LLVMConstVector(shuffles, zs_type.length), "");
   *s_fb = *z_fb;

   if (format_desc->block.bits == 8) {
      /* Extend stencil-only 8 bit values (S8_UINT) */
      *s_fb = LLVMBuildZExt(builder, *s_fb,
                            lp_build_int_vec_type(gallivm, z_src_type), "");
   }

   if (format_desc->block.bits < z_src_type.width) {
      /* Extend destination ZS values (e.g., when reading from Z16_UNORM) */
      *z_fb = LLVMBuildZExt(builder, *z_fb,
                            lp_build_int_vec_type(gallivm, z_src_type), "");
   }

   else if (format_desc->block.bits > 32) {
      /* rely on llvm to handle too wide vector we have here nicely */
      struct lp_type typex2 = zs_type;
      struct lp_type s_type = zs_type;
      LLVMValueRef shuffles1[LP_MAX_VECTOR_LENGTH / 4];
      LLVMValueRef shuffles2[LP_MAX_VECTOR_LENGTH / 4];
      LLVMValueRef tmp;

      typex2.width = typex2.width / 2;
      typex2.length = typex2.length * 2;
      s_type.width = s_type.width / 2;
      s_type.floating = 0;

      tmp = LLVMBuildBitCast(builder, *z_fb,
                             lp_build_vec_type(gallivm, typex2), "");

      for (unsigned i = 0; i < zs_type.length; i++) {
         shuffles1[i] = lp_build_const_int32(gallivm, i * 2);
         shuffles2[i] = lp_build_const_int32(gallivm, i * 2 + 1);
      }
      *z_fb = LLVMBuildShuffleVector(builder, tmp, tmp,
                                     LLVMConstVector(shuffles1, zs_type.length), "");
      *s_fb = LLVMBuildShuffleVector(builder, tmp, tmp,
                                     LLVMConstVector(shuffles2, zs_type.length), "");
      *s_fb = LLVMBuildBitCast(builder, *s_fb,
                               lp_build_vec_type(gallivm, s_type), "");
      lp_build_name(*s_fb, "s_dst");
   }

   lp_build_name(*z_fb, "z_dst");
   lp_build_name(*s_fb, "s_dst");
   lp_build_name(*z_fb, "z_dst");
}


/**
 * Store depth/stencil values.
 * Incoming values are swizzled (typically n 2x2 quads), stored linear.
 * If there's a mask it will do select/store otherwise just store.
 *
 * \param type  the data type of the fragment depth/stencil values
 * \param format_desc  description of the depth/stencil surface
 * \param is_1d  whether this resource has only one dimension
 * \param mask_value the alive/dead pixel mask for the quad (vector)
 * \param z_fb  z values read from fb (with padding)
 * \param s_fb  s values read from fb (with padding)
 * \param loop_counter  the current loop iteration
 * \param depth_ptr  pointer to the depth/stencil values of this 4x4 block
 * \param depth_stride  stride of the depth/stencil buffer
 * \param z_value the depth values to store (with padding)
 * \param s_value the stencil values to store (with padding)
 */
void
lp_build_depth_stencil_write_swizzled(struct gallivm_state *gallivm,
                                      struct lp_type z_src_type,
                                      const struct util_format_description *format_desc,
                                      bool is_1d,
                                      LLVMValueRef mask_value,
                                      LLVMValueRef z_fb,
                                      LLVMValueRef s_fb,
                                      LLVMValueRef loop_counter,
                                      LLVMValueRef depth_ptr,
                                      LLVMValueRef depth_stride,
                                      LLVMValueRef z_value,
                                      LLVMValueRef s_value)
{
   struct lp_build_context z_bld;
   LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH / 4];
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef zs_dst1, zs_dst2;
   LLVMValueRef zs_dst_ptr1, zs_dst_ptr2;
   LLVMValueRef depth_offset1, depth_offset2;
   LLVMTypeRef load_ptr_type;
   unsigned depth_bytes = format_desc->block.bits / 8;
   struct lp_type zs_type = lp_depth_type(format_desc, z_src_type.length);
   struct lp_type z_type = zs_type;
   struct lp_type zs_load_type = zs_type;

   zs_load_type.length = zs_load_type.length / 2;
   load_ptr_type = LLVMPointerType(lp_build_vec_type(gallivm, zs_load_type), 0);

   z_type.width = z_src_type.width;

   lp_build_context_init(&z_bld, gallivm, z_type);

   /*
    * This is far from ideal, at least for late depth write we should do this
    * outside the fs loop to avoid all the swizzle stuff.
    */
   if (z_src_type.length == 4) {
      LLVMValueRef looplsb = LLVMBuildAnd(builder, loop_counter,
                                          lp_build_const_int32(gallivm, 1), "");
      LLVMValueRef loopmsb = LLVMBuildAnd(builder, loop_counter,
                                          lp_build_const_int32(gallivm, 2), "");
      LLVMValueRef offset2 = LLVMBuildMul(builder, loopmsb,
                                          depth_stride, "");
      depth_offset1 = LLVMBuildMul(builder, looplsb,
                                   lp_build_const_int32(gallivm, depth_bytes * 2), "");
      depth_offset1 = LLVMBuildAdd(builder, depth_offset1, offset2, "");
   } else {
      LLVMValueRef loopx2 = LLVMBuildShl(builder, loop_counter,
                                         lp_build_const_int32(gallivm, 1), "");
      assert(z_src_type.length == 8);
      depth_offset1 = LLVMBuildMul(builder, loopx2, depth_stride, "");
      /*
       * We load 2x4 values, and need to swizzle them (order
       * 0,1,4,5,2,3,6,7) - not so hot with avx unfortunately.
       */
      for (unsigned i = 0; i < 8; i++) {
         shuffles[i] = lp_build_const_int32(gallivm, (i&1) + (i&2) * 2 + (i&4) / 2);
      }
   }

   depth_offset2 = LLVMBuildAdd(builder, depth_offset1, depth_stride, "");

   LLVMTypeRef int8_type = LLVMInt8TypeInContext(gallivm->context);
   zs_dst_ptr1 = LLVMBuildGEP2(builder, int8_type, depth_ptr, &depth_offset1, 1, "");
   zs_dst_ptr1 = LLVMBuildBitCast(builder, zs_dst_ptr1, load_ptr_type, "");
   zs_dst_ptr2 = LLVMBuildGEP2(builder, int8_type, depth_ptr, &depth_offset2, 1, "");
   zs_dst_ptr2 = LLVMBuildBitCast(builder, zs_dst_ptr2, load_ptr_type, "");

   if (format_desc->block.bits > 32) {
      s_value = LLVMBuildBitCast(builder, s_value, z_bld.vec_type, "");
   }

   if (mask_value) {
      z_value = lp_build_select(&z_bld, mask_value, z_value, z_fb);
      if (format_desc->block.bits > 32) {
         s_fb = LLVMBuildBitCast(builder, s_fb, z_bld.vec_type, "");
         s_value = lp_build_select(&z_bld, mask_value, s_value, s_fb);
      }
   }

   if (zs_type.width < z_src_type.width) {
      /* Truncate ZS values (e.g., when writing to Z16_UNORM) */
      z_value = LLVMBuildTrunc(builder, z_value,
                               lp_build_int_vec_type(gallivm, zs_type), "");
   }

   if (format_desc->block.bits <= 32) {
      if (z_src_type.length == 4) {
         zs_dst1 = lp_build_extract_range(gallivm, z_value, 0, 2);
         zs_dst2 = lp_build_extract_range(gallivm, z_value, 2, 2);
      } else {
         assert(z_src_type.length == 8);
         zs_dst1 = LLVMBuildShuffleVector(builder, z_value, z_value,
                                          LLVMConstVector(&shuffles[0],
                                                          zs_load_type.length), "");
         zs_dst2 = LLVMBuildShuffleVector(builder, z_value, z_value,
                                          LLVMConstVector(&shuffles[4],
                                                          zs_load_type.length), "");
      }
   } else {
      if (z_src_type.length == 4) {
         zs_dst1 = lp_build_interleave2(gallivm, z_type,
                                        z_value, s_value, 0);
         zs_dst2 = lp_build_interleave2(gallivm, z_type,
                                        z_value, s_value, 1);
      } else {
         LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH / 2];
         assert(z_src_type.length == 8);
         for (unsigned i = 0; i < 8; i++) {
            shuffles[i*2] = lp_build_const_int32(gallivm, (i&1) + (i&2) * 2 + (i&4) / 2);
            shuffles[i*2+1] = lp_build_const_int32(gallivm, (i&1) + (i&2) * 2 + (i&4) / 2 +
                                                   z_src_type.length);
         }
         zs_dst1 = LLVMBuildShuffleVector(builder, z_value, s_value,
                                          LLVMConstVector(&shuffles[0],
                                                          z_src_type.length), "");
         zs_dst2 = LLVMBuildShuffleVector(builder, z_value, s_value,
                                          LLVMConstVector(&shuffles[8],
                                                          z_src_type.length), "");
      }
      zs_dst1 = LLVMBuildBitCast(builder, zs_dst1,
                                 lp_build_vec_type(gallivm, zs_load_type), "");
      zs_dst2 = LLVMBuildBitCast(builder, zs_dst2,
                                 lp_build_vec_type(gallivm, zs_load_type), "");
   }

   LLVMBuildStore(builder, zs_dst1, zs_dst_ptr1);
   if (!is_1d) {
      LLVMBuildStore(builder, zs_dst2, zs_dst_ptr2);
   }
}


/**
 * Generate code for performing depth and/or stencil tests.
 * We operate on a vector of values (typically n 2x2 quads).
 *
 * \param depth  the depth test state
 * \param stencil  the front/back stencil state
 * \param type  the data type of the fragment depth/stencil values
 * \param format_desc  description of the depth/stencil surface
 * \param mask  the alive/dead pixel mask for the quad (vector)
 * \param cov_mask coverage mask
 * \param stencil_refs  the front/back stencil ref values (scalar)
 * \param z_src  the incoming depth/stencil values (n 2x2 quad values, float32)
 * \param zs_dst  the depth/stencil values in framebuffer
 * \param face  contains boolean value indicating front/back facing polygon
 */
void
lp_build_depth_stencil_test(struct gallivm_state *gallivm,
                            const struct lp_depth_state *depth,
                            const struct pipe_stencil_state stencil[2],
                            struct lp_type z_src_type,
                            const struct util_format_description *format_desc,
                            struct lp_build_mask_context *mask,
                            LLVMValueRef *cov_mask,
                            LLVMValueRef stencil_refs[2],
                            LLVMValueRef z_src,
                            LLVMValueRef z_fb,
                            LLVMValueRef s_fb,
                            LLVMValueRef face,
                            LLVMValueRef *z_value,
                            LLVMValueRef *s_value,
                            bool do_branch,
                            bool restrict_depth)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type z_type;
   struct lp_build_context z_bld;
   struct lp_build_context s_bld;
   struct lp_type s_type;
   unsigned z_shift = 0, z_width = 0, z_mask = 0;
   LLVMValueRef z_dst = NULL;
   LLVMValueRef stencil_vals = NULL;
   LLVMValueRef z_bitmask = NULL, stencil_shift = NULL;
   LLVMValueRef z_pass = NULL, s_pass_mask = NULL;
   LLVMValueRef current_mask = mask ? lp_build_mask_value(mask) : *cov_mask;
   LLVMValueRef front_facing = NULL;
   bool have_z, have_s;

   /*
    * Depths are expected to be between 0 and 1, even if they are stored in
    * floats. Setting these bits here will ensure that the lp_build_conv() call
    * below won't try to unnecessarily clamp the incoming values.
    * If depths are expected outside 0..1 don't set these bits.
    */
   if (z_src_type.floating) {
      if (restrict_depth) {
         z_src_type.sign = false;
         z_src_type.norm = true;
      }
   } else {
      assert(!z_src_type.sign);
      assert(z_src_type.norm);
   }

   /* Pick the type matching the depth-stencil format. */
   z_type = lp_depth_type(format_desc, z_src_type.length);

   /* Pick the intermediate type for depth operations. */
   z_type.width = z_src_type.width;
   assert(z_type.length == z_src_type.length);

   /* FIXME: for non-float depth/stencil might generate better code
    * if we'd always split it up to use 128bit operations.
    * For stencil we'd almost certainly want to pack to 8xi16 values,
    * for z just run twice.
    */

   /* Sanity checking */
   {
      ASSERTED const unsigned z_swizzle = format_desc->swizzle[0];
      ASSERTED const unsigned s_swizzle = format_desc->swizzle[1];

      assert(z_swizzle != PIPE_SWIZZLE_NONE ||
             s_swizzle != PIPE_SWIZZLE_NONE);

      assert(depth->enabled || stencil[0].enabled);

      assert(format_desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS);
      assert(format_desc->block.width == 1);
      assert(format_desc->block.height == 1);

      if (stencil[0].enabled) {
         assert(s_swizzle < 4);
         assert(format_desc->channel[s_swizzle].type == UTIL_FORMAT_TYPE_UNSIGNED);
         assert(format_desc->channel[s_swizzle].pure_integer);
         assert(!format_desc->channel[s_swizzle].normalized);
         assert(format_desc->channel[s_swizzle].size == 8);
      }

      if (depth->enabled) {
         assert(z_swizzle < 4);
         if (z_type.floating) {
            assert(z_swizzle == 0);
            assert(format_desc->channel[z_swizzle].type ==
                   UTIL_FORMAT_TYPE_FLOAT);
            assert(format_desc->channel[z_swizzle].size == 32);
         } else {
            assert(format_desc->channel[z_swizzle].type ==
                   UTIL_FORMAT_TYPE_UNSIGNED);
            assert(format_desc->channel[z_swizzle].normalized);
            assert(!z_type.fixed);
         }
      }
   }


   /* Setup build context for Z vals */
   lp_build_context_init(&z_bld, gallivm, z_type);

   /* Setup build context for stencil vals */
   s_type = lp_int_type(z_type);
   lp_build_context_init(&s_bld, gallivm, s_type);

   /* Compute and apply the Z/stencil bitmasks and shifts.
    */
   {
      unsigned s_shift, s_mask;

      z_dst = z_fb;
      stencil_vals = s_fb;

      have_z = get_z_shift_and_mask(format_desc, &z_shift, &z_width, &z_mask);
      have_s = get_s_shift_and_mask(format_desc, &s_shift, &s_mask);

      if (have_z) {
         if (z_mask != 0xffffffff) {
            z_bitmask = lp_build_const_int_vec(gallivm, z_type, z_mask);
         }

         /*
          * Align the framebuffer Z 's LSB to the right.
          */
         if (z_shift) {
            LLVMValueRef shift = lp_build_const_int_vec(gallivm, z_type, z_shift);
            z_dst = LLVMBuildLShr(builder, z_dst, shift, "z_dst");
         } else if (z_bitmask) {
            z_dst = LLVMBuildAnd(builder, z_dst, z_bitmask, "z_dst");
         } else {
            lp_build_name(z_dst, "z_dst");
         }
      }

      if (have_s) {
         if (s_shift) {
            LLVMValueRef shift = lp_build_const_int_vec(gallivm, s_type, s_shift);
            stencil_vals = LLVMBuildLShr(builder, stencil_vals, shift, "");
            stencil_shift = shift;  /* used below */
         }

         if (s_mask != 0xffffffff) {
            LLVMValueRef mask = lp_build_const_int_vec(gallivm, s_type, s_mask);
            stencil_vals = LLVMBuildAnd(builder, stencil_vals, mask, "");
         }

         lp_build_name(stencil_vals, "s_dst");
      }
   }

   if (stencil[0].enabled) {

      if (face) {
         if (0) {
            /*
             * XXX: the scalar expansion below produces atrocious code
             * (basically producing a 64bit scalar value, then moving the 2
             * 32bit pieces separately to simd, plus 4 shuffles, which is
             * seriously lame). But the scalar-simd transitions are always
             * tricky, so no big surprise there.
             * This here would be way better, however llvm has some serious
             * trouble later using it in the select, probably because it will
             * recognize the expression as constant and move the simd value
             * away (out of the loop) - and then it will suddenly try
             * constructing i1 high-bit masks out of it later...
             * (Try piglit stencil-twoside.)
             * Note this is NOT due to using SExt/Trunc, it fails exactly the
             * same even when using native compare/select.
             * I cannot reproduce this problem when using stand-alone compiler
             * though, suggesting some problem with optimization passes...
             * (With stand-alone compilation, the construction of this mask
             * value, no matter if the easy 3 instruction here or the complex
             * 16+ one below, never gets separated from where it's used.)
             * The scalar code still has the same problem, but the generated
             * code looks a bit better at least for some reason, even if
             * mostly by luck (the fundamental issue clearly is the same).
             */
            front_facing = lp_build_broadcast(gallivm, s_bld.vec_type, face);
            /* front_facing = face != 0 ? ~0 : 0 */
            front_facing = lp_build_compare(gallivm, s_bld.type,
                                            PIPE_FUNC_NOTEQUAL,
                                            front_facing, s_bld.zero);
         } else {
            LLVMValueRef zero = lp_build_const_int32(gallivm, 0);

            /* front_facing = face != 0 ? ~0 : 0 */
            front_facing = LLVMBuildICmp(builder, LLVMIntNE, face, zero, "");
            front_facing = LLVMBuildSExt(builder, front_facing,
                                         LLVMIntTypeInContext(gallivm->context,
                                                s_bld.type.length*s_bld.type.width),
                                         "");
            front_facing = LLVMBuildBitCast(builder, front_facing,
                                            s_bld.int_vec_type, "");

         }
      }

      s_pass_mask = lp_build_stencil_test(&s_bld, stencil,
                                          stencil_refs, stencil_vals,
                                          front_facing);

      /* apply stencil-fail operator */
      {
         LLVMValueRef s_fail_mask = lp_build_andnot(&s_bld, current_mask, s_pass_mask);
         stencil_vals = lp_build_stencil_op(&s_bld, stencil, S_FAIL_OP,
                                            stencil_refs, stencil_vals,
                                            s_fail_mask, front_facing);
      }
   }

   if (depth->enabled) {
      /*
       * Convert fragment Z to the desired type, aligning the LSB to the right.
       */

      assert(z_type.width == z_src_type.width);
      assert(z_type.length == z_src_type.length);
      assert(lp_check_value(z_src_type, z_src));
      if (z_src_type.floating) {
         /*
          * Convert from floating point values
          */

         if (!z_type.floating) {
            z_src = lp_build_clamped_float_to_unsigned_norm(gallivm,
                                                            z_src_type,
                                                            z_width,
                                                            z_src);
         }
      } else {
         /*
          * Convert from unsigned normalized values.
          */

         assert(!z_src_type.sign);
         assert(!z_src_type.fixed);
         assert(z_src_type.norm);
         assert(!z_type.floating);
         if (z_src_type.width > z_width) {
            LLVMValueRef shift = lp_build_const_int_vec(gallivm, z_src_type,
                                                        z_src_type.width - z_width);
            z_src = LLVMBuildLShr(builder, z_src, shift, "");
         }
      }
      assert(lp_check_value(z_type, z_src));

      lp_build_name(z_src, "z_src");

      /* compare src Z to dst Z, returning 'pass' mask */
      z_pass = lp_build_cmp(&z_bld, depth->func, z_src, z_dst);

      /* mask off bits that failed stencil test */
      if (s_pass_mask) {
         current_mask = LLVMBuildAnd(builder, current_mask, s_pass_mask, "");
      }

      if (!stencil[0].enabled && mask) {
         /* We can potentially skip all remaining operations here, but only
          * if stencil is disabled because we still need to update the stencil
          * buffer values.  Don't need to update Z buffer values.
          */
         lp_build_mask_update(mask, z_pass);

         if (do_branch) {
            lp_build_mask_check(mask);
         }
      }

      if (depth->writemask) {
         LLVMValueRef z_pass_mask;

         /* mask off bits that failed Z test */
         z_pass_mask = LLVMBuildAnd(builder, current_mask, z_pass, "");

         /* Mix the old and new Z buffer values.
          * z_dst[i] = zselectmask[i] ? z_src[i] : z_dst[i]
          */
         z_dst = lp_build_select(&z_bld, z_pass_mask, z_src, z_dst);
      }

      if (stencil[0].enabled) {
         /* update stencil buffer values according to z pass/fail result */
         LLVMValueRef z_fail_mask, z_pass_mask;

         /* apply Z-fail operator */
         z_fail_mask = lp_build_andnot(&s_bld, current_mask, z_pass);
         stencil_vals = lp_build_stencil_op(&s_bld, stencil, Z_FAIL_OP,
                                            stencil_refs, stencil_vals,
                                            z_fail_mask, front_facing);

         /* apply Z-pass operator */
         z_pass_mask = LLVMBuildAnd(builder, current_mask, z_pass, "");
         stencil_vals = lp_build_stencil_op(&s_bld, stencil, Z_PASS_OP,
                                            stencil_refs, stencil_vals,
                                            z_pass_mask, front_facing);
      }
   } else {
      /* No depth test: apply Z-pass operator to stencil buffer values which
       * passed the stencil test.
       */
      s_pass_mask = LLVMBuildAnd(builder, current_mask, s_pass_mask, "");
      stencil_vals = lp_build_stencil_op(&s_bld, stencil, Z_PASS_OP,
                                         stencil_refs, stencil_vals,
                                         s_pass_mask, front_facing);
   }

   /* Put Z and stencil bits in the right place */
   if (have_z && z_shift) {
      LLVMValueRef shift = lp_build_const_int_vec(gallivm, z_type, z_shift);
      z_dst = LLVMBuildShl(builder, z_dst, shift, "");
   }
   if (stencil_vals && stencil_shift)
      stencil_vals = LLVMBuildShl(builder, stencil_vals,
                                  stencil_shift, "");

   /* Finally, merge the z/stencil values */
   if (format_desc->block.bits <= 32) {
      if (have_z && have_s)
         *z_value = LLVMBuildOr(builder, z_dst, stencil_vals, "");
      else if (have_z)
         *z_value = z_dst;
      else
         *z_value = stencil_vals;
      *s_value = *z_value;
   } else {
      *z_value = z_dst;
      *s_value = stencil_vals;
   }

   if (mask) {
      if (s_pass_mask)
         lp_build_mask_update(mask, s_pass_mask);

      if (depth->enabled && stencil[0].enabled)
         lp_build_mask_update(mask, z_pass);
   } else {
      LLVMValueRef tmp_mask = *cov_mask;
      if (s_pass_mask)
         tmp_mask = LLVMBuildAnd(builder, tmp_mask, s_pass_mask, "");

      /* for multisample we don't do the stencil optimisation so update always */
      if (depth->enabled)
         tmp_mask = LLVMBuildAnd(builder, tmp_mask, z_pass, "");
      *cov_mask = tmp_mask;
   }
}

