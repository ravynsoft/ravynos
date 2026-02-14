/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "internal.h"

#if TARGET_OS_MAC
dispatch_static_assert(DLOCK_LOCK_DATA_CONTENTION ==
		ULF_WAIT_WORKQ_DATA_CONTENTION);

#if !HAVE_UL_UNFAIR_LOCK
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_thread_switch(dispatch_lock value, dispatch_lock_options_t flags,
		uint32_t timeout)
{
	int option;
	if (flags & DLOCK_LOCK_DATA_CONTENTION) {
		option = SWITCH_OPTION_OSLOCK_DEPRESS;
	} else {
		option = SWITCH_OPTION_DEPRESS;
	}
	thread_switch(_dispatch_lock_owner(value), option, timeout);
}
#endif // HAVE_UL_UNFAIR_LOCK
#endif

#if defined(_WIN32)
#if !HAVE_UL_UNFAIR_LOCK
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_thread_switch(dispatch_lock value, dispatch_lock_options_t flags,
		uint32_t timeout)
{
	(void)value;
	(void)flags;
	(void)timeout;
	SwitchToThread();
}
#endif
#endif

#pragma mark - semaphores

#if USE_MACH_SEM
#if __has_include(<os/semaphore_private.h>)
#include <os/semaphore_private.h>
#define DISPATCH_USE_OS_SEMAPHORE_CACHE 1
#else
#define DISPATCH_USE_OS_SEMAPHORE_CACHE 0
#endif

#define DISPATCH_SEMAPHORE_VERIFY_KR(x) do { \
		DISPATCH_VERIFY_MIG(x); \
		if (unlikely((x) == KERN_INVALID_NAME)) { \
			DISPATCH_CLIENT_CRASH((x), \
				"Use-after-free of dispatch_semaphore_t or dispatch_group_t"); \
		} else if (unlikely(x)) { \
			DISPATCH_INTERNAL_CRASH((x), "mach semaphore API failure"); \
		} \
	} while (0)

void
_dispatch_sema4_create_slow(_dispatch_sema4_t *s4, int policy)
{
	semaphore_t tmp = MACH_PORT_NULL;

	_dispatch_fork_becomes_unsafe();

	// lazily allocate the semaphore port

	// Someday:
	// 1) Switch to a doubly-linked FIFO in user-space.
	// 2) User-space timers for the timeout.

#if DISPATCH_USE_OS_SEMAPHORE_CACHE
	if (policy == _DSEMA4_POLICY_FIFO) {
		tmp = (_dispatch_sema4_t)os_get_cached_semaphore();
		if (!os_atomic_cmpxchg(s4, MACH_PORT_NULL, tmp, relaxed)) {
			os_put_cached_semaphore((os_semaphore_t)tmp);
		}
		return;
	}
#endif

	kern_return_t kr = semaphore_create(mach_task_self(), &tmp, policy, 0);
	DISPATCH_SEMAPHORE_VERIFY_KR(kr);

	if (!os_atomic_cmpxchg(s4, MACH_PORT_NULL, tmp, relaxed)) {
		kr = semaphore_destroy(mach_task_self(), tmp);
		DISPATCH_SEMAPHORE_VERIFY_KR(kr);
	}
}

void
_dispatch_sema4_dispose_slow(_dispatch_sema4_t *sema, int policy)
{
	semaphore_t sema_port = *sema;
	*sema = MACH_PORT_DEAD;
#if DISPATCH_USE_OS_SEMAPHORE_CACHE
	if (policy == _DSEMA4_POLICY_FIFO) {
		return os_put_cached_semaphore((os_semaphore_t)sema_port);
	}
#endif
	kern_return_t kr = semaphore_destroy(mach_task_self(), sema_port);
	DISPATCH_SEMAPHORE_VERIFY_KR(kr);
}

void
_dispatch_sema4_signal(_dispatch_sema4_t *sema, long count)
{
	do {
		kern_return_t kr = semaphore_signal(*sema);
		DISPATCH_SEMAPHORE_VERIFY_KR(kr);
	} while (--count);
}

