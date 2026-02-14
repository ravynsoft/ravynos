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

/*
 * CommonCryptor.c - common crypto context.
 *
 */

// #define COMMON_CRYPTOR_FUNCTIONS

#include "ccGlobals.h"
#include "ccdebug.h"
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonCryptorSPI.h>
#include "CommonCryptorPriv.h"

#include "CCCryptorReset_internal.h"

#ifdef DEBUG
#include <stdio.h>
#include "CommonRandomSPI.h"
#endif

/* 
 * CommonCryptor's portion of a CCCryptorRef. 
 */

static inline uint32_t ccGetCipherBlockSize(CCCryptor *ref) {
    switch(ref->cipher) {
        case kCCAlgorithmAES128:    return kCCBlockSizeAES128;
        case kCCAlgorithmDES:       return kCCBlockSizeDES;  
        case kCCAlgorithm3DES:      return kCCBlockSize3DES;       
        case kCCAlgorithmCAST:      return kCCBlockSizeCAST;      
        case kCCAlgorithmRC4:       return 1;
        case kCCAlgorithmRC2:       return kCCBlockSizeRC2;
        case kCCAlgorithmBlowfish:  return kCCBlockSizeBlowfish;
        default: return kCCBlockSizeAES128;
    }
}

const corecryptoMode getCipherMode(CCAlgorithm cipher, CCMode mode, CCOperation direction)
{
    switch(mode) {
        case kCCModeECB:  return (corecryptoMode) ccmodeList[cipher][direction].ecb();
        case kCCModeCBC:  return (corecryptoMode) ccmodeList[cipher][direction].cbc();
        case kCCModeCFB:  return (corecryptoMode) ccmodeList[cipher][direction].cfb();
        case kCCModeCFB8: return (corecryptoMode) ccmodeList[cipher][direction].cfb8();
        case kCCModeCTR:  return (corecryptoMode) ccmodeList[cipher][direction].ctr();
        case kCCModeOFB:  return (corecryptoMode) ccmodeList[cipher][direction].ofb();
        case kCCModeXTS:  return (corecryptoMode) ccmodeList[cipher][direction].xts();
        case kCCModeGCM:  return (corecryptoMode) ccmodeList[cipher][direction].gcm();
        case kCCModeCCM:  return (corecryptoMode) ccmodeList[cipher][direction].ccm();
    }
    return (corecryptoMode) (const struct ccmode_ecb*) NULL;
}

static inline CCCryptorStatus setCryptorCipherMode(CCCryptor *ref, CCAlgorithm cipher, CCMode mode, CCOperation direction) {
    switch(mode) {
        case kCCModeECB: if((ref->symMode[direction].ecb = getCipherMode(cipher, mode, direction).ecb) == NULL) return kCCUnimplemented;
            ref->modeDesc = &ccecb_mode; break;
        case kCCModeCBC: if((ref->symMode[direction].cbc = getCipherMode(cipher, mode, direction).cbc) == NULL) return kCCUnimplemented;
            ref->modeDesc = &cccbc_mode; break;
        case kCCModeCFB: if((ref->symMode[direction].cfb = getCipherMode(cipher, mode, direction).cfb) == NULL) return kCCUnimplemented;
            ref->modeDesc = &cccfb_mode; break;
        case kCCModeCFB8: if((ref->symMode[direction].cfb8 = getCipherMode(cipher, mode, direction).cfb8) == NULL) return kCCUnimplemented;
            ref->modeDesc = &cccfb8_mode; break;
        case kCCModeCTR: if((ref->symMode[direction].ctr = getCipherMode(cipher, mode, direction).ctr) == NULL) return kCCUnimplemented;
            ref->modeDesc = &ccctr_mode; break;
        case kCCModeOFB: if((ref->symMode[direction].ofb = getCipherMode(cipher, mode, direction).ofb) == NULL) return kCCUnimplemented;
            ref->modeDesc = &ccofb_mode; break;
        case kCCModeXTS: if((ref->symMode[direction].xts = getCipherMode(cipher, mode, direction).xts) == NULL) return kCCUnimplemented;
            ref->modeDesc = &ccxts_mode; break;
        case kCCModeGCM: if((ref->symMode[direction].gcm = getCipherMode(cipher, mode, direction).gcm) == NULL) return kCCUnimplemented;
            ref->modeDesc = &ccgcm_mode; break;
        case kCCModeCCM: if((ref->symMode[direction].ccm = getCipherMode(cipher, mode, direction).ccm) == NULL) return kCCUnimplemented;
            ref->modeDesc = &ccccm_mode; break;
        default: return kCCParamError;
    }
    return kCCSuccess;

}


