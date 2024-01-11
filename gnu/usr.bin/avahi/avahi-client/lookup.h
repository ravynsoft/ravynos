#ifndef fooclientlookuphfoo
#define fooclientlookuphfoo

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

#include <inttypes.h>

#include <avahi-common/cdecl.h>
#include <avahi-common/address.h>
#include <avahi-common/strlst.h>
#include <avahi-common/defs.h>
#include <avahi-common/watch.h>
#include <avahi-common/gccmacro.h>

#include <avahi-client/client.h>

/** \file avahi-client/lookup.h Lookup Client API */

/** \example client-browse-services.c Example how to browse for DNS-SD
 * services using the client interface to avahi-daemon. */

AVAHI_C_DECL_BEGIN

/** @{ \name Domain Browser */

/** A domain browser object */
typedef struct AvahiDomainBrowser AvahiDomainBrowser;

/** The function prototype for the callback of an AvahiDomainBrowser */
typedef void (*AvahiDomainBrowserCallback) (
    AvahiDomainBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *domain,
    AvahiLookupResultFlags flags,
    void *userdata);

/** Browse for domains on the local network */
AvahiDomainBrowser* avahi_domain_browser_new (
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiDomainBrowserType btype,
    AvahiLookupFlags flags,
    AvahiDomainBrowserCallback callback,
    void *userdata);

/** Get the parent client of an AvahiDomainBrowser object */
AvahiClient* avahi_domain_browser_get_client (AvahiDomainBrowser *);

/** Cleans up and frees an AvahiDomainBrowser object */
int avahi_domain_browser_free (AvahiDomainBrowser *);

/** @} */

/** @{ \name Service Browser */

/** A service browser object */
typedef struct AvahiServiceBrowser AvahiServiceBrowser;

/** The function prototype for the callback of an AvahiServiceBrowser */
typedef void (*AvahiServiceBrowserCallback) (
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AvahiLookupResultFlags flags,
    void *userdata);

/** Browse for services of a type on the network. In most cases you
 * probably want to pass AVAHI_IF_UNSPEC and AVAHI_PROTO_UNSPED in
 * interface, resp. protocol to browse on all local networks. The
 * specified callback will be called whenever a new service appears
 * or is removed from the network. Please note that events may be
 * collapsed to minimize traffic (i.e. a REMOVED followed by a NEW for
 * the same service data is dropped because redundant). If you want to
 * subscribe to service data changes, you should use
 * avahi_service_resolver_new() and keep it open, in which case you
 * will be notified via AVAHI_RESOLVE_FOUND everytime the service data
 * changes. */
AvahiServiceBrowser* avahi_service_browser_new (
    AvahiClient *client,
    AvahiIfIndex interface,     /**< In most cases pass AVAHI_IF_UNSPEC here */
    AvahiProtocol protocol,     /**< In most cases pass AVAHI_PROTO_UNSPEC here */
    const char *type,           /**< A service type such as "_http._tcp" */
    const char *domain,         /**< A domain to browse in. In most cases you want to pass NULL here for the default domain (usually ".local") */
    AvahiLookupFlags flags,
    AvahiServiceBrowserCallback callback,
    void *userdata);

/** Get the parent client of an AvahiServiceBrowser object */
AvahiClient* avahi_service_browser_get_client (AvahiServiceBrowser *);

/** Cleans up and frees an AvahiServiceBrowser object */
int avahi_service_browser_free (AvahiServiceBrowser *);

/** @} */

/** \cond fulldocs */
/** A service type browser object */
typedef struct AvahiServiceTypeBrowser AvahiServiceTypeBrowser;

/** The function prototype for the callback of an AvahiServiceTypeBrowser */
typedef void (*AvahiServiceTypeBrowserCallback) (
    AvahiServiceTypeBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *type,
    const char *domain,
    AvahiLookupResultFlags flags,
    void *userdata);

/** Browse for service types on the local network */
AvahiServiceTypeBrowser* avahi_service_type_browser_new (
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiLookupFlags flags,
    AvahiServiceTypeBrowserCallback callback,
    void *userdata);

/** Get the parent client of an AvahiServiceTypeBrowser object */
AvahiClient* avahi_service_type_browser_get_client (AvahiServiceTypeBrowser *);

/** Cleans up and frees an AvahiServiceTypeBrowser object */
int avahi_service_type_browser_free (AvahiServiceTypeBrowser *);

/** \endcond */

/** @{ \name Service Resolver */

/** A service resolver object */
typedef struct AvahiServiceResolver AvahiServiceResolver;

/** The function prototype for the callback of an AvahiServiceResolver */
typedef void (*AvahiServiceResolverCallback) (
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
    AvahiLookupResultFlags flags,
    void *userdata);

