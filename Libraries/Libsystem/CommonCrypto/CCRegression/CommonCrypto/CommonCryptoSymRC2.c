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

/*
 *  CCRC2KAT.c
 *  CCRegressions
 *
 */

#include <stdio.h>
#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

#if (CCSYMRC2 == 0)
entryPoint(CommonCryptoSymRC2,"Common Crypto RC2 Test")
#else


#ifdef WEIRDCASE
static int kTestTestCount = 12;
#else
static int kTestTestCount = 8;
#endif


int CommonCryptoSymRC2(int __unused argc, char *const * __unused argv) {
	char *keyStr;
	char *iv;
	char *plainText;
	char *cipherText;
    CCAlgorithm alg;
    CCOptions options;
	int retval;
    int rkeylen, ekeylenBits;
    char printString[128];

    alg = kCCAlgorithmRC2;
    iv = NULL;
    options = 0;
	plan_tests(kTestTestCount);

    rkeylen = 8;
    ekeylenBits = 63;
    keyStr =    "0000000000000000";
    plainText = "0000000000000000";
    cipherText = "ebb773f993278eff";
    retval = CCCryptTestCase(keyStr, iv, alg, options, cipherText, plainText, true);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) One-Shot", rkeylen, ekeylenBits);
    ok(retval == 0, printString);
    retval = CCMultiCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) Multi", rkeylen, ekeylenBits);
    ok(retval == 0, printString);

    rkeylen = 8;
    ekeylenBits = 64;
    keyStr =    "ffffffffffffffff";
    plainText = "ffffffffffffffff";
    cipherText = "278b27e42e2f0d49";
    retval = CCCryptTestCase(keyStr, iv, alg, options, cipherText, plainText, true);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) One-Shot", rkeylen, ekeylenBits);
    ok(retval == 0, printString);
    retval = CCMultiCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) Multi", rkeylen, ekeylenBits);
    ok(retval == 0, printString);

    rkeylen = 8;
    ekeylenBits = 64;
    keyStr =    "3000000000000000";
    plainText = "1000000000000001";
    cipherText = "30649edf9be7d2c2";
    retval = CCCryptTestCase(keyStr, iv, alg, options, cipherText, plainText, true);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) One-Shot", rkeylen, ekeylenBits);
    ok(retval == 0, printString);
    retval = CCMultiCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) Multi", rkeylen, ekeylenBits);
    ok(retval == 0, printString);

#ifdef WEIRDCASE
    rkeylen = 1;
    ekeylenBits = 64;
    keyStr =    "88";
    plainText = "0000000000000000";
    cipherText = "61a8a244adacccf0";
    retval = CCCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) One-Shot", rkeylen, ekeylenBits);
    ok(retval == 0, printString);
    retval = CCMultiCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) Multi", rkeylen, ekeylenBits);
    ok(retval == 0, printString);

    rkeylen = 7;
    ekeylenBits = 64;
    keyStr = "88bca90e90875a";
    plainText = "0000000000000000";
    cipherText = "6ccf4308974c267f";
    retval = CCCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) One-Shot", rkeylen, ekeylenBits);
    ok(retval == 0, printString);
    retval = CCMultiCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) Multi", rkeylen, ekeylenBits);
    ok(retval == 0, printString);
    
    rkeylen = 16;
    ekeylenBits = 64;
    keyStr = "88bca90e90875a7f0f79c384627bafb2";
    plainText = "0000000000000000";
    cipherText = "1a807d272bbe5db1";
    retval = CCCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) One-Shot", rkeylen, ekeylenBits);
    ok(retval == 0, printString);
    retval = CCMultiCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) Multi", rkeylen, ekeylenBits);
    ok(retval == 0, printString);
#endif

    rkeylen = 16;
    ekeylenBits = 128;
    keyStr = "88bca90e90875a7f0f79c384627bafb2";
    plainText = "0000000000000000";
    cipherText = "2269552ab0f85ca6";
    retval = CCCryptTestCase(keyStr, iv, alg, options, cipherText, plainText, true);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) One-Shot", rkeylen, ekeylenBits);
    ok(retval == 0, printString);
    retval = CCMultiCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    sprintf(printString, "RC2 %d byte Key (effective %d bits) Multi", rkeylen, ekeylenBits);
    ok(retval == 0, printString);

    
    return 0;
}
#endif
