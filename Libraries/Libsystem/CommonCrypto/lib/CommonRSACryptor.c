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


#include <stdlib.h>

#include <CommonCrypto/CommonRSACryptor.h>
#include <CommonCrypto/CommonRSACryptorSPI.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>
#include "CommonDigestPriv.h"

#include <CommonCrypto/CommonRandomSPI.h>
#include <corecrypto/ccrsa.h>
#include <corecrypto/ccrsa_priv.h>
#include <corecrypto/ccasn1.h>
#include <corecrypto/ccn.h>

#include "ccErrors.h"
#include "ccdebug.h"
#include "cc_macros_priv.h"

#pragma mark internal

#define kCCMaximumRSAKeyBits 4096

static inline size_t
ccRSAFullContextSize(size_t bits)
{
    return cc_ctx_sizeof(struct ccrsa_full_ctx, ccrsa_full_ctx_size(ccn_sizeof(bits)));
}

typedef struct _CCRSACryptor {
    size_t key_nbits;
    CCRSAKeyType keyType;
    struct ccrsa_full_ctx fk[];
} CCRSACryptor;

static CCRSACryptor *
ccMallocRSACryptor(size_t nbits)
{
    if (nbits > kCCMaximumRSAKeyBits) {
        return NULL;
    }

    CCRSACryptor *retval;
    cc_size n = ccn_nof(nbits);

    if ((retval = malloc(sizeof(CCRSACryptor) + ccRSAFullContextSize(nbits))) == NULL) {
        return NULL;
    }

    retval->key_nbits = nbits;
    ccrsa_ctx_n(retval->fk) = n;
    return retval;
}

static void
ccRSACryptorClear(CCRSACryptorRef theKey)
{
    CCRSACryptor *key = (CCRSACryptor *) theKey;
    if(key==NULL) return;
    cc_clear(sizeof(CCRSACryptor) + ccRSAFullContextSize(key->key_nbits), key);
    free(key);
}

static inline size_t
ccRSAkeysize(CCRSACryptor *cryptor) {
    return ccn_bitlen(ccrsa_ctx_n(cryptor->fk), ccrsa_ctx_m(cryptor->fk));
}

#pragma mark APIDone

CCCryptorStatus 
CCRSACryptorGeneratePair(size_t keysize, uint32_t e, CCRSACryptorRef *publicKey, CCRSACryptorRef *privateKey)
{
    CC_DEBUG_LOG("Entering\n");
    CCCryptorStatus retval;
    CCRSACryptor *privateCryptor = NULL;
    CCRSACryptor *publicCryptor = NULL;
    struct ccrng_state *rng;
    
    
    // ccrsa_generate_key() requires the exponent as length / pointer to bytes
    cc_unit cc_unit_e = (cc_unit) e;
    
    size_t eSize = ccn_write_int_size(1, &cc_unit_e);
    uint8_t eBytes[eSize];
    ccn_write_int(1, &cc_unit_e, eSize, eBytes);
    
    *publicKey = *privateKey = NULL;
    __Require_Action(keysize<=kCCMaximumRSAKeyBits, errOut, retval = kCCParamError);

    rng=ccDRBGGetRngState();
    __Require_Action(rng!=NULL, errOut, retval=kCCRNGFailure);

    // Allocate memory for the private key
    __Require_Action((privateCryptor = ccMallocRSACryptor(keysize)) != NULL, errOut, retval = kCCMemoryFailure);

    // Generate a public / private key pair compliant with FIPS 186 standard
    // as long as the keysize is one specified by the standard and that |e|>=17bits.
    // Consistency check done in corecrypto.
    __Require_Action((ccrsa_generate_fips186_key(keysize, privateCryptor->fk, eSize, eBytes, rng, rng) == 0), errOut, retval = kCCDecodeError);
    
    privateCryptor->keyType = ccRSAKeyPrivate;
    __Require_Action((publicCryptor = CCRSACryptorCreatePublicKeyFromPrivateKey(privateCryptor)) != NULL, errOut, retval = kCCMemoryFailure);
    
    *publicKey = publicCryptor;
    *privateKey = privateCryptor;



    return kCCSuccess;
    
errOut:
    ccRSACryptorClear(privateCryptor);
    ccRSACryptorClear(publicCryptor);
    *publicKey = *privateKey = NULL;
    return retval;
}

