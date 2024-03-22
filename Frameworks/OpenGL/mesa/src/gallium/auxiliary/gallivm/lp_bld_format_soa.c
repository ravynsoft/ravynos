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


#include "pipe/p_defines.h"

#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_string.h"
#include "util/u_math.h"

#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_conv.h"
#include "lp_bld_swizzle.h"
#include "lp_bld_gather.h"
#include "lp_bld_debug.h"
#include "lp_bld_format.h"
#include "lp_bld_arit.h"
#include "lp_bld_pack.h"
#include "lp_bld_flow.h"
#include "lp_bld_printf.h"
#include "lp_bld_intr.h"

static void
convert_to_soa(struct gallivm_state *gallivm,
               LLVMValueRef src_aos[LP_MAX_VECTOR_WIDTH / 32],
               LLVMValueRef dst_soa[4],
               const struct lp_type soa_type)
{
   unsigned j, k;
   struct lp_type aos_channel_type = soa_type;

   LLVMValueRef aos_channels[4];
   unsigned pixels_per_channel = soa_type.length / 4;

   assert((soa_type.length % 4) == 0);

   aos_channel_type.length >>= 1;

   for (j = 0; j < 4; ++j) {
      LLVMValueRef channel[LP_MAX_VECTOR_LENGTH] = { 0 };

      assert(pixels_per_channel <= LP_MAX_VECTOR_LENGTH);

      for (k = 0; k < pixels_per_channel; ++k) {
         channel[k] = src_aos[j + 4 * k];
      }

      aos_channels[j] = lp_build_concat(gallivm, channel, aos_channel_type, pixels_per_channel);
   }

   lp_build_transpose_aos(gallivm, soa_type, aos_channels, dst_soa);
}


void
lp_build_format_swizzle_soa(const struct util_format_description *format_desc,
                            struct lp_build_context *bld,
                            const LLVMValueRef unswizzled[4],
                            LLVMValueRef swizzled_out[4])
{
   if (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS) {
      enum pipe_swizzle swizzle;
      LLVMValueRef depth_or_stencil;

      if (util_format_has_stencil(format_desc) &&
          !util_format_has_depth(format_desc)) {
         assert(!bld->type.floating);
         swizzle = format_desc->swizzle[1];
      }
      else {
         assert(bld->type.floating);
         swizzle = format_desc->swizzle[0];
      }
      /*
       * Return zzz1 or sss1 for depth-stencil formats here.
       * Correct swizzling will be handled by apply_sampler_swizzle() later.
       */
      depth_or_stencil = lp_build_swizzle_soa_channel(bld, unswizzled, swizzle);

      swizzled_out[2] = swizzled_out[1] = swizzled_out[0] = depth_or_stencil;
      swizzled_out[3] = bld->one;
   }
   else {
      unsigned chan;
      for (chan = 0; chan < 4; ++chan) {
         enum pipe_swizzle swizzle = format_desc->swizzle[chan];
         swizzled_out[chan] = lp_build_swizzle_soa_channel(bld, unswizzled, swizzle);
      }
   }
}



