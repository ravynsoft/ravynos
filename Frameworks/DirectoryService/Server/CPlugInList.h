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
 * @header CPlugInList
 */

#ifndef __CPlugInList_h__
#define __CPlugInList_h__	1

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugIn.h>

#include "PrivateTypes.h"
#include "DSMutexSemaphore.h"
#include "DSEventSemaphore.h"
#include "PluginData.h"

#define	kRecTypeRestrictionsFilePath "/Library/Preferences/DirectoryService/DSRecordTypeRestrictions.plist"
#define	kRecTypeRestrictionsCorruptedFilePath "/Library/Preferences/DirectoryService/DSRecordTypeRestrictionsCorrupted.plist"
#define kDefaultRecTypeRestrictionsConfig \
"<dict>\
	<key>Version</key>\
	<string>1.0</string>\
	<key>BSD</key>\
	<dict>\
	<key>/BSD/local</key>\
	<dict>\
	<key>Deny Record Types</key>\
	<array>\
	<string>dsRecTypeStandard:Users</string>\
	<string>dsRecTypeStandard:Groups</string>\
	</array>\
	</dict>\
	</dict>\
</dict>"

#define kDefaultDisableBSDUsersAndGroups \
"<dict>\
	<key>/BSD/local</key>\
	<dict>\
	<key>Deny Record Types</key>\
	<array>\
	<string>dsRecTypeStandard:Users</string>\
	<string>dsRecTypeStandard:Groups</string>\
	</array>\
	</dict>\
</dict>"

#define kRTRVersionKey		"Version"
#define	kRTRAllowKey		"Allow Record Types"
#define kRTRDenyKey			"Deny Record Types"

#include "CServerPlugin.h"

// Typedefs --------------------------------------------------------------------

class CPlugInList {
public:
typedef struct sTableData
{
   	const char			*fName;
   	const char			*fVersion;
    const char			*fConfigAvail;
    const char			*fConfigFile;
	CServerPlugin		*fPluginPtr;
	CFPlugInRef			fPluginRef;
	CFUUIDRef			fCFuuidFactory;
	UInt32				fULVers;
	FourCharCode		fKey;
	UInt32				fState;
	UInt32				fValidDataStamp; //perhaps better if uuid can seed this mod count?
	eDSPluginLevel		fLevel;
	sTableData		   *pNext;
} sTableData;

enum {
	kMaxPlugIns	= 128
};

public:
				CPlugInList			( void );
	virtual	   ~CPlugInList			( void );

	SInt32	   	AddPlugIn			(	const char 		*inName,
										const char 		*inVersion,
										const char 		*inConfigAvail,
										const char 		*inConfigFile,
										eDSPluginLevel	 inLevel,
										FourCharCode	 inKey,
										CServerPlugin	*inPlugin,
										CFPlugInRef 	 inPluginRef = NULL,
										CFUUIDRef		 inCFuuidFactory = NULL,
										UInt32			 inULVers = 0 );

										
	void		LoadPlugin			( sTableData *inTableEntry );
	void		InitPlugIns			( eDSPluginLevel inLevel );

	SInt32	 	IsPresent			( const char *inName );

	SInt32		GetState			( const char *inName, UInt32 *outState );
	SInt32		SetState			( const char *inName, const UInt32 inState );

	SInt32		UpdateValidDataStamp( const char *inName );
	UInt32		GetValidDataStamp	( const char *inName );

	UInt32		GetPlugInCount		( void );
	UInt32		GetActiveCount		( void );

	sTableData*	GetPlugInInfo		( UInt32 inIndex );
	
	void		RegisterPlugins		(void);

CServerPlugin*	Next				( UInt32 *inIndex );

CServerPlugin* 	GetPlugInPtr		( const char *inName, bool loadIfNeeded = true );
CServerPlugin* 	GetPlugInPtr		( const UInt32 inKey, bool loadIfNeeded = true );

	SInt32		ReadRecordTypeRestrictions( void );
	bool		IsOKToServiceQuery	( const char *inPluginName, const char *inNodeName, const char *inRecordTypeList, UInt32 inNumberRecordTypes );
	
	CFMutableDictionaryRef	CopyRecordTypeRestrictionsDictionary( void );
	void					SetRecordTypeRestrictionsDictionary( CFMutableDictionaryRef inDictionary );

protected:
	bool		CreatePrefDirectory	( void );
	sTableData*	MakeTableEntryCopy	( sTableData* inEntry );
	void		SetPluginState		( CServerPlugin	*inPluginPtr, ePluginState inPluginState );


	CFDictionaryRef		fCFRecordTypeRestrictions;

private:
	UInt32				fPICount;
	DSMutexSemaphore		fMutex;
	sTableData			*fTable;
	sTableData			*fTableTail;
	DSEventSemaphore   	fWaitToInit;
};

#endif
