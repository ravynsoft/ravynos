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
 * Private routines used by NtlmGenerator module. 
 */
#include "ntlmBlobPriv.h"
#include <CoreServices/CoreServices.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>
#include <strings.h>
#include <CommonCrypto/CommonDigest.h>
#include <Security/cssmapi.h>
#include <Security/cssmapple.h>
#include <CoreFoundation/CFDate.h>

#if		DEBUG_FIXED_CHALLENGE
/* Fixed 64-bit timestamp for sourceforge test vectors */
static unsigned char dbgStamp[] = 
{ 
	0x00, 0x90, 0xd3, 0x36, 0xb7, 0x34, 0xc3, 0x01 
};
#endif  /* DEBUG_FIXED_CHALLENGE */

#pragma mark --- encode/decode routines ---

/* uint32_t <--> unsigned char array */
void serializeUint32(
	uint32_t		num,
	unsigned char   *buf)
{
	buf[0] = num & 0xff;
	buf[1] = num >> 8;
	buf[2] = num >> 16;
	buf[3] = num >> 24;
}

uint32_t deserializeUint32(
	const unsigned char *buf)
{
	uint32_t rtn = *buf++;
	rtn |= ((uint32_t)(*buf++)) << 8;
	rtn |= ((uint32_t)(*buf++)) << 16;
	rtn |= ((uint32_t)(*buf)) << 24;
	return rtn;
}

uint16_t deserializeUint16(
	const unsigned char *buf)
{
	uint16_t rtn = *buf++;
	rtn |= ((uint16_t)(*buf)) << 8;
	return rtn;
}

/* write a 32-bit word, little endian */
void appendUint32(
	CFMutableDataRef	buf,
	uint32_t			word)
{
	unsigned char cb[4];
	serializeUint32(word, cb);
	CFDataAppendBytes(buf, cb, 4);
}

/* write a 16-bit word, little endian */
void appendUint16(
	CFMutableDataRef	buf,
	uint16_t			word)
{
	unsigned char cb[2];
	cb[0] = word & 0xff;
	cb[1] = word >> 8;
	CFDataAppendBytes(buf, cb, 2);
}

/* 
 * Write a security buffer, providing the index into the CFData at which 
 * this security buffer's offset is located. Just before the actual data is written,
 * go back and update the offset with the start of that data using secBufOffset().
 */
void appendSecBuf(
	CFMutableDataRef	buf,
	uint16_t			len,
	CFIndex				*offsetIndex)
{
	appendUint16(buf, len);					/* buffer length */
	appendUint16(buf, len);					/* buffer allocated size */
	*offsetIndex = CFDataGetLength(buf);	/* offset will go here */
	appendUint32(buf, 0);					/* but it's empty for now */
}

/*
 * Update a security buffer's offset to be the current end of data in a CFData.
 */
void secBufOffset(
	CFMutableDataRef	buf,
	CFIndex				offsetIndex)		/* obtained from appendSecBuf() */
{
	CFIndex currPos = CFDataGetLength(buf);
	unsigned char cb[4];
	serializeUint32((uint32_t)currPos, cb);
	CFRange range = {offsetIndex, 4};
	CFDataReplaceBytes(buf, range, cb, 4);
}

/*
 * Parse/validate a security buffer. Verifies that supplied offset/length don't go
 * past end of avaialble data. Returns ptr to actual data and its length. Returns
 * NTLM_ERR_PARSE_ERR on bogus values.
 */
OSStatus ntlmParseSecBuffer(
	const unsigned char *cp,			/* start of security buffer */
	const unsigned char *bufStart,		/* start of whole msg buffer */
	unsigned bufLen,					/* # of valid bytes starting at bufStart */
	const unsigned char **data,			/* RETURNED, start of actual data */
	uint16_t *dataLen)					/* RETURNED, length of actual data */
{
	assert(cp >= bufStart);

	uint16_t secBufLen = deserializeUint16(cp);
	/* skip length we just parsed plus alloc size, which we don't use */
	cp += 4;
	uint32_t offset = deserializeUint32(cp);
	if((offset + secBufLen) > bufLen) {
		dprintf("ntlmParseSecBuffer: buf overflow\n");
		return NTLM_ERR_PARSE_ERR;
	}
	*data = bufStart + offset;
	*dataLen = secBufLen;
	return noErr;
}

