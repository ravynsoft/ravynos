# time.m4 serial 4
dnl Copyright (C) 2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

AC_DEFUN([gl_FUNC_TIME],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  dnl glibc has the bug https://sourceware.org/bugzilla/show_bug.cgi?id=30200 .
  AC_CACHE_CHECK([whether time() works],
    [gl_cv_func_time_works],
    [dnl Guess that it works except on
     dnl   - glibc >= 2.31 with Linux. And binaries produced on glibc < 2.31
     dnl     need to run fine on newer glibc versions as well; therefore ignore
     dnl     __GLIBC_MINOR__.
     dnl   - AIX,
     dnl   - native Windows.
     case "$host_os" in
       linux*-gnu*)
         AC_EGREP_CPP([Unlucky], [
           #include <features.h>
           #ifdef __GNU_LIBRARY__
            #if __GLIBC__ == 2
             Unlucky GNU user
            #endif
           #endif
           ],
           [gl_cv_func_time_works="guessing no"],
           [gl_cv_func_time_works="guessing yes"])
         ;;
       aix*)              gl_cv_func_time_works="guessing no";;
       mingw* | windows*) gl_cv_func_time_works="guessing no";;
       *)                 gl_cv_func_time_works="guessing yes";;
     esac
    ])
  case "$gl_cv_func_time_works" in
    *no) REPLACE_TIME=1 ;;
  esac
])

# Prerequisites of lib/time.c.
AC_DEFUN([gl_PREREQ_TIME],
[
  :
])
