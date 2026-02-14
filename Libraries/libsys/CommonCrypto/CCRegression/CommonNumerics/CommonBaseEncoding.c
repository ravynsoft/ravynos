
//
//  CNEncoding.c
//  CCRegressions
//
#include <stdio.h>
#include "testbyteBuffer.h"
#include "capabilities.h"
#include "testmore.h"

#if (CNENCODER == 0)
entryPoint(CommonBaseEncoding,"Base XX Encoding")
#else

#include <CommonNumerics/CommonBaseXX.h>

static int kTestTestCount = 416;

#define BUFSIZE 1024

static int
doWithEncoders(CNEncoderRef encoder, CNEncoderRef decoder, char *input, char *expected)
{
    CNStatus retval;
    char outBuf[BUFSIZE], secondBuf[BUFSIZE];
    size_t outLen, outAvailable, secondLen, secondAvailable;
    
    outAvailable = BUFSIZE;

    retval = CNEncoderUpdate(encoder, input, strlen(input), outBuf, &outAvailable);
    
    ok(retval == kCNSuccess, "encoded");
    
    outLen = outAvailable;
    outAvailable = BUFSIZE - outLen;
    
    retval = CNEncoderFinal(encoder, &outBuf[outLen], &outAvailable);
    
    ok(retval == kCNSuccess, "finalized");
    
    outLen += outAvailable;
    
    if(expected) {
        ok(strncmp(expected, outBuf, strlen(expected)) == 0, "output matches");
        if(strncmp(expected, outBuf, strlen(expected))) {
            printf("Encoding %s\n%s\n%s\n", input, expected, outBuf);
        }
    }
    
    retval = CNEncoderRelease(&encoder);
    
    ok(retval == kCNSuccess, "released");
    
    
    secondAvailable = BUFSIZE;
    secondLen = 0;
    retval = CNEncoderUpdate(decoder, outBuf, outLen, secondBuf, &secondAvailable);
    
    ok(retval == kCNSuccess, "encoded");
    
    
    secondLen = secondAvailable;
    secondAvailable = BUFSIZE - secondLen;
    
    retval = CNEncoderFinal(decoder, &secondBuf[secondLen], &secondLen);
    
    ok(retval == kCNSuccess, "finalized");
    
    secondLen += secondAvailable;
    
    ok(strncmp(input, secondBuf, strlen(input)) == 0, "output matches");
    
    retval = CNEncoderRelease(&decoder);
    ok(retval == kCNSuccess, "encoder Released");
    
    return 0;
}


static int
doEncoder(CNEncodings encodingStrat, char *input, char *expected)
{
    CNStatus retval;
    CNEncoderRef encoder, decoder;
        
    retval = CNEncoderCreate(encodingStrat, kCNEncode, &encoder);
        
    ok(retval == kCNSuccess, "got an encoder");
    
    retval = CNEncoderCreate(encodingStrat, kCNDecode, &decoder);
    
    ok(retval == kCNSuccess, "got a decoder");
    

    doWithEncoders(encoder, decoder, input, expected);
    
    return 0;
}


static int
doCustomEncoder(const char *name,
                const uint8_t baseNum,
                const char *charMap,
                const uint8_t padChar, char *input, char *expected)
{
    CNStatus retval;
    CNEncoderRef encoder, decoder;
    
    retval = CNEncoderCreateCustom(name, baseNum, charMap, padChar, kCNEncode, &encoder);
    
    ok(retval == kCNSuccess, "got an encoder");
        
    retval = CNEncoderCreateCustom(name, baseNum, charMap, padChar, kCNDecode, &decoder);
    
    ok(retval == kCNSuccess, "got a decoder");
    
    doWithEncoders(encoder, decoder, input, expected);
    return 0;
}

static int
doOneShotStyle(CNEncodings encodingStrat, char *input, char *expected)
{
    CNStatus retval;
    char outBuf[BUFSIZE], secondBuf[BUFSIZE];
    size_t outLen, outAvailable, secondAvailable;
    
    outAvailable = BUFSIZE;
    retval = CNEncode(encodingStrat, kCNEncode, input, strlen(input), outBuf, &outAvailable);
    ok(retval == kCNSuccess, "CNEncode passes");
    outLen = outAvailable;
    
    if(expected) {
        ok(strncmp(expected, outBuf, strlen(expected)) == 0, "output matches");
        if(strncmp(expected, outBuf, strlen(expected))) {
            printf("Encoding %s\n%s\n%s\n", input, expected, outBuf);
        }
    }
        
    
    secondAvailable = BUFSIZE;
    retval = CNEncode(encodingStrat, kCNDecode, outBuf, outLen, secondBuf, &secondAvailable);
    ok(retval == kCNSuccess, "CNEncode passes");
    
    ok(strncmp(input, secondBuf, strlen(input)) == 0, "output matches");
    if(strncmp(input, secondBuf, strlen(input))) {
        printf("input:  %s\n", input);
        printf("result: %s\n", secondBuf);
    }
    
    return 0;    
}

    

