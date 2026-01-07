
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"
#include <CommonCrypto/CommonBigNum.h>




#if (CCBIGNUM == 0)
entryPoint(CommonBigNum,"Big Number Arithmetic")
#else

#include <CommonCrypto/CommonBigNum.h>
static int kTestTestCount = 200620;



#define STRESSSIZE 5000

static int testCreateFree()
{
    CCStatus status;
    CCBigNumRef stress[STRESSSIZE];
    for(size_t i=0; i<STRESSSIZE; i++) stress[i] = NULL;

    byteBuffer bb = hexStringToBytes("0102030405060708091011121314151617181920");
    for(int i=0; i<10000; i++) {
            CCBigNumRef r = CCBigNumCreateRandom(&status, 31, 31, 0);
            ok(status == kCCSuccess, "Created Random Number");
            size_t ri = CCBigNumGetI(&status, r);
            ok(status == kCCSuccess, "translated to int");
            
            ri %= STRESSSIZE;

            if(stress[ri] == NULL) {
                int sel = 4;
                switch(sel) {
                    case 0: /* printf("(%lu) BigNum\n", ri); */ stress[ri] = CCCreateBigNum(&status); break;
                    case 1: /* printf("(%lu) FromHex\n", ri); */ stress[ri] = CCBigNumFromHexString(&status, "0003"); break;
                    case 2: /* printf("(%lu) FromData\n", ri); */ stress[ri] = CCBigNumFromData(&status, bb->bytes, bb->len); break;
                    case 3: /* printf("(%lu) FromDecimal\n", ri); */ stress[ri] = CCBigNumFromDecimalString(&status, "128"); break;
                }
            } else {
                /* printf("(%lu) Freeing\n", ri); */
                CCBigNumClear(stress[ri]);
                CCBigNumFree(stress[ri]);
                stress[ri] = NULL;
            }
            CCBigNumFree(r);
    }
    // Free allocated numbers
    for(size_t i=0; i<STRESSSIZE; i++) {
        if (stress[i] != NULL) CCBigNumFree(stress[i]);
    }

    free(bb);
    return 0;
}

static int testHexString()
{
    CCStatus status;
    char *hexstring = "1002030405060708090021222324252627282920";
    CCBigNumRef num1 = CCBigNumFromHexString(&status, hexstring);
    char *output;
    
    ok(status == 0, "BigNum Created");
    output = CCBigNumToHexString(&status, num1);
    ok(status == 0, "Value retrieved");
    ok(strcmp(output, hexstring) == 0, "strings are equal");
    if(strcmp(output, hexstring)) {
        printf("output: %s\n", output);
        printf("input : %s\n", hexstring);
    }
    free(output);
    CCBigNumFree(num1);

    return 0;

}


static int testDecimalString()
{
    CCStatus status;
    char *decimalstring = "1002030405060708090021222324252627282920";
    CCBigNumRef num1 = CCBigNumFromDecimalString(&status, decimalstring);
    char *output;
    
    ok(status == 0, "BigNum Created");
    output = CCBigNumToDecimalString(&status, num1);
    ok(status == 0, "Value retrieved");
    ok(strcmp(output, decimalstring) == 0, "strings are equal");
    if(strcmp(output, decimalstring)) {
        printf("output: %s\n", output);
        printf("input : %s\n", decimalstring);
    }
    free(output);
    CCBigNumFree(num1);
    
    return 0;
    
}


static int testData()
{
    CCStatus status;
    char *hexstring = "1002030405060708090021222324252627282920";
    byteBuffer bb = hexStringToBytes(hexstring);
    CCBigNumRef num1 = CCBigNumFromData(&status, bb->bytes, bb->len);
    char *output;
    
    ok(status == 0, "BigNum Created");
    output = CCBigNumToHexString(&status, num1);
    ok(status == 0, "Value retrieved");
    ok(strcmp(output, hexstring) == 0, "strings are equal");
    if(strcmp(output, hexstring)) {
        printf("output: %s\n", output);
        printf("input : %s\n", hexstring);
    }
    free(output);
    
    byteBuffer outbuf = mallocByteBuffer(64);
    outbuf->len = CCBigNumToData(&status, num1, outbuf->bytes);
    ok(status == 0, "Value retrieved 2");
    
    ok(bytesAreEqual(bb, outbuf), "input == output");

    free(bb);
    free(outbuf);
    CCBigNumFree(num1);

    return 0;
    
}

