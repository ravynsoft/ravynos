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

#include "response-sched.h"
#include "log.h"
#include "rr-util.h"

/* Local packets are supressed this long after sending them */
#define AVAHI_RESPONSE_HISTORY_MSEC 500

/* Local packets are deferred this long before sending them */
#define AVAHI_RESPONSE_DEFER_MSEC 20

/* Additional jitter for deferred packets */
#define AVAHI_RESPONSE_JITTER_MSEC 100

/* Remote packets can suppress local traffic as long as this value */
#define AVAHI_RESPONSE_SUPPRESS_MSEC 700

typedef struct AvahiResponseJob AvahiResponseJob;

typedef enum {
    AVAHI_SCHEDULED,
    AVAHI_DONE,
    AVAHI_SUPPRESSED
} AvahiResponseJobState;

struct AvahiResponseJob {
    AvahiResponseScheduler *scheduler;
    AvahiTimeEvent *time_event;

    AvahiResponseJobState state;
    struct timeval delivery;

    AvahiRecord *record;
    int flush_cache;
    AvahiAddress querier;
    int querier_valid;

    AVAHI_LLIST_FIELDS(AvahiResponseJob, jobs);
};

struct AvahiResponseScheduler {
    AvahiInterface *interface;
    AvahiTimeEventQueue *time_event_queue;

    AVAHI_LLIST_HEAD(AvahiResponseJob, jobs);
    AVAHI_LLIST_HEAD(AvahiResponseJob, history);
    AVAHI_LLIST_HEAD(AvahiResponseJob, suppressed);
};

static AvahiResponseJob* job_new(AvahiResponseScheduler *s, AvahiRecord *record, AvahiResponseJobState state) {
    AvahiResponseJob *rj;

    assert(s);
    assert(record);

    if (!(rj = avahi_new(AvahiResponseJob, 1))) {
        avahi_log_error(__FILE__": Out of memory");
        return NULL;
    }

    rj->scheduler = s;
    rj->record = avahi_record_ref(record);
    rj->time_event = NULL;
    rj->flush_cache = 0;
    rj->querier_valid = 0;

    if ((rj->state = state) == AVAHI_SCHEDULED)
        AVAHI_LLIST_PREPEND(AvahiResponseJob, jobs, s->jobs, rj);
    else if (rj->state == AVAHI_DONE)
        AVAHI_LLIST_PREPEND(AvahiResponseJob, jobs, s->history, rj);
    else  /* rj->state == AVAHI_SUPPRESSED */
        AVAHI_LLIST_PREPEND(AvahiResponseJob, jobs, s->suppressed, rj);

    return rj;
}

static void job_free(AvahiResponseScheduler *s, AvahiResponseJob *rj) {
    assert(s);
    assert(rj);

    if (rj->time_event)
        avahi_time_event_free(rj->time_event);

    if (rj->state == AVAHI_SCHEDULED)
        AVAHI_LLIST_REMOVE(AvahiResponseJob, jobs, s->jobs, rj);
    else if (rj->state == AVAHI_DONE)
        AVAHI_LLIST_REMOVE(AvahiResponseJob, jobs, s->history, rj);
    else /* rj->state == AVAHI_SUPPRESSED */
        AVAHI_LLIST_REMOVE(AvahiResponseJob, jobs, s->suppressed, rj);

    avahi_record_unref(rj->record);
    avahi_free(rj);
}

static void elapse_callback(AvahiTimeEvent *e, void* data);

static void job_set_elapse_time(AvahiResponseScheduler *s, AvahiResponseJob *rj, unsigned msec, unsigned jitter) {
    struct timeval tv;

    assert(s);
    assert(rj);

    avahi_elapse_time(&tv, msec, jitter);

    if (rj->time_event)
        avahi_time_event_update(rj->time_event, &tv);
    else
        rj->time_event = avahi_time_event_new(s->time_event_queue, &tv, elapse_callback, rj);
}

