/*
 * Copyright © 2014-2015 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "libinput-private.h"
#include "timer.h"

void
libinput_timer_init(struct libinput_timer *timer,
		    struct libinput *libinput,
		    const char *timer_name,
		    void (*timer_func)(uint64_t now, void *timer_func_data),
		    void *timer_func_data)
{
	timer->libinput = libinput;
	timer->timer_name = safe_strdup(timer_name);
	timer->timer_func = timer_func;
	timer->timer_func_data = timer_func_data;
	/* at most 5 "expiry in the past" log messages per hour */
	ratelimit_init(&libinput->timer.expiry_in_past_limit,
		       s2us(60 * 60), 5);
}

void
libinput_timer_destroy(struct libinput_timer *timer)
{
	if (timer->link.prev != NULL && timer->link.next != NULL &&
	    !list_empty(&timer->link)) {
		log_bug_libinput(timer->libinput,
				 "timer: %s has not been cancelled\n",
				 timer->timer_name);
		assert(!"timer not cancelled");
	}
	free(timer->timer_name);
}

static void
libinput_timer_arm_timer_fd(struct libinput *libinput)
{
	int r;
	struct libinput_timer *timer;
	struct itimerspec its = { { 0, 0 }, { 0, 0 } };
	uint64_t earliest_expire = UINT64_MAX;

	list_for_each(timer, &libinput->timer.list, link) {
		if (timer->expire < earliest_expire)
			earliest_expire = timer->expire;
	}

	if (earliest_expire != UINT64_MAX) {
		its.it_value.tv_sec = earliest_expire / ms2us(1000);
		its.it_value.tv_nsec = (earliest_expire % ms2us(1000)) * 1000;
	}

	r = timerfd_settime(libinput->timer.fd, TFD_TIMER_ABSTIME, &its, NULL);
	if (r)
		log_error(libinput, "timer: timerfd_settime error: %s\n", strerror(errno));

	libinput->timer.next_expiry = earliest_expire;
}

void
libinput_timer_set_flags(struct libinput_timer *timer,
			 uint64_t expire,
			 uint32_t flags)
{
#ifndef NDEBUG
	/* We only warn if we're more than 20ms behind */
	const uint64_t timer_warning_limit = ms2us(20);
	uint64_t now = libinput_now(timer->libinput);
	if (expire < now) {
		if ((flags & TIMER_FLAG_ALLOW_NEGATIVE) == 0 &&
		    now - expire > timer_warning_limit)
			log_bug_client_ratelimit(timer->libinput,
						 &timer->libinput->timer.expiry_in_past_limit,
						 "timer %s: scheduled expiry is in the past (-%dms), your system is too slow\n",
						 timer->timer_name,
						 us2ms(now - expire));
	} else if ((expire - now) > ms2us(5000)) {
		log_bug_libinput(timer->libinput,
			 "timer %s: offset more than 5s, now %d expire %d\n",
			 timer->timer_name,
			 us2ms(now), us2ms(expire));
	}
#endif

	assert(expire);

	if (!timer->expire)
		list_insert(&timer->libinput->timer.list, &timer->link);

	timer->expire = expire;
	libinput_timer_arm_timer_fd(timer->libinput);
}

void
libinput_timer_set(struct libinput_timer *timer, uint64_t expire)
{
	libinput_timer_set_flags(timer, expire, TIMER_FLAG_NONE);
}

void
libinput_timer_cancel(struct libinput_timer *timer)
{
	if (!timer->expire)
		return;

	timer->expire = 0;
	list_remove(&timer->link);
	libinput_timer_arm_timer_fd(timer->libinput);
}

static void
libinput_timer_handler(struct libinput *libinput , uint64_t now)
{
	struct libinput_timer *timer;

restart:
	list_for_each_safe(timer, &libinput->timer.list, link) {
		if (timer->expire == 0)
			continue;

		if (timer->expire <= now) {
			/* Clear the timer before calling timer_func,
			   as timer_func may re-arm it */
			libinput_timer_cancel(timer);
			timer->timer_func(now, timer->timer_func_data);

			/*
			 * Restart the loop. We can't use
			 * list_for_each_safe() here because that only
			 * allows removing one (our) timer per timer_func.
			 * But the timer func may trigger another unrelated
			 * timer to be cancelled and removed, causing a
			 * segfault.
			 */
			goto restart;
		}
	}
}

static void
libinput_timer_dispatch(void *data)
{
	struct libinput *libinput = data;
	uint64_t now;
	uint64_t discard;
	int r;

	r = read(libinput->timer.fd, &discard, sizeof(discard));
	if (r == -1 && errno != EAGAIN)
		log_bug_libinput(libinput,
				 "timer: error %d reading from timerfd (%s)",
				 errno,
				 strerror(errno));

	now = libinput_now(libinput);
	if (now == 0)
		return;

	libinput_timer_handler(libinput, now);
}

int
libinput_timer_subsys_init(struct libinput *libinput)
{
	libinput->timer.fd = timerfd_create(CLOCK_MONOTONIC,
					    TFD_CLOEXEC | TFD_NONBLOCK);
	if (libinput->timer.fd < 0)
		return -1;

	list_init(&libinput->timer.list);

	libinput->timer.source = libinput_add_fd(libinput,
						 libinput->timer.fd,
						 libinput_timer_dispatch,
						 libinput);
	if (!libinput->timer.source) {
		close(libinput->timer.fd);
		return -1;
	}

	return 0;
}

void
libinput_timer_subsys_destroy(struct libinput *libinput)
{
#ifndef NDEBUG
	if (!list_empty(&libinput->timer.list)) {
		struct libinput_timer *t;

		list_for_each(t, &libinput->timer.list, link) {
			log_bug_libinput(libinput,
					 "timer: %s still present on shutdown\n",
					 t->timer_name);
		}
	}
#endif

	/* All timer users should have destroyed their timers now */
	assert(list_empty(&libinput->timer.list));

	libinput_remove_source(libinput, libinput->timer.source);
	close(libinput->timer.fd);
}

/**
 * For a caller calling libinput_dispatch() only infrequently, we may have a
 * timer expiry *and* a later input event waiting in the pipe. We cannot
 * guarantee that we read the timer expiry first, so this hook exists to
 * flush any timers.
 *
 * Assume 'now' is the current time check if there is a current timer expiry
 * before this time. If so, trigger the timer func.
 */
void
libinput_timer_flush(struct libinput *libinput, uint64_t now)
{
	if (libinput->timer.next_expiry == 0 ||
	    libinput->timer.next_expiry > now)
		return;

	libinput_timer_handler(libinput, now);
}
