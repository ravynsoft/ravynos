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
 * Abstract, Singleton, Factory base class for server plugin.
 */

#ifndef __CDSServerModule_H__
#define __CDSServerModule_H__	1

#include <DirectoryServiceCore/PluginData.h>

// ============================================================================
//	* CDSServerModule Abstract Base Class
// ============================================================================

class CDSServerModule
{
public:
	/**** Class (static) methods. ****/
	static CDSServerModule	*Instance	( void )
				{	if (!sInstance) sInstance = sCreator ();
					return sInstance; }

public:
	virtual SInt32	Validate			( const char *inVersionStr, const UInt32 inSignature );
	virtual SInt32	Initialize			( void );
	virtual SInt32	Configure			( void );
	virtual SInt32	SetPluginState		( const UInt32 inState );
	virtual SInt32	PeriodicTask		( void );
	virtual SInt32	ProcessRequest		( void *inData );
	virtual SInt32	Shutdown			( void );

protected:
	/**** Instance methods accessible only to class and subclasses. ****/
	// ctor and dtor are protected so only static methods can create
	// and destroy modules.
						CDSServerModule		( void );
	virtual			   ~CDSServerModule		( void );

protected:
	/**** Private typedefs. ****/
	typedef CDSServerModule			*(*tCreator) (void);

	/**** Class globals. ****/
	static CDSServerModule			*sInstance;
	static tCreator					sCreator;

	/**** Instance data. ****/
	FourCharCode	fPlugInSignature;
	const char		*fPlugInName;
};

#endif	/* _CDSServerModule_H_ */
