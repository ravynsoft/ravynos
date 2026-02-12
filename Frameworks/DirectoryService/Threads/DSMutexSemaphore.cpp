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

/*!
 * @header DSMutexSemaphore
 * Implementation of the DSMutexSemaphore ( mutually exclusive lock ) class.
 */

#include "DSMutexSemaphore.h"
#include <libkern/OSAtomic.h>
#include <machine/types.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>		// for _POSIX_THREADS
#include "dslockstat.h"

// Uncomment the following line in addition to enabling DEBUG_LOCKS and we will use try lock to acquire a lock and spin
// until we do.  After 60 seconds we will log what locks we are waiting on
//#define DEBUG_LOCKS_WAITING

#if defined(DEBUG_LOCKS_HISTORY)
	#define threadHistoryStringLen 256
	#define threadHistoryCount 16
#elif defined(DEBUG_LOCKS)
	#define threadHistoryStringLen 256
	#define threadHistoryCount 1
#endif

struct sLockHistoryInfo
{
	bool			fShouldDTrace;
#if defined(DEBUG_LOCKS_HISTORY) || defined(DEBUG_LOCKS)
	OSSpinLock		fOSLock;
	pthread_key_t	fThreadKey;
	int				fThreadOwnerCount;
	char			*fCurrHolder;
	char			fLastHoldersBuffer[threadHistoryCount*threadHistoryStringLen];
#endif
#if defined(DEBUG_LOCKS_HISTORY)
	char			*fLockHistory[threadHistoryCount];
	int				fCurrHolderIndex;
	pthread_t		fThreadOwner;
	time_t			fLockTime;
	time_t			fLockHoldTime[threadHistoryCount];		
#endif
};

#if defined(DEBUG_LOCKS_WAITING) && !defined(DEBUG_LOCKS)
	#error cant define DEBUG_LOCKS_WAITING without DEBUG_LOCKS
#endif

#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY) || defined(DEBUG_LOCKS_WAITING)
	#warning DEBUG_LOCKS, DEBUG_LOCKS_WAITING or DEBUG_LOCKS_HISTORY is enabled, ensure it is disabled before GM
#endif

//--------------------------------------------------------------------------------------------------
//	DSMutexSemaphore class implementation
//--------------------------------------------------------------------------------------------------

DSMutexSemaphore::DSMutexSemaphore ( const char *inName, bool bShouldDTrace )
{
    int error;
    pthread_mutexattr_t mutexType;
    
    pthread_mutexattr_init( &mutexType );
    error = pthread_mutexattr_settype( &mutexType, PTHREAD_MUTEX_RECURSIVE );
    if( error )
    {
        syslog( LOG_CRIT, "Error %d - Setting mutex thread type to PTHREAD_MUTEX_RECURSIVE - Thread %p", error, pthread_self() );
		abort(); // we can't recover from this
    }
	
    error = pthread_mutex_init( &mMutex, &mutexType );
    if( error )
    {
        syslog( LOG_CRIT, "Error %d - Initializing mutex - Thread = %p", error, pthread_self() );
		abort(); // we can't recover from this
    }
    pthread_mutexattr_destroy( &mutexType );

	mMutexName = (inName != NULL ? inName : "no name provided");
	mLockHistoryInfo = new sLockHistoryInfo;
	mLockHistoryInfo->fShouldDTrace = bShouldDTrace;
	
#if defined(DEBUG_LOCKS_HISTORY) || defined(DEBUG_LOCKS)
	mLockHistoryInfo->fOSLock = OS_SPINLOCK_INIT;
	pthread_key_create( &mLockHistoryInfo->fThreadKey, LockCleanup );
	mLockHistoryInfo->fThreadOwnerCount = 0;
	mLockHistoryInfo->fCurrHolder = mLockHistoryInfo->fLastHoldersBuffer;
	mLockHistoryInfo->fLastHoldersBuffer[0] = '\0';
#endif

#if defined(DEBUG_LOCKS_HISTORY)
	for( int ii = 0; ii < threadHistoryCount; ii++ )
	{
		mLockHistoryInfo->fLockHistory[ii] = &mLockHistoryInfo->fLastHoldersBuffer[ii*threadHistoryStringLen];
		mLockHistoryInfo->fLockHistory[ii][0] = '\0';
		mLockHistoryInfo->fLockHoldTime[ii] = 0;
	}

	mLockHistoryInfo->fCurrHolderIndex = 0;
	mLockHistoryInfo->fThreadOwner = NULL;
	mLockHistoryInfo->fLockTime = 0;
#endif
}

