/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if.h>
#include <linux/if_arp.h>

#include <libdaemon/dlog.h>

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>

#ifndef IFLA_RTA
#include <linux/if_addr.h>
#define IFLA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#endif

#ifndef IFA_RTA
#include <linux/if_addr.h>
#define IFA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#endif

#include "iface.h"

static int fd = -1;
static int ifindex = -1;

typedef struct Address Address;

struct Address {
    uint32_t address;
    AVAHI_LLIST_FIELDS(Address, addresses);
};

AVAHI_LLIST_HEAD(Address, addresses) = NULL;

int iface_init(int i) {
    struct sockaddr_nl addr;
    int on = 1;

    if ((fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) {
        daemon_log(LOG_ERR, "socket(PF_NETLINK): %s", strerror(errno));
        goto fail;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups =  RTMGRP_LINK|RTMGRP_IPV4_IFADDR;
    addr.nl_pid = getpid();

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        daemon_log(LOG_ERR, "bind(): %s", strerror(errno));
        goto fail;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0) {
        daemon_log(LOG_ERR, "SO_PASSCRED: %s", strerror(errno));
        goto fail;
    }

    ifindex = i;

    return fd;

fail:
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }

    return -1;
}

static int process_nlmsg(struct nlmsghdr *n) {
    assert(n);

    if (n->nlmsg_type == RTM_NEWLINK || n->nlmsg_type == RTM_DELLINK) {
        /* A link appeared or was removed */

        struct ifinfomsg *ifi;
        ifi = NLMSG_DATA(n);

        if (ifi->ifi_family != AF_UNSPEC || (int) ifi->ifi_index != ifindex)
            return 0;

        if (n->nlmsg_type == RTM_DELLINK) {
            daemon_log(LOG_ERR, "Interface vanished.");
            return -1;
        }

        assert(n->nlmsg_type == RTM_NEWLINK);

        if ((ifi->ifi_flags & IFF_LOOPBACK) ||
            (ifi->ifi_flags & IFF_NOARP) ||
            ifi->ifi_type != ARPHRD_ETHER) {
            daemon_log(LOG_ERR, "Interface not suitable.");
            return -1;
        }

    } else if (n->nlmsg_type == RTM_NEWADDR || n->nlmsg_type == RTM_DELADDR) {

        /* An address was added or removed */

        struct rtattr *a = NULL;
        struct ifaddrmsg *ifa;
        int l;
        uint32_t address = 0;
        Address *i;

        ifa = NLMSG_DATA(n);

        if (ifa->ifa_family != AF_INET || (int) ifa->ifa_index != ifindex)
            return 0;

        l = NLMSG_PAYLOAD(n, sizeof(*ifa));
        a = IFA_RTA(ifa);

        while(RTA_OK(a, l)) {

            switch(a->rta_type) {
                case IFA_LOCAL:
                case IFA_ADDRESS:
                    assert(RTA_PAYLOAD(a) == 4);
                    memcpy(&address, RTA_DATA(a), sizeof(uint32_t));
                    break;
            }

            a = RTA_NEXT(a, l);
        }

        if (!address || is_ll_address(address))
            return 0;

        for (i = addresses; i; i = i->addresses_next)
            if (i->address == address)
                break;

        if (n->nlmsg_type == RTM_DELADDR && i) {
            AVAHI_LLIST_REMOVE(Address, addresses, addresses, i);
            avahi_free(i);
        } if (n->nlmsg_type == RTM_NEWADDR && !i) {
            i = avahi_new(Address, 1);
            i->address = address;
            AVAHI_LLIST_PREPEND(Address, addresses, addresses, i);
        }
    }

    return 0;
}

