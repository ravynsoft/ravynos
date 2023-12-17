# ungetc.m4 serial 12
dnl Copyright (C) 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN_ONCE([gl_FUNC_UNGETC_WORKS],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles

  AC_CACHE_CHECK([whether ungetc works on arbitrary bytes],
    [gl_cv_func_ungetc_works],
    [AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
      ]], [[FILE *f;
            if (!(f = fopen ("conftest.tmp", "w+")))
              return 1;
            if (fputs ("abc", f) < 0)
              { fclose (f); return 2; }
            rewind (f);
            if (fgetc (f) != 'a')
              { fclose (f); return 3; }
            if (fgetc (f) != 'b')
              { fclose (f); return 4; }
            if (ungetc ('d', f) != 'd')
              { fclose (f); return 5; }
            if (ftell (f) != 1)
              { fclose (f); return 6; }
            if (fgetc (f) != 'd')
              { fclose (f); return 7; }
            if (ftell (f) != 2)
              { fclose (f); return 8; }
            if (fseek (f, 0, SEEK_CUR) != 0)
              { fclose (f); return 9; }
            if (ftell (f) != 2)
              { fclose (f); return 10; }
            if (fgetc (f) != 'c')
              { fclose (f); return 11; }
            fclose (f);
            remove ("conftest.tmp");
          ]])],
        [gl_cv_func_ungetc_works=yes], [gl_cv_func_ungetc_works=no],
        [case "$host_os" in
                               # Guess yes on glibc systems.
           *-gnu* | gnu*)      gl_cv_func_ungetc_works="guessing yes" ;;
                               # Guess yes on musl systems.
           *-musl* | midipix*) gl_cv_func_ungetc_works="guessing yes" ;;
                               # Guess yes on bionic systems.
           *-android*)         gl_cv_func_ungetc_works="guessing yes" ;;
                               # Guess yes on native Windows.
           mingw* | windows*)  gl_cv_func_ungetc_works="guessing yes" ;;
                               # If we don't know, obey --enable-cross-guesses.
           *)                  gl_cv_func_ungetc_works="$gl_cross_guess_normal" ;;
         esac
        ])
    ])
  gl_ftello_broken_after_ungetc=no
  case "$gl_cv_func_ungetc_works" in
    *yes) ;;
    *)
      dnl On macOS >= 10.15, where the above program fails with exit code 6,
      dnl we fix it through an ftello override.
      case "$host_os" in
        darwin*) gl_ftello_broken_after_ungetc=yes ;;
        *)
          AC_DEFINE([FUNC_UNGETC_BROKEN], [1],
            [Define to 1 if ungetc is broken when used on arbitrary bytes.])
          ;;
      esac
      ;;
  esac
])
