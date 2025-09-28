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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>

#include "iface.h"
#include "dns.h"
#include "socket.h"
#include "announce.h"
#include "util.h"
#include "log.h"
#include "multicast-lookup.h"
#include "querier.h"

void avahi_interface_address_update_rrs(AvahiInterfaceAddress *a, int remove_rrs) {
    AvahiInterfaceMonitor *m;

    assert(a);
    m = a->monitor;

    if (m->list_complete &&
        avahi_interface_address_is_relevant(a) &&
        avahi_interface_is_relevant(a->interface) &&
        !remove_rrs &&
        m->server->config.publish_addresses &&
        (m->server->state == AVAHI_SERVER_RUNNING ||
        m->server->state == AVAHI_SERVER_REGISTERING)) {

        /* Fill the entry group */
        if (!a->entry_group)
            a->entry_group = avahi_s_entry_group_new(m->server, avahi_host_rr_entry_group_callback, NULL);

        if (!a->entry_group) /* OOM */
            return;

        if (avahi_s_entry_group_is_empty(a->entry_group)) {
            char t[AVAHI_ADDRESS_STR_MAX];
            AvahiProtocol p;

            p = (a->interface->protocol == AVAHI_PROTO_INET && m->server->config.publish_a_on_ipv6) ||
                (a->interface->protocol == AVAHI_PROTO_INET6 && m->server->config.publish_aaaa_on_ipv4) ? AVAHI_PROTO_UNSPEC : a->interface->protocol;

            avahi_address_snprint(t, sizeof(t), &a->address);
            avahi_log_info("Registering new address record for %s on %s.%s.", t, a->interface->hardware->name, p == AVAHI_PROTO_UNSPEC ? "*" : avahi_proto_to_string(p));

            if (avahi_server_add_address(m->server, a->entry_group, a->interface->hardware->index, p, 0, NULL, &a->address) < 0) {
                if (!m->server->config.disable_publishing || m->server->error != AVAHI_ERR_NOT_PERMITTED) {
                    /* suppress warning if disable_publishing set as this is expected state */
                    avahi_log_warn(__FILE__": avahi_server_add_address() failed: %s", avahi_strerror(m->server->error));
                }
                avahi_s_entry_group_free(a->entry_group);
                a->entry_group = NULL;
                return;
            }

            avahi_s_entry_group_commit(a->entry_group);
        }
    } else {

        /* Clear the entry group */

        if (a->entry_group && !avahi_s_entry_group_is_empty(a->entry_group)) {
            char t[AVAHI_ADDRESS_STR_MAX];
            avahi_address_snprint(t, sizeof(t), &a->address);

            avahi_log_info("Withdrawing address record for %s on %s.", t, a->interface->hardware->name);

            if (avahi_s_entry_group_get_state(a->entry_group) == AVAHI_ENTRY_GROUP_REGISTERING &&
                m->server->state == AVAHI_SERVER_REGISTERING)
                avahi_server_decrease_host_rr_pending(m->server);

            avahi_s_entry_group_reset(a->entry_group);
        }
    }
}

void avahi_interface_update_rrs(AvahiInterface *i, int remove_rrs) {
    AvahiInterfaceAddress *a;

    assert(i);

    for (a = i->addresses; a; a = a->address_next)
        avahi_interface_address_update_rrs(a, remove_rrs);
}