static int process_response(int wait_for_done, unsigned seq) {
    assert(fd >= 0);

    do {
        size_t bytes;
        ssize_t r;
        char replybuf[8*1024];
        char cred_msg[CMSG_SPACE(sizeof(struct ucred))];
        struct msghdr msghdr;
        struct cmsghdr *cmsghdr;
        struct ucred *ucred;
        struct iovec iov;
        struct nlmsghdr *p = (struct nlmsghdr *) replybuf;

        memset(&iov, 0, sizeof(iov));
        iov.iov_base = replybuf;
 	iov.iov_len = sizeof(replybuf);

        memset(&msghdr, 0, sizeof(msghdr));
        msghdr.msg_name = (void*) NULL;
        msghdr.msg_namelen = 0;
        msghdr.msg_iov = &iov;
        msghdr.msg_iovlen = 1;
        msghdr.msg_control = cred_msg;
        msghdr.msg_controllen = sizeof(cred_msg);
 	msghdr.msg_flags = 0;

        if ((r = recvmsg(fd, &msghdr, 0)) < 0) {
            daemon_log(LOG_ERR, "recvmsg() failed: %s", strerror(errno));
            return -1;
        }

        if (!(cmsghdr = CMSG_FIRSTHDR(&msghdr)) || cmsghdr->cmsg_type != SCM_CREDENTIALS) {
            daemon_log(LOG_WARNING, "No sender credentials received, ignoring data.");
            return -1;
        }

        ucred = (struct ucred*) CMSG_DATA(cmsghdr);

        if (ucred->pid != 0)
            return -1;

        bytes = (size_t) r;

        for (; bytes > 0; p = NLMSG_NEXT(p, bytes)) {

            if (!NLMSG_OK(p, bytes) || bytes < sizeof(struct nlmsghdr) || bytes < p->nlmsg_len) {
                daemon_log(LOG_ERR, "Netlink packet too small.");
                return -1;
            }

            if (p->nlmsg_type == NLMSG_DONE && wait_for_done && p->nlmsg_seq == seq && (pid_t) p->nlmsg_pid == getpid())
                return 0;

            if (p->nlmsg_type == NLMSG_ERROR) {
                struct nlmsgerr *e = (struct nlmsgerr *) NLMSG_DATA (p);

                if (e->error) {
                    daemon_log(LOG_ERR, "Netlink error: %s", strerror(-e->error));
                    return -1;
                }
            }

            if (process_nlmsg(p) < 0)
                return -1;
        }
    } while (wait_for_done);

    return 0;
}

int iface_get_initial_state(State *state) {
    struct nlmsghdr *n;
    struct ifinfomsg *ifi;
    struct ifaddrmsg *ifa;
    uint8_t req[1024];
    int seq = 0;

    assert(fd >= 0);
    assert(state);

    memset(&req, 0, sizeof(req));
    n = (struct nlmsghdr*) req;
    n->nlmsg_len = NLMSG_LENGTH(sizeof(*ifi));
    n->nlmsg_type = RTM_GETLINK;
    n->nlmsg_seq = seq;
    n->nlmsg_flags = NLM_F_REQUEST|NLM_F_DUMP;
    n->nlmsg_pid = 0;

    ifi = NLMSG_DATA(n);
    ifi->ifi_family = AF_UNSPEC;
    ifi->ifi_change = -1;

    if (send(fd, n, n->nlmsg_len, 0) < 0) {
        daemon_log(LOG_ERR, "send(): %s", strerror(errno));
        return -1;
    }

    if (process_response(1, 0) < 0)
        return -1;

    n->nlmsg_type = RTM_GETADDR;
    n->nlmsg_len = NLMSG_LENGTH(sizeof(*ifa));
    n->nlmsg_seq = ++seq;

    ifa = NLMSG_DATA(n);
    ifa->ifa_family = AF_INET;
    ifa->ifa_index = ifindex;

    if (send(fd, n, n->nlmsg_len, 0) < 0) {
        daemon_log(LOG_ERR, "send(): %s", strerror(errno));
        return -1;
    }

    if (process_response(1, seq) < 0)
        return -1;

    *state = addresses ? STATE_SLEEPING : STATE_START;

    return 0;
}

int iface_process(Event *event) {
    int b;
    assert(fd >= 0);

    b = !!addresses;

    if (process_response(0, 0) < 0)
        return -1;

    if (b && !addresses)
        *event = EVENT_ROUTABLE_ADDR_UNCONFIGURED;
    else if (!b && addresses)
        *event = EVENT_ROUTABLE_ADDR_CONFIGURED;

    return 0;
}

void iface_done(void) {
    Address *a;

    if (fd >= 0) {
        close(fd);
        fd = -1;
    }

    while ((a = addresses)) {
        AVAHI_LLIST_REMOVE(Address, addresses, addresses, a);
        avahi_free(a);
    }
}


