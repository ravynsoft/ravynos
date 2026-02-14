//
//  CommonKeyDerivation.c
//  CommonCrypto
//
//  Created by Richard Murphy on 1/21/14.
//  Copyright (c) 2014 Apple Inc. All rights reserved.
//

#include <stdio.h>
#include "testbyteBuffer.h"
#include "testutil.h"
#include "capabilities.h"
#include "testmore.h"
#include <string.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonHMAC.h>
#include <CommonCrypto/CommonKeyDerivationSPI.h>
#include <CommonCrypto/CommonDigestSPI.h>
#include <CommonCrypto/CommonKeyDerivation.h>

typedef struct KDFVector_t {
    char *password;
    char *salt;
    int saltLen; //negative means get the salt from the string
    unsigned rounds;
    CCDigestAlgorithm alg;
    int dklen;
    char *expectedstr;
    int expected_failure;
} KDFVector;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static KDFVector kdfv[] = {
    // Test Case PBKDF2 - HMACSHA1 http://tools.ietf.org/html/draft-josefsson-pbkdf2-test-vectors-00
    { "password", "salt", -1, 1   , kCCDigestSHA1, 20, "0c60c80f961f0e71f3a9b524af6012062fe037a6", 0 },
    { "password", "salt", -1, 2   , kCCDigestSHA1, 20, "ea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957", 0 },
    { "password", "salt", -1, 4096, kCCDigestSHA1, 20, "4b007901b765489abead49d926f721d065a429c1", 0 },
    { "password", "salt", -1, 1   , 0            , 20, NULL, kCCParamError},
    { "",         "salt", -1, 1   , kCCDigestSHA1, 20, "a33dddc30478185515311f8752895d36ea4363a2", kCCParamError},
    {.password=NULL},
};

static KDFVector kdfv_for_OriginalKDF[] = {
    // some extra
    { "password", "salt",  0, 4096, kCCDigestSHA1, 20, "546878f250c3baf85d44fbf77435a03828811dfb", 0 },
    { "password", NULL  ,  0, 4096, kCCDigestSHA1, 20, "546878f250c3baf85d44fbf77435a03828811dfb", 0 },
    { "password", NULL  ,999, 4096, kCCDigestSHA1, 20, "", kCCParamError },
    {.password=NULL},
};
#pragma clang diagnostic pop
static int testOriginalKDF(KDFVector *v, int testid) {
    CCPseudoRandomAlgorithm prf = digestID2PRF(v->alg);
    byteBuffer derivedKey = mallocByteBuffer(v->dklen);
    byteBuffer expected = hexStringToBytesIfNotNULL(v->expectedstr);
    
    size_t saltLen;
    
    if (v->saltLen<0&& v->salt!=NULL)
        saltLen = strlen(v->salt);
    else
        saltLen = v->saltLen;
    
    int retval = CCKeyDerivationPBKDF(kCCPBKDF2, v->password, strlen(v->password), (uint8_t *) v->salt, saltLen, prf, v->rounds, derivedKey->bytes, derivedKey->len);
    if(v->expected_failure && strlen(v->password)!=0) {
        is(retval, v->expected_failure, "Test %d: CCPBKDF2 should have failed",testid);
    } else {
        is(retval,0,"Test %d: non-zero return value",testid);
        is(derivedKey->len,expected->len,"Test %d: Length failure",testid);
        ok_memcmp(derivedKey->bytes, expected->bytes, expected->len, "Test %d: Derived key failure", testid);
    }
    
    free(derivedKey);
    free(expected);
    return 1;
}

static int testNewKDF(KDFVector *v, int testid) {
    byteBuffer derivedKey = mallocByteBuffer(v->dklen);
    byteBuffer expected = hexStringToBytesIfNotNULL(v->expectedstr);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    CCStatus retval = CCKeyDerivationHMac(kCCKDFAlgorithmPBKDF2_HMAC, v->alg, v->rounds, v->password, strlen(v->password), NULL, 0, NULL, 0, NULL, 0, v->salt, strlen(v->salt), derivedKey->bytes, derivedKey->len);
#pragma clang diagnostic pop
    if(v->expected_failure) {
        is(retval, v->expected_failure,"Test %d: PBKDF2_HMAC Expected failure",testid);
    } else {
        is(retval,0,"Test %d: non-zero return value",testid);
        is(derivedKey->len,expected->len,"Test %d: Length failure",testid);
        ok_memcmp(derivedKey->bytes, expected->bytes, expected->len, "Test %d: Derived key failure", testid);
    }
    free(derivedKey);
    free(expected);
    return 1;
}

int CommonKeyDerivation(int __unused argc, char *const * __unused argv) {
	plan_tests(31);
    
    int i;
    for(i=0; kdfv[i].password!=NULL; i++) {
        testOriginalKDF(&kdfv[i],i);
        testNewKDF(&kdfv[i],i);
    }
    
    for(int k=0; kdfv_for_OriginalKDF[k].password!=NULL; k++, i++) {
        testOriginalKDF(&kdfv_for_OriginalKDF[k],i);
    }


    unsigned iter;
    // Password of length 10byte, salt 16bytes, output 32bytes, 100msec
    iter=CCCalibratePBKDF(kCCPBKDF2, 10, 0, kCCPRFHmacAlgSHA1, 32, 100);
    diag("CCCalibratePBKDF kCCPBKDF2 100ms for kCCPRFHmacAlgSHA1 no salt:   %7lu", iter);
    iter=CCCalibratePBKDF(kCCPBKDF2, 10, 16, kCCPRFHmacAlgSHA1, 32, 100);
    diag("CCCalibratePBKDF kCCPBKDF2 100ms for kCCPRFHmacAlgSHA1:   %7lu", iter);
    iter=CCCalibratePBKDF(kCCPBKDF2, 10, 16, kCCPRFHmacAlgSHA224, 32, 100);
    diag("CCCalibratePBKDF kCCPBKDF2 100ms for kCCPRFHmacAlgSHA224: %7lu", iter);
    iter=CCCalibratePBKDF(kCCPBKDF2, 10, 16, kCCPRFHmacAlgSHA256, 32, 100);
    diag("CCCalibratePBKDF kCCPBKDF2 100ms for kCCPRFHmacAlgSHA256: %7lu", iter);
    iter=CCCalibratePBKDF(kCCPBKDF2, 10, 16, kCCPRFHmacAlgSHA384, 32, 100);
    diag("CCCalibratePBKDF kCCPBKDF2 100ms for kCCPRFHmacAlgSHA384: %7lu", iter);
    iter=CCCalibratePBKDF(kCCPBKDF2, 10, 16, kCCPRFHmacAlgSHA512, 32, 100);
    diag("CCCalibratePBKDF kCCPBKDF2 100ms for kCCPRFHmacAlgSHA512: %7lu", iter);

    return 0;
}
