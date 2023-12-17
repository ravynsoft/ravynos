# mkdtemp.m4 serial 8
dnl Copyright (C) 2001-2003, 2006-2007, 2009-2023 Free Software Foundation,
dnl Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_MKDTEMP],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_CHECK_FUNCS([mkdtemp])
  if test $ac_cv_func_mkdtemp = no; then
    HAVE_MKDTEMP=0
  fi
])

# Prerequisites of lib/mkdtemp.c
AC_DEFUN([gl_PREREQ_MKDTEMP],
[:
])
