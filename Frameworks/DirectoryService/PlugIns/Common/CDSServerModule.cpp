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
 * @header CDSServerModule
 * Implementation of server-side module base class.
 */

#include <cassert>				// for assert() macro

#include "ServerModuleLib.h"
#include "CDSServerModule.h"


// ---------------------------------------------------------------------------
//	* CDSServerModule Class Globals
//
//		These private typedefs, globals, and functions are not declared statically
//		in the class definition because I want to hide the implementation details
//		and reduce unrelated dependencies in the class header.
// ----------------------------------------------------------------------------

CDSServerModule	*CDSServerModule::sInstance = 0;

// ----------------------------------------------------------------------------
//	sCreator is not defined here. It should be defined in the subclass.
//	Copy and uncomment the following line in the subclass and replace
//	SubclassCreator with the appropriate static method name.
//	CDSServerModule::Creator	CDSServerModule::sCreator = SubclassCreator;


extern "C" {
extern void	*ModuleFactory ( CFAllocatorRef, CFUUIDRef );
// Do not delete the following function; it guarantees that PluginLib is
// properly linked so the server can load this as a CFPlugIn.
static void _LinkRefDoNotDelete ( void ) __attribute__((used));

static void _LinkRefDoNotDelete ( void )
{
	ModuleFactory( NULL, NULL );
}
}


// ----------------------------------------------------------------------------
//	* Validate
//
//		Make sure everything the module requires is available.
// ----------------------------------------------------------------------------

SInt32 CDSServerModule::Validate ( const char *inVersionStr, const UInt32 inSignature )
{
	fPlugInSignature = inSignature;
	
	return( eDSNoErr );
} // Validate

SInt32 CDSServerModule::Initialize ( void )
{
	return( eDSNoErr );
} // Initialize

SInt32 CDSServerModule::Configure ( void )
{
	return( eDSNoErr );
} // Configure

SInt32 CDSServerModule::SetPluginState ( const UInt32 inState )
{
#pragma unused ( inState )
	return( eDSNoErr );
} // SetPluginState

SInt32 CDSServerModule::PeriodicTask ( void )
{
	return( eDSNoErr );
} // PeriodicTask

SInt32 CDSServerModule::ProcessRequest ( void *inData )
{
#pragma unused ( inData )
	return( eDSNoErr );
} // ProcessRequest

SInt32 CDSServerModule::Shutdown ( void )
{
	return( eDSNoErr );
} // Shutdown

// ----------------------------------------------------------------------------
//	* CDSServerModule Protected Instance Methods
// ----------------------------------------------------------------------------
#pragma mark **** Protected Instance Methods ****

// ----------------------------------------------------------------------------
//	* Constructor and destructor
// ----------------------------------------------------------------------------

CDSServerModule::CDSServerModule ( void )
{
} // 

CDSServerModule::~CDSServerModule ( void )
{
} // 

