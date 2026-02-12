/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include	"StdAfx.h"

#include	"DebugServices.h"

#include	"ExplorerBar.h"
#include	"ExplorerPlugin.h"

#include	"ClassFactory.h"

// MFC Debugging

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//===========================================================================================================================
//	ClassFactory
//===========================================================================================================================

ClassFactory::ClassFactory( CLSID inCLSID )
{
	mCLSIDObject 	= inCLSID;
	mRefCount		= 1;
	++gDLLRefCount;
}

//===========================================================================================================================
//	~ClassFactory
//===========================================================================================================================

ClassFactory::~ClassFactory( void )
{
	check( gDLLRefCount > 0 );
	
	--gDLLRefCount;
}

#if 0
#pragma mark -
#pragma mark == IUnknown methods ==
#endif

//===========================================================================================================================
//	QueryInterface
//===========================================================================================================================

STDMETHODIMP	ClassFactory::QueryInterface( REFIID inID, LPVOID *outResult )
{
	HRESULT		err;
	
	check( outResult );
	
	if( IsEqualIID( inID, IID_IUnknown ) )
	{
		*outResult = this;
	}
	else if( IsEqualIID( inID, IID_IClassFactory ) )
	{
		*outResult = (IClassFactory *) this;
	}   
	else
	{
		*outResult = NULL;
		err = E_NOINTERFACE;
		goto exit;
	}
	
	( *( (LPUNKNOWN *) outResult ) )->AddRef();
	err = S_OK;
	
exit:
	return( err );
}                                             

//===========================================================================================================================
//	AddRef
//===========================================================================================================================

STDMETHODIMP_( DWORD )	ClassFactory::AddRef( void )
{
	return( ++mRefCount );
}

//===========================================================================================================================
//	Release
//===========================================================================================================================

STDMETHODIMP_( DWORD )	ClassFactory::Release( void )
{
	DWORD		count;
	
	count = --mRefCount;
	if( count == 0 )
	{
		delete this;
	}
	return( count );
}

#if 0
#pragma mark -
#pragma mark == IClassFactory methods ==
#endif

//===========================================================================================================================
//	CreateInstance
//===========================================================================================================================

STDMETHODIMP	ClassFactory::CreateInstance( LPUNKNOWN inUnknown, REFIID inID, LPVOID *outObject )
{
	HRESULT		err;
	LPVOID		obj;
	
	check( outObject );
	
	obj 		= NULL;
	*outObject 	= NULL;
	require_action( !inUnknown, exit, err = CLASS_E_NOAGGREGATION );
	
	// Create the object based on the CLSID.
	
	if( IsEqualCLSID( mCLSIDObject, CLSID_ExplorerBar ) )
	{
		try
		{
			obj = new ExplorerBar();
		}
		catch( ... )
		{
			// Don't let exception escape.
		}
		require_action( obj, exit, err = E_OUTOFMEMORY );
	}
	else
	{
		err = E_FAIL;
		goto exit;
	}
	
	// Query for the specified interface. Release the factory since QueryInterface retains it.
		
	err = ( (LPUNKNOWN ) obj )->QueryInterface( inID, outObject );
	( (LPUNKNOWN ) obj )->Release();
	
exit:
	return( err );
}

//===========================================================================================================================
//	LockServer
//===========================================================================================================================

STDMETHODIMP	ClassFactory::LockServer( BOOL inLock )
{
	DEBUG_UNUSED( inLock );
	
	return( E_NOTIMPL );
}
