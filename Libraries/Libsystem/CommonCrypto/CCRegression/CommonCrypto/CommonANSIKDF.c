//
//  CommonANSIKDF.c
//  CommonCrypto
//
//  Copyright 2018 Apple Inc. All rights reserved.
//

#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"

#if (CCANSIKDFTEST == 0)
entryPoint(CommonANSIKDF,"CommonANSIKDF test")
#else
#include <CommonCrypto/CommonKeyDerivationSPI.h>

#define type_sha1 1
#define type_sha224 224
#define type_sha256 256
#define type_sha384 384
#define type_sha512 512

typedef struct {
    int type;
    const char *Z;
    const char *sharedinfo;
    size_t len;
    const char *okm;
} test_vector_t;

static const test_vector_t test_vectors[] = {
    {
        .type = type_sha1,
        .Z = "1c7d7b5f0597b03d06a018466ed1a93e30ed4b04dc64ccdd",
        .sharedinfo = "",
        .len = 16,
        .okm = "bf71dffd8f4d99223936beb46fee8ccc",
    },
    {
        .type = type_sha1,
        .Z = "fd17198b89ab39c4ab5d7cca363b82f9fd7e23c3984dc8a2",
        .sharedinfo = "856a53f3e36a26bbc5792879f307cce2",
        .len = 128,
        .okm = "6e5fad865cb4a51c95209b16df0cc490"
               "bc2c9064405c5bccd4ee4832a531fbe7"
               "f10cb79e2eab6ab1149fbd5a23cfdabc"
               "41242269c9df22f628c4424333855b64"
               "e95e2d4fb8469c669f17176c07d10337"
               "6b10b384ec5763d8b8c610409f19aca8"
               "eb31f9d85cc61a8d6d4a03d03e5a506b"
               "78d6847e93d295ee548c65afedd2efec",
    },
    {
        .type = type_sha224,
        .Z = "9ba3226ba0fca6bd5ddaef5b8d763a4d3303bc258d90468c",
        .sharedinfo = "",
        .len = 16,
        .okm = "15ccbd7d6b8f918335799b3920e69c1f",
    },
    {
        .type = type_sha224,
        .Z = "da67a73072d521a8272c69023573012ddf9b46bff65b3900",
        .sharedinfo = "727997aed53e78f74b1d66743a4ea4d2",
        .len = 128,
        .okm = "dfc3126c5eebf9a58d89730e8d8ff7cc"
               "772592f28c10b349b437d9d068698a22"
               "e532eae975dfaf9c5c6a9f2935eafb05"
               "353013c253444e61f07bc9ddd15948e6"
               "14bdc7e445ba3b1893f42f87f18fb352"
               "d49956009a642c362d45410b43a9ab37"
               "6e9261210739174759511d1f9e52f6ec"
               "73dfed446dbafaf7fd1a57113abc2e8d",
    },
    {
        .type = type_sha256,
        .Z = "96c05619d56c328ab95fe84b18264b08725b85e33fd34f08",
        .sharedinfo = "",
        .len = 16,
        .okm = "443024c3dae66b95e6f5670601558f71",
    },
    {
        .type = type_sha256,
        .Z = "22518b10e70f2a3f243810ae3254139efbee04aa57c7af7d",
        .sharedinfo = "75eef81aa3041e33b80971203d2c0c52",
        .len = 128,
        .okm = "c498af77161cc59f2962b9a713e2b215"
               "152d139766ce34a776df11866a69bf2e"
               "52a13d9c7c6fc878c50c5ea0bc7b00e0"
               "da2447cfd874f6cf92f30d0097111485"
               "500c90c3af8b487872d04685d14c8d1d"
               "c8d7fa08beb0ce0ababc11f0bd496269"
               "142d43525a78e5bc79a17f59676a5706"
               "dc54d54d4d1f0bd7e386128ec26afc21",
    },
    {
        .type = type_sha384,
        .Z = "d8554db1b392cd55c3fe957bed76af09c13ac2a9392f88f6",
        .sharedinfo = "",
        .len = 16,
        .okm = "671a46aada145162f8ddf1ca586a1cda",
    },
    {
        .type = type_sha384,
        .Z = "c051fd22539c9de791d6c43a854b8f80a6bf70190050854a",
        .sharedinfo = "1317504aa34759bb4c931e3b78201945",
        .len = 128,
        .okm = "cf6a84434734ac6949e1d7976743277b"
               "e789906908ad3ca3a8923da7f476abbe"
               "b574306d7243031a85566914bfd247d2"
               "519c479953d9d55b6b831e56260806c3"
               "9af21b74e3ecf470e3bd8332791c8a23"
               "c13352514fdef00c2d1a408ba31b2d3f"
               "9fdcb373895484649a645d1845eec91b"
               "5bfdc5ad28c7824984482002dd4a8677",
    },
    {
        .type = type_sha512,
        .Z = "87fc0d8c4477485bb574f5fcea264b30885dc8d90ad82782",
        .sharedinfo = "",
        .len = 16,
        .okm = "947665fbb9152153ef460238506a0245",
    },
    {
        .type = type_sha512,
        .Z = "00aa5bb79b33e389fa58ceadc047197f"
             "14e73712f452caa9fc4c9adb369348b8"
             "1507392f1a86ddfdb7c4ff8231c4bd0f"
             "44e44a1b55b1404747a9e2e753f55ef0"
             "5a2d",
        .sharedinfo = "e3b5b4c1b0d5cf1d2b3a2f9937895d31",
        .len = 128,
        .okm = "4463f869f3cc18769b52264b0112b585"
               "8f7ad32a5a2d96d8cffabf7fa733633d"
               "6e4dd2a599acceb3ea54a6217ce0b50e"
               "ef4f6b40a5c30250a5a8eeee20800226"
               "7089dbf351f3f5022aa9638bf1ee419d"
               "ea9c4ff745a25ac27bda33ca08bd56dd"
               "1a59b4106cf2dbbc0ab2aa8e2efa7b17"
               "902d34276951ceccab87f9661c3e8816",
    },
};