static void job_mark_done(AvahiResponseScheduler *s, AvahiResponseJob *rj) {
    assert(s);
    assert(rj);

    assert(rj->state == AVAHI_SCHEDULED);

    AVAHI_LLIST_REMOVE(AvahiResponseJob, jobs, s->jobs, rj);
    AVAHI_LLIST_PREPEND(AvahiResponseJob, jobs, s->history, rj);

    rj->state = AVAHI_DONE;

    job_set_elapse_time(s, rj, AVAHI_RESPONSE_HISTORY_MSEC, 0);

    gettimeofday(&rj->delivery, NULL);
}

AvahiResponseScheduler *avahi_response_scheduler_new(AvahiInterface *i) {
    AvahiResponseScheduler *s;
    assert(i);

    if (!(s = avahi_new(AvahiResponseScheduler, 1))) {
        avahi_log_error(__FILE__": Out of memory");
        return NULL;
    }

    s->interface = i;
    s->time_event_queue = i->monitor->server->time_event_queue;

    AVAHI_LLIST_HEAD_INIT(AvahiResponseJob, s->jobs);
    AVAHI_LLIST_HEAD_INIT(AvahiResponseJob, s->history);
    AVAHI_LLIST_HEAD_INIT(AvahiResponseJob, s->suppressed);

    return s;
}

void avahi_response_scheduler_free(AvahiResponseScheduler *s) {
    assert(s);

    avahi_response_scheduler_clear(s);
    avahi_free(s);
}

void avahi_response_scheduler_clear(AvahiResponseScheduler *s) {
    assert(s);

    while (s->jobs)
        job_free(s, s->jobs);
    while (s->history)
        job_free(s, s->history);
    while (s->suppressed)
        job_free(s, s->suppressed);
}

static void enumerate_aux_records_callback(AVAHI_GCC_UNUSED AvahiServer *s, AvahiRecord *r, int flush_cache, void* userdata) {
    AvahiResponseJob *rj = userdata;

    assert(r);
    assert(rj);

    avahi_response_scheduler_post(rj->scheduler, r, flush_cache, rj->querier_valid ? &rj->querier : NULL, 0);
}

static int packet_add_response_job(AvahiResponseScheduler *s, AvahiDnsPacket *p, AvahiResponseJob *rj) {
    assert(s);
    assert(p);
    assert(rj);

    /* Try to add this record to the packet */
    if (!avahi_dns_packet_append_record(p, rj->record, rj->flush_cache, 0))
        return 0;

    /* Ok, this record will definitely be sent, so schedule the
     * auxilliary packets, too */
    avahi_server_enumerate_aux_records(s->interface->monitor->server, s->interface, rj->record, enumerate_aux_records_callback, rj);
    job_mark_done(s, rj);

    return 1;
}

static void send_response_packet(AvahiResponseScheduler *s, AvahiResponseJob *rj) {
    AvahiDnsPacket *p;
    unsigned n;

    assert(s);
    assert(rj);

    if (!(p = avahi_dns_packet_new_response(s->interface->hardware->mtu, 1)))
        return; /* OOM */
    n = 1;

    /* Put it in the packet. */
    if (packet_add_response_job(s, p, rj)) {

        /* Try to fill up packet with more responses, if available */
        while (s->jobs) {

            if (!packet_add_response_job(s, p, s->jobs))
                break;

            n++;
        }

    } else {
        size_t size;

        avahi_dns_packet_free(p);

        /* OK, the packet was too small, so create one that fits */
        size = avahi_record_get_estimate_size(rj->record) + AVAHI_DNS_PACKET_HEADER_SIZE;

        if (!(p = avahi_dns_packet_new_response(size + AVAHI_DNS_PACKET_EXTRA_SIZE, 1)))
            return; /* OOM */

        if (!packet_add_response_job(s, p, rj)) {
            avahi_dns_packet_free(p);

            avahi_log_warn("Record too large, cannot send");
            job_mark_done(s, rj);
            return;
        }
    }

    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_ANCOUNT, n);
    avahi_interface_send_packet(s->interface, p);
    avahi_dns_packet_free(p);
}

