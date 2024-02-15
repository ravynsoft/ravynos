/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/resource.h>

#include <limits.h>
#include <poll.h>
#include <stdlib.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_fatal.h>
#include <sudo_debug.h>
#include <sudo_event.h>

#if defined(OPEN_MAX) && OPEN_MAX > 256
# define SUDO_OPEN_MAX  OPEN_MAX
#else
# define SUDO_OPEN_MAX  256
#endif

int
sudo_ev_base_alloc_impl(struct sudo_event_base *base)
{
    int i;
    debug_decl(sudo_ev_base_alloc_impl, SUDO_DEBUG_EVENT);

    base->pfd_high = -1;
    base->pfd_max = 32;
    base->pfds = reallocarray(NULL, (size_t)base->pfd_max, sizeof(struct pollfd));
    if (base->pfds == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "%s: unable to allocate %d pollfds", __func__, base->pfd_max);
	base->pfd_max = 0;
	debug_return_int(-1);
    }
    for (i = 0; i < base->pfd_max; i++) {
	base->pfds[i].fd = -1;
    }

    debug_return_int(0);
}

void
sudo_ev_base_free_impl(struct sudo_event_base *base)
{
    debug_decl(sudo_ev_base_free_impl, SUDO_DEBUG_EVENT);
    free(base->pfds);
    debug_return;
}

int
sudo_ev_add_impl(struct sudo_event_base *base, struct sudo_event *ev)
{
    static int nofile_max = -1;
    struct pollfd *pfd;
    debug_decl(sudo_ev_add_impl, SUDO_DEBUG_EVENT);

    if (nofile_max == -1) {
	struct rlimit rlim;
	if (getrlimit(RLIMIT_NOFILE, &rlim) == 0) {
	    nofile_max = (int)rlim.rlim_cur;
	} else {
	    nofile_max = SUDO_OPEN_MAX;
	}
    }

    /* If out of space in pfds array, realloc. */
    if (base->pfd_free == base->pfd_max) {
	struct pollfd *pfds;
	int i, new_max;

	/* Don't allow pfd_max to go over RLIM_NOFILE */
	new_max = base->pfd_max * 2;
	if (new_max > nofile_max)
	    new_max = nofile_max;
	if (base->pfd_free == new_max) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: out of fds (max %d)", __func__, nofile_max);
	    debug_return_int(-1);
	}
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "%s: pfd_max %d -> %d", __func__, base->pfd_max, new_max);
	pfds = reallocarray(base->pfds, (size_t)new_max, sizeof(struct pollfd));
	if (pfds == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: unable to allocate %d pollfds", __func__, new_max);
	    debug_return_int(-1);
	}
	base->pfds = pfds;
	base->pfd_max = new_max;
	for (i = base->pfd_free; i < base->pfd_max; i++) {
	    base->pfds[i].fd = -1;
	}
    }

    /* Fill in pfd entry. */
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"%s: choosing free slot %d", __func__, base->pfd_free);
    ev->pfd_idx = (short)base->pfd_free;
    pfd = &base->pfds[ev->pfd_idx];
    pfd->fd = ev->fd;
    pfd->events = 0;
    if (ISSET(ev->events, SUDO_EV_READ))
	pfd->events |= POLLIN;
    if (ISSET(ev->events, SUDO_EV_WRITE))
	pfd->events |= POLLOUT;

    /* Update pfd_high and pfd_free. */
    if (ev->pfd_idx > base->pfd_high)
	base->pfd_high = ev->pfd_idx;
    for (;;) {
	if (++base->pfd_free == base->pfd_max)
	    break;
	if (base->pfds[base->pfd_free].fd == -1)
	    break;
    }

    debug_return_int(0);
}

int
sudo_ev_del_impl(struct sudo_event_base *base, struct sudo_event *ev)
{
    debug_decl(sudo_ev_del_impl, SUDO_DEBUG_EVENT);

    /* Mark pfd entry unused, add to free list and adjust high slot. */
    base->pfds[ev->pfd_idx].fd = -1;
    if (ev->pfd_idx < base->pfd_free) {
	base->pfd_free = ev->pfd_idx;
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "%s: new free slot %d", __func__, base->pfd_free);
    }
    while (base->pfd_high >= 0 && base->pfds[base->pfd_high].fd == -1)
	base->pfd_high--;

    debug_return_int(0);
}

#ifdef HAVE_PPOLL
static int
sudo_ev_poll(struct pollfd *fds, nfds_t nfds, const struct timespec *timo)
{
    return ppoll(fds, nfds, timo, NULL);
}
#else
static int
sudo_ev_poll(struct pollfd *fds, nfds_t nfds, const struct timespec *timo)
{
    const int timeout =
	timo ? (timo->tv_sec * 1000) + (timo->tv_nsec / 1000000) : -1;

    return poll(fds, nfds, timeout);
}
#endif /* HAVE_PPOLL */

int
sudo_ev_scan_impl(struct sudo_event_base *base, unsigned int flags)
{
    struct timespec now, ts, *timeout;
    struct sudo_event *ev;
    int nready;
    debug_decl(sudo_ev_scan_impl, SUDO_DEBUG_EVENT);

    if ((ev = TAILQ_FIRST(&base->timeouts)) != NULL) {
	sudo_gettime_mono(&now);
	sudo_timespecsub(&ev->timeout, &now, &ts);
	if (ts.tv_sec < 0)
	    sudo_timespecclear(&ts);
	timeout = &ts;
    } else {
	if (ISSET(flags, SUDO_EVLOOP_NONBLOCK)) {
	    sudo_timespecclear(&ts);
	    timeout = &ts;
	} else {
	    timeout = NULL;
	}
    }

    nready = sudo_ev_poll(base->pfds, (nfds_t)base->pfd_high + 1, timeout);
    switch (nready) {
    case -1:
	/* Error: EINTR (signal) or EINVAL (nfds > RLIMIT_NOFILE) */
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "sudo_ev_poll");
	break;
    case 0:
	/* Front end will activate timeout events. */
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: timeout", __func__);
	break;
    default:
	/* Activate each I/O event that fired. */
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: %d fds ready", __func__,
	    nready);
	TAILQ_FOREACH(ev, &base->events, entries) {
	    if (ev->pfd_idx != -1 && base->pfds[ev->pfd_idx].revents) {
		int what = 0;
		if (base->pfds[ev->pfd_idx].revents & (POLLIN|POLLHUP|POLLNVAL|POLLERR))
		    what |= (ev->events & SUDO_EV_READ);
		if (base->pfds[ev->pfd_idx].revents & (POLLOUT|POLLHUP|POLLNVAL|POLLERR))
		    what |= (ev->events & SUDO_EV_WRITE);
		/* Make event active. */
		sudo_debug_printf(SUDO_DEBUG_DEBUG,
		    "%s: polled fd %d, events %d, activating %p",
		    __func__, ev->fd, what, ev);
		ev->revents = (short)what;
		sudo_ev_activate(base, ev);
	    }
	}
	break;
    }
    debug_return_int(nready);
}
