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
#include <stdint.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonCryptorSPI.h>
#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "capabilities.h"
#include "testmore.h"

#if (CCPADCTS == 0)
entryPoint(CommonCryptoCTSPadding,"CommonCrypto CTS Padding Testing")

#else

static int kTestTestCount = 3;

int CommonCryptoCTSPadding(int __unused argc, char *const * __unused argv)
{
	char *keyStr;
	char *iv;
	char *plainText;
	char *cipherText;
    CCMode mode;
    CCAlgorithm alg;
    CCPadding padding;
    int retval, accum = 0;
    char *test;
    int verbose = 0;
    
	keyStr 	   = "636869636b656e207465726979616b69";
	iv         = "0f0e0d0c0b0a09080706050403020100";
    mode 	   = kCCModeCBC;
    alg		= kCCAlgorithmAES128;
    padding = ccCBCCTS3;

	plan_tests(kTestTestCount);

    test = "CTS3 Test - Length 64";
	plainText  = "0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a";
	cipherText = "e22abba9d2a201b18dc2f57e04aba21a16e0ed6358164c59ca64d204f33247ee04b1c432a7a71395b36d820e2c3de4ee2dc88b70f6ae0243d2dbcd6822a10586";
    if(verbose) diag(test);
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, test);
    accum += retval;
    
    test = "CTS3 Test - Length 63";
	plainText  = "0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a";
	cipherText = "e22abba9d2a201b18dc2f57e04aba21a16e0ed6358164c59ca64d204f33247ee950b6576660739916d058623d688e27e2dc88b70f6ae0243d2dbcd6822a105";
    if(verbose) diag(test);
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, test);
    accum += retval;
    
    test = "CTS3 Test - Length 57";
	plainText  = "0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a";
	cipherText = "e22abba9d2a201b18dc2f57e04aba21a16e0ed6358164c59ca64d204f33247ee751002ef7a0f9d915d15346571eee7aa2dc88b70f6ae0243d2db";
    if(verbose) diag(test);
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, test);
    accum += retval;
    
    
    return accum != 0;
}
#endif
