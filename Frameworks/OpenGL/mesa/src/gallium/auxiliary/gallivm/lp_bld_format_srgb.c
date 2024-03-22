/**************************************************************************
 *
 * Copyright 2013 VMware, Inc.
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
 * Format conversion code for srgb formats.
 *
 * Functions for converting from srgb to linear and vice versa.
 * From http://www.opengl.org/registry/specs/EXT/texture_sRGB.txt:
 *
 * srgb->linear:
 * cl = cs / 12.92,                 cs <= 0.04045
 * cl = ((cs + 0.055)/1.055)^2.4,   cs >  0.04045
 *
 * linear->srgb:
 * if (isnan(cl)) {
 *    Map IEEE-754 Not-a-number to zero.
 *    cs = 0.0;
 * } else if (cl > 1.0) {
 *    cs = 1.0;
 * } else if (cl < 0.0) {
 *    cs = 0.0;
 * } else if (cl < 0.0031308) {
 *    cs = 12.92 * cl;
 * } else {
 *    cs = 1.055 * pow(cl, 0.41666) - 0.055;
 * }
 *
 * This does not need to be accurate, however at least for d3d10
 * (http://msdn.microsoft.com/en-us/library/windows/desktop/dd607323%28v=vs.85%29.aspx):
 * 1) For srgb->linear, it is required that the error on the srgb side is
 *    not larger than 0.5f, which I interpret that if you map the value back
 *    to srgb from linear using the ideal conversion, it would not be off by
 *    more than 0.5f (that is, it would map to the same 8-bit integer value
 *    as it was before conversion to linear).
 * 2) linear->srgb is permitted 0.6f which luckily looks like quite a large
 *    error is allowed.
 * 3) Additionally, all srgb values converted to linear and back must result
 *    in the same value as they were originally.
 *
 * @author Roland Scheidegger <sroland@vmware.com>
 */


#include "util/u_debug.h"
#include "util/u_math.h"

#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_arit.h"
#include "lp_bld_bitarit.h"
#include "lp_bld_logic.h"
#include "lp_bld_format.h"



/**
 * Convert srgb int values to linear float values.
 * Several possibilities how to do this, e.g.
 * - table
 * - doing the pow() with int-to-float and float-to-int tricks
 *   (http://stackoverflow.com/questions/6475373/optimizations-for-pow-with-const-non-integer-exponent)
 * - just using standard polynomial approximation
 *   (3rd order polynomial is required for crappy but just sufficient accuracy)
 *
 * @param src   integer (vector) value(s) to convert
 *              (chan_bits bit values unpacked to 32 bit already).
 */
