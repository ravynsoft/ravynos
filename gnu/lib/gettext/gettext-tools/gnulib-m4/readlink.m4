# readlink.m4 serial 17
dnl Copyright (C) 2003, 2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_READLINK],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CHECK_FUNCS_ONCE([readlink])
  if test $ac_cv_func_readlink = no; then
    HAVE_READLINK=0
  else
    AC_CACHE_CHECK([whether readlink signature is correct],
      [gl_cv_decl_readlink_works],
      [AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM(
           [[#include <unistd.h>
      /* Cause compilation failure if original declaration has wrong type.  */
      ssize_t readlink (const char *, char *, size_t);]])],
         [gl_cv_decl_readlink_works=yes], [gl_cv_decl_readlink_works=no])])
    dnl Solaris 9 ignores trailing slash.
    dnl FreeBSD 7.2 dereferences only one level of links with trailing slash.
    AC_CACHE_CHECK([whether readlink handles trailing slash correctly],
      [gl_cv_func_readlink_trailing_slash],
      [# We have readlink, so assume ln -s works.
       ln -s conftest.no-such conftest.link
       ln -s conftest.link conftest.lnk2
       AC_RUN_IFELSE(
         [AC_LANG_PROGRAM(
           [[#include <unistd.h>
]], [[char buf[20];
      return readlink ("conftest.lnk2/", buf, sizeof buf) != -1;]])],
         [gl_cv_func_readlink_trailing_slash=yes],
         [gl_cv_func_readlink_trailing_slash=no],
         [case "$host_os" in
            # Guess yes on Linux or glibc systems.
            linux-* | linux | *-gnu* | gnu*)
              gl_cv_func_readlink_trailing_slash="guessing yes" ;;
            # Guess yes on systems that emulate the Linux system calls.
            midipix*)
              gl_cv_func_readlink_trailing_slash="guessing yes" ;;
            # Guess no on AIX or HP-UX.
            aix* | hpux*)
              gl_cv_func_readlink_trailing_slash="guessing no" ;;
            # If we don't know, obey --enable-cross-guesses.
            *)
              gl_cv_func_readlink_trailing_slash="$gl_cross_guess_normal" ;;
          esac
         ])
      rm -f conftest.link conftest.lnk2])
    case "$gl_cv_func_readlink_trailing_slash" in
      *yes)
        if test "$gl_cv_decl_readlink_works" != yes; then
          REPLACE_READLINK=1
        fi
        ;;
      *)
        AC_DEFINE([READLINK_TRAILING_SLASH_BUG], [1], [Define to 1 if readlink
          fails to recognize a trailing slash.])
        REPLACE_READLINK=1
        ;;
    esac

    AC_CACHE_CHECK([whether readlink truncates results correctly],
      [gl_cv_func_readlink_truncate],
      [# We have readlink, so assume ln -s works.
       ln -s ab conftest.link
       AC_RUN_IFELSE(
         [AC_LANG_PROGRAM(
           [[#include <unistd.h>
]], [[char c;
      return readlink ("conftest.link", &c, 1) != 1;]])],
         [gl_cv_func_readlink_truncate=yes],
         [gl_cv_func_readlink_truncate=no],
         [case "$host_os" in
            # Guess yes on Linux or glibc systems.
            linux-* | linux | *-gnu* | gnu*)
              gl_cv_func_readlink_truncate="guessing yes" ;;
            # Guess yes on systems that emulate the Linux system calls.
            midipix*)
              gl_cv_func_readlink_truncate="guessing yes" ;;
            # Guess no on AIX or HP-UX.
            aix* | hpux*)
              gl_cv_func_readlink_truncate="guessing no" ;;
            # If we don't know, obey --enable-cross-guesses.
            *)
              gl_cv_func_readlink_truncate="$gl_cross_guess_normal" ;;
          esac
         ])
      rm -f conftest.link conftest.lnk2])
    case $gl_cv_func_readlink_truncate in
      *yes)
        if test "$gl_cv_decl_readlink_works" != yes; then
          REPLACE_READLINK=1
        fi
        ;;
      *)
        AC_DEFINE([READLINK_TRUNCATE_BUG], [1], [Define to 1 if readlink
          sets errno instead of truncating a too-long link.])
        REPLACE_READLINK=1
        ;;
    esac
  fi
])

# Like gl_FUNC_READLINK, except prepare for separate compilation
# (no REPLACE_READLINK, no AC_LIBOBJ).
AC_DEFUN([gl_FUNC_READLINK_SEPARATE],
[
  AC_CHECK_FUNCS_ONCE([readlink])
  gl_PREREQ_READLINK
])

# Prerequisites of lib/readlink.c.
AC_DEFUN([gl_PREREQ_READLINK],
[
  :
])