CCRSACryptorRef CCRSACryptorCreatePublicKeyFromPrivateKey(CCRSACryptorRef privateCryptorRef)
{
    CCRSACryptor *publicCryptor = NULL, *privateCryptor = privateCryptorRef;
    
    CC_DEBUG_LOG("Entering\n");
    if((publicCryptor = ccMallocRSACryptor(privateCryptor->key_nbits)) == NULL)  return NULL;
    int ccrc = ccrsa_init_pub(ccrsa_ctx_public(publicCryptor->fk), ccrsa_ctx_m(privateCryptor->fk), ccrsa_ctx_e(privateCryptor->fk));
    publicCryptor->keyType = ccRSAKeyPublic;
    if (ccrc != CCERR_OK) {
        CCRSACryptorRelease(publicCryptor);
        return NULL;
    }
    return publicCryptor;
}

// Deprecated. Use CCRSACryptorCreatePublicKeyFromPrivateKey()
CCRSACryptorRef CCRSACryptorGetPublicKeyFromPrivateKey(CCRSACryptorRef privateCryptorRef)
{
    return CCRSACryptorCreatePublicKeyFromPrivateKey(privateCryptorRef);
}

CCRSAKeyType CCRSAGetKeyType(CCRSACryptorRef key)
{
    CCRSACryptor *cryptor = key;
    CCRSAKeyType retval;

    CC_DEBUG_LOG("Entering\n");
    if(key == NULL) return ccRSABadKey;
    retval = cryptor->keyType;
    if(retval != ccRSAKeyPublic && retval != ccRSAKeyPrivate) return ccRSABadKey;
    return retval;
}

int CCRSAGetKeySize(CCRSACryptorRef key)
{
    CCRSACryptor *cryptor = key;
    CC_DEBUG_LOG("Entering\n");
    if(key == NULL) return kCCParamError;    
    
    return (int) cryptor->key_nbits;
}

void 
CCRSACryptorRelease(CCRSACryptorRef key)
{
    CC_DEBUG_LOG("Entering\n");
    ccRSACryptorClear(key);
}

CCCryptorStatus CCRSACryptorImport(const void *keyPackage, size_t keyPackageLen, CCRSACryptorRef *key)
{
    CCRSACryptor *cryptor = NULL;
    CCCryptorStatus retval;
    CCRSAKeyType keyToMake;
    cc_size keyN;

    CC_DEBUG_LOG("Entering\n");
    if(!keyPackage || !key) return kCCParamError;
    if((keyN = ccrsa_import_priv_n(keyPackageLen, keyPackage)) != 0) keyToMake = ccRSAKeyPrivate;
    else if((keyN = ccrsa_import_pub_n(keyPackageLen, keyPackage)) != 0) keyToMake = ccRSAKeyPublic;
    else return kCCDecodeError;

    __Require_Action((cryptor = ccMallocRSACryptor(ccn_bitsof_n(keyN))) != NULL, errOut, retval = kCCMemoryFailure);

    switch(keyToMake) {
        case ccRSAKeyPublic:
            ccrsa_ctx_n(ccrsa_ctx_public(cryptor->fk)) = keyN;
            if(ccrsa_import_pub(ccrsa_ctx_public(cryptor->fk), keyPackageLen, keyPackage)) {
                ccRSACryptorClear(cryptor);
                return kCCDecodeError;
            }
            break;
        case ccRSAKeyPrivate:
            ccrsa_ctx_n(cryptor->fk) = keyN;
            if(ccrsa_import_priv(cryptor->fk, keyPackageLen, keyPackage)) {
                ccRSACryptorClear(cryptor);
                return kCCDecodeError;
            }
            break;
    }
    cryptor->keyType = keyToMake;
    *key = cryptor;
    cryptor->key_nbits = ccRSAkeysize(cryptor);

    return kCCSuccess;

errOut:
    ccRSACryptorClear(cryptor);
    *key = NULL;
    return retval;
}


