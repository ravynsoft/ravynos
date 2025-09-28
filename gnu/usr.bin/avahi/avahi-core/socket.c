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

#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/uio.h>

#ifdef IP_RECVIF
#include <net/if_dl.h>
#endif

#include "dns.h"
#include "fdutil.h"
#include "socket.h"
#include "log.h"
#include "addr-util.h"

/* this is a portability hack */
#ifndef IPV6_ADD_MEMBERSHIP
#ifdef  IPV6_JOIN_GROUP
#define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#endif
#endif

#ifndef IPV6_DROP_MEMBERSHIP
#ifdef  IPV6_LEAVE_GROUP
#define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#endif
#endif

static void mdns_mcast_group_ipv4(struct sockaddr_in *ret_sa) {
    assert(ret_sa);

    memset(ret_sa, 0, sizeof(struct sockaddr_in));
    ret_sa->sin_family = AF_INET;
    ret_sa->sin_port = htons(AVAHI_MDNS_PORT);
    inet_pton(AF_INET, AVAHI_IPV4_MCAST_GROUP, &ret_sa->sin_addr);
}

static void mdns_mcast_group_ipv6(struct sockaddr_in6 *ret_sa) {
    assert(ret_sa);

    memset(ret_sa, 0, sizeof(struct sockaddr_in6));
    ret_sa->sin6_family = AF_INET6;
    ret_sa->sin6_port = htons(AVAHI_MDNS_PORT);
    inet_pton(AF_INET6, AVAHI_IPV6_MCAST_GROUP, &ret_sa->sin6_addr);
}

static void ipv4_address_to_sockaddr(struct sockaddr_in *ret_sa, const AvahiIPv4Address *a, uint16_t port) {
    assert(ret_sa);
    assert(a);
    assert(port > 0);

    memset(ret_sa, 0, sizeof(struct sockaddr_in));
    ret_sa->sin_family = AF_INET;
    ret_sa->sin_port = htons(port);
    memcpy(&ret_sa->sin_addr, a, sizeof(AvahiIPv4Address));
}

static void ipv6_address_to_sockaddr(struct sockaddr_in6 *ret_sa, const AvahiIPv6Address *a, uint16_t port) {
    assert(ret_sa);
    assert(a);
    assert(port > 0);

    memset(ret_sa, 0, sizeof(struct sockaddr_in6));
    ret_sa->sin6_family = AF_INET6;
    ret_sa->sin6_port = htons(port);
    memcpy(&ret_sa->sin6_addr, a, sizeof(AvahiIPv6Address));
}

int avahi_mdns_mcast_join_ipv4(int fd, const AvahiIPv4Address *a, int idx, int join) {
#ifdef HAVE_STRUCT_IP_MREQN
    struct ip_mreqn mreq;
#else
    struct ip_mreq mreq;
#endif
    struct sockaddr_in sa;

    assert(fd >= 0);
    assert(idx >= 0);
    assert(a);

    memset(&mreq, 0, sizeof(mreq));
#ifdef HAVE_STRUCT_IP_MREQN
    mreq.imr_ifindex = idx;
    mreq.imr_address.s_addr = a->address;
#else
    mreq.imr_interface.s_addr = a->address;
#endif
    mdns_mcast_group_ipv4(&sa);
    mreq.imr_multiaddr = sa.sin_addr;

    /* Some network drivers have issues with dropping membership of
     * mcast groups when the iface is down, but don't allow rejoining
     * when it comes back up. This is an ugly workaround */
    if (join)
        setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));

    if (setsockopt(fd, IPPROTO_IP, join ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        avahi_log_warn("%s failed: %s", join ? "IP_ADD_MEMBERSHIP" : "IP_DROP_MEMBERSHIP", strerror(errno));
        return -1;
    }

    return 0;
}

