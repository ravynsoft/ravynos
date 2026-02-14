//
//  CCCMACtests.c
//  CCRegressions
//

#include <stdio.h>
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

#if (CCCMAC == 0)
entryPoint(CommonCMac,"Common CMac")
#else

#include <CommonCrypto/CommonCMACSPI.h>

static void CCAES_Discrete_Cmac(const void *key, const uint8_t *data, size_t dataLength, void *macOut) {
    CCCmacContextPtr cmac = CCAESCmacCreate(key, 16);
    if(!cmac) return;
    for(size_t i=0; i<dataLength; i++) {
        CCAESCmacUpdate(cmac, data+i, 1);
    }
    CCAESCmacFinal(cmac, macOut);
    CCAESCmacDestroy(cmac);
}

static void CCAES_Discrete_Cmac_EvenChunk(const void *key, const uint8_t *data, size_t dataLength, void *macOut) {
    const size_t chunksize = 64;
    CCCmacContextPtr cmac = CCAESCmacCreate(key, 16);
    if(!cmac) return;
    
    if(dataLength > chunksize) {
        CCAESCmacUpdate(cmac, data, chunksize);
        dataLength -= chunksize; data += chunksize;
    }
    
    CCAESCmacUpdate(cmac, data, dataLength);
    CCAESCmacFinal(cmac, macOut);
    CCAESCmacDestroy(cmac);
}


static int
CMACTest(char *input, char *keystr, char *expected)
{
    byteBuffer mdBufdiscrete, mdBufEvenChunk, mdBufoneshot;
    byteBuffer inputBytes, expectedBytes, keyBytes;
    char *digestName;
    char outbuf[2048];
    int retval = 0;
    
    inputBytes = hexStringToBytes(input);
    expectedBytes = hexStringToBytes(expected);
    keyBytes = hexStringToBytes(keystr);

    mdBufdiscrete = mallocByteBuffer(CC_CMACAES_DIGEST_LENGTH); digestName = "CMAC-AES";
    mdBufoneshot = mallocByteBuffer(CC_CMACAES_DIGEST_LENGTH);
    mdBufEvenChunk = mallocByteBuffer(CC_CMACAES_DIGEST_LENGTH);

    CCAES_Discrete_Cmac(keyBytes->bytes, inputBytes->bytes, inputBytes->len, mdBufdiscrete->bytes);
    CCAES_Discrete_Cmac_EvenChunk(keyBytes->bytes, inputBytes->bytes, inputBytes->len, mdBufEvenChunk->bytes);
    CCAESCmac(keyBytes->bytes, inputBytes->bytes, inputBytes->len, mdBufoneshot->bytes);
    
	sprintf(outbuf, "Hmac-%s test for %s", digestName, input);

    ok(bytesAreEqual(mdBufdiscrete, expectedBytes), outbuf);
    ok(bytesAreEqual(mdBufEvenChunk, expectedBytes), outbuf);
    ok(bytesAreEqual(mdBufoneshot, expectedBytes), outbuf);

    if(!bytesAreEqual(mdBufdiscrete, expectedBytes)) {
        diag("HMAC FAIL: HMAC-%s(\"%s\")\n expected %s\n      got %s\n", digestName, input, expected, bytesToHexString(mdBufdiscrete));
        retval = 1;
    } else {
        // diag("HMAC PASS: HMAC-%s(\"%s\")\n", digestName, input);
    }

    free(mdBufdiscrete);
    free(mdBufEvenChunk);
    free(mdBufoneshot);
    free(expectedBytes);
    free(keyBytes);
    free(inputBytes);
    return retval;
}

static int kTestTestCount = 5;

int CommonCMac (int __unused argc, char *const * __unused argv) {
	char *strvalue, *keyvalue;
	plan_tests(kTestTestCount*3+2);
    int accum = 0;
    const unsigned char key[16]={0};
    ok(CCAESCmacCreate(key, 15)==NULL,  "Detect incorrect key length");
    ok(CCAESCmacCreate(NULL, 16)==NULL, "Detect incorrect key pointer");

    strvalue = "";
    keyvalue = "2b7e151628aed2a6abf7158809cf4f3c";
	accum |= CMACTest(strvalue, keyvalue, "bb1d6929e95937287fa37d129b756746");   
    strvalue = "6bc1bee22e409f96e93d7e117393172a";
	accum |= CMACTest(strvalue, keyvalue, "070a16b46b4d4144f79bdd9dd04a287c");   
    strvalue = "6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e5130c81c46a35ce411";
	accum |= CMACTest(strvalue, keyvalue, "dfa66747de9ae63030ca32611497c827");   
    strvalue = "6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e5130c81c46a35ce411e5fbc1191a0a52eff69f2445df4f9b17ad2b417be66c3710";
	accum |= CMACTest(strvalue, keyvalue, "51f0bebf7e3b9d92fc49741779363cfe");  
    
    keyvalue = "eb7dea983b2bc12585799e733ab3c473";
    strvalue = "fe534d4240000100000000000300010009000000000000000c00000000000000fffe00000100000003000000540005c400000000000000000000000000000000100001003008000000000000ff011f00";
    accum |= CMACTest(strvalue, keyvalue, "7532b859e70ad6692e24b747f5f4b44d");

    return accum;
}
#endif


