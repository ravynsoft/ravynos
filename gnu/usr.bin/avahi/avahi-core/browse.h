#ifndef foobrowsehfoo
#define foobrowsehfoo

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

#include <avahi-common/llist.h>

#include "core.h"
#include "timeeventq.h"
#include "internal.h"
#include "dns.h"
#include "lookup.h"

typedef struct AvahiSRBLookup AvahiSRBLookup;

struct AvahiSRecordBrowser {
    AVAHI_LLIST_FIELDS(AvahiSRecordBrowser, browser);
    int dead;
    AvahiServer *server;

    AvahiKey *key;
    AvahiIfIndex interface;
    AvahiProtocol protocol;
    AvahiLookupFlags flags;

    AvahiTimeEvent *defer_time_event;

    AvahiSRecordBrowserCallback callback;
    void* userdata;

    /* Lookup data */
    AVAHI_LLIST_HEAD(AvahiSRBLookup, lookups);
    unsigned n_lookups;

    AvahiSRBLookup *root_lookup;
};

void avahi_browser_cleanup(AvahiServer *server);

void avahi_s_record_browser_destroy(AvahiSRecordBrowser *b);
void avahi_s_record_browser_restart(AvahiSRecordBrowser *b);

#endif
