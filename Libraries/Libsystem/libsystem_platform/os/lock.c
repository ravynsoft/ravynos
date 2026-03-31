/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#define OS_UNFAIR_LOCK_INLINE 1
#include "lock_internal.h"
#include "os/internal.h"
#include "resolver.h"
#include "libkern/OSAtomic.h"
#include "os/lock.h"
#include "os/lock_private.h"
#include "os/once_private.h"

#include <mach/mach_init.h>
#include <mach/mach_traps.h>
#include <mach/thread_switch.h>
#include <mach/mach_time.h>
#include <os/tsd.h>

#pragma mark -
#pragma mark _os_lock_base_t

OS_NOINLINE OS_NORETURN OS_COLD
void _os_lock_corruption_abort(void *lock_ptr OS_UNUSED, uintptr_t lock_value);


OS_LOCK_STRUCT_DECL_INTERNAL(base);
OS_USED static OS_LOCK_TYPE_STRUCT_DECL(base);

void
os_lock_lock(os_lock_t l)
{
	return l._osl_base->osl_type->osl_lock(l);
}

bool
os_lock_trylock(os_lock_t l)
{
	return l._osl_base->osl_type->osl_trylock(l);
}

void
os_lock_unlock(os_lock_t l)
{
	return l._osl_base->osl_type->osl_unlock(l);
}

OS_NOINLINE OS_NORETURN OS_COLD
void
_os_lock_corruption_abort(void *lock_ptr OS_UNUSED, uintptr_t lock_value)
{
	__LIBPLATFORM_CLIENT_CRASH__(lock_value, "os_lock is corrupt");
}


#pragma mark -
#pragma mark OSSpinLock

OS_NOEXPORT OS_NOINLINE void _OSSpinLockLockSlow(volatile OSSpinLock *l);

OS_ATOMIC_EXPORT void OSSpinLockLock(volatile OSSpinLock *l);
OS_ATOMIC_EXPORT bool OSSpinLockTry(volatile OSSpinLock *l);
OS_ATOMIC_EXPORT int spin_lock_try(volatile OSSpinLock *l);
OS_ATOMIC_EXPORT void OSSpinLockUnlock(volatile OSSpinLock *l);

#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
static const OSSpinLock _OSSpinLockLocked = 1;
#else
static const OSSpinLock _OSSpinLockLocked = -1;
#endif



#if OS_ATOMIC_UP
// Don't spin on UP
#else
#define OS_LOCK_SPIN_SPIN_TRIES 1000
#define OS_LOCK_SPIN_PAUSE() os_hardware_pause()
#endif

OS_ALWAYS_INLINE
static uint64_t
_os_lock_yield_deadline(mach_msg_timeout_t timeout)
{
	uint64_t abstime = timeout * NSEC_PER_MSEC;
#if !(defined(__i386__) || defined(__x86_64__))
	mach_timebase_info_data_t tbi;
	kern_return_t kr = mach_timebase_info(&tbi);
	if (kr) return UINT64_MAX;
	abstime *= tbi.denom;
	abstime /= tbi.numer;
#endif
	return mach_absolute_time() + abstime;
}

OS_ALWAYS_INLINE
static bool
_os_lock_yield_until(uint64_t deadline)
{
	return mach_absolute_time() < deadline;
}

OS_NOINLINE
static void
_OSSpinLockLockYield(volatile OSSpinLock *l)
{
	int option = SWITCH_OPTION_DEPRESS;
	mach_msg_timeout_t timeout = 1;
	uint64_t deadline = _os_lock_yield_deadline(timeout);
	OSSpinLock lock;
	while (unlikely(lock = *l)) {
_yield:
		if (unlikely(lock != _OSSpinLockLocked)) {
			_os_lock_corruption_abort((void *)l, (uintptr_t)lock);
		}
		thread_switch(MACH_PORT_NULL, option, timeout);
		if (option == SWITCH_OPTION_WAIT) {
			timeout++;
		} else if (!_os_lock_yield_until(deadline)) {
			option = SWITCH_OPTION_WAIT;
		}
	}
	bool r = os_atomic_cmpxchgv(l, 0, _OSSpinLockLocked, &lock, acquire);
	if (likely(r)) return;
	goto _yield;
}

#if OS_ATOMIC_UP
void
_OSSpinLockLockSlow(volatile OSSpinLock *l)
{
	return _OSSpinLockLockYield(l); // Don't spin on UP
}
#else // !OS_ATOMIC_UP
void
_OSSpinLockLockSlow(volatile OSSpinLock *l)
{
	uint32_t tries = OS_LOCK_SPIN_SPIN_TRIES;
	OSSpinLock lock;
	while (unlikely(lock = *l)) {
_spin:
		if (unlikely(lock != _OSSpinLockLocked)) {
			return _os_lock_corruption_abort((void *)l, (uintptr_t)lock);
		}
		if (unlikely(!tries--)) return _OSSpinLockLockYield(l);
		OS_LOCK_SPIN_PAUSE();
	}
	bool r = os_atomic_cmpxchgv(l, 0, _OSSpinLockLocked, &lock, acquire);
	if (likely(r)) return;
	goto _spin;
}
#endif // !OS_ATOMIC_UP



