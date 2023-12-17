# fabs.m4 serial 1
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FABS],
[
  dnl Determine FABS_LIBM.
  gl_MATHFUNC([fabs], [double], [(double)])
])
