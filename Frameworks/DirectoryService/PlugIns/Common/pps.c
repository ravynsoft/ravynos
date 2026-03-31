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

#include <stdlib.h>
#include <syslog.h>
#include <sasl/saslutil.h>
#include <sys/param.h>
#include <DirectoryService/DirServices.h>
#include <DirectoryService/DirServicesUtils.h>
#include <PasswordServer/PSUtilitiesDefs.h>
#include <DirectoryServiceCore/pps.h>

#define NONCE_SIZE_PPS			(30)
#define NONCE64_SIZE			((NONCE_SIZE_PPS + 3) * 4 / 3)
#define NONCE_LONG_COUNT		((NONCE_SIZE_PPS / sizeof(u_int32_t)) + 1)
#define kSaltSize				(4)
#define kSaltedHashLen			(kSaltSize + CC_SHA1_DIGEST_LENGTH)

// keys
#define kKeyUserName			"username="
#define kKeyUserID				"userid="
#define kKeyChallenge			"challenge="
#define kKeyHashType			"hashtype="
#define kKeySalt				"salt="
#define kKeyResponse			"response="
#define kKeyPeerChallenge		"peerchallenge="
#define kKeyNonce				"nonce="

// security flags
#define SASL_SEC_PPS_FLAGS		(SASL_SEC_NOPLAINTEXT | SASL_SEC_NOANONYMOUS | SASL_SEC_MUTUAL_AUTH)

// macros
#define ALIGN_TO_BLOCKSIZE(size, blocksize)		((size) + ((blocksize) - ((size) % (blocksize))))
#define SETERROR(A,args...)						syslog(LOG_ALERT, (A), ##args)

// protos
static int
server_step_1_parse_input(const char *inClientData, char **outUserName);

static char *
server_step_1_format(ServerAuthDataBlockPtr inAuthData);

static int
server_step_2_parse_input(const char *inClientData, ServerAuthDataBlockPtr inOutAuthData);

static char *
server_step_2_format(ServerAuthDataBlockPtr inAuthData);


#pragma mark -
#pragma mark UTILS
#pragma mark -

// --------------------------------------------------------------------------------
//	create_nonce
// --------------------------------------------------------------------------------
static int
create_nonce(char *nonce, size_t maxnonce)
{
	int result = 0;
	unsigned int idx = 0;
	unsigned int len = 0;
	
	union {
		long randomLongs[NONCE_LONG_COUNT];
		char randomData[NONCE_SIZE_PPS + 1];
	} u;
	
	char nonce64[NONCE64_SIZE];
	
	for (idx = 0; idx < NONCE_LONG_COUNT; idx++)
		u.randomLongs[idx] = arc4random();
	
	result = sasl_encode64(u.randomData, NONCE_SIZE_PPS, nonce64, sizeof(nonce64), &len);
	if ( result == SASL_OK )
		strlcpy( nonce, nonce64, maxnonce );
	
	return result;
}


#pragma mark -
#pragma mark SERVER
#pragma mark -

/* server side */

void
server_step_0_set_hash(
	const unsigned char *inSaltedSHA1Hash,
	ServerAuthDataBlockPtr inOutAuthData)
{	
	if (inSaltedSHA1Hash != NULL)
	{		
		inOutAuthData->c.saltLen = kSaltSize;
		inOutAuthData->c.salt = (unsigned char *) malloc(inOutAuthData->c.saltLen);
		memcpy(inOutAuthData->c.salt, inSaltedSHA1Hash, inOutAuthData->c.saltLen);
		
		inOutAuthData->saltedHashLen = inOutAuthData->c.saltLen + CC_SHA1_DIGEST_LENGTH;
		inOutAuthData->saltedHash = (unsigned char *) malloc(inOutAuthData->saltedHashLen);
		memcpy(inOutAuthData->saltedHash, inSaltedSHA1Hash, inOutAuthData->saltedHashLen);
	}
}


