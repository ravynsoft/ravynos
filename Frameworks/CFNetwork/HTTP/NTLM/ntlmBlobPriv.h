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

#ifndef _NTLM_BLOB_PRIV_H_
#define _NTLM_BLOB_PRIV_H_

#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFString.h>
#include <stdint.h>
#include <Security/cssmtype.h>
#include <Security/SecBase.h>
#include <CoreServices/CoreServices.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef NDEBUG

#include <stdio.h>

#define dprintf(args...) printf(args)
#else
#define dprintf(args...)
#endif
 
/*
 * Common error returns.
 *
 * This one for "I don't understand the server blob".
 */
#define NTLM_ERR_PARSE_ERR			paramErr

/*
 * This one for protocol variant mismatch (e.g., app requires NTLMv2 but server
 * doesn't accept that).
 */
#define NTLM_ERR_PROTOCOL_MISMATCH  errSecAuthFailed

/* 
 * For debugging using fixed pamaters via sourceforge "test vectors".
 */
#define DEBUG_FIXED_CHALLENGE   0

/* handy portable NULL-tolerant free() */
#define CFREE(p)		if(p != NULL) { free(p); }

#define NTLM_SIGNATURE				"NTLMSSP"
#define NTLM_SIGNATURE_LEN			8			/* including NULL! */

#define NTLM_MSG_MARKER_TYPE1		1			/* first client msg */
#define NTLM_MSG_MARKER_TYPE2		2			/* server challenge */
#define NTLM_MSG_MARKER_TYPE3		3			/* client response */

/* Size of a security buffer */
#define NTLM_SIZEOF_SEC_BUF		(sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t))

/* length of server challenge in bytes */
#define NTLM_CHALLENGE_LEN			8

/* length of client nonce in bytes */
#define NTLM_CLIENT_NONCE_LEN		8

/* length of LM and NTLM responses */
#define NTLM_LM_RESPONSE_LEN		24

/* foreced length of LM-style uppper case password */
#define NTLM_LM_PASSWORD_LEN		14

/* 
 * Flags - defined here in native endianness; sent over the wire little-endian 
 */
#define NTLM_NegotiateUnicode		0x00000001
#define NTLM_NegotiateOEM			0x00000002
#define NTLM_RequestTarget			0x00000004
#define NTLM_Unknown1				0x00000008
#define NTLM_NegotiateSign			0x00000010
#define NTLM_NegotiateSeal			0x00000020
#define NTLM_NegotiateDatagram		0x00000040
#define NTLM_NegotiateLMKey			0x00000080
#define NTLM_NegotiateNetware		0x00000100
#define NTLM_NegotiateNTLM			0x00000200
#define NTLM_Unknown2				0x00000400
#define NTLM_Unknown3				0x00000800
#define NTLM_DomainSupplied			0x00001000
#define NTLM_WorkstationSupplies	0x00002000
#define NTLM_LocalCall				0x00004000
#define NTLM_AlwaysSign				0x00008000
#define NTLM_TargetTypeDomain		0x00010000
#define NTLM_TargetTypeServer		0x00020000
#define NTLM_TargetTypeShare		0x00040000
#define NTLM_NegotiateNTLM2Key		0x00080000
#define NTLM_RequestInitResp		0x00100000
#define NTLM_RequestAcceptResp		0x00200000
#define NTLM_RequestNonNTSessionKey 0x00400000
#define NTLM_NegotiateTargetInfo	0x00800000
#define NTLM_Unknown4				0x01000000
#define NTLM_Unknown5				0x02000000
#define NTLM_Unknown6				0x04000000
#define NTLM_Unknown7				0x08000000
#define NTLM_Unknown8				0x10000000
#define NTLM_Negotiate128Bit		0x20000000
#define NTLM_NegotiateKeyExchange   0x40000000
#define NTLM_Negotiate56Bit			0x80000000

/* uint32_t <--> unsigned char array */
void serializeUint32(
	uint32_t			num,
	unsigned char		*buf);
	
uint32_t deserializeUint32(
	const unsigned char *buf);
	
uint16_t deserializeUint16(
	const unsigned char *buf);

/* write a 32-bit word, little endian */
void appendUint32(
	CFMutableDataRef	buf,
	uint32_t			word);
	
/* write a 16-bit word, little endian */
void appendUint16(
	CFMutableDataRef	buf,
	uint16_t			word);

/* 
 * Write a security buffer, providing the index into the CFData at which 
 * this security buffer's offset is located. Just before the actual data is written,
 * go back and update the offset with the start of that data using secBufOffset().
 */
void appendSecBuf(
	CFMutableDataRef	buf,
	uint16_t			len,
	CFIndex				*offsetIndex);

/*
 * Update a security buffer's offset to be the current end of data in a CFData.
 */
