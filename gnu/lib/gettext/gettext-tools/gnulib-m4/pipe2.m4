# pipe2.m4 serial 4
dnl Copyright (C) 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_PIPE2],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])

  dnl Persuade glibc <unistd.h> to declare pipe2().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  gl_CHECK_FUNCS_ANDROID([pipe2], [[#include <unistd.h>]])
  if test $ac_cv_func_pipe2 != yes; then
    HAVE_PIPE2=0
    case "$gl_cv_onwards_func_pipe2" in
      future*) REPLACE_PIPE2=1 ;;
    esac
  else
    REPLACE_PIPE2=1
  fi
])
