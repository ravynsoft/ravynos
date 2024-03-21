# sockets.m4 serial 7
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_SOCKETS],
[
  AC_REQUIRE([AC_C_INLINE])
  AC_REQUIRE([gl_SOCKETLIB])
  gl_PREREQ_SOCKETS
])

# Prerequisites of lib/sockets.c.
AC_DEFUN([gl_PREREQ_SOCKETS], [
  :
])
