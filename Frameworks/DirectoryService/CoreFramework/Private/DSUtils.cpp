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
 * @header DSUtils
 */

#include "CLog.h"
#include "DSUtils.h"
#include "SharedConsts.h"
#include "GetMACAddress.h"
#include <DirectoryService/DirServicesConst.h>
#include <DirectoryService/DirServicesConstPriv.h>
#include <SystemConfiguration/SCDynamicStore.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <mach/mach_time.h>	// for dsTimeStamp
#include <syslog.h>			// for syslog()
#include <sys/sysctl.h>		// for struct kinfo_proc and sysctl()


typedef struct AuthMethodMap {
	const char *name;
	int value;
} AuthMethodMap;

AuthMethodMap gAuthMethodTable[] = 
{
	{ kDSStdAuthClearText, kAuthClearText },
	{ kDSStdAuthNodeNativeClearTextOK, kAuthNativeClearTextOK },
	{ kDSStdAuthNodeNativeNoClearText, kAuthNativeNoClearText },
	{ kDSStdAuthCrypt, kAuthCrypt },
	{ kDSStdAuth2WayRandom, kAuth2WayRandom },
	{ kDSStdAuth2WayRandomChangePasswd, kAuth2WayRandomChangePass },
	{ kDSStdAuthSMB_NT_Key, kAuthSMB_NT_Key },
	{ kDSStdAuthSMB_LM_Key, kAuthSMB_LM_Key },
	{ kDSStdAuthSecureHash, kAuthSecureHash },
	{ kDSStdAuthReadSecureHash, kAuthReadSecureHash },
	{ kDSStdAuthWriteSecureHash, kAuthWriteSecureHash },
	{ kDSStdAuthSetPasswd, kAuthSetPasswd },
	{ kDSStdAuthSetPasswdAsRoot, kAuthSetPasswdAsRoot },
	{ kDSStdAuthSetComputerAcctPasswdAsRoot, kAuthSetComputerAcctPasswdAsRoot },
	{ kDSStdAuthChangePasswd, kAuthChangePasswd },
	{ kDSStdAuthAPOP, kAuthAPOP },
	{ kDSStdAuthCRAM_MD5, kAuthCRAM_MD5 },
	{ kDSStdAuthSetPolicy, kAuthSetPolicy },
	{ kDSStdAuthWithAuthorizationRef, kAuthWithAuthorizationRef },
	{ kDSStdAuthGetPolicy, kAuthGetPolicy },
	{ kDSStdAuthGetGlobalPolicy, kAuthGetGlobalPolicy },
	{ kDSStdAuthSetGlobalPolicy, kAuthSetGlobalPolicy },
	{ "dsAuthMethodSetPasswd:dsAuthNodeNativeCanUseClearText", kAuthSetPasswdCheckAdmin },
	{ kDSStdAuthSetPolicyAsRoot, kAuthSetPolicyAsRoot },
	{ kDSStdAuthSetLMHash, kAuthSetLMHash },
	{ "dsAuthMethodStandard:dsAuthVPN_MPPEMasterKeys", kAuthVPN_PPTPMasterKeys },
	{ "dsAuthMethodStandard:dsAuthVPN_PPTPMasterKeys", kAuthVPN_PPTPMasterKeys },
	{ kDSStdAuthMPPEMasterKeys, kAuthVPN_PPTPMasterKeys },
	{ kDSStdAuthGetKerberosPrincipal, kAuthGetKerberosPrincipal },
	{ kDSStdAuthGetEffectivePolicy, kAuthGetEffectivePolicy },
	{ kDSStdAuthSetWorkstationPasswd, kAuthNTSetWorkstationPasswd },
	{ kDSStdAuthSMBWorkstationCredentialSessionKey, kAuthSMBWorkstationCredentialSessionKey },
	{ kDSStdAuthSetNTHash, kAuthSMB_NTUserSessionKey },
	{ kDSStdAuthSMB_NT_UserSessionKey, kAuthSMB_NTUserSessionKey },
	{ kDSStdAuthGetUserName, kAuthGetUserName },
	{ kDSStdAuthSetUserName, kAuthSetUserName },
	{ kDSStdAuthGetUserData, kAuthGetUserData },
	{ kDSStdAuthSetUserData, kAuthSetUserData },
	{ kDSStdAuthDeleteUser, kAuthDeleteUser },
	{ kDSStdAuthNewUser, kAuthNewUser },
	{ kDSStdAuthNewComputer, kAuthNewComputer },
	{ kDSStdAuthNTLMv2, kAuthNTLMv2 },
	{ kDSStdAuthMSCHAP2, kAuthMSCHAP2 },
	{ kDSStdAuthDIGEST_MD5, kAuthDIGEST_MD5 },
	{ kDSStdAuth2WayRandom, kAuth2WayRandom },
	{ kDSStdAuth2WayRandomChangePasswd, kAuth2WayRandomChangePass },
	{ kDSStdAuthNewUserWithPolicy, kAuthNewUserWithPolicy },
	{ kDSStdAuthSetShadowHashWindows, kAuthSetShadowHashWindows },
	{ kDSStdAuthSetShadowHashSecure, kAuthSetShadowHashSecure },
	{ kDSStdAuthGetMethodsForUser, kAuthGetMethodListForUser },
	{ kDSStdAuthSMB_NT_WithUserSessionKey, kAuthNTSessionKey },
	{ kDSStdAuthKerberosTickets, kAuthKerberosTickets },
	{ kDSStdAuthNTLMv2WithSessionKey, kAuthNTLMv2WithSessionKey },
	{ "dsAuthMethodStandard:dsAuthMSLMCHAP2ChangePasswd", kAuthMSLMCHAP2ChangePasswd },
	{ "dsAuthMethodStandard:dsAuthNodePPS", kAuthPPS },
	{ kDSStdAuthNodeNativeRetainCredential, kAuthNativeRetainCredential },
	{ kDSStdAuthSetCertificateHashAsRoot, kAuthSetCertificateHashAsRoot },
	{ kDSStdAuthSASLProxy, kAuthSASLProxy },
	{ NULL, 0 }
};


//--------------------------------------------------------------------------------------------------
//	Name:	dsDataBufferAllocatePriv
//
//--------------------------------------------------------------------------------------------------

tDataBufferPtr dsDataBufferAllocatePriv ( UInt32 inBufferSize )
{
	UInt32				size	= 0;
	tDataBufferPtr		outBuff	= nil;

	size = (UInt32) (sizeof( tDataBufferPriv ) + inBufferSize);
	outBuff = (tDataBuffer *)::calloc( size + 1, sizeof( char ) );
	if ( outBuff != nil )
	{
		outBuff->fBufferSize = inBufferSize;
		outBuff->fBufferLength = 0;
	}

	return( outBuff );

} // dsDataBufferAllocatePriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsDataBufferDeallocatePriv
//
//--------------------------------------------------------------------------------------------------

