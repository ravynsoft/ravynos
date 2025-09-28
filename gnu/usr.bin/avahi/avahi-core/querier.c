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

#include <avahi-common/timeval.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/domain.h>

#include "querier.h"
#include "log.h"

struct AvahiQuerier {
    AvahiInterface *interface;

    AvahiKey *key;
    int n_used;

    unsigned sec_delay;

    AvahiTimeEvent *time_event;

    struct timeval creation_time;

    unsigned post_id;
    int post_id_valid;

    AVAHI_LLIST_FIELDS(AvahiQuerier, queriers);
};

void avahi_querier_free(AvahiQuerier *q) {
    assert(q);

    AVAHI_LLIST_REMOVE(AvahiQuerier, queriers, q->interface->queriers, q);
    avahi_hashmap_remove(q->interface->queriers_by_key, q->key);

    avahi_key_unref(q->key);
    avahi_time_event_free(q->time_event);

    avahi_free(q);
}

static void querier_elapse_callback(AVAHI_GCC_UNUSED AvahiTimeEvent *e, void *userdata) {
    AvahiQuerier *q = userdata;
    struct timeval tv;

    assert(q);

    if (q->n_used <= 0) {

        /* We are not referenced by anyone anymore, so let's free
         * ourselves. We should not send out any further queries from
         * this querier object anymore. */

        avahi_querier_free(q);
        return;
    }

    if (avahi_interface_post_query(q->interface, q->key, 0, &q->post_id)) {

        /* The queue accepted our query. We store the query id here,
         * that allows us to drop the query at a later point if the
         * query is very short-lived. */

        q->post_id_valid = 1;
    }

    q->sec_delay *= 2;

    if (q->sec_delay >= 60*60)  /* 1h */
        q->sec_delay = 60*60;

    avahi_elapse_time(&tv, q->sec_delay*1000, 0);
    avahi_time_event_update(q->time_event, &tv);
}

void avahi_querier_add(AvahiInterface *i, AvahiKey *key, struct timeval *ret_ctime) {
    AvahiQuerier *q;
    struct timeval tv;

    assert(i);
    assert(key);

    if ((q = avahi_hashmap_lookup(i->queriers_by_key, key))) {

        /* Someone is already browsing for records of this RR key */
        q->n_used++;

        /* Return the creation time. This is used for generating the
         * ALL_FOR_NOW event one second after the querier was
         * initially created. */
        if (ret_ctime)
            *ret_ctime = q->creation_time;
        return;
    }

    /* No one is browsing for this RR key, so we add a new querier */
    if (!(q = avahi_new(AvahiQuerier, 1)))
        return; /* OOM */

    q->key = avahi_key_ref(key);
    q->interface = i;
    q->n_used = 1;
    q->sec_delay = 1;
    q->post_id_valid = 0;
    gettimeofday(&q->creation_time, NULL);

    /* Do the initial query */
    if (avahi_interface_post_query(i, key, 0, &q->post_id))
        q->post_id_valid = 1;

    /* Schedule next queries */
    q->time_event = avahi_time_event_new(i->monitor->server->time_event_queue, avahi_elapse_time(&tv, q->sec_delay*1000, 0), querier_elapse_callback, q);

    AVAHI_LLIST_PREPEND(AvahiQuerier, queriers, i->queriers, q);
    avahi_hashmap_insert(i->queriers_by_key, q->key, q);

    /* Return the creation time. This is used for generating the
     * ALL_FOR_NOW event one second after the querier was initially
     * created. */
    if (ret_ctime)
        *ret_ctime = q->creation_time;
}

void avahi_querier_remove(AvahiInterface *i, AvahiKey *key) {
    AvahiQuerier *q;

    /* There was no querier for this RR key, or it wasn't referenced
     * by anyone. */
    if (!(q = avahi_hashmap_lookup(i->queriers_by_key, key)) || q->n_used <= 0)
        return;

    if ((--q->n_used) <= 0) {

        /* Nobody references us anymore. */

        if (q->post_id_valid && avahi_interface_withraw_query(i, q->post_id)) {

            /* We succeeded in withdrawing our query from the queue,
             * so let's drop dead. */

            avahi_querier_free(q);
        }

        /* If we failed to withdraw our query from the queue, we stay
         * alive, in case someone else might recycle our querier at a
         * later point. We are freed at our next expiry, in case
         * nobody recycled us. */
    }
}

static void remove_querier_callback(AvahiInterfaceMonitor *m, AvahiInterface *i, void* userdata) {
    assert(m);
    assert(i);
    assert(userdata);

    if (i->announcing)
        avahi_querier_remove(i, (AvahiKey*) userdata);
}

void avahi_querier_remove_for_all(AvahiServer *s, AvahiIfIndex idx, AvahiProtocol protocol, AvahiKey *key) {
    assert(s);
    assert(key);

    avahi_interface_monitor_walk(s->monitor, idx, protocol, remove_querier_callback, key);
}

struct cbdata {
    AvahiKey *key;
    struct timeval *ret_ctime;
};

static void add_querier_callback(AvahiInterfaceMonitor *m, AvahiInterface *i, void* userdata) {
    struct cbdata *cbdata = userdata;

    assert(m);
    assert(i);
    assert(cbdata);

    if (i->announcing) {
        struct timeval tv;
        avahi_querier_add(i, cbdata->key, &tv);

        if (cbdata->ret_ctime && avahi_timeval_compare(&tv, cbdata->ret_ctime) > 0)
            *cbdata->ret_ctime = tv;
    }
}

void avahi_querier_add_for_all(AvahiServer *s, AvahiIfIndex idx, AvahiProtocol protocol, AvahiKey *key, struct timeval *ret_ctime) {
    struct cbdata cbdata;

    assert(s);
    assert(key);

    cbdata.key = key;
    cbdata.ret_ctime = ret_ctime;

    if (ret_ctime)
        ret_ctime->tv_sec = ret_ctime->tv_usec = 0;

    avahi_interface_monitor_walk(s->monitor, idx, protocol, add_querier_callback, &cbdata);
}

int avahi_querier_shall_refresh_cache(AvahiInterface *i, AvahiKey *key) {
    AvahiQuerier *q;

    assert(i);
    assert(key);

    /* Called by the cache maintainer */

    if (!(q = avahi_hashmap_lookup(i->queriers_by_key, key)))
        /* This key is currently not subscribed at all, so no cache
         * refresh is needed */
        return 0;

    if (q->n_used <= 0) {

        /* If this is an entry nobody references right now, don't
         * consider it "existing". */

        /* Remove this querier since it is referenced by nobody
         * and the cached data will soon be out of date */
        avahi_querier_free(q);

        /* Tell the cache that no refresh is needed */
        return 0;

    } else {
        struct timeval tv;

        /* We can defer our query a little, since the cache will now
         * issue a refresh query anyway. */
        avahi_elapse_time(&tv, q->sec_delay*1000, 0);
        avahi_time_event_update(q->time_event, &tv);

        /* Tell the cache that a refresh should be issued */
        return 1;
    }
}

void avahi_querier_free_all(AvahiInterface *i) {
    assert(i);

    while (i->queriers)
        avahi_querier_free(i->queriers);
}
