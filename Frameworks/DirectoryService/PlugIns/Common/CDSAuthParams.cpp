/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#include <string.h>
#include "buffer_unpackers.h"
#include "CDSAuthDefs.h"
#include "CDSLocalAuthHelper.h"
#include "CDSAuthParams.h"
#include "PrivateTypes.h"
#include "DSUtils.h"
#include "AuthHelperUtils.h"
#include "DirServiceMain.h"

CDSAuthParams::CDSAuthParams()
{
	uiAuthMethod						= kAuthUnknownMethod;
	mAuthMethodStr						= NULL;
	pUserName							= NULL;
	pNewPassword						= NULL;
	pOldPassword						= NULL;
	pNTLMDigest							= NULL;
	ntlmDigestLen						= 0;
	pCramResponse						= NULL;
	cramResponseLen						= 0;
	pSambaName							= NULL;
	pDomain								= NULL;
	pAdminUser							= NULL;
	pAdminPassword						= NULL;
	PeerC16								= NULL;
	dataList							= NULL;
	path								= NULL;
	hashLength							= kHashTotalLength;
	hashesLengthFromFile				= 0;
	secureHashNode						= NULL;
	itemCount							= 0;
	nativeAttrType						= NULL;
	policyStr							= NULL;
	policyStrLen						= 0;
	keySize								= 0;
	challenge							= NULL;
	apopResponse						= NULL;
	aaData								= NULL;
	aaDataLocalCacheUser				= NULL;
	serviceInfoDict						= NULL;
	mPostAuthEvent						= false;
	
	bzero(&modDateOfPassword, sizeof(modDateOfPassword));
	bzero(&modDateAssist, sizeof(modDateAssist));
	bzero(&globalAccess, sizeof(globalAccess));
	bzero(&globalMoreAccess, sizeof(globalMoreAccess));
	bzero(&digestContext, sizeof(digestContext));
	
	ZeroHashes();
}


CDSAuthParams::~CDSAuthParams()
{
	DSFreeString( mAuthMethodStr );
	DSFreeString( path );
	DSFreeString( policyStr );
	
	if ( nativeAttrType != NULL )
	{
		delete nativeAttrType;
		nativeAttrType = NULL;
	}
	if (dataList != NULL)
	{
		dsDataListDeallocatePriv(dataList);
		free(dataList);
		dataList = NULL;
	}
	
	DSFreeString( pUserName );

	if ( pNewPassword != NULL )
	{
		bzero(pNewPassword, strlen(pNewPassword));
		free( pNewPassword );
		pNewPassword = NULL;
	}
	
	if ( pOldPassword != NULL )
	{
		bzero(pOldPassword, strlen(pOldPassword));
		free( pOldPassword );
		pOldPassword = NULL;
	}
	
	DSFree( PeerC16 );
	DSFree( pNTLMDigest );
	DSFreeString( pSambaName );
	DSFreeString( pDomain );
	DSFreeString( pAdminUser );
	
	if ( pAdminPassword != NULL )
	{
		bzero(pAdminPassword, strlen(pAdminPassword));
		free( pAdminPassword );
		pAdminPassword = NULL;
	}
	
	digest_dispose( &digestContext );
	
	DSFreeString( challenge );
	DSFreeString( apopResponse );
	DSFree( pCramResponse );
	DSFreeString( aaData );
	DSFreeString( aaDataLocalCacheUser );
	DSCFRelease( serviceInfoDict );
	
	ZeroHashes();
}


void
CDSAuthParams::ZeroHashes( void )
{
	//zero out all the hashes used above
	bzero(P21, kHashShadowKeyLength);
	bzero(C8, kHashShadowChallengeLength);
	bzero(P24, kHashShadowResponseLength);
	bzero(P24Input, kHashShadowResponseLength);
	bzero(hashes, kHashTotalLength);
	bzero(generatedHashes, kHashTotalLength);
	bzero(secureHash, kHashSecureLength);
	
	bzero(C16, sizeof(C16));
	bzero(GeneratedNTLM, sizeof(GeneratedNTLM));
	bzero(MSCHAP2Response, sizeof(MSCHAP2Response));
}


