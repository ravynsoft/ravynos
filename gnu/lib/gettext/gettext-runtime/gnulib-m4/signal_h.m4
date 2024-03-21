# signal_h.m4 serial 22
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN_ONCE([gl_SIGNAL_H],
[
  AC_REQUIRE([gl_SIGNAL_H_DEFAULTS])
  AC_REQUIRE([gl_CHECK_TYPE_SIGSET_T])
  gl_NEXT_HEADERS([signal.h])

# AIX declares sig_atomic_t to already include volatile, and C89 compilers
# then choke on 'volatile sig_atomic_t'.  C99 requires that it compile.
  AC_CHECK_TYPE([volatile sig_atomic_t], [],
    [HAVE_TYPE_VOLATILE_SIG_ATOMIC_T=0], [[
#include <signal.h>
    ]])

  dnl Ensure the type pid_t gets defined.
  AC_REQUIRE([AC_TYPE_PID_T])

  AC_REQUIRE([AC_TYPE_UID_T])

  dnl Persuade glibc <signal.h> to define sighandler_t.
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])
  AC_CHECK_TYPE([sighandler_t], [], [HAVE_SIGHANDLER_T=0], [[
#include <signal.h>
    ]])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <signal.h>
    ]], [pthread_sigmask sigaction
    sigaddset sigdelset sigemptyset sigfillset sigismember
    sigpending sigprocmask])

  AC_REQUIRE([AC_C_RESTRICT])
])

AC_DEFUN([gl_CHECK_TYPE_SIGSET_T],
[
  AC_CHECK_TYPES([sigset_t],
    [gl_cv_type_sigset_t=yes], [gl_cv_type_sigset_t=no],
    [[
      #include <signal.h>
      /* Mingw defines sigset_t not in <signal.h>, but in <sys/types.h>.  */
      #include <sys/types.h>
    ]])
  if test $gl_cv_type_sigset_t != yes; then
    HAVE_SIGSET_T=0
  fi
])

# gl_SIGNAL_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_SIGNAL_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_SIGNAL_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_SIGNAL_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_SIGNAL_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_PTHREAD_SIGMASK])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_RAISE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SIGNAL_H_SIGPIPE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SIGPROCMASK])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SIGACTION])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_SIGNAL_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_SIGNAL_H_DEFAULTS])
])

AC_DEFUN([gl_SIGNAL_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_POSIX_SIGNALBLOCKING=1; AC_SUBST([HAVE_POSIX_SIGNALBLOCKING])
  HAVE_PTHREAD_SIGMASK=1;      AC_SUBST([HAVE_PTHREAD_SIGMASK])
  HAVE_RAISE=1;                AC_SUBST([HAVE_RAISE])
  HAVE_SIGSET_T=1;             AC_SUBST([HAVE_SIGSET_T])
  HAVE_SIGINFO_T=1;            AC_SUBST([HAVE_SIGINFO_T])
  HAVE_SIGACTION=1;            AC_SUBST([HAVE_SIGACTION])
  HAVE_STRUCT_SIGACTION_SA_SIGACTION=1;
                               AC_SUBST([HAVE_STRUCT_SIGACTION_SA_SIGACTION])
  HAVE_TYPE_VOLATILE_SIG_ATOMIC_T=1;
                               AC_SUBST([HAVE_TYPE_VOLATILE_SIG_ATOMIC_T])
  HAVE_SIGHANDLER_T=1;         AC_SUBST([HAVE_SIGHANDLER_T])
  REPLACE_PTHREAD_SIGMASK=0;   AC_SUBST([REPLACE_PTHREAD_SIGMASK])
  REPLACE_RAISE=0;             AC_SUBST([REPLACE_RAISE])
])