void
_dispatch_sema4_wait(_dispatch_sema4_t *sema)
{
	kern_return_t kr;
	do {
		kr = semaphore_wait(*sema);
	} while (kr == KERN_ABORTED);
	DISPATCH_SEMAPHORE_VERIFY_KR(kr);
}

bool
_dispatch_sema4_timedwait(_dispatch_sema4_t *sema, dispatch_time_t timeout)
{
	mach_timespec_t _timeout;
	kern_return_t kr;

	do {
		uint64_t nsec = _dispatch_timeout(timeout);
		_timeout.tv_sec = (__typeof__(_timeout.tv_sec))(nsec / NSEC_PER_SEC);
		_timeout.tv_nsec = (__typeof__(_timeout.tv_nsec))(nsec % NSEC_PER_SEC);
		kr = semaphore_timedwait(*sema, _timeout);
	} while (unlikely(kr == KERN_ABORTED));

	if (kr == KERN_OPERATION_TIMED_OUT) {
		return true;
	}
	DISPATCH_SEMAPHORE_VERIFY_KR(kr);
	return false;
}
#elif USE_POSIX_SEM
#define DISPATCH_SEMAPHORE_VERIFY_RET(x) do { \
		if (unlikely((x) == -1)) { \
			DISPATCH_INTERNAL_CRASH(errno, "POSIX semaphore API failure"); \
		} \
	} while (0)

void
_dispatch_sema4_init(_dispatch_sema4_t *sema, int policy DISPATCH_UNUSED)
{
	int rc = sem_init(sema, 0, 0);
	DISPATCH_SEMAPHORE_VERIFY_RET(rc);
}

void
_dispatch_sema4_dispose_slow(_dispatch_sema4_t *sema, int policy DISPATCH_UNUSED)
{
	int rc = sem_destroy(sema);
	DISPATCH_SEMAPHORE_VERIFY_RET(rc);
}

void
_dispatch_sema4_signal(_dispatch_sema4_t *sema, long count)
{
	do {
		int ret = sem_post(sema);
		DISPATCH_SEMAPHORE_VERIFY_RET(ret);
	} while (--count);
}

void
_dispatch_sema4_wait(_dispatch_sema4_t *sema)
{
	int ret = sem_wait(sema);
	DISPATCH_SEMAPHORE_VERIFY_RET(ret);
}

bool
_dispatch_sema4_timedwait(_dispatch_sema4_t *sema, dispatch_time_t timeout)
{
	struct timespec _timeout;
	int ret;

	do {
		uint64_t nsec = _dispatch_time_nanoseconds_since_epoch(timeout);
		_timeout.tv_sec = (__typeof__(_timeout.tv_sec))(nsec / NSEC_PER_SEC);
		_timeout.tv_nsec = (__typeof__(_timeout.tv_nsec))(nsec % NSEC_PER_SEC);
		ret = sem_timedwait(sema, &_timeout);
	} while (unlikely(ret == -1 && errno == EINTR));

	if (ret == -1 && errno == ETIMEDOUT) {
		return true;
	}
	DISPATCH_SEMAPHORE_VERIFY_RET(ret);
	return false;
}
#elif USE_WIN32_SEM
// rdar://problem/8428132
static DWORD best_resolution = 1; // 1ms

static DWORD
_push_timer_resolution(DWORD ms)
{
	MMRESULT res;
	static dispatch_once_t once;

	if (ms > 16) {
		// only update timer resolution if smaller than default 15.6ms
		// zero means not updated
		return 0;
	}

	// aim for the best resolution we can accomplish
	dispatch_once(&once, ^{
		TIMECAPS tc;
		if (timeGetDevCaps(&tc, sizeof(tc)) == MMSYSERR_NOERROR) {
			best_resolution = min(max(tc.wPeriodMin, best_resolution),
					tc.wPeriodMax);
		}
	});

	res = timeBeginPeriod(best_resolution);
	if (res == TIMERR_NOERROR) {
		return best_resolution;
	}
	// zero means not updated
	return 0;
}

