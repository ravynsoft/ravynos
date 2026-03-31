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

#ifndef DISABLE_LOCAL_PLUGIN

#include <string.h>
#include "buffer_unpackers.h"
#include "CDSAuthDefs.h"
#include "CDSLocalAuthHelper.h"
#include "CDSLocalAuthParams.h"
#include "PrivateTypes.h"
#include "DSUtils.h"
#include "AuthHelperUtils.h"

CDSLocalAuthParams::CDSLocalAuthParams() : CDSAuthParams()
{
	stateFilePath						= NULL;
	targetUserStateFilePath				= NULL;
	bFetchHashFiles						= false;
	
	bzero(&state, sizeof(state));
	bzero(&initialState, sizeof(initialState));
	bzero(&targetUserState, sizeof(targetUserState));
	
	ZeroHashes();
}


CDSLocalAuthParams::~CDSLocalAuthParams()
{
	DSFreeString( stateFilePath );
	DSFreeString( targetUserStateFilePath );
}


tDirStatus
CDSLocalAuthParams::LoadDSLocalParamsForAuthMethod(
	UInt32 inAuthMethod,
	UInt32 inUserLevelHashList,
	const char* inGUIDString,
	bool inAuthedUserIsAdmin,
	tDataBufferPtr inAuthData,
	tDataBufferPtr inAuthStepData )
{
	bFetchHashFiles = false;
	
	tDirStatus siResult = ExtractServiceInfo( inAuthStepData );
	if ( siResult != eDSNoErr )
		return siResult;
	
	// release values that can leak if reused
	DSFreeString( path );
	DSFreeString( stateFilePath );
	DSFreeString( targetUserStateFilePath );
	
	switch( inAuthMethod )
	{
		case kAuthDIGEST_MD5:
			bFetchHashFiles = true;
			break;
			
		case kAuthCRAM_MD5:
			if ( (inUserLevelHashList & ePluginHashCRAM_MD5) == 0 )
				return( eDSAuthFailed );
			bFetchHashFiles = true;
			break;
			
		case kAuthAPOP:
			if ( (inUserLevelHashList & ePluginHashRecoverable) == 0 )
				return( eDSAuthFailed );
			bFetchHashFiles = true;
			break;
			
		case kAuthSMB_NT_Key:
			if ( (inUserLevelHashList & ePluginHashNT) == 0 )
				return( eDSAuthFailed );
			bFetchHashFiles = true;
			break;
			
		case kAuthSMB_LM_Key:
			if ( (inUserLevelHashList & ePluginHashLM) == 0 )
				return( eDSAuthFailed );
			bFetchHashFiles = true;
			break;
			
		case kAuthNTLMv2:
			if ( (inUserLevelHashList & ePluginHashNT) == 0 )
				return( eDSAuthFailed );
			bFetchHashFiles = true;
			break;
		
		case kAuthMSCHAP2:
			if ( (inUserLevelHashList & ePluginHashNT) == 0 )
				return( eDSAuthFailed );
			if ( inAuthStepData == NULL )
				return( eDSNullAuthStepData );
			if ( inAuthStepData->fBufferSize < 4 + MS_AUTH_RESPONSE_LENGTH )
				return( eDSBufferTooSmall );
			bFetchHashFiles = true;
			break;
		
		case kAuthVPN_PPTPMasterKeys:
			if ( inAuthStepData == NULL )
				return( eDSNullAuthStepData );
			if ( inAuthStepData->fBufferSize < (UInt32)(8 + keySize*2) )
				return( eDSBufferTooSmall );
			bFetchHashFiles = true;
			break;
			
		case kAuthSMBWorkstationCredentialSessionKey:
			if ( inAuthStepData == NULL )
				return( eDSNullAuthStepData );
			if ( inAuthStepData->fBufferSize < (UInt32)(sizeof(UInt32) + 8) )
				return( eDSBufferTooSmall );
			bFetchHashFiles = true;
			break;
			
		case kAuthSecureHash:
			if ( (inUserLevelHashList & ePluginHashSaltedSHA1) == 0 )
				return( eDSAuthFailed );
			
			bFetchHashFiles = true;
			break;

		case kAuthWriteSecureHash:
		case kAuthReadSecureHash:
			break;
		
		// set password operations
		case kAuthSetPasswd:
		case kAuthSetPasswdAsRoot:
			bFetchHashFiles = true;
			break;
		
		case kAuthSetPolicyAsRoot:
			siResult = ReadStateFile(pUserName, inGUIDString, &modDateOfPassword, &path, &targetUserStateFilePath, &targetUserState,
				&hashesLengthFromFile);
			break;
					
		case kAuthChangePasswd:
		case kAuthSetShadowHashWindows:
		case kAuthSetShadowHashSecure:
		case kAuthNativeClearTextOK:
		case kAuthNativeNoClearText:
		case kAuthNativeRetainCredential:
			bFetchHashFiles = true;
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
				
				if ( !modifyingSelf && !inAuthedUserIsAdmin )
					return( eDSPermissionError );
				
				//read file
				bFetchHashFiles = true;
			}
			break;
			
		case kAuthGetPolicy:
		case kAuthGetEffectivePolicy:
			ReadStateFile(pUserName, inGUIDString, &modDateOfPassword, &path, &stateFilePath, &state, &hashesLengthFromFile);
			DSFreeString( stateFilePath );	// macro sets to NULL (required)
			break;
			
		case kAuthSetPolicy:
			siResult = ReadStateFile(pUserName, inGUIDString, &modDateOfPassword, &path, &targetUserStateFilePath, &targetUserState,
				&hashesLengthFromFile);
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
		case kAuthNTSetWorkstationPasswd:
		case kAuthSMB_NTUserSessionKey:
			bFetchHashFiles = true;
			break;
		
		case kAuthMSLMCHAP2ChangePasswd:
			bFetchHashFiles = true;
			break;
		
		case kAuthPPS:
			bFetchHashFiles = true;
			break;
		
		default:
			break;
	}

	if ( stateFilePath != NULL )
		memcpy( &initialState, &state, sizeof(initialState) );
	
	return siResult;
}


bool
CDSLocalAuthParams::PolicyStateChanged( void )
{
	return ( memcmp(&initialState, &state, sizeof(initialState)) != 0 );
}

#endif // DISABLE_LOCAL_PLUGIN
