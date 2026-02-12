/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#ifndef DISABLE_LOCAL_PLUGIN

#include "AuthHelperUtils.h"
#include "DSUtils.h"
#include "CDSPluginUtils.h"
#include "CRefTable.h"
#include <PasswordServer/AuthFile.h>

extern pid_t gDaemonPID;
extern in_addr_t gDaemonIPAddress;
extern CRefTable gRefTable;

// ---------------------------------------------------------------------------
//	* ParseLocalCacheUserAuthData
//    retrieve network nodename, user recordname, and user generated UID from authdata
//    format is version;tag;data
// ---------------------------------------------------------------------------

tDirStatus ParseLocalCacheUserAuthData(		const char	   *inAuthData,
											char		  **outNodeName,
											char		  **outRecordName,
											char		  **outGUID )
{
	char* authData = NULL;
	char* current = NULL;
	char* tempPtr = NULL;
	tDirStatus result = eDSAuthFailed;

	if ( inAuthData == NULL )
	{
		return eDSEmptyBuffer;
	}
	if ( outNodeName == NULL )
	{
		return eDSEmptyNodeName;
	}
	if ( outRecordName == NULL )
	{
		return eDSEmptyRecordName;
	}
	if ( outGUID == NULL )
	{
		return eDSEmptyParameter;
	}

	authData = strdup(inAuthData);
	if (authData == NULL)
	{
		return eDSAuthFailed;
	}
	current = authData;
	do {
		tempPtr = strsep(&current, ":");
		if (tempPtr == NULL)
		{
			result = eDSEmptyNodeName;
			break;
		}
		*outNodeName = strdup(tempPtr);
		
		tempPtr = strsep(&current, ":");
		if (tempPtr == NULL)
		{
			result = eDSEmptyRecordName;
			break;
		}
		*outRecordName = strdup(tempPtr);
		
		tempPtr = strsep(&current, ":");
		if (tempPtr == NULL)
		{
			result = eDSEmptyParameter;
			break;
		}
		*outGUID = strdup(tempPtr);
		
		result = eDSNoErr;
	} while (false);
	
	free(authData);
	authData = NULL;
	
	if (result != eDSNoErr)
	{
		if (*outNodeName != NULL)
		{
			free(*outNodeName);
			*outNodeName = NULL;
		}
		if (*outRecordName != NULL)
		{
			free(*outRecordName);
			*outRecordName = NULL;
		}
		if (*outGUID != NULL)
		{
			free(*outGUID);
			*outGUID = NULL;
		}
	}
	return result;
} // ParseLocalCacheUserAuthData


//------------------------------------------------------------------------------------
//	GetStateFilePath
//
//	Returns: ds err code
//
//  <outStateFilePath> is malloc'd and must be freed by the caller.
//------------------------------------------------------------------------------------

tDirStatus GetStateFilePath( const char *inHashPath, char **outStateFilePath )
{
	tDirStatus siResult = eDSNoErr;
	
	if ( inHashPath == NULL || outStateFilePath == NULL )
		return eParameterError;
	
	*outStateFilePath = NULL;
	
	char *stateFilePath = (char *) calloc( strlen(inHashPath) + sizeof(kShadowHashStateFileSuffix) + 1, 1 );
	if ( stateFilePath != NULL )
	{
		strcpy( stateFilePath, inHashPath );
		strcat( stateFilePath, kShadowHashStateFileSuffix );
		*outStateFilePath = stateFilePath;
	}
	else
	{
		siResult = eMemoryError;
	}
	
	return siResult;
}


//--------------------------------------------------------------------------------------------------
// * ReadShadowHash ()
//
//  <outUserHashPath> can be NULL, if non-null and a value is returned, caller must free.
//--------------------------------------------------------------------------------------------------

