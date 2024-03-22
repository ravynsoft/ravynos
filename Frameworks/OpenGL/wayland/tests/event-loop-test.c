/*
 * Copyright © 2012 Intel Corporation
 * Copyright © 2012 Jason Ekstrand
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

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

#include "wayland-private.h"
#include "wayland-server.h"
#include "test-runner.h"

static int
fd_dispatch(int fd, uint32_t mask, void *data)
{
	int *p = data;

	assert(mask == 0);
	++(*p);

	return 0;
}

TEST(event_loop_post_dispatch_check)
{
	struct wl_event_loop *loop = wl_event_loop_create();
	struct wl_event_source *source;
	int dispatch_ran = 0;
	int p[2];

	assert(loop);
	assert(pipe(p) == 0);

	source = wl_event_loop_add_fd(loop, p[0], WL_EVENT_READABLE,
				      fd_dispatch, &dispatch_ran);
	assert(source);
	wl_event_source_check(source);

	wl_event_loop_dispatch(loop, 0);
	assert(dispatch_ran == 1);

	assert(close(p[0]) == 0);
	assert(close(p[1]) == 0);
	wl_event_source_remove(source);
	wl_event_loop_destroy(loop);
}

struct free_source_context {
	struct wl_event_source *source1, *source2;
	int p1[2], p2[2];
	int count;
};

static int
free_source_callback(int fd, uint32_t mask, void *data)
{
	struct free_source_context *context = data;

	context->count++;

	/* Remove other source */
	if (fd == context->p1[0]) {
		wl_event_source_remove(context->source2);
		context->source2 = NULL;
	} else if (fd == context->p2[0]) {
		wl_event_source_remove(context->source1);
		context->source1 = NULL;
	} else {
		assert(0);
	}

	return 1;
}

TEST(event_loop_free_source_with_data)
{
	struct wl_event_loop *loop = wl_event_loop_create();
	struct free_source_context context;
	int data;

	/* This test is a little tricky to get right, since we don't
	 * have any guarantee from the event loop (ie epoll) on the
	 * order of which it reports events.  We want to have one
	 * source free the other, but we don't know which one is going
	 * to run first.  So we add two fd sources with a callback
	 * that frees the other source and check that only one of them
	 * run (and that we don't crash, of course).
	 */

	assert(loop);

	context.count = 0;
	assert(pipe(context.p1) == 0);
	assert(pipe(context.p2) == 0);
	context.source1 =
		wl_event_loop_add_fd(loop, context.p1[0], WL_EVENT_READABLE,
				     free_source_callback, &context);
	assert(context.source1);
	context.source2 =
		wl_event_loop_add_fd(loop, context.p2[0], WL_EVENT_READABLE,
				     free_source_callback, &context);
	assert(context.source2);

	data = 5;
	assert(write(context.p1[1], &data, sizeof data) == sizeof data);
	assert(write(context.p2[1], &data, sizeof data) == sizeof data);

	wl_event_loop_dispatch(loop, 0);

	assert(context.count == 1);

	if (context.source1)
		wl_event_source_remove(context.source1);
	if (context.source2)
		wl_event_source_remove(context.source2);
	wl_event_loop_destroy(loop);

	assert(close(context.p1[0]) == 0);
	assert(close(context.p1[1]) == 0);
	assert(close(context.p2[0]) == 0);
	assert(close(context.p2[1]) == 0);
}

static int
signal_callback(int signal_number, void *data)
{
	int *got_it = data;

	assert(signal_number == SIGUSR1);
	++(*got_it);

	return 1;
}

