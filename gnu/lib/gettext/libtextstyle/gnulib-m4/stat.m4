# serial 20

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_STAT],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([gl_SYS_STAT_H_DEFAULTS])
  AC_CHECK_FUNCS_ONCE([lstat])
  case "$host_os" in
    mingw* | windows*)
      dnl On this platform, the original stat() returns st_atime, st_mtime,
      dnl st_ctime values that are affected by the time zone.
      REPLACE_STAT=1
      ;;
    *)
      dnl AIX 7.1, Solaris 9, mingw64 mistakenly succeed on stat("file/").
      dnl (For mingw, this is due to a broken stat() override in libmingwex.a.)
      dnl FreeBSD 7.2 mistakenly succeeds on stat("link-to-file/").
      AC_CACHE_CHECK([whether stat handles trailing slashes on files],
        [gl_cv_func_stat_file_slash],
        [touch conftest.tmp
         # Assume that if we have lstat, we can also check symlinks.
         if test $ac_cv_func_lstat = yes; then
           ln -s conftest.tmp conftest.lnk
         fi
         AC_RUN_IFELSE(
           [AC_LANG_PROGRAM(
             [[#include <sys/stat.h>
]], [[int result = 0;
               struct stat st;
               if (!stat ("conftest.tmp/", &st))
                 result |= 1;
#if HAVE_LSTAT
               if (!stat ("conftest.lnk/", &st))
                 result |= 2;
#endif
               return result;
             ]])],
           [gl_cv_func_stat_file_slash=yes], [gl_cv_func_stat_file_slash=no],
           [case "$host_os" in
                               # Guess yes on Linux systems.
              linux-* | linux) gl_cv_func_stat_file_slash="guessing yes" ;;
                               # Guess yes on systems that emulate the Linux system calls.
              midipix*)        gl_cv_func_stat_file_slash="guessing yes" ;;
                               # Guess yes on glibc systems.
              *-gnu* | gnu*)   gl_cv_func_stat_file_slash="guessing yes" ;;
                               # If we don't know, obey --enable-cross-guesses.
              *)               gl_cv_func_stat_file_slash="$gl_cross_guess_normal" ;;
            esac
           ])
         rm -f conftest.tmp conftest.lnk])
      case $gl_cv_func_stat_file_slash in
        *no)
          REPLACE_STAT=1
          AC_DEFINE([REPLACE_FUNC_STAT_FILE], [1], [Define to 1 if stat needs
            help when passed a file name with a trailing slash]);;
      esac
      case $host_os in
        dnl Solaris stat can return a negative tv_nsec.
        solaris*)
          REPLACE_FSTAT=1 ;;
      esac
      ;;
  esac
])

# Prerequisites of lib/stat.c and lib/stat-w32.c.
AC_DEFUN([gl_PREREQ_STAT], [
  AC_REQUIRE([gl_SYS_STAT_H])
  AC_REQUIRE([gl_PREREQ_STAT_W32])
  :
])

# Prerequisites of lib/stat-w32.c.
AC_DEFUN([gl_PREREQ_STAT_W32], [
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw* | windows*)
      AC_CHECK_HEADERS([sdkddkver.h])
      ;;
  esac
])
