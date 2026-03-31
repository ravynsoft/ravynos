/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#include "buffer_unpackers.h"
#include "DSUtils.h"

#define kMethodStr		",method=\""

//------------------------------------------------------------------------------------
//	* Get2FromBuffer
//------------------------------------------------------------------------------------

SInt32 Get2FromBuffer( tDataBufferPtr inAuthData, tDataList **inOutDataList, char **inOutItemOne, char **inOutItemTwo, unsigned int *outItemCount )
{
	SInt32			siResult		= eDSNoErr;
	tDataList		*dataList		= NULL;
	unsigned int	itemCount		= 0;
	
	try
	{
		// parse input first
		dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
		if ( dataList == nil ) throw( (SInt32)eDSInvalidBuffFormat );
		itemCount = dsDataListGetNodeCountPriv(dataList);
		if ( outItemCount != NULL )
			*outItemCount = itemCount;
		if ( itemCount < 2 ) throw( (SInt32)eDSInvalidBuffFormat );
		
		// this allocates a copy of the string
		*inOutItemOne = dsDataListGetNodeStringPriv(dataList, 1);
		if ( *inOutItemOne == nil ) throw( (SInt32)eDSInvalidBuffFormat );
		if ( strlen(*inOutItemOne) < 1 )
			siResult = eDSInvalidBuffFormat;
		
		// this allocates a copy of the string
		*inOutItemTwo = dsDataListGetNodeStringPriv(dataList, 2);
		if ( *inOutItemTwo == nil ) throw( (SInt32)eDSInvalidBuffFormat );
	}
	catch( SInt32 catchErr )
	{
		siResult = catchErr;
	}
	
	if ( inOutDataList != NULL )
	{
		*inOutDataList = dataList;
	}
	else
	{
		if ( dataList != NULL )
		{
			dsDataListDeallocatePriv(dataList);
			free(dataList);
			dataList = NULL;
		}
	}
	
	return siResult;
}


//------------------------------------------------------------------------------------
//	* GetNameAndDataFromBuffer
//------------------------------------------------------------------------------------

tDirStatus GetNameAndDataFromBuffer(
	tDataBufferPtr inAuthData,
	tDataList **inOutDataList,
	char **inOutItemOne,
	unsigned char **inOutItemTwo,
	UInt32 *outItemTwoLength,
	unsigned int *outItemCount )
{
	tDirStatus		siResult		= eDSNoErr;
	tDataList		*dataList		= NULL;
	unsigned int	itemCount		= 0;
	tDataNodePtr	dataNodePtr		= NULL;
	
	try
	{
		// parse input first
		dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
		if ( dataList == nil ) throw( (tDirStatus)eDSInvalidBuffFormat );
		itemCount = dsDataListGetNodeCountPriv(dataList);
		if ( outItemCount != NULL )
			*outItemCount = itemCount;
		if ( itemCount < 2 ) throw( (tDirStatus)eDSInvalidBuffFormat );
		
		// this allocates a copy of the string
		*inOutItemOne = dsDataListGetNodeStringPriv(dataList, 1);
		if ( *inOutItemOne == nil ) throw( (tDirStatus)eDSInvalidBuffFormat );
		if ( strlen(*inOutItemOne) < 1 )
			siResult = eDSInvalidBuffFormat;
		
		// this allocates a copy of the string
		siResult = dsDataListGetNodePriv( dataList, 2, &dataNodePtr );
		if ( siResult != eDSNoErr )
			throw( siResult );
		if ( dataNodePtr == NULL )
			throw( (tDirStatus)eDSInvalidBuffFormat );
		
		*inOutItemTwo = (unsigned char *) calloc( dataNodePtr->fBufferLength + 1, 1 );
		if ( (*inOutItemTwo) == NULL )
			throw( (tDirStatus)eMemoryError );
		
		memcpy( *inOutItemTwo, ((tDataBufferPriv*)dataNodePtr)->fBufferData, dataNodePtr->fBufferLength );
		*outItemTwoLength = dataNodePtr->fBufferLength;
	}
	catch( tDirStatus catchErr )
	{
		siResult = catchErr;
	}
	
	if ( inOutDataList != NULL )
	{
		*inOutDataList = dataList;
	}
	else
	{
		if ( dataList != NULL )
		{
			dsDataListDeallocatePriv(dataList);
			free(dataList);
			dataList = NULL;
		}
	}
	
	return siResult;
}