tDirStatus ReadShadowHash(
	const char *inUserName,
	const char *inGUIDString,
	unsigned char outHashes[kHashTotalLength],
	struct timespec *outModTime,
	char **outUserHashPath,
	SInt32 *outHashDataLen,
	bool readHashes )
{
	tDirStatus	siResult							= eDSAuthFailed;
	char		*path								= NULL;
	char		hexHashes[kHashTotalHexLength + 1]	= { 0 };
	SInt32		readBytes							= 0;
	UInt32		outBytes							= 0;
	CFile		*hashFile							= NULL;
	UInt32		pathSize							= 0;
	
	try
	{
		if ( outModTime != NULL )
		{
			outModTime->tv_sec = 0;
			outModTime->tv_nsec = 0;
		}
		if ( outUserHashPath != NULL )
			*outUserHashPath = NULL;
		if ( outHashDataLen != NULL )
			*outHashDataLen = 0;
		
		if (inGUIDString != NULL)
		{
			pathSize = sizeof(kShadowHashDirPath) + strlen(inGUIDString) + 1;
		}
		else
		{
			pathSize = sizeof(kShadowHashDirPath) + strlen(inUserName) + 1;
		}
		
		path = (char*)::calloc(pathSize, 1);
		if ( path != NULL )
		{
			if (inGUIDString != NULL)
			{
				strcpy( path, kShadowHashDirPath );
				strcat( path, inGUIDString );
			}
			else
			{
				strcpy( path, kShadowHashDirPath );
				strcat( path, inUserName );
			}
			
			// CFile throws, so let's catch, otherwise our logic won't work, could use stat,
			// but for consistency using try/catch
			try {
				hashFile = new CFile(path, false);
			} catch ( ... ) {
				
			}
			
			if (hashFile != NULL && hashFile->is_open())
			{
				if ( outModTime != NULL )
					hashFile->ModDate( outModTime );
				
				if ( readHashes )
				{
					bzero( hexHashes, sizeof(hexHashes) );
					readBytes = hashFile->ReadBlock( hexHashes, kHashTotalHexLength );
					delete(hashFile);
					hashFile = NULL;
					
					// should check the right number of bytes is there
					if ( readBytes < kHashShadowBothHexLength ) throw( eDSAuthFailed );
					HexToBinaryConversion( hexHashes, &outBytes, outHashes );
					if ( readBytes == (kHashTotalLength - 16)*2 )
					{
						memmove( outHashes + kHashOffsetToSaltedSHA1, outHashes + kHashOffsetToSaltedSHA1 - 16,
							readBytes - kHashOffsetToSaltedSHA1 );
						bzero( outHashes + kHashOffsetToCramMD5, kHashCramLength );
					}
				}
				siResult = eDSNoErr;
			}
			else //support older hash files
			{
				if (hashFile != NULL)
				{
					delete(hashFile);
					hashFile = NULL;
				}
				free( path );
				path = NULL;
				path = (char*)::calloc(sizeof(kShadowHashOldDirPath) + strlen(inUserName) + 1, 1);
				if ( path != NULL )
				{
					sprintf(path, "%s%s", kShadowHashOldDirPath, inUserName);
					
					// CFile throws so we must catch...
					try
					{
						hashFile = new CFile(path, false);
						
						if (hashFile->is_open())
						{
							if ( outModTime != NULL )
								hashFile->ModDate( outModTime );
							
							if ( readHashes )
							{
								//old hash file format has only kHashShadowBothHexLength bytes
								readBytes = hashFile->ReadBlock( hexHashes, kHashShadowBothHexLength );
								delete(hashFile);
								hashFile = NULL;
								// should check the right number of bytes is there
								if ( readBytes != kHashShadowBothHexLength ) throw( eDSAuthFailed );
								HexToBinaryConversion( hexHashes, &outBytes, outHashes );
							}
							siResult = eDSNoErr;
						}
					}
					catch( ... )
					{
					}
				}
			}
		}
	}
	catch( ... )
    {
        siResult = eDSAuthFailed;
    }
	
	if ( path != NULL )
	{
		if ( outUserHashPath != NULL )
		{
			*outUserHashPath = path;
		}
		else
		{
			free( path );
			path = NULL;
		}
	}
	if (hashFile != NULL)
	{
		delete(hashFile);
		hashFile = NULL;
	}
	
	bzero(hexHashes, kHashTotalHexLength);
	
	if ( outHashDataLen != NULL )
		*outHashDataLen = outBytes;
	
	return( siResult );
} // ReadShadowHash


tDirStatus ReadStateFile(
	const char *inUserName,
	const char *inGUIDString,
	struct timespec *outModTime,
	char **outUserHashPath,
	char **outStateFilePath,
	sHashState *inOutHashState,
	SInt32 *outHashDataLen )
{
	unsigned char hashes[kHashTotalLength];
	
	if ( outStateFilePath == NULL || outUserHashPath == NULL )
		return eParameterError;
	
	*outStateFilePath = NULL;
	
	tDirStatus siResult = ReadShadowHash( inUserName, inGUIDString, hashes, outModTime, outUserHashPath,
		outHashDataLen, false );
	if ( siResult == eDSNoErr )
		siResult = GetStateFilePath( *outUserHashPath, outStateFilePath );
	if ( siResult == eDSNoErr && inOutHashState != NULL )
	{
		siResult = (tDirStatus)ReadHashStateFile( *outStateFilePath, inOutHashState );
		if (siResult != eDSNoErr)
		{
			//We have a state file path but nothing is there right now.
			//At the end of the shadow hash auth it will be correctly written
			//so don't fail this call.
			siResult = eDSNoErr;
		}
	}
	
	return siResult;
} // ReadStateFile


