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
 * @header SMBAuth
 */


#ifndef __SMBAUTH_H__
#define	__SMBAUTH_H__		1

#include <CoreFoundation/CoreFoundation.h>

/* utility functions prototypes */
#ifdef __cplusplus
extern "C" {
#endif

	void CalculateSMBNTHash(const char *utf8Password, unsigned char outHash[16]);
	void CalculateSMBLANManagerHash(const char *password, unsigned char outHash[16]);
	int32_t LittleEndianCharsToInt32( const char *inCharPtr );
	void CalculateWorkstationCredentialStrongSessKey( const unsigned char inNTHash[16], const char serverChallenge[8], const char clientChallenge[8], unsigned char outWCSK[16] );
	void CalculateWorkstationCredentialSessKey( const unsigned char inNTHash[16], const char serverChallenge[8], const char clientChallenge[8], unsigned char outWCSK[8] );
	void CalculatePPTPSessionKeys( const unsigned char inNTHash[16], const unsigned char inNTResponse[24], int inSessionKeyLen, unsigned char *outSendKey, unsigned char *outReceiveKey );
	void GetMasterKey( const unsigned char inNTHashHash[16], const unsigned char inNTResponse[24], unsigned char outMasterKey[16] );
	void GetAsymetricStartKey( unsigned char inMasterKey[16], unsigned char *outSessionKey, int inSessionKeyLen, bool inIsSendKey, bool inIsServer );
	void CalculateP24(unsigned char *P21, unsigned char *C8, unsigned char *P24);
	int NTLMv2(unsigned char *V2, unsigned char *inNTHash,
		 const char *authid, const char *target,
		 const unsigned char *challenge,
		 const unsigned char *blob, unsigned bloblen);
	void CalculateNTLMv2SessionKey(
			const unsigned char *inServerChallenge,
			const unsigned char *inClientChallenge,
			const unsigned char *inNTLMHash,
			unsigned char *outSessionKey );
	void DESEncode(const void *str, void *data);
	void MD4Encode(unsigned char *output, const unsigned char *input, unsigned int len);
	void str_to_key(unsigned char *str, unsigned char *key);
	void LittleEndianUnicodeToUnicode(const u_int16_t *unistr, int unistrLen, u_int16_t *unicode);

#ifdef __cplusplus
}
#endif

#endif
