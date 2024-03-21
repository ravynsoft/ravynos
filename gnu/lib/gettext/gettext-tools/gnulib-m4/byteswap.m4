# byteswap.m4 serial 5
dnl Copyright (C) 2005, 2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Written by Oskar Liljeblad.

AC_DEFUN([gl_BYTESWAP],
[
  dnl Prerequisites of lib/byteswap.in.h.
  AC_CHECK_HEADERS([byteswap.h], [
    GL_GENERATE_BYTESWAP_H=false
  ], [
    GL_GENERATE_BYTESWAP_H=true
  ])
])
