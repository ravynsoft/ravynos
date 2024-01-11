#ifndef foodnssrvhfoo
#define foodnssrvhfoo

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

/** \file avahi-core/dns-srv-rr.h Functions for announcing and browsing for unicast DNS servers via mDNS */

/** A domain service browser object. Use this to browse for
 * conventional unicast DNS servers which may be used to resolve
 * conventional domain names */
typedef struct AvahiSDNSServerBrowser AvahiSDNSServerBrowser;

#include <avahi-common/cdecl.h>
#include <avahi-common/defs.h>
#include <avahi-core/core.h>
#include <avahi-core/publish.h>

AVAHI_C_DECL_BEGIN

/** The type of DNS server */
typedef enum {
    AVAHI_DNS_SERVER_RESOLVE,         /**< Unicast DNS servers for normal resolves (_domain._udp)*/
    AVAHI_DNS_SERVER_UPDATE,           /**< Unicast DNS servers for updates (_dns-update._udp)*/
    AVAHI_DNS_SERVER_MAX
} AvahiDNSServerType;

/** Publish the specified unicast DNS server address via mDNS. You may
 * browse for records create this way wit
 * avahi_s_dns_server_browser_new(). */
int avahi_server_add_dns_server_address(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *domain,
    AvahiDNSServerType type,
    const AvahiAddress *address,
    uint16_t port /** should be 53 */);

/** Callback prototype for AvahiSDNSServerBrowser events */
typedef void (*AvahiSDNSServerBrowserCallback)(
    AvahiSDNSServerBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *host_name,       /**< Host name of the DNS server, probably useless */
    const AvahiAddress *a,        /**< Address of the DNS server */
    uint16_t port,                 /**< Port number of the DNS servers, probably 53 */
    AvahiLookupResultFlags flags,  /**< Lookup flags */
    void* userdata);

/** Create a new AvahiSDNSServerBrowser object */
AvahiSDNSServerBrowser *avahi_s_dns_server_browser_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiDNSServerType type,
    AvahiProtocol aprotocol,  /**< Address protocol for the DNS server */
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSDNSServerBrowserCallback callback,
    void* userdata);

/** Prepare a new AvahiSDNSServerBrowser object */
AvahiSDNSServerBrowser *avahi_s_dns_server_browser_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiDNSServerType type,
    AvahiProtocol aprotocol,  /**< Address protocol for the DNS server */
    AvahiLookupFlags flags,                 /**< Lookup flags. */
    AvahiSDNSServerBrowserCallback callback,
    void* userdata);

/** Start querying on an AvahiSDNSBrowser object */
void avahi_s_dns_server_browser_start(AvahiSDNSServerBrowser *b);

/** Free an AvahiSDNSServerBrowser object */
void avahi_s_dns_server_browser_free(AvahiSDNSServerBrowser *b);

AVAHI_C_DECL_END

#endif
