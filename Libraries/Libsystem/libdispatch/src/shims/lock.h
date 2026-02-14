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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_SHIMS_LOCK__
#define __DISPATCH_SHIMS_LOCK__

#pragma mark - platform macros

DISPATCH_OPTIONS(dispatch_lock_options, uint32_t,
	DLOCK_LOCK_NONE				= 0x00000000,
	DLOCK_LOCK_DATA_CONTENTION  = 0x00010000,
);

#if TARGET_OS_MAC

typedef mach_port_t dispatch_tid;
typedef uint32_t dispatch_lock;

#define DLOCK_OWNER_MASK			((dispatch_lock)0xfffffffc)
#define DLOCK_WAITERS_BIT			((dispatch_lock)0x00000001)
#define DLOCK_FAILED_TRYLOCK_BIT	((dispatch_lock)0x00000002)

#define DLOCK_OWNER_NULL			((dispatch_tid)MACH_PORT_NULL)
#define _dispatch_tid_self()		((dispatch_tid)_dispatch_thread_port())

DISPATCH_ALWAYS_INLINE
static inline dispatch_tid
_dispatch_lock_owner(dispatch_lock lock_value)
{
	if (lock_value & DLOCK_OWNER_MASK) {
		return lock_value | DLOCK_WAITERS_BIT | DLOCK_FAILED_TRYLOCK_BIT;
	}
	return DLOCK_OWNER_NULL;
}

#elif defined(__linux__)

#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

typedef uint32_t dispatch_tid;
typedef uint32_t dispatch_lock;

#define DLOCK_OWNER_MASK			((dispatch_lock)FUTEX_TID_MASK)
#define DLOCK_WAITERS_BIT			((dispatch_lock)FUTEX_WAITERS)
#define DLOCK_FAILED_TRYLOCK_BIT	((dispatch_lock)FUTEX_OWNER_DIED)

#define DLOCK_OWNER_NULL			((dispatch_tid)0)
#define _dispatch_tid_self()        ((dispatch_tid)(_dispatch_get_tsd_base()->tid))

DISPATCH_ALWAYS_INLINE
static inline dispatch_tid
_dispatch_lock_owner(dispatch_lock lock_value)
{
	return lock_value & DLOCK_OWNER_MASK;
}

#elif defined(_WIN32)

#include <Windows.h>

typedef DWORD dispatch_tid;
typedef uint32_t dispatch_lock;

#define DLOCK_OWNER_NULL			((dispatch_tid)0)
#define DLOCK_OWNER_MASK			((dispatch_lock)0xfffffffc)
#define DLOCK_WAITERS_BIT			((dispatch_lock)0x00000001)
#define DLOCK_FAILED_TRYLOCK_BIT		((dispatch_lock)0x00000002)

#define _dispatch_tid_self()		((dispatch_tid)(_dispatch_get_tsd_base()->tid << 2))

DISPATCH_ALWAYS_INLINE
static inline dispatch_tid
_dispatch_lock_owner(dispatch_lock lock_value)
{
	return lock_value & DLOCK_OWNER_MASK;
}

#else
#  error define _dispatch_lock encoding scheme for your platform here
#endif

