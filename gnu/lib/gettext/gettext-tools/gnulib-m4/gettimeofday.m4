# serial 30

# Copyright (C) 2001-2003, 2005, 2007, 2009-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl From Jim Meyering.

AC_DEFUN([gl_FUNC_GETTIMEOFDAY],
[
  AC_REQUIRE([gl_SYS_TIME_H_DEFAULTS])
  AC_REQUIRE([AC_C_RESTRICT])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([gl_SYS_TIME_H])
  AC_CHECK_FUNCS_ONCE([gettimeofday])

  gl_gettimeofday_timezone=void
  if test $ac_cv_func_gettimeofday != yes; then
    HAVE_GETTIMEOFDAY=0
  else
    AC_CACHE_CHECK([for gettimeofday with POSIX signature],
      [gl_cv_func_gettimeofday_posix_signature],
      [AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <sys/time.h>
              struct timeval c;
              int gettimeofday (struct timeval *restrict, void *restrict);
            ]],
            [[/* glibc uses struct timezone * rather than the POSIX void *
                 if _GNU_SOURCE is defined.  However, since the only portable
                 use of gettimeofday uses NULL as the second parameter, and
                 since the glibc definition is actually more typesafe, it is
                 not worth wrapping this to get a compliant signature.  */
              int (*f) (struct timeval *restrict, void *restrict)
                = gettimeofday;
              int x = f (&c, 0);
              return !(x | c.tv_sec | c.tv_usec);
            ]])],
          [gl_cv_func_gettimeofday_posix_signature=yes],
          [AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM(
              [[#include <sys/time.h>
int gettimeofday (struct timeval *restrict, struct timezone *restrict);
              ]])],
            [gl_cv_func_gettimeofday_posix_signature=almost],
            [gl_cv_func_gettimeofday_posix_signature=no])])])
    if test $gl_cv_func_gettimeofday_posix_signature = almost; then
      gl_gettimeofday_timezone='struct timezone'
    elif test $gl_cv_func_gettimeofday_posix_signature != yes; then
      REPLACE_GETTIMEOFDAY=1
    fi
    dnl If we override 'struct timeval', we also have to override gettimeofday.
    if test $REPLACE_STRUCT_TIMEVAL = 1; then
      REPLACE_GETTIMEOFDAY=1
    fi
    dnl On mingw, the original gettimeofday has only a precision of 15.6
    dnl milliseconds. So override it.
    case "$host_os" in
      mingw* | windows*) REPLACE_GETTIMEOFDAY=1 ;;
    esac
  fi
  AC_DEFINE_UNQUOTED([GETTIMEOFDAY_TIMEZONE], [$gl_gettimeofday_timezone],
    [Define this to 'void' or 'struct timezone' to match the system's
     declaration of the second argument to gettimeofday.])
])

# Prerequisites of lib/gettimeofday.c.
AC_DEFUN([gl_PREREQ_GETTIMEOFDAY], [:])
