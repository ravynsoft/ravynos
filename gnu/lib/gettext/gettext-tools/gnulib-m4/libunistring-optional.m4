# libunistring-optional.m4 serial 1
dnl Copyright (C) 2010-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl gl_LIBUNISTRING_OPTIONAL
dnl Searches for an installed libunistring or uses the included source code
dnl parts.
dnl If found, it sets and AC_SUBSTs HAVE_LIBUNISTRING=yes and the LIBUNISTRING
dnl and LTLIBUNISTRING variables and augments the CPPFLAGS variable, and
dnl #defines HAVE_LIBUNISTRING to 1. Otherwise, it sets and AC_SUBSTs
dnl HAVE_LIBUNISTRING=no and LIBUNISTRING and LTLIBUNISTRING to empty.

AC_DEFUN([gl_LIBUNISTRING_OPTIONAL],
[
  dnl gl_LIBUNISTRING does a couple of extra things if this macro is used.
  AC_REQUIRE([gl_LIBUNISTRING])

  AC_MSG_CHECKING([whether to use the included libunistring])
  AC_MSG_RESULT([$gl_libunistring_use_included])
])
