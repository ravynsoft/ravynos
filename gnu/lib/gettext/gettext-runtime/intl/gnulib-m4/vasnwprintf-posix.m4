# vasnwprintf-posix.m4 serial 3
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_VASNWPRINTF_POSIX],
[
  AC_REQUIRE([gl_FUNC_VASNWPRINTF_IS_POSIX])
  if test $gl_cv_func_vasnwprintf_posix = no; then
    dnl Note: This invokes gl_PREREQ_VASNPRINTF_DIRECTIVE_LC although not needed
    dnl here. Doesn't matter.
    gl_PREREQ_VASNPRINTF_WITH_POSIX_EXTRAS
    gl_FUNC_VASNWPRINTF
  fi
])

dnl Test whether vasnwprintf exists and is POSIX compliant.
dnl Result is gl_cv_func_vasnwprintf_posix.
AC_DEFUN([gl_FUNC_VASNWPRINTF_IS_POSIX],
[
  gl_cv_func_vasnwprintf_posix=no
])
