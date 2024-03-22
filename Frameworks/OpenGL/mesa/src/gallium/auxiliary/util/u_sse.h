/**************************************************************************
 *
 * Copyright 2008-2021 VMware, Inc.
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
 * SSE intrinsics portability header.
 * 
 * Although the SSE intrinsics are support by all modern x86 and x86-64 
 * compilers, there are some intrisincs missing in some implementations 
 * (especially older MSVC versions). This header abstracts that away.
 */

#ifndef U_SSE_H_
#define U_SSE_H_

#include "util/detect.h"
#include "util/compiler.h"
#include "util/u_debug.h"

#if DETECT_ARCH_SSE

#include <emmintrin.h>


union m128i {
   __m128i m;
   uint8_t ub[16];
   uint16_t us[8];
   uint32_t ui[4];
};

/*
 * Provide an SSE implementation of _mm_mul_epi32() in terms of
 * _mm_mul_epu32().
 *
 * Basically, albeit surprising at first (and second, and third...) look
 * if a * b is done signed instead of unsigned, can just
 * subtract b from the high bits of the result if a is negative
 * (and the same for a if b is negative). Modular arithmetic at its best!
 *
 * So for int32 a,b in crude pseudo-code ("*" here denoting a widening mul)
 * fixupb = (signmask(b) & a) << 32ULL
 * fixupa = (signmask(a) & b) << 32ULL
 * a * b = (unsigned)a * (unsigned)b - fixupb - fixupa
 * = (unsigned)a * (unsigned)b -(fixupb + fixupa)
 *
 * This does both lo (dwords 0/2) and hi parts (1/3) at the same time due
 * to some optimization potential.
 */
static inline __m128i
mm_mullohi_epi32(const __m128i a, const __m128i b, __m128i *res13)
{
   __m128i a13, b13, mul02, mul13;
   __m128i anegmask, bnegmask, fixup, fixup02, fixup13;
   a13 = _mm_shuffle_epi32(a, _MM_SHUFFLE(2,3,0,1));
   b13 = _mm_shuffle_epi32(b, _MM_SHUFFLE(2,3,0,1));
   anegmask = _mm_srai_epi32(a, 31);
   bnegmask = _mm_srai_epi32(b, 31);
   fixup = _mm_add_epi32(_mm_and_si128(anegmask, b),
                         _mm_and_si128(bnegmask, a));
   mul02 = _mm_mul_epu32(a, b);
   mul13 = _mm_mul_epu32(a13, b13);
   fixup02 = _mm_slli_epi64(fixup, 32);
   fixup13 = _mm_and_si128(fixup, _mm_set_epi32(-1,0,-1,0));
   *res13 = _mm_sub_epi64(mul13, fixup13);
   return _mm_sub_epi64(mul02, fixup02);
}


/* Provide an SSE2 implementation of _mm_mullo_epi32() in terms of
 * _mm_mul_epu32().
 *
 * This always works regardless the signs of the operands, since
 * the high bits (which would be different) aren't used.
 *
 * This seems close enough to the speed of SSE4 and the real
 * _mm_mullo_epi32() intrinsic as to not justify adding an sse4
 * dependency at this point.
 */
static inline __m128i mm_mullo_epi32(const __m128i a, const __m128i b)
{
   __m128i a4   = _mm_srli_epi64(a, 32);  /* shift by one dword */
   __m128i b4   = _mm_srli_epi64(b, 32);  /* shift by one dword */
   __m128i ba   = _mm_mul_epu32(b, a);   /* multply dwords 0, 2 */
   __m128i b4a4 = _mm_mul_epu32(b4, a4); /* multiply dwords 1, 3 */

   /* Interleave the results, either with shuffles or (slightly
    * faster) direct bit operations:
    * XXX: might be only true for some cpus (in particular 65nm
    * Core 2). On most cpus (including that Core 2, but not Nehalem...)
    * using _mm_shuffle_ps/_mm_shuffle_epi32 might also be faster
    * than using the 3 instructions below. But logic should be fine
    * as well, we can't have optimal solution for all cpus (if anything,
    * should just use _mm_mullo_epi32() if sse41 is available...).
    */
#if 0
   __m128i ba8             = _mm_shuffle_epi32(ba, 8);
   __m128i b4a48           = _mm_shuffle_epi32(b4a4, 8);
   __m128i result          = _mm_unpacklo_epi32(ba8, b4a48);
#else
   __m128i mask            = _mm_setr_epi32(~0,0,~0,0);
   __m128i ba_mask         = _mm_and_si128(ba, mask);
   __m128i b4a4_mask_shift = _mm_slli_epi64(b4a4, 32);
   __m128i result          = _mm_or_si128(ba_mask, b4a4_mask_shift);
#endif

   return result;
}


