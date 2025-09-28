#include "timerfd_ctx.h"

#include <sys/types.h>

#include <sys/event.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/time.h>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

#include "timespec_util.h"
#include "wrap.h"

static bool
timerfd_ctx_is_disarmed(TimerFDCtx const *timerfd)
{
	return /**/
	    timerfd->current_itimerspec.it_value.tv_sec == 0 &&
	    timerfd->current_itimerspec.it_value.tv_nsec == 0;
}

static bool
timerfd_ctx_is_interval_timer(TimerFDCtx const *timerfd)
{
	return /**/
	    timerfd->current_itimerspec.it_interval.tv_sec != 0 ||
	    timerfd->current_itimerspec.it_interval.tv_nsec != 0;
}

static void
timerfd_ctx_disarm(TimerFDCtx *timerfd)
{
	timerfd->current_itimerspec.it_value.tv_sec = 0;
	timerfd->current_itimerspec.it_value.tv_nsec = 0;
	timerfd->timer_type = TIMER_TYPE_UNSPECIFIED;
}

static errno_t
ts_to_nanos(struct timespec const *ts, int64_t *ts_nanos_out)
{
	int64_t ts_nanos;

	if (__builtin_mul_overflow(ts->tv_sec, 1000000000, &ts_nanos) ||
	    __builtin_add_overflow(ts_nanos, ts->tv_nsec, &ts_nanos)) {
		return EOVERFLOW;
	}

	*ts_nanos_out = ts_nanos;
	return 0;
}

static struct timespec
nanos_to_ts(int64_t ts_nanos)
{
	return (struct timespec) {
		.tv_sec = ts_nanos / 1000000000,
		.tv_nsec = ts_nanos % 1000000000,
	};
}

static bool
timerfd_ctx_can_jump(TimerFDCtx const *timerfd)
{
	return timerfd->clockid == CLOCK_REALTIME &&
	    timerfd->timer_type == TIMER_TYPE_ABSOLUTE;
}

static errno_t
timerfd_ctx_get_clocktime(clockid_t clockid, TimerType timer_type,
    struct timespec *current_time)
{
	assert(timer_type != TIMER_TYPE_UNSPECIFIED);

	if (clockid == CLOCK_REALTIME && timer_type == TIMER_TYPE_RELATIVE) {
		clockid = CLOCK_MONOTONIC;
	}

	if (clock_gettime(clockid, current_time) < 0) {
		return errno;
	}

	return 0;
}

errno_t
timerfd_ctx_get_monotonic_offset(struct timespec *monotonic_offset)
{
#if defined(__DragonFly__)
	if (sysctlbyname("kern.basetime", monotonic_offset,
		&(size_t) { sizeof(*monotonic_offset) }, NULL, 0) < 0) {
		return errno;
	}
#else
	struct timeval boottime;
	if (sysctl((int const[2]) { CTL_KERN, KERN_BOOTTIME }, 2, /**/
		&boottime, &(size_t) { sizeof(boottime) }, NULL, 0) < 0) {
		return errno;
	}

	*monotonic_offset = (struct timespec) {
		.tv_sec = boottime.tv_sec,
		.tv_nsec = (long)boottime.tv_usec * 1000,
	};
#endif
	return 0;
}

static void timerfd_ctx_rearm_kevent(TimerFDCtx *timerfd, int kq,
    struct timespec const *current_time,
    bool need_kevent_delete_on_disarmed_timer);

void
timerfd_ctx_realtime_change(TimerFDCtx *timerfd, int kq)
{
	if (!timerfd_ctx_can_jump(timerfd)) {
		return;
	}

	struct timespec monotonic_offset;
	if (timerfd_ctx_get_monotonic_offset(&monotonic_offset) != 0) {
		return;
	}

	if (timerfd->monotonic_offset.tv_sec == monotonic_offset.tv_sec &&
	    timerfd->monotonic_offset.tv_nsec == monotonic_offset.tv_nsec) {
		return;
	}
	timerfd->monotonic_offset = monotonic_offset;

	if (!timerfd->is_cancel_on_set) {
		struct pollfd pfd = { .fd = kq, .events = POLLIN };
		if (real_poll(&pfd, 1, 0) != 0) {
			return;
		}

		struct timespec current_time;
		if (timerfd_ctx_get_clocktime(timerfd->clockid,
			timerfd->timer_type, &current_time) != 0) {
			return;
		}

		/*
		 * Slightly racy. The timer might have fired in the time
		 * between our check with "poll" and the rearming below.
		 *
		 * To mitigate those kinds of races, TFD_TIMER_CANCEL_ON_SET
		 * should be used anyway.
		 */

		timerfd_ctx_rearm_kevent(timerfd, kq, &current_time, true);
	} else {
		timerfd->force_cancel = true;

		struct kevent kev[2];

		EV_SET(&kev[0], 1, EVFILT_TIMER, /**/
		    EV_DELETE | EV_RECEIPT,	 /**/
		    0, 0, 0);
#ifdef NOTE_USECONDS
		EV_SET(&kev[1], 1, EVFILT_TIMER,
		    EV_ADD | EV_ONESHOT | EV_RECEIPT, /**/
		    NOTE_USECONDS, 0, 0);
#else
		EV_SET(&kev[1], 1, EVFILT_TIMER,
		    EV_ADD | EV_ONESHOT | EV_RECEIPT, /**/
		    0, 0, 0);
#ifdef QUIRK_EVFILT_TIMER_DISALLOWS_ONESHOT_TIMEOUT_ZERO
		kev[1].data = 1;
#endif
#endif

		(void)kevent(kq, kev, 2, kev, 2, NULL);
	}
}