tDirStatus dsDataBufferDeallocatePriv ( tDataBufferPtr inDataBufferPtr )
{
	tDirStatus		tdsResult	= eDSNoErr;

	if ( inDataBufferPtr != nil )
	{
		free( inDataBufferPtr );
		inDataBufferPtr = nil;
	}
	else
	{
		tdsResult = eDSNullDataBuff;
	}

	return( tdsResult );

} // dsDataBufferDeallocatePriv



//--------------------------------------------------------------------------------------------------
//	Name:	dsDataListAllocatePriv
//
//--------------------------------------------------------------------------------------------------

tDataList* dsDataListAllocatePriv ( void )
{
	tDataList		   *outResult	= nil;

	outResult = (tDataList *)::calloc( sizeof( tDataList ), sizeof( char ) );

	return( outResult );

} // dsDataListAllocatePriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsDataListDeallocatePriv
//
//--------------------------------------------------------------------------------------------------

tDirStatus dsDataListDeallocatePriv ( tDataListPtr inDataList  )
{
	tDirStatus			tdsResult	= eDSNoErr;
	tDataBufferPriv    *pPrivData	= nil;
	tDataBuffer		   *pTmpBuff	= nil;
	tDataBuffer		   *pDataBuff	= nil;

	if ( inDataList == nil )
	{
		return( eDSNullDataList );
	}

	if ( inDataList->fDataListHead != nil )
	{
		pDataBuff = inDataList->fDataListHead;
		
		inDataList->fDataListHead = nil;
		inDataList->fDataNodeCount = 0;
		
		pPrivData = (tDataBufferPriv *)pDataBuff;
		while ( pDataBuff != nil )
		{
			pTmpBuff = pDataBuff;

			if ( pPrivData != nil )
			{
				pDataBuff = pPrivData->fNextPtr;
				if ( pDataBuff != nil )
				{
					pPrivData = (tDataBufferPriv *)pDataBuff;
				}
			}
			else
			{
				pDataBuff = nil;
			}

			free( pTmpBuff );
			pTmpBuff = nil;
		}
	}

	return( tdsResult );

} // dsDataListDeallocatePriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsGetPathFromListPriv
//
//--------------------------------------------------------------------------------------------------

char* dsGetPathFromListPriv ( tDataListPtr inDataList, const char *inDelimiter )
{
	char			   *outStr			= nil;
	UInt32				uiSafetyCntr	= 0;
	size_t				uiStrLen		= 0;
	tDataNode		   *pCurrNode		= nil;
	tDataBufferPriv	   *pPrivData		= nil;
	char			   *prevStr			= nil;
	UInt32				cStrLen			= 256;
	char			   *nextStr			= nil;

	if ( (inDataList == nil) || (inDelimiter == nil) )
	{
		return( nil );
	}

	if ( (inDataList->fDataNodeCount == 0) || (inDataList->fDataListHead == nil) )
	{
		return( nil );
	}

        prevStr = (char *)calloc(cStrLen,sizeof(char));
	pCurrNode = inDataList->fDataListHead;
	while ( pCurrNode != nil )
	{
		// Append the delimiter
        strncat(prevStr,inDelimiter,strlen(inDelimiter));

        //check if there is more char buffer length required
		// Append the string
		pPrivData = (tDataBufferPriv *)pCurrNode;
        //check if there is more char buffer length required ie. look at the next string plus
        //the delimiter plus termination null plus pad of 4
        while (cStrLen < (1+strlen(prevStr)+pPrivData->fBufferLength+4))
        {
            nextStr = (char *)calloc(strlen(prevStr)+1,sizeof(char));
            strcpy(nextStr,prevStr);
            free(prevStr);
            cStrLen *= 2;
            prevStr = (char *)calloc(cStrLen,sizeof(char));
            strcpy(prevStr,nextStr);
            free(nextStr);
        }
        strncat(prevStr,(const char *)pPrivData->fBufferData,pPrivData->fBufferLength);

		pCurrNode = pPrivData->fNextPtr;

		uiSafetyCntr++;

		if ( uiSafetyCntr == inDataList->fDataNodeCount )
		{
			// Yes, we are done
			pCurrNode = nil;
		}
	}

    uiStrLen = strlen(prevStr);
	if ( uiStrLen != 0 )
	{
        outStr = (char *)calloc( uiStrLen + 1, sizeof(char));
        ::strcpy( outStr, prevStr );
	}

    free(prevStr);

	return( outStr );

} // dsGetPathFromListPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsBuildFromPathPriv
//
//--------------------------------------------------------------------------------------------------

tDataListPtr dsBuildFromPathPriv ( const char *inPathCString, const char *inPathSeparatorCString )
{
	const char		   *inStr		= nil;
	char			   *ptr			= nil;
	const char		   *inDelim		= nil;
	size_t				delimLen	= 0;
	bool				done		= false;
	size_t				len			= 0;
	tDataList		   *outDataList	= nil;
    char			   *cStr		= nil;

	if ( (inPathCString == nil) || (inPathSeparatorCString == nil) )
	{
		return( nil );
	}

	inStr = inPathCString;
	len = ::strlen( inStr );

   	inDelim = inPathSeparatorCString;
   	delimLen = ::strlen( inDelim );

	// Does the string == the delimiter
	if ( ::strcmp( inStr, inDelim ) == 0 )
	{
		return( nil );
	}

	outDataList = ::dsDataListAllocatePriv();
	if ( outDataList == nil )
	{
		return( nil );
	}

	ptr = strstr( inStr, inDelim );

	// Does the first char(s) == the delimiter
	if ( (ptr != nil) && (ptr == inStr) )
	{
		inStr += delimLen;
	}

	while ( !done && (*inStr != '\0') )
	{
		ptr = ::strstr( inStr, inDelim );
		if ( ptr == nil )
		{
			len = strlen( inStr );
			cStr = strdup( inStr );
			
            dsAppendStringToListAllocPriv( outDataList, cStr );
            free(cStr);
			
            done = true;
		}
		else
		{
			len = ptr - inStr;
			
			cStr = dsCStrFromCharacters( inStr, len );
			dsAppendStringToListAllocPriv( outDataList, cStr );
            free(cStr);
            
			inStr += len + delimLen;
		}
	}

	return( outDataList );

} // dsBuildFromPathPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsAppendStringToListAllocPriv
//
//--------------------------------------------------------------------------------------------------

