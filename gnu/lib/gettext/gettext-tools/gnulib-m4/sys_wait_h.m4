# sys_wait_h.m4 serial 9
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN_ONCE([gl_SYS_WAIT_H],
[
  AC_REQUIRE([gl_SYS_WAIT_H_DEFAULTS])

  dnl <sys/wait.h> is always overridden, because of GNULIB_POSIXCHECK.
  gl_CHECK_NEXT_HEADERS([sys/wait.h])

  dnl Ensure the type pid_t gets defined.
  AC_REQUIRE([AC_TYPE_PID_T])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <sys/wait.h>]],
    [waitpid])
])

# gl_SYS_WAIT_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_SYS_WAIT_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_SYS_WAIT_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_SYS_WAIT_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_SYS_WAIT_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_WAITPID])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_SYS_WAIT_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_SYS_WAIT_H_DEFAULTS])
])

AC_DEFUN([gl_SYS_WAIT_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
])
