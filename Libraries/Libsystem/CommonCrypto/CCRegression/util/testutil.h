//
//  testutil.h
//  CommonCrypto
//
//  Created by Richard Murphy on 1/21/14.
//  Copyright (c) 2014 Apple Inc. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "testmore.h"
#include "testbyteBuffer.h"
#define COMMON_DIGEST_FOR_RFC_1321

#include <CommonCrypto/CommonDigestSPI.h>
#include <CommonCrypto/CommonHMAC.h>
#include <CommonCrypto/CommonKeyDerivation.h>

#ifndef CommonCrypto_testutil_h
#define CommonCrypto_testutil_h

int expectedEqualsComputed(char *label, byteBuffer expected, byteBuffer computed);
char *digestName(CCDigestAlgorithm digestSelector);

#define HMAC_UNIMP 99
CCHmacAlgorithm digestID2HMacID(CCDigestAlgorithm digestSelector);
CCPseudoRandomAlgorithm digestID2PRF(CCDigestAlgorithm digestSelector);



static inline byteBuffer mallocDigestByteBuffer(CCDigestAlgorithm alg) {
    return mallocByteBuffer(CCDigestGetOutputSize(alg));
}

#endif
