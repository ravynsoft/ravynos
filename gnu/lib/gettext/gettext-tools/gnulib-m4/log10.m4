# log10.m4 serial 13
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_LOG10],
[
  m4_divert_text([DEFAULTS], [gl_log10_required=plain])
  AC_REQUIRE([gl_MATH_H_DEFAULTS])

  dnl Determine LOG10_LIBM.
  gl_COMMON_DOUBLE_MATHFUNC([log10])

  save_LIBS="$LIBS"
  LIBS="$LIBS $LOG10_LIBM"
  gl_FUNC_LOG10_WORKS
  LIBS="$save_LIBS"
  case "$gl_cv_func_log10_works" in
    *yes) ;;
    *) REPLACE_LOG10=1 ;;
  esac

  m4_ifdef([gl_FUNC_LOG10_IEEE], [
    if test $gl_log10_required = ieee && test $REPLACE_LOG10 = 0; then
      AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
      AC_CACHE_CHECK([whether log10 works according to ISO C 99 with IEC 60559],
        [gl_cv_func_log10_ieee],
        [
          save_LIBS="$LIBS"
          LIBS="$LIBS $LOG10_LIBM"
          AC_RUN_IFELSE(
            [AC_LANG_SOURCE([[
#ifndef __NO_MATH_INLINES
# define __NO_MATH_INLINES 1 /* for glibc */
#endif
#include <math.h>
/* Compare two numbers with ==.
   This is a separate function because IRIX 6.5 "cc -O" miscompiles an
   'x == x' test.  */
static int
numeric_equal (double x, double y)
{
  return x == y;
}
static double dummy (double x) { return 0; }
int main (int argc, char *argv[])
{
  double (* volatile my_log10) (double) = argc ? log10 : dummy;
  /* Test log10(negative).
     This test fails on NetBSD 5.1, Solaris 11.4.  */
  double y = my_log10 (-1.0);
  if (numeric_equal (y, y))
    return 1;
  return 0;
}
            ]])],
            [gl_cv_func_log10_ieee=yes],
            [gl_cv_func_log10_ieee=no],
            [case "$host_os" in
                                   # Guess yes on glibc systems.
               *-gnu* | gnu*)      gl_cv_func_log10_ieee="guessing yes" ;;
                                   # Guess yes on musl systems.
               *-musl* | midipix*) gl_cv_func_log10_ieee="guessing yes" ;;
                                   # Guess yes on native Windows.
               mingw* | windows*)  gl_cv_func_log10_ieee="guessing yes" ;;
                                   # If we don't know, obey --enable-cross-guesses.
               *)                  gl_cv_func_log10_ieee="$gl_cross_guess_normal" ;;
             esac
            ])
          LIBS="$save_LIBS"
        ])
      case "$gl_cv_func_log10_ieee" in
        *yes) ;;
        *) REPLACE_LOG10=1 ;;
      esac
    fi
  ])
])

dnl Test whether log10() works.
dnl On OSF/1 5.1, log10(-0.0) is NaN.
AC_DEFUN([gl_FUNC_LOG10_WORKS],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether log10 works], [gl_cv_func_log10_works],
    [
      AC_RUN_IFELSE(
        [AC_LANG_SOURCE([[
#include <math.h>
volatile double x;
double y;
int main ()
{
  x = -0.0;
  y = log10 (x);
  if (!(y + y == y))
    return 1;
  return 0;
}
]])],
        [gl_cv_func_log10_works=yes],
        [gl_cv_func_log10_works=no],
        [case "$host_os" in
           osf*)              gl_cv_func_log10_works="guessing no" ;;
                              # Guess yes on native Windows.
           mingw* | windows*) gl_cv_func_log10_works="guessing yes" ;;
           *)                 gl_cv_func_log10_works="guessing yes" ;;
         esac
        ])
    ])
])
