# strpbrk.m4 serial 7
dnl Copyright (C) 2002-2003, 2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_STRPBRK],
[
  AC_REQUIRE([gl_STRING_H_DEFAULTS])
  AC_CHECK_FUNCS([strpbrk])
  if test $ac_cv_func_strpbrk = no; then
    HAVE_STRPBRK=0
  fi
])

# Prerequisites of lib/strpbrk.c.
AC_DEFUN([gl_PREREQ_STRPBRK], [:])
