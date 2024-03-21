# snan.m4 serial 3
dnl Copyright (C) 2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Prerequisites for lib/snan.h.
AC_DEFUN_ONCE([gl_SNAN],
[
  gl_FLOAT_EXPONENT_LOCATION
  gl_DOUBLE_EXPONENT_LOCATION
  gl_LONG_DOUBLE_EXPONENT_LOCATION
  AC_REQUIRE([gl_LONG_DOUBLE_VS_DOUBLE])
])
