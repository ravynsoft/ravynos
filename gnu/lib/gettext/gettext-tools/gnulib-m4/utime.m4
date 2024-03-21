# utime.m4 serial 5
dnl Copyright (C) 2017-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_UTIME],
[
  AC_REQUIRE([gl_UTIME_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CHECK_FUNCS_ONCE([lstat])
  case "$host_os" in
    mingw* | windows*)
      dnl On this platform, the original utime() or _utime() produces
      dnl timestamps that are affected by the time zone.
      dnl Use the function name 'rpl_utime' always, in order to avoid a
      dnl possible conflict with the function name 'utime' from oldnames.lib
      dnl (MSVC) or liboldnames.a (mingw).
      REPLACE_UTIME=1
      ;;
    *)
      AC_CHECK_FUNCS([utime])
      if test $ac_cv_func_utime = no; then
        HAVE_UTIME=0
      else
        dnl On macOS 10.13, utime("link-to-file/", NULL) mistakenly succeeds.
        AC_CACHE_CHECK([whether utime handles trailing slashes on files],
          [gl_cv_func_utime_file_slash],
          [touch conftest.tmp
           # Assume that if we have lstat, we can also check symlinks.
           if test $ac_cv_func_lstat = yes; then
             ln -s conftest.tmp conftest.lnk
           fi
           AC_RUN_IFELSE(
             [AC_LANG_PROGRAM(
               [[#include <stddef.h>
                 #include <utime.h>
               ]],
               [[int result = 0;
                 if (!utime ("conftest.tmp/", NULL))
                   result |= 1;
                 #if HAVE_LSTAT
                 if (!utime ("conftest.lnk/", NULL))
                   result |= 2;
                 #endif
                 return result;
               ]])],
             [gl_cv_func_utime_file_slash=yes],
             [gl_cv_func_utime_file_slash=no],
             [case "$host_os" in
                                 # Guess yes on Linux systems.
                linux-* | linux) gl_cv_func_utime_file_slash="guessing yes" ;;
                                 # Guess yes on glibc systems.
                *-gnu* | gnu*)   gl_cv_func_utime_file_slash="guessing yes" ;;
                                 # Guess no on macOS.
                darwin*)         gl_cv_func_utime_file_slash="guessing no" ;;
                                 # If we don't know, obey --enable-cross-guesses.
                *)               gl_cv_func_utime_file_slash="$gl_cross_guess_normal" ;;
              esac
             ])
           rm -f conftest.tmp conftest.lnk
          ])
        case $gl_cv_func_stat_file_slash in
          *no)
            REPLACE_UTIME=1
            AC_DEFINE([REPLACE_FUNC_UTIME_FILE], [1],
              [Define to 1 if utime needs help when passed a file name with a trailing slash])
            ;;
        esac
      fi
      ;;
  esac
])

# Prerequisites of lib/utime.c.
AC_DEFUN([gl_PREREQ_UTIME], [:])
