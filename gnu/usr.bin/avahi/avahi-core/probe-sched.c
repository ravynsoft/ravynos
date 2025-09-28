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

#include <avahi-common/domain.h>
#include <avahi-common/timeval.h>
#include <avahi-common/malloc.h>

#include "probe-sched.h"
#include "log.h"
#include "rr-util.h"

#define AVAHI_PROBE_HISTORY_MSEC 150
#define AVAHI_PROBE_DEFER_MSEC 50

typedef struct AvahiProbeJob AvahiProbeJob;

struct AvahiProbeJob {
    AvahiProbeScheduler *scheduler;
    AvahiTimeEvent *time_event;

    int chosen; /* Use for packet assembling */
    int done;
    struct timeval delivery;

    AvahiRecord *record;

    AVAHI_LLIST_FIELDS(AvahiProbeJob, jobs);
};

struct AvahiProbeScheduler {
    AvahiInterface *interface;
    AvahiTimeEventQueue *time_event_queue;

    AVAHI_LLIST_HEAD(AvahiProbeJob, jobs);
    AVAHI_LLIST_HEAD(AvahiProbeJob, history);
};

static AvahiProbeJob* job_new(AvahiProbeScheduler *s, AvahiRecord *record, int done) {
    AvahiProbeJob *pj;

    assert(s);
    assert(record);

    if (!(pj = avahi_new(AvahiProbeJob, 1))) {
        avahi_log_error(__FILE__": Out of memory");
        return NULL; /* OOM */
    }

    pj->scheduler = s;
    pj->record = avahi_record_ref(record);
    pj->time_event = NULL;
    pj->chosen = 0;

    if ((pj->done = done))
        AVAHI_LLIST_PREPEND(AvahiProbeJob, jobs, s->history, pj);
    else
        AVAHI_LLIST_PREPEND(AvahiProbeJob, jobs, s->jobs, pj);

    return pj;
}

static void job_free(AvahiProbeScheduler *s, AvahiProbeJob *pj) {
    assert(pj);

    if (pj->time_event)
        avahi_time_event_free(pj->time_event);

    if (pj->done)
        AVAHI_LLIST_REMOVE(AvahiProbeJob, jobs, s->history, pj);
    else
        AVAHI_LLIST_REMOVE(AvahiProbeJob, jobs, s->jobs, pj);

    avahi_record_unref(pj->record);
    avahi_free(pj);
}

static void elapse_callback(AvahiTimeEvent *e, void* data);

static void job_set_elapse_time(AvahiProbeScheduler *s, AvahiProbeJob *pj, unsigned msec, unsigned jitter) {
    struct timeval tv;

    assert(s);
    assert(pj);

    avahi_elapse_time(&tv, msec, jitter);

    if (pj->time_event)
        avahi_time_event_update(pj->time_event, &tv);
    else
        pj->time_event = avahi_time_event_new(s->time_event_queue, &tv, elapse_callback, pj);
}

static void job_mark_done(AvahiProbeScheduler *s, AvahiProbeJob *pj) {
    assert(s);
    assert(pj);

    assert(!pj->done);

    AVAHI_LLIST_REMOVE(AvahiProbeJob, jobs, s->jobs, pj);
    AVAHI_LLIST_PREPEND(AvahiProbeJob, jobs, s->history, pj);

    pj->done = 1;

    job_set_elapse_time(s, pj, AVAHI_PROBE_HISTORY_MSEC, 0);
    gettimeofday(&pj->delivery, NULL);
}

AvahiProbeScheduler *avahi_probe_scheduler_new(AvahiInterface *i) {
    AvahiProbeScheduler *s;

    assert(i);

    if (!(s = avahi_new(AvahiProbeScheduler, 1))) {
        avahi_log_error(__FILE__": Out of memory");
        return NULL;
    }

    s->interface = i;
    s->time_event_queue = i->monitor->server->time_event_queue;

    AVAHI_LLIST_HEAD_INIT(AvahiProbeJob, s->jobs);
    AVAHI_LLIST_HEAD_INIT(AvahiProbeJob, s->history);

    return s;
}

void avahi_probe_scheduler_free(AvahiProbeScheduler *s) {
    assert(s);

    avahi_probe_scheduler_clear(s);
    avahi_free(s);
}

void avahi_probe_scheduler_clear(AvahiProbeScheduler *s) {
    assert(s);

    while (s->jobs)
        job_free(s, s->jobs);
    while (s->history)
        job_free(s, s->history);
}

static int packet_add_probe_query(AvahiProbeScheduler *s, AvahiDnsPacket *p, AvahiProbeJob *pj) {
    size_t size;
    AvahiKey *k;
    int b;

    assert(s);
    assert(p);
    assert(pj);

    assert(!pj->chosen);

    /* Estimate the size for this record */
    size =
        avahi_key_get_estimate_size(pj->record->key) +
        avahi_record_get_estimate_size(pj->record);

    /* Too large */
    if (size > avahi_dns_packet_reserved_space(p))
        return 0;

    /* Create the probe query */
    if (!(k = avahi_key_new(pj->record->key->name, pj->record->key->clazz, AVAHI_DNS_TYPE_ANY)))
        return 0; /* OOM */

    b = !!avahi_dns_packet_append_key(p, k, 0);
    assert(b);

    /* reserve size for record data */
    avahi_dns_packet_reserve_size(p, avahi_record_get_estimate_size(pj->record));

    /* Mark this job for addition to the packet */
    pj->chosen = 1;

    /* Scan for more jobs whith matching key pattern */
    for (pj = s->jobs; pj; pj = pj->jobs_next) {
        if (pj->chosen)
            continue;

        /* Does the record match the probe? */
        if (k->clazz != pj->record->key->clazz || !avahi_domain_equal(k->name, pj->record->key->name))
            continue;

        /* This job wouldn't fit in */
        if (avahi_record_get_estimate_size(pj->record) > avahi_dns_packet_reserved_space(p))
            break;

   /* reserve size for record data */
   avahi_dns_packet_reserve_size(p, avahi_record_get_estimate_size(pj->record));

        /* Mark this job for addition to the packet */
        pj->chosen = 1;
    }

    avahi_key_unref(k);

    return 1;
}