tDirStatus dsAppendStringToListAllocPriv (	tDataList	   *inOutDataList,
											const char	   *inCString )
{
	tDirStatus			tdsResult		= eDSNoErr;
	const char		   *pInString		= inCString;
	tDataNodePtr		pNewNode		= nil;
	tDataNodePtr		pCurrNode		= nil;
	tDataBufferPriv    *pNewNodeData	= nil;
	tDataBufferPriv    *pCurNodeData	= nil;

	if ( inOutDataList == nil )
	{
		return( eDSNullDataList );
	}

	if ( inCString == nil )
	{
		return( eDSNullParameter );
	}

	if ( ((inOutDataList->fDataNodeCount != 0) && (inOutDataList->fDataListHead == nil)) ||
		 ((inOutDataList->fDataNodeCount == 0) && (inOutDataList->fDataListHead != nil)) )
	{
		return( eDSBadDataNodeFormat );
	}

	pNewNode = ::dsAllocListNodeFromStringPriv( pInString );
	if ( pNewNode == nil )
	{
		return( eMemoryAllocError );
	}

	if ( inOutDataList->fDataNodeCount == 0 )
	{
		inOutDataList->fDataListHead = pNewNode;
		pNewNodeData = (tDataBufferPriv *)pNewNode;
		pNewNodeData->fPrevPtr		= nil;
		pNewNodeData->fNextPtr		= nil;
		pNewNodeData->fScriptCode	= kASCIICodeScript;

		inOutDataList->fDataNodeCount++;
	}
	else
	{
		pCurrNode = ::dsGetLastNodePriv( inOutDataList->fDataListHead );
		if ( pCurrNode != nil )
		{
			// Get the current node's header and point it to the new
			pCurNodeData = (tDataBufferPriv *)pCurrNode;
			pCurNodeData->fNextPtr = pNewNode;

			// Get the new node's header and point it to the prevous end
			pNewNodeData = (tDataBufferPriv *)pNewNode;
			pNewNodeData->fPrevPtr	= pCurrNode;
			pNewNodeData->fNextPtr	= nil;

			// Set the script code to ASCII
			pNewNodeData->fScriptCode = kASCIICodeScript;

			inOutDataList->fDataNodeCount++;
		}
		else
		{
			tdsResult = eDSInvalidIndex;
		}
	}

	return( tdsResult );

} // dsAppendStringToListAllocPriv



//--------------------------------------------------------------------------------------------------
//	Name:	dsAllocListNodeFromStringPriv
//
//--------------------------------------------------------------------------------------------------

tDataNodePtr dsAllocListNodeFromStringPriv ( const char *inString )
{
	UInt32				nodeSize	= 0;
	size_t				strLen		= 0;
	tDataNode		   *pOutNode	= nil;
	tDataBufferPriv	   *pPrivData	= nil;

	if ( inString != nil )
	{
		strLen = ::strlen( inString );
		nodeSize = (UInt32) (sizeof( tDataBufferPriv ) + strLen + 1);
		pOutNode = (tDataNode *)::calloc( nodeSize, sizeof( char ) );
		if ( pOutNode != nil )
		{
			pOutNode->fBufferSize = nodeSize;
			pOutNode->fBufferLength = nodeSize;

			pPrivData = (tDataBufferPriv *)pOutNode;
			pPrivData->fBufferSize = (UInt32) strLen;
			pPrivData->fBufferLength = (UInt32) strLen;

			::strcpy( pPrivData->fBufferData, inString );
		}
	}

	return( pOutNode );

} // dsAllocListNodeFromStringPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsAllocListNodeFromBuffPriv
//
//--------------------------------------------------------------------------------------------------

tDataNodePtr dsAllocListNodeFromBuffPriv ( const void *inData, const UInt32 inSize )
{
	UInt32				nodeSize	= 0;
	tDataNode		   *pOutNode	= nil;
	tDataBufferPriv	   *pPrivData	= nil;

	if ( inData != nil )
	{
		nodeSize = (UInt32) (sizeof( tDataBufferPriv ) + inSize + 1);
		pOutNode = (tDataNode *)::calloc( nodeSize, sizeof( char ) );
		if ( pOutNode != nil )
		{
			pOutNode->fBufferSize = nodeSize;
			pOutNode->fBufferLength = nodeSize;

			pPrivData = (tDataBufferPriv *)pOutNode;
			pPrivData->fBufferSize = inSize;
			pPrivData->fBufferLength = inSize;

			::memcpy( pPrivData->fBufferData, inData, inSize );
		}
	}

	return( pOutNode );

} // dsAllocListNodeFromBuffPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsGetThisNodePriv
//
//--------------------------------------------------------------------------------------------------

tDataNodePtr dsGetThisNodePriv ( tDataNode *inFirsNode, const UInt32 inIndex )
{
	UInt32				i			= 1;
	tDataNode		   *pCurrNode	= nil;
	tDataBufferPriv    *pPrivData	= nil;

	pCurrNode = inFirsNode;
	while ( pCurrNode != nil )
	{
		if ( i == inIndex )
		{
			break;
		}
		else
		{
			pPrivData = (tDataBufferPriv *)pCurrNode;
			pCurrNode = pPrivData->fNextPtr;
		}
		i++;
	}

	return( pCurrNode );

} // dsGetThisNodePriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsGetLastNodePriv
//
//--------------------------------------------------------------------------------------------------

tDataNodePtr dsGetLastNodePriv ( tDataNode *inFirsNode )
{
	tDataNode		   *pCurrNode	= nil;
	tDataBufferPriv    *pPrivData	= nil;

	pCurrNode = inFirsNode;
	pPrivData = (tDataBufferPriv *)pCurrNode;

	while ( pPrivData->fNextPtr != nil )
	{
		pCurrNode = pPrivData->fNextPtr;
		pPrivData = (tDataBufferPriv *)pCurrNode;
	}

	return( pCurrNode );

} // dsGetLastNodePriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsAppendStringToListPriv
//
//--------------------------------------------------------------------------------------------------

