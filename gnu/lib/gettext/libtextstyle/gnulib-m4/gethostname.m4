# gethostname.m4 serial 15
dnl Copyright (C) 2002, 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Ensure
# - the gethostname() function,
# - the HOST_NAME_MAX macro in <limits.h>.
AC_DEFUN([gl_FUNC_GETHOSTNAME],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  gl_PREREQ_SYS_H_WINSOCK2

  dnl Where is gethostname() defined?
  dnl - On native Windows, it is in ws2_32.dll.
  dnl - Otherwise it is in libc.
  GETHOSTNAME_LIB=
  AC_CHECK_FUNCS([gethostname], , [
    AC_CACHE_CHECK([for gethostname in winsock2.h and -lws2_32],
      [gl_cv_w32_gethostname],
      [gl_cv_w32_gethostname=no
       gl_save_LIBS="$LIBS"
       LIBS="$LIBS -lws2_32"
       AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#include <stddef.h>
]], [[gethostname(NULL, 0);]])], [gl_cv_w32_gethostname=yes])
       LIBS="$gl_save_LIBS"
      ])
    if test "$gl_cv_w32_gethostname" = "yes"; then
      GETHOSTNAME_LIB="-lws2_32"
    fi
  ])
  AC_SUBST([GETHOSTNAME_LIB])

  if test "$ac_cv_func_gethostname" = no; then
    HAVE_GETHOSTNAME=0
  fi

  gl_PREREQ_HOST_NAME_MAX
])

# Provide HOST_NAME_MAX when <limits.h> lacks it.
AC_DEFUN([gl_PREREQ_HOST_NAME_MAX], [
  dnl - On most Unix systems, use MAXHOSTNAMELEN from <sys/param.h> instead.
  dnl - On Solaris, Cygwin, BeOS, use MAXHOSTNAMELEN from <netdb.h> instead.
  dnl - On mingw, use 256, because
  dnl   <https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-gethostname> says:
  dnl   "if a buffer of 256 bytes is passed in the name parameter and
  dnl    the namelen parameter is set to 256, the buffer size will always
  dnl    be adequate."
  dnl With this, there is no need to use sysconf (_SC_HOST_NAME_MAX), which
  dnl is not a compile-time constant.
  dnl We cannot override <limits.h> using the usual technique, because
  dnl gl_CHECK_NEXT_HEADERS does not work for <limits.h>. Therefore retrieve
  dnl the value of HOST_NAME_MAX at configure time.
  AC_CHECK_HEADERS_ONCE([sys/param.h])
  AC_CHECK_HEADERS_ONCE([sys/socket.h])
  AC_CHECK_HEADERS_ONCE([netdb.h])
  AC_CACHE_CHECK([for HOST_NAME_MAX], [gl_cv_decl_HOST_NAME_MAX], [
    gl_cv_decl_HOST_NAME_MAX=
    AC_EGREP_CPP([lucky], [
#include <limits.h>
#ifdef HOST_NAME_MAX
lucky
#endif
      ], [gl_cv_decl_HOST_NAME_MAX=yes])
    if test -z "$gl_cv_decl_HOST_NAME_MAX"; then
      dnl It's not defined in <limits.h>. Substitute it.
      if test "$gl_cv_w32_gethostname" = yes; then
        dnl mingw.
        gl_cv_decl_HOST_NAME_MAX=256
      else
        AC_COMPUTE_INT([gl_cv_decl_HOST_NAME_MAX], [MAXHOSTNAMELEN], [
#include <sys/types.h>
#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NETDB_H
# include <netdb.h>
#endif
],
          [dnl The system does not define MAXHOSTNAMELEN in any of the common
           dnl headers. Use a safe fallback.
           gl_cv_decl_HOST_NAME_MAX=256
          ])
      fi
    fi
  ])
  if test "$gl_cv_decl_HOST_NAME_MAX" != yes; then
    AC_DEFINE_UNQUOTED([HOST_NAME_MAX], [$gl_cv_decl_HOST_NAME_MAX],
      [Define HOST_NAME_MAX when <limits.h> does not define it.])
  fi
])

# Prerequisites of lib/gethostname.c.
AC_DEFUN([gl_PREREQ_GETHOSTNAME], [
  if test "$gl_cv_w32_gethostname" != "yes"; then
    AC_CHECK_FUNCS([uname])
  fi
])
