/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
 * @header CDSLocalAuthHelper
 */

#ifndef DISABLE_LOCAL_PLUGIN

#include <CoreFoundation/CoreFoundation.h>

#include "CDSLocalAuthHelper.h"
#include "AuthHelperUtils.h"
#include "CDSAuthDefs.h"
#include "CAuthAuthority.h"
#include "CDSLocalAuthParams.h"

#include "DirServices.h"
#include "DirServicesTypes.h"
#include "DirServicesUtils.h"
#include "DirServicesConst.h"
#include "DirServicesConstPriv.h"
#include "CFile.h"
#include "DSUtils.h"
#include "CLog.h"
#include "SMBAuth.h"
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <Security/Authorization.h>
#include <PasswordServer/AuthDBFileDefs.h>
#include <PasswordServer/KerberosInterface.h>
#include <DirectoryServiceCore/CContinue.h>
#include <syslog.h>
#include <sys/time.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <mach/mach_time.h>	// for dsTimeStamp
#include <uuid/uuid.h>

#include "chap.h"
#include "chap_ms.h"
#include "buffer_unpackers.h"
#include "CDSPluginUtils.h"
#include "CDSLocalPluginNode.h"
#include "CRefTable.h"
#include <DirectoryServiceCore/pps.h>
#include <Mbrd_MembershipResolver.h>

#include <string>		//STL string class


#define kDoNotTouchTheAuthAuthorities		false

extern "C" {
extern void CvtHex(HASH Bin, HASHHEX Hex);
};

typedef struct AuthAuthorityHandler {
	const char* fTag;
	AuthAuthorityHandlerProc fHandler;
} AuthAuthorityHandler;

tDirStatus ParseLocalCacheUserAuthData(	const char	   *inAuthData,
										char		  **outNodeName,
										char		  **outRecordName,
										char		  **outGUID );

static AuthAuthorityHandler sAuthAuthorityHandlerProcs[] =
{
	{ kDSTagAuthAuthorityBasic,				(AuthAuthorityHandlerProc)CDSLocalAuthHelper::DoBasicAuth },
	{ kDSTagAuthAuthorityLocalWindowsHash,	(AuthAuthorityHandlerProc)CDSLocalAuthHelper::DoShadowHashAuth },
	{ kDSTagAuthAuthorityShadowHash,		(AuthAuthorityHandlerProc)CDSLocalAuthHelper::DoShadowHashAuth },
	{ kDSTagAuthAuthorityKerberosv5,		(AuthAuthorityHandlerProc)CDSLocalAuthHelper::DoKerberosAuth },
	{ kDSTagAuthAuthorityKerberosv5Cert,	(AuthAuthorityHandlerProc)CDSLocalAuthHelper::DoKerberosCertAuth },
	{ kDSTagAuthAuthorityLocalCachedUser,	(AuthAuthorityHandlerProc)CDSLocalAuthHelper::DoLocalCachedUserAuth },
	{ kDSTagAuthAuthorityPasswordServer,	(AuthAuthorityHandlerProc)CDSLocalAuthHelper::DoPasswordServerAuth },
	{ kDSTagAuthAuthorityDisabledUser,		(AuthAuthorityHandlerProc)CDSLocalAuthHelper::DoDisabledAuth },
	{ NULL, NULL }
};

extern DSMutexSemaphore *gHashAuthFailedLocalMapLock;
extern dsBool gServerOS;
extern pid_t					gDaemonPID;
extern in_addr_t				gDaemonIPAddress;
extern CContinue				*gLocalContinueTable;
extern CRefTable				gRefTable;

HashAuthFailedMap gHashAuthFailedLocalMap;
static char sZeros[kHashRecoverableLength] = {0};


// ---------------------------------------------------------------------------
//	* GetUserNameFromAuthBuffer
//    retrieve the username from a standard auth buffer
//    buffer format should be 4 byte length followed by username, then optional
//    additional data after. Buffer length must be at least 5 (length + 1 character name)
// ---------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::CopyUserNameFromAuthBuffer( tDataBufferPtr inAuthData, UInt32 inUserNameIndex,
	 CFStringRef *outUserName )
{
	tDataListPtr dataList = ::dsAuthBufferGetDataListAllocPriv(inAuthData);
	if (dataList != NULL)
	{
		char* userNameCStr = ::dsDataListGetNodeStringPriv(dataList, inUserNameIndex);
		// this allocates a copy of the string
		
		*outUserName = CFStringCreateWithCString( NULL, userNameCStr, kCFStringEncodingUTF8 );
		
		::free( userNameCStr );
		
		::dsDataListDeallocatePriv(dataList);
		::free(dataList);
		dataList = NULL;
		return eDSNoErr;
	}
	return eDSInvalidBuffFormat;
}

bool CDSLocalAuthHelper::AuthAuthoritiesHaveTag( CFStringRef inRecordName, CFArrayRef inAuthAuthorities, CFStringRef inTag )
{
	if ( inAuthAuthorities == NULL )
		return false;
	if ( inTag == NULL )
		return false;
	if ( inRecordName == NULL )
		return false;
	
	CFIndex numAuthAuthorities = CFArrayGetCount( inAuthAuthorities );

	// we need to be a little more careful in the case the record name  and the tag are the same
	if ( CFStringCompare( inRecordName, inTag, 0 ) == kCFCompareEqualTo )
	{
		char* aaVersion = NULL;
		char* aaTag = NULL;
		char* aaData = NULL;
		char* cStr = NULL;
		size_t cStrSize = 0;
		char* cStr2 = NULL;
		size_t cStrSize2 = 0;
		bool hasTag = false;
		for( CFIndex i=0; i<numAuthAuthorities; i++ )
		{
			const char* authAuthorityCStr = CStrFromCFString( (CFStringRef)CFArrayGetValueAtIndex(
				inAuthAuthorities, i ), &cStr, &cStrSize, NULL );
			tDirStatus dirStatus = ::dsParseAuthAuthority( authAuthorityCStr, &aaVersion, &aaTag, &aaData );
			if ( dirStatus != eDSNoErr )
			{
				DbgLog(  kLogPlugin, "CDSLocalPlugin::AuthAuthoritiesHaveTag(): dsParseAuthAuthority() returned an error %d",
					dirStatus );
				continue;
			}
			const char* tagCStr = CStrFromCFString( inTag, &cStr2, &cStrSize2, NULL );
			if ( ::strcasecmp( tagCStr, aaTag ) == 0 )
			{
				hasTag = true;
				break;
			}
			else if ( ::strcasecmp( kDSTagAuthAuthorityDisabledUser, aaTag ) == 0 &&
				::strcasestr( authAuthorityCStr, tagCStr ) != NULL )
			{
				hasTag = true;
				break;
			}
		}
		
		if ( cStr != NULL )
			::free( cStr );
		if ( cStr2 != NULL )
			::free( cStr2 );
		if ( aaVersion != NULL )
			::free( aaVersion );
		if ( aaTag != NULL )
			::free( aaTag );
		if ( aaData != NULL )
			::free( aaData );
		
		return hasTag;
	}
	else
	{
		for( CFIndex i=0; i<numAuthAuthorities; i++ )
		{
			if ( CFStringFind( (CFStringRef)CFArrayGetValueAtIndex( inAuthAuthorities, i ), inTag, 0 ).location !=
					kCFNotFound )
				return  true;
		}
		
		return false;
	}
}

tDirStatus
CDSLocalAuthHelper::DoKerberosAuth(
	tDirNodeReference inNodeRef,
	CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData,
	tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData,
	bool inAuthOnly,
	bool isSecondary,
	CAuthAuthority &inAuthAuthorityList,
	const char* inGUIDString,
	bool inAuthedUserIsAdmin,
	CFMutableDictionaryRef inMutableRecordDict,
	unsigned int inHashList,
	CDSLocalPlugin* inPlugin,
	CDSLocalPluginNode* inNode,
	CFStringRef inAuthedUserName,
	uid_t inUID,
	uid_t inEffectiveUID,
	CFStringRef inNativeRecType )
{
	tDirStatus			siResult				= eDSAuthFailed;
	unsigned int		userLevelHashList		= inHashList;
	char				*aaData					= NULL;
	
	if ( inAuthData == NULL )
		return( eDSNullAuthStepData );
	
	if ( inParams.mAuthMethodStr != NULL )
	{
		DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoKerberosAuth(): Attempting use of authentication method %s",
				inParams.mAuthMethodStr );
	}
	
	// Parse input buffer
	siResult = inParams.LoadDSLocalParamsForAuthMethod( inParams.uiAuthMethod, userLevelHashList, inGUIDString, inAuthedUserIsAdmin,
														inAuthData, outAuthData );
	switch( siResult )
	{
		case eDSNoErr:
			break;
		case eDSAuthFailed:
			return eDSAuthMethodNotSupported;
		default:
			return siResult;
	}
	
	if ( inParams.bFetchHashFiles )
	{
		CDSLocalAuthHelper::ReadShadowHashAndStateFiles(
			inParams.pUserName,
			inGUIDString,
			inParams.hashes,
			&inParams.modDateOfPassword,
			&inParams.path,
			&inParams.stateFilePath,
			&inParams.state,
			&inParams.hashesLengthFromFile );
	}
	
	// get the current principal name
	aaData = inAuthAuthorityList.GetDataForTag( kDSTagAuthAuthorityKerberosv5, 1 );
	
	switch( inParams.uiAuthMethod )
	{
		// set password operations
		case kAuthSetPasswd:
			siResult = GetUserPolicies( inMutableRecordDict, NULL, inPlugin, &inParams.policyStr );
			if ( siResult == eDSNoErr )
			{
				CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess,
					&inParams.globalMoreAccess );
				if ( ! inAuthedUserIsAdmin )
				{
					// non-admins can only change their own passwords
					bool authFailed = false;
					if ( ( inAuthedUserName == NULL ) || ( inParams.pUserName == NULL ) )
						authFailed = true;
					else
					{
						CFStringRef userNameCFStr = CFStringCreateWithCString( NULL, inParams.pUserName,
							kCFStringEncodingUTF8 );
						if ( CFStringCompare( userNameCFStr, inAuthedUserName, 0 ) != kCFCompareEqualTo )
							authFailed = true;
						CFRelease( userNameCFStr );
					}
					
					if ( authFailed )
						siResult = eDSPermissionError;
					else
					{
						siResult = CDSLocalAuthHelper::PasswordOkForPolicies( inParams.policyStr, &inParams.globalAccess,
										inParams.pUserName, inParams.pNewPassword );
						if ( siResult == eDSAuthPasswordTooShort &&
							 inParams.globalAccess.minChars == 0 &&
							 inParams.pNewPassword != NULL &&
							 *inParams.pNewPassword == '\0' &&
							 ((inParams.policyStr == NULL) || strstr(inParams.policyStr, "minChars=0") != NULL) )
						{
							// special-case for ShadowHash and blank password.
							siResult = eDSNoErr;
						}
					}
				}
				if ( siResult == eDSNoErr )
				{
					char *localKDCRealmStr = GetLocalKDCRealmWithCache( kLocalKDCRealmCacheTimeout );
					if ( localKDCRealmStr != NULL )
					{
						bool oldNewPrincEq = (aaData && inParams.pUserName && strcmp(aaData, inParams.pUserName) == 0);
						pwsf_DeletePrincipalInLocalRealm( aaData, localKDCRealmStr );
						if ( !oldNewPrincEq )
							pwsf_DeletePrincipalInLocalRealm( inParams.pUserName, localKDCRealmStr );
						if ( pwsf_AddPrincipalToLocalRealm(inParams.pUserName, inParams.pNewPassword, localKDCRealmStr) != 0 )
							DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoKerberosAuth(): Unable to add principal %s@%s",
								inParams.pUserName, localKDCRealmStr );
						
						if ( !oldNewPrincEq )
						{
							size_t princStrLen = strlen(inParams.pUserName) + strlen(localKDCRealmStr) + 2;
							char *princStr = (char *)malloc( princStrLen );
							if ( princStr != NULL )
							{
								snprintf( princStr, princStrLen, "%s@%s", inParams.pUserName, localKDCRealmStr );
								inAuthAuthorityList.SetDataForTag( kDSTagAuthAuthorityKerberosv5, princStr, 1 );
								SaveAuthAuthorities( inPlugin, inNodeRef, inParams.pUserName, inNativeRecType, inAuthAuthorityList );
								free( princStr );
							}
							else
							{
								siResult = eMemoryError;
							}
						}
						
						free( localKDCRealmStr );
					}
				}
			}
			break;
		
		case kAuthSetPasswdAsRoot:
			{
				siResult = eDSAuthFailed;
				if ( inAuthedUserName != NULL ) {
					CFStringRef userNameCFStr = CFStringCreateWithCString( NULL, inParams.pUserName,
						kCFStringEncodingUTF8 );
					if ( CFStringCompare( userNameCFStr, inAuthedUserName, 0 ) == kCFCompareEqualTo )
						siResult = eDSNoErr;
					DSCFRelease( userNameCFStr );
				}
				
				if ( siResult != eDSNoErr ) {
					bool accessAllowed = inNode->AccessAllowed( inAuthedUserName, inEffectiveUID, inNativeRecType, 
															    inPlugin->AttrNativeTypeForStandardType(CFSTR(kDS1AttrPassword)), 
															    eDSAccessModeWriteAttr );
					if ( accessAllowed == true ) {
						siResult = eDSNoErr;
					}
					else {
						siResult = eDSPermissionError;
					}
				}
				
				if ( siResult == eDSNoErr )
				{
					CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess,
						&inParams.globalMoreAccess );
					
					// admins are allowed to set passwords not governed by policy
					// TODO: why is admin allowed to bypass password policies when setting a password
					if ( inAuthedUserIsAdmin == false )
					{
						siResult = CDSLocalAuthHelper::PasswordOkForPolicies( inParams.policyStr, &inParams.globalAccess,
							inParams.pUserName, inParams.pNewPassword );
						if ( siResult == eDSAuthPasswordTooShort &&
							 inParams.globalAccess.minChars == 0 &&
							 inParams.pNewPassword != NULL &&
							 *inParams.pNewPassword == '\0' &&
							 ((inParams.policyStr == NULL) || strstr(inParams.policyStr, "minChars=0") != NULL) )
						{
							// special-case for ShadowHash and blank password.
							siResult = eDSNoErr;
						}
					}
					if ( siResult == eDSNoErr )
					{
						char *localKDCRealmStr = GetLocalKDCRealmWithCache( kLocalKDCRealmCacheTimeout );
						if ( localKDCRealmStr != NULL )
						{
							bool oldNewPrincEq = (aaData && inParams.pUserName && strcmp(aaData, inParams.pUserName) == 0);
							pwsf_DeletePrincipalInLocalRealm( aaData, localKDCRealmStr );
							if ( !oldNewPrincEq )
								pwsf_DeletePrincipalInLocalRealm( inParams.pUserName, localKDCRealmStr );
							if ( pwsf_AddPrincipalToLocalRealm(inParams.pUserName, inParams.pNewPassword, localKDCRealmStr) != 0 )
								DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoKerberosAuth(): Unable to add principal %s@%s",
									inParams.pUserName, localKDCRealmStr );
							
							if ( !oldNewPrincEq )
							{
								size_t princStrLen = strlen(inParams.pUserName) + strlen(localKDCRealmStr) + 2;
								char *princStr = (char *)malloc( princStrLen );
								if ( princStr != NULL )
								{
									snprintf( princStr, princStrLen, "%s@%s", inParams.pUserName, localKDCRealmStr );
									inAuthAuthorityList.SetDataForTag( kDSTagAuthAuthorityKerberosv5, princStr, 1 );
									SaveAuthAuthorities( inPlugin, inNodeRef, inParams.pUserName, inNativeRecType, inAuthAuthorityList );
									free( princStr );
								}
								else
								{
									siResult = eMemoryError;
								}
							}
							
							free( localKDCRealmStr );
						}
					}
				}
			}
			break;

		case kAuthChangePasswd:
			siResult = GetUserPolicies( inMutableRecordDict, NULL, inPlugin, &inParams.policyStr );
			if ( siResult == eDSNoErr )
			{
				CDSLocalAuthHelper::GenerateShadowHashes( gServerOS,
									inParams.pOldPassword,
									strlen(inParams.pOldPassword),
									userLevelHashList,
									inParams.hashes + kHashOffsetToSaltedSHA1,
									inParams.generatedHashes,
									&inParams.hashLength );
				
				if ( HashesEqual( inParams.hashes, inParams.generatedHashes ) || isSecondary )
				{
					bool notAdmin = ( (inParams.pUserName != NULL) && !inAuthedUserIsAdmin );
					
					CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess,
						&inParams.globalMoreAccess );
					if ( notAdmin )
					{
						siResult = CDSLocalAuthHelper::PasswordOkForPolicies( inParams.policyStr, &inParams.globalAccess,
										inParams.pUserName, inParams.pNewPassword );
						if ( siResult == eDSAuthPasswordTooShort &&
							 inParams.globalAccess.minChars == 0 &&
							 inParams.pNewPassword != NULL &&
							 *inParams.pNewPassword == '\0' &&
							 ((inParams.policyStr == NULL) || strstr(inParams.policyStr, "minChars=0") != NULL) )
						{
							// special-case for ShadowHash and blank password.
							siResult = eDSNoErr;
						}
					}
					if ( siResult == eDSNoErr )
					{
						char *localKDCRealmStr = GetLocalKDCRealmWithCache( kLocalKDCRealmCacheTimeout );
						if ( localKDCRealmStr != NULL ) {
							pwsf_ChangePasswordInLocalRealm( inParams.pUserName, localKDCRealmStr, inParams.pNewPassword );
							free( localKDCRealmStr );
						}
					}
				}
				else
				{
					siResult = eDSAuthFailed;
				}
			}
			break;
		
		case kAuthMSLMCHAP2ChangePasswd:
			siResult = eDSAuthMethodNotSupported;
			break;
			
		default:
			siResult = eDSAuthMethodNotSupported;
	}
	
	DSFreeString( aaData );
	
	return siResult;
}


tDirStatus
CDSLocalAuthHelper::DoKerberosCertAuth(
	tDirNodeReference inNodeRef,
	CDSLocalAuthParams &inParams,
    tContextData *inOutContinueData,
	tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData,
	bool inAuthOnly,
	bool isSecondary,
	CAuthAuthority &inAuthAuthorityList,
	const char* inGUIDString,
	bool inAuthedUserIsAdmin,
	CFMutableDictionaryRef inMutableRecordDict,
	unsigned int inHashList,
	CDSLocalPlugin* inPlugin,
	CDSLocalPluginNode* inNode,
	CFStringRef inAuthedUserName,
	uid_t inUID,
	uid_t inEffectiveUID,
	CFStringRef inNativeRecType )
{
	tDirStatus			siResult				= eDSAuthFailed;
	unsigned int		userLevelHashList		= inHashList;
	char				*aaData					= NULL;
	
	if ( inAuthData == NULL )
		return( eDSNullAuthStepData );
	
	if ( inParams.mAuthMethodStr != NULL )
	{
		DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoKerberosCertAuth(): Attempting use of authentication method %s",
				inParams.mAuthMethodStr );
	}
	
	// Parse input buffer
	siResult = inParams.LoadDSLocalParamsForAuthMethod(
								inParams.uiAuthMethod, userLevelHashList, inGUIDString, inAuthedUserIsAdmin,
								inAuthData, outAuthData );
	switch( siResult )
	{
		case eDSNoErr:
			break;
		case eDSAuthFailed:
			return eDSAuthMethodNotSupported;
		default:
			return siResult;
	}
	
	// get the current principal name
	aaData = inAuthAuthorityList.GetDataForTag( kDSTagAuthAuthorityKerberosv5Cert, 1 );
	
	switch( inParams.uiAuthMethod )
	{		
		case kAuthSetCertificateHashAsRoot:
			{
				char *localKDCRealmStr = GetLocalKDCRealmWithCache( kLocalKDCRealmCacheTimeout );
				if ( localKDCRealmStr != NULL )
				{
					char *rndPassData = GenerateRandomComputerPassword();
					if ( rndPassData != NULL )
					{
						bool oldNewPrincEq = (aaData && inParams.pNewPassword && strcmp(aaData, inParams.pNewPassword) == 0);
						pwsf_DeletePrincipalInLocalRealm( inParams.pNewPassword, localKDCRealmStr );
						if ( !oldNewPrincEq && aaData != NULL )
							pwsf_DeletePrincipalInLocalRealm( aaData, localKDCRealmStr );
						if ( pwsf_AddPrincipalToLocalRealm(inParams.pNewPassword, rndPassData, localKDCRealmStr) == 0 )
						{
							pwsf_SetCertHashInLocalRealm( inParams.pNewPassword, inParams.pNewPassword, localKDCRealmStr );
						}
						else
						{
							DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoKerberosAuth(): Unable to add principal %s@%s",
								inParams.pUserName, localKDCRealmStr );
						}
						
						if ( !oldNewPrincEq )
						{
							size_t princStrLen = strlen(inParams.pNewPassword) + strlen(localKDCRealmStr) + 2;
							char *princStr = (char *)malloc( princStrLen );
							if ( princStr != NULL )
							{
								snprintf( princStr, princStrLen, "%s@%s", inParams.pNewPassword, localKDCRealmStr );
								inAuthAuthorityList.SetDataForTag( kDSTagAuthAuthorityKerberosv5Cert, princStr, 1 );
								SaveAuthAuthorities( inPlugin, inNodeRef, inParams.pUserName, inNativeRecType, inAuthAuthorityList );
								free( princStr );
							}
							else
							{
								siResult = eMemoryError;
							}
						}
						free( rndPassData );
					}
					
					free( localKDCRealmStr );
				}
			}
			break;
			
		default:
			siResult = eDSAuthMethodNotSupported;
	}
	
	DSFreeString( aaData );
	
	return siResult;
}


// ---------------------------------------------------------------------------
//	* DoShadowHashAuth
//
//	RETURNS: tDirStatus	final result of the authentication
//
//	This handler has an optional parameter <inOKToChangeAuthAuthorities>.
//	It is set to TRUE by default (called from CDSLocalPlugin::DoAuthentication),
//	but should be set to FALSE when forwarded from another type, such as
//	LocalCachedUser or Disabled. The original handler should be the one
//	to make changes.
// ---------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::DoShadowHashAuth(
	tDirNodeReference inNodeRef,
	CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData,
	tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData,
	bool inAuthOnly,
	bool isSecondary,
	CAuthAuthority &inAuthAuthorityList,
	const char* inGUIDString,
	bool inAuthedUserIsAdmin,
	CFMutableDictionaryRef inMutableRecordDict,
	unsigned int inHashList,
	CDSLocalPlugin* inPlugin,
	CDSLocalPluginNode* inNode,
	CFStringRef inAuthedUserName,
	uid_t inUID,
	uid_t inEffectiveUID,
	CFStringRef inNativeRecType )
{
	// add a true on the end; by default, it's ok to change the auth authorities.
	return DoShadowHashAuth( inNodeRef, inParams, inOutContinueData, inAuthData, outAuthData, inAuthOnly, isSecondary,
							 inAuthAuthorityList, inGUIDString, inAuthedUserIsAdmin, inMutableRecordDict, inHashList,
							 inPlugin, inNode, inAuthedUserName, inUID, inEffectiveUID, inNativeRecType, true );
}