static void elapse_callback(AVAHI_GCC_UNUSED AvahiTimeEvent *e, void* data) {
    AvahiProbeJob *pj = data, *next;
    AvahiProbeScheduler *s;
    AvahiDnsPacket *p;
    unsigned n;

    assert(pj);
    s = pj->scheduler;

    if (pj->done) {
        /* Lets remove it  from the history */
        job_free(s, pj);
        return;
    }

    if (!(p = avahi_dns_packet_new_query(s->interface->hardware->mtu)))
        return; /* OOM */
    n = 1;

    /* Add the import probe */
    if (!packet_add_probe_query(s, p, pj)) {
        size_t size;
        AvahiKey *k;
        int b;

        avahi_dns_packet_free(p);

        /* The probe didn't fit in the package, so let's allocate a larger one */

        size =
            avahi_key_get_estimate_size(pj->record->key) +
            avahi_record_get_estimate_size(pj->record) +
            AVAHI_DNS_PACKET_HEADER_SIZE;

        if (!(p = avahi_dns_packet_new_query(size + AVAHI_DNS_PACKET_EXTRA_SIZE)))
            return; /* OOM */

        if (!(k = avahi_key_new(pj->record->key->name, pj->record->key->clazz, AVAHI_DNS_TYPE_ANY))) {
            avahi_dns_packet_free(p);
            return;  /* OOM */
        }

        b = avahi_dns_packet_append_key(p, k, 0) && avahi_dns_packet_append_record(p, pj->record, 0, 0);
        avahi_key_unref(k);

        if (b) {
            avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_NSCOUNT, 1);
            avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_QDCOUNT, 1);
            avahi_interface_send_packet(s->interface, p);
        } else
            avahi_log_warn("Probe record too large, cannot send");

        avahi_dns_packet_free(p);
        job_mark_done(s, pj);

        return;
    }

    /* Try to fill up packet with more probes, if available */
    for (pj = s->jobs; pj; pj = pj->jobs_next) {

        if (pj->chosen)
            continue;

        if (!packet_add_probe_query(s, p, pj))
            break;

        n++;
    }

    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_QDCOUNT, n);

    n = 0;

    /* Now add the chosen records to the authorative section */
    for (pj = s->jobs; pj; pj = next) {

        next = pj->jobs_next;

        if (!pj->chosen)
            continue;

        if (!avahi_dns_packet_append_record(p, pj->record, 0, 0)) {
/*             avahi_log_warn("Bad probe size estimate!"); */

            /* Unmark all following jobs */
            for (; pj; pj = pj->jobs_next)
                pj->chosen = 0;

            break;
        }

        job_mark_done(s, pj);

        n ++;
    }

    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_NSCOUNT, n);

    /* Send it now */
    avahi_interface_send_packet(s->interface, p);
    avahi_dns_packet_free(p);
}

static AvahiProbeJob* find_scheduled_job(AvahiProbeScheduler *s, AvahiRecord *record) {
    AvahiProbeJob *pj;

    assert(s);
    assert(record);

    for (pj = s->jobs; pj; pj = pj->jobs_next) {
        assert(!pj->done);

        if (avahi_record_equal_no_ttl(pj->record, record))
            return pj;
    }

    return NULL;
}

static AvahiProbeJob* find_history_job(AvahiProbeScheduler *s, AvahiRecord *record) {
    AvahiProbeJob *pj;

    assert(s);
    assert(record);

    for (pj = s->history; pj; pj = pj->jobs_next) {
        assert(pj->done);

        if (avahi_record_equal_no_ttl(pj->record, record)) {
            /* Check whether this entry is outdated */

            if (avahi_age(&pj->delivery) > AVAHI_PROBE_HISTORY_MSEC*1000) {
                /* it is outdated, so let's remove it */
                job_free(s, pj);
                return NULL;
            }

            return pj;
        }
    }

    return NULL;
}

int avahi_probe_scheduler_post(AvahiProbeScheduler *s, AvahiRecord *record, int immediately) {
    AvahiProbeJob *pj;
    struct timeval tv;

    assert(s);
    assert(record);
    assert(!avahi_key_is_pattern(record->key));

    if ((pj = find_history_job(s, record)))
        return 0;

    avahi_elapse_time(&tv, immediately ? 0 : AVAHI_PROBE_DEFER_MSEC, 0);

    if ((pj = find_scheduled_job(s, record))) {

        if (avahi_timeval_compare(&tv, &pj->delivery) < 0) {
            /* If the new entry should be scheduled earlier, update the old entry */
            pj->delivery = tv;
            avahi_time_event_update(pj->time_event, &pj->delivery);
        }

        return 1;
    } else {
        /* Create a new job and schedule it */
        if (!(pj = job_new(s, record, 0)))
            return 0; /* OOM */

        pj->delivery = tv;
        pj->time_event = avahi_time_event_new(s->time_event_queue, &pj->delivery, elapse_callback, pj);


/*     avahi_log_debug("Accepted new probe job."); */

        return 1;
    }
}
