# fstat.m4 serial 9
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FSTAT],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([gl_SYS_STAT_H_DEFAULTS])

  case "$host_os" in
    mingw* | windows* | solaris*)
      dnl On MinGW, the original stat() returns st_atime, st_mtime,
      dnl st_ctime values that are affected by the time zone.
      dnl Solaris stat can return a negative tv_nsec.
      REPLACE_FSTAT=1
      ;;
  esac

  dnl Replace fstat() for supporting the gnulib-defined open() on directories.
  m4_ifdef([gl_FUNC_FCHDIR], [
    gl_TEST_FCHDIR
    if test $HAVE_FCHDIR = 0; then
      case "$gl_cv_func_open_directory_works" in
        *yes) ;;
        *)
          REPLACE_FSTAT=1
          ;;
      esac
    fi
  ])
])

# Prerequisites of lib/fstat.c and lib/stat-w32.c.
AC_DEFUN([gl_PREREQ_FSTAT], [
  AC_REQUIRE([gl_SYS_STAT_H])
  AC_REQUIRE([gl_PREREQ_STAT_W32])
  :
])