void avahi_hw_interface_update_rrs(AvahiHwInterface *hw, int remove_rrs) {
    AvahiInterface *i;
    AvahiInterfaceMonitor *m;

    assert(hw);
    m = hw->monitor;

    for (i = hw->interfaces; i; i = i->by_hardware_next)
        avahi_interface_update_rrs(i, remove_rrs);

    if (m->list_complete &&
        !remove_rrs &&
        m->server->config.publish_workstation &&
        (m->server->state == AVAHI_SERVER_RUNNING)) {

        if (!hw->entry_group)
            hw->entry_group = avahi_s_entry_group_new(m->server, avahi_host_rr_entry_group_callback, NULL);

        if (!hw->entry_group)
            return; /* OOM */

        if (avahi_s_entry_group_is_empty(hw->entry_group)) {
            char name[AVAHI_LABEL_MAX], unescaped[AVAHI_LABEL_MAX], mac[256];
            const char *p = m->server->host_name;

            avahi_unescape_label(&p, unescaped, sizeof(unescaped));
            avahi_format_mac_address(mac, sizeof(mac), hw->mac_address, hw->mac_address_size);
            snprintf(name, sizeof(name), "%s [%s]", unescaped, mac);

            if (avahi_server_add_service(m->server, hw->entry_group, hw->index, AVAHI_PROTO_UNSPEC, 0, name, "_workstation._tcp", NULL, NULL, 9, NULL) < 0) {
                avahi_log_warn(__FILE__": avahi_server_add_service() failed: %s", avahi_strerror(m->server->error));
                avahi_s_entry_group_free(hw->entry_group);
                hw->entry_group = NULL;
            } else
                avahi_s_entry_group_commit(hw->entry_group);
        }

    } else {

        if (hw->entry_group && !avahi_s_entry_group_is_empty(hw->entry_group)) {

            avahi_log_info("Withdrawing workstation service for %s.", hw->name);

            if (avahi_s_entry_group_get_state(hw->entry_group) == AVAHI_ENTRY_GROUP_REGISTERING &&
                m->server->state == AVAHI_SERVER_REGISTERING)
                avahi_server_decrease_host_rr_pending(m->server);

            avahi_s_entry_group_reset(hw->entry_group);
        }
    }
}

void avahi_interface_monitor_update_rrs(AvahiInterfaceMonitor *m, int remove_rrs) {
    AvahiHwInterface *hw;

    assert(m);

    for (hw = m->hw_interfaces; hw; hw = hw->hardware_next)
        avahi_hw_interface_update_rrs(hw, remove_rrs);
}

static int interface_mdns_mcast_join(AvahiInterface *i, int join) {
    char at[AVAHI_ADDRESS_STR_MAX];
    int r;
    assert(i);

    if (!!join  == !!i->mcast_joined)
        return 0;

    if ((i->protocol == AVAHI_PROTO_INET6 && i->monitor->server->fd_ipv6 < 0) ||
        (i->protocol == AVAHI_PROTO_INET && i->monitor->server->fd_ipv4 < 0))
        return -1;

    if (join) {
        AvahiInterfaceAddress *a;

        /* Look if there's an address with global scope */
        for (a = i->addresses; a; a = a->address_next)
            if (a->global_scope)
                break;

        /* No address with a global scope has been found, so let's use
         * any. */
        if (!a)
            a = i->addresses;

        /* Hmm, there is no address available. */
        if (!a)
            return -1;

        i->local_mcast_address = a->address;
    }

    avahi_log_info("%s mDNS multicast group on interface %s.%s with address %s.",
                   join ? "Joining" : "Leaving",
                   i->hardware->name,
                   avahi_proto_to_string(i->protocol),
                   avahi_address_snprint(at, sizeof(at), &i->local_mcast_address));

    if (i->protocol == AVAHI_PROTO_INET6)
        r = avahi_mdns_mcast_join_ipv6(i->monitor->server->fd_ipv6, &i->local_mcast_address.data.ipv6, i->hardware->index, join);
    else {
        assert(i->protocol == AVAHI_PROTO_INET);

        r = avahi_mdns_mcast_join_ipv4(i->monitor->server->fd_ipv4, &i->local_mcast_address.data.ipv4, i->hardware->index, join);
    }

    if (r < 0)
        i->mcast_joined = 0;
    else
        i->mcast_joined = join;

    return 0;
}

static int interface_mdns_mcast_rejoin(AvahiInterface *i) {
    AvahiInterfaceAddress *a, *usable = NULL, *found = NULL;
    assert(i);

    if (!i->mcast_joined)
        return 0;

    /* Check whether old address we joined with is still available. If
     * not, rejoin using an other address. */

    for (a = i->addresses; a; a = a->address_next) {
        if (a->global_scope && !usable)
            usable = a;

        if (avahi_address_cmp(&a->address, &i->local_mcast_address) == 0) {

            if (a->global_scope)
                /* No action necessary: the address still exists and
                 * has global scope. */
                return 0;

            found = a;
        }
    }

    if (found && !usable)
        /* No action necessary: the address still exists and no better one has been found */
        return 0;

    interface_mdns_mcast_join(i, 0);
    return interface_mdns_mcast_join(i, 1);
}

