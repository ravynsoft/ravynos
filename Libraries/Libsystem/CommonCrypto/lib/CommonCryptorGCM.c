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

#include "ccdebug.h"
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonCryptorSPI.h>
#include "CommonCryptorPriv.h"
#include <corecrypto/ccn.h>
#include "CommonCryptorPriv.h"

/*
 typical GCM use case: sending an authenticated packet

 +--------------+-------+--------------+
 |    header    |  seq. |   Data       |
 +--------------+-------+--------------+
        |           |         |
        |           |         |
 Addtl auth data   IV     plain text
        |           |         |
        |           |         V
        |           |     +--------------------+
        |           +---->|                    |
        |           |     | GCM encryption     |
        +---------------->|                    |
        |           |     +--------------------+
        |           |         |            |
        |           |     cipher text    Auth tag
        |           |         |            |
        V           V         V            V
 +--------------+-------+----------------+---------+
 |    header    |  seq. | encrypted data |   ICV   |
 +--------------+-------+----------------+---------+
 */

#define decl_cryptor()  CCCryptor *cryptor = getRealCryptor(cryptorRef, 0); \
                        CC_DEBUG_LOG("Entering\n"); \
                        if(!cryptor) return kCCParamError;

static inline CCCryptorStatus translate_err_code(int err)
{
    switch (err) {
        case CCERR_OK:
            return kCCSuccess;
        case CCERR_PARAMETER:
            return kCCParamError;
        default:
            return kCCUnspecifiedError;
    }
}

//Deprecated. Use CCCryptorGCMSetIV()
CCCryptorStatus
CCCryptorGCMAddIV(CCCryptorRef cryptorRef,
                  const void 		*iv,
                  size_t ivLen)
{
    decl_cryptor();
    if(ivLen!=0 && iv==NULL) return kCCParamError;
    //it is okay to call with ivLen 0 and/OR iv==NULL
    //infact this needs to be done even with NULL values, otherwise ccgcm_ is going to return call sequence error.
    //currently corecrypto accepts NULL
    //rdar://problem/23523093
    int rc = ccgcm_set_iv_legacy(cryptor->symMode[cryptor->op].gcm,cryptor->ctx[cryptor->op].gcm, ivLen, iv);
    return translate_err_code(rc);
}

//replaces CCCryptorGCMAddIV, to prevent NULL IV
CCCryptorStatus
CCCryptorGCMSetIV(CCCryptorRef cryptorRef,
                  const void 		*iv,
                  size_t ivLen)
{
    decl_cryptor();
    if(ivLen<AESGCM_MIN_IV_LEN || iv==NULL) return kCCParamError;

    int rc = ccgcm_set_iv(cryptor->symMode[cryptor->op].gcm,cryptor->ctx[cryptor->op].gcm, ivLen, iv);
    return translate_err_code(rc);
}

//Add additional authentication data
CCCryptorStatus
CCCryptorGCMAddAAD(CCCryptorRef cryptorRef,
                   const void  *aData,
                   size_t aDataLen)
{
    decl_cryptor();
    if(aDataLen!=0 && aData==NULL) return kCCParamError;
    //it is okay to call with aData zero
    int rc = ccgcm_aad(cryptor->symMode[cryptor->op].gcm,cryptor->ctx[cryptor->op].gcm, aDataLen, aData);
    return translate_err_code(rc);
}

// This is for old iOS5 clients
CCCryptorStatus
CCCryptorGCMAddADD(CCCryptorRef cryptorRef,
                   const void 		*aData,
                   size_t aDataLen)
{
    return  CCCryptorGCMAddAAD(cryptorRef, aData, aDataLen);
}

// This was a temp mistake in MacOSX8
CCCryptorStatus
CCCryptorGCMaddAAD(CCCryptorRef cryptorRef,
                   const void 		*aData,
                   size_t aDataLen)
{
    return CCCryptorGCMAddAAD(cryptorRef, aData, aDataLen);
}

