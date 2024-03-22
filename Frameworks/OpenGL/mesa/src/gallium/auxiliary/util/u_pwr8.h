/*
 * Copyright 2015 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author: Oded Gabbay <oded.gabbay@redhat.com>
 */

/**
 * @file
 * POWER8 intrinsics portability header.
 *
 */

#ifndef U_PWR8_H_
#define U_PWR8_H_

#if defined(_ARCH_PWR8) && UTIL_ARCH_LITTLE_ENDIAN

#define VECTOR_ALIGN_16 __attribute__ ((__aligned__ (16)))

typedef VECTOR_ALIGN_16 vector unsigned char __m128i;

typedef VECTOR_ALIGN_16 union m128i {
   __m128i m128i;
   vector signed int m128si;
   vector unsigned int m128ui;
   uint8_t ub[16];
   uint16_t us[8];
   int32_t i[4];
   uint32_t ui[4];
} __m128i_union;

static inline __m128i
vec_set_epi32 (int i3, int i2, int i1, int i0)
{
   __m128i_union vdst;

#if UTIL_ARCH_LITTLE_ENDIAN
   vdst.i[0] = i0;
   vdst.i[1] = i1;
   vdst.i[2] = i2;
   vdst.i[3] = i3;
#else
   vdst.i[3] = i0;
   vdst.i[2] = i1;
   vdst.i[1] = i2;
   vdst.i[0] = i3;
#endif

   return (__m128i) vdst.m128si;
}

static inline __m128i
vec_setr_epi32 (int i0, int i1, int i2, int i3)
{
  return vec_set_epi32 (i3, i2, i1, i0);
}

static inline __m128i
vec_unpacklo_epi32 (__m128i even, __m128i odd)
{
   static const __m128i perm_mask =
#if UTIL_ARCH_LITTLE_ENDIAN
      { 0,  1,  2,  3, 16, 17, 18, 19,  4,  5,  6,  7, 20, 21, 22, 23};
#else
      {24, 25, 26, 27,  8,  9, 10, 11, 28, 29, 30, 31, 12, 13, 14, 15};
#endif

   return vec_perm (even, odd, perm_mask);
}

static inline __m128i
vec_unpackhi_epi32 (__m128i even, __m128i odd)
{
   static const __m128i perm_mask =
#if UTIL_ARCH_LITTLE_ENDIAN
      { 8,  9, 10, 11, 24, 25, 26, 27, 12, 13, 14, 15, 28, 29, 30, 31};
#else
      {16, 17, 18, 19,  0,  1,  2,  3, 20, 21, 22, 23,  4,  5,  6,  7};
#endif

   return vec_perm (even, odd, perm_mask);
}

static inline __m128i
vec_unpacklo_epi64 (__m128i even, __m128i odd)
{
   static const __m128i perm_mask =
#if UTIL_ARCH_LITTLE_ENDIAN
      { 0,  1,  2,  3,  4,  5,  6,  7, 16, 17, 18, 19, 20, 21, 22, 23};
#else
      {24, 25, 26, 27, 28, 29, 30, 31,  8,  9, 10, 11, 12, 13, 14, 15};
#endif

   return vec_perm (even, odd, perm_mask);
}

static inline __m128i
vec_unpackhi_epi64 (__m128i even, __m128i odd)
{
   static const __m128i perm_mask =
#if UTIL_ARCH_LITTLE_ENDIAN
      { 8,  9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31};
#else
      {16, 17, 18, 19, 20, 21, 22, 23,  0,  1,  2,  3,  4,  5,  6,  7};
#endif

   return vec_perm (even, odd, perm_mask);
}

static inline __m128i
vec_add_epi32 (__m128i a, __m128i b)
{
   return (__m128i) vec_add ((vector signed int) a, (vector signed int) b);
}

static inline __m128i
vec_sub_epi32 (__m128i a, __m128i b)
{
   return (__m128i) vec_sub ((vector signed int) a, (vector signed int) b);
}

/* Call this function ONLY on POWER8 and newer platforms */
static inline __m128i
vec_mullo_epi32 (__m128i a, __m128i b)
{
   __m128i v;

   __asm__(
           "vmuluwm %0, %1, %2   \n"
           : "=v" (v)
           : "v" (a), "v" (b)
           );

   return v;
}

