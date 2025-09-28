# c32rtomb.m4 serial 7
dnl Copyright (C) 2020-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_C32RTOMB],
[
  AC_REQUIRE([gl_UCHAR_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])

  AC_REQUIRE([gl_MBRTOC32_SANITYCHECK])

  dnl Cf. gl_CHECK_FUNCS_ANDROID
  AC_CHECK_DECL([c32rtomb], , ,
    [[#ifdef __HAIKU__
       #include <stdint.h>
      #endif
      #include <uchar.h>
    ]])
  if test $ac_cv_have_decl_c32rtomb = yes; then
    dnl We can't use AC_CHECK_FUNC here, because c32rtomb() is defined as a
    dnl static inline function on Haiku 2020.
    AC_CACHE_CHECK([for c32rtomb], [gl_cv_func_c32rtomb],
      [AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <stdlib.h>
              #ifdef __HAIKU__
               #include <stdint.h>
              #endif
              #include <uchar.h>
            ]],
            [[char buf[8];
              return c32rtomb (buf, 0, NULL) == 0;
            ]])
         ],
         [gl_cv_func_c32rtomb=yes],
         [gl_cv_func_c32rtomb=no])
      ])
  else
    gl_cv_func_c32rtomb=no
  fi
  if test $gl_cv_func_c32rtomb = no; then
    HAVE_C32RTOMB=0
  else
    dnl When we override mbrtoc32, redefining the meaning of the char32_t
    dnl values, we need to override c32rtomb as well, for consistency.
    if test $HAVE_WORKING_MBRTOC32 = 0; then
      REPLACE_C32RTOMB=1
    fi
    AC_CACHE_CHECK([whether c32rtomb return value is correct],
      [gl_cv_func_c32rtomb_retval],
      [
        dnl Initial guess, used when cross-compiling.
changequote(,)dnl
        case "$host_os" in
          # Guess no on AIX.
          aix*) gl_cv_func_c32rtomb_retval="guessing no" ;;
          # Guess yes otherwise.
          *)    gl_cv_func_c32rtomb_retval="guessing yes" ;;
        esac
changequote([,])dnl
        AC_RUN_IFELSE(
          [AC_LANG_SOURCE([[
#include <stddef.h>
#ifdef __HAIKU__
 #include <stdint.h>
#endif
#include <uchar.h>
int main ()
{
  int result = 0;
  if (c32rtomb (NULL, 0, NULL) != 1)
    result |= 1;
  return result;
}]])],
          [gl_cv_func_c32rtomb_retval=yes],
          [gl_cv_func_c32rtomb_retval=no],
          [:])
      ])
    case "$gl_cv_func_c32rtomb_retval" in
      *yes) ;;
      *) AC_DEFINE([C32RTOMB_RETVAL_BUG], [1],
           [Define if the c32rtomb function has an incorrect return value.])
         REPLACE_C32RTOMB=1 ;;
    esac
  fi
])