#if OS_LOCK_OSSPINLOCK_IS_NOSPINLOCK && !TARGET_OS_SIMULATOR

typedef struct _os_nospin_lock_s *_os_nospin_lock_t;

OS_ATOMIC_EXPORT void _os_nospin_lock_lock(_os_nospin_lock_t lock);
OS_ATOMIC_EXPORT bool _os_nospin_lock_trylock(_os_nospin_lock_t lock);
OS_ATOMIC_EXPORT void _os_nospin_lock_unlock(_os_nospin_lock_t lock);

void
OSSpinLockLock(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(spin_lock, OSSpinLockLock);
	OS_ATOMIC_ALIAS(_spin_lock, OSSpinLockLock);
	return _os_nospin_lock_lock((_os_nospin_lock_t)l);
}

bool
OSSpinLockTry(volatile OSSpinLock *l)
{
	return _os_nospin_lock_trylock((_os_nospin_lock_t)l);
}

int
spin_lock_try(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(_spin_lock_try, spin_lock_try);
	return _os_nospin_lock_trylock((_os_nospin_lock_t)l);
}

void
OSSpinLockUnlock(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(spin_unlock, OSSpinLockUnlock);
	OS_ATOMIC_ALIAS(_spin_unlock, OSSpinLockUnlock);
	return _os_nospin_lock_unlock((_os_nospin_lock_t)l);
}

#undef OS_ATOMIC_ALIAS
#define OS_ATOMIC_ALIAS(n, o)
static void _OSSpinLockLock(volatile OSSpinLock *l);
#undef OSSpinLockLock
#define OSSpinLockLock _OSSpinLockLock
static bool _OSSpinLockTry(volatile OSSpinLock *l);
#undef OSSpinLockTry
#define OSSpinLockTry _OSSpinLockTry
static __unused int __spin_lock_try(volatile OSSpinLock *l);
#undef spin_lock_try
#define spin_lock_try __spin_lock_try
static void _OSSpinLockUnlock(volatile OSSpinLock *l);
#undef OSSpinLockUnlock
#define OSSpinLockUnlock _OSSpinLockUnlock

#endif // OS_LOCK_OSSPINLOCK_IS_NOSPINLOCK

void
OSSpinLockLock(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(spin_lock, OSSpinLockLock);
	OS_ATOMIC_ALIAS(_spin_lock, OSSpinLockLock);
	bool r = os_atomic_cmpxchg(l, 0, _OSSpinLockLocked, acquire);
	if (likely(r)) return;
	return _OSSpinLockLockSlow(l);
}

bool
OSSpinLockTry(volatile OSSpinLock *l)
{
	bool r = os_atomic_cmpxchg(l, 0, _OSSpinLockLocked, acquire);
	return r;
}

int
spin_lock_try(volatile OSSpinLock *l) // <rdar://problem/13316060>
{
	OS_ATOMIC_ALIAS(_spin_lock_try, spin_lock_try);
	return OSSpinLockTry(l);
}

void
OSSpinLockUnlock(volatile OSSpinLock *l)
{
	OS_ATOMIC_ALIAS(spin_unlock, OSSpinLockUnlock);
	OS_ATOMIC_ALIAS(_spin_unlock, OSSpinLockUnlock);
	os_atomic_store(l, 0, release);
}


#pragma mark -
#pragma mark os_lock_spin_t

OS_LOCK_STRUCT_DECL_INTERNAL(spin,
	OSSpinLock volatile osl_spinlock;
);
OS_LOCK_METHODS_DECL(spin);
OS_LOCK_TYPE_INSTANCE(spin);

void
_os_lock_spin_lock(_os_lock_spin_t l)
{
	return OSSpinLockLock(&l->osl_spinlock);
}

bool
_os_lock_spin_trylock(_os_lock_spin_t l)
{
	return OSSpinLockTry(&l->osl_spinlock);
}

void
_os_lock_spin_unlock(_os_lock_spin_t l)
{
	return OSSpinLockUnlock(&l->osl_spinlock);
}


#pragma mark -
#pragma mark os_lock_owner_t

#ifndef __TSD_MACH_THREAD_SELF
#define __TSD_MACH_THREAD_SELF 3
#endif

typedef mach_port_name_t os_lock_owner_t;
#define OS_LOCK_NO_OWNER MACH_PORT_NULL


OS_ALWAYS_INLINE OS_CONST
static inline os_lock_owner_t
_os_lock_owner_get_self(void)
{
	os_lock_owner_t self;
	self = (os_lock_owner_t)_os_tsd_get_direct(__TSD_MACH_THREAD_SELF);
	return self;
}


OS_NOINLINE OS_NORETURN OS_COLD
static void
_os_lock_recursive_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Trying to recursively lock an "
			"os_lock");
}


