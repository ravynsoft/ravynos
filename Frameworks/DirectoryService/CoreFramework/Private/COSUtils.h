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
 * @header COSUtils
 */

#ifndef __COSUtils_h__
#define __COSUtils_h__	1

#include <DirectoryServiceCore/PrivateTypes.h>


enum eAppResourceIDs {
	kAppStringsListID	= 256		// list with application related strings
};

// Strings in the kAppInfo String List
enum eAppInfoStringOffsets {
	kStrProductFolder       = 1,
	kStrPluginFolder,
	kStrLogFolder,
	kStrServerLogFileName,
	kStrErrorLogFileName,
	kStrDebugLogFileName,
	kStrInfoLogFileName,
	kStrPluginExtension,
	kStrOtherPlugInsFolder,
	kStrLocalProductFolder,
	kStrAppStrEnd
};

class COSUtils
{
public:
	static	const char*	GetStringFromList	( const UInt32 inListID, const SInt32 inIndex );
};

int dsTouch( const char* path );
int dsRemove( const char* path );

#endif
