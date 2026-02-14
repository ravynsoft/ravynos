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
 *  ECBTest.c
 *  CommonCrypto
 */

#include <stdio.h>
#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

#if (CCSYMECB == 0)
entryPoint(CommonCryptoSymECB,"CommonCrypto Symmetric ECB Testing")
#else


static int kTestTestCount = 4;

int CommonCryptoSymECB(int __unused argc, char *const * __unused argv) {
	char *keyStr;
	char *iv;
	char *plainText;
	char *cipherText;
    CCAlgorithm alg;
    CCOptions options;
	int retval;
    int accum = 0;

	keyStr 	   = "000102030405060708090a0b0c0d0e0f";
    alg		   = kCCAlgorithmAES128;
    options    = kCCOptionECBMode;
    
	plan_tests(kTestTestCount);
    
    accum = (int) genRandomSize(1,10);
    
    iv = NULL;

	// 16
    plainText  = "0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a";
	cipherText = "d307b25d3abaf87c0053e8188152992a";
    retval = CCCryptTestCase(keyStr, iv, alg, options, cipherText, plainText, true);
    ok(retval == 0, "ECB with Padding 16 byte CCCrypt NULL IV");
    retval = CCMultiCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    ok(retval == 0, "ECB with Padding 16 byte Multiple Updates NULL IV");
    accum |= retval;

	// 32
	plainText  = "0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a";
	cipherText = "d307b25d3abaf87c0053e8188152992ad307b25d3abaf87c0053e8188152992a";
    retval = CCCryptTestCase(keyStr, iv, alg, options, cipherText, plainText, true);
    ok(retval == 0, "ECB 32 byte CCCrypt NULL IV");
    accum |= retval;
    retval = CCMultiCryptTestCase(keyStr, iv, alg, options, cipherText, plainText);
    ok(retval == 0, "ECB 32 byte Multiple Updates NULL IV");
    accum |= retval;

    return accum != 0;
}
#endif

