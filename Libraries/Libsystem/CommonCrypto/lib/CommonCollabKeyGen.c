//
//  CommonCollabKeyGen.c
//  CommonCrypto
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#include <CommonCrypto/CommonCryptoErrorSPI.h>
#include <CommonCrypto/CommonCollabKeyGen.h>
#include <CommonCrypto/CommonRandomSPI.h>
#include "CommonDigestPriv.h"

#include <corecrypto/ccckg.h>

#include <stdlib.h>

typedef struct _CCCollabKeyGenContributor {
    ccckg_ctx_t ctx;
} CCCollabKeyGenContributor;

typedef struct _CCCollabKeyGenOwner {
    ccckg_ctx_t ctx;
} CCCollabKeyGenOwner;

#define CheckCurveAndDigestParams(_nbits_, _alg_)                  \
    if (!ccec_keysize_is_supported(_nbits_)) {                     \
        return kCCParamError;                                      \
    }                                                              \
    ccec_const_cp_t cp = ccec_get_cp(_nbits_);                     \
    const struct ccdigest_info *di = CCDigestGetDigestInfo(_alg_); \
    if (di == NULL) {                                              \
        return kCCParamError;                                      \
    }

static CCCryptorStatus
CCCKGMapReturnCode(int rv)
{
    switch (rv) {
        case CCERR_OK:
            return kCCSuccess;
        case CCERR_PARAMETER:
            return kCCParamError;
        case CCERR_INTEGRITY:
            return kCCDecodeError;
        case CCERR_CALL_SEQUENCE:
            return kCCCallSequenceError;
    }

    return kCCUnspecifiedError;
}

int
CCCKGGetCommitmentSize(size_t nbits, CCDigestAlgorithm alg)
{
    CheckCurveAndDigestParams(nbits, alg);
    return (int)ccckg_sizeof_commitment(cp, di);
}

int
CCCKGGetShareSize(size_t nbits, CCDigestAlgorithm alg)
{
    CheckCurveAndDigestParams(nbits, alg);
    return (int)ccckg_sizeof_share(cp, di);
}

int
CCCKGGetOpeningSize(size_t nbits, CCDigestAlgorithm alg)
{
    CheckCurveAndDigestParams(nbits, alg);
    return (int)ccckg_sizeof_opening(cp, di);
}

static CCCryptorStatus
CCCKGContextCreate(size_t nbits, CCDigestAlgorithm alg, ccckg_ctx_t *ctx)
{
    CheckCurveAndDigestParams(nbits, alg);

    *ctx = malloc(ccckg_sizeof_ctx(cp, di));
    if (*ctx == NULL) {
        return kCCMemoryFailure;
    }

    struct ccrng_state *rng = ccDRBGGetRngState();
    ccckg_init(*ctx, cp, di, rng);

    return kCCSuccess;
}

CCCryptorStatus
CCCKGContributorCreate(size_t nbits, CCDigestAlgorithm alg, CCCollabKeyGenContributorRef *contrib)
{
    if (contrib == NULL) {
        return kCCParamError;
    }

    CCCollabKeyGenContributor *ctx = malloc(sizeof(CCCollabKeyGenContributor));
    if (ctx == NULL) {
        return kCCMemoryFailure;
    }

    CCCryptorStatus rv = CCCKGContextCreate(nbits, alg, &ctx->ctx);
    if (rv == kCCSuccess) {
        *contrib = ctx;
    } else {
        free(ctx);
    }

    return rv;
}

CCCryptorStatus
CCCKGOwnerCreate(size_t nbits, CCDigestAlgorithm alg, CCCollabKeyGenOwnerRef *owner)
{
    if (owner == NULL) {
        return kCCParamError;
    }

    CCCollabKeyGenOwner *ctx = malloc(sizeof(CCCollabKeyGenOwner));
    if (ctx == NULL) {
        return kCCMemoryFailure;
    }

    CCCryptorStatus rv = CCCKGContextCreate(nbits, alg, &ctx->ctx);
    if (rv == kCCSuccess) {
        *owner = ctx;
    } else {
        free(ctx);
    }

    return rv;
}