void avahi_interface_address_free(AvahiInterfaceAddress *a) {
    assert(a);
    assert(a->interface);

    avahi_interface_address_update_rrs(a, 1);
    AVAHI_LLIST_REMOVE(AvahiInterfaceAddress, address, a->interface->addresses, a);

    if (a->entry_group)
        avahi_s_entry_group_free(a->entry_group);

    interface_mdns_mcast_rejoin(a->interface);

    avahi_free(a);
}

void avahi_interface_free(AvahiInterface *i, int send_goodbye) {
    assert(i);

    /* Handle goodbyes and remove announcers */
    avahi_goodbye_interface(i->monitor->server, i, send_goodbye, 1);
    avahi_response_scheduler_force(i->response_scheduler);
    assert(!i->announcers);

    if (i->mcast_joined)
        interface_mdns_mcast_join(i, 0);

    /* Remove queriers */
    avahi_querier_free_all(i);
    avahi_hashmap_free(i->queriers_by_key);

    /* Remove local RRs */
    avahi_interface_update_rrs(i, 1);

    while (i->addresses)
        avahi_interface_address_free(i->addresses);

    avahi_response_scheduler_free(i->response_scheduler);
    avahi_query_scheduler_free(i->query_scheduler);
    avahi_probe_scheduler_free(i->probe_scheduler);
    avahi_cache_free(i->cache);

    AVAHI_LLIST_REMOVE(AvahiInterface, interface, i->monitor->interfaces, i);
    AVAHI_LLIST_REMOVE(AvahiInterface, by_hardware, i->hardware->interfaces, i);

    avahi_free(i);
}

void avahi_hw_interface_free(AvahiHwInterface *hw, int send_goodbye) {
    assert(hw);

    avahi_hw_interface_update_rrs(hw, 1);

    while (hw->interfaces)
        avahi_interface_free(hw->interfaces, send_goodbye);

    if (hw->entry_group)
        avahi_s_entry_group_free(hw->entry_group);

    AVAHI_LLIST_REMOVE(AvahiHwInterface, hardware, hw->monitor->hw_interfaces, hw);
    avahi_hashmap_remove(hw->monitor->hashmap, &hw->index);

    avahi_free(hw->name);
    avahi_free(hw);
}

AvahiInterface* avahi_interface_new(AvahiInterfaceMonitor *m, AvahiHwInterface *hw, AvahiProtocol protocol) {
    AvahiInterface *i;

    assert(m);
    assert(hw);
    assert(AVAHI_PROTO_VALID(protocol));

    if (!(i = avahi_new(AvahiInterface, 1)))
        goto fail; /* OOM */

    i->monitor = m;
    i->hardware = hw;
    i->protocol = protocol;
    i->announcing = 0;
    i->mcast_joined = 0;

    AVAHI_LLIST_HEAD_INIT(AvahiInterfaceAddress, i->addresses);
    AVAHI_LLIST_HEAD_INIT(AvahiAnnouncer, i->announcers);

    AVAHI_LLIST_HEAD_INIT(AvahiQuerier, i->queriers);
    i->queriers_by_key = avahi_hashmap_new((AvahiHashFunc) avahi_key_hash, (AvahiEqualFunc) avahi_key_equal, NULL, NULL);

    i->cache = avahi_cache_new(m->server, i);
    i->response_scheduler = avahi_response_scheduler_new(i);
    i->query_scheduler = avahi_query_scheduler_new(i);
    i->probe_scheduler = avahi_probe_scheduler_new(i);

    if (!i->cache || !i->response_scheduler || !i->query_scheduler || !i->probe_scheduler)
        goto fail; /* OOM */

    AVAHI_LLIST_PREPEND(AvahiInterface, by_hardware, hw->interfaces, i);
    AVAHI_LLIST_PREPEND(AvahiInterface, interface, m->interfaces, i);

    return i;

fail:

    if (i) {
        if (i->cache)
            avahi_cache_free(i->cache);
        if (i->response_scheduler)
            avahi_response_scheduler_free(i->response_scheduler);
        if (i->query_scheduler)
            avahi_query_scheduler_free(i->query_scheduler);
        if (i->probe_scheduler)
            avahi_probe_scheduler_free(i->probe_scheduler);
    }

    return NULL;
}