CCCryptorStatus CCRSACryptorExport(CCRSACryptorRef cryptor, void *out, size_t *outLen)
{
    CCCryptorStatus retval = kCCSuccess;
    size_t bufsiz;
    
    CC_DEBUG_LOG("Entering\n");
    if(!cryptor || !out) return kCCParamError;
    switch(cryptor->keyType) {
        case ccRSAKeyPublic:
            bufsiz = ccrsa_export_pub_size(ccrsa_ctx_public(cryptor->fk));
            if(*outLen < bufsiz) {
                *outLen = bufsiz;
                return kCCBufferTooSmall;
            }
            *outLen = bufsiz;
            if(ccrsa_export_pub(ccrsa_ctx_public(cryptor->fk), bufsiz, out))
                return kCCDecodeError;
            break;
        case ccRSAKeyPrivate:
            bufsiz = ccrsa_export_priv_size(cryptor->fk);
            if(*outLen < bufsiz) {
                *outLen = bufsiz;
                return kCCBufferTooSmall;
            }
            *outLen = bufsiz;
            if(ccrsa_export_priv(cryptor->fk, bufsiz, out))
                return kCCDecodeError;
            break;
        default:
            retval = kCCParamError;
    }
    return retval;
}


CCCryptorStatus 
CCRSACryptorEncrypt(CCRSACryptorRef publicKey, CCAsymmetricPadding padding, const void *plainText, size_t plainTextLen, void *cipherText, size_t *cipherTextLen,
	const void *tagData, size_t tagDataLen, CCDigestAlgorithm digestType)
{
    CCCryptorStatus retval = kCCSuccess;

    CC_DEBUG_LOG("Entering\n");
    if(!publicKey || !cipherText || !plainText || !cipherTextLen) return kCCParamError;
    
    switch(padding) {
        case ccPKCS1Padding:
            if(ccrsa_encrypt_eme_pkcs1v15(ccrsa_ctx_public(publicKey->fk), ccDRBGGetRngState(), cipherTextLen, cipherText, plainTextLen, plainText)  != 0)
                retval =  kCCDecodeError;
            break;
        case ccOAEPPadding:         
            if(ccrsa_encrypt_oaep(ccrsa_ctx_public(publicKey->fk), CCDigestGetDigestInfo(digestType), ccDRBGGetRngState(), cipherTextLen, cipherText, plainTextLen, plainText, tagDataLen, tagData) != 0)
                retval =  kCCDecodeError;
            break;
        default:
            retval = kCCParamError;
            break;
    }
        
    return retval;
}



CCCryptorStatus 
CCRSACryptorDecrypt(CCRSACryptorRef privateKey, CCAsymmetricPadding padding, const void *cipherText, size_t cipherTextLen,
				 void *plainText, size_t *plainTextLen, const void *tagData, size_t tagDataLen, CCDigestAlgorithm digestType)
{
    CCCryptorStatus retval = kCCSuccess;
    
    CC_DEBUG_LOG("Entering\n");
    if(!privateKey || !cipherText || !plainText || !plainTextLen) return kCCParamError;
    
    switch (padding) {
        case ccPKCS1Padding:
            if(ccrsa_decrypt_eme_pkcs1v15(privateKey->fk, plainTextLen, plainText, cipherTextLen, (uint8_t *) cipherText) != 0)
                retval =  kCCDecodeError;
            break;
        case ccOAEPPadding:
            if(ccrsa_decrypt_oaep(privateKey->fk, CCDigestGetDigestInfo(digestType), plainTextLen, plainText, cipherTextLen, (uint8_t *) cipherText,
                                  tagDataLen, tagData) != 0) 
                retval =  kCCDecodeError;
            break;
        default:
            goto errOut;
    }
    
errOut:
    
    return retval;
}

