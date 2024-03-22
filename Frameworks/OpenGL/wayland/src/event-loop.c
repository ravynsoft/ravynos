/*
 * Copyright © 2008 Kristian Høgsberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include "wayland-util.h"
#include "wayland-private.h"
#include "wayland-server-core.h"
#include "wayland-os.h"

/** \cond INTERNAL */

#define TIMER_REMOVED -2

struct wl_event_loop;
struct wl_event_source_interface;
struct wl_event_source_timer;

struct wl_event_source {
	struct wl_event_source_interface *interface;
	struct wl_event_loop *loop;
	struct wl_list link;
	void *data;
	int fd;
};

struct wl_timer_heap {
	struct wl_event_source base;
	/* pointers to the user-visible event sources */
	struct wl_event_source_timer **data;
	int space, active, count;
};

struct wl_event_loop {
	int epoll_fd;
	struct wl_list check_list;
	struct wl_list idle_list;
	struct wl_list destroy_list;

	struct wl_signal destroy_signal;

	struct wl_timer_heap timers;
};

struct wl_event_source_interface {
	int (*dispatch)(struct wl_event_source *source,
			struct epoll_event *ep);
};


struct wl_event_source_fd {
	struct wl_event_source base;
	wl_event_loop_fd_func_t func;
	int fd;
};

/** \endcond */

static int
wl_event_source_fd_dispatch(struct wl_event_source *source,
			    struct epoll_event *ep)
{
	struct wl_event_source_fd *fd_source = (struct wl_event_source_fd *) source;
	uint32_t mask;

	mask = 0;
	if (ep->events & EPOLLIN)
		mask |= WL_EVENT_READABLE;
	if (ep->events & EPOLLOUT)
		mask |= WL_EVENT_WRITABLE;
	if (ep->events & EPOLLHUP)
		mask |= WL_EVENT_HANGUP;
	if (ep->events & EPOLLERR)
		mask |= WL_EVENT_ERROR;

	return fd_source->func(fd_source->fd, mask, source->data);
}

struct wl_event_source_interface fd_source_interface = {
	wl_event_source_fd_dispatch,
};

static struct wl_event_source *
add_source(struct wl_event_loop *loop,
	   struct wl_event_source *source, uint32_t mask, void *data)
{
	struct epoll_event ep;

	if (source->fd < 0) {
		free(source);
		return NULL;
	}

	source->loop = loop;
	source->data = data;
	wl_list_init(&source->link);

	memset(&ep, 0, sizeof ep);
	if (mask & WL_EVENT_READABLE)
		ep.events |= EPOLLIN;
	if (mask & WL_EVENT_WRITABLE)
		ep.events |= EPOLLOUT;
	ep.data.ptr = source;

	if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, source->fd, &ep) < 0) {
		close(source->fd);
		free(source);
		return NULL;
	}

	return source;
}

/** Create a file descriptor event source
 *
 * \param loop The event loop that will process the new source.
 * \param fd The file descriptor to watch.
 * \param mask A bitwise-or of which events to watch for: \c WL_EVENT_READABLE,
 * \c WL_EVENT_WRITABLE.
 * \param func The file descriptor dispatch function.
 * \param data User data.
 * \return A new file descriptor event source.
 *
 * The given file descriptor is initially watched for the events given in
 * \c mask. This can be changed as needed with wl_event_source_fd_update().
 *
 * If it is possible that program execution causes the file descriptor to be
 * read while leaving the data in a buffer without actually processing it,
 * it may be necessary to register the file descriptor source to be re-checked,
 * see wl_event_source_check(). This will ensure that the dispatch function
 * gets called even if the file descriptor is not readable or writable
 * anymore. This is especially useful with IPC libraries that automatically
 * buffer incoming data, possibly as a side-effect of other operations.
 *
 * \sa wl_event_loop_fd_func_t
 * \memberof wl_event_source
 */
WL_EXPORT struct wl_event_source *
wl_event_loop_add_fd(struct wl_event_loop *loop,
		     int fd, uint32_t mask,
		     wl_event_loop_fd_func_t func,
		     void *data)
{
	struct wl_event_source_fd *source;

	source = zalloc(sizeof *source);
	if (source == NULL)
		return NULL;

	source->base.interface = &fd_source_interface;
	source->base.fd = wl_os_dupfd_cloexec(fd, 0);
	source->func = func;
	source->fd = fd;

	return add_source(loop, &source->base, mask, data);
}

