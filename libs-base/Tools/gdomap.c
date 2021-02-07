/* This is a simple name server for GNUstep Distributed Objects
   Copyright (C) 1996, 1997, 1998, 2002 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 1996

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.
   */

/* Ported to mingw 07/12/00 by Bjorn Giesler <Bjoern.Giesler@gmx.de> */

#include "config.h"

#if	!defined(__MINGW__)
#  if	defined(__MINGW32__) || defined(__MINGW64__)
#    define	__MINGW__
#  endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>		/* for gethostname() */
#include <inttypes.h>
#ifndef __MINGW__
#include <sys/param.h>		/* for MAXHOSTNAMELEN */
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>		/* for inet_ntoa() */
#endif /* !__MINGW__ */
#include <errno.h>
#include <limits.h>
#include <string.h>		/* for strchr() */
#include <ctype.h>		/* for strchr() */
#include <fcntl.h>
#if	defined(__MINGW__)
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wininet.h>
#include <process.h>
#include <sys/time.h>
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/file.h>
#if	defined(HAVE_TIME_H)
#include <time.h>
#endif
#if	defined(HAVE_PWD_H)
#include <pwd.h>
#endif
#if	defined(HAVE_GRP_H)
#include <grp.h>
#endif

#if HAVE_GETOPT_H
#include <getopt.h>
#endif

/*
 *	Stuff for setting the sockets into non-blocking mode.
 */
#if	defined(__POSIX_SOURCE) || defined(__EXT_POSIX1_198808)
#define NBLK_OPT     O_NONBLOCK
#else
#define NBLK_OPT     FNDELAY
#endif

#include <netinet/in.h>
#include <net/if.h>
#if	!defined(SIOCGIFCONF) || defined(__CYGWIN__)
#include <sys/ioctl.h>
#ifndef	SIOCGIFCONF
#include <sys/sockio.h>
#endif
#endif

#if	defined(__svr4__)
#if defined(HAVE_SYS_STROPTS_H)
#include <sys/stropts.h>
#endif
#endif
#endif /* !__MINGW__ */


#if	defined(HAVE_SYSLOG_H)
#include <syslog.h>
#elif   defined(HAVE_SYS_SLOG_H)
#  include <sys/slog.h>
#  if 	defined(HAVE_SYS_SLOGCODES_H)
#    include <sys/slogcodes.h>
#  endif
#endif

#if	HAVE_STRERROR
  #define	lastErr()	strerror(errno)
#else
#if	defined(__MINGW__)
static	errbuf[BUFSIZ];
  #define	lastErr()	(sprintf(errbuf, "WSAGetLastError()=%d", WSAGetLastError()), errbuf)
#else
static	errbuf[BUFSIZ];
  #define	lastErr()	(sprintf(errbuf, "%m"), errbuf)
#endif
#endif

#include	"gdomap.h"
/*
 *	ABOUT THIS PROGRAM
 *
 *	This is a simple name server for GNUstep Distributed Objects
 *	The server program listens on a well known port (service name 'gdomap')
 *
 *	The officially assigned port is 538.  On most systems port numbers
 *	under 1024 can only be used by root (for security).  So this program
 *	needs to be run as root.
 *
 *	This is UNIX code - I have no idea how portable to other OSs it may be.
 *
 *	For detailed information about the communication protocol used - see
 *	the include file.
 */

/* For IRIX machines, which don't define this */
#ifndef        IPPORT_USERRESERVED
#define        IPPORT_USERRESERVED     5000
#endif /* IPPORT_USERRESERVED */

#define QUEBACKLOG	(16)	/* How many coonections to queue.	*/
#define	MAX_IFACE	(256)	/* How many network interfaces.		*/
#define	IASIZE		(sizeof(struct in_addr))

#define	MAX_EXTRA	((GDO_NAME_MAX_LEN - 2 * IASIZE)/IASIZE)

#define ROUND(V, A) \
  ({ __typeof__(V) __v=(V); __typeof__(A) __a=(A); \
	 __a*((__v+__a-1)/__a); })

typedef	unsigned char	*uptr;
#ifndef __MINGW__
static int	is_daemon = 0;		/* Currently running as daemon.	 */
#endif
static int	in_config = 0;		/* Reading config file.	 */
static int	debug = 0;		/* Extra debug gdomap_logging.	 */
static int	nobcst = 0;		/* turn off broadcast probing.	 */
static int	nofork = 0;		/* turn off fork() for debugging. */
static int	noprobe = 0;		/* disable probe for unknown servers. */
static int	interval = 600;		/* Minimum time (sec) between probes. */
static char	*pidfile = NULL;	/* file to write PID to		*/

static int	udp_sent = 0;
static int	tcp_sent = 0;
static int	udp_read = 0;
static int	tcp_read = 0;
static int	soft_int = 0;

static long	last_probe;
static struct in_addr	loopback;

static unsigned short	my_port;	/* Set in init_iface()		*/

static struct in_addr	class_a_mask;
static struct in_addr	class_b_mask;
static uint32_t	class_a_net;
static uint32_t	class_b_net;
static uint32_t	class_c_net;
struct in_addr	class_c_mask;

static char	*local_hostname = 0;

/*
 *	Predeclare some of the functions used.
 */
static void	dump_stats();
#ifndef __MINGW__
static void	dump_tables();
#endif
static void	handle_accept();
static void	handle_io();
static void	handle_read(int);
static void	handle_recv();
static void	handle_request(int);
static void	handle_send();
static void	handle_write(int);
static void	init_iface();
static void	load_iface(const char* from);
static void	init_ports();
static void	init_probe();
static void	queue_msg(struct sockaddr_in* a, uptr d, unsigned l);
static void	queue_pop();
static void	queue_probe(struct in_addr* to, struct in_addr *from,
  int num_extras, struct in_addr* extra, int is_reply);

char *xgethostname (void);

#if	defined(__MINGW__)
#ifndef HAVE_GETOPT
/* A simple implementation of getopt() */

static int
indexof(char c, char *string)
{
  int i;

  for (i = 0; i < strlen(string); i++)
    {
      if (string[i] == c)
	{
	  return i;
	}
    }
  return -1;
}

static char *optarg;

static char
getopt(int argc, char **argv, char *options)
{
  static int	argi;
  static char	*arg;
  int		index;
  char		retval = '\0';

  optarg = NULL;
  if (argi == 0)
    {
      argi = 1;
    }
  while (argi < argc)
    {
      arg = argv[argi];
      if (strlen(arg) == 2)
	{
	  if (arg[0] == '-')
	    {
	      if ((index = indexof(arg[1], options)) != -1)
		{
		  retval = arg[1];
		  if (index < strlen(options))
		    {
		      if (options[index+1] == ':')
			{
			  if (argi < argc-1)
			    {
			      argi++;
			      optarg = argv[argi];
			    }
			  else
			    {
			      return -1; /* ':' given, but argv exhausted */
			    }
			}
		    }
		}
	    }
	}
      argi++;
      return retval;
    }
  return -1;
}
#endif
#endif


static char	ebuf[2048];


#if	defined(HAVE_SYSLOG) || defined(HAVE_SLOGF)
#  if defined(HAVE_SLOGF)
#    define LOG_CRIT _SLOG_CRITICAL
#    define LOG_DEBUG _SLOG_DEBUG1
#    define LOG_ERR _SLOG_ERROR
#    define LOG_INFO _SLOG_INFO
#    define LOG_WARNING _SLOG_WARNING
#    define syslog(prio, msg,...) slogf(_SLOG_SETCODE(_SLOG_SYSLOG, 0), prio, msg, __VA_ARGS__)
#  endif

static int	log_priority = 0;

static void
gdomap_log (int prio)
{
  if (in_config)
    {
#ifndef __MINGW__
      if (geteuid () != getuid ())
        {
	  strcpy(ebuf, "problem with config file");
	}
#endif
    }
  if (is_daemon)
    {
#if   defined(HAVE_SLOGF)
      // QNX doesn't like 0 as the prio. It means "shutdown" to them.
      syslog (prio ? log_priority : prio, "%s", ebuf);
#     else
      syslog (log_priority | prio, "%s", ebuf);
#endif
    }
  else if (prio == LOG_INFO)
    {
      write (1, ebuf, strlen (ebuf));
      write (1, "\n", 1);
    }
  else
    {
      write (2, ebuf, strlen (ebuf));
      write (2, "\n", 1);
    }

  if (prio == LOG_CRIT)
    {
      if (is_daemon)
	{
	  syslog (LOG_CRIT, "%s", "exiting.");
	}
      else
     	{
	  fprintf (stderr, "exiting.\n");
	  fflush (stderr);
	}
      exit(EXIT_FAILURE);
    }
}
#else

#define	LOG_CRIT	2
#define LOG_DEBUG	0
#define LOG_ERR		1
#define LOG_INFO	0
#define LOG_WARNING	0
void
gdomap_log (int prio)
{
  if (in_config)
    {
#ifndef __MINGW__
      if (geteuid () != getuid ())
        {
	  strcpy(ebuf, "problem with config file");
	}
#endif
    }
  write (2, ebuf, strlen (ebuf));
  write (2, "\n", 1);
  if (prio == LOG_CRIT)
    {
      fprintf (stderr, "exiting.\n");
      fflush (stderr);
      exit(EXIT_FAILURE);
    }
}
#endif

/*
 *	Structure for linked list of addresses to probe rather than
 *	probing entire network.
 */
typedef struct plstruct {
  struct plstruct	*next;
  int			direct;
  struct in_addr	addr;
} plentry;

static plentry	*plist = 0;

/*
 *	Variables used for determining if a connection is from a process
 *	on the local host.
 */
static int interfaces = 0;		/* Number of interfaces.	*/
static struct in_addr	*addr;		/* Address of each interface.	*/
static unsigned char	*bcok;		/* Broadcast OK for interface?	*/
static struct in_addr	*bcst;		/* Broadcast for interface.	*/
static struct in_addr	*mask;		/* Netmask of each interface.	*/

static int
is_local_host(struct in_addr a)
{
  int	i;

  for (i = 0; i < interfaces; i++)
    {
      if (a.s_addr == addr[i].s_addr)
	{
	  return 1;
	}
    }
  return 0;
}

static int
is_probe_host(struct in_addr a)
{
  if (plist)
    {
      plentry	*p;

      /* Hosts explicitly configured to be probed are ones we treat
       * as being part of our LAN and therefore trusted.
       */
      for (p = plist; p != 0; p = p->next)
	{
	  if (a.s_addr == p->addr.s_addr)
	    {
	      return 1;
	    }
	}
    }
  return 0;
}

static int
is_local_net(struct in_addr a)
{
  int	i;

  for (i = 0; i < interfaces; i++)
    {
      if ((mask[i].s_addr & addr[i].s_addr) == (mask[i].s_addr & a.s_addr))
	{
	  return 1;
	}
    }
  return 0;
}

/*
 *	Variables used for handling non-blocking I/O on channels.
 */
static int	tcp_desc = -1;	/* Socket for incoming TCP connections.	*/
static int	udp_desc = -1;	/* Socket for UDP communications.	*/
static fd_set	read_fds;	/* Descriptors which are readable.	*/
static fd_set	write_fds;	/* Descriptors which are writable.	*/


/* Internal info structures. Rewritten Wed Jul 12 14:51:19  2000 by
   Bjoern Giesler <Bjoern.Giesler@gmx.de> to work on Win32. */

typedef struct {
#if	defined(__MINGW__)
  SOCKET s;
#else
  int s;
#endif /* __MINGW__ */
  struct sockaddr_in	addr;	/* Address of process making request.	*/
  socklen_t		pos;	/* Position reading data.		*/
  union {
    gdo_req		r;
    unsigned char	b[GDO_REQ_SIZE];
  } buf;
} RInfo;		/* State of reading each request.	*/

typedef struct {
#if	defined(__MINGW__)
  SOCKET s;
#else
  int s;
#endif /* __MINGW__ */
  int	len;		/* Length of data to be written.	*/
  int	pos;		/* Amount of data already written.	*/
  char*	buf;		/* Buffer for data.			*/
} WInfo;

static RInfo *_rInfo = NULL;
static unsigned _rInfoCapacity = 0;
static unsigned _rInfoCount = 0;
static WInfo *_wInfo = NULL;
static unsigned _wInfoCapacity = 0;
static unsigned _wInfoCount = 0;

static void
#if	defined(__MINGW__)
delRInfo(SOCKET s)
#else
delRInfo(int s)
#endif /* __MINGW__ */
{
  unsigned int	i;

  for (i = 0; i < _rInfoCount; i++)
    {
      if (_rInfo[i].s == s)
	{
	  break;
	}
    }
  if (i == _rInfoCount)
    {
      snprintf(ebuf, sizeof(ebuf),
	"%s requested unallocated RInfo struct (socket %ld)",
	__FUNCTION__, (long int)s);
      gdomap_log(LOG_ERR);
      return;
    }
  _rInfoCount--;
  if (i != _rInfoCount) /* not last element */
    {
      memmove(&(_rInfo[i]), &(_rInfo[i+1]), (_rInfoCount-i)*sizeof(RInfo));
    }
}


static RInfo *
#if	defined(__MINGW__)
getRInfo(SOCKET s, int make)
#else
getRInfo(int s, int make)
#endif
{
  unsigned int	i;

  for (i = 0; i < _rInfoCount; i++)
    {
      if (_rInfo[i].s == s)
	{
	  break;
	}
    }
  if (i == _rInfoCount)
    {
      if (make)
	{
	  if (_rInfoCount >= _rInfoCapacity)
	    {
	      RInfo	*tmp;

	      _rInfoCapacity = _rInfoCount + 1;
	      tmp = (RInfo *)calloc(_rInfoCapacity, sizeof(RInfo));
	      if (_rInfoCount > 0)
		{
		  memcpy(tmp, _rInfo, sizeof(RInfo)*_rInfoCount);
		  free(_rInfo);
		}
	      _rInfo = tmp;
	    }
	  _rInfoCount++;
	  _rInfo[_rInfoCount-1].s = s;
	  return &(_rInfo[_rInfoCount-1]);
	}
      return NULL;
    }
  return &(_rInfo[i]);
}

static void
#if	defined(__MINGW__)
delWInfo(SOCKET s)
#else
delWInfo(int s)
#endif /* __MINGW__ */
{
  unsigned int	i;

  for (i = 0; i < _wInfoCount; i++)
    {
      if (_wInfo[i].s == s)
	{
	  break;
	}
    }
  if (i == _wInfoCount)
    {
      snprintf(ebuf, sizeof(ebuf),
	"%s requested unallocated WInfo struct (socket %ld)",
	__FUNCTION__, (long int)s);
      gdomap_log(LOG_ERR);
      return;
    }
  _wInfoCount--;
  if (i != _wInfoCount) /* not last element */
    {
      memmove(&(_wInfo[i]), &(_wInfo[i+1]), (_wInfoCount-i)*sizeof(WInfo));
    }
}


static WInfo *
#if	defined(__MINGW__)
getWInfo(SOCKET s, int make)
#else
getWInfo(int s, int make)
#endif
{
  unsigned int	i;

  for (i = 0; i < _wInfoCount; i++)
    {
      if (_wInfo[i].s == s)
	{
	  break;
	}
    }
  if (i == _wInfoCount)
    {
      if (make)
	{
	  if (_wInfoCount >= _wInfoCapacity)
	    {
	      WInfo	*tmp;

	      _wInfoCapacity = _wInfoCount + 1;
	      tmp = (WInfo *)calloc(_wInfoCapacity, sizeof(WInfo));
	      if (_wInfoCount > 0)
		{
		  memcpy(tmp, _wInfo, sizeof(WInfo)*_wInfoCount);
		  free(_wInfo);
		}
	      _wInfo = tmp;
	    }
	  _wInfoCount++;
	  _wInfo[_wInfoCount-1].s = s;
	  return &(_wInfo[_wInfoCount-1]);
	}
      return NULL;
    }
  return &(_wInfo[i]);
}


static struct	u_data	{
  struct sockaddr_in	addr;	/* Address to send to.			*/
  int			pos;	/* Number of bytes already sent.	*/
  int	 		len;	/* Length of data to send.		*/
  uptr			dat;	/* Data to be sent.			*/
  struct u_data		*next;	/* Next message to send.		*/
} *u_queue = 0;
static int	udp_pending = 0;

/*
 *	Name -		queue_msg()
 *	Purpose -	Add a message to the queue of those to be sent
 *			on the UDP socket.
 */
static void
queue_msg(struct sockaddr_in* a, uptr d, unsigned l)
{
  struct u_data*	entry = (struct u_data*)malloc(sizeof(struct u_data));

  memcpy(&entry->addr, a, sizeof(*a));
  entry->pos = 0;
  entry->len = l;
  entry->dat = malloc(l);
  memcpy(entry->dat, d, l);
  entry->next = 0;
  if (u_queue)
    {
      struct u_data*	tmp = u_queue;

      while (tmp->next)
	{
	  tmp = tmp->next;
	}
      tmp->next = entry;
    }
  else
    {
      u_queue = entry;
    }
  udp_pending++;
}

static void
queue_pop()
{
  struct u_data*	tmp = u_queue;

  if (tmp)
    {
      u_queue = tmp->next;
      free(tmp->dat);
      free(tmp);
      udp_pending--;
    }
}

/*
 *	Primitive mapping stuff.
 */
typedef struct {
  uptr			name;	/* Service name registered.	*/
  unsigned int		port;	/* Port it was mapped to.	*/
  unsigned short	size;	/* Number of bytes in name.	*/
  unsigned char		net;	/* Type of port registered.	*/
  unsigned char		svc;	/* Type of port registered.	*/
} map_ent;

static int	map_used = 0;
static int	map_size = 0;
static map_ent	**map = 0;

static int
compare(uptr n0, int l0, uptr n1, int l1)
{
  if (l0 == l1)
    {
      return memcmp(n0, n1, l0);
    }
  else if (l0 < l1)
    {
      return -1;
    }
  return 1;
}

/*
 *	Name -		map_add()
 *	Purpose -	Create a new map entry structure and insert it
 *			into the map in the appropriate position.
 */