tDirStatus dsAppendStringToListPriv ( tDataListPtr inOutDataList, const char *inCString )
{
	tDirStatus			tdsResult		= eDSNoErr;
	const char		   *pInString		= inCString;
	tDataNodePtr		pNewNode		= nil;
	tDataNodePtr		pCurrNode		= nil;
	tDataBufferPriv    *pNewNodeData	= nil;
	tDataBufferPriv    *pCurNodeData	= nil;

	if ( inOutDataList == nil )
	{
		return( eDSNullDataList );
	}

	if ( inCString == nil )
	{
		return( eDSNullParameter );
	}

	if ( ((inOutDataList->fDataNodeCount != 0) && (inOutDataList->fDataListHead == nil)) ||
		 ((inOutDataList->fDataNodeCount == 0) && (inOutDataList->fDataListHead != nil)) )
	{
		return( eDSBadDataNodeFormat );
	}

	pNewNode = ::dsAllocListNodeFromStringPriv( pInString );
	if ( pNewNode == nil )
	{
		return( eMemoryAllocError );
	}

	if ( inOutDataList->fDataNodeCount == 0 )
	{
		inOutDataList->fDataListHead = pNewNode;
		pNewNodeData = (tDataBufferPriv *)pNewNode;
		pNewNodeData->fPrevPtr		= nil;
		pNewNodeData->fNextPtr		= nil;
		pNewNodeData->fScriptCode	= kASCIICodeScript;

		inOutDataList->fDataNodeCount++;
	}
	else
	{
		pCurrNode = ::dsGetLastNodePriv( inOutDataList->fDataListHead );
		if ( pCurrNode != nil )
		{
			// Get the current node's header and point it to the new
			pCurNodeData = (tDataBufferPriv *)pCurrNode;
			pCurNodeData->fNextPtr = pNewNode;

			// Get the new node's header and point it to the prevous end
			pNewNodeData = (tDataBufferPriv *)pNewNode;
			pNewNodeData->fPrevPtr	= pCurrNode;
			pNewNodeData->fNextPtr	= nil;

			// Set the script code to ASCII
			pNewNodeData->fScriptCode = kASCIICodeScript;

			inOutDataList->fDataNodeCount++;
		}
		else
		{
			tdsResult = eDSInvalidIndex;
		}
	}

	return( tdsResult );

} // dsAppendStringToListPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsDataListGetNodeCountPriv
//
//--------------------------------------------------------------------------------------------------

UInt32 dsDataListGetNodeCountPriv ( tDataListPtr inDataList )
{
	return( inDataList->fDataNodeCount );
} // dsDataListGetNodeCountPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsGetDataLengthPriv
//
//--------------------------------------------------------------------------------------------------

UInt32 dsGetDataLengthPriv ( tDataListPtr inDataList )
{
	bool				done		= false;
	UInt32				outStrLen	= 0;
	tDataNodePtr		pCurrNode	= nil;
	tDataBufferPriv    *pPrivData	= nil;

	if ( inDataList != nil )
	{
		if ( inDataList->fDataListHead != nil )
		{
			pCurrNode = inDataList->fDataListHead;

			// Get the list total length
			while ( !done )
			{
				outStrLen += pCurrNode->fBufferLength;
				pPrivData = (tDataBufferPriv *)pCurrNode;
				if ( pPrivData->fNextPtr == nil )
				{
					done = true;
				}
				else
				{
					pCurrNode = pPrivData->fNextPtr;
				}
			}
		}
	}

	return( outStrLen );

} // dsGetDataLengthPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsDataListGetNodePriv
// PLEASE note that this really returns a tDataBufferPriv
//--------------------------------------------------------------------------------------------------

tDirStatus dsDataListGetNodePriv ( tDataListPtr		inDataList,
									UInt32			inNodeIndex,
									tDataNodePtr	*outDataListNode )
{
	UInt32				i			= 0;
	tDirStatus			tdsResult	= eDSNoErr;
	tDataNodePtr		pCurrNode	= nil;
	tDataBufferPriv    *pPrivData	= nil;

	*outDataListNode = nil;

	if ( inDataList != nil )
	{
		pCurrNode = inDataList->fDataListHead;

		// Get the list total length
		for ( i = 1; i < inNodeIndex; i++ )
		{
			pPrivData = (tDataBufferPriv *)pCurrNode;
			if ( pPrivData->fNextPtr == nil )
			{
				pCurrNode = nil;
				break;
			}
			else
			{
				pCurrNode = pPrivData->fNextPtr;
			}
		}

		if ( pCurrNode != nil )
		{
			*outDataListNode = pCurrNode;
		}
		else
		{
			tdsResult = eDSInvalDataList;
		}
	}

	return( tdsResult );

} // dsDataListGetNodePriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsDataListGetNodeStringPriv
//
//--------------------------------------------------------------------------------------------------

char* dsDataListGetNodeStringPriv (	tDataListPtr	inDataList,
									UInt32			inNodeIndex )
{
	UInt32				iSegment	= 0;
	tDataNodePtr		pCurrNode	= nil;
	tDataBufferPriv    *pPrivData	= nil;
	char			   *outSegStr	= nil;

	if ( ( inDataList != nil ) && ( inNodeIndex > 0 ) && ( inNodeIndex <= inDataList->fDataNodeCount ) )
	{
		pCurrNode = inDataList->fDataListHead;

		// Find the one we are interested in
		iSegment = 1;
		while ( (iSegment < inNodeIndex ) && ( pCurrNode != nil) )
		{
			pPrivData = (tDataBufferPriv *)pCurrNode;
			
			if ( pPrivData->fNextPtr == nil )
			{
				pCurrNode = nil;
			}
			else
			{
				pCurrNode = pPrivData->fNextPtr;
			}
			iSegment++;
		}

		if ( pCurrNode != nil )
		{
			pPrivData = (tDataBufferPriv *)pCurrNode;
			outSegStr = (char *) calloc(1, 1 + pPrivData->fBufferLength);
			memcpy(outSegStr, (const char *)pPrivData->fBufferData, pPrivData->fBufferLength);
		}
	}

	return( outSegStr );

} // dsDataListGetNodeStringPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsDeleteLastNodePriv
//
//--------------------------------------------------------------------------------------------------

tDirStatus dsDeleteLastNodePriv ( tDataListPtr inList )
{
	tDirStatus			result			= eDSNoErr;
	return( result );

} // dsDeleteLastNodePriv



//--------------------------------------------------------------------------------------------------
//	Name:	dsBuildListFromStringsPriv
//
//--------------------------------------------------------------------------------------------------

