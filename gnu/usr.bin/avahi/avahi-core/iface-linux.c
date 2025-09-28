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

#include <string.h>
#include <net/if.h>
#include <errno.h>
#include <string.h>

#include <avahi-common/malloc.h>

#include "log.h"
#include "iface.h"
#include "iface-linux.h"

#ifndef IFLA_RTA
#include <linux/if_addr.h>
#define IFLA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#endif

#ifndef IFA_RTA
#include <linux/if_addr.h>
#define IFA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#endif

static int netlink_list_items(AvahiNetlink *nl, uint16_t type, unsigned *ret_seq) {
    struct nlmsghdr *n;
    struct rtgenmsg *gen;
    uint8_t req[1024];

    /* Issue a wild dump NETLINK request */

    memset(&req, 0, sizeof(req));
    n = (struct nlmsghdr*) req;
    n->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
    n->nlmsg_type = type;
    n->nlmsg_flags = NLM_F_REQUEST|NLM_F_DUMP;
    n->nlmsg_pid = 0;

    gen = NLMSG_DATA(n);
    memset(gen, 0, sizeof(struct rtgenmsg));
    gen->rtgen_family = AF_UNSPEC;

    return avahi_netlink_send(nl, n, ret_seq);
}

static void netlink_callback(AvahiNetlink *nl, struct nlmsghdr *n, void* userdata) {
    AvahiInterfaceMonitor *m = userdata;

    /* This routine is called for every RTNETLINK response packet */

    assert(m);
    assert(n);
    assert(m->osdep.netlink == nl);

    if (n->nlmsg_type == RTM_NEWLINK) {

        /* A new interface appeared or an existing one has been modified */

        struct ifinfomsg *ifinfomsg = NLMSG_DATA(n);
        AvahiHwInterface *hw;
        struct rtattr *a = NULL;
        size_t l;

        /* A (superfluous?) sanity check */
        if (ifinfomsg->ifi_family != AF_UNSPEC)
            return;

        /* Check whether there already is an AvahiHwInterface object
         * for this link, so that we can update its data. Note that
         * Netlink sends us an RTM_NEWLINK not only when a new
         * interface appears, but when it changes, too */

        if (!(hw = avahi_interface_monitor_get_hw_interface(m, ifinfomsg->ifi_index)))

            /* No object found, so let's create a new
             * one. avahi_hw_interface_new() will call
             * avahi_interface_new() internally twice for IPv4 and
             * IPv6, so there is no need for us to do that
             * ourselves */
            if (!(hw = avahi_hw_interface_new(m, (AvahiIfIndex) ifinfomsg->ifi_index)))
                return; /* OOM */

        /* Check whether the flags of this interface are OK for us */
        hw->flags_ok =
            (ifinfomsg->ifi_flags & IFF_UP) &&
            (!m->server->config.use_iff_running || (ifinfomsg->ifi_flags & IFF_RUNNING)) &&
            ((ifinfomsg->ifi_flags & IFF_LOOPBACK) ||
             (ifinfomsg->ifi_flags & IFF_MULTICAST)) &&
            (m->server->config.allow_point_to_point || !(ifinfomsg->ifi_flags & IFF_POINTOPOINT));

        /* Handle interface attributes */
        l = NLMSG_PAYLOAD(n, sizeof(struct ifinfomsg));
        a = IFLA_RTA(ifinfomsg);

        while (RTA_OK(a, l)) {
            switch(a->rta_type) {
                case IFLA_IFNAME:

                    /* Fill in interface name */
                    avahi_free(hw->name);
                    hw->name = avahi_strndup(RTA_DATA(a), RTA_PAYLOAD(a));
                    break;

                case IFLA_MTU:

                    /* Fill in MTU */
                    assert(RTA_PAYLOAD(a) == sizeof(unsigned int));
                    hw->mtu = *((unsigned int*) RTA_DATA(a));
                    break;

                case IFLA_ADDRESS:

                    /* Fill in hardware (MAC) address */
                    hw->mac_address_size = RTA_PAYLOAD(a);
                    if (hw->mac_address_size > AVAHI_MAC_ADDRESS_MAX)
                        hw->mac_address_size = AVAHI_MAC_ADDRESS_MAX;

                    memcpy(hw->mac_address, RTA_DATA(a), hw->mac_address_size);
                    break;

                default:
                    ;
            }

            a = RTA_NEXT(a, l);
        }

        /* Check whether this interface is now "relevant" for us. If
         * it is Avahi will start to announce its records on this
         * interface and send out queries for subscribed records on
         * it */
        avahi_hw_interface_check_relevant(hw);

        /* Update any associated RRs of this interface. (i.e. the
         * _workstation._tcp record containing the MAC address) */
        avahi_hw_interface_update_rrs(hw, 0);

    } else if (n->nlmsg_type == RTM_DELLINK) {

        /* An interface has been removed */

        struct ifinfomsg *ifinfomsg = NLMSG_DATA(n);
        AvahiHwInterface *hw;

        /* A (superfluous?) sanity check */
        if (ifinfomsg->ifi_family != AF_UNSPEC)
            return;

        /* Get a reference to our AvahiHwInterface object of this interface */
        if (!(hw = avahi_interface_monitor_get_hw_interface(m, (AvahiIfIndex) ifinfomsg->ifi_index)))
            return;

        /* Free our object */
        avahi_hw_interface_free(hw, 0);

    } else if (n->nlmsg_type == RTM_NEWADDR || n->nlmsg_type == RTM_DELADDR) {

        /* An address has been added, modified or removed */

        struct ifaddrmsg *ifaddrmsg = NLMSG_DATA(n);
        AvahiInterface *i;
        struct rtattr *a = NULL;
        size_t l;
        AvahiAddress raddr, rlocal, *r;
        int raddr_valid = 0, rlocal_valid = 0;

        /* We are only interested in IPv4 and IPv6 */
        if (ifaddrmsg->ifa_family != AF_INET && ifaddrmsg->ifa_family != AF_INET6)
            return;

        /* Try to get a reference to our AvahiInterface object for the
         * interface this address is assigned to. If ther is no object
         * for this interface, we ignore this address. */
        if (!(i = avahi_interface_monitor_get_interface(m, (AvahiIfIndex) ifaddrmsg->ifa_index, avahi_af_to_proto(ifaddrmsg->ifa_family))))
            return;

        /* Fill in address family for our new address */
        rlocal.proto = raddr.proto = avahi_af_to_proto(ifaddrmsg->ifa_family);

        l = NLMSG_PAYLOAD(n, sizeof(struct ifaddrmsg));
        a = IFA_RTA(ifaddrmsg);

        while (RTA_OK(a, l)) {

            switch(a->rta_type) {

                case IFA_LOCAL:

                    if ((rlocal.proto == AVAHI_PROTO_INET6 && RTA_PAYLOAD(a) != 16) ||
                        (rlocal.proto == AVAHI_PROTO_INET && RTA_PAYLOAD(a) != 4))
                        return;

                    memcpy(rlocal.data.data, RTA_DATA(a), RTA_PAYLOAD(a));
                    rlocal_valid = 1;

                    break;

                case IFA_ADDRESS:

                    /* Fill in local address data. Usually this is
                     * preferable over IFA_ADDRESS if both are set,
                     * since this refers to the local address of a PPP
                     * link while IFA_ADDRESS refers to the other
                     * end. */

                    if ((raddr.proto == AVAHI_PROTO_INET6 && RTA_PAYLOAD(a) != 16) ||
                        (raddr.proto == AVAHI_PROTO_INET && RTA_PAYLOAD(a) != 4))
                        return;

                    memcpy(raddr.data.data, RTA_DATA(a), RTA_PAYLOAD(a));
                    raddr_valid = 1;

                    break;

                default:
                    ;
            }

            a = RTA_NEXT(a, l);
        }

        /* If there was no adress attached to this message, let's quit. */
        if (rlocal_valid)
            r = &rlocal;
        else if (raddr_valid)
            r = &raddr;
        else
            return;

        if (n->nlmsg_type == RTM_NEWADDR) {
            AvahiInterfaceAddress *addr;

            /* This address is new or has been modified, so let's get an object for it */
            if (!(addr = avahi_interface_monitor_get_address(m, i, r)))

                /* Mmm, no object existing yet, so let's create a new one */
                if (!(addr = avahi_interface_address_new(m, i, r, ifaddrmsg->ifa_prefixlen)))
                    return; /* OOM */

            /* Update the scope field for the address */
            addr->global_scope = ifaddrmsg->ifa_scope == RT_SCOPE_UNIVERSE || ifaddrmsg->ifa_scope == RT_SCOPE_SITE;
            addr->deprecated = !!(ifaddrmsg->ifa_flags & IFA_F_DEPRECATED);
        } else {
            AvahiInterfaceAddress *addr;
            assert(n->nlmsg_type == RTM_DELADDR);

            /* Try to get a reference to our AvahiInterfaceAddress object for this address */
            if (!(addr = avahi_interface_monitor_get_address(m, i, r)))
                return;

            /* And free it */
            avahi_interface_address_free(addr);
        }

        /* Avahi only considers interfaces with at least one address
         * attached relevant. Since we migh have added or removed an
         * address, let's have it check again whether the interface is
         * now relevant */
        avahi_interface_check_relevant(i);

        /* Update any associated RRs, like A or AAAA for our new/removed address */
        avahi_interface_update_rrs(i, 0);

    } else if (n->nlmsg_type == NLMSG_DONE) {

        /* This wild dump request ended, so let's see what we do next */

        if (m->osdep.list == LIST_IFACE) {

            /* Mmmm, interfaces have been wild dumped already, so
             * let's go on with wild dumping the addresses */

            if (netlink_list_items(m->osdep.netlink, RTM_GETADDR, &m->osdep.query_addr_seq) < 0) {
                avahi_log_warn("NETLINK: Failed to list addrs: %s", strerror(errno));
                m->osdep.list = LIST_DONE;
            } else

                /* Update state information */
                m->osdep.list = LIST_ADDR;

        } else
            /* We're done. Tell avahi_interface_monitor_sync() to finish. */
            m->osdep.list = LIST_DONE;

        if (m->osdep.list == LIST_DONE) {

            /* Only after this boolean variable has been set, Avahi
             * will start to announce or browse on all interfaces. It
             * is originaly set to 0, which means that relevancy
             * checks and RR updates are disabled during the wild
             * dumps. */
            m->list_complete = 1;

            /* So let's check if any interfaces are relevant now */
            avahi_interface_monitor_check_relevant(m);

            /* And update all RRs attached to any interface */
            avahi_interface_monitor_update_rrs(m, 0);

            /* Tell the user that the wild dump is complete */
            avahi_log_info("Network interface enumeration completed.");
        }

    } else if (n->nlmsg_type == NLMSG_ERROR &&
               (n->nlmsg_seq == m->osdep.query_link_seq || n->nlmsg_seq == m->osdep.query_addr_seq)) {
        struct nlmsgerr *e = NLMSG_DATA (n);

        /* Some kind of error happened. Let's just tell the user and
         * ignore it otherwise */

        if (e->error)
            avahi_log_warn("NETLINK: Failed to browse: %s", strerror(-e->error));
    }
}