DISPATCH_ALWAYS_INLINE
static inline dispatch_lock
_dispatch_lock_value_from_tid(dispatch_tid tid)
{
	return tid & DLOCK_OWNER_MASK;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_lock
_dispatch_lock_value_for_self(void)
{
	return _dispatch_lock_value_from_tid(_dispatch_tid_self());
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_lock_is_locked(dispatch_lock lock_value)
{
	// equivalent to _dispatch_lock_owner(lock_value) == 0
	return (lock_value & DLOCK_OWNER_MASK) != 0;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_lock_is_locked_by(dispatch_lock lock_value, dispatch_tid tid)
{
	// equivalent to _dispatch_lock_owner(lock_value) == tid
	return ((lock_value ^ tid) & DLOCK_OWNER_MASK) == 0;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_lock_is_locked_by_self(dispatch_lock lock_value)
{
	// equivalent to _dispatch_lock_owner(lock_value) == tid
	return ((lock_value ^ _dispatch_tid_self()) & DLOCK_OWNER_MASK) == 0;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_lock_has_waiters(dispatch_lock lock_value)
{
	return (lock_value & DLOCK_WAITERS_BIT);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_lock_has_failed_trylock(dispatch_lock lock_value)
{
	return (lock_value & DLOCK_FAILED_TRYLOCK_BIT);
}

#if __has_include(<sys/ulock.h>)
#include <sys/ulock.h>
#ifdef UL_COMPARE_AND_WAIT
#define HAVE_UL_COMPARE_AND_WAIT 1
#endif
#ifdef UL_UNFAIR_LOCK
#define HAVE_UL_UNFAIR_LOCK 1
#endif
#endif

#ifndef HAVE_FUTEX
#ifdef __linux__
#define HAVE_FUTEX 1
#else
#define HAVE_FUTEX 0
#endif
#endif // HAVE_FUTEX

#if defined(__x86_64__) || defined(__i386__) || defined(__s390x__)
#define DISPATCH_ONCE_USE_QUIESCENT_COUNTER 0
#elif __APPLE__
#define DISPATCH_ONCE_USE_QUIESCENT_COUNTER 1
#else
#define DISPATCH_ONCE_USE_QUIESCENT_COUNTER 0
#endif

#pragma mark - semaphores

#if USE_MACH_SEM

typedef semaphore_t _dispatch_sema4_t;
#define _DSEMA4_POLICY_FIFO  SYNC_POLICY_FIFO
#define _DSEMA4_POLICY_LIFO  SYNC_POLICY_LIFO
#define _DSEMA4_TIMEOUT() KERN_OPERATION_TIMED_OUT

#define _dispatch_sema4_init(sema, policy) (void)(*(sema) = MACH_PORT_NULL)
#define _dispatch_sema4_is_created(sema)   (*(sema) != MACH_PORT_NULL)
void _dispatch_sema4_create_slow(_dispatch_sema4_t *sema, int policy);

#elif USE_POSIX_SEM

typedef sem_t _dispatch_sema4_t;
#define _DSEMA4_POLICY_FIFO 0
#define _DSEMA4_POLICY_LIFO 0
#define _DSEMA4_TIMEOUT() ((errno) = ETIMEDOUT, -1)

void _dispatch_sema4_init(_dispatch_sema4_t *sema, int policy);
#define _dispatch_sema4_is_created(sema) ((void)sema, 1)
#define _dispatch_sema4_create_slow(sema, policy) ((void)sema, (void)policy)

#elif USE_WIN32_SEM

typedef HANDLE _dispatch_sema4_t;
#define _DSEMA4_POLICY_FIFO 0
#define _DSEMA4_POLICY_LIFO 0
#define _DSEMA4_TIMEOUT() ((errno) = ETIMEDOUT, -1)

#define _dispatch_sema4_init(sema, policy) (void)(*(sema) = 0)
#define _dispatch_sema4_is_created(sema)   (*(sema) != 0)
void _dispatch_sema4_create_slow(_dispatch_sema4_t *sema, int policy);

#else
#error "port has to implement _dispatch_sema4_t"
#endif

void _dispatch_sema4_dispose_slow(_dispatch_sema4_t *sema, int policy);
void _dispatch_sema4_signal(_dispatch_sema4_t *sema, long count);
void _dispatch_sema4_wait(_dispatch_sema4_t *sema);
bool _dispatch_sema4_timedwait(_dispatch_sema4_t *sema, dispatch_time_t timeout);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_sema4_create(_dispatch_sema4_t *sema, int policy)
{
	if (!_dispatch_sema4_is_created(sema)) {
		_dispatch_sema4_create_slow(sema, policy);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_sema4_dispose(_dispatch_sema4_t *sema, int policy)
{
	if (_dispatch_sema4_is_created(sema)) {
		_dispatch_sema4_dispose_slow(sema, policy);
	}
}

#pragma mark - compare and wait

DISPATCH_NOT_TAIL_CALLED
int _dispatch_wait_on_address(uint32_t volatile *address, uint32_t value,
		dispatch_time_t timeout, dispatch_lock_options_t flags);
void _dispatch_wake_by_address(uint32_t volatile *address);

#pragma mark - thread event
/*!
 * @typedef dispatch_thread_event_t
 *
 * @abstract
 * Dispatch Thread Events are used for one-time synchronization between threads.
 *
 * @discussion
 * Dispatch Thread Events are cheap synchronization points used when a thread
 * needs to block until a certain event has happened. Dispatch Thread Event
 * must be initialized and destroyed with _dispatch_thread_event_init() and
 * _dispatch_thread_event_destroy().
 *
 * A Dispatch Thread Event must be waited on and signaled exactly once between
 * initialization and destruction. These objects are simpler than semaphores
 * and do not support being signaled and waited on an arbitrary number of times.
 *
 * This locking primitive has no notion of ownership
 */
typedef struct dispatch_thread_event_s {
#if HAVE_UL_COMPARE_AND_WAIT || HAVE_FUTEX
	// 1 means signalled but not waited on yet
	// UINT32_MAX means waited on, but not signalled yet
	// 0 is the initial and final state
	uint32_t dte_value;
#else
	_dispatch_sema4_t dte_sema;
#endif
} dispatch_thread_event_s, *dispatch_thread_event_t;

DISPATCH_NOT_TAIL_CALLED
void _dispatch_thread_event_wait_slow(dispatch_thread_event_t);
void _dispatch_thread_event_signal_slow(dispatch_thread_event_t);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_thread_event_init(dispatch_thread_event_t dte)
{
#if HAVE_UL_COMPARE_AND_WAIT || HAVE_FUTEX
	dte->dte_value = 0;
#else
	_dispatch_sema4_init(&dte->dte_sema, _DSEMA4_POLICY_FIFO);
#endif
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_thread_event_signal(dispatch_thread_event_t dte)
{
#if HAVE_UL_COMPARE_AND_WAIT || HAVE_FUTEX
	if (os_atomic_inc_orig(&dte->dte_value, release) == 0) {
		// 0 -> 1 transition doesn't need a signal
		// force a wake even when the value is corrupt,
		// waiters do the validation
		return;
	}
#else
	// fallthrough
#endif
	_dispatch_thread_event_signal_slow(dte);
}


DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_thread_event_wait(dispatch_thread_event_t dte)
{
#if HAVE_UL_COMPARE_AND_WAIT || HAVE_FUTEX
	if (os_atomic_dec(&dte->dte_value, acquire) == 0) {
		// 1 -> 0 is always a valid transition, so we can return
		// for any other value, take the slow path which checks it's not corrupt
		return;
	}
#else
	// fallthrough
#endif
	_dispatch_thread_event_wait_slow(dte);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_thread_event_destroy(dispatch_thread_event_t dte)
{
#if HAVE_UL_COMPARE_AND_WAIT || HAVE_FUTEX
	// nothing to do
	dispatch_assert(dte->dte_value == 0);
#else
	_dispatch_sema4_dispose(&dte->dte_sema, _DSEMA4_POLICY_FIFO);
#endif
}

#pragma mark - unfair lock

typedef struct dispatch_unfair_lock_s {
	dispatch_lock dul_lock;
} dispatch_unfair_lock_s, *dispatch_unfair_lock_t;

DISPATCH_NOT_TAIL_CALLED
void _dispatch_unfair_lock_lock_slow(dispatch_unfair_lock_t l,
		dispatch_lock_options_t options);
void _dispatch_unfair_lock_unlock_slow(dispatch_unfair_lock_t l,
		dispatch_lock tid_cur);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_unfair_lock_lock(dispatch_unfair_lock_t l)
{
	dispatch_lock value_self = _dispatch_lock_value_for_self();
	if (likely(os_atomic_cmpxchg(&l->dul_lock,
			DLOCK_OWNER_NULL, value_self, acquire))) {
		return;
	}
	return _dispatch_unfair_lock_lock_slow(l, DLOCK_LOCK_DATA_CONTENTION);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_unfair_lock_trylock(dispatch_unfair_lock_t l, dispatch_tid *owner)
{
	dispatch_lock value_self = _dispatch_lock_value_for_self();
	dispatch_lock old_value, new_value;

	os_atomic_rmw_loop(&l->dul_lock, old_value, new_value, acquire, {
		if (likely(!_dispatch_lock_is_locked(old_value))) {
			new_value = value_self;
		} else {
			new_value = old_value | DLOCK_FAILED_TRYLOCK_BIT;
		}
	});
	if (owner) *owner = _dispatch_lock_owner(new_value);
	return !_dispatch_lock_is_locked(old_value);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_unfair_lock_tryunlock(dispatch_unfair_lock_t l)
{
	dispatch_lock old_value, new_value;

	os_atomic_rmw_loop(&l->dul_lock, old_value, new_value, release, {
		if (unlikely(old_value & DLOCK_FAILED_TRYLOCK_BIT)) {
			new_value = old_value ^ DLOCK_FAILED_TRYLOCK_BIT;
		} else {
			new_value = DLOCK_OWNER_NULL;
		}
	});
	if (unlikely(new_value)) {
		// unlock failed, renew the lock, which needs an acquire barrier
		os_atomic_thread_fence(acquire);
		return false;
	}
	if (unlikely(_dispatch_lock_has_waiters(old_value))) {
		_dispatch_unfair_lock_unlock_slow(l, old_value);
	}
	return true;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_unfair_lock_unlock_had_failed_trylock(dispatch_unfair_lock_t l)
{
	dispatch_lock cur, value_self = _dispatch_lock_value_for_self();
#if HAVE_FUTEX
	if (likely(os_atomic_cmpxchgv(&l->dul_lock,
			value_self, DLOCK_OWNER_NULL, &cur, release))) {
		return false;
	}
#else
	cur = os_atomic_xchg(&l->dul_lock, DLOCK_OWNER_NULL, release);
	if (likely(cur == value_self)) return false;
#endif
	_dispatch_unfair_lock_unlock_slow(l, cur);
	return _dispatch_lock_has_failed_trylock(cur);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_unfair_lock_unlock(dispatch_unfair_lock_t l)
{
	(void)_dispatch_unfair_lock_unlock_had_failed_trylock(l);
}

#pragma mark - gate lock

#define DLOCK_GATE_UNLOCKED	((dispatch_lock)0)

#define DLOCK_ONCE_UNLOCKED	((uintptr_t)0)
#define DLOCK_ONCE_DONE		(~(uintptr_t)0)

typedef struct dispatch_gate_s {
	dispatch_lock dgl_lock;
} dispatch_gate_s, *dispatch_gate_t;

typedef struct dispatch_once_gate_s {
	union {
		dispatch_gate_s dgo_gate;
		uintptr_t dgo_once;
	};
} dispatch_once_gate_s, *dispatch_once_gate_t;

#if DISPATCH_ONCE_USE_QUIESCENT_COUNTER
#define DISPATCH_ONCE_MAKE_GEN(gen)  (((gen) << 2) + DLOCK_FAILED_TRYLOCK_BIT)
#define DISPATCH_ONCE_IS_GEN(gen)    (((gen) & 3) == DLOCK_FAILED_TRYLOCK_BIT)

/*
 * the _COMM_PAGE_CPU_QUIESCENT_COUNTER value is incremented every time
 * all CPUs have performed a context switch.
 *
 * A counter update algorithm is:
 *
 *     // atomic_or acq_rel is marked as ======== below
 *     if (atomic_or(&mask, acq_rel) == full_mask) {
 *
 *         tmp = atomic_load(&generation, relaxed);
 *         atomic_store(&generation, gen + 1, relaxed);
 *
 *         // atomic_store release is marked as -------- below
 *         atomic_store(&mask, 0, release);
 *     }
 *
 * This enforces boxes delimited by the acq_rel/release barriers to only be able
 * to observe two possible values for the counter which have been marked below.
 *
 * Lemma 1
 * ~~~~~~~
 *
 * Between two acq_rel barriers, a thread can only observe two possible values
 * of the generation counter G maintained by the kernel.
 *
 * The Figure below, adds the happens-before-relationships and assertions:
 *
 * |     Thread A     |     Thread B     |     Thread C     |
 * |                  |                  |                  |
 * |==================|                  |                  |
 * |      G = N       |                  |                  |
 * |------------------|--------.         |                  |
 * |                  |        |         |                  |
 * |                  |        v         |                  |
 * |                  |==================|                  |
 * |                  |  assert(G >= N)  |                  |
 * |                  |                  |                  |
 * |                  |                  |                  |
 * |                  |                  |                  |
 * |                  |  assert(G < N+2) |                  |
 * |                  |==================|--------.         |
 * |                  |                  |        |         |
 * |                  |                  |        v         |
 * |                  |                  |==================|
 * |                  |                  |      G = N + 2   |
 * |                  |                  |------------------|
 * |                  |                  |                  |
 *
 *
 * This allows us to name the area delimited by two consecutive acq_rel
 * barriers { N, N+1 } after the two possible values of G they can observe,
 * which we'll use from now on.
 *
 *
 * Lemma 2
 * ~~~~~~~
 *
 * Any operation that a thread does while observing G in { N-2, N-1 } will be
 * visible to a thread that can observe G in { N, N + 1 }.
 *
 * Any operation that a thread does while observing G in { N, N + 1 } cannot
 * possibly be visible to a thread observing G in { N-2, N-1 }
 *
 * This is a corollary of Lemma 1: the only possibility is for the update
 * of G to N to have happened between two acq_rel barriers of the considered
 * threads.
 *
 * Below is a figure of why instantiated with N = 2
 *
 * |     Thread A     |     Thread B     |     Thread C     |
 * |                  |                  |                  |
 * |   G ∈ { 0, 1 }   |                  |                  |
 * |                  |                  |                  |
 * |                  |                  |                  |
 * |   store(X, 1)    |                  |                  |
 * |   assert(!Z)     |                  |                  |
 * |                  |                  |                  |
 * |==================|--------.         |                  |
 * |   G ∈ { 1, 2 }   |        |         |                  |
 * |                  |        v         |                  |
 * |                  |==================|--------.         |
 * |                  |      G = 2       |        |         |
 * |                  |------------------|        |         |
 * |                  |                  |        |         |
 * |                  |                  |        v         |
 * |                  |                  |==================|
 * |                  |                  |   G ∈ { 2, 3 }   |
 * |                  |                  |                  |
 * |                  |                  |                  |
 * |                  |                  |   store(Z, 1)    |
 * |                  |                  |   assert(X)      |
 * |                  |                  |                  |
 * |                  |                  |                  |
 *
 *
 * Theorem
 * ~~~~~~~
 *
 * The optimal number of increments to observe for the dispatch once algorithm
 * to be safe is 4.
 *
 * Proof (correctness):
 *
 *  Consider a dispatch once initializer thread in its { N, N+1 } "zone".
 *
 *  Per Lemma 2, any observer thread in its { N+2, N+3 } zone will see the
 *  effect of the dispatch once initialization.
 *
 *  Per Lemma 2, when the DONE transition happens in a thread zone { N+3, N+4 },
 *  then threads can observe this transiton in their { N+2, N+3 } zone at the
 *  earliest.
 *
 *  Hence for an initializer bracket of { N, N+1 }, the first safe bracket for
 *  the DONE transition is { N+3, N+4 }.
 *
 *
 * Proof (optimal):
 *
 *  The following ordering is possible if waiting only for three periods:
 *
 * |     Thread A     |     Thread B     |     Thread C     |
 * |                  |                  |                  |
 * |                  |                  |                  |
 * |                  |                  |==================|
 * |                  |                  |   G ∈ { 1, 2 }   |
 * |                  |                  |                  |
 * |                  |                  |                  |
 * |                  |                  |  R(once == -1) <-+--.
 * |                  |                  |                  |  |
 * |           -------+------------------+---------.        |  |
 * |                  |                  |         |        |  |
 * |  W(global, 42)   |                  |         |        |  |
 * |  WRel(once, G:0) |                  |         |        |  |
 * |                  |                  |         |        |  |
 * |                  |                  |         v        |  |
 * |                  |                  |   R(global == 0) |  |
 * |                  |                  |                  |  |
 * |                  |                  |                  |  |
 * |==================|                  |                  |  |
 * |   G ∈ { 1, 2 }   |                  |                  |  |
 * |                  |==================|                  |  |
 * |                  |      G = 2       |                  |  |
 * |                  |------------------|                  |  |
 * |                  |                  |                  |  |
 * |==================|                  |                  |  |
 * |   G ∈ { 2, 3 }   |                  |                  |  |
 * |                  |                  |                  |  |
 * |                  |                  |                  |  |
 * |   W(once, -1) ---+------------------+------------------+--'
 * |                  |                  |                  |
 * |                  |                  |==================|
 * |                  |                  |   G ∈ { 2, 3 }   |
 * |                  |                  |                  |
 *
 */
#define DISPATCH_ONCE_GEN_SAFE_DELTA  (4 << 2)

DISPATCH_ALWAYS_INLINE
static inline uintptr_t
_dispatch_once_generation(void)
{
	uintptr_t value;
	value = *(volatile uintptr_t *)_COMM_PAGE_CPU_QUIESCENT_COUNTER;
	return (uintptr_t)DISPATCH_ONCE_MAKE_GEN(value);
}

DISPATCH_ALWAYS_INLINE
static inline uintptr_t
_dispatch_once_mark_quiescing(dispatch_once_gate_t dgo)
{
	return os_atomic_xchg(&dgo->dgo_once, _dispatch_once_generation(), release);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_once_mark_done_if_quiesced(dispatch_once_gate_t dgo, uintptr_t gen)
{
	if (_dispatch_once_generation() - gen >= DISPATCH_ONCE_GEN_SAFE_DELTA) {
		/*
		 * See explanation above, when the quiescing counter approach is taken
		 * then this store needs only to be relaxed as it is used as a witness
		 * that the required barriers have happened.
		 */
		os_atomic_store(&dgo->dgo_once, DLOCK_ONCE_DONE, relaxed);
	}
}
#else
DISPATCH_ALWAYS_INLINE
static inline uintptr_t
_dispatch_once_mark_done(dispatch_once_gate_t dgo)
{
	return os_atomic_xchg(&dgo->dgo_once, DLOCK_ONCE_DONE, release);
}
#endif // DISPATCH_ONCE_USE_QUIESCENT_COUNTER

void _dispatch_once_wait(dispatch_once_gate_t l);
void _dispatch_gate_broadcast_slow(dispatch_gate_t l, dispatch_lock tid_cur);

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_gate_tryenter(dispatch_gate_t l)
{
	return os_atomic_cmpxchg(&l->dgl_lock, DLOCK_GATE_UNLOCKED,
			_dispatch_lock_value_for_self(), acquire);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_gate_broadcast(dispatch_gate_t l)
{
	dispatch_lock cur, value_self = _dispatch_lock_value_for_self();
	cur = os_atomic_xchg(&l->dgl_lock, DLOCK_GATE_UNLOCKED, release);
	if (likely(cur == value_self)) return;
	_dispatch_gate_broadcast_slow(l, cur);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_once_gate_tryenter(dispatch_once_gate_t l)
{
	return os_atomic_cmpxchg(&l->dgo_once, DLOCK_ONCE_UNLOCKED,
			(uintptr_t)_dispatch_lock_value_for_self(), relaxed);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_once_gate_broadcast(dispatch_once_gate_t l)
{
	dispatch_lock value_self = _dispatch_lock_value_for_self();
	uintptr_t v;
#if DISPATCH_ONCE_USE_QUIESCENT_COUNTER
	v = _dispatch_once_mark_quiescing(l);
#else
	v = _dispatch_once_mark_done(l);
#endif
	if (likely((dispatch_lock)v == value_self)) return;
	_dispatch_gate_broadcast_slow(&l->dgo_gate, (dispatch_lock)v);
}

#if TARGET_OS_MAC

DISPATCH_NOT_TAIL_CALLED
void _dispatch_firehose_gate_wait(dispatch_gate_t l, uint32_t owner,
		uint32_t flags);

#endif // TARGET_OS_MAC

#endif // __DISPATCH_SHIMS_LOCK__
