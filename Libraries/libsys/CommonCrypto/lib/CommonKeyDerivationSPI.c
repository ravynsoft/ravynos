//
//  CommonNistKeyDerivation.c
//  CommonCrypto
//
//  Created by Richard Murphy on 12/15/13.
//  Copyright (c) 2013 Platform Security. All rights reserved.
//

#include <stdlib.h>
#include <CommonCrypto/CommonKeyDerivationSPI.h>
#include <corecrypto/cchkdf.h>
#include <corecrypto/ccansikdf.h>
#include <corecrypto/ccnistkdf.h>
#include <corecrypto/ccpbkdf2.h>
#include "CommonDigestPriv.h"
#include "ccdebug.h"

typedef struct CCKDFParameters {
    CCKDFAlgorithm algorithm;

    union {
        struct {
            unsigned rounds;
            const void *salt;
            size_t saltLen;
        } pbkdf2;

        struct {
            const void *label;
            size_t labelLen;
            const void *context;
            size_t contextLen;
        } ctr_hmac;

        struct {
            const void *context;
            size_t contextLen;
        } ctr_hmac_fixed;

        struct {
            const void *salt;
            size_t saltLen;
            const void *context;
            size_t contextLen;
        } hkdf;

        struct {
            const void *sharedinfo;
            size_t sharedinfoLen;
        } ansi_x963;
    } u;
} CCKDFParameters;

CCStatus
CCKDFParametersCreatePbkdf2(CCKDFParametersRef *params,
                            unsigned rounds, const void *salt, size_t saltLen)
{
    if (rounds == 0) {
        return kCCParamError;
    }
    if (salt == NULL && saltLen > 0) {
        return kCCParamError;
    }

    CCKDFParametersRef p = malloc(sizeof(CCKDFParameters));
    if (p == NULL) {
        return kCCMemoryFailure;
    }

    p->algorithm = kCCKDFAlgorithmPBKDF2_HMAC;
    p->u.pbkdf2.rounds = rounds;
    p->u.pbkdf2.salt = salt;
    p->u.pbkdf2.saltLen = saltLen;

    *params = p;
    return kCCSuccess;
}

CCStatus
CCKDFParametersCreateCtrHmac(CCKDFParametersRef *params,
                             const void *label, size_t labelLen,
                             const void *context, size_t contextLen)
{
    if (label == NULL && labelLen > 0) {
        return kCCParamError;
    }
    if (context == NULL && contextLen > 0) {
        return kCCParamError;
    }

    CCKDFParametersRef p = malloc(sizeof(CCKDFParameters));
    if (p == NULL) {
        return kCCMemoryFailure;
    }

    p->algorithm = kCCKDFAlgorithmCTR_HMAC;
    p->u.ctr_hmac.label = label;
    p->u.ctr_hmac.labelLen = labelLen;
    p->u.ctr_hmac.context = context;
    p->u.ctr_hmac.contextLen = contextLen;

    *params = p;
    return kCCSuccess;
}

CCStatus
CCKDFParametersCreateCtrHmacFixed(CCKDFParametersRef *params,
                                  const void *context, size_t contextLen)
{
    if (context == NULL && contextLen > 0) {
        return kCCParamError;
    }

    CCKDFParametersRef p = malloc(sizeof(CCKDFParameters));
    if (p == NULL) {
        return kCCMemoryFailure;
    }

    p->algorithm = kCCKDFAlgorithmCTR_HMAC_FIXED;
    p->u.ctr_hmac_fixed.context = context;
    p->u.ctr_hmac_fixed.contextLen = contextLen;

    *params = p;
    return kCCSuccess;
}

CCStatus
CCKDFParametersCreateHkdf(CCKDFParametersRef *params,
                          const void *salt, size_t saltLen,
                          const void *context, size_t contextLen)
{
    if (salt == NULL && saltLen > 0) {
        return kCCParamError;
    }
    if (context == NULL && contextLen > 0) {
        return kCCParamError;
    }

    CCKDFParametersRef p = malloc(sizeof(CCKDFParameters));
    if (p == NULL) {
        return kCCMemoryFailure;
    }

    p->algorithm = kCCKDFAlgorithmHKDF;
    p->u.hkdf.salt = salt;
    p->u.hkdf.saltLen = saltLen;
    p->u.hkdf.context = context;
    p->u.hkdf.contextLen = contextLen;

    *params = p;
    return kCCSuccess;
}

CCStatus
CCKDFParametersCreateAnsiX963(CCKDFParametersRef *params,
                              const void *sharedinfo, size_t sharedinfoLen)
{
    if (sharedinfo == NULL && sharedinfoLen > 0) {
        return kCCParamError;
    }

    CCKDFParametersRef p = malloc(sizeof(CCKDFParameters));
    if (p == NULL) {
        return kCCMemoryFailure;
    }

    p->algorithm = kCCKDFAlgorithmAnsiX963;
    p->u.ansi_x963.sharedinfo = sharedinfo;
    p->u.ansi_x963.sharedinfoLen = sharedinfoLen;

    *params = p;
    return kCCSuccess;
}