//------------------------------------------------------------------------------------
//	* GetDataFromAuthBuffer
//------------------------------------------------------------------------------------
tDirStatus GetDataFromAuthBuffer(tDataBufferPtr inAuthData, int nodeNum, unsigned char **outData, UInt32 *outLen)
{
	tDataNodePtr pDataNode;
	tDirStatus status = eDSInvalidBuffFormat;
	*outData = NULL;
	*outLen = 0;

	tDataListPtr dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
	if (dataList != NULL)
	{
		status = dsDataListGetNodePriv(dataList, nodeNum, &pDataNode);
		if ( status != eDSNoErr )
		{
			status = eDSInvalidBuffFormat;
			goto getdataerr;
		}

		if ( pDataNode->fBufferLength > 0 )
		{
			*outData = (unsigned char *) calloc(pDataNode->fBufferLength + 1, 1);
			if ( ! (*outData) )
			{
				status = eMemoryAllocError;
				goto getdataerr;
			}
			memcpy(*outData, ((tDataBufferPriv*)pDataNode)->fBufferData, pDataNode->fBufferLength);
			*outLen = pDataNode->fBufferLength;
		}
	}

getdataerr:
	if ( dataList )
	{
		dsDataListDeallocatePriv(dataList);
		free(dataList);
		dataList = NULL;
	}
	return status;
}

//------------------------------------------------------------------------------------
//	UnpackSambaBufferFirstThreeItems
//
//	Returns: ds err code
//------------------------------------------------------------------------------------

SInt32 UnpackSambaBufferFirstThreeItems( tDataBufferPtr inAuthData, tDataListPtr *outDataList, char **outUserName, unsigned char *outChallenge, UInt32 *outChallengeLen, unsigned char **outResponse, UInt32 *outResponseLen )
{
	SInt32				siResult					= eDSNoErr;
	tDataListPtr		dataList					= NULL;
	tDataNodePtr		pC8Node						= NULL;
	tDataNodePtr		pP24InputNode				= NULL;
	
	try
	{
		// parse input first
		dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
		if ( dataList == nil ) throw( (SInt32)eDSInvalidBuffFormat );
		if ( dsDataListGetNodeCountPriv(dataList) < 3 ) throw( (SInt32)eDSInvalidBuffFormat );
		
		// this allocates a copy of the string
		*outUserName = dsDataListGetNodeStringPriv(dataList, 1);
		if ( *outUserName == nil ) throw( (SInt32)eDSInvalidBuffFormat );
		if ( strlen(*outUserName) < 1 ) throw( (SInt32)eDSInvalidBuffFormat );
		
		// these are not copies
		siResult = dsDataListGetNodePriv(dataList, 2, &pC8Node);
		if ( pC8Node == nil ) throw( (SInt32)eDSInvalidBuffFormat );
		if ( pC8Node->fBufferLength > 16 ) throw( (SInt32)eDSInvalidBuffFormat);
		if ( siResult != eDSNoErr ) throw( (SInt32)eDSInvalidBuffFormat );
		memmove(outChallenge, ((tDataBufferPriv*)pC8Node)->fBufferData, pC8Node->fBufferLength);
		*outChallengeLen = pC8Node->fBufferLength;
		
		siResult = dsDataListGetNodePriv(dataList, 3, &pP24InputNode);
		if ( siResult != eDSNoErr || pP24InputNode == nil )
			throw( (SInt32)eDSInvalidBuffFormat );
		
		*outResponse = (unsigned char *) malloc( pP24InputNode->fBufferLength );
		if ( *outResponse == NULL )
			throw( (SInt32)eMemoryError );
		*outResponseLen = pP24InputNode->fBufferLength;
		memmove(*outResponse, ((tDataBufferPriv*)pP24InputNode)->fBufferData, pP24InputNode->fBufferLength);
	}
	catch ( SInt32 error )
	{
		siResult = error;
	}
	
	*outDataList = dataList;
	
	return siResult;
}


