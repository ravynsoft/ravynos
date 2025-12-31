/*
 * Copyright (c) 2013-2016 Apple Inc. All rights reserved.
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

#ifndef __OS_LOCK_PRIVATE__
#define __OS_LOCK_PRIVATE__

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <os/base_private.h>
#include <os/lock.h>

OS_ASSUME_NONNULL_BEGIN

/*! @header
 * Low-level lock SPI
 */

#define OS_LOCK_SPI_VERSION 20171006

/*!
 * @typedef os_lock_t
 *
 * @abstract
 * Pointer to one of the os_lock variants.
 */

#define OS_LOCK_TYPE_STRUCT(type) const struct _os_lock_type_##type##_s
#define OS_LOCK_TYPE_REF(type) _os_lock_type_##type
#define OS_LOCK_TYPE_DECL(type) OS_LOCK_TYPE_STRUCT(type) OS_LOCK_TYPE_REF(type)

#define OS_LOCK(type) os_lock_##type##_s
#define OS_LOCK_STRUCT(type) struct OS_LOCK(type)

#if defined(__cplusplus) && __cplusplus >= 201103L

#define OS_LOCK_DECL(type, size) \
		typedef OS_LOCK_STRUCT(type) : public OS_LOCK(base) { \
			private: \
			OS_LOCK_TYPE_STRUCT(type) * osl_type OS_UNUSED; \
			uintptr_t _osl_##type##_opaque[size-1] OS_UNUSED; \
			public: \
			constexpr OS_LOCK(type)() : \
				osl_type(&OS_LOCK_TYPE_REF(type)), _osl_##type##_opaque() {} \
		} OS_LOCK(type)
#define OS_LOCK_INIT(type) {}

typedef OS_LOCK_STRUCT(base) {
	protected:
	constexpr OS_LOCK(base)() {}
} *os_lock_t;

#else

#define OS_LOCK_DECL(type, size) \
		typedef OS_LOCK_STRUCT(type) { \
			OS_LOCK_TYPE_STRUCT(type) * osl_type; \
			uintptr_t _osl_##type##_opaque[size-1]; \
		} OS_LOCK(type)

#define OS_LOCK_INIT(type) { .osl_type = &OS_LOCK_TYPE_REF(type), }

#ifndef OS_LOCK_T_MEMBER
#define OS_LOCK_T_MEMBER(type) OS_LOCK_STRUCT(type) *_osl_##type
#endif

typedef OS_TRANSPARENT_UNION union {
	OS_LOCK_T_MEMBER(base);
	OS_LOCK_T_MEMBER(unfair);
	OS_LOCK_T_MEMBER(nospin);
	OS_LOCK_T_MEMBER(spin);
	OS_LOCK_T_MEMBER(handoff);
} os_lock_t;

#endif

/*!
 * @typedef os_lock_unfair_s
 *
 * @abstract
 * os_lock variant equivalent to os_unfair_lock. Does not spin on contention but
 * waits in the kernel to be woken up by an unlock. The lock value contains
 * ownership information that the system may use to attempt to resolve priority
 * inversions.
 *
 * @discussion
 * Intended as a replacement for os_lock_spin_s or OSSpinLock. Like with
 * OSSpinLock there is no attempt at fairness or lock ordering, e.g. an unlocker
 * can potentially immediately reacquire the lock before a woken up waiter gets
 * an opportunity to attempt to acquire the lock, so starvation is possibile.
 *
 * Must be initialized with OS_LOCK_UNFAIR_INIT
 */
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0)
__TVOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0)
OS_EXPORT OS_LOCK_TYPE_DECL(unfair);
OS_LOCK_DECL(unfair, 2);
#define OS_LOCK_UNFAIR_INIT OS_LOCK_INIT(unfair)

