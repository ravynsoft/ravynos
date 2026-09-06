# error.m4
# serial 16
dnl Copyright (C) 1996-1998, 2001-2004, 2009-2026 Free Software Foundation,
dnl Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

AC_DEFUN([gl_ERROR],
[
])

# Prerequisites of lib/error.c.
AC_DEFUN([gl_PREREQ_ERROR],
[
  dnl Use system extensions on Android, so that AC_FUNC_STRERROR_R
  dnl discovers the GNU API for strerror_r on Android API level 23 and later.
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])

  AC_REQUIRE([AC_FUNC_STRERROR_R])
  :
])