#pragma mark -
#pragma mark os_lock_handoff_t

OS_LOCK_STRUCT_DECL_INTERNAL(handoff,
	os_lock_owner_t volatile osl_owner;
);
OS_LOCK_METHODS_DECL(handoff);
OS_LOCK_TYPE_INSTANCE(handoff);

#define OS_LOCK_HANDOFF_YIELD_TRIES 100

OS_NOINLINE
static void
_os_lock_handoff_lock_slow(_os_lock_handoff_t l)
{
	int option = SWITCH_OPTION_OSLOCK_DEPRESS;
	mach_msg_timeout_t timeout = 1;
	uint32_t tries = OS_LOCK_HANDOFF_YIELD_TRIES;
	os_lock_owner_t self = _os_lock_owner_get_self(), owner;
	while (unlikely(owner = l->osl_owner)) {
_handoff:
		if (unlikely(owner == self)) return _os_lock_recursive_abort(self);
		// Yield until tries first hits zero, then permanently switch to wait
		if (unlikely(!tries--)) option = SWITCH_OPTION_OSLOCK_WAIT;
		thread_switch(owner, option, timeout);
		// Redrive the handoff every 1ms until switching to wait
		if (option == SWITCH_OPTION_OSLOCK_WAIT) timeout++;
	}
	bool r = os_atomic_cmpxchgv2o(l, osl_owner, MACH_PORT_NULL, self, &owner,
			acquire);
	if (likely(r)) return;
	goto _handoff;
}

void
_os_lock_handoff_lock(_os_lock_handoff_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, osl_owner, MACH_PORT_NULL, self, acquire);
	if (likely(r)) return;
	return _os_lock_handoff_lock_slow(l);
}

bool
_os_lock_handoff_trylock(_os_lock_handoff_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, osl_owner, MACH_PORT_NULL, self, acquire);
	return r;
}

void
_os_lock_handoff_unlock(_os_lock_handoff_t l)
{
	os_atomic_store2o(l, osl_owner, MACH_PORT_NULL, release);
}


#pragma mark -
#pragma mark os_ulock_value_t

#include <sys/errno.h>
#include <sys/ulock.h>

typedef os_lock_owner_t os_ulock_value_t;

// This assumes that all thread mach port values always have the low bit set!
// Clearing this bit is used to communicate the existence of waiters to unlock.
#define OS_ULOCK_NOWAITERS_BIT ((os_ulock_value_t)1u)
#define OS_ULOCK_OWNER(value) ((value) | OS_ULOCK_NOWAITERS_BIT)

#define OS_ULOCK_ANONYMOUS_OWNER MACH_PORT_DEAD
#define OS_ULOCK_IS_OWNER(value, self, allow_anonymous_owner) ({ \
		os_lock_owner_t _owner = OS_ULOCK_OWNER(value); (_owner == (self)) && \
		(!(allow_anonymous_owner) || _owner != OS_ULOCK_ANONYMOUS_OWNER); })
#define OS_ULOCK_IS_NOT_OWNER(value, self, allow_anonymous_owner) ({ \
		os_lock_owner_t _owner = OS_ULOCK_OWNER(value); (_owner != (self)) && \
		(!(allow_anonymous_owner) || _owner != OS_ULOCK_ANONYMOUS_OWNER); })

#pragma mark -
#pragma mark os_unfair_lock

typedef struct _os_unfair_lock_s {
	os_ulock_value_t oul_value;
} *_os_unfair_lock_t;

_Static_assert(sizeof(struct os_unfair_lock_s) ==
		sizeof(struct _os_unfair_lock_s), "os_unfair_lock size mismatch");

OS_ATOMIC_EXPORT void os_unfair_lock_lock(os_unfair_lock_t lock);
OS_ATOMIC_EXPORT void os_unfair_lock_lock_with_options(os_unfair_lock_t lock,
		os_unfair_lock_options_t options);
OS_ATOMIC_EXPORT bool os_unfair_lock_trylock(os_unfair_lock_t lock);
OS_ATOMIC_EXPORT void os_unfair_lock_unlock(os_unfair_lock_t lock);

OS_ATOMIC_EXPORT void os_unfair_lock_lock_no_tsd_4libpthread(
		os_unfair_lock_t lock);
OS_ATOMIC_EXPORT void os_unfair_lock_unlock_no_tsd_4libpthread(
		os_unfair_lock_t lock);
OS_ATOMIC_EXPORT void os_unfair_lock_lock_with_options_4Libc(
		os_unfair_lock_t lock, os_unfair_lock_options_t options);
OS_ATOMIC_EXPORT void os_unfair_lock_unlock_4Libc(os_unfair_lock_t lock);

OS_NOINLINE OS_NORETURN OS_COLD
void _os_unfair_lock_recursive_abort(os_lock_owner_t owner);
OS_NOINLINE OS_NORETURN OS_COLD
void _os_unfair_lock_unowned_abort(os_lock_owner_t owner);
OS_NOINLINE OS_NORETURN OS_COLD
void _os_unfair_lock_corruption_abort(os_ulock_value_t current);