static map_ent*
map_add(uptr n, unsigned char l, unsigned int p, unsigned char t)
{
  map_ent	*m;
  int		i;

  m = (map_ent*)malloc(sizeof(map_ent));
  if (0 == m)
    {
      perror("no memory for map entry");
      exit(EXIT_FAILURE);
    }
  m->port = p;
  m->name = (unsigned char*)malloc(l);
  if (0 == m->name)
    {
      perror("no memory for map entry name");
      exit(EXIT_FAILURE);
    }
  m->size = l;
  m->net = (t & GDO_NET_MASK);
  m->svc = (t & GDO_SVC_MASK);
  memcpy(m->name, n, l);

  if (map_used == map_size)
    {
      map_size += 16;
      if (map)
	{
	  map = (map_ent**)realloc(map, map_size * sizeof(map_ent*));
	}
      else
	{
	  map = (map_ent**)calloc(map_size, sizeof(map_ent*));
	}
      if (0 == map)
        {
          perror("no memory for map");
          exit(EXIT_FAILURE);
        }
    }
  for (i = 0; i < map_used; i++)
    {
      if (compare(map[i]->name, map[i]->size, m->name, m->size) > 0)
	{
	  int	j;

	  for (j = map_used; j > i; j--)
	    {
	      map[j] = map[j-1];
	    }
	  break;
	}
    }
  map[i] = m;
  map_used++;
  if (debug > 2)
    {
      snprintf(ebuf, sizeof(ebuf), "Added port %d to map for %.*s",
	m->port, m->size, m->name);
      gdomap_log(LOG_DEBUG);
    }
  return m;
}

/*
 *	Name -		map_by_name()
 *	Purpose -	Search the map for an entry for a particular name
 */
static map_ent*
map_by_name(uptr n, int s)
{
  int		lower = 0;
  int		upper = map_used;
  int		index;

  if (debug > 2)
    {
      snprintf(ebuf, sizeof(ebuf), "Searching map for %.*s", s, n);
      gdomap_log(LOG_DEBUG);
    }
  for (index = upper/2; upper != lower; index = lower + (upper - lower)/2)
    {
      int	i = compare(map[index]->name, map[index]->size, n, s);

      if (i < 0)
	{
	  lower = index + 1;
        }
      else if (i > 0)
	{
	  upper = index;
        }
      else
	{
	  break;
        }
    }
  if (index<map_used && compare(map[index]->name,map[index]->size,n,s) == 0)
    {
      if (debug > 2)
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "Found port %d for %.*s", map[index]->port, s, n);
	  gdomap_log(LOG_DEBUG);
	}
      return map[index];
    }
  if (debug > 2)
    {
      snprintf(ebuf, sizeof(ebuf), "Failed to find map entry for %.*s", s, n);
      gdomap_log(LOG_DEBUG);
    }
  return 0;
}

/*
 *	Name -		map_by_port()
 *	Purpose -	Search the map for an entry for a particular port
 */
static map_ent*
map_by_port(unsigned p, unsigned char t)
{
  int	index;

  if (debug > 2)
    {
      snprintf(ebuf, sizeof(ebuf), "Searching map for %u:%x", p, t);
      gdomap_log(LOG_DEBUG);
    }
  for (index = 0; index < map_used; index++)
    {
      map_ent	*e = map[index];

      if (e->port == p && (e->net | e->svc) == t)
	{
	  break;
	}
    }
  if (index < map_used)
    {
      if (debug > 2)
	{
	  snprintf(ebuf, sizeof(ebuf), "Found port %d with name %s",
		map[index]->port, map[index]->name);
	  gdomap_log(LOG_DEBUG);
	}
      return map[index];
    }
  if (debug > 2)
    {
      snprintf(ebuf, sizeof(ebuf), "Failed to find map entry for %u:%x", p, t);
      gdomap_log(LOG_DEBUG);
    }
  return 0;
}

/*
 *	Name -		map_del()
 *	Purpose -	Remove a mapping entry from the map and release
 *			the memory it uses.
 */
static void
map_del(map_ent* e)
{
  int	i;

  if (debug > 2)
    {
      snprintf(ebuf, sizeof(ebuf), "Removing port %d from map for %.*s",
	e->port, e->size, e->name);
      gdomap_log(LOG_DEBUG);
    }
  for (i = 0; i < map_used; i++)
    {
      if (map[i] == e)
	{
	  int	j;

	  free(e->name);
	  free(e);
	  for (j = i + 1; j < map_used; j++)
	    {
	      map[j-1] = map[j];
	    }
	  map_used--;
	  return;
	}
    }
}

/*
 *	Variables and functions for keeping track of the IP addresses of
 *	hosts which are running the name server.
 */
static uint32_t	prb_used = 0;
static uint32_t	prb_size = 0;
typedef struct	{
  struct in_addr	sin;
  time_t		when;
} prb_type;
static prb_type	**prb = 0;

/*
 *	Name -		prb_add()
 *	Purpose -	Create a new probe entry in the list.
 *			The new entry is always placed at the end of the list
 *			so that the list remains in the order in which hosts
 *			have been contancted.
 */
static void
prb_add(struct in_addr *p)
{
  prb_type	*n = 0;
  unsigned int	i;

  if (is_local_host(*p) != 0)
    {
      return;
    }
  if (is_local_net(*p) == 0 && is_probe_host(*p) == 0)
    {
      return;
    }

  /*
   * If we already have an entry for this address, remove it from the list
   * ready for re-insertion in the correct place.
   */
  i = prb_used;
  while (i-- > 0)
    {
      if (memcmp(&prb[i]->sin, p, IASIZE) == 0)
	{
	  n = prb[i];
	  prb_used--;
	  while (i++ < prb_used)
	    {
	      prb[i-1] = prb[i];
	    }
	  break;
	}
    }

  /*
   * Create a new entry structure if necessary.
   * Set the current time in the structure, so we know when we last had contact.
   */
  if (0 == n)
    {
      n = (prb_type*)calloc(sizeof(prb_type), 1);
      n->sin = *p;
    }
  n->when = time(0);

  /*
   * Grow the list if we need more space.
   */
  if (prb_used >= prb_size)
    {
      prb_size = prb_used + 16;
      if (prb)
	{
	  prb = (prb_type**)realloc(prb, prb_size * sizeof(prb_type*));
	}
      else
	{
	  prb = (prb_type**)calloc(prb_size * sizeof(prb_type*), 1);
	}
    }

  /*
   * Append the new item at the end of the list.
   */
  prb[prb_used++] = n;
}


/*
 *	Name -		prb_del()
 *	Purpose -	Remove an entry from the list.
 */
static void
prb_del(struct in_addr *p)
{
  unsigned int	i = prb_used;

  while (i-- > 0)
    {
      if (memcmp(&prb[i]->sin, p, IASIZE) == 0)
	{
	  free(prb[i]);
	  prb_used--;
	  while (i++ < prb_used)
	    {
	      prb[i - 1] = prb[i];
	    }
	  return;
	}
    }
}

/*
 *	Remove any server from which we have had no messages in the last
 *	thirty minutes (as long as we have sent as probe in that time).
 */
static void
prb_tim(time_t when)
{
  int	i = prb_used;

  when -= 1800;
  while (i-- > 0)
    {
      if (noprobe == 0 && prb[i]->when < when && prb[i]->when < last_probe)
	{
	  prb_del(&prb[i]->sin);
	}
    }
}

/*
 *	Name -		clear_chan()
 *	Purpose -	Release all resources associated with a channel
 *			and remove it from the list of requests being
 *			serviced.
 */
static void
clear_chan(int desc)
{
#if	defined(__MINGW__)
  if (desc != INVALID_SOCKET)
#else
  if (desc >= 0 && desc < FD_SETSIZE)
#endif
    {
      WInfo	*wi;

      FD_CLR(desc, &write_fds);
      if (desc == tcp_desc || desc == udp_desc)
	{
	  FD_SET(desc, &read_fds);
	}
      else
	{
	  FD_CLR(desc, &read_fds);
#if	defined(__MINGW__)
	  closesocket(desc);
#else
	  close(desc);
#endif
	}
      if ((wi = getWInfo(desc, 0)) != 0)
	{
	  if (wi->buf)
	    {
	      free(wi->buf);
	      wi->buf = 0;
	    }
	  wi->len = 0;
	  wi->pos = 0;
	}
      if (!(desc == tcp_desc || desc == udp_desc))
	{
	  if (wi != 0)
	    {
	      delWInfo(desc);
	    }
	  delRInfo(desc);
	}
    }
}

static void
dump_stats()
{
  int	tcp_pending = 0;
  unsigned int	i;

  for (i = 0; i < _wInfoCount; i++)
    {
      if (_wInfo[i].len > 0)
	{
	  tcp_pending++;
	}
    }
  snprintf(ebuf, sizeof(ebuf),
    "tcp messages waiting for send - %d", tcp_pending);
  gdomap_log(LOG_INFO);
  snprintf(ebuf, sizeof(ebuf),
    "udp messages waiting for send - %d", udp_pending);
  gdomap_log(LOG_INFO);
  snprintf(ebuf, sizeof(ebuf), "size of name-to-port map - %d", map_used);
  gdomap_log(LOG_INFO);
  snprintf(ebuf, sizeof(ebuf), "number of known name servers - %"PRIu32,
    prb_used);
  gdomap_log(LOG_INFO);
  snprintf(ebuf, sizeof(ebuf), "TCP %d read, %d sent", tcp_read, tcp_sent);
  gdomap_log(LOG_INFO);
  snprintf(ebuf, sizeof(ebuf), "UDP %d read, %d sent", udp_read, udp_sent);
  gdomap_log(LOG_INFO);
}

#ifndef __MINGW__
static void
dump_tables()
{
  FILE	*fptr;

  soft_int++;
  if (access(".", W_OK) != 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Failed to access gdomap.dump file for output\n");
      gdomap_log(LOG_ERR);
      return;
    }
  fptr = fopen("gdomap.dump", "w");
  if (fptr != 0)
    {
      fprintf(fptr, "\n");
      fprintf(fptr, "Known nameserver addresses\n");
      fprintf(fptr, "==========================\n");
      if (prb_used == 0)
	{
	  fprintf(fptr, "None.\n");
	}
      else
	{
	  unsigned int	i;

	  for (i = 0; i < prb_used; i++)
	    {
	      fprintf(fptr, "%16s %s\n",
		inet_ntoa(prb[i]->sin), (const char*)ctime(&prb[i]->when));
	    }
	}

      fprintf(fptr, "\n");
      fclose(fptr);
    }
  else
    {
      snprintf(ebuf, sizeof(ebuf),
	"Failed to open gdomap.dump file for output\n");
      gdomap_log(LOG_ERR);
    }
}
#endif

/*
 *	Name -		init_iface()
 *	Purpose -	Build up an array of the IP addresses supported on
 *			the network interfaces of this machine.
 */
