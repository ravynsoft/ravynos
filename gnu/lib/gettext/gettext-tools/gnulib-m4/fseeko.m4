# fseeko.m4 serial 20
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FSEEKO],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_REQUIRE([gl_STDIN_LARGE_OFFSET])
  AC_REQUIRE([gl_SYS_TYPES_H])
  AC_REQUIRE([AC_PROG_CC])

  dnl Persuade glibc <stdio.h> to declare fseeko().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  AC_CACHE_CHECK([for fseeko], [gl_cv_func_fseeko],
    [
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdio.h>
]], [[fseeko (stdin, 0, 0);]])],
        [gl_cv_func_fseeko=yes], [gl_cv_func_fseeko=no])
    ])

  AC_CHECK_DECLS_ONCE([fseeko])
  if test $ac_cv_have_decl_fseeko = no; then
    HAVE_DECL_FSEEKO=0
  fi

  if test $gl_cv_func_fseeko = no; then
    HAVE_FSEEKO=0
  else
    if test $WINDOWS_64_BIT_OFF_T = 1; then
      REPLACE_FSEEKO=1
    fi
    if test $gl_cv_var_stdin_large_offset = no; then
      REPLACE_FSEEKO=1
    fi
    m4_ifdef([gl_FUNC_FFLUSH_STDIN], [
      gl_FUNC_FFLUSH_STDIN
      case "$gl_cv_func_fflush_stdin" in
        *yes) ;;
        *) REPLACE_FSEEKO=1 ;;
      esac
    ])
  fi
])

dnl Code shared by fseeko and ftello.  Determine if large files are supported,
dnl but stdin does not start as a large file by default.
AC_DEFUN([gl_STDIN_LARGE_OFFSET],
  [
    AC_CACHE_CHECK([whether stdin defaults to large file offsets],
      [gl_cv_var_stdin_large_offset],
      [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdio.h>]],
[[#if defined __SL64 && defined __SCLE /* cygwin */
  /* Cygwin 1.5.24 and earlier fail to put stdin in 64-bit mode, making
     fseeko/ftello needlessly fail.  This bug was fixed in 1.5.25, and
     it is easier to do a version check than building a runtime test.  */
# include <cygwin/version.h>
# if CYGWIN_VERSION_DLL_COMBINED < CYGWIN_VERSION_DLL_MAKE_COMBINED (1005, 25)
  choke me
# endif
#endif]])],
        [gl_cv_var_stdin_large_offset=yes],
        [gl_cv_var_stdin_large_offset=no])])
])

# Prerequisites of lib/fseeko.c.
AC_DEFUN([gl_PREREQ_FSEEKO],
[
  dnl Native Windows has the function _fseeki64. mingw hides it in some
  dnl circumstances, but mingw64 makes it usable again.
  AC_CHECK_FUNCS([_fseeki64])
  if test $ac_cv_func__fseeki64 = yes; then
    AC_CHECK_DECLS([_fseeki64])
  fi
])
