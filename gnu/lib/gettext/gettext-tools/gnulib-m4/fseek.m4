# fseek.m4 serial 4
dnl Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FSEEK],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_FSEEKO])
  dnl When fseeko needs fixes, fseek needs them too.
  if test $HAVE_FSEEKO = 0 || test $REPLACE_FSEEKO = 1; then
    REPLACE_FSEEK=1
  fi
])
