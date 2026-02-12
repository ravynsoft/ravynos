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
 * @header ServerModule
 */

#include <stdio.h>	// for fprintf()

#include <CoreFoundation/CoreFoundation.h>

#include "ServerModule.h"
#include "ServerModuleLib.h"
#include "CDSServerModule.h"
#include "DirServicesTypes.h"

using namespace DSServerPlugin;

// To fix a bug in previous versions of CF
#ifndef _HRESULT_TYPEDEF_
 #define _HRESULT_TYPEDEF_(x)	x
#endif


// The structs and functions in this file must be laid out according to C
// standards or the COM stuff won't work.
extern "C" {
extern CFUUIDRef	ModuleFactoryUUID;
extern void		   *ModuleFactory( CFAllocatorRef, CFUUIDRef );
}


// ----------------------------------------------------------------------------
//	* Private stuff
// ----------------------------------------------------------------------------

typedef struct _tagModuleType : public ModuleFtbl
{
	UInt32			mRefCount;
	CDSServerModule	*mInstance;
} _ModuleType;


static _ModuleType	*_VTablePrototype = NULL;


// ----------------------------------------------------------------------------
//	* COM Support functions
//
//		Boilerplate stuff to support COM's three required methods.
// ----------------------------------------------------------------------------

#pragma mark **** COM Support functions ****

//--------------------------------------------------------------------------------------------------
//	* _COMQueryInterface ()
//
//--------------------------------------------------------------------------------------------------

static HRESULT _COMQueryInterface ( void *thisp, REFIID iid, LPVOID *ppv )
{
	CFUUIDRef	uuidInterface = ::CFUUIDCreateFromUUIDBytes ( NULL, iid );

	if ( ::CFEqual ( uuidInterface, kModuleInterfaceUUID ) ||
		 ::CFEqual ( uuidInterface, IUnknownUUID ) )
	{
		// IUnknownVTbl type is used in case the object is not one of our types.
		((IUnknownVTbl *)thisp)->AddRef(thisp);
		*ppv = thisp;
		::CFRelease( uuidInterface );

		return( S_OK );
	}

	::CFRelease( uuidInterface );
	*ppv = NULL;

	return( E_NOINTERFACE );

} // _COMQueryInterface


//--------------------------------------------------------------------------------------------------
//	* _COMAddRef ()
//
//--------------------------------------------------------------------------------------------------

static ULONG _COMAddRef ( void *thisp )
{
	_ModuleType	*opThis = static_cast<_ModuleType *> (thisp);

	if ( opThis == NULL )
	{
		::fputs ( "Bad cast to _ModuleType!\n", stderr );
		return( (ULONG)-1 );
	}

	return( ++(opThis->mRefCount) );

} // _COMAddRef


//--------------------------------------------------------------------------------------------------
//	* _COMRelease ()
//
//--------------------------------------------------------------------------------------------------

static ULONG _COMRelease ( void *thisp )
{
	_ModuleType	*opThis = static_cast<_ModuleType *> (thisp);

	if ( opThis == NULL )
	{
		::fputs ( "Bad cast to _ModuleType in Release!\n", stderr );
		return( (ULONG)-1 );
	}
	// Dealloc the ref count.
	if ( --(opThis->mRefCount) )
	{
		return( opThis->mRefCount );
	}

	// If there are no ref counts left, dealloc the object for real.
#if DEBUG
	::puts ( "_COMRelease: deallocating...\n" );
#endif

	delete( opThis );
	::CFPlugInRemoveInstanceForFactory ( ModuleFactoryUUID );

	return( 0 );

} // _COMRelease




// ----------------------------------------------------------------------------
//	* ModuleInterface functions
//
//		C glue code from COM interface functions to C++ instance methods.
//
// ----------------------------------------------------------------------------

#pragma mark **** ModuleInterface functions ****

//--------------------------------------------------------------------------------------------------
//	* _Validate ()
//
//--------------------------------------------------------------------------------------------------

