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

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <avahi-common/timeval.h>
#include <avahi-common/malloc.h>

#include "cache.h"
#include "log.h"
#include "rr-util.h"

static void remove_entry(AvahiCache *c, AvahiCacheEntry *e) {
    AvahiCacheEntry *t;

    assert(c);
    assert(e);

/*     avahi_log_debug("removing from cache: %p %p", c, e); */

    /* Remove from hash table */
    t = avahi_hashmap_lookup(c->hashmap, e->record->key);
    AVAHI_LLIST_REMOVE(AvahiCacheEntry, by_key, t, e);
    if (t)
        avahi_hashmap_replace(c->hashmap, t->record->key, t);
    else
        avahi_hashmap_remove(c->hashmap, e->record->key);

    /* Remove from linked list */
    AVAHI_LLIST_REMOVE(AvahiCacheEntry, entry, c->entries, e);

    if (e->time_event)
        avahi_time_event_free(e->time_event);

    avahi_multicast_lookup_engine_notify(c->server->multicast_lookup_engine, c->interface, e->record, AVAHI_BROWSER_REMOVE);

    avahi_record_unref(e->record);

    avahi_free(e);

    assert(c->n_entries >= 1);
    --c->n_entries;
}

AvahiCache *avahi_cache_new(AvahiServer *server, AvahiInterface *iface) {
    AvahiCache *c;
    assert(server);

    if (!(c = avahi_new(AvahiCache, 1))) {
        avahi_log_error(__FILE__": Out of memory.");
        return NULL; /* OOM */
    }

    c->server = server;
    c->interface = iface;

    if (!(c->hashmap = avahi_hashmap_new((AvahiHashFunc) avahi_key_hash, (AvahiEqualFunc) avahi_key_equal, NULL, NULL))) {
        avahi_log_error(__FILE__": Out of memory.");
        avahi_free(c);
        return NULL; /* OOM */
    }

    AVAHI_LLIST_HEAD_INIT(AvahiCacheEntry, c->entries);
    c->n_entries = 0;

    c->last_rand_timestamp = 0;

    return c;
}

void avahi_cache_free(AvahiCache *c) {
    assert(c);

    while (c->entries)
        remove_entry(c, c->entries);
    assert(c->n_entries == 0);

    avahi_hashmap_free(c->hashmap);

    avahi_free(c);
}

static AvahiCacheEntry *lookup_key(AvahiCache *c, AvahiKey *k) {
    assert(c);
    assert(k);

    assert(!avahi_key_is_pattern(k));

    return avahi_hashmap_lookup(c->hashmap, k);
}

void* avahi_cache_walk(AvahiCache *c, AvahiKey *pattern, AvahiCacheWalkCallback cb, void* userdata) {
    void* ret;

    assert(c);
    assert(pattern);
    assert(cb);

    if (avahi_key_is_pattern(pattern)) {
        AvahiCacheEntry *e, *n;

        for (e = c->entries; e; e = n) {
            n = e->entry_next;

            if (avahi_key_pattern_match(pattern, e->record->key))
                if ((ret = cb(c, pattern, e, userdata)))
                    return ret;
        }

    } else {
        AvahiCacheEntry *e, *n;

        for (e = lookup_key(c, pattern); e; e = n) {
            n = e->by_key_next;

            if ((ret = cb(c, pattern, e, userdata)))
                return ret;
        }
    }

    return NULL;
}

static void* lookup_record_callback(AvahiCache *c, AvahiKey *pattern, AvahiCacheEntry *e, void *userdata) {
    assert(c);
    assert(pattern);
    assert(e);

    if (avahi_record_equal_no_ttl(e->record, userdata))
        return e;

    return NULL;
}

static AvahiCacheEntry *lookup_record(AvahiCache *c, AvahiRecord *r) {
    assert(c);
    assert(r);

    return avahi_cache_walk(c, r->key, lookup_record_callback, r);
}

static void next_expiry(AvahiCache *c, AvahiCacheEntry *e, unsigned percent);

