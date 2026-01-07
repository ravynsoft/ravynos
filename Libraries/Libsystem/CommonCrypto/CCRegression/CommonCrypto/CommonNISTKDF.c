//
//  CommonNISTKDF.c
//  CommonCrypto
//
//  Created by Richard Murphy on 12/19/13.
//  Copyright (c) 2013 Platform Security. All rights reserved.
//

#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"

static int verbose = 0;

#if (CCNISTKDFTEST == 0)
entryPoint(CommonNISTKDF,"CommonNISTKDF test")
#else
#include <CommonCrypto/CommonKeyDerivationSPI.h>

typedef struct test_vector_t {
    int testnum;
    char *k1Str;
    char *labelStr; // or fixedData
    char *contextStr;
    size_t result_len;
    char *sha1_answer;
    char *sha224_answer;
    char *sha256_answer;
    char *sha384_answer;
    char *sha512_answer;
} test_vector;

static char *
CCDigestName(CCDigestAlgorithm alg) {
    switch(alg) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    case kCCDigestSHA1: return "SHA-1";
#pragma clang diagnostic pop
    case kCCDigestSHA224: return "SHA-224";
    case kCCDigestSHA256: return "SHA-256";
    case kCCDigestSHA384: return "SHA-384";
    case kCCDigestSHA512: return "SHA-512";
    }
    return "";
}

static int test_answer(CCDigestAlgorithm alg, test_vector *vector, size_t answer_len, void*answer) {
    char *correct_answer = NULL;
    switch(alg) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    case kCCDigestSHA1: correct_answer = vector->sha1_answer; break;
#pragma clang diagnostic pop
    case kCCDigestSHA224: correct_answer = vector->sha224_answer; break;
    case kCCDigestSHA256: correct_answer = vector->sha256_answer; break;
    case kCCDigestSHA384: correct_answer = vector->sha384_answer; break;
    case kCCDigestSHA512: correct_answer = vector->sha512_answer; break;
    default: correct_answer = NULL;
    }
    byteBuffer answer_bb = bytesToBytes(answer, answer_len);
    if(correct_answer == NULL || strcmp(correct_answer, "") == 0) {
        diag("Returned Answer for Test (%d) for NistKDF-CTR-HMac-%s\n", vector->testnum, CCDigestName(alg));
        diag("\t\t\"%s\", // %s\n", bytesToHexString(answer_bb), CCDigestName(alg));
        if (answer_bb) free(answer_bb);
        return 1;
    }
    byteBuffer correct_answer_bb = hexStringToBytes((char *) correct_answer);
    ok(bytesAreEqual(correct_answer_bb, answer_bb), "compare memory of answer");
    if(bytesAreEqual(correct_answer_bb, answer_bb) == 0) {
        diag("Failed Test (%d) for NistKDF-CTR-HMac-%s\n", vector->testnum, CCDigestName(alg));
        printByteBuffer(correct_answer_bb, "Correct Answer");
        printByteBuffer(answer_bb, "Provided Answer");
    }
    free(correct_answer_bb);
    free(answer_bb);
    return 1;
}

static int test_ctr_hmac_fixed_oneshot(CCDigestAlgorithm alg, test_vector *vector) {
    uint8_t answer[vector->result_len];
    byteBuffer k1 = hexStringToBytes(vector->k1Str);
    byteBuffer label = hexStringToBytes(vector->labelStr);
    byteBuffer context = hexStringToBytes(vector->contextStr);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    CCKeyDerivationHMac(kCCKDFAlgorithmCTR_HMAC_FIXED, alg, 0, k1->bytes, k1->len, NULL, 0,
                       label->bytes, label->len,
                       NULL, 0, NULL, 0,
                       answer, vector->result_len);
#pragma clang diagnostic pop
    ok(test_answer(alg, vector, vector->result_len, answer), "check answer");
    free(k1);
    free(label);
    free(context);
    return 1;
}


