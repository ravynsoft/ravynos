//
//  CommonCryptoReset.c
//  CommonCrypto
//
//  Created by Richard Murphy on 2/10/12.
//  Copyright (c) 2012 McKenzie-Murphy. All rights reserved.
//

#include "capabilities.h"
#include <stdio.h>
#include <string.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonCryptorSPI.h>
#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"
#include "CCCryptorReset_internal.h"

#if (CCRESET == 0)
entryPoint(CommonCryptoReset,"CommonCrypto Reset Testing")
#else

static int kTestTestCount = 49;

static CCCryptorStatus CommonCryptoReset_mode(CCMode mode, CCCryptorStatus rv, CCOptions options)
{
    CCCryptorRef cref;
	CCCryptorStatus retval;
    size_t moved;
    uint8_t key[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t plain[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t nulliv[16];
    uint8_t cipher1[16], cipher2[16], cipher3[16], cipher4[16], cipher5[16], unused[16];

    cc_clear(16, nulliv);

   	retval = CCCryptorCreateWithMode(kCCEncrypt, mode, kCCAlgorithmAES128, ccNoPadding, NULL, key, 16, NULL, 0, 0, options, &cref);
    ok(retval == kCCSuccess, "cryptor created");
    
    retval = CCCryptorUpdate(cref, plain, 16, cipher1, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "first update");

    retval = CCCryptorUpdate(cref, plain, 16, cipher2, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "second (chained) update");
    ok((memcmp(cipher1, cipher2, 16) != 0) || (mode == kCCModeECB), "chained crypts shouldn't be the same even with the same data");  // New test for ECB mode should have equal ciphertexts
    
    retval = CCCryptorReset(cref, NULL);
    is(retval, rv, "cryptor NULL reset");  // ECB should return unimplemented to deal with radar 32948209
    if(retval!= kCCSuccess) goto out;

    retval = CCCryptorUpdate(cref, plain, 16, cipher3, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "third update - NULL Reset");
    ok(memcmp(cipher1, cipher3, 16) == 0, "reset crypt should be the same as the start");

    retval = CCCryptorReset(cref, nulliv);
    ok(retval == rv, "cryptor zero iv reset");

    retval = CCCryptorUpdate(cref, plain, 16, cipher4, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "fourth update - zero iv Reset");
    ok(memcmp(cipher1, cipher4, 16) == 0, "reset crypt should be the same as the start");
    
    retval = CCCryptorUpdate(cref, plain, 16, cipher5, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "fifth (chained) update");
    ok(memcmp(cipher2, cipher5, 16) == 0, "reset crypt should be the same as the second");

out:
    retval = CCCryptorFinal(cref, unused, 16, &moved);
    ok((retval == kCCSuccess && moved == 0), "Final - no work");
	CCCryptorRelease(cref);
    
    return retval;

}

static int CommonCryptoReset_before_10_13(void)
{
    CCCryptorRef cref;
    CCCryptorStatus retval;
    size_t moved;
    uint8_t key[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t plain[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t nulliv[16];
    uint8_t cipher1[16], cipher2[16], cipher3[16], cipher4[16], cipher5[16], unused[16];

    cc_clear(16, nulliv);

    retval = CCCryptorCreateWithMode(kCCEncrypt, kCCModeCBC, kCCAlgorithmAES128, ccNoPadding, NULL, key, 16, NULL, 0, 0, 0, &cref);
    ok(retval == kCCSuccess, "cryptor created");
    
    retval = CCCryptorUpdate(cref, plain, 16, cipher1, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "first update");
    
    retval = CCCryptorUpdate(cref, plain, 16, cipher2, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "second (chained) update");
    ok(memcmp(cipher1, cipher2, 16) != 0, "chained crypts shouldn't be the same even with the same data");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    retval = CCCryptorReset_binary_compatibility(cref, NULL);
    ok(retval == kCCSuccess, "cryptor NULL reset");
#pragma clang diagnostic pop
    retval = CCCryptorUpdate(cref, plain, 16, cipher3, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "third update - NULL Reset");
    ok(memcmp(cipher1, cipher3, 16) == 0, "reset crypt should be the same as the start");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    retval = CCCryptorReset_binary_compatibility(cref, nulliv);
    ok(retval == kCCSuccess, "cryptor zero iv reset");
#pragma clang diagnostic pop
    
    retval = CCCryptorUpdate(cref, plain, 16, cipher4, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "fourth update - zero iv Reset");
    ok(memcmp(cipher1, cipher4, 16) == 0, "reset crypt should be the same as the start");
    
    retval = CCCryptorUpdate(cref, plain, 16, cipher5, 16, &moved);
    ok((retval == kCCSuccess && moved == 16), "fifth (chained) update");
    ok(memcmp(cipher2, cipher5, 16) == 0, "reset crypt should be the same as the second");
    retval = CCCryptorFinal(cref, unused, 16, &moved);
    ok((retval == kCCSuccess && moved == 0), "Final - no work");
    
    CCCryptorRelease(cref);
    
    return kCCSuccess;
}

//to test behavior for macOS prior to 10.13, add "-Wl,-sdk_version,10.12" to Other Linker Flags
int CommonCryptoReset(int __unused argc, char *const * __unused argv)
{
    CCCryptorStatus rc;

    plan_tests(kTestTestCount);
    
    rc = CommonCryptoReset_mode(kCCModeCBC, kCCSuccess, 0);
    ok(rc == kCCSuccess, "CCCryptorReset() for CBC");
    rc = CommonCryptoReset_mode(kCCModeCTR, kCCSuccess, kCCModeOptionCTR_BE);
    ok(rc == kCCSuccess, "CCCryptorReset() for CTR");
    
    if(ProgramLinkedOnOrAfter_macOS1013_iOS11()){
        rc = CommonCryptoReset_mode(kCCModeECB, kCCUnimplemented, 0);  // Testing for radar 32948209
        ok(rc == kCCSuccess, "CCCryptorReset() for ECB");
    }
    rc = CommonCryptoReset_before_10_13(); ok(rc == kCCSuccess, "CCCryptorReset() before macos 10.13");
    return kCCSuccess;
    
}

#endif