//------------------------------------------------------------------------------------
//	UnpackSambaBuffer
//
//	Returns: ds err code
//------------------------------------------------------------------------------------

SInt32 UnpackSambaBuffer( tDataBufferPtr inAuthData, char **outUserName, unsigned char *outC8, unsigned char *outP24 )
{
	SInt32				siResult					= eDSNoErr;
	tDataListPtr		dataList					= NULL;
	unsigned char		*response					= NULL;
	UInt32				challengeLen				= 0;
	UInt32				responseLen					= 0;
	unsigned char		challengeBytes[17];
	
	siResult = UnpackSambaBufferFirstThreeItems( inAuthData, &dataList, outUserName, challengeBytes, &challengeLen, &response, &responseLen );
	if ( siResult == eDSNoErr ) {
		if ( dsDataListGetNodeCountPriv(dataList) != 3 ) 
			siResult = eDSInvalidBuffFormat;
		if ( challengeLen != kHashShadowChallengeLength || responseLen != kHashShadowResponseLength )
			siResult = eDSInvalidBuffFormat;
		
		if ( siResult == eDSNoErr ) {
			memmove( outC8, challengeBytes, challengeLen );
			memmove( outP24, response, responseLen );
		}
	}
	
	if ( dataList != NULL ) {
		dsDataListDeallocatePriv( dataList );
		free( dataList );
	}
	
	if ( response != NULL )
		free( response );
	
	return siResult;
}


//------------------------------------------------------------------------------------
//	UnpackNTLMv2Buffer
//
//	Returns: ds err code
//------------------------------------------------------------------------------------

SInt32 UnpackNTLMv2Buffer(
	tDataBufferPtr inAuthData,
	char **outNIName,
	unsigned char *outChal,
	unsigned char **outDigest,
	UInt32		*outDigestLen,
	char **outSambaName,
	char **outDomain)
{
	SInt32				siResult					= eDSNoErr;
	tDataListPtr		dataList					= NULL;
	UInt32				challengeLen				= 0;
	
	siResult = UnpackSambaBufferFirstThreeItems( inAuthData, &dataList, outNIName, outChal, &challengeLen, outDigest, outDigestLen );
	if ( siResult == eDSNoErr ) {
		if ( dsDataListGetNodeCountPriv(dataList) != 5 || challengeLen != kHashShadowChallengeLength ) 
			siResult = eDSInvalidBuffFormat;
	}
	
	try
	{
		if ( siResult == eDSNoErr )
		{
			// this allocates a copy of the string
			*outSambaName = dsDataListGetNodeStringPriv(dataList, 4);
			if ( *outSambaName == nil ) throw( (SInt32)eDSInvalidBuffFormat );
			if ( strlen(*outSambaName) < 1 ) throw( (SInt32)eDSInvalidBuffFormat );
			
			// this allocates a copy of the string
			*outDomain = dsDataListGetNodeStringPriv(dataList, 5);
			if ( *outDomain == nil ) {
				*outDomain = (char *)calloc(1,1);
				if ( *outDomain == nil ) throw( (SInt32)eMemoryError );
			}
		}
	}
	catch ( SInt32 error )
	{
		siResult = error;
	}
	
	if (dataList != NULL) {
		dsDataListDeallocatePriv(dataList);
		free(dataList);
		dataList = NULL;
	}
	
	return siResult;
}


//------------------------------------------------------------------------------------
//	UnpackMSCHAPv2Buffer
//
//	Returns: ds err code
//------------------------------------------------------------------------------------