static void
init_iface()
{
#if	defined(__MINGW__)
  INTERFACE_INFO InterfaceList[20];
  DWORD nBytesReturned;
  int i, countActive, nNumInterfaces;
  SOCKET desc = WSASocket(PF_INET, SOCK_RAW, AF_INET, 0, 0, 0);

  if (desc == INVALID_SOCKET)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Failed to get a socket. Error %d\n", WSAGetLastError());
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }

  memset((void*)InterfaceList, '\0', sizeof(InterfaceList));
  if (WSAIoctl(desc, SIO_GET_INTERFACE_LIST, 0, 0, (void*)InterfaceList,
    sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Failed WSAIoctl. Error %d\n", WSAGetLastError());
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }

  nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

  /*
   * See how many active entries there are.
   */
  countActive = 0;
  for (i = 0; i < nNumInterfaces; i++)
    {
      u_long	nFlags = InterfaceList[i].iiFlags;

      if ((nFlags & IFF_UP)
	&& (InterfaceList[i].iiAddress.Address.sa_family == AF_INET))
	{
	  countActive++;
	}
    }

  /*
   * Allocate enough space for all interfaces.
   */
  if (addr != 0) free(addr);
  addr = (struct in_addr*)malloc((countActive+1)*IASIZE);
  if (bcok != 0) free(bcok);
  bcok = (unsigned char*)malloc((countActive+1)*sizeof(unsigned char));
  if (bcst != 0) free(bcst);
  bcst = (struct in_addr*)malloc((countActive+1)*IASIZE);
  if (mask != 0) free(mask);
  mask = (struct in_addr*)malloc((countActive+1)*IASIZE);

  for (i = 0; i < nNumInterfaces; i++)
    {
      u_long	nFlags = InterfaceList[i].iiFlags;

      if ((nFlags & IFF_UP)
	&& (InterfaceList[i].iiAddress.Address.sa_family == AF_INET))
	{
	  int	broadcast = 0;
	  int	pointopoint = 0;
	  int	loopback = 0;

	  if (nFlags & IFF_BROADCAST)
	    {
	      broadcast = 1;
	    }
	  if (nFlags & IFF_POINTTOPOINT)
	    {
	      pointopoint = 1;
	    }
	  if (nFlags & IFF_LOOPBACK)
	    {
	      loopback = 1;
	    }
	  addr[interfaces] = ((struct sockaddr_in*)
	    &(InterfaceList[i].iiAddress))->sin_addr;
	  mask[interfaces] = ((struct sockaddr_in*)
	    &(InterfaceList[i].iiNetmask))->sin_addr;
	  bcst[interfaces] = ((struct sockaddr_in*)
	    &(InterfaceList[i].iiBroadcastAddress))->sin_addr;
	  bcok[interfaces] = (broadcast | pointopoint);

	  if (addr[interfaces].s_addr == 0)
	    {
	      addr[interfaces].s_addr = htonl(0x8f000001);
	      fprintf(stderr, "Bad iface addr (0.0.0.0) guess (%s)\n",
		inet_ntoa(addr[interfaces]));
	    }
	  if (mask[interfaces].s_addr == 0)
	    {
	      mask[interfaces].s_addr = htonl(0xffffff00);
	      fprintf(stderr, "Bad iface mask (0.0.0.0) guess (%s)\n",
		inet_ntoa(mask[interfaces]));
	    }
	  if (bcst[interfaces].s_addr == 0)
	    {
	      u_long	l = ntohl(addr[interfaces].s_addr);
	      bcst[interfaces].s_addr = htonl(l | 0xff);
	      fprintf(stderr, "Bad iface bcst (0.0.0.0) guess (%s)\n",
		inet_ntoa(bcst[interfaces]));
	    }
	  interfaces++;
	}
    }
  closesocket(desc);
#else
#if	defined(SIOCGIFCONF)
  struct ifconf	ifc;
  struct ifreq	ifreq;
  void		*final;
  void		*ifr_ptr;
  char		buf[MAX_IFACE * sizeof(struct ifreq)];
  int		desc;

  if ((desc = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror("socket for init_iface");
      exit(EXIT_FAILURE);
    }
// QNX seems to disagree about what it means to be SysV r4.
#if	defined(__svr4__) && !defined(__QNXNTO__)
    {
      struct strioctl	ioc;

      ioc.ic_cmd = SIOCGIFCONF;
      ioc.ic_timout = 0;
      ioc.ic_len = sizeof(buf);
      ioc.ic_dp = buf;
      if (ioctl(desc, I_STR, (char*)&ioc) < 0)
	{
	  ioc.ic_len = 0;
	}
      ifc.ifc_len = ioc.ic_len;
      ifc.ifc_buf = ioc.ic_dp;
    }
#else
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(desc, SIOCGIFCONF, (char*)&ifc) < 0)
    {
      ifc.ifc_len = 0;
    }
#endif

  /*
   *	Find the IP address of each active network interface.
   */
  if (ifc.ifc_len == 0)
    {
      int	res = errno;

      snprintf(ebuf, sizeof(ebuf),
	"SIOCGIFCONF for init_iface found no active interfaces; %s", lastErr());
      gdomap_log(LOG_ERR);

      if (res == EINVAL)
	{
	  snprintf(ebuf, sizeof(ebuf),
"Either you have too many network interfaces on your machine (in which case\n"
"you need to change the 'MAX_IFACE' constant in gdomap.c and rebuild it), or\n"
"your system is buggy, and you need to use the '-a' command line flag for\n"
"gdomap to manually set the interface addresses and masks to be used.\n"
"Try 'gdomap -C' for more information.\n");
	  gdomap_log(LOG_INFO);
	}
      close(desc);
      exit(EXIT_FAILURE);
    }
  /*
   * We cannot know the number of interfaces in advance, thus we
   * need to malloc to MAX_IFACE toensure sufficient space
   */
  if (addr != 0) free(addr);
  addr = (struct in_addr*)malloc((MAX_IFACE+1)*IASIZE);
  if (bcok != 0) free(bcok);
  bcok = (unsigned char*)malloc((MAX_IFACE+1)*sizeof(unsigned char));
  if (bcst != 0) free(bcst);
  bcst = (struct in_addr*)malloc((MAX_IFACE+1)*IASIZE);
  if (mask != 0) free(mask);
  mask = (struct in_addr*)malloc((MAX_IFACE+1)*IASIZE);

  final = &ifc.ifc_buf[ifc.ifc_len];
  for (ifr_ptr = ifc.ifc_req; ifr_ptr < final;)
    {
      ifreq = *(struct ifreq*)ifr_ptr;
#if	defined(HAVE_SA_LEN)
      ifr_ptr += sizeof(ifreq) - sizeof(ifreq.ifr_addr)
	+ ROUND(ifreq.ifr_addr.sa_len, sizeof(struct ifreq*));
#else
      ifr_ptr += sizeof(ifreq);
#endif

      if (ioctl(desc, SIOCGIFFLAGS, (char *)&ifreq) < 0)
        {
          snprintf(ebuf, sizeof(ebuf), "SIOCGIFFLAGS: %s", lastErr());
          gdomap_log(LOG_ERR);
        }
      else if (ifreq.ifr_flags & IFF_UP)
        {  /* interface is up */
	  int	broadcast = 0;
	  int	pointopoint = 0;
	  int	loopback = 0;

	  if (ifreq.ifr_flags & IFF_BROADCAST)
	    {
	      broadcast = 1;
	    }
#if	defined(IFF_POINTOPOINT)
	  if (ifreq.ifr_flags & IFF_POINTOPOINT)
	    {
	      pointopoint = 1;
	    }
#endif
#if	defined(IFF_LOOPBACK)
	  if (ifreq.ifr_flags & IFF_LOOPBACK)
	    {
	      loopback = 1;
	    }
#endif
          if (ioctl(desc, SIOCGIFADDR, (char *)&ifreq) < 0)
            {
              snprintf(ebuf, sizeof(ebuf), "SIOCGIFADDR: %s", lastErr());
              gdomap_log(LOG_ERR);
            }
          else if (ifreq.ifr_addr.sa_family == AF_INET)
            {	/* IP interface */
	      if (interfaces >= MAX_IFACE)
	        {
	          snprintf(ebuf, sizeof(ebuf),
"You have too many network interfaces on your machine (in which case you need\n"
"to change the 'MAX_IFACE' constant in gdomap.c and rebuild it), or your\n"
"system is buggy, and you need to use the '-a' command line flag for\n"
"gdomap to manually set the interface addresses and masks to be used.\n"
"Try 'gdomap -C' for more information.\n");
	          gdomap_log(LOG_INFO);
		  close(desc);
	          exit(EXIT_FAILURE);
	        }
	      addr[interfaces] =
		((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
	      bcok[interfaces] = (broadcast | pointopoint);
#if	defined(IFF_POINTOPOINT)
	      if (pointopoint)
		{
		  if (ioctl(desc, SIOCGIFDSTADDR, (char*)&ifreq) < 0)
		    {
		      snprintf(ebuf, sizeof(ebuf),
			"SIOCGIFADDR: %s", lastErr());
		      gdomap_log(LOG_ERR);
		      bcok[interfaces] = 0;
		    }
		  else
		    {
		      bcst[interfaces]
			= ((struct sockaddr_in *)&ifreq.ifr_dstaddr)->sin_addr;
		    }
		}
	      else
#endif
		{
		  if (!loopback
		    && ioctl(desc, SIOCGIFBRDADDR, (char*)&ifreq) < 0)
		    {
		      snprintf(ebuf, sizeof(ebuf),
			"SIOCGIFBRDADDR: %s", lastErr());
		      gdomap_log(LOG_ERR);
		      bcok[interfaces] = 0;
		    }
		  else
		    {
		      bcst[interfaces]
			= ((struct sockaddr_in*)&ifreq.ifr_broadaddr)->sin_addr;
		    }
		}
	      if (ioctl(desc, SIOCGIFNETMASK, (char *)&ifreq) < 0)
	        {
		  snprintf(ebuf, sizeof(ebuf), "SIOCGIFNETMASK: %s", lastErr());
		  gdomap_log(LOG_ERR);
		  /*
		   *	If we can't get a netmask - assume a class-c
		   *	network.
		   */
		   mask[interfaces] = class_c_mask;
		}
              else
                {
/*
 *	Some systems don't have ifr_netmask
 */
#ifdef	ifr_netmask
                  mask[interfaces] =
                    ((struct sockaddr_in *)&ifreq.ifr_netmask)->sin_addr;
#else
                  mask[interfaces] =
                    ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
#endif
                }
              interfaces++;
	    }
	}
    }
  close(desc);
#endif	/* SIOCGIFCONF */
#endif	/* MINGW */

  if (interfaces == 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"I can't find any network interfaces on this platform - "
	"use the '-a' flag to load interface details from a file instead.\n"
	"Try 'gdomap -C' for more information.\n");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
}

/*
 *	Name -		load_iface()
 *	Purpose -	Read addresses and netmasks for interfaces on this
 *			machine from a file.
 */
static void
load_iface(const char* from)
{
  FILE	*fptr;
  char	buf[128];
  int	line = 0;
  int	num_iface = 0;

  in_config = 1;

  /* Can we open the fiel with current privs
   */
  fptr = fopen(from, "rt");
  if (fptr == 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Unable to open address config - '%s'", from);
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
  /* Should we be opening it?
   */
  if (access(from, R_OK) != 0)
    {
      fclose(fptr);
      snprintf(ebuf, sizeof(ebuf),
	"Unable to access address config - '%s'", from);
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }

  while (fgets(buf, sizeof(buf), fptr) != 0)
    {
      char	*ptr = buf;

      line++;
      /*
       *	Strip leading white space.
       */
      while (isspace(*ptr))
	{
	  ptr++;
	}
      if (ptr != buf)
	{
	  memmove(buf, ptr, strlen(ptr) + 1);
	}
      /*
       *	Strip comments.
       */
      ptr = strchr(buf, '#');
      if (ptr)
	{
	  *ptr = '\0';
	}
      /*
       *	Strip trailing white space.
       */
      ptr = buf;
      while (*ptr)
	{
	  ptr++;
	}
      while (ptr > buf && isspace(ptr[-1]))
	{
	  ptr--;
	}
      *ptr = '\0';
      /*
       *	Ignore blank lines.
       */
      if (*buf == '\0')
	{
	  continue;
	}
      if (num_iface++ > 1000)
	{
          snprintf(ebuf, sizeof(ebuf), "Too many network interfaces found");
          gdomap_log(LOG_CRIT);
          exit(EXIT_FAILURE);
	}
    }
  fseek(fptr, 0, 0);

  if (num_iface == 0)
    {
      snprintf(ebuf, sizeof(ebuf), "No network interfaces found");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
  num_iface++;
  addr = (struct in_addr*)malloc((num_iface+1)*IASIZE);
  mask = (struct in_addr*)malloc((num_iface+1)*IASIZE);
  bcok = (unsigned char*)malloc((num_iface+1)*sizeof(unsigned char));
  bcst = (struct in_addr*)malloc((num_iface+1)*IASIZE);

  addr[interfaces].s_addr = inet_addr("127.0.0.1");
  mask[interfaces].s_addr = inet_addr("255.255.255.0");
  bcok[interfaces] = 0;
  bcst[interfaces].s_addr = inet_addr("127.0.0.255");
  interfaces++;

  while (fgets(buf, sizeof(buf), fptr) != 0)
    {
      char	*ptr = buf;
      char	*msk;

      line++;
      /*
       *	Strip leading white space.
       */
      while (isspace(*ptr))
	{
	  ptr++;
	}
      if (ptr != buf)
	{
	  memmove(buf, ptr, strlen(ptr) + 1);
	}
      /*
       *	Strip comments.
       */
      ptr = strchr(buf, '#');
      if (ptr)
	{
	  *ptr = '\0';
	}
      /*
       *	Strip trailing white space.
       */
      ptr = buf;
      while (*ptr)
	{
	  ptr++;
	}
      while (ptr > buf && isspace(ptr[-1]))
	{
	  ptr--;
	}
      *ptr = '\0';
      /*
       *	Ignore blank lines.
       */
      if (*buf == '\0')
	{
	  continue;
	}

      ptr = buf;
      while (*ptr && (isdigit(*ptr) || (*ptr == '.')))
	{
	  ptr++;
	}
      while (isspace(*ptr))
	{
	  *ptr++ = '\0';
	}
      msk = ptr;
      while (*ptr && (isdigit(*ptr) || (*ptr == '.')))
	{
	  ptr++;
	}
      while (isspace(*ptr))
	{
	  *ptr++ = '\0';
	}
      addr[interfaces].s_addr = inet_addr(buf);
      mask[interfaces].s_addr = inet_addr(msk);
      if (isdigit(*ptr))
	{
	  bcok[interfaces] = 1;
	  bcst[interfaces].s_addr = inet_addr(ptr);
	}
      else
	{
	  bcok[interfaces] = 0;
	  bcst[interfaces].s_addr = inet_addr("0.0.0.0");
	}
      if (addr[interfaces].s_addr == (uint32_t)-1)
	{
	  snprintf(ebuf, sizeof(ebuf), "line %d of '%s' bad address (%s)",
	    line, from, buf);
	  gdomap_log(LOG_CRIT);
	  exit(EXIT_FAILURE);
	}
      else if (mask[interfaces].s_addr == (uint32_t)-1)
	{
	  snprintf(ebuf, sizeof(ebuf), "line %d of '%s' bad netmask (%s)",
	    line, from, ptr);
	  gdomap_log(LOG_CRIT);
	  exit(EXIT_FAILURE);
	}
      else
	{
	  interfaces++;
	}
    }
  fclose(fptr);
  in_config = 0;
}

/*
 *	Name -		init_my_port()
 *	Purpose -	Establish our well-known port (my_port).
 */
static void
init_my_port()
{
  /*
   *	First we determine the port for the 'gdomap' service - ideally
   *	this should be the default port, since we should have registered
   *	this with the appropriate authority and have it reserved for us.
   */
#if	defined(GDOMAP_PORT_OVERRIDE)
  my_port = htons(GDOMAP_PORT_OVERRIDE);
#else
  struct servent	*sp;

  my_port = htons(GDOMAP_PORT);
  if ((sp = getservbyname("gdomap", "tcp")) == 0)
    {
      if (debug)
	{
	  snprintf(ebuf, sizeof(ebuf), "Unable to find service 'gdomap'");
	  gdomap_log(LOG_WARNING);
	  snprintf(ebuf, sizeof(ebuf),
	    "On a unix host it should be in /etc/services "
	    "as 'gdomap %d/tcp' and 'gdomap %d/udp'\n",
	    GDOMAP_PORT, GDOMAP_PORT);
	  gdomap_log(LOG_INFO);
	}
    }
  else
    {
      unsigned short	tcp_port = sp->s_port;

      if ((sp = getservbyname("gdomap", "udp")) == 0)
	{
	  if (debug)
	    {
	      snprintf(ebuf, sizeof(ebuf), "Unable to find service 'gdomap'");
	      snprintf(ebuf, sizeof(ebuf),
		"On a unix host it should be in /etc/services "
		"as 'gdomap %d/tcp' and 'gdomap %d/udp'\n",
		GDOMAP_PORT, GDOMAP_PORT);
	      gdomap_log(LOG_INFO);
	    }
	}
      else if (sp->s_port != tcp_port)
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "UDP and TCP service entries differ. "
	    "Using the TCP entry for both!");
	  gdomap_log(LOG_WARNING);
	}
      if (tcp_port != my_port)
	{
	  snprintf(ebuf, sizeof(ebuf), "gdomap not running on normal port");
	  gdomap_log(LOG_WARNING);
	}
      my_port = tcp_port;
    }
#endif
}

/*
 *	Name -		init_ports()
 *	Purpose -	Set up the ports for accepting incoming requests.
 */
static void
init_ports()
{
  int		r;
  struct sockaddr_in	sa;
#if	defined(__MINGW__)
  DWORD dummy;
#endif /* __MINGW__ */

  /*
   *	Now we set up the sockets to accept incoming connections and set
   *	options on it so that if this program is killed, we can restart
   *	immediately and not find the socket addresses hung.
   */

  udp_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#if	defined(__MINGW__)
  if (udp_desc == INVALID_SOCKET)
#else
  if (udp_desc < 0)
#endif
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to create UDP socket");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
  else if (debug)
    {
      snprintf(ebuf, sizeof(ebuf), "Opened UDP socket %d", udp_desc);
      gdomap_log(LOG_DEBUG);
    }
#if	0 && defined(SO_EXCLUSIVEADDRUSE)
  r = 1;
  if ((setsockopt(udp_desc,SOL_SOCKET,SO_REUSEADDR,(char*)&r,sizeof(r)))<0)
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to set 're-use' on UDP socket");
      gdomap_log(LOG_WARNING);
    }
  r = 1;
  if ((setsockopt
    (udp_desc,SOL_SOCKET,SO_EXCLUSIVEADDRUSE,(char*)&r,sizeof(r)))<0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Unable to set 'exclusive-use' on UDP socket");
      gdomap_log(LOG_WARNING);
    }
#endif
#ifndef BROKEN_SO_REUSEADDR
  /*
   * Under decent systems, SO_REUSEADDR means that the port can be reused
   * immediately that this process exits.  Under some it means
   * that multiple processes can serve the same port simultaneously.
   * We don't want that broken behavior!
   */
  r = 1;
  if ((setsockopt(udp_desc,SOL_SOCKET,SO_REUSEADDR,(char*)&r,sizeof(r)))<0)
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to set 're-use' on UDP socket");
      gdomap_log(LOG_WARNING);
    }
#endif
  if (nobcst == 0)
    {
      r = 1;
      if ((setsockopt(udp_desc,SOL_SOCKET,SO_BROADCAST,(char*)&r,sizeof(r)))<0)
	{
	  nobcst++;
	  snprintf(ebuf, sizeof(ebuf), "Unable to use 'broadcast' for probes");
	  gdomap_log(LOG_WARNING);
	}
    }
#if	defined(__MINGW__)
  dummy = 1;
  if (ioctlsocket(udp_desc, FIONBIO, &dummy) < 0)
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to handle UDP socket non-blocking");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
#else /* !__MINGW__ */
  if ((r = fcntl(udp_desc, F_GETFL, 0)) >= 0)
    {
      r |= NBLK_OPT;
      if (fcntl(udp_desc, F_SETFL, r) < 0)
	{
	  snprintf(ebuf, sizeof(ebuf), "Unable to set UDP socket non-blocking");
	  gdomap_log(LOG_CRIT);
	  exit(EXIT_FAILURE);
	}
    }
  else
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to handle UDP socket non-blocking");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
#endif
 /*
   *	Now we bind our address to the socket and prepare to accept incoming
   *	connections by listening on it.
   */
  memset(&sa, '\0', sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = my_port;
  if (bind(udp_desc, (void*)&sa, sizeof(sa)) < 0)
    {
      snprintf(ebuf, sizeof(ebuf),
"Unable to bind address to UDP socket. Perhaps gdomap is already running");
      gdomap_log(LOG_ERR);
      if (errno == EACCES)
	{
	  snprintf(ebuf, sizeof(ebuf),
"You probably need to run gdomap as root/system administrator (recommended),\n"
"or run the nameserver on a non-standard port that does not require root\n"
"privilege (poor option and last resort!!!)");
	  gdomap_log(LOG_INFO);
	}
      exit(EXIT_FAILURE);
    }

  /*
   *	Now we do the TCP socket.
   */
  tcp_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#if	defined(__MINGW__)
  if (tcp_desc == INVALID_SOCKET)
#else
  if (tcp_desc < 0)
#endif
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to create TCP socket");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
  else if (debug)
    {
      snprintf(ebuf, sizeof(ebuf), "Opened TDP socket %d", tcp_desc);
      gdomap_log(LOG_DEBUG);
    }
#if	0 && defined(SO_EXCLUSIVEADDRUSE)
  r = 1;
  if ((setsockopt(tcp_desc,SOL_SOCKET,SO_REUSEADDR,(char*)&r,sizeof(r)))<0)
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to set 're-use' on TCP socket");
      gdomap_log(LOG_WARNING);
    }
  r = 1;
  if ((setsockopt
    (tcp_desc,SOL_SOCKET,SO_EXCLUSIVEADDRUSE,(char*)&r,sizeof(r)))<0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Unable to set 'exclusive-use' on TCP socket");
      gdomap_log(LOG_WARNING);
    }
#endif
#ifndef BROKEN_SO_REUSEADDR
  /*
   * Under decent systems, SO_REUSEADDR means that the port can be reused
   * immediately that this process exits.  Under some it means
   * that multiple processes can serve the same port simultaneously.
   * We don't want that broken behavior!
   */
  r = 1;
  if ((setsockopt(tcp_desc,SOL_SOCKET,SO_REUSEADDR,(char*)&r,sizeof(r)))<0)
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to set 're-use' on TCP socket");
      gdomap_log(LOG_WARNING);
    }
#endif