// match ms parameter to result from _push_timer_resolution
DISPATCH_ALWAYS_INLINE
static inline void
_pop_timer_resolution(DWORD ms)
{
	if (ms) timeEndPeriod(ms);
}

void
_dispatch_sema4_create_slow(_dispatch_sema4_t *s4, int policy DISPATCH_UNUSED)
{
	HANDLE tmp;

	// lazily allocate the semaphore port

	while (!dispatch_assume(tmp = CreateSemaphore(NULL, 0, LONG_MAX, NULL))) {
		_dispatch_temporary_resource_shortage();
	}

	if (!os_atomic_cmpxchg(s4, 0, tmp, relaxed)) {
		CloseHandle(tmp);
	}
}

void
_dispatch_sema4_dispose_slow(_dispatch_sema4_t *sema, int policy DISPATCH_UNUSED)
{
	HANDLE sema_handle = *sema;
	CloseHandle(sema_handle);
	*sema = 0;
}

void
_dispatch_sema4_signal(_dispatch_sema4_t *sema, long count)
{
	int ret = ReleaseSemaphore(*sema, count, NULL);
	dispatch_assume(ret);
}

void
_dispatch_sema4_wait(_dispatch_sema4_t *sema)
{
	WaitForSingleObject(*sema, INFINITE);
}

bool
_dispatch_sema4_timedwait(_dispatch_sema4_t *sema, dispatch_time_t timeout)
{
	uint64_t nsec;
	DWORD msec;
	DWORD resolution;
	DWORD wait_result;

	nsec = _dispatch_timeout(timeout);
	msec = (DWORD)(nsec / (uint64_t)1000000);
	resolution = _push_timer_resolution(msec);
	wait_result = WaitForSingleObject(sema, msec);
	_pop_timer_resolution(resolution);
	return wait_result == WAIT_TIMEOUT;
}
#else
#error "port has to implement _dispatch_sema4_t"
#endif

#pragma mark - ulock wrappers
#if HAVE_UL_COMPARE_AND_WAIT || HAVE_UL_UNFAIR_LOCK

// returns 0, ETIMEDOUT, ENOTEMPTY, EFAULT, EINTR
static int
_dlock_wait(uint32_t *uaddr, uint32_t val, uint32_t timeout, uint32_t flags)
{
	for (;;) {
		int rc = __ulock_wait(flags | ULF_NO_ERRNO, uaddr, val, timeout);
		if (rc > 0) {
			return ENOTEMPTY;
		}
		switch (-rc) {
		case 0:
			return 0;
		case EINTR:
			/*
			 * if we have a timeout, we need to return for the caller to
			 * recompute the new deadline, else just go back to wait.
			 */
			if (timeout == 0) {
				continue;
			}
			/* FALLTHROUGH */
		case ETIMEDOUT:
		case EFAULT:
			return -rc;
		default:
			DISPATCH_INTERNAL_CRASH(-rc, "ulock_wait() failed");
		}
	}
}

static void
_dlock_wake(uint32_t *uaddr, uint32_t flags)
{
	int rc = __ulock_wake(flags | ULF_NO_ERRNO, uaddr, 0);
	if (rc == 0 || rc == -ENOENT) return;
	DISPATCH_INTERNAL_CRASH(-rc, "ulock_wake() failed");
}

#endif // HAVE_UL_COMPARE_AND_WAIT || HAVE_UL_UNFAIR_LOCK
#if HAVE_UL_COMPARE_AND_WAIT

static int
_dispatch_ulock_wait(uint32_t *uaddr, uint32_t val, uint32_t timeout,
		uint32_t flags)
{
	return _dlock_wait(uaddr, val, timeout, flags | UL_COMPARE_AND_WAIT);
}

static void
_dispatch_ulock_wake(uint32_t *uaddr, uint32_t flags)
{
	return _dlock_wake(uaddr, flags | UL_COMPARE_AND_WAIT);
}

#endif // HAVE_UL_COMPARE_AND_WAIT
#if HAVE_UL_UNFAIR_LOCK