/** Update a file descriptor source's event mask
 *
 * \param source The file descriptor event source to update.
 * \param mask The new mask, a bitwise-or of: \c WL_EVENT_READABLE,
 * \c WL_EVENT_WRITABLE.
 * \return 0 on success, -1 on failure.
 *
 * This changes which events, readable and/or writable, cause the dispatch
 * callback to be called on.
 *
 * File descriptors are usually writable to begin with, so they do not need to
 * be polled for writable until a write actually fails. When a write fails,
 * the event mask can be changed to poll for readable and writable, delivering
 * a dispatch callback when it is possible to write more. Once all data has
 * been written, the mask can be changed to poll only for readable to avoid
 * busy-looping on dispatch.
 *
 * \sa wl_event_loop_add_fd()
 * \memberof wl_event_source
 */
WL_EXPORT int
wl_event_source_fd_update(struct wl_event_source *source, uint32_t mask)
{
	struct wl_event_loop *loop = source->loop;
	struct epoll_event ep;

	memset(&ep, 0, sizeof ep);
	if (mask & WL_EVENT_READABLE)
		ep.events |= EPOLLIN;
	if (mask & WL_EVENT_WRITABLE)
		ep.events |= EPOLLOUT;
	ep.data.ptr = source;

	return epoll_ctl(loop->epoll_fd, EPOLL_CTL_MOD, source->fd, &ep);
}

/** \cond INTERNAL */

struct wl_event_source_timer {
	struct wl_event_source base;
	wl_event_loop_timer_func_t func;
	struct wl_event_source_timer *next_due;
	struct timespec deadline;
	int heap_idx;
};

static int
noop_dispatch(struct wl_event_source *source,
	      struct epoll_event *ep) {
	return 0;
}

struct wl_event_source_interface timer_heap_source_interface = {
	noop_dispatch,
};

static bool
time_lt(struct timespec ta, struct timespec tb)
{
	if (ta.tv_sec != tb.tv_sec) {
		return ta.tv_sec < tb.tv_sec;
	}
	return ta.tv_nsec < tb.tv_nsec;
}

static int
set_timer(int timerfd, struct timespec deadline) {
	struct itimerspec its;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value = deadline;
	return timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &its, NULL);
}

static int
clear_timer(int timerfd)
{
	struct itimerspec its;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	return timerfd_settime(timerfd, 0, &its, NULL);
}

static void
wl_timer_heap_init(struct wl_timer_heap *timers, struct wl_event_loop *loop)
{
	timers->base.fd = -1;
	timers->base.data = NULL;
	wl_list_init(&timers->base.link);
	timers->base.interface = &timer_heap_source_interface;
	timers->base.loop = loop;

	loop->timers.data = NULL;
	loop->timers.active = 0;
	loop->timers.space = 0;
	loop->timers.count = 0;
}

static void
wl_timer_heap_release(struct wl_timer_heap *timers)
{
	if (timers->base.fd != -1) {
		close(timers->base.fd);
	}
	free(timers->data);
}

static int
wl_timer_heap_ensure_timerfd(struct wl_timer_heap *timers)
{
	struct epoll_event ep;
	int timer_fd;

	if (timers->base.fd != -1)
		return 0;

	memset(&ep, 0, sizeof ep);
	ep.events = EPOLLIN;
	ep.data.ptr = timers;

	timer_fd = timerfd_create(CLOCK_MONOTONIC,
				  TFD_CLOEXEC | TFD_NONBLOCK);
	if (timer_fd < 0)
		return -1;

	if (epoll_ctl(timers->base.loop->epoll_fd,
		      EPOLL_CTL_ADD, timer_fd, &ep) < 0) {
		close(timer_fd);
		return -1;
	}

	timers->base.fd = timer_fd;
	return 0;
}

static int
wl_timer_heap_reserve(struct wl_timer_heap *timers)
{
	struct wl_event_source_timer **n;
	int new_space;

	if (timers->count + 1 > timers->space) {
		new_space = timers->space >= 8 ? timers->space * 2 : 8;
		n = realloc(timers->data, (size_t)new_space * sizeof(*n));
		if (!n) {
			wl_log("Allocation failure when expanding timer list\n");
			return -1;
		}
		timers->data = n;
		timers->space = new_space;
	}

	timers->count++;
	return 0;
}

