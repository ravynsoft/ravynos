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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <avahi-common/error.h>
#include <avahi-common/dbus.h>

static const char * const table[- AVAHI_ERR_MAX] = {
    AVAHI_DBUS_ERR_OK,
    AVAHI_DBUS_ERR_FAILURE,
    AVAHI_DBUS_ERR_BAD_STATE,
    AVAHI_DBUS_ERR_INVALID_HOST_NAME,
    AVAHI_DBUS_ERR_INVALID_DOMAIN_NAME,
    AVAHI_DBUS_ERR_NO_NETWORK,
    AVAHI_DBUS_ERR_INVALID_TTL,
    AVAHI_DBUS_ERR_IS_PATTERN,
    AVAHI_DBUS_ERR_COLLISION,
    AVAHI_DBUS_ERR_INVALID_RECORD,

    AVAHI_DBUS_ERR_INVALID_SERVICE_NAME,
    AVAHI_DBUS_ERR_INVALID_SERVICE_TYPE,
    AVAHI_DBUS_ERR_INVALID_PORT,
    AVAHI_DBUS_ERR_INVALID_KEY,
    AVAHI_DBUS_ERR_INVALID_ADDRESS,
    AVAHI_DBUS_ERR_TIMEOUT,
    AVAHI_DBUS_ERR_TOO_MANY_CLIENTS,
    AVAHI_DBUS_ERR_TOO_MANY_OBJECTS,
    AVAHI_DBUS_ERR_TOO_MANY_ENTRIES,
    AVAHI_DBUS_ERR_OS,

    AVAHI_DBUS_ERR_ACCESS_DENIED,
    AVAHI_DBUS_ERR_INVALID_OPERATION,
    AVAHI_DBUS_ERR_DBUS_ERROR,
    AVAHI_DBUS_ERR_DISCONNECTED,
    AVAHI_DBUS_ERR_NO_MEMORY,
    AVAHI_DBUS_ERR_INVALID_OBJECT,
    AVAHI_DBUS_ERR_NO_DAEMON,
    AVAHI_DBUS_ERR_INVALID_INTERFACE,
    AVAHI_DBUS_ERR_INVALID_PROTOCOL,
    AVAHI_DBUS_ERR_INVALID_FLAGS,

    AVAHI_DBUS_ERR_NOT_FOUND,
    AVAHI_DBUS_ERR_INVALID_CONFIG,
    AVAHI_DBUS_ERR_VERSION_MISMATCH,
    AVAHI_DBUS_ERR_INVALID_SERVICE_SUBTYPE,
    AVAHI_DBUS_ERR_INVALID_PACKET,
    AVAHI_DBUS_ERR_INVALID_DNS_ERROR,
    AVAHI_DBUS_ERR_DNS_FORMERR,
    AVAHI_DBUS_ERR_DNS_SERVFAIL,
    AVAHI_DBUS_ERR_DNS_NXDOMAIN,
    AVAHI_DBUS_ERR_DNS_NOTIMP,

    AVAHI_DBUS_ERR_DNS_REFUSED,
    AVAHI_DBUS_ERR_DNS_YXDOMAIN,
    AVAHI_DBUS_ERR_DNS_YXRRSET,
    AVAHI_DBUS_ERR_DNS_NXRRSET,
    AVAHI_DBUS_ERR_DNS_NOTAUTH,
    AVAHI_DBUS_ERR_DNS_NOTZONE,
    AVAHI_DBUS_ERR_INVALID_RDATA,
    AVAHI_DBUS_ERR_INVALID_DNS_CLASS,
    AVAHI_DBUS_ERR_INVALID_DNS_TYPE,
    AVAHI_DBUS_ERR_NOT_SUPPORTED,

    AVAHI_DBUS_ERR_NOT_PERMITTED,
    AVAHI_DBUS_ERR_INVALID_ARGUMENT,
    AVAHI_DBUS_ERR_IS_EMPTY,
    AVAHI_DBUS_ERR_NO_CHANGE
};

struct error_map {
    const char *dbus_error;
    int avahi_error;
};

static struct error_map error_map[] = {
    { DBUS_ERROR_FAILED,           AVAHI_ERR_FAILURE },
    { DBUS_ERROR_NO_MEMORY,        AVAHI_ERR_NO_MEMORY },
    { DBUS_ERROR_SERVICE_UNKNOWN,  AVAHI_ERR_NO_DAEMON },
    { DBUS_ERROR_BAD_ADDRESS,      AVAHI_ERR_NO_DAEMON },
    { DBUS_ERROR_NOT_SUPPORTED,    AVAHI_ERR_NOT_SUPPORTED },
    { DBUS_ERROR_LIMITS_EXCEEDED,  AVAHI_ERR_TOO_MANY_OBJECTS },
    { DBUS_ERROR_ACCESS_DENIED,    AVAHI_ERR_ACCESS_DENIED },
    { DBUS_ERROR_AUTH_FAILED,      AVAHI_ERR_ACCESS_DENIED },
    { DBUS_ERROR_NO_SERVER,        AVAHI_ERR_NO_DAEMON },
    { DBUS_ERROR_TIMEOUT,          AVAHI_ERR_TIMEOUT },
    { DBUS_ERROR_NO_NETWORK,       AVAHI_ERR_NO_NETWORK },
    { DBUS_ERROR_DISCONNECTED,     AVAHI_ERR_DISCONNECTED },
    { DBUS_ERROR_INVALID_ARGS,     AVAHI_ERR_INVALID_ARGUMENT },
    { DBUS_ERROR_TIMED_OUT,        AVAHI_ERR_TIMEOUT },
    { NULL, 0 }
};

int avahi_error_dbus_to_number(const char *s) {
    int e;
    const struct error_map *m;

    assert(s);

    for (e = -1; e > AVAHI_ERR_MAX; e--)
        if (strcmp(s, table[-e]) == 0)
            return e;

    for (m = error_map; m->dbus_error; m++)
        if (strcmp(m->dbus_error, s) == 0)
            return m->avahi_error;

    return AVAHI_ERR_DBUS_ERROR;
}

const char *avahi_error_number_to_dbus(int error) {
    assert(error > AVAHI_ERR_MAX);
    assert(error < 1);

    return table[-error];
}
