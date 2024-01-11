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

#include <assert.h>
#include <stdlib.h>

#include <avahi-common/timeval.h>
#include <avahi-common/malloc.h>

#include "timeeventq.h"
#include "log.h"

struct AvahiTimeEvent {
    AvahiTimeEventQueue *queue;
    AvahiPrioQueueNode *node;
    struct timeval expiry;
    struct timeval last_run;
    AvahiTimeEventCallback callback;
    void* userdata;
};

struct AvahiTimeEventQueue {
    const AvahiPoll *poll_api;
    AvahiPrioQueue *prioq;
    AvahiTimeout *timeout;
};

static int compare(const void* _a, const void* _b) {
    const AvahiTimeEvent *a = _a,  *b = _b;
    int ret;

    if ((ret = avahi_timeval_compare(&a->expiry, &b->expiry)) != 0)
        return ret;

    /* If both exevents are scheduled for the same time, put the entry
     * that has been run earlier the last time first. */
    return avahi_timeval_compare(&a->last_run, &b->last_run);
}

static AvahiTimeEvent* time_event_queue_root(AvahiTimeEventQueue *q) {
    assert(q);

    return q->prioq->root ? q->prioq->root->data : NULL;
}

static void update_timeout(AvahiTimeEventQueue *q) {
    AvahiTimeEvent *e;
    assert(q);

    if ((e = time_event_queue_root(q)))
        q->poll_api->timeout_update(q->timeout, &e->expiry);
    else
        q->poll_api->timeout_update(q->timeout, NULL);
}

static void expiration_event(AVAHI_GCC_UNUSED AvahiTimeout *timeout, void *userdata) {
    AvahiTimeEventQueue *q = userdata;
    AvahiTimeEvent *e;

    if ((e = time_event_queue_root(q))) {
        struct timeval now;

        gettimeofday(&now, NULL);

        /* Check if expired */
        if (avahi_timeval_compare(&now, &e->expiry) >= 0) {

            /* Make sure to move the entry away from the front */
            e->last_run = now;
            avahi_prio_queue_shuffle(q->prioq, e->node);

            /* Run it */
            assert(e->callback);
            e->callback(e, e->userdata);

            update_timeout(q);
            return;
        }
    }

    avahi_log_debug(__FILE__": Strange, expiration_event() called, but nothing really happened.");
    update_timeout(q);
}

static void fix_expiry_time(AvahiTimeEvent *e) {
    struct timeval now;
    assert(e);

    return; /*** DO WE REALLY NEED THIS? ***/

    gettimeofday(&now, NULL);

    if (avahi_timeval_compare(&now, &e->expiry) > 0)
        e->expiry = now;
}

AvahiTimeEventQueue* avahi_time_event_queue_new(const AvahiPoll *poll_api) {
    AvahiTimeEventQueue *q;

    if (!(q = avahi_new(AvahiTimeEventQueue, 1))) {
        avahi_log_error(__FILE__": Out of memory");
        goto oom;
    }

    q->poll_api = poll_api;

    if (!(q->prioq = avahi_prio_queue_new(compare)))
        goto oom;

    if (!(q->timeout = poll_api->timeout_new(poll_api, NULL, expiration_event, q)))
        goto oom;

    return q;

oom:

    if (q) {
        avahi_free(q);

        if (q->prioq)
            avahi_prio_queue_free(q->prioq);
    }

    return NULL;
}

void avahi_time_event_queue_free(AvahiTimeEventQueue *q) {
    AvahiTimeEvent *e;

    assert(q);

    while ((e = time_event_queue_root(q)))
        avahi_time_event_free(e);
    avahi_prio_queue_free(q->prioq);

    q->poll_api->timeout_free(q->timeout);

    avahi_free(q);
}

AvahiTimeEvent* avahi_time_event_new(
    AvahiTimeEventQueue *q,
    const struct timeval *timeval,
    AvahiTimeEventCallback callback,
    void* userdata) {

    AvahiTimeEvent *e;

    assert(q);
    assert(callback);
    assert(userdata);

    if (!(e = avahi_new(AvahiTimeEvent, 1))) {
        avahi_log_error(__FILE__": Out of memory");
        return NULL; /* OOM */
    }

    e->queue = q;
    e->callback = callback;
    e->userdata = userdata;

    if (timeval)
        e->expiry = *timeval;
    else {
        e->expiry.tv_sec = 0;
        e->expiry.tv_usec = 0;
    }

    fix_expiry_time(e);

    e->last_run.tv_sec = 0;
    e->last_run.tv_usec = 0;

    if (!(e->node = avahi_prio_queue_put(q->prioq, e))) {
        avahi_free(e);
        return NULL;
    }

    update_timeout(q);
    return e;
}

void avahi_time_event_free(AvahiTimeEvent *e) {
    AvahiTimeEventQueue *q;
    assert(e);

    q = e->queue;

    avahi_prio_queue_remove(q->prioq, e->node);
    avahi_free(e);

    update_timeout(q);
}

void avahi_time_event_update(AvahiTimeEvent *e, const struct timeval *timeval) {
    assert(e);
    assert(timeval);

    e->expiry = *timeval;
    fix_expiry_time(e);
    avahi_prio_queue_shuffle(e->queue->prioq, e->node);

    update_timeout(e->queue);
}