static inline void
transpose4_epi32(const __m128i * restrict a,
                 const __m128i * restrict b,
                 const __m128i * restrict c,
                 const __m128i * restrict d,
                 __m128i * restrict o,
                 __m128i * restrict p,
                 __m128i * restrict q,
                 __m128i * restrict r)
{
   __m128i t0 = _mm_unpacklo_epi32(*a, *b);
   __m128i t1 = _mm_unpacklo_epi32(*c, *d);
   __m128i t2 = _mm_unpackhi_epi32(*a, *b);
   __m128i t3 = _mm_unpackhi_epi32(*c, *d);

   *o = _mm_unpacklo_epi64(t0, t1);
   *p = _mm_unpackhi_epi64(t0, t1);
   *q = _mm_unpacklo_epi64(t2, t3);
   *r = _mm_unpackhi_epi64(t2, t3);
}


/*
 * Same as above, except the first two values are already interleaved
 * (i.e. contain 64bit values).
 */
static inline void
transpose2_64_2_32(const __m128i * restrict a01,
                   const __m128i * restrict a23,
                   const __m128i * restrict c,
                   const __m128i * restrict d,
                   __m128i * restrict o,
                   __m128i * restrict p,
                   __m128i * restrict q,
                   __m128i * restrict r)
{
   __m128i t0 = *a01;
   __m128i t1 = _mm_unpacklo_epi32(*c, *d);
   __m128i t2 = *a23;
   __m128i t3 = _mm_unpackhi_epi32(*c, *d);

   *o = _mm_unpacklo_epi64(t0, t1);
   *p = _mm_unpackhi_epi64(t0, t1);
   *q = _mm_unpacklo_epi64(t2, t3);
   *r = _mm_unpackhi_epi64(t2, t3);
}


#define SCALAR_EPI32(m, i) _mm_shuffle_epi32((m), _MM_SHUFFLE(i,i,i,i))


/*
 * Implements (1-w)*a + w*b = a - wa + wb = w(b-a) + a
 * ((b-a)*w >> 8) + a
 * The math behind negative sub results (logic shift/mask) is tricky.
 *
 * w -- weight values
 * a -- src0 values
 * b -- src1 values
 */
static ALWAYS_INLINE __m128i
util_sse2_lerp_epi16(__m128i w, __m128i a, __m128i b)
{
   __m128i res;

   res = _mm_sub_epi16(b, a);
   res = _mm_mullo_epi16(res, w);
   res = _mm_srli_epi16(res, 8);
   /* use add_epi8 instead of add_epi16 so no need to mask off upper bits */
   res = _mm_add_epi8(res, a);

   return res;
}


/* Apply premultiplied-alpha blending on two pixels simultaneously.
 * All parameters are packed as 8.8 fixed point values in __m128i SSE
 * registers, with the upper 8 bits all zero.
 *
 * a -- src alpha values
 * d -- dst color values
 * s -- src color values
 */
static inline __m128i
util_sse2_premul_blend_epi16( __m128i a, __m128i d, __m128i s)
{
   __m128i da, d_sub_da, tmp;
   tmp      = _mm_mullo_epi16(d, a);
   da       = _mm_srli_epi16(tmp, 8);
   d_sub_da = _mm_sub_epi16(d, da);

   return  _mm_add_epi16(s, d_sub_da);
}


/* Apply premultiplied-alpha blending on four pixels in packed BGRA
 * format (one/inv_src_alpha blend mode).
 *
 * src    -- four pixels (bgra8 format)
 * dst    -- four destination pixels (bgra8)
 * return -- blended pixels (bgra8)
 */