//----------------------------------------------------------------------------------------------------
//  ReadHashStateFile
//
//  Returns: -1 = error, 0 = ok.
//----------------------------------------------------------------------------------------------------

int ReadHashStateFile( const char *inFilePath, sHashState *inOutHashState )
{
	CFStringRef myReplicaDataFilePathRef;
	CFURLRef myReplicaDataFileRef;
	CFReadStreamRef myReadStreamRef;
	CFPropertyListRef myPropertyListRef;
	CFStringRef errorString;
	CFPropertyListFormat myPLFormat;
	struct stat sb;
	CFMutableDictionaryRef stateDict = NULL;
	CFStringRef keyString = NULL;
	CFDateRef dateValue = NULL;
	CFNumberRef numberValue = NULL;
	long aLongValue;
	short aShortValue;
	int returnValue = 0;
	
	if ( inFilePath == NULL || inOutHashState == NULL )
		return -1;
	
	do
	{
		if ( stat( inFilePath, &sb ) != 0 )
		{
			time_t now;
			
			// initialize the creation date.
			time( &now );
			gmtime_r( &now, &inOutHashState->creationDate );
			returnValue = -1;
			break;
		}
		
		myReplicaDataFilePathRef = CFStringCreateWithCString( kCFAllocatorDefault, inFilePath, kCFStringEncodingUTF8 );
		if ( myReplicaDataFilePathRef == NULL )
		{
			returnValue = -1;
			break;
		}
	
		myReplicaDataFileRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, myReplicaDataFilePathRef,
			kCFURLPOSIXPathStyle, false );
	
		CFRelease( myReplicaDataFilePathRef );
	
		if ( myReplicaDataFileRef == NULL )
		{
			returnValue = -1;
			break;
		}
		
		myReadStreamRef = CFReadStreamCreateWithFile( kCFAllocatorDefault, myReplicaDataFileRef );
	
		CFRelease( myReplicaDataFileRef );
	
		if ( myReadStreamRef == NULL )
		{
			returnValue = -1;
			break;
		}
		
		CFReadStreamOpen( myReadStreamRef );
	
		errorString = NULL;
		myPLFormat = kCFPropertyListXMLFormat_v1_0;
		myPropertyListRef = CFPropertyListCreateFromStream( kCFAllocatorDefault, myReadStreamRef, 0,
			kCFPropertyListMutableContainersAndLeaves, &myPLFormat, &errorString );
		
		CFReadStreamClose( myReadStreamRef );
		CFRelease( myReadStreamRef );
		
		if ( errorString != NULL )
		{
			char errMsg[256];
			
			if ( CFStringGetCString( errorString, errMsg, sizeof(errMsg), kCFStringEncodingUTF8 ) )
				DbgLog(kLogPlugin, "ReadHashStateFile: could not load the state file, error = %s", errMsg );
			CFRelease( errorString );
		}
		
		if ( myPropertyListRef == NULL )
		{
			DbgLog(kLogPlugin, "ReadHashStateFile: could not load the hash state file because the property list is empty." );
			returnValue = -1;
			break;
		}
		
		if ( CFGetTypeID(myPropertyListRef) != CFDictionaryGetTypeID() )
		{
			CFRelease( myPropertyListRef );
			DbgLog(kLogPlugin, "ReadHashStateFile: could not load the hash state file because the property list is not a dictionary." );
			returnValue = -1;
			break;
		}
		
		stateDict = (CFMutableDictionaryRef)myPropertyListRef;
					
		keyString = CFStringCreateWithCString( kCFAllocatorDefault, "CreationDate", kCFStringEncodingUTF8 );
		if ( keyString != NULL )
		{
			if ( CFDictionaryGetValueIfPresent( stateDict, keyString, (const void **)&dateValue ) )
			{
				pwsf_ConvertCFDateToBSDTime( dateValue, &inOutHashState->creationDate );
			}
			CFRelease( keyString );
		}
		keyString = CFStringCreateWithCString( kCFAllocatorDefault, "LastLoginDate", kCFStringEncodingUTF8 );
		if ( keyString != NULL )
		{
			if ( CFDictionaryGetValueIfPresent( stateDict, keyString, (const void **)&dateValue ) )
			{
				pwsf_ConvertCFDateToBSDTime( dateValue, &inOutHashState->lastLoginDate );
			}
			CFRelease( keyString );
		}
		keyString = CFStringCreateWithCString( kCFAllocatorDefault, "FailedLoginCount", kCFStringEncodingUTF8 );
		if ( keyString != NULL )
		{
			if ( CFDictionaryGetValueIfPresent( stateDict, keyString, (const void **)&numberValue ) &&
					CFGetTypeID(numberValue) == CFNumberGetTypeID() &&
					CFNumberGetValue( (CFNumberRef)numberValue, kCFNumberLongType, &aLongValue) )
			{
				inOutHashState->failedLoginAttempts = (UInt16)aLongValue;
			}
			CFRelease( keyString );
		}
		keyString = CFStringCreateWithCString( kCFAllocatorDefault, "NewPasswordRequired", kCFStringEncodingUTF8 );
		if ( keyString != NULL )
		{
			if ( CFDictionaryGetValueIfPresent( stateDict, keyString, (const void **)&numberValue ) &&
					CFGetTypeID(numberValue) == CFNumberGetTypeID() &&
					CFNumberGetValue( (CFNumberRef)numberValue, kCFNumberSInt16Type, &aShortValue) )
			{
				inOutHashState->newPasswordRequired = (UInt16)aShortValue;
			}
			CFRelease( keyString );
		}
	
		CFRelease( stateDict );
		
	} while( false );
	
	return returnValue;
}