DSMutexSemaphore::~DSMutexSemaphore( void )
{
#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
	pthread_setspecific( mLockHistoryInfo->fThreadKey, NULL );
	pthread_key_delete( mLockHistoryInfo->fThreadKey );
#endif
	if ( mLockHistoryInfo != NULL )
	{
		delete mLockHistoryInfo;
		mLockHistoryInfo = NULL;
	}
	
	pthread_mutex_destroy( &mMutex );
}

#pragma mark -
#pragma mark Debug Mutex code when not inlined

void DSMutexSemaphore::Signal( void )
{
	int error = pthread_mutex_unlock( &mMutex );
	if ( error != 0 )
	{
#ifdef __LP64__
		syslog( LOG_CRIT, "DSMutexSemaphore::Signal failed error %d for mutex 0x%016lX", error, (unsigned long) &mMutex );
#else
		syslog( LOG_CRIT, "DSMutexSemaphore::Signal failed error %d for mutex 0x%08lX", error, (unsigned long) &mMutex );
#endif
		
#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
		BreakIfDebugging();
#endif
	}
	else
	{
		if ( mLockHistoryInfo->fShouldDTrace )
			DSLOCKSTAT_MUTEX_RELEASE( (long) &mMutex, mMutexName, "no file info", 0 );
	}
}

void DSMutexSemaphore::Wait( void )
{
	int error = pthread_mutex_lock( &mMutex );
	if ( error != 0 )
	{
#ifdef __LP64__
		syslog( LOG_CRIT, "DSMutexSemaphore::Wait failed error %d for mutex 0x%016lX", error, (unsigned long) &mMutex );
#else
		syslog( LOG_CRIT, "DSMutexSemaphore::Wait failed error %d for mutex 0x%08lX", error, (unsigned long) &mMutex );
#endif
		
#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
		BreakIfDebugging();
#endif
		abort(); // we don't expect this to fail
	}
}

bool DSMutexSemaphore::WaitTry( void )
{
	int error = pthread_mutex_trylock( &mMutex );
	if ( error != EBUSY && error != 0 )
	{
#ifdef __LP64__
		syslog( LOG_CRIT, "DSMutexSemaphore::Wait failed error %d for mutex 0x%016lX", error, (unsigned long) &mMutex );
#else
		syslog( LOG_CRIT, "DSMutexSemaphore::Wait failed error %d for mutex 0x%08lX", error, (unsigned long) &mMutex );
#endif
		
#if defined(DEBUG_LOCKS) || defined(DEBUG_LOCKS_HISTORY)
		BreakIfDebugging();
#endif
	}
	
	return (error == 0);
}

#pragma mark -
#pragma mark Debug mutex support stuff

// Returns true if the current process is being debugged (either
// running under the debugger or has a debugger attached post facto).
bool DSMutexSemaphore::BeingDebugged( void )
{
    int                 mib[4];
    struct kinfo_proc   info;
    size_t              size;
	
    info.kp_proc.p_flag = 0;
	
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
	
    size = sizeof(info);
    sysctl(mib, (u_int) (sizeof(mib) / sizeof(*mib)), &info, &size, NULL, 0);
	
    // We're being debugged if the P_TRACED flag is set.
    return ( (info.kp_proc.p_flag & P_TRACED) != 0 );
}

// For non-fatal conditions, if we're debugging break in such a way that we can continue.
// inline this routine so that we don't add additional call stacks.
inline void DSMutexSemaphore::BreakIfDebugging( void )
{
	if ( BeingDebugged() == true )
	{
#if defined(__ppc__)
		asm("trap");
#elif defined(__i386__)
		asm("int3");
#endif
	}
	else
	{
		syslog( LOG_CRIT, "If you run under the debugger, it will automatically break here.");
	}
}