tDirStatus CDSLocalAuthHelper::DoShadowHashAuth(
	tDirNodeReference inNodeRef,
	CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData,
	tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData,
	bool inAuthOnly,
	bool isSecondary,
	CAuthAuthority &inAuthAuthorityList,
	const char *inGUIDString,
	bool inAuthedUserIsAdmin,
	CFMutableDictionaryRef inMutableRecordDict,
	unsigned int inHashList,
	CDSLocalPlugin *inPlugin,
	CDSLocalPluginNode *inNode,
	CFStringRef inAuthedUserName,
	uid_t inUID,
	uid_t inEffectiveUID,
	CFStringRef inNativeRecType,
	bool inOKToChangeAuthAuthorities )
{
	tDirStatus				siResult						= eDSAuthFailed;
	int						saslError						= SASL_OK;
	unsigned int			userLevelHashList				= inHashList;
	time_t					now								= 0;
	sHashAuthFailed			*pHashAuthFailed				= NULL;
	UInt32					len								= 0;
	bool					bufferUserIsAdmin				= false;
	CFMutableArrayRef		myHashTypeArray					= NULL;
	bool					needToChange					= false;
	bool					bCheckDelay						= true;
	CFStringRef				adminString						= NULL;
	const char				*serverout						= NULL;
	unsigned int			serveroutlen					= 0;
	ServerAuthDataBlockPtr	ppsServerContext				= NULL;

	if ( inAuthData == NULL )
		return( eDSNullAuthStepData );
	
	if (inParams.mAuthMethodStr != NULL)
	{
		DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoShadowHashAuth(): Attempting use of authentication method %s",
			inParams.mAuthMethodStr );
	}
	
	inParams.aaData = inAuthAuthorityList.GetDataForTag( kDSTagAuthAuthorityShadowHash, 0 );
	
	// If a user hash-list is specified, then it takes precedence over the global list
	if ( inParams.aaData != NULL && inParams.aaData[0] != '\0' )
	{
		siResult = CDSLocalAuthHelper::GetHashSecurityLevelForUser( inParams.aaData, &userLevelHashList );
		if ( siResult != eDSNoErr )
		{
			DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoShadowHashAuth() - encountered invalid record hash list: %s",
				inParams.aaData );
			userLevelHashList = inHashList;
			siResult = eDSNoErr;
		}
	}
	
	// Parse input buffer, read hash(es)
	siResult = inParams.LoadDSLocalParamsForAuthMethod( inParams.uiAuthMethod, userLevelHashList, inGUIDString, inAuthedUserIsAdmin,
											inAuthData, outAuthData );
	if ( siResult != eDSNoErr )
		return( siResult );
	
	if ( inParams.bFetchHashFiles )
	{
		siResult = CDSLocalAuthHelper::ReadShadowHashAndStateFiles(
					inParams.pUserName,
					inGUIDString,
					inParams.hashes,
					&inParams.modDateOfPassword,
					&inParams.path,
					&inParams.stateFilePath,
					&inParams.state,
					&inParams.hashesLengthFromFile );
		
		if ( siResult != eDSNoErr )
		{
			// the password setting methods (except changepass) don't need
			// a pre-existing hash file.
			switch( inParams.uiAuthMethod )
			{
				case kAuthSetPasswd:
				case kAuthSetPasswdAsRoot:
				case kAuthWriteSecureHash:
				case kAuthMSLMCHAP2ChangePasswd:
				case kAuthNTSetWorkstationPasswd:
				case kAuthNTSetNTHash:
				case kAuthSetLMHash:
				case kAuthSetPolicyAsRoot:
				case kAuthSetShadowHashWindows:
				case kAuthSetShadowHashSecure:
				case kAuthSetComputerAcctPasswdAsRoot:
				case kAuthSetCertificateHashAsRoot:
					siResult = eDSNoErr;
					break;
				
				default:
					break;
			}
		}
	}
	
	if ( (siResult != eDSNoErr) && !(isSecondary && (inParams.uiAuthMethod == kAuthChangePasswd)) )
		return( siResult );
	
	if ( inParams.pUserName != NULL ) {
		bufferUserIsAdmin = dsIsUserMemberOfGroup( inParams.pUserName, "admin" );
	}
	
	// complete the operation
	switch( inParams.uiAuthMethod )
	{
		case kAuthGetGlobalPolicy:
			siResult = CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess,
							&inParams.globalMoreAccess );
			if ( siResult != eDSNoErr )
				return( siResult );
			
			{
				char policies[2048];
				
				PWGlobalAccessFeaturesToStringExtra( &inParams.globalAccess, &inParams.globalMoreAccess, sizeof(policies), policies );
				inParams.policyStrLen = strlen( policies );
				siResult = dsFillAuthBuffer( outAuthData, 1, inParams.policyStrLen, policies );
			}
			if ( outAuthData->fBufferLength == 0 )
				return( eDSEmptyAttribute );
			break;
		
		case kAuthSetGlobalPolicy:
			CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess, &inParams.globalMoreAccess );
			
			inParams.dataList = dsAuthBufferGetDataListAllocPriv( inAuthData );
			if ( inParams.dataList == NULL ) {
				siResult = eDSInvalidBuffFormat;
				goto finish;
			}
			// NOTE: using <pNewPassword> variable for the policy string
			inParams.pNewPassword = dsDataListGetNodeStringPriv(inParams.dataList, 3);
			if ( inParams.pNewPassword == NULL || strlen(inParams.pNewPassword) < 1 ) {
				siResult = eDSInvalidBuffFormat;
				goto finish;
			}
			
			StringToPWGlobalAccessFeaturesExtra( inParams.pNewPassword, &inParams.globalAccess, &inParams.globalMoreAccess );
			siResult = CDSLocalAuthHelper::SetShadowHashGlobalPolicies( inPlugin, inNode, inNodeRef,
				inAuthedUserName, inUID, inEffectiveUID, &inParams.globalAccess, &inParams.globalMoreAccess );
			break;
		
		case kAuthSetPolicy:
		case kAuthSetPolicyAsRoot:
			siResult = CDSLocalAuthHelper::SetUserPolicies( inMutableRecordDict, inPlugin, inNode, inAuthedUserName,
				inUID, inEffectiveUID, inNativeRecType, inParams.pUserName, inParams.pNewPassword, inAuthedUserIsAdmin, inNodeRef,
				&inParams.targetUserState );
			if ( siResult == eDSNoErr && inParams.targetUserStateFilePath != NULL )
				CDSLocalAuthHelper::WriteHashStateFile( inParams.targetUserStateFilePath, &inParams.targetUserState );
			break;
		
		case kAuthGetEffectivePolicy:
		case kAuthGetPolicy:
			// get policy attribute
			siResult = GetUserPolicies( inMutableRecordDict, &inParams.state, inPlugin, &inParams.policyStr );
			if ( siResult != 0 ) return( siResult );
			
			if ( inParams.policyStr != NULL )
			{
				inParams.policyStrLen = strlen( inParams.policyStr );
				siResult = dsFillAuthBuffer( outAuthData, 1, inParams.policyStrLen, inParams.policyStr );
			}
			break;
		
		case kAuthDIGEST_MD5:
			{
				char *mutualDigest = NULL;
				unsigned int mutualDigestLen;
				UInt32 passwordLength;
				
				siResult = CDSLocalAuthHelper::UnobfuscateRecoverablePassword( inParams.hashes + kHashOffsetToRecoverable,
					(unsigned char **)&inParams.pOldPassword, &passwordLength );
				if ( siResult == eDSNoErr )
				{
					siResult = SASLErrToDirServiceError( 
									digest_verify(
										&inParams.digestContext, inParams.pOldPassword, passwordLength,
										&mutualDigest, &mutualDigestLen ) );
				}
				if ( siResult == eDSNoErr && mutualDigest != NULL )
				{
					// pack the return digest
					siResult = dsFillAuthBuffer( outAuthData, 1, mutualDigestLen, mutualDigest );
				}
				DSFree( mutualDigest );
			}
			break;
		
		case kAuthCRAM_MD5:
			siResult = CDSLocalAuthHelper::CRAM_MD5( inParams.hashes + kHashOffsetToCramMD5, inParams.challenge, inParams.pCramResponse );
			break;
		
		case kAuthAPOP:
			{
				UInt32 passwordLength;

				siResult = CDSLocalAuthHelper::UnobfuscateRecoverablePassword( inParams.hashes + kHashOffsetToRecoverable,
					(unsigned char **)&inParams.pOldPassword, &passwordLength );
				if ( siResult == eDSNoErr )
					siResult = CDSLocalAuthHelper::Verify_APOP( inParams.pUserName, (unsigned char *)inParams.pOldPassword,
									passwordLength, inParams.challenge, inParams.apopResponse );
			}
			break;
		
		case kAuthSMB_NT_Key:
			memmove(inParams.P21, inParams.hashes, kHashShadowOneLength);
			CalculateP24(inParams.P21, inParams.C8, inParams.P24);
			siResult = (memcmp(inParams.P24, inParams.P24Input, kHashShadowResponseLength) == 0) ? eDSNoErr : eDSAuthFailed;
			break;
			
		case kAuthSMB_LM_Key:
			memmove(inParams.P21, inParams.hashes + kHashOffsetToLM, kHashShadowOneLength);
			CalculateP24(inParams.P21, inParams.C8, inParams.P24);
			siResult = (memcmp(inParams.P24, inParams.P24Input, kHashShadowResponseLength) == 0) ? eDSNoErr : eDSAuthFailed;
			break;
		
		case kAuthNTLMv2:
			if ( NTLMv2(inParams.GeneratedNTLM, inParams.hashes, inParams.pSambaName, inParams.pDomain, inParams.C8,
						inParams.pNTLMDigest + kShadowHashNTLMv2Length, inParams.ntlmDigestLen - kShadowHashNTLMv2Length) == 0 )
			{
				if ( memcmp(inParams.GeneratedNTLM, inParams.pNTLMDigest, kShadowHashNTLMv2Length) == 0 )
				{
					siResult = eDSNoErr;
				}
				else
				{
					siResult = eDSAuthFailed;
				}
			}
			break;
		
		case kAuthMSCHAP2:
			siResult = CDSLocalAuthHelper::MSCHAPv2( inParams.C16, inParams.PeerC16, inParams.pNTLMDigest, inParams.pSambaName,
							inParams.hashes, inParams.MSCHAP2Response );
			if ( siResult == eDSNoErr )
			{
				// put the response in the out buffer
				outAuthData->fBufferLength = 4 + MS_AUTH_RESPONSE_LENGTH;
				inParams.ntlmDigestLen = MS_AUTH_RESPONSE_LENGTH;
				memcpy( outAuthData->fBufferData, &inParams.ntlmDigestLen, 4 );
				memcpy( outAuthData->fBufferData + 4, inParams.MSCHAP2Response, MS_AUTH_RESPONSE_LENGTH );
			}
			break;
		
		case kAuthPPS:
			if ( *inOutContinueData == 0 )
			{
				// first pass
				CFMutableDictionaryRef	continueData = CFDictionaryCreateMutable( NULL, 0,
																				  &kCFTypeDictionaryKeyCallBacks,
																				  &kCFTypeDictionaryValueCallBacks );
				if ( continueData == NULL )
					throw( eMemoryError );
				
				ppsServerContext = (ServerAuthDataBlockPtr)calloc( 1, sizeof(ServerAuthDataBlock) );
				server_step_0_set_hash( inParams.hashes + kHashOffsetToSaltedSHA1, ppsServerContext );

				CFDataRef ppsContext = CFDataCreate( NULL, (UInt8 *)&ppsServerContext, sizeof(ServerAuthDataBlockPtr) );
				CFDictionarySetValue( continueData, CFSTR("ppsContext"), ppsContext );
				CFRelease( ppsContext );
				
				inPlugin->AddContinueData( inNodeRef, continueData, inOutContinueData );
			}
			else
			{
				CFMutableDictionaryRef cfContinueDict = (CFMutableDictionaryRef) gLocalContinueTable->GetPointer( *inOutContinueData );
				if ( cfContinueDict != NULL && CFGetTypeID(cfContinueDict) == CFDictionaryGetTypeID() )
				{
					CFDataRef ppsContext = (CFDataRef)CFDictionaryGetValue( cfContinueDict, CFSTR("ppsContext") );
					if ( ppsContext != NULL )
					{
						ServerAuthDataBlockPtr *ctxPtr = (ServerAuthDataBlockPtr *)CFDataGetBytePtr( ppsContext );
						if ( ctxPtr != NULL )
							ppsServerContext = *ctxPtr;
					}
					
					if ( ppsServerContext == NULL )
						siResult = eDSAuthContinueDataBad;
				}
				else
				{
					siResult = eDSAuthContinueDataBad;
				}
			}
			
			if ( siResult == eDSNoErr )
			{
				saslError = pps_server_mech_step(
								ppsServerContext,
								inParams.challenge, strlen(inParams.challenge),
								&serverout, &serveroutlen );
				siResult = SASLErrToDirServiceError( saslError );
			}
			
			if ( siResult == eDSNoErr )
				siResult = dsFillAuthBuffer( outAuthData, 1, serveroutlen, serverout );
			
			if ( siResult != eDSNoErr || saslError == SASL_OK ) {
				pps_server_mech_dispose( ppsServerContext );
				if ( *inOutContinueData != 0 ) {
					gLocalContinueTable->RemoveContext( *inOutContinueData );
					*inOutContinueData = 0;
				}
			}
			break;
			
		case kAuthVPN_PPTPMasterKeys:
			if ( inEffectiveUID == 0 )
			{
				unsigned char sendKey[CC_SHA1_DIGEST_LENGTH];
				unsigned char receiveKey[CC_SHA1_DIGEST_LENGTH];
				
				CalculatePPTPSessionKeys( inParams.hashes, inParams.P24Input, inParams.keySize, sendKey, receiveKey );
				inParams.ntlmDigestLen = inParams.keySize;
				memcpy( outAuthData->fBufferData, &inParams.ntlmDigestLen, 4 );
				memcpy( outAuthData->fBufferData + 4, sendKey, inParams.ntlmDigestLen );
				memcpy( outAuthData->fBufferData + 4 + inParams.ntlmDigestLen, &inParams.ntlmDigestLen, 4 );
				memcpy( outAuthData->fBufferData + 4 + inParams.ntlmDigestLen + 4, receiveKey, inParams.ntlmDigestLen );
				outAuthData->fBufferLength = 8 + inParams.keySize*2;
			}
			else
			{
				siResult = eDSPermissionError;
			}
			break;
		
		case kAuthSMBWorkstationCredentialSessionKey:
			if ( inParams.ntlmHashType == 1 )
			{
				CalculateWorkstationCredentialStrongSessKey( inParams.hashes, (char *)inParams.pNTLMDigest, (char *)inParams.pNTLMDigest + 8, inParams.P24 );
				siResult = dsFillAuthBuffer( outAuthData, 1, 16, inParams.P24 );
			}
			else
			{
				CalculateWorkstationCredentialSessKey( inParams.hashes, (char *)inParams.pNTLMDigest, (char *)inParams.pNTLMDigest + 8,
					inParams.P24 );
				siResult = dsFillAuthBuffer( outAuthData, 1, 8, inParams.P24 );
			}
			break;
			
		case kAuthSecureHash:
			if ( inParams.secureHashNode->fBufferLength == kHashSaltedSHA1Length )
			{
				siResult = (memcmp(inParams.secureHash, inParams.hashes + kHashOffsetToSaltedSHA1,
					kHashSaltedSHA1Length) == 0) ? eDSNoErr : eDSAuthFailed;
			}
			else
			{
				siResult = eDSAuthFailed;
			}
			break;
		
		case kAuthWriteSecureHash:
			// allow root to directly overwrite the secure hash
			if ( inEffectiveUID != 0 )
			{
				siResult = eDSPermissionError;
				goto finish;
			}
			//write this secure hash with the other hashes as empty
			//we will reconstruct the other hashes upon first successful auth
			memmove(inParams.generatedHashes + kHashOffsetToSaltedSHA1, inParams.secureHash, kHashSaltedSHA1Length);
			siResult = CDSLocalAuthHelper::WriteShadowHash(inParams.pUserName, inGUIDString, inParams.generatedHashes);
			break;

		case kAuthReadSecureHash:
			// allow root to directly read the secure hash
			if ( inEffectiveUID != 0 )
			{
				siResult = eDSPermissionError;
				goto finish;
			}
			//read file
			siResult = CDSLocalAuthHelper::ReadShadowHashAndStateFiles(inParams.pUserName, inGUIDString, inParams.hashes,
							&inParams.modDateOfPassword, &inParams.path, &inParams.stateFilePath, &inParams.state,
							&inParams.hashesLengthFromFile);
			if ( siResult == eDSNoErr )
			{
				UInt32 sha1hashLen = kHashSaltedSHA1Length;
				unsigned char *sha1hashPtr = inParams.hashes + kHashOffsetToSaltedSHA1;
				if ( inParams.hashesLengthFromFile < (kHashOffsetToSaltedSHA1 + kHashSaltedSHA1Length) ||
					 memcmp(sha1hashPtr, sZeros, kHashSaltedSHA1Length ) == 0 ) {
					sha1hashLen = kHashSecureLength;
					sha1hashPtr = inParams.hashes + kHashOffsetToSHA1;
				}
				
				siResult = dsFillAuthBuffer( outAuthData, 1, sha1hashLen, sha1hashPtr );
				if ( siResult != eDSNoErr )
					goto finish;
			}
			break;
		
		case kAuthSetShadowHashWindows:
		case kAuthSetShadowHashSecure:
		case kAuthNativeClearTextOK:
		case kAuthNativeNoClearText:
		case kAuthSetPasswdCheckAdmin:
		case kAuthNativeRetainCredential:
		{
			UInt32 pwdLen = 0;
			
			siResult = GetUserPolicies( inMutableRecordDict, NULL, inPlugin, &inParams.policyStr );
			if ( siResult != eDSNoErr )
				goto finish;
			
			if (inParams.pOldPassword != nil)
			{
				pwdLen = strlen(inParams.pOldPassword);
			}
			else
			{
				siResult = eDSAuthFailed;
				goto finish;
			}
			if ( pwdLen >= kHashRecoverableLength ) {
				siResult = eDSAuthPasswordTooLong;
				goto finish;
			}
			if (inParams.hashesLengthFromFile == (kHashShadowBothLength + kHashSecureLength))
			{
				//legacy length so compare legacy hashes
				//will rewrite upgraded hashes below
				CDSLocalAuthHelper::GenerateShadowHashes( gServerOS,
										inParams.pOldPassword,
										strlen(inParams.pOldPassword),
										ePluginHashSHA1 | ePluginHashNT | ePluginHashLM,
										inParams.hashes + kHashOffsetToSaltedSHA1,
										inParams.generatedHashes,
										&inParams.hashLength );
			}
			else
			{
				//generate proper hashes according to policy
				CDSLocalAuthHelper::GenerateShadowHashes( gServerOS,
										inParams.pOldPassword,
										strlen(inParams.pOldPassword),
										userLevelHashList,
										inParams.hashes + kHashOffsetToSaltedSHA1,
										inParams.generatedHashes,
										&inParams.hashLength );
			}
			
			if ( HashesEqual( inParams.hashes, inParams.generatedHashes ) )
			{
				siResult = eDSNoErr;
				// update old hash file formats
				// 1. If the shadowhash file is short, save all the proper hashes.
				// 2. If the hash list is out-of-date, update.
				if ( inParams.hashesLengthFromFile < kHashTotalLength)
				{
					//generate proper hashes according to policy
					CDSLocalAuthHelper::GenerateShadowHashes( gServerOS,
											inParams.pOldPassword,
											strlen(inParams.pOldPassword),
											userLevelHashList,
											inParams.hashes + kHashOffsetToSaltedSHA1,
											inParams.generatedHashes,
											&inParams.hashLength );
					// sync up the hashes
					siResult = CDSLocalAuthHelper::WriteShadowHash( inParams.pUserName, inGUIDString, inParams.generatedHashes );
				}
				else if ( memcmp(inParams.hashes, inParams.generatedHashes, kHashTotalLength) != 0 )
				{
					// sync up the hashes
					siResult = CDSLocalAuthHelper::WriteShadowHash( inParams.pUserName, inGUIDString, inParams.generatedHashes );
				}
				
				MigrateAddKerberos( inNodeRef, inParams, inOutContinueData, inAuthData, outAuthData, inAuthOnly, isSecondary,
					inAuthAuthorityList, inGUIDString, inAuthedUserIsAdmin, inMutableRecordDict, inHashList, inPlugin, inNode,
					inAuthedUserName, inUID, inEffectiveUID, inNativeRecType, inOKToChangeAuthAuthorities );
				
				//see if we need to set the shadowhash tags
				if ( inParams.uiAuthMethod == kAuthSetShadowHashWindows ||
					 inParams.uiAuthMethod == kAuthSetShadowHashSecure )
				{
					//set the auth authority
					pwsf_ShadowHashDataToArray( inParams.aaData, &myHashTypeArray );
					if ( myHashTypeArray == NULL )
						myHashTypeArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
					
					if (userLevelHashList & ePluginHashCRAM_MD5)
					{
						CFStringRef cram_md5_cfString = CFSTR( kHashNameCRAM_MD5 );
						CFIndex locTag = CFArrayGetFirstIndexOfValue(myHashTypeArray,
							CFRangeMake(0,CFArrayGetCount(myHashTypeArray)), cram_md5_cfString);
						if (locTag == kCFNotFound)
						{
							CFArrayAppendValue(myHashTypeArray, cram_md5_cfString);
							needToChange = true;
						}
					}
					if (userLevelHashList & ePluginHashRecoverable)
					{
						CFStringRef recoverable_cfString = CFSTR( kHashNameRecoverable );
						CFIndex locTag = CFArrayGetFirstIndexOfValue(myHashTypeArray,
							CFRangeMake(0,CFArrayGetCount(myHashTypeArray)), recoverable_cfString);
						if (locTag == kCFNotFound)
						{
							CFArrayAppendValue(myHashTypeArray, recoverable_cfString);
							needToChange = true;
						}
					}
					if (userLevelHashList & ePluginHashSecurityTeamFavorite)
					{
						CFStringRef team_fav_cfString = CFSTR( kHashNameSecure );
						CFIndex locTag = CFArrayGetFirstIndexOfValue(myHashTypeArray,
							CFRangeMake(0,CFArrayGetCount(myHashTypeArray)), team_fav_cfString);
						if (locTag == kCFNotFound)
						{
							CFArrayAppendValue(myHashTypeArray, team_fav_cfString);
							needToChange = true;
						}
					}
					CFStringRef salted_sha1_cfString = CFSTR( kHashNameSHA1 );
					CFIndex locTag = CFArrayGetFirstIndexOfValue(myHashTypeArray,
						CFRangeMake(0,CFArrayGetCount(myHashTypeArray)), salted_sha1_cfString);
					if (locTag == kCFNotFound)
					{
						CFArrayAppendValue(myHashTypeArray, salted_sha1_cfString);
						needToChange = true;
					}
				
					// Set the NT hash on or off, the LM hash is always off for Leopard and later
					if ( inParams.uiAuthMethod == kAuthSetShadowHashWindows )
					{
						CFStringRef smb_nt_cfString = CFSTR( kHashNameNT );
						locTag = CFArrayGetFirstIndexOfValue(myHashTypeArray,
							CFRangeMake(0,CFArrayGetCount(myHashTypeArray)), smb_nt_cfString);
						if (locTag == kCFNotFound)
						{
							CFArrayAppendValue(myHashTypeArray, smb_nt_cfString);
							needToChange = true;
						}
						CFStringRef smb_lm_cfString = CFSTR( kHashNameLM );
						locTag = CFArrayGetFirstIndexOfValue(myHashTypeArray,
							CFRangeMake(0,CFArrayGetCount(myHashTypeArray)), smb_lm_cfString);
						if (locTag != kCFNotFound)
						{
							CFArrayRemoveValueAtIndex(myHashTypeArray, locTag);
							needToChange = true;
						}
					}
					else
					if ( inParams.uiAuthMethod == kAuthSetShadowHashSecure )
					{
						CFStringRef smb_nt_cfString = CFSTR( kHashNameNT );
						locTag = CFArrayGetFirstIndexOfValue(myHashTypeArray,
							CFRangeMake(0,CFArrayGetCount(myHashTypeArray)), smb_nt_cfString);
						if (locTag != kCFNotFound)
						{
							CFArrayRemoveValueAtIndex(myHashTypeArray, locTag);
							needToChange = true;
						}
						CFStringRef smb_lm_cfString = CFSTR( kHashNameLM );
						locTag = CFArrayGetFirstIndexOfValue(myHashTypeArray,
							CFRangeMake(0,CFArrayGetCount(myHashTypeArray)), smb_lm_cfString);
						if (locTag != kCFNotFound)
						{
							CFArrayRemoveValueAtIndex(myHashTypeArray, locTag);
							needToChange = true;
						}
					}
					
					if ( needToChange )
					{
						long convertResult = 0;
						char *newAuthAuthority = pwsf_ShadowHashArrayToData( myHashTypeArray, &convertResult );
						if ( newAuthAuthority != NULL )
						{
							char * fullAuthAuthority = (char *) calloc(1,
								1 + strlen(kDSValueAuthAuthorityShadowHash) + strlen(newAuthAuthority));
							strcpy(fullAuthAuthority, kDSValueAuthAuthorityShadowHash);
							strcat(fullAuthAuthority, newAuthAuthority);
							if ( inOKToChangeAuthAuthorities ) {
								siResult = ::SetUserAuthAuthorityAsRoot( inMutableRecordDict, inPlugin,
											inNode, CFSTR( kDSStdRecordTypeUsers ), inParams.pUserName, fullAuthAuthority,
											inNodeRef, false );
							}
							
							//get the hashlist anew since we have written a new auth authority
							// If a user hash-list is specified, then it takes precedence over the global list
							if ( siResult == eDSNoErr )
							{
								siResult = CDSLocalAuthHelper::GetHashSecurityLevelForUser( newAuthAuthority, &userLevelHashList );
								if ( siResult != eDSNoErr )
								{
									DbgLog(  kLogPlugin, "CDSLocalAuthHelper::DoShadowHashAuth(): got invalid rec hash list: %s",
										fullAuthAuthority );
									userLevelHashList = inHashList;
									siResult = eDSNoErr;
								}
								
								CDSLocalAuthHelper::GenerateShadowHashes( gServerOS,
														inParams.pOldPassword,
														strlen(inParams.pOldPassword),
														userLevelHashList,
														inParams.hashes + kHashOffsetToSaltedSHA1,
														inParams.generatedHashes,
														&inParams.hashLength );
								// sync up the hashes
								siResult = CDSLocalAuthHelper::WriteShadowHash( inParams.pUserName, inGUIDString, inParams.generatedHashes );
							}
	
							DSFreeString( newAuthAuthority );
							DSFreeString( fullAuthAuthority );
						}
					}
				}
				else
				{
					if ( inAuthOnly == false )
					{
						siResult = inPlugin->AuthOpen( inNodeRef, inParams.pUserName, inParams.pOldPassword,
													   dsIsUserMemberOfGroup(inParams.pUserName, "admin") );
					}
				}
			}
			else
			{
				siResult = eDSAuthFailed;
			}
			break;
		}
		// set password operations
		case kAuthSetPasswd:
			{
				siResult = eDSAuthFailed;
				if ( inParams.pAdminUser != NULL )
				{
					UInt32 pwdLen = 0;
				
					adminString = CFStringCreateWithCString( kCFAllocatorDefault, inParams.pAdminUser, kCFStringEncodingUTF8 );
					if ( adminString == NULL ) {
						siResult = eMemoryError;
						goto finish;
					}
					
					bool accessAllowed = inNode->AccessAllowed( adminString, inEffectiveUID, inNativeRecType, 
															    inPlugin->AttrNativeTypeForStandardType(CFSTR(kDS1AttrPassword)), 
															    eDSAccessModeWriteAttr );
					if ( accessAllowed == false ) {
						siResult = eDSPermissionError;
						goto finish;
					}
					
					if ( inParams.pAdminPassword != NULL )
					{
						pwdLen = strlen( inParams.pAdminPassword );
					}
					else
					{
						siResult = eDSAuthFailed;
						goto finish;
					}
					
					if ( pwdLen >= kHashRecoverableLength ) {
						siResult = eDSAuthPasswordTooLong;
						goto finish;
					}
					
					bool haveCStr = false;
					char guidCStr[128];
					CFStringRef guidString = inPlugin->GetUserGUID( adminString, inNode );
					if ( guidString != NULL ) {
						haveCStr = CFStringGetCString( guidString, guidCStr, sizeof(guidCStr), kCFStringEncodingUTF8 );
						CFRelease( guidString );
					}
					
					char *adminPath = NULL;
					char *adminStateFilePath = NULL;
					struct timespec adminModDateOfPassword;
					sHashState adminState;
					unsigned char adminHashes[kHashTotalLength];
					
					siResult = CDSLocalAuthHelper::ReadShadowHashAndStateFiles(
						inParams.pAdminUser,
						haveCStr ? guidCStr : NULL,
						adminHashes,
						&adminModDateOfPassword,
						&adminPath,
						&adminStateFilePath,
						&adminState,
						&inParams.hashesLengthFromFile );
					
					DSFreeString( adminPath );
					DSFreeString( adminStateFilePath );
					
					if ( inParams.hashesLengthFromFile == (kHashShadowBothLength + kHashSecureLength) )
					{
						//legacy length so compare legacy hashes
						//will rewrite upgraded hashes below
						CDSLocalAuthHelper::GenerateShadowHashes( gServerOS,
												inParams.pAdminPassword,
												pwdLen,
												ePluginHashSHA1 | ePluginHashNT | ePluginHashLM,
												inParams.hashes + kHashOffsetToSaltedSHA1,
												inParams.generatedHashes,
												&inParams.hashLength );
					}
					else
					{
						// generate proper hashes according to policy
						CDSLocalAuthHelper::GenerateShadowHashes( gServerOS,
												inParams.pAdminPassword,
												pwdLen,
												userLevelHashList,
												inParams.hashes + kHashOffsetToSaltedSHA1,
												inParams.generatedHashes,
												&inParams.hashLength );
					}
					
					if ( !HashesEqual( adminHashes, inParams.generatedHashes ) ) {
						siResult = eDSAuthFailed;
						goto finish;
					}
				}
				else
				{
					bool accessAllowed = inNode->AccessAllowed( inAuthedUserName, inEffectiveUID, inNativeRecType, 
															    inPlugin->AttrNativeTypeForStandardType(CFSTR(kDS1AttrPassword)), 
															    eDSAccessModeWriteAttr );
					if ( accessAllowed == false ) {
						siResult = eDSPermissionError;
						goto finish;
					}
				}
				
				if ( siResult == eDSNoErr )
				{
					siResult = GetUserPolicies( inMutableRecordDict, NULL, inPlugin, &inParams.policyStr );
					if ( siResult != eDSNoErr )
						goto finish;
					
					CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess, &inParams.globalMoreAccess );
					
					// non-admins can only change their own passwords
					bool authFailed = false;
					if ( adminString == NULL || inParams.pUserName == NULL )
					{
						authFailed = true;
					}
					else
					{
						CFStringRef userNameCFStr = CFStringCreateWithCString( NULL, inParams.pUserName, kCFStringEncodingUTF8 );
						if ( CFStringCompare( userNameCFStr, adminString, 0 ) != kCFCompareEqualTo )
							authFailed = true;
						
						CFRelease( userNameCFStr );
					}
					
					if ( authFailed )
					{
						siResult = eDSPermissionError;
					}
					else
					{
						siResult = CDSLocalAuthHelper::PasswordOkForPolicies( inParams.policyStr, &inParams.globalAccess,
										inParams.pUserName, inParams.pNewPassword );
						if ( siResult == eDSAuthPasswordTooShort &&
							 inParams.globalAccess.minChars == 0 &&
							 inParams.pNewPassword != NULL &&
							 *inParams.pNewPassword == '\0' &&
							 ((inParams.policyStr == NULL) || strstr(inParams.policyStr, "minChars=0") != NULL) )
						{
							// special-case for ShadowHash and blank password.
							siResult = eDSNoErr;
						}
					}
				}
				
				if ( siResult == eDSNoErr )
				{
					AddKerberosAuthAuthority(
						inNodeRef, inParams.pUserName, kDSTagAuthAuthorityKerberosv5, inAuthAuthorityList, inMutableRecordDict,
						inPlugin, inNode, inNativeRecType, inOKToChangeAuthAuthorities );
					
					bzero( inParams.generatedHashes, kHashTotalLength );
					CDSLocalAuthHelper::GenerateShadowHashes( gServerOS, inParams.pNewPassword, strlen(inParams.pNewPassword),
							userLevelHashList, NULL, inParams.generatedHashes, &inParams.hashLength );
					
					siResult = CDSLocalAuthHelper::WriteShadowHash( inParams.pUserName, inGUIDString, inParams.generatedHashes );
					if ( siResult == eDSNoErr )
					{
						inParams.state.newPasswordRequired = 0;
						time( &now );
						gmtime_r( &now, &inParams.state.modDateOfPassword );
						gettimeofday( &inParams.modDateAssist, NULL );
						TIMEVAL_TO_TIMESPEC( &inParams.modDateAssist, &inParams.modDateOfPassword );
					
						// update Kerberos
						siResult = CDSLocalAuthHelper::DoKerberosAuth( inNodeRef, inParams, inOutContinueData, inAuthData,
										outAuthData, inAuthOnly, isSecondary, inAuthAuthorityList, inGUIDString, inAuthedUserIsAdmin,
										inMutableRecordDict, inHashList, inPlugin, inNode, inAuthedUserName, inUID, inEffectiveUID,
										inNativeRecType );
					}
				}
			}
			break;
		
		case kAuthSetPasswdAsRoot:
			{
				siResult = eDSAuthFailed;
				if ( inAuthedUserName != NULL && inParams.pUserName != NULL )
				{
					CFStringRef userNameCFStr = CFStringCreateWithCString( NULL, inParams.pUserName, kCFStringEncodingUTF8 );
					if ( CFStringCompare(userNameCFStr, inAuthedUserName, 0) == kCFCompareEqualTo ) {
						siResult = eDSNoErr;
					}
					CFRelease( userNameCFStr );
				}
				
				if ( siResult != eDSNoErr ) {
					bool accessAllowed = inNode->AccessAllowed( inAuthedUserName, inEffectiveUID, inNativeRecType, 
															    inPlugin->AttrNativeTypeForStandardType(CFSTR(kDS1AttrPassword)), 
															    eDSAccessModeWriteAttr );
					if ( accessAllowed == true ) {
						siResult = eDSNoErr;
					}
					else {
						siResult = eDSPermissionError;
					}
				}
				
				if ( siResult == eDSNoErr )
				{
					CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess,
						&inParams.globalMoreAccess );
					
					// admins are allowed to set passwords not governed by policy
					// TODO: why is admin allowed to bypass password policies when setting a password
					if ( inAuthedUserIsAdmin == false )
					{
						siResult = CDSLocalAuthHelper::PasswordOkForPolicies( inParams.policyStr, &inParams.globalAccess,
										inParams.pUserName, inParams.pNewPassword );
						if ( siResult == eDSAuthPasswordTooShort &&
							 inParams.globalAccess.minChars == 0 &&
							 inParams.pNewPassword != NULL &&
							 *inParams.pNewPassword == '\0' &&
							 ((inParams.policyStr == NULL) || strstr(inParams.policyStr, "minChars=0") != NULL) )
						{
							// special-case for ShadowHash and blank password.
							siResult = eDSNoErr;
						}
					}
				}
				
				if ( siResult == eDSNoErr )
				{
					AddKerberosAuthAuthority(
						inNodeRef, inParams.pUserName, kDSTagAuthAuthorityKerberosv5, inAuthAuthorityList, inMutableRecordDict,
						inPlugin, inNode, inNativeRecType, inOKToChangeAuthAuthorities );
					
					CDSLocalAuthHelper::GenerateShadowHashes( gServerOS, inParams.pNewPassword, strlen(inParams.pNewPassword),
						userLevelHashList, NULL, inParams.generatedHashes, &inParams.hashLength );
			
					siResult = CDSLocalAuthHelper::WriteShadowHash(inParams.pUserName, inGUIDString, inParams.generatedHashes);
					if ( siResult == eDSNoErr )
					{
						inParams.state.newPasswordRequired = 0;
						time( &now );
						gmtime_r( &now, &inParams.state.modDateOfPassword );
						gettimeofday( &inParams.modDateAssist, NULL );
						TIMEVAL_TO_TIMESPEC( &inParams.modDateAssist, &inParams.modDateOfPassword );
					
						// update Kerberos
						siResult = CDSLocalAuthHelper::DoKerberosAuth( inNodeRef, inParams, inOutContinueData, inAuthData,
										outAuthData, inAuthOnly, isSecondary, inAuthAuthorityList, inGUIDString, inAuthedUserIsAdmin,
										inMutableRecordDict, inHashList, inPlugin, inNode, inAuthedUserName, inUID, inEffectiveUID,
										inNativeRecType );
					}
				}
			}
			break;

		case kAuthChangePasswd:
			siResult = GetUserPolicies( inMutableRecordDict, NULL, inPlugin, &inParams.policyStr );
			if ( siResult != eDSNoErr )
				goto finish;
			
			CDSLocalAuthHelper::GenerateShadowHashes( gServerOS,
								inParams.pOldPassword,
								strlen(inParams.pOldPassword),
								userLevelHashList,
								inParams.hashes + kHashOffsetToSaltedSHA1,
								inParams.generatedHashes,
								&inParams.hashLength );
			
			if ( HashesEqual( inParams.hashes, inParams.generatedHashes ) || isSecondary )
			{
				CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess,
					&inParams.globalMoreAccess );
				if ( !bufferUserIsAdmin )
				{
					siResult = CDSLocalAuthHelper::PasswordOkForPolicies( inParams.policyStr, &inParams.globalAccess,
									inParams.pUserName, inParams.pNewPassword );
					if ( siResult == eDSAuthPasswordTooShort &&
						 inParams.globalAccess.minChars == 0 &&
						 inParams.pNewPassword != NULL &&
						 *inParams.pNewPassword == '\0' &&
						 ((inParams.policyStr == NULL) || strstr(inParams.policyStr, "minChars=0") != NULL) )
					{
						// special-case for ShadowHash and blank password.
						siResult = eDSNoErr;
					}
				}
				if ( siResult == eDSNoErr )
				{
					bzero(inParams.generatedHashes, kHashTotalLength);
					CDSLocalAuthHelper::GenerateShadowHashes( gServerOS, inParams.pNewPassword, strlen(inParams.pNewPassword),
						userLevelHashList, NULL, inParams.generatedHashes, &inParams.hashLength );
					
					siResult = CDSLocalAuthHelper::WriteShadowHash(inParams.pUserName, inGUIDString, inParams.generatedHashes);
					inParams.state.newPasswordRequired = 0;
					
					// password changed
					time( &now );
					gmtime_r( &now, &inParams.state.modDateOfPassword );
					gettimeofday( &inParams.modDateAssist, NULL );
					TIMEVAL_TO_TIMESPEC( &inParams.modDateAssist, &inParams.modDateOfPassword );
				}
			}
			else
			{
				siResult = eDSAuthFailed;
			}
			break;
		
		case kAuthSetLMHash:
			if ( inEffectiveUID != 0 && !inAuthedUserIsAdmin ) {
				siResult = eDSPermissionError;
				goto finish;
			}				
			memcpy( inParams.hashes + kHashOffsetToLM, inParams.pNTLMDigest, kHashShadowOneLength );
			siResult = WriteShadowHash( inParams.pUserName, inGUIDString, inParams.hashes );
			break;
		
		case kAuthNTSetNTHash:
		case kAuthNTSetWorkstationPasswd:
		case kAuthSMB_NTUserSessionKey:
			if ( inEffectiveUID != 0 && !inAuthedUserIsAdmin ) {
				siResult = eDSPermissionError;
				goto finish;
			}				
			memcpy( inParams.hashes + kHashOffsetToNT, inParams.pNTLMDigest, kHashShadowOneLength );
			siResult = WriteShadowHash( inParams.pUserName, inGUIDString, inParams.hashes );
			break;
		
		case kAuthMSLMCHAP2ChangePasswd:
            {
                bool notAdmin = ( (inParams.pUserName != NULL) && !inAuthedUserIsAdmin );
                
                siResult = GetUserPolicies( inMutableRecordDict, NULL, inPlugin, &inParams.policyStr );
                if ( siResult != eDSNoErr )
                    goto finish;
                
                // Note: the character encoding is in <pOldPassword>
                siResult = MSCHAPv2ChangePass( inParams.pUserName, inParams.hashes + kHashOffsetToLM, inParams.pOldPassword,
                                              inParams.pNTLMDigest, inParams.ntlmDigestLen, (uint8_t **)&inParams.pNewPassword, &len );
                if ( siResult != eDSNoErr )
                    goto finish;
                
                if ( notAdmin )
                {
                    siResult = CDSLocalAuthHelper::PasswordOkForPolicies( inParams.policyStr, &inParams.globalAccess, inParams.pUserName,
                                                                         inParams.pNewPassword );
                    if ( siResult == eDSAuthPasswordTooShort &&
                        inParams.globalAccess.minChars == 0 &&
                        inParams.pNewPassword != NULL &&
                        *inParams.pNewPassword == '\0' &&
                        ((inParams.policyStr == NULL) || strstr(inParams.policyStr, "minChars=0") != NULL) )
                    {
                        // special-case for ShadowHash and blank password.
                        siResult = eDSNoErr;
                    }
                    if ( siResult != eDSNoErr )
                        goto finish;
                }
                
                CDSLocalAuthHelper::GenerateShadowHashes( gServerOS, inParams.pNewPassword, len, userLevelHashList, NULL,
                                                         inParams.generatedHashes, &inParams.hashLength );
                siResult = CDSLocalAuthHelper::WriteShadowHash(inParams.pUserName, inGUIDString, inParams.generatedHashes);
                if ( siResult != eDSNoErr )
                    goto finish;
                
                // password changed
                time( &now );
                gmtime_r( &now, &inParams.state.modDateOfPassword );
                gettimeofday( &inParams.modDateAssist, NULL );
                TIMEVAL_TO_TIMESPEC( &inParams.modDateAssist, &inParams.modDateOfPassword );
            }
			break;
		
		case kAuthSetCertificateHashAsRoot:
			AddKerberosAuthAuthority(
					inNodeRef, inParams.pUserName, kDSTagAuthAuthorityKerberosv5Cert, inAuthAuthorityList, inMutableRecordDict,
					inPlugin, inNode, inNativeRecType, inOKToChangeAuthAuthorities );
			
			siResult = CDSLocalAuthHelper::DoKerberosCertAuth(
												inNodeRef, inParams, inOutContinueData, inAuthData, outAuthData, inAuthOnly,
												isSecondary, inAuthAuthorityList, inGUIDString, inAuthedUserIsAdmin,
												inMutableRecordDict, inHashList, inPlugin, inNode, inAuthedUserName, inUID,
												inEffectiveUID, inNativeRecType );
			break;
		
		default:
			siResult = eDSAuthMethodNotSupported;
			break;
	}

	// do not enforce login-time policies for administrators
	if ( !bufferUserIsAdmin )
	{
		// test policies and update the current user's state
		if ( siResult == eDSNoErr &&
			inParams.uiAuthMethod != kAuthGetEffectivePolicy &&
			inParams.uiAuthMethod != kAuthGetPolicy && inParams.uiAuthMethod != kAuthGetGlobalPolicy &&
			inParams.uiAuthMethod != kAuthSetPolicy && inParams.uiAuthMethod != kAuthSetGlobalPolicy )
		{
			CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess, &inParams.globalMoreAccess );
			siResult = CDSLocalAuthHelper::TestPolicies( inParams.policyStr, &inParams.globalAccess, &inParams.state,
							&inParams.modDateOfPassword, inParams.path );

			if ( siResult == eDSNoErr )
			{
				inParams.state.failedLoginAttempts = 0;
			}
			else
			if ( inParams.state.disabled == 1 )
			{
				siResult = eDSAuthAccountDisabled;
				if ( inOKToChangeAuthAuthorities )
					CDSLocalAuthHelper::SetUserAAtoDisabled( inMutableRecordDict, inPlugin, inNode, inNativeRecType,
						inParams.pUserName, inNodeRef, CFSTR("root"), inEffectiveUID );
			}
		}
		else
		if ( siResult == eDSAuthFailed )
		{
			inParams.state.failedLoginAttempts++;
			
			CDSLocalAuthHelper::GetShadowHashGlobalPolicies( inPlugin, inNode, &inParams.globalAccess, &inParams.globalMoreAccess );
			CDSLocalAuthHelper::TestPolicies( inParams.policyStr, &inParams.globalAccess, &inParams.state,
					&inParams.modDateOfPassword, inParams.path );
			if ( inParams.state.disabled == 1 )
			{
				siResult = eDSAuthAccountDisabled;
				if ( inOKToChangeAuthAuthorities )
					CDSLocalAuthHelper::SetUserAAtoDisabled( inMutableRecordDict, inPlugin, inNode, inNativeRecType,
						inParams.pUserName, inNodeRef, CFSTR("root"), inEffectiveUID );
			}
		}
	}
	if ( siResult == eDSNoErr || siResult == eDSAuthNewPasswordRequired )
	{
		time( &now );
		gmtime_r( &now, &(inParams.state.lastLoginDate) );
	}
		
	// check for failure in the operation and delay response based on heuristic of number of attempts
	switch( inParams.uiAuthMethod )
	{
		case kAuthNativeClearTextOK:
		case kAuthNativeNoClearText:
		case kAuthNativeRetainCredential:
			if ( (inParams.pOldPassword == nil) || ( (inParams.pOldPassword != nil) && (DSIsStringEmpty(inParams.pOldPassword) == true) ) )
				bCheckDelay = false;
			// fall through
		
		case kAuthSetPasswd:
		case kAuthSetPasswdAsRoot:
		case kAuthSMB_NT_Key:
		case kAuthSMB_LM_Key:
		case kAuthSecureHash:
		case kAuthChangePasswd:
			if ( bCheckDelay && (siResult == eDSAuthFailed) && ( inPlugin->DelayFailedLocalAuthReturnsDeltaInSeconds() != 0) )
			{
				//save time of last failure
				//save the current failure time
				//save the inParams.pUserName and total count of failures
				//if > 5th failure AND (current failure time - last failure time) > 10*(# failures -5)
				//then remove entry
				//if (> 5th failure) then delay return
				//delay return time = (# failures - 5) seconds
				gHashAuthFailedLocalMapLock->WaitLock();
				string aUserName(inParams.pUserName);
				HashAuthFailedMapI  aHashAuthFailedMapI	= gHashAuthFailedLocalMap.find(aUserName);
				if (aHashAuthFailedMapI == gHashAuthFailedLocalMap.end())
				{
					//create an entry in the map
					pHashAuthFailed = (sHashAuthFailed *)calloc(1, sizeof(sHashAuthFailed));
					pHashAuthFailed->nowTime = dsTimestamp();
					pHashAuthFailed->lastTime = pHashAuthFailed->nowTime;
					pHashAuthFailed->failCount = 1;
					gHashAuthFailedLocalMap[aUserName] = pHashAuthFailed;
					gHashAuthFailedLocalMapLock->SignalLock();
				}
				else
				{
					//use the current entry
					pHashAuthFailed = aHashAuthFailedMapI->second;
					pHashAuthFailed->lastTime = pHashAuthFailed->nowTime;
					pHashAuthFailed->nowTime = dsTimestamp();
					pHashAuthFailed->failCount++;
					if ( pHashAuthFailed->failCount > 5 )
					{
						if (pHashAuthFailed->failCount == 6)
						{
							syslog(LOG_ALERT, "Failed Authentication return is being delayed due to over five\
							 recent auth failures for username: %s.", inParams.pUserName);
						}
						// after two minutes, remove the delay
						if ( (pHashAuthFailed->nowTime - pHashAuthFailed->lastTime) > 120 * USEC_PER_SEC )
						{
							//it has been a long time since the last failure so let's remove this and not track
							//first new failure
							gHashAuthFailedLocalMap.erase(aUserName);
							free(pHashAuthFailed);
							pHashAuthFailed = NULL;
							gHashAuthFailedLocalMapLock->SignalLock();
						}
						else
						{
							//let's delay the return for the failed auth since too many failures have occurred
							//too rapidly
							struct mach_timebase_info timeBaseInfo;
							mach_timebase_info( &timeBaseInfo );
							uint64_t delay = MAX(inPlugin->DelayFailedLocalAuthReturnsDeltaInSeconds(), 2) *
								(((uint64_t)NSEC_PER_SEC * (uint64_t)timeBaseInfo.denom) / (uint64_t)timeBaseInfo.numer);
							
							gHashAuthFailedLocalMapLock->SignalLock(); //don't hold the lock when delaying
							mach_wait_until( mach_absolute_time() + delay );
						}
					}
					else
					{
						gHashAuthFailedLocalMapLock->SignalLock();
					}
				}
			}
			break;
		default:
			break;
	}

