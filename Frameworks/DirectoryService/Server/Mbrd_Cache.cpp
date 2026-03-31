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

#include "Mbrd_Cache.h"
#include "CPlugInList.h"

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <uuid/uuid.h>
#include <DirectoryServiceCore/CLog.h>
#include <membership.h>

extern CPlugInList		   *gPlugins;

struct _MbrdCache
{
	int32_t					fRefCount;
	
	int32_t					fNumItems;
	int32_t					fDefaultExpiration;
	int32_t					fDefaultNegativeExpiration;
	int32_t					fKernelExpiration;
	int32_t					fMaximumRefresh;
	int32_t					fKerberosFallback;
	
	pthread_mutex_t			fCacheLock;	// used to update cache
	
	UserGroup				*fListHead;
	UserGroup				*fListTail;
	
	struct HashTable		fGUIDHash;
	struct HashTable		fSIDHash;
	struct HashTable		fUIDHash;
	struct HashTable		fGIDHash;
	
	struct HashTable		fUserNameHash;
	struct HashTable		fGroupNameHash;
	struct HashTable		fComputerNameHash;
	struct HashTable		fComputerGroupNameHash;
	struct HashTable		fKerberosHash;
	struct HashTable		fX509Hash;
};

#pragma mark -
#pragma mark Internal routines

void ConvertSIDToString( char* string, ntsid_t* sid )
{
	char* current = string;
	long long temp = 0;
	int i;
	
	for (i = 0; i < 6; i++)
		temp = (temp << 8) | sid->sid_authority[i];
	
	sprintf(current,"S-%u-%llu", sid->sid_kind, temp);
	
	for(i=0; i < sid->sid_authcount; i++)
	{
		current = current + strlen(current);
		sprintf(current, "-%u", sid->sid_authorities[i]);
	}
}

static bool ItemOutdated( UserGroup* item, int flags )
{
	// node not available, just say it is fine
	if ( item->fNodeAvailable == false )
		return false;
	
	if ( item->fNode != NULL )
	{
		uint32_t	iToken	= 0;
		char		tempNode[512];	// should never exceed
		
		strlcpy( tempNode, item->fNode, sizeof(tempNode) );
		
		// if we have a token and we can validate the stamp use it
		char *nodeName = strtok( tempNode, "/" );
		
		if ( nodeName != NULL )
			iToken = gPlugins->GetValidDataStamp( nodeName );
		
		if ( iToken != item->fToken )
			return true;
	}
	
	if ( (flags & KAUTH_EXTLOOKUP_REFRESH_MEMBERSHIP) != 0 )
		return (item->fMaximumRefresh <= GetElapsedSeconds());
	
	return (item->fExpiration <= GetElapsedSeconds());
}

static void MbrdCache_RemoveFromList( MbrdCache *cache, UserGroup* ug )
{
	if ( ug->fLink == NULL )
		cache->fListTail = ug->fBackLink;
	else
		ug->fLink->fBackLink = ug->fBackLink;
	
	if ( ug->fBackLink == NULL )
		cache->fListHead = ug->fLink;
	else
		ug->fBackLink->fLink = ug->fLink;
	
	UserGroup_Release( ug );
	__sync_sub_and_fetch( &cache->fNumItems, 1 );
}

static void MbrdCache_AddToHeadOfList( MbrdCache *cache, UserGroup* ug )
{
	UserGroup_Retain( ug );
	
	ug->fBackLink = NULL;
	if ( cache->fListHead == NULL )
	{
		cache->fListHead = cache->fListTail = ug;
		ug->fLink = NULL;
	}
	else
	{
		ug->fLink = cache->fListHead;
		cache->fListHead->fBackLink = ug;
		cache->fListHead = ug;
	}

	__sync_add_and_fetch( &cache->fNumItems, 1 );
}