static int test_nistkdf_hmac(CCDigestAlgorithm alg) {
    static test_vector vector_ctr_hmac[] = {
        { // from published NIST KAT for PRF=SHA256, R=32, CTRLOCATION=Before Raw FixedData
            1,
            "dd1d91b7d90b2bd3138533ce92b272fbf8a369316aefe242e659cc0ae238afe0", // K1
            "01322b96b30acd197979444e468e1c5c6859bf1b1cf951b7e725303e237e46b8"\
            "64a145fab25e517b08f8683d0315bb2911d80a0e8aba17f3b413faac", // Label/FixedData
            "", // Context
            16,  // result_length
            "cb2f32a1708b762bddb4ce6ae3c99f72", // SHA1
            "f4bbe37ff89bfba838fbde108c1298e5", // SHA224
            "10621342bfb0fd40046c0e29f2cfdbf0", // SHA256
            "b8ede53b06a02d734c8f52abaf67e5d9", // SHA384
            "6056854e0cd0b03a9026c42c1cd17de4", // SHA512
        },
        { // from published NIST KAT for PRF=SHA224, R=32, CTRLOCATION=Before Raw FixedData
            2,
            "f5cb7cc6207f5920dd60155ddb68c3fbbdf5104365305d2c1abcd311", // K1
            "4e5ac7539803da89581ee088c7d10235a10536360054b72b8e9f18f7"\
            "7c25af01019b290656b60428024ce01fccf49022d831941407e6bd27"\
            "ff9e2d28", // Label/FixedData
            "", // Context
            16, // result_length
            "9a379dd1e18ecf2a00c1c9e4ca0f21e7", // SHA1
            "0adbaab43edd532b560a322c84ac540e", // SHA224
            "c571250d780cf51e6332713dfe84022b", // SHA256
            "0d955e07d933eec54e8871c7fbceb2d3", // SHA384
            "19c0c3c0c323efd19fa18f21830471e0", // SHA512
        },
        { // from published NIST KAT for PRF=SHA1, R=32, CTRLOCATION=Before Raw FixedData
            3,
            "f7591733c856593565130975351954d0155abf3c", // K1
            "8e347ef55d5f5e99eab6de706b51de7ce004f388"\
            "2889e259ff4e5cff102167a5a4bd711578d4ce17"\
            "dd9abe56e51c1f2df950e2fc812ec1b217ca08d6", // Label/FixedData
            "", // Context
            16, // result_length
            "34fe44b0d8c41b93f5fa64fb96f00e5b", // SHA1
            "6ea5f4643c111f2de3d7c4a4e9e23ae0", // SHA224
            "2be4e09a1bd9c8c462205e3ecd9b9de2", // SHA256
            "799b86a06a73ee08ba54151b219518a8", // SHA384
            "f7d86befdde6d2353661995a6da53057", // SHA512
        },
        { // from published NIST KAT for PRF=SHA384, R=32, CTRLOCATION=Before Raw FixedData
            4,
            "216ed044769c4c3908188ece61601af8"\
            "819c30f501d12995df608e06f5e0e607"\
            "ab54f542ee2da41906dfdb4971f20f9d", // K1
            "638e9506a2c7be69ea346b84629a010c"\
            "0e225b7548f508162c89f29c1ddbfd70"\
            "472c2b58e7dc8aa6a5b06602f1c8ed49"\
            "48cda79c62708218e26ac0e2", // Label/FixedData
            "", // Context
            16, // result_length
            "42f4c5206acb6fde73ee356a97069e10", // SHA1
            "8a943715e5568f12b13430ee88d53e57", // SHA224
            "155b48ca2900dc1043f3c973454d1097", // SHA256
            "d4b144bb40c7cabed13963d7d4318e72", // SHA384
            "8b32a1e63e3633032f2f9cc6159aea21", // SHA512
        },
        { // from published NIST KAT for PRF=SHA512, R=32, CTRLOCATION=Before Raw FixedData
            5,
            "dd5dbd45593ee2ac139748e7645b450f"\
            "223d2ff297b73fd71cbcebe71d41653c"\
            "950b88500de5322d99ef18dfdd304282"\
            "94c4b3094f4c954334e593bd982ec614", // K1
            "b50b0c963c6b3034b8cf19cd3f5c4ebe"\
            "4f4985af0c03e575db62e6fdf1ecfe4f"\
            "28b95d7ce16df85843246e1557ce95bb"\
            "26cc9a21974bbd2eb69e8355", // Label/FixedData
            "", // Context
            16, // result_length
            "a63a98af96ca265d7033963e9e6ed86d", // SHA1
            "7eca8b022660a1161c441239b9e63f1f", // SHA224
            "7d178ea8448edcad7e14aa32eb84f022", // SHA256
            "a68e8d4b7102e6c75e77c072294f54c7", // SHA384
            "e5993bf9bd2aa1c45746042e12598155", // SHA512
        },
        { // from published NIST KAT for PRF=SHA1, R=32, CTRLOCATION=Before Raw FixedData, length 320
            6,
            "eea784c3e3181af8348385456878a775c3a41708", // K1
            "51d601ecb9cabb4c5cc6348983a1d243"\
            "44831812f6d3559399396418ff8824b7"\
            "eae36350bb40dd66ec0677f49b5f5ab9"\
            "805cb272562ed5c7ce0b30c9", // Label/FixedData
            "", // Context
            40, // result_length
            "2fd6dc21e31cee812ae3738cea9f7c2a121c0a0f4b985b1d506eff72fde7fb6f0948c92a86b4b525", // SHA1
            "2bc034e19979a8589be6c2c3eb20fb1dadd394b4cea3ce8d9621d0ccd40c852858dbd2c1a7628290", // SHA224
            "9550a2395ea822cc16f4f041d21ebf1cf62204656291284cdacfa3c6b0c255cd5e93944771aa5648", // SHA256
            "110066b403b2ab67198a2ba3a57868f87e322c3aad7e42d444ecf14174fbee5049b25f9d5aadfd7f", // SHA384
            "b54833fd62e5d03cd678471174c3efc9b0a26a023724ab318b34b767f987d172e1e0fab0e48b0f82", // SHA512
        },
        { // from published NIST KAT for PRF=SHA224, R=32, CTRLOCATION=Before Raw FixedData, length 320
            7,
            "6d4411ccfeca782ffc87ed9fc9163992d9e1cab24dea690b966b3231", // K1
            "ee7967ddcaa6ddb242bbb2cee1fbf78632172d74f9bc0be645d52c19c7ee5b961ff6765432013372f693b7ca7f4489025fab6e599985c63e551e3733", // Label/FixedData
            "", // Context
            40, // result_length
            "b3359f55651dd68ac85645b24908a15bdbb52be6d75673ecd5a4a14c4577c94e0c166e4cda7441f7", // SHA1
            "221bb18f086c7396b73ecb68b20ce8ef61739a6db97b2ab8a3396722a93be1c759855a95de86d469", // SHA224
            "438d0d10e935acc632168941eacd69d76bdf3f0270a49c793414f28c9cec29a081f2366b9e025987", // SHA256
            "667d4d85cd8b08cf994296b1f5511c4c64da3f8e5c2daea23423d4750c99d2c39387c996f0b1c4cf", // SHA384
            "3e3503794a770b9141c24e28ad52069cacf1cb9943b170999fab036674c6a56a1ec46bd621bb0191", // SHA512
        },
        { // from published NIST KAT for PRF=SHA256, R=32, CTRLOCATION=Before Raw FixedData, length 320
            8,
            "9a6e83b91bd999737e577e449142dae05968e774b223c1185dc574da785c93cc", // K1
            "4b5845c6737202632b2946c3579d9d4582b475dfa373945b0abc68c8f0daa36520179439086c6809aa182094453bc0bffef0dc2888b96295fcd6e442", // Label/FixedData
            "", // Context
            40, // result_length
            "6245d8321bd5af6a96ace0f7367b3eb6de6e4cff1e83f58e33bc6f72ec6071fab0e5ea1b981e211e", // SHA1
            "02daabf2c978cd4cec0a5730da4cffe505a7489c6c4b1f79b724d7b47429e8b5e09280c3aca62d70", // SHA224
            "e90e3ed902a8eb1fc67823af534a2b48466bf2c5877dad0aadc7d6ff741d8f437b2e6d0031846960", // SHA256
            "fe29134814802cf556b65b335a2350b58f634fe89bead19383670e55ec293e48f605895e51661947", // SHA384
            "177fe6c90f460b18a62e3035c15bb35f7968ff6b2e431c56becbe3e83e7b3f1014d6154dbff103b5", // SHA512
        },
        { // from published NIST KAT for PRF=SHA384, R=32, CTRLOCATION=Before Raw FixedData, length 320
            9,
            "0c5620f34aa7029f655a9eb9b051f13251d65bdf99d390b8f67898eb2216ec10bacb29358b895529db64fbfc942fd0ff", // K1
            "6d8b8cdf7b699c0205c6feb4ac1839d3c436cf962f8575ee67ff20d69103c4aa93bb369d360980181e38c44215065c99a066946733ede23185183617", // Label/FixedData
            "", // Context
            40, // result_length
            "8be493b61cfb17c9b07d946203ce6821e235b47e8929d06b666968f0660b3979b877dc9ccfbf5664", // SHA1
            "0c82c5484d5ceddc3286a95849cd47d69d0d4ff5cdcf73f94c32c37d187ac33dbc0af3a4d34d67be", // SHA224
            "f71a8f2ae10aebcffa80a43d113f05c8c5276a63ec6356b960ccb5cf8e2626bc2d540d04b8a6021c", // SHA256
            "2857c7b4221a02b6717a1c67b1eeb64dcda8162284faae88466414b317e45457b5aaef5b5089722f", // SHA384
            "83daadfab0823929bb358d1c650587e844a0bc9da9890da1694f14ce26c7eb8473d3ee07d0116561", // SHA512
        },
        { // from published NIST KAT for PRF=SHA512, R=32, CTRLOCATION=Before Raw FixedData, length 320
            10,
            "46a65f2e432fe2aab26de24d1b9d39b44da270230f17844d44e249565a125f87a070b4b2eeac0b3a6c54acfc49ddc6360f59fe0e330e0605c61e85c5c27fe756", // K1
            "b06a23e05945f96a293de59dc3db5972ca1fa00b4647ac38f753790335d5daeb2ffe09cf8924ac4e80b275ea4dbec53b9e2aaf90df0e6fd82ad69f7c", // Label/FixedData
            "", // Context
            40, // result_length
            "0252c5b6e21f4de10b5f59d13b241c435b2b96cea07b2c45e1ec55eeeb9920c28a8eb889b2febf00", // SHA1
            "3b63de00aacc5530280a99e92013525b1ed216ec9bb732734f469611f1ec3e6b065758ad9674611a", // SHA224
            "dc68d700c478e6a1cf5f92952711c72ef7b4cd2d637bc63fa6249187a1326fd46eaea99fa3b7776c", // SHA256
            "51eb9fb2f19253b9e236205dcb1c62e897dd7af387d54a9f466e71c63d9ea70a93f30bbb5f12a6bf", // SHA384
            "04bbc4f93fb5c3589690798bf793c10bb726f87a4a2de93b8fdf6be8015abe156577119c5637ae71", // SHA512
        },


    };
    int vector_size = sizeof (vector_ctr_hmac) / sizeof (test_vector);
    if(verbose) diag("nistkdf_hmac LT Test\n");
    
    for(int i=0; i<vector_size; i++) {
        ok(test_ctr_hmac_fixed_oneshot(alg, &vector_ctr_hmac[i]), "test one-shot with data less than blocksize");
    }
    // test_size(di);
    return 1;
}

int CommonNISTKDF(int __unused argc, char *const * __unused argv)
{
	plan_tests(155);
    if(verbose) diag("Starting nistkdf_hmac tests\n");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    ok(test_nistkdf_hmac(kCCDigestSHA1), "ccsha1_ltc_di");
#pragma clang diagnostic pop
    ok(test_nistkdf_hmac(kCCDigestSHA224), "ccsha224_ltc_di");
    ok(test_nistkdf_hmac(kCCDigestSHA256), "ccsha256_ltc_di");
    ok(test_nistkdf_hmac(kCCDigestSHA384), "ccsha384_ltc_di");
    ok(test_nistkdf_hmac(kCCDigestSHA512), "ccsha512_ltc_di");
    return 0;
}
#endif