tDirStatus GetUserPolicies(
	CFMutableDictionaryRef inMutableRecordDict,
	sHashState* inState,
	CDSLocalPlugin* inPlugin,
	char** outPolicyStr )
{
	tDirStatus dirStatus = eDSNoErr;
	char* internalPolicyStr = NULL;
	long length = 0;
	char* cStr = NULL;
	size_t cStrSize = 0;
	
	try
	{
		if ( outPolicyStr == NULL )
			throw( eParameterError );
		*outPolicyStr = NULL;
		
		CFStringRef nativeAttr = inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPasswordPolicyOptions ) );
		CFArrayRef attrValues = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict, nativeAttr );
		CFStringRef xmlPolicyString = NULL;
		if ( ( attrValues != NULL ) && ( CFArrayGetCount( attrValues ) > 0 ) )
			xmlPolicyString = (CFStringRef)CFArrayGetValueAtIndex( attrValues, 0 );
		
		if ( ( xmlPolicyString != NULL ) && ( CFStringGetLength( xmlPolicyString ) > 0 ) )
			ConvertXMLPolicyToSpaceDelimited( CStrFromCFString( xmlPolicyString, &cStr, &cStrSize, NULL ),
				&internalPolicyStr );

		// prefix state information if requested
		if ( inState != NULL )
		{
			if ( internalPolicyStr != NULL )
				length = ::strlen( internalPolicyStr );
			*outPolicyStr = (char*)::malloc( sizeof(kPWPolicyStr_newPasswordRequired) + 3 + length );
			if ( (*outPolicyStr) != NULL )
			{
				::strcpy( (*outPolicyStr), kPWPolicyStr_newPasswordRequired );
				::strcpy( (*outPolicyStr) + sizeof(kPWPolicyStr_newPasswordRequired) - 1,
					inState->newPasswordRequired ? "=1" : "=0" );
				if ( internalPolicyStr != NULL && length > 0 )
				{
					*((*outPolicyStr) + sizeof(kPWPolicyStr_newPasswordRequired) + 1) = ' ';
					::strcpy( (*outPolicyStr) + sizeof(kPWPolicyStr_newPasswordRequired) + 2, internalPolicyStr );
					::free( internalPolicyStr );
				}
			}
		}
		else
			*outPolicyStr = internalPolicyStr;
	}
	catch( tDirStatus err )
	{
		DbgLog(  kLogPlugin, "GetUserPolicies(): got error %d", err );
		dirStatus = err;
	}
	
	if ( cStr != NULL )
		::free( cStr );
	
	return dirStatus;
}