static void MbrdCache_AddToHashes( MbrdCache *cache, UserGroup *ug )
{
	uint32_t secs = GetElapsedSeconds();
	
	ug->fMaximumRefresh = secs + cache->fMaximumRefresh;
	ug->fExpiration = secs + ((ug->fFlags & kUGFlagNotFound) != 0 ? cache->fDefaultNegativeExpiration : cache->fDefaultExpiration);

	// all records get added to the GUID hash
	if ( (ug->fFlags & kUGFlagHasGUID) != 0 )
		HashTable_Add( &cache->fGUIDHash, ug, (ug->fFoundBy & kUGFoundByGUID) != 0 );
	
	if ( (ug->fRecordType & kUGRecordTypeGroup) != 0 ) {
		if ( (ug->fFlags & kUGFlagHasID) != 0 )
			HashTable_Add( &cache->fGIDHash, ug, (ug->fFoundBy & kUGFoundByID) != 0 );
		if ( ug->fName != NULL )
			HashTable_Add( &cache->fGroupNameHash, ug, (ug->fFoundBy & kUGFoundByName) != 0 );
	}
	
	if ( (ug->fRecordType & kUGRecordTypeUser) != 0 ) {
		if ( (ug->fFlags & kUGFlagHasID) != 0 )
			HashTable_Add( &cache->fUIDHash, ug, (ug->fFoundBy & kUGFoundByID) != 0 );
		if ( ug->fName != NULL )
			HashTable_Add( &cache->fUserNameHash, ug, (ug->fFoundBy & kUGFoundByName) != 0 );
		if ( ug->fKerberos[0] != NULL )
			HashTable_Add( &cache->fKerberosHash, ug, (ug->fFoundBy & kUGFoundByKerberos) != 0 );
		if ( ug->fX509DN[0] != NULL )
			HashTable_Add( &cache->fX509Hash, ug, (ug->fFoundBy & kUGFoundByX509DN) != 0 );
	}
	
	if ( (ug->fRecordType & kUGRecordTypeComputer) != 0 ) {
		if ( (ug->fFlags & kUGFlagHasID) != 0 )
			HashTable_Add( &cache->fUIDHash, ug, (ug->fFoundBy & kUGFoundByID) != 0 );
		if ( ug->fName != NULL )
			HashTable_Add( &cache->fComputerNameHash, ug, (ug->fFoundBy & kUGFoundByName) != 0 );
		if ( ug->fKerberos[0] != NULL )
			HashTable_Add( &cache->fKerberosHash, ug, (ug->fFoundBy & kUGFoundByKerberos) != 0 );
		if ( ug->fX509DN[0] != NULL )
			HashTable_Add( &cache->fX509Hash, ug, (ug->fFoundBy & kUGFoundByX509DN) != 0 );
	}
	
	if ( (ug->fRecordType & kUGRecordTypeComputerGroup) != 0 ) {
		if ( ug->fName != NULL )
			HashTable_Add( &cache->fComputerGroupNameHash, ug, (ug->fFoundBy & kUGFoundByName) != 0 );
	}
	
	if ( (ug->fFlags & kUGFlagHasSID) != 0 )
		HashTable_Add( &cache->fSIDHash, ug, (ug->fFoundBy & kUGFoundBySID) != 0 );
}

static void MbrdCache_RemoveFromHashes( MbrdCache *cache, UserGroup *ug )
{
	// since an entry can be all types, we have to go through all
	HashTable_Remove( &cache->fGUIDHash, ug );
	
	if ( (ug->fRecordType & kUGRecordTypeGroup) != 0 ) {
		if ( (ug->fFlags & kUGFlagHasID) != 0 )
			HashTable_Remove( &cache->fGIDHash, ug );
		if ( ug->fName != NULL )
			HashTable_Remove( &cache->fGroupNameHash, ug );
	}
	
	if ( (ug->fRecordType & kUGRecordTypeUser) != 0 ) {
		if ( (ug->fFlags & kUGFlagHasID) != 0 )
			HashTable_Remove( &cache->fUIDHash, ug );
		if ( ug->fName != NULL )
			HashTable_Remove( &cache->fUserNameHash, ug );
	}
	
	if ( (ug->fRecordType & kUGRecordTypeComputer) != 0 ) {
		if ( (ug->fFlags & kUGFlagHasID) != 0 )
			HashTable_Remove( &cache->fUIDHash, ug );
		if ( ug->fName != NULL )
			HashTable_Remove( &cache->fComputerNameHash, ug );
	}
	
	if ( (ug->fRecordType & kUGRecordTypeComputerGroup) != 0 ) {
		if ( ug->fName != NULL )
			HashTable_Remove( &cache->fComputerGroupNameHash, ug );
	}
	
	if ( (ug->fFlags & kUGFlagHasSID) != 0 )
		HashTable_Remove( &cache->fSIDHash, ug );
}

static void MbrdCache_AddEntry( MbrdCache *cache, UserGroup *ug )
{
	int32_t beforeRefCount = ug->fRefCount;
	MbrdCache_AddToHashes( cache, ug );
	
	// if the ref count changed (i.e., it was added to some hashes), then we need to add to our list as well
	if ( beforeRefCount != ug->fRefCount ) {
		MbrdCache_AddToHeadOfList( cache, ug );
	}
}

