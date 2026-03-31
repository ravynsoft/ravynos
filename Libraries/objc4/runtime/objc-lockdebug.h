/*
 * Copyright (c) 2015 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#if LOCKDEBUG
extern void lockdebug_assert_all_locks_locked();
extern void lockdebug_assert_no_locks_locked();
extern void lockdebug_setInForkPrepare(bool);
extern void lockdebug_lock_precedes_lock(const void *oldlock, const void *newlock);
#else
static constexpr inline void lockdebug_assert_all_locks_locked() { }
static constexpr inline void lockdebug_assert_no_locks_locked() { }
static constexpr inline void lockdebug_setInForkPrepare(bool) { }
static constexpr inline void lockdebug_lock_precedes_lock(const void *, const void *) { }
#endif

extern void lockdebug_remember_mutex(mutex_tt<true> *lock);
extern void lockdebug_mutex_lock(mutex_tt<true> *lock);
extern void lockdebug_mutex_try_lock(mutex_tt<true> *lock);
extern void lockdebug_mutex_unlock(mutex_tt<true> *lock);
extern void lockdebug_mutex_assert_locked(mutex_tt<true> *lock);
extern void lockdebug_mutex_assert_unlocked(mutex_tt<true> *lock);

static constexpr inline void lockdebug_remember_mutex(mutex_tt<false> *lock) { }
static constexpr inline void lockdebug_mutex_lock(mutex_tt<false> *lock) { }
static constexpr inline void lockdebug_mutex_try_lock(mutex_tt<false> *lock) { }
static constexpr inline void lockdebug_mutex_unlock(mutex_tt<false> *lock) { }
static constexpr inline void lockdebug_mutex_assert_locked(mutex_tt<false> *lock) { }
static constexpr inline void lockdebug_mutex_assert_unlocked(mutex_tt<false> *lock) { }


extern void lockdebug_remember_monitor(monitor_tt<true> *lock);
extern void lockdebug_monitor_enter(monitor_tt<true> *lock);
extern void lockdebug_monitor_leave(monitor_tt<true> *lock);
extern void lockdebug_monitor_wait(monitor_tt<true> *lock);
extern void lockdebug_monitor_assert_locked(monitor_tt<true> *lock);
extern void lockdebug_monitor_assert_unlocked(monitor_tt<true> *lock);

static constexpr inline void lockdebug_remember_monitor(monitor_tt<false> *lock) { }
static constexpr inline void lockdebug_monitor_enter(monitor_tt<false> *lock) { }
static constexpr inline void lockdebug_monitor_leave(monitor_tt<false> *lock) { }
static constexpr inline void lockdebug_monitor_wait(monitor_tt<false> *lock) { }
static constexpr inline void lockdebug_monitor_assert_locked(monitor_tt<false> *lock) { }
static constexpr inline void lockdebug_monitor_assert_unlocked(monitor_tt<false> *lock) {}


extern void 
lockdebug_remember_recursive_mutex(recursive_mutex_tt<true> *lock);
extern void 
lockdebug_recursive_mutex_lock(recursive_mutex_tt<true> *lock);
extern void 
lockdebug_recursive_mutex_unlock(recursive_mutex_tt<true> *lock);
extern void 
lockdebug_recursive_mutex_assert_locked(recursive_mutex_tt<true> *lock);
extern void 
lockdebug_recursive_mutex_assert_unlocked(recursive_mutex_tt<true> *lock);

static constexpr inline void
lockdebug_remember_recursive_mutex(recursive_mutex_tt<false> *lock) { }
static constexpr inline void
lockdebug_recursive_mutex_lock(recursive_mutex_tt<false> *lock) { }
static constexpr inline void
lockdebug_recursive_mutex_unlock(recursive_mutex_tt<false> *lock) { }
static constexpr inline void
lockdebug_recursive_mutex_assert_locked(recursive_mutex_tt<false> *lock) { }
static constexpr inline void
lockdebug_recursive_mutex_assert_unlocked(recursive_mutex_tt<false> *lock) { }
