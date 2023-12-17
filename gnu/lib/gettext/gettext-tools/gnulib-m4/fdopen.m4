# fdopen.m4 serial 6
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FDOPEN],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  m4_ifdef([gl_MSVC_INVAL], [
    AC_REQUIRE([gl_MSVC_INVAL])
    if test $HAVE_MSVC_INVALID_PARAMETER_HANDLER = 1; then
      REPLACE_FDOPEN=1
    fi
  ])
  if test $REPLACE_FDOPEN = 0; then
    dnl Test whether fdopen() sets errno when it fails due to a bad fd argument.
    AC_CACHE_CHECK([whether fdopen sets errno], [gl_cv_func_fdopen_works],
      [
        AC_RUN_IFELSE(
          [AC_LANG_SOURCE([[
#include <stdio.h>
#include <errno.h>
]GL_MDA_DEFINES[
int
main (void)
{
  FILE *fp;
  errno = 0;
  fp = fdopen (-1, "r");
  if (fp == NULL && errno == 0)
    return 1;
  return 0;
}]])],
          [gl_cv_func_fdopen_works=yes],
          [gl_cv_func_fdopen_works=no],
          [case "$host_os" in
             mingw* | windows*) gl_cv_func_fdopen_works="guessing no" ;;
             *)                 gl_cv_func_fdopen_works="guessing yes" ;;
           esac
          ])
      ])
    case "$gl_cv_func_fdopen_works" in
      *no) REPLACE_FDOPEN=1 ;;
    esac
  fi
])

dnl Prerequisites of lib/fdopen.c.
AC_DEFUN([gl_PREREQ_FDOPEN], [])