int avahi_mdns_mcast_join_ipv6(int fd, const AvahiIPv6Address *a, int idx, int join) {
    struct ipv6_mreq mreq6;
    struct sockaddr_in6 sa6;

    assert(fd >= 0);
    assert(idx >= 0);
    assert(a);

    memset(&mreq6, 0, sizeof(mreq6));
    mdns_mcast_group_ipv6 (&sa6);
    mreq6.ipv6mr_multiaddr = sa6.sin6_addr;
    mreq6.ipv6mr_interface = idx;

    if (join)
        setsockopt(fd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, &mreq6, sizeof(mreq6));

    if (setsockopt(fd, IPPROTO_IPV6, join ? IPV6_ADD_MEMBERSHIP : IPV6_DROP_MEMBERSHIP, &mreq6, sizeof(mreq6)) < 0) {
        avahi_log_warn("%s failed: %s", join ? "IPV6_ADD_MEMBERSHIP" : "IPV6_DROP_MEMBERSHIP", strerror(errno));
        return -1;
    }

    return 0;
}

static int reuseaddr(int fd) {
    int yes;

    yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("SO_REUSEADDR failed: %s", strerror(errno));
        return -1;
    }

#ifdef SO_REUSEPORT
    yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("SO_REUSEPORT failed: %s", strerror(errno));
        if (errno != ENOPROTOOPT)
            return -1;
    }
#endif

    return 0;
}

static int bind_with_warn(int fd, const struct sockaddr *sa, socklen_t l) {

    assert(fd >= 0);
    assert(sa);
    assert(l > 0);

    if (bind(fd, sa, l) < 0) {

        if (errno != EADDRINUSE) {
            avahi_log_warn("bind() failed: %s", strerror(errno));
            return -1;
        }

        avahi_log_warn("*** WARNING: Detected another %s mDNS stack running on this host. This makes mDNS unreliable and is thus not recommended. ***",
                       sa->sa_family == AF_INET ? "IPv4" : "IPv6");

        /* Try again, this time with SO_REUSEADDR set */
        if (reuseaddr(fd) < 0)
            return -1;

        if (bind(fd, sa, l) < 0) {
            avahi_log_warn("bind() failed: %s", strerror(errno));
            return -1;
        }
    } else {

        /* We enable SO_REUSEADDR afterwards, to make sure that the
         * user may run other mDNS implementations if he really
         * wants. */

        if (reuseaddr(fd) < 0)
            return -1;
    }

    return 0;
}

static int ipv4_pktinfo(int fd) {
    int yes;

#ifdef IP_PKTINFO
    yes = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_PKTINFO, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IP_PKTINFO failed: %s", strerror(errno));
        return -1;
    }
#else

#ifdef IP_RECVINTERFACE
    yes = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_RECVINTERFACE, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IP_RECVINTERFACE failed: %s", strerror(errno));
        return -1;
    }
#elif defined(IP_RECVIF)
    yes = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_RECVIF, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IP_RECVIF failed: %s", strerror(errno));
        return -1;
    }
#endif

#ifdef IP_RECVDSTADDR
    yes = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_RECVDSTADDR, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IP_RECVDSTADDR failed: %s", strerror(errno));
        return -1;
    }
#endif

#endif /* IP_PKTINFO */

#ifdef IP_RECVTTL
    yes = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_RECVTTL, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IP_RECVTTL failed: %s", strerror(errno));
        return -1;
    }
#endif

    return 0;
}

static int ipv6_pktinfo(int fd) {
    int yes;

#ifdef IPV6_RECVPKTINFO
    yes = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IPV6_RECVPKTINFO failed: %s", strerror(errno));
        return -1;
    }
#elif defined(IPV6_PKTINFO)
    yes = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_PKTINFO, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IPV6_PKTINFO failed: %s", strerror(errno));
        return -1;
    }
#endif

#ifdef IPV6_RECVHOPS
    yes = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_RECVHOPS, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IPV6_RECVHOPS failed: %s", strerror(errno));
        return -1;
    }