static ALWAYS_INLINE __m128i
util_sse2_blend_premul_4(const __m128i src,
                         const __m128i dst)
{

   __m128i al, ah, dl, dh, sl, sh, rl, rh;
   __m128i zero = _mm_setzero_si128();

   /* Blend first two pixels:
    */
   sl = _mm_unpacklo_epi8(src, zero);
   dl = _mm_unpacklo_epi8(dst, zero);

   al = _mm_shufflehi_epi16(sl, 0xff);
   al = _mm_shufflelo_epi16(al, 0xff);

   rl = util_sse2_premul_blend_epi16(al, dl, sl);

   /* Blend second two pixels:
    */
   sh = _mm_unpackhi_epi8(src, zero);
   dh = _mm_unpackhi_epi8(dst, zero);

   ah = _mm_shufflehi_epi16(sh, 0xff);
   ah = _mm_shufflelo_epi16(ah, 0xff);

   rh = util_sse2_premul_blend_epi16(ah, dh, sh);

   /* Pack the results down to four bgra8 pixels:
    */
   return _mm_packus_epi16(rl, rh);
}


/* Apply src-alpha blending on four pixels in packed BGRA
 * format (srcalpha/inv_src_alpha blend mode).
 *
 * src    -- four pixels (bgra8 format)
 * dst    -- four destination pixels (bgra8)
 * return -- blended pixels (bgra8)
 */
static ALWAYS_INLINE __m128i
util_sse2_blend_srcalpha_4(const __m128i src,
                           const __m128i dst)
{

   __m128i al, ah, dl, dh, sl, sh, rl, rh;
   __m128i zero = _mm_setzero_si128();

   /* Blend first two pixels:
    */
   sl = _mm_unpacklo_epi8(src, zero);
   dl = _mm_unpacklo_epi8(dst, zero);

   al = _mm_shufflehi_epi16(sl, 0xff);
   al = _mm_shufflelo_epi16(al, 0xff);

   rl = util_sse2_lerp_epi16(al, dl, sl);

   /* Blend second two pixels:
    */
   sh = _mm_unpackhi_epi8(src, zero);
   dh = _mm_unpackhi_epi8(dst, zero);

   ah = _mm_shufflehi_epi16(sh, 0xff);
   ah = _mm_shufflelo_epi16(ah, 0xff);

   rh = util_sse2_lerp_epi16(ah, dh, sh);

   /* Pack the results down to four bgra8 pixels:
    */
   return _mm_packus_epi16(rl, rh);
}


/**
 * premultiplies src with constant alpha then
 * does one/inv_src_alpha blend.
 *
 * src 16xi8 (normalized)
 * dst 16xi8 (normalized)
 * cst_alpha (constant alpha (u8 value))
 */
static ALWAYS_INLINE __m128i
util_sse2_blend_premul_src_4(const __m128i src,
                             const __m128i dst,
                             const unsigned cst_alpha)
{

   __m128i srca, d, s, rl, rh;
   __m128i zero = _mm_setzero_si128();
   __m128i cst_alpha_vec = _mm_set1_epi16(cst_alpha);

   /* Blend first two pixels:
    */
   s = _mm_unpacklo_epi8(src, zero);
   s = _mm_mullo_epi16(s, cst_alpha_vec);
   /* the shift will cause some precision loss */
   s = _mm_srli_epi16(s, 8);

   srca = _mm_shufflehi_epi16(s, 0xff);
   srca = _mm_shufflelo_epi16(srca, 0xff);

   d = _mm_unpacklo_epi8(dst, zero);
   rl = util_sse2_premul_blend_epi16(srca, d, s);

   /* Blend second two pixels:
    */
   s = _mm_unpackhi_epi8(src, zero);
   s = _mm_mullo_epi16(s, cst_alpha_vec);
   /* the shift will cause some precision loss */
   s = _mm_srli_epi16(s, 8);

   srca = _mm_shufflehi_epi16(s, 0xff);
   srca = _mm_shufflelo_epi16(srca, 0xff);

   d = _mm_unpackhi_epi8(dst, zero);
   rh = util_sse2_premul_blend_epi16(srca, d, s);

   /* Pack the results down to four bgra8 pixels:
    */
   return _mm_packus_epi16(rl, rh);
}


/**
 * Linear interpolation with SSE2.
 *
 * dst, src0, src1 are 16 x i8 vectors, with [0..255] normalized values.
 *
 * weight_lo and weight_hi should be a 8 x i16 vectors, in 8.8 fixed point
 * format, for the low and high components.
 * We'd want to pass these as values but MSVC limitation forces us to pass these
 * as pointers since it will complain if more than 3 __m128 are passed by value.
 */