int
server_step_1(
	const char *inClientData,
	ServerAuthDataBlockPtr inOutAuthData)
{
	CCCryptorStatus status = kCCSuccess;
	size_t len = 0;
	int result = SASL_FAIL;
	size_t dataMoved = 0;
	unsigned char ivec[kCCBlockSizeAES128] = {0};
	CC_MD5_CTX ctx;
		
	result = server_step_1_parse_input(inClientData, &inOutAuthData->userName);
	if (result != SASL_OK)
		return result;
	
	/* get MD5(SaltedHash) */
	CC_MD5_Init(&ctx);
	CC_MD5_Update(&ctx, inOutAuthData->saltedHash, inOutAuthData->saltedHashLen);
	CC_MD5_Final(inOutAuthData->saltedHashHash, &ctx);
	
	inOutAuthData->c.serverChallenge = (char *)calloc(1, NONCE64_SIZE + kCCBlockSizeAES128 + 1);
	result = create_nonce(inOutAuthData->c.serverChallenge, NONCE64_SIZE);
	if (result != SASL_OK)
		return result;
	
	len = strlen(inOutAuthData->c.serverChallenge) + 1;
	inOutAuthData->c.encryptedChallengeLen = ALIGN_TO_BLOCKSIZE(len, kCCBlockSizeAES128);
	inOutAuthData->encryptedChallenge = (unsigned char *)malloc(inOutAuthData->c.encryptedChallengeLen);
	
	memcpy(ivec, inOutAuthData->saltedHash, kSaltSize);
	memcpy(ivec + kSaltSize, inOutAuthData->saltedHashHash, 12);
	
	status = CCCrypt(
				kCCEncrypt, kCCAlgorithmAES128, 0,
				inOutAuthData->saltedHashHash, kCCKeySizeAES128,
				ivec,
				(const unsigned char *)inOutAuthData->c.serverChallenge,
				inOutAuthData->c.encryptedChallengeLen,
				inOutAuthData->encryptedChallenge,
				inOutAuthData->c.encryptedChallengeLen,
				&dataMoved );
		
	inOutAuthData->c.hashType = 1;
	
	return SASL_OK;
}


static int
server_step_1_parse_input(const char *inClientData, char **outUserName)
{
	const char *tptr;
	const char *endPtr;
	char bufStr[512];
	size_t uname_len;
	unsigned long	binlen;
	
	if (inClientData == NULL || outUserName == NULL)
		return SASL_FAIL;
	
	tptr = strstr(inClientData, kKeyUserID);
	if (tptr != NULL)
	{
		// password server appended an ID
		tptr += sizeof(kKeyUserID) - 1;
		endPtr = strchr(tptr, ';');
		if (endPtr == NULL)
			strlcpy(bufStr, tptr, sizeof(bufStr));
		else {
			strlcpy(bufStr, tptr, MIN((size_t)(endPtr - tptr + 1), sizeof(bufStr)));
		}
		*outUserName = strdup( bufStr );
	}
	else
	{
		tptr = strstr(inClientData, kKeyUserName);
		if (tptr == NULL)
			return SASL_FAIL;
		tptr += sizeof(kKeyUserName) - 1;
		endPtr = strchr(tptr, ';');
		if (endPtr == NULL)
			strlcpy(bufStr, tptr, sizeof(bufStr));
		else {
			strlcpy(bufStr, tptr, MIN((size_t)(endPtr - tptr + 1), sizeof(bufStr)));
		}
		*outUserName = (char *) malloc( (uname_len = strlen(bufStr) * 3 / 4 + 4) );
		Convert64ToBinary(bufStr, *outUserName, uname_len, &binlen);
		(*outUserName)[binlen] = '\0';
	}
	
	return SASL_OK;
}


static char *
server_step_1_format(ServerAuthDataBlockPtr inAuthData)
{
	char saltStr[128] = {0};
	char encryptedChallengeStr[512];
	char buffStr[512];
	unsigned int len64;
	int len;
	int result;
	
	result = sasl_encode64((const char *)inAuthData->c.salt,
				inAuthData->c.saltLen,
				saltStr, sizeof(saltStr), &len64);

	result = sasl_encode64((const char *)inAuthData->encryptedChallenge,
				inAuthData->c.encryptedChallengeLen,
				encryptedChallengeStr, sizeof(encryptedChallengeStr), &len64);
	
	len = snprintf(buffStr, sizeof(buffStr),
					"salt=%s;challenge=%s;hashtype=%d",
					saltStr, encryptedChallengeStr, inAuthData->c.hashType);
	if (len >= (int)sizeof(buffStr))
		return NULL;
	
	return strdup(buffStr);	
}


