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

#include <poll.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "llist.h"
#include "malloc.h"
#include "timeval.h"
#include "simple-watch.h"

struct AvahiWatch {
    AvahiSimplePoll *simple_poll;
    int dead;

    int idx;
    struct pollfd pollfd;

    AvahiWatchCallback callback;
    void *userdata;

    AVAHI_LLIST_FIELDS(AvahiWatch, watches);
};

struct AvahiTimeout {
    AvahiSimplePoll *simple_poll;
    int dead;

    int enabled;
    struct timeval expiry;

    AvahiTimeoutCallback callback;
    void  *userdata;

    AVAHI_LLIST_FIELDS(AvahiTimeout, timeouts);
};

struct AvahiSimplePoll {
    AvahiPoll api;
    AvahiPollFunc poll_func;
    void *poll_func_userdata;

    struct pollfd* pollfds;
    int n_pollfds, max_pollfds, rebuild_pollfds;

    int watch_req_cleanup, timeout_req_cleanup;
    int quit;
    int events_valid;

    int n_watches;
    AVAHI_LLIST_HEAD(AvahiWatch, watches);
    AVAHI_LLIST_HEAD(AvahiTimeout, timeouts);

    int wakeup_pipe[2];
    int wakeup_issued;

    int prepared_timeout;

    enum {
        STATE_INIT,
        STATE_PREPARING,
        STATE_PREPARED,
        STATE_RUNNING,
        STATE_RAN,
        STATE_DISPATCHING,
        STATE_DISPATCHED,
        STATE_QUIT,
        STATE_FAILURE
    } state;
};

void avahi_simple_poll_wakeup(AvahiSimplePoll *s) {
    char c = 'W';
    assert(s);

    write(s->wakeup_pipe[1], &c, sizeof(c));
    s->wakeup_issued = 1;
}

static void clear_wakeup(AvahiSimplePoll *s) {
    char c[10]; /* Read ten at a time */

    if (!s->wakeup_issued)
        return;

    s->wakeup_issued = 0;

    for(;;)
        if (read(s->wakeup_pipe[0], &c, sizeof(c)) != sizeof(c))
            break;
}

static int set_nonblock(int fd) {
    int n;

    assert(fd >= 0);

    if ((n = fcntl(fd, F_GETFL)) < 0)
        return -1;

    if (n & O_NONBLOCK)
        return 0;

    return fcntl(fd, F_SETFL, n|O_NONBLOCK);
}

static AvahiWatch* watch_new(const AvahiPoll *api, int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void *userdata) {
    AvahiWatch *w;
    AvahiSimplePoll *s;

    assert(api);
    assert(fd >= 0);
    assert(callback);

    s = api->userdata;
    assert(s);

    if (!(w = avahi_new(AvahiWatch, 1)))
        return NULL;

    /* If there is a background thread running the poll() for us, tell it to exit the poll() */
    avahi_simple_poll_wakeup(s);

    w->simple_poll = s;
    w->dead = 0;

    w->pollfd.fd = fd;
    w->pollfd.events = event;
    w->pollfd.revents = 0;

    w->callback = callback;
    w->userdata = userdata;

    w->idx = -1;
    s->rebuild_pollfds = 1;

    AVAHI_LLIST_PREPEND(AvahiWatch, watches, s->watches, w);
    s->n_watches++;

    return w;
}

static void watch_update(AvahiWatch *w, AvahiWatchEvent events) {
    assert(w);
    assert(!w->dead);

    /* If there is a background thread running the poll() for us, tell it to exit the poll() */
    avahi_simple_poll_wakeup(w->simple_poll);

    w->pollfd.events = events;

    if (w->idx != -1) {
        assert(w->simple_poll);
        w->simple_poll->pollfds[w->idx] = w->pollfd;
    } else
        w->simple_poll->rebuild_pollfds = 1;
}

static AvahiWatchEvent watch_get_events(AvahiWatch *w) {
    assert(w);
    assert(!w->dead);

    if (w->idx != -1 && w->simple_poll->events_valid)
        return w->simple_poll->pollfds[w->idx].revents;

    return 0;
}