static inline CCCryptorStatus ccSetupCryptor(CCCryptor *ref, CCAlgorithm cipher, CCMode mode, CCOperation direction, CCPadding padding)
{
    CCCryptorStatus retval;
    
    if(cipher > 6) return kCCParamError;
    if(direction > kCCBoth) return kCCParamError;
    if(cipher == kCCAlgorithmRC4) mode = kCCModeOFB;
    
    ref->mode = mode;
    CCOperation op = direction;
    if(ref->mode == kCCModeXTS || ref->mode == kCCModeECB || ref->mode == kCCModeCBC) op = kCCBoth;
    ref->ctx[kCCEncrypt].data = NULL;
    ref->ctx[kCCDecrypt].data = NULL;
    
    // printf("Cryptor setup - cipher %d mode %d direction %d padding %d\n", cipher, mode, direction, padding);
    switch(op) {
        case kCCEncrypt:
        case kCCDecrypt:
            if((retval = setCryptorCipherMode(ref, cipher, mode, op)) != kCCSuccess) return retval;
            if((ref->ctx[op].data = malloc(ref->modeDesc->mode_get_ctx_size(ref->symMode[op]))) == NULL) return kCCMemoryFailure;
            break;
        case kCCBoth:
            if((retval = setCryptorCipherMode(ref, cipher, mode, kCCEncrypt)) != kCCSuccess) return retval;
            if((ref->ctx[kCCEncrypt].data = malloc(ref->modeDesc->mode_get_ctx_size(ref->symMode[kCCEncrypt]))) == NULL) return kCCMemoryFailure;
            if((retval = setCryptorCipherMode(ref, cipher, mode, kCCDecrypt)) != kCCSuccess) return retval;
            if((ref->ctx[kCCDecrypt].data = malloc(ref->modeDesc->mode_get_ctx_size(ref->symMode[kCCDecrypt]))) == NULL) return kCCMemoryFailure;
            break;
    }
    
    switch(padding) {
        case ccNoPadding:
            ref->padptr = &ccnopad_pad;
            break;
        case ccPKCS7Padding:
            if(mode == kCCModeCBC)
                ref->padptr = &ccpkcs7_pad;
            else
                ref->padptr = &ccpkcs7_ecb_pad;
            break;
        case ccCBCCTS3:
            ref->padptr = &cccts3_pad;
            break;
        default:
            ref->padptr = &ccnopad_pad;
    }
    ref->cipher = cipher;
    ref->cipherBlocksize = ccGetCipherBlockSize(ref);
    ref->op = direction;
    ref->bufferPos = 0;
    ref->bytesProcessed = 0;
    return kCCSuccess;
}

#define OP4INFO(X) (((X)->op == 3) ? 0: (X)->op)

static inline bool ccIsStreaming(CCCryptor *ref) {
    return ref->modeDesc->mode_get_block_size(ref->symMode[ref->op]) == 1;
}

static int check_algorithm_keysize(CCAlgorithm alg, size_t keysize)
{
    int rc;

#if (kCCAlgorithmAES128!= kCCAlgorithmAES)
#error If kCCAlgorithmAES128 and kCCAlgorithmAES are not the same, a case statement must be defined
#endif

    switch (alg) {
        //case kCCAlgorithmAES128:
        case kCCAlgorithmAES: rc = keysize==kCCKeySizeAES128 || keysize==kCCKeySizeAES192 || keysize==kCCKeySizeAES256; break;
        case kCCAlgorithmDES: rc = keysize==kCCKeySizeDES; break;
        case kCCAlgorithm3DES:rc = keysize==kCCKeySize3DES; break;
        case kCCAlgorithmCAST:rc = keysize>=kCCKeySizeMinCAST && keysize<=kCCKeySizeMaxCAST; break;
        case kCCAlgorithmRC4: rc = keysize>=kCCKeySizeMinRC4  && keysize<=kCCKeySizeMaxRC4; break;
        case kCCAlgorithmRC2: rc = keysize>=kCCKeySizeMinRC2  && keysize<=kCCKeySizeMaxRC2; break;
        case kCCAlgorithmBlowfish: rc = keysize>=kCCKeySizeMinBlowfish && keysize<=kCCKeySizeMaxBlowfish; break;

        default: rc=0;
    }
    return rc==1?0:-1;

}

static inline CCCryptorStatus ccInitCryptor(CCCryptor *ref, const void *key, unsigned long key_len, const void *tweak_key, const void *iv)
{
    int ccrc = CCERR_OK;
    if( check_algorithm_keysize(ref->cipher, key_len) <0 )
        return kCCKeySizeError;

    size_t blocksize = ccGetCipherBlockSize(ref);
    uint8_t defaultIV[blocksize];
    
    if(iv == NULL) {
        cc_clear(blocksize, defaultIV);
        iv = defaultIV;
    }
     
    CCOperation op = ref->op;
    
    // This will create both sides of the context/mode pairs for now.
    if(ref->mode == kCCModeXTS || ref->mode == kCCModeECB || ref->mode == kCCModeCBC) op = kCCBoth;
    
    switch(op) {
        case kCCEncrypt:
        case kCCDecrypt:
            ccrc = ref->modeDesc->mode_setup(ref->symMode[ref->op], iv, key, key_len, tweak_key, 0, 0, ref->ctx[ref->op]);
            break;
        case kCCBoth:
            ccrc = ref->modeDesc->mode_setup(ref->symMode[kCCEncrypt], iv, key, key_len, tweak_key, 0, 0, ref->ctx[kCCEncrypt]);
            ccrc |= ref->modeDesc->mode_setup(ref->symMode[kCCDecrypt], iv, key, key_len, tweak_key, 0, 0, ref->ctx[kCCDecrypt]);
            break;
    }

    // In practice we won't fail on initialization of anything except
    // 1. XTS when the data key and tweak key are equal
    // 2. 3DES when the key's are equal
    // We need to ignore the error in these cases so as not to break clients.
    if (ccrc == CCERR_OK) {
        return kCCSuccess;
    } else if (ref->cipher == kCCAlgorithm3DES || ref->mode == kCCModeXTS) {
        // Ignore the error
        return kCCSuccess;
    } else {
        return kCCUnspecifiedError;
    }
}

static inline CCCryptorStatus ccDoEnCrypt(CCCryptor *ref, const void *dataIn, size_t dataInLength, void *dataOut) {
    if(!ref->modeDesc->mode_encrypt) return kCCParamError;
    
    int ccrc = ref->modeDesc->mode_encrypt(ref->symMode[kCCEncrypt], dataIn, dataOut, dataInLength, ref->ctx[kCCEncrypt]);
    
    if (ccrc == CCMODE_INVALID_CALL_SEQUENCE) {
        return kCCCallSequenceError;
    } else if (ccrc != CCERR_OK) {
        return kCCParamError;
    }
    return kCCSuccess;
}