static LLVMValueRef
lp_build_extract_soa_chan(struct lp_build_context *bld,
                          unsigned blockbits,
                          bool srgb_chan,
                          struct util_format_channel_description chan_desc,
                          LLVMValueRef packed)
{
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type type = bld->type;
   LLVMValueRef input = packed;
   const unsigned width = chan_desc.size;
   const unsigned start = chan_desc.shift;
   const unsigned stop = start + width;

   /* Decode the input vector component */

   switch(chan_desc.type) {
   case UTIL_FORMAT_TYPE_VOID:
      input = bld->undef;
      break;

   case UTIL_FORMAT_TYPE_UNSIGNED:
      /*
       * Align the LSB
       */
      if (start) {
         input = LLVMBuildLShr(builder, input,
                               lp_build_const_int_vec(gallivm, type, start), "");
      }

      /*
       * Zero the MSBs
       */
      if (stop < blockbits) {
         unsigned mask = ((unsigned long long)1 << width) - 1;
         input = LLVMBuildAnd(builder, input,
                              lp_build_const_int_vec(gallivm, type, mask), "");
      }

      /*
       * Type conversion
       */
      if (type.floating) {
         if (srgb_chan) {
            struct lp_type conv_type = lp_uint_type(type);
            input = lp_build_srgb_to_linear(gallivm, conv_type, width, input);
         }
         else {
            if(chan_desc.normalized)
               input = lp_build_unsigned_norm_to_float(gallivm, width, type, input);
            else
               input = LLVMBuildUIToFP(builder, input, bld->vec_type, "");
         }
      }
      else if (chan_desc.pure_integer) {
         /* Nothing to do */
      } else {
          /* FIXME */
          assert(0);
      }
      break;

   case UTIL_FORMAT_TYPE_SIGNED:
      /*
       * Align the sign bit first.
       */
      if (stop < type.width) {
         unsigned bits = type.width - stop;
         LLVMValueRef bits_val = lp_build_const_int_vec(gallivm, type, bits);
         input = LLVMBuildShl(builder, input, bits_val, "");
      }

      /*
       * Align the LSB (with an arithmetic shift to preserve the sign)
       */
      if (chan_desc.size < type.width) {
         unsigned bits = type.width - chan_desc.size;
         LLVMValueRef bits_val = lp_build_const_int_vec(gallivm, type, bits);
         input = LLVMBuildAShr(builder, input, bits_val, "");
      }

      /*
       * Type conversion
       */
      if (type.floating) {
         input = LLVMBuildSIToFP(builder, input, bld->vec_type, "");
         if (chan_desc.normalized) {
            double scale = 1.0 / ((1 << (chan_desc.size - 1)) - 1);
            LLVMValueRef scale_val = lp_build_const_vec(gallivm, type, scale);
            input = LLVMBuildFMul(builder, input, scale_val, "");
            /*
             * The formula above will produce value below -1.0 for most negative values.
             * compliance requires clamping it.
             * GTF-GL45.gtf33.GL3Tests.vertex_type_2_10_10_10_rev.vertex_type_2_10_10_10_rev_conversion.
             */
            input = lp_build_max(bld, input,
                                 lp_build_const_vec(gallivm, type, -1.0f));
         }
      }
      else if (chan_desc.pure_integer) {
         /* Nothing to do */
      } else {
          /* FIXME */
          assert(0);
      }
      break;

   case UTIL_FORMAT_TYPE_FLOAT:
      if (type.floating) {
         if (chan_desc.size == 16) {
            struct lp_type f16i_type = type;
            f16i_type.width /= 2;
            f16i_type.floating = 0;
            if (start) {
               input = LLVMBuildLShr(builder, input,
                                     lp_build_const_int_vec(gallivm, type, start), "");
            }
            input = LLVMBuildTrunc(builder, input,
                                   lp_build_vec_type(gallivm, f16i_type), "");
            input = lp_build_half_to_float(gallivm, input);
         } else {
            assert(start == 0);
            assert(stop == 32);
            assert(type.width == 32);
         }
         input = LLVMBuildBitCast(builder, input, bld->vec_type, "");
      }
      else {
         /* FIXME */
         assert(0);
         input = bld->undef;
      }
      break;

   case UTIL_FORMAT_TYPE_FIXED:
      if (type.floating) {
         double scale = 1.0 / ((1 << (chan_desc.size/2)) - 1);
         LLVMValueRef scale_val = lp_build_const_vec(gallivm, type, scale);
         input = LLVMBuildSIToFP(builder, input, bld->vec_type, "");
         input = LLVMBuildFMul(builder, input, scale_val, "");
      }
      else {
         /* FIXME */
         assert(0);
         input = bld->undef;
      }
      break;

   default:
      assert(0);
      input = bld->undef;
      break;
   }

   return input;
}


/**
 * Unpack several pixels in SoA.
 *
 * It takes a vector of packed pixels:
 *
 *   packed = {P0, P1, P2, P3, ..., Pn}
 *
 * And will produce four vectors:
 *
 *   red    = {R0, R1, R2, R3, ..., Rn}
 *   green  = {G0, G1, G2, G3, ..., Gn}
 *   blue   = {B0, B1, B2, B3, ..., Bn}
 *   alpha  = {A0, A1, A2, A3, ..., An}
 *
 * It requires that a packed pixel fits into an element of the output
 * channels. The common case is when converting pixel with a depth of 32 bit or
 * less into floats.
 *
 * \param format_desc  the format of the 'packed' incoming pixel vector
 * \param type  the desired type for rgba_out (type.length = n, above)
 * \param packed  the incoming vector of packed pixels
 * \param rgba_out  returns the SoA R,G,B,A vectors
 */
void
lp_build_unpack_rgba_soa(struct gallivm_state *gallivm,
                         const struct util_format_description *format_desc,
                         struct lp_type type,
                         LLVMValueRef packed,
                         LLVMValueRef rgba_out[4])
{
   struct lp_build_context bld;
   LLVMValueRef inputs[4];
   unsigned chan;

   assert(format_desc->layout == UTIL_FORMAT_LAYOUT_PLAIN);
   assert(format_desc->block.width == 1);
   assert(format_desc->block.height == 1);
   assert(format_desc->block.bits <= type.width);
   /* FIXME: Support more output types */
   assert(type.width == 32);

   lp_build_context_init(&bld, gallivm, type);

   /* Decode the input vector components */
   for (chan = 0; chan < format_desc->nr_channels; ++chan) {
      struct util_format_channel_description chan_desc = format_desc->channel[chan];
      bool srgb_chan = false;

      if (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB &&
          format_desc->swizzle[3] != chan) {
         srgb_chan = true;
      }

      inputs[chan] = lp_build_extract_soa_chan(&bld,
                                               format_desc->block.bits,
                                               srgb_chan,
                                               chan_desc,
                                               packed);
   }

   lp_build_format_swizzle_soa(format_desc, &bld, inputs, rgba_out);
}


/**
 * Convert a vector of rgba8 values into 32bit wide SoA vectors.
 *
 * \param dst_type  The desired return type. For pure integer formats
 *                  this should be a 32bit wide int or uint vector type,
 *                  otherwise a float vector type.
 *
 * \param packed    The rgba8 values to pack.
 *
 * \param rgba      The 4 SoA return vectors.
 */
