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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <avahi-common/domain.h>
#include <avahi-common/timeval.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include "internal.h"
#include "iface.h"
#include "socket.h"
#include "browse.h"
#include "log.h"
#include "util.h"
#include "dns-srv-rr.h"
#include "addr-util.h"
#include "domain-util.h"
#include "rr-util.h"

#define AVAHI_DEFAULT_CACHE_ENTRIES_MAX 4096

static void enum_aux_records(AvahiServer *s, AvahiInterface *i, const char *name, uint16_t type, void (*callback)(AvahiServer *s, AvahiRecord *r, int flush_cache, void* userdata), void* userdata) {
    assert(s);
    assert(i);
    assert(name);
    assert(callback);

    if (type == AVAHI_DNS_TYPE_ANY) {
        AvahiEntry *e;

        for (e = s->entries; e; e = e->entries_next)
            if (!e->dead &&
                avahi_entry_is_registered(s, e, i) &&
                e->record->key->clazz == AVAHI_DNS_CLASS_IN &&
                avahi_domain_equal(name, e->record->key->name))
                callback(s, e->record, e->flags & AVAHI_PUBLISH_UNIQUE, userdata);

    } else {
        AvahiEntry *e;
        AvahiKey *k;

        if (!(k = avahi_key_new(name, AVAHI_DNS_CLASS_IN, type)))
            return; /** OOM */

        for (e = avahi_hashmap_lookup(s->entries_by_key, k); e; e = e->by_key_next)
            if (!e->dead && avahi_entry_is_registered(s, e, i))
                callback(s, e->record, e->flags & AVAHI_PUBLISH_UNIQUE, userdata);

        avahi_key_unref(k);
    }
}

void avahi_server_enumerate_aux_records(AvahiServer *s, AvahiInterface *i, AvahiRecord *r, void (*callback)(AvahiServer *s, AvahiRecord *r, int flush_cache, void* userdata), void* userdata) {
    assert(s);
    assert(i);
    assert(r);
    assert(callback);

    /* Call the specified callback far all records referenced by the one specified in *r */

    if (r->key->clazz == AVAHI_DNS_CLASS_IN) {
        if (r->key->type == AVAHI_DNS_TYPE_PTR) {
            enum_aux_records(s, i, r->data.ptr.name, AVAHI_DNS_TYPE_SRV, callback, userdata);
            enum_aux_records(s, i, r->data.ptr.name, AVAHI_DNS_TYPE_TXT, callback, userdata);
        } else if (r->key->type == AVAHI_DNS_TYPE_SRV) {
            enum_aux_records(s, i, r->data.srv.name, AVAHI_DNS_TYPE_A, callback, userdata);
            enum_aux_records(s, i, r->data.srv.name, AVAHI_DNS_TYPE_AAAA, callback, userdata);
        } else if (r->key->type == AVAHI_DNS_TYPE_CNAME)
            enum_aux_records(s, i, r->data.cname.name, AVAHI_DNS_TYPE_ANY, callback, userdata);
    }
}

void avahi_server_prepare_response(AvahiServer *s, AvahiInterface *i, AvahiEntry *e, int unicast_response, int auxiliary) {
    assert(s);
    assert(i);
    assert(e);

    avahi_record_list_push(s->record_list, e->record, e->flags & AVAHI_PUBLISH_UNIQUE, unicast_response, auxiliary);
}

void avahi_server_prepare_matching_responses(AvahiServer *s, AvahiInterface *i, AvahiKey *k, int unicast_response) {
    assert(s);
    assert(i);
    assert(k);

    /* Push all records that match the specified key to the record list */

    if (avahi_key_is_pattern(k)) {
        AvahiEntry *e;

        /* Handle ANY query */

        for (e = s->entries; e; e = e->entries_next)
            if (!e->dead && avahi_key_pattern_match(k, e->record->key) && avahi_entry_is_registered(s, e, i))
                avahi_server_prepare_response(s, i, e, unicast_response, 0);

    } else {
        AvahiEntry *e;

        /* Handle all other queries */

        for (e = avahi_hashmap_lookup(s->entries_by_key, k); e; e = e->by_key_next)
            if (!e->dead && avahi_entry_is_registered(s, e, i))
                avahi_server_prepare_response(s, i, e, unicast_response, 0);
    }

    /* Look for CNAME records */

    if ((k->clazz == AVAHI_DNS_CLASS_IN || k->clazz == AVAHI_DNS_CLASS_ANY)
        && k->type != AVAHI_DNS_TYPE_CNAME && k->type != AVAHI_DNS_TYPE_ANY) {

        AvahiKey *cname_key;

        if (!(cname_key = avahi_key_new(k->name, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_CNAME)))
            return;

        avahi_server_prepare_matching_responses(s, i, cname_key, unicast_response);
        avahi_key_unref(cname_key);
    }
}

static void withdraw_entry(AvahiServer *s, AvahiEntry *e) {
    assert(s);
    assert(e);

    /* Withdraw the specified entry, and if is part of an entry group,
     * put that into COLLISION state */

    if (e->dead)
        return;

    if (e->group) {
        AvahiEntry *k;

        for (k = e->group->entries; k; k = k->by_group_next)
            if (!k->dead) {
                avahi_goodbye_entry(s, k, 0, 1);
                k->dead = 1;
            }

        e->group->n_probing = 0;

        avahi_s_entry_group_change_state(e->group, AVAHI_ENTRY_GROUP_COLLISION);
    } else {
        avahi_goodbye_entry(s, e, 0, 1);
        e->dead = 1;
    }

    s->need_entry_cleanup = 1;
}

static void withdraw_rrset(AvahiServer *s, AvahiKey *key) {
    AvahiEntry *e;

    assert(s);
    assert(key);

    /* Withdraw an entry RRSset */

    for (e = avahi_hashmap_lookup(s->entries_by_key, key); e; e = e->by_key_next)
        withdraw_entry(s, e);
}

static void incoming_probe(AvahiServer *s, AvahiRecord *record, AvahiInterface *i) {
    AvahiEntry *e, *n;
    int ours = 0, won = 0, lost = 0;

    assert(s);
    assert(record);
    assert(i);

    /* Handle incoming probes and check if they conflict our own probes */

    for (e = avahi_hashmap_lookup(s->entries_by_key, record->key); e; e = n) {
        int cmp;
        n = e->by_key_next;

        if (e->dead)
            continue;

        if ((cmp = avahi_record_lexicographical_compare(e->record, record)) == 0) {
            ours = 1;
            break;
        } else {

            if (avahi_entry_is_probing(s, e, i)) {
                if (cmp > 0)
                    won = 1;
                else /* cmp < 0 */
                    lost = 1;
            }
        }
    }

    if (!ours) {
        char *t = avahi_record_to_string(record);

        if (won)
            avahi_log_debug("Received conflicting probe [%s]. Local host won.", t);
        else if (lost) {
            avahi_log_debug("Received conflicting probe [%s]. Local host lost. Withdrawing.", t);
            withdraw_rrset(s, record->key);
        }

        avahi_free(t);
    }
}