static void elapse_func(AvahiTimeEvent *t, void *userdata) {
    AvahiCacheEntry *e = userdata;
/*     char *txt; */
    unsigned percent = 0;

    assert(t);
    assert(e);

/*     txt = avahi_record_to_string(e->record); */

    switch (e->state) {

        case AVAHI_CACHE_EXPIRY_FINAL:
        case AVAHI_CACHE_POOF_FINAL:
        case AVAHI_CACHE_GOODBYE_FINAL:
        case AVAHI_CACHE_REPLACE_FINAL:

            remove_entry(e->cache, e);

            e = NULL;
/*         avahi_log_debug("Removing entry from cache due to expiration (%s)", txt); */
            break;

        case AVAHI_CACHE_VALID:
        case AVAHI_CACHE_POOF:
            e->state = AVAHI_CACHE_EXPIRY1;
            percent = 85;
            break;

        case AVAHI_CACHE_EXPIRY1:
            e->state = AVAHI_CACHE_EXPIRY2;
            percent = 90;
            break;
        case AVAHI_CACHE_EXPIRY2:
            e->state = AVAHI_CACHE_EXPIRY3;
            percent = 95;
            break;

        case AVAHI_CACHE_EXPIRY3:
            e->state = AVAHI_CACHE_EXPIRY_FINAL;
            percent = 100;
            break;
    }

    if (e) {

        assert(percent > 0);

        /* Request a cache update if we are subscribed to this entry */
        if (avahi_querier_shall_refresh_cache(e->cache->interface, e->record->key))
            avahi_interface_post_query(e->cache->interface, e->record->key, 0, NULL);

        /* Check again later */
        next_expiry(e->cache, e, percent);

    }

/*     avahi_free(txt); */
}

static void update_time_event(AvahiCache *c, AvahiCacheEntry *e) {
    assert(c);
    assert(e);

    if (e->time_event)
        avahi_time_event_update(e->time_event, &e->expiry);
    else
        e->time_event = avahi_time_event_new(c->server->time_event_queue, &e->expiry, elapse_func, e);
}

static void next_expiry(AvahiCache *c, AvahiCacheEntry *e, unsigned percent) {
    AvahiUsec usec, left, right;
    time_t now;

    assert(c);
    assert(e);
    assert(percent > 0 && percent <= 100);

    usec = (AvahiUsec) e->record->ttl * 10000;

    left = usec * percent;
    right = usec * (percent+2); /* 2% jitter */

    now = time(NULL);

    if (now >= c->last_rand_timestamp + 10) {
        c->last_rand = rand();
        c->last_rand_timestamp = now;
    }

    usec = left + (AvahiUsec) ((double) (right-left) * c->last_rand / (RAND_MAX+1.0));

    e->expiry = e->timestamp;
    avahi_timeval_add(&e->expiry, usec);

/*     g_message("wake up in +%lu seconds", e->expiry.tv_sec - e->timestamp.tv_sec); */

    update_time_event(c, e);
}

static void expire_in_one_second(AvahiCache *c, AvahiCacheEntry *e, AvahiCacheEntryState state) {
    assert(c);
    assert(e);

    e->state = state;
    gettimeofday(&e->expiry, NULL);
    avahi_timeval_add(&e->expiry, 1000000); /* 1s */
    update_time_event(c, e);
}

void avahi_cache_update(AvahiCache *c, AvahiRecord *r, int cache_flush, const AvahiAddress *a) {
/*     char *txt; */

    assert(c);
    assert(r && r->ref >= 1);

/*     txt = avahi_record_to_string(r); */

    if (r->ttl == 0) {
        /* This is a goodbye request */

        AvahiCacheEntry *e;

        if ((e = lookup_record(c, r)))
            expire_in_one_second(c, e, AVAHI_CACHE_GOODBYE_FINAL);

    } else {
        AvahiCacheEntry *e = NULL, *first;
        struct timeval now;

        gettimeofday(&now, NULL);

        /* This is an update request */

        if ((first = lookup_key(c, r->key))) {

            if (cache_flush) {

                /* For unique entries drop all entries older than one second */
                for (e = first; e; e = e->by_key_next) {
                    AvahiUsec t;

                    t = avahi_timeval_diff(&now, &e->timestamp);

                    if (t > 1000000)
                        expire_in_one_second(c, e, AVAHI_CACHE_REPLACE_FINAL);
                }
            }

            /* Look for exactly the same entry */
            for (e = first; e; e = e->by_key_next)
                if (avahi_record_equal_no_ttl(e->record, r))
                    break;
        }

        if (e) {

/*             avahi_log_debug("found matching cache entry");  */

            /* We need to update the hash table key if we replace the
             * record */
            if (e->by_key_prev == NULL)
                avahi_hashmap_replace(c->hashmap, r->key, e);

            /* Update the record */
            avahi_record_unref(e->record);
            e->record = avahi_record_ref(r);

/*             avahi_log_debug("cache: updating %s", txt);   */

        } else {
            /* No entry found, therefore we create a new one */

/*             avahi_log_debug("cache: couldn't find matching cache entry for %s", txt);   */

            if (c->n_entries >= c->server->config.n_cache_entries_max)
                return;

            if (!(e = avahi_new(AvahiCacheEntry, 1))) {
                avahi_log_error(__FILE__": Out of memory");
                return;
            }

            e->cache = c;
            e->time_event = NULL;
            e->record = avahi_record_ref(r);

            /* Append to hash table */
            AVAHI_LLIST_PREPEND(AvahiCacheEntry, by_key, first, e);
            avahi_hashmap_replace(c->hashmap, e->record->key, first);

            /* Append to linked list */
            AVAHI_LLIST_PREPEND(AvahiCacheEntry, entry, c->entries, e);

            c->n_entries++;

            /* Notify subscribers */
            avahi_multicast_lookup_engine_notify(c->server->multicast_lookup_engine, c->interface, e->record, AVAHI_BROWSER_NEW);
        }

        e->origin = *a;
        e->timestamp = now;
        next_expiry(c, e, 80);
        e->state = AVAHI_CACHE_VALID;
        e->cache_flush = cache_flush;
    }

/*     avahi_free(txt);  */
}

