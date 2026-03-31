/*
 * Copyright (c) 2007 Apple Inc.  All Rights Reserved.
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

/***********************************************************************
* objc-lock.m
* Error-checking locks for debugging.
**********************************************************************/

#include "objc-private.h"

#if LOCKDEBUG  &&  !TARGET_OS_WIN32

#include <unordered_map>


/***********************************************************************
* Thread-local bool set during _objc_atfork_prepare().
* That function is allowed to break some lock ordering rules.
**********************************************************************/

static tls_key_t fork_prepare_tls;

void
lockdebug_setInForkPrepare(bool inForkPrepare)
{
    INIT_ONCE_PTR(fork_prepare_tls, tls_create(nil), (void)0);
    tls_set(fork_prepare_tls, (void*)inForkPrepare);
}

static bool
inForkPrepare()
{
    INIT_ONCE_PTR(fork_prepare_tls, tls_create(nil), (void)0);
    return (bool)tls_get(fork_prepare_tls);
}



/***********************************************************************
* Lock order graph.
* "lock X precedes lock Y" means that X must be acquired first.
* This property is transitive.
**********************************************************************/

struct lockorder {
    const void *l;
    std::vector<const lockorder *> predecessors;

    mutable std::unordered_map<const lockorder *, bool> memo;

    lockorder(const void *newl) : l(newl) { }
};

static std::unordered_map<const void*, lockorder *> lockOrderList;
// not mutex_t because we don't want lock debugging on this lock
static mutex_tt<false> lockOrderLock;

static bool 
lockPrecedesLock(const lockorder *oldlock, const lockorder *newlock)
{
    auto memoed = newlock->memo.find(oldlock);
    if (memoed != newlock->memo.end()) {
        return memoed->second;
    }

    bool result = false;
    for (const auto *pre : newlock->predecessors) {
        if (oldlock == pre  ||  lockPrecedesLock(oldlock, pre)) {
            result = true;
            break;
        }
    }

    newlock->memo[oldlock] = result;
    return result;
}

static bool 
lockPrecedesLock(const void *oldlock, const void *newlock)
{
    mutex_tt<false>::locker lock(lockOrderLock);

    auto oldorder = lockOrderList.find(oldlock);
    auto neworder = lockOrderList.find(newlock);
    if (neworder == lockOrderList.end() || oldorder == lockOrderList.end()) {
        return false;
    }
    return lockPrecedesLock(oldorder->second, neworder->second);
}

static bool
lockUnorderedWithLock(const void *oldlock, const void *newlock)
{
    mutex_tt<false>::locker lock(lockOrderLock);
    
    auto oldorder = lockOrderList.find(oldlock);
    auto neworder = lockOrderList.find(newlock);
    if (neworder == lockOrderList.end() || oldorder == lockOrderList.end()) {
        return true;
    }
    
    if (lockPrecedesLock(oldorder->second, neworder->second) ||
        lockPrecedesLock(neworder->second, oldorder->second))
    {
        return false;
    }

    return true;
}

void lockdebug_lock_precedes_lock(const void *oldlock, const void *newlock)
{
    if (lockPrecedesLock(newlock, oldlock)) {
        _objc_fatal("contradiction in lock order declaration");
    }

    mutex_tt<false>::locker lock(lockOrderLock);

    auto oldorder = lockOrderList.find(oldlock);
    auto neworder = lockOrderList.find(newlock);
    if (oldorder == lockOrderList.end()) {
        lockOrderList[oldlock] = new lockorder(oldlock);
        oldorder = lockOrderList.find(oldlock);
    }
    if (neworder == lockOrderList.end()) {
        lockOrderList[newlock] = new lockorder(newlock);
        neworder = lockOrderList.find(newlock);
    }

    neworder->second->predecessors.push_back(oldorder->second);
}


/***********************************************************************
* Recording - per-thread list of mutexes and monitors held
**********************************************************************/

enum class lockkind {
    MUTEX = 1, MONITOR = 2, RDLOCK = 3, WRLOCK = 4, RECURSIVE = 5
};

#define MUTEX     lockkind::MUTEX
#define MONITOR   lockkind::MONITOR
#define RDLOCK    lockkind::RDLOCK
#define WRLOCK    lockkind::WRLOCK
#define RECURSIVE lockkind::RECURSIVE

