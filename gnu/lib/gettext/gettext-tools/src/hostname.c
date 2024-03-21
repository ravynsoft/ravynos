/* Display hostname in various forms.
   Copyright (C) 2001-2003, 2006-2007, 2012, 2014, 2018-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#if defined _WIN32
# define WIN32_NATIVE
#endif


/* We use the getaddrinfo and getnameinfo implementation from gnulib.  */
#define HAVE_GETADDRINFO 1

/* Support for using getaddrinfo() and getnameinfo().  */
#if HAVE_GETADDRINFO
# include <sys/types.h>
# include <sys/socket.h> /* defines AF_INET, AF_INET6 */
# include <netdb.h>      /* declares getaddrinfo(), getnameinfo() */
# include <netinet/in.h> /* defines struct sockaddr_in, struct sockaddr_in6 */
/* Support for using gethostbyname().  */
#elif HAVE_GETHOSTBYNAME
# include <sys/types.h>
# include <sys/socket.h> /* defines AF_INET, AF_INET6 */
# include <netinet/in.h> /* declares ntohs(), defines struct sockaddr_in */
# if HAVE_ARPA_INET_H
#  include <arpa/inet.h> /* declares inet_ntoa(), inet_ntop() */
# endif
# if HAVE_IPV6
#  if !defined(__CYGWIN__) /* Cygwin has only s6_addr, no s6_addr16 */
#   if defined(__APPLE__) && defined(__MACH__) /* MacOS X */
#    define in6_u __u6_addr
#    define u6_addr16 __u6_addr16
#   endif
    /* Use s6_addr16 for portability.  See RFC 2553.  */
#   ifndef s6_addr16
#    define s6_addr16 in6_u.u6_addr16
#   endif
#   define HAVE_IN6_S6_ADDR16 1
#  endif
# endif
# include <netdb.h> /* defines struct hostent, declares gethostbyname() */
#endif


/* Do these includes after the network-related ones, because on native Windows,
   the #include <winsock2.h> must precede the #include <windows.h>.  */

/* Get gethostname().  */
#include <unistd.h>

#ifdef WIN32_NATIVE
/* Native Woe32 API lacks gethostname() but has GetComputerName() instead.  */
# include <windows.h>
#else
/* Some systems, like early Solaris versions, lack gethostname() but
   have uname() instead.  */
# if !HAVE_GETHOSTNAME
#  include <sys/utsname.h>
# endif
#endif

/* Get MAXHOSTNAMELEN.  */
#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif


/* Include this after <sys/socket.h>, to avoid a syntax error on BeOS.  */
#include <stdbool.h>

#include "noreturn.h"
#include "closeout.h"
#include "error.h"
#include "error-progname.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "xalloc.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Output format.  */
static enum { default_format, short_format, long_format, ip_format } format;

/* Long options.  */
static const struct option long_options[] =
{
  { "fqdn", no_argument, NULL, 'f' },
  { "help", no_argument, NULL, 'h' },
  { "ip-address", no_argument, NULL, 'i' },
  { "long", no_argument, NULL, 'f' },
  { "short", no_argument, NULL, 's' },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};


/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);
static void print_hostname (void);

