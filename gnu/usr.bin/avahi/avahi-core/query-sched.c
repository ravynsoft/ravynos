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

#include "query-sched.h"
#include "log.h"

#define AVAHI_QUERY_HISTORY_MSEC 100
#define AVAHI_QUERY_DEFER_MSEC 100

typedef struct AvahiQueryJob AvahiQueryJob;
typedef struct AvahiKnownAnswer AvahiKnownAnswer;

struct AvahiQueryJob {
    unsigned id;
    int n_posted;

    AvahiQueryScheduler *scheduler;
    AvahiTimeEvent *time_event;

    int done;
    struct timeval delivery;

    AvahiKey *key;

    /* Jobs are stored in a simple linked list. It might turn out in
     * the future that this list grows too long and we must switch to
     * some other kind of data structure. This needs further
     * investigation. I expect the list to be very short (< 20
     * entries) most of the time, but this might be a wrong
     * assumption, especially on setups where traffic reflection is
     * involved. */

    AVAHI_LLIST_FIELDS(AvahiQueryJob, jobs);
};

struct AvahiKnownAnswer {
    AvahiQueryScheduler *scheduler;
    AvahiRecord *record;

    AVAHI_LLIST_FIELDS(AvahiKnownAnswer, known_answer);
};

struct AvahiQueryScheduler {
    AvahiInterface *interface;
    AvahiTimeEventQueue *time_event_queue;

    unsigned next_id;

    AVAHI_LLIST_HEAD(AvahiQueryJob, jobs);
    AVAHI_LLIST_HEAD(AvahiQueryJob, history);
    AVAHI_LLIST_HEAD(AvahiKnownAnswer, known_answers);
};

static AvahiQueryJob* job_new(AvahiQueryScheduler *s, AvahiKey *key, int done) {
    AvahiQueryJob *qj;

    assert(s);
    assert(key);

    if (!(qj = avahi_new(AvahiQueryJob, 1))) {
        avahi_log_error(__FILE__": Out of memory");
        return NULL;
    }

    qj->scheduler = s;
    qj->key = avahi_key_ref(key);
    qj->time_event = NULL;
    qj->n_posted = 1;
    qj->id = s->next_id++;

    if ((qj->done = done))
        AVAHI_LLIST_PREPEND(AvahiQueryJob, jobs, s->history, qj);
    else
        AVAHI_LLIST_PREPEND(AvahiQueryJob, jobs, s->jobs, qj);

    return qj;
}

static void job_free(AvahiQueryScheduler *s, AvahiQueryJob *qj) {
    assert(s);
    assert(qj);

    if (qj->time_event)
        avahi_time_event_free(qj->time_event);

    if (qj->done)
        AVAHI_LLIST_REMOVE(AvahiQueryJob, jobs, s->history, qj);
    else
        AVAHI_LLIST_REMOVE(AvahiQueryJob, jobs, s->jobs, qj);

    avahi_key_unref(qj->key);
    avahi_free(qj);
}

static void elapse_callback(AvahiTimeEvent *e, void* data);

static void job_set_elapse_time(AvahiQueryScheduler *s, AvahiQueryJob *qj, unsigned msec, unsigned jitter) {
    struct timeval tv;

    assert(s);
    assert(qj);

    avahi_elapse_time(&tv, msec, jitter);

    if (qj->time_event)
        avahi_time_event_update(qj->time_event, &tv);
    else
        qj->time_event = avahi_time_event_new(s->time_event_queue, &tv, elapse_callback, qj);
}

static void job_mark_done(AvahiQueryScheduler *s, AvahiQueryJob *qj) {
    assert(s);
    assert(qj);

    assert(!qj->done);

    AVAHI_LLIST_REMOVE(AvahiQueryJob, jobs, s->jobs, qj);
    AVAHI_LLIST_PREPEND(AvahiQueryJob, jobs, s->history, qj);

    qj->done = 1;

    job_set_elapse_time(s, qj, AVAHI_QUERY_HISTORY_MSEC, 0);
    gettimeofday(&qj->delivery, NULL);
}