TEST(event_loop_signal)
{
	struct wl_event_loop *loop = wl_event_loop_create();
	struct wl_event_source *source;
	int got_it = 0;

	source = wl_event_loop_add_signal(loop, SIGUSR1,
					  signal_callback, &got_it);
	assert(source);

	assert(wl_event_loop_dispatch(loop, 0) == 0);
	assert(!got_it);
	assert(kill(getpid(), SIGUSR1) == 0);
	/*
	 * On Linux the signal will be immediately visible in the epoll_wait()
	 * call. However, on FreeBSD we may need a small delay between kill()
	 * call and the signal being visible to the kevent() call. This
	 * sometimes happens when the signal processing and kevent processing
	 * runs on different CPUs, so becomes more likely when the system is
	 * under load (e.g. running all tests in parallel).
	 * See https://github.com/jiixyj/epoll-shim/pull/32
	 * Passing 1ms as the timeout appears to avoid this race condition in
	 * all cases tested so far, but to be safe we use 1000ms which should
	 * be enough time even on a really slow (or emulated) system.
	 */
	assert(wl_event_loop_dispatch(loop, 1000) == 0);
	assert(got_it == 1);

	wl_event_source_remove(source);
	wl_event_loop_destroy(loop);
}

TEST(event_loop_multiple_same_signals)
{
	struct wl_event_loop *loop = wl_event_loop_create();
	struct wl_event_source *s1, *s2;
	int calls_no = 0;
	int i;

	s1 = wl_event_loop_add_signal(loop, SIGUSR1,
				      signal_callback, &calls_no);
	assert(s1);

	s2 = wl_event_loop_add_signal(loop, SIGUSR1,
				      signal_callback, &calls_no);
	assert(s2);

	assert(wl_event_loop_dispatch(loop, 0) == 0);
	assert(!calls_no);

	/* Try it more times */
	for (i = 0; i < 5; ++i) {
		calls_no = 0;
		assert(kill(getpid(), SIGUSR1) == 0);
		/*
		 * We need a non-zero timeout here to allow the test to pass
		 * on non-Linux systems (see comment in event_loop_signal).
		 */
		assert(wl_event_loop_dispatch(loop, 1000) == 0);
		assert(calls_no == 2);
	}

	wl_event_source_remove(s1);

	/* Try it again  with one source */
	calls_no = 0;
	assert(kill(getpid(), SIGUSR1) == 0);
	/*
	 * We need a non-zero timeout here to allow the test to pass
	 * on non-Linux systems (see comment in event_loop_signal).
	 */
	assert(wl_event_loop_dispatch(loop, 1000) == 0);
	assert(calls_no == 1);

	wl_event_source_remove(s2);

	wl_event_loop_destroy(loop);
}

static int
timer_callback(void *data)
{
	int *got_it = data;

	++(*got_it);

	return 1;
}

TEST(event_loop_timer)
{
	struct wl_event_loop *loop = wl_event_loop_create();
	struct wl_event_source *source1, *source2;
	int got_it = 0;

	source1 = wl_event_loop_add_timer(loop, timer_callback, &got_it);
	assert(source1);
	wl_event_source_timer_update(source1, 20);

	source2 = wl_event_loop_add_timer(loop, timer_callback, &got_it);
	assert(source2);
	wl_event_source_timer_update(source2, 100);

	/* Check that the timer marked for 20 msec from now fires within 30
	 * msec, and that the timer marked for 100 msec is expected to fire
	 * within an additional 90 msec. (Some extra wait time is provided to
	 * account for reasonable code execution / thread preemption delays.) */

	wl_event_loop_dispatch(loop, 0);
	assert(got_it == 0);
	wl_event_loop_dispatch(loop, 30);
	assert(got_it == 1);
	wl_event_loop_dispatch(loop, 0);
	assert(got_it == 1);
	wl_event_loop_dispatch(loop, 90);
	assert(got_it == 2);

	wl_event_source_remove(source1);
	wl_event_source_remove(source2);
	wl_event_loop_destroy(loop);
}

#define MSEC_TO_USEC(msec) ((msec) * 1000)

struct timer_update_context {
	struct wl_event_source *source1, *source2;
	int count;
};

static int
timer_update_callback_1(void *data)
{
	struct timer_update_context *context = data;

	context->count++;
	wl_event_source_timer_update(context->source2, 1000);
	return 1;
}

