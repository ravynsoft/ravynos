/*
 * Minimal math.h for ravynOS
 * Copyright (C) 2026 Zoe Knox <zoe@pixin.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _MATH_H_
#define _MATH_H_

#include <limits.h>
#include <stdlib.h>

#if FLT_EVAL_METHOD == 1
  typedef double float_t;
  typedef double double_t;
#else
  #if FLT_EVAL_METHOD == 2
    typedef long double float_t;
    typedef long double double_t;
  #else
    typedef float float_t;
    typedef double double_t;
  #endif
#endif

/* Make sure these stay compatible with macOS */
#define FP_NAN          1
#define FP_INFINITE     2
#define FP_ZERO         3
#define FP_NORMAL       4
#define FP_SUBNORMAL    5
#define FP_SUPERNORMAL  6

#define HUGE_VAL    __builtin_huge_val()
#define HUGE_VALF   __builtin_huge_valf()
#define HUGE_VALL   __builtin_huge_vall()
#define NAN         __builtin_nanf("0x7fc00000")
#define INFINITY    HUGE_VALF

#define M_PI        3.14159265358979323846

#ifndef __cplusplus
#ifndef bool
typedef _Bool bool;
#endif
#endif

#ifndef __cplusplus
extern bool signbit(double __x);
extern int fpclassify(double __x);
extern bool isfinite(double __x);
extern bool isinf(double __x);
extern bool isnan(double __x);
extern bool isnormal(double __x);
extern bool isgreater(double __x, double __y);
extern bool isgreaterequal(double __x, double __y);
extern bool isless(double __x, double __y);
extern bool islessequal(double __x, double __y);
extern bool islessgreater(double __x, double __y);
extern bool isunordered(double __x, double __y);

extern float acosf(float __x);
extern double acos(double __x);
extern long double acosl(long double __x);

extern float asinf(float __x);
extern double asin(double __x);
extern long double asinl(long double __x);

extern float atanf(float __x);
extern double atan(double __x);
extern long double atanl(long double __x);

extern float atan2f(float __y, float __x);
extern double atan2(double __x, double __y);
extern long double atan2l(long double __y, long double __x);

extern float ceilf(float __x);
extern double ceil(double __x);
extern long double ceill(long double __x);

extern float cosf(float __x);
extern double cos(double __x);
extern long double cosl(long double __x);

extern float coshf(float __x);
extern double cosh(double __x);
extern long double coshl(long double __x);

extern float expf(float __x);
extern double exp(double __x);
extern long double expl(long double __x);

extern float fabsf(float __x);
extern double fabs(double __x);
extern long double fabsl(long double __x);

extern float floorf(float __x);
extern double floor(double __x);
extern long double floorl(long double __x);

extern float fmodf(float __x, float __y);
extern double fmod(double __x, double __y);
extern long double fmodl(long double __x, long double __y);

extern float frexpf(float __x, int* __e);
extern double frexp(double __x, int* __e);
extern long double frexpl(long double __x, int* __e);

extern float ldexpf(float __x, int __e);
extern double ldexp(double __x, int __e);
extern long double ldexpl(long double __x, int __e);

extern float logf(float __x);
extern double log(double __x);
extern long double logl(long double __x);

extern float log10f(float __x);
extern double log10(double __x);
extern long double log10l(long double __x);

extern float modff(float __x, float* __y);
extern double modf(double __x, double* __y);
extern long double modfl(long double __x, long double* __y);

extern float powf(float __x, float __y);
extern double pow(double __x, double __y);
extern long double powl(long double __x, long double __y);

extern float sinf(float __x);
extern double sin(double __x);
extern long double sinl(long double __x);

extern float sinhf(float __x);
extern double sinh(double __x);
extern long double sinhl(long double __x);

extern float sqrtf(float __x);
extern double sqrt(double __x);
extern long double sqrtl(long double __x);

extern float tanf(float __x);
extern double tan(double __x);
extern long double tanl(long double __x);

extern float tanhf(float __x);
extern double tanh(double __x);
extern long double tanhl(long double __x);