void
lp_build_rgba8_to_fi32_soa(struct gallivm_state *gallivm,
                           struct lp_type dst_type,
                           LLVMValueRef packed,
                           LLVMValueRef *rgba)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef mask = lp_build_const_int_vec(gallivm, dst_type, 0xff);
   unsigned chan;

   /* XXX technically shouldn't use that for uint dst_type */
   packed = LLVMBuildBitCast(builder, packed,
                             lp_build_int_vec_type(gallivm, dst_type), "");

   /* Decode the input vector components */
   for (chan = 0; chan < 4; ++chan) {
#if UTIL_ARCH_LITTLE_ENDIAN
      unsigned start = chan*8;
#else
      unsigned start = (3-chan)*8;
#endif
      unsigned stop = start + 8;
      LLVMValueRef input;

      input = packed;

      if (start)
         input = LLVMBuildLShr(builder, input,
                               lp_build_const_int_vec(gallivm, dst_type, start), "");

      if (stop < 32)
         input = LLVMBuildAnd(builder, input, mask, "");

      if (dst_type.floating)
         input = lp_build_unsigned_norm_to_float(gallivm, 8, dst_type, input);

      rgba[chan] = input;
   }
}



/**
 * Fetch a texels from a texture, returning them in SoA layout.
 *
 * \param type  the desired return type for 'rgba'.  The vector length
 *              is the number of texels to fetch
 * \param aligned if the offset is guaranteed to be aligned to element width
 *
 * \param base_ptr  points to the base of the texture mip tree.
 * \param offset    offset to start of the texture image block.  For non-
 *                  compressed formats, this simply is an offset to the texel.
 *                  For compressed formats, it is an offset to the start of the
 *                  compressed data block.
 *
 * \param i, j  the sub-block pixel coordinates.  For non-compressed formats
 *              these will always be (0,0).  For compressed formats, i will
 *              be in [0, block_width-1] and j will be in [0, block_height-1].
 * \param cache  optional value pointing to a lp_build_format_cache structure
 */
