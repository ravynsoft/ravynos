#ifndef foolookuphfoo
#define foolookuphfoo

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

/** \file avahi-core/lookup.h Functions for browsing/resolving services and other RRs */

/** \example core-browse-services.c Example how to browse for DNS-SD
 * services using an embedded mDNS stack. */

/** A browsing object for arbitrary RRs */
typedef struct AvahiSRecordBrowser AvahiSRecordBrowser;

/** A host name to IP adddress resolver object */
typedef struct AvahiSHostNameResolver AvahiSHostNameResolver;

/** An IP address to host name resolver object ("reverse lookup") */
typedef struct AvahiSAddressResolver AvahiSAddressResolver;

/** A local domain browsing object. May be used to enumerate domains used on the local LAN */
typedef struct AvahiSDomainBrowser AvahiSDomainBrowser;

/** A DNS-SD service type browsing object. May be used to enumerate the service types of all available services on the local LAN */
typedef struct AvahiSServiceTypeBrowser AvahiSServiceTypeBrowser;

/** A DNS-SD service browser. Use this to enumerate available services of a certain kind on the local LAN. Use AvahiSServiceResolver to get specific service data like address and port for a service. */
typedef struct AvahiSServiceBrowser AvahiSServiceBrowser;

/** A DNS-SD service resolver.  Use this to retrieve addres, port and TXT data for a DNS-SD service */
typedef struct AvahiSServiceResolver AvahiSServiceResolver;

#include <avahi-common/cdecl.h>
#include <avahi-common/defs.h>
#include <avahi-core/core.h>

AVAHI_C_DECL_BEGIN

/** Callback prototype for AvahiSRecordBrowser events */
typedef void (*AvahiSRecordBrowserCallback)(
    AvahiSRecordBrowser *b,          /**< The AvahiSRecordBrowser object that is emitting this callback */
    AvahiIfIndex interface,          /**< Logical OS network interface number the record was found on */
    AvahiProtocol protocol,          /**< Protocol number the record was found. */
    AvahiBrowserEvent event,         /**< Browsing event, either AVAHI_BROWSER_NEW or AVAHI_BROWSER_REMOVE */
    AvahiRecord *record,             /**< The record that was found */
    AvahiLookupResultFlags flags,  /**< Lookup flags */
    void* userdata                   /**< Arbitrary user data passed to avahi_s_record_browser_new() */ );

/** Create a new browsing object for arbitrary RRs */
AvahiSRecordBrowser *avahi_s_record_browser_new(
    AvahiServer *server,                    /**< The server object to which attach this query */
    AvahiIfIndex interface,                 /**< Logical OS interface number where to look for the records, or AVAHI_IF_UNSPEC to look on interfaces */
    AvahiProtocol protocol,                 /**< Protocol number to use when looking for the record, or AVAHI_PROTO_UNSPEC to look on all protocols */
    AvahiKey *key,                          /**< The search key */
    AvahiLookupFlags flags,                 /**< Lookup flags. Must have set either AVAHI_LOOKUP_FORCE_WIDE_AREA or AVAHI_LOOKUP_FORCE_MULTICAST, since domain based detection is not available here. */
    AvahiSRecordBrowserCallback callback,   /**< The callback to call on browsing events */
    void* userdata                          /**< Arbitrary use suppliable data which is passed to the callback */);

/** Prepare a new browsing object for arbitrary RRs */
AvahiSRecordBrowser *avahi_s_record_browser_prepare(
    AvahiServer *server,                    /**< The server object to which attach this query */
    AvahiIfIndex interface,                 /**< Logical OS interface number where to look for the records, or AVAHI_IF_UNSPEC to look on interfaces */
    AvahiProtocol protocol,                 /**< Protocol number to use when looking for the record, or AVAHI_PROTO_UNSPEC to look on all protocols */
    AvahiKey *key,                          /**< The search key */
    AvahiLookupFlags flags,                 /**< Lookup flags. Must have set either AVAHI_LOOKUP_FORCE_WIDE_AREA or AVAHI_LOOKUP_FORCE_MULTICAST, since domain based detection is not available here. */
    AvahiSRecordBrowserCallback callback,   /**< The callback to call on browsing events */
    void* userdata                          /**< Arbitrary use suppliable data which is passed to the callback */);

