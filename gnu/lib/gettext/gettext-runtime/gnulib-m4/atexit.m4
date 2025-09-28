# atexit.m4 serial 4
dnl Copyright (C) 2002, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_ATEXIT],
[
  AC_CHECK_FUNCS([atexit])
])

# Prerequisites of lib/atexit.c.
AC_DEFUN([gl_PREREQ_ATEXIT], [
  :
])
