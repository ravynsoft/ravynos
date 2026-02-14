/*
 * Copyright (c) 2012 Apple Inc. All Rights Reserved.
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

#if (CCBLOWFISH == 0)
entryPoint(CommonCryptoBlowfish,"CommonCrypto Blowfish Testing")
#else

static int kTestTestCount = 1;

int CommonCryptoBlowfish(int __unused argc, char *const * __unused argv)
{
    char *keyStr;
	char *iv = NULL;
	char *plainText;
	char *cipherText;
    CCMode mode;
    CCAlgorithm alg;
    CCPadding padding;
    int retval, accum = 0;
	keyStr 	   = "FEDCBA9876543210FEDCBA9876543210";
    plainText =  "31323334353637383132333435363738313233343536373831323334353637383132333435363738313233343536373831323334353637383132333435363738";
    cipherText = "695347477477FC1E695347477477FC1E695347477477FC1E695347477477FC1E695347477477FC1E695347477477FC1E695347477477FC1E695347477477FC1E";
    mode 	   = kCCModeECB;
    alg		= kCCAlgorithmBlowfish;
    padding = ccNoPadding;
    
	plan_tests(kTestTestCount);
    
    retval = CCModeTestCase(keyStr, iv, mode, alg, padding, cipherText, plainText);
    
    ok(retval == 0, "Blowfish Test 1");
    accum += retval;
    return accum;
}
#endif
