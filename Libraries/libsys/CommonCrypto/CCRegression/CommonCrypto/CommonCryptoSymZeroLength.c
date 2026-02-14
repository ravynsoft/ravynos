#include <stdio.h>
#include <CommonCrypto/CommonCryptor.h>
#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

#if (CCSYMZEROLEN == 0)
entryPoint(CommonCryptoSymZeroLength,"CommonCrypto Symmetric Zero-Length Testing")
#else

static int kTestTestCount = 4;


int CommonCryptoSymZeroLength (int __unused argc, char *const * __unused argv) 
{
	char *keyStr;
	char *iv;
	char *plainText;
	char *cipherText;
    int retval, accum = 0;

	plan_tests(kTestTestCount);

	/* Two Test cases - "" and NULL WITH an IV */
        
	keyStr 	   = "000102030405060708090a0b0c0d0e0f";
	iv         = "0f0e0d0c0b0a09080706050403020100";

	// 1
	plainText  = "";
	cipherText = "efddc425a6fa0c5f25e444092eb0f503";
    retval = CCCryptTestCase(keyStr, iv, kCCAlgorithmAES128, kCCOptionPKCS7Padding, cipherText, plainText, true);
    ok(retval == 0, "CBC Zero Length String, IV defined");
    accum += retval;
    

	// 1
	plainText  = NULL;
	cipherText = "efddc425a6fa0c5f25e444092eb0f503";
    retval = CCCryptTestCase(keyStr, iv, kCCAlgorithmAES128, kCCOptionPKCS7Padding, cipherText, plainText, true);
    ok(retval == 0, "CBC NULL String, IV defined");
    accum += retval;
    
	/* Two more Test cases - "" and NULL WITH IV=NULL */
        
	keyStr 	   = "000102030405060708090a0b0c0d0e0f";
	iv         = NULL;

	// 1
	plainText  = "";
	cipherText = "954f64f2e4e86e9eee82d20216684899";
    retval = CCCryptTestCase(keyStr, iv, kCCAlgorithmAES128, kCCOptionPKCS7Padding, cipherText, plainText, true);
    ok(retval == 0, "CBC Zero Length String, IV NULL");
    accum += retval;
    

	// 1
	plainText  = NULL;
	cipherText = "954f64f2e4e86e9eee82d20216684899";
    retval = CCCryptTestCase(keyStr, iv, kCCAlgorithmAES128, kCCOptionPKCS7Padding, cipherText, plainText, true);
    //retval = 0;
    ok(retval == 0, "CBC NULL String, IV NULL");
    accum += retval;

    return accum != 0;
}
#endif