AvahiHwInterface *avahi_hw_interface_new(AvahiInterfaceMonitor *m, AvahiIfIndex idx) {
    AvahiHwInterface *hw;

    assert(m);
    assert(AVAHI_IF_VALID(idx));

    if  (!(hw = avahi_new(AvahiHwInterface, 1)))
        return NULL;

    hw->monitor = m;
    hw->name = NULL;
    hw->flags_ok = 0;
    hw->mtu = 1500;
    hw->index = idx;
    hw->mac_address_size = 0;
    hw->entry_group = NULL;
    hw->ratelimit_begin.tv_sec = 0;
    hw->ratelimit_begin.tv_usec = 0;
    hw->ratelimit_counter = 0;

    AVAHI_LLIST_HEAD_INIT(AvahiInterface, hw->interfaces);
    AVAHI_LLIST_PREPEND(AvahiHwInterface, hardware, m->hw_interfaces, hw);

    avahi_hashmap_insert(m->hashmap, &hw->index, hw);

    if (m->server->fd_ipv4 >= 0 || m->server->config.publish_a_on_ipv6)
        avahi_interface_new(m, hw, AVAHI_PROTO_INET);
    if (m->server->fd_ipv6 >= 0 || m->server->config.publish_aaaa_on_ipv4)
        avahi_interface_new(m, hw, AVAHI_PROTO_INET6);

    return hw;
}

AvahiInterfaceAddress *avahi_interface_address_new(AvahiInterfaceMonitor *m, AvahiInterface *i, const AvahiAddress *addr, unsigned prefix_len) {
    AvahiInterfaceAddress *a;

    assert(m);
    assert(i);

    if (!(a = avahi_new(AvahiInterfaceAddress, 1)))
        return NULL;

    a->interface = i;
    a->monitor = m;
    a->address = *addr;
    a->prefix_len = prefix_len;
    a->global_scope = 0;
    a->deprecated = 0;
    a->entry_group = NULL;

    AVAHI_LLIST_PREPEND(AvahiInterfaceAddress, address, i->addresses, a);

    return a;
}

void avahi_interface_check_relevant(AvahiInterface *i) {
    int b;
    AvahiInterfaceMonitor *m;

    assert(i);
    m = i->monitor;

    b = avahi_interface_is_relevant(i);

    if (m->list_complete && b && !i->announcing) {
        interface_mdns_mcast_join(i, 1);

        if (i->mcast_joined) {
            avahi_log_info("New relevant interface %s.%s for mDNS.", i->hardware->name, avahi_proto_to_string(i->protocol));

            i->announcing = 1;
            avahi_announce_interface(m->server, i);
            avahi_multicast_lookup_engine_new_interface(m->server->multicast_lookup_engine, i);
        }

    } else if (!b && i->announcing) {
        avahi_log_info("Interface %s.%s no longer relevant for mDNS.", i->hardware->name, avahi_proto_to_string(i->protocol));

        interface_mdns_mcast_join(i, 0);

        avahi_goodbye_interface(m->server, i, 0, 1);
        avahi_querier_free_all(i);

        avahi_response_scheduler_clear(i->response_scheduler);
        avahi_query_scheduler_clear(i->query_scheduler);
        avahi_probe_scheduler_clear(i->probe_scheduler);
        avahi_cache_flush(i->cache);

        i->announcing = 0;

    } else
        interface_mdns_mcast_rejoin(i);
}

void avahi_hw_interface_check_relevant(AvahiHwInterface *hw) {
    AvahiInterface *i;

    assert(hw);

    for (i = hw->interfaces; i; i = i->by_hardware_next)
        avahi_interface_check_relevant(i);
}