#pragma mark --- CFString converters ---

/*
 * Convert CFString to little-endian unicode. 
 */
void ntlmStringToLE(
	CFStringRef		pwd,
	unsigned char   **ucode,		// mallocd and RETURNED
	unsigned		*ucodeLen)		// RETURNED
{
	CFIndex len = CFStringGetLength(pwd);
	unsigned char *data = (unsigned char *)malloc(len * 2);
	unsigned char *cp = data;
	
	for(CFIndex dex=0; dex<len; dex++) {
		UniChar uc = CFStringGetCharacterAtIndex(pwd, dex);
		*cp++ = uc & 0xff;
		*cp++ = uc >> 8;
	}
	*ucode = data;
	*ucodeLen = len * 2;
}

/*
 * Convert a CFStringRef into a mallocd array of chars suitable for the specified
 * encoding. This might return an error if the string can't be converted 
 * appropriately. 
 */
OSStatus ntlmStringFlatten(
	CFStringRef str,
	bool unicode,
	unsigned char **flat,			// mallocd and RETURNED
	unsigned *flatLen)				// RETURNED
{
	if(unicode) {
		/* convert to little-endian unicode */
		ntlmStringToLE(str, flat, flatLen);
		return noErr;
	}
	else {
		/* convert to ASCII C string */
		CFIndex strLen = CFStringGetLength(str);
		char *cStr = (char *)malloc(strLen + 1);
		if(cStr == NULL) {
			return memFullErr;
		}
		if(CFStringGetCString(str, cStr, strLen + 1, kCFStringEncodingASCII)) {
			*flat = (unsigned char *)cStr;
			*flatLen = strLen;
			return noErr;
		}
		
		/*
		 * Well that didn't work. Try UTF8 - I don't know how a MS would behave if
		 * this portion of auth (only used for the LM response) didn't work.
		 */
		dprintf("lmPasswordHash: ASCII password conversion failed; trying UTF8\n");
		free(cStr);
		cStr = (char *)malloc(strLen * 4);
		if(cStr == NULL) {
			return memFullErr;
		}
		if(CFStringCreateExternalRepresentation(NULL, str, kCFStringEncodingUTF8, 0)) {
			*flat = (unsigned char *)cStr;
			*flatLen = strLen;
			return noErr;
		}
		dprintf("lmPasswordHash: UTF8 password conversion failed\n");
		free(cStr);
		return NTLM_ERR_PARSE_ERR;
	}
}

#pragma mark --- machine dependent cruft ---

/* random number generator */
void ntlmRand(
	unsigned		len,
	void			*buf)				/* allocated by caller, random data RETURNED */
{
	int fd = open("/dev/random", O_RDONLY, 0);
	if(fd < 0) {
		dprintf("***ntlmRand failed to open /dev/random\n");
		return;
	}
	read(fd, buf, len);
	close(fd);
}

/* Obtain host name in appropriate encoding */
OSStatus ntlmHostName(
	bool unicode,
	unsigned char **flat,			// mallocd and RETURNED
	unsigned *flatLen)				// RETURNED
{
	char hostname[MAXHOSTNAMELEN];
	if(gethostname(hostname, MAXHOSTNAMELEN)) {
		#ifndef NDEBUG
		perror("gethostname");
		#endif
		return internalComponentErr;
	}
	int len = strlen(hostname);
	if(unicode) {
		/* quickie "little endian unicode" conversion */
		*flat = (unsigned char *)malloc(len * 2);
		unsigned char *cp = *flat;
		for(int dex=0; dex<len; dex++) {
			*cp++ = hostname[dex];
			*cp++ = 0;
		}
		*flatLen = len * 2;
		return noErr;
	}
	else {
		*flat = (unsigned char *)malloc(len);
		*flatLen = len;
		memmove(*flat, hostname, len);
		return noErr;
	}
}
	
/* 
 * Append 64-bit little-endiam timestamp to a CFData. Time is relative to 
 * January 1 1601, in tenths of a microsecond. 
 */
static const CFGregorianDate ntlmTimeBasis = 
{
	1601,   // year
	1,		// month
	1,		// day
	0,		// hour
	0,		// minute
	0.0		// second
};