_Static_assert(OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION ==
		ULF_WAIT_WORKQ_DATA_CONTENTION,
		"check value for OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION");
_Static_assert(OS_UNFAIR_LOCK_ADAPTIVE_SPIN ==
		ULF_WAIT_ADAPTIVE_SPIN,
		"check value for OS_UNFAIR_LOCK_ADAPTIVE_SPIN");
#define OS_UNFAIR_LOCK_OPTIONS_MASK \
		(os_unfair_lock_options_t)(OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION | \
				OS_UNFAIR_LOCK_ADAPTIVE_SPIN)
#define OS_UNFAIR_LOCK_ALLOW_ANONYMOUS_OWNER 0x01000000u


OS_NOINLINE OS_NORETURN OS_COLD
void
_os_unfair_lock_recursive_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Trying to recursively lock an "
			"os_unfair_lock");
}

OS_NOINLINE OS_NORETURN OS_COLD
void
_os_unfair_lock_unowned_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Unlock of an os_unfair_lock not "
			"owned by current thread");
}

OS_NOINLINE OS_NORETURN OS_COLD
void
_os_unfair_lock_corruption_abort(os_ulock_value_t current)
{
	__LIBPLATFORM_CLIENT_CRASH__(current, "os_unfair_lock is corrupt");
}


OS_NOINLINE
static void
_os_unfair_lock_lock_slow(_os_unfair_lock_t l, os_lock_owner_t self,
		os_unfair_lock_options_t options)
{
	os_unfair_lock_options_t allow_anonymous_owner =
			options & OS_UNFAIR_LOCK_ALLOW_ANONYMOUS_OWNER;
	options &= ~OS_UNFAIR_LOCK_ALLOW_ANONYMOUS_OWNER;
	if (unlikely(options & ~OS_UNFAIR_LOCK_OPTIONS_MASK)) {
		__LIBPLATFORM_CLIENT_CRASH__(options, "Invalid options");
	}
	os_ulock_value_t current, new, waiters_mask = 0;
	while (unlikely((current = os_atomic_load2o(l, oul_value, relaxed)) !=
			OS_LOCK_NO_OWNER)) {
_retry:
		if (unlikely(OS_ULOCK_IS_OWNER(current, self, allow_anonymous_owner))) {
			return _os_unfair_lock_recursive_abort(self);
		}
		new = current & ~OS_ULOCK_NOWAITERS_BIT;
		if (current != new) {
			// Clear nowaiters bit in lock value before waiting
			if (!os_atomic_cmpxchgv2o(l, oul_value, current, new, &current,
					relaxed)){
				continue;
			}
			current = new;
		}
		int ret = __ulock_wait(UL_UNFAIR_LOCK | ULF_NO_ERRNO | options,
				l, current, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
			case EFAULT:
				continue;
			case EOWNERDEAD:
				_os_unfair_lock_corruption_abort(current);
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wait failure");
			}
		}
		if (ret > 0) {
			// If there are more waiters, unset nowaiters bit when acquiring lock
			waiters_mask = OS_ULOCK_NOWAITERS_BIT;
		}
	}
	new = self & ~waiters_mask;
	bool r = os_atomic_cmpxchgv2o(l, oul_value, OS_LOCK_NO_OWNER, new,
			&current, acquire);
	if (unlikely(!r)) goto _retry;
}

OS_NOINLINE
static void
_os_unfair_lock_unlock_slow(_os_unfair_lock_t l, os_ulock_value_t current,
		os_lock_owner_t self, os_unfair_lock_options_t options)
{
	os_unfair_lock_options_t allow_anonymous_owner =
			options & OS_UNFAIR_LOCK_ALLOW_ANONYMOUS_OWNER;
	options &= ~OS_UNFAIR_LOCK_ALLOW_ANONYMOUS_OWNER;
	if (unlikely(OS_ULOCK_IS_NOT_OWNER(current, self, allow_anonymous_owner))) {
		return _os_unfair_lock_unowned_abort(OS_ULOCK_OWNER(current));
	}
	if (current & OS_ULOCK_NOWAITERS_BIT) {
		__LIBPLATFORM_INTERNAL_CRASH__(current, "unlock_slow with no waiters");
	}
	for (;;) {
		int ret = __ulock_wake(UL_UNFAIR_LOCK | ULF_NO_ERRNO, l, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
				continue;
			case ENOENT:
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wake failure");
			}
		}
		break;
	}
}

void
os_unfair_lock_lock(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	if (likely(r)) return;
	return _os_unfair_lock_lock_slow(l, self, OS_UNFAIR_LOCK_NONE);
}

void
os_unfair_lock_lock_with_options(os_unfair_lock_t lock,
		os_unfair_lock_options_t options)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	if (likely(r)) return;
	return _os_unfair_lock_lock_slow(l, self, options);
}

bool
os_unfair_lock_trylock(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	return r;
}

