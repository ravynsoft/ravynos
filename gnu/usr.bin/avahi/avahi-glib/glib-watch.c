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

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>
#include <avahi-common/timeval.h>

#include "glib-watch.h"

struct AvahiWatch {
    AvahiGLibPoll *glib_poll;
    int dead;

    GPollFD pollfd;
    int pollfd_added;

    AvahiWatchCallback callback;
    void *userdata;

    AVAHI_LLIST_FIELDS(AvahiWatch, watches);
};

struct AvahiTimeout {
    AvahiGLibPoll *glib_poll;
    gboolean dead;

    gboolean enabled;
    struct timeval expiry;

    AvahiTimeoutCallback callback;
    void  *userdata;

    AVAHI_LLIST_FIELDS(AvahiTimeout, timeouts);
};

struct AvahiGLibPoll {
    GSource source;
    AvahiPoll api;
    GMainContext *context;

    gboolean timeout_req_cleanup;
    gboolean watch_req_cleanup;

    AVAHI_LLIST_HEAD(AvahiWatch, watches);
    AVAHI_LLIST_HEAD(AvahiTimeout, timeouts);
};

static void destroy_watch(AvahiWatch *w) {
    assert(w);

    if (w->pollfd_added)
        g_source_remove_poll(&w->glib_poll->source, &w->pollfd);

    AVAHI_LLIST_REMOVE(AvahiWatch, watches, w->glib_poll->watches, w);

    avahi_free(w);
}

static void cleanup_watches(AvahiGLibPoll *g, int all) {
    AvahiWatch *w, *next;
    assert(g);

    for (w = g->watches; w; w = next) {
        next = w->watches_next;

        if (all || w->dead)
            destroy_watch(w);
    }

    g->watch_req_cleanup = FALSE;
}

static gushort map_events_to_glib(AvahiWatchEvent events) {
    return
        (events & AVAHI_WATCH_IN ? G_IO_IN : 0) |
        (events & AVAHI_WATCH_OUT ? G_IO_OUT : 0) |
        (events & AVAHI_WATCH_ERR ? G_IO_ERR : 0) |
        (events & AVAHI_WATCH_HUP ? G_IO_HUP : 0);
}

static AvahiWatchEvent map_events_from_glib(gushort events) {
    return
        (events & G_IO_IN ? AVAHI_WATCH_IN : 0) |
        (events & G_IO_OUT ? AVAHI_WATCH_OUT : 0) |
        (events & G_IO_ERR ? AVAHI_WATCH_ERR : 0) |
        (events & G_IO_HUP ? AVAHI_WATCH_HUP : 0);
}

static AvahiWatch* watch_new(const AvahiPoll *api, int fd, AvahiWatchEvent events, AvahiWatchCallback callback, void *userdata) {
    AvahiWatch *w;
    AvahiGLibPoll *g;

    assert(api);
    assert(fd >= 0);
    assert(callback);

    g = api->userdata;
    assert(g);

    if (!(w = avahi_new(AvahiWatch, 1)))
        return NULL;

    w->glib_poll = g;
    w->pollfd.fd = fd;
    w->pollfd.events = map_events_to_glib(events);
    w->pollfd.revents = 0;
    w->callback = callback;
    w->userdata = userdata;
    w->dead = FALSE;

    g_source_add_poll(&g->source, &w->pollfd);
    w->pollfd_added = TRUE;

    AVAHI_LLIST_PREPEND(AvahiWatch, watches, g->watches, w);

    return w;
}

static void watch_update(AvahiWatch *w, AvahiWatchEvent events) {
    assert(w);
    assert(!w->dead);

    w->pollfd.events = map_events_to_glib(events);
}

static AvahiWatchEvent watch_get_events(AvahiWatch *w) {
    assert(w);
    assert(!w->dead);

    return map_events_from_glib(w->pollfd.revents);
}

static void watch_free(AvahiWatch *w) {
    assert(w);
    assert(!w->dead);

    if (w->pollfd_added) {
        g_source_remove_poll(&w->glib_poll->source, &w->pollfd);
        w->pollfd_added = FALSE;
    }

    w->dead = TRUE;
    w->glib_poll->watch_req_cleanup = TRUE;
}

static AvahiTimeout* timeout_new(const AvahiPoll *api, const struct timeval *tv, AvahiTimeoutCallback callback, void *userdata) {
    AvahiTimeout *t;
    AvahiGLibPoll *g;

    assert(api);
    assert(callback);

    g = api->userdata;
    assert(g);

    if (!(t = avahi_new(AvahiTimeout, 1)))
        return NULL;

    t->glib_poll = g;
    t->dead = FALSE;

    if ((t->enabled = !!tv))
        t->expiry = *tv;

    t->callback = callback;
    t->userdata = userdata;

    AVAHI_LLIST_PREPEND(AvahiTimeout, timeouts, g->timeouts, t);

    return t;
}

static void timeout_update(AvahiTimeout *t, const struct timeval *tv) {
    assert(t);
    assert(!t->dead);

    if ((t->enabled = !!tv))
        t->expiry = *tv;
}

static void timeout_free(AvahiTimeout *t) {
    assert(t);
    assert(!t->dead);

    t->dead = TRUE;
    t->glib_poll->timeout_req_cleanup = TRUE;
}

static void destroy_timeout(AvahiTimeout *t) {
    assert(t);

    AVAHI_LLIST_REMOVE(AvahiTimeout, timeouts, t->glib_poll->timeouts, t);
    avahi_free(t);
}

