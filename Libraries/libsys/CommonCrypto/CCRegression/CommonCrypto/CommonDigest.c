//
//  CommonDigest.c
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

#define COMMON_DIGEST_FOR_RFC_1321
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonHMAC.h>
#include <CommonCrypto/CommonKeyDerivationSPI.h>
#include <CommonCrypto/CommonDigestSPI.h>
#include "ccGlobals.h"

typedef struct DigestVector_t {
    char *input;
    char *md2intr;
    char *md4intr;
    char *md5intr;
    char *sha1intr;
    char *sha224intr;
    char *sha256intr;
    char *sha384intr;
    char *sha512intr;
    char *rmd160intr;
    char *md2str;
    char *md4str;
    char *md5str;
    char *sha1str;
    char *sha224str;
    char *sha256str;
    char *sha384str;
    char *sha512str;
    char *rmd160str;
} DigestVector;

static DigestVector dv[] = {
    {
        NULL,
        "00000000000000000000000000000000",
        "0123456789abcdeffedcba9876543210",
        "0123456789abcdeffedcba9876543210",
        "0123456789abcdeffedcba9876543210f0e1d2c3",
        "d89e05c107d57c3617dd703039590ef7310bc0ff11155868a78ff964",
        "67e6096a85ae67bb72f36e3c3af54fa57f520e518c68059babd9831f19cde05b",
        "d89e05c15d9dbbcb07d57c362a299a6217dd70305a01599139590ef7d8ec2f15310bc0ff6726336711155868874ab48e",
        "08c9bcf367e6096a3ba7ca8485ae67bb2bf894fe72f36e3cf1361d5f3af54fa5d182e6ad7f520e511f6c3e2b8c68059b6bbd41fbabd9831f79217e1319cde05b",
        NULL,
        "8350e5a3e24c153df2275c9f80692773",
        "31d6cfe0d16ae931b73c59d7e0c089c0",
        "d41d8cd98f00b204e9800998ecf8427e",
        "da39a3ee5e6b4b0d3255bfef95601890afd80709",
        "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f",
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b",
        "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e",
        "9c1185a5c5e9fc54612808977ee8f548b2258d31",
    },
    {
        "",
        "00000000000000000000000000000000",
        "0123456789abcdeffedcba9876543210",
        "0123456789abcdeffedcba9876543210",
        "0123456789abcdeffedcba9876543210f0e1d2c3",
        "d89e05c107d57c3617dd703039590ef7310bc0ff11155868a78ff964",
        "67e6096a85ae67bb72f36e3c3af54fa57f520e518c68059babd9831f19cde05b",
        "d89e05c15d9dbbcb07d57c362a299a6217dd70305a01599139590ef7d8ec2f15310bc0ff6726336711155868874ab48e",
        "08c9bcf367e6096a3ba7ca8485ae67bb2bf894fe72f36e3cf1361d5f3af54fa5d182e6ad7f520e511f6c3e2b8c68059b6bbd41fbabd9831f79217e1319cde05b",
        NULL,
        "8350e5a3e24c153df2275c9f80692773",
        "31d6cfe0d16ae931b73c59d7e0c089c0",
        "d41d8cd98f00b204e9800998ecf8427e",
        "da39a3ee5e6b4b0d3255bfef95601890afd80709",
        "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f",
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b",
        "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e",
        "9c1185a5c5e9fc54612808977ee8f548b2258d31",
    },
    {
        "Test vector from febooti.com",
        "82aa234f7a00b5488c01b3b44a67c045",
        "0123456789abcdeffedcba9876543210",
        "0123456789abcdeffedcba9876543210",
        "0123456789abcdeffedcba9876543210f0e1d2c3",
        "d89e05c107d57c3617dd703039590ef7310bc0ff11155868a78ff964",
        "67e6096a85ae67bb72f36e3c3af54fa57f520e518c68059babd9831f19cde05b",
        "d89e05c15d9dbbcb07d57c362a299a6217dd70305a01599139590ef7d8ec2f15310bc0ff6726336711155868874ab48e",
        "08c9bcf367e6096a3ba7ca8485ae67bb2bf894fe72f36e3cf1361d5f3af54fa5d182e6ad7f520e511f6c3e2b8c68059b6bbd41fbabd9831f79217e1319cde05b",
        NULL,
        "db128d6e0d20a1192a6bd1fade401150",
        "6578f2664bc56e0b5b3f85ed26ecc67b",
        "500ab6613c6db7fbd30c62f5ff573d0f",
        "a7631795f6d59cd6d14ebd0058a6394a4b93d868",
        "3628b402254caa96827e3c79c0a559e4558da8ee2b65f1496578137d",
        "077b18fe29036ada4890bdec192186e10678597a67880290521df70df4bac9ab",
        "388bb2d487de48740f45fcb44152b0b665428c49def1aaf7c7f09a40c10aff1cd7c3fe3325193c4dd35d4eaa032f49b0",
        "09fb898bc97319a243a63f6971747f8e102481fb8d5346c55cb44855adc2e0e98f304e552b0db1d4eeba8a5c8779f6a3010f0e1a2beb5b9547a13b6edca11e8a",
        "4e1ff644ca9f6e86167ccb30ff27e0d84ceb2a61",
    },
    { // Test from <rdar://problem/11285435> CC_SHA512_Init(),CC_SHA512_Update(),CC_SHA512_Final() gives wrong digest
        "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
        "f49dab72e8309bfd25ea1ed03a0b17b3",
        "1b6d7cb5746e7989cf73c1b50e9f6e8c",
        "6ac4e1d93aeb8428d339ca51d00a7a88",
        "e978c90687a072c78c44842d2ea19813bca6f3f7",
        "944688a12a9ca8a694a9d1bb6feb447762fffe9b675c287ecbccdeba",
        "db7d194f1e9054fee1a0f6d67bdb0cfc69cdbcdc36450b12e6d4085b70ec2e02",
        "d89e05c15d9dbbcb07d57c362a299a6217dd70305a01599139590ef7d8ec2f15310bc0ff6726336711155868874ab48e",
        "08c9bcf367e6096a3ba7ca8485ae67bb2bf894fe72f36e3cf1361d5f3af54fa5d182e6ad7f520e511f6c3e2b8c68059b6bbd41fbabd9831f79217e1319cde05b",
        NULL,
        "2c194d0376411dc0b8485d3abe2a4b6b",
        "2102d1d94bd58ebf5aa25c305bb783ad",
        "03dd8807a93175fb062dfb55dc7d359c",
        "a49b2446a02c645bf419f995b67091253a04a259",
        "c97ca9a559850ce97a04a96def6d99a9e0e0e2ab14e6b8df265fc0b3",
        "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1",
        "09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712fcc7c71a557e2db966c3e9fa91746039",
        "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909",
        "6f3fa39b6b503c384f919a49a7aa5c2c08bdfb45",
    }
};

