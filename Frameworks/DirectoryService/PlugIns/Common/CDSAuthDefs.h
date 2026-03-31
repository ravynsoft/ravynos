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
 * @header CDSAuthDefs
 */

#ifndef __CDSAuthDefs_h__
#define __CDSAuthDefs_h__		1

#include <time.h>

#include <DirectoryServiceCore/PrivateTypes.h>
#include <DirectoryService/DirServicesConst.h>
#include <DirectoryService/DirServicesTypes.h>
#include <DirectoryService/DirServices.h>
#include <DirectoryService/DirServicesUtils.h>

// --------------------------------------------------------------------------------
//	Hash Length Constants
#define		kHashShadowChallengeLength			8
#define		kHashShadowKeyLength				21
#define		kHashShadowResponseLength			24
#define		kHashShadowOneLength				16
#define		kHashShadowBothLength				32
#define		kHashSecureLength					20
#define		kHashCramLength						32
#define		kHashSaltedSHA1Length				24
#define		kHashRecoverableLength				512 //also used to limit size of password to 511 chars

#define		kHashOffsetToNT						(0)
#define		kHashOffsetToLM						(kHashShadowOneLength)
#define		kHashOffsetToSHA1					(kHashOffsetToLM + kHashShadowOneLength)
#define		kHashOffsetToCramMD5				(kHashShadowBothLength + kHashSecureLength)
#define		kHashOffsetToSaltedSHA1				(kHashOffsetToCramMD5 + kHashCramLength)
#define		kHashOffsetToRecoverable			(kHashOffsetToSaltedSHA1 + kHashSaltedSHA1Length)

#define		kHashTotalLength					(kHashShadowBothLength + kHashSecureLength + \
												 kHashCramLength + kHashSaltedSHA1Length + \
												 kHashRecoverableLength)
#define		kHashShadowBothHexLength			64
#define		kHashOldHexLength					104
#define		kHashTotalHexLength					(kHashTotalLength * 2)

// --------------------------------------------------------------------------------
//	More Hash Defines
#define		kAESVector							"qawe ptajilja;sdqawe ptajilja;sd"
#define		kShadowHashDirPath					"/var/db/shadow/hash/"
#define		kShadowHashOldDirPath				"/var/db/samba/hash/"
#define		kShadowHashStateFileSuffix			".state"
#define		kShadowHashRecordName				"shadowhash"
#define		kShadowHashNTLMv2Length				16
#define		kLocalCachedUserHashList			"HASHLIST:<SALTED-SHA1>"

using namespace std;

typedef struct {
	double  lastTime;
	double  nowTime;
	UInt32  failCount;
} sHashAuthFailed;

typedef struct {
	SInt16 disabled;
	UInt16 failedLoginAttempts;
	UInt16 newPasswordRequired;
	struct tm creationDate;
	struct tm lastLoginDate;
	struct tm modDateOfPassword;
} sHashState;

#endif // __CDSAuthDefs_h_
