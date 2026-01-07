/*
 * Copyright (c) 2012 Apple Inc. All Rights Reserved.
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

// #define COMMON_HMAC_FUNCTIONS
#include <stdlib.h>
#include <CommonCrypto/CommonHMAC.h>
#include <CommonCrypto/CommonHMacSPI.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>
#include "CommonDigestPriv.h"
#include <corecrypto/cchmac.h>
#include <corecrypto/cc_priv.h>
#include "ccdebug.h"

#ifndef	NDEBUG
#define ASSERT(s)
#else
#define ASSERT(s)	assert(s)
#endif

#define	HMAC_MAX_BLOCK_SIZE     CC_SHA512_BLOCK_BYTES
#define	HMAC_MAX_DIGEST_SIZE    CC_SHA512_DIGEST_LENGTH

/* 
 * This is what a CCHmacContext actually points to.
 * we have 384 bytes to work with
 */


typedef struct {
    const struct ccdigest_info *di;
#if defined(_WIN32) //rdar://problem/27873676
    struct cchmac_ctx ctx[cc_ctx_n(struct cchmac_ctx, cchmac_ctx_size(HMAC_MAX_DIGEST_SIZE, HMAC_MAX_BLOCK_SIZE))];
#else
    cchmac_ctx_decl(HMAC_MAX_DIGEST_SIZE, HMAC_MAX_BLOCK_SIZE, ctx); 
#endif
} _NewHmacContext;


typedef struct {
    CCHmacAlgorithm ccHmacValue;
    CCDigestAlgorithm ccDigestAlg;
    const char *ccDigestName;
} ccHmac2DigestConversion;


const ccHmac2DigestConversion ccconversionTable[] = {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    { kCCHmacAlgSHA1, kCCDigestSHA1, "sha1" },
    { kCCHmacAlgMD5, kCCDigestMD5, "md5" },
#pragma clang diagnostic pop
    { kCCHmacAlgSHA224, kCCDigestSHA224, "sha224" },
    { kCCHmacAlgSHA256, kCCDigestSHA256, "sha256" },
    { kCCHmacAlgSHA384, kCCDigestSHA384, "sha384" },
    { kCCHmacAlgSHA512, kCCDigestSHA512, "sha512" },
};

const static int ccHmacConversionTableLength = sizeof(ccconversionTable) / sizeof(ccHmac2DigestConversion);

static const struct ccdigest_info *
convertccHmacSelector(CCHmacAlgorithm oldSelector)
{
    int i;
    
    for(i=0; i<ccHmacConversionTableLength; i++) 
        if(oldSelector == ccconversionTable[i].ccHmacValue) {
            return CCDigestGetDigestInfo(ccconversionTable[i].ccDigestAlg);
        }
    return NULL;
}


void CCHmacInit(
                CCHmacContext *ctx, 
                CCHmacAlgorithm algorithm,	/* kCCHmacAlgSHA1, kCCHmacAlgMD5 */
                const void *key,
                size_t keyLength)		/* length of key in bytes */
{
	_NewHmacContext		*hmacCtx = (_NewHmacContext *)ctx;
    // CCDigestCtxPtr  digestCtx = &hmacCtx->digestCtx;
    
    CC_DEBUG_LOG("Entering Algorithm: %d\n", algorithm);


	ASSERT(sizeof(_NewHmacContext) < sizeof(CCHmacContext));    
	
	if(hmacCtx == NULL) {
        CC_DEBUG_LOG( "NULL Context passed in\n");
        return;
    }

	cc_clear(sizeof(_NewHmacContext), hmacCtx);

    if((hmacCtx->di = convertccHmacSelector(algorithm)) == NULL) {
        CC_DEBUG_LOG( "CCHMac Unknown Digest %d\n", algorithm);
        return;
	}
    
    cchmac_init(hmacCtx->di, hmacCtx->ctx, keyLength, key);
    
    
}

void CCHmacUpdate(
                  CCHmacContext *ctx, 
                  const void *dataIn,
                  size_t dataInLength)	/* length of data in bytes */
{
	_NewHmacContext	*hmacCtx = (_NewHmacContext *)ctx;

    CC_DEBUG_LOG("Entering\n");
    cchmac_update(hmacCtx->di, hmacCtx->ctx, dataInLength, dataIn);
}

void CCHmacFinal(
                 CCHmacContext *ctx, 
                 void *macOut)
{
	_NewHmacContext	*hmacCtx = (_NewHmacContext *)ctx;
    
    CC_DEBUG_LOG("Entering\n");
    cchmac_final(hmacCtx->di, hmacCtx->ctx, macOut);
}

void
CCHmacDestroy(CCHmacContextRef ctx)
{
	cc_clear(sizeof(_NewHmacContext), ctx);
    free(ctx);
}


size_t
CCHmacOutputSizeFromRef(CCHmacContextRef ctx)
{
	_NewHmacContext		*hmacCtx = (_NewHmacContext *)ctx;
    CC_DEBUG_LOG("Entering\n");
	return hmacCtx->di->output_size;
}


size_t
CCHmacOutputSize(CCDigestAlg alg)
{
    CC_DEBUG_LOG("Entering\n");
	return CCDigestGetOutputSize(alg);
}


/*
 * Stateless, one-shot HMAC function. 
 * Output is written to caller-supplied buffer, as in CCHmacFinal().
 */
void CCHmac(
            CCHmacAlgorithm algorithm,	/* kCCHmacAlgSHA1, kCCHmacAlgMD5 */
            const void *key,
            size_t keyLength,		/* length of key in bytes */
            const void *data,
            size_t dataLength,		/* length of data in bytes */
            void *macOut)			/* MAC written here */
{
    CC_DEBUG_LOG("Entering Algorithm: %d\n", algorithm);
    cchmac(convertccHmacSelector(algorithm), keyLength, key, dataLength, data, macOut);
}



CCHmacContextRef
CCHmacCreate(CCDigestAlg alg, const void *key, size_t keyLength)
{
	_NewHmacContext		*hmacCtx;
    
    CC_DEBUG_LOG("Entering\n");
	/* if this fails, it's time to adjust CC_HMAC_CONTEXT_SIZE */
    if((hmacCtx = malloc(sizeof(_NewHmacContext))) == NULL) return NULL;

	cc_clear(sizeof(_NewHmacContext), hmacCtx);

    if((hmacCtx->di = CCDigestGetDigestInfo(alg)) == NULL) {
        CC_DEBUG_LOG( "CCHMac Unknown Digest %d\n");
        free(hmacCtx);
        return NULL;
	}
    
    cchmac_init(hmacCtx->di, hmacCtx->ctx, keyLength, key);
	return (CCHmacContextRef) hmacCtx;
}

void CCHmacOneShot(CCDigestAlg alg,  const void *key, size_t keyLength, const void *data, size_t dataLength, void *macOut) {
    const struct ccdigest_info *di = CCDigestGetDigestInfo(alg);
    cchmac(di, keyLength, key, dataLength, data, macOut);
}

CCHmacContextRef
CCHmacClone(CCHmacContextRef ctx) {
	_NewHmacContext		*hmacCtx;
    
    CC_DEBUG_LOG("Entering\n");
	/* if this fails, it's time to adjust CC_HMAC_CONTEXT_SIZE */
    if((hmacCtx = malloc(sizeof(_NewHmacContext))) == NULL) return NULL;

	memcpy(hmacCtx, ctx, sizeof(_NewHmacContext));
	return (CCHmacContextRef) hmacCtx;
}

