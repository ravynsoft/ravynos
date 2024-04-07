#define PERL_EXT_POSIX
#define PERL_EXT

#if defined(_WIN32) && defined(__GNUC__) /* mingw compiler */
#define _POSIX_
#endif
#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#define PERLIO_NOT_STDIO 1
#include "perl.h"
#include "XSUB.h"

static int not_here(const char *s);

#if defined(PERL_IMPLICIT_SYS)
#  undef signal
#  undef open
#  undef setmode
#  define open PerlLIO_open3
#endif
#include <ctype.h>
#ifdef I_DIRENT    /* XXX maybe better to just rely on perl.h? */
#include <dirent.h>
#endif
#include <errno.h>
#ifdef WIN32
#include <sys/errno2.h>
#endif
#include <float.h>
#ifdef I_FENV
#if !(defined(__vax__) && defined(__NetBSD__))
#include <fenv.h>
#endif
#endif
#include <limits.h>
#include <locale.h>
#include <math.h>
#ifdef I_PWD
#include <pwd.h>
#endif
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>

#ifdef I_UNISTD
#include <unistd.h>
#endif

#ifdef I_SYS_TIME
# include <sys/time.h>
#endif

#ifdef I_SYS_RESOURCE
# include <sys/resource.h>
#endif

/* Cygwin's stdio.h doesn't make cuserid() visible with -D_GNU_SOURCE,
   unlike Linux.
*/
#ifdef __CYGWIN__
# undef HAS_CUSERID
#endif

#if defined(USE_QUADMATH) && defined(I_QUADMATH)

#  undef M_E
#  undef M_LOG2E
#  undef M_LOG10E
#  undef M_LN2
#  undef M_LN10
#  undef M_PI
#  undef M_PI_2
#  undef M_PI_4
#  undef M_1_PI
#  undef M_2_PI
#  undef M_2_SQRTPI
#  undef M_SQRT2
#  undef M_SQRT1_2

#  define M_E        M_Eq
#  define M_LOG2E    M_LOG2Eq
#  define M_LOG10E   M_LOG10Eq
#  define M_LN2      M_LN2q
#  define M_LN10     M_LN10q
#  define M_PI       M_PIq
#  define M_PI_2     M_PI_2q
#  define M_PI_4     M_PI_4q
#  define M_1_PI     M_1_PIq
#  define M_2_PI     M_2_PIq
#  define M_2_SQRTPI M_2_SQRTPIq
#  define M_SQRT2    M_SQRT2q
#  define M_SQRT1_2  M_SQRT1_2q

#else

#  ifdef USE_LONG_DOUBLE
#    undef M_E
#    undef M_LOG2E
#    undef M_LOG10E
#    undef M_LN2
#    undef M_LN10
#    undef M_PI
#    undef M_PI_2
#    undef M_PI_4
#    undef M_1_PI
#    undef M_2_PI
#    undef M_2_SQRTPI
#    undef M_SQRT2
#    undef M_SQRT1_2
#    define FLOAT_C(c) CAT2(c,L)
#  else
#    define FLOAT_C(c) (c)
#  endif

#  ifndef M_E
#    define M_E		FLOAT_C(2.71828182845904523536028747135266250)
#  endif
#  ifndef M_LOG2E
#    define M_LOG2E	FLOAT_C(1.44269504088896340735992468100189214)
#  endif
#  ifndef M_LOG10E
#    define M_LOG10E	FLOAT_C(0.434294481903251827651128918916605082)
#  endif
#  ifndef M_LN2
#    define M_LN2	FLOAT_C(0.693147180559945309417232121458176568)
#  endif
#  ifndef M_LN10
#    define M_LN10	FLOAT_C(2.30258509299404568401799145468436421)
#  endif
#  ifndef M_PI
#    define M_PI	FLOAT_C(3.14159265358979323846264338327950288)
#  endif
#  ifndef M_PI_2
#    define M_PI_2	FLOAT_C(1.57079632679489661923132169163975144)
#  endif
#  ifndef M_PI_4
#    define M_PI_4	FLOAT_C(0.785398163397448309615660845819875721)
#  endif
#  ifndef M_1_PI
#    define M_1_PI	FLOAT_C(0.318309886183790671537767526745028724)
#  endif
#  ifndef M_2_PI
#    define M_2_PI	FLOAT_C(0.636619772367581343075535053490057448)
#  endif
#  ifndef M_2_SQRTPI
#    define M_2_SQRTPI	FLOAT_C(1.12837916709551257389615890312154517)
#  endif
#  ifndef M_SQRT2
#    define M_SQRT2	FLOAT_C(1.41421356237309504880168872420969808)
#  endif
#  ifndef M_SQRT1_2
#    define M_SQRT1_2	FLOAT_C(0.707106781186547524400844362104849039)
#  endif

#endif

#if !defined(INFINITY) && defined(NV_INF)
#  define INFINITY NV_INF
#endif

#if !defined(NAN) && defined(NV_NAN)
#  define NAN NV_NAN
#endif

#if !defined(Inf) && defined(NV_INF)
#  define Inf NV_INF
#endif

#if !defined(NaN) && defined(NV_NAN)
#  define NaN NV_NAN
#endif

/* We will have an emulation. */
#ifndef FP_INFINITE
#  define FP_INFINITE	0
#  define FP_NAN	1
#  define FP_NORMAL	2
#  define FP_SUBNORMAL	3
#  define FP_ZERO	4
#endif

/* We will have an emulation. */
#ifndef FE_TONEAREST
#  define FE_TOWARDZERO	0
#  define FE_TONEAREST	1
#  define FE_UPWARD	2
#  define FE_DOWNWARD	3
#endif

/* C89 math.h:

   acos asin atan atan2 ceil cos cosh exp fabs floor fmod frexp ldexp
   log log10 modf pow sin sinh sqrt tan tanh

 * Implemented in core:

   atan2 cos exp log pow sin sqrt

 * C99 math.h added:

   acosh asinh atanh cbrt copysign erf erfc exp2 expm1 fdim fma fmax
   fmin fpclassify hypot ilogb isfinite isgreater isgreaterequal isinf
   isless islessequal islessgreater isnan isnormal isunordered lgamma
   log1p log2 logb lrint lround nan nearbyint nextafter nexttoward remainder
   remquo rint round scalbn signbit tgamma trunc

   See:
   http://pubs.opengroup.org/onlinepubs/009695399/basedefs/math.h.html

 * Berkeley/SVID extensions:

   j0 j1 jn y0 y1 yn

 * Configure already (5.21.5) scans for:

   copysign*l* fpclassify isfinite isinf isnan isnan*l* ilogb*l* signbit scalbn*l*

 * For floating-point round mode (which matters for e.g. lrint and rint)

   fegetround fesetround

*/

/* XXX Constant FP_FAST_FMA (if true, FMA is faster) */

/* XXX Add ldiv(), lldiv()?  It's C99, but from stdlib.h, not math.h  */

/* XXX Beware old gamma() -- one cannot know whether that is the
 * gamma or the log of gamma, that's why the new tgamma and lgamma.
 * Though also remember lgamma_r. */

/* Certain AIX releases have the C99 math, but not in long double.
 * The <math.h> has them, e.g. __expl128, but no library has them!
 *
 * Also see the comments in hints/aix.sh about long doubles. */

#if defined(USE_QUADMATH) && defined(I_QUADMATH)
#  define c99_acosh	acoshq
#  define c99_asinh	asinhq
#  define c99_atanh	atanhq
#  define c99_cbrt	cbrtq
#  define c99_copysign	copysignq
#  define c99_erf	erfq
#  define c99_erfc	erfcq
/* no exp2q */
#  define c99_expm1	expm1q
#  define c99_fdim	fdimq
#  define c99_fma	fmaq
#  define c99_fmax	fmaxq
#  define c99_fmin	fminq
#  define c99_hypot	hypotq
#  define c99_ilogb	ilogbq
#  define c99_lgamma	lgammaq
#  define c99_log1p	log1pq
#  define c99_log2	log2q
/* no logbq */
#  if defined(USE_64_BIT_INT) && QUADKIND == QUAD_IS_LONG_LONG
#    define c99_lrint	llrintq
#    define c99_lround	llroundq
#  else
#    define c99_lrint	lrintq
#    define c99_lround	lroundq
#  endif
#  define c99_nan	nanq
#  define c99_nearbyint	nearbyintq
#  define c99_nextafter	nextafterq
/* no nexttowardq */
#  define c99_remainder	remainderq
#  define c99_remquo	remquoq
#  define c99_rint	rintq
#  define c99_round	roundq
#  define c99_scalbn	scalbnq
/* We already define Perl_signbit to signbitq in perl.h. */
#  define c99_tgamma	tgammaq
#  define c99_trunc	truncq
#  define bessel_j0 j0q
#  define bessel_j1 j1q
#  define bessel_jn jnq
#  define bessel_y0 y0q
#  define bessel_y1 y1q
#  define bessel_yn ynq
#elif defined(USE_LONG_DOUBLE) && \
  (defined(HAS_FREXPL) || defined(HAS_ILOGBL)) && defined(HAS_SQRTL)
/* Use some of the Configure scans for long double math functions
 * as the canary for all the C99 *l variants being defined. */
#  define c99_acosh	acoshl
#  define c99_asinh	asinhl
#  define c99_atanh	atanhl
#  define c99_cbrt	cbrtl
#  define c99_copysign	copysignl
#  define c99_erf	erfl
#  define c99_erfc	erfcl
#  define c99_exp2	exp2l
#  define c99_expm1	expm1l
#  define c99_fdim	fdiml
#  define c99_fma	fmal
#  define c99_fmax	fmaxl
#  define c99_fmin	fminl
#  define c99_hypot	hypotl
#  define c99_ilogb	ilogbl
#  define c99_lgamma	lgammal
#  define c99_log1p	log1pl
#  define c99_log2	log2l
#  define c99_logb	logbl
#  if defined(USE_64_BIT_INT) && QUADKIND == QUAD_IS_LONG_LONG && defined(HAS_LLRINTL)
#    define c99_lrint	llrintl
#  elif defined(HAS_LRINTL)
#    define c99_lrint	lrintl
#  endif
#  if defined(USE_64_BIT_INT) && QUADKIND == QUAD_IS_LONG_LONG && defined(HAS_LLROUNDL)
#    define c99_lround	llroundl
#  elif defined(HAS_LROUNDL)
#    define c99_lround	lroundl
#  endif
#  define c99_nan	nanl
#  define c99_nearbyint	nearbyintl
#  define c99_nextafter	nextafterl
#  define c99_nexttoward	nexttowardl
#  define c99_remainder	remainderl
#  define c99_remquo	remquol
#  define c99_rint	rintl
#  define c99_round	roundl
#  define c99_scalbn	scalbnl
/* We already define Perl_signbit in perl.h. */
#  define c99_tgamma	tgammal
#  define c99_trunc	truncl
#else
#  define c99_acosh	acosh
#  define c99_asinh	asinh
#  define c99_atanh	atanh
#  define c99_cbrt	cbrt
#  define c99_copysign	copysign
#  define c99_erf	erf
#  define c99_erfc	erfc
#  define c99_exp2	exp2
#  define c99_expm1	expm1
#  define c99_fdim	fdim
#  define c99_fma	fma
#  define c99_fmax	fmax
#  define c99_fmin	fmin
#  define c99_hypot	hypot
#  define c99_ilogb	ilogb
#  define c99_lgamma	lgamma
#  define c99_log1p	log1p
#  define c99_log2	log2
#  define c99_logb	logb
#  if defined(USE_64_BIT_INT) && QUADKIND == QUAD_IS_LONG_LONG && defined(HAS_LLRINT)
#    define c99_lrint	llrint
#  else
#    define c99_lrint	lrint
#  endif
#  if defined(USE_64_BIT_INT) && QUADKIND == QUAD_IS_LONG_LONG && defined(HAS_LLROUND)
#    define c99_lround	llround
#  else
#    define c99_lround	lround
#  endif
#  define c99_nan	nan
#  define c99_nearbyint	nearbyint
#  define c99_nextafter	nextafter
#  define c99_nexttoward	nexttoward
#  define c99_remainder	remainder
#  define c99_remquo	remquo
#  define c99_rint	rint
#  define c99_round	round
#  define c99_scalbn	scalbn
/* We already define Perl_signbit in perl.h. */
#  define c99_tgamma	tgamma
#  define c99_trunc	trunc
#endif

/* AIX xlc (__IBMC__) really doesn't have the following long double
 * math interfaces (no __acoshl128 aka acoshl, etc.), see
 * hints/aix.sh.  These are in the -lc128 but fail to be found
 * during dynamic linking/loading.
 *
 * XXX1 Better Configure scans
 * XXX2 Is this xlc version dependent? */
#if defined(USE_LONG_DOUBLE) && defined(__IBMC__)
#  undef c99_acosh
#  undef c99_asinh
#  undef c99_atanh
#  undef c99_cbrt
#  undef c99_copysign
#  undef c99_exp2
#  undef c99_expm1
#  undef c99_fdim
#  undef c99_fma
#  undef c99_fmax
#  undef c99_fmin
#  undef c99_hypot
#  undef c99_ilogb
#  undef c99_lrint
#  undef c99_lround
#  undef c99_log1p
#  undef c99_log2
#  undef c99_logb
#  undef c99_nan
#  undef c99_nearbyint
#  undef c99_nextafter
#  undef c99_nexttoward
#  undef c99_remainder
#  undef c99_remquo
#  undef c99_rint
#  undef c99_round
#  undef c99_scalbn
#  undef c99_tgamma
#  undef c99_trunc
#endif

/* The cc with NetBSD 8.0 and 9.0 claims to be a C11 hosted compiler,
 * but doesn't define several functions required by C99, let alone C11.
 * http://gnats.netbsd.org/53234
 */
#if defined(USE_LONG_DOUBLE) && defined(__NetBSD__) \
  && !defined(NETBSD_HAVE_FIXED_LONG_DOUBLE_MATH)
#  undef c99_expm1
#  undef c99_lgamma
#  undef c99_log1p
#  undef c99_log2
#  undef c99_nexttoward
#  undef c99_remainder
#  undef c99_remquo
#  undef c99_tgamma
#endif

