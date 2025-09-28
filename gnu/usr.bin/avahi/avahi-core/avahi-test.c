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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <avahi-common/malloc.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/alternative.h>
#include <avahi-common/timeval.h>

#include <avahi-core/core.h>
#include <avahi-core/log.h>
#include <avahi-core/publish.h>
#include <avahi-core/lookup.h>
#include <avahi-core/dns-srv-rr.h>

static AvahiSEntryGroup *group = NULL;
static AvahiServer *server = NULL;
static char *service_name = NULL;

static const AvahiPoll *poll_api;

static void quit_timeout_callback(AVAHI_GCC_UNUSED AvahiTimeout *timeout, void* userdata) {
    AvahiSimplePoll *simple_poll = userdata;

    avahi_simple_poll_quit(simple_poll);
}

static void dump_line(const char *text, AVAHI_GCC_UNUSED void* userdata) {
    printf("%s\n", text);
}

static void dump_timeout_callback(AvahiTimeout *timeout, void* userdata) {
    struct timeval tv;

    AvahiServer *avahi = userdata;
    avahi_server_dump(avahi, dump_line, NULL);

    avahi_elapse_time(&tv, 5000, 0);
    poll_api->timeout_update(timeout, &tv);
}

static const char *browser_event_to_string(AvahiBrowserEvent event) {
    switch (event) {
        case AVAHI_BROWSER_NEW : return "NEW";
        case AVAHI_BROWSER_REMOVE : return "REMOVE";
        case AVAHI_BROWSER_CACHE_EXHAUSTED : return "CACHE_EXHAUSTED";
        case AVAHI_BROWSER_ALL_FOR_NOW : return "ALL_FOR_NOW";
        case AVAHI_BROWSER_FAILURE : return "FAILURE";
    }

    abort();
}

static const char *resolver_event_to_string(AvahiResolverEvent event) {
    switch (event) {
        case AVAHI_RESOLVER_FOUND: return "FOUND";
        case AVAHI_RESOLVER_FAILURE: return "FAILURE";
    }
    abort();
}

