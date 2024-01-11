/* rcs tags go here */
/* Original author: Bruce M. Simpson <bms@FreeBSD.org> */

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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <unistd.h>

#include <libdaemon/dlog.h>

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>

#include "iface.h"

#ifndef IN_LINKLOCAL
#define IN_LINKLOCAL(i) (((u_int32_t)(i) & (0xffff0000)) == (0xa9fe0000))
#endif

#ifndef elementsof
#define elementsof(array)       (sizeof(array)/sizeof(array[0]))
#endif

#ifndef so_set_nonblock
#define so_set_nonblock(s, val) \
        do {                                            \
                int __flags;                            \
                __flags = fcntl((s), F_GETFL);          \
                if (__flags == -1)                      \
                        break;                          \
                if (val != 0)                           \
                        __flags |= O_NONBLOCK;          \
                else                                    \
                        __flags &= ~O_NONBLOCK;         \
                (void)fcntl((s), F_SETFL, __flags);     \
        } while (0)
#endif

#define MAX_RTMSG_SIZE 2048

struct rtm_dispinfo {
        u_char          *di_buf;
        ssize_t          di_buflen;
        ssize_t          di_len;
};

union rtmunion {
        struct rt_msghdr                 rtm;
        struct if_msghdr                 ifm;
        struct ifa_msghdr                ifam;
        struct ifma_msghdr               ifmam;
        struct if_announcemsghdr         ifan;
};
typedef union rtmunion rtmunion_t;

struct Address;
typedef struct Address Address;

struct Address {
        in_addr_t       address;
        AVAHI_LLIST_FIELDS(Address, addresses);
};

static int rtm_dispatch(void);
static int rtm_dispatch_newdeladdr(struct rtm_dispinfo *di);
static int rtm_dispatch_ifannounce(struct rtm_dispinfo *di);
static struct sockaddr *next_sa(struct sockaddr *sa);

static int fd = -1;
static int ifindex = -1;
static AVAHI_LLIST_HEAD(Address, addresses) = NULL;

int
iface_init(int idx)
{

        fd = socket(PF_ROUTE, SOCK_RAW, AF_INET);
        if (fd == -1) {
                daemon_log(LOG_ERR, "socket(PF_ROUTE): %s", strerror(errno));
                return (-1);
        }

        so_set_nonblock(fd, 1);

        ifindex = idx;

        return (fd);
}

int
iface_get_initial_state(State *state)
{
        int                      mib[6];
        char                    *buf;
        struct if_msghdr        *ifm;
        struct ifa_msghdr       *ifam;
        char                    *lim;
        char                    *next;
        struct sockaddr         *sa;
        size_t                   len;
        int                      naddrs;

        assert(state != NULL);
        assert(fd != -1);

        naddrs = 0;

        mib[0] = CTL_NET;
        mib[1] = PF_ROUTE;
        mib[2] = 0;
        mib[3] = 0;
        mib[4] = NET_RT_IFLIST;
        mib[5] = ifindex;

        if (sysctl(mib, elementsof(mib), NULL, &len, NULL, 0) != 0) {
                daemon_log(LOG_ERR, "sysctl(NET_RT_IFLIST): %s",
                    strerror(errno));
                return (-1);
        }

        buf = malloc(len);
        if (buf == NULL) {
                daemon_log(LOG_ERR, "malloc(%d): %s", len, strerror(errno));
                return (-1);
        }

        if (sysctl(mib, elementsof(mib), buf, &len, NULL, 0) != 0) {
                daemon_log(LOG_ERR, "sysctl(NET_RT_IFLIST): %s",
                    strerror(errno));
                free(buf);
                return (-1);
        }

        lim = buf + len;
        for (next = buf; next < lim; next += ifm->ifm_msglen) {
                ifm = (struct if_msghdr *)next;
                if (ifm->ifm_type == RTM_NEWADDR) {
                        ifam = (struct ifa_msghdr *)next;
                        sa = (struct sockaddr *)(ifam + 1);
                        if (sa->sa_family != AF_INET)
                                continue;
                        ++naddrs;
                }
        }
        free(buf);

        *state = (naddrs > 0) ? STATE_SLEEPING : STATE_START;

        return (0);
}

