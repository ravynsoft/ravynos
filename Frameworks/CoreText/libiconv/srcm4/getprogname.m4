# getprogname.m4
# serial 9
dnl Copyright (C) 2016-2026 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

# Check for getprogname or replacements for it

AC_DEFUN([gl_FUNC_GETPROGNAME],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_CHECK_HEADERS_ONCE([sys/process.h])
  if test $ac_cv_header_sys_process_h = yes; then
    HAVE_SYS_PROCESS_H=1
  else
    HAVE_SYS_PROCESS_H=0
  fi
  gl_CHECK_FUNCS_ANDROID([getprogname], [[#include <stdlib.h>]])
  if test $ac_cv_func_getprogname = no; then
    HAVE_GETPROGNAME=0
    case "$gl_cv_onwards_func_getprogname" in
      future*) REPLACE_GETPROGNAME=1 ;;
    esac
  fi
  AC_CHECK_DECLS([program_invocation_name],
    [],
    [HAVE_DECL_PROGRAM_INVOCATION_NAME=0],
    [[#include <errno.h>]])
])

AC_DEFUN([gl_PREREQ_GETPROGNAME],
[
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_CHECK_FUNCS_ONCE([getexecname])
  ac_found=0
  AC_CHECK_DECLS([program_invocation_name], [ac_found=1], [],
    [#include <errno.h>])
  AC_CHECK_DECLS([program_invocation_short_name], [ac_found=1], [],
    [#include <errno.h>])
  AC_CHECK_DECLS([__argv], [ac_found=1], [], [#include <stdlib.h>])

  # Incur the cost of this test only if none of the above worked.
  if test $ac_found = 0; then
    # On OpenBSD 5.1, using the global __progname variable appears to be
    # the only way to implement getprogname.
    AC_CACHE_CHECK([whether __progname is defined in default libraries],
      [gl_cv_var___progname],
      [
        gl_cv_var___progname=
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM(
            [[extern char *__progname;]],
            [[return *__progname;]]
          )],
          [gl_cv_var___progname=yes]
        )
      ]
    )
    if test "$gl_cv_var___progname" = yes; then
      AC_DEFINE([HAVE_VAR___PROGNAME], 1,
        [Define if you have a global __progname variable])
    fi
  fi
])
