//
//  CommonHMac.c
//  CommonCrypto
//
//  Created by Richard Murphy on 1/21/14.
//  Copyright (c) 2014 Apple Inc. All rights reserved.
//

#include <stdio.h>
#include "testbyteBuffer.h"
#include "capabilities.h"
#include "testmore.h"
#include "testutil.h"
#include <string.h>

#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonHMAC.h>
#include <CommonCrypto/CommonHMacSPI.h>
#include <CommonCrypto/CommonKeyDerivationSPI.h>
#include <CommonCrypto/CommonDigestSPI.h>


typedef struct HMacVector_t {
    char *keystr;
    char *input;
    char *md5str;
    char *sha1str;
    char *sha224str;
    char *sha256str;
    char *sha384str;
    char *sha512str;
} HMacVector;

static HMacVector hmv[] = {
    { // Test Case 1 http://www.faqs.org/rfcs/rfc4231.html MD5 derived
        "0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
        "Hi There",
        "5ccec34ea9656392457fa1ac27f08fbc",
        "b617318655057264e28bc0b6fb378c8ef146be00",
        "896fb1128abbdf196832107cd49df33f47b4b1169912ba4f53684b22",
        "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7",
        "afd03944d84895626b0825f4ab46907f15f9dadbe4101ec682aa034c7cebc59cfaea9ea9076ede7f4af152e8b2fa9cb6",
        "87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cdedaa833b7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854",
    },
    { // Test Vector from http://www.faqs.org/rfcs/rfc2104.html - all but MD5 derived
        "0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
        "Hi There",
        "9294727a3638bb1c13f48ef8158bfc9d",
        "675b0b3a1b4ddf4e124872da6c2f632bfed957e9",
        "4e841ce7a4ae83fbcf71e3cd64bfbf277f73a14680aae8c518ac7861",
        "492ce020fe2534a5789dc3848806c78f4f6711397f08e7e7a12ca5a4483c8aa6",
        "7afaa633e20d379b02395915fbc385ff8dc27dcd3885e1068ab942eeab52ec1f20ad382a92370d8b2e0ac8b83c4d53bf",
        "7641c48a3b4aa8f887c07b3e83f96affb89c978fed8c96fcbbf4ad596eebfe496f9f16da6cd080ba393c6f365ad72b50d15c71bfb1d6b81f66a911786c6ce932",
    },
    { // Try this with a NULL key
        NULL,
        "Hi There",
        "72c33c78cac0b7a581ac263a344ed01d",
        "69536cc84eee5fe51c5b051aff8485f5c9ef0b58",
        "da8f94de91d62154b55ea4e8d6eb133f6d553bcd1f1ba205b9488945",
        "e48411262715c8370cd5e7bf8e82bef53bd53712d007f3429351843b77c7bb9b",
        "da5393cef424a670d6db42c6ed6e7920779dfa4cbb98bf1c2e9c12ae10d10905d0c9e9d576c2a613be54b8daea246d4b",
        "f7688a104326d36c1940f6d28d746c0661d383e0d14fe8a04649444777610f5dd9565a36846ab9e9e734cf380d3a070d8ef021b5f3a50c481710a464968e3419",
    },
    { // Same with a "" Key - Zero Length with string
        "",
        "Hi There",
        "72c33c78cac0b7a581ac263a344ed01d",
        "69536cc84eee5fe51c5b051aff8485f5c9ef0b58",
        "da8f94de91d62154b55ea4e8d6eb133f6d553bcd1f1ba205b9488945",
        "e48411262715c8370cd5e7bf8e82bef53bd53712d007f3429351843b77c7bb9b",
        "da5393cef424a670d6db42c6ed6e7920779dfa4cbb98bf1c2e9c12ae10d10905d0c9e9d576c2a613be54b8daea246d4b",
        "f7688a104326d36c1940f6d28d746c0661d383e0d14fe8a04649444777610f5dd9565a36846ab9e9e734cf380d3a070d8ef021b5f3a50c481710a464968e3419",
    },
    
};

static size_t hmvLen = sizeof(hmv) / sizeof(HMacVector);



static char * testString(char *format, CCDigestAlgorithm alg) {
    static char thestring[80];
    sprintf(thestring, format, digestName(alg));
    return thestring;
}

static int testOriginalOneShotHMac(CCDigestAlgorithm alg, byteBuffer key, char *input, byteBuffer expected) {
    CCHmacAlgorithm hmacAlg = digestID2HMacID(alg);
    byteBuffer computedMD = mallocDigestByteBuffer(alg);
    int status = 0;
    
    CCHmac(hmacAlg, key->bytes, key->len, input, strlen(input), computedMD->bytes);
    ok(status = expectedEqualsComputed(testString("Original OneShot HMac-%s", alg), expected, computedMD), "HMac is as expected");
    free(computedMD);
    return status;
}