static inline CCCryptorStatus ccDoDeCrypt(CCCryptor *ref, const void *dataIn, size_t dataInLength, void *dataOut) {
    if(!ref->modeDesc->mode_decrypt) return kCCParamError;
    
    int ccrc = ref->modeDesc->mode_decrypt(ref->symMode[kCCDecrypt], dataIn, dataOut, dataInLength, ref->ctx[kCCDecrypt]);
    
    if (ccrc == CCMODE_INVALID_CALL_SEQUENCE) {
        return kCCCallSequenceError;
    } else if (ccrc != CCERR_OK) {
        return kCCParamError;
    }
    return kCCSuccess;
}

static inline CCCryptorStatus ccDoEnCryptTweaked(CCCryptor *ref, const void *dataIn, size_t dataInLength, void *dataOut, const void *tweak) {
    if(!ref->modeDesc->mode_encrypt_tweaked) return kCCParamError;
    
    int ccrc = ref->modeDesc->mode_encrypt_tweaked(ref->symMode[kCCEncrypt], dataIn, dataInLength, dataOut, tweak, ref->ctx[kCCEncrypt]);
    
    if (ccrc != CCERR_OK) {
        return kCCParamError;
    }
    return kCCSuccess;
}

static inline CCCryptorStatus ccDoDeCryptTweaked(CCCryptor *ref, const void *dataIn, size_t dataInLength, void *dataOut, const void *tweak) {
    if(!ref->modeDesc->mode_decrypt_tweaked) return kCCParamError;
    
    int ccrc = ref->modeDesc->mode_decrypt_tweaked(ref->symMode[kCCDecrypt], dataIn, dataInLength, dataOut, tweak, ref->ctx[kCCDecrypt]);
    
    if (ccrc != CCERR_OK) {
        return kCCParamError;
    }
    return kCCSuccess;
}

static inline CCCryptorStatus ccGetIV(CCCryptor *ref, void *iv, size_t *ivLen) {
    if(ref->modeDesc->mode_getiv == NULL) return kCCParamError;
    uint32_t tmp = (uint32_t) *ivLen;
    if(ref->modeDesc->mode_getiv(ref->symMode[OP4INFO(ref)], iv, &tmp, ref->ctx[OP4INFO(ref)]) != 0) return kCCMemoryFailure;
    *ivLen = tmp;
    return kCCSuccess;
}

static inline CCCryptorStatus ccSetIV(CCCryptor *ref, const void *iv, size_t ivLen) {
    if(ref->modeDesc->mode_setiv == NULL) return kCCParamError;
    if(ref->modeDesc->mode_setiv(ref->symMode[OP4INFO(ref)], iv, (uint32_t ) ivLen, ref->ctx[OP4INFO(ref)]) != 0) return kCCMemoryFailure;
    return kCCSuccess;
}

static inline void ccClearCryptor(CCCryptor *ref) {
    cc_clear(sizeof(ref->buffptr), ref->buffptr);
    CCOperation op = ref->op;
    
    // This will clear both sides of the context/mode pairs for now.
    if(ref->mode == kCCModeXTS || ref->mode == kCCModeECB || ref->mode == kCCModeCBC) op = kCCBoth;
    switch(op) {
        case kCCEncrypt:
        case kCCDecrypt:
            cc_clear(ref->modeDesc->mode_get_ctx_size(ref->symMode[ref->op]), ref->ctx[ref->op].data);
            free(ref->ctx[ref->op].data);
            break;
        case kCCBoth:
            for(int i = 0; i<2; i++) {
                cc_clear(ref->modeDesc->mode_get_ctx_size(ref->symMode[i]), ref->ctx[i].data);
                free(ref->ctx[i].data);
            }
            break;
    }
    cc_clear(CCCRYPTOR_SIZE, ref);
}

static inline void returnLengthIfPossible(size_t length, size_t *returnPtr) {
    if(returnPtr) *returnPtr = length;
}

static inline CCCryptorStatus ccEncryptPad(CCCryptor *cryptor, void *buf, size_t *moved) {
    if(cryptor->padptr->encrypt_pad(cryptor->ctx[cryptor->op], cryptor->modeDesc, cryptor->symMode[cryptor->op], cryptor->buffptr, cryptor->bufferPos, buf, moved)) return kCCAlignmentError;
    return kCCSuccess;
}

static inline CCCryptorStatus ccDecryptPad(CCCryptor *cryptor, void *buf, size_t *moved) {
    if(cryptor->padptr->decrypt_pad(cryptor->ctx[cryptor->op], cryptor->modeDesc, cryptor->symMode[cryptor->op], cryptor->buffptr, cryptor->bufferPos, buf, moved)) return kCCAlignmentError;
    return kCCSuccess;
}

static inline size_t ccGetReserve(CCCryptor *cryptor) {
    return cryptor->padptr->padreserve(cryptor->op == kCCEncrypt, cryptor->modeDesc, cryptor->symMode[cryptor->op]);
}

static inline size_t ccGetPadOutputlen(CCCryptor *cryptor, size_t inputLength, bool final) {
    size_t totalLen = cryptor->bufferPos + inputLength;
    return cryptor->padptr->padlen(cryptor->op == kCCEncrypt, cryptor->modeDesc, cryptor->symMode[cryptor->op], totalLen, final);
}


static inline CCCryptor *
ccCreateCompatCryptorFromData(const void *data, size_t dataLength, size_t *dataUsed) {
    uintptr_t startptr = (uintptr_t) data;
	uintptr_t retval = (uintptr_t) data;
    if((retval & 0x07) ) retval = ((startptr / 8) * 8 + 8);
    size_t usedLen = retval - startptr + CC_COMPAT_SIZE;
    returnLengthIfPossible(usedLen, dataUsed);
    if(usedLen > dataLength) return NULL;
	return (CCCryptor *) retval;
}