#if	defined(__MINGW__)
  dummy = 1;
  if (ioctlsocket(tcp_desc, FIONBIO, &dummy) < 0)
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to handle TCP socket non-blocking");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
#else /* !__MINGW__ */
  if ((r = fcntl(tcp_desc, F_GETFL, 0)) >= 0)
    {
      r |= NBLK_OPT;
      if (fcntl(tcp_desc, F_SETFL, r) < 0)
	{
	  snprintf(ebuf, sizeof(ebuf), "Unable to set TCP socket non-blocking");
	  gdomap_log(LOG_CRIT);
	  exit(EXIT_FAILURE);
	}
    }
  else
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to handle TCP socket non-blocking");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
#endif /* __MINGW__ */

  memset(&sa, '\0', sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = my_port;
  if (bind(tcp_desc, (void*)&sa, sizeof(sa)) < 0)
    {
      snprintf(ebuf, sizeof(ebuf),
"Unable to bind address to TCP socket. Perhaps gdomap is already running");
      gdomap_log(LOG_ERR);
      if (errno == EACCES)
	{
	  snprintf(ebuf, sizeof(ebuf),
"You probably need to run gdomap as root/system administrator (recommended),\n"
"or run the nameserver on a non-standard port that does not require root\n"
"privilege (poor option and last resort!!!)");
	  gdomap_log(LOG_INFO);
	}
      exit(EXIT_FAILURE);
    }
  if (listen(tcp_desc, QUEBACKLOG) < 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Unable to listen for connections on TCP socket");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }

  /*
   *	Set up masks to say we are interested in these descriptors.
   */
  memset(&read_fds, '\0', sizeof(read_fds));
  memset(&write_fds, '\0', sizeof(write_fds));

  getRInfo(tcp_desc, 1);
  getRInfo(udp_desc, 1);

  FD_SET(tcp_desc, &read_fds);
  FD_SET(udp_desc, &read_fds);

#ifndef __MINGW__
  /*
   *	Turn off pipe signals so we don't get interrupted if we attempt
   *	to write a response to a process which has died.
   */
  signal(SIGPIPE, SIG_IGN);
  /*
   *	Enable table dumping to /tmp/gdomap.dump
   */
  signal(SIGUSR1, dump_tables);
#endif /* !__MINGW__  */
}


static int
other_addresses_on_net(struct in_addr old, struct in_addr **extra)
{
  int	numExtra = 0;
  int	iface;

  for (iface = 0; iface < interfaces; iface++)
    {
      if (addr[iface].s_addr == old.s_addr)
	{
	  continue;
	}
      if ((addr[iface].s_addr & mask[iface].s_addr) ==
	    (old.s_addr & mask[iface].s_addr))
	{
	  numExtra++;
	}
    }
  if (numExtra > 0)
    {
      struct in_addr	*addrs;

      addrs = (struct in_addr*)malloc(sizeof(struct in_addr)*numExtra);
      *extra = addrs;
      numExtra = 0;

      for (iface = 0; iface < interfaces; iface++)
	{
	  if (addr[iface].s_addr == old.s_addr)
	    {
	      continue;
	    }
	  if ((addr[iface].s_addr & mask[iface].s_addr) ==
		(old.s_addr & mask[iface].s_addr))
	    {
	      addrs[numExtra].s_addr = addr[iface].s_addr;
	      numExtra++;
	    }
	}
    }
  return numExtra;
}


/*
 *	Name -		init_probe()
 *	Purpose -	Send a request to all hosts on the local network
 *			to see if there is a name server running on them.
 */
static void
init_probe()
{
  uint32_t nlist[interfaces];
  int	nlist_size = 0;
  int	iface;
  int	i;

  if (noprobe > 0)
    {
      return;
    }
  if (debug > 2)
    {
      snprintf(ebuf, sizeof(ebuf), "Initiating probe requests.");
      gdomap_log(LOG_DEBUG);
    }

  /*
   *	Make a list of the different networks to which we must send.
   */
  for (iface = 0; iface < interfaces; iface++)
    {
      uint32_t	net = (addr[iface].s_addr & mask[iface].s_addr);

      if (addr[iface].s_addr == loopback.s_addr)
	{
	  continue;		/* Skip loopback	*/
	}
      for (i = 0; i < nlist_size; i++)
	{
	  if (net == nlist[i])
	    {
	      break;
	    }
	}
      if (i == nlist_size)
	{
	  nlist[i] = net;
	  nlist_size++;
	}
    }

  for (i = 0; i < nlist_size; i++)
    {
      int		broadcast = 0;
      int		elen = 0;
      struct in_addr	*other = 0;
      struct in_addr	sin = { 0 };
      int		high = 0;
      int		low = 0;
      uint32_t  	net = 0;
      int		j;
      struct in_addr	b;

      /*
       *	Build up a list of addresses that we serve on this network.
       */
      for (iface = 0; iface < interfaces; iface++)
	{
	  if ((addr[iface].s_addr & mask[iface].s_addr) == nlist[i])
	    {
	      sin = addr[iface];
	      if (bcok[iface])
		{
		  /*
		   * Simple broadcast for this address.
		   */
		  b.s_addr = bcst[iface].s_addr;
		  broadcast = 1;
		}
	      else
		{
		  uint32_t ha;		/* full host address.	*/
		  uint32_t hm;		/* full netmask.	*/

		  ha = ntohl(addr[iface].s_addr);
		  hm = ntohl(mask[iface].s_addr);

		  /*
		   *	Make sure that our netmasks are restricted
		   *	to class-c networks and subnets of those
		   *	networks - we don't want to be probing
		   *	more than a couple of hundred hosts!
		   */
		  if ((mask[iface].s_addr | class_c_mask.s_addr)
			!= mask[iface].s_addr)
		    {
		      snprintf(ebuf, sizeof(ebuf), "netmask %s will be "
			"treated as 255.255.255.0 for ",
			inet_ntoa(mask[iface]));
		      strncat(ebuf, inet_ntoa(addr[iface]),
			sizeof(ebuf) - strlen(ebuf) - 1);
		      gdomap_log(LOG_WARNING);
		      hm |= ~255;
		    }
		  net = ha & hm & ~255;		/* class-c net number.	*/
		  low = ha & hm & 255;		/* low end of subnet.	*/
		  high = low | (255 & ~hm);	/* high end of subnet.	*/
		  elen = other_addresses_on_net(sin, &other);
		}
	      break;
	    }
	}

      if (plist)
	{
	  plentry	*p;

	  /*
	   *	Now start probes for servers on machines in our probe config
	   *	list for which we have a direct connection.
	   */
	  for (p = plist; p != 0; p = p->next)
	    {
	      if ((p->addr.s_addr & mask[iface].s_addr) ==
		    (addr[iface].s_addr & mask[iface].s_addr))
		{
		  unsigned int	len = elen;

		  p->direct = 1;
		  /* Kick off probe.	*/
		  if (is_local_host(p->addr))
		    {
		      continue;	/* Don't probe self.	*/
		    }
		  while (len > MAX_EXTRA)
		    {
		      len -= MAX_EXTRA;
		      queue_probe(&p->addr, &sin, MAX_EXTRA, &other[len], 0);
		    }
		  queue_probe(&p->addr, &sin, len, other, 0);
		}
	    }
	}
      else if (broadcast)
	{
	  /*
	   *	Now broadcast probe on this network.
	   */
	  queue_probe(&b, &sin, 0, 0, 0);
	}
      else
	{
	  /*
	   *	Now start probes for servers on machines which may be on
	   *	any network for which we have an interface.
	   *
	   *	Assume 'low' and 'high' are not valid host addresses as 'low'
	   *	is the network address and 'high' is the broadcast address.
	   */
	  for (j = low + 1; j < high; j++)
	    {
	      struct in_addr	a;
	      unsigned int	len = elen;

	      a.s_addr = htonl(net + j);
	      if (is_local_host(a))
		{
		  continue;	/* Don't probe self - that's silly.	*/
		}
	      /* Kick off probe.	*/
	      while (len > MAX_EXTRA)
		{
		  len -= MAX_EXTRA;
		  queue_probe(&a, &sin, MAX_EXTRA, &other[len], 0);
		}
	      queue_probe(&a, &sin, len, other, 0);
	    }
	}

      if (other != NULL)
	{
	  free(other);
	}
    }

  if (plist)
    {
      plentry	*p;
      int	indirect = 0;

      /*
       *	Are there any hosts for which we do not have a direct
       *	network connection, and to which we have therefore not
       *	queued a probe?
       */
      for (p = plist; p != 0; p = p->next)
	{
	  if (p->direct == 0)
	    {
	      indirect = 1;
	    }
	}
      if (indirect)
	{
	  struct in_addr	*other = 0;
	  int			elen;

	  /*
	   *	Queue probes for indirect connections to hosts from our
	   *	primary interface and let the routing system handle it.
	   */
	  elen = other_addresses_on_net(addr[0], &other);
	  for (p = plist; p != 0; p = p->next)
	    {
	      if (p->direct == 0)
		{
		  unsigned int	len = elen;

		  if (is_local_host(p->addr))
		    {
		      continue;	/* Don't probe self.	*/
		    }
		  /* Kick off probe.	*/
		  while (len > MAX_EXTRA)
		    {
		      len -= MAX_EXTRA;
		      queue_probe(&p->addr, addr, MAX_EXTRA, &other[len], 0);
		    }
		  queue_probe(&p->addr, addr, len, other, 0);
		}
	    }
	  if (0 != other)
	    {
	      free(other);
	    }
	}
    }

  if (debug > 2)
    {
      snprintf(ebuf, sizeof(ebuf), "Probe requests initiated.");
      gdomap_log(LOG_DEBUG);
    }
  last_probe = time(0);
}

/*
 *	Name -		handle_accept()
 *	Purpose -	Handle an incoming connection, setting up resources
 *			for the request. Ensure that the channel is in
 *			non-blocking mode so that we can't hang.
 */
static void
handle_accept()
{
  struct sockaddr_in	sa;
  socklen_t		len = sizeof(sa);
  int			desc;

  memset(&sa, '\0', len);
  desc = accept(tcp_desc, (void*)&sa, &len);
  if (desc >= 0)
    {
      RInfo		*ri;
#if	defined(__MINGW__)
      DWORD	dummy = 1;
#else
      int		r;
#endif /* !__MINGW__ */

      FD_SET(desc, &read_fds);
      ri = getRInfo(desc, 1);
      ri->pos = 0;
      memcpy((char*)&ri->addr, (char*)&sa, sizeof(sa));

      if (debug)
	{
	  snprintf(ebuf, sizeof(ebuf), "accept from %s(%d) to chan %d",
	    inet_ntoa(sa.sin_addr), ntohs(sa.sin_port), desc);
	  gdomap_log(LOG_DEBUG);
	}
      /*
       *	Ensure that the connection is non-blocking.
       */
#if	defined(__MINGW__)
      if (ioctlsocket(desc, FIONBIO, &dummy) < 0)
	{
	  if (debug)
	    {
	      snprintf(ebuf, sizeof(ebuf),
		"failed to set chan %d non-blocking", desc);
	      gdomap_log(LOG_DEBUG);
	    }
	  clear_chan(desc);
	}
#else /* !__MINGW__ */
      if ((r = fcntl(desc, F_GETFL, 0)) >= 0)
	{
	  r |= NBLK_OPT;
	  if (fcntl(desc, F_SETFL, r) < 0)
	    {
	      if (debug)
		{
		  snprintf(ebuf, sizeof(ebuf),
		    "failed to set chan %d non-blocking", desc);
		  gdomap_log(LOG_DEBUG);
		}
	      clear_chan(desc);
	    }
	}
      else
	{
	  if (debug)
	    {
	      snprintf(ebuf, sizeof(ebuf),
		"failed to set chan %d non-blocking", desc);
	      gdomap_log(LOG_DEBUG);
	    }
	  clear_chan(desc);
	}
#endif /* __MINGW__ */
    }
  else if (debug)
    {
      snprintf(ebuf, sizeof(ebuf), "accept failed - errno %d",
#if	defined(__MINGW__)
	WSAGetLastError());
#else
	errno);
#endif /* __MINGW__ */
      gdomap_log(LOG_DEBUG);
    }
}

/*
 *	Name -		handle_io()
 *	Purpose -	Main loop to handle I/O on multiple simultaneous
 *			connections.  All non-blocking stuff.
 */
static void
handle_io()
{
  struct timeval timeout;
  void		*to;
  int		rval = 0;
  int		i;
  fd_set	rfds;
  fd_set	wfds;

  while (rval >= 0)
    {
      rfds = read_fds;
      wfds = write_fds;
      to = 0;

      /*
       *	If there is anything waiting to be sent on the UDP socket
       *	we must check to see if it is writable.
       */
      if (u_queue != 0)
	{
	  FD_SET(udp_desc, &wfds);
	}

      timeout.tv_sec = 10;
      timeout.tv_usec = 0;
      to = &timeout;
      soft_int = 0;
      rval = select(FD_SETSIZE, &rfds, &wfds, 0, to);

      if (rval < 0)
	{
	  /*
	   *	Let's handle any error return.
	   */
	  if (errno == EBADF)
	    {
	      fd_set	efds;

	      /*
	       *	Almost certainly lost a connection - try each
	       *	descriptor in turn to see which one it is.
	       *	Remove descriptor from bitmask and close it.
	       *	If the error is on the listener socket we die.
	       */
	      memset(&efds, '\0', sizeof(efds));
	      for (i = 0; i < FD_SETSIZE; i++)
		{
		  if (FD_ISSET(i, &rfds) || FD_ISSET(i, &wfds))
		    {
		      FD_SET(i, &efds);
		      timeout.tv_sec = 0;
		      timeout.tv_usec = 0;
		      to = &timeout;
		      rval = select(FD_SETSIZE, &efds, 0, 0, to);
		      FD_CLR(i, &efds);
		      if (rval < 0 && errno == EBADF)
			{
			  clear_chan(i);
			  if (i == tcp_desc || i == udp_desc)
			    {
			      snprintf(ebuf, sizeof(ebuf),
				"Fatal error on socket.");
			      gdomap_log(LOG_CRIT);
			      exit(EXIT_FAILURE);
			    }
			}
		    }
		}
	      rval = 0;
	    }
	  else if (soft_int > 0)
	    {
	      /*
	       * We were interrupted - but it was one we were expecting.
	       */
	      rval = 0;
	    }
	  else if (errno == EINTR)
            {
              rval = 0;
            }
	  else
	    {
	      snprintf(ebuf, sizeof(ebuf),
		"Interrupted in select: %s",strerror(errno));
	      gdomap_log(LOG_CRIT);
	      exit(EXIT_FAILURE);
	    }
	}
      else if (rval == 0)
	{
	  long		now = time(0);

	  /*
	   *	Let's handle a timeout.
	   */
	  prb_tim(now);	/* Remove dead servers	*/
	  if (udp_pending == 0 && (now - last_probe) >= interval)
	    {
	      /*
	       *	If there is no output pending on the udp channel and
	       *	it is at least five minutes since we sent out a probe
	       *	we can re-probe the network for other name servers.
	       */
	      init_probe();
	    }
	}
      else
	{
	  /*
	   *	Got some descriptor activity - deal with it.
	   */
#if	defined(__MINGW__)
	  /* read file descriptors */
	  for (i = 0; i < rfds.fd_count; i++)
	    {
	      if (rfds.fd_array[i] == tcp_desc)
		{
		  handle_accept();
		}
	      else if (rfds.fd_array[i] == udp_desc)
		{
		  handle_recv();
		}
	      else
		{
		  handle_read(rfds.fd_array[i]);
		}
	      if (debug > 2)
		{
		  dump_stats();
		}
	    }
	  for (i = 0; i < wfds.fd_count; i++)
	    {
	      if (wfds.fd_array[i] == udp_desc)
		{
		  handle_send();
		}
	      else
		{
		  handle_write(wfds.fd_array[i]);
		}
	    }
#else /* !__MINGW__ */
	  for (i = 0; i < FD_SETSIZE; i++)
	    {
	      if (FD_ISSET(i, &rfds))
		{
		  if (i == tcp_desc)
		    {
		      handle_accept();
		    }
		  else if (i == udp_desc)
		    {
		      handle_recv();
		    }
		  else
		    {
		      handle_read(i);
		    }
		  if (debug > 2)
		    {
		      dump_stats();
		    }
		}
              /* Look for a descriptor found to be writeable and which
               * was not closed (due to reading eof etc)
               */
	      if (FD_ISSET(i, &wfds)
                && (FD_ISSET(i, &write_fds) || i == udp_desc))
		{
		  if (i == udp_desc)
		    {
		      handle_send();
		    }
		  else
		    {
		      handle_write(i);
		    }
		}
	    }
#endif /* __MINGW__ */
	}
    }
}

/*
 *	Name -		handle_read()
 *	Purpose -	Read a request from a channel.  This may be called in
 *			many stages if the read is blocking.
 */
static void
handle_read(int desc)
{
  RInfo	*ri;
  uptr	ptr;
  int	nothingRead = 1;
  int	done = 0;
  int	r;

  ri = getRInfo(desc, 0);
  if (0 == ri)
    {
      snprintf(ebuf, sizeof(ebuf),
	"request not found on descriptor %d", desc);
      gdomap_log(LOG_DEBUG);
      return;
    }
  ptr = ri->buf.b;

  while (ri->pos < GDO_REQ_SIZE && done == 0)
    {
#if	defined(__MINGW__)
      r = recv(desc, (char *)&ptr[ri->pos], GDO_REQ_SIZE - ri->pos, 0);
#else
      r = read(desc, &ptr[ri->pos], GDO_REQ_SIZE - ri->pos);
#endif
      if (r > 0)
	{
	  nothingRead = 0;
	  ri->pos += r;
	}
      else
	{
	  done = 1;
	}
    }
  if (ri->pos == GDO_REQ_SIZE)
    {
      tcp_read++;
      handle_request(desc);
    }
#if	defined(__MINGW__)
  else if (WSAGetLastError() != WSAEWOULDBLOCK || nothingRead == 1)
#else
  else if (errno != EWOULDBLOCK || nothingRead == 1)
#endif
    {
      /*
       *	If there is an error or end-of-file on the descriptor then
       *	we must close it down.
       */
      clear_chan(desc);
    }
}

/*
 *	Name -		handle_recv()
 *	Purpose -	Read a request from the UDP socket.
 */
static void
handle_recv()
{
  RInfo	*ri;
  uptr	ptr;
  struct sockaddr_in* addr;
  socklen_t len = sizeof(struct sockaddr_in);
  int	r;

  ri = getRInfo(udp_desc, 0);
  if (0 == ri)
    {
      snprintf(ebuf, sizeof(ebuf),
	"request not found on descriptor %d", udp_desc);
      gdomap_log(LOG_DEBUG);
      return;
    }
  addr = &(ri->addr);
  ptr = ri->buf.b;

  r = recvfrom(udp_desc, (char *)ptr, GDO_REQ_SIZE, 0, (void*)addr, &len);
  if (r == GDO_REQ_SIZE)
    {
      udp_read++;
      ri->pos = GDO_REQ_SIZE;
      if (debug)
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "recvfrom %s", inet_ntoa(addr->sin_addr));
	  gdomap_log(LOG_DEBUG);
	}
      if (is_local_host(addr->sin_addr) == 1)
	{
	  if (debug)
	    {
	      snprintf(ebuf, sizeof(ebuf),
		"recvfrom packet from self discarded");
	      gdomap_log(LOG_DEBUG);
	    }
	  return;
	}
      handle_request(udp_desc);
    }
  else
    {
      if (debug)
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "recvfrom returned %d - %s", r, lastErr());
	  gdomap_log(LOG_DEBUG);
	}
    }
}

/*
 *	Name -		handle_request()
 *	Purpose -	Once we have read a full request, we come here
 *			to take action depending on the request type.
 */