finish:
	DSCFRelease( myHashTypeArray );
	DSCFRelease( adminString );
	
	if ( inParams.stateFilePath != NULL && inParams.PolicyStateChanged() )
		CDSLocalAuthHelper::WriteHashStateFile( inParams.stateFilePath, &inParams.state );
	
	return( siResult );
} // DoShadowHashAuth


tDirStatus CDSLocalAuthHelper::DoBasicAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData, tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
	CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
	CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
	CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
	uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType )
{
	tDirStatus siResult = eDSNoErr;
	UInt32 uiAuthMethod	= inParams.uiAuthMethod;
	
	try
	{
		if (inParams.mAuthMethodStr != NULL)
		{
			DbgLog(  kLogPlugin, "CDSLocalAuthHelper::DoBasicAuth(): Attempting use of authentication method %s",
				inParams.mAuthMethodStr );
		}
		
		switch( uiAuthMethod )
		{
			//native auth is always UNIX crypt possibly followed by 2-way random using tim auth server
			case kAuthNativeMethod:
			case kAuthNativeNoClearText:
			case kAuthNativeClearTextOK:
			case kAuthNativeRetainCredential:
				if ( outAuthData == NULL ) throw( eDSNullAuthStepData );
				siResult = CDSLocalAuthHelper::DoUnixCryptAuth( inNodeRef, inParams,
					inOutContinueData, inAuthData, outAuthData, inAuthOnly, isSecondary, inAuthAuthorityList,
					inGUIDString, inAuthedUserIsAdmin, inMutableRecordDict, inHashList, inPlugin, inNode,
					inAuthedUserName, inUID, inEffectiveUID, inNativeRecType );
				if ( siResult == eDSNoErr )
				{
					if ( outAuthData->fBufferSize > sizeof(kDSStdAuthCrypt) - 1 )
					{
						::strcpy( outAuthData->fBufferData, kDSStdAuthCrypt );
					}
				}
				else if ( (siResult != eDSAuthFailed) && (siResult != eDSInvalidBuffFormat) )
				{
					siResult = CDSLocalAuthHelper::DoNodeNativeAuth( inNodeRef, inParams,
						inOutContinueData, inAuthData, outAuthData, inAuthOnly, isSecondary, NULL,
						inGUIDString, inAuthedUserIsAdmin, inMutableRecordDict, inHashList, inPlugin, inNode,
						inAuthedUserName, inUID, inEffectiveUID, inNativeRecType );

					if ( siResult == eDSNoErr )
					{
						if ( outAuthData->fBufferSize > sizeof(kDSStdAuth2WayRandom) - 1 )
						{
							::strcpy( outAuthData->fBufferData, kDSStdAuth2WayRandom );
						}
					}
				}
				break;

			case kAuthClearText:
			case kAuthCrypt:
				siResult = CDSLocalAuthHelper::DoUnixCryptAuth( inNodeRef, inParams,
					inOutContinueData, inAuthData, outAuthData, inAuthOnly, isSecondary, inAuthAuthorityList,
					inGUIDString, inAuthedUserIsAdmin, inMutableRecordDict, inHashList, inPlugin, inNode,
					inAuthedUserName, inUID, inEffectiveUID, inNativeRecType );
				break;

			case kAuthSetPasswd:
				siResult = CDSLocalAuthHelper::DoSetPassword( inNodeRef, inParams,
					inOutContinueData, inAuthData, outAuthData, inAuthOnly, isSecondary, inAuthAuthorityList, NULL,
					inGUIDString, inAuthedUserIsAdmin, inMutableRecordDict, inHashList, inPlugin, inNode,
					inAuthedUserName, inUID, inEffectiveUID, inNativeRecType );
				break;

			case kAuthSetPasswdAsRoot:
				siResult = CDSLocalAuthHelper::DoSetPasswordAsRoot( inNodeRef, inParams,
					inOutContinueData, inAuthData, outAuthData, inAuthOnly, isSecondary, inAuthAuthorityList, NULL,
					inGUIDString, inAuthedUserIsAdmin, inMutableRecordDict, inHashList, inPlugin, inNode,
					inAuthedUserName, inUID, inEffectiveUID, inNativeRecType );
				break;

			case kAuthChangePasswd:
				siResult = CDSLocalAuthHelper::DoChangePassword( inNodeRef, inParams,
					inOutContinueData, inAuthData, outAuthData, inAuthOnly, isSecondary, NULL,
					inGUIDString, inAuthedUserIsAdmin, inMutableRecordDict, inHashList, inPlugin, inNode,
					inAuthedUserName, inUID, inEffectiveUID, inNativeRecType );
				break;
			
			case kAuthSetCertificateHashAsRoot:
				AddKerberosAuthAuthority(
						inNodeRef, inParams.pUserName, kDSTagAuthAuthorityKerberosv5Cert, inAuthAuthorityList, inMutableRecordDict,
						inPlugin, inNode, inNativeRecType, true );
				
				siResult = CDSLocalAuthHelper::DoKerberosCertAuth(
													inNodeRef, inParams, inOutContinueData, inAuthData, outAuthData, inAuthOnly,
													isSecondary, inAuthAuthorityList, inGUIDString, inAuthedUserIsAdmin,
													inMutableRecordDict, inHashList, inPlugin, inNode, inAuthedUserName, inUID,
													inEffectiveUID, inNativeRecType );
				break;
			
			default:
				siResult = eDSAuthMethodNotSupported;
		}
	}

	catch( tDirStatus err )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::DoBasicAuth(): got error %d", err );
		siResult = err;
	}

	return( siResult );
}