static void MbrdCache_RemoveEntry( MbrdCache *cache, UserGroup* ug )
{
	MbrdCache_RemoveFromHashes( cache, ug );
	MbrdCache_RemoveFromList( cache, ug );
}

static UserGroup *MbrdCache_UpdateExistingRecord( MbrdCache *cache, UserGroup *existing, UserGroup *source )
{
	if ( source == NULL ) {
		return existing;
	}
	
	if ( existing == NULL ) {
		DbgLog( kLogDebug, "mbr_mig - Membership - Adding record %X (%s) because it's new", source, source->fName );
		MbrdCache_AddEntry( cache, source );
		return source;
	}
	
	// this should never be true
	if ( source == existing ) {
		DbgLog( kLogDebug, "mbr_mig - Membership - Not updating record %X (%s) because update is identical record", existing, existing->fName );
		return source;
	}
	
	if ( (existing->fFlags & kUGFlagNotFound) != 0 && (source->fFlags & kUGFlagNotFound) != 0 ) {
		DbgLog( kLogDebug, "mbr_mig - Membership - Not updating record %X (%s) because both entries are negative", existing, existing->fName );
		UserGroup_Release( source );
		return existing;
	}
	
	// if we are updating a record as a not found, but he original is offline, we don't do anything
	if ( existing->fNodeAvailable == false && (source->fFlags & kUGFlagNotFound) != 0 ) {
		DbgLog( kLogInfo, "mbr_mig - Membership - Not updating record %X (%s) because it's offline", existing, existing->fName );
		UserGroup_Release( source );
		return existing;
	}
	
	// if the nodes match
	// if the type and name match
	// they must be the same record, merge current information
	if ( existing->fNode != NULL && source->fNode != NULL && strcmp(existing->fNode, source->fNode) == 0 &&
		 existing->fRecordType == source->fRecordType &&
		 existing->fName != NULL && source->fName != NULL && strcmp(existing->fName, source->fName) == 0 )
	{
		char	buffer[128]	= { 0, };

		MbrdCache_RemoveFromHashes( cache, existing ); // remove from hashes
		UserGroup_Merge( existing, source, false );
		MbrdCache_AddToHashes( cache, existing ); // add back to hashes after update

		if ( source->fFoundBy & kUGFoundByNestedGroup ) {
			DbgLog( kLogInfo, "mbr_mig - Membership - Refreshing record '%s' (%X) with result of indirect search", source->fName, source, 
				   existing, UserGroup_GetFoundByString(source, buffer, sizeof(buffer)) );
		}
		else {
			DbgLog( kLogInfo, "mbr_mig - Membership - Merged record '%s' (%X) into %X - new authority '%s'", source->fName, source, 
				    existing, UserGroup_GetFoundByString(source, buffer, sizeof(buffer)) );
		}
		
		UserGroup_Release( source );
		return existing;
	}
	
	DbgLog( kLogInfo, "mbr_mig - Membership - Removing record %X (%s) to replace with %X (%s)", 
		    existing, existing->fName ? : "", 
		    source, source->fName ? : "" );
	
	MbrdCache_RemoveEntry( cache, existing ); // remove the existing entry
	MbrdCache_AddEntry( cache, source ); // add new entry to cache
	
	UserGroup_Release( existing );

	return source;
}

#pragma mark -
#pragma mark Public routines