static SInt32 _Validate ( void *thisp, const char *inVersionStr, const UInt32 inSignature )
{
	SInt32	nResult = eDSNoErr;
	_ModuleType	*opThis = static_cast<_ModuleType *> (thisp);

	if ( opThis == NULL )
	{
		::fputs ("Bad cast to _ModuleType in Validate!\n", stderr);
		return( -1 );
	}

	try {
		nResult = opThis->mInstance->Validate( inVersionStr, inSignature );
	}

	catch( SInt32 err )
	{
		nResult = err;
	}

	catch( SInt16 err )
	{
		nResult = err;
	}

	catch( ... )
	{
		nResult = eParameterError;
	}

	return( nResult );

} // _Validate


//--------------------------------------------------------------------------------------------------
//	* _Initialize()
//
//--------------------------------------------------------------------------------------------------

static SInt32 _Initialize ( void *thisp )
{
	SInt32	nResult = eDSNoErr;
	_ModuleType	*opThis = static_cast<_ModuleType *> (thisp);

	if ( opThis == NULL )
	{
		::fputs ("Bad cast to _ModuleType in Initialize!\n", stderr);
		return( -1 );
	}

	try {
		nResult = opThis->mInstance->Initialize();
	}

	catch( SInt32 err )
	{
		nResult = err;
	}

	catch( SInt16 err )
	{
		nResult = err;
	}

	catch( ... )
	{
		nResult = eParameterError;
	}

	return( nResult );

} // _Initialize


//--------------------------------------------------------------------------------------------------
//	* _Configure ()
//
//--------------------------------------------------------------------------------------------------

static SInt32 _Configure ( void *thisp )
{
	SInt32	nResult = eDSNoErr;
	_ModuleType	*opThis = static_cast<_ModuleType *> (thisp);

	if ( opThis == NULL )
	{
		::fputs ("Bad cast to _ModuleType in Configure!\n", stderr);
		return( -1 );
	}

	try {
		nResult = opThis->mInstance->Configure();
	}

	catch( SInt32 err )
	{
		nResult = err;
	}

	catch( SInt16 err )
	{
		nResult = err;
	}

	catch( ... )
	{
		nResult = eParameterError;
	}

	return( nResult );

} // _Configure


//--------------------------------------------------------------------------------------------------
//	* _ProcessRequest ()
//
//--------------------------------------------------------------------------------------------------

static SInt32 _ProcessRequest ( void *thisp, void *inData )
{
	SInt32	nResult = eDSNoErr;
	_ModuleType	*opThis = static_cast<_ModuleType *> (thisp);

	if ( opThis == NULL )
	{
		::fputs ( "Bad cast to _ModuleType in ProcessRequest!\n", stderr );
		return( -1 );
	}

	try {
		nResult = opThis->mInstance->ProcessRequest( inData );
	}

	catch ( SInt32 err )
	{
		nResult = err;
	}

	catch ( SInt16 err )
	{
		nResult = err;
	}

	catch ( ... )
	{
		nResult = eParameterError;
	}

	return( nResult );

} // _ReceiveFromClient


//--------------------------------------------------------------------------------------------------
//	* _SetPluginState ()
//
//--------------------------------------------------------------------------------------------------

static SInt32 _SetPluginState ( void *thisp, const UInt32 inState )
{
	SInt32	nResult = eDSNoErr;
	_ModuleType	*opThis = static_cast<_ModuleType *> (thisp);

	if ( opThis == NULL )
	{
		::fputs( "Bad cast to _ModuleType in SetPluginState!\n", stderr );
		return( -1 );
	}

	try {
		nResult = opThis->mInstance->SetPluginState( inState );
	}

	catch( SInt32 err )
	{
		nResult = err;
	}

	catch( SInt16 err )
	{
		nResult = err;
	}

	catch( ... )
	{
		nResult = eParameterError;
	}

	return( nResult );

} // _SetPluginState


//--------------------------------------------------------------------------------------------------
//	* _PeriodicTask ()
//
//--------------------------------------------------------------------------------------------------

