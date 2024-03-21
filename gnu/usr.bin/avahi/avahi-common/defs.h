#ifndef foodefshfoo
#define foodefshfoo

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

/** \file defs.h Some common definitions */

#include <avahi-common/cdecl.h>

/** \mainpage
 *
 * \section choose_api Choosing an API
 *
 * Avahi provides three programming APIs for integration of
 * mDNS/DNS-SD features into your C progams:
 *
 * \li <b>avahi-core</b>: an API for embedding a complete mDNS/DNS-SD stack
 * into your software. This is intended for developers of embedded
 * appliances only. We dissuade from using this API in normal desktop
 * applications since it is not a good idea to run multiple mDNS
 * stacks simultaneously on the same host.
 * \li <b>the D-Bus API</b>: an extensive D-Bus interface for browsing and
 * registering mDNS/DNS-SD services using avahi-daemon. We recommend
 * using this API for software written in any language other than
 * C (e.g. Python).
 * \li <b>avahi-client</b>: a simplifying C wrapper around the D-Bus API. We
 * recommend using this API in C or C++ progams. The D-Bus internals
 * are hidden completely.
 * \li <b>avahi-gobject</b>: an object-oriented C wrapper based on
 * GLib's GObject. We recommd using this API for GNOME/Gtk programs.
 *
 * All three APIs are very similar, however avahi-core is the most powerful.
 *
 * In addition to the three APIs described above Avahi supports two
 * compatibility libraries:
 *
 * \li <b>avahi-compat-libdns_sd</b>: the original Bonjour API as documented
 * in the header file "dns_sd.h" by Apple Computer, Inc.
 *
 * \li <b>avahi-compat-howl</b>: the HOWL API as released with HOWL 0.9.8 by
 * Porchdog Software.
 *
 * Please note that these compatibility layers are incomplete and
 * generally a waste of resources. We strongly encourage everyone to
 * use our native APIs for newly written programs and to port older
 * programs to avahi-client!
 *
 * The native APIs (avahi-client and avahi-core) can be integrated
 * into external event loops. We provide adapters for the following
 * event loop implementations:
 *
 * \li <b>avahi-glib</b>: The GLIB main loop as used by GTk+/GNOME
 *
 * \li <b>avahi-qt</b>: The Qt main loop as used by Qt/KDE
 *
 * Finally, we provide a high-level Gtk+ GUI dialog called
 * <b>avahi-ui</b> for user-friendly browsing for services.
 *
 * The doxygen-generated API documentation covers avahi-client
 * (including its auxiliary APIs), the event loop adapters and
 * avahi-ui. For the other APIs please consult the original
 * documentation (for the compatibility APIs) or the header files.
 *
 * Please note that the doxygen-generated API documentation of the
 * native Avahi API is not complete. A few definitions that are part
 * of the Avahi API have been removed from this documentation, either
 * because they are only relevant in a very few low-level applications
 * or because they are considered obsolete. Please consult the C header
 * files for all definitions that are part of the Avahi API. Please
 * note that these hidden definitions are considered part of the Avahi
 * API and will stay available in the API in the future.
 *
 * \section error_reporting Error Reporting
 *
 * Some notes on the Avahi error handling:
 *
 * - Error codes are negative integers and defined as AVAHI_ERR_xx
 * - If a function returns some kind of non-negative integer value on
 * success, a failure is indicated by returning the error code
 * directly.
 * - If a function returns a pointer of some kind on success, a
 * failure is indicated by returning NULL
 * - The last error number may be retrieved by calling
 * avahi_client_errno()
 * - Just like the libc errno variable the Avahi errno is NOT reset to
 * AVAHI_OK if a function call succeeds.
 * - You may convert a numeric error code into a human readable string
 * using avahi_strerror()
 * - The constructor function avahi_client_new() returns the error
 * code in a call-by-reference argument
 *
 * \section event_loop Event Loop Abstraction
 *
 * Avahi uses a simple event loop abstraction layer. A table AvahiPoll
 * which contains function pointers for user defined timeout and I/O
 * condition event source implementations needs to be passed to
 * avahi_client_new(). An adapter for this abstraction layer is
 * available for the GLib main loop in the object AvahiGLibPoll. A
 * simple stand-alone implementation is available under the name
 * AvahiSimplePoll. An adpater for the Qt main loop is available from
 * avahi_qt_poll_get().
 *
 * \section good_publish How to Register Services
 *
 * - Subscribe to server state changes. Pass a callback function
 * pointer to avahi_client_new(). It will be called
 * whenever the server state changes.
 * - Only register your services when the server is in state
 * AVAHI_SERVER_RUNNING. If you register your services in other server
 * states they might not be accessible since the local host name might not necessarily
 * be established.
 * - Remove your services when the server enters
 * AVAHI_SERVER_COLLISION or AVAHI_SERVER_REGISTERING state. Your
 * services may not be reachable anymore since the local host name is
 * no longer established or is currently in the process of being
 * established.
 * - When registering services, use the following algorithm:
 *   - Create a new entry group (i.e. avahi_entry_group_new())
 *   - Add your service(s)/additional RRs/subtypes (e.g. avahi_entry_group_add_service())
 *   - Commit the entry group (i.e. avahi_entry_group_commit())
 * - Subscribe to entry group state changes.
 * - If the entry group enters AVAHI_ENTRY_GROUP_COLLISION state the
 * services of the entry group are automatically removed from the
 * server. You may immediately add your services back to the entry
 * group (but with new names, perhaps using
 * avahi_alternative_service_name()) and commit again. Please do not
 * free the entry group and create a new one. This would inhibit some
 * traffic limiting algorithms in mDNS.
 * - When you need to modify your services (i.e. change the TXT data
 * or the port number), use the AVAHI_PUBLISH_UPDATE flag. Please do
 * not free the entry group and create a new one. This would inhibit
 * some traffic limiting algorithms in mDNS. When changing just the
 * TXT data avahi_entry_group_update_txt() is a shortcut for
 * AVAHI_PUBLISH_UPDATE. Please note that you cannot use
 * AVAHI_PUBLISH_UPDATE when changing the service name! Renaming a
 * DNS-SD service is identical to deleting and creating a new one, and
 * that's exactly what you should do in that case. First call
 * avahi_entry_group_reset() to remove it and then read it normally.
 *
 * \section good_browse How to Browse for Services
 *
 * - For normal applications you need to call avahi_service_browser_new()
 * for the service type you want to browse for. Use
 * avahi_service_resolver_new() to acquire service data for a service
 * name.
 * - You can use avahi_domain_browser_new() to get a list of announced
 * browsing domains. Please note that not all domains whith services
 * on the LAN are mandatorily announced.
 * - There is no need to subscribe to server state changes.
 *
 * \section daemon_dies How to Write a Client That Can Deal with Daemon Restarts
 *
 * With Avahi it is possible to write client applications that can
 * deal with Avahi daemon restarts. To accomplish that make sure to
 * pass AVAHI_CLIENT_NO_FAIL to avahi_client_new()'s flags
 * parameter. That way avahi_client_new() will succeed even when the
 * daemon is not running. In that case the object will enter
 * AVAHI_CLIENT_CONNECTING state. As soon as the daemon becomes
 * available the object will enter one of the AVAHI_CLIENT_S_xxx
 * states. Make sure to not create browsers or entry groups before the
 * client object has entered one of those states. As usual you will be
 * informed about state changes with the callback function supplied to
 * avahi_client_new(). If the client is forced to disconnect from the
 * server it will enter AVAHI_CLIENT_FAILURE state with
 * avahi_client_errno() == AVAHI_ERR_DISCONNECTED. Free the
 * AvahiClient object in that case (and all its associated objects
 * such as entry groups and browser objects prior to that) and
 * reconnect to the server anew - again with passing
 * AVAHI_CLIENT_NO_FAIL to avahi_client_new().
 *
 * We encourage implementing this in all software where service
 * discovery is not an integral part of application. e.g. use it in
 * all kinds of background daemons, but not necessarily in software
 * like iChat compatible IM software.
 *
 * For now AVAHI_CLIENT_NO_FAIL cannot deal with D-Bus daemon restarts.
 *
 * \section domains How to Deal Properly with Browsing Domains
 *
 * Due to the introduction of wide-area DNS-SD the correct handling of
 * domains becomes more important for Avahi enabled applications. All
 * applications that offer the user a list of services discovered with
 * Avahi should offer some kind of editable drop down box where the
 * user can either enter his own domain or select one of those offered
 * by AvahiDomainBrowser. The default domain to browse should be the
 * one returned by avahi_client_get_domain_name(). The list of domains
 * returned by AvahiDomainBrowser is assembled by the browsing domains
 * configured in the daemon's configuration file, the domains
 * announced inside the default domain, the domains set with the
 * environment variable $AVAHI_BROWSE_DOMAINS (colon-seperated) on the
 * client side and the domains set in the XDG configuration file
 * ~/.config/avahi/browse-domains on the client side (seperated by
 * newlines). File managers offering some kind of "Network
 * Neighborhood" folder should show the entries of the default domain
 * right inside that and offer subfolders for the browsing domains
 * returned by AvahiDomainBrowser.
 */

