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
 * @header CServerPlugin
 * Interface for server-side plug-in wrapper class.
 */

#ifndef __CServerPlugin_h__
#define __CServerPlugin_h__	1

#include <CoreFoundation/CFPlugIn.h>
#include <CoreFoundation/CFString.h>

#include "ServerModule.h"
#include "PrivateTypes.h"

enum eDSPluginLevel {
	/*  1 */ kStaticPlugin			= 1,
	/*    */ kAppleLoadedPlugin
};

static	const	UInt32		kBuffPad					= 16;

//-----------------------------------------------------------------------------
//	* CServerPlugin Class Definition
//
//		- CServerPlugin is used to wrap CFPlugin type com.apple.DSServer.Module
//			instances for (hopefully) transparent use by C++.
//-----------------------------------------------------------------------------

class CServerPlugin {
public:
	static	SInt32	ProcessURL				(	CFURLRef urlPlugin );
    static	SInt32	ProcessStaticPlugin		(	const char* inPluginName,
                                                const char* inPluginVersion );

public:
	/**** Instance methods. ****/
	// ctor and dtor.
				CServerPlugin	( void );
                CServerPlugin	( FourCharCode inSig, const char *inName );
				CServerPlugin	( CFPlugInRef inThis, CFUUIDRef inFactoryID, FourCharCode inSig, UInt32 inVers, const char *inName );
	virtual	   ~CServerPlugin	( void );

	// New methods.
		// Pass-thru functions to CFPlugin function table.
	virtual SInt32	Validate		( const char *inVersionStr, const UInt32 inSignature );
	virtual SInt32	Initialize		( void );
	virtual SInt32	Configure		( void );
	virtual SInt32	SetPluginState	( const UInt32 inState );
	virtual SInt32	PeriodicTask	( void );
	virtual SInt32	ProcessRequest	( void *inData );
	virtual SInt32	Shutdown		( void );

	char*			GetPluginName	( void );
	FourCharCode	GetSignature	( void );
	static SInt32	_RegisterNode	( const UInt32, tDataList *, eDirNodeType );
	static SInt32	InternalRegisterNode ( const UInt32 inToken, tDataList *inNodeList, eDirNodeType inNodeType, bool isProxyRegistration = false );
    static SInt32	_UnregisterNode	( const UInt32, tDataList * );
    
protected:
	FourCharCode	fPlugInSignature;
	char		   *fPlugInName;

private:
	// Instance data
	DSServerPlugin::ModuleFtbl	*mInstance;

	CFPlugInRef		fPlugInRef;
	UInt32			fPlugInVers;
};

#endif	// __CServerPlugin_h__
