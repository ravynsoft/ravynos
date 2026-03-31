/*
 * tcp.c - TCP module
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
#include "tcp.h"
#include "tcp.mdh"

/*
 * We use poll() in preference to select because some subset of manuals says
 * that's the thing to do, plus it's a bit less fiddly.  I don't actually
 * have access to a system with poll but not select, however, though
 * both bits of the code have been tested on a machine with both.
 */
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#if defined(HAVE_POLL) && !defined(POLLIN) && !defined(POLLNORM)
# undef HAVE_POLL
#endif

#ifdef USE_LOCAL_H_ERRNO
int h_errno;
#endif

/* We use the RFC 2553 interfaces.  If the functions don't exist in the
 * library, simulate them. */

#ifndef INET_ADDRSTRLEN
# define INET_ADDRSTRLEN 16
#endif

#ifndef INET6_ADDRSTRLEN
# define INET6_ADDRSTRLEN 46
#endif

/**/
#ifndef HAVE_INET_NTOP

/**/
mod_export char const *
zsh_inet_ntop(int af, void const *cp, char *buf, size_t len)
{       
    if (af != AF_INET) {
	errno = EAFNOSUPPORT;
	return NULL;
    } 
    if (len < INET_ADDRSTRLEN) {
	errno = ENOSPC;
	return NULL;
    }
    strcpy(buf, inet_ntoa(*(struct in_addr *)cp));
    return buf;
}

/**/
#else /* !HAVE_INET_NTOP */

/**/
# define zsh_inet_ntop inet_ntop

/**/
#endif /* !HAVE_INET_NTOP */

/**/
#ifndef HAVE_INET_ATON

# ifndef INADDR_NONE
#  define INADDR_NONE 0xffffffffUL
# endif

/**/
mod_export int zsh_inet_aton(char const *src, struct in_addr *dst)
{
    return (dst->s_addr = inet_addr(src)) != INADDR_NONE;
}

/**/
#else /* !HAVE_INET_ATON */

/**/
# define zsh_inet_aton inet_aton

/**/
#endif /* !HAVE_INET_ATON */

/**/
#ifndef HAVE_INET_PTON

/**/
mod_export int
zsh_inet_pton(int af, char const *src, void *dst)
{
    if (af != AF_INET) {
	errno = EAFNOSUPPORT;
	return -1;
    }
    return !!zsh_inet_aton(src, dst);
}

#else /* !HAVE_INET_PTON */

# define zsh_inet_pton inet_pton

/**/
#endif /* !HAVE_INET_PTON */

/**/
#ifndef HAVE_GETIPNODEBYNAME

/**/
# ifndef HAVE_GETHOSTBYNAME2

/**/
mod_export struct hostent *
zsh_gethostbyname2(char const *name, int af)
{
    if (af != AF_INET) {
	h_errno = NO_RECOVERY;
	return NULL;
    }
    return gethostbyname(name);
}

/**/
#else /* !HAVE_GETHOSTBYNAME2 */

/**/
# define zsh_gethostbyname2 gethostbyname2

/**/
# endif /* !HAVE_GETHOSTBYNAME2 */

/* note: this is not a complete implementation.  If ignores the flags,
   and does not provide the memory allocation of the standard interface.
   Each returned structure will overwrite the previous one. */

/**/
mod_export struct hostent *
zsh_getipnodebyname(char const *name, int af, UNUSED(int flags), int *errorp)
{
    static struct hostent ahe;
    static char nbuf[16];
    static char *addrlist[] = { nbuf, NULL };
# ifdef SUPPORT_IPV6
    static char pbuf[INET6_ADDRSTRLEN];
# else
    static char pbuf[INET_ADDRSTRLEN];
# endif
    struct hostent *he;
    if (zsh_inet_pton(af, name, nbuf) == 1) {
	zsh_inet_ntop(af, nbuf, pbuf, sizeof(pbuf));
	ahe.h_name = pbuf;
	ahe.h_aliases = addrlist+1;
	ahe.h_addrtype = af;
	ahe.h_length = (af == AF_INET) ? 4 : 16;
	ahe.h_addr_list = addrlist;
	return &ahe;
    }
    he = zsh_gethostbyname2(name, af);
    if (!he)
	*errorp = h_errno;
    return he;
}

