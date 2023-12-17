/* Test the getaddrinfo module.

   Copyright (C) 2006-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Simon Josefsson.  */

#include <config.h>

#include <netdb.h>

#include "signature.h"
SIGNATURE_CHECK (gai_strerror, char const *, (int));
/* On native Windows, these two functions may have the __stdcall calling
   convention.  But the SIGNATURE_CHECK works only for functions with __cdecl
   calling convention.  */
#if !(defined _WIN32 && !defined __CYGWIN__)
SIGNATURE_CHECK (freeaddrinfo, void, (struct addrinfo *));
SIGNATURE_CHECK (getaddrinfo, int, (char const *, char const *,
                                    struct addrinfo const *,
                                    struct addrinfo **));
#endif

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "sockets.h"

/* Whether to print debugging messages.  */
#define ENABLE_DEBUGGING 0

#if ENABLE_DEBUGGING
# define dbgprintf printf
#else
# define dbgprintf if (0) printf
#endif

/* BeOS does not have AF_UNSPEC.  */
#ifndef AF_UNSPEC
# define AF_UNSPEC 0
#endif

#ifndef EAI_SERVICE
# define EAI_SERVICE 0
#endif

static int
simple (char const *host, char const *service)
{
  char buf[BUFSIZ];
  static int skip = 0;
  struct addrinfo hints;
  struct addrinfo *ai0, *ai;
  int res;
  int err;

  /* Once we skipped the test, do not try anything else */
  if (skip)
    return 0;

  dbgprintf ("Finding %s service %s...\n", host, service);

  /* This initializes "hints" but does not use it.  Is there a reason
     for this?  If so, please fix this comment.  */
  memset (&hints, 0, sizeof (hints));
  hints.ai_flags = AI_CANONNAME;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  res = getaddrinfo (host, service, 0, &ai0);
  err = errno;

  dbgprintf ("res %d: %s\n", res, gai_strerror (res));

  if (res != 0)
    {
      /* EAI_AGAIN is returned if no network is available. Don't fail
         the test merely because someone is down the country on their
         in-law's farm. */
      if (res == EAI_AGAIN)
        {
          skip++;
          fprintf (stderr, "skipping getaddrinfo test: no network?\n");
          return 77;
        }
      /* IRIX reports EAI_NONAME for "https".  Don't fail the test
         merely because of this.  */
      if (res == EAI_NONAME)
        return 0;
      /* Solaris reports EAI_SERVICE for "http" and "https".  Don't
         fail the test merely because of this.  */
      if (res == EAI_SERVICE)
        return 0;
#ifdef EAI_NODATA
      /* AIX reports EAI_NODATA for "https".  Don't fail the test
         merely because of this.  */
      if (res == EAI_NODATA)
        return 0;
#endif
      /* Provide details if errno was set.  */
      if (res == EAI_SYSTEM)
        fprintf (stderr, "system error: %s\n", strerror (err));

      return 1;
    }

  for (ai = ai0; ai; ai = ai->ai_next)
    {
      void *ai_addr = ai->ai_addr;
      struct sockaddr_in *sock_addr = ai_addr;
      dbgprintf ("\tflags %x\n", ai->ai_flags + 0u);
      dbgprintf ("\tfamily %x\n", ai->ai_family + 0u);
      dbgprintf ("\tsocktype %x\n", ai->ai_socktype + 0u);
      dbgprintf ("\tprotocol %x\n", ai->ai_protocol + 0u);
      dbgprintf ("\taddrlen %lu: ", (unsigned long) ai->ai_addrlen);
      dbgprintf ("\tFound %s\n",
                 inet_ntop (ai->ai_family,
                            &sock_addr->sin_addr,
                            buf, sizeof (buf) - 1));
      if (ai->ai_canonname)
        dbgprintf ("\tFound %s...\n", ai->ai_canonname);

      {
        char ipbuf[BUFSIZ];
        char portbuf[BUFSIZ];

        res = getnameinfo (ai->ai_addr, ai->ai_addrlen,
                           ipbuf, sizeof (ipbuf) - 1,
                           portbuf, sizeof (portbuf) - 1,
                           NI_NUMERICHOST|NI_NUMERICSERV);
        dbgprintf ("\t\tgetnameinfo %d: %s\n", res, gai_strerror (res));
        if (res == 0)
          {
            dbgprintf ("\t\tip %s\n", ipbuf);
            dbgprintf ("\t\tport %s\n", portbuf);
          }
      }

    }

  freeaddrinfo (ai0);

  return 0;
}

#define HOST1 "www.gnu.org"
#define SERV1 "http"
#define HOST2 "www.ibm.com"
#define SERV2 "https"
#define HOST3 "microsoft.com"
#define SERV3 "http"
#define HOST4 "google.org"
#define SERV4 "ldap"

int main (void)
{
  (void) gl_sockets_startup (SOCKETS_1_1);

  return simple (HOST1, SERV1)
    + simple (HOST2, SERV2)
    + simple (HOST3, SERV3)
    + simple (HOST4, SERV4);
}