SInt32 UnpackMSCHAPv2Buffer(
	tDataBufferPtr inAuthData,
	char **outNIName,
	unsigned char *outChal,
	unsigned char **outPeerChal,
	unsigned char **outDigest,
	UInt32 *outDigestLen,
	char **outSambaName)
{
	SInt32				siResult					= eDSNoErr;
	tDataListPtr		dataList					= NULL;
	tDataNodePtr		pP24InputNode				= NULL;
	UInt32				challengeLen				= 0;
	UInt32				peerChallengeLen			= 0;
	
	siResult = UnpackSambaBufferFirstThreeItems( inAuthData, &dataList, outNIName, outChal, &challengeLen, outPeerChal, &peerChallengeLen );
	if ( siResult == eDSNoErr ) {
		if ( dsDataListGetNodeCountPriv(dataList) != 5 || challengeLen != 16 || peerChallengeLen != 16 ) 
			siResult = eDSInvalidBuffFormat;
	}
	
	try
	{
		if ( siResult == eDSNoErr )
		{
			siResult = dsDataListGetNodePriv( dataList, 4, &pP24InputNode );
			if ( siResult != eDSNoErr || pP24InputNode == NULL )
				throw( (SInt32)eDSInvalidBuffFormat );
			
			*outDigest = (unsigned char *) malloc( pP24InputNode->fBufferLength );
			if ( *outDigest == NULL )
				throw( (SInt32)eMemoryError );
			*outDigestLen = pP24InputNode->fBufferLength;
			memmove(*outDigest, ((tDataBufferPriv*)pP24InputNode)->fBufferData, pP24InputNode->fBufferLength);
			
			// this allocates a copy of the string
			*outSambaName = dsDataListGetNodeStringPriv( dataList, 5 );
			if ( *outSambaName == nil ) throw( (SInt32)eDSInvalidBuffFormat );
			if ( strlen(*outSambaName) < 1 ) throw( (SInt32)eDSInvalidBuffFormat );
		}
	}
	catch ( SInt32 error )
	{
		siResult = error;
	}
	
	if (dataList != NULL) {
		dsDataListDeallocatePriv(dataList);
		free(dataList);
		dataList = NULL;
	}
	
	return siResult;
}


//------------------------------------------------------------------------------------
//	UnpackMSCHAP2SessionKeyBuffer
//
//	Returns: ds err code
//------------------------------------------------------------------------------------

SInt32 UnpackMPPEKeyBuffer( tDataBufferPtr inAuthData, char **outUserName, unsigned char *outP24, int *outKeySize )
{
	SInt32				siResult					= eDSNoErr;
	tDataListPtr		dataList					= NULL;
	tDataNodePtr		pP24Node					= NULL;
	tDataNodePtr		pKeySizeNode				= NULL;
	
	try
	{
		// User name
		dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
		if ( dataList == nil ) throw( (SInt32)eDSInvalidBuffFormat );
		if ( dsDataListGetNodeCountPriv(dataList) != 3 ) throw( (SInt32)eDSInvalidBuffFormat );
		
		*outUserName = dsDataListGetNodeStringPriv(dataList, 1);
		if ( *outUserName == nil ) throw( (SInt32)eDSInvalidBuffFormat );
		if ( strlen(*outUserName) < 1 ) throw( (SInt32)eDSInvalidBuffFormat );
		
		// P24 - NT response
		siResult = dsDataListGetNodePriv(dataList, 2, &pP24Node);
		if ( siResult != eDSNoErr )
			throw( siResult );
		if ( pP24Node == NULL || pP24Node->fBufferLength != 24)
			throw( (SInt32)eDSInvalidBuffFormat );
		memmove(outP24, ((tDataBufferPriv*)pP24Node)->fBufferData, pP24Node->fBufferLength);
		
		// key size (8 or 16)
		siResult = dsDataListGetNodePriv(dataList, 3, &pKeySizeNode);
		if ( siResult != eDSNoErr || pKeySizeNode == NULL || pKeySizeNode->fBufferLength != 1 )
			throw( (SInt32)eDSInvalidBuffFormat );
		
		*outKeySize = ((tDataBufferPriv*)pKeySizeNode)->fBufferData[0];
		if ( *outKeySize != 8 && *outKeySize != 16 )
			throw( (SInt32)eDSInvalidBuffFormat );
	}
	catch ( SInt32 error )
	{
		siResult = error;
	}
	
	if (dataList != NULL) {
		dsDataListDeallocatePriv(dataList);
		free(dataList);
		dataList = NULL;
	}
	
	return siResult;
}


