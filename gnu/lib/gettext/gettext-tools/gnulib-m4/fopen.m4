# fopen.m4 serial 16
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FOPEN_ITSELF],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw* | windows* | pw*)
      dnl Replace fopen, for handling of "/dev/null".
      REPLACE_FOPEN=1
      dnl fopen on mingw also has the trailing slash bug.
      gl_cv_func_fopen_slash="guessing no"
      ;;
    *)
      dnl fopen("foo/", "w") should not create a file when the file name has a
      dnl trailing slash.
      AC_CACHE_CHECK([whether fopen recognizes a trailing slash],
        [gl_cv_func_fopen_slash],
        [
          AC_RUN_IFELSE(
            [AC_LANG_SOURCE([[
#include <stddef.h>
#include <stdio.h>
int main ()
{
  FILE *fp = fopen ("conftest.sl/", "w");
  int result = (fp != NULL);
  if (fp != NULL)
    fclose (fp);
  return result;
}]])],
            [gl_cv_func_fopen_slash=yes],
            [gl_cv_func_fopen_slash=no],
            [
changequote(,)dnl
             case "$host_os" in
               aix* | hpux* | solaris2.[0-9] | solaris2.[0-9].*)
                 gl_cv_func_fopen_slash="guessing no" ;;
               *)
                 gl_cv_func_fopen_slash="guessing yes" ;;
             esac
changequote([,])dnl
            ])
          rm -f conftest.sl
        ])
      ;;
  esac
  case "$gl_cv_func_fopen_slash" in
    *no)
      AC_DEFINE([FOPEN_TRAILING_SLASH_BUG], [1],
        [Define to 1 if fopen() fails to recognize a trailing slash.])
      REPLACE_FOPEN=1
      ;;
  esac
])

AC_DEFUN([gl_FUNC_FOPEN],
[
  AC_REQUIRE([gl_FUNC_FOPEN_ITSELF])
  AC_REQUIRE([gl_FUNC_FCLOSE])
  if test $REPLACE_FCLOSE = 1; then
    REPLACE_FOPEN=1
  fi
])

AC_DEFUN([gl_FUNC_FOPEN_GNU],
[
  AC_REQUIRE([gl_FUNC_FOPEN])
  AC_CACHE_CHECK([whether fopen supports the mode character 'x'],
    [gl_cv_func_fopen_mode_x],
    [rm -f conftest.x
     AC_RUN_IFELSE(
       [AC_LANG_SOURCE([[
#include <stdio.h>
#include <errno.h>
int main ()
{
  FILE *fp;
  fp = fopen ("conftest.x", "w");
  fclose (fp);
  fp = fopen ("conftest.x", "wx");
  if (fp != NULL)
    /* 'x' ignored */
    return 1;
  else if (errno == EEXIST)
    return 0;
  else
    /* 'x' rejected */
    return 2;
}]])],
       [gl_cv_func_fopen_mode_x=yes],
       [gl_cv_func_fopen_mode_x=no],
       [case "$host_os" in
          # Guess yes on glibc and musl systems.
          linux*-gnu* | gnu* | kfreebsd*-gnu | *-musl* | midipix*)
            gl_cv_func_fopen_mode_x="guessing yes" ;;
          # If we don't know, obey --enable-cross-guesses.
          *)
            gl_cv_func_fopen_mode_x="$gl_cross_guess_normal" ;;
        esac
       ])
     rm -f conftest.x
    ])
  AC_CACHE_CHECK([whether fopen supports the mode character 'e'],
    [gl_cv_func_fopen_mode_e],
    [echo foo > conftest.x
     AC_RUN_IFELSE(
       [AC_LANG_SOURCE([[
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
]GL_MDA_DEFINES[
int main ()
{
  FILE *fp = fopen ("conftest.x", "re");
  if (fp != NULL)
    {
      if (fcntl (fileno (fp), F_GETFD) & FD_CLOEXEC)
        return 0;
      else
        /* 'e' ignored */
        return 1;
    }
  else
    /* 'e' rejected */
    return 2;
}]])],
       [gl_cv_func_fopen_mode_e=yes],
       [gl_cv_func_fopen_mode_e=no],
       [case "$host_os" in
          # Guess yes on glibc and musl systems.
          linux*-gnu* | gnu* | kfreebsd*-gnu | *-musl* | midipix*)
            gl_cv_func_fopen_mode_e="guessing yes" ;;
          # Guess no on native Windows.
          mingw* | windows*)
            gl_cv_func_fopen_mode_e="guessing no" ;;
          # If we don't know, obey --enable-cross-guesses.
          *)
            gl_cv_func_fopen_mode_e="$gl_cross_guess_normal" ;;
        esac
       ])
     rm -f conftest.x
    ])
  REPLACE_FOPEN_FOR_FOPEN_GNU="$REPLACE_FOPEN"
  case "$gl_cv_func_fopen_mode_x" in
    *no) REPLACE_FOPEN_FOR_FOPEN_GNU=1 ;;
  esac
  case "$gl_cv_func_fopen_mode_e" in
    *no) REPLACE_FOPEN_FOR_FOPEN_GNU=1 ;;
  esac
])

# Prerequisites of lib/fopen.c.
AC_DEFUN([gl_PREREQ_FOPEN], [:])
