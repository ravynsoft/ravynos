/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007, 2010-2013
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

#ifndef SUDOERS_INTERFACES_H
#define SUDOERS_INTERFACES_H

/*
 * Union to hold either strucr in_addr or in6_add
 */
union sudo_in_addr_un {
    struct in_addr ip4;
#ifdef HAVE_STRUCT_IN6_ADDR
    struct in6_addr ip6;
#endif
};

/*
 * IP address and netmask pairs for checking against local interfaces.
 */
struct interface {
    SLIST_ENTRY(interface) entries;
    unsigned int family;	/* AF_INET or AF_INET6 */
    union sudo_in_addr_un addr;
    union sudo_in_addr_un netmask;
};

SLIST_HEAD(interface_list, interface);

/*
 * Prototypes for external functions.
 */
int get_net_ifs(char **addrinfo);
void dump_interfaces(const char *);
bool set_interfaces(const char *);
struct interface_list *get_interfaces(void);

#endif /* SUDOERS_INTERFACES_H */