tDirStatus CDSLocalAuthHelper::DoPasswordServerAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData, tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
	CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
	CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
	CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
	uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType )
{
    tDirStatus result = eDSAuthFailed;
    tDirStatus error;
    UInt32 authMethod;
    char *serverAddr = NULL;
    char *uidStr = NULL;
    long uidStrLen;
    tDataBufferPtr authDataBuff = NULL;
    tDataBufferPtr authDataBuffTemp = NULL;
    char *userName = NULL;
	char *password = NULL;
	CFMutableDictionaryRef continueDataDict = NULL;
    tContextData passPluginContinueData = NULL;
	char* cStr = NULL;
	size_t cStrSize = 0;
    char *aaData = NULL;
	CFMutableDictionaryRef nodeDict = NULL;
	
    try
    {
		aaData = inAuthAuthorityList.GetDataForTag( kDSTagAuthAuthorityPasswordServer, 0 );
		if ( aaData == NULL || *aaData == '\0' )
      		throw( eDSAuthParameterError );
    
       //DbgLog( kLogPlugin, "AuthorityData=%s\n", aaData);
        
        serverAddr = strchr( aaData, ':' );
        if ( serverAddr )
        {
            uidStrLen = serverAddr - aaData;
			
			uidStr = dsCStrFromCharacters( aaData, uidStrLen );
            if ( uidStr == NULL ) throw( eMemoryError );
            
            // advance past the colon
            serverAddr++;
            
			// try any method to the password server, even if unknown
			if (inParams.mAuthMethodStr != NULL)
			{
				DbgLog(  kLogPlugin, "DSLocalAuthHelper::DoPasswordSerberAuth(): Attempting use of authentication method %s",
					inParams.mAuthMethodStr );
			}
			
			authMethod = inParams.uiAuthMethod;
            switch( authMethod )
            {
                case kAuth2WayRandom:
                    if ( *inOutContinueData == 0 )
                    {
						continueDataDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks,
							&kCFTypeDictionaryValueCallBacks );
                        if ( continueDataDict == NULL )
                            throw( eMemoryError );
						
						inPlugin->AddContinueData( inNodeRef, continueDataDict, inOutContinueData );

                        // make a buffer for the user ID
                        authDataBuff = ::dsDataBufferAllocatePriv( uidStrLen + 1 );
                        if ( authDataBuff == NULL ) throw ( eMemoryError );
                        
                        // fill
                        strcpy( authDataBuff->fBufferData, uidStr );
                        authDataBuff->fBufferLength = uidStrLen;
                    }
                    else
                    {
                        continueDataDict = (CFMutableDictionaryRef) gLocalContinueTable->GetPointer( *inOutContinueData );
						if ( continueDataDict == NULL )
							throw eDSInvalidContinueData;
                            
                        authDataBuff = inAuthData;
                    }
                    break;
                    
                case kAuthSetPasswd:
				case kAuthSetPolicy:
				    {
                        char* aaVersion = NULL;
                        char* aaTag = NULL;
                        char* aa2Data = NULL;
                        char* endPtr = NULL;
                        
                        // lookup the user that wasn't passed to us
                       	error = (tDirStatus)GetUserNameFromAuthBuffer( inAuthData, 3, &userName );
                        if ( error != eDSNoErr ) throw( error );
						
						CFArrayRef authAuthorities = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict,
							inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrAuthenticationAuthority ) ) );
						CFIndex numAuthAuthorities = 0;
						if ( authAuthorities != NULL )
							numAuthAuthorities = CFArrayGetCount( authAuthorities );
						
                        tDirStatus lookupResult = eDSAuthFailed;
                        // don't break or throw to guarantee cleanup
						for( CFIndex authAuthorityIndex = 0;
							authAuthorityIndex < numAuthAuthorities && lookupResult == eDSAuthFailed;
							authAuthorityIndex++ )
						{
							CFStringRef authAuthority = (CFStringRef)CFArrayGetValueAtIndex( authAuthorities,
								authAuthorityIndex );
							const char* authAuthorityCStr = CStrFromCFString( authAuthority, &cStr,
								&cStrSize );

                            // parse this value of auth authority
                            error = dsParseAuthAuthority( authAuthorityCStr, &aaVersion, &aaTag, &aa2Data );
                            // need to check version
                            if (error != eDSNoErr)
                                lookupResult = eParameterError;
                            
                            if ( error == eDSNoErr && strcmp(aaTag, kDSTagAuthAuthorityPasswordServer) == 0 )
                            {
                                endPtr = strchr( aa2Data, ':' );
                                if ( endPtr == NULL )
                                {
                                    lookupResult = eParameterError;
                                }
                                else
                                {
                                    *endPtr = '\0';
                                    lookupResult = eDSNoErr;
                                }
                            }
                            
                            if (aaVersion != NULL) {
                                free(aaVersion);
                                aaVersion = NULL;
                            }
                            if (aaTag != NULL) {
                                free(aaTag);
                                aaTag = NULL;
                            }
                            if (lookupResult != eDSNoErr && aa2Data != NULL) {
                                free(aa2Data);
                                aa2Data = NULL;
                            }
                        }

                        
                        if ( lookupResult != eDSNoErr ) throw( eDSAuthFailed );
                        
                        // do the usual
                        error = (tDirStatus)RepackBufferForPWServer(inAuthData, uidStr, 1, &authDataBuffTemp );
                        if ( error != eDSNoErr ) throw( error );
                        
                        // put the admin user ID in slot 3
                        error = (tDirStatus)RepackBufferForPWServer(authDataBuffTemp, aa2Data, 3, &authDataBuff );
						DSFreeString( aa2Data );
                        if ( error != eDSNoErr ) throw( error );
                    }
                    break;
					
				case kAuthClearText:
				case kAuthNativeClearTextOK:
				case kAuthNativeNoClearText:
				case kAuthCrypt:
				case kAuthNativeRetainCredential:
					{
						tDataListPtr dataList = dsAuthBufferGetDataListAllocPriv(inAuthData);
						if (dataList != NULL)
						{
							userName = dsDataListGetNodeStringPriv(dataList, 1);
							password = dsDataListGetNodeStringPriv(dataList, 2);
							// this allocates a copy of the string
							
							dsDataListDeallocatePriv(dataList);
							free(dataList);
							dataList = NULL;
						}
					}
					//fall through
                
                default:
                    error = (tDirStatus)RepackBufferForPWServer(inAuthData, uidStr, 1, &authDataBuff );
                    if ( error != eDSNoErr ) throw( error );
            }
			
			nodeDict = inPlugin->CopyNodeDictForNodeRef( inNodeRef );
			if ( nodeDict == NULL )
				throw(  eDSInvalidNodeRef );

			tDirReference pwsDirRef = 0;
			tDirNodeReference pwsNodeRef = 0;
			
			result = OpenPasswordServerNode( inPlugin, nodeDict, serverAddr, &pwsDirRef, &pwsNodeRef );
			if ( result != eDSNoErr )
				throw( result );
			
            if ( continueDataDict != NULL ) {
				CFNumberRef cfContinue = (CFNumberRef) CFDictionaryGetValue( continueDataDict,
																			 CFSTR(kAuthCOntinueDataPassPluginContData) );
				if ( cfContinue != NULL )
					CFNumberGetValue( cfContinue, kCFNumberIntType, &passPluginContinueData );
			}
            
			tDataNodePtr authMethodNodePtr = dsDataNodeAllocateString( 0, inParams.mAuthMethodStr );
            result = dsDoDirNodeAuth( pwsNodeRef, authMethodNodePtr, inAuthOnly, authDataBuff, outAuthData,
				&passPluginContinueData );
			
			if ( result == eDSAuthNoAuthServerFound || result == eDSAuthServerError )
			{
				result = CDSLocalAuthHelper::PWSetReplicaData( inPlugin, inNode, inNodeRef, pwsNodeRef, uidStr );
				if ( result == eDSNoErr )
				{
					result = dsDoDirNodeAuth( pwsNodeRef, authMethodNodePtr, inAuthOnly, authDataBuff, outAuthData,
						&passPluginContinueData );
				}		
			}
			dsDataBufferDeAllocate( 0, authMethodNodePtr );
			authMethodNodePtr = NULL;
			
            if ( continueDataDict != NULL )
            {
				if ( passPluginContinueData == 0 )
					CFDictionaryRemoveValue( continueDataDict, CFSTR( kAuthCOntinueDataPassPluginContData ) );
				else
				{
					CFNumberRef	cfRefNum = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &passPluginContinueData );
					CFDictionaryAddValue( continueDataDict, CFSTR( kAuthCOntinueDataPassPluginContData ), cfRefNum );
					CFRelease( cfRefNum );

                    if ( inOutContinueData == NULL )
                        throw( eDSNullParameter );
                    *inOutContinueData = NULL;
                }
            }
            
			if ( (result == eDSNoErr) && (inAuthOnly == false) && (userName != NULL) && (password != NULL) )
			{
				result = inPlugin->AuthOpen( inNodeRef, userName, password, inAuthedUserIsAdmin );
			}
        }
    }
    catch( tDirStatus err )
    {
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::DoPasswordServerAuth(): got error %d", err );
        result = err;
    }
    
	DSCFRelease( nodeDict );
	DSFreeString( aaData );
	DSFree( cStr );
	DSFree( uidStr );
	DSFree( userName );
	DSFreePassword( password );
    
    if ( authDataBuff )
        dsDataBufferDeallocatePriv( authDataBuff );
    if ( authDataBuffTemp )
        dsDataBufferDeallocatePriv( authDataBuffTemp );
                        
	return( result );
} // DoPasswordServerAuth

tDirStatus CDSLocalAuthHelper::DoDisabledAuth(	tDirNodeReference inNodeRef,
												CDSLocalAuthParams &inParams,
												tContextData* inOutContinueData,
												tDataBufferPtr inAuthData,
												tDataBufferPtr outAuthData,
												bool inAuthOnly,
												bool isSecondary,
												CAuthAuthority &inAuthAuthorityList,
												const char* inGUIDString,
												bool inAuthedUserIsAdmin, 
												CFMutableDictionaryRef inMutableRecordDict,
												unsigned int inHashList,
												CDSLocalPlugin* inPlugin,
												CDSLocalPluginNode* inNode,
												CFStringRef inAuthedUserName,
												uid_t inUID,
												uid_t inEffectiveUID,
												CFStringRef inNativeRecType )
{
	tDirStatus				retResult						= eDSAuthAccountDisabled;
	tDirStatus				siResult						= eDSAuthFailed;
	UInt32					uiAuthMethod					= 0;
	bool					bNetworkNodeReachable			= false;
	tDirReference			aDSRef							= 0;
	tDataListPtr			aNetworkNode					= nil;
	char					*pUserName						= NULL;
	tDataListPtr			dataList						= NULL;
	char					*localCachedUser				= NULL;
	char					*origAuthAuthorityTag			= NULL;
	
	try
	{
		// The disabled auth authority keeps the enabled version in the data section.
		// Example: ;DisabledUser;;ShadowHash;
		// Double-check that the original auth-authority is ShadowHash and allow get policy methods.
		// We can check for a non-NULL auth-authority data, but the data is not included because of the extra
		// semicolon. 

		// the second data item (index 1) is the original authentication authority tag
		origAuthAuthorityTag = inAuthAuthorityList.GetDataForTag( kDSTagAuthAuthorityDisabledUser, 1 );
		if ( origAuthAuthorityTag == NULL || *origAuthAuthorityTag == '\0' )
			throw( eDSAuthParameterError );
		
		// the third data item (index 2) is the original authentication authority's data
		inParams.aaDataLocalCacheUser = inAuthAuthorityList.GetDataForTag( kDSTagAuthAuthorityDisabledUser, 2 );
		
		uiAuthMethod = inParams.uiAuthMethod;

		if ( strcasecmp(origAuthAuthorityTag, kDSTagAuthAuthorityLocalCachedUser) == 0 )
		{
			// "Reachable" in this context means that the user's network node is configured on the
			// search policy. It's not a guarantee that the ldap server will respond.
			siResult = CDSLocalAuthHelper::LocalCachedUserReachable(
												inNodeRef,
												inOutContinueData,
												inAuthData,
												outAuthData,
												inAuthOnly,
												isSecondary,
												inGUIDString,
												inAuthedUserIsAdmin,
												inParams,
												inMutableRecordDict,
												inHashList,
												inPlugin,
												inNode,
												inAuthedUserName,
												inUID,
												inEffectiveUID,
												inNativeRecType,
												&bNetworkNodeReachable,
												&aDSRef,
												&aNetworkNode,
												&localCachedUser);

			if ( siResult != eDSNoErr ) {
				retResult = siResult;
				throw( siResult );
			}
			
			if ( bNetworkNodeReachable )
			{
				inAuthAuthorityList.ToggleDisabledAuthority( true );
				
				siResult = CDSLocalAuthHelper::DoLocalCachedUserAuthPhase2(
												inNodeRef,
												inParams,
												inOutContinueData,
												inAuthData,
												outAuthData,
												inAuthOnly,
												isSecondary,
												inAuthAuthorityList,
												inGUIDString,
												inAuthedUserIsAdmin, 
												inMutableRecordDict,
												inHashList,
												inPlugin,
												inNode,
												inAuthedUserName,
												inUID,
												inEffectiveUID,
												inNativeRecType,
												&bNetworkNodeReachable,
												&aDSRef,
												&aNetworkNode,
												localCachedUser,
												kDoNotTouchTheAuthAuthorities );
				
				if ( bNetworkNodeReachable && siResult == eDSNoErr )
				{
					switch( uiAuthMethod )
					{
						// re-enable the user for these methods
						case kAuthNativeClearTextOK:
						case kAuthNativeNoClearText:
						case kAuthNativeMethod:
						case kAuthNativeRetainCredential:
							retResult = eDSNoErr;
							
							dataList = dsAuthBufferGetDataListAllocPriv( inAuthData );
							if ( dataList == NULL )
								throw( eDSInvalidBuffFormat );
							
							if ( dsDataListGetNodeCountPriv(dataList) < 1 )
								throw( eDSInvalidBuffFormat );
							
							pUserName = dsDataListGetNodeStringPriv(dataList, 1);
							if ( pUserName == NULL || DSIsStringEmpty(pUserName) )
								throw( eDSInvalidBuffFormat );
							
							siResult = CDSLocalAuthHelper::SetUserAuthAuthorityAsRoot(
																inMutableRecordDict,
																inPlugin,
																inNode,
																CFSTR( kDSStdRecordTypeUsers ),
																pUserName,
																inAuthAuthorityList,
																inNodeRef );
							break;
						
						default:
							break;
					}
				}
				
				throw( retResult );
			}
			else
			{
				throw( eDSAuthAccountDisabled );
			}
		}
		
		switch ( uiAuthMethod )
		{
			case kAuthGetPolicy:
			case kAuthGetEffectivePolicy:
			case kAuthSetPolicyAsRoot:
			case kAuthSetPasswdAsRoot:
				inAuthAuthorityList.ToggleDisabledAuthority( true );
				siResult = CDSLocalAuthHelper::DoShadowHashAuth(
													inNodeRef,
													inParams,
													inOutContinueData,
													inAuthData,
													outAuthData,
													inAuthOnly,
													isSecondary,
													inAuthAuthorityList,
													inGUIDString,
													inAuthedUserIsAdmin,
													inMutableRecordDict,
													inHashList,
													inPlugin,
													inNode,
													inAuthedUserName,
													inUID,
													inEffectiveUID,
													inNativeRecType );
				inAuthAuthorityList.ToggleDisabledAuthority( false );
				break;
			
			default: 
				siResult = eDSAuthFailed;
		}
	}
	catch( tDirStatus status )
	{
		siResult = status;
	}
	
	if ( aNetworkNode != NULL )
	{
		dsDataListDeallocate( aDSRef, aNetworkNode );
		free( aNetworkNode );
		aNetworkNode = NULL;
	}
	if ( dataList != NULL )
	{
		dsDataListDeallocatePriv( dataList );
		free( dataList );
		dataList = NULL;
	}
	DSFreeString(localCachedUser);
	DSFreeString( pUserName );
	DSFreeString( origAuthAuthorityTag );
	
	return retResult;
} // DoDisabledAuth


tDirStatus CDSLocalAuthHelper::DoLocalCachedUserAuth(	tDirNodeReference inNodeRef,
														CDSLocalAuthParams &inParams,
														tContextData *inOutContinueData,
														tDataBufferPtr inAuthData,
														tDataBufferPtr outAuthData,
														bool inAuthOnly,
														bool isSecondary,
														CAuthAuthority &inAuthAuthorityList,
														const char* inGUIDString,
														bool inAuthedUserIsAdmin, 
														CFMutableDictionaryRef inMutableRecordDict,
														unsigned int inHashList,
														CDSLocalPlugin* inPlugin,
														CDSLocalPluginNode* inNode,
														CFStringRef inAuthedUserName,
														uid_t inUID,
														uid_t inEffectiveUID,
														CFStringRef inNativeRecType )
{
	tDirStatus				siResult				= eDSAuthFailed;
	bool					bNetworkNodeReachable	= false;
	tDirReference			aDSRef					= 0;
	tDataListPtr			aNetworkNode			= nil;
	char					*localCachedUser		= NULL;
	
    try
    {
		if ( inParams.mAuthMethodStr != NULL )
			DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoLocalCachedUserAuth: Attempting use of authentication method %s",
					inParams.mAuthMethodStr );
		
		// Parse input buffer, read hash(es)
		siResult = inParams.LoadDSLocalParamsForAuthMethod( inParams.uiAuthMethod, ePluginHashAll, inGUIDString, inAuthedUserIsAdmin,
						inAuthData, outAuthData );
		if ( siResult != eDSNoErr ) {
			DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoLocalCachedUserAuth: LoadDSLocalParamsForAuthMethod = %d", siResult );
			throw( siResult );
		}
		
		inParams.aaDataLocalCacheUser = inAuthAuthorityList.GetDataForTag( kDSTagAuthAuthorityLocalCachedUser, 0 );
		if ( inParams.aaDataLocalCacheUser == NULL ) {
			DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoLocalCachedUserAuth: GetDataForTag = NULL" );
      		throw( eDSAuthParameterError );
		}
		
		siResult = LocalCachedUserReachable( inNodeRef, inOutContinueData, inAuthData, outAuthData,
											 inAuthOnly, isSecondary, inGUIDString, inAuthedUserIsAdmin,
											 inParams, inMutableRecordDict, inHashList, inPlugin, inNode,
											 inAuthedUserName, inUID, inEffectiveUID, inNativeRecType,
											 &bNetworkNodeReachable, &aDSRef, &aNetworkNode, &localCachedUser);				
		DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoLocalCachedUserAuth:LocalCachedUserReachable result = %d, valid-node-on-search-path = %d",
			    siResult, bNetworkNodeReachable );
		
		if ( siResult == eDSNoErr )
		{
			siResult = DoLocalCachedUserAuthPhase2( inNodeRef, inParams, inOutContinueData, inAuthData,
												    outAuthData, inAuthOnly, isSecondary, inAuthAuthorityList,
												    inGUIDString, inAuthedUserIsAdmin,  inMutableRecordDict,
												    inHashList, inPlugin, inNode, inAuthedUserName, inUID,
												    inEffectiveUID, inNativeRecType, &bNetworkNodeReachable,
													&aDSRef, &aNetworkNode, localCachedUser );
		}
	}
	catch( tDirStatus status )
	{
		siResult = status;
	}
	
	if ( aNetworkNode != NULL )
	{
		dsDataListDeallocate( aDSRef, aNetworkNode );
		free( aNetworkNode );
		aNetworkNode = NULL;
	}
	DSFreeString( localCachedUser );
	
	return( (tDirStatus)siResult );
} // DoLocalCachedUserAuth


//------------------------------------------------------------------------------------
//	* DoNodeNativeAuth
//------------------------------------------------------------------------------------

//this is REALLY 2-way random with tim authserver ie. name is very mis-leading
tDirStatus CDSLocalAuthHelper::DoNodeNativeAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData, tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
	const char* inAuthAuthorityData, const char* inGUIDString, bool inAuthedUserIsAdmin, 
	CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
	CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
	uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType )
{
	tDirStatus			siResult		= eDSAuthFailed;
	return( siResult );
}

tDirStatus CDSLocalAuthHelper::DoUnixCryptAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData, tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
	CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
	CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
	CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
	uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType )
{
	tDirStatus			siResult			= eDSAuthFailed;
	bool				bResetCache			= false;
	const char			*pwdAttrValueCStr	= NULL;
	char*				cStr				= NULL;
	size_t				cStrSize			= 0;
	char				*name				= NULL;
	UInt32				nameLen				= 0;
	char				*pwd				= NULL;
	UInt32				pwdLen				= 0;
	unsigned int		itemCount			= 0;
	char				salt[9];
	char				hashPwd[32];
	
	try
	{
		if ( inAuthData == NULL )
			throw( eDSNullAuthStepData );
		
		// need length for username, password
		siResult = (tDirStatus)Get2FromBuffer( inAuthData, NULL, &name, &pwd, &itemCount );
		if ( siResult != eDSNoErr )
			throw( siResult );
			
		nameLen = strlen( name );
		if ( itemCount != 2 || nameLen > 256 )
			throw( eDSInvalidBuffFormat );
		
		pwdLen = strlen( pwd );
		if ( pwdLen >= kHashRecoverableLength )
			throw ( eDSAuthPasswordTooLong );
		
		DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoUnixCryptAuth(): Attempting UNIX Crypt authentication" );
		
		CFStringRef nameCFStr = CFStringCreateWithCString( NULL, name, kCFStringEncodingUTF8 );
		CFArrayRef values = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrRecordName ) ) );

		bool nameIsValid = CFArrayContainsValue( values, ::CFRangeMake( 0, CFArrayGetCount( values ) ), nameCFStr );
		CFRelease( nameCFStr );
		if ( !nameIsValid )
			throw( eDSAuthUnknownUser );
		
		values =  (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ) );
		if ( values == NULL )
			throw( eDSAuthBadPassword );
		
		CFStringRef pwdAttrValue = NULL;
		if ( CFArrayGetCount( values ) > 0 )
			pwdAttrValue = (CFStringRef)CFArrayGetValueAtIndex( values, 0 );
		else
			pwdAttrValue = CFSTR( "" );
		
		pwdAttrValueCStr = CStrFromCFString( pwdAttrValue, &cStr, &cStrSize, NULL );
		
		//account for the case where pwdAttrValueCStr == "" such that we will auth if pwdLen is 0
		if ( strcmp(pwdAttrValueCStr,"") != 0 )
		{
			salt[0] = pwdAttrValueCStr[0];
			salt[1] = pwdAttrValueCStr[1];
			salt[2] = '\0';

			bzero( hashPwd, 32 );
			strcpy( hashPwd, crypt(pwd, salt) );
			
			siResult = (strcmp(hashPwd, pwdAttrValueCStr) == 0) ? eDSNoErr : eDSAuthFailed;
			bzero( hashPwd, 32 );
		}
		else // pwdAttrValueCStr is == ""
		{
			if ( DSIsStringEmpty(pwd) )
				siResult = eDSNoErr;
		}
		
		if (siResult == eDSNoErr)
		{
			if ( inNode->IsLocalNode() )
			{
				if (pwdLen < 8)
				{
					// local node and true crypt password since length is 7 or less
					CDSLocalAuthHelper::MigrateToShadowHash( inNodeRef, inPlugin, inNode, inMutableRecordDict, name, pwd,
						&bResetCache, inHashList, inNativeRecType );
				}
				else
				{
					tRecordReference		recordRef					= 0;
					CFMutableDictionaryRef	nodeDict					= NULL;
					CFStringRef				preRootAuthString			= NULL;
					CFStringRef				cfString					= NULL;
					
					// ensure authentication_authority attribute
					CFStringRef nativeAAType = inPlugin->AttrNativeTypeForStandardType( CFSTR(kDSNAttrAuthenticationAuthority) );
					values = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict, nativeAAType );
					if ( values == NULL )
					{
						try
						{
							cfString = CFStringCreateWithCString( NULL, name, kCFStringEncodingUTF8 );
							siResult = inPlugin->OpenRecord( inNativeRecType, cfString, &recordRef );
							DSCFRelease( cfString );
							if ( siResult != eDSNoErr )
								throw( siResult );
							
							// get root powers
							// retrieve the same nodeDict the plugin object is going to use 
							
							CFDictionaryRef openRecordDict = inPlugin->RecordDictForRecordRef( recordRef );
							if ( openRecordDict == NULL )
								throw( eDSInvalidRecordRef );
							
							nodeDict = (CFMutableDictionaryRef)CFDictionaryGetValue( openRecordDict, CFSTR(kOpenRecordDictNodeDict) );
							if ( nodeDict == NULL )
								throw( eDSInvalidNodeRef );
							
							preRootAuthString = (CFStringRef) CFDictionaryGetValue( nodeDict, CFSTR(kNodeAuthenticatedUserName) );
							if ( preRootAuthString != NULL )
								CFRetain( preRootAuthString );

							// does this need to be protected since nodeDicts are per-client?
							CFDictionarySetValue( nodeDict, CFSTR(kNodeAuthenticatedUserName), CFSTR("root") );
							
							// replace the auth authority and password attributes in the record
							siResult = ::SetUserAuthAuthorityAsRoot(
											inMutableRecordDict,
											inPlugin,
											inNode,
											inPlugin->RecordStandardTypeForNativeType(inNativeRecType),
											name,
											kDSValueAuthAuthorityBasic,
											inNodeRef,
											false );
							
							if ( siResult != eDSNoErr )
								throw( siResult );
							
							// and modify the copy of the user record that we're passing around			
							CFMutableArrayRef mutableArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
							CFArrayAppendValue( mutableArray, CFSTR( kDSValueAuthAuthorityBasic ) );
							CFDictionarySetValue( inMutableRecordDict, nativeAAType, mutableArray );
							DSCFRelease( mutableArray );
						}
						catch( tDirStatus catchStatus )
						{
							siResult = catchStatus;
						}
						
						if ( recordRef != 0 ) {
							DbgLog( kLogDebug, "CDSLocalAuthHelper::DoUnixCryptAuth - closing internal record reference" );
							gRefTable.RemoveReference( recordRef, eRefTypeRecord, 0, 0 ); // remove closes the record automatically
							recordRef = 0;
						}
						
						DSFreeString( cStr );
						
						if ( nodeDict != NULL )
						{
							if ( preRootAuthString != NULL )
								CFDictionarySetValue( nodeDict, CFSTR(kNodeAuthenticatedUserName), preRootAuthString );
							else
								CFDictionaryRemoveValue( nodeDict, CFSTR(kNodeAuthenticatedUserName) );
						}
						DSCFRelease( preRootAuthString );
					}
				}
				
				MigrateAddKerberos( inNodeRef, inParams, inOutContinueData, inAuthData, outAuthData, inAuthOnly, isSecondary,
					inAuthAuthorityList, inGUIDString, inAuthedUserIsAdmin, inMutableRecordDict, inHashList, inPlugin, inNode,
					inAuthedUserName, inUID, inEffectiveUID, inNativeRecType, true );
			}
			
			if (inAuthOnly == false)
				siResult = inPlugin->AuthOpen( inNodeRef, name, pwd, inAuthedUserIsAdmin );
		}
	}
	catch( tDirStatus err )
	{
		DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoUnixCryptAuth(): Crypt authentication error %d", err );
		siResult = err;
	}
	
	DSFreeString( name );
	DSFreePassword( pwd );
	DSFreeString( cStr );
	
	return( siResult );

}