static int handle_conflict(AvahiServer *s, AvahiInterface *i, AvahiRecord *record, int unique) {
    int valid = 1, ours = 0, conflict = 0, withdraw_immediately = 0;
    AvahiEntry *e, *n, *conflicting_entry = NULL;

    assert(s);
    assert(i);
    assert(record);

    /* Check whether an incoming record conflicts with one of our own */

    for (e = avahi_hashmap_lookup(s->entries_by_key, record->key); e; e = n) {
        n = e->by_key_next;

        if (e->dead)
            continue;

        /* Check if the incoming is a goodbye record */
        if (avahi_record_is_goodbye(record)) {

            if (avahi_record_equal_no_ttl(e->record, record)) {
                char *t;

                /* Refresh */
                t = avahi_record_to_string(record);
                avahi_log_debug("Received goodbye record for one of our records [%s]. Refreshing.", t);
                avahi_server_prepare_matching_responses(s, i, e->record->key, 0);

                valid = 0;
                avahi_free(t);
                break;
            }

            /* If the goodybe packet doesn't match one of our own RRs, we simply ignore it. */
            continue;
        }

        if (!(e->flags & AVAHI_PUBLISH_UNIQUE) && !unique)
            continue;

        /* Either our entry or the other is intended to be unique, so let's check */

        if (avahi_record_equal_no_ttl(e->record, record)) {
            ours = 1; /* We have an identical record, so this is no conflict */

            /* Check wheter there is a TTL conflict */
            if (record->ttl <= e->record->ttl/2 &&
                avahi_entry_is_registered(s, e, i)) {
                char *t;
                /* Refresh */
                t = avahi_record_to_string(record);

                avahi_log_debug("Received record with bad TTL [%s]. Refreshing.", t);
                avahi_server_prepare_matching_responses(s, i, e->record->key, 0);
                valid = 0;

                avahi_free(t);
            }

            /* There's no need to check the other entries of this RRset */
            break;

        } else {

            if (avahi_entry_is_registered(s, e, i)) {

                /* A conflict => we have to return to probe mode */
                conflict = 1;
                conflicting_entry = e;

            } else if (avahi_entry_is_probing(s, e, i)) {

                /* We are currently registering a matching record, but
                 * someone else already claimed it, so let's
                 * withdraw */
                conflict = 1;
                withdraw_immediately = 1;
            }
        }
    }

    if (!ours && conflict) {
        char *t;

        valid = 0;

        t = avahi_record_to_string(record);

        if (withdraw_immediately) {
            avahi_log_debug("Received conflicting record [%s] with local record to be. Withdrawing.", t);
            withdraw_rrset(s, record->key);
        } else {
            assert(conflicting_entry);
            avahi_log_debug("Received conflicting record [%s]. Resetting our record.", t);
            avahi_entry_return_to_initial_state(s, conflicting_entry, i);

            /* Local unique records are returned to probing
             * state. Local shared records are reannounced. */
        }

        avahi_free(t);
    }

    return valid;
}

static void append_aux_callback(AvahiServer *s, AvahiRecord *r, int flush_cache, void* userdata) {
    int *unicast_response = userdata;

    assert(s);
    assert(r);
    assert(unicast_response);

    avahi_record_list_push(s->record_list, r, flush_cache, *unicast_response, 1);
}

static void append_aux_records_to_list(AvahiServer *s, AvahiInterface *i, AvahiRecord *r, int unicast_response) {
    assert(s);
    assert(r);

    avahi_server_enumerate_aux_records(s, i, r, append_aux_callback, &unicast_response);
}

void avahi_server_generate_response(AvahiServer *s, AvahiInterface *i, AvahiDnsPacket *p, const AvahiAddress *a, uint16_t port, int legacy_unicast, int immediately) {

    assert(s);
    assert(i);
    assert(!legacy_unicast || (a && port > 0 && p));

    if (legacy_unicast) {
        AvahiDnsPacket *reply;
        AvahiRecord *r;

        if (!(reply = avahi_dns_packet_new_reply(p, 512 + AVAHI_DNS_PACKET_EXTRA_SIZE /* unicast DNS maximum packet size is 512 */ , 1, 1)))
            return; /* OOM */

        while ((r = avahi_record_list_next(s->record_list, NULL, NULL, NULL))) {

            append_aux_records_to_list(s, i, r, 0);

            if (avahi_dns_packet_append_record(reply, r, 0, 10))
                avahi_dns_packet_inc_field(reply, AVAHI_DNS_FIELD_ANCOUNT);
            else {
                char *t = avahi_record_to_string(r);
                avahi_log_warn("Record [%s] not fitting in legacy unicast packet, dropping.", t);
                avahi_free(t);
            }

            avahi_record_unref(r);
        }

        if (avahi_dns_packet_get_field(reply, AVAHI_DNS_FIELD_ANCOUNT) != 0)
            avahi_interface_send_packet_unicast(i, reply, a, port);

        avahi_dns_packet_free(reply);

    } else {
        int unicast_response, flush_cache, auxiliary;
        AvahiDnsPacket *reply = NULL;
        AvahiRecord *r;

        /* In case the query packet was truncated never respond
        immediately, because known answer suppression records might be
        contained in later packets */
        int tc = p && !!(avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_FLAGS) & AVAHI_DNS_FLAG_TC);

        while ((r = avahi_record_list_next(s->record_list, &flush_cache, &unicast_response, &auxiliary))) {

            int im = immediately;

            /* Only send the response immediately if it contains a
             * unique entry AND it is not in reply to a truncated
             * packet AND it is not an auxiliary record AND all other
             * responses for this record are unique too. */

            if (flush_cache && !tc && !auxiliary && avahi_record_list_all_flush_cache(s->record_list))
                im = 1;

            if (!avahi_interface_post_response(i, r, flush_cache, a, im) && unicast_response) {

                /* Due to some reasons the record has not been scheduled.
                 * The client requested an unicast response in that
                 * case. Therefore we prepare such a response */

                append_aux_records_to_list(s, i, r, unicast_response);

                for (;;) {

                    if (!reply) {
                        assert(p);

                        if (!(reply = avahi_dns_packet_new_reply(p, i->hardware->mtu, 0, 0)))
                            break; /* OOM */
                    }

                    if (avahi_dns_packet_append_record(reply, r, flush_cache, 0)) {

                        /* Appending this record succeeded, so incremeant
                         * the specific header field, and return to the caller */

                        avahi_dns_packet_inc_field(reply, AVAHI_DNS_FIELD_ANCOUNT);
                        break;
                    }

                    if (avahi_dns_packet_get_field(reply, AVAHI_DNS_FIELD_ANCOUNT) == 0) {
                        size_t size;

                        /* The record is too large for one packet, so create a larger packet */

                        avahi_dns_packet_free(reply);
                        size = avahi_record_get_estimate_size(r) + AVAHI_DNS_PACKET_HEADER_SIZE;

                        if (!(reply = avahi_dns_packet_new_reply(p, size + AVAHI_DNS_PACKET_EXTRA_SIZE, 0, 1)))
                            break; /* OOM */

                        if (avahi_dns_packet_append_record(reply, r, flush_cache, 0)) {

                            /* Appending this record succeeded, so incremeant
                             * the specific header field, and return to the caller */

                            avahi_dns_packet_inc_field(reply, AVAHI_DNS_FIELD_ANCOUNT);
                            break;

                        }  else {

                            /* We completely fucked up, there's
                             * nothing we can do. The RR just doesn't
                             * fit in. Let's ignore it. */

                            char *t;
                            avahi_dns_packet_free(reply);
                            reply = NULL;
                            t = avahi_record_to_string(r);
                            avahi_log_warn("Record [%s] too large, doesn't fit in any packet!", t);
                            avahi_free(t);
                            break;
                        }
                    }

                    /* Appending the record didn't succeeed, so let's send this packet, and create a new one */
                    avahi_interface_send_packet_unicast(i, reply, a, port);
                    avahi_dns_packet_free(reply);
                    reply = NULL;
                }
            }

            avahi_record_unref(r);
        }

        if (reply) {
            if (avahi_dns_packet_get_field(reply, AVAHI_DNS_FIELD_ANCOUNT) != 0)
                avahi_interface_send_packet_unicast(i, reply, a, port);
            avahi_dns_packet_free(reply);
        }
    }

    avahi_record_list_flush(s->record_list);
}

static void reflect_response(AvahiServer *s, AvahiInterface *i, AvahiRecord *r, int flush_cache) {
    AvahiInterface *j;

    assert(s);
    assert(i);
    assert(r);

    if (!s->config.enable_reflector)
        return;

    for (j = s->monitor->interfaces; j; j = j->interface_next)
        if (j != i && (s->config.reflect_ipv || j->protocol == i->protocol))
            avahi_interface_post_response(j, r, flush_cache, NULL, 1);
}