void
os_unfair_lock_unlock(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current;
	current = os_atomic_xchg2o(l, oul_value, OS_LOCK_NO_OWNER, release);
	if (likely(current == self)) return;
	return _os_unfair_lock_unlock_slow(l, current, self, 0);
}

void
os_unfair_lock_lock_no_tsd_4libpthread(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = OS_ULOCK_ANONYMOUS_OWNER;
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	if (likely(r)) return;
	return _os_unfair_lock_lock_slow(l, self,
			OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION|
			OS_UNFAIR_LOCK_ALLOW_ANONYMOUS_OWNER);
}

void
os_unfair_lock_unlock_no_tsd_4libpthread(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = OS_ULOCK_ANONYMOUS_OWNER;
	os_ulock_value_t current;
	current = os_atomic_xchg2o(l, oul_value, OS_LOCK_NO_OWNER, release);
	if (likely(current == self)) return;
	return _os_unfair_lock_unlock_slow(l, current, self,
			OS_UNFAIR_LOCK_ALLOW_ANONYMOUS_OWNER);
}


void
os_unfair_lock_assert_owner(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current = os_atomic_load2o(l, oul_value, relaxed);
	if (unlikely(OS_ULOCK_IS_NOT_OWNER(current, self, 0))) {
		__LIBPLATFORM_CLIENT_CRASH__(current, "Assertion failed: "
				"Lock unexpectedly not owned by current thread");
	}
}

void
os_unfair_lock_assert_not_owner(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current = os_atomic_load2o(l, oul_value, relaxed);
	if (unlikely(OS_ULOCK_IS_OWNER(current, self, 0))) {
		__LIBPLATFORM_CLIENT_CRASH__(current, "Assertion failed: "
				"Lock unexpectedly owned by current thread");
	}
}


#pragma mark -
#pragma mark os_unfair_recursive_lock

OS_ATOMIC_EXPORT
void os_unfair_recursive_lock_lock_with_options(os_unfair_recursive_lock_t lock,
		os_unfair_lock_options_t options);

OS_ATOMIC_EXPORT
bool os_unfair_recursive_lock_trylock(os_unfair_recursive_lock_t lock);

OS_ATOMIC_EXPORT
void os_unfair_recursive_lock_unlock(os_unfair_recursive_lock_t lock);

OS_ATOMIC_EXPORT
bool os_unfair_recursive_lock_tryunlock4objc(os_unfair_recursive_lock_t lock);

OS_ATOMIC_EXPORT
void os_unfair_recursive_lock_unlock_forked_child(os_unfair_recursive_lock_t lock);


static inline os_lock_owner_t
_os_unfair_lock_owner(os_unfair_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	return OS_ULOCK_OWNER(os_atomic_load(&l->oul_value, relaxed));
}


bool
os_unfair_recursive_lock_owned(os_unfair_recursive_lock_t lock)
{
	return _os_unfair_lock_owner(&lock->ourl_lock) ==
			_os_lock_owner_get_self();
}


void
os_unfair_recursive_lock_lock_with_options(os_unfair_recursive_lock_t lock,
		os_unfair_lock_options_t options)
{
	os_lock_owner_t cur, self = _os_lock_owner_get_self();
	_os_unfair_lock_t l = (_os_unfair_lock_t)&lock->ourl_lock;

	if (likely(os_atomic_cmpxchgv2o(l, oul_value,
			OS_LOCK_NO_OWNER, self, &cur, acquire))) {
		return;
	}

	if (OS_ULOCK_OWNER(cur) == self) {
		lock->ourl_count++;
		return;
	}

	return _os_unfair_lock_lock_slow(l, self, options);
}

bool
os_unfair_recursive_lock_trylock(os_unfair_recursive_lock_t lock)
{
	os_lock_owner_t cur, self = _os_lock_owner_get_self();
	_os_unfair_lock_t l = (_os_unfair_lock_t)&lock->ourl_lock;

	if (likely(os_atomic_cmpxchgv2o(l, oul_value,
			OS_LOCK_NO_OWNER, self, &cur, acquire))) {
		return true;
	}

	if (likely(OS_ULOCK_OWNER(cur) == self)) {
		lock->ourl_count++;
		return true;
	}

	return false;
}


OS_ALWAYS_INLINE
static inline void
_os_unfair_recursive_lock_unlock(os_unfair_recursive_lock_t lock,
		os_lock_owner_t self)
{
	if (unlikely(lock->ourl_count)) {
		os_lock_owner_t cur = _os_unfair_lock_owner(&lock->ourl_lock);
		if (unlikely(cur != self)) {
			_os_unfair_lock_unowned_abort(cur);
		}
		lock->ourl_count--;
		return;
	}

	_os_unfair_lock_t l = (_os_unfair_lock_t)lock;
	os_ulock_value_t current;
	current = os_atomic_xchg2o(l, oul_value, OS_LOCK_NO_OWNER, release);
	if (likely(current == self)) return;
	return _os_unfair_lock_unlock_slow(l, current, self, 0);
}