/**/
mod_export void
freehostent(UNUSED(struct hostent *ptr))
{
}

/**/
#else /* !HAVE_GETIPNODEBYNAME */

/**/
# define zsh_getipnodebyname getipnodebyname

/**/
#endif /* !HAVE_GETIPNODEBYNAME */

static LinkList ztcp_sessions;

/* "allocate" a tcp_session */
static Tcp_session
zts_alloc(int ztflags)
{
    Tcp_session sess;

    sess = (Tcp_session)zshcalloc(sizeof(struct tcp_session));
    if (!sess) return NULL;
    sess->fd=-1;
    sess->flags=ztflags;

    zinsertlinknode(ztcp_sessions, lastnode(ztcp_sessions), (void *)sess);

    return sess;
}

/**/
mod_export Tcp_session
tcp_socket(int domain, int type, int protocol, int ztflags)
{
    Tcp_session sess;

    sess = zts_alloc(ztflags);
    if (!sess) return NULL;

    sess->fd = socket(domain, type, protocol);
    /* We'll check failure and tidy up in caller */
    addmodulefd(sess->fd, FDT_MODULE);
    return sess;
}

static int
ztcp_free_session(Tcp_session sess)
{
    zfree(sess, sizeof(struct tcp_session));

    return 0;
}

static int
zts_delete(Tcp_session sess)
{
    LinkNode node;

    node = linknodebydatum(ztcp_sessions, (void *)sess);

    if (!node)
    {
	return 1;
    }

    ztcp_free_session(getdata(node));
    remnode(ztcp_sessions, node);

    return 0;
}

static Tcp_session
zts_byfd(int fd)
{
    LinkNode node;
    
    for (node = firstnode(ztcp_sessions); node; incnode(node))
	if (((Tcp_session)getdata(node))->fd == fd)
	    return (Tcp_session)getdata(node);
    
    return NULL;
}

static void
tcp_cleanup(void)
{
    LinkNode node, next;

    for (node = firstnode(ztcp_sessions); node; node = next) {
	next = node->next;
	tcp_close((Tcp_session)getdata(node));
    }
}

/**/
mod_export int
tcp_close(Tcp_session sess)
{
    int err;
    
    if (sess)
    {  
	if (sess->fd != -1)
	{
	    err = zclose(sess->fd);
	    if (err)
		zwarn("connection close failed: %e", errno);
	}
	zts_delete(sess);
	return 0;
    }

    return -1;
}

/**/
mod_export int
tcp_connect(Tcp_session sess, char *addrp, struct hostent *zhost, int d_port)
{
    int salen;
#ifdef SUPPORT_IPV6
    if (zhost->h_addrtype==AF_INET6) {
	memcpy(&(sess->peer.in6.sin6_addr), addrp, zhost->h_length);
	sess->peer.in6.sin6_port = d_port;
	sess->peer.in6.sin6_flowinfo = 0;
# ifdef HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID
	sess->peer.in6.sin6_scope_id = 0;
# endif
	sess->peer.in6.sin6_family = zhost->h_addrtype;
	salen = sizeof(struct sockaddr_in6);
    } else
#endif /* SUPPORT_IPV6 */
    {
	memcpy(&(sess->peer.in.sin_addr), addrp, zhost->h_length);
	sess->peer.in.sin_port = d_port;
	sess->peer.in.sin_family = zhost->h_addrtype;
	salen = sizeof(struct sockaddr_in);
    }

    return connect(sess->fd, (struct sockaddr *)&(sess->peer), salen);
}