static void remove_pollfd(AvahiWatch *w) {
    assert(w);

    if (w->idx == -1)
        return;

    w->simple_poll->rebuild_pollfds = 1;
}

static void watch_free(AvahiWatch *w) {
    assert(w);

    assert(!w->dead);

    /* If there is a background thread running the poll() for us, tell it to exit the poll() */
    avahi_simple_poll_wakeup(w->simple_poll);

    remove_pollfd(w);

    w->dead = 1;
    w->simple_poll->n_watches --;
    w->simple_poll->watch_req_cleanup = 1;
}

static void destroy_watch(AvahiWatch *w) {
    assert(w);

    remove_pollfd(w);
    AVAHI_LLIST_REMOVE(AvahiWatch, watches, w->simple_poll->watches, w);

    if (!w->dead)
        w->simple_poll->n_watches --;

    avahi_free(w);
}

static void cleanup_watches(AvahiSimplePoll *s, int all) {
    AvahiWatch *w, *next;
    assert(s);

    for (w = s->watches; w; w = next) {
        next = w->watches_next;

        if (all || w->dead)
            destroy_watch(w);
    }

    s->timeout_req_cleanup = 0;
}

static AvahiTimeout* timeout_new(const AvahiPoll *api, const struct timeval *tv, AvahiTimeoutCallback callback, void *userdata) {
    AvahiTimeout *t;
    AvahiSimplePoll *s;

    assert(api);
    assert(callback);

    s = api->userdata;
    assert(s);

    if (!(t = avahi_new(AvahiTimeout, 1)))
        return NULL;

    /* If there is a background thread running the poll() for us, tell it to exit the poll() */
    avahi_simple_poll_wakeup(s);

    t->simple_poll = s;
    t->dead = 0;

    if ((t->enabled = !!tv))
        t->expiry = *tv;

    t->callback = callback;
    t->userdata = userdata;

    AVAHI_LLIST_PREPEND(AvahiTimeout, timeouts, s->timeouts, t);
    return t;
}

static void timeout_update(AvahiTimeout *t, const struct timeval *tv) {
    assert(t);
    assert(!t->dead);

    /* If there is a background thread running the poll() for us, tell it to exit the poll() */
    avahi_simple_poll_wakeup(t->simple_poll);

    if ((t->enabled = !!tv))
        t->expiry = *tv;
}

static void timeout_free(AvahiTimeout *t) {
    assert(t);
    assert(!t->dead);

    /* If there is a background thread running the poll() for us, tell it to exit the poll() */
    avahi_simple_poll_wakeup(t->simple_poll);

    t->dead = 1;
    t->simple_poll->timeout_req_cleanup = 1;
}


static void destroy_timeout(AvahiTimeout *t) {
    assert(t);

    AVAHI_LLIST_REMOVE(AvahiTimeout, timeouts, t->simple_poll->timeouts, t);

    avahi_free(t);
}

static void cleanup_timeouts(AvahiSimplePoll *s, int all) {
    AvahiTimeout *t, *next;
    assert(s);

    for (t = s->timeouts; t; t = next) {
        next = t->timeouts_next;

        if (all || t->dead)
            destroy_timeout(t);
    }

    s->timeout_req_cleanup = 0;
}

AvahiSimplePoll *avahi_simple_poll_new(void) {
    AvahiSimplePoll *s;

    if (!(s = avahi_new(AvahiSimplePoll, 1)))
        return NULL;

    if (pipe(s->wakeup_pipe) < 0) {
        avahi_free(s);
        return NULL;
    }

    set_nonblock(s->wakeup_pipe[0]);
    set_nonblock(s->wakeup_pipe[1]);

    s->api.userdata = s;

    s->api.watch_new = watch_new;
    s->api.watch_free = watch_free;
    s->api.watch_update = watch_update;
    s->api.watch_get_events = watch_get_events;

    s->api.timeout_new = timeout_new;
    s->api.timeout_free = timeout_free;
    s->api.timeout_update = timeout_update;

    s->pollfds = NULL;
    s->max_pollfds = s->n_pollfds = 0;
    s->rebuild_pollfds = 1;
    s->quit = 0;
    s->n_watches = 0;
    s->events_valid = 0;

    s->watch_req_cleanup = 0;
    s->timeout_req_cleanup = 0;

    s->prepared_timeout = 0;

    s->state = STATE_INIT;

    s->wakeup_issued = 0;

    avahi_simple_poll_set_func(s, NULL, NULL);

    AVAHI_LLIST_HEAD_INIT(AvahiWatch, s->watches);
    AVAHI_LLIST_HEAD_INIT(AvahiTimeout, s->timeouts);

    return s;
}