AVAHI_C_DECL_BEGIN

/** @{ \name States */

/** States of a server object */
typedef enum {
    AVAHI_SERVER_INVALID,          /**< Invalid state (initial) */
    AVAHI_SERVER_REGISTERING,      /**< Host RRs are being registered */
    AVAHI_SERVER_RUNNING,          /**< All host RRs have been established */
    AVAHI_SERVER_COLLISION,        /**< There is a collision with a host RR. All host RRs have been withdrawn, the user should set a new host name via avahi_server_set_host_name() */
    AVAHI_SERVER_FAILURE           /**< Some fatal failure happened, the server is unable to proceed */
} AvahiServerState;

/** States of an entry group object */
typedef enum {
    AVAHI_ENTRY_GROUP_UNCOMMITED,    /**< The group has not yet been commited, the user must still call avahi_entry_group_commit() */
    AVAHI_ENTRY_GROUP_REGISTERING,   /**< The entries of the group are currently being registered */
    AVAHI_ENTRY_GROUP_ESTABLISHED,   /**< The entries have successfully been established */
    AVAHI_ENTRY_GROUP_COLLISION,     /**< A name collision for one of the entries in the group has been detected, the entries have been withdrawn */
    AVAHI_ENTRY_GROUP_FAILURE        /**< Some kind of failure happened, the entries have been withdrawn */
} AvahiEntryGroupState;

