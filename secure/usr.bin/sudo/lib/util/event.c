/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2018 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_debug.h>
#include <sudo_event.h>
#include <sudo_util.h>

static void sudo_ev_init(struct sudo_event *ev, int fd, int events,
    sudo_ev_callback_t callback, void *closure);

/* Default event base when none is specified. */
static struct sudo_event_base *default_base;

/* We need the event base to be available from the signal handler. */
static struct sudo_event_base *signal_base;

/*
 * Add an event to the base's active queue and mark it active.
 * This is extern so sudo_ev_scan_impl() can call it.
 */
void
sudo_ev_activate(struct sudo_event_base *base, struct sudo_event *ev)
{
    TAILQ_INSERT_TAIL(&base->active, ev, active_entries);
    SET(ev->flags, SUDO_EVQ_ACTIVE);
}

/*
 * Remove an event from the base's active queue and mark it inactive.
 */
static inline void
sudo_ev_deactivate(struct sudo_event_base *base, struct sudo_event *ev)
{
    CLR(ev->flags, SUDO_EVQ_ACTIVE);
    TAILQ_REMOVE(&base->active, ev, active_entries);
}

/*
 * Clear out the base's active queue and mark all events as inactive.
 */
static void
sudo_ev_deactivate_all(struct sudo_event_base *base)
{
    struct sudo_event *ev;
    debug_decl(sudo_ev_deactivate_all, SUDO_DEBUG_EVENT);

    while ((ev = TAILQ_FIRST(&base->active)) != NULL)
	sudo_ev_deactivate(base, ev);

    debug_return;
}

/*
 * Activate all signal events for which the corresponding signal_pending[]
 * flag is set.
 */
static void
sudo_ev_activate_sigevents(struct sudo_event_base *base)
{
    struct sudo_event *ev;
    sigset_t set, oset;
    unsigned int i;
    debug_decl(sudo_ev_activate_sigevents, SUDO_DEBUG_EVENT);

    /*
     * We treat this as a critical section since the signal handler
     * could modify the siginfo[] entry.
     */
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, &oset);
    base->signal_caught = 0;
    for (i = 0; i < NSIG; i++) {
	if (!base->signal_pending[i])
	    continue;
	base->signal_pending[i] = 0;
	TAILQ_FOREACH(ev, &base->signals[i], entries) {
	    if (ISSET(ev->events, SUDO_EV_SIGINFO)) {
		struct sudo_ev_siginfo_container *sc = ev->closure;
		if (base->siginfo[i]->si_signo == 0) {
		    /* No siginfo available. */
		    sc->siginfo = NULL;
		} else {
		    sc->siginfo = (siginfo_t *)sc->si_buf;
		    memcpy(sc->siginfo, base->siginfo[i], sizeof(siginfo_t));
		}
	    }
	    /* Make event active. */
	    ev->revents = ev->events & (SUDO_EV_SIGNAL|SUDO_EV_SIGINFO);
	    TAILQ_INSERT_TAIL(&base->active, ev, active_entries);
	    SET(ev->flags, SUDO_EVQ_ACTIVE);
	}
    }
    sigprocmask(SIG_SETMASK, &oset, NULL);

    debug_return;
}

/*
 * Internal callback for SUDO_EV_SIGNAL and SUDO_EV_SIGINFO.
 */
static void
signal_pipe_cb(int fd, int what, void *v)
{
    struct sudo_event_base *base = v;
    unsigned char ch;
    ssize_t nread;
    debug_decl(signal_pipe_cb, SUDO_DEBUG_EVENT);

    /*
     * Drain signal_pipe, the signal handler updated base->signals_pending.
     * Actual processing of signal events is done when poll/select is
     * interrupted by a signal.
     */
    while ((nread = read(fd, &ch, 1)) > 0) {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: received signal %d", __func__, (int)ch);
    }
    if (nread == -1 && errno != EAGAIN) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "%s: error reading from signal pipe fd %d", __func__, fd);
    }

    /* Activate signal events. */
    sudo_ev_activate_sigevents(base);

    debug_return;
}