static void elapse_callback(AVAHI_GCC_UNUSED AvahiTimeEvent *e, void* data) {
    AvahiResponseJob *rj = data;

    assert(rj);

    if (rj->state == AVAHI_DONE || rj->state == AVAHI_SUPPRESSED)
        job_free(rj->scheduler, rj);         /* Lets drop this entry */
    else
        send_response_packet(rj->scheduler, rj);
}

static AvahiResponseJob* find_scheduled_job(AvahiResponseScheduler *s, AvahiRecord *record) {
    AvahiResponseJob *rj;

    assert(s);
    assert(record);

    for (rj = s->jobs; rj; rj = rj->jobs_next) {
        assert(rj->state == AVAHI_SCHEDULED);

        if (avahi_record_equal_no_ttl(rj->record, record))
            return rj;
    }

    return NULL;
}

static AvahiResponseJob* find_history_job(AvahiResponseScheduler *s, AvahiRecord *record) {
    AvahiResponseJob *rj;

    assert(s);
    assert(record);

    for (rj = s->history; rj; rj = rj->jobs_next) {
        assert(rj->state == AVAHI_DONE);

        if (avahi_record_equal_no_ttl(rj->record, record)) {
            /* Check whether this entry is outdated */

/*             avahi_log_debug("history age: %u", (unsigned) (avahi_age(&rj->delivery)/1000)); */

            if (avahi_age(&rj->delivery)/1000 > AVAHI_RESPONSE_HISTORY_MSEC) {
                /* it is outdated, so let's remove it */
                job_free(s, rj);
                return NULL;
            }

            return rj;
        }
    }

    return NULL;
}

static AvahiResponseJob* find_suppressed_job(AvahiResponseScheduler *s, AvahiRecord *record, const AvahiAddress *querier) {
    AvahiResponseJob *rj;

    assert(s);
    assert(record);
    assert(querier);

    for (rj = s->suppressed; rj; rj = rj->jobs_next) {
        assert(rj->state == AVAHI_SUPPRESSED);
        assert(rj->querier_valid);

        if (avahi_record_equal_no_ttl(rj->record, record) &&
            avahi_address_cmp(&rj->querier, querier) == 0) {
            /* Check whether this entry is outdated */

            if (avahi_age(&rj->delivery) > AVAHI_RESPONSE_SUPPRESS_MSEC*1000) {
                /* it is outdated, so let's remove it */
                job_free(s, rj);
                return NULL;
            }

            return rj;
        }
    }

    return NULL;
}

int avahi_response_scheduler_post(AvahiResponseScheduler *s, AvahiRecord *record, int flush_cache, const AvahiAddress *querier, int immediately) {
    AvahiResponseJob *rj;
    struct timeval tv;
/*     char *t; */

    assert(s);
    assert(record);

    assert(!avahi_key_is_pattern(record->key));

/*     t = avahi_record_to_string(record); */
/*     avahi_log_debug("post %i %s", immediately, t); */
/*     avahi_free(t); */

    /* Check whether this response is suppressed */
    if (querier &&
        (rj = find_suppressed_job(s, record, querier)) &&
        avahi_record_is_goodbye(record) == avahi_record_is_goodbye(rj->record) &&
        rj->record->ttl >= record->ttl/2) {

/*         avahi_log_debug("Response suppressed by known answer suppression.");  */
        return 0;
    }

    /* Check if we already sent this response recently */
    if ((rj = find_history_job(s, record))) {

        if (avahi_record_is_goodbye(record) == avahi_record_is_goodbye(rj->record) &&
            rj->record->ttl >= record->ttl/2 &&
            (rj->flush_cache || !flush_cache)) {
/*             avahi_log_debug("Response suppressed by local duplicate suppression (history)");  */
            return 0;
        }

        /* Outdated ... */
        job_free(s, rj);
    }

    avahi_elapse_time(&tv, immediately ? 0 : AVAHI_RESPONSE_DEFER_MSEC, immediately ? 0 : AVAHI_RESPONSE_JITTER_MSEC);

    if ((rj = find_scheduled_job(s, record))) {
/*          avahi_log_debug("Response suppressed by local duplicate suppression (scheduled)"); */

        /* Update a little ... */

        /* Update the time if the new is prior to the old */
        if (avahi_timeval_compare(&tv, &rj->delivery) < 0) {
            rj->delivery = tv;
            avahi_time_event_update(rj->time_event, &rj->delivery);
        }

        /* Update the flush cache bit */
        if (flush_cache)
            rj->flush_cache = 1;

        /* Update the querier field */
        if (!querier || (rj->querier_valid && avahi_address_cmp(querier, &rj->querier) != 0))
            rj->querier_valid = 0;

        /* Update record data (just for the TTL) */
        avahi_record_unref(rj->record);
        rj->record = avahi_record_ref(record);

        return 1;
    } else {
/*         avahi_log_debug("Accepted new response job.");  */

        /* Create a new job and schedule it */
        if (!(rj = job_new(s, record, AVAHI_SCHEDULED)))
            return 0; /* OOM */

        rj->delivery = tv;
        rj->time_event = avahi_time_event_new(s->time_event_queue, &rj->delivery, elapse_callback, rj);
        rj->flush_cache = flush_cache;

        if ((rj->querier_valid = !!querier))
            rj->querier = *querier;

        return 1;
    }
}