tDirStatus CDSLocalAuthHelper::DoSetPassword( tDirNodeReference inNodeRef, CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData, tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary, CAuthAuthority &inAuthAuthorityList,
	const char* inAuthAuthorityData, const char* inGUIDString, bool inAuthedUserIsAdmin, 
	CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
	CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
	uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType )
{
	tDirStatus			siResult		= eDSAuthFailed;
	bool				bResetCache		= false;
	char			   *userName		= NULL;
	char			   *userPwd			= NULL;
	char			   *rootName		= NULL;
	char			   *rootPwd			= NULL;
	CFMutableArrayRef	recordsArray	= NULL;
	char*				cStr			= NULL;
	size_t				cStrSize		= 0;
	CFStringRef			userNameCFStr	= NULL;
	tRecordReference	recordRef		= 0;
	CFStringRef			hashPwdCFStr	= NULL;
	tDataListPtr		dataList		= NULL;
	unsigned int		itemCount		= 0;
	
	try
	{
		if ( inAuthData == NULL )
			throw( eDSNullAuthStepData );
		
		// need length for username, password, root username, and root password.
		// both usernames must be at least one character
		siResult = (tDirStatus)Get2FromBuffer( inAuthData, &dataList, &userName, &userPwd, &itemCount );
		if ( siResult != eDSNoErr )
			throw( siResult );
		if ( itemCount != 4 || userName == NULL || DSIsStringEmpty(userName) || userPwd == NULL )
			throw( eDSInvalidBuffFormat );
		
		// Get the root users name
		rootName = dsDataListGetNodeStringPriv( dataList, 3 );
		if ( rootName == NULL || DSIsStringEmpty(rootName) )
			throw( eDSInvalidBuffFormat );
		
		// Get the root users password
		rootPwd = dsDataListGetNodeStringPriv( dataList, 4 );
		if ( rootPwd == NULL )
			throw( eDSInvalidBuffFormat );
		
		//need some code here to directly set the NI Crypt password
		//also first check to see if the password already exists

		char			salt[3];
		char			hashPwd[ 32 ];
		const char	   *pwdFromRecord	= NULL;

		//need some code here to directly change the NI Crypt password
		//first check to see that the old password is valid
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::DoSetPassword(): Attempting UNIX Crypt password change" );

		recordsArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		CFStringRef rootNameCFStr = CFStringCreateWithCString( NULL, rootName, kCFStringEncodingUTF8 );
		CFArrayRef patternsToMatch = CFArrayCreate( NULL, (const void**)&rootNameCFStr, 1, &kCFTypeArrayCallBacks );
		CFRelease( rootNameCFStr );
		siResult = inNode->GetRecords( inPlugin->RecordNativeTypeForStandardType( CFSTR( kDSStdRecordTypeUsers ) ),
			patternsToMatch, CFSTR( kDSNAttrRecordName ), eDSExact, false, 1, recordsArray );
		CFRelease( patternsToMatch );

		if ( ( siResult != eDSNoErr ) || ( CFArrayGetCount( recordsArray ) == 0 ) )
#ifdef DEBUG
			throw( eDSAuthUnknownUser );
#else
			throw( eDSAuthFailed );
#endif

		CFDictionaryRef rootDict = (CFDictionaryRef)CFArrayGetValueAtIndex( recordsArray, 0 );
		CFArrayRef uidsArray = (CFArrayRef)CFDictionaryGetValue( rootDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrUniqueID ) ) );
		if ( ( uidsArray == NULL ) || ( CFArrayGetCount( uidsArray ) == 0 ) )
			throw( eDSAuthFailed );

		siResult = eDSAuthFailed;

		if ( dsIsUserMemberOfGroup(rootName, "admin") == false )
			throw( eDSPermissionError );

		CFArrayRef passwordsArray = (CFArrayRef)CFDictionaryGetValue( rootDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ) );
		if ( ( passwordsArray == NULL ) || ( CFArrayGetCount( passwordsArray ) == 0 ) )
			pwdFromRecord = (char*)""; // empty string, we are not freeing it so direct assignment is OK
		else
		{
			CFStringRef pwdFromRecordCFStr = (CFStringRef)CFArrayGetValueAtIndex( passwordsArray, 0 );
			pwdFromRecord = CStrFromCFString( pwdFromRecordCFStr, &cStr, &cStrSize );
		}

		//account for the case where pwdFromRecord == "" such that we will auth if pwdLen is 0
		if (::strcmp(pwdFromRecord,"") != 0)
		{
			salt[ 0 ] = pwdFromRecord[0];
			salt[ 1 ] = pwdFromRecord[1];
			salt[ 2 ] = '\0';

			bzero(hashPwd, 32);
			::strcpy( hashPwd, ::crypt( rootPwd, salt ) );

			siResult = eDSAuthFailed;
			if ( ::strcmp( hashPwd, pwdFromRecord ) == 0 )
			{
				siResult = eDSNoErr;
			}
			bzero(hashPwd, 32);
		}
		else // pwdFromRecord is == ""
		{
			if (::strcmp(rootPwd,"") != 0)
			{
				siResult = eDSNoErr;
			}
		}

		if (siResult == eDSNoErr)
		{
			if ( inNode->IsLocalNode() ) //local node and assume crypt password length is correct
			{
				if ( CDSLocalAuthHelper::MigrateToShadowHash( inNodeRef, inPlugin, inNode, inMutableRecordDict,
							userName, userPwd, &bResetCache, inHashList, inNativeRecType ) == eDSNoErr )
				{
					// Add principal to local KDC
					AddKerberosAuthAuthority(
						inNodeRef, inParams.pUserName, kDSTagAuthAuthorityKerberosv5, inAuthAuthorityList, inMutableRecordDict,
						inPlugin, inNode, inNativeRecType, true );
					
					// update Kerberos
					siResult = CDSLocalAuthHelper::DoKerberosAuth( inNodeRef, inParams, inOutContinueData, inAuthData,
									outAuthData, inAuthOnly, isSecondary, inAuthAuthorityList, inGUIDString, inAuthedUserIsAdmin,
									inMutableRecordDict, inHashList, inPlugin, inNode, inAuthedUserName, inUID, inEffectiveUID,
									inNativeRecType );
				}
			}
			else
			{
				// we successfully authenticated with root password, now set new user password.
				//set with the new password
				const char *saltchars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

				bzero(hashPwd, 32);

				if ( strlen(userPwd) > 0 )
				{
					// only need crypt if password is not empty
					salt[0] = saltchars[arc4random() % 64];
					salt[1] = saltchars[arc4random() % 64];
					salt[2] = '\0';

					::strcpy( hashPwd, ::crypt( userPwd, salt ) );
				}

				CFArrayRef shortNames = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict,
					inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrRecordName ) ) );
				userNameCFStr = CFStringCreateWithCString( NULL, userName, kCFStringEncodingUTF8 );

				if ( !CFArrayContainsValue( shortNames, ::CFRangeMake( 0, CFArrayGetCount( shortNames ) ),
					userNameCFStr ) )
#ifdef DEBUG
					throw( eDSAuthUnknownUser );
#else
					throw( eDSAuthFailed );
#endif

				// change the password attribute in the record
				siResult = inPlugin->OpenRecord( inPlugin->RecordStandardTypeForNativeType( inNativeRecType ),
					userNameCFStr, &recordRef );
				if ( siResult != eDSNoErr )
					throw( siResult );

				inPlugin->RemoveAttribute( recordRef, CFSTR( kDS1AttrPassword ) );
				
				hashPwdCFStr = CFStringCreateWithCString( NULL, hashPwd, kCFStringEncodingUTF8 );
				siResult = inPlugin->AddAttribute( recordRef, CFSTR( kDS1AttrPassword ), hashPwdCFStr );
				if ( siResult != eDSNoErr )
					throw( siResult );
				
				// now change the dciotnary that's being passed around
				CFDictionaryRemoveValue( inMutableRecordDict,
					inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ) );
				CFMutableArrayRef mutableValuesArray = CFArrayCreateMutable( kCFAllocatorDefault, 0,
					&kCFTypeArrayCallBacks );
				CFArrayAppendValue( mutableValuesArray, hashPwdCFStr );
				CFDictionaryAddValue( inMutableRecordDict,
					inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ), mutableValuesArray );
				CFRelease( mutableValuesArray );
				
// "TODO: need to zero out hashPwdCFStr with a custom allocator / deallocator"
				bzero(hashPwd, 32);
			}
		}
	}
	catch( tDirStatus err )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::DoSetPassword(): got error %d", err );
		siResult = err;
	}

	if ( dataList != NULL )
	{
		dsDataListDeallocatePriv(dataList);
		free(dataList);
	}
	if ( recordsArray != NULL )
		CFRelease( recordsArray );
	if ( cStr != NULL )
		::free( cStr );
	
	DSCFRelease( userNameCFStr );
	DSCFRelease( hashPwdCFStr );
	if ( recordRef != 0 ) {
		DbgLog( kLogDebug, "CDSLocalAuthHelper::DoSetPassword - closing internal record reference" );
		gRefTable.RemoveReference( recordRef, eRefTypeRecord, 0, 0 ); // remove closes the record automatically
		recordRef = 0;
	}
	DSFreeString( userName );
	DSFreePassword( userPwd );
	DSFreeString( rootName );
	DSFreePassword( rootPwd );

	return( siResult );
}

//------------------------------------------------------------------------------------
//	* DoSetPasswordAsRoot
//------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::DoSetPasswordAsRoot( tDirNodeReference inNodeRef, CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData, tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary, CAuthAuthority &inAuthAuthorityList,
	const char* inAuthAuthorityData, const char* inGUIDString, bool inAuthedUserIsAdmin, 
	CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
	CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
	uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType )
{
	tDirStatus			siResult		= eDSAuthFailed;
	bool				bResetCache		= false;
	char			   *userName		= NULL;
	char			   *newPasswd		= NULL;
	CFStringRef			userNameCFStr	= NULL;
	tRecordReference	recordRef		= 0;
	CFStringRef			hashPwdCFStr	= NULL;
	tDataListPtr		dataList		= NULL;
	unsigned int		itemCount		= 0;

	try
	{
		if ( inAuthData == NULL )
			throw( eDSNullAuthStepData );

		siResult = (tDirStatus)Get2FromBuffer( inAuthData, &dataList, &userName, &newPasswd, &itemCount );
		if ( siResult != eDSNoErr )
			throw( siResult );
		if ( itemCount != 2 || userName == NULL || DSIsStringEmpty(userName) || newPasswd == NULL )
			throw( eDSInvalidBuffFormat );
		
		userNameCFStr = CFStringCreateWithCString( kCFAllocatorDefault, userName, kCFStringEncodingUTF8 );
		
		// allow root to change anyone's password and
		// others to change their own password
		if ( inAuthedUserName != NULL && inParams.pUserName != NULL )
		{
			CFStringRef tempUserName = CFStringCreateWithCString( NULL, inParams.pUserName, kCFStringEncodingUTF8 );
			if ( CFStringCompare(tempUserName, inAuthedUserName, 0) == kCFCompareEqualTo ) {
				siResult = eDSNoErr;
			}
			CFRelease( tempUserName );
		}
		
		if ( siResult != eDSNoErr ) {
			bool accessAllowed = inNode->AccessAllowed( inAuthedUserName, inEffectiveUID, inNativeRecType, 
													   inPlugin->AttrNativeTypeForStandardType(CFSTR(kDS1AttrPassword)), 
													   eDSAccessModeWriteAttr );
			if ( accessAllowed == false ) {
				throw eDSPermissionError;
			}
		}
				
		//need some code here to directly set the NI Crypt password
		//also first check to see if the password already exists
		char			salt[3];
		char			hashPwd[ 32 ];

		//need some code here to directly change the NI Crypt password
		//first check to see that the old password is valid
		DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoSetPasswordAsRoot():Attempting password change" );

		CFArrayRef shortNames = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrRecordName ) ) );

		if ( !CFArrayContainsValue(shortNames, CFRangeMake(0, CFArrayGetCount(shortNames)), userNameCFStr) )
#ifdef DEBUG
			throw( eDSAuthUnknownUser );
#else
			throw( eDSAuthFailed );
#endif

		siResult = eDSNoErr;
		
		if (siResult == eDSNoErr)
		{
			if ( inNode->IsLocalNode() ) //local node and assume crypt password length is correct
			{
				siResult = CDSLocalAuthHelper::MigrateToShadowHash( inNodeRef, inPlugin, inNode, inMutableRecordDict,
					userName, newPasswd, &bResetCache, inHashList, inNativeRecType );
					
				if ( siResult == eDSNoErr )
				{
					// reload the edited AuthenticationAuthority list
					siResult = inPlugin->OpenRecord( inPlugin->RecordStandardTypeForNativeType(inNativeRecType),
													userNameCFStr, &recordRef );
					if ( siResult != eDSNoErr )
						throw( siResult );
					
					CAuthAuthority aaTank;
					LoadAuthAuthorities( inPlugin, recordRef, aaTank );
					
					// Add principal to local KDC
					AddKerberosAuthAuthority(
						inNodeRef, inParams.pUserName, kDSTagAuthAuthorityKerberosv5, aaTank, inMutableRecordDict,
						inPlugin, inNode, inNativeRecType, true );
					
					// update Kerberos
					siResult = CDSLocalAuthHelper::DoKerberosAuth( inNodeRef, inParams, inOutContinueData, inAuthData,
									outAuthData, inAuthOnly, isSecondary, aaTank, inGUIDString, inAuthedUserIsAdmin,
									inMutableRecordDict, inHashList, inPlugin, inNode, inAuthedUserName, inUID, inEffectiveUID,
									inNativeRecType );
				}
			}
			else
			{
				// we successfully authenticated with old password, now change to new password.
				//set with the new password
				const char *saltchars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

				bzero(hashPwd, 32);

				if ( strlen(newPasswd) > 0 )
				{
					// only need crypt if password is not empty
					salt[0] = saltchars[arc4random() % 64];
					salt[1] = saltchars[arc4random() % 64];
					salt[2] = '\0';

					::strcpy( hashPwd, ::crypt( newPasswd, salt ) );
				}
				// change the password attribute in the record
				if ( recordRef == 0 )
				{
					siResult = inPlugin->OpenRecord( inPlugin->RecordStandardTypeForNativeType( inNativeRecType ),
													userNameCFStr, &recordRef );
					if ( siResult != eDSNoErr )
						throw( siResult );
				}

				inPlugin->RemoveAttribute( recordRef, CFSTR( kDS1AttrPassword ) );
				
				hashPwdCFStr = CFStringCreateWithCString( NULL, hashPwd, kCFStringEncodingUTF8 );
				siResult = inPlugin->AddAttribute( recordRef, CFSTR( kDS1AttrPassword ), hashPwdCFStr );
				if ( siResult != eDSNoErr )
					throw( siResult );
				
				// now change the dciotnary that's being passed around
				CFDictionaryRemoveValue( inMutableRecordDict,
					inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ) );
				CFMutableArrayRef mutableValuesArray = CFArrayCreateMutable( kCFAllocatorDefault, 0,
					&kCFTypeArrayCallBacks );
				CFArrayAppendValue( mutableValuesArray, hashPwdCFStr );
				CFDictionaryAddValue( inMutableRecordDict,
					inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ), mutableValuesArray );
				CFRelease( mutableValuesArray );

// "TODO: need to zero out hashPwdCFStr with a custom allocator / deallocator"
				bzero(hashPwd, 32);
			}
		}
	}
	catch( tDirStatus err )
	{
		DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoSetPasswordAsRoot(): got error %d", err );
		siResult = err;
	}
	
	if ( dataList != NULL )
	{
		dsDataListDeallocatePriv(dataList);
		free(dataList);
	}
	if ( userNameCFStr != NULL )
		CFRelease( userNameCFStr );
	if ( hashPwdCFStr != NULL )
		CFRelease( hashPwdCFStr );
	if ( recordRef != 0 ) {
		DbgLog( kLogDebug, "CDSLocalAuthHelper::DoSetPasswordAsRoot - closing internal record reference" );
		gRefTable.RemoveReference( recordRef, eRefTypeRecord, 0, 0 ); // remove closes the record automatically
		recordRef = 0;
	}
	
	DSFreeString( userName );
	DSFreePassword( newPasswd );
	
	return( siResult );
} // DoSetPasswordAsRoot


tDirStatus CDSLocalAuthHelper::DoChangePassword( tDirNodeReference inNodeRef, CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData, tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
	const char* inAuthAuthorityData, const char* inGUIDString, bool inAuthedUserIsAdmin, 
	CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
	CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
	uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType )
{
	tDirStatus			siResult		= eDSAuthFailed;
	bool				bResetCache		= false;
	char				*name			= NULL;
	char				*oldPwd			= NULL;
	char				*newPwd			= NULL;
	CFStringRef			userNameCFStr	= NULL;
	tRecordReference	recordRef		= 0;
	CFStringRef			hashPwdCFStr	= NULL;
	char*				cStr			= NULL;
	size_t				cStrSize		= 0;
	tDataListPtr		dataList		= NULL;
	unsigned int		itemCount		= 0;
	
	try
	{
		if ( inAuthData == NULL )
			throw( eDSNullAuthStepData );

		siResult = (tDirStatus)Get2FromBuffer( inAuthData, &dataList, &name, &oldPwd, &itemCount );
		if ( siResult != eDSNoErr )
			throw( siResult );
		if ( itemCount != 3 || name == NULL || DSIsStringEmpty(name) || oldPwd == NULL )
			throw( eDSInvalidBuffFormat );

		newPwd = dsDataListGetNodeStringPriv( dataList, 3 );
		if ( newPwd == NULL )
			throw( eDSInvalidBuffFormat );
		
		char			salt[3];
		char			hashPwd[ 32 ];
		const char	   *pwdFromRecord			= NULL;
		
		userNameCFStr = CFStringCreateWithCString( NULL, name, kCFStringEncodingUTF8 );
		CFArrayRef userNamesFromRecord = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrRecordName ) ) );

		if ( !CFArrayContainsValue(userNamesFromRecord, CFRangeMake(0, CFArrayGetCount(userNamesFromRecord)),
				userNameCFStr) )
#ifdef DEBUG
			throw( eDSAuthUnknownUser );
#else
			throw( eDSAuthFailed );
#endif
		
		CFArrayRef passwordValues = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ) );
		if ( ( passwordValues == NULL ) || ( CFArrayGetCount( passwordValues ) == 0 ) )
			pwdFromRecord = (char*)""; // empty string, we are not freeing it so direct assignment is OK
		else
		{
			CFStringRef passwordAttrValue = (CFStringRef)CFArrayGetValueAtIndex( passwordValues, 0 );
			pwdFromRecord = CStrFromCFString( passwordAttrValue, &cStr, &cStrSize );
		}


		siResult = eDSNoErr;

	
		if ( !isSecondary )
		{
			//if another auth authority has successfully changed the password
			//assume we have out of sync passwords and allow the change
			//account for the case where pwdFromRecord == "" such that we will auth if pwdLen is 0
			siResult = eDSAuthFailed;
			if (::strcmp(pwdFromRecord,"") != 0)
			{
				salt[ 0 ] = pwdFromRecord[0];
				salt[ 1 ] = pwdFromRecord[1];
				salt[ 2 ] = '\0';

				bzero(hashPwd, 32);
				::strcpy( hashPwd, ::crypt( oldPwd, salt ) );

				if ( ::strcmp( hashPwd, pwdFromRecord ) == 0 )
				{
					siResult = eDSNoErr;
				}
				bzero(hashPwd, 32);
			}
			else // pwdFromRecord is == ""
			{
				if (::strcmp(oldPwd,"") == 0)
				{
					siResult = eDSNoErr;
				}
			}
		}

		if (siResult == eDSNoErr)
		{
			if ( inNode->IsLocalNode() ) //local node and assume crypt password length is correct
			{
				CDSLocalAuthHelper::MigrateToShadowHash( inNodeRef, inPlugin, inNode, inMutableRecordDict, name, newPwd,
					&bResetCache, inHashList, inNativeRecType );
			}
			else
			{
				// we successfully authenticated with old password, now change to new password.
				//set with the new password
				const char *saltchars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

				bzero(hashPwd, 32);

				if ( strlen(newPwd) > 0 )
				{
					// only need crypt if password is not empty
					salt[0] = saltchars[arc4random() % 64];
					salt[1] = saltchars[arc4random() % 64];
					salt[2] = '\0';

					::strcpy( hashPwd, ::crypt( newPwd, salt ) );
				}
				// change the password attribute in the record
				siResult = inPlugin->OpenRecord( inPlugin->RecordStandardTypeForNativeType( inNativeRecType ),
					userNameCFStr, &recordRef );
				if ( siResult != eDSNoErr )
					throw( siResult );

				inPlugin->RemoveAttribute( recordRef, CFSTR( kDS1AttrPassword ) );
				
				hashPwdCFStr = CFStringCreateWithCString( NULL, hashPwd, kCFStringEncodingUTF8 );
				siResult = inPlugin->AddAttribute( recordRef, CFSTR( kDS1AttrPassword ), hashPwdCFStr );
				if ( siResult != eDSNoErr )
					throw( siResult );
				
				// now change the dciotnary that's being passed around
				CFDictionaryRemoveValue( inMutableRecordDict,
					inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ) );
				CFMutableArrayRef mutableValuesArray = CFArrayCreateMutable( kCFAllocatorDefault, 0,
					&kCFTypeArrayCallBacks );
				CFArrayAppendValue( mutableValuesArray, hashPwdCFStr );
				CFDictionaryAddValue( inMutableRecordDict,
					inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ), mutableValuesArray );
				CFRelease( mutableValuesArray );

// "TODO: need to zero out hashPwdCFStr with a custom allocator / deallocator"
				bzero(hashPwd, 32);
			}
		}
	}

	catch( tDirStatus err )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::DoChangePassword(): got error %d", err );
		siResult = err;
	}
	
	if ( dataList != NULL )
	{
		dsDataListDeallocatePriv(dataList);
		free(dataList);
	}
	if ( userNameCFStr != NULL )
		CFRelease( userNameCFStr );

	if ( hashPwdCFStr != NULL )
		CFRelease( hashPwdCFStr );
	if ( recordRef != 0 ) {
		DbgLog( kLogDebug, "CDSLocalAuthHelper::DoChangePassword - closing internal record reference" );
		gRefTable.RemoveReference( recordRef, eRefTypeRecord, 0, 0 ); // remove closes the record automatically
		recordRef = 0;
	}

	if ( cStr != NULL )
		::free( cStr );

	if ( name != NULL )
	{
		free( name );
		name = NULL;
	}

	if ( newPwd != NULL )
	{
		bzero(newPwd, strlen(newPwd));
		free( newPwd );
		newPwd = NULL;
	}

	if ( oldPwd != NULL )
	{
		bzero(oldPwd, strlen(oldPwd));
		free( oldPwd );
		oldPwd = NULL;
	}

	return( siResult );

} // DoChangePassword


//------------------------------------------------------------------------------------
//	* FindNodeForSearchPolicyAuthUser
//------------------------------------------------------------------------------------

tDataList* CDSLocalAuthHelper::FindNodeForSearchPolicyAuthUser ( const char *userName )
{
	tDirStatus				siResult		= eDSNoErr;
	tDirStatus				returnVal		= (tDirStatus)-3;
	UInt32					nodeCount		= 0;
	UInt32					recCount		= 0;
	tContextData			context			= 0;
	tDataListPtr			nodeName		= NULL;
	tDataListPtr			recName			= NULL;
	tDataListPtr			recType			= NULL;
	tDataListPtr			attrTypes		= NULL;
	tDataBufferPtr			dataBuff		= NULL;
	tRecordEntry		   *pRecEntry		= NULL;
	tAttributeListRef		attrListRef		= 0;
	tAttributeValueListRef	valueRef		= 0;
	tAttributeEntry		   *pAttrEntry		= NULL;
	tAttributeValueEntry   *pValueEntry		= NULL;
	tDirReference			aDSRef			= 0;
	tDirNodeReference		aSearchNodeRef	= 0;
	
	try
	{
		siResult = dsOpenDirService( &aDSRef );
		if ( siResult != eDSNoErr ) throw( siResult );

		dataBuff = dsDataBufferAllocate( aDSRef, 2048 );
		if ( dataBuff == NULL ) throw( eMemoryAllocError );
		
		siResult = dsFindDirNodes( aDSRef, dataBuff, NULL, 
									eDSAuthenticationSearchNodeName, &nodeCount, &context );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = dsGetDirNodeName( aDSRef, dataBuff, 1, &nodeName );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = dsOpenDirNode( aDSRef, nodeName, &aSearchNodeRef );
		if ( siResult != eDSNoErr ) throw( siResult );
		if ( nodeName != NULL )
		{
			dsDataListDeallocate( aDSRef, nodeName );
			free( nodeName );
			nodeName = NULL;
		}

		recName		= dsBuildListFromStrings( aDSRef, userName, NULL );
		recType		= dsBuildListFromStrings( aDSRef, kDSStdRecordTypeUsers, NULL );
		attrTypes   = dsBuildListFromStrings( aDSRef, kDSNAttrMetaNodeLocation, NULL );

		recCount = 1; // only care about first match
		do 
		{
			siResult = dsGetRecordList( aSearchNodeRef, dataBuff, recName, eDSExact, recType,
										attrTypes, false, &recCount, &context);
			if (siResult == eDSBufferTooSmall)
			{
				UInt32 bufSize = dataBuff->fBufferSize;
				dsDataBufferDeallocatePriv( dataBuff );
				dataBuff = NULL;
				dataBuff = ::dsDataBufferAllocate( aDSRef, bufSize * 2 );
			}
		} while ( (siResult == eDSBufferTooSmall) || ( (siResult == eDSNoErr) && (recCount == 0) && (context != 0) ) );
		//worry about multiple calls (ie. continue data) since we continue until first match or no more searching
		
		if ( (siResult == eDSNoErr) && (recCount > 0) )
		{
			siResult = ::dsGetRecordEntry( aSearchNodeRef, dataBuff, 1, &attrListRef, &pRecEntry );
			if ( (siResult == eDSNoErr) && (pRecEntry != NULL) )
			{
				//index starts at one - should have one entry
				for (unsigned int i = 1; i <= pRecEntry->fRecordAttributeCount; i++)
				{
					siResult = ::dsGetAttributeEntry( aSearchNodeRef, dataBuff, attrListRef, i, &valueRef, &pAttrEntry );
					//need to have at least one value - get first only
					if ( ( siResult == eDSNoErr ) && ( pAttrEntry->fAttributeValueCount > 0 ) )
					{
						// Get the first attribute value
						siResult = ::dsGetAttributeValue( aSearchNodeRef, dataBuff, 1, valueRef, &pValueEntry );
						// Is it what we expected
						if ( ::strcmp( pAttrEntry->fAttributeSignature.fBufferData, kDSNAttrMetaNodeLocation ) == 0 )
						{
							nodeName = dsBuildFromPath( aDSRef, pValueEntry->fAttributeValueData.fBufferData, "/" );
						}
						if ( pValueEntry != NULL )
						{
							dsDeallocAttributeValueEntry( aDSRef, pValueEntry );
							pValueEntry = NULL;
						}
					}
					dsCloseAttributeValueList(valueRef);
					if (pAttrEntry != NULL)
					{
						dsDeallocAttributeEntry(aDSRef, pAttrEntry);
						pAttrEntry = NULL;
					}
				} //loop over attrs requested
			}//found 1st record entry
			dsCloseAttributeList(attrListRef);
			if (pRecEntry != NULL)
			{
				dsDeallocRecordEntry(aDSRef, pRecEntry);
				pRecEntry = NULL;
			}
		}// got records returned
	}
	
	catch( tDirStatus err )
	{
		returnVal = err;
	}	
	
	if ( recName != NULL )
	{
		dsDataListDeallocate( aDSRef, recName );
		free( recName );
		recName = NULL;
	}
	if ( recType != NULL )
	{
		dsDataListDeallocate( aDSRef, recType );
		free( recType );
		recType = NULL;
	}
	if ( attrTypes != NULL )
	{
		dsDataListDeallocate( aDSRef, attrTypes );
		free( attrTypes );
		attrTypes = NULL;
	}
	if ( dataBuff != NULL )
	{
		dsDataBufferDeAllocate( aDSRef, dataBuff );
		dataBuff = NULL;
	}
	if ( aSearchNodeRef != 0 )
	{
		dsCloseDirNode( aSearchNodeRef );
		aSearchNodeRef = 0;
	}
	if ( aDSRef != 0 )
	{
		dsCloseDirService( aDSRef );
		aDSRef = 0;
	}
	
	return nodeName;
} // FindNodeForSearchPolicyAuthUser