static ALWAYS_INLINE __m128i
util_sse2_lerp_epi8_fixed88(__m128i src0, __m128i src1,
                            const __m128i * restrict weight_lo,
                            const __m128i * restrict weight_hi)
{
   const __m128i zero = _mm_setzero_si128();

   __m128i src0_lo = _mm_unpacklo_epi8(src0, zero);
   __m128i src0_hi = _mm_unpackhi_epi8(src0, zero);

   __m128i src1_lo = _mm_unpacklo_epi8(src1, zero);
   __m128i src1_hi = _mm_unpackhi_epi8(src1, zero);

   __m128i dst_lo;
   __m128i dst_hi;

   dst_lo = util_sse2_lerp_epi16(*weight_lo, src0_lo, src1_lo);
   dst_hi = util_sse2_lerp_epi16(*weight_hi, src0_hi, src1_hi);

   return _mm_packus_epi16(dst_lo, dst_hi);
}


/**
 * Linear interpolation with SSE2.
 *
 * dst, src0, src1 are 16 x i8 vectors, with [0..255] normalized values.
 *
 * weight should be a 16 x i8 vector, in 0.8 fixed point values.
 */
static ALWAYS_INLINE __m128i
util_sse2_lerp_epi8_fixed08(__m128i src0, __m128i src1,
                            __m128i weight)
{
   const __m128i zero = _mm_setzero_si128();
   __m128i weight_lo = _mm_unpacklo_epi8(weight, zero);
   __m128i weight_hi = _mm_unpackhi_epi8(weight, zero);

   return util_sse2_lerp_epi8_fixed88(src0, src1,
                                      &weight_lo, &weight_hi);
}


/**
 * Linear interpolation with SSE2.
 *
 * dst, src0, src1, and weight are 16 x i8 vectors, with [0..255] normalized
 * values.
 */
static ALWAYS_INLINE __m128i
util_sse2_lerp_unorm8(__m128i src0, __m128i src1,
                      __m128i weight)
{
   const __m128i zero = _mm_setzero_si128();
   __m128i weight_lo = _mm_unpacklo_epi8(weight, zero);
   __m128i weight_hi = _mm_unpackhi_epi8(weight, zero);

#if 0
   /*
    * Rescale from [0..255] to [0..256].
    */
   weight_lo = _mm_add_epi16(weight_lo, _mm_srli_epi16(weight_lo, 7));
   weight_hi = _mm_add_epi16(weight_hi, _mm_srli_epi16(weight_hi, 7));
#endif

   return util_sse2_lerp_epi8_fixed88(src0, src1,
                                      &weight_lo, &weight_hi);
}


/**
 * Linear interpolation with SSE2.
 *
 * dst, src0, src1, src2, src3 are 16 x i8 vectors, with [0..255] normalized
 * values.
 *
 * ws_lo, ws_hi, wt_lo, wt_hi should be a 8 x i16 vectors, in 8.8 fixed point
 * format, for the low and high components.
 * We'd want to pass these as values but MSVC limitation forces us to pass these
 * as pointers since it will complain if more than 3 __m128 are passed by value.
 *
 * This uses ws_lo, ws_hi to interpolate between src0 and src1, as well as to
 * interpolate between src2 and src3, then uses wt_lo and wt_hi to interpolate
 * between the resulting vectors.
 */
static ALWAYS_INLINE __m128i
util_sse2_lerp_2d_epi8_fixed88(__m128i src0, __m128i src1,
                               const __m128i * restrict src2,
                               const __m128i * restrict src3,
                               const __m128i * restrict ws_lo,
                               const __m128i * restrict ws_hi,
                               const __m128i * restrict wt_lo,
                               const __m128i * restrict wt_hi)
{
   const __m128i zero = _mm_setzero_si128();

   __m128i src0_lo = _mm_unpacklo_epi8(src0, zero);
   __m128i src0_hi = _mm_unpackhi_epi8(src0, zero);

   __m128i src1_lo = _mm_unpacklo_epi8(src1, zero);
   __m128i src1_hi = _mm_unpackhi_epi8(src1, zero);

   __m128i src2_lo = _mm_unpacklo_epi8(*src2, zero);
   __m128i src2_hi = _mm_unpackhi_epi8(*src2, zero);

   __m128i src3_lo = _mm_unpacklo_epi8(*src3, zero);
   __m128i src3_hi = _mm_unpackhi_epi8(*src3, zero);

   __m128i dst_lo, dst01_lo, dst23_lo;
   __m128i dst_hi, dst01_hi, dst23_hi;

   dst01_lo = util_sse2_lerp_epi16(*ws_lo, src0_lo, src1_lo);
   dst01_hi = util_sse2_lerp_epi16(*ws_hi, src0_hi, src1_hi);
   dst23_lo = util_sse2_lerp_epi16(*ws_lo, src2_lo, src3_lo);
   dst23_hi = util_sse2_lerp_epi16(*ws_hi, src2_hi, src3_hi);

   dst_lo = util_sse2_lerp_epi16(*wt_lo, dst01_lo, dst23_lo);
   dst_hi = util_sse2_lerp_epi16(*wt_hi, dst01_hi, dst23_hi);

   return _mm_packus_epi16(dst_lo, dst_hi);
}

