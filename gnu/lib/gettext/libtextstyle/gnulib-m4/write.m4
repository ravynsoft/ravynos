# write.m4 serial 6
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_WRITE],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  m4_ifdef([gl_MSVC_INVAL], [
    AC_REQUIRE([gl_MSVC_INVAL])
    if test $HAVE_MSVC_INVALID_PARAMETER_HANDLER = 1; then
      REPLACE_WRITE=1
    fi
  ])
  dnl This ifdef is just an optimization, to avoid performing a configure
  dnl check whose result is not used. It does not make the test of
  dnl GNULIB_UNISTD_H_SIGPIPE or GNULIB_SIGPIPE redundant.
  m4_ifdef([gl_SIGNAL_SIGPIPE], [
    gl_SIGNAL_SIGPIPE
    if test $gl_cv_header_signal_h_SIGPIPE != yes; then
      REPLACE_WRITE=1
    fi
  ])
  m4_ifdef([gl_NONBLOCKING_IO], [
    gl_NONBLOCKING_IO
    if test $gl_cv_have_nonblocking != yes; then
      REPLACE_WRITE=1
    fi
  ])
])

# Prerequisites of lib/write.c.
AC_DEFUN([gl_PREREQ_WRITE], [:])