static int
sudo_ev_base_init(struct sudo_event_base *base)
{
    unsigned int i;
    debug_decl(sudo_ev_base_init, SUDO_DEBUG_EVENT);

    TAILQ_INIT(&base->events);
    TAILQ_INIT(&base->timeouts);
    for (i = 0; i < NSIG; i++)
	TAILQ_INIT(&base->signals[i]);
    if (sudo_ev_base_alloc_impl(base) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR,
	    "%s: unable to allocate impl base", __func__);
	goto bad;
    }
    if (pipe2(base->signal_pipe, O_NONBLOCK|O_CLOEXEC) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR,
	    "%s: unable to create signal pipe", __func__);
	goto bad;
    }
    sudo_ev_init(&base->signal_event, base->signal_pipe[0],
	SUDO_EV_READ|SUDO_EV_PERSIST, signal_pipe_cb, base);

    debug_return_int(0);
bad:
    /* Note: signal_pipe[] not filled in. */
    sudo_ev_base_free_impl(base);
    debug_return_int(-1);
}

struct sudo_event_base *
sudo_ev_base_alloc_v1(void)
{
    struct sudo_event_base *base;
    debug_decl(sudo_ev_base_alloc, SUDO_DEBUG_EVENT);

    base = calloc(1, sizeof(*base));
    if (base == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "%s: unable to allocate base", __func__);
	debug_return_ptr(NULL);
    }
    if (sudo_ev_base_init(base) != 0) {
	free(base);
	debug_return_ptr(NULL);
    }
    debug_return_ptr(base);
}

void
sudo_ev_base_free_v1(struct sudo_event_base *base)
{
    struct sudo_event *ev, *next;
    unsigned int i;
    debug_decl(sudo_ev_base_free, SUDO_DEBUG_EVENT);

    if (base == NULL)
	debug_return;

    /* Reset the default base if necessary. */
    if (default_base == base)
	default_base = NULL;

    /* Remove any existing events before freeing the base. */
    TAILQ_FOREACH_SAFE(ev, &base->events, entries, next) {
	sudo_ev_del(base, ev);
	ev->base = NULL;
    }
    for (i = 0; i < NSIG; i++) {
	TAILQ_FOREACH_SAFE(ev, &base->signals[i], entries, next) {
	    sudo_ev_del(base, ev);
	    ev->base = NULL;
	}
	free(base->siginfo[i]);
	free(base->orig_handlers[i]);
    }
    sudo_ev_base_free_impl(base);
    close(base->signal_pipe[0]);
    close(base->signal_pipe[1]);
    free(base);

    debug_return;
}

void
sudo_ev_base_setdef_v1(struct sudo_event_base *base)
{
    debug_decl(sudo_ev_base_setdef, SUDO_DEBUG_EVENT);

    default_base = base;

    debug_return;
}

/*
 * Clear and fill in a struct sudo_event.
 */
static void
sudo_ev_init(struct sudo_event *ev, int fd, int events,
    sudo_ev_callback_t callback, void *closure)
{
    debug_decl(sudo_ev_init, SUDO_DEBUG_EVENT);

    memset(ev, 0, sizeof(*ev));
    ev->fd = fd;
    ev->events = events & SUDO_EV_MASK;
    ev->pfd_idx = -1;
    ev->callback = callback;
    ev->closure = closure;

    debug_return;
}

/*
 * Set a pre-allocated struct sudo_event.
 * Allocates space for siginfo_t for SUDO_EV_SIGINFO as needed.
 */
int
sudo_ev_set_v2(struct sudo_event *ev, int fd, int events,
    sudo_ev_callback_t callback, void *closure)
{
    debug_decl(sudo_ev_set, SUDO_DEBUG_EVENT);

    /* For SUDO_EV_SIGINFO we use a container to store closure + siginfo_t */
    if (ISSET(events, SUDO_EV_SIGINFO)) {
	struct sudo_ev_siginfo_container *container =
	    malloc(sizeof(*container) + sizeof(siginfo_t));
	if (container == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: unable to allocate siginfo container", __func__);
	    debug_return_int(-1);
	}
	container->closure = closure;
	closure = container;
    }
    sudo_ev_init(ev, fd, events, callback, closure);

    debug_return_int(0);
}