static void
handle_request(int desc)
{
  RInfo		*ri;
  WInfo		*wi;
  unsigned char	type;
  unsigned char	size;
  unsigned char	ptype;
  uint32_t	port;
  unsigned char	*buf;
  map_ent	*m;

  ri = getRInfo(desc, 0);
  if (0 == ri)
    {
      snprintf(ebuf, sizeof(ebuf), "request not found on descriptor %d", desc);
      gdomap_log(LOG_DEBUG);
      return;
    }
  type = ri->buf.r.rtype;
  size = ri->buf.r.nsize;
  ptype = ri->buf.r.ptype;
  port = ntohl(ri->buf.r.port);
  buf = (unsigned char*)ri->buf.r.name;

  FD_CLR(desc, &read_fds);
  FD_SET(desc, &write_fds);

  if (debug > 1)
    {
      if (desc == udp_desc)
	{
	  snprintf(ebuf, sizeof(ebuf), "request type '%c' on UDP chan", type);
	  gdomap_log(LOG_DEBUG);
	}
      else
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "request type '%c' from chan %d", type, desc);
	  gdomap_log(LOG_DEBUG);
	}
      if (type == GDO_PROBE || type == GDO_PREPLY || type == GDO_SERVERS
	|| type == GDO_NAMES)
	{
	  /* fprintf(stderr, "\n"); */
	}
      else
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "  name: '%.*s' port: %"PRIu32, size, buf, port);
	  gdomap_log(LOG_DEBUG);
	}
    }

  wi = getWInfo(desc, 1);
  wi->pos = 0;

  if (ptype != GDO_TCP_GDO && ptype != GDO_TCP_FOREIGN
    && ptype != GDO_UDP_GDO && ptype != GDO_UDP_FOREIGN)
    {
      if (ptype != 0 || (type != GDO_PROBE && type != GDO_PREPLY
	&& type != GDO_SERVERS && type != GDO_NAMES))
	{
	  if (debug)
	    {
	      snprintf(ebuf, sizeof(ebuf), "Illegal port type in request");
	      gdomap_log(LOG_DEBUG);
	    }
	  clear_chan(desc);
	  return;
	}
    }

  /*
   *	The default return value is a four byte number set to zero.
   *	We assume that malloc returns data aligned on a 4 byte boundary.
   */
  wi->len = 4;
  wi->buf = (char*)malloc(4);
  wi->buf[0] = 0;
  wi->buf[1] = 0;
  wi->buf[2] = 0;
  wi->buf[3] = 0;

  if (type == GDO_REGISTER)
    {
      /*
       *	See if this is a request from a local process.
       */
      if (is_local_host(ri->addr.sin_addr) == 0)
	{
	  snprintf(ebuf, sizeof(ebuf), "Illegal attempt to register!");
	  gdomap_log(LOG_ERR);
	  clear_chan(desc);		/* Only local progs may register. */
	  return;
	}

      /*
       *	What should we do if we already have the name registered?
       *	Simple algorithm -
       *		We check to see if we can bind to the old port,
       *		and if we can we assume that the original process
       *		has gone away and permit a new registration for the
       *		same name.
       *		This is not foolproof - if the machine has more
       *		than one IP address, we could bind to the port on
       *		one address even though the server is using it on
       *		another.
       *		Also - the operating system is not guaranteed to
       *		let us bind to the port if another process has only
       *		recently stopped using it.
       *		Also - what if an old server used the port that the
       *		new one is using?  In this case the registration
       *		attempt will be refused even though it shouldn't be!
       *		On the other hand - the occasional registration
       *		failure MUST be better than permitting a process to
       *		grab a name already in use! If a server fails to
       *		register a name/port combination, it can always be
       *		coded to retry on a different port.
       */
      m = map_by_name(buf, size);
      if (m != 0 && port == m->port)
	{
	  /*
	   *	Special case - we already have this name registered for this
	   *	port - so everything is already ok.
	   */
	  if (debug)
	    {
	      snprintf(ebuf, sizeof(ebuf), "Already registered ... success");
	      gdomap_log(LOG_DEBUG);
	    }
	  *(uint32_t*)wi->buf = htonl(port);
	}
      else if (m != 0)
	{
	  int	sock = -1;

	  if ((ptype & GDO_NET_MASK) == GDO_NET_TCP)
	    {
	      sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	    }
	  else if ((ptype & GDO_NET_MASK) == GDO_NET_UDP)
	    {
	      sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	    }

	  if (sock < 0)
	    {
	      perror("unable to create new socket");
	    }
	  else
	    {
#ifndef BROKEN_SO_REUSEADDR
	      int	r = 1;

	      /*
	       * Under decent systems, SO_REUSEADDR means that the port can
	       * be reused immediately that this process exits.  Under some
	       * it means that multiple processes can serve the same port
	       * simultaneously.
	       * We don't want that broken behavior!
	       */
	      if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		(char*)&r, sizeof(r)) < 0)
		{
		  perror("unable to set socket options");
		}
	      else
#endif
		{
		  struct sockaddr_in	sa;
		  int			result;
		  short			p = m->port;

		  memset(&sa, '\0', sizeof(sa));
		  sa.sin_family = AF_INET;
		  sa.sin_addr.s_addr = htonl(INADDR_ANY);
		  sa.sin_port = htons(p);
		  result = bind(sock, (void*)&sa, sizeof(sa));
		  if (result == 0)
		    {
		      if (debug > 1)
			{
			  snprintf(ebuf, sizeof(ebuf),
			    "re-register from %d to %"PRIu32,
			    m->port, port);
			  gdomap_log(LOG_DEBUG);
			}
		      m->port = port;
		      m->net = (ptype & GDO_NET_MASK);
		      m->svc = (ptype & GDO_SVC_MASK);
		      port = htonl(m->port);
		      *(uint32_t*)wi->buf = port;
		    }
		}
#if	defined(__MINGW__)
	      /* closesocket(sock); */
#else
	      close(sock);
#endif
	    }
	}
      else if (port == 0)
	{	/* Port not provided!	*/
	  if (debug)
	    {
	      snprintf(ebuf, sizeof(ebuf), "Port not provided in request!");
	      gdomap_log(LOG_DEBUG);
	    }
	  *(uint32_t*)wi->buf = 0;
	}
      else
	{		/* Use port provided in request.	*/
	  if (debug)
	    {
	      snprintf(ebuf, sizeof(ebuf), "Registered on port %"PRIu32, port);
	      gdomap_log(LOG_DEBUG);
	    }
	  m = map_add(buf, size, port, ptype);
	  port = htonl(m->port);
	  *(uint32_t*)wi->buf = port;
	}
    }
  else if (type == GDO_LOOKUP)
    {
      m = map_by_name(buf, size);
      if (m != 0 && (m->net | m->svc) != ptype)
	{
	  if (debug > 1)
	    {
	      snprintf(ebuf, sizeof(ebuf),
		"requested service is of wrong type");
	      gdomap_log(LOG_DEBUG);
	    }
	  m = 0;	/* Name exists but is of wrong type.	*/
	}
      if (m)
	{
	  int	sock = -1;

	  /*
	   *	We check to see if we can bind to the old port, and if we can
	   *	we assume that the process has gone away and remove it from
	   *	the map.
	   *	This is not foolproof - if the machine has more
	   *	than one IP address, we could bind to the port on
	   *	one address even though the server is using it on
	   *	another.
	   */
	  if ((ptype & GDO_NET_MASK) == GDO_NET_TCP)
	    {
	      sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	    }
	  else if ((ptype & GDO_NET_MASK) == GDO_NET_UDP)
	    {
	      sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	    }

	  if (sock < 0)
	    {
	      perror("unable to create new socket");
	    }
	  else
	    {
#ifndef BROKEN_SO_REUSEADDR
	      int	r = 1;

	      /*
	       * Under decent systems, SO_REUSEADDR means that the port can
	       * be reused immediately that this process exits.  Under some
	       * it means that multiple processes can serve the same port
	       * simultaneously.
	       * We don't want that broken behavior!
	       */
	      if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		(char*)&r, sizeof(r)) < 0)
		{
		  perror("unable to set socket options");
		}
	      else
#endif
		{
		  struct sockaddr_in	sa;
		  int			result;
		  unsigned short	p = (unsigned short)m->port;

		  memset(&sa, '\0', sizeof(sa));
		  sa.sin_family = AF_INET;

#if	defined(__MINGW__)
		  /* COMMENT:  (3 Nov 2004 by Wim Oudshoorn):
		     The comment below might be true.  But
		     using addr[0].s_addr has on windows 2003 server
		     (and some other versions of windows.)
		     exactly the same sympton as it tries to avoid.
		     The funny thing is that the original line, just
		     using INADDR_ANY seems to work on windows.
		     However, I assume the FIXME below was put there
		     for a reason.  But for now I just revert it because
		     the platform independent code seems to work.
		  */
 		  /* FIXME: This must not be INADDR_ANY on Win,
 		     otherwise the system will try to bind on any of
 		     the local addresses (including 127.0.0.1), which
 		     works. - bjoern */
/*		  sa.sin_addr.s_addr = addr[0].s_addr; */
		  sa.sin_addr.s_addr = htonl(INADDR_ANY);
#else
 		  sa.sin_addr.s_addr = htonl(INADDR_ANY);
#endif /* __MINGW__ */
		  sa.sin_port = htons(p);
		  result = bind(sock, (void*)&sa, sizeof(sa));
		  if (result == 0)
		    {
		      map_del(m);
		      m = 0;
		    }
		}
#if	defined(__MINGW__)
	      closesocket(sock);
#else
	      close(sock);
#endif
	    }
	}
      if (m)
	{	/* Lookup found live server.	*/
	  *(uint32_t*)wi->buf = htonl(m->port);
	}
      else
	{		/* Not found.			*/
	  if (debug > 1)
	    {
	      snprintf(ebuf, sizeof(ebuf), "requested service not found");
	      gdomap_log(LOG_DEBUG);
	    }
	  *(unsigned short*)wi->buf = 0;
	}
    }
  else if (type == GDO_UNREG)
    {
      /*
       *	See if this is a request from a local process.
       */
      if (is_local_host(ri->addr.sin_addr) == 0)
	{
	  snprintf(ebuf, sizeof(ebuf), "Illegal attempt to un-register!");
	  gdomap_log(LOG_ERR);
	  clear_chan(desc);
	  return;
	}
      if (port == 0 || size > 0)
	{
	  m = map_by_name(buf, size);
	  if (m)
	    {
	      if ((m->net | m->svc) != ptype)
		{
		  if (debug)
		    {
		      snprintf(ebuf, sizeof(ebuf),
			"Attempted unregister with wrong type");
		      gdomap_log(LOG_DEBUG);
		    }
		}
	      else
		{
		  *(uint32_t*)wi->buf = htonl(m->port);
		  map_del(m);
		}
	    }
	  else
	    {
	      if (debug > 1)
		{
		  snprintf(ebuf, sizeof(ebuf), "requested service not found");
		  gdomap_log(LOG_DEBUG);
		}
	    }
	}
      else
	{
	  *(uint32_t*)wi->buf = 0;

	  while ((m = map_by_port(port, ptype)) != 0)
	    {
	      *(uint32_t*)wi->buf = htonl(m->port);
	      map_del(m);
	    }
	}
    }
  else if (type == GDO_SERVERS)
    {
      unsigned int	i;
      unsigned int	j;

      /* See if this is a request from a local process.
       *
       * This request is only useful locally.  Do not allow remote
       * requests for the server list.  Our response can be large,
       * so it would make a great UDP amplification attack.
       */
      if (is_local_host(ri->addr.sin_addr) == 0)
	{
	  snprintf(ebuf, sizeof(ebuf), "Illegal attempt to list servers!");
	  gdomap_log(LOG_ERR);
	  clear_chan(desc);
	  return;
	}

      free(wi->buf);
      wi->buf = (char*)calloc(sizeof(uint32_t)
	+ (prb_used+1)*IASIZE, 1);
      *(uint32_t*)wi->buf = htonl(prb_used+1);
      memcpy(&wi->buf[4], &ri->addr.sin_addr, IASIZE);

      /*
       * Copy the addresses of the hosts we have probed into the buffer.
       * During the copy, reverse the order of the addresses so that the
       * address we have contacted most recently is first.  This should
       * ensure that the client process will attempt to contact live
       * hosts before dead ones.
       */
      for (i = 0, j = prb_used; i < prb_used; i++)
	{
	  memcpy(&wi->buf[4+(i+1)*IASIZE], &prb[--j]->sin, IASIZE);
	}
      wi->len = 4 + (prb_used+1)*IASIZE;
    }
  else if (type == GDO_NAMES)
    {
      int	bytes = 0;
      uptr	ptr;
      int	i;

      free(wi->buf);

      /*
       * Size buffer for names.
       */
      for (i = 0; i < map_used; i++)
	{
	  bytes += 2 + map[i]->size;
	}
      /*
       * Allocate with space for number of names and set it up.
       */
      wi->buf = (char*)malloc(4 + bytes);
      *(uint32_t*)wi->buf = htonl(bytes);
      ptr = (uptr)wi->buf;
      ptr += 4;
      for (i = 0; i < map_used; i++)
	{
	  ptr[0] = (unsigned char)map[i]->size;
	  ptr[1] = (unsigned char)(map[i]->net | map[i]->svc);
	  memcpy(&ptr[2], map[i]->name, ptr[0]);
	  ptr += 2 + ptr[0];
	}
      wi->len = 4 + bytes;
    }
  else if (type == GDO_PROBE)
    {
      /*
       *	If the client is a name server, we add it to the list.
       */
      if (ri->addr.sin_port == my_port)
	{
	  struct in_addr	sin;

	  memcpy(&sin, ri->buf.r.name, IASIZE);
	  if (debug > 2)
	    {
	      snprintf(ebuf, sizeof(ebuf), "Probe from '%s'", inet_ntoa(sin));
	      gdomap_log(LOG_DEBUG);
	    }
	  prb_add(&sin);

	  /*
	   *	Irrespective of what we are told to do - we also add the
	   *	interface from which this packet arrived so we have a
	   *	route we KNOW we can use.
	   */
	  prb_add(&ri->addr.sin_addr);

#if 0
	{
	  struct in_addr	*ptr;
	  uint32_t	net;
	  int	c;

#if	defined(__MINGW__)
	  if (IN_CLASSA(sin.s_addr))
	    {
	      net = sin.s_addr & IN_CLASSA_NET;
	    }
	  else if (IN_CLASSB(sin.s_addr))
	    {
	      net = sin.s_addr & IN_CLASSB_NET;
	    }
	  else if (IN_CLASSC(sin.s_addr))
	    {
	      net = sin.s_addr & IN_CLASSC_NET;
	    }
#else
	  net = inet_netof(sin);
#endif
	  ptr = (struct in_addr*)&ri->buf.r.name[2*IASIZE];
	  c = (ri->buf.r.nsize - 2*IASIZE)/IASIZE;
	  while (c-- > 0)
	    {
	      if (debug > 2)
		{
		  snprintf(ebuf, sizeof(ebuf),
		    "Add server '%s'", inet_ntoa(*ptr));
		  gdomap_log(LOG_DEBUG);
		}
	      prb_add(ptr);
	      ptr++;
	    }
	}
#endif
	}
      /*
       *	For a UDP request from another name server, we send a reply
       *	packet.  We shouldn't be getting probes from anywhere else,
       *	but just to be nice, we send back our port number anyway.
       */
      if (desc == udp_desc && ri->addr.sin_port == my_port)
	{
	  struct in_addr	laddr;
	  struct in_addr	raddr;
	  struct in_addr	*other = 0;
	  unsigned int		elen;
	  void			*rbuf = ri->buf.r.name;
	  void			*wbuf;
	  int			i;
	  gdo_req		*r;

	  free(wi->buf);
	  r = (gdo_req*)calloc(GDO_REQ_SIZE, 1);
	  wi->buf = (char*)r;
	  wbuf = r->name;
	  r->rtype = GDO_PREPLY;
	  r->nsize = IASIZE*2;

	  memcpy(&raddr, rbuf, IASIZE);
	  memcpy(&laddr, rbuf+IASIZE, IASIZE);
	  if (debug > 2)
	    {
	      snprintf(ebuf, sizeof(ebuf),
		"Probe sent remote '%s'", inet_ntoa(raddr));
	      snprintf(ebuf, sizeof(ebuf),
		"Probe sent local  '%s'", inet_ntoa(laddr));
	    }

	  memcpy(wbuf+IASIZE, &raddr, IASIZE);
	  /*
	   *	If the other end did not tell us which of our addresses it was
	   *	probing, try to select one on the same network to send back.
	   *	otherwise, respond with the address it was probing.
	   */
	  if (is_local_host(laddr) == 0
	    || laddr.s_addr == loopback.s_addr)
	    {
	      for (i = 0; i < interfaces; i++)
		{
		  if (addr[i].s_addr == loopback.s_addr)
		    {
		      continue;
		    }
		  if ((mask[i].s_addr & addr[i].s_addr) ==
			(mask[i].s_addr & ri->addr.sin_addr.s_addr))
		    {
		      laddr = addr[i];
		      memcpy(wbuf, &laddr, IASIZE);
		      break;
		    }
		}
	    }
	  else
	    {
	      memcpy(wbuf, &laddr, IASIZE);
	    }
	  wi->len = GDO_REQ_SIZE;

	  elen = other_addresses_on_net(laddr, &other);
	  if (elen > 0)
	    {
	      while (elen > MAX_EXTRA)
		{
		  elen -= MAX_EXTRA;
		  queue_probe(&raddr, &laddr, MAX_EXTRA, &other[elen], 1);
		}
	      queue_probe(&raddr, &laddr, elen, other, 1);
	    }
	  if (0 != other)
	    {
	      free(other);
	    }
	}
      else
	{
	  port = my_port;
	  *(uint32_t*)wi->buf = htonl(port);
	}
    }
  else if (type == GDO_PREPLY)
    {
      /*
       *	This should really be a reply by UDP to a probe we sent
       *	out earlier.  We should add the name server to our list.
       */
      if (ri->addr.sin_port == my_port)
	{
	  struct in_addr	sin;

	  memcpy(&sin, &ri->buf.r.name, IASIZE);
	  if (debug > 2)
	    {
	      snprintf(ebuf, sizeof(ebuf),
		"Probe reply from '%s'", inet_ntoa(sin));
	      gdomap_log(LOG_DEBUG);
	    }
	  prb_add(&sin);
	  /*
	   *	Irrespective of what we are told to do - we also add the
	   *	interface from which this packet arrived so we have a
	   *	route we KNOW we can use.
	   */
	  prb_add(&ri->addr.sin_addr);
	}
      /*
       *	Because this is really a reply to us, we don't want to reply
       *	to it or we would get a feedback loop.
       */
      clear_chan(desc);
      return;
    }
  else
    {
      snprintf(ebuf, sizeof(ebuf), "Illegal operation code received!");
      gdomap_log(LOG_ERR);
      clear_chan(desc);
      return;
    }

  /*
   *	If the request was via UDP, we send a response back by queuing
   *	rather than letting the normal 'write_handler()' function do it.
   */
  if (desc == udp_desc)
    {
      queue_msg(&ri->addr, (unsigned char*)wi->buf, wi->len);
      clear_chan(desc);
    }
}

/*
 *	Name -		handle_send()
 *	Purpose -	Send any pending message on UDP socket.
 *			The code is designed to send the message in parts if
 *			the 'sendto()' function returns a positive integer
 *			indicating that only part of the message has been
 *			written.  This should never happen - but I coded it
 *			this way in case we have to run on a system which
 *			implements sendto() badly (I used such a system
 *			many years ago).
 */