#elif defined(IPV6_RECVHOPLIMIT)
    yes = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IPV6_RECVHOPLIMIT failed: %s", strerror(errno));
        return -1;
    }
#elif defined(IPV6_HOPLIMIT)
    yes = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_HOPLIMIT, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IPV6_HOPLIMIT failed: %s", strerror(errno));
        return -1;
    }
#endif

    return 0;
}

int avahi_open_socket_ipv4(int no_reuse) {
    struct sockaddr_in local;
    int fd = -1, r, ittl;
    uint8_t ttl, cyes;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        avahi_log_warn("socket() failed: %s", strerror(errno));
        goto fail;
    }

    ttl = 255;
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        avahi_log_warn("IP_MULTICAST_TTL failed: %s", strerror(errno));
        goto fail;
    }

    ittl = 255;
    if (setsockopt(fd, IPPROTO_IP, IP_TTL, &ittl, sizeof(ittl)) < 0) {
        avahi_log_warn("IP_TTL failed: %s", strerror(errno));
        goto fail;
    }

    cyes = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &cyes, sizeof(cyes)) < 0) {
        avahi_log_warn("IP_MULTICAST_LOOP failed: %s", strerror(errno));
        goto fail;
    }

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(AVAHI_MDNS_PORT);

    if (no_reuse)
        r = bind(fd, (struct sockaddr*) &local, sizeof(local));
    else
        r = bind_with_warn(fd, (struct sockaddr*) &local, sizeof(local));

    if (r < 0)
        goto fail;

    if (ipv4_pktinfo (fd) < 0)
         goto fail;

    if (avahi_set_cloexec(fd) < 0) {
        avahi_log_warn("FD_CLOEXEC failed: %s", strerror(errno));
        goto fail;
    }

    if (avahi_set_nonblock(fd) < 0) {
        avahi_log_warn("O_NONBLOCK failed: %s", strerror(errno));
        goto fail;
    }

    return fd;

fail:
    if (fd >= 0)
        close(fd);

    return -1;
}

int avahi_open_socket_ipv6(int no_reuse) {
    struct sockaddr_in6 sa, local;
    int fd = -1, yes, r;
    int ttl;

    mdns_mcast_group_ipv6(&sa);

    if ((fd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        avahi_log_warn("socket() failed: %s", strerror(errno));
        goto fail;
    }

    ttl = 255;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl)) < 0) {
        avahi_log_warn("IPV6_MULTICAST_HOPS failed: %s", strerror(errno));
        goto fail;
    }

    ttl = 255;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &ttl, sizeof(ttl)) < 0) {
        avahi_log_warn("IPV6_UNICAST_HOPS failed: %s", strerror(errno));
        goto fail;
    }

    yes = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IPV6_V6ONLY failed: %s", strerror(errno));
        goto fail;
    }

    yes = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IPV6_MULTICAST_LOOP failed: %s", strerror(errno));
        goto fail;
    }

    memset(&local, 0, sizeof(local));
    local.sin6_family = AF_INET6;
    local.sin6_port = htons(AVAHI_MDNS_PORT);

    if (no_reuse)
        r = bind(fd, (struct sockaddr*) &local, sizeof(local));
    else
        r = bind_with_warn(fd, (struct sockaddr*) &local, sizeof(local));

    if (r < 0)
        goto fail;

    if (ipv6_pktinfo(fd) < 0)
        goto fail;

    if (avahi_set_cloexec(fd) < 0) {
        avahi_log_warn("FD_CLOEXEC failed: %s", strerror(errno));
        goto fail;
    }

    if (avahi_set_nonblock(fd) < 0) {
        avahi_log_warn("O_NONBLOCK failed: %s", strerror(errno));
        goto fail;
    }

    return fd;

fail:
    if (fd >= 0)
        close(fd);

    return -1;
}