int
sudo_ev_set_v1(struct sudo_event *ev, int fd, short events,
    sudo_ev_callback_t callback, void *closure)
{
    return sudo_ev_set_v2(ev, fd, events, callback, closure);
}

struct sudo_event *
sudo_ev_alloc_v2(int fd, int events, sudo_ev_callback_t callback, void *closure)
{
    struct sudo_event *ev;
    debug_decl(sudo_ev_alloc, SUDO_DEBUG_EVENT);

    ev = malloc(sizeof(*ev));
    if (ev == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "%s: unable to allocate event", __func__);
	debug_return_ptr(NULL);
    }
    if (sudo_ev_set(ev, fd, events, callback, closure) == -1) {
	free(ev);
	debug_return_ptr(NULL);
    }
    debug_return_ptr(ev);
}

struct sudo_event *
sudo_ev_alloc_v1(int fd, short events, sudo_ev_callback_t callback, void *closure)
{
    return sudo_ev_alloc_v2(fd, events, callback, closure);
}

void
sudo_ev_free_v1(struct sudo_event *ev)
{
    debug_decl(sudo_ev_free, SUDO_DEBUG_EVENT);

    if (ev == NULL)
	debug_return;

    /* Make sure ev is not in use before freeing it. */
    if (ISSET(ev->flags, SUDO_EVQ_INSERTED))
	(void)sudo_ev_del(NULL, ev);
    if (ISSET(ev->events, SUDO_EV_SIGINFO))
	free(ev->closure);
    free(ev);

    debug_return;
}

static void
sudo_ev_handler(int signo, siginfo_t *info, void *context)
{
    unsigned char ch = (unsigned char)signo;

    if (signal_base != NULL) {
	/*
	 * Update signals_pending[] and siginfo[].
	 * All signals must be blocked any time siginfo[] is accessed.
	 * If no siginfo available, zero out the struct in base.
	 */
	if (info == NULL)
	    memset(signal_base->siginfo[signo], 0, sizeof(*info));
	else
	    memcpy(signal_base->siginfo[signo], info, sizeof(*info));
	signal_base->signal_pending[signo] = 1;
	signal_base->signal_caught = 1;

	/* Wake up the other end of the pipe. */
	ignore_result(write(signal_base->signal_pipe[1], &ch, 1));
    }
}

static int
sudo_ev_add_signal(struct sudo_event_base *base, struct sudo_event *ev,
    bool tohead)
{
    const int signo = ev->fd;
    debug_decl(sudo_ev_add_signal, SUDO_DEBUG_EVENT);

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: adding event %p to base %p, signal %d, events %d",
	__func__, ev, base, signo, ev->events);
    if (signo >= NSIG) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "%s: signo %d larger than max %d", __func__, signo, NSIG - 1);
	debug_return_int(-1);
    }
    if ((ev->events & ~(SUDO_EV_SIGNAL|SUDO_EV_SIGINFO|SUDO_EV_PERSIST)) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "%s: invalid event set 0x%x", __func__, ev->events);
	debug_return_int(-1);
    }

    /*
     * Allocate base->siginfo[signo] and base->orig_handlers[signo] as needed.
     */
    if (base->siginfo[signo] == NULL) {
	base->siginfo[signo] = malloc(sizeof(*base->siginfo[signo]));
	if (base->siginfo[signo] == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: unable to allocate siginfo for signo %d",
		__func__, signo);
	    debug_return_int(-1);
	}
    }
    if (base->orig_handlers[signo] == NULL) {
	base->orig_handlers[signo] =
	    malloc(sizeof(*base->orig_handlers[signo]));
	if (base->orig_handlers[signo] == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: unable to allocate orig_handlers for signo %d",
		__func__, signo);
	    debug_return_int(-1);
	}
    }

    /* Install signal handler as needed, saving the original value. */
    if (TAILQ_EMPTY(&base->signals[signo])) {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sigfillset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART|SA_SIGINFO;
	sa.sa_sigaction = sudo_ev_handler;
	if (sigaction(signo, &sa, base->orig_handlers[signo]) != 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: unable to install handler for signo %d", __func__, signo);
	    debug_return_int(-1);
	}
	base->num_handlers++;
    }

    /*
     * Insert signal event into the proper tail queue.
     * Signal events are always persistent.
     */
    ev->base = base;
    if (tohead) {
	TAILQ_INSERT_HEAD(&base->signals[signo], ev, entries);
    } else {
	TAILQ_INSERT_TAIL(&base->signals[signo], ev, entries);
    }
    SET(ev->events, SUDO_EV_PERSIST);
    SET(ev->flags, SUDO_EVQ_INSERTED);

    /* Add the internal signal_pipe event on demand. */
    if (!ISSET(base->signal_event.flags, SUDO_EVQ_INSERTED))
	sudo_ev_add(base, &base->signal_event, NULL, true);

    /* Update global signal base so handler to update signals_pending[] */
    signal_base = base;

    debug_return_int(0);
}

