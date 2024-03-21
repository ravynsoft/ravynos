# check-math-lib.m4 serial 4
dnl Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl
dnl gl_CHECK_MATH_LIB (VARIABLE, EXPRESSION [, EXTRA-CODE])
dnl
dnl Sets the shell VARIABLE according to the libraries needed by EXPRESSION
dnl to compile and link: to the empty string if no extra libraries are needed,
dnl to "-lm" if -lm is needed, or to "missing" if it does not compile and
dnl link either way.
dnl
dnl Example: gl_CHECK_MATH_LIB([ROUNDF_LIBM], [x = roundf (x);])
AC_DEFUN([gl_CHECK_MATH_LIB], [
  save_LIBS=$LIBS
  $1=missing
  for libm in "" "-lm"; do
    LIBS="$save_LIBS $libm"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
         #ifndef __NO_MATH_INLINES
         # define __NO_MATH_INLINES 1 /* for glibc */
         #endif
         #include <math.h>
         $3
         double x;]],
      [$2])],
      [$1=$libm
break])
  done
  LIBS=$save_LIBS
])
