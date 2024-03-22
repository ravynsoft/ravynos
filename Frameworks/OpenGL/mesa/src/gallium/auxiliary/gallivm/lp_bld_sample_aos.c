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
 * Texture sampling -- AoS.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 * @author Brian Paul <brianp@vmware.com>
 */

#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/u_debug.h"
#include "util/u_dump.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/format/u_format.h"
#include "util/u_cpu_detect.h"
#include "lp_bld_debug.h"
#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_conv.h"
#include "lp_bld_arit.h"
#include "lp_bld_bitarit.h"
#include "lp_bld_logic.h"
#include "lp_bld_swizzle.h"
#include "lp_bld_pack.h"
#include "lp_bld_flow.h"
#include "lp_bld_gather.h"
#include "lp_bld_format.h"
#include "lp_bld_init.h"
#include "lp_bld_sample.h"
#include "lp_bld_sample_aos.h"
#include "lp_bld_quad.h"


/**
 * Build LLVM code for texture coord wrapping, for nearest filtering,
 * for scaled integer texcoords.
 * \param block_length  is the length of the pixel block along the
 *                      coordinate axis
 * \param coord  the incoming texcoord (s,t or r) scaled to the texture size
 * \param coord_f  the incoming texcoord (s,t or r) as float vec
 * \param length  the texture size along one dimension
 * \param stride  pixel stride along the coordinate axis (in bytes)
 * \param offset  the texel offset along the coord axis
 * \param is_pot  if TRUE, length is a power of two
 * \param wrap_mode  one of PIPE_TEX_WRAP_x
 * \param out_offset  byte offset for the wrapped coordinate
 * \param out_i  resulting sub-block pixel coordinate for coord0
 */
static void
lp_build_sample_wrap_nearest_int(struct lp_build_sample_context *bld,
                                 unsigned block_length,
                                 LLVMValueRef coord,
                                 LLVMValueRef coord_f,
                                 LLVMValueRef length,
                                 LLVMValueRef stride,
                                 LLVMValueRef offset,
                                 bool is_pot,
                                 unsigned wrap_mode,
                                 LLVMValueRef *out_offset,
                                 LLVMValueRef *out_i)
{
   struct lp_build_context *int_coord_bld = &bld->int_coord_bld;
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef length_minus_one;

   length_minus_one = lp_build_sub(int_coord_bld, length, int_coord_bld->one);

   switch(wrap_mode) {
   case PIPE_TEX_WRAP_REPEAT:
      if(is_pot)
         coord = LLVMBuildAnd(builder, coord, length_minus_one, "");
      else {
         struct lp_build_context *coord_bld = &bld->coord_bld;
         LLVMValueRef length_f = lp_build_int_to_float(coord_bld, length);
         if (offset) {
            offset = lp_build_int_to_float(coord_bld, offset);
            offset = lp_build_div(coord_bld, offset, length_f);
            coord_f = lp_build_add(coord_bld, coord_f, offset);
         }
         coord = lp_build_fract_safe(coord_bld, coord_f);
         coord = lp_build_mul(coord_bld, coord, length_f);
         coord = lp_build_itrunc(coord_bld, coord);
      }
      break;

   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      coord = lp_build_max(int_coord_bld, coord, int_coord_bld->zero);
      coord = lp_build_min(int_coord_bld, coord, length_minus_one);
      break;

   case PIPE_TEX_WRAP_CLAMP:
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
   default:
      assert(0);
   }

   lp_build_sample_partial_offset(int_coord_bld, block_length, coord, stride,
                                  out_offset, out_i);
}


/**
 * Helper to compute the first coord and the weight for
 * linear wrap repeat npot textures
 */
static void
lp_build_coord_repeat_npot_linear_int(struct lp_build_sample_context *bld,
                                      LLVMValueRef coord_f,
                                      LLVMValueRef length_i,
                                      LLVMValueRef length_f,
                                      LLVMValueRef *coord0_i,
                                      LLVMValueRef *weight_i)
{
   struct lp_build_context *coord_bld = &bld->coord_bld;
   struct lp_build_context *int_coord_bld = &bld->int_coord_bld;
   struct lp_build_context abs_coord_bld;
   struct lp_type abs_type;
   LLVMValueRef length_minus_one = lp_build_sub(int_coord_bld, length_i,
                                                int_coord_bld->one);
   LLVMValueRef mask, i32_c8, i32_c128, i32_c255;

   /* wrap with normalized floats is just fract */
   coord_f = lp_build_fract(coord_bld, coord_f);
   /* mul by size */
   coord_f = lp_build_mul(coord_bld, coord_f, length_f);
   /* convert to int, compute lerp weight */
   coord_f = lp_build_mul_imm(&bld->coord_bld, coord_f, 256);

   /* At this point we don't have any negative numbers so use non-signed
    * build context which might help on some archs.
    */
   abs_type = coord_bld->type;
   abs_type.sign = 0;
   lp_build_context_init(&abs_coord_bld, bld->gallivm, abs_type);
   *coord0_i = lp_build_iround(&abs_coord_bld, coord_f);

   /* subtract 0.5 (add -128) */
   i32_c128 = lp_build_const_int_vec(bld->gallivm, bld->int_coord_type, -128);
   *coord0_i = LLVMBuildAdd(bld->gallivm->builder, *coord0_i, i32_c128, "");

   /* compute fractional part (AND with 0xff) */
   i32_c255 = lp_build_const_int_vec(bld->gallivm, bld->int_coord_type, 255);
   *weight_i = LLVMBuildAnd(bld->gallivm->builder, *coord0_i, i32_c255, "");

   /* compute floor (shift right 8) */
   i32_c8 = lp_build_const_int_vec(bld->gallivm, bld->int_coord_type, 8);
   *coord0_i = LLVMBuildAShr(bld->gallivm->builder, *coord0_i, i32_c8, "");
   /*
    * we avoided the 0.5/length division before the repeat wrap,
    * now need to fix up edge cases with selects
    */
   mask = lp_build_compare(int_coord_bld->gallivm, int_coord_bld->type,
                           PIPE_FUNC_LESS, *coord0_i, int_coord_bld->zero);
   *coord0_i = lp_build_select(int_coord_bld, mask, length_minus_one, *coord0_i);
   /*
    * We should never get values too large - except if coord was nan or inf,
    * in which case things go terribly wrong...
    * Alternatively, could use fract_safe above...
    */
   *coord0_i = lp_build_min(int_coord_bld, *coord0_i, length_minus_one);
}


