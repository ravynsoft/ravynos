# math_h.m4 serial 126
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN_ONCE([gl_MATH_H],
[
  AC_REQUIRE([gl_MATH_H_DEFAULTS])
  gl_CHECK_NEXT_HEADERS([math.h])

  AC_CACHE_CHECK([whether NAN macro works], [gl_cv_header_math_nan_works],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <math.h>]],
      [[/* Solaris 10 has a broken definition of NAN.  Other platforms
        fail to provide NAN, or provide it only in C99 mode; this
        test only needs to fail when NAN is provided but wrong.  */
         float f = 1.0f;
#ifdef NAN
         f = NAN;
#endif
         return f == 0;]])],
      [gl_cv_header_math_nan_works=yes],
      [gl_cv_header_math_nan_works=no])])
  if test $gl_cv_header_math_nan_works = no; then
    REPLACE_NAN=1
  fi
  AC_CACHE_CHECK([whether HUGE_VAL works], [gl_cv_header_math_huge_val_works],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <math.h>]],
      [[/* Solaris 10 has a broken definition of HUGE_VAL.  */
         double d = HUGE_VAL;
         return d == 0;]])],
      [gl_cv_header_math_huge_val_works=yes],
      [gl_cv_header_math_huge_val_works=no])])
  if test $gl_cv_header_math_huge_val_works = no; then
    REPLACE_HUGE_VAL=1
  fi

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <math.h>]],
    [acosf acosl asinf asinl atanf atanl
     cbrt cbrtf cbrtl ceilf ceill copysign copysignf copysignl cosf cosl coshf
     expf expl exp2 exp2f exp2l expm1 expm1f expm1l
     fabsf fabsl floorf floorl fma fmaf fmal
     fmod fmodf fmodl frexpf frexpl hypotf hypotl
     ilogb ilogbf ilogbl
     ldexpf ldexpl
     log logf logl log10 log10f log10l log1p log1pf log1pl log2 log2f log2l
     logb logbf logbl
     modf modff modfl powf
     remainder remainderf remainderl
     rint rintf rintl round roundf roundl sinf sinl sinhf sqrtf sqrtl
     tanf tanl tanhf totalorder totalorderf totalorderl trunc truncf truncl])
])

# gl_MATH_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_MATH_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_MATH_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_MATH_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_MATH_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ACOSF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ACOSL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ASINF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ASINL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ATANF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ATANL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ATAN2F])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_CBRT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_CBRTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_CBRTL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_CEIL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_CEILF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_CEILL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_COPYSIGN])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_COPYSIGNF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_COPYSIGNL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_COSF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_COSL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_COSHF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_EXPF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_EXPL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_EXP2])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_EXP2F])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_EXP2L])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_EXPM1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_EXPM1F])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_EXPM1L])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FABSF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FABSL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FLOOR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FLOORF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FLOORL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FMA])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FMAF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FMAL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FMOD])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FMODF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FMODL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FREXPF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FREXP])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FREXPL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_HYPOT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_HYPOTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_HYPOTL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ILOGB])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ILOGBF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ILOGBL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ISFINITE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ISINF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ISNAN])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ISNANF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ISNAND])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ISNANL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LDEXP])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LDEXPF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LDEXPL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOGF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOGL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG10])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG10F])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG10L])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG1P])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG1PF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG1PL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG2])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG2F])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOG2L])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOGB])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOGBF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LOGBL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MODF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MODFF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MODFL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POWF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_REMAINDER])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_REMAINDERF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_REMAINDERL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_RINT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_RINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_RINTL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ROUND])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ROUNDF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ROUNDL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SIGNBIT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SINF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SINL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SINHF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SQRTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SQRTL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TANF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TANL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TANHF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TOTALORDER])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TOTALORDERF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TOTALORDERL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TRUNC])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TRUNCF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TRUNCL])
    dnl Support Microsoft deprecated alias function names by default.
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_J0], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_J1], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_JN], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_Y0], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_Y1], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_YN], [1])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_MATH_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_MATH_H_DEFAULTS])
])