int
server_step_2(const char *inClientData, ServerAuthDataBlockPtr inOutAuthData)
{
	int result;
	unsigned char ivec[kCCBlockSizeAES128] = {0};
	char nonceStr[256] = {0};
	CC_SHA256_CTX shaCtx;
	unsigned char calculated_response[CC_SHA256_DIGEST_LENGTH];
	
	result = server_step_2_parse_input(inClientData, inOutAuthData);
	if (result != SASL_OK)
		return result;
	
	/* check response */
	CC_SHA256_Init(&shaCtx);
	CC_SHA256_Update(&shaCtx, inOutAuthData->c.serverChallenge, strlen(inOutAuthData->c.serverChallenge));
	CC_SHA256_Update(&shaCtx, inOutAuthData->c.peerChallenge, strlen(inOutAuthData->c.peerChallenge));
	CC_SHA256_Update(&shaCtx, inOutAuthData->saltedHash, kSaltedHashLen);
	CC_SHA256_Final(calculated_response, &shaCtx);
	
	if (memcmp(calculated_response, inOutAuthData->c.response, CC_SHA256_DIGEST_LENGTH) != 0)
		return SASL_BADAUTH;
	
	sprintf(nonceStr, "%u", inOutAuthData->c.nonce + 1);
	inOutAuthData->c.encryptedNonceLen = sizeof(inOutAuthData->encryptedNonce);
	result = AES_set_encrypt_key(inOutAuthData->c.sessionKey, 128, &inOutAuthData->c.sessionEncryptKey);
	AES_cbc_encrypt(
				(const unsigned char *)nonceStr,
				(unsigned char *)inOutAuthData->encryptedNonce,
				inOutAuthData->c.encryptedNonceLen,
				&inOutAuthData->c.sessionEncryptKey,
				ivec,
				AES_ENCRYPT);	
	
	return SASL_OK;
}


static int
server_step_2_parse_input(const char *inClientData, ServerAuthDataBlockPtr inOutAuthData)
{
	const char *tptr;
	const char *endPtr;
	char response64Str[256] = {0};
	char peerchal64Str[512] = {0};
	unsigned char encPeerChal[256] = {0};
	unsigned long	encPeerChalLen = 0;
	char nonce64Str[256] = {0};
	unsigned long len;
	unsigned char encNonce[256];
	unsigned char decNonce[256];
	unsigned long	encNonceLen = 0;
	unsigned char ivec[kCCBlockSizeAES128] = {0};
	int result;
	AES_KEY aesEncryptKey;
	CC_MD5_CTX ctx;

	/* response */
	tptr = strstr(inClientData, kKeyResponse);
	if (tptr == NULL)
		return SASL_FAIL;
	tptr += sizeof(kKeyResponse) - 1;
	endPtr = strchr(tptr, ';');
	if (endPtr == NULL)
		return SASL_FAIL;
	len = endPtr - tptr + 1;
	strlcpy(response64Str, tptr, len);
	Convert64ToBinary(
					response64Str,
					(char *)inOutAuthData->c.response,
					sizeof(inOutAuthData->c.response),
					&len);
	
	/* peer challenge */
	tptr = strstr(inClientData, kKeyPeerChallenge);
	if (tptr == NULL)
		return SASL_FAIL;
	tptr += sizeof(kKeyPeerChallenge) - 1;
	endPtr = strchr(tptr, ';');
	if (endPtr == NULL)
		return SASL_FAIL;
	len = endPtr - tptr + 1;
	strlcpy(peerchal64Str, tptr, len);
	Convert64ToBinary(peerchal64Str, (char *)encPeerChal, sizeof(encPeerChal), &encPeerChalLen);

	/* decrypt peer challenge */
	inOutAuthData->c.peerChallenge = (char *)malloc(256);
	memcpy(ivec, inOutAuthData->saltedHash, kSaltSize);
	memcpy(ivec + kSaltSize, inOutAuthData->saltedHashHash, 12);
	result = AES_set_decrypt_key(inOutAuthData->saltedHashHash, CC_MD5_DIGEST_LENGTH * 8, &aesEncryptKey);
	AES_cbc_encrypt(
				encPeerChal,
				(unsigned char *)inOutAuthData->c.peerChallenge,
				encPeerChalLen,
				&aesEncryptKey,
				ivec,
				AES_DECRYPT);
	
	/* make session key */
	CC_MD5_Init(&ctx);
	CC_MD5_Update(&ctx, inOutAuthData->c.serverChallenge, strlen(inOutAuthData->c.serverChallenge));
	CC_MD5_Update(&ctx, inOutAuthData->c.peerChallenge, strlen(inOutAuthData->c.peerChallenge));
	CC_MD5_Update(&ctx, inOutAuthData->saltedHashHash, CC_MD5_DIGEST_LENGTH);
	CC_MD5_Final(inOutAuthData->c.sessionKey, &ctx);

	/* nonce */
	tptr = strstr(inClientData, kKeyNonce);
	if (tptr == NULL)
		return SASL_FAIL;
	tptr += sizeof(kKeyNonce) - 1;
	endPtr = strchr(tptr, ';');
	if (endPtr == NULL)
		strlcpy(nonce64Str, tptr, sizeof(nonce64Str));
	else
		strlcpy(nonce64Str, tptr, MIN((size_t)(endPtr - tptr + 1), sizeof(nonce64Str)));
	Convert64ToBinary(nonce64Str, (char *)encNonce, sizeof(encNonce), &encNonceLen);
	
	bzero(ivec, sizeof(ivec));
	result = AES_set_decrypt_key(inOutAuthData->c.sessionKey, 128, &aesEncryptKey);
	AES_cbc_encrypt(
				(const unsigned char *)encNonce,
				decNonce,
				encNonceLen,
				&aesEncryptKey,
				ivec,
				AES_DECRYPT);
	
	sscanf((char *)decNonce, "%u", &inOutAuthData->c.nonce);
	
	return SASL_OK;
}


