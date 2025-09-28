/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2015
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

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef NEED_RESOLV_H
# include <arpa/nameser.h>
# include <resolv.h>
#endif /* NEED_RESOLV_H */

#include <sudoers.h>
#include <interfaces.h>

static int
addr_matches_if(const char *n)
{
    union sudo_in_addr_un addr;
    struct interface *ifp;
#ifdef HAVE_STRUCT_IN6_ADDR
    size_t j;
#endif
    unsigned int family;
    debug_decl(addr_matches_if, SUDOERS_DEBUG_MATCH);

#ifdef HAVE_STRUCT_IN6_ADDR
    if (inet_pton(AF_INET6, n, &addr.ip6) == 1) {
	family = AF_INET6;
    } else
#endif /* HAVE_STRUCT_IN6_ADDR */
    if (inet_pton(AF_INET, n, &addr.ip4) == 1) {
	family = AF_INET;
    } else {
	debug_return_int(DENY);
    }

    SLIST_FOREACH(ifp, get_interfaces(), entries) {
	if (ifp->family != family)
	    continue;
	switch (family) {
	    case AF_INET:
		if (ifp->addr.ip4.s_addr == addr.ip4.s_addr ||
		    (ifp->addr.ip4.s_addr & ifp->netmask.ip4.s_addr)
		    == addr.ip4.s_addr)
		    debug_return_int(ALLOW);
		break;
#ifdef HAVE_STRUCT_IN6_ADDR
	    case AF_INET6:
		if (memcmp(ifp->addr.ip6.s6_addr, addr.ip6.s6_addr,
		    sizeof(addr.ip6.s6_addr)) == 0)
		    debug_return_int(ALLOW);
		for (j = 0; j < sizeof(addr.ip6.s6_addr); j++) {
		    if ((ifp->addr.ip6.s6_addr[j] & ifp->netmask.ip6.s6_addr[j]) != addr.ip6.s6_addr[j])
			break;
		}
		if (j == sizeof(addr.ip6.s6_addr))
		    debug_return_int(ALLOW);
		break;
#endif /* HAVE_STRUCT_IN6_ADDR */
	}
    }

    debug_return_int(DENY);
}

static int
addr_matches_if_netmask(const char *n, const char *m)
{
    size_t i;
    union sudo_in_addr_un addr, mask;
    struct interface *ifp;
#ifdef HAVE_STRUCT_IN6_ADDR
    size_t j;
#endif
    unsigned int family;
    const char *errstr;
    debug_decl(addr_matches_if, SUDOERS_DEBUG_MATCH);

#ifdef HAVE_STRUCT_IN6_ADDR
    if (inet_pton(AF_INET6, n, &addr.ip6) == 1)
	family = AF_INET6;
    else
#endif /* HAVE_STRUCT_IN6_ADDR */
    if (inet_pton(AF_INET, n, &addr.ip4) == 1) {
	family = AF_INET;
    } else {
	debug_return_int(DENY);
    }

    if (family == AF_INET) {
	if (strchr(m, '.')) {
	    if (inet_pton(AF_INET, m, &mask.ip4) != 1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "IPv4 netmask %s: %s", m, "invalid value");
		debug_return_int(DENY);
	    }
	} else {
	    i = (size_t)sudo_strtonum(m, 1, 32, &errstr);
	    if (errstr != NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "IPv4 netmask %s: %s", m, errstr);
		debug_return_int(DENY);
	    }
	    mask.ip4.s_addr = htonl(0xffffffffU << (32 - i));
	}
	addr.ip4.s_addr &= mask.ip4.s_addr;
    }
#ifdef HAVE_STRUCT_IN6_ADDR
    else {
	if (inet_pton(AF_INET6, m, &mask.ip6) != 1) {
	    j = (size_t)sudo_strtonum(m, 1, 128, &errstr);
	    if (errstr != NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "IPv6 netmask %s: %s", m, errstr);
		debug_return_int(DENY);
	    }
	    for (i = 0; i < sizeof(addr.ip6.s6_addr); i++) {
		if (j < i * 8)
		    mask.ip6.s6_addr[i] = 0;
		else if (i * 8 + 8 <= j)
		    mask.ip6.s6_addr[i] = 0xff;
		else
		    mask.ip6.s6_addr[i] = 0xff00 >> (j - i * 8);
		addr.ip6.s6_addr[i] &= mask.ip6.s6_addr[i];
	    }
	}
    }
#endif /* HAVE_STRUCT_IN6_ADDR */

    SLIST_FOREACH(ifp, get_interfaces(), entries) {
	if (ifp->family != family)
	    continue;
	switch (family) {
	    case AF_INET:
		if ((ifp->addr.ip4.s_addr & mask.ip4.s_addr) == addr.ip4.s_addr)
		    debug_return_int(ALLOW);
		break;
#ifdef HAVE_STRUCT_IN6_ADDR
	    case AF_INET6:
		for (j = 0; j < sizeof(addr.ip6.s6_addr); j++) {
		    if ((ifp->addr.ip6.s6_addr[j] & mask.ip6.s6_addr[j]) != addr.ip6.s6_addr[j])
			break;
		}
		if (j == sizeof(addr.ip6.s6_addr))
		    debug_return_int(ALLOW);
		break;
#endif /* HAVE_STRUCT_IN6_ADDR */
	}
    }

    debug_return_int(DENY);
}

/*
 * Returns ALLOW if "n" is one of our ip addresses or if
 * "n" is a network that we are on, else returns DENY.
 */
int
addr_matches(char *n)
{
    char *m;
    int ret;
    debug_decl(addr_matches, SUDOERS_DEBUG_MATCH);

    /* If there's an explicit netmask, use it. */
    if ((m = strchr(n, '/'))) {
	*m++ = '\0';
	ret = addr_matches_if_netmask(n, m);
	*(m - 1) = '/';
    } else
	ret = addr_matches_if(n);

    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"IP address %s matches local host: %s", n,
	ret == ALLOW ? "ALLOW" : "DENY");
    debug_return_int(ret);
}