int avahi_interface_monitor_init_osdep(AvahiInterfaceMonitor *m) {
    assert(m);

    /* Initialize our own data */

    m->osdep.netlink = NULL;
    m->osdep.query_addr_seq = m->osdep.query_link_seq = 0;

    /* Create a netlink object for us. It abstracts some things and
     * makes netlink easier to use. It will attach to the main loop
     * for us and call netlink_callback() whenever an event
     * happens. */
    if (!(m->osdep.netlink = avahi_netlink_new(m->server->poll_api, RTMGRP_LINK|RTMGRP_IPV4_IFADDR|RTMGRP_IPV6_IFADDR, netlink_callback, m)))
        goto fail;

    /* Set the initial state. */
    m->osdep.list = LIST_IFACE;

    /* Start the wild dump for the interfaces */
    if (netlink_list_items(m->osdep.netlink, RTM_GETLINK, &m->osdep.query_link_seq) < 0)
        goto fail;

    return 0;

fail:

    if (m->osdep.netlink) {
        avahi_netlink_free(m->osdep.netlink);
        m->osdep.netlink = NULL;
    }

    return -1;
}

void avahi_interface_monitor_free_osdep(AvahiInterfaceMonitor *m) {
    assert(m);

    if (m->osdep.netlink) {
        avahi_netlink_free(m->osdep.netlink);
        m->osdep.netlink = NULL;
    }
}

void avahi_interface_monitor_sync(AvahiInterfaceMonitor *m) {
    assert(m);

    /* Let's handle netlink events until we are done with wild
     * dumping */

    while (!m->list_complete)
        if (!avahi_netlink_work(m->osdep.netlink, 1) == 0)
            break;

    /* At this point Avahi knows about all local interfaces and
     * addresses in existance. */
}
