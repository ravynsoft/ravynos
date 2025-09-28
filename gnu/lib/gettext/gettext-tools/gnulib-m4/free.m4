# free.m4 serial 6
# Copyright (C) 2003-2005, 2009-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# Written by Paul Eggert and Bruno Haible.

AC_DEFUN([gl_FUNC_FREE],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])

  dnl In the next release of POSIX, free must preserve errno.
  dnl https://www.austingroupbugs.net/view.php?id=385
  dnl https://sourceware.org/bugzilla/show_bug.cgi?id=17924
  dnl So far, we know of three platforms that do this:
  dnl * glibc >= 2.33, thanks to the fix for this bug:
  dnl   <https://sourceware.org/bugzilla/show_bug.cgi?id=17924>
  dnl * OpenBSD >= 4.5, thanks to this commit:
  dnl   <https://cvsweb.openbsd.org/cgi-bin/cvsweb/src/lib/libc/stdlib/malloc.c.diff?r1=1.100&r2=1.101&f=h>
  dnl * Solaris, because its malloc() implementation is based on brk(),
  dnl   not mmap(); hence its free() implementation makes no system calls.
  dnl For other platforms, you can only be sure if they state it in their
  dnl documentation, or by code inspection of the free() implementation in libc.
  AC_CACHE_CHECK([whether free is known to preserve errno],
    [gl_cv_func_free_preserves_errno],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <stdlib.h>
          ]],
          [[#if 2 < __GLIBC__ + (33 <= __GLIBC_MINOR__)
            #elif defined __OpenBSD__
            #elif defined __sun
            #else
              #error "'free' is not known to preserve errno"
            #endif
          ]])],
       [gl_cv_func_free_preserves_errno=yes],
       [gl_cv_func_free_preserves_errno=no])
    ])

  case $gl_cv_func_free_preserves_errno in
   *yes)
    AC_DEFINE([HAVE_FREE_POSIX], [1],
      [Define if the 'free' function is guaranteed to preserve errno.])
    ;;
   *) REPLACE_FREE=1 ;;
  esac
])

# Prerequisites of lib/free.c.
AC_DEFUN([gl_PREREQ_FREE], [:])
