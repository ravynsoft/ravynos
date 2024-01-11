# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

# Some definitions matching those in avahi-common/defs.h

import sys

import dbus

SERVER_INVALID, SERVER_REGISTERING, SERVER_RUNNING, SERVER_COLLISION, SERVER_FAILURE = range(0, 5)

ENTRY_GROUP_UNCOMMITED, ENTRY_GROUP_REGISTERING, ENTRY_GROUP_ESTABLISHED, ENTRY_GROUP_COLLISION, ENTRY_GROUP_FAILURE = range(0, 5)

DOMAIN_BROWSER_BROWSE, DOMAIN_BROWSER_BROWSE_DEFAULT, DOMAIN_BROWSER_REGISTER, DOMAIN_BROWSER_REGISTER_DEFAULT, DOMAIN_BROWSER_BROWSE_LEGACY = range(0, 5)

PROTO_UNSPEC, PROTO_INET, PROTO_INET6  = -1, 0, 1

IF_UNSPEC = -1

PUBLISH_UNIQUE = 1
PUBLISH_NO_PROBE = 2
PUBLISH_NO_ANNOUNCE = 4
PUBLISH_ALLOW_MULTIPLE = 8
PUBLISH_NO_REVERSE = 16
PUBLISH_NO_COOKIE = 32
PUBLISH_UPDATE = 64
PUBLISH_USE_WIDE_AREA = 128
PUBLISH_USE_MULTICAST = 256

LOOKUP_USE_WIDE_AREA = 1
LOOKUP_USE_MULTICAST = 2
LOOKUP_NO_TXT = 4
LOOKUP_NO_ADDRESS = 8

LOOKUP_RESULT_CACHED = 1
LOOKUP_RESULT_WIDE_AREA = 2
LOOKUP_RESULT_MULTICAST = 4
LOOKUP_RESULT_LOCAL = 8
LOOKUP_RESULT_OUR_OWN = 16
LOOKUP_RESULT_STATIC = 32

SERVICE_COOKIE = "org.freedesktop.Avahi.cookie"
SERVICE_COOKIE_INVALID = 0

DBUS_NAME = "org.freedesktop.Avahi"
DBUS_INTERFACE_SERVER = DBUS_NAME + ".Server"
DBUS_PATH_SERVER = "/"
DBUS_INTERFACE_ENTRY_GROUP = DBUS_NAME + ".EntryGroup"
DBUS_INTERFACE_DOMAIN_BROWSER = DBUS_NAME + ".DomainBrowser"
DBUS_INTERFACE_SERVICE_TYPE_BROWSER = DBUS_NAME + ".ServiceTypeBrowser"
DBUS_INTERFACE_SERVICE_BROWSER = DBUS_NAME + ".ServiceBrowser"
DBUS_INTERFACE_ADDRESS_RESOLVER = DBUS_NAME + ".AddressResolver"
DBUS_INTERFACE_HOST_NAME_RESOLVER = DBUS_NAME + ".HostNameResolver"
DBUS_INTERFACE_SERVICE_RESOLVER = DBUS_NAME + ".ServiceResolver"
DBUS_INTERFACE_RECORD_BROWSER = DBUS_NAME + ".RecordBrowser"

if sys.version_info[0] >= 3:
    unicode = str

def byte_array_to_string(s):
    r = ""
    
    for c in s:
        
        if c >= 32 and c < 127:
            r += "%c" % c
        else:
            r += "."

    return r

def txt_array_to_string_array(t):
    l = []

    for s in t:
        l.append(byte_array_to_string(s))

    return l

def string_to_byte_array(s):
    if isinstance(s, unicode):
        s = s.encode('utf-8')

    r = []

    for c in s:
        if isinstance(c, int):
            # Python 3: iterating over bytes yields ints
            r.append(dbus.Byte(c))
        else:
            # Python 2: iterating over str yields str
            r.append(dbus.Byte(ord(c)))

    return r

def string_array_to_txt_array(t):
    l = []

    for s in t:
        l.append(string_to_byte_array(s))

    return l

def dict_to_txt_array(txt_dict):
    l = []

    for k,v in txt_dict.items():
        if isinstance(k, unicode):
            k = k.encode('utf-8')

        if isinstance(v, unicode):
            v = v.encode('utf-8')

        l.append(string_to_byte_array(b"%s=%s" % (k,v)))

    return l
