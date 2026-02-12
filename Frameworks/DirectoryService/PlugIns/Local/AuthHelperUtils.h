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

#ifndef __AUTH_HELPER_UTILS__
#define __AUTH_HELPER_UTILS__

#include <CoreFoundation/CoreFoundation.h>
#include "CDSAuthDefs.h"
#include "CAuthAuthority.h"

#define kLocalKDCRealmCacheTimeout		5

__BEGIN_DECLS

tDirStatus GetStateFilePath( const char *inHashPath, char **outStateFilePath );

tDirStatus ReadShadowHash(
	const char *inUserName,
	const char *inGUIDString,
	unsigned char outHashes[kHashTotalLength],
	struct timespec *outModTime,
	char **outUserHashPath,
	SInt32 *outHashDataLen,
	bool readHashes );

tDirStatus ReadStateFile(
	const char *inUserName,
	const char *inGUIDString,
	struct timespec *outModTime,
	char **outUserHashPath,
	char **outStateFilePath,
	sHashState *inOutHashState,
	SInt32 *outHashDataLen );

int ReadHashStateFile( const char *inFilePath, sHashState *inOutHashState );

tDirStatus GetUserPolicies(
	CFMutableDictionaryRef inMutableRecordDict,
	sHashState* inState,
	CDSLocalPlugin* inPlugin,
	char** outPolicyStr );

tDirStatus
OpenPasswordServerNode(
	CDSLocalPlugin *inPlugin,
	CFMutableDictionaryRef inNodeDict,
	const char *inServerAddr,
	tDirReference *outDSRef,
	tDirNodeReference *outNodeRef );
	
tDirStatus
OpenLDAPNode(
	CDSLocalPlugin *inPlugin,
	CFMutableDictionaryRef inNodeDict,
	tDataListPtr inNodeName,
	tDirReference *outDSRef,
	tDirNodeReference *outNodeRef );

void
LoadAuthAuthorities(
	CDSLocalPlugin* inPlugin,
	tRecordReference inRecordRef,
	CAuthAuthority &inOutAATank );

tDirStatus
SaveAuthAuthorities(
	CDSLocalPlugin* inPlugin,
	tDirNodeReference inNodeRef,
	const char *inUsername,
	CFStringRef inNativeRecType,
	CAuthAuthority &inAATank );

tDirStatus
SaveAuthAuthoritiesWithRecordRef(
	CDSLocalPlugin* inPlugin,
	tDirNodeReference inNodeRef,
	tRecordReference inRecordRef,
	CAuthAuthority &inAATank );

tDirStatus
SetUserAuthAuthorityAsRoot(
	CFMutableDictionaryRef inMutableRecordDict,
	CDSLocalPlugin* inPlugin,
	CDSLocalPluginNode* inNode,
	CFStringRef inStdRecType,
	const char *inUsername,
	const char *inAuthAuthority,
	tDirNodeReference inNodeRef,
	bool inRemoveBasic );

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
	bool inOKToChangeAuthAuthorities );

char *
GetLocalKDCRealmWithCache(
	time_t inMaxTimeUntilQuery );

tDirStatus
SASLErrToDirServiceError(
	int inSASLError );

__END_DECLS


#endif

#endif //DISABLE_LOCAL_PLUGIN
