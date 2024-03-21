# libunistring.m4 serial 12
dnl Copyright (C) 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_PREREQ([2.64])

dnl gl_LIBUNISTRING
dnl Searches for an installed libunistring.
dnl If found, it sets and AC_SUBSTs HAVE_LIBUNISTRING=yes and the LIBUNISTRING
dnl and LTLIBUNISTRING variables, sets the LIBUNISTRING_VERSION variable, and
dnl augments the CPPFLAGS variable, and #defines HAVE_LIBUNISTRING to 1.
dnl Otherwise, it sets and AC_SUBSTs HAVE_LIBUNISTRING=no and LIBUNISTRING and
dnl LTLIBUNISTRING to empty.

dnl Define gl_LIBUNISTRING using AC_DEFUN_ONCE, in order dnl to avoid warnings
dnl like
dnl "warning: AC_REQUIRE: `gl_LIBUNISTRING' was expanded before it was required".
dnl This is tricky because of the way 'aclocal' is implemented:
dnl - It requires defining an auxiliary macro whose name ends in AC_DEFUN.
dnl   Otherwise aclocal's initial scan pass would miss the macro definition.
dnl - It requires a line break inside the AC_DEFUN_ONCE and AC_DEFUN expansions.
dnl   Otherwise aclocal would emit many "Use of uninitialized value $1"
dnl   warnings.
AC_DEFUN_ONCE([gl_LIBUNISTRING],
[
  AC_BEFORE([$0], [gl_LIBUNISTRING_MODULE])
  AC_BEFORE([$0], [gl_LIBUNISTRING_LIBHEADER])
  AC_BEFORE([$0], [gl_LIBUNISTRING_LIB_PREPARE])

  m4_ifdef([gl_LIBUNISTRING_OPTIONAL],
    [
      AC_MSG_CHECKING([whether included libunistring is requested])
      AC_ARG_WITH([included-libunistring],
        [  --with-included-libunistring  use the libunistring parts included here],
        [gl_libunistring_force_included=$withval],
        [gl_libunistring_force_included=no])
      AC_MSG_RESULT([$gl_libunistring_force_included])
      gl_libunistring_use_included="$gl_libunistring_force_included"
      if test "$gl_libunistring_use_included" = yes; then
        dnl Assume that libunistring is not installed until some other macro
        dnl explicitly invokes gl_LIBUNISTRING_CORE.
        if test -z "$HAVE_LIBUNISTRING"; then
          HAVE_LIBUNISTRING=no
        fi
        LIBUNISTRING=
        LTLIBUNISTRING=
      else
        gl_LIBUNISTRING_CORE
        if test $HAVE_LIBUNISTRING = no; then
          gl_libunistring_use_included=yes
          LIBUNISTRING=
          LTLIBUNISTRING=
        fi
      fi
    ],
    [gl_LIBUNISTRING_CORE])
])

AC_DEFUN([gl_LIBUNISTRING_CORE],
[
  AC_REQUIRE([AM_ICONV])
  if test -n "$LIBICONV"; then
    dnl First, try to link without -liconv. libunistring often depends on
    dnl libiconv, but we don't know (and often don't need to know) where
    dnl libiconv is installed.
    AC_LIB_HAVE_LINKFLAGS([unistring], [],
      [#include <uniconv.h>], [u8_strconv_from_locale((char*)0);],
      [no, trying again together with libiconv])
    if test "$ac_cv_libunistring" != yes; then
      dnl Second try, with -liconv.
      dnl We have to erase the cached result of the first AC_LIB_HAVE_LINKFLAGS
      dnl invocation, otherwise the second one will not be run.
      unset ac_cv_libunistring
      glus_save_LIBS="$LIBS"
      LIBS="$LIBS $LIBICONV"
      AC_LIB_HAVE_LINKFLAGS([unistring], [],
        [#include <uniconv.h>], [u8_strconv_from_locale((char*)0);],
        [no, consider installing GNU libunistring])
      if test -n "$LIBUNISTRING"; then
        LIBUNISTRING="$LIBUNISTRING $LIBICONV"
        LTLIBUNISTRING="$LTLIBUNISTRING $LTLIBICONV"
      fi
      LIBS="$glus_save_LIBS"
    fi
  else
    AC_LIB_HAVE_LINKFLAGS([unistring], [],
      [#include <uniconv.h>], [u8_strconv_from_locale((char*)0);],
      [no, consider installing GNU libunistring])
  fi
  if test $HAVE_LIBUNISTRING = yes; then
    dnl Determine the installed version.
    AC_CACHE_CHECK([for libunistring version], [gl_cv_libunistring_version],
      [AC_COMPUTE_INT([gl_libunistring_hexversion],
                      [_LIBUNISTRING_VERSION],
                      [#include <unistring/version.h>])
       dnl Versions <= 0.9.3 had a hexversion of 0x0009.
       dnl Use other tests to distinguish them.
       if test $gl_libunistring_hexversion = 9; then
         dnl Version 0.9.2 introduced the header <unistring/cdefs.h>.
         AC_COMPILE_IFELSE(
           [AC_LANG_PROGRAM([[#include <unistring/cdefs.h>]], [[]])],
           [gl_cv_libunistring_version092=true],
           [gl_cv_libunistring_version092=false])
         if $gl_cv_libunistring_version092; then
           dnl Version 0.9.3 changed a comment in <unistr.h>.
           gl_ABSOLUTE_HEADER_ONE([unistr.h])
           if test -n "$gl_cv_absolute_unistr_h" \
              && grep 'Copy no more than N units of SRC to DEST.  Return a pointer' $gl_cv_absolute_unistr_h > /dev/null; then
             dnl Detected version 0.9.3.
             gl_libunistring_hexversion=2307
           else
             dnl Detected version 0.9.2.
             gl_libunistring_hexversion=2306
           fi
         else
           dnl Version 0.9.1 introduced the type casing_suffix_context_t.
           AC_COMPILE_IFELSE(
             [AC_LANG_PROGRAM(
                [[#include <unicase.h>
                  casing_suffix_context_t ct;]],
                [[]])],
             [gl_cv_libunistring_version091=true],
             [gl_cv_libunistring_version091=false])
           if $gl_cv_libunistring_version091; then
             dnl Detected version 0.9.1.
             gl_libunistring_hexversion=2305
           else
             dnl Detected version 0.9.
             gl_libunistring_hexversion=2304
           fi
         fi
       fi
       dnl Transform into the usual major.minor.subminor notation.
       gl_libunistring_major=`expr $gl_libunistring_hexversion / 65536`
       gl_libunistring_minor=`expr $gl_libunistring_hexversion / 256 % 256`
       gl_libunistring_subminor=`expr $gl_libunistring_hexversion % 256`
       gl_cv_libunistring_version="$gl_libunistring_major.$gl_libunistring_minor.$gl_libunistring_subminor"
      ])
    LIBUNISTRING_VERSION="$gl_cv_libunistring_version"
  fi
])