#ifndef isunordered
#  ifdef Perl_isnan
#    define isunordered(x, y) (Perl_isnan(x) || Perl_isnan(y))
#  elif defined(HAS_UNORDERED)
#    define isunordered(x, y) unordered(x, y)
#  endif
#endif

/* XXX these isgreater/isnormal/isunordered macros definitions should
 * be moved further in the file to be part of the emulations, so that
 * platforms can e.g. #undef c99_isunordered and have it work like
 * it does for the other interfaces. */

#if !defined(isgreater) && defined(isunordered)
#  define isgreater(x, y)         (!isunordered((x), (y)) && (x) > (y))
#  define isgreaterequal(x, y)    (!isunordered((x), (y)) && (x) >= (y))
#  define isless(x, y)            (!isunordered((x), (y)) && (x) < (y))
#  define islessequal(x, y)       (!isunordered((x), (y)) && (x) <= (y))
#  define islessgreater(x, y)     (!isunordered((x), (y)) && \
                                     ((x) > (y) || (y) > (x)))
#endif

/* Check both the Configure symbol and the macro-ness (like C99 promises). */ 
#if defined(HAS_FPCLASSIFY) && defined(fpclassify)
#  define c99_fpclassify	fpclassify
#endif
/* Like isnormal(), the isfinite(), isinf(), and isnan() are also C99
   and also (sizeof-arg-aware) macros, but they are already well taken
   care of by Configure et al, and defined in perl.h as
   Perl_isfinite(), Perl_isinf(), and Perl_isnan(). */
#ifdef isnormal
#  define c99_isnormal	isnormal
#endif
#ifdef isgreater /* canary for all the C99 is*<cmp>* macros. */
#  define c99_isgreater	isgreater
#  define c99_isgreaterequal	isgreaterequal
#  define c99_isless		isless
#  define c99_islessequal	islessequal
#  define c99_islessgreater	islessgreater
#  define c99_isunordered	isunordered
#endif

/* The Great Wall of Undef where according to the definedness of HAS_FOO symbols
 * the corresponding c99_foo wrappers are undefined.  This list doesn't include
 * the isfoo() interfaces because they are either type-aware macros, or dealt
 * separately, already in perl.h */

#ifndef HAS_ACOSH
#  undef c99_acosh
#endif
#ifndef HAS_ASINH
#  undef c99_asinh
#endif
#ifndef HAS_ATANH
#  undef c99_atanh
#endif
#ifndef HAS_CBRT
#  undef c99_cbrt
#endif
#ifndef HAS_COPYSIGN
#  undef c99_copysign
#endif
#ifndef HAS_ERF
#  undef c99_erf
#endif
#ifndef HAS_ERFC
#  undef c99_erfc
#endif
#ifndef HAS_EXP2
#  undef c99_exp2
#endif
#ifndef HAS_EXPM1
#  undef c99_expm1
#endif
#ifndef HAS_FDIM
#  undef c99_fdim
#endif
#ifndef HAS_FMA
#  undef c99_fma
#endif
#ifndef HAS_FMAX
#  undef c99_fmax
#endif
#ifndef HAS_FMIN
#  undef c99_fmin
#endif
#ifndef HAS_FPCLASSIFY
#  undef c99_fpclassify
#endif
#ifndef HAS_HYPOT
#  undef c99_hypot
#endif
#ifndef HAS_ILOGB
#  undef c99_ilogb
#endif
#ifndef HAS_LGAMMA
#  undef c99_lgamma
#endif
#ifndef HAS_LOG1P
#  undef c99_log1p
#endif
#ifndef HAS_LOG2
#  undef c99_log2
#endif
#ifndef HAS_LOGB
#  undef c99_logb
#endif
#ifndef HAS_LRINT
#  undef c99_lrint
#endif
#ifndef HAS_LROUND
#  undef c99_lround
#endif
#ifndef HAS_NAN
#  undef c99_nan
#endif
#ifndef HAS_NEARBYINT
#  undef c99_nearbyint
#endif
#ifndef HAS_NEXTAFTER
#  undef c99_nextafter
#endif
#ifndef HAS_NEXTTOWARD
#  undef c99_nexttoward
#endif
#ifndef HAS_REMAINDER
#  undef c99_remainder
#endif
#ifndef HAS_REMQUO
#  undef c99_remquo
#endif
#ifndef HAS_RINT
#  undef c99_rint
#endif
#ifndef HAS_ROUND
#  undef c99_round
#endif
#ifndef HAS_SCALBN
#  undef c99_scalbn
#endif
#ifndef HAS_TGAMMA
#  undef c99_tgamma
#endif
#ifndef HAS_TRUNC
#  undef c99_trunc
#endif

#ifdef _MSC_VER

/* Some APIs exist under Win32 with "underbar" names. */
#  undef c99_hypot
#  undef c99_logb
#  undef c99_nextafter
#  define c99_hypot _hypot
#  define c99_logb _logb
#  define c99_nextafter _nextafter

#  define bessel_j0 _j0
#  define bessel_j1 _j1
#  define bessel_jn _jn
#  define bessel_y0 _y0
#  define bessel_y1 _y1
#  define bessel_yn _yn

#endif

/* The Bessel functions: BSD, SVID, XPG4, and POSIX.  But not C99. */
#if defined(HAS_J0) && !defined(bessel_j0)
#  if defined(USE_LONG_DOUBLE) && defined(HAS_J0L)
#    define bessel_j0 j0l
#    define bessel_j1 j1l
#    define bessel_jn jnl
#    define bessel_y0 y0l
#    define bessel_y1 y1l
#    define bessel_yn ynl
#  else
#    define bessel_j0 j0
#    define bessel_j1 j1
#    define bessel_jn jn
#    define bessel_y0 y0
#    define bessel_y1 y1
#    define bessel_yn yn
#  endif
#endif

/* Emulations for missing math APIs.
 *
 * Keep in mind that the point of many of these functions is that
 * they, if available, are supposed to give more precise/more
 * numerically stable results.
 *
 * See e.g. http://www.johndcook.com/math_h.html
 */

#ifndef c99_acosh
static NV my_acosh(NV x)
{
  return Perl_log(x + Perl_sqrt(x * x - 1));
}
#  define c99_acosh my_acosh
#endif

#ifndef c99_asinh
static NV my_asinh(NV x)
{
  return Perl_log(x + Perl_sqrt(x * x + 1));
}
#  define c99_asinh my_asinh
#endif

#ifndef c99_atanh
static NV my_atanh(NV x)
{
  return (Perl_log(1 + x) - Perl_log(1 - x)) / 2;
}
#  define c99_atanh my_atanh
#endif

#ifndef c99_cbrt
static NV my_cbrt(NV x)
{
  static const NV one_third = (NV)1.0/3;
  return x >= 0.0 ? Perl_pow(x, one_third) : -Perl_pow(-x, one_third);
}
#  define c99_cbrt my_cbrt
#endif

#ifndef c99_copysign
static NV my_copysign(NV x, NV y)
{
  return y >= 0 ? (x < 0 ? -x : x) : (x < 0 ? x : -x);
}
#  define c99_copysign my_copysign
#endif

/* XXX cosh (though c89) */

#ifndef c99_erf
static NV my_erf(NV x)
{
  /* http://www.johndcook.com/cpp_erf.html -- public domain */
  NV a1 =  0.254829592;
  NV a2 = -0.284496736;
  NV a3 =  1.421413741;
  NV a4 = -1.453152027;
  NV a5 =  1.061405429;
  NV p  =  0.3275911;
  NV t, y;
  int sign = x < 0 ? -1 : 1; /* Save the sign. */
  x = PERL_ABS(x);

  /* Abramowitz and Stegun formula 7.1.26 */
  t = 1.0 / (1.0 + p * x);
  y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1) * t * Perl_exp(-x*x);

  return sign * y;
}
#  define c99_erf my_erf
#endif

#ifndef c99_erfc
static NV my_erfc(NV x) {
  /* This is not necessarily numerically stable, but better than nothing. */
  return 1.0 - c99_erf(x);
}
#  define c99_erfc my_erfc
#endif

#ifndef c99_exp2
static NV my_exp2(NV x)
{
  return Perl_pow((NV)2.0, x);
}
#  define c99_exp2 my_exp2
#endif

#ifndef c99_expm1
static NV my_expm1(NV x)
{
  if (PERL_ABS(x) < 1e-5)
    /* http://www.johndcook.com/cpp_expm1.html -- public domain.
     * Taylor series, the first four terms (the last term quartic). */
    /* Probably not enough for long doubles. */
    return x * (1.0 + x * (1/2.0 + x * (1/6.0 + x/24.0)));
  else
    return Perl_exp(x) - 1;
}
#  define c99_expm1 my_expm1
#endif

#ifndef c99_fdim
static NV my_fdim(NV x, NV y)
{
#ifdef NV_NAN
  return (Perl_isnan(x) || Perl_isnan(y)) ? NV_NAN : (x > y ? x - y : 0);
#else
  return (x > y ? x - y : 0);
#endif
}
#  define c99_fdim my_fdim
#endif

#ifndef c99_fma
static NV my_fma(NV x, NV y, NV z)
{
  return (x * y) + z;
}
#  define c99_fma my_fma
#endif

#ifndef c99_fmax
static NV my_fmax(NV x, NV y)
{
#ifdef NV_NAN
  if (Perl_isnan(x)) {
    return Perl_isnan(y) ? NV_NAN : y;
  } else if (Perl_isnan(y)) {
    return x;
  }
#endif
  return x > y ? x : y;
}
#  define c99_fmax my_fmax
#endif

#ifndef c99_fmin
static NV my_fmin(NV x, NV y)
{
#ifdef NV_NAN
  if (Perl_isnan(x)) {
    return Perl_isnan(y) ? NV_NAN : y;
  } else if (Perl_isnan(y)) {
    return x;
  }
#endif
  return x < y ? x : y;
}
#  define c99_fmin my_fmin
#endif

#ifndef c99_fpclassify

static IV my_fpclassify(NV x)
{
#ifdef Perl_fp_class_inf
  if (Perl_fp_class_inf(x))    return FP_INFINITE;
  if (Perl_fp_class_nan(x))    return FP_NAN;
  if (Perl_fp_class_norm(x))   return FP_NORMAL;
  if (Perl_fp_class_denorm(x)) return FP_SUBNORMAL;
  if (Perl_fp_class_zero(x))   return FP_ZERO;
#  define c99_fpclassify my_fpclassify
#endif
  return -1;
}

#endif

#ifndef c99_hypot
static NV my_hypot(NV x, NV y)
{
  /* http://en.wikipedia.org/wiki/Hypot */
  NV t;
  x = PERL_ABS(x); /* Take absolute values. */
  if (y == 0)
    return x;
#ifdef NV_INF
  if (Perl_isnan(y))
    return NV_INF;
#endif
  y = PERL_ABS(y);
  if (x < y) { /* Swap so that y is less. */
    t = x;
    x = y;
    y = t;
  }
  t = y / x;
  return x * Perl_sqrt(1.0 + t * t);
}
#  define c99_hypot my_hypot
#endif

#ifndef c99_ilogb
static IV my_ilogb(NV x)
{
  return (IV)(Perl_log(x) * M_LOG2E);
}
#  define c99_ilogb my_ilogb
#endif

/* tgamma and lgamma emulations based on
 * http://www.johndcook.com/cpp_gamma.html,
 * code placed in public domain.
 *
 * Note that these implementations (neither the johndcook originals
 * nor these) do NOT set the global signgam variable.  This is not
 * necessarily a bad thing. */

/* Note that the tgamma() and lgamma() implementations
 * here depend on each other. */

#if !defined(HAS_TGAMMA) || !defined(c99_tgamma)
static NV my_tgamma(NV x);
#  define c99_tgamma my_tgamma
#  define USE_MY_TGAMMA
#endif
#if !defined(HAS_LGAMMA) || !defined(c99_lgamma)
static NV my_lgamma(NV x);
#  define c99_lgamma my_lgamma
#  define USE_MY_LGAMMA
#endif

#ifdef USE_MY_TGAMMA
static NV my_tgamma(NV x)
{
  const NV gamma = 0.577215664901532860606512090; /* Euler's gamma constant. */
#ifdef NV_NAN
  if (Perl_isnan(x) || x < 0.0)
    return NV_NAN;
#endif
#ifdef NV_INF
  if (x == 0.0 || x == NV_INF)
#ifdef DOUBLE_IS_IEEE_FORMAT
    return x == -0.0 ? -NV_INF : NV_INF;
#else
    return NV_INF;
#endif
#endif

  /* The function domain is split into three intervals:
   * (0, 0.001), [0.001, 12), and (12, infinity) */

  /* First interval: (0, 0.001)
   * For small values, 1/tgamma(x) has power series x + gamma x^2,
   * so in this range, 1/tgamma(x) = x + gamma x^2 with error on the order of x^3.
   * The relative error over this interval is less than 6e-7. */
  if (x < 0.001)
    return 1.0 / (x * (1.0 + gamma * x));

  /* Second interval: [0.001, 12) */
  if (x < 12.0) {
    double y = x; /* Working copy. */
    int n = 0;
    /* Numerator coefficients for approximation over the interval (1,2) */
    static const NV p[] = {
      -1.71618513886549492533811E+0,
      2.47656508055759199108314E+1,
      -3.79804256470945635097577E+2,
      6.29331155312818442661052E+2,
      8.66966202790413211295064E+2,
      -3.14512729688483675254357E+4,
      -3.61444134186911729807069E+4,
      6.64561438202405440627855E+4
    };
    /* Denominator coefficients for approximation over the interval (1, 2) */
    static const NV q[] = {
      -3.08402300119738975254353E+1,
      3.15350626979604161529144E+2,
      -1.01515636749021914166146E+3,
      -3.10777167157231109440444E+3,
      2.25381184209801510330112E+4,
      4.75584627752788110767815E+3,
      -1.34659959864969306392456E+5,
      -1.15132259675553483497211E+5
    };
    NV num = 0.0;
    NV den = 1.0;
    NV z;
    NV result;
    int i;

    if (x < 1.0)
      y += 1.0;
    else {
      n = (int)Perl_floor(y) - 1;
      y -= n;
    }
    z = y - 1;
    for (i = 0; i < 8; i++) {
      num = (num + p[i]) * z;
      den = den * z + q[i];
    }
    result = num / den + 1.0;

    if (x < 1.0) {
      /* Use the identity tgamma(z) = tgamma(z+1)/z
       * The variable "result" now holds tgamma of the original y + 1
       * Thus we use y - 1 to get back the original y. */
      result /= (y - 1.0);
    }
    else {
      /* Use the identity tgamma(z+n) = z*(z+1)* ... *(z+n-1)*tgamma(z) */
      for (i = 0; i < n; i++)
        result *= y++;
    }

    return result;
  }

#ifdef NV_INF
  /* Third interval: [12, +Inf) */
#if LDBL_MANT_DIG == 113 /* IEEE quad prec */
  if (x > 1755.548) {
    return NV_INF;
  }
#else
  if (x > 171.624) {
    return NV_INF;
  }
#endif
#endif

  return Perl_exp(c99_lgamma(x));
}
#endif