int
sudo_ev_add_v1(struct sudo_event_base *base, struct sudo_event *ev,
    const struct timeval *timo, bool tohead)
{
    struct timespec tsbuf, *ts = NULL;

    if (timo != NULL) {
	TIMEVAL_TO_TIMESPEC(timo, &tsbuf);
	ts = &tsbuf;
    }

    return sudo_ev_add_v2(base, ev, ts, tohead);
}

int
sudo_ev_add_v2(struct sudo_event_base *base, struct sudo_event *ev,
    const struct timespec *timo, bool tohead)
{
    debug_decl(sudo_ev_add, SUDO_DEBUG_EVENT);

    /* If no base specified, use existing or default base. */
    if (base == NULL) {
	if (ev->base != NULL) {
	    base = ev->base;
	} else if (default_base != NULL) {
	    base = default_base;
	} else {
	    sudo_debug_printf(SUDO_DEBUG_ERROR, "%s: no base specified",
		__func__);
	    debug_return_int(-1);
	}
    }

    /* Only add new events to the events list. */
    if (ISSET(ev->flags, SUDO_EVQ_INSERTED)) {
	/* If event no longer has a timeout, remove from timeouts queue. */
	if (timo == NULL && ISSET(ev->flags, SUDO_EVQ_TIMEOUTS)) {
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"%s: removing event %p from timeouts queue", __func__, ev);
	    CLR(ev->flags, SUDO_EVQ_TIMEOUTS);
	    TAILQ_REMOVE(&base->timeouts, ev, timeouts_entries);
	}
    } else {
	/* Special handling for signal events. */
	if (ev->events & (SUDO_EV_SIGNAL|SUDO_EV_SIGINFO))
	    debug_return_int(sudo_ev_add_signal(base, ev, tohead));

	/* Add event to the base. */
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: adding event %p to base %p, fd %d, events %d",
	    __func__, ev, base, ev->fd, ev->events);
	if (ev->events & (SUDO_EV_READ|SUDO_EV_WRITE)) {
	    if (sudo_ev_add_impl(base, ev) != 0)
		debug_return_int(-1);
	}
	ev->base = base;
	if (tohead) {
	    TAILQ_INSERT_HEAD(&base->events, ev, entries);
	} else {
	    TAILQ_INSERT_TAIL(&base->events, ev, entries);
	}
	SET(ev->flags, SUDO_EVQ_INSERTED);
    }
    /* Timeouts can be changed for existing events. */
    if (timo != NULL) {
	struct sudo_event *evtmp;
	if (ISSET(ev->flags, SUDO_EVQ_TIMEOUTS)) {
	    /* Remove from timeouts list, then add back. */
	    TAILQ_REMOVE(&base->timeouts, ev, timeouts_entries);
	}
	/* Convert to absolute time and insert in sorted order; O(n). */
	sudo_gettime_mono(&ev->timeout);
	sudo_timespecadd(&ev->timeout, timo, &ev->timeout);
	TAILQ_FOREACH(evtmp, &base->timeouts, timeouts_entries) {
	    if (sudo_timespeccmp(&ev->timeout, &evtmp->timeout, <))
		break;
	}
	if (evtmp != NULL) {
	    TAILQ_INSERT_BEFORE(evtmp, ev, timeouts_entries);
	} else {
	    TAILQ_INSERT_TAIL(&base->timeouts, ev, timeouts_entries);
	}
	SET(ev->flags, SUDO_EVQ_TIMEOUTS);
    }
    debug_return_int(0);
}

