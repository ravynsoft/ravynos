/* Get address information (partial implementation).
   Copyright (C) 1997, 2001-2002, 2004-2023 Free Software Foundation, Inc.
   Contributed by Simon Josefsson <simon@josefsson.org>.

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

/* Don't use __attribute__ __nonnull__ in this compilation unit.  Otherwise gcc
   optimizes away the sa == NULL test below.  */
#define _GL_ARG_NONNULL(params)

#include <config.h>

#include <netdb.h>

#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

/* Get inet_ntop.  */
#include <arpa/inet.h>

/* Get calloc. */
#include <stdlib.h>

/* Get memcpy, strdup. */
#include <string.h>

/* Get snprintf. */
#include <stdio.h>

#include "gettext.h"
#define _(String) gettext (String)
#define N_(String) String

/* BeOS has AF_INET, but not PF_INET.  */
#ifndef PF_INET
# define PF_INET AF_INET
#endif
/* BeOS also lacks PF_UNSPEC.  */
#ifndef PF_UNSPEC
# define PF_UNSPEC 0
#endif

#if HAVE_GETADDRINFO

/* Override with cdecl calling convention.  */

int
getaddrinfo (const char *restrict nodename,
             const char *restrict servname,
             const struct addrinfo *restrict hints,
             struct addrinfo **restrict res)
# undef getaddrinfo
{
  return getaddrinfo (nodename, servname, hints, res);
}

void
freeaddrinfo (struct addrinfo *ai)
# undef freeaddrinfo
{
  freeaddrinfo (ai);
}

#else

# if defined _WIN32 && !defined __CYGWIN__
#  define WINDOWS_NATIVE
# endif

/* gl_sockets_startup */
# include "sockets.h"

# ifdef WINDOWS_NATIVE

/* Don't assume that UNICODE is not defined.  */
#  undef GetModuleHandle
#  define GetModuleHandle GetModuleHandleA

#  if !(_WIN32_WINNT >= _WIN32_WINNT_WINXP)

/* Avoid warnings from gcc -Wcast-function-type.  */
#   define GetProcAddress \
     (void *) GetProcAddress

typedef int (WSAAPI *getaddrinfo_func) (const char*, const char*,
                                        const struct addrinfo*,
                                        struct addrinfo**);
typedef void (WSAAPI *freeaddrinfo_func) (struct addrinfo*);
typedef int (WSAAPI *getnameinfo_func) (const struct sockaddr*,
                                        socklen_t, char*, DWORD,
                                        char*, DWORD, int);

static getaddrinfo_func getaddrinfo_ptr = NULL;
static freeaddrinfo_func freeaddrinfo_ptr = NULL;
static getnameinfo_func getnameinfo_ptr = NULL;

static int
use_win32_p (void)
{
  static int done = 0;
  HMODULE h;

  if (done)
    return getaddrinfo_ptr ? 1 : 0;

  done = 1;

  h = GetModuleHandle ("ws2_32.dll");

  if (h)
    {
      getaddrinfo_ptr = (getaddrinfo_func) GetProcAddress (h, "getaddrinfo");
      freeaddrinfo_ptr = (freeaddrinfo_func) GetProcAddress (h, "freeaddrinfo");
      getnameinfo_ptr = (getnameinfo_func) GetProcAddress (h, "getnameinfo");
    }

  /* If either is missing, something is odd. */
  if (!getaddrinfo_ptr || !freeaddrinfo_ptr || !getnameinfo_ptr)
    {
      getaddrinfo_ptr = NULL;
      freeaddrinfo_ptr = NULL;
      getnameinfo_ptr = NULL;
      return 0;
    }

  gl_sockets_startup (SOCKETS_1_1);

  return 1;
}

#  else

static int
use_win32_p (void)
{
  static int done = 0;

  if (!done)
    {
      done = 1;

      gl_sockets_startup (SOCKETS_1_1);
    }

  return 1;
}

#   define getaddrinfo_ptr getaddrinfo
#   define freeaddrinfo_ptr freeaddrinfo
#   define getnameinfo_ptr getnameinfo

#  endif
# endif

static bool
validate_family (int family)
{
  /* FIXME: Support more families. */
# if HAVE_IPV4
     if (family == PF_INET)
       return true;
# endif
# if HAVE_IPV6
     if (family == PF_INET6)
       return true;
# endif
     if (family == PF_UNSPEC)
       return true;
     return false;
}

