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

#ifndef __PPS_H__
#define __PPS_H__

#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <sasl/sasl.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonCryptor.h>
#include <DirectoryService/DirServicesTypes.h>
#include "digestmd5.h"

typedef struct CommonAuthDataBlock {
	int step;
	char *outPtr;
	unsigned char *salt;
	int saltLen;
	char *serverChallenge;
	char *peerChallenge;
	int encryptedChallengeLen;
	int hashType;
	unsigned char sessionKey[CC_MD5_DIGEST_LENGTH];	
	unsigned char response[CC_SHA256_DIGEST_LENGTH];
	uint32_t nonce;
	int encryptedNonceLen;
	AES_KEY sessionEncryptKey;
	AES_KEY sessionDecryptKey;
	unsigned char encrypt_ivec[kCCBlockSizeAES128];
	unsigned char decrypt_ivec[kCCBlockSizeAES128];
	
	// encode/decode support
	buffer_info_t *enc_in_buf;
    unsigned int encode_buf_len;
    char *encode_buf;
	char *decode_buf;
	char *decode_once_buf;
    unsigned int decode_buf_len;
	unsigned int decode_once_buf_len;
} CommonAuthDataBlock, *CommonAuthDataBlockPtr;

typedef struct ServerAuthDataBlock {
	CommonAuthDataBlock c;
	char *userName;
	unsigned char *saltedHash;
	int saltedHashLen;
	unsigned char saltedHashHash[CC_MD5_DIGEST_LENGTH];
	unsigned char *encryptedChallenge;
	int peerChallengeLen;
	char encryptedNonce[64];
	char *proxyNode;
	tDirReference dsRef;
	tDirNodeReference nodeRef;
	tDataBufferPtr authBuff;
	tDataBufferPtr authStepBuff;
	tContextData continueData;
	tDataNodePtr typeBuff;
} ServerAuthDataBlock, *ServerAuthDataBlockPtr;

typedef struct ClientAuthDataBlock {
	CommonAuthDataBlock c;
	const char *authenticatorName;
	const char *userName;
	sasl_secret_t *password;
	unsigned int free_password;
	int serverChallengeLen;
	char *encryptedPeerChallenge;
	int encryptedPeerChallengeLen;
	char *encryptedNonce;
} ClientAuthDataBlock, *ClientAuthDataBlockPtr;

__BEGIN_DECLS

/* server side */

void
server_step_0_set_hash(const unsigned char *inSaltedSHA1Hash, ServerAuthDataBlockPtr inOutAuthData);

int
server_step_1(const char *inClientData, ServerAuthDataBlockPtr inOutAuthData);

int
server_step_2(const char *inClientData, ServerAuthDataBlockPtr inOutAuthData);

int
pps_server_mech_step(
	ServerAuthDataBlockPtr contextPtr,
	const char *clientin,
	unsigned clientinlen,
	const char **serverout,
	unsigned *serveroutlen );

void
pps_server_mech_dispose(ServerAuthDataBlockPtr contextPtr);

__END_DECLS

#endif
