# sched_yield.m4 serial 3
dnl Copyright (C) 2019-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_SCHED_YIELD],
[
  AC_REQUIRE([gl_SCHED_H_DEFAULTS])
  AC_REQUIRE([gl_PTHREADLIB])
  AC_REQUIRE([AC_CANONICAL_HOST])

  if { case "$host_os" in mingw* | windows*) true;; *) false;; esac; } \
     && test $gl_threads_api = windows; then
    dnl Choose function names that don't conflict with the mingw-w64 winpthreads
    dnl library.
    REPLACE_SCHED_YIELD=1
  else
    AC_CHECK_DECL([sched_yield], , [HAVE_SCHED_YIELD=0],
      [[#include <sched.h>]])
  fi
])