#ifdef USE_MY_LGAMMA
static NV my_lgamma(NV x)
{
#ifdef NV_NAN
  if (Perl_isnan(x))
    return NV_NAN;
#endif
#ifdef NV_INF
  if (x <= 0 || x == NV_INF)
    return NV_INF;
#endif
  if (x == 1.0 || x == 2.0)
    return 0;
  if (x < 12.0)
    return Perl_log(PERL_ABS(c99_tgamma(x)));
  /* Abramowitz and Stegun 6.1.41
   * Asymptotic series should be good to at least 11 or 12 figures
   * For error analysis, see Whittiker and Watson
   * A Course in Modern Analysis (1927), page 252 */
  {
    static const NV c[8] = {
      1.0/12.0,
      -1.0/360.0,
      1.0/1260.0,
      -1.0/1680.0,
      1.0/1188.0,
      -691.0/360360.0,
      1.0/156.0,
      -3617.0/122400.0
    };
    NV z = 1.0 / (x * x);
    NV sum = c[7];
    static const NV half_log_of_two_pi =
      0.91893853320467274178032973640562;
    NV series;
    int i;
    for (i = 6; i >= 0; i--) {
      sum *= z;
      sum += c[i];
    }
    series = sum / x;
    return (x - 0.5) * Perl_log(x) - x + half_log_of_two_pi + series;
  }
}
#endif

#ifndef c99_log1p
static NV my_log1p(NV x)
{
  /* http://www.johndcook.com/cpp_log_one_plus_x.html -- public domain.
   * Taylor series, the first four terms (the last term quartic). */
#ifdef NV_NAN
  if (x < -1.0)
    return NV_NAN;
#endif
#ifdef NV_INF
  if (x == -1.0)
    return -NV_INF;
#endif
  if (PERL_ABS(x) > 1e-4)
    return Perl_log(1.0 + x);
  else
    /* Probably not enough for long doubles. */
    return x * (1.0 + x * (-1/2.0 + x * (1/3.0 - x/4.0)));
}
#  define c99_log1p my_log1p
#endif

#ifndef c99_log2
static NV my_log2(NV x)
{
  return Perl_log(x) * M_LOG2E;
}
#  define c99_log2 my_log2
#endif

/* XXX nextafter */

/* XXX nexttoward */

/* GCC's FLT_ROUNDS is (wrongly) hardcoded to 1 (at least up to 11.x) */
#if defined(PERL_IS_GCC) /* && __GNUC__ < XXX */ || (defined(__clang__) && defined(__s390x__))
#  define BROKEN_FLT_ROUNDS
#endif

static int my_fegetround()
{
#ifdef HAS_FEGETROUND
  return fegetround();
#elif defined(HAS_FPGETROUND)
  switch (fpgetround()) {
  case FP_RN: return FE_TONEAREST;
  case FP_RZ: return FE_TOWARDZERO;
  case FP_RM: return FE_DOWNWARD;
  case FP_RP: return FE_UPWARD;
  default: return -1;
  }
#elif defined(FLT_ROUNDS)
  switch (FLT_ROUNDS) {
  case 0: return FE_TOWARDZERO;
  case 1: return FE_TONEAREST;
  case 2: return FE_UPWARD;
  case 3: return FE_DOWNWARD;
  default: return -1;
  }
#elif defined(__osf__) /* Tru64 */
  switch (read_rnd()) {
  case FP_RND_RN: return FE_TONEAREST;
  case FP_RND_RZ: return FE_TOWARDZERO;
  case FP_RND_RM: return FE_DOWNWARD;
  case FP_RND_RP: return FE_UPWARD;
  default: return -1;
  }
#else
  return -1;
#endif
}

/* Toward closest integer. */
#define MY_ROUND_NEAREST(x) ((NV)((IV)((x) >= 0.0 ? (x) + 0.5 : (x) - 0.5)))

/* Toward zero. */
#define MY_ROUND_TRUNC(x) ((NV)((IV)(x)))

/* Toward minus infinity. */
#define MY_ROUND_DOWN(x) ((NV)((IV)((x) >= 0.0 ? (x) : (x) - 0.5)))

/* Toward plus infinity. */
#define MY_ROUND_UP(x) ((NV)((IV)((x) >= 0.0 ? (x) + 0.5 : (x))))

#if (!defined(c99_nearbyint) || !defined(c99_lrint)) && defined(FE_TONEAREST)
static NV my_rint(NV x)
{
#ifdef FE_TONEAREST
  switch (my_fegetround()) {
  case FE_TONEAREST:  return MY_ROUND_NEAREST(x);
  case FE_TOWARDZERO: return MY_ROUND_TRUNC(x);
  case FE_DOWNWARD:   return MY_ROUND_DOWN(x);
  case FE_UPWARD:     return MY_ROUND_UP(x);
  default: break;
  }
#elif defined(HAS_FPGETROUND)
  switch (fpgetround()) {
  case FP_RN: return MY_ROUND_NEAREST(x);
  case FP_RZ: return MY_ROUND_TRUNC(x);
  case FP_RM: return MY_ROUND_DOWN(x);
  case FE_RP: return MY_ROUND_UP(x);
  default: break;
  }
#endif
  not_here("rint");
  NOT_REACHED; /* NOTREACHED */
}
#endif

/* XXX nearbyint() and rint() are not really identical -- but the difference
 * is messy: nearbyint is defined NOT to raise FE_INEXACT floating point
 * exceptions, while rint() is defined to MAYBE raise them.  At the moment
 * Perl is blissfully unaware of such fine detail of floating point. */
#ifndef c99_nearbyint
#  ifdef FE_TONEAREST
#    define c99_nearbyrint my_rint
#  endif
#endif

#ifndef c99_lrint
#  ifdef FE_TONEAREST
static IV my_lrint(NV x)
{
  return (IV)my_rint(x);
}
#    define c99_lrint my_lrint
#  endif
#endif

#ifndef c99_lround
static IV my_lround(NV x)
{
  return (IV)MY_ROUND_NEAREST(x);
}
#  define c99_lround my_lround
#endif

/* XXX remainder */

/* XXX remquo */

#ifndef c99_rint
#  ifdef FE_TONEAREST
#    define c99_rint my_rint
#  endif
#endif

#ifndef c99_round
static NV my_round(NV x)
{
  return MY_ROUND_NEAREST(x);
}
#  define c99_round my_round
#endif

#ifndef c99_scalbn
#   if defined(Perl_ldexp) && FLT_RADIX == 2
static NV my_scalbn(NV x, int y)
{
  return Perl_ldexp(x, y);
}
#    define c99_scalbn my_scalbn
#  endif
#endif

/* XXX sinh (though c89) */

/* tgamma -- see lgamma */

/* XXX tanh (though c89) */

#ifndef c99_trunc
static NV my_trunc(NV x)
{
  return MY_ROUND_TRUNC(x);
}
#  define c99_trunc my_trunc
#endif

#ifdef NV_NAN

#undef NV_PAYLOAD_DEBUG

/* NOTE: the NaN payload API implementation is hand-rolled, since the
 * APIs are only proposed ones as of June 2015, so very few, if any,
 * platforms have implementations yet, so HAS_SETPAYLOAD and such are
 * unlikely to be helpful.
 *
 * XXX - if the core numification wants to actually generate
 * the nan payload in "nan(123)", and maybe "nans(456)", for
 * signaling payload", this needs to be moved to e.g. numeric.c
 * (look for grok_infnan)
 *
 * Conversely, if the core stringification wants the nan payload
 * and/or the nan quiet/signaling distinction, S_getpayload()
 * from this file needs to be moved, to e.g. sv.c (look for S_infnan_2pv),
 * and the (trivial) functionality of issignaling() copied
 * (for generating "NaNS", or maybe even "NaNQ") -- or maybe there
 * are too many formatting parameters for simple stringification?
 */

/* While it might make sense for the payload to be UV or IV,
 * to avoid conversion loss, the proposed ISO interfaces use
 * a floating point input, which is then truncated to integer,
 * and only the integer part being used.  This is workable,
 * except for: (1) the conversion loss (2) suboptimal for
 * 32-bit integer platforms.  A workaround API for (2) and
 * in general for bit-honesty would be an array of integers
 * as the payload... but the proposed C API does nothing of
 * the kind. */
#if NVSIZE == UVSIZE
#  define NV_PAYLOAD_TYPE UV
#else
#  define NV_PAYLOAD_TYPE NV
#endif

#if defined(USE_LONG_DOUBLE) && defined(LONGDOUBLE_DOUBLEDOUBLE)
#  define NV_PAYLOAD_SIZEOF_ASSERT(a) \
    STATIC_ASSERT_STMT(sizeof(a) == NVSIZE / 2)
#else
#  define NV_PAYLOAD_SIZEOF_ASSERT(a) \
    STATIC_ASSERT_STMT(sizeof(a) == NVSIZE)
#endif

static void S_setpayload(NV* nvp, NV_PAYLOAD_TYPE payload, bool signaling)
{
  dTHX;
  static const U8 m[] = { NV_NAN_PAYLOAD_MASK };
  static const U8 p[] = { NV_NAN_PAYLOAD_PERM };
  UV a[(NVSIZE + UVSIZE - 1) / UVSIZE] = { 0 };
  int i;
  NV_PAYLOAD_SIZEOF_ASSERT(m);
  NV_PAYLOAD_SIZEOF_ASSERT(p);
  *nvp = NV_NAN;
  /* Divide the input into the array in "base unsigned integer" in
   * little-endian order.  Note that the integer might be smaller than
   * an NV (if UV is U32, for example). */
#if NVSIZE == UVSIZE
  a[0] = payload;  /* The trivial case. */
#else
  {
    NV t1 = c99_trunc(payload); /* towards zero (drop fractional) */
#ifdef NV_PAYLOAD_DEBUG
    Perl_warn(aTHX_ "t1 = %" NVgf " (payload %" NVgf ")\n", t1, payload);
#endif
    if (t1 <= UV_MAX) {
      a[0] = (UV)t1;  /* Fast path, also avoids rounding errors (right?) */
    } else {
      /* UVSIZE < NVSIZE or payload > UV_MAX.
       *
       * This may happen for example if:
       * (1) UVSIZE == 32 and common 64-bit double NV
       *     (32-bit system not using -Duse64bitint)
       * (2) UVSIZE == 64 and the x86-style 80-bit long double NV
       *     (note that here the room for payload is actually the 64 bits)
       * (3) UVSIZE == 64 and the 128-bit IEEE 764 quadruple NV
       *     (112 bits in mantissa, 111 bits room for payload)
       *
       * NOTE: this is very sensitive to correctly functioning
       * fmod()/fmodl(), and correct casting of big-unsigned-integer to NV.
       * If these don't work right, especially the low order bits
       * are in danger.  For example Solaris and AIX seem to have issues
       * here, especially if using 32-bit UVs. */
      NV t2;
      for (i = 0, t2 = t1; i < (int)C_ARRAY_LENGTH(a); i++) {
        a[i] = (UV)Perl_fmod(t2, (NV)UV_MAX);
        t2 = Perl_floor(t2 / (NV)UV_MAX);
      }
    }
  }
#endif
#ifdef NV_PAYLOAD_DEBUG
  for (i = 0; i < (int)C_ARRAY_LENGTH(a); i++) {
    Perl_warn(aTHX_ "a[%d] = 0x%" UVxf "\n", i, a[i]);
  }
#endif
  for (i = 0; i < (int)sizeof(p); i++) {
    if (m[i] && p[i] < sizeof(p)) {
      U8 s = (p[i] % UVSIZE) << 3;
      UV u = a[p[i] / UVSIZE] & ((UV)0xFF << s);
      U8 b = (U8)((u >> s) & m[i]);
      ((U8 *)(nvp))[i] &= ~m[i]; /* For NaNs with non-zero payload bits. */
      ((U8 *)(nvp))[i] |= b;
#ifdef NV_PAYLOAD_DEBUG
      Perl_warn(aTHX_
                "set p[%2d] = %02x (i = %d, m = %02x, s = %2d, b = %02x, u = %08"
                UVxf ")\n", i, ((U8 *)(nvp))[i], i, m[i], s, b, u);
#endif
      a[p[i] / UVSIZE] &= ~u;
    }
  }
  if (signaling) {
    NV_NAN_SET_SIGNALING(nvp);
  }
#ifdef USE_LONG_DOUBLE
# if LONG_DOUBLEKIND == 3 || LONG_DOUBLEKIND == 4
#  if LONG_DOUBLESIZE > 10
  memset((char *)nvp + 10, '\0', LONG_DOUBLESIZE - 10); /* x86 long double */
#  endif
# endif
#endif
  for (i = 0; i < (int)C_ARRAY_LENGTH(a); i++) {
    if (a[i]) {
      Perl_warn(aTHX_ "payload lost bits (%" UVxf ")", a[i]);
      break;
    }
  }
#ifdef NV_PAYLOAD_DEBUG
  for (i = 0; i < NVSIZE; i++) {
    PerlIO_printf(Perl_debug_log, "%02x ", ((U8 *)(nvp))[i]);
  }
  PerlIO_printf(Perl_debug_log, "\n");
#endif
}

