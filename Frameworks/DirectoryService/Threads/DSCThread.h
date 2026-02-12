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
 * Defines the base application thread.
 */

#ifndef __DSCThread_h__
#define __DSCThread_h__	1

#include <DirectoryServiceCore/DSLThread.h>
#include <DirectoryServiceCore/PrivateTypes.h>

const SInt32 kEventPeriod			= 60;		// ticks
const SInt32 kLongPeriod			= 10;		// ticks
const SInt32 kShortPeriod			=  5;		// ticks

const UInt32 kMinPerHour			= 60;
const UInt32 kSecondsPerMin			= 60;
const UInt32 kMilliSecsPerSec		= 1000;
const UInt32 kMicroSecsPerSec		= 1000*1000;
const UInt32 kNanoSecPerSec			= 1000*1000*1000;

const UInt32 kBadValue				= 0xffffffff;
void ** const kThreadIgnoreResult	= NULL;


class DSCThread : public DSLThread
{
public:

	enum eSignature {
		// Normal Threads
		kTSUndefinedThread			= '----',
		kTSMigHandlerThread			= 'mhdl',
		kTSPlugInHndlrThread		= 'pihn',
		kTSSearchPlugInHndlrThread	= 'sphn',
		kTSMemberdKernelHndlrThread	= 'mkhn',
		kTSLauncherThread			= 'lnch',
		kTSNodeRegisterThread		= 'ndrg',
		kTSTCPListenerThread		= 'tcpl',
		kTSTCPConnectionThread		= 'tcpc',
		kTSPluginRunloopThread		= 'prlt',
		kTSLibinfoQueueThread		= 'liqt'
	};

	enum eRunState {
		kThreadRun	= 0x00000000,
		kThreadStop	= 0x00000001,
		kThreadWait	= 0x00000002
	};

						DSCThread				( const UInt32 inThreadSig = 0 );
	virtual			   ~DSCThread				( void );

public:
	// Class methods (static)
	static	UInt32		GetCurThreadRunState	( void );
	static	SInt32		Count					( void );

	// Class methods
	virtual long		GetID					( void ) const;
	virtual UInt32		GetSignature			( void ) const;

protected:
	virtual void*		Run						( void );
	virtual SInt32		ThreadMain				( void ) = 0;	// pure virtual
	virtual void		LastChance				( void ) { }

	virtual	UInt32		GetThreadRunState		( void );
	virtual	void		SetThreadRunState		( eRunState inState );

	virtual	void		SetThreadRunStateFlag	( eRunState inStateFlag );
	virtual	void		UnSetThreadRunStateFlag	( eRunState inStateFlag );

	static	SInt32		fStatThreadCount;

protected:
	FourCharCode		fThreadSignature;

private:
	UInt32				fStateFlags;
};



#endif // __DSCThread_h__
