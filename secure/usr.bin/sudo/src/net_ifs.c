/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2015, 2018-2021
 *	Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

/*
 * Suppress a warning w/ gcc on Digital UN*X.
 * The system headers should really do this....
 */
#if defined(__osf__) && !defined(__cplusplus)
struct mbuf;
struct rtentry;
#endif

/* Avoid a compilation problem with gcc and machine/sys/getppdp.h */
#define _MACHINE_SYS_GETPPDP_INCLUDED

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#if defined(HAVE_SYS_SOCKIO_H) && !defined(SIOCGIFCONF)
# include <sys/sockio.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef NEED_RESOLV_H
# include <arpa/nameser.h>
# include <resolv.h>
#endif /* NEED_RESOLV_H */
#include <net/if.h>
#ifdef HAVE_GETIFADDRS
# include <ifaddrs.h>
#endif

#define NEED_INET_NTOP		/* to expose sudo_inet_ntop in sudo_compat.h */

#define DEFAULT_TEXT_DOMAIN	"sudo"

#include "sudo.h"

/* Minix apparently lacks IFF_LOOPBACK */
#ifndef IFF_LOOPBACK
# define IFF_LOOPBACK	0
#endif

#ifndef INET6_ADDRSTRLEN
# define INET6_ADDRSTRLEN 46
#endif

#ifndef INADDR_NONE
# define INADDR_NONE	0xffffffffU
#endif

#if defined(STUB_LOAD_INTERFACES) || \
    !(defined(HAVE_GETIFADDRS) || defined(SIOCGIFCONF) || defined(SIOCGLIFCONF))

/*
 * Stub function for those without SIOCGIFCONF or getifaddrs()
 */
int
get_net_ifs(char **addrinfo_out)
{
    debug_decl(get_net_ifs, SUDO_DEBUG_NETIF);
    debug_return_int(0);
}

#elif defined(HAVE_GETIFADDRS)

/*
 * Fill in the interfaces string with the machine's ip addresses and netmasks
 * and return the number of interfaces found.  Returns -1 on error.
 */
