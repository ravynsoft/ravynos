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
# include <config.h>
#endif

#include <stdlib.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>
#include <avahi-common/timeval.h>
#include <avahi-common/watch.h>

#include "libevent-watch.h"

#ifndef HOST_NAME_MAX
# include <limits.h>
# define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#endif

struct AvahiWatch {
	AvahiLibeventPoll *eventpoll;

	struct event ev;

	AvahiWatchCallback cb;
	void *userdata;

	AVAHI_LLIST_FIELDS(AvahiWatch, watches);
};

struct AvahiTimeout {
	AvahiLibeventPoll *eventpoll;

	struct event ev;

	AvahiTimeoutCallback cb;
	void *userdata;

	AVAHI_LLIST_FIELDS(AvahiTimeout, timeouts);
};

struct AvahiLibeventPoll {
	AvahiPoll api;

	struct event_base *base;

	AVAHI_LLIST_HEAD(AvahiWatch, watches);
	AVAHI_LLIST_HEAD(AvahiTimeout, timeouts);
};

/* AvahiPoll implementation for libevent */

static void
watch_cb(evutil_socket_t fd, short what, void *arg)
{
	AvahiWatch *w = arg;
	AvahiWatchEvent events = 0;

	if (what & EV_READ)
		events |= AVAHI_WATCH_IN;
	if (what & EV_WRITE)
		events |= AVAHI_WATCH_OUT;

	w->cb(w, fd, events, w->userdata);
}

static int
watch_add(AvahiWatch *w, int fd, AvahiWatchEvent events)
{
	AvahiLibeventPoll *ep = w->eventpoll;
	short ev_events = 0;

	if (events & AVAHI_WATCH_IN)
		ev_events |= EV_READ;
	if (events & AVAHI_WATCH_OUT)
		ev_events |= EV_WRITE;

	event_assign(&w->ev, ep->base, fd, ev_events | EV_PERSIST, watch_cb, w);

	return event_add(&w->ev, NULL);
}

static AvahiWatch *
watch_new(const AvahiPoll *api, int fd, AvahiWatchEvent events, AvahiWatchCallback cb, void *userdata)
{
	AvahiLibeventPoll *ep;
	AvahiWatch *w;
	int ret;

	assert(api);
	assert(fd >= 0);
	assert(cb);

	ep = api->userdata;
	assert(ep);

	w = avahi_new(AvahiWatch, 1);
	if (!w)
		return NULL;

	w->eventpoll = ep;
	w->cb = cb;
	w->userdata = userdata;

	ret = watch_add(w, fd, events);
	if (ret != 0) {
		free(w);
		return NULL;
	}

	AVAHI_LLIST_PREPEND(AvahiWatch, watches, ep->watches, w);

	return w;
}

static void
watch_update(AvahiWatch *w, AvahiWatchEvent events)
{
	event_del(&w->ev);

	watch_add(w, (int)event_get_fd(&w->ev), events);
}

static AvahiWatchEvent
watch_get_events(AvahiWatch *w)
{
	AvahiWatchEvent events = 0;

	if (event_pending(&w->ev, EV_READ, NULL))
		events |= AVAHI_WATCH_IN;
	if (event_pending(&w->ev, EV_WRITE, NULL))
		events |= AVAHI_WATCH_OUT;

	return events;
}

static void
watch_free(AvahiWatch *w)
{
	AvahiLibeventPoll *ep = w->eventpoll;

	event_del(&w->ev);

	AVAHI_LLIST_REMOVE(AvahiWatch, watches, ep->watches, w);

	free(w);
}

static void
timeout_cb(AVAHI_GCC_UNUSED evutil_socket_t fd, AVAHI_GCC_UNUSED short events, void *arg)
{
	AvahiTimeout *t = arg;

	t->cb(t, t->userdata);
}

static int
timeout_add(AvahiTimeout *t, const struct timeval *tv)
{
	AvahiLibeventPoll *ep = t->eventpoll;
	struct timeval now, e_tv;

	event_assign(&t->ev, ep->base, -1, EV_TIMEOUT, timeout_cb, t);

	if (!tv || ((tv->tv_sec == 0) && (tv->tv_usec == 0)))
		evutil_timerclear(&e_tv);
	else {
		(void)gettimeofday(&now, NULL);
		evutil_timersub(tv, &now, &e_tv);
	}

	return evtimer_add(&t->ev, &e_tv);
}

static AvahiTimeout *
timeout_new(const AvahiPoll *api, const struct timeval *tv, AvahiTimeoutCallback cb, void *userdata)
{
	AvahiLibeventPoll *ep;
	AvahiTimeout *t;
	int ret;

	assert(api);
	assert(cb);

	ep = api->userdata;

	assert(ep);

	t = avahi_new(AvahiTimeout, 1);
	if (!t)
		return NULL;

	t->eventpoll = ep;
	t->cb = cb;
	t->userdata = userdata;

	ret = timeout_add(t, tv);
	if (ret != 0) {
		free(t);
		return NULL;
	}

	AVAHI_LLIST_PREPEND(AvahiTimeout, timeouts, ep->timeouts, t);

	return t;
}

static void
timeout_update(AvahiTimeout *t, const struct timeval *tv)
{
	struct timeval now, e_tv;

	event_del(&t->ev);

	if (!tv)
		return;

	(void)gettimeofday(&now, NULL);
	evutil_timersub(tv, &now, &e_tv);

	event_add(&t->ev, &e_tv);
}

static void
timeout_free(AvahiTimeout *t)
{
	AvahiLibeventPoll *ep = t->eventpoll;

	event_del(&t->ev);

	AVAHI_LLIST_REMOVE(AvahiTimeout, timeouts, ep->timeouts, t);

	free(t);
}

AvahiLibeventPoll *
avahi_libevent_poll_new(struct event_base *base)
{
	AvahiLibeventPoll *ep = avahi_new(AvahiLibeventPoll, 1);

	ep->base = base;

	ep->api.userdata = ep;

	ep->api.watch_new = watch_new;
	ep->api.watch_free = watch_free;
	ep->api.watch_update = watch_update;
	ep->api.watch_get_events = watch_get_events;

	ep->api.timeout_new = timeout_new;
	ep->api.timeout_free = timeout_free;
	ep->api.timeout_update = timeout_update;

	AVAHI_LLIST_HEAD_INIT(AvahiWatch, ep->watches);
	AVAHI_LLIST_HEAD_INIT(AvahiTimeout, ep->timeouts);

	return ep;
}

void
avahi_libevent_poll_free(AvahiLibeventPoll *ep)
{
	assert(ep);

	for (AvahiWatch *w_next, *w = ep->watches; w; w = w_next) {
		w_next = w->watches_next;

		watch_free(w);
	}

	for (AvahiTimeout *t_next, *t = ep->timeouts; t; t = t_next) {
		t_next = t->timeouts_next;

		timeout_free(t);
	}

	free(ep);
}

void
avahi_libevent_poll_quit(AvahiLibeventPoll *ep)
{
	assert(ep);

	/* we don't actually have anything to do, since events are
	 * associated with watches and timeouts, not with this
	 * polling object itself.
	 */
}

const AvahiPoll *
avahi_libevent_poll_get(AvahiLibeventPoll *ep)
{
	assert(ep);

	return &ep->api;
}

