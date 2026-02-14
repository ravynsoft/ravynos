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
#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

#if (CCSYMOFB == 0)
entryPoint(CommonCryptoSymOFB,"CommonCrypto Symmetric OFB Testing")
#else
static int kTestTestCount = 5;

int CommonCryptoSymOFB(int __unused argc, char *const * __unused argv)
{
	char *keyStr;
	char *iv;
	char *plainText;
	char *cipherText;
    CCMode mode;
    CCAlgorithm alg;
    CCPadding padding;
    int retval, accum = 0;
    
	keyStr 	   = "000102030405060708090a0b0c0d0e0f";
	iv         = "0f0e0d0c0b0a09080706050403020100";
    mode 	   = kCCModeOFB;
    alg		= kCCAlgorithmAES128;
    padding = ccNoPadding;
    
	plan_tests(kTestTestCount);
    
	// 1
	plainText  = "0a";
	cipherText = "2a";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "OFB Mode single byte");
    accum += retval;
    
	// 15
	plainText  = "0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a";
	cipherText = "2aa3f398be4651e20e15f6d666a493";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "OFB Mode 15 byte");
    accum += retval;
    
	// 16
    plainText  = "0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a";
	cipherText = "2aa3f398be4651e20e15f6d666a49360";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "OFB Mode single byte");
    accum += retval;
    
	// from OFBVarTxt256e KAT test 1
	keyStr 	   = "0000000000000000000000000000000000000000000000000000000000000000";
	iv         = "80000000000000000000000000000000";
    mode 	   = kCCModeOFB;
    alg		= kCCAlgorithmAES128;
    padding = ccNoPadding;
    plainText  = "00000000000000000000000000000000";
	cipherText = "ddc6bf790c15760d8d9aeb6f9a75fd4e";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
	ok(retval == 0, "OFB Mode OFBVarTxt256e KAT test 1");
    accum += retval;
    
	// from OFBVarTxt256e KAT test 13
	keyStr 	   = "0000000000000000000000000000000000000000000000000000000000000000";
	iv         = "fffc0000000000000000000000000000";
    mode 	   = kCCModeOFB;
    alg		= kCCAlgorithmAES128;
    padding = ccNoPadding;
    plainText  = "00000000000000000000000000000000";
	cipherText = "dc8f0e4915fd81ba70a331310882f6da";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "OFB Mode OFBVarTxt256e KAT test 13");
    accum += retval;
    
    return accum != 0;
}
#endif