void
os_unfair_recursive_lock_unlock(os_unfair_recursive_lock_t lock)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	_os_unfair_recursive_lock_unlock(lock, self);
}

bool
os_unfair_recursive_lock_tryunlock4objc(os_unfair_recursive_lock_t lock)
{
	os_lock_owner_t cur = _os_unfair_lock_owner(&lock->ourl_lock);
	os_lock_owner_t self = _os_lock_owner_get_self();
	if (likely(cur == self)) {
		_os_unfair_recursive_lock_unlock(lock, self);
		return true;
	}
	return false;
}

void
os_unfair_recursive_lock_unlock_forked_child(os_unfair_recursive_lock_t lock)
{
	_os_unfair_lock_t l = (_os_unfair_lock_t)&lock->ourl_lock;

	if (os_atomic_load(&l->oul_value, relaxed) == OS_LOCK_NO_OWNER) {
		__LIBPLATFORM_CLIENT_CRASH__(0, "Lock was not held");
	}
	if (lock->ourl_count) {
		os_lock_owner_t self = _os_lock_owner_get_self();
		lock->ourl_count--;
		os_atomic_store(&l->oul_value, self, relaxed);
	} else {
		os_atomic_store(&l->oul_value, OS_LOCK_NO_OWNER, relaxed);
	}
}


#pragma mark -
#pragma mark _os_lock_unfair_t

OS_LOCK_STRUCT_DECL_INTERNAL(unfair,
	os_unfair_lock osl_unfair_lock;
);
OS_LOCK_METHODS_DECL(unfair);
OS_LOCK_TYPE_INSTANCE(unfair);

void
_os_lock_unfair_lock(_os_lock_unfair_t l)
{
	return os_unfair_lock_lock(&l->osl_unfair_lock);
}

bool
_os_lock_unfair_trylock(_os_lock_unfair_t l)
{
	return os_unfair_lock_trylock(&l->osl_unfair_lock);
}

void
_os_lock_unfair_unlock(_os_lock_unfair_t l)
{
	return os_unfair_lock_unlock(&l->osl_unfair_lock);
}


#pragma mark -
#pragma mark _os_nospin_lock

typedef struct _os_nospin_lock_s {
	os_ulock_value_t oul_value;
} _os_nospin_lock, *_os_nospin_lock_t;

_Static_assert(sizeof(OSSpinLock) ==
		sizeof(struct _os_nospin_lock_s), "os_nospin_lock size mismatch");

OS_ATOMIC_EXPORT void _os_nospin_lock_lock(_os_nospin_lock_t lock);
OS_ATOMIC_EXPORT bool _os_nospin_lock_trylock(_os_nospin_lock_t lock);
OS_ATOMIC_EXPORT void _os_nospin_lock_unlock(_os_nospin_lock_t lock);


OS_NOINLINE
static void
_os_nospin_lock_lock_slow(_os_nospin_lock_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current, new, waiters_mask = 0;
	uint32_t timeout = 1;
	while (unlikely((current = os_atomic_load2o(l, oul_value, relaxed)) !=
			OS_LOCK_NO_OWNER)) {
_retry:
		new = current & ~OS_ULOCK_NOWAITERS_BIT;
		// For safer compatibility with OSSpinLock where _OSSpinLockLocked may
		// be 1, check that new didn't become 0 (unlocked) by clearing this bit
		if (current != new && new) {
			// Clear nowaiters bit in lock value before waiting
			if (!os_atomic_cmpxchgv2o(l, oul_value, current, new, &current,
					relaxed)){
				continue;
			}
			current = new;
		}
		int ret = __ulock_wait(UL_COMPARE_AND_WAIT | ULF_NO_ERRNO, l, current,
				timeout * 1000);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case ETIMEDOUT:
				timeout++;
				continue;
			case EINTR:
			case EFAULT:
				continue;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wait failure");
			}
		}
		if (ret > 0) {
			// If there are more waiters, unset nowaiters bit when acquiring lock
			waiters_mask = OS_ULOCK_NOWAITERS_BIT;
		}
	}
	new = self & ~waiters_mask;
	bool r = os_atomic_cmpxchgv2o(l, oul_value, OS_LOCK_NO_OWNER, new,
			&current, acquire);
	if (unlikely(!r)) goto _retry;
}

OS_NOINLINE
static void
_os_nospin_lock_unlock_slow(_os_nospin_lock_t l, os_ulock_value_t current)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	if (unlikely(OS_ULOCK_OWNER(current) != self)) {
		return; // no unowned_abort for drop-in compatibility with OSSpinLock
	}
	if (current & OS_ULOCK_NOWAITERS_BIT) {
		__LIBPLATFORM_INTERNAL_CRASH__(current, "unlock_slow with no waiters");
	}
	for (;;) {
		int ret = __ulock_wake(UL_COMPARE_AND_WAIT | ULF_NO_ERRNO, l, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
				continue;
			case ENOENT:
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wake failure");
			}
		}
		break;
	}
}