int
iface_process(Event *event)
{
        int routable;

        assert(fd != -1);

        routable = !!addresses;

        if (rtm_dispatch() == -1)
                return (-1);

        if (routable && !addresses)
                *event = EVENT_ROUTABLE_ADDR_UNCONFIGURED;
        else if (!routable && addresses)
                *event = EVENT_ROUTABLE_ADDR_CONFIGURED;

        return (0);
}

void
iface_done(void)
{
        Address *a;

        if (fd != -1) {
                close(fd);
                fd = -1;
        }

        while ((a = addresses) != NULL) {
                AVAHI_LLIST_REMOVE(Address, addresses, addresses, a);
                avahi_free(a);
        }
}

/*
 * Dispatch kernel routing socket messages.
 */
static int
rtm_dispatch(void)
{
        struct msghdr mh;
        struct iovec iov[1];
        struct rt_msghdr *rtm;
        struct rtm_dispinfo *di;
        ssize_t len;
        int retval;

        di = malloc(sizeof(*di));
        if (di == NULL) {
                daemon_log(LOG_ERR, "malloc(%d): %s", sizeof(*di),
                    strerror(errno));
                return (-1);
        }
        di->di_buflen = MAX_RTMSG_SIZE;
        di->di_buf = calloc(MAX_RTMSG_SIZE, 1);
        if (di->di_buf == NULL) {
                free(di);
                daemon_log(LOG_ERR, "calloc(%d): %s", MAX_RTMSG_SIZE,
                    strerror(errno));
                return (-1);
        }

        memset(&mh, 0, sizeof(mh));
        iov[0].iov_base = di->di_buf;
        iov[0].iov_len = di->di_buflen;
        mh.msg_iov = iov;
        mh.msg_iovlen = 1;

        retval = 0;
        for (;;) {
                len = recvmsg(fd, &mh, MSG_DONTWAIT);
                if (len == -1) {
                        if (errno == EWOULDBLOCK)
                                break;
                        else {
                                daemon_log(LOG_ERR, "recvmsg(): %s",
                                    strerror(errno));
                                retval = -1;
                                break;
                        }
                }

                rtm = (void *)di->di_buf;
                if (rtm->rtm_version != RTM_VERSION) {
                        daemon_log(LOG_ERR,
                            "unknown routing socket message (version %d)\n",
                            rtm->rtm_version);
                        /* this is non-fatal; just ignore it for now. */
                        continue;
                }

                switch (rtm->rtm_type) {
                case RTM_NEWADDR:
                case RTM_DELADDR:
                        retval = rtm_dispatch_newdeladdr(di);
                        break;
                case RTM_IFANNOUNCE:
                        retval = rtm_dispatch_ifannounce(di);
                        break;
                default:
                        daemon_log(LOG_DEBUG, "%s: rtm_type %d ignored", __func__, rtm->rtm_type);
                        break;
                }

                /*
                 * If we got an error; assume our position on the call
                 * stack is enclosed by a level-triggered event loop,
                 * and signal the error condition.
                 */
                if (retval != 0)
                        break;
        }
        free(di->di_buf);
        free(di);

        return (retval);
}

/* handle link coming or going away */
static int
rtm_dispatch_ifannounce(struct rtm_dispinfo *di)
{
        rtmunion_t *rtm = (void *)di->di_buf;

        assert(rtm->rtm.rtm_type == RTM_IFANNOUNCE);

        daemon_log(LOG_DEBUG, "%s: IFANNOUNCE for ifindex %d",
            __func__, rtm->ifan.ifan_index);

        switch (rtm->ifan.ifan_what) {
        case IFAN_ARRIVAL:
                if (rtm->ifan.ifan_index == ifindex) {
                        daemon_log(LOG_ERR,
"RTM_IFANNOUNCE IFAN_ARRIVAL, for ifindex %d, which we already manage.",
                            ifindex);
                        return (-1);
                }
                break;
        case IFAN_DEPARTURE:
                if (rtm->ifan.ifan_index == ifindex) {
                        daemon_log(LOG_ERR, "Interface vanished.");
                        return (-1);
                }
                break;
        default:
                /* ignore */
                break;
        }

        return (0);
}