void ntlmAppendTimestamp(
	CFMutableDataRef ntlmV2Blob)
{
	#if DEBUG_FIXED_CHALLENGE
	/* Fixed 64-bit timestamp for sourceforge test vectors */
	CFDataAppendBytes(ntlmV2Blob, dbgStamp, 8);
	#else

	assert(CFGregorianDateIsValid(ntlmTimeBasis, kCFGregorianAllUnits));
										
	/* NULL OK for CFTimeZoneRef? */
	CFAbsoluteTime basisTime = CFGregorianDateGetAbsoluteTime(ntlmTimeBasis, NULL);
	CFAbsoluteTime nowTime   = CFAbsoluteTimeGetCurrent();
	
	/* elapsed := time in seconds since basis */
	CFTimeInterval elapsed = nowTime - basisTime;
	/* now in tenths of microseconds */
	elapsed *= 10000000.0;
	
	uint32 lowWord = (uint32)elapsed;
	elapsed /=  0x100000000ULL;
	uint32 highWord = (uint32)elapsed;
	
	/* append this in little endian format */
	appendUint32(ntlmV2Blob, lowWord);
	appendUint32(ntlmV2Blob, highWord);
	#endif
}

#pragma mark --- crypto ---

/* MD4 and MD5 hash */
#define NTLM_DIGEST_LENGTH   16
void md4Hash(
	const unsigned char *data,
	unsigned			dataLen,
	unsigned char		*digest)		// caller-supplied, NTLM_DIGEST_LENGTH */
{
	CC_MD4_CTX ctx;
	CC_MD4_Init(&ctx);
	CC_MD4_Update(&ctx, data, dataLen);
	CC_MD4_Final(digest, &ctx);
}

void md5Hash(
	const unsigned char *data,
	unsigned			dataLen,
	unsigned char		*digest)		// caller-supplied, NTLM_DIGEST_LENGTH */
{
	CC_MD5_CTX ctx;
	CC_MD5_Init(&ctx);
	CC_MD5_Update(&ctx, data, dataLen);
	CC_MD5_Final(digest, &ctx);
}

/*
 * Given 7 bytes, create 8-byte DES key. Our implementation ignores the 
 * parity bit (lsb), which simplifies this somewhat. 
 */
void ntlmMakeDesKey(
	const unsigned char *inKey,			// 7 bytes
	unsigned char *outKey)				// 8 bytes
{
	outKey[0] =   inKey[0] & 0xfe;
	outKey[1] = ((inKey[0] << 7) | (inKey[1] >> 1)) & 0xfe;
	outKey[2] = ((inKey[1] << 6) | (inKey[2] >> 2)) & 0xfe;
	outKey[3] = ((inKey[2] << 5) | (inKey[3] >> 3)) & 0xfe;
	outKey[4] = ((inKey[3] << 4) | (inKey[4] >> 4)) & 0xfe;
	outKey[5] = ((inKey[4] << 3) | (inKey[5] >> 5)) & 0xfe;
	outKey[6] = ((inKey[5] << 2) | (inKey[6] >> 6)) & 0xfe;
	outKey[7] =  (inKey[6] << 1) & 0xfe;
}

static void ntlmSetupKey(
	CSSM_ALGORITHMS		alg,
	const unsigned char *keyData,
	unsigned			keyDataLen,		/* in bytes */
	unsigned			logicalKeySizeInBits,
	CSSM_KEY_PTR		ckey)
{
	memset(ckey, 0, sizeof(*ckey));
	CSSM_KEYHEADER &hdr = ckey->KeyHeader;
	hdr.BlobType = CSSM_KEYBLOB_RAW;
	hdr.Format = CSSM_KEYBLOB_RAW_FORMAT_OCTET_STRING;
	hdr.AlgorithmId = alg;
	hdr.KeyClass = CSSM_KEYCLASS_SESSION_KEY;
	hdr.LogicalKeySizeInBits = logicalKeySizeInBits;
	hdr.KeyUsage = CSSM_KEYUSE_ANY;
	ckey->KeyData.Data = (uint8 *)keyData;
	ckey->KeyData.Length = keyDataLen;
}

