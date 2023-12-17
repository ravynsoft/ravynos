# sys_ioctl_h.m4 serial 15
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Written by Bruno Haible.

AC_DEFUN_ONCE([gl_SYS_IOCTL_H],
[
  dnl Ensure to expand the default settings once only, before all statements
  dnl that occur in other macros.
  AC_REQUIRE([gl_SYS_IOCTL_H_DEFAULTS])

  AC_CHECK_HEADERS_ONCE([sys/ioctl.h])
  if test $ac_cv_header_sys_ioctl_h = yes; then
    HAVE_SYS_IOCTL_H=1
    dnl Test whether <sys/ioctl.h> declares ioctl(), or whether some other
    dnl header file, such as <unistd.h> or <stropts.h>, is needed for that.
    AC_CACHE_CHECK([whether <sys/ioctl.h> declares ioctl],
      [gl_cv_decl_ioctl_in_sys_ioctl_h],
      [dnl We cannot use AC_CHECK_DECL because it produces its own messages.
       AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <sys/ioctl.h>]],
            [[(void) ioctl;]])],
         [gl_cv_decl_ioctl_in_sys_ioctl_h=yes],
         [gl_cv_decl_ioctl_in_sys_ioctl_h=no])
      ])
  else
    HAVE_SYS_IOCTL_H=0
  fi
  AC_SUBST([HAVE_SYS_IOCTL_H])
  dnl <sys/ioctl.h> is always overridden, because of GNULIB_POSIXCHECK.
  gl_CHECK_NEXT_HEADERS([sys/ioctl.h])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use.
  gl_WARN_ON_USE_PREPARE([[#include <sys/ioctl.h>
/* Some platforms declare ioctl in the wrong header.  */
#if !(defined __GLIBC__ && !defined __UCLIBC__)
# include <unistd.h>
#endif
    ]], [ioctl])
])

# gl_SYS_IOCTL_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_SYS_IOCTL_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_SYS_IOCTL_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_SYS_IOCTL_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_SYS_IOCTL_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_IOCTL])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_SYS_IOCTL_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_SYS_IOCTL_H_DEFAULTS])
])

AC_DEFUN([gl_SYS_IOCTL_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  SYS_IOCTL_H_HAVE_WINSOCK2_H=0; AC_SUBST([SYS_IOCTL_H_HAVE_WINSOCK2_H])
  SYS_IOCTL_H_HAVE_WINSOCK2_H_AND_USE_SOCKETS=0;
                        AC_SUBST([SYS_IOCTL_H_HAVE_WINSOCK2_H_AND_USE_SOCKETS])
  REPLACE_IOCTL=0;      AC_SUBST([REPLACE_IOCTL])
])
