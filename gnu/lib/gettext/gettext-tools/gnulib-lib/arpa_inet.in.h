/* A GNU-like <arpa/inet.h>.

   Copyright (C) 2005-2006, 2008-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _@GUARD_PREFIX@_ARPA_INET_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

#if @HAVE_FEATURES_H@
# include <features.h> /* for __GLIBC__ */
#endif

/* Gnulib's sys/socket.h is responsible for defining socklen_t (used below) and
   for pulling in winsock2.h etc. under MinGW.
   But avoid namespace pollution on glibc systems.  */
#ifndef __GLIBC__
# include <sys/socket.h>
#endif

/* On NonStop Kernel, inet_ntop and inet_pton are declared in <netdb.h>.
   But avoid namespace pollution on glibc systems.  */
#if defined __TANDEM && !defined __GLIBC__
# include <netdb.h>
#endif

#if @HAVE_ARPA_INET_H@

/* The include_next requires a split double-inclusion guard.  */
# @INCLUDE_NEXT@ @NEXT_ARPA_INET_H@

#endif

#ifndef _@GUARD_PREFIX@_ARPA_INET_H
#define _@GUARD_PREFIX@_ARPA_INET_H

/* This file uses GNULIB_POSIXCHECK, HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Get all possible declarations of inet_ntop() and inet_pton().  */
#if (@GNULIB_INET_NTOP@ || @GNULIB_INET_PTON@ || defined GNULIB_POSIXCHECK) \
    && @HAVE_WS2TCPIP_H@
# include <ws2tcpip.h>
#endif

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */


#if @GNULIB_INET_NTOP@
/* Converts an internet address from internal format to a printable,
   presentable format.
   AF is an internet address family, such as AF_INET or AF_INET6.
   SRC points to a 'struct in_addr' (for AF_INET) or 'struct in6_addr'
   (for AF_INET6).
   DST points to a buffer having room for CNT bytes.
   The printable representation of the address (in numeric form, not
   surrounded by [...], no reverse DNS is done) is placed in DST, and
   DST is returned.  If an error occurs, the return value is NULL and
   errno is set.  If CNT bytes are not sufficient to hold the result,
   the return value is NULL and errno is set to ENOSPC.  A good value
   for CNT is 46.

   For more details, see the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/inet_ntop.html>.  */
# if @REPLACE_INET_NTOP@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef inet_ntop
#   define inet_ntop rpl_inet_ntop
#  endif
_GL_FUNCDECL_RPL (inet_ntop, const char *,
                  (int af, const void *restrict src,
                   char *restrict dst, socklen_t cnt)
                  _GL_ARG_NONNULL ((2, 3)));
_GL_CXXALIAS_RPL (inet_ntop, const char *,
                  (int af, const void *restrict src,
                   char *restrict dst, socklen_t cnt));
# else
#  if !@HAVE_DECL_INET_NTOP@
_GL_FUNCDECL_SYS (inet_ntop, const char *,
                  (int af, const void *restrict src,
                   char *restrict dst, socklen_t cnt)
                  _GL_ARG_NONNULL ((2, 3)));
#  endif
/* Need to cast, because on NonStop Kernel, the fourth parameter is
                                            size_t cnt.  */
_GL_CXXALIAS_SYS_CAST (inet_ntop, const char *,
                       (int af, const void *restrict src,
                        char *restrict dst, socklen_t cnt));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (inet_ntop);
# endif
#elif defined GNULIB_POSIXCHECK
# undef inet_ntop
# if HAVE_RAW_DECL_INET_NTOP
_GL_WARN_ON_USE (inet_ntop, "inet_ntop is unportable - "
                 "use gnulib module inet_ntop for portability");
# endif
#endif

#if @GNULIB_INET_PTON@
# if @REPLACE_INET_PTON@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef inet_pton
#   define inet_pton rpl_inet_pton
#  endif
_GL_FUNCDECL_RPL (inet_pton, int,
                  (int af, const char *restrict src, void *restrict dst)
                  _GL_ARG_NONNULL ((2, 3)));
_GL_CXXALIAS_RPL (inet_pton, int,
                  (int af, const char *restrict src, void *restrict dst));
# else
#  if !@HAVE_DECL_INET_PTON@
_GL_FUNCDECL_SYS (inet_pton, int,
                  (int af, const char *restrict src, void *restrict dst)
                  _GL_ARG_NONNULL ((2, 3)));
#  endif
_GL_CXXALIAS_SYS (inet_pton, int,
                  (int af, const char *restrict src, void *restrict dst));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (inet_pton);
# endif
#elif defined GNULIB_POSIXCHECK
# undef inet_pton
# if HAVE_RAW_DECL_INET_PTON
_GL_WARN_ON_USE (inet_pton, "inet_pton is unportable - "
                 "use gnulib module inet_pton for portability");
# endif
#endif


#endif /* _@GUARD_PREFIX@_ARPA_INET_H */
#endif /* _@GUARD_PREFIX@_ARPA_INET_H */
