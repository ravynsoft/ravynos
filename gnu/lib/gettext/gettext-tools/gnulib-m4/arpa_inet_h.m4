# arpa_inet_h.m4 serial 17
dnl Copyright (C) 2006, 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Written by Simon Josefsson and Bruno Haible

AC_DEFUN_ONCE([gl_ARPA_INET_H],
[
  dnl Ensure to expand the default settings once only, before all statements
  dnl that occur in other macros.
  AC_REQUIRE([gl_ARPA_INET_H_DEFAULTS])

  AC_CHECK_HEADERS_ONCE([arpa/inet.h])
  if test $ac_cv_header_arpa_inet_h = yes; then
    HAVE_ARPA_INET_H=1
  else
    HAVE_ARPA_INET_H=0
  fi
  AC_SUBST([HAVE_ARPA_INET_H])
  dnl <arpa/inet.h> is always overridden, because of GNULIB_POSIXCHECK.
  gl_CHECK_NEXT_HEADERS([arpa/inet.h])

  AC_REQUIRE([gl_FEATURES_H])

  gl_PREREQ_SYS_H_WS2TCPIP

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[
/* On some systems, this header is not self-consistent.  */
#if !(defined __GLIBC__ || defined __UCLIBC__)
# include <sys/socket.h>
#endif
#ifdef __TANDEM
# include <netdb.h>
#endif
#include <arpa/inet.h>
    ]], [inet_ntop inet_pton])
])

# gl_ARPA_INET_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_ARPA_INET_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_ARPA_INET_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_ARPA_INET_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_ARPA_INET_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_INET_NTOP])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_INET_PTON])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_ARPA_INET_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_ARPA_INET_H_DEFAULTS])
])

AC_DEFUN([gl_ARPA_INET_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_DECL_INET_NTOP=1;  AC_SUBST([HAVE_DECL_INET_NTOP])
  HAVE_DECL_INET_PTON=1;  AC_SUBST([HAVE_DECL_INET_PTON])
  REPLACE_INET_NTOP=0;    AC_SUBST([REPLACE_INET_NTOP])
  REPLACE_INET_PTON=0;    AC_SUBST([REPLACE_INET_PTON])
])