static void
timerfd_ctx_clear_force_cancel(TimerFDCtx *timerfd, int kq)
{
	if (timerfd->force_cancel) {
		struct kevent kev;

		EV_SET(&kev, 1, EVFILT_TIMER, EV_DELETE, 0, 0, 0);
		(void)kevent(kq, &kev, 1, NULL, 0, NULL);
	}

	timerfd->force_cancel = false;
}

static bool
timerfd_ctx_check_for_cancel(TimerFDCtx *timerfd, int kq)
{
	if (timerfd->clockid != CLOCK_REALTIME) {
		return false;
	}

	bool is_stepped = false;
	if (timerfd_ctx_can_jump(timerfd)) {
		struct timespec monotonic_offset;
		if (timerfd_ctx_get_monotonic_offset(&monotonic_offset) != 0) {
			/*
			 * If getting the offset fails here, assume
			 * that it *hasn't* changed.
			 */
			monotonic_offset = timerfd->monotonic_offset;
		}

		is_stepped = timerfd->force_cancel ||
		    timerfd->monotonic_offset.tv_sec !=
			monotonic_offset.tv_sec ||
		    timerfd->monotonic_offset.tv_nsec !=
			monotonic_offset.tv_nsec;
		timerfd->monotonic_offset = monotonic_offset;
	}

	timerfd_ctx_clear_force_cancel(timerfd, kq);

	return is_stepped && timerfd->is_cancel_on_set;
}

static void
timerfd_ctx_update_to_current_time(TimerFDCtx *timerfd,
    struct timespec const *current_time)
{
	if (timerfd_ctx_is_disarmed(timerfd)) {
		return;
	}

	if (timerfd_ctx_is_interval_timer(timerfd)) {
		struct timespec diff_time;

		if (!timespecsub_safe(current_time,
			&timerfd->current_itimerspec.it_value, &diff_time)) {
			goto disarm;
		}

		if (diff_time.tv_sec >= 0) {
			int64_t diff_nanos;
			if (ts_to_nanos(&diff_time, &diff_nanos)) {
				goto disarm;
			}

			int64_t interval_nanos;
			if (ts_to_nanos(
				&timerfd->current_itimerspec.it_interval,
				&interval_nanos)) {
				goto disarm;
			}

			int64_t expirations = diff_nanos / interval_nanos;
			if (expirations == INT64_MAX) {
				goto disarm;
			}
			++expirations;

			int64_t nanos_to_add;
			if (__builtin_mul_overflow(expirations, interval_nanos,
				&nanos_to_add)) {
				goto disarm;
			}

			struct timespec next_ts = nanos_to_ts(nanos_to_add);
			if (!timespecadd_safe(&next_ts,
				&timerfd->current_itimerspec.it_value,
				&next_ts)) {
				goto disarm;
			}

			assert(expirations >= 0);

			timerfd->nr_expirations += (uint64_t)expirations;
			timerfd->current_itimerspec.it_value = next_ts;
		}
	} else {
		if (timespeccmp(current_time,
			&timerfd->current_itimerspec.it_value, >=)) {
			++timerfd->nr_expirations;
			goto disarm;
		}
	}

	assert(timespeccmp(current_time, /**/
	    &timerfd->current_itimerspec.it_value, <));

	return;

disarm:
	timerfd_ctx_disarm(timerfd);
}

#if defined(__NetBSD__) && __NetBSD_Version__ < 999009100