//we are not providing this function to users.
//The reason is that we don't want to create more liability for ourself
//and a new interface function just increases the number of APIs
//without actually helping users
//User's use the old CCCryptorGCMEncrypt() and CCCryptorGCMDecrypt()
static CCCryptorStatus gcm_update(CCCryptorRef cryptorRef,
                                  const void *dataIn, size_t dataInLength,
                                  void *dataOut)
{
    decl_cryptor();
    if(dataInLength!=0 && dataIn==NULL) return kCCParamError;
    //no data is okay
    if(dataOut == NULL) return kCCParamError;
    int rc = ccgcm_update(cryptor->symMode[cryptor->op].gcm,cryptor->ctx[cryptor->op].gcm, dataInLength, dataIn, dataOut);
    return translate_err_code(rc);
}


CCCryptorStatus CCCryptorGCMEncrypt(CCCryptorRef cryptorRef,
                                    const void *dataIn, size_t dataInLength,
                                    void *dataOut)
{
    return gcm_update(cryptorRef, dataIn, dataInLength, dataOut);
}



CCCryptorStatus CCCryptorGCMDecrypt(CCCryptorRef cryptorRef,
                                    const void *dataIn, size_t dataInLength,
                                    void *dataOut)
{
    return gcm_update(cryptorRef, dataIn, dataInLength, dataOut);
}

//Deprecated. Use CCCryptorGCMFinalize()
CCCryptorStatus CCCryptorGCMFinal(CCCryptorRef cryptorRef,
                                  void *tagOut, size_t *tagLength)
{
    decl_cryptor();
    if(tagOut == NULL || tagLength == NULL)  return kCCParamError;
    int rc = ccgcm_finalize(cryptor->symMode[cryptor->op].gcm,cryptor->ctx[cryptor->op].gcm, *tagLength, (void *) tagOut);
    if(rc == -1)
        return kCCUnspecifiedError;
    else
        return kCCSuccess; //this includes 0 and any error message other than -1

   // ccgcm_finalize() returns CCMODE_INTEGRITY_FAILURE (-3) if the expected tag is not coppied to the buffer. but that doesn't mean there is an error
}

//replaces CCCryptorGCMFinal()
CCCryptorStatus CCCryptorGCMFinalize(CCCryptorRef cryptorRef,
                                     void *tag, size_t tagLength)
{
    decl_cryptor();
    if(tag==NULL  || tagLength<AESGCM_MIN_TAG_LEN || tagLength>AESGCM_BLOCK_LEN)  return kCCParamError;
    if(cryptorRef->op!=kCCEncrypt && cryptorRef->op!=kCCDecrypt) return kCCParamError;

    //for decryption only
    char dec_tag[tagLength];

    //if decrypting, write the computed tag in an internal buffer
    if(cryptorRef->op == kCCDecrypt){
        memcpy(dec_tag, tag, sizeof(dec_tag));
        tag = dec_tag;
    }
    
    int rc = ccgcm_finalize(cryptor->symMode[cryptor->op].gcm,cryptor->ctx[cryptor->op].gcm, tagLength, tag);
    CCCryptorStatus rv = rc == 0 ? kCCSuccess : kCCUnspecifiedError;

    if(cryptorRef->op == kCCDecrypt)
        cc_clear(sizeof dec_tag, dec_tag);
    
    return rv;
}


CCCryptorStatus CCCryptorGCMReset(CCCryptorRef cryptorRef)
{
    decl_cryptor();
    int rc = ccgcm_reset(cryptor->symMode[cryptor->op].gcm,cryptor->ctx[cryptor->op].gcm);
    return translate_err_code(rc);
}


//Deprecated because decryption should not return the tag and IV cannot be zero/NULL.
//Use CCCryptorGCMOneshotEncrypt() or CCCryptorGCMOneshotDecrypt() instead.
CCCryptorStatus CCCryptorGCM(CCOperation op,				/* kCCEncrypt, kCCDecrypt */
                             CCAlgorithm alg,
                             const void  *key,    size_t keyLength, /* raw key material */
                             const void  *iv,     size_t ivLen,
                             const void  *aData,  size_t aDataLen,
                             const void  *dataIn, size_t dataInLength,
                             void 		 *dataOut,
                             void        *tagOut,    size_t *tagLength)

