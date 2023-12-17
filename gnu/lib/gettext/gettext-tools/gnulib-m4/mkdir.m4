# serial 20

# Copyright (C) 2001, 2003-2004, 2006, 2008-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# On some systems, mkdir ("foo/", 0700) fails because of the trailing slash.
# On others, mkdir ("foo/./", 0700) mistakenly succeeds.
# On such systems, arrange to use a wrapper function.
AC_DEFUN([gl_FUNC_MKDIR],
[dnl
  AC_REQUIRE([gl_SYS_STAT_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CHECK_HEADERS_ONCE([unistd.h])
  AC_CACHE_CHECK([whether mkdir handles trailing slash],
    [gl_cv_func_mkdir_trailing_slash_works],
    [rm -rf conftest.dir
     AC_RUN_IFELSE(
       [AC_LANG_PROGRAM([[
          #include <sys/types.h>
          #include <sys/stat.h>
          ]GL_MDA_DEFINES],
          [[return mkdir ("conftest.dir/", 0700);]])],
       [gl_cv_func_mkdir_trailing_slash_works=yes],
       [gl_cv_func_mkdir_trailing_slash_works=no],
       [case "$host_os" in
                             # Guess yes on Linux systems.
          linux-* | linux)   gl_cv_func_mkdir_trailing_slash_works="guessing yes" ;;
                             # Guess yes on systems that emulate the Linux system calls.
          midipix*)          gl_cv_func_mkdir_trailing_slash_works="guessing yes" ;;
                             # Guess yes on glibc systems.
          *-gnu* | gnu*)     gl_cv_func_mkdir_trailing_slash_works="guessing yes" ;;
                             # Guess yes on MSVC, no on mingw.
          windows*-msvc*)    gl_cv_func_mkdir_trailing_slash_works="guessing yes" ;;
          mingw* | windows*) AC_EGREP_CPP([Known], [
#ifdef _MSC_VER
 Known
#endif
                               ],
                               [gl_cv_func_mkdir_trailing_slash_works="guessing yes"],
                               [gl_cv_func_mkdir_trailing_slash_works="guessing no"])
                             ;;
                             # If we don't know, obey --enable-cross-guesses.
          *)                 gl_cv_func_mkdir_trailing_slash_works="$gl_cross_guess_normal" ;;
        esac
       ])
     rm -rf conftest.dir
    ])
  case "$gl_cv_func_mkdir_trailing_slash_works" in
    *yes) ;;
    *)
      REPLACE_MKDIR=1
      ;;
  esac

  AC_CACHE_CHECK([whether mkdir handles trailing dot],
    [gl_cv_func_mkdir_trailing_dot_works],
    [rm -rf conftest.dir
     AC_RUN_IFELSE(
       [AC_LANG_PROGRAM([[
          #include <sys/types.h>
          #include <sys/stat.h>
          ]GL_MDA_DEFINES],
          [[return !mkdir ("conftest.dir/./", 0700);]])],
       [gl_cv_func_mkdir_trailing_dot_works=yes],
       [gl_cv_func_mkdir_trailing_dot_works=no],
       [case "$host_os" in
                             # Guess yes on glibc systems.
          *-gnu* | gnu*)     gl_cv_func_mkdir_trailing_dot_works="guessing yes" ;;
                             # Guess yes on musl systems.
          *-musl*)           gl_cv_func_mkdir_trailing_dot_works="guessing yes" ;;
                             # Guess yes on systems that emulate the Linux system calls.
          midipix*)          gl_cv_func_mkdir_trailing_dot_works="guessing yes" ;;
                             # Guess no on native Windows.
          mingw* | windows*) gl_cv_func_mkdir_trailing_dot_works="guessing no" ;;
                             # If we don't know, obey --enable-cross-guesses.
          *)                 gl_cv_func_mkdir_trailing_dot_works="$gl_cross_guess_normal" ;;
        esac
       ])
     rm -rf conftest.dir
    ]
  )
  case "$gl_cv_func_mkdir_trailing_dot_works" in
    *yes) ;;
    *)
      REPLACE_MKDIR=1
      AC_DEFINE([FUNC_MKDIR_DOT_BUG], [1], [Define to 1 if mkdir mistakenly
        creates a directory given with a trailing dot component.])
      ;;
  esac
])