/*!
 * @typedef os_lock_nospin_s
 *
 * @abstract
 * os_lock variant that does not spin on contention but waits in the kernel to
 * be woken up by an unlock. No attempt to resolve priority inversions is made
 * so os_unfair_lock or os_lock_unfair_s should generally be preferred.
 *
 * @discussion
 * Intended as a replacement for os_lock_spin_s or OSSpinLock. Like with
 * OSSpinLock there is no attempt at fairness or lock ordering, e.g. an unlocker
 * can potentially immediately reacquire the lock before a woken up waiter gets
 * an opportunity to attempt to acquire the lock, so starvation is possibile.
 *
 * Must be initialized with OS_LOCK_NOSPIN_INIT
 */
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0)
__TVOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0)
OS_EXPORT OS_LOCK_TYPE_DECL(nospin);
OS_LOCK_DECL(nospin, 2);
#define OS_LOCK_NOSPIN_INIT OS_LOCK_INIT(nospin)

/*!
 * @typedef os_lock_spin_s
 *
 * @abstract
 * Deprecated os_lock variant that on contention starts by spinning trying to
 * acquire the lock, then depressing the priority of the current thread and
 * finally blocking the thread waiting for the lock to become available.
 * Equivalent to OSSpinLock and equally not recommended, see discussion in
 * libkern/OSAtomic.h headerdoc.
 *
 * @discussion
 * Spinlocks are intended to be held only for very brief periods of time. The
 * critical section must not make syscalls and should avoid touching areas of
 * memory that may trigger a page fault, in particular if the critical section
 * may be executing on threads of widely differing priorities or on a mix of
 * IO-throttled and unthrottled threads.
 *
 * Must be initialized with OS_LOCK_SPIN_INIT
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_LOCK_TYPE_DECL(spin);
OS_LOCK_DECL(spin, 2);
#define OS_LOCK_SPIN_INIT OS_LOCK_INIT(spin)

/*!
 * @typedef os_lock_handoff_s
 *
 * @abstract
 * os_lock variant that on contention hands off the current kernel thread to the
 * lock-owning userspace thread (if it is not running), temporarily overriding
 * its priority and IO throttle if necessary.
 *
 * @discussion
 * Intended for use in limited circumstances where the critical section might
 * be executing on threads of widely differing priorities or on a mix of
 * IO-throttled and unthrottled threads where the ordinary os_lock_spin_s would
 * be likely to encounter a priority inversion.
 *
 * IMPORTANT: This lock variant is NOT intended as a general replacement for all
 * uses of os_lock_spin_s or OSSpinLock.
 *
 * Must be initialized with OS_LOCK_HANDOFF_INIT
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_LOCK_TYPE_DECL(handoff);
OS_LOCK_DECL(handoff, 2);
#define OS_LOCK_HANDOFF_INIT OS_LOCK_INIT(handoff)


__BEGIN_DECLS

/*!
 * @function os_lock_lock
 *
 * @abstract
 * Locks an os_lock variant.
 *
 * @param lock
 * Pointer to one of the os_lock variants.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_lock_lock(os_lock_t lock);

/*!
 * @function os_lock_trylock
 *
 * @abstract
 * Locks an os_lock variant if it is not already locked.
 *
 * @param lock
 * Pointer to one of the os_lock variants.
 *
 * @result
 * Returns true if the lock was succesfully locked and false if the lock was
 * already locked.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
bool os_lock_trylock(os_lock_t lock);

/*!
 * @function os_lock_unlock
 *
 * @abstract
 * Unlocks an os_lock variant.
 *
 * @param lock
 * Pointer to one of the os_lock variants.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_lock_unlock(os_lock_t lock);

/*! @group os_unfair_lock SPI
 *
 * @abstract
 * Replacement for the deprecated OSSpinLock. Does not spin on contention but
 * waits in the kernel to be woken up by an unlock. The opaque lock value
 * contains thread ownership information that the system may use to attempt to
 * resolve priority inversions.
 *
 * This lock must be unlocked from the same thread that locked it, attempts to
 * unlock from a different thread will cause an assertion aborting the process.
 *
 * This lock must not be accessed from multiple processes or threads via shared
 * or multiply-mapped memory, the lock implementation relies on the address of
 * the lock value and owning process.
 *
 * @discussion
 * As with OSSpinLock there is no attempt at fairness or lock ordering, e.g. an
 * unlocker can potentially immediately reacquire the lock before a woken up
 * waiter gets an opportunity to attempt to acquire the lock. This may be
 * advantageous for performance reasons, but also makes starvation of waiters a
 * possibility.
 *
 * Must be initialized with OS_UNFAIR_LOCK_INIT
 */

