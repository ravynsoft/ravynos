# memrchr.m4 serial 11
dnl Copyright (C) 2002-2003, 2005-2007, 2009-2023 Free Software Foundation,
dnl Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_MEMRCHR],
[
  dnl Persuade glibc <string.h> to declare memrchr().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  AC_REQUIRE([gl_STRING_H_DEFAULTS])
  AC_CHECK_DECLS_ONCE([memrchr])
  if test $ac_cv_have_decl_memrchr = no; then
    HAVE_DECL_MEMRCHR=0
  fi

  AC_CHECK_FUNCS([memrchr])
])

# Prerequisites of lib/memrchr.c.
AC_DEFUN([gl_PREREQ_MEMRCHR], [:])