tDirStatus
CDSAuthParams::LoadParamsForAuthMethod(
	tDataNodePtr inAuthMethod,
	tDataBufferPtr inAuthData,
	tDataBufferPtr inAuthStepData )
{
	tDirStatus siResult = dsGetAuthMethodEnumValue( inAuthMethod, &uiAuthMethod );
	if ( siResult != eDSNoErr && siResult != eDSAuthMethodNotSupported )
		return siResult;
	
	mAuthMethodStr = strdup( inAuthMethod->fBufferData );
	
	siResult = ExtractServiceInfo( inAuthStepData );
	if ( siResult != eDSNoErr )
		return siResult;
	
	switch( uiAuthMethod )
	{
		case kAuthPPS:
			mPostAuthEvent = true;
			siResult = (tDirStatus)Get2FromBuffer( inAuthData, NULL, &pUserName, &challenge, NULL );
			break;
		
		case kAuthDIGEST_MD5:
			mPostAuthEvent = true;
			siResult = (tDirStatus)UnpackDigestBuffer( inAuthData, &pUserName, &digestContext );
			break;
			
		case kAuthCRAM_MD5:
			mPostAuthEvent = true;
			siResult = (tDirStatus)UnpackCramBuffer( inAuthData, &pUserName, &challenge, &pCramResponse,
				&cramResponseLen );
			break;
			
		case kAuthAPOP:
			mPostAuthEvent = true;
			siResult = (tDirStatus)UnpackAPOPBuffer( inAuthData, &pUserName, &challenge, &apopResponse );
			break;
			
		case kAuthSMB_NT_Key:
			mPostAuthEvent = true;
			siResult = (tDirStatus)UnpackSambaBuffer( inAuthData, &pUserName, C8, P24Input );
			break;
			
		case kAuthSMB_LM_Key:
			mPostAuthEvent = true;
			siResult = (tDirStatus)UnpackSambaBuffer( inAuthData, &pUserName, C8, P24Input );
			break;
			
		case kAuthNTLMv2:
			mPostAuthEvent = true;
			siResult = (tDirStatus)UnpackNTLMv2Buffer( inAuthData, &pUserName, C8, &pNTLMDigest, &ntlmDigestLen,
				&pSambaName, &pDomain );
			break;
		
		case kAuthMSCHAP2:
			mPostAuthEvent = true;
			if ( inAuthStepData == NULL )
				return( eDSNullAuthStepData );
			if ( inAuthStepData->fBufferSize < 4 + MS_AUTH_RESPONSE_LENGTH )
				return( eDSBufferTooSmall );
			
			siResult = (tDirStatus)UnpackMSCHAPv2Buffer( inAuthData, &pUserName, C16, &PeerC16, &pNTLMDigest,
				&ntlmDigestLen, &pSambaName );
			break;
		
		case kAuthVPN_PPTPMasterKeys:
			if ( inAuthStepData == NULL )
				return( eDSNullAuthStepData );
			siResult = (tDirStatus)UnpackMPPEKeyBuffer( inAuthData, &pUserName, P24Input, &keySize );
			if ( inAuthStepData->fBufferSize < (UInt32)(8 + keySize*2) )
				return( eDSBufferTooSmall );
			break;
		
		case kAuthSMBWorkstationCredentialSessionKey:
			if ( inAuthStepData == NULL )
				return( eDSNullAuthStepData );
			ntlmHashType = 0;
			siResult = GetNameAndDataFromBuffer( inAuthData, &dataList, &pUserName, &pNTLMDigest, &ntlmDigestLen, &itemCount );
			if ( inAuthStepData->fBufferSize < (UInt32)(sizeof(UInt32) + 8) )
				return( eDSBufferTooSmall );
			if ( siResult == eDSNoErr )
			{
				tDirStatus tmpstatus;
				UInt32 tmplen;
				unsigned char *type = NULL;
				tmpstatus = GetDataFromAuthBuffer( inAuthData, 3, &type, &tmplen);
				if ( tmpstatus == eDSNoErr && tmplen == sizeof(UInt32) )
					ntlmHashType = *(UInt32*)type;
				if( type )
					free(type);
			}
			break;
			
		case kAuthSecureHash:
			mPostAuthEvent = true;
			// parse input first
			dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
			if ( dataList == NULL ) return( eDSInvalidBuffFormat );
			if ( dsDataListGetNodeCountPriv(dataList) != 2 ) return( eDSInvalidBuffFormat );
			
			// this allocates a copy of the string
			pUserName = dsDataListGetNodeStringPriv(dataList, 1);
			if ( pUserName == NULL ) return( eDSInvalidBuffFormat );
			if ( strlen(pUserName) < 1 ) return( eDSInvalidBuffFormat );
			// these are not copies
			siResult = dsDataListGetNodePriv(dataList, 2, &secureHashNode);
			if ( secureHashNode == NULL ) return( eDSInvalidBuffFormat );
			if ( secureHashNode->fBufferLength != kHashSaltedSHA1Length ) return( eDSInvalidBuffFormat);
			if ( siResult != eDSNoErr ) return( eDSInvalidBuffFormat );
			memmove(secureHash, ((tDataBufferPriv*)secureHashNode)->fBufferData, secureHashNode->fBufferLength);
			break;

		case kAuthWriteSecureHash:
			// parse input first
			dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
			if ( dataList == NULL ) return( eDSInvalidBuffFormat );
			if ( dsDataListGetNodeCountPriv(dataList) != 2 ) return( eDSInvalidBuffFormat );
				
			// this allocates a copy of the string
			pUserName = dsDataListGetNodeStringPriv(dataList, 1);
			if ( pUserName == NULL ) return( eDSInvalidBuffFormat );
			if ( strlen(pUserName) < 1 ) return( eDSInvalidBuffFormat );
			// these are not copies
			siResult = dsDataListGetNodePriv(dataList, 2, &secureHashNode);
			if ( secureHashNode == NULL ) return( eDSInvalidBuffFormat );
			if ( secureHashNode->fBufferLength != kHashSaltedSHA1Length ) return( eDSInvalidBuffFormat);
			if ( siResult != eDSNoErr ) return( eDSInvalidBuffFormat );
			memmove(secureHash, ((tDataBufferPriv*)secureHashNode)->fBufferData, secureHashNode->fBufferLength);
			break;
			
		case kAuthReadSecureHash:
			if ( inAuthStepData == NULL ) return( eDSNullAuthStepData );
			// parse input first
			dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
			if ( dataList == NULL ) return( eDSInvalidBuffFormat );
			if ( dsDataListGetNodeCountPriv(dataList) != 1 ) return( eDSInvalidBuffFormat );
				
			// this allocates a copy of the string
			pUserName = dsDataListGetNodeStringPriv(dataList, 1);
			if ( pUserName == NULL ) return( eDSInvalidBuffFormat );
			if ( strlen(pUserName) < 1 ) return( eDSInvalidBuffFormat );
			break;

		// set password operations
		case kAuthSetPasswd:
		case kAuthSetPasswdAsRoot:
			// parse input first
			siResult = (tDirStatus)Get2FromBuffer( inAuthData, &dataList, &pUserName, &pNewPassword, &itemCount );
			if ( (pNewPassword != nil) && (strlen(pNewPassword) >= kHashRecoverableLength) )
				return ( eDSAuthPasswordTooLong );
			if ( siResult == eDSNoErr )
			{
				if ( uiAuthMethod == kAuthSetPasswd )
				{
					if ( itemCount != 4 )
						return( eDSInvalidBuffFormat );
					
					pAdminUser = dsDataListGetNodeStringPriv( dataList, 3 );
					if ( pAdminUser == NULL ) return( eDSInvalidBuffFormat );
					
					pAdminPassword = dsDataListGetNodeStringPriv( dataList, 4 );
					if ( pAdminPassword == NULL ) return( eDSInvalidBuffFormat );
				}
				else if ( uiAuthMethod == kAuthSetPasswdAsRoot && itemCount != 2 )
					return( eDSInvalidBuffFormat );
			}
			break;
		
		case kAuthSetPolicyAsRoot:
			// parse input first, using <pNewPassword> to hold the policy string
			siResult = (tDirStatus)Get2FromBuffer(inAuthData, &dataList, &pUserName, &pNewPassword, &itemCount );
			if ( siResult != eDSNoErr )
				return( siResult );
			if ( itemCount != 2 || pNewPassword == NULL || pNewPassword[0] == '\0' )
				return( eDSInvalidBuffFormat );
			break;
			
		case kAuthChangePasswd:
			// parse input first
			siResult = (tDirStatus)Get2FromBuffer(inAuthData, &dataList, &pUserName, &pOldPassword, &itemCount );
			if ( (pOldPassword != nil) && (strlen(pOldPassword) >= kHashRecoverableLength) )
				return ( eDSAuthPasswordTooLong );
			if ( siResult != eDSNoErr )
				return( siResult );
			if ( itemCount != 3 )
				return( eDSInvalidBuffFormat );
			
			// this allocates a copy of the string
			pNewPassword = dsDataListGetNodeStringPriv(dataList, 3);
			if ( pNewPassword == NULL ) return( eDSInvalidBuffFormat );
			if ( (pNewPassword != nil) && (strlen(pNewPassword) >= kHashRecoverableLength) )
				return ( eDSAuthPasswordTooLong );
			break;
		
		case kAuthSetShadowHashWindows:
		case kAuthSetShadowHashSecure:
		case kAuthNativeClearTextOK:
		case kAuthNativeNoClearText:
		case kAuthNativeRetainCredential:
			mPostAuthEvent = true;
			siResult = (tDirStatus)Get2FromBuffer(inAuthData, &dataList, &pUserName, &pOldPassword, &itemCount );
			break;
		
		case kAuthSetPasswdCheckAdmin:
			{
				char *pUserToChangeName = NULL;
				bool modifyingSelf;
				
				dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
				if ( dataList == NULL ) return( eDSInvalidBuffFormat );
				itemCount = dsDataListGetNodeCountPriv(dataList);
				if ( itemCount != 4 ) return( eDSInvalidBuffFormat );
				
				// this allocates a copy of the string
				pUserName = dsDataListGetNodeStringPriv(dataList, 3);
				if ( pUserName == NULL ) return( eDSInvalidBuffFormat );
				if ( strlen(pUserName) < 1 ) return( eDSInvalidBuffFormat );
				
				pOldPassword = dsDataListGetNodeStringPriv(dataList, 4);
				if ( pOldPassword == NULL )
					return( eDSInvalidBuffFormat );
				if ( strlen(pOldPassword) < 1 )
					return( eDSInvalidBuffFormat );
				
				pUserToChangeName = dsDataListGetNodeStringPriv(dataList, 1);
				if ( pUserToChangeName == NULL )
					return( eDSInvalidBuffFormat );
				if ( strlen(pUserToChangeName) < 1 ) {
					free( pUserToChangeName );
					return( eDSInvalidBuffFormat );
				}
				
				modifyingSelf = (pUserToChangeName != NULL) && (pUserName != NULL) && (strcmp(pUserToChangeName,
					pUserName) == 0);
					
				DSFreeString( pUserToChangeName );
			}
			break;
			
		case kAuthGetPolicy:
		case kAuthGetEffectivePolicy:
			// possible to return data
			if ( inAuthStepData == NULL ) return( eDSNullAuthStepData );
			inAuthStepData->fBufferLength = 0;
			
			// parse input
			dataList = dsAuthBufferGetDataListAllocPriv( inAuthData );
			if ( dataList == NULL ) return( eDSInvalidBuffFormat );
			itemCount = dsDataListGetNodeCountPriv( dataList );
			if ( (uiAuthMethod == kAuthGetPolicy && itemCount != 3) || (uiAuthMethod == kAuthGetEffectivePolicy && itemCount != 1) )
				return( eDSInvalidBuffFormat );
			
			// this allocates a copy of the string
			pUserName = dsDataListGetNodeStringPriv(dataList, (uiAuthMethod == kAuthGetPolicy ? 3 : 1));
			if ( pUserName == NULL ) return( eDSInvalidBuffFormat );
			if ( strlen(pUserName) < 1 ) return( eDSInvalidBuffFormat );
			break;
		
		case kAuthSetPolicy:
			// parse input
			dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
			if ( dataList == NULL ) return( eDSInvalidBuffFormat );
			itemCount = dsDataListGetNodeCountPriv(dataList);
			if ( itemCount != 4 ) return( eDSInvalidBuffFormat );
			
			// this allocates a copy of the string
			pAdminUser = dsDataListGetNodeStringPriv(dataList, 1);
			if ( DSIsStringEmpty(pAdminUser) )
			{
				DSFreeString( pAdminUser );
			}
			else
			{
				pOldPassword = dsDataListGetNodeStringPriv(dataList, 2);
			}
			
			pUserName = dsDataListGetNodeStringPriv(dataList, 3);
			if ( pUserName == NULL || pUserName[0] == '\0' )
				return( eDSInvalidBuffFormat );
			
			pNewPassword = dsDataListGetNodeStringPriv(dataList, 4);
			if ( pNewPassword == NULL || pNewPassword[0] == '\0' )
				return( eDSInvalidBuffFormat );
			break;
			
		case kAuthGetGlobalPolicy:
			// possible to return data
			if ( inAuthStepData == NULL )
				return( eDSNullAuthStepData );
			inAuthStepData->fBufferLength = 0;
			break;
		
		case kAuthSetGlobalPolicy:
			break;
		
		case kAuthSetLMHash:
			siResult = GetNameAndDataFromBuffer( inAuthData, &dataList, &pUserName, &pNTLMDigest, &ntlmDigestLen, &itemCount );
			break;
		
		case kAuthNTSetWorkstationPasswd:
		case kAuthSMB_NTUserSessionKey:
			siResult = GetNameAndDataFromBuffer( inAuthData, &dataList, &pUserName, &pNTLMDigest, &ntlmDigestLen, &itemCount );
			break;
		
		case kAuthMSLMCHAP2ChangePasswd:
			// use <pOldPassword> to store the character encoding ("0"=UTF8 or "1"=Unicode(16))
			siResult = (tDirStatus)Get2FromBuffer( inAuthData, &dataList, &pUserName, &pOldPassword, &itemCount );
			if ( siResult != eDSNoErr )
				return( siResult );
			if ( itemCount != 3 )
				return( eDSInvalidBuffFormat );
			
			siResult = dsDataListGetNodePriv( dataList, 3, &secureHashNode );
			if ( siResult != eDSNoErr || secureHashNode == NULL )
				return( eDSInvalidBuffFormat );
			
			ntlmDigestLen = secureHashNode->fBufferLength;
			pNTLMDigest = (uint8_t *)malloc( ntlmDigestLen );
			if ( pNTLMDigest == NULL )
				return( eMemoryError );
			
			memmove( pNTLMDigest, ((tDataBufferPriv*)secureHashNode)->fBufferData, ntlmDigestLen );
			break;
		
		case kAuthSetCertificateHashAsRoot:
			siResult = (tDirStatus)Get2FromBuffer( inAuthData, &dataList, &pUserName, &pNewPassword, &itemCount );
			if ( siResult != eDSNoErr )
				return( siResult );
			if ( itemCount != 2 )
				return( eDSInvalidBuffFormat );
			break;
		
		case kAuthSASLProxy:
			siResult = (tDirStatus)Get2FromBuffer( inAuthData, &dataList, &pUserName, &nativeAttrType, &itemCount );
			if ( siResult != eDSNoErr )
				return( siResult );
			if ( itemCount == 3 )
			{
				tDataNodePtr dataNodePtr = NULL;
				
				siResult = dsDataListGetNodePriv( dataList, 3, &dataNodePtr );
				if ( siResult != eDSNoErr )
					return( siResult );
				if ( dataNodePtr == NULL )
					return( (tDirStatus)eDSInvalidBuffFormat );
				
				pCramResponse = (unsigned char *) calloc( dataNodePtr->fBufferLength + 1, 1 );
				if ( pCramResponse == NULL )
					throw( (tDirStatus)eMemoryError );
				
				cramResponseLen = dataNodePtr->fBufferLength;
				memcpy( pCramResponse, ((tDataBufferPriv*)dataNodePtr)->fBufferData, cramResponseLen );
			}
			else
			{
				return( eDSInvalidBuffFormat );
			}
			break;
		
		default:
			break;
	}
	
	return siResult;
}


