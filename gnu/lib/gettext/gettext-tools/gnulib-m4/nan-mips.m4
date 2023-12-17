# nan-mips.m4 serial 1
dnl Copyright (C) 2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Extra meta-info mentioned by lib/snan.h.
AC_DEFUN_ONCE([gl_NAN_MIPS],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_cpu" in
    mips*)
      AC_CACHE_CHECK([whether the NaN float encoding is IEEE 754-2008 compliant],
        [gl_cv_nan2008_f],
        [AC_RUN_IFELSE(
           [AC_LANG_PROGRAM([[
              float volatile zero;
              /* Assume 'float' has 32 bits, i.e. IEEE single-float.  */
              union { float value; unsigned int word; } qnan;
              ]],
              [[qnan.value = zero / zero;
                return !((qnan.word >> 22) & 1);
              ]])
           ],
           [gl_cv_nan2008_f=yes],
           [gl_cv_nan2008_f=no],
           [gl_cv_nan2008_f="guessing no"])
        ])
      case "$gl_cv_nan2008_f" in
        *yes) gl_mips_nan2008_f=1 ;;
        *)    gl_mips_nan2008_f=0 ;;
      esac
      AC_DEFINE_UNQUOTED([MIPS_NAN2008_FLOAT], [$gl_mips_nan2008_f],
        [Define to 1 if the encoding of NaN 'float's is as in IEEE 754-2008 § 6.2.1.])

      AC_CACHE_CHECK([whether the NaN double encoding is IEEE 754-2008 compliant],
        [gl_cv_nan2008_d],
        [AC_RUN_IFELSE(
           [AC_LANG_PROGRAM([[
              double volatile zero;
              /* Assume 'double' has 64 bits, i.e. IEEE double-float.  */
              union { double value; unsigned long long word; } qnan;
              ]],
              [[qnan.value = zero / zero;
                return !((qnan.word >> 51) & 1);
              ]])
           ],
           [gl_cv_nan2008_d=yes],
           [gl_cv_nan2008_d=no],
           [gl_cv_nan2008_d="guessing no"])
        ])
      case "$gl_cv_nan2008_d" in
        *yes) gl_mips_nan2008_d=1 ;;
        *)    gl_mips_nan2008_d=0 ;;
      esac
      AC_DEFINE_UNQUOTED([MIPS_NAN2008_DOUBLE], [$gl_mips_nan2008_d],
        [Define to 1 if the encoding of NaN 'double's is as in IEEE 754-2008 § 6.2.1.])

      AC_CACHE_CHECK([whether the NaN long double encoding is IEEE 754-2008 compliant],
        [gl_cv_nan2008_l],
        [AC_RUN_IFELSE(
           [AC_LANG_PROGRAM([[
              #include <float.h>
              long double volatile zero;
              #define NWORDS \
                ((sizeof (long double) + sizeof (unsigned int) - 1) / sizeof (unsigned int))
              union { long double value; unsigned int word[NWORDS]; } qnan;
              ]],
              [[qnan.value = zero / zero;
                #if defined _MIPSEB /* equivalent: __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
                return !((qnan.word[0] >> ((LDBL_MANT_DIG - 2) % 32)) & 1);
                #else
                return !((qnan.word[NWORDS - 1] >> ((LDBL_MANT_DIG - 2) % 32)) & 1);
                #endif
              ]])
           ],
           [gl_cv_nan2008_l=yes],
           [gl_cv_nan2008_l=no],
           [gl_cv_nan2008_l="guessing no"])
        ])
      case "$gl_cv_nan2008_l" in
        *yes) gl_mips_nan2008_l=1 ;;
        *)    gl_mips_nan2008_l=0 ;;
      esac
      AC_DEFINE_UNQUOTED([MIPS_NAN2008_LONG_DOUBLE], [$gl_mips_nan2008_l],
        [Define to 1 if the encoding of NaN 'long double's is as in IEEE 754-2008 § 6.2.1.])
      ;;
  esac
])
