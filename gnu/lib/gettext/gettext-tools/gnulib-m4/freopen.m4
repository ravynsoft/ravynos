# freopen.m4 serial 8
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FREOPEN],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw* | windows* | pw* | os2*)
      REPLACE_FREOPEN=1
      ;;
    *)
      AC_CACHE_CHECK([whether freopen works on closed fds],
        [gl_cv_func_freopen_works_on_closed],
        [AC_RUN_IFELSE(
           [AC_LANG_PROGRAM(
              [[#include <stdio.h>
                #include <unistd.h>
              ]GL_MDA_DEFINES],
              [[close (0);
                return !(freopen ("/dev/null", "r", stdin)
                         && getchar () == EOF
                         && !ferror (stdin) && feof (stdin));]])],
           [gl_cv_func_freopen_works_on_closed=yes],
           [gl_cv_func_freopen_works_on_closed=no],
           [case $host_os in
              *gnu*) gl_cv_func_freopen_works_on_closed="guessing no" ;;
              *)     gl_cv_func_freopen_works_on_closed="guessing yes";;
            esac])])
      case $gl_cv_func_freopen_works_on_closed in
        *no) REPLACE_FREOPEN=1;;
      esac
  esac
])

# Prerequisites of lib/freopen.c.
AC_DEFUN([gl_PREREQ_FREOPEN], [:])