MbrdCache *MbrdCache_Create( int32_t defaultExpiration, int32_t defaultNegativeExpiration, int32_t kernelExp, int32_t maxRefresh, int32_t kerberosFallback )
{
	MbrdCache *cache = (MbrdCache *) calloc( 1, sizeof(MbrdCache) );
	assert( cache != NULL );
	
	cache->fDefaultExpiration = defaultExpiration;
	cache->fDefaultNegativeExpiration = defaultNegativeExpiration;
	cache->fKernelExpiration = kernelExp;
	cache->fMaximumRefresh = maxRefresh;
	cache->fKerberosFallback = kerberosFallback;
	
	pthread_mutexattr_t attr;
	assert( pthread_mutexattr_init(&attr) == 0);
	assert( pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) == 0);
	assert( pthread_mutex_init(&cache->fCacheLock, &attr) == 0);
	
	pthread_mutexattr_destroy( &attr );
	
	HashTable_Initialize( &cache->fGUIDHash, "Global GUID", cache, eGUIDHash );
	HashTable_Initialize( &cache->fSIDHash, "Global SID", cache, eSIDHash );
	HashTable_Initialize( &cache->fUIDHash, "Global UID", cache, eIDHash );
	HashTable_Initialize( &cache->fGIDHash, "Global GID", cache, eIDHash );
	
	HashTable_Initialize( &cache->fUserNameHash, "User Name", cache, eNameHash );
	HashTable_Initialize( &cache->fGroupNameHash, "Group Name", cache, eNameHash );
	HashTable_Initialize( &cache->fComputerNameHash, "Computer Name", cache, eNameHash );
	HashTable_Initialize( &cache->fComputerGroupNameHash, "ComputerGroup Name", cache, eNameHash );
	HashTable_Initialize( &cache->fKerberosHash, "Kerberos", cache, eKerberosHash );
	HashTable_Initialize( &cache->fX509Hash, "X509DN", cache, eNameHash );
	
	return cache;
}

void MbrdCache_Release( MbrdCache *cache )
{
	if ( dsReleaseObject(cache, &cache->fRefCount, false) == true ) {
		pthread_mutex_destroy( &cache->fCacheLock );
		
		HashTable_FreeContents( &cache->fGUIDHash );
		HashTable_FreeContents( &cache->fSIDHash );
		HashTable_FreeContents( &cache->fUIDHash );
		HashTable_FreeContents( &cache->fGIDHash );
		
		HashTable_FreeContents( &cache->fUserNameHash );
		HashTable_FreeContents( &cache->fGroupNameHash );
		HashTable_FreeContents( &cache->fComputerNameHash );
		HashTable_FreeContents( &cache->fComputerGroupNameHash );
		HashTable_FreeContents( &cache->fKerberosHash );
		HashTable_FreeContents( &cache->fX509Hash );
		
		free( cache );
		cache = NULL;
	}
}

int32_t MbrdCache_GetDefaultExpiration( MbrdCache *cache )
{
	if ( cache == NULL ) return 0;

	return cache->fDefaultExpiration;
}

UserGroup* MbrdCache_GetAndRetain( MbrdCache *cache, int recordType, int idType, const void *idValue, int32_t flags )
{
	if ( cache == NULL ) return NULL;
	
	UserGroup	*cacheResult	= NULL;
	ntsid_t		tempsid;
	ntsid_t		*sid			= (ntsid_t *) idValue;
	const char *reqOrigin = ((flags & kKernelRequest) != 0 ? "mbr_syscall" : "mbr_mig");

	switch ( idType )
	{
		case ID_TYPE_UID:
			if ( (recordType & kUGRecordTypeUser) != 0 )
				cacheResult = HashTable_GetAndRetain( &cache->fUIDHash, idValue );
			
			if ( cacheResult == NULL )
				DbgLog( kLogInfo, "%s - Membership - Cache miss - by UID %d", reqOrigin, *((id_t *) idValue) );
			break;
		
		case ID_TYPE_GID:
			if ( cacheResult == NULL && (recordType & kUGRecordTypeGroup) != 0 )
				cacheResult = HashTable_GetAndRetain( &cache->fGIDHash, idValue );
			
			if ( cacheResult == NULL )
				DbgLog( kLogInfo, "%s - Membership - Cache miss - by GID %d", reqOrigin, *((id_t *) idValue) );
			break;
			
		case ID_TYPE_SID:
			if ( sid != NULL )
			{
				// make sure unused portion of sid structure is all zeros so compares succeed.
				if ( sid->sid_authcount > KAUTH_NTSID_MAX_AUTHORITIES )
					return NULL;
				
				memset( &tempsid, 0, sizeof(tempsid) );
				memcpy( &tempsid, sid, KAUTH_NTSID_SIZE(sid) );
				sid = &tempsid;
				
				cacheResult = HashTable_GetAndRetain( &cache->fSIDHash, sid );
				if ( cacheResult == NULL && LoggingEnabled(kLogInfo) ) {
					char sidString[256];
					
					ConvertSIDToString( sidString, sid );
					DbgLog( kLogInfo, "%s - Membership - Cache miss - search by SID %s for type %X", reqOrigin, sidString, recordType );
				}
			}
			break;
			
		case ID_TYPE_USERNAME:
			if ( (recordType & kUGRecordTypeUser) != 0 )
				cacheResult = HashTable_GetAndRetain( &cache->fUserNameHash, idValue );
			if ( cacheResult == NULL && (recordType & kUGRecordTypeComputer) != 0 )
				cacheResult = HashTable_GetAndRetain( &cache->fComputerNameHash, idValue );
			
			if ( cacheResult == NULL )
				DbgLog( kLogInfo, "%s - Membership - Cache miss - by Name %s for type %X", reqOrigin, (char *)idValue, recordType );
			break;
			
		case ID_TYPE_GROUPNAME:
			if ( (recordType & kUGRecordTypeGroup) != 0 )
				cacheResult = HashTable_GetAndRetain( &cache->fGroupNameHash, idValue );
			if ( cacheResult == NULL && (recordType & kUGRecordTypeComputerGroup) != 0 )
				cacheResult = HashTable_GetAndRetain( &cache->fComputerGroupNameHash, idValue );
			
			if ( cacheResult == NULL )
				DbgLog( kLogInfo, "%s - Membership - Cache miss - by Name %s for type %X", reqOrigin, (char *)idValue, recordType );
			break;
			
		case ID_TYPE_KERBEROS:
			cacheResult = HashTable_GetAndRetain( &cache->fKerberosHash, idValue );
			break;
			
		case ID_TYPE_X509_DN:
			cacheResult = HashTable_GetAndRetain( &cache->fX509Hash, idValue );
			break;
			
		case ID_TYPE_GUID:
			cacheResult = HashTable_GetAndRetain( &cache->fGUIDHash, idValue );
			if ( cacheResult == NULL && LoggingEnabled(kLogInfo) ) {
				uuid_string_t guidString;
				
				uuid_unparse_upper( (unsigned char *)idValue, guidString );
				DbgLog( kLogInfo, "%s - Membership - Cache miss - search by GUID %s for type %X", reqOrigin, guidString, recordType );
			}
			break;
			
		default:
			DbgLog( kLogError, "%s - Membership - unknown record search requested for ID type %d", reqOrigin, idType );
			break;
	};
	
	if ( cacheResult != NULL ) {
		DbgLog( kLogDebug, "%s - Membership - Cache hit - %s (%X)", reqOrigin, (cacheResult->fName ? : "\"no name\""), cacheResult );
	}
		
	return cacheResult;
}