tDirStatus
OpenPasswordServerNode(
	CDSLocalPlugin *inPlugin,
	CFMutableDictionaryRef inNodeDict,
	const char *inServerAddr,
	tDirReference *outDSRef,
	tDirNodeReference *outNodeRef )
{
	tDirStatus status = eDSNoErr;
	tDirReference pwsDirRef = 0;
	tDirNodeReference pwsNodeRef = 0;
	
	try
	{
		CFNumberRef pwsDirRefNumber = (CFNumberRef)CFDictionaryGetValue( inNodeDict, CFSTR(kNodePWSDirRef) );
		if ( pwsDirRefNumber != NULL )
			CFNumberGetValue( pwsDirRefNumber, kCFNumberLongType, &pwsDirRef );
		if ( pwsDirRef == 0 )
		{
			status = inPlugin->GetDirServiceRef( &pwsDirRef );
			if ( status != eDSNoErr )
				throw( status );
			
			pwsDirRefNumber = CFNumberCreate( NULL, kCFNumberLongType, &pwsDirRef );
			CFDictionaryAddValue( inNodeDict, CFSTR(kNodePWSDirRef), pwsDirRefNumber );
			CFRelease( pwsDirRefNumber );
		}
		
		CFNumberRef pwsNodeRefNumber = (CFNumberRef)CFDictionaryGetValue( inNodeDict, CFSTR(kNodePWSNodeRef) );
		if ( pwsNodeRefNumber != NULL )
			CFNumberGetValue( pwsNodeRefNumber, kCFNumberLongType, &pwsNodeRef );
		if ( pwsNodeRef == 0 )
		{
			CFStringRef pwsNodeName = CFStringCreateWithFormat( NULL, NULL, CFSTR("/PasswordServer/%s"), inServerAddr );
			if ( pwsNodeName == NULL )
				throw ( eMemoryError );
			
			status = CDSLocalPlugin::OpenDirNodeFromPath( pwsNodeName, pwsDirRef, &pwsNodeRef );
			CFRelease( pwsNodeName );
			if ( status != eDSNoErr )
				throw( status );
			
			pwsNodeRefNumber = CFNumberCreate( NULL, kCFNumberLongType, &pwsNodeRef );
			CFDictionaryAddValue( inNodeDict, CFSTR(kNodePWSNodeRef), pwsNodeRefNumber );
			CFRelease( pwsNodeRefNumber );
		}
	}
	catch ( tDirStatus catchStatus )
	{
		status = catchStatus;
	}
	
	*outDSRef = pwsDirRef;
	*outNodeRef = pwsNodeRef;
	
	return status;
}


tDirStatus
OpenLDAPNode(
	CDSLocalPlugin *inPlugin,
	CFMutableDictionaryRef inNodeDict,
	tDataListPtr inNodeName,
	tDirReference *outDSRef,
	tDirNodeReference *outNodeRef )
{
	tDirStatus status = eDSNoErr;
	tDirReference ldapDirRef = 0;
	tDirNodeReference ldapNodeRef = 0;
	CFNumberRef ldapNodeRefNumber = NULL;
	
	try
	{
		CFNumberRef ldapDirRefNumber = (CFNumberRef)CFDictionaryGetValue( inNodeDict, CFSTR(kNodeLDAPDirRef) );
		if ( ldapDirRefNumber != NULL )
			CFNumberGetValue( ldapDirRefNumber, kCFNumberLongType, &ldapDirRef );
		if ( ldapDirRef == 0 )
		{
			status = inPlugin->GetDirServiceRef( &ldapDirRef );
			if ( status != eDSNoErr )
				throw( status );
			
			ldapDirRefNumber = CFNumberCreate( NULL, kCFNumberLongType, &ldapDirRef );
			CFDictionaryAddValue( inNodeDict, CFSTR(kNodeLDAPDirRef), ldapDirRefNumber );
			CFRelease( ldapDirRefNumber );
		}
		
		// Free prior LDAP node
		ldapNodeRefNumber = (CFNumberRef)CFDictionaryGetValue( inNodeDict, CFSTR(kNodeLDAPNodeRef) );
		if ( ldapNodeRefNumber != NULL )
		{
			CFNumberGetValue( ldapNodeRefNumber, kCFNumberLongType, &ldapNodeRef );
			if ( ldapNodeRef != 0 ) {
				dsCloseDirNode( ldapNodeRef );
				ldapNodeRef = 0;
			}
			
			CFDictionaryRemoveValue( inNodeDict, CFSTR(kNodeLDAPNodeRef) );
			ldapNodeRefNumber = NULL;
		}
		
		status = dsOpenDirNode( ldapDirRef, inNodeName, &ldapNodeRef );
		if ( status == eDSNoErr )
		{
			ldapNodeRefNumber = CFNumberCreate( NULL, kCFNumberLongType, &ldapNodeRef );
			CFDictionaryAddValue( inNodeDict, CFSTR(kNodeLDAPNodeRef), ldapNodeRefNumber );
			CFRelease( ldapNodeRefNumber );
		}
	}
	catch ( tDirStatus catchStatus )
	{
		status = catchStatus;
	}
	
	*outDSRef = ldapDirRef;
	*outNodeRef = ldapNodeRef;

	return status;
}


//--------------------------------------------------------------------------------------------------
// * LoadAuthAuthorities
//--------------------------------------------------------------------------------------------------