tDataList* dsBuildListFromStringsPriv ( const char *in1stCString, ... )
{
	tDirStatus			tdsResult	= eDSNoErr;
	tDataList		   *pOutList	= nil;
	const char		   *pStr		= nil;
	UInt32				nodeCount	= 0;
	tDataNodePtr		pCurrNode	= nil;
	tDataNodePtr		pPrevNode	= nil;
	tDataBufferPriv    *pPrivData	= nil;
	va_list				args;

	pOutList = ::dsDataListAllocatePriv();
	if ( pOutList == nil )
	{
		return( nil );
	}

	va_start( args, in1stCString );

	pStr = in1stCString;

	while ( (pStr != nil) && (tdsResult == eDSNoErr) )
	{
		// Node size is: struct size + string length + null term byte
		pCurrNode = ::dsAllocListNodeFromStringPriv ( pStr );
		if ( pCurrNode != nil )
		{
			// Increment the node list count
			nodeCount++;

			if ( pOutList->fDataNodeCount == 0 )
			{
				pOutList->fDataListHead = pCurrNode;
				pPrivData = (tDataBufferPriv *)pCurrNode;
				pPrivData->fPrevPtr		= nil;
				pPrivData->fNextPtr		= nil;
				pPrivData->fScriptCode	= kASCIICodeScript;

				pOutList->fDataNodeCount++;
			}
			else if ( pPrevNode != nil )
			{
				// Get the previous node's front pointer it to the next
				pPrivData = (tDataBufferPriv *)pPrevNode;
				pPrivData->fNextPtr = pCurrNode;

				// Get the current node's back pointer it to the prevous
				pPrivData = (tDataBufferPriv *)pCurrNode;
				pPrivData->fPrevPtr = pPrevNode;

				// Set the script code to ASCII
				pPrivData->fScriptCode = kASCIICodeScript;

				pOutList->fDataNodeCount++;
			}
			else
			{
				tdsResult = eMemoryAllocError;
			}

		}

		// Get the next string
		pStr = va_arg( args, char * );

		pPrevNode = pCurrNode;
		pCurrNode = nil;
	}

	va_end( args );

	pOutList->fDataNodeCount = nodeCount;

	return( pOutList );

} // dsBuildListFromStringsPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsDataListGetNodeAllocPriv
//
//--------------------------------------------------------------------------------------------------

tDirStatus dsDataListGetNodeAllocPriv ( const tDataList		   *inDataList,
										const UInt32			inIndex,
										tDataNode			  **outDataNode )
{
	tDirStatus			tResult			= eDSNoErr;
	UInt32				uiLength		= 0;
	tDataBuffer		   *pOutDataNode	= nil;
	tDataNode		   *pCurrNode		= nil;
	tDataBufferPriv	   *pPrivData		= nil;


	LogAssert_( inDataList != nil );
	if ( inDataList == nil )
	{
		return( eDSNullDataList );
	}

	LogAssert_( inDataList->fDataListHead != nil );
	if ( inDataList->fDataListHead == nil )
	{
		return( eDSEmptyDataList );
	}

	pCurrNode = ::dsGetThisNodePriv( inDataList->fDataListHead, inIndex );
	if ( pCurrNode == nil )
	{
		return( eDSIndexOutOfRange );
	}

	if ( outDataNode == nil )
	{
		return( eDSNullTargetArgument );
	}

	pPrivData = (tDataBufferPriv *)pCurrNode;
	uiLength = pPrivData->fBufferLength;

	pOutDataNode = ::dsDataBufferAllocatePriv( uiLength + 1 );
	if ( pOutDataNode != nil )
	{
		::memcpy( pOutDataNode->fBufferData, pPrivData->fBufferData, uiLength );
		pOutDataNode->fBufferSize = uiLength;
		pOutDataNode->fBufferLength = uiLength;
		*outDataNode = pOutDataNode;
	}
	else
	{
		tResult = eMemoryAllocError;
	}

	return( tResult );

} // dsDataListGetNodeAllocPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsAuthBufferGetDataListAllocPriv
//
//--------------------------------------------------------------------------------------------------

tDataListPtr dsAuthBufferGetDataListAllocPriv ( tDataBufferPtr inAuthBuff )
{
	tDataListPtr pOutList = NULL;
	tDirStatus status = eDSNoErr;
	if (inAuthBuff == NULL)
	{
		LOG2( kStdErr, "*** DS NULL Error: File: %s. Line: %d.\n", __FILE__, __LINE__ );
		return NULL;
	}

	pOutList = dsDataListAllocatePriv();
	if (pOutList != NULL)
	{
		status = dsAuthBufferGetDataListPriv( inAuthBuff, pOutList );
		if (status != eDSNoErr)
		{
			dsDataListDeallocatePriv( pOutList );
			free( pOutList );
			pOutList = NULL;
		}
	}

	return pOutList;
} // dsAuthBufferGetDataListAllocPriv


//--------------------------------------------------------------------------------------------------
//	Name:	dsAuthBufferGetDataListPriv
//
//--------------------------------------------------------------------------------------------------

tDirStatus dsAuthBufferGetDataListPriv ( tDataBufferPtr inAuthBuff, tDataListPtr inOutDataList )
{
	char		   *pData			= nil;
	UInt32			itemLen			= 0;
	UInt32			offset			= 0;
	UInt32			buffSize		= 0;
	UInt32			buffLen			= 0;
	tDirStatus			tResult		= eDSNoErr;
	tDataNode		   *pCurrNode	= nil;
	tDataNode		   *pPrevNode	= nil;
	tDataBufferPriv    *pPrivData	= nil;

	if (inAuthBuff == NULL)
	{
		LOG2( kStdErr, "*** DS NULL Error: File: %s. Line: %d.\n", __FILE__, __LINE__ );
		return eDSNullDataBuff;
	}
	if ( inOutDataList == nil )
	{
		LOG2( kStdErr, "*** DS NULL Error: File: %s. Line: %d.\n", __FILE__, __LINE__ );
		return( eDSNullDataList );
	}

	// This could leak, but the client should not pass us a nonempty data list
	inOutDataList->fDataNodeCount = 0;
	inOutDataList->fDataListHead  = nil;

	pData		= inAuthBuff->fBufferData;
	buffSize	= inAuthBuff->fBufferSize;
	buffLen		= inAuthBuff->fBufferLength;

	if (buffLen > buffSize)
		return eDSInvalidBuffFormat;

	while ( (offset < buffLen) && (tResult == eDSNoErr) )
	{
		if (offset + sizeof( UInt32 ) > buffLen)
		{
			tResult = eDSInvalidBuffFormat;
			break;
		}
		::memcpy( &itemLen, pData, sizeof( UInt32 ) );
		pData += sizeof( UInt32 );
		offset += (UInt32) ( sizeof(UInt32) );
		if (itemLen + offset > buffLen)
		{
			tResult = eDSInvalidBuffFormat;
			break;
		}

		// Node size is: struct size + string length + null term byte
		pCurrNode = dsAllocListNodeFromBuffPriv( pData, itemLen );
		if ( pCurrNode != nil )
		{
			pData += itemLen;
			offset += itemLen;

			if ( inOutDataList->fDataNodeCount == 0 )
			{
				inOutDataList->fDataListHead = pCurrNode;
				pPrivData = (tDataBufferPriv *)pCurrNode;
				pPrivData->fPrevPtr		= nil;
				pPrivData->fNextPtr		= nil;
				pPrivData->fScriptCode	= kASCIICodeScript;

				inOutDataList->fDataNodeCount++;
			}
			else if ( pPrevNode != nil )
			{
				// Get the previous node's header and point it to the next
				pPrivData = (tDataBufferPriv *)pPrevNode;
				pPrivData->fNextPtr = pCurrNode;

				// Get the current node's header and point it to the prevous
				pPrivData = (tDataBufferPriv *)pCurrNode;
				pPrivData->fPrevPtr = pPrevNode;

				// Set the script code to ASCII
				pPrivData->fScriptCode = kASCIICodeScript;

				inOutDataList->fDataNodeCount++;
			}
			else
			{
				tResult = eMemoryAllocError;
				LOG3( kStdErr, "*** DSError: File: %s. Line: %d. Error = %d.\n", __FILE__, __LINE__, tResult );
			}
		}
		else
		{
			tResult = eMemoryAllocError;
			LOG3( kStdErr, "*** DSError: File: %s. Line: %d. Error = %d.\n", __FILE__, __LINE__, tResult );
		}

		pPrevNode = pCurrNode;
		pCurrNode = nil;
	}

	if ( tResult != eDSNoErr )
	{
		// clean up if there was an errors
		dsDataListDeallocatePriv(inOutDataList);
	}
	return( tResult );

} // dsAuthBufferGetDataListPriv