static void
wl_timer_heap_unreserve(struct wl_timer_heap *timers)
{
	struct wl_event_source_timer **n;

	timers->count--;

	if (timers->space >= 16 && timers->space >= 4 * timers->count) {
		n = realloc(timers->data, (size_t)timers->space / 2 * sizeof(*n));
		if (!n) {
			wl_log("Reallocation failure when shrinking timer list\n");
			return;
		}
		timers->data = n;
		timers->space = timers->space / 2;
	}
}

static int
heap_set(struct wl_event_source_timer **data,
	 struct wl_event_source_timer *a,
	 int idx)
{
	int tmp;

	tmp = a->heap_idx;
	a->heap_idx = idx;
	data[a->heap_idx] = a;

	return tmp;
}

static void
heap_sift_down(struct wl_event_source_timer **data,
	       int num_active,
	       struct wl_event_source_timer *source)
{
	struct wl_event_source_timer *child, *other_child;
	int cursor_idx;
	struct timespec key;

	cursor_idx = source->heap_idx;
	key = source->deadline;
	while (1) {
		int lchild_idx = cursor_idx * 2 + 1;

		if (lchild_idx >= num_active) {
			break;
		}

		child = data[lchild_idx];
		if (lchild_idx + 1 < num_active) {
			other_child = data[lchild_idx + 1];
			if (time_lt(other_child->deadline, child->deadline))
				child = other_child;
		}

		if (time_lt(child->deadline, key))
			cursor_idx = heap_set(data, child, cursor_idx);
		else
			break;
	}

	heap_set(data, source, cursor_idx);
}

static void
heap_sift_up(struct wl_event_source_timer **data,
	     struct wl_event_source_timer *source)
{
	int cursor_idx;
	struct timespec key;

	cursor_idx = source->heap_idx;
	key = source->deadline;
	while (cursor_idx > 0) {
		struct wl_event_source_timer *parent =
			data[(cursor_idx - 1) / 2];

		if (time_lt(key, parent->deadline))
			cursor_idx = heap_set(data, parent, cursor_idx);
		else
			break;
	}
	heap_set(data, source, cursor_idx);
}

/* requires timer be armed */
static void
wl_timer_heap_disarm(struct wl_timer_heap *timers,
		     struct wl_event_source_timer *source)
{
	struct wl_event_source_timer *last_end_evt;
	int old_source_idx;

	assert(source->heap_idx >= 0);

	old_source_idx = source->heap_idx;
	source->heap_idx = -1;
	source->deadline.tv_sec = 0;
	source->deadline.tv_nsec = 0;

	last_end_evt = timers->data[timers->active - 1];
	timers->data[timers->active - 1] = NULL;
	timers->active--;

	if (old_source_idx == timers->active)
		return;

	timers->data[old_source_idx] = last_end_evt;
	last_end_evt->heap_idx = old_source_idx;

	/* Move the displaced (active) element to its proper place.
	 * Only one of sift-down and sift-up will have any effect */
	heap_sift_down(timers->data, timers->active, last_end_evt);
	heap_sift_up(timers->data, last_end_evt);
}

/* requires timer be disarmed */
static void
wl_timer_heap_arm(struct wl_timer_heap *timers,
		  struct wl_event_source_timer *source,
		  struct timespec deadline)
{
	assert(source->heap_idx == -1);

	source->deadline = deadline;
	timers->data[timers->active] = source;
	source->heap_idx = timers->active;
	timers->active++;
	heap_sift_up(timers->data, source);
}


static int
wl_timer_heap_dispatch(struct wl_timer_heap *timers)
{
	struct timespec now;
	struct wl_event_source_timer *root;
	struct wl_event_source_timer *list_cursor = NULL, *list_tail = NULL;

	clock_gettime(CLOCK_MONOTONIC, &now);

	while (timers->active > 0) {
		root = timers->data[0];
		if (time_lt(now, root->deadline))
			break;

		wl_timer_heap_disarm(timers, root);

		if (list_cursor == NULL)
			list_cursor = root;
		else
			list_tail->next_due = root;
		list_tail = root;
	}
	if (list_tail)
		list_tail->next_due = NULL;

	if (timers->active > 0) {
		if (set_timer(timers->base.fd, timers->data[0]->deadline) < 0)
			return -1;
	} else {
		if (clear_timer(timers->base.fd) < 0)
			return -1;
	}

	/* Execute precisely the functions for events before `now`, in order.
	 * Because wl_event_loop_dispatch ignores return codes, do the same
	 * here as well */
	for (; list_cursor; list_cursor = list_cursor->next_due) {
		if (list_cursor->base.fd != TIMER_REMOVED)
			list_cursor->func(list_cursor->base.data);
	}

	return 0;
}