{
    CCCryptorRef cryptorRef;
    CCCryptorStatus retval;
    
    CC_DEBUG_LOG("Entering Op: %d Cipher: %d\n", op, alg);
    
    retval = CCCryptorCreateWithMode(op, kCCModeGCM, alg, 0, NULL, key, keyLength,
                                     NULL, 0, 0, 0, &cryptorRef);
    if(retval) return retval;
    
    //call even with NULL pointer and zero length IV
    retval = CCCryptorGCMAddIV(cryptorRef, iv, ivLen);
    if(retval) return retval;

    retval = CCCryptorGCMaddAAD(cryptorRef, aData, aDataLen);
    if(retval) return retval;

    retval = gcm_update(cryptorRef, dataIn, dataInLength, dataOut);
    if(retval) return retval;

    retval = CCCryptorGCMFinal(cryptorRef, tagOut, tagLength);
    CCCryptorRelease(cryptorRef);

    return retval;
}

static CCCryptorStatus validate_gcm_params(CCAlgorithm alg, CCOperation op, const void *key,    size_t keyLength,
                                           const void  *iv,     size_t ivLen,
                                           const void  *aData,  size_t aDataLen,
                                           const void  *dataIn, size_t dataInLength,
                                           void 	   *dataOut,
                                           const void  *tag,    size_t tagLength){
    
    (void)aData; (void)aDataLen;
    (void)dataIn; (void)dataInLength;
    (void) op;
    
    if(alg!=kCCAlgorithmAES)
        return kCCParamError;

    if (keyLength != kCCKeySizeAES128 && keyLength != kCCKeySizeAES192 && keyLength != kCCKeySizeAES256) {
        return kCCKeySizeError;
    }
    
    if(tagLength<AESGCM_MIN_TAG_LEN || tagLength>AESGCM_BLOCK_LEN)
        return kCCParamError;
    
    if(keyLength<AESGCM_BLOCK_LEN || ivLen<AESGCM_MIN_IV_LEN)
        return kCCParamError;
    
    if(key==NULL || iv==NULL || tag==NULL || dataOut==NULL)
        return kCCParamError;
    
    return kCCSuccess;
}

//replaces CCCryptorGCM()
CCCryptorStatus CCCryptorGCMOneshotEncrypt(CCAlgorithm alg, const void  *key, size_t keyLength,
                                           const void  *iv,      size_t ivLen,
                                           const void  *aData,   size_t aDataLen,
                                           const void  *dataIn,  size_t dataInLength,
                                           void 	   *dataOut,
                                           void        *tagOut,  size_t tagLength)

{
    CCCryptorStatus rv = validate_gcm_params(alg, kCCEncrypt, key, keyLength, iv, ivLen, aData, aDataLen, dataIn, dataInLength, dataOut, tagOut, tagLength);
    if(rv!=kCCSuccess)
        return rv;
    
    int rc = ccgcm_one_shot(ccaes_gcm_encrypt_mode(), keyLength, key, ivLen, iv, aDataLen, aData, dataInLength, dataIn, dataOut, tagLength, tagOut);

    return translate_err_code(rc);
}

//replaces CCCryptorGCM()
CCCryptorStatus CCCryptorGCMOneshotDecrypt(CCAlgorithm alg, const void  *key, size_t keyLength,
                                           const void  *iv,      size_t ivLen,
                                           const void  *aData,   size_t aDataLen,
                                           const void  *dataIn,  size_t dataInLength,
                                           void 	   *dataOut,
                                           const void  *tagIn,   size_t tagLength)

{
    CCCryptorStatus rv = validate_gcm_params(alg, kCCDecrypt, key, keyLength, iv, ivLen, aData, aDataLen, dataIn, dataInLength, dataOut, tagIn, tagLength);
    if(rv!=kCCSuccess)
        return rv;
    
    char tag[tagLength]; //we are sure tagLength is not very large
    memcpy(tag, tagIn, sizeof(tag));

    int rc = ccgcm_one_shot(ccaes_gcm_decrypt_mode(), keyLength, key, ivLen, iv, aDataLen, aData, dataInLength, dataIn, dataOut, tagLength, tag);

    if (rc != CCERR_OK) {
        cc_clear(dataInLength, dataOut);
    }

    cc_clear(sizeof tag, tag);
    return translate_err_code(rc);
}