LLVMValueRef
lp_build_srgb_to_linear(struct gallivm_state *gallivm,
                        struct lp_type src_type,
                        unsigned chan_bits,
                        LLVMValueRef src)
{
   struct lp_type f32_type = lp_type_float_vec(32, src_type.length * 32);
   struct lp_build_context f32_bld;
   LLVMValueRef srcf, part_lin, part_pow, is_linear, lin_const, lin_thresh;
   double coeffs[4] = {0.0023f,
                       0.0030f / 255.0f,
                       0.6935f / (255.0f * 255.0f),
                       0.3012f / (255.0f * 255.0f * 255.0f)
   };

   assert(src_type.width == 32);
   /* Technically this would work with more bits too but would be inaccurate. */
   assert(chan_bits <= 8);

   lp_build_context_init(&f32_bld, gallivm, f32_type);

   /*
    * using polynomial: (src * (src * (src * 0.3012 + 0.6935) + 0.0030) + 0.0023)
    * ( poly =  0.3012*x^3 + 0.6935*x^2 + 0.0030*x + 0.0023)
    * (found with octave polyfit and some magic as I couldn't get the error
    * function right). Using the above mentioned error function, the values stay
    * within +-0.35, except for the lowest values - hence tweaking linear segment
    * to cover the first 16 instead of the first 11 values (the error stays
    * just about acceptable there too).
    * Hence: lin = src > 15 ? poly : src / 12.6
    * This function really only makes sense for vectors, should use LUT otherwise.
    * All in all (including float conversion) 11 instructions (with sse4.1),
    * 6 constants (polynomial could be done with 1 instruction less at the cost
    * of slightly worse dependency chain, fma should also help).
    */
   /* doing the 1/255 mul as part of the approximation */
   srcf = lp_build_int_to_float(&f32_bld, src);
   if (chan_bits != 8) {
      /* could adjust all the constants instead */
      LLVMValueRef rescale_const = lp_build_const_vec(gallivm, f32_type,
                                                      255.0f / ((1 << chan_bits) - 1));
      srcf = lp_build_mul(&f32_bld, srcf, rescale_const);
   }
   lin_const = lp_build_const_vec(gallivm, f32_type, 1.0f / (12.6f * 255.0f));
   part_lin = lp_build_mul(&f32_bld, srcf, lin_const);

   part_pow = lp_build_polynomial(&f32_bld, srcf, coeffs, 4);

   lin_thresh = lp_build_const_vec(gallivm, f32_type, 15.0f);
   is_linear = lp_build_compare(gallivm, f32_type, PIPE_FUNC_LEQUAL, srcf, lin_thresh);
   return lp_build_select(&f32_bld, is_linear, part_lin, part_pow);
}


/**
 * Convert linear float values to srgb int values.
 * Several possibilities how to do this, e.g.
 * - use table (based on exponent/highest order mantissa bits) and do
 *   linear interpolation (https://gist.github.com/rygorous/2203834)
 * - Chebyshev polynomial
 * - Approximation using reciprocals
 * - using int-to-float and float-to-int tricks for pow()
 *   (http://stackoverflow.com/questions/6475373/optimizations-for-pow-with-const-non-integer-exponent)
 *
 * @param src   float (vector) value(s) to convert.
 */
