# select.m4 serial 16
dnl Copyright (C) 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_SELECT],
[
  AC_REQUIRE([gl_SYS_SELECT_H])
  AC_REQUIRE([AC_C_RESTRICT])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_REQUIRE([gl_SOCKETS])
  if test "$ac_cv_header_winsock2_h" = yes; then
    REPLACE_SELECT=1
  else
    dnl On Interix 3.5, select(0, NULL, NULL, NULL, timeout) fails with error
    dnl EFAULT.
    AC_CHECK_HEADERS_ONCE([sys/select.h])
    AC_CACHE_CHECK([whether select supports a 0 argument],
      [gl_cv_func_select_supports0],
      [
        AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/time.h>
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
int main ()
{
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 5;
  return select (0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &timeout) < 0;
}]])], [gl_cv_func_select_supports0=yes], [gl_cv_func_select_supports0=no],
          [
changequote(,)dnl
           case "$host_os" in
                       # Guess no on Interix.
             interix*) gl_cv_func_select_supports0="guessing no";;
                       # Guess yes otherwise.
             *)        gl_cv_func_select_supports0="guessing yes";;
           esac
changequote([,])dnl
          ])
      ])
    case "$gl_cv_func_select_supports0" in
      *yes) ;;
      *) REPLACE_SELECT=1 ;;
    esac

    dnl On FreeBSD 8.2, select() doesn't always reject bad fds.
    AC_CACHE_CHECK([whether select detects invalid fds],
      [gl_cv_func_select_detects_ebadf],
      [
        AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/time.h>
#if HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#include <unistd.h>
#include <errno.h>
]GL_MDA_DEFINES],
[[
  fd_set set;
  dup2(0, 16);
  FD_ZERO(&set);
  FD_SET(16, &set);
  close(16);
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 5;
  return select (17, &set, NULL, NULL, &timeout) != -1 || errno != EBADF;
]])], [gl_cv_func_select_detects_ebadf=yes],
      [gl_cv_func_select_detects_ebadf=no],
          [
           case "$host_os" in
                             # Guess yes on Linux systems.
            linux-* | linux) gl_cv_func_select_detects_ebadf="guessing yes" ;;
                             # Guess yes on systems that emulate the Linux system calls.
            midipix*)        gl_cv_func_select_detects_ebadf="guessing yes" ;;
                             # Guess yes on glibc systems.
            *-gnu* | gnu*)   gl_cv_func_select_detects_ebadf="guessing yes" ;;
                             # If we don't know, obey --enable-cross-guesses.
            *)               gl_cv_func_select_detects_ebadf="$gl_cross_guess_normal" ;;
           esac
          ])
      ])
    case $gl_cv_func_select_detects_ebadf in
      *yes) ;;
      *) REPLACE_SELECT=1 ;;
    esac
  fi

  dnl Determine the needed libraries.
  SELECT_LIB="$LIBSOCKET"
  if test $REPLACE_SELECT = 1; then
    case "$host_os" in
      mingw* | windows*)
        dnl On the MSVC platform, the function MsgWaitForMultipleObjects
        dnl (used in lib/select.c) requires linking with -luser32. On mingw,
        dnl it is implicit.
        AC_LINK_IFELSE(
          [AC_LANG_SOURCE([[
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
int
main ()
{
  MsgWaitForMultipleObjects (0, NULL, 0, 0, 0);
  return 0;
}]])],
          [],
          [SELECT_LIB="$SELECT_LIB -luser32"])
        ;;
    esac
  fi
  AC_SUBST([SELECT_LIB])
  dnl For backward compatibility.
  LIB_SELECT="$LIB_SELECT"
  AC_SUBST([LIB_SELECT])
])