AC_DEFUN([gl_MATH_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_ACOSF=1;                     AC_SUBST([HAVE_ACOSF])
  HAVE_ACOSL=1;                     AC_SUBST([HAVE_ACOSL])
  HAVE_ASINF=1;                     AC_SUBST([HAVE_ASINF])
  HAVE_ASINL=1;                     AC_SUBST([HAVE_ASINL])
  HAVE_ATANF=1;                     AC_SUBST([HAVE_ATANF])
  HAVE_ATANL=1;                     AC_SUBST([HAVE_ATANL])
  HAVE_ATAN2F=1;                    AC_SUBST([HAVE_ATAN2F])
  HAVE_CBRT=1;                      AC_SUBST([HAVE_CBRT])
  HAVE_CBRTF=1;                     AC_SUBST([HAVE_CBRTF])
  HAVE_CBRTL=1;                     AC_SUBST([HAVE_CBRTL])
  HAVE_COPYSIGN=1;                  AC_SUBST([HAVE_COPYSIGN])
  HAVE_COPYSIGNL=1;                 AC_SUBST([HAVE_COPYSIGNL])
  HAVE_COSF=1;                      AC_SUBST([HAVE_COSF])
  HAVE_COSL=1;                      AC_SUBST([HAVE_COSL])
  HAVE_COSHF=1;                     AC_SUBST([HAVE_COSHF])
  HAVE_EXPF=1;                      AC_SUBST([HAVE_EXPF])
  HAVE_EXPL=1;                      AC_SUBST([HAVE_EXPL])
  HAVE_EXPM1=1;                     AC_SUBST([HAVE_EXPM1])
  HAVE_EXPM1F=1;                    AC_SUBST([HAVE_EXPM1F])
  HAVE_FABSF=1;                     AC_SUBST([HAVE_FABSF])
  HAVE_FABSL=1;                     AC_SUBST([HAVE_FABSL])
  HAVE_FMA=1;                       AC_SUBST([HAVE_FMA])
  HAVE_FMAF=1;                      AC_SUBST([HAVE_FMAF])
  HAVE_FMAL=1;                      AC_SUBST([HAVE_FMAL])
  HAVE_FMODF=1;                     AC_SUBST([HAVE_FMODF])
  HAVE_FMODL=1;                     AC_SUBST([HAVE_FMODL])
  HAVE_FREXPF=1;                    AC_SUBST([HAVE_FREXPF])
  HAVE_HYPOTF=1;                    AC_SUBST([HAVE_HYPOTF])
  HAVE_HYPOTL=1;                    AC_SUBST([HAVE_HYPOTL])
  HAVE_ILOGB=1;                     AC_SUBST([HAVE_ILOGB])
  HAVE_ILOGBF=1;                    AC_SUBST([HAVE_ILOGBF])
  HAVE_ILOGBL=1;                    AC_SUBST([HAVE_ILOGBL])
  HAVE_ISNANF=1;                    AC_SUBST([HAVE_ISNANF])
  HAVE_ISNAND=1;                    AC_SUBST([HAVE_ISNAND])
  HAVE_ISNANL=1;                    AC_SUBST([HAVE_ISNANL])
  HAVE_LDEXPF=1;                    AC_SUBST([HAVE_LDEXPF])
  HAVE_LOGF=1;                      AC_SUBST([HAVE_LOGF])
  HAVE_LOGL=1;                      AC_SUBST([HAVE_LOGL])
  HAVE_LOG10F=1;                    AC_SUBST([HAVE_LOG10F])
  HAVE_LOG10L=1;                    AC_SUBST([HAVE_LOG10L])
  HAVE_LOG1P=1;                     AC_SUBST([HAVE_LOG1P])
  HAVE_LOG1PF=1;                    AC_SUBST([HAVE_LOG1PF])
  HAVE_LOG1PL=1;                    AC_SUBST([HAVE_LOG1PL])
  HAVE_LOGBF=1;                     AC_SUBST([HAVE_LOGBF])
  HAVE_LOGBL=1;                     AC_SUBST([HAVE_LOGBL])
  HAVE_MODFF=1;                     AC_SUBST([HAVE_MODFF])
  HAVE_MODFL=1;                     AC_SUBST([HAVE_MODFL])
  HAVE_POWF=1;                      AC_SUBST([HAVE_POWF])
  HAVE_REMAINDER=1;                 AC_SUBST([HAVE_REMAINDER])
  HAVE_REMAINDERF=1;                AC_SUBST([HAVE_REMAINDERF])
  HAVE_RINT=1;                      AC_SUBST([HAVE_RINT])
  HAVE_RINTL=1;                     AC_SUBST([HAVE_RINTL])
  HAVE_SINF=1;                      AC_SUBST([HAVE_SINF])
  HAVE_SINL=1;                      AC_SUBST([HAVE_SINL])
  HAVE_SINHF=1;                     AC_SUBST([HAVE_SINHF])
  HAVE_SQRTF=1;                     AC_SUBST([HAVE_SQRTF])
  HAVE_SQRTL=1;                     AC_SUBST([HAVE_SQRTL])
  HAVE_TANF=1;                      AC_SUBST([HAVE_TANF])
  HAVE_TANL=1;                      AC_SUBST([HAVE_TANL])
  HAVE_TANHF=1;                     AC_SUBST([HAVE_TANHF])
  HAVE_TOTALORDER=1;                AC_SUBST([HAVE_TOTALORDER])
  HAVE_TOTALORDERF=1;               AC_SUBST([HAVE_TOTALORDERF])
  HAVE_TOTALORDERL=1;               AC_SUBST([HAVE_TOTALORDERL])
  HAVE_DECL_ACOSL=1;                AC_SUBST([HAVE_DECL_ACOSL])
  HAVE_DECL_ASINL=1;                AC_SUBST([HAVE_DECL_ASINL])
  HAVE_DECL_ATANL=1;                AC_SUBST([HAVE_DECL_ATANL])
  HAVE_DECL_CBRTF=1;                AC_SUBST([HAVE_DECL_CBRTF])
  HAVE_DECL_CBRTL=1;                AC_SUBST([HAVE_DECL_CBRTL])
  HAVE_DECL_CEILF=1;                AC_SUBST([HAVE_DECL_CEILF])
  HAVE_DECL_CEILL=1;                AC_SUBST([HAVE_DECL_CEILL])
  HAVE_DECL_COPYSIGNF=1;            AC_SUBST([HAVE_DECL_COPYSIGNF])
  HAVE_DECL_COSL=1;                 AC_SUBST([HAVE_DECL_COSL])
  HAVE_DECL_EXPL=1;                 AC_SUBST([HAVE_DECL_EXPL])
  HAVE_DECL_EXP2=1;                 AC_SUBST([HAVE_DECL_EXP2])
  HAVE_DECL_EXP2F=1;                AC_SUBST([HAVE_DECL_EXP2F])
  HAVE_DECL_EXP2L=1;                AC_SUBST([HAVE_DECL_EXP2L])
  HAVE_DECL_EXPM1L=1;               AC_SUBST([HAVE_DECL_EXPM1L])
  HAVE_DECL_FLOORF=1;               AC_SUBST([HAVE_DECL_FLOORF])
  HAVE_DECL_FLOORL=1;               AC_SUBST([HAVE_DECL_FLOORL])
  HAVE_DECL_FREXPL=1;               AC_SUBST([HAVE_DECL_FREXPL])
  HAVE_DECL_LDEXPL=1;               AC_SUBST([HAVE_DECL_LDEXPL])
  HAVE_DECL_LOGL=1;                 AC_SUBST([HAVE_DECL_LOGL])
  HAVE_DECL_LOG10L=1;               AC_SUBST([HAVE_DECL_LOG10L])
  HAVE_DECL_LOG2=1;                 AC_SUBST([HAVE_DECL_LOG2])
  HAVE_DECL_LOG2F=1;                AC_SUBST([HAVE_DECL_LOG2F])
  HAVE_DECL_LOG2L=1;                AC_SUBST([HAVE_DECL_LOG2L])
  HAVE_DECL_LOGB=1;                 AC_SUBST([HAVE_DECL_LOGB])
  HAVE_DECL_REMAINDER=1;            AC_SUBST([HAVE_DECL_REMAINDER])
  HAVE_DECL_REMAINDERL=1;           AC_SUBST([HAVE_DECL_REMAINDERL])
  HAVE_DECL_RINTF=1;                AC_SUBST([HAVE_DECL_RINTF])
  HAVE_DECL_ROUND=1;                AC_SUBST([HAVE_DECL_ROUND])
  HAVE_DECL_ROUNDF=1;               AC_SUBST([HAVE_DECL_ROUNDF])
  HAVE_DECL_ROUNDL=1;               AC_SUBST([HAVE_DECL_ROUNDL])
  HAVE_DECL_SINL=1;                 AC_SUBST([HAVE_DECL_SINL])
  HAVE_DECL_SQRTL=1;                AC_SUBST([HAVE_DECL_SQRTL])
  HAVE_DECL_TANL=1;                 AC_SUBST([HAVE_DECL_TANL])
  HAVE_DECL_TRUNC=1;                AC_SUBST([HAVE_DECL_TRUNC])
  HAVE_DECL_TRUNCF=1;               AC_SUBST([HAVE_DECL_TRUNCF])
  HAVE_DECL_TRUNCL=1;               AC_SUBST([HAVE_DECL_TRUNCL])
  REPLACE_ACOSF=0;                  AC_SUBST([REPLACE_ACOSF])
  REPLACE_ASINF=0;                  AC_SUBST([REPLACE_ASINF])
  REPLACE_ATANF=0;                  AC_SUBST([REPLACE_ATANF])
  REPLACE_ATAN2F=0;                 AC_SUBST([REPLACE_ATAN2F])
  REPLACE_CBRTF=0;                  AC_SUBST([REPLACE_CBRTF])
  REPLACE_CBRTL=0;                  AC_SUBST([REPLACE_CBRTL])
  REPLACE_CEIL=0;                   AC_SUBST([REPLACE_CEIL])
  REPLACE_CEILF=0;                  AC_SUBST([REPLACE_CEILF])
  REPLACE_CEILL=0;                  AC_SUBST([REPLACE_CEILL])
  REPLACE_COSF=0;                   AC_SUBST([REPLACE_COSF])
  REPLACE_COSHF=0;                  AC_SUBST([REPLACE_COSHF])
  REPLACE_EXPF=0;                   AC_SUBST([REPLACE_EXPF])
  REPLACE_EXPL=0;                   AC_SUBST([REPLACE_EXPL])
  REPLACE_EXPM1=0;                  AC_SUBST([REPLACE_EXPM1])
  REPLACE_EXPM1F=0;                 AC_SUBST([REPLACE_EXPM1F])
  REPLACE_EXPM1L=0;                 AC_SUBST([REPLACE_EXPM1L])
  REPLACE_EXP2=0;                   AC_SUBST([REPLACE_EXP2])
  REPLACE_EXP2L=0;                  AC_SUBST([REPLACE_EXP2L])
  REPLACE_FABSL=0;                  AC_SUBST([REPLACE_FABSL])
  REPLACE_FLOOR=0;                  AC_SUBST([REPLACE_FLOOR])
  REPLACE_FLOORF=0;                 AC_SUBST([REPLACE_FLOORF])
  REPLACE_FLOORL=0;                 AC_SUBST([REPLACE_FLOORL])
  REPLACE_FMA=0;                    AC_SUBST([REPLACE_FMA])
  REPLACE_FMAF=0;                   AC_SUBST([REPLACE_FMAF])
  REPLACE_FMAL=0;                   AC_SUBST([REPLACE_FMAL])
  REPLACE_FMOD=0;                   AC_SUBST([REPLACE_FMOD])
  REPLACE_FMODF=0;                  AC_SUBST([REPLACE_FMODF])
  REPLACE_FMODL=0;                  AC_SUBST([REPLACE_FMODL])
  REPLACE_FREXPF=0;                 AC_SUBST([REPLACE_FREXPF])
  REPLACE_FREXP=0;                  AC_SUBST([REPLACE_FREXP])
  REPLACE_FREXPL=0;                 AC_SUBST([REPLACE_FREXPL])
  REPLACE_HUGE_VAL=0;               AC_SUBST([REPLACE_HUGE_VAL])
  REPLACE_HYPOT=0;                  AC_SUBST([REPLACE_HYPOT])
  REPLACE_HYPOTF=0;                 AC_SUBST([REPLACE_HYPOTF])
  REPLACE_HYPOTL=0;                 AC_SUBST([REPLACE_HYPOTL])
  REPLACE_ILOGB=0;                  AC_SUBST([REPLACE_ILOGB])
  REPLACE_ILOGBF=0;                 AC_SUBST([REPLACE_ILOGBF])
  REPLACE_ILOGBL=0;                 AC_SUBST([REPLACE_ILOGBL])
  REPLACE_ISFINITE=0;               AC_SUBST([REPLACE_ISFINITE])
  REPLACE_ISINF=0;                  AC_SUBST([REPLACE_ISINF])
  REPLACE_ISNAN=0;                  AC_SUBST([REPLACE_ISNAN])
  REPLACE_LDEXP=0;                  AC_SUBST([REPLACE_LDEXP])
  REPLACE_LDEXPL=0;                 AC_SUBST([REPLACE_LDEXPL])
  REPLACE_LOG=0;                    AC_SUBST([REPLACE_LOG])
  REPLACE_LOGF=0;                   AC_SUBST([REPLACE_LOGF])
  REPLACE_LOGL=0;                   AC_SUBST([REPLACE_LOGL])
  REPLACE_LOG10=0;                  AC_SUBST([REPLACE_LOG10])
  REPLACE_LOG10F=0;                 AC_SUBST([REPLACE_LOG10F])
  REPLACE_LOG10L=0;                 AC_SUBST([REPLACE_LOG10L])
  REPLACE_LOG1P=0;                  AC_SUBST([REPLACE_LOG1P])
  REPLACE_LOG1PF=0;                 AC_SUBST([REPLACE_LOG1PF])
  REPLACE_LOG1PL=0;                 AC_SUBST([REPLACE_LOG1PL])
  REPLACE_LOG2=0;                   AC_SUBST([REPLACE_LOG2])
  REPLACE_LOG2F=0;                  AC_SUBST([REPLACE_LOG2F])
  REPLACE_LOG2L=0;                  AC_SUBST([REPLACE_LOG2L])
  REPLACE_LOGB=0;                   AC_SUBST([REPLACE_LOGB])
  REPLACE_LOGBF=0;                  AC_SUBST([REPLACE_LOGBF])
  REPLACE_LOGBL=0;                  AC_SUBST([REPLACE_LOGBL])
  REPLACE_MODF=0;                   AC_SUBST([REPLACE_MODF])
  REPLACE_MODFF=0;                  AC_SUBST([REPLACE_MODFF])
  REPLACE_MODFL=0;                  AC_SUBST([REPLACE_MODFL])
  REPLACE_NAN=0;                    AC_SUBST([REPLACE_NAN])
  REPLACE_REMAINDER=0;              AC_SUBST([REPLACE_REMAINDER])
  REPLACE_REMAINDERF=0;             AC_SUBST([REPLACE_REMAINDERF])
  REPLACE_REMAINDERL=0;             AC_SUBST([REPLACE_REMAINDERL])
  REPLACE_RINTL=0;                  AC_SUBST([REPLACE_RINTL])
  REPLACE_ROUND=0;                  AC_SUBST([REPLACE_ROUND])
  REPLACE_ROUNDF=0;                 AC_SUBST([REPLACE_ROUNDF])
  REPLACE_ROUNDL=0;                 AC_SUBST([REPLACE_ROUNDL])
  REPLACE_SIGNBIT=0;                AC_SUBST([REPLACE_SIGNBIT])
  REPLACE_SIGNBIT_USING_BUILTINS=0; AC_SUBST([REPLACE_SIGNBIT_USING_BUILTINS])
  REPLACE_SINF=0;                   AC_SUBST([REPLACE_SINF])
  REPLACE_SINHF=0;                  AC_SUBST([REPLACE_SINHF])
  REPLACE_SQRTF=0;                  AC_SUBST([REPLACE_SQRTF])
  REPLACE_SQRTL=0;                  AC_SUBST([REPLACE_SQRTL])
  REPLACE_TANF=0;                   AC_SUBST([REPLACE_TANF])
  REPLACE_TANHF=0;                  AC_SUBST([REPLACE_TANHF])
  REPLACE_TOTALORDER=0;             AC_SUBST([REPLACE_TOTALORDER])
  REPLACE_TOTALORDERF=0;            AC_SUBST([REPLACE_TOTALORDERF])
  REPLACE_TOTALORDERL=0;            AC_SUBST([REPLACE_TOTALORDERL])
  REPLACE_TRUNC=0;                  AC_SUBST([REPLACE_TRUNC])
  REPLACE_TRUNCF=0;                 AC_SUBST([REPLACE_TRUNCF])
  REPLACE_TRUNCL=0;                 AC_SUBST([REPLACE_TRUNCL])
])