CCCryptorStatus 
CCRSACryptorCrypt(CCRSACryptorRef rsaKey, const void *in, size_t inLen, void *out, size_t *outLen)
{    
    CC_DEBUG_LOG("Entering\n");
    if(!rsaKey || !in || !out || !outLen) return kCCParamError;
    
    size_t keysizeBytes = (rsaKey->key_nbits+7)/8;
    
    if(inLen != keysizeBytes || *outLen < keysizeBytes) return kCCMemoryFailure;
    
    cc_size n = ccrsa_ctx_n(rsaKey->fk);
    cc_unit buf[n];
    ccn_read_uint(n, buf, inLen, in);
    
    int rc;
    switch(rsaKey->keyType) {
        case ccRSAKeyPublic: 
            rc = ccrsa_pub_crypt(ccrsa_ctx_public(rsaKey->fk), buf, buf);
            break;
        case ccRSAKeyPrivate:
            rc = ccrsa_priv_crypt(rsaKey->fk, buf, buf);
            break;
        default:
            rc=-1;
    }
    
    if(rc==0){
        *outLen = keysizeBytes;
        ccn_write_uint_padded(n, buf, *outLen, out);
        return kCCSuccess;
    } else
        return kCCParamError;
    
}


static inline
CCCryptorStatus ccn_write_arg(size_t n, const cc_unit *source, uint8_t *dest, size_t *destLen)
{
    size_t len;
    if((len = ccn_write_uint_size(n, source)) > *destLen) {
        return kCCMemoryFailure;
    }
    *destLen = len;
    ccn_write_uint(n, source, *destLen, dest);
    return kCCSuccess;
}


static CCRSACryptor *malloc_rsa_cryptor(size_t modulus_nbytes)
{
    if(modulus_nbytes==0)
        return NULL;
    
    size_t nbits = ccn_bitsof_size(modulus_nbytes);
    CCRSACryptor * rsaKey= ccMallocRSACryptor(nbits);
    return rsaKey;
}

#define expect(cond)  __Require_Action((cond) , errOut, retval = kCCParamError)
static CCCryptorStatus
create_pub(const uint8_t *modulus, size_t modulusLength,
           const uint8_t *publicExponent, size_t publicExponentLength,
           CCRSACryptorRef *ref)
{
    CC_DEBUG_LOG("Entering\n");
    if( modulus==NULL || publicExponent==NULL || modulusLength==0 || publicExponentLength ==0 || ref==NULL)
        return kCCParamError;
    
    CCCryptorStatus retval;
    CCRSACryptor *rsaKey = NULL;
    
    size_t n = ccn_nof_size(modulusLength);
    expect(n!=0);
    
    __Require_Action((rsaKey = malloc_rsa_cryptor(modulusLength)) != NULL, errOut, retval = kCCMemoryFailure);
    ccrsa_full_ctx_t fk = rsaKey->fk;
   
    CCZP_N(ccrsa_ctx_zm(fk)) = n;
    int rc = ccrsa_make_pub(ccrsa_ctx_public(fk), publicExponentLength, publicExponent, modulusLength, modulus);  expect(rc==0);
    
    rsaKey->key_nbits = ccn_bitlen(n, ccrsa_ctx_m(fk));
    rsaKey->keyType = ccRSAKeyPublic;
    
    *ref = rsaKey;
    return kCCSuccess;
    
errOut:
    ccRSACryptorClear(rsaKey);
    return retval;
}

static CCCryptorStatus
create_priv(const uint8_t *publicExponent, size_t publicExponentLength, const uint8_t *p, size_t pLength, const uint8_t *q, size_t qLength, CCRSACryptorRef *ref)
{
    //ccrsa_ctx_e
    CC_DEBUG_LOG("Entering\n");
    CCCryptorStatus retval;
    CCRSACryptor *rsaKey = NULL;
    int rc = CCERR_OK;

    expect(publicExponent!=NULL && publicExponentLength!=0 && p!=NULL && pLength!=0 && q!=NULL && qLength!=0 && ref!=NULL);

    size_t modulusLength = pLength+qLength;
    size_t n = ccn_nof_size(modulusLength);
    expect(n!=0);

    __Require_Action((rsaKey = malloc_rsa_cryptor(modulusLength)) != NULL, errOut, retval = kCCMemoryFailure);
    ccrsa_full_ctx_t fk = rsaKey->fk;

    CCZP_N(ccrsa_ctx_zm(fk)) = n;

    rc = ccrsa_make_priv(fk, publicExponentLength, publicExponent, pLength, p, qLength, q);
    expect(rc==0);

    rsaKey->key_nbits = ccn_bitlen(n, ccrsa_ctx_m(rsaKey->fk));
    rsaKey->keyType = ccRSAKeyPrivate;

	*ref = rsaKey;
	return kCCSuccess;
	
errOut:
	ccRSACryptorClear(rsaKey);
	return retval;
}

