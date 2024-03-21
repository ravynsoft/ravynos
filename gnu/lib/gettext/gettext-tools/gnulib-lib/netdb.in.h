/* Provide a netdb.h header file for systems lacking it (read: MinGW).
   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Written by Simon Josefsson.

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

/* This file is supposed to be used on platforms that lack <netdb.h>.
   It is intended to provide definitions and prototypes needed by an
   application.  */

#ifndef _@GUARD_PREFIX@_NETDB_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

#if @HAVE_NETDB_H@

/* The include_next requires a split double-inclusion guard.  */
# @INCLUDE_NEXT@ @NEXT_NETDB_H@

#endif

#ifndef _@GUARD_PREFIX@_NETDB_H
#define _@GUARD_PREFIX@_NETDB_H

/* This file uses GNULIB_POSIXCHECK, HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Get <netdb.h> definitions such as 'socklen_t' on IRIX 6.5 and OSF/1 4.0 and
   'struct hostent' on MinGW.  */
#include <sys/socket.h>

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */

/* Declarations for a platform that lacks <netdb.h>, or where it is
   incomplete.  */

#if @GNULIB_GETADDRINFO@

# if !@HAVE_STRUCT_ADDRINFO@

#  ifdef __cplusplus
extern "C" {
#  endif

#  if !GNULIB_defined_struct_addrinfo
/* Structure to contain information about address of a service provider.  */
struct addrinfo
{
  int ai_flags;                 /* Input flags.  */
  int ai_family;                /* Protocol family for socket.  */
  int ai_socktype;              /* Socket type.  */
  int ai_protocol;              /* Protocol for socket.  */
  socklen_t ai_addrlen;         /* Length of socket address.  */
  struct sockaddr *ai_addr;     /* Socket address for socket.  */
  char *ai_canonname;           /* Canonical name for service location.  */
  struct addrinfo *ai_next;     /* Pointer to next in list.  */
};
#   define GNULIB_defined_struct_addrinfo 1
#  endif

#  ifdef __cplusplus
}
#  endif

# endif

/* Possible values for 'ai_flags' field in 'addrinfo' structure.  */
# ifndef AI_PASSIVE
#  define AI_PASSIVE    0x0001  /* Socket address is intended for 'bind'.  */
# endif
# ifndef AI_CANONNAME
#  define AI_CANONNAME  0x0002  /* Request for canonical name.  */
# endif
# ifndef AI_NUMERICSERV
#  define AI_NUMERICSERV        0x0400  /* Don't use name resolution.  */
# endif

# if 0
#  define AI_NUMERICHOST        0x0004  /* Don't use name resolution.  */
# endif

/* These symbolic constants are required to be present by POSIX, but
   our getaddrinfo replacement doesn't use them (yet).  Setting them
   to 0 on systems that doesn't have them avoids causing problems for
   system getaddrinfo implementations that would be confused by
   unknown values.  */
# ifndef AI_V4MAPPED
#  define AI_V4MAPPED    0 /* 0x0008: IPv4 mapped addresses are acceptable.  */
# endif
# ifndef AI_ALL
#  define AI_ALL         0 /* 0x0010: Return IPv4 mapped and IPv6 addresses. */
# endif
# ifndef AI_ADDRCONFIG
#  define AI_ADDRCONFIG  0 /* 0x0020: Use configuration of this host to choose
                                      returned address type.  */
# endif

/* Error values for 'getaddrinfo' function.  */
# ifndef EAI_BADFLAGS
#  define EAI_BADFLAGS    -1    /* Invalid value for 'ai_flags' field.  */
#  define EAI_NONAME      -2    /* NAME or SERVICE is unknown.  */
#  define EAI_AGAIN       -3    /* Temporary failure in name resolution.  */
#  define EAI_FAIL        -4    /* Non-recoverable failure in name res.  */
#  define EAI_NODATA      -5    /* No address associated with NAME.  */
#  define EAI_FAMILY      -6    /* 'ai_family' not supported.  */
#  define EAI_SOCKTYPE    -7    /* 'ai_socktype' not supported.  */
#  define EAI_SERVICE     -8    /* SERVICE not supported for 'ai_socktype'.  */
#  define EAI_MEMORY      -10   /* Memory allocation failure.  */
# endif

/* Since EAI_NODATA is deprecated by RFC3493, some systems (at least
   FreeBSD, which does define EAI_BADFLAGS) have removed the definition
   in favor of EAI_NONAME.  */
# if !defined EAI_NODATA && defined EAI_NONAME
#  define EAI_NODATA EAI_NONAME
# endif

# ifndef EAI_OVERFLOW
/* Not defined on mingw32 and Haiku. */
#  define EAI_OVERFLOW    -12   /* Argument buffer overflow.  */
# endif
# ifndef EAI_ADDRFAMILY
/* Not defined on mingw32. */
#  define EAI_ADDRFAMILY  -9    /* Address family for NAME not supported.  */
# endif
# ifndef EAI_SYSTEM
/* Not defined on mingw32. */
#  define EAI_SYSTEM      -11   /* System error returned in 'errno'.  */
# endif

# if 0
/* The commented out definitions below are not yet implemented in the
   GNULIB getaddrinfo() replacement, so are not yet needed.

   If they are restored, be sure to protect the definitions with #ifndef.  */