// ---------------------------------------------------------------------------
//	* DictionaryFromAuthItems
// ---------------------------------------------------------------------------

void
CDSAuthParams::PostEvent( tDirStatus inAuthResult )
{
	if ( mPostAuthEvent )
	{
		CFStringRef eventType = (inAuthResult == eDSNoErr) ? CFSTR("auth.success") : CFSTR("auth.failure");
		CFMutableDictionaryRef eventDict = dsCreateEventLogDict( eventType, pUserName, serviceInfoDict );
		if ( eventDict != NULL ) {
			dsPostEvent( eventType, eventDict );
			CFRelease( eventDict );
		}
	}
}


// ---------------------------------------------------------------------------
//	* DictionaryFromAuthItems
//
//	Returns: CFDictionaryRef that contains an associated version of the
//				authentication buffer items.
//
//	Example:
/*
		<?xml version="1.0" encoding="UTF-8"?>
		<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
		<plist version="1.0">
		<dict>
			<key>Authenticator</key>
			<dict>
				<key>authentication_authority</key>
				<array>
					<string>;ShadowHash;</string>
				</array>
				<key>guid</key>
				<string>665B2208-DC58-4EA5-AB9B-12A6C06C6047</string>
				<key>name</key>
				<string>admin</string>
				<key>password</key>
				<string>password</string>
			</dict>
			<key>ServiceInformation</key>
			<dict/>
			<key>TargetUser</key>
			<dict>
				<key>authentication_authority</key>
				<array>
					<string>;ShadowHash;&lt;HASHLIST:SALTED-SHA1&gt;</string>
				</array>
				<key>policy</key>
				<dict/>
				<key>guid</key>
				<string>665B2208-DC58-4EA5-AB9B-12A6C06C6048</string>
				<key>name</key>
				<string>local</string>
				<key>NewPassword</key>
				<string>password2</string>
				<key>OldPassword</key>
				<string>password1</string>
			</dict>
		</dict>
		</plist>
*/
//	Some keys must be provided by the individual plug-ins. Each plug-in
//	has its own mechanism for retrieving data from user records.
//	plug-in keys: authentication_authority, guid, state.
// ---------------------------------------------------------------------------

