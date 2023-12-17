# isatty.m4 serial 4
dnl Copyright (C) 2012-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_ISATTY],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  dnl On native Windows, the system's isatty(), defined as an alias of _isatty()
  dnl in the "oldnames" library, returns true for the NUL device.
  case $host_os in
    mingw* | windows*) REPLACE_ISATTY=1 ;;
  esac
])

# Prerequisites of lib/isatty.c.
AC_DEFUN([gl_PREREQ_ISATTY], [:])