/*
 * Remove an event from the base, if specified, or the base embedded
 * in the event if not.  Note that there are multiple tail queues.
 */
int
sudo_ev_del_v1(struct sudo_event_base *base, struct sudo_event *ev)
{
    debug_decl(sudo_ev_del, SUDO_DEBUG_EVENT);

    /* Make sure event is really in the queue. */
    if (!ISSET(ev->flags, SUDO_EVQ_INSERTED)) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: event %p not in queue",
	    __func__, ev);
	debug_return_int(0);
    }

    /* Check for event base mismatch, if one is specified. */
    if (base == NULL) {
	if (ev->base == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR, "%s: no base specified",
		__func__);
	    debug_return_int(-1);
	}
	base = ev->base;
    } else if (base != ev->base) {
	sudo_debug_printf(SUDO_DEBUG_ERROR, "%s: mismatch base %p, ev->base %p",
	    __func__, base, ev->base);
	debug_return_int(-1);
    }

    if (ev->events & (SUDO_EV_SIGNAL|SUDO_EV_SIGINFO)) {
	const int signo = ev->fd;

	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: removing event %p from base %p, signo %d, events %d",
	    __func__, ev, base, signo, ev->events);

	/* Unlink from signal event list. */
	TAILQ_REMOVE(&base->signals[signo], ev, entries);
	if (TAILQ_EMPTY(&base->signals[signo])) {
	    if (sigaction(signo, base->orig_handlers[signo], NULL) != 0) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "%s: unable to restore handler for signo %d",
		    __func__, signo);
		debug_return_int(-1);
	    }
	    base->num_handlers--;
	}
	if (base->num_handlers == 0) {
	    /* No registered signal events, remove internal event. */
	    sudo_ev_del(base, &base->signal_event);
	}
    } else {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: removing event %p from base %p, fd %d, events %d",
	    __func__, ev, base, ev->fd, ev->events);

	/* Call backend. */
	if (ev->events & (SUDO_EV_READ|SUDO_EV_WRITE)) {
	    if (sudo_ev_del_impl(base, ev) != 0)
		debug_return_int(-1);
	}

	/* Unlink from event list. */
	TAILQ_REMOVE(&base->events, ev, entries);

	/* Unlink from timeouts list. */
	if (ISSET(ev->flags, SUDO_EVQ_TIMEOUTS))
	    TAILQ_REMOVE(&base->timeouts, ev, timeouts_entries);
    }

    /* Unlink from active list. */
    if (ISSET(ev->flags, SUDO_EVQ_ACTIVE))
	TAILQ_REMOVE(&base->active, ev, active_entries);

    /* Mark event unused. */
    ev->flags = 0;
    ev->pfd_idx = -1;

    debug_return_int(0);
}

int
sudo_ev_dispatch_v1(struct sudo_event_base *base)
{
    return sudo_ev_loop_v1(base, 0);
}

/*
 * Run main event loop.
 * Returns 0 on success, 1 if no events registered  and -1 on error 
 */