/** @} */

/** @{ \name Flags */

/** Some flags for publishing functions */
typedef enum {
    AVAHI_PUBLISH_UNIQUE = 1,           /**< For raw records: The RRset is intended to be unique */
    AVAHI_PUBLISH_NO_PROBE = 2,         /**< For raw records: Though the RRset is intended to be unique no probes shall be sent */
    AVAHI_PUBLISH_NO_ANNOUNCE = 4,      /**< For raw records: Do not announce this RR to other hosts */
    AVAHI_PUBLISH_ALLOW_MULTIPLE = 8,   /**< For raw records: Allow multiple local records of this type, even if they are intended to be unique */
/** \cond fulldocs */
    AVAHI_PUBLISH_NO_REVERSE = 16,      /**< For address records: don't create a reverse (PTR) entry */
    AVAHI_PUBLISH_NO_COOKIE = 32,       /**< For service records: do not implicitly add the local service cookie to TXT data */
/** \endcond */
    AVAHI_PUBLISH_UPDATE = 64,          /**< Update existing records instead of adding new ones */
/** \cond fulldocs */
    AVAHI_PUBLISH_USE_WIDE_AREA = 128,  /**< Register the record using wide area DNS (i.e. unicast DNS update) */
    AVAHI_PUBLISH_USE_MULTICAST = 256   /**< Register the record using multicast DNS */
/** \endcond */
} AvahiPublishFlags;

/** Some flags for lookup functions */
typedef enum {
/** \cond fulldocs */
    AVAHI_LOOKUP_USE_WIDE_AREA = 1,    /**< Force lookup via wide area DNS */
    AVAHI_LOOKUP_USE_MULTICAST = 2,    /**< Force lookup via multicast DNS */
/** \endcond */
    AVAHI_LOOKUP_NO_TXT = 4,           /**< When doing service resolving, don't lookup TXT record */
    AVAHI_LOOKUP_NO_ADDRESS = 8        /**< When doing service resolving, don't lookup A/AAAA record */
} AvahiLookupFlags;

