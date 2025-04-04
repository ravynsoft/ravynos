/*
 * Header for sinf, cosf and sincosf.
 *
 * Copyright (c) 2018-2024, Arm Limited.
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <stdint.h>
#include <math.h>
#include "math_config.h"

/* 2PI * 2^-64.  */
static const double pi63 = 0x1.921FB54442D18p-62;
/* PI / 4.  */
static const float pio4f = 0x1.921FB6p-1f;

/* The constants and polynomials for sine and cosine.  */
typedef struct
{
  double sign[4];		/* Sign of sine in quadrants 0..3.  */
  double hpi_inv;		/* 2 / PI ( * 2^24 if !TOINT_INTRINSICS).  */
  double hpi;			/* PI / 2.  */
  double c0, c1, c2, c3, c4;	/* Cosine polynomial.  */
  double s1, s2, s3;		/* Sine polynomial.  */
} sincos_t;

/* Polynomial data (the cosine polynomial is negated in the 2nd entry).  */
extern const sincos_t __sincosf_table[2] HIDDEN;

/* Top 12 bits of the float representation with the sign bit cleared.  */
static inline uint32_t
abstop12 (float x)
{
  return (asuint (x) >> 20) & 0x7ff;
}

/* Compute the sine and cosine of inputs X and X2 (X squared), using the
   polynomial P and store the results in SINP and COSP.  N is the quadrant,
   if odd the cosine and sine polynomials are swapped.  */
static inline void
sincosf_poly (double x, double x2, const sincos_t *p, int n, float *sinp,
	      float *cosp)
{
  double x3, x4, x5, x6, s, c, c1, c2, s1;

  x4 = x2 * x2;
  x3 = x2 * x;
  c2 = p->c3 + x2 * p->c4;
  s1 = p->s2 + x2 * p->s3;

  /* Swap sin/cos result based on quadrant.  */
  float *tmp = (n & 1 ? cosp : sinp);
  cosp = (n & 1 ? sinp : cosp);
  sinp = tmp;

  c1 = p->c0 + x2 * p->c1;
  x5 = x3 * x2;
  x6 = x4 * x2;

  s = x + x3 * p->s1;
  c = c1 + x4 * p->c2;

  *sinp = s + x5 * s1;
  *cosp = c + x6 * c2;
}

/* Return the sine of inputs X and X2 (X squared) using the polynomial P.
   N is the quadrant, and if odd the cosine polynomial is used.  */
static inline float
sinf_poly (double x, double x2, const sincos_t *p, int n)
{
  double x3, x4, x6, x7, s, c, c1, c2, s1;

  if ((n & 1) == 0)
    {
      x3 = x * x2;
      s1 = p->s2 + x2 * p->s3;

      x7 = x3 * x2;
      s = x + x3 * p->s1;

      return s + x7 * s1;
    }
  else
    {
      x4 = x2 * x2;
      c2 = p->c3 + x2 * p->c4;
      c1 = p->c0 + x2 * p->c1;

      x6 = x4 * x2;
      c = c1 + x4 * p->c2;

      return c + x6 * c2;
    }
}

/* Fast range reduction using single multiply-subtract.  Return the modulo of
   X as a value between -PI/4 and PI/4 and store the quadrant in NP.
   The values for PI/2 and 2/PI are accessed via P.  Since PI/2 as a double
   is accurate to 55 bits and the worst-case cancellation happens at 6 * PI/4,
   the result is accurate for |X| <= 120.0.  */
static inline double
reduce_fast (double x, const sincos_t *p, int *np)
{
  double r;
#if TOINT_INTRINSICS
  /* Use fast round and lround instructions when available.  */
  r = x * p->hpi_inv;
  *np = converttoint (r);
  return x - roundtoint (r) * p->hpi;
#else
  /* Use scaled float to int conversion with explicit rounding.
     hpi_inv is prescaled by 2^24 so the quadrant ends up in bits 24..31.
     This avoids inaccuracies introduced by truncating negative values.  */
  r = x * p->hpi_inv;
  int n = ((int32_t)r + 0x800000) >> 24;
  *np = n;
  return x - n * p->hpi;
#endif
}

/* Reduce the range of XI to a multiple of PI/2 using fast integer arithmetic.
   XI is a reinterpreted float and must be >= 2.0f (the sign bit is ignored).
   Return the modulo between -PI/4 and PI/4 and store the quadrant in NP.
   Reduction uses a table of 4/PI with 192 bits of precision.  A 32x96->128 bit
   multiply computes the exact 2.62-bit fixed-point modulo.  Since the result
   can have at most 29 leading zeros after the binary point, the double
   precision result is accurate to 33 bits.  */
static inline double
reduce_large (uint32_t xi, int *np)
{
  const uint32_t *arr = &__inv_pio4[(xi >> 26) & 15];
  int shift = (xi >> 23) & 7;
  uint64_t n, res0, res1, res2;

  xi = (xi & 0xffffff) | 0x800000;
  xi <<= shift;

  res0 = xi * arr[0];
  res1 = (uint64_t)xi * arr[4];
  res2 = (uint64_t)xi * arr[8];
  res0 = (res2 >> 32) | (res0 << 32);
  res0 += res1;

  n = (res0 + (1ULL << 61)) >> 62;
  res0 -= n << 62;
  double x = (int64_t)res0;
  *np = n;
  return x * pi63;
}
