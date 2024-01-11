#ifndef foocorehfoo
#define foocorehfoo

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

/** \file core.h The Avahi Multicast DNS and DNS Service Discovery implementation. */

/** An mDNS responder object */
typedef struct AvahiServer AvahiServer;

#include <avahi-common/cdecl.h>
#include <avahi-common/address.h>
#include <avahi-common/defs.h>
#include <avahi-common/watch.h>
#include <avahi-common/timeval.h>
#include <avahi-core/rr.h>

AVAHI_C_DECL_BEGIN

/** Maximum number of defined DNS servers for wide area DNS */
#define AVAHI_WIDE_AREA_SERVERS_MAX 4

/** Prototype for callback functions which are called whenever the state of an AvahiServer object changes */
typedef void (*AvahiServerCallback) (AvahiServer *s, AvahiServerState state, void* userdata);

/** Stores configuration options for a server instance */
typedef struct AvahiServerConfig {
    char *host_name;                  /**< Default host name. If left empty defaults to the result of gethostname(2) of the libc */
    char *domain_name;                /**< Default domain name. If left empty defaults to .local */
    int use_ipv4;                     /**< Enable IPv4 support */
    int use_ipv6;                     /**< Enable IPv6 support */
    AvahiStringList *allow_interfaces;/**< Allow specific interface to be used for Avahi */
    AvahiStringList *deny_interfaces; /**< Deny specific interfaces to be used for Avahi */
    int publish_hinfo;                /**< Register a HINFO record for the host containing the local OS and CPU type */
    int publish_addresses;            /**< Register A, AAAA and PTR records for all local IP addresses */
    int publish_workstation;          /**< Register a _workstation._tcp service */
    int publish_domain;               /**< Announce the local domain for browsing */
    int check_response_ttl;           /**< If enabled the server ignores all incoming responses with IP TTL != 255. Newer versions of the RFC do no longer contain this check, so it is disabled by default. */
    int use_iff_running;              /**< Require IFF_RUNNING on local network interfaces. This is the official way to check for link beat. Unfortunately this doesn't work with all drivers. So bettere leave this off. */
    int enable_reflector;             /**< Reflect incoming mDNS traffic to all local networks. This allows mDNS based network browsing beyond ethernet borders */
    int reflect_ipv;                  /**< if enable_reflector is 1, enable/disable reflecting between IPv4 and IPv6 */
    AvahiStringList *reflect_filters;  /**< if enable_reflector is 1, will only add services containing one of these strings */
    int add_service_cookie;           /**< Add magic service cookie to all locally generated records implicitly */
    int enable_wide_area;             /**< Enable wide area support */
    AvahiAddress wide_area_servers[AVAHI_WIDE_AREA_SERVERS_MAX]; /** Unicast DNS server to use for wide area lookup */
    unsigned n_wide_area_servers;     /**< Number of servers in wide_area_servers[] */
    int disallow_other_stacks;        /**< Make sure that only one mDNS responder is run at the same time on the local machine. If this is enable Avahi will not set SO_REUSADDR on its sockets, effectively preventing other stacks from running on the local machine */
    AvahiStringList *browse_domains;  /**< Additional browsing domains */
    int disable_publishing;           /**< Disable publishing of any record */
    int allow_point_to_point;         /**< Enable publishing on POINTOPOINT interfaces */
    int publish_a_on_ipv6;            /**< Publish an IPv4 A RR on IPv6 sockets */
    int publish_aaaa_on_ipv4;         /**< Publish an IPv6 AAAA RR on IPv4 sockets */
    unsigned n_cache_entries_max;     /**< Maximum number of cache entries per interface */
    AvahiUsec ratelimit_interval;     /**< If non-zero, rate-limiting interval parameter. */
    unsigned ratelimit_burst;         /**< If ratelimit_interval is non-zero, rate-limiting burst parameter. */
} AvahiServerConfig;