/** Free an AvahiSRecordBrowser object */
void avahi_s_record_browser_free(AvahiSRecordBrowser *b);

/** Start querying on an AvahiSRecordBrowser object */
void avahi_s_record_browser_start_query(AvahiSRecordBrowser *b);

/** Callback prototype for AvahiSHostNameResolver events */
typedef void (*AvahiSHostNameResolverCallback)(
    AvahiSHostNameResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event, /**< Resolving event */
    const char *host_name,   /**< Host name which should be resolved. May differ in case from the query */
    const AvahiAddress *a,    /**< The address, or NULL if the host name couldn't be resolved. */
    AvahiLookupResultFlags flags,  /**< Lookup flags */
    void* userdata);

/** Create an AvahiSHostNameResolver object for resolving a host name to an adddress. See AvahiSRecordBrowser for more info on the paramters. */
AvahiSHostNameResolver *avahi_s_host_name_resolver_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *host_name,                  /**< The host name to look for */
    AvahiProtocol aprotocol,                /**< The address family of the desired address or AVAHI_PROTO_UNSPEC if doesn't matter. */
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSHostNameResolverCallback calback,
    void* userdata);

/** Prepare an AvahiSHostNameResolver object for resolving a host name to an adddress. See AvahiSRecordBrowser for more info on the paramters. */
AvahiSHostNameResolver *avahi_s_host_name_resolver_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *host_name,                  /**< The host name to look for */
    AvahiProtocol aprotocol,                /**< The address family of the desired address or AVAHI_PROTO_UNSPEC if doesn't matter. */
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSHostNameResolverCallback calback,
    void* userdata);

/** Start querying on an AvahiSHostNameResolver object */
void avahi_s_host_name_resolver_start(AvahiSHostNameResolver *r);

/** Free a AvahiSHostNameResolver object */
void avahi_s_host_name_resolver_free(AvahiSHostNameResolver *r);

/** Callback prototype for AvahiSAddressResolver events */
typedef void (*AvahiSAddressResolverCallback)(
    AvahiSAddressResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const AvahiAddress *a,
    const char *host_name,   /**< A host name for the specified address, if one was found, i.e. event == AVAHI_RESOLVER_FOUND */
    AvahiLookupResultFlags flags,  /**< Lookup flags */
    void* userdata);

/** Create an AvahiSAddressResolver object. See AvahiSRecordBrowser for more info on the paramters. */
AvahiSAddressResolver *avahi_s_address_resolver_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const AvahiAddress *address,
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSAddressResolverCallback calback,
    void* userdata);

/** Prepare an AvahiSAddressResolver object. See AvahiSRecordBrowser for more info on the paramters. */
AvahiSAddressResolver *avahi_s_address_resolver_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const AvahiAddress *address,
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSAddressResolverCallback calback,
    void* userdata);

/** Start querying on an AvahiSAddressResolver object */
void avahi_s_address_resolver_start(AvahiSAddressResolver *r);

/** Free an AvahiSAddressResolver object */
void avahi_s_address_resolver_free(AvahiSAddressResolver *r);

/** Callback prototype for AvahiSDomainBrowser events */
typedef void (*AvahiSDomainBrowserCallback)(
    AvahiSDomainBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *domain,
    AvahiLookupResultFlags flags,  /**< Lookup flags */
    void* userdata);

/** Create a new AvahiSDomainBrowser object */
AvahiSDomainBrowser *avahi_s_domain_browser_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiDomainBrowserType type,
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSDomainBrowserCallback callback,
    void* userdata);

/** Prepare a new AvahiSDomainBrowser object */
AvahiSDomainBrowser *avahi_s_domain_browser_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiDomainBrowserType type,
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSDomainBrowserCallback callback,
    void* userdata);

/** Start querying on an AvahiSDomainBrowser object */
void avahi_s_domain_browser_start(AvahiSDomainBrowser *b);

/** Free an AvahiSDomainBrowser object */
void avahi_s_domain_browser_free(AvahiSDomainBrowser *b);