int
get_net_ifs(char **addrinfo_out)
{
    struct ifaddrs *ifa, *ifaddrs;
    struct sockaddr_in *sin4;
# ifdef HAVE_STRUCT_IN6_ADDR
    struct sockaddr_in6 *sin6;
# endif
    char addrstr[INET6_ADDRSTRLEN], maskstr[INET6_ADDRSTRLEN];
    char *addrinfo = NULL;
    int len, num_interfaces = 0;
    size_t ailen;
    char *cp;
    debug_decl(get_net_ifs, SUDO_DEBUG_NETIF);

    if (!sudo_conf_probe_interfaces())
	debug_return_int(0);

    if (getifaddrs(&ifaddrs) == -1)
	debug_return_int(-1);

    /* Allocate space for the interfaces info string. */
    for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
	/* Skip interfaces marked "down" and "loopback". */
	if (ifa->ifa_addr == NULL || ifa->ifa_netmask == NULL ||
	    !ISSET(ifa->ifa_flags, IFF_UP) || ISSET(ifa->ifa_flags, IFF_LOOPBACK))
	    continue;

	switch (ifa->ifa_addr->sa_family) {
	    case AF_INET:
# ifdef HAVE_STRUCT_IN6_ADDR
	    case AF_INET6:
# endif
		num_interfaces++;
		break;
	}
    }
    if (num_interfaces == 0)
	goto done;
    ailen = (size_t)num_interfaces * 2 * INET6_ADDRSTRLEN;
    if ((cp = malloc(ailen)) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	goto bad;
    }
    addrinfo = cp;

    for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
	/* Skip interfaces marked "down" and "loopback". */
	if (ifa->ifa_addr == NULL || ifa->ifa_netmask == NULL ||
	    !ISSET(ifa->ifa_flags, IFF_UP) || ISSET(ifa->ifa_flags, IFF_LOOPBACK))
		continue;

	switch (ifa->ifa_addr->sa_family) {
	case AF_INET:
	    sin4 = (struct sockaddr_in *)ifa->ifa_addr;
	    if (sin4->sin_addr.s_addr == INADDR_ANY || sin4->sin_addr.s_addr == INADDR_NONE) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring unspecified AF_INET addr for %s", ifa->ifa_name);
		continue;
	    }
	    if (inet_ntop(AF_INET, &sin4->sin_addr, addrstr, sizeof(addrstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET addr for %s", ifa->ifa_name);
		continue;
	    }
	    sin4 = (struct sockaddr_in *)ifa->ifa_netmask;
	    if (inet_ntop(AF_INET, &sin4->sin_addr, maskstr, sizeof(maskstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET mask for %s", ifa->ifa_name);
		continue;
	    }
	    break;
# ifdef HAVE_STRUCT_IN6_ADDR
	case AF_INET6:
	    sin6 = (struct sockaddr_in6 *)ifa->ifa_addr;
	    if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring unspecified AF_INET6 addr for %s", ifa->ifa_name);
		continue;
	    }
	    if (inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, sizeof(addrstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET6 addr for %s", ifa->ifa_name);
		continue;
	    }
	    sin6 = (struct sockaddr_in6 *)ifa->ifa_netmask;
	    if (inet_ntop(AF_INET6, &sin6->sin6_addr, maskstr, sizeof(maskstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET6 mask for %s", ifa->ifa_name);
		continue;
	    }
	    break;
# endif /* HAVE_STRUCT_IN6_ADDR */
	default:
	    continue;
	}

	/* Store the IP addr/netmask pairs. */
	len = snprintf(cp, ailen, "%s%s/%s",
	    cp == addrinfo ? "" : " ", addrstr, maskstr);
	if (len < 0 || (size_t)len >= ailen) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    goto bad;
	}
	cp += len;
	ailen -= (size_t)len;
    }
    *addrinfo_out = addrinfo;
    goto done;

bad:
    free(addrinfo);
    num_interfaces = -1;
done:
# ifdef HAVE_FREEIFADDRS
    freeifaddrs(ifaddrs);
# else
    free(ifaddrs);
# endif
    debug_return_int(num_interfaces);
}

#elif defined(SIOCGLIFCONF)

# if defined(__hpux)

/*
 * Fill in the interfaces string with the machine's ip addresses and netmasks
 * and return the number of interfaces found.  Returns -1 on error.
 * HP-UX has incompatible SIOCGLIFNUM and SIOCGLIFCONF ioctls.
 */
int
get_net_ifs(char **addrinfo_out)
{
    struct if_laddrconf laddrconf;
    struct ifconf ifconf;
    char addrstr[INET6_ADDRSTRLEN], maskstr[INET6_ADDRSTRLEN];
    char *addrinfo = NULL;
    int i, n, sock4, sock6 = -1;
    int num_interfaces = 0;
    size_t ailen;
    char *cp;
    debug_decl(get_net_ifs, SUDO_DEBUG_NETIF);

    if (!sudo_conf_probe_interfaces())
	debug_return_int(0);

    memset(&ifconf, 0, sizeof(ifconf));
    memset(&laddrconf, 0, sizeof(laddrconf));

    /* Allocate and fill in the IPv4 interface list. */
    sock4 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock4 != -1 && ioctl(sock4, SIOCGIFNUM, &n) != -1) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "SIOCGIFNUM reports %d interfaces", n);
	n += 4;	/* in case new interfaces come up */

	ifconf.ifc_len = n * sizeof(struct ifreq);
	ifconf.ifc_buf = malloc(ifconf.ifc_len);
	if (ifconf.ifc_buf == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	    goto bad;
	}

	if (ioctl(sock4, SIOCGIFCONF, &ifconf) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"unable to get interface list (SIOCGIFCONF)");
	    goto bad;
	}
    }

    /* Allocate and fill in the IPv6 interface list. */
    sock6 = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock6 != -1 && ioctl(sock6, SIOCGLIFNUM, &n) != -1) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "SIOCGLIFNUM reports %d interfaces", n);
	n += 4;	/* in case new interfaces come up */

	laddrconf.iflc_len = n * sizeof(struct if_laddrreq);
	laddrconf.iflc_buf = malloc(laddrconf.iflc_len);
	if (laddrconf.iflc_buf == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	    goto bad;
	}

	if (ioctl(sock4, SIOCGLIFCONF, &laddrconf) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"unable to get interface list (SIOCGLIFCONF)");
	    goto bad;
	}
    }

    /* Allocate space for the maximum number of interfaces that could exist. */
    n = ifconf.ifc_len / sizeof(struct ifconf) +
	laddrconf.iflc_len / sizeof(struct if_laddrreq);
    if (n == 0)
	goto done;
    ailen = n * 2 * INET6_ADDRSTRLEN;
    if ((cp = malloc(ailen)) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	goto bad;
    }
    addrinfo = cp;

    /*
     * For each interface, store the ip address and netmask.
     * Keep a copy of the address family, else it will be overwritten.
     */
    for (i = 0; i < ifconf.ifc_len; ) {
	struct ifreq *ifr = (struct ifreq *)&ifconf.ifc_buf[i];
	struct sockaddr_in *sin4;

	/* Set i to the subscript of the next interface (no sa_len). */
	i += sizeof(struct ifreq);

	/* IPv4 only. */
	if (ifr->ifr_addr.sa_family != AF_INET) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unexpected address family %d for %s",
		ifr->ifr_addr.sa_family, ifr->ifr_name);
	    continue;
	}

	/* Store the address. */
	sin4 = (struct sockaddr_in *)&ifr->ifr_addr;
	if (sin4->sin_addr.s_addr == INADDR_ANY || sin4->sin_addr.s_addr == INADDR_NONE) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"ignoring unspecified AF_INET addr for %s", ifr->ifr_name);
	    continue;
	}
	if (inet_ntop(AF_INET, &sin4->sin_addr, addrstr, sizeof(addrstr)) == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"ignoring bad AF_INET addr for %s", ifr->ifr_name);
	    continue;
	}

	/* Skip interfaces marked "down" and "loopback". */
	if (ioctl(sock4, SIOCGIFFLAGS, ifr) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"SIOCGLIFFLAGS for %s", ifr->ifr_name);
	    continue;
	}
	if (!ISSET(ifr->ifr_flags, IFF_UP) ||
	    ISSET(ifr->ifr_flags, IFF_LOOPBACK))
		continue;

	/* Fetch and store the netmask. */
	if (ioctl(sock4, SIOCGIFNETMASK, ifr) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"SIOCGLIFNETMASK for %s", ifr->ifr_name);
	    continue;
	}

	/* Convert the mask to string form. */
	sin4 = (struct sockaddr_in *)&ifr->ifr_addr;
	if (inet_ntop(AF_INET, &sin4->sin_addr, maskstr, sizeof(maskstr)) == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"ignoring bad AF_INET mask for %s", ifr->ifr_name);
	    continue;
	}

	n = snprintf(cp, ailen, "%s%s/%s",
	    cp == addrinfo ? "" : " ", addrstr, maskstr);
	if (n < 0 || (size_t)n >= ailen) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    goto bad;
	}
	cp += n;
	ailen -= n;

	num_interfaces++;
    }
    for (i = 0; i < laddrconf.iflc_len; ) {
	struct if_laddrreq *lreq = (struct if_laddrreq *)&laddrconf.iflc_buf[i];
	struct sockaddr_in6 *sin6;

	/* Set i to the subscript of the next interface (no sa_len). */
	i += sizeof(struct if_laddrreq);

	/* IPv6 only. */
	if (lreq->iflr_addr.sa_family != AF_INET6) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unexpected address family %d for %s",
		lreq->iflr_addr.sa_family, lreq->iflr_name);
	    continue;
	}

	sin6 = (struct sockaddr_in6 *)&lreq->iflr_addr;
	if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"ignoring unspecified AF_INET6 addr for %s", lreq->iflr_name);
	    continue;
	}
	if (inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, sizeof(addrstr)) == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"ignoring bad AF_INET6 addr for %s", lreq->iflr_name);
	    continue;
	}

	/* Skip interfaces marked "down" and "loopback". */
	if (ioctl(sock6, SIOCGLIFFLAGS, lreq) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"SIOCGLIFFLAGS for %s", lreq->iflr_name);
	    continue;
	}
	if (!ISSET(lreq->iflr_flags, IFF_UP) ||
	    ISSET(lreq->iflr_flags, IFF_LOOPBACK))
		continue;

	/* Fetch and store the netmask. */
	if (ioctl(sock6, SIOCGLIFNETMASK, lreq) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"SIOCGLIFNETMASK for %s", lreq->iflr_name);
	    continue;
	}
	sin6 = (struct sockaddr_in6 *)&lreq->iflr_addr;
	if (inet_ntop(AF_INET6, &sin6->sin6_addr, maskstr, sizeof(maskstr)) == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"ignoring bad AF_INET6 mask for %s", lreq->iflr_name);
	    continue;
	}

	n = snprintf(cp, ailen, "%s%s/%s",
	    cp == addrinfo ? "" : " ", addrstr, maskstr);
	if (n < 0 || (size_t)n >= ailen) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    goto bad;
	}
	cp += n;
	ailen -= n;

	num_interfaces++;
    }
    *addrinfo_out = addrinfo;
    goto done;

