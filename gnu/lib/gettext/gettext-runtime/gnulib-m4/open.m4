# open.m4 serial 16
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_OPEN],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([gl_PREPROC_O_CLOEXEC])
  case "$host_os" in
    mingw* | windows* | pw*)
      REPLACE_OPEN=1
      ;;
    *)
      dnl open("foo/") should not create a file when the file name has a
      dnl trailing slash.  FreeBSD only has the problem on symlinks.
      AC_CHECK_FUNCS_ONCE([lstat])
      if test "$gl_cv_macro_O_CLOEXEC" != yes; then
        REPLACE_OPEN=1
      fi
      gl_OPEN_TRAILING_SLASH_BUG
      case "$gl_cv_func_open_slash" in
        *no)
          REPLACE_OPEN=1
          ;;
      esac
      ;;
  esac
  dnl Replace open() for supporting the gnulib-defined fchdir() function,
  dnl to keep fchdir's bookkeeping up-to-date.
  m4_ifdef([gl_FUNC_FCHDIR], [
    if test $REPLACE_OPEN = 0; then
      gl_TEST_FCHDIR
      if test $HAVE_FCHDIR = 0; then
        REPLACE_OPEN=1
      fi
    fi
  ])
  dnl Replace open() for supporting the gnulib-defined O_NONBLOCK flag.
  m4_ifdef([gl_NONBLOCKING_IO], [
    if test $REPLACE_OPEN = 0; then
      gl_NONBLOCKING_IO
      if test $gl_cv_have_open_O_NONBLOCK != yes; then
        REPLACE_OPEN=1
      fi
    fi
  ])
])

# Prerequisites of lib/open.c.
AC_DEFUN([gl_PREREQ_OPEN],
[
  AC_REQUIRE([gl_PROMOTED_TYPE_MODE_T])
  :
])
