/*
 * Copyright (c) 2006-2009 Apple Inc. All rights reserved.
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

#include "Mbrd_MembershipResolver.h"
#include "Mbrd_Cache.h"
#include <sys/syslog.h>
#include <libkern/OSByteOrder.h>
#include <mach/mach_error.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>			// for fcntl() and O_* flags
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <uuid/uuid.h>
#include <DirectoryServiceCore/CLog.h>
#include <DirectoryServiceCore/DSMutexSemaphore.h>
#include <DirectoryService/DirectoryService.h>
#include <DirectoryService/DirServicesConstPriv.h>
#include <assert.h>
#include <membership.h>
#include <dispatch/dispatch.h>
#include <gssapi/gssapi.h>
#include "CInternalDispatch.h"
#include "CPlugInList.h"
#include <map>
#include <string>
#include <membershipPriv.h>

#define kMaxItemsInCacheStr "MaxItemsInCache"
#define kDefaultExpirationStr "DefaultExpirationInSecs"
#define kDefaultMaximumRefreshInSecsStr "DefaultMaximumRefreshInSecs"
#define kDefaultNegativeExpirationStr "DefaultFailureExpirationInSecs"
#define kDefaultKernelExpirationInSecsStr "DefaultKernelExpirationInSecs"
#define kKerberosFallbackToRecordName "KerberosFallbackToRecordName"

#define kUUIDBlock		1
#define kSmallSIDBlock	2
#define kLargeSIDBlock	3

#define WELL_KNOWN_RID_BASE 1000

#define COMPATIBLITY_SID_PREFIX	"S-1-5-21-987654321-987654321-987654321"
#define COMPATIBLITY_SID_PREFIX_SIZE (sizeof(COMPATIBLITY_SID_PREFIX)-1)

typedef struct TempUIDCacheBlockBase
{
	struct TempUIDCacheBlockBase* fNext;
	int fKind;
	int fNumIDs;
	uid_t fStartID;
} TempUIDCacheBlockBase;

typedef struct TempUIDCacheBlockSmall
{
	struct TempUIDCacheBlock* fNext;
	int fKind;
	int fNumIDs;
	uid_t fStartID;
	uuid_t fGUIDs[1024];
} TempUIDCacheBlockSmall;

typedef struct TempUIDCacheBlockLarge
{
	struct TempUIDCacheBlock* fNext;
	int fKind;
	int fNumIDs;
	uid_t fStartID;
	ntsid_t fSIDs[1024];
} TempUIDCacheBlockLarge;


extern bool			gCacheFlushDisabled;
extern dsBool		gDSLocalOnlyMode;
extern dsBool		gDSInstallDaemonMode;
extern dsBool		gServerOS;
extern CPlugInList	*gPlugins;

static MbrdCache			*gMbrdCache		= NULL;

// keep these around, we use them a lot
static tDirReference		gMbrdDirRef		= 0;
static tDirNodeReference	gMbrdSearchNode = 0;

static tDataListPtr			gUserType		= NULL;
static tDataListPtr			gAllGroupTypes	= NULL;
static tDataListPtr			gUnknownType	= NULL;

static tDataListPtr			gAttrsToGet		= NULL;

static uuid_t				gEveryoneUUID;
static ntsid_t				gEveryoneSID;

static uuid_t				gLocalAccountsUUID;
static uuid_t				gNetAccountsUUID;
static uuid_t				gRootUserUUID;

static DSMutexSemaphore			gMbrdGlobalMutex( "::gMbrdGlobalMutex" );
static StatBlock				gStatBlock;
static TempUIDCacheBlockBase	*gUIDCache = NULL;

static map<string, string>		sidMap;
static pthread_mutex_t			sidMapLock = PTHREAD_MUTEX_INITIALIZER;	// waiting for dispatch version

static dispatch_queue_t			gLookupQueue = NULL;
static pthread_key_t			gMembershipThreadKey = NULL;

#ifndef DISABLE_CACHE_PLUGIN
extern CCachePlugin				*gCacheNode;

extern sCacheValidation* ParsePasswdEntry( tDirReference inDirRef, tDirNodeReference inNodeRef, kvbuf_t *inBuffer, tDataBufferPtr inDataBuffer, 
										   tRecordEntryPtr inRecEntry, tAttributeListRef inAttrListRef, void *additionalInfo, CCache *inCache,
										   const char **inKeys );
extern sCacheValidation* ParseGroupEntry( tDirReference inDirRef, tDirNodeReference inNodeRef, kvbuf_t *inBuffer, tDataBufferPtr inDataBuffer, 
										  tRecordEntryPtr inRecEntry, tAttributeListRef inAttrListRef, void *additionalInfo, CCache *inCache, 
										  const char **inKeys );
#endif

#pragma mark -
#pragma mark Internal Routines

static void Mbrd_AddToAverage( uint64_t* average, uint64_t* numDataPoints, uint64_t newDataPoint )
{
	gMbrdGlobalMutex.WaitLock();
	*average = (((*average) * (*numDataPoints)) + newDataPoint) / (*numDataPoints + 1);
	__sync_add_and_fetch( numDataPoints, 1 );
	gMbrdGlobalMutex.SignalLock();
}

static int IsCompatibilityGUID( uuid_t guid, int* isUser, uid_t* uid )
{
	uint32_t * temp = (uint32_t *)guid;
	int result = 0;
	if ((temp[0] == htonl(0xFFFFEEEE)) && (temp[1] == htonl(0xDDDDCCCC)) && (temp[2] == htonl(0xBBBBAAAA)))
	{
		*uid = ntohl(temp[3]);
		*isUser = 1;
		result = 1;
	}
	else if ((temp[0] == htonl(0xAAAABBBB)) && (temp[1] == htonl(0xCCCCDDDD)) && (temp[2] == htonl(0xEEEEFFFF)))
	{
		*uid = ntohl(temp[3]);
		*isUser = 0;
		result = 1;
	}
	
	return result;
}

static void* UIDCacheIndexToPointer(TempUIDCacheBlockBase* block, int inIndex)
{
	void* result;
	
	if (block->fKind == kUUIDBlock || block->fKind == kSmallSIDBlock)
	{
		TempUIDCacheBlockSmall* temp = (TempUIDCacheBlockSmall*)block;
		result = &temp->fGUIDs[inIndex];
	}
	else
	{
		TempUIDCacheBlockLarge* temp = (TempUIDCacheBlockLarge*)block;
		result = &temp->fSIDs[inIndex];
	}
	
	return result;
}

static TempUIDCacheBlockBase *Mbrd_EntryForID( void *id, int blockKind, int idSize, TempUIDCacheBlockBase **lastblock, uid_t *tempID )
{
	gMbrdGlobalMutex.WaitLock();
	
	TempUIDCacheBlockBase*	block			= gUIDCache;
	TempUIDCacheBlockBase*	returnValue		= NULL;
	
	while ( returnValue == NULL && block != NULL )
	{
		int i;
		
		if ( block->fKind == blockKind )
		{
			for ( i = 0; i < block->fNumIDs; i++ )
			{
				if ( memcmp(id, UIDCacheIndexToPointer(block, i), idSize) == 0 ) {
					(*tempID) = block->fStartID + i;
					returnValue = block;
					break;
				}
			}
		}
		
		(*lastblock) = block;
		block = block->fNext;
	}
	
	gMbrdGlobalMutex.SignalLock();
	
	return returnValue;
}

static uid_t Mbrd_CreateTempID( void *id, int blockKind, int idSize )
{
	TempUIDCacheBlockBase	*lastblock		= NULL;
	uid_t					tempID			= 0;
	TempUIDCacheBlockBase	*lasttypeblock	= Mbrd_EntryForID( id, blockKind, idSize, &lastblock, &tempID );

	if ( lasttypeblock != NULL ) {
		return tempID;
	}
	
	gMbrdGlobalMutex.WaitLock();

	if ((lasttypeblock == NULL) || (lasttypeblock->fNumIDs == 1024))
	{
		if (idSize == sizeof(uuid_t))
		{
			lasttypeblock = (TempUIDCacheBlockBase*)malloc(sizeof(TempUIDCacheBlockSmall));
			memset(lasttypeblock, 0, sizeof(TempUIDCacheBlockSmall));
		}
		else
		{
			lasttypeblock = (TempUIDCacheBlockBase*)malloc(sizeof(TempUIDCacheBlockLarge));
			memset(lasttypeblock, 0, sizeof(TempUIDCacheBlockLarge));
		}
		lasttypeblock->fKind = blockKind;
		if (lastblock != NULL)
		{
			lastblock->fNext = lasttypeblock;
			lasttypeblock->fStartID = lastblock->fStartID + 1024;
		}
		else
		{
			gUIDCache = lasttypeblock;
			lasttypeblock->fStartID = 0x82000000;
		}
	}
	
	memcpy(UIDCacheIndexToPointer(lasttypeblock, lasttypeblock->fNumIDs), id, idSize);
	lasttypeblock->fNumIDs++;
	
	tempID = lasttypeblock->fStartID + lasttypeblock->fNumIDs - 1;
	
	gMbrdGlobalMutex.SignalLock();
	
	return tempID;
}

static uid_t Mbrd_CreateTempIDForGUID( uuid_t guid )
{
	return Mbrd_CreateTempID( guid, kUUIDBlock, sizeof(guid) );
}

static uid_t Mbrd_CreateTempIDForSID( ntsid_t* sid)
{
	if (sid->sid_authcount <= 2)
		return Mbrd_CreateTempID( sid, kSmallSIDBlock, sizeof(*sid) );
	
	return Mbrd_CreateTempID( sid, kLargeSIDBlock, sizeof(*sid) );
}

static bool Mbrd_ConvertSIDFromString(const char* sidString, ntsid_t* sid)
{
	char* current = NULL;
	int count = 0;
	long long temp;
	
	memset(sid, 0, sizeof(ntsid_t));
	if (sidString[0] != 'S' || sidString[1] != '-') return false;
	
	sid->sid_kind = strtoul(sidString+2, &current, 10);
	if (*current == '\0') return false;
	current++;
	temp = strtoll(current, &current, 10);
	
	// convert to BigEndian before copying
	temp = OSSwapHostToBigInt64(temp);
	memcpy(sid->sid_authority, ((char*)&temp)+2, 6);
	while (*current != '\0')
	{
		current++;
		sid->sid_authorities[count] = strtoul(current, &current, 10);
		count++;
	}
	
	sid->sid_authcount = count;
	
	return true;
}

static UserGroup *Mbrd_AddNegative( tDataListPtr recType, int idType, const char *value, uint32_t flags ) 
{
	__sync_add_and_fetch( &gStatBlock.fNumFailedRecordLookups, 1 );
	
	UserGroup	*result		= UserGroup_Create();
	char		*endPtr		= NULL;
	id_t		theId;

	if ( recType == gAllGroupTypes ) {
		result->fRecordType = kUGRecordTypeGroup | kUGRecordTypeComputerGroup;
	}
	else if ( recType == gUserType ) {
		result->fRecordType = kUGRecordTypeUser | kUGRecordTypeComputer;
	}
	else {
		result->fRecordType = kUGRecordTypeUnknown;
	}
	
	switch ( idType )
	{
		case ID_TYPE_UID:
		case ID_TYPE_GID:
			theId = strtol( value, &endPtr, 10 );
			if ( endPtr == NULL || endPtr[0] == '\0' ) {
				result->fID = theId;
				result->fFlags = kUGFlagHasID | kUGFlagNotFound;
				result->fFoundBy = kUGFoundByID;
			}
			else {
				UserGroup_Release( result );
				result = NULL;
			}
			break;
			
		case ID_TYPE_SID:
			if ( Mbrd_ConvertSIDFromString(value, &result->fSID) ) {
				result->fFlags = kUGFlagHasSID | kUGFlagNotFound;
				result->fFoundBy = kUGFoundBySID;
				if ( (flags & kKernelRequest) != 0 ) {
					result->fID = Mbrd_CreateTempIDForSID( &result->fSID );
					result->fFlags |= kUGFlagHasID;
					result->fFoundBy |= kUGFoundByID;
				}
			}
			break;
			
		case ID_TYPE_USERNAME:
		case ID_TYPE_GROUPNAME:
			result->fName = strdup( value );
			result->fFlags = kUGFlagNotFound;
			result->fFoundBy = kUGFoundByName;
			break;
			
		case ID_TYPE_X509_DN:
			result->fX509DN[0] = strdup( value );
			result->fFlags = kUGFlagNotFound;
			result->fFoundBy = kUGFoundByX509DN;
			break;
			
		case ID_TYPE_KERBEROS:
			result->fKerberos[0] = strdup( value );
			result->fFlags = kUGFlagNotFound;
			result->fFoundBy = kUGFoundByKerberos;
			break;
			
		case ID_TYPE_GUID:
			if ( uuid_parse(value, result->fGUID) == 0 ) {
				result->fFlags = kUGFlagHasGUID | kUGFlagNotFound;
				result->fFoundBy = kUGFoundByGUID;
				
				// create a temporary ID for the kernel only
				if ( (flags & kKernelRequest) != 0 ) {
					result->fID = Mbrd_CreateTempIDForGUID( result->fGUID );
					result->fFlags |= kUGFlagHasID;
					result->fFoundBy |= kUGFoundByID;
				}
			}
			else {
				UserGroup_Release( result );
				result = NULL;
			}
			break;
			
		default:
			UserGroup_Release( result );
			result = NULL;
			break;
	}
	
	if ( result != NULL ) {
		return MbrdCache_AddOrUpdate( gMbrdCache, result, 0 );
	}

	return NULL;
}

static void ParseConfigEntry( tDirNodeReference nodeRef, tDataBufferPtr searchBuffer, UInt32 count )
{
	tAttributeValueListRef 	attributeValueListRef	= 0;
	tAttributeListRef 		attributeListRef		= 0;
	tRecordEntryPtr 		recordEntryPtr			= NULL;
	tAttributeEntryPtr 		attributeInfo			= NULL;
	tAttributeValueEntryPtr attrValue				= NULL;
	
	for ( UInt32 recIndex = 1; recIndex <= count; recIndex++ )
	{
		tDirStatus status = dsGetRecordEntry( nodeRef, searchBuffer, recIndex, &attributeListRef, &recordEntryPtr );
		if ( status == eDSNoErr )
		{
			char sidStr[MBR_MAX_SID_STRING_SIZE] = { 0 };
			char *nodeStr	= NULL;
			
			// should only be one attribute, but check anyway
			for ( UInt32 attrIndex = 1; attrIndex <= recordEntryPtr->fRecordAttributeCount; attrIndex++ )
			{
				status = dsGetAttributeEntry( nodeRef, searchBuffer, attributeListRef, attrIndex, &attributeValueListRef, &attributeInfo);									 		
				if ( status == eDSNoErr )
				{
					status = dsGetAttributeValue( nodeRef, searchBuffer, 1, attributeValueListRef, &attrValue );
					if ( status == eDSNoErr )
					{
						char *attribute = attributeInfo->fAttributeSignature.fBufferData;
						if ( strcmp(attribute, kDS1AttrXMLPlist) == 0 ) {
							CFDataRef data = CFDataCreateWithBytesNoCopy( kCFAllocatorDefault, (UInt8 *) attrValue->fAttributeValueData.fBufferData, 
																		  attrValue->fAttributeValueData.fBufferLength, kCFAllocatorNull );
							CFPropertyListRef dict = CFPropertyListCreateFromXMLData( kCFAllocatorDefault, data, kCFPropertyListImmutable, NULL );
							if ( CFGetTypeID(dict) == CFDictionaryGetTypeID() ) {
								CFStringRef sid = (CFStringRef) CFDictionaryGetValue( (CFDictionaryRef) dict, CFSTR("SID") );
								if ( sid != NULL ) {
									CFStringGetCString( sid, sidStr, sizeof(sidStr), kCFStringEncodingUTF8 );
								}
							}
							
							DSCFRelease( data );
							DSCFRelease( dict );
						}
						else if ( strcmp(attribute, kDSNAttrMetaNodeLocation) == 0 ) {
							nodeStr = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData, attrValue->fAttributeValueData.fBufferLength );
						}
						else if ( strcmp(attribute, kDS1AttrSMBSID) == 0 ) {
							if ( attrValue->fAttributeValueData.fBufferLength < MBR_MAX_SID_STRING_SIZE ) {
								bcopy( attrValue->fAttributeValueData.fBufferData, sidStr, attrValue->fAttributeValueData.fBufferLength );
								sidStr[attrValue->fAttributeValueData.fBufferLength] = '\0';
							}
						}
						
						dsDeallocAttributeValueEntry( nodeRef, attrValue );
					}
					
					dsCloseAttributeValueList( attributeValueListRef );
					dsDeallocAttributeEntry( nodeRef, attributeInfo );
				}
			}
			
			if ( sidStr[0] != '\0' && nodeStr != NULL ) {
				sidMap[nodeStr] = sidStr;
				DbgLog( kLogInfo, "Mbrd_CacheSIDSFromNode - Node: %s, SID Prefix: %s", nodeStr, sidStr );
			}

			DSFree( nodeStr );
			
			dsDeallocRecordEntry( nodeRef, recordEntryPtr );
			dsCloseAttributeList( attributeListRef );
		}
	}
}

static void Mbrd_CacheSIDSFromNode( const char *inNodeName, const char *inRecordType, const char *inRecordName, const char *inAttrType )
{
	UInt32				buffSize		= 4096;
	tDataBufferPtr		searchBuffer	= dsDataBufferAllocatePriv( buffSize );
	tDataListPtr		nodeName		= dsBuildFromPathPriv( inNodeName, "/" );
	tDataListPtr		nameList		= dsBuildListFromStringsPriv( inRecordName, NULL );
	tDataListPtr		recTypeList		= dsBuildListFromStringsPriv( inRecordType, NULL );
	tDataListPtr		attrTypeList	= dsBuildListFromStringsPriv( inAttrType, kDSNAttrMetaNodeLocation, NULL );
	UInt32				recCount		= 0;
	tDirNodeReference	nodeRef			= 0;
	tContextData		localContext	= 0;
	
	Mbrd_SetMembershipThread( true );

	tDirStatus status = dsOpenDirNode( gMbrdDirRef, nodeName, &nodeRef );
	if ( status == eDSNoErr )
	{
		do {
			do {
				status = dsGetRecordList( nodeRef, searchBuffer, nameList, eDSiExact, recTypeList, attrTypeList, 0, 
										  &recCount, &localContext );
				if ( status == eDSNoErr ) {
					ParseConfigEntry( nodeRef, searchBuffer, recCount );
				}
				else if ( status == eDSBufferTooSmall ) {
					buffSize *= 2;
					
					// a safety for a runaway condition
					if ( buffSize > 1024 * 1024 )
						break;
					
					dsDataBufferDeAllocate( gMbrdDirRef, searchBuffer );
					searchBuffer = dsDataBufferAllocate( gMbrdDirRef, buffSize );
					if ( searchBuffer == NULL ) {
						goto done;
					}
				}
				
				
			} while (((status == eDSNoErr) && (recCount == 0) && (localContext != 0)) || 
					 (status == eDSBufferTooSmall));
		} while ( localContext != 0 );
		
	done:
		if ( localContext != 0 ) {
			dsReleaseContinueData( gMbrdDirRef, localContext );
			localContext = 0;
		}
		
		dsCloseDirNode( nodeRef );
	}
	
	dsDataBufferDeallocatePriv( searchBuffer );
	
	dsDataListDeallocatePriv( nodeName );
	free( nodeName );
	
	dsDataListDeallocatePriv( nameList );
	free( nameList );
	
	dsDataListDeallocatePriv( recTypeList );
	free( recTypeList );
	
	dsDataListDeallocatePriv( attrTypeList );
	free( attrTypeList );
	
	Mbrd_SetMembershipThread( false );
}

static const char *Mbrd_GetNodenameOrSIDFromCache( const char *inNodeName, const char *inPrefix )
{
	map<string, string>::iterator	iter;
	const char						*result = NULL;
	
	// if we have no SID map, let's go get our current list
	pthread_mutex_lock( &sidMapLock );
	if ( sidMap.empty() ) {
		// TODO: search all nodes for CIFSServer
		Mbrd_CacheSIDSFromNode( "/Local/Default", kDSStdRecordTypeComputers, "localhost", kDS1AttrSMBSID );
		
		// use the same logic as the smb code
		// generate SID from the hardware UUID
		// TODO: do we want to do this, or let samba do it when it is turn on
		if ( sidMap.find("/Local/Default") == sidMap.end() ) {
			uuid_t hostuuid;
			struct timespec timeout = {0};
			
			if ( gethostuuid(hostuuid, &timeout) == 0 ) {
				ntsid_t		hostsid = { 0 };
				char		tempResult[MBR_MAX_SID_STRING_SIZE];
				
				hostsid.sid_kind = 1;
				hostsid.sid_authcount = 4;
				hostsid.sid_authority[5] = 5;
				hostsid.sid_authorities[0] = 21;
//				hostsid.sid_authorities[1] = ((uint32_t)hostuuid[0] << 24) |
//											 ((uint32_t)hostuuid[1] << 16) |
//											 ((uint32_t)hostuuid[2] << 8) |
//											 (uint32_t)hostuuid[3];
				hostsid.sid_authorities[1] = ((uint32_t)hostuuid[4] << 24) |
											 ((uint32_t)hostuuid[5] << 16) |
											 ((uint32_t)hostuuid[6] << 8) |
											 (uint32_t)hostuuid[7];
				hostsid.sid_authorities[2] = ((uint32_t)hostuuid[8] << 24) |
											 ((uint32_t)hostuuid[9] << 16) |
											 ((uint32_t)hostuuid[10] << 8)  |
											 (uint32_t)hostuuid[11];
				hostsid.sid_authorities[3] = ((uint32_t)hostuuid[12] << 24) |
											 ((uint32_t)hostuuid[13] << 16) |
											 ((uint32_t)hostuuid[14] << 8) |
											 (uint32_t)hostuuid[15];
				ConvertSIDToString( tempResult, &hostsid );
				sidMap["/Local/Default"] = tempResult;
			}
		}
		
		// now get the rest of the SIDs
		Mbrd_CacheSIDSFromNode( "/Search", kDSStdRecordTypeConfig, "CIFSServer", kDS1AttrXMLPlist );
	}

	if ( inPrefix != NULL )
	{
		// if not the compatibility prefix
		if ( strcmp(inPrefix, COMPATIBLITY_SID_PREFIX) != 0 )
		{
			for ( iter = sidMap.begin(); iter != sidMap.end(); iter++ )
			{
				if ( iter->second == inPrefix ) {
					result = iter->first.c_str();
					break;
				}
			}
		}
		else {
			// compatibility SIDs use the Search base
			result = "/Search";
		}
	}
	else if ( inNodeName != NULL )
	{
		iter = sidMap.find( inNodeName );
		if ( iter != sidMap.end() ) {
			result = iter->second.c_str();
		}
		else if ( strcmp(inNodeName, "/Local/Default") != 0 ) {
			// wasn't in map, let's look at the node specifically just in case
			Mbrd_CacheSIDSFromNode( inNodeName, kDSStdRecordTypeConfig, "CIFSServer", kDS1AttrXMLPlist );
			iter = sidMap.find( inNodeName );
			if ( iter != sidMap.end() ) {
				result = iter->second.c_str();
			}
			
			// all nodes that are not local get a "compatibility" SID prefix
			if ( result == NULL ) {
				sidMap[inNodeName] = COMPATIBLITY_SID_PREFIX;
				result = COMPATIBLITY_SID_PREFIX;
			}
		}
	}
	
	pthread_mutex_unlock( &sidMapLock );
	
	return result;
}

UserGroup **Mbrd_FindItemsAndRetain( tDirNodeReference dirNode, tDataListPtr recType, int idType, const char *value, uint32_t flags, UInt32 *recCount )
{
	UInt32 count;
	tContextData localContext = 0;
	UInt32 buffSize = 4096;
	tDataBufferPtr searchBuffer = dsDataBufferAllocate( gMbrdDirRef, buffSize );
	UserGroup* result = NULL;
	UserGroup** results = NULL;
	tDirStatus status;
	unsigned int recordIndex;
	unsigned int attrIndex;
	uint64_t microsec = GetElapsedMicroSeconds();
	uint64_t totalTime = 0;
	UInt32 totalCnt = 0;
	uint32_t foundBy = 0;
	uint64_t *statTime = &gStatBlock.fAverageuSecPerRecordLookup;
	uint64_t *statCount = &gStatBlock.fTotalRecordLookups;
	const char *attribute = NULL;
	tDirPatternMatch match = eDSExact;
	
	switch ( idType )
	{
		case ID_TYPE_UID:
			attribute = kDS1AttrUniqueID;
			foundBy = kUGFoundByID;
			break;
			
		case ID_TYPE_GID:
			attribute = kDS1AttrPrimaryGroupID;
			foundBy = kUGFoundByID;
			break;
			
		case ID_TYPE_SID:
			attribute = kDS1AttrSMBSID;
			foundBy = kUGFoundBySID;
			break;
			
		case ID_TYPE_RID:
			attribute = kDS1AttrSMBRID;
			foundBy = kUGFoundBySID;
			break;
			
		case ID_TYPE_GROUPSID:
			attribute = kDS1AttrSMBPrimaryGroupSID;
			foundBy = kUGFoundBySID;
			break;
			
		case ID_TYPE_GROUPRID:
			attribute = kDS1AttrSMBGroupRID;
			foundBy = kUGFoundBySID;
			break;
			
		case ID_TYPE_USERNAME:
		case ID_TYPE_GROUPNAME:
			attribute = kDSNAttrRecordName;
			foundBy = kUGFoundByName;
			break;
			
		case ID_TYPE_X509_DN:
			attribute = kDSNAttrAltSecurityIdentities;
			foundBy = kUGFoundByX509DN;
			break;
			
		case ID_TYPE_KERBEROS:
			attribute = kDSNAttrAltSecurityIdentities;
			foundBy = kUGFoundByKerberos;
			break;
			
		case ID_TYPE_GUID:
			attribute = kDS1AttrGeneratedUID;
			foundBy = kUGFoundByGUID;
			match = eDSiExact;
			break;
			
		case ID_TYPE_GROUPMEMBERS:
			attribute = kDSNAttrGroupMembers;
			foundBy = kUGFoundByNestedGroup;
			statTime = &gStatBlock.fAverageuSecPerGUIDMemberSearch;
			statCount = &gStatBlock.fTotalGUIDMemberSearches;			
			break;
			
		case ID_TYPE_GROUPMEMBERSHIP:
			attribute = kDSNAttrGroupMembership;
			foundBy = kUGFoundByNestedGroup;
			statTime = &gStatBlock.fAverageuSecPerLegacySearch;
			statCount = &gStatBlock.fTotalLegacySearches;			
			break;
			
		case ID_TYPE_NESTEDGROUPS:
			attribute = kDSNAttrNestedGroups;
			foundBy = kUGFoundByNestedGroup;
			statTime = &gStatBlock.fAverageuSecPerNestedMemberSearch;
			statCount = &gStatBlock.fTotalNestedMemberSearches;
			break;
	}
	
	tDataNodePtr attrType = dsDataNodeAllocateString( gMbrdDirRef, attribute );
	tDataNodePtr lookUpPtr = dsDataNodeAllocateString( gMbrdDirRef, value );
	
	Mbrd_SetMembershipThread( true );

	do {
		do {
			count = (*recCount);
			status = dsDoAttributeValueSearchWithData(dirNode, searchBuffer, recType,
													  attrType, match, lookUpPtr, gAttrsToGet, 0,
													  &count, &localContext);
			if (status == eDSBufferTooSmall) {
				buffSize *= 2;
				
				// a safety for a runaway condition
				if ( buffSize > 1024 * 1024 )
					break;
				
				dsDataBufferDeAllocate( gMbrdDirRef, searchBuffer );
				searchBuffer = dsDataBufferAllocate( gMbrdDirRef, buffSize );
				if ( searchBuffer == NULL )
					status = eMemoryError;
			}
		} while (((status == eDSNoErr) && (recCount == 0) && (localContext != 0)) || 
				 (status == eDSBufferTooSmall));
		
		if ((status == eDSNoErr) && (recCount <= 0))
			break;
		
		// this can happen if policy changed mid query, let's try to recover
		if ( status == eDSInvalidContinueData || status == eDSBadContextData )
		{
			localContext = 0;
			continue;
		}
		else if (status != eDSNoErr)
		{
			break;
		}
		
		if ( count > 0 ) {
			if ( results != NULL ) {
				results = (UserGroup **) reallocf( results, (totalCnt + count) * sizeof(UserGroup *) );
			}
			else {
				results = (UserGroup **) malloc( count * sizeof(UserGroup *) );
			}
		}
		
		for (recordIndex = 1; recordIndex <= count; ++recordIndex)
		{
			tAttributeValueListRef 	attributeValueListRef = 0;
			tAttributeListRef 		attributeListRef = 0;
			tRecordEntryPtr 		recordEntryPtr = NULL;
			tAttributeEntryPtr 		attributeInfo = NULL;
			tAttributeValueEntryPtr attrValue = NULL;
			
			status = dsGetRecordEntry(dirNode, searchBuffer, recordIndex, &attributeListRef, &recordEntryPtr);
			if (status == eDSNoErr)
			{
				char		*recTypeStr	= NULL;
				bool		serviceAccount = false;
				
				status = dsGetRecordTypeFromEntry( recordEntryPtr, &recTypeStr );
				if ( status == eDSNoErr )
				{
					result = UserGroup_Create();
					if ( strcmp(recTypeStr, kDSStdRecordTypeUsers) == 0 ) {
#ifndef DISABLE_CACHE_PLUGIN
						const char	*keys[] = { "pw_name", "pw_uid", "pw_gecos", NULL }; // "pw_uuid"
						sCacheValidation *valid;
						
						result->fRecordType = kUGRecordTypeUser;
						
						if ( gCacheNode != NULL ) {
							valid = ParsePasswdEntry( gMbrdDirRef, dirNode, NULL, searchBuffer, recordEntryPtr, attributeListRef, NULL, 
													  gCacheNode->fLibinfoCache, keys );
							DSRelease( valid );
						}
#endif
					}
					else if ( strcmp(recTypeStr, kDSStdRecordTypeComputers) == 0 ) {
						result->fRecordType = kUGRecordTypeComputer;
					}
					else if ( strcmp(recTypeStr, kDSStdRecordTypeComputerGroups) == 0 ) {
						result->fRecordType = kUGRecordTypeComputerGroup;
					}
					else {
#ifndef DISABLE_CACHE_PLUGIN
						const char	*keys[] = { "gr_name", "gr_gid", "gr_uuid", NULL };
						sCacheValidation *valid;
						
						result->fRecordType = kUGRecordTypeGroup;
						
						if ( gCacheNode != NULL ) {
							valid = ParseGroupEntry( gMbrdDirRef, dirNode, NULL, searchBuffer, recordEntryPtr, attributeListRef,
													 NULL, gCacheNode->fLibinfoCache, keys );
							DSRelease( valid );
						}
#endif
					}
					
					result->fFoundBy = foundBy;
				}
				else
				{
					free( recTypeStr );
					continue;
				}
				
				free(recTypeStr);
				
				char *smbRID = NULL;
				char *smbGroupRID = NULL;
				char *smbPrimaryGroupSID = NULL;
				char *origHome = NULL;
				bool bWasSetByCopyTimestamp = false;
				for (attrIndex = 1; attrIndex <= recordEntryPtr->fRecordAttributeCount; ++attrIndex)
				{
					status = dsGetAttributeEntry(dirNode, searchBuffer, attributeListRef, 
												 attrIndex, &attributeValueListRef, &attributeInfo);									 		
					if (status == eDSNoErr)
					{
						status = dsGetAttributeValue(dirNode, searchBuffer, 1, attributeValueListRef, &attrValue);	
						if (status == eDSNoErr)
						{
							// TODO: need to make this more efficient instead of scanning each one
							char* attrName = attributeInfo->fAttributeSignature.fBufferData;
							if (strcmp(attrName, kDSNAttrRecordName) == 0)
							{
								result->fName = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
																	  attrValue->fAttributeValueData.fBufferLength );
								result->fFlags |= kUGFlagHasName;
							}
							else if (strcmp(attrName, kDSNAttrAltSecurityIdentities) == 0 )
							{
								int		iKerb	= 0;
								int		iX509	= 0;
								UInt32	index	= 1;
								do
								{
									if ( attrValue->fAttributeValueData.fBufferLength > sizeof("Kerberos:") &&
										 strncasecmp(attrValue->fAttributeValueData.fBufferData, "Kerberos:", sizeof("Kerberos:")-1) == 0 )
									{
										if ( iKerb < kMaxAltIdentities ) {
											result->fKerberos[iKerb] = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData+(sizeof("Kerberos:")-1),
																							 attrValue->fAttributeValueData.fBufferLength-(sizeof("Kerberos:")-1) );
											iKerb++;
											result->fFlags |= kUGFlagHasKerberos;
										}
										else {
											DbgLog( kLogError, "Mbrd_FindItemsAndRetain - more than '%d' Kerberos identities '%s' for user is not supported", 
												    kMaxAltIdentities, attrValue->fAttributeValueData.fBufferData );
										}
									}
									else if ( attrValue->fAttributeValueData.fBufferLength > sizeof("X509:") &&
											  strncasecmp(attrValue->fAttributeValueData.fBufferData, "X509:", sizeof("X509:")-1) == 0 )
									{
										if ( iX509 < kMaxAltIdentities ) {
											result->fX509DN[iX509] = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData+(sizeof("X509:")-1),
																						   attrValue->fAttributeValueData.fBufferLength-(sizeof("X509:")-1) );
											iX509++;
											result->fFlags |= kUGFlagHasX509DN;
										}
										else {
											DbgLog( kLogError, "Mbrd_FindItemsAndRetain - more than '%d' X509 identities '%s' for user is not supported", 
												    kMaxAltIdentities, attrValue->fAttributeValueData.fBufferData );
										}
									}
									
									if ( ++index <= attributeInfo->fAttributeValueCount ) {
										if ( attrValue != NULL ) {
											dsDeallocAttributeValueEntry( gMbrdDirRef, attrValue );
											attrValue = NULL;
										}
										status = dsGetAttributeValue( dirNode, searchBuffer, index, attributeValueListRef, &attrValue );
										continue;
									}
									
									break;
								} while ( 1 );
							}
							else if (strcmp(attrName, kDS1AttrGeneratedUID) == 0)
							{
								if ( attrValue->fAttributeValueData.fBufferLength >= sizeof(uuid_t) ) {
									uuid_parse( attrValue->fAttributeValueData.fBufferData, result->fGUID );
									result->fFlags |= kUGFlagHasGUID;
								}
							}
							else if (strcmp(attrName, kDS1AttrSMBRID) == 0)
							{
								smbRID = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
															   attrValue->fAttributeValueData.fBufferLength );
							}
							else if (strcmp(attrName, kDS1AttrSMBGroupRID) == 0)
							{
								smbGroupRID = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
																    attrValue->fAttributeValueData.fBufferLength );
							}
							else if (strcmp(attrName, kDS1AttrSMBSID) == 0)
							{					 
								char *temp = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
																   attrValue->fAttributeValueData.fBufferLength );
								if ( Mbrd_ConvertSIDFromString(temp, &result->fSID) ) {
									result->fFlags |= kUGFlagHasSID;
								}
								DSFree( temp );
							}
							else if (strcmp(attrName, kDS1AttrUniqueID) == 0)
							{					 
								char *temp = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
																   attrValue->fAttributeValueData.fBufferLength );
								char *endPtr = NULL;
								int num = strtol(temp, &endPtr, 10);
								if ( endPtr == NULL || (*endPtr) == '\0' ) {
									result->fID = num;
									result->fFlags |= kUGFlagHasID;
								}
								
								DSFree( temp );
							}
							else if (strcmp(attrName, kDS1AttrTimeToLive) == 0)
							{
								int multiplier = 1;
								char* endPtr = NULL; 
								char *temp = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
																   attrValue->fAttributeValueData.fBufferLength );
								int num = strtol(temp, &endPtr, 10);
								switch (*endPtr)
								{
									case 's':
										multiplier = 1;
										break;
									case 'm':
										multiplier = 60;
										break;
									case 'h':
										multiplier = 60 * 60;
										break;
									case 'd':
										multiplier = 60 * 60 * 24;
										break;
								}
								
								DSFree( temp );
								
								result->fExpiration = GetElapsedSeconds() + num * multiplier;
							}
							else if (strcmp(attrName, kDS1AttrPrimaryGroupID) == 0)
							{					 
								char *temp = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
																   attrValue->fAttributeValueData.fBufferLength );
								char* endPtr = NULL; 
								int num = strtol( temp, &endPtr, 10 );
								if ( endPtr == NULL || endPtr[0] == '\0' )
								{
									if ( (result->fRecordType & (kUGRecordTypeUser | kUGRecordTypeComputer)) != 0  ) {
										result->fPrimaryGroup = num;
									}
									else {
										result->fID = num;
										result->fFlags |= kUGFlagHasID;
									}
								}
								
								DSFree( temp );
							}
							else if ( strcmp(attrName, kDSNAttrMetaNodeLocation) == 0 )
							{
								char *temp = result->fNode = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
																				   attrValue->fAttributeValueData.fBufferLength );
								
								if ( bWasSetByCopyTimestamp == false && (strcmp(temp, "/Local/Default") == 0 || strcmp(temp, "/BSD/local") == 0) )
									result->fFlags |= kUGFlagLocalAccount;
								
								char *nodeName = strtok( attrValue->fAttributeValueData.fBufferData, "/" );
								if ( nodeName != NULL ) {
									result->fToken = gPlugins->GetValidDataStamp( nodeName );
								}
								result->fNodeAvailable = true;
								
								temp = NULL; // don't free because fNode is owned
							}
							else if ( strcmp(attrName, kDS1AttrCopyTimestamp) == 0 && attrValue->fAttributeValueData.fBufferLength > 0 )
							{
								// if the account has a copyTimeStamp it is not local so we flag it as remote
								result->fFlags &= ~kUGFlagLocalAccount;
								bWasSetByCopyTimestamp = true; // save this because attr order is not guaranteed
							}
							else if ( strcmp(attrName, kDS1AttrSMBPrimaryGroupSID) == 0 ) {
								smbPrimaryGroupSID = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
																		   attrValue->fAttributeValueData.fBufferLength );
							}
							else if ( strcmp(attrName, kDS1AttrOriginalNodeName) == 0 ) {
								origHome = dsCStrFromCharacters( attrValue->fAttributeValueData.fBufferData,
																 attrValue->fAttributeValueData.fBufferLength );
							}
							else if ( strcmp(attrName, kDSNAttrKeywords) == 0 ) {
								UInt32 index = 1;
								do {
									if ( strcmp(attrValue->fAttributeValueData.fBufferData, "com.apple.ServiceAccount") == 0 ) {
										serviceAccount = true;
										break;
									}

									if ( ++index <= attributeInfo->fAttributeValueCount ) {
										if ( attrValue != NULL ) {
											dsDeallocAttributeValueEntry( gMbrdDirRef, attrValue );
											attrValue = NULL;
										}
										status = dsGetAttributeValue( dirNode, searchBuffer, index, attributeValueListRef, &attrValue );
										continue;
									}
									
									break;
								} while (1);
							}
							
							if ( attrValue != NULL ) {
								dsDeallocAttributeValueEntry(gMbrdDirRef, attrValue);	
							}
						}
						
						dsDeallocAttributeEntry(gMbrdDirRef, attributeInfo);	
						dsCloseAttributeValueList(attributeValueListRef);
					}
				}
				
				dsDeallocRecordEntry(gMbrdDirRef, recordEntryPtr);
				dsCloseAttributeList(attributeListRef);
				
				// if we don't have a sid but have one of the RIDs, let's build the SID for the entry
				if ( (result->fFlags & kUGFlagHasSID) == 0 ) {
					
					const char *sidPrefix = Mbrd_GetNodenameOrSIDFromCache( (origHome ? : result->fNode), NULL );
					if ( sidPrefix != NULL ) {
						void (^calcSID)(const char *) = ^(const char *sidAttr) {
							if ( sidAttr == NULL ) return;
							
							char sidStr[MBR_MAX_SID_STRING_SIZE];
							
							strlcpy( sidStr, sidPrefix, sizeof(sidStr) );
							strlcat( sidStr, "-", sizeof(sidStr) );
							strlcat( sidStr, sidAttr, sizeof(sidStr) );
							
							if ( Mbrd_ConvertSIDFromString(sidStr, &result->fSID) ) {
								result->fFlags |= kUGFlagHasSID;
							}
						};
						
						if ( (result->fRecordType & (kUGRecordTypeUser | kUGRecordTypeComputer)) != 0 ) {
							calcSID( smbRID );
						}
						else if ( (result->fRecordType & kUGRecordTypeGroup) != 0 ) {
							if ( smbPrimaryGroupSID != NULL ) {
								if ( Mbrd_ConvertSIDFromString(smbPrimaryGroupSID, &result->fSID) ) {
									result->fFlags |= kUGFlagHasSID;
								}
							} else {
								calcSID( smbGroupRID );
								if ( (result->fFlags & kUGFlagHasSID) == 0 ) {
									calcSID( smbRID );
								}
							}
						}
						
						// last resort, use the algorithmic SID
						if ( (result->fFlags & kUGFlagHasSID) == 0 ) {
							char ridStr[16];
							
							uint32_t rid = ((result->fID << 1) + WELL_KNOWN_RID_BASE) | 
											((result->fRecordType & (kUGRecordTypeUser | kUGRecordTypeComputer)) != 0 ? 0 : 1);
							snprintf( ridStr, sizeof(ridStr), "%u", rid );
							calcSID( ridStr );
						}						
					}
				}
				
				DSFree( smbRID );
				DSFree( smbGroupRID );
				DSFree( smbPrimaryGroupSID );
				DSFree( origHome );
				
				// we can skip the temporary ID creation if this is a service account
				if ( serviceAccount == true ) {
					UserGroup_Release( result );
					result = NULL;
					continue;
				}
				
				// if we don't have an ID but we have a GUID, then we need transient one, unless it is a computer group
				// but only if asked by the kernel
				if ( (flags & kKernelRequest) != 0 &&
					 result->fRecordType != kUGRecordTypeComputerGroup && 
					 (result->fFlags & kUGFlagHasID) == 0 && 
					 (result->fFlags & kUGFlagHasGUID) != 0 )
				{
					result->fID = Mbrd_CreateTempIDForGUID( result->fGUID );
					result->fFlags |= kUGFlagHasID;
					result->fFoundBy |= kUGFoundByID; // since transient ID it'll never be found
				}				
				
				UserGroup *temp = MbrdCache_AddOrUpdate( gMbrdCache, result, flags );
				if ( temp != NULL ) {
					results[totalCnt++] = temp;
					temp = NULL;
				}
			}
		}
	} while ( localContext != 0 || status == eDSInvalidContinueData || status == eDSBadContextData );
	
	if ( localContext != 0 ) {
		dsReleaseContinueData( gMbrdDirRef, localContext );
		localContext = 0;
	}
	
	Mbrd_SetMembershipThread( false );

	dsDataBufferDeAllocate( gMbrdDirRef, searchBuffer );
	dsDataNodeDeAllocate( gMbrdDirRef, attrType );
	dsDataNodeDeAllocate( gMbrdDirRef, lookUpPtr );
	
	totalTime += GetElapsedMicroSeconds() - microsec;
	
	Mbrd_AddToAverage( statTime, statCount, totalTime );
	
	(*recCount) = totalCnt;
	
	return results;
}

static UserGroup *Mbrd_FindItemAndRetain( tDirNodeReference dirNode, tDataListPtr recType, int idType, const char *value, uint32_t flags )
{
	UInt32		count	= 1;
	UserGroup	**items = Mbrd_FindItemsAndRetain( dirNode, recType, idType, value, flags, &count );
	UserGroup	*result	= NULL;
	
	if ( count > 0 ) {
		result = items[0];
		items[0] = NULL;
		
		for ( UInt32 ii = 1; ii < count; ii++ ) {
			UserGroup_Release( items[ii] );
			items[ii] = NULL;
		}
		
		free( items );
		items = NULL;
	}
	
	// if we didn't find it we add a negative entry
	if ( result == NULL && (flags & kNoNegativeEntry) == 0 ) {
		result = Mbrd_AddNegative( recType, idType, value, flags );
	}

	return result;
}

static UserGroup *__Mbrd_FindItemWithIdentifierAndRetain( UserGroup *origItem, int idType, const char *identifier, int32_t flags )
{
	UserGroup *item = NULL;
	
	switch ( idType )
	{
		case ID_TYPE_UID:
			item = Mbrd_FindItemAndRetain( gMbrdSearchNode, gUserType, idType, identifier, flags );
			break;
			
		case ID_TYPE_USERNAME:
		case ID_TYPE_X509_DN:
		case ID_TYPE_KERBEROS:
			item = Mbrd_FindItemAndRetain( gMbrdSearchNode, gUserType, idType, identifier, flags );
			break;
			
		case ID_TYPE_GID:
			item = Mbrd_FindItemAndRetain( gMbrdSearchNode, gAllGroupTypes, idType, identifier, flags );
			break;
			
		case ID_TYPE_GROUPNAME:
			item = Mbrd_FindItemAndRetain( gMbrdSearchNode, gAllGroupTypes, idType, identifier, flags );
			break;
			
		case ID_TYPE_SID:
			// if not the compatibility prefix we will search for it directly first
			if ( strncmp(identifier, COMPATIBLITY_SID_PREFIX, COMPATIBLITY_SID_PREFIX_SIZE) != 0 ) {
				item = Mbrd_FindItemAndRetain( gMbrdSearchNode, gUnknownType, idType, identifier, flags | kNoNegativeEntry );
				if ( item == NULL ) {
					// TODO: can users and groups have GROUPSID, according to some notes, yes
					item = Mbrd_FindItemAndRetain( gMbrdSearchNode, gUnknownType, ID_TYPE_GROUPSID, identifier, flags | kNoNegativeEntry );
				}
			}
			
			// if we were searching for SID we might have to go a different route as backup
			if ( item == NULL && origItem == NULL ) {
				ntsid_t expectedSID;
				char *rid;

				Mbrd_ConvertSIDFromString( identifier, &expectedSID );
				rid = strrchr( identifier, '-' );
				if ( rid != NULL ) {
					(*rid) = '\0';
					rid++;
					
					const char *nodeName = Mbrd_GetNodenameOrSIDFromCache( NULL, identifier );
					if ( nodeName != NULL ) {
						tDirNodeReference dirNode = 0;
						tDataListPtr dirNodeName = dsBuildFromPathPriv( nodeName, "/" );
						
						if ( dsOpenDirNode(gMbrdDirRef, dirNodeName, &dirNode) == eDSNoErr ) {
							item = Mbrd_FindItemAndRetain( dirNode, gUnknownType, ID_TYPE_RID, rid, flags | kNoNegativeEntry );
							if ( item == NULL ) {
								item = Mbrd_FindItemAndRetain( dirNode, gUnknownType, ID_TYPE_GROUPRID, rid, flags | kNoNegativeEntry );
							}
							
							uint32_t ridInt = strtoul( rid, NULL, 10 );
							if ( item == NULL && ridInt >= WELL_KNOWN_RID_BASE ) {
								char ridStr[16];
								
								uint32_t uid = (ridInt - 1000) >> 1;
								snprintf( ridStr, sizeof(ridStr), "%u", uid );
								
								if ( (ridInt & 1) == 0 ) {
									item = Mbrd_FindItemAndRetain( dirNode, gUserType, ID_TYPE_UID, ridStr, flags );
								}
								else {
									item = Mbrd_FindItemAndRetain( dirNode, gAllGroupTypes, ID_TYPE_GID, ridStr, flags );
								}
							}
							
							// now ensure it's the SID we expected
							if ( item != NULL ) {
								if ( memcmp(&expectedSID, &item->fSID, sizeof(ntsid_t)) == 0 ) {
									item->fFoundBy |= kUGFoundBySID; // ensure it's flagged as found by SID
								}
								else {
									UserGroup_Release( item );
									item = NULL;
								}
							}
							
							dsCloseDirNode( dirNode );
							dirNode = 0;
						}
						
						dsDataListDeallocatePriv( dirNodeName );
						free( dirNodeName );
						dirNodeName = NULL;
					}
					
					// restore the hyphen
					*(--rid) = '-';
				}
			}
			
			// all failed, now create a negative answer
			if ( item == NULL && origItem == NULL ) {
				item = Mbrd_AddNegative( gUnknownType, idType, identifier, flags );
			}
			break;
			
		case ID_TYPE_GUID:
			item = Mbrd_FindItemAndRetain( gMbrdSearchNode, gUnknownType, ID_TYPE_GUID, identifier, flags );
			break;				
	}
	
	return item;
}

static UserGroup* __Mbrd_GetItemWithIdentifierAndRetain( MbrdCache *cache, int idType, const void *identifier, int32_t flags )
{
	if ( cache == NULL || identifier == NULL ) return NULL;

	const char *reqOrigin = ((flags & kKernelRequest) != 0 ? "mbr_syscall" : "mbr_mig");
	UserGroup *item = NULL;
	int32_t foundBy = 0;
	id_t theID;
	int isUser;
	guid_t *guid = (guid_t *) identifier;
	const char *stringVal = (char *) identifier;
	tDataListPtr recType = NULL;
	char *(^copyIdentifierAsString)(int, const void *) = ^(int theType, const void *theIdentifier) {
		char *returnValue = NULL;
		
		switch ( theType )
		{
			case ID_TYPE_UID:
			case ID_TYPE_GID:
				asprintf( &returnValue, "%d", *((id_t *) theIdentifier) );
				break;
				
			case ID_TYPE_USERNAME:
			case ID_TYPE_GROUPNAME:
			case ID_TYPE_X509_DN:
			case ID_TYPE_KERBEROS:
				returnValue = strdup( (char *) theIdentifier );
				break;
				
			case ID_TYPE_SID:
				returnValue = (char *) calloc( MBR_MAX_SID_STRING_SIZE, sizeof(char) );
				ConvertSIDToString( returnValue, (ntsid_t *) theIdentifier );
				break;
				
			case ID_TYPE_GUID:
				returnValue = (char *) calloc( 1, sizeof(uuid_string_t) );
				uuid_unparse_upper( (unsigned char *) theIdentifier, returnValue );
				break;				
		}
		
		return returnValue;
	};

	switch ( idType )
	{
		case ID_TYPE_UID:
		case ID_TYPE_USERNAME:
			item = MbrdCache_GetAndRetain( cache, kUGRecordTypeUser | kUGRecordTypeComputer, idType, identifier, flags );
			recType = gUserType;
			break;
			
		case ID_TYPE_X509_DN:
			// cache doesn't have the prefix
			item = MbrdCache_GetAndRetain( cache, kUGRecordTypeUser, idType, stringVal + (sizeof("X509:")-1), flags );
			break;
			
		case ID_TYPE_KERBEROS:
			// cache doesn't have the prefix
			item = MbrdCache_GetAndRetain( cache, kUGRecordTypeUser, idType, stringVal + (sizeof("Kerberos:")-1), flags );
			recType = gUserType;
			break;
			
		case ID_TYPE_GID:
		case ID_TYPE_GROUPNAME:
			item = MbrdCache_GetAndRetain( cache, kUGRecordTypeGroup | kUGRecordTypeComputerGroup, idType, identifier, flags );
			recType = gAllGroupTypes;
			break;
			
		case ID_TYPE_SID:
			item = MbrdCache_GetAndRetain( cache, kUGRecordTypeUnknown, idType, identifier, flags );
			recType = gUnknownType;
			break;
			
		case ID_TYPE_GUID:
			if ( IsCompatibilityGUID((unsigned char *) guid, &isUser, &theID) == true ) {
				if ( isUser ) {
					item = MbrdCache_GetAndRetain( cache, kUGRecordTypeUser | kUGRecordTypeComputer, ID_TYPE_UID, &theID, flags );
					idType = ID_TYPE_UID;
				}
				else {
					item = MbrdCache_GetAndRetain( cache, kUGRecordTypeGroup | kUGRecordTypeComputerGroup, ID_TYPE_GID, &theID, flags );
					idType = ID_TYPE_GID;
				}
				
				identifier = &theID;
				DbgLog( kLogInfo, "%s - Membership - Compatibility GUID detected, switched to UID/GID", reqOrigin );
			}
			else {
				item = MbrdCache_GetAndRetain( cache, kUGRecordTypeUnknown, idType, guid, flags );
			}
			recType = gUnknownType;
			break;
			
		default:
			recType = gUnknownType;
			break;
	}
	
	// now check if it was found by the key we expected, if not we need to search again (asynchronously)
	if ( item != NULL )
	{
		bool bAuthoritative = true;
		bool bSearchAgain = false;
		
		switch ( idType )
		{
			case ID_TYPE_UID:
			case ID_TYPE_GID:
				foundBy = kUGFoundByID;
				bAuthoritative = ((item->fFoundBy & kUGFoundByID) == 0);
				bSearchAgain = ((__sync_fetch_and_or(&item->fFoundBy, kUGFoundByIDSched) & kUGFoundByIDSched) == 0);
				if ( bAuthoritative )
					DbgLog( kLogInfo, "%s - Membership - Cache entry was not found by ID (non-authoritative)", reqOrigin );
				break;
				
			case ID_TYPE_USERNAME:
			case ID_TYPE_GROUPNAME:
				foundBy = kUGFoundByName;
				bAuthoritative = ((item->fFoundBy & kUGFoundByName) == 0);
				bSearchAgain = ((__sync_fetch_and_or(&item->fFoundBy, kUGFoundByNameSched) & kUGFoundByNameSched) == 0);
				if ( bAuthoritative )
					DbgLog( kLogInfo, "%s - Membership - Cache entry was not found by Name (non-authoritative)", reqOrigin );
				break;
				
			case ID_TYPE_X509_DN:
				foundBy = kUGFoundByX509DN;
				bAuthoritative = ((item->fFoundBy & kUGFoundByX509DN) == 0);
				bSearchAgain = ((__sync_fetch_and_or(&item->fFoundBy, kUGFoundByX509DNSched) & kUGFoundByX509DNSched) == 0);
				if ( bAuthoritative )
					DbgLog( kLogInfo, "%s - Membership - Cache entry was not found by X509DN (non-authoritative)", reqOrigin );
				break;
				
			case ID_TYPE_KERBEROS:
				foundBy = kUGFoundByKerberos;
				bAuthoritative = ((item->fFoundBy & kUGFoundByKerberos) == 0);
				bSearchAgain = ((__sync_fetch_and_or(&item->fFoundBy, kUGFoundByKerberosSched) & kUGFoundByKerberosSched) == 0);
				if ( bAuthoritative )
					DbgLog( kLogInfo, "%s - Membership - Cache entry was not found by KerberosID (non-authoritative)", reqOrigin );
				break;
				
			case ID_TYPE_SID:
				foundBy = kUGFoundBySID;
				bAuthoritative = ((item->fFoundBy & kUGFoundBySID) == 0);
				bSearchAgain = ((__sync_fetch_and_or(&item->fFoundBy, kUGFoundBySIDSched) & kUGFoundBySIDSched) == 0);
				if ( bAuthoritative )
					DbgLog( kLogInfo, "%s - Membership - Cache entry was not found by SID (non-authoritative)", reqOrigin );
				break;
				
			case ID_TYPE_GUID:
				foundBy = kUGFoundByGUID;
				bAuthoritative = ((item->fFoundBy & kUGFoundByGUID) == 0);
				bSearchAgain = ((__sync_fetch_and_or(&item->fFoundBy, kUGFoundByGUIDSched) & kUGFoundByGUIDSched) == 0);
				if ( bAuthoritative )
					DbgLog( kLogInfo, "%s - Membership - Cache entry was not found by GUID (non-authoritative)", reqOrigin );
				break;
		}
	
		if ( bAuthoritative == false ) {
			// if was a positive result keep it just in case
			if ( item != NULL && (item->fFlags & kUGFlagNotFound) == 0 ) {
				flags |= kNoNegativeEntry;
			}
		}
		
		// if it has reserved IDs, but is not builtin, then we cannot return it as authoritative for Group ID, SID and Name
		if ( (item->fFlags & kUGFlagIsBuiltin) == 0 ) {
			bool	bUsable = true; // assume usable
			
			switch ( idType )
			{
				case ID_TYPE_GID:
					if ( (item->fFlags & kUGFlagReservedID) != 0 ) {
						bUsable = false;
					}
					break;
					
				case ID_TYPE_SID:
					if ( (item->fFlags & kUGFlagReservedSID) != 0 ) {
						bUsable = false;
					}
					break;
					
				case ID_TYPE_GROUPNAME:
					if ( (item->fFlags & kUGFlagReservedName) != 0 ) {
						bUsable = false;
					}
					break;
			}
			
			if ( bUsable == false ) {
				DbgLog( kLogInfo, "%s - Membership - Not using cached entry '%s' cause it is a reserved ID/Name/SID but was not local", 
					    reqOrigin, (item->fName ? item->fName : "") );
				UserGroup_Release( item );
				item = NULL;
				bSearchAgain = false;
			}
		}
		
		// issue a search again to find conflicts asynchronously
		if ( bSearchAgain == true ) {
			UserGroup *origItem = UserGroup_Retain( item );
			char *phID = copyIdentifierAsString( idType, identifier );
			
			dispatch_async( item->fQueue,
						    ^(void) {
								CInternalDispatch::AddCapability();
								UserGroup *tempItem = __Mbrd_FindItemWithIdentifierAndRetain( origItem, idType, phID, flags );
								if ( tempItem != NULL ) {
									UserGroup_Release( tempItem );
								}
								
								UserGroup_Release( origItem );
								free( phID );
							} );
		}
	}
	
	if ( item != NULL ) {
		__sync_add_and_fetch( &gStatBlock.fCacheHits, 1 );
	}
	else {
		char *phID = copyIdentifierAsString( idType, identifier );

		__sync_add_and_fetch( &gStatBlock.fCacheMisses, 1 );
		item = __Mbrd_FindItemWithIdentifierAndRetain( NULL, idType, phID, flags & ~kNoNegativeEntry );
		
		DSFree( phID );
	}
	
	// if not found and kKernelRequest is not set then came from userspace don't return the entry
	// but if it is set, we do return it because kernel always gets a conversion when it is requested
	// TODO: need to confirm this with Kernel folks (but this was the expected design)
	if ( item != NULL && (item->fFlags & kUGFlagNotFound) != 0 ) {
		if ( (flags & kKernelRequest) == 0 ) {
			DbgLog( kLogInfo, "%s - Membership - Cache entry is a negative entry (discarding result)", reqOrigin );
			UserGroup_Release( item );
			item = NULL;
		}
		else {
			int rc = pthread_mutex_lock( &item->fMutex );
			assert( rc == 0 );
			
			// if the kernel requests translation, but the current entry doesn't have an ID, add one
			if ( (item->fFlags & kUGFlagHasID) == 0 ) {
				// if we don't have an ID yet, we need to assign one for the kernel
				if ( (item->fFlags & kUGFlagHasGUID) != 0 ) {
					item->fID = Mbrd_CreateTempIDForGUID( item->fGUID );
				}
				else if ( (item->fFlags & kUGFlagHasSID) != 0 ) {
					item->fID = Mbrd_CreateTempIDForSID( &item->fSID );
				}
				
				item->fFlags |= kUGFlagHasID;
				
				// no item locks are held by the cache for this, so we can do while holding the item lock (no lock inversion)
				MbrdCache_RefreshHashes( gMbrdCache, item );
			}
			
			rc = pthread_mutex_unlock( &item->fMutex );
			assert( rc == 0 );
		}
	}
	
	return item;
}

// we funnel mapping lookups to ensure ordered lookups and flushes
static UserGroup* Mbrd_GetItemWithIdentifierAndRetain( MbrdCache *cache, int idType, const void *identifier, int32_t flags )
{
	__block UserGroup *item = NULL;
	
	// we serialize these lookups to prevent collision lookups for the same record etc.
	// memberships are handled async for the record
	// additionally prevents issues where clients can call in the middle of network changes causing random membership failures
	dispatch_sync( gLookupQueue,
				   ^(void) {
					   CInternalDispatch::AddCapability();
					   item = __Mbrd_GetItemWithIdentifierAndRetain( cache, idType, identifier, flags );
				   } );
	
	return item;
}

static void Mbrd_ResolveGroupsForItem( UserGroup *item, uint32_t flags, UserGroup *membershipRoot = NULL )
{
	UserGroup **items;
	uuid_string_t guidString;
	UInt32 count = 0;
	
	// if this item is not found, we don't try to resolve memberships
	if ( item == NULL || (item->fFlags & kUGFlagNotFound) != 0 )
		return;
	
	uint64_t microsec = GetElapsedMicroSeconds();
	const char *reqOrigin = ((flags & kKernelRequest) != 0 ? "mbr_syscall" : "mbr_mig");

	uuid_unparse_upper( item->fGUID, guidString );
	
	// WARNING: do not use fQueue as refreshes are initiated on this queue to prevent collisions
	dispatch_queue_t dispatchQueue = dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 );
	dispatch_group_t dispatchGroup = dispatch_group_create();
	
	void (^addToHashesAndRelease)(UserGroup *, UserGroup *, UserGroup *) = ^(UserGroup *userItem, UserGroup *memberItem, UserGroup *groupItem) {
		// add to the membership root (user or computer)
		//    success - continue resolve the group
		//    failure - redundant or cyclic group
		
		// if groupItem was passed then it is a result of nested searches
		if ( groupItem != NULL ) {
			UserGroup_AddToHashes( groupItem, memberItem );
		}
		
		// user item is the cyclic guard, so use it as the check
		if ( UserGroup_AddToHashes(userItem, memberItem) == true ) {
			if ( (memberItem->fFlags & kUGFlagNotFound) == 0 ) {
				UserGroup_Retain( memberItem );
				dispatch_group_async( dispatchGroup,
									  dispatchQueue,
									  ^(void) {
										  DbgLog( kLogInfo, "%s - Membership - Resolve Groups - check nested group '%s' to membership for '%s'",
												  reqOrigin, memberItem->fName ?: "", userItem->fName ?: "" );

										  CInternalDispatch::AddCapability();
										  Mbrd_ResolveGroupsForItem( memberItem, flags, userItem );
										  
										  UserGroup_Release( memberItem );
									  } );
			}
		}
		else {
			DbgLog( kLogInfo, "%s - Membership - Resolve Groups - group '%s' already in membership for '%s', skipping nested check", 
				    reqOrigin, memberItem->fName ?: "", userItem->fName ?: "" );
		}
		
		UserGroup_Release( memberItem ); // retained when it is added to hashes
	};
	
	// first lets see if it's a user so we can do user-specific lookups
	if ( (item->fRecordType & (kUGRecordTypeUser | kUGRecordTypeComputer)) != 0 )
	{
		// things that only apply to users
		if ( (item->fRecordType & kUGRecordTypeUser) != 0 )
		{
			// first deal with the Primary group
			UserGroup *tempItem = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_GID, &item->fPrimaryGroup, flags );
			if ( tempItem != NULL )
			{
				DbgLog( kLogInfo, "%s - Membership - Resolve Groups - adding primary group '%d' to membership for '%s'", 
					    reqOrigin, tempItem->fID, item->fName ?: "" );
				
				addToHashesAndRelease( item, tempItem, NULL );
				tempItem = NULL;
			}
			else
			{
				__sync_add_and_fetch( &gStatBlock.fNumFailedRecordLookups, 1 );
			}
			
			count = 0;
			items = Mbrd_FindItemsAndRetain( gMbrdSearchNode, gAllGroupTypes, ID_TYPE_GROUPMEMBERSHIP, item->fName, flags, &count );
			if ( items != NULL )
			{
				DbgLog( kLogInfo, "%s - Membership - Resolve Groups - adding %d direct name memberships via GUID to membership for '%s'", 
					    reqOrigin, count, (item->fName ? :"") );
				for ( UInt32 ii = 0; ii < count; ii++ )
				{
					addToHashesAndRelease( item, items[ii], NULL );
					items[ii] = NULL;
				}
									
				free( items );
				items = NULL;
			}
			else
			{
				__sync_add_and_fetch( &gStatBlock.fNumFailedRecordLookups, 1 );
			}
		}
		
		// everyone may be nested inside of groups
		UserGroup *tempItem = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_GUID, gEveryoneUUID, flags );
		if ( tempItem != NULL ) // should never fail, but safety
		{
			DbgLog( kLogInfo, "%s - Membership - Resolve Groups - adding 'everyone' group %d to membership for '%s'", 
				    reqOrigin, tempItem->fID, item->fName ?: "" );
			
			addToHashesAndRelease( item, tempItem, NULL );
			tempItem = NULL;
		}
		
		// now check netaccounts or localaccounts
		if ( (item->fFlags & kUGFlagLocalAccount) != 0 )
		{
			tempItem = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_GUID, gLocalAccountsUUID, flags );
			if ( tempItem != NULL ) // should never fail, but safety
			{
				DbgLog( kLogInfo, "%s - Membership - Resolve Groups - adding 'localaccounts' group %d to membership for '%s'", 
					    reqOrigin, tempItem->fID, (item->fName ? :"") );
				
				addToHashesAndRelease( item, tempItem, NULL );
				tempItem = NULL;
			}
		}
		else
		{
			tempItem = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_GUID, gNetAccountsUUID, flags );
			if ( tempItem != NULL ) // should never fail, but safety
			{
				DbgLog( kLogInfo, "%s - Membership - Resolve Groups - adding 'netaccounts' group %d to membership for '%s'", 
					    reqOrigin, tempItem->fID, (item->fName ? :"") );
				
				addToHashesAndRelease( item, tempItem, NULL );
				tempItem = NULL;
			}
		}

		count = 0;
		items = Mbrd_FindItemsAndRetain( gMbrdSearchNode, gAllGroupTypes, ID_TYPE_GROUPMEMBERS, guidString, flags, &count );
		if ( items != NULL )
		{
			DbgLog( kLogInfo, "%s - Membership - Resolve Groups - adding %d direct UUID memberships to membership for '%s'", 
				    reqOrigin, count, (item->fName ? :"") );
			for ( UInt32 ii = 0; ii < count; ii++ )
			{
				addToHashesAndRelease( item, items[ii], NULL );
				items[ii] = NULL;
			}
			
			free( items );
			items = NULL;
		}
		else
		{
			__sync_add_and_fetch( &gStatBlock.fNumFailedRecordLookups, 1 );
		}
	}
	else if ( membershipRoot != NULL )
	{
		items = Mbrd_FindItemsAndRetain( gMbrdSearchNode, gAllGroupTypes, ID_TYPE_NESTEDGROUPS, guidString, flags, &count );
		if ( items != NULL )
		{
			DbgLog( kLogInfo, "%s - Membership - Resolve Groups - adding %d nested groups to membership for '%s'", 
				    reqOrigin, count, membershipRoot->fName ?: "" );
			for ( UInt32 ii = 0; ii < count; ii++ )
			{
				addToHashesAndRelease( membershipRoot, items[ii], item );
				items[ii] = NULL;
			}

			free( items );
			items = NULL;
		}
		else
		{
			__sync_add_and_fetch( &gStatBlock.fNumFailedRecordLookups, 1 );
		}
	}
	
	// now wait for group to finish
	dispatch_group_wait( dispatchGroup, UINT64_MAX );
	dispatch_release( dispatchGroup );
	
	DbgLog( kLogInfo, "%s - Membership - Finished resolving groups for %s '%s' - total %d", reqOrigin, 
		    ((item->fRecordType & (kUGRecordTypeGroup | kUGRecordTypeComputerGroup)) ? "group" : "user"), 
		    item->fName ?: "", item->fGUIDMembershipHash.fNumEntries );
	
	// can only be flagged for users since groups are partial resolution
	if ( (item->fRecordType & (kUGRecordTypeUser | kUGRecordTypeComputer)) != 0 ) {
		__sync_or_and_fetch( &item->fFlags, kUGFlagValidMembership );
	}
	
	microsec = GetElapsedMicroSeconds() - microsec;
	Mbrd_AddToAverage( &gStatBlock.fAverageuSecPerMembershipSearch, &gStatBlock.fTotalMembershipSearches, microsec);
}

static void Mbrd_GenerateItemMembership( UserGroup *item, uint32_t flags, bool bAsyncRefresh = false )
{
	// don't issue a refresh if one is in flight
	if ( item == NULL ) return;
	
	__block bool bDoWaitBlock = false;
	__block bool bIssueRefresh = false;
	const char *reqOrigin = ((flags & kKernelRequest) != 0 ? "mbr_syscall" : "mbr_mig");

	dispatch_sync( item->fQueue,
				   ^(void) {
					   uint32_t currentTime = GetElapsedSeconds();
					   
					   // if we don't have valid memberships or the flags say to refresh them
					   if ( (item->fFlags & kUGFlagValidMembership) == 0 || 
						    ((flags & KAUTH_EXTLOOKUP_REFRESH_MEMBERSHIP) != 0 && item->fMaximumRefresh <= currentTime) )
					   {
						   if ( __sync_bool_compare_and_swap(&item->fRefreshActive, false, true) == true ) {
							   if ( (item->fFlags & kUGFlagValidMembership) == 0 ) {
								   DbgLog( kLogInfo, "%s - Membership - '%s' (%s) - generating group memberships", 
										   reqOrigin, item->fName ? : "", item->fNode ? : "" );
							   }
							   else {
								   DbgLog( kLogInfo, "%s - Membership - '%s' (%s) force refresh requested - entry and group memberships %s", 
										   reqOrigin, item->fName ? : "", item->fNode ? : "", bAsyncRefresh ? "(Async requested)" : "" );
							   }
							   
							   bIssueRefresh = true;
						   }
						   
						   bDoWaitBlock = true;
					   }
					   else if ( item->fExpiration <= currentTime )
					   {
						   if ( __sync_bool_compare_and_swap(&item->fRefreshActive, false, true) == true ) {
							   DbgLog( kLogInfo, "%s - Membership - '%s' (%s) outdated - will refresh entry and group memberships asynchronously", 
									   reqOrigin, item->fName ? : "", item->fNode ? : "" );
							   bIssueRefresh = true;
						   }
					   }
				   } );
	
	if ( bIssueRefresh == true ) {
		UserGroup_Retain( item );
		dispatch_async( item->fRefreshQueue, 
					    ^(void) {
							CInternalDispatch::AddCapability();
							
							UserGroup *refreshed = NULL;
							int isUser;
							uid_t uid;
							
							// if it is not a compatibility GUID, search via GUID again to refresh the entry
							if ( (item->fFlags & kUGFoundByGUID) != 0 && IsCompatibilityGUID(item->fGUID, &isUser, &uid) == false ) {
								uuid_string_t uuidStr;
								
								uuid_unparse_upper( item->fGUID, uuidStr );
								refreshed = Mbrd_FindItemAndRetain( gMbrdSearchNode, gUserType, ID_TYPE_GUID, uuidStr, flags );
							}
							else if ( (item->fFlags & kUGFoundByID) != 0 ) {
								char uidStr[16];
								snprintf( uidStr, sizeof(uidStr), "%d", item->fID );
								refreshed = Mbrd_FindItemAndRetain( gMbrdSearchNode, gUserType, ID_TYPE_UID, uidStr, flags );
							}
							else if ( (item->fFlags & kUGFoundByName) != 0 ) {
								refreshed = Mbrd_FindItemAndRetain( gMbrdSearchNode, gUserType, ID_TYPE_USERNAME, item->fName, flags );
							}
							
							// the find logic removes old entry if it's no longer found
							if ( refreshed != NULL )
							{
								UserGroup *workingCopy = UserGroup_Create();
								
								// copy the entry (skipping memberships) and resolve the memberships, then we'll merge back into the existing
								UserGroup_Merge( workingCopy, refreshed, false );
								
								Mbrd_ResolveGroupsForItem( workingCopy, flags );
								
								// now merge back into the original item
								UserGroup_Merge( refreshed, workingCopy, true );
								if ( refreshed != item ) {
									UserGroup_Merge( item, workingCopy, true );
									item->fRefreshActive = false;
									DbgLog( kLogNotice, "%s - Membership - '%s' (%s) - inflight membership generation encountered a new entry - syncing groups to original request",
										    reqOrigin, item->fName ? : "", item->fNode ? : ""  );
								}
								
								refreshed->fRefreshActive = false;
								
								UserGroup_Release( workingCopy );
								UserGroup_Release( refreshed );
							}
							
							UserGroup_Release( item );
						} );
	}
	
	if ( bDoWaitBlock == true && bAsyncRefresh == false ) {
		DbgLog( kLogInfo, "%s - Membership - '%s' (%s) - waiting for an inflight membership generation to complete",
			    reqOrigin, item->fName ? : "", item->fNode ? : "" );
		dispatch_sync( item->fRefreshQueue, 
					   ^(void) {
						   DbgLog( kLogDebug, "%s - Membership - '%s' (%s) - finished waiting for an inflight membership generation to complete",
								   reqOrigin, item->fName ? : "", item->fNode ? : "" );
					   } );
	}
}

int IsUserMemberOfGroupByGUID( UserGroup* user, uuid_t groupGUID, uint32_t flags )
{
	bool bAsyncRefresh = false;
	
	// a Refresh request and a NULL groupGUID is meant to force a refresh asynchronously
	if ( (flags & KAUTH_EXTLOOKUP_REFRESH_MEMBERSHIP) != 0 && uuid_is_null(groupGUID) != 0 ) {
		if ( (user->fFlags & kUGFlagValidMembership) == 0 ) {
			// this isn't a true membership check, it's a trigger to refresh the groups asynchronously to ensure up-to-date membership
			// but if there is no current membership, let's not generate it now until someone actually needs them
			return 0;
		}
		
		bAsyncRefresh = true;
	}
	
	if ( uuid_compare(gEveryoneUUID, groupGUID) == 0 )
		return 1;
	
	if ( user == NULL )
		return 0;
	
	if ( uuid_compare(gNetAccountsUUID, groupGUID) == 0 )
		return (((user->fRecordType & kUGRecordTypeUser) != 0 || (user->fRecordType & kUGRecordTypeComputer) != 0) && 
				 (user->fFlags & kUGFlagLocalAccount) == 0);
	
	if ( uuid_compare(gLocalAccountsUUID, groupGUID) == 0 )
		return (((user->fRecordType & kUGRecordTypeUser) != 0 || (user->fRecordType & kUGRecordTypeComputer) != 0) && 
				 (user->fFlags & kUGFlagLocalAccount) != 0);
	
	Mbrd_GenerateItemMembership( user, flags, bAsyncRefresh );
	
	UserGroup *result = HashTable_GetAndRetain( &user->fGUIDMembershipHash, groupGUID );
	
	// all groups are encompassed in the actual membership hash table
	UserGroup_Release( result ); // we don't NULL cause we use it as a boolean
	
	return (result != NULL);
}

int IsUserMemberOfGroupBySID( UserGroup* user, ntsid_t* groupSID, uint32_t flags )
{
	ntsid_t tempSID;
	UserGroup* result;
	
	if ( user == NULL || groupSID->sid_authcount > KAUTH_NTSID_MAX_AUTHORITIES )
		return 0;
		
	memset(&tempSID, 0, sizeof(ntsid_t));
	memcpy(&tempSID, groupSID, KAUTH_NTSID_SIZE(groupSID));
	
	if (memcmp(&user->fSID, &tempSID, sizeof(ntsid_t)) == 0)
		return 1;
	
	if (memcmp(&gEveryoneSID, &tempSID, sizeof(ntsid_t)) == 0)
		return 1;
	
	Mbrd_GenerateItemMembership( user, flags );
	
	result = HashTable_GetAndRetain( &user->fSIDMembershipHash, &tempSID );
	
	// all groups are encompassed in the actual membership hash table
	UserGroup_Release( result ); // we don't NULL since we are using as a boolean
	
	return (result != NULL);
}

static int IsUserMemberOfGroupByGID( UserGroup *user, gid_t gid, uint32_t flags )
{
	if ( user == NULL ) return false;
	
	Mbrd_GenerateItemMembership( user, flags );
	
	UserGroup *result = HashTable_GetAndRetain( &user->fGIDMembershipHash, (void *)&gid );

	// all groups are encompassed in the actual membership hash table
	UserGroup_Release( result ); // we don't NULL since we are using as a boolean

	return (result != NULL);
}

#pragma mark -
#pragma mark Public routines

void Mbrd_SwapRequest(struct kauth_identity_extlookup* request)
{
	int i;

	// swap 32 bit values
	request->el_seqno = OSSwapInt32(request->el_seqno);
	request->el_result = OSSwapInt32(request->el_result);
	request->el_flags = OSSwapInt32(request->el_flags);
	request->el_uid = OSSwapInt32(request->el_uid);
	request->el_uguid_valid = OSSwapInt32(request->el_uguid_valid);
	request->el_usid_valid = OSSwapInt32(request->el_usid_valid);
	request->el_gid = OSSwapInt32(request->el_gid);
	request->el_gguid_valid = OSSwapInt32(request->el_gguid_valid);
	request->el_member_valid = OSSwapInt32(request->el_member_valid);

	// uuids don't need swapping, and only the sid_authorities part of the sids do
	for (i = 0; i < KAUTH_NTSID_MAX_AUTHORITIES; i++)
	{
		request->el_usid.sid_authorities[i] = OSSwapInt32(request->el_usid.sid_authorities[i]);
		request->el_gsid.sid_authorities[i] = OSSwapInt32(request->el_gsid.sid_authorities[i]);
	}
}

void Mbrd_InitializeGlobals( void )
{
	const char* path = "/etc/memberd.conf";
	char buffer[1024];
	int fd;
	off_t len;
	int rewriteConfig = 0;
	struct stat sb;
	int defaultExpiration = kDefaultExpirationClient;
	int defaultNegExpiration = kDefaultNegativeExpirationClient;
	int kernelExpiration = kDefaultKernelExpiration;
	int maximumRefresh = kDefaultMaximumRefresh;
	int kerberosFallback = 0;
	
	if ( gServerOS == true )
	{
		defaultExpiration = kDefaultExpirationServer;
		defaultNegExpiration = kDefaultNegativeExpirationServer;
	}
	
	// if this is the installer or local only mode, we ignore the .conf file because we are probably read-only
	if ( !gDSInstallDaemonMode && !gDSLocalOnlyMode )
	{
		int result = stat(path, &sb);
		
		if ((result != 0) || (sb.st_size > 1023))
			rewriteConfig = 1;
		else
		{
			fd = open(path, O_RDONLY, 0);
			if (fd < 0) goto initialize;
			len = read(fd, buffer, sb.st_size);
			close(fd);
			if (strncmp(buffer, "#1.3", 4) != 0)
				rewriteConfig = 1;
		}
		
		if (rewriteConfig)
		{
			fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0755);
			if (fd < 0) goto initialize;
			snprintf(buffer, sizeof(buffer), "#1.3\n%s %d\n%s %d\n%s %d\n%s %d\n%s %d\n",
					 kDefaultExpirationStr, defaultExpiration, 
					 kDefaultNegativeExpirationStr, defaultNegExpiration,
					 kDefaultKernelExpirationInSecsStr, kernelExpiration,
					 kDefaultMaximumRefreshInSecsStr, maximumRefresh,
					 kKerberosFallbackToRecordName, 1);
			len = write(fd, buffer, strlen(buffer));
			close(fd);
		}
		else
		{
			char* temp;
			off_t i;
			
			if (len != sb.st_size) goto initialize;
			buffer[len] = '\0';
			
			for (i = 0; i < len; i++)
				if (buffer[i] == '\n') buffer[i] = '\0';
			
			i = 0;
			while (i < len)
			{
				temp = buffer + i;
				if (strncmp(temp, kDefaultExpirationStr, sizeof(kDefaultExpirationStr) - 1) == 0)
				{
					temp += sizeof(kDefaultExpirationStr) - 1;
					defaultExpiration = strtol(temp, &temp, 10);
					if (defaultExpiration < 30)
						defaultExpiration = 30;
					else if (defaultExpiration > 24 * 60 * 60)
						defaultExpiration = 24 * 60 * 60;
				}
				else if (strncmp(temp, kDefaultNegativeExpirationStr, sizeof(kDefaultNegativeExpirationStr) - 1) == 0)
				{
					temp += sizeof(kDefaultNegativeExpirationStr) - 1;
					defaultNegExpiration = strtol(temp, &temp, 10);
					if (defaultNegExpiration < 30)
						defaultNegExpiration = 30;
					else if (defaultNegExpiration > 24 * 60 * 60)
						defaultNegExpiration = 24 * 60 * 60;
				}
				else if (strncmp(temp, kDefaultKernelExpirationInSecsStr, sizeof(kDefaultKernelExpirationInSecsStr) - 1) == 0)
				{
					temp += sizeof(kDefaultKernelExpirationInSecsStr) - 1;
					kernelExpiration = strtol(temp, &temp, 10);
					if (kernelExpiration < 30)
						kernelExpiration = 30;
					else if (kernelExpiration > 1 * 60 * 60)
						kernelExpiration = 1 * 60 * 60;
				}
				else if (strncmp(temp, kDefaultMaximumRefreshInSecsStr, sizeof(kDefaultMaximumRefreshInSecsStr) - 1) == 0)
				{
					temp += sizeof(kDefaultMaximumRefreshInSecsStr) - 1;
					maximumRefresh = strtol(temp, &temp, 10);
					if (maximumRefresh < 2 * 60)
						maximumRefresh = 2 * 60;
					else if (maximumRefresh > 1 * 60 * 60)
						maximumRefresh = 1 * 60 * 60;
				}
				else if (strncmp(temp, kKerberosFallbackToRecordName, sizeof(kKerberosFallbackToRecordName) - 1) == 0 )
				{
					temp += sizeof(kKerberosFallbackToRecordName) - 1;
					kerberosFallback = strtol(temp, &temp, 10);
				}
				
				i += strlen(temp) + 1;
			}
		}
	}
	
initialize:

	gMbrdCache = MbrdCache_Create( defaultExpiration, defaultNegExpiration, kernelExpiration, maximumRefresh, kerberosFallback );
	assert( gMbrdCache != NULL );
	
	gLookupQueue = dispatch_queue_create( "Membership lookup queue", NULL );
	pthread_key_create( &gMembershipThreadKey, NULL ); // no cleanup needed, just a flag

	uuid_parse( "ABCDEFAB-CDEF-ABCD-EFAB-CDEF0000000C", gEveryoneUUID );
	Mbrd_ConvertSIDFromString("S-1-1-0", &gEveryoneSID);
	
	uuid_parse( "ABCDEFAB-CDEF-ABCD-EFAB-CDEF0000003D", gLocalAccountsUUID );
	uuid_parse( "ABCDEFAB-CDEF-ABCD-EFAB-CDEF0000003E", gNetAccountsUUID );
	uuid_parse( "FFFFEEEE-DDDD-CCCC-BBBB-AAAA00000000", gRootUserUUID );
	
	bzero( &gStatBlock, sizeof(gStatBlock) );
	gStatBlock.fTotalUpTime = GetElapsedSeconds();
}

void Mbrd_Initialize( void )
{
	// Directory Service related
	tDataBufferPtr  nodeBuffer		= NULL;
	tContextData	localContext	= 0;
	tDataListPtr	nodeName		= NULL;
	UInt32			returnCount		= 0;
	
	tDirStatus status = dsOpenDirService( &gMbrdDirRef );
	assert( status == eDSNoErr );
	
	nodeBuffer = dsDataBufferAllocate( gMbrdDirRef, 4096 );
	assert( nodeBuffer != NULL );
	
	status = dsFindDirNodes( gMbrdDirRef, nodeBuffer, NULL, eDSAuthenticationSearchNodeName, &returnCount, &localContext );
	assert( status == eDSNoErr && returnCount != 0 );
	
	status = dsGetDirNodeName( gMbrdDirRef, nodeBuffer, 1, &nodeName ); //Currently we only look at the 1st node.
	assert( status == eDSNoErr );
	
	status = dsOpenDirNode( gMbrdDirRef, nodeName, &gMbrdSearchNode );
	assert( status == eDSNoErr );
	
	if ( localContext != 0 )
		dsReleaseContinueData( gMbrdDirRef, localContext );
	
	dsDataBufferDeAllocate( gMbrdDirRef, nodeBuffer );
	dsDataListDeallocate( gMbrdDirRef, nodeName );
	free( nodeName );

	gUserType = dsBuildListFromStringsPriv( kDSStdRecordTypeUsers, kDSStdRecordTypeComputers, NULL );
	gAllGroupTypes = dsBuildListFromStringsPriv( kDSStdRecordTypeGroups, kDSStdRecordTypeComputerGroups, NULL );
	gUnknownType = dsBuildListFromStringsPriv( kDSStdRecordTypeUsers, kDSStdRecordTypeComputers, kDSStdRecordTypeGroups,
											   kDSStdRecordTypeComputerGroups, NULL );
	
	// extended list so that Cache node can use
	gAttrsToGet = dsBuildListFromStringsPriv( kDSNAttrMetaNodeLocation, kDSNAttrRecordName, kDS1AttrPassword, kDS1AttrUniqueID,
											  kDS1AttrGeneratedUID, kDS1AttrPrimaryGroupID, kDS1AttrNFSHomeDirectory, kDS1AttrUserShell,
											  kDS1AttrDistinguishedName, kDSNAttrGroupMembership, kDS1AttrTimeToLive, kDS1AttrSMBSID,
											  kDS1AttrENetAddress, kDS1AttrCopyTimestamp, kDSNAttrAltSecurityIdentities, kDS1AttrSMBRID,
											  kDS1AttrSMBGroupRID, kDS1AttrSMBPrimaryGroupSID, kDS1AttrOriginalNodeName, kDSNAttrKeywords, NULL );
}

void Mbrd_ProcessLookup(struct kauth_identity_extlookup* request)
{
	uint32_t flags = request->el_flags;
	UserGroup* user = NULL;
	UserGroup* group = NULL;
	int isMember = -1;
	uint64_t microsec = GetElapsedMicroSeconds();
	const char *reqOrigin = ((flags & kKernelRequest) != 0 ? "mbr_syscall" : "mbr_mig");
	
	// let's see if this is something related to root that we can just answer
	//
	// ensure we don't have any other bits set, if we do, we have more work and cannot shortcut the process, 
	// but if this is just a lookup then we can use default for UUID FFFFEEEE-DDDD-CCCC-BBBB-AAAA00000000 and UID 0
	if ( 0 == (flags & ~(KAUTH_EXTLOOKUP_VALID_UGUID | KAUTH_EXTLOOKUP_VALID_GGUID | KAUTH_EXTLOOKUP_WANT_UID | KAUTH_EXTLOOKUP_WANT_GID)) && 
		 (flags & (KAUTH_EXTLOOKUP_VALID_UGUID | KAUTH_EXTLOOKUP_WANT_UID)) == (KAUTH_EXTLOOKUP_VALID_UGUID | KAUTH_EXTLOOKUP_WANT_UID) &&
		 uuid_compare(gRootUserUUID, request->el_uguid.g_guid) == 0 )
	{
		request->el_flags |= KAUTH_EXTLOOKUP_VALID_UID;
		request->el_uid = 0;
		request->el_result = KAUTH_EXTLOOKUP_SUCCESS;
		DbgLog( kLogPlugin, "%s - Dispatch - Lookup - UUID FFFFEEEE-DDDD-CCCC-BBBB-AAAA00000000 default answer 0 (root)", reqOrigin );
		return;
	}
	else if ( flags == (flags & (KAUTH_EXTLOOKUP_VALID_UID | KAUTH_EXTLOOKUP_WANT_UGUID)) && request->el_uid == 0 )
	{
		request->el_flags |= KAUTH_EXTLOOKUP_VALID_UGUID;
		uuid_copy( request->el_uguid.g_guid, gRootUserUUID );
		request->el_uguid_valid = MbrdCache_GetDefaultExpiration( gMbrdCache );
		request->el_result = KAUTH_EXTLOOKUP_SUCCESS;
		DbgLog( kLogPlugin, "%s - Dispatch - Lookup - UID 0 default answer FFFFEEEE-DDDD-CCCC-BBBB-AAAA00000000 (root) - TTL %u",
			    reqOrigin, request->el_uguid_valid );
		return;
	}
	
	// special-case the everyone group.. if we had a valid group GUID that matches everyone
	// and we have a valid UID, UGUID or USID, it's always true
	// TODO: don't like hardcoding everyone GID, we should discover that from the directory
	if ( (flags & KAUTH_EXTLOOKUP_WANT_MEMBERSHIP) != 0 &&
		 ((flags & KAUTH_EXTLOOKUP_VALID_UGUID) != 0 || (flags & KAUTH_EXTLOOKUP_VALID_UID) != 0 || (flags & KAUTH_EXTLOOKUP_VALID_USID) != 0) &&
		 (((flags & KAUTH_EXTLOOKUP_VALID_GID) != 0 && request->el_gid == 12) ||
		  ((flags & KAUTH_EXTLOOKUP_VALID_GGUID) != 0 && uuid_compare(gEveryoneUUID, request->el_gguid.g_guid) == 0) ||
		  ((flags & KAUTH_EXTLOOKUP_VALID_GSID) != 0 && memcmp(&gEveryoneSID, &request->el_gsid, sizeof(gEveryoneSID)) == 0)) )
	{
		request->el_member_valid = MbrdCache_GetDefaultExpiration( gMbrdCache );	// use longer expiration for this one, no reason to ask us very often
		request->el_flags |= KAUTH_EXTLOOKUP_VALID_MEMBERSHIP | KAUTH_EXTLOOKUP_ISMEMBER;
		DbgLog( kLogPlugin, "%s - Dispatch - Membership - is user member of group everyone is always true - TTL %u", 
			    reqOrigin, request->el_member_valid );
		return;
	}
	
	if ( (flags & KAUTH_EXTLOOKUP_VALID_UGUID) != 0 )
	{
		user = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_GUID, request->el_uguid.g_guid, flags );
		
		if ( LoggingEnabled(kLogPlugin) )
		{
			uuid_string_t guidString;
			uuid_unparse( request->el_uguid.g_guid, guidString );
			DbgLog( kLogPlugin, "%s - Dispatch - Lookup - user/computer GUID %s - %s %s", reqOrigin, guidString, 
				   (user != NULL && (user->fFlags & kUGFlagNotFound) == 0 ? "succeeded" : "was not found"), 
				   (user != NULL && user->fName != NULL ? user->fName : "") );
		}
	}
	else if ( (flags & KAUTH_EXTLOOKUP_VALID_USID) != 0 )
	{
		user = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_SID, &request->el_usid, flags );
		
		if ( LoggingEnabled(kLogPlugin) )
		{
			char sidString[256] = { 0, };
			ConvertSIDToString( sidString, &request->el_usid );
			DbgLog( kLogPlugin, "%s - Dispatch - Lookup - user/computer SID %s - %s %s", reqOrigin, sidString, 
				   (user != NULL && (user->fFlags & kUGFlagNotFound) == 0 ? "succeeded" : "was not found"), 
				   (user != NULL ? user->fName : "") );
		}
	}
	else if ( (flags & KAUTH_EXTLOOKUP_VALID_UID) != 0 )
	{
		user = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_UID, &request->el_uid, flags );
		DbgLog( kLogPlugin, "%s - Dispatch - Lookup - user/computer ID %d - %s %s", reqOrigin, request->el_uid, 
			   (user != NULL && (user->fFlags & kUGFlagNotFound) == 0 ? "succeeded" : "was not found"), 
			   (user != NULL ? user->fName : "") );
	}
	
	if ( user != NULL && (user->fRecordType & (kUGRecordTypeGroup | kUGRecordTypeComputerGroup)) != 0 )
	{
		if ( (user->fFlags & kUGFlagNotFound) == 0 ) {
			DbgLog( kLogPlugin, "%s - Dispatch - Lookup - result rejected was not a user/computer", reqOrigin );
		}
		
		// we discard when it is the wrong type if:
		//   userland request - discard regardless
		//   kernel request - only if it was really found (need to send kernel transient IDs even if it wasn't found)
		if ( (flags & kKernelRequest) == 0 || (user->fFlags & kUGFlagNotFound) == 0 ) {
			UserGroup_Release( user );
			user = NULL;
		}
	}
				
	if ( (flags & KAUTH_EXTLOOKUP_WANT_MEMBERSHIP) != 0 && user != NULL && (user->fFlags & kUGFlagNotFound) == 0 )
	{
		request->el_member_valid = MbrdCache_TTL( gMbrdCache, user, flags );
		if ( (flags & KAUTH_EXTLOOKUP_VALID_GGUID) != 0 )
		{
			isMember = IsUserMemberOfGroupByGUID( user, request->el_gguid.g_guid, flags );
			
			if ( user != NULL && user->fName != NULL && LoggingEnabled(kLogPlugin) && uuid_is_null(request->el_gguid.g_guid) == 0 ) {
				uuid_string_t guidString;
				uuid_unparse( request->el_gguid.g_guid, guidString );
				
				DbgLog( kLogPlugin, "%s - Dispatch - Membership - is %s %s member of group GUID %s = %s - TTL %u", reqOrigin, 
						((user->fRecordType & kUGRecordTypeUser) != 0 ? "user" : "computer"), user->fName, guidString, (isMember == 1 ? "true" : "false"),
						request->el_member_valid );
			}
		}
		else if ( (flags & KAUTH_EXTLOOKUP_VALID_GSID) != 0 )
		{
			isMember = IsUserMemberOfGroupBySID( user, &request->el_gsid, flags );
			
			if ( user != NULL && user->fName != NULL && LoggingEnabled(kLogPlugin) )
			{
				char sidString[256] = { 0, };
				ConvertSIDToString( sidString, &request->el_gsid );
				DbgLog( kLogPlugin, "%s - Dispatch - Membership - is %s %s member of group SID %s = %s - TTL %u", reqOrigin, 
						((user->fRecordType & kUGRecordTypeUser) != 0 ? "user" : "computer"), user->fName, sidString, (isMember == 1 ? "true" : "false"),
						request->el_member_valid );
			}
		}
		else if ( (flags & KAUTH_EXTLOOKUP_VALID_GID) != 0 )
		{
			isMember = IsUserMemberOfGroupByGID( user, request->el_gid, flags );
			if ( user != NULL && user->fName != NULL && LoggingEnabled(kLogPlugin) )
			{
				DbgLog( kLogPlugin, "%s - Dispatch - Membership - is %s %s member of group GID %d = %s - TTL %u", reqOrigin, 
					   ((user->fRecordType & kUGRecordTypeUser) != 0 ? "user" : "computer"), user->fName, request->el_gid, (isMember == 1 ? "true" : "false"),
					   request->el_member_valid );
			}
		}
			
		request->el_flags |= KAUTH_EXTLOOKUP_VALID_MEMBERSHIP;
		if (isMember == 1) {
			request->el_flags |= KAUTH_EXTLOOKUP_ISMEMBER;
		}
	}
	
	if ( (flags & KAUTH_EXTLOOKUP_WANT_UID) != 0 && user != NULL )
	{
		request->el_flags |= KAUTH_EXTLOOKUP_VALID_UID;
		request->el_uid = user->fID;
		DbgLog( kLogPlugin, "%s - Dispatch - WantUID - found %d - %s", reqOrigin, user->fID, (user->fName ?: "") );
	}
	
	if ( (flags & KAUTH_EXTLOOKUP_WANT_UGUID) != 0 )
	{
		request->el_uguid_valid = MbrdCache_TTL( gMbrdCache, user, flags );
		if ( user != NULL ) {
			request->el_flags |= KAUTH_EXTLOOKUP_VALID_UGUID;
			uuid_copy( request->el_uguid.g_guid, user->fGUID );
			
			if ( LoggingEnabled(kLogPlugin) )
			{
				uuid_string_t guidString;
				uuid_unparse( user->fGUID, guidString );
				DbgLog( kLogPlugin, "%s - Dispatch - WantGUID - found %s - %s - TTL %u", 
					    reqOrigin, guidString, (user->fName ?: ""), request->el_uguid_valid );
			}
		}
	}
	
	if ( (flags & KAUTH_EXTLOOKUP_WANT_USID) != 0 )
	{
		request->el_usid_valid = MbrdCache_TTL( gMbrdCache, user, flags );
		if ( user != NULL && (user->fFlags & kUGFlagHasSID) != 0 ) {
			request->el_flags |= KAUTH_EXTLOOKUP_VALID_USID;
			memcpy(&request->el_usid, &user->fSID, sizeof(ntsid_t));
			
			if ( LoggingEnabled(kLogPlugin) )
			{
				char sidString[256] = { 0, };
				ConvertSIDToString( sidString, &user->fSID );
				DbgLog( kLogPlugin, "%s - Dispatch - WantSID - found %s - %s - TTL %u", 
					    reqOrigin, sidString, (user->fName ?: ""), request->el_usid_valid );
			}
		}
	}

	UserGroup_Release( user );
	user = NULL;
	
	if ( (flags & (KAUTH_EXTLOOKUP_WANT_GID | KAUTH_EXTLOOKUP_WANT_GGUID | KAUTH_EXTLOOKUP_WANT_GSID)) != 0 )
	{
		if (flags & KAUTH_EXTLOOKUP_VALID_GGUID)
		{
			group = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_GUID, request->el_gguid.g_guid, flags );
			
			if ( LoggingEnabled(kLogPlugin) )
			{
				uuid_string_t guidString;
				uuid_unparse( request->el_gguid.g_guid, guidString );
				DbgLog( kLogPlugin, "%s - Dispatch - Lookup - group/computergroup GUID %s - %s %s", reqOrigin, guidString, 
					    (group != NULL && (group->fFlags & kUGFlagNotFound) == 0 ? "succeeded" : "was not found"), 
					    (group != NULL && group->fName != NULL ? group->fName : "") );
			}
		}
		else if (flags & KAUTH_EXTLOOKUP_VALID_GSID)
		{
			group = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_SID, &request->el_gsid, flags );

			if ( LoggingEnabled(kLogPlugin) )
			{
				char sidString[256] = { 0, };
				ConvertSIDToString( sidString, &request->el_gsid );
				DbgLog( kLogPlugin, "%s - Dispatch - Lookup - group/computergroup SID %s - %s %s", reqOrigin, sidString, 
					    (group != NULL && (group->fFlags & kUGFlagNotFound) == 0 ? "succeeded" : "was not found"), 
					    (group != NULL && group->fName != NULL ? group->fName : "") );
			}
		}
		else if (flags & KAUTH_EXTLOOKUP_VALID_GID)
		{
			group = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_GID, &request->el_gid, flags );
			
			DbgLog( kLogPlugin, "%s - Dispatch - Lookup - group/computergroup GID %d - %s %s", reqOrigin, request->el_gid, 
				    (group != NULL && (group->fFlags & kUGFlagNotFound) == 0 ? "succeeded" : "was not found"), 
				    (group != NULL && group->fName != NULL ? group->fName : "") );
		}
	}
	
	if ( group != NULL && (group->fRecordType & (kUGRecordTypeUser | kUGRecordTypeComputer)) != 0 )
	{
		if ( (group->fFlags & kUGFlagNotFound) == 0 ) {
			DbgLog( kLogPlugin, "%s - Dispatch - Lookup - result rejected was not a group", reqOrigin );
		}
		UserGroup_Release( group );
		group = NULL;
	}
	
	if ( (flags & KAUTH_EXTLOOKUP_WANT_GID) != 0 && group != NULL)
	{
		request->el_flags |= KAUTH_EXTLOOKUP_VALID_GID;
		request->el_gid = group->fID;
		if ( group->fName != NULL ) {
			DbgLog( kLogPlugin, "%s - Dispatch - WantGID - found %d - %s", reqOrigin, group->fID, group->fName );
		}
		else {
			DbgLog( kLogPlugin, "%s - Dispatch - WantGID - found %d", reqOrigin, group->fID );
		}
	}
	
	if ( (flags & KAUTH_EXTLOOKUP_WANT_GGUID) != 0 )
	{
		request->el_gguid_valid = MbrdCache_TTL( gMbrdCache, group, flags );
		if ( group != NULL ) {
			uuid_copy( request->el_gguid.g_guid, group->fGUID );
			request->el_flags |= KAUTH_EXTLOOKUP_VALID_GGUID;
			
			if ( group->fName != NULL && LoggingEnabled(kLogPlugin) )
			{
				uuid_string_t guidString;
				uuid_unparse( group->fGUID, guidString );
				DbgLog( kLogPlugin, "%s - Dispatch - WantGGUID - found %s - %s - TTL %u", 
					    reqOrigin, guidString, group->fName, request->el_gguid_valid );
			}
		}
	}
	
	if ( (flags & KAUTH_EXTLOOKUP_WANT_GSID) != 0 )
	{
		request->el_gsid_valid = MbrdCache_TTL( gMbrdCache, group, flags );
		if ( group != NULL && (group->fFlags & kUGFlagHasSID) != 0 ) {
			request->el_flags |= KAUTH_EXTLOOKUP_VALID_GSID;
			memcpy(&request->el_gsid, &group->fSID, sizeof(ntsid_t));
			
			if ( group->fName != NULL && LoggingEnabled(kLogPlugin) )
			{
				char sidString[256] = { 0, };
				ConvertSIDToString( sidString, &group->fSID );
				DbgLog( kLogPlugin, "%s - Dispatch - WantGSID - found %s - %s - TTL %u", 
					    reqOrigin, sidString, group->fName, request->el_gsid_valid );
			}
		}
	}
	
	UserGroup_Release( group );
	group = NULL;

	microsec = GetElapsedMicroSeconds() - microsec;
	Mbrd_AddToAverage(&gStatBlock.fAverageuSecPerCall, &gStatBlock.fTotalCallsHandled, microsec);
	request->el_result = KAUTH_EXTLOOKUP_SUCCESS;
}

void Mbrd_SweepCache( void *)
{
	dispatch_async( gLookupQueue,
				    ^(void) {
						MbrdCache_Sweep( gMbrdCache );
					} );
}

int Mbrd_SetNodeAvailability( const char *nodeName, bool nodeAvailable )
{
	return MbrdCache_SetNodeAvailability( gMbrdCache, nodeName, nodeAvailable );
}

int Mbrd_ProcessGetGroups(uint32_t uid, uint32_t* numGroups, GIDArray gids)
{
	uint64_t microsec = GetElapsedMicroSeconds();
	int result = KERN_SUCCESS;

	UserGroup* user = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_UID, &uid, 0 );
	
	*numGroups = 0;

	if (user != NULL)
	{
		Mbrd_GenerateItemMembership( user, 0 );
		*numGroups = UserGroup_Get16Groups( user, gids );
		UserGroup_Release( user );
		user = NULL;
	}
	else
	{
		result = KERN_FAILURE;
		syslog(LOG_ERR, "GetGroups couldn't find uid %d", uid);
	}
	
	microsec = GetElapsedMicroSeconds() - microsec;
	Mbrd_AddToAverage(&gStatBlock.fAverageuSecPerCall, &gStatBlock.fTotalCallsHandled, microsec);
	
	return result;
}

int Mbrd_ProcessGetAllGroups(uint32_t uid, uint32_t *numGroups, GIDList *gids )
{
	uint64_t microsec = GetElapsedMicroSeconds();
	int result = KERN_SUCCESS;
		
	UserGroup* user = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, ID_TYPE_UID, &uid, 0 );
	
	*numGroups = 0;
	
	if (user != NULL)
	{
		Mbrd_GenerateItemMembership( user, 0 );
		*numGroups = UserGroup_GetGroups( user, gids );
		UserGroup_Release( user );
		user = NULL;
	}
	else
	{
		result = KERN_FAILURE;
		syslog(LOG_ERR, "GetGroups couldn't find uid %d", uid);
	}
	
	microsec = GetElapsedMicroSeconds() - microsec;
	Mbrd_AddToAverage(&gStatBlock.fAverageuSecPerCall, &gStatBlock.fTotalCallsHandled, microsec);
	
	return result;
}

static int
parse_external_name( const void *data, size_t len, gss_buffer_desc *oid, gss_buffer_desc *name )
{
    oid->length = 0;
    oid->value = NULL;
    name->length = 0;
    name->value = NULL;
	
    if (len < 4)
		return 1;
	
    /* TOK_ID */
    const unsigned char *p = (const unsigned char *) data;
    if (memcmp(p, "\x04\x01", 2) != 0)
		return 1;
    len -= 2;
    p += 2;
	
    /* MECH_LEN */
    size_t l = (p[0] << 8) | p[1];
    len -= 2;
    p += 2;
    if (len < l)
		return 1;
	
    /* MECH */
    oid->length = l;
    oid->value = (void *) p;
    len -= l;
    p += l;
	
    /* MECHNAME_LEN */
    if (len < 4)
		return 1;
    l = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    len -= 4;
    p += 4;
	
    /* MECH NAME */
    if (len != l)
		return 1;
	
    name->value = (void *) p;
    name->length = l;
	
    return 0;
}