static inline int ccAddBuff(CCCryptor *cryptor, const void *dataIn, size_t dataInLength) {
    memcpy((char *) cryptor->buffptr + cryptor->bufferPos, dataIn, dataInLength);
    cryptor->bufferPos += dataInLength;
    return (int) dataInLength;
}


CCCryptorStatus CCCryptorCreateFromData(
    CCOperation op,             /* kCCEncrypt, etc. */
    CCAlgorithm alg,            /* kCCAlgorithmDES, etc. */
    CCOptions options,          /* kCCOptionPKCS7Padding, etc. */
    const void *key,            /* raw key material */
    size_t keyLength,	
    const void *iv,             /* optional initialization vector */
    const void *data,			/* caller-supplied memory */
    size_t dataLength,			/* length of data in bytes */
    CCCryptorRef *cryptorRef,   /* RETURNED */
    size_t *dataUsed)			/* optional, RETURNED */

{
    CCCryptor *cryptor = ccCreateCompatCryptorFromData(data, dataLength, dataUsed);
    if(!cryptor) return kCCBufferTooSmall;
    CCCryptorStatus err = CCCryptorCreate(op, alg, options, key,  keyLength, iv, &cryptor->compat);
    if(err == kCCSuccess) *cryptorRef = cryptor;    
    return err;
}

CCCryptorStatus CCCryptorCreate(
	CCOperation op,             /* kCCEncrypt, etc. */
	CCAlgorithm alg,            /* kCCAlgorithmDES, etc. */
	CCOptions options,          /* kCCOptionPKCS7Padding, etc. */
	const void *key,            /* raw key material */
	size_t keyLength,	
	const void *iv,             /* optional initialization vector */
	CCCryptorRef *cryptorRef)  /* RETURNED */
{
	CCMode			mode;
    CCPadding		padding;		
	const void 		*tweak;
	size_t 			tweakLength;	
	int				numRounds;
	CCModeOptions 	modeOptions;
    
    CC_DEBUG_LOG("Entering\n");
	/* Determine mode from options - old call only supported ECB and CBC 
       we treat RC4 as a "mode" in that it's the only streaming cipher
       currently supported 
    */
    if(alg == kCCAlgorithmRC4) mode = kCCModeRC4;
    else if(options & kCCOptionECBMode) mode = kCCModeECB;
	else mode = kCCModeCBC;
    
	/* Determine padding from options - only PKCS7 was available */
    padding = ccNoPadding;
	if(options & kCCOptionPKCS7Padding) padding = ccPKCS7Padding;
   
	/* No tweak was ever used */
   	tweak = NULL;
    tweakLength = 0;
    
	/* default rounds */
    numRounds = 0;
    
	/* No mode options needed */
    modeOptions = 0;
    
	return CCCryptorCreateWithMode(op, mode, alg, padding, iv, key, keyLength, tweak, tweakLength, numRounds, modeOptions, cryptorRef);
}

/* This version mallocs the CCCryptorRef */

CCCryptorStatus CCCryptorCreateFromDataWithMode(
    CCOperation 	op,				/* kCCEncrypt, kCCEncrypt, kCCBoth (default for BlockMode) */
    CCMode			mode,
    CCAlgorithm		alg,
    CCPadding		padding,		
    const void 		*iv,			/* optional initialization vector */
    const void 		*key,			/* raw key material */
    size_t 			keyLength,	
    const void 		*tweak,			/* raw tweak material */
    size_t 			tweakLength,	
    int				numRounds,
    CCModeOptions 	options,
    const void		*data,			/* caller-supplied memory */
    size_t			dataLength,		/* length of data in bytes */
    CCCryptorRef	*cryptorRef,	/* RETURNED */
    size_t			*dataUsed)		/* optional, RETURNED */
{
    CCCryptor *cryptor = ccCreateCompatCryptorFromData(data, dataLength, dataUsed);
    if(!cryptor) return kCCBufferTooSmall;
    CCCryptorStatus err = CCCryptorCreateWithMode(op, mode, alg, padding, iv, key,  keyLength, tweak, tweakLength, numRounds, options, &cryptor->compat);
    if(err == kCCSuccess) *cryptorRef = cryptor;    
    return err;
}