int
main (int argc, char *argv[])
{
  int optchar;
  bool do_help;
  bool do_version;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  /* Set default values for variables.  */
  do_help = false;
  do_version = false;
  format = default_format;

  /* Parse command line options.  */
  while ((optchar = getopt_long (argc, argv, "fhisV", long_options, NULL))
         != EOF)
    switch (optchar)
    {
    case '\0':          /* Long option.  */
      break;
    case 'f':
      format = long_format;
      break;
    case 's':
      format = short_format;
      break;
    case 'i':
      format = ip_format;
      break;
    case 'h':
      do_help = true;
      break;
    case 'V':
      do_version = true;
      break;
    default:
      usage (EXIT_FAILURE);
      /* NOTREACHED */
    }

  /* Version information requested.  */
  if (do_version)
    {
      printf ("%s (GNU %s) %s\n", last_component (program_name),
              PACKAGE, VERSION);
      /* xgettext: no-wrap */
      printf (_("Copyright (C) %s Free Software Foundation, Inc.\n\
License GPLv3+: GNU GPL version 3 or later <%s>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
"),
              "2001-2023", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Bruno Haible"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Test for extraneous arguments.  */
  if (optind != argc)
    error (EXIT_FAILURE, 0, _("too many arguments"));

  /* Get and print the hostname.  */
  print_hostname ();

  exit (EXIT_SUCCESS);
}

/* Display usage information and exit.  */
static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try '%s --help' for more information.\n"),
             program_name);
  else
    {
      printf (_("\
Usage: %s [OPTION]\n\
"), program_name);
      printf ("\n");
      printf (_("\
Print the machine's hostname.\n"));
      printf ("\n");
      printf (_("\
Output format:\n"));
      printf (_("\
  -s, --short                 short host name\n"));
      printf (_("\
  -f, --fqdn, --long          long host name, includes fully qualified domain\n\
                                name, and aliases\n"));
      printf (_("\
  -i, --ip-address            addresses for the hostname\n"));
      printf ("\n");
      printf (_("\
Informative output:\n"));
      printf (_("\
  -h, --help                  display this help and exit\n"));
      printf (_("\
  -V, --version               output version information and exit\n"));
      printf ("\n");
      /* TRANSLATORS: The first placeholder is the web address of the Savannah
         project of this package.  The second placeholder is the bug-reporting
         email address for this package.  Please add _another line_ saying
         "Report translation bugs to <...>\n" with the address for translation
         bugs (typically your translation team's web or email address).  */
      printf(_("\
Report bugs in the bug tracker at <%s>\n\
or by email to <%s>.\n"),
             "https://savannah.gnu.org/projects/gettext",
             "bug-gettext@gnu.org");
    }

  exit (status);
}

/* Returns an xmalloc()ed string containing the machine's host name.  */
static char *
xgethostname ()
{
#ifdef WIN32_NATIVE
  char hostname[MAX_COMPUTERNAME_LENGTH+1];
  DWORD size = sizeof (hostname);

  if (!GetComputerName (hostname, &size))
    error (EXIT_FAILURE, 0, _("could not get host name"));
  return xstrdup (hostname);
#elif HAVE_GETHOSTNAME
  char hostname[MAXHOSTNAMELEN+1];

  if (gethostname (hostname, MAXHOSTNAMELEN) < 0)
    error (EXIT_FAILURE, errno, _("could not get host name"));
  hostname[MAXHOSTNAMELEN] = '\0';
  return xstrdup (hostname);
#else
  struct utsname utsname;

  if (uname (&utsname) < 0)
    error (EXIT_FAILURE, errno, _("could not get host name"));
  return xstrdup (utsname.nodename);
#endif
}

/* Converts an AF_INET address to a printable, presentable format.
   BUFFER is an array with at least 15+1 bytes.  ADDR is 'struct in_addr'.  */
#if HAVE_INET_NTOP
# define ipv4_ntop(buffer,addr) \
    inet_ntop (AF_INET, &addr, buffer, 15+1)
#else
# define ipv4_ntop(buffer,addr) \
    strcpy (buffer, inet_ntoa (addr))
#endif

#if HAVE_IPV6
/* Converts an AF_INET6 address to a printable, presentable format.
   BUFFER is an array with at least 45+1 bytes.  ADDR is 'struct in6_addr'.  */
# if HAVE_INET_NTOP
#  define ipv6_ntop(buffer,addr) \
     inet_ntop (AF_INET6, &addr, buffer, 45+1)
# elif HAVE_IN6_S6_ADDR16
#  define ipv6_ntop(buffer,addr) \
     sprintf (buffer, "%x:%x:%x:%x:%x:%x:%x:%x", \
              ntohs ((addr).s6_addr16[0]), \
              ntohs ((addr).s6_addr16[1]), \
              ntohs ((addr).s6_addr16[2]), \
              ntohs ((addr).s6_addr16[3]), \
              ntohs ((addr).s6_addr16[4]), \
              ntohs ((addr).s6_addr16[5]), \
              ntohs ((addr).s6_addr16[6]), \
              ntohs ((addr).s6_addr16[7]))
# else
#  define ipv6_ntop(buffer,addr) \
     sprintf (buffer, "%x:%x:%x:%x:%x:%x:%x:%x", \
              ((addr).s6_addr[0] << 8) | (addr).s6_addr[1], \
              ((addr).s6_addr[2] << 8) | (addr).s6_addr[3], \
              ((addr).s6_addr[4] << 8) | (addr).s6_addr[5], \
              ((addr).s6_addr[6] << 8) | (addr).s6_addr[7], \
              ((addr).s6_addr[8] << 8) | (addr).s6_addr[9], \
              ((addr).s6_addr[10] << 8) | (addr).s6_addr[11], \
              ((addr).s6_addr[12] << 8) | (addr).s6_addr[13], \
              ((addr).s6_addr[14] << 8) | (addr).s6_addr[15])
# endif
#endif

/* Tests whether an IPv4 address is link-local.  */
static bool
ipv4_is_linklocal (const struct in_addr *addr)
{
  return (((const unsigned char *) addr)[0] == 169)
         && (((const unsigned char *) addr)[1] == 254);
}

#if HAVE_IPV6
/* Tests whether an IPv6 address is link-local.  */
static bool
ipv6_is_linklocal (const struct in6_addr *addr)
{
  /* Cf. IN6_IS_ADDR_LINKLOCAL macro.  */
  return (((const unsigned char *) addr)[0] == 0xFE)
         && ((((const unsigned char *) addr)[1] & 0xC0) == 0x80);
}
#endif

/* Print the hostname according to the specified format.  */
static void
print_hostname ()
{
  char *hostname;
  char *dot;

  hostname = xgethostname ();

  switch (format)
    {
    case default_format:
      /* Print the hostname, as returned by the system call.  */
      printf ("%s\n", hostname);
      break;

    case short_format:
      /* Print only the part before the first dot.  */
      dot = strchr (hostname, '.');
      if (dot != NULL)
        *dot = '\0';
      printf ("%s\n", hostname);
      break;

    case long_format:
#if HAVE_GETADDRINFO
      /* Look for netwide usable hostname and aliases using getaddrinfo().
         getnameinfo() is not even needed.  */
      {
        struct addrinfo hints;
        struct addrinfo *res;
        int ret;

        memset (&hints, 0, sizeof (hints));
        hints.ai_family = AF_UNSPEC; /* either AF_INET or AF_INET6 is ok */
        hints.ai_socktype = SOCK_STREAM; /* or SOCK_DGRAM or 0 */
        hints.ai_protocol = 0; /* any protocol is ok */
        hints.ai_flags = AI_CANONNAME;

        ret = getaddrinfo (hostname, NULL, &hints, &res);
        if (ret == 0)
          {
            struct addrinfo *p;

            for (p = res; p != NULL; p = p->ai_next)
              {
                /* Typically p->ai_socktype == SOCK_STREAM, p->ai_protocol == IPPROTO_TCP,
                   or        p->ai_socktype == SOCK_DGRAM, p->ai_protocol == IPPROTO_UDP.  */
                /* p->ai_canonname is only set on the first 'struct addrinfo'.  */
                if (p->ai_canonname != NULL)
                  printf ("%s\n", p->ai_canonname);
              }

            freeaddrinfo (res);
          }
        else
          printf ("%s\n", hostname);
      }
#elif HAVE_GETHOSTBYNAME
      /* Look for netwide usable hostname and aliases using gethostbyname().  */
      {
        struct hostent *h;
        size_t i;

        h = gethostbyname (hostname);
        if (h != NULL)
          {
            printf ("%s\n", h->h_name);
            if (h->h_aliases != NULL)
              for (i = 0; h->h_aliases[i] != NULL; i++)
                printf ("%s\n", h->h_aliases[i]);
          }
        else
          printf ("%s\n", hostname);
      }
#else
      printf ("%s\n", hostname);
#endif
      break;

    case ip_format:
#if HAVE_GETADDRINFO
      /* Look for netwide usable IP addresses using getaddrinfo() and
         getnameinfo().  */
      {
        struct addrinfo hints;
        struct addrinfo *res;
        int ret;
        char host[1025];

        memset (&hints, 0, sizeof (hints));
        hints.ai_family = AF_UNSPEC; /* either AF_INET or AF_INET6 is ok */
        hints.ai_socktype = SOCK_STREAM; /* or SOCK_DGRAM */
        hints.ai_protocol = 0; /* any protocol is ok */
        hints.ai_flags = 0;

        ret = getaddrinfo (hostname, NULL, &hints, &res);
        if (ret == 0)
          {
            struct addrinfo *p;

            for (p = res; p != NULL; p = p->ai_next)
              {
                /* Typically p->ai_socktype == SOCK_STREAM, p->ai_protocol == IPPROTO_TCP,
                   or        p->ai_socktype == SOCK_DGRAM, p->ai_protocol == IPPROTO_UDP.  */
                /* Ignore link-local addresses.
                   <https://en.wikipedia.org/wiki/Link-local_address>.  */
                if (!((p->ai_family == AF_INET
                       && ipv4_is_linklocal (&((const struct sockaddr_in *) p->ai_addr)->sin_addr))
# if HAVE_IPV6
                      || (p->ai_family == AF_INET6
                          && ipv6_is_linklocal (&((const struct sockaddr_in6 *) p->ai_addr)->sin6_addr))
# endif
                   ) )
                  if (getnameinfo (p->ai_addr, p->ai_addrlen,
                                   host, sizeof (host),
                                   NULL, 0,
                                   NI_NUMERICHOST)
                      == 0)
                    {
                      printf ("[%.*s]\n", (int) sizeof (host), host);
                    }
              }

            freeaddrinfo (res);
          }
      }
#elif HAVE_GETHOSTBYNAME
      /* Look for netwide usable IP addresses using gethostbyname().  */
      {
        struct hostent *h;
        size_t i;

        h = gethostbyname (hostname);
        if (h != NULL && h->h_addr_list != NULL)
          for (i = 0; h->h_addr_list[i] != NULL; i++)
            {
# if HAVE_IPV6
              if (h->h_addrtype == AF_INET6)
                {
                  char buffer[45+1];
                  ipv6_ntop (buffer, *(const struct in6_addr*) h->h_addr_list[i]);
                  printf("[%s]\n", buffer);
                }
              else
# endif
              if (h->h_addrtype == AF_INET)
                {
                  char buffer[15+1];
                  ipv4_ntop (buffer, *(const struct in_addr*) h->h_addr_list[i]);
                  printf("[%s]\n", buffer);
                }
            }
      }
#endif
      break;

    default:
      abort ();
    }
}