void DSMutexSemaphore::LockCleanup( void *value )
{
	syslog( LOG_CRIT, "DSMutexSemaphore::LockCleanup was called, this should never happen unless a lock was still held by thread");

	if ( value != NULL )
	{
#if defined(DEBUG_LOCKS_HISTORY)
		DSMutexSemaphore *mutex = (DSMutexSemaphore *) value;
		OSSpinLockLock( &mutex->mLockHistoryInfo->fOSLock );
		DumpLockHistory( mutex );
		OSSpinLockUnlock( &mutex->mLockHistoryInfo->fOSLock );
#elif defined(DEBUG_LOCKS)
		DSMutexSemaphore *mutex = (DSMutexSemaphore *) value;
		OSSpinLockLock( &mutex->mLockHistoryInfo->fOSLock );
		DumpLockDebug( mutex );
		OSSpinLockUnlock( &mutex->mLockHistoryInfo->fOSLock );
#endif
	}
}

#pragma mark -
#pragma mark Debug Mutexes

void DSMutexSemaphore::SignalDebug( const char *file, int line )
{
	const char *shortName = NULL;
	
	// we clear the owner if it us
#if defined(DEBUG_LOCKS)
	OSSpinLockLock( &mLockHistoryInfo->fOSLock );
#endif

    int error = pthread_mutex_unlock( &mMutex );
	if ( error == 0 )
	{
		if ( DSLOCKSTAT_MUTEX_RELEASE_ENABLED() && mLockHistoryInfo->fShouldDTrace )
		{
			shortName = rindex( file, '/' );
			if ( shortName == NULL )
				shortName = file;
			else 
				shortName++;
			DSLOCKSTAT_MUTEX_RELEASE( (long) &mMutex, (char *) mMutexName, shortName, line );
		}
		
#if defined(DEBUG_LOCKS)
		if ( (--mLockHistoryInfo->fThreadOwnerCount) == 0 )
		{
			mLockHistoryInfo->fCurrHolder[0] = '\0';
			pthread_setspecific( mLockHistoryInfo->fThreadKey, NULL );
		}
#endif
	}
	else
	{
		shortName = strrchr( file, '/' );
		if ( shortName == NULL )
			shortName = file;
		else 
			shortName++;

#ifdef __LP64__
		syslog( LOG_CRIT, "DSMutexSemaphore::SignalDebug - Error %d for mutex 0x%016lX - Caller %s:%d", error, (unsigned long) &mMutex, shortName, line );
#else
		syslog( LOG_CRIT, "DSMutexSemaphore::SignalDebug - Error %d for mutex 0x%08lX - Caller %s:%d", error, (unsigned long) &mMutex, shortName, line );
#endif

		BreakIfDebugging();
	}
	
#if defined(DEBUG_LOCKS)
	OSSpinLockUnlock( &mLockHistoryInfo->fOSLock );
#endif
}

