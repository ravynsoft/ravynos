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

#include <stdio.h>
#include <assert.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>

#include <avahi-common/error.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/timeval.h>

static const AvahiPoll *poll_api = NULL;
static AvahiSimplePoll *simple_poll = NULL;

static void avahi_client_callback (AvahiClient *c, AvahiClientState state, void *userdata) {
    printf ("CLIENT: Callback on %p, state -> %d, data -> %s\n", (void*) c, state, (char*)userdata);
}

static void avahi_entry_group_callback (AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata) {
    printf ("ENTRY-GROUP: Callback on %p, state -> %d, data -> %s\n", (void*) g, state, (char*)userdata);
}

static void avahi_entry_group2_callback (AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata) {
    printf ("ENTRY-GROUP2: Callback on %p, state -> %d, data -> %s\n", (void*) g, state, (char*)userdata);
}

static void avahi_domain_browser_callback(
    AvahiDomainBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    printf ("DOMAIN-BROWSER: Callback on %p, interface (%d), protocol (%d), event (%d), domain (%s), data (%s)\n", (void*) b, interface, protocol, event, domain ? domain : "NULL", (char*)userdata);
}

static void avahi_service_resolver_callback(
    AvahiServiceResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *a,
    uint16_t port,
    AvahiStringList *txt,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    char addr[64];
    char *txtr;
    if (event == AVAHI_RESOLVER_FAILURE) {
        printf ("SERVICE-RESOLVER: ServiceResolver %p timed out (%s %s)\n", (void*) r, name, type);
        return;
    }
    avahi_address_snprint (addr, sizeof (addr), a);
    txtr = avahi_string_list_to_string (txt);
    printf ("SERVICE-RESOLVER: Callback on ServiceResolver, interface (%d), protocol (%d), event (%d), name (%s), type (%s), domain (%s), host_name (%s), address (%s), port (%d), txtdata (%s), data(%s)\n", interface, protocol, event, name, type, domain, host_name, addr, port, txtr, (char*)userdata);
    avahi_free(txtr);
}

static void avahi_service_browser_callback (
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    AvahiServiceResolver *sr;

    printf ("SERVICE-BROWSER: Callback on %p, interface (%d), protocol (%d), event (%d), name (%s), type (%s), domain (%s), data (%s)\n", (void*) b, interface, protocol, event, name ? name : "NULL", type, domain ? domain : "NULL", (char*)userdata);

    if (b && name)
    {
        sr = avahi_service_resolver_new (avahi_service_browser_get_client (b), interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, 0, avahi_service_resolver_callback, (char*) "xxXXxx");
        printf("New service resolver %p\n", (void*) sr);
    }
}

static void avahi_service_type_browser_callback (
    AvahiServiceTypeBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    printf ("SERVICE-TYPE-BROWSER: Callback on %p, interface (%d), protocol (%d), event (%d), type (%s), domain (%s), data (%s)\n", (void*) b, interface, protocol, event, type ? type : "NULL", domain ? domain : "NULL", (char*)userdata);
}

static void avahi_address_resolver_callback (
    AVAHI_GCC_UNUSED AvahiAddressResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const AvahiAddress *address,
    const char *name,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    char addr[64];
    if (event == AVAHI_RESOLVER_FAILURE) {
        printf ("ADDRESS-RESOLVER: Callback on AddressResolver, timed out.\n");
        return;
    }
    avahi_address_snprint (addr, sizeof (addr), address);
    printf ("ADDRESS-RESOLVER: Callback on AddressResolver, interface (%d), protocol (%d), even (%d), address (%s), name (%s), data(%s)\n", interface, protocol, event, addr, name, (char*) userdata);
}

static void avahi_host_name_resolver_callback (
    AvahiHostNameResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const AvahiAddress *a,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    AvahiClient *client;
    AvahiAddressResolver *ar;
    char addr[64];

    if (event == AVAHI_RESOLVER_FAILURE) {
        printf ("HOST-NAME-RESOLVER: Callback on HostNameResolver, timed out.\n");
        return;
    }
    client = avahi_host_name_resolver_get_client (r);
ar = avahi_address_resolver_new(client, interface, protocol, a, 0, avahi_address_resolver_callback, (char*) "omghai6u");
    if (ar)
    {
        printf ("Succesfully created address resolver object\n");
    } else {
        printf ("Failed to create AddressResolver\n");
    }
    avahi_address_snprint (addr, sizeof (addr), a);
    printf ("HOST-NAME-RESOLVER: Callback on HostNameResolver, interface (%d), protocol (%d), event (%d), name (%s), address (%s), data (%s)\n", interface, protocol, event, name, addr, (char*)userdata);
}
static void test_free_domain_browser(AVAHI_GCC_UNUSED AvahiTimeout *timeout, void* userdata)
{
    AvahiServiceBrowser *b = userdata;
    printf ("Freeing domain browser\n");
    avahi_service_browser_free (b);
}

static void test_free_entry_group (AVAHI_GCC_UNUSED AvahiTimeout *timeout, void* userdata)
{
    AvahiEntryGroup *g = userdata;
    printf ("Freeing entry group\n");
    avahi_entry_group_free (g);
}

static void test_entry_group_reset (AVAHI_GCC_UNUSED AvahiTimeout *timeout, void* userdata)
{
    AvahiEntryGroup *g = userdata;

    printf ("Resetting entry group\n");
    avahi_entry_group_reset (g);

    avahi_entry_group_add_service (g, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, "Lathiat's Site", "_http._tcp", NULL, NULL, 80, "foo=bar2", NULL);

    avahi_entry_group_commit (g);
}

