# secure_getenv.m4 serial 2
dnl Copyright 2013-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_SECURE_GETENV],
[
  dnl Persuade glibc <stdlib.h> to declare secure_getenv().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_CHECK_FUNCS_ONCE([secure_getenv])
  if test $ac_cv_func_secure_getenv = no; then
    HAVE_SECURE_GETENV=0
  fi
])

# Prerequisites of lib/secure_getenv.c.
AC_DEFUN([gl_PREREQ_SECURE_GETENV], [
  AC_CHECK_FUNCS([__secure_getenv])
  if test $ac_cv_func___secure_getenv = no; then
    gl_CHECK_FUNCS_ANDROID([issetugid], [[#include <unistd.h>]])
  fi
  AC_CHECK_FUNCS_ONCE([getuid geteuid getgid getegid])
])