static void* reflect_cache_walk_callback(AvahiCache *c, AvahiKey *pattern, AvahiCacheEntry *e, void* userdata) {
    AvahiServer *s = userdata;
    AvahiRecord* r;

    assert(c);
    assert(pattern);
    assert(e);
    assert(s);

    /* Don't reflect cache entry with ipv6 link-local addresses. */
    r = e->record;
    if ((r->key->type == AVAHI_DNS_TYPE_AAAA) &&
            (r->data.aaaa.address.address[0] == 0xFE) &&
            (r->data.aaaa.address.address[1] == 0x80))
      return NULL;

    avahi_record_list_push(s->record_list, e->record, e->cache_flush, 0, 0);
    return NULL;
}

static void reflect_query(AvahiServer *s, AvahiInterface *i, AvahiKey *k) {
    AvahiInterface *j;

    assert(s);
    assert(i);
    assert(k);

    if (!s->config.enable_reflector)
        return;

    for (j = s->monitor->interfaces; j; j = j->interface_next)
        if (j != i && (s->config.reflect_ipv || j->protocol == i->protocol)) {
            /* Post the query to other networks */
            avahi_interface_post_query(j, k, 1, NULL);

            /* Reply from caches of other network. This is needed to
             * "work around" known answer suppression. */

            avahi_cache_walk(j->cache, k, reflect_cache_walk_callback, s);
        }
}

static void reflect_probe(AvahiServer *s, AvahiInterface *i, AvahiRecord *r) {
    AvahiInterface *j;

    assert(s);
    assert(i);
    assert(r);

    if (!s->config.enable_reflector)
        return;

    for (j = s->monitor->interfaces; j; j = j->interface_next)
        if (j != i && (s->config.reflect_ipv || j->protocol == i->protocol))
            avahi_interface_post_probe(j, r, 1);
}

static void handle_query_packet(AvahiServer *s, AvahiDnsPacket *p, AvahiInterface *i, const AvahiAddress *a, uint16_t port, int legacy_unicast, int from_local_iface) {
    size_t n;
    int is_probe;

    assert(s);
    assert(p);
    assert(i);
    assert(a);

    assert(avahi_record_list_is_empty(s->record_list));

    is_probe = avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_NSCOUNT) > 0;

    /* Handle the questions */
    for (n = avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_QDCOUNT); n > 0; n --) {
        AvahiKey *key;
        int unicast_response = 0;

        if (!(key = avahi_dns_packet_consume_key(p, &unicast_response))) {
            avahi_log_debug(__FILE__": Packet too short or invalid while reading question key. (Maybe a UTF-8 problem?)");
            goto fail;
        }

        if (!legacy_unicast && !from_local_iface) {
            reflect_query(s, i, key);
            if (!unicast_response)
              avahi_cache_start_poof(i->cache, key, a);
        }

        if (avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_ANCOUNT) == 0 &&
            !(avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_FLAGS) & AVAHI_DNS_FLAG_TC))
            /* Allow our own queries to be suppressed by incoming
             * queries only when they do not include known answers */
            avahi_query_scheduler_incoming(i->query_scheduler, key);

        avahi_server_prepare_matching_responses(s, i, key, unicast_response);
        avahi_key_unref(key);
    }

    if (!legacy_unicast) {

        /* Known Answer Suppression */
        for (n = avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_ANCOUNT); n > 0; n --) {
            AvahiRecord *record;
            int unique = 0;

            if (!(record = avahi_dns_packet_consume_record(p, &unique))) {
                avahi_log_debug(__FILE__": Packet too short or invalid while reading known answer record. (Maybe a UTF-8 problem?)");
                goto fail;
            }

            avahi_response_scheduler_suppress(i->response_scheduler, record, a);
            avahi_record_list_drop(s->record_list, record);
            avahi_cache_stop_poof(i->cache, record, a);

            avahi_record_unref(record);
        }

        /* Probe record */
        for (n = avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_NSCOUNT); n > 0; n --) {
            AvahiRecord *record;
            int unique = 0;

            if (!(record = avahi_dns_packet_consume_record(p, &unique))) {
                avahi_log_debug(__FILE__": Packet too short or invalid while reading probe record. (Maybe a UTF-8 problem?)");
                goto fail;
            }

            if (!avahi_key_is_pattern(record->key)) {
                if (!from_local_iface)
                    reflect_probe(s, i, record);
                incoming_probe(s, record, i);
            }

            avahi_record_unref(record);
        }
    }

    if (!avahi_record_list_is_empty(s->record_list))
        avahi_server_generate_response(s, i, p, a, port, legacy_unicast, is_probe);

    return;

fail:
    avahi_record_list_flush(s->record_list);
}

static void handle_response_packet(AvahiServer *s, AvahiDnsPacket *p, AvahiInterface *i, const AvahiAddress *a, int from_local_iface) {
    unsigned n;

    assert(s);
    assert(p);
    assert(i);
    assert(a);

    for (n = avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_ANCOUNT) +
             avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_ARCOUNT); n > 0; n--) {
        AvahiRecord *record;
        int cache_flush = 0;

        if (!(record = avahi_dns_packet_consume_record(p, &cache_flush))) {
            avahi_log_debug(__FILE__": Packet too short or invalid while reading response record. (Maybe a UTF-8 problem?)");
            break;
        }

        if (!avahi_key_is_pattern(record->key)) {
            /* Filter services that will be cached. Allow all local services */
            if (!from_local_iface && s->config.enable_reflector && s->config.reflect_filters != NULL) {
               AvahiStringList *l;
               int match = 0;

                if (record->key->type == AVAHI_DNS_TYPE_PTR) {
                    /* Need to match DNS pointer target with filter */
                    for (l = s->config.reflect_filters; l; l = l->next) {
                        if (strstr(record->data.ptr.name, (char*) l->text) != NULL) {
                            match = 1;
                            break;
                        }
                    }

                    if (!match) {
                        avahi_log_debug("Reject Ptr SRC [%s] Dest [%s]", record->key->name, record->data.ptr.name);
                        goto unref;
                    }
                    else
                        avahi_log_debug("Match Ptr SRC [%s] Dest [%s]", record->key->name, record->data.ptr.name);
                }
                else if (record->key->type == AVAHI_DNS_TYPE_SRV || record->key->type == AVAHI_DNS_TYPE_TXT) {
                    /* Need to match key name with filter */
                    for (l = s->config.reflect_filters; l; l = l->next) {
                        if (strstr(record->key->name, (char*) l->text) != NULL) {
                            match = 1;
                            break;
                        }
                    }

                    if (!match) {
                        avahi_log_debug("Reject Key [%s] iface [%d]", record->key->name, from_local_iface);
                        goto unref;
                    }
                    else
                        avahi_log_debug("Match Key [%s] iface [%d]", record->key->name, from_local_iface);
                }
            }

            if (handle_conflict(s, i, record, cache_flush)) {
                if (!from_local_iface) {
                    if (!avahi_record_is_link_local_address(record))
                        reflect_response(s, i, record, cache_flush);
                    avahi_cache_update(i->cache, record, cache_flush, a);
                }
                avahi_response_scheduler_incoming(i->response_scheduler, record, cache_flush);
            }
        }

    unref:
        avahi_record_unref(record);
    }

    /* If the incoming response contained a conflicting record, some
       records have been scheduled for sending. We need to flush them
       here. */
    if (!avahi_record_list_is_empty(s->record_list))
        avahi_server_generate_response(s, i, NULL, NULL, 0, 0, 1);
}

static AvahiLegacyUnicastReflectSlot* allocate_slot(AvahiServer *s) {
    unsigned n, idx = (unsigned) -1;
    AvahiLegacyUnicastReflectSlot *slot;

    assert(s);

    if (!s->legacy_unicast_reflect_slots)
        s->legacy_unicast_reflect_slots = avahi_new0(AvahiLegacyUnicastReflectSlot*, AVAHI_LEGACY_UNICAST_REFLECT_SLOTS_MAX);

    for (n = 0; n < AVAHI_LEGACY_UNICAST_REFLECT_SLOTS_MAX; n++, s->legacy_unicast_reflect_id++) {
        idx = s->legacy_unicast_reflect_id % AVAHI_LEGACY_UNICAST_REFLECT_SLOTS_MAX;

        if (!s->legacy_unicast_reflect_slots[idx])
            break;
    }

    if (idx == (unsigned) -1 || s->legacy_unicast_reflect_slots[idx])
        return NULL;

    if (!(slot = avahi_new(AvahiLegacyUnicastReflectSlot, 1)))
        return NULL; /* OOM */

    s->legacy_unicast_reflect_slots[idx] = slot;
    slot->id = s->legacy_unicast_reflect_id++;
    slot->server = s;

    return slot;
}