/** Some flags for lookup callback functions */
typedef enum {
    AVAHI_LOOKUP_RESULT_CACHED = 1,         /**< This response originates from the cache */
    AVAHI_LOOKUP_RESULT_WIDE_AREA = 2,      /**< This response originates from wide area DNS */
    AVAHI_LOOKUP_RESULT_MULTICAST = 4,      /**< This response originates from multicast DNS */
    AVAHI_LOOKUP_RESULT_LOCAL = 8,          /**< This record/service resides on and was announced by the local host. Only available in service and record browsers and only on AVAHI_BROWSER_NEW. */
    AVAHI_LOOKUP_RESULT_OUR_OWN = 16,       /**< This service belongs to the same local client as the browser object. Only available in avahi-client, and only for service browsers and only on AVAHI_BROWSER_NEW. */
    AVAHI_LOOKUP_RESULT_STATIC = 32         /**< The returned data has been defined statically by some configuration option */
} AvahiLookupResultFlags;

/** @} */

/** @{ \name Events */

/** Type of callback event when browsing */
typedef enum {
    AVAHI_BROWSER_NEW,               /**< The object is new on the network */
    AVAHI_BROWSER_REMOVE,            /**< The object has been removed from the network */
    AVAHI_BROWSER_CACHE_EXHAUSTED,   /**< One-time event, to notify the user that all entries from the caches have been sent */
    AVAHI_BROWSER_ALL_FOR_NOW,       /**< One-time event, to notify the user that more records will probably not show up in the near future, i.e. all cache entries have been read and all static servers been queried */
    AVAHI_BROWSER_FAILURE            /**< Browsing failed due to some reason which can be retrieved using avahi_server_errno()/avahi_client_errno() */
} AvahiBrowserEvent;

/** Type of callback event when resolving */
typedef enum {
    AVAHI_RESOLVER_FOUND,          /**< RR found, resolving successful */
    AVAHI_RESOLVER_FAILURE         /**< Resolving failed due to some reason which can be retrieved using avahi_server_errno()/avahi_client_errno() */
} AvahiResolverEvent;

/** @} */

/** @{ \name Other definitions */

/** The type of domain to browse for */
typedef enum {
    AVAHI_DOMAIN_BROWSER_BROWSE,            /**< Browse for a list of available browsing domains */
    AVAHI_DOMAIN_BROWSER_BROWSE_DEFAULT,    /**< Browse for the default browsing domain */
    AVAHI_DOMAIN_BROWSER_REGISTER,          /**< Browse for a list of available registering domains */
    AVAHI_DOMAIN_BROWSER_REGISTER_DEFAULT,  /**< Browse for the default registering domain */
    AVAHI_DOMAIN_BROWSER_BROWSE_LEGACY,     /**< Legacy browse domain - see DNS-SD spec for more information */
    AVAHI_DOMAIN_BROWSER_MAX
} AvahiDomainBrowserType;

/** @} */

/** \cond fulldocs */
/** For every service a special TXT item is implicitly added, which
 * contains a random cookie which is private to the local daemon. This
 * can be used by clients to determine if two services on two
 * different subnets are effectively the same. */
#define AVAHI_SERVICE_COOKIE "org.freedesktop.Avahi.cookie"

/** In invalid cookie as special value */
#define AVAHI_SERVICE_COOKIE_INVALID (0)
/** \endcond fulldocs */

/** @{ \name DNS RR definitions */

/** DNS record types, see RFC 1035 */
enum {
    AVAHI_DNS_TYPE_A = 0x01,
    AVAHI_DNS_TYPE_NS = 0x02,
    AVAHI_DNS_TYPE_CNAME = 0x05,
    AVAHI_DNS_TYPE_SOA = 0x06,
    AVAHI_DNS_TYPE_PTR = 0x0C,
    AVAHI_DNS_TYPE_HINFO = 0x0D,
    AVAHI_DNS_TYPE_MX = 0x0F,
    AVAHI_DNS_TYPE_TXT = 0x10,
    AVAHI_DNS_TYPE_AAAA = 0x1C,
    AVAHI_DNS_TYPE_SRV = 0x21
};

/** DNS record classes, see RFC 1035 */
enum {
    AVAHI_DNS_CLASS_IN = 0x01          /**< Probably the only class we will ever use */
};

/** @} */

/** The default TTL for RRs which contain a host name of some kind. */
#define AVAHI_DEFAULT_TTL_HOST_NAME (120)

/** The default TTL for all other records. */
#define AVAHI_DEFAULT_TTL (75*60)

AVAHI_C_DECL_END

#endif