/**
 * Build LLVM code for texture coord wrapping, for linear filtering,
 * for scaled integer texcoords.
 * \param block_length  is the length of the pixel block along the
 *                      coordinate axis
 * \param coord0  the incoming texcoord (s,t or r) scaled to the texture size
 * \param coord_f  the incoming texcoord (s,t or r) as float vec
 * \param length  the texture size along one dimension
 * \param stride  pixel stride along the coordinate axis (in bytes)
 * \param offset  the texel offset along the coord axis
 * \param is_pot  if TRUE, length is a power of two
 * \param wrap_mode  one of PIPE_TEX_WRAP_x
 * \param offset0  resulting relative offset for coord0
 * \param offset1  resulting relative offset for coord0 + 1
 * \param i0  resulting sub-block pixel coordinate for coord0
 * \param i1  resulting sub-block pixel coordinate for coord0 + 1
 */
static void
lp_build_sample_wrap_linear_int(struct lp_build_sample_context *bld,
                                unsigned block_length,
                                LLVMValueRef coord0,
                                LLVMValueRef *weight_i,
                                LLVMValueRef coord_f,
                                LLVMValueRef length,
                                LLVMValueRef stride,
                                LLVMValueRef offset,
                                bool is_pot,
                                unsigned wrap_mode,
                                LLVMValueRef *offset0,
                                LLVMValueRef *offset1,
                                LLVMValueRef *i0,
                                LLVMValueRef *i1)
{
   struct lp_build_context *int_coord_bld = &bld->int_coord_bld;
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef length_minus_one;
   LLVMValueRef lmask, umask, mask;

   /*
    * If the pixel block covers more than one pixel then there is no easy
    * way to calculate offset1 relative to offset0. Instead, compute them
    * independently. Otherwise, try to compute offset0 and offset1 with
    * a single stride multiplication.
    */

   length_minus_one = lp_build_sub(int_coord_bld, length, int_coord_bld->one);

   if (block_length != 1) {
      LLVMValueRef coord1;
      switch(wrap_mode) {
      case PIPE_TEX_WRAP_REPEAT:
         if (is_pot) {
            coord1 = lp_build_add(int_coord_bld, coord0, int_coord_bld->one);
            coord0 = LLVMBuildAnd(builder, coord0, length_minus_one, "");
            coord1 = LLVMBuildAnd(builder, coord1, length_minus_one, "");
         }
         else {
            LLVMValueRef mask;
            LLVMValueRef length_f = lp_build_int_to_float(&bld->coord_bld, length);
            if (offset) {
               offset = lp_build_int_to_float(&bld->coord_bld, offset);
               offset = lp_build_div(&bld->coord_bld, offset, length_f);
               coord_f = lp_build_add(&bld->coord_bld, coord_f, offset);
            }
            lp_build_coord_repeat_npot_linear_int(bld, coord_f,
                                                  length, length_f,
                                                  &coord0, weight_i);
            mask = lp_build_compare(bld->gallivm, int_coord_bld->type,
                                    PIPE_FUNC_NOTEQUAL, coord0, length_minus_one);
            coord1 = LLVMBuildAnd(builder,
                                  lp_build_add(int_coord_bld, coord0,
                                               int_coord_bld->one),
                                  mask, "");
         }
         break;

      case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
         coord1 = lp_build_add(int_coord_bld, coord0, int_coord_bld->one);
         coord0 = lp_build_clamp(int_coord_bld, coord0, int_coord_bld->zero,
                                length_minus_one);
         coord1 = lp_build_clamp(int_coord_bld, coord1, int_coord_bld->zero,
                                length_minus_one);
         break;

      case PIPE_TEX_WRAP_CLAMP:
      case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      case PIPE_TEX_WRAP_MIRROR_REPEAT:
      case PIPE_TEX_WRAP_MIRROR_CLAMP:
      case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
      case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
      default:
         assert(0);
         coord0 = int_coord_bld->zero;
         coord1 = int_coord_bld->zero;
         break;
      }
      lp_build_sample_partial_offset(int_coord_bld, block_length, coord0, stride,
                                     offset0, i0);
      lp_build_sample_partial_offset(int_coord_bld, block_length, coord1, stride,
                                     offset1, i1);
      return;
   }

   *i0 = int_coord_bld->zero;
   *i1 = int_coord_bld->zero;

   switch(wrap_mode) {
   case PIPE_TEX_WRAP_REPEAT:
      if (is_pot) {
         coord0 = LLVMBuildAnd(builder, coord0, length_minus_one, "");
      }
      else {
         LLVMValueRef length_f = lp_build_int_to_float(&bld->coord_bld, length);
         if (offset) {
            offset = lp_build_int_to_float(&bld->coord_bld, offset);
            offset = lp_build_div(&bld->coord_bld, offset, length_f);
            coord_f = lp_build_add(&bld->coord_bld, coord_f, offset);
         }
         lp_build_coord_repeat_npot_linear_int(bld, coord_f,
                                               length, length_f,
                                               &coord0, weight_i);
      }

      mask = lp_build_compare(bld->gallivm, int_coord_bld->type,
                              PIPE_FUNC_NOTEQUAL, coord0, length_minus_one);

      *offset0 = lp_build_mul(int_coord_bld, coord0, stride);
      *offset1 = LLVMBuildAnd(builder,
                              lp_build_add(int_coord_bld, *offset0, stride),
                              mask, "");
      break;

   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      /* XXX this might be slower than the separate path
       * on some newer cpus. With sse41 this is 8 instructions vs. 7
       * - at least on SNB this is almost certainly slower since
       * min/max are cheaper than selects, and the muls aren't bad.
       */
      lmask = lp_build_compare(int_coord_bld->gallivm, int_coord_bld->type,
                               PIPE_FUNC_GEQUAL, coord0, int_coord_bld->zero);
      umask = lp_build_compare(int_coord_bld->gallivm, int_coord_bld->type,
                               PIPE_FUNC_LESS, coord0, length_minus_one);

      coord0 = lp_build_select(int_coord_bld, lmask, coord0, int_coord_bld->zero);
      coord0 = lp_build_select(int_coord_bld, umask, coord0, length_minus_one);

      mask = LLVMBuildAnd(builder, lmask, umask, "");

      *offset0 = lp_build_mul(int_coord_bld, coord0, stride);
      *offset1 = lp_build_add(int_coord_bld,
                              *offset0,
                              LLVMBuildAnd(builder, stride, mask, ""));
      break;

   case PIPE_TEX_WRAP_CLAMP:
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
   default:
      assert(0);
      *offset0 = int_coord_bld->zero;
      *offset1 = int_coord_bld->zero;
      break;
   }
}