static int testOriginalDiscreteHMac(CCDigestAlgorithm alg, byteBuffer key, char *input, byteBuffer expected) {
    CCHmacAlgorithm hmacAlg = digestID2HMacID(alg);
    byteBuffer computedMD = mallocDigestByteBuffer(alg);
    CCHmacContext ctx;
    int status = 0;
    
    CCHmacInit(&ctx, hmacAlg, key->bytes, key->len);
    CCHmacUpdate(&ctx, input, strlen(input));
    CCHmacFinal(&ctx, computedMD->bytes);
    ok(status = expectedEqualsComputed(testString("Original Discrete HMac-%s", alg), expected, computedMD), "HMac is as expected");
    free(computedMD);
    return status;
}

static int testNewOneShotHMac(CCDigestAlgorithm alg, byteBuffer key, char *input, byteBuffer expected) {
    byteBuffer computedMD = mallocDigestByteBuffer(alg);
    int status = 0;
    
    CCHmacOneShot(alg, key->bytes, key->len, input, strlen(input), computedMD->bytes);
    ok(status = expectedEqualsComputed(testString("New OneShot HMac-%s", alg), expected, computedMD), "HMac is as expected");
    free(computedMD);
    return status;
}

static int testNewDiscreteHMac(CCDigestAlgorithm alg, byteBuffer key, char *input, byteBuffer expected) {
    byteBuffer computedMD = mallocDigestByteBuffer(alg);
    byteBuffer computedMD2 = mallocDigestByteBuffer(alg);
    int status = 0;
    
    CCHmacContextRef ctx = CCHmacCreate(alg, key->bytes, key->len);
    CCHmacContextRef ctx2 = CCHmacClone(ctx);
    CCHmacUpdate(ctx, input, strlen(input));
    CCHmacFinal(ctx, computedMD->bytes);
    CCHmacDestroy(ctx);
    CCHmacUpdate(ctx2, input, strlen(input));
    CCHmacFinal(ctx2, computedMD2->bytes);
    CCHmacDestroy(ctx2);
    ok(status = expectedEqualsComputed(testString("New OneShot HMac-%s", alg), expected, computedMD), "HMac is as expected");
    ok(status = expectedEqualsComputed(testString("New OneShot HMac-%s", alg), expected, computedMD2), "HMac is as expected");
    free(computedMD);
    free(computedMD2);
    return status;
}

static int testAllHMacs(CCDigestAlgorithm alg, byteBuffer key, char *input, byteBuffer expected) {
    int status = 0;
    
    ok(status = testOriginalOneShotHMac(alg, key, input, expected), "Test Original One Shot version of HMac");
    ok(status &= testOriginalDiscreteHMac(alg, key, input, expected), "Test Original Discrete version of HMac");
    ok(status &= testNewOneShotHMac(alg, key, input, expected), "Test New One Shot version of HMac");
    ok(status &= testNewDiscreteHMac(alg, key, input, expected), "Test New Discrete version of HMac");
    return status;
}

static int testHMac(HMacVector *hv) {
    byteBuffer key = hexStringToBytes(hv->keystr);
    int status = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    byteBuffer expectedMD = hexStringToBytesIfNotNULL(hv->md5str);
    ok(status = testAllHMacs(kCCDigestMD5, key, hv->input, expectedMD), "Testing all MD5 Implementations");
    free(expectedMD);
    
    expectedMD = hexStringToBytesIfNotNULL(hv->sha1str);
    ok(status &= testAllHMacs(kCCDigestSHA1, key, hv->input, expectedMD), "Testing all SHA1 Implementations");
    free(expectedMD);
#pragma clang diagnostic pop
    expectedMD = hexStringToBytesIfNotNULL(hv->sha224str);
    ok(status &= testAllHMacs(kCCDigestSHA224, key, hv->input, expectedMD), "Testing all SHA224 Implementations");
    free(expectedMD);
    
    expectedMD = hexStringToBytesIfNotNULL(hv->sha256str);
    ok(status &= testAllHMacs(kCCDigestSHA256, key, hv->input, expectedMD), "Testing all SHA256 Implementations");
    free(expectedMD);
    
    expectedMD = hexStringToBytesIfNotNULL(hv->sha384str);
    ok(status &= testAllHMacs(kCCDigestSHA384, key, hv->input, expectedMD), "Testing all SHA384 Implementations");
    free(expectedMD);
    
    expectedMD = hexStringToBytesIfNotNULL(hv->sha512str);
    ok(status &= testAllHMacs(kCCDigestSHA512, key, hv->input, expectedMD), "Testing all SHA512 Implementations");
    free(expectedMD);
    free(key);
    return status;
}

static size_t testsPerVector = 61;

int CommonHMac(int __unused argc, char *const * __unused argv) {
	plan_tests((int) (hmvLen*testsPerVector));
    
    for(size_t testcase = 0; testcase < hmvLen; testcase++) {
        // diag("Test %lu\n", testcase + 1);
        ok(testHMac(&hmv[testcase]), "Successful full test of HMAC Vector");
    }
    return 0;
}