#define KEYALIGNMENT (sizeof(int)-1)
CCCryptorStatus CCCryptorCreateWithMode(
	CCOperation 	op,				/* kCCEncrypt, kCCEncrypt, kCCBoth (default for BlockMode) */
	CCMode			mode,
	CCAlgorithm		alg,
	CCPadding		padding,		
	const void 		*iv,			/* optional initialization vector */
	const void 		*key,			/* raw key material */
	size_t 			keyLength,	
	const void 		*tweak,			/* raw tweak material */
	size_t 			 __unused tweakLength,	
	int				 __unused numRounds,		/* 0 == default */
	CCModeOptions 	__unused options,
	CCCryptorRef	*cryptorRef)	/* RETURNED */
{
	CCCryptorStatus retval = kCCSuccess;
	CCCryptor *cryptor = NULL;
    uint8_t *alignedKey = NULL;

    CC_DEBUG_LOG("Entering Op: %d Mode: %d Cipher: %d Padding: %d\n", op, mode, alg, padding);

    // validate pointers
	if((cryptorRef == NULL) || (key == NULL)) {
		CC_DEBUG_LOG("bad arguments\n", 0);
		return kCCParamError;
	}
    
    /*
     * Some implementations are sensitive to keys not being 4 byte aligned.
     * We'll move the key into an aligned buffer for the call to setup
     * the key schedule.
     */
    
    if((intptr_t) key & KEYALIGNMENT) {
        if((alignedKey = malloc(keyLength)) == NULL) {
            return kCCMemoryFailure;
        }
        memcpy(alignedKey, key, keyLength);
        key = alignedKey;
    }

    if((cryptor = (CCCryptor *)malloc(DEFAULT_CRYPTOR_MALLOC)) == NULL) {
        retval = kCCMemoryFailure;
        goto out;
    }
	
    cryptor->compat = NULL;
    
    if((retval = ccSetupCryptor(cryptor, alg, mode, op, padding)) != kCCSuccess) {
        goto out;
    }
    
    if((retval = ccInitCryptor(cryptor, key, keyLength, tweak, iv)) != kCCSuccess) {
        goto out;
    }

	*cryptorRef = cryptor;
#ifdef DEBUG
    cryptor->active = ACTIVE;
    retval=CCRandomGenerateBytes(&cryptor->cryptorID, sizeof(cryptor->cryptorID));
#endif

out:
    // Things to destroy if setup failed
    if(retval) {
        *cryptorRef = NULL;
        if(cryptor) {
            ccClearCryptor(cryptor);
            free(cryptor);
        }
    } else {
        // printf("Blocksize = %d mode = %d pad = %d\n", ccGetBlockSize(cryptor), cryptor->mode, padding);
    }
    
    // Things to destroy all the time
    if(alignedKey) {
        cc_clear(keyLength, alignedKey);
        free(alignedKey);
    }
    
    return retval;
}


CCCryptorStatus CCCryptorRelease(
	CCCryptorRef cryptorRef)
{
    CCCryptor *cryptor = getRealCryptor(cryptorRef, 0);
    
    CC_DEBUG_LOG("Entering\n");
    if(cryptor) {
        ccClearCryptor(cryptor);
        free(cryptor);
    }
	return kCCSuccess;
}

#define FULLBLOCKSIZE(X,BLOCKSIZE) (((X)/(BLOCKSIZE))*BLOCKSIZE)
#define FULLBLOCKREMAINDER(X,BLOCKSIZE) ((X)%(BLOCKSIZE))

static CCCryptorStatus ccSimpleUpdate(CCCryptor *cryptor, const void *dataIn, size_t dataInLength, void **dataOut, size_t *dataOutAvailable, size_t *dataOutMoved)
{		
	CCCryptorStatus	retval;
    if(cryptor->op == kCCEncrypt) {
        if((retval = ccDoEnCrypt(cryptor, dataIn, dataInLength, *dataOut)) != kCCSuccess) return retval;
    } else {
        if((retval = ccDoDeCrypt(cryptor, dataIn, dataInLength, *dataOut)) != kCCSuccess) return retval;
    }
    if(dataOutMoved) *dataOutMoved += dataInLength;
    if(*dataOutAvailable < dataInLength) return kCCBufferTooSmall;
    cryptor->bytesProcessed += dataInLength;
    *dataOut += dataInLength;
    *dataOutAvailable -= dataInLength;
    return kCCSuccess;
}

