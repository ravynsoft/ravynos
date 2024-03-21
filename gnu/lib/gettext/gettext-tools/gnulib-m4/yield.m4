# yield.m4 serial 5
dnl Copyright (C) 2005-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_YIELD],
[
  AC_REQUIRE([gl_PTHREADLIB])
  AC_REQUIRE([gl_THREADLIB])

  if test $gl_threads_api = posix; then
    YIELD_LIB="$SCHED_YIELD_LIB"
  else
    YIELD_LIB=
  fi
  AC_SUBST([YIELD_LIB])
])