/**
 * Stretch a row of pixels using linear filter.
 *
 * Uses Bresenham's line algorithm using 16.16 fixed point representation for
 * the error term.
 *
 * @param dst_width destination width in pixels
 * @param src_x    start x0 in 16.16 fixed point format
 * @param src_xstep step in 16.16. fixed point format
 *
 * @return final src_x value (i.e., src_x + dst_width*src_xstep)
 */
static ALWAYS_INLINE int32_t
util_sse2_stretch_row_8unorm(__m128i * restrict dst,
                             int32_t dst_width,
                             const uint32_t * restrict src,
                             int32_t src_x,
                             int32_t src_xstep)
{
   int16_t error0, error1, error2, error3;
   __m128i error_lo, error_hi, error_step;

   assert(dst_width >= 0);
   assert(dst_width % 4 == 0);

   error0 = src_x;
   error1 = error0 + src_xstep;
   error2 = error1 + src_xstep;
   error3 = error2 + src_xstep;

   error_lo   = _mm_setr_epi16(error0, error0, error0, error0,
                               error1, error1, error1, error1);
   error_hi   = _mm_setr_epi16(error2, error2, error2, error2,
                               error3, error3, error3, error3);
   error_step = _mm_set1_epi16(src_xstep << 2);

   dst_width >>= 2;
   while (dst_width) {
      uint16_t src_x0;
      uint16_t src_x1;
      uint16_t src_x2;
      uint16_t src_x3;
      __m128i src0, src1;
      __m128i weight_lo, weight_hi;

      /*
       * It is faster to re-compute the coordinates in the scalar integer unit here,
       * than to fetch the values from the SIMD integer unit.
       */

      src_x0 = src_x >> 16;
      src_x += src_xstep;
      src_x1 = src_x >> 16;
      src_x += src_xstep;
      src_x2 = src_x >> 16;
      src_x += src_xstep;
      src_x3 = src_x >> 16;
      src_x += src_xstep;

      /*
       * Fetch pairs of pixels 64bit at a time, and then swizzle them inplace.
       */

      {
         __m128i src_00_10 = _mm_loadl_epi64((const __m128i *)&src[src_x0]);
         __m128i src_01_11 = _mm_loadl_epi64((const __m128i *)&src[src_x1]);
         __m128i src_02_12 = _mm_loadl_epi64((const __m128i *)&src[src_x2]);
         __m128i src_03_13 = _mm_loadl_epi64((const __m128i *)&src[src_x3]);

         __m128i src_00_01_10_11 = _mm_unpacklo_epi32(src_00_10, src_01_11);
         __m128i src_02_03_12_13 = _mm_unpacklo_epi32(src_02_12, src_03_13);

         src0 = _mm_unpacklo_epi64(src_00_01_10_11, src_02_03_12_13);
         src1 = _mm_unpackhi_epi64(src_00_01_10_11, src_02_03_12_13);
      }

      weight_lo = _mm_srli_epi16(error_lo, 8);
      weight_hi = _mm_srli_epi16(error_hi, 8);

      *dst = util_sse2_lerp_epi8_fixed88(src0, src1,
                                         &weight_lo, &weight_hi);

      error_lo = _mm_add_epi16(error_lo, error_step);
      error_hi = _mm_add_epi16(error_hi, error_step);

      ++dst;
      --dst_width;
   }

   return src_x;
}



#endif /* DETECT_ARCH_SSE */

#endif /* U_SSE_H_ */