static CCCryptorStatus ccBlockUpdate(CCCryptor *cryptor, const void *dataIn, size_t dataInLength, void *dataOut, size_t *dataOutAvailable, size_t *dataOutMoved)
{
    CCCryptorStatus retval;
    size_t dataCount = cryptor->bufferPos + dataInLength;
    size_t reserve = ccGetReserve(cryptor);
    size_t blocksize = ccGetCipherBlockSize(cryptor);
    size_t buffsize = (reserve) ? reserve: blocksize; /* minimum buffering is a block */
    size_t dataCountToHold, dataCountToProcess;
    size_t remainder, movecnt;
    
    /* This is a simple optimization */
    if(reserve == 0 && cryptor->bufferPos == 0 && (dataInLength % blocksize) == 0) { // No Padding, not buffering, even blocks
        // printf("simple processing\n");
    	return ccSimpleUpdate(cryptor, dataIn, dataInLength, &dataOut, dataOutAvailable, dataOutMoved);
    }
    
    /* From this point on we're dealing with a Block Cipher with Block oriented I/O
     
     We always fallback to buffering once we're processing non-block aligned data.
     If the data inputs result in data becoming block aligned once again we can 
     move back to block aligned I/O - even if it's only for partial processing
     of the data supplied to this routine.
     
     */
    
    if(dataCount <= reserve) {
    	dataCountToHold = dataCount;
    } else {
    	remainder = FULLBLOCKREMAINDER(dataCount, blocksize);
		dataCountToHold = buffsize - blocksize + remainder;
        dataCountToHold = (remainder) ? dataCountToHold: reserve;
    }
    
    dataCountToProcess = dataCount - dataCountToHold;
    // printf("DataCount %d Processing %d Holding %d\n", dataCount, dataCountToProcess, dataCountToHold);
    
    if(dataCountToProcess > 0) {
    	if(cryptor->bufferPos == 0) {
        	// printf("CCCryptorUpdate checkpoint 0\n");
        	/* nothing to do yet */
    	} else if(cryptor->bufferPos < dataCountToProcess) {
        	// printf("CCCryptorUpdate checkpoint 1\n");
            movecnt = blocksize - (cryptor->bufferPos % blocksize);
            ccAddBuff(cryptor, dataIn, movecnt);
            dataIn += movecnt; dataInLength -= movecnt;
            
         	// printf("CCCryptorUpdate checkpoint 1.1 bufpos = %d\n", (int) cryptor->bufferPos);
           	if((retval = ccSimpleUpdate(cryptor, cryptor->buffptr, cryptor->bufferPos, &dataOut, dataOutAvailable, dataOutMoved)) != kCCSuccess) {
                return retval;
        	}
			// printf("CCCryptorUpdate checkpoint 1.2\n");
            
			dataCountToProcess -= cryptor->bufferPos;
        	cryptor->bufferPos = 0;
        } else if(cryptor->bufferPos == dataCountToProcess) {
        	// printf("CCCryptorUpdate checkpoint 2\n");
			if((retval = ccSimpleUpdate(cryptor, cryptor->buffptr, cryptor->bufferPos, &dataOut, dataOutAvailable, dataOutMoved)) != kCCSuccess) {
                return retval;
        	}
			dataCountToProcess -= cryptor->bufferPos;
        	cryptor->bufferPos = 0;
        } else /* (cryptor->bufferPos > dataCountToProcess) */ {
         	// printf("CCCryptorUpdate checkpoint 3\n");
       		if(dataCountToHold) {
            	// printf("CCCryptorUpdate bad calculation 1\n");
                return kCCDecodeError;
            }
			if((retval = ccSimpleUpdate(cryptor, cryptor->buffptr, dataCountToProcess, &dataOut, dataOutAvailable, dataOutMoved)) != kCCSuccess) {
                return retval;
        	}
            cryptor->bufferPos = (uint32_t) (reserve - dataCountToProcess);
            memmove(cryptor->buffptr, ((uint8_t *) cryptor->buffptr)+ dataCountToProcess, cryptor->bufferPos);
            return kCCSuccess;
        }
        
        if(dataCountToProcess > 0) {
         	// printf("CCCryptorUpdate checkpoint 4\n");
   			movecnt = FULLBLOCKREMAINDER(dataCountToProcess, blocksize);
            if(movecnt) {
            	// printf("CCCryptorUpdate bad calculation 2\n");
                return kCCDecodeError;
            }
        	if((retval = ccSimpleUpdate(cryptor, dataIn, dataCountToProcess, &dataOut, dataOutAvailable, dataOutMoved)) != kCCSuccess) return retval;
        	dataIn += dataCountToProcess; dataInLength -= dataCountToProcess;
        }
    }
    
    if(dataCountToHold) {
		// printf("CCCryptorUpdate checkpoint 1\n");
    	movecnt = dataCountToHold - cryptor->bufferPos;
        if(movecnt) {
        	if(movecnt != dataInLength) {
            	// printf("CCCryptorUpdate bad calculation 3\n");
                return kCCDecodeError;
            }
            ccAddBuff(cryptor, dataIn, movecnt);
        	dataInLength -= movecnt;
        }
    }
    
    if(dataInLength) {
        // printf("CCCryptorUpdate bad calculation 4\n");
        return kCCDecodeError;
    }
    return kCCSuccess;
}

static inline size_t ccGetOutputLength(CCCryptor *cryptor, size_t inputLength, bool final) {
    if(ccIsStreaming(cryptor)) return inputLength;
    return ccGetPadOutputlen(cryptor, inputLength, final);
}

size_t CCCryptorGetOutputLength(
    CCCryptorRef cryptorRef,
    size_t inputLength,
    bool final)
{
    CC_DEBUG_LOG("Entering\n");
    CCCryptor *cryptor = getRealCryptor(cryptorRef, 1);
    if(cryptor == NULL) return kCCParamError;
    return ccGetOutputLength(cryptor, inputLength, final);
}

CCCryptorStatus CCCryptorUpdate(
    CCCryptorRef cryptorRef,
    const void *dataIn,
    size_t dataInLength,
    void *dataOut,
    size_t dataOutAvailable,
    size_t *dataOutMoved)
{
    CC_DEBUG_LOG("Entering\n");
	CCCryptorStatus	retval;
    CCCryptor *cryptor = getRealCryptor(cryptorRef, 1);
    if(!cryptor) return kCCParamError;
	if(dataOutMoved) *dataOutMoved = 0;
    if(0 == dataInLength) return kCCSuccess;

    size_t needed = ccGetOutputLength(cryptor, dataInLength, false);
    if(needed > dataOutAvailable) {
        if(dataOutMoved) *dataOutMoved = needed;
        return kCCBufferTooSmall;
    }

    
	if(ccIsStreaming(cryptor))
        retval = ccSimpleUpdate(cryptor, dataIn, dataInLength, &dataOut, &dataOutAvailable, dataOutMoved);
    else
        retval = ccBlockUpdate(cryptor, dataIn, dataInLength, dataOut, &dataOutAvailable, dataOutMoved);

    return retval;
}



CCCryptorStatus CCCryptorFinal(
	CCCryptorRef cryptorRef,
	void *dataOut,					/* data RETURNED here */
	size_t dataOutAvailable,
	size_t *dataOutMoved)		/* number of bytes written */
{
    CC_DEBUG_LOG("Entering\n");
    CCCryptor   *cryptor = getRealCryptor(cryptorRef, 0);
    // Some old behavior .. CDSA? has zapped the Cryptor.
    if(cryptor == NULL) return kCCSuccess;
    
    
	CCCryptorStatus	retval;
    int encrypting = (cryptor->op == kCCEncrypt);
	uint32_t blocksize = ccGetCipherBlockSize(cryptor);
    
    size_t moved;
	char tmpbuf[blocksize*2];

	if(dataOutMoved) *dataOutMoved = 0;

    if(ccIsStreaming(cryptor)) {
        if(cryptor->modeDesc->mode_done) {
            cryptor->modeDesc->mode_done(cryptor->symMode[cryptor->op], cryptor->ctx[cryptor->op]);
        }
        return kCCSuccess;
    }

	if(encrypting) {
        retval = ccEncryptPad(cryptor, tmpbuf, &moved);
        if(retval != kCCSuccess) return retval;
		if(dataOutAvailable < moved) {
            return kCCBufferTooSmall;
        }
        if(dataOut) {
            memcpy(dataOut, tmpbuf, moved);
            if(dataOutMoved) *dataOutMoved = moved;
        }
		cryptor->bufferPos = 0;
	} else {
        retval = ccDecryptPad(cryptor, tmpbuf, &moved);
        if(retval != kCCSuccess) return retval;
        if(dataOutAvailable < moved) {
            return kCCBufferTooSmall;
        }
        if(dataOut) {
            memcpy(dataOut, tmpbuf, moved);
            if(dataOutMoved) *dataOutMoved = moved;
        }
        cryptor->bytesProcessed += moved;
        cryptor->bufferPos = 0;
	}
    cryptor->bufferPos = 0;
#ifdef DEBUG
    cryptor->active = RELEASED;
#endif
	return kCCSuccess;
}