int Mbrd_ProcessMapIdentifier(int idType, const void *identifier, ssize_t identifierSize, guid_t *guid)
{
	uint64_t microsec = GetElapsedMicroSeconds();
	int result = KERN_FAILURE;
	UserGroup* item = NULL;
	const void *searchID = NULL;
	const char *tempSearchID = NULL;
	int nextIDType = idType;
	char *allocedString = NULL;
	gss_buffer_desc gssName;

	// we have to handle GSS export names first because we'll make the type one we support
	if ( idType == ID_TYPE_GSS_EXPORT_NAME ) {
		static gss_buffer_desc kerberosOID = { 11, (void *) "\x06\x09\x2a\x86\x48\x86\xf7\x12\x01\x02\x02" };
		gss_buffer_desc oid;

		if ( parse_external_name(identifier, identifierSize, &oid, &gssName) == 0 )
		{
			if ( oid.length == kerberosOID.length && memcmp(oid.value, kerberosOID.value, oid.length) == 0 ) {
				identifier = gssName.value;
				identifierSize = gssName.length;
				nextIDType = ID_TYPE_KERBEROS;
			}
		}
	}
	
again:

	switch ( nextIDType )
	{
		case ID_TYPE_UID:
			if ( sizeof(uid_t) == identifierSize ) {
				if ( *((uid_t *) identifier) == 0 ) {
					DbgLog( kLogPlugin, "mbr_mig - Dispatch - Map name using default UUID for UID 0 - FFFFEEEE-DDDD-CCCC-BBBB-AAAA00000000" );
					uuid_copy( guid->g_guid, gRootUserUUID );
					result = KERN_SUCCESS;
				}
				else {
					searchID = identifier;
				}
			}
			break;
			
		case ID_TYPE_GID:
			if ( sizeof(gid_t) == identifierSize ) {
				searchID = identifier;
			}
			break;
			
		case ID_TYPE_SID:
			if ( sizeof(ntsid_t) == identifierSize ) {
				searchID = identifier;
			}
			break;

		case ID_TYPE_USERNAME:
			tempSearchID = (const char *) identifier;
			if ( identifierSize > 0 ) {
				// safety to ensure string is NULL terminated
				if ( tempSearchID[identifierSize] != '\0' ) {
					size_t len = identifierSize + 1;
					allocedString = (char *) malloc( len );
					strlcpy( allocedString, (const char *) identifier, len ); // will NULL terminate automatically since strlcpy limits buffer
					tempSearchID = allocedString;
				}
			}
			
			if ( DSIsStringEmpty(tempSearchID) == false ) {
				if ( strcmp(tempSearchID, "root") == 0 ) {
					DbgLog( kLogPlugin, "mbr_mig - Dispatch - Map name using default UUID for root - FFFFEEEE-DDDD-CCCC-BBBB-AAAA00000000" );
					uuid_copy( guid->g_guid, gRootUserUUID );
				}
				
				searchID = tempSearchID;
			}
			break;
			
		case ID_TYPE_GROUPNAME:
			searchID = identifier;
			break;
			
		case ID_TYPE_X509_DN:
			if ( identifierSize > 0 ) {
				size_t len = sizeof("X509:") + identifierSize;
				searchID = allocedString = (char *) malloc( len );
				strlcpy( allocedString, "X509:", len );
				strlcat( allocedString, (const char *) identifier, len );
				searchID = allocedString;
			}
			break;
			
		case ID_TYPE_KERBEROS:
			if ( identifierSize > 0 ) {
				size_t len = sizeof("Kerberos:") + identifierSize;
				searchID = allocedString = (char *) malloc( len );
				strlcpy( allocedString, "Kerberos:", len );
				strlcat( allocedString, (const char *) identifier, len );
			}			
			break;
	}
		
	if ( searchID != NULL ) {
		item = Mbrd_GetItemWithIdentifierAndRetain( gMbrdCache, nextIDType, searchID, 0 );
		if ( item != NULL ) {
			if ( (item->fFlags & kUGFlagNotFound) == 0 ) {
				uuid_copy( guid->g_guid, item->fGUID );
				result = KERN_SUCCESS;
			}
			UserGroup_Release( item );
			item = NULL;
		}
		
		DSFree( allocedString );

		if ( result != KERN_SUCCESS ) {
			if ( nextIDType == ID_TYPE_KERBEROS && MbrdCache_KerberosFallback(gMbrdCache) != 0 ) {
				DbgLog( kLogInfo, "mbr_mig - Membership - Kerberos fallback enabled, looking via record name next" );
				nextIDType = ID_TYPE_USERNAME;
				goto again;
			}
		}
	}
	
	microsec = GetElapsedMicroSeconds() - microsec;
	Mbrd_AddToAverage(&gStatBlock.fAverageuSecPerCall, &gStatBlock.fTotalCallsHandled, microsec);
	
	return result;
}