void
CCKDFParametersDestroy(CCKDFParametersRef params)
{
    cc_clear(sizeof(CCKDFParameters), params);
    free(params);
}

CCStatus
CCKeyDerivationHMac(CCKDFAlgorithm algorithm, CCDigestAlgorithm digest,
                    unsigned rounds, // ignored except for PBKDF
                    const void *keyDerivationKey, size_t keyDerivationKeyLen,
                    const void *label, size_t labelLen,
                    const void *context, size_t contextLen, // or FIXED buffer (label | context)
                    const void * __unused iv, size_t __unused ivLen,           // for FB
                    const void *salt, size_t saltLen,       // for PBKDF or HKDF
                    void *derivedKey, size_t derivedKeyLen)
{
    CCStatus rv = kCCUnimplemented;
    CCKDFParametersRef params;

    switch (algorithm) {
        case kCCKDFAlgorithmPBKDF2_HMAC:
            rv = CCKDFParametersCreatePbkdf2(&params, rounds, salt, saltLen);
            break;
        case kCCKDFAlgorithmCTR_HMAC:
            rv = CCKDFParametersCreateCtrHmac(&params, label, labelLen, context, contextLen);
            break;
        case kCCKDFAlgorithmCTR_HMAC_FIXED:
            rv = CCKDFParametersCreateCtrHmacFixed(&params, context, contextLen);
            break;
        case kCCKDFAlgorithmHKDF:
            rv = CCKDFParametersCreateHkdf(&params, salt, saltLen, context, contextLen);
            break;
    }

    if (rv != kCCSuccess) {
        return rv;
    }

    rv = CCDeriveKey(params, digest, keyDerivationKey, keyDerivationKeyLen,
                     derivedKey, derivedKeyLen);

    CCKDFParametersDestroy(params);
    return rv;
}

CCStatus
CCDeriveKey(const CCKDFParametersRef params, CCDigestAlgorithm digest,
            const void *keyDerivationKey, size_t keyDerivationKeyLen,
            void *derivedKey, size_t derivedKeyLen)
{
    int rv = 0;
    const struct ccdigest_info *di = CCDigestGetDigestInfo(digest);

    if (di == NULL) {
        return kCCParamError;
    }

    if (derivedKey == NULL || derivedKeyLen == 0) {
        return kCCParamError;
    }
    if (keyDerivationKey == NULL && keyDerivationKeyLen > 0) {
        return kCCParamError;
    }

    // Allow empty IKM for HKDF.
    if (params->algorithm != kCCKDFAlgorithmHKDF) {
        if (keyDerivationKey == NULL || keyDerivationKeyLen == 0) {
            return kCCParamError;
        }
    }

    switch (params->algorithm) {
        case kCCKDFAlgorithmPBKDF2_HMAC: {
            rv = ccpbkdf2_hmac(di, keyDerivationKeyLen, keyDerivationKey,
                               params->u.pbkdf2.saltLen, params->u.pbkdf2.salt,
                               params->u.pbkdf2.rounds, derivedKeyLen, derivedKey);
            break;
        }
        case kCCKDFAlgorithmCTR_HMAC: {
            rv = ccnistkdf_ctr_hmac(di, keyDerivationKeyLen, keyDerivationKey,
                                    params->u.ctr_hmac.labelLen,
                                    params->u.ctr_hmac.label,
                                    params->u.ctr_hmac.contextLen,
                                    params->u.ctr_hmac.context,
                                    derivedKeyLen, derivedKey);
            break;
        }
        case kCCKDFAlgorithmCTR_HMAC_FIXED: {
            rv = ccnistkdf_ctr_hmac_fixed(di, keyDerivationKeyLen, keyDerivationKey,
                                          params->u.ctr_hmac_fixed.contextLen,
                                          params->u.ctr_hmac_fixed.context,
                                          derivedKeyLen, derivedKey);
            break;
        }
        case kCCKDFAlgorithmHKDF: {
            rv = cchkdf(di, keyDerivationKeyLen, keyDerivationKey,
                        params->u.hkdf.saltLen, params->u.hkdf.salt,
                        params->u.hkdf.contextLen, params->u.hkdf.context,
                        derivedKeyLen, derivedKey);
            break;
        }
        case kCCKDFAlgorithmAnsiX963: {
            rv = ccansikdf_x963(di, keyDerivationKeyLen, keyDerivationKey,
                                params->u.ansi_x963.sharedinfoLen,
                                params->u.ansi_x963.sharedinfo,
                                derivedKeyLen, derivedKey);
            break;
        }
        default: {
            return kCCUnimplemented;
        }
    }

    if (rv) {
        // Probably a bad derivedKeyLen.
        return kCCParamError;
    }

    return kCCSuccess;
}