static int
wl_event_source_timer_dispatch(struct wl_event_source *source,
			       struct epoll_event *ep)
{
	struct wl_event_source_timer *timer;

	timer = wl_container_of(source, timer, base);
	return timer->func(timer->base.data);
}

struct wl_event_source_interface timer_source_interface = {
	wl_event_source_timer_dispatch,
};

/** \endcond */

/** Create a timer event source
 *
 * \param loop The event loop that will process the new source.
 * \param func The timer dispatch function.
 * \param data User data.
 * \return A new timer event source.
 *
 * The timer is initially disarmed. It needs to be armed with a call to
 * wl_event_source_timer_update() before it can trigger a dispatch call.
 *
 * \sa wl_event_loop_timer_func_t
 * \memberof wl_event_source
 */
WL_EXPORT struct wl_event_source *
wl_event_loop_add_timer(struct wl_event_loop *loop,
			wl_event_loop_timer_func_t func,
			void *data)
{
	struct wl_event_source_timer *source;

	if (wl_timer_heap_ensure_timerfd(&loop->timers) < 0)
		return NULL;

	source = zalloc(sizeof *source);
	if (source == NULL)
		return NULL;

	source->base.interface = &timer_source_interface;
	source->base.fd = -1;
	source->func = func;
	source->base.loop = loop;
	source->base.data = data;
	wl_list_init(&source->base.link);
	source->next_due = NULL;
	source->deadline.tv_sec = 0;
	source->deadline.tv_nsec = 0;
	source->heap_idx = -1;

	if (wl_timer_heap_reserve(&loop->timers) < 0) {
		free(source);
		return NULL;
	}

	return &source->base;
}

/** Arm or disarm a timer
 *
 * \param source The timer event source to modify.
 * \param ms_delay The timeout in milliseconds.
 * \return 0 on success, -1 on failure.
 *
 * If the timeout is zero, the timer is disarmed.
 *
 * If the timeout is non-zero, the timer is set to expire after the given
 * timeout in milliseconds. When the timer expires, the dispatch function
 * set with wl_event_loop_add_timer() is called once from
 * wl_event_loop_dispatch(). If another dispatch is desired after another
 * expiry, wl_event_source_timer_update() needs to be called again.
 *
 * \memberof wl_event_source
 */
WL_EXPORT int
wl_event_source_timer_update(struct wl_event_source *source, int ms_delay)
{
	struct wl_event_source_timer *tsource =
		wl_container_of(source, tsource, base);
	struct wl_timer_heap *timers = &tsource->base.loop->timers;

	if (ms_delay > 0) {
		struct timespec deadline;

		clock_gettime(CLOCK_MONOTONIC, &deadline);

		deadline.tv_nsec += (ms_delay % 1000) * 1000000L;
		deadline.tv_sec += ms_delay / 1000;
		if (deadline.tv_nsec >= 1000000000L) {
			deadline.tv_nsec -= 1000000000L;
			deadline.tv_sec += 1;
		}

		if (tsource->heap_idx == -1) {
			wl_timer_heap_arm(timers, tsource, deadline);
		} else if (time_lt(deadline, tsource->deadline)) {
			tsource->deadline = deadline;
			heap_sift_up(timers->data, tsource);
		} else {
			tsource->deadline = deadline;
			heap_sift_down(timers->data, timers->active, tsource);
		}

		if (tsource->heap_idx == 0) {
			/* Only update the timerfd if the new deadline is
			 * the earliest */
			if (set_timer(timers->base.fd, deadline) < 0)
				return -1;
		}
	} else {
		if (tsource->heap_idx == -1)
			return 0;
		wl_timer_heap_disarm(timers, tsource);

		if (timers->active == 0) {
			/* Only update the timerfd if this was the last
			 * active timer */
			if (clear_timer(timers->base.fd) < 0)
				return -1;
		}
	}

	return 0;
}