static int sendmsg_loop(int fd, struct msghdr *msg, int flags) {
    assert(fd >= 0);
    assert(msg);

    for (;;) {

        if (sendmsg(fd, msg, flags) >= 0)
            break;

        if (errno == EINTR)
            continue;

        if (errno != EAGAIN) {
            char where[64];
            struct sockaddr_storage *ss = msg->msg_name;

            if (ss->ss_family == PF_INET) {
                inet_ntop(ss->ss_family, &((struct sockaddr_in*)ss)->sin_addr, where, sizeof(where));
            } else if (ss->ss_family == PF_INET6) {
                inet_ntop(ss->ss_family, &((struct sockaddr_in6*)ss)->sin6_addr, where, sizeof(where));
            } else {
                where[0] = '\0';
            }

            avahi_log_debug("sendmsg() to %s failed: %s", where, strerror(errno));

            return -1;

        }

        if (avahi_wait_for_write(fd) < 0)
            return -1;
    }

    return 0;
}

int avahi_send_dns_packet_ipv4(
        int fd,
        AvahiIfIndex interface,
        AvahiDnsPacket *p,
        const AvahiIPv4Address *src_address,
        const AvahiIPv4Address *dst_address,
        uint16_t dst_port) {

    struct sockaddr_in sa;
    struct msghdr msg;
    struct iovec io;
#ifdef IP_PKTINFO
    struct cmsghdr *cmsg;
    size_t cmsg_data[( CMSG_SPACE(sizeof(struct in_pktinfo)) / sizeof(size_t)) + 1];
#elif !defined(IP_MULTICAST_IF) && defined(IP_SENDSRCADDR)
    struct cmsghdr *cmsg;
    size_t cmsg_data[( CMSG_SPACE(sizeof(struct in_addr)) / sizeof(size_t)) + 1];
#endif

    assert(fd >= 0);
    assert(p);
    assert(avahi_dns_packet_check_valid(p) >= 0);
    assert(!dst_address || dst_port > 0);

    if (!dst_address)
        mdns_mcast_group_ipv4(&sa);
    else
        ipv4_address_to_sockaddr(&sa, dst_address, dst_port);

    memset(&io, 0, sizeof(io));
    io.iov_base = AVAHI_DNS_PACKET_DATA(p);
    io.iov_len = p->size;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;

#ifdef IP_PKTINFO
    if (interface > 0 || src_address) {
        struct in_pktinfo *pkti;

        memset(cmsg_data, 0, sizeof(cmsg_data));
        msg.msg_control = cmsg_data;
        msg.msg_controllen = CMSG_LEN(sizeof(struct in_pktinfo));

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = msg.msg_controllen;
        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_PKTINFO;

        pkti = (struct in_pktinfo*) CMSG_DATA(cmsg);

        if (interface > 0)
            pkti->ipi_ifindex = interface;

        if (src_address)
            pkti->ipi_spec_dst.s_addr = src_address->address;
    }
#elif defined(IP_MULTICAST_IF)
    if (src_address) {
        struct in_addr any = { INADDR_ANY };
        if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, src_address ? &src_address->address : &any, sizeof(struct in_addr)) < 0) {
            avahi_log_warn("IP_MULTICAST_IF failed: %s", strerror(errno));
            return -1;
        }
    }
#elif defined(IP_SENDSRCADDR)
    if (src_address) {
        struct in_addr *addr;

        memset(cmsg_data, 0, sizeof(cmsg_data));
        msg.msg_control = cmsg_data;
        msg.msg_controllen = CMSG_LEN(sizeof(struct in_addr));

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = msg.msg_controllen;
        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_SENDSRCADDR;

        addr = (struct in_addr *)CMSG_DATA(cmsg);
        addr->s_addr =  src_address->address;
    }
#elif defined(__GNUC__)
#warning "FIXME: We need some code to set the outgoing interface/local address here if IP_PKTINFO/IP_MULTICAST_IF is not available"
#endif

    return sendmsg_loop(fd, &msg, 0);
}