struct lockcount {
    lockkind k;  // the kind of lock it is (MUTEX, MONITOR, etc)
    int i;       // the lock's nest count
};

using objc_lock_list = std::unordered_map<const void *, lockcount>;


// Thread-local list of locks owned by a thread.
// Used by lock ownership checks.
static tls_key_t lock_tls;

// Global list of all locks.
// Used by fork() safety check.
// This can't be a static struct because of C++ initialization order problems.
static objc_lock_list& AllLocks() {
    static objc_lock_list *locks;
    INIT_ONCE_PTR(locks, new objc_lock_list, (void)0);
    return *locks;
}


static void
destroyLocks(void *value)
{
    auto locks = (objc_lock_list *)value;
    // fixme complain about any still-held locks?
    if (locks) delete locks;
}

static objc_lock_list&
ownedLocks()
{
    // Use a dedicated tls key to prevent differences vs non-debug in 
    // usage of objc's other tls keys (required for some unit tests).
    INIT_ONCE_PTR(lock_tls, tls_create(&destroyLocks), (void)0);

    auto locks = (objc_lock_list *)tls_get(lock_tls);
    if (!locks) {
        locks = new objc_lock_list;
        tls_set(lock_tls, locks);
    }

    return *locks;
}

static bool 
hasLock(objc_lock_list& locks, const void *lock, lockkind kind)
{
    auto iter = locks.find(lock);
    if (iter != locks.end() && iter->second.k == kind) return true;
    return false;
}


static const char *sym(const void *lock)
{
    Dl_info info;
    int ok = dladdr(lock, &info);
    if (ok && info.dli_sname && info.dli_sname[0]) return info.dli_sname;
    else return "??";
}

static void 
setLock(objc_lock_list& locks, const void *lock, lockkind kind)
{
    // Check if we already own this lock.
    auto iter = locks.find(lock);
    if (iter != locks.end() && iter->second.k == kind) {
        iter->second.i++;
        return;
    }

    // Newly-acquired lock. Verify lock ordering.
    // Locks not in AllLocks are exempt (i.e. @synchronize locks)
    if (&locks != &AllLocks() && AllLocks().find(lock) != AllLocks().end()) {
        for (auto& oldlock : locks) {
            if (AllLocks().find(oldlock.first) == AllLocks().end()) {
                // oldlock is exempt
                continue;
            }

            if (lockPrecedesLock(lock, oldlock.first)) {
                _objc_fatal("lock %p (%s) incorrectly acquired before %p (%s)",
                            oldlock.first, sym(oldlock.first), lock, sym(lock));
            }
            if (!inForkPrepare() &&
                lockUnorderedWithLock(lock, oldlock.first))
            {
                // _objc_atfork_prepare is allowed to acquire
                // otherwise-unordered locks, but nothing else may.
                _objc_fatal("lock %p (%s) acquired before %p (%s) "
                            "with no defined lock order",
                            oldlock.first, sym(oldlock.first), lock, sym(lock));
            }
        }
    }

    locks[lock] = lockcount{kind, 1};
}

static void 
clearLock(objc_lock_list& locks, const void *lock, lockkind kind)
{
    auto iter = locks.find(lock);
    if (iter != locks.end()) {
        auto& l = iter->second;
        if (l.k == kind) {
            if (--l.i == 0) {
                locks.erase(iter);
            }
            return;
        }
    }

    _objc_fatal("lock not found!");
}


/***********************************************************************
* fork() safety checking
**********************************************************************/

void
lockdebug_remember_mutex(mutex_t *lock)
{
    setLock(AllLocks(), lock, MUTEX);
}

void
lockdebug_remember_recursive_mutex(recursive_mutex_t *lock)
{
    setLock(AllLocks(), lock, RECURSIVE);
}

void
lockdebug_remember_monitor(monitor_t *lock)
{
    setLock(AllLocks(), lock, MONITOR);
}

void
lockdebug_assert_all_locks_locked()
{
    auto& owned = ownedLocks();

    for (const auto& l : AllLocks()) {
        if (!hasLock(owned, l.first, l.second.k)) {
            _objc_fatal("lock %p:%d is incorrectly not owned",
                        l.first, l.second.k);
        }
    }
}

