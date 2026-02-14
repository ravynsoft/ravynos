//
//  CommonCollabKeyGen.c
//  CCRegression
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"

#include <CommonCrypto/CommonCollabKeyGen.h>

int CommonCollabKeyGen(int __unused argc, char *const * __unused argv)
{
    plan_tests(11);

    CCCryptorStatus rv;

    size_t commitment_len = CCCKGGetCommitmentSize(224, kCCDigestSHA256);
    size_t share_len = CCCKGGetShareSize(224, kCCDigestSHA256);
    size_t opening_len = CCCKGGetOpeningSize(224, kCCDigestSHA256);

    CCCollabKeyGenContributorRef contrib;
    CCCKGContributorCreate(224, kCCDigestSHA256, &contrib);

    CCCollabKeyGenOwnerRef owner;
    CCCKGOwnerCreate(224, kCCDigestSHA256, &owner);

    uint8_t commitment[commitment_len];
    rv = CCCKGContributorCommit(contrib, commitment, sizeof(commitment));
    is(rv, kCCSuccess, "Contributor committed");

    uint8_t share[share_len];
    rv = CCCKGOwnerGenerateShare(owner, commitment, sizeof(commitment), share, sizeof(share));
    is(rv, kCCSuccess, "Owner generated share");

    uint8_t sk1[32];
    CCECCryptorRef publicKey;
    uint8_t opening[opening_len];
    rv = CCCKGContributorFinish(contrib, share, sizeof(share), opening, sizeof(opening), sk1, sizeof(sk1), &publicKey);
    is(rv, kCCSuccess, "Contributor finished");

    uint8_t sk2[32];
    CCECCryptorRef privateKey;
    rv = CCCKGOwnerFinish(owner, opening, sizeof(opening), sk2, sizeof(sk2), &privateKey);
    is(rv, kCCSuccess, "Owner finished");

    ok_memcmp(sk1, sk2, sizeof(sk1), "SKs match");

    size_t pub1_len = 28;
    size_t pub2_len = 28;
    uint8_t pub1[pub1_len], pub2[pub2_len];
    rv = CCECCryptorExportKey(kCCImportKeyCompact, pub1, &pub1_len, ccECKeyPublic, publicKey);
    is(rv, kCCSuccess, "Public key 1 exported");
    rv = CCECCryptorExportKey(kCCImportKeyCompact, pub2, &pub2_len, ccECKeyPublic, privateKey);
    is(rv, kCCSuccess, "Public key 2 exported");

    is(pub1_len, pub2_len, "Public keys match");
    ok_memcmp(pub1, pub2, pub1_len, "Public keys match");

    size_t priv_len = 85;
    uint8_t priv[priv_len];
    rv = CCECCryptorExportKey(kCCImportKeyBinary, priv, &priv_len, ccECKeyPrivate, privateKey);
    is(rv, kCCSuccess, "Private key exported");
    CCECCryptorRelease(privateKey);
    rv = CCECCryptorImportKey(kCCImportKeyBinary, priv, priv_len, ccECKeyPrivate, &privateKey);
    is(rv, kCCSuccess, "Private key imported");

    CCCKGContributorDestroy(contrib);
    CCCKGOwnerDestroy(owner);

    CCECCryptorRelease(publicKey);
    CCECCryptorRelease(privateKey);

    return 0;
}
