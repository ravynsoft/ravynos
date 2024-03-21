# isinf.m4 serial 14
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_ISINF],
[
  AC_REQUIRE([gl_MATH_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  dnl Persuade glibc <math.h> to declare isinf.
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_CHECK_DECLS([isinf], , ,
    [[#include <math.h>
      #ifndef isinf
      #error "isinf must be a macro, not a function"
      #endif
    ]])
  if test "$ac_cv_have_decl_isinf" = yes; then
    gl_CHECK_MATH_LIB([ISINF_LIBM], [x = isinf (x) + isinf ((float) x);])
    if test "$ISINF_LIBM" != missing; then
      dnl Test whether isinf() on 'long double' works.
      gl_ISINFL_WORKS
      case "$gl_cv_func_isinfl_works" in
        *yes) ;;
        *)    ISINF_LIBM=missing;;
      esac
    fi
  fi
  dnl On Solaris 10, with CC in C++ mode, isinf is not available although
  dnl is with cc in C mode. This cannot be worked around by defining
  dnl _XOPEN_SOURCE=600, because the latter does not work in C++ mode on
  dnl Solaris 11.0. Therefore use the replacement functions on Solaris.
  if test "$ac_cv_have_decl_isinf" != yes \
     || test "$ISINF_LIBM" = missing \
     || { case "$host_os" in solaris*) true;; *) false;; esac; }; then
    REPLACE_ISINF=1
    dnl No libraries are needed to link lib/isinf.c.
    ISINF_LIBM=
  fi
  AC_SUBST([ISINF_LIBM])
])

dnl Test whether isinf() works:
dnl 1) Whether it correctly returns false for LDBL_MAX.
dnl 2) Whether on 'long double' recognizes all canonical values which are
dnl    infinite.
AC_DEFUN([gl_ISINFL_WORKS],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([gl_BIGENDIAN])
  AC_REQUIRE([gl_LONG_DOUBLE_VS_DOUBLE])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether isinf(long double) works], [gl_cv_func_isinfl_works],
    [
      AC_RUN_IFELSE(
        [AC_LANG_SOURCE([[
#include <float.h>
#include <limits.h>
#include <math.h>
#define NWORDS \
  ((sizeof (long double) + sizeof (unsigned int) - 1) / sizeof (unsigned int))
typedef union { unsigned int word[NWORDS]; long double value; }
        memory_long_double;
/* On Irix 6.5, gcc 3.4.3 can't compute compile-time NaN, and needs the
   runtime type conversion.  */
#ifdef __sgi
static long double NaNl ()
{
  double zero = 0.0;
  return zero / zero;
}
#else
# define NaNl() (0.0L / 0.0L)
#endif
int main ()
{
  int result = 0;

  if (isinf (LDBL_MAX))
    result |= 1;

  {
    memory_long_double m;
    unsigned int i;

    /* The isinf macro should be immune against changes in the sign bit and
       in the mantissa bits.  The xor operation twiddles a bit that can only be
       a sign bit or a mantissa bit (since the exponent never extends to
       bit 31).  */
    m.value = NaNl ();
    m.word[NWORDS / 2] ^= (unsigned int) 1 << (sizeof (unsigned int) * CHAR_BIT - 1);
    for (i = 0; i < NWORDS; i++)
      m.word[i] |= 1;
    if (isinf (m.value))
      result |= 2;
  }

#if ((defined __ia64 && LDBL_MANT_DIG == 64) || (defined __x86_64__ || defined __amd64__) || (defined __i386 || defined __i386__ || defined _I386 || defined _M_IX86 || defined _X86_)) && !HAVE_SAME_LONG_DOUBLE_AS_DOUBLE
/* Representation of an 80-bit 'long double' as an initializer for a sequence
   of 'unsigned int' words.  */
# ifdef WORDS_BIGENDIAN
#  define LDBL80_WORDS(exponent,manthi,mantlo) \
     { ((unsigned int) (exponent) << 16) | ((unsigned int) (manthi) >> 16), \
       ((unsigned int) (manthi) << 16) | ((unsigned int) (mantlo) >> 16),   \
       (unsigned int) (mantlo) << 16                                        \
     }
# else
#  define LDBL80_WORDS(exponent,manthi,mantlo) \
     { mantlo, manthi, exponent }
# endif
  { /* Quiet NaN.  */
    static memory_long_double x =
      { LDBL80_WORDS (0xFFFF, 0xC3333333, 0x00000000) };
    if (isinf (x.value))
      result |= 2;
  }
  {
    /* Signalling NaN.  */
    static memory_long_double x =
      { LDBL80_WORDS (0xFFFF, 0x83333333, 0x00000000) };
    if (isinf (x.value))
      result |= 2;
  }
  /* isinf should return something even for noncanonical values.  */
  { /* Pseudo-NaN.  */
    static memory_long_double x =
      { LDBL80_WORDS (0xFFFF, 0x40000001, 0x00000000) };
    if (isinf (x.value) && !isinf (x.value))
      result |= 4;
  }
  { /* Pseudo-Infinity.  */
    static memory_long_double x =
      { LDBL80_WORDS (0xFFFF, 0x00000000, 0x00000000) };
    if (isinf (x.value) && !isinf (x.value))
      result |= 8;
  }
  { /* Pseudo-Zero.  */
    static memory_long_double x =
      { LDBL80_WORDS (0x4004, 0x00000000, 0x00000000) };
    if (isinf (x.value) && !isinf (x.value))
      result |= 16;
  }
  { /* Unnormalized number.  */
    static memory_long_double x =
      { LDBL80_WORDS (0x4000, 0x63333333, 0x00000000) };
    if (isinf (x.value) && !isinf (x.value))
      result |= 32;
  }
  { /* Pseudo-Denormal.  */
    static memory_long_double x =
      { LDBL80_WORDS (0x0000, 0x83333333, 0x00000000) };
    if (isinf (x.value) && !isinf (x.value))
      result |= 64;
  }
#endif

  return result;
}]])],
      [gl_cv_func_isinfl_works=yes],
      [gl_cv_func_isinfl_works=no],
      [case "$host_os" in
         mingw* | windows*) # Guess yes on mingw, no on MSVC.
           AC_EGREP_CPP([Known], [
#ifdef __MINGW32__
 Known
#endif
             ],
             [gl_cv_func_isinfl_works="guessing yes"],
             [gl_cv_func_isinfl_works="guessing no"])
           ;;
         *)
           gl_cv_func_isinfl_works="guessing yes"
           ;;
       esac
      ])
    ])
])