void avahi_interface_monitor_check_relevant(AvahiInterfaceMonitor *m) {
    AvahiInterface *i;

    assert(m);

    for (i = m->interfaces; i; i = i->interface_next)
        avahi_interface_check_relevant(i);
}

AvahiInterfaceMonitor *avahi_interface_monitor_new(AvahiServer *s) {
    AvahiInterfaceMonitor *m = NULL;

    if (!(m = avahi_new0(AvahiInterfaceMonitor, 1)))
        return NULL; /* OOM */

    m->server = s;
    m->list_complete = 0;
    m->hashmap = avahi_hashmap_new(avahi_int_hash, avahi_int_equal, NULL, NULL);

    AVAHI_LLIST_HEAD_INIT(AvahiInterface, m->interfaces);
    AVAHI_LLIST_HEAD_INIT(AvahiHwInterface, m->hw_interfaces);

    if (avahi_interface_monitor_init_osdep(m) < 0)
        goto fail;

    return m;

fail:
    avahi_interface_monitor_free(m);
    return NULL;
}

void avahi_interface_monitor_free(AvahiInterfaceMonitor *m) {
    assert(m);

    while (m->hw_interfaces)
        avahi_hw_interface_free(m->hw_interfaces, 1);

    assert(!m->interfaces);

    avahi_interface_monitor_free_osdep(m);

    if (m->hashmap)
        avahi_hashmap_free(m->hashmap);

    avahi_free(m);
}


AvahiInterface* avahi_interface_monitor_get_interface(AvahiInterfaceMonitor *m, AvahiIfIndex idx, AvahiProtocol protocol) {
    AvahiHwInterface *hw;
    AvahiInterface *i;

    assert(m);
    assert(idx >= 0);
    assert(protocol != AVAHI_PROTO_UNSPEC);

    if (!(hw = avahi_interface_monitor_get_hw_interface(m, idx)))
        return NULL;

    for (i = hw->interfaces; i; i = i->by_hardware_next)
        if (i->protocol == protocol)
            return i;

    return NULL;
}

AvahiHwInterface* avahi_interface_monitor_get_hw_interface(AvahiInterfaceMonitor *m, AvahiIfIndex idx) {
    assert(m);
    assert(idx >= 0);

    return avahi_hashmap_lookup(m->hashmap, &idx);
}

AvahiInterfaceAddress* avahi_interface_monitor_get_address(AvahiInterfaceMonitor *m, AvahiInterface *i, const AvahiAddress *raddr) {
    AvahiInterfaceAddress *ia;

    assert(m);
    assert(i);
    assert(raddr);

    for (ia = i->addresses; ia; ia = ia->address_next)
        if (avahi_address_cmp(&ia->address, raddr) == 0)
            return ia;

    return NULL;
}

void avahi_interface_send_packet_unicast(AvahiInterface *i, AvahiDnsPacket *p, const AvahiAddress *a, uint16_t port) {
    assert(i);
    assert(p);

    if (!i->announcing)
        return;

    assert(!a || a->proto == i->protocol);

    if (i->monitor->server->config.ratelimit_interval > 0) {
        struct timeval now, end;

        gettimeofday(&now, NULL);

        end = i->hardware->ratelimit_begin;
        avahi_timeval_add(&end, i->monitor->server->config.ratelimit_interval);

        if (i->hardware->ratelimit_begin.tv_sec <= 0 ||
            avahi_timeval_compare(&end, &now) < 0) {

            i->hardware->ratelimit_begin = now;
            i->hardware->ratelimit_counter = 0;
        }

        if (i->hardware->ratelimit_counter > i->monitor->server->config.ratelimit_burst)
            return;

        i->hardware->ratelimit_counter++;
    }

    if (i->protocol == AVAHI_PROTO_INET && i->monitor->server->fd_ipv4 >= 0)
        avahi_send_dns_packet_ipv4(i->monitor->server->fd_ipv4, i->hardware->index, p, i->mcast_joined ? &i->local_mcast_address.data.ipv4 : NULL, a ? &a->data.ipv4 : NULL, port);
    else if (i->protocol == AVAHI_PROTO_INET6 && i->monitor->server->fd_ipv6 >= 0)
        avahi_send_dns_packet_ipv6(i->monitor->server->fd_ipv6, i->hardware->index, p, i->mcast_joined ? &i->local_mcast_address.data.ipv6 : NULL, a ? &a->data.ipv6 : NULL, port);
}