AvahiQueryScheduler *avahi_query_scheduler_new(AvahiInterface *i) {
    AvahiQueryScheduler *s;
    assert(i);

    if (!(s = avahi_new(AvahiQueryScheduler, 1))) {
        avahi_log_error(__FILE__": Out of memory");
        return NULL; /* OOM */
    }

    s->interface = i;
    s->time_event_queue = i->monitor->server->time_event_queue;
    s->next_id = 0;

    AVAHI_LLIST_HEAD_INIT(AvahiQueryJob, s->jobs);
    AVAHI_LLIST_HEAD_INIT(AvahiQueryJob, s->history);
    AVAHI_LLIST_HEAD_INIT(AvahiKnownAnswer, s->known_answers);

    return s;
}

void avahi_query_scheduler_free(AvahiQueryScheduler *s) {
    assert(s);

    assert(!s->known_answers);
    avahi_query_scheduler_clear(s);
    avahi_free(s);
}

void avahi_query_scheduler_clear(AvahiQueryScheduler *s) {
    assert(s);

    while (s->jobs)
        job_free(s, s->jobs);
    while (s->history)
        job_free(s, s->history);
}

static void* known_answer_walk_callback(AvahiCache *c, AvahiKey *pattern, AvahiCacheEntry *e, void* userdata) {
    AvahiQueryScheduler *s = userdata;
    AvahiKnownAnswer *ka;

    assert(c);
    assert(pattern);
    assert(e);
    assert(s);

    if (avahi_cache_entry_half_ttl(c, e))
        return NULL;

    if (!(ka = avahi_new0(AvahiKnownAnswer, 1))) {
        avahi_log_error(__FILE__": Out of memory");
        return NULL;
    }

    ka->scheduler = s;
    ka->record = avahi_record_ref(e->record);

    AVAHI_LLIST_PREPEND(AvahiKnownAnswer, known_answer, s->known_answers, ka);
    return NULL;
}

static int packet_add_query_job(AvahiQueryScheduler *s, AvahiDnsPacket *p, AvahiQueryJob *qj) {
    assert(s);
    assert(p);
    assert(qj);

    if (!avahi_dns_packet_append_key(p, qj->key, 0))
        return 0;

    /* Add all matching known answers to the list */
    avahi_cache_walk(s->interface->cache, qj->key, known_answer_walk_callback, s);

    job_mark_done(s, qj);

    return 1;
}

static void append_known_answers_and_send(AvahiQueryScheduler *s, AvahiDnsPacket *p) {
    AvahiKnownAnswer *ka;
    unsigned n;
    assert(s);
    assert(p);

    n = 0;

    while ((ka = s->known_answers)) {
        int too_large = 0;

        while (!avahi_dns_packet_append_record(p, ka->record, 0, 0)) {

            if (avahi_dns_packet_is_empty(p)) {
                /* The record is too large to fit into one packet, so
                   there's no point in sending it. Better is letting
                   the owner of the record send it as a response. This
                   has the advantage of a cache refresh. */

                too_large = 1;
                break;
            }

            avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_FLAGS, avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_FLAGS) | AVAHI_DNS_FLAG_TC);
            avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_ANCOUNT, n);
            avahi_interface_send_packet(s->interface, p);
            avahi_dns_packet_free(p);

            p = avahi_dns_packet_new_query(s->interface->hardware->mtu);
            n = 0;
        }

        AVAHI_LLIST_REMOVE(AvahiKnownAnswer, known_answer, s->known_answers, ka);
        avahi_record_unref(ka->record);
        avahi_free(ka);

        if (!too_large)
            n++;
    }

    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_ANCOUNT, n);
    avahi_interface_send_packet(s->interface, p);
    avahi_dns_packet_free(p);
}

static void elapse_callback(AVAHI_GCC_UNUSED AvahiTimeEvent *e, void* data) {
    AvahiQueryJob *qj = data;
    AvahiQueryScheduler *s;
    AvahiDnsPacket *p;
    unsigned n;
    int b;

    assert(qj);
    s = qj->scheduler;

    if (qj->done) {
        /* Lets remove it  from the history */
        job_free(s, qj);
        return;
    }

    assert(!s->known_answers);

    if (!(p = avahi_dns_packet_new_query(s->interface->hardware->mtu)))
        return; /* OOM */

    b = packet_add_query_job(s, p, qj);
    assert(b); /* An query must always fit in */
    n = 1;

    /* Try to fill up packet with more queries, if available */
    while (s->jobs) {

        if (!packet_add_query_job(s, p, s->jobs))
            break;

        n++;
    }

    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_QDCOUNT, n);

    /* Now add known answers */
    append_known_answers_and_send(s, p);
}

