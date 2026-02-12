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

#ifndef _CDSLocalAuthHelper_
#define _CDSLocalAuthHelper_	1

#include <CoreFoundation/CoreFoundation.h>
#include <PasswordServer/AuthFile.h>
#include <PasswordServer/SASLCode.h>
#include "CDSAuthDefs.h"
#include "CAuthAuthority.h"
#include "CDSLocalAuthParams.h"

#include <map>			//STL map class

#define HMAC_MD5_SIZE 16

typedef tDirStatus (*AuthAuthorityHandlerProc) (	tDirNodeReference inNodeRef,
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
													CFStringRef inNativeRecType );


typedef map<string, sHashAuthFailed*>	HashAuthFailedMap;
typedef HashAuthFailedMap::iterator		HashAuthFailedMapI;

class CDSLocalAuthHelper
{
	public:
		static tDirStatus		CopyUserNameFromAuthBuffer( tDataBufferPtr inAuthData, UInt32 inUserNameIndex,
									CFStringRef *outUserName );
		static bool				AuthAuthoritiesHaveTag( CFStringRef inRecordName, CFArrayRef inAuthAuthorities,
									CFStringRef inTag );
		static tDirStatus		DoKerberosAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoKerberosCertAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoShadowHashAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoShadowHashAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType,
									bool inOKToChangeAuthAuthorities );
		static tDirStatus		DoBasicAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoPasswordServerAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoDisabledAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoLocalCachedUserAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoNodeNativeAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									const char* inAuthAuthorityData, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoUnixCryptAuth( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoSetPassword( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList,
									const char* inAuthAuthorityData, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoSetPasswordAsRoot( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList,
									const char* inAuthAuthorityData, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDirStatus		DoChangePassword( tDirNodeReference inNodeRef, CDSLocalAuthParams &inPB,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									const char* inAuthAuthorityData, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType );
		static tDataList*		FindNodeForSearchPolicyAuthUser ( const char *userName );
		static bool				IsWriteAuthRequest ( UInt32 uiAuthMethod );
		static AuthAuthorityHandlerProc		GetAuthAuthorityHandler ( const char* inTag );

	private:
		static tDirStatus		SetUserPolicies( CFMutableDictionaryRef inMutableRecordDict, CDSLocalPlugin* inPlugin,
									CDSLocalPluginNode* inNode, CFStringRef inAuthedUserName, uid_t inUID,
									uid_t inEffectiveUID, CFStringRef inNativeRecType, const char *inUsername,
									const char *inPolicyStr, bool inAuthedUserIsAdmin, tDirNodeReference inNodeRef,
									sHashState *inOutHashState );
		static tDirStatus		SetUserAAtoDisabled( CFMutableDictionaryRef inMutableRecordDict,
									CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode, CFStringRef inStdRecType,
									const char *inUsername, tDirNodeReference inNodeRef, CFStringRef inAuthedUserName,
									uid_t inEffectiveUID );
		static tDirStatus		SetUserAuthAuthorityAsRoot(	CFMutableDictionaryRef inMutableRecordDict,
															CDSLocalPlugin* inPlugin,
															CDSLocalPluginNode* inNode,
															CFStringRef inStdRecType,
															const char *inUsername,
															CAuthAuthority &inAuthAuthorityList,
															tDirNodeReference inNodeRef );
		static tDirStatus		GetHashSecurityLevelForUser( const char *inHashList, unsigned int *outHashList );
		static bool				GetHashSecurityBitsForString( const char *inHashType, unsigned int *inOutHashList );
		static tDirStatus		WriteShadowHash ( const char *inUserName, const char *inGUIDString,
									unsigned char inHashes[kHashTotalLength] );
		static void				RemoveShadowHash ( const char *inUserName, const char *inGUIDString, bool bShadowToo );
		static bool				HashesEqual( const unsigned char *inUserHashes, const unsigned char *inGeneratedHashes );
		static int				WriteHashStateFile( const char *inFilePath, sHashState *inHashState );
		static tDirStatus		ReadShadowHashAndStateFiles( const char *inUserName, const char *inGUIDString,
									unsigned char outHashes[kHashTotalLength], struct timespec *outModTime,
									char **outUserHashPath, char **outStateFilePath, sHashState *inOutHashState,
									SInt32 *outHashDataLen );
		static tDirStatus		GetShadowHashGlobalPolicies( CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode,
									PWGlobalAccessFeatures *inOutGAccess, PWGlobalMoreAccessFeatures *inOutGMoreAccess );
		static tDirStatus		SetShadowHashGlobalPolicies( CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode,
									tDirNodeReference inNodeRef, CFStringRef inAuthedUserName, uid_t inUID,
									uid_t inEffectiveUID, PWGlobalAccessFeatures *inGAccess,
									PWGlobalMoreAccessFeatures *inGMoreAccess );
		static void				GenerateShadowHashes( bool inServerOS, const char *inPassword, long inPasswordLen,
									UInt32 inAdditionalHashList, const unsigned char *inSHA1Salt, unsigned char *outHashes,
									UInt32 *outHashTotalLength );
		static tDirStatus		UnobfuscateRecoverablePassword( unsigned char *inData, unsigned char **outPassword,
									UInt32 *outPasswordLength );
		static tDirStatus		MSCHAPv2( const unsigned char *inC16, const unsigned char *inPeerC16,
									const unsigned char *inNTLMDigest, const char *inSambaName,
									const unsigned char *inOurHash, char *outMSCHAP2Response );
		static tDirStatus		CRAM_MD5( const unsigned char *inHash, const char *inChallenge,
									const unsigned char *inResponse );
		static void				hmac_md5_import( HMAC_MD5_CTX *hmac, HMAC_MD5_STATE *state );
		static void				hmac_md5_final( unsigned char digest[HMAC_MD5_SIZE], HMAC_MD5_CTX *hmac );
		static tDirStatus		Verify_APOP( const char *userstr, const unsigned char *inPassword,
									UInt32 inPasswordLen, const char *challenge, const char *response );
		static tDirStatus		PasswordOkForPolicies( const char *inSpaceDelimitedPolicies,
									PWGlobalAccessFeatures *inGAccess, const char *inUsername, const char *inPassword );
		static tDirStatus		TestPolicies( const char *inSpaceDelimitedPolicies, PWGlobalAccessFeatures *inGAccess,
									sHashState *inOutHashState, struct timespec *inModDateOfPassword,
									const char *inHashPath );
									
		/* Panther Migration: basic -> ShadowHash */
		static tDirStatus		MigrateToShadowHash( tDirNodeReference inNodeRef, CDSLocalPlugin* inPlugin,
									CDSLocalPluginNode* inNode, CFMutableDictionaryRef inMutableRecordDict,
									const char *inUserName, const char *inPassword, bool *outResetCache,
									unsigned int inHashList, CFStringRef inNativeRecType );
		
		/* Leopard Migration: ShadowHash -> ShadowHash + Kerberosv5 */
		static tDirStatus		MigrateAddKerberos( tDirNodeReference inNodeRef, CDSLocalAuthParams &inParams,
									tContextData *inOutContinueData, tDataBufferPtr inAuthData,
									tDataBufferPtr outAuthData, bool inAuthOnly, bool isSecondary,
									CAuthAuthority &inAuthAuthorityList, const char* inGUIDString, bool inAuthedUserIsAdmin, 
									CFMutableDictionaryRef inMutableRecordDict, unsigned int inHashList,
									CDSLocalPlugin *inPlugin, CDSLocalPluginNode *inNode, CFStringRef inAuthedUserName,
									uid_t inUID, uid_t inEffectiveUID, CFStringRef inNativeRecType,	
									bool inOKToChangeAuthAuthorities );
		
		static tDirStatus		PWSetReplicaData( CDSLocalPlugin* inPlugin, CDSLocalPluginNode* inNode,
									tDirNodeReference inNodeRef, tDirNodeReference inPWSNodeRef,
									const char *inAuthorityData );
		static tDirStatus		LocalCachedUserReachable(
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
									char **localCachedUserName );
								
		static tDirStatus		DoLocalCachedUserAuthPhase2(
									tDirNodeReference inNodeRef,
									CDSLocalAuthParams &inPB,
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
									tDataList **outDSNetworkNode,
									char *localCachedUserName,
									bool inOKToModifyAuthAuthority = true );
};

#endif

#endif // DISABLE_LOCAL_PLUGIN
