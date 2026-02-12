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
 * Interface for the DSMutexSemaphore (mutually exclusive lock) base class.
 */

#ifndef _DSMutexSemaphore_H_
#define _DSMutexSemaphore_H_

#include <pthread.h>	// for pthread_*_t

#include <DirectoryServiceCore/PrivateTypes.h>

// Two mutually-exclusive defines can be done at during compile, either DEBUG_LOCKS or DEBUG_LOCKS_HISTORY
//    DEBUG_LOCKS			will break in a debugger and log to syslog on errors with locks
//    DEBUG_LOCKS_HISTORY	This flag REQUIRES recompile of Framework -- it will track the file and line a 
//							lock was acquired and how long it was held on an error the lock history will be 
//							dumped to system.log, and can be done manually by calling DSMutexSemaphore::DumpLockHistory(mutex)

// Convenience for DS project, by default DEBUG_LOCKS enabled in Development mode
//#undef DEBUG_LOCKS
//#define DEBUG_LOCKS_HISTORY

#pragma mark Macros

#if defined(DEBUG_LOCKS) && defined(DEBUG_LOCKS_HISTORY)
	#error cannot define both DEBUG_LOCKS and DEBUG_LOCKS_HISTORY
#elif defined(DEBUG_LOCKS_HISTORY)
	#define WaitLock()				WaitDebugHistory( __FILE__, __LINE__ )
	#define WaitTryLock()			WaitTryDebugHistory( __FILE__, __LINE__ )
	#define SignalLock()			SignalDebugHistory( __FILE__, __LINE__ )
#else
	#define WaitLock()				WaitDebug( __FILE__, __LINE__ )
	#define WaitTryLock()			WaitTryDebug(__FILE__, __LINE__ )
	#define SignalLock()			SignalDebug( __FILE__, __LINE__ )
#endif

class DSMutexSemaphore
{
public:
				DSMutexSemaphore	( const char *mutexName = NULL, bool bShouldDTrace = true );
				~DSMutexSemaphore   ( void );

	void		Wait				( void );
	bool		WaitTry				( void );
	void		Signal				( void );

	void		WaitDebug			( const char *file, int line );
	bool		WaitTryDebug		( const char *file, int line );
	void		SignalDebug			( const char *file, int line );
	
	static void	DumpLockDebug		( DSMutexSemaphore *inMutex );
	static bool BeingDebugged		( void );
	static void	BreakIfDebugging	( void );
	static void	LockCleanup			( void *value );

#if defined(DEBUG_LOCKS_HISTORY)
	void		WaitDebugHistory	( const char *file, int line );
	bool		WaitTryDebugHistory	( const char *file, int line );
	void		SignalDebugHistory	( const char *file, int line );
	
	static void	DumpLockHistory		( DSMutexSemaphore *inMutex );
#endif
	
private:
	pthread_mutex_t			mMutex;
	const char				*mMutexName;
	
	// we allocate this separate so we don't have to recompile other components
	struct sLockHistoryInfo	*mLockHistoryInfo;
};

#endif	/* _DSMutexSemaphore_H_ */