static size_t dvLen = sizeof(dv) / sizeof(DigestVector);

static char * testString(char *format, CCDigestAlgorithm alg) {
    static char thestring[80];
    sprintf(thestring, format, digestName(alg));
    return thestring;
}

static int testOriginalOneShotDigest(CCDigestAlgorithm alg, char *input, byteBuffer expected) {
    byteBuffer computedMD = mallocDigestByteBuffer(alg);
    int status = 0;
    CC_LONG inputLen = (input) ? (CC_LONG) strlen(input): 0;
    unsigned char* p;
    switch(alg) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

        case kCCDigestMD2:		p=CC_MD2(input, inputLen, computedMD->bytes); break;
        case kCCDigestMD4:		p=CC_MD4(input, inputLen, computedMD->bytes); break;
        case kCCDigestMD5:		p=CC_MD5(input, inputLen, computedMD->bytes); break;
        case kCCDigestSHA1:     p=CC_SHA1(input, inputLen, computedMD->bytes); break;
#pragma clang diagnostic pop
        case kCCDigestSHA224:	p=CC_SHA224(input, inputLen, computedMD->bytes); break;
        case kCCDigestSHA256:	p=CC_SHA256(input, inputLen, computedMD->bytes); break;
        case kCCDigestSHA384:	p=CC_SHA384(input, inputLen, computedMD->bytes); break;
        case kCCDigestSHA512:	p=CC_SHA512(input, inputLen, computedMD->bytes); break;
        default: {
            free(computedMD);
            return 1;
        } break;
    }
    is(p,computedMD->bytes, "Return value");
    ok(status = expectedEqualsComputed(testString("Original OneShot Digest %s", alg), expected, computedMD), "Digest is as expected");
    free(computedMD);
    return status;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static byteBuffer cc_md2_discrete_legacy(char *input, CC_LONG len, void *out) {
    CC_MD2_CTX ctx;
    ok(CC_MD2_Init(&ctx) == 1, "Old Hash init should result in 1\n");
    ok(CC_MD2_Update(&ctx, input, len) == 1, "Old Hash update should result in 1\n");
    byteBuffer retval = mallocByteBuffer(CC_MD2_BLOCK_LONG);
    memcpy(retval->bytes, ctx.state, CC_MD2_BLOCK_LONG);
    ok(CC_MD2_Final(out, &ctx) == 1, "Old Hash final should result in 1\n");
    return retval;
}