static int testI()
{
    CCStatus status;
    uint32_t I=0x10203040;
    char *hexstring = "10203040";
    byteBuffer bb = hexStringToBytes(hexstring);
    CCBigNumRef num1 = CCCreateBigNum(&status);
    char *output;
    
    ok(status == 0, "BigNum Created");
    status = CCBigNumSetI(num1, I);
    ok(status == 0, "BigNum Set to I");
    output = CCBigNumToHexString(&status, num1);
    ok(status == 0, "Value retrieved");
    ok(strcmp(output, hexstring) == 0, "strings are equal");
    if(strcmp(output, hexstring)) {
        printf("output: %s\n", output);
        printf("input : %s\n", hexstring);
    }
    free(output);
    
    uint32_t outI = CCBigNumGetI(&status, num1);
    ok(status == 0, "Value retrieved 2");
    
    ok(outI == I, "input == output");
    free(bb);
    CCBigNumFree(num1);
    
    return 0;
    
}


static int testCompare()
{
    CCStatus status;
    char *lowstring = "030405060708090021222324252627282920";
    char *midstring = "1002030405060708090021222324252627282920";
    char *histring  = "1002030405060708090f21222324252627282920";
    CCBigNumRef low = CCBigNumFromHexString(&status, lowstring);
    ok(status == 0, "BigNum Created");
    CCBigNumRef mid = CCBigNumFromHexString(&status, midstring);
    ok(status == 0, "BigNum Created");
    CCBigNumRef midsame = CCBigNumFromHexString(&status, midstring);
    ok(status == 0, "BigNum Created");
    CCBigNumRef hi = CCBigNumFromHexString(&status, histring);
    ok(status == 0, "BigNum Created");
    CCBigNumRef iVal = CCCreateBigNum(&status);
    ok(status == 0, "BigNum Created");
    status = CCBigNumSetI(iVal, 67);

    ok(CCBigNumCompare(mid, low) == 1, "mid > low");
    ok(CCBigNumCompare(mid, hi) == -1, "mid < hi");
    ok(CCBigNumCompare(mid, midsame) == 0, "mid == midsame");
    ok(CCBigNumCompareI(iVal, 67) == 0, "iVal equality is correct");
    ok(CCBigNumCompareI(iVal, 66) > 0, "iVal greater is correct");
    ok(CCBigNumCompareI(iVal, 68) < 0, "iVal less than is correct");

    CCBigNumFree(low);
    CCBigNumFree(mid);
    CCBigNumFree(midsame);
    CCBigNumFree(hi);
    CCBigNumFree(iVal);

    return 0;
    
}

static int testBitCount()
{
    CCStatus status;
    char *hexstring = "1002030405060708090021222324252627282920";
    CCBigNumRef num1 = CCBigNumFromHexString(&status, hexstring);
    ok(status == 0, "BigNum Created");
    
    uint32_t bits = CCBigNumBitCount(num1);
    ok(bits == 157, "bit count is correct");

    CCBigNumFree(num1);
    return 0;
}

static int testAddSub()
{
    CCStatus status;
    char *hex1 = "1002030405060708090021222324252627282920";
    char *hex2 = "1002030405060708090021222324252627282920";
    char *result = "200406080a0c0e101200424446484a4c4e505240";
    CCBigNumRef num1 = CCBigNumFromHexString(&status, hex1);
    CCBigNumRef num2 = CCBigNumFromHexString(&status, hex2);
    CCBigNumRef output = CCCreateBigNum(&status);
    CCBigNumRef resultExpected = CCBigNumFromHexString(&status, result);
    
    status = CCBigNumAdd(output, num1, num2);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected) == 0, "expected operation result");

    status = CCBigNumSub(output, num1, num2);
    ok(status == 0, "operation completed");
    CCBigNumSetI(resultExpected, 0);
    ok(CCBigNumCompare(output, resultExpected) == 0, "expected operation result");
    
    CCBigNumFree(num1);
    CCBigNumFree(num2);
    CCBigNumFree(output);
    CCBigNumFree(resultExpected);
    return 0;
}

