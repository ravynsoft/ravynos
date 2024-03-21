# libtextstyle.m4 serial 3
dnl Copyright (C) 2019-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

dnl gl_LIBTEXTSTYLE([MINIMUM-VERSION])
dnl Searches for an installed libtextstyle with version >= MINIMUM-VERSION.
dnl   MINIMUM-VERSION = 0.20      - the first release in 2019
dnl   MINIMUM-VERSION = 0.20.5    - adds hyperlink support and ostream_printf
dnl   MINIMUM-VERSION unspecified - the newest release
dnl If found, it sets and AC_SUBSTs HAVE_LIBTEXTSTYLE=yes and the LIBTEXTSTYLE
dnl and LTLIBTEXTSTYLE variables, and augments the CPPFLAGS variable, and
dnl #defines HAVE_LIBTEXTSTYLE to 1.
dnl Otherwise, it sets and AC_SUBSTs HAVE_LIBTEXTSTYLE=no and LIBTEXTSTYLE and
dnl LTLIBTEXTSTYLE to empty.

AC_DEFUN([gl_LIBTEXTSTYLE],
[
  AC_REQUIRE([gl_LIBTEXTSTYLE_INITIALIZE])
  AC_REQUIRE([gl_LIBTEXTSTYLE_SEARCH])
  pushdef([MINVERSION], m4_if([$1], [], [gl_LIBTEXTSTYLE_NEWEST_VERSION], [$1]))
  dnl Signal a fatal error if MINVERSION is not among the allowed values.
  m4_if(m4_if(MINVERSION, [0.20], [x], [])m4_if(MINVERSION, [0.20.5], [x], []), [],
    [m4_fatal([The argument to gl_LIBTEXTSTYLE or gl_LIBTEXTSTYLE_OPTIONAL is not one of the expected values.])])
  dnl Store the specified minimum version in gl_libtextstyle_minversion.
  dnl (This needs to be outside the m4_if. m4_divert_text inside m4_if does
  dnl not work reliably in Autoconf 2.69.)
  m4_divert_text([INIT_PREPARE],
    [gl_libtextstyle_minversion="$gl_libtextstyle_minversion MINVERSION "])
  popdef([MINVERSION])
])

AC_DEFUN([gl_LIBTEXTSTYLE_NEWEST_VERSION], [0.20.5])

AC_DEFUN([gl_LIBTEXTSTYLE_INITIALIZE],
[
  m4_divert_text([DEFAULTS], [gl_libtextstyle_minversion=' 0.20 '])
])

AC_DEFUN([gl_LIBTEXTSTYLE_SEARCH],
[
  dnl $gl_libtextstyle_minversion evaluates to a space-separated list of
  dnl specified minimum versions. The maximum of these requirement matters.
  case "$gl_libtextstyle_minversion" in
    *" 0.20.5 "*)
      snippet='styled_ostream_t s = term_styled_ostream_create(1,"",TTYCTL_AUTO,"");
               ostream_printf(s,"%d",42);'
      ;;
    *" 0.20 "*)
      snippet='term_styled_ostream_create(1,"",TTYCTL_AUTO,"");'
      ;;
  esac
  AC_LIB_HAVE_LINKFLAGS([textstyle], [],
    [#include <textstyle.h>], [$snippet],
    [no])
])