void dsNodeStateChangeOccurred(void)
{
	dispatch_async( gLookupQueue,
				    ^(void) {
						MbrdCache_NodeChangeOccurred( gMbrdCache );
						pthread_mutex_lock( &sidMapLock );
						sidMap.clear();
						pthread_mutex_unlock( &sidMapLock );
						DbgLog( kLogNotice, "Membership - dsNodeStateChangeOccurred - flagging all entries as expired" );
					} );
}

void dsFlushMembershipCache(void)
{
	if (gCacheFlushDisabled == true) {
		DbgLog(kLogNotice, "mbr_mig - dsFlushMembershipCache - skipped disabled externally");
		return;
	}

	dispatch_async( gLookupQueue,
				    ^(void) {
						MbrdCache_ResetCache( gMbrdCache );
						pthread_mutex_lock( &sidMapLock );
						sidMap.clear();
						pthread_mutex_unlock( &sidMapLock );
						DbgLog( kLogNotice, "mbr_mig - dsFlushMembershipCache - force cache flush (internally initiated)" );
					} );
}

void Mbrd_ProcessResetCache(void)
{
	dispatch_async( gLookupQueue,
				    ^(void) {
						MbrdCache_ResetCache( gMbrdCache );
						pthread_mutex_lock( &sidMapLock );
						sidMap.clear();
						pthread_mutex_unlock( &sidMapLock );
						DbgLog( kLogNotice, "mbr_mig - external flush cache requested" );
					} );
}

