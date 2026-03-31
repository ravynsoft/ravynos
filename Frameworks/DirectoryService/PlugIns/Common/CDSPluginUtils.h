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
 * @header CDSPluginUtils
 */

#ifndef __CDSPluginUtils_h__
#define __CDSPluginUtils_h__		1

#include <string.h>		//used for strcpy, etc.
#include <stdlib.h>		//used for malloc

#include "PrivateTypes.h"
#include "DirServicesConst.h"
#include "DirServicesTypes.h"
#include "DirServices.h"
#include "DirServicesUtils.h"

#include <CoreFoundation/CoreFoundation.h>

//TODO DoAnyMatch should be made common ie. needs argument rework though


const char*		CStrFromCFString	( CFStringRef inCFStr, char** ioCStr, size_t* ioCStrSize, bool* outCStrAllocated = NULL );

__BEGIN_DECLS
void CFDebugLog					(	SInt32 lType, const char* format, ... );
void CFDebugLogV				(	SInt32 lType, const char* format, va_list ap );

SInt32 PWOpenDirNode			(	tDirNodeReference fDSRef,
									char *inNodeName,
									tDirNodeReference *outNodeRef );

bool DoesThisMatch				(	const char *inString,
									const char *inPatt,
									tDirPatternMatch inHow );

tDirStatus MSCHAPv2ChangePass	(	const char *inUsername,
									const uint8_t *inLMHash,
									const char *inEncoding,
									const uint8_t *inData,
									UInt32 inDataLen,
									uint8_t **outPassword,
									UInt32 *outPasswordLen );

char *GetLocalKDCRealm			( void );

void LaunchKerberosAutoConfigTool( void );

void dsNotifyUpdatedRecord		(	const char *inModule, const char *inNodeName, const char *inRecType );

char *GenerateRandomComputerPassword( void );

__END_DECLS

#endif