/** \cond INTERNAL */

struct wl_event_source_signal {
	struct wl_event_source base;
	int signal_number;
	wl_event_loop_signal_func_t func;
};

/** \endcond */

static int
wl_event_source_signal_dispatch(struct wl_event_source *source,
				struct epoll_event *ep)
{
	struct wl_event_source_signal *signal_source =
		(struct wl_event_source_signal *) source;
	struct signalfd_siginfo signal_info;
	int len;

	len = read(source->fd, &signal_info, sizeof signal_info);
	if (!(len == -1 && errno == EAGAIN) && len != sizeof signal_info)
		/* Is there anything we can do here?  Will this ever happen? */
		wl_log("signalfd read error: %s\n", strerror(errno));

	return signal_source->func(signal_source->signal_number,
				   signal_source->base.data);
}

struct wl_event_source_interface signal_source_interface = {
	wl_event_source_signal_dispatch,
};

/** Create a POSIX signal event source
 *
 * \param loop The event loop that will process the new source.
 * \param signal_number Number of the signal to watch for.
 * \param func The signal dispatch function.
 * \param data User data.
 * \return A new signal event source.
 *
 * This function blocks the normal delivery of the given signal in the calling
 * thread, and creates a "watch" for it. Signal delivery no longer happens
 * asynchronously, but by wl_event_loop_dispatch() calling the dispatch
 * callback function \c func.
 *
 * It is the caller's responsibility to ensure that all other threads have
 * also blocked the signal.
 *
 * \sa wl_event_loop_signal_func_t
 * \memberof wl_event_source
 */
WL_EXPORT struct wl_event_source *
wl_event_loop_add_signal(struct wl_event_loop *loop,
			 int signal_number,
			 wl_event_loop_signal_func_t func,
			 void *data)
{
	struct wl_event_source_signal *source;
	sigset_t mask;

	source = zalloc(sizeof *source);
	if (source == NULL)
		return NULL;

	source->base.interface = &signal_source_interface;
	source->signal_number = signal_number;

	sigemptyset(&mask);
	sigaddset(&mask, signal_number);
	source->base.fd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	source->func = func;

	return add_source(loop, &source->base, WL_EVENT_READABLE, data);
}

/** \cond INTERNAL */

struct wl_event_source_idle {
	struct wl_event_source base;
	wl_event_loop_idle_func_t func;
};

/** \endcond */

struct wl_event_source_interface idle_source_interface = {
	NULL,
};

/** Create an idle task
 *
 * \param loop The event loop that will process the new task.
 * \param func The idle task dispatch function.
 * \param data User data.
 * \return A new idle task (an event source).
 *
 * Idle tasks are dispatched before wl_event_loop_dispatch() goes to sleep.
 * See wl_event_loop_dispatch() for more details.
 *
 * Idle tasks fire once, and are automatically destroyed right after the
 * callback function has been called.
 *
 * An idle task can be cancelled before the callback has been called by
 * wl_event_source_remove(). Calling wl_event_source_remove() after or from
 * within the callback results in undefined behaviour.
 *
 * \sa wl_event_loop_idle_func_t
 * \memberof wl_event_source
 */
WL_EXPORT struct wl_event_source *
wl_event_loop_add_idle(struct wl_event_loop *loop,
		       wl_event_loop_idle_func_t func,
		       void *data)
{
	struct wl_event_source_idle *source;

	source = zalloc(sizeof *source);
	if (source == NULL)
		return NULL;

	source->base.interface = &idle_source_interface;
	source->base.loop = loop;
	source->base.fd = -1;

	source->func = func;
	source->base.data = data;

	wl_list_insert(loop->idle_list.prev, &source->base.link);

	return &source->base;
}

/** Mark event source to be re-checked
 *
 * \param source The event source to be re-checked.
 *
 * This function permanently marks the event source to be re-checked after
 * the normal dispatch of sources in wl_event_loop_dispatch(). Re-checking
 * will keep iterating over all such event sources until the dispatch
 * function for them all returns zero.
 *
 * Re-checking is used on sources that may become ready to dispatch as a
 * side-effect of dispatching themselves or other event sources, including idle
 * sources. Re-checking ensures all the incoming events have been fully drained
 * before wl_event_loop_dispatch() returns.
 *
 * \memberof wl_event_source
 */
