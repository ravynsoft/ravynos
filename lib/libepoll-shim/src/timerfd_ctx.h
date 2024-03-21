#ifndef TIMERFD_CTX_H_
#define TIMERFD_CTX_H_

#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <pthread.h>
#include <time.h>

typedef enum {
	TIMER_TYPE_UNSPECIFIED,
	TIMER_TYPE_RELATIVE,
	TIMER_TYPE_ABSOLUTE,
} TimerType;

typedef struct {
	bool is_abstime;

	clockid_t clockid;
	TimerType timer_type;
	bool is_cancel_on_set;
	bool force_cancel;
	struct timespec monotonic_offset;
	/*
	 * Next expiration time, absolute (clock given by clockid).
	 * If it_interval is != 0, it is a periodic timer.
	 * If it_value is == 0, the timer is disarmed.
	 */
	struct itimerspec current_itimerspec;
	uint64_t nr_expirations;
} TimerFDCtx;

errno_t timerfd_ctx_init(TimerFDCtx *timerfd, int clockid);
errno_t timerfd_ctx_terminate(TimerFDCtx *timerfd);

errno_t timerfd_ctx_settime(TimerFDCtx *timerfd, int kq, /**/
    bool is_abstime, bool is_cancel_on_set,		 /**/
    struct itimerspec const *new, struct itimerspec *old);
errno_t timerfd_ctx_gettime(TimerFDCtx *timerfd, struct itimerspec *cur);

errno_t timerfd_ctx_read(TimerFDCtx *timerfd, int kq, uint64_t *value);
void timerfd_ctx_poll(TimerFDCtx *timerfd, int kq, uint32_t *revents);

errno_t timerfd_ctx_get_monotonic_offset(struct timespec *monotonic_offset);
void timerfd_ctx_realtime_change(TimerFDCtx *timerfd, int kq);

#endif
