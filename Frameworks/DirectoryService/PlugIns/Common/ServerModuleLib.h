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
 * @header ServerModuleLib
 * Function prototype for server library functions.
 */

#ifndef __ServerModuleLib_h__
#define __ServerModuleLib_h__	1

#include <stdarg.h>		// for va_list

#include <DirectoryServiceCore/PrivateTypes.h>
#include <DirectoryServiceCore/ServerModule.h>	// for SvrLibFtbl

extern "C"
{
	// For use by all modules
	void		SetupLinkTable		( DSServerPlugin::SvrLibFtbl *inTable );
	SInt32		DSRegisterNode		( const UInt32 inToken, tDataList *inNode, eDirNodeType inNodeType );
	SInt32		DSUnregisterNode	( const UInt32 inToken, tDataList *inNode );
	SInt32		DSDebugLog			( const char *inFormat, va_list inArgs );
	SInt32		DSDebugLogWithType	( const UInt32 inSignature, const UInt32 inLogType, const char *inFormat, va_list inArgs );
}

#endif	// __ServerModuleLib_h__