int CommonANSIKDF(int __unused argc, char *const * __unused argv)
{
    size_t n = sizeof(test_vectors) / sizeof(*test_vectors);
    plan_tests((int)n * 4);

    int rv;
    for (size_t i = 0; i < n; ++i) {
        const test_vector_t *tv = &test_vectors[i];

        byteBuffer Z = hexStringToBytes(tv->Z);
        byteBuffer sharedinfo = hexStringToBytes(tv->sharedinfo);
        byteBuffer okmActual = mallocByteBuffer(tv->len);
        byteBuffer okmExpected = hexStringToBytes(tv->okm);

        CCDigestAlgorithm digest;
        switch (tv->type) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            case type_sha1:
                digest = kCCDigestSHA1;
                break;
#pragma clang diagnostic pop
            case type_sha224:
                digest = kCCDigestSHA224;
                break;
            case type_sha256:
                digest = kCCDigestSHA256;
                break;
            case type_sha384:
                digest = kCCDigestSHA384;
                break;
            case type_sha512:
                digest = kCCDigestSHA512;
                break;
            default:
                abort();
        }

        CCKDFParametersRef params;
        rv = CCKDFParametersCreateAnsiX963(&params, sharedinfo->bytes, sharedinfo->len);
        is(rv, kCCSuccess, "CCKDFParametersCreateAnsiX963 failed");

        rv = CCDeriveKey(params, digest, Z->bytes, Z->len, okmActual->bytes, okmActual->len);
        is(rv, kCCSuccess, "CCDeriveKey failed");

        is(okmActual->len, okmExpected->len, "Wrong length for derived key");
        ok_memcmp(okmActual->bytes, okmExpected->bytes, okmExpected->len, "Wrong derived key");

        free(Z);
        free(sharedinfo);
        free(okmActual);
        free(okmExpected);
    }

    return 0;
}
#endif // CCANSIKDFTEST
