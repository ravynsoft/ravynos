# ftell.m4 serial 3
dnl Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FTELL],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_FTELLO])
  dnl When ftello needs fixes, ftell needs them too.
  if test $HAVE_FTELLO = 0 || test $REPLACE_FTELLO = 1; then
    REPLACE_FTELL=1
  fi
])