void
lp_build_fetch_rgba_soa(struct gallivm_state *gallivm,
                        const struct util_format_description *format_desc,
                        struct lp_type type,
                        bool aligned,
                        LLVMValueRef base_ptr,
                        LLVMValueRef offset,
                        LLVMValueRef i,
                        LLVMValueRef j,
                        LLVMValueRef cache,
                        LLVMValueRef rgba_out[4])
{
   LLVMBuilderRef builder = gallivm->builder;
   enum pipe_format format = format_desc->format;
   struct lp_type fetch_type;

   if (format_desc->layout == UTIL_FORMAT_LAYOUT_PLAIN &&
       (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB ||
        format_desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB ||
        format_desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS) &&
       format_desc->block.width == 1 &&
       format_desc->block.height == 1 &&
       format_desc->block.bits <= type.width &&
       (format_desc->channel[0].type != UTIL_FORMAT_TYPE_FLOAT ||
        format_desc->channel[0].size == 32 ||
        format_desc->channel[0].size == 16))
   {
      /*
       * The packed pixel fits into an element of the destination format. Put
       * the packed pixels into a vector and extract each component for all
       * vector elements in parallel.
       */

      LLVMValueRef packed;

      /*
       * gather the texels from the texture
       * Ex: packed = {XYZW, XYZW, XYZW, XYZW}
       */
      assert(format_desc->block.bits <= type.width);
      fetch_type = lp_type_uint(type.width);
      packed = lp_build_gather(gallivm,
                               type.length,
                               format_desc->block.bits,
                               fetch_type,
                               aligned,
                               base_ptr, offset, false);

      /*
       * convert texels to float rgba
       */
      lp_build_unpack_rgba_soa(gallivm,
                               format_desc,
                               type,
                               packed, rgba_out);
      return;
   }


   if (format_desc->layout == UTIL_FORMAT_LAYOUT_PLAIN &&
       (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB) &&
       format_desc->block.width == 1 &&
       format_desc->block.height == 1 &&
       format_desc->block.bits > type.width &&
       ((format_desc->block.bits <= type.width * type.length &&
         format_desc->channel[0].size <= type.width) ||
        (format_desc->channel[0].size == 64 &&
         format_desc->channel[0].type == UTIL_FORMAT_TYPE_FLOAT &&
         type.floating)))
   {
      /*
       * Similar to above, but the packed pixel is larger than what fits
       * into an element of the destination format. The packed pixels will be
       * shuffled into SoA vectors appropriately, and then the extraction will
       * be done in parallel as much as possible.
       * Good for 16xn (n > 2) and 32xn (n > 1) formats, care is taken so
       * the gathered vectors can be shuffled easily (even with avx).
       * 64xn float -> 32xn float is handled too but it's a bit special as
       * it does the conversion pre-shuffle.
       */

      LLVMValueRef packed[4], dst[4], output[4], shuffles[LP_MAX_VECTOR_WIDTH/32];
      struct lp_type fetch_type, gather_type = type;
      unsigned num_gather, fetch_width, i, j;
      struct lp_build_context bld;
      bool fp64 = format_desc->channel[0].size == 64;

      lp_build_context_init(&bld, gallivm, type);

      assert(type.width == 32);
      assert(format_desc->block.bits > type.width);

      /*
       * First, figure out fetch order.
       */
      fetch_width = util_next_power_of_two(format_desc->block.bits);
      /*
       * fp64 are treated like fp32 except we fetch twice wide values
       * (as we shuffle after trunc). The shuffles for that work out
       * mostly fine (slightly suboptimal for 4-wide, perfect for AVX)
       * albeit we miss the potential opportunity for hw gather (as it
       * only handles native size).
       */
      num_gather = fetch_width / type.width;
      gather_type.width *= num_gather;
      if (fp64) {
         num_gather /= 2;
      }
      gather_type.length /= num_gather;

      for (i = 0; i < num_gather; i++) {
         LLVMValueRef offsetr, shuf_vec;
         if(num_gather == 4) {
            for (j = 0; j < gather_type.length; j++) {
               unsigned idx = i + 4*j;
               shuffles[j] = lp_build_const_int32(gallivm, idx);
            }
            shuf_vec = LLVMConstVector(shuffles, gather_type.length);
            offsetr = LLVMBuildShuffleVector(builder, offset, offset, shuf_vec, "");

         }
         else if (num_gather == 2) {
            assert(num_gather == 2);
            for (j = 0; j < gather_type.length; j++) {
               unsigned idx = i*2 + (j%2) + (j/2)*4;
               shuffles[j] = lp_build_const_int32(gallivm, idx);
            }
            shuf_vec = LLVMConstVector(shuffles, gather_type.length);
            offsetr = LLVMBuildShuffleVector(builder, offset, offset, shuf_vec, "");
         }
         else {
            assert(num_gather == 1);
            offsetr = offset;
         }
         if (gather_type.length == 1) {
            LLVMValueRef zero = lp_build_const_int32(gallivm, 0);
            offsetr = LLVMBuildExtractElement(builder, offsetr, zero, "");
         }

         /*
          * Determine whether to use float or int loads. This is mostly
          * to outsmart the (stupid) llvm int/float shuffle logic, we
          * don't really care much if the data is floats or ints...
          * But llvm will refuse to use single float shuffle with int data
          * and instead use 3 int shuffles instead, the code looks atrocious.
          * (Note bitcasts often won't help, as llvm is too smart to be
          * fooled by that.)
          * Nobody cares about simd float<->int domain transition penalties,
          * which usually don't even exist for shuffles anyway.
          * With 4x32bit (and 3x32bit) fetch, we use float vec (the data is
          * going into transpose, which is unpacks, so doesn't really matter
          * much).
          * With 2x32bit or 4x16bit fetch, we use float vec, since those
          * go into the weird channel separation shuffle. With floats,
          * this is (with 128bit vectors):
          * - 2 movq, 2 movhpd, 2 shufps
          * With ints it would be:
          * - 4 movq, 2 punpcklqdq, 4 pshufd, 2 blendw
          * I've seen texture functions increase in code size by 15% just due
          * to that (there's lots of such fetches in them...)
          * (We could chose a different gather order to improve this somewhat
          * for the int path, but it would basically just drop the blends,
          * so the float path with this order really is optimal.)
          * Albeit it is tricky sometimes llvm doesn't ignore the float->int
          * casts so must avoid them until we're done with the float shuffle...
          * 3x16bit formats (the same is also true for 3x8) are pretty bad but
          * there's nothing we can do about them (we could overallocate by
          * those couple bytes and use unaligned but pot sized load).
          * Note that this is very much x86 specific. I don't know if this
          * affect other archs at all.
          */
         if (num_gather > 1) {
            /*
             * We always want some float type here (with x86)
             * due to shuffles being float ones afterwards (albeit for
             * the num_gather == 4 case int should work fine too
             * (unless there's some problems with avx but not avx2).
             */
            if (format_desc->channel[0].size == 64) {
               fetch_type = lp_type_float_vec(64, gather_type.width);
            } else {
               fetch_type = lp_type_int_vec(32, gather_type.width);
            }
         }
         else {
            /* type doesn't matter much */
            if (format_desc->channel[0].type == UTIL_FORMAT_TYPE_FLOAT &&
                (format_desc->channel[0].size == 32 ||
                 format_desc->channel[0].size == 64)) {
            fetch_type = lp_type_float(gather_type.width);
            } else {
               fetch_type = lp_type_uint(gather_type.width);
            }
         }

         /* Now finally gather the values */
         packed[i] = lp_build_gather(gallivm, gather_type.length,
                                     format_desc->block.bits,
                                     fetch_type, aligned,
                                     base_ptr, offsetr, false);
         if (fp64) {
            struct lp_type conv_type = type;
            conv_type.width *= 2;
            packed[i] = LLVMBuildBitCast(builder, packed[i],
                                         lp_build_vec_type(gallivm, conv_type), "");
            packed[i] = LLVMBuildFPTrunc(builder, packed[i], bld.vec_type, "");
         }
      }

      /* shuffle the gathered values to SoA */
      if (num_gather == 2) {
         for (i = 0; i < num_gather; i++) {
            for (j = 0; j < type.length; j++) {
               unsigned idx = (j%2)*2 + (j/4)*4 + i;
               if ((j/2)%2)
                  idx += type.length;
               shuffles[j] = lp_build_const_int32(gallivm, idx);
            }
            dst[i] = LLVMBuildShuffleVector(builder, packed[0], packed[1],
                                            LLVMConstVector(shuffles, type.length), "");
         }
      }
      else if (num_gather == 4) {
         lp_build_transpose_aos(gallivm, lp_int_type(type), packed, dst);
      }
      else {
         assert(num_gather == 1);
         dst[0] = packed[0];
      }

      /*
       * And finally unpack exactly as above, except that
       * chan shift is adjusted and the right vector selected.
       */
      if (!fp64) {
         for (i = 0; i < num_gather; i++) {
            dst[i] = LLVMBuildBitCast(builder, dst[i], bld.int_vec_type, "");
         }
         for (i = 0; i < format_desc->nr_channels; i++) {
            struct util_format_channel_description chan_desc = format_desc->channel[i];
            unsigned blockbits = type.width;
            unsigned vec_nr;

#if UTIL_ARCH_BIG_ENDIAN
            vec_nr = (format_desc->block.bits - (chan_desc.shift + chan_desc.size)) / type.width;
#else
            vec_nr = chan_desc.shift / type.width;
#endif
            chan_desc.shift %= type.width;

            output[i] = lp_build_extract_soa_chan(&bld,
                                                  blockbits,
                                                  false,
                                                  chan_desc,
                                                  dst[vec_nr]);
         }
      }
      else {
         for (i = 0; i < format_desc->nr_channels; i++)  {
            output[i] = dst[i];
         }
      }

      lp_build_format_swizzle_soa(format_desc, &bld, output, rgba_out);
      return;
   }

   if (format == PIPE_FORMAT_R11G11B10_FLOAT ||
       format == PIPE_FORMAT_R9G9B9E5_FLOAT) {
      /*
       * similar conceptually to above but requiring special
       * AoS packed -> SoA float conversion code.
       */
      LLVMValueRef packed;
      struct lp_type fetch_type = lp_type_uint(type.width);

      assert(type.floating);
      assert(type.width == 32);

      packed = lp_build_gather(gallivm, type.length,
                               format_desc->block.bits,
                               fetch_type, aligned,
                               base_ptr, offset, false);
      if (format == PIPE_FORMAT_R11G11B10_FLOAT) {
         lp_build_r11g11b10_to_float(gallivm, packed, rgba_out);
      }
      else {
         lp_build_rgb9e5_to_float(gallivm, packed, rgba_out);
      }
      return;
   }

   if (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS &&
       format_desc->block.bits == 64) {
      /*
       * special case the format is 64 bits but we only require
       * 32bit (or 8bit) from each block.
       */
      LLVMValueRef packed;
      struct lp_type fetch_type = lp_type_uint(type.width);

      if (format == PIPE_FORMAT_X32_S8X24_UINT) {
         /*
          * for stencil simply fix up offsets - could in fact change
          * base_ptr instead even outside the shader.
          */
         unsigned mask = (1 << 8) - 1;
         LLVMValueRef s_offset = lp_build_const_int_vec(gallivm, type, 4);
         offset = LLVMBuildAdd(builder, offset, s_offset, "");
         packed = lp_build_gather(gallivm, type.length, 32, fetch_type,
                                  aligned, base_ptr, offset, false);
         packed = LLVMBuildAnd(builder, packed,
                               lp_build_const_int_vec(gallivm, type, mask), "");
      }
      else {
         assert (format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT);
         packed = lp_build_gather(gallivm, type.length, 32, fetch_type,
                                  aligned, base_ptr, offset, true);
         packed = LLVMBuildBitCast(builder, packed,
                                   lp_build_vec_type(gallivm, type), "");
      }
      /* for consistency with lp_build_unpack_rgba_soa() return sss1 or zzz1 */
      rgba_out[0] = rgba_out[1] = rgba_out[2] = packed;
      rgba_out[3] = lp_build_const_vec(gallivm, type, 1.0f);
      return;
   }

   /*
    * Try calling lp_build_fetch_rgba_aos for all pixels.
    * Should only really hit subsampled, compressed
    * (for s3tc srgb and rgtc too).
    * (This is invalid for plain 8unorm formats because we're lazy with
    * the swizzle since some results would arrive swizzled, some not.)
    */

   if ((format_desc->layout != UTIL_FORMAT_LAYOUT_PLAIN) &&
       (util_format_fits_8unorm(format_desc) ||
        format_desc->layout == UTIL_FORMAT_LAYOUT_RGTC ||
        format_desc->layout == UTIL_FORMAT_LAYOUT_S3TC) &&
       type.floating && type.width == 32 &&
       (type.length == 1 || (type.length % 4 == 0))) {
      struct lp_type tmp_type;
      struct lp_build_context bld;
      LLVMValueRef packed, rgba[4];
      const struct util_format_description *flinear_desc;
      const struct util_format_description *frgba8_desc;
      unsigned chan;
      bool is_signed = (format_desc->format == PIPE_FORMAT_RGTC1_SNORM ||
                        format_desc->format == PIPE_FORMAT_RGTC2_SNORM ||
                        format_desc->format == PIPE_FORMAT_LATC1_SNORM ||
                        format_desc->format == PIPE_FORMAT_LATC2_SNORM);

      lp_build_context_init(&bld, gallivm, type);

      /*
       * Make sure the conversion in aos really only does convert to rgba8
       * and not anything more (so use linear format, adjust type).
       */
      flinear_desc = util_format_description(util_format_linear(format));
      memset(&tmp_type, 0, sizeof tmp_type);
      tmp_type.width = 8;
      tmp_type.length = type.length * 4;
      tmp_type.norm = true;
      tmp_type.sign = is_signed;

      packed = lp_build_fetch_rgba_aos(gallivm, flinear_desc, tmp_type,
                                       aligned, base_ptr, offset, i, j, cache);
      packed = LLVMBuildBitCast(builder, packed, bld.int_vec_type, "");

      /*
       * The values are now packed so they match ordinary (srgb) RGBA8 format,
       * hence need to use matching format for unpack.
       */
      frgba8_desc = util_format_description(is_signed ? PIPE_FORMAT_R8G8B8A8_SNORM : PIPE_FORMAT_R8G8B8A8_UNORM);
      if (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB) {
         assert(format_desc->layout == UTIL_FORMAT_LAYOUT_S3TC);
         frgba8_desc = util_format_description(PIPE_FORMAT_R8G8B8A8_SRGB);
      }
      lp_build_unpack_rgba_soa(gallivm,
                               frgba8_desc,
                               type,
                               packed, rgba);

      /*
       * We converted 4 channels. Make sure llvm can drop unneeded ones
       * (luckily the rgba order is fixed, only LA needs special case).
       */
      for (chan = 0; chan < 4; chan++) {
         enum pipe_swizzle swizzle = format_desc->swizzle[chan];
         if (chan == 3 && util_format_is_luminance_alpha(format)) {
            swizzle = PIPE_SWIZZLE_W;
         }
         rgba_out[chan] = lp_build_swizzle_soa_channel(&bld, rgba, swizzle);
      }
      return;
   }


   /*
    * Fallback to calling lp_build_fetch_rgba_aos for each pixel.
    *
    * This is not the most efficient way of fetching pixels, as we
    * miss some opportunities to do vectorization, but this is
    * convenient for formats or scenarios for which there was no
    * opportunity or incentive to optimize.
    *
    * We do NOT want to end up here, this typically is quite terrible,
    * in particular if the formats have less than 4 channels.
    *
    * Right now, this should only be hit for:
    * - ETC formats
    *   (those miss fast fetch functions hence they are terrible anyway)
    */

   {
      unsigned k;
      struct lp_type tmp_type;
      LLVMValueRef aos_fetch[LP_MAX_VECTOR_WIDTH / 32];

      if (gallivm_debug & GALLIVM_DEBUG_PERF) {
         debug_printf("%s: AoS fetch fallback for %s\n",
                      __func__, format_desc->short_name);
      }

      tmp_type = type;
      tmp_type.length = 4;

      if (type.length == 1) {
         LLVMValueRef fetch = lp_build_fetch_rgba_aos(gallivm, format_desc, tmp_type,
                                                      aligned, base_ptr, offset,
                                                      i, j, cache);

         for (k = 0; k < 4; k++)
            rgba_out[k] = LLVMBuildExtractElement(gallivm->builder, fetch, lp_build_const_int32(gallivm, k), "");
         return;
      }

      /*
       * Note that vector transpose can be worse compared to insert/extract
       * for aos->soa conversion (for formats with 1 or 2 channels). However,
       * we should try to avoid getting here for just about all formats, so
       * don't bother.
       */

      /* loop over number of pixels */
      for(k = 0; k < type.length; ++k) {
         LLVMValueRef index = lp_build_const_int32(gallivm, k);
         LLVMValueRef offset_elem;
         LLVMValueRef i_elem, j_elem;

         offset_elem = LLVMBuildExtractElement(builder, offset,
                                               index, "");

         i_elem = LLVMBuildExtractElement(builder, i, index, "");
         j_elem = LLVMBuildExtractElement(builder, j, index, "");

         /* Get a single float[4]={R,G,B,A} pixel */
         aos_fetch[k] = lp_build_fetch_rgba_aos(gallivm, format_desc, tmp_type,
                                                aligned, base_ptr, offset_elem,
                                                i_elem, j_elem, cache);

      }
      convert_to_soa(gallivm, aos_fetch, rgba_out, type);
   }
}

