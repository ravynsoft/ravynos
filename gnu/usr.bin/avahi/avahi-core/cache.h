#ifndef foocachehfoo
#define foocachehfoo

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

typedef struct AvahiCache AvahiCache;

#include <avahi-common/llist.h>
#include "prioq.h"
#include "internal.h"
#include "timeeventq.h"
#include "hashmap.h"

typedef enum {
    AVAHI_CACHE_VALID,
    AVAHI_CACHE_EXPIRY1,
    AVAHI_CACHE_EXPIRY2,
    AVAHI_CACHE_EXPIRY3,
    AVAHI_CACHE_EXPIRY_FINAL,
    AVAHI_CACHE_POOF,       /* Passive observation of failure */
    AVAHI_CACHE_POOF_FINAL,
    AVAHI_CACHE_GOODBYE_FINAL,
    AVAHI_CACHE_REPLACE_FINAL
} AvahiCacheEntryState;

typedef struct AvahiCacheEntry AvahiCacheEntry;

struct AvahiCacheEntry {
    AvahiCache *cache;
    AvahiRecord *record;
    struct timeval timestamp;
    struct timeval poof_timestamp;
    struct timeval expiry;
    int cache_flush;
    int poof_num;

    AvahiAddress origin;

    AvahiCacheEntryState state;
    AvahiTimeEvent *time_event;

    AvahiAddress poof_address;

    AVAHI_LLIST_FIELDS(AvahiCacheEntry, by_key);
    AVAHI_LLIST_FIELDS(AvahiCacheEntry, entry);
};

struct AvahiCache {
    AvahiServer *server;

    AvahiInterface *interface;

    AvahiHashmap *hashmap;

    AVAHI_LLIST_HEAD(AvahiCacheEntry, entries);

    unsigned n_entries;

    int last_rand;
    time_t last_rand_timestamp;
};

AvahiCache *avahi_cache_new(AvahiServer *server, AvahiInterface *interface);
void avahi_cache_free(AvahiCache *c);

void avahi_cache_update(AvahiCache *c, AvahiRecord *r, int cache_flush, const AvahiAddress *a);

int avahi_cache_dump(AvahiCache *c, AvahiDumpCallback callback, void* userdata);

typedef void* AvahiCacheWalkCallback(AvahiCache *c, AvahiKey *pattern, AvahiCacheEntry *e, void* userdata);
void* avahi_cache_walk(AvahiCache *c, AvahiKey *pattern, AvahiCacheWalkCallback cb, void* userdata);

int avahi_cache_entry_half_ttl(AvahiCache *c, AvahiCacheEntry *e);

/** Start the "Passive observation of Failure" algorithm for all
 * records of the specified key. The specified address is  */
void avahi_cache_start_poof(AvahiCache *c, AvahiKey *key, const AvahiAddress *a);

/* Stop a previously started POOF algorithm for a record. (Used for response suppresions records */
void avahi_cache_stop_poof(AvahiCache *c, AvahiRecord *record, const AvahiAddress *a);

void avahi_cache_flush(AvahiCache *c);

#endif