static byteBuffer cc_md4_discrete_legacy(char *input, CC_LONG len, void *out) {
    CC_MD4_CTX ctx;
    ok(CC_MD4_Init(&ctx) == 1, "Old Hash init should result in 1\n");
    ok(CC_MD4_Update(&ctx, input, len) == 1, "Old Hash update should result in 1\n");
    byteBuffer retval = mallocByteBuffer(CC_MD4_DIGEST_LENGTH);
    memcpy(retval->bytes, &ctx.A, sizeof(CC_LONG));
    memcpy(retval->bytes + sizeof(CC_LONG), &ctx.B, sizeof(CC_LONG));
    memcpy(retval->bytes + 2*sizeof(CC_LONG), &ctx.C, sizeof(CC_LONG));
    memcpy(retval->bytes + 3*sizeof(CC_LONG), &ctx.D, sizeof(CC_LONG));
    ok(CC_MD4_Final(out, &ctx) == 1, "Old Hash final should result in 1\n");
    return retval;
}

static byteBuffer cc_md5_discrete_legacy(char *input, CC_LONG len, void *out) {
    CC_MD5_CTX ctx;
    ok(CC_MD5_Init(&ctx) == 1, "Old Hash init should result in 1\n");
    ok(CC_MD5_Update(&ctx, input, len) == 1, "Old Hash update should result in 1\n");
    byteBuffer retval = mallocByteBuffer(CC_MD5_DIGEST_LENGTH);
    memcpy(retval->bytes, &ctx.A, sizeof(CC_LONG));
    memcpy(retval->bytes + sizeof(CC_LONG), &ctx.B, sizeof(CC_LONG));
    memcpy(retval->bytes + 2*sizeof(CC_LONG), &ctx.C, sizeof(CC_LONG));
    memcpy(retval->bytes + 3*sizeof(CC_LONG), &ctx.D, sizeof(CC_LONG));
    ok(CC_MD5_Final(out, &ctx) == 1, "Old Hash final should result in 1\n");
    return retval;
}

static byteBuffer cc_sha1_discrete_legacy(char *input, CC_LONG len, void *out) {
    CC_SHA1_CTX ctx;
    ok(CC_SHA1_Init(&ctx) == 1, "Old Hash init should result in 1\n");
    ok(CC_SHA1_Update(&ctx, input, len) == 1, "Old Hash update should result in 1\n");
    byteBuffer retval = mallocByteBuffer(CC_SHA1_DIGEST_LENGTH);
    memcpy(retval->bytes, &ctx.h0, sizeof(CC_LONG));
    memcpy(retval->bytes + sizeof(CC_LONG), &ctx.h1, sizeof(CC_LONG));
    memcpy(retval->bytes + 2*sizeof(CC_LONG), &ctx.h2, sizeof(CC_LONG));
    memcpy(retval->bytes + 3*sizeof(CC_LONG), &ctx.h3, sizeof(CC_LONG));
    memcpy(retval->bytes + 4*sizeof(CC_LONG), &ctx.h4, sizeof(CC_LONG));
    ok(CC_SHA1_Final(out, &ctx) == 1, "Old Hash final should result in 1\n");
    return retval;
}
#pragma clang diagnostic pop

static byteBuffer cc_sha224_discrete_legacy(char *input, CC_LONG len, void *out) {
    CC_SHA256_CTX ctx;
    ok(CC_SHA224_Init(&ctx) == 1, "Old Hash init should result in 1\n");
    ok(CC_SHA224_Update(&ctx, input, len) == 1, "Old Hash update should result in 1\n");
    byteBuffer retval = mallocByteBuffer(CC_SHA224_DIGEST_LENGTH);
    memcpy(retval->bytes, ctx.hash, CC_SHA224_DIGEST_LENGTH);
    ok(CC_SHA224_Final(out, &ctx) == 1, "Old Hash final should result in 1\n");
    return retval;
}

