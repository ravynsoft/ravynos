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

#include <sys/param.h>		/* for howmany() on Linux */
#include <sys/time.h>
#ifdef HAVE_SYS_SYSMACROS_H
# include <sys/sysmacros.h>	/* for howmany() on Solaris */
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_fatal.h>
#include <sudo_debug.h>
#include <sudo_event.h>

int
sudo_ev_base_alloc_impl(struct sudo_event_base *base)
{
    debug_decl(sudo_ev_base_alloc_impl, SUDO_DEBUG_EVENT);

    base->maxfd = NFDBITS - 1;
    base->readfds_in = calloc(1, sizeof(fd_mask));
    base->writefds_in = calloc(1, sizeof(fd_mask));
    base->readfds_out = calloc(1, sizeof(fd_mask));
    base->writefds_out = calloc(1, sizeof(fd_mask));

    if (base->readfds_in == NULL || base->writefds_in == NULL ||
	base->readfds_out == NULL || base->writefds_out == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "%s: unable to calloc(1, %zu)", __func__, sizeof(fd_mask));
	sudo_ev_base_free_impl(base);
	debug_return_int(-1);
    }
    debug_return_int(0);
}

void
sudo_ev_base_free_impl(struct sudo_event_base *base)
{
    debug_decl(sudo_ev_base_free_impl, SUDO_DEBUG_EVENT);
    free(base->readfds_in);
    free(base->writefds_in);
    free(base->readfds_out);
    free(base->writefds_out);
    debug_return;
}

int
sudo_ev_add_impl(struct sudo_event_base *base, struct sudo_event *ev)
{
    debug_decl(sudo_ev_add_impl, SUDO_DEBUG_EVENT);

    /* If out of space in fd sets, realloc. */
    if (ev->fd > base->maxfd) {
	const int o = (base->maxfd + 1) / NFDBITS;
	const int n = howmany(ev->fd + 1, NFDBITS);
	const size_t used_bytes = (size_t)o * sizeof(fd_mask);
	const size_t new_bytes = (size_t)(n - o) * sizeof(fd_mask);
	fd_set *rfds_in, *wfds_in, *rfds_out, *wfds_out;

	rfds_in = reallocarray(base->readfds_in, (size_t)n, sizeof(fd_mask));
	wfds_in = reallocarray(base->writefds_in, (size_t)n, sizeof(fd_mask));
	rfds_out = reallocarray(base->readfds_out, (size_t)n, sizeof(fd_mask));
	wfds_out = reallocarray(base->writefds_out, (size_t)n, sizeof(fd_mask));
	if (rfds_in == NULL || wfds_in == NULL ||
	    rfds_out == NULL || wfds_out == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: unable to reallocarray(%d, %zu)",
		__func__, n, sizeof(fd_mask));
	    free(rfds_in);
	    free(wfds_in);
	    free(rfds_out);
	    free(wfds_out);
	    debug_return_int(-1);
	}

	/* Clear newly allocated space. */
	memset((char *)rfds_in + used_bytes, 0, new_bytes);
	memset((char *)wfds_in + used_bytes, 0, new_bytes);
	memset((char *)rfds_out + used_bytes, 0, new_bytes);
	memset((char *)wfds_out + used_bytes, 0, new_bytes);

	/* Update base. */
	base->readfds_in = rfds_in;
	base->writefds_in = wfds_in;
	base->readfds_out = rfds_out;
	base->writefds_out = wfds_out;
	base->maxfd = (n * NFDBITS) - 1;
    }

    /* Set events and adjust high fd as needed. */
    if (ISSET(ev->events, SUDO_EV_READ)) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: added fd %d to readfs",
	    __func__, ev->fd);
	FD_SET(ev->fd, (fd_set *)base->readfds_in);
    }
    if (ISSET(ev->events, SUDO_EV_WRITE)) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: added fd %d to writefds",
	    __func__, ev->fd);
	FD_SET(ev->fd, (fd_set *)base->writefds_in);
    }
    if (ev->fd > base->highfd)
	base->highfd = ev->fd;

    debug_return_int(0);
}

int
sudo_ev_del_impl(struct sudo_event_base *base, struct sudo_event *ev)
{
    debug_decl(sudo_ev_del_impl, SUDO_DEBUG_EVENT);

    /* Remove from readfds and writefds and adjust high fd. */
    if (ISSET(ev->events, SUDO_EV_READ)) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: removed fd %d from readfds",
	    __func__, ev->fd);
	FD_CLR(ev->fd, (fd_set *)base->readfds_in);
    }
    if (ISSET(ev->events, SUDO_EV_WRITE)) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: removed fd %d from writefds",
	    __func__, ev->fd);
	FD_CLR(ev->fd, (fd_set *)base->writefds_in);
    }
    if (base->highfd == ev->fd) {
	for (;;) {
	    if (FD_ISSET(base->highfd, (fd_set *)base->readfds_in) ||
		FD_ISSET(base->highfd, (fd_set *)base->writefds_in))
		break;
	    if (--base->highfd < 0)
		break;
	}
    }

    debug_return_int(0);
}

#ifdef HAVE_PSELECT
static int
sudo_ev_select(int nfds, fd_set *readfds, fd_set *writefds,
    fd_set *exceptfds, const struct timespec *timeout)
{
    return pselect(nfds, readfds, writefds, exceptfds, timeout, NULL);
}
#else
static int
sudo_ev_select(int nfds, fd_set *readfds, fd_set *writefds,
    fd_set *exceptfds, const struct timespec *timeout)
{
    struct timeval tvbuf, *tv = NULL;

    if (timeout != NULL) {
	TIMESPEC_TO_TIMEVAL(&tvbuf, timeout);
	tv = &tvbuf;
    }
    return select(nfds, readfds, writefds, exceptfds, tv);
}
#endif /* HAVE_PSELECT */

int
sudo_ev_scan_impl(struct sudo_event_base *base, unsigned int flags)
{
    struct timespec now, ts, *timeout;
    struct sudo_event *ev;
    size_t setsize;
    int nready;
    debug_decl(sudo_ev_loop, SUDO_DEBUG_EVENT);

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

    /* select() overwrites readfds/writefds so make a copy. */
    setsize = (size_t)howmany(base->highfd + 1, NFDBITS) * sizeof(fd_mask);
    memcpy(base->readfds_out, base->readfds_in, setsize);
    memcpy(base->writefds_out, base->writefds_in, setsize);

    sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: select high fd %d",
	__func__, base->highfd);
    nready = sudo_ev_select(base->highfd + 1, base->readfds_out,
	base->writefds_out, NULL, timeout);
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: %d fds ready", __func__, nready);
    switch (nready) {
    case -1:
	/* Error or interrupted by signal. */
	break;
    case 0:
	/* Front end will activate timeout events. */
	break;
    default:
	/* Activate each I/O event that fired. */
	TAILQ_FOREACH(ev, &base->events, entries) {
	    if (ev->fd >= 0) {
		short what = 0;
		if (FD_ISSET(ev->fd, (fd_set *)base->readfds_out))
		    what |= (ev->events & SUDO_EV_READ);
		if (FD_ISSET(ev->fd, (fd_set *)base->writefds_out))
		    what |= (ev->events & SUDO_EV_WRITE);
		if (what != 0) {
		    /* Make event active. */
		    sudo_debug_printf(SUDO_DEBUG_DEBUG,
			"%s: selected fd %d, events %hd, activating %p",
			__func__, ev->fd, what, ev);
		    ev->revents = what;
		    sudo_ev_activate(base, ev);
		}
	    }
	}
	break;
    }
    debug_return_int(nready);
}