static void deallocate_slot(AvahiServer *s, AvahiLegacyUnicastReflectSlot *slot) {
    unsigned idx;

    assert(s);
    assert(slot);

    idx = slot->id % AVAHI_LEGACY_UNICAST_REFLECT_SLOTS_MAX;

    assert(s->legacy_unicast_reflect_slots[idx] == slot);

    avahi_time_event_free(slot->time_event);

    avahi_free(slot);
    s->legacy_unicast_reflect_slots[idx] = NULL;
}

static void free_slots(AvahiServer *s) {
    unsigned idx;
    assert(s);

    if (!s->legacy_unicast_reflect_slots)
        return;

    for (idx = 0; idx < AVAHI_LEGACY_UNICAST_REFLECT_SLOTS_MAX; idx ++)
        if (s->legacy_unicast_reflect_slots[idx])
            deallocate_slot(s, s->legacy_unicast_reflect_slots[idx]);

    avahi_free(s->legacy_unicast_reflect_slots);
    s->legacy_unicast_reflect_slots = NULL;
}

static AvahiLegacyUnicastReflectSlot* find_slot(AvahiServer *s, uint16_t id) {
    unsigned idx;

    assert(s);

    if (!s->legacy_unicast_reflect_slots)
        return NULL;

    idx = id % AVAHI_LEGACY_UNICAST_REFLECT_SLOTS_MAX;

    if (!s->legacy_unicast_reflect_slots[idx] || s->legacy_unicast_reflect_slots[idx]->id != id)
        return NULL;

    return s->legacy_unicast_reflect_slots[idx];
}

static void legacy_unicast_reflect_slot_timeout(AvahiTimeEvent *e, void *userdata) {
    AvahiLegacyUnicastReflectSlot *slot = userdata;

    assert(e);
    assert(slot);
    assert(slot->time_event == e);

    deallocate_slot(slot->server, slot);
}

static void reflect_legacy_unicast_query_packet(AvahiServer *s, AvahiDnsPacket *p, AvahiInterface *i, const AvahiAddress *a, uint16_t port) {
    AvahiLegacyUnicastReflectSlot *slot;
    AvahiInterface *j;

    assert(s);
    assert(p);
    assert(i);
    assert(a);
    assert(port > 0);
    assert(i->protocol == a->proto);

    if (!s->config.enable_reflector)
        return;

    /* Reflecting legacy unicast queries is a little more complicated
       than reflecting normal queries, since we must route the
       responses back to the right client. Therefore we must store
       some information for finding the right client contact data for
       response packets. In contrast to normal queries legacy
       unicast query and response packets are reflected untouched and
       are not reassembled into larger packets */

    if (!(slot = allocate_slot(s))) {
        /* No slot available, we drop this legacy unicast query */
        avahi_log_warn("No slot available for legacy unicast reflection, dropping query packet.");
        return;
    }

    slot->original_id = avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_ID);
    slot->address = *a;
    slot->port = port;
    slot->interface = i->hardware->index;

    avahi_elapse_time(&slot->elapse_time, 2000, 0);
    slot->time_event = avahi_time_event_new(s->time_event_queue, &slot->elapse_time, legacy_unicast_reflect_slot_timeout, slot);

    /* Patch the packet with our new locally generatet id */
    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_ID, slot->id);

    for (j = s->monitor->interfaces; j; j = j->interface_next)
        if (j->announcing &&
            j != i &&
            (s->config.reflect_ipv || j->protocol == i->protocol)) {

            if (j->protocol == AVAHI_PROTO_INET && s->fd_legacy_unicast_ipv4 >= 0) {
                avahi_send_dns_packet_ipv4(s->fd_legacy_unicast_ipv4, j->hardware->index, p, NULL, NULL, 0);
            } else if (j->protocol == AVAHI_PROTO_INET6 && s->fd_legacy_unicast_ipv6 >= 0)
                avahi_send_dns_packet_ipv6(s->fd_legacy_unicast_ipv6, j->hardware->index, p, NULL, NULL, 0);
        }

    /* Reset the id */
    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_ID, slot->original_id);
}

static int originates_from_local_legacy_unicast_socket(AvahiServer *s, const AvahiAddress *address, uint16_t port) {
    assert(s);
    assert(address);
    assert(port > 0);

    if (!s->config.enable_reflector)
        return 0;

    if (!avahi_address_is_local(s->monitor, address))
        return 0;

    if (address->proto == AVAHI_PROTO_INET && s->fd_legacy_unicast_ipv4 >= 0) {
        struct sockaddr_in lsa;
        socklen_t l = sizeof(lsa);

        if (getsockname(s->fd_legacy_unicast_ipv4, (struct sockaddr*) &lsa, &l) != 0)
            avahi_log_warn("getsockname(): %s", strerror(errno));
        else
            return avahi_port_from_sockaddr((struct sockaddr*) &lsa) == port;

    }

    if (address->proto == AVAHI_PROTO_INET6 && s->fd_legacy_unicast_ipv6 >= 0) {
        struct sockaddr_in6 lsa;
        socklen_t l = sizeof(lsa);

        if (getsockname(s->fd_legacy_unicast_ipv6, (struct sockaddr*) &lsa, &l) != 0)
            avahi_log_warn("getsockname(): %s", strerror(errno));
        else
            return avahi_port_from_sockaddr((struct sockaddr*) &lsa) == port;
    }

    return 0;
}

static int is_mdns_mcast_address(const AvahiAddress *a) {
    AvahiAddress b;
    assert(a);

    avahi_address_parse(a->proto == AVAHI_PROTO_INET ? AVAHI_IPV4_MCAST_GROUP : AVAHI_IPV6_MCAST_GROUP, a->proto, &b);
    return avahi_address_cmp(a, &b) == 0;
}

static int originates_from_local_iface(AvahiServer *s, AvahiIfIndex iface, const AvahiAddress *a, uint16_t port) {
    assert(s);
    assert(iface != AVAHI_IF_UNSPEC);
    assert(a);

    /* If it isn't the MDNS port it can't be generated by us */
    if (port != AVAHI_MDNS_PORT)
        return 0;

    return avahi_interface_has_address(s->monitor, iface, a);
}