struct dump_data {
    AvahiDumpCallback callback;
    void* userdata;
};

static void dump_callback(void* key, void* data, void* userdata) {
    AvahiCacheEntry *e = data;
    AvahiKey *k = key;
    struct dump_data *dump_data = userdata;

    assert(k);
    assert(e);
    assert(data);

    for (; e; e = e->by_key_next) {
        char *t;

        if (!(t = avahi_record_to_string(e->record)))
            continue; /* OOM */

        dump_data->callback(t, dump_data->userdata);
        avahi_free(t);
    }
}

int avahi_cache_dump(AvahiCache *c, AvahiDumpCallback callback, void* userdata) {
    struct dump_data data;

    assert(c);
    assert(callback);

    callback(";;; CACHE DUMP FOLLOWS ;;;", userdata);

    data.callback = callback;
    data.userdata = userdata;

    avahi_hashmap_foreach(c->hashmap, dump_callback, &data);

    return 0;
}

int avahi_cache_entry_half_ttl(AvahiCache *c, AvahiCacheEntry *e) {
    struct timeval now;
    unsigned age;

    assert(c);
    assert(e);

    gettimeofday(&now, NULL);

    age = (unsigned) (avahi_timeval_diff(&now, &e->timestamp)/1000000);

/*     avahi_log_debug("age: %lli, ttl/2: %u", age, e->record->ttl);  */

    return age >= e->record->ttl/2;
}

void avahi_cache_flush(AvahiCache *c) {
    assert(c);

    while (c->entries)
        remove_entry(c, c->entries);
}

/*** Passive observation of failure ***/

static void* start_poof_callback(AvahiCache *c, AvahiKey *pattern, AvahiCacheEntry *e, void *userdata) {
    AvahiAddress *a = userdata;
    struct timeval now;

    assert(c);
    assert(pattern);
    assert(e);
    assert(a);

    gettimeofday(&now, NULL);

    switch (e->state) {
        case AVAHI_CACHE_VALID:

            /* The entry was perfectly valid till, now, so let's enter
             * POOF mode */

            e->state = AVAHI_CACHE_POOF;
            e->poof_address = *a;
            e->poof_timestamp = now;
            e->poof_num = 0;

            break;

        case AVAHI_CACHE_POOF:
            if (avahi_timeval_diff(&now, &e->poof_timestamp) < 1000000)
              break;

            e->poof_timestamp = now;
            e->poof_address = *a;
            e->poof_num ++;

            /* This is the 4th time we got no response, so let's
             * fucking remove this entry. */
            if (e->poof_num > 3)
              expire_in_one_second(c, e, AVAHI_CACHE_POOF_FINAL);
            break;

        default:
            ;
    }

    return NULL;
}

void avahi_cache_start_poof(AvahiCache *c, AvahiKey *key, const AvahiAddress *a) {
    assert(c);
    assert(key);

    avahi_cache_walk(c, key, start_poof_callback, (void*) a);
}

void avahi_cache_stop_poof(AvahiCache *c, AvahiRecord *record, const AvahiAddress *a) {
    AvahiCacheEntry *e;

    assert(c);
    assert(record);
    assert(a);

    if (!(e = lookup_record(c, record)))
        return;

    /* This function is called for each response suppression
       record. If the matching cache entry is in POOF state and the
       query address is the same, we put it back into valid mode */

    if (e->state == AVAHI_CACHE_POOF || e->state == AVAHI_CACHE_POOF_FINAL)
        if (avahi_address_cmp(a, &e->poof_address) == 0) {
            e->state = AVAHI_CACHE_VALID;
            next_expiry(c, e, 80);
        }
}