/** Create a new service resolver object. Please make sure to pass all the
 * service data you received via avahi_service_browser_new()'s callback
 * function, especially interface and protocol. The protocol argument specifies
 * the protocol (IPv4 or IPv6) to use as transport for the queries which are
 * sent out by this resolver. The aprotocol argument specifies the adress
 * family (IPv4 or IPv6) of the address of the service we are looking for.
 * Generally, on "protocol" you should only pass what was supplied to you as
 * parameter to your AvahiServiceBrowserCallback. In "aprotocol" you should
 * pass what your application code can deal with when connecting to the
 * service. Or, more technically speaking: protocol specifies if the mDNS
 * queries should be sent as UDP/IPv4 resp. UDP/IPv6 packets. aprotocol
 * specifies whether the query is for a A resp. AAAA resource record.
 *
 * Avahi browser and resolver callbacks only receive a concrete protocol;
 * always AVAHI_PROTO_INET or AVAHI_PROTO_INET6 and never AVAHI_PROTO_UNSPEC. A
 * new browser given UNSPEC will receive both (separate) INET and INET6 events.
 * A new resolver given a query protocol of UNSPEC will default to querying
 * with INET6. A new resolver given an address protocol of UNSPEC will always
 * resolve a service to an address matching the query protocol. So a resolver
 * with UNSPEC/UNSPEC is equivalent to INET6/INET6. By default the avahi daemon
 * publishes AAAA (IPv6) records over IPv4, but not A (IPv4) records over IPv6
 * (see 'publish-aaaa-on-ipv4' and 'publish-a-on-ipv6' in 'avahi-daemon.conf').
 * That's why, given most daemons, all four combinations of concrete query and
 * address protocols resolve except INET addresses via INET6 queries. */
AvahiServiceResolver * avahi_service_resolver_new(
    AvahiClient *client,
    AvahiIfIndex interface,   /**< Pass the interface argument you received in AvahiServiceBrowserCallback here. */
    AvahiProtocol protocol,   /**< Pass the protocol argument you received in AvahiServiceBrowserCallback here. */
    const char *name,         /**< Pass the name argument you received in AvahiServiceBrowserCallback here. */
    const char *type,         /**< Pass the type argument you received in AvahiServiceBrowserCallback here. */
    const char *domain,       /**< Pass the domain argument you received in AvahiServiceBrowserCallback here. */
    AvahiProtocol aprotocol,  /**< The desired address family of the service address to resolve. AVAHI_PROTO_UNSPEC if your application can deal with both IPv4 and IPv6 */
    AvahiLookupFlags flags,
    AvahiServiceResolverCallback callback,
    void *userdata);

/** Get the parent client of an AvahiServiceResolver object */
AvahiClient* avahi_service_resolver_get_client (AvahiServiceResolver *);

/** Free a service resolver object */
int avahi_service_resolver_free(AvahiServiceResolver *r);

/** @} */

/** \cond fulldocs */
/** A service resolver object */
typedef struct AvahiHostNameResolver AvahiHostNameResolver;

/** The function prototype for the callback of an AvahiHostNameResolver */
typedef void (*AvahiHostNameResolverCallback) (
    AvahiHostNameResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const AvahiAddress *a,
    AvahiLookupResultFlags flags,
    void *userdata);

/** Create a new hostname resolver object */
AvahiHostNameResolver * avahi_host_name_resolver_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    AvahiProtocol aprotocol,
    AvahiLookupFlags flags,
    AvahiHostNameResolverCallback callback,
    void *userdata);

/** Get the parent client of an AvahiHostNameResolver object */
AvahiClient* avahi_host_name_resolver_get_client (AvahiHostNameResolver *);

/** Free a hostname resolver object */
int avahi_host_name_resolver_free(AvahiHostNameResolver *r);

/** An address resolver object */
typedef struct AvahiAddressResolver AvahiAddressResolver;

/** The function prototype for the callback of an AvahiAddressResolver */
typedef void (*AvahiAddressResolverCallback) (
    AvahiAddressResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const AvahiAddress *a,
    const char *name,
    AvahiLookupResultFlags flags,
    void *userdata);

/** Create a new address resolver object from an AvahiAddress object */
AvahiAddressResolver* avahi_address_resolver_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const AvahiAddress *a,
    AvahiLookupFlags flags,
    AvahiAddressResolverCallback callback,
    void *userdata);

/** Get the parent client of an AvahiAddressResolver object */
AvahiClient* avahi_address_resolver_get_client (AvahiAddressResolver *);

/** Free a AvahiAddressResolver resolver object */
int avahi_address_resolver_free(AvahiAddressResolver *r);

/** \endcond */

/** @{ \name Record Browser */

/** A record browser object */
typedef struct AvahiRecordBrowser AvahiRecordBrowser;

/** The function prototype for the callback of an AvahiRecordBrowser */
typedef void (*AvahiRecordBrowserCallback) (
    AvahiRecordBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    uint16_t clazz,
    uint16_t type,
    const void *rdata,
    size_t size,
    AvahiLookupResultFlags flags,
    void *userdata);

/** Browse for records of a type on the local network */
AvahiRecordBrowser* avahi_record_browser_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    uint16_t clazz,
    uint16_t type,
    AvahiLookupFlags flags,
    AvahiRecordBrowserCallback callback,
    void *userdata);

/** Get the parent client of an AvahiRecordBrowser object */
AvahiClient* avahi_record_browser_get_client(AvahiRecordBrowser *);

/** Cleans up and frees an AvahiRecordBrowser object */
int avahi_record_browser_free(AvahiRecordBrowser *);

/** @} */

AVAHI_C_DECL_END

#endif