// --------------------------------------------------------------------------------
//	dsCStrFromCharacters
// --------------------------------------------------------------------------------

char* dsCStrFromCharacters( const char *inChars, size_t inLen )
{
	register char *retVal = NULL;
	
	if ( inChars != NULL )
	{
		retVal = (char *) malloc( inLen + 1 );
		if ( retVal != NULL )
			strlcpy( retVal, inChars, inLen + 1 );
	}
	
	return retVal;
}


// --------------------------------------------------------------------------------
//	BinaryToHexConversion
// --------------------------------------------------------------------------------
void BinaryToHexConversion( const unsigned char *inBinary, UInt32 inLength, char *outHexStr )
{
    const unsigned char		   *sptr	= inBinary;
    char					   *tptr	= outHexStr;
    UInt32						index	= 0;
    char 						high;
	char						low;
    
    for ( index = 0; index < inLength; index++ )
    {
        high	= (*sptr & 0xF0) >> 4;
        low		= (*sptr & 0x0F);
        
        if ( high >= 0x0A )
            *tptr++ = (high - 0x0A) + 'A';
        else
            *tptr++ = high + '0';
            
        if ( low >= 0x0A )
            *tptr++ = (low - 0x0A) + 'A';
        else
            *tptr++ = low + '0';
            
        sptr++;
    }
    
    *tptr = '\0';
}


// --------------------------------------------------------------------------------
//	HexToBinaryConversion
// --------------------------------------------------------------------------------

void HexToBinaryConversion( const char *inHexStr, UInt32 *outLength, unsigned char *outBinary )
{
    unsigned char	   *tptr = outBinary;
    unsigned char		val;
    
    while ( *inHexStr && *(inHexStr+1) )
    {
        if ( *inHexStr >= 'A' )
            val = (*inHexStr - 'A' + 0x0A) << 4;
        else
            val = (*inHexStr - '0') << 4;
        
        inHexStr++;
        
        if ( *inHexStr >= 'A' )
            val += (*inHexStr - 'A' + 0x0A);
        else
            val += (*inHexStr - '0');
        
        inHexStr++;
        
        *tptr++ = val;
    }
    
    *outLength = (UInt32) (tptr - outBinary);
}

//--------------------------------------------------------------------------------------------------
//	Name:	dsTimestamp
//
//--------------------------------------------------------------------------------------------------
// Actually, [gettimeofday] is not so great, since it uses the calendar clock and has to call into the kernel. 
// mach_absolute_time() doesn't call into the kernel (on platforms where it doesn't have to...) 
// This is done for you by mach_absolute_time() and mach_timebase_info().

double dsTimestamp(void)
{
	static UInt32	num		= 0;
	static UInt32	denom	= 0;
	uint64_t		now;
	
	if (denom == 0) 
	{
		struct mach_timebase_info tbi;
		kern_return_t r;
		r = mach_timebase_info(&tbi);
		if (r != KERN_SUCCESS) 
		{
			syslog( LOG_ALERT, "Warning: mach_timebase_info FAILED! - error = %u\n", r);
			return 0;
		}
		else
		{
			num		= tbi.numer;
			denom	= tbi.denom;
		}
	}
	now = mach_absolute_time();
	
//	return (double)(now * (double)num / denom / NSEC_PER_SEC);	// return seconds
	return (double)(now * (double)num / denom / NSEC_PER_USEC);	// return microsecs
//	return (double)(now * (double)num / denom);	// return nanoseconds
}


tDirStatus dsGetAuthMethodEnumValue( tDataNode *inData, UInt32 *outAuthMethod )
{
	tDirStatus		siResult			= eDSNoErr;
	size_t			uiNativeLen			= 0;
	char		   *authMethodPtr		= NULL;
	int				index				= 0;
	bool			found				= false;
	
	if ( inData == NULL )
	{
		*outAuthMethod = kAuthUnknownMethod;
		return eDSAuthMethodNotSupported;
	}
	
	authMethodPtr = (char *)inData->fBufferData;
	
	//DbgLog( kLogPlugin, "Using authentication method %s.", authMethodPtr );
	
	for ( index = 0; gAuthMethodTable[index].name != NULL; index++ )
	{
		if ( strcmp(authMethodPtr, gAuthMethodTable[index].name) == 0 )
		{
			*outAuthMethod = gAuthMethodTable[index].value;
			found = true;
			break;
		}
	}
	
	if ( !found )
	{
		uiNativeLen	= strlen( kDSNativeAuthMethodPrefix );

		if ( strncmp( authMethodPtr, kDSNativeAuthMethodPrefix, uiNativeLen ) == 0 )
		{
			*outAuthMethod = kAuthNativeMethod;
		}
		else
		{
			*outAuthMethod = kAuthUnknownMethod;
			siResult = eDSAuthMethodNotSupported;
		}
	}
	
	return( siResult );
} // dsGetAuthMethodEnumValue


