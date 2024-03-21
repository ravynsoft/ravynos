# stpcpy.m4 serial 11
dnl Copyright (C) 2002, 2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_STPCPY],
[
  dnl Persuade glibc <string.h> to declare stpcpy().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  dnl The stpcpy() declaration in lib/string.in.h uses 'restrict'.
  AC_REQUIRE([AC_C_RESTRICT])

  AC_REQUIRE([gl_STRING_H_DEFAULTS])
  gl_CHECK_FUNCS_ANDROID([stpcpy], [[#include <string.h>]])
  if test $ac_cv_func_stpcpy = no; then
    HAVE_STPCPY=0
    case "$gl_cv_onwards_func_stpcpy" in
      future*) REPLACE_STPCPY=1 ;;
    esac
  fi
])

# Prerequisites of lib/stpcpy.c.
AC_DEFUN([gl_PREREQ_STPCPY], [
  :
])