int
sudo_ev_loop_v1(struct sudo_event_base *base, unsigned int flags)
{
    struct timespec now;
    struct sudo_event *ev;
    int nready, rc = 0;
    debug_decl(sudo_ev_loop, SUDO_DEBUG_EVENT);

    /*
     * If sudo_ev_loopexit() was called when events were not running
     * the next invocation of sudo_ev_loop() only runs once.
     * All other base flags are ignored unless we are running events.
     * Note that SUDO_EVLOOP_ONCE and SUDO_EVBASE_LOOPONCE are equivalent.
     */
    base->flags |= (flags & SUDO_EVLOOP_ONCE);
    base->flags &= (SUDO_EVBASE_LOOPEXIT|SUDO_EVBASE_LOOPONCE);

    for (;;) {
rescan:
	/* Make sure we have some events. */
	if (TAILQ_EMPTY(&base->events)) {
	    rc = 1;
	    break;
	}

	/* Call backend to scan for I/O events. */
	TAILQ_INIT(&base->active);
	nready = sudo_ev_scan_impl(base, flags);
	switch (nready) {
	case -1:
	    if (errno == ENOMEM || errno == EAGAIN)
		continue;
	    if (errno == EINTR) {
		/* Interrupted by signal, check for sigevents. */
		if (base->signal_caught) {
		    signal_pipe_cb(base->signal_pipe[0], SUDO_EV_READ, base);
		    break;
		}
		continue;
	    }
	    rc = -1;
	    goto done;
	case 0:
	    /* Timed out, activate timeout events. */
	    sudo_gettime_mono(&now);
	    while ((ev = TAILQ_FIRST(&base->timeouts)) != NULL) {
		if (sudo_timespeccmp(&ev->timeout, &now, >))
		    break;
		/* Remove from timeouts list. */
		CLR(ev->flags, SUDO_EVQ_TIMEOUTS);
		TAILQ_REMOVE(&base->timeouts, ev, timeouts_entries);
		/* Make event active. */
		ev->revents = SUDO_EV_TIMEOUT;
		TAILQ_INSERT_TAIL(&base->active, ev, active_entries);
		SET(ev->flags, SUDO_EVQ_ACTIVE);
	    }
	    if (ISSET(flags, SUDO_EVLOOP_NONBLOCK)) {
		/* If nonblocking, return immediately if no active events. */
		if (TAILQ_EMPTY(&base->active))
		    goto done;
	    }
	    break;
	default:
	    /* I/O events active, sudo_ev_scan_impl() already added them. */
	    break;
	}

	/*
	 * Service each event in the active queue.
	 * We store the current event pointer in the base so that
	 * it can be cleared by sudo_ev_del().  This prevents a use
	 * after free if the callback frees its own event.
	 */
	while ((ev = TAILQ_FIRST(&base->active)) != NULL) {
	    /* Pop first event off the active queue. */
	    sudo_ev_deactivate(base, ev);
	    /* Remove from base unless persistent. */
	    if (!ISSET(ev->events, SUDO_EV_PERSIST))
		sudo_ev_del(base, ev);
	    ev->callback(ev->fd, ev->revents,
		ev->closure == sudo_ev_self_cbarg() ? ev : ev->closure);
	    if (ISSET(base->flags, SUDO_EVBASE_LOOPBREAK)) {
		/* Stop processing events immediately. */
		SET(base->flags, SUDO_EVBASE_GOT_BREAK);
		sudo_ev_deactivate_all(base);
		goto done;
	    }
	    if (ISSET(base->flags, SUDO_EVBASE_LOOPCONT)) {
		/* Rescan events and start polling again. */
		CLR(base->flags, SUDO_EVBASE_LOOPCONT);
		sudo_ev_deactivate_all(base);
		goto rescan;
	    }
	}
	if (ISSET(base->flags, SUDO_EVBASE_LOOPONCE)) {
	    /* SUDO_EVBASE_LOOPEXIT is always set w/ SUDO_EVBASE_LOOPONCE */
	    if (ISSET(base->flags, SUDO_EVBASE_LOOPEXIT))
		SET(base->flags, SUDO_EVBASE_GOT_EXIT);
	    sudo_ev_deactivate_all(base);
	    break;
	}
    }
done:
    base->flags &= SUDO_EVBASE_GOT_MASK;
    debug_return_int(rc);
}

void
sudo_ev_loopexit_v1(struct sudo_event_base *base)
{
    debug_decl(sudo_ev_loopexit, SUDO_DEBUG_EVENT);

    if (base == NULL) {
	if ((base = default_base) == NULL)
	    debug_return;
    }

    /* SUDO_EVBASE_LOOPBREAK trumps SUDO_EVBASE_LOOPEXIT */
    if (!ISSET(base->flags, SUDO_EVBASE_LOOPBREAK)) {
	/* SUDO_EVBASE_LOOPEXIT trumps SUDO_EVBASE_LOOPCONT */
	CLR(base->flags, SUDO_EVBASE_LOOPCONT);
	SET(base->flags, (SUDO_EVBASE_LOOPEXIT|SUDO_EVBASE_LOOPONCE));
    }
    debug_return;
}

