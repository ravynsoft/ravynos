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
 *  CCSymOffset.c
 *  CommonCrypto
 */

#include <stdio.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonCryptorSPI.h>
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

#if (CCSYMOFFSET == 0)
entryPoint(CommonCryptoSymOffset,"CommonCrypto Symmetric Unaligned Testing")
#else



#define ILIKEEMDISBIG 4096
#define ALITTLEONTHESIDE 5


static int kTestTestCount = ALITTLEONTHESIDE * 3;


int CommonCryptoSymOffset(int __unused argc, char *const * __unused argv) {
    int accum = 0;
    uint8_t iLikeBigBuffs[ILIKEEMDISBIG];
    uint8_t andICannotLie[ILIKEEMDISBIG];
    uint8_t iLikeEmRoundandBig[ILIKEEMDISBIG];
    int i;
    size_t moved;
    CCCryptorStatus retval;
    byteBuffer key = hexStringToBytes("010203040506070809000a0b0c0d0e0f");
    
    plan_tests(kTestTestCount);

    for(i=0; i<ALITTLEONTHESIDE; i++) {
        retval = CCCrypt(kCCEncrypt, kCCAlgorithmAES128, 0, key->bytes, key->len, NULL, iLikeBigBuffs+i, ILIKEEMDISBIG-16, andICannotLie+i, ILIKEEMDISBIG, &moved);
        ok(retval == 0, "Encrypt worked");
        retval = CCCrypt(kCCDecrypt, kCCAlgorithmAES128, 0, key->bytes, key->len, NULL, andICannotLie+i, moved, iLikeEmRoundandBig+i, ILIKEEMDISBIG, &moved);
        ok(retval == 0, "Decrypt worked");
        if(moved != (ILIKEEMDISBIG-16))
            retval = 99;
        else if(memcmp(iLikeBigBuffs+i, iLikeEmRoundandBig+i, moved))
            retval = 999;
        ok(retval == 0, "Encrypt/Decrypt Cycle");
        accum += retval;
    }
    free(key);
    return accum != 0;
}
#endif