UserGroup *MbrdCache_AddOrUpdate( MbrdCache *cache, UserGroup *entry, uint32_t flags )
{
	if ( cache == NULL ) return NULL;

	if ( (entry->fFlags & kUGFlagHasID) != 0 && (entry->fFlags & kUGFlagHasGUID) == 0 )
	{
		uint32_t* temp = (uint32_t *) &entry->fGUID;
		
		if ( (entry->fRecordType & kUGRecordTypeUser) != 0 ) {
			temp[0] = htonl( 0xFFFFEEEE );
			temp[1] = htonl( 0xDDDDCCCC );
			temp[2] = htonl( 0xBBBBAAAA );
			temp[3] = htonl( entry->fID );
		}
		else if ( (entry->fRecordType & kUGRecordTypeComputer) != 0 ) {
			temp[0] = htonl( 0xBBBBAAAA );
			temp[1] = htonl( 0xDDDDBBBB );
			temp[2] = htonl( 0xFFFFCCCC );
			temp[3] = htonl( entry->fID );
		}
		else if ( (entry->fRecordType & kUGRecordTypeGroup) != 0 ) {
			temp[0] = htonl( 0xAAAABBBB );
			temp[1] = htonl( 0xCCCCDDDD );
			temp[2] = htonl( 0xEEEEFFFF );
			temp[3] = htonl( entry->fID );
		}
		else if ( (entry->fRecordType & kUGRecordTypeComputerGroup) != 0 ) {
			temp[0] = htonl( 0xEEEEFFFF );
			temp[1] = htonl( 0xCCCCDDDD );
			temp[2] = htonl( 0xAAAABBBB );
			temp[3] = htonl( entry->fID );
		}
		
		entry->fFlags |= kUGFlagHasGUID;
	}
	
	UserGroup *result = NULL;

	// need to hold the lock until we add, otherwise we run into race condition
	int rc = pthread_mutex_lock( &cache->fCacheLock );
	assert( rc == 0 );
	
	// all entries should always have a GUID, so always use that hash
	switch ( entry->fFoundBy ) 
	{
		case kUGFoundByNestedGroup: // all records have a GUID
		case kUGFoundByGUID:
			result = HashTable_GetAndRetain( &cache->fGUIDHash, entry->fGUID );
			break;
		
		case kUGFoundByID:
			if ( (entry->fRecordType & (kUGRecordTypeUser | kUGRecordTypeComputer)) != 0 ) {
				result = HashTable_GetAndRetain( &cache->fUIDHash, &entry->fID );
				break;
			}
			
			if ( (entry->fRecordType & kUGRecordTypeGroup) != 0 ) {
				result = HashTable_GetAndRetain( &cache->fGIDHash, &entry->fID );
			}
			break;
			
		case kUGFoundByName:
			if ( (entry->fRecordType & kUGRecordTypeUser) != 0 ) {
				result = HashTable_GetAndRetain( &cache->fUserNameHash, entry->fName );
				break;
			}
			
			if ( (entry->fRecordType & kUGRecordTypeComputer) != 0 ) {
				result = HashTable_GetAndRetain( &cache->fComputerNameHash, entry->fName );
				break;
			}
			
			if ( (entry->fRecordType & kUGRecordTypeGroup) != 0 ) {
				result = HashTable_GetAndRetain( &cache->fGroupNameHash, entry->fName );
				break;
			}

			if ( (entry->fRecordType & kUGRecordTypeComputerGroup) != 0 ) {
				result = HashTable_GetAndRetain( &cache->fComputerGroupNameHash, entry->fName );
			}
			break;
			
		case kUGFoundByX509DN:
			result = HashTable_GetAndRetain( &cache->fX509Hash, entry->fX509DN[0] );
			break;
			
		case kUGFoundByKerberos:
			result = HashTable_GetAndRetain( &cache->fKerberosHash, entry->fKerberos[0] );
			break;
	}
		
	// if the recordtype changed completely or the item is outdated, we remove the existing entry
	if ( result != NULL && result->fRecordType != entry->fRecordType )
	{
		result->fNodeAvailable = true;
		MbrdCache_RemoveEntry( cache, result );
		UserGroup_Release( result );
		result = NULL;
	}
	
	// if we still have a result we push the expiration forward
	if ( result != NULL ) {
		uint32_t secs = GetElapsedSeconds();
		
		result->fTimestamp = time( NULL );
		result->fMaximumRefresh = secs + cache->fMaximumRefresh;
		result->fExpiration = secs + ((result->fFlags & kUGFlagNotFound) != 0 ? cache->fDefaultNegativeExpiration : cache->fDefaultExpiration);

		result = MbrdCache_UpdateExistingRecord( cache, result, entry );
	}
	else {
		entry->fTimestamp = time( NULL );
		MbrdCache_AddEntry( cache, entry );
		result = entry;
	}
	
	rc = pthread_mutex_unlock( &cache->fCacheLock );
	assert( rc == 0 );
	
	return result;
}

