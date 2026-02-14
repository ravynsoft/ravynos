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

#if (CCSYMCFB == 0)
entryPoint(CommonCryptoSymCFB,"CommonCrypto Symmetric CFB Testing")
#else
static int kTestTestCount = 7;

int CommonCryptoSymCFB(int __unused argc, char *const * __unused argv)
{
	char *keyStr;
	char *iv;
	char *plainText;
	char *cipherText;
    CCMode mode;
    CCAlgorithm alg;
    CCPadding padding;
    int retval, accum = 0;
    
	keyStr 	   = "00000000000000000000000000000000";
    mode 	   = kCCModeCFB;
    alg		= kCCAlgorithmAES128;
    padding = ccNoPadding;
    
	plan_tests(kTestTestCount);
    
	plainText  = "00000000000000000000000000000000";
	iv         = "80000000000000000000000000000000";
	cipherText = "3ad78e726c1ec02b7ebfe92b23d9ec34";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "CFB Test 1");
    accum += retval;
    
	plainText  = "00000000000000000000000000000000";
	iv         = "c0000000000000000000000000000000";
	cipherText = "aae5939c8efdf2f04e60b9fe7117b2c2";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "CFB Test 2");
    accum += retval;
    
	plainText  = "00000000000000000000000000000000";
	iv         = "ffffffffffffffffffffffc000000000";
	cipherText = "90684a2ac55fe1ec2b8ebd5622520b73";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "CFB Test 3");
    accum += retval;
    
	keyStr 	   = "f36f40aeb3e20c19440081919df4ecb9";
	plainText  = "5552ee4fbf25859cdfecf34742d640855c55a00a1c6aa571c322b4ddf561b6e110de0f9dbe6fd42ac687383928fc48f6680ed9332aa6bec2ffdb3e227fbac55f9847d93325bf5071c220c0a3dfeb38f214292d47b4acb7b0a597fe056f21eecb";
	iv         = "cf75d0cae8dbce85e0bc0e58eb4e82c0";
	cipherText = "d579c0e622047e0efef9ddacc94be36450d09366e38ea9e4ed3c2e39923f580fdc436a874e34fb39330f8a8a7bf1f68d8a93644a2b9d1fed260eb6e7c8ac874d616ec02b57d08f043aae552fc52bf11eb4a4ed5918ff81738b3ea3ce244c9cd1";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "CFB Test 4");
    accum += retval;
    
	keyStr 	   = "02f5f980a3c6af2984c9eed684f5224f";
	plainText  = "cb9056b3c195cef43da2634271f2f34e556868ac41d0ea35bb6854aa8e9fe49a1c1c3685f4027341a773c69dd8530f45934edce5e01c95919703f665759ec26870deb6b6484ebcdf5f9b628aca538a5ea8ed6322b20a982214fac7d57c8234be70dcbf90e40013950c2221506f7ccd623d4de6aae2225cb0db54ad44c8d2a162c3203759f1a0df2c5611e939ee553e18f9774ec696ec4903931992f88416711d";
	iv         = "7844c96a2c65e9c334573b9894d91046";
	cipherText = "227d8e80650e60c1c9e9ab2f8fe8bc24185725f1f1b8e326b73c04501e91995d5a7be17d0c74ce1cd2d5a3d917738dea486fb523297aea2badc6f8766ffdfcc6e8efd640a8d5d1f55d1e4fe4f03bb9f66956eaa9864ee760d1ad3e5faaae6da36476c55d1f25d7c064b9c518ce5b3f42ecdb1c3ef57c7b3fa3ef188218d0d0c417056935bcc76a90ff277f9e698ff9a599ebaa6c2723288c9f0817694e400ce6";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "CFB Test 5");
    accum += retval;
    
    keyStr 	   = "e0000000000000000000000000000000";
    iv         = NULL;
    plainText  = "00000000000000000000000000000000";
    cipherText = "72a1da770f5d7ac4c9ef94d822affd97";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "CFB Test NULLIV");
    accum += retval;
    
    // Blowfish vector that was failing for Jim
    
    alg = kCCAlgorithmBlowfish;
    mode 	   = kCCModeCFB;
    padding = ccNoPadding;
    keyStr = "0123456789ABCDEFF0E1D2C3B4A59687";
    iv = "FEDCBA9876543210";
    plainText =  "37363534333231204E6F77206973207468652074696D6520666F722000";
    cipherText = "E73214A2822139CAF26ECF6D2EB9E76E3DA3DE04D1517200519D57A6C3";
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    ok(retval == 0, "CFB-blowfish vector 1");
    accum |= retval;


    
    return accum != 0;
}
#endif