extern float acoshf(float __x);
extern double acosh(double __x);
extern long double acoshl(long double __x);

extern float asinhf(float __x);
extern double asinh(double __x);
extern long double asinhl(long double __x);

extern float atanhf(float __x);
extern double atanh(double __x);
extern long double atanhl(long double __x);

extern float cbrtf(float __x);
extern double cbrt(double __x);
extern long double cbrtl(long double __x);

extern double copysignf(float __x, float __y);
extern double copysign(double __x, double __y);
extern long double copysignl(long double __x, long double __y);

extern float erff(float __x);
extern double erf(double __x);
extern long double erfl(long double __x);

extern float erfcf(float __x);
extern double erfc(double __x);
extern long double erfcl(long double __x);

extern float exp2f(float __x);
extern double exp2(double __x);
extern long double exp2l(long double __x);

extern float expm1f(float __x);
extern double expm1(double __x);
extern long double expm1l(long double __x);

extern float fdimf(float __x, float __y);
extern double fdim(double __x, double __y);
extern long double fdiml(long double __x, long double __y);

extern float fmaf(float __x, float __y, float __z);
extern double fma(double __x, double __y, double __z);
extern long double fmal(long double __x, long double __y, long double __z);

extern float fmaxf(float __x, float __y);
extern double fmax(double __x, double __y);
extern long double fmaxl(long double __x, long double __y);

extern float fminf(float __x, float __y);
extern double fmin(double __x, double __y);
extern long double fminl(long double __x, long double __y);

extern float hypotf(float __x, float __y);
extern double hypot(double __x, double __y);
extern long double hypotl(long double __x, long double __y);

extern int ilogbf(float __x);
extern double ilogb(double __x);
extern int ilogbl(long double __x);

extern float lgammaf(float __x);
extern double lgamma(double __x);
extern long double lgammal(long double __x);

extern long long llrintf(float __x);
extern long long llrint(double __x);
extern long long llrintl(long double __x);

extern long long llroundf(float __x);
extern long long llround(double __x);
extern long long llroundl(long double __x);

extern float log1pf(float __x);
extern double log1p(double __x);
extern long double log1pl(long double __x);

extern float log2f(float __x);
extern double log2(double __x);
extern long double log2l(long double __x);

extern float logbf(float __x);
extern double logb(double __x);
extern long double logbl(long double __x);

extern long lrintf(float __x);
extern long lrint(double __x);
extern long lrintl(long double __x);

extern long lroundf(float __x);
extern long lround(double __x);
extern long lroundl(long double __x);

float nanf(const char * __s);
double nan(const char * __s);
long double nanl(const char * __s);

extern  float nearbyintf(float __x);
extern double nearbyint(double __x);
extern long double nearbyintl(long double __x);

extern float nextafterf(float __x, float __y);
extern double nextafter(double __x, double __y);
extern long double nextafterl(long double __x, long double __y);

extern float nexttowardf(float __x, long double __y);
extern double nexttoward(double __x, long double __y);
extern long double nexttowardl(long double __x, long double __y);

extern float remainderf(float __x, float __y);
extern double remainder(double __x, double __y);
extern long double remainderl(long double __x, long double __y);

extern float remquof(float __x, float __y, int* __z);
extern double remquo(double __x, double __y, int* __z);
extern long double remquol(long double __x, long double __y, int* __z);

extern float rintf(float __x);
extern double rint(double __x);
extern long double rintl(long double __x);

extern float roundf(float __x);
extern double round(double __x);
extern long double roundl(long double __x);

extern float scalblnf(float __x, long __y);
extern double scalbln(double __x, long __y);
extern long double scalblnl(long double __x, long __y);

extern float scalbnf(float __x, int __y);
extern double scalbn(double __x, int __y);
extern long double scalbnl(long double __x, int __y);

extern float tgammaf(float __x);
extern double tgamma(double __x);
extern long double tgammal(long double __x);

extern float truncf(float __x);
extern double trunc(double __x);
extern long double truncl(long double __x);

#endif /* __cplusplus */
#endif /* _MATH_H_ */
