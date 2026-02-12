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
 * @header DSLThread
 * Defines the base application thread.
 */

#ifndef __DSLThread_h__
#define __DSLThread_h__	1

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef pthread_t	ThreadIDT;

class DSLThread
{
public:

enum {
	errKilledThread = 28000, 
	errBadThreadState
};

							DSLThread			( void );
	virtual				   ~DSLThread			( void );

	static	DSLThread*		GetCurrentThread	( void );

			bool			IsCurrent			( void ) const;
			void			SetNextOfKin		( DSLThread *inThread )	{ fNextOfKin = inThread; }
	virtual void			Resume				( void );

protected:
	virtual void*			Run					( void ) = 0;	// pure virtual
	virtual void			ThreadDied			( const DSLThread &inThread );

		ThreadIDT			fThread;
		DSLThread		   *fNextOfKin;
		void			   *fResult;

private:
	static void*			_RunWrapper			( void *arg );

	static bool				sIsInited;

	/**** Invalid methods and undefined operations. ****/
	// Copy constructor
						DSLThread		( const DSLThread & );
	// Assignment
			DSLThread	&operator=		( const DSLThread & );
};


#endif // __DSLThread_h__