int CommonBaseEncoding(int __unused argc, char *const * __unused argv) {
    int accum = 0;
    int verbose = 0;
    
    plan_tests(kTestTestCount);
    
    // diag("Base64\n");
    
	doEncoder(kCNEncodingBase64, "", ""); 
	doEncoder(kCNEncodingBase64, "f", "Zg=="); 
	doEncoder(kCNEncodingBase64, "fo", "Zm8="); 
	doEncoder(kCNEncodingBase64, "foo", "Zm9v"); 
	doEncoder(kCNEncodingBase64, "foob", "Zm9vYg=="); 
	doEncoder(kCNEncodingBase64, "fooba", "Zm9vYmE="); 
	doEncoder(kCNEncodingBase64, "foobar", "Zm9vYmFy"); 
    
    //diag("Base32\n");

	doEncoder(kCNEncodingBase32, "", ""); 
	doEncoder(kCNEncodingBase32, "f", "MY======"); 
	doEncoder(kCNEncodingBase32, "fo", "MZXQ===="); 
	doEncoder(kCNEncodingBase32, "foo", "MZXW6==="); 
	doEncoder(kCNEncodingBase32, "foob", "MZXW6YQ="); 
	doEncoder(kCNEncodingBase32, "fooba", "MZXW6YTB"); 
	doEncoder(kCNEncodingBase32, "foobar", "MZXW6YTBOI======");
    
    //diag("Base32HEX\n");

	doEncoder(kCNEncodingBase32HEX, "", ""); 
	doEncoder(kCNEncodingBase32HEX, "f", "CO======"); 
	doEncoder(kCNEncodingBase32HEX, "fo", "CPNG===="); 
	doEncoder(kCNEncodingBase32HEX, "foo", "CPNMU==="); 
	doEncoder(kCNEncodingBase32HEX, "foob", "CPNMUOG="); 
	doEncoder(kCNEncodingBase32HEX, "fooba", "CPNMUOJ1"); 
	doEncoder(kCNEncodingBase32HEX, "foobar", "CPNMUOJ1E8======"); 
    
    //diag("Base16\n");

	doEncoder(kCNEncodingBase16, "", ""); 
	doEncoder(kCNEncodingBase16, "f", "66"); 
	doEncoder(kCNEncodingBase16, "fo", "666F"); 
	doEncoder(kCNEncodingBase16, "foo", "666F6F"); 
	doEncoder(kCNEncodingBase16, "foob", "666F6F62"); 
	doEncoder(kCNEncodingBase16, "fooba", "666F6F6261"); 
	doEncoder(kCNEncodingBase16, "foobar", "666F6F626172"); 
    
    //diag("Base64 Long\n");
    accum |= doEncoder(kCNEncodingBase64, 
    "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.",
                       "");
    
    if(verbose) diag("Base64 - One-Shot\n");
    
	doOneShotStyle(kCNEncodingBase64, "", ""); 
	doOneShotStyle(kCNEncodingBase64, "f", "Zg=="); 
	doOneShotStyle(kCNEncodingBase64, "fo", "Zm8="); 
	doOneShotStyle(kCNEncodingBase64, "foo", "Zm9v"); 
	doOneShotStyle(kCNEncodingBase64, "foob", "Zm9vYg=="); 
	doOneShotStyle(kCNEncodingBase64, "fooba", "Zm9vYmE="); 
	doOneShotStyle(kCNEncodingBase64, "foobar", "Zm9vYmFy"); 
    
    if(verbose) diag("Base32 - One-Shot\n");
    
	doOneShotStyle(kCNEncodingBase32, "", ""); 
	doOneShotStyle(kCNEncodingBase32, "f", "MY======"); 
	doOneShotStyle(kCNEncodingBase32, "fo", "MZXQ===="); 
	doOneShotStyle(kCNEncodingBase32, "foo", "MZXW6==="); 
	doOneShotStyle(kCNEncodingBase32, "foob", "MZXW6YQ="); 
	doOneShotStyle(kCNEncodingBase32, "fooba", "MZXW6YTB"); 
	doOneShotStyle(kCNEncodingBase32, "foobar", "MZXW6YTBOI======");
    
    if(verbose) diag("Base32HEX - One-Shot\n");
    
	doOneShotStyle(kCNEncodingBase32HEX, "", ""); 
	doOneShotStyle(kCNEncodingBase32HEX, "f", "CO======"); 
	doOneShotStyle(kCNEncodingBase32HEX, "fo", "CPNG===="); 
	doOneShotStyle(kCNEncodingBase32HEX, "foo", "CPNMU==="); 
	doOneShotStyle(kCNEncodingBase32HEX, "foob", "CPNMUOG="); 
	doOneShotStyle(kCNEncodingBase32HEX, "fooba", "CPNMUOJ1"); 
	doOneShotStyle(kCNEncodingBase32HEX, "foobar", "CPNMUOJ1E8======"); 
    
    if(verbose) diag("Base16 - One-Shot\n");
    
	doOneShotStyle(kCNEncodingBase16, "", ""); 
	doOneShotStyle(kCNEncodingBase16, "f", "66"); 
	doOneShotStyle(kCNEncodingBase16, "fo", "666F"); 
	doOneShotStyle(kCNEncodingBase16, "foo", "666F6F"); 
	doOneShotStyle(kCNEncodingBase16, "foob", "666F6F62"); 
	doOneShotStyle(kCNEncodingBase16, "fooba", "666F6F6261"); 
	doOneShotStyle(kCNEncodingBase16, "foobar", "666F6F626172"); 
    
    if(verbose) diag("Base64 Long - One-Shot\n");
    accum |= doOneShotStyle(kCNEncodingBase64, 
                       "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.",
                       "");

    
    
    
    if(verbose) diag("Custom\n");
    accum |= doCustomEncoder("Custom64", 64, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+/", '*',
                       "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.",
                       "");
    
    return accum;
}

#endif