// This is the old reset function that could be called mistakenly for
// modes other than CBC
CCCryptorStatus CCCryptorReset_binary_compatibility(
                               CCCryptorRef cryptorRef,
                               const void *iv)
{    CC_DEBUG_LOG("Entering\n");
    CCCryptor *cryptor = getRealCryptor(cryptorRef, 1);
    if(!cryptor) return kCCParamError;
    CCCryptorStatus retval;
    
    /*
     This routine resets all buffering and sets or clears the IV.  It is
     documented to throw away any in-flight buffer data.
     */
    
    cryptor->bytesProcessed = cryptor->bufferPos = 0;
    
    /*
     Call the common routine to reset the IV - this will copy in the new
     value. There is now always space for an IV in the cryptor.
     */
    
    if(iv) {
        retval = ccSetIV(cryptor, iv, ccGetCipherBlockSize(cryptor));
    } else {
        uint8_t ivzero[ccGetCipherBlockSize(cryptor)];
        cc_clear(ccGetCipherBlockSize(cryptor), ivzero);
        retval = ccSetIV(cryptor, ivzero, ccGetCipherBlockSize(cryptor));
    }
    if(retval == kCCParamError) return kCCSuccess; //that is for when reset is unimplemented
    return retval;
}

CCCryptorStatus CCCryptorReset(CCCryptorRef cryptorRef, const void *iv)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if(!ProgramLinkedOnOrAfter_macOS1013_iOS11()) //switch to the old behavior
        return CCCryptorReset_binary_compatibility(cryptorRef, iv);
#pragma clang diagnostic pop
    
    //continue with the new behavior: can only be called for CBC
    CC_DEBUG_LOG("Entering\n");
    CCCryptor *cryptor = getRealCryptor(cryptorRef, 1);
    if(!cryptor) return kCCParamError;
    if(cryptor->mode != kCCModeCBC && cryptor->mode != kCCModeCTR) {
        return kCCUnimplemented;
    }
    CCCryptorStatus retval;
    
    /*
        This routine resets all buffering and sets or clears the IV.  It is
        documented to throw away any in-flight buffer data. Currectly, it only works
        for the CBC mode.
    */
    
    cryptor->bytesProcessed = cryptor->bufferPos = 0;
    
    /*
        Call the common routine to reset the IV - this will copy in the new
           value. There is now always space for an IV in the cryptor.
    */
    
    if(iv) {
        retval = ccSetIV(cryptor, iv, ccGetCipherBlockSize(cryptor));
    } else {
        uint8_t ivzero[ccGetCipherBlockSize(cryptor)];
        cc_clear(ccGetCipherBlockSize(cryptor), ivzero);
        retval = ccSetIV(cryptor, ivzero, ccGetCipherBlockSize(cryptor));
    }
    
    return retval;
}

CCCryptorStatus
CCCryptorGetIV(CCCryptorRef cryptorRef, void *iv)
{
    CCCryptor   *cryptor = getRealCryptor(cryptorRef, 1);
    CC_DEBUG_LOG("Entering\n");
    if(!cryptor) return kCCParamError;
    
    if(ccIsStreaming(cryptor)) return kCCParamError;
    size_t blocksize = ccGetCipherBlockSize(cryptor);
    return ccGetIV(cryptor, iv, &blocksize);
}

/* 
 * One-shot is mostly service provider independent, except for the
 * dataOutLength check.
 */
CCCryptorStatus CCCrypt(
	CCOperation op,			/* kCCEncrypt, etc. */
	CCAlgorithm alg,		/* kCCAlgorithmAES128, etc. */
	CCOptions options,		/* kCCOptionPKCS7Padding, etc. */
	const void *key,
	size_t keyLength,
	const void *iv,			/* optional initialization vector */
	const void *dataIn,		/* optional per op and alg */
	size_t dataInLength,
	void *dataOut,			/* data RETURNED here */
	size_t dataOutAvailable,
	size_t *dataOutMoved)	
{
    CC_DEBUG_LOG("Entering\n");
	CCCryptorRef cryptor = NULL;
	CCCryptorStatus retval;
	size_t updateLen, finalLen;
            
	if(kCCSuccess != (retval = CCCryptorCreate(op, alg, options, key, keyLength, iv, &cryptor))) return retval;
    size_t needed = CCCryptorGetOutputLength(cryptor, dataInLength, true);
    if(dataOutMoved != NULL) *dataOutMoved = needed;
    if(needed > dataOutAvailable) {
        retval = kCCBufferTooSmall;
        goto out;
    }
    
	if(kCCSuccess != (retval = CCCryptorUpdate(cryptor, dataIn, dataInLength, dataOut, dataOutAvailable, &updateLen))) {
        goto out;
    }
    dataOut += updateLen; dataOutAvailable -= updateLen;
    retval = CCCryptorFinal(cryptor, dataOut, dataOutAvailable, &finalLen);
    if(dataOutMoved != NULL) *dataOutMoved = updateLen + finalLen;
out:
	CCCryptorRelease(cryptor);
	return retval;
}

