# pipe.m4 serial 2
dnl Copyright (C) 2010-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_PIPE],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])

  AC_CHECK_FUNCS_ONCE([pipe])
  if test $ac_cv_func_pipe != yes; then
    HAVE_PIPE=0
  fi
])