/*!
 * @typedef os_unfair_lock_options_t
 *
 * @const OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION
 * This flag informs the runtime that the specified lock is used for data
 * synchronization and that the lock owner is always able to make progress
 * toward releasing the lock without the help of another thread in the same
 * process. This hint will cause the workqueue subsystem to not create new
 * threads to offset for threads waiting for the lock.
 *
 * When this flag is used, the code running under the critical section should
 * be well known and under your control  (Generally it should not call into
 * framework code).
 *
 * @const OS_UNFAIR_LOCK_ADAPTIVE_SPIN
 * This flag allows for the kernel to use adaptive spinning when the holder
 * of the lock is currently on core. This should only be used for locks
 * where the protected critical section is always extremely short.
 */
OS_OPTIONS(os_unfair_lock_options, uint32_t,
	OS_UNFAIR_LOCK_NONE OS_SWIFT_NAME(None)
		OS_UNFAIR_LOCK_AVAILABILITY = 0x00000000,
	OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION OS_SWIFT_NAME(DataSynchronization)
		OS_UNFAIR_LOCK_AVAILABILITY = 0x00010000,
	OS_UNFAIR_LOCK_ADAPTIVE_SPIN OS_SWIFT_NAME(AdaptiveSpin)
		__API_AVAILABLE(macos(10.15), ios(13.0),
		tvos(13.0), watchos(6.0)) = 0x00040000,
);

#if __swift__
#define OS_UNFAIR_LOCK_OPTIONS_COMPAT_FOR_SWIFT(name) \
		static const os_unfair_lock_options_t \
		name##_FOR_SWIFT OS_SWIFT_NAME(name) = name
OS_UNFAIR_LOCK_OPTIONS_COMPAT_FOR_SWIFT(OS_UNFAIR_LOCK_NONE);
OS_UNFAIR_LOCK_OPTIONS_COMPAT_FOR_SWIFT(OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
OS_UNFAIR_LOCK_OPTIONS_COMPAT_FOR_SWIFT(OS_UNFAIR_LOCK_ADAPTIVE_SPIN);
#undef OS_UNFAIR_LOCK_OPTIONS_COMPAT_FOR_SWIFT
#endif

/*!
 * @function os_unfair_lock_lock_with_options
 *
 * @abstract
 * Locks an os_unfair_lock.
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 *
 * @param options
 * Options to alter the behavior of the lock. See os_unfair_lock_options_t.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_unfair_lock_lock_with_options(os_unfair_lock_t lock,
		os_unfair_lock_options_t options);

/*! @group os_unfair_lock no-TSD interfaces
 *
 * Like the above, but don't require being on a thread with valid TSD, so they
 * can be called from injected mach-threads.  The normal routines use the TSD
 * value for mach_thread_self(), these routines use MACH_PORT_DEAD for the
 * locked value instead.  As a result, they will be unable to resolve priority
 * inversions.
 *
 * This should only be used by libpthread.
 *
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_unfair_lock_lock_no_tsd_4libpthread(os_unfair_lock_t lock);

OS_UNFAIR_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_unfair_lock_unlock_no_tsd_4libpthread(os_unfair_lock_t lock);

/*! @group os_unfair_recursive_lock SPI
 *
 * @abstract
 * Similar to os_unfair_lock, but recursive.
 *
 * @discussion
 * Must be initialized with OS_UNFAIR_RECURSIVE_LOCK_INIT
 */

#define OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY \
		__OSX_AVAILABLE(10.14) __IOS_AVAILABLE(12.0) \
		__TVOS_AVAILABLE(12.0) __WATCHOS_AVAILABLE(5.0)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define OS_UNFAIR_RECURSIVE_LOCK_INIT \
		((os_unfair_recursive_lock){OS_UNFAIR_LOCK_INIT, 0})
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define OS_UNFAIR_RECURSIVE_LOCK_INIT \
		(os_unfair_recursive_lock{OS_UNFAIR_LOCK_INIT, 0})
#elif defined(__cplusplus)
#define OS_UNFAIR_RECURSIVE_LOCK_INIT (os_unfair_recursive_lock(\
		(os_unfair_recursive_lock){OS_UNFAIR_LOCK_INIT, 0}))