void
sudo_ev_loopbreak_v1(struct sudo_event_base *base)
{
    debug_decl(sudo_ev_loopbreak, SUDO_DEBUG_EVENT);

    if (base == NULL) {
	if ((base = default_base) == NULL)
	    debug_return;
    }

    /* SUDO_EVBASE_LOOPBREAK trumps SUDO_EVBASE_LOOP{CONT,EXIT,ONCE}. */
    CLR(base->flags, (SUDO_EVBASE_LOOPCONT|SUDO_EVBASE_LOOPEXIT|SUDO_EVBASE_LOOPONCE));
    SET(base->flags, SUDO_EVBASE_LOOPBREAK);
    debug_return;
}

void
sudo_ev_loopcontinue_v1(struct sudo_event_base *base)
{
    debug_decl(sudo_ev_loopcontinue, SUDO_DEBUG_EVENT);

    if (base == NULL) {
	if ((base = default_base) == NULL)
	    debug_return;
    }

    /* SUDO_EVBASE_LOOP{BREAK,EXIT} trumps SUDO_EVBASE_LOOPCONT */
    if (!ISSET(base->flags, SUDO_EVBASE_LOOPONCE|SUDO_EVBASE_LOOPBREAK)) {
	SET(base->flags, SUDO_EVBASE_LOOPCONT);
    }
    debug_return;
}

bool
sudo_ev_got_exit_v1(struct sudo_event_base *base)
{
    debug_decl(sudo_ev_got_exit, SUDO_DEBUG_EVENT);

    if (base == NULL) {
	if ((base = default_base) == NULL)
	    debug_return_bool(false);
    }
    debug_return_bool(ISSET(base->flags, SUDO_EVBASE_GOT_EXIT));
}

bool
sudo_ev_got_break_v1(struct sudo_event_base *base)
{
    debug_decl(sudo_ev_got_break, SUDO_DEBUG_EVENT);

    if (base == NULL) {
	if ((base = default_base) == NULL)
	    debug_return_bool(false);
    }
    debug_return_bool(ISSET(base->flags, SUDO_EVBASE_GOT_BREAK));
}

int
sudo_ev_get_timeleft_v1(struct sudo_event *ev, struct timeval *tv)
{
    struct timespec ts;
    int ret;

    ret = sudo_ev_get_timeleft_v2(ev, &ts);
    TIMESPEC_TO_TIMEVAL(tv, &ts);

    return ret;
}

int
sudo_ev_get_timeleft_v2(struct sudo_event *ev, struct timespec *ts)
{
    debug_decl(sudo_ev_get_timeleft, SUDO_DEBUG_EVENT);

    sudo_timespecclear(ts);
    if (sudo_ev_pending_v1(ev, SUDO_EV_TIMEOUT, ts) != SUDO_EV_TIMEOUT)
	debug_return_int(-1);
    debug_return_int(0);
}

int
sudo_ev_pending_v2(struct sudo_event *ev, int events, struct timespec *ts)
{
    int ret;
    debug_decl(sudo_ev_pending, SUDO_DEBUG_EVENT);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: event %p, flags 0x%x, events 0x%x",
	__func__, ev, ev->flags, ev->events);

    if (!ISSET(ev->flags, SUDO_EVQ_INSERTED))
	debug_return_int(0);

    ret = ev->events & events;
    CLR(ret, SUDO_EV_TIMEOUT);
    if (ISSET(ev->flags, SUDO_EVQ_TIMEOUTS) && ISSET(events, SUDO_EV_TIMEOUT)) {
	ret |= SUDO_EV_TIMEOUT;
	if (ts != NULL) {
	    struct timespec now;

	    sudo_gettime_mono(&now);
	    sudo_timespecsub(&ev->timeout, &now, ts);
	    if (ts->tv_sec < 0)
		sudo_timespecclear(ts);
	}
    }

    debug_return_int(ret);
}

int
sudo_ev_pending_v1(struct sudo_event *ev, short events, struct timespec *ts)
{
    return sudo_ev_pending_v2(ev, events, ts);
}