static void dispatch_packet(AvahiServer *s, AvahiDnsPacket *p, const AvahiAddress *src_address, uint16_t port, const AvahiAddress *dst_address, AvahiIfIndex iface, int ttl) {
    AvahiInterface *i;
    int from_local_iface = 0;

    assert(s);
    assert(p);
    assert(src_address);
    assert(dst_address);
    assert(iface > 0);
    assert(src_address->proto == dst_address->proto);

    if (!(i = avahi_interface_monitor_get_interface(s->monitor, iface, src_address->proto)) ||
        !i->announcing) {
        avahi_log_debug("Received packet from invalid interface.");
        return;
    }

    if (port <= 0) {
        /* This fixes RHBZ #475394 */
        avahi_log_debug("Received packet from invalid source port %u.", (unsigned) port);
        return;
    }

    if (avahi_address_is_ipv4_in_ipv6(src_address))
        /* This is an IPv4 address encapsulated in IPv6, so let's ignore it. */
        return;

    if (originates_from_local_legacy_unicast_socket(s, src_address, port))
        /* This originates from our local reflector, so let's ignore it */
        return;

    /* We don't want to reflect local traffic, so we check if this packet is generated locally. */
    if (s->config.enable_reflector)
        from_local_iface = originates_from_local_iface(s, iface, src_address, port);

    if (avahi_dns_packet_check_valid_multicast(p) < 0) {
        avahi_log_debug("Received invalid packet.");
        return;
    }

    if (avahi_dns_packet_is_query(p)) {
        int legacy_unicast = 0;
        char t[AVAHI_ADDRESS_STR_MAX];

        /* For queries EDNS0 might allow ARCOUNT != 0. We ignore the
         * AR section completely here, so far. Until the day we add
         * EDNS0 support. */

        if (port != AVAHI_MDNS_PORT) {
            /* Legacy Unicast */

            if ((avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_ANCOUNT) != 0 ||
                 avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_NSCOUNT) != 0)) {
                avahi_log_debug("Invalid legacy unicast query packet.");
                return;
            }

            legacy_unicast = 1;
        }

        if (!is_mdns_mcast_address(dst_address) &&
            !avahi_interface_address_on_link(i, src_address)) {

            avahi_log_debug("Received non-local unicast query from host %s on interface '%s.%i'.", avahi_address_snprint(t, sizeof(t), src_address), i->hardware->name, i->protocol);
            return;
        }

        if (legacy_unicast)
            reflect_legacy_unicast_query_packet(s, p, i, src_address, port);

        handle_query_packet(s, p, i, src_address, port, legacy_unicast, from_local_iface);

    } else {
        char t[AVAHI_ADDRESS_STR_MAX];

        if (port != AVAHI_MDNS_PORT) {
            avahi_log_debug("Received response from host %s with invalid source port %u on interface '%s.%i'", avahi_address_snprint(t, sizeof(t), src_address), port, i->hardware->name, i->protocol);
            return;
        }

        if (ttl != 255 && s->config.check_response_ttl) {
            avahi_log_debug("Received response from host %s with invalid TTL %u on interface '%s.%i'.", avahi_address_snprint(t, sizeof(t), src_address), ttl, i->hardware->name, i->protocol);
            return;
        }

        if (!is_mdns_mcast_address(dst_address) &&
            !avahi_interface_address_on_link(i, src_address)) {

            avahi_log_debug("Received non-local response from host %s on interface '%s.%i'.", avahi_address_snprint(t, sizeof(t), src_address), i->hardware->name, i->protocol);
            return;
        }

        if (avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_QDCOUNT) != 0 ||
            avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_ANCOUNT) == 0 ||
            avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_NSCOUNT) != 0) {

            avahi_log_debug("Invalid response packet from host %s.", avahi_address_snprint(t, sizeof(t), src_address));
            return;
        }

        handle_response_packet(s, p, i, src_address, from_local_iface);
    }
}

static void dispatch_legacy_unicast_packet(AvahiServer *s, AvahiDnsPacket *p) {
    AvahiInterface *j;
    AvahiLegacyUnicastReflectSlot *slot;

    assert(s);
    assert(p);

    if (avahi_dns_packet_check_valid(p) < 0 || avahi_dns_packet_is_query(p)) {
        avahi_log_debug("Received invalid packet.");
        return;
    }

    if (!(slot = find_slot(s, avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_ID)))) {
        avahi_log_debug("Received legacy unicast response with unknown id");
        return;
    }

    if (!(j = avahi_interface_monitor_get_interface(s->monitor, slot->interface, slot->address.proto)) ||
        !j->announcing)
        return;

    /* Patch the original ID into this response */
    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_ID, slot->original_id);

    /* Forward the response to the correct client */
    avahi_interface_send_packet_unicast(j, p, &slot->address, slot->port);

    /* Undo changes to packet */
    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_ID, slot->id);
}

static void mcast_socket_event(AvahiWatch *w, int fd, AvahiWatchEvent events, void *userdata) {
    AvahiServer *s = userdata;
    AvahiAddress dest, src;
    AvahiDnsPacket *p = NULL;
    AvahiIfIndex iface;
    uint16_t port;
    uint8_t ttl;

    assert(w);
    assert(fd >= 0);
    assert(events & AVAHI_WATCH_IN);

    if (fd == s->fd_ipv4) {
        dest.proto = src.proto = AVAHI_PROTO_INET;
        p = avahi_recv_dns_packet_ipv4(s->fd_ipv4, &src.data.ipv4, &port, &dest.data.ipv4, &iface, &ttl);
    } else {
        assert(fd == s->fd_ipv6);
        dest.proto = src.proto = AVAHI_PROTO_INET6;
        p = avahi_recv_dns_packet_ipv6(s->fd_ipv6, &src.data.ipv6, &port, &dest.data.ipv6, &iface, &ttl);
    }

    if (p) {
        if (iface == AVAHI_IF_UNSPEC)
            iface = avahi_find_interface_for_address(s->monitor, &dest);

        if (iface != AVAHI_IF_UNSPEC)
            dispatch_packet(s, p, &src, port, &dest, iface, ttl);
        else
            avahi_log_error("Incoming packet received on address that isn't local.");

        avahi_dns_packet_free(p);

        avahi_cleanup_dead_entries(s);
    }
}

static void legacy_unicast_socket_event(AvahiWatch *w, int fd, AvahiWatchEvent events, void *userdata) {
    AvahiServer *s = userdata;
    AvahiDnsPacket *p = NULL;

    assert(w);
    assert(fd >= 0);
    assert(events & AVAHI_WATCH_IN);

    if (fd == s->fd_legacy_unicast_ipv4)
        p = avahi_recv_dns_packet_ipv4(s->fd_legacy_unicast_ipv4, NULL, NULL, NULL, NULL, NULL);
    else {
        assert(fd == s->fd_legacy_unicast_ipv6);
        p = avahi_recv_dns_packet_ipv6(s->fd_legacy_unicast_ipv6, NULL, NULL, NULL, NULL, NULL);
    }

    if (p) {
        dispatch_legacy_unicast_packet(s, p);
        avahi_dns_packet_free(p);

        avahi_cleanup_dead_entries(s);
    }
}

static void server_set_state(AvahiServer *s, AvahiServerState state) {
    assert(s);

    if (s->state == state)
        return;

    s->state = state;

    avahi_interface_monitor_update_rrs(s->monitor, 0);

    if (s->callback)
        s->callback(s, state, s->userdata);
}

static void withdraw_host_rrs(AvahiServer *s) {
    assert(s);

    if (s->hinfo_entry_group)
        avahi_s_entry_group_reset(s->hinfo_entry_group);

    if (s->browse_domain_entry_group)
        avahi_s_entry_group_reset(s->browse_domain_entry_group);

    avahi_interface_monitor_update_rrs(s->monitor, 1);
    s->n_host_rr_pending = 0;
}

void avahi_server_decrease_host_rr_pending(AvahiServer *s) {
    assert(s);

    assert(s->n_host_rr_pending > 0);

    if (--s->n_host_rr_pending == 0)
        server_set_state(s, AVAHI_SERVER_RUNNING);
}

void avahi_host_rr_entry_group_callback(AvahiServer *s, AvahiSEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) {
    assert(s);
    assert(g);

    if (state == AVAHI_ENTRY_GROUP_REGISTERING &&
        s->state == AVAHI_SERVER_REGISTERING)
        s->n_host_rr_pending ++;

    else if (state == AVAHI_ENTRY_GROUP_COLLISION &&
        (s->state == AVAHI_SERVER_REGISTERING || s->state == AVAHI_SERVER_RUNNING)) {
        withdraw_host_rrs(s);
        server_set_state(s, AVAHI_SERVER_COLLISION);

    } else if (state == AVAHI_ENTRY_GROUP_ESTABLISHED &&
               s->state == AVAHI_SERVER_REGISTERING)
        avahi_server_decrease_host_rr_pending(s);
}

static void register_hinfo(AvahiServer *s) {
    struct utsname utsname;
    AvahiRecord *r;

    assert(s);

    if (!s->config.publish_hinfo)
        return;

    if (s->hinfo_entry_group)
        assert(avahi_s_entry_group_is_empty(s->hinfo_entry_group));
    else
        s->hinfo_entry_group = avahi_s_entry_group_new(s, avahi_host_rr_entry_group_callback, NULL);

    if (!s->hinfo_entry_group) {
        avahi_log_warn("Failed to create HINFO entry group: %s", avahi_strerror(s->error));
        return;
    }

    /* Fill in HINFO rr */
    if ((r = avahi_record_new_full(s->host_name_fqdn, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_HINFO, AVAHI_DEFAULT_TTL_HOST_NAME))) {

        if (uname(&utsname) < 0)
            avahi_log_warn("uname() failed: %s\n", avahi_strerror(errno));
        else {

            r->data.hinfo.cpu = avahi_strdup(avahi_strup(utsname.machine));
            r->data.hinfo.os = avahi_strdup(avahi_strup(utsname.sysname));

            avahi_log_info("Registering HINFO record with values '%s'/'%s'.", r->data.hinfo.cpu, r->data.hinfo.os);

            if (avahi_server_add(s, s->hinfo_entry_group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, AVAHI_PUBLISH_UNIQUE, r) < 0) {
                avahi_log_warn("Failed to add HINFO RR: %s", avahi_strerror(s->error));
                avahi_record_unref(r);
                return;
            }
        }

        avahi_record_unref(r);
    }

    if (avahi_s_entry_group_commit(s->hinfo_entry_group) < 0)
        avahi_log_warn("Failed to commit HINFO entry group: %s", avahi_strerror(s->error));

}

