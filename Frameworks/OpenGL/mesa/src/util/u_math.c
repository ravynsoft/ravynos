/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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



#include "util/detect.h"
#include "util/u_math.h"
#include "util/u_cpu_detect.h"

#if DETECT_ARCH_SSE
#include <xmmintrin.h>
/* This is defined in pmmintrin.h, but it can only be included when -msse3 is
 * used, so just define it here to avoid further. */
#ifndef _MM_DENORMALS_ZERO_MASK
#define _MM_DENORMALS_ZERO_MASK	0x0040
#endif
#endif


/** log2(x), for x in [1.0, 2.0) */
float log2_table[LOG2_TABLE_SIZE];


static void
init_log2_table(void)
{
   unsigned i;
   for (i = 0; i < LOG2_TABLE_SIZE; i++)
      log2_table[i] = (float) log2(1.0 + i * (1.0 / LOG2_TABLE_SCALE));
}


/**
 * One time init for math utilities.
 */
void
util_init_math(void)
{
   static bool initialized = false;
   if (!initialized) {
      init_log2_table();
      initialized = true;
   }
}

/**
 * Fetches the contents of the fpstate (mxcsr on x86) register.
 *
 * On platforms without support for it just returns 0.
 */
unsigned
util_fpstate_get(void)
{
   unsigned mxcsr = 0;

#if DETECT_ARCH_SSE
   if (util_get_cpu_caps()->has_sse) {
      mxcsr = _mm_getcsr();
   }
#endif

   return mxcsr;
}

/**
 * Make sure that the fp treats the denormalized floating
 * point numbers as zero.
 *
 * This is the behavior required by D3D10. OpenGL doesn't care.
 */
unsigned
util_fpstate_set_denorms_to_zero(unsigned current_mxcsr)
{
#if DETECT_ARCH_SSE
   if (util_get_cpu_caps()->has_sse) {
      /* Enable flush to zero mode */
      current_mxcsr |= _MM_FLUSH_ZERO_MASK;
      if (util_get_cpu_caps()->has_daz) {
         /* Enable denormals are zero mode */
         current_mxcsr |= _MM_DENORMALS_ZERO_MASK;
      }
      util_fpstate_set(current_mxcsr);
   }
#endif
   return current_mxcsr;
}

/**
 * Set the state of the fpstate (mxcsr on x86) register.
 *
 * On platforms without support for it's a noop.
 */
void
util_fpstate_set(unsigned mxcsr)
{
#if DETECT_ARCH_SSE
   if (util_get_cpu_caps()->has_sse) {
      _mm_setcsr(mxcsr);
   }
#endif
}

/**
 * Compute inverse of 4x4 matrix.
 *
 * \return false if the source matrix is singular.
 *
 * \author
 * Code contributed by Jacques Leroy jle@star.be
 *
 * Calculates the inverse matrix by performing the gaussian matrix reduction
 * with partial pivoting followed by back/substitution with the loops manually
 * unrolled.
 */
bool
util_invert_mat4x4(float *out, const float *m)
{
   float wtmp[4][8];
   float m0, m1, m2, m3, s;
   float *r0, *r1, *r2, *r3;

#define MAT(m, r, c) (m)[(c)*4 + (r)]
#define SWAP_ROWS(a, b)                                                                            \
   {                                                                                               \
      float *_tmp = a;                                                                             \
      (a) = (b);                                                                                   \
      (b) = _tmp;                                                                                  \
   }

   r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

   r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1), r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
   r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

   r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1), r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
   r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

   r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1), r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
   r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

   r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1), r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
   r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

   /* choose pivot - or die */
   if (fabsf(r3[0]) > fabsf(r2[0]))
      SWAP_ROWS(r3, r2);
   if (fabsf(r2[0]) > fabsf(r1[0]))
      SWAP_ROWS(r2, r1);
   if (fabsf(r1[0]) > fabsf(r0[0]))
      SWAP_ROWS(r1, r0);
   if (0.0F == r0[0])
      return false;

   /* eliminate first variable     */
   m1 = r1[0] / r0[0];
   m2 = r2[0] / r0[0];
   m3 = r3[0] / r0[0];
   s = r0[1];
   r1[1] -= m1 * s;
   r2[1] -= m2 * s;
   r3[1] -= m3 * s;
   s = r0[2];
   r1[2] -= m1 * s;
   r2[2] -= m2 * s;
   r3[2] -= m3 * s;
   s = r0[3];
   r1[3] -= m1 * s;
   r2[3] -= m2 * s;
   r3[3] -= m3 * s;
   s = r0[4];
   if (s != 0.0F) {
      r1[4] -= m1 * s;
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r0[5];
   if (s != 0.0F) {
      r1[5] -= m1 * s;
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r0[6];
   if (s != 0.0F) {
      r1[6] -= m1 * s;
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r0[7];
   if (s != 0.0F) {
      r1[7] -= m1 * s;
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }

   /* choose pivot - or die */
   if (fabsf(r3[1]) > fabsf(r2[1]))
      SWAP_ROWS(r3, r2);
   if (fabsf(r2[1]) > fabsf(r1[1]))
      SWAP_ROWS(r2, r1);
   if (0.0F == r1[1])
      return false;

   /* eliminate second variable */
   m2 = r2[1] / r1[1];
   m3 = r3[1] / r1[1];
   r2[2] -= m2 * r1[2];
   r3[2] -= m3 * r1[2];
   r2[3] -= m2 * r1[3];
   r3[3] -= m3 * r1[3];
   s = r1[4];
   if (0.0F != s) {
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r1[5];
   if (0.0F != s) {
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r1[6];
   if (0.0F != s) {
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r1[7];
   if (0.0F != s) {
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }

   /* choose pivot - or die */
   if (fabsf(r3[2]) > fabsf(r2[2]))
      SWAP_ROWS(r3, r2);
   if (0.0F == r2[2])
      return false;

   /* eliminate third variable */
   m3 = r3[2] / r2[2];
   r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4], r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
      r3[7] -= m3 * r2[7];

   /* last check */
   if (0.0F == r3[3])
      return false;

   s = 1.0F / r3[3]; /* now back substitute row 3 */
   r3[4] *= s;
   r3[5] *= s;
   r3[6] *= s;
   r3[7] *= s;

   m2 = r2[3]; /* now back substitute row 2 */
   s = 1.0F / r2[2];
   r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
   r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
   m1 = r1[3];
   r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1, r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
   m0 = r0[3];
   r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0, r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

   m1 = r1[2]; /* now back substitute row 1 */
   s = 1.0F / r1[1];
   r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
   r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
   m0 = r0[2];
   r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0, r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

   m0 = r0[1]; /* now back substitute row 0 */
   s = 1.0F / r0[0];
   r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
   r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

   MAT(out, 0, 0) = r0[4];
   MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
   MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
   MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
   MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
   MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
   MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
   MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
   MAT(out, 3, 3) = r3[7];

#undef MAT
#undef SWAP_ROWS

   return true;
}