static int
bin_ztcp(char *nam, char **args, Options ops, UNUSED(int func))
{
    int herrno, err=1, destport, force=0, verbose=0, test=0, targetfd=0;
    ZSOCKLEN_T  len;
    char **addrp, *desthost;
    const char *localname, *remotename;
    struct hostent *zthost = NULL, *ztpeer = NULL;
    struct servent *srv;
    Tcp_session sess = NULL;

    if (OPT_ISSET(ops,'f'))
	force = 1;

    if (OPT_ISSET(ops,'v'))
	verbose = 1;

    if (OPT_ISSET(ops,'t'))
        test = 1;

    if (OPT_ISSET(ops,'d')) {
	targetfd = atoi(OPT_ARG(ops,'d'));
	if (!targetfd) {
	    zwarnnam(nam, "%s is an invalid argument to -d", OPT_ARG(ops,'d'));
	    return 1;
	}
    }


    if (OPT_ISSET(ops,'c')) {
	if (!args[0]) {
	    tcp_cleanup();
	}
	else {
	    targetfd = atoi(args[0]);
	    sess = zts_byfd(targetfd);
	    if(!targetfd) {
		zwarnnam(nam, "%s is an invalid argument to -c", args[0]);
		return 1;
	    }

	    if (sess)
	    {
		if ((sess->flags & ZTCP_ZFTP) && !force)
		{
		    zwarnnam(nam, "use -f to force closure of a zftp control connection");
		    return 1;
		}
		tcp_close(sess);
		return 0;
	    }
	    else
	    {
		zwarnnam(nam, "fd %s not found in tcp table", args[0]);
		return 1;
	    }
	}
    }
    else if (OPT_ISSET(ops,'l')) {
	int lport = 0;

	if (!args[0]) {
	    zwarnnam(nam, "-l requires an argument");
	    return 1;
	}

	srv = getservbyname(args[0], "tcp");
	if (srv)
	    lport = srv->s_port;
	else
	    lport = htons(atoi(args[0]));
	if (!lport) { zwarnnam(nam, "bad service name or port number");
	return 1;
	}
	sess = tcp_socket(PF_INET, SOCK_STREAM, 0, ZTCP_LISTEN);

	if (!sess) {
	    zwarnnam(nam, "unable to allocate a TCP session slot");
	    return 1;
	}
#ifdef SO_OOBINLINE
	len = 1;
	setsockopt(sess->fd, SOL_SOCKET, SO_OOBINLINE, (char *)&len, sizeof(len));
#endif
	if (!zsh_inet_aton("0.0.0.0", &(sess->sock.in.sin_addr)))
	{
	    zwarnnam(nam, "bad address: %s", "0.0.0.0");
	    return 1;
	}

	sess->sock.in.sin_family = AF_INET;
	sess->sock.in.sin_port = lport;


	if (bind(sess->fd, (struct sockaddr *)&sess->sock.in, sizeof(struct sockaddr_in)))
	{
	    char buf[DIGBUFSIZE];
	    convbase(buf, (zlong)ntohs(lport), 10);
	    zwarnnam(nam, "could not bind to port %s: %e", buf, errno);
	    tcp_close(sess);
	    return 1;
	}

	if (listen(sess->fd, 1))
	{
	    zwarnnam(nam, "could not listen on socket: %e", errno);
	    tcp_close(sess);
	    return 1;
	}

	if (targetfd) {
	    sess->fd = redup(sess->fd, targetfd);
	}
	else {
	    /* move the fd since no one will want to read from it */
	    sess->fd = movefd(sess->fd);
	}

	if (sess->fd == -1) {
	    zwarnnam(nam, "cannot duplicate fd %d: %e", sess->fd, errno);
	    tcp_close(sess);
	    return 1;
	}

	setiparam_no_convert("REPLY", (zlong)sess->fd);

	if (verbose)
	    printf("%d listener is on fd %d\n", ntohs(sess->sock.in.sin_port), sess->fd);

	return 0;

    }
    else if (OPT_ISSET(ops,'a'))
    {
	int lfd, rfd;

	if (!args[0]) {
	    zwarnnam(nam, "-a requires an argument");
	    return 1;
	}

	lfd = atoi(args[0]);

	if (!lfd) {
	    zwarnnam(nam, "invalid numerical argument");
	    return 1;
	}

	sess = zts_byfd(lfd);
	if (!sess) {
	    zwarnnam(nam, "fd %s is not registered as a tcp connection", args[0]);
	    return 1;
	}

	if (!(sess->flags & ZTCP_LISTEN))
	{
	    zwarnnam(nam, "tcp connection not a listener");
	    return 1;
	}

	if (test) {
#if defined(HAVE_POLL) || defined(HAVE_SELECT)
# ifdef HAVE_POLL
	    struct pollfd pfd;
	    int ret;

	    pfd.fd = lfd;
	    pfd.events = POLLIN;
	    if ((ret = poll(&pfd, 1, 0)) == 0) return 1;
	    else if (ret == -1)
	    {
		zwarnnam(nam, "poll error: %e", errno);
		return 1;
	    }
# else
	    fd_set rfds;
	    struct timeval tv;
	    int ret;
	    
	    FD_ZERO(&rfds);
	    FD_SET(lfd, &rfds);
	    tv.tv_sec = 0;
	    tv.tv_usec = 0;
	    
	    if ((ret = select(lfd+1, &rfds, NULL, NULL, &tv)) == 0) return 1;
	    else if (ret == -1)
	    {
		zwarnnam(nam, "select error: %e", errno);
		return 1;
	    }
	    
# endif
	    
#else
	    zwarnnam(nam, "not currently supported");
	    return 1;
#endif
	}
	sess = zts_alloc(ZTCP_INBOUND);

	len = sizeof(sess->peer.in);
	do {
	    rfd = accept(lfd, (struct sockaddr *)&sess->peer.in, &len);
	} while (rfd < 0 && errno == EINTR && !errflag);

	if (rfd == -1) {
	    zwarnnam(nam, "could not accept connection: %e", errno);
	    tcp_close(sess);
	    return 1;
	}

	/* redup expects fd is already registered */
	addmodulefd(rfd, FDT_MODULE);

	if (targetfd) {
	    sess->fd = redup(rfd, targetfd);
	    if (sess->fd < 0) {
		zerrnam(nam, "could not duplicate socket fd to %d: %e", targetfd, errno);
		return 1;
	    }
	}
	else {
	    sess->fd = rfd;
	}

	setiparam_no_convert("REPLY", (zlong)sess->fd);

	if (verbose)
	    printf("%d is on fd %d\n", ntohs(sess->peer.in.sin_port), sess->fd);
    }
    else
    {
	if (!args[0]) {
	    LinkNode node;
	    for(node = firstnode(ztcp_sessions); node; incnode(node))
	    {
		sess = (Tcp_session)getdata(node);

		if (sess->fd != -1)
		{
		    zthost = gethostbyaddr((const void *)&(sess->sock.in.sin_addr), sizeof(sess->sock.in.sin_addr), AF_INET);
		    if (zthost)
			localname = zthost->h_name;
		    else
			localname = inet_ntoa(sess->sock.in.sin_addr);
		    ztpeer = gethostbyaddr((const void *)&(sess->peer.in.sin_addr), sizeof(sess->peer.in.sin_addr), AF_INET);
		    if (ztpeer)
			remotename = ztpeer->h_name;
		    else
			remotename = inet_ntoa(sess->peer.in.sin_addr);
		    if (OPT_ISSET(ops,'L')) {
			int schar;
			if (sess->flags & ZTCP_ZFTP)
			    schar = 'Z';
			else if (sess->flags & ZTCP_LISTEN)
			    schar = 'L';
			else if (sess->flags & ZTCP_INBOUND)
			    schar = 'I';
			else
			    schar = 'O';
			printf("%d %c %s %d %s %d\n",
			       sess->fd, schar,
			       localname, ntohs(sess->sock.in.sin_port),
			       remotename, ntohs(sess->peer.in.sin_port));
		    } else {
			printf("%s:%d %s %s:%d is on fd %d%s\n",
			       localname, ntohs(sess->sock.in.sin_port),
			       ((sess->flags & ZTCP_LISTEN) ? "-<" :
				((sess->flags & ZTCP_INBOUND) ? "<-" : "->")),
			       remotename, ntohs(sess->peer.in.sin_port),
			       sess->fd,
			       (sess->flags & ZTCP_ZFTP) ? " ZFTP" : "");
		    }
		}
	    }
	    return 0;
	}
	else if (!args[1]) {
	    destport = htons(23);
	}
	else {

	    srv = getservbyname(args[1],"tcp");
	    if (srv)
		destport = srv->s_port;
	    else
		destport = htons(atoi(args[1]));
	}
	
	desthost = ztrdup(args[0]);
	
	zthost = zsh_getipnodebyname(desthost, AF_INET, 0, &herrno);
	if (!zthost || errflag) {
	    zwarnnam(nam, "host resolution failure: %s", desthost);
	    zsfree(desthost);
	    return 1;
	}
	
	sess = tcp_socket(PF_INET, SOCK_STREAM, 0, 0);

	if (!sess) {
	    zwarnnam(nam, "unable to allocate a TCP session slot");
	    zsfree(desthost);
	    return 1;
	}

#ifdef SO_OOBINLINE
	len = 1;
	setsockopt(sess->fd, SOL_SOCKET, SO_OOBINLINE, (char *)&len, sizeof(len));
#endif

	if (sess->fd < 0) {
	    zwarnnam(nam, "socket creation failed: %e", errno);
	    zsfree(desthost);
	    zts_delete(sess);
	    return 1;
	}
	
	for (addrp = zthost->h_addr_list; err && *addrp; addrp++) {
	    if (zthost->h_length != 4)
		zwarnnam(nam, "address length mismatch");
	    do {
		err = tcp_connect(sess, *addrp, zthost, destport);
	    } while (err && errno == EINTR && !errflag);
	}
	
	if (err) {
	    zwarnnam(nam, "connection failed: %e", errno);
	    tcp_close(sess);
	    zsfree(desthost);
	    return 1;
	}
	else
	{
	    if (targetfd) {
		sess->fd = redup(sess->fd, targetfd);
		if (sess->fd < 0) {
		    zerrnam(nam, "could not duplicate socket fd to %d: %e", targetfd, errno);
		    zsfree(desthost);
		    tcp_close(sess);
		    return 1;
		}
	    }

	    setiparam_no_convert("REPLY", (zlong)sess->fd);

	    if (verbose)
		printf("%s:%d is now on fd %d\n",
			desthost, destport, sess->fd);
	}
	
	zsfree(desthost);
    }

    return 0;
}

static struct builtin bintab[] = {
    BUILTIN("ztcp", 0, bin_ztcp, 0, 3, 0, "acd:flLtv", NULL),
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    NULL, 0,
    0
};

/* The load/unload routines required by the zsh library interface */

/**/
int
setup_(UNUSED(Module m))
{
    return 0;
}

/**/
int
features_(Module m, char ***features)
{
    *features = featuresarray(m, &module_features);
    return 0;
}

/**/
int
enables_(Module m, int **enables)
{
    return handlefeatures(m, &module_features, enables);
}

/**/
int
boot_(UNUSED(Module m))
{
    ztcp_sessions = znewlinklist();
    return 0;
}


/**/
int
cleanup_(Module m)
{
    tcp_cleanup();
    freelinklist(ztcp_sessions, (FreeFunc) ztcp_free_session);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