CCCryptorStatus
CCRSACryptorCreateFromData( CCRSAKeyType keyType, const uint8_t *modulus, size_t modulusLength,
                           const uint8_t *publicExponent, size_t publicExponentLength,
                           const uint8_t *p, size_t pLength, const uint8_t *q, size_t qLength,
                           CCRSACryptorRef *ref)
{
    CC_DEBUG_LOG("Entering\n");
    CCCryptorStatus rv;
    if(keyType==ccRSAKeyPublic)
        rv = create_pub(modulus, modulusLength, publicExponent, publicExponentLength, ref);
    else if(keyType==ccRSAKeyPrivate)
        rv = create_priv(publicExponent, publicExponentLength, p, pLength, q, qLength, ref);
    else
        rv = kCCParamError;

    return rv;
 }


CCCryptorStatus
CCRSAGetKeyComponents(CCRSACryptorRef rsaKey, uint8_t *modulus, size_t *modulusLength, uint8_t *exponent, size_t *exponentLength,
                      uint8_t *p, size_t *pLength, uint8_t *q, size_t *qLength)
{
    CCRSACryptor *rsa = rsaKey;
    int rc;
    
    CC_DEBUG_LOG("Entering\n");
    switch(rsa->keyType) {
		case ccRSAKeyPublic:
            rc = ccrsa_get_pubkey_components(ccrsa_ctx_public(rsaKey->fk), modulus, modulusLength, exponent, exponentLength); //returns encryption exponent
            break;
            
		case ccRSAKeyPrivate:
            rc = ccrsa_get_fullkey_components(rsaKey->fk, modulus, modulusLength, exponent, exponentLength, p, pLength, q, qLength); //returns decryption exponent
            break;

		default:
			rc=-1;
    }
    return rc==0 ? kCCSuccess : kCCParamError;
}

#define DP_N(rsa)   cczp_n(ccrsa_ctx_private_zp(rsa->fk))
#define DQ_N(rsa)   cczp_n(ccrsa_ctx_private_zq(rsa->fk))
#define QINV_N(rsa) cczp_n(ccrsa_ctx_private_zp(rsa->fk))

#define DP(rsa)     ccrsa_ctx_private_dp(rsa->fk)
#define DQ(rsa)     ccrsa_ctx_private_dq(rsa->fk)
#define QINV(rsa)   ccrsa_ctx_private_qinv(rsa->fk)
//CRT functions assume that dp, dq and qinv are initilized in the ccrsa_full_ctx structure.
//Functions that create a CCRSACryptorRef object, must make sure dp, dq and qinv are initilized.

#define PP_N(rsa)   ccn_n(cczp_n(ccrsa_ctx_private_zp(rsa->fk)), PP(rsa))
#define QQ_N(rsa)   ccn_n(cczp_n(ccrsa_ctx_private_zq(rsa->fk)), QQ(rsa))
#define PP(rsa)     cczp_prime(ccrsa_ctx_private_zp(rsa->fk))
#define QQ(rsa)     cczp_prime(ccrsa_ctx_private_zq(rsa->fk))


CCCryptorStatus
CCRSAGetCRTComponentsSizes(CCRSACryptorRef rsaKey, size_t *dpSize, size_t *dqSize, size_t *qinvSize)
{
    CCRSACryptor *rsa = rsaKey;
    CCCryptorStatus rc=kCCParamError;
    
    CC_DEBUG_LOG("Entering\n");
    if(rsa->keyType==ccRSAKeyPrivate){
        *dpSize = ccn_write_uint_size(DP_N(rsaKey), DP(rsaKey));
        *dqSize = ccn_write_uint_size(DQ_N(rsaKey), DQ(rsaKey));
        *qinvSize = ccn_write_uint_size(QINV_N(rsaKey), QINV(rsaKey));
        rc = kCCSuccess;
    }
    
    return rc;
}