static NV_PAYLOAD_TYPE S_getpayload(NV nv)
{
  dTHX;
  static const U8 m[] = { NV_NAN_PAYLOAD_MASK };
  static const U8 p[] = { NV_NAN_PAYLOAD_PERM };
  UV a[(NVSIZE + UVSIZE - 1) / UVSIZE] = { 0 };
  int i;
  NV payload;
  NV_PAYLOAD_SIZEOF_ASSERT(m);
  NV_PAYLOAD_SIZEOF_ASSERT(p);
  payload = 0;
  for (i = 0; i < (int)sizeof(p); i++) {
    if (m[i] && p[i] < NVSIZE) {
      U8 s = (p[i] % UVSIZE) << 3;
      a[p[i] / UVSIZE] |= (UV)(((U8 *)(&nv))[i] & m[i]) << s;
    }
  }
  for (i = (int)C_ARRAY_LENGTH(a) - 1; i >= 0; i--) {
#ifdef NV_PAYLOAD_DEBUG
    Perl_warn(aTHX_ "a[%d] = %" UVxf "\n", i, a[i]);
#endif
    payload *= (NV) UV_MAX;
    payload += a[i];
  }
#ifdef NV_PAYLOAD_DEBUG
  for (i = 0; i < NVSIZE; i++) {
    PerlIO_printf(Perl_debug_log, "%02x ", ((U8 *)(&nv))[i]);
  }
  PerlIO_printf(Perl_debug_log, "\n");
#endif
  return payload;
}

#endif  /* #ifdef NV_NAN */

/* XXX This comment is just to make I_TERMIO and I_SGTTY visible to
   metaconfig for future extension writers.  We don't use them in POSIX.
   (This is really sneaky :-)  --AD
*/
#if defined(I_TERMIOS)
#include <termios.h>
#endif
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#ifdef I_UNISTD
#include <unistd.h>
#endif
#include <fcntl.h>

#ifdef HAS_TZNAME
#  if !defined(WIN32) && !defined(__CYGWIN__)
extern char *tzname[];
#  endif
#else
#if !defined(WIN32) || (defined(__MINGW32__) && !defined(tzname))
char *tzname[] = { "" , "" };
#endif
#endif

#if defined(__VMS) && !defined(__POSIX_SOURCE)

#  include <utsname.h>

#  undef mkfifo
#  define mkfifo(a,b) (not_here("mkfifo"),-1)

   /* The POSIX notion of ttyname() is better served by getname() under VMS */
   static char ttnambuf[64];
#  define ttyname(fd) (isatty(fd) > 0 ? getname(fd,ttnambuf,0) : NULL)

#else
#if defined (__CYGWIN__)
#    define tzname _tzname
#endif
#if defined (WIN32)
#  undef mkfifo
#  define mkfifo(a,b) not_here("mkfifo")
#  define ttyname(a) (char*)not_here("ttyname")
#  define sigset_t long
#  define pid_t long
#  ifdef _MSC_VER
#    define mode_t short
#  endif
#  ifdef __MINGW32__
#    define mode_t short
#    ifndef tzset
#      define tzset()		not_here("tzset")
#    endif
#    ifndef _POSIX_OPEN_MAX
#      define _POSIX_OPEN_MAX	FOPEN_MAX	/* XXX bogus ? */
#    endif
#  endif
#  define sigaction(a,b,c)	not_here("sigaction")
#  define sigpending(a)		not_here("sigpending")
#  define sigprocmask(a,b,c)	not_here("sigprocmask")
#  define sigsuspend(a)		not_here("sigsuspend")
#  define sigemptyset(a)	not_here("sigemptyset")
#  define sigaddset(a,b)	not_here("sigaddset")
#  define sigdelset(a,b)	not_here("sigdelset")
#  define sigfillset(a)		not_here("sigfillset")
#  define sigismember(a,b)	not_here("sigismember")
#  undef setuid
#  undef setgid
#  define setuid(a)		not_here("setuid")
#  define setgid(a)		not_here("setgid")
#if !defined(USE_LONG_DOUBLE) && !defined(USE_QUADMATH)
#  define strtold(s1,s2)	not_here("strtold")
#endif  /* !(USE_LONG_DOUBLE) && !(USE_QUADMATH) */
#else

#  ifndef HAS_MKFIFO
#    if defined(OS2) || defined(__amigaos4__)
#      define mkfifo(a,b) not_here("mkfifo")
#    else	/* !( defined OS2 ) */
#      ifndef mkfifo
#        define mkfifo(path, mode) (mknod((path), (mode) | S_IFIFO, 0))
#      endif
#    endif
#  endif /* !HAS_MKFIFO */

#  ifdef I_GRP
#    include <grp.h>
#  endif
#  include <sys/times.h>
#  ifdef HAS_UNAME
#    include <sys/utsname.h>
#  endif
#  ifndef __amigaos4__
#    include <sys/wait.h>
#  endif
#  ifdef I_UTIME
#    include <utime.h>
#  endif
#endif /* WIN32 */
#endif /* __VMS */

typedef int SysRet;
typedef long SysRetLong;
typedef sigset_t* POSIX__SigSet;
typedef HV* POSIX__SigAction;
typedef int POSIX__SigNo;
typedef int POSIX__Fd;
#ifdef I_TERMIOS
typedef struct termios* POSIX__Termios;
#else /* Define termios types to int, and call not_here for the functions.*/
#define POSIX__Termios int
#define speed_t int
#define tcflag_t int
#define cc_t int
#define cfgetispeed(x) not_here("cfgetispeed")
#define cfgetospeed(x) not_here("cfgetospeed")
#define tcdrain(x) not_here("tcdrain")
#define tcflush(x,y) not_here("tcflush")
#define tcsendbreak(x,y) not_here("tcsendbreak")
#define cfsetispeed(x,y) not_here("cfsetispeed")
#define cfsetospeed(x,y) not_here("cfsetospeed")
#define ctermid(x) (char *) not_here("ctermid")
#define tcflow(x,y) not_here("tcflow")
#define tcgetattr(x,y) not_here("tcgetattr")
#define tcsetattr(x,y,z) not_here("tcsetattr")
#endif

/* Possibly needed prototypes */
#ifndef WIN32
START_EXTERN_C
double strtod (const char *, char **);
long strtol (const char *, char **, int);
unsigned long strtoul (const char *, char **, int);
#ifdef HAS_STRTOLD
long double strtold (const char *, char **);
#endif
END_EXTERN_C
#endif

#ifndef HAS_DIFFTIME
#ifndef difftime
#define difftime(a,b) not_here("difftime")
#endif
#endif
#ifndef HAS_FPATHCONF
#define fpathconf(f,n)	(SysRetLong) not_here("fpathconf")
#endif
#ifndef HAS_MKTIME
#define mktime(a) not_here("mktime")
#endif
#ifndef HAS_NICE
#define nice(a) not_here("nice")
#endif
#ifndef HAS_PATHCONF
#define pathconf(f,n)	(SysRetLong) not_here("pathconf")
#endif
#ifndef HAS_SYSCONF
#define sysconf(n)	(SysRetLong) not_here("sysconf")
#endif
#ifndef HAS_READLINK
#define readlink(a,b,c) not_here("readlink")
#endif
#ifndef HAS_SETPGID
#define setpgid(a,b) not_here("setpgid")
#endif
#ifndef HAS_SETSID
#define setsid() not_here("setsid")
#endif
#ifndef HAS_STRCOLL
#define strcoll(s1,s2) not_here("strcoll")
#endif
#ifndef HAS_STRTOD
#define strtod(s1,s2) not_here("strtod")
#endif
#ifndef HAS_STRTOLD
#define strtold(s1,s2) not_here("strtold")
#endif
#ifndef HAS_STRTOL
#define strtol(s1,s2,b) not_here("strtol")
#endif
#ifndef HAS_STRTOUL
#define strtoul(s1,s2,b) not_here("strtoul")
#endif
#ifndef HAS_STRXFRM
#define strxfrm(s1,s2,n) not_here("strxfrm")
#endif
#ifndef HAS_TCGETPGRP
#define tcgetpgrp(a) not_here("tcgetpgrp")
#endif
#ifndef HAS_TCSETPGRP
#define tcsetpgrp(a,b) not_here("tcsetpgrp")
#endif
#ifndef HAS_TIMES
#define times(a) not_here("times")
#endif
#ifndef HAS_UNAME
#define uname(a) not_here("uname")
#endif
#ifndef HAS_WAITPID
#define waitpid(a,b,c) not_here("waitpid")
#endif

#if ! defined(HAS_MBLEN) && ! defined(HAS_MBRLEN)
#  define mblen(a,b) not_here("mblen")
#endif
#if ! defined(HAS_MBTOWC) && ! defined(HAS_MBRTOWC)
#  define mbtowc(pwc, s, n) not_here("mbtowc")
#endif
#if ! defined(HAS_WCTOMB) && ! defined(HAS_WCRTOMB)
#  define wctomb(s, wchar) not_here("wctomb")
#endif
#if !defined(HAS_MBLEN) && !defined(HAS_MBSTOWCS) && !defined(HAS_MBTOWC) && !defined(HAS_WCSTOMBS) && !defined(HAS_WCTOMB)
/* If we don't have these functions, then we wouldn't have gotten a typedef
   for wchar_t, the wide character type.  Defining wchar_t allows the
   functions referencing it to compile.  Its actual type is then meaningless,
   since without the above functions, all sections using it end up calling
   not_here() and croak.  --Kaveh Ghazi (ghazi@noc.rutgers.edu) 9/18/94. */
#ifndef wchar_t
#define wchar_t char
#endif
#endif

#ifdef HAS_LONG_DOUBLE
#  if LONG_DOUBLESIZE > NVSIZE
#    undef HAS_LONG_DOUBLE  /* XXX until we figure out how to use them */
#  endif
#endif

#ifndef HAS_LONG_DOUBLE
#ifdef LDBL_MAX
#undef LDBL_MAX
#endif
#ifdef LDBL_MIN
#undef LDBL_MIN
#endif
#ifdef LDBL_EPSILON
#undef LDBL_EPSILON
#endif
#endif

/* Background: in most systems the low byte of the wait status
 * is the signal (the lowest 7 bits) and the coredump flag is
 * the eight bit, and the second lowest byte is the exit status.
 * BeOS bucks the trend and has the bytes in different order.
 * See beos/beos.c for how the reality is bent even in BeOS
 * to follow the traditional.  However, to make the POSIX
 * wait W*() macros to work in BeOS, we need to unbend the
 * reality back in place. --jhi */
/* In actual fact the code below is to blame here. Perl has an internal
 * representation of the exit status ($?), which it re-composes from the
 * OS's representation using the W*() POSIX macros. The code below
 * incorrectly uses the W*() macros on the internal representation,
 * which fails for OSs that have a different representation (namely BeOS
 * and Haiku). WMUNGE() is a hack that converts the internal
 * representation into the OS specific one, so that the W*() macros work
 * as expected. The better solution would be not to use the W*() macros
 * in the first place, though. -- Ingo Weinhold
 */
#if defined(__HAIKU__)
#    define WMUNGE(x) (((x) & 0xFF00) >> 8 | (((U8) (x)) << 8))
#else
#    define WMUNGE(x) (x)
#endif

static int
not_here(const char *s)
{
    croak("POSIX::%s not implemented on this architecture", s);
    return -1;
}

#include "const-c.inc"

static void
restore_sigmask(pTHX_ SV *osset_sv)
{
     /* Fortunately, restoring the signal mask can't fail, because
      * there's nothing we can do about it if it does -- we're not
      * supposed to return -1 from sigaction unless the disposition
      * was unaffected.
      */
#if !(defined(__amigaos4__) && defined(__NEWLIB__))
     sigset_t *ossetp = (sigset_t *) SvPV_nolen( osset_sv );
     (void)sigprocmask(SIG_SETMASK, ossetp, (sigset_t *)0);
#endif
}

static void *
allocate_struct(pTHX_ SV *rv, const STRLEN size, const char *packname) {
    SV *const t = newSVrv(rv, packname);
    void *const p = sv_grow(t, size + 1);

    /* Ensure at least one use of not_here() to avoid "defined but not
     * used" warning.  This is not at all related to allocate_struct(); I
     * just needed somewhere to dump it - DAPM */
    if (0) { not_here(""); }

    SvCUR_set(t, size);
    SvPOK_on(t);
    return p;
}

#ifdef WIN32

/*
 * (1) The CRT maintains its own copy of the environment, separate from
 * the Win32API copy.
 *
 * (2) CRT getenv() retrieves from this copy. CRT putenv() updates this
 * copy, and then calls SetEnvironmentVariableA() to update the Win32API
 * copy.
 *
 * (3) win32_getenv() and win32_putenv() call GetEnvironmentVariableA() and
 * SetEnvironmentVariableA() directly, bypassing the CRT copy of the
 * environment.
 *
 * (4) The CRT strftime() "%Z" implementation calls __tzset(). That
 * calls CRT tzset(), but only the first time it is called, and in turn
 * that uses CRT getenv("TZ") to retrieve the timezone info from the CRT
 * local copy of the environment and hence gets the original setting as
 * perl never updates the CRT copy when assigning to $ENV{TZ}.
 *
 * Therefore, we need to retrieve the value of $ENV{TZ} and call CRT
 * putenv() to update the CRT copy of the environment (if it is different)
 * whenever we're about to call tzset().
 *
 * In addition to all that, when perl is built with PERL_IMPLICIT_SYS
 * defined:
 *
 * (a) Each interpreter has its own copy of the environment inside the
 * perlhost structure. That allows applications that host multiple
 * independent Perl interpreters to isolate environment changes from
 * each other. (This is similar to how the perlhost mechanism keeps a
 * separate working directory for each Perl interpreter, so that calling
 * chdir() will not affect other interpreters.)
 *
 * (b) Only the first Perl interpreter instantiated within a process will
 * "write through" environment changes to the process environment.
 *
 * (c) Even the primary Perl interpreter won't update the CRT copy of the
 * environment, only the Win32API copy (it calls win32_putenv()).
 *
 * As with CPerlHost::Getenv() and CPerlHost::Putenv() themselves, it makes
 * sense to only update the process environment when inside the main
 * interpreter, but we don't have access to CPerlHost's m_bTopLevel member
 * from here so we'll just have to check PL_curinterp instead.
 *
 * Therefore, we can simply #undef getenv() and putenv() so that those names
 * always refer to the CRT functions, and explicitly call win32_getenv() to
 * access perl's %ENV.
 *
 * We also #undef malloc() and free() to be sure we are using the CRT
 * functions otherwise under PERL_IMPLICIT_SYS they are redefined to calls
 * into VMem::Malloc() and VMem::Free() and all allocations will be freed
 * when the Perl interpreter is being destroyed so we'd end up with a pointer
 * into deallocated memory in environ[] if a program embedding a Perl
 * interpreter continues to operate even after the main Perl interpreter has
 * been destroyed.
 *
 * Note that we don't free() the malloc()ed memory unless and until we call
 * malloc() again ourselves because the CRT putenv() function simply puts its
 * pointer argument into the environ[] array (it doesn't make a copy of it)
 * so this memory must otherwise be leaked.
 */

