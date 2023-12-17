# spawn_h.m4 serial 24
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Written by Bruno Haible.

AC_DEFUN_ONCE([gl_SPAWN_H],
[
  dnl Ensure to expand the default settings once only, before all statements
  dnl that occur in other macros.
  AC_REQUIRE([gl_SPAWN_H_DEFAULTS])

  dnl <spawn.h> is always overridden, because of GNULIB_POSIXCHECK.
  gl_CHECK_NEXT_HEADERS([spawn.h])

  if test $ac_cv_header_spawn_h = yes; then
    HAVE_SPAWN_H=1
    AC_CHECK_TYPES([posix_spawnattr_t], [], [HAVE_POSIX_SPAWNATTR_T=0], [[
#include <spawn.h>
      ]])
    AC_CHECK_TYPES([posix_spawn_file_actions_t], [],
      [HAVE_POSIX_SPAWN_FILE_ACTIONS_T=0], [[
#include <spawn.h>
      ]])
  else
    HAVE_SPAWN_H=0
    HAVE_POSIX_SPAWNATTR_T=0
    HAVE_POSIX_SPAWN_FILE_ACTIONS_T=0
  fi
  AC_SUBST([HAVE_SPAWN_H])

  dnl Ensure the type pid_t gets defined.
  AC_REQUIRE([AC_TYPE_PID_T])

  dnl Ensure the type mode_t gets defined.
  AC_REQUIRE([AC_TYPE_MODE_T])

  AC_REQUIRE([gl_HAVE_POSIX_SPAWN])

  AC_REQUIRE([AC_C_RESTRICT])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <spawn.h>
    ]], [posix_spawn posix_spawnp posix_spawnattr_init posix_spawnattr_destroy
    posix_spawnattr_getsigdefault posix_spawnattr_setsigdefault
    posix_spawnattr_getsigmask posix_spawnattr_setsigmask
    posix_spawnattr_getflags posix_spawnattr_setflags
    posix_spawnattr_getpgroup posix_spawnattr_setpgroup
    posix_spawnattr_getschedpolicy posix_spawnattr_setschedpolicy
    posix_spawnattr_getschedparam posix_spawnattr_setschedparam
    posix_spawn_file_actions_init posix_spawn_file_actions_destroy
    posix_spawn_file_actions_addopen posix_spawn_file_actions_addclose
    posix_spawn_file_actions_adddup2 posix_spawn_file_actions_addchdir
    posix_spawn_file_actions_addfchdir])
])

dnl Checks whether the system has the functions posix_spawn.
dnl Sets ac_cv_func_posix_spawn and HAVE_POSIX_SPAWN.
AC_DEFUN([gl_HAVE_POSIX_SPAWN],
[
  dnl Ensure to expand the default settings once only, before all statements
  dnl that occur in other macros.
  AC_REQUIRE([gl_SPAWN_H_DEFAULTS])

  POSIX_SPAWN_LIB=
  AC_SUBST([POSIX_SPAWN_LIB])
  gl_saved_libs=$LIBS
    AC_SEARCH_LIBS([posix_spawn], [rt],
                   [test "$ac_cv_search_posix_spawn" = "none required" ||
                    POSIX_SPAWN_LIB=$ac_cv_search_posix_spawn])
    gl_CHECK_FUNCS_ANDROID([posix_spawn], [[#include <spawn.h>]])
  LIBS=$gl_saved_libs
  dnl For backward compatibility.
  LIB_POSIX_SPAWN="$POSIX_SPAWN_LIB"
  AC_SUBST([LIB_POSIX_SPAWN])

  if test $ac_cv_func_posix_spawn != yes; then
    HAVE_POSIX_SPAWN=0
    case "$gl_cv_onwards_func_posix_spawn" in
      future*) REPLACE_POSIX_SPAWN=1 ;;
    esac
  fi
])

# gl_SPAWN_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_SPAWN_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_SPAWN_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_SPAWN_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_SPAWN_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWN])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNP])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWN_FILE_ACTIONS_INIT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWN_FILE_ACTIONS_ADDCLOSE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWN_FILE_ACTIONS_ADDDUP2])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWN_FILE_ACTIONS_ADDFCHDIR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWN_FILE_ACTIONS_ADDOPEN])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWN_FILE_ACTIONS_DESTROY])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_INIT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_GETFLAGS])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_SETFLAGS])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_GETPGROUP])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_SETPGROUP])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_GETSCHEDPARAM])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_SETSCHEDPARAM])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_GETSCHEDPOLICY])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_SETSCHEDPOLICY])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_GETSIGDEFAULT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_SETSIGDEFAULT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_GETSIGMASK])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_SETSIGMASK])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POSIX_SPAWNATTR_DESTROY])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_SPAWN_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_SPAWN_H_DEFAULTS])
])

AC_DEFUN([gl_SPAWN_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_POSIX_SPAWN=1;        AC_SUBST([HAVE_POSIX_SPAWN])
  HAVE_POSIX_SPAWNATTR_T=1;  AC_SUBST([HAVE_POSIX_SPAWNATTR_T])
  HAVE_POSIX_SPAWN_FILE_ACTIONS_T=1;
                             AC_SUBST([HAVE_POSIX_SPAWN_FILE_ACTIONS_T])
  HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR=1;
                             AC_SUBST([HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR])
  HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDFCHDIR=1;
                             AC_SUBST([HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDFCHDIR])
  REPLACE_POSIX_SPAWN=0;     AC_SUBST([REPLACE_POSIX_SPAWN])
  REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR=0;
                             AC_SUBST([REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR])
  REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDCLOSE=0;
                             AC_SUBST([REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDCLOSE])
  REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDDUP2=0;
                             AC_SUBST([REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDDUP2])
  REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDFCHDIR=0;
                             AC_SUBST([REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDFCHDIR])
  REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDOPEN=0;
                             AC_SUBST([REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDOPEN])
])