void LoadAuthAuthorities( CDSLocalPlugin* inPlugin, tRecordReference inRecordRef, CAuthAuthority &inOutAATank )
{
	tDirStatus status = eDSNoErr;
	tAttributeEntryPtr attributeInfo = NULL;
	tAttributeValueEntryPtr valueEntryPtr = NULL;
	
	status = inPlugin->GetRecAttribInfo( inRecordRef, kDSNAttrAuthenticationAuthority, &attributeInfo );
	if ( status == eDSNoErr && attributeInfo != NULL )
	{
		for ( UInt32 avIndex = 1; avIndex <= attributeInfo->fAttributeValueCount; avIndex++ )
		{
			status = inPlugin->GetRecAttrValueByIndex( inRecordRef, kDSNAttrAuthenticationAuthority, avIndex, &valueEntryPtr );
			if ( status == eDSNoErr ) {
				inOutAATank.AddValue( valueEntryPtr->fAttributeValueData.fBufferData );
				dsDeallocAttributeValueEntry( 0, valueEntryPtr );
			}
		}
		
		dsDeallocAttributeEntry( 0, attributeInfo );
	}
}


//--------------------------------------------------------------------------------------------------
// * SaveAuthAuthorities
//--------------------------------------------------------------------------------------------------

tDirStatus
SaveAuthAuthorities(
	CDSLocalPlugin* inPlugin,
	tDirNodeReference inNodeRef,
	const char *inUsername,
	CFStringRef inNativeRecType,
	CAuthAuthority &inAATank )
{
	tDirStatus siResult = eDSNoErr;
	tRecordReference recordRef = 0;
	CFStringRef recordName = NULL;
	
	if ( inNodeRef == 0 )
		return eDSInvalidNodeRef;
	if ( inUsername == NULL )
		return eDSUserUnknown;
	if ( inNativeRecType == NULL )
		return eParameterError;
	
	recordName = CFStringCreateWithCString( kCFAllocatorDefault, inUsername, kCFStringEncodingUTF8 );
	if ( recordName == NULL )
		return( eMemoryError );
		
	siResult = inPlugin->OpenRecord( inNativeRecType, recordName, &recordRef );
	if ( siResult == eDSNoErr )
		siResult = SaveAuthAuthoritiesWithRecordRef( inPlugin, inNodeRef, recordRef, inAATank );
	
	if ( recordRef != 0 )
	{
		gRefTable.RemoveReference( recordRef ); // need to manually close this ref otherwise our refs get out of hand
		recordRef = 0;
	}
	DSCFRelease( recordName );
	
	return siResult;
}


//--------------------------------------------------------------------------------------------------
// * SaveAuthAuthorities
//--------------------------------------------------------------------------------------------------

tDirStatus
SaveAuthAuthoritiesWithRecordRef(
	CDSLocalPlugin* inPlugin,
	tDirNodeReference inNodeRef,
	tRecordReference inRecordRef,
	CAuthAuthority &inAATank )
{
	tDirStatus siResult = eDSAuthFailed;
	UInt32 avIndex = 0;
	UInt32 avCount = 0;
	char *aaStr = NULL;
	CFMutableDictionaryRef nodeDict = NULL;
	CFStringRef preRootAuthString = NULL;
	tDataListPtr attrValueList = NULL;
	
	try
	{
		// retrieve the same nodeDict the plugin object is going to use 
		CFDictionaryRef openRecordDict = inPlugin->RecordDictForRecordRef( inRecordRef );
		if ( openRecordDict == NULL )
			throw( eDSInvalidRecordRef );
		
		nodeDict = (CFMutableDictionaryRef)CFDictionaryGetValue( openRecordDict, CFSTR(kOpenRecordDictNodeDict) );
		if ( nodeDict == NULL )
			throw( eDSInvalidNodeRef );
		
		preRootAuthString = (CFStringRef) CFDictionaryGetValue( nodeDict, CFSTR(kNodeAuthenticatedUserName) );
		if ( preRootAuthString != NULL )
			CFRetain( preRootAuthString );
		
		CFDictionarySetValue( nodeDict, CFSTR(kNodeAuthenticatedUserName), CFSTR("root") );
		
		avCount = inAATank.GetValueCount();
		if ( avCount == 0 )
		{
			siResult = inPlugin->RemoveAttribute( inRecordRef, CFSTR(kDSNAttrAuthenticationAuthority) );
		}
		else
		{
			attrValueList = dsDataListAllocate( 0 );
			
			for ( avIndex = 0; avIndex < avCount; avIndex++ )
			{
				aaStr = inAATank.GetValueAtIndex( avIndex );
				if ( aaStr != NULL )
				{
					siResult = dsAppendStringToListAlloc( 0, attrValueList, aaStr );
					DSFreeString( aaStr );
					if ( siResult != eDSNoErr )
						throw( siResult );
				}
			}
			
			// need to use SetAttributeValues() instead of RemoveAttribute() to avoid deleting the
			// shadow hash files.
			tDataNodePtr attrTypeNode = dsDataNodeAllocateString( 0, kDSNAttrAuthenticationAuthority );
			sSetAttributeValues apiData = { kSetAttributeValues, 0, inRecordRef, attrTypeNode, attrValueList };
			char *recTypeStr = inPlugin->GetRecordTypeFromRef( inRecordRef );
			
			siResult = inPlugin->SetAttributeValues( &apiData, recTypeStr );
			
			DSFreeString( recTypeStr );
			dsDataNodeDeAllocate( 0, attrTypeNode );
		}
		
		if ( siResult != eDSNoErr )
			throw( siResult );
	}
	catch( tDirStatus err )
	{
		DbgLog(  kLogPlugin, "SaveAuthAuthorities(): got error %d", err );
		siResult = err;
	}
	
	if ( preRootAuthString != NULL ) {
		CFDictionarySetValue( nodeDict, CFSTR(kNodeAuthenticatedUserName), preRootAuthString );
		CFRelease( preRootAuthString );
	}
	else {
		CFDictionaryRemoveValue( nodeDict, CFSTR(kNodeAuthenticatedUserName) );
	}
	
	DSFreeString( aaStr );
	if ( attrValueList != NULL ) {
		dsDataListDeallocate( 0, attrValueList );
		free( attrValueList );
	}
	
	return siResult;
}


