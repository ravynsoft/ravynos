# hostent.m4 serial 4
dnl Copyright (C) 2008, 2010-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_HOSTENT],
[
  dnl Where are gethostent(), sethostent(), endhostent(), gethostbyname(),
  dnl gethostbyaddr() defined?
  dnl - On Solaris < 11.4, they are in libnsl. Ignore libxnet.
  dnl - On Haiku, they are in libnetwork.
  dnl - On BeOS, they are in libnet.
  dnl - On native Windows, they are in ws2_32.dll.
  dnl - Otherwise they are in libc.
  AC_REQUIRE([gl_SYS_SOCKET_H])dnl for HAVE_SYS_SOCKET_H, HAVE_WINSOCK2_H
  HOSTENT_LIB=
  gl_saved_libs="$LIBS"
  AC_SEARCH_LIBS([gethostbyname], [nsl network net],
    [if test "$ac_cv_search_gethostbyname" != "none required"; then
       HOSTENT_LIB="$ac_cv_search_gethostbyname"
     fi])
  LIBS="$gl_saved_libs"
  if test -z "$HOSTENT_LIB"; then
    AC_CHECK_FUNCS([gethostbyname], , [
      AC_CACHE_CHECK([for gethostbyname in winsock2.h and -lws2_32],
        [gl_cv_w32_gethostbyname],
        [gl_cv_w32_gethostbyname=no
         gl_save_LIBS="$LIBS"
         LIBS="$LIBS -lws2_32"
         AC_LINK_IFELSE(
           [AC_LANG_PROGRAM(
              [[
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#include <stddef.h>
              ]],
              [[gethostbyname(NULL);]])],
           [gl_cv_w32_gethostbyname=yes])
         LIBS="$gl_save_LIBS"
        ])
      if test "$gl_cv_w32_gethostbyname" = "yes"; then
        HOSTENT_LIB="-lws2_32"
      fi
    ])
  fi
  AC_SUBST([HOSTENT_LIB])
])
