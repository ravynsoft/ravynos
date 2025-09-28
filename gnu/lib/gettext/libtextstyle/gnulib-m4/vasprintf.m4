# vasprintf.m4 serial 6
dnl Copyright (C) 2002-2003, 2006-2007, 2009-2023 Free Software Foundation,
dnl Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_VASPRINTF],
[
  AC_CHECK_FUNCS([vasprintf])
  if test $ac_cv_func_vasprintf = no; then
    gl_REPLACE_VASPRINTF
  fi
])

AC_DEFUN([gl_REPLACE_VASPRINTF],
[
  AC_LIBOBJ([vasprintf])
  AC_LIBOBJ([asprintf])
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  if test $ac_cv_func_vasprintf = yes; then
    REPLACE_VASPRINTF=1
  else
    HAVE_VASPRINTF=0
  fi
  gl_PREREQ_VASPRINTF_H
  gl_PREREQ_VASPRINTF
  gl_PREREQ_ASPRINTF
])

# Prerequisites of the vasprintf portion of lib/stdio.h.
AC_DEFUN([gl_PREREQ_VASPRINTF_H],
[
  dnl Persuade glibc <stdio.h> to declare asprintf() and vasprintf().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])
])

# Prerequisites of lib/vasprintf.c.
AC_DEFUN([gl_PREREQ_VASPRINTF],
[
])

# Prerequisites of lib/asprintf.c.
AC_DEFUN([gl_PREREQ_ASPRINTF],
[
])