/**
 * Fetch texels for image with nearest sampling.
 * Return filtered color as two vectors of 16-bit fixed point values.
 */
static void
lp_build_sample_fetch_image_nearest(struct lp_build_sample_context *bld,
                                    LLVMValueRef data_ptr,
                                    LLVMValueRef offset,
                                    LLVMValueRef x_subcoord,
                                    LLVMValueRef y_subcoord,
                                    LLVMValueRef *colors)
{
   /*
    * Fetch the pixels as 4 x 32bit (rgba order might differ):
    *
    *   rgba0 rgba1 rgba2 rgba3
    *
    * bit cast them into 16 x u8
    *
    *   r0 g0 b0 a0 r1 g1 b1 a1 r2 g2 b2 a2 r3 g3 b3 a3
    *
    * unpack them into two 8 x i16:
    *
    *   r0 g0 b0 a0 r1 g1 b1 a1
    *   r2 g2 b2 a2 r3 g3 b3 a3
    *
    * The higher 8 bits of the resulting elements will be zero.
    */
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef rgba8;
   struct lp_build_context u8n;
   LLVMTypeRef u8n_vec_type;
   struct lp_type fetch_type;

   lp_build_context_init(&u8n, bld->gallivm, lp_type_unorm(8, bld->vector_width));
   u8n_vec_type = lp_build_vec_type(bld->gallivm, u8n.type);

   fetch_type = lp_type_uint(bld->texel_type.width);
   if (util_format_is_rgba8_variant(bld->format_desc)) {
      /*
       * Given the format is a rgba8, just read the pixels as is,
       * without any swizzling. Swizzling will be done later.
       */
      rgba8 = lp_build_gather(bld->gallivm,
                              bld->texel_type.length,
                              bld->format_desc->block.bits,
                              fetch_type,
                              true,
                              data_ptr, offset, true);

      rgba8 = LLVMBuildBitCast(builder, rgba8, u8n_vec_type, "");
   }
   else {
      rgba8 = lp_build_fetch_rgba_aos(bld->gallivm,
                                      bld->format_desc,
                                      u8n.type,
                                      true,
                                      data_ptr, offset,
                                      x_subcoord,
                                      y_subcoord,
                                      bld->cache);
   }

   *colors = rgba8;
}


/**
 * Sample a single texture image with nearest sampling.
 * If sampling a cube texture, r = cube face in [0,5].
 * Return filtered color as two vectors of 16-bit fixed point values.
 */