static void cleanup_timeouts(AvahiGLibPoll *g, int all) {
    AvahiTimeout *t, *next;
    assert(g);

    for (t = g->timeouts; t; t = next) {
        next = t->timeouts_next;

        if (all || t->dead)
            destroy_timeout(t);
    }

    g->timeout_req_cleanup = FALSE;
}

static AvahiTimeout* find_next_timeout(AvahiGLibPoll *g) {
    AvahiTimeout *t, *n = NULL;
    assert(g);

    for (t = g->timeouts; t; t = t->timeouts_next) {

        if (t->dead || !t->enabled)
            continue;

        if (!n || avahi_timeval_compare(&t->expiry, &n->expiry) < 0)
            n = t;
    }

    return n;
}

static void start_timeout_callback(AvahiTimeout *t) {
    assert(t);
    assert(!t->dead);
    assert(t->enabled);

    t->enabled = 0;
    t->callback(t, t->userdata);
}

static gboolean prepare_func(GSource *source, gint *timeout) {
    AvahiGLibPoll *g = (AvahiGLibPoll*) source;
    AvahiTimeout *next_timeout;

    g_assert(g);
    g_assert(timeout);

    if (g->watch_req_cleanup)
        cleanup_watches(g, 0);

    if (g->timeout_req_cleanup)
        cleanup_timeouts(g, 0);

    if ((next_timeout = find_next_timeout(g))) {
        GTimeVal now;
        struct timeval tvnow;
        AvahiUsec usec;

        g_source_get_current_time(source, &now);
        tvnow.tv_sec = now.tv_sec;
        tvnow.tv_usec = now.tv_usec;

        usec = avahi_timeval_diff(&next_timeout->expiry, &tvnow);

        if (usec <= 0) {
	   *timeout = 0;
            return TRUE;
	}

        *timeout = (gint) (usec / 1000);
    } else
        *timeout = -1;

    return FALSE;
}

static gboolean check_func(GSource *source) {
    AvahiGLibPoll *g = (AvahiGLibPoll*) source;
    AvahiWatch *w;
    AvahiTimeout *next_timeout;

    g_assert(g);

    if ((next_timeout = find_next_timeout(g))) {
        GTimeVal now;
        struct timeval tvnow;
        g_source_get_current_time(source, &now);
        tvnow.tv_sec = now.tv_sec;
        tvnow.tv_usec = now.tv_usec;

        if (avahi_timeval_compare(&next_timeout->expiry, &tvnow) <= 0)
            return TRUE;
    }

    for (w = g->watches; w; w = w->watches_next)
        if (w->pollfd.revents > 0)
            return TRUE;

    return FALSE;
}

static gboolean dispatch_func(GSource *source, AVAHI_GCC_UNUSED GSourceFunc callback, AVAHI_GCC_UNUSED gpointer userdata) {
    AvahiGLibPoll* g = (AvahiGLibPoll*) source;
    AvahiWatch *w;
    AvahiTimeout *next_timeout;

    g_assert(g);

    if ((next_timeout = find_next_timeout(g))) {
        GTimeVal now;
        struct timeval tvnow;
        g_source_get_current_time(source, &now);
        tvnow.tv_sec = now.tv_sec;
        tvnow.tv_usec = now.tv_usec;

        if (avahi_timeval_compare(&next_timeout->expiry, &tvnow) < 0) {
            start_timeout_callback(next_timeout);
            return TRUE;
        }
    }

    for (w = g->watches; w; w = w->watches_next)
        if (w->pollfd.revents > 0) {
            assert(w->callback);
            w->callback(w, w->pollfd.fd, map_events_from_glib(w->pollfd.revents), w->userdata);
            w->pollfd.revents = 0;
            return TRUE;
        }

    return TRUE;
}

AvahiGLibPoll *avahi_glib_poll_new(GMainContext *context, gint priority) {
    AvahiGLibPoll *g;

    static GSourceFuncs source_funcs = {
        prepare_func,
        check_func,
        dispatch_func,
        NULL,
        NULL,
        NULL
    };

    g = (AvahiGLibPoll*) g_source_new(&source_funcs, sizeof(AvahiGLibPoll));
    g_main_context_ref(g->context = context ? context : g_main_context_default());

    g->api.userdata = g;

    g->api.watch_new = watch_new;
    g->api.watch_free = watch_free;
    g->api.watch_update = watch_update;
    g->api.watch_get_events = watch_get_events;

    g->api.timeout_new = timeout_new;
    g->api.timeout_free = timeout_free;
    g->api.timeout_update = timeout_update;

    g->watch_req_cleanup = FALSE;
    g->timeout_req_cleanup = FALSE;

    AVAHI_LLIST_HEAD_INIT(AvahiWatch, g->watches);
    AVAHI_LLIST_HEAD_INIT(AvahiTimeout, g->timeouts);

    g_source_attach(&g->source, g->context);
    g_source_set_priority(&g->source, priority);
    g_source_set_can_recurse(&g->source, FALSE);

    return g;
}

void avahi_glib_poll_free(AvahiGLibPoll *g) {
    GSource *s = &g->source;
    assert(g);

    cleanup_watches(g, 1);
    cleanup_timeouts(g, 1);

    g_main_context_unref(g->context);
    g_source_destroy(s);
    g_source_unref(s);
}

const AvahiPoll* avahi_glib_poll_get(AvahiGLibPoll *g) {
    assert(g);

    return &g->api;
}
