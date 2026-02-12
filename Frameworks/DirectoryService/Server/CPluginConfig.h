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
 * @header CPluginConfig
 */

#ifndef __CPluginConfig_h__
#define __CPluginConfig_h__ 1

#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFPropertyList.h>

#include "PluginData.h"
#include "PrivateTypes.h"


#define	kConfigFilePath		"/Library/Preferences/DirectoryService/DirectoryService.plist"
#define kDefaultConfig		"<dict>\
	<key>Version</key>\
	<string>1.1</string>\
	<key>BSD</key>\
	<string>Active</string>\
	<key>AppleTalk</key>\
	<string>Active</string>\
	<key>Active Directory</key>\
	<string>Inactive</string>\
</dict>"
#define kServerDefaultConfig	"<dict>\
	<key>Version</key>\
	<string>1.1</string>\
	<key>AppleTalk</key>\
	<string>Inactive</string>\
	<key>Active Directory</key>\
	<string>Inactive</string>\
</dict>"
#define	kJaguarUpdateFilePath		"/Library/Preferences/DirectoryService/.DSJaguarUpdate"
#define kDefaultUpgradeConfig		"<dict>\
	<key>Version</key>\
	<string>1.1</string>\
	<key>BSD</key>\
	<string>Inactive</string>\
	<key>Active Directory</key>\
	<string>Inactive</string>\
</dict>"
#define kServerDefaultUpgradeConfig	"<dict>\
	<key>Version</key>\
	<string>1.1</string>\
	<key>Active Directory</key>\
	<string>Inactive</string>\
</dict>"

#define kVersionKey			"Version"
#define kAppleTalkPluginKey "AppleTalk"

#define	kActiveValue		"Active"
#define kInactiveValue		"Inactive"

#define kTooManyReferencesWarningCount				"Too Many References Warning Count"
#define kDelayFailedLocalAuthReturnsDeltaInSeconds  "Delay Failed Local Auth Returns Delta In Seconds"
#define kMaxHandlerThreadCount						"Maximum Number of Handler Threads"

class CPluginConfig
{
public:
					CPluginConfig		( void );
	virtual		   ~CPluginConfig		( void );

	SInt32			Initialize			( void );
	SInt32			SaveConfigData		( void );

	ePluginState	GetPluginState		( const char *inPluginName );
	SInt32			SetPluginState		( const char *inPluginName, const ePluginState inPluginState );

protected:

private:
	//CFDataRef			fDataRef;
	CFPropertyListRef	fPlistRef;
	CFMutableDictionaryRef	fDictRef;
};

#endif