static void
lp_build_sample_image_nearest(struct lp_build_sample_context *bld,
                              LLVMValueRef int_size,
                              LLVMValueRef row_stride_vec,
                              LLVMValueRef img_stride_vec,
                              LLVMValueRef data_ptr,
                              LLVMValueRef mipoffsets,
                              LLVMValueRef s,
                              LLVMValueRef t,
                              LLVMValueRef r,
                              const LLVMValueRef *offsets,
                              LLVMValueRef *colors)
{
   const unsigned dims = bld->dims;
   struct lp_build_context i32;
   LLVMValueRef width_vec, height_vec, depth_vec;
   LLVMValueRef s_ipart, t_ipart = NULL, r_ipart = NULL;
   LLVMValueRef s_float, t_float = NULL, r_float = NULL;
   LLVMValueRef x_stride;
   LLVMValueRef x_offset, offset;
   LLVMValueRef x_subcoord, y_subcoord = NULL, z_subcoord;

   lp_build_context_init(&i32, bld->gallivm, lp_type_int_vec(32, bld->vector_width));

   lp_build_extract_image_sizes(bld,
                                &bld->int_size_bld,
                                bld->int_coord_type,
                                int_size,
                                &width_vec,
                                &height_vec,
                                &depth_vec);

   s_float = s; t_float = t; r_float = r;

   if (bld->static_sampler_state->normalized_coords) {
      LLVMValueRef flt_size;

      flt_size = lp_build_int_to_float(&bld->float_size_bld, int_size);

      lp_build_unnormalized_coords(bld, flt_size, &s, &t, &r);
   }

   /* convert float to int */
   /* For correct rounding, need floor, not truncation here.
    * Note that in some cases (clamp to edge, no texel offsets) we
    * could use a non-signed build context which would help archs
    * greatly which don't have arch rounding.
    */
   s_ipart = lp_build_ifloor(&bld->coord_bld, s);
   if (dims >= 2)
      t_ipart = lp_build_ifloor(&bld->coord_bld, t);
   if (dims >= 3)
      r_ipart = lp_build_ifloor(&bld->coord_bld, r);

   /* add texel offsets */
   if (offsets[0]) {
      s_ipart = lp_build_add(&i32, s_ipart, offsets[0]);
      if (dims >= 2) {
         t_ipart = lp_build_add(&i32, t_ipart, offsets[1]);
         if (dims >= 3) {
            r_ipart = lp_build_add(&i32, r_ipart, offsets[2]);
         }
      }
   }

   /* get pixel, row, image strides */
   x_stride = lp_build_const_vec(bld->gallivm,
                                 bld->int_coord_bld.type,
                                 bld->format_desc->block.bits/8);

   /* Do texcoord wrapping, compute texel offset */
   lp_build_sample_wrap_nearest_int(bld,
                                    bld->format_desc->block.width,
                                    s_ipart, s_float,
                                    width_vec, x_stride, offsets[0],
                                    bld->static_texture_state->pot_width,
                                    bld->static_sampler_state->wrap_s,
                                    &x_offset, &x_subcoord);
   offset = x_offset;
   if (dims >= 2) {
      LLVMValueRef y_offset;
      lp_build_sample_wrap_nearest_int(bld,
                                       bld->format_desc->block.height,
                                       t_ipart, t_float,
                                       height_vec, row_stride_vec, offsets[1],
                                       bld->static_texture_state->pot_height,
                                       bld->static_sampler_state->wrap_t,
                                       &y_offset, &y_subcoord);
      offset = lp_build_add(&bld->int_coord_bld, offset, y_offset);
      if (dims >= 3) {
         LLVMValueRef z_offset;
         lp_build_sample_wrap_nearest_int(bld,
                                          1, /* block length (depth) */
                                          r_ipart, r_float,
                                          depth_vec, img_stride_vec, offsets[2],
                                          bld->static_texture_state->pot_depth,
                                          bld->static_sampler_state->wrap_r,
                                          &z_offset, &z_subcoord);
         offset = lp_build_add(&bld->int_coord_bld, offset, z_offset);
      }
   }
   if (has_layer_coord(bld->static_texture_state->target)) {
      LLVMValueRef z_offset;
      /* The r coord is the cube face in [0,5] or array layer */
      z_offset = lp_build_mul(&bld->int_coord_bld, r, img_stride_vec);
      offset = lp_build_add(&bld->int_coord_bld, offset, z_offset);
   }
   if (mipoffsets) {
      offset = lp_build_add(&bld->int_coord_bld, offset, mipoffsets);
   }

   lp_build_sample_fetch_image_nearest(bld, data_ptr, offset,
                                       x_subcoord, y_subcoord,
                                       colors);
}


/**
 * Fetch texels for image with linear sampling.
 * Return filtered color as two vectors of 16-bit fixed point values.
 */