#else
#define OS_UNFAIR_RECURSIVE_LOCK_INIT \
		{OS_UNFAIR_LOCK_INIT, 0}
#endif // OS_UNFAIR_RECURSIVE_LOCK_INIT

/*!
 * @typedef os_unfair_recursive_lock
 *
 * @abstract
 * Low-level lock that allows waiters to block efficiently on contention.
 *
 * @discussion
 * See os_unfair_lock.
 *
 */
OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY
typedef struct os_unfair_recursive_lock_s {
	os_unfair_lock ourl_lock;
	uint32_t ourl_count;
} os_unfair_recursive_lock, *os_unfair_recursive_lock_t;

/*!
 * @function os_unfair_recursive_lock_lock_with_options
 *
 * @abstract
 * See os_unfair_lock_lock_with_options
 */
OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_unfair_recursive_lock_lock_with_options(os_unfair_recursive_lock_t lock,
		os_unfair_lock_options_t options);

/*!
 * @function os_unfair_recursive_lock_lock
 *
 * @abstract
 * See os_unfair_lock_lock
 */
OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_NOTHROW OS_NONNULL_ALL
void
os_unfair_recursive_lock_lock(os_unfair_recursive_lock_t lock)
{
	os_unfair_recursive_lock_lock_with_options(lock, OS_UNFAIR_LOCK_NONE);
}

/*!
 * @function os_unfair_recursive_lock_trylock
 *
 * @abstract
 * See os_unfair_lock_trylock
 */
OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_WARN_RESULT OS_NONNULL_ALL
bool os_unfair_recursive_lock_trylock(os_unfair_recursive_lock_t lock);

/*!
 * @function os_unfair_recursive_lock_unlock
 *
 * @abstract
 * See os_unfair_lock_unlock
 */
OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_unfair_recursive_lock_unlock(os_unfair_recursive_lock_t lock);

/*!
 * @function os_unfair_recursive_lock_tryunlock4objc
 *
 * @abstract
 * See os_unfair_lock_unlock
 */
OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
bool os_unfair_recursive_lock_tryunlock4objc(os_unfair_recursive_lock_t lock);

/*!
 * @function os_unfair_recursive_lock_assert_owner
 *
 * @abstract
 * See os_unfair_lock_assert_owner
 */
OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_NOTHROW OS_NONNULL_ALL
void
os_unfair_recursive_lock_assert_owner(os_unfair_recursive_lock_t lock)
{
	os_unfair_lock_assert_owner(&lock->ourl_lock);
}

/*!
 * @function os_unfair_recursive_lock_assert_not_owner
 *
 * @abstract
 * See os_unfair_lock_assert_not_owner
 */
OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_NOTHROW OS_NONNULL_ALL
void
os_unfair_recursive_lock_assert_not_owner(os_unfair_recursive_lock_t lock)
{
	os_unfair_lock_assert_not_owner(&lock->ourl_lock);
}

