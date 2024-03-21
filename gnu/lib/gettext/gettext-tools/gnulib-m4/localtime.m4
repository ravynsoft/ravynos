# localtime.m4 serial 3
dnl Copyright (C) 2017-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_LOCALTIME],
[
  AC_REQUIRE([gl_TIME_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw* | windows*) REPLACE_LOCALTIME=1 ;;
  esac
])