static void
handle_send()
{
  struct u_data*	entry = u_queue;

  if (entry)
    {
      int	r;

      r = sendto(udp_desc, (const char *)&entry->dat[entry->pos],
      entry->len - entry->pos, 0, (void*)&entry->addr, sizeof(entry->addr));
      /*
       *	'r' is the number of bytes sent. This should be the number
       *	of bytes we asked to send, or -1 to indicate failure.
       */
      if (r > 0)
	{
	  entry->pos += r;
	}

      /*
       *	If we haven't written all the data, it should have been
       *	because we blocked.  Anything else is a major problem
       *	so we remove the message from the queue.
       */
      if (entry->pos != entry->len)
	{
#if	defined(__MINGW__)
	  if (WSAGetLastError() != WSAEWOULDBLOCK)
#else
	  if (errno != EWOULDBLOCK)
#endif
	    {
	      if (debug)
		{
		  snprintf(ebuf, sizeof(ebuf),
		    "failed sendto on %d for %s - %s",
		    udp_desc, inet_ntoa(entry->addr.sin_addr), strerror(errno));
		  gdomap_log(LOG_DEBUG);
		}
	      queue_pop();
	    }
	}
      else
	{
	  udp_sent++;
	  if (debug > 1)
	    {
	      snprintf(ebuf, sizeof(ebuf), "performed sendto for %s",
		inet_ntoa(entry->addr.sin_addr));
	      gdomap_log(LOG_DEBUG);
	    }
	  /*
	   *	If we have sent the entire message - remove it from queue.
	   */
	  if (entry->pos == entry->len)
	    {
	      queue_pop();
	    }
	}
    }
}

/*
 *	Name -		handle_write()
 *	Purpose -	Write data to a channel.  When all writing for the
 *			channel is complete, close the channel down.
 *
 *			This is all probably totally paranoid - the reply
 *			to any request is so short that the write operation
 *			should not block so there shouldn't be any need to
 *			handle non-blocking I/O.
 */
static void
handle_write(int desc)
{
  WInfo	*wi;
  char	*ptr;
  int	len;
  int	r;

  wi = getWInfo(desc, 0);
  if (wi == 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"handle_write for unknown descriptor (%d)", desc);
      gdomap_log(LOG_ERR);
      return;
    }
  ptr = wi->buf;
  len = wi->len;

#if	defined(__MINGW__)
  r = send(desc, &ptr[wi->pos], len - wi->pos, 0);
#else
  r = write(desc, &ptr[wi->pos], len - wi->pos);
#endif
  if (r < 0)
    {
      if (debug > 1)
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "Failed write on chan %d - closing", desc);
	  gdomap_log(LOG_DEBUG);
	}
      /*
       *	Failure - close connection silently.
       */
      clear_chan(desc);
    }
  else
    {
      wi->pos += r;
      if (wi->pos >= len)
	{
	  tcp_sent++;
	  if (debug > 1)
	    {
	      snprintf(ebuf, sizeof(ebuf),
		"Completed write on chan %d - closing", desc);
	      gdomap_log(LOG_DEBUG);
	    }
	  /*
	   *	Success - written all information.
	   */
	  clear_chan(desc);
	}
    }
}

/*
 *	Name -		tryRead()
 *	Purpose -	Attempt to read from a non blocking channel.
 *			Time out in specified time.
 *			If length of data is zero then just wait for
 *			descriptor to be readable.
 *			If the length is negative then attempt to
 *			read the absolute value of length but return
 *			as soon as anything is read.
 *
 *			Return -1 on failure
 *			Return -2 on timeout
 *			Return number of bytes read
 */
static int
tryRead(int desc, int tim, unsigned char* dat, int len)
{
  struct timeval timeout;
  fd_set	fds;
  void		*to;
  int		rval;
  int		pos = 0;
  time_t	when = 0;
  int		neg = 0;

  if (len < 0)
    {
      neg = 1;
      len = -len;
    }

  /*
   *	First time round we do a select with an instant timeout to see
   *	if the descriptor is already readable.
   */
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  for (;;)
    {
      to = &timeout;
      memset(&fds, '\0', sizeof(fds));
      FD_SET(desc, &fds);

      rval = select(FD_SETSIZE, &fds, 0, 0, to);
      if (rval == 0)
	{
	  time_t	now = time(0);

	  if (when == 0)
	    {
	      when = now;
	    }
	  else if (now - when >= tim)
	    {
	      return -2;		/* Timed out.		*/
	    }
	  else
	    {
	      /*
	       *	Set the timeout for a new call to select next time
	       *	round the loop.
	       */
	      timeout.tv_sec = tim - (now - when);
	      timeout.tv_usec = 0;
	    }
	}
      else if (rval < 0)
	{
	  return -1;		/* Error in select.	*/
	}
      else if (len > 0)
	{
#if	defined(__MINGW__)
	  rval = recv(desc, (char *)&dat[pos], len - pos, 0);
#else
	  rval = read(desc, &dat[pos], len - pos);
#endif
	  if (rval < 0)
	    {
#if	defined(__MINGW__)
	      if (WSAGetLastError() != WSAEWOULDBLOCK)
#else
	      if (errno != EWOULDBLOCK)
#endif
		{
		  return -1;		/* Error in read.	*/
		}
	    }
	  else if (rval == 0)
	    {
	      return -1;		/* End of file.		*/
	    }
	  else
	    {
	      pos += rval;
	      if (pos == len || neg == 1)
		{
		  return pos;	/* Read as needed.	*/
		}
	    }
	}
      else
	{
	  return 0;	/* Not actually asked to read.	*/
	}
    }
}

/*
 *	Name -		tryWrite()
 *	Purpose -	Attempt to write to a non blocking channel.
 *			Time out in specified time.
 *			If length of data is zero then just wait for
 *			descriptor to be writable.
 *			If the length is negative then attempt to
 *			write the absolute value of length but return
 *			as soon as anything is written.
 *
 *			Return -1 on failure
 *			Return -2 on timeout
 *			Return number of bytes written
 */
static int
tryWrite(int desc, int tim, unsigned char* dat, int len)
{
  struct timeval timeout;
  fd_set	fds;
  void		*to;
  int		rval;
  int		pos = 0;
  time_t	when = 0;
  int		neg = 0;

  if (len < 0) {
    neg = 1;
    len = -len;
  }

  /*
   *	First time round we do a select with an instant timeout to see
   *	if the descriptor is already writable.
   */
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  for (;;)
    {
      to = &timeout;
      memset(&fds, '\0', sizeof(fds));
      FD_SET(desc, &fds);

      rval = select(FD_SETSIZE, 0, &fds, 0, to);
      if (rval == 0)
	{
	  time_t	now = time(0);

	  if (when == 0)
	    {
	      when = now;
	    }
	  else if (now - when >= tim)
	    {
	      return -2;		/* Timed out.		*/
	    }
	  else
	    {
	      /* Set the timeout for a new call to select next time round
	       * the loop. */
	      timeout.tv_sec = tim - (now - when);
	      timeout.tv_usec = 0;
	    }
	}
      else if (rval < 0)
	{
	  return -1;		/* Error in select.	*/
	}
      else if (len > 0)
	{
#if	defined(__MINGW__) /* FIXME: Is this correct? */
	  rval = send(desc, (const char*)&dat[pos], len - pos, 0);
#else
	  void	(*ifun)();

	  /*
	   *	Should be able to write this short a message immediately, but
	   *	if the connection is lost we will get a signal we must trap.
	   */
	  ifun = signal(SIGPIPE, (void(*)(int))SIG_IGN);
	  rval = write(desc, &dat[pos], len - pos);
	  signal(SIGPIPE, ifun);
#endif

	  if (rval <= 0)
	    {
#if	defined(__MINGW__)
	      if (WSAGetLastError() != WSAEWOULDBLOCK)
#else
	      if (errno != EWOULDBLOCK)
#endif
		{
		  return -1;		/* Error in write.	*/
		}
	    }
	  else
	    {
	      pos += rval;
	      if (pos == len || neg == 1)
		{
		  return pos;	/* Written as needed.	*/
		}
	    }
	}
      else
	{
	  return 0;	/* Not actually asked to write.	*/
	}
    }
}

/*
 *	Name -		tryHost()
 *	Purpose -	Perform a name server operation with a given
 *			request packet to a server at specified address.
 *			On error - return non-zero with reason in 'errno'
 */
static int
tryHost(unsigned char op, unsigned char len, const unsigned char *name,
int ptype, struct sockaddr_in *addr, unsigned short *p, uptr *v)
{
  int           desc = socket(AF_INET, SOCK_STREAM, 0);
  int	        e = 0;
  uint32_t	port = *p;
  gdo_req	msg;
  struct sockaddr_in sin;
#if	defined(__MINGW__)
  DWORD dummy;
#endif /* __MINGW__ */

  *p = 0;
  if (desc < 0)
    {
      return 1;	/* Couldn't create socket.	*/
    }

#if	defined(__MINGW__)
  dummy = 1;
  if (ioctlsocket(desc, FIONBIO, &dummy) < 0)
    {
      e = WSAGetLastError();
      closesocket(desc);
      WSASetLastError(e);
      return 2;	/* Couldn't set non-blocking.	*/
    }
#else /* !__MINGW__ */
  if ((e = fcntl(desc, F_GETFL, 0)) >= 0)
    {
      e |= NBLK_OPT;
      if (fcntl(desc, F_SETFL, e) < 0)
	{
	  e = errno;
	  close(desc);
	  errno = e;
	  return 2;	/* Couldn't set non-blocking.	*/
	}
    }
  else
    {
      e = errno;
      close(desc);
      errno = e;
      return 2;	/* Couldn't set non-blocking.	*/
    }
#endif /* __MINGW__ */

  memcpy(&sin, addr, sizeof(sin));
  if (connect(desc, (struct sockaddr*)&sin, sizeof(sin)) != 0)
    {
#if	defined(__MINGW__)
      if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
      if (errno == EINPROGRESS)
#endif
	{
	  e = tryWrite(desc, 10, 0, 0);
	  if (e == -2)
	    {
	      e = errno;
#if	defined(__MINGW__)
	      closesocket(desc);
#else
	      close(desc);
#endif
	      errno = e;
	      return 3;	/* Connect timed out.	*/
	    }
	  else if (e == -1)
	    {
	      e = errno;
#if	defined(__MINGW__)
	      closesocket(desc);
#else
	      close(desc);
#endif
	      errno = e;
	      return 3;	/* Select failed.	*/
	    }
	}
      else
	{
	  e = errno;
#if	defined(__MINGW__)
	  closesocket(desc);
#else
	  close(desc);
#endif
	  errno = e;
	  return 3;		/* Failed connect.	*/
	}
    }

  memset((char*)&msg, '\0', GDO_REQ_SIZE);
  msg.rtype = op;
  msg.nsize = len;
  msg.ptype = ptype;
  if (op != GDO_REGISTER)
    {
      port = 0;
    }
  msg.port = htonl(port);
  if (name && len)
    {
      memcpy(msg.name, name, len);
    }

  e = tryWrite(desc, 10, (uptr)&msg, GDO_REQ_SIZE);
  if (e != GDO_REQ_SIZE)
    {
#if	defined(__MINGW__)
      e = WSAGetLastError();
      closesocket(desc);
      WSASetLastError(e);
#else
      e = errno;
      close(desc);
      errno = e;
#endif
      return 4;
    }
  e = tryRead(desc, 3, (uptr)&port, 4);
  if (e != 4)
    {
#if	defined(__MINGW__)
      e = WSAGetLastError();
      closesocket(desc);
      WSASetLastError(e);
#else
      e = errno;
      close(desc);
      errno = e;
#endif
      return 5;	/* Read timed out.	*/
    }
  port = ntohl(port);

  if (port > 0xffff)
    {
      if (GDO_NAMES == op)
        {
          if (port > 10000000)
            {
              snprintf(ebuf, sizeof(ebuf),
                "Insanely large list of registered names");
              gdomap_log(LOG_ERR);
              close(desc);
              return 5;     // Unreasonable number of registrations
            }
        }
      else
        {
          snprintf(ebuf, sizeof(ebuf),
            "Port number of incoming message is out of range");
          gdomap_log(LOG_ERR);
          close(desc);
          return 5;     // Unreasonable port number
        }
    }

  /*
   *	Special case for GDO_SERVERS - allocate buffer and read list.
   */
  if (op == GDO_SERVERS)
    {
      int	len = port * sizeof(struct in_addr);
      uptr	b;

      b = (uptr)malloc(len);
      if (tryRead(desc, 3, b, len) != len)
	{
	  free(b);
#if	defined(__MINGW__)
	  e = WSAGetLastError();
	  closesocket(desc);
	  WSASetLastError(e);
#else
	  e = errno;
	  close(desc);
	  errno = e;
#endif
	  return 5;
	}
      if (0 != v)
	{
          *v = b;
	}
    }
  /*
   *	Special case for GDO_NAMES - allocate buffer and read list.
   */
  else if (op == GDO_NAMES)
    {
      int	len = port;
      uptr	ptr;
      uptr	b;

      b = (uptr)malloc(len);
      if (tryRead(desc, 3, b, len) != len)
	{
	  free(b);
#if	defined(__MINGW__)
	  e = WSAGetLastError();
	  closesocket(desc);
	  WSASetLastError(e);
#else
	  e = errno;
	  close(desc);
	  errno = e;
#endif
	  return 5;
	}
      /*
       * Count the number of registered names and return them.
       */
      ptr = b;
      port = 0;
      while (ptr < (b + len))
	{
	  ptr += 2 + ptr[0];
	  port++;
	}
      if ((port & 0xffff) != port)
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "Insanely large number of registered names");
	  gdomap_log(LOG_ERR);
	  port = 0;
	}
      if (0 != v)
	{
	  *v = b;
	}
    }

  *p = (unsigned short)port;
#if	defined(__MINGW__)
  closesocket(desc);
#else
  close(desc);
#endif
  errno = 0;
  return 0;
}

/*
 *	Name -		nameFail()
 *	Purpose -	If given a failure status from tryHost()
 *			raise an appropriate exception.
 */
static void
nameFail(int why)
{
  switch (why)
    {
      case 0:	break;

      case 1:
	snprintf(ebuf, sizeof(ebuf),
	  "failed to contact name server - socket - %s",
	  strerror(errno));
	gdomap_log(LOG_ERR);
        break;

      case 2:
	snprintf(ebuf, sizeof(ebuf),
	  "failed to contact name server - socket - %s",
	  strerror(errno));
	gdomap_log(LOG_ERR);
        break;

      case 3:
	snprintf(ebuf, sizeof(ebuf),
	  "failed to contact name server - socket - %s",
	  strerror(errno));
	gdomap_log(LOG_ERR);
        break;

      case 4:
	snprintf(ebuf, sizeof(ebuf),
	  "failed to contact name server - socket - %s",
	  strerror(errno));
	gdomap_log(LOG_ERR);
        break;
    }
}

/*
 *	Name -		nameServer()
 *	Purpose -	Perform name server lookup or registration.
 *			Return success/failure status and set up an
 *			address structure for use in bind or connect.
 *	Restrictions -	0xffff byte name limit
 *			Uses old style host lookup - only handles the
 *			primary network interface for each host!
 */
static int
nameServer(const char* name, const char* host, int op, int ptype, struct sockaddr_in* addr, int pnum, int max)
{
  struct sockaddr_in	sin;
  unsigned short	p;
  unsigned short	port = 0;
  int			len = strlen(name);
  int			multi = 0;
  int			found = 0;
  int			rval;
  char			*first_dot = 0;

  if (len == 0)
    {
      snprintf(ebuf, sizeof(ebuf), "no name specified.");
      gdomap_log(LOG_ERR);
      return -1;
    }
  if (len > 0xffff)
    {
      snprintf(ebuf, sizeof(ebuf), "name length to large.");
      gdomap_log(LOG_ERR);
      return -1;
    }

  memset((char*)&sin, '\0', sizeof(sin));
  sin.sin_family = AF_INET;

#if	GDOMAP_PORT_OVERRIDE
  p = htons(GDOMAP_PORT_OVERRIDE);
#else
{
  struct servent        *sp;
  /*
   *	Ensure we have port number to connect to name server.
   *	The TCP service name 'gdomap' overrides the default port.
   */
  if ((sp = getservbyname("gdomap", "tcp")) != 0)
    {
      p = sp->s_port;		/* Network byte order.	*/
    }
  else
    {
      p = htons(GDOMAP_PORT);
    }
}
#endif
  sin.sin_port = p;

  /*
   *	The host name '*' matches any host on the local network.
   */
  if (host && host[0] == '*' && host[1] == '\0')
    {
      multi = 1;
    }
  /*
   *	If no host name is given, we use the name of the local host.
   *	NB. This should always be the case for operations other than lookup.
   */
  if (multi || host == 0 || *host == '\0')
    {
      first_dot = strchr(local_hostname, '.');
      if (first_dot)
	{
	  *first_dot = '\0';
	}
      host = local_hostname;
    }

#if     HAVE_GETADDRINFO
  {
    struct addrinfo       hints;
    struct addrinfo       *info;
    int                   err;

    memset(&hints, '\0', sizeof(hints));
    hints.ai_family = AF_INET;

    if (getaddrinfo(host, NULL, &hints, &info) != 0 && first_dot != 0)
      {
        *first_dot = '.';
        if ((err = getaddrinfo(host, NULL, &hints, &info)) != 0)
          {
            snprintf(ebuf, sizeof(ebuf),
              "getaddrinfo('%s') failed: %s", host, gai_strerror(err));
            gdomap_log(LOG_ERR);
            return -1;
          }
      }
    sin.sin_addr = ((struct sockaddr_in *)info->ai_addr)->sin_addr;
    freeaddrinfo(info);
  }
#else
  {
    struct hostent        *hp;

    if ((hp = gethostbyname(host)) == 0 && first_dot != 0)
      {
        *first_dot = '.';
        hp = gethostbyname(host);
      }
    if (hp == 0)
      {
        snprintf(ebuf, sizeof(ebuf),
          "gethostbyname('%s') failed: %s", host, strerror(errno));
        gdomap_log(LOG_ERR);
        return -1;
      }
    if (hp->h_addrtype != AF_INET)
      {
        snprintf(ebuf, sizeof(ebuf),
          "non-internet network not supported for %s", host);
        gdomap_log(LOG_ERR);
        return -1;
      }
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
  }
#endif

  if (multi)
    {
      unsigned short	num;
      struct in_addr	*b;

      /*
       * A host name of '*' is a special case which should do lookup on
       * all machines on the local network until one is found which has
       * the specified server on it.
       */
      rval = tryHost(GDO_SERVERS, 0, 0, ptype, &sin, &num, (uptr*)&b);
      if (0 == rval)
	{
	  int	i;

	  for (i = 0; found == 0 && i < num; i++)
	    {
	      memset((char*)&sin, '\0', sizeof(sin));
	      sin.sin_family = AF_INET;
	      sin.sin_port = p;
	      memcpy(&sin.sin_addr, &b[i], sizeof(struct in_addr));
	      if (sin.sin_addr.s_addr == 0)
		{
		  continue;
		}

	      if (tryHost(GDO_LOOKUP, len, (unsigned char*)name,
		ptype, &sin, &port, 0)==0)
		{
		  if (port != 0)
		    {
		      memset((char*)&addr[found], '\0', sizeof(*addr));
		      memcpy(&addr[found].sin_addr, &sin.sin_addr,
			sizeof(sin.sin_addr));
		      addr[found].sin_family = AF_INET;
		      addr[found].sin_port = htons(port);
		      found++;
		      if (found == max)
			{
			  break;
			}
		    }
		}
	    }
	  free(b);
	  return found;
	}
      else
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "failed to contact gdomap on %s(%s) - %s",
	    local_hostname, inet_ntoa(sin.sin_addr), strerror(errno));
	  gdomap_log(LOG_ERR);
	  return -1;
	}
    }
  else
    {
      if (op == GDO_REGISTER)
	{
	  port = (unsigned short)pnum;
	  if (port == 0 || htons(port) == p)
	    {
	      snprintf(ebuf, sizeof(ebuf),
	        "attempted registration with bad port (%d).", port);
	      gdomap_log(LOG_ERR);
	      return -1;
	    }
	}
      rval = tryHost(op, len, (unsigned char*)name, ptype, &sin, &port, 0);
      if (rval != 0)
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "failed to contact gdomap on %s(%s) - %s",
	    local_hostname, inet_ntoa(sin.sin_addr), strerror(errno));
	  gdomap_log(LOG_ERR);
	  return -1;
	}
      nameFail(rval);
    }

  if (op == GDO_REGISTER)
    {
      if (port == 0 || (pnum != 0 && port != pnum))
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "service already registered.");
	  gdomap_log(LOG_ERR);
	  return -1;
	}
    }
  if (port == 0)
    {
      return 0;
    }
  memset((char*)addr, '\0', sizeof(*addr));
  memcpy(&addr->sin_addr, &sin.sin_addr, sizeof(sin.sin_addr));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  return 1;
}

