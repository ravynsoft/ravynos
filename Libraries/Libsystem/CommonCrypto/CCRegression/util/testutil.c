//
//  testutil.c
//  CommonCrypto
//
//  Created by Richard Murphy on 1/21/14.
//  Copyright (c) 2014 Apple Inc. All rights reserved.
//

#include <stdio.h>
#include "testutil.h"

int expectedEqualsComputed(char *label, byteBuffer expected, byteBuffer computed) {
    int retval;
    
    if(expected == NULL) {
        printf("%s >>> Computed Results = \"%s\"\n", label, bytesToHexString(computed));
        retval = 1;
    } else if(!bytesAreEqual(expected, computed)) {
        diag(label);
        printByteBuffer(expected, "Expected: ");
        printByteBuffer(computed, "  Result: ");
        retval=0;
    } else {
        retval = 1;
    }
    return retval;
}



char *digestName(CCDigestAlgorithm digestSelector) {
    switch(digestSelector) {
        default: return "None";
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        case  kCCDigestMD2: return "MD2";
        case  kCCDigestMD4: return "MD4";
        case  kCCDigestMD5: return "MD5";
        case  kCCDigestRMD160: return "RMD160";
        case  kCCDigestSHA1: return "SHA1";
#pragma clang diagnostic pop
        case  kCCDigestSHA224: return "SHA224";
        case  kCCDigestSHA256: return "SHA256";
        case  kCCDigestSHA384: return "SHA384";
        case  kCCDigestSHA512: return "SHA512";
    }
}


CCHmacAlgorithm digestID2HMacID(CCDigestAlgorithm digestSelector) {
    CCHmacAlgorithm hmacAlg;
    switch(digestSelector) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        case kCCDigestMD5: hmacAlg = kCCHmacAlgMD5; break;
        case kCCDigestSHA1: hmacAlg = kCCHmacAlgSHA1; break;
#pragma clang diagnostic pop
        case kCCDigestSHA224: hmacAlg = kCCHmacAlgSHA224; break;
        case kCCDigestSHA256: hmacAlg = kCCHmacAlgSHA256; break;
        case kCCDigestSHA384: hmacAlg = kCCHmacAlgSHA384; break;
        case kCCDigestSHA512: hmacAlg = kCCHmacAlgSHA512; break;
        default: return HMAC_UNIMP;
    }
    return hmacAlg;
}

CCPseudoRandomAlgorithm digestID2PRF(CCDigestAlgorithm digestSelector) {
    switch(digestSelector) {
        case 0: return 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        case kCCDigestSHA1: return kCCPRFHmacAlgSHA1;
#pragma clang diagnostic pop
        case kCCDigestSHA224: return kCCPRFHmacAlgSHA224;
        case kCCDigestSHA256: return kCCPRFHmacAlgSHA256;
        case kCCDigestSHA384: return kCCPRFHmacAlgSHA384;
        case kCCDigestSHA512: return kCCPRFHmacAlgSHA512;
        default:
            diag("Unrecognized PRF translation for %s", digestName(digestSelector));
            return 0;
    }
}

