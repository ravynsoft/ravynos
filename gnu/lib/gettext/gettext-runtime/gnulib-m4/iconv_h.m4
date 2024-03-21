# iconv_h.m4 serial 16
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN_ONCE([gl_ICONV_H],
[
  AC_REQUIRE([gl_ICONV_H_DEFAULTS])

  dnl Execute this unconditionally, because GL_GENERATE_ICONV_H may be set to
  dnl true by other modules, after this code is executed.
  gl_CHECK_NEXT_HEADERS([iconv.h])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use, and which is not
  dnl guaranteed by C89.
  gl_WARN_ON_USE_PREPARE([[#include <iconv.h>
    ]], [iconv iconv_open])

  AC_REQUIRE([AC_C_RESTRICT])
])

dnl Unconditionally enables the replacement of <iconv.h>.
AC_DEFUN([gl_REPLACE_ICONV_H],
[
  gl_ICONV_H_REQUIRE_DEFAULTS
  GL_GENERATE_ICONV_H=true
])

# gl_ICONV_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_ICONV_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_ICONV_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_ICONV_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_ICONV_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_ICONV])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_ICONV_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_ICONV_H_DEFAULTS])
])

AC_DEFUN([gl_ICONV_H_DEFAULTS],
[
  m4_ifdef([gl_ANSI_CXX], [AC_REQUIRE([gl_ANSI_CXX])])
  dnl Assume proper GNU behavior unless another module says otherwise.
  ICONV_CONST=;         AC_SUBST([ICONV_CONST])
  REPLACE_ICONV=0;      AC_SUBST([REPLACE_ICONV])
  REPLACE_ICONV_OPEN=0; AC_SUBST([REPLACE_ICONV_OPEN])
  REPLACE_ICONV_UTF=0;  AC_SUBST([REPLACE_ICONV_UTF])
  GL_GENERATE_ICONV_H=false
  m4_ifdef([gl_POSIXCHECK],
    [GL_GENERATE_ICONV_H=true],
    [if m4_ifdef([gl_ANSI_CXX], [test "$CXX" != no], [false]); then
       dnl Override <fnmatch.h> always, to support the C++ GNULIB_NAMESPACE.
       GL_GENERATE_ICONV_H=true
     fi
    ])
])