static inline __m128i
vec_andnot_si128 (__m128i a, __m128i b)
{
   return vec_andc (b, a);
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
   __m128i t0 = vec_unpacklo_epi32(*a, *b);
   __m128i t1 = vec_unpacklo_epi32(*c, *d);
   __m128i t2 = vec_unpackhi_epi32(*a, *b);
   __m128i t3 = vec_unpackhi_epi32(*c, *d);

   *o = vec_unpacklo_epi64(t0, t1);
   *p = vec_unpackhi_epi64(t0, t1);
   *q = vec_unpacklo_epi64(t2, t3);
   *r = vec_unpackhi_epi64(t2, t3);
}

static inline __m128i
vec_slli_epi32 (__m128i vsrc, unsigned int count)
{
   __m128i_union vec_count;

   if (count >= 32)
      return (__m128i) vec_splats (0);
   else if (count == 0)
      return vsrc;

   /* In VMX, all shift count fields must contain the same value */
   vec_count.m128si = (vector signed int) vec_splats (count);
   return (__m128i) vec_sl ((vector signed int) vsrc, vec_count.m128ui);
}

static inline __m128i
vec_srli_epi32 (__m128i vsrc, unsigned int count)
{
   __m128i_union vec_count;

   if (count >= 32)
      return (__m128i) vec_splats (0);
   else if (count == 0)
      return vsrc;

   /* In VMX, all shift count fields must contain the same value */
   vec_count.m128si = (vector signed int) vec_splats (count);
   return (__m128i) vec_sr ((vector signed int) vsrc, vec_count.m128ui);
}

static inline __m128i
vec_srai_epi32 (__m128i vsrc, unsigned int count)
{
   __m128i_union vec_count;

   if (count >= 32)
      return (__m128i) vec_splats (0);
   else if (count == 0)
      return vsrc;

   /* In VMX, all shift count fields must contain the same value */
   vec_count.m128si = (vector signed int) vec_splats (count);
   return (__m128i) vec_sra ((vector signed int) vsrc, vec_count.m128ui);
}

static inline __m128i
vec_cmpeq_epi32 (__m128i a, __m128i b)
{
   return (__m128i) vec_cmpeq ((vector signed int) a, (vector signed int) b);
}

static inline __m128i
vec_loadu_si128 (const uint32_t* src)
{
   __m128i_union vsrc;

#if UTIL_ARCH_LITTLE_ENDIAN

   vsrc.m128ui = *((vector unsigned int *) src);

#else

   __m128i vmask, tmp1, tmp2;

   vmask = vec_lvsl(0, src);

   tmp1 = (__m128i) vec_ld (0, src);
   tmp2 = (__m128i) vec_ld (15, src);
   vsrc.m128ui = (vector unsigned int) vec_perm (tmp1, tmp2, vmask);

#endif

   return vsrc.m128i;
}

static inline __m128i
vec_load_si128 (const uint32_t* src)
{
   __m128i_union vsrc;

   vsrc.m128ui = *((vector unsigned int *) src);

   return vsrc.m128i;
}

static inline void
vec_store_si128 (uint32_t* dest, __m128i vdata)
{
   vec_st ((vector unsigned int) vdata, 0, dest);
}

/* Call this function ONLY on POWER8 and newer platforms */
static inline int
vec_movemask_epi8 (__m128i vsrc)
{
   __m128i_union vtemp;
   int result;

   vtemp.m128i = vec_vgbbd(vsrc);

#if UTIL_ARCH_LITTLE_ENDIAN
   result = vtemp.ub[15] << 8 | vtemp.ub[7];
#else
   result = vtemp.ub[0] << 8 | vtemp.ub[8];
#endif

   return result;
}

static inline __m128i
vec_packs_epi16 (__m128i a, __m128i b)
{
#if UTIL_ARCH_LITTLE_ENDIAN
   return (__m128i) vec_packs ((vector signed short) a,
                               (vector signed short) b);
#else
   return (__m128i) vec_packs ((vector signed short) b,
                               (vector signed short) a);
#endif
}

static inline __m128i
vec_packs_epi32 (__m128i a, __m128i b)
{
#if UTIL_ARCH_LITTLE_ENDIAN
   return (__m128i) vec_packs ((vector signed int) a, (vector signed int) b);
#else
   return (__m128i) vec_packs ((vector signed int) b, (vector signed int) a);
#endif
}

#endif /* _ARCH_PWR8 && UTIL_ARCH_LITTLE_ENDIAN */

#endif /* U_PWR8_H_ */