static void register_localhost(AvahiServer *s) {
    AvahiAddress a;
    assert(s);

    /* Add localhost entries */
    avahi_address_parse("127.0.0.1", AVAHI_PROTO_INET, &a);
    avahi_server_add_address(s, NULL, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, AVAHI_PUBLISH_NO_PROBE|AVAHI_PUBLISH_NO_ANNOUNCE, "localhost", &a);

    avahi_address_parse("::1", AVAHI_PROTO_INET6, &a);
    avahi_server_add_address(s, NULL, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, AVAHI_PUBLISH_NO_PROBE|AVAHI_PUBLISH_NO_ANNOUNCE, "ip6-localhost", &a);
}

static void register_browse_domain(AvahiServer *s) {
    assert(s);

    if (!s->config.publish_domain)
        return;

    if (avahi_domain_equal(s->domain_name, "local"))
        return;

    if (s->browse_domain_entry_group)
        assert(avahi_s_entry_group_is_empty(s->browse_domain_entry_group));
    else
        s->browse_domain_entry_group = avahi_s_entry_group_new(s, NULL, NULL);

    if (!s->browse_domain_entry_group) {
        avahi_log_warn("Failed to create browse domain entry group: %s", avahi_strerror(s->error));
        return;
    }

    if (avahi_server_add_ptr(s, s->browse_domain_entry_group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, AVAHI_DEFAULT_TTL, "b._dns-sd._udp.local", s->domain_name) < 0) {
        avahi_log_warn("Failed to add browse domain RR: %s", avahi_strerror(s->error));
        return;
    }

    if (avahi_s_entry_group_commit(s->browse_domain_entry_group) < 0)
        avahi_log_warn("Failed to commit browse domain entry group: %s", avahi_strerror(s->error));
}

static void register_stuff(AvahiServer *s) {
    assert(s);

    server_set_state(s, AVAHI_SERVER_REGISTERING);
    s->n_host_rr_pending ++; /** Make sure that the state isn't changed tp AVAHI_SERVER_RUNNING too early */

    register_hinfo(s);
    register_browse_domain(s);
    avahi_interface_monitor_update_rrs(s->monitor, 0);

    assert(s->n_host_rr_pending > 0);
    s->n_host_rr_pending --;

    if (s->n_host_rr_pending == 0)
        server_set_state(s, AVAHI_SERVER_RUNNING);
}

static void update_fqdn(AvahiServer *s) {
    char *n;

    assert(s);
    assert(s->host_name);
    assert(s->domain_name);

    if (!(n = avahi_strdup_printf("%s.%s", s->host_name, s->domain_name)))
        return; /* OOM */

    avahi_free(s->host_name_fqdn);
    s->host_name_fqdn = n;
}

int avahi_server_set_host_name(AvahiServer *s, const char *host_name) {
    char *hn = NULL;
    assert(s);

    AVAHI_CHECK_VALIDITY(s, !host_name || avahi_is_valid_host_name(host_name), AVAHI_ERR_INVALID_HOST_NAME);

    if (!host_name)
        hn = avahi_get_host_name_strdup();
    else
        hn = avahi_normalize_name_strdup(host_name);

    hn[strcspn(hn, ".")] = 0;

    if (avahi_domain_equal(s->host_name, hn) && s->state != AVAHI_SERVER_COLLISION) {
        avahi_free(hn);
        return avahi_server_set_errno(s, AVAHI_ERR_NO_CHANGE);
    }

    withdraw_host_rrs(s);

    avahi_free(s->host_name);
    s->host_name = hn;

    update_fqdn(s);

    register_stuff(s);
    return AVAHI_OK;
}

int avahi_server_set_domain_name(AvahiServer *s, const char *domain_name) {
    char *dn = NULL;
    assert(s);

    AVAHI_CHECK_VALIDITY(s, !domain_name || avahi_is_valid_domain_name(domain_name), AVAHI_ERR_INVALID_DOMAIN_NAME);

    if (!domain_name)
        dn = avahi_strdup("local");
    else
        dn = avahi_normalize_name_strdup(domain_name);

    if (avahi_domain_equal(s->domain_name, domain_name)) {
        avahi_free(dn);
        return avahi_server_set_errno(s, AVAHI_ERR_NO_CHANGE);
    }

    withdraw_host_rrs(s);

    avahi_free(s->domain_name);
    s->domain_name = dn;
    update_fqdn(s);

    register_stuff(s);

    avahi_free(dn);
    return AVAHI_OK;
}

static int valid_server_config(const AvahiServerConfig *sc) {
    AvahiStringList *l;

    assert(sc);

    if (sc->n_wide_area_servers > AVAHI_WIDE_AREA_SERVERS_MAX)
        return AVAHI_ERR_INVALID_CONFIG;

    if (sc->host_name && !avahi_is_valid_host_name(sc->host_name))
        return AVAHI_ERR_INVALID_HOST_NAME;

    if (sc->domain_name && !avahi_is_valid_domain_name(sc->domain_name))
        return AVAHI_ERR_INVALID_DOMAIN_NAME;

    for (l = sc->browse_domains; l; l = l->next)
        if (!avahi_is_valid_domain_name((char*) l->text))
            return AVAHI_ERR_INVALID_DOMAIN_NAME;

    return AVAHI_OK;
}

static int setup_sockets(AvahiServer *s) {
    assert(s);

    s->fd_ipv4 = s->config.use_ipv4 ? avahi_open_socket_ipv4(s->config.disallow_other_stacks) : -1;
    s->fd_ipv6 = s->config.use_ipv6 ? avahi_open_socket_ipv6(s->config.disallow_other_stacks) : -1;

    if (s->fd_ipv6 < 0 && s->fd_ipv4 < 0)
        return AVAHI_ERR_NO_NETWORK;

    if (s->fd_ipv4 < 0 && s->config.use_ipv4)
        avahi_log_notice("Failed to create IPv4 socket, proceeding in IPv6 only mode");
    else if (s->fd_ipv6 < 0 && s->config.use_ipv6)
        avahi_log_notice("Failed to create IPv6 socket, proceeding in IPv4 only mode");

    s->fd_legacy_unicast_ipv4 = s->fd_ipv4 >= 0 && s->config.enable_reflector ? avahi_open_unicast_socket_ipv4() : -1;
    s->fd_legacy_unicast_ipv6 = s->fd_ipv6 >= 0 && s->config.enable_reflector ? avahi_open_unicast_socket_ipv6() : -1;

    s->watch_ipv4 =
        s->watch_ipv6 =
        s->watch_legacy_unicast_ipv4 =
        s->watch_legacy_unicast_ipv6 = NULL;

    if (s->fd_ipv4 >= 0)
        s->watch_ipv4 = s->poll_api->watch_new(s->poll_api, s->fd_ipv4, AVAHI_WATCH_IN, mcast_socket_event, s);
    if (s->fd_ipv6 >= 0)
        s->watch_ipv6 = s->poll_api->watch_new(s->poll_api, s->fd_ipv6, AVAHI_WATCH_IN, mcast_socket_event, s);

    if (s->fd_legacy_unicast_ipv4 >= 0)
        s->watch_legacy_unicast_ipv4 = s->poll_api->watch_new(s->poll_api, s->fd_legacy_unicast_ipv4, AVAHI_WATCH_IN, legacy_unicast_socket_event, s);
    if (s->fd_legacy_unicast_ipv6 >= 0)
        s->watch_legacy_unicast_ipv6 = s->poll_api->watch_new(s->poll_api, s->fd_legacy_unicast_ipv6, AVAHI_WATCH_IN, legacy_unicast_socket_event, s);

    return 0;
}

