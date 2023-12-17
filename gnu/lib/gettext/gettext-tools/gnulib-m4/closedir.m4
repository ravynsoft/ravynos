# closedir.m4 serial 8
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_CLOSEDIR],
[
  AC_REQUIRE([gl_DIRENT_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles

  AC_CHECK_FUNCS([closedir])
  if test $ac_cv_func_closedir = no; then
    HAVE_CLOSEDIR=0
  else
    dnl Replace closedir() on native Windows and OS/2 kLIBC,
    dnl to support fdopendir().
    AC_REQUIRE([gl_DIRENT_DIR])
    if test $DIR_HAS_FD_MEMBER = 0; then
      REPLACE_CLOSEDIR=1
    fi
    dnl Replace closedir() for supporting the gnulib-defined fchdir() function,
    dnl to keep fchdir's bookkeeping up-to-date.
    m4_ifdef([gl_FUNC_FCHDIR], [
      gl_TEST_FCHDIR
      if test $HAVE_FCHDIR = 0; then
        REPLACE_CLOSEDIR=1
      fi
    ])
  fi
])