//------------------------------------------------------------------------------------
//	UnpackDigestBuffer
//
//	Returns: ds err code
//------------------------------------------------------------------------------------

SInt32 UnpackDigestBuffer( tDataBufferPtr inAuthData, char **outUserName, digest_context_t *digestContext )
{
	SInt32				siResult					= eDSNoErr;
	tDataListPtr		dataList					= NULL;
	tDataNodePtr		pResponseNode				= NULL;
	unsigned int		itemCount					= 0;
	char				*challenge					= NULL;
	char				*challengePlus				= NULL;
	char				*response					= NULL;
	char				*method						= NULL;
	int					saslResult					= 0;

	try
	{
		siResult = Get2FromBuffer( inAuthData, &dataList, outUserName, &challenge, &itemCount );
		if ( siResult != eDSNoErr )
			throw( siResult );

		// this allocates a copy of the string
		method = dsDataListGetNodeStringPriv( dataList, 4 );
		if ( method != NULL )
		{
			if ( strlen(method) < 1 )
				throw( (SInt32)eDSInvalidBuffFormat );

			challengePlus = (char *) malloc( strlen(challenge) + sizeof(kMethodStr) + strlen(method) + 1 );
			strcpy( challengePlus, challenge );
			strcat( challengePlus, kMethodStr );
			strcat( challengePlus, method );
			strcat( challengePlus, "\"" );
		}
		else
		{
			challengePlus = strdup( challenge );
		}
		
		// these are not copies
		siResult = dsDataListGetNodePriv( dataList, 3, &pResponseNode );
		if ( siResult != eDSNoErr ) throw( (SInt32)eDSInvalidBuffFormat );
		if ( pResponseNode == NULL ) throw( (SInt32)eDSInvalidBuffFormat );
		
		response = (char *) calloc( 1, pResponseNode->fBufferLength + 1 );
		memmove( response, ((tDataBufferPriv*)pResponseNode)->fBufferData, ((tDataBufferPriv*)pResponseNode)->fBufferLength );
		
		// parse the digest strings
		saslResult = digest_server_parse( challengePlus, strlen(challengePlus), response, digestContext );
		if ( saslResult != 0 )
			throw( (SInt32)eDSAuthFailed );
	}
	catch ( SInt32 error )
	{
		siResult = error;
	}
	
	if ( challenge != NULL ) {
		free( challenge );
		challenge = NULL;
	}
	if ( challengePlus != NULL ) {
		free( challengePlus );
		challengePlus = NULL;
	}
	if ( method != NULL ) {
		free( method );
		method = NULL;
	}
	if ( response != NULL ) {
		free( response );
		response = NULL;
	}
	if (dataList != NULL) {
		dsDataListDeallocatePriv(dataList);
		free(dataList);
		dataList = NULL;
	}
	
	return siResult;
}


//--------------------------------------------------------------------------------------------------
//	UnpackCramBuffer
//
//	Returns: ds err code
//--------------------------------------------------------------------------------------------------

SInt32 UnpackCramBuffer( tDataBufferPtr inAuthData, char **outUserName, char **outChal, unsigned char **outResponse, UInt32 *outResponseLen )
{
	SInt32				siResult					= eDSNoErr;
	tDataListPtr		dataList					= NULL;
	tDataNodePtr		pResponseNode				= NULL;
	unsigned int		itemCount					= 0;
	
	try
	{
		siResult = Get2FromBuffer( inAuthData, &dataList, outUserName, outChal, &itemCount );
		if ( siResult != eDSNoErr )
			throw( siResult );
		if ( itemCount != 3 )
			throw( (SInt32)eDSInvalidBuffFormat );
		
		siResult = dsDataListGetNodePriv( dataList, 3, &pResponseNode );
		if ( siResult != eDSNoErr || pResponseNode == NULL || pResponseNode->fBufferLength < 32 )
			throw( (SInt32)eDSInvalidBuffFormat );
		
		*outResponse = (unsigned char *) malloc( pResponseNode->fBufferLength );
		if ( *outResponse == NULL )
			throw( (SInt32)eMemoryError );
		*outResponseLen = pResponseNode->fBufferLength;
		memmove( *outResponse, ((tDataBufferPriv*)pResponseNode)->fBufferData, pResponseNode->fBufferLength );
	}
	catch ( SInt32 error )
	{
		siResult = error;
		
		if ( *outUserName != NULL ) {
			free( *outUserName );
			*outUserName = NULL;
		}
		if ( *outChal != NULL ) {
			free( *outChal );
			*outChal = NULL;
		}
	}
	
	if ( dataList != NULL ) {
		dsDataListDeallocatePriv( dataList );
		free( dataList );
		dataList = NULL;
	}
	
	return siResult;
}