static void
lp_build_sample_fetch_image_linear(struct lp_build_sample_context *bld,
                                   LLVMValueRef data_ptr,
                                   LLVMValueRef offset[2][2][2],
                                   LLVMValueRef x_subcoord[2],
                                   LLVMValueRef y_subcoord[2],
                                   LLVMValueRef s_fpart,
                                   LLVMValueRef t_fpart,
                                   LLVMValueRef r_fpart,
                                   LLVMValueRef *colors)
{
   const unsigned dims = bld->dims;
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_build_context u8n;
   LLVMTypeRef u8n_vec_type;
   LLVMTypeRef elem_type = LLVMInt32TypeInContext(bld->gallivm->context);
   LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH];
   LLVMValueRef shuffle;
   LLVMValueRef neighbors[2][2][2]; /* [z][y][x] */
   LLVMValueRef packed;
   unsigned i, j, k;
   unsigned numj, numk;

   lp_build_context_init(&u8n, bld->gallivm, lp_type_unorm(8, bld->vector_width));
   u8n_vec_type = lp_build_vec_type(bld->gallivm, u8n.type);

   /*
    * Transform 4 x i32 in
    *
    *   s_fpart = {s0, s1, s2, s3}
    *
    * where each value is between 0 and 0xff,
    *
    * into one 16 x i20
    *
    *   s_fpart = {s0, s0, s0, s0, s1, s1, s1, s1, s2, s2, s2, s2, s3, s3, s3, s3}
    *
    * and likewise for t_fpart. There is no risk of loosing precision here
    * since the fractional parts only use the lower 8bits.
    */
   s_fpart = LLVMBuildBitCast(builder, s_fpart, u8n_vec_type, "");
   if (dims >= 2)
      t_fpart = LLVMBuildBitCast(builder, t_fpart, u8n_vec_type, "");
   if (dims >= 3)
      r_fpart = LLVMBuildBitCast(builder, r_fpart, u8n_vec_type, "");

   for (j = 0; j < u8n.type.length; j += 4) {
#if UTIL_ARCH_LITTLE_ENDIAN
      unsigned subindex = 0;
#else
      unsigned subindex = 3;
#endif
      LLVMValueRef index;

      index = LLVMConstInt(elem_type, j + subindex, 0);
      for (i = 0; i < 4; ++i)
         shuffles[j + i] = index;
   }

   shuffle = LLVMConstVector(shuffles, u8n.type.length);

   s_fpart = LLVMBuildShuffleVector(builder, s_fpart, u8n.undef,
                                    shuffle, "");
   if (dims >= 2) {
      t_fpart = LLVMBuildShuffleVector(builder, t_fpart, u8n.undef,
                                       shuffle, "");
   }
   if (dims >= 3) {
      r_fpart = LLVMBuildShuffleVector(builder, r_fpart, u8n.undef,
                                       shuffle, "");
   }

   /*
    * Fetch the pixels as 4 x 32bit (rgba order might differ):
    *
    *   rgba0 rgba1 rgba2 rgba3
    *
    * bit cast them into 16 x u8
    *
    *   r0 g0 b0 a0 r1 g1 b1 a1 r2 g2 b2 a2 r3 g3 b3 a3
    *
    * unpack them into two 8 x i16:
    *
    *   r0 g0 b0 a0 r1 g1 b1 a1
    *   r2 g2 b2 a2 r3 g3 b3 a3
    *
    * The higher 8 bits of the resulting elements will be zero.
    */
   numj = 1 + (dims >= 2);
   numk = 1 + (dims >= 3);

   for (k = 0; k < numk; k++) {
      for (j = 0; j < numj; j++) {
         for (i = 0; i < 2; i++) {
            LLVMValueRef rgba8;

            if (util_format_is_rgba8_variant(bld->format_desc)) {
               struct lp_type fetch_type;
               /*
                * Given the format is a rgba8, just read the pixels as is,
                * without any swizzling. Swizzling will be done later.
                */
               fetch_type = lp_type_uint(bld->texel_type.width);
               rgba8 = lp_build_gather(bld->gallivm,
                                       bld->texel_type.length,
                                       bld->format_desc->block.bits,
                                       fetch_type,
                                       true,
                                       data_ptr, offset[k][j][i], true);

               rgba8 = LLVMBuildBitCast(builder, rgba8, u8n_vec_type, "");
            }
            else {
               rgba8 = lp_build_fetch_rgba_aos(bld->gallivm,
                                               bld->format_desc,
                                               u8n.type,
                                               true,
                                               data_ptr, offset[k][j][i],
                                               x_subcoord[i],
                                               y_subcoord[j],
                                               bld->cache);
            }

            neighbors[k][j][i] = rgba8;
         }
      }
   }

   /*
    * Linear interpolation with 8.8 fixed point.
    */

   /* general 1/2/3-D lerping */
   if (dims == 1) {
      lp_build_reduce_filter(&u8n,
                              bld->static_sampler_state->reduction_mode,
                              LP_BLD_LERP_PRESCALED_WEIGHTS,
                              1,
                              s_fpart,
                              &neighbors[0][0][0],
                              &neighbors[0][0][1],
                              &packed);
   } else if (dims == 2) {
      /* 2-D lerp */
      lp_build_reduce_filter_2d(&u8n,
                                 bld->static_sampler_state->reduction_mode,
                                 LP_BLD_LERP_PRESCALED_WEIGHTS,
                                 1,
                                 s_fpart, t_fpart,
                                 &neighbors[0][0][0],
                                 &neighbors[0][0][1],
                                 &neighbors[0][1][0],
                                 &neighbors[0][1][1],
                                 &packed);
   } else {
      /* 3-D lerp */
      assert(dims == 3);
      lp_build_reduce_filter_3d(&u8n,
                                 bld->static_sampler_state->reduction_mode,
                                 LP_BLD_LERP_PRESCALED_WEIGHTS,
                                 1,
                                 s_fpart, t_fpart, r_fpart,
                                 &neighbors[0][0][0],
                                 &neighbors[0][0][1],
                                 &neighbors[0][1][0],
                                 &neighbors[0][1][1],
                                 &neighbors[1][0][0],
                                 &neighbors[1][0][1],
                                 &neighbors[1][1][0],
                                 &neighbors[1][1][1],
                                 &packed);
   }

   *colors = packed;
}

/**
 * Sample a single texture image with (bi-)(tri-)linear sampling.
 * Return filtered color as two vectors of 16-bit fixed point values.
 */
