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

#ifndef __BUFFER_UNPACKERS_H__
#define __BUFFER_UNPACKERS_H__

#include <Kerberos/krb5.h>
#include "CDSAuthDefs.h"
#include "digestmd5.h"

SInt32 Get2FromBuffer					(	tDataBufferPtr inAuthData,
											tDataList **inOutDataList,
											char **inOutItemOne,
											char **inOutItemTwo,
											unsigned int *outItemCount );

tDirStatus GetNameAndDataFromBuffer		(	tDataBufferPtr inAuthData,
											tDataList **inOutDataList,
											char **inOutItemOne,
											unsigned char **inOutItemTwo,
											UInt32 *outItemTwoLength,
											unsigned int *outItemCount );
tDirStatus GetDataFromAuthBuffer		(	tDataBufferPtr inAuthData,
                                			int nodeNum,
                                			unsigned char **outData,
                                			UInt32 *outLen );


SInt32 UnpackSambaBufferFirstThreeItems	(	tDataBufferPtr inAuthData,
											tDataListPtr *outDataList,
											char **outUserName, 
											unsigned char *outChallenge,
											UInt32 *outChallengeLen,
											unsigned char **outResponse,
											UInt32 *outResponseLen );

SInt32 UnpackSambaBuffer				(	tDataBufferPtr inAuthData,
											char **outUserName, 
											unsigned char *outC8,
											unsigned char *outP24 );

SInt32 UnpackNTLMv2Buffer				(	tDataBufferPtr inAuthData,
											char **outNIName,
											unsigned char *outChal,
											unsigned char **outDigest,
											UInt32 *outDigestLen,
											char **outSambaName,
											char **outDomain );

SInt32 UnpackMSCHAPv2Buffer				(	tDataBufferPtr inAuthData,
											char **outNIName,
											unsigned char *outChal,
											unsigned char **outPeerChal,
											unsigned char **outDigest,
											UInt32 *outDigestLen,
											char **outSambaName);

SInt32 UnpackMPPEKeyBuffer				(	tDataBufferPtr inAuthData,
											char **outUserName,
											unsigned char *outP24,
											int *outKeySize );

SInt32 UnpackDigestBuffer				(	tDataBufferPtr inAuthData,
											char **outUserName,
											digest_context_t *digestContext );

SInt32 UnpackCramBuffer					(	tDataBufferPtr inAuthData,
											char **outUserName,
											char **outChal,
											unsigned char **outResponse,
											UInt32 *outResponseLen );

SInt32 UnpackAPOPBuffer					(	tDataBufferPtr inAuthData,
											char **outUserName,
											char **outChal,
											char **outResponse );

SInt32 RepackBufferForPWServer			(	tDataBufferPtr inBuff,
											const char *inUserID,
											UInt32 inUserIDNodeNum,
											tDataBufferPtr *outBuff );
											
SInt32 GetUserNameFromAuthBuffer		(	tDataBufferPtr inAuthData,
											UInt32 inUserNameIndex, 
											char **outUserName,
											int *outUserNameBufferLength = NULL );

SInt32 UnpackUserWithAABuffer			(	tDataBufferPtr inAuthData,
											UInt32 *outAACount,
											char **outAAList[] );

tDirStatus GetKrbCredentialFromAuthBuffer	(	tDataBufferPtr inAuthData,
												char **outUserName,
												char **outPrincipal,
												krb5_creds **outCred );

#endif
