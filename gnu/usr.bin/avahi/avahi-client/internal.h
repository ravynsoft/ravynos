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

#include <dbus/dbus.h>

#include "client.h"
#include "lookup.h"
#include "publish.h"

struct AvahiClient {
    const AvahiPoll *poll_api;
    DBusConnection *bus;
    int error;
    AvahiClientState state;
    AvahiClientFlags flags;

    /* Cache for some seldom changing server data */
    char *version_string, *host_name, *host_name_fqdn, *domain_name;
    uint32_t local_service_cookie;
    int local_service_cookie_valid;

    AvahiClientCallback callback;
    void *userdata;

    AVAHI_LLIST_HEAD(AvahiEntryGroup, groups);
    AVAHI_LLIST_HEAD(AvahiDomainBrowser, domain_browsers);
    AVAHI_LLIST_HEAD(AvahiServiceBrowser, service_browsers);
    AVAHI_LLIST_HEAD(AvahiServiceTypeBrowser, service_type_browsers);
    AVAHI_LLIST_HEAD(AvahiServiceResolver, service_resolvers);
    AVAHI_LLIST_HEAD(AvahiHostNameResolver, host_name_resolvers);
    AVAHI_LLIST_HEAD(AvahiAddressResolver, address_resolvers);
    AVAHI_LLIST_HEAD(AvahiRecordBrowser, record_browsers);
};

struct AvahiEntryGroup {
    char *path;
    AvahiEntryGroupState state;
    int state_valid;
    AvahiClient *client;
    AvahiEntryGroupCallback callback;
    void *userdata;
    AVAHI_LLIST_FIELDS(AvahiEntryGroup, groups);
};

struct AvahiDomainBrowser {
    int ref;

    char *path;
    AvahiClient *client;
    AvahiDomainBrowserCallback callback;
    void *userdata;
    AVAHI_LLIST_FIELDS(AvahiDomainBrowser, domain_browsers);

    AvahiIfIndex interface;
    AvahiProtocol protocol;

    AvahiTimeout *defer_timeout;

    AvahiStringList *static_browse_domains;
};

struct AvahiServiceBrowser {
    char *path;
    AvahiClient *client;
    AvahiServiceBrowserCallback callback;
    void *userdata;
    AVAHI_LLIST_FIELDS(AvahiServiceBrowser, service_browsers);

    char *type, *domain;
    AvahiIfIndex interface;
    AvahiProtocol protocol;
};

struct AvahiServiceTypeBrowser {
    char *path;
    AvahiClient *client;
    AvahiServiceTypeBrowserCallback callback;
    void *userdata;
    AVAHI_LLIST_FIELDS(AvahiServiceTypeBrowser, service_type_browsers);

    char *domain;
    AvahiIfIndex interface;
    AvahiProtocol protocol;
};

struct AvahiServiceResolver {
    char *path;
    AvahiClient *client;
    AvahiServiceResolverCallback callback;
    void *userdata;
    AVAHI_LLIST_FIELDS(AvahiServiceResolver, service_resolvers);

    char *name, *type, *domain;
    AvahiIfIndex interface;
    AvahiProtocol protocol;
};

struct AvahiHostNameResolver {
    char *path;
    AvahiClient *client;
    AvahiHostNameResolverCallback callback;
    void *userdata;
    AVAHI_LLIST_FIELDS(AvahiHostNameResolver, host_name_resolvers);

    char *host_name;
    AvahiIfIndex interface;
    AvahiProtocol protocol;
};

struct AvahiAddressResolver {
    char *path;
    AvahiClient *client;
    AvahiAddressResolverCallback callback;
    void *userdata;
    AVAHI_LLIST_FIELDS(AvahiAddressResolver, address_resolvers);

    AvahiAddress address;
    AvahiIfIndex interface;
    AvahiProtocol protocol;
};

struct AvahiRecordBrowser {
    char *path;
    AvahiClient *client;
    AvahiRecordBrowserCallback callback;
    void *userdata;
    AVAHI_LLIST_FIELDS(AvahiRecordBrowser, record_browsers);

    char *name;
    uint16_t clazz, type;
    AvahiIfIndex interface;
    AvahiProtocol protocol;
};

int avahi_client_set_errno (AvahiClient *client, int error);
int avahi_client_set_dbus_error(AvahiClient *client, DBusError *error);

void avahi_entry_group_set_state(AvahiEntryGroup *group, AvahiEntryGroupState state);

DBusHandlerResult avahi_domain_browser_event (AvahiClient *client, AvahiBrowserEvent event, DBusMessage *message);
DBusHandlerResult avahi_service_type_browser_event (AvahiClient *client, AvahiBrowserEvent event, DBusMessage *message);
DBusHandlerResult avahi_service_browser_event (AvahiClient *client, AvahiBrowserEvent event, DBusMessage *message);
DBusHandlerResult avahi_record_browser_event(AvahiClient *client, AvahiBrowserEvent event, DBusMessage *message);

DBusHandlerResult avahi_service_resolver_event (AvahiClient *client, AvahiResolverEvent event, DBusMessage *message);
DBusHandlerResult avahi_host_name_resolver_event (AvahiClient *client, AvahiResolverEvent event, DBusMessage *message);
DBusHandlerResult avahi_address_resolver_event (AvahiClient *client, AvahiResolverEvent event, DBusMessage *message);

int avahi_client_simple_method_call(AvahiClient *client, const char *path, const char *interface, const char *method);

int avahi_client_is_connected(AvahiClient *client);

#endif
