# rmdir.m4 serial 19
dnl Copyright (C) 2002, 2005, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_RMDIR],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  dnl Detect cygwin 1.5.x bug.
  AC_CHECK_HEADERS_ONCE([unistd.h])
  AC_CACHE_CHECK([whether rmdir works], [gl_cv_func_rmdir_works],
    [mkdir conftest.dir
     touch conftest.file
     AC_RUN_IFELSE(
       [AC_LANG_PROGRAM(
         [[#include <stdio.h>
           #include <errno.h>
           #if HAVE_UNISTD_H
           # include <unistd.h>
           #else /* on Windows with MSVC */
           # include <direct.h>
           #endif
         ]GL_MDA_DEFINES],
         [[int result = 0;
           if (!rmdir ("conftest.file/"))
             result |= 1;
           else if (errno != ENOTDIR)
             result |= 2;
           if (!rmdir ("conftest.dir/./"))
             result |= 4;
           return result;
         ]])],
       [gl_cv_func_rmdir_works=yes], [gl_cv_func_rmdir_works=no],
       [case "$host_os" in
                             # Guess yes on Linux systems.
          linux-* | linux)   gl_cv_func_rmdir_works="guessing yes" ;;
                             # Guess yes on systems that emulate the Linux system calls.
          midipix*)          gl_cv_func_rmdir_works="guessing yes" ;;
                             # Guess yes on glibc systems.
          *-gnu* | gnu*)     gl_cv_func_rmdir_works="guessing yes" ;;
                             # Guess no on native Windows.
          mingw* | windows*) gl_cv_func_rmdir_works="guessing no" ;;
                             # If we don't know, obey --enable-cross-guesses.
          *)                 gl_cv_func_rmdir_works="$gl_cross_guess_normal" ;;
        esac
       ])
     rm -rf conftest.dir conftest.file])
  case "$gl_cv_func_rmdir_works" in
    *yes) ;;
    *)
      REPLACE_RMDIR=1
      ;;
  esac
])