void
lockdebug_assert_no_locks_locked()
{
    auto& owned = ownedLocks();

    for (const auto& l : AllLocks()) {
        if (hasLock(owned, l.first, l.second.k)) {
            _objc_fatal("lock %p:%d is incorrectly owned", l.first, l.second.k);
        }
    }
}


/***********************************************************************
* Mutex checking
**********************************************************************/

void 
lockdebug_mutex_lock(mutex_t *lock)
{
    auto& locks = ownedLocks();
    
    if (hasLock(locks, lock, MUTEX)) {
        _objc_fatal("deadlock: relocking mutex");
    }
    setLock(locks, lock, MUTEX);
}

// try-lock success is the only case with lockdebug effects.
// try-lock when already locked is OK (will fail)
// try-lock failure does nothing.
void 
lockdebug_mutex_try_lock_success(mutex_t *lock)
{
    auto& locks = ownedLocks();
    setLock(locks, lock, MUTEX);
}

void 
lockdebug_mutex_unlock(mutex_t *lock)
{
    auto& locks = ownedLocks();

    if (!hasLock(locks, lock, MUTEX)) {
        _objc_fatal("unlocking unowned mutex");
    }
    clearLock(locks, lock, MUTEX);
}


void 
lockdebug_mutex_assert_locked(mutex_t *lock)
{
    auto& locks = ownedLocks();

    if (!hasLock(locks, lock, MUTEX)) {
        _objc_fatal("mutex incorrectly not locked");
    }
}

void 
lockdebug_mutex_assert_unlocked(mutex_t *lock)
{
    auto& locks = ownedLocks();

    if (hasLock(locks, lock, MUTEX)) {
        _objc_fatal("mutex incorrectly locked");
    }
}


/***********************************************************************
* Recursive mutex checking
**********************************************************************/

void 
lockdebug_recursive_mutex_lock(recursive_mutex_t *lock)
{
    auto& locks = ownedLocks();
    setLock(locks, lock, RECURSIVE);
}

void 
lockdebug_recursive_mutex_unlock(recursive_mutex_t *lock)
{
    auto& locks = ownedLocks();

    if (!hasLock(locks, lock, RECURSIVE)) {
        _objc_fatal("unlocking unowned recursive mutex");
    }
    clearLock(locks, lock, RECURSIVE);
}


void 
lockdebug_recursive_mutex_assert_locked(recursive_mutex_t *lock)
{
    auto& locks = ownedLocks();

    if (!hasLock(locks, lock, RECURSIVE)) {
        _objc_fatal("recursive mutex incorrectly not locked");
    }
}

void 
lockdebug_recursive_mutex_assert_unlocked(recursive_mutex_t *lock)
{
    auto& locks = ownedLocks();

    if (hasLock(locks, lock, RECURSIVE)) {
        _objc_fatal("recursive mutex incorrectly locked");
    }
}


/***********************************************************************
* Monitor checking
**********************************************************************/

void 
lockdebug_monitor_enter(monitor_t *lock)
{
    auto& locks = ownedLocks();

    if (hasLock(locks, lock, MONITOR)) {
        _objc_fatal("deadlock: relocking monitor");
    }
    setLock(locks, lock, MONITOR);
}

void 
lockdebug_monitor_leave(monitor_t *lock)
{
    auto& locks = ownedLocks();

    if (!hasLock(locks, lock, MONITOR)) {
        _objc_fatal("unlocking unowned monitor");
    }
    clearLock(locks, lock, MONITOR);
}

void 
lockdebug_monitor_wait(monitor_t *lock)
{
    auto& locks = ownedLocks();

    if (!hasLock(locks, lock, MONITOR)) {
        _objc_fatal("waiting in unowned monitor");
    }
}


void 
lockdebug_monitor_assert_locked(monitor_t *lock)
{
    auto& locks = ownedLocks();

    if (!hasLock(locks, lock, MONITOR)) {
        _objc_fatal("monitor incorrectly not locked");
    }
}

void 
lockdebug_monitor_assert_unlocked(monitor_t *lock)
{
    auto& locks = ownedLocks();

    if (hasLock(locks, lock, MONITOR)) {
        _objc_fatal("monitor incorrectly held");
    }
}

#endif