# gl_LONG_DOUBLE_VS_DOUBLE
# determines whether 'long double' and 'double' have the same representation.
# Sets variable HAVE_SAME_LONG_DOUBLE_AS_DOUBLE to 0 or 1, and defines
# HAVE_SAME_LONG_DOUBLE_AS_DOUBLE accordingly.
# The currently known platforms where this is the case are:
# Linux/HPPA, NetBSD/sparc32, Minix 3.1.8, AIX 5, AIX 6 and 7 with xlc, MSVC 9.
AC_DEFUN([gl_LONG_DOUBLE_VS_DOUBLE],
[
  AC_CACHE_CHECK([whether long double and double are the same],
    [gl_cv_long_double_equals_double],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM([[#include <float.h>]],
          [[typedef int check[sizeof (long double) == sizeof (double)
                              && LDBL_MANT_DIG == DBL_MANT_DIG
                              && LDBL_MAX_EXP == DBL_MAX_EXP
                              && LDBL_MIN_EXP == DBL_MIN_EXP
                              ? 1 : -1];
          ]])],
       [gl_cv_long_double_equals_double=yes],
       [gl_cv_long_double_equals_double=no])
    ])
  if test $gl_cv_long_double_equals_double = yes; then
    AC_DEFINE([HAVE_SAME_LONG_DOUBLE_AS_DOUBLE], [1],
      [Define to 1 if 'long double' and 'double' have the same representation.])
    HAVE_SAME_LONG_DOUBLE_AS_DOUBLE=1
  else
    HAVE_SAME_LONG_DOUBLE_AS_DOUBLE=0
  fi
  AC_SUBST([HAVE_SAME_LONG_DOUBLE_AS_DOUBLE])
])