int avahi_send_dns_packet_ipv6(
        int fd,
        AvahiIfIndex interface,
        AvahiDnsPacket *p,
        const AvahiIPv6Address *src_address,
        const AvahiIPv6Address *dst_address,
        uint16_t dst_port) {

    struct sockaddr_in6 sa;
    struct msghdr msg;
    struct iovec io;
    struct cmsghdr *cmsg;
    size_t cmsg_data[(CMSG_SPACE(sizeof(struct in6_pktinfo))/sizeof(size_t)) + 1];

    assert(fd >= 0);
    assert(p);
    assert(avahi_dns_packet_check_valid(p) >= 0);
    assert(!dst_address || dst_port > 0);

    if (!dst_address)
        mdns_mcast_group_ipv6(&sa);
    else
        ipv6_address_to_sockaddr(&sa, dst_address, dst_port);

    memset(&io, 0, sizeof(io));
    io.iov_base = AVAHI_DNS_PACKET_DATA(p);
    io.iov_len = p->size;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;

    if (interface > 0 || src_address) {
        struct in6_pktinfo *pkti;

        memset(cmsg_data, 0, sizeof(cmsg_data));
        msg.msg_control = cmsg_data;
        msg.msg_controllen = CMSG_LEN(sizeof(struct in6_pktinfo));

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = msg.msg_controllen;
        cmsg->cmsg_level = IPPROTO_IPV6;
        cmsg->cmsg_type = IPV6_PKTINFO;

        pkti = (struct in6_pktinfo*) CMSG_DATA(cmsg);

        if (interface > 0)
            pkti->ipi6_ifindex = interface;

        if (src_address)
            memcpy(&pkti->ipi6_addr, src_address->address, sizeof(src_address->address));
    } else {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
    }

    return sendmsg_loop(fd, &msg, 0);
}