/** Callback prototype for AvahiSServiceTypeBrowser events */
typedef void (*AvahiSServiceTypeBrowserCallback)(
    AvahiSServiceTypeBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *type,
    const char *domain,
    AvahiLookupResultFlags flags,  /**< Lookup flags */
    void* userdata);

/** Create a new AvahiSServiceTypeBrowser object. */
AvahiSServiceTypeBrowser *avahi_s_service_type_browser_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSServiceTypeBrowserCallback callback,
    void* userdata);

/** Prepare a new AvahiSServiceTypeBrowser object. */
AvahiSServiceTypeBrowser *avahi_s_service_type_browser_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSServiceTypeBrowserCallback callback,
    void* userdata);

/** Start querying on an AvahiSServiceTypeBrowser object */
void avahi_s_service_type_browser_start(AvahiSServiceTypeBrowser *b);

/** Free an AvahiSServiceTypeBrowser object */
void avahi_s_service_type_browser_free(AvahiSServiceTypeBrowser *b);

/** Callback prototype for AvahiSServiceBrowser events */
typedef void (*AvahiSServiceBrowserCallback)(
    AvahiSServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name     /**< Service name, e.g. "Lennart's Files" */,
    const char *type     /**< DNS-SD type, e.g. "_http._tcp" */,
    const char *domain   /**< Domain of this service, e.g. "local" */,
    AvahiLookupResultFlags flags,  /**< Lookup flags */
    void* userdata);

/** Create a new AvahiSServiceBrowser object. */
AvahiSServiceBrowser *avahi_s_service_browser_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *service_type /** DNS-SD service type, e.g. "_http._tcp" */,
    const char *domain,
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSServiceBrowserCallback callback,
    void* userdata);

/** Prepare a new AvahiSServiceBrowser object. */
AvahiSServiceBrowser *avahi_s_service_browser_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *service_type /** DNS-SD service type, e.g. "_http._tcp" */,
    const char *domain,
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSServiceBrowserCallback callback,
    void* userdata);

/** Start querying on an AvahiSServiceBrowser object */
void avahi_s_service_browser_start(AvahiSServiceBrowser *b);

/** Free an AvahiSServiceBrowser object */
void avahi_s_service_browser_free(AvahiSServiceBrowser *b);

/** Callback prototype for AvahiSServiceResolver events */
typedef void (*AvahiSServiceResolverCallback)(
    AvahiSServiceResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,  /**< Is AVAHI_RESOLVER_FOUND when the service was resolved successfully, and everytime it changes. Is AVAHI_RESOLVER_TIMOUT when the service failed to resolve or disappeared. */
    const char *name,       /**< Service name */
    const char *type,       /**< Service Type */
    const char *domain,
    const char *host_name,  /**< Host name of the service */
    const AvahiAddress *a,   /**< The resolved host name */
    uint16_t port,            /**< Service name */
    AvahiStringList *txt,    /**< TXT record data */
    AvahiLookupResultFlags flags,  /**< Lookup flags */
    void* userdata);

/** Create a new AvahiSServiceResolver object. The specified callback function will be called with the resolved service data. */
AvahiSServiceResolver *avahi_s_service_resolver_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    const char *type,
    const char *domain,
    AvahiProtocol aprotocol,    /**< Address family of the desired service address. Use AVAHI_PROTO_UNSPEC if you don't care */
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSServiceResolverCallback calback,
    void* userdata);

/** Prepare a new AvahiSServiceResolver object. The specified callback function will be called with the resolved service data. */
AvahiSServiceResolver *avahi_s_service_resolver_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    const char *type,
    const char *domain,
    AvahiProtocol aprotocol,    /**< Address family of the desired service address. Use AVAHI_PROTO_UNSPEC if you don't care */
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSServiceResolverCallback calback,
    void* userdata);

/** Start querying on an AvahiSServiceResolver object */
void avahi_s_service_resolver_start(AvahiSServiceResolver *r);

/** Free an AvahiSServiceResolver object */
void avahi_s_service_resolver_free(AvahiSServiceResolver *r);

AVAHI_C_DECL_END

#endif