/* On NetBSD, EVFILT_TIMER sometimes returns early. */
#define QUIRKY_EVFILT_TIMER

static bool
round_up_millis(int64_t millis, int64_t *result)
{

	long ticks = CLK_TCK;
	if (ticks <= 0) {
		return false;
	}
	uint32_t ms_per_tick = (uint32_t)(1000 / ticks + !!(1000 % ticks));

	uint32_t ms = (uint32_t)millis;

	/* We need to round up ms to a multiple of the ms per tick. */
	uint32_t fixed_ms = ms / ms_per_tick * ms_per_tick;
	if (fixed_ms < ms) {
		fixed_ms += ms_per_tick;
	}

	/* Then add one tick so that we never sleep shorter than requested. */
	fixed_ms += ms_per_tick;

	/* Add one second for large timeout values (see mstohz in NetBSD for
	 * the reason). */
	if (fixed_ms >= 0x20000 && (fixed_ms % 1000) != 0) {
		fixed_ms += 1000;
	}

	*result = fixed_ms;
	return true;
}
#endif

static errno_t
timerfd_ctx_register_event(TimerFDCtx *timerfd, int kq,
    struct timespec const *new, struct timespec const *current_time)
{
	(void)timerfd;
	assert(new->tv_sec != 0 || new->tv_nsec != 0);

	struct timespec diff_time;
	if (!timespecsub_safe(new, current_time, &diff_time) ||
	    diff_time.tv_sec < 0) {
		diff_time.tv_sec = 0;
		diff_time.tv_nsec = 0;
	}

	struct kevent kev[2];
	bool kev_is_set = false;

	/* Let's hope nobody needs timeouts larger than 10 years. */
	if (diff_time.tv_sec >= 315360000) {
		goto out;
	}

#ifdef NOTE_USECONDS
	if (!kev_is_set) {
		int64_t micros = (int64_t)diff_time.tv_sec * 1000000 +
		    diff_time.tv_nsec / 1000;

		if ((diff_time.tv_nsec % 1000) != 0) {
			++micros;
		}

		/* If there is NOTE_USECONDS support we assume timeout values of
		 * 0 are valid. For FreeBSD this is the case. */

		/* The data field is only 32 bit wide on FreeBSD 11 i386. If
		 * this would overflow, try again with milliseconds. */
		if (!__builtin_add_overflow(micros, 0, &kev[1].data)) {
			EV_SET(&kev[1], 0, EVFILT_TIMER,
			    EV_ADD | EV_ONESHOT | EV_RECEIPT, /**/
			    NOTE_USECONDS, micros, 0);
			kev_is_set = true;
		}
	}
#endif

	if (!kev_is_set) {
#ifdef QUIRKY_EVFILT_TIMER
		/* Let's hope 49 days are enough. */
		if (diff_time.tv_sec >= 4233600) {
			goto out;
		}
#endif

		int64_t millis = (int64_t)diff_time.tv_sec * 1000 +
		    diff_time.tv_nsec / 1000000;

		if ((diff_time.tv_nsec % 1000000) != 0) {
			++millis;
		}

#ifdef QUIRKY_EVFILT_TIMER
		if (millis != 0 && !round_up_millis(millis, &millis)) {
			goto out;
		}
#endif

		if (__builtin_add_overflow(millis, 0, &kev[1].data)) {
			goto out;
		}
		EV_SET(&kev[1], 0, EVFILT_TIMER,      /**/
		    EV_ADD | EV_ONESHOT | EV_RECEIPT, /**/
		    0, millis, 0);
		kev_is_set = true;
	}

	assert(kev_is_set);

#ifdef QUIRK_EVFILT_TIMER_DISALLOWS_ONESHOT_TIMEOUT_ZERO
	if (kev[1].data == 0) {
		kev[1].data = 1;
	}
#endif

out:
	/*
	 * On some BSD's, EVFILT_TIMER ignores timer resets using
	 * EV_ADD, so we have to do it manually.
	 * Do it unconditionally on every OS because the cost should be cheap.
	 */
	EV_SET(&kev[0], 0, EVFILT_TIMER, EV_DELETE | EV_RECEIPT, 0, 0, 0);

	int kev_size = kev_is_set ? 2 : 1;
	int n;
	if ((n = kevent(kq, kev, kev_size, kev, kev_size, NULL)) < 0) {
		return errno;
	}
	assert(n == kev_size);
	assert((kev[0].flags & EV_ERROR) != 0);
	if (!kev_is_set) {
		return 0;
	}

	assert((kev[1].flags & EV_ERROR) != 0);
	return (errno_t)kev[1].data;
}

