#ifndef fooifacehfoo
#define fooifacehfoo

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

typedef struct AvahiInterfaceMonitor AvahiInterfaceMonitor;
typedef struct AvahiInterfaceAddress AvahiInterfaceAddress;
typedef struct AvahiInterface AvahiInterface;
typedef struct AvahiHwInterface AvahiHwInterface;

#include <avahi-common/llist.h>
#include <avahi-common/address.h>

#include "internal.h"
#include "cache.h"
#include "response-sched.h"
#include "query-sched.h"
#include "probe-sched.h"
#include "dns.h"
#include "announce.h"
#include "browse.h"
#include "querier.h"

#ifdef HAVE_NETLINK
#include "iface-linux.h"
#elif defined(HAVE_PF_ROUTE)
#include "iface-pfroute.h"
#else
typedef struct AvahiInterfaceMonitorOSDep AvahiInterfaceMonitorOSDep;
struct AvahiInterfaceMonitorOSDep {

    unsigned query_addr_seq, query_link_seq;

    enum {
        LIST_IFACE,
        LIST_ADDR,
        LIST_DONE
    } list;
};
#endif

#define AVAHI_MAC_ADDRESS_MAX 32

struct AvahiInterfaceMonitor {
    AvahiServer *server;
    AvahiHashmap *hashmap;

    AVAHI_LLIST_HEAD(AvahiInterface, interfaces);
    AVAHI_LLIST_HEAD(AvahiHwInterface, hw_interfaces);

    int list_complete;
    AvahiInterfaceMonitorOSDep osdep;
};

struct AvahiHwInterface {
    AvahiInterfaceMonitor *monitor;

    AVAHI_LLIST_FIELDS(AvahiHwInterface, hardware);

    char *name;
    AvahiIfIndex index;
    int flags_ok;

    unsigned mtu;

    uint8_t mac_address[AVAHI_MAC_ADDRESS_MAX];
    size_t mac_address_size;

    AvahiSEntryGroup *entry_group;

    /* Packet rate limiting */
    struct timeval ratelimit_begin;
    unsigned ratelimit_counter;

    AVAHI_LLIST_HEAD(AvahiInterface, interfaces);
};

struct AvahiInterface {
    AvahiInterfaceMonitor *monitor;
    AvahiHwInterface *hardware;

    AVAHI_LLIST_FIELDS(AvahiInterface, interface);
    AVAHI_LLIST_FIELDS(AvahiInterface, by_hardware);

    AvahiProtocol protocol;
    int announcing;
    AvahiAddress local_mcast_address;
    int mcast_joined;

    AvahiCache *cache;

    AvahiQueryScheduler *query_scheduler;
    AvahiResponseScheduler * response_scheduler;
    AvahiProbeScheduler *probe_scheduler;

    AVAHI_LLIST_HEAD(AvahiInterfaceAddress, addresses);
    AVAHI_LLIST_HEAD(AvahiAnnouncer, announcers);

    AvahiHashmap *queriers_by_key;
    AVAHI_LLIST_HEAD(AvahiQuerier, queriers);
};

struct AvahiInterfaceAddress {
    AvahiInterfaceMonitor *monitor;
    AvahiInterface *interface;

    AVAHI_LLIST_FIELDS(AvahiInterfaceAddress, address);

    AvahiAddress address;
    unsigned prefix_len;

    int global_scope;
    int deprecated;

    AvahiSEntryGroup *entry_group;
};

AvahiInterfaceMonitor *avahi_interface_monitor_new(AvahiServer *server);
void avahi_interface_monitor_free(AvahiInterfaceMonitor *m);

int avahi_interface_monitor_init_osdep(AvahiInterfaceMonitor *m);
void avahi_interface_monitor_free_osdep(AvahiInterfaceMonitor *m);
void avahi_interface_monitor_sync(AvahiInterfaceMonitor *m);

typedef void (*AvahiInterfaceMonitorWalkCallback)(AvahiInterfaceMonitor *m, AvahiInterface *i, void* userdata);
void avahi_interface_monitor_walk(AvahiInterfaceMonitor *m, AvahiIfIndex idx, AvahiProtocol protocol, AvahiInterfaceMonitorWalkCallback callback, void* userdata);
int avahi_dump_caches(AvahiInterfaceMonitor *m, AvahiDumpCallback callback, void* userdata);

void avahi_interface_monitor_update_rrs(AvahiInterfaceMonitor *m, int remove_rrs);
int avahi_address_is_local(AvahiInterfaceMonitor *m, const AvahiAddress *a);
void avahi_interface_monitor_check_relevant(AvahiInterfaceMonitor *m);

/* AvahiHwInterface */

AvahiHwInterface *avahi_hw_interface_new(AvahiInterfaceMonitor *m, AvahiIfIndex idx);
void avahi_hw_interface_free(AvahiHwInterface *hw, int send_goodbye);

void avahi_hw_interface_update_rrs(AvahiHwInterface *hw, int remove_rrs);
void avahi_hw_interface_check_relevant(AvahiHwInterface *hw);

AvahiHwInterface* avahi_interface_monitor_get_hw_interface(AvahiInterfaceMonitor *m, int idx);

/* AvahiInterface */

AvahiInterface* avahi_interface_new(AvahiInterfaceMonitor *m, AvahiHwInterface *hw, AvahiProtocol protocol);
void avahi_interface_free(AvahiInterface *i, int send_goodbye);

void avahi_interface_update_rrs(AvahiInterface *i, int remove_rrs);
void avahi_interface_check_relevant(AvahiInterface *i);
int avahi_interface_is_relevant(AvahiInterface *i);

void avahi_interface_send_packet(AvahiInterface *i, AvahiDnsPacket *p);
void avahi_interface_send_packet_unicast(AvahiInterface *i, AvahiDnsPacket *p, const AvahiAddress *a, uint16_t port);

int avahi_interface_post_query(AvahiInterface *i, AvahiKey *k, int immediately, unsigned *ret_id);
int avahi_interface_withraw_query(AvahiInterface *i, unsigned id);
int avahi_interface_post_response(AvahiInterface *i, AvahiRecord *record, int flush_cache, const AvahiAddress *querier, int immediately);
int avahi_interface_post_probe(AvahiInterface *i, AvahiRecord *p, int immediately);

int avahi_interface_match(AvahiInterface *i, AvahiIfIndex idx, AvahiProtocol protocol);
int avahi_interface_address_on_link(AvahiInterface *i, const AvahiAddress *a);
int avahi_interface_has_address(AvahiInterfaceMonitor *m, AvahiIfIndex iface, const AvahiAddress *a);

AvahiInterface* avahi_interface_monitor_get_interface(AvahiInterfaceMonitor *m, AvahiIfIndex idx, AvahiProtocol protocol);

/* AvahiInterfaceAddress */

AvahiInterfaceAddress *avahi_interface_address_new(AvahiInterfaceMonitor *m, AvahiInterface *i, const AvahiAddress *addr, unsigned prefix_len);
void avahi_interface_address_free(AvahiInterfaceAddress *a);

void avahi_interface_address_update_rrs(AvahiInterfaceAddress *a, int remove_rrs);
int avahi_interface_address_is_relevant(AvahiInterfaceAddress *a);

AvahiInterfaceAddress* avahi_interface_monitor_get_address(AvahiInterfaceMonitor *m, AvahiInterface *i, const AvahiAddress *raddr);

AvahiIfIndex avahi_find_interface_for_address(AvahiInterfaceMonitor *m, const AvahiAddress *a);

#endif
