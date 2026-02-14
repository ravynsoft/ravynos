//
//  CommonCPP.cpp
//  CommonCrypto
//
//  Created on 4/17/17.
//  Copyright © 2017 Apple Inc. All rights reserved.
//
//  This is to make sure CommonCrypto links with C++

#include <stdio.h>
#include <vector>
#include <CommonCrypto/CommonKeyDerivationSPI.h>
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

extern "C" int CommonCPP(int __unused argc, char *const * __unused argv);

#if (COMMONCPPTEST == 0)
entryPoint(CommonCPP,"CommonCPP test")
#else

uint8_t ikm[] = {0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B};
int len=42;
uint8_t exp_okm[]={0xF5, 0xFA, 0x02, 0xB1, 0x82, 0x98, 0xA7, 0x2A, 0x8C, 0x23, 0x89, 0x8A, 0x87, 0x03, 0x47, 0x2C, 0x6E, 0xB1, 0x79, 0xDC, 0x20, 0x4C, 0x03, 0x42, 0x5C, 0x97, 0x0E, 0x3B, 0x16, 0x4B, 0xF9, 0x0F, 0xFF, 0x22, 0xD0, 0x48, 0x36, 0xD0, 0xE2, 0x34, 0x3B, 0xAC};

using namespace std;
static void test_CCKeyDerivationHMac()
{
    vector<char> okm(100);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    CCStatus status=CCKeyDerivationHMac(kCCKDFAlgorithmHKDF, kCCDigestSHA512, 0, ikm, sizeof ikm, NULL, 0, NULL, 0, NULL, 0, NULL, 0, okm.data(), sizeof exp_okm );
#pragma clang diagnostic pop
    ok(status==kCCSuccess, "CPP interface");
    ok_memcmp(okm.data(), exp_okm, sizeof exp_okm, "CPP interface");
}

int CommonCPP(int __unused argc, char *const * __unused argv)
{
    plan_tests(2);
    test_CCKeyDerivationHMac();
    return 1;
}
#endif