static LLVMValueRef
lp_build_linear_to_srgb(struct gallivm_state *gallivm,
                        struct lp_type src_type,
                        unsigned chan_bits,
                        LLVMValueRef src)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context f32_bld;
   LLVMValueRef lin_thresh, lin, lin_const, is_linear, tmp, pow_final;

   lp_build_context_init(&f32_bld, gallivm, src_type);

   src = lp_build_clamp(&f32_bld, src, f32_bld.zero, f32_bld.one);

   if (0) {
      /*
       * using int-to-float and float-to-int trick for pow().
       * This is much more accurate than necessary thanks to the correction,
       * but it most certainly makes no sense without rsqrt available.
       * Bonus points if you understand how this works...
       * All in all (including min/max clamp, conversion) 19 instructions.
       */

      float exp_f = 2.0f / 3.0f;
      /* some compilers can't do exp2f, so this is exp2f(127.0f/exp_f - 127.0f) */
      float exp2f_c = 1.30438178253e+19f;
      float coeff_f = 0.62996f;
      LLVMValueRef pow_approx, coeff, x2, exponent, pow_1, pow_2;
      struct lp_type int_type = lp_int_type(src_type);

      /*
       * First calculate approx x^8/12
       */
      exponent = lp_build_const_vec(gallivm, src_type, exp_f);
      coeff = lp_build_const_vec(gallivm, src_type,
                                 exp2f_c * powf(coeff_f, 1.0f / exp_f));

      /* premultiply src */
      tmp = lp_build_mul(&f32_bld, coeff, src);
      /* "log2" */
      tmp = LLVMBuildBitCast(builder, tmp, lp_build_vec_type(gallivm, int_type), "");
      tmp = lp_build_int_to_float(&f32_bld, tmp);
      /* multiply for pow */
      tmp = lp_build_mul(&f32_bld, tmp, exponent);
      /* "exp2" */
      pow_approx = lp_build_itrunc(&f32_bld, tmp);
      pow_approx = LLVMBuildBitCast(builder, pow_approx,
                                    lp_build_vec_type(gallivm, src_type), "");

      /*
       * Since that pow was inaccurate (like 3 bits, though each sqrt step would
       * give another bit), compensate the error (which is why we chose another
       * exponent in the first place).
       */
      /* x * x^(8/12) = x^(20/12) */
      pow_1 = lp_build_mul(&f32_bld, pow_approx, src);

      /* x * x * x^(-4/12) = x^(20/12) */
      /* Should avoid using rsqrt if it's not available, but
       * using x * x^(4/12) * x^(4/12) instead will change error weight */
      tmp = lp_build_fast_rsqrt(&f32_bld, pow_approx);
      x2 = lp_build_mul(&f32_bld, src, src);
      pow_2 = lp_build_mul(&f32_bld, x2, tmp);

      /* average the values so the errors cancel out, compensate bias,
       * we also squeeze the 1.055 mul of the srgb conversion plus the 255.0 mul
       * for conversion to int in here */
      tmp = lp_build_add(&f32_bld, pow_1, pow_2);
      coeff = lp_build_const_vec(gallivm, src_type,
                                 1.0f / (3.0f * coeff_f) * 0.999852f *
                                 powf(1.055f * 255.0f, 4.0f));
      pow_final = lp_build_mul(&f32_bld, tmp, coeff);

      /* x^(5/12) = rsqrt(rsqrt(x^20/12)) */
      if (lp_build_fast_rsqrt_available(src_type)) {
         pow_final = lp_build_fast_rsqrt(&f32_bld,
                        lp_build_fast_rsqrt(&f32_bld, pow_final));
      }
      else {
         pow_final = lp_build_sqrt(&f32_bld, lp_build_sqrt(&f32_bld, pow_final));
      }
      pow_final = lp_build_add(&f32_bld, pow_final,
                               lp_build_const_vec(gallivm, src_type, -0.055f * 255.0f));
   }

   else {
      /*
       * using "rational polynomial" approximation here.
       * Essentially y = a*x^0.375 + b*x^0.5 + c, with also
       * factoring in the 255.0 mul and the scaling mul.
       * (a is closer to actual value so has higher weight than b.)
       * Note: the constants are magic values. They were found empirically,
       * possibly could be improved but good enough (be VERY careful with
       * error metric if you'd want to tweak them, they also MUST fit with
       * the crappy polynomial above for srgb->linear since it is required
       * that each srgb value maps back to the same value).
       * This function has an error of max +-0.17. Not sure this is actually
       * enough, we require +-0.6 but that may include the +-0.5 from integer
       * conversion. Seems to pass all relevant tests though...
       * For the approximated srgb->linear values the error is naturally larger
       * (+-0.42) but still accurate enough (required +-0.5 essentially).
       * All in all (including min/max clamp, conversion) 15 instructions.
       * FMA would help (minus 2 instructions).
       */

      LLVMValueRef x05, x0375, a_const, b_const, c_const, tmp2;

      if (lp_build_fast_rsqrt_available(src_type)) {
         tmp = lp_build_fast_rsqrt(&f32_bld, src);
         x05 = lp_build_mul(&f32_bld, src, tmp);
      }
      else {
         /*
          * I don't really expect this to be practical without rsqrt
          * but there's no reason for triple punishment so at least
          * save the otherwise resulting division and unnecessary mul...
          */
         x05 = lp_build_sqrt(&f32_bld, src);
      }

      tmp = lp_build_mul(&f32_bld, x05, src);
      if (lp_build_fast_rsqrt_available(src_type)) {
         x0375 = lp_build_fast_rsqrt(&f32_bld, lp_build_fast_rsqrt(&f32_bld, tmp));
      }
      else {
         x0375 = lp_build_sqrt(&f32_bld, lp_build_sqrt(&f32_bld, tmp));
      }

      a_const = lp_build_const_vec(gallivm, src_type, 0.675f * 1.0622 * 255.0f);
      b_const = lp_build_const_vec(gallivm, src_type, 0.325f * 1.0622 * 255.0f);
      c_const = lp_build_const_vec(gallivm, src_type, -0.0620f * 255.0f);

      tmp = lp_build_mul(&f32_bld, a_const, x0375);
      tmp2 = lp_build_mad(&f32_bld, b_const, x05, c_const);
      pow_final = lp_build_add(&f32_bld, tmp, tmp2);
   }

   /* linear part is easy */
   lin_const = lp_build_const_vec(gallivm, src_type, 12.92f * 255.0f);
   lin = lp_build_mul(&f32_bld, src, lin_const);

   lin_thresh = lp_build_const_vec(gallivm, src_type, 0.0031308f);
   is_linear = lp_build_compare(gallivm, src_type, PIPE_FUNC_LEQUAL, src, lin_thresh);
   tmp = lp_build_select(&f32_bld, is_linear, lin, pow_final);

   if (chan_bits != 8) {
      /* could adjust all the constants instead */
      LLVMValueRef rescale_const = lp_build_const_vec(gallivm, src_type,
                                                      ((1 << chan_bits) - 1) / 255.0f);
      tmp = lp_build_mul(&f32_bld, tmp, rescale_const);
   }

   f32_bld.type.sign = 0;
   return lp_build_iround(&f32_bld, tmp);
}


