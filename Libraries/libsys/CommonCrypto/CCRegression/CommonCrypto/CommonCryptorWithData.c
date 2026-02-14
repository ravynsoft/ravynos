//
//  CommonCryptorWithData.c
//  CommonCrypto
//
//  Created by Richard Murphy on 8/8/12.
//  Copyright (c) 2012 Platform Security. All rights reserved.
//

#include <stdio.h>
#include <CommonCrypto/CommonCryptor.h>
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

#if (CCWITHDATA == 0)
entryPoint(CommonCryptoWithData,"CommonCrypto With Data Testing")
#else
#define AES_KEYST_SIZE    (kCCContextSizeAES128 + 8)

static int kTestTestCount = 1;

int CommonCryptoWithData(int __unused argc, char *const * __unused argv)
{
    CCCryptorStatus retval;
    CCCryptorRef cryptor;
    unsigned char data[AES_KEYST_SIZE];

    
	plan_tests(kTestTestCount);
    
    byteBuffer key = hexStringToBytes("2b7e151628aed2a6abf7158809cf4f3c");
    retval = CCCryptorCreateFromData(kCCEncrypt, kCCAlgorithmAES128,
                                     kCCOptionECBMode, key->bytes, key->len, NULL,
                                     data, AES_KEYST_SIZE, &cryptor, NULL);

    CCCryptorRelease(cryptor);
    ok(retval == kCCSuccess, "Cryptor was created");
    free(key);
    return 0;
}
#endif