void avahi_simple_poll_free(AvahiSimplePoll *s) {
    assert(s);

    cleanup_timeouts(s, 1);
    cleanup_watches(s, 1);
    assert(s->n_watches == 0);

    avahi_free(s->pollfds);

    if (s->wakeup_pipe[0] >= 0)
        close(s->wakeup_pipe[0]);

    if (s->wakeup_pipe[1] >= 0)
        close(s->wakeup_pipe[1]);

    avahi_free(s);
}

static int rebuild(AvahiSimplePoll *s) {
    AvahiWatch *w;
    int idx;

    assert(s);

    if (s->n_watches+1 > s->max_pollfds) {
        struct pollfd *n;

        s->max_pollfds = s->n_watches + 10;

        if (!(n = avahi_realloc(s->pollfds, sizeof(struct pollfd) * s->max_pollfds)))
            return -1;

        s->pollfds = n;
    }


    s->pollfds[0].fd = s->wakeup_pipe[0];
    s->pollfds[0].events = POLLIN;
    s->pollfds[0].revents = 0;

    idx = 1;

    for (w = s->watches; w; w = w->watches_next) {

        if(w->dead)
            continue;

        assert(w->idx < s->max_pollfds);
        s->pollfds[w->idx = idx++] = w->pollfd;
    }

    s->n_pollfds = idx;
    s->events_valid = 0;
    s->rebuild_pollfds = 0;

    return 0;
}

static AvahiTimeout* find_next_timeout(AvahiSimplePoll *s) {
    AvahiTimeout *t, *n = NULL;
    assert(s);

    for (t = s->timeouts; t; t = t->timeouts_next) {

        if (t->dead || !t->enabled)
            continue;

        if (!n || avahi_timeval_compare(&t->expiry, &n->expiry) < 0)
            n = t;
    }

    return n;
}

static void timeout_callback(AvahiTimeout *t) {
    assert(t);
    assert(!t->dead);
    assert(t->enabled);

    t->enabled = 0;
    t->callback(t, t->userdata);
}

int avahi_simple_poll_prepare(AvahiSimplePoll *s, int timeout) {
    AvahiTimeout *next_timeout;

    assert(s);
    assert(s->state == STATE_INIT || s->state == STATE_DISPATCHED || s->state == STATE_FAILURE);
    s->state = STATE_PREPARING;

    /* Clear pending wakeup requests */
    clear_wakeup(s);

    /* Cleanup things first */
    if (s->watch_req_cleanup)
        cleanup_watches(s, 0);

    if (s->timeout_req_cleanup)
        cleanup_timeouts(s, 0);

    /* Check whether a quit was requested */
    if (s->quit) {
        s->state = STATE_QUIT;
        return 1;
    }

    /* Do we need to rebuild our array of pollfds? */
    if (s->rebuild_pollfds)
        if (rebuild(s) < 0) {
            s->state = STATE_FAILURE;
            return -1;
        }

    /* Calculate the wakeup time */
    if ((next_timeout = find_next_timeout(s))) {
        struct timeval now;
        int t;
        AvahiUsec usec;

        if (next_timeout->expiry.tv_sec == 0 &&
            next_timeout->expiry.tv_usec == 0) {

            /* Just a shortcut so that we don't need to call gettimeofday() */
            timeout = 0;
            goto finish;
        }

        gettimeofday(&now, NULL);
        usec = avahi_timeval_diff(&next_timeout->expiry, &now);

        if (usec <= 0) {
            /* Timeout elapsed */

            timeout = 0;
            goto finish;
        }

        /* Calculate sleep time. We add 1ms because otherwise we'd
         * wake up too early most of the time */
        t = (int) (usec / 1000) + 1;

        if (timeout < 0 || timeout > t)
            timeout = t;
    }

finish:
    s->prepared_timeout = timeout;
    s->state = STATE_PREPARED;
    return 0;
}