static SInt32 _PeriodicTask ( void *thisp )
{
	SInt32	nResult = eDSNoErr;
	_ModuleType	*opThis = static_cast<_ModuleType *> (thisp);

	if ( opThis == NULL )
	{
		::fputs( "Bad cast to _ModuleType in PeriodicTask!\n", stderr );
		return( -1 );
	}

	try {
		nResult = opThis->mInstance->PeriodicTask();
	}

	catch( SInt32 err )
	{
		nResult = err;
	}

	catch( SInt16 err )
	{
		nResult = err;
	}

	catch( ... )
	{
		nResult = eParameterError;
	}

	return( nResult );

} // _PeriodicTask


//--------------------------------------------------------------------------------------------------
//	* _Shutdown ()
//
//--------------------------------------------------------------------------------------------------

static SInt32 _Shutdown ( void *thisp )
{
	SInt32	nResult = eDSNoErr;
	_ModuleType	*opThis = static_cast<_ModuleType *> (thisp);

	if ( opThis == NULL )
	{
		::fputs( "Bad cast to _ModuleType in Shutdown!\n", stderr );
		return( -1 );
	}

	try {
		nResult = opThis->mInstance->Shutdown();
	}

	catch( SInt32 err )
	{
		nResult = err;
	}

	catch( SInt16 err )
	{
		nResult = err;
	}

	catch( ... )
	{
		nResult = eParameterError;
	}

	return( nResult );

} // _Shutdown


//--------------------------------------------------------------------------------------------------
//	* _LinkLibFtbl ()
//
//--------------------------------------------------------------------------------------------------

static void _LinkLibFtbl ( void *thisp, SvrLibFtbl *inLinkBack )
{
	::SetupLinkTable( inLinkBack );
} // _LinkLibFtbl


//--------------------------------------------------------------------------------------------------
//	* _InitializeModule ()
//
//--------------------------------------------------------------------------------------------------

static void _InitializeModule ( void )
{
	_VTablePrototype = new _ModuleType;

	if ( _VTablePrototype != nil )
	{
		// Assign the function pointers.
		_VTablePrototype->QueryInterface	= _COMQueryInterface;
		_VTablePrototype->AddRef			= _COMAddRef;
		_VTablePrototype->Release			= _COMRelease;

		_VTablePrototype->validate			= _Validate;
		_VTablePrototype->initialize		= _Initialize;
		_VTablePrototype->configure			= _Configure;
		_VTablePrototype->processRequest	= _ProcessRequest;
		_VTablePrototype->setPluginState	= _SetPluginState;
		_VTablePrototype->periodicTask		= _PeriodicTask;
		_VTablePrototype->shutdown			= _Shutdown;
		_VTablePrototype->linkLibFtbl 		= _LinkLibFtbl;

		// Set the instance data variables.
		_VTablePrototype->mInstance			= 0;
		_VTablePrototype->mRefCount			= 1;
	}
	else
	{
		::fputs ( "Serious memory allocation error!\n", stderr );
	}

} // _InitializeModule


// ----------------------------------------------------------------------------
//	* Factory function
//	This is the only exported function in the file.
// ----------------------------------------------------------------------------

void *ModuleFactory ( CFAllocatorRef allocator, CFUUIDRef typeID )
{
	if ( _VTablePrototype == nil )
	{
		_InitializeModule();
	}

#if DEBUG
	::puts( "ModuleFactory: loaded and called!\n" );
	::fflush( stdout );
#endif

	if ( ::CFEqual( typeID, kModuleTypeUUID ) )
	{
		_ModuleType	*opNew = new _ModuleType;
		// Set the instance data variables.
		*opNew = *_VTablePrototype;
		opNew->mInstance = CDSServerModule::Instance();
		::CFPlugInAddInstanceForFactory( ModuleFactoryUUID );

		return( opNew );
	}
	else
	{
		return( NULL );
	}

} // ModuleFactory