#  ifndef EAI_INPROGRESS
#   define EAI_INPROGRESS       -100    /* Processing request in progress.  */
#   define EAI_CANCELED         -101    /* Request canceled.  */
#   define EAI_NOTCANCELED      -102    /* Request not canceled.  */
#   define EAI_ALLDONE          -103    /* All requests done.  */
#   define EAI_INTR             -104    /* Interrupted by a signal.  */
#   define EAI_IDN_ENCODE       -105    /* IDN encoding failed.  */
#  endif
# endif

/* Translate name of a service location and/or a service name to set of
   socket addresses.
   For more details, see the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/getaddrinfo.html>.  */
# if @REPLACE_GETADDRINFO@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getaddrinfo
#   define getaddrinfo rpl_getaddrinfo
#  endif
_GL_FUNCDECL_RPL (getaddrinfo, int,
                  (const char *restrict nodename,
                   const char *restrict servname,
                   const struct addrinfo *restrict hints,
                   struct addrinfo **restrict res)
                  _GL_ARG_NONNULL ((4)));
_GL_CXXALIAS_RPL (getaddrinfo, int,
                  (const char *restrict nodename,
                   const char *restrict servname,
                   const struct addrinfo *restrict hints,
                   struct addrinfo **restrict res));
# else
#  if !@HAVE_DECL_GETADDRINFO@
_GL_FUNCDECL_SYS (getaddrinfo, int,
                  (const char *restrict nodename,
                   const char *restrict servname,
                   const struct addrinfo *restrict hints,
                   struct addrinfo **restrict res)
                  _GL_ARG_NONNULL ((4)));
#  endif
_GL_CXXALIAS_SYS (getaddrinfo, int,
                  (const char *restrict nodename,
                   const char *restrict servname,
                   const struct addrinfo *restrict hints,
                   struct addrinfo **restrict res));
# endif
_GL_CXXALIASWARN (getaddrinfo);

/* Free 'addrinfo' structure AI including associated storage.
   For more details, see the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/getaddrinfo.html>.  */
# if @REPLACE_GETADDRINFO@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef freeaddrinfo
#   define freeaddrinfo rpl_freeaddrinfo
#  endif
_GL_FUNCDECL_RPL (freeaddrinfo, void, (struct addrinfo *ai)
                                      _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (freeaddrinfo, void, (struct addrinfo *ai));
# else
#  if !@HAVE_DECL_FREEADDRINFO@
_GL_FUNCDECL_SYS (freeaddrinfo, void, (struct addrinfo *ai)
                                      _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (freeaddrinfo, void, (struct addrinfo *ai));
# endif
_GL_CXXALIASWARN (freeaddrinfo);

# if @REPLACE_GAI_STRERROR@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef gai_strerror
#   define gai_strerror rpl_gai_strerror
#  endif
_GL_FUNCDECL_RPL (gai_strerror, const char *, (int ecode));
_GL_CXXALIAS_RPL (gai_strerror, const char *, (int ecode));
# else
#  if !@HAVE_DECL_GAI_STRERROR@
/* Convert error return from getaddrinfo() to a string.
   For more details, see the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/gai_strerror.html>.  */
_GL_FUNCDECL_SYS (gai_strerror, const char *, (int ecode));
#  endif
_GL_CXXALIAS_SYS (gai_strerror, const char *, (int ecode));
# endif
# if __GLIBC__ >= 2
_GL_CXXALIASWARN (gai_strerror);
# endif

# if !@HAVE_DECL_GETNAMEINFO@
/* Convert socket address to printable node and service names.
   For more details, see the POSIX:2008 specification
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnameinfo.html>.  */
_GL_FUNCDECL_SYS (getnameinfo, int,
                  (const struct sockaddr *restrict sa, socklen_t salen,
                   char *restrict node, socklen_t nodelen,
                   char *restrict service, socklen_t servicelen,
                   int flags)
                  _GL_ARG_NONNULL ((1)));
# endif
/* Need to cast, because on glibc systems, the seventh parameter is
                        unsigned int flags.  */
_GL_CXXALIAS_SYS_CAST (getnameinfo, int,
                       (const struct sockaddr *restrict sa, socklen_t salen,
                        char *restrict node, socklen_t nodelen,
                        char *restrict service, socklen_t servicelen,
                        int flags));
_GL_CXXALIASWARN (getnameinfo);

/* Possible flags for getnameinfo.  */
# ifndef NI_NUMERICHOST
#  define NI_NUMERICHOST 1
# endif
# ifndef NI_NUMERICSERV
#  define NI_NUMERICSERV 2
# endif

#elif defined GNULIB_POSIXCHECK

# undef getaddrinfo
# if HAVE_RAW_DECL_GETADDRINFO
_GL_WARN_ON_USE (getaddrinfo, "getaddrinfo is unportable - "
                 "use gnulib module getaddrinfo for portability");
# endif

# undef freeaddrinfo
# if HAVE_RAW_DECL_FREEADDRINFO
_GL_WARN_ON_USE (freeaddrinfo, "freeaddrinfo is unportable - "
                 "use gnulib module getaddrinfo for portability");
# endif

# undef gai_strerror
# if HAVE_RAW_DECL_GAI_STRERROR
_GL_WARN_ON_USE (gai_strerror, "gai_strerror is unportable - "
                 "use gnulib module getaddrinfo for portability");
# endif

# undef getnameinfo
# if HAVE_RAW_DECL_GETNAMEINFO
_GL_WARN_ON_USE (getnameinfo, "getnameinfo is unportable - "
                 "use gnulib module getaddrinfo for portability");
# endif

#endif

#endif /* _@GUARD_PREFIX@_NETDB_H */
#endif /* _@GUARD_PREFIX@_NETDB_H */
