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

#include <avahi-common/llist.h>
#include <avahi-common/domain.h>
#include <avahi-common/malloc.h>

#include "hashmap.h"
#include "util.h"

#define HASH_MAP_SIZE 123

typedef struct Entry Entry;
struct Entry {
    AvahiHashmap *hashmap;
    void *key;
    void *value;

    AVAHI_LLIST_FIELDS(Entry, bucket);
    AVAHI_LLIST_FIELDS(Entry, entries);
};

struct AvahiHashmap {
    AvahiHashFunc hash_func;
    AvahiEqualFunc equal_func;
    AvahiFreeFunc key_free_func, value_free_func;

    Entry *entries[HASH_MAP_SIZE];
    AVAHI_LLIST_HEAD(Entry, entries_list);
};

static Entry* entry_get(AvahiHashmap *m, const void *key) {
    unsigned idx;
    Entry *e;

    idx = m->hash_func(key) % HASH_MAP_SIZE;

    for (e = m->entries[idx]; e; e = e->bucket_next)
        if (m->equal_func(key, e->key))
            return e;

    return NULL;
}

static void entry_free(AvahiHashmap *m, Entry *e, int stolen) {
    unsigned idx;
    assert(m);
    assert(e);

    idx = m->hash_func(e->key) % HASH_MAP_SIZE;

    AVAHI_LLIST_REMOVE(Entry, bucket, m->entries[idx], e);
    AVAHI_LLIST_REMOVE(Entry, entries, m->entries_list, e);

    if (m->key_free_func)
        m->key_free_func(e->key);
    if (m->value_free_func && !stolen)
        m->value_free_func(e->value);

    avahi_free(e);
}

AvahiHashmap* avahi_hashmap_new(AvahiHashFunc hash_func, AvahiEqualFunc equal_func, AvahiFreeFunc key_free_func, AvahiFreeFunc value_free_func) {
    AvahiHashmap *m;

    assert(hash_func);
    assert(equal_func);

    if (!(m = avahi_new0(AvahiHashmap, 1)))
        return NULL;

    m->hash_func = hash_func;
    m->equal_func = equal_func;
    m->key_free_func = key_free_func;
    m->value_free_func = value_free_func;

    AVAHI_LLIST_HEAD_INIT(Entry, m->entries_list);

    return m;
}

void avahi_hashmap_free(AvahiHashmap *m) {
    assert(m);

    while (m->entries_list)
        entry_free(m, m->entries_list, 0);

    avahi_free(m);
}

void* avahi_hashmap_lookup(AvahiHashmap *m, const void *key) {
    Entry *e;

    assert(m);

    if (!(e = entry_get(m, key)))
        return NULL;

    return e->value;
}

int avahi_hashmap_insert(AvahiHashmap *m, void *key, void *value) {
    unsigned idx;
    Entry *e;

    assert(m);

    if ((e = entry_get(m, key))) {
        if (m->key_free_func)
            m->key_free_func(key);
        if (m->value_free_func)
            m->value_free_func(value);

        return 1;
    }

    if (!(e = avahi_new(Entry, 1)))
        return -1;

    e->hashmap = m;
    e->key = key;
    e->value = value;

    AVAHI_LLIST_PREPEND(Entry, entries, m->entries_list, e);

    idx = m->hash_func(key) % HASH_MAP_SIZE;
    AVAHI_LLIST_PREPEND(Entry, bucket, m->entries[idx], e);

    return 0;
}


int avahi_hashmap_replace(AvahiHashmap *m, void *key, void *value) {
    unsigned idx;
    Entry *e;

    assert(m);

    if ((e = entry_get(m, key))) {
        if (m->key_free_func)
            m->key_free_func(e->key);
        if (m->value_free_func)
            m->value_free_func(e->value);

        e->key = key;
        e->value = value;

        return 1;
    }

    if (!(e = avahi_new(Entry, 1)))
        return -1;

    e->hashmap = m;
    e->key = key;
    e->value = value;

    AVAHI_LLIST_PREPEND(Entry, entries, m->entries_list, e);

    idx = m->hash_func(key) % HASH_MAP_SIZE;
    AVAHI_LLIST_PREPEND(Entry, bucket, m->entries[idx], e);

    return 0;
}

void avahi_hashmap_remove(AvahiHashmap *m, const void *key) {
    Entry *e;

    assert(m);

    if (!(e = entry_get(m, key)))
        return;

    entry_free(m, e, 0);
}

void avahi_hashmap_foreach(AvahiHashmap *m, AvahiHashmapForeachCallback callback, void *userdata) {
    Entry *e, *next;
    assert(m);
    assert(callback);

    for (e = m->entries_list; e; e = next) {
        next = e->entries_next;

        callback(e->key, e->value, userdata);
    }
}

unsigned avahi_string_hash(const void *data) {
    const char *p = data;
    unsigned hash = 0;

    assert(p);

    for (; *p; p++)
        hash = 31 * hash + *p;

    return hash;
}

int avahi_string_equal(const void *a, const void *b) {
    const char *p = a, *q = b;

    assert(p);
    assert(q);

    return strcmp(p, q) == 0;
}

unsigned avahi_int_hash(const void *data) {
    const int *i = data;

    assert(i);

    return (unsigned) *i;
}

int avahi_int_equal(const void *a, const void *b) {
    const int *_a = a, *_b = b;

    assert(_a);
    assert(_b);

    return *_a == *_b;
}