static void
lp_build_sample_image_linear(struct lp_build_sample_context *bld,
                             LLVMValueRef int_size,
                             LLVMValueRef row_stride_vec,
                             LLVMValueRef img_stride_vec,
                             LLVMValueRef data_ptr,
                             LLVMValueRef mipoffsets,
                             LLVMValueRef s,
                             LLVMValueRef t,
                             LLVMValueRef r,
                             const LLVMValueRef *offsets,
                             LLVMValueRef *colors)
{
   const unsigned dims = bld->dims;
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_build_context i32;
   LLVMValueRef i32_c8, i32_c128, i32_c255;
   LLVMValueRef width_vec, height_vec, depth_vec;
   LLVMValueRef s_ipart, s_fpart, s_float;
   LLVMValueRef t_ipart = NULL, t_fpart = NULL, t_float = NULL;
   LLVMValueRef r_ipart = NULL, r_fpart = NULL, r_float = NULL;
   LLVMValueRef x_stride, y_stride, z_stride;
   LLVMValueRef x_offset0, x_offset1;
   LLVMValueRef y_offset0, y_offset1;
   LLVMValueRef z_offset0, z_offset1;
   LLVMValueRef offset[2][2][2]; /* [z][y][x] */
   LLVMValueRef x_subcoord[2], y_subcoord[2] = {NULL, NULL}, z_subcoord[2];
   unsigned x, y, z;

   lp_build_context_init(&i32, bld->gallivm, lp_type_int_vec(32, bld->vector_width));

   lp_build_extract_image_sizes(bld,
                                &bld->int_size_bld,
                                bld->int_coord_type,
                                int_size,
                                &width_vec,
                                &height_vec,
                                &depth_vec);

   s_float = s; t_float = t; r_float = r;

   if (bld->static_sampler_state->normalized_coords) {
      LLVMValueRef scaled_size;
      LLVMValueRef flt_size;

      /* scale size by 256 (8 fractional bits) */
      scaled_size = lp_build_shl_imm(&bld->int_size_bld, int_size, 8);

      flt_size = lp_build_int_to_float(&bld->float_size_bld, scaled_size);

      lp_build_unnormalized_coords(bld, flt_size, &s, &t, &r);
   }
   else {
      /* scale coords by 256 (8 fractional bits) */
      s = lp_build_mul_imm(&bld->coord_bld, s, 256);
      if (dims >= 2)
         t = lp_build_mul_imm(&bld->coord_bld, t, 256);
      if (dims >= 3)
         r = lp_build_mul_imm(&bld->coord_bld, r, 256);
   }

   /* convert float to int */
   /* For correct rounding, need round to nearest, not truncation here.
    * Note that in some cases (clamp to edge, no texel offsets) we
    * could use a non-signed build context which would help archs which
    * don't have fptosi intrinsic with nearest rounding implemented.
    */
   s = lp_build_iround(&bld->coord_bld, s);
   if (dims >= 2)
      t = lp_build_iround(&bld->coord_bld, t);
   if (dims >= 3)
      r = lp_build_iround(&bld->coord_bld, r);

   /* subtract 0.5 (add -128) */
   i32_c128 = lp_build_const_int_vec(bld->gallivm, i32.type, -128);

   s = LLVMBuildAdd(builder, s, i32_c128, "");
   if (dims >= 2) {
      t = LLVMBuildAdd(builder, t, i32_c128, "");
   }
   if (dims >= 3) {
      r = LLVMBuildAdd(builder, r, i32_c128, "");
   }

   /* compute floor (shift right 8) */
   i32_c8 = lp_build_const_int_vec(bld->gallivm, i32.type, 8);
   s_ipart = LLVMBuildAShr(builder, s, i32_c8, "");
   if (dims >= 2)
      t_ipart = LLVMBuildAShr(builder, t, i32_c8, "");
   if (dims >= 3)
      r_ipart = LLVMBuildAShr(builder, r, i32_c8, "");

   /* add texel offsets */
   if (offsets[0]) {
      s_ipart = lp_build_add(&i32, s_ipart, offsets[0]);
      if (dims >= 2) {
         t_ipart = lp_build_add(&i32, t_ipart, offsets[1]);
         if (dims >= 3) {
            r_ipart = lp_build_add(&i32, r_ipart, offsets[2]);
         }
      }
   }

   /* compute fractional part (AND with 0xff) */
   i32_c255 = lp_build_const_int_vec(bld->gallivm, i32.type, 255);
   s_fpart = LLVMBuildAnd(builder, s, i32_c255, "");
   if (dims >= 2)
      t_fpart = LLVMBuildAnd(builder, t, i32_c255, "");
   if (dims >= 3)
      r_fpart = LLVMBuildAnd(builder, r, i32_c255, "");

   /* get pixel, row and image strides */
   x_stride = lp_build_const_vec(bld->gallivm, bld->int_coord_bld.type,
                                 bld->format_desc->block.bits/8);
   y_stride = row_stride_vec;
   z_stride = img_stride_vec;

   /* do texcoord wrapping and compute texel offsets */
   lp_build_sample_wrap_linear_int(bld,
                                   bld->format_desc->block.width,
                                   s_ipart, &s_fpart, s_float,
                                   width_vec, x_stride, offsets[0],
                                   bld->static_texture_state->pot_width,
                                   bld->static_sampler_state->wrap_s,
                                   &x_offset0, &x_offset1,
                                   &x_subcoord[0], &x_subcoord[1]);

   /* add potential cube/array/mip offsets now as they are constant per pixel */
   if (has_layer_coord(bld->static_texture_state->target)) {
      LLVMValueRef z_offset;
      z_offset = lp_build_mul(&bld->int_coord_bld, r, img_stride_vec);
      /* The r coord is the cube face in [0,5] or array layer */
      x_offset0 = lp_build_add(&bld->int_coord_bld, x_offset0, z_offset);
      x_offset1 = lp_build_add(&bld->int_coord_bld, x_offset1, z_offset);
   }
   if (mipoffsets) {
      x_offset0 = lp_build_add(&bld->int_coord_bld, x_offset0, mipoffsets);
      x_offset1 = lp_build_add(&bld->int_coord_bld, x_offset1, mipoffsets);
   }

   for (z = 0; z < 2; z++) {
      for (y = 0; y < 2; y++) {
         offset[z][y][0] = x_offset0;
         offset[z][y][1] = x_offset1;
      }
   }

   if (dims >= 2) {
      lp_build_sample_wrap_linear_int(bld,
                                      bld->format_desc->block.height,
                                      t_ipart, &t_fpart, t_float,
                                      height_vec, y_stride, offsets[1],
                                      bld->static_texture_state->pot_height,
                                      bld->static_sampler_state->wrap_t,
                                      &y_offset0, &y_offset1,
                                      &y_subcoord[0], &y_subcoord[1]);

      for (z = 0; z < 2; z++) {
         for (x = 0; x < 2; x++) {
            offset[z][0][x] = lp_build_add(&bld->int_coord_bld,
                                           offset[z][0][x], y_offset0);
            offset[z][1][x] = lp_build_add(&bld->int_coord_bld,
                                           offset[z][1][x], y_offset1);
         }
      }
   }

   if (dims >= 3) {
      lp_build_sample_wrap_linear_int(bld,
                                      1, /* block length (depth) */
                                      r_ipart, &r_fpart, r_float,
                                      depth_vec, z_stride, offsets[2],
                                      bld->static_texture_state->pot_depth,
                                      bld->static_sampler_state->wrap_r,
                                      &z_offset0, &z_offset1,
                                      &z_subcoord[0], &z_subcoord[1]);
      for (y = 0; y < 2; y++) {
         for (x = 0; x < 2; x++) {
            offset[0][y][x] = lp_build_add(&bld->int_coord_bld,
                                           offset[0][y][x], z_offset0);
            offset[1][y][x] = lp_build_add(&bld->int_coord_bld,
                                           offset[1][y][x], z_offset1);
         }
      }
   }

   lp_build_sample_fetch_image_linear(bld, data_ptr, offset,
                                      x_subcoord, y_subcoord,
                                      s_fpart, t_fpart, r_fpart,
                                      colors);
}