void Mbrd_ProcessGetStats(StatBlock *stats)
{
	gMbrdGlobalMutex.WaitLock();
	memcpy( stats, &gStatBlock, sizeof(StatBlock) );
	gMbrdGlobalMutex.SignalLock();
	stats->fTotalUpTime = GetElapsedSeconds() - stats->fTotalUpTime;
	DbgLog( kLogDebug, "mbr_mig - Membership - Get stats" );
}

void Mbrd_ProcessResetStats(void)
{
	gMbrdGlobalMutex.WaitLock();
	memset( &gStatBlock, 0, sizeof(StatBlock) );
	gMbrdGlobalMutex.SignalLock();
	DbgLog( kLogDebug, "mbr_mig - Membership - Reset stats" );
}

void Mbrd_ProcessDumpState(void)
{
	MbrdCache_DumpState( gMbrdCache );
	DbgLog( kLogDebug, "mbr_mig - Membership - Dump State" );
}

void Mbrd_SetMembershipThread( bool bActive )
{
	// need to track the amount of times we enable so we don't disable blindly (handle recursion)
	long current = (long) pthread_getspecific( gMembershipThreadKey );
	pthread_setspecific( gMembershipThreadKey, (void *) (bActive == true ? ++current : --current) );
}

bool Mbrd_IsMembershipThread( void )
{
	return (pthread_getspecific(gMembershipThreadKey) != NULL);
}

