# setlocale.m4 serial 10
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_SETLOCALE],
[
  AC_REQUIRE([gl_LOCALE_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_SETLOCALE_NULL])
  AC_REQUIRE([AC_CANONICAL_HOST])

  dnl Test whether we need to improve on the general working of setlocale.
  NEED_SETLOCALE_IMPROVED=0
  case "$host_os" in
    dnl On native Windows systems, setlocale(category,NULL) does not look at
    dnl the environment variables LC_ALL, category, and LANG.
    mingw* | windows*) NEED_SETLOCALE_IMPROVED=1 ;;
    dnl On Cygwin 1.5.x, setlocale always succeeds but setlocale(LC_CTYPE,NULL)
    dnl is then still "C".
    cygwin*)
      case `uname -r` in
        1.5.*) NEED_SETLOCALE_IMPROVED=1 ;;
      esac
      ;;
    dnl On Android 4.3, setlocale(category,"C") always fails.
    *)
      AC_CACHE_CHECK([whether setlocale supports the C locale],
        [gl_cv_func_setlocale_works],
        [AC_RUN_IFELSE(
           [AC_LANG_SOURCE([[
#include <locale.h>
int main ()
{
  return setlocale (LC_ALL, "C") == NULL;
}]])],
           [gl_cv_func_setlocale_works=yes],
           [gl_cv_func_setlocale_works=no],
           [case "$host_os" in
                               # Guess no on Android.
              linux*-android*) gl_cv_func_setlocale_works="guessing no";;
                               # Guess yes otherwise.
              *)               gl_cv_func_setlocale_works="guessing yes";;
            esac
           ])
        ])
      case "$gl_cv_func_setlocale_works" in
        *yes) ;;
        *) NEED_SETLOCALE_IMPROVED=1 ;;
      esac
      ;;
  esac
  AC_DEFINE_UNQUOTED([NEED_SETLOCALE_IMPROVED], [$NEED_SETLOCALE_IMPROVED],
    [Define to 1 to enable general improvements of setlocale.])

  dnl Test whether we need a multithread-safe setlocale(category,NULL).
  NEED_SETLOCALE_MTSAFE=0
  if test $SETLOCALE_NULL_ALL_MTSAFE = 0 || test $SETLOCALE_NULL_ONE_MTSAFE = 0; then
    NEED_SETLOCALE_MTSAFE=1
  fi
  AC_DEFINE_UNQUOTED([NEED_SETLOCALE_MTSAFE], [$NEED_SETLOCALE_MTSAFE],
    [Define to 1 to enable a multithread-safety fix of setlocale.])

  if test $NEED_SETLOCALE_IMPROVED = 1 || test $NEED_SETLOCALE_MTSAFE = 1; then
    REPLACE_SETLOCALE=1
  fi

  if test $NEED_SETLOCALE_MTSAFE = 1; then
    SETLOCALE_LIB="$SETLOCALE_NULL_LIB"
  else
    SETLOCALE_LIB=
  fi
  dnl SETLOCALE_LIB is expected to be '-pthread' or '-lpthread' on AIX with gcc
  dnl or xlc, and empty otherwise.
  AC_SUBST([SETLOCALE_LIB])
  dnl For backward compatibility.
  LIB_SETLOCALE="$SETLOCALE_LIB"
  AC_SUBST([LIB_SETLOCALE])
])

# Prerequisites of lib/setlocale.c.
AC_DEFUN([gl_PREREQ_SETLOCALE],
[
  dnl No need to check for CFLocaleCopyPreferredLanguages and
  dnl CFPreferencesCopyAppValue because lib/setlocale.c is not used on Mac OS X.
  dnl (The Mac OS X specific code is only used in libintl.)
  :
])