void
_os_nospin_lock_lock(_os_nospin_lock_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	if (likely(r)) return;
	return _os_nospin_lock_lock_slow(l);
}

bool
_os_nospin_lock_trylock(_os_nospin_lock_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	bool r = os_atomic_cmpxchg2o(l, oul_value, OS_LOCK_NO_OWNER, self, acquire);
	return r;
}

void
_os_nospin_lock_unlock(_os_nospin_lock_t l)
{
	os_lock_owner_t self = _os_lock_owner_get_self();
	os_ulock_value_t current;
	current = os_atomic_xchg2o(l, oul_value, OS_LOCK_NO_OWNER, release);
	if (likely(current == self)) return;
	return _os_nospin_lock_unlock_slow(l, current);
}


#pragma mark -
#pragma mark _os_lock_nospin_t

OS_LOCK_STRUCT_DECL_INTERNAL(nospin,
	_os_nospin_lock osl_nospin_lock;
);
OS_LOCK_METHODS_DECL(nospin);
OS_LOCK_TYPE_INSTANCE(nospin);

void
_os_lock_nospin_lock(_os_lock_nospin_t l)
{
	return _os_nospin_lock_lock(&l->osl_nospin_lock);
}

bool
_os_lock_nospin_trylock(_os_lock_nospin_t l)
{
	return _os_nospin_lock_trylock(&l->osl_nospin_lock);
}

void
_os_lock_nospin_unlock(_os_lock_nospin_t l)
{
	return _os_nospin_lock_unlock(&l->osl_nospin_lock);
}


#pragma mark -
#pragma mark os_once_t

typedef struct os_once_gate_s {
	union {
		os_ulock_value_t ogo_lock;
		uintptr_t ogo_once;
	};
} os_once_gate_s, *os_once_gate_t;

#define OS_ONCE_INIT ((uintptr_t)0l)
#define OS_ONCE_DONE (~(uintptr_t)0l)

#if defined(__i386__) || defined(__x86_64__)
#define OS_ONCE_USE_QUIESCENT_COUNTER 0
#else
#define OS_ONCE_USE_QUIESCENT_COUNTER 1
#endif

OS_ATOMIC_EXPORT void _os_once(os_once_t *val, void *ctxt, os_function_t func);
OS_ATOMIC_EXPORT void __os_once_reset(os_once_t *val);

OS_NOINLINE OS_NORETURN OS_COLD
void _os_once_gate_recursive_abort(os_lock_owner_t owner);
OS_NOINLINE OS_NORETURN OS_COLD
void _os_once_gate_unowned_abort(os_lock_owner_t owner);
OS_NOINLINE OS_NORETURN OS_COLD
void _os_once_gate_corruption_abort(os_ulock_value_t current);


OS_NOINLINE OS_NORETURN OS_COLD
void
_os_once_gate_recursive_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Trying to recursively lock an "
			"os_once_t");
}

OS_NOINLINE OS_NORETURN OS_COLD
void
_os_once_gate_unowned_abort(os_lock_owner_t owner)
{
	__LIBPLATFORM_CLIENT_CRASH__(owner, "Unlock of an os_once_t not "
			"owned by current thread");
}

OS_NOINLINE OS_NORETURN OS_COLD
void
_os_once_gate_corruption_abort(os_ulock_value_t current)
{
	__LIBPLATFORM_CLIENT_CRASH__(current, "os_once_t is corrupt");
}


#if OS_ONCE_USE_QUIESCENT_COUNTER
#define OS_ONCE_MAKE_GEN(gen)  (((gen) << 2) + OS_ULOCK_NOWAITERS_BIT)
#define OS_ONCE_IS_GEN(gen)    (((gen) & 3) == OS_ULOCK_NOWAITERS_BIT)

// the _COMM_PAGE_CPU_QUIESCENT_COUNTER value is incremented every time
// all CPUs have performed a context switch.
//
// To make sure all CPUs context switched at least once since `gen`,
// we need to observe 4 increments, see libdispatch/src/shims/lock.h
#define OS_ONCE_GEN_SAFE_DELTA  (4 << 2)

OS_ALWAYS_INLINE
static inline uintptr_t
_os_once_generation(void)
{
	uintptr_t value = *(volatile uintptr_t *)_COMM_PAGE_CPU_QUIESCENT_COUNTER;
	return OS_ONCE_MAKE_GEN(value);
}

OS_ALWAYS_INLINE
static inline uintptr_t
_os_once_mark_quiescing(os_once_gate_t og)
{
	return os_atomic_xchg(&og->ogo_once, _os_once_generation(), release);
}

OS_ALWAYS_INLINE
static void
_os_once_mark_done_if_quiesced(os_once_gate_t og, uintptr_t gen)
{
	if (_os_once_generation() - gen >= OS_ONCE_GEN_SAFE_DELTA) {
		os_atomic_store(&og->ogo_once, OS_ONCE_DONE, relaxed);
	}
}
#else
OS_ALWAYS_INLINE
static inline uintptr_t
_os_once_mark_done(os_once_gate_t og)
{
	return os_atomic_xchg(&og->ogo_once, OS_ONCE_DONE, release);
}
#endif