CCCryptorStatus
CCRSAGetCRTComponents(CCRSACryptorRef rsaKey, void *dp, size_t dpSize, void *dq, size_t dqSize, void *qinv, size_t qinvSize)
{
    CCRSACryptor *rsa = rsaKey;
    CCCryptorStatus rc=kCCParamError;
    size_t _dpSize, _dqSize, _qinvSize;
    
    CC_DEBUG_LOG("Entering\n");
    if(rsa->keyType==ccRSAKeyPrivate)
    {
        __Require(CCRSAGetCRTComponentsSizes(rsa, &_dpSize, &_dqSize, &_qinvSize)==kCCSuccess, out);
        __Require(ccn_cmpn(PP_N(rsa), PP(rsa), QQ_N(rsa), QQ(rsa))>0, out);
        if(dpSize>=_dpSize && dqSize>=_dqSize && qinvSize>=_qinvSize)
        {
            ccn_write_uint(DP_N(rsa), DP(rsa), _dpSize, dp);
            ccn_write_uint(DQ_N(rsa), DQ(rsa), _dqSize, dq);
            ccn_write_uint(QINV_N(rsa), QINV(rsa), _qinvSize, qinv);
            rc = kCCSuccess;
        }
    }
    
out:
    return rc;
}

static const struct ccdigest_info *validate_sign_verify_params(CCRSACryptorRef privateKey, CCAsymmetricPadding padding, const void *hash,
                           CCDigestAlgorithm digestType, const void *data, size_t *DataLen)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    
    if(digestType!=kCCDigestSHA1 && digestType!=kCCDigestSHA224 && digestType!=kCCDigestSHA256 && digestType!= kCCDigestSHA384 && digestType!=kCCDigestSHA512)
        return NULL;
 #pragma clang diagnostic pop
    
    const struct ccdigest_info *di = CCDigestGetDigestInfo(digestType);

    if(privateKey==NULL || hash==NULL || data==NULL || DataLen==NULL || di==NULL)
        return NULL;
    
    if(padding!=ccPKCS1Padding && padding!=ccRSAPSSPadding)
        return NULL;
    
    return di;
}

CCCryptorStatus 
CCRSACryptorSign(CCRSACryptorRef privateKey, CCAsymmetricPadding padding, 
                 const void *hashToSign, size_t hashSignLen,
                 CCDigestAlgorithm digestType, size_t saltLen,
                 void *signedData, size_t *signedDataLen)
{    
    CC_DEBUG_LOG("Entering\n");
    const struct ccdigest_info *di = validate_sign_verify_params(privateKey, padding, hashToSign, digestType, signedData, signedData);
    if(di==NULL) return kCCParamError;

    int rc;
    if(padding==ccPKCS1Padding)
        rc=ccrsa_sign_pkcs1v15(privateKey->fk, di->oid, hashSignLen, hashToSign, signedDataLen, signedData);
    else
        rc=ccrsa_sign_pss(privateKey->fk, di, di, saltLen, ccDRBGGetRngState(), hashSignLen, hashToSign, signedDataLen, signedData);
    
    return rc==0? kCCSuccess:kCCDecodeError;
}

CCCryptorStatus
CCRSACryptorVerify(CCRSACryptorRef publicKey, CCAsymmetricPadding padding,
                   const void *hash, size_t hashLen, 
                   CCDigestAlgorithm digestType, size_t saltLen,
                   const void *signedData, size_t signedDataLen)
{
    CC_DEBUG_LOG("Entering\n");
    const struct ccdigest_info *di = validate_sign_verify_params(publicKey, padding, hash, digestType, signedData, &signedDataLen);
    if(di==NULL) return kCCParamError;
    
    bool valid = false;
    ccrsa_pub_ctx_t fk = ccrsa_ctx_public(publicKey->fk);
    int rc;
    
    if(padding==ccPKCS1Padding)
        rc=ccrsa_verify_pkcs1v15(fk, di->oid, hashLen, hash, signedDataLen, signedData, &valid);
    else
        rc=ccrsa_verify_pss(fk, di, di, hashLen, hash, signedDataLen, signedData, saltLen, &valid);
    
    CCCryptorStatus rv;
    if(!valid || rc!=0 )
        rv = kCCDecodeError;
    else
        rv = kCCSuccess;
    
    return rv;
}