static int
_dispatch_unfair_lock_wait(uint32_t *uaddr, uint32_t val, uint32_t timeout,
		dispatch_lock_options_t flags)
{
	return _dlock_wait(uaddr, val, timeout, flags | UL_UNFAIR_LOCK);
}

static void
_dispatch_unfair_lock_wake(uint32_t *uaddr, uint32_t flags)
{
	return _dlock_wake(uaddr, flags | UL_UNFAIR_LOCK);
}

#endif // HAVE_UL_UNFAIR_LOCK
#pragma mark - futex wrappers
#if HAVE_FUTEX
#include <sys/time.h>
#ifdef __ANDROID__
#include <sys/syscall.h>
#else
#include <syscall.h>
#endif /* __ANDROID__ */

DISPATCH_ALWAYS_INLINE
static inline int
_dispatch_futex(uint32_t *uaddr, int op, uint32_t val,
		const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3,
		int opflags)
{
	return (int)syscall(SYS_futex, uaddr, op | opflags, val, timeout, uaddr2, val3);
}

static int
_dispatch_futex_wait(uint32_t *uaddr, uint32_t val,
		const struct timespec *timeout, int opflags)
{
	_dlock_syscall_switch(err,
		_dispatch_futex(uaddr, FUTEX_WAIT, val, timeout, NULL, 0, opflags),
		case 0: case EWOULDBLOCK: case ETIMEDOUT: return err;
		default: DISPATCH_CLIENT_CRASH(err, "futex_wait() failed");
	);
}

static void
_dispatch_futex_wake(uint32_t *uaddr, int wake, int opflags)
{
	int rc;
	_dlock_syscall_switch(err,
		rc = _dispatch_futex(uaddr, FUTEX_WAKE, (uint32_t)wake, NULL, NULL, 0, opflags),
		case 0: return;
		default: DISPATCH_CLIENT_CRASH(err, "futex_wake() failed");
	);
}

static void
_dispatch_futex_lock_pi(uint32_t *uaddr, struct timespec *timeout, int detect,
	      int opflags)
{
	_dlock_syscall_switch(err,
		_dispatch_futex(uaddr, FUTEX_LOCK_PI, (uint32_t)detect, timeout,
				NULL, 0, opflags),
		case 0: return;
		default: DISPATCH_CLIENT_CRASH(errno, "futex_lock_pi() failed");
	);
}

static void
_dispatch_futex_unlock_pi(uint32_t *uaddr, int opflags)
{
	_dlock_syscall_switch(err,
		_dispatch_futex(uaddr, FUTEX_UNLOCK_PI, 0, NULL, NULL, 0, opflags),
		case 0: return;
		default: DISPATCH_CLIENT_CRASH(errno, "futex_unlock_pi() failed");
	);
}

#endif
#pragma mark - wait for address

int
_dispatch_wait_on_address(uint32_t volatile *_address, uint32_t value,
		dispatch_time_t timeout, dispatch_lock_options_t flags)
{
	uint32_t *address = (uint32_t *)_address;
	uint64_t nsecs = _dispatch_timeout(timeout);
	if (nsecs == 0) {
		return ETIMEDOUT;
	}
#if HAVE_UL_COMPARE_AND_WAIT
	uint64_t usecs = 0;
	int rc;
	if (nsecs == DISPATCH_TIME_FOREVER) {
		return _dispatch_ulock_wait(address, value, 0, flags);
	}
	do {
		usecs = howmany(nsecs, NSEC_PER_USEC);
		if (usecs > UINT32_MAX) usecs = UINT32_MAX;
		rc = _dispatch_ulock_wait(address, value, (uint32_t)usecs, flags);
	} while (usecs == UINT32_MAX && rc == ETIMEDOUT &&
			(nsecs = _dispatch_timeout(timeout)) != 0);
	return rc;
#elif HAVE_FUTEX
	if (nsecs != DISPATCH_TIME_FOREVER) {
		struct timespec ts = {
			.tv_sec = (__typeof__(ts.tv_sec))(nsec / NSEC_PER_SEC),
			.tv_nsec = (__typeof__(ts.tv_nsec))(nsec % NSEC_PER_SEC),
		};
		return _dispatch_futex_wait(address, value, &ts, FUTEX_PRIVATE_FLAG);
	}
	return _dispatch_futex_wait(address, value, NULL, FUTEX_PRIVATE_FLAG);
#elif defined(_WIN32)
	WaitOnAddress(address, (PVOID)(uintptr_t)value, sizeof(value), INFINITE);
#else
#error _dispatch_wait_on_address unimplemented for this platform
#endif
}