//--------------------------------------------------------------------------------------------------
//	UnpackAPOPBuffer
//
//	Returns: ds err code
//--------------------------------------------------------------------------------------------------

SInt32 UnpackAPOPBuffer( tDataBufferPtr inAuthData, char **outUserName, char **outChal, char **outResponse )
{
	SInt32				siResult					= eDSNoErr;
	tDataListPtr		dataList					= NULL;
	unsigned int		itemCount					= 0;
	
	try
	{
		siResult = Get2FromBuffer( inAuthData, &dataList, outUserName, outChal, &itemCount );
		if ( siResult != eDSNoErr )
			throw( siResult );
		if ( itemCount != 3 )
			throw( (SInt32)eDSInvalidBuffFormat );
		
		// this allocates a copy of the string
		*outResponse = dsDataListGetNodeStringPriv( dataList, 3 );
		if ( *outResponse == NULL )
			throw( (SInt32)eDSInvalidBuffFormat );
	}
	catch ( SInt32 error )
	{
		siResult = error;
		
		if ( *outUserName != NULL ) {
			free( *outUserName );
			*outUserName = NULL;
		}
		if ( *outChal != NULL ) {
			free( *outChal );
			*outChal = NULL;
		}
	}
	
	if ( dataList != NULL ) {
		dsDataListDeallocatePriv( dataList );
		free( dataList );
		dataList = NULL;
	}
	
	return siResult;
}


//--------------------------------------------------------------------------------------------------
//	RepackBufferForPWServer
//
//	Replace the user name with the uesr id.
//--------------------------------------------------------------------------------------------------

SInt32 RepackBufferForPWServer ( tDataBufferPtr inBuff, const char *inUserID, UInt32 inUserIDNodeNum, tDataBufferPtr *outBuff )
{
	SInt32 result = eDSNoErr;
    tDataListPtr dataList = NULL;
    tDataNodePtr dataNode = NULL;
	UInt32 index, nodeCount;
	UInt32 uidLen;
                
    if ( !inBuff || !inUserID || !outBuff )
        return eDSAuthParameterError;
    
    try
    {	
        uidLen = strlen(inUserID);
        *outBuff = ::dsDataBufferAllocatePriv( inBuff->fBufferLength + uidLen + 1 );
        if ( *outBuff == nil ) throw( (SInt32)eMemoryError );
        
        (*outBuff)->fBufferLength = 0;
        
        dataList = dsAuthBufferGetDataListAllocPriv(inBuff);
        if ( dataList == nil ) throw( (SInt32)eDSInvalidBuffFormat );
        
        nodeCount = dsDataListGetNodeCountPriv(dataList);
        if ( nodeCount < 1 ) throw( (SInt32)eDSInvalidBuffFormat );
        
        for ( index = 1; index <= nodeCount; index++ )
        {
            if ( index == inUserIDNodeNum )
            {
                // write 4 byte length
                memcpy( (*outBuff)->fBufferData + (*outBuff)->fBufferLength, &uidLen, sizeof(UInt32) );
                (*outBuff)->fBufferLength += sizeof(UInt32);
                
                // write uid
                memcpy( (*outBuff)->fBufferData + (*outBuff)->fBufferLength, inUserID, uidLen );
                (*outBuff)->fBufferLength += uidLen;
            }
            else
            {
                // get a node
                result = dsDataListGetNodeAllocPriv(dataList, index, &dataNode);
                if ( result != eDSNoErr ) throw( (SInt32)eDSInvalidBuffFormat );
            
                // copy it
                memcpy((*outBuff)->fBufferData + (*outBuff)->fBufferLength, &dataNode->fBufferLength, sizeof(UInt32));
                (*outBuff)->fBufferLength += sizeof(UInt32);
                
                memcpy( (*outBuff)->fBufferData + (*outBuff)->fBufferLength, dataNode->fBufferData, dataNode->fBufferLength );
                (*outBuff)->fBufferLength += dataNode->fBufferLength;
                
                // clean up
                dsDataBufferDeallocatePriv(dataNode);
            }
            
        }
        
        (void)dsDataListDeallocatePriv(dataList);
        free(dataList);
    }
    
    catch( SInt32 error )
    {
        result = error;
    }
    
    return result;
} // RepackBufferForPWServer


