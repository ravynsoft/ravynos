# sockpfaf.m4 serial 10
dnl Copyright (C) 2004, 2006, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Test for some common socket protocol families (PF_INET, PF_INET6, ...)
dnl and some common address families (AF_INET, AF_INET6, ...).
dnl This test assumes that a system supports an address family if and only if
dnl it supports the corresponding protocol family.

dnl From Bruno Haible.

AC_DEFUN([gl_SOCKET_FAMILIES],
[
  AC_REQUIRE([gl_SYS_SOCKET_H])
  AC_CHECK_HEADERS_ONCE([netinet/in.h])

  AC_CACHE_CHECK([for IPv4 sockets],
    [gl_cv_socket_ipv4],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif]],
[[int x = AF_INET; struct in_addr y; struct sockaddr_in z;
 if (&x && &y && &z) return 0;]])],
       gl_cv_socket_ipv4=yes, gl_cv_socket_ipv4=no)])
  if test $gl_cv_socket_ipv4 = yes; then
    AC_DEFINE([HAVE_IPV4], [1], [Define to 1 if <sys/socket.h> defines AF_INET.])
  fi

  AC_CACHE_CHECK([for IPv6 sockets],
    [gl_cv_socket_ipv6],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif]],
[[int x = AF_INET6; struct in6_addr y; struct sockaddr_in6 z;
 if (&x && &y && &z) return 0;]])],
       gl_cv_socket_ipv6=yes, gl_cv_socket_ipv6=no)])
  if test $gl_cv_socket_ipv6 = yes; then
    AC_DEFINE([HAVE_IPV6], [1], [Define to 1 if <sys/socket.h> defines AF_INET6.])
  fi
])

AC_DEFUN([gl_SOCKET_FAMILY_UNIX],
[
  AC_REQUIRE([gl_SYS_SOCKET_H])
  AC_CHECK_HEADERS_ONCE([sys/un.h])

  AC_CACHE_CHECK([for UNIX domain sockets],
    [gl_cv_socket_unix],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif]],
[[int x = AF_UNIX; struct sockaddr_un y;
 if (&x && &y) return 0;]])],
       gl_cv_socket_unix=yes, gl_cv_socket_unix=no)])
  if test $gl_cv_socket_unix = yes; then
    AC_DEFINE([HAVE_UNIXSOCKET], [1], [Define to 1 if <sys/socket.h> defines AF_UNIX.])
  fi
])
