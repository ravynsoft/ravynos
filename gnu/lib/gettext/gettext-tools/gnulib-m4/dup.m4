# dup.m4 serial 8
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_DUP],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  m4_ifdef([gl_MSVC_INVAL], [
    AC_REQUIRE([gl_MSVC_INVAL])
    if test $HAVE_MSVC_INVALID_PARAMETER_HANDLER = 1; then
      REPLACE_DUP=1
    fi
  ])
  dnl Replace dup() for supporting the gnulib-defined fchdir() function,
  dnl to keep fchdir's bookkeeping up-to-date.
  m4_ifdef([gl_FUNC_FCHDIR], [
    gl_TEST_FCHDIR
    if test $HAVE_FCHDIR = 0; then
      REPLACE_DUP=1
    fi
  ])
  AC_CACHE_CHECK([whether dup works], [gl_cv_func_dup_works],
    [AC_RUN_IFELSE(
      [AC_LANG_PROGRAM([[#include <unistd.h>
                         #include <fcntl.h>
                         #include <errno.h>
                       ]GL_MDA_DEFINES],
         [[/* On OS/2 kLIBC, dup does not work on a directory fd.  */
           int fd = open (".", O_RDONLY);
           return fd < 0 ? 1 : dup (fd) < 0 ? 2 : 0;
         ]])
      ],
      [gl_cv_func_dup_works=yes],
      [gl_cv_func_dup_works=no],
      [case "$host_os" in
                            # Guess no on native Windows.
         mingw* | windows*) gl_cv_func_dup_works="guessing no" ;;
         *)                 gl_cv_func_dup_works="guessing yes" ;;
       esac
      ])
    ])
  case "$gl_cv_func_dup_works" in
    *yes) ;;
    *)
      REPLACE_DUP=1
      ;;
  esac
])

# Prerequisites of lib/dup.c.
AC_DEFUN([gl_PREREQ_DUP], [:])