bool dsIsUserMemberOfGroup( const char *inUsername, const char *inGroupName )
{
	//check if user is in the groupName group
	bool						returnVal	= false;
	guid_t						user_uuid;
	guid_t						group_uuid;
	kauth_identity_extlookup	request		= { 0 };
	
	// note the length is not required, it's not checked for c-strings
	if ( Mbrd_ProcessMapIdentifier(ID_TYPE_USERNAME, inUsername, -1, &user_uuid) != KERN_SUCCESS ) {
		DbgLog( kLogNotice, "dsIsUserMemberOfGroup - Unable to map user %s to UUID", inUsername );
		return false;
	}
	
	if ( Mbrd_ProcessMapIdentifier(ID_TYPE_GROUPNAME, inGroupName, -1, &group_uuid) != KERN_SUCCESS ) {
		DbgLog( kLogNotice, "dsIsUserMemberOfGroup - Unable to map group %s to UUID", inGroupName );
		return false;
	}
	
	// we formulate the request ourself here so no need to dispatch to ourself..
	request.el_flags = KAUTH_EXTLOOKUP_VALID_UGUID | KAUTH_EXTLOOKUP_VALID_GGUID | KAUTH_EXTLOOKUP_WANT_MEMBERSHIP;
	memcpy( &request.el_uguid, &user_uuid, sizeof(guid_t) );
	memcpy( &request.el_gguid, &group_uuid, sizeof(guid_t) );
	
	Mbrd_ProcessLookup( &request );
	
	if ( (request.el_flags & KAUTH_EXTLOOKUP_VALID_MEMBERSHIP) != 0 && (request.el_flags & KAUTH_EXTLOOKUP_ISMEMBER) != 0 ) {
		returnVal = true;
	}
	
	return returnVal;
}

#else

#include "Mbrd_MembershipResolver.h"
#include <membership.h>
#include <membershipPriv.h>
#include "CSharedData.h"

bool dsIsUserMemberOfGroup( const char *inUsername, const char *inGroupName )
{
	uuid_t uu;
	uuid_t group;
	int isMember;
	
	if (mbr_user_name_to_uuid(inUsername, uu) == 0) {
		if (mbr_group_name_to_uuid(inGroupName, group) == 0) {
			if (mbr_check_membership(uu, group, &isMember) == 0) {
				return isMember != 0;
			}
		}
	}
	return false;
}

void dsFlushMembershipCache( void )
{
	mbr_reset_cache();
}

void dsSetNodeCacheAvailability( char *inNodeName, int inAvailable )
{
	
}

void dsFlushLibinfoCache( void )
{
	
}


#endif // DISABLE_SEARCH_PLUGIN