/* Translate name of a service location and/or a service name to set of
   socket addresses. */
int
getaddrinfo (const char *restrict nodename,
             const char *restrict servname,
             const struct addrinfo *restrict hints,
             struct addrinfo **restrict res)
#undef getaddrinfo
{
  struct addrinfo *tmp;
  int port = 0;
  struct hostent *he;
  void *storage;
  size_t size;
# if HAVE_IPV6
  struct v6_pair {
    struct addrinfo addrinfo;
    struct sockaddr_in6 sockaddr_in6;
  };
# endif
# if HAVE_IPV4
  struct v4_pair {
    struct addrinfo addrinfo;
    struct sockaddr_in sockaddr_in;
  };
# endif

# ifdef WINDOWS_NATIVE
  if (use_win32_p ())
    return getaddrinfo_ptr (nodename, servname, hints, res);
# endif

  if (hints && (hints->ai_flags & ~(AI_CANONNAME|AI_PASSIVE)))
    /* FIXME: Support more flags. */
    return EAI_BADFLAGS;

  if (hints && !validate_family (hints->ai_family))
    return EAI_FAMILY;

  if (hints &&
      hints->ai_socktype != SOCK_STREAM && hints->ai_socktype != SOCK_DGRAM)
    /* FIXME: Support other socktype. */
    return EAI_SOCKTYPE; /* FIXME: Better return code? */

  if (!nodename)
    {
      if (!(hints->ai_flags & AI_PASSIVE))
        return EAI_NONAME;

# ifdef HAVE_IPV6
      nodename = (hints->ai_family == AF_INET6) ? "::" : "0.0.0.0";
# else
      nodename = "0.0.0.0";
# endif
    }

  if (servname)
    {
      struct servent *se = NULL;
      const char *proto =
        (hints && hints->ai_socktype == SOCK_DGRAM) ? "udp" : "tcp";

      if (hints == NULL || !(hints->ai_flags & AI_NUMERICSERV))
        /* FIXME: Use getservbyname_r if available. */
        se = getservbyname (servname, proto);

      if (!se)
        {
          char *c;
          if (!(*servname >= '0' && *servname <= '9'))
            return EAI_NONAME;
          port = strtoul (servname, &c, 10);
          if (*c || port > 0xffff)
            return EAI_NONAME;
          port = htons (port);
        }
      else
        port = se->s_port;
    }

  /* FIXME: Use gethostbyname_r if available. */
  he = gethostbyname (nodename);
  if (!he || he->h_addr_list[0] == NULL)
    return EAI_NONAME;

  switch (he->h_addrtype)
    {
# if HAVE_IPV6
    case PF_INET6:
      size = sizeof (struct v6_pair);
      break;
# endif

# if HAVE_IPV4
    case PF_INET:
      size = sizeof (struct v4_pair);
      break;
# endif

    default:
      return EAI_NODATA;
    }

  storage = calloc (1, size);
  if (!storage)
    return EAI_MEMORY;

  switch (he->h_addrtype)
    {
# if HAVE_IPV6
    case PF_INET6:
      {
        struct v6_pair *p = storage;
        struct sockaddr_in6 *sinp = &p->sockaddr_in6;
        tmp = &p->addrinfo;

        if (port)
          sinp->sin6_port = port;

        if (he->h_length != sizeof (sinp->sin6_addr))
          {
            free (storage);
            return EAI_SYSTEM; /* FIXME: Better return code?  Set errno? */
          }

        memcpy (&sinp->sin6_addr, he->h_addr_list[0], sizeof sinp->sin6_addr);

        tmp->ai_addr = (struct sockaddr *) sinp;
        tmp->ai_addrlen = sizeof *sinp;
      }
      break;
# endif

# if HAVE_IPV4
    case PF_INET:
      {
        struct v4_pair *p = storage;
        struct sockaddr_in *sinp = &p->sockaddr_in;
        tmp = &p->addrinfo;

        if (port)
          sinp->sin_port = port;

        if (he->h_length != sizeof (sinp->sin_addr))
          {
            free (storage);
            return EAI_SYSTEM; /* FIXME: Better return code?  Set errno? */
          }

        memcpy (&sinp->sin_addr, he->h_addr_list[0], sizeof sinp->sin_addr);

        tmp->ai_addr = (struct sockaddr *) sinp;
        tmp->ai_addrlen = sizeof *sinp;
      }
      break;
# endif

    default:
      free (storage);
      return EAI_NODATA;
    }

  if (hints && hints->ai_flags & AI_CANONNAME)
    {
      const char *cn;
      if (he->h_name)
        cn = he->h_name;
      else
        cn = nodename;

      tmp->ai_canonname = strdup (cn);
      if (!tmp->ai_canonname)
        {
          free (storage);
          return EAI_MEMORY;
        }
    }

  tmp->ai_protocol = (hints) ? hints->ai_protocol : 0;
  tmp->ai_socktype = (hints) ? hints->ai_socktype : 0;
  tmp->ai_addr->sa_family = he->h_addrtype;
  tmp->ai_family = he->h_addrtype;

# ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
  switch (he->h_addrtype)
    {
#  if HAVE_IPV4
    case AF_INET:
      tmp->ai_addr->sa_len = sizeof (struct sockaddr_in);
      break;
#  endif
#  if HAVE_IPV6
    case AF_INET6:
      tmp->ai_addr->sa_len = sizeof (struct sockaddr_in6);
      break;
#  endif
    }
# endif

  /* FIXME: If more than one address, create linked list of addrinfo's. */

  *res = tmp;

  return 0;
}