void avahi_response_scheduler_incoming(AvahiResponseScheduler *s, AvahiRecord *record, int flush_cache) {
    AvahiResponseJob *rj;
    assert(s);

    /* This function is called whenever an incoming response was
     * receieved. We drop scheduled responses which match here. The
     * keyword is "DUPLICATE ANSWER SUPPRESION". */

    if ((rj = find_scheduled_job(s, record))) {

        if ((!rj->flush_cache || flush_cache) &&    /* flush cache bit was set correctly */
            avahi_record_is_goodbye(record) == avahi_record_is_goodbye(rj->record) &&   /* both goodbye packets, or both not */
            record->ttl >= rj->record->ttl/2) {     /* sensible TTL */

            /* A matching entry was found, so let's mark it done */
/*             avahi_log_debug("Response suppressed by distributed duplicate suppression"); */
            job_mark_done(s, rj);
        }

        return;
    }

    if ((rj = find_history_job(s, record))) {
        /* Found a history job, let's update it */
        avahi_record_unref(rj->record);
        rj->record = avahi_record_ref(record);
    } else
        /* Found no existing history job, so let's create a new one */
        if (!(rj = job_new(s, record, AVAHI_DONE)))
            return; /* OOM */

    rj->flush_cache = flush_cache;
    rj->querier_valid = 0;

    gettimeofday(&rj->delivery, NULL);
    job_set_elapse_time(s, rj, AVAHI_RESPONSE_HISTORY_MSEC, 0);
}

void avahi_response_scheduler_suppress(AvahiResponseScheduler *s, AvahiRecord *record, const AvahiAddress *querier) {
    AvahiResponseJob *rj;

    assert(s);
    assert(record);
    assert(querier);

    if ((rj = find_scheduled_job(s, record))) {

        if (rj->querier_valid && avahi_address_cmp(querier, &rj->querier) == 0 && /* same originator */
            avahi_record_is_goodbye(record) == avahi_record_is_goodbye(rj->record) && /* both goodbye packets, or both not */
            record->ttl >= rj->record->ttl/2) {                                  /* sensible TTL */

            /* A matching entry was found, so let's drop it */
/*             avahi_log_debug("Known answer suppression active!"); */
            job_free(s, rj);
        }
    }

    if ((rj = find_suppressed_job(s, record, querier))) {

        /* Let's update the old entry */
        avahi_record_unref(rj->record);
        rj->record = avahi_record_ref(record);

    } else {

        /* Create a new entry */
        if (!(rj = job_new(s, record, AVAHI_SUPPRESSED)))
            return; /* OOM */
        rj->querier_valid = 1;
        rj->querier = *querier;
    }

    gettimeofday(&rj->delivery, NULL);
    job_set_elapse_time(s, rj, AVAHI_RESPONSE_SUPPRESS_MSEC, 0);
}

void avahi_response_scheduler_force(AvahiResponseScheduler *s) {
    assert(s);

    /* Send all scheduled responses immediately */
    while (s->jobs)
        send_response_packet(s, s->jobs);
}
