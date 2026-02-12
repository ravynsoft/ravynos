/*
 * Copyright (c) 2004-2009 Apple Inc. All rights reserved.
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

#if !defined(DISABLE_SEARCH_PLUGIN) || !defined(DISABLE_MEMBERSHIP_CACHE)

#include "Mbrd_UserGroup.h"

#include <mach/mach_time.h>
#include <pthread.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include <DirectoryServiceCore/CLog.h>

static long double		gScaleNumerator		= 0.0;
static long double		gScaleDenominator	= 0.0;
static pthread_once_t	gInitializeTimebase	= PTHREAD_ONCE_INIT;

static void InitializeTimebase( void )
{
	struct mach_timebase_info mti = { 0 };
	
	mach_timebase_info( &mti );
	
	gScaleNumerator = (long double) mti.numer;
	gScaleDenominator = (long double) mti.denom;	
}

uint32_t GetElapsedSeconds( void )
{
	pthread_once( &gInitializeTimebase, InitializeTimebase );
	
	long double elapsed = (long double) mach_absolute_time();
	long double temp = (elapsed * gScaleNumerator) / gScaleDenominator;
	
	return (uint32_t) (temp / (long double) NSEC_PER_SEC);
}

uint64_t GetElapsedMicroSeconds( void )
{
	pthread_once( &gInitializeTimebase, InitializeTimebase );
	
	long double elapsed = (long double) mach_absolute_time();
	long double temp = (elapsed * gScaleNumerator) / gScaleDenominator;
	
	return (uint64_t) (temp / (long double) NSEC_PER_USEC);
}

#pragma mark -
#pragma mark UserGroup routines

UserGroup* UserGroup_Create( void )
{
	UserGroup* result = (UserGroup *) calloc( 1, sizeof(UserGroup) );
	assert( result != NULL );
	
	result->fMagic = 'free';
	result->fRefCount = 1;
	
	UserGroup_Initialize( result );
	
	return result;
}

void UserGroup_Initialize( UserGroup *source )
{
	pthread_mutexattr_t attr;
	
	// if set with 'free' we calloc'd it, so don't rezero here
	if ( source->fMagic != 'free' ) {
		bzero( source, sizeof(UserGroup) );
		source->fRefCount = INT32_MAX;
	}
	
	assert( pthread_mutexattr_init(&attr) == 0 );
	assert( pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) == 0 );
	assert( pthread_mutex_init(&source->fMutex, &attr) == 0 );
	assert( pthread_mutex_init(&source->fHashLock, &attr) == 0 );
	
	pthread_mutexattr_destroy( &attr );
	
	source->fNodeAvailable = true;
	source->fID = -1; // safety in case they are not set
	source->fPrimaryGroup = -1; // safety in case they are not set
	source->fTimestamp = time( NULL );
	source->fQueue = dispatch_queue_create( "UserGroup queue", NULL );
	source->fRefreshQueue = dispatch_queue_create( "UserGroup refresh queue", NULL );

	HashTable_Initialize( &source->fGUIDMembershipHash, "GUID", source, eGUIDHash );
	HashTable_Initialize( &source->fSIDMembershipHash, "SID", source, eSIDHash );
	HashTable_Initialize( &source->fGIDMembershipHash, "GID", source, eIDHash );
}

void UserGroup_Release( UserGroup *source )
{
	if ( dsReleaseObject(source, &source->fRefCount, false) == true ) {
		UserGroup_Free( source );
		
		if ( source->fMagic == 'free' ) {
			free( source );	
		}
	}
}

void UserGroup_Free( UserGroup *source )
{
	assert( pthread_mutex_lock(&source->fMutex) == 0 );
	
	DSFree( source->fName );
	DSFree( source->fNode );
	
	for ( int ii = 0; ii < kMaxAltIdentities && source->fKerberos[ii] != NULL; ii++ ) {
		DSFree( source->fKerberos[ii] );
	}
	
	for ( int ii = 0; ii < kMaxAltIdentities && source->fX509DN[ii] != NULL; ii++ ) {
		DSFree( source->fX509DN[ii] );
	}
	
	dispatch_release( source->fQueue );
	dispatch_release( source->fRefreshQueue );
	
	HashTable_FreeContents( &source->fGUIDMembershipHash );
	HashTable_FreeContents( &source->fSIDMembershipHash );
	HashTable_FreeContents( &source->fGIDMembershipHash );
	
	pthread_mutex_destroy( &source->fMutex );
	pthread_mutex_destroy( &source->fHashLock );
}

void UserGroup_Merge( UserGroup *existing, UserGroup *source, bool includeMemberships )
{
	assert( pthread_mutex_lock(&existing->fMutex) == 0 );
	
	existing->fTimestamp = source->fTimestamp;
	existing->fExpiration = source->fExpiration;
	existing->fMaximumRefresh = source->fMaximumRefresh;
	uuid_copy( existing->fGUID, source->fGUID );
	existing->fID = source->fID;
	existing->fPrimaryGroup = source->fPrimaryGroup;
	// do not change fRefreshActive here cause it could cancel an inflight lookup
	// since we can lookup the user at the same time
	
	if ( (source->fFlags & kUGFlagHasSID) != 0 ) {
		bcopy( &source->fSID, &existing->fSID, sizeof(ntsid_t) );
	}
	else {
		existing->fFlags &= ~kUGFlagHasSID;
	}
	
	if ( existing->fNode != NULL ) {
		free( existing->fNode );
	}
	
	if ( source->fNode != NULL ) {
		existing->fNode = strdup( source->fNode );
	}
	else {
		existing->fNode = NULL;
	}
	
	existing->fToken = source->fToken;
	existing->fNodeAvailable = source->fNodeAvailable;

	if ( existing->fName != NULL ) {
		free( existing->fName );
	}
	
	if ( source->fName != NULL ) {
		existing->fName = strdup( source->fName );
	}
	else {
		existing->fName = NULL;
	}
	
	// X509 field
	for ( int ii = 0; ii < kMaxAltIdentities; ii++ ) {
		DSFree( existing->fX509DN[ii] );
		if ( source->fX509DN[ii] != NULL ) {
			existing->fX509DN[ii] = strdup( source->fX509DN[ii] );
		}
	}
	
	// Kerberos field
	for ( int ii = 0; ii < kMaxAltIdentities; ii++ ) {
		DSFree( existing->fKerberos[ii] );
		if ( source->fKerberos[ii] != NULL ) {
			existing->fKerberos[ii] = strdup( source->fKerberos[ii] );
		}
	}
	
	existing->fRecordType = source->fRecordType;

	existing->fFlags |= (source->fFlags & ~kUGFlagValidMembership);
	existing->fFoundBy |= source->fFoundBy;
	
	// if the new entry also has updated memberships we should dump the old and add the new
	if ( includeMemberships == true && (source->fFlags & kUGFlagValidMembership) != 0 )
	{
		assert( pthread_mutex_lock(&existing->fHashLock) == 0 );

		HashTable_Reset( &existing->fGUIDMembershipHash );
		HashTable_Merge( &existing->fGUIDMembershipHash, &source->fGUIDMembershipHash );

		HashTable_Reset( &existing->fSIDMembershipHash );
		HashTable_Merge( &existing->fSIDMembershipHash, &source->fSIDMembershipHash );

		HashTable_Reset( &existing->fGIDMembershipHash );
		HashTable_Merge( &existing->fGIDMembershipHash, &source->fGIDMembershipHash );
		
		assert( pthread_mutex_unlock(&existing->fHashLock) == 0 );

		existing->fFlags |= kUGFlagValidMembership;
	}
	
	assert( pthread_mutex_unlock(&existing->fMutex) == 0 );
}

bool UserGroup_AddToHashes( UserGroup *item, UserGroup *group )
{
	bool bSuccess = false;
	
	assert( pthread_mutex_lock(&item->fHashLock) == 0 );

	if ( item != NULL )
	{
		if ( (group->fFlags & kUGFlagHasGUID) != 0 )
			bSuccess = HashTable_Add( &item->fGUIDMembershipHash, group, false );
		if ( (group->fFlags & kUGFlagHasID) != 0 )
			bSuccess = (HashTable_Add(&item->fGIDMembershipHash, group, false) ? true : bSuccess);
		if ( (group->fFlags & kUGFlagHasSID) != 0 )
			bSuccess = (HashTable_Add(&item->fSIDMembershipHash, group, false) ? true : bSuccess);
	}
	
	assert( pthread_mutex_unlock(&item->fHashLock) == 0 );
	
	return bSuccess;
}

int UserGroup_ResetMemberships( UserGroup *ug )
{
	int totalOffline = 0;
	
	// this will force a refresh, but keep the existing entries
	__sync_and_and_fetch( &ug->fFlags, ~kUGFlagValidMembership );
	
	assert( pthread_mutex_lock(&ug->fHashLock) == 0 );
	totalOffline += HashTable_ResetMemberships( &ug->fGUIDMembershipHash );
	totalOffline += HashTable_ResetMemberships( &ug->fSIDMembershipHash );
	totalOffline += HashTable_ResetMemberships( &ug->fGIDMembershipHash );
	DbgLog( kLogInfo, "mbr_mig - Membership - User/Group - Reset Memberships for %s (%d) - %d offline memberships", ug->fName ? :"\"no name\"", 
		    ug->fID, totalOffline );
	assert( pthread_mutex_unlock(&ug->fHashLock) == 0 );

	return totalOffline;
}

const char *UserGroup_GetRecordTypeString( UserGroup *user )
{
	const char *type;
	
	switch( user->fRecordType )
	{
		case kUGRecordTypeUser:
			type = "User";
			break;
			
		case kUGRecordTypeGroup:
			type = "Group";
			break;
			
		case kUGRecordTypeUser | kUGRecordTypeComputer:
			type = "User/Computer";
			break;
			
		case kUGRecordTypeComputer:
			type = "Computer";
			break;
			
		case kUGRecordTypeComputerGroup:
			type = "ComputerGroup";
			break;
			
		case kUGRecordTypeGroup | kUGRecordTypeComputerGroup:
			type = "Group/ComputerGroup";
			break;
			
		default:
			type = "No record type";
			break;
	}
	
	return type;
}

const char *UserGroup_GetFoundByString( UserGroup *user, char *buffer, size_t bufferLen )
{
	bool	bAddComma	= false;
	
	assert( buffer != NULL && bufferLen > 0 );
	
	buffer[0] = '\0';
	if ( (user->fFoundBy & kUGFoundByGUID) != 0 ) {
		strlcat( buffer, "UUID", bufferLen );
		bAddComma = true;
	}
	
	if ( (user->fFoundBy & kUGFoundByID) != 0 ) {
		if ( bAddComma == true ) {
			strlcat( buffer, ", ", bufferLen );
		};
		strlcat( buffer, "ID", bufferLen );
		bAddComma = true;
	}
	
	if ( (user->fFoundBy & kUGFoundByName) != 0 ) {
		if ( bAddComma == true ) {
			strlcat( buffer, ", ", bufferLen );
		};
		strlcat( buffer, "Name", bufferLen );
		bAddComma = true;
	}
	
	if ( (user->fFoundBy & kUGFoundBySID) != 0 ) {
		if ( bAddComma == true ) {
			strlcat( buffer, ", ", bufferLen );
		};
		strlcat( buffer, "SID", bufferLen );
		bAddComma = true;
	}
	
	if ( (user->fFoundBy & kUGFoundByNestedGroup) != 0 ) {
		if ( bAddComma == true ) {
			strlcat( buffer, ", ", bufferLen );
		};
		strlcat( buffer, "Nested group search", bufferLen );
	}
	
	if ( (user->fFoundBy & kUGFoundByX509DN) != 0 ) {
		if ( bAddComma == true ) {
			strlcat( buffer, ", ", bufferLen );
		};
		strlcat( buffer, "X509DN", bufferLen );
	}
	
	if ( (user->fFoundBy & kUGFoundByKerberos) != 0 ) {
		if ( bAddComma == true ) {
			strlcat( buffer, ", ", bufferLen );
		};
		strlcat( buffer, "Kerberos ID", bufferLen );
	}
	
	if ( user->fFoundBy == 0 ) {
		strlcat( buffer, "Unspecified", bufferLen );
	}
	
	return buffer;
}

int UserGroup_Get16Groups( UserGroup* user, gid_t* gidArray )
{
	gid_t *tempArray = NULL;
	
	int numGroups = UserGroup_GetGroups( user, &tempArray );
	if ( numGroups > 16 ) numGroups = 16;
	
	if ( numGroups > 0 ) {
		bcopy( tempArray, gidArray, numGroups * sizeof(gid_t) );
		free( tempArray );
	}
	
	return numGroups;
}

int UserGroup_GetGroups( UserGroup* user, gid_t** gidArray )
{
	UserGroup	**groupArray	= NULL;
	gid_t		*itemArray		= NULL;
	int			itemArrayCount	= 0;
	int			i				= 0;
	gid_t		pgid			= -1;
	
	if ( user == NULL || gidArray == NULL ) return 0;
	
	int groupArrayCount = HashTable_CreateItemArray( &user->fGUIDMembershipHash, &groupArray );
	if ( groupArrayCount > 0 )
	{
		(*gidArray) = itemArray = (gid_t *) calloc( groupArrayCount, sizeof(gid_t) );
		assert( itemArray != NULL );
		
		for ( i = 0; i < groupArrayCount; i++ )
		{
			gid_t tempGID = groupArray[i]->fID;
			if ( tempGID != pgid ) {
				itemArray[itemArrayCount++] = tempGID;
			}
			
			UserGroup_Release( groupArray[i] );
			groupArray[i] = NULL;
		}
		
		if ( groupArray != NULL ) {
			free( groupArray );
			groupArray = NULL;
		}
	}
	else {
		// safety if no groups, always return PGID
		(*gidArray) = itemArray = (gid_t *) calloc( 1, sizeof(gid_t) );
		assert( itemArray != NULL );

		itemArray[0] = user->fPrimaryGroup;
		itemArrayCount = 1;
	}
	
	return itemArrayCount;
}

#endif // DISABLE_SEARCH_PLUGIN