static void
lookup(const char *name, const char *host, int ptype)
{
  struct sockaddr_in	sin[100];
  int			found;
  int			i;

  found = nameServer(name, host, GDO_LOOKUP, ptype, sin, 0, 100);
  for (i = 0; i < found; i++)
    {
      snprintf(ebuf, sizeof(ebuf), "Found %s on '%s' port %d", name,
	inet_ntoa(sin[i].sin_addr), ntohs(sin[i].sin_port));
      gdomap_log(LOG_INFO);
    }
  if (found == 0)
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to find %s.", name);
      gdomap_log(LOG_INFO);
    }
}

static void
donames(const char *host)
{
  struct sockaddr_in	sin;
  unsigned short	p;
  unsigned short	num = 0;
  int			rval;
  uptr			b;
  char			*first_dot = 0;

  if (host == 0 || *host == '\0')
    {
      /*
       * If no host name is given, we use the name of the local host.
       */
      first_dot = strchr(local_hostname, '.');
      if (first_dot)
	{
	  *first_dot = '\0';
	}
      host = local_hostname;
    }

  memset((char*)&sin, '\0', sizeof(sin));
  sin.sin_family = AF_INET;

#if	GDOMAP_PORT_OVERRIDE
  p = htons(GDOMAP_PORT_OVERRIDE);
#else
  {
    struct servent      *sp;
    /*
     *	Ensure we have port number to connect to name server.
     *	The TCP service name 'gdomap' overrides the default port.
     */
    if ((sp = getservbyname("gdomap", "tcp")) != 0)
      {
        p = sp->s_port;		/* Network byte order.	*/
      }
    else
      {
        p = htons(GDOMAP_PORT);
      }
  }
#endif

  sin.sin_port = p;
 
#if     HAVE_GETADDRINFO
  {
    struct addrinfo       hints;
    struct addrinfo       *info;
    int                   err;

    memset(&hints, '\0', sizeof(hints));
    hints.ai_family = AF_INET;

    if ((err = getaddrinfo(host, NULL, &hints, &info) != 0) && first_dot != 0)
      {
        *first_dot = '.';
        err = getaddrinfo(host, NULL, &hints, &info);
      }
    if (err != 0)
    {
        snprintf(ebuf, sizeof(ebuf),
          "getaddrinfo('%s') failed: %s", host, gai_strerror(err));
        gdomap_log(LOG_ERR);
        return;
    }
    sin.sin_addr = ((struct sockaddr_in *)info->ai_addr)->sin_addr;
    freeaddrinfo(info);
  }
#else
  {
    struct hostent        *hp;

    if ((hp = gethostbyname(host)) == 0 && first_dot != 0)
      {
        *first_dot = '.';
        hp = gethostbyname(host);
      }
    if (hp == 0)
      {
        snprintf(ebuf, sizeof(ebuf),
          "gethostbyname('%s') failed: %s", host, strerror(errno));
        gdomap_log(LOG_ERR);
        return;
      }
    if (hp->h_addrtype != AF_INET)
      {
        snprintf(ebuf, sizeof(ebuf),
          "non-internet network not supported for %s", host);
        gdomap_log(LOG_ERR);
        return;
      }

    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
  }
#endif

  rval = tryHost(GDO_NAMES, 0, 0, 0, &sin, &num, (uptr*)&b);
  if (rval != 0)
    {
      snprintf(ebuf, sizeof(ebuf), "failed to contact gdomap on %s(%s) - %s",
        local_hostname, inet_ntoa(sin.sin_addr), strerror(errno));
      gdomap_log(LOG_ERR);
      return;
    }
  if (num == 0)
    {
      snprintf(ebuf, sizeof(ebuf), "No names currently registered with gdomap");
      gdomap_log(LOG_INFO);
    }
  else
    {
      uptr	p = b;

      snprintf(ebuf, sizeof(ebuf), "Registered names are -");
      gdomap_log(LOG_INFO);
      while (num-- > 0)
	{
	  char	buf[256];

	  memcpy(buf, &p[2], p[0]);
	  buf[p[0]] = '\0';
	  snprintf(ebuf, sizeof(ebuf), "  %s", buf);
	  gdomap_log(LOG_INFO);
	  p += 2 + p[0];
	}
    }
  free(b);
}

static void
doservers(const char *host)
{
  struct sockaddr_in	sin;
  unsigned short	p;
  unsigned short	num = 0;
  int			rval;
  uptr			b;
  char			*first_dot = 0;

  if (host == 0 || *host == '\0')
    {
      /*
       * If no host name is given, we use the name of the local host.
       */
      first_dot = strchr(local_hostname, '.');
      if (first_dot)
	{
	  *first_dot = '\0';
	}
      host = local_hostname;
    }

  memset((char*)&sin, '\0', sizeof(sin));
  sin.sin_family = AF_INET;

#if	GDOMAP_PORT_OVERRIDE
  p = htons(GDOMAP_PORT_OVERRIDE);
#else
  {
    struct servent      *sp;
    /*
     *	Ensure we have port number to connect to name server.
     *	The TCP service name 'gdomap' overrides the default port.
     */
    if ((sp = getservbyname("gdomap", "tcp")) != 0)
      {
        p = sp->s_port;		/* Network byte order.	*/
      }
    else
      {
        p = htons(GDOMAP_PORT);
      }
  }
#endif

  sin.sin_port = p;
 
#if     HAVE_GETADDRINFO
  {
    struct addrinfo       hints;
    struct addrinfo       *info;
    int                   err;

    memset(&hints, '\0', sizeof(hints));
    hints.ai_family = AF_INET;

    if ((err = getaddrinfo(host, NULL, &hints, &info) != 0) && first_dot != 0)
      {
        *first_dot = '.';
        err = getaddrinfo(host, NULL, &hints, &info);
      }
    if (err != 0)
    {
        snprintf(ebuf, sizeof(ebuf),
          "getaddrinfo('%s') failed: %s", host, gai_strerror(err));
        gdomap_log(LOG_ERR);
        return;
    }
    sin.sin_addr = ((struct sockaddr_in *)info->ai_addr)->sin_addr;
    freeaddrinfo(info);
  }
#else
  {
    struct hostent        *hp;

    if ((hp = gethostbyname(host)) == 0 && first_dot != 0)
      {
        *first_dot = '.';
        hp = gethostbyname(host);
      }
    if (hp == 0)
      {
        snprintf(ebuf, sizeof(ebuf),
          "gethostbyname('%s') failed: %s", host, strerror(errno));
        gdomap_log(LOG_ERR);
        return;
      }
    if (hp->h_addrtype != AF_INET)
      {
        snprintf(ebuf, sizeof(ebuf),
          "non-internet network not supported for %s", host);
        gdomap_log(LOG_ERR);
        return;
      }

    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
  }
#endif

  rval = tryHost(GDO_SERVERS, 0, 0, 0, &sin, &num, (uptr*)&b);
  if (rval != 0)
    {
      snprintf(ebuf, sizeof(ebuf), "failed to contact gdomap on %s(%s) - %s",
        local_hostname, inet_ntoa(sin.sin_addr), strerror(errno));
      gdomap_log(LOG_ERR);
      return;
    }
  if (num == 0)
    {
      snprintf(ebuf, sizeof(ebuf), "No servers currently known to gdomap");
      gdomap_log(LOG_INFO);
    }
  else
    {
      struct in_addr	*p = (struct in_addr*)(b + 4);

      snprintf(ebuf, sizeof(ebuf), "Known servers are -");
      gdomap_log(LOG_INFO);
      while (num-- > 0)
	{
	  snprintf(ebuf, sizeof(ebuf), "  %s", inet_ntoa(*p));
	  gdomap_log(LOG_INFO);
	  p++;
	}
    }
  free(b);
}