/**
 * Convert linear float soa values to packed srgb AoS values.
 * This only handles packed formats which are 4x8bit in size
 * (rgba and rgbx plus swizzles), and 16bit 565-style formats
 * with no alpha. (In the latter case the return values won't be
 * fully packed, it will look like r5g6b5x16r5g6b5x16...)
 *
 * @param src   float SoA (vector) values to convert.
 */
LLVMValueRef
lp_build_float_to_srgb_packed(struct gallivm_state *gallivm,
                              const struct util_format_description *dst_fmt,
                              struct lp_type src_type,
                              LLVMValueRef *src)
{
   LLVMBuilderRef builder = gallivm->builder;
   unsigned chan;
   struct lp_build_context f32_bld;
   struct lp_type int32_type = lp_int_type(src_type);
   LLVMValueRef tmpsrgb[4], alpha, dst;

   lp_build_context_init(&f32_bld, gallivm, src_type);

   /* rgb is subject to linear->srgb conversion, alpha is not */
   for (chan = 0; chan < 3; chan++) {
      unsigned chan_bits = dst_fmt->channel[dst_fmt->swizzle[chan]].size;
      tmpsrgb[chan] = lp_build_linear_to_srgb(gallivm, src_type, chan_bits, src[chan]);
   }
   /*
    * can't use lp_build_conv since we want to keep values as 32bit
    * here so we can interleave with rgb to go from SoA->AoS.
    */
   alpha = lp_build_clamp_zero_one_nanzero(&f32_bld, src[3]);
   alpha = lp_build_mul(&f32_bld, alpha,
                        lp_build_const_vec(gallivm, src_type, 255.0f));
   tmpsrgb[3] = lp_build_iround(&f32_bld, alpha);

   dst = lp_build_zero(gallivm, int32_type);
   for (chan = 0; chan < dst_fmt->nr_channels; chan++) {
      if (dst_fmt->swizzle[chan] <= PIPE_SWIZZLE_W) {
         unsigned ls;
         LLVMValueRef shifted, shift_val;
         ls = dst_fmt->channel[dst_fmt->swizzle[chan]].shift;
         shift_val = lp_build_const_int_vec(gallivm, int32_type, ls);
         shifted = LLVMBuildShl(builder, tmpsrgb[chan], shift_val, "");
         dst = LLVMBuildOr(builder, dst, shifted, "");
      }
   }
   return dst;
}