#undef getenv
#undef putenv
#undef malloc
#undef free

static void
fix_win32_tzenv(void)
{
    static char* oldenv = NULL;
    char* newenv;
    const char* perl_tz_env = win32_getenv("TZ");
    const char* crt_tz_env = getenv("TZ");

    if (perl_tz_env == NULL)
        perl_tz_env = "";
    if (crt_tz_env == NULL)
        crt_tz_env = "";
    if (strNE(perl_tz_env, crt_tz_env)) {
        newenv = (char*)malloc((strlen(perl_tz_env) + 4) * sizeof(char));
        if (newenv != NULL) {
            sprintf(newenv, "TZ=%s", perl_tz_env);
            putenv(newenv);
            if (oldenv != NULL)
                free(oldenv);
            oldenv = newenv;
        }
    }
}

#endif

/*
 * my_tzset - wrapper to tzset() with a fix to make it work (better) on Win32.
 * This code is duplicated in the Time-Piece module, so any changes made here
 * should be made there too.
 */
static void
my_tzset(pTHX)
{
#ifdef WIN32
#if defined(USE_ITHREADS) && defined(PERL_IMPLICIT_SYS)
    if (PL_curinterp == aTHX)
#endif
        fix_win32_tzenv();
#endif
    TZSET_LOCK;
    tzset();
    TZSET_UNLOCK;
    /* After the unlock, another thread could change things, but this is a
     * problem with the Posix API generally, not Perl; and the result will be
     * self-consistent */
}

MODULE = SigSet		PACKAGE = POSIX::SigSet		PREFIX = sig

void
new(packname = "POSIX::SigSet", ...)
    const char *	packname
    CODE:
	{
	    int i;
	    sigset_t *const s
		= (sigset_t *) allocate_struct(aTHX_ (ST(0) = sv_newmortal()),
					       sizeof(sigset_t),
					       packname);
	    sigemptyset(s);
	    for (i = 1; i < items; i++) {
                IV sig = SvIV(ST(i));
		if (sigaddset(s, sig) < 0)
                    croak("POSIX::Sigset->new: failed to add signal %" IVdf, sig);
            }
	    XSRETURN(1);
	}

SysRet
addset(sigset, sig)
	POSIX::SigSet	sigset
	POSIX::SigNo	sig
   ALIAS:
	delset = 1
   CODE:
	RETVAL = ix ? sigdelset(sigset, sig) : sigaddset(sigset, sig);
   OUTPUT:
	RETVAL

SysRet
emptyset(sigset)
	POSIX::SigSet	sigset
   ALIAS:
	fillset = 1
   CODE:
	RETVAL = ix ? sigfillset(sigset) : sigemptyset(sigset);
   OUTPUT:
	RETVAL

int
sigismember(sigset, sig)
	POSIX::SigSet	sigset
	POSIX::SigNo	sig

MODULE = Termios	PACKAGE = POSIX::Termios	PREFIX = cf

void
new(packname = "POSIX::Termios", ...)
    const char *	packname
    CODE:
	{
#ifdef I_TERMIOS
	    void *const p = allocate_struct(aTHX_ (ST(0) = sv_newmortal()),
					    sizeof(struct termios), packname);
	    /* The previous implementation stored a pointer to an uninitialised
	       struct termios. Seems safer to initialise it, particularly as
	       this implementation exposes the struct to prying from perl-space.
	    */
	    memset(p, 0, 1 + sizeof(struct termios));
	    XSRETURN(1);
#else
	    not_here("termios");
#endif
	}

SysRet
getattr(termios_ref, fd = 0)
	POSIX::Termios	termios_ref
	POSIX::Fd		fd
    CODE:
	RETVAL = tcgetattr(fd, termios_ref);
    OUTPUT:
	RETVAL

    # If we define TCSANOW here then both a found and not found constant sub
    # are created causing a Constant subroutine TCSANOW redefined warning

#ifndef TCSANOW
#  define DEF_SETATTR_ACTION 0
#else
#  define DEF_SETATTR_ACTION TCSANOW
#endif
SysRet
setattr(termios_ref, fd = 0, optional_actions = DEF_SETATTR_ACTION)
	POSIX::Termios	termios_ref
	POSIX::Fd	fd
	int		optional_actions
    CODE:
	/* The second argument to the call is mandatory, but we'd like to give
	   it a useful default. 0 isn't valid on all operating systems - on
           Solaris (at least) TCSANOW, TCSADRAIN and TCSAFLUSH have the same
           values as the equivalent ioctls, TCSETS, TCSETSW and TCSETSF.  */
	if (optional_actions < 0) {
            SETERRNO(EINVAL, LIB_INVARG);
            RETVAL = -1;
        } else {
            RETVAL = tcsetattr(fd, optional_actions, termios_ref);
        }
    OUTPUT:
	RETVAL

speed_t
getispeed(termios_ref)
	POSIX::Termios	termios_ref
    ALIAS:
	getospeed = 1
    CODE:
	RETVAL = ix ? cfgetospeed(termios_ref) : cfgetispeed(termios_ref);
    OUTPUT:
	RETVAL

tcflag_t
getiflag(termios_ref)
	POSIX::Termios	termios_ref
    ALIAS:
	getoflag = 1
	getcflag = 2
	getlflag = 3
    CODE:
#ifdef I_TERMIOS /* References a termios structure member so ifdef it out. */
	switch(ix) {
	case 0:
	    RETVAL = termios_ref->c_iflag;
	    break;
	case 1:
	    RETVAL = termios_ref->c_oflag;
	    break;
	case 2:
	    RETVAL = termios_ref->c_cflag;
	    break;
	case 3:
	    RETVAL = termios_ref->c_lflag;
	    break;
        default:
	    RETVAL = 0; /* silence compiler warning */
	}
#else
	not_here(GvNAME(CvGV(cv)));
	RETVAL = 0;
#endif
    OUTPUT:
	RETVAL

cc_t
getcc(termios_ref, ccix)
	POSIX::Termios	termios_ref
	unsigned int	ccix
    CODE:
#ifdef I_TERMIOS /* References a termios structure member so ifdef it out. */
	if (ccix >= NCCS)
	    croak("Bad getcc subscript");
	RETVAL = termios_ref->c_cc[ccix];
#else
     not_here("getcc");
     RETVAL = 0;
#endif
    OUTPUT:
	RETVAL

SysRet
setispeed(termios_ref, speed)
	POSIX::Termios	termios_ref
	speed_t		speed
    ALIAS:
	setospeed = 1
    CODE:
	RETVAL = ix
	    ? cfsetospeed(termios_ref, speed) : cfsetispeed(termios_ref, speed);
    OUTPUT:
	RETVAL

void
setiflag(termios_ref, flag)
	POSIX::Termios	termios_ref
	tcflag_t	flag
    ALIAS:
	setoflag = 1
	setcflag = 2
	setlflag = 3
    CODE:
#ifdef I_TERMIOS /* References a termios structure member so ifdef it out. */
	switch(ix) {
	case 0:
	    termios_ref->c_iflag = flag;
	    break;
	case 1:
	    termios_ref->c_oflag = flag;
	    break;
	case 2:
	    termios_ref->c_cflag = flag;
	    break;
	case 3:
	    termios_ref->c_lflag = flag;
	    break;
	}
#else
	not_here(GvNAME(CvGV(cv)));
#endif

void
setcc(termios_ref, ccix, cc)
	POSIX::Termios	termios_ref
	unsigned int	ccix
	cc_t		cc
    CODE:
#ifdef I_TERMIOS /* References a termios structure member so ifdef it out. */
	if (ccix >= NCCS)
	    croak("Bad setcc subscript");
	termios_ref->c_cc[ccix] = cc;
#else
	    not_here("setcc");
#endif


MODULE = POSIX		PACKAGE = POSIX

INCLUDE: const-xs.inc

int
WEXITSTATUS(status)
	int status
    ALIAS:
	POSIX::WIFEXITED = 1
	POSIX::WIFSIGNALED = 2
	POSIX::WIFSTOPPED = 3
	POSIX::WSTOPSIG = 4
	POSIX::WTERMSIG = 5
    CODE:
#if !defined(WEXITSTATUS) || !defined(WIFEXITED) || !defined(WIFSIGNALED) \
      || !defined(WIFSTOPPED) || !defined(WSTOPSIG) || !defined(WTERMSIG)
        RETVAL = 0; /* Silence compilers that notice this, but don't realise
		       that not_here() can't return.  */
#endif
	switch(ix) {
	case 0:
#ifdef WEXITSTATUS
	    RETVAL = WEXITSTATUS(WMUNGE(status));
#else
	    not_here("WEXITSTATUS");
#endif
	    break;
	case 1:
#ifdef WIFEXITED
	    RETVAL = WIFEXITED(WMUNGE(status));
#else
	    not_here("WIFEXITED");
#endif
	    break;
	case 2:
#ifdef WIFSIGNALED
	    RETVAL = WIFSIGNALED(WMUNGE(status));
#else
	    not_here("WIFSIGNALED");
#endif
	    break;
	case 3:
#ifdef WIFSTOPPED
	    RETVAL = WIFSTOPPED(WMUNGE(status));
#else
	    not_here("WIFSTOPPED");
#endif
	    break;
	case 4:
#ifdef WSTOPSIG
	    RETVAL = WSTOPSIG(WMUNGE(status));
#else
	    not_here("WSTOPSIG");
#endif
	    break;
	case 5:
#ifdef WTERMSIG
	    RETVAL = WTERMSIG(WMUNGE(status));
#else
	    not_here("WTERMSIG");
#endif
	    break;
	default:
	    croak("Illegal alias %d for POSIX::W*", (int)ix);
	}
    OUTPUT:
	RETVAL

SysRet
open(filename, flags = O_RDONLY, mode = 0666)
	char *		filename
	int		flags
	Mode_t		mode
    CODE:
	if (flags & (O_APPEND|O_CREAT|O_TRUNC|O_RDWR|O_WRONLY|O_EXCL))
	    TAINT_PROPER("open");
	RETVAL = open(filename, flags, mode);
    OUTPUT:
	RETVAL


HV *
localeconv()
    CODE:
#ifndef HAS_LOCALECONV
        RETVAL = NULL;
        not_here("localeconv");
#else
        RETVAL = Perl_localeconv(aTHX);
#endif  /* HAS_LOCALECONV */
    OUTPUT:
	RETVAL

char *
setlocale(category, locale = 0)
	int		category
	const char *    locale
    PREINIT:
	char *		retval;
    CODE:
	retval = (char *) Perl_setlocale(category, locale);
        if (! retval) {
            XSRETURN_UNDEF;
        }

        RETVAL = retval;
    OUTPUT:
	RETVAL

NV
acos(x)
	NV		x
    ALIAS:
	acosh = 1
	asin = 2
	asinh = 3
	atan = 4
	atanh = 5
	cbrt = 6
	ceil = 7
	cosh = 8
	erf = 9
	erfc = 10
	exp2 = 11
	expm1 = 12
	floor = 13
	j0 = 14
	j1 = 15
	lgamma = 16
	log10 = 17
	log1p = 18
	log2 = 19
	logb = 20
	nearbyint = 21
	rint = 22
	round = 23
	sinh = 24
	tan = 25
	tanh = 26
	tgamma = 27
	trunc = 28
	y0 = 29
	y1 = 30
    CODE:
	PERL_UNUSED_VAR(x);
#ifdef NV_NAN
	RETVAL = NV_NAN;
#else
	RETVAL = 0;
