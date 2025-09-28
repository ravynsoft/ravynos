#ifndef foowideareahfoo
#define foowideareahfoo

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

typedef struct AvahiWideAreaLookupEngine AvahiWideAreaLookupEngine;
typedef struct AvahiWideAreaLookup AvahiWideAreaLookup;

typedef void (*AvahiWideAreaLookupCallback)(
    AvahiWideAreaLookupEngine *e,
    AvahiBrowserEvent event,
    AvahiLookupResultFlags flags,
    AvahiRecord *r,
    void *userdata);

AvahiWideAreaLookupEngine *avahi_wide_area_engine_new(AvahiServer *s);
void avahi_wide_area_engine_free(AvahiWideAreaLookupEngine *e);

unsigned avahi_wide_area_scan_cache(AvahiWideAreaLookupEngine *e, AvahiKey *key, AvahiWideAreaLookupCallback callback, void *userdata);
void avahi_wide_area_cache_dump(AvahiWideAreaLookupEngine *e, AvahiDumpCallback callback, void* userdata);
void avahi_wide_area_set_servers(AvahiWideAreaLookupEngine *e, const AvahiAddress *a, unsigned n);
void avahi_wide_area_clear_cache(AvahiWideAreaLookupEngine *e);
void avahi_wide_area_cleanup(AvahiWideAreaLookupEngine *e);
int avahi_wide_area_has_servers(AvahiWideAreaLookupEngine *e);

AvahiWideAreaLookup *avahi_wide_area_lookup_new(AvahiWideAreaLookupEngine *e, AvahiKey *key, AvahiWideAreaLookupCallback callback, void *userdata);
void avahi_wide_area_lookup_free(AvahiWideAreaLookup *q);



#endif