/*!
 * @function os_unfair_recursive_lock_owned
 *
 * @abstract
 * This function is reserved for the use of people who want to soft-fault
 * when locking models have been violated.
 *
 * @discussion
 * This is meant for SQLite use to detect existing misuse of the API surface,
 * and is not meant for anything else than calling os_log_fault() when such
 * contracts are violated.
 *
 * There's little point to use this value for logic as the
 * os_unfair_recursive_lock is already recursive anyway.
 */
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0)
__TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0)
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
bool
os_unfair_recursive_lock_owned(os_unfair_recursive_lock_t lock);

/*!
 * @function os_unfair_recursive_lock_unlock_forked_child
 *
 * @abstract
 * Function to be used in an atfork child handler to unlock a recursive unfair
 * lock.
 *
 * @discussion
 * This function helps with handling recursive locks in the presence of fork.
 *
 * It is typical to setup atfork handlers that will:
 * - take the lock in the pre-fork handler,
 * - drop the lock in the parent handler,
 * - reset the lock in the forked child.
 *
 * However, because a recursive lock may have been held by the current thread
 * already, reseting needs to act like an unlock.  This function serves for this
 * purpose.  Unlike os_unfair_recursive_lock_unlock(), this function will fixup
 * the lock ownership to match the new identity of the thread after fork().
 */
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0)
__TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0)
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void
os_unfair_recursive_lock_unlock_forked_child(os_unfair_recursive_lock_t lock);

#if __has_attribute(cleanup)

/*!
 * @function os_unfair_lock_scoped_guard_unlock
 *
 * @abstract
 * Used by os_unfair_lock_lock_scoped_guard
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_NOTHROW OS_NONNULL_ALL
void
os_unfair_lock_scoped_guard_unlock(os_unfair_lock_t _Nonnull * _Nonnull lock)
{
	os_unfair_lock_unlock(*lock);
}

/*!
 * @function os_unfair_lock_lock_scoped_guard
 *
 * @abstract
 * Same as os_unfair_lock_lock() except that os_unfair_lock_unlock() is
 * automatically called when the enclosing C scope ends.
 *
 * @param name
 * Name for the variable holding the guard.
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 *
 * @see os_unfair_lock_lock
 * @see os_unfair_lock_unlock
 */
#define os_unfair_lock_lock_scoped_guard(guard_name, lock) \
	os_unfair_lock_t \
		__attribute__((cleanup(os_unfair_lock_scoped_guard_unlock))) \
		guard_name = lock; \
	os_unfair_lock_lock(guard_name)

#endif // __has_attribute(cleanup)

__END_DECLS

OS_ASSUME_NONNULL_END

/*! @group Inline os_unfair_lock interfaces
 *
 * Inline versions of the os_unfair_lock fastpath.
 *
 * Intended exclusively for special highly performance-sensitive cases where the
 * function calls to the os_unfair_lock API entrypoints add measurable overhead.
 *
 * Do not use in frameworks to implement synchronization API primitives that are
 * exposed to developers, that would lead to false positives for that API from
 * tools such as ThreadSanitizer.
 *
 * !!!!!!!!!!!!!!!!!!!!! WARNING WARNING WARNING WARNING !!!!!!!!!!!!!!!!!!!!!
 * DO NOT USE IN CODE THAT IS NOT PART OF THE OPERATING SYSTEM OR THAT IS NOT
 *          REBUILT AS PART OF AN OS WORLDBUILD. YOU HAVE BEEN WARNED!
 * !!!!!!!!!!!!!!!!!!!!! WARNING WARNING WARNING WARNING !!!!!!!!!!!!!!!!!!!!!
 *
 * Define OS_UNFAIR_LOCK_INLINE=1 to indicate that you have read the warning
 * above and still wish to use these interfaces.
 */

