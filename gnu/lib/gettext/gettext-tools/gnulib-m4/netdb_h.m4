# netdb_h.m4 serial 15
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN_ONCE([gl_NETDB_H],
[
  AC_REQUIRE([gl_NETDB_H_DEFAULTS])
  gl_CHECK_NEXT_HEADERS([netdb.h])
  if test $ac_cv_header_netdb_h = yes; then
    HAVE_NETDB_H=1
  else
    HAVE_NETDB_H=0
  fi
  AC_SUBST([HAVE_NETDB_H])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <netdb.h>]],
    [getaddrinfo freeaddrinfo gai_strerror getnameinfo])
])

# gl_NETDB_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_NETDB_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_NETDB_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_NETDB_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_NETDB_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_GETADDRINFO])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_NETDB_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_NETDB_H_DEFAULTS])
])

AC_DEFUN([gl_NETDB_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_STRUCT_ADDRINFO=1;   AC_SUBST([HAVE_STRUCT_ADDRINFO])
  HAVE_DECL_FREEADDRINFO=1; AC_SUBST([HAVE_DECL_FREEADDRINFO])
  HAVE_DECL_GAI_STRERROR=1; AC_SUBST([HAVE_DECL_GAI_STRERROR])
  HAVE_DECL_GETADDRINFO=1;  AC_SUBST([HAVE_DECL_GETADDRINFO])
  HAVE_DECL_GETNAMEINFO=1;  AC_SUBST([HAVE_DECL_GETNAMEINFO])
  REPLACE_GAI_STRERROR=0;   AC_SUBST([REPLACE_GAI_STRERROR])
  REPLACE_GETADDRINFO=0;    AC_SUBST([REPLACE_GETADDRINFO])
])
