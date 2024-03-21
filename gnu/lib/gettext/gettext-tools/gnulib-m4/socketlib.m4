# socketlib.m4 serial 3
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl gl_SOCKETLIB
dnl Determines the library to use for socket functions.
dnl Sets and AC_SUBSTs LIBSOCKET.

AC_DEFUN([gl_SOCKETLIB],
[
  gl_PREREQ_SYS_H_WINSOCK2 dnl for HAVE_WINSOCK2_H
  LIBSOCKET=
  if test $HAVE_WINSOCK2_H = 1; then
    dnl Native Windows API (not Cygwin).
    dnl If the function WSAStartup exists (declared in <winsock2.h> and
    dnl defined through -lws2_32), we need to call it.
    AC_CACHE_CHECK([for WSAStartup],
      [gl_cv_func_wsastartup], [
       gl_save_LIBS="$LIBS"
       LIBS="$LIBS -lws2_32"
       AC_LINK_IFELSE(
         [AC_LANG_PROGRAM([[
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif]], [[
            WORD wVersionRequested = MAKEWORD(1, 1);
            WSADATA wsaData;
            int err = WSAStartup(wVersionRequested, &wsaData);
            WSACleanup ();
            ]])
         ],
         [gl_cv_func_wsastartup=yes],
         [gl_cv_func_wsastartup=no])
       LIBS="$gl_save_LIBS"
      ])
    if test "$gl_cv_func_wsastartup" = "yes"; then
      AC_DEFINE([WINDOWS_SOCKETS], [1], [Define if WSAStartup is needed.])
      LIBSOCKET='-lws2_32'
    fi
  else
    dnl Unix API.
    dnl Solaris has most socket functions in libsocket.
    dnl Haiku has most socket functions in libnetwork.
    dnl BeOS has most socket functions in libnet.
    dnl On HP-UX, do NOT link with libxnet, because in 64-bit mode this would
    dnl break code (e.g. in libraries) that invokes accept(), getpeername(),
    dnl getsockname(), getsockopt(), or recvfrom() with a 32-bit addrlen. See
    dnl "man xopen_networking" for details.
    AC_CACHE_CHECK([for library containing setsockopt], [gl_cv_lib_socket], [
      gl_cv_lib_socket=
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[extern
#ifdef __cplusplus
"C"
#endif
char setsockopt();]], [[setsockopt();]])],
        [],
        [gl_save_LIBS="$LIBS"
         LIBS="$gl_save_LIBS -lsocket"
         AC_LINK_IFELSE([AC_LANG_PROGRAM([[extern
#ifdef __cplusplus
"C"
#endif
char setsockopt();]], [[setsockopt();]])],
           [gl_cv_lib_socket="-lsocket"])
         if test -z "$gl_cv_lib_socket"; then
           LIBS="$gl_save_LIBS -lnetwork"
           AC_LINK_IFELSE([AC_LANG_PROGRAM([[extern
#ifdef __cplusplus
"C"
#endif
char setsockopt();]], [[setsockopt();]])],
             [gl_cv_lib_socket="-lnetwork"])
           if test -z "$gl_cv_lib_socket"; then
             LIBS="$gl_save_LIBS -lnet"
             AC_LINK_IFELSE([AC_LANG_PROGRAM([[extern
#ifdef __cplusplus
"C"
#endif
char setsockopt();]], [[setsockopt();]])],
               [gl_cv_lib_socket="-lnet"])
           fi
         fi
         LIBS="$gl_save_LIBS"
        ])
      if test -z "$gl_cv_lib_socket"; then
        gl_cv_lib_socket="none needed"
      fi
    ])
    if test "$gl_cv_lib_socket" != "none needed"; then
      LIBSOCKET="$gl_cv_lib_socket"
    fi
  fi
  AC_SUBST([LIBSOCKET])
])