errno_t
timerfd_ctx_init(TimerFDCtx *timerfd, int clockid)
{
	assert(clockid == CLOCK_MONOTONIC || clockid == CLOCK_REALTIME);

	*timerfd = (TimerFDCtx) {
		.clockid = (clockid_t)clockid,
	};

	return 0;
}

errno_t
timerfd_ctx_terminate(TimerFDCtx *timerfd)
{
	(void)timerfd;

	return 0;
}

static void
timerfd_ctx_gettime_impl(TimerFDCtx *timerfd, struct itimerspec *cur,
    struct timespec const *current_time)
{
	if (current_time != NULL) {
		timerfd_ctx_update_to_current_time(timerfd, current_time);
	}

	*cur = timerfd->current_itimerspec;

	if (current_time != NULL && !timerfd_ctx_is_disarmed(timerfd)) {
		assert(timespeccmp(current_time, &cur->it_value, <));
		timespecsub(&cur->it_value, current_time, &cur->it_value);
	}
}

errno_t
timerfd_ctx_settime(TimerFDCtx *timerfd, int kq, /**/
    bool const is_abstime, bool const is_cancel_on_set,
    struct itimerspec const *new, struct itimerspec *old)
{
	errno_t ec;

	assert(new != NULL);

	if (!itimerspec_is_valid(new)) {
		return EINVAL;
	}

	bool const old_is_cancel_on_set = timerfd->is_cancel_on_set;

	bool has_current_time = false;
	struct timespec current_time;

	if (old) {
		if (timerfd->timer_type != TIMER_TYPE_UNSPECIFIED) {
			if ((ec = timerfd_ctx_get_clocktime(timerfd->clockid,
				 timerfd->timer_type, &current_time)) != 0) {
				return ec;
			}
			has_current_time = true;
		}

		timerfd_ctx_gettime_impl(timerfd, old,
		    timerfd->timer_type == TIMER_TYPE_UNSPECIFIED ?
			NULL :
			&current_time);
	}

	if (new->it_value.tv_sec == 0 && new->it_value.tv_nsec == 0) {
		struct kevent kev;

		EV_SET(&kev, 0, EVFILT_TIMER, EV_DELETE, 0, 0, 0);
		(void)kevent(kq, &kev, 1, NULL, 0, NULL);

		timerfd_ctx_disarm(timerfd);
		timerfd->is_cancel_on_set = false;
		timerfd->is_abstime = false;
	} else {
		TimerType new_timer_type = is_abstime ? /**/
		    TIMER_TYPE_ABSOLUTE :
		    TIMER_TYPE_RELATIVE;

		if (!has_current_time ||
		    timerfd->timer_type != new_timer_type) {
			if ((ec = timerfd_ctx_get_clocktime(timerfd->clockid,
				 new_timer_type, &current_time)) != 0) {
				return ec;
			}
			has_current_time = true;
		}

		struct itimerspec new_absolute;
		if (is_abstime) {
			new_absolute = *new;
		} else {
			new_absolute = (struct itimerspec) {
				.it_interval = new->it_interval,
				.it_value = current_time,
			};

			if (!timespecadd_safe(&new_absolute.it_value,
				&new->it_value, &new_absolute.it_value)) {
				return EINVAL;
			}
		}

		if ((ec = timerfd_ctx_register_event(timerfd, kq,
			 &new_absolute.it_value, &current_time)) != 0) {
			return ec;
		}

		timerfd->current_itimerspec = new_absolute;
		timerfd->timer_type = new_timer_type;
		timerfd->is_cancel_on_set = is_cancel_on_set &&
		    timerfd->clockid == CLOCK_REALTIME &&
		    timerfd->timer_type == TIMER_TYPE_ABSOLUTE;
		timerfd->is_abstime = /**/
		    timerfd->timer_type == TIMER_TYPE_ABSOLUTE;
	}

	timerfd->nr_expirations = 0;
	return (timerfd_ctx_check_for_cancel(timerfd, kq) &&
		   old_is_cancel_on_set) ?
	    ECANCELED :
	    0;
}

errno_t
timerfd_ctx_gettime(TimerFDCtx *timerfd, struct itimerspec *cur)
{
	errno_t ec;

	struct timespec current_time;
	if (timerfd->timer_type != TIMER_TYPE_UNSPECIFIED) {
		if ((ec = timerfd_ctx_get_clocktime(timerfd->clockid,
			 timerfd->timer_type, &current_time)) != 0) {
			return ec;
		}
	}

	timerfd_ctx_gettime_impl(timerfd, cur,
	    timerfd->timer_type == TIMER_TYPE_UNSPECIFIED ? /**/
		NULL :
		&current_time);

	return 0;
}

