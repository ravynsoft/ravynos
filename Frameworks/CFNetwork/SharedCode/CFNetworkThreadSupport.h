/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
/*
 *  CFNetworkThreadSupport.h
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on 11/4/04.
 *  Copyright 2004 Apple Computer, Inc. All rights reserved.
 *
 */
#ifndef __CFNETWORKTHREADSUPPORT__
#define __CFNETWORKTHREADSUPPORT__

#include <CoreFoundation/CoreFoundation.h>		/* for CF_INLINE */


#if defined(__cplusplus)
extern "C" {
#endif
	
#if defined(__MACH__)

#include <libkern/OSAtomic.h>
#include <pthread.h>					// For spin_lock stuff below
#include <unistd.h>
	
typedef OSSpinLock CFSpinLock_t;

#if defined(__i386__)
extern void _spin_lock(CFSpinLock_t *lockp);
extern void _spin_unlock(CFSpinLock_t *lockp);
#define OSSpinLockLock(p) _spin_lock(p)
#define OSSpinLockUnlock(p) _spin_unlock(p)
#endif

extern int _spin_lock_try(CFSpinLock_t *lockp);
extern void _spin_unlock(CFSpinLock_t *lockp);
extern int nanosleep(const struct timespec *, struct timespec *);

CF_INLINE Boolean __CFSpinTryLock(CFSpinLock_t *lockp) {
	return(OSSpinLockTry(lockp));
}

CF_INLINE void __CFSpinLock(CFSpinLock_t *lockp) {
	OSSpinLockLock(lockp);
}

CF_INLINE void __CFSpinUnlock(CFSpinLock_t *lockp) {
	OSSpinLockUnlock(lockp);
}

#elif defined(__WIN32__)

typedef LONG CFSpinLock_t;

CF_INLINE Boolean __CFSpinTryLock(CFSpinLock_t *slock) {
    return InterlockedExchange(slock, 1) == 0;
}

CF_INLINE void __CFSpinLock(CFSpinLock_t *slock) {
    while (InterlockedExchange(slock, 1) != 0) {
        Sleep(1);   // 1ms
    }
}

CF_INLINE void __CFSpinUnlock(CFSpinLock_t *lock) {
    *lock = 0;
}

#else

#warning CF spin locks not defined for this platform -- CF is not thread-safe
#define __CFSpinLock(A)		do {} while (0)
#define __CFSpinUnlock(A)	do {} while (0)

#endif

/*
 * Insulation layer over mutex locks and simple threads
 */

#if !defined(__WIN32__)

typedef pthread_mutex_t _CFMutex;

CF_INLINE void _CFMutexLock(_CFMutex *lock)   {
    pthread_mutex_lock(lock);
}

CF_INLINE Boolean _CFMutexTryLock(_CFMutex *lock)   {
    return pthread_mutex_trylock(lock) == 0;
}

CF_INLINE void _CFMutexUnlock(_CFMutex *lock) {
    pthread_mutex_unlock(lock);
}

extern void _CFMutexInit(_CFMutex *lock, Boolean recursive);

CF_INLINE void _CFMutexDestroy(_CFMutex *lock) {
    pthread_mutex_destroy(lock);
}

typedef pthread_once_t _CFOnceLock;
#define _CFOnceInitializer PTHREAD_ONCE_INIT

CF_INLINE void _CFDoOnce(_CFOnceLock *lock, void (*func)(void)) {
    pthread_once(lock, func);
}

typedef pthread_t _CFThread;

CF_INLINE int _CFThreadSpawn(_CFThread *thread, void *(*func)(void *), void *arg) {
    return pthread_create(thread, NULL, func, arg);
}

#else   // __WIN32__

typedef CRITICAL_SECTION _CFMutex;

CF_INLINE void _CFMutexLock(_CFMutex *lock)   {
    EnterCriticalSection(lock);
}

CF_INLINE Boolean _CFMutexTryLock(_CFMutex *lock)   {
    return TryEnterCriticalSection(lock) != 0;
}

CF_INLINE void _CFMutexUnlock(_CFMutex *lock) {
    LeaveCriticalSection(lock);
}

CF_INLINE void _CFMutexInit(_CFMutex *lock, Boolean recursive) {
    // critical sections are always recursive on Win32
    InitializeCriticalSection(lock);
}

CF_INLINE void _CFMutexDestroy(_CFMutex *lock) {
    DeleteCriticalSection(lock);
}

typedef struct {
    CFSpinLock_t lock;
    UInt32 state;
} _CFOnceLock;

#define _CFOnceInitializer { 0, 0 }

CF_INLINE void _CFDoOnce(_CFOnceLock *once, void (*func)(void)) {
    __CFSpinLock(&once->lock);
    if (once->state == 0) {
        (*func)();
        once->state = 1;
    }
    __CFSpinUnlock(&once->lock);
}

typedef HANDLE _CFThread;

extern int _CFThreadSpawn(_CFThread *thread, void *(*func)(void *), void *arg);

#endif  // __WIN32__


#if defined(__cplusplus)
}
#endif


#endif	/* __CFNETWORKTHREADSUPPORT__ */