void MbrdCache_RefreshHashes( MbrdCache *cache, UserGroup *existing )
{
	// need to hold the lock until we add, otherwise we run into race condition
	int rc = pthread_mutex_lock( &cache->fCacheLock );
	assert( rc == 0 );
	
	MbrdCache_RemoveFromHashes( cache, existing ); // remove from hashes
	MbrdCache_AddToHashes( cache, existing ); // add back to hashes
	
	rc = pthread_mutex_unlock( &cache->fCacheLock );
	assert( rc == 0 );
}

int MbrdCache_SetNodeAvailability( MbrdCache *cache, const char *nodeName, bool nodeAvailable )
{
	int iCount = 0;
	
	if ( cache == NULL ) return 0;
	
	assert( pthread_mutex_lock(&cache->fCacheLock) == 0);
	
	UserGroup* temp = cache->fListHead;
	while ( temp != NULL )
	{
		// TODO: some hash code for the name and store the hash would speed this dramatically
		if ( temp->fNodeAvailable != nodeAvailable && temp->fNode != NULL && strcmp(nodeName, temp->fNode) == 0 ) {
			__sync_bool_compare_and_swap( &temp->fNodeAvailable, temp->fNodeAvailable, nodeAvailable );
			iCount++;
		}
		temp = temp->fLink;
	}
	
	assert( pthread_mutex_unlock(&cache->fCacheLock) == 0);
	
	return iCount;
}