/** Allocate a new mDNS responder object. */
AvahiServer *avahi_server_new(
    const AvahiPoll *api,          /**< The main loop adapter */
    const AvahiServerConfig *sc,   /**< If non-NULL a pointer to a configuration structure for the server. The server makes an internal deep copy of this structure, so you may free it using avahi_server_config_done() immediately after calling this function. */
    AvahiServerCallback callback,  /**< A callback which is called whenever the state of the server changes */
    void* userdata,                /**< An opaque pointer which is passed to the callback function */
    int *error);

/** Free an mDNS responder object */
void avahi_server_free(AvahiServer* s);

/** Fill in default values for a server configuration structure. If you
 * make use of an AvahiServerConfig structure be sure to initialize
 * it with this function for the sake of upwards library
 * compatibility. This call may allocate strings on the heap. To
 * release this memory make sure to call
 * avahi_server_config_done(). If you want to replace any strings in
 * the structure be sure to free the strings filled in by this
 * function with avahi_free() first and allocate the replacements with
 * g_malloc() (or g_strdup()).*/
AvahiServerConfig* avahi_server_config_init(
   AvahiServerConfig *c /**< A structure which shall be filled in */ );

/** Make a deep copy of the configuration structure *c to *ret. */
AvahiServerConfig* avahi_server_config_copy(
    AvahiServerConfig *ret /**< destination */,
    const AvahiServerConfig *c /**< source */);

/** Free the data in a server configuration structure. */
void avahi_server_config_free(AvahiServerConfig *c);

/** Return the currently chosen domain name of the server object. The
 * return value points to an internally allocated string. Be sure to
 * make a copy of the string before calling any other library
 * functions. */
const char* avahi_server_get_domain_name(AvahiServer *s);

/** Return the currently chosen host name. The return value points to a internally allocated string. */
const char* avahi_server_get_host_name(AvahiServer *s);

/** Return the currently chosen host name as a FQDN ("fully qualified
 * domain name", i.e. the concatenation of the host and domain
 * name). The return value points to a internally allocated string. */
const char* avahi_server_get_host_name_fqdn(AvahiServer *s);

/** Change the host name of a running mDNS responder. This will drop
all automicatilly generated RRs and readd them with the new
name. Since the responder has to probe for the new RRs this function
takes some time to take effect altough it returns immediately. This
function is intended to be called when a host name conflict is
reported using AvahiServerCallback. The caller should readd all user
defined RRs too since they otherwise continue to point to the outdated
host name..*/
int avahi_server_set_host_name(AvahiServer *s, const char *host_name);

/** Change the domain name of a running mDNS responder. The same rules
 * as with avahi_server_set_host_name() apply. */
int avahi_server_set_domain_name(AvahiServer *s, const char *domain_name);

/** Return the opaque user data pointer attached to a server object */
void* avahi_server_get_data(AvahiServer *s);

/** Change the opaque user data pointer attached to a server object */
void avahi_server_set_data(AvahiServer *s, void* userdata);

/** Return the current state of the server object */
AvahiServerState avahi_server_get_state(AvahiServer *s);

/** Callback prototype for avahi_server_dump() */
typedef void (*AvahiDumpCallback)(const char *text, void* userdata);

/** Dump the current server status by calling "callback" for each line.  */
int avahi_server_dump(AvahiServer *s, AvahiDumpCallback callback, void* userdata);

/** Return the last error code */
int avahi_server_errno(AvahiServer *s);

/** Return the local service cookie */
uint32_t avahi_server_get_local_service_cookie(AvahiServer *s);

/** Set the wide area DNS servers */
int avahi_server_set_wide_area_servers(AvahiServer *s, const AvahiAddress *a, unsigned n);

/** Set the browsing domains */
int avahi_server_set_browse_domains(AvahiServer *s, AvahiStringList *domains);

/** Return the current configuration of the server \since 0.6.17 */
const AvahiServerConfig* avahi_server_get_config(AvahiServer *s);

AVAHI_C_DECL_END

#endif
