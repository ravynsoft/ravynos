# sleep.m4 serial 13
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_SLEEP],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  dnl We expect to see the declaration of sleep() in a header file.
  dnl Older versions of mingw have a sleep() function that is an alias to
  dnl _sleep() in MSVCRT. It has a different signature than POSIX sleep():
  dnl it takes the number of milliseconds as argument and returns void.
  dnl mingw does not declare this function.
  AC_CHECK_DECLS([sleep], , , [[#include <unistd.h>]])
  AC_CHECK_FUNCS_ONCE([sleep])
  if test $ac_cv_have_decl_sleep != yes; then
    HAVE_SLEEP=0
  else
    dnl Cygwin 1.5.x has a bug where sleep can't exceed 49.7 days.
    AC_CACHE_CHECK([for working sleep], [gl_cv_func_sleep_works],
      [AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <errno.h>
#include <unistd.h>
#include <signal.h>
static void
handle_alarm (int sig)
{
  if (sig != SIGALRM)
    _exit (2);
}
]], [[
    /* Failure to compile this test due to missing alarm is okay,
       since all such platforms (mingw) also lack sleep.  */
    unsigned int pentecost = 50 * 24 * 60 * 60; /* 50 days.  */
    unsigned int remaining;
    signal (SIGALRM, handle_alarm);
    alarm (1);
    remaining = sleep (pentecost);
    if (remaining > pentecost)
      return 3;
    if (remaining <= pentecost - 10)
      return 4;
    return 0;
    ]])],
      [gl_cv_func_sleep_works=yes], [gl_cv_func_sleep_works=no],
      [case "$host_os" in
                            # Guess yes on glibc systems.
         *-gnu* | gnu*)     gl_cv_func_sleep_works="guessing yes" ;;
                            # Guess yes on musl systems.
         *-musl*)           gl_cv_func_sleep_works="guessing yes" ;;
                            # Guess yes on systems that emulate the Linux system calls.
         midipix*)          gl_cv_func_sleep_works="guessing yes" ;;
                            # Guess no on native Windows.
         mingw* | windows*) gl_cv_func_sleep_works="guessing no" ;;
                            # If we don't know, obey --enable-cross-guesses.
         *)                 gl_cv_func_sleep_works="$gl_cross_guess_normal" ;;
       esac
      ])])
    case "$gl_cv_func_sleep_works" in
      *yes) ;;
      *)
        REPLACE_SLEEP=1
        ;;
    esac
  fi
])