void MbrdCache_Sweep( MbrdCache *cache )
{
	if ( cache == NULL ) return;

	assert( pthread_mutex_lock(&cache->fCacheLock) == 0 );
	
	UserGroup* temp = cache->fListHead;
	while ( temp != NULL )
	{
		UserGroup *delItem = temp;
		
		temp = temp->fLink;
		if ( ItemOutdated(delItem, 0) == true ) {
			MbrdCache_RemoveEntry( cache, delItem );
		}
	}
	
	assert( pthread_mutex_unlock(&cache->fCacheLock) == 0 );
}

void MbrdCache_NodeChangeOccurred( MbrdCache *cache )
{
	if ( cache == NULL ) return;
	
	uint32_t currentTime = GetElapsedSeconds();
	
	assert( pthread_mutex_lock(&cache->fCacheLock) == 0);
	
	UserGroup* temp = cache->fListHead;
	while ( temp != NULL ) {
		UserGroup *delItem = temp;

		temp = temp->fLink;
		if ( (delItem->fFlags & kUGFlagNotFound) != 0 ) {
			// we delete negative entries on node changes
			MbrdCache_RemoveEntry( cache, delItem );
		}
		else {
			delItem->fExpiration = currentTime;
		}
	}
	
	assert( pthread_mutex_unlock(&cache->fCacheLock) == 0 );
}

void MbrdCache_ResetCache( MbrdCache *cache )
{
	if ( cache == NULL ) return;

	assert( pthread_mutex_lock(&cache->fCacheLock) == 0);
		
	HashTable_Reset( &cache->fGUIDHash );
	HashTable_Reset( &cache->fSIDHash );
	HashTable_Reset( &cache->fUIDHash );
	HashTable_Reset( &cache->fGIDHash );
	
	HashTable_Reset( &cache->fUserNameHash );
	HashTable_Reset( &cache->fGroupNameHash );
	HashTable_Reset( &cache->fComputerNameHash );
	HashTable_Reset( &cache->fComputerGroupNameHash );

	UserGroup* temp = cache->fListHead;
	cache->fListHead = NULL;
	cache->fListTail = NULL;
	assert( pthread_mutex_unlock(&cache->fCacheLock) == 0 );
	
	while (temp != NULL)
	{
		UserGroup *delItem = temp;
		
		temp = delItem->fLink;
		
		UserGroup_Release( delItem );
	}
}

