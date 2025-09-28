#ifndef foodbushfoo
#define foodbushfoo

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

/** \file dbus.h Some definitions for the D-Bus interface */

#include <avahi-common/cdecl.h>
#include <dbus/dbus.h>

AVAHI_C_DECL_BEGIN

#define AVAHI_DBUS_NAME "org.freedesktop.Avahi"
#define AVAHI_DBUS_INTERFACE_SERVER AVAHI_DBUS_NAME".Server"
#define AVAHI_DBUS_INTERFACE_SERVER2 AVAHI_DBUS_NAME".Server2"
#define AVAHI_DBUS_PATH_SERVER "/"
#define AVAHI_DBUS_INTERFACE_ENTRY_GROUP AVAHI_DBUS_NAME".EntryGroup"
#define AVAHI_DBUS_INTERFACE_DOMAIN_BROWSER AVAHI_DBUS_NAME".DomainBrowser"
#define AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER AVAHI_DBUS_NAME".ServiceTypeBrowser"
#define AVAHI_DBUS_INTERFACE_SERVICE_BROWSER AVAHI_DBUS_NAME".ServiceBrowser"
#define AVAHI_DBUS_INTERFACE_ADDRESS_RESOLVER AVAHI_DBUS_NAME".AddressResolver"
#define AVAHI_DBUS_INTERFACE_HOST_NAME_RESOLVER AVAHI_DBUS_NAME".HostNameResolver"
#define AVAHI_DBUS_INTERFACE_SERVICE_RESOLVER AVAHI_DBUS_NAME".ServiceResolver"
#define AVAHI_DBUS_INTERFACE_RECORD_BROWSER AVAHI_DBUS_NAME".RecordBrowser"

/** The D-Bus API version identifier. The first byte specifies the API
release, the second byte specifies the revision. If the revision
number is increased the API has been extended but is downwards
compatible. If the release changes compatibility is lost.

Avahi 0.6 implements API version 0x0201;
Avahi 0.6.1 implements API version 0x0202
Avahi 0.7 implements API version 0x0203 */
#define AVAHI_DBUS_API_VERSION ((uint32_t) 0x0204)

#define AVAHI_DBUS_ERR_OK "org.freedesktop.Avahi.Success"
#define AVAHI_DBUS_ERR_FAILURE "org.freedesktop.Avahi.Failure"
#define AVAHI_DBUS_ERR_BAD_STATE "org.freedesktop.Avahi.BadStateError"
#define AVAHI_DBUS_ERR_INVALID_HOST_NAME "org.freedesktop.Avahi.InvalidHostNameError"
#define AVAHI_DBUS_ERR_INVALID_DOMAIN_NAME "org.freedesktop.Avahi.InvalidDomainNameError"
#define AVAHI_DBUS_ERR_NO_NETWORK "org.freedesktop.Avahi.NoNetworkError"
#define AVAHI_DBUS_ERR_INVALID_TTL "org.freedesktop.Avahi.InvalidTTLError"
#define AVAHI_DBUS_ERR_IS_PATTERN "org.freedesktop.Avahi.IsPatternError"
#define AVAHI_DBUS_ERR_COLLISION "org.freedesktop.Avahi.CollisionError"
#define AVAHI_DBUS_ERR_INVALID_RECORD "org.freedesktop.Avahi.InvalidRecordError"

#define AVAHI_DBUS_ERR_INVALID_SERVICE_NAME "org.freedesktop.Avahi.InvalidServiceNameError"
#define AVAHI_DBUS_ERR_INVALID_SERVICE_TYPE "org.freedesktop.Avahi.InvalidServiceTypeError"
#define AVAHI_DBUS_ERR_INVALID_PORT "org.freedesktop.Avahi.InvalidPortError"
#define AVAHI_DBUS_ERR_INVALID_KEY "org.freedesktop.Avahi.InvalidKeyError"
#define AVAHI_DBUS_ERR_INVALID_ADDRESS "org.freedesktop.Avahi.InvalidAddressError"
#define AVAHI_DBUS_ERR_TIMEOUT "org.freedesktop.Avahi.TimeoutError"
#define AVAHI_DBUS_ERR_TOO_MANY_CLIENTS "org.freedesktop.Avahi.TooManyClientsError"
#define AVAHI_DBUS_ERR_TOO_MANY_OBJECTS "org.freedesktop.Avahi.TooManyObjectsError"
#define AVAHI_DBUS_ERR_TOO_MANY_ENTRIES "org.freedesktop.Avahi.TooManyEntriesError"
#define AVAHI_DBUS_ERR_OS "org.freedesktop.Avahi.OSError"

