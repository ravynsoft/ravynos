dnl Copyright 2022-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_MEMSET_EXPLICIT],
[
  AC_REQUIRE([gl_STRING_H_DEFAULTS])

  AC_CHECK_FUNCS_ONCE([memset_explicit])
  if test $ac_cv_func_memset_explicit = no; then
    HAVE_MEMSET_EXPLICIT=0
  fi
])

AC_DEFUN([gl_PREREQ_MEMSET_EXPLICIT],
[
  AC_CHECK_FUNCS([explicit_memset])
  AC_CHECK_FUNCS_ONCE([memset_s])
])