void
_dispatch_wake_by_address(uint32_t volatile *address)
{
#if HAVE_UL_COMPARE_AND_WAIT
	_dispatch_ulock_wake((uint32_t *)address, ULF_WAKE_ALL);
#elif HAVE_FUTEX
	_dispatch_futex_wake((uint32_t *)address, INT_MAX, FUTEX_PRIVATE_FLAG);
#elif defined(_WIN32)
	WakeByAddressAll((uint32_t *)address);
#else
	(void)address;
#endif
}

#pragma mark - thread event

void
_dispatch_thread_event_signal_slow(dispatch_thread_event_t dte)
{
#if HAVE_UL_COMPARE_AND_WAIT
	_dispatch_ulock_wake(&dte->dte_value, 0);
#elif HAVE_FUTEX
	_dispatch_futex_wake(&dte->dte_value, 1, FUTEX_PRIVATE_FLAG);
#else
	_dispatch_sema4_signal(&dte->dte_sema, 1);
#endif
}

void
_dispatch_thread_event_wait_slow(dispatch_thread_event_t dte)
{
#if HAVE_UL_COMPARE_AND_WAIT || HAVE_FUTEX
	for (;;) {
		uint32_t value = os_atomic_load(&dte->dte_value, acquire);
		if (likely(value == 0)) return;
		if (unlikely(value != UINT32_MAX)) {
			DISPATCH_CLIENT_CRASH(value, "Corrupt thread event value");
		}
#if HAVE_UL_COMPARE_AND_WAIT
		int rc = _dispatch_ulock_wait(&dte->dte_value, UINT32_MAX, 0, 0);
		dispatch_assert(rc == 0 || rc == EFAULT || rc == EINTR);
#elif HAVE_FUTEX
		_dispatch_futex_wait(&dte->dte_value, UINT32_MAX,
				NULL, FUTEX_PRIVATE_FLAG);
#endif
	}
#else
	_dispatch_sema4_wait(&dte->dte_sema);
#endif
}

#pragma mark - unfair lock

#if HAVE_UL_UNFAIR_LOCK
void
_dispatch_unfair_lock_lock_slow(dispatch_unfair_lock_t dul,
		dispatch_lock_options_t flags)
{
	dispatch_lock value_self = _dispatch_lock_value_for_self();
	dispatch_lock old_value, new_value, next = value_self;
	int rc;

	for (;;) {
		os_atomic_rmw_loop(&dul->dul_lock, old_value, new_value, acquire, {
			if (likely(!_dispatch_lock_is_locked(old_value))) {
				new_value = next;
			} else {
				new_value = old_value | DLOCK_WAITERS_BIT;
				if (new_value == old_value) os_atomic_rmw_loop_give_up(break);
			}
		});
		if (unlikely(_dispatch_lock_is_locked_by(old_value, value_self))) {
			DISPATCH_CLIENT_CRASH(0, "trying to lock recursively");
		}
		if (new_value == next) {
			return;
		}
		rc = _dispatch_unfair_lock_wait(&dul->dul_lock, new_value, 0, flags);
		if (rc == ENOTEMPTY) {
			next |= DLOCK_WAITERS_BIT;
		}
	}
}
#elif HAVE_FUTEX
void
_dispatch_unfair_lock_lock_slow(dispatch_unfair_lock_t dul,
		dispatch_lock_options_t flags)
{
	(void)flags;
	_dispatch_futex_lock_pi(&dul->dul_lock, NULL, 1, FUTEX_PRIVATE_FLAG);
}
#else
void
_dispatch_unfair_lock_lock_slow(dispatch_unfair_lock_t dul,
		dispatch_lock_options_t flags)
{
	dispatch_lock cur, self = _dispatch_lock_value_for_self();
	uint32_t timeout = 1;

	while (unlikely(!os_atomic_cmpxchgv(&dul->dul_lock,
			DLOCK_OWNER_NULL, self, &cur, acquire))) {
		if (unlikely(_dispatch_lock_is_locked_by(cur, self))) {
			DISPATCH_CLIENT_CRASH(0, "trying to lock recursively");
		}
		_dispatch_thread_switch(cur, flags, timeout++);
	}
}
#endif