/*
 * single block DES encrypt.
 * This would really benefit from a DES implementation in CommonCrypto. 
 */
OSStatus ntlmDesCrypt(
	CSSM_CSP_HANDLE		cspHand,
	const unsigned char *key,		// 8 bytes
	const unsigned char *inData,	// 8 bytes
	const unsigned char *outData)   // 8 bytes
{
	CSSM_CC_HANDLE ccHand;
	CSSM_RETURN crtn;
	CSSM_KEY ckey;
	
	ntlmSetupKey(CSSM_ALGID_DES, key, DES_KEY_SIZE, 
		DES_RAW_KEY_SIZE * 8, &ckey);
	
	crtn = CSSM_CSP_CreateSymmetricContext(cspHand,
		CSSM_ALGID_DES,
		CSSM_ALGMODE_ECB,
		NULL,			// access cred
		&ckey,
		NULL,			// InitVector
		CSSM_PADDING_NONE,
		NULL,			// Params
		&ccHand);
	if(crtn) {
		#ifndef NDEBUG
		cssmPerror("CSSM_CSP_CreateSymmetricContext", crtn);
		#endif
		return crtn;
	}
	
	CSSM_DATA ptext = {8, (uint8 *)inData};
	CSSM_DATA ctext = {9, (uint8 *)outData};
	uint32 bytesEncrypted;
	
	crtn = CSSM_EncryptDataInit(ccHand);
	if(crtn) {
		#ifndef NDEBUG
		cssmPerror("CSSM_EncryptDataInit", crtn);
		#endif
		goto errOut;
	}
	crtn = CSSM_EncryptDataUpdate(ccHand,
		&ptext, 1,
		&ctext, 1,
		&bytesEncrypted);
	if(crtn) {
		#ifndef NDEBUG
		cssmPerror("CSSM_EncryptDataUpdate", crtn);
		#endif
	}
errOut:
	CSSM_DeleteContext(ccHand);
	return crtn;
}

/*
 * HMAC/MD5.
 */
OSStatus ntlmHmacMD5(
	CSSM_CSP_HANDLE		cspHand,
	const unsigned char *key,	
	unsigned			keyLen,
	const unsigned char *inData,
	unsigned			inDataLen,
	unsigned char		*mac)		// caller provided, NTLM_DIGEST_LENGTH
{
	CSSM_CC_HANDLE ccHand;
	CSSM_RETURN crtn;
	CSSM_KEY ckey;
	CSSM_DATA cdata = { inDataLen, (uint8 *)inData };
	
	ntlmSetupKey(CSSM_ALGID_MD5HMAC, key, keyLen, 
		keyLen * 8, &ckey);
	crtn = CSSM_CSP_CreateMacContext(cspHand,
		CSSM_ALGID_MD5HMAC,	&ckey, &ccHand);
	if(crtn) {
		#ifndef NDEBUG
		cssmPerror("CSSM_CSP_CreateMacContext", crtn);
		#endif
		return crtn;
	}
	crtn = CSSM_GenerateMacInit(ccHand);
	if(crtn) {
		#ifndef NDEBUG
		cssmPerror("CSSM_GenerateMacInit", crtn);
		#endif
		goto errOut;
	}
	crtn = CSSM_GenerateMacUpdate(ccHand, &cdata, 1);
	if(crtn) {
		#ifndef NDEBUG
		cssmPerror("CSSM_GenerateMacUpdate", crtn);
		#endif
		goto errOut;
	}
	
	/* provide pre-allocated output buffer */
	cdata.Data = (uint8 *)mac;
	cdata.Length = NTLM_DIGEST_LENGTH;
 	crtn = CSSM_GenerateMacFinal(ccHand, &cdata);
	if(crtn) {
		#ifndef NDEBUG
		cssmPerror("CSSM_GenerateMacFinal", crtn);
		#endif
	}
errOut:
	CSSM_DeleteContext(ccHand);
	return crtn;
}

#pragma mark --- LM and NTLM password and digest munging ---

/*
 * Calculate LM-style password hash. This really only works if the password 
 * is convertible to ASCII (that is, it will indeed return an error if that
 * is not true). 
 *
 * This is the most gawdawful constant I've ever seen in security-related code.
 */
