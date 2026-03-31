/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
 
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonHMAC.h>
#include <CoreFoundation/CoreFoundation.h>
#include <openssl/hmac.h>
#include "SMBAuth.h"
#include <stdio.h>
#include <sys/types.h>
#include "digestmd5.h"
#include <syslog.h>

#include "DSEndian.h"

/*!
 * @header SMBAuth
 */
typedef long KeysArray[32];

unsigned char apsasl_odd_parity[256] = {
        1,  1,  2,  2,  4,  4,  7,  7,  8,  8, 11, 11, 13, 13, 14, 14,
        16, 16, 19, 19, 21, 21, 22, 22, 25, 25, 26, 26, 28, 28, 31, 31,
        32, 32, 35, 35, 37, 37, 38, 38, 41, 41, 42, 42, 44, 44, 47, 47,
        49, 49, 50, 50, 52, 52, 55, 55, 56, 56, 59, 59, 61, 61, 62, 62,
        64, 64, 67, 67, 69, 69, 70, 70, 73, 73, 74, 74, 76, 76, 79, 79,
        81, 81, 82, 82, 84, 84, 87, 87, 88, 88, 91, 91, 93, 93, 94, 94,
        97, 97, 98, 98, 100, 100, 103, 103, 104, 104, 107, 107, 109, 109, 110, 110,
        112, 112, 115, 115, 117, 117, 118, 118, 121, 121, 122, 122, 124, 124, 127, 127,
        128, 128, 131, 131, 133, 133, 134, 134, 137, 137, 138, 138, 140, 140, 143, 143,
        145, 145, 146, 146, 148, 148, 151, 151, 152, 152, 155, 155, 157, 157, 158, 158,
        161, 161, 162, 162, 164, 164, 167, 167, 168, 168, 171, 171, 173, 173, 174, 174,
        176, 176, 179, 179, 181, 181, 182, 182, 185, 185, 186, 186, 188, 188, 191, 191,
        193, 193, 194, 194, 196, 196, 199, 199, 200, 200, 203, 203, 205, 205, 206, 206,
        208, 208, 211, 211, 213, 213, 214, 214, 217, 217, 218, 218, 220, 220, 223, 223,
        224, 224, 227, 227, 229, 229, 230, 230, 233, 233, 234, 234, 236, 236, 239, 239,
        241, 241, 242, 242, 244, 244, 247, 247, 248, 248, 251, 251, 253, 253, 254, 254};
typedef struct EncryptBlk
{
	UInt32 	keyHi;
	UInt32 	keyLo;

} EncryptBlk;
/*
 * Pads used in key derivation
 */
enum {
    NTLM_NONCE_LENGTH		= 8,
    NTLM_HASH_LENGTH		= 21,
    NTLM_RESP_LENGTH		= 24,
    NTLM_SESSKEY_LENGTH		= 16,
};

/* utility functions prototypes */
#ifdef __cplusplus
extern "C" {
#endif

u_int16_t ByteSwapInt16(u_int16_t value);
void CStringToUnicode(const char *cstr, int cstrLen, u_int16_t *unicode, size_t unicodeLen, size_t *outUnicodeByteCount);
void strnupper(char *str, int maxlen);

#ifdef __cplusplus
}
#endif

//extern void desKeySched(EncryptBlk *, long *, short);
#define desKeySched		KeySched

//extern void desEncode(long *, char *);
#define desEncode(A,B)		Encode((A),8,(B))

extern void desDecode (long *, char *);

#define kDESVersion1	1

/*
 * Pads used in key derivation
 */

 unsigned char SHSpad1[40] =
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

 unsigned char SHSpad2[40] =
   {0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
    0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
    0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
    0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2};

/*
 * "Magic" constants used in key derivations
 */
 
 unsigned char Magic1[27] =
   {0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74,
    0x68, 0x65, 0x20, 0x4d, 0x50, 0x50, 0x45, 0x20, 0x4d,
    0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x4b, 0x65, 0x79};

unsigned char Magic2[84] =
   {0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
    0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
    0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
    0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20, 0x6b, 0x65, 0x79,
    0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73,
    0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73, 0x69, 0x64, 0x65,
    0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
    0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
    0x6b, 0x65, 0x79, 0x2e};

unsigned char Magic3[84] =
   {0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
    0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
    0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
    0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
    0x6b, 0x65, 0x79, 0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68,
    0x65, 0x20, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73,
    0x69, 0x64, 0x65, 0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73,

    0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20,
    0x6b, 0x65, 0x79, 0x2e};
	
