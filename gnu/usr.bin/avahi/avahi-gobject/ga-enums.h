/*
 * ga-enums.h
 * Copyright (C) 2006-2007 Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GA_ENUMS_H__
#define __GA_ENUMS_H__

#include <glib-object.h>
#include <avahi-common/defs.h>
#include <avahi-common/address.h>

G_BEGIN_DECLS
/** Values for GaProtocol */
        typedef enum {
    GA_PROTOCOL_INET = AVAHI_PROTO_INET,     /**< IPv4 */
    GA_PROTOCOL_INET6 = AVAHI_PROTO_INET6,   /**< IPv6 */
    GA_PROTOCOL_UNSPEC = AVAHI_PROTO_UNSPEC  /**< Unspecified/all protocol(s) */
} GaProtocol;


/** Some flags for lookup callback functions */
typedef enum {
    GA_LOOKUP_RESULT_CACHED = AVAHI_LOOKUP_RESULT_CACHED,     /**< This response originates from the cache */
    GA_LOOKUP_RESULT_WIDE_AREA = AVAHI_LOOKUP_RESULT_WIDE_AREA,
                                                              /**< This response originates from wide area DNS */
    GA_LOOKUP_RESULT_MULTICAST = AVAHI_LOOKUP_RESULT_MULTICAST,
                                                              /**< This response originates from multicast DNS */
    GA_LOOKUP_RESULT_LOCAL = AVAHI_LOOKUP_RESULT_LOCAL,       /**< This record/service resides on and was announced by the local host. Only available in service and record browsers and only on AVAHI_BROWSER_NEW. */
    GA_LOOKUP_RESULT_OUR_OWN = AVAHI_LOOKUP_RESULT_OUR_OWN,   /**< This service belongs to the same local client as the browser object. Only available in avahi-client, and only for service browsers and only on AVAHI_BROWSER_NEW. */
    GA_LOOKUP_RESULT_STATIC = AVAHI_LOOKUP_RESULT_STATIC      /**< The returned data has been defined statically by some configuration option */
} GaLookupResultFlags;

typedef enum {
    GA_LOOKUP_NO_FLAGS = 0,
    GA_LOOKUP_USE_WIDE_AREA = AVAHI_LOOKUP_USE_WIDE_AREA,    /**< Force lookup via wide area DNS */
    GA_LOOKUP_USE_MULTICAST = AVAHI_LOOKUP_USE_MULTICAST,    /**< Force lookup via multicast DNS */
    GA_LOOKUP_NO_TXT = AVAHI_LOOKUP_NO_TXT,                  /**< When doing service resolving, don't lookup TXT record */
    GA_LOOKUP_NO_ADDRESS = AVAHI_LOOKUP_NO_ADDRESS           /**< When doing service resolving, don't lookup A/AAAA record */
} GaLookupFlags;

typedef enum {
    GA_RESOLVER_FOUND = AVAHI_RESOLVER_FOUND,           /**< RR found, resolving successful */
    GA_RESOLVER_FAILURE = AVAHI_RESOLVER_FAILURE        /**< Resolving failed due to some reason which can be retrieved using avahi_server_errno()/avahi_client_errno() */
} GaResolverEvent;

typedef enum {
    GA_BROWSER_NEW = AVAHI_BROWSER_NEW,             /**< The object is new on the network */
    GA_BROWSER_REMOVE = AVAHI_BROWSER_REMOVE,                     /**< The object has been removed from the network */
    GA_BROWSER_CACHE_EXHAUSTED = AVAHI_BROWSER_CACHE_EXHAUSTED,   /**< One-time event, to notify the user that all entries from the caches have been send */
    GA_BROWSER_ALL_FOR_NOW = AVAHI_BROWSER_ALL_FOR_NOW,           /**< One-time event, to notify the user that more records will probably not show up in the near future, i.e. all cache entries have been read and all static servers been queried */
    GA_BROWSER_FAILURE = AVAHI_BROWSER_FAILURE                    /**< Browsing failed due to some reason which can be retrieved using avahi_server_errno()/avahi_client_errno() */
} GaBrowserEvent;

G_END_DECLS
#endif /* #ifndef __GA_CLIENT_H__ */