static const unsigned char lmHashPlaintext[] = {'K', 'G', 'S', '!', '@', '#', '$', '%'};

OSStatus lmPasswordHash(	
	CSSM_CSP_HANDLE cspHand,
	CFStringRef		pwd,
	unsigned char   *digest)		// caller-supplied, NTLM_DIGEST_LENGTH
{
	/* convert to ASCII */
	unsigned strLen;
	unsigned char *cStr;
	OSStatus ortn;
	ortn = ntlmStringFlatten(pwd, false, &cStr, &strLen);
	if(ortn) {
		dprintf("lmPasswordHash: ASCII password conversion failed\n");
		return ortn;
	}
	
	/* truncate/pad to 14 bytes and convert to upper case */
	unsigned char pwdFix[NTLM_LM_PASSWORD_LEN];
	unsigned toMove = NTLM_LM_PASSWORD_LEN;
	if(strLen < NTLM_LM_PASSWORD_LEN) {
		toMove = strLen;
	}
	memmove(pwdFix, cStr, toMove);
	free(cStr);
	for(unsigned dex=0; dex<NTLM_LM_PASSWORD_LEN; dex++) {
		pwdFix[dex] = toupper(pwdFix[dex]);
	}
	
	/* two DES keys - raw material 7 bytes, munge to 8 bytes */
	unsigned char desKey1[DES_KEY_SIZE], desKey2[DES_KEY_SIZE];
	ntlmMakeDesKey(pwdFix, desKey1);
	ntlmMakeDesKey(pwdFix + DES_RAW_KEY_SIZE, desKey2);
	
	/* use each of those keys to encrypt the magic string */
	ortn = ntlmDesCrypt(cspHand, desKey1, lmHashPlaintext, digest);
	if(ortn == noErr) {
		ortn = ntlmDesCrypt(cspHand, desKey2, lmHashPlaintext, digest + DES_BLOCK_SIZE);
	}
	return ortn;
}
	
/*
 * Calculate NTLM password hash (MD4 on a unicode password).
 */
void ntlmPasswordHash(
	CFStringRef		pwd,
	unsigned char   *digest)		// caller-supplied, NTLM_DIGEST_LENGTH
{
	unsigned char *data;
	unsigned len;

	/* convert to little-endian unicode */
	ntlmStringToLE(pwd, &data, &len);
	/* md4 hash of that */
	md4Hash(data, len, digest);
	free(data);
}

/* 
 * NTLM response: DES encrypt the challenge (or session hash) with three 
 * different keys derived from the password hash. Result is concatenation 
 * of three DES encrypts. 
 */
#define ALL_KEYS_LENGTH (3 * DES_RAW_KEY_SIZE)
OSStatus ntlmResponse(
	CSSM_CSP_HANDLE		cspHand,
	const unsigned char *digest,		// NTLM_DIGEST_LENGTH bytes
	const unsigned char *ptext,			// challenge or session hash 
	unsigned char		*ntlmResp)		// caller-supplied NTLM_LM_RESPONSE_LEN
{
	unsigned char allKeys[ALL_KEYS_LENGTH];
	unsigned char key1[DES_KEY_SIZE], key2[DES_KEY_SIZE], key3[DES_KEY_SIZE];
	OSStatus ortn;
	
	memmove(allKeys, digest, NTLM_DIGEST_LENGTH);
	memset(allKeys + NTLM_DIGEST_LENGTH, 0, ALL_KEYS_LENGTH - NTLM_DIGEST_LENGTH);
	ntlmMakeDesKey(allKeys, key1);
	ntlmMakeDesKey(allKeys + DES_RAW_KEY_SIZE, key2);
	ntlmMakeDesKey(allKeys + (2 * DES_RAW_KEY_SIZE), key3);
	ortn = ntlmDesCrypt(cspHand, key1, ptext, ntlmResp);
	if(ortn == noErr) {
		ortn = ntlmDesCrypt(cspHand, key2, ptext, ntlmResp + DES_BLOCK_SIZE);
	}
	if(ortn == noErr) {
		ortn = ntlmDesCrypt(cspHand, key3, ptext, ntlmResp + (2 * DES_BLOCK_SIZE));
	}
	return ortn;
}