#endif
	switch (ix) {
	case 0:
	    RETVAL = Perl_acos(x); /* C89 math */
	    break;
	case 1:
#ifdef c99_acosh
	    RETVAL = c99_acosh(x);
#else
	    not_here("acosh");
#endif
	    break;
	case 2:
	    RETVAL = Perl_asin(x); /* C89 math */
	    break;
	case 3:
#ifdef c99_asinh
	    RETVAL = c99_asinh(x);
#else
	    not_here("asinh");
#endif
	    break;
	case 4:
	    RETVAL = Perl_atan(x); /* C89 math */
	    break;
	case 5:
#ifdef c99_atanh
	    RETVAL = c99_atanh(x);
#else
	    not_here("atanh");
#endif
	    break;
	case 6:
#ifdef c99_cbrt
	    RETVAL = c99_cbrt(x);
#else
	    not_here("cbrt");
#endif
	    break;
	case 7:
	    RETVAL = Perl_ceil(x); /* C89 math */
	    break;
	case 8:
	    RETVAL = Perl_cosh(x); /* C89 math */
	    break;
	case 9:
#ifdef c99_erf
	    RETVAL = c99_erf(x);
#else
	    not_here("erf");
#endif
	    break;
	case 10:
#ifdef c99_erfc
	    RETVAL = c99_erfc(x);
#else
	    not_here("erfc");
#endif
	    break;
	case 11:
#ifdef c99_exp2
	    RETVAL = c99_exp2(x);
#else
	    not_here("exp2");
#endif
	    break;
	case 12:
#ifdef c99_expm1
	    RETVAL = c99_expm1(x);
#else
	    not_here("expm1");
#endif
	    break;
	case 13:
	    RETVAL = Perl_floor(x); /* C89 math */
	    break;
	case 14:
#ifdef bessel_j0
	    RETVAL = bessel_j0(x);
#else
	    not_here("j0");
#endif
	    break;
	case 15:
#ifdef bessel_j1
	    RETVAL = bessel_j1(x);
#else
	    not_here("j1");
#endif
	    break;
	case 16:
        /* XXX Note: the lgamma modifies a global variable (signgam),
         * which is evil.  Some platforms have lgamma_r, which has
         * extra output parameter instead of the global variable. */
#ifdef c99_lgamma
	    RETVAL = c99_lgamma(x);
#else
	    not_here("lgamma");
#endif
	    break;
	case 17:
	    RETVAL = Perl_log10(x); /* C89 math */
	    break;
	case 18:
#ifdef c99_log1p
	    RETVAL = c99_log1p(x);
#else
	    not_here("log1p");
#endif
	    break;
	case 19:
#ifdef c99_log2
	    RETVAL = c99_log2(x);
#else
	    not_here("log2");
#endif
	    break;
	case 20:
#ifdef c99_logb
	    RETVAL = c99_logb(x);
#elif defined(c99_log2) && FLT_RADIX == 2
	    RETVAL = Perl_floor(c99_log2(PERL_ABS(x)));
#else
	    not_here("logb");
#endif
	    break;
	case 21:
#ifdef c99_nearbyint
	    RETVAL = c99_nearbyint(x);
#else
	    not_here("nearbyint");
#endif
	    break;
	case 22:
#ifdef c99_rint
	    RETVAL = c99_rint(x);
#else
	    not_here("rint");
#endif
	    break;
	case 23:
#ifdef c99_round
	    RETVAL = c99_round(x);
#else
	    not_here("round");
#endif
	    break;
	case 24:
	    RETVAL = Perl_sinh(x); /* C89 math */
	    break;
	case 25:
	    RETVAL = Perl_tan(x); /* C89 math */
	    break;
	case 26:
	    RETVAL = Perl_tanh(x); /* C89 math */
	    break;
	case 27:
#ifdef c99_tgamma
	    RETVAL = c99_tgamma(x);
#else
	    not_here("tgamma");
#endif
	    break;
	case 28:
#ifdef c99_trunc
	    RETVAL = c99_trunc(x);
#else
	    not_here("trunc");
#endif
	    break;
	case 29:
#ifdef bessel_y0
	    RETVAL = bessel_y0(x);
#else
	    not_here("y0");
#endif
	    break;
        case 30:
	default:
#ifdef bessel_y1
	    RETVAL = bessel_y1(x);
#else
	    not_here("y1");
#endif
	}
    OUTPUT:
	RETVAL

IV
fegetround()
    PROTOTYPE:
    ALIAS:
        FLT_ROUNDS = 1
    CODE:
        switch (ix) {
        case 0:
        default:
#ifdef HAS_FEGETROUND
            RETVAL = my_fegetround();
#else
            RETVAL = -1;
            not_here("fegetround");
#endif
            break;
        case 1:
#if defined(FLT_ROUNDS) && !defined(BROKEN_FLT_ROUNDS)
            RETVAL = FLT_ROUNDS;
#elif defined(HAS_FEGETROUND) || defined(HAS_FPGETROUND) || defined(__osf__)
            switch (my_fegetround()) {
                /* C standard seems to say that each of the FE_* macros is
                   defined if and only if the implementation supports it. */
#  ifdef FE_TOWARDZERO
            case FE_TOWARDZERO: RETVAL = 0;  break;
#  endif
#  ifdef FE_TONEAREST
            case FE_TONEAREST:  RETVAL = 1;  break;
#  endif
#  ifdef FE_UPWARD
            case FE_UPWARD:     RETVAL = 2;  break;
#  endif
#  ifdef FE_DOWNWARD
            case FE_DOWNWARD:   RETVAL = 3;  break;
#  endif
            default:            RETVAL = -1; break;
            }
#else
            RETVAL = -1;
            not_here("FLT_ROUNDS");
#endif
            break;
        }
    OUTPUT:
	RETVAL

IV
fesetround(x)
	IV	x
    CODE:
#ifdef HAS_FEGETROUND /* canary for fesetround */
	RETVAL = fesetround(x);
#elif defined(HAS_FPGETROUND) /* canary for fpsetround */
	switch (x) {
	case FE_TONEAREST:  RETVAL = fpsetround(FP_RN); break;
	case FE_TOWARDZERO: RETVAL = fpsetround(FP_RZ); break;
	case FE_DOWNWARD:   RETVAL = fpsetround(FP_RM); break;
	case FE_UPWARD:     RETVAL = fpsetround(FP_RP); break;
        default: RETVAL = -1; break;
	}
#elif defined(__osf__) /* Tru64 */
	switch (x) {
	case FE_TONEAREST:  RETVAL = write_rnd(FP_RND_RN); break;
	case FE_TOWARDZERO: RETVAL = write_rnd(FP_RND_RZ); break;
	case FE_DOWNWARD:   RETVAL = write_rnd(FP_RND_RM); break;
	case FE_UPWARD:     RETVAL = write_rnd(FP_RND_RP); break;
        default: RETVAL = -1; break;
	}
#else
	PERL_UNUSED_VAR(x);
	RETVAL = -1;
	not_here("fesetround");
#endif
    OUTPUT:
	RETVAL

IV
fpclassify(x)
	NV		x
    ALIAS:
	ilogb = 1
	isfinite = 2
	isinf = 3
	isnan = 4
	isnormal = 5
	lrint = 6
	lround = 7
        signbit = 8
    CODE:
        PERL_UNUSED_VAR(x);
	RETVAL = -1;
	switch (ix) {
	case 0:
#ifdef c99_fpclassify
	    RETVAL = c99_fpclassify(x);
#else
	    not_here("fpclassify");
#endif
	    break;
	case 1:
#ifdef c99_ilogb
	    RETVAL = c99_ilogb(x);
#else
	    not_here("ilogb");
#endif
	    break;
	case 2:
	    RETVAL = Perl_isfinite(x);
	    break;
	case 3:
	    RETVAL = Perl_isinf(x);
	    break;
	case 4:
	    RETVAL = Perl_isnan(x);
	    break;
	case 5:
#ifdef c99_isnormal
	    RETVAL = c99_isnormal(x);
#else
	    not_here("isnormal");
#endif
	    break;
	case 6:
#ifdef c99_lrint
	    RETVAL = c99_lrint(x);
#else
	    not_here("lrint");
#endif
	    break;
	case 7:
#ifdef c99_lround
	    RETVAL = c99_lround(x);
#else
	    not_here("lround");
#endif
	    break;
	case 8:
	default:
	    RETVAL = Perl_signbit(x);
	    break;
	}
    OUTPUT:
	RETVAL

NV
getpayload(nv)
	NV nv
    CODE:
#ifdef DOUBLE_HAS_NAN
	RETVAL = S_getpayload(nv);
#else
        PERL_UNUSED_VAR(nv);
        RETVAL = 0.0;
	not_here("getpayload");
#endif
    OUTPUT:
	RETVAL

void
setpayload(nv, payload)
	NV nv
	NV payload
    CODE:
#ifdef DOUBLE_HAS_NAN
	S_setpayload(&nv, payload, FALSE);
#else
        PERL_UNUSED_VAR(nv);
        PERL_UNUSED_VAR(payload);
	not_here("setpayload");
#endif
    OUTPUT:
	nv

void
setpayloadsig(nv, payload)
	NV nv
	NV payload
    CODE:
#ifdef DOUBLE_HAS_NAN
	nv = NV_NAN;
	S_setpayload(&nv, payload, TRUE);
#else
        PERL_UNUSED_VAR(nv);
        PERL_UNUSED_VAR(payload);
	not_here("setpayloadsig");
#endif
    OUTPUT:
	nv

int
issignaling(nv)
	NV nv
    CODE:
#ifdef DOUBLE_HAS_NAN
	RETVAL = Perl_isnan(nv) && NV_NAN_IS_SIGNALING(&nv);
#else
        PERL_UNUSED_VAR(nv);
        RETVAL = 0.0;
	not_here("issignaling");
#endif
    OUTPUT:
	RETVAL

NV
copysign(x,y)
	NV		x
	NV		y
    ALIAS:
	fdim = 1
	fmax = 2
	fmin = 3
	fmod = 4
	hypot = 5
	isgreater = 6
	isgreaterequal = 7
	isless = 8
	islessequal = 9
	islessgreater = 10
	isunordered = 11
	nextafter = 12
	nexttoward = 13
	remainder = 14
    CODE:
        PERL_UNUSED_VAR(x);
        PERL_UNUSED_VAR(y);
#ifdef NV_NAN
	RETVAL = NV_NAN;
#else
	RETVAL = 0;
#endif
	switch (ix) {
	case 0:
#ifdef c99_copysign
	    RETVAL = c99_copysign(x, y);
#else
	    not_here("copysign");
#endif
	    break;
	case 1:
#ifdef c99_fdim
	    RETVAL = c99_fdim(x, y);
#else
	    not_here("fdim");
#endif
	    break;
	case 2:
#ifdef c99_fmax
	    RETVAL = c99_fmax(x, y);
#else
	    not_here("fmax");
#endif
	    break;
	case 3:
#ifdef c99_fmin
	    RETVAL = c99_fmin(x, y);
#else
	    not_here("fmin");
#endif
	    break;
	case 4:
	    RETVAL = Perl_fmod(x, y); /* C89 math */
	    break;
	case 5:
#ifdef c99_hypot
	    RETVAL = c99_hypot(x, y);
#else
	    not_here("hypot");
#endif
	    break;
	case 6:
#ifdef c99_isgreater
	    RETVAL = c99_isgreater(x, y);
#else
	    not_here("isgreater");
#endif
	    break;
	case 7:
#ifdef c99_isgreaterequal
	    RETVAL = c99_isgreaterequal(x, y);
#else
	    not_here("isgreaterequal");
#endif
	    break;
	case 8:
#ifdef c99_isless
	    RETVAL = c99_isless(x, y);
#else
	    not_here("isless");
#endif
	    break;
	case 9:
#ifdef c99_islessequal
	    RETVAL = c99_islessequal(x, y);
#else
	    not_here("islessequal");
#endif
	    break;
	case 10:
#ifdef c99_islessgreater
	    RETVAL = c99_islessgreater(x, y);
#else
	    not_here("islessgreater");
#endif
	    break;
	case 11:
#ifdef c99_isunordered
	    RETVAL = c99_isunordered(x, y);
#else
	    not_here("isunordered");
#endif
	    break;
	case 12:
#ifdef c99_nextafter
	    RETVAL = c99_nextafter(x, y);
#else
	    not_here("nextafter");
#endif
	    break;
	case 13:
#ifdef c99_nexttoward
	    RETVAL = c99_nexttoward(x, y);
#else
	    not_here("nexttoward");
#endif
	    break;
	case 14:
	default:
#ifdef c99_remainder
          RETVAL = c99_remainder(x, y);
#else
          not_here("remainder");
#endif
	    break;
	}
	OUTPUT:
	    RETVAL

void
frexp(x)
	NV		x
    PPCODE:
	int expvar;
	/* (We already know stack is long enough.) */
	PUSHs(sv_2mortal(newSVnv(Perl_frexp(x,&expvar)))); /* C89 math */
	PUSHs(sv_2mortal(newSViv(expvar)));

NV
ldexp(x,exp)
	NV		x
	int		exp
    CODE:
        RETVAL = Perl_ldexp(x, exp);
    OUTPUT:
        RETVAL

void
modf(x)
	NV		x
    PPCODE:
	NV intvar;
	/* (We already know stack is long enough.) */
	PUSHs(sv_2mortal(newSVnv(Perl_modf(x,&intvar)))); /* C89 math */
	PUSHs(sv_2mortal(newSVnv(intvar)));

void
remquo(x,y)
	NV		x
	NV		y
    PPCODE:
#ifdef c99_remquo
        int intvar;
        PUSHs(sv_2mortal(newSVnv(c99_remquo(x,y,&intvar))));
        PUSHs(sv_2mortal(newSVnv(intvar)));
#else
	PERL_UNUSED_VAR(x);
	PERL_UNUSED_VAR(y);
	not_here("remquo");
#endif

NV
scalbn(x,y)
	NV		x
	IV		y
    CODE:
#ifdef c99_scalbn
	RETVAL = c99_scalbn(x, y);
#else
	PERL_UNUSED_VAR(x);
	PERL_UNUSED_VAR(y);
	RETVAL = NV_NAN;
	not_here("scalbn");
#endif
    OUTPUT:
	RETVAL

NV
fma(x,y,z)
	NV		x
	NV		y
	NV		z
    CODE:
#ifdef c99_fma
	RETVAL = c99_fma(x, y, z);
#else
	PERL_UNUSED_VAR(x);
	PERL_UNUSED_VAR(y);
	PERL_UNUSED_VAR(z);
	not_here("fma");
#endif
    OUTPUT:
	RETVAL

NV
nan(payload = 0)
	NV payload
    CODE:
#ifdef NV_NAN
        /* If no payload given, just return the default NaN.
         * This makes a difference in platforms where the default
         * NaN is not all zeros. */
	if (items == 0) {
          RETVAL = NV_NAN;
	} else {
          S_setpayload(&RETVAL, payload, FALSE);
        }
#elif defined(c99_nan)
	{
	  STRLEN elen = my_snprintf(PL_efloatbuf, PL_efloatsize, "%g", payload);
          if ((IV)elen == -1) {
#ifdef NV_NAN
	    RETVAL = NV_NAN;
#else            
            RETVAL = 0.0;
            not_here("nan");
#endif
          } else {
            RETVAL = c99_nan(PL_efloatbuf);
          }
        }
#else
	not_here("nan");
#endif
    OUTPUT:
	RETVAL

NV
jn(x,y)
	IV		x
	NV		y
    ALIAS:
	yn = 1
    CODE:
#ifdef NV_NAN
	RETVAL = NV_NAN;
#else
	RETVAL = 0;
#endif
        switch (ix) {
	case 0:
#ifdef bessel_jn
          RETVAL = bessel_jn(x, y);
#else
	  PERL_UNUSED_VAR(x);
	  PERL_UNUSED_VAR(y);
          not_here("jn");
#endif
            break;
	case 1:
	default:
#ifdef bessel_yn
          RETVAL = bessel_yn(x, y);
#else
	  PERL_UNUSED_VAR(x);
	  PERL_UNUSED_VAR(y);
          not_here("yn");
#endif
            break;
	}
    OUTPUT:
	RETVAL

