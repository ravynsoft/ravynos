# sys_stat_h.m4 serial 42   -*- Autoconf -*-
dnl Copyright (C) 2006-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Eric Blake.
dnl Provide a GNU-like <sys/stat.h>.

AC_DEFUN_ONCE([gl_SYS_STAT_H],
[
  AC_REQUIRE([gl_SYS_STAT_H_DEFAULTS])

  dnl Check for broken stat macros.
  AC_REQUIRE([AC_HEADER_STAT])

  gl_CHECK_NEXT_HEADERS([sys/stat.h])

  dnl Ensure the type mode_t gets defined.
  AC_REQUIRE([AC_TYPE_MODE_T])

  dnl Whether to enable precise timestamps in 'struct stat'.
  m4_ifdef([gl_WINDOWS_STAT_TIMESPEC], [
    AC_REQUIRE([gl_WINDOWS_STAT_TIMESPEC])
  ], [
    WINDOWS_STAT_TIMESPEC=0
  ])
  AC_SUBST([WINDOWS_STAT_TIMESPEC])

  dnl Whether to ensure that struct stat.st_size is 64-bit wide.
  m4_ifdef([gl_LARGEFILE], [
    AC_REQUIRE([gl_LARGEFILE])
  ], [
    WINDOWS_64_BIT_ST_SIZE=0
  ])
  AC_SUBST([WINDOWS_64_BIT_ST_SIZE])

  dnl Define types that are supposed to be defined in <sys/types.h> or
  dnl <sys/stat.h>.
  AC_CHECK_TYPE([nlink_t], [],
    [AC_DEFINE([nlink_t], [int],
       [Define to the type of st_nlink in struct stat, or a supertype.])],
    [#include <sys/types.h>
     #include <sys/stat.h>])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <sys/stat.h>
    ]], [chmod fchmodat fstat fstatat futimens getumask lchmod lstat
    mkdirat mkfifo mkfifoat mknod mknodat stat utimensat])

  AC_REQUIRE([AC_C_RESTRICT])
])

# gl_SYS_STAT_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_SYS_STAT_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_SYS_STAT_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_SYS_STAT_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_SYS_STAT_H_MODULE_INDICATOR_DEFAULTS], [
    gl_UNISTD_H_REQUIRE_DEFAULTS dnl for REPLACE_FCHDIR
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_CHMOD])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FCHMODAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FSTAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FSTATAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FUTIMENS])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_GETUMASK])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LCHMOD])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_LSTAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MKDIR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MKDIRAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MKFIFO])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MKFIFOAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MKNOD])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MKNODAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_STAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_UTIMENSAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_OVERRIDES_STRUCT_STAT])
    dnl Support Microsoft deprecated alias function names by default.
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_CHMOD], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_MKDIR], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_UMASK], [1])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_SYS_STAT_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_SYS_STAT_H_DEFAULTS])
])

AC_DEFUN([gl_SYS_STAT_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_FCHMODAT=1;      AC_SUBST([HAVE_FCHMODAT])
  HAVE_FSTATAT=1;       AC_SUBST([HAVE_FSTATAT])
  HAVE_FUTIMENS=1;      AC_SUBST([HAVE_FUTIMENS])
  HAVE_GETUMASK=1;      AC_SUBST([HAVE_GETUMASK])
  HAVE_LCHMOD=1;        AC_SUBST([HAVE_LCHMOD])
  HAVE_LSTAT=1;         AC_SUBST([HAVE_LSTAT])
  HAVE_MKDIRAT=1;       AC_SUBST([HAVE_MKDIRAT])
  HAVE_MKFIFO=1;        AC_SUBST([HAVE_MKFIFO])
  HAVE_MKFIFOAT=1;      AC_SUBST([HAVE_MKFIFOAT])
  HAVE_MKNOD=1;         AC_SUBST([HAVE_MKNOD])
  HAVE_MKNODAT=1;       AC_SUBST([HAVE_MKNODAT])
  HAVE_UTIMENSAT=1;     AC_SUBST([HAVE_UTIMENSAT])
  REPLACE_CHMOD=0;      AC_SUBST([REPLACE_CHMOD])
  REPLACE_FCHMODAT=0;   AC_SUBST([REPLACE_FCHMODAT])
  REPLACE_FSTAT=0;      AC_SUBST([REPLACE_FSTAT])
  REPLACE_FSTATAT=0;    AC_SUBST([REPLACE_FSTATAT])
  REPLACE_FUTIMENS=0;   AC_SUBST([REPLACE_FUTIMENS])
  REPLACE_LSTAT=0;      AC_SUBST([REPLACE_LSTAT])
  REPLACE_MKDIR=0;      AC_SUBST([REPLACE_MKDIR])
  REPLACE_MKFIFO=0;     AC_SUBST([REPLACE_MKFIFO])
  REPLACE_MKFIFOAT=0;   AC_SUBST([REPLACE_MKFIFOAT])
  REPLACE_MKNOD=0;      AC_SUBST([REPLACE_MKNOD])
  REPLACE_MKNODAT=0;    AC_SUBST([REPLACE_MKNODAT])
  REPLACE_STAT=0;       AC_SUBST([REPLACE_STAT])
  REPLACE_UTIMENSAT=0;  AC_SUBST([REPLACE_UTIMENSAT])
])
