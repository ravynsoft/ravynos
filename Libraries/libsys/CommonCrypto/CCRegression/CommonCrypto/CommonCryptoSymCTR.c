/*
 * Copyright (c) 2010 Apple Inc. All Rights Reserved.
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

#include <stdio.h>
#include <CommonCrypto/CommonCryptor.h>
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

#if (CCSYMCTR == 0)
entryPoint(CommonCryptoSymCTR,"CommonCrypto Symmetric CTR Testing")
#else
static int kTestTestCount = 43;

static CCCryptorStatus doCrypt(char *in, char *out, CCCryptorRef cryptor) {
    byteBuffer inbb = hexStringToBytes(in);
    byteBuffer outbb = hexStringToBytes(out);
    byteBuffer buf = mallocByteBuffer(inbb->len);
    CCCryptorStatus retval = CCCryptorUpdate(cryptor, inbb->bytes, inbb->len, buf->bytes, buf->len, &buf->len);
    if(!retval) {
        is(outbb->len, buf->len, "crypt results same length");
        ok_memcmp(outbb->bytes, buf->bytes,buf->len, "crypt results are equal");
    }
    free(inbb);
    free(outbb);
    free(buf);
    return retval;
}

#define keystr128    "000102030405060708090a0b0c0d0e0f"
#define keystr256    "000102030405060708090a0b0c0d0e0ff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"
#define keystr128_2  "2b7e151628aed2a6abf7158809cf4f3c"
#define ivstr128     "0f0e0d0c0b0a09080706050403020100"
#define ivstr128_2   "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"
#define ivstrff64    "0000000000000000ffffffffffffffff"
#define ivstrff128   "ffffffffffffffffffffffffffffffff"
#define ivstrff128_1 "fffffffffffffffeffffffffffffffff"
#define ivstrff128_2 "fffffffffffffffdffffffffffffffff"
#define zeroX1       "00"
#define zeroX16      "00000000000000000000000000000000"
#define zeroX32      "0000000000000000000000000000000000000000000000000000000000000000"
#define zeroX33      "000000000000000000000000000000000000000000000000000000000000000000"
#define aX21         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

typedef struct ccsymmetric_test_t {
    char *keyStr;
    char *init_ivStr;
    char *ptStr;
    char *ctStr;
} ccsymmetric_test_vector;

ccsymmetric_test_vector aes_ctr_vectors[] = {
    { keystr256, ivstrff128, aX21, "b52a889d57fff0809d8dce586b8b1e3b7b0e69b14c"},
    { keystr256, ivstrff128_1, aX21, "79cf2b330d78a0937a94fcc7210778ec287335330b"},
    { keystr256, ivstrff128_2, aX21, "440a237cab106f75ae7544a9a8223242da63cb68ad"},
    { keystr128, ivstr128, zeroX1, "20"},
    { keystr128, ivstr128, zeroX16, "20a9f992b44c5be8041ffcdc6cae996a"},
    { keystr128, ivstr128, zeroX33, "20a9f992b44c5be8041ffcdc6cae996a47a6a4a5755e60446eb291ec4939015fbb"},
#if 0
    /* For reference / debug */
    /* Counter rolls over 32bit */
    { keystr128, ivstrff128, zeroX32, "3c441f32ce07822364d7a2990e50bb13dd94a22c83d419e0f9e7dcda9b8da9d4"},
    /* Counter rolls over 128bit */
    { keystr128, ivstrff128, zeroX32, "3c441f32ce07822364d7a2990e50bb13c6a13b37878f5b826f4f8162a1c8d879"},
#else
    /* corecrypto implementation: counter rolls over 64bit */
    { keystr128, ivstrff128, zeroX32, "3c441f32ce07822364d7a2990e50bb1325d4e948bd5e1296afc0bf87095a7248"},
#endif
    { keystr128, zeroX16,zeroX16, "c6a13b37878f5b826f4f8162a1c8d879"}, // IV is 00
    { keystr128, ivstrff64,zeroX32, "39a7ef0a0a5852a8bfd2032344bf9412c6a13b37878f5b826f4f8162a1c8d879"}, // Second block matches one block encrypted with IV 0
    { keystr128_2, ivstr128_2, "6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e5130c81c46a35ce411e5fbc1191a0a52eff69f2445df4f9b17ad2b417be66c3710", "874d6191b620e3261bef6864990db6ce9806f66b7970fdff8617187bb9fffdff5ae4df3edbd5d35e5b4f09020db03eab1e031dda2fbe03d1792170a0f3009cee"},
};

int CommonCryptoSymCTR(int __unused argc, char *const * __unused argv)
{
    CCCryptorStatus retval;
    CCCryptorRef cryptor;
    byteBuffer key;
    byteBuffer counter;
	plan_tests(kTestTestCount);

    {
        key = hexStringToBytes("2b7e151628aed2a6abf7158809cf4f3c");
        counter = hexStringToBytes("f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff");

        retval = CCCryptorCreateWithMode(kCCEncrypt, kCCModeCTR, kCCAlgorithmAES128, 
                                         ccNoPadding, counter->bytes, key->bytes, key->len, 
                                         NULL, 0, 0, kCCModeOptionCTR_BE, &cryptor);
                                         
        ok(retval == kCCSuccess, "CTR Mode Encrypt");

        retval = doCrypt("6bc1bee22e409f96e93d7e117393172a",
                         "874d6191b620e3261bef6864990db6ce",
                         cryptor);
       
        
        ok(retval == kCCSuccess, "CTR Mode Encrypt");

        retval = doCrypt("ae2d8a571e03ac9c9eb76fac45af8e51",
                         "9806f66b7970fdff8617187bb9fffdff",
                         cryptor);
        
        
        ok(retval == kCCSuccess, "CTR Mode Encrypt");

        retval = doCrypt("30c81c46a35ce411e5fbc1191a0a52ef",
                         "5ae4df3edbd5d35e5b4f09020db03eab",
                         cryptor);
        
        ok(retval == kCCSuccess, "CTR Mode Encrypt");

        retval = doCrypt("f69f2445df4f9b17ad2b417be66c3710",
                         "1e031dda2fbe03d1792170a0f3009cee",
                         cryptor);
        
        ok(retval == kCCSuccess, "CTR Mode Encrypt");
        free(key);
        free(counter);
        CCCryptorRelease(cryptor);
    }

    for (size_t i=0;i < sizeof(aes_ctr_vectors)/sizeof(aes_ctr_vectors[0]);i++) {
        key     = hexStringToBytes(aes_ctr_vectors[i].keyStr);
        counter = hexStringToBytes(aes_ctr_vectors[i].init_ivStr);

        retval = CCCryptorCreateWithMode(kCCEncrypt, kCCModeCTR, kCCAlgorithmAES128,
                                         ccNoPadding, counter->bytes, key->bytes, key->len,
                                         NULL, 0, 0, kCCModeOptionCTR_BE, &cryptor);
        ok(retval == kCCSuccess, "CTR Mode Encrypt");

        doCrypt(aes_ctr_vectors[i].ptStr,
                         aes_ctr_vectors[i].ctStr,
                         cryptor);
        free(key);
        free(counter);
        CCCryptorRelease(cryptor);
    }

    return 0;
}
#endif