void avahi_interface_send_packet(AvahiInterface *i, AvahiDnsPacket *p) {
    assert(i);
    assert(p);

    avahi_interface_send_packet_unicast(i, p, NULL, 0);
}

int avahi_interface_post_query(AvahiInterface *i, AvahiKey *key, int immediately, unsigned *ret_id) {
    assert(i);
    assert(key);

    if (!i->announcing)
        return 0;

    return avahi_query_scheduler_post(i->query_scheduler, key, immediately, ret_id);
}

int avahi_interface_withraw_query(AvahiInterface *i, unsigned id) {

    return avahi_query_scheduler_withdraw_by_id(i->query_scheduler, id);
}

int avahi_interface_post_response(AvahiInterface *i, AvahiRecord *record, int flush_cache, const AvahiAddress *querier, int immediately) {
    assert(i);
    assert(record);

    if (!i->announcing)
        return 0;

    return avahi_response_scheduler_post(i->response_scheduler, record, flush_cache, querier, immediately);
}

int avahi_interface_post_probe(AvahiInterface *i, AvahiRecord *record, int immediately) {
    assert(i);
    assert(record);

    if (!i->announcing)
        return 0;

    return avahi_probe_scheduler_post(i->probe_scheduler, record, immediately);
}

int avahi_dump_caches(AvahiInterfaceMonitor *m, AvahiDumpCallback callback, void* userdata) {
    AvahiInterface *i;
    assert(m);

    for (i = m->interfaces; i; i = i->interface_next) {
        if (avahi_interface_is_relevant(i)) {
            char ln[256];
            snprintf(ln, sizeof(ln), ";;; INTERFACE %s.%s ;;;", i->hardware->name, avahi_proto_to_string(i->protocol));
            callback(ln, userdata);
            if (avahi_cache_dump(i->cache, callback, userdata) < 0)
                return -1;
        }
    }

    return 0;
}

static int avahi_interface_is_relevant_internal(AvahiInterface *i) {
    AvahiInterfaceAddress *a;

    assert(i);

    if (!i->hardware->flags_ok)
        return 0;

    for (a = i->addresses; a; a = a->address_next)
        if (avahi_interface_address_is_relevant(a))
            return 1;

    return 0;
}

int avahi_interface_is_relevant(AvahiInterface *i) {
    AvahiStringList *l;
    assert(i);

    for (l = i->monitor->server->config.deny_interfaces; l; l = l->next)
        if (strcasecmp((char*) l->text, i->hardware->name) == 0)
            return 0;

    if (i->monitor->server->config.allow_interfaces) {

        for (l = i->monitor->server->config.allow_interfaces; l; l = l->next)
            if (strcasecmp((char*) l->text, i->hardware->name) == 0)
                goto good;

        return 0;
    }

good:
    return avahi_interface_is_relevant_internal(i);
}

int avahi_interface_address_is_relevant(AvahiInterfaceAddress *a) {
    AvahiInterfaceAddress *b;
    assert(a);

    /* Publish public and non-deprecated IP addresses */
    if (a->global_scope && !a->deprecated)
        return 1;

    /* Publish link-local and deprecated IP addresses only if they are
     * the only ones on the link */
    for (b = a->interface->addresses; b; b = b->address_next) {
        if (b == a)
            continue;

        if (b->global_scope && !b->deprecated)
            return 0;
    }

    return 1;
}

int avahi_interface_match(AvahiInterface *i, AvahiIfIndex idx, AvahiProtocol protocol) {
    assert(i);

    if (idx != AVAHI_IF_UNSPEC && idx != i->hardware->index)
        return 0;

    if (protocol != AVAHI_PROTO_UNSPEC && protocol != i->protocol)
        return 0;

    return 1;
}

