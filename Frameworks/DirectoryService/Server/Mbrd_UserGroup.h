/*
 * Copyright (c) 2004-2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __Mbrd_UserGroup_h__
#define	__Mbrd_UserGroup_h__		1

#include "Mbrd_HashTable.h"

#include <sys/types.h>
#include <sys/kauth.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <dispatch/dispatch.h>

enum
{
	kUGRecordTypeUser			= 0x00000001,
	kUGRecordTypeComputer		= 0x00000002,
	kUGRecordTypeGroup			= 0x00000004,
	kUGRecordTypeComputerGroup	= 0x00000008,
	kUGRecordTypeUnknown		= 0xffffffff
};

enum
{
	kUGFoundByID			= 0x00000001,
	kUGFoundByGUID			= 0x00000002,
	kUGFoundByName			= 0x00000004,
	kUGFoundBySID			= 0x00000008,
	kUGFoundByNestedGroup	= 0x00000010,
	kUGFoundByX509DN		= 0x00000020,
	kUGFoundByKerberos		= 0x00000040,
	
	// this flag says that a search was scheduled already
	// if a conflict was found, then the previous answer will be invalid (erratic behavior, which is expected)
	// if the same answer is found, then it'll be tagged as such
	kUGFoundByIDSched			= 0x00010000,
	kUGFoundByGUIDSched			= 0x00020000,
	kUGFoundByNameSched			= 0x00040000,
	kUGFoundBySIDSched			= 0x00080000,
//	kUGFoundByNestedGroupSched	-- not used for searches
	kUGFoundByX509DNSched		= 0x00100000,
	kUGFoundByKerberosSched		= 0x00200000,
};

enum
{
	kUGFlagHasID			= 0x00000001,
	kUGFlagHasGUID			= 0x00000002,
	kUGFlagHasName			= 0x00000004,
	kUGFlagHasSID			= 0x00000008,
	kUGFlagHasX509DN		= 0x00000010,
	kUGFlagHasKerberos		= 0x00000020,
	
	kUGFlagReservedID		= 0x00200000,
	kUGFlagReservedName		= 0x00400000,
	kUGFlagReservedSID		= 0x00800000,
	
	kUGFlagBuiltinChecked	= 0x04000000,
	kUGFlagIsBuiltin		= 0x08000000,
	
	kUGFlagValidMembership	= 0x20000000,
	kUGFlagLocalAccount		= 0x40000000,
	kUGFlagNotFound			= 0x80000000,
};

#define kMaxAltIdentities	5

typedef struct UserGroup
{
	int32_t				fMagic;
	volatile int32_t	fRefCount;

	pthread_mutex_t		fMutex;
	struct UserGroup*   fLink;		// owned by the Mbrd_Cache
	struct UserGroup*   fBackLink;
	uint32_t			fExpiration;
	uint32_t			fMaximumRefresh;
	uuid_t				fGUID;
	ntsid_t				fSID;
	id_t				fID;
	gid_t				fPrimaryGroup;
	char *				fX509DN[kMaxAltIdentities];
	char *				fKerberos[kMaxAltIdentities];
	uint32_t			fFlags;
	
	// used for validation purposes so we don't flush entries
	char*				fNode;
	uint32_t            fToken;
	bool                fNodeAvailable;
	dispatch_queue_t	fQueue;
	dispatch_queue_t	fRefreshQueue;
	bool				fRefreshActive;	// used to track inflight refresh

	char*				fName;
	int32_t				fRecordType;
	uint32_t			fFoundBy;
	time_t				fTimestamp;
	
	pthread_mutex_t		fHashLock;
	struct HashTable	fGUIDMembershipHash;
	struct HashTable	fSIDMembershipHash;
	struct HashTable	fGIDMembershipHash;
} UserGroup;

__BEGIN_DECLS

UserGroup* UserGroup_Create( void );
#define UserGroup_Retain(a)			((UserGroup *) dsRetainObject(a, &a->fRefCount))
void UserGroup_Release( UserGroup *source );
void UserGroup_Free( UserGroup *source );
void UserGroup_Initialize( UserGroup *source );
void UserGroup_Merge( UserGroup *existing, UserGroup *source, bool includeMemberships );

bool UserGroup_AddToHashes( UserGroup *item, UserGroup *group );
int UserGroup_ResetMemberships( UserGroup *ug );

int UserGroup_Get16Groups( UserGroup* user, gid_t* gidArray );
int UserGroup_GetGroups( UserGroup* user, gid_t** gidArray );

const char *UserGroup_GetRecordTypeString( UserGroup *user );
const char *UserGroup_GetFoundByString( UserGroup *user, char *buffer, size_t bufferLen );

// Utility functions
uint32_t GetElapsedSeconds( void );
uint64_t GetElapsedMicroSeconds( void );

__END_DECLS

#endif