static int testAddSubI()
{
    CCStatus status;
    char *hex1 = "1002030405060708090021222324252627282920";
    char *result = "1002030405060708090021222324252627282921";
    CCBigNumRef num1 = CCBigNumFromHexString(&status, hex1);
    CCBigNumRef output = CCCreateBigNum(&status);
    CCBigNumRef resultExpected = CCBigNumFromHexString(&status, result);
    
    status = CCBigNumAddI(output, num1, 1);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected) == 0, "expected operation result");
    
    status = CCBigNumSubI(output, output, 1);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, num1) == 0, "expected operation result");
    
    CCBigNumFree(num1);
    CCBigNumFree(output);
    CCBigNumFree(resultExpected);
    return 0;
}


static int testShift()
{
    CCStatus status;
    
    // Shift n right by 11
    CCBigNumRef n = CCBigNumFromHexString(&status, "1002030405060708090021222324252627282920");
    CCBigNumRef resultExpected = CCBigNumFromHexString(&status, "200406080a0c0e101200424446484a4c4e505");
    CCBigNumRef output = CCCreateBigNum(&status);
    status = CCBigNumRightShift(output, n, 11);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected) == 0, "expected operation result");

    // Shift n left by 14
    CCBigNumRef resultExpected2 = CCBigNumFromHexString(&status, "40080c1014181c20240084888c9094989ca0a480000");
    status = CCBigNumLeftShift(output, n, 14);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected2) == 0, "expected operation result");

    
    CCBigNumFree(n);
    CCBigNumFree(output);
    CCBigNumFree(resultExpected);
    CCBigNumFree(resultExpected2);
    return 0;
}

static int testModExp()
{
    CCStatus status;
    
    // Shift n right by 11
    CCBigNumRef a = CCBigNumFromHexString(&status, "c968e40c5304364b057425920b18cc358f254ddb0f42f84850d6deec46006b4a692e52b7c3bddead45f77f2c1be1c606521d8a24260429f362d65b57873dbf270e97e210b872e45e97cb4cd87977ad20491e53c48cf0e88da9a61312675a2527c86ac537740c5e4206972f09c0f91fa1c9f14a2cf1be07e82a3b6fd58dc12c3a");
    CCBigNumRef b = CCBigNumFromHexString(&status, "010001");
    CCBigNumRef c = CCBigNumFromHexString(&status, "354c912b09ee7abff5b3d94ed52a9e8dcae582e094daa375c495f970710af73efcc4f9776010511f654c7408a6d5d351ab1d94a0fede757d782b54ddcf6fe8d714870b78b0e67a9754cb03a5cf63bbda1c71791902ea4527fb0cd76437391e5422c704ffb6d6018261171d8cee98adcf0243f1fd520fb3761afe94a2f4d99f94");
    CCBigNumRef resultExpected = CCBigNumFromHexString(&status, "0fb978a4f4fccdb14c7268918b784c4f6d5281c0d6ff43e60e88e97f97f2617608de2488c84eb99a3f467013c860536ec74f4968abeccbc1b026ee5873e40bdd292f8f7416a93df619288b49ba21d3e09aa796cb35a340b1abfda4e3b6cd92df2de64967e6a59f787586929c4d2920da20caeb384594d7f2b7e999dab0d6a1ac");
    CCBigNumRef output = CCCreateBigNum(&status);
    
    status = CCBigNumModExp(output, a, b, c);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected) == 0, "expected operation result");

    CCBigNumFree(a);
    CCBigNumFree(b);
    CCBigNumFree(c);
    CCBigNumFree(output);
    CCBigNumFree(resultExpected);
    return 0;
}

