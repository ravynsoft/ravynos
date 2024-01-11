#ifndef foomulticastlookuphfoo
#define foomulticastlookuphfoo

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

#include "lookup.h"
#include "browse.h"

typedef struct AvahiMulticastLookupEngine AvahiMulticastLookupEngine;
typedef struct AvahiMulticastLookup AvahiMulticastLookup;

typedef void (*AvahiMulticastLookupCallback)(
    AvahiMulticastLookupEngine *e,
    AvahiIfIndex idx,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    AvahiLookupResultFlags flags,
    AvahiRecord *r,
    void *userdata);

AvahiMulticastLookupEngine *avahi_multicast_lookup_engine_new(AvahiServer *s);
void avahi_multicast_lookup_engine_free(AvahiMulticastLookupEngine *e);

unsigned avahi_multicast_lookup_engine_scan_cache(AvahiMulticastLookupEngine *e, AvahiIfIndex idx, AvahiProtocol protocol, AvahiKey *key, AvahiMulticastLookupCallback callback, void *userdata);
void avahi_multicast_lookup_engine_new_interface(AvahiMulticastLookupEngine *e, AvahiInterface *i);
void avahi_multicast_lookup_engine_cleanup(AvahiMulticastLookupEngine *e);
void avahi_multicast_lookup_engine_notify(AvahiMulticastLookupEngine *e, AvahiInterface *i, AvahiRecord *record, AvahiBrowserEvent event);

AvahiMulticastLookup *avahi_multicast_lookup_new(AvahiMulticastLookupEngine *e, AvahiIfIndex idx, AvahiProtocol protocol, AvahiKey *key, AvahiMulticastLookupCallback callback, void *userdata);
void avahi_multicast_lookup_free(AvahiMulticastLookup *q);


#endif

