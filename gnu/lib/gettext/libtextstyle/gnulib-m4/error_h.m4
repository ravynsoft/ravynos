# error_h.m4 serial 4
dnl Copyright (C) 1996-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.
dnl Provide a working "error.h".

AC_DEFUN_ONCE([gl_ERROR_H],
[
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles

  gl_CHECK_NEXT_HEADERS([error.h])
  if test $ac_cv_header_error_h = yes; then
    HAVE_ERROR_H=1
  else
    HAVE_ERROR_H=0
  fi
  AC_SUBST([HAVE_ERROR_H])

  REPLACE_ERROR=0

  gl_CHECK_FUNCS_ANDROID([error], [[#include <error.h>]])
  if test $ac_cv_func_error = yes; then
    HAVE_ERROR=1
  else
    HAVE_ERROR=0
    case "$gl_cv_onwards_func_error" in
      future*) REPLACE_ERROR=1 ;;
    esac
  fi

  dnl We don't use AC_FUNC_ERROR_AT_LINE any more, because it is no longer
  dnl maintained in Autoconf and because it invokes AC_LIBOBJ.
  dnl We need to notice a missing declaration, like gl_CHECK_FUNCS_ANDROID does.
  AC_CHECK_DECL([error_at_line], , , [[#include <error.h>]])
  if test $ac_cv_have_decl_error_at_line = yes; then
    AC_CACHE_CHECK([for error_at_line], [ac_cv_lib_error_at_line],
      [AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <error.h>]],
            [[error_at_line (0, 0, "", 0, "an error occurred");]])],
         [ac_cv_lib_error_at_line=yes],
         [ac_cv_lib_error_at_line=no])])
  else
    ac_cv_lib_error_at_line=no
  fi
  if test $ac_cv_lib_error_at_line = yes; then
    HAVE_ERROR_AT_LINE=1
  else
    HAVE_ERROR_AT_LINE=0
  fi
  REPLACE_ERROR_AT_LINE=0

  if test $ac_cv_func_error = yes && test $ac_cv_lib_error_at_line = yes; then
    dnl On Android 11, when error_print_progname is set, the output of the
    dnl error() function contains an extra space.
    AC_CACHE_CHECK([for working error function],
      [gl_cv_func_working_error],
      [if test $cross_compiling != yes; then
         AC_LINK_IFELSE(
           [AC_LANG_PROGRAM([[
              #include <error.h>
              static void print_no_progname (void) {}
            ]], [[
              error_print_progname = print_no_progname;
              error (0, 0, "foo");
            ]])
           ],
           [rm -f conftest.out
            if test -s conftest$ac_exeext \
               && ./conftest$ac_exeext 2> conftest.out; then
              if grep ' ' conftest.out >/dev/null; then
                gl_cv_func_working_error=no
              else
                gl_cv_func_working_error=yes
              fi
            else
              gl_cv_func_working_error=no
            fi
            rm -f conftest.out
           ],
           [gl_cv_func_working_error=no])
       else
         AC_COMPILE_IFELSE(
           [AC_LANG_PROGRAM([[
              #include <error.h>
            ]], [[
              error (0, 0, "foo");
            ]])
           ],
           [case "$host_os" in
                               # Guess yes on glibc systems.
              *-gnu* | gnu*)   gl_cv_func_working_error="guessing yes" ;;
                               # Guess no on Android.
              linux*-android*) gl_cv_func_working_error="guessing no" ;;
                               # If we don't know, obey --enable-cross-guesses.
              *)               gl_cv_func_working_error="$gl_cross_guess_normal" ;;
            esac
           ],
           [gl_cv_func_working_error=no])
       fi
      ])
    case "$gl_cv_func_working_error" in
      *no)
        REPLACE_ERROR=1
        REPLACE_ERROR_AT_LINE=1
        ;;
    esac
  fi

  if test $HAVE_ERROR = 0 || test $REPLACE_ERROR = 1 \
     || test $HAVE_ERROR_AT_LINE = 0 || test $REPLACE_ERROR_AT_LINE = 1; then
    COMPILE_ERROR_C=1
  else
    COMPILE_ERROR_C=0
  fi

  AC_SUBST([HAVE_ERROR])
  AC_SUBST([HAVE_ERROR_AT_LINE])
  AC_SUBST([REPLACE_ERROR])
  AC_SUBST([REPLACE_ERROR_AT_LINE])
])