const char* dsGetPatternMatchName ( tDirPatternMatch inPatternMatchEnum )
{
	const char	   *outString   = nil;
	
	switch (inPatternMatchEnum)
	{
		case eDSNoMatch1:
			outString = "eDSNoMatch1";
			break;
			
		case eDSAnyMatch:
			outString = "eDSAnyMatch";
			break;
			
		case eDSExact:
			outString = "eDSExact";
			break;
			
		case eDSStartsWith:
			outString = "eDSStartsWith";
			break;
			
		case eDSEndsWith:
			outString = "eDSEndsWith";
			break;
			
		case eDSContains:
			outString = "eDSContains";
			break;
			
		case eDSLessThan:
			outString = "eDSLessThan";
			break;
			
		case eDSGreaterThan:
			outString = "eDSGreaterThan";
			break;
			
		case eDSLessEqual:
			outString = "eDSLessEqual";
			break;
			
		case eDSGreaterEqual:
			outString = "eDSGreaterEqual";
			break;
			
		case eDSWildCardPattern:
			outString = "eDSWildCardPattern";
			break;
			
		case eDSRegularExpression:
			outString = "eDSRegularExpression";
			break;
			
		case eDSCompoundExpression:
			outString = "eDSCompoundExpression";
			break;
			
		case eDSiExact:
			outString = "eDSiExact";
			break;
			
		case eDSiStartsWith:
			outString = "eDSiStartsWith";
			break;
			
		case eDSiEndsWith:
			outString = "eDSiEndsWith";
			break;
			
		case eDSiContains:
			outString = "eDSiContains";
			break;
			
		case eDSiLessThan:
			outString = "eDSiLessThan";
			break;
			
		case eDSiGreaterThan:
			outString = "eDSiGreaterThan";
			break;
			
		case eDSiLessEqual:
			outString = "eDSiLessEqual";
			break;
			
		case eDSiGreaterEqual:
			outString = "eDSiGreaterEqual";
			break;
			
		case eDSiWildCardPattern:
			outString = "eDSiWildCardPattern";
			break;
			
		case eDSiRegularExpression:
			outString = "eDSiRegularExpression";
			break;
			
		case eDSiCompoundExpression:
			outString = "eDSiCompoundExpression";
			break;
			
		case eDSLocalNodeNames:
			outString = "eDSLocalNodeNames";
			break;
			
		case eDSAuthenticationSearchNodeName:
			outString = "eDSAuthenticationSearchNodeName";
			break;
			
		case eDSConfigNodeName:
			outString = "eDSConfigNodeName";
			break;
			
		case eDSLocalHostedNodes:
			outString = "eDSLocalHostedNodes";
			break;
			
		case eDSContactsSearchNodeName:
			outString = "eDSContactsSearchNodeName";
			break;
			
		case eDSNetworkSearchNodeName:
			outString = "eDSNetworkSearchNodeName";
			break;
			
		case eDSDefaultNetworkNodes:
			outString = "eDSDefaultNetworkNodes";
			break;

		case eDSCacheNodeName:
			outString = "eDSCacheNodeName";
			break;

		default:
			outString = "Pattern Match Unknown";
	}
	
	return(outString);
}

char* dsGetNameForProcessID ( pid_t inPID )
{
	int mib []		= { CTL_KERN, KERN_PROC, KERN_PROC_PID, (int)inPID };
	size_t ulSize	= 0;
	char *procName	= nil;

	// Look for a given pid
	if (inPID > 1) {
		struct kinfo_proc kpsInfo;
		ulSize = sizeof (kpsInfo);
		if (!::sysctl (mib, 4, &kpsInfo, &ulSize, NULL, 0)
			&& (kpsInfo.kp_proc.p_pid == inPID))
		{
			if (kpsInfo.kp_proc.p_comm != NULL)
			{
				procName = (char *)calloc(1, 1+strlen(kpsInfo.kp_proc.p_comm));
				strcpy(procName,kpsInfo.kp_proc.p_comm);
			}
		}
	}

	return( procName );
}


//--------------------------------------------------------------------------------------------------
//	Name:	dsConvertAuthBufferToCFArray
//
//--------------------------------------------------------------------------------------------------

CFArrayRef dsConvertAuthBufferToCFArray( tDataBufferPtr inAuthBuff )
{
	CFArrayRef itemArray = NULL;
	tDataListPtr bufferItemListPtr = dsAuthBufferGetDataListAllocPriv( inAuthBuff );
	if ( bufferItemListPtr != NULL )
	{
		itemArray = dsConvertDataListToCFArray( bufferItemListPtr );
		dsDataListDeallocatePriv( bufferItemListPtr );
		free( bufferItemListPtr );
	}
	
	return itemArray;
}


//--------------------------------------------------------------------------------------------------
//	Name:	dsConvertDataListToCFArray
//
//--------------------------------------------------------------------------------------------------

CFArrayRef dsConvertDataListToCFArray( tDataList *inDataList )
{
	CFMutableArrayRef   cfArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	tDataBufferPriv     *dsDataNode  = (tDataBufferPriv *)(NULL != inDataList ? inDataList->fDataListHead : NULL);
	
	while( NULL != dsDataNode )
	{
		if( NULL != dsDataNode->fBufferData )
		{
			CFDataRef cfRef = CFDataCreate( kCFAllocatorDefault, (const UInt8 *) dsDataNode->fBufferData, dsDataNode->fBufferLength );
			
			if( NULL != cfRef )
			{
				CFArrayAppendValue( cfArray, cfRef );
				
				CFRelease( cfRef );
				cfRef = NULL;
			}
		}
		
		dsDataNode = (tDataBufferPriv *)dsDataNode->fNextPtr;
	}
	
	return cfArray;
}

tDataListPtr dsConvertCFArrayToDataList( CFArrayRef inArray )
{
	tDataListPtr    dsDataList  = dsDataListAllocatePriv();
	
	if( NULL != inArray )
	{
		tDataBufferPriv *pCurNodeData   = NULL;
		CFIndex         iCount          = CFArrayGetCount( inArray );
		CFIndex         ii;
		
		// extract values out of the CFArray into a tDataList
		for( ii = 0; ii < iCount; ii++ )
		{
			CFTypeRef       cfRef           = CFArrayGetValueAtIndex( inArray, ii );
			char            *pTempBuffer    = NULL;
			Boolean         bDeallocBuffer  = FALSE;
			size_t			uiLength        = 0;
			
			if( CFStringGetTypeID() == CFGetTypeID(cfRef) )
			{
				CFIndex iBufferSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength((CFStringRef)cfRef), kCFStringEncodingUTF8) + 1;
				pTempBuffer         = (char *)malloc( iBufferSize );
				
				if( NULL != pTempBuffer )
				{
					CFStringGetCString( (CFStringRef) cfRef, pTempBuffer, iBufferSize, kCFStringEncodingUTF8 );
					uiLength = strlen( pTempBuffer );
					bDeallocBuffer = TRUE;
				}
			}
			else if( CFDataGetTypeID() == CFGetTypeID(cfRef) )
			{
				uiLength = (uint32_t) CFDataGetLength( (CFDataRef) cfRef );
				pTempBuffer = (char *) CFDataGetBytePtr( (CFDataRef) cfRef );
			}
			
			if( NULL != pTempBuffer )
			{
				tDataBufferPriv *pNewNodeData = (tDataBufferPriv *)::calloc( sizeof(tDataBufferPriv) + uiLength, sizeof(char) );
				if ( pNewNodeData != nil )
				{
					pNewNodeData->fBufferSize = (UInt32) uiLength;
					pNewNodeData->fBufferLength = (UInt32) uiLength;
					
					bcopy( pTempBuffer, pNewNodeData->fBufferData, uiLength );
					
					// Get the new node's header and point it to the prevous end
					pNewNodeData->fPrevPtr	= (tDataNodePtr) pCurNodeData;
					
					// Set the script code to ASCII
					pNewNodeData->fScriptCode = kASCIICodeScript;
				}
				
				// if we have a current node, means we are appending to the list
				if( pCurNodeData != nil )
					pCurNodeData->fNextPtr = (tDataNodePtr) pNewNodeData;
				else
					dsDataList->fDataListHead = (tDataNodePtr) pNewNodeData;
				
				if( TRUE == bDeallocBuffer )
				{
					free( pTempBuffer );
					pTempBuffer = NULL;
				}
				
				dsDataList->fDataNodeCount++;
				
				pCurNodeData = pNewNodeData;
			}
		}
	}
	
	return dsDataList;
}

