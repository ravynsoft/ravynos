# servent.m4 serial 4
dnl Copyright (C) 2008, 2010-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_SERVENT],
[
  dnl Where are getservent(), setservent(), endservent(), getservbyname(),
  dnl getservbyport() defined?
  dnl Where are getprotoent(), setprotoent(), endprotoent(), getprotobyname(),
  dnl getprotobynumber() defined?
  dnl - On Solaris < 11.4, they are in libsocket. Ignore libxnet.
  dnl - On Haiku, they are in libnetwork.
  dnl - On BeOS, they are in libnet.
  dnl - On native Windows, they are in ws2_32.dll.
  dnl - Otherwise they are in libc.
  AC_REQUIRE([gl_SYS_SOCKET_H])dnl for HAVE_SYS_SOCKET_H, HAVE_WINSOCK2_H
  SERVENT_LIB=
  gl_saved_libs="$LIBS"
  AC_SEARCH_LIBS([getservbyname], [socket network net],
    [if test "$ac_cv_search_getservbyname" != "none required"; then
       SERVENT_LIB="$ac_cv_search_getservbyname"
     fi])
  LIBS="$gl_saved_libs"
  if test -z "$SERVENT_LIB"; then
    AC_CHECK_FUNCS([getservbyname], , [
      AC_CACHE_CHECK([for getservbyname in winsock2.h and -lws2_32],
        [gl_cv_w32_getservbyname],
        [gl_cv_w32_getservbyname=no
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
              [[getservbyname(NULL,NULL);]])],
           [gl_cv_w32_getservbyname=yes])
         LIBS="$gl_save_LIBS"
        ])
      if test "$gl_cv_w32_getservbyname" = "yes"; then
        SERVENT_LIB="-lws2_32"
      fi
    ])
  fi
  AC_SUBST([SERVENT_LIB])
])