static void
timerfd_ctx_rearm_kevent(TimerFDCtx *timerfd, int kq,
    struct timespec const *current_time,
    bool need_kevent_delete_on_disarmed_timer)
{
	if (!timerfd_ctx_is_disarmed(timerfd)) {
		if (timerfd_ctx_register_event(timerfd, kq,
			&timerfd->current_itimerspec.it_value,
			current_time) != 0) {
			timerfd_ctx_disarm(timerfd);
		}
	} else {
		if (need_kevent_delete_on_disarmed_timer) {
			struct kevent kev;

			EV_SET(&kev, 0, EVFILT_TIMER, EV_DELETE, 0, 0, 0);
			(void)kevent(kq, &kev, 1, NULL, 0, NULL);
		}
	}
}

errno_t
timerfd_ctx_read(TimerFDCtx *timerfd, int kq, uint64_t *value)
{
	errno_t ec;

	if (timerfd->timer_type == TIMER_TYPE_UNSPECIFIED) {
		return EAGAIN;
	}

	bool got_kevent = false;
	unsigned long event_ident;
	{
		struct kevent kevs[3];
		int n = kevent(kq, NULL, 0, kevs, 3,
		    &(struct timespec) { 0, 0 });
		if (n < 0) {
			return errno;
		}

		for (int i = 0; i < n; ++i) {
			assert(kevs[i].filter == EVFILT_TIMER);

			if (!got_kevent) {
				event_ident = kevs[i].ident;
			} else {
				if (kevs[i].ident == 0) {
					event_ident = 0;
				}
			}
			got_kevent = true;
		}
	}

	bool is_cancelled = timerfd_ctx_check_for_cancel(timerfd, kq);
	if (is_cancelled) {
		if (got_kevent && event_ident == 0) {
			return ECANCELED;
		}

		/*
		 * Since we didn't get the "true" timer event (ident 0) in the
		 * kevent call above, we need to fix it up because the
		 * CLOCK_REALTIME was stepped.
		 *
		 * It's still a bit racy -- it is possible to observe a POLLIN
		 * on the kq but a subsequent read could block, if a clock
		 * change occured after the poll, but before the read.
		 */
	}

	assert(/* Either everything is normal */
	    !is_cancelled ||
	    /* ...or we got a cancellation without kevent
	     * (we just noticed a different boottime) */
	    !got_kevent ||
	    /* ...or we got a kevent (ident 1) cancellation */
	    (got_kevent && event_ident != 0));

	/*
	 * clock_settime(CLOCK_REALTIME) could be called right here
	 * from another thread/process, leading to a race.
	 */

	struct timespec current_time;
	if ((ec = timerfd_ctx_get_clocktime(timerfd->clockid,
		 timerfd->timer_type, &current_time)) != 0) {
		return is_cancelled ? ECANCELED : ec;
	}

	uint64_t nr_expirations;
	if (!is_cancelled) {
		timerfd_ctx_update_to_current_time(timerfd, &current_time);

		nr_expirations = timerfd->nr_expirations;
		timerfd->nr_expirations = 0;

		if (nr_expirations == 0) {
			if (!got_kevent) {
				return EAGAIN;
			}

			assert(event_ident == 0);
			assert(timerfd_ctx_can_jump(timerfd));

			/*
			 * Detect/mitigate the above race (but only for
			 * TFD_TIMER_CANCEL_ON_SET).
			 */
			if (timerfd->is_cancel_on_set) {
				is_cancelled = true;
			} else {
				if (!timerfd_ctx_is_interval_timer(timerfd)) {
					timerfd_ctx_disarm(timerfd);
					nr_expirations = 1;
				}
			}
		}
	}

	assert(is_cancelled || nr_expirations > 0 ||
	    (timerfd_ctx_can_jump(timerfd) &&
		timerfd_ctx_is_interval_timer(timerfd) &&
		!timerfd->is_cancel_on_set));

	timerfd_ctx_rearm_kevent(timerfd, kq, &current_time,
	    (!got_kevent || event_ident != 0));

	if (is_cancelled) {
		return ECANCELED;
	}

	*value = nr_expirations;
	return 0;
}

void
timerfd_ctx_poll(TimerFDCtx *timerfd, int kq, uint32_t *revents)
{
	if (revents != NULL) {
		*revents = POLLIN;
		return;
	}

	timerfd_ctx_realtime_change(timerfd, kq);
}
