/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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

#include "DSSemaphore.h"
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <libkern/OSAtomic.h>
#include <unistd.h>		// for _POSIX_THREADS

#if defined(DEBUG_LOCKS_HISTORY) || defined(DEBUG_LOCKS)
struct sLockHistoryInfo
{
	OSSpinLock		fOSLock;
	pthread_key_t	fThreadKey;
	const char      *fFile;
	int32_t			fLine;
};
#endif

DSSemaphore::DSSemaphore( const char *inName )
{
	pthread_mutexattr_t attr;

	mMutexName = (inName != NULL ? inName : "no name provided");
	mLockHistoryInfo = NULL;

	pthread_mutexattr_init( &attr );
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
	
	int error = pthread_mutex_init( &mMutex, &attr );
	if ( error != 0 )
	{
		syslog( LOG_CRIT, "Error %d - Setting mutex thread type to PTHREAD_MUTEX_ERRORCHECK - Thread %p - aborting", error, pthread_self() );
		abort(); // we can't recover from this
	}
	
	pthread_mutexattr_destroy( &attr);
	
#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
	mLockHistoryInfo = new sLockHistoryInfo;
	mLockHistoryInfo->fOSLock = OS_SPINLOCK_INIT;
	pthread_key_create( &mLockHistoryInfo->fThreadKey, LockCleanup );
	mLockHistoryInfo->fFile = "";
	mLockHistoryInfo->fLine = 0;
#endif
}

DSSemaphore::~DSSemaphore( void )
{
#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
	if ( mLockHistoryInfo )
	{
		pthread_setspecific( mLockHistoryInfo->fThreadKey, NULL );
		pthread_key_delete( mLockHistoryInfo->fThreadKey );
		delete mLockHistoryInfo;
	}
#endif
	pthread_mutex_destroy( &mMutex );
}

void DSSemaphore::WaitDebug( const char *file, int line )
{
	int error = pthread_mutex_lock( &mMutex );
	if ( error != 0 )
	{
#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
		DSMutexSemaphore::BreakIfDebugging();
		syslog( LOG_CRIT, "DSSemaphore::WaitDebug - %s - error %d in %s:%d attempting to lock mutex - aborting", mMutexName, error, file, line );		
#else
		syslog( LOG_CRIT, "DSSemaphore::Wait - %s - error %d attempting to lock mutex - aborting", mMutexName, error );
#endif
		abort();
	}

#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
	pthread_setspecific( mLockHistoryInfo->fThreadKey, this );
#endif
}

bool DSSemaphore::WaitTryDebug( const char *file, int line )
{
	int error = pthread_mutex_trylock( &mMutex );
	if ( error == 0 )
	{
#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
		pthread_setspecific( mLockHistoryInfo->fThreadKey, this );
		mLockHistoryInfo->fFile = file;
		mLockHistoryInfo->fLine = line;
#endif
	}
	
	return (error == 0);
}

void DSSemaphore::SignalDebug( const char *file, int line )
{
#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
	OSSpinLockLock( &mLockHistoryInfo->fOSLock );
	int error = pthread_mutex_unlock( &mMutex );
	if ( error == 0 )
	{
		pthread_setspecific( mLockHistoryInfo->fThreadKey, NULL );
		mLockHistoryInfo->fFile = "";
		mLockHistoryInfo->fLine = 0;
	}
	else
	{
		DSMutexSemaphore::BreakIfDebugging();
		syslog( LOG_CRIT, "DSSemaphore::SignalDebug - %s - attempt to unlock not owned by %s:%d", mMutexName, file, line );
	}
	OSSpinLockUnlock( &mLockHistoryInfo->fOSLock );
#else
	int error = pthread_mutex_unlock( &mMutex );
	if ( error != 0 )
	{
		syslog( LOG_CRIT, "DSSemaphore::Signal - %s - error %d attempting to unlock mutex - aborting", mMutexName, error );
		abort();
	}
#endif
}

void DSSemaphore::LockCleanup( void *value )
{
	if ( value != NULL )
	{
		syslog( LOG_CRIT, "DSSemaphore::LockCleanup was called, this should never happen unless a lock was still held by thread");
#if defined(DEBUG_LOCKS_HISTORY) || defined(DEBUG_LOCKS)
		DSSemaphore *mutex = (DSSemaphore *) value;
		if ( mutex->mLockHistoryInfo != NULL )
			syslog( LOG_CRIT, "DSSemaphore::LockCleanup - %s - last owner %s:%d", mutex->mMutexName, mutex->mLockHistoryInfo->fFile, 
				    mutex->mLockHistoryInfo->fLine );
#endif
	}
}
