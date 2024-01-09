/*
 * tcp.h - builtin FTP client
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1998-2001 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development
 * Group be liable to any party for direct, indirect, special, incidental,
 * or consequential damages arising out of the use of this software and
 * its documentation, even if Peter Stephenson, and the Zsh
 * Development Group have been advised of the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically
 * disclaim any warranties, including, but not limited to, the implied
 * warranties of merchantability and fitness for a particular purpose.  The
 * software provided hereunder is on an "as is" basis, and Peter Stephenson
 * and the Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

/*
 * We need to include the zsh headers later to avoid clashes with
 * the definitions on some systems, however we need the configuration
 * file to decide whether we can include netinet/in_systm.h, which
 * doesn't exist on cygwin.
 */
#include "../../config.h"

#include <sys/types.h>
#include <sys/socket.h>

#ifdef HAVE_BIND_NETDB_H
/*
 * On systems where we're using -lbind, this has more definitions
 * than the standard header.
 */
#include <bind/netdb.h>
#else
#include <netdb.h>
#endif

/*
 * For some reason, configure doesn't always detect netinet/in_systm.h.
 * On some systems, including linux, this seems to be because gcc is
 * throwing up a warning message about the redefinition of
 * __USE_LARGEFILE.  This means the problem is somewhere in the
 * header files where we can't get at it.  For now, revert to
 * not including this file only on systems where we know it's missing.
 * Currently this is just some older versions of cygwin.
 */
#if defined(HAVE_NETINET_IN_SYSTM_H) || !defined(__CYGWIN__)
# include <netinet/in_systm.h>
#endif
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* Is IPv6 supported by the library? */

#if defined(AF_INET6) && defined(IN6ADDR_LOOPBACK_INIT) \
	&& defined(HAVE_INET_NTOP) && defined(HAVE_INET_PTON)
# define SUPPORT_IPV6 1
#endif

union tcp_sockaddr {
    struct sockaddr a;
    struct sockaddr_in in;
#ifdef SUPPORT_IPV6
    struct sockaddr_in6 in6;
#endif
};

typedef struct tcp_session *Tcp_session;

#define ZTCP_LISTEN  1
#define ZTCP_INBOUND 2
#define ZTCP_ZFTP    16

struct tcp_session {
    int fd;				/* file descriptor */
    union tcp_sockaddr sock;  	/* local address   */
    union tcp_sockaddr peer;  	/* remote address  */
    int flags;
};

#include "tcp.pro"

#ifndef INET_ADDRSTRLEN
# define INET_ADDRSTRLEN 16
#endif

#ifndef INET6_ADDRSTRLEN
# define INET6_ADDRSTRLEN 46
#endif
