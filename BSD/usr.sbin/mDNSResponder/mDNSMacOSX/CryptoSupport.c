/*
 * Copyright (c) 2011-2019 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// ***************************************************************************
// CryptoSupport.c
// Supporting routines for DNSSEC crypto
// ***************************************************************************

#include "mDNSEmbeddedAPI.h"
#include <CommonCrypto/CommonDigest.h>  // For Hash algorithms SHA1 etc.
#include <dispatch/dispatch.h>          // For Base32/Base64 encoding/decoding
#include <dispatch/private.h>          // dispatch_data_create_with_transform
#include "CryptoAlg.h"
#include "CryptoSupport.h"
#include "dnssec.h"
#include "DNSSECSupport.h"

#if TARGET_OS_IPHONE
#include <Security/SecRSAKey.h>                  // For RSA_SHA1 etc. verification
#else
#include <Security/Security.h>
#endif

#if !TARGET_OS_IPHONE
mDNSlocal SecKeyRef SecKeyCreateRSAPublicKey_OSX(unsigned char *asn1, int length);
#endif

typedef struct
{
    dispatch_data_t encData;
    dispatch_data_t encMap;
    dispatch_data_t encNULL;
}encContext;

mDNSlocal mStatus enc_create(AlgContext *ctx)
{
    encContext *ptr;

    switch (ctx->alg)
    {
    case ENC_BASE32:
    case ENC_BASE64:
        ptr = (encContext *) mDNSPlatformMemAllocateClear(sizeof(*ptr));
        if (!ptr) return mStatus_NoMemoryErr;
        break;
    default:
        LogMsg("enc_create: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
    ptr->encData = NULL;
    ptr->encMap = NULL;
    // The encoded data is not NULL terminated. So, we concatenate a null byte later when we encode and map
    // the real data.
    ptr->encNULL = dispatch_data_create("", 1, dispatch_get_global_queue(0, 0), ^{});
    if (!ptr->encNULL)
    {
        mDNSPlatformMemFree(ptr);
        return mStatus_NoMemoryErr;
    }
    ctx->context = ptr;
    return mStatus_NoError;
}

mDNSlocal mStatus enc_destroy(AlgContext *ctx)
{
    encContext *ptr = (encContext *)ctx->context;
    if (ptr->encData) dispatch_release(ptr->encData);
    if (ptr->encMap) dispatch_release(ptr->encMap);
    if (ptr->encNULL) dispatch_release(ptr->encNULL);
    mDNSPlatformMemFree(ptr);
    return mStatus_NoError;
}

mDNSlocal mStatus enc_add(AlgContext *ctx, const void *data, mDNSu32 len)
{
    switch (ctx->alg)
    {
    case ENC_BASE32:
    case ENC_BASE64:
    {
        encContext *ptr = (encContext *)ctx->context;
        dispatch_data_t src_data = dispatch_data_create(data, len, dispatch_get_global_queue(0, 0), ^{});
        if (!src_data)
        {
            LogMsg("enc_add: dispatch_data_create src failed");
            return mStatus_BadParamErr;
        }
        dispatch_data_t dest_data = dispatch_data_create_with_transform(src_data, DISPATCH_DATA_FORMAT_TYPE_NONE,
                                                                        (ctx->alg == ENC_BASE32 ? DISPATCH_DATA_FORMAT_TYPE_BASE32HEX : DISPATCH_DATA_FORMAT_TYPE_BASE64));
        dispatch_release(src_data);
        if (!dest_data)
        {
            LogMsg("enc_add: dispatch_data_create dst failed");
            return mStatus_BadParamErr;
        }
        ptr->encData = dest_data;

        return mStatus_NoError;
    }
    default:
        LogMsg("enc_add: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
}

mDNSlocal mDNSu8* enc_encode(AlgContext *ctx)
{
    const void *result = NULL;

    switch (ctx->alg)
    {
    case ENC_BASE32:
    case ENC_BASE64:
    {
        encContext *ptr = (encContext *)ctx->context;
        size_t size;
        dispatch_data_t dest_data = ptr->encData;
        dispatch_data_t data = dispatch_data_create_concat(dest_data, ptr->encNULL);

        if (!data)
        {
            LogMsg("enc_encode: cannot concatenate");
            return NULL;
        }

        dispatch_data_t map = dispatch_data_create_map(data, &result, &size);
        if (!map)
        {
            LogMsg("enc_encode: cannot create map %d", ctx->alg);
            return NULL;
        }
        dispatch_release(dest_data);
        ptr->encData = data;
        ptr->encMap = map;

        return (mDNSu8 *)result;
    }
    default:
        LogMsg("enc_encode: Unsupported algorithm %d", ctx->alg);
        return mDNSNULL;
    }
}

mDNSlocal mStatus sha_create(AlgContext *ctx)
{
    mDNSu8 *ptr;
    switch (ctx->alg)
    {
    case SHA1_DIGEST_TYPE:
        ptr = (mDNSu8 *) mDNSPlatformMemAllocate(sizeof(CC_SHA1_CTX));
        if (!ptr) return mStatus_NoMemoryErr;
        CC_SHA1_Init((CC_SHA1_CTX *)ptr);
        break;
    case SHA256_DIGEST_TYPE:
        ptr = (mDNSu8 *) mDNSPlatformMemAllocate(sizeof(CC_SHA256_CTX));
        if (!ptr) return mStatus_NoMemoryErr;
        CC_SHA256_Init((CC_SHA256_CTX *)ptr);
        break;
    default:
        LogMsg("sha_create: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
    ctx->context = ptr;
    return mStatus_NoError;
}

mDNSlocal mStatus sha_destroy(AlgContext *ctx)
{
    mDNSPlatformMemFree(ctx->context);
    return mStatus_NoError;
}

mDNSlocal mDNSu32 sha_len(AlgContext *ctx)
{
    switch (ctx->alg)
    {
    case SHA1_DIGEST_TYPE:
        return CC_SHA1_DIGEST_LENGTH;
    case SHA256_DIGEST_TYPE:
        return CC_SHA256_DIGEST_LENGTH;
    default:
        LogMsg("sha_len: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
}

mDNSlocal mStatus sha_add(AlgContext *ctx, const void *data, mDNSu32 len)
{
    switch (ctx->alg)
    {
    case SHA1_DIGEST_TYPE:
        CC_SHA1_Update((CC_SHA1_CTX *)ctx->context, data, len);
        break;
    case SHA256_DIGEST_TYPE:
        CC_SHA256_Update((CC_SHA256_CTX *)ctx->context, data, len);
        break;
    default:
        LogMsg("sha_add: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
    return mStatus_NoError;
}

mDNSlocal mStatus sha_verify(AlgContext *ctx, mDNSu8 *key, mDNSu32 keylen, mDNSu8 *digestIn, mDNSu32 dlen)
{
    mDNSu8 digest[CC_SHA512_DIGEST_LENGTH];
    mDNSu32 digestLen;

    (void) key;   //unused
    (void)keylen; //unused
    switch (ctx->alg)
    {
    case SHA1_DIGEST_TYPE:
        digestLen = CC_SHA1_DIGEST_LENGTH;
        CC_SHA1_Final(digest, (CC_SHA1_CTX *)ctx->context);
        break;
    case SHA256_DIGEST_TYPE:
        digestLen = CC_SHA256_DIGEST_LENGTH;
        CC_SHA256_Final(digest, (CC_SHA256_CTX *)ctx->context);
        break;
    default:
        LogMsg("sha_verify: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
    if (dlen != digestLen)
    {
        LogMsg("sha_verify(Alg %d): digest len mismatch len %u, expected %u", ctx->alg, (unsigned int)dlen, (unsigned int)digestLen);
        return mStatus_BadParamErr;
    }
    if (!memcmp(digest, digestIn, digestLen))
        return mStatus_NoError;
    else
        return mStatus_NoAuth;
}

mDNSlocal mStatus sha_final(AlgContext *ctx, void *digestOut, mDNSu32 dlen)
{
    mDNSu8 digest[CC_SHA512_DIGEST_LENGTH];
    mDNSu32 digestLen;

    switch (ctx->alg)
    {
    case SHA1_DIGEST_TYPE:
        digestLen = CC_SHA1_DIGEST_LENGTH;
        CC_SHA1_Final(digest, (CC_SHA1_CTX *)ctx->context);
        break;
    case SHA256_DIGEST_TYPE:
        digestLen = CC_SHA256_DIGEST_LENGTH;
        CC_SHA256_Final(digest, (CC_SHA256_CTX *)ctx->context);
        break;
    default:
        LogMsg("sha_final: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
    if (dlen != digestLen)
    {
        LogMsg("sha_final(Alg %d): digest len mismatch len %u, expected %u", ctx->alg, (unsigned int)dlen, (unsigned int)digestLen);
        return mStatus_BadParamErr;
    }
    memcpy(digestOut, digest, digestLen);
    return mStatus_NoError;
}

mDNSlocal mStatus rsa_sha_create(AlgContext *ctx)
{
    mDNSu8 *ptr;
    switch (ctx->alg)
    {
    case CRYPTO_RSA_NSEC3_SHA1:
    case CRYPTO_RSA_SHA1:
        ptr = (mDNSu8 *) mDNSPlatformMemAllocate(sizeof(CC_SHA1_CTX));
        if (!ptr) return mStatus_NoMemoryErr;
        CC_SHA1_Init((CC_SHA1_CTX *)ptr);
        break;
    case CRYPTO_RSA_SHA256:
        ptr = (mDNSu8 *) mDNSPlatformMemAllocate(sizeof(CC_SHA256_CTX));
        if (!ptr) return mStatus_NoMemoryErr;
        CC_SHA256_Init((CC_SHA256_CTX *)ptr);
        break;
    case CRYPTO_RSA_SHA512:
        ptr = (mDNSu8 *) mDNSPlatformMemAllocate(sizeof(CC_SHA512_CTX));
        if (!ptr) return mStatus_NoMemoryErr;
        CC_SHA512_Init((CC_SHA512_CTX *)ptr);
        break;
    default:
        LogMsg("rsa_sha_create: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
    ctx->context = ptr;
    return mStatus_NoError;
}

mDNSlocal mStatus rsa_sha_destroy(AlgContext *ctx)
{
    mDNSPlatformMemFree(ctx->context);
    return mStatus_NoError;
}

mDNSlocal mDNSu32 rsa_sha_len(AlgContext *ctx)
{
    switch (ctx->alg)
    {
    case CRYPTO_RSA_NSEC3_SHA1:
    case CRYPTO_RSA_SHA1:
        return CC_SHA1_DIGEST_LENGTH;
    case CRYPTO_RSA_SHA256:
        return CC_SHA256_DIGEST_LENGTH;
    case CRYPTO_RSA_SHA512:
        return CC_SHA512_DIGEST_LENGTH;
    default:
        LogMsg("rsa_sha_len: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
}

mDNSlocal mStatus rsa_sha_add(AlgContext *ctx, const void *data, mDNSu32 len)
{
    switch (ctx->alg)
    {
    case CRYPTO_RSA_NSEC3_SHA1:
    case CRYPTO_RSA_SHA1:
        CC_SHA1_Update((CC_SHA1_CTX *)ctx->context, data, len);
        break;
    case CRYPTO_RSA_SHA256:
        CC_SHA256_Update((CC_SHA256_CTX *)ctx->context, data, len);
        break;
    case CRYPTO_RSA_SHA512:
        CC_SHA512_Update((CC_SHA512_CTX *)ctx->context, data, len);
        break;
    default:
        LogMsg("rsa_sha_add: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }
    return mStatus_NoError;
}

mDNSlocal SecKeyRef rfc3110_import(const mDNSu8 *data, const mDNSu32 len)
{
    static const int max_modulus_bytes = 512;    // Modulus is limited to 4096 bits (512 octets) in length.
    static const int max_exp_bytes = 512;        // Exponent is limited to 4096 bits (512 octets) in length.
    static const int asn1_type_bytes = 3;        // Since there is an ASN1 SEQ and two INTs.
    static const int asn1_max_len_bytes = 3 * 3; // Capped at 3 due to max payload size.
    unsigned char asn1[max_modulus_bytes + 1 + max_exp_bytes + asn1_type_bytes + asn1_max_len_bytes]; // +1 is for leading 0 for non negative asn1 number
    const mDNSu8 *modulus;
    unsigned int modulus_length;
    const mDNSu8 *exponent;
    unsigned int exp_length;
    unsigned int num_length_bytes;
    mDNSu32 index = 0;
    mDNSu32 asn1_length = 0;

    // Validate Input
    if (!data)
        return NULL;

    // we have to have at least 1 byte for the length
    if (len < 1)
        return NULL;

    // Parse Modulus and Exponent

    // If the first byte is zero, then the exponent length is in the three-byte format, otherwise the length is in the first byte.
    if (data[0] == 0)
    {
        if (len < 3)
            return NULL;
        exp_length = (data[1] << 8) | data[2];
        num_length_bytes = 3;
    }
    else
    {
        exp_length = data[0];
        num_length_bytes = 1;
    }

    // RFC3110 limits the exponent length to 4096 bits (512 octets).
    if (exp_length > 512)
        return NULL;

    // We have to have at least len bytes + size of exponent.
    if (len < (num_length_bytes + exp_length))
        return NULL;

    // The modulus is the remaining space.
    modulus_length = len - (num_length_bytes + exp_length);

    // RFC3110 limits the modulus length to 4096 bits (512 octets).
    if (modulus_length > 512)
        return NULL;

    if (modulus_length < 1)
        return NULL;

    // add 1 to modulus length for pre-ceding 0 t make ASN1 value non-negative
    ++modulus_length;

    exponent = &data[num_length_bytes];
    modulus = &data[num_length_bytes + exp_length];

    // 2 bytes for commands since first doesn't count
    // 2 bytes for min 1 byte length field
    asn1_length = modulus_length + exp_length + 2 + 2;

    // Account for modulus length causing INT length field to grow.
    if (modulus_length >= 128)
    {
        if (modulus_length > 255)
            asn1_length += 2;
        else
            asn1_length += 1;
    }

    // Account for exponent length causing INT length field to grow.
    if (exp_length >= 128)
    {
        if (exp_length > 255)
            asn1_length += 2;
        else
            asn1_length += 1;
    }

    // Construct ASN1 formatted public key
    // Write ASN1 SEQ byte
    asn1[index++] = 0x30;

    // Write ASN1 length for SEQ
    if (asn1_length < 128)
    {
        asn1[index++] = asn1_length & 0xFF;
    }
    else
    {
        asn1[index++] = (0x80 | ((asn1_length > 255) ? 2 : 1));
        if (asn1_length > 255)
            asn1[index++] = (asn1_length & 0xFF00) >> 8;
        asn1[index++] = asn1_length & 0xFF;
    }

    // Write ASN1 INT for modulus
    asn1[index++] = 0x02;
    // Write ASN1 length for INT
    if (modulus_length < 128)
    {
        asn1[index++] = modulus_length & 0xFF;
    }
    else
    {
        asn1[index++] = 0x80 | ((modulus_length > 255) ? 2 : 1);
        if (modulus_length > 255)
            asn1[index++] = (modulus_length & 0xFF00) >> 8;
        asn1[index++] = modulus_length & 0xFF;
    }

    // Write preceding 0 so our integer isn't negative
    asn1[index++] = 0x00;
    // Write actual modulus (-1 for preceding 0)
    memcpy(&asn1[index], modulus, modulus_length - 1);
    index += (modulus_length - 1);

    // Write ASN1 INT for exponent
    asn1[index++] = 0x02;
    // Write ASN1 length for INT
    if (exp_length < 128)
    {
        asn1[index++] = exp_length & 0xFF;
    }
    else
    {
        asn1[index++] = 0x80 | ((exp_length > 255) ? 2 : 1);
        if (exp_length > 255)
            asn1[index++] = (exp_length & 0xFF00) >> 8;
        asn1[index++] = exp_length & 0xFF;
    }
    // Write exponent bytes
    memcpy(&asn1[index], exponent, exp_length);
    index += exp_length;

#if TARGET_OS_IPHONE
    // index contains bytes written, use it for length
    return (SecKeyCreateRSAPublicKey(NULL, asn1, index, kSecKeyEncodingPkcs1));
#else
    return (SecKeyCreateRSAPublicKey_OSX(asn1, index));
#endif
}

#if TARGET_OS_IPHONE
mDNSlocal mStatus rsa_sha_verify(AlgContext *ctx, mDNSu8 *key, mDNSu32 keylen, mDNSu8 *signature, mDNSu32 siglen)
{
    SecKeyRef keyref;
    OSStatus result;
    mDNSu8 digest[CC_SHA512_DIGEST_LENGTH];
    int digestlen;
    int cryptoAlg;

    switch (ctx->alg)
    {
    case CRYPTO_RSA_NSEC3_SHA1:
    case CRYPTO_RSA_SHA1:
        cryptoAlg = kSecPaddingPKCS1SHA1;
        digestlen = CC_SHA1_DIGEST_LENGTH;
        CC_SHA1_Final(digest, (CC_SHA1_CTX *)ctx->context);
        break;
    case CRYPTO_RSA_SHA256:
        cryptoAlg = kSecPaddingPKCS1SHA256;
        digestlen = CC_SHA256_DIGEST_LENGTH;
        CC_SHA256_Final(digest, (CC_SHA256_CTX *)ctx->context);
        break;
    case CRYPTO_RSA_SHA512:
        cryptoAlg = kSecPaddingPKCS1SHA512;
        digestlen = CC_SHA512_DIGEST_LENGTH;
        CC_SHA512_Final(digest, (CC_SHA512_CTX *)ctx->context);
        break;
    default:
        LogMsg("rsa_sha_verify: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }

    keyref = rfc3110_import(key, keylen);
    if (!keyref)
    {
        LogMsg("rsa_sha_verify: Error decoding rfc3110 key data");
        return mStatus_NoMemoryErr;
    }
    result = SecKeyRawVerify(keyref, cryptoAlg, digest, digestlen, signature, siglen);
    CFRelease(keyref);
    if (result != noErr)
    {
        LogMsg("rsa_sha_verify: Failed for alg %d", ctx->alg);
        return mStatus_BadParamErr;
    }
    else
    {
        LogInfo("rsa_sha_verify: Passed for alg %d", ctx->alg);
        return mStatus_NoError;
    }
}
#else // TARGET_OS_IPHONE

mDNSlocal SecKeyRef SecKeyCreateRSAPublicKey_OSX(unsigned char *asn1, int length)
{
    SecKeyRef result = NULL;
    
    SecExternalFormat extFormat = kSecFormatBSAFE;
    SecExternalItemType itemType = kSecItemTypePublicKey;
    CFArrayRef outArray = NULL;

    CFDataRef keyData = CFDataCreate(NULL, asn1, length);
    if (!keyData)
        return NULL;

    OSStatus err =  SecItemImport(keyData, NULL, &extFormat, &itemType, 0, NULL, NULL, &outArray);
    
    CFRelease(keyData);
    if (noErr != err || outArray == NULL)
    {
        if (outArray)
            CFRelease(outArray);
        return NULL;
    }
    
    result = (SecKeyRef)CFArrayGetValueAtIndex(outArray, 0);
    if (result == NULL)
    {
        CFRelease(outArray);
        return NULL;
    }
    
    CFRetain(result);
    CFRelease(outArray);
    return result;
}

mDNSlocal Boolean VerifyData(SecKeyRef key, CFStringRef digestStr, mDNSu8 *digest, int dlen, int digestlenAttr, mDNSu8 *sig, int siglen, CFStringRef digest_type)
{
    CFErrorRef error;
    Boolean ret;
    
    CFDataRef signature = CFDataCreate(NULL, sig, siglen);
    if (!signature)
        return false;
    
    SecTransformRef verifyXForm = SecVerifyTransformCreate(key, signature, &error);
    CFRelease(signature);
    if (verifyXForm == NULL)
    {
        return false;
    }
    
    // tell the transform what type of data it is geting
    if (!SecTransformSetAttribute(verifyXForm, kSecInputIsAttributeName, digest_type, &error))
    {
        LogMsg("VerifyData: SecTransformSetAttribute digest_type");
        goto err;
    }
    
    if (!SecTransformSetAttribute(verifyXForm, kSecDigestTypeAttribute, digestStr, &error))
    {
        LogMsg("VerifyData: SecTransformSetAttribute digestStr");
        goto err;
    }
    
    CFNumberRef digestLengthRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &digestlenAttr);
    if (digestLengthRef == NULL)
    {
        LogMsg("VerifyData: CFNumberCreate failed");
        goto err;
    }
    
    ret = SecTransformSetAttribute(verifyXForm, kSecDigestLengthAttribute, digestLengthRef, &error);
    CFRelease(digestLengthRef);
    if (!ret)
    {
        LogMsg("VerifyData: SecTransformSetAttribute digestLengthRef");
        goto err;
    }
    
    CFDataRef dataToSign = CFDataCreate(NULL, digest, dlen);
    if (dataToSign ==  NULL)
    {
        LogMsg("VerifyData: CFDataCreate failed");
        goto err;
    }

    ret = SecTransformSetAttribute(verifyXForm, kSecTransformInputAttributeName, dataToSign, &error);
    CFRelease(dataToSign);
    if (!ret)
    {
        LogMsg("VerifyData: SecTransformSetAttribute TransformAttributeName");
        goto err;
    }
    
    CFBooleanRef boolRef = SecTransformExecute(verifyXForm, &error);
    ret = (boolRef != NULL) ? CFBooleanGetValue(boolRef) : false;
    if (boolRef != NULL) CFRelease(boolRef);
    CFRelease(verifyXForm);

    if (error != NULL)
    {
        CFStringRef errStr = CFErrorCopyDescription(error);
        char errorbuf[128];
        errorbuf[0] = 0;
        if (errStr != NULL)
        {
            if (!CFStringGetCString(errStr, errorbuf, sizeof(errorbuf), kCFStringEncodingUTF8))
            {
                LogMsg("VerifyData: CFStringGetCString failed");
            }
            CFRelease(errStr);
        }
        LogMsg("VerifyData: SecTransformExecute failed with %s", errorbuf);
        return false;
    }
    return ret;
err:
    CFRelease(verifyXForm);
    return false;
}

mDNSlocal mStatus rsa_sha_verify(AlgContext *ctx, mDNSu8 *key, mDNSu32 keylen, mDNSu8 *signature, mDNSu32 siglen)
{
    SecKeyRef keyref;
    mDNSu8 digest[CC_SHA512_DIGEST_LENGTH];
    int digestlen;
    int digestlenAttr;
    CFStringRef digestStr;
    mDNSBool ret;

    switch (ctx->alg)
    {
    case CRYPTO_RSA_NSEC3_SHA1:
    case CRYPTO_RSA_SHA1:
        digestStr = kSecDigestSHA1;
        digestlen = CC_SHA1_DIGEST_LENGTH;
        digestlenAttr = 0;
        CC_SHA1_Final(digest, (CC_SHA1_CTX *)ctx->context);
        break;
    case CRYPTO_RSA_SHA256:
        digestStr = kSecDigestSHA2;
        digestlen = CC_SHA256_DIGEST_LENGTH;
        digestlenAttr = 256;
        CC_SHA256_Final(digest, (CC_SHA256_CTX *)ctx->context);
        break;
    case CRYPTO_RSA_SHA512:
        digestStr = kSecDigestSHA2;
        digestlen = CC_SHA512_DIGEST_LENGTH;
        digestlenAttr = 512;
        CC_SHA512_Final(digest, (CC_SHA512_CTX *)ctx->context);
        break;
    default:
        LogMsg("rsa_sha_verify: Unsupported algorithm %d", ctx->alg);
        return mStatus_BadParamErr;
    }

    keyref = rfc3110_import(key, keylen);
    if (!keyref)
    {
        LogMsg("rsa_sha_verify: Error decoding rfc3110 key data");
        return mStatus_NoMemoryErr;
    }
    ret = VerifyData(keyref, digestStr, digest, digestlen, digestlenAttr, signature, siglen, kSecInputIsDigest);
    CFRelease(keyref);
    if (!ret)
    {
        LogMsg("rsa_sha_verify: Failed for alg %d", ctx->alg);
        return mStatus_BadParamErr;
    }
    else
    {
        LogInfo("rsa_sha_verify: Passed for alg %d", ctx->alg);
        return mStatus_NoError;
    }
}
#endif // TARGET_OS_IPHONE

AlgFuncs sha_funcs = {sha_create, sha_destroy, sha_len, sha_add, sha_verify, mDNSNULL, sha_final};
AlgFuncs rsa_sha_funcs = {rsa_sha_create, rsa_sha_destroy, rsa_sha_len, rsa_sha_add, rsa_sha_verify, mDNSNULL, mDNSNULL};
AlgFuncs enc_funcs = {enc_create, enc_destroy, mDNSNULL, enc_add, mDNSNULL, enc_encode, mDNSNULL};

#ifndef DNSSEC_DISABLED

mDNSexport mStatus DNSSECCryptoInit(mDNS *const m)
{
    mStatus result;

    result = DigestAlgInit(SHA1_DIGEST_TYPE, &sha_funcs);
    if (result != mStatus_NoError)
        return result;
    result = DigestAlgInit(SHA256_DIGEST_TYPE, &sha_funcs);
    if (result != mStatus_NoError)
        return result;
    result = CryptoAlgInit(CRYPTO_RSA_SHA1, &rsa_sha_funcs);
    if (result != mStatus_NoError)
        return result;
    result = CryptoAlgInit(CRYPTO_RSA_NSEC3_SHA1, &rsa_sha_funcs);
    if (result != mStatus_NoError)
        return result;
    result = CryptoAlgInit(CRYPTO_RSA_SHA256, &rsa_sha_funcs);
    if (result != mStatus_NoError)
        return result;
    result = CryptoAlgInit(CRYPTO_RSA_SHA512, &rsa_sha_funcs);
    if (result != mStatus_NoError)
        return result;
    result = EncAlgInit(ENC_BASE32, &enc_funcs);
    if (result != mStatus_NoError)
        return result;
    result = EncAlgInit(ENC_BASE64, &enc_funcs);
    if (result != mStatus_NoError)
        return result;

    result = DNSSECPlatformInit(m);

    return result;
}

#else // !DNSSEC_DISABLED

mDNSexport mStatus DNSSECCryptoInit(mDNS *const m)
{
    (void) m;
    
    return mStatus_NoError;
}

#endif // !DNSSEC_DISABLED