void MbrdCache_DumpState( MbrdCache *cache )
{
	if ( cache == NULL ) return;
	
    struct stat sb;
	const char		*logName = "/Library/Logs/membership_dump.log";
	int			ii;
	
	// roll logs starting from the end
	// only keep the last 9 logs around
	for ( ii = 8; ii >= 0; ii-- )
	{
		char fileName[128];
		char newName[128];
		
		snprintf( fileName, sizeof(fileName), "%s.%d", logName, ii );
		if ( lstat(fileName, &sb) == 0 )
		{
			snprintf( newName, sizeof(newName), "%s.%d", logName, ii+1 );
			
			// first unlink any existing file and then rename current to the new
			unlink( newName );
			rename( fileName, newName );
		}
	}
	
	// now rename any existing log to /Library/Logs/membership_dump.log.0
    if ( lstat(logName, &sb) == 0 )
    {
		char newName[128];
		
		snprintf( newName, sizeof(newName), "%s.0", logName );
		
		// unlink the new name and immediately rename
		unlink( newName );
		rename( logName, newName );
    }
	
	int fd = open( logName, O_EXCL | O_CREAT | O_RDWR, 0644 );
	if ( fd == -1 )
		return;
	
	FILE* dumpFile = fdopen( fd, "w" );
	if ( dumpFile == NULL ) {
		close( fd );
		return;
	}
	
	assert( pthread_mutex_lock(&cache->fCacheLock) == 0 );
	
	fprintf( dumpFile, "Global UID count: %ld\n", cache->fUIDHash.fNumEntries );
	fprintf( dumpFile, "Global GID count: %ld\n", cache->fGIDHash.fNumEntries );
	fprintf( dumpFile, "Global GUID count: %ld\n", cache->fGUIDHash.fNumEntries );
	fprintf( dumpFile, "Global SID count: %ld\n", cache->fSIDHash.fNumEntries );
	fprintf( dumpFile, "Global User Name count: %ld\n", cache->fUserNameHash.fNumEntries );
	fprintf( dumpFile, "Global Group Name count: %ld\n", cache->fGroupNameHash.fNumEntries );
	fprintf( dumpFile, "Global Computer Name count: %ld\n", cache->fComputerNameHash.fNumEntries );
	fprintf( dumpFile, "Global ComputerGroup Name count: %ld\n", cache->fComputerGroupNameHash.fNumEntries );
	fprintf( dumpFile, "Global Kerberos count: %ld\n", cache->fKerberosHash.fNumEntries );
	fprintf( dumpFile, "Global X509DN count: %ld\n\n", cache->fX509Hash.fNumEntries );
	
	UserGroup* temp = cache->fListHead;
	while (temp != NULL)
	{
		fprintf( dumpFile, "%p: %s - %s\n", temp, UserGroup_GetRecordTypeString(temp), (temp->fName ? : "(not found)") );
		
		if ( (temp->fFlags & kUGFlagHasID) != 0 ) fprintf( dumpFile, "\tid: %d\n", temp->fID );
		if ( (temp->fFlags & kUGFlagHasGUID) != 0 ) {
			uuid_string_t guidString;
			
			uuid_unparse_upper( temp->fGUID, guidString );
			fprintf( dumpFile, "\tuuid: %s\n", guidString );	
		}
		
		if ( (temp->fFlags & kUGFlagHasSID) != 0 ) {
			char sidString[256];
			
			ConvertSIDToString( sidString, &temp->fSID );
			fprintf( dumpFile, "\tsid: %s\n", sidString );
		}
		
		for ( ii = 0; ii < kMaxAltIdentities && temp->fKerberos[ii]; ii++ ) {
			fprintf( dumpFile, "\tKerberos ID: %s\n", temp->fKerberos[ii] );	
		}
		
		for ( ii = 0; ii < kMaxAltIdentities && temp->fX509DN[ii] != NULL; ii++ ) {
			fprintf( dumpFile, "\tX509 DN: %s\n", temp->fX509DN[ii] );	
		}
		
		fprintf( dumpFile, "\tref: %d\n", temp->fRefCount);
		
		if ( temp->fNode != NULL ) {
			fprintf( dumpFile, "\tnode: '%s'\n\tstate: %s\n", temp->fNode, (temp->fNodeAvailable == true ? "online" : "offline") );
		}
		
		char timeBuffer[26];
		fprintf( dumpFile, "\ttimestamp: %s", ctime_r(&temp->fTimestamp, timeBuffer) );
		
		fprintf( dumpFile, "\tTTL: %d sec\n", MbrdCache_TTL(cache, temp, 0) );
		
		char buffer[128] = { 0, };
		fprintf( dumpFile, "\t%sfound by: %s\n", ((temp->fFlags & kUGFlagNotFound) != 0 ? "not " : ""), 
				 UserGroup_GetFoundByString(temp, buffer, sizeof(buffer)) );
		
		UserGroup **groups = NULL;
		int numResults = HashTable_CreateItemArray( &temp->fGUIDMembershipHash, &groups );
		if ( numResults > 0 )
		{
			int i;
			
			fprintf(dumpFile, "\tmember of %d groups: ", numResults);
			for (i = 0; i < numResults; i++)
			{
				if (i != 0) fprintf(dumpFile, ", ");
				if ( groups[i]->fNode != NULL ) {
					fprintf( dumpFile, "%s(%d - %p - %s - %s)", (groups[i]->fName ? :"Unknown"), groups[i]->fID, groups[i],
							groups[i]->fNode ?:"", (groups[i]->fNodeAvailable == true ? "online" : "offline") );
				}
				else {
					fprintf( dumpFile, "not found(%d - %p)", groups[i]->fID, groups[i] );
				}
				
				UserGroup_Release( groups[i] );
			}
			
			fprintf(dumpFile, "\n");
		}
		
		fprintf( dumpFile, "\n" );
		
		if ( groups != NULL ) {
			free( groups );
			groups = NULL;
		}
		
		temp = temp->fLink;
	}
	
	assert( pthread_mutex_unlock(&cache->fCacheLock) == 0 );
	
	fclose( dumpFile );
}

int32_t MbrdCache_TTL( MbrdCache *cache, UserGroup *entry, int32_t flags )
{
	if ( (flags & kKernelRequest) != 0 ) {
		return cache->fKernelExpiration;
	}

	uint32_t current = GetElapsedSeconds();
	if ( entry != NULL && current < entry->fExpiration ) {
		return entry->fExpiration - current;
	}

	// we're past our expiration time, so use 10 seconds
	return 10;
}

int32_t MbrdCache_KerberosFallback( MbrdCache *cache )
{
	return cache->fKerberosFallback;
}

#endif // DISABLE_SEARCH_PLUGIN