#define AVAHI_DBUS_ERR_ACCESS_DENIED DBUS_ERROR_ACCESS_DENIED
#define AVAHI_DBUS_ERR_INVALID_OPERATION "org.freedesktop.Avahi.InvalidOperationError"
#define AVAHI_DBUS_ERR_DBUS_ERROR "org.freedesktop.Avahi.DBusError"
#define AVAHI_DBUS_ERR_DISCONNECTED "org.freedesktop.Avahi.DisconnectedError"
#define AVAHI_DBUS_ERR_NO_MEMORY "org.freedesktop.Avahi.NoMemoryError"
#define AVAHI_DBUS_ERR_INVALID_OBJECT "org.freedesktop.Avahi.InvalidObjectError"
#define AVAHI_DBUS_ERR_NO_DAEMON "org.freedesktop.Avahi.NoDaemonError"
#define AVAHI_DBUS_ERR_INVALID_INTERFACE "org.freedesktop.Avahi.InvalidInterfaceError"
#define AVAHI_DBUS_ERR_INVALID_PROTOCOL "org.freedesktop.Avahi.InvalidInterfaceProtocolError"
#define AVAHI_DBUS_ERR_INVALID_FLAGS "org.freedesktop.Avahi.InvalidFlagsError"

#define AVAHI_DBUS_ERR_NOT_FOUND "org.freedesktop.Avahi.NotFoundError"
#define AVAHI_DBUS_ERR_INVALID_CONFIG "org.freedesktop.Avahi.InvalidConfigurationError"
#define AVAHI_DBUS_ERR_VERSION_MISMATCH "org.freedesktop.Avahi.VersionMismatchError"
#define AVAHI_DBUS_ERR_INVALID_SERVICE_SUBTYPE "org.freedesktop.Avahi.InvalidServiceSubtypeError"
#define AVAHI_DBUS_ERR_INVALID_PACKET "org.freedesktop.Avahi.InvalidPacketError"
#define AVAHI_DBUS_ERR_INVALID_DNS_ERROR "org.freedesktop.Avahi.InvalidDNSErrorError"
#define AVAHI_DBUS_ERR_DNS_FORMERR "org.freedesktop.Avahi.DNSFORMERR"
#define AVAHI_DBUS_ERR_DNS_SERVFAIL "org.freedesktop.Avahi.DNSSERVFAIL"
#define AVAHI_DBUS_ERR_DNS_NXDOMAIN "org.freedesktop.Avahi.DNSNXDOMAIN"
#define AVAHI_DBUS_ERR_DNS_NOTIMP "org.freedesktop.Avahi.DNSNOTIMP"

#define AVAHI_DBUS_ERR_DNS_REFUSED "org.freedesktop.Avahi.DNSREFUSED"
#define AVAHI_DBUS_ERR_DNS_YXDOMAIN "org.freedesktop.Avahi.DNSYXDOMAIN"
#define AVAHI_DBUS_ERR_DNS_YXRRSET "org.freedesktop.Avahi.DNSYXRRSET"
#define AVAHI_DBUS_ERR_DNS_NXRRSET "org.freedesktop.Avahi.DNSNXRRSET"
#define AVAHI_DBUS_ERR_DNS_NOTAUTH "org.freedesktop.Avahi.DNSNOTAUTH"
#define AVAHI_DBUS_ERR_DNS_NOTZONE "org.freedesktop.Avahi.DNSNOTZONE"
#define AVAHI_DBUS_ERR_INVALID_RDATA "org.freedesktop.Avahi.InvalidRDataError"
#define AVAHI_DBUS_ERR_INVALID_DNS_CLASS "org.freedesktop.Avahi.InvalidDNSClassError"
#define AVAHI_DBUS_ERR_INVALID_DNS_TYPE "org.freedesktop.Avahi.InvalidDNSTypeError"
#define AVAHI_DBUS_ERR_NOT_SUPPORTED "org.freedesktop.Avahi.NotSupportedError"

#define AVAHI_DBUS_ERR_NOT_PERMITTED "org.freedesktop.Avahi.NotPermittedError"
#define AVAHI_DBUS_ERR_INVALID_ARGUMENT "org.freedesktop.Avahi.InvalidArgumentError"
#define AVAHI_DBUS_ERR_IS_EMPTY "org.freedesktop.Avahi.IsEmptyError"
#define AVAHI_DBUS_ERR_NO_CHANGE "org.freedesktop.Avahi.NoChangeError"

/** Convert a DBus error string into an Avahi error number */
int avahi_error_dbus_to_number(const char *s);

/** Convert an Avahi error number into a DBus error string.  Result should not be freed */
const char * avahi_error_number_to_dbus(int error);

AVAHI_C_DECL_END

#endif
