# random.m4 serial 8
dnl Copyright (C) 2012-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_RANDOM],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])

  dnl We can't use AC_CHECK_FUNC here, because random() is defined as a
  dnl static inline function when compiling for Android 4.4 or older.
  AC_CACHE_CHECK([for random], [gl_cv_func_random],
    [AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <stdlib.h>]],
          [[return random() == 0;]])
       ],
       [gl_cv_func_random=yes],
       [gl_cv_func_random=no])
    ])
  gl_CHECK_FUNCS_ANDROID([initstate], [[#include <stdlib.h>]])
  gl_CHECK_FUNCS_ANDROID([setstate], [[#include <stdlib.h>]])
  if test $gl_cv_func_random = no; then
    HAVE_RANDOM=0
    HAVE_INITSTATE=0
    HAVE_SETSTATE=0
  else
    if test $ac_cv_func_initstate = no; then
      HAVE_INITSTATE=0
    fi
    if test $ac_cv_func_setstate = no; then
      HAVE_SETSTATE=0
    fi
  fi
  if test $HAVE_INITSTATE = 0; then
    case "$gl_cv_onwards_func_initstate" in
      future*) REPLACE_INITSTATE=1 ;;
    esac
  fi
  if test $HAVE_SETSTATE = 0; then
    case "$gl_cv_onwards_func_setstate" in
      future*) REPLACE_SETSTATE=1 ;;
    esac
  fi
  dnl On several platforms, random() is not multithread-safe.
  if test $ac_cv_func_initstate = no || test $ac_cv_func_setstate = no \
     || case "$host_os" in \
          darwin* | freebsd* | solaris* | cygwin* | haiku*) true ;; \
          *) false ;; \
        esac
  then
    dnl In order to define initstate or setstate, we need to define all the
    dnl functions at once.
    REPLACE_RANDOM=1
    if test $ac_cv_func_initstate = yes; then
      REPLACE_INITSTATE=1
    fi
    if test $ac_cv_func_setstate = yes; then
      REPLACE_SETSTATE=1
    fi
  fi

  AC_CHECK_DECLS_ONCE([initstate])
  if test $ac_cv_have_decl_initstate = no; then
    HAVE_DECL_INITSTATE=0
  fi

  AC_CHECK_DECLS_ONCE([setstate])
  if test $ac_cv_have_decl_setstate = no; then
    HAVE_DECL_SETSTATE=0
  fi
])

# Prerequisites of lib/random.c.
AC_DEFUN([gl_PREREQ_RANDOM], [
  :
])