CFMutableDictionaryRef
CDSAuthParams::DictionaryFromAuthItems( void )
{
	CFMutableDictionaryRef rootItemDict;
	rootItemDict = CFDictionaryCreateMutable(
							kCFAllocatorDefault,
							0,
							&kCFTypeDictionaryKeyCallBacks,
							&kCFTypeDictionaryValueCallBacks );
	
	CFMutableDictionaryRef authenticatorDict;
	authenticatorDict = CFDictionaryCreateMutable(
							kCFAllocatorDefault,
							0,
							&kCFTypeDictionaryKeyCallBacks,
							&kCFTypeDictionaryValueCallBacks );
	
	CFMutableDictionaryRef targetUserDict;
	targetUserDict = CFDictionaryCreateMutable(
							kCFAllocatorDefault,
							0,
							&kCFTypeDictionaryKeyCallBacks,
							&kCFTypeDictionaryValueCallBacks );
		
	CFDictionarySetValue( rootItemDict, CFSTR("Authenticator"), authenticatorDict );
	CFDictionarySetValue( rootItemDict, CFSTR("TargetUser"), targetUserDict );

	// authenticator dict
	if ( pAdminUser != NULL ) {
		CFStringRef adminName = CFStringCreateWithCString( kCFAllocatorDefault, pAdminUser, kCFStringEncodingUTF8 );
		CFDictionarySetValue( authenticatorDict, CFSTR("name"), adminName );
		CFRelease( adminName );
	}
		
	if ( pAdminPassword != NULL ) {
		CFStringRef adminPassword = CFStringCreateWithCString( kCFAllocatorDefault, pAdminPassword, kCFStringEncodingUTF8 );
		CFDictionarySetValue( authenticatorDict, CFSTR("password"), adminPassword );
		CFRelease( adminPassword );
	}
	
	// service information dict
	CFMutableDictionaryRef lServiceInfoDict = NULL;
	
	if ( serviceInfoDict != NULL ) {
		lServiceInfoDict = CFDictionaryCreateMutableCopy(
							kCFAllocatorDefault,
							0,
							serviceInfoDict );
		CFDictionarySetValue( rootItemDict, CFSTR("ServiceInformation"), lServiceInfoDict );
		CFRelease( lServiceInfoDict );
	}
	
	// target user dict
	if ( pUserName != NULL ) {
		CFStringRef userName = CFStringCreateWithCString( kCFAllocatorDefault, pUserName, kCFStringEncodingUTF8 );
		CFDictionarySetValue( targetUserDict, CFSTR("name"), userName );
		CFRelease( userName );
	}
	
	if ( pNewPassword != NULL ) {
		CFStringRef userNewPassword = CFStringCreateWithCString( kCFAllocatorDefault, pNewPassword, kCFStringEncodingUTF8 );
		CFDictionarySetValue( targetUserDict, CFSTR("NewPassword"), userNewPassword );
		CFRelease( userNewPassword );
	}
		
	if ( pOldPassword != NULL ) {
		CFStringRef userOldPassword = CFStringCreateWithCString( kCFAllocatorDefault, pOldPassword, kCFStringEncodingUTF8 );
		CFDictionarySetValue( targetUserDict, CFSTR("OldPassword"), userOldPassword );
		CFRelease( userOldPassword );
	}
	
	char *xmlDataStr = NULL;
	CFDataRef xmlData = NULL;
	CFMutableDictionaryRef xmlDataDict = NULL;
	CFStringRef errorString = NULL;

	if ( policyStr != NULL ) {
		if ( ConvertGlobalSpaceDelimitedPolicyToXML(policyStr, &xmlDataStr) == 0 ) {
			xmlData = CFDataCreate( kCFAllocatorDefault, (UInt8 *)xmlDataStr, strlen(xmlDataStr) );
			xmlDataDict = (CFMutableDictionaryRef) CFPropertyListCreateFromXMLData( kCFAllocatorDefault, xmlData,
												kCFPropertyListMutableContainersAndLeaves, &errorString );
			
			CFDictionarySetValue( targetUserDict, CFSTR("policy"), xmlDataDict );
			CFRelease( xmlDataDict );
		}
	}
	
	// clean up
	CFRelease( authenticatorDict );
	CFRelease( targetUserDict );
	
	return rootItemDict;
}


