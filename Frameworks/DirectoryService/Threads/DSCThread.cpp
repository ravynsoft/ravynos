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
 * @header DSCThread
 * Defines abstract basic application thread object.
 */

#include "DSCThread.h"
#include "PrivateTypes.h"
#include <syslog.h>

SInt32 DSCThread::fStatThreadCount = 0;

// ----------------------------------------------------------------------------
//	* DSCThread()
//
// ----------------------------------------------------------------------------

DSCThread::DSCThread ( const UInt32 inThreadSig )
{
	fStatThreadCount++;
	fThreadSignature	= inThreadSig;
	fStateFlags			= kThreadWait;

} // DSCThread


// ----------------------------------------------------------------------------
//	* ~DSCThread()
//
// ----------------------------------------------------------------------------

DSCThread::~DSCThread ()
{
	fStatThreadCount--;
} // ~DSCThread


// ----------------------------------------------------------------------------
//	* GetCurThreadRunState() (static)
//
// ----------------------------------------------------------------------------

UInt32 DSCThread::GetCurThreadRunState ( void )
{
	// All threads in our universe are DSCThread objects
	DSCThread *cur = (DSCThread *)DSLThread::GetCurrentThread();
	if ( cur == nil ) throw((SInt32)eMemoryError);

	return( cur->GetThreadRunState() );

} // GetCurThreadRunState


// ----------------------------------------------------------------------------
//	* Count() (static)
//
// ----------------------------------------------------------------------------

SInt32 DSCThread::Count ( void )
{
	return( fStatThreadCount );

} // Count


// ----------------------------------------------------------------------------
//	* GetID()
//
// ----------------------------------------------------------------------------

long DSCThread::GetID ( void ) const
{
	return (long) fThread;
} // GetID


// ----------------------------------------------------------------------------
//	* GetSignature()
//
// ----------------------------------------------------------------------------

UInt32 DSCThread::GetSignature ( void ) const
{
	return( fThreadSignature );
} // GetSignature


// ----------------------------------------------------------------------------
//	* Run()
//
//		- DSLThread::Run override
//
// ----------------------------------------------------------------------------

void *DSCThread::Run ( void )
{
	UInt32 result = 0;

	try {
		fStateFlags = kThreadRun;
		result = ThreadMain();
	}

	catch (...)
	{
		syslog(LOG_ALERT,"DSCThread::Caught unknown error from ThreadMain.");
	}

	try {
		// Give the thread a last chance to do clean up before
		// it gets destructed
		LastChance();
	}

	catch (...)
	{
		syslog(LOG_ALERT,"DSCThread::Caught unknown error from LastChance.");
	}

	return( (void *)result );

} // Run

// ----------------------------------------------------------------------------
//	* GetThreadRunState()
//
// ----------------------------------------------------------------------------

UInt32 DSCThread::GetThreadRunState ( void )
{
	return( fStateFlags );
} // GetThreadRunState


// ----------------------------------------------------------------------------
//	* SetThreadRunState()
//
// ----------------------------------------------------------------------------

void DSCThread::SetThreadRunState ( eRunState inState )
{
	fStateFlags = inState;
} // SetThreadRunState


// ----------------------------------------------------------------------------
//	* SetThreadRunStateFlag()
//
// ----------------------------------------------------------------------------

void DSCThread::SetThreadRunStateFlag ( eRunState inStateFlag )
{
	fStateFlags |= inStateFlag;
} // SetThreadRunStateFlag


// ----------------------------------------------------------------------------
//	* UnSetThreadRunStateFlag()
//
// ----------------------------------------------------------------------------

void DSCThread::UnSetThreadRunStateFlag ( eRunState inStateFlag )
{
	fStateFlags &= ~inStateFlag;
} // UnSetThreadRunStateFlag