static void
doregister(const char *name, int port, int ptype)
{
  struct sockaddr_in	sin;
  int			found;
  int			i;

  found = nameServer(name, 0, GDO_REGISTER, ptype, &sin, port, 1);
  for (i = 0; i < found; i++)
    {
      snprintf(ebuf, sizeof(ebuf), "Registered %s on '%s' port %d", name,
	inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
      gdomap_log(LOG_INFO);
    }
  if (found == 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Unable to register %s on port %d.", name, port);
      gdomap_log(LOG_ERR);
    }
}

static void
unregister(const char *name, int port, int ptype)
{
  struct sockaddr_in	sin;
  int			found;
  int			i;

  found = nameServer(name, 0, GDO_UNREG, ptype, &sin, port, 1);
  for (i = 0; i < found; i++)
    {
      snprintf(ebuf, sizeof(ebuf), "Unregistered %s on '%s' port %d", name,
	inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
      gdomap_log(LOG_INFO);
    }
  if (found == 0)
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to unregister %s.", name);
      gdomap_log(LOG_INFO);
    }
}

static void do_help(int argc, char **argv, char *options)
{
  /*
   * Where argc > 0 we check to see if there is a '--help' argument
   * and do nothing if there isn't.
   */
  if (argc > 0)
    {
      int	i;

      for (i = 1; i < argc; i++)
	{
	  if (strcmp(argv[i], "--help") == 0)
	    {
	      break;
	    }
	}
      if (i == argc)
	{
	  return;	// --help not found ...
	}
    }
  printf("%s -[%s]\n", argv[0], options);
  printf("GNU Distributed Objects name server\n");
  printf("-C		help about interfaces and configuration\n");
  printf("-H		general help\n");
  printf("-I		pid file to write pid\n");
  printf("-L name		perform lookup for name then quit.\n");
  printf("-M name		machine name for -L and -N\n");
  printf("-N		list all names registered on host\n");
  printf("-P number	port number required for R option.\n");
  printf("-R name		register name locally then quit.\n");
  printf("-S		list all gdomap servers known to host\n");
  printf("-T type		port type for L, R and U options -\n");
  printf("		tcp_gdo, udp_gdo,\n");
  printf("		tcp_foreign, udp_foreign.\n");
  printf("-U name		unregister name locally then quit.\n");
  printf("-a file		use config file for interface list.\n");
  printf("-c file		use config file for probe.\n");
  printf("-d		extra debug logging (normally via syslog).\n");
  printf("-f		avoid fork() to make debugging easy\n");
  printf("-i seconds	re-probe at this interval (roughly), min 60\n");
#if	!defined(__MINGW__)
  printf("-j path		specify a jail directory the process is to\n");
  printf("		run in (if omitted, /tmp is used).\n");
#endif
  printf("-p		disable probing for other servers\n");
  printf("\n");
  printf("Kill with SIGUSR1 to obtain a dump of all known peers\n");
  printf("in /tmp/gdomap.dump\n");
  printf("\n");
  exit(EXIT_SUCCESS);
}

#if	defined(__MINGW__)
static char*
quoteArg(const char *arg)
{
  int	len = strlen(arg);
  int	in;
  int	out = 0;
  int	quote = 0;
  char	*ptr = malloc(len*2+3);

  /*
   * Check for white space ... if present, must quote argument.
   */
  for (in = 0; in < len; in++)
    {
      if (isspace(arg[in]))
	{
	  quote = 1;
	  break;
	}
    }

  if (quote)
    {
      ptr[out++] = '"';
    }

  for (in = 0; in < len; in++)
    {
      if (arg[in] == '\\')
	{
	  int	pos = in + 1;

	  // Step past any backslashes
	  while (pos < len && ptr[pos] == '\\')
	    {
	      pos++;
	    }
	  // If backslashes precede a quote ... double them.
	  if (pos < len && ptr[pos] == '"')
	    {
	      int	num = pos - in;

	      while (num-- > 0)
		{
		  ptr[out++] = '\\';
		}
	    }
	  // Copy the original backslashes
	  while (in < pos)
	    {
	      ptr[out++] = arg[in++];
	    }
	  // Copy the character after the backslashes
	  if (in < len)
	    {
	      ptr[out++] = arg[in];
	    }
	}
      else if (arg[in] == '"')
	{
	  ptr[out++] = '\\';	// Escape the quote.
	  ptr[out++] = arg[in];
	}
      else
	{
	  ptr[out++] = arg[in];
	}
    }


  if (quote)
    {
      ptr[out++] = '"';
    }

  ptr[out] = '\0';

  return ptr;
}
#endif

/**
 * (A dummy comment to help autogsdoc realize this is a command-line tool.)
 */
#if	GS_FAKE_MAIN
/* Since we don't link gnustep-base, the main function below has to be
 * the real main function. */
#undef	main
#endif
int
main(int argc, char** argv)
{
  extern char	*optarg;
#if	defined(__MINGW__)
  char	*options = "-CHI:L:M:NP:R:ST:U:a:bc:dfi:p";
#else
  char	*options = "-CHI:L:M:NP:R:ST:U:a:bc:dfi:j:p";
  const char    *jail = 0;
#endif
  int		c;
  int		ptype = GDO_TCP_GDO;
  int		port = 0;
  const char	*machine = 0;
  const char	*lookupf = 0;
  int		donamesf = 0;
  int		doserversf = 0;

#if	defined(__MINGW__)
  WORD wVersionRequested;
  WSADATA wsaData;

  wVersionRequested = MAKEWORD(2, 2);
  WSAStartup(wVersionRequested, &wsaData);
#else
  int           forked = 0;
#endif

  local_hostname = xgethostname();
  if (!local_hostname)
    {
      snprintf(ebuf, sizeof(ebuf),
	"gethostname() failed: %s", strerror(errno));
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }

  /*
   *	Would use inet_aton(), but older systems don't have it.
   */
  loopback.s_addr = inet_addr("127.0.0.1");
#if	defined(__MINGW__)
  class_a_net = IN_CLASSA_NET;
  class_a_mask.s_addr = class_a_net;
  class_b_net = IN_CLASSB_NET;
  class_b_mask.s_addr = class_b_net;
  class_c_net = IN_CLASSC_NET;
  class_c_mask.s_addr = class_c_net;
#else
  class_a_net = inet_network("255.0.0.0");
  class_a_mask = inet_makeaddr(class_a_net, 0);
  class_b_net = inet_network("255.255.0.0");
  class_b_mask = inet_makeaddr(class_b_net, 0);
  class_c_net = inet_network("255.255.255.0");
  class_c_mask = inet_makeaddr(class_c_net, 0);
#endif

  do_help(argc, argv, options);
  while ((c = getopt(argc, argv, options)) != -1)
    {
      switch(c)
	{
	  case 'H':
	    do_help(0, argv, options);
	    exit(EXIT_SUCCESS);

	  case 'C':
	    printf("\n");
	    printf(
"Gdomap normally probes every machine on the local network to see if there\n"
"is a copy of gdomap running on it.  This is done for class-C networks and\n"
"subnets of class-C networks.  If your host is on a class-B or class-A net\n"
"then the default behaviour is to treat it as a class-C net and probe only\n"
"the hosts that would be expected on a class-C network of the same number.\n");
	    printf("\n");
	    printf(
"If you are running on a class-A or class-B network, or if your net has a\n"
"large number of hosts which will not have gdomap on them - you may want to\n"
"supply a configuration file listing the hosts to be probed explicitly,\n"
"rather than getting gdomap to probe all hosts on the local net.\n");
	    printf("\n");
	    printf(
"You may also want to supply the configuration file so that hosts which are\n"
"not actually on your local network can still be found when your code tries\n"
"to connect to a host using @\"*\" as the host name.  NB. this functionality\n"
"does not exist in OpenStep.\n");
	    printf("\n");
	    printf(
"A configuration file consists of a list of IP addresses to be probed.\n"
"The IP addresses should be in standard 'dot' notation, one per line.\n"
"Empty lines are permitted in the configuration file.\n"
"Anything on a line after a hash ('#') is ignored.\n"
"You tell gdomap about the config file with the '-c' command line option.\n");
	    printf("\n");
	    printf("\n");
printf(
"gdomap uses the SIOCGIFCONF ioctl to build a list of IP addresses and\n"
"netmasks for the network interface cards on your machine.  On some operating\n"
"systems, this facility is not available (or is broken), so you must tell\n"
"gdomap the addresses and masks of the interfaces using the '-a' command line\n"
"option.  The file named with '-a' should contain a series of lines with\n"
"space separated pairs of addresses and masks in 'dot' notation, eg.\n"
"192.168.1.2 255.255.255.0\n"
"You must NOT include loopback interfaces in this list.\n"
"If you want to support broadcasting of probe information on a network,\n"
"you may supply the broadcast address as a third item on the line, eg.\n"
"192.168.1.9 255.255.255.0 192.168.1.255\n"
"If your operating system has some other method of giving you a list of\n"
"network interfaces and masks, please send me example code so that I can\n"
"implement it in gdomap.\n");
	    printf("\n");
	    exit(EXIT_SUCCESS);

	  case 'L':
	    lookupf = optarg;
	    break;

	  case 'M':
	    machine = optarg;
	    break;

	  case 'N':
	    donamesf = 1;
	    break;

	  case 'P':
	    port = atoi(optarg);
	    break;

	  case 'R':
	    if (machine && *machine)
	      {
		fprintf(stderr, "-M flag is ignored for registration.\n");
		fprintf(stderr, "Registration will take place locally.\n");
	      }
	    doregister(optarg, port, ptype);
	    exit(EXIT_SUCCESS);

	  case 'S':
	    doserversf = 1;
	    break;

	  case 'T':
	    if (strcmp(optarg, "tcp_gdo") == 0)
	      {
		ptype = GDO_TCP_GDO;
	      }
	    else if (strcmp(optarg, "udp_gdo") == 0)
	      {
		ptype = GDO_UDP_GDO;
	      }
	    else if (strcmp(optarg, "tcp_foreign") == 0)
	      {
		ptype = GDO_TCP_FOREIGN;
	      }
	    else if (strcmp(optarg, "udp_foreign") == 0)
	      {
		ptype = GDO_UDP_FOREIGN;
	      }
	    else
	      {
		fprintf(stderr, "Warning - -P selected unknown type -");
		fprintf(stderr, " using tcp_gdo.\n");
		ptype = GDO_TCP_GDO;
	      }
	    break;

	  case 'U':
	    if (machine && *machine)
	      {
		fprintf(stderr, "-M flag is ignored for unregistration.\n");
		fprintf(stderr, "Operation will take place locally.\n");
	      }
	    unregister(optarg, port, ptype);
	    exit(EXIT_SUCCESS);

	  case 'a':
	    load_iface(optarg);
	    break;

	  case 'b':
	    nobcst++;
	    break;

	  case 'c':
	    {
	      FILE	*fptr;
	      int	line = 0;
	      int	count = 0;
	      char	buf[128];

	      in_config = 1;
              /* First see if we *can* access the file.
               */
	      fptr = fopen(optarg, "rt");
	      if (fptr == 0)
		{
		  snprintf(ebuf, sizeof(ebuf),
		    "Unable to open probe config - '%s'\n",
		    optarg);
		  gdomap_log(LOG_CRIT);
		  exit(EXIT_FAILURE);
		}
              /* Now check that we *should* be accessing it.
               */
	      if (access(optarg, R_OK) != 0)
		{
                  fclose(fptr);
		  snprintf(ebuf, sizeof(ebuf),
		    "Unable to access probe config - '%s'\n",
		    optarg);
		  gdomap_log(LOG_CRIT);
		  exit(EXIT_FAILURE);
		}
	      while (fgets(buf, sizeof(buf), fptr) != 0)
		{
		  char		*ptr = buf;
		  plentry	*prb;

		  line++;
		  /*
		   *	Strip leading white space.
		   */
		  while (isspace(*ptr))
		    {
		      ptr++;
		    }
		  if (ptr != buf)
		    {
		      memmove(buf, ptr, strlen(ptr) + 1);
		    }
		  /*
		   *	Strip comments.
		   */
		  ptr = strchr(buf, '#');
		  if (ptr)
		    {
		      *ptr = '\0';
		    }
		  /*
		   *	Strip trailing white space.
		   */
		  ptr = buf;
		  while (*ptr)
		    {
		      ptr++;
		    }
		  while (ptr > buf && isspace(ptr[-1]))
		    {
		      ptr--;
		    }
		  *ptr = '\0';
		  /*
		   *	Ignore blank lines.
		   */
		  if (*buf == '\0')
		    {
		      continue;
		    }

		  if (count++ > 1000)
		    {
		      snprintf(ebuf, sizeof(ebuf),
			"Too many probe configurations found");
		      gdomap_log(LOG_CRIT);
		      exit(EXIT_FAILURE);
		    }
		  prb = (plentry*)malloc(sizeof(plentry));
		  memset((char*)prb, '\0', sizeof(plentry));
		  prb->addr.s_addr = inet_addr(buf);
		  if (prb->addr.s_addr == (uint32_t)-1)
		    {
		      snprintf(ebuf, sizeof(ebuf),
			"line %d of '%s' (%s) is not a valid address\n",
			line, optarg, buf);
		      free(prb);
		      gdomap_log(LOG_CRIT);
		      exit(EXIT_FAILURE);
		    }
		  else
		    {
		      /*
		       *	Add this address at the end of the list.
		       */
		      if (plist == 0)
			{
			  plist = prb;
			}
		      else
			{
			  plentry	*tmp = plist;

			  while (tmp->next)
			    {
			      if (tmp->addr.s_addr == prb->addr.s_addr)
				{
				  snprintf(ebuf, sizeof(ebuf),
				    "'%s' repeat in '%s'\n",
				    buf, optarg);
				  free(prb);
				  gdomap_log(LOG_CRIT);
				  exit(EXIT_FAILURE);
				  break;
				}
			      tmp = tmp->next;
			    }
			  if (tmp->next == 0)
			    {
			      tmp->next = prb;
			    }
			}
		    }
		}
	      fclose(fptr);
	      in_config = 0;
	    }
	    break;

	  case 'I':
	    pidfile = optarg;
	    break;

	  case 'd':
	    debug++;
	    break;

	  case 'f':
	    nofork++;
	    break;

	  case 'i':
	    interval = atoi(optarg);
	    if (interval < 60)
	      {
		interval = 60;
	      }
	    break;

#if	!defined(__MINGW__)
	  case 'j':
	    jail = optarg;
            break;
#endif

	  case 'p':
	    noprobe++;
	    break;

	  default:
	    printf("%s - GNU Distributed Objects name server\n", argv[0]);
	    printf("-H	for help\n");
	    exit(EXIT_SUCCESS);
	}
    }
  if (donamesf || doserversf || lookupf)
    {
      if (donamesf)
	{
	  donames(machine);
	}
      if (doserversf)
	{
	  doservers(machine);
	}
      if (lookupf)
	{
	  lookup(lookupf, machine, ptype);
	}
      exit (0);
    }

#if	defined(__MINGW__) /* On Win32, we don't fork */
  if (nofork == 0)
    {
      char	**a = malloc((argc+2) * sizeof(char*));
      int	i;

      for (i = 0; i < argc; i++)
	{
	  a[i] = quoteArg(argv[i]);
	}
      a[argc] = "-f";
      a[argc+1] = 0;
      if (_spawnv(_P_NOWAIT, argv[0], (const char* const*)a) == -1)
	{
	  fprintf(stderr, "gdomap - spawn '%s' failed - bye.\n", argv[0]);
	  exit(EXIT_FAILURE);
	}
      if (debug)
        {
	  snprintf(ebuf, sizeof(ebuf), "initialisation complete.");
	  gdomap_log(LOG_DEBUG);
        }
      exit(EXIT_SUCCESS);
    }
#else
  if (nofork == 0)
    {
      /*
       *	Now fork off child process to run in background.
       */
      switch (fork())
	{
	  case -1:
	    fprintf(stderr, "gdomap - fork failed - bye.\n");
	    exit(EXIT_FAILURE);

	  case 0:
	    /*
	     *	Try to run in background.
	     */
          forked = 1;
#if	defined(NeXT)
	    setpgrp(0, getpid());
#else
	    setsid();
#endif
	    break;

	  default:
	    if (debug)
	      {
		snprintf(ebuf, sizeof(ebuf), "initialisation complete.");
		gdomap_log(LOG_DEBUG);
	      }
	    exit(EXIT_SUCCESS);
	}
    }

  /*
   *	Ensure we don't have any open file descriptors which may refer
   *	to sockets bound to ports we may try to use.
   *
   *	Use '/dev/null' for stdin and stdout.  Assume stderr is ok.
   */
  for (c = 0; c < FD_SETSIZE; c++)
    {
      if (is_daemon || (c != 2))
	{
	  (void)close(c);
	}
    }
  if (open("/dev/null", O_RDONLY) != 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"failed to open stdin from /dev/null (%s)\n", strerror(errno));
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
  if (open("/dev/null", O_WRONLY) != 1)
    {
      snprintf(ebuf, sizeof(ebuf),
	"failed to open stdout from /dev/null (%s)\n", strerror(errno));
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
  if (is_daemon && open("/dev/null", O_WRONLY) != 2)
    {
      snprintf(ebuf, sizeof(ebuf),
	"failed to open stderr from /dev/null (%s)\n", strerror(errno));
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
  if (debug)
    {
      snprintf(ebuf, sizeof(ebuf), "Closed descriptors");
      gdomap_log(LOG_DEBUG);
    }

  if (forked)
    {
      is_daemon = 1;
#if	defined(HAVE_SYSLOG)
#if	defined(SYSLOG_4_2)
      openlog ("gdomap", LOG_NDELAY);
      log_priority = LOG_DAEMON;
#elif !defined(HAVE_SLOGF)
      openlog ("gdomap", LOG_NDELAY, LOG_DAEMON);
#endif
#endif
    }

#endif /* !__MINGW__ */

  init_my_port();	/* Determine port to listen on.		*/
  init_ports();		/* Create ports to handle requests.	*/

  if (interfaces == 0)
    {
      init_iface();	/* Build up list of network interfaces.	*/
    }

  if (!is_local_host(loopback))
    {
      snprintf(ebuf, sizeof(ebuf),
	 "I can't find the loopback interface on this machine.");
      gdomap_log(LOG_ERR);
      snprintf(ebuf, sizeof(ebuf),
"Perhaps you should correct your machine configuration or use the -a flag.\n"
"Try 'gdomap -C' for more information.\n");
      gdomap_log(LOG_INFO);
      if (interfaces < MAX_IFACE)
	{
	  addr[interfaces].s_addr = loopback.s_addr;
	  mask[interfaces] = class_c_mask;
	  interfaces++;
	  snprintf(ebuf, sizeof(ebuf),
	    "I am assuming loopback interface on 127.0.0.1");
	  gdomap_log(LOG_INFO);
	}
      else
	{
	  snprintf(ebuf, sizeof(ebuf),
"You have too many network interfaces to add the loopback interface on "
"127.0.0.1 - you need to change the 'MAX_IFACE' constant in gdomap.c and "
"rebuild it.");
	  gdomap_log(LOG_CRIT);
	  exit(EXIT_FAILURE);
	}
    }

  /* Write the pidfile, but only if the user is root. This allows us
     to write to restricted directories without allowing normal users
     to mess it up. Because we are writing as root, we must be careful
     to create/open the file in exclusive mode */
  if (pidfile)
    {
      FILE	*fptr;

#ifndef __MINGW__
      if (getuid () == 0)
#endif
	{
	  int	desc = open(pidfile, O_EXCL|O_CREAT|O_WRONLY, 0644);
	  fptr = fdopen(desc, "w");

	  if (fptr == 0)
	    {
	      snprintf(ebuf, sizeof(ebuf),
		"Unable to create new pid file - '%s'", pidfile);
	      gdomap_log(LOG_CRIT);
	      exit(EXIT_FAILURE);
	    }
	  fprintf(fptr, "%d\n", (int) getpid());
	  fclose(fptr);
	  chmod(pidfile, 0644);
	}
#ifndef __MINGW__
      else
	{
	  snprintf(ebuf, sizeof(ebuf),
	    "Only root user can write to pid file\n");
	  gdomap_log(LOG_WARNING);
	}
#endif
    }
{
#ifndef __MINGW__
  uid_t uid = -2;
  gid_t gid = -2;
#endif

#if	defined(HAVE_PWD_H)
#if	defined(HAVE_GETPWNAM)
  struct passwd *pw = getpwnam("nobody");

  if (pw != 0)
    {
      uid = pw->pw_uid;
      gid = pw->pw_gid;
    }
#endif
#endif

#if	!defined(__svr4__)
  /*
   * As another level of paranoia - jail this process to a directory
   */
#ifndef __MINGW__
  if (0 == jail)
    {
      jail = "/tmp";    /* Not great, but better than nothing */
    }
  if (chdir(jail) < 0)
    {
      snprintf(ebuf, sizeof(ebuf), "Unable to change directory to %s", jail);
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }

  if (geteuid() == 0)
    {
      if (chroot(jail) < 0)
	{
	  snprintf(ebuf, sizeof(ebuf), "Unable to change root to %s", jail);
	  gdomap_log(LOG_CRIT);
	  exit(EXIT_FAILURE);
	}
      chdir("/");
    }
#endif /* __MINGW__ */
#endif /* __svr4__ */

#ifndef __MINGW__
  /*
   * Try to become a 'safe' user now that we have
   * done everything that needs root priv.
   *
   * Try to be the user who launched us ... so they can kill us too.
   * Otherwise default to user nobody.
   */
  if (getuid () != 0)
    {
      uid = getuid();
      gid = getgid();
    }
  /* The call to setgroups may fail if we don't have that capability ...
   * but in that case we aren't too bothered anyway.
   */
  setgroups (1, &gid);
  if (setgid (gid) < 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Failed setgid(%d) - %s", gid, strerror(errno));
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
  if (setuid (uid) < 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Failed setuid(%d) - %s", uid, strerror(errno));
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
  if (getuid () == 0)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Still running as root after trying to change");
      gdomap_log(LOG_CRIT);
      exit(EXIT_FAILURE);
    }
#endif /* __MINGW__ */
}

  init_probe();	/* Probe other name servers on net.	*/

  if (debug)
    {
      snprintf(ebuf, sizeof(ebuf), "entering main loop.\n");
      gdomap_log(LOG_DEBUG);
    }
  handle_io();
  return 0;
}

/*
 *	Name -		queue_probe()
 *	Purpose -	Send a probe request to a specified host so we
 *			can see if a name server is running on it.
 *			We don't bother to check to see if it worked.
 */
static void
queue_probe(struct in_addr* to, struct in_addr* from, int l, struct in_addr* e, int f)
{
  struct sockaddr_in	sin;
  gdo_req	msg;

  if (debug > 2)
    {
      snprintf(ebuf, sizeof(ebuf),
	"Probing for server on '%s' from '", inet_ntoa(*to));
      strncat(ebuf, inet_ntoa(*from), sizeof(ebuf) - strlen(ebuf) - 1);
      strncat(ebuf, "'", sizeof(ebuf) - strlen(ebuf) - 1);
      gdomap_log(LOG_DEBUG);
      if (l > 0)
	{
	  int	i;

	  snprintf(ebuf, sizeof(ebuf),
	    " %d additional local addresses sent -", l);
	  gdomap_log(LOG_DEBUG);
	  for (i = 0; i < l; i++)
	    {
	      snprintf(ebuf, sizeof(ebuf), " '%s'", inet_ntoa(e[i]));
	      gdomap_log(LOG_DEBUG);
	    }
	}
    }
  memset(&sin, '\0', sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, to, sizeof(*to));
  sin.sin_port = my_port;

  memset((char*)&msg, '\0', GDO_REQ_SIZE);
  if (f)
    {
      msg.rtype = GDO_PREPLY;
    }
  else
    {
      msg.rtype = GDO_PROBE;
    }
  msg.nsize = 2*IASIZE;
  msg.ptype = 0;
  msg.dummy = 0;
  msg.port = 0;
  memcpy(msg.name, from, IASIZE);
  memcpy(&msg.name[IASIZE], to, IASIZE);
  if (l > 0)
    {
      memcpy(&msg.name[msg.nsize], e, l*IASIZE);
      msg.nsize += l*IASIZE;
    }

  queue_msg(&sin, (uptr)&msg, GDO_REQ_SIZE);
}

/* Copyright (c) 2001 Neal H Walfield <neal@cs.uml.edu>.

   This file is placed into the public domain.  Its distribution
   is unlimited.

   THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* NAME

	xgethostname - get the host name.

   SYNOPSIS

   	char *xgethostname (void);

   DESCRIPTION

	The xhostname function is intended to replace gethostname(2), a
	function used to access the host name.  The old interface is
	inflexable given that it assumes the existance of the
	MAXHOSTNAMELEN macro, which neither POSIX nor the proposed
	Single Unix Specification version 3 guarantee to be defined.

   RETURN VALUE

	On success, a malloced, null terminated (possibly truncated)
	string containing the host name is returned.  On failure,
	NULL is returned and errno is set.
 */

char *
xgethostname (void)
{
  int size = 0;
  int addnull = 0;
  char *buf;
  int err;

#if	defined(MAXHOSTNAMELEN)
  size = MAXHOSTNAMELEN;
  addnull = 1;
#else /* MAXHOSTNAMELEN */
#if	defined(_SC_HOST_NAME_MAX)
  size = sysconf (_SC_HOST_NAME_MAX);
  addnull = 1;
#endif /* _SC_HOST_NAME_MAX */
#if	defined(INTERNET_MAX_HOST_NAME_LENGTH)
  size = INTERNET_MAX_HOST_NAME_LENGTH;
  addnull = 1;
#endif
  if (size <= 0)
    size = 256;
#endif /* MAXHOSTNAMELEN */

  buf = malloc (size + addnull);
  if (! buf)
    {
      errno = ENOMEM;
      return NULL;
    }

  err = gethostname (buf, size);
  while (err == -1 && errno == ENAMETOOLONG)
    {
      free (buf);

      size *= 2;
      buf = malloc (size + addnull);
      if (! buf)
	{
	  errno = ENOMEM;
	  return NULL;
	}

      err = gethostname (buf, size);
    }

  if (err)
    {
      if (buf)
        free (buf);
      errno = err;
      return NULL;
    }

  if (addnull)
    buf[size] = '\0';

  return buf;
}