// ----------------------------------------------------------------------------------------
//  dsCreateEventLogDict
// ----------------------------------------------------------------------------------------

CFMutableDictionaryRef dsCreateEventLogDict( CFStringRef inEventType, const char *inUser, CFDictionaryRef inDetails )
{
	CFMutableDictionaryRef eventDict = NULL;
	
	// add the service supplied items first so our event keys override collisions
	if ( inDetails != NULL )
		eventDict = CFDictionaryCreateMutableCopy( NULL, 0, inDetails );
	else
		eventDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

	if ( eventDict == NULL )
		return NULL;
	
	// add required items
	CFDictionaryAddValue( eventDict, CFSTR("event_type"), inEventType );
	CFStringRef userString = CFStringCreateWithCString( NULL, inUser, kCFStringEncodingUTF8 );
	if ( userString != NULL ) {
		CFDictionaryAddValue( eventDict, CFSTR("user"), userString );
		CFRelease( userString );
	}
	
	return eventDict;
}


// ----------------------------------------------------------------------------------------
//  mkdir_p
//
//	Returns: 0=ok, -1=fail
// ----------------------------------------------------------------------------------------

static int mkdir_p( const char *path, mode_t mode )
{
	int err = 0;
	char buffer[PATH_MAX];
	char *segPtr;
	char *inPtr;
		
	// make the directory
	int len = snprintf( buffer, sizeof(buffer), "%s", path );
	if ( len >= (int)sizeof(buffer) - 1 )
		return -1;
	
	inPtr = buffer;
	if ( *inPtr == '/' )
		inPtr++;
	while ( inPtr != NULL )
	{
		segPtr = strsep( &inPtr, "/" );
		if ( segPtr != NULL )
		{
			err = mkdir( buffer, mode );
			if ( err != 0 && errno != EEXIST )
				break;
			if ( err == 0 )
				chmod( buffer, mode );
			err = 0;
			
			if ( inPtr != NULL )
				*(inPtr - 1) = '/';
		}
	}
	
	return err;
}


// ---------------------------------------------------------------------------
//	dsCreatePrefsDirectory
// ---------------------------------------------------------------------------

int dsCreatePrefsDirectory( void )
{
	int err = 0;
	mode_t saved_mask;
    struct stat statResult;
		
	//step 1- see if the file exists
	//if not then make sure the directories exist or create them
	//then create a new file if necessary
	err = stat( kDSLDAPPrefsDirPath, &statResult );
	
	//if path does not exist
	if (err != 0)
	{
		saved_mask = umask( 0777 );
		err = mkdir_p( kDSLDAPPrefsDirPath, 0755 );
		umask( saved_mask );
	}
	
	return err;
}


// ---------------------------------------------------------------------------
//	* dsCreatePrefsFilename
// ---------------------------------------------------------------------------

CFStringRef dsCreatePrefsFilename( const char *inFileNameBase )
{
	CFStringRef		cfENetAddr			= NULL;
	CFStringRef		fileString			= nil;
	struct stat		statResult;
	char			filenameStr[PATH_MAX];
	
	// this routine is used during reading and writing to ensure we use a specific config for this
	// computer if it exists

	if ( inFileNameBase == NULL )
		return NULL;
		
	GetMACAddress( &cfENetAddr, NULL, false );
	if ( cfENetAddr )
	{
		fileString = CFStringCreateWithFormat( NULL, NULL, CFSTR("%s.%@.plist"), inFileNameBase, cfENetAddr );
		
		DSCFRelease( cfENetAddr );
		
		if ( CFStringGetCString(fileString, filenameStr, sizeof(filenameStr), kCFStringEncodingUTF8) )
		{
			if ( stat(filenameStr, &statResult) != 0 )
			{
				DbgLog( kLogInfo, "dsCreatePrefsFilename couldn't find host-specific file '%s'", filenameStr );
				DSCFRelease( fileString );
			}
			else
			{
				DbgLog( kLogDebug, "dsCreatePrefsFilename will use host-specific file '%s'", filenameStr );
			}
		}
	}
	
	if ( fileString == NULL )
		fileString = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("%s.plist"), inFileNameBase );
	
	return fileString;
}


void dsPostNodeEvent( void )
{
	SCDynamicStoreRef store = SCDynamicStoreCreate( kCFAllocatorDefault, NULL, NULL, NULL );
	if ( store != NULL ) {
		SCDynamicStoreNotifyValue( store, CFSTR(kDSNodeEvent) );
		DSCFRelease( store );
	}
}

void *dsRetainObject( void *object, volatile int32_t *refcount )
{
	if ( object == NULL ) return NULL;
	if ( (*refcount) == INT32_MAX ) return object;

	int32_t newCount = __sync_add_and_fetch( refcount, 1 );
	if ( newCount > 1 ) {
		return object;
	}
	
	return NULL;
}

bool dsReleaseObject( void *object, volatile int32_t *refcount, bool bFree )
{
	if ( object == NULL || (*refcount) == INT32_MAX ) return false;
	
	int32_t newCount = __sync_sub_and_fetch( refcount, 1 );
	if ( newCount > 0 ) {
		return false;
	}
	else if ( newCount == 0 ) {
		if ( bFree == true ) free( object );
		return true;
	}
	
	DbgLog( kLogAlert, "Aborting - Releasing object at %X -- new refCount = %d", object, newCount );
	assert( newCount >= 0 );
	
	return true;
}