/* Utility functions */


void CalculateP24(unsigned char *P21, unsigned char *C8, unsigned char *P24)
{
	// setup P24
	memcpy(P24, C8, 8);
	memcpy(P24+8, C8, 8);
	memcpy(P24+16, C8, 8);

	DESEncode(P21, P24);
	DESEncode(P21+7, P24+8);
	DESEncode(P21+14, P24+16);
}

void CalculateSMBNTHash(const char *utf8Password, unsigned char outHash[16])
{
	size_t unicodeLen = 0;
	u_int16_t unicodepwd[258] = {0};
	size_t passLen = 0;
	
	if (utf8Password == NULL || outHash == NULL)
		return;
	
	passLen = strlen(utf8Password);
	if (passLen > 128)
		passLen = 128;
	
	CStringToUnicode(utf8Password, (int) passLen, unicodepwd, sizeof(unicodepwd), &unicodeLen);
	MD4Encode(outHash, (unsigned char *)unicodepwd, (int) unicodeLen);
	bzero(unicodepwd, unicodeLen);
}

void CalculateSMBLANManagerHash(const char *password, unsigned char outHash[16])
{
	unsigned char S8[8] = {0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25};
	int passLen = 0;
	unsigned char P21[21] = {0};
	unsigned char P14[14] = {0};
	unsigned char *P16 = P21;

	passLen = (int) strlen(password);
	if ( passLen > 14 )
		passLen = 14;

	// setup P14
	memmove(P14, password, (int) passLen);
	strnupper((char *)P14, 14);

	// setup P16
	memmove(P16, S8, 8);
	memmove(P16+8, S8, 8);

	DESEncode(P14, P16);
	DESEncode(P14+7, P16+8);

	memmove(outHash, P16, 16);
}


int32_t LittleEndianCharsToInt32( const char *inCharPtr )
{
	int32_t anInt;
	
	memcpy( &anInt, inCharPtr, 4 );
	anInt = EndianU32_NtoL( anInt );
	
	return anInt;
}

void CalculateWorkstationCredentialStrongSessKey( const unsigned char inNTHash[16], const char serverChallenge[8], const char clientChallenge[8], unsigned char outWCSK[16] )
{
	uint32_t zero = 0;
	CCHmacContext ctx;
	CC_MD5_CTX md5;
	unsigned char tmp[16];

	CCHmacInit(&ctx, kCCHmacAlgMD5, inNTHash, CC_MD5_DIGEST_LENGTH);

	CC_MD5_Init(&md5);
	CC_MD5_Update(&md5, &zero, 4);
	CC_MD5_Update(&md5, clientChallenge, 8);
	CC_MD5_Update(&md5, serverChallenge, 8);
	CC_MD5_Final(tmp, &md5);
	
	CCHmacUpdate(&ctx, tmp, 16);
	CCHmacFinal(&ctx, outWCSK);
	memset(tmp, 0, sizeof(tmp));
	memset(&ctx, 0, sizeof(ctx));
	return;
}

void CalculateWorkstationCredentialSessKey( const unsigned char inNTHash[16], const char serverChallenge[8], const char clientChallenge[8], unsigned char outWCSK[8] )
{
	int32_t schal, cchal;
	int32_t add1, add2;
	
	schal = LittleEndianCharsToInt32( serverChallenge );
	cchal = LittleEndianCharsToInt32( clientChallenge );
	add1 = schal + cchal;
	
	schal = LittleEndianCharsToInt32( serverChallenge + 4 );
	cchal = LittleEndianCharsToInt32( clientChallenge + 4 );
	add2 = schal + cchal;
	
	add1 = EndianU32_NtoL( add1 );
	add2 = EndianU32_NtoL( add2 );
		
	memcpy( outWCSK, &add1, 4 );
	memcpy( outWCSK + 4, &add2, 4 );
	
	DESEncode( inNTHash, outWCSK );
	DESEncode( inNTHash + 9, outWCSK );	
}


void CalculatePPTPSessionKeys( const unsigned char inNTHash[16], const unsigned char inNTResponse[24], int inSessionKeyLen, unsigned char *outSendKey, unsigned char *outReceiveKey )
{
	unsigned char hashHashBuff[CC_MD4_DIGEST_LENGTH + 1];
    unsigned char masterKey[16];
	
	MD4Encode( hashHashBuff, inNTHash, CC_MD4_DIGEST_LENGTH );
	GetMasterKey( hashHashBuff, inNTResponse, masterKey );
	GetAsymetricStartKey( masterKey, outSendKey, inSessionKeyLen, TRUE, TRUE );
	GetAsymetricStartKey( masterKey, outReceiveKey, inSessionKeyLen, FALSE, TRUE );
}