static byteBuffer cc_sha256_discrete_legacy(char *input, CC_LONG len, void *out) {
    CC_SHA256_CTX ctx;
    ok(CC_SHA256_Init(&ctx) == 1, "Old Hash init should result in 1\n");
    ok(CC_SHA256_Update(&ctx, input, len) == 1, "Old Hash update should result in 1\n");
    byteBuffer retval = mallocByteBuffer(CC_SHA256_DIGEST_LENGTH);
    memcpy(retval->bytes, ctx.hash, CC_SHA256_DIGEST_LENGTH);
    ok(CC_SHA256_Final(out, &ctx) == 1, "Old Hash final should result in 1\n");
    return retval;
}

static byteBuffer cc_sha384_discrete_legacy(char *input, CC_LONG len, void *out) {
    CC_SHA512_CTX ctx;
    ok(CC_SHA384_Init(&ctx) == 1, "Old Hash init should result in 1\n");
    ok(CC_SHA384_Update(&ctx, input, len) == 1, "Old Hash update should result in 1\n");
    byteBuffer retval = mallocByteBuffer(CC_SHA384_DIGEST_LENGTH);
    memcpy(retval->bytes, ctx.hash, CC_SHA384_DIGEST_LENGTH);
    ok(CC_SHA384_Final(out, &ctx) == 1, "Old Hash final should result in 1\n");
    return retval;
}

static byteBuffer cc_sha512_discrete_legacy(char *input, CC_LONG len, void *out) {
    CC_SHA512_CTX ctx;
    ok(CC_SHA512_Init(&ctx) == 1, "Old Hash init should result in 1\n");
    ok(CC_SHA512_Update(&ctx, input, len) == 1, "Old Hash update should result in 1\n");
    byteBuffer retval = mallocByteBuffer(CC_SHA512_DIGEST_LENGTH);
    memcpy(retval->bytes, ctx.hash, CC_SHA512_DIGEST_LENGTH);
    ok(CC_SHA512_Final(out, &ctx) == 1, "Old Hash final should result in 1\n");
    return retval;
}




static int
testOriginalDiscreteDigest(CCDigestAlgorithm alg, char *input, byteBuffer expected, byteBuffer expectedIntermediate) {
    byteBuffer computedMD = mallocDigestByteBuffer(alg);
    int status = 0;
    CC_LONG inputLen = (input) ? (CC_LONG) strlen(input): 0;
    byteBuffer comp_intr;

    switch(alg) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        case kCCDigestMD2:		comp_intr = cc_md2_discrete_legacy(input, inputLen, computedMD->bytes); break;
        case kCCDigestMD4:		comp_intr = cc_md4_discrete_legacy(input, inputLen, computedMD->bytes); break;
        case kCCDigestMD5:		comp_intr = cc_md5_discrete_legacy(input, inputLen, computedMD->bytes); break;
        case kCCDigestSHA1:     comp_intr = cc_sha1_discrete_legacy(input, inputLen, computedMD->bytes); break;
#pragma clang diagnostic pop
        case kCCDigestSHA224:	comp_intr = cc_sha224_discrete_legacy(input, inputLen, computedMD->bytes); break;
        case kCCDigestSHA256:	comp_intr = cc_sha256_discrete_legacy(input, inputLen, computedMD->bytes); break;
        case kCCDigestSHA384:	comp_intr = cc_sha384_discrete_legacy(input, inputLen, computedMD->bytes); break;
        case kCCDigestSHA512:	comp_intr = cc_sha512_discrete_legacy(input, inputLen, computedMD->bytes); break;
        default: {
            free(computedMD);
            return 1;
        } break;
    }
    ok(status = expectedEqualsComputed(testString("Original Discrete Digest %s", alg), expected, computedMD), "Digest is as expected");
    ok(status = expectedEqualsComputed(testString("Original Discrete Digest Intermediate %s", alg), expectedIntermediate, comp_intr), "Intermediate State is as expected");
    free(computedMD);
    free(comp_intr);
    return status;
}

