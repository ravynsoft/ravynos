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
 * @header DSSemaphore
 * Interface for the DSSemaphore (lock) base class.
 */

#ifndef _DSSemaphore_H_
#define _DSSemaphore_H_

#include <DirectoryServiceCore/DSMutexSemaphore.h> // needed for lock defines

class DSSemaphore
{
	public:
					DSSemaphore			( const char *inName = NULL );
					~DSSemaphore		( void );

		void		Wait				( void ) { WaitDebug(NULL, 0 ); }
		void		Signal				( void ) { SignalDebug(NULL, 0); }
		int			WaitTry				( void ) { return WaitTryDebug(NULL, 0); }
		
		void		WaitDebug			( const char *file, int line );
		bool		WaitTryDebug		( const char *file, int line );
		void		SignalDebug			( const char *file, int line );

		static void	LockCleanup			( void *value );
		
		void		WaitDebugHistory	( const char *file, int line ) { WaitDebug(file, line); }
		bool		WaitTryDebugHistory	( const char *file, int line ) { return WaitTryDebug(file, line); }
		void		SignalDebugHistory	( const char *file, int line ) { SignalDebug( file, line ); }

	private:
		pthread_mutex_t			mMutex;
		const char				*mMutexName;
		struct sLockHistoryInfo	*mLockHistoryInfo;
};

#endif	/* _DSSemaphore_H_ */