OS_NOINLINE
static void
_os_once_gate_broadcast(os_once_gate_t og, os_ulock_value_t current,
		os_lock_owner_t self)
{
	if (unlikely(OS_ULOCK_IS_NOT_OWNER(current, self, 0))) {
		return _os_once_gate_unowned_abort(OS_ULOCK_OWNER(current));
	}
	if (current & OS_ULOCK_NOWAITERS_BIT) {
		__LIBPLATFORM_INTERNAL_CRASH__(current, "unlock_slow with no waiters");
	}
	for (;;) {
		int ret = __ulock_wake(UL_UNFAIR_LOCK | ULF_NO_ERRNO | ULF_WAKE_ALL,
				&og->ogo_lock, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
				continue;
			case ENOENT:
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wake failure");
			}
		}
		break;
	}
}

OS_NOINLINE
static void
_os_once_callout(os_once_gate_t og, void *ctxt, os_function_t func,
		os_lock_owner_t self)
{
	uintptr_t v;

	func(ctxt);

#if OS_ONCE_USE_QUIESCENT_COUNTER
	v = _os_once_mark_quiescing(og);
#else
	v = _os_once_mark_done(og);
#endif
	if (likely((os_ulock_value_t)v == self)) return;
	_os_once_gate_broadcast(og, (os_ulock_value_t)v, self);
}

OS_NOINLINE
static void
_os_once_gate_wait(os_once_gate_t og, void *ctxt, os_function_t func,
		os_lock_owner_t self)
{
	uintptr_t old, new;

	for (;;) {
		os_atomic_rmw_loop(&og->ogo_once, old, new, relaxed, {
			if (old == OS_ONCE_DONE) {
				os_atomic_rmw_loop_give_up(return);
#if OS_ONCE_USE_QUIESCENT_COUNTER
			} else if (OS_ONCE_IS_GEN(old)) {
				os_atomic_rmw_loop_give_up({
					os_atomic_thread_fence(acquire);
					return _os_once_mark_done_if_quiesced(og, old);
				});
#endif
			} else if (old == OS_ONCE_INIT) {
				// __os_once_reset was used, try to become the new initializer
				new = (uintptr_t)self;
			} else {
				new = old & ~(uintptr_t)OS_ULOCK_NOWAITERS_BIT;
				if (new == old) os_atomic_rmw_loop_give_up(break);
			}
		});
		if (old == OS_ONCE_INIT) {
			// see comment in _os_once, pairs with the release barrier
			// in __os_once_reset()
			os_atomic_thread_fence(acquire);
			return _os_once_callout(og, ctxt, func, self);
		}
		if (unlikely(OS_ULOCK_IS_OWNER((os_lock_owner_t)old, self, 0))) {
			return _os_once_gate_recursive_abort(self);
		}
		int ret = __ulock_wait(UL_UNFAIR_LOCK | ULF_NO_ERRNO,
				&og->ogo_lock, (os_ulock_value_t)new, 0);
		if (unlikely(ret < 0)) {
			switch (-ret) {
			case EINTR:
			case EFAULT:
				continue;
			case EOWNERDEAD:
				_os_once_gate_corruption_abort((os_lock_owner_t)old);
				break;
			default:
				__LIBPLATFORM_INTERNAL_CRASH__(-ret, "ulock_wait failure");
			}
		}
	}
}

// Atomically resets the once value to zero and then signals all
// pending waiters to return from their __ulock_wait()
void
__os_once_reset(os_once_t *val)
{
	os_once_gate_t og = (os_once_gate_t)val;
	os_lock_owner_t self = _os_lock_owner_get_self();
	uintptr_t v;

	v = os_atomic_xchg(&og->ogo_once, OS_ONCE_INIT, release);
	if (likely((os_ulock_value_t)v == self)) return;
	return _os_once_gate_broadcast(og, (os_ulock_value_t)v, self);
}

void
_os_once(os_once_t *val, void *ctxt, os_function_t func)
{
	os_once_gate_t og = (os_once_gate_t)val;
	os_lock_owner_t self;
	uintptr_t v;

#if OS_ONCE_USE_QUIESCENT_COUNTER
	v = os_atomic_load(&og->ogo_once, acquire);
	if (likely(OS_ONCE_IS_GEN(v))) {
		return _os_once_mark_done_if_quiesced(og, v);
	}
#endif

	self = _os_lock_owner_get_self();
	v = (uintptr_t)self;

	// The acquire barrier pairs with the release in __os_once_reset()
	// for cases when a previous initializer failed.
	if (likely(os_atomic_cmpxchg(&og->ogo_once, OS_ONCE_INIT, v, acquire))) {
		return _os_once_callout(og, ctxt, func, self);
	}
	return _os_once_gate_wait(og, ctxt, func, self);
}

