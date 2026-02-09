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
/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All Rights Reserved.
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

/*
 * NTLM client-side authentication engine. 
 *
 * In the usual absence of documentation from Microsoft, the "inventors" of this
 * protocol, this module was written using the superb revers engineering documented
 * at 
 *
 *     http://davenport.sourceforge.net/ntlm.html#localAuthentication
 */

#include "NtlmGenerator.h"
#include "ntlmBlobPriv.h"
#include <CoreServices/CoreServices.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <strings.h>
#include <security_cdsa_utils/cuCdsaUtils.h>

/* 
 * For debugging using fixed server challenge and client nonce. 
 */
#if		DEBUG_FIXED_CHALLENGE

/* these are "test vectors", effectively, from sourceforge */
/* use pwd SecREt01, host/domain DOMAIN */
static const unsigned char fixServerChallenge[8] = 
	{ 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
static const unsigned char fixClientNonce[8] = 
	{ 0xff, 0xff, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44 };

static const unsigned char fixTargetInfo[] = {
	0x02, 0x00, 0x0c, 0x00, 0x44, 0x00, 0x4f, 0x00,
	0x4d, 0x00, 0x41, 0x00, 0x49, 0x00, 0x4e, 0x00,
	0x01, 0x00, 0x0c, 0x00, 0x53, 0x00, 0x45, 0x00,
	0x52, 0x00, 0x56, 0x00, 0x45, 0x00, 0x52, 0x00,
	0x04, 0x00, 0x14, 0x00, 0x64, 0x00, 0x6f, 0x00,
	0x6d, 0x00, 0x61, 0x00, 0x69, 0x00, 0x6e, 0x00,
	0x2e, 0x00, 0x63, 0x00, 0x6f, 0x00, 0x6d, 0x00,
	0x03, 0x00, 0x22, 0x00, 0x73, 0x00, 0x65, 0x00, 
	0x72, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00,
	0x2e, 0x00, 0x64, 0x00, 0x6f, 0x00, 0x6d, 0x00,
	0x61, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x2e, 0x00,
	0x63, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x00, 0x00,
	0x00, 0x00
};
#endif

/* app's NtlmGeneratorRef is a pointer to one of these */
struct NtlmGenerator {
	NLTM_Which			mWhich;
	CSSM_CSP_HANDLE		mCspHand;
	NLTM_Which			mNegotiatedVersion;
	uint32_t			mSentFlags;			/* the flags we sent in first mst */
};	

static OSStatus _NtlmGeneratePasswordHashes(
	CFAllocatorRef alloc,
	NtlmGeneratorRef ntlm,
	CFStringRef password,
	CFDataRef* ntlmHash,
	CFDataRef* lmHash);
										  
/*
 * Validate type 2 message sent by the server; return interesting fields. 
 * NOTE we do not deal with the Context field here, which is only used
 * for local authetication.
 */
static OSStatus ntlmParseServerChallenge(
	CFDataRef		serverBlob,
	uint32_t		*serverFlags,		/* RETURNED */
	unsigned char   *challenge,			/* 8 bytes, mallocd by caller, RETURNED */
	unsigned char   **targetName,		/* mallocd and RETURNED */
	unsigned		*targetNameLen,		/* RETURNED */
	unsigned char   **targetInfo,		/* optionally mallocd and RETURNED */
	unsigned		*targetInfoLen)		/* optionally RETURNED */
{
	int minLength;
	
	*targetName = NULL;
	*targetNameLen = 0;
	*targetInfo = NULL;
	*targetInfoLen = 0;
	
	if(serverBlob == NULL) {
		return NTLM_ERR_PARSE_ERR;
	}
	
	minLength = NTLM_SIGNATURE_LEN +
		sizeof(uint32_t) +			/* msg type */
		NTLM_SIZEOF_SEC_BUF +		/* target name */
		sizeof(uint32_t) +			/* flags */
		NTLM_CHALLENGE_LEN;		
	CFIndex bufLen = CFDataGetLength(serverBlob);
	if(bufLen < minLength) {
		dprintf("ntlmParseServerChallenge: bad length\n");
		return NTLM_ERR_PARSE_ERR;
	}
	
	/* do not even think of touching serverBlob after this */
	const unsigned char *cp = CFDataGetBytePtr(serverBlob);
	
	/* byte 0: signature */
	if(memcmp(cp, NTLM_SIGNATURE, NTLM_SIGNATURE_LEN)) {
		dprintf("ntlmParseServerChallenge: signature mismatch\n");
		return NTLM_ERR_PARSE_ERR;
	}
	
	const unsigned char *currCp = cp + NTLM_SIGNATURE_LEN;
	
	/* byte 8: message type */
	uint32_t msgType = deserializeUint32(currCp);
	if(msgType != NTLM_MSG_MARKER_TYPE2) {
		dprintf("ntlmParseServerChallenge: bad msg type\n");
		return NTLM_ERR_PARSE_ERR;
	}
	currCp += sizeof(uint32_t);
	
	/* byte 12: target name, security buffer */
	const unsigned char *sbData;
	uint16_t sbLen;
	OSStatus ortn = ntlmParseSecBuffer(currCp, cp, bufLen, &sbData, &sbLen);
	if(ortn) {
		return ortn;
	}
	*targetName = (unsigned char *)malloc(sbLen);
	*targetNameLen = sbLen;
	memmove(*targetName, sbData, sbLen);
	currCp += NTLM_SIZEOF_SEC_BUF;
	
	/* byte 20: flags */
	*serverFlags = deserializeUint32(currCp);
	currCp += sizeof(uint32_t);
	
	/* byte 24: challenge */
	#if		DEBUG_FIXED_CHALLENGE
	memmove(challenge, fixServerChallenge, NTLM_CHALLENGE_LEN);
	#else
	memmove(challenge, currCp, NTLM_CHALLENGE_LEN);
	#endif
	currCp += NTLM_CHALLENGE_LEN;
	
	/* remaining fields optional */
	const unsigned char *endOfBuf = cp + bufLen;
	assert(endOfBuf >= currCp);
	if(endOfBuf == currCp) {
		return noErr;
	}
	
	if(endOfBuf < (currCp + NTLM_SIZEOF_SEC_BUF)) {
		/* not enough left for even one security buf; ignore */
		return noErr;
	}
	
	/* byte 32: context: skip */
	currCp += NTLM_SIZEOF_SEC_BUF;
	
	if(endOfBuf < (currCp + NTLM_SIZEOF_SEC_BUF)) {
		/* not enough left for target info security buf; ignore */
		return noErr;
	}
	
	/* byte 40: target info */
	ortn = ntlmParseSecBuffer(currCp, cp, bufLen, &sbData, &sbLen);
	if(ortn) {
		free(*targetName);
		*targetName = NULL;
		return ortn;
	}
	#if		DEBUG_FIXED_CHALLENGE
	sbData = fixTargetInfo;
	sbLen = sizeof(fixTargetInfo);
	#endif  /* DEBUG_FIXED_CHALLENGE */
	*targetInfo = (unsigned char *)malloc(sbLen);
	*targetInfoLen = sbLen;
	memmove(*targetInfo, sbData, sbLen);
	return noErr;
}

/* 
 * Create NTLMv2 responses (both NTLM and LM).
 */
static OSStatus ntlmGenerateNtlmV2Response(
	CSSM_CSP_HANDLE cspHand, 
	
	/* from app */
	CFStringRef		domain,		
	CFStringRef		userName,
	CFDataRef		ntlmHash,
	
	/* from server */
	const unsigned char *serverChallenge,
	const unsigned char *targetInfo,
	unsigned			targetInfoLen,
	
	/* returned */
	unsigned char	*lmV2Response,		// caller supplied, NTLM_LM_RESPONSE_LEN bytes
	unsigned char   **ntlmv2Response,   // mallocd and RETURNED
	unsigned		*ntlmV2ResponseLen) // RETURNED
{
	/* Random challenge used in both responses */
	unsigned char challenge[NTLM_CLIENT_NONCE_LEN];
	#if DEBUG_FIXED_CHALLENGE
	memmove(challenge, fixClientNonce, NTLM_CLIENT_NONCE_LEN);
	#else
	ntlmRand(NTLM_CLIENT_NONCE_LEN, challenge);
	#endif
	
	/* NTLM password hash */
	unsigned char ntlmPwdHash[NTLM_DIGEST_LENGTH];
//	ntlmPasswordHash(password, ntlmPwdHash);
	memmove(ntlmPwdHash, CFDataGetBytePtr(ntlmHash), sizeof(ntlmPwdHash));
	
	/* uppercase(userName | domain) */
	CFMutableStringRef userDomain = CFStringCreateMutableCopy(NULL, 0, userName);
	if(domain != NULL) {
		CFStringAppend(userDomain, domain);
	}
	CFStringUppercase(userDomain, NULL);
	
	/* declare some locals prior to any gotos */
	unsigned char *ucode = NULL;
	unsigned ucodeLen;
	unsigned char ntlmV2Hash[NTLM_DIGEST_LENGTH];
	unsigned char macText2[NTLM_CHALLENGE_LEN + NTLM_CLIENT_NONCE_LEN];
	unsigned char challengeMac[NTLM_DIGEST_LENGTH];
	unsigned char blobMac[NTLM_DIGEST_LENGTH];
	unsigned char *ntlmv2Resp = NULL;
	CFMutableDataRef ntlmV2Blob = NULL;
	CFMutableDataRef catBlob = NULL;
	unsigned ntlmV2BlobLen;
	unsigned char blobSig[4] = {0x01, 0x01, 0x00, 0x00};
	
	/* HMAC(passwordHash, uppercase(userName | domain)) */
	ntlmStringToLE(userDomain, &ucode, &ucodeLen);
	OSStatus ortn = ntlmHmacMD5(cspHand, ntlmPwdHash, NTLM_DIGEST_LENGTH,
		ucode, ucodeLen, ntlmV2Hash);
	if(ortn) {
		goto errOut;
	}
	
	/* HMAC(ntlmV2Hash, serverChallenge | clientChallenge) */
	memmove(macText2, serverChallenge, NTLM_CHALLENGE_LEN);
	memmove(macText2 + NTLM_CHALLENGE_LEN, challenge, NTLM_CLIENT_NONCE_LEN);
	ortn = ntlmHmacMD5(cspHand, ntlmV2Hash, NTLM_DIGEST_LENGTH,
		macText2, NTLM_CHALLENGE_LEN + NTLM_CLIENT_NONCE_LEN, challengeMac);
	if(ortn) {
		goto errOut;
	}

	/* LMv2 response := challengeMac | clientChallenge */
	memmove(lmV2Response, challengeMac, NTLM_DIGEST_LENGTH);
	memmove(lmV2Response + NTLM_DIGEST_LENGTH, challenge, NTLM_CLIENT_NONCE_LEN);
	
	/* Prepare the NTLMv2 'blob' */
	ntlmV2Blob = CFDataCreateMutable(NULL, 0);
	
	/* 0: 0x01010000 */
	CFDataAppendBytes(ntlmV2Blob, blobSig, 4);
	/* 4: reserved, zeroes */
	appendUint32(ntlmV2Blob, 0);
	/* 8: Timestamp */
	ntlmAppendTimestamp(ntlmV2Blob);
	/* 16: client challenge */
	CFDataAppendBytes(ntlmV2Blob, challenge, NTLM_CLIENT_NONCE_LEN);
	/* 24: unknown, zeroes */
	appendUint32(ntlmV2Blob, 0);
	/* 28: target info from server */
	CFDataAppendBytes(ntlmV2Blob, targetInfo, targetInfoLen);
	/* *: unknown, zeroes */
	appendUint32(ntlmV2Blob, 0);
	
	/* keep that blob; it'll go directly into the response. Now cook up 
	 * another one, the concatentation of the server challenge with the
	 * ntlmV2Blob */
	ntlmV2BlobLen = CFDataGetLength(ntlmV2Blob);
	catBlob = CFDataCreateMutable(NULL, 0);
	CFDataAppendBytes(catBlob, serverChallenge, NTLM_CHALLENGE_LEN);
	CFDataAppendBytes(catBlob, CFDataGetBytePtr(ntlmV2Blob), ntlmV2BlobLen);

	/* HMAC(ntlmV2Hash, serverChallenge | blob) */
	ortn = ntlmHmacMD5(cspHand, ntlmV2Hash, NTLM_DIGEST_LENGTH,
		CFDataGetBytePtr(catBlob), CFDataGetLength(catBlob),
		blobMac);
	if(ortn) {
		goto errOut;
	}
	
	/* Finally, NTLMv2 response := (blobMac | ntlmV2Blob) */
	ntlmv2Resp = (unsigned char *)malloc(NTLM_DIGEST_LENGTH + ntlmV2BlobLen);
	memmove(ntlmv2Resp, blobMac, NTLM_DIGEST_LENGTH);
	memmove(ntlmv2Resp + NTLM_DIGEST_LENGTH, CFDataGetBytePtr(ntlmV2Blob), ntlmV2BlobLen);
	*ntlmv2Response = ntlmv2Resp;
	*ntlmV2ResponseLen = NTLM_DIGEST_LENGTH + ntlmV2BlobLen;
	ortn = noErr;
errOut:
	if(userDomain) {
		CFRelease(userDomain);
	}
	if(ntlmV2Blob) {
		CFRelease(ntlmV2Blob);
	}
	if(catBlob) {
		CFRelease(catBlob);
	}
	CFREE(ucode);
	return ortn;
}

/*
 * Create/release NtlmGenerator objects.
 */
OSStatus NtlmGeneratorCreate(
	NLTM_Which			which,
	NtlmGeneratorRef	*ntlmGen)			/* RETURNED */
{
	struct NtlmGenerator *gen = 
		(struct NtlmGenerator *)malloc(sizeof(struct NtlmGenerator));
	if(gen == NULL) {
		return memFullErr;
	}
	gen->mWhich = which;
	gen->mCspHand = cuCspStartup(CSSM_TRUE);
	if(gen->mCspHand == 0) {
		return internalComponentErr;
	}
	gen->mNegotiatedVersion = 0;			/* i.e., unknown */
	gen->mSentFlags = 0;
	*ntlmGen = gen;
	return noErr;
}
	
void NtlmGeneratorRelease(
	NtlmGeneratorRef	ntlmGen)
{
	if(ntlmGen == NULL) {
		return;
	}
	if(ntlmGen->mCspHand) {
		cuCspDetachUnload(ntlmGen->mCspHand, CSSM_TRUE);
	}
	free(ntlmGen);
}

OSStatus NtlmCreateClientRequest(
	NtlmGeneratorRef	ntlmGen,
	CFDataRef			*clientRequest)		/* RETURNED */
{
	CFMutableDataRef req = CFDataCreateMutable(NULL, 0);
	if(req == NULL) {
		return memFullErr;
	}
	/* byte 0: signature, NULL terminated */
	CFDataAppendBytes(req, (UInt8 *)NTLM_SIGNATURE, NTLM_SIGNATURE_LEN);
		 
	/* byte 8: message type */
	appendUint32(req, NTLM_MSG_MARKER_TYPE1);
	
	/* byte 12: the standard flags we send - we're wide open to all types */
	/* FIXME isn't there a way to tell the server we support NTLMv2? */
	ntlmGen->mSentFlags = NTLM_NegotiateUnicode |
		NTLM_NegotiateOEM |
		NTLM_RequestTarget |
		NTLM_NegotiateNTLM |
		NTLM_AlwaysSign;
	if(ntlmGen->mWhich & NW_NTLM2) {
		ntlmGen->mSentFlags |= NTLM_NegotiateNTLM2Key;
	}
	appendUint32(req, ntlmGen->mSentFlags);
	
	/* byte 16: optional supplied domain: not needed */
	CFIndex dex;
	appendSecBuf(req, 0, &dex);
		
	/* byte 24: optional supplied workstation: not needed */
	appendSecBuf(req, 0, &dex);

	*clientRequest = req;
	return noErr;
}
	
/* 
 * The meat & potatoes: given a server type 2 message, cook up a type 3 response. 
 */
OSStatus NtlmCreateClientResponse(
	NtlmGeneratorRef	ntlmGen,
	CFDataRef			serverBlob,
	CFStringRef			domain,				/* optional */
	CFStringRef			userName,
	CFStringRef			password,
	CFDataRef			*clientResponse)	/* RETURNED */
{
	CFDataRef ntlmHash = NULL;
	CFDataRef lmHash = NULL;
	OSStatus result = _NtlmGeneratePasswordHashes(kCFAllocatorDefault, ntlmGen, password, &ntlmHash, &lmHash);
	
	if (result == noErr) {
		
		result = _NtlmCreateClientResponse(ntlmGen, serverBlob, domain, userName, ntlmHash, lmHash, clientResponse);
	}
	
	if (ntlmHash)
		CFRelease(ntlmHash);
	
	if (lmHash)
		CFRelease(lmHash);
	
	return result;
}

OSStatus _NtlmCreateClientResponse(
	NtlmGeneratorRef	ntlmGen,
	CFDataRef			serverBlob,
	CFStringRef			domain,				/* optional */
	CFStringRef			userName,
	CFDataRef			ntlmHash,
	CFDataRef			lmHash,
	CFDataRef			*clientResponse)	/* RETURNED */
{
	OSStatus		ortn;
	uint32_t		serverFlags;
	unsigned char   serverChallenge[NTLM_CHALLENGE_LEN];  
	unsigned char   *targetName = NULL;
	unsigned		targetNameLen = 0;
	unsigned char   *targetInfo = NULL;
	unsigned		targetInfoLen = 0;
	CFIndex			lmRespOffset;
	unsigned char   lmResp[NTLM_LM_RESPONSE_LEN];
	CFIndex			ntlmRespOffset;
	unsigned char   ntlmResp[NTLM_LM_RESPONSE_LEN];
	unsigned char   *ntlmResponsePtr = NULL;
	unsigned		ntlmResponseLen = 0;
	unsigned char   *domainNameFlat = NULL;
	unsigned		domainNameFlatLen = 0;
	CFIndex			domainNameOffset;
	unsigned char   *userNameFlat = NULL;
	unsigned		userNameFlatLen = 0;
	CFIndex			userNameOffset;
	unsigned char   *workstationName = NULL;
	unsigned		workstationNameLen = 0;
	CFIndex			workstationNameOffset;
	CFIndex			nullDex;
	unsigned char   pwdHash[NTLM_DIGEST_LENGTH];
	
	ortn = ntlmParseServerChallenge(serverBlob, &serverFlags, serverChallenge,
		&targetName, &targetNameLen,
		&targetInfo, &targetInfoLen);
	if(ortn) {
		return ortn;
	}
	/* subsequent errors to errOut: */

	/* gather negotiated parameters */
	bool lm2Key  = (serverFlags & NTLM_NegotiateNTLM2Key) ? true : false;
	bool unicode = (serverFlags & NTLM_NegotiateUnicode) ? true : false;
	/* any others? */
	
	CFMutableDataRef clientBuf = CFDataCreateMutable(NULL, 0);
	if(clientBuf == NULL) {
		ortn = memFullErr;
		goto errOut;
	}
	
	if (domain) {
		domain = CFStringCreateMutableCopy(NULL, 0, domain);
		if (domain)
			CFStringUppercase((CFMutableStringRef)domain, NULL);
		else {
			ortn = memFullErr;
			goto errOut;
		}
	}
	
	/* byte 0: signature, NULL terminated */
	CFDataAppendBytes(clientBuf, (UInt8 *)NTLM_SIGNATURE, NTLM_SIGNATURE_LEN);
		 
	/* byte 8: message type */
	appendUint32(clientBuf, NTLM_MSG_MARKER_TYPE3);
	
	/* LM and NTLM responses */
	if( (targetInfo != NULL) &&							// server is NTLMv2 capable
	    (targetInfoLen != 0) &&							// ditto
		(serverFlags & NTLM_NegotiateTargetInfo) &&		// ditto
		(ntlmGen->mWhich & NW_NTLMv2) ) {				// ...and we are
		/*
		 * NTLMv2
		 */
		ortn = ntlmGenerateNtlmV2Response(ntlmGen->mCspHand, 
			domain, userName, ntlmHash,
			serverChallenge, targetInfo, targetInfoLen,
			lmResp, &ntlmResponsePtr, &ntlmResponseLen);
		if(ortn) {
			goto errOut;
		}
		
		/* 
		 * Write security buffers.
		 * 
		 * byte 12: LM response
		 * byte 20: NTLM response
		 */
		appendSecBuf(clientBuf, NTLM_LM_RESPONSE_LEN, &lmRespOffset);
		appendSecBuf(clientBuf, ntlmResponseLen, &ntlmRespOffset);
		ntlmGen->mNegotiatedVersion = NW_NTLMv2;
	}
	else {
		if(lm2Key && (ntlmGen->mWhich & NW_NTLM2)) {
			/* LM response: 8 random bytes, rest zeroes */
			#if DEBUG_FIXED_CHALLENGE
			memmove(lmResp, fixClientNonce, NTLM_CLIENT_NONCE_LEN);
			#else
			ntlmRand(NTLM_CLIENT_NONCE_LEN, lmResp);
			#endif
			memset(lmResp + NTLM_CLIENT_NONCE_LEN, 0, 
				NTLM_LM_RESPONSE_LEN - NTLM_CLIENT_NONCE_LEN);
			
			/* session nonce: server challenge | client nonce */
			unsigned char sessionNonce[NTLM_CHALLENGE_LEN + NTLM_CLIENT_NONCE_LEN];
			memmove(sessionNonce, serverChallenge, NTLM_CHALLENGE_LEN);
			memmove(sessionNonce + NTLM_CHALLENGE_LEN, lmResp, NTLM_CLIENT_NONCE_LEN);
			
			/* NTLM2 session hash: the first 8 bytes of MD5(sessionNonce) */
			unsigned char sessionHash[NTLM_DIGEST_LENGTH];
			md5Hash(sessionNonce, NTLM_CHALLENGE_LEN + NTLM_CLIENT_NONCE_LEN, sessionHash);
			
			/* standard password hash */
//			ntlmPasswordHash(password, pwdHash);
			memmove(pwdHash, CFDataGetBytePtr(ntlmHash), sizeof(pwdHash));
			
			/* NTLM response: DES with three different keys */
			ortn = ntlmResponse(ntlmGen->mCspHand, pwdHash, sessionHash, ntlmResp);
			if(ortn) {
				dprintf("***Error on ntlmResponse (3)\n");
				goto errOut;
			}
			ntlmGen->mNegotiatedVersion = NW_NTLM2;
		}
		else if(ntlmGen->mWhich & NW_NTLM1) {
			/* 
			 * LM response - the old style 2-DES "password hash" applied
			 * the the server's challenge 
			 */
//			ortn = lmPasswordHash(ntlmGen->mCspHand, password, pwdHash);
//			if(ortn) {
//				dprintf("***Error on lmPasswordHash\n");
//				goto errOut;
//			}
			memmove(pwdHash, CFDataGetBytePtr(lmHash), sizeof(pwdHash));
			
			ortn = ntlmResponse(ntlmGen->mCspHand, pwdHash, serverChallenge, lmResp);
			if(ortn) {
				dprintf("***Error on ntlmResponse (1)\n");
				goto errOut;
			}
			
			/*
			 * NTLM response: md4 password hash, DES with three different keys 
			 */
//			ntlmPasswordHash(password, pwdHash);
			memmove(pwdHash, CFDataGetBytePtr(ntlmHash), sizeof(pwdHash));

			ortn = ntlmResponse(ntlmGen->mCspHand, pwdHash, serverChallenge, ntlmResp);
			if(ortn) {
				dprintf("***Error on ntlmResponse (2)\n");
				goto errOut;
			}
			ntlmGen->mNegotiatedVersion = NW_NTLM1;
		}
		else {
			dprintf("***NTLM protocol mismatch\n");
			ortn = NTLM_ERR_PROTOCOL_MISMATCH;
			goto errOut;
		
		}
		
		/* 
		 * Write security buffers.
		 * 
		 * byte 12: LM response
		 * byte 20: NTLM response
		 */
		appendSecBuf(clientBuf, NTLM_LM_RESPONSE_LEN, &lmRespOffset);
		appendSecBuf(clientBuf, NTLM_LM_RESPONSE_LEN, &ntlmRespOffset);
		ntlmResponsePtr = ntlmResp;
		ntlmResponseLen = NTLM_LM_RESPONSE_LEN;
	}   /* not NTLMv2 */
	
	/* 
	 * convert domain and user as appropriate
	 * byte 28: domain (server) name
	 */
	if(domain != NULL) {
		ortn = ntlmStringFlatten(domain, unicode, &domainNameFlat, &domainNameFlatLen);
		if(ortn) {
			dprintf("createClientResponse: error converting domain name\n");
			ortn = NTLM_ERR_PARSE_ERR;
			goto errOut;
		}
	}
	appendSecBuf(clientBuf, domainNameFlatLen, &domainNameOffset);

	/* byte 36: user name */
	ortn = ntlmStringFlatten(userName, unicode, &userNameFlat, &userNameFlatLen);
	if(ortn) {
		dprintf("createClientResponse: error converting user name\n");
		ortn = NTLM_ERR_PARSE_ERR;
		goto errOut;
	}
	appendSecBuf(clientBuf, userNameFlatLen, &userNameOffset);
	
	/* byte 44: hostname */
	ortn = ntlmHostName(unicode, &workstationName, &workstationNameLen);
	if(ortn) {
		dprintf("createClientResponse: error getting host name\n");
		goto errOut;
	}
	appendSecBuf(clientBuf, workstationNameLen, &workstationNameOffset);
	
	/* byte 52: session key (whatever that is): optional, empty here */
	appendSecBuf(clientBuf, 0, &nullDex);
	
	/* byte 60: negotiated flags */
	appendUint32(clientBuf, ntlmGen->mSentFlags & serverFlags);

	/* finally, the data associated with the security buffers */
	secBufOffset(clientBuf, lmRespOffset);
	CFDataAppendBytes(clientBuf, lmResp, NTLM_LM_RESPONSE_LEN);

	secBufOffset(clientBuf, ntlmRespOffset);
	CFDataAppendBytes(clientBuf, ntlmResponsePtr, ntlmResponseLen);

	if(domain != NULL) {
		secBufOffset(clientBuf, domainNameOffset);
		CFDataAppendBytes(clientBuf, domainNameFlat, domainNameFlatLen);
	}
	
	secBufOffset(clientBuf, userNameOffset);
	CFDataAppendBytes(clientBuf, userNameFlat, userNameFlatLen);
	
	secBufOffset(clientBuf, workstationNameOffset);
	CFDataAppendBytes(clientBuf, workstationName, workstationNameLen);

errOut:
	CFREE(targetName);
	CFREE(targetInfo);
	CFREE(domainNameFlat);
	CFREE(userNameFlat);
	CFREE(workstationName);
	if (domain) CFRelease(domain);
	if(ntlmResponsePtr != ntlmResp) {
		/* i.e., it was mallocd by ntlmGenerateNtlmV2Response */
		CFREE(ntlmResponsePtr);
	}
	if(ortn == noErr) {
		*clientResponse = clientBuf;
	}
	else {
		CFRelease(clientBuf);
	}
	return ortn;
}
	
/* replacement for NtlmNegotiatedNtlm2: returns NW_NTLM1Only, NW_NTLM2Only,
 * or NW_NTLMv2Only */
NLTM_Which NtlmGetNegotiatedVersion(
	NtlmGeneratorRef	ntlmGen)
{
	return ntlmGen->mNegotiatedVersion;
}

OSStatus _NtlmGeneratePasswordHashes(
	CFAllocatorRef alloc,
	NtlmGeneratorRef ntlm,
	CFStringRef password,
	CFDataRef* ntlmHash,
	CFDataRef* lmHash)
{
	OSStatus result = noErr;
	unsigned char hash[NTLM_DIGEST_LENGTH];
	
	ntlmPasswordHash(password, hash);
	
	*ntlmHash = CFDataCreate(alloc, hash, sizeof(hash));
	
	result = lmPasswordHash(ntlm->mCspHand, password, hash);
	
	if (result == noErr)
		*lmHash = CFDataCreate(alloc, hash, sizeof(hash));
	
	return result;
}

OSStatus NtlmGeneratePasswordHashes(
	CFAllocatorRef alloc,
	CFStringRef password,
	CFDataRef* ntlmHash,
	CFDataRef* lmHash)
{
	NtlmGeneratorRef ntlm = NULL;
	
	OSStatus result = NtlmGeneratorCreate(NW_Any, &ntlm);
	
	if (result == noErr) {
		result = _NtlmGeneratePasswordHashes(alloc, ntlm, password, ntlmHash, lmHash);
	}
	
	if (ntlm)
		NtlmGeneratorRelease(ntlm);
	
	return result;
}