static int
timer_update_callback_2(void *data)
{
	struct timer_update_context *context = data;

	context->count++;
	wl_event_source_timer_update(context->source1, 1000);
	return 1;
}

TEST(event_loop_timer_updates)
{
	struct wl_event_loop *loop = wl_event_loop_create();
	struct timer_update_context context;
	struct timeval start_time, end_time, interval;

	/* Create two timers that should expire at the same time (after 10ms).
	 * The first timer to receive its expiry callback updates the other timer
	 * with a much larger timeout (1s). This highlights a bug where
	 * wl_event_source_timer_dispatch would block for this larger timeout
	 * when reading from the timer fd, before calling the second timer's
	 * callback.
	 */

	context.source1 = wl_event_loop_add_timer(loop, timer_update_callback_1,
						  &context);
	assert(context.source1);
	assert(wl_event_source_timer_update(context.source1, 10) == 0);

	context.source2 = wl_event_loop_add_timer(loop, timer_update_callback_2,
						  &context);
	assert(context.source2);
	assert(wl_event_source_timer_update(context.source2, 10) == 0);

	context.count = 0;

	/* Since calling the functions between source2's update and
	 * wl_event_loop_dispatch() takes some time, it may happen
	 * that only one timer expires until we call epoll_wait.
	 * This naturally means that only one source is dispatched
	 * and the test fails. To fix that, sleep 15 ms before
	 * calling wl_event_loop_dispatch(). That should be enough
	 * for the second timer to expire.
	 *
	 * https://bugs.freedesktop.org/show_bug.cgi?id=80594
	 */
	usleep(MSEC_TO_USEC(15));

	gettimeofday(&start_time, NULL);
	wl_event_loop_dispatch(loop, 20);
	gettimeofday(&end_time, NULL);

	assert(context.count == 2);

	/* Dispatching the events should not have taken much more than 20ms,
	 * since this is the timeout passed to wl_event_loop_dispatch. If it
	 * blocked, then it will have taken over 1s.
	 * Of course, it could take over 1s anyway on a very slow or heavily
	 * loaded system, so this test isn't 100% perfect.
	 */

	timersub(&end_time, &start_time, &interval);
	assert(interval.tv_sec < 1);

	wl_event_source_remove(context.source1);
	wl_event_source_remove(context.source2);
	wl_event_loop_destroy(loop);
}

struct timer_order_data {
	struct wl_event_source *source;
	int *last_number;
	int number;
};

static int
timer_order_callback(void *data)
{
	struct timer_order_data *tod = data;

	/* Check that the timers have the correct sequence */
	assert(tod->number == *tod->last_number + 2);
	*tod->last_number = tod->number;
	return 0;
}

TEST(event_loop_timer_order)
{
	struct wl_event_loop *loop = wl_event_loop_create();
	struct timer_order_data order[20];
	int i, j;
	int last = -1;

	/* Configure a set of timers so that only timers 1, 3, 5, ..., 19
	 * (in that order) will be dispatched when the event loop is run */

	for (i = 0; i < 20; i++) {
		order[i].number = i;
		order[i].last_number = &last;
		order[i].source =
			wl_event_loop_add_timer(loop, timer_order_callback,
						&order[i]);
		assert(order[i].source);
		assert(wl_event_source_timer_update(order[i].source, 10) == 0);
	}

	for (i = 0; i < 20; i++) {
		/* Permute the order in which timers are updated, so as to
		 * more exhaustively test the underlying priority queue code */
		j = ((i + 3) * 17) % 20;
		assert(wl_event_source_timer_update(order[j].source, j) == 0);
	}
	for (i = 0; i < 20; i += 2) {
		assert(wl_event_source_timer_update(order[i].source, 0) == 0);
	}

	/* Wait until all timers are due */
	usleep(MSEC_TO_USEC(21));
	wl_event_loop_dispatch(loop, 0);
	assert(last == 19);

	for (i = 0; i < 20; i++) {
		wl_event_source_remove(order[i].source);
	}
	wl_event_loop_destroy(loop);
}