/**
 * Sample the texture/mipmap using given image filter and mip filter.
 * data0_ptr and data1_ptr point to the two mipmap levels to sample
 * from.  width0/1_vec, height0/1_vec, depth0/1_vec indicate their sizes.
 * If we're using nearest miplevel sampling the '1' values will be null/unused.
 */
static void
lp_build_sample_mipmap(struct lp_build_sample_context *bld,
                       unsigned img_filter,
                       unsigned mip_filter,
                       LLVMValueRef s,
                       LLVMValueRef t,
                       LLVMValueRef r,
                       const LLVMValueRef *offsets,
                       LLVMValueRef ilevel0,
                       LLVMValueRef ilevel1,
                       LLVMValueRef lod_fpart,
                       LLVMValueRef colors_var)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef size0;
   LLVMValueRef size1;
   LLVMValueRef row_stride0_vec = NULL;
   LLVMValueRef row_stride1_vec = NULL;
   LLVMValueRef img_stride0_vec = NULL;
   LLVMValueRef img_stride1_vec = NULL;
   LLVMValueRef data_ptr0;
   LLVMValueRef data_ptr1;
   LLVMValueRef mipoff0 = NULL;
   LLVMValueRef mipoff1 = NULL;
   LLVMValueRef colors0;
   LLVMValueRef colors1;

   /* sample the first mipmap level */
   lp_build_mipmap_level_sizes(bld, ilevel0,
                               &size0,
                               &row_stride0_vec, &img_stride0_vec);
   if (bld->num_mips == 1) {
      data_ptr0 = lp_build_get_mipmap_level(bld, ilevel0);
   }
   else {
      /* This path should work for num_lods 1 too but slightly less efficient */
      data_ptr0 = bld->base_ptr;
      mipoff0 = lp_build_get_mip_offsets(bld, ilevel0);
   }

   if (img_filter == PIPE_TEX_FILTER_NEAREST) {
      lp_build_sample_image_nearest(bld,
                                    size0,
                                    row_stride0_vec, img_stride0_vec,
                                    data_ptr0, mipoff0, s, t, r, offsets,
                                    &colors0);
   }
   else {
      assert(img_filter == PIPE_TEX_FILTER_LINEAR);
      lp_build_sample_image_linear(bld,
                                   size0,
                                   row_stride0_vec, img_stride0_vec,
                                   data_ptr0, mipoff0, s, t, r, offsets,
                                   &colors0);
   }

   /* Store the first level's colors in the output variables */
   LLVMBuildStore(builder, colors0, colors_var);

   if (mip_filter == PIPE_TEX_MIPFILTER_LINEAR) {
      LLVMValueRef h16vec_scale = lp_build_const_vec(bld->gallivm,
                                                     bld->lodf_bld.type, 256.0);
      LLVMTypeRef i32vec_type = bld->lodi_bld.vec_type;
      struct lp_build_if_state if_ctx;
      LLVMValueRef need_lerp;
      unsigned num_quads = bld->coord_bld.type.length / 4;
      unsigned i;

      lod_fpart = LLVMBuildFMul(builder, lod_fpart, h16vec_scale, "");
      lod_fpart = LLVMBuildFPToSI(builder, lod_fpart, i32vec_type, "lod_fpart.fixed16");

      /* need_lerp = lod_fpart > 0 */
      if (bld->num_lods == 1) {
         need_lerp = LLVMBuildICmp(builder, LLVMIntSGT,
                                   lod_fpart, bld->lodi_bld.zero,
                                   "need_lerp");
      }
      else {
         /*
          * We'll do mip filtering if any of the quads need it.
          * It might be better to split the vectors here and only fetch/filter
          * quads which need it.
          */
         /*
          * We need to clamp lod_fpart here since we can get negative
          * values which would screw up filtering if not all
          * lod_fpart values have same sign.
          * We can however then skip the greater than comparison.
          */
         lod_fpart = lp_build_max(&bld->lodi_bld, lod_fpart,
                                  bld->lodi_bld.zero);
         need_lerp = lp_build_any_true_range(&bld->lodi_bld, bld->num_lods, lod_fpart);
      }

      lp_build_if(&if_ctx, bld->gallivm, need_lerp);
      {
         struct lp_build_context u8n_bld;

         lp_build_context_init(&u8n_bld, bld->gallivm, lp_type_unorm(8, bld->vector_width));

         /* sample the second mipmap level */
         lp_build_mipmap_level_sizes(bld, ilevel1,
                                     &size1,
                                     &row_stride1_vec, &img_stride1_vec);
         if (bld->num_mips == 1) {
            data_ptr1 = lp_build_get_mipmap_level(bld, ilevel1);
         }
         else {
            data_ptr1 = bld->base_ptr;
            mipoff1 = lp_build_get_mip_offsets(bld, ilevel1);
         }

         if (img_filter == PIPE_TEX_FILTER_NEAREST) {
            lp_build_sample_image_nearest(bld,
                                          size1,
                                          row_stride1_vec, img_stride1_vec,
                                          data_ptr1, mipoff1, s, t, r, offsets,
                                          &colors1);
         }
         else {
            lp_build_sample_image_linear(bld,
                                         size1,
                                         row_stride1_vec, img_stride1_vec,
                                         data_ptr1, mipoff1, s, t, r, offsets,
                                         &colors1);
         }

         /* interpolate samples from the two mipmap levels */

         if (num_quads == 1 && bld->num_lods == 1) {
            lod_fpart = LLVMBuildTrunc(builder, lod_fpart, u8n_bld.elem_type, "");
            lod_fpart = lp_build_broadcast_scalar(&u8n_bld, lod_fpart);
         }
         else {
            unsigned num_chans_per_lod = 4 * bld->coord_type.length / bld->num_lods;
            LLVMTypeRef tmp_vec_type = LLVMVectorType(u8n_bld.elem_type, bld->lodi_bld.type.length);
            LLVMValueRef shuffle[LP_MAX_VECTOR_LENGTH];

            /* Take the LSB of lod_fpart */
            lod_fpart = LLVMBuildTrunc(builder, lod_fpart, tmp_vec_type, "");

            /* Broadcast each lod weight into their respective channels */
            for (i = 0; i < u8n_bld.type.length; ++i) {
               shuffle[i] = lp_build_const_int32(bld->gallivm, i / num_chans_per_lod);
            }
            lod_fpart = LLVMBuildShuffleVector(builder, lod_fpart, LLVMGetUndef(tmp_vec_type),
                                               LLVMConstVector(shuffle, u8n_bld.type.length), "");
         }

         lp_build_reduce_filter(&u8n_bld,
                                bld->static_sampler_state->reduction_mode,
                                LP_BLD_LERP_PRESCALED_WEIGHTS,
                                1,
                                lod_fpart,
                                &colors0,
                                &colors1,
                                &colors0);

         LLVMBuildStore(builder, colors0, colors_var);
      }
      lp_build_endif(&if_ctx);
   }
}