static int testMod()
{
    CCStatus status;
    
    // Shift n right by 11
    CCBigNumRef a = CCBigNumFromHexString(&status, "c968e40c5304364b057425920b18cc358f254ddb0f42f84850d6deec46006b4a692e52b7c3bddead45f77f2c1be1c606521d8a24260429f362d65b57873dbf270e97e210b872e45e97cb4cd87977ad20491e53c48cf0e88da9a61312675a2527c86ac537740c5e4206972f09c0f91fa1c9f14a2cf1be07e82a3b6fd58dc12c3a");
    CCBigNumRef c = CCBigNumFromHexString(&status, "354c912b09ee7abff5b3d94ed52a9e8dcae582e094daa375c495f970710af73efcc4f9776010511f654c7408a6d5d351ab1d94a0fede757d782b54ddcf6fe8d714870b78b0e67a9754cb03a5cf63bbda1c71791902ea4527fb0cd76437391e5422c704ffb6d6018261171d8cee98adcf0243f1fd520fb3761afe94a2f4d99f94");
    CCBigNumRef resultExpected = CCBigNumFromHexString(&status, "2983308b3538c60b245899a58b98f08c2e74c53950b30de70314f29af2df858d72df6651a38ceb4f1612231227604c1150c4cc412968c97afa545cbe18ee04a1d102bfa6a5bf7498996a41e70b4c7991f3c9e87984321915b87f8ce5c1aeca2b6015b6384f8a59bae351d662f52f1634c3257434fb8eed85d93fb1ecaf344d7e");
    CCBigNumRef output = CCCreateBigNum(&status);
    
    status = CCBigNumMod(output, a, c);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected) == 0, "expected operation result");
    
    CCBigNumFree(a);
    CCBigNumFree(c);
    CCBigNumFree(output);
    CCBigNumFree(resultExpected);
    return 0;
}


static int testModI()
{
    CCStatus status;
    
    // Shift n right by 11
    CCBigNumRef a = CCBigNumFromHexString(&status, "c968e40c5304364b057425920b18cc358f254ddb0f42f84850d6deec46006b4a692e52b7c3bddead45f77f2c1be1c606521d8a24260429f362d65b57873dbf270e97e210b872e45e97cb4cd87977ad20491e53c48cf0e88da9a61312675a2527c86ac537740c5e4206972f09c0f91fa1c9f14a2cf1be07e82a3b6fd58dc12c3a");
    uint32_t c=63;
    uint32_t resultExpected = 23;
    uint32_t output;
    
    status = CCBigNumModI(&output, a, c);
    ok(status == 0, "operation completed");
    ok(resultExpected == output, "expected operation result");
    
    CCBigNumFree(a);
    return 0;
}


static int testMul()
{
    CCStatus status;
    
    // Shift n right by 11
    CCBigNumRef a = CCBigNumFromHexString(&status, "c968e40c5304364b057425920b18cc358f254ddb0f42f84850d6deec46006b4a692e52b7c3bddead45f77f2c1be1c606521d8a24260429f362d65b57873dbf270e97e210b872e45e97cb4cd87977ad20491e53c48cf0e88da9a61312675a2527c86ac537740c5e4206972f09c0f91fa1c9f14a2cf1be07e82a3b6fd58dc12c3a");
    CCBigNumRef c = CCBigNumFromHexString(&status, "354c912b09ee7abff5b3d94ed52a9e8dcae582e094daa375c495f970710af73efcc4f9776010511f654c7408a6d5d351ab1d94a0fede757d782b54ddcf6fe8d714870b78b0e67a9754cb03a5cf63bbda1c71791902ea4527fb0cd76437391e5422c704ffb6d6018261171d8cee98adcf0243f1fd520fb3761afe94a2f4d99f94");
    CCBigNumRef resultExpected = CCBigNumFromHexString(&status, "29eef49086721db0707b81175570af0db1517307f313564ff3a07f695514e92f525efea63ab5de4830c06b365f5c21cba747b96c6cb315d5b3dfa6642324e0a30f0a999acd196c1c78b99bff4d53e1e1e5d02b521a964a286279129e9bfbb51738743617a73f896bfd21e7c4c24e8f72a4a995781b0b4fa1b37a2320b935b0c5e130f27a65135ac4b7247db58e7f752550d92e08b177a84d5d0364cd8c74d4e2bb086d1cd9bc4c541e86ecf66940be30b73675a63921f9fd1fdfa6db57bd4b13304ac0ac84d64c262d5802a1363ee1d519f88b8ca0997a77f7ece081042d88814da526c44f1323c7ac5b7eeedccda0e28bc65bc415bba767d34f161ab34f9788");
    CCBigNumRef output = CCCreateBigNum(&status);
    
    status = CCBigNumMul(output, a, c);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected) == 0, "expected operation result");
    
    CCBigNumFree(a);
    CCBigNumFree(c);
    CCBigNumFree(output);
    CCBigNumFree(resultExpected);
    return 0;
}

