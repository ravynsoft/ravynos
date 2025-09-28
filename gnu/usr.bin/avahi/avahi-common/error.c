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

#include "error.h"
#include "i18n.h"

const char *avahi_strerror(int error) {

    const char * const msg[- AVAHI_ERR_MAX] = {
        N_("OK"),
        N_("Operation failed"),
        N_("Bad state"),
        N_("Invalid host name"),
        N_("Invalid domain name"),
        N_("No suitable network protocol available"),
        N_("Invalid DNS TTL"),
        N_("Resource record key is pattern"),
        N_("Local name collision"),
        N_("Invalid record"),

        N_("Invalid service name"),
        N_("Invalid service type"),
        N_("Invalid port number"),
        N_("Invalid record key"),
        N_("Invalid address"),
        N_("Timeout reached"),
        N_("Too many clients"),
        N_("Too many objects"),
        N_("Too many entries"),
        N_("OS Error"),

        N_("Access denied"),
        N_("Invalid operation"),
        N_("An unexpected D-Bus error occurred"),
        N_("Daemon connection failed"),
        N_("Memory exhausted"),
        N_("The object passed in was not valid"),
        N_("Daemon not running"),
        N_("Invalid interface index"),
        N_("Invalid protocol specification"),
        N_("Invalid flags"),

        N_("Not found"),
        N_("Invalid configuration"),
        N_("Version mismatch"),
        N_("Invalid service subtype"),
        N_("Invalid packet"),
        N_("Invalid DNS return code"),
        N_("DNS failure: FORMERR"),
        N_("DNS failure: SERVFAIL"),
        N_("DNS failure: NXDOMAIN"),
        N_("DNS failure: NOTIMP"),

        N_("DNS failure: REFUSED"),
        N_("DNS failure: YXDOMAIN"),
        N_("DNS failure: YXRRSET"),
        N_("DNS failure: NXRRSET"),
        N_("DNS failure: NOTAUTH"),
        N_("DNS failure: NOTZONE"),
        N_("Invalid RDATA"),
        N_("Invalid DNS type"),
        N_("Invalid DNS class"),
        N_("Not supported"),

        N_("Not permitted"),
        N_("Invalid argument"),
        N_("Is empty"),
        N_("The requested operation is invalid because redundant")
    };

    avahi_init_i18n();

    if (-error < 0 || -error >= -AVAHI_ERR_MAX)
        return _("Invalid Error Code");

    return _(msg[-error]);
}