AvahiDnsPacket *avahi_recv_dns_packet_ipv4(
        int fd,
        AvahiIPv4Address *ret_src_address,
        uint16_t *ret_src_port,
        AvahiIPv4Address *ret_dst_address,
        AvahiIfIndex *ret_iface,
        uint8_t *ret_ttl) {

    AvahiDnsPacket *p= NULL;
    struct msghdr msg;
    struct iovec io;
    size_t aux[1024 / sizeof(size_t)]; /* for alignment on ia64 ! */
    ssize_t l;
    struct cmsghdr *cmsg;
    int found_addr = 0;
    int ms;
    struct sockaddr_in sa;

    assert(fd >= 0);

    if (ioctl(fd, FIONREAD, &ms) < 0) {
        avahi_log_warn("ioctl(): %s", strerror(errno));
        goto fail;
    }

    if (ms < 0) {
        avahi_log_warn("FIONREAD returned negative value.");
        goto fail;
    }

    p = avahi_dns_packet_new(ms + AVAHI_DNS_PACKET_EXTRA_SIZE);

    io.iov_base = AVAHI_DNS_PACKET_DATA(p);
    io.iov_len = p->max_size;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = aux;
    msg.msg_controllen = sizeof(aux);
    msg.msg_flags = 0;

    if ((l = recvmsg(fd, &msg, 0)) < 0) {
        /* Linux returns EAGAIN when an invalid IP packet has been
        received. We suppress warnings in this case because this might
        create quite a bit of log traffic on machines with unstable
        links. (See #60) */

        if (errno != EAGAIN)
            avahi_log_warn("recvmsg(): %s", strerror(errno));

        goto fail;
    }

    /* For corrupt packets FIONREAD returns zero size (See rhbz #607297). So
     * fail after having read them. */
    if (!ms)
        goto fail;

    if (sa.sin_addr.s_addr == INADDR_ANY)
        /* Linux 2.4 behaves very strangely sometimes! */
        goto fail;

    assert(!(msg.msg_flags & MSG_CTRUNC));
    assert(!(msg.msg_flags & MSG_TRUNC));

    p->size = (size_t) l;

    if (ret_src_port)
        *ret_src_port = avahi_port_from_sockaddr((struct sockaddr*) &sa);

    if (ret_src_address) {
        AvahiAddress a;
        avahi_address_from_sockaddr((struct sockaddr*) &sa, &a);
        *ret_src_address = a.data.ipv4;
    }

    if (ret_ttl)
        *ret_ttl = 255;

    if (ret_iface)
        *ret_iface = AVAHI_IF_UNSPEC;

    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {

        if (cmsg->cmsg_level == IPPROTO_IP) {

            switch (cmsg->cmsg_type) {
#ifdef IP_RECVTTL
                case IP_RECVTTL:
#endif
                case IP_TTL:
                    if (ret_ttl)
                        *ret_ttl = (uint8_t) (*(int *) CMSG_DATA(cmsg));

                    break;

#ifdef IP_PKTINFO
                case IP_PKTINFO: {
                    struct in_pktinfo *i = (struct in_pktinfo*) CMSG_DATA(cmsg);

                    if (ret_iface && i->ipi_ifindex > 0)
                        *ret_iface = (int) i->ipi_ifindex;

                    if (ret_dst_address)
                        ret_dst_address->address = i->ipi_addr.s_addr;

                    found_addr = 1;

                    break;
                }
#endif

#ifdef IP_RECVIF
                case IP_RECVIF: {
                    struct sockaddr_dl *sdl = (struct sockaddr_dl *) CMSG_DATA (cmsg);

                    if (ret_iface) {
#ifdef __sun
                        if (*(uint_t*) sdl > 0)
                            *ret_iface = *(uint_t*) sdl;
#else

                        if (sdl->sdl_index > 0)
                            *ret_iface = (int) sdl->sdl_index;
#endif
                    }

                    break;
                }
#endif

#ifdef IP_RECVDSTADDR
                case IP_RECVDSTADDR:
                    if (ret_dst_address)
                        memcpy(&ret_dst_address->address, CMSG_DATA (cmsg), 4);

                    found_addr = 1;
                    break;
#endif

                default:
                    avahi_log_warn("Unhandled cmsg_type: %d", cmsg->cmsg_type);
                    break;
            }
        }
    }

    assert(found_addr);

    return p;

fail:
    if (p)
        avahi_dns_packet_free(p);

    return NULL;
}

