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
 * Defines abstract basic application thread object.
 */

#include <unistd.h>

#include "DSLThread.h"
#include "PrivateTypes.h"

bool	DSLThread::sIsInited	= false;

// ----------------------------------------------------------------------------
//	* DSLThread Class Globals
// ----------------------------------------------------------------------------

#define NO_THREAD NULL

static pthread_attr_t	_DefaultAttrs;
static pthread_key_t	_NameKey;
static pthread_key_t	_ObjectKey;

// ----------------------------------------------------------------------------
//	* DSLThread()
//
// ----------------------------------------------------------------------------

DSLThread::DSLThread ( void )
{
	fThread				= NO_THREAD;
	fNextOfKin			= NULL;
	fResult				= NULL;

	// All ctor arguments are ignored, except for main thread flag.
	if ( sIsInited == false )
	{
		sIsInited = true;

		::pthread_attr_init( &_DefaultAttrs );
		::pthread_key_create(&_NameKey, NULL);
		::pthread_key_create(&_ObjectKey, NULL);
		::pthread_attr_setdetachstate( &_DefaultAttrs, PTHREAD_CREATE_DETACHED);
	}
} // DSLThread


// ----------------------------------------------------------------------------
//	* ~DSLThread()
//
// ----------------------------------------------------------------------------

DSLThread::~DSLThread ( void )
{
	fThread = NO_THREAD;
} // ~DSLThread


// ----------------------------------------------------------------------------
//	* _RunWrapper()
//
//		cthread_fork() / pthread_create() callback function.
//		This must be a static method so it can access member variables.
//
// ----------------------------------------------------------------------------

void *DSLThread::_RunWrapper ( void *arg )
{
	DSLThread		*oMe = (DSLThread *)arg;
	void			*theResult;
	ThreadIDT		tMe = ::pthread_self();

	::pthread_setspecific( _ObjectKey, oMe );
	::pthread_setspecific( _NameKey, "DSLThread" );

	oMe->fThread = tMe;

	// Execute the thread.
	try {
		oMe->fResult = theResult = oMe->Run();
	}

	catch (...)
	{
        oMe->fResult = theResult = (void *)errKilledThread;
	}

	// Notify next of kin.
	if ( oMe->fNextOfKin )
	{
		oMe->fNextOfKin->ThreadDied( *oMe );
	}

	// Mark the thread as completed.
	delete( oMe );

	return( theResult );

} // _RunWrapper


// ----------------------------------------------------------------------------
//	* Resume()
//
//		Start the thread running
//
// ----------------------------------------------------------------------------

void DSLThread::Resume ( void )
{
	// Throw an exception if the thread is already running.
	if ( fThread != NO_THREAD )
	{
		throw( (SInt32)errBadThreadState );
	}

	// Currently detaching so threads don't stick around.
	pthread_t	tNew = NULL;

    ::pthread_create( &tNew, &_DefaultAttrs, _RunWrapper, (void *)this );

} // Resume


// ----------------------------------------------------------------------------
//	* GetCurrentThread()
//
//		Start the thread running
//
// ----------------------------------------------------------------------------

DSLThread *DSLThread::GetCurrentThread ( void )
{
	DSLThread *pOutResult	= NULL;

	if ( sIsInited == true )
	{
		pOutResult = (DSLThread *)::pthread_getspecific (_ObjectKey);
	}

	return( pOutResult );

} // GetCurrentThread


// ----------------------------------------------------------------------------
//	* IsCurrent()
//
//		Is this the thread that's executing?
//
// ----------------------------------------------------------------------------

bool DSLThread::IsCurrent ( void ) const
{
	return( fThread == ::pthread_self() );
} // IsCurrent


// ----------------------------------------------------------------------------
//	* Run()
//
//		As a debugging convenience, throw an exception.
//
// ----------------------------------------------------------------------------

void* DSLThread::Run ( void )
{
	throw -619;

	return NULL;
} // Run


// ----------------------------------------------------------------------------
//	* ThreadDied()
//
// ----------------------------------------------------------------------------


void DSLThread::ThreadDied ( const DSLThread &inThread )
{
#pragma unused ( inThread )
} // ThreadDied