AvahiServer *avahi_server_new(const AvahiPoll *poll_api, const AvahiServerConfig *sc, AvahiServerCallback callback, void* userdata, int *error) {
    AvahiServer *s;
    int e;

    if (sc && (e = valid_server_config(sc)) < 0) {
        if (error)
            *error = e;
        return NULL;
    }

    if (!(s = avahi_new(AvahiServer, 1))) {
        if (error)
            *error = AVAHI_ERR_NO_MEMORY;

        return NULL;
    }

    s->poll_api = poll_api;

    if (sc)
        avahi_server_config_copy(&s->config, sc);
    else
        avahi_server_config_init(&s->config);

    if ((e = setup_sockets(s)) < 0) {
        if (error)
            *error = e;

        avahi_server_config_free(&s->config);
        avahi_free(s);

        return NULL;
    }

    s->n_host_rr_pending = 0;
    s->need_entry_cleanup = 0;
    s->need_group_cleanup = 0;
    s->need_browser_cleanup = 0;
    s->cleanup_time_event = NULL;
    s->hinfo_entry_group = NULL;
    s->browse_domain_entry_group = NULL;
    s->error = AVAHI_OK;
    s->state = AVAHI_SERVER_INVALID;

    s->callback = callback;
    s->userdata = userdata;

    s->time_event_queue = avahi_time_event_queue_new(poll_api);

    s->entries_by_key = avahi_hashmap_new((AvahiHashFunc) avahi_key_hash, (AvahiEqualFunc) avahi_key_equal, NULL, NULL);
    AVAHI_LLIST_HEAD_INIT(AvahiEntry, s->entries);
    AVAHI_LLIST_HEAD_INIT(AvahiGroup, s->groups);

    s->record_browser_hashmap = avahi_hashmap_new((AvahiHashFunc) avahi_key_hash, (AvahiEqualFunc) avahi_key_equal, NULL, NULL);
    AVAHI_LLIST_HEAD_INIT(AvahiSRecordBrowser, s->record_browsers);
    AVAHI_LLIST_HEAD_INIT(AvahiSHostNameResolver, s->host_name_resolvers);
    AVAHI_LLIST_HEAD_INIT(AvahiSAddressResolver, s->address_resolvers);
    AVAHI_LLIST_HEAD_INIT(AvahiSDomainBrowser, s->domain_browsers);
    AVAHI_LLIST_HEAD_INIT(AvahiSServiceTypeBrowser, s->service_type_browsers);
    AVAHI_LLIST_HEAD_INIT(AvahiSServiceBrowser, s->service_browsers);
    AVAHI_LLIST_HEAD_INIT(AvahiSServiceResolver, s->service_resolvers);
    AVAHI_LLIST_HEAD_INIT(AvahiSDNSServerBrowser, s->dns_server_browsers);

    s->legacy_unicast_reflect_slots = NULL;
    s->legacy_unicast_reflect_id = 0;

    s->record_list = avahi_record_list_new();

    /* Get host name */
    s->host_name = s->config.host_name ? avahi_normalize_name_strdup(s->config.host_name) : avahi_get_host_name_strdup();
    s->host_name[strcspn(s->host_name, ".")] = 0;
    s->domain_name = s->config.domain_name ? avahi_normalize_name_strdup(s->config.domain_name) : avahi_strdup("local");
    s->host_name_fqdn = NULL;
    update_fqdn(s);

    do {
        s->local_service_cookie = (uint32_t) rand() * (uint32_t) rand();
    } while (s->local_service_cookie == AVAHI_SERVICE_COOKIE_INVALID);

    if (s->config.enable_wide_area) {
        s->wide_area_lookup_engine = avahi_wide_area_engine_new(s);
        avahi_wide_area_set_servers(s->wide_area_lookup_engine, s->config.wide_area_servers, s->config.n_wide_area_servers);
    } else
        s->wide_area_lookup_engine = NULL;

    s->multicast_lookup_engine = avahi_multicast_lookup_engine_new(s);

    s->monitor = avahi_interface_monitor_new(s);
    avahi_interface_monitor_sync(s->monitor);

    register_localhost(s);
    register_stuff(s);

    return s;
}

void avahi_server_free(AvahiServer* s) {
    assert(s);

    /* Remove all browsers */

    while (s->dns_server_browsers)
        avahi_s_dns_server_browser_free(s->dns_server_browsers);
    while (s->host_name_resolvers)
        avahi_s_host_name_resolver_free(s->host_name_resolvers);
    while (s->address_resolvers)
        avahi_s_address_resolver_free(s->address_resolvers);
    while (s->domain_browsers)
        avahi_s_domain_browser_free(s->domain_browsers);
    while (s->service_type_browsers)
        avahi_s_service_type_browser_free(s->service_type_browsers);
    while (s->service_browsers)
        avahi_s_service_browser_free(s->service_browsers);
    while (s->service_resolvers)
        avahi_s_service_resolver_free(s->service_resolvers);
    while (s->record_browsers)
        avahi_s_record_browser_destroy(s->record_browsers);

    /* Remove all locally rgeistered stuff */

    while(s->entries)
        avahi_entry_free(s, s->entries);

    avahi_interface_monitor_free(s->monitor);

    while (s->groups)
        avahi_entry_group_free(s, s->groups);

    free_slots(s);

    avahi_hashmap_free(s->entries_by_key);
    avahi_record_list_free(s->record_list);
    avahi_hashmap_free(s->record_browser_hashmap);

    if (s->wide_area_lookup_engine)
        avahi_wide_area_engine_free(s->wide_area_lookup_engine);
    avahi_multicast_lookup_engine_free(s->multicast_lookup_engine);

    if (s->cleanup_time_event)
        avahi_time_event_free(s->cleanup_time_event);

    avahi_time_event_queue_free(s->time_event_queue);

    /* Free watches */

    if (s->watch_ipv4)
        s->poll_api->watch_free(s->watch_ipv4);
    if (s->watch_ipv6)
        s->poll_api->watch_free(s->watch_ipv6);

    if (s->watch_legacy_unicast_ipv4)
        s->poll_api->watch_free(s->watch_legacy_unicast_ipv4);
    if (s->watch_legacy_unicast_ipv6)
        s->poll_api->watch_free(s->watch_legacy_unicast_ipv6);

    /* Free sockets */

    if (s->fd_ipv4 >= 0)
        close(s->fd_ipv4);
    if (s->fd_ipv6 >= 0)
        close(s->fd_ipv6);

    if (s->fd_legacy_unicast_ipv4 >= 0)
        close(s->fd_legacy_unicast_ipv4);
    if (s->fd_legacy_unicast_ipv6 >= 0)
        close(s->fd_legacy_unicast_ipv6);

    /* Free other stuff */

    avahi_free(s->host_name);
    avahi_free(s->domain_name);
    avahi_free(s->host_name_fqdn);

    avahi_server_config_free(&s->config);

    avahi_free(s);
}

const char* avahi_server_get_domain_name(AvahiServer *s) {
    assert(s);

    return s->domain_name;
}

const char* avahi_server_get_host_name(AvahiServer *s) {
    assert(s);

    return s->host_name;
}

const char* avahi_server_get_host_name_fqdn(AvahiServer *s) {
    assert(s);

    return s->host_name_fqdn;
}

void* avahi_server_get_data(AvahiServer *s) {
    assert(s);

    return s->userdata;
}

void avahi_server_set_data(AvahiServer *s, void* userdata) {
    assert(s);

    s->userdata = userdata;
}

AvahiServerState avahi_server_get_state(AvahiServer *s) {
    assert(s);

    return s->state;
}