SysRet
sigaction(sig, optaction, oldaction = 0)
	int			sig
	SV *			optaction
	POSIX::SigAction	oldaction
    CODE:
#if defined(WIN32) || (defined(__amigaos4__) && defined(__NEWLIB__))
	RETVAL = not_here("sigaction");
#else
# This code is really grody because we are trying to make the signal
# interface look beautiful, which is hard.

	{
	    POSIX__SigAction action;
	    GV *siggv = gv_fetchpvs("SIG", GV_ADD, SVt_PVHV);
	    struct sigaction act;
	    struct sigaction oact;
	    sigset_t sset;
	    SV *osset_sv;
	    sigset_t osset;
	    POSIX__SigSet sigset;
	    SV** svp;
	    SV** sigsvp;

            if (sig < 0) {
                croak("Negative signals are not allowed");
            }

	    if (sig == 0 && SvPOK(ST(0))) {
	        const char *s = SvPVX_const(ST(0));
		int i = whichsig(s);

	        if (i < 0 && memBEGINs(s, SvCUR(ST(0)), "SIG"))
		    i = whichsig(s + 3);
	        if (i < 0) {
	            if (ckWARN(WARN_SIGNAL))
		        Perl_warner(aTHX_ packWARN(WARN_SIGNAL),
                                    "No such signal: SIG%s", s);
	            XSRETURN_UNDEF;
		}
	        else
		    sig = i;
            }
#ifdef NSIG
	    if (sig > NSIG) { /* NSIG - 1 is still okay. */
	        Perl_warner(aTHX_ packWARN(WARN_SIGNAL),
                            "No such signal: %d", sig);
	        XSRETURN_UNDEF;
	    }
#endif
	    sigsvp = hv_fetch(GvHVn(siggv),
			      PL_sig_name[sig],
			      strlen(PL_sig_name[sig]),
			      TRUE);

	    /* Check optaction and set action */
	    if(SvTRUE(optaction)) {
		if(sv_isa(optaction, "POSIX::SigAction"))
			action = (HV*)SvRV(optaction);
		else
			croak("action is not of type POSIX::SigAction");
	    }
	    else {
		action=0;
	    }

	    /* sigaction() is supposed to look atomic. In particular, any
	     * signal handler invoked during a sigaction() call should
	     * see either the old or the new disposition, and not something
	     * in between. We use sigprocmask() to make it so.
	     */
	    sigfillset(&sset);
	    RETVAL=sigprocmask(SIG_BLOCK, &sset, &osset);
	    if(RETVAL == -1)
               XSRETURN_UNDEF;
	    ENTER;
	    /* Restore signal mask no matter how we exit this block. */
	    osset_sv = newSVpvn((char *)(&osset), sizeof(sigset_t));
	    SAVEFREESV( osset_sv );
	    SAVEDESTRUCTOR_X(restore_sigmask, osset_sv);

	    RETVAL=-1; /* In case both oldaction and action are 0. */

	    /* Remember old disposition if desired. */
	    if (oldaction) {
                int safe;

		svp = hv_fetchs(oldaction, "HANDLER", TRUE);
		if(!svp)
		    croak("Can't supply an oldaction without a HANDLER");
		if(SvTRUE(*sigsvp)) { /* TBD: what if "0"? */
			sv_setsv(*svp, *sigsvp);
		}
		else {
			sv_setpvs(*svp, "DEFAULT");
		}
		RETVAL = sigaction(sig, (struct sigaction *)0, & oact);
		if(RETVAL == -1) {
                   LEAVE;
                   XSRETURN_UNDEF;
                }
		/* Get back the mask. */
		svp = hv_fetchs(oldaction, "MASK", TRUE);
		if (sv_isa(*svp, "POSIX::SigSet")) {
		    sigset = (sigset_t *) SvPV_nolen(SvRV(*svp));
		}
		else {
		    sigset = (sigset_t *) allocate_struct(aTHX_ *svp,
							  sizeof(sigset_t),
							  "POSIX::SigSet");
		}
		*sigset = oact.sa_mask;

		/* Get back the flags. */
		svp = hv_fetchs(oldaction, "FLAGS", TRUE);
		sv_setiv(*svp, oact.sa_flags);

		/* Get back whether the old handler used safe signals;
                 * i.e. it used Perl_csighandler[13] rather than
                 * Perl_sighandler[13]
                 */
                safe =
#ifdef SA_SIGINFO
                    (oact.sa_flags & SA_SIGINFO)
                        ? (  oact.sa_sigaction == PL_csighandler3p
#ifdef PERL_USE_3ARG_SIGHANDLER
                          || oact.sa_sigaction == PL_csighandlerp
#endif
                          )
                        :
#endif
                           (  oact.sa_handler   == PL_csighandler1p
#ifndef PERL_USE_3ARG_SIGHANDLER
                          || oact.sa_handler   == PL_csighandlerp
#endif
                           );

		svp = hv_fetchs(oldaction, "SAFE", TRUE);
		sv_setiv(*svp, safe);
	    }

	    if (action) {
                int safe;

		/* Set up any desired flags. */
		svp = hv_fetchs(action, "FLAGS", FALSE);
		act.sa_flags = svp ? SvIV(*svp) : 0;

		/* Safe signals use "csighandler", which vectors through the
		   PL_sighandlerp pointer when it's safe to do so.
		   (BTW, "csighandler" is very different from "sighandler".) */
		svp = hv_fetchs(action, "SAFE", FALSE);
                safe = *svp && SvTRUE(*svp);
#ifdef SA_SIGINFO
                if (act.sa_flags & SA_SIGINFO) {
                    /* 3-arg handler */
                    act.sa_sigaction =
			    safe ? PL_csighandler3p : PL_sighandler3p;
                }
                else
#endif
                {
                    /* 1-arg handler */
                    act.sa_handler =
			    safe ? PL_csighandler1p : PL_sighandler1p;
                }

		/* Vector new Perl handler through %SIG.
		   (The core signal handlers read %SIG to dispatch.) */
		svp = hv_fetchs(action, "HANDLER", FALSE);
		if (!svp)
		    croak("Can't supply an action without a HANDLER");
		sv_setsv(*sigsvp, *svp);

		/* This call actually calls sigaction() with almost the
		   right settings, including appropriate interpretation
		   of DEFAULT and IGNORE.  However, why are we doing
		   this when we're about to do it again just below?  XXX */
		SvSETMAGIC(*sigsvp);

		/* And here again we duplicate -- DEFAULT/IGNORE checking. */
		if(SvPOK(*svp)) {
			const char *s=SvPVX_const(*svp);
			if(strEQ(s,"IGNORE")) {
				act.sa_handler = SIG_IGN;
			}
			else if(strEQ(s,"DEFAULT")) {
				act.sa_handler = SIG_DFL;
			}
		}

		/* Set up any desired mask. */
		svp = hv_fetchs(action, "MASK", FALSE);
		if (svp && sv_isa(*svp, "POSIX::SigSet")) {
		    sigset = (sigset_t *) SvPV_nolen(SvRV(*svp));
		    act.sa_mask = *sigset;
		}
		else
		    sigemptyset(& act.sa_mask);

		/* Don't worry about cleaning up *sigsvp if this fails,
		 * because that means we tried to disposition a
		 * nonblockable signal, in which case *sigsvp is
		 * essentially meaningless anyway.
		 */
		RETVAL = sigaction(sig, & act, (struct sigaction *)0);
		if(RETVAL == -1) {
                    LEAVE;
		    XSRETURN_UNDEF;
                }
	    }

	    LEAVE;
	}
#endif
    OUTPUT:
	RETVAL

SysRet
sigpending(sigset)
	POSIX::SigSet		sigset
    ALIAS:
	sigsuspend = 1
    CODE:
#ifdef __amigaos4__
	RETVAL = not_here("sigpending");
#else
	RETVAL = ix ? sigsuspend(sigset) : sigpending(sigset);
#endif
    OUTPUT:
	RETVAL
    CLEANUP:
    PERL_ASYNC_CHECK();

SysRet
sigprocmask(how, sigset, oldsigset = 0)
	int			how
	POSIX::SigSet		sigset = NO_INIT
	POSIX::SigSet		oldsigset = NO_INIT
INIT:
	if (! SvOK(ST(1))) {
	    sigset = NULL;
	} else if (sv_isa(ST(1), "POSIX::SigSet")) {
	    sigset = (sigset_t *) SvPV_nolen(SvRV(ST(1)));
	} else {
	    croak("sigset is not of type POSIX::SigSet");
	}

	if (items < 3 || ! SvOK(ST(2))) {
	    oldsigset = NULL;
	} else if (sv_isa(ST(2), "POSIX::SigSet")) {
	    oldsigset = (sigset_t *) SvPV_nolen(SvRV(ST(2)));
	} else {
	    croak("oldsigset is not of type POSIX::SigSet");
	}

void
_exit(status)
	int		status

SysRet
dup2(fd1, fd2)
	int		fd1
	int		fd2
    CODE:
	if (fd1 >= 0 && fd2 >= 0) {
#ifdef WIN32
            /* RT #98912 - More Microsoft muppetry - failing to
               actually implemented the well known documented POSIX
               behaviour for a POSIX API.
               http://msdn.microsoft.com/en-us/library/8syseb29.aspx  */
            RETVAL = dup2(fd1, fd2) == -1 ? -1 : fd2;
#else
            RETVAL = dup2(fd1, fd2);
#endif
        } else {
            SETERRNO(EBADF,RMS_IFI);
            RETVAL = -1;
        }
    OUTPUT:
	RETVAL

SV *
lseek(fd, offset, whence)
	POSIX::Fd	fd
	Off_t		offset
	int		whence
    CODE:
	{
              Off_t pos = PerlLIO_lseek(fd, offset, whence);
              RETVAL = sizeof(Off_t) > sizeof(IV)
                ? newSVnv((NV)pos) : newSViv((IV)pos);
        }
    OUTPUT:
	RETVAL

void
nice(incr)
	int		incr
    PPCODE:
	errno = 0;
	if ((incr = nice(incr)) != -1 || errno == 0) {
	    if (incr == 0)
		XPUSHs(newSVpvs_flags("0 but true", SVs_TEMP));
	    else
		XPUSHs(sv_2mortal(newSViv(incr)));
	}

void
pipe()
    PPCODE:
	int fds[2];
	if (pipe(fds) != -1) {
	    EXTEND(SP,2);
	    PUSHs(sv_2mortal(newSViv(fds[0])));
	    PUSHs(sv_2mortal(newSViv(fds[1])));
	}

SysRet
read(fd, buffer, nbytes)
    PREINIT:
        SV *sv_buffer = SvROK(ST(1)) ? SvRV(ST(1)) : ST(1);
    INPUT:
	POSIX::Fd	fd
        size_t          nbytes
        char *          buffer = sv_grow( sv_buffer, nbytes+1 );
    CLEANUP:
        if (RETVAL >= 0) {
            SvCUR_set(sv_buffer, RETVAL);
            SvPOK_only(sv_buffer);
            *SvEND(sv_buffer) = '\0';
            SvTAINTED_on(sv_buffer);
        }

SysRet
setpgid(pid, pgid)
	pid_t		pid
	pid_t		pgid

pid_t
setsid()

pid_t
tcgetpgrp(fd)
	POSIX::Fd	fd

SysRet
tcsetpgrp(fd, pgrp_id)
	POSIX::Fd	fd
	pid_t		pgrp_id

void
uname()
    PPCODE:
#ifdef HAS_UNAME
	struct utsname buf;
	if (uname(&buf) >= 0) {
	    EXTEND(SP, 5);
	    PUSHs(newSVpvn_flags(buf.sysname, strlen(buf.sysname), SVs_TEMP));
	    PUSHs(newSVpvn_flags(buf.nodename, strlen(buf.nodename), SVs_TEMP));
	    PUSHs(newSVpvn_flags(buf.release, strlen(buf.release), SVs_TEMP));
	    PUSHs(newSVpvn_flags(buf.version, strlen(buf.version), SVs_TEMP));
	    PUSHs(newSVpvn_flags(buf.machine, strlen(buf.machine), SVs_TEMP));
	}
#else
	uname((char *) 0); /* A stub to call not_here(). */
#endif

SysRet
write(fd, buffer, nbytes)
	POSIX::Fd	fd
	char *		buffer
	size_t		nbytes

void
abort()

#if defined(HAS_MBRLEN) && (defined(USE_ITHREADS) || ! defined(HAS_MBLEN))
#  define USE_MBRLEN
#else
#  undef USE_MBRLEN
#endif

int
mblen(s, n = ~0)
	SV *		s
	size_t		n
    CODE:
        errno = 0;

        SvGETMAGIC(s);
        if (! SvOK(s)) {
#ifdef USE_MBRLEN
            /* Initialize the shift state in PL_mbrlen_ps.  The Standard says
             * that should be all zeros. */
            memzero(&PL_mbrlen_ps, sizeof(PL_mbrlen_ps));
            RETVAL = 0;
#else
            MBLEN_LOCK_;
            RETVAL = mblen(NULL, 0);
            MBLEN_UNLOCK_;
#endif
        }
        else {  /* Not resetting state */
            SV * byte_s = sv_2mortal(newSVsv_nomg(s));
            if (! sv_utf8_downgrade_nomg(byte_s, TRUE)) {
                SETERRNO(EINVAL, LIB_INVARG);
                RETVAL = -1;
            }
            else {
                size_t len;
                char * string = SvPVbyte(byte_s, len);
                if (n < len) len = n;
#ifdef USE_MBRLEN
                MBRLEN_LOCK_;
                RETVAL = (SSize_t) mbrlen(string, len, &PL_mbrlen_ps);
                MBRLEN_UNLOCK_;
                if (RETVAL < 0) RETVAL = -1;    /* Use mblen() ret code for
                                                   transparency */
#else
                /* Locking prevents races, but locales can be switched out
                 * without locking, so this isn't a cure all */
                MBLEN_LOCK_;
                RETVAL = mblen(string, len);
                MBLEN_UNLOCK_;
#endif
            }
        }
    OUTPUT:
        RETVAL