void DSMutexSemaphore::WaitDebug( const char *file, int line )
{
	const char *shortName = NULL;
	
#if defined(DEBUG_LOCKS_WAITING) && defined(DEBUG_LOCKS)
	int		error;
	time_t	lockAttempt = time(NULL);
	bool	bLogged		= false;
	while ( (error = pthread_mutex_trylock(&mMutex)) == EBUSY )
	{
		if ( bLogged == false && time(NULL) - lockAttempt > 60 )
		{
			shortName = strrchr( file, '/' );
			if ( shortName == NULL )
				shortName = file;
			else 
				shortName++;
			
			OSSpinLockLock( &mLockHistoryInfo->fOSLock );
			pthread_t thisThread = pthread_self();

#ifdef __LP64__
			syslog( LOG_CRIT, "Caller 0x%016lX (0x%x) - %s:%d - has been waiting > 60 sec for lock acquisition", 
				    (unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#else
			syslog( LOG_CRIT, "Caller 0x%08lX (0x%x) - %s:%d - has been waiting > 60 sec for lock acquisition", 
				    (unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#endif
			DumpLockDebug( this );
			OSSpinLockUnlock( &mLockHistoryInfo->fOSLock );
			bLogged = true;
		}
	}
#else
	int error = pthread_mutex_lock( &mMutex );
#endif

	if ( error == 0 )
	{
		// we have our own flag so we don't dtrace certain locks
		if ( DSLOCKSTAT_MUTEX_ACQUIRE_ENABLED() && mLockHistoryInfo->fShouldDTrace )
		{
			shortName = rindex( file, '/' );
			if ( shortName == NULL )
				shortName = file;
			else 
				shortName++;
			DSLOCKSTAT_MUTEX_ACQUIRE( (long) &mMutex, (char *) mMutexName, shortName, line );
		}

#if defined(DEBUG_LOCKS)
		OSSpinLockLock( &mLockHistoryInfo->fOSLock );

		shortName = strrchr( file, '/' );
		if ( shortName == NULL )
			shortName = file;
		else 
			shortName++;
		
		pthread_t thisThread = pthread_self();
		
		mLockHistoryInfo->fThreadOwnerCount++;
#ifdef __LP64__
		snprintf( mLockHistoryInfo->fCurrHolder, threadHistoryStringLen, "Lock last held - 0x%016lX (0x%x) - Count %d - %s:%d", 
				  (unsigned long) thisThread, pthread_mach_thread_np(thisThread), mLockHistoryInfo->fThreadOwnerCount, shortName, line );
#else
		snprintf( mLockHistoryInfo->fCurrHolder, threadHistoryStringLen, "Lock last held - 0x%08lX (0x%x) - Count %d - %s:%d", 
				  (unsigned long) thisThread, pthread_mach_thread_np(thisThread), mLockHistoryInfo->fThreadOwnerCount, shortName, line );
#endif
		
		pthread_setspecific( mLockHistoryInfo->fThreadKey, this );
		OSSpinLockUnlock( &mLockHistoryInfo->fOSLock );
#endif
	}
	else
	{
		shortName = strrchr( file, '/' );
		if ( shortName == NULL )
			shortName = file;
		else 
			shortName++;
		
#ifdef __LP64__
		syslog( LOG_CRIT, "DSMutexSemaphore::WaitDebug - Error %d for mutex 0x%016lX - Caller %s:%d", error, (unsigned long) &mMutex, shortName, line );
#else
		syslog( LOG_CRIT, "DSMutexSemaphore::WaitDebug - Error %d for mutex 0x%08lX - Caller %s:%d", error, (unsigned long) &mMutex, shortName, line );
#endif
		BreakIfDebugging();
	}
}

bool DSMutexSemaphore::WaitTryDebug( const char *file, int line )
{
	const char *shortName = NULL;
	int error = pthread_mutex_trylock( &mMutex );
	if ( error == 0 )
	{
		if ( DSLOCKSTAT_MUTEX_ACQUIRE_ENABLED() && mLockHistoryInfo->fShouldDTrace )
		{
			shortName = rindex( file, '/' );
			if ( shortName == NULL )
				shortName = file;
			else 
				shortName++;
			DSLOCKSTAT_MUTEX_ACQUIRE( (long) &mMutex, (char *) mMutexName, shortName, line );
		}
		
#if defined(DEBUG_LOCKS)
		OSSpinLockLock( &mLockHistoryInfo->fOSLock );
		
		shortName = strrchr( file, '/' );
		if ( shortName == NULL )
			shortName = file;
		else 
			shortName++;
		
		pthread_t thisThread = pthread_self();
		
		mLockHistoryInfo->fThreadOwnerCount++;
#ifdef __LP64__
		snprintf( mLockHistoryInfo->fCurrHolder, threadHistoryStringLen, "Lock last held - 0x%016lX (0x%x) - Count %d - %s:%d", 
				  (unsigned long) thisThread, pthread_mach_thread_np(thisThread), mLockHistoryInfo->fThreadOwnerCount, shortName, line );
#else
		snprintf( mLockHistoryInfo->fCurrHolder, threadHistoryStringLen, "Lock last held - 0x%08lX (0x%x) - Count %d - %s:%d", 
				  (unsigned long) thisThread, pthread_mach_thread_np(thisThread), mLockHistoryInfo->fThreadOwnerCount, shortName, line );
#endif
		
		pthread_setspecific( mLockHistoryInfo->fThreadKey, this );
		OSSpinLockUnlock( &mLockHistoryInfo->fOSLock );
#endif
	}
	else if ( error != EBUSY )
	{
		shortName = strrchr( file, '/' );
		if ( shortName == NULL )
			shortName = file;
		else 
			shortName++;

#ifdef __LP64__
		syslog( LOG_CRIT, "DSMutexSemaphore::WaitTryDebug - Error %d for mutex 0x%016lX - Caller %s:%d", error, (unsigned long) &mMutex, shortName, line );
#else
		syslog( LOG_CRIT, "DSMutexSemaphore::WaitTryDebug - Error %d for mutex 0x%08lX - Caller %s:%d", error, (unsigned long) &mMutex, shortName, line );
#endif
		BreakIfDebugging();
	}
	
	return (error == 0);
}

void DSMutexSemaphore::DumpLockDebug( DSMutexSemaphore *inMutex )
{
#if defined(DEBUG_LOCKS)
	static OSSpinLock	localLock	= OS_SPINLOCK_INIT;
	
	OSSpinLockLock( &localLock );
	
	syslog( LOG_CRIT, "Lock - %s", inMutex->mMutexName );
	syslog( LOG_CRIT, "    %s", inMutex->mLockHistoryInfo->fCurrHolder );
	
	OSSpinLockUnlock( &localLock );
#endif
}

#pragma mark -
#pragma mark Debug History Mutexes

#if defined(DEBUG_LOCKS_HISTORY)

void DSMutexSemaphore::SignalDebugHistory( const char *file, int line )
{
	const char *shortName = NULL;
	
	OSSpinLockLock( &mLockHistoryInfo->fOSLock );
	
	// we clear the owner if it us
    int error = pthread_mutex_unlock( &mMutex );

	shortName = strrchr( file, '/' );
	if ( shortName == NULL )
		shortName = file;
	else 
		shortName++;
	
	if ( error == 0 )
	{
		mLockHistoryInfo->fLockHoldTime[mLockHistoryInfo->fCurrHolderIndex] = time(NULL) - mLockHistoryInfo->fLockTime;
		mLockHistoryInfo->fCurrHolderIndex = (mLockHistoryInfo->fCurrHolderIndex + 1) % threadHistoryCount;
#ifdef __LP64__
		snprintf( mLockHistoryInfo->fLockHistory[mLockHistoryInfo->fCurrHolderIndex], threadHistoryStringLen, "Succeed - Unlock Depth %d - 0x%016lX (0x%x) - %s:%d", (mLockHistoryInfo->fThreadOwnerCount - 1),
				 (unsigned long) mLockHistoryInfo->fThreadOwner, pthread_mach_thread_np(mLockHistoryInfo->fThreadOwner), shortName, line );
#else
		snprintf( mLockHistoryInfo->fLockHistory[mLockHistoryInfo->fCurrHolderIndex], threadHistoryStringLen, "Succeed - Unlock Depth %d - 0x%08lX (0x%x) - %s:%d", (mLockHistoryInfo->fThreadOwnerCount - 1),
				  (unsigned long) mLockHistoryInfo->fThreadOwner, pthread_mach_thread_np(mLockHistoryInfo->fThreadOwner), shortName, line );
#endif
		mLockHistoryInfo->fCurrHolder = mLockHistoryInfo->fLockHistory[mLockHistoryInfo->fCurrHolderIndex];

		if( mLockHistoryInfo->fThreadOwnerCount > 0 )
		{
			if ( (--mLockHistoryInfo->fThreadOwnerCount) == 0 )
			{
				mLockHistoryInfo->fThreadOwner = NULL;
				pthread_setspecific( mLockHistoryInfo->fThreadKey, NULL );
			}
		}
		else
		{
			pthread_t thisThread = pthread_self();
#ifdef __LP64__
			syslog( LOG_CRIT, "Error %d - Unlocking mutex 0x%016lX - Thread 0x%016lX (0x%x) - Count would go negative count - Caller '%s:%d'", error, 
				    (unsigned long) &mMutex, (unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#else
			syslog( LOG_CRIT, "Error %d - Unlocking mutex 0x%08lX - Thread 0x%08lX (0x%x) - Count would go negative count - Caller '%s:%d'", error, 
				    (unsigned long) &mMutex, (unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#endif
			DumpLockHistory( this );
		}
	}
	else
	{
		pthread_t thisThread = pthread_self();

#ifdef __LP64__
		syslog( LOG_CRIT, "Error %d - Unlocking mutex 0x%016lX - Thread 0x%016lX (0x%x) - Not owned - Caller '%s:%d'", error, 
			    (unsigned long) &mMutex, (unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#else
		syslog( LOG_CRIT, "Error %d - Unlocking mutex 0x%08lX - Thread 0x%08lX (0x%x) - Not owned - Caller '%s:%d'", error, 
			    (unsigned long) &mMutex, (unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#endif
		
		DumpLockHistory( this );
	}

	OSSpinLockUnlock( &mLockHistoryInfo->fOSLock );
}

void DSMutexSemaphore::WaitDebugHistory( const char *file, int line )
{
	const char *shortName = NULL;
	int error = pthread_mutex_lock( &mMutex );

	OSSpinLockLock( &mLockHistoryInfo->fOSLock );
	
	shortName = strrchr( file, '/' );
	if ( shortName != NULL )
		shortName = file;
	
	pthread_t thisThread = pthread_self();

	if ( error == 0 )
	{
		if( mLockHistoryInfo->fThreadOwner == NULL )
		{
			mLockHistoryInfo->fThreadOwner = thisThread;
			mLockHistoryInfo->fLockTime = time( NULL );
			pthread_setspecific( mLockHistoryInfo->fThreadKey, this );
		}

		mLockHistoryInfo->fThreadOwnerCount++;
		mLockHistoryInfo->fCurrHolderIndex = (mLockHistoryInfo->fCurrHolderIndex + 1) % threadHistoryCount;
		
#ifdef __LP64__
		snprintf( mLockHistoryInfo->fLockHistory[mLockHistoryInfo->fCurrHolderIndex], threadHistoryStringLen, "Succeed - Lock Depth %d - 0x%016lX (0x%x) - %s:%d", mLockHistoryInfo->fThreadOwnerCount,
				  (unsigned long) mLockHistoryInfo->fThreadOwner, pthread_mach_thread_np(mLockHistoryInfo->fThreadOwner), shortName, line );
#else
		snprintf( mLockHistoryInfo->fLockHistory[mLockHistoryInfo->fCurrHolderIndex], threadHistoryStringLen, "Succeed - Lock Depth %d - 0x%08lX (0x%x) - %s:%d", mLockHistoryInfo->fThreadOwnerCount,
				  (unsigned long) mLockHistoryInfo->fThreadOwner, pthread_mach_thread_np(mLockHistoryInfo->fThreadOwner), shortName, line );
#endif
		mLockHistoryInfo->fCurrHolder = mLockHistoryInfo->fLockHistory[mLockHistoryInfo->fCurrHolderIndex];
	}
	else
	{
		// copy this into our threadHistory list, so we have the last few callers, should not impact locks above
#ifdef __LP64__
		syslog( LOG_CRIT, "Error %d - Locking mutex - Thread 0x%016lX (0x%x) - Caller '%s:%d'", error, 
			    (unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#else
		syslog( LOG_CRIT, "Error %d - Locking mutex - Thread 0x%08lX (0x%x) - Caller '%s:%d'", error, 
				(unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#endif
		DumpLockHistory( this );
	}

	OSSpinLockUnlock( &mLockHistoryInfo->fOSLock );
}

bool DSMutexSemaphore::WaitTryDebugHistory( const char *file, int line )
{
	const char *shortName = NULL;
	int error = pthread_mutex_trylock( &mMutex );
	
	OSSpinLockLock( &mLockHistoryInfo->fOSLock );
	
	shortName = strrchr( file, '/' );
	if ( shortName != NULL )
		shortName = file;
	
	pthread_t thisThread = pthread_self();
	
	if ( error == 0 )
	{
		if( mLockHistoryInfo->fThreadOwner == NULL )
		{
			mLockHistoryInfo->fThreadOwner = thisThread;
			mLockHistoryInfo->fLockTime = time( NULL );
			pthread_setspecific( mLockHistoryInfo->fThreadKey, this );
		}
		
		mLockHistoryInfo->fThreadOwnerCount++;
		mLockHistoryInfo->fCurrHolderIndex = (mLockHistoryInfo->fCurrHolderIndex + 1) % threadHistoryCount;

#ifdef __LP64__
		snprintf( mLockHistoryInfo->fLockHistory[mLockHistoryInfo->fCurrHolderIndex], threadHistoryStringLen, "Succeed - Lock Depth %d - 0x%016lX (0x%x) - %s:%d", mLockHistoryInfo->fThreadOwnerCount,
				  (unsigned long) mLockHistoryInfo->fThreadOwner, pthread_mach_thread_np(mLockHistoryInfo->fThreadOwner), shortName, line );
#else
		snprintf( mLockHistoryInfo->fLockHistory[mLockHistoryInfo->fCurrHolderIndex], threadHistoryStringLen, "Succeed - Lock Depth %d - 0x%08lX (0x%x) - %s:%d", mLockHistoryInfo->fThreadOwnerCount,
				 (unsigned long) mLockHistoryInfo->fThreadOwner, pthread_mach_thread_np(mLockHistoryInfo->fThreadOwner), shortName, line );
#endif
		mLockHistoryInfo->fCurrHolder = mLockHistoryInfo->fLockHistory[mLockHistoryInfo->fCurrHolderIndex];
	}
	else if ( error != EBUSY )
	{
		// copy this into our threadHistory list, so we have the last few callers, should not impact locks above
#ifdef __LP64__
		syslog( LOG_CRIT, "Error %d - TryLocking mutex - Thread 0x%016lX (0x%x) - Caller '%s:%d'", error, 
			    (unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#else
		syslog( LOG_CRIT, "Error %d - TryLocking mutex - Thread 0x%08lX (0x%x) - Caller '%s:%d'", error, 
			   (unsigned long) thisThread, pthread_mach_thread_np(thisThread), shortName, line );
#endif
		DumpLockHistory( this );
	}
	
	OSSpinLockUnlock( &mLockHistoryInfo->fOSLock );
	
	return (error == 0);
}

void DSMutexSemaphore::DumpLockHistory( DSMutexSemaphore *inMutex )
{
	static OSSpinLock	localLock	= OS_SPINLOCK_INIT;
	int					ii;
	
	OSSpinLockLock( &localLock );
	
	syslog( LOG_CRIT, "Lock - %s", inMutex->mMutexName );
	
	if ( inMutex->mLockHistoryInfo->fThreadOwner )
	{
#ifdef __LP64__
		syslog( LOG_CRIT, "    Current Holder - Thread 0x%016lX (0x%x) - %s", (unsigned long) inMutex->mLockHistoryInfo->fThreadOwner, 
			    pthread_mach_thread_np(inMutex->mLockHistoryInfo->fThreadOwner), inMutex->mLockHistoryInfo->fCurrHolder );
#else
		syslog( LOG_CRIT, "    Current Holder - Thread 0x%08lX (0x%x) - %s", (unsigned long) inMutex->mLockHistoryInfo->fThreadOwner, 
			   pthread_mach_thread_np(inMutex->mLockHistoryInfo->fThreadOwner), inMutex->mLockHistoryInfo->fCurrHolder );
#endif
	}
	syslog( LOG_CRIT, "    Lock depth - %d", inMutex->mLockHistoryInfo->fThreadOwnerCount );
#ifdef __LP64__
	syslog( LOG_CRIT, "    DumpLockHistory - 0x%016lX - Begin\n", (unsigned long) &inMutex->mMutex );
#else
	syslog( LOG_CRIT, "    DumpLockHistory - 0x%08lX - Begin\n", (unsigned long) &inMutex->mMutex );
#endif
	
	for ( ii = ((inMutex->mLockHistoryInfo->fCurrHolderIndex + 1) % threadHistoryCount); ii < threadHistoryCount; ii++ )
	{
		if ( inMutex->mLockHistoryInfo->fLockHistory[ii][0] != '\0' )
			syslog( LOG_CRIT, "        %s", inMutex->mLockHistoryInfo->fLockHistory[ii] );
	}
	
	for ( ii = 0; ii <= inMutex->mLockHistoryInfo->fCurrHolderIndex; ii++ )
	{
		if ( inMutex->mLockHistoryInfo->fLockHistory[ii][0] != '\0' )
			syslog( LOG_CRIT, "        %s", inMutex->mLockHistoryInfo->fLockHistory[ii] );
	}
	
	syslog( LOG_CRIT, "    DumpLockHistory - End\n" );
	
	OSSpinLockUnlock( &localLock );
	
	BreakIfDebugging();
}

#endif