#if defined(OS_UNFAIR_LOCK_INLINE) && OS_UNFAIR_LOCK_INLINE

#include <pthread/tsd_private.h>

#ifdef __cplusplus
extern "C++" {
#if !(__has_include(<atomic>) && __has_extension(cxx_atomic))
#error Cannot use inline os_unfair_lock without <atomic> and C++11 atomics
#endif
#include <atomic>
typedef std::atomic<os_unfair_lock> _os_atomic_unfair_lock;
#define OSLOCK_STD(_a) std::_a
__BEGIN_DECLS
#else
#if !(__has_include(<stdatomic.h>) && __has_extension(c_atomic))
#error Cannot use inline os_unfair_lock without <stdatomic.h> and C11 atomics
#endif
#include <stdatomic.h>
typedef _Atomic(os_unfair_lock) _os_atomic_unfair_lock;
#define OSLOCK_STD(_a) _a
#endif

OS_ASSUME_NONNULL_BEGIN

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define OS_UNFAIR_LOCK_UNLOCKED ((os_unfair_lock){0})
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define OS_UNFAIR_LOCK_UNLOCKED (os_unfair_lock{})
#elif defined(__cplusplus)
#define OS_UNFAIR_LOCK_UNLOCKED (os_unfair_lock())
#else
#define OS_UNFAIR_LOCK_UNLOCKED {0}
#endif

/*!
 * @function os_unfair_lock_lock_inline
 *
 * @abstract
 * Locks an os_unfair_lock.
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_NONNULL_ALL
void
os_unfair_lock_lock_inline(os_unfair_lock_t lock)
{
	if (!_pthread_has_direct_tsd()) return os_unfair_lock_lock(lock);
	uint32_t mts = (uint32_t)(uintptr_t)_pthread_getspecific_direct(
			_PTHREAD_TSD_SLOT_MACH_THREAD_SELF);
	os_unfair_lock unlocked = OS_UNFAIR_LOCK_UNLOCKED, locked = { mts };
	if (!OSLOCK_STD(atomic_compare_exchange_strong_explicit)(
			(_os_atomic_unfair_lock*)lock, &unlocked, locked,
			OSLOCK_STD(memory_order_acquire),
			OSLOCK_STD(memory_order_relaxed))) {
		return os_unfair_lock_lock(lock);
	}
}

/*!
 * @function os_unfair_lock_lock_with_options_inline
 *
 * @abstract
 * Locks an os_unfair_lock.
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 *
 * @param options
 * Options to alter the behavior of the lock. See os_unfair_lock_options_t.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_NONNULL_ALL
void
os_unfair_lock_lock_with_options_inline(os_unfair_lock_t lock,
		os_unfair_lock_options_t options)
{
	if (!_pthread_has_direct_tsd()) {
		return os_unfair_lock_lock_with_options(lock, options);
	}
	uint32_t mts = (uint32_t)(uintptr_t)_pthread_getspecific_direct(
			_PTHREAD_TSD_SLOT_MACH_THREAD_SELF);
	os_unfair_lock unlocked = OS_UNFAIR_LOCK_UNLOCKED, locked = { mts };
	if (!OSLOCK_STD(atomic_compare_exchange_strong_explicit)(
			(_os_atomic_unfair_lock*)lock, &unlocked, locked,
			OSLOCK_STD(memory_order_acquire),
			OSLOCK_STD(memory_order_relaxed))) {
		return os_unfair_lock_lock_with_options(lock, options);
	}
}

/*!
 * @function os_unfair_lock_trylock_inline
 *
 * @abstract
 * Locks an os_unfair_lock if it is not already locked.
 *
 * @discussion
 * It is invalid to surround this function with a retry loop, if this function
 * returns false, the program must be able to proceed without having acquired
 * the lock, or it must call os_unfair_lock_lock_inline() instead.
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 *
 * @result
 * Returns true if the lock was succesfully locked and false if the lock was
 * already locked.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_WARN_RESULT OS_NONNULL_ALL
bool
os_unfair_lock_trylock_inline(os_unfair_lock_t lock)
{
	if (!_pthread_has_direct_tsd()) return os_unfair_lock_trylock(lock);
	uint32_t mts = (uint32_t)(uintptr_t)_pthread_getspecific_direct(
			_PTHREAD_TSD_SLOT_MACH_THREAD_SELF);
	os_unfair_lock unlocked = OS_UNFAIR_LOCK_UNLOCKED, locked = { mts };
	return OSLOCK_STD(atomic_compare_exchange_strong_explicit)(
			(_os_atomic_unfair_lock*)lock, &unlocked, locked,
			OSLOCK_STD(memory_order_acquire), OSLOCK_STD(memory_order_relaxed));
}

/*!
 * @function os_unfair_lock_unlock_inline
 *
 * @abstract
 * Unlocks an os_unfair_lock.
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_NONNULL_ALL
void
os_unfair_lock_unlock_inline(os_unfair_lock_t lock)
{
	if (!_pthread_has_direct_tsd()) return os_unfair_lock_unlock(lock);
	uint32_t mts = (uint32_t)(uintptr_t)_pthread_getspecific_direct(
			_PTHREAD_TSD_SLOT_MACH_THREAD_SELF);
	os_unfair_lock unlocked = OS_UNFAIR_LOCK_UNLOCKED, locked = { mts };
	if (!OSLOCK_STD(atomic_compare_exchange_strong_explicit)(
			(_os_atomic_unfair_lock*)lock, &locked, unlocked,
			OSLOCK_STD(memory_order_release),
			OSLOCK_STD(memory_order_relaxed))) {
		return os_unfair_lock_unlock(lock);
	}
}

/*!
 * @function os_unfair_lock_lock_inline_no_tsd_4libpthread
 *
 * @abstract
 * Locks an os_unfair_lock, without requiring valid TSD.
 *
 * This should only be used by libpthread.
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_NONNULL_ALL
void
os_unfair_lock_lock_inline_no_tsd_4libpthread(os_unfair_lock_t lock)
{
	uint32_t mts = MACH_PORT_DEAD;
	os_unfair_lock unlocked = OS_UNFAIR_LOCK_UNLOCKED, locked = { mts };
	if (!OSLOCK_STD(atomic_compare_exchange_strong_explicit)(
			(_os_atomic_unfair_lock*)lock, &unlocked, locked,
			OSLOCK_STD(memory_order_acquire),
			OSLOCK_STD(memory_order_relaxed))) {
		return os_unfair_lock_lock_no_tsd_4libpthread(lock);
	}
}

/*!
 * @function os_unfair_lock_unlock_inline_no_tsd_4libpthread
 *
 * @abstract
 * Unlocks an os_unfair_lock, without requiring valid TSD.
 *
 * This should only be used by libpthread.
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_INLINE OS_ALWAYS_INLINE OS_NONNULL_ALL
void
os_unfair_lock_unlock_inline_no_tsd_4libpthread(os_unfair_lock_t lock)
{
	uint32_t mts = MACH_PORT_DEAD;
	os_unfair_lock unlocked = OS_UNFAIR_LOCK_UNLOCKED, locked = { mts };
	if (!OSLOCK_STD(atomic_compare_exchange_strong_explicit)(
			(_os_atomic_unfair_lock*)lock, &locked, unlocked,
			OSLOCK_STD(memory_order_release),
			OSLOCK_STD(memory_order_relaxed))) {
		return os_unfair_lock_unlock_no_tsd_4libpthread(lock);
	}
}

OS_ASSUME_NONNULL_END

#undef OSLOCK_STD
#ifdef __cplusplus
__END_DECLS
} // extern "C++"
#endif

#endif // OS_UNFAIR_LOCK_INLINE

#endif // __OS_LOCK_PRIVATE__