AvahiDnsPacket *avahi_recv_dns_packet_ipv6(
        int fd,
        AvahiIPv6Address *ret_src_address,
        uint16_t *ret_src_port,
        AvahiIPv6Address *ret_dst_address,
        AvahiIfIndex *ret_iface,
        uint8_t *ret_ttl) {

    AvahiDnsPacket *p = NULL;
    struct msghdr msg;
    struct iovec io;
    size_t aux[1024 / sizeof(size_t)];
    ssize_t l;
    int ms;
    struct cmsghdr *cmsg;
    int found_ttl = 0, found_iface = 0;
    struct sockaddr_in6 sa;

    assert(fd >= 0);

    if (ioctl(fd, FIONREAD, &ms) < 0) {
        avahi_log_warn("ioctl(): %s", strerror(errno));
        goto fail;
    }

    if (ms < 0) {
        avahi_log_warn("FIONREAD returned negative value.");
        goto fail;
    }

    p = avahi_dns_packet_new(ms + AVAHI_DNS_PACKET_EXTRA_SIZE);

    io.iov_base = AVAHI_DNS_PACKET_DATA(p);
    io.iov_len = p->max_size;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (struct sockaddr*) &sa;
    msg.msg_namelen = sizeof(sa);

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = aux;
    msg.msg_controllen = sizeof(aux);
    msg.msg_flags = 0;

    if ((l = recvmsg(fd, &msg, 0)) < 0) {
        /* Linux returns EAGAIN when an invalid IP packet has been
        received. We suppress warnings in this case because this might
        create quite a bit of log traffic on machines with unstable
        links. (See #60) */

        if (errno != EAGAIN)
            avahi_log_warn("recvmsg(): %s", strerror(errno));

        goto fail;
    }

    /* For corrupt packets FIONREAD returns zero size (See rhbz #607297). So
     * fail after having read them. */
    if (!ms)
        goto fail;

    assert(!(msg.msg_flags & MSG_CTRUNC));
    assert(!(msg.msg_flags & MSG_TRUNC));

    p->size = (size_t) l;

    if (ret_src_port)
        *ret_src_port = avahi_port_from_sockaddr((struct sockaddr*) &sa);

    if (ret_src_address) {
        AvahiAddress a;
        avahi_address_from_sockaddr((struct sockaddr*) &sa, &a);
        *ret_src_address = a.data.ipv6;
    }

    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {

        if (cmsg->cmsg_level == IPPROTO_IPV6) {

            switch (cmsg->cmsg_type) {

                case IPV6_HOPLIMIT:

                    if (ret_ttl)
                        *ret_ttl = (uint8_t) (*(int *) CMSG_DATA(cmsg));

                    found_ttl = 1;

                    break;

                case IPV6_PKTINFO: {
                    struct in6_pktinfo *i = (struct in6_pktinfo*) CMSG_DATA(cmsg);

                    if (ret_iface && i->ipi6_ifindex > 0)
                        *ret_iface = i->ipi6_ifindex;

                    if (ret_dst_address)
                        memcpy(ret_dst_address->address, i->ipi6_addr.s6_addr, 16);

                    found_iface = 1;
                    break;
                }

                default:
                    avahi_log_warn("Unhandled cmsg_type: %d", cmsg->cmsg_type);
                    break;
            }
        }
    }

    assert(found_iface);
    assert(found_ttl);

    return p;

fail:
    if (p)
        avahi_dns_packet_free(p);

    return NULL;
}

int avahi_open_unicast_socket_ipv4(void) {
    struct sockaddr_in local;
    int fd = -1;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        avahi_log_warn("socket() failed: %s", strerror(errno));
        goto fail;
    }

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;

    if (bind(fd, (struct sockaddr*) &local, sizeof(local)) < 0) {
        avahi_log_warn("bind() failed: %s", strerror(errno));
        goto fail;
    }

    if (ipv4_pktinfo(fd) < 0) {
         goto fail;
    }

    if (avahi_set_cloexec(fd) < 0) {
        avahi_log_warn("FD_CLOEXEC failed: %s", strerror(errno));
        goto fail;
    }

    if (avahi_set_nonblock(fd) < 0) {
        avahi_log_warn("O_NONBLOCK failed: %s", strerror(errno));
        goto fail;
    }

    return fd;

fail:
    if (fd >= 0)
        close(fd);

    return -1;
}

int avahi_open_unicast_socket_ipv6(void) {
    struct sockaddr_in6 local;
    int fd = -1, yes;

    if ((fd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        avahi_log_warn("socket() failed: %s", strerror(errno));
        goto fail;
    }

    yes = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) < 0) {
        avahi_log_warn("IPV6_V6ONLY failed: %s", strerror(errno));
        goto fail;
    }

    memset(&local, 0, sizeof(local));
    local.sin6_family = AF_INET6;

    if (bind(fd, (struct sockaddr*) &local, sizeof(local)) < 0) {
        avahi_log_warn("bind() failed: %s", strerror(errno));
        goto fail;
    }

    if (ipv6_pktinfo(fd) < 0)
        goto fail;

    if (avahi_set_cloexec(fd) < 0) {
        avahi_log_warn("FD_CLOEXEC failed: %s", strerror(errno));
        goto fail;
    }

    if (avahi_set_nonblock(fd) < 0) {
        avahi_log_warn("O_NONBLOCK failed: %s", strerror(errno));
        goto fail;
    }

    return fd;

fail:
    if (fd >= 0)
        close(fd);

    return -1;
}