//------------------------------------------------------------------------------------
//	* IsWriteAuthRequest
//------------------------------------------------------------------------------------

bool CDSLocalAuthHelper::IsWriteAuthRequest ( UInt32 uiAuthMethod )
{
	switch ( uiAuthMethod )
	{
		case kAuthSetPasswd:
		case kAuthSetPasswdAsRoot:
		case kAuthChangePasswd:
		case kAuthSetGlobalPolicy:
		case kAuthSetPolicy:
		case kAuthSetCertificateHashAsRoot:
			return true;
			
		default:
			return false;
	}
} // IsWriteAuthRequest

// ---------------------------------------------------------------------------
//	* GetAuthAuthorityHandler
// ---------------------------------------------------------------------------

AuthAuthorityHandlerProc CDSLocalAuthHelper::GetAuthAuthorityHandler ( const char* inTag )
{
	if (inTag == NULL)
		return NULL;
	
	for (unsigned int i = 0; sAuthAuthorityHandlerProcs[i].fTag != NULL; ++i)
	{
		if (strcasecmp( inTag, sAuthAuthorityHandlerProcs[i].fTag) == 0)
		{
			// found it
			return sAuthAuthorityHandlerProcs[i].fHandler;
		}
	}
	return NULL;
}

#pragma mark -- Private methods ---

//--------------------------------------------------------------------------------------------------
// * SetUserPolicies
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::SetUserPolicies( CFMutableDictionaryRef inMutableRecordDict, CDSLocalPlugin* inPlugin,
	CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName, uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType,
	const char *inUsername, const char *inPolicyStr, bool inAuthedUserIsAdmin, tDirNodeReference inNodeRef,
	sHashState *inOutHashState )
{
	tDirStatus siResult = eDSAuthFailed;
	char* currentPolicyStr = NULL;
	char* newPassRequiredStr = NULL;
	PWAccessFeatures access = {0};
	PWMoreAccessFeatures moreAccess = {0};
	CFStringRef recordName = NULL;
	CFStringRef xmlDataCFStr = NULL;
	tRecordReference recordRef = 0;
	char *xmlDataStr = NULL;
	
	try
	{
		if ( inPolicyStr == NULL || inUsername == NULL )
			throw( eDSAuthFailed );
		
		if ( (inEffectiveUID != 0) && !inAuthedUserIsAdmin )
			throw( eDSPermissionError );
		
		// special case for newPasswordRequired because it is stored in the state file
		if ( inOutHashState != NULL && (newPassRequiredStr = strstr(inPolicyStr, kPWPolicyStr_newPasswordRequired)) != NULL )
		{
			newPassRequiredStr += sizeof(kPWPolicyStr_newPasswordRequired) - 1;
			if ( (*newPassRequiredStr == '=') &&
				 (*(newPassRequiredStr + 1) == '0' || *(newPassRequiredStr + 1) == '1') )
			{
				inOutHashState->newPasswordRequired = *(newPassRequiredStr + 1) - '0';
			}
		}
		
		// policies that go in the user record
		GetUserPolicies( inMutableRecordDict, NULL, inPlugin, &currentPolicyStr );
		CFStringRef stdRecType = inPlugin->RecordStandardTypeForNativeType( inNativeRecType );
		recordName = CFStringCreateWithCString( NULL, inUsername, kCFStringEncodingUTF8 );

		bool accessAllowed = inNode->AccessAllowed( inAuthedUserName, inEffectiveUID, inNativeRecType, inPlugin->AttrNativeTypeForStandardType(CFSTR(kDS1AttrPasswordPolicyOptions)), eDSAccessModeWriteAttr );
		if ( !accessAllowed )
			throw( eDSPermissionError );
		
		{
			char policyStr[2048];
			
			::GetDefaultUserPolicies( &access );
			if ( currentPolicyStr != NULL )
				::StringToPWAccessFeaturesExtra( currentPolicyStr, &access, &moreAccess );
			::StringToPWAccessFeaturesExtra( inPolicyStr, &access, &moreAccess );
			::PWAccessFeaturesToStringWithoutStateInfoExtra( &access, &moreAccess, sizeof(policyStr), policyStr );
			::pwsf_PreserveUnrepresentedPolicies( inPolicyStr, sizeof(policyStr), policyStr );
			if ( ::ConvertSpaceDelimitedPolicyToXML( policyStr, &xmlDataStr ) == 0 )
			{
				siResult = inPlugin->OpenRecord( stdRecType, recordName, &recordRef );
				if ( siResult != eDSNoErr )
					throw( siResult );
				siResult = inPlugin->RemoveAttribute( recordRef, CFSTR( kDS1AttrPasswordPolicyOptions ) );
				xmlDataCFStr = CFStringCreateWithCString( NULL, xmlDataStr, kCFStringEncodingUTF8 );

				siResult = inPlugin->AddAttribute( recordRef, CFSTR( kDS1AttrPasswordPolicyOptions ), xmlDataCFStr );
				if ( siResult != eDSNoErr )
					throw( siResult );

				// set the corresponding Kerberos policy for the local realm
				CAuthAuthority aaTank;
				LoadAuthAuthorities( inPlugin, recordRef, aaTank );
				char *princStr = aaTank.GetDataForTag( kDSTagAuthAuthorityKerberosv5, 1 );
				if ( princStr != NULL )
				{
					char *realmStr = princStr;
					strsep( &realmStr, "@" );
					if ( realmStr != NULL )
					{
						char *buff = NULL;
						size_t buffLen = 0;
						pwsf_ModifyPrincipalInLocalRealm( princStr, realmStr, &access, access.maxMinutesUntilChangePassword,
							&buff, &buffLen );
					}
					
					free( princStr );
				}
				
				// now massage the cached copy of the record dict to match the one we just wrote to the directory
				CFStringRef nativeAttrType =
					inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPasswordPolicyOptions ) );
				CFDictionaryRemoveValue( inMutableRecordDict, nativeAttrType );
				
				CFArrayRef values = CFArrayCreate( NULL, (const void**)&xmlDataCFStr, 1, &kCFTypeArrayCallBacks );
				CFDictionaryAddValue( inMutableRecordDict, nativeAttrType, values );
				
				CFRelease( values );
			}
		}
	}

	catch( tDirStatus err )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::SetUserPolicies(): got error %d", err );
		siResult = err;
	}

	DSFreeString( xmlDataStr );
	if ( recordRef != 0 ) {
		DbgLog( kLogDebug, "CDSLocalAuthHelper::SetUserPolicies - closing internal record reference" );
		gRefTable.RemoveReference( recordRef, eRefTypeRecord, 0, 0 ); // remove closes the record automatically
		recordRef = 0;
	}
	if ( recordName != NULL )
		CFRelease( recordName );
	if ( xmlDataCFStr != NULL )
		CFRelease( xmlDataCFStr );
	if ( currentPolicyStr != NULL )
		::free( currentPolicyStr );

	return( siResult );

} // SetUserPolicies

//--------------------------------------------------------------------------------------------------
// * SetUserAAtoDisabled
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::SetUserAAtoDisabled( CFMutableDictionaryRef inMutableRecordDict,
	CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inStdRecType, const char *inUsername,
	tDirNodeReference inNodeRef, CFStringRef inAuthedUserName, uid_t inEffectiveUID )
{
	tDirStatus siResult = eDSAuthFailed;
	CFStringRef authAuthority = NULL;
	tRecordReference recordRef = 0;
	CFStringRef recordName = NULL;
	CAuthAuthority aaTank;
	
	try
	{
		CFArrayRef values = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrAuthenticationAuthority ) ) );
		if ( ( values != NULL ) && ( CFArrayGetCount( values ) > 0 ) )
			authAuthority = (CFStringRef)CFArrayGetValueAtIndex( values, 0 );
		
		if ( authAuthority != NULL && ( CFStringGetLength( authAuthority ) > 0 ) )
			authAuthority = CFStringCreateWithFormat( NULL, 0, CFSTR( kDSValueAuthAuthorityDisabledUser"%@" ),
				authAuthority );
		else
			authAuthority = CFSTR( kDSValueAuthAuthorityDisabledUser kDSValueAuthAuthorityShadowHash );
		
		recordName = CFStringCreateWithCString( NULL, inUsername, kCFStringEncodingUTF8 );
		siResult = inPlugin->OpenRecord( inStdRecType, recordName, &recordRef );
		if ( siResult != eDSNoErr )
			throw( siResult );
		
		LoadAuthAuthorities( inPlugin, recordRef, aaTank );

		if ( aaTank.GetValueForTagAsCFDict(kDSTagAuthAuthorityLocalCachedUser) != NULL )
		{
			// remove the shadowhash authority from the list (we put it there to
			// forward the handling from the DoLocalCachedUserAuth handler),
			// and disable the original authority
			aaTank.RemoveValueForTag( kDSTagAuthAuthorityShadowHash );
			aaTank.RemoveValueForTag( kDSTagAuthAuthorityDisabledUser );
			aaTank.SetValueDisabledForTag( kDSTagAuthAuthorityLocalCachedUser );
		}
		else
		{
			aaTank.SetValueDisabledForTag( kDSTagAuthAuthorityShadowHash );
		}
		
		siResult = SaveAuthAuthoritiesWithRecordRef( inPlugin, inNodeRef, recordRef, aaTank );
	}
	
	catch( tDirStatus err )
	{
		if ( err != eDSNoErr )
			DbgLog(  kLogPlugin, "DSLocalAuthHelper::SetUserAAToDisabled(): got error %d", err );
		siResult = err;
	}

	if ( recordRef != 0 ) {
		DbgLog( kLogDebug, "CDSLocalAuthHelper::SetUserAAtoDisabled - closing internal record reference" );
		gRefTable.RemoveReference( recordRef, eRefTypeRecord, 0, 0 ); // remove closes the record automatically
		recordRef = 0;
	}
	if ( authAuthority != NULL )
		CFRelease( authAuthority );
	if ( recordName != NULL )
		CFRelease( recordName );
	
	return( siResult );

} // SetUserAAtoDisabled


//--------------------------------------------------------------------------------------------------
// * SetUserAuthAuthorityAsRoot
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::SetUserAuthAuthorityAsRoot(	CFMutableDictionaryRef inMutableRecordDict,
															CDSLocalPlugin* inPlugin,
															CDSLocalPluginNode* inNode,
															CFStringRef inStdRecType,
															const char *inUsername,
															CAuthAuthority &inAuthAuthorityList,
															tDirNodeReference inNodeRef )
{
	tDirStatus siResult = eDSAuthFailed;
	tRecordReference recordRef = 0;
	CFStringRef authAuthorityCFStr = NULL;
	UInt32 avIndex = 0;
	UInt32 avCount = 0;
	char *aaStr = NULL;
	CFMutableArrayRef aaValueArray = NULL;
	
	try
	{
		CFStringRef recordName = CFStringCreateWithCString( kCFAllocatorDefault, inUsername, kCFStringEncodingUTF8 );
		siResult = inPlugin->OpenRecord( inStdRecType, recordName, &recordRef );
		CFRelease( recordName );
		if ( siResult != eDSNoErr )
			throw( siResult );
		
		siResult = SaveAuthAuthoritiesWithRecordRef( inPlugin, inNodeRef, recordRef, inAuthAuthorityList );
		if ( siResult != eDSNoErr )
			throw( siResult );
		
		// set the in-memory copy to what we set on disk
		aaValueArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
		if ( aaValueArray == NULL )
			throw( eMemoryError );
		
		avCount = inAuthAuthorityList.GetValueCount();
		for ( avIndex = 0; avIndex < avCount; avIndex++ )
		{
			aaStr = inAuthAuthorityList.GetValueAtIndex( avIndex );
			if ( aaStr != NULL )
			{
				authAuthorityCFStr = CFStringCreateWithCString( kCFAllocatorDefault, aaStr, kCFStringEncodingUTF8 );
				if ( authAuthorityCFStr == NULL )
					throw( eMemoryError );
				
				CFArrayAppendValue( aaValueArray, authAuthorityCFStr );
				
				if ( authAuthorityCFStr != NULL ) {
					CFRelease( authAuthorityCFStr );
					authAuthorityCFStr = NULL;
				}
				
				DSFreeString( aaStr );
			}
		}
		
		CFStringRef nativeAttrType = inPlugin->AttrNativeTypeForStandardType( CFSTR(kDSNAttrAuthenticationAuthority) );
		if ( nativeAttrType != NULL )
			CFDictionarySetValue( inMutableRecordDict, nativeAttrType, aaValueArray );		
	}
	catch( tDirStatus err )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::SetUserAuthAuthorityAsRoot(): got error %d", err );
		siResult = err;
	}
	
	if ( recordRef != 0 )
	{
		// force flush and close our ref
		sFlushRecord params = { kFlushRecord, 0, recordRef };
		inPlugin->FlushRecord( &params );
		DbgLog( kLogDebug, "CDSLocalAuthHelper::SetUserAuthAuthorityAsRoot - closing internal record reference" );
		gRefTable.RemoveReference( recordRef, eRefTypeRecord, 0, 0 ); // remove closes the record automatically
		recordRef = 0;
	}
	
	if ( authAuthorityCFStr != NULL )
		CFRelease( authAuthorityCFStr );
	if ( aaValueArray != NULL )
		CFRelease( aaValueArray );
	
	return( siResult );

} // SetUserAuthAuthorityAsRoot


tDirStatus CDSLocalAuthHelper::GetHashSecurityLevelForUser( const char *inHashList, unsigned int *outHashList )
{
	char				*hashListStr			= NULL;
	char				*hashListPtr			= NULL;
	char				*hashTypeStr			= NULL;
	char				*endPtr					= NULL;
	
	if ( inHashList == NULL || outHashList == NULL )
		return eParameterError;
	
	// legacy data value (returns the default set, with NT forced on and LM forced off)
	if ( strcasecmp( inHashList, kDSTagAuthAuthorityBetterHashOnly ) == 0 )
	{
		*outHashList |= ePluginHashNT;
		*outHashList &= (0x7FFF ^ ePluginHashLM);
		
		return eDSNoErr;
	}
	else
	if ( strncasecmp( inHashList, kHashNameListPrefix, sizeof(kHashNameListPrefix)-1 ) == 0 )
	{
		// look for optional methods
		hashListPtr = hashListStr = strdup( inHashList );
		hashListPtr += sizeof(kHashNameListPrefix) - 1;
		
		// require open and close brackets
		if ( *hashListPtr++ != '<' )
			return eParameterError;
		endPtr = strchr( hashListPtr, '>' );
		if ( endPtr == NULL )
			return eParameterError;
			
		*endPtr = '\0';
		
		*outHashList = 0;
	
		// walk the list
		while ( (hashTypeStr = strsep( &hashListPtr, "," )) != NULL )
		{
			if ( CDSLocalAuthHelper::GetHashSecurityBitsForString( hashTypeStr, outHashList ) )
				break;
		}
		
		if ( hashListStr != NULL )
			free( hashListStr );
	}
	else
	{
		return eParameterError;
	}
		
	return eDSNoErr;
}


//--------------------------------------------------------------------------------------------------
// * GetHashSecurityBitsForString ()
//
//	Returns: TRUE if the caller should stop (secure mode)
//--------------------------------------------------------------------------------------------------

bool CDSLocalAuthHelper::GetHashSecurityBitsForString( const char *inHashType, unsigned int *inOutHashList )
{
	bool returnVal = false;
	
	if ( strcasecmp( inHashType, kHashNameNT ) == 0 )
		*inOutHashList |= ePluginHashNT;
	else if ( strcasecmp( inHashType, kHashNameLM ) == 0 )
		*inOutHashList |= ePluginHashLM;
	else if ( strcasecmp( inHashType, kHashNameCRAM_MD5 ) == 0 )
		*inOutHashList |= ePluginHashCRAM_MD5;
	else if ( strcasecmp( inHashType, kHashNameSHA1 ) == 0 )
		*inOutHashList |= ePluginHashSaltedSHA1;
	else if ( strcasecmp( inHashType, kHashNameRecoverable ) == 0 )
		*inOutHashList |= ePluginHashRecoverable;
	else if ( strcasecmp( inHashType, kHashNameSecure ) == 0 )
	{
		// If the secure hash is used, all other hashes are OFF
		*inOutHashList = ePluginHashSecurityTeamFavorite;
		returnVal = true;
	}
	
	return returnVal;
}


//--------------------------------------------------------------------------------------------------
// * WriteShadowHash ()
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::WriteShadowHash ( const char *inUserName, const char *inGUIDString,
	unsigned char inHashes[kHashTotalLength] )
{
	tDirStatus	result			= eDSAuthFailed;
	char	   *path			= NULL;
	char		hexHashes[kHashTotalHexLength + 1]	= { 0 };
	tDirStatus	siResult		= eDSNoErr;
	struct stat	statResult;
	CFile	  *hashFile			= NULL;
	
	try
	{
		//check to remove any old hash file
		CDSLocalAuthHelper::RemoveShadowHash( inUserName, NULL, false );
		if (inGUIDString != NULL)
		{
			path = (char*)::calloc(1, strlen(kShadowHashDirPath) + strlen(inGUIDString) + 1);
		}
		else
		{
			path = (char*)::calloc(strlen(kShadowHashDirPath) + strlen(inUserName) + 1, 1);
		}
		if ( path != NULL )
		{
			if (inGUIDString != NULL)
			{
				sprintf(path, "%s%s", kShadowHashDirPath, inGUIDString);
			}
			else
			{
				sprintf(path, "%s%s", kShadowHashDirPath, inUserName);
			}
			
			siResult = (tDirStatus)stat( "/var/db/shadow/hash", &statResult );
			if (siResult != eDSNoErr)
			{
				siResult = (tDirStatus)::stat( "/var/db/shadow", &statResult );
				//if first sub directory does not exist
				if (siResult != eDSNoErr)
				{
					::mkdir( "/var/db/shadow", 0700 );
					::chmod( "/var/db/shadow", 0700 );
				}
				siResult = (tDirStatus)::stat( "/var/db/shadow/hash", &statResult );
				//if second sub directory does not exist
				if (siResult != eDSNoErr)
				{
					::mkdir( "/var/db/shadow/hash", 0700 );
					::chmod( "/var/db/shadow/hash", 0700 );
				}
			}
			
			// CFile throws, but it is okay here
			hashFile = new CFile(path, true);
			if (hashFile->is_open())
			{
				chmod( path, 0600 ); //set root as rw only before writing data
				BinaryToHexConversion( inHashes, kHashTotalLength, hexHashes );
				hashFile->seekp( 0 ); // start at beginning
				hashFile->write( hexHashes, kHashTotalHexLength );
				delete(hashFile);
				hashFile = NULL;
				result = eDSNoErr;
			}
		}
	}
	catch( ... )
    {
        result = eDSAuthFailed;
    }
	
	if ( path != NULL ) {
		free( path );
		path = NULL;
	}
	if (hashFile != NULL)
	{
		delete(hashFile);
		hashFile = NULL;
	}

	bzero(hexHashes, kHashTotalHexLength);
	
	return( result );
} // WriteShadowHash

//--------------------------------------------------------------------------------------------------
// * RemoveShadowHash ()
//--------------------------------------------------------------------------------------------------

void CDSLocalAuthHelper::RemoveShadowHash ( const char *inUserName, const char *inGUIDString, bool bShadowToo )
{
	char   *path							= NULL;
	char	hexHashes[kHashTotalHexLength]	= { 0 };
	bool	bRemovePath						= false;
	CFile  *hashFile						= NULL;

	try
	{
		if (bShadowToo) //if flag set remove shadow file
		{
			//accept possibility that orignal file creation used only username but now we have the GUID
			//ie. don't want to cleanup in this case since we might cleanup a file that doesn't actually
			//belong to this record
			if (inGUIDString != NULL)
			{
				path = (char*)::calloc(1, strlen(kShadowHashDirPath) + strlen(inGUIDString) + 1);
			}
			else
			{
				path = (char*)::calloc(strlen(kShadowHashDirPath) + strlen(inUserName) + 1, 1);
			}
			if ( path != NULL )
			{
				if (inGUIDString != NULL)
				{
					sprintf(path, "%s%s", kShadowHashDirPath, inGUIDString);
				}
				else
				{
					sprintf(path, "%s%s", kShadowHashDirPath, inUserName);
				}
				
				// CFile throws, so we need to catch here so we can continue
				try
				{
					hashFile = new CFile(path, false); //destructor calls close
		
					if (hashFile->is_open())
					{
						hashFile->seekp( 0 ); // start at beginning
						//overwrite with zeros
						hashFile->write( hexHashes, kHashTotalHexLength );
						delete(hashFile);
						hashFile = NULL;
						bRemovePath = true;
					}
					if (bRemovePath)
					{
						//remove the file
						unlink(path);
					}
				}
				catch( ... )
				{
					
				}
				free( path );
				path = NULL;
			}
		}
		
		if (hashFile != NULL)
		{
			delete(hashFile);
			hashFile = NULL;
		}
		bRemovePath = false;
		//check always to remove the older file if present
		if (inUserName != NULL)
		{
			path = (char*)::calloc(sizeof(kShadowHashOldDirPath) + strlen(inUserName) + 1, 1);
		}
		if ( path != NULL )
		{
			sprintf(path, "%s%s", kShadowHashOldDirPath, inUserName);
			
			// this throws, but is okay because we expect the throw
			hashFile = new CFile(path, false); //destructor calls close

			if (hashFile->is_open())
			{
				hashFile->seekp( 0 ); // start at beginning
				//overwrite with zeros
				hashFile->write( hexHashes, kHashShadowBothHexLength );
				delete(hashFile);
				hashFile = NULL;
				bRemovePath = true;
			}
			if (bRemovePath)
			{
				//remove the file
				unlink(path);
			}
			free( path );
			path = NULL;
		}
	}
	catch( ... )
    {
    }
	
	if ( path != NULL ) {
		free( path );
		path = NULL;
	}
	if (hashFile != NULL)
	{
		delete(hashFile);
		hashFile = NULL;
	}

	bzero(hexHashes, kHashTotalHexLength);
	
	return;
} // RemoveShadowHash

//----------------------------------------------------------------------------------------------------
//  HashesEqual
//
//  Returns: BOOL
//
// ================================================================================
//	Hash File Matrix (Tiger)
// ---------------------------------------------------------------------
//	Hash Type						 Desktop		 Server		Priority
// ---------------------------------------------------------------------
//		NT								X				X			3
//		LM							   Opt.				X			4
//	   SHA1							  Erase			  Erase			-
//	 CRAM-MD5											X			5
//	Salted SHA1						   Opt.			   Opt.			2
//	RECOVERABLE										   Opt.			6
//	Security Team Favorite			  Only			  Only			1
//	
// ================================================================================
//----------------------------------------------------------------------------------------------------

bool CDSLocalAuthHelper::HashesEqual( const unsigned char *inUserHashes, const unsigned char *inGeneratedHashes )
{
	static int sPriorityMap[  ][ 2 ] =
	{
		//	start, len
						// security team favorite goes here //
		{ kHashOffsetToSaltedSHA1, kHashSaltedSHA1Length },						// Salted SHA1
		{ kHashOffsetToSHA1, kHashSecureLength },								// SHA1
		{ kHashOffsetToNT, kHashShadowOneLength },								// NT
		{ kHashOffsetToLM, 16 },												// LM
		{ kHashOffsetToCramMD5, kHashCramLength },								// CRAM-MD5
		{ kHashOffsetToRecoverable, kHashRecoverableLength },					// RECOVERABLE
		{ 0, 0 }																// END
	};
	
	int start, len;
	bool result = false;
	
	for ( int idx = 0; ; idx++ )
	{
		start = sPriorityMap[idx][0];
		len = sPriorityMap[idx][1];
		
		if ( start == 0 && len == 0 )
			break;
		
		// verify with the highest priority hash that exists
		if ( memcmp( inUserHashes + start, sZeros, len ) != 0 )
		{
			if ( memcmp( inUserHashes + start, inGeneratedHashes + start, len ) == 0 )
				result = true;
			
			// stop here - do not fallback to lower priority hashes
			break;
		}
	}
	
	return result;
}


//----------------------------------------------------------------------------------------------------
//  WriteHashStateFile
//
//  Returns: -1 = error, 0 = ok.
//----------------------------------------------------------------------------------------------------

int CDSLocalAuthHelper::WriteHashStateFile( const char *inFilePath, sHashState *inHashState )
{
	CFStringRef myReplicaDataFilePathRef;
	CFURLRef myReplicaDataFileRef;
	CFWriteStreamRef myWriteStreamRef;
	CFStringRef errorString;
	int err = 0;
    //struct stat sb;
	CFMutableDictionaryRef prefsDict;
	CFDateRef aDateRef;
	
	if ( inFilePath == NULL || inHashState == NULL )
		return -1;
	
	// make the dict
	prefsDict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks );
	if ( prefsDict == NULL )
		return -1;
	
	do
	{
		if ( pwsf_ConvertBSDTimeToCFDate( &inHashState->creationDate, &aDateRef ) )
		{
			CFDictionaryAddValue( prefsDict, CFSTR("CreationDate"), aDateRef );
			CFRelease( aDateRef );
		}
		if ( pwsf_ConvertBSDTimeToCFDate( &inHashState->lastLoginDate, &aDateRef ) )
		{
			CFDictionaryAddValue( prefsDict, CFSTR("LastLoginDate"), aDateRef );
			CFRelease( aDateRef );
		}
		
		CFNumberRef failedAttemptCountRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt16Type,
			&(inHashState->failedLoginAttempts) );
		if ( failedAttemptCountRef != NULL )
		{
			CFDictionaryAddValue( prefsDict, CFSTR("FailedLoginCount"), failedAttemptCountRef );
			CFRelease( failedAttemptCountRef );
		}
		
		CFNumberRef newPasswordRequiredRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt16Type,
			&(inHashState->newPasswordRequired) );
		if ( newPasswordRequiredRef != NULL )
		{
			CFDictionaryAddValue( prefsDict, CFSTR("NewPasswordRequired"), newPasswordRequiredRef );
			CFRelease( newPasswordRequiredRef );
		}
		
		// WARNING: make sure the path to the file exists or CFStream code is unhappy
		/*
			err = stat( kPWReplicaDir, &sb );
			if ( err != 0 )
			{
				// make sure the directory exists
				err = mkdir( kPWReplicaDir, S_IRWXU );
				if ( err != 0 )
					return -1;
			}
		*/
		
		myReplicaDataFilePathRef = CFStringCreateWithCString( kCFAllocatorDefault, inFilePath, kCFStringEncodingUTF8 );
		if ( myReplicaDataFilePathRef == NULL )
		{
			err = -1;
			break;
		}
		
		myReplicaDataFileRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, myReplicaDataFilePathRef,
			kCFURLPOSIXPathStyle, false );
		
		CFRelease( myReplicaDataFilePathRef );
		
		if ( myReplicaDataFileRef == NULL )
		{
			err = -1;
			break;
		}
		
		myWriteStreamRef = CFWriteStreamCreateWithFile( kCFAllocatorDefault, myReplicaDataFileRef );
		
		CFRelease( myReplicaDataFileRef );
		
		if ( myWriteStreamRef == NULL )
		{
			err = -1;
			break;
		}
		
		CFWriteStreamOpen( myWriteStreamRef );
		chmod( inFilePath, 0600 );
		
		errorString = NULL;
		CFPropertyListWriteToStream( prefsDict, myWriteStreamRef, kCFPropertyListBinaryFormat_v1_0, NULL );
		
		CFWriteStreamClose( myWriteStreamRef );
		CFRelease( myWriteStreamRef );
		
	} while( false );
	
	if ( prefsDict != NULL )
		CFRelease( prefsDict );
	
	return err;
}