void
CCCKGContributorDestroy(CCCollabKeyGenContributorRef contrib)
{
    free(contrib->ctx);
    free(contrib);
}

void
CCCKGOwnerDestroy(CCCollabKeyGenOwnerRef owner)
{
    free(owner->ctx);
    free(owner);
}

CCCryptorStatus
CCCKGContributorCommit(CCCollabKeyGenContributorRef contrib, void *commitment, size_t commitmentLen)
{
    if (contrib == NULL || commitment == NULL) {
        return kCCParamError;
    }

    return CCCKGMapReturnCode(ccckg_contributor_commit(contrib->ctx, commitmentLen, commitment));
}

CCCryptorStatus
CCCKGOwnerGenerateShare(CCCollabKeyGenOwnerRef owner,
                        const void *commitment, size_t commitmentLen,
                        void *share, size_t shareLen)
{
    if (owner == NULL || commitment == NULL || share == NULL) {
        return kCCParamError;
    }

    return CCCKGMapReturnCode(ccckg_owner_generate_share(owner->ctx, commitmentLen, commitment, shareLen, share));
}

static CCCryptorStatus
CCCKGConvertNativeToECCryptor(ccckg_ctx_t context, ccec_full_ctx_t P, CCECKeyType keyType, CCECCryptorRef *key)
{
    bool fullkey = (keyType == ccECKeyPrivate);
    uint8_t key_data[ccec_x963_export_size_cp(fullkey, context->cp)];

    // Export the raw key.
    ccec_x963_export(fullkey, key_data, P);

    // Import into a CCECCryptor.
    CCCryptorStatus rv = CCECCryptorImportKey(kCCImportKeyBinary, key_data, sizeof(key_data), keyType, key);
    cc_clear(sizeof(key_data), key_data);
    return rv;
}

CCCryptorStatus
CCCKGContributorFinish(CCCollabKeyGenContributorRef contrib,
                       const void *share, size_t shareLen,
                       void *opening, size_t openingLen,
                       void *sharedKey, size_t sharedKeyLen,
                       CCECCryptorRef *publicKey)
{
    if (contrib == NULL || share == NULL || opening == NULL || sharedKey == NULL || publicKey == NULL) {
        return kCCParamError;
    }

    CCCryptorStatus retval;
    ccec_pub_ctx_decl_cp(contrib->ctx->cp, P);
    ccec_ctx_init(contrib->ctx->cp, P);

    int rv = ccckg_contributor_finish(contrib->ctx, shareLen, share, openingLen, opening, P, sharedKeyLen, sharedKey);
    if (rv != kCCSuccess) {
        retval = CCCKGMapReturnCode(rv);
        goto cleanup;
    }

    // Convert ccec context to ECCryptor.
    retval = CCCKGConvertNativeToECCryptor(contrib->ctx, (ccec_full_ctx_t)P, ccECKeyPublic, publicKey);

cleanup:
    ccec_pub_ctx_clear_cp(contrib->ctx->cp, P);
    return retval;
}

CCCryptorStatus
CCCKGOwnerFinish(CCCollabKeyGenOwnerRef owner,
                 const void *opening, size_t openingLen,
                 void *sharedKey, size_t sharedKeyLen,
                 CCECCryptorRef *privateKey)
{
    if (owner == NULL || opening == NULL || sharedKey == NULL || privateKey == NULL) {
        return kCCParamError;
    }

    CCCryptorStatus retval;
    ccec_full_ctx_decl_cp(owner->ctx->cp, P);
    ccec_ctx_init(owner->ctx->cp, P);

    int rv = ccckg_owner_finish(owner->ctx, openingLen, opening, P, sharedKeyLen, sharedKey);
    if (rv != kCCSuccess) {
        retval = CCCKGMapReturnCode(rv);
        goto cleanup;
    }

    // Convert ccec context to ECCryptor.
    retval = CCCKGConvertNativeToECCryptor(owner->ctx, P, ccECKeyPrivate, privateKey);

cleanup:
    ccec_full_ctx_clear_cp(owner->ctx->cp, P);
    return retval;
}
