# mbslen.m4 serial 3
dnl Copyright (C) 2010-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_MBSLEN],
[
  AC_REQUIRE([gl_STRING_H_DEFAULTS])
  AC_CHECK_FUNCS_ONCE([mbslen])
  if test $ac_cv_func_mbslen = yes; then
    HAVE_MBSLEN=1
  else
    HAVE_MBSLEN=0
  fi
])