/* Free 'addrinfo' structure AI including associated storage.  */
void
freeaddrinfo (struct addrinfo *ai)
#undef freeaddrinfo
{
# ifdef WINDOWS_NATIVE
  if (use_win32_p ())
    {
      freeaddrinfo_ptr (ai);
      return;
    }
# endif

  while (ai)
    {
      struct addrinfo *cur;

      cur = ai;
      ai = ai->ai_next;

      free (cur->ai_canonname);
      free (cur);
    }
}

int
getnameinfo (const struct sockaddr *restrict sa, socklen_t salen,
             char *restrict node, socklen_t nodelen,
             char *restrict service, socklen_t servicelen,
             int flags)
#undef getnameinfo
{
# ifdef WINDOWS_NATIVE
  if (use_win32_p ())
    return getnameinfo_ptr (sa, salen, node, nodelen,
                            service, servicelen, flags);
# endif

  /* FIXME: Support other flags. */
  if ((node && nodelen > 0 && !(flags & NI_NUMERICHOST)) ||
      (service && servicelen > 0 && !(flags & NI_NUMERICHOST)) ||
      (flags & ~(NI_NUMERICHOST|NI_NUMERICSERV)))
    return EAI_BADFLAGS;

  if (sa == NULL || salen < sizeof (sa->sa_family))
    return EAI_FAMILY;

  switch (sa->sa_family)
    {
# if HAVE_IPV4
    case AF_INET:
      if (salen < sizeof (struct sockaddr_in))
        return EAI_FAMILY;
      break;
# endif
# if HAVE_IPV6
    case AF_INET6:
      if (salen < sizeof (struct sockaddr_in6))
        return EAI_FAMILY;
      break;
# endif
    default:
      return EAI_FAMILY;
    }

  if (node && nodelen > 0 && flags & NI_NUMERICHOST)
    {
      switch (sa->sa_family)
        {
# if HAVE_IPV4
        case AF_INET:
          if (!inet_ntop (AF_INET,
                          &(((const struct sockaddr_in *) sa)->sin_addr),
                          node, nodelen))
            return EAI_SYSTEM;
          break;
# endif

# if HAVE_IPV6
        case AF_INET6:
          if (!inet_ntop (AF_INET6,
                          &(((const struct sockaddr_in6 *) sa)->sin6_addr),
                          node, nodelen))
            return EAI_SYSTEM;
          break;
# endif

        default:
          return EAI_FAMILY;
        }
    }

  if (service && servicelen > 0 && flags & NI_NUMERICSERV)
    switch (sa->sa_family)
      {
# if HAVE_IPV4
      case AF_INET:
# endif
# if HAVE_IPV6
      case AF_INET6:
# endif
        {
          unsigned short int port
            = ntohs (((const struct sockaddr_in *) sa)->sin_port);
          if (servicelen <= snprintf (service, servicelen, "%u", port))
            return EAI_OVERFLOW;
        }
        break;
      }

  return 0;
}

#endif