void GetMasterKey( const unsigned char inNTHashHash[16], const unsigned char inNTResponse[24], unsigned char outMasterKey[16] )
{
	unsigned char Digest[20];
	CC_SHA1_CTX Context;
	
	bzero( Digest, sizeof(Digest) );

	CC_SHA1_Init( &Context );
	CC_SHA1_Update( &Context, inNTHashHash, 16 );
	CC_SHA1_Update( &Context, inNTResponse, 24 );
	CC_SHA1_Update( &Context, Magic1, 27 );
	CC_SHA1_Final( Digest, &Context );

	memmove( outMasterKey, Digest, 16 );
}


void GetAsymetricStartKey( unsigned char inMasterKey[16], unsigned char *outSessionKey, int inSessionKeyLen, bool inIsSendKey, bool inIsServer )
{
	unsigned char Digest[20];
	unsigned char *magicPtr;
	CC_SHA1_CTX Context;
	
	if ( inSessionKeyLen > (int)sizeof(Digest) )
		return;
	
	bzero( Digest, sizeof(Digest) );
	
	if ( inIsSendKey )
	{
		if (inIsServer) {
			magicPtr = Magic3;
		}
		else {
			magicPtr = Magic2;
		}
	}
	else
	{
		if (inIsServer) {
			magicPtr = Magic2;
		} else {
			magicPtr = Magic3;
		}
	}
	
	CC_SHA1_Init( &Context );
	CC_SHA1_Update( &Context, inMasterKey, 16 );
	CC_SHA1_Update( &Context, SHSpad1, 40 );
	CC_SHA1_Update( &Context, magicPtr, 84 );
	CC_SHA1_Update( &Context, SHSpad2, 40 );
	CC_SHA1_Final( Digest, &Context );
	
	memmove( outSessionKey, Digest, inSessionKeyLen );
}

#pragma mark -
#pragma mark NTLM
#pragma mark -

/*
	Note: <V2> must be at least EVP_MAX_MD_SIZE
*/

int NTLMv2(unsigned char *V2, unsigned char *inNTHash,
		 const char *authid, const char *target,
		 const unsigned char *challenge,
		 const unsigned char *blob, unsigned bloblen)
{
    CCHmacContext ctx;
    char *upper;
    size_t len = 0;
	char *buf;
	unsigned int buflen;
	unsigned char hmac1[CC_MD4_DIGEST_LENGTH];
	
    /* Allocate enough space for the unicode target */
    len = strlen(authid);
	if (target)
		len += strlen(target);
	buflen = (unsigned int) (2 * len + 1);
	buf = (char *) malloc( buflen );
	if ( buf == NULL )
		return -1;
	
	/* NTLMv2hash = HMAC-MD5(NTLMhash, unicode(ucase(authid + domain))) */
	
	/* Use the tail end of the buffer for ucase() conversion */
	upper = buf + len;
	strcpy(upper, authid);
	if (target)
		strcat(upper, target);
	strnupper(upper, (int) len);
	CStringToUnicode(upper, (int) len, (u_int16_t *)buf, buflen, &len);
	
	CCHmac(
		kCCHmacAlgMD5,
		inNTHash, CC_MD4_DIGEST_LENGTH, 
		(unsigned char*)buf, len, 
		hmac1 );
	
	/* V2 = HMAC-MD5(NTLMv2hash, challenge + blob) + blob */
	CCHmacInit(&ctx, kCCHmacAlgMD5, hmac1, CC_MD5_DIGEST_LENGTH);
	CCHmacUpdate(&ctx, challenge, NTLM_NONCE_LENGTH);
	CCHmacUpdate(&ctx, blob, bloblen);
	CCHmacFinal(&ctx, V2);
	
	/* the blob is concatenated outside of this function */
	bzero(buf, len);
	free(buf);
    
	return 0;
}