AvahiServerConfig* avahi_server_config_init(AvahiServerConfig *c) {
    assert(c);

    memset(c, 0, sizeof(AvahiServerConfig));
    c->use_ipv6 = 1;
    c->use_ipv4 = 1;
    c->allow_interfaces = NULL;
    c->deny_interfaces = NULL;
    c->host_name = NULL;
    c->domain_name = NULL;
    c->check_response_ttl = 0;
    c->publish_hinfo = 0;
    c->publish_addresses = 1;
    c->publish_workstation = 0;
    c->publish_domain = 1;
    c->use_iff_running = 0;
    c->enable_reflector = 0;
    c->reflect_ipv = 0;
    c->reflect_filters = NULL;
    c->add_service_cookie = 0;
    c->enable_wide_area = 0;
    c->n_wide_area_servers = 0;
    c->disallow_other_stacks = 0;
    c->browse_domains = NULL;
    c->disable_publishing = 0;
    c->allow_point_to_point = 0;
    c->publish_aaaa_on_ipv4 = 1;
    c->publish_a_on_ipv6 = 0;
    c->n_cache_entries_max = AVAHI_DEFAULT_CACHE_ENTRIES_MAX;
    c->ratelimit_interval = 0;
    c->ratelimit_burst = 0;

    return c;
}

void avahi_server_config_free(AvahiServerConfig *c) {
    assert(c);

    avahi_free(c->host_name);
    avahi_free(c->domain_name);
    avahi_string_list_free(c->browse_domains);
    avahi_string_list_free(c->reflect_filters);
    avahi_string_list_free(c->allow_interfaces);
    avahi_string_list_free(c->deny_interfaces);
}

AvahiServerConfig* avahi_server_config_copy(AvahiServerConfig *ret, const AvahiServerConfig *c) {
    char *d = NULL, *h = NULL;
    AvahiStringList *browse = NULL, *allow = NULL, *deny = NULL, *reflect = NULL ;
    assert(ret);
    assert(c);

    if (c->host_name)
        if (!(h = avahi_strdup(c->host_name)))
            return NULL;

    if (c->domain_name)
        if (!(d = avahi_strdup(c->domain_name))) {
            avahi_free(h);
            return NULL;
        }

    if (!(browse = avahi_string_list_copy(c->browse_domains)) && c->browse_domains) {
        avahi_free(h);
        avahi_free(d);
        return NULL;
    }

    if (!(allow = avahi_string_list_copy(c->allow_interfaces)) && c->allow_interfaces) {
        avahi_string_list_free(browse);
        avahi_free(h);
        avahi_free(d);
        return NULL;
    }

    if (!(deny = avahi_string_list_copy(c->deny_interfaces)) && c->deny_interfaces) {
        avahi_string_list_free(allow);
        avahi_string_list_free(browse);
        avahi_free(h);
        avahi_free(d);
        return NULL;
    }

   if (!(reflect = avahi_string_list_copy(c->reflect_filters)) && c->reflect_filters) {
        avahi_string_list_free(allow);
        avahi_string_list_free(browse);
        avahi_string_list_free(deny);
        avahi_free(h);
        avahi_free(d);
        return NULL;
    }

    *ret = *c;
    ret->host_name = h;
    ret->domain_name = d;
    ret->browse_domains = browse;
    ret->allow_interfaces = allow;
    ret->deny_interfaces = deny;
    ret->reflect_filters = reflect;

    return ret;
}

int avahi_server_errno(AvahiServer *s) {
    assert(s);

    return s->error;
}

/* Just for internal use */
int avahi_server_set_errno(AvahiServer *s, int error) {
    assert(s);

    return s->error = error;
}

uint32_t avahi_server_get_local_service_cookie(AvahiServer *s) {
    assert(s);

    return s->local_service_cookie;
}

static AvahiEntry *find_entry(AvahiServer *s, AvahiIfIndex interface, AvahiProtocol protocol, AvahiKey *key) {
    AvahiEntry *e;

    assert(s);
    assert(key);

    for (e = avahi_hashmap_lookup(s->entries_by_key, key); e; e = e->by_key_next)

        if ((e->interface == interface || e->interface <= 0 || interface <= 0) &&
            (e->protocol == protocol || e->protocol == AVAHI_PROTO_UNSPEC || protocol == AVAHI_PROTO_UNSPEC) &&
            (!e->group || e->group->state == AVAHI_ENTRY_GROUP_ESTABLISHED || e->group->state == AVAHI_ENTRY_GROUP_REGISTERING))

            return e;

    return NULL;
}

int avahi_server_get_group_of_service(AvahiServer *s, AvahiIfIndex interface, AvahiProtocol protocol, const char *name, const char *type, const char *domain, AvahiSEntryGroup** ret_group) {
    AvahiKey *key = NULL;
    AvahiEntry *e;
    int ret;
    char n[AVAHI_DOMAIN_NAME_MAX];

    assert(s);
    assert(name);
    assert(type);
    assert(ret_group);

    AVAHI_CHECK_VALIDITY(s, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY(s, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY(s, avahi_is_valid_service_name(name), AVAHI_ERR_INVALID_SERVICE_NAME);
    AVAHI_CHECK_VALIDITY(s, avahi_is_valid_service_type_strict(type), AVAHI_ERR_INVALID_SERVICE_TYPE);
    AVAHI_CHECK_VALIDITY(s, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);

    if ((ret = avahi_service_name_join(n, sizeof(n), name, type, domain) < 0))
        return avahi_server_set_errno(s, ret);

    if (!(key = avahi_key_new(n, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_SRV)))
        return avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);

    e = find_entry(s, interface, protocol, key);
    avahi_key_unref(key);

    if (e) {
        *ret_group = e->group;
        return AVAHI_OK;
    }

    return avahi_server_set_errno(s, AVAHI_ERR_NOT_FOUND);
}

int avahi_server_is_service_local(AvahiServer *s, AvahiIfIndex interface, AvahiProtocol protocol, const char *name) {
    AvahiKey *key = NULL;
    AvahiEntry *e;

    assert(s);
    assert(name);

    if (!s->host_name_fqdn)
        return 0;

    if (!(key = avahi_key_new(name, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_SRV)))
        return 0;

    e = find_entry(s, interface, protocol, key);
    avahi_key_unref(key);

    if (!e)
        return 0;

    return avahi_domain_equal(s->host_name_fqdn, e->record->data.srv.name);
}

int avahi_server_is_record_local(AvahiServer *s, AvahiIfIndex interface, AvahiProtocol protocol, AvahiRecord *record) {
    AvahiEntry *e;

    assert(s);
    assert(record);

    for (e = avahi_hashmap_lookup(s->entries_by_key, record->key); e; e = e->by_key_next)

        if ((e->interface == interface || e->interface <= 0 || interface <= 0) &&
            (e->protocol == protocol || e->protocol == AVAHI_PROTO_UNSPEC || protocol == AVAHI_PROTO_UNSPEC) &&
            (!e->group || e->group->state == AVAHI_ENTRY_GROUP_ESTABLISHED || e->group->state == AVAHI_ENTRY_GROUP_REGISTERING) &&
            avahi_record_equal_no_ttl(record, e->record))
            return 1;

    return 0;
}

/** Set the wide area DNS servers */
int avahi_server_set_wide_area_servers(AvahiServer *s, const AvahiAddress *a, unsigned n) {
    assert(s);

    if (!s->wide_area_lookup_engine)
        return avahi_server_set_errno(s, AVAHI_ERR_INVALID_CONFIG);

    avahi_wide_area_set_servers(s->wide_area_lookup_engine, a, n);
    return AVAHI_OK;
}

const AvahiServerConfig* avahi_server_get_config(AvahiServer *s) {
    assert(s);

    return &s->config;
}

/** Set the browsing domains */
int avahi_server_set_browse_domains(AvahiServer *s, AvahiStringList *domains) {
    AvahiStringList *l;

    assert(s);

    for (l = domains; l; l = l->next)
        if (!avahi_is_valid_domain_name((char*) l->text))
            return avahi_server_set_errno(s, AVAHI_ERR_INVALID_DOMAIN_NAME);

    avahi_string_list_free(s->config.browse_domains);
    s->config.browse_domains = avahi_string_list_copy(domains);

    return AVAHI_OK;
}