// ---------------------------------------------------------------------------
//	* DictionaryFromAuthItems
//
//	Returns: CFDictionaryRef that contains an associated version of the
//				authentication buffer items.
// ---------------------------------------------------------------------------
void
CDSAuthParams::SetParamsFromDictionary( CFDictionaryRef inKeyedAuthItems )
{
}


// ---------------------------------------------------------------------------
//	* ExtractServiceInfo
//
//	Returns: DS error
//
//	Checks for service information in the second dsDoDirNodeAuth() buffer.
//	Returns eDSNoErr if the information is not included; only returns an
//	error if the data is provided but invalid.
// ---------------------------------------------------------------------------

tDirStatus CDSAuthParams::ExtractServiceInfo( tDataBufferPtr inAuthStepData )
{
	tDirStatus				status					= eDSNoErr;
	char					*plistStr				= NULL;
	int						plistStrLen				= 0;
	CFDataRef				plistData				= NULL;
	CFDictionaryRef			plistDict				= NULL;
	CFDictionaryRef			infoDict				= NULL;
	CFDataRef				infoData				= NULL;
	CFStringRef				errorString				= NULL;
	
	DSCFRelease( serviceInfoDict );
	
	do
	{
		if ( GetUserNameFromAuthBuffer(inAuthStepData, 1, &plistStr, &plistStrLen) == eDSNoErr && plistStr != NULL )
		{
			// make sure we got a plist
			plistData = CFDataCreate( kCFAllocatorDefault, (const unsigned char *)plistStr, plistStrLen );
			if ( plistData == NULL )
				return( eMemoryError );
			
			plistDict = (CFDictionaryRef) CFPropertyListCreateFromXMLData( kCFAllocatorDefault, plistData,
							kCFPropertyListImmutable, &errorString );
			if ( plistDict == NULL || CFGetTypeID(plistDict) != CFDictionaryGetTypeID() ) {
				status = eDSInvalidBuffFormat;
				break;
			}
			
			// Extract the ServiceInformation dictionary
			if ( CFDictionaryGetValueIfPresent(plistDict, CFSTR("ServiceInformation"), (const void **)&infoDict) && infoDict != NULL )
			{
				if ( CFGetTypeID(infoDict) != CFDictionaryGetTypeID() ) {
					status = eDSInvalidBuffFormat;
					break;
				}
				
				serviceInfoDict = (CFDictionaryRef) CFRetain( infoDict );
			}
		}
	}
	while ( 0 );
	
	DSCFRelease( plistDict );
	DSCFRelease( plistData );
	DSCFRelease( errorString );
	DSCFRelease( infoData );
	DSFreeString( plistStr );		
	
	return status;
}

