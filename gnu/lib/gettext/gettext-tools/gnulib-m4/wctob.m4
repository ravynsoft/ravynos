# wctob.m4 serial 14
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_WCTOB],
[
  AC_REQUIRE([gl_WCHAR_H_DEFAULTS])

  AC_CHECK_FUNCS_ONCE([wctob])
  if test $ac_cv_func_wctob = no; then
    HAVE_WCTOB=0
    HAVE_DECL_WCTOB=0
  else
    HAVE_WCTOB=1

    dnl Solaris 9 has the wctob() function but it does not work.
    dnl Cygwin 1.7.2 has the wctob() function but it clobbers caller-owned
    dnl registers, see <https://cygwin.com/ml/cygwin/2010-05/msg00015.html>.
    AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([gt_LOCALE_FR])
    AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
    AC_CACHE_CHECK([whether wctob works],
      [gl_cv_func_wctob_works],
      [
        dnl Initial guess, used when cross-compiling or when no suitable locale
        dnl is present.
changequote(,)dnl
        case "$host_os" in
            # Guess no on Solaris <= 9 and Cygwin.
          solaris2.[1-9] | solaris2.[1-9].* | cygwin*)
            gl_cv_func_wctob_works="guessing no" ;;
            # Guess no on native Windows.
          mingw* | windows*)
            gl_cv_func_wctob_works="guessing no" ;;
            # Guess yes otherwise.
          *) gl_cv_func_wctob_works="guessing yes" ;;
        esac
changequote([,])dnl
        case "$host_os" in
          cygwin*)
            AC_RUN_IFELSE(
              [AC_LANG_SOURCE([[
#include <locale.h>
#include <wchar.h>

register long global __asm__ ("%ebx");

int main ()
{
  setlocale (LC_ALL, "en_US.UTF-8");

  global = 0x12345678;
  if (wctob (0x00FC) != -1)
    return 1;
  if (global != 0x12345678)
    return 2;
  return 0;
}]])],
              [:],
              [gl_cv_func_wctob_works=no],
              [:])
            ;;
        esac
        if test "$gl_cv_func_wctob_works" != no && test $LOCALE_FR != none; then
          AC_RUN_IFELSE(
            [AC_LANG_SOURCE([[
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>
int main ()
{
  if (setlocale (LC_ALL, "$LOCALE_FR") != NULL)
    {
      wchar_t wc;

      if (mbtowc (&wc, "\374", 1) == 1)
        if (wctob (wc) != (unsigned char) '\374')
          return 1;
    }
  return 0;
}]])],
            [gl_cv_func_wctob_works=yes],
            [gl_cv_func_wctob_works=no],
            [:])
        fi
      ])
    case "$gl_cv_func_wctob_works" in
      *yes) ;;
      *) REPLACE_WCTOB=1 ;;
    esac
    if test $REPLACE_WCTOB = 0; then

      dnl IRIX 6.5 has the wctob() function but does not declare it.
      AC_CHECK_DECLS([wctob], [], [], [[
        #include <wchar.h>
      ]])
      if test $ac_cv_have_decl_wctob != yes; then
        HAVE_DECL_WCTOB=0
      fi
    fi
  fi
])

# Prerequisites of lib/wctob.c.
AC_DEFUN([gl_PREREQ_WCTOB], [
  :
])
