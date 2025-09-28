# lseek.m4 serial 15
dnl Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_LSEEK],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])

  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([AC_PROG_CC])
  AC_CHECK_HEADERS_ONCE([unistd.h])
  AC_CACHE_CHECK([whether lseek detects pipes], [gl_cv_func_lseek_pipe],
    [case "$host_os" in
       mingw* | windows*)
         dnl Native Windows.
         dnl The result of lseek (fd, (off_t)0, SEEK_CUR) or
         dnl SetFilePointer(handle, 0, NULL, FILE_CURRENT)
         dnl for a pipe depends on the environment:
         dnl In a Cygwin 1.5 environment it succeeds (wrong);
         dnl in a Cygwin 1.7 environment it fails with a wrong errno value;
         dnl in a Cygwin 2.9.0 environment it fails correctly;
         dnl in a Cygwin 3.4.6 environment it succeeds again (wrong).
         gl_cv_func_lseek_pipe=no
         ;;
       *)
         if test $cross_compiling = no; then
           AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h> /* for off_t */
#include <stdio.h> /* for SEEK_CUR */
#if HAVE_UNISTD_H
# include <unistd.h>
#else /* on Windows with MSVC */
# include <io.h>
#endif
]GL_MDA_DEFINES],
[[
  /* Exit with success only if stdin is seekable.  */
  return lseek (0, (off_t)0, SEEK_CUR) < 0;
]])],
             [if test -s conftest$ac_exeext \
                 && ./conftest$ac_exeext < conftest.$ac_ext \
                 && test 1 = "`echo hi \
                   | { ./conftest$ac_exeext; echo $?; cat >/dev/null; }`"; then
                gl_cv_func_lseek_pipe=yes
              else
                gl_cv_func_lseek_pipe=no
              fi
             ],
             [gl_cv_func_lseek_pipe=no])
         else
           AC_COMPILE_IFELSE(
             [AC_LANG_SOURCE([[
#if defined __BEOS__
/* BeOS mistakenly return 0 when trying to seek on pipes.  */
  Choke me.
#endif]])],
             [gl_cv_func_lseek_pipe=yes], [gl_cv_func_lseek_pipe=no])
         fi
         ;;
     esac
    ])
  if test "$gl_cv_func_lseek_pipe" = no; then
    REPLACE_LSEEK=1
    AC_DEFINE([LSEEK_PIPE_BROKEN], [1],
      [Define to 1 if lseek does not detect pipes.])
  fi

  AC_REQUIRE([gl_SYS_TYPES_H])
  if test $WINDOWS_64_BIT_OFF_T = 1; then
    REPLACE_LSEEK=1
  fi

  AS_IF([test $REPLACE_LSEEK = 0],
    [AC_CACHE_CHECK([whether SEEK_DATA works but is incompatible with GNU],
       [gl_cv_func_lseek_works_but_incompatible],
       [AC_PREPROC_IFELSE(
          [AC_LANG_SOURCE(
             dnl Use macOS "9999" to stand for a future fixed macOS version.
             dnl See ../lib/unistd.in.h and <https://bugs.gnu.org/61386>.
             [[#include <unistd.h>
               #if defined __APPLE__ && defined __MACH__ && defined SEEK_DATA
               # ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
               #  include <AvailabilityMacros.h>
               # endif
               # if 99990000 <= MAC_OS_X_VERSION_MIN_REQUIRED
               #  define LSEEK_WORKS_BUT_IS_INCOMPATIBLE_WITH_GNU
               # endif
               #endif
               #ifndef LSEEK_WORKS_BUT_IS_INCOMPATIBLE_WITH_GNU
                #error "No need to work around the bug"
               #endif
             ]])],
          [gl_cv_func_lseek_works_but_incompatible=yes],
          [gl_cv_func_lseek_works_but_incompatible=no])])
     if test "$gl_cv_func_lseek_works_but_incompatible" = yes; then
       REPLACE_LSEEK=1
     fi])
])