WL_EXPORT void
wl_event_source_check(struct wl_event_source *source)
{
	wl_list_insert(source->loop->check_list.prev, &source->link);
}

/** Remove an event source from its event loop
 *
 * \param source The event source to be removed.
 * \return Zero.
 *
 * The event source is removed from the event loop it was created for,
 * and is effectively destroyed. This invalidates \c source .
 * The dispatch function of the source will no longer be called through this
 * source.
 *
 * \memberof wl_event_source
 */
WL_EXPORT int
wl_event_source_remove(struct wl_event_source *source)
{
	struct wl_event_loop *loop = source->loop;

	/* We need to explicitly remove the fd, since closing the fd
	 * isn't enough in case we've dup'ed the fd. */
	if (source->fd >= 0) {
		epoll_ctl(loop->epoll_fd, EPOLL_CTL_DEL, source->fd, NULL);
		close(source->fd);
		source->fd = -1;
	}

	if (source->interface == &timer_source_interface &&
	    source->fd != TIMER_REMOVED) {
		/* Disarm the timer (and the loop's timerfd, if necessary),
		 * before removing its space in the loop timer heap */
		wl_event_source_timer_update(source, 0);
		wl_timer_heap_unreserve(&loop->timers);
		/* Set the fd field to to indicate that the timer should NOT
		 * be dispatched in `wl_event_loop_dispatch` */
		source->fd = TIMER_REMOVED;
	}

	wl_list_remove(&source->link);
	wl_list_insert(&loop->destroy_list, &source->link);

	return 0;
}

static void
wl_event_loop_process_destroy_list(struct wl_event_loop *loop)
{
	struct wl_event_source *source, *next;

	wl_list_for_each_safe(source, next, &loop->destroy_list, link)
		free(source);

	wl_list_init(&loop->destroy_list);
}

/** Create a new event loop context
 *
 * \return A new event loop context object.
 *
 * This creates a new event loop context. Initially this context is empty.
 * Event sources need to be explicitly added to it.
 *
 * Normally the event loop is run by calling wl_event_loop_dispatch() in
 * a loop until the program terminates. Alternatively, an event loop can be
 * embedded in another event loop by its file descriptor, see
 * wl_event_loop_get_fd().
 *
 * \memberof wl_event_loop
 */
WL_EXPORT struct wl_event_loop *
wl_event_loop_create(void)
{
	struct wl_event_loop *loop;

	loop = zalloc(sizeof *loop);
	if (loop == NULL)
		return NULL;

	loop->epoll_fd = wl_os_epoll_create_cloexec();
	if (loop->epoll_fd < 0) {
		free(loop);
		return NULL;
	}
	wl_list_init(&loop->check_list);
	wl_list_init(&loop->idle_list);
	wl_list_init(&loop->destroy_list);

	wl_signal_init(&loop->destroy_signal);

	wl_timer_heap_init(&loop->timers, loop);

	return loop;
}

/** Destroy an event loop context
 *
 * \param loop The event loop to be destroyed.
 *
 * This emits the event loop destroy signal, closes the event loop file
 * descriptor, and frees \c loop.
 *
 * If the event loop has existing sources, those cannot be safely removed
 * afterwards. Therefore one must call wl_event_source_remove() on all
 * event sources before destroying the event loop context.
 *
 * \memberof wl_event_loop
 */
WL_EXPORT void
wl_event_loop_destroy(struct wl_event_loop *loop)
{
	wl_signal_emit(&loop->destroy_signal, loop);

	wl_event_loop_process_destroy_list(loop);
	wl_timer_heap_release(&loop->timers);
	close(loop->epoll_fd);
	free(loop);
}

static bool
post_dispatch_check(struct wl_event_loop *loop)
{
	struct epoll_event ep;
	struct wl_event_source *source, *next;
	bool needs_recheck = false;

	ep.events = 0;
	wl_list_for_each_safe(source, next, &loop->check_list, link) {
		int dispatch_result;

		dispatch_result = source->interface->dispatch(source, &ep);
		if (dispatch_result < 0) {
			wl_log("Source dispatch function returned negative value!\n");
			wl_log("This would previously accidentally suppress a follow-up dispatch\n");
		}
		needs_recheck |= dispatch_result != 0;
	}

	return needs_recheck;
}