static int testNewOneShotDigest(CCDigestAlgorithm alg, char *input, byteBuffer expected) {
    byteBuffer computedMD = mallocDigestByteBuffer(alg);
    int status = 0;
    size_t inputLen = (input) ? strlen(input): 0;
    
    is(CCDigest(alg, (const uint8_t *) NULL, inputLen,  computedMD->bytes),(inputLen==0)?0:kCCParamError, "NULL data return value");
    is(CCDigest(alg, (const uint8_t *) input, inputLen, NULL),kCCParamError, "NULL output return value");
    is(CCDigest(alg, (const uint8_t *) input, inputLen, computedMD->bytes),0, "Digest return value");
    ok(status = expectedEqualsComputed(testString("New OneShot Digest %s", alg), expected, computedMD), "Digest is as expected");
    free(computedMD);
    return status;
}

static int testNewDiscreteDigest(CCDigestAlgorithm alg, char *input, byteBuffer expected) {
    byteBuffer computedMD = mallocDigestByteBuffer(alg);
    byteBuffer computedMD2 = mallocDigestByteBuffer(alg);
    int status = 0;
    size_t inputLen = (input) ? strlen(input): 0;
    CCDigestRef d;

    ok((d = CCDigestCreate(alg)) != NULL, "Got CCDigestRef from discrete new call");
    if(!d) goto out;
    size_t fromAlg = CCDigestGetOutputSize(alg);
    size_t fromRef = CCDigestGetOutputSizeFromRef(d);
    size_t fromOldRoutine = CCDigestOutputSize(d);
    ok(fromAlg == fromRef, "Size is the same from ref or alg");
    ok(fromAlg == fromOldRoutine, "Size is the same from ref or alg");

    int retval;
    ok((retval = CCDigestUpdate(d, input, inputLen)) == kCCSuccess, "Update Call Succeeded");
    if(retval) goto out;
    ok((retval = CCDigestFinal(d, computedMD->bytes)) == kCCSuccess, "Final Call Succeeded");
    if(retval) goto out;

    CCDigestReset(d);
    ok((retval = CCDigestUpdate(d, input, inputLen)) == kCCSuccess, "Update Call Succeeded");
    if(retval) goto out;
    ok((retval = CCDigestFinal(d, computedMD2->bytes)) == kCCSuccess, "Final Call Succeeded");
    if(retval) goto out;
    
    CCDigestDestroy(d);
    ok(status = expectedEqualsComputed(testString("New OneShot Digest %s", alg), expected, computedMD), "Digest is as expected");
    ok(status = expectedEqualsComputed(testString("New OneShot Digest %s", alg), expected, computedMD2), "Digest is as expected");
out:
    free(computedMD);
    free(computedMD2);
    return status;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static int rfc1321Test(CCDigestAlgorithm alg, char *input, byteBuffer expected) {
    if(alg != kCCDigestMD5) return 1;
    CC_LONG inputLen = (input) ? (CC_LONG) strlen(input): 0;
    byteBuffer computedMD = mallocDigestByteBuffer(alg);
    int status = 0;
    MD5_CTX ctx;
    MD5Init(&ctx);
    MD5Update(&ctx, input, inputLen);
    MD5Final(computedMD->bytes, &ctx);
    ok(status = expectedEqualsComputed("Legacy MD5-1321", expected, computedMD), "Digest is as expected");
    free(computedMD);
    return status;
}
#pragma clang diagnostic pop

static int testAllDigests(CCDigestAlgorithm alg, char *input, byteBuffer expected, byteBuffer expectedIntermediate) {
    int status = 0;
    
    ok(status = testOriginalOneShotDigest(alg, input, expected), "Test Original One Shot Digest");
    ok(status &= testOriginalDiscreteDigest(alg, input, expected, expectedIntermediate), "Test Original Discrete version of Digest");
    ok(status &= testNewOneShotDigest(alg, input, expected), "Test New One Shot Digest");
    ok(status &= testNewDiscreteDigest(alg, input, expected), "Test New Discrete Digest");
    ok(status &= rfc1321Test(alg, input, expected), "Legacy MD5-1321 Digest");
    return status;
}

static int testDigests(DigestVector *dv) {
    int status = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    byteBuffer expectedMD = hexStringToBytesIfNotNULL(dv->md2str);
    byteBuffer expectedIntermediate = hexStringToBytesIfNotNULL(dv->md2intr);
    ok(status = testAllDigests(kCCDigestMD2, dv->input, expectedMD, expectedIntermediate), "Testing all MD2 Implementations");
    free(expectedMD);
    free(expectedIntermediate);
    
    expectedMD = hexStringToBytesIfNotNULL(dv->md4str);
    expectedIntermediate = hexStringToBytesIfNotNULL(dv->md4intr);
    ok(status &= testAllDigests(kCCDigestMD4, dv->input, expectedMD, expectedIntermediate), "Testing all MD4 Implementations");
    free(expectedMD);
    free(expectedIntermediate);
    
    expectedMD = hexStringToBytesIfNotNULL(dv->md5str);
    expectedIntermediate = hexStringToBytesIfNotNULL(dv->md5intr);
    ok(status &= testAllDigests(kCCDigestMD5, dv->input, expectedMD, expectedIntermediate), "Testing all MD5 Implementations");
    free(expectedMD);
    free(expectedIntermediate);
    
    expectedMD = hexStringToBytesIfNotNULL(dv->sha1str);
    expectedIntermediate = hexStringToBytesIfNotNULL(dv->sha1intr);
    ok(status &= testAllDigests(kCCDigestSHA1, dv->input, expectedMD, expectedIntermediate), "Testing all SHA1 Implementations");
    free(expectedMD);
    free(expectedIntermediate);
    
    expectedMD = hexStringToBytesIfNotNULL(dv->rmd160str);
    expectedIntermediate = hexStringToBytesIfNotNULL(dv->rmd160intr);
    ok(status &= testAllDigests(kCCDigestRMD160, dv->input, expectedMD, expectedIntermediate), "Testing all RMD160 Implementations");
    free(expectedMD);
    free(expectedIntermediate);
#pragma clang diagnostic pop
    
    expectedMD = hexStringToBytesIfNotNULL(dv->sha224str);
    expectedIntermediate = hexStringToBytesIfNotNULL(dv->sha224intr);
    ok(status &= testAllDigests(kCCDigestSHA224, dv->input, expectedMD, expectedIntermediate), "Testing all SHA224 Implementations");
    free(expectedMD);
    free(expectedIntermediate);
    
    expectedMD = hexStringToBytesIfNotNULL(dv->sha256str);
    expectedIntermediate = hexStringToBytesIfNotNULL(dv->sha256intr);
    ok(status &= testAllDigests(kCCDigestSHA256, dv->input, expectedMD, expectedIntermediate), "Testing all SHA256 Implementations");
    free(expectedMD);
    free(expectedIntermediate);
    
    expectedMD = hexStringToBytesIfNotNULL(dv->sha384str);
    expectedIntermediate = hexStringToBytesIfNotNULL(dv->sha384intr);
    ok(status &= testAllDigests(kCCDigestSHA384, dv->input, expectedMD, expectedIntermediate), "Testing all SHA384 Implementations");
    free(expectedMD);
    free(expectedIntermediate);
    
    expectedMD = hexStringToBytesIfNotNULL(dv->sha512str);
    expectedIntermediate = hexStringToBytesIfNotNULL(dv->sha512intr);
    ok(status &= testAllDigests(kCCDigestSHA512, dv->input, expectedMD, expectedIntermediate), "Testing all SHA512 Implementations");
    free(expectedMD);
    free(expectedIntermediate);

    return status;
}

static size_t testsPerVector = 229;

int CommonDigest(int __unused argc, char *const * __unused argv) {

	plan_tests((int) (dvLen*testsPerVector+5));
    is(CC_SHA256(NULL, 1, (unsigned char *)1),NULL, "NULL data");
    is(CC_SHA256(NULL, 0, NULL),NULL, "NULL output");
    is(CCDigestGetOutputSize(kCCDigestSHA512),(size_t)64, "Out of bound by one");
    is(CCDigestGetOutputSize(CC_MAX_N_DIGESTS),(size_t)kCCUnimplemented, "Out of bound by one");
    is(CCDigestGetOutputSize(CC_MAX_N_DIGESTS+500),(size_t)kCCUnimplemented, "Out of bound by a lot");
    
    for(size_t testcase = 0; testcase < dvLen; testcase++) {
        ok(testDigests(&dv[testcase]), "Testcase %d", testcase);
    }
    return 0;
}