void
_dispatch_unfair_lock_unlock_slow(dispatch_unfair_lock_t dul, dispatch_lock cur)
{
	if (unlikely(!_dispatch_lock_is_locked_by_self(cur))) {
		DISPATCH_CLIENT_CRASH(cur, "lock not owned by current thread");
	}

#if HAVE_UL_UNFAIR_LOCK
	if (_dispatch_lock_has_waiters(cur)) {
		_dispatch_unfair_lock_wake(&dul->dul_lock, 0);
	}
#elif HAVE_FUTEX
	// futex_unlock_pi() handles both OWNER_DIED which we abuse & WAITERS
	_dispatch_futex_unlock_pi(&dul->dul_lock, FUTEX_PRIVATE_FLAG);
#else
	(void)dul;
#endif
}

#pragma mark - gate lock

void
_dispatch_once_wait(dispatch_once_gate_t dgo)
{
	dispatch_lock self = _dispatch_lock_value_for_self();
	uintptr_t old_v, new_v;
	dispatch_lock *lock = &dgo->dgo_gate.dgl_lock;
	uint32_t timeout = 1;

	for (;;) {
		os_atomic_rmw_loop(&dgo->dgo_once, old_v, new_v, relaxed, {
			if (likely(old_v == DLOCK_ONCE_DONE)) {
				os_atomic_rmw_loop_give_up(return);
			}
#if DISPATCH_ONCE_USE_QUIESCENT_COUNTER
			if (DISPATCH_ONCE_IS_GEN(old_v)) {
				os_atomic_rmw_loop_give_up({
					os_atomic_thread_fence(acquire);
					return _dispatch_once_mark_done_if_quiesced(dgo, old_v);
				});
			}
#endif
			new_v = old_v | (uintptr_t)DLOCK_WAITERS_BIT;
			if (new_v == old_v) os_atomic_rmw_loop_give_up(break);
		});
		if (unlikely(_dispatch_lock_is_locked_by((dispatch_lock)old_v, self))) {
			DISPATCH_CLIENT_CRASH(0, "trying to lock recursively");
		}
#if HAVE_UL_UNFAIR_LOCK
		_dispatch_unfair_lock_wait(lock, (dispatch_lock)new_v, 0,
				DLOCK_LOCK_NONE);
#elif HAVE_FUTEX
		_dispatch_futex_wait(lock, (dispatch_lock)new_v, NULL,
				FUTEX_PRIVATE_FLAG);
#else
		_dispatch_thread_switch(new_v, flags, timeout++);
#endif
		(void)timeout;
	}
}

void
_dispatch_gate_broadcast_slow(dispatch_gate_t dgl, dispatch_lock cur)
{
	if (unlikely(!_dispatch_lock_is_locked_by_self(cur))) {
		DISPATCH_CLIENT_CRASH(cur, "lock not owned by current thread");
	}

#if HAVE_UL_UNFAIR_LOCK
	_dispatch_unfair_lock_wake(&dgl->dgl_lock, ULF_WAKE_ALL);
#elif HAVE_FUTEX
	_dispatch_futex_wake(&dgl->dgl_lock, INT_MAX, FUTEX_PRIVATE_FLAG);
#else
	(void)dgl;
#endif
}

#if TARGET_OS_MAC

void
_dispatch_firehose_gate_wait(dispatch_gate_t dgl, uint32_t owner,
		uint32_t flags)
{
	_dispatch_unfair_lock_wait(&dgl->dgl_lock, owner, 0, flags);
}

#endif
