# serial 24

# See if we need to emulate a missing ftruncate function using _chsize.

# Copyright (C) 2000-2001, 2003-2007, 2009-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FTRUNCATE],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  gl_CHECK_FUNCS_ANDROID([ftruncate], [[#include <unistd.h>]])
  if test $ac_cv_func_ftruncate = yes; then
    m4_ifdef([gl_LARGEFILE], [
      AC_REQUIRE([AC_CANONICAL_HOST])
      case "$host_os" in
        mingw* | windows*)
          dnl Native Windows, and Large File Support is requested.
          dnl The MSVCRT _chsize() function only accepts a 32-bit file size,
          dnl and the mingw64 ftruncate64() function is unreliable (it may
          dnl delete the file, see
          dnl <https://web.archive.org/web/20160425005423/http://mingw-w64.sourcearchive.com/documentation/2.0-1/ftruncate64_8c_source.html>).
          dnl Use gnulib's ftruncate() implementation instead.
          REPLACE_FTRUNCATE=1
          ;;
      esac
    ], [
      :
    ])
  else
    HAVE_FTRUNCATE=0
    case "$gl_cv_onwards_func_ftruncate" in
      future*) REPLACE_FTRUNCATE=1 ;;
    esac
  fi
])

# Prerequisites of lib/ftruncate.c.
AC_DEFUN([gl_PREREQ_FTRUNCATE],
[
  AC_CHECK_FUNCS([_chsize])
])
