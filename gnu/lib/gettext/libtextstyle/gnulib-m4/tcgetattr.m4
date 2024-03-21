# tcgetattr.m4 serial 1
dnl Copyright (C) 2002-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_HAVE_TCGETATTR],
[
  dnl We can't use AC_CHECK_FUNC here, because tcgetattr() is defined as a
  dnl static inline function when compiling for Android 4.4 or older.
  AC_CACHE_CHECK([for tcgetattr], [gl_cv_func_tcgetattr],
    [AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <termios.h>
            struct termios x;
          ]],
          [[return tcgetattr(0,&x);]])
       ],
       [gl_cv_func_tcgetattr=yes],
       [gl_cv_func_tcgetattr=no])
    ])
  if test $gl_cv_func_tcgetattr = yes; then
    HAVE_TCGETATTR=1
  else
    HAVE_TCGETATTR=0
  fi
  AC_DEFINE_UNQUOTED([HAVE_TCGETATTR], [$HAVE_TCGETATTR],
    [Define to 1 if the system has the 'tcgetattr' function.])
])