static char *
server_step_2_format(ServerAuthDataBlockPtr inAuthData)
{
	char encryptedNonceStr[128] = {0};
	char buffStr[512];
	unsigned int len64;
	int len;
	int result;
	
	result = sasl_encode64(inAuthData->encryptedNonce,
				inAuthData->c.encryptedNonceLen,
				encryptedNonceStr, sizeof(encryptedNonceStr), &len64);
	if (result != SASL_OK)
		return NULL;
	
	len = snprintf(buffStr, sizeof(buffStr), "nonce=%s", encryptedNonceStr);
	if (len >= (int)sizeof(buffStr))
		return NULL;
	
	return strdup(buffStr);	
}


int
pps_server_mech_step(
	ServerAuthDataBlockPtr contextPtr,
	const char *clientin,
	unsigned clientinlen,
	const char **serverout,
	unsigned *serveroutlen )
{
	int result = 0;
	
	switch ( contextPtr->c.step )
	{
		case 0:
			result = server_step_1(clientin, contextPtr);
			if (result != SASL_OK)
				return result;
			if ((contextPtr->c.outPtr = server_step_1_format(contextPtr)) == NULL)
				return SASL_FAIL;
			
			*serverout = contextPtr->c.outPtr;
			*serveroutlen = strlen(*serverout);
			contextPtr->c.step++;
			break;
		
		case 1:
			result = server_step_2(clientin, contextPtr);
			if (result != SASL_OK)
				return result;
			if (contextPtr->c.outPtr != NULL)
				free(contextPtr->c.outPtr);
			if ((contextPtr->c.outPtr = server_step_2_format(contextPtr)) == NULL)
				return SASL_FAIL;

			*serverout = contextPtr->c.outPtr;
			*serveroutlen = strlen(*serverout);
			
			/*
			oparams->encode = pps_encode;
			oparams->decode = pps_decode;
			oparams->mech_ssf = 128;
			oparams->doneflag = 1;
			*/
			contextPtr->c.step++;
			break;
				
		default:
			//oparams->doneflag = 1;
			return SASL_FAIL;
	}
	
	return (contextPtr->c.step == 2) ? SASL_OK : SASL_CONTINUE;
}


void
pps_server_mech_dispose(ServerAuthDataBlockPtr contextPtr)
{	
	if ( contextPtr != NULL )
	{
		if ( contextPtr->userName )
			free( contextPtr->userName );
		if ( contextPtr->c.outPtr )
			free( contextPtr->c.outPtr );
		if ( contextPtr->c.salt )
			free( contextPtr->c.salt );
		if ( contextPtr->saltedHash )
			free( contextPtr->saltedHash );
		if ( contextPtr->c.serverChallenge )
			free( contextPtr->c.serverChallenge );
		if ( contextPtr->encryptedChallenge )
			free( contextPtr->encryptedChallenge );
		if ( contextPtr->c.peerChallenge )
			free( contextPtr->c.peerChallenge );
		
		if (contextPtr->dsRef != 0)
		{
			if (contextPtr->nodeRef != 0)
				dsCloseDirNode(contextPtr->nodeRef);
			if (contextPtr->authBuff)
				dsDataBufferDeAllocate(contextPtr->dsRef, contextPtr->authBuff);
			if (contextPtr->authStepBuff)
				dsDataBufferDeAllocate(contextPtr->dsRef, contextPtr->authStepBuff);
			if (contextPtr->typeBuff)
				dsDataNodeDeAllocate(contextPtr->dsRef, contextPtr->typeBuff);
			
			dsCloseDirService(contextPtr->dsRef);
		}
		
		free(contextPtr);
	}
}