//--------------------------------------------------------------------------------------------------
// * ReadShadowHashAndStateFiles
//
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::ReadShadowHashAndStateFiles( const char *inUserName, const char *inGUIDString,
	unsigned char outHashes[kHashTotalLength], struct timespec *outModTime, char **outUserHashPath, char **outStateFilePath,
	sHashState *inOutHashState, SInt32 *outHashDataLen )
{
	if ( inUserName == NULL || outStateFilePath == NULL || outUserHashPath == NULL )
		return eParameterError;
	
	*outStateFilePath = NULL;
	
	tDirStatus siResult = ReadShadowHash( inUserName, inGUIDString, outHashes, outModTime,
		outUserHashPath, outHashDataLen, true );
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
} // ReadShadowHashAndStateFiles

//--------------------------------------------------------------------------------------------------
// * GetShadowHashGlobalPolicies ()
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::GetShadowHashGlobalPolicies( CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode,
	PWGlobalAccessFeatures *inOutGAccess, PWGlobalMoreAccessFeatures *inOutGMoreAccess )
{
	tDirStatus error = eDSNoErr;
	char* policyStr = NULL;
	CFMutableArrayRef recordsArray = NULL;
	char* cStr = NULL;
	size_t cStrSize = 0;
	
	if ( inOutGAccess == NULL )
		return eParameterError;
	
	bzero( inOutGAccess, sizeof(PWGlobalAccessFeatures) );
	
	CFStringRef nativeRecType = inPlugin->RecordNativeTypeForStandardType( CFSTR( kDSStdRecordTypeConfig ) );

	try
	{
		recordsArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	
		CFStringRef recName = CFSTR( kShadowHashRecordName );
		CFArrayRef recNames = CFArrayCreate( NULL, (const void**)&recName, 1, &kCFTypeArrayCallBacks );
		error = inNode->GetRecords( nativeRecType, recNames, CFSTR( kDSNAttrRecordName ), eDSExact, false, 1,
			recordsArray );
		CFRelease( recNames );
		if ( error != eDSNoErr )
			throw( error );
		
		CFStringRef pwdPolicyOptions = NULL;
		if ( CFArrayGetCount( recordsArray ) > 0 )
		{
			CFDictionaryRef shadowHashRecord = (CFDictionaryRef)CFArrayGetValueAtIndex( recordsArray, 0 );

			CFArrayRef pwdPolicyOptionsVals = (CFArrayRef)CFDictionaryGetValue( shadowHashRecord,
				inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPasswordPolicyOptions ) ) );
			
			if ( ( pwdPolicyOptionsVals != NULL ) && ( CFArrayGetCount( pwdPolicyOptionsVals ) > 0 ) )
				pwdPolicyOptions = (CFStringRef)CFArrayGetValueAtIndex( pwdPolicyOptionsVals, 0 );
		}
		
		if ( ( pwdPolicyOptions != NULL ) && ( CFStringGetLength( pwdPolicyOptions ) > 0 ) )
		{
			if ( ::ConvertGlobalXMLPolicyToSpaceDelimited(
					CStrFromCFString( pwdPolicyOptions, &cStr, &cStrSize, NULL ), &policyStr ) == 0 )
				::StringToPWGlobalAccessFeaturesExtra( policyStr, inOutGAccess, inOutGMoreAccess );
		}
	}
	catch( tDirStatus catchErr )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::GetShadowHashGlobalPolicies(): got error %d", catchErr );
		error = catchErr;
	}
	
	if ( recordsArray != NULL )
		CFRelease( recordsArray );
	if ( policyStr != NULL )
		::free( policyStr );
	if ( cStr != NULL )
		::free( cStr );
	
	return error;
}

//--------------------------------------------------------------------------------------------------
// * SetShadowHashGlobalPolicies
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::SetShadowHashGlobalPolicies( CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode,
	tDirNodeReference inNodeRef, CFStringRef inAuthedUserName, uid_t inUID, uid_t inEffectiveUID,
	PWGlobalAccessFeatures *inGAccess, PWGlobalMoreAccessFeatures *inGMoreAccess )
{
	tDirStatus siResult = eDSAuthFailed;
	CFMutableArrayRef recordsArray = NULL;
	CFStringRef recName = NULL;
	CFArrayRef recNames = NULL;
	tRecordReference shadowHashRecRef = 0;
	
	try
	{
		if ( inGAccess == NULL )
			throw( eDSAuthFailed );
		
		if ( inAuthedUserName == NULL )
			throw( eDSPermissionError );
		
		char username[256];
		if ( CFStringGetCString(inAuthedUserName, username, sizeof(username), kCFStringEncodingUTF8) == true )
		{
			if ( dsIsUserMemberOfGroup(username, "admin") == false ) 
				throw eDSPermissionError;
		}

		recordsArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	
		recName = CFSTR( kShadowHashRecordName );
		recNames = CFArrayCreate( NULL, (const void**)&recName, 1, &kCFTypeArrayCallBacks );
		CFStringRef recordType = inPlugin->RecordNativeTypeForStandardType( CFSTR(kDSStdRecordTypeConfig) );
		siResult = inNode->GetRecords( recordType,
			recNames, CFSTR( kDSNAttrRecordName ), eDSExact, false, 1, recordsArray );
		CFRelease( recNames );
		if ( siResult != eDSNoErr )
			throw( siResult );

		if ( CFArrayGetCount( recordsArray ) == 0 )
		{
			siResult = inPlugin->CreateRecord( CFSTR( kDSStdRecordTypeConfig ), CFSTR( kShadowHashRecordName ),
				true, &shadowHashRecRef );
			if ( siResult != eDSNoErr )
				throw( siResult );
		}
		else
		{
			siResult = inPlugin->OpenRecord( CFSTR( kDSStdRecordTypeConfig ), CFSTR( kShadowHashRecordName ),
				&shadowHashRecRef );
			if ( siResult != eDSNoErr )
				throw( siResult );
		}
		
		if ( !inNode->AccessAllowed(inAuthedUserName, inEffectiveUID, recordType, inPlugin->AttrNativeTypeForStandardType(CFSTR(kDS1AttrPasswordPolicyOptions)), eDSAccessModeWriteAttr) )
			throw( eDSPermissionError );
		
		{
			char *xmlDataStr;
			char policyStr[2048];
			
			::PWGlobalAccessFeaturesToStringExtra( inGAccess, inGMoreAccess, sizeof( policyStr ), policyStr );
			if ( ::ConvertGlobalSpaceDelimitedPolicyToXML( policyStr, &xmlDataStr ) == 0 )
			{
				inPlugin->RemoveAttribute( shadowHashRecRef, CFSTR( kDS1AttrPasswordPolicyOptions ) );
				
				CFStringRef xmlDataCFStr = CFStringCreateWithCString( NULL, xmlDataStr, kCFStringEncodingUTF8 );
				siResult = inPlugin->AddAttribute( shadowHashRecRef, CFSTR( kDS1AttrPasswordPolicyOptions ), xmlDataCFStr );
				CFRelease( xmlDataCFStr );
				::free( xmlDataStr );
				
				if ( siResult != eDSNoErr )
					throw( siResult );
			}
		}
	}

	catch( tDirStatus err )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::SetShadowHashGlobalPolicies(): got error %d", err );
		siResult = err;
	}

	if ( shadowHashRecRef != 0 ) {
		DbgLog( kLogDebug, "CDSLocalAuthHelper::SetShadowHashGlobalPolicies - closing internal record reference" );
		gRefTable.RemoveReference( shadowHashRecRef, eRefTypeRecord, 0, 0 ); // remove closes the record automatically
		shadowHashRecRef = 0;
	}

	if ( recordsArray != NULL )
		CFRelease( recordsArray );
	
	return( siResult );

} // SetShadowHashGlobalPolicies

//--------------------------------------------------------------------------------------------------
// * GenerateShadowHashes
//--------------------------------------------------------------------------------------------------

void CDSLocalAuthHelper::GenerateShadowHashes( bool inServerOS, const char *inPassword, long inPasswordLen,
											   UInt32 inAdditionalHashList, const unsigned char *inSHA1Salt, unsigned char *outHashes,
											   UInt32 *outHashTotalLength )
{
	CC_SHA1_CTX		sha_context						= {};
	unsigned char	digestData[kHashSecureLength]	= {0};
	long			pos								= 0;
	
	/* start clean */
	bzero( outHashes, kHashTotalLength );
	
	/* NT */
	if ( (inAdditionalHashList & ePluginHashNT) )
		CalculateSMBNTHash( inPassword, outHashes );
	pos = kHashShadowOneLength;
	
	/* LM */
	if ( (inAdditionalHashList & ePluginHashLM) )
		CalculateSMBLANManagerHash( inPassword, outHashes + kHashShadowOneLength );
	pos = kHashShadowBothLength;
	
	/* SHA1 - Deprecated BUT required for automated legacy upgrades */
	if ( (inAdditionalHashList & ePluginHashSHA1) )
	{
		CC_SHA1_Init( &sha_context );
		CC_SHA1_Update( &sha_context, (unsigned char *)inPassword, inPasswordLen );
		CC_SHA1_Final( digestData, &sha_context );
		memmove( outHashes + pos, digestData, kHashSecureLength );
	}
	pos += kHashSecureLength;
	
	/* CRAM-MD5 */
	if ( (inAdditionalHashList & ePluginHashCRAM_MD5) )
	{
		unsigned long cramHashLen = 0;
		pwsf_getHashCramMD5( (const unsigned char *)inPassword, inPasswordLen, outHashes + pos, &cramHashLen );
	}
	pos += kHashCramLength;
	
	/* 4-byte Salted SHA1 */
	if ( (inAdditionalHashList & ePluginHashSaltedSHA1) )
	{
		UInt32 salt;
		
		if ( inSHA1Salt != NULL )
		{
			memcpy( &salt, inSHA1Salt, 4 );
			memcpy( outHashes + pos, inSHA1Salt, 4 );
		}
		else
		{
			salt = (UInt32) arc4random();
			memcpy( outHashes + pos, &salt, 4 );
		}
		
		pos += 4;
		CC_SHA1_Init( &sha_context );
		CC_SHA1_Update( &sha_context, (unsigned char *)&salt, 4 );
		CC_SHA1_Update( &sha_context, (unsigned char *)inPassword, inPasswordLen );
		CC_SHA1_Final( digestData, &sha_context );
		memmove( outHashes + pos, digestData, kHashSecureLength );
		pos += kHashSecureLength;
	}
	else
	{
		pos += 4 + kHashSecureLength;
	}
	
	/* recoverable */
	if ( inServerOS && (inAdditionalHashList & ePluginHashRecoverable) )
	{
		CCCryptorStatus status = kCCSuccess;
		unsigned char iv[kCCBlockSizeAES128];
		size_t dataMoved;
		unsigned char passCopy[kHashRecoverableLength + kCCBlockSizeAES128];
		
		bzero( passCopy, sizeof(passCopy) );
		memcpy( passCopy, inPassword, (inPasswordLen < kHashRecoverableLength) ? inPasswordLen : (kHashRecoverableLength - 1) );
		
		memcpy( iv, kAESVector, sizeof(iv) );
		
		status = CCCrypt(
					kCCEncrypt,
					kCCAlgorithmAES128,
					0,
					(const unsigned char *)"key4now-key4now-key4now", kCCKeySizeAES128,
					iv,
					passCopy,
					kHashRecoverableLength,
					outHashes + pos,
					kHashRecoverableLength,
					&dataMoved );
	}
	pos += kHashRecoverableLength;
	
	*outHashTotalLength = kHashTotalLength;
}

//--------------------------------------------------------------------------------------------------
// * UnobfuscateRecoverablePassword()
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::UnobfuscateRecoverablePassword( unsigned char *inData, unsigned char **outPassword,
	UInt32 *outPasswordLength )
{
	// un-obfuscate
	CCCryptorStatus status = kCCSuccess;
	unsigned char iv[kCCBlockSizeAES128];
	size_t dataMoved;
	unsigned char passCopy[kHashRecoverableLength + kCCBlockSizeAES128];
	
	if ( inData == NULL || outPassword == NULL || outPasswordLength == NULL )
		return eParameterError;

	bzero( passCopy, sizeof(passCopy) );
	memcpy( iv, kAESVector, sizeof(iv) );
	status = CCCrypt(
				kCCDecrypt,
				kCCAlgorithmAES128,
				0,
				(const unsigned char *)"key4now-key4now-key4now", kCCKeySizeAES128,
				iv,
				inData,
				kHashRecoverableLength,
				passCopy,
				sizeof(passCopy),
				&dataMoved );
	
	*outPasswordLength = strlen( (char *)passCopy );
	*outPassword = (unsigned char *) malloc( (*outPasswordLength) + 1 );
	if ( (*outPassword) == NULL )
		return eMemoryError;
	
	strlcpy( (char *)*outPassword, (char *)passCopy, (*outPasswordLength) + 1 );
	
	return eDSNoErr;
}

//--------------------------------------------------------------------------------------------------
// * MSCHAPv2 ()
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::MSCHAPv2( const unsigned char *inC16, const unsigned char *inPeerC16,
	const unsigned char *inNTLMDigest, const char *inSambaName, const unsigned char *inOurHash,
	char *outMSCHAP2Response )
{
	unsigned char ourP24[kHashShadowResponseLength];
	unsigned char Challenge[8];
	tDirStatus result = eDSAuthFailed;
	
	ChallengeHash( inPeerC16, inC16, inSambaName, Challenge );
	ChallengeResponse( Challenge, inOurHash, ourP24 );
	
	if ( memcmp( ourP24, inNTLMDigest, kHashShadowResponseLength ) == 0 )
	{
		GenerateAuthenticatorResponse( inOurHash, ourP24, Challenge, outMSCHAP2Response );
		result = eDSNoErr;
	}
	
	return result;
}

//--------------------------------------------------------------------------------------------------
// * CRAM_MD5 ()
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::CRAM_MD5( const unsigned char *inHash, const char *inChallenge,
	const unsigned char *inResponse )
{
	tDirStatus siResult = eDSAuthFailed;
	HMAC_MD5_STATE md5state;
	HMAC_MD5_CTX tmphmac;
	unsigned char digest[MD5_DIGEST_LENGTH];
	char correctAnswer[HASHHEXLEN+1];
	
	memcpy(&md5state, inHash, sizeof(HMAC_MD5_STATE));
	CDSLocalAuthHelper::hmac_md5_import(&tmphmac, (HMAC_MD5_STATE *) &md5state);
	MD5_Update(&(tmphmac.ictx), (const unsigned char *) inChallenge, strlen(inChallenge));
	CDSLocalAuthHelper::hmac_md5_final(digest, &tmphmac);
    
    /* convert to hex with lower case letters */
    CvtHex(digest, (unsigned char *)correctAnswer);
	
	if ( strncasecmp((char *)inResponse, correctAnswer, 32) == 0 )
		siResult = eDSNoErr;
	
	return siResult;
}

void CDSLocalAuthHelper::hmac_md5_import(HMAC_MD5_CTX *hmac, HMAC_MD5_STATE *state)
{
	bzero((char *)hmac, sizeof(HMAC_MD5_CTX));
		
	hmac->ictx.A = ntohl(state->istate[0]);
	hmac->ictx.B = ntohl(state->istate[1]);
	hmac->ictx.C = ntohl(state->istate[2]);
	hmac->ictx.D = ntohl(state->istate[3]);
	
	hmac->octx.A = ntohl(state->ostate[0]);
	hmac->octx.B = ntohl(state->ostate[1]);
	hmac->octx.C = ntohl(state->ostate[2]);
	hmac->octx.D = ntohl(state->ostate[3]);
	
	/* Init the counts to account for our having applied
	* 64 bytes of key; this works out to 0x200 (64 << 3; see
	* MD5Update above...) */
	hmac->ictx.Nl = hmac->octx.Nl = 0x200;
}

void CDSLocalAuthHelper::hmac_md5_final(unsigned char digest[HMAC_MD5_SIZE], HMAC_MD5_CTX *hmac)
{
	MD5_Final(digest, &hmac->ictx);  /* Finalize inner md5 */
	MD5_Update(&hmac->octx, digest, MD5_DIGEST_LENGTH); /* Update outer ctx */
	MD5_Final(digest, &hmac->octx); /* Finalize outer md5 */
}

//--------------------------------------------------------------------------------------------------
// * Verify_APOP ()
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::Verify_APOP( const char *userstr, const unsigned char *inPassword,
	UInt32 inPasswordLen, const char *challenge, const char *response )
{
    tDirStatus siResult = eDSAuthFailed;
    unsigned char digest[16];
    char digeststr[33];
    CC_MD5_CTX ctx;

    if ( challenge == NULL || inPassword == NULL || response == NULL )
       return eParameterError;
	
    CC_MD5_Init( &ctx );
    CC_MD5_Update( &ctx, challenge, strlen(challenge) );
    CC_MD5_Update( &ctx, inPassword, inPasswordLen );
    CC_MD5_Final( digest, &ctx );
	
    /* convert digest from binary to ASCII hex */
	CvtHex(digest, (unsigned char *)digeststr);
	
    if ( strncasecmp(digeststr, response, 32) == 0 )
	{
      /* password verified! */
      siResult = eDSNoErr;
    }
	
    return siResult;
}

//------------------------------------------------------------------------------------
//	PasswordOkForPolicies
//
//	Returns: ds err code
//------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::PasswordOkForPolicies( const char *inSpaceDelimitedPolicies,
	PWGlobalAccessFeatures *inGAccess, const char *inUsername, const char *inPassword )
{
	PWAccessFeatures access;
	PWMoreAccessFeatures moreAccess = {0};
	tDirStatus siResult = eDSNoErr;
	int result;
	
DbgLog( kLogPlugin, "CDSLocalAuthHelper::PasswordOkForPolicies()" );

	if ( inPassword == NULL )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::PasswordOkForPolicies(): no password" );
		return eDSNoErr;
	}
	
	if ( inGAccess->noModifyPasswordforSelf )
		return eDSAuthFailed;
	
	// setup user policy default
	::GetDefaultUserPolicies( &access );
	
	// apply user policies
	if ( inSpaceDelimitedPolicies != NULL )
		::StringToPWAccessFeaturesExtra( inSpaceDelimitedPolicies, &access, &moreAccess );
	
	try
	{
		if ( !access.canModifyPasswordforSelf )
			throw( eDSAuthFailed );
		
		result = ::pwsf_RequiredCharacterStatusExtra( &access, inGAccess, inUsername, inPassword, &moreAccess );
		switch( result )
		{
			case kAuthOK:						siResult = eDSNoErr;							break;
			case kAuthUserDisabled:				siResult = eDSAuthAccountDisabled;				break;
			case kAuthPasswordExpired:			siResult = eDSAuthPasswordExpired;				break;
			case kAuthPasswordNeedsChange:		siResult = eDSAuthPasswordQualityCheckFailed;   break;
			case kAuthPasswordTooShort:			siResult = eDSAuthPasswordTooShort;				break;
			case kAuthPasswordTooLong:			siResult = eDSAuthPasswordTooLong;				break;
			case kAuthPasswordNeedsAlpha:		siResult = eDSAuthPasswordNeedsLetter;			break;
			case kAuthPasswordNeedsDecimal:		siResult = eDSAuthPasswordNeedsDigit;			break;
			case kAuthPasswordNeedsMixedCase:	siResult = eDSAuthPasswordQualityCheckFailed;	break;
			
			default:
				siResult = eDSAuthFailed;
				break;
		}
		/*
		int usingHistory:1;						// TRUE == user has a password history file
		int usingExpirationDate:1;				// TRUE == look at expirationDateGMT
		int usingHardExpirationDate:1;			// TRUE == look at hardExpirationDateGMT
		unsigned int historyCount:4;
		
		BSDTimeStructCopy expirationDateGMT;	// if exceeded, user is required to change the password at next login
		BSDTimeStructCopy hardExpireDateGMT;	// if exceeded, user is disabled
		*/
	}
	catch( tDirStatus catchErr )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::PasswordOkForPolicies(): got error: %d", catchErr );
		siResult = catchErr;
	}
	
    return siResult;	
}

//------------------------------------------------------------------------------------
//	TestPolicies
//
//	Returns: ds err code
//------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::TestPolicies( const char *inSpaceDelimitedPolicies, PWGlobalAccessFeatures *inGAccess,
	sHashState *inOutHashState, struct timespec *inModDateOfPassword, const char *inHashPath )
{
	PWAccessFeatures access;
	PWMoreAccessFeatures moreAccess = {0};
	int result;
	tDirStatus siResult = eDSNoErr;
	
	if ( inHashPath == NULL )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::TestPolicies(): no path" );
		return eDSNoErr;
	}
	
	GetDefaultUserPolicies( &access );
	
	if ( inSpaceDelimitedPolicies != NULL )
		StringToPWAccessFeaturesExtra( inSpaceDelimitedPolicies, &access, &moreAccess );
		
	try
	{
		result = pwsf_TestDisabledStatus( &access, inGAccess, &(inOutHashState->creationDate),
			&(inOutHashState->lastLoginDate), &(inOutHashState->failedLoginAttempts) );
		if ( result == kAuthUserDisabled )
		{
			inOutHashState->disabled = 1;
			throw( eDSAuthAccountDisabled );
		}
		
		if ( inOutHashState->newPasswordRequired )
			throw( eDSAuthNewPasswordRequired );
		
		gmtime_r( (const time_t *)&inModDateOfPassword->tv_sec, &(inOutHashState->modDateOfPassword) );
		result = pwsf_ChangePasswordStatus( &access, inGAccess, &(inOutHashState->modDateOfPassword) );
		switch( result )
		{
			case kAuthPasswordNeedsChange:
				siResult = eDSAuthNewPasswordRequired;
				break;
				
			case kAuthPasswordExpired:
				siResult = eDSAuthPasswordExpired;
				break;
			
			default:
				break;
		}
	}
	catch( tDirStatus catchErr)
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::TestPolicies(): got error %d", catchErr );
		siResult = catchErr;
	}
		
    return siResult;
}

//------------------------------------------------------------------------------------
//	* MigrateToShadowHash ()
//
//------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::MigrateToShadowHash( tDirNodeReference inNodeRef, CDSLocalPlugin* inPlugin,
	CDSLocalPluginNode* inNode, CFMutableDictionaryRef inMutableRecordDict, const char *inUserName, const char *inPassword,
	bool *outResetCache, unsigned int inHashList, CFStringRef inNativeRecType )
{
	tDirStatus				siResult							= eDSAuthFailed;
	unsigned char			generatedHashes[kHashTotalLength]   = {0};
	UInt32					hashTotalLength						= 0;
	char*					cStr								= NULL;
	size_t					cStrSize							= 0;
	CFStringRef				cfString							= NULL;
	tRecordReference		recordRef							= 0;
	CFMutableDictionaryRef	nodeDict							= NULL;
	CFStringRef				preRootAuthString					= NULL;
	CFStringRef				nativeTypeGUIDString				= NULL;
	
	nativeTypeGUIDString = inPlugin->AttrNativeTypeForStandardType( CFSTR(kDS1AttrGeneratedUID) );
	if ( nativeTypeGUIDString == NULL )
		return eDSNullAttributeType;
	
	try
	{
		CFArrayRef values = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict, nativeTypeGUIDString );
		if ( ( values != NULL ) && ( CFArrayGetCount( values ) > 0 ) )
			cfString = (CFStringRef)CFArrayGetValueAtIndex( values, 0 );

		//find the GUID
		const char* guidCStr = NULL;
		
		if ( ( cfString != NULL ) && ( CFStringGetLength( cfString ) > 0 ) )
			guidCStr = CStrFromCFString( cfString, &cStr, &cStrSize, NULL );

		values = (CFArrayRef)CFDictionaryGetValue( inMutableRecordDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrRecordName ) ) );
		if ( CFArrayGetCount( values ) > 0 )
			cfString = (CFStringRef)CFArrayGetValueAtIndex( values, 0 );
		else
			throw( eDSRecordNotFound );

		siResult = inPlugin->OpenRecord( inNativeRecType, cfString, &recordRef );
		if ( siResult != eDSNoErr )
			throw( siResult );

		uuid_t uuid;
		uuid_string_t uuid_str;

		if ( guidCStr != NULL )
		{
			if ( uuid_parse(guidCStr, uuid) == 0 ) {
				uuid_unparse_upper( uuid, uuid_str );
				guidCStr = (const char *) uuid_str;
			}
			else {
				DbgLog( kLogError, "Record has malformed UUID - '%s' - expected format 'ADC9246E-8E90-4165-95E1-BED50931021B'", guidCStr );
				throw eDSSchemaError;
			}
		}
		else if ( guidCStr == NULL )
		{
			uuid_generate_random( uuid );
			uuid_unparse_upper( uuid, uuid_str );
			
			//write the GUID value to the user record
			cfString = CFStringCreateWithCString( NULL, uuid_str, kCFStringEncodingUTF8 );
			siResult = inPlugin->AddAttribute( recordRef, CFSTR( kDS1AttrGeneratedUID ), cfString );
			if ( siResult != eDSNoErr )
				throw( siResult );
			
			//and modify the copy of the user record that we're passing around
			CFDictionaryRemoveValue( inMutableRecordDict, nativeTypeGUIDString );
			CFMutableArrayRef mutableArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
			CFArrayAppendValue( mutableArray, cfString );
			CFDictionaryAddValue( inMutableRecordDict, nativeTypeGUIDString, mutableArray );
			
			DSCFRelease( mutableArray );
			DSCFRelease( cfString );
			*outResetCache = true;
		}
		
		CDSLocalAuthHelper::GenerateShadowHashes( gServerOS, inPassword, strlen(inPassword), inHashList, NULL,
			generatedHashes, &hashTotalLength );
		
		siResult = CDSLocalAuthHelper::WriteShadowHash(inUserName, guidCStr, generatedHashes);
		if (siResult != eDSNoErr)
			throw( siResult );
		{
			// get root powers
			// retrieve the same nodeDict the plugin object is going to use 
			
			CFDictionaryRef openRecordDict = inPlugin->RecordDictForRecordRef( recordRef );
			if ( openRecordDict == NULL )
				throw( eDSInvalidRecordRef );
			
			nodeDict = (CFMutableDictionaryRef)CFDictionaryGetValue( openRecordDict, CFSTR(kOpenRecordDictNodeDict) );
			if ( nodeDict == NULL )
				throw( eDSInvalidNodeRef );
			
			preRootAuthString = (CFStringRef) CFDictionaryGetValue( nodeDict, CFSTR(kNodeAuthenticatedUserName) );
			if ( preRootAuthString != NULL )
				CFRetain( preRootAuthString );

			// does this need to be protected since nodeDicts are per-client?
			CFDictionarySetValue( nodeDict, CFSTR(kNodeAuthenticatedUserName), CFSTR("root") );
			
			// replace the auth authority and password attributes in the record
			siResult = ::SetUserAuthAuthorityAsRoot(
							inMutableRecordDict,
							inPlugin,
							inNode,
							inPlugin->RecordStandardTypeForNativeType(inNativeRecType),
							inUserName,
							kDSValueAuthAuthorityShadowHash,
							inNodeRef,
							true );
			
			if ( siResult != eDSNoErr )
				throw( siResult );
			
			inPlugin->RemoveAttribute( recordRef, CFSTR( kDS1AttrPassword ) );
			siResult = inPlugin->AddAttribute( recordRef, CFSTR(kDS1AttrPassword), CFSTR(kDSValueNonCryptPasswordMarker) );
			if ( siResult != eDSNoErr )
				throw( siResult );

			// and modify the copy of the user record that we're passing around			
			CFMutableArrayRef mutableArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
			CFArrayAppendValue( mutableArray, CFSTR( kDSValueAuthAuthorityShadowHash ) );
			CFDictionarySetValue( inMutableRecordDict,
				inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrAuthenticationAuthority ) ), mutableArray );
			DSCFRelease( mutableArray );
			
			mutableArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
			CFArrayAppendValue( mutableArray, CFSTR( kDSValueNonCryptPasswordMarker ) );
			CFDictionarySetValue( inMutableRecordDict,
				inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPassword ) ), mutableArray );
			DSCFRelease( mutableArray );
		}
	}
	catch( tDirStatus err )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::MigrateToShadowHash(): got error %d", err );
		siResult = err;
	}
	
	if ( recordRef != 0 ) {
		DbgLog( kLogDebug, "CDSLocalAuthHelper::MigrateToShadowHash - closing internal record reference" );
		gRefTable.RemoveReference( recordRef, eRefTypeRecord, 0, 0 ); // remove closes the record automatically
		recordRef = 0;
	}

	DSFreeString( cStr );
	
	if ( nodeDict != NULL )
	{
		if ( preRootAuthString != NULL )
			CFDictionarySetValue( nodeDict, CFSTR(kNodeAuthenticatedUserName), preRootAuthString );
		else
			CFDictionaryRemoveValue( nodeDict, CFSTR(kNodeAuthenticatedUserName) );
	}
	DSCFRelease( preRootAuthString );
	
	return siResult;
}