/** Dispatch the idle sources
 *
 * \param loop The event loop whose idle sources are dispatched.
 *
 * \sa wl_event_loop_add_idle()
 * \memberof wl_event_loop
 */
WL_EXPORT void
wl_event_loop_dispatch_idle(struct wl_event_loop *loop)
{
	struct wl_event_source_idle *source;

	while (!wl_list_empty(&loop->idle_list)) {
		source = wl_container_of(loop->idle_list.next,
					 source, base.link);
		source->func(source->base.data);
		wl_event_source_remove(&source->base);
	}
}

/** Wait for events and dispatch them
 *
 * \param loop The event loop whose sources to wait for.
 * \param timeout The polling timeout in milliseconds.
 * \return 0 for success, -1 for polling (or timer update) error.
 *
 * All the associated event sources are polled. This function blocks until
 * any event source delivers an event (idle sources excluded), or the timeout
 * expires. A timeout of -1 disables the timeout, causing the function to block
 * indefinitely. A timeout of zero causes the poll to always return immediately.
 *
 * All idle sources are dispatched before blocking. An idle source is destroyed
 * when it is dispatched. After blocking, all other ready sources are
 * dispatched. Then, idle sources are dispatched again, in case the dispatched
 * events created idle sources. Finally, all sources marked with
 * wl_event_source_check() are dispatched in a loop until their dispatch
 * functions all return zero.
 *
 * \memberof wl_event_loop
 */
WL_EXPORT int
wl_event_loop_dispatch(struct wl_event_loop *loop, int timeout)
{
	struct epoll_event ep[32];
	struct wl_event_source *source;
	int i, count;
	bool has_timers = false;

	wl_event_loop_dispatch_idle(loop);

	count = epoll_wait(loop->epoll_fd, ep, ARRAY_LENGTH(ep), timeout);
	if (count < 0)
		return -1;

	for (i = 0; i < count; i++) {
		source = ep[i].data.ptr;
		if (source == &loop->timers.base)
			has_timers = true;
	}

	if (has_timers) {
		/* Dispatch timer sources before non-timer sources, so that
		 * the non-timer sources can not cancel (by calling
		 * `wl_event_source_timer_update`) the dispatching of the timers
		 * (Note that timer sources also can't cancel pending non-timer
		 * sources, since epoll_wait has already been called) */
		if (wl_timer_heap_dispatch(&loop->timers) < 0)
			return -1;
	}

	for (i = 0; i < count; i++) {
		source = ep[i].data.ptr;
		if (source->fd != -1)
			source->interface->dispatch(source, &ep[i]);
	}

	wl_event_loop_process_destroy_list(loop);

	wl_event_loop_dispatch_idle(loop);

	while (post_dispatch_check(loop));

	return 0;
}

/** Get the event loop file descriptor
 *
 * \param loop The event loop context.
 * \return The aggregate file descriptor.
 *
 * This function returns the aggregate file descriptor, that represents all
 * the event sources (idle sources excluded) associated with the given event
 * loop context. When any event source makes an event available, it will be
 * reflected in the aggregate file descriptor.
 *
 * When the aggregate file descriptor delivers an event, one can call
 * wl_event_loop_dispatch() on the event loop context to dispatch all the
 * available events.
 *
 * \memberof wl_event_loop
 */
WL_EXPORT int
wl_event_loop_get_fd(struct wl_event_loop *loop)
{
	return loop->epoll_fd;
}

/** Register a destroy listener for an event loop context
 *
 * \param loop The event loop context whose destruction to listen for.
 * \param listener The listener with the callback to be called.
 *
 * \sa wl_listener
 * \memberof wl_event_loop
 */
WL_EXPORT void
wl_event_loop_add_destroy_listener(struct wl_event_loop *loop,
				   struct wl_listener *listener)
{
	wl_signal_add(&loop->destroy_signal, listener);
}

/** Get the listener struct for the specified callback
 *
 * \param loop The event loop context to inspect.
 * \param notify The destroy callback to find.
 * \return The wl_listener registered to the event loop context with
 * the given callback pointer.
 *
 * \memberof wl_event_loop
 */
WL_EXPORT struct wl_listener *
wl_event_loop_get_destroy_listener(struct wl_event_loop *loop,
				   wl_notify_func_t notify)
{
	return wl_signal_get(&loop->destroy_signal, notify);
}