void avahi_interface_monitor_walk(AvahiInterfaceMonitor *m, AvahiIfIndex interface, AvahiProtocol protocol, AvahiInterfaceMonitorWalkCallback callback, void* userdata) {
    assert(m);
    assert(callback);

    if (interface != AVAHI_IF_UNSPEC) {
        if (protocol != AVAHI_PROTO_UNSPEC) {
            AvahiInterface *i;

            if ((i = avahi_interface_monitor_get_interface(m, interface, protocol)))
                callback(m, i, userdata);

        } else {
            AvahiHwInterface *hw;
            AvahiInterface *i;

            if ((hw = avahi_interface_monitor_get_hw_interface(m, interface)))
                for (i = hw->interfaces; i; i = i->by_hardware_next)
                    if (avahi_interface_match(i, interface, protocol))
                        callback(m, i, userdata);
        }

    } else {
        AvahiInterface *i;

        for (i = m->interfaces; i; i = i->interface_next)
            if (avahi_interface_match(i, interface, protocol))
                callback(m, i, userdata);
    }
}


int avahi_address_is_local(AvahiInterfaceMonitor *m, const AvahiAddress *a) {
    AvahiInterface *i;
    AvahiInterfaceAddress *ia;
    assert(m);
    assert(a);

    for (i = m->interfaces; i; i = i->interface_next)
        for (ia = i->addresses; ia; ia = ia->address_next)
            if (avahi_address_cmp(a, &ia->address) == 0)
                return 1;

    return 0;
}

int avahi_interface_address_on_link(AvahiInterface *i, const AvahiAddress *a) {
    AvahiInterfaceAddress *ia;

    assert(i);
    assert(a);

    if (a->proto != i->protocol)
        return 0;

    for (ia = i->addresses; ia; ia = ia->address_next) {

        if (a->proto == AVAHI_PROTO_INET) {
            uint32_t m;

            m = ~(((uint32_t) -1) >> ia->prefix_len);

            if ((ntohl(a->data.ipv4.address) & m) == (ntohl(ia->address.data.ipv4.address) & m))
                return 1;
        } else {
            unsigned j;
            unsigned char pl;
            assert(a->proto == AVAHI_PROTO_INET6);

            pl = ia->prefix_len;

            for (j = 0; j < 16; j++) {
                uint8_t m;

                if (pl == 0)
                    return 1;

                if (pl >= 8) {
                    m = 0xFF;
                    pl -= 8;
                } else {
                    m = ~(0xFF >> pl);
                    pl = 0;
                }

                if ((a->data.ipv6.address[j] & m) != (ia->address.data.ipv6.address[j] & m))
                    break;
            }
        }
    }

    return 0;
}

int avahi_interface_has_address(AvahiInterfaceMonitor *m, AvahiIfIndex iface, const AvahiAddress *a) {
    AvahiInterface *i;
    AvahiInterfaceAddress *j;

    assert(m);
    assert(iface != AVAHI_IF_UNSPEC);
    assert(a);

    if (!(i = avahi_interface_monitor_get_interface(m, iface, a->proto)))
        return 0;

    for (j = i->addresses; j; j = j->address_next)
        if (avahi_address_cmp(a, &j->address) == 0)
            return 1;

    return 0;
}

AvahiIfIndex avahi_find_interface_for_address(AvahiInterfaceMonitor *m, const AvahiAddress *a) {
    AvahiInterface *i;
    assert(m);

    /* Some stupid OS don't support passing the interface index when a
     * packet is received. We have to work around that limitation by
     * looking for an interface that has the incoming address
     * attached. This is sometimes ambiguous, but we have to live with
     * it. */

    for (i = m->interfaces; i; i = i->interface_next) {
        AvahiInterfaceAddress *ai;

        if (i->protocol != a->proto)
            continue;

        for (ai = i->addresses; ai; ai = ai->address_next)
            if (avahi_address_cmp(a, &ai->address) == 0)
                return i->hardware->index;
    }

    return AVAHI_IF_UNSPEC;
}