// ---------------------------------------------------------------------------
//	* GetUserNameFromAuthBuffer
//    retrieve the username from a standard auth buffer
//    buffer format should be 4 byte length followed by username, then optional
//    additional data after. Buffer length must be at least 5 (length + 1 character name)
// ---------------------------------------------------------------------------

SInt32 GetUserNameFromAuthBuffer ( tDataBufferPtr inAuthData, UInt32 inUserNameIndex, 
											  char  **outUserName, int *outUserNameBufferLength )
{
	tDirStatus status = eDSNoErr;
	tDataNodePtr dataListNode = NULL;
	
	if ( outUserName == NULL )
		return eDSNullParameter;
	*outUserName = NULL;
	
	tDataListPtr dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
	if (dataList != NULL)
	{
		status = dsDataListGetNodePriv(dataList, inUserNameIndex, &dataListNode);
		if (status == eDSNoErr)
		{
			if (outUserNameBufferLength != NULL)
				*outUserNameBufferLength = dataListNode->fBufferLength;
			
			*outUserName = dsDataListGetNodeStringPriv(dataList, inUserNameIndex);
			// this allocates a copy of the string
		}
		
		dsDataListDeallocatePriv(dataList);
		free(dataList);
		dataList = NULL;
		if (*outUserName != NULL)
			return eDSNoErr;
		else
			return eDSNullParameter;
	}
	return eDSInvalidBuffFormat;
}


// ---------------------------------------------------------------------------
//	* UnpackUserWithAABuffer
// ---------------------------------------------------------------------------

SInt32 UnpackUserWithAABuffer( tDataBufferPtr inAuthData, UInt32 *outAACount, char **outAAList[] )
{
	SInt32			siResult		= eDSNoErr;
	tDataList		*dataList		= NULL;
	unsigned int	itemIndex		= 0;
	unsigned int	itemCount		= 0;
	
	if ( outAAList == NULL || outAACount == NULL )
		return eParameterError;
	
	dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
	if ( dataList == NULL )
		return eDSInvalidBuffFormat;
	
	itemCount = dsDataListGetNodeCountPriv(dataList);
	*outAACount = itemCount;
	*outAAList = (char **)calloc(sizeof(char *), itemCount + 1);
	if ( *outAAList != NULL )
	{	
		for ( itemIndex = 0; itemIndex < itemCount; itemIndex++ ) {
			(*outAAList)[itemIndex] = dsDataListGetNodeStringPriv(dataList, itemIndex + 1);
		}
	}
	else
	{
		siResult = eMemoryError;
	}
	
	if ( dataList != NULL )
	{
		dsDataListDeallocatePriv(dataList);
		free(dataList);
		dataList = NULL;
	}
	
	return siResult;
}


// ---------------------------------------------------------------------------
//	* GetKrbCredentialFromAuthBuffer
// ---------------------------------------------------------------------------