int avahi_simple_poll_run(AvahiSimplePoll *s) {
    assert(s);
    assert(s->state == STATE_PREPARED || s->state == STATE_FAILURE);

    s->state = STATE_RUNNING;

    for (;;) {
        errno = 0;

        if (s->poll_func(s->pollfds, s->n_pollfds, s->prepared_timeout, s->poll_func_userdata) < 0) {

            if (errno == EINTR)
                continue;

            s->state = STATE_FAILURE;
            return -1;
        }

        break;
    }

    /* The poll events are now valid again */
    s->events_valid = 1;

    /* Update state */
    s->state = STATE_RAN;
    return 0;
}

int avahi_simple_poll_dispatch(AvahiSimplePoll *s) {
    AvahiTimeout *next_timeout;
    AvahiWatch *w;

    assert(s);
    assert(s->state == STATE_RAN);
    s->state = STATE_DISPATCHING;

    /* We execute only on callback in every iteration */

    /* Check whether the wakeup time has been reached now */
    if ((next_timeout = find_next_timeout(s))) {

        if (next_timeout->expiry.tv_sec == 0 && next_timeout->expiry.tv_usec == 0) {

            /* Just a shortcut so that we don't need to call gettimeofday() */
            timeout_callback(next_timeout);
            goto finish;
        }

        if (avahi_age(&next_timeout->expiry) >= 0) {

            /* Timeout elapsed */
            timeout_callback(next_timeout);
            goto finish;
        }
    }

    /* Look for some kind of I/O event */
    for (w = s->watches; w; w = w->watches_next) {

        if (w->dead)
            continue;

        assert(w->idx >= 0);
        assert(w->idx < s->n_pollfds);

        if (s->pollfds[w->idx].revents != 0) {
            w->callback(w, w->pollfd.fd, s->pollfds[w->idx].revents, w->userdata);
            goto finish;
        }
    }

finish:

    s->state = STATE_DISPATCHED;
    return 0;
}

int avahi_simple_poll_iterate(AvahiSimplePoll *s, int timeout) {
    int r;

    if ((r = avahi_simple_poll_prepare(s, timeout)) != 0)
        return r;

    if ((r = avahi_simple_poll_run(s)) != 0)
        return r;

    if ((r = avahi_simple_poll_dispatch(s)) != 0)
        return r;

    return 0;
}

void avahi_simple_poll_quit(AvahiSimplePoll *s) {
    assert(s);

    s->quit = 1;

    /* If there is a background thread running the poll() for us, tell it to exit the poll() */
    avahi_simple_poll_wakeup(s);
}

const AvahiPoll* avahi_simple_poll_get(AvahiSimplePoll *s) {
    assert(s);

    return &s->api;
}

static int system_poll(struct pollfd *ufds, unsigned int nfds, int timeout, AVAHI_GCC_UNUSED void *userdata) {
    return poll(ufds, nfds, timeout);
}

void avahi_simple_poll_set_func(AvahiSimplePoll *s, AvahiPollFunc func, void *userdata) {
    assert(s);

    s->poll_func = func ? func : system_poll;
    s->poll_func_userdata = func ? userdata : NULL;

    /* If there is a background thread running the poll() for us, tell it to exit the poll() */
    avahi_simple_poll_wakeup(s);
}

int avahi_simple_poll_loop(AvahiSimplePoll *s) {
    int r;

    assert(s);

    for (;;)
        if ((r = avahi_simple_poll_iterate(s, -1)) != 0)
            if (r >= 0 || errno != EINTR)
                return r;
}