//--------------------------------------------------------------------------------------------------
// * MigrateAddKerberos()
//
//	Migration from Tiger to Leopard requires adding a Kerberosv5 principal to the local realm.
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::MigrateAddKerberos(
	tDirNodeReference inNodeRef, CDSLocalAuthParams &inParams,
	tContextData *inOutContinueData, tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
	CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
	CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
	CDSLocalPlugin *inPlugin, CDSLocalPluginNode *inNode, CFStringRef inAuthedUserName,
	uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType,	
	bool inOKToChangeAuthAuthorities )
{
	tDirStatus siResult = eDSNoErr;

	if ( !inOKToChangeAuthAuthorities )
		return siResult;
	
	char *localKDCRealmStr = GetLocalKDCRealmWithCache( kLocalKDCRealmCacheTimeout );
	if ( localKDCRealmStr != NULL )
	{
		// if AuthenticationAuthority has only ;ShadowHash;
		if ( inAuthAuthorityList.GetValueCount() == 1 &&
			 (inAuthAuthorityList.GetValueForTagAsCFDict(kDSTagAuthAuthorityShadowHash) != NULL ||
			  inAuthAuthorityList.GetValueForTagAsCFDict(kDSTagAuthAuthorityLocalCachedUser) != NULL ||
			  inAuthAuthorityList.GetValueForTagAsCFDict(kDSTagAuthAuthorityBasic) != NULL) )
		{
			AddKerberosAuthAuthority(
				inNodeRef, inParams.pUserName, kDSTagAuthAuthorityKerberosv5, inAuthAuthorityList, inMutableRecordDict,
				inPlugin, inNode, inNativeRecType, inOKToChangeAuthAuthorities );
			
			if ( inParams.uiAuthMethod == kAuthNativeClearTextOK || inParams.uiAuthMethod == kAuthNativeNoClearText )
			{
				// Update Kerberos: first login, need to create principal with current password regardless
				// of password policy
				if ( pwsf_AddPrincipalToLocalRealm(inParams.pUserName, inParams.pOldPassword, localKDCRealmStr) != 0 )
					DbgLog( kLogPlugin, "CDSLocalAuthHelper::MigrateAddKerberos(): Unable to add principal %s@%s",
						inParams.pUserName, localKDCRealmStr );
			}
			else
			{
				// Update Kerberos: works for the set/change password methods, enforces password policy
				siResult = CDSLocalAuthHelper::DoKerberosAuth( inNodeRef, inParams, inOutContinueData, inAuthData,
								outAuthData, inAuthOnly, isSecondary, inAuthAuthorityList, inGUIDString, inAuthedUserIsAdmin,
								inMutableRecordDict, inHashList, inPlugin, inNode, inAuthedUserName, inUID, inEffectiveUID,
								inNativeRecType );
			}
		}
		
		free( localKDCRealmStr );
	}
	
	return siResult;
}
		

//--------------------------------------------------------------------------------------------------
// * PWSetReplicaData ()
//
//	Note:	inAuthorityData is the UserID + RSA_key, but the IP address should be pre-stripped by
//			the calling function.
//--------------------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::PWSetReplicaData( CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode,
	tDirNodeReference inNodeRef, tDirNodeReference inPWSNodeRef, const char *inAuthorityData )
{
	tDirStatus			error					= eDSNoErr;
	long				replicaListLen			= 0;
	char				*rsaKeyPtr				= NULL;
	tDataBufferPtr		replicaBuffer			= NULL;
    tDataBufferPtr		replyBuffer				= NULL;
	char				hashStr[34];
	CFMutableArrayRef	recordsArray			= NULL;
	char*				cStr					= NULL;
	size_t				cStrSize				= 0;
	
	try
	{
		// get /config/passwordserver_HEXHASH
		CFStringRef nativeConfigRecType = inPlugin->RecordNativeTypeForStandardType( CFSTR( kDSStdRecordTypeConfig ) );
		
		recordsArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		CFDictionaryRef recordDict = NULL;
		rsaKeyPtr = strchr( inAuthorityData, ',' );
		if ( rsaKeyPtr != NULL )
		{
			CC_MD5_CTX ctx;
			unsigned char pubKeyHash[CC_MD5_DIGEST_LENGTH];
			
			CC_MD5_Init( &ctx );
			rsaKeyPtr++;
			CC_MD5_Update( &ctx, rsaKeyPtr, strlen(rsaKeyPtr) );
			CC_MD5_Final( pubKeyHash, &ctx );
			
			BinaryToHexConversion( pubKeyHash, CC_MD5_DIGEST_LENGTH, hashStr );
			CFStringRef recordName = CFStringCreateWithFormat( NULL, 0, CFSTR( "passwordserver_%s" ), hashStr );
			CFArrayRef patternsToMatch = CFArrayCreate( NULL, (const void**)&recordName, 1, &kCFTypeArrayCallBacks );
			error = inNode->GetRecords( nativeConfigRecType, patternsToMatch,
				inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrRecordName ) ), eDSExact, true, 1, recordsArray );
			CFRelease( recordName );
			CFRelease( patternsToMatch );
			if ( error != eDSNoErr )
				throw( error );
			if ( CFArrayGetCount( recordsArray ) > 0 )
				recordDict = (CFDictionaryRef)CFArrayGetValueAtIndex( recordsArray, 0 );
		}
		
		if ( recordDict == NULL ) 
		{
			CFStringRef recordName = CFSTR( "passwordserver" );
			CFArrayRef patternsToMatch = CFArrayCreate( NULL, (const void**)&recordName, 1, &kCFTypeArrayCallBacks );
			error = inNode->GetRecords( nativeConfigRecType, patternsToMatch,
				inPlugin->AttrNativeTypeForStandardType( CFSTR( kDSNAttrRecordName ) ), eDSExact, true, 1, recordsArray );
			CFRelease( recordName );
			CFRelease( patternsToMatch );

			if ( error != eDSNoErr )
				throw( error );
			else if ( CFArrayGetCount( recordsArray ) == 0 )
				throw( eDSRecordNotFound );
			else
				recordDict = (CFDictionaryRef)CFArrayGetValueAtIndex( recordsArray, 0 );
		}
			
		//lookup kDS1AttrPasswordServerList attribute
		CFStringRef pwsList = NULL;
		CFArrayRef pwsListAttrValues = (CFArrayRef)CFDictionaryGetValue( recordDict,
			inPlugin->AttrNativeTypeForStandardType( CFSTR( kDS1AttrPasswordServerList ) ) );
		if ( CFArrayGetCount( pwsListAttrValues ) > 0 )
			pwsList = (CFStringRef)CFArrayGetValueAtIndex( pwsListAttrValues, 0 );
		
		if ( ( pwsList != NULL ) && ( CFStringGetLength( pwsList ) > 0 ) )
		{
			const char* pwsListCStr = CStrFromCFString( pwsList, &cStr, &cStrSize );
			replicaListLen = strlen( pwsListCStr );
			replicaBuffer = ::dsDataBufferAllocatePriv( replicaListLen + 1 );
			if ( replicaBuffer == NULL ) throw( eMemoryError );
			
			replyBuffer = ::dsDataBufferAllocatePriv( 1 );
			if ( replyBuffer == NULL ) throw( eMemoryError );
			
			replicaBuffer->fBufferLength = replicaListLen;
			memcpy( replicaBuffer->fBufferData, pwsListCStr, replicaListLen );
			
			error = dsDoPlugInCustomCall( inPWSNodeRef, 1, replicaBuffer, replyBuffer );
			
			::dsDataBufferDeallocatePriv( replicaBuffer );
			::dsDataBufferDeallocatePriv( replyBuffer );
		}
	}
	catch( tDirStatus catchErr )
	{
		DbgLog(  kLogPlugin, "CDSLocalAuthHelper::PWSSetReplicaData(): got error %d", catchErr );
		error = catchErr;
	}
	
	if ( recordsArray != NULL )
		CFRelease( recordsArray );
	if ( cStr != NULL )
		::free( cStr );
	
	return error;
}


// ---------------------------------------------------------------------------
//	* LocalCachedUserReachable
//
//	RETURNS: tDirStatus
//
//	Verifies that the user's network account is "reachable" from the DS
//	perspective, i.e. the node is on the search policy. If
//	<inOutNodeReachable> is TRUE, that's not a guarantee that the LDAP
//	server is responding.
// ---------------------------------------------------------------------------

tDirStatus
CDSLocalAuthHelper::LocalCachedUserReachable(
	tDirNodeReference inNodeRef,
	tContextData *inOutContinueData,
	tDataBufferPtr inAuthData,
	tDataBufferPtr outAuthData,
	bool inAuthOnly,
	bool isSecondary,
	const char* inGUIDString,
	bool inAuthedUserIsAdmin,
	CDSLocalAuthParams &inPB,
	CFMutableDictionaryRef inMutableRecordDict,
	unsigned int inHashList,
	CDSLocalPlugin* inPlugin,
	CDSLocalPluginNode* inNode,
	CFStringRef inAuthedUserName,
	uid_t inUID,
	uid_t inEffectiveUID,
	CFStringRef inNativeRecType,
	bool* inOutNodeReachable,
	tDirReference *outDSRef,
	tDataList **outDSNetworkNode,
	char **localCachedUserName )
{
	tDirStatus				siResult				= eDSAuthFailed;
	char				   *networkNodename			= nil;
	char				   *userGUID				= nil;
	SInt32					result					= eDSNoErr;
	tDataBuffer			   *dataBuffer				= nil;
	UInt32					nodeCount				= 0;
	tDirNodeReference		aSearchNodeRef			= 0;
	tDataList			   *pSearchNode				= nil;
	tDataList			   *pSearchNodeList			= nil;
	tAttributeListRef		attrListRef				= 0;
	tAttributeValueListRef	attrValueListRef		= 0;
	tAttributeValueEntry   *pAttrValueEntry			= nil;
	tAttributeEntry		   *pAttrEntry				= nil;
	UInt32					aIndex					= 0;
		
	if ( inAuthData == nil ) return( eDSNullAuthStepData );
	if ( inOutNodeReachable == nil ) return( eParameterError );
	if ( outDSRef == nil ) return( eParameterError );
	if ( outDSNetworkNode == nil ) return( eParameterError );
	if ( localCachedUserName == nil ) return( eParameterError );
	
	*inOutNodeReachable = false;
	
	DbgLog( kLogPlugin, "LocalCachedUserReachable::checking" );
	
	try
	{
		siResult = ParseLocalCacheUserAuthData( inPB.aaDataLocalCacheUser, &networkNodename, localCachedUserName, &userGUID );
		
		result = inPlugin->GetDirServiceRef( outDSRef );
		if ( result == eDSNoErr )
		{
			*outDSNetworkNode = dsBuildFromPathPriv( networkNodename, "/" );
			if ( *outDSNetworkNode == nil ) throw( eMemoryError );
			dataBuffer = ::dsDataBufferAllocate( *outDSRef, 1024 );
			if ( dataBuffer == nil ) throw( eMemoryError );

			// if this is the Active Directory plugin we will make an exception because the plugin always allows
			// itself to be opened and does not always register all of it's nodes
			if ( strncmp("/Active Directory/", networkNodename, sizeof("/Active Directory/")-1) == 0 )
			{
				*inOutNodeReachable = true;
				siResult = eDSNoErr;
				throw( siResult );
			}
			
			result = dsFindDirNodes( *outDSRef, dataBuffer, *outDSNetworkNode, eDSiExact, &nodeCount, nil );
			if ( (result == eDSNoErr) && (nodeCount == 1) )
			{
				//now check if the node is actually on the search policy
				//get the search node. open it and call dsGetDirNodeInfo for kDS1AttrSearchPath
				//extract the node list
				result = dsFindDirNodes( *outDSRef, dataBuffer, nil, eDSAuthenticationSearchNodeName, &nodeCount, nil );
				if ( ( result == eDSNoErr ) && ( nodeCount == 1 ) )
				{
					result = dsGetDirNodeName( *outDSRef, dataBuffer, 1, &pSearchNode );
					if ( result == eDSNoErr )
					{
						result = dsOpenDirNode( *outDSRef, pSearchNode, &aSearchNodeRef );
						if ( pSearchNode != NULL )
						{
							dsDataListDeallocatePriv( pSearchNode );
							free( pSearchNode );
							pSearchNode = NULL;
						}
						if ( result == eDSNoErr )
						{
							pSearchNodeList = dsBuildFromPathPriv( kDS1AttrSearchPath, "/" );
							if ( pSearchNodeList == nil ) throw( eMemoryError );
							do
							{
								nodeCount = 0;
								result = dsGetDirNodeInfo( aSearchNodeRef, pSearchNodeList, dataBuffer, false, &nodeCount, &attrListRef, nil );
								if (result == eDSBufferTooSmall)
								{
									UInt32 bufSize = dataBuffer->fBufferSize;
									dsDataBufferDeallocatePriv( dataBuffer );
									dataBuffer = nil;
									dataBuffer = ::dsDataBufferAllocate( *outDSRef, bufSize * 2 );
								}
							} while (result == eDSBufferTooSmall);
							
							dsDataListDeallocatePriv( pSearchNodeList );
							free( pSearchNodeList );
							pSearchNodeList = NULL;
							
							if ( (result == eDSNoErr) && (nodeCount > 0) )
							{
								//assume first attribute since only 1 expected
								result = dsGetAttributeEntry( aSearchNodeRef, dataBuffer, attrListRef, 1, &attrValueListRef, &pAttrEntry );
								if ( result != eDSNoErr ) throw( result );
								
								//retrieve the node path strings
								for (aIndex=1; aIndex < (pAttrEntry->fAttributeValueCount+1); aIndex++)
								{
									result = dsGetAttributeValue( aSearchNodeRef, dataBuffer, aIndex, attrValueListRef, &pAttrValueEntry );
									if ( result != eDSNoErr ) throw( result );
									if ( pAttrValueEntry->fAttributeValueData.fBufferData == nil )
										throw( eMemoryAllocError );
									
									if (strcmp( networkNodename, pAttrValueEntry->fAttributeValueData.fBufferData ) == 0 )
									{
										*inOutNodeReachable = true; //node is registered in DS
										dsDeallocAttributeValueEntry(*outDSRef, pAttrValueEntry);
										pAttrValueEntry = nil;
										break;
									}
									dsDeallocAttributeValueEntry(*outDSRef, pAttrValueEntry);
									pAttrValueEntry = nil;
								}
								
								dsCloseAttributeList(attrListRef);
								dsCloseAttributeValueList(attrValueListRef);
								dsDeallocAttributeEntry(*outDSRef, pAttrEntry);
								pAttrEntry = nil;
								
								//close dir node after releasing attr references
								result = ::dsCloseDirNode(aSearchNodeRef);
								aSearchNodeRef = 0;
								if ( result != eDSNoErr ) throw( result );
							}
						}
					}
				}
			}
		}
	}
	catch( tDirStatus err )
	{
		siResult = err;
	}
	
	//cleanup
	DSFreeString( networkNodename );
	DSFreeString( userGUID );
	
    if ( dataBuffer != nil )
	{
        dsDataBufferDeallocatePriv( dataBuffer );
		dataBuffer = nil;
	}
	
	DbgLog( kLogPlugin, "LocalCachedUserReachable::result = %d, on SearchNode = %d", siResult, *inOutNodeReachable );
	
	return( siResult );
} //LocalCachedUserReachable


//------------------------------------------------------------------------------------
//	* DoLocalCachedUserAuthPhase2
//------------------------------------------------------------------------------------

tDirStatus CDSLocalAuthHelper::DoLocalCachedUserAuthPhase2(	tDirNodeReference inNodeRef,
															CDSLocalAuthParams &inParams,
															tContextData *inOutContinueData,
															tDataBufferPtr inAuthData,
															tDataBufferPtr outAuthData,
															bool inAuthOnly,
															bool isSecondary,
															CAuthAuthority &inAuthAuthorityList,
															const char* inGUIDString,
															bool inAuthedUserIsAdmin,
															CFMutableDictionaryRef inMutableRecordDict,
															unsigned int inHashList,
															CDSLocalPlugin* inPlugin,
															CDSLocalPluginNode* inNode,
															CFStringRef inAuthedUserName,
															uid_t inUID,
															uid_t inEffectiveUID,
															CFStringRef inNativeRecType,
															bool *inOutNodeReachable,
															tDirReference *inOutDSRef,
															tDataList **inOutDSNetworkNode,
															char *localCachedUserName,
															bool inOKToModifyAuthAuthority )
{
	tDirStatus				siResult				= eDSAuthFailed;
	tDirStatus				siResult2				= eDSAuthFailed;
	tDirNodeReference		aNodeRef				= 0;
	tDataBufferPtr			authDataBuff			= NULL;
	bool					nodeIsOnSearchPolicy	= *inOutNodeReachable;
	CFMutableDictionaryRef	nodeDict				= NULL;
	CAuthAuthority			tempAuthAuthorityList(inAuthAuthorityList);

	// node not reached until proven otherwise
	*inOutNodeReachable = false;
	
	if ( inAuthData == NULL )
		return( eDSNullAuthStepData );
	if ( localCachedUserName == NULL )
		return( eDSUserUnknown );
	
	switch ( inParams.uiAuthMethod )
	{
		case kAuthSetPolicyAsRoot:
		case kAuthSetCertificateHashAsRoot:
			if ( inEffectiveUID == 0 || inAuthedUserIsAdmin == true ) {
				nodeIsOnSearchPolicy = false; // just set node as not on the search policy to skip network check
			}
			else {
				return eDSAuthFailed;
			}
			break;	
	}

	// If the node is reachable, it's authoritative
	if ( nodeIsOnSearchPolicy && (*inOutDSNetworkNode) != NULL )
	{
		nodeDict = inPlugin->CopyNodeDictForNodeRef( inNodeRef );
		if ( nodeDict == NULL )
			return( eDSInvalidNodeRef );
		
		// we have to lock the node because we're changing the data
		siResult = OpenLDAPNode( inPlugin, nodeDict, *inOutDSNetworkNode, inOutDSRef, &aNodeRef );
		if ( siResult == eDSNoErr )
		{
			// user records only here
			// construct new authData with network user name
			siResult = (tDirStatus)RepackBufferForPWServer( inAuthData, localCachedUserName, 1, &authDataBuff );
			if ( siResult == eDSNoErr )
			{
				tDataNodePtr authMethodNodePtr = dsDataNodeAllocateString( 0, inParams.mAuthMethodStr );
				siResult = dsDoDirNodeAuth( aNodeRef, authMethodNodePtr, inAuthOnly, authDataBuff, outAuthData, nil );
				dsDataNodeDeAllocate( 0, authMethodNodePtr );
				
				DbgLog(kLogPlugin, "CDSLocalAuthHelper::DoLocalCachedUserAuthPhase2(): dsDoDirNodeAuth = %d", siResult);
			}
			switch( siResult )
			{
				case eDSNoErr:
				case eDSNotAuthorized:
				case eDSAuthUnknownUser:
					*inOutNodeReachable = true;
					// try local auth
					break;
				
				case eDSInvalidNodeRef:
				case eDSMaxSessionsOpen:
				case eDSCannotAccessSession:
				case eDSAuthNoAuthServerFound:
				case eDSAuthMasterUnreachable:
					// try local auth
					break;
				
				case eDSAuthAccountDisabled:
					*inOutNodeReachable = true;
					if ( inOKToModifyAuthAuthority )
					{
						inAuthAuthorityList.SetValueDisabledForTag( kDSTagAuthAuthorityLocalCachedUser );
						SaveAuthAuthorities( inPlugin, inNodeRef, inParams.pUserName, inNativeRecType, inAuthAuthorityList );
					}
					goto cleanup;
					
				default:
					*inOutNodeReachable = true;
					DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoLocalCachedUserAuthPhase2(): dsDoDirNodeAuth = %d", siResult );
					goto cleanup;
			}
		}
		else if ( siResult == eDSAuthMasterUnreachable || siResult == eDSOpenNodeFailed || siResult == eDSNodeNotFound || 
				  siResult == eDSCannotAccessSession || (siResult >= ePlugInDataError && siResult <= ePlugInCallTimedOut) )
		{
			// try local auth
			siResult = eDSCannotAccessSession;
		}
		else
		{
			DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoLocalCachedUserAuthPhase2(): OpenLDAPNode = %d", siResult );
			goto cleanup;
		}
	}
	else
	{
		siResult = eDSCannotAccessSession;
	}
	
	// don't allow password changes while offline
	if ( *inOutNodeReachable == false )
	{
		switch( inParams.uiAuthMethod )
		{
			case kAuthSetPasswd:
			case kAuthSetPasswdAsRoot:
			case kAuthChangePasswd:
			case kAuthSetPolicy:
			case kAuthSetUserName:
			case kAuthSetUserData:
			case kAuthSetShadowHashWindows:
			case kAuthSetShadowHashSecure:
				goto cleanup;
				break;
			
			default:
				break;
		}
	}
	
	// set up a ShadowHash authority to auth/update the local version of the account
	tempAuthAuthorityList.SetValueForTag( kDSTagAuthAuthorityShadowHash, kDSValueAuthAuthorityShadowHash kLocalCachedUserHashList );
	
	// replay the request to the ShadowHash handler
	siResult2 = CDSLocalAuthHelper::DoShadowHashAuth(
									inNodeRef,
									inParams,
									inOutContinueData,
									inAuthData,
									outAuthData,
									inAuthOnly,
									isSecondary,
									tempAuthAuthorityList,
									inGUIDString,
									inAuthedUserIsAdmin,
									inMutableRecordDict,
									inHashList,
									inPlugin,
									inNode,
									inAuthedUserName,
									inUID,
									inEffectiveUID,
									inNativeRecType,
									kDoNotTouchTheAuthAuthorities );

	DbgLog(kLogPlugin, "CDSLocalAuthHelper::DoLocalCachedUserAuthPhase2(): DoShadowHashAuth = %d", siResult2);
	if ( siResult == eDSNoErr )
	{
		// fix the local password if it was changed on the server
		if ( siResult2 != eDSNoErr )
		{
			char *name = NULL;
			char *pwd = NULL;
			UInt32 hashTotalLength = 0;
			unsigned char generatedHashes[kHashTotalLength];
			
			siResult2 = (tDirStatus) Get2FromBuffer( inAuthData, NULL, &name, &pwd, NULL );
			if ( siResult2 == eDSNoErr )
			{
				GenerateShadowHashes( gServerOS, pwd, strlen(pwd), inHashList, NULL, generatedHashes, &hashTotalLength );
				siResult2 = CDSLocalAuthHelper::WriteShadowHash( name, inGUIDString, generatedHashes );
			}
			
			DSFreeString( name );
			DSFreePassword( pwd );
		}
		
		siResult = siResult2;
	}
	else if ( *inOutNodeReachable == false )
	{
		// Network offline, use local auth error code
		siResult = siResult2;
	}
	
	if ( inOKToModifyAuthAuthority && siResult == eDSAuthAccountDisabled )
	{
		inAuthAuthorityList.SetValueDisabledForTag( kDSTagAuthAuthorityLocalCachedUser );
		siResult = SaveAuthAuthorities( inPlugin, inNodeRef, inParams.pUserName, inNativeRecType, inAuthAuthorityList );
		if ( siResult != eDSNoErr )
			DbgLog( kLogPlugin, "CDSLocalAuthHelper::DoLocalCachedUserAuthPhase2(): SaveAuthAuthorities = %d", siResult );
	}
	
cleanup:
	DSCFRelease( nodeDict );
	
	if ( authDataBuff != NULL )
        dsDataBufferDeallocatePriv( authDataBuff );
	
	if (aNodeRef != 0) {
		dsCloseDirNode(aNodeRef);
		aNodeRef = 0;
	}

	return( siResult );
}

#endif // DISABLE_LOCAL_PLUGIN