int
mbtowc(pwc, s, n = ~0)
	SV *	        pwc
	SV *		s
	size_t		n
    CODE:
        RETVAL = -1;
#if ! defined(HAS_MBTOWC) && ! defined(HAS_MBRTOWC)
        PERL_UNUSED_ARG(pwc);
        PERL_UNUSED_ARG(s);
        PERL_UNUSED_ARG(n);
#else
        errno = 0;
        SvGETMAGIC(s);
        if (! SvOK(s)) { /* Initialize state */
            mbtowc_(NULL, NULL, 0);
        }
        else {  /* Not resetting state */
            wchar_t wc = 0;
            SV * byte_s = sv_2mortal(newSVsv_nomg(s));
            if (! sv_utf8_downgrade_nomg(byte_s, TRUE)) {
                SETERRNO(EINVAL, LIB_INVARG);
                RETVAL = -1;
            }
            else {
                size_t len;
                char * string = SvPVbyte(byte_s, len);
                if (n < len) len = n;
                RETVAL = mbtowc_(&wc, string, len);
                if (RETVAL >= 0) {
                    sv_setiv_mg(pwc, wc);
                }
                else { /* Use mbtowc() ret code for transparency */
                    RETVAL = -1;
                }
            }
        }
#endif
    OUTPUT:
        RETVAL

#if defined(HAS_WCRTOMB) && (defined(USE_ITHREADS) || ! defined(HAS_WCTOMB))
#  define USE_WCRTOMB
#else
#  undef USE_WCRTOMB
#endif

int
wctomb(s, wchar)
	SV *		s
	wchar_t		wchar
    CODE:
        errno = 0;
        SvGETMAGIC(s);
        if (s == &PL_sv_undef) {
#ifdef USE_WCRTOMB
            /* The man pages khw looked at are in agreement that this works.
             * But probably memzero would too */
            WCRTOMB_LOCK_;
            RETVAL = wcrtomb(NULL, L'\0', &PL_wcrtomb_ps);
            WCRTOMB_UNLOCK_;
#else
            WCTOMB_LOCK_;
            RETVAL = wctomb(NULL, L'\0');
            WCTOMB_UNLOCK_;
#endif
        }
        else {  /* Not resetting state */
            char buffer[MB_LEN_MAX];
#ifdef USE_WCRTOMB
            WCRTOMB_LOCK_;
            RETVAL = wcrtomb(buffer, wchar, &PL_wcrtomb_ps);
            WCRTOMB_UNLOCK_;
#else
            /* Locking prevents races, but locales can be switched out without
             * locking, so this isn't a cure all */
            WCTOMB_LOCK_;
            RETVAL = wctomb(buffer, wchar);
            WCTOMB_UNLOCK_;
#endif
            if (RETVAL >= 0) {
                sv_setpvn_mg(s, buffer, RETVAL);
            }
        }
    OUTPUT:
        RETVAL

int
strcoll(s1, s2)
	char *		s1
	char *		s2
    CODE:
        LC_COLLATE_LOCK;
        RETVAL = strcoll(s1, s2);
        LC_COLLATE_UNLOCK;
    OUTPUT:
        RETVAL

void
strtod(str)
	char *		str
    PREINIT:
	double num;
	char *unparsed;
    PPCODE:
        DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
        STORE_LC_NUMERIC_FORCE_TO_UNDERLYING();
	num = strtod(str, &unparsed);
        RESTORE_LC_NUMERIC();
	PUSHs(sv_2mortal(newSVnv(num)));
	if (GIMME_V == G_LIST) {
	    EXTEND(SP, 1);
	    if (unparsed)
		PUSHs(sv_2mortal(newSViv(strlen(unparsed))));
	    else
		PUSHs(&PL_sv_undef);
	}

#ifdef HAS_STRTOLD

void
strtold(str)
	char *		str
    PREINIT:
	long double num;
	char *unparsed;
    PPCODE:
        DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
        STORE_LC_NUMERIC_FORCE_TO_UNDERLYING();
	num = strtold(str, &unparsed);
        RESTORE_LC_NUMERIC();
	PUSHs(sv_2mortal(newSVnv(num)));
	if (GIMME_V == G_LIST) {
	    EXTEND(SP, 1);
	    if (unparsed)
		PUSHs(sv_2mortal(newSViv(strlen(unparsed))));
	    else
		PUSHs(&PL_sv_undef);
	}

#endif

void
strtol(str, base = 0)
	char *		str
	int		base
    PREINIT:
	long num;
	char *unparsed;
    PPCODE:
	if (base == 0 || inRANGE(base, 2, 36)) {
            num = strtol(str, &unparsed, base);
#if IVSIZE < LONGSIZE
            if (num < IV_MIN || num > IV_MAX)
                PUSHs(sv_2mortal(newSVnv((NV)num)));
            else
#endif
                PUSHs(sv_2mortal(newSViv((IV)num)));
            if (GIMME_V == G_LIST) {
                EXTEND(SP, 1);
                if (unparsed)
                    PUSHs(sv_2mortal(newSViv(strlen(unparsed))));
                else
                    PUSHs(&PL_sv_undef);
            }
        } else {
	    SETERRNO(EINVAL, LIB_INVARG);
            PUSHs(&PL_sv_undef);
            if (GIMME_V == G_LIST) {
               EXTEND(SP, 1);
               PUSHs(&PL_sv_undef);
            }
        }

void
strtoul(str, base = 0)
	const char *	str
	int		base
    PREINIT:
	unsigned long num;
	char *unparsed = NULL;
    PPCODE:
	PERL_UNUSED_VAR(str);
	PERL_UNUSED_VAR(base);
	if (base == 0 || inRANGE(base, 2, 36)) {
            num = strtoul(str, &unparsed, base);
#if UVSIZE < LONGSIZE
            if (num > UV_MAX)
                PUSHs(sv_2mortal(newSVnv((NV)num)));
            else
#endif
                PUSHs(sv_2mortal(newSVuv((UV)num)));
            if (GIMME_V == G_LIST) {
                EXTEND(SP, 1);
                if (unparsed)
                    PUSHs(sv_2mortal(newSViv(strlen(unparsed))));
                else
                  PUSHs(&PL_sv_undef);
            }
	} else {
	    SETERRNO(EINVAL, LIB_INVARG);
            PUSHs(&PL_sv_undef);
            if (GIMME_V == G_LIST) {
               EXTEND(SP, 1);
               PUSHs(&PL_sv_undef);
            }
        }

void
strxfrm(src)
	SV *		src
    CODE:
#ifdef USE_LOCALE_COLLATE
      ST(0) = Perl_strxfrm(aTHX_ src);
#else
      ST(0) = src;
#endif

SysRet
mkfifo(filename, mode)
	char *		filename
	Mode_t		mode
    ALIAS:
	access = 1
    CODE:
	if(ix) {
	    RETVAL = access(filename, mode);
	} else {
	    TAINT_PROPER("mkfifo");
	    RETVAL = mkfifo(filename, mode);
	}
    OUTPUT:
	RETVAL

SysRet
tcdrain(fd)
	POSIX::Fd	fd
    ALIAS:
	close = 1
	dup = 2
    CODE:
	if (fd >= 0) {
	    RETVAL = ix == 1 ? close(fd)
	      : (ix < 1 ? tcdrain(fd) : dup(fd));
	} else {
	    SETERRNO(EBADF,RMS_IFI);
	    RETVAL = -1;
	}
    OUTPUT:
	RETVAL


SysRet
tcflow(fd, action)
	POSIX::Fd	fd
	int		action
    ALIAS:
	tcflush = 1
	tcsendbreak = 2
    CODE:
        if (action >= 0) {
            RETVAL = ix == 1 ? tcflush(fd, action)
              : (ix < 1 ? tcflow(fd, action) : tcsendbreak(fd, action));
        } else {
            SETERRNO(EINVAL,LIB_INVARG);
            RETVAL = -1;
        }
    OUTPUT:
	RETVAL

void
asctime(sec, min, hour, mday, mon, year, wday = 0, yday = 0, isdst = -1)
	int		sec
	int		min
	int		hour
	int		mday
	int		mon
	int		year
	int		wday
	int		yday
	int		isdst
    ALIAS:
	mktime = 1
    PPCODE:
	{
	    dXSTARG;
	    struct tm mytm;
	    init_tm(&mytm);	/* XXX workaround - see init_tm() in core util.c */
	    mytm.tm_sec = sec;
	    mytm.tm_min = min;
	    mytm.tm_hour = hour;
	    mytm.tm_mday = mday;
	    mytm.tm_mon = mon;
	    mytm.tm_year = year;
	    mytm.tm_wday = wday;
	    mytm.tm_yday = yday;
	    mytm.tm_isdst = isdst;
	    if (ix) {
	        time_t result;
                MKTIME_LOCK;
	        result = mktime(&mytm);
                MKTIME_UNLOCK;
		if (result == (time_t)-1)
		    SvOK_off(TARG);
		else if (result == 0)
		    sv_setpvs(TARG, "0 but true");
		else
		    sv_setiv(TARG, (IV)result);
	    } else {
                ASCTIME_LOCK;
		sv_setpv(TARG, asctime(&mytm));
                ASCTIME_UNLOCK;
	    }
	    ST(0) = TARG;
	    XSRETURN(1);
	}

long
clock()

char *
ctime(time)
	Time_t		&time

void
times()
	PPCODE:
	struct tms tms;
	clock_t realtime;
	realtime = times( &tms );
	EXTEND(SP,5);
	PUSHs( sv_2mortal( newSViv( (IV) realtime ) ) );
	PUSHs( sv_2mortal( newSViv( (IV) tms.tms_utime ) ) );
	PUSHs( sv_2mortal( newSViv( (IV) tms.tms_stime ) ) );
	PUSHs( sv_2mortal( newSViv( (IV) tms.tms_cutime ) ) );
	PUSHs( sv_2mortal( newSViv( (IV) tms.tms_cstime ) ) );

double
difftime(time1, time2)
	Time_t		time1
	Time_t		time2

#XXX: if $xsubpp::WantOptimize is always the default
#     sv_setpv(TARG, ...) could be used rather than
#     ST(0) = sv_2mortal(newSVpv(...))
void
strftime(fmt, sec, min, hour, mday, mon, year, wday = -1, yday = -1, isdst = -1)
	SV *		fmt
	int		sec
	int		min
	int		hour
	int		mday
	int		mon
	int		year
	int		wday
	int		yday
	int		isdst
    CODE:
	{
	    char *buf;
            SV *sv;
            utf8ness_t is_utf8;

            /* allowing user-supplied (rather than literal) formats
             * is normally frowned upon as a potential security risk;
             * but this is part of the API so we have to allow it */
            GCC_DIAG_IGNORE_STMT(-Wformat-nonliteral);
	    buf = my_strftime8_temp(SvPV_nolen(fmt), sec, min, hour, mday, mon, year, wday, yday, isdst, &is_utf8);
            GCC_DIAG_RESTORE_STMT;
            sv = sv_newmortal();
	    if (buf) {
                STRLEN len = strlen(buf);
		sv_usepvn_flags(sv, buf, len, SV_HAS_TRAILING_NUL);
		if (SvUTF8(fmt) || is_utf8 == UTF8NESS_YES) {
		    SvUTF8_on(sv);
		}
            }
            else {  /* We can't distinguish between errors and just an empty
                     * return; in all cases just return an empty string */
                SvUPGRADE(sv, SVt_PV);
                SvPV_set(sv, (char *) "");
                SvPOK_on(sv);
                SvCUR_set(sv, 0);
                SvLEN_set(sv, 0);   /* Won't attempt to free the string when sv
                                       gets destroyed */
            }
            ST(0) = sv;
	}

void
tzset()
  PPCODE:
    my_tzset(aTHX);

void
tzname()
    PPCODE:
	EXTEND(SP,2);
        /* It is undefined behavior if another thread is changing this while
         * its being read */
        ENVr_LOCALEr_LOCK;
	PUSHs(newSVpvn_flags(tzname[0], strlen(tzname[0]), SVs_TEMP));
	PUSHs(newSVpvn_flags(tzname[1], strlen(tzname[1]), SVs_TEMP));
        ENVr_LOCALEr_UNLOCK;

char *
ctermid(s = 0)
	char *          s = 0;
    CODE:
#ifdef I_TERMIOS
        /* On some systems L_ctermid is a #define; but not all; this code works
         * for all cases (so far...) */
	s = (char *) safemalloc((size_t) L_ctermid);
#endif
	RETVAL = ctermid(s);
    OUTPUT:
	RETVAL
    CLEANUP:
#ifdef I_TERMIOS
	Safefree(s);
#endif

char *
cuserid(s = 0)
	char *		s = 0;
    CODE:
#ifdef HAS_CUSERID
  RETVAL = cuserid(s);
#else
  PERL_UNUSED_VAR(s);
  RETVAL = 0;
  not_here("cuserid");
#endif
    OUTPUT:
  RETVAL

SysRetLong
fpathconf(fd, name)
	POSIX::Fd	fd
	int		name

SysRetLong
pathconf(filename, name)
	char *		filename
	int		name

SysRet
pause()
    CLEANUP:
    PERL_ASYNC_CHECK();

unsigned int
sleep(seconds)
	unsigned int	seconds
    CODE:
	RETVAL = PerlProc_sleep(seconds);
    OUTPUT:
	RETVAL

SysRet
setgid(gid)
	Gid_t		gid

SysRet
setuid(uid)
	Uid_t		uid

SysRetLong
sysconf(name)
	int		name

char *
ttyname(fd)
	POSIX::Fd	fd

void
getcwd()
    PPCODE:
      {
	dXSTARG;
	getcwd_sv(TARG);
	XSprePUSH; PUSHTARG;
      }

SysRet
lchown(uid, gid, path)
       Uid_t           uid
       Gid_t           gid
       char *          path
    CODE:
#ifdef HAS_LCHOWN
       /* yes, the order of arguments is different,
        * but consistent with CORE::chown() */
       RETVAL = lchown(path, uid, gid);
#else
       PERL_UNUSED_VAR(uid);
       PERL_UNUSED_VAR(gid);
       PERL_UNUSED_VAR(path);
       RETVAL = not_here("lchown");
#endif
    OUTPUT:
       RETVAL