static void
lp_build_insert_soa_chan(struct lp_build_context *bld,
                         unsigned blockbits,
                         struct util_format_channel_description chan_desc,
                         LLVMValueRef *output,
                         LLVMValueRef rgba)
{
    struct gallivm_state *gallivm = bld->gallivm;
    LLVMBuilderRef builder = gallivm->builder;
    struct lp_type type = bld->type;
    const unsigned width = chan_desc.size;
    const unsigned start = chan_desc.shift;
    const uint32_t chan_mask = (1ULL << width) - 1;
    ASSERTED const unsigned stop = start + width;
    LLVMValueRef chan = NULL;
    switch(chan_desc.type) {
    case UTIL_FORMAT_TYPE_UNSIGNED:

       if (chan_desc.pure_integer) {
          chan = LLVMBuildBitCast(builder, rgba, bld->int_vec_type, "");
          LLVMValueRef mask_val = lp_build_const_int_vec(gallivm, type, chan_mask);
          LLVMValueRef mask = LLVMBuildICmp(builder, LLVMIntUGT, chan, mask_val, "");
          chan = LLVMBuildSelect(builder, mask, mask_val, chan, "");
       }
       else if (type.floating) {
          if (chan_desc.normalized) {
             rgba = lp_build_clamp(bld, rgba, bld->zero, bld->one);
             chan = lp_build_clamped_float_to_unsigned_norm(gallivm, type, width, rgba);
          } else
             chan = LLVMBuildFPToSI(builder, rgba, bld->vec_type, "");
       }
       if (start)
          chan = LLVMBuildShl(builder, chan,
                              lp_build_const_int_vec(gallivm, type, start), "");
       if (!*output)
          *output = chan;
       else
          *output = LLVMBuildOr(builder, *output, chan, "");
       break;
    case UTIL_FORMAT_TYPE_SIGNED:
       if (chan_desc.pure_integer) {
          chan = LLVMBuildBitCast(builder, rgba, bld->int_vec_type, "");
          /* clamp to SINT range for < 32-bit values */
          if (width < 32) {
             struct lp_build_context int_bld;
             lp_build_context_init(&int_bld, gallivm, lp_int_type(bld->type));
             chan = lp_build_clamp(&int_bld, chan,
                                   lp_build_const_int_vec(gallivm, type, -(1ULL << (width - 1))),
                                   lp_build_const_int_vec(gallivm, type, (1ULL << (width - 1)) - 1));
             chan = LLVMBuildAnd(builder, chan, lp_build_const_int_vec(gallivm, type, chan_mask), "");
          }
       } else if (type.floating) {
          if (chan_desc.normalized) {
             char intrin[32];
             double scale = ((1 << (chan_desc.size - 1)) - 1);
             LLVMValueRef scale_val = lp_build_const_vec(gallivm, type, scale);
             rgba = lp_build_clamp(bld, rgba, lp_build_negate(bld, bld->one), bld->one);
             rgba = LLVMBuildFMul(builder, rgba, scale_val, "");
             lp_format_intrinsic(intrin, sizeof intrin, "llvm.rint", bld->vec_type);
             rgba = lp_build_intrinsic_unary(builder, intrin, bld->vec_type, rgba);
          }
          chan = LLVMBuildFPToSI(builder, rgba, bld->int_vec_type, "");
          chan = LLVMBuildAnd(builder, chan, lp_build_const_int_vec(gallivm, type, chan_mask), "");
       }
       if (start)
          chan = LLVMBuildShl(builder, chan,
                              lp_build_const_int_vec(gallivm, type, start), "");
       if (!*output)
          *output = chan;
       else
          *output = LLVMBuildOr(builder, *output, chan, "");
       break;
    case UTIL_FORMAT_TYPE_FLOAT:
       if (type.floating) {
          if (chan_desc.size == 16) {
             chan = lp_build_float_to_half(gallivm, rgba);
             chan = LLVMBuildBitCast(builder, chan,
				     lp_build_vec_type(gallivm, lp_type_int_vec(16, 16 * type.length)), "");
             chan = LLVMBuildZExt(builder, chan, bld->int_vec_type, "");
             if (start)
                chan = LLVMBuildShl(builder, chan,
                                    lp_build_const_int_vec(gallivm, type, start), "");
             if (!*output)
                *output = chan;
             else
                *output = LLVMBuildOr(builder, *output, chan, "");
          } else {
             assert(start == 0);
             assert(stop == 32);
             assert(type.width == 32);
             *output = LLVMBuildBitCast(builder, rgba, bld->int_vec_type, "");
          }
       } else
          assert(0);
       break;
    default:
       assert(0);
       *output = bld->undef;
    }
}