//--------------------------------------------------------------------------------------------------
// * SetUserAuthAuthorityAsRoot
//--------------------------------------------------------------------------------------------------

tDirStatus
SetUserAuthAuthorityAsRoot(
	CFMutableDictionaryRef inMutableRecordDict,
	CDSLocalPlugin* inPlugin,
	CDSLocalPluginNode* inNode,
	CFStringRef inStdRecType,
	const char *inUsername,
	const char *inAuthAuthority,
	tDirNodeReference inNodeRef,
	bool inRemoveBasic )
{
	tDirStatus			siResult			= eDSAuthFailed;
	tRecordReference	recordRef			= 0;
	CFStringRef			authAuthorityCFStr	= NULL;
	CFMutableArrayRef	aaValueArray		= NULL;
	CFStringRef			recordName			= NULL;
	char				*aaVers				= NULL;
	char				*aaTag				= NULL;
	char				*aaData				= NULL;
	CAuthAuthority		aaTank;
	
	if ( inAuthAuthority == NULL )
		return( eDSAuthFailed );
		
	recordName = CFStringCreateWithCString( kCFAllocatorDefault, inUsername, kCFStringEncodingUTF8 );
	if ( recordName == NULL )
		return eMemoryError;
	
	siResult = inPlugin->OpenRecord( inStdRecType, recordName, &recordRef );
	CFRelease( recordName );
	if ( siResult != eDSNoErr )
		return siResult;
	
	// find the attribute value to replace, if any
	siResult = dsParseAuthAuthority( inAuthAuthority, &aaVers, &aaTag, &aaData );
	if ( siResult == eDSNoErr )
	{
		LoadAuthAuthorities( inPlugin, recordRef, aaTank );
		aaTank.SetValueForTag( aaTag, inAuthAuthority );
		
		if ( inRemoveBasic )
			aaTank.RemoveValueForTag( kDSTagAuthAuthorityBasic );
		
		DSFreeString( aaVers );
		DSFreeString( aaTag );
		DSFreeString( aaData );
		
		siResult = SaveAuthAuthoritiesWithRecordRef( inPlugin, inNodeRef, recordRef, aaTank );
	}

	// clean up
	if ( recordRef != 0 )
	{
		// force flush and close our ref
		sFlushRecord params = { kFlushRecord, 0, recordRef };
		inPlugin->FlushRecord( &params );
		gRefTable.RemoveReference( recordRef ); // need to manually close this ref otherwise our refs get out of hand
		recordRef = 0;
	}
	DSCFRelease( authAuthorityCFStr );
	DSCFRelease( aaValueArray );
	
	return( siResult );
}


//--------------------------------------------------------------------------------------------------
// * AddKerberosAuthAuthority
//--------------------------------------------------------------------------------------------------

