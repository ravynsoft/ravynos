#ifndef fooclientpublishhfoo
#define fooclientpublishhfoo

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

/** \file avahi-client/publish.h Publishing Client API */

/** \example client-publish-service.c Example how to register a DNS-SD
 * service using the client interface to avahi-daemon. It behaves like a network
 * printer registering both an IPP and a BSD LPR service. */

AVAHI_C_DECL_BEGIN

/** An entry group object */
typedef struct AvahiEntryGroup AvahiEntryGroup;

/** The function prototype for the callback of an AvahiEntryGroup */
typedef void (*AvahiEntryGroupCallback) (
    AvahiEntryGroup *g,
    AvahiEntryGroupState state /**< The new state of the entry group */,
    void* userdata /* The arbitrary user data pointer originally passed to avahi_entry_group_new()*/);

/** @{ \name Construction and destruction */

/** Create a new AvahiEntryGroup object */
AvahiEntryGroup* avahi_entry_group_new(
    AvahiClient* c,
    AvahiEntryGroupCallback callback /**< This callback is called whenever the state of this entry group changes. May not be NULL. Please note that this function is called for the first time from within the avahi_entry_group_new() context! Thus, in the callback you should not make use of global variables that are initialized only after your call to avahi_entry_group_new(). A common mistake is to store the AvahiEntryGroup pointer returned by avahi_entry_group_new() in a global variable and assume that this global variable already contains the valid pointer when the callback is called for the first time. A work-around for this is to always use the AvahiEntryGroup pointer passed to the callback function instead of the global pointer. */,
    void *userdata /**< This arbitrary user data pointer will be passed to the callback functon */);

/** Clean up and free an AvahiEntryGroup object */
int avahi_entry_group_free (AvahiEntryGroup *);

/** @} */

/** @{ \name State */

/** Commit an AvahiEntryGroup. The entries in the entry group are now registered on the network. Commiting empty entry groups is considered an error. */
int avahi_entry_group_commit (AvahiEntryGroup*);

/** Reset an AvahiEntryGroup. This takes effect immediately. */
int avahi_entry_group_reset (AvahiEntryGroup*);

/** Get an AvahiEntryGroup's state */
int avahi_entry_group_get_state (AvahiEntryGroup*);

/** Check if an AvahiEntryGroup is empty */
int avahi_entry_group_is_empty (AvahiEntryGroup*);

/** Get an AvahiEntryGroup's owning client instance */
AvahiClient* avahi_entry_group_get_client (AvahiEntryGroup*);

/** @} */

/** @{ \name Adding and updating entries */

/** Add a service. Takes a variable NULL terminated list of TXT record strings as last arguments. Please note that this service is not announced on the network before avahi_entry_group_commit() is called. */
int avahi_entry_group_add_service(
    AvahiEntryGroup *group,
    AvahiIfIndex interface /**< The interface this service shall be announced on. We recommend to pass AVAHI_IF_UNSPEC here, to announce on all interfaces. */,
    AvahiProtocol protocol /**< The protocol this service shall be announced with, i.e. MDNS over IPV4 or MDNS over IPV6. We recommend to pass AVAHI_PROTO_UNSPEC here, to announce this service on all protocols the daemon supports. */,
    AvahiPublishFlags flags /**< Usually 0, unless you know what you do */,
    const char *name        /**< The name for the new service. Must be valid service name. i.e. a string shorter than 63 characters and valid UTF-8. May not be NULL. */,
    const char *type        /**< The service type for the new service, such as _http._tcp. May not be NULL. */,
    const char *domain      /**< The domain to register this domain in. We recommend to pass NULL here, to let the daemon decide */,
    const char *host        /**< The host this services is residing on. We recommend to pass NULL here, the daemon will than automatically insert the local host name in that case */,
    uint16_t port           /**< The IP port number of this service */,
    ...) AVAHI_GCC_SENTINEL;

/** Add a service, takes an AvahiStringList for TXT records. Arguments have the same meaning as for avahi_entry_group_add_service(). */
int avahi_entry_group_add_service_strlst(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    const char *host,
    uint16_t port,
    AvahiStringList *txt /**< The TXT data for this service. You may free this object after calling this function, it is not referenced any further */);

/** Add a subtype for a service. The service should already be existent in the entry group. You may add as many subtypes for a service as you wish. */
int avahi_entry_group_add_service_subtype(
    AvahiEntryGroup *group,
    AvahiIfIndex interface /**< The interface this subtype shall be announced on. This should match the value passed for the original avahi_entry_group_add_service() call. */,
    AvahiProtocol protocol /**< The protocol this subtype shall be announced with. This should match the value passed for the original avahi_entry_group_add_service() call. */,
    AvahiPublishFlags flags  /**< Only != 0 if you really know what you do */,
    const char *name         /**< The name of the service, as passed to avahi_entry_group_add_service(). May not be NULL. */,
    const char *type         /**< The type of the service, as passed to avahi_entry_group_add_service(). May not be NULL. */,
    const char *domain       /**< The domain this service resides is, as passed to avahi_entry_group_add_service(). May be NULL. */,
    const char *subtype /**< The new subtype to register for the specified service. May not be NULL. */);

/** Update a TXT record for an existing service. The service should already be existent in the entry group. */
int avahi_entry_group_update_service_txt(
    AvahiEntryGroup *g,
    AvahiIfIndex interface   /**< The interface this service is announced on. This should match the value passed to the original avahi_entry_group_add_service() call. */,
    AvahiProtocol protocol   /**< The protocol this service is announced with. This should match the value passed to the original avahi_entry_group_add_service() call. */,
    AvahiPublishFlags flags  /**< Only != 0 if you really know what you do */,
    const char *name         /**< The name of the service, as passed to avahi_entry_group_add_service(). May not be NULL. */,
    const char *type         /**< The type of the service, as passed to avahi_entry_group_add_service(). May not be NULL. */,
    const char *domain       /**< The domain this service resides is, as passed to avahi_entry_group_add_service(). May be NULL. */,
    ...) AVAHI_GCC_SENTINEL;

/** Update a TXT record for an existing service. Similar to avahi_entry_group_update_service_txt() but takes an AvahiStringList for the TXT strings, instead of a NULL terminated list of arguments. */
int avahi_entry_group_update_service_txt_strlst(
    AvahiEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    AvahiStringList *strlst);

/** \cond fulldocs */
/** Add a host/address pair */
int avahi_entry_group_add_address(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name /**< The FDQN of the new hostname to register */,
    const AvahiAddress *a /**< The address this host name shall map to */);
/** \endcond */

/** Add an arbitrary record. I hope you know what you do. */
int avahi_entry_group_add_record(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    uint16_t clazz,
    uint16_t type,
    uint32_t ttl,
    const void *rdata,
    size_t size);

/** @} */

AVAHI_C_DECL_END

#endif