static void test_entry_group_update(AVAHI_GCC_UNUSED AvahiTimeout *timeout, void* userdata) {
    AvahiEntryGroup *g = userdata;

    printf ("Updating entry group\n");

    avahi_entry_group_update_service_txt(g, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, "Lathiat's Site", "_http._tcp", NULL, "foo=bar3", NULL);
}

static void terminate(AVAHI_GCC_UNUSED AvahiTimeout *timeout, AVAHI_GCC_UNUSED void *userdata) {

    avahi_simple_poll_quit(simple_poll);
}

int main (AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    AvahiClient *avahi;
    AvahiEntryGroup *group, *group2;
    AvahiDomainBrowser *domain;
    AvahiServiceBrowser *sb;
    AvahiServiceTypeBrowser *st;
    AvahiHostNameResolver *hnr;
    AvahiAddress *aar;
    const char *ret;
    int error;
    uint32_t cookie;
    struct timeval tv;
    AvahiAddress a;

    simple_poll = avahi_simple_poll_new();
    poll_api = avahi_simple_poll_get(simple_poll);

    if (!(avahi = avahi_client_new(poll_api, 0, avahi_client_callback, (char*) "omghai2u", &error))) {
        fprintf(stderr, "Client failed: %s\n", avahi_strerror(error));
        goto fail;
    }

    printf("State: %i\n", avahi_client_get_state(avahi));

    ret = avahi_client_get_version_string (avahi);
    printf("Avahi Server Version: %s (Error Return: %s)\n", ret, ret ? "OK" : avahi_strerror(avahi_client_errno(avahi)));

    ret = avahi_client_get_host_name (avahi);
    printf("Host Name: %s (Error Return: %s)\n", ret, ret ? "OK" : avahi_strerror(avahi_client_errno(avahi)));

    ret = avahi_client_get_domain_name (avahi);
    printf("Domain Name: %s (Error Return: %s)\n", ret, ret ? "OK" : avahi_strerror(avahi_client_errno(avahi)));

    ret = avahi_client_get_host_name_fqdn (avahi);
    printf("FQDN: %s (Error Return: %s)\n", ret, ret ? "OK" : avahi_strerror(avahi_client_errno(avahi)));

    cookie = avahi_client_get_local_service_cookie(avahi);
    printf("Local service cookie: %u (Error Return: %s)\n", cookie, cookie != AVAHI_SERVICE_COOKIE_INVALID ? "OK" : avahi_strerror(avahi_client_errno(avahi)));

    group = avahi_entry_group_new(avahi, avahi_entry_group_callback, (char*) "omghai");
    printf("Creating entry group: %s\n", group ? "OK" : avahi_strerror(avahi_client_errno (avahi)));

    assert(group);

    printf("Sucessfully created entry group %p\n", (void*) group);

    printf("%s\n", avahi_strerror(avahi_entry_group_add_service (group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, "Lathiat's Site", "_http._tcp", NULL, NULL, 80, "foo=bar", NULL)));
    printf("add_record: %d\n", avahi_entry_group_add_record (group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, "TestX", 0x01, 0x10, 120, "\5booya", 6));

    avahi_entry_group_commit (group);

    domain = avahi_domain_browser_new (avahi, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, AVAHI_DOMAIN_BROWSER_BROWSE, 0, avahi_domain_browser_callback, (char*) "omghai3u");

    if (domain == NULL)
        printf ("Failed to create domain browser object\n");
    else
        printf ("Sucessfully created domain browser %p\n", (void*) domain);

    st = avahi_service_type_browser_new (avahi, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, 0, avahi_service_type_browser_callback, (char*) "omghai3u");
    if (st == NULL)
        printf ("Failed to create service type browser object\n");
    else
        printf ("Sucessfully created service type browser %p\n", (void*) st);

    sb = avahi_service_browser_new (avahi, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_http._tcp", NULL, 0, avahi_service_browser_callback, (char*) "omghai3u");
    if (sb == NULL)
        printf ("Failed to create service browser object\n");
    else
        printf ("Sucessfully created service browser %p\n", (void*) sb);

    hnr = avahi_host_name_resolver_new (avahi, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "ecstasy.local", AVAHI_PROTO_UNSPEC, 0, avahi_host_name_resolver_callback, (char*) "omghai4u");
    if (hnr == NULL)
        printf ("Failed to create hostname resolver object\n");
    else
        printf ("Successfully created hostname resolver object\n");

    aar = avahi_address_parse ("224.0.0.251", AVAHI_PROTO_UNSPEC, &a);
    if (aar == NULL) {
        printf ("failed to create address object\n");
    } else {
        group2 = avahi_entry_group_new (avahi, avahi_entry_group2_callback, (char*) "omghai222");
        if ((error = avahi_entry_group_add_address (group2, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, "test-mdns.local.", aar)) < 0)
        {
            printf ("*** failed to add address to entry group: %s\n", avahi_strerror (error));
            avahi_entry_group_free (group2);
        } else {
            printf ("*** success, added address\n");
            avahi_entry_group_commit (group2);
        }
    }

    avahi_elapse_time(&tv, 8000, 0);
    poll_api->timeout_new(poll_api, &tv, test_entry_group_reset, group);
    avahi_elapse_time(&tv, 15000, 0);
    poll_api->timeout_new(poll_api, &tv, test_entry_group_update, group);
    avahi_elapse_time(&tv, 20000, 0);
    poll_api->timeout_new(poll_api, &tv, test_free_entry_group, group);
    avahi_elapse_time(&tv, 25000, 0);
    poll_api->timeout_new(poll_api, &tv, test_free_domain_browser, sb);

    avahi_elapse_time(&tv, 30000, 0);
    poll_api->timeout_new(poll_api, &tv, terminate, NULL);

    avahi_simple_poll_loop(simple_poll);

    printf("terminating...\n");

fail:

    if (avahi)
        avahi_client_free (avahi);

    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    return 0;
}
