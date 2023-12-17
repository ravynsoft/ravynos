# strtol.m4 serial 10
dnl Copyright (C) 2002-2003, 2006, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_STRTOL],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CHECK_FUNCS([strtol])
  if test $ac_cv_func_strtol = yes; then
    AC_CACHE_CHECK([whether strtol works],
      [gl_cv_func_strtol_works],
      [AC_RUN_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <stdlib.h>]],
            [[int result = 0;
              char *term;
              /* This test fails on Minix and native Windows.  */
              {
                static char const input[2][3] = {"0x", "0b"};
                static int const base[] = {0, 2, 10};
                int i, j;
                for (i = 0; i < 2; i++)
                  for (j = 0; j < 3; j++)
                    {
                      (void) strtol (input[i], &term, base[j]);
                      if (term != input[i] + 1)
                        result |= 1;
                    }
              }
              /* This test fails on pre-C23 platforms.  */
              {
                const char input[] = "0b1";
                (void) strtol (input, &term, 2);
                if (term != input + 3)
                  result |= 2;
              }
              return result;
            ]])
         ],
         [gl_cv_func_strtol_works=yes],
         [gl_cv_func_strtol_works=no],
         [case "$host_os" in
                                # Guess no on native Windows.
            mingw* | windows*)  gl_cv_func_strtol_works="guessing no" ;;
                                # Guess no on glibc systems.
            *-gnu* | gnu*)      gl_cv_func_strtol_works="guessing no" ;;
                                # Guess no on musl systems.
            *-musl* | midipix*) gl_cv_func_strtol_works="guessing no" ;;
            *)                  gl_cv_func_strtol_works="$gl_cross_guess_normal" ;;
          esac
         ])
    ])
    case "$gl_cv_func_strtol_works" in
      *yes) ;;
      *)    REPLACE_STRTOL=1 ;;
    esac
  else
    HAVE_STRTOL=0
  fi
])
