# serial 5
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_RANDOM_R],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])

  AC_CHECK_HEADERS([random.h], [], [], [AC_INCLUDES_DEFAULT])
  if test $ac_cv_header_random_h = no; then
    HAVE_RANDOM_H=0
  fi

  AC_CHECK_TYPES([struct random_data],
    [], [HAVE_STRUCT_RANDOM_DATA=0],
    [[#include <stdlib.h>
      #if HAVE_RANDOM_H
      # include <random.h>
      #endif
    ]])

  dnl On AIX and OSF/1, these functions exist, but with different declarations.
  dnl Override them all.
  case "$host_os" in
    aix* | osf*)
      REPLACE_RANDOM_R=1
      ;;
    *)
      AC_CHECK_FUNCS([random_r])
      if test $ac_cv_func_random_r = no; then
        HAVE_RANDOM_R=0
      fi
      ;;
  esac
])

# Prerequisites of lib/random_r.c.
AC_DEFUN([gl_PREREQ_RANDOM_R], [
  :
])