static struct sockaddr *
next_sa(struct sockaddr *sa)
{
        void            *p;
        size_t           sa_size;

#ifdef SA_SIZE
        sa_size = SA_SIZE(sa);
#else
        /* This is not foolproof, kernel may round. */
        sa_size = sa->sa_len;
        if (sa_size < sizeof(u_long))
                sa_size = sizeof(u_long);
#endif

        p = ((char *)sa) + sa_size;

        return (struct sockaddr *)p;
}

/* handle address coming or going away */
static int
rtm_dispatch_newdeladdr(struct rtm_dispinfo *di)
{
        Address                 *ap;
        struct ifa_msghdr       *ifam;
        struct sockaddr         *sa;
        struct sockaddr_in      *sin;
        int                     link_local;

/* macro to skip to next RTA; has side-effects */
#define SKIPRTA(ifamsgp, rta, sa)                                       \
        do {                                                            \
                if ((ifamsgp)->ifam_addrs & (rta))                      \
                        (sa) = next_sa((sa));                           \
        } while (0)

        ifam = &((rtmunion_t *)di->di_buf)->ifam;

        assert(ifam->ifam_type == RTM_NEWADDR ||
               ifam->ifam_type == RTM_DELADDR);

        daemon_log(LOG_DEBUG, "%s: %s for iface %d (%s)", __func__,
            ifam->ifam_type == RTM_NEWADDR ? "NEWADDR" : "DELADDR",
            ifam->ifam_index, (ifam->ifam_index == ifindex) ? "ours" : "not ours");

        if (ifam->ifam_index != ifindex)
                return (0);

        if (!(ifam->ifam_addrs & RTA_IFA)) {
                daemon_log(LOG_ERR, "ifa msg has no RTA_IFA.");
                return (0);
        }

        /* skip over rtmsg padding correctly */
        sa = (struct sockaddr *)(ifam + 1);
        SKIPRTA(ifam, RTA_DST, sa);
        SKIPRTA(ifam, RTA_GATEWAY, sa);
        SKIPRTA(ifam, RTA_NETMASK, sa);
        SKIPRTA(ifam, RTA_GENMASK, sa);
        SKIPRTA(ifam, RTA_IFP, sa);

        /*
         * sa now points to RTA_IFA sockaddr; we are only interested
         * in updates for routable addresses.
         */
        if (sa->sa_family != AF_INET) {
                daemon_log(LOG_DEBUG, "%s: RTA_IFA family not AF_INET (=%d)", __func__, sa->sa_family);
                return (0);
        }

        sin = (struct sockaddr_in *)sa;
        link_local = IN_LINKLOCAL(ntohl(sin->sin_addr.s_addr));

        daemon_log(LOG_DEBUG, "%s: %s for %s (%s)", __func__,
            ifam->ifam_type == RTM_NEWADDR ? "NEWADDR" : "DELADDR",
            inet_ntoa(sin->sin_addr), link_local ? "link local" : "routable");

        if (link_local)
                return (0);

        for (ap = addresses; ap; ap = ap->addresses_next) {
                if (ap->address == sin->sin_addr.s_addr)
                        break;
        }
        if (ifam->ifam_type == RTM_DELADDR && ap != NULL) {
                AVAHI_LLIST_REMOVE(Address, addresses, addresses, ap);
                avahi_free(ap);
        }
        if (ifam->ifam_type == RTM_NEWADDR && ap == NULL) {
                ap = avahi_new(Address, 1);
                ap->address = sin->sin_addr.s_addr;
                AVAHI_LLIST_PREPEND(Address, addresses, addresses, ap);
        }

        return (0);
#undef SKIPRTA
}