tDirStatus GetKrbCredentialFromAuthBuffer( tDataBufferPtr inAuthData, char **outUserName, char **outPrincipal, krb5_creds **outCred )
{
	tDirStatus			status			= eDSNoErr;
	tDataListPtr		dataList		= NULL;
	tDataBufferPriv		*dataListNode	= NULL;
	char				*userName		= NULL;
	char				*principal		= NULL;
	krb5_data			krb_data		= {0};
	krb5_context		kctx			= NULL;
	krb5_creds			**rcredsArray	= NULL;
	krb5_error_code		krbErr			= 0;
	krb5_auth_context	authContext		= NULL;
	krb5_rcache			rcache			= NULL;
	krb5_data			piece			= {0};
	
	if ( inAuthData == NULL || outUserName == NULL || outPrincipal == NULL || outCred == NULL )
		return eParameterError;
	
	*outUserName = NULL;
	*outPrincipal = NULL;
	*outCred = NULL;
	
	try
	{
		dataList = dsAuthBufferGetDataListAllocPriv( inAuthData );
		if ( dataList == NULL )
			throw( (tDirStatus)eMemoryError );
		
		// this allocates a copy of the string
		userName = dsDataListGetNodeStringPriv( dataList, 1 );
		if ( userName == NULL )
			throw( (tDirStatus)eDSInvalidBuffFormat );
		if ( strlen(userName) < 1 )
			throw( (tDirStatus)eDSInvalidBuffFormat );
			
		status = dsDataListGetNodePriv( dataList, 2, (tDataNodePtr *)&dataListNode );
		if ( status != eDSNoErr )
			throw( status );
		
		krb_data.length = dataListNode->fBufferLength;
		krb_data.data = (char *) calloc( 1, krb_data.length );
		if ( krb_data.data == NULL )
			throw( (tDirStatus)eMemoryError );
		
		memcpy( krb_data.data, dataListNode->fBufferData, dataListNode->fBufferLength );
		
		krbErr = krb5_init_context( &kctx );
		if ( krbErr != 0 )
			throw( (tDirStatus)eDSAuthFailed );
		
		krbErr = krb5_auth_con_init( kctx, &authContext );
		if ( krbErr != 0 )
			throw( (tDirStatus)eDSAuthFailed );
		
		krb5_address **addresses = NULL;
		krb5_os_localaddr( kctx, &addresses );
		
		krbErr = krb5_auth_con_setaddrs( kctx, authContext, *addresses, *addresses );
		if ( krbErr != 0 )
			throw( (tDirStatus)eDSAuthFailed );
		
		piece.data = strdup( userName );
		if ( piece.data == NULL )
			throw( (tDirStatus)eMemoryError );
		
		piece.length = strlen( userName );
		
		krbErr = krb5_get_server_rcache( kctx, &piece, &rcache );
		if ( krbErr != 0 )
			throw( (tDirStatus)eDSAuthFailed );
		
		krbErr = krb5_auth_con_setrcache( kctx, authContext, rcache );		
		if ( krbErr != 0 )
			throw( (tDirStatus)eDSAuthFailed );
		
		krbErr = krb5_rd_cred( kctx, authContext, &krb_data, &rcredsArray, NULL );
		if ( krbErr != 0 )
			throw( (tDirStatus)eDSAuthFailed );
		
		krbErr = krb5_unparse_name( kctx, (*rcredsArray)->client, &principal );
		if ( krbErr != 0 )
			throw( (tDirStatus)eDSAuthFailed );
		
		// success, set the return values
		*outUserName = userName;
		*outPrincipal = principal;
		*outCred = *rcredsArray;
	}
	catch ( tDirStatus catchStatus )
	{
		status = catchStatus;
		
		DSFreeString( userName );
		if ( principal != NULL )	
			krb5_free_unparsed_name( kctx, principal );
		if ( rcredsArray != NULL )
			krb5_free_creds( kctx, *rcredsArray );
	}
	
	if ( authContext != NULL )
		krb5_auth_con_free( kctx, authContext );
	
	if ( kctx != NULL )
		krb5_free_context( kctx );
	
	if ( dataList != NULL ) {
		dsDataListDeallocatePriv( dataList );
		free( dataList );
	}

	return status;
}


