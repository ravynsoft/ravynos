# access.m4 serial 3
dnl Copyright (C) 2019-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_ACCESS],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  dnl On native Windows, access (= _access) does not support the X_OK mode.
  dnl It works by chance on some versions of mingw.
  case "$host_os" in
    mingw* | windows*)
      REPLACE_ACCESS=1
      ;;
    *)
      dnl Mac OS X 10.5 mistakenly allows access("link-to-file/",amode).
      AC_CHECK_FUNCS_ONCE([lstat])
      AC_CACHE_CHECK([whether access honors trailing slash],
        [gl_cv_func_access_slash_works],
        [# Assume that if we have lstat, we can also check symlinks.
         if test $ac_cv_func_lstat = yes; then
           rm -rf conftest.f conftest.lnk
           touch conftest.f || AC_MSG_ERROR([cannot create temporary files])
           ln -s conftest.f conftest.lnk
           AC_RUN_IFELSE(
             [AC_LANG_PROGRAM([[
                #include <unistd.h>
                ]],
                [[int result = 0;
                  if (access ("conftest.lnk/", R_OK) == 0)
                    result |= 1;
                  return result;
                ]])],
             [gl_cv_func_access_slash_works=yes],
             [gl_cv_func_access_slash_works=no],
             dnl When crosscompiling, assume access is broken.
             [case "$host_os" in
                                   # Guess yes on Linux systems.
                linux-* | linux)   gl_cv_func_access_slash_works="guessing yes" ;;
                                   # Guess yes on systems that emulate the Linux system calls.
                midipix*)          gl_cv_func_access_slash_works="guessing yes" ;;
                                   # Guess yes on glibc systems.
                *-gnu*)            gl_cv_func_access_slash_works="guessing yes" ;;
                                   # If we don't know, obey --enable-cross-guesses.
                *)                 gl_cv_func_access_slash_works="$gl_cross_guess_normal" ;;
              esac
             ])
           rm -rf conftest.f conftest.lnk
         else
           gl_cv_func_access_slash_works="guessing yes"
         fi
      ])
      case "$gl_cv_func_access_slash_works" in
        *yes) ;;
        *)
          REPLACE_ACCESS=1
          AC_DEFINE([ACCESS_TRAILING_SLASH_BUG], [1],
            [Define if access does not correctly handle trailing slashes.])
          ;;
      esac
      ;;
  esac
])
