# opendir.m4 serial 7
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_OPENDIR],
[
  AC_REQUIRE([gl_DIRENT_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles

  AC_CHECK_FUNCS([opendir])
  if test $ac_cv_func_opendir = no; then
    HAVE_OPENDIR=0
  else
    dnl Replace opendir() on native Windows and OS/2 kLIBC,
    dnl to support fdopendir().
    AC_REQUIRE([gl_DIRENT_DIR])
    if test $DIR_HAS_FD_MEMBER = 0; then
      REPLACE_OPENDIR=1
    fi
    dnl Replace opendir() for supporting the gnulib-defined fchdir() function,
    dnl to keep fchdir's bookkeeping up-to-date.
    m4_ifdef([gl_FUNC_FCHDIR], [
      gl_TEST_FCHDIR
      if test $HAVE_FCHDIR = 0; then
        REPLACE_OPENDIR=1
      fi
    ])
  fi
])