static AvahiQueryJob* find_scheduled_job(AvahiQueryScheduler *s, AvahiKey *key) {
    AvahiQueryJob *qj;

    assert(s);
    assert(key);

    for (qj = s->jobs; qj; qj = qj->jobs_next) {
        assert(!qj->done);

        if (avahi_key_equal(qj->key, key))
            return qj;
    }

    return NULL;
}

static AvahiQueryJob* find_history_job(AvahiQueryScheduler *s, AvahiKey *key) {
    AvahiQueryJob *qj;

    assert(s);
    assert(key);

    for (qj = s->history; qj; qj = qj->jobs_next) {
        assert(qj->done);

        if (avahi_key_equal(qj->key, key)) {
            /* Check whether this entry is outdated */

            if (avahi_age(&qj->delivery) > AVAHI_QUERY_HISTORY_MSEC*1000) {
                /* it is outdated, so let's remove it */
                job_free(s, qj);
                return NULL;
            }

            return qj;
        }
    }

    return NULL;
}

int avahi_query_scheduler_post(AvahiQueryScheduler *s, AvahiKey *key, int immediately, unsigned *ret_id) {
    struct timeval tv;
    AvahiQueryJob *qj;

    assert(s);
    assert(key);

    if ((qj = find_history_job(s, key)))
        return 0;

    avahi_elapse_time(&tv, immediately ? 0 : AVAHI_QUERY_DEFER_MSEC, 0);

    if ((qj = find_scheduled_job(s, key))) {
        /* Duplicate questions suppression */

        if (avahi_timeval_compare(&tv, &qj->delivery) < 0) {
            /* If the new entry should be scheduled earlier,
             * update the old entry */
            qj->delivery = tv;
            avahi_time_event_update(qj->time_event, &qj->delivery);
        }

        qj->n_posted++;

    } else {

        if (!(qj = job_new(s, key, 0)))
            return 0; /* OOM */

        qj->delivery = tv;
        qj->time_event = avahi_time_event_new(s->time_event_queue, &qj->delivery, elapse_callback, qj);
    }

    if (ret_id)
        *ret_id = qj->id;

    return 1;
}

void avahi_query_scheduler_incoming(AvahiQueryScheduler *s, AvahiKey *key) {
    AvahiQueryJob *qj;

    assert(s);
    assert(key);

    /* This function is called whenever an incoming query was
     * received. We drop scheduled queries that match. The keyword is
     * "DUPLICATE QUESTION SUPPRESION". */

    if ((qj = find_scheduled_job(s, key))) {
        job_mark_done(s, qj);
        return;
    }

    /* Look if there's a history job for this key. If there is, just
     * update the elapse time */
    if (!(qj = find_history_job(s, key)))
        if (!(qj = job_new(s, key, 1)))
            return; /* OOM */

    gettimeofday(&qj->delivery, NULL);
    job_set_elapse_time(s, qj, AVAHI_QUERY_HISTORY_MSEC, 0);
}

int avahi_query_scheduler_withdraw_by_id(AvahiQueryScheduler *s, unsigned id) {
    AvahiQueryJob *qj;

    assert(s);

    /* Very short lived queries can withdraw an already scheduled item
     * from the queue using this function, simply by passing the id
     * returned by avahi_query_scheduler_post(). */

    for (qj = s->jobs; qj; qj = qj->jobs_next) {
        assert(!qj->done);

        if (qj->id == id) {
            /* Entry found */

            assert(qj->n_posted >= 1);

            if (--qj->n_posted <= 0) {

                /* We withdraw this job only if the calling object was
                 * the only remaining poster. (Usually this is the
                 * case since there should exist only one querier per
                 * key, but there are exceptions, notably reflected
                 * traffic.) */

                job_free(s, qj);
                return 1;
            }
        }
    }

    return 0;
}