void CalculateNTLMv2SessionKey(
	const unsigned char *inServerChallenge,
	const unsigned char *inClientChallenge,
	const unsigned char *inNTLMHash,
	unsigned char *outSessionKey )
{
	CC_MD5_CTX ctx;
    unsigned char md5ResultOnly8BytesUsed[CC_MD5_DIGEST_LENGTH];
	unsigned char paddedHash[21];
	
	CC_MD5_Init( &ctx );
	CC_MD5_Update( &ctx, inServerChallenge, 8 );
	CC_MD5_Update( &ctx, inClientChallenge, 8 );
	CC_MD5_Final( md5ResultOnly8BytesUsed, &ctx );
	
	memcpy( paddedHash, inNTLMHash, 16 );
	bzero(paddedHash + 16, 5);
	
	CalculateP24( paddedHash, md5ResultOnly8BytesUsed, outSessionKey );
}


#pragma mark -
#pragma mark Utilities
#pragma mark -

u_int16_t ByteSwapInt16(u_int16_t value)
{
	u_int16_t mask = value;
	mask <<= 8;
	value >>= 8;
	value |= mask;
	return value;
}


#if DEBUG
void
print_as_hex( void *bin, int len )
{
	int idx;
	char byteStr[10];
	char logStr[256] = {0};
	
	for ( idx = 0; idx < len; idx++ ) {
		sprintf(byteStr, "%.2x ", ((unsigned char *)bin)[idx]);
		strlcat(logStr, byteStr, sizeof(logStr));
	}
	strlcat(logStr, "\n\n", sizeof(logStr));
	syslog(LOG_ALERT, "xxx NT hex: %s", logStr);
}
#endif

void CStringToUnicode(const char *cstr, int cstrLen, u_int16_t *unicode, size_t unicodeLen, size_t *outUnicodeByteCount)
{
	CFStringRef convertString = CFStringCreateWithBytes( NULL, (const UInt8 *)cstr, (CFIndex)cstrLen, kCFStringEncodingUTF8, 0 );
	if ( convertString != NULL ) {
		 CFStringGetCString( convertString, (char *)unicode, (CFIndex) unicodeLen, kCFStringEncodingUTF16LE );
		*outUnicodeByteCount = CFStringGetLength( convertString ) * 2;		
		CFRelease( convertString );
	}
	
#if DEBUG
	print_as_hex( unicode, *outUnicodeByteCount );
#endif
}


void LittleEndianUnicodeToUnicode(const u_int16_t *unistr, int unistrLen, u_int16_t *unicode)
{
	int i;
	u_int16_t val;
 	 
	for(i = 0; i < unistrLen; i++)
	{
		val = *unistr;
		if (BYTE_ORDER == BIG_ENDIAN)
			*unicode = ByteSwapInt16(val);
		else
			*unicode = val;			
		unicode++;
		unistr++;
		if (val == 0) break;
	}
}


void MD4Encode(unsigned char *output, const unsigned char *input, unsigned int len)
{
	CC_MD4_CTX context = {};

	CC_MD4_Init(&context);
	CC_MD4_Update(&context, (unsigned char *)input, len);
	CC_MD4_Final(output, &context);
}

void strnupper(char *str, int maxlen)
{
	char *s = str;

	while (*s && maxlen)
	{
		if (islower(*s))
			*s = toupper(*s);
		s++;
		maxlen--;
	}
}

void DESEncode(const void *str, void *data)
{
	CCCryptorStatus status = kCCSuccess;
	unsigned char key[8] = {0};
	size_t dataMoved = 0;
	
	str_to_key((unsigned char *)str, key);
	status = CCCrypt( kCCEncrypt, kCCAlgorithmDES, 0,
						key, sizeof(key),
						NULL,
						data, 8,
						data, 8, &dataMoved );
	if ( status != kCCSuccess )
		bzero( data, 8 );
}


void apsasl_des_set_odd_parity(unsigned char *key)
{
	int idx;

	for (idx = 0; idx < 8; idx++)
		key[idx] = apsasl_odd_parity[key[idx]];
}

void str_to_key(unsigned char *str, unsigned char *key)
{
	int i;
	key[0] = str[0]>>1;
	key[1] = ((str[0]&0x01)<<6) | (str[1]>>2);
	key[2] = ((str[1]&0x03)<<5) | (str[2]>>3);
	key[3] = ((str[2]&0x07)<<4) | (str[3]>>4);
	key[4] = ((str[3]&0x0F)<<3) | (str[4]>>5);
	key[5] = ((str[4]&0x1F)<<2) | (str[5]>>6);
	key[6] = ((str[5]&0x3F)<<1) | (str[6]>>7);
	key[7] = str[6]&0x7F;
	for (i = 0; i < 8; i++) {
		key[i] = (key[i] << 1);
	}
	apsasl_des_set_odd_parity((unsigned char *)&key);
}

