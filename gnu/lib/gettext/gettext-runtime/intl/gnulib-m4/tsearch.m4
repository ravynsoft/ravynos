# tsearch.m4 serial 13
dnl Copyright (C) 2006-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_TSEARCH],
[
  AC_REQUIRE([gl_SEARCH_H_DEFAULTS])
  gl_CHECK_FUNCS_ANDROID([tsearch], [[#include <search.h>]])
  gl_CHECK_FUNCS_ANDROID([twalk], [[#include <search.h>]])
  if test $ac_cv_func_tsearch = yes; then
    dnl On OpenBSD 4.0, the return value of tdelete() is incorrect.
    AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
    AC_CACHE_CHECK([whether tdelete works], [gl_cv_func_tdelete_works],
      [
        AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stddef.h>
#include <search.h>
static int
cmp_fn (const void *a, const void *b)
{
  return *(const int *) a - *(const int *) b;
}
int
main ()
{
  int result = 0;
  int x = 0;
  void *root = NULL;
  if (!(tfind (&x, &root, cmp_fn) == NULL))
    result |= 1;
  tsearch (&x, &root, cmp_fn);
  if (!(tfind (&x, &root, cmp_fn) != NULL))
    result |= 2;
  if (!(tdelete (&x, &root, cmp_fn) != NULL))
    result |= 4;
  return result;
}]])], [gl_cv_func_tdelete_works=yes], [gl_cv_func_tdelete_works=no],
            [case "$host_os" in
               openbsd*)            gl_cv_func_tdelete_works="guessing no" ;;
                                    # Guess yes on native Windows.
               mingw* | windows*)   gl_cv_func_tdelete_works="guessing yes" ;;
               *)                   gl_cv_func_tdelete_works="guessing yes" ;;
             esac
            ])
      ])
    case "$gl_cv_func_tdelete_works" in
      *no)
        REPLACE_TSEARCH=1
        REPLACE_TWALK=1
        ;;
    esac
  else
    HAVE_TSEARCH=0
    case "$gl_cv_onwards_func_tsearch" in
      future*) REPLACE_TSEARCH=1 ;;
    esac
  fi
  if test $ac_cv_func_twalk != yes; then
    HAVE_TWALK=0
    case "$gl_cv_onwards_func_twalk" in
      future*) REPLACE_TWALK=1 ;;
    esac
  fi
])

# Prerequisites of lib/tsearch.c.
AC_DEFUN([gl_PREREQ_TSEARCH], [
  :
])