void secBufOffset(
	CFMutableDataRef	buf,
	CFIndex				offsetIndex);		/* obtained from appendSecBuf() */

/*
 * Parse/validate a security buffer. Verifies that supplied offset/length don't go
 * past end of avaialble data. Returns ptr to actual data and its length. Returns
 * paramErr on bogus values.
 */
OSStatus ntlmParseSecBuffer(
	const unsigned char *cp,			/* start of security buffer */
	const unsigned char *bufStart,		/* start of whole msg buffer */
	unsigned bufLen,					/* # of valid bytes starting at bufStart */
	const unsigned char **data,			/* RETURNED, start of actual data */
	uint16_t *dataLen);					/* RETURNED, length of actual data */

/* random number generator */
void ntlmRand(
	unsigned		len,
	void			*buf);				/* allocated by caller, random data RETURNED */
	
/* Obtain host name in appropriate encoding */
OSStatus ntlmHostName(
	bool			unicode,
	unsigned char   **flat,				// mallocd and RETURNED
	unsigned		*flatLen);			// RETURNED

void ntlmAppendTimestamp(
	CFMutableDataRef ntlmV2Blob);

/*
 * Convert CFString to little-endian unicode. 
 */
void ntlmStringToLE(
	CFStringRef		pwd,
	unsigned char   **ucode,		// mallocd and RETURNED
	unsigned		*ucodeLen);		// RETURNED

/*
 * Convert a CFStringRef into a mallocd array of chars suitable for the specified
 * encoding. This might return an error if the string can't be converted 
 * appropriately. 
 */
OSStatus ntlmStringFlatten(
	CFStringRef str,
	bool unicode,
	unsigned char **flat,				// mallocd and RETURNED
	unsigned *flatLen);					// RETURNED

/* MD4 and MD5 hash */
#define NTLM_DIGEST_LENGTH   16
void md4Hash(
	const unsigned char *data,
	unsigned			dataLen,
	unsigned char		*digest);		// caller-supplied, NTLM_DIGEST_LENGTH */
void md5Hash(
	const unsigned char *data,
	unsigned			dataLen,
	unsigned char		*digest);		// caller-supplied, NTLM_DIGEST_LENGTH */
	
/*
 * Calculate LM-style password hash. This really only works if the password 
 * is convertible to ASCII.
 */
OSStatus lmPasswordHash(	
	CSSM_CSP_HANDLE		cspHand,
	CFStringRef			pwd,
	unsigned char		*digest);		// caller-supplied, NTLM_DIGEST_LENGTH

/*
 * Calculate NTLM password hash (MD4 on a unicode password).
 */
void ntlmPasswordHash(
	CFStringRef			pwd,
	unsigned char		*digest);		// caller-supplied, NTLM_DIGEST_LENGTH
	
/* 
 * NTLM response: DES with three different keys.
 */
OSStatus ntlmResponse(
	CSSM_CSP_HANDLE		cspHand,
	const unsigned char *digest,		// NTLM_DIGEST_LENGTH bytes
	const unsigned char *challenge,		// actually challenge or session hash 
	unsigned char		*ntlmResp);		// caller-supplied NTLM_LM_RESPONSE_LEN
	
/* DES-related consts */
#define DES_BLOCK_SIZE		8
#define DES_RAW_KEY_SIZE	7
#define DES_KEY_SIZE		8

/*
 * Given 7 bytes, create 8-byte DES key. Our implementation ignores the 
 * parity bit (lsb), which simplifies this somewhat. 
 */
void ntlmMakeDesKey(
	const unsigned char *inKey,			// DES_RAW_KEY_SIZE bytes
	unsigned char *outKey);				// DES_KEY_SIZE bytes

/*
 * single block DES encrypt.
 * This would really benefit from a DES implementation in CommonCrypto. 
 */
OSStatus ntlmDesCrypt(
	CSSM_CSP_HANDLE		cspHand,
	const unsigned char *key,			// DES_KEY_SIZE bytes
	const unsigned char *inData,		// DES_BLOCK_SIZE bytes
	const unsigned char *outData);		// DES_BLOCK_SIZE bytes

/*
 * HMAC/MD5.
 */
OSStatus ntlmHmacMD5(
	CSSM_CSP_HANDLE		cspHand,
	const unsigned char *key,	
	unsigned			keyLen,
	const unsigned char *inData,
	unsigned			inDataLen,
	unsigned char		*mac);			// caller provided, NTLM_DIGEST_LENGTH

#if NTLM_DUMP
void ntlmPrintFlags(
	const char *whereFrom,
	uint32_t flags);
#else
#define ntlmPrintFlags(w, f)
#endif

#ifdef  __cplusplus
}
#endif

#endif  /* _NTLM_BLOB_PRIV_H_ */