static int testMulI()
{
    CCStatus status;
    
    // Shift n right by 11
    CCBigNumRef a = CCBigNumFromHexString(&status, "c968e40c5304364b057425920b18cc358f254ddb0f42f84850d6deec46006b4a692e52b7c3bddead45f77f2c1be1c606521d8a24260429f362d65b57873dbf270e97e210b872e45e97cb4cd87977ad20491e53c48cf0e88da9a61312675a2527c86ac537740c5e4206972f09c0f91fa1c9f14a2cf1be07e82a3b6fd58dc12c3a");
    uint32_t c = 63;
    CCBigNumRef resultExpected = CCBigNumFromHexString(&status, "3190d01f086e095c7657953ef0bb1a412e3a2e28e8c17b19cbe4e0dc253a1a674fe2665b392bb9cca437e84bdadc8fbb8e3544fee55b0652e552c07a8a48320a9c9760a21d644633475b07e945e4739af1fe769d5eaf493adcbfdeb1876f2f24ca524688a58f0b323f9f3493667d4ec8d0b261410f7dc3f22264a0858de289e246");
    CCBigNumRef output = CCCreateBigNum(&status);
    
    status = CCBigNumMulI(output, a, c);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected) == 0, "expected operation result");
    
    CCBigNumFree(a);
    CCBigNumFree(output);
    CCBigNumFree(resultExpected);
    return 0;
}

static int testPrime()
{
    CCStatus status;
    
    // Shift n right by 11
    CCBigNumRef a = CCBigNumFromHexString(&status, "09c75b");
    ok(CCBigNumIsPrime(&status, a), "prime number");
    CCBigNumRef b = CCBigNumFromHexString(&status, "09c75c");
    ok(!CCBigNumIsPrime(&status, b), "not prime number");

    CCBigNumFree(a);
    CCBigNumFree(b);
    return 0;
}


static int testDiv()
{
    CCStatus status;
    
    // Shift n right by 11
    CCBigNumRef a = CCBigNumFromHexString(&status, "c968e40c5304364b057425920b18cc358f254ddb0f42f84850d6deec46006b4a692e52b7c3bddead45f77f2c1be1c606521d8a24260429f362d65b57873dbf270e97e210b872e45e97cb4cd87977ad20491e53c48cf0e88da9a61312675a2527c86ac537740c5e4206972f09c0f91fa1c9f14a2cf1be07e82a3b6fd58dc12c3a");
    CCBigNumRef c = CCBigNumFromHexString(&status, "354c912b09ee7abff5b3d94ed52a9e8dcae582e094daa375c495f970710af73efcc4f9776010511f654c7408a6d5d351ab1d94a0fede757d782b54ddcf6fe8d714870b78b0e67a9754cb03a5cf63bbda1c71791902ea4527fb0cd76437391e5422c704ffb6d6018261171d8cee98adcf0243f1fd520fb3761afe94a2f4d99f94");
    CCBigNumRef resultExpected = CCBigNumFromHexString(&status, "03");
    CCBigNumRef c2 = CCBigNumFromHexString(&status, "43"); // dec 67
    CCBigNumRef resultExpected2 = CCBigNumFromHexString(&status, "30190c6ded9da99f49e63e76ef8ba736d1f1fbaac2b0786cf88f03904dd9e4210da7b80d5121e11dea7fe13c3482557f99574b04cfc3dc2ad6a5d1097089039e578b6f4c9708551a690684fa61c8850b87e8acd70e8d970eb9b5086018ae5ceec10e08e33675949951d00b3ba487e8fc9b35cceffc985d96f6fb16e2c25456f");
    CCBigNumRef output = CCCreateBigNum(&status);
    
    status = CCBigNumDiv(output, NULL, a, c);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected) == 0, "expected operation result");
    
    status = CCBigNumDiv(output, NULL, a, c2);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected2) == 0, "expected operation result");

    CCBigNumFree(a);
    CCBigNumFree(c);
    CCBigNumFree(c2);
    CCBigNumFree(output);
    CCBigNumFree(resultExpected);
    CCBigNumFree(resultExpected2);
    return 0;
}


