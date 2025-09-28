# iconv_open.m4 serial 16
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_ICONV_OPEN],
[
  AC_REQUIRE([AM_ICONV])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([gl_ICONV_H_DEFAULTS])
  if test "$am_cv_func_iconv" = yes; then
    dnl Provide the <iconv.h> override, for the sake of the C++ aliases.
    gl_REPLACE_ICONV_H
    dnl Test whether iconv_open accepts standardized encoding names.
    dnl We know that GNU libiconv and GNU libc do.
    AC_EGREP_CPP([gnu_iconv], [
      #include <iconv.h>
      #if defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__)
       gnu_iconv
      #endif
      ], [gl_func_iconv_gnu=yes], [gl_func_iconv_gnu=no])
    if test $gl_func_iconv_gnu = no; then
      iconv_flavor=
      case "$host_os" in
        aix*)         iconv_flavor=ICONV_FLAVOR_AIX ;;
        irix*)        iconv_flavor=ICONV_FLAVOR_IRIX ;;
        hpux*)        iconv_flavor=ICONV_FLAVOR_HPUX ;;
        osf*)         iconv_flavor=ICONV_FLAVOR_OSF ;;
        solaris*)     iconv_flavor=ICONV_FLAVOR_SOLARIS ;;
        openedition*) iconv_flavor=ICONV_FLAVOR_ZOS ;;
      esac
      if test -n "$iconv_flavor"; then
        AC_DEFINE_UNQUOTED([ICONV_FLAVOR], [$iconv_flavor],
          [Define to a symbolic name denoting the flavor of iconv_open()
           implementation.])
        gl_REPLACE_ICONV_OPEN
      fi
    fi
    m4_ifdef([gl_FUNC_ICONV_OPEN_UTF_SUPPORT], [
      gl_FUNC_ICONV_OPEN_UTF_SUPPORT
      case "$gl_cv_func_iconv_supports_utf" in
        *yes) ;;
        *)
          REPLACE_ICONV_UTF=1
          AC_DEFINE([REPLACE_ICONV_UTF], [1],
            [Define if the iconv() functions are enhanced to handle the UTF-{16,32}{BE,LE} encodings.])
          REPLACE_ICONV=1
          gl_REPLACE_ICONV_OPEN
          ;;
      esac
    ])
  fi
])

AC_DEFUN([gl_REPLACE_ICONV_OPEN],
[
  gl_REPLACE_ICONV_H
  REPLACE_ICONV_OPEN=1
])
