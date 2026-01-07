

#include "capabilities.h"
#include "testmore.h"
#include "testbyteBuffer.h"

#if (CCDH == 0)
entryPoint(CommonDH,"Diffie-Hellman Key Agreement")
#else

#include <CommonCrypto/CommonDH.h>
static int kTestTestCount = 1+2*13*100;

static int testDHgroup(CCDHParameters gp) {
    CCDHRef dh1, dh2;

    dh1 = CCDHCreate(gp);
    ok(dh1 != NULL, "got a DH ref");

    dh2 = CCDHCreate(gp);
    ok(dh2 != NULL, "got a DH ref");


    uint8_t pubkey1[4096], pubkey2[4096];
    size_t len1 = 4096, len2 = 4096;
    int ret1 = CCDHGenerateKey(dh1, pubkey1, &len1);
    int ret2 = CCDHGenerateKey(dh2, pubkey2, &len2);
    ok(len1>10, "Sanity check on length");
    ok(len2>10, "Sanity check on length");
    
    is(ret1,0, "pubkey1 generated");
    is(ret2,0, "pubkey2 generated");
    
    uint8_t sharedsecret1[4096], sharedsecret2[4096];
    size_t slen1 = sizeof(sharedsecret1), slen2 = sizeof(sharedsecret2);

    int sret1 = CCDHComputeKey(sharedsecret1, &slen1, pubkey2, len2, dh1);
    int sret2 = CCDHComputeKey(sharedsecret2, &slen2, pubkey1, len1, dh2);

    is(sret1,0, "shared secret 1 generated");
    is(sret2,0, "shared secret 2 generated");
    isnt(slen1,0, "Length error");
    ok(slen1>(len1-10),"shared secret is unexpectedly short");
    isnt(sharedsecret1[0],0, "Leading zero not stripped out");
    is(slen1,slen2, "shared secret lengths are equal");

    ok_memcmp(sharedsecret1, sharedsecret2, slen1, "shared secrets are equal");

    CCDHRelease(dh1);
    CCDHRelease(dh2);
    return 0;
}


int CommonDH(int __unused argc, char *const * __unused argv) {

    plan_tests(kTestTestCount);
    for (int i = 0 ; i < 100 ; i++) {
        testDHgroup(kCCDHRFC2409Group2);
        testDHgroup(kCCDHRFC3526Group5);
    }
    ok(1, "Didn't crash");

    return 0;
}

#endif
