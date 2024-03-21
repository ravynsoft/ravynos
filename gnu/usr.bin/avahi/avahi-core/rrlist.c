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
#include <assert.h>

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>

#include "rrlist.h"
#include "log.h"

typedef struct AvahiRecordListItem AvahiRecordListItem;

struct AvahiRecordListItem {
    int read;
    AvahiRecord *record;
    int unicast_response;
    int flush_cache;
    int auxiliary;
    AVAHI_LLIST_FIELDS(AvahiRecordListItem, items);
};

struct AvahiRecordList {
    AVAHI_LLIST_HEAD(AvahiRecordListItem, read);
    AVAHI_LLIST_HEAD(AvahiRecordListItem, unread);

    int all_flush_cache;
};

AvahiRecordList *avahi_record_list_new(void) {
    AvahiRecordList *l;

    if (!(l = avahi_new(AvahiRecordList, 1))) {
        avahi_log_error("avahi_new() failed.");
        return NULL;
    }

    AVAHI_LLIST_HEAD_INIT(AvahiRecordListItem, l->read);
    AVAHI_LLIST_HEAD_INIT(AvahiRecordListItem, l->unread);

    l->all_flush_cache = 1;
    return l;
}

void avahi_record_list_free(AvahiRecordList *l) {
    assert(l);

    avahi_record_list_flush(l);
    avahi_free(l);
}

static void item_free(AvahiRecordList *l, AvahiRecordListItem *i) {
    assert(l);
    assert(i);

    if (i->read)
        AVAHI_LLIST_REMOVE(AvahiRecordListItem, items, l->read, i);
    else
        AVAHI_LLIST_REMOVE(AvahiRecordListItem, items, l->unread, i);

    avahi_record_unref(i->record);
    avahi_free(i);
}

void avahi_record_list_flush(AvahiRecordList *l) {
    assert(l);

    while (l->read)
        item_free(l, l->read);
    while (l->unread)
        item_free(l, l->unread);

    l->all_flush_cache = 1;
}

AvahiRecord* avahi_record_list_next(AvahiRecordList *l, int *ret_flush_cache, int *ret_unicast_response, int *ret_auxiliary) {
    AvahiRecord *r;
    AvahiRecordListItem *i;

    if (!(i = l->unread))
        return NULL;

    assert(!i->read);

    r = avahi_record_ref(i->record);
    if (ret_unicast_response)
        *ret_unicast_response = i->unicast_response;
    if (ret_flush_cache)
        *ret_flush_cache = i->flush_cache;
    if (ret_auxiliary)
        *ret_auxiliary = i->auxiliary;

    AVAHI_LLIST_REMOVE(AvahiRecordListItem, items, l->unread, i);
    AVAHI_LLIST_PREPEND(AvahiRecordListItem, items, l->read, i);

    i->read = 1;

    return r;
}

static AvahiRecordListItem *get(AvahiRecordList *l, AvahiRecord *r) {
    AvahiRecordListItem *i;

    assert(l);
    assert(r);

    for (i = l->read; i; i = i->items_next)
        if (avahi_record_equal_no_ttl(i->record, r))
            return i;

    for (i = l->unread; i; i = i->items_next)
        if (avahi_record_equal_no_ttl(i->record, r))
            return i;

    return NULL;
}

void avahi_record_list_push(AvahiRecordList *l, AvahiRecord *r, int flush_cache, int unicast_response, int auxiliary) {
    AvahiRecordListItem *i;

    assert(l);
    assert(r);

    if (get(l, r))
        return;

    if (!(i = avahi_new(AvahiRecordListItem, 1))) {
        avahi_log_error("avahi_new() failed.");
        return;
    }

    i->unicast_response = unicast_response;
    i->flush_cache = flush_cache;
    i->auxiliary = auxiliary;
    i->record = avahi_record_ref(r);
    i->read = 0;

    l->all_flush_cache = l->all_flush_cache && flush_cache;

    AVAHI_LLIST_PREPEND(AvahiRecordListItem, items, l->unread, i);
}

void avahi_record_list_drop(AvahiRecordList *l, AvahiRecord *r) {
    AvahiRecordListItem *i;

    assert(l);
    assert(r);

    if (!(i = get(l, r)))
        return;

    item_free(l, i);
}

int avahi_record_list_is_empty(AvahiRecordList *l) {
    assert(l);

    return !l->unread && !l->read;
}

int avahi_record_list_all_flush_cache(AvahiRecordList *l) {
    assert(l);

    /* Return TRUE if all entries in this list have flush_cache set */

    return l->all_flush_cache;
}
