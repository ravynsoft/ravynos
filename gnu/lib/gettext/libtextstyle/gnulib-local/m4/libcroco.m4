# libcroco.m4 serial 4
dnl Copyright (C) 2006-2007, 2019-2020 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

dnl gl_LIBCROCO
dnl   gives the user the option to decide whether to use the included or
dnl   an external libcroco.
dnl gl_LIBCROCO(FORCE-INCLUDED)
dnl   forces the use of the included or an external libcroco.
AC_DEFUN([gl_LIBCROCO],
[
  ifelse([$1], [yes], , [
    dnl libcroco depends on libglib.
    AC_REQUIRE([gl_LIBGLIB])
  ])

  ifelse([$1], , [
    AC_MSG_CHECKING([whether included libcroco is requested])
    AC_ARG_WITH([included-libcroco],
      [  --with-included-libcroco  use the libcroco included here],
      [gl_cv_libcroco_force_included=$withval],
      [gl_cv_libcroco_force_included=no])
    AC_MSG_RESULT([$gl_cv_libcroco_force_included])
  ], [gl_cv_libcroco_force_included=$1])

  gl_cv_libcroco_use_included="$gl_cv_libcroco_force_included"
  LIBCROCO=
  LTLIBCROCO=
  INCCROCO=
  ifelse([$1], [yes], , [
    if test "$gl_cv_libcroco_use_included" != yes; then
      dnl Figure out whether we can use a preinstalled libcroco-0.6, or have to
      dnl use the included one.
      AC_CACHE_VAL([gl_cv_libcroco], [
        gl_cv_libcroco=no
        gl_cv_LIBCROCO=
        gl_cv_LTLIBCROCO=
        gl_cv_INCCROCO=
        gl_save_LIBS="$LIBS"
        dnl Search for libcroco and define LIBCROCO_0_6, LTLIBCROCO_0_6 and
        dnl INCCROCO_0_6 accordingly.
        dnl Don't use croco-0.6-config nor pkg-config, since it doesn't work when
        dnl cross-compiling or when the C compiler in use is different from the
        dnl one that built the library.
        AC_LIB_LINKFLAGS_BODY([croco-0.6], [glib-2.0])
        LIBS="$gl_save_LIBS $LIBCROCO_0_6"
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM([[#include <libcroco-config.h>]],
            [[const char *version = LIBCROCO_VERSION; return !version;]])],
          [gl_cv_libcroco=yes
           gl_cv_LIBCROCO="$LIBCROCO_0_6"
           gl_cv_LTLIBCROCO="$LTLIBCROCO_0_6"
          ])
        if test "$gl_cv_libcroco" != yes; then
          gl_save_CPPFLAGS="$CPPFLAGS"
          CPPFLAGS="$CPPFLAGS $INCCROCO_0_6"
          AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <libcroco-config.h>]],
              [[const char *version = LIBCROCO_VERSION; return !version;]])],
            [gl_cv_libcroco=yes
             gl_cv_LIBCROCO="$LIBCROCO_0_6"
             gl_cv_LTLIBCROCO="$LTLIBCROCO_0_6"
             gl_cv_INCCROCO="$INCCROCO_0_6"
            ])
          if test "$gl_cv_libcroco" != yes; then
            dnl Often the include files are installed in
            dnl /usr/include/libcroco-0.6/libcroco.
            AC_LINK_IFELSE(
              [AC_LANG_PROGRAM([[#include <libcroco-config.h>]],
                [[const char *version = LIBCROCO_VERSION; return !version;]])],
              [gl_ABSOLUTE_HEADER([libcroco-0.6/libcroco/libcroco-config.h])
               libcroco_include_dir=`echo "$gl_cv_absolute_libcroco_0_6_libcroco_libcroco_config_h" | sed -e 's,.libcroco-config\.h$,,'`
               if test -d "$libcroco_include_dir"; then
                 gl_cv_libcroco=yes
                 gl_cv_LIBCROCO="$LIBCROCO_0_6"
                 gl_cv_LTLIBCROCO="$LTLIBCROCO_0_6"
                 gl_cv_INCCROCO="-I$libcroco_include_dir"
               fi
              ])
          fi
          CPPFLAGS="$gl_save_CPPFLAGS"
        fi
        LIBS="$gl_save_LIBS"
      ])
      AC_MSG_CHECKING([for libcroco])
      AC_MSG_RESULT([$gl_cv_libcroco])
      if test $gl_cv_libcroco = yes; then
        LIBCROCO="$gl_cv_LIBCROCO"
        LTLIBCROCO="$gl_cv_LTLIBCROCO"
        INCCROCO="$gl_cv_INCCROCO"
      else
        gl_cv_libcroco_use_included=yes
      fi
    fi
  ])
  AC_SUBST([LIBCROCO])
  AC_SUBST([LTLIBCROCO])
  AC_SUBST([INCCROCO])
  AC_MSG_CHECKING([whether to use the included libcroco])
  AC_MSG_RESULT([$gl_cv_libcroco_use_included])

  AM_CONDITIONAL([INCLUDED_LIBCROCO],
    [test "$gl_cv_libcroco_use_included" = yes])
])