static void record_browser_callback(
    AvahiSRecordBrowser *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    AvahiRecord *record,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {
    char *t;

    assert(r);

    if (record) {
        avahi_log_debug("RB: record [%s] on %i.%i is %s", t = avahi_record_to_string(record), interface, protocol, browser_event_to_string(event));
        avahi_free(t);
    } else
        avahi_log_debug("RB: [%s]", browser_event_to_string(event));

}

static void remove_entries(void);
static void create_entries(int new_name);

static void entry_group_callback(AVAHI_GCC_UNUSED AvahiServer *s, AVAHI_GCC_UNUSED AvahiSEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void* userdata) {
    avahi_log_debug("entry group state: %i", state);

    if (state == AVAHI_ENTRY_GROUP_COLLISION) {
        remove_entries();
        create_entries(1);
        avahi_log_debug("Service name conflict, retrying with <%s>", service_name);
    } else if (state == AVAHI_ENTRY_GROUP_ESTABLISHED) {
        avahi_log_debug("Service established under name <%s>", service_name);
    }
}

static void server_callback(AvahiServer *s, AvahiServerState state, AVAHI_GCC_UNUSED void* userdata) {

    server = s;
    avahi_log_debug("server state: %i", state);

    if (state == AVAHI_SERVER_RUNNING) {
        avahi_log_debug("Server startup complete. Host name is <%s>. Service cookie is %u", avahi_server_get_host_name_fqdn(s), avahi_server_get_local_service_cookie(s));
        create_entries(0);
    } else if (state == AVAHI_SERVER_COLLISION) {
        char *n;
        remove_entries();

        n = avahi_alternative_host_name(avahi_server_get_host_name(s));

        avahi_log_debug("Host name conflict, retrying with <%s>", n);
        avahi_server_set_host_name(s, n);
        avahi_free(n);
    }
}

static void remove_entries(void) {
    if (group)
        avahi_s_entry_group_reset(group);
}

static void create_entries(int new_name) {
    AvahiAddress a;
    AvahiRecord *r;

    remove_entries();

    if (!group)
        group = avahi_s_entry_group_new(server, entry_group_callback, NULL);

    assert(avahi_s_entry_group_is_empty(group));

    if (!service_name)
        service_name = avahi_strdup("Test Service");
    else if (new_name) {
        char *n = avahi_alternative_service_name(service_name);
        avahi_free(service_name);
        service_name = n;
    }

    if (avahi_server_add_service(server, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, service_name, "_http._tcp", NULL, NULL, 80, "foo", NULL) < 0) {
        avahi_log_error("Failed to add HTTP service");
        goto fail;
    }

    if (avahi_server_add_service(server, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, service_name, "_ftp._tcp", NULL, NULL, 21, "foo", NULL) < 0) {
        avahi_log_error("Failed to add FTP service");
        goto fail;
    }

    if (avahi_server_add_service(server, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0,service_name, "_webdav._tcp", NULL, NULL, 80, "foo", NULL) < 0) {
        avahi_log_error("Failed to add WEBDAV service");
        goto fail;
    }

    if (avahi_server_add_dns_server_address(server, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, NULL, AVAHI_DNS_SERVER_RESOLVE, avahi_address_parse("192.168.50.1", AVAHI_PROTO_UNSPEC, &a), 53) < 0) {
        avahi_log_error("Failed to add new DNS Server address");
        goto fail;
    }

    r = avahi_record_new_full("cname.local", AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_CNAME, AVAHI_DEFAULT_TTL);
    r->data.cname.name = avahi_strdup("cocaine.local");

    if (avahi_server_add(server, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, r) < 0) {
        avahi_record_unref(r);
        avahi_log_error("Failed to add CNAME record");
        goto fail;
    }
    avahi_record_unref(r);

    avahi_s_entry_group_commit(group);
    return;

fail:
    if (group)
        avahi_s_entry_group_free(group);

    group = NULL;
}

static void hnr_callback(
    AVAHI_GCC_UNUSED AvahiSHostNameResolver *r,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *hostname,
    const AvahiAddress *a,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {
    char t[AVAHI_ADDRESS_STR_MAX];

    if (a)
        avahi_address_snprint(t, sizeof(t), a);

    avahi_log_debug("HNR: (%i.%i) <%s> -> %s [%s]", iface, protocol, hostname, a ? t : "n/a", resolver_event_to_string(event));
}

static void ar_callback(
    AVAHI_GCC_UNUSED AvahiSAddressResolver *r,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const AvahiAddress *a,
    const char *hostname,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {
    char t[AVAHI_ADDRESS_STR_MAX];

    avahi_address_snprint(t, sizeof(t), a);

    avahi_log_debug("AR: (%i.%i) %s -> <%s> [%s]", iface, protocol, t, hostname ? hostname : "n/a", resolver_event_to_string(event));
}

static void db_callback(
    AVAHI_GCC_UNUSED AvahiSDomainBrowser *b,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {

    avahi_log_debug("DB: (%i.%i) <%s> [%s]", iface, protocol, domain ? domain : "NULL", browser_event_to_string(event));
}

static void stb_callback(
    AVAHI_GCC_UNUSED AvahiSServiceTypeBrowser *b,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *service_type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {

    avahi_log_debug("STB: (%i.%i) %s in <%s> [%s]", iface, protocol, service_type ? service_type : "NULL", domain ? domain : "NULL", browser_event_to_string(event));
}

static void sb_callback(
    AVAHI_GCC_UNUSED AvahiSServiceBrowser *b,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *service_type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {
    avahi_log_debug("SB: (%i.%i) <%s> as %s in <%s> [%s]", iface, protocol, name ? name : "NULL", service_type ? service_type : "NULL", domain ? domain : "NULL", browser_event_to_string(event));
}

static void sr_callback(
    AVAHI_GCC_UNUSED AvahiSServiceResolver *r,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char*service_type,
    const char*domain_name,
    const char*hostname,
    const AvahiAddress *a,
    uint16_t port,
    AvahiStringList *txt,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {

    if (event != AVAHI_RESOLVER_FOUND)
        avahi_log_debug("SR: (%i.%i) <%s> as %s in <%s> [%s]", iface, protocol, name, service_type, domain_name, resolver_event_to_string(event));
    else {
        char t[AVAHI_ADDRESS_STR_MAX], *s;

        avahi_address_snprint(t, sizeof(t), a);

        s = avahi_string_list_to_string(txt);
        avahi_log_debug("SR: (%i.%i) <%s> as %s in <%s>: %s/%s:%i (%s) [%s]", iface, protocol, name, service_type, domain_name, hostname, t, port, s, resolver_event_to_string(event));
        avahi_free(s);
    }
}

static void dsb_callback(
    AVAHI_GCC_UNUSED AvahiSDNSServerBrowser *b,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char*hostname,
    const AvahiAddress *a,
    uint16_t port,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {

    char t[AVAHI_ADDRESS_STR_MAX] = "n/a";

    if (a)
        avahi_address_snprint(t, sizeof(t), a);

    avahi_log_debug("DSB: (%i.%i): %s/%s:%i [%s]", iface, protocol, hostname ? hostname : "NULL", t, port, browser_event_to_string(event));
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    AvahiSRecordBrowser *r;
    AvahiSHostNameResolver *hnr;
    AvahiSAddressResolver *ar;
    AvahiKey *k;
    AvahiServerConfig config;
    AvahiAddress a;
    AvahiSDomainBrowser *db;
    AvahiSServiceTypeBrowser *stb;
    AvahiSServiceBrowser *sb;
    AvahiSServiceResolver *sr;
    AvahiSDNSServerBrowser *dsb;
    AvahiSimplePoll *simple_poll;
    int error;
    struct timeval tv;

    simple_poll = avahi_simple_poll_new();
    poll_api = avahi_simple_poll_get(simple_poll);

    avahi_server_config_init(&config);

    avahi_address_parse("192.168.50.1", AVAHI_PROTO_UNSPEC, &config.wide_area_servers[0]);
    config.n_wide_area_servers = 1;
    config.enable_wide_area = 1;

    server = avahi_server_new(poll_api, &config, server_callback, NULL, &error);
    avahi_server_config_free(&config);

    k = avahi_key_new("_http._tcp.0pointer.de", AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_PTR);
    r = avahi_s_record_browser_prepare(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, k, 0, record_browser_callback, NULL);
    avahi_s_record_browser_start_query(r);
    avahi_key_unref(k);

    hnr = avahi_s_host_name_resolver_prepare(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "cname.local", AVAHI_PROTO_UNSPEC, 0, hnr_callback, NULL);
    avahi_s_host_name_resolver_start(hnr);

    ar = avahi_s_address_resolver_prepare(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, avahi_address_parse("192.168.50.1", AVAHI_PROTO_INET, &a), 0, ar_callback, NULL);
    avahi_s_address_resolver_start(ar);

    db = avahi_s_domain_browser_prepare(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, AVAHI_DOMAIN_BROWSER_BROWSE, 0, db_callback, NULL);
    avahi_s_domain_browser_start(db);

    stb = avahi_s_service_type_browser_prepare(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, 0, stb_callback, NULL);
    avahi_s_service_type_browser_start(stb);

    sb = avahi_s_service_browser_prepare(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_http._tcp", NULL, 0, sb_callback, NULL);
    avahi_s_service_browser_start(sb);

    sr = avahi_s_service_resolver_prepare(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "Ecstasy HTTP", "_http._tcp", "local", AVAHI_PROTO_UNSPEC, 0, sr_callback, NULL);
    avahi_s_service_resolver_start(sr);

    dsb = avahi_s_dns_server_browser_prepare(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "local", AVAHI_DNS_SERVER_RESOLVE, AVAHI_PROTO_UNSPEC, 0, dsb_callback, NULL);
    avahi_s_dns_server_browser_start(dsb);

    avahi_elapse_time(&tv, 1000*5, 0);
    poll_api->timeout_new(poll_api, &tv, dump_timeout_callback, server);

    avahi_elapse_time(&tv, 1000*60, 0);
    poll_api->timeout_new(poll_api, &tv, quit_timeout_callback, simple_poll);

    avahi_simple_poll_loop(simple_poll);

    avahi_s_record_browser_free(r);
    avahi_s_host_name_resolver_free(hnr);
    avahi_s_address_resolver_free(ar);
    avahi_s_domain_browser_free(db);
    avahi_s_service_type_browser_free(stb);
    avahi_s_service_browser_free(sb);
    avahi_s_service_resolver_free(sr);
    avahi_s_dns_server_browser_free(dsb);

    if (group)
        avahi_s_entry_group_free(group);

    if (server)
        avahi_server_free(server);

    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    avahi_free(service_name);

    return 0;
}