static int testMulMod()
{
    CCStatus status;
    
    // Shift n right by 11
    CCBigNumRef a = CCBigNumFromHexString(&status, "c968e40c5304364b057425920b18cc358f254ddb0f42f84850d6deec46006b4a692e52b7c3bddead45f77f2c1be1c606521d8a24260429f362d65b57873dbf270e97e210b872e45e97cb4cd87977ad20491e53c48cf0e88da9a61312675a2527c86ac537740c5e4206972f09c0f91fa1c9f14a2cf1be07e82a3b6fd58dc12c3a");
    CCBigNumRef c = CCBigNumFromHexString(&status, "354c912b09ee7abff5b3d94ed52a9e8dcae582e094daa375c495f970710af73efcc4f9776010511f654c7408a6d5d351ab1d94a0fede757d782b54ddcf6fe8d714870b78b0e67a9754cb03a5cf63bbda1c71791902ea4527fb0cd76437391e5422c704ffb6d6018261171d8cee98adcf0243f1fd520fb3761afe94a2f4d99f94");
    CCBigNumRef mod1 = CCBigNumFromHexString(&status, "912b09ee7abff5b3d94ed52a9e8dcae582e094daa375c495f970710af73efcc4f9776010511f654c7408a6d5d351ab1d94a0fede757d782b54ddcf6fe8d714870b78b0e67a9754cb03a5cf63bbda1c71791902ea4527fb0cd76437391e5422c704ffb6d6018261171d8cee98adcf0243f1fd520fb3761afe94a2f4d99f94");
    CCBigNumRef mod2 = CCBigNumFromHexString(&status, "43"); // dec 67
    CCBigNumRef resultExpected1 = CCBigNumFromHexString(&status, "57c1dfcd53105f1f653539172a789fc97067101320d12b93dd400eaad0bbb9d8d9857beeeae28c1ad0614075f1d59bb12556f78d85e4af9d2283fa5d3c192a03a59932dd537d4c1a9d74a8a2d647f266cc4fe9365c5c9b8ac4d5afc960002850243b2175eb09842d1f1cfd59bdd5c2564cb056586d2186aae39583061a88");
    CCBigNumRef resultExpected2 = CCBigNumFromHexString(&status, "3d");
    CCBigNumRef output = CCCreateBigNum(&status);
    
    status = CCBigNumMulMod(output, a, c, mod1);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected1) == 0, "expected operation result");
    
    status = CCBigNumMulMod(output, a, c, mod2);
    ok(status == 0, "operation completed");
    ok(CCBigNumCompare(output, resultExpected2) == 0, "expected operation result");
 
    CCBigNumFree(a);
    CCBigNumFree(c);
    CCBigNumFree(output);
    CCBigNumFree(resultExpected1);
    CCBigNumFree(resultExpected2);
    CCBigNumFree(mod1);
    CCBigNumFree(mod2);
    return 0;
}

int CommonBigNum(int __unused argc, char *const * __unused argv) {

    
	plan_tests(kTestTestCount);

    for(int i=0; i<10; i++) {
        testCreateFree();
        testHexString();
        testDecimalString();
        testData();
        testI();
        testCompare();
        testBitCount();
        testAddSub();
        testAddSubI();
        testShift();
        testMod();
        testModI();
        testModExp();
        testMul();
        testMulI();
        testPrime();
        testDiv();
        testMulMod();
    }

    return 0;
}
#endif /* CCBigNum */