bad:
    free(addrinfo);
    num_interfaces = -1;
done:
    free(ifconf.ifc_buf);
    free(laddrconf.iflc_buf);
    if (sock4 != -1)
	close(sock4);
    if (sock6 != -1)
	close(sock6);

    debug_return_int(num_interfaces);
}

# else

/*
 * Fill in the interfaces string with the machine's ip addresses and netmasks
 * and return the number of interfaces found.  Returns -1 on error.
 * SIOCGLIFCONF version (IPv6 compatible).
 */
int
get_net_ifs(char **addrinfo_out)
{
    struct lifconf lifconf;
    struct lifnum lifn;
    struct sockaddr_in *sin4;
    struct sockaddr_in6 *sin6;
    char addrstr[INET6_ADDRSTRLEN], maskstr[INET6_ADDRSTRLEN];
    char *addrinfo = NULL;
    int i, n, sock, sock4, sock6 = -1;
    int num_interfaces = 0;
    size_t ailen;
    char *cp;
    debug_decl(get_net_ifs, SUDO_DEBUG_NETIF);

    if (!sudo_conf_probe_interfaces())
	debug_return_int(0);

    /* We need both INET4 and INET6 sockets to get flags and netmask. */
    sock4 = socket(AF_INET, SOCK_DGRAM, 0);
    sock6 = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock4 == -1 && sock6 == -1)
	debug_return_int(-1);

    /* Use INET6 socket with SIOCGLIFCONF if possible (may not matter). */
    sock = sock6 != -1 ? sock6 : sock4;

    /* Get number of interfaces if possible. */
    memset(&lifn, 0, sizeof(lifn));
    if (ioctl(sock, SIOCGLIFNUM, &lifn) != -1) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "SIOCGLIFNUM reports %d interfaces", lifn.lifn_count);
	lifn.lifn_count += 4;		/* in case new interfaces come up */
    } else {
	lifn.lifn_count = 512;
    }

    /* Allocate and fill in the interface buffer. */
    memset(&lifconf, 0, sizeof(lifconf));
    lifconf.lifc_len = lifn.lifn_count * sizeof(struct lifreq);
    lifconf.lifc_buf = malloc(lifconf.lifc_len);
    if (lifconf.lifc_buf == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	goto bad;
    }
    if (ioctl(sock, SIOCGLIFCONF, &lifconf) < 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to get interface list (SIOCGLIFCONF)");
	goto bad;
    }

    /* Allocate space for the maximum number of interfaces that could exist. */
    n = lifconf.lifc_len / sizeof(struct lifreq);
    if (n == 0)
	goto done;
    ailen = n * 2 * INET6_ADDRSTRLEN;
    if ((cp = malloc(ailen)) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	goto bad;
    }
    addrinfo = cp;

    /*
     * For each interface, store the ip address and netmask.
     * Keep a copy of the address family, else it will be overwritten.
     */
    for (i = 0; i < lifconf.lifc_len; ) {
	struct lifreq *lifr = (struct lifreq *)&lifconf.lifc_buf[i];
	const int family = lifr->lifr_addr.ss_family;

	/* Set i to the subscript of the next interface (no sa_len). */
	i += sizeof(struct lifreq);

	/* Store the address. */
	switch (family) {
	case AF_INET:
	    sin4 = (struct sockaddr_in *)&lifr->lifr_addr;
	    if (sin4->sin_addr.s_addr == INADDR_ANY || sin4->sin_addr.s_addr == INADDR_NONE) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring unspecified AF_INET addr for %s", lifr->lifr_name);
		continue;
	    }
	    if (inet_ntop(AF_INET, &sin4->sin_addr, addrstr, sizeof(addrstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET addr for %s", lifr->lifr_name);
		continue;
	    }
	    sock = sock4;
	    break;
	case AF_INET6:
	    sin6 = (struct sockaddr_in6 *)&lifr->lifr_addr;
	    if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring unspecified AF_INET6 addr for %s", lifr->lifr_name);
		continue;
	    }
	    if (inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, sizeof(addrstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET6 addr for %s", lifr->lifr_name);
		continue;
	    }
	    sock = sock6;
	    break;
	default:
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"ignoring address with family %d for %s",
		family, lifr->lifr_name);
	    continue;
	}

	/* Skip interfaces marked "down" and "loopback". */
	if (ioctl(sock, SIOCGLIFFLAGS, lifr) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"SIOCGLIFFLAGS for %s", lifr->lifr_name);
	    continue;
	}
	if (!ISSET(lifr->lifr_flags, IFF_UP) ||
	    ISSET(lifr->lifr_flags, IFF_LOOPBACK))
		continue;

	/* Fetch and store the netmask. */
	if (ioctl(sock, SIOCGLIFNETMASK, lifr) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"SIOCGLIFNETMASK for %s", lifr->lifr_name);
	    continue;
	}
	switch (family) {
	case AF_INET:
	    sin4 = (struct sockaddr_in *)&lifr->lifr_addr;
	    if (inet_ntop(AF_INET, &sin4->sin_addr, maskstr, sizeof(maskstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET mask for %s", lifr->lifr_name);
		continue;
	    }
	    break;
	case AF_INET6:
	    sin6 = (struct sockaddr_in6 *)&lifr->lifr_addr;
	    if (inet_ntop(AF_INET6, &sin6->sin6_addr, maskstr, sizeof(maskstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET6 mask for %s", lifr->lifr_name);
		continue;
	    }
	    break;
	default:
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unexpected address family %d for %s",
		family, lifr->lifr_name);
	    continue;
	}

	n = snprintf(cp, ailen, "%s%s/%s",
	    cp == addrinfo ? "" : " ", addrstr, maskstr);
	if (n < 0 || (size_t)n >= ailen) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    goto bad;
	}
	cp += n;
	ailen -= n;

	num_interfaces++;
    }
    *addrinfo_out = addrinfo;
    goto done;

bad:
    free(addrinfo);
    num_interfaces = -1;
done:
    free(lifconf.lifc_buf);
    if (sock4 != -1)
	close(sock4);
    if (sock6 != -1)
	close(sock6);

    debug_return_int(num_interfaces);
}
# endif /* !__hpux */