struct timer_cancel_context {
	struct wl_event_source *timers[4];
	struct timer_cancel_context *back_refs[4];
	int order[4];
	int called, first;
};

static int
timer_cancel_callback(void *data) {
	struct timer_cancel_context **context_ref = data;
	struct timer_cancel_context *context = *context_ref;
	int i = (int)(context_ref - context->back_refs);

	context->called++;
	context->order[i] = context->called;

	if (context->called == 1) {
		context->first = i;
		/* Removing a timer always prevents its callback from
		 * being called ... */
		wl_event_source_remove(context->timers[(i + 1) % 4]);
		/* ... but disarming or rescheduling a timer does not,
		 * (in the case where the modified timers had already expired
		 * as of when `wl_event_loop_dispatch` was called.) */
		assert(wl_event_source_timer_update(context->timers[(i + 2) % 4],
						    0) == 0);
		assert(wl_event_source_timer_update(context->timers[(i + 3) % 4],
						    2000000000) == 0);
	}

	return 0;
}

TEST(event_loop_timer_cancellation)
{
	struct wl_event_loop *loop = wl_event_loop_create();
	struct timer_cancel_context context;
	int i;

	memset(&context, 0, sizeof(context));

	/* Test that when multiple timers are dispatched in a single call
	 * of `wl_event_loop_dispatch`, that having some timers run code
	 * to modify the other timers only actually prevents the other timers
	 * from running their callbacks when the those timers are removed, not
	 * when they are disarmed or rescheduled. */

	for (i = 0; i < 4; i++) {
		context.back_refs[i] = &context;
		context.timers[i] =
			wl_event_loop_add_timer(loop, timer_cancel_callback,
						&context.back_refs[i]);
		assert(context.timers[i]);

		assert(wl_event_source_timer_update(context.timers[i], 1) == 0);
	}

	usleep(MSEC_TO_USEC(2));
	assert(wl_event_loop_dispatch(loop, 0) == 0);

	/* Tracking which timer was first makes this test independent of the
	 * actual timer dispatch order, which is not guaranteed by the docs */
	assert(context.order[context.first] == 1);
	assert(context.order[(context.first + 1) % 4] == 0);
	assert(context.order[(context.first + 2) % 4] > 1);
	assert(context.order[(context.first + 3) % 4] > 1);

	wl_event_source_remove(context.timers[context.first]);
	wl_event_source_remove(context.timers[(context.first + 2) % 4]);
	wl_event_source_remove(context.timers[(context.first + 3) % 4]);

	wl_event_loop_destroy(loop);
}

struct event_loop_destroy_listener {
	struct wl_listener listener;
	int done;
};

static void
event_loop_destroy_notify(struct wl_listener *l, void *data)
{
	struct event_loop_destroy_listener *listener =
		wl_container_of(l, listener, listener);

	listener->done = 1;
}

TEST(event_loop_destroy)
{
	struct wl_event_loop *loop;
	struct wl_display * display;
	struct event_loop_destroy_listener a, b;

	loop = wl_event_loop_create();
	assert(loop);

	a.listener.notify = &event_loop_destroy_notify;
	a.done = 0;
	wl_event_loop_add_destroy_listener(loop, &a.listener);

	assert(wl_event_loop_get_destroy_listener(loop,
	       event_loop_destroy_notify) == &a.listener);

	b.listener.notify = &event_loop_destroy_notify;
	b.done = 0;
	wl_event_loop_add_destroy_listener(loop, &b.listener);

	wl_list_remove(&a.listener.link);
	wl_event_loop_destroy(loop);

	assert(!a.done);
	assert(b.done);

	/* Test to make sure it gets fired on display destruction */
	display = wl_display_create();
	assert(display);
	loop = wl_display_get_event_loop(display);
	assert(loop);

	a.done = 0;
	wl_event_loop_add_destroy_listener(loop, &a.listener);

	wl_display_destroy(display);

	assert(a.done);
}

