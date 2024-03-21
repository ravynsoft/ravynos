# libglib.m4 serial 7
dnl Copyright (C) 2006-2007, 2019-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

dnl gl_LIBGLIB
dnl   gives the user the option to decide whether to use the included or
dnl   an external libglib.
dnl gl_LIBGLIB(FORCE-INCLUDED)
dnl   forces the use of the included or an external libglib.
AC_DEFUN([gl_LIBGLIB],
[
  ifelse([$1], , [
    AC_MSG_CHECKING([whether included glib is requested])
    AC_ARG_WITH([included-glib],
      [  --with-included-glib    use the glib2 included here],
      [gl_cv_libglib_force_included=$withval],
      [gl_cv_libglib_force_included=no])
    AC_MSG_RESULT([$gl_cv_libglib_force_included])
  ], [gl_cv_libglib_force_included=$1])

  gl_cv_libglib_use_included="$gl_cv_libglib_force_included"
  LIBGLIB=
  LTLIBGLIB=
  INCGLIB=
  ifelse([$1], [yes], , [
    if test "$gl_cv_libglib_use_included" != yes; then
      dnl Figure out whether we can use a preinstalled libglib-2.0, or have to use
      dnl the included one.
      AC_CACHE_VAL([gl_cv_libglib], [
        gl_cv_libglib=no
        gl_cv_LIBGLIB=
        gl_cv_LTLIBGLIB=
        gl_cv_INCGLIB=
        gl_save_LIBS="$LIBS"
        dnl Search for libglib2 and define LIBGLIB_2_0, LTLIBGLIB_2_0 and
        dnl INCGLIB_2_0 accordingly.
        dnl Don't use glib-config nor pkg-config, since it doesn't work when
        dnl cross-compiling or when the C compiler in use is different from the
        dnl one that built the library.
        AC_LIB_LINKFLAGS_BODY([glib-2.0])
        LIBS="$gl_save_LIBS $LIBGLIB_2_0"
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM(
             [[#include <glib.h>
               #ifndef G_BEGIN_DECLS
               error this glib.h includes a glibconfig.h from a glib version 1.x
               #endif
             ]],
             [[g_string_new ("foo");]])],
          [gl_cv_libglib=yes
           gl_cv_LIBGLIB="$LIBGLIB_2_0"
           gl_cv_LTLIBGLIB="$LTLIBGLIB_2_0"
          ])
        if test "$gl_cv_libglib" != yes; then
          gl_save_CPPFLAGS="$CPPFLAGS"
          CPPFLAGS="$CPPFLAGS $INCGLIB_2_0"
          AC_LINK_IFELSE(
            [AC_LANG_PROGRAM(
               [[#include <glib.h>
                 #ifndef G_BEGIN_DECLS
                 error this glib.h includes a glibconfig.h from a glib version 1.x
                 #endif
               ]],
               [[g_string_new ("foo");]])],
            [gl_cv_libglib=yes
             gl_cv_LIBGLIB="$LIBGLIB_2_0"
             gl_cv_LTLIBGLIB="$LTLIBGLIB_2_0"
             gl_cv_INCGLIB="$INCGLIB_2_0"
            ])
          if test "$gl_cv_libglib" != yes; then
            dnl Often the include files are installed in /usr/include/glib-2.0
            dnl and /usr/lib/glib-2.0/include.
            if test -n "$LIBGLIB_2_0_PREFIX"; then
              CPPFLAGS="$gl_save_CPPFLAGS -I$LIBGLIB_2_0_PREFIX/include/glib-2.0 -I$LIBGLIB_2_0_PREFIX/$acl_libdirstem/glib-2.0/include"
              AC_LINK_IFELSE(
                [AC_LANG_PROGRAM(
                   [[#include <glib.h>
                     #ifndef G_BEGIN_DECLS
                     error this glib.h includes a glibconfig.h from a glib version 1.x
                     #endif
                   ]],
                   [[g_string_new ("foo");]])],
                [gl_cv_libglib=yes
                 gl_cv_LIBGLIB="$LIBGLIB_2_0"
                 gl_cv_LTLIBGLIB="$LTLIBGLIB_2_0"
                 gl_cv_INCGLIB="-I$LIBGLIB_2_0_PREFIX/include/glib-2.0 -I$LIBGLIB_2_0_PREFIX/$acl_libdirstem/glib-2.0/include"
                ])
            fi
          fi
          CPPFLAGS="$gl_save_CPPFLAGS"
        fi
        LIBS="$gl_save_LIBS"
      ])
      AC_MSG_CHECKING([for glib])
      AC_MSG_RESULT([$gl_cv_libglib])
      if test $gl_cv_libglib = yes; then
        LIBGLIB="$gl_cv_LIBGLIB"
        LTLIBGLIB="$gl_cv_LTLIBGLIB"
        INCGLIB="$gl_cv_INCGLIB"
      else
        gl_cv_libglib_use_included=yes
      fi
    fi
  ])
  AC_SUBST([LIBGLIB])
  AC_SUBST([LTLIBGLIB])
  AC_SUBST([INCGLIB])
  AC_MSG_CHECKING([whether to use the included glib])
  AC_MSG_RESULT([$gl_cv_libglib_use_included])

  if test "$gl_cv_libglib_use_included" = yes; then
    LIBGLIB_H=
    LIBGLIB_H="$LIBGLIB_H glib.h"
    LIBGLIB_H="$LIBGLIB_H glibconfig.h"
    LIBGLIB_H="$LIBGLIB_H glib/ghash.h"
    LIBGLIB_H="$LIBGLIB_H glib/glist.h"
    LIBGLIB_H="$LIBGLIB_H glib/gmacros.h"
    LIBGLIB_H="$LIBGLIB_H glib/gprimes.h"
    LIBGLIB_H="$LIBGLIB_H glib/gprintfint.h"
    LIBGLIB_H="$LIBGLIB_H glib/gstrfuncs.h"
    LIBGLIB_H="$LIBGLIB_H glib/gstring.h"
    LIBGLIB_H="$LIBGLIB_H glib/gtypes.h"
    AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])
    AC_CHECK_HEADERS([unistd.h])
    dnl Don't bother checking for pthread.h and other multithread facilities.
    AC_CHECK_MEMBERS([struct lconv.decimal_point], [], [],
      [[#include <locale.h>]])
  else
    LIBGLIB_H=
  fi
  AC_SUBST([LIBGLIB_H])

  AM_CONDITIONAL([INCLUDED_LIBGLIB],
    [test "$gl_cv_libglib_use_included" = yes])
])