CCCryptorStatus CCCryptorEncryptDataBlock(
	CCCryptorRef cryptorRef,
	const void *iv,
	const void *dataIn,
	size_t dataInLength,
	void *dataOut)
{
    CC_DEBUG_LOG("Entering\n");
    CCCryptor   *cryptor = getRealCryptor(cryptorRef, 1);
    if(!cryptor) return kCCParamError;
    if(ccIsStreaming(cryptor)) return kCCParamError;
    if(!iv) return ccDoEnCrypt(cryptor, dataIn, dataInLength, dataOut);
    return ccDoEnCryptTweaked(cryptor, dataIn, dataInLength, dataOut, iv);    
}


CCCryptorStatus CCCryptorDecryptDataBlock(
	CCCryptorRef cryptorRef,
	const void *iv,
	const void *dataIn,
	size_t dataInLength,
	void *dataOut)
{
    CC_DEBUG_LOG("Entering\n");
    CCCryptor   *cryptor = getRealCryptor(cryptorRef, 1);
    if(!cryptor) return kCCParamError;
    if(ccIsStreaming(cryptor)) return kCCParamError;
    if(!iv) return ccDoDeCrypt(cryptor, dataIn, dataInLength, dataOut);
    return ccDoDeCryptTweaked(cryptor, dataIn, dataInLength, dataOut, iv);    
}

static bool ccm_ready(modeCtx ctx) {
// FIX THESE NOW XXX
    if(ctx.ccm->mac_size == (size_t) 0xffffffffffffffff ||
       ctx.ccm->nonce_size == (size_t) 0xffffffffffffffff ||
       ctx.ccm->total_len == (size_t) 0xffffffffffffffff) return false;
    return true;
}

CCCryptorStatus CCCryptorAddParameter(
    CCCryptorRef cryptorRef,
    CCParameter parameter,
    const void *data,
    size_t dataSize)
{
    CC_DEBUG_LOG("Entering\n");
    CCCryptor   *cryptor = getRealCryptor(cryptorRef, 1);
    if(!cryptor) return kCCParamError;
    
    int rc = CCERR_OK;

    switch(parameter) {
    case kCCParameterIV:
        // GCM version
        if(cryptor->mode == kCCModeGCM) {
            rc = ccgcm_set_iv_legacy(cryptor->symMode[cryptor->op].gcm,cryptor->ctx[cryptor->op].gcm, dataSize, data);
            if (rc != CCERR_OK) return kCCParamError;
        } else if(cryptor->mode == kCCModeCCM) {
            ccm_nonce_ctx *ccm = cryptor->ctx[cryptor->op].ccm;
            ccm->nonce_size = dataSize;
            memcpy(ccm->nonce_buf, data, dataSize);
        } else return kCCUnimplemented;
        break;

    case kCCParameterAuthData:
        // GCM version
        if(cryptor->mode == kCCModeGCM) {
            rc = ccgcm_aad(cryptor->symMode[cryptor->op].gcm,cryptor->ctx[cryptor->op].gcm, dataSize, data);
            if (rc != CCERR_OK) {
                return kCCCallSequenceError;
            }
        } else if(cryptor->mode == kCCModeCCM) {
            if(!ccm_ready(cryptor->ctx[cryptor->op])) return kCCParamError;
            ccm_nonce_ctx *ccm = cryptor->ctx[cryptor->op].ccm;
            const struct ccmode_ccm *mode = cryptor->symMode[cryptor->op].ccm;
            ccm->ad_len = dataSize;
            rc = ccccm_set_iv(mode,&ccm->ccm, (ccccm_nonce *) &ccm->nonce,
                    ccm->nonce_size, ccm->nonce_buf, ccm->mac_size, ccm->ad_len, ccm->total_len);
            if (rc != CCERR_OK) {
                return kCCParamError;
            }
            rc = ccccm_cbcmac(mode,&ccm->ccm, (ccccm_nonce *) &ccm->nonce,
                    ccm->ad_len, data);
            if (rc != CCERR_OK) {
                return kCCCallSequenceError;
            }
        } else return kCCUnimplemented;
        break;
        
    case kCCMacSize:
        if(cryptor->mode == kCCModeCCM) {
            cryptor->ctx[cryptor->op].ccm->mac_size = dataSize;
        } else return kCCUnimplemented;
        break;
        
    case kCCDataSize:
        if(cryptor->mode == kCCModeCCM) {
            cryptor->ctx[cryptor->op].ccm->total_len = dataSize;
        } else return kCCUnimplemented;
        break;
        
    default:
        return kCCParamError;
    }
    return kCCSuccess;
}

CCCryptorStatus CCCryptorGetParameter(    
    CCCryptorRef cryptorRef,
    CCParameter parameter,
    void *data,
    size_t *dataSize)
{
    CC_DEBUG_LOG("Entering\n");
    CCCryptor   *cryptor = getRealCryptor(cryptorRef, 1);
    if(!cryptor) return kCCParamError;

    switch(parameter) {
    case kCCParameterAuthTag:
        // GCM version
        if(cryptor->mode == kCCModeGCM) {
            return kCCUnimplemented;
        } else if(cryptor->mode == kCCModeCCM) {
            ccm_nonce_ctx *ccm = cryptor->ctx[cryptor->op].ccm;
            memcpy(data, ccm->mac, ccm->mac_size);
            *dataSize = ccm->mac_size;
        } else return kCCUnimplemented;
        break;
    default:
        return kCCParamError;
    }
    return kCCSuccess;
}