#elif defined(SIOCGIFCONF)

/*
 * Fill in the interfaces string with the machine's ip addresses and netmasks
 * and return the number of interfaces found.  Returns -1 on error.
 * SIOCGIFCONF version.
 */
int
get_net_ifs(char **addrinfo_out)
{
    struct ifconf ifconf;
    struct ifreq *ifr;
    struct sockaddr_in *sin4;
# ifdef HAVE_STRUCT_IN6_ADDR
    struct sockaddr_in6 *sin6;
# endif
    char addrstr[INET6_ADDRSTRLEN], maskstr[INET6_ADDRSTRLEN];
    char *addrinfo = NULL;
    int i, n, sock, sock4, sock6 = -1;
    int num_interfaces = 0;
    size_t ailen, buflen;
    char *cp, *ifconf_buf = NULL;
    debug_decl(get_net_ifs, SUDO_DEBUG_NETIF);

    if (!sudo_conf_probe_interfaces())
	debug_return_int(0);

    sock4 = socket(AF_INET, SOCK_DGRAM, 0);
# ifdef HAVE_STRUCT_IN6_ADDR
    sock6 = socket(AF_INET6, SOCK_DGRAM, 0);
# endif
    if (sock4 == -1 && sock6 == -1)
	debug_return_int(-1);

    /* Use INET6 socket with SIOCGIFCONF if possible (may not matter). */
    sock = sock6 != -1 ? sock6 : sock4;

    /*
     * Get the size of the interface buffer (if possible).
     * We over-allocate a bit in case interfaces come up afterward.
     */
    i = 0;
# if defined(SIOCGSIZIFCONF)
    /* AIX */
    if (ioctl(sock, SIOCGSIZIFCONF, &i) != -1) {
	buflen = i + (sizeof(struct ifreq) * 4);
    } else
# elif defined(SIOCGIFANUM)
    /* SCO OpenServer 5/6 */
    if (ioctl(sock, SIOCGIFANUM, &i) != -1) {
	buflen = (i + 4) * sizeof(struct ifreq);
    } else
# elif defined(SIOCGIFNUM)
    /* HP-UX, Solaris, others? */
    if (ioctl(sock, SIOCGIFNUM, &i) != -1) {
	buflen = (i + 4) * sizeof(struct ifreq);
    } else
# endif
    {
	buflen = 256 * sizeof(struct ifreq);
    }

    /* Get interface configuration. */
    memset(&ifconf, 0, sizeof(ifconf));
    for (i = 0; i < 4; i++) {
	ifconf.ifc_len = buflen;
	ifconf.ifc_buf = malloc(buflen);
	if (ifconf.ifc_buf == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	    goto bad;
	}

	/* Note that some kernels return EINVAL if the buffer is too small */
	if (ioctl(sock, SIOCGIFCONF, &ifconf) < 0 && errno != EINVAL)
	    goto bad;

	/* Break out of loop if we have a big enough buffer. */
	if (ifconf.ifc_len + sizeof(struct ifreq) < buflen)
	    break;
	buflen *= 2;
	free(ifconf.ifc_buf);
    }

    /*
     * Allocate space for the maximum number of interfaces that could exist.
     * We walk the list for systems with sa_len in struct sockaddr.
     */
    for (i = 0, n = 0; i < ifconf.ifc_len; n++) {
	/* Set i to the subscript of the next interface. */
	i += sizeof(struct ifreq);
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
	ifr = (struct ifreq *)&ifconf.ifc_buf[i];
	if (ifr->ifr_addr.sa_len > sizeof(ifr->ifr_addr))
	    i += ifr->ifr_addr.sa_len - sizeof(struct sockaddr);
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */
    }
    if (n == 0)
	goto done;
    ailen = n * 2 * INET6_ADDRSTRLEN;
    if ((cp = malloc(ailen)) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	goto bad;
    }
    addrinfo = cp;

    /*
     * For each interface, store the ip address and netmask.
     * Keep a copy of the address family, else it will be overwritten.
     */
    for (i = 0; i < ifconf.ifc_len; ) {
	int family;

	ifr = (struct ifreq *)&ifconf.ifc_buf[i];
	family = ifr->ifr_addr.sa_family;

	/* Set i to the subscript of the next interface. */
	i += sizeof(struct ifreq);
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
	if (ifr->ifr_addr.sa_len > sizeof(ifr->ifr_addr))
	    i += ifr->ifr_addr.sa_len - sizeof(struct sockaddr);
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */

	/* Store the address. */
	switch (family) {
	case AF_INET:
	    sin4 = (struct sockaddr_in *)&ifr->ifr_addr;
	    if (sin4->sin_addr.s_addr == INADDR_ANY || sin4->sin_addr.s_addr == INADDR_NONE) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring unspecified AF_INET addr for %s", ifr->ifr_name);
		continue;
	    }
	    if (inet_ntop(AF_INET, &sin4->sin_addr, addrstr, sizeof(addrstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET addr for %s", ifr->ifr_name);
		continue;
	    }
	    sock = sock4;
	    break;
# ifdef HAVE_STRUCT_IN6_ADDR
	case AF_INET6:
	    sin6 = (struct sockaddr_in6 *)&ifr->ifr_addr;
	    if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring unspecified AF_INET6 addr for %s", ifr->ifr_name);
		continue;
	    }
	    if (inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, sizeof(addrstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET6 addr for %s", ifr->ifr_name);
		continue;
	    }
	    sock = sock6;
	    break;
# endif /* HAVE_STRUCT_IN6_ADDR */
	default:
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unexpected address family %d for %s",
		family, ifr->ifr_name);
	    continue;
	}

	/* Skip interfaces marked "down" and "loopback". */
	if (ioctl(sock, SIOCGIFFLAGS, ifr) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"SIOCGLIFFLAGS for %s", ifr->ifr_name);
	    continue;
	}
	if (!ISSET(ifr->ifr_flags, IFF_UP) ||
	    ISSET(ifr->ifr_flags, IFF_LOOPBACK))
		continue;

	/* Fetch and store the netmask. */
	if (ioctl(sock, SIOCGIFNETMASK, ifr) < 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"SIOCGLIFNETMASK for %s", ifr->ifr_name);
	    continue;
	}

	/* Convert the mask to string form. */
	switch (family) {
	case AF_INET:
	    sin4 = (struct sockaddr_in *)&ifr->ifr_addr;
	    if (inet_ntop(AF_INET, &sin4->sin_addr, maskstr, sizeof(maskstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET mask for %s", ifr->ifr_name);
		continue;
	    }
	    break;
# ifdef HAVE_STRUCT_IN6_ADDR
	case AF_INET6:
	    sin6 = (struct sockaddr_in6 *)&ifr->ifr_addr;
	    if (inet_ntop(AF_INET6, &sin6->sin6_addr, maskstr, sizeof(maskstr)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "ignoring bad AF_INET6 mask for %s", ifr->ifr_name);
		continue;
	    }
	    break;
# endif /* HAVE_STRUCT_IN6_ADDR */
	default:
	    continue;
	}

	n = snprintf(cp, ailen, "%s%s/%s",
	    cp == addrinfo ? "" : " ", addrstr, maskstr);
	if (n < 0 || (size_t)n >= ailen) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    goto bad;
	}
	cp += n;
	ailen -= n;

	num_interfaces++;
    }
    *addrinfo_out = addrinfo;
    goto done;

bad:
    free(addrinfo);
    num_interfaces = -1;
done:
    free(ifconf_buf);
    if (sock4 != -1)
        close(sock4);
    if (sock6 != -1)
        close(sock6);

    debug_return_int(num_interfaces);
}

#endif /* SIOCGIFCONF */
