# libdl.m4
# serial 3
dnl Copyright (C) 2024-2026 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

dnl From Bruno Haible.

dnl Set LIBDL to '-ldl' if it is needed to use the functions declared
dnl in <dlfcn.h> (dlopen, dlsym, etc.), or to empty otherwise.
AC_DEFUN([gl_LIBDL],
[
  dnl dlopen, dlsym are
  dnl - in libc on glibc >= 2.34, musl libc, macOS, FreeBSD, NetBSD, OpenBSD,
  dnl   AIX, Solaris, Cygwin, Haiku,
  dnl - in a separate libdl on glibc < 2.34, uClibc, Android.
  AC_CACHE_CHECK([for library needed for dlopen and dlsym],
    [gl_cv_lib_dl],
    [AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <dlfcn.h>
          ]],
          [[return ! dlsym (RTLD_DEFAULT, "main");]])],
       [gl_cv_lib_dl=none],
       [gl_cv_lib_dl=maybe])
     if test $gl_cv_lib_dl = maybe; then
       saved_LIBS="$LIBS"
       LIBS="$LIBS -ldl"
       AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <dlfcn.h>
            ]],
            [[return ! dlsym (RTLD_DEFAULT, "main");]])],
         [gl_cv_lib_dl='-ldl'],
         [gl_cv_lib_dl=none])
       LIBS="$saved_LIBS"
     fi
    ])
  case "$gl_cv_lib_dl" in
    none) LIBDL='' ;;
    *)    LIBDL="$gl_cv_lib_dl" ;;
  esac
  AC_SUBST([LIBDL])
])
