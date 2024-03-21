# search_h.m4 serial 16
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN_ONCE([gl_SEARCH_H],
[
  AC_REQUIRE([gl_SEARCH_H_DEFAULTS])
  gl_CHECK_NEXT_HEADERS([search.h])
  if test $ac_cv_header_search_h = yes; then
    HAVE_SEARCH_H=1
  else
    HAVE_SEARCH_H=0
  fi
  AC_SUBST([HAVE_SEARCH_H])

  if test $HAVE_SEARCH_H = 1; then
    AC_CACHE_CHECK([for type VISIT], [gl_cv_type_VISIT],
      [AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM(
            [[#if HAVE_SEARCH_H
               #include <search.h>
              #endif
            ]],
            [[static VISIT x; x = postorder;]])],
         [gl_cv_type_VISIT=yes],
         [gl_cv_type_VISIT=no])])
  else
    gl_cv_type_VISIT=no
  fi
  if test $gl_cv_type_VISIT = yes; then
    HAVE_TYPE_VISIT=1
  else
    HAVE_TYPE_VISIT=0
  fi
  AC_SUBST([HAVE_TYPE_VISIT])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <search.h>
    ]], [tdelete tfind tsearch twalk])

  AC_REQUIRE([AC_C_RESTRICT])
])

# gl_SEARCH_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_SEARCH_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_SEARCH_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_SEARCH_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_SEARCH_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TSEARCH])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_SEARCH_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_SEARCH_H_DEFAULTS])
])

AC_DEFUN([gl_SEARCH_H_DEFAULTS],
[
  dnl Support Microsoft deprecated alias function names by default.
  gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_LFIND], [1])
  gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_LSEARCH], [1])
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_TSEARCH=1;    AC_SUBST([HAVE_TSEARCH])
  HAVE_TWALK=1;      AC_SUBST([HAVE_TWALK])
  REPLACE_TSEARCH=0; AC_SUBST([REPLACE_TSEARCH])
  REPLACE_TWALK=0;   AC_SUBST([REPLACE_TWALK])
])