static void
lp_build_pack_rgba_soa(struct gallivm_state *gallivm,
                       const struct util_format_description *format_desc,
                       struct lp_type type,
                       const LLVMValueRef rgba_in[4],
                       LLVMValueRef *packed)
{
   unsigned chan;
   struct lp_build_context bld;
   LLVMValueRef rgba_swiz[4];
   assert(format_desc->layout == UTIL_FORMAT_LAYOUT_PLAIN);
   assert(format_desc->block.width == 1);
   assert(format_desc->block.height == 1);
   assert(format_desc->block.bits <= type.width);
   /* FIXME: Support more output types */
   assert(type.width == 32);

   lp_build_context_init(&bld, gallivm, type);

   lp_build_format_swizzle_soa(format_desc, &bld, rgba_in, rgba_swiz);

   for (chan = 0; chan < format_desc->nr_channels; ++chan) {
      struct util_format_channel_description chan_desc = format_desc->channel[chan];

      lp_build_insert_soa_chan(&bld, format_desc->block.bits,
                               chan_desc,
                               packed,
                               rgba_swiz[chan]);
   }
}

void
lp_build_store_rgba_soa(struct gallivm_state *gallivm,
                        const struct util_format_description *format_desc,
                        struct lp_type type,
                        LLVMValueRef exec_mask,
                        LLVMValueRef base_ptr,
                        LLVMValueRef offset,
                        LLVMValueRef out_of_bounds,
                        const LLVMValueRef rgba_in[4])
{
   enum pipe_format format = format_desc->format;
   LLVMValueRef packed[4];
   unsigned num_stores = 0;

   memset(packed, 0, sizeof(LLVMValueRef) * 4);
   if (format_desc->layout == UTIL_FORMAT_LAYOUT_PLAIN &&
       format_desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB &&
       !util_format_is_alpha(format) &&
       format_desc->block.width == 1 &&
       format_desc->block.height == 1 &&
       format_desc->block.bits <= type.width &&
       (format_desc->channel[0].type != UTIL_FORMAT_TYPE_FLOAT ||
        format_desc->channel[0].size == 32 ||
        format_desc->channel[0].size == 16))
   {
      lp_build_pack_rgba_soa(gallivm, format_desc, type, rgba_in, &packed[0]);

      num_stores = 1;
   } else if (format_desc->layout == UTIL_FORMAT_LAYOUT_PLAIN &&
       (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB) &&
       format_desc->block.width == 1 &&
       format_desc->block.height == 1 &&
       format_desc->block.bits > type.width &&
       ((format_desc->block.bits <= type.width * type.length &&
         format_desc->channel[0].size <= type.width) ||
        (format_desc->channel[0].size == 64 &&
         format_desc->channel[0].type == UTIL_FORMAT_TYPE_FLOAT &&
         type.floating)))
   {
      /*
       * Similar to above, but the packed pixel is larger than what fits
       * into an element of the destination format. The packed pixels will be
       * shuffled into SoA vectors appropriately, and then the extraction will
       * be done in parallel as much as possible.
       * Good for 16xn (n > 2) and 32xn (n > 1) formats, care is taken so
       * the gathered vectors can be shuffled easily (even with avx).
       * 64xn float -> 32xn float is handled too but it's a bit special as
       * it does the conversion pre-shuffle.
       */
      struct lp_build_context bld;

      lp_build_context_init(&bld, gallivm, type);
      assert(type.width == 32);
      assert(format_desc->block.bits > type.width);

      unsigned store_width = util_next_power_of_two(format_desc->block.bits);
      num_stores = store_width / type.width;
      for (unsigned i = 0; i < format_desc->nr_channels; i++) {
            struct util_format_channel_description chan_desc = format_desc->channel[i];
            unsigned blockbits = type.width;
            unsigned vec_nr;

            vec_nr = chan_desc.shift / type.width;
            chan_desc.shift %= type.width;

            lp_build_insert_soa_chan(&bld, blockbits,
                                     chan_desc,
                                     &packed[vec_nr],
                                     rgba_in[i]);
      }

      assert(num_stores == 4 || num_stores == 2);
      /* we can transpose and store at the same time */
   } else if (format == PIPE_FORMAT_R11G11B10_FLOAT) {
      packed[0] = lp_build_float_to_r11g11b10(gallivm, rgba_in);
      num_stores = 1;
   } else if (util_format_is_alpha(format)) {
      assert(format_desc->format == PIPE_FORMAT_A8_UNORM);
      struct lp_build_context bld;
      lp_build_context_init(&bld, gallivm, type);
      lp_build_insert_soa_chan(&bld, type.width, format_desc->channel[0], &packed[0], rgba_in[3]);
      num_stores = 1;
   } else
      assert(0);

   assert(exec_mask);

   LLVMTypeRef int32_ptr_type = LLVMPointerType(LLVMInt32TypeInContext(gallivm->context), 0);
   LLVMTypeRef int16_ptr_type = LLVMPointerType(LLVMInt16TypeInContext(gallivm->context), 0);
   LLVMTypeRef int8_ptr_type = LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0);

   LLVMValueRef should_store_mask = LLVMBuildAnd(gallivm->builder, exec_mask, LLVMBuildNot(gallivm->builder, out_of_bounds, ""), "store_mask");
   should_store_mask = LLVMBuildICmp(gallivm->builder, LLVMIntNE, should_store_mask, lp_build_const_int_vec(gallivm, type, 0), "");
   for (unsigned i = 0; i < num_stores; i++) {
      struct lp_build_loop_state loop_state;

      LLVMValueRef store_offset = LLVMBuildAdd(gallivm->builder, offset, lp_build_const_int_vec(gallivm, type, i * 4), "");
      store_offset = LLVMBuildGEP2(gallivm->builder, LLVMInt8TypeInContext(gallivm->context), base_ptr, &store_offset, 1, "");

      lp_build_loop_begin(&loop_state, gallivm, lp_build_const_int32(gallivm, 0));

      struct lp_build_if_state ifthen;
      LLVMValueRef cond = LLVMBuildExtractElement(gallivm->builder, should_store_mask, loop_state.counter, "");
      lp_build_if(&ifthen, gallivm, cond);

      LLVMValueRef data = LLVMBuildExtractElement(gallivm->builder, packed[i], loop_state.counter, "");
      LLVMValueRef this_offset = LLVMBuildExtractElement(gallivm->builder, store_offset, loop_state.counter, "");

      if (format_desc->block.bits == 8) {
         this_offset = LLVMBuildBitCast(gallivm->builder, this_offset, int8_ptr_type, "");
         data = LLVMBuildTrunc(gallivm->builder, data, LLVMInt8TypeInContext(gallivm->context), "");
      } else if (format_desc->block.bits == 16) {
         this_offset = LLVMBuildBitCast(gallivm->builder, this_offset, int16_ptr_type, "");
         data = LLVMBuildTrunc(gallivm->builder, data, LLVMInt16TypeInContext(gallivm->context), "");
      } else
         this_offset = LLVMBuildBitCast(gallivm->builder, this_offset, int32_ptr_type, "");
      LLVMBuildStore(gallivm->builder, data, this_offset);
      lp_build_endif(&ifthen);
      lp_build_loop_end_cond(&loop_state, lp_build_const_int32(gallivm, type.length),
                             NULL, LLVMIntUGE);
   }
}