/**
 * Texture sampling in AoS format.  Used when sampling common 32-bit/texel
 * formats.  1D/2D/3D/cube texture supported.  All mipmap sampling modes
 * but only limited texture coord wrap modes.
 */
void
lp_build_sample_aos(struct lp_build_sample_context *bld,
                    LLVMValueRef s,
                    LLVMValueRef t,
                    LLVMValueRef r,
                    const LLVMValueRef *offsets,
                    LLVMValueRef lod_positive,
                    LLVMValueRef lod_fpart,
                    LLVMValueRef ilevel0,
                    LLVMValueRef ilevel1,
                    LLVMValueRef texel_out[4])
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const unsigned mip_filter = bld->static_sampler_state->min_mip_filter;
   const unsigned min_filter = bld->static_sampler_state->min_img_filter;
   const unsigned mag_filter = bld->static_sampler_state->mag_img_filter;
   const unsigned dims = bld->dims;
   LLVMValueRef packed_var, packed;
   LLVMValueRef unswizzled[4];
   struct lp_build_context u8n_bld;

   /* we only support the common/simple wrap modes at this time */
   assert(lp_is_simple_wrap_mode(bld->static_sampler_state->wrap_s));
   if (dims >= 2)
      assert(lp_is_simple_wrap_mode(bld->static_sampler_state->wrap_t));
   if (dims >= 3)
      assert(lp_is_simple_wrap_mode(bld->static_sampler_state->wrap_r));


   /* make 8-bit unorm builder context */
   lp_build_context_init(&u8n_bld, bld->gallivm, lp_type_unorm(8, bld->vector_width));

   /*
    * Get/interpolate texture colors.
    */

   packed_var = lp_build_alloca(bld->gallivm, u8n_bld.vec_type, "packed_var");

   if (min_filter == mag_filter) {
      /* no need to distinguish between minification and magnification */
      lp_build_sample_mipmap(bld,
                             min_filter, mip_filter,
                             s, t, r, offsets,
                             ilevel0, ilevel1, lod_fpart,
                             packed_var);
   }
   else {
      /* Emit conditional to choose min image filter or mag image filter
       * depending on the lod being > 0 or <= 0, respectively.
       */
      struct lp_build_if_state if_ctx;

      /*
       * FIXME this should take all lods into account, if some are min
       * some max probably could hack up the weights in the linear
       * path with selects to work for nearest.
       */
      if (bld->num_lods > 1)
         lod_positive = LLVMBuildExtractElement(builder, lod_positive,
                                                lp_build_const_int32(bld->gallivm, 0), "");

      lod_positive = LLVMBuildTrunc(builder, lod_positive,
                                    LLVMInt1TypeInContext(bld->gallivm->context), "");

      lp_build_if(&if_ctx, bld->gallivm, lod_positive);
      {
         /* Use the minification filter */
         lp_build_sample_mipmap(bld,
                                min_filter, mip_filter,
                                s, t, r, offsets,
                                ilevel0, ilevel1, lod_fpart,
                                packed_var);
      }
      lp_build_else(&if_ctx);
      {
         /* Use the magnification filter */
         lp_build_sample_mipmap(bld, 
                                mag_filter, PIPE_TEX_MIPFILTER_NONE,
                                s, t, r, offsets,
                                ilevel0, NULL, NULL,
                                packed_var);
      }
      lp_build_endif(&if_ctx);
   }

   packed = LLVMBuildLoad2(builder, u8n_bld.vec_type, packed_var, "");

   /*
    * Convert to SoA and swizzle.
    */
   lp_build_rgba8_to_fi32_soa(bld->gallivm,
                             bld->texel_type,
                             packed, unswizzled);

   if (util_format_is_rgba8_variant(bld->format_desc)) {
      lp_build_format_swizzle_soa(bld->format_desc,
                                  &bld->texel_bld,
                                  unswizzled, texel_out);
   }
   else {
      texel_out[0] = unswizzled[0];
      texel_out[1] = unswizzled[1];
      texel_out[2] = unswizzled[2];
      texel_out[3] = unswizzled[3];
   }
}
