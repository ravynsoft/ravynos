/*
 * Copyright 2020 Lag Free Games, LLC
 * Copyright 2022 Yonggang Luo
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include "rwlock.h"

#if defined(_WIN32) && !defined(HAVE_PTHREAD)
#include <windows.h>
static_assert(sizeof(struct u_rwlock) == sizeof(SRWLOCK),
   "struct u_rwlock should have equal size with SRWLOCK");
#endif

int u_rwlock_init(struct u_rwlock *rwlock)
{
#if defined(_WIN32) && !defined(HAVE_PTHREAD)
   InitializeSRWLock((PSRWLOCK)(&rwlock->rwlock));
   return 0;
#else
   return pthread_rwlock_init(&rwlock->rwlock, NULL);
#endif
}

int u_rwlock_destroy(struct u_rwlock *rwlock)
{
#if defined(_WIN32) && !defined(HAVE_PTHREAD)
   return 0;
#else
   return pthread_rwlock_destroy(&rwlock->rwlock);
#endif
}

int u_rwlock_rdlock(struct u_rwlock *rwlock)
{
#if defined(_WIN32) && !defined(HAVE_PTHREAD)
   AcquireSRWLockShared((PSRWLOCK)&rwlock->rwlock);
   return 0;
#else
   return pthread_rwlock_rdlock(&rwlock->rwlock);
#endif
}

int u_rwlock_rdunlock(struct u_rwlock *rwlock)
{
#if defined(_WIN32) && !defined(HAVE_PTHREAD)
   ReleaseSRWLockShared((PSRWLOCK)&rwlock->rwlock);
   return 0;
#else
   return pthread_rwlock_unlock(&rwlock->rwlock);
#endif
}

int u_rwlock_wrlock(struct u_rwlock *rwlock)
{
#if defined(_WIN32) && !defined(HAVE_PTHREAD)
   AcquireSRWLockExclusive((PSRWLOCK)&rwlock->rwlock);
   return 0;
#else
   return pthread_rwlock_wrlock(&rwlock->rwlock);
#endif
}

int u_rwlock_wrunlock(struct u_rwlock *rwlock)
{
#if defined(_WIN32) && !defined(HAVE_PTHREAD)
   ReleaseSRWLockExclusive((PSRWLOCK)&rwlock->rwlock);
   return 0;
#else
   return pthread_rwlock_unlock(&rwlock->rwlock);
#endif
}