void
AddKerberosAuthAuthority(
	tDirNodeReference inNodeRef,
	const char *inPrincName,
	const char *inAuthAuthorityTag,
	CAuthAuthority &inAuthAuthorityList,
	CFMutableDictionaryRef inMutableRecordDict,
	CDSLocalPlugin* inPlugin,
	CDSLocalPluginNode* inNode,
	CFStringRef inNativeRecType,
	bool inOKToChangeAuthAuthorities )
{
	if ( !inOKToChangeAuthAuthorities )
		return;
	
	// abort if the tag already exists
	char *kerbAA = inAuthAuthorityList.GetValueForTag( inAuthAuthorityTag );
	if ( kerbAA != NULL )
	{
		free( kerbAA );
		return;
	}
	
	// abort if the tag already exists as a disabled tag
	if ( inAuthAuthorityList.TagDisabled(inAuthAuthorityTag) )
		return;
	
	char *localKDCRealmStr = GetLocalKDCRealmWithCache( kLocalKDCRealmCacheTimeout );
	if ( localKDCRealmStr != NULL )
	{
		char kerbAAStr[1024];
		snprintf(
			kerbAAStr, sizeof(kerbAAStr),
			";%s;;%s@%s;%s;",
			inAuthAuthorityTag, inPrincName, localKDCRealmStr, localKDCRealmStr );
		
		inAuthAuthorityList.SetValueForTag( inAuthAuthorityTag, kerbAAStr );
		
		tDirStatus siResult = SetUserAuthAuthorityAsRoot(
								inMutableRecordDict,
								inPlugin,
								inNode,
								inPlugin->RecordStandardTypeForNativeType(inNativeRecType),
								inPrincName,
								kerbAAStr,
								inNodeRef,
								false );
		
		if ( siResult != eDSNoErr )
			DbgLog( kLogPlugin, "SetUserAuthAuthorityAsRoot = %d", siResult );
		
		DSFreeString( localKDCRealmStr );
	}
}


//--------------------------------------------------------------------------------------------------
// * GetLocalKDCRealmWithCache
//--------------------------------------------------------------------------------------------------

char *
GetLocalKDCRealmWithCache(
	time_t inMaxTimeUntilQuery )
{
	static pthread_mutex_t sLocalRealmMutex = PTHREAD_MUTEX_INITIALIZER;
	static char *localKDCRealmStr = NULL;
	static time_t queryTime = 0;
	
	time_t now = time(NULL);
	char *returnValue = NULL;
	
	pthread_mutex_lock( &sLocalRealmMutex );
	
	if ( (localKDCRealmStr == NULL) || (now - queryTime > inMaxTimeUntilQuery) )
	{
		DSFreeString( localKDCRealmStr );
		localKDCRealmStr = GetLocalKDCRealm();
	}
	
	returnValue = localKDCRealmStr ? strdup( localKDCRealmStr ) : NULL;
	
	pthread_mutex_unlock( &sLocalRealmMutex );
	
	return returnValue;
}


//------------------------------------------------------------------------------------
//	* SASLErrToDirServiceError
//------------------------------------------------------------------------------------

tDirStatus SASLErrToDirServiceError( int inSASLError )
{
    tDirStatus dirServiceErr = eDSAuthFailed;
	
    switch (inSASLError)
    {
        case SASL_CONTINUE:		dirServiceErr = eDSNoErr;					break;
        case SASL_OK:			dirServiceErr = eDSNoErr;					break;
        case SASL_FAIL:			dirServiceErr = eDSAuthFailed;				break;
        case SASL_NOMEM:		dirServiceErr = eMemoryError;				break;
        case SASL_BUFOVER:		dirServiceErr = eDSBufferTooSmall;			break;
        case SASL_NOMECH:		dirServiceErr = eDSAuthMethodNotSupported;	break;
        case SASL_BADPROT:		dirServiceErr = eDSAuthParameterError;		break;
        case SASL_NOTDONE:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_BADPARAM:		dirServiceErr = eDSAuthParameterError;		break;
        case SASL_TRYAGAIN:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_BADMAC:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_NOTINIT:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_INTERACT:		dirServiceErr = eDSAuthParameterError;		break;
        case SASL_BADSERV:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_WRONGMECH:	dirServiceErr = eDSAuthParameterError;		break;
        case SASL_BADAUTH:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_NOAUTHZ:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_TOOWEAK:		dirServiceErr = eDSAuthMethodNotSupported;	break;
        case SASL_ENCRYPT:		dirServiceErr = eDSAuthInBuffFormatError;	break;
        case SASL_TRANS:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_EXPIRED:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_DISABLED:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_NOUSER:		dirServiceErr = eDSAuthUnknownUser;			break;
        case SASL_BADVERS:		dirServiceErr = eDSAuthServerError;			break;
        case SASL_UNAVAIL:		dirServiceErr = eDSAuthNoAuthServerFound;	break;
        case SASL_NOVERIFY:		dirServiceErr = eDSAuthNoAuthServerFound;	break;
        case SASL_PWLOCK:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_NOCHANGE:		dirServiceErr = eDSAuthFailed;				break;
        case SASL_WEAKPASS:		dirServiceErr = eDSAuthBadPassword;			break;
        case SASL_NOUSERPASS:	dirServiceErr = eDSAuthFailed;				break;
    }
    
    return dirServiceErr;
}

#endif // DISABLE_LOCAL_PLUGIN
