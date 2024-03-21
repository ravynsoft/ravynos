#ifndef foointernalhfoo
#define foointernalhfoo

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

/** A locally registered DNS resource record */
typedef struct AvahiEntry AvahiEntry;

#include <avahi-common/llist.h>
#include <avahi-common/watch.h>
#include <avahi-common/timeval.h>

#include "core.h"
#include "iface.h"
#include "prioq.h"
#include "timeeventq.h"
#include "announce.h"
#include "browse.h"
#include "dns.h"
#include "rrlist.h"
#include "hashmap.h"
#include "wide-area.h"
#include "multicast-lookup.h"
#include "dns-srv-rr.h"

#define AVAHI_LEGACY_UNICAST_REFLECT_SLOTS_MAX 100

#define AVAHI_FLAGS_VALID(flags, max) (!((flags) & ~(max)))

#define AVAHI_RR_HOLDOFF_MSEC 1000
#define AVAHI_RR_HOLDOFF_MSEC_RATE_LIMIT 20000
#define AVAHI_RR_RATE_LIMIT_COUNT 15

typedef struct AvahiLegacyUnicastReflectSlot AvahiLegacyUnicastReflectSlot;

struct AvahiLegacyUnicastReflectSlot {
    AvahiServer *server;

    uint16_t id, original_id;
    AvahiAddress address;
    uint16_t port;
    int interface;
    struct timeval elapse_time;
    AvahiTimeEvent *time_event;
};

struct AvahiEntry {
    AvahiServer *server;
    AvahiSEntryGroup *group;

    int dead;

    AvahiPublishFlags flags;
    AvahiRecord *record;
    AvahiIfIndex interface;
    AvahiProtocol protocol;

    AVAHI_LLIST_FIELDS(AvahiEntry, entries);
    AVAHI_LLIST_FIELDS(AvahiEntry, by_key);
    AVAHI_LLIST_FIELDS(AvahiEntry, by_group);

    AVAHI_LLIST_HEAD(AvahiAnnouncer, announcers);
};

struct AvahiSEntryGroup {
    AvahiServer *server;
    int dead;

    AvahiEntryGroupState state;
    void* userdata;
    AvahiSEntryGroupCallback callback;

    unsigned n_probing;

    unsigned n_register_try;
    struct timeval register_time;
    AvahiTimeEvent *register_time_event;

    struct timeval established_at;

    AVAHI_LLIST_FIELDS(AvahiSEntryGroup, groups);
    AVAHI_LLIST_HEAD(AvahiEntry, entries);
};

struct AvahiServer {
    const AvahiPoll *poll_api;

    AvahiInterfaceMonitor *monitor;
    AvahiServerConfig config;

    AVAHI_LLIST_HEAD(AvahiEntry, entries);
    AvahiHashmap *entries_by_key;

    AVAHI_LLIST_HEAD(AvahiSEntryGroup, groups);

    AVAHI_LLIST_HEAD(AvahiSRecordBrowser, record_browsers);
    AvahiHashmap *record_browser_hashmap;
    AVAHI_LLIST_HEAD(AvahiSHostNameResolver, host_name_resolvers);
    AVAHI_LLIST_HEAD(AvahiSAddressResolver, address_resolvers);
    AVAHI_LLIST_HEAD(AvahiSDomainBrowser, domain_browsers);
    AVAHI_LLIST_HEAD(AvahiSServiceTypeBrowser, service_type_browsers);
    AVAHI_LLIST_HEAD(AvahiSServiceBrowser, service_browsers);
    AVAHI_LLIST_HEAD(AvahiSServiceResolver, service_resolvers);
    AVAHI_LLIST_HEAD(AvahiSDNSServerBrowser, dns_server_browsers);

    int need_entry_cleanup, need_group_cleanup, need_browser_cleanup;

    /* Used for scheduling RR cleanup */
    AvahiTimeEvent *cleanup_time_event;

    AvahiTimeEventQueue *time_event_queue;

    char *host_name, *host_name_fqdn, *domain_name;

    int fd_ipv4, fd_ipv6,
        /* The following two sockets two are used for reflection only */
        fd_legacy_unicast_ipv4, fd_legacy_unicast_ipv6;

    AvahiWatch *watch_ipv4, *watch_ipv6,
        *watch_legacy_unicast_ipv4, *watch_legacy_unicast_ipv6;

    AvahiServerState state;
    AvahiServerCallback callback;
    void* userdata;

    AvahiSEntryGroup *hinfo_entry_group;
    AvahiSEntryGroup *browse_domain_entry_group;
    unsigned n_host_rr_pending;

    /* Used for assembling responses */
    AvahiRecordList *record_list;

    /* Used for reflection of legacy unicast packets */
    AvahiLegacyUnicastReflectSlot **legacy_unicast_reflect_slots;
    uint16_t legacy_unicast_reflect_id;

    /* The last error code */
    int error;

    /* The local service cookie */
    uint32_t local_service_cookie;

    AvahiMulticastLookupEngine *multicast_lookup_engine;
    AvahiWideAreaLookupEngine *wide_area_lookup_engine;
};

void avahi_entry_free(AvahiServer*s, AvahiEntry *e);
void avahi_entry_group_free(AvahiServer *s, AvahiSEntryGroup *g);

void avahi_cleanup_dead_entries(AvahiServer *s);

void avahi_server_prepare_response(AvahiServer *s, AvahiInterface *i, AvahiEntry *e, int unicast_response, int auxiliary);
void avahi_server_prepare_matching_responses(AvahiServer *s, AvahiInterface *i, AvahiKey *k, int unicast_response);
void avahi_server_generate_response(AvahiServer *s, AvahiInterface *i, AvahiDnsPacket *p, const AvahiAddress *a, uint16_t port, int legacy_unicast, int is_probe);

void avahi_s_entry_group_change_state(AvahiSEntryGroup *g, AvahiEntryGroupState state);

int avahi_entry_is_commited(AvahiEntry *e);

void avahi_server_enumerate_aux_records(AvahiServer *s, AvahiInterface *i, AvahiRecord *r, void (*callback)(AvahiServer *s, AvahiRecord *r, int flush_cache, void* userdata), void* userdata);

void avahi_host_rr_entry_group_callback(AvahiServer *s, AvahiSEntryGroup *g, AvahiEntryGroupState state, void *userdata);

void avahi_server_decrease_host_rr_pending(AvahiServer *s);

int avahi_server_set_errno(AvahiServer *s, int error);

int avahi_server_is_service_local(AvahiServer *s, AvahiIfIndex interface, AvahiProtocol protocol, const char *name);
int avahi_server_is_record_local(AvahiServer *s, AvahiIfIndex interface, AvahiProtocol protocol, AvahiRecord *record);

int avahi_server_add_ptr(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    uint32_t ttl,
    const char *name,
    const char *dest);

#define AVAHI_CHECK_VALIDITY(server, expression, error) { \
        if (!(expression)) \
            return avahi_server_set_errno((server), (error)); \
}

#define AVAHI_CHECK_VALIDITY_RETURN_NULL(server, expression, error) { \
        if (!(expression)) { \
            avahi_server_set_errno((server), (error)); \
            return NULL; \
        } \
}

#define AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(server, expression, error) {\
    if (!(expression)) { \
        ret = avahi_server_set_errno((server), (error)); \
        goto fail; \
    } \
}

#define AVAHI_ASSERT_TRUE(expression) { \
    int __tmp = !!(expression); \
    assert(__tmp); \
}

#define AVAHI_ASSERT_SUCCESS(expression) { \
    int __tmp = (expression); \
    assert(__tmp == 0); \
}

#endif
