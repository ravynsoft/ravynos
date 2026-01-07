//
//  CCXTSTest.c
//  CCRegressions

#include <stdio.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonCryptorSPI.h>
#include "CCCryptorTestFuncs.h"
#include "testbyteBuffer.h"
#include "testmore.h"

#if (CCSYMXTS == 0)
entryPoint(CommonCryptoSymXTS,"CommonCrypto Symmetric XTS Testing")
#else

static int kTestTestCount = 1000;

/*
 #  CAVS 9.0
 #  XTSGen information for "sample";
 #  State tested: Encrypt/Decrypt
 #  combinedKey Length:  AES128
 #  Data Unit Lengths Tested: 128 256 192 576 4096
 #  Generated on Wed Mar 31 11:08:59 2010
 */

#define ENCRYPT 0
#define DECRYPT 1


static void
doXTSTestCase(int  __unused caseNumber, int direction, int dataLenBits, char *ivStr, char *cipherText, char *plainText, char *combinedKey)
{
	char keyString[65], twkString[65];
    size_t ckLen;
    byteBuffer key, tweak, iv;
    byteBuffer pt, ct;
	CCCryptorRef encCryptorRef;
	CCCryptorStatus retval;
    char dataOut[4096];
    int dataLen;
    
    dataLen = dataLenBits / 8;
    ckLen = strlen(combinedKey)/2;
    strncpy(keyString, combinedKey, ckLen);
    keyString[ckLen] = 0;
    strncpy(twkString, combinedKey+ckLen, ckLen);
    twkString[ckLen] = 0;
        
    key = hexStringToBytes(keyString);
    tweak = hexStringToBytes(twkString);
    iv = hexStringToBytes(ivStr);
    
    pt = hexStringToBytes(plainText);
    ct = hexStringToBytes(cipherText);

    if((retval = CCCryptorCreateWithMode(0, kCCModeXTS, kCCAlgorithmAES128, ccNoPadding, NULL, key->bytes, key->len, tweak->bytes, tweak->len, 0, 0,  &encCryptorRef)) == kCCSuccess) {
        if(direction == ENCRYPT) {
            if((retval = CCCryptorEncryptDataBlock(encCryptorRef, iv->bytes, pt->bytes, dataLen, dataOut)) == kCCSuccess) {
                byteBuffer output = bytesToBytes(dataOut, dataLen);
                ok(bytesAreEqual(output, ct), "Bytes are Equal to expected value");
                free(output);
            } else  printf("Failed to encrypt %d\n", retval);
            
        } else {
            if((retval = CCCryptorDecryptDataBlock(encCryptorRef, iv->bytes, ct->bytes, dataLen, dataOut)) == kCCSuccess) {
                byteBuffer output = bytesToBytes(dataOut, dataLen);
                ok(bytesAreEqual(output, pt), "Bytes are Equal to expected value");
                free(output);
        	} else  printf("Failed to decrypt %d\n", retval);
    	}
        
        if((retval = CCCryptorRelease(encCryptorRef)) != kCCSuccess) printf("Finalize failed\n");
    } else {
        printf("Failed to create Cryptor\n");
    }
    
    
    free(pt);
    free(ct);
    free(key);
    free(tweak);
    free(iv);
    
}

int CommonCryptoSymXTS(int __unused argc, char *const * __unused argv) {
	int direction;
	int caseNumber;
	int dataLen;
	char *combinedKey;
	char *iv;
	char *plainText;
	char *cipherText;
        
	plan_tests(kTestTestCount);
    	
	direction = ENCRYPT;
        
	caseNumber = 1;
	dataLen = 128;
	combinedKey = "46e6ed9ef42dcdb3c893093c28e1fc0f91f5caa3b6e0bc5a14e783215c1d5b61";
	iv = "72f3b054cbdc2f9e3c5bc551d44ddba0";
	plainText = "e3778d68e730ef945b4ae3bc5b936bdd";
	cipherText = "97409f1f71ae4521cb49a32973de4d05";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);

	caseNumber = 2;
	dataLen = 128;
	combinedKey = "9356cdad251ab61114cec2c44a6092dde9f746cc65ae3bd4966864aa3626d188";
	iv = "68882783652436c4857a88c0c373417e";
	plainText = "ce176bdde339505ba15dea36d28ce87d";
	cipherText = "22f5f937dfb39e5b7425ed863d310be1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 3;
	dataLen = 128;
	combinedKey = "c3daa5a0b67699f781050478cbfb8e93f381f71663d4c17375df9112c46a1863";
	iv = "26be92ac9883cea1da85335f8c169edb";
	plainText = "d988cd5db0502786c519b71cb38f4c62";
	cipherText = "e90a71b4bcd1baf3eb1f51ffecc464a9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 4;
	dataLen = 128;
	combinedKey = "924d6e017be6ed2686f4b2b7f50629b860c5e0a51d40f0c57847fc3fd9e79c48";
	iv = "0f5a522a6d4bc6650531860ac6223471";
	plainText = "0d8e5ba22f3fedb14ab6ab2fce9b0f84";
	cipherText = "b80a22c1e6522c47844158774b55131e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 5;
	dataLen = 128;
	combinedKey = "5a7d8971136edc537e767135958c13fa7549c059cd14a2f19abaa86aad09b56c";
	iv = "4108e1a5e460b29a52f1d960f7d8541d";
	plainText = "9634a03111720ef421e6dd5a6a9869c3";
	cipherText = "8437d5973fae25c6e5f3fd6795e5ea7a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 6;
	dataLen = 128;
	combinedKey = "e70c3574ba7d4be13d944c87c94ec8c5bf7b7fb10e1bb34c326af55c2385cfc2";
	iv = "200fc1e13044c4d473f410b91a26402f";
	plainText = "109184aefa5ebade528f50f497819af9";
	cipherText = "1e4cccaf01de731a3b5d5882b2db3af6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 7;
	dataLen = 128;
	combinedKey = "aa94d75ee53206db4ebc4d0bd8b5c0aa7600677c620bbe2a6c08bde8d6b2ebc5";
	iv = "bafe6f820df92ae653e70fce03159093";
	plainText = "b425ce1c7d7524404d2a2be5de5c1c28";
	cipherText = "d2c2c145a71daf7bd3339596474439ba";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 8;
	dataLen = 128;
	combinedKey = "9bb0eaf85277f24f15a3d86aed6c9cc5a218531886efb7c98be7a864b8c1d365";
	iv = "d4d6ad1e9d90f44c08c30dd9216c984a";
	plainText = "ca212d69459d37e284b8e03cfc99c1ad";
	cipherText = "09d9be40648420ee46141d717654c569";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 9;
	dataLen = 128;
	combinedKey = "af558471cdedddf76da56c3428ec8977b812152d386ed35dcbc8f833a36cc612";
	iv = "6f19db800dc2f23642bbbf1348d8141c";
	plainText = "22590e7d9368bb8c3fbf543144704fec";
	cipherText = "a394535e6069737682389f44d29b1f04";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 10;
	dataLen = 128;
	combinedKey = "b193c8d188a759da598bf5cd2e2c18a714dfcc29d4872c379208769b3c24c754";
	iv = "985a005543fe821949ff374d14abac1f";
	plainText = "5c916b0688c37acc2d413d37e941becf";
	cipherText = "1b3dfebd87bca7e745bdd5969280e3b4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 11;
	dataLen = 128;
	combinedKey = "9eeb844d6133cc6250015374883d394d99756aa7c9ff87b263b2ed8623a343a1";
	iv = "ea49deb5767de58ac448ecd0870b1133";
	plainText = "e94be598b8b79af4a96b0a75eff1624c";
	cipherText = "de1bc0e8065f47725abcd960eb7b0007";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 12;
	dataLen = 128;
	combinedKey = "0b9272f968591908540cdc73bdc4e21e675afecea6b6ed11787f879a35ac8a76";
	iv = "ef8b4a7f6840d527139f27cd12d5b503";
	plainText = "26adc7c756b66b768c18ca524bc7eaf4";
	cipherText = "4910b9bf93e6f5396c6dc1fbca34ae93";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 13;
	dataLen = 128;
	combinedKey = "d1e5752b6d4edcba52c7cdc783d639deb4949b7c0fd49f7e990758e93d2427a7";
	iv = "999568ef48e3e5be3ee183d1cf6cadc6";
	plainText = "ebadc47d64132e5a947b2bbb6cb74de7";
	cipherText = "e12ec44d0a161af9a2f610d3d7ee80cb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 14;
	dataLen = 128;
	combinedKey = "8343d923672c7251dac3c0e2b4ba938ddda625d96d87f65728dbc1c7a19d7956";
	iv = "d2985181e51af64ca2fe258b0532f01e";
	plainText = "ea36ad6982084bae3dfb5da13a848bc9";
	cipherText = "5756b2c27e3759d8aac41c112ce46f1d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 15;
	dataLen = 128;
	combinedKey = "9bdea2cb7980cf3b6ad584aff48cd9b48bcaa04a22d2d285f987c7472ea26462";
	iv = "314b49ef42bead17d471edb4f7bd8249";
	plainText = "593fddd423b07dd305e6fa755646eb87";
	cipherText = "c4c90e697472d8654728e476686707b5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 16;
	dataLen = 128;
	combinedKey = "a9d1d6fc95145da71f855ca00987583a31dd2e8042b7f2cf18b401f73d50b455";
	iv = "02371d129cb6549113eae4fdb4f33a58";
	plainText = "a83389303b071ada2ad63fb022ce8185";
	cipherText = "fc007f02d048b0fd5ac1f4961ad371d4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 17;
	dataLen = 128;
	combinedKey = "e43aa519a45d3995f1ef7de5ffebb1ef953c1e725489dc6a79c44b8339cb4ede";
	iv = "6fdcc5d82dda465558ea491f2ee95965";
	plainText = "44f98b934211c32167e6ca984fa72f48";
	cipherText = "0e6b661c07514e535e154d4fece0efbb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 18;
	dataLen = 128;
	combinedKey = "05331feac8e26c7abf7b0199471b1b3156046f7d5d6366e0dfcd58d16b7fd7bb";
	iv = "3231245be6cdbf7ba62e72744d8d5d44";
	plainText = "5b0003774dedbee211b62b14389129f8";
	cipherText = "14fc9642efb9d7cbac1535f849c25329";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 19;
	dataLen = 128;
	combinedKey = "ebdb518a8fda4dc1626865f0152c874f0b939c2f5cff005c3ff73f40fefcccb6";
	iv = "dc88a417430c423d03fda7592b9dcfed";
	plainText = "f7c296278e13d62c8d7a6d0423897769";
	cipherText = "5c2331739d4719b4b17641732d1d507c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 20;
	dataLen = 128;
	combinedKey = "8562619696b172f713f292e4e1f6d8b70f786ff2ff737c259a14ae0f4e453235";
	iv = "4c3f1509a5913c01e008a90b16345199";
	plainText = "362357e4d3941c67f93510bc84eb87ef";
	cipherText = "d837806c7dc8568b196236499b901eab";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 21;
	dataLen = 128;
	combinedKey = "b9c8c973664be2404ff6b8939f0f43597b84f4bf3ac8a2c34586e37a12a55c8c";
	iv = "8f6543078cb5544cfdded59f997236c2";
	plainText = "e23581d57ad3da3338f89244f7e53226";
	cipherText = "003c2ef98b83e55e033a7d19baf07209";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 22;
	dataLen = 128;
	combinedKey = "a6fbbf745b5846556beea67b2e9cab498b7635b674b9e0c999bf66d09a7398d0";
	iv = "2b3fbde91758802b76a7c980d42b43a7";
	plainText = "2d804bf5cda27a0eb3f7e0de08637abe";
	cipherText = "48b2778badffa7bfff409c3671e2c231";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 23;
	dataLen = 128;
	combinedKey = "e345a20273cf591c5b8ac1078f4fc21c80b8a55d1b3d7c89c548b5ae133ad33d";
	iv = "09083e8b63955a9e7589f2fe426c3923";
	plainText = "8aafab9e4004337c4a25432829bfc64b";
	cipherText = "f4abb4567ee454ce8900207addb64255";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 24;
	dataLen = 128;
	combinedKey = "d9bb58abf817a3819de1f9019dbc419d54e017e0dde1f3e9362db83adeede286";
	iv = "b28fc90a246ad2e7b312b39521324566";
	plainText = "5ce6cb2cb0d7c47267daa31fb72645da";
	cipherText = "6a9429e36ad17e3ca724c0638022baa5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 25;
	dataLen = 128;
	combinedKey = "9dbd6ff6274013ec4899f545172533a536706d896f543086d125c7b89c745a3e";
	iv = "22f7ff47ee95c5a92699c670ee187ce2";
	plainText = "0eba4b729eddc6afe88b8023d70ce102";
	cipherText = "3fbf5577e484aaef548bb4caef1b23ef";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 26;
	dataLen = 128;
	combinedKey = "be1627c0dd8151c2598ba752c0088453c9a21a37b9f1089d089b2b196fd88e03";
	iv = "d45e2ece32961f017b325583e8ee464e";
	plainText = "d353fea07c903a8ff976812cf3e61413";
	cipherText = "27f2cb552e96fc9c2c5c35e3787a1b3b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 27;
	dataLen = 128;
	combinedKey = "2b94e2c5ced0d24e89b459a073c66b4b0dfd051217a07dd429661291ac4cedb3";
	iv = "8d3eb008b842709a6a243b350cc1a7de";
	plainText = "c3af28e1c47b04adff1320b479a944c7";
	cipherText = "4f32d026c8f7bf0ddca47e155ef207a7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 28;
	dataLen = 128;
	combinedKey = "5b2374e8a8a9b8236ae1738583aaad5461abbb80ea14fb8f143965f0c612fa90";
	iv = "22f9721dde3fbe8dc8ad24078249fb4d";
	plainText = "65708c99b1ef29119f39833f992b05d9";
	cipherText = "be21c7ce2f2969bd4c918fd847cf8c0e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 29;
	dataLen = 128;
	combinedKey = "7362e35382a84edcdf02a858d24c6e1239a4feefeb3840bc440cbc20b38d9ab1";
	iv = "b268a26206a4161e7f00bbe7d1de3aec";
	plainText = "3a9c8280677c9f00a9c8bfc5eb2b7552";
	cipherText = "1ecc638cb3f8cc1beaec26109973a313";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 30;
	dataLen = 128;
	combinedKey = "a0d42681a867aa924323c1dfef7ccbb9a8ec0885b2528f0ac86c942b39531a74";
	iv = "bd0d971dfdaa454acdafc8a992dbdd50";
	plainText = "ab4f20d49ae5579069044aa261875638";
	cipherText = "b9671ea04b324745bb1a0ebdb2e074f2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 31;
	dataLen = 128;
	combinedKey = "a0aa082c8ecfc68787fac6c0e78d7fe9c99bebe150300bacb74136b7cd9810e6";
	iv = "c9b3f621a1374e0eda23376ca2c26fd0";
	plainText = "1d9ede1aafa7af65da4b56bd65e95d18";
	cipherText = "072dc9fc2631552242253fccb7ff34d6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 32;
	dataLen = 128;
	combinedKey = "5f6710301f15c2a1e350db816a7dc2baea8038b1de58b23271f00e1bbdba291e";
	iv = "e83aed0175373f8110e5bc0cb0ed5ebe";
	plainText = "2f640069cd78d0c7433c126e7533d837";
	cipherText = "b147bfbbbafeefedd7fea3f985475c8c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 33;
	dataLen = 128;
	combinedKey = "e1f3b00669ef69c1a13cc3fc2612d69e59a817cdb147392c319a7e16fab86b73";
	iv = "b94b110641d6f9c9882b880dcf39477a";
	plainText = "5c11f78d35c075e69cc89c99f8e6ffeb";
	cipherText = "6bfaf03085a13d4bf791dd73377671a5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 34;
	dataLen = 128;
	combinedKey = "71c29ff5285aeca9778d4e3c2c189015aa1b809e9925920389ec02c05171352a";
	iv = "16413f69056a1d0cd00676f5c6006a0f";
	plainText = "c96302351d0b1f012033e48f61b34d54";
	cipherText = "03e93d77d1d87f80fe938135b1bafd55";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 35;
	dataLen = 128;
	combinedKey = "704e0bdf9bb04b45a77206875521d0389645761d17cf9e1d11e108775747f02b";
	iv = "a19f4719fda33bbe44f91bd595badb37";
	plainText = "80fc4ae7787a8295171ab7ab3bd60e53";
	cipherText = "aa5278853d472e1fbc6cf86cd83c34e9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 36;
	dataLen = 128;
	combinedKey = "832f3883c68141587892056f63fa07d8ac6828a065b4a7504b910d4a6ab7e1a2";
	iv = "850f6075d946c84ba3eb43045c73d9db";
	plainText = "6e3e792aee862cbbeb8d1e9379d6c5b9";
	cipherText = "b29ba178d9a3550c2a1fa6e8d62b3ad6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 37;
	dataLen = 128;
	combinedKey = "cca67d91764890c26b04ec62822b98b63879ce4dc6abdc680564660f2dcb6eb6";
	iv = "3d15f99e28819305095dcea7fd62efea";
	plainText = "f9a767fbd3f43ad79774140d56a3b544";
	cipherText = "f21615ca2cf2ab30e65c933dfe814dc8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 38;
	dataLen = 128;
	combinedKey = "0979ef8f8fc0ef73e6a51af3289e63bad7453d2f7c460deac6533f39a98194dc";
	iv = "a3c446f313ed8f05622a969c7c58ec03";
	plainText = "21fb484c68a9caf60fde8d22994751aa";
	cipherText = "b4965ff7237bec62268d98723a6b8d22";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 39;
	dataLen = 128;
	combinedKey = "8c1022158815a2072388dec097648836d300d8eb4c0fec669edd585a00b09461";
	iv = "b4e1d80d8009246e026b243ab8e21fc1";
	plainText = "e6e3d67a7e32dcda953b5c9aef6e2468";
	cipherText = "410d67bc7b0b9db8cd285027db2f142f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 40;
	dataLen = 128;
	combinedKey = "6805e0920fdc81817c16ff312042812d4d0fea04e97a0088d5ce033f77f57516";
	iv = "6782e9e185ee7c2dbd5f20739f7470b8";
	plainText = "32f03cab33cfe6870946d8ef9926d40f";
	cipherText = "0d44276de44fcc1a22c0a23f5947741a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 41;
	dataLen = 128;
	combinedKey = "13de7c37bc706262b3dd21e512f7a1c304b0b8a1a5ac990c0d7d44beb7c7c241";
	iv = "d283e9a36ce280bf92ad05b9d1070194";
	plainText = "de8369c5f7b7efaa5d693ccfbc1ceaa2";
	cipherText = "11a61b1439b00d4723957dcbd1d283c2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 42;
	dataLen = 128;
	combinedKey = "c6986f1ff45871a394fe42bb5f28bd1fe94587c0da5211a9da75b98fa38708cf";
	iv = "1c87714aae7220b31a97f7c158dbc414";
	plainText = "71fdf4428222172b949efb7d6b5b4b01";
	cipherText = "6fdab096ae06c8981ffddff8b924d4f3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 43;
	dataLen = 128;
	combinedKey = "0ddf33773e2aa3b1ae099bb3a7196af03808f9b4067c9352c895e4569908e279";
	iv = "cd2e892985428763375455ddde7aaea1";
	plainText = "b49af688227f7e370d419e4375d0934a";
	cipherText = "db905a71789d633bceaa95f91b3f79e2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 44;
	dataLen = 128;
	combinedKey = "c292b87b0420866b6cfdfd176a6ee3e53a16cf0c4714fd04c7415dd664957690";
	iv = "a1f87b4d8c7b5c39ba079b70f183074f";
	plainText = "2328fb489b22b4b641e9e220b8908ee5";
	cipherText = "3cb229bc84c14196e648ce4557e28aef";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 45;
	dataLen = 128;
	combinedKey = "c5e3f3a4602298c3b06ab95709fc75d1c6127f940732b8d7e19de13060c3c870";
	iv = "a691bd37bcbc1feb55dcbc0dd8741596";
	plainText = "3fc1ee7738d5327b10cbb6ba30163372";
	cipherText = "c410953aaa123407bb9cec43899d1afb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 46;
	dataLen = 128;
	combinedKey = "6cb9b7c3b70f882bf3d52b4438fedf6ea93470f4f333a0a9d2278de371c28470";
	iv = "9fa157f1c4dc9ed96b4003baf9bca591";
	plainText = "2a3728d273291c2ea36eff4e0284e304";
	cipherText = "c88c50df49d10fe6fa9fd2e06bd25683";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 47;
	dataLen = 128;
	combinedKey = "3f90966532aa543c112edb9fe44f84e54bfdc3f1b8c706e499e8838f3dbbffc7";
	iv = "d8184540b9f12055cec7eaf9f3654292";
	plainText = "f31df15bfa89416a45cb3a75dc00cbbb";
	cipherText = "a252bb4a094b1b9ce6078b1302c2fe46";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 48;
	dataLen = 128;
	combinedKey = "0e54eb8e1a4c86689f9251d5305cdf032ffea11f817c7b6ceba4420e5a405346";
	iv = "f247a2e183647843b38d6a2bfef5261a";
	plainText = "67a7407531c6c97f5789e6c8f5fba76f";
	cipherText = "cb0479e1af394846ed87265b2973ec62";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 49;
	dataLen = 128;
	combinedKey = "e391abf5c55268ec220e04afc56176abd8be2cbbc3e59781aada92559d1f75cc";
	iv = "ee5f726f0ffbdf1b74032c2edc610bfd";
	plainText = "99cba520cf1b6fef60cfc2ea5b925214";
	cipherText = "81510ac069d947a0ff083cbb8a710d21";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 50;
	dataLen = 128;
	combinedKey = "0bc73b1d487f0fa300c9fa391b9465ad3ac774de11d1c28f3caf05d612526f9d";
	iv = "d9d5248904c97fba325791d31c283236";
	plainText = "0331ef217d21e7d1a1225ead8c24e532";
	cipherText = "dac01da3abaf93d84cd488623d2f30db";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 51;
	dataLen = 128;
	combinedKey = "5eb1e3ed21073b30c4a86b800f5016063b4323e45e3960ea8f02e925a0e5028a";
	iv = "35134aa893fade41cce5166b187ecdae";
	plainText = "025f1b8d80affad53bd24f1b8549a1d4";
	cipherText = "d778d225b5b7f279af04505a603f6825";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 52;
	dataLen = 128;
	combinedKey = "f70c0d0607942bd86c9e0a10fec0764b810610b3db0480b2603d210f827a6c17";
	iv = "628e0b549af3631772ed9d6fb2e583a5";
	plainText = "bb20b8c6079b4031b0ca8167ebcc0bfe";
	cipherText = "71985d1264332ebac96d78acc22da238";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 53;
	dataLen = 128;
	combinedKey = "95907c8ab5ffcacd6edc0ed1653d0952f32712345e8d90e70e0a578e4fabe406";
	iv = "de3fceb65a26f8b7f733687e1449b33f";
	plainText = "51921ed1a94dc00848b1720b2c3b0a2e";
	cipherText = "7176d3cc65da80ddd31ee5a7e04e6da5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 54;
	dataLen = 128;
	combinedKey = "bde5cb87cc9d3f4f82080e24e7772505f8d120cc30914e10d8ca26ab351107ac";
	iv = "13dc996addc206f37faa1582f23cd988";
	plainText = "1096b53b6ee922d2118c3687041e11b5";
	cipherText = "0be8b71493abe873cbd4b0418fd54a07";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 55;
	dataLen = 128;
	combinedKey = "9574df5e0f2d41575692cc17fac224fe248536e14e3a77c96a0ca562ced70357";
	iv = "fa0dec91a79d9727795103e171c0945a";
	plainText = "c716d57bc533cd638083e0d000dd10c6";
	cipherText = "6fc24608851e700f555d76063ff3c532";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 56;
	dataLen = 128;
	combinedKey = "039e8bb0db225f832d14579ea186862ff141a91fcd791bfd238d3e74fc26e1dd";
	iv = "2b0468421fae752d6f092e48e4dfe616";
	plainText = "9fe68f428392a0a2b3e9324036a6c74f";
	cipherText = "6243f127847ce42adef3c7a97894971b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 57;
	dataLen = 128;
	combinedKey = "f01c0794bf1578bcdeeec7ce3343174f47d0cdafdff8fde019ea39c2d7836fff";
	iv = "ee2ca121a9e02418938774e6ad7f1d79";
	plainText = "a46eb1a762fb4ea048a15047f1e333f3";
	cipherText = "a975a6ac7de0a128db676dd973106448";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 58;
	dataLen = 128;
	combinedKey = "67207b23513be52bb908118bd9862b4e52bbafb601c24c342ffe018ec9cbe272";
	iv = "233c205982f614f2ee64bb937ad85be5";
	plainText = "99dbdf6ff5fc4450cef2f6fab860ba9c";
	cipherText = "47fca4290504c3df315d27ad95c7c7e4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 59;
	dataLen = 128;
	combinedKey = "aa30edb92a8d7e5b63bd7be9215fe12f426f2374d62369972acba999c60ad354";
	iv = "38aa14a2a214bd6c672b16638787f1ba";
	plainText = "4a0bc1a2cfc4757a37781614fd508aa2";
	cipherText = "6d542689ebeaf63188b247238a4f227d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 60;
	dataLen = 128;
	combinedKey = "4172acf17cd3a35e25f8f63f81925d2a280144306ae6a31ba3122f7a259b73ba";
	iv = "88ad8b3fa62c0859bbccbcbba00c6a7e";
	plainText = "110677671b17aa047be06aa8ecc3e4d1";
	cipherText = "ca0db6fbec082af5932ba15455d519cd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 61;
	dataLen = 128;
	combinedKey = "2151f570c4abaafb47c5e8202ae3b2d79e507f52f4b6d02f31a9e629377bdd43";
	iv = "155b4ab7c2c8834bb633a9bb5f5b2d17";
	plainText = "b29b5bce0684ae4118679363d3a9f7be";
	cipherText = "477c30645d8e09a1fa42be96c8315fbb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 62;
	dataLen = 128;
	combinedKey = "0d7e885a32ca4fdd91e175ca993330dc3d7ad2f2ad39896406bef6cb20c6b725";
	iv = "b7cfedeb9f34f2d21d291cff9d2d3fd3";
	plainText = "49bc8db5279a47ff8231479840eab25c";
	cipherText = "f852ebf7396e6a1bc29ec6fe62f0e88c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 63;
	dataLen = 128;
	combinedKey = "7aeabad799722b259b95757d3cfc6b5ca5bebef59e8892e9cbbd783803682554";
	iv = "834d045ce3c9fb95f5ef351c7b8dad02";
	plainText = "b855dcd03cc80de45b39031163911cd0";
	cipherText = "f7aa96945867bedd459c9702ec417410";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 64;
	dataLen = 128;
	combinedKey = "cff5c8a1cebee0e46c2f4151cf8830d0b57bafd715e2064beb4d6219a1f94276";
	iv = "491e35cd472072be033210ce2e1e46dd";
	plainText = "bf6a53fdfa4e325299bf1f0526617cd5";
	cipherText = "70737ac9d0739e07fe6d02700f956a8d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 65;
	dataLen = 128;
	combinedKey = "bc029f3cf23d7fc964a255eaecffbcb320e8942ea28ff4ec1a47f9e91cd0087f";
	iv = "6a8dfa2418fd8acb74fc7d0b38b736be";
	plainText = "51a835dda82e4b017056e44c928da4b1";
	cipherText = "22b823847488545d93177aa37b5d9b9a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 66;
	dataLen = 128;
	combinedKey = "a1c0787bf4de26b10053bf1c4615c1a602883d6566a7184baa11b2d39859b1d0";
	iv = "a0875430151f7d0a065966a3909e5339";
	plainText = "642c9617344156c4a3e259497c370382";
	cipherText = "04b86a5628cb39e39378c09d8a6c675b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 67;
	dataLen = 128;
	combinedKey = "f4c98233c56c9954bbdd23d9052e2ee5392807ff2f310eb5063a1f6da2bbfb63";
	iv = "07fdc8f424e6c0fbeba275c0cbff2661";
	plainText = "58b69bae6465fb3646bd0a561792fd37";
	cipherText = "05182e495d2c6efc8385d4c4a8e0b34a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 68;
	dataLen = 128;
	combinedKey = "ded3bc05f96c73bc19ff9e3b7924f6aeb99a808a40f529b8871c3ed1a91528ac";
	iv = "1e1e6d1eb7aeb8073c16e9fc40a1f6b3";
	plainText = "85cc116def8a2011c0e6cd87c074240f";
	cipherText = "3f5de8a874871e18820e49e122f08a09";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 69;
	dataLen = 128;
	combinedKey = "5efe21747bf4f1c1d40b78504773211c25ee89c9144d8d92f62ab7961830ceec";
	iv = "f15f589d6b539ae3b07fe17b65ebe8b0";
	plainText = "f688af4b8f7ba44e4598c85577784ba0";
	cipherText = "f8b9ee876bfa3b8a76b9e177e0f3c350";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 70;
	dataLen = 128;
	combinedKey = "459d9ca7a00b4a991c594b9d8811485338e3c778155f682292416aca9dfdde76";
	iv = "985a0997419d71c5fb44cb9cd919d9e9";
	plainText = "6d6640d6aef06a17b2f5234e968e1300";
	cipherText = "4002ea43561885a63ca623bfbb92b4d1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 71;
	dataLen = 128;
	combinedKey = "3b420ca90eb3164c7bae206c7c0b998dadd137ccfd8dae1e3e1982e157ffe54a";
	iv = "475fb55a3e2aedb247fac73a2911d5aa";
	plainText = "4ccbf906500a90ed0ca37d36dcdb94b9";
	cipherText = "2006e048d21ff72e207e99a1669f7e54";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 72;
	dataLen = 128;
	combinedKey = "49019a26481f3c4b96152fbee1cd6062db95585ce41f8c152412f3e80b9be04f";
	iv = "f742332f722cfd65d99eb7a41101c781";
	plainText = "064ff066318deb0456453344f8ecd2b2";
	cipherText = "eafae9829732af77fa7d15be007b9572";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 73;
	dataLen = 128;
	combinedKey = "f5565f09b164ffad67160cc6cc756fa206b628c8f217f9d88df26a50487be3e9";
	iv = "387de8be00c8c1247efa76a8de256f99";
	plainText = "2945847377fd128f974a35b88633615b";
	cipherText = "308ddfb2e666ae52ee2e335fe6c2e417";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 74;
	dataLen = 128;
	combinedKey = "892b137375e05b418e690871cb9e148346651fd17801025421a5beff77d866a6";
	iv = "af9bfc09932e51904cae248a81e3e423";
	plainText = "49d4da9feb315f53c51fcbd92a7df31c";
	cipherText = "4dc2fc3083a64a080be6bbbf75482542";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 75;
	dataLen = 128;
	combinedKey = "22e2cdb46071edebaf579ac87f76697c3039d7b24b732501f1760440ff2a945b";
	iv = "2b54feaa1089b4e211fc2b59ac504606";
	plainText = "89e6133f383c4846dde4c8884798714c";
	cipherText = "d37b188b6c28e308b78a71f15769cdfd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 76;
	dataLen = 128;
	combinedKey = "6b56792baaadcbfd9278e8a35f9ce05224bc932e5c2309142a739e0781827ba8";
	iv = "ef9237736e1c80ae0c4ff8ea4a18ee57";
	plainText = "fdb0ce22a0e20a38d59158a28f82a898";
	cipherText = "477bc6120c1001598d248b79e87ad1fd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 77;
	dataLen = 128;
	combinedKey = "5bde251c2ed13c163ee932bbb2d75f5c242d3ddb2b79dd09e2398bcad56e6148";
	iv = "e5b2793f1abe4eb4453ca9943132d0cd";
	plainText = "e064397bfd890f40c668cb70f8766eb6";
	cipherText = "bf4482c770d049a6a47e9c5fc4a83ab9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 78;
	dataLen = 128;
	combinedKey = "ea92fa8a37554b4236c0e26ebc58836715253b8370b270eccaf54f8ea1367ab1";
	iv = "fbfa557bbf11ab6995a11bdcf75a00b7";
	plainText = "ba5a69b64b8a0046238d882dfddd8519";
	cipherText = "04142c0e8153d293ee8231c5169e8e9e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 79;
	dataLen = 128;
	combinedKey = "eba364e12d39c43d66ae2044cd00aace61cfeb51729613ac655051b8d23d601e";
	iv = "c212065e39efc646a25f6f7c04713017";
	plainText = "09be2e4df15463335fa23da38bb15ee1";
	cipherText = "07a60189e3840c0e2e637950d31c427a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 80;
	dataLen = 128;
	combinedKey = "9ab67592094b580f4c744d5318a1abe84188a13c0e9a76735c7d04edb878110c";
	iv = "930e93300d03ba45c228707d86f6f132";
	plainText = "0d1d2d79804b5ced3f5ff416561cbd77";
	cipherText = "8c45a7067282f3f6e1cdd9a2d4ec4d1f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 81;
	dataLen = 128;
	combinedKey = "dd73892731abbb61a53e2a10b501ccd68fac3c847b70add43fc341e296bd1d33";
	iv = "118e9286d54eafeadd36c82fd1f0471d";
	plainText = "4b3c7a217cdabf7e09ceaa3dc7abde38";
	cipherText = "ec01c43a3f4fa9ba7609a47e3c13a9f2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 82;
	dataLen = 128;
	combinedKey = "8b54dc39148094a75c409545ed7561bc32e33590582fbcabd770d2cbac780af3";
	iv = "1bb8035e783930d4ee5f987444f32535";
	plainText = "044f7efffdf536b2981b317e72661317";
	cipherText = "cee7a05f00b273526fa71a87aa63980f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 83;
	dataLen = 128;
	combinedKey = "a1c32733e4099c2e523ec542b90fe47fb834b0c21d10d7f1575e5a7f9be63c03";
	iv = "574fbb893e52913d2187f7e9b36bb1b2";
	plainText = "6d11ba6c40fde736081a5c4f635f1004";
	cipherText = "5a1ba8a0c296fde62780bd3ab66889e2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 84;
	dataLen = 128;
	combinedKey = "948912a5453dc0641158039dd159d4e57058a7e4e3007d45b8ae0af8cbd80c82";
	iv = "6dd2af9c5727d65b745d29eb05827961";
	plainText = "bb0378abf90cde34f0f630db175fe345";
	cipherText = "cda72c110d1e11fba06c93f7a75cd6ac";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 85;
	dataLen = 128;
	combinedKey = "b59042a135e9fac74daa76d9a6cd2b752f47c3b79589023c537a522d187f674d";
	iv = "f49d7bd75460049350654e3ee28e85d6";
	plainText = "409173735af74af9a74ddbb6e00e72bd";
	cipherText = "59aa8122cc80d6e8b8188c5fff138c34";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 86;
	dataLen = 128;
	combinedKey = "7b417a0151266f1e76bc41238bf20647001556b9cfa1af9b7a6a4666e8f16395";
	iv = "1e62967ce97b5038437cd2054d9b7a53";
	plainText = "d5eb1acf5c4594809f83afe4b28dffb2";
	cipherText = "a3654e33ad5afe939e514502438ff26c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 87;
	dataLen = 128;
	combinedKey = "c6e1067dbb927b95b5a9810819e3379a7de05b13cbb403ebfdae8053c0256158";
	iv = "c7e42b26d7acd4d3797f423d305d8d81";
	plainText = "e47c0a19194be29220a1da312cc12607";
	cipherText = "b2b05f88075ab53903758193ff49ea55";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 88;
	dataLen = 128;
	combinedKey = "ce24a5004274e90a9b6501f9539615c287fa238b9019f98cd4c95af652ac0c2c";
	iv = "68c8cd9a82ca38a789baf9b53ff5c6c8";
	plainText = "4efa06f07b9acc0ee4ac524bdcb7495b";
	cipherText = "805d16c4010890fda3dcf0f511c36716";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 89;
	dataLen = 128;
	combinedKey = "2c66060d58339180ad03a9b45f8625994c68c916b639c45034776356119a18a9";
	iv = "53ca5b86394762089083adc9386fa0e7";
	plainText = "4b012fed344f187ea9562cfb6c4355cf";
	cipherText = "d612fda0371f4c2affca867b33c8abe3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 90;
	dataLen = 128;
	combinedKey = "da9539b1add00192323e104ae59a17a38ddd45a3e6e19b7c3f7ae2cc4927f8b7";
	iv = "cb50cc089c09d518d56f9282aa61b217";
	plainText = "22f755f2b50128cecea8108dc07573df";
	cipherText = "18e6c76d12abb7ef1fd7653a32d150a6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 91;
	dataLen = 128;
	combinedKey = "2d2036ad520a30fc8b656be3a339f0fffb2934b64afe1aa2ea91972096a93ab6";
	iv = "d2c1012df83b7ffd9b8515a0fc43e6d3";
	plainText = "9d6ecbd13728aa28f9987b14843ca5ff";
	cipherText = "d5669cb7b36f321ddf65ca667930b7c6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 92;
	dataLen = 128;
	combinedKey = "5b2d617a9c3634f7baa0c173e9cf77a5ee98c22709f7521d75db48fe608ae85d";
	iv = "b6220b0eef27fb45c5348242dd2eca6f";
	plainText = "157e07c77b7302428938889e7cfe054a";
	cipherText = "f6d0f0987b21970d12002e918fb8a137";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 93;
	dataLen = 128;
	combinedKey = "1cf728ffb420f5895afba534687a613bf7b525a1355438e8283a2e5f22209c9f";
	iv = "59b7cf637d51e1fce46bf04c4814f927";
	plainText = "98a67247d79b75274c0f6b26e893e158";
	cipherText = "07f0bcdc2ba2b6eb2db9ff4cff869cd1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 94;
	dataLen = 128;
	combinedKey = "6a1528a110baf8134c15e46dad1b206455d50b56861ea0b106a7ad84e9c4b607";
	iv = "c36c11d6c1786d4a9a066af6b8218699";
	plainText = "d14e1778109fe3b5cf83d5dd159c45c8";
	cipherText = "157628c394d91927de372bbc037fb7f1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 95;
	dataLen = 128;
	combinedKey = "71dfd80de7a706d60f76224cb813cc8c13170cf0c9fa0ccb9db77bafd8311776";
	iv = "0c9693bcf4d4346c430f021afb25de9e";
	plainText = "56dc37b6473882df2e6b89934f67463b";
	cipherText = "c0d60f4c7ebe08d1a87238df7497b295";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 96;
	dataLen = 128;
	combinedKey = "bb41248db8388b5c809728838d45d6b5ca536ef43860976345b0b1d64d4e1b46";
	iv = "0d7d53c6c3d46cad187f71b46f008a27";
	plainText = "606c97a5d698caabb76b3ff63ea82873";
	cipherText = "85330dabf5162ad8fdb01fe24c0fbeac";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 97;
	dataLen = 128;
	combinedKey = "a5654ff447d985078c267806ebc611d3fef0a119693e30d754a6835d57f1ec04";
	iv = "1715d4ece0455826b09ddaa745ca831e";
	plainText = "527b08c654cacb38106f2639aca980a3";
	cipherText = "eb425eccd25f12358a140aff19d74d46";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 98;
	dataLen = 128;
	combinedKey = "0a54f704753393e93d20c7a35322a5bde43daf418af1421c6d7a5ce94d18a9df";
	iv = "45984e9b2df2b80aa2826b1b751d4a4c";
	plainText = "8a8ae807266dda61304ac4ddcaf5dcbe";
	cipherText = "3ff1c17cb6461454c0f3f34b6bd157f9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 99;
	dataLen = 128;
	combinedKey = "f933a0b1742194c3c264084a763d2b7fc9cfa5f8d3954d20a1c62179b2748695";
	iv = "649429d48bedbef7de785de35ec68cb9";
	plainText = "a48be035a8421cac2c99d1eadc13975c";
	cipherText = "fcdd77570d7856e470bf60a81fd10732";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 100;
	dataLen = 128;
	combinedKey = "ec9abff0ebf84e10af5fe85e5783410db6e434dc4e56d2bef6716a1fb60def9e";
	iv = "340f40818f10b2b27121d4f42842bcea";
	plainText = "fa3208895cbe866b124b301a4b09751a";
	cipherText = "1efdb4b8a973ade739b73877697eb138";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 101;
	dataLen = 256;
	combinedKey = "c17c372b5626916d98c02629f5d276474557e3feaa75783634d55f154e0a8d18";
	iv = "dc052215ba655cce707e513d543a5085";
	plainText = "e9b9e29fdfa1af535a1ec50ed9204f75c62f40a9a19e5049cc1048e7047b8acb";
	cipherText = "b7b416a56cc32f753c259b943ea07a9dc030840c8055831e2231f7bfbd0a734b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 102;
	dataLen = 256;
	combinedKey = "8712dc338dc23f7db7e24a5615bcf7763906175650e8f7b958185b8aa8724c2b";
	iv = "0525e21ec21c95f840f99017c72f276e";
	plainText = "65046acef34bdaf401fb7427103ed25bac7cd09898bf9f6c07c8b99b51d6f3c1";
	cipherText = "ddb95bc391d4820d7655126cf11db4681bd2f1e1e42f20b479aae1fb3a8e3e39";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 103;
	dataLen = 256;
	combinedKey = "25f7bba87c5ea6d1907e16aeeab834422b447493f9fc4198e05eaa01c71a2ec1";
	iv = "86c0fd2de3dad674a741e20cdec7d7b2";
	plainText = "4e95911733d5b4e43e5efd875771b0bd574e7db6e19aed31622c08c7f1472bf5";
	cipherText = "0caa97270bb41f092cf649a70833a6424dd50fcf2e9f55f02e91976dcd2c2ff9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 104;
	dataLen = 256;
	combinedKey = "a6fd2e5ca44fba1cbbe368f3bf5c45857b7d13ef7998fd915a36b471efb3b73b";
	iv = "08a420e18abd36a969ae33ac59c3cb8f";
	plainText = "642b97b2ee7f12050461742c07d1154bceeb15c7cbe1228fe21a64b6ab5fe796";
	cipherText = "b427a817ee891f9e40a4ddcf8d78f9289c7ccfa244e8c5d3a72c8245e76646bb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 105;
	dataLen = 256;
	combinedKey = "ba4108734b283393922952d5e32ce539fbe675d94ab54f389559ce05c36a0f2c";
	iv = "5c36625dda1b2528db7bf7aeef95185d";
	plainText = "71c70aafe1b7e355aca682e322c0a1dca4411c0922029f895ccaa6bca9fc2583";
	cipherText = "5e3c5fdfc6714deb37ad1642e94ee96b4ccdc501626dd50d1369a3ca2898b84c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 106;
	dataLen = 256;
	combinedKey = "de347dc30cf773853def15df0a45fbdc657f0a433497d63382e0d409dbe88b91";
	iv = "a786b4d4f9139d56f6bc422658d2ef29";
	plainText = "214b3f186a58001800f74b38845665d5e8c15cba2516f4f4f615cfd59f3102c7";
	cipherText = "035f6e4cb86f7fa9fe8d28fc3c0943c5875c1b0ce69eb2b7933345080d0d95d5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 107;
	dataLen = 256;
	combinedKey = "7abf4b5cb9534b99e954f1c0fc52edab6a105664c54d59f5b12d7f9c01c63c50";
	iv = "40f4d9b5bb05b7409ee787dd299f1fb0";
	plainText = "c62c0f01fa8fce3a6b456743f655905995d3cef9f08dd94af51e6e76c9599efe";
	cipherText = "26df5ad6ec95a5f598f558ab179ab30979d5a8f66d59d114769a667fb8513b9e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 108;
	dataLen = 256;
	combinedKey = "dca0b4e2b30ddb8cc6a3c8fe5f32f32303d36aa48cce71f3dfdce8f1e40caf14";
	iv = "253aa216299fcd983e4f3ed90bcb8e01";
	plainText = "b0032714ee64789b7b4c23f0a5a19179f1ec5dc7a81a07c1d5e69a9ff9347187";
	cipherText = "3170763465743ce55eb4da71b27bb503cb63a8e668d5cfbaa48456650ad97187";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 109;
	dataLen = 256;
	combinedKey = "73e5b3d89a90df344f21c8d8c91b196e866c0368b57a843c04e95639c7ce2288";
	iv = "e8365f08da13e301d02a4cba18448124";
	plainText = "e83364b77fe97045f279e69a35781f28d62d5d0eb7ed51abf07d3525e401e73a";
	cipherText = "3c0e4ccdd867ef191ae59c6ea8584e65aa5f15e86f3ee06304c0c0e63e2b7643";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 110;
	dataLen = 256;
	combinedKey = "419f46458ba9ca86635003ca1406ccbb8fa8fff30b65ed595ea2fe4cd0c1dfad";
	iv = "809dbc7702f49db627be507e65be5fdc";
	plainText = "121540d511ed015770f02093e255ba0be6c1ea8ad0bdfe39414a11a5d61edd16";
	cipherText = "ac1fb064974950f3a60b7af3c7639ba59bc93b26ca33f8abd5bc1210e3473656";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 111;
	dataLen = 256;
	combinedKey = "54f7657fb97329f679d00f66be9f589b739e85371f84945ad0ad1bcddaa15529";
	iv = "8c23e7f4604d85c1ad1b588378864c37";
	plainText = "70a9b84698b9125c58d6b5f1962afcb83928324762804866c607f199412d0645";
	cipherText = "cc36dd360807fc92322cc4e74890e43663657fe79325c9eeeea1c476eec8fedb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 112;
	dataLen = 256;
	combinedKey = "160efe62676d0de0b7aed7eeaf064f4761937077122e3c9ad229350f599a4040";
	iv = "f3d95f6c23ed16fa618c764bffc581be";
	plainText = "a4b9c7c5badcdac11136e72bcee8bcf54d3f7a4fd0f9e1e563ac08b694b0ed90";
	cipherText = "d534ca942d918af061f05dd0cbac5f4f702ce2d2082c3751265fb728e56914fb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 113;
	dataLen = 256;
	combinedKey = "86a5ac4605ef1f878cecf5a9cf92a1b6192f8dab384cd75ccb84262bb4bc73cc";
	iv = "7653870a4e7b3699d0e8b71d7fb3c1c1";
	plainText = "7c0365cdb477aadbfdb26ed46efd0d2df6bd5883125b3305f3542857ef135b66";
	cipherText = "d6b1a9b4efc82cbd8abe4f5c35a951cb932fd4d0ad3ff686b78ceea26fa1f7a2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 114;
	dataLen = 256;
	combinedKey = "2357c1def609ff88b50ac23dce634df70f5cf9209ac851b4e2638dfc1e4351b6";
	iv = "2479ff2e39f851bc119d78df794191c7";
	plainText = "19c7ead499b1ab2f6a6946d2bec289bddfbe0a2e39dd9cca52429c65291f3229";
	cipherText = "f9ad4a526fb19b6d2c05d6dd5aa1445d04e55dc4cf8f4eea5847af90e25b4017";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 115;
	dataLen = 256;
	combinedKey = "63a77b6dd778fca13d0deea4fb66161b5652f04d3efe4c86dea02e1c08fac73c";
	iv = "50c8307d3ec80900a239b4f68863e787";
	plainText = "b0973852b8f692e6dfd1ce30b2e4311036a6df891ab0d5b3974c2a70b28d2e90";
	cipherText = "d1b2e1ffb86590aeb2d6608a6a5799ae154f2c21ecd2213db621ed72cee9c276";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 116;
	dataLen = 256;
	combinedKey = "3c557e959f555ee0de73dce8350e079490812bae4e902217bfc9690b61e2d9fd";
	iv = "e49c169831f792b4c2826f05eaaa24bd";
	plainText = "c2c072c7231318bc44abff3b84e8937779e0140adf2252fc3b1fbbd4ae50d4b0";
	cipherText = "caa4276ca7a9bbc1dab01fd550c7094e7560e3adb367ffbb92d163cf5a6c83ec";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 117;
	dataLen = 256;
	combinedKey = "84ba78b12227e29a501b4f6abc35c9b9a7c9b9cdff98af957e487ee130d4e116";
	iv = "24df05f1ff24fbc7853ac52761a6ec8c";
	plainText = "befd5682b64501b57e822da9874c789bd117b46e127813169304d69d224d9d0c";
	cipherText = "d0663c5a96097318885a732cf266e7edae01f3bf9a560051b03a976bb19ee88d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 118;
	dataLen = 256;
	combinedKey = "f58450c79e335ec7eb1706b7497cb7a2c55c0dfabe2f1a2708a789840e6a441e";
	iv = "26bf01918d9abf182ed62c95356f9bf7";
	plainText = "b8246768361ceb689c8dcee70833a495c2b019539bb6af2bf9fbed6001c7f567";
	cipherText = "fbf1bdd97a2655aaef9cde7cda9d1c2143b5913a371d2ac44c3e339f2e3af5e2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 119;
	dataLen = 256;
	combinedKey = "7da9b23985ff82f9e0b97b9e3ccb810ba10449b4ec4021d4378ed8b254d24f42";
	iv = "2eb0019fe6a51ccb4e9dc85750919027";
	plainText = "b2abe40e07ae6ca9aa112dc4e4145f1c2a21dee8d43d2a6be305c1fe8228723e";
	cipherText = "f5d5784fbf46d84c6cc56d88bc1ef58636d8a888f5065af91810fb8f2133fbac";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 120;
	dataLen = 256;
	combinedKey = "feaad2c51acad21b490be4b6855c0822a97af8271f5a2fdd7a4d2d5a0ab25fa0";
	iv = "49d5f8f501bc6817e566396c42ee234d";
	plainText = "2b2c080fca4b071374961a61a449b98b2fce6a8c4e1291eb8132e9f4a80b845d";
	cipherText = "98ff2ddbc71eec4374e916ef56b7c3da881f07930e7cfb79bd54f7b3dcf2d689";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 121;
	dataLen = 256;
	combinedKey = "1fe9f90a37e71431f505457ba7f90bf0598be1a59378c445aff75e8e458ec657";
	iv = "362a35f22dfb33ac208209acd878c027";
	plainText = "9efd492a3ab077a12a67a405d17553d23a8ec60c9de52e425b91c8031403d4d3";
	cipherText = "2e2062f7fb2bf70c4c37ebf9fa967cff665f2bea91dc9336ca293816de996170";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 122;
	dataLen = 256;
	combinedKey = "c9a844684c7e31347765b98a01ad672c3578a7cca73fdab032f7ecd10ada0689";
	iv = "03e90d5c02b6d040a872ccefb4ab38d2";
	plainText = "cbc6d2f141866dabccb850a34fb27c821f113c0429b4b73442fce7ca7f1300af";
	cipherText = "122a05473e5bb53614e6d68596bb27ee317590542efbc39dbca2c5523eed57cd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 123;
	dataLen = 256;
	combinedKey = "f220324197652ea93d97cd63becca9d5732683ef0569acb42ff9cba56fde3d73";
	iv = "3f40d9594e8b0d182589d8a0e6fecd17";
	plainText = "eb25def15641f80845392381f5c486698ad5355883809f1a99f7cf1ad8521d71";
	cipherText = "1afd1b88e62ac39d2895aa98ea5f1fe0f3ad48ec6271aeef0e4f4f61cd03dd7d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 124;
	dataLen = 256;
	combinedKey = "186d4ff1bd586a31efc153ac46ffa0bb4eaaccbb7ab6f21b40bd89ae3419c208";
	iv = "bdb0d4a2841a59aa125cc572fe3de5e4";
	plainText = "92a42a9f4aafd9a817ae5df2587035d30361c271793cf668399f194fb972dca3";
	cipherText = "f670b6a95aa854d187402fe15b72df586d10afd78b0d529a5954522fa8b1f248";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 125;
	dataLen = 256;
	combinedKey = "9eb927d686e5380cee80e4313dfc4a6fb7b93c67787060e84459850d305d0dad";
	iv = "a0a2d7a7d863282ebafbe0712bf298c6";
	plainText = "0d49c003942a76b9c97460d69309cd5c4d529b24c931683574992f95e9000257";
	cipherText = "555efb9e49b2961059563ed59ba0a2f49682599d7f3c1bbb4f8c5cdd517773e3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 126;
	dataLen = 256;
	combinedKey = "f5c9f118a1d0a27590d34b0e520a4632ef0b8ef49ac8d670edc346ead5ae88a8";
	iv = "542f3ef5b3027e03273b54077dff2a3e";
	plainText = "8d1472eeb26e2904ea1b5dff164b1441804a58bb39e7e99d4e324ca40d2895c7";
	cipherText = "ea1127a766df06cb5af5b7a25f16caa706b067372366b9f5f6ba3a4efa23c812";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 127;
	dataLen = 256;
	combinedKey = "6bb4ad6ced2b1d50a9ad5f66e1d16e2a821e563529697055e7ae3178b6814a68";
	iv = "bd16e5dc1fb18be7c47531ed1923234b";
	plainText = "48a0861ebe8179456cee8e3a412f5b85fbad124f6cf8355f9d0f70200c740672";
	cipherText = "0a378360a0eb0ab6835d01880c23b1931d22b06b8504b6a014bcab79d963ec53";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 128;
	dataLen = 256;
	combinedKey = "150d03c17da5fe7422dac51298287a5d4b30aebaed785ba66cebc1f1ab007fb1";
	iv = "fc3ed71740ae0aa909724d501f04ae91";
	plainText = "3d3eaddb3a73db2892d91caf8dae82545fcb836c2ab9c0329ef3e9b6c527a53e";
	cipherText = "c636d13577b37a68e823b6d0e87827a47c54d735483fe0b2487b48576574d465";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 129;
	dataLen = 256;
	combinedKey = "ffee0b5a65efeb42e538aeda73037d94bf9d36b8a6a9c2b145ba142a607c760c";
	iv = "199a0ced9c9fcaee9c0b38bbd06f6674";
	plainText = "cdc3d155ff05ea58512b25d705b162d9b22fed9b691e79dd581eefd71fc40e17";
	cipherText = "8405bc24cdea6eb7948e4f29d2c34cd344dee8bf71906048776ab2413e435dfc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 130;
	dataLen = 256;
	combinedKey = "9f33c089f3bd6c50c5f2332eb0a0d023696bc770d863b033879e7191f5f9655a";
	iv = "d3d4e8e95a42183e61ecfdbdb7b7260f";
	plainText = "e5819181143e83fd0c21e0d6d8e4edb8240fa13955a41ee18bf6d7fb3eb27888";
	cipherText = "7d4bcdb6fb84c8dfc03a5c07857896fab0aa3554cd2967fc6510be2eecd520fd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 131;
	dataLen = 256;
	combinedKey = "d2bd1f7f6584fab6d93659be4c72ef2c1e855b725b1c2f6e22e56595591a8c32";
	iv = "653f53b9e016b22ffbe0f70dd3d5e0cc";
	plainText = "f95d51444a01b6505d3cca7bec5050aef5951146a1ca11c743bd853bbbed48ff";
	cipherText = "95ffb2d80c8623b500243e31b7488632b279e3bf35c5b62d687976938b19f25d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 132;
	dataLen = 256;
	combinedKey = "fd16abcb0209b43993c1d1607682b8e9a6347087f39b3591a02d3fba49b35248";
	iv = "4518764a3a3d362132a65802373c88a0";
	plainText = "643d5c7bf3d6932233863f97869de49c7edf056a0570174a275a9d23a1681953";
	cipherText = "02cb12154b49f814d698d8e2cc475d8c703682f1cbd58cf264590192a57e15e8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 133;
	dataLen = 256;
	combinedKey = "cab9925ff7a83d3081a384bff6fd701c69e2d1fe14773cd882b6156f531fb4f6";
	iv = "a25bf3170e3441c11dc09b75e931209a";
	plainText = "e89fb9ee859a907c09b6a2bb0be8763b8d5ff78b2df416353b34fe7799e47d68";
	cipherText = "accef6b1e3c81551585dd9f0caa79051cbe711f29478914cf3edb40b89a3b459";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 134;
	dataLen = 256;
	combinedKey = "cd7ae924c7b16d69beb5f2bb49e63359ac0c1e85e6629356f20e61ad587beefb";
	iv = "babdb59861255627188b2baf855220f6";
	plainText = "ca586003ca561172fe12e88d1cc87e46a818d1c4345fcec600b1899c9872e94d";
	cipherText = "56fa3f1129afe29ffacdc23b96a1d0cb684cde37d7bdc865f119003bb35b1625";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 135;
	dataLen = 256;
	combinedKey = "ea86ecb69cb95379089af2282de2c1777bd045990805fec77af59a59a60e0973";
	iv = "3a2e81ea77e49c98d9b3348ec2e8c3bf";
	plainText = "42faa6457bb4d1b765a8d69213a881972e0e6f69f6aef8121c8b3229feb9a7d0";
	cipherText = "f76a933486daf9c18b11ccc56e4f06516c74dd4d859d6101b2321b264be4d988";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 136;
	dataLen = 256;
	combinedKey = "2bc8c8c2ede375f68342b85599aebe95053f9540ffb5c6dfd546d727b85866d3";
	iv = "a9c7a140749916169ab7d03ad69b09db";
	plainText = "2b880c706181778df9dbb149b5cc7332fce0c6f2396a62bea9f63d436b676f5d";
	cipherText = "e735c33c64c46833ae9004a3242ea8a4483a23446c8a30039bf1ce9e9aac648c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 137;
	dataLen = 256;
	combinedKey = "d2dcbbbd6dd4837fdf67cec26a2dac9a34f8f3155d912084913233fee995e0e6";
	iv = "b724fa1b5544987ee02507334eb4b403";
	plainText = "bbb881aa6cde8fffbafc60ebbb92f1408f62efa890a34165302d955a50d28a8f";
	cipherText = "543556475807367e29ecc9df9a3cf3e5c2bdcac2fb3a1a34880b428a77ef058e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 138;
	dataLen = 256;
	combinedKey = "e8890d22487499a7e9484aa5a2b907c7a74e6e4dd8acd6f8295f5fa8fb428fcf";
	iv = "3d39a8273577aa6f2cc77a825f008fb0";
	plainText = "7e9eae63331a1141796302f6f8faa4a1e57cf88c451aa9d0a197c604f39d6e73";
	cipherText = "a51059302f51dba607ae3a6c147d117026642a2141fdb7ec32063d66c7cb732f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 139;
	dataLen = 256;
	combinedKey = "8328bcee227cbebb333439fcb2f6a6983c1036d32acb876d5c497d0672a79f93";
	iv = "c156adc18410efed7e40e1f71d1c346a";
	plainText = "c5569bc57aa6335ef418a88f431e207df1344b7871519628dcffb3dd4014eed1";
	cipherText = "8a9759b7b861646cdc69d71899ce1b7945974ed5cf30fd7fa6522d0365b0a456";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 140;
	dataLen = 256;
	combinedKey = "b250c7ebb90ffaf586237b4ee2fb7a50a55be3b36240717d350677a8bec184ff";
	iv = "8aa8d620f4c7d912c1fcf8c9c4d97120";
	plainText = "436e79a7aa1ef3ea7711b50bce0a4b42d74262ede83285e2440105b6273555fb";
	cipherText = "1249c501c7c9d8738b44ea886aa937b51166d2f1d78b2bdeeb89a6f57c92ad84";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 141;
	dataLen = 256;
	combinedKey = "108d6aa5663f607ff23baa0a2df009c8ba66a961609e1ddb55ecd3e7c004843a";
	iv = "9e1bc5989069195701e463573fdb449d";
	plainText = "fe74b0bae73e63e14380acadad688813782b4e58c0a66dc0f5d4a0afa6ffead5";
	cipherText = "2a1278fd5b629a99e490ede424d60638a342a422c3724297e67e7655fa30ab33";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 142;
	dataLen = 256;
	combinedKey = "68e87481fbc1a6ad5212b5a90623a19259e815bc480c965218940935e719c2a8";
	iv = "c5a0c814d862dca858a8fde9349082b1";
	plainText = "0e51977754a6e12543987cd5c198f2b2d2fa557489554cf21d2d6475b368ad89";
	cipherText = "b5168def622240d072635fcdac76f53792e0911659f9164f462aff6393bc3230";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 143;
	dataLen = 256;
	combinedKey = "00aaaf503ff48e0b5e243c04e2964318e025515a3af61d17389924ecd7f5b78a";
	iv = "574c5546dfdbb250c65d7a1393535e38";
	plainText = "f0bfb5a2ee5bc76fa824a995cf21474cda83f7772b745029aa5db45efa2a5702";
	cipherText = "7b799a04b0b7523e6bf1780c92b0e00165c95db33cb1d6dae31b95fd1d06852e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 144;
	dataLen = 256;
	combinedKey = "fe67a19db571d11b510bcc68582f920c900bbbf1179f49fd165b8d43d22fd76b";
	iv = "fe1c65e0f7dcc447efc0b73f3eccb387";
	plainText = "e6a5ee6d2d7dac1d49e6dc729707d8629233ddd624a6e0f4e6f2f94da5b4a22e";
	cipherText = "2a084f60b0984284887fbceecb34dc06522501d49132b8c09953d8f1e7f92526";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 145;
	dataLen = 256;
	combinedKey = "1fbe68d25691e994174402172568270cd19bb73ae569f58bf416db2583cf3dfc";
	iv = "a24fa3f6ae4eff66987c3440fa970411";
	plainText = "b54e1a6cf7a25e1b28c83bfedda0bdb3dc2016c6934d9a46fb761552c2983e14";
	cipherText = "412f8d6f00e06749fe997839e355170fc226a30859035a2f5c8534c3eb75a305";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 146;
	dataLen = 256;
	combinedKey = "b3a64b061daf37b460aaff669ecfeaf9e1184e75979e3625a39a8f4efd224060";
	iv = "3508616ae6beecd711bef5bf36272b6a";
	plainText = "de2a2664734c2951bb379927bf1fbd59611389f062459aef1dc1785622bd3c7d";
	cipherText = "b1d4fa46e5ca6e1cff727defb393c0b6705ab20103dc5389e1e16510e7825043";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 147;
	dataLen = 256;
	combinedKey = "08b61fd53e44012c3e79439c5d70510dc397779e6eda447f4726bde244e54fc3";
	iv = "74d64e519f70f6a07cf78eb5bee0daf9";
	plainText = "54d0d71ee3150949a4a9e7ed5a2511b962284a72b19a5bcbfa2b374739679f74";
	cipherText = "93e047ca46b1c895be3d40b4e1a8ae7e2445b7e4b5b73096e8348d83de41a9f9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 148;
	dataLen = 256;
	combinedKey = "b0793620d1e0e70e09761ab823375f8254f6d9aa19c1b0e9b0e93537e4e9d273";
	iv = "171888926b3234a78f85a18699a78d66";
	plainText = "4967ed2f9b45c88c0fc4dbe62c5e60d3b869f442feff68235115eb721dc8f97b";
	cipherText = "d11da60d048b85951aa2c2a1904b1466b65b42d0a884af1babd52d819bde90ba";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 149;
	dataLen = 256;
	combinedKey = "914f0fa83fbf0672e2e8750eff6b63e76d995b92b8381cc0467583cf1383b753";
	iv = "c09411a193cda4fbc6b41cf311fbbfe3";
	plainText = "06c6d47114af28ff9552c25eeac02e063b073f1c805f0aacd7101e45a3e54335";
	cipherText = "d8a63140cb2193fda13c6e562dd478e53b947b5fe0ca986258f9545645ce7484";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 150;
	dataLen = 256;
	combinedKey = "79d81e8f16e131591c4478b2359629863d05e4cb8a8cc448029fc3dcf359136d";
	iv = "1c4d0b9bc104259141f8eba79b02c0e3";
	plainText = "a6abb3d0ad235cf19e2abb3f08419e46fc7f085df88d920c5ef6c2339f35d5a1";
	cipherText = "e27a9b1080c250b1ccfc29f156842851fbbbdfc1b0304df6f9113bafbfd4eaf0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 151;
	dataLen = 256;
	combinedKey = "db676f35d34d6cf87926320c9a5d176f5bfbe43ca3c086dd84d4523fc3dd286a";
	iv = "b76091a033157e2a8a6bdb0a41a3ceaa";
	plainText = "015ccf8269e7b550683a1d464a4af8de8d247dba5b522399a098873bca306f4e";
	cipherText = "921fc41eda0390ab6fd000dfd17a4e435dcfc3edee865ad6bc63e14e9be08cc2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 152;
	dataLen = 256;
	combinedKey = "c0ece3ca6208cdad6e80960f688bc6d44f667e63d2d2334809f5a56097108a70";
	iv = "5c02ea040a42ace63506ccf11bfc416a";
	plainText = "db6faf2b5c5b3405ac29c81f5f9a4226b78a47f6c5d88a84aca5f949464649a2";
	cipherText = "0e698e0b904eac5043067621c1c2e4d4d70e112ebe4bb0d36d9c7f98f5b11bbb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 153;
	dataLen = 256;
	combinedKey = "ec1ae252e67461610fdbc89430d5b86cdc742b137c98b522b748c9dd7b19d4c2";
	iv = "1760c4f448e9fd9f1a1ca87066e85357";
	plainText = "9878f297ca65cc5bdb75ce1aaa9cdcc606851be4c7102a2b1be93330610701be";
	cipherText = "16df0807260558ec9ead9546740b4a26fd18db929fac98bc626705e0353e575b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 154;
	dataLen = 256;
	combinedKey = "d490bcaa38f5dbfba1bda07766c89360a073ab286b08e1eb2313d20ae2c8bfd6";
	iv = "0781515a38152d9bd6ccc767fd8790c6";
	plainText = "033ebfd9307cb933d7f31bba686d12131aee098bbafc78e34432818945316e43";
	cipherText = "731c4518dad667ccfdaf9ca43687b0e7b229701bd424e13744b98c092edda272";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 155;
	dataLen = 256;
	combinedKey = "7c6a18620a938f9a7db57daf12228e3726b627885b64acc66663feac88723bc5";
	iv = "b711b097148cf15399798a797b32e40b";
	plainText = "1db63594d07288752e02e80c96a00a27b2622aefcf892d620d6af143842b685d";
	cipherText = "fd6fbeecb81fbe6a01a383bfb93821978496dc8e6e7cc849a5f303eaf991f44e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 156;
	dataLen = 256;
	combinedKey = "b3c5019d4a999f9867dc273e774004e52aa6e8f8ef30f111a8fa9f33caf911d1";
	iv = "1389912f693a91dd75c6abc8e3178f38";
	plainText = "d2a45db4fe4977f9eeffeab5c298672db54340acc7fd330a603a5d50ba094048";
	cipherText = "60a2b46f265a130bbe945c311487947be98f95dd94d07463f856e22e2f00ae10";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 157;
	dataLen = 256;
	combinedKey = "42ab6421bb941ae050265e2cba5428d2b8d809a4e8955b19e8f9bb6c2a506ab8";
	iv = "47b2af6be281dba549b5edda6013c33e";
	plainText = "a7b464c1a1d3e02ce6418e1aba07afea9e3b1875960425ade82c3a7d8e28ca20";
	cipherText = "a52dde23a7ef21c5b71baa35db4c866de399b2413c256ad27897ee3fb1fc3fc2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 158;
	dataLen = 256;
	combinedKey = "bb8c1ce52a26eff7891b236f210983804445895a662232737667d5b9c1a1a756";
	iv = "a1f9b914f51c2fc518fd4e1dc0cd9e20";
	plainText = "a9d85a3cab881046ec08dad7b67a35bd0574eaafaf644057ac1bfee853c787b5";
	cipherText = "ff834a29b9327682d47740d48790c2d6a87ef9d5bdb23746539da3f112ccf4a8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 159;
	dataLen = 256;
	combinedKey = "f355921856d81f1233201ad82d008adb3333443eb874ea25e3bb6a560daadfc4";
	iv = "4480165e94acc6b505aea96a248db153";
	plainText = "2c647fe94baff35d9a959b2dbae93bf96e03ca9a110990f39a6f2391cb279d13";
	cipherText = "3293052aab4957e2ba0da725643ad7836ffbf180eec23c6cb96dc383374a48fc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 160;
	dataLen = 256;
	combinedKey = "1a4cee08ecae3bbe396cb117973ac97af26f28c829a1f1fb6014bf4487b76748";
	iv = "a82332d44b621faa723c25c04ff78b18";
	plainText = "5b5d453697b9d6d300c6424390eeaf0dab87a1750a9117c88c22605ff1bd0aa7";
	cipherText = "2290a3a6d9fcf3c53f4eb8bdf5d4bc2871ea85a4feab8a5e619506a66c252196";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 161;
	dataLen = 256;
	combinedKey = "d90cfd2eba0a28ffea8ef2c3b4724c276b5e7eb326d35e5dc1f01a6410826760";
	iv = "d216fe8a649d360ee6f41e0a84f9b412";
	plainText = "2fa6e6aa438d1d450bd5ba279a886284fe048fc63b53815b29a8c5e9b830d298";
	cipherText = "205d6048869dea91a1c799fbcc907131a39f28592bbfdb35ae9bc1839ab7d671";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 162;
	dataLen = 256;
	combinedKey = "32c0a67f54d6dd70d89c0150455ab94aff12f2ea92becefd8125cbec0c93d78f";
	iv = "a39cf92f616655971829bb8a8e3b9a50";
	plainText = "9a22c80cdf832a4b4d74b1ece1858a3bb6f1269f7c4975b0d985fee5b6e3165d";
	cipherText = "a3ae25527c1bfe1951273c088140c40a2b8251e2fe6e9f7d2d0e89888643f254";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 163;
	dataLen = 256;
	combinedKey = "2b76066d453f38a0cc44ede9e0ec0dbb9144deed683324c6c8f28208febbb41c";
	iv = "3de8fe112175b3cea80eb0eb6658e81a";
	plainText = "ae33b068bf0bfb9ba4be59fd8dc086514d03296f338c34946921d623a07f86c0";
	cipherText = "e8684a34400f07358b93e34bad4e53e7fb109d12d039d0a1deca87ee352c6a74";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 164;
	dataLen = 256;
	combinedKey = "1af055188f7fd5da6a6c7ee99ef97a2295fdad2ea65baa2c5548117a0dd75984";
	iv = "d4c35df7239cad770fdc5bcd3af16aae";
	plainText = "b9bfe972829a5245b94111f0af264541edc6d18862f1a5cb5c19db19cd4e5832";
	cipherText = "8c7800b4f9afafce059273e420ab9295801249b0d1e0534eaf2cd4ccde982043";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 165;
	dataLen = 256;
	combinedKey = "f580f9de21574e590afed986d799bdc0f2be0e1bccc8dc63094831efbd580aa1";
	iv = "a66ddfee9ca403ff4c1ca6e45f5e1935";
	plainText = "f9ba39feda0770932737ed3c852d42424faef25e950d0308683e178c97beac95";
	cipherText = "b968ee9951a40ba83a3a9186a13043ecb9c811aad39af3192c53476cd0bfb597";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 166;
	dataLen = 256;
	combinedKey = "372e74a3faae8e542ae35b72af0c3d5aab74c00d271e0841ea9f99ebf61de426";
	iv = "fe9cadde37eae0c0fcd22dfd4702eb64";
	plainText = "db3426357ba84cde7773deb9824ab8c946a82b35f10ecf524c5e131c70a12c6e";
	cipherText = "3c52364ae04cef376f8a45a60ed7a257356c1fde848fd9966dd04f49a5a45b41";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 167;
	dataLen = 256;
	combinedKey = "328c716fb35f2ecd9b06f508d748bf6eaaa760e1b15338dd39803f8608865e8e";
	iv = "ec72bccdec634e52fe577eb01f94b8c4";
	plainText = "53afb290b590eb021f13983f8b2fdc81c860b00b7c49a66a63ec2e80b7857e7a";
	cipherText = "c57805f331ad8e258a6802cf4452564b22775f233fc3986b51dad90f088f90ce";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 168;
	dataLen = 256;
	combinedKey = "6cbb2b84ddae119bc3d7348616f5c4e6e6761573eca2b50c0dd0cc38f83552d1";
	iv = "7c39e0aa1571028239acf8a146a6171f";
	plainText = "f43b8b274e8c663e1dc67d3e2fb1130687f06cda32acf79654f63afd689659d2";
	cipherText = "15707ebaba24b5edc88b196e64a811fe26daaa98addd63ea95d94dde31251918";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 169;
	dataLen = 256;
	combinedKey = "0697a78949959e880bb0642dc9d20b1b4e111d9d0c5137e3b02509fe2dadee98";
	iv = "b5c9c886255c939a7783718abc98d0f5";
	plainText = "f30933e383fd28d2caaccab312121427444e2897866926644f32941186f842de";
	cipherText = "dca8a029f08d4605f2ff52d4a06c67f4a79c955288892db6bcfada530173c075";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 170;
	dataLen = 256;
	combinedKey = "14fb356f5a2750fe29eb541a917b82a74133316098bb2d3d5f44f9aa08d7c982";
	iv = "669a3152e5511028ab67e22d4c3d6c0f";
	plainText = "c21977b084b3338b2025194ebaa961a36bf1be1d879b31431d338bed698cb8f9";
	cipherText = "3794283e4d14645f4f21829df1eaf8281c89417c7ac5ee91aa0d1a1c068fccc3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 171;
	dataLen = 256;
	combinedKey = "e90be15f0803cc97096fdb065cf0c6d814d5d8c809d91ac252208af5575c4638";
	iv = "1b82d8540723fdf2356b2a7aea44942b";
	plainText = "07359f3446194255b462dc4b6588b93d8fd2be2bc266d539033e80f695c99050";
	cipherText = "705f20eaff14726b157c4bea1e0da2fc7457b93ca9b71d2d6af4bc00e2fa6082";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 172;
	dataLen = 256;
	combinedKey = "68546cd53bffd10c39de9e4a5e508824fc28e6b2a8072e7f997424634693db56";
	iv = "fc8cff998ca83046f912fd0555a1b439";
	plainText = "74acd5b9ff6af0bc26c5e5ecd99c7650018617e84c6ade4c7f0811532ba165a5";
	cipherText = "fe2cd465fd16c295dd31e3286401e3e6ad67b0abc6cd9fe7c74a09954944e190";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 173;
	dataLen = 256;
	combinedKey = "3e9921d44330dad2794cc8a62a26936e0721d15e987778bf8e03bce123ecebbb";
	iv = "b172688b348de5867f9b3f516319aa2c";
	plainText = "b55370d0816bf2021b0f5a8f993ab50de06184805ea2ebe41db8f4fc8aed40e2";
	cipherText = "7095ac3d612c4e95db2ef9c4279628bd763b56c5e3197606cb4e8c62cbe400af";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 174;
	dataLen = 256;
	combinedKey = "69720864b5e6b66964f27e8cd1c5160571c7396fe096b9fdfe0a442b1f3f39b9";
	iv = "cf6c57f85d2b9ce5512f39ce89945805";
	plainText = "b1f90e2fb206cc3b16efe6ddd7610f3c831e0aabd91ac14aa93ad03afb5a1e66";
	cipherText = "9017bbc0f50dd2a0ff9abd5ef3e75fab3d8a18baad81cc4eda6fb4f71739982b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 175;
	dataLen = 256;
	combinedKey = "3d9e46f24063030aad20769cf2946ad1d5627692827dcc678f9cea729ce1c524";
	iv = "6eb2e1f09ba7cf8d8407b2be460bb23d";
	plainText = "3a18c3a87f58f16e87584f60cc8b9fc595470bbe00b90b2cbdc5455b9de93606";
	cipherText = "fbef0c4744b0d6c317d0a05d0cce8b11eb2b58daaed818be8470a6002785f0e2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 176;
	dataLen = 256;
	combinedKey = "88040dbd3b437f159ba731bec440d15a1bdaa919f8e1c91a2b92b77b42ca187d";
	iv = "5459bf420e194bf33d07c53c9bfdd26a";
	plainText = "fa437b5d765f4cfe8bbd84eb7f0c65554be6f64ea9706e6024465f83512c8fd9";
	cipherText = "7f7efc731bd8cbfee383f0e7cacca4a8d4f086b996749fd7e590cf48e5a626fb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 177;
	dataLen = 256;
	combinedKey = "4048849c1bfba6bac2b87b82c22c52b3a59885dc83b23bb9ff7fae3a1f299813";
	iv = "34a078618e0d69bacb4b19b631686d5b";
	plainText = "0c9120c4393ad8124e9fe93a0a04ab2a9fcc07b1fc734e6e0922cbbcb1c6d056";
	cipherText = "cbd8ccb0896e7647deb88beb0688151858f1bbee6e70dfaf90cafc35f6c2a037";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 178;
	dataLen = 256;
	combinedKey = "dd26ae8af60c270e04114037007bcd2b4df9110dce28f1080d1ad8d7015819c1";
	iv = "c1e4da905712b5e999858a580735fe61";
	plainText = "04d53aa586b03e04ec375a3ff5e1829b89bd5f2171e187d8f33f26e4bae4bad6";
	cipherText = "ece10ba75a58ffb35185815112b53c62c5a57515b4b811e3157bd5d6acf59ba5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 179;
	dataLen = 256;
	combinedKey = "47cee261912713727f7e2031bc98c7f57faec23438667caf4d76cfadebc0563a";
	iv = "00410c09bc2516e7973b9e2c3e842764";
	plainText = "bc0db82db7727356edce2ac5a5761a4002a697fa1e3c3fee1960738213f6541a";
	cipherText = "0f4bb01371ed9ebe8d4665bd9251e0a10c5879febc790fe3a40407acc45422bc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 180;
	dataLen = 256;
	combinedKey = "b2ffe72136cfe6b61d81a215aadd2ff7c59663946fe79731d81decf24be926f1";
	iv = "b706cbc26ac6e101731441c2fa2cc8a1";
	plainText = "a3eefb4cfaad4c5d93a79dde4b7b7d3abe4eed4c702300558e6ec884d11492cb";
	cipherText = "997ab70bdd9059ca0eaf13f3e76045bdd101ed3063582668cc65e127a5143d48";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 181;
	dataLen = 256;
	combinedKey = "a3f410eba2dec862ef4c2b2966bcefef141a318dade1644b4d869113fb1187f5";
	iv = "fdee2ac34f54f1afd5c8cbdbc92d6b1c";
	plainText = "63999d19f106f7e549df4ed39fd7f9df288518fa548d2f91fe696973bd9f8a96";
	cipherText = "708dd39a3abeeb23ad2b68a9e3d7fbba5c29273d66cb22cbff6131b6790ff989";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 182;
	dataLen = 256;
	combinedKey = "7a60d4af523e8c8341741b858557f653e4ec680b2184fd3a7eebcfb501599b84";
	iv = "02dcb83b94125354eced55b8894378fd";
	plainText = "7c8d8bfd081e5f514915b5fcbac8848a5c6b617c54cd53f6feefd25169830c5b";
	cipherText = "1dd8629021b11316e155c7c6bdd5dac27e4a783cb2c0a722e90aec96e345a14d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 183;
	dataLen = 256;
	combinedKey = "84e39898c01a7583d63a89588d7f21678c662e69f7ceccfad8fe95aafb0c2722";
	iv = "18e70f3664884f72278efd16e9713873";
	plainText = "ef4c00e214a80cc414a7dc72bf24ba603754facef9777b9ebfb5d651837a3d03";
	cipherText = "9fd22135b2667597129f0894a283482899655307e07b47a2bf0e788587854d22";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 184;
	dataLen = 256;
	combinedKey = "e5783de37c5c2421f4d5afea7e2b6bc44390cc75a1534a39078908916025a592";
	iv = "4d824958dbf6ea151a481dda3881abeb";
	plainText = "6f7338f14613863e2d8b03e0250a0653210f7c29a5e8d5151c7f5c254d73c885";
	cipherText = "f845449e89be60016647cecc51ad023bb8b1d5f7081f69ddbb4bb540804196c9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 185;
	dataLen = 256;
	combinedKey = "6f56eb4f5f925daf0e71870a05b7866edf1ea425a03d294620aa0a11aa5ca32a";
	iv = "1271cfb272f070c29c51b4f6e5351c43";
	plainText = "65a0d1a91ce6438a92907c152804c7d10cacb9a40050a2f8665ced96dc749894";
	cipherText = "2466a1b111007f25672cfbf180d59d3456c769c28352a1af3d8ad43f8bf56c0e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 186;
	dataLen = 256;
	combinedKey = "5095097cd958ef09a1781c1e7969792e17e107f283d40f13efe1b82abac8e667";
	iv = "28b1d79c3f2e79922c159bf9c33ffa04";
	plainText = "6720b63cf0f58bc656cf51bc130e9ed39cade14b09e66f062efb9a8a5132ac99";
	cipherText = "d36fc5506d3175f29afed429f0197eed5b660467a877c96d80b6ea8a7ef82d24";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 187;
	dataLen = 256;
	combinedKey = "dd2d73be1cfd9ec197befb9dc519db73266ccd17ef146f1011f4c3c4186a6ab4";
	iv = "1a79f1dae544d630d9e0d69bb3a437d0";
	plainText = "9ac83550c2dee72878f183439db19c07b75828f73645cff270c55befc61fe9ee";
	cipherText = "bede927accc06fea11269dbd8277f4137f7d3cde0c8ebdcbcf6ceeb1d73079d4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 188;
	dataLen = 256;
	combinedKey = "d65f059c365cec0916ec7750546bd542b9b62864d60dc4446155a788104b04ce";
	iv = "18dbd1147d6d6b9ea3b09cc685c84aff";
	plainText = "16d47b6427570495c11e6b0913992429802f593c2fd73cd8be08a2dc0d7c190d";
	cipherText = "f923401cce5350b36bb515f6b2a222b3f0e0e944666eeb728869af5940921ef7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 189;
	dataLen = 256;
	combinedKey = "0ccb23bfd8bb00060d728a0c4286a4fe148a73019e18bc74020b482f9ba69593";
	iv = "cbc73c9074079e0b9a5d8242a3815a0b";
	plainText = "eab22af948e6818e8d6c6e22c46e9bb45ca704ff3d3f88c680dcd02ac0dea94f";
	cipherText = "1ed6cd529802a136ac4eadef8e873933f85e21f041ded59409cb35ce5770e888";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 190;
	dataLen = 256;
	combinedKey = "d6250ff8814fcf5c56b5cbc16b63872fe16d8216ece3443a015b31ddddcf9c10";
	iv = "8b01f97ea86f56ea25a520627591122f";
	plainText = "55e0e42e7e7a32830e890398d42de8a6bad5a4e35f06e708feddaca0496ff3c3";
	cipherText = "0bd4f2f7ff747bfa05139ebd4dde9f45578b4a68e61608875579239b280a2401";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 191;
	dataLen = 256;
	combinedKey = "e8fac0515e137e6f502ad4686a9485e9749e251164fc6b9a84b2bba8f8b535bd";
	iv = "71f71fc025baa3be5dba771dd10f26c0";
	plainText = "e5f7eabceeff50fdae6965945f79daecefdb5bad381f9a331f111b0764b4c39a";
	cipherText = "fdc1a0506fee2c1fb26f8da692a19bb6467db8e89573eb4b772110e2a0968cfa";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 192;
	dataLen = 256;
	combinedKey = "f55834c2c6bf0ca9675ed45a295ca1fc506fb1b6ce9f75ea1014ca03658f8b8b";
	iv = "c9567323a7d5952f3311a34a7a1f12f4";
	plainText = "037f595a8092a30b9b973a9320546581ce77191b66b59d4abe7ae313a53ecb59";
	cipherText = "dfc47a1e0f1ab4600b7454c1d6e03fa005e9e0c8f68297ad40441fb8aa36282a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 193;
	dataLen = 256;
	combinedKey = "77191695e30bdec01b8187dac706e2d93b9b695646164fa1c3e964b548f02aef";
	iv = "ec683efce4f936d6ec7b715a0ab1cebe";
	plainText = "eb16ef9d09db4d1a6382a2e1386cc5b6241e93b9f489fb1ecc80e5c4d0636b38";
	cipherText = "71dedda5c684b3fd651eaa76394a89a5da5088189a119bc7dd83049739e9efd0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 194;
	dataLen = 256;
	combinedKey = "56e785d82ab69daf5bc8d840ae55c76270daf8e5a30a06d19723371fe94fc137";
	iv = "3df4db3f1368402cd6f407be96a9c585";
	plainText = "bad8a12e48b1e5d481e793d4069e0f183d16545c28bcf65aed9f4a88ad269da7";
	cipherText = "13b76e3d2fbb067da7ada818e1d7c62d11f68c9830b1778a4e17d09e24dd25de";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 195;
	dataLen = 256;
	combinedKey = "cd717bd6709b2f5bb930cb4746ccfa58d2a3fc7cec5659251f62bebb7b0b2fbd";
	iv = "9c3957fd829514ba3961ea9149b527a3";
	plainText = "3002f2ee9e6c39a0b9dc14a36cd19ce2f6377ab60b8311dcf5d1845d509d5d6e";
	cipherText = "66849b4b41426fe8cc914338db6137446ac59a218cd335d880023043dedce07e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 196;
	dataLen = 256;
	combinedKey = "b620c1dad56964e070eafa7e0efc3ab4c38a0f5fefc09169b93890badc3fd2b1";
	iv = "9b8bb985f2469affc9d60332058d7fca";
	plainText = "c292e5e276bf28ade313ce2ffd1db6a943a386cc242ab7699df5a51bbaeab0d0";
	cipherText = "b7e3feae419608710cde65a67707b8bb91880545d3b9b3c494ad9be6c6a1ea0c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 197;
	dataLen = 256;
	combinedKey = "c927db32fbe3cc3d2f81fb6af9ac387679c0fe7d0105ade9bdb26ceaadb4a43c";
	iv = "ef5ba85e322a843c757a95ce47657bff";
	plainText = "d296c49a3b64395b81933818c68b5a38de22b23b8ed47e94b0dfd646c9ddb611";
	cipherText = "f9189c25cd7644615ac268db342ef7ee0a38c9efca6219cccc877c39f9302452";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 198;
	dataLen = 256;
	combinedKey = "70f39cb194b976a6fa56c01d8e79f73c21b785c70f6d3c7a5fce4eb014be4eb4";
	iv = "a49282dd9d82e517fdfa223e5e32d2ae";
	plainText = "8beb3486fae1978bcaf78dbfad311ced5fd500f9a10c3779048572dfc786f616";
	cipherText = "4fc195349b6797946c5506da2f5ed4ec61707736aa2bbbac9183a5bb3d0b02f6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 199;
	dataLen = 256;
	combinedKey = "5e3c6025a11678745c7c28ac3932dcc89ae3d90277ec3044742c5e35d4d073d1";
	iv = "0d473cdcd031285916593b3cae4cd7eb";
	plainText = "3b6fef42eb4788b4859b63dec4470585b86c366a5945ed7fdc9299895735d936";
	cipherText = "5ddb078085e07aabbb66ae09dcb27d98e5a4f667f0a64b56f81da1c19549ea71";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 200;
	dataLen = 256;
	combinedKey = "23694bae3f9d3d4870e34497c86500d5a97dfeb47c47552213e1f72f66d9770a";
	iv = "121a5c7d36e3c156a74512f8c0fe52f8";
	plainText = "968ce29ed9a126c1a531ce40aed453646d17f375fa69200b0523816ede9b1365";
	cipherText = "71ee6e7ba532898fb76c9d2828cc1e8ad826fb4a6d666f25c25f116780730107";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 201;
	dataLen = 192;
	combinedKey = "58dcd1be774595bf5f5b6880b4b123b8f2bf175d335aae07bb17302bb2c4443f";
	iv = "5f9d62baf3628f790afcd7964be504fe";
	plainText  = "bb6e724e5343f4e1391262e39dd0975f7f82ff118a4d843a";
	cipherText = "96ab8de2934048158a78193baa893e671bae183a79fb7933";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 202;
	dataLen = 192;
	combinedKey = "e75e512b231efed91cd60438b0e213ae34b99f1d4fc9ad981117af53a40c9cb1";
	iv = "408094779056e4c612f1001ad4404793";
	plainText = "6a64ef535ee231498fc9212fda0b38254c86958dc8f09dad";
	cipherText = "7ce7d9d0515931b8ab84e8a4a6ba334c7374ded5df83d2cc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 203;
	dataLen = 192;
	combinedKey = "e733ca4c36e6174b55221b68ca76f2d40538c9d55b0a1011374a5843731906bf";
	iv = "fe36e49d0db2babea10731d391cdee12";
	plainText = "83d85b072b794ce4c6d2817103abb27b606e9db2b232994a";
	cipherText = "aa404725ae7a8b9ece9d406605be86c83c8798a3e0c209b4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 204;
	dataLen = 192;
	combinedKey = "8b4936d9fa250b59e181ec3d8073571124e166b1d1a0aa4557e3d297e860ac95";
	iv = "722e85866697b6360bc7f6beb9befa32";
	plainText = "c878a731c06d5281c70c6557c6281c4833baad19140aa6cb";
	cipherText = "bc59c2dbca762dffe83082c2cbb6e84634eabaed7c6b64b0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 205;
	dataLen = 192;
	combinedKey = "d036ed73522c1f32a75cb054143b87d086649abe6b26bd5610307ee0540d9c2d";
	iv = "9ddbdb685270f0b6717dde6317e2a099";
	plainText = "0ce23722d8eeafcdc8f5d3a5de187f499ccbd7ffbf6e89c2";
	cipherText = "a868ffb10d253449d0ad07b1716820c68e5da3a148db1a84";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 206;
	dataLen = 192;
	combinedKey = "d030ef0f358fc1c43a9b8c8654a8bf77fc7f335c89c4133f63c130f4a025f01b";
	iv = "b4337594524fe4b349f0088cc0b988de";
	plainText = "d5f569697b3afa39485a100eb5d58dcbd7b295e62d568f8e";
	cipherText = "5f929e3340b545836d7fe4ed1e8372cb222f5b053693c65e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 207;
	dataLen = 192;
	combinedKey = "84ea6dc7c780e71b158722d47a5b22e0ba7ed9a434fec1a310cda09879bd16d0";
	iv = "cfa31199d777a6be50fedc3d9b282244";
	plainText = "4b395ea31683f4d00434ceb66b4f76610ca05972887617e6";
	cipherText = "4e8b8a824946bfa0acf116038d2ddfd974e6aa5286ab8942";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 208;
	dataLen = 192;
	combinedKey = "a4e6557ea45523b2ce658d72763f2d616015aa029c89ec29029bab3849fd62df";
	iv = "93433ad27b4e265744424e7a511f2413";
	plainText = "128e41e0529f8d032c040acd2ddd126bc7ab53bfb1067d2e";
	cipherText = "3d989c699d40cc2480dda7f4f15e4ce66e8705651a6febaf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 209;
	dataLen = 192;
	combinedKey = "64c2edb4b9164a4809ca52c3f7110d96bb37d577bdf2968d65f525c02c677f02";
	iv = "4d478f5f9cdfeab3059fb1c06e7a6002";
	plainText = "75508467bdf40bb822ceb17708ccc7d5adc29c6333fb0d3b";
	cipherText = "fa67e35d488db3dcc634000925bd231ff30c69287df23933";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 210;
	dataLen = 192;
	combinedKey = "2d12f1e7de982a20172091e1a30198f02c238f8adf82728a305585b0153dd1f8";
	iv = "ec98eba8f9f8c4b619e5e28f7c0ab03c";
	plainText = "a50b44cefedcddb6d224ffa041b8c56a9baf7963cc900b6c";
	cipherText = "48af0bd878a3398fadec0087c257d3859f4864ef2e26180b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 211;
	dataLen = 192;
	combinedKey = "60dca2cf0a8a6c6d4b5fc9a9d9c3f7bfdeb236b1752b3a713039d80cee0b586c";
	iv = "a0bc17cecc4b6e2a44a30bca6eb1e3fc";
	plainText = "0d701913397b8155fdeeaba6e25800dcb5aa40051a8e0ba0";
	cipherText = "831a6711ece4243e6cb5dd6aec6ff6e99ffa896d73b6c070";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 212;
	dataLen = 192;
	combinedKey = "119b3f064672b787c51ad4f791367b87c179c4575a4acf12c4ab97b395be5284";
	iv = "d5fb2fddac5730f46b63597f0b26e104";
	plainText = "1dde28834bb9f57eda926b2455e16ec5763b611db4dd1269";
	cipherText = "f715a9a58f1d88851d3b5e03b0d6d083e094500b21d87a56";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 213;
	dataLen = 192;
	combinedKey = "a52214318f7f38f2a234d7d07199929eb44a00851125012510588db2b1cf6149";
	iv = "8ba4aa93c8e2243b8b22c417876defb7";
	plainText = "54b9fa2b40ad9abbf7a7c60a6c16f4aa4b4ce8b5ad6ba60a";
	cipherText = "63abefd45e0f603622c5d5c4c399f91650709e2a2aa5c604";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 214;
	dataLen = 192;
	combinedKey = "5ae3e19ab5cd033cc36c46aac7d8864352d6edb80b63d3eb8e2f9ba0d0f9b199";
	iv = "2559e196e5415ac4b731d4a32acabcbe";
	plainText = "5a2972e0b83fb46447aef2d6addb4ab84c4ce36b6cf7c4ac";
	cipherText = "1ae7f962476191931ba374461531432053d943ca1628e877";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 215;
	dataLen = 192;
	combinedKey = "36a0cb3a4b69c6948ee71f742192cc7325d027876cac1b1077e06e0e5a23518b";
	iv = "413696e05473f82ce6116989f75e88e8";
	plainText = "d80eacb237627fa36d3f3feefbb1d16f919832eef20515e0";
	cipherText = "b1ab67e9c467679b3af57f03ced738475ca2fbbefe2e8004";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 216;
	dataLen = 192;
	combinedKey = "d4149d796074bb288bbb6978d16273e34f46cd1d0c67fbdbdc43a75f67682d48";
	iv = "5a2b3a4c004bd31d249d30cf879a3cd9";
	plainText = "616038cd825d0f5de8c9b6a88150d97df8e10f0221a3c251";
	cipherText = "d977d93e41d038ba2983be4c83869dd206ec8007b26ec06b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 217;
	dataLen = 192;
	combinedKey = "0507c4f687e5d67b1bf80c39bb9c825b8875b35742803233f20109bbea20820a";
	iv = "1ac6a10bbfe427608ffbbae687e748fb";
	plainText = "da3a23efde1a691b8b091f3ae6f6657b81ad4149eac3a102";
	cipherText = "8c41078f7d1a91e216a5ec4ecc2727ffaf70ae22666a3ffb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 218;
	dataLen = 192;
	combinedKey = "860daee1882b803f77b6c260fd2a3f3f84f1106ebe4f35e9a713e4c25e554396";
	iv = "6d028f6e284c883f93d572c0738a2468";
	plainText = "74f3a68dc5dbe3bbf91a344ef1be2b47198b3054d4fd6995";
	cipherText = "5875d4e29a2b76bf43a63c3f78d774985fa5f94ae2d94241";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 219;
	dataLen = 192;
	combinedKey = "2403bca495fa083bf329a31df41bd777698975301e19e1c07a3822b1c46e123d";
	iv = "2d540cca55594d659b9a8cf34ac48b3f";
	plainText = "5a2a4b6e6739f752ed807c9d251f9961c1ae2c6baa64976a";
	cipherText = "762ade540d34e38cf83973fd0113f7cd30e05a685758a490";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 220;
	dataLen = 192;
	combinedKey = "0b0f444c99bed94ff6eb730900f9f27ded92d7b9fc9e16aea4a3602fef1b8421";
	iv = "4cdcefe438ff1d2977b43bd24ffd3594";
	plainText = "6e1f19b74715540a45a918cd5099798099b1f6d11eb0b855";
	cipherText = "8a275fdd2f95752086cf70e9e2641192b198cbc1905a5707";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 221;
	dataLen = 192;
	combinedKey = "40989dd90f7e18431e0d09c3bc8f729ff00be9ab633a21fc9ca9bdc1da68636f";
	iv = "8f5376432a9d902e08a5004a4683a0ab";
	plainText = "fa5a7250d86c3e5c4ce1eb3ea0d20e45ef1b10b9ba9468a6";
	cipherText = "6861079119a67a07c6fee59cfe9f0b1452dc0ab6a5477c53";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 222;
	dataLen = 192;
	combinedKey = "880656e0cd40b169c3df7b45739c5c34374ca809a1e368422a0d558e236304db";
	iv = "ea2ff0db4c96222bc8f5d9615041489c";
	plainText = "ecae1943fab11e6dc113124582982da58220bad24f3885d9";
	cipherText = "aecff7c2f13e2c754d564d4f62346b7fb2490a1d1bb98625";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 223;
	dataLen = 192;
	combinedKey = "fabd574b0a82792013cc967b657b8206e6519a249c628c13a5d0505839feb86a";
	iv = "c6ff61a9a551d6b65ccf23e7b7a5e1aa";
	plainText = "07ab2158630d5f47ebc2e6cf236dc40ac16f120d579e438b";
	cipherText = "306bca44401ddf7d0531722e6123836137dfe2a7c3c8df21";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 224;
	dataLen = 192;
	combinedKey = "2984e3e85eecf101e9080bd2cd8de733dbd88fbd2b0770896be589ccdcc93a36";
	iv = "1dbdfc6499d2ff9ec4ce35046dd58d58";
	plainText = "6bf174de979249087f685eb36576f7f10cf6ad6e944e3587";
	cipherText = "de54080aee291808414e3df242ccc970a5ab2db3342fb131";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 225;
	dataLen = 192;
	combinedKey = "a8aca703c4a960dba946191fecb4125207bc70f339a4e4919ab10109185875d2";
	iv = "e1867d390ea90c54ff6ef7bd7e685ebd";
	plainText = "97b4511d1a2bfd1f31eb12abb05c96a89b10987c52fd5cf2";
	cipherText = "fb6d18af4eb2cdca7ca73aea236d6290559460fccef637b7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 226;
	dataLen = 192;
	combinedKey = "2b4953105b59f8ba271b3404cb24b6f4d863cc8b43a511f1d32cf1223bb9b8e4";
	iv = "2f9eefad143acabf60c893ae572008b1";
	plainText = "9d9406d7cea1ccb5fce2e5a6a6914c55e3a4637d0dfb8a70";
	cipherText = "ba1828567067ff15ccef8d4e71e7f2e24f88669bc9d854f7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 227;
	dataLen = 192;
	combinedKey = "fb0daec0d265db482a80daf6105a2d7c6dcc5f789ca96be0d7d49080f2192057";
	iv = "8e3152ce1d813d7c84bf449883553579";
	plainText = "74458376492cff5002dc826a5a1b94f121d1ca5b6c87709b";
	cipherText = "f7cd23b4e932bd05fbaa0f00cd0d575d37ec041e9ab5ba2d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 228;
	dataLen = 192;
	combinedKey = "8c23cc0515604dfdde10834f435a092c13ce1feb82b2642b9a35fcb849b272ba";
	iv = "53586f47f2529e347e06781bc5e0d1e9";
	plainText = "611682c4161238149812eb17ec5128a6eb1e937603033513";
	cipherText = "848750ed005a79ceeb2ccfee1ae4d4f07c7ae1d2ef46378b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 229;
	dataLen = 192;
	combinedKey = "78cec5f095b1181c42562133c3ddcabe6a58f46c2463c6f7c9674147106d5ef5";
	iv = "f1e1404292af1d99053483768222aacb";
	plainText = "4e13c5ba74ec8d4d8c494ad17edfad404618135f978b2c1f";
	cipherText = "6f6c69726afe9812e16b288cc4b3ef7d36b6a0d3e49bab5d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 230;
	dataLen = 192;
	combinedKey = "444bb1b5ae7e36bd0bafc396077bcfa3b2f7cbc56e35f337c531863b796160be";
	iv = "a3284513536815f0870b80fdc500192f";
	plainText = "ec7e5404deaa31235516aedadd98875a72ff255eec04814d";
	cipherText = "2e9d6e985618b1c01570a880e98281cebc3cdf98a6724333";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 231;
	dataLen = 192;
	combinedKey = "00cf714c2231a614f1701c0689daf99c31397a39c2c36191207fed34be45d176";
	iv = "6fe41af548268677b7fc0f4c8217cfb1";
	plainText = "1910cd1b79d646e865a11656681029b44d63926f372d6988";
	cipherText = "2ff1390e003620d8c7b24967c551466d97e7cf0acfa7302b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 232;
	dataLen = 192;
	combinedKey = "2a34cf27dad8d839b1e7a4e5cecb64d0989869bf3f0b58acb12e4abe8a576819";
	iv = "0613358d1f1945556b356bd1788e75c6";
	plainText = "46e61515b9e030f12c6a8c3b58b21046b2de55d6c084742c";
	cipherText = "52570d9a386822b3a5408cb7c90ef7baf11a3c03f00ce715";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 233;
	dataLen = 192;
	combinedKey = "5eb635e5a692204a55a942203c7f1b96d439f6856b68e05a0bab906c5e49e11e";
	iv = "674468f99ff80e595a9b28080d4d8d78";
	plainText = "c422bc317a81179874b3d7c1c2f50467047966b6c6611a16";
	cipherText = "76a0062b1d43c714946c42aa871f02715e9d6f87d1316500";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 234;
	dataLen = 192;
	combinedKey = "5ff616b84dc6cef762fab7796399b020cba4dcdcacc4799f6e32e79d4228097f";
	iv = "b30e9784cdce7c5da9332dba264a842f";
	plainText = "412f064bcbb1d388f78253bb3044ca2424f131ee323b7383";
	cipherText = "2f27ed530358f9d67ade6fb3c6352252118f5911ddbacc4a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 235;
	dataLen = 192;
	combinedKey = "75fbf194212c655cecd8331d010ff41f9ad59b47985e46bfb5421a7ccd8c7cfb";
	iv = "8befc7337153c8077fcce1c4abd3c4cc";
	plainText = "764fa204c2336770a89eb1a7018d7da35a702b7f186e27ba";
	cipherText = "1b339516c59bc48271b260ca56f218b5ed65c26c13ba3f21";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 236;
	dataLen = 192;
	combinedKey = "dc62f4dd2c532ddbfbbe3258a544d265ed9f6ca4e71332062775fc261d6b7e61";
	iv = "ecdb45751a8a61ed784697455ad0a87d";
	plainText = "59a55490efe6b34d2f92bed98c4250769ddffcf98f9048d4";
	cipherText = "1c89e43d4543ba1425d01878fa94b9464954d8ee671b72af";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 237;
	dataLen = 192;
	combinedKey = "81e160a14776a81a12b1fd6970738823a9161c1a3ee2da940fdf3d2c9e3b4b01";
	iv = "e168e98b72ec010d2e271e5e4fec5d98";
	plainText = "73ed9dd6b0e6827fd7e9986de56ebd676553241233ab5dfa";
	cipherText = "5e24e4c8925cad4417a41e7c5b52c4eace7b4b0c55a493f9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 238;
	dataLen = 192;
	combinedKey = "e33ce9c857a0287073aacc58bd461e52bb22cdcdb62b84ded6c9bab66d3dbfbb";
	iv = "2057c203c73b54b3fa860a88987ce613";
	plainText = "191b946dea6518c355a2420b4d313986af0f2c44d0bcba2a";
	cipherText = "aa6d132963a1f44d2817285aa96bc0d492899b58ec6ebc12";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 239;
	dataLen = 192;
	combinedKey = "83af2896547db2fcc4f2ad63a048da769f723dad00fd911405c0294717037e29";
	iv = "8586b1b992901e444bd72d01628da084";
	plainText = "c89a94442167d03d3461e11f88fc05e7909aba3ad189ddf4";
	cipherText = "bc30509a68bdeebb3ce651f4a409995cf143c1130d8208f5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 240;
	dataLen = 192;
	combinedKey = "479f9c4a2ee3c3cfd5158d50537ec441bcd31c9a820abe444d56fadd7bb7d749";
	iv = "b62ec0474b5b298f8108e307caf77b31";
	plainText = "8b43f4eca273e2fff394ce8134d29dd577fa290501a65d36";
	cipherText = "57d79669b0e0a327c343ffd519ef0adaacea04a04db936c2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 241;
	dataLen = 192;
	combinedKey = "b03b9408a6d961e0c43af57a4d734a59692e670a63122c9c7016e8d3f68a4aad";
	iv = "7a338a1933a5889db9ace3b640d66379";
	plainText = "5a9391d456d4ee9be29b4890e53242065ef2ac40992dc264";
	cipherText = "16f87e6dd594ba8b3879d71a256162742901af23225ad625";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 242;
	dataLen = 192;
	combinedKey = "c9be1e0c22d78b6fcbb658517bb8ef29aebe3b06b936f5b61b341f9775e13323";
	iv = "c929d36f34431592143222617a1eba31";
	plainText = "c184a47cd2ef4938c5cdb6e8dd0cd40b15cd5c18566382f5";
	cipherText = "177a6d918c084146eb24ea4fccb6ebe2e0e71d1c5e63d01f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 243;
	dataLen = 192;
	combinedKey = "83e51a46a6400803c20d82147f5163fe61aeb8356a11a5de132449f9344e4e8b";
	iv = "690fc52e36c22f175b175b9dc768085a";
	plainText = "37d39328e35b89208f60e16819fae98076559122a96051c5";
	cipherText = "230fcbb71cd9e9aed6c86cb3b2ac85d3014c45300d211f08";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 244;
	dataLen = 192;
	combinedKey = "959dd01eee7d22e047135142d1622edb7e53eda6a2acbe1f9d28d02690d2ae7b";
	iv = "e3bc6cafb639f6de6a8254bc6202fe34";
	plainText = "708093b85b527a45a578ffff999a3ce40eecb2ecf7c50210";
	cipherText = "e2c9833cd519e40f2bd6b13d834278af3673741c5beca103";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 245;
	dataLen = 192;
	combinedKey = "d2bb39424139bd98e2395b0b5017d40c262bc18f4c93e167914c04d755ad4e81";
	iv = "6124a472684c1dd1955bf8a8273e3e80";
	plainText = "e9e045ec762e9a8471d20a8c5fe5f7cd1a180feb214a763a";
	cipherText = "dc118d535950bce83b5bb425afceb3e9b5ecfc47e0ccdbaa";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 246;
	dataLen = 192;
	combinedKey = "a6f009bf5cdefd0605ae42a612ef4630c996c4cbd283c2ec1c517fb1e961ca33";
	iv = "5ab6fc1be86cd8a789606f157807e718";
	plainText = "a6c83a1e41d8c23ae46c9a43651d5c48a32f048f206eed65";
	cipherText = "c75717555bfbc045e059528771a46d182b3d0d9946cedf9a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 247;
	dataLen = 192;
	combinedKey = "c6006ec65920d564bbbb7af75c0def30904f67cb4267bb8ae59dea637818d803";
	iv = "a9319b84adeab56e938c6825d70b4a87";
	plainText = "f7f96a67d4699a3159fea6c8af8f89f1d1a7d11f53b75281";
	cipherText = "cd23ddd93bffb09f740da649fd394114c2e700d7a0770c02";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 248;
	dataLen = 192;
	combinedKey = "07c4fbc70b5f4ec54f4a0615ed0d02bc54054b512770d0ae15c6679995354485";
	iv = "dd8bac2d20235ba4088467adcfd156f4";
	plainText = "d502cf061e155267eb9f87291a2a65c3809a2f5e0aa64d02";
	cipherText = "e9c673a1e8daf26ff8d42d667e10d80e646baebc286c98d4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 249;
	dataLen = 192;
	combinedKey = "77fba954027a1298f76e277f67f75b7b832a054e5d1aef9c9a26748f6d0143fc";
	iv = "00db4597eae09665f43116ab384ba160";
	plainText = "cd282ef3c5f92c2a1d437982194278fe50ad85c4bf5461d2";
	cipherText = "c6aebec6c4965ce62fa2eb6524c1d018c95d221e5de1aaf7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 250;
	dataLen = 192;
	combinedKey = "b1c2c0a992e8aa4c8a8efcda185e8376b6f2a3c75b7311c81456ec38aa71c14c";
	iv = "46499c4df6e33198f57c641f71cef89c";
	plainText = "269cc20d7d1e78452facd9826f134fe0613ec0aced7ac5af";
	cipherText = "827e58370bca1e9a295dc5328bbde015a95ca3694c57c91e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 251;
	dataLen = 192;
	combinedKey = "554498f565d90b8add8282e21eb4f87645adcde0ddd2af3685403e84b28e5603";
	iv = "f6ad34d6d5e663c0f9c343eb18cb9ec3";
	plainText = "4d021db638a80a70f49548bd08ada14a6278ff53ec8b3c43";
	cipherText = "34f5c08ef5a179ab230f29281810248a8f9bd245e048ab94";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 252;
	dataLen = 192;
	combinedKey = "c731eb57c4b56be1131e1f91e03a2b766629cb14b666762d6b6689d33fcf6a87";
	iv = "0821831122c0bf7f72c2615832e7d4ad";
	plainText = "b082187850035fc5a802ce52f1f9f9d9baed6a61f59a4741";
	cipherText = "f4beabb93cade84dbab0f3bf9f5f989b89b1e65ed347c7c5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 253;
	dataLen = 192;
	combinedKey = "c9ad52aa28dbf6f3c214a2f3de2773c26383cee6233c158d70c5a2919622714e";
	iv = "282767ab166d1a7264e89d0705fa2141";
	plainText = "688602458c6468139a484920ace83fa24ad20d3542827199";
	cipherText = "28d2131b469e81a163d29add1c5463a1bd87113d4543d909";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 254;
	dataLen = 192;
	combinedKey = "2854f19670c1fd147ca9a8ded5d6cec44bedcc027f691893d82e0f7e2555f101";
	iv = "cda61b9757c608fa7f92e4ea5e6a01de";
	plainText = "0aed79a1ceb2072dd0a813ead4a00d2aec32d26b8b5c6a88";
	cipherText = "f293cdb7efbc2df99302223875a2a4f98c94260f18517b2d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 255;
	dataLen = 192;
	combinedKey = "1c8dc974539ac4045dbcd876113e085e3786d16acc1489473fa5d4f0300af31b";
	iv = "fc91bfd3ce811f02da3d42091dc3d391";
	plainText = "92a7b91170a3269515903868f83d0cc18d25c6c28cf0c8a7";
	cipherText = "fd8ae728f3d5a27e88bda8b36b126ec85d0ca842788a4baf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 256;
	dataLen = 192;
	combinedKey = "5b8e02cf3fe0595aa7e3200bff6a2abcba8a85a154da4757ce9fdc15014d0c46";
	iv = "ea44c41b327f238cc69dd52cb5cffc13";
	plainText = "f96cf1885ada571e369de7a84556e540f798ff1bdc748cef";
	cipherText = "b9430408986a5b4f043d1d054ec472707ab1f4ef1f1b7add";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 257;
	dataLen = 192;
	combinedKey = "4428abe0c054f98c703dab8dd54cb0144e57f705bcf5d8674f87d094fd79f105";
	iv = "92d140c014671422dded741daa34f142";
	plainText = "a5064e5bb679127602d2ef1a0138f1026e351257cc9d929b";
	cipherText = "41a20a8c1941f44f71c516243ffb5b50eba4ec0b695878af";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 258;
	dataLen = 192;
	combinedKey = "e4ba3a2ce6e629801ba6e45691ffc95922886c2104f657d83ad151f9c5b60b57";
	iv = "369ffc3e4c2d2664b5db56384404d557";
	plainText = "5c7a5ad7bdd874d6dc7c72139690c44885d0c817cb897400";
	cipherText = "2e6712fb9fba53d8c176f4a4d8387682da8f4d3d7eff35ac";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 259;
	dataLen = 192;
	combinedKey = "7d4d49a02bb7d356edfb962653d0fd7582fc42156aefcfd56317c8e356304229";
	iv = "49966cdb0136dcd1bfa3a7ab5d74142c";
	plainText = "91ed05fe4c5e02998bb9318ade59963a8e81f01f48ebca64";
	cipherText = "4b063a75ae8eb225bdf5fc7308aa49391ccf968cc685ddba";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 260;
	dataLen = 192;
	combinedKey = "a74c21522d2fc8575d56aedc28f75c1d863401a576a9e6bb114cf5437b0843e5";
	iv = "b11709952a726aef4363663a9cb7de84";
	plainText = "74e725665519436f59b9ce6a813226650a6ed32921bed27c";
	cipherText = "d69875e881fcc4841865fed31d623e36f576bf7416759919";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 261;
	dataLen = 192;
	combinedKey = "754e034ef5a1a7daecb47b6d0adb386e1ffa9f253e0d0e859d35b1f08d812293";
	iv = "53ca1355e3d51814b318756dee865f63";
	plainText = "ccbd9c0579cda8455fb3aa2a0d8c6aa60dde3fc224c478ee";
	cipherText = "5d6c73280b0e07370a6c162c217781c522b679affc93c71e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 262;
	dataLen = 192;
	combinedKey = "8ec39e76a05d07405e5ebc0d1e8dcb0c12c3e9906d71132927d4b9265d577b71";
	iv = "3505382198f5e9ab011f3b0e861df633";
	plainText = "a53125b8d0b7c25fbedfb04b8bb5d82153b6de57c0b31edb";
	cipherText = "5ff7f2d4c722c8a228929013ae1e92af06b834b08b0b8d04";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 263;
	dataLen = 192;
	combinedKey = "e76226421749f0f793ba3beb05c8501de5c488f7fe6cfe2b3feca247e37d899d";
	iv = "5ae0dbe70fb112d24153accd8ecdefd2";
	plainText = "4ba61c42513be932b4cfd4c4e38ed4c3776a6132e5d5b7ea";
	cipherText = "6af72c4736ea63861f40c7258e89d72cb3ba1395f37ea025";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 264;
	dataLen = 192;
	combinedKey = "59a5a7e8297292df2697d823023a9d8446b2edf59a77d81e62a36846c737402c";
	iv = "6958af3b20f6aee0fcc62aae80506368";
	plainText = "f40765a38ad8eda342a08776a3d9d1d1876264e8330e2954";
	cipherText = "ad315bdbd64b6eb339b4bee2e8360381446ba1e2471bfaaf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 265;
	dataLen = 192;
	combinedKey = "92a3e074db72e54902ca8cf5504fc46a63b583a5adb7b99f74d1fef051c3ed6a";
	iv = "ae0e5deb4bc9738b59eaba9b93a9e5fd";
	plainText = "38d15eb93b4b4e7956a0b17ec2d4d48db26d73348dd25935";
	cipherText = "8754232345522f4b592c0b42fd766ea0a8d5943fcb0186c8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 266;
	dataLen = 192;
	combinedKey = "e2abbafff6e5ecf3eedf5a36bd54a90a7294678f975271983bbc736038e9eb03";
	iv = "01c535c829628b39ff9f69ea30cf1653";
	plainText = "be55771576e9d661f0c2fc1478cbf81ecbae5643ca62493b";
	cipherText = "6cec891bf2d17e5372c5664d2085dd1d4de8ef489dfca2c8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 267;
	dataLen = 192;
	combinedKey = "ee7ea9b5f0e2ffe4ed9bae2f171121ba7163d4b2ae2ae442f0f4cb29920b9ee1";
	iv = "a1357f5351966bae3916e5ca4ace2040";
	plainText = "172539f3b1a195cccc6833713173700d2ff4e3f408203757";
	cipherText = "0af5cbc54e7c1fc276b9646ab474a2b33061e2c2e0e11f4f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 268;
	dataLen = 192;
	combinedKey = "5982b4a419a206bcc9201c392be22b60f4febf2ad1b7cb33968393a56f38f641";
	iv = "ef7319ae387ff4963d4580b5095899c9";
	plainText = "590d0acebfa088501227d51b59c9b2b19695bb89450107b8";
	cipherText = "f8eefa9d3ebcc70a028d172ff8a11ac9fbd6e68f79942054";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 269;
	dataLen = 192;
	combinedKey = "c161ec880ecf416d283896463c14acd8432d17e0bb0b28b0e961b5c4d48b18eb";
	iv = "8bc121db4d2e530e0bf9f6b771059ff7";
	plainText = "7aa7baebf96e2695055445c6fb64e9ab9bdcc0f2475b1ab6";
	cipherText = "7810261edba45688be33f87ddd09334ea5c51d8e8187b372";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 270;
	dataLen = 192;
	combinedKey = "50e4a004954ad740f4e9449e419ddf99df977ce956eaafa7d139e6c0a760d0b6";
	iv = "1386e16c78a1b63cc45779964dac59d2";
	plainText = "bad6de5f4c10b8391b86e169a94b75095d6011ba41d35616";
	cipherText = "44a56fada7887a9c11309d4f15b60f2086b6aad6f9af6fb6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 271;
	dataLen = 192;
	combinedKey = "672adfc4418ab5aca0b9aef6ac85bbc9a4f4d1701e67d844ebedf493430f712b";
	iv = "c80a9c7c80e563f68649f28100633efc";
	plainText = "8fcce65e3feb24f085f604cca1e7c6b709a4ffbf05e41b5f";
	cipherText = "a6e07ee9f1e3dae4adb3a28d69c8ac89f4fc620e6865620d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 272;
	dataLen = 192;
	combinedKey = "a7c2ee504cd4bb0b84b0399a7b85ec492e896ab9ae52ba3f06799aef9c8eeaf4";
	iv = "505191be00ee877f8aded6eb6d863bff";
	plainText = "3df7d778429a2f3fbee2e50aade8d4540678622860e04fb1";
	cipherText = "a0a1a7fb434487d0344aaee75020f82485cb266697606792";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 273;
	dataLen = 192;
	combinedKey = "ac8273705b9b948b713f511d0f380f96e61586874764053a4843fd6870fc7f37";
	iv = "536af19cd75c6eface2fb01a33a545a5";
	plainText = "e525895015718f71ee87dc8a60cb0cca2c63bb84efc7f9b2";
	cipherText = "71242b1e71b4d726871d1e050104465bb97742e00efde136";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 274;
	dataLen = 192;
	combinedKey = "abc99c4653a1ad75f50373166951f87e6b119f45ee8c98f129b79004398af844";
	iv = "b2687f07721671ed92ecb44ef09c2db3";
	plainText = "a5bc2bf156e1746b4704affc7339f8e3a6b787e52b8ee471";
	cipherText = "4c2dd14b495996698b5476fa94cde44b506bf054b4a4a63f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 275;
	dataLen = 192;
	combinedKey = "b47e98c44367332795e3a8fb725cb42c6372ec22f591a60456a7eb9922a93d26";
	iv = "d63df1ca8fa6ad10e592fa6aed70051f";
	plainText = "6aa116afb49827a3de11ee3e21b3afaf40dfd3c02cb52720";
	cipherText = "694341279e4e20c383a0d9b2fb69919b9231ded277cf160b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 276;
	dataLen = 192;
	combinedKey = "cfc73c32028663e260de49302228145594cdbc66550c6d4a4e21b24964c90e89";
	iv = "002aef62589f57e93b0d1a32ec178193";
	plainText = "8e43e11809e493a318ef3a98ff1f85d66d9bb1613b250334";
	cipherText = "2a254e09dae287364aa1bde30a805875ebaa7b8094b24e4d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 277;
	dataLen = 192;
	combinedKey = "4ee59c5c37d61a755bd4d305bf32771536930b208f353d4c423a6f8dbb5eca87";
	iv = "50594f7f6c06add1a4aee8b96589ec9c";
	plainText = "d596c504a65f5dc651f926d89b761f7b644095c3969f99a1";
	cipherText = "f807cdc23f9916c7ce88bb2447511a7591ab571a2fd0bfe2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 278;
	dataLen = 192;
	combinedKey = "3b601df82a93f7f88fe161a26b5d7ae32572e5ae8cfb0dfa93190eac99e74111";
	iv = "0b1ba26b8e74c0128f516ece51c5ea2f";
	plainText = "9d4b4339954de9fdbc4bdcb4b7ff4493eff049e079f75738";
	cipherText = "c1520a1404bc01e3d9afa7d84672e37c74a1b794da2ef156";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 279;
	dataLen = 192;
	combinedKey = "3fabd3370270910769d6dfc4ecba5b86cca84a9a167cde673ec4795b1e0a2758";
	iv = "c074acafadfc236fcaa62bec0d878911";
	plainText = "24c8b9b952b1cc9f1e8829bf43c0fcb9f57939e1a97bafa7";
	cipherText = "e0919ec788edf222896e24b04dafed729d790effe09f77c1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 280;
	dataLen = 192;
	combinedKey = "c9d06a81a355193e29ba49b893e021682cc6e5294c3cca2d1f34aeddb4643a61";
	iv = "5040847dd028fcd388b5d12998a1600f";
	plainText = "3c6d62fcd550679d24f1ecceab099a39168c8a8db6cf5a72";
	cipherText = "9ce83e974737845b95a6df55bc5d17a8455a613f0cc33484";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 281;
	dataLen = 192;
	combinedKey = "74ddca7f5b371cee11e1ae5b7e0ffd13caf97f30ed5c5c44391e8ba65752f338";
	iv = "dcb518f15b09aa4bc906291e4e1db422";
	plainText = "6d633684d8334aca001600a28b089bbea97ba619855dc925";
	cipherText = "191e8da0994d3d0960d1d090e1342b55281d4f6399a8d9aa";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 282;
	dataLen = 192;
	combinedKey = "04298fc4bea37160003adf498f1bc8f2fbe6c5454f7642c4a4dcc86cf781ade4";
	iv = "43c14419e232f327aaa88a612a11814b";
	plainText = "bad9d0de4afa7600f5d380351140632974dd8dbc34d155a0";
	cipherText = "485eb953a325697251f0b32374822bfb8d8f70206f2a2606";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 283;
	dataLen = 192;
	combinedKey = "88214fb598309bfb4d1bb22d5b442d248a8e2a07cf0a90d39bdc23854635ada5";
	iv = "d7fadc8b9493cc9f03cf2ed2b2c03478";
	plainText = "62accfbf5c5a178bbb6ecf124060eee3f985b2c5e89916f7";
	cipherText = "fd94e64e0ca7f87f6f10d5e4088da71066c49c065f542c39";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 284;
	dataLen = 192;
	combinedKey = "46885303da5cc733a7b1bd776c8129b5a4bb5ba0db28f4cfc0d25c35395416d0";
	iv = "ed955f6e5e2ee63fac8a7f6facf24dfc";
	plainText = "89f7e516bc869bf25f3e751f956061e8912e53ad0d9d8543";
	cipherText = "dcb1f7ee8231a27732725a92989c3fdf1fdf4aba4bcaee9c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 285;
	dataLen = 192;
	combinedKey = "b70f111000c670cb04ce89d63795a28584173495d16af2cfcc07774e9493f924";
	iv = "84a2bfd8fb29f28c86ab307dd676532c";
	plainText = "a029165375ff34493b1e898915b6790f3a79dc2ccf0adb96";
	cipherText = "137cba7f486038d0651aaf4991e79f1a6956916825c1caa9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 286;
	dataLen = 192;
	combinedKey = "53feb988c30c9f149816803f5cbf4b5939aa973242cec006577e76b2b2df3d24";
	iv = "0807a5be65b6b20b4ac91b9f6c09de04";
	plainText = "590337e3cee42f8bff8639fdd81879300fcaa00974836485";
	cipherText = "4856eeddcfc1fc40b6a266864a05aab8d915541890879d50";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 287;
	dataLen = 192;
	combinedKey = "694718f85dd85e327cff5cf51854fc30c787adce8de573698a076cbddd2a70d5";
	iv = "fef6bf2957967ed8aec1abb830a169ae";
	plainText = "5ee7d68d296b00b6f9b3c2b1157822b8fb867ba9491691c3";
	cipherText = "ff00b95423b7c16105eb81d037fadef5575e423f72f50a63";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 288;
	dataLen = 192;
	combinedKey = "978ea40886e79ad4d156c5f0d7006b09a21659a1007ace4552873ca92dba207e";
	iv = "82d2e6616a39ba906093a7909de03084";
	plainText = "5b1fdb5021aac75d3860dda19b438a65a2cee667c5b6a47d";
	cipherText = "b283a8429a209d824ce65d62da05a6274c4dffa1e23a5475";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 289;
	dataLen = 192;
	combinedKey = "a21832ff8c6d5310ea702f9f1c98fb2a85737d6bdfdb162417f37e225a6cb733";
	iv = "437fc74eaf80d456b90ab321ac86704b";
	plainText = "8040c4bb8d9e268871cb9a7fe0dc4feb00f88e5b83e2d518";
	cipherText = "c3a07f0306f1f9890412d221d633cc1b91230f12ffb0949e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 290;
	dataLen = 192;
	combinedKey = "08a38dad8f4c5c2ac97a9601341f3911a712b8b5ac259434357ae1d5d478f36a";
	iv = "7dec5ffefc14f5d31b1ce95ffda09b43";
	plainText = "5c1ecfb16b81d04386a0179da4adfb0f09cbb26dd2820fa5";
	cipherText = "b899083e1c2342e54f38aa635174a20da746b83a8ce07c2e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 291;
	dataLen = 192;
	combinedKey = "b21eb3c4253c88f6653ce8102860e570d748dc7ef5cf6f606fd289c746f71571";
	iv = "3ec99b802fcfd6562dceb70b8e0eee8c";
	plainText = "c42930210c6116a4b6b34324a514062b8d0487b5775cea40";
	cipherText = "16f68268ea6bce874b08c4890828acdb59791740a6dc07ce";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 292;
	dataLen = 192;
	combinedKey = "846fba38a117b22bfb84d0596471dd11503c8d5e7ba80b669a96655bea35b6e0";
	iv = "d8212784b3b77c9a663aef0858266899";
	plainText = "1966c23af2b2387ea7c4b70bdb00371f12412278a0b7c70d";
	cipherText = "e71f539b783407dd4983abe90a1068b394f0c15d43acdad8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 293;
	dataLen = 192;
	combinedKey = "f8bc88e83679dad11eb7e4afeb37d291aecd5578b56a6c2945a603cabc32bc13";
	iv = "f46a60052423ae39b10d7a459a11bc87";
	plainText = "fbc71eec2192d52f09bff2db10810cb9a40840d75ba0fd39";
	cipherText = "28460fef7eec76e056773e68277b13058096ca51763e51cb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 294;
	dataLen = 192;
	combinedKey = "57b4273325cd109e26304285e26e4d5361e0274488330c056b57c87ecc9624b3";
	iv = "3b908d77d4e7ca2af7617e67aa034b70";
	plainText = "2851ab889d3349575fb5345d63207d991c9dfc30887eada7";
	cipherText = "91e0874dd0357a3e43d753cf13b75cb8fe64b353de1c95eb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 295;
	dataLen = 192;
	combinedKey = "e0d7529f6e77a3a88725309da07be4891b48497653e3a58a15167eda911cda92";
	iv = "fdf2eb54938a3daa2419f6bb62e0c42d";
	plainText = "2dd6db0972fdf23ea927492785d86bb8a95611707682489f";
	cipherText = "66795f5b29cbaa74e16190336231c79f9e88e53c1d44aa4f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 296;
	dataLen = 192;
	combinedKey = "15dd3bc6ccd5bbcfbe5b0f8fffdbc86785190366888b8c1385f40854e86ba42d";
	iv = "ea67164de6b08cdb33debc9c4b5abc0a";
	plainText = "4ed7e1100463e67ca0ef177b282860c95f3df95c74cf0ea9";
	cipherText = "5c0f78416b809448b9da687d33b06b9971400c9ce7f53355";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 297;
	dataLen = 192;
	combinedKey = "0ec4ef2e750958a2b1bfd2f934a6ba4b1bf7a5e0f1135691a07dafdac38307c4";
	iv = "e29566668d501a6b5c598068534ef166";
	plainText = "3c3a470ab9b74c9a321edd055f347717d987f4a4c0ca521f";
	cipherText = "273724b9edbffe6f18255479ddafcfb38be6ca6f265b2f15";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 298;
	dataLen = 192;
	combinedKey = "c5669c3c55b602d4a396ea2e858963e24ff4f26b3d54f22628d436960f1f4bbb";
	iv = "53a7b5ab5b9d9637d86eedfc1c8381d4";
	plainText = "60ee0d601eee70a3f49314cdf751f003f17a9bc053af2fd9";
	cipherText = "c140a2422b1d16da0c88279755401490345e8518794f2d23";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 299;
	dataLen = 192;
	combinedKey = "775370f84eb869df06b5a2089bd779443c0b12524455d064d9d6c60b99c04424";
	iv = "6c45366eda54344b2f45e802cda36a48";
	plainText = "6aea3ec1cc8399cfb5254285fad19be0e2210c42fe018cca";
	cipherText = "7438cb679d0644f3ea5f8749d63a9b240b42e78ec1716736";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 300;
	dataLen = 192;
	combinedKey = "bb89e6108756e04339060dd642c842290c1a52cc416c3f60df7607e511fa79f0";
	iv = "6703f5dbe4af41c2dd4c82abe7c4dd6f";
	plainText = "144e50f2d6292e225a0baf8cb0aa3e4cb33359ced3572c15";
	cipherText = "1f17cc6968286c151dbf4aaab11c05105fcefcc7a8ae2979";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 301;
	dataLen = 576;
	combinedKey = "4f5ea01ac23120fd07d0494e0e395d420a14cd2043c0d3749dc783ac0c9122cd";
	iv = "010c3ae44f1140785f4b9e37304f03b7";
	plainText = "9ff385167b97be7a66783d3a9a9c631d3575839503e87fbda744ab8f7a32e1331df2490058c50f715abcd2214073565538a5f28c15894ac2a0f2665c307e08853133c00d2a02fd57";
	cipherText = "8f43529ef67cc5de025f306301dc379b785f097db95ffc5671d98c6da3a85347fd42efd679be3a602b2aad588143d7e4fbd371a87ce309a337761e0d4cf57533f46cdd34f0003bba";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 302;
	dataLen = 576;
	combinedKey = "78dee6a75cd5e0444d637316fde6f2fdceba1c5a9692cf3e569af8ffa15323d8";
	iv = "c17190d88f064914cd46215972f8223a";
	plainText = "68b457d0056c1d86804fbb5da40b786081279308c73de577632661fdb7f42d38347795919e5df5ce0b2d35fe9a195cd4cfef84a3ead7e992e15262408b1a07b1ea82a6a9a52028df";
	cipherText = "d2d7458ae2578162770b7391cb132ec962fc7d78b79b14ca537d3ec004bc58050dec70d0cc26e8eb1024ea8cc2a383ba93b306e81c9625de59e201dffa449ed54bcc298b478dde4f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 303;
	dataLen = 576;
	combinedKey = "92df22e03a41f4abf00b37137fff7631370e2f4924708fb7d1b5ed6fc869af0f";
	iv = "20bc327c12ae85090b103cc99a07b3c3";
	plainText = "5df90e16c440180e58a931541d771fe2e5117ff9228e1d678c729680f30958f795f3fe148149541484f187eccca970d9a39d19c76154be8c6b5d2220ec4fe437f0daccde0b05f082";
	cipherText = "4c57a6409d69a359d2fb43aea8ad09c87f10abf5517b9bd8f141921fb6cb95bee3b64c865f885181c078e37a08aff35a066060764237de79025e181433f52ea38c5d468db22e9ac0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 304;
	dataLen = 576;
	combinedKey = "fd5492006b8891d333963163246897a8ec225528d2114d35e4468ec704e67969";
	iv = "5c9836986eaac8bfb0d195e4203e7bd9";
	plainText = "0fbe3c7691516914c8fa67a21b611bb70df44a7434ebe9098e5d6d091fbdd91160645c985bd1f9eec2a394df785e05210ed5d8a0bea2a374acc8e263f42c0857c5acfd2d6edcfb0b";
	cipherText = "083966bc5a7e07d1450450c88d9f6e46a752f194005f5032c1f14e3ba1098e0987dba2bc467346f17c19db09e83d39d4ad6e6772c7266b393a4b12f941e5b31f85cddd3af5ac977f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 305;
	dataLen = 576;
	combinedKey = "e50840ec8f8cf588901966914c3f9286890c23c15a681db1246eca2b7b353518";
	iv = "18a0adb32c86c294b45e10d8460a25e1";
	plainText = "99b7233d25bc5464f61b8478ce0fde93cde99f1afe8a66ebadf6a0147406e0d818ce90cdbe77cfb64d31e0c41ce6ae5389b7f6ce2599c6245afc0e1fc0d32eaaadc1aa5f50ab7714";
	cipherText = "5e199949fc5c362387a514889d66be0a5e3d569f57025077579a168d89922c7946a2882e3c05996a1a30f3211805ca507c2078f20953177489e3d21ea84036b1ff241ba1fe8a05a9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 306;
	dataLen = 576;
	combinedKey = "d062354708ff28a89fea1ac431c93ec1bd3665e85f9f186d4b6b0bf32ec5de59";
	iv = "f42df1c0faed7e7f1c20c1416941afc4";
	plainText = "e0dd1a0dcc44ef74a97af533048af70527a1f0bf5623e3a3738109a65dbbfe1d163e930eb129a7e893bcf46f1c9f9a83ccdf7fc3b15f3d30bc2a37dad1418a31196fd4ecd1b86103";
	cipherText = "78c048a20b473410fba0006749be273f37c42bec661e2718f44ef0d3de55b3b74017f7877d62c6d9f0bdabc393ff5590d864245aa62f33e71692d23efadcc3d2b301946d3fe6ad37";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 307;
	dataLen = 576;
	combinedKey = "f94bf9199c2fd409241b4867c9616f0393ae91e613c9e75db3d542ffb60811e7";
	iv = "b8425eed3cc5b7ab272ee965eccc46f5";
	plainText = "c9301b6768dee7268d2ff23586e89a2bfd15f4e869d1b24f3322653ff8e93c012d9eeaa31edd0a167a0d8e6d724a84b3bb73f4accf22e933742ee50328f694c641032ad6be30b4e2";
	cipherText = "7c714f75b7edb36d1cd9134bb20034ea7bf82e04912a946d8091ee404bc4cb64da4b4bd9e4db15a33b885f51b64d22eca36724b92c7ff76c08a87aee3b0a5cf7a0d1cf60a450c595";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 308;
	dataLen = 576;
	combinedKey = "5a884bc3e0803388803a6a81111f5668fb4194b115b050238f665798e368b4c6";
	iv = "e3812527de170f8c973b514cff8c7a3c";
	plainText = "8f2464bab88b941efd3d5295e51e3ee5f4246716abbbd868183439a29d6b36a65f6f789ce924d2d3613235cd6d2c618c08ea8ff74f3f971a501f5088e0d2eae50fbe140a5f2d8df4";
	cipherText = "ff6058f83f32411a0cf88a0e0d2e9b387a889a0915dbfc51512a52375f85e5d75881a7ab1f4b356a809bd0fc0e62e26affdfa0f628d14c1a0c42d72f2a0292a3b19ddd77235f5f81";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 309;
	dataLen = 576;
	combinedKey = "116c72e4af91a8f1cce7eebc772a712b7ef58eee49f3c1d01c94d07227366665";
	iv = "23b0ebb89af01002b84039bf248aafc6";
	plainText = "d0d9d7c9e91a69148df12619889fda51407121a90bef151ed4db00a82a342858c6437b3b82d916890f27d4ad92f042d3cc94a2bb29083198735de1c2658447e93044869e895b5557";
	cipherText = "734744df1aa767b440fa1f55021cdaf11380f2848fd6f685a4acb6b62ca879c54472c58bb3534655f7e549e873ab4d2d2922757dd9015ab8a11472c77cff3e38173428f96b3342bd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 310;
	dataLen = 576;
	combinedKey = "3dc513adc70a8bc420156facfa7ea56a891a47c005c59f747747a5c8191041f4";
	iv = "87234f6483a299b34985a608218580b2";
	plainText = "4cb9048ee8dca2935a6ee4815c183c26bc4ccf73c0505724677ab4b4e4955de770e6bcd6dc96fa82a41b70302ad4a1d7c9e0ad950279cf15fbafebdc9d54afe439aaf1af6dbae465";
	cipherText = "9197a1bfe557852e4bb69d0f00b75c62bae9f2155dd1869dd721eaf0f82ef4eba387feb2a818845987cfc63d6b65b5c28faabc86595762f9050f0dd4e3bef363fb937a580c183cc4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 311;
	dataLen = 576;
	combinedKey = "a2be8628e22fcdc8248d18cdb89d867484b592f83a80a559af172a51107e5f24";
	iv = "551e423e4caad7ea2428a84e639fb70d";
	plainText = "782ed2aef970312199e3c9d368435981458f1a35eaf6e50586143a24afc024b338470b3e23c6c7d3c1edd86f01d577fc29a277d078ceac06d662d143571998564917c08195a05314";
	cipherText = "0b85212932badefde17179c81098eebe4ecd01a921133b6daca8364b48866c0030e5c35c4981b3610ca8a6167a8f62981d8c79fdfd8fba18ca0696d1e7b9d01ae61a8b76f6c1ed86";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 312;
	dataLen = 576;
	combinedKey = "2361677bcfe9e6ec35a708e1985ea7cc18bf02de06a0340d8bcdfd76fca171fe";
	iv = "3596b0fbd4410ebc34f3addce0acd0f9";
	plainText = "abfc00da8625472ebca1f95056de9c32085aa44dd3738f1e30037fa48f51f61589cc295300ef21840fb8dd8b9f5491985d53cb44574343f5997567a0057d7f8226642a0a0bb0dec6";
	cipherText = "fd56b3b4067029289a08b6d763f8d3754f5775b5f28be5bc8e6d51a7090314382ff527d311753e9476f04d01e8ce97cf82256ba9961531d547fc80e63cbcc256fc3627205ebfeab8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 313;
	dataLen = 576;
	combinedKey = "5eef3e1a3b4fbb552d187800e35a6b69dc5768914afb142b4ca5d43521cd03ee";
	iv = "f2da0ee1f3ea4f8e65553fe4aa3db479";
	plainText = "6faea810fa613ead15899825b745ada9c4f7037b9aca4b5d862b84c0bd3a0ac50f36e86680bef25772455fd7efa932d38eda1d42c6cb144c71a0b6f84a68a9468bad55831fd40b34";
	cipherText = "7a71edb37ef0070fead75c5e41e223b5c3fbdcae21924ac3b8384ae292fd33553849862b49e5045247e71f4a4159e8b5edfa3a347bb16ca118e0e9e9992d2bf6ef6d4416ff278842";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 314;
	dataLen = 576;
	combinedKey = "bfbfe981ff957bffbc4682e22c22d8d6412447579bb9ba4bdf0d9c879d1e1d17";
	iv = "068f9da2552bcf2d712406531b2a2ae0";
	plainText = "cabc637a64b054976330d1ad373d819bc5b9be5ea90c88a060c06c3578f3549f3895abc251fac031a3f0e4be9b7fa62b7ee9f72abefb00a2d0ba5f4b3759de1b2327ea965490e159";
	cipherText = "a55d0f50c12633c8291e0b59443ffdfdbc98b9c3aae6b58fae2e4041e4513aaea8e8b6ee60a987b1db1b7332646182ac26e9e9495c5647cf8da721685a530bb1dcec87e1a2a20537";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 315;
	dataLen = 576;
	combinedKey = "d2aae8dfd134023d4af3c87496a1e6c476bb46a9ffa50dd2c7f3eda3e30d6de2";
	iv = "1eabf1d099d8264073af0f6aade42eaf";
	plainText = "83abec4ea35d305e1d634e3b2fd680e7322fe05da3ab52005e1489aa3c93a4c459fe7f7b771807482df4b97e52dab75419ccac3dd08355d209584080e0804f2ca0ab9c000c2c84a4";
	cipherText = "c31eca980a5a23457f21f5ef41501c64e2ccca8ae5309bf445f56e7a588d80bc8e00ce66e12bd6c8319481952cd20c6e6bf2524a5d4cc886de82d20f896656ab56bd63d40b44f956";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 316;
	dataLen = 576;
	combinedKey = "39a9b5c74526b6314735a1dc16706897d0fbe23ba76f088b766661d271988998";
	iv = "e3ebbd8ab48996007256d9789e15bd68";
	plainText = "4603b9384eda5a3134468ce985403e35aeed85b3675776c4f4b6d7e9b1e624f0370835f1cde45489d6f3d2219458b03103fb0d264f3e623b06fae194a506117ffe045de923a864f6";
	cipherText = "427fff890cfae5c5069a1df38a14a94bc89dae71abeddf0a72de7e67793f23f5fc8055086c1f391f71cc945dce68ce0cd8f4a6c519f6968572ba5c39b15a041bc85aa4e56eae198d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 317;
	dataLen = 576;
	combinedKey = "1c61f6af52b4196feceec1c23476058e2fdb16064437134facb8b8d09e2f122c";
	iv = "3fc7211d288a726a48a62e87a2d463dc";
	plainText = "68626660cdb444e1bc873a019c65739d40d770cc3b0b64c7a61f3ec970cf06828252fdab0270b5dc238eeb914966a41da9f77020eb7aea8dc3882cf967de67eda91f3500ddf68472";
	cipherText = "62ab0b70a2183f0e2f7d7c9a78c93279e4b539bcb8234b03273bcb0ec52aed5e6f2445f250873e2ca54edc2932c2d7b1012e343cf4e243c003ad434b48aa1a90f49e054c12345b0a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 318;
	dataLen = 576;
	combinedKey = "b019486656f331fc3f5a9a35faf93b57521072f2bca4ad7f4014a888420a28ad";
	iv = "88e0e9f8bc5e53a5d8ed4fd7496c71ee";
	plainText = "bcc00bdbf525ef48ff0dccd78b3393de252e0724b4d2f42b9007778dcddae7530f9384df6a5ce5c268dcf861a1928a27ce117f1cab47b7d42657f4f264ec1b73f6469fb33655f233";
	cipherText = "908eeed952bbce591280c4ce701814c77486780beb99c4da49659d3772b1e27a3495b5526f1bf0a6bc12087c491f13d301464b27d4e47dcc7306b6e9740213730a49a68a110364fb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 319;
	dataLen = 576;
	combinedKey = "59ff4232505c6f5c6dcf6b80aee1d6621a674f16bd82dc9a8f26a17f16513bd6";
	iv = "822c794d99db4e588604adde02ae811d";
	plainText = "58615e3ca717aa49fde838f843d58e14d7f696ac8aea179989e10f6bd0cd9ec29dcdb757109e232a65c5903f2c252cd691128c8fa8cdbe42e8e53dfe4650e51607adff61aa42573f";
	cipherText = "c94b201698a88002d846b878f9e97b33f5ca0b273d6a273433a27e2f85be0a86540abc602e78c7908de717a81353b993f58febaf2942dbe6cc7c8f7920f0f73b4a0b16048c8789e8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 320;
	dataLen = 576;
	combinedKey = "57341022f4245ef0a0e1bc542df68975d2ab7ff6d91a8332901dca4b0040a26e";
	iv = "83408dd9f3c5865805f5870f03fa1df2";
	plainText = "97ee791349f08c6810ce2582f77518b48de1b01bb5c51ce5a1eccaa5baee924dfb5fa2f8f05808229ba117019abf51ba917ab4543e94e93d2bb822d6aa33e1cf24f631c10ac7eb54";
	cipherText = "1670ee14cba05c25573a062e019742a33ff1a7286c1326b6ea698d4fb7c62e8c9e3a06098e07e28b95a579f3193d7b47d5c8bc62f56eb4195855fd180fdc22042f1ea6d89e0e4ce0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 321;
	dataLen = 576;
	combinedKey = "f8d2899f35671d41ba261990a7d78722f6bf7a29a25d57c10f2b55e4d2c26adb";
	iv = "093d22f99aa8bc268c9ad7cccc98233a";
	plainText = "d1dec73593c36b9750553f4275e5acf395d66f2536e27909605c25e74070aab9198b288dbb8eecb18f0b88c46d33012098cdb45554503fbd437ebab7857e00539b72e3218042d006";
	cipherText = "1a0fa3472d11702c22c1fb468c14dcc8ce3c60ff31ca20b8250536dd8adba7673ab961550c49909abf6c56f824ae0dc069daa705029cef97fed4f83dac784b70465bcf648b91a3f3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 322;
	dataLen = 576;
	combinedKey = "17ec16ac72683144fbc30a22b5be83775f50289aa041013782ad400ec8a88688";
	iv = "50e8774c65e1618f5f50f148dda2a447";
	plainText = "b7434bcb325876252c5751da25bdf9676ec784b128e83967efe1ccd01ff1d5d94f6f4642095f4ddc3755a409fbb6e31c33b6f094ee253b73aae3f3bb094dadc3dd9286fd5fbb17bd";
	cipherText = "9c3c769683b7e7ea2a51fc7851a70d38e89bb94573458e65484e5ace6ff25fff18ce1f62cf1553ef9090c93648098c4cb70a2bf898835176581e2e7ae79a7a79cbb28734c1aa24ed";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 323;
	dataLen = 576;
	combinedKey = "a174aeea7c7aee4c3a38eccbb185f50e207edea8f9b214d177765f1b92c4eba3";
	iv = "04356ea681b370e867ff4350b831e11b";
	plainText = "fa76757d36b99c9ef234fad419620238bba91460343c111aec7c182c3ae59c9964c5ee4b133a0692718f308bec97947b58a71ac4b988776153e27cb8b8aa3a372c31aaeb39bab4ab";
	cipherText = "f743b17808f9d3fda6f507099223151582e338a27f4d884a0c7d2744a9522de0731b39991782e7f1828b451c406aae1b73cba0eecfef990116be787080097f5fe11f195ceec59496";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 324;
	dataLen = 576;
	combinedKey = "274abcbf08b5d47e52d86fb7802231272c5526fd3c293262e37df5df0424b77c";
	iv = "25604c8d5414026a6e2c7d303a98d2f2";
	plainText = "eff7ffed44fa1a3b236723fd7bd1bc1eb24859781110359b19b321411315b2df8775b031539e62b71d1a5d198593b93893e036bd92bdaf50c0f1587c4fb9ebb9fc65bca0e1e9b8b3";
	cipherText = "f77223ce5cdf33b4edf3c594d1f31fc648fc7b666651d1253e74ec5b12c593a9adef408c8540be179dd4e9d9d32e0879ee09f29a27204e6a95f8267ae9aee1259f2160359215d50f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 325;
	dataLen = 576;
	combinedKey = "6d8e9f9dba77490489aea7c602cb535d74726f07c176037473023b203e901edc";
	iv = "31855890540fdd9fac974f9de05a3025";
	plainText = "1142d175dfeb759e8ddc4d01032fd923f135fc0e7f0352d610696bbb438e1bc195ee8ab7fb649784d960d35c904031efd8c12e925db006c36d03de2c21c67264af921b07a29af7b2";
	cipherText = "6236d8fa0281decde5f74075cd8e45a920d15c3150ad10fce129e753217cc1fa8be30a66916d7e5f5c5b613c56a3417bdfa01511329cc68f2fc0775f233370a1c8f4a422dbdf72f8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 326;
	dataLen = 576;
	combinedKey = "2ddeacdde13574b269137de0cc713af34275e82dbd1b3f657a9d27bc5e1e9e8f";
	iv = "9eafe2b6aed2f10ae9b08aff5bd43121";
	plainText = "3edca2fac0bef7eabd4f2b87a5e54cdf1830643b9149edee4eee1dc6c4d0fff38e5a5553c6996b6072bbcce6b6a0423fce8a95e3a889d613f34903b91414cfd215d13d806b4499e8";
	cipherText = "7d826c0c87b5ed24b213f114d4fbc97f284cdc8103caca9c33a0e37d432f32af91ded655d07656043d80aefcec4f43611896d5ca954b00595411daf03be0f28a946e9ad1ce389334";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 327;
	dataLen = 576;
	combinedKey = "e5cfa31f408a198a552055a81f7b46645dcb43a01172009cb7ea23e58d01d873";
	iv = "cd04dae79b36bd850587645005d079fb";
	plainText = "8edf7214aaa56d6ca4b95b82c42f7a309c98c7b2690b83c2d8485da839a42c02a645556d399d4bc5cc7d8bed856cc2bc3323356a408135f929dcafc3d9724af223c30f8359df35e9";
	cipherText = "cc16b3aec7ea425de420a1be3a3981169c70f0468af2c3fd2db484bb0f82e8d0caef7835d97ac2db5f8a98c9eb4ebb064e9458e81e19217dbf77343330ba04a95252da03b8ecca7d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 328;
	dataLen = 576;
	combinedKey = "99c8c7753da988b27e44efbce638769f5b63866393dcccb95c0df6ca80eaf321";
	iv = "6efe2740345ceb51669dc39c92b20387";
	plainText = "6dfdff728f2933c7984423046ee3047cabe41f5fab881dc69c684182f154129f9a6aaf1437c28c282167277404b9daf7f82e03950bc7bfcf8f74ab194b01e401033cb3663f24a29d";
	cipherText = "b18f6598ef4a95b88b2b27becab6eb157946b5b3947cc585f02b3dc87671515064dfee802e15ca2a7cfc9031af78b17250d8790dc45f84c4259e667a061b4ee5ac401a26f51d29b5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 329;
	dataLen = 576;
	combinedKey = "19daf6b36621e584cad0c9d8d82cbc09ba0ab7b87dcc503dd63469555bf338c0";
	iv = "2c61a840cd1eaba11ce08cc33d686b1a";
	plainText = "0d9df5a391bc61ba81a6999d1f5d21aa587f41415a366b4aa40ceaf93485b586658efe4ebb9a270357a58b32cfd97e29d18865e15b7abae52ecfe4b4de2460198feeec02cc9c3633";
	cipherText = "64ed99281c0aebbd6c6f5884ad9796c7278a0b2028a93fdf817e3ff30197039cbe55b4cc6bc026eed5c9450622c3bd971379083169eb6f39a74ec1f9e010736db84bd10c1d946866";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 330;
	dataLen = 576;
	combinedKey = "7eec9786c049d89c8dde9d58007fe742758f0f04fdcf1985cd07f4c17ebdcedb";
	iv = "4d67f16c9ccab19db623a6f9c9e1251e";
	plainText = "812f9952dbb520acb47ed1e211991b2923a699436ebf710f40dbba5875ccb5f96942a4a46a9ba36da8cc5d86125b7137e78e3f07c8431c480bf3898ad86056113ae3e7125ab00030";
	cipherText = "3d49d59fdac533dde6ee1d767ec011dbac466cbd6fa5a1594703685f4e29812f9b957c1631ab43c60802de5f05bb796ceca51245f3d9180b29bf5753632f10f4aa0912856138371f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 331;
	dataLen = 576;
	combinedKey = "3cd34568498f6bfb1b91cc8cf6747a39a2b04d8f3f4f731ff7210bafa1bf00cb";
	iv = "ee95da77a88f2dd206f01f447d26a2d0";
	plainText = "1660393812c7c6b5b2b2e78fb4b6d7ab86ef7bb99a516f0c01c9f50ebcac870825af48ebf97719e23cd69988a6a35922dc160c03659c56e3e59dfd39afb263f5a384357b5aaeb0cd";
	cipherText = "f0f32363f3d29200e1b8b4cb903cc3b9a7f7f7a34925e18c6c2d7cee9687b462bc0b1ee72398cb200203717be59c5bc016146b03ac99f7b6485d4cdddbe379629ca72f3f174bc8cb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 332;
	dataLen = 576;
	combinedKey = "5be9b8709c66e41198066e25a4defa260d2c199b45fd84babdc936275b8c17dc";
	iv = "f0a8fe41d270dd19548458d3b5a3dbe4";
	plainText = "066df3a98b8ea09cd7c2cbc025734c2e9a0b639905febfc350a321b1e788a81c541eaa5bda5a7ff9ab0d2737b96d894c71806eb5cd1b8b2c62401005078a4267f162bb2a41604510";
	cipherText = "415e189ad8a92069fdd3f7b53fe3fe6058ccef7e9c7e5fc99cee28d49af23de39d1c8bcbe0a0a1dcb3f9db49a746f03532f5aa9969cfedc817d4c2c7687393f9a12997bde5f2c0fa";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 333;
	dataLen = 576;
	combinedKey = "5cb89453baca650f6aa6d11ea5a441d126109be5e9c0971b8fb2781e7f053b0b";
	iv = "de93053d96b192ecb917bc497c8ef78c";
	plainText = "2927e6a29b6ae771de7169c813702d24d603f7fbff8b7898c71f8547a71dda8bb2830fb80b9bef59e6abda5231559513b444f8a7c563c596491e3391834816c8be9058e2fb120276";
	cipherText = "b0af55295f0d383d24418856897b66c400a8475f705da935c5c4b3c9949ce79ed2ee0af657afcda9de3affbf56d4aefd8b5ced0b9fe80886db02d02f16bcb7b208e49b453d12ebdb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 334;
	dataLen = 576;
	combinedKey = "b4f41291870a357a73572692747de99d40a2bd61d23218b0eecc5897f7beae72";
	iv = "a0f450b3ea356fd43bd25026e692ef91";
	plainText = "a4e934d6d18d07969b4c9c50004d405ad8a8e3430d6af306e990754954f5e48d29aec432ff0afab73a694e644db90876340bdc129e1cc18754d151fdf7351fe13385c6c349dbb9cc";
	cipherText = "b7d605705a5a6e7285120867cd88104bf8f14069d489b8b5b7ec0723ee4f4de6c2a43a591781ce366d0f5b1c92f6d37c755766cbddd5064be8bfcd4e06d65a5e455f1a4e2bf41d39";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 335;
	dataLen = 576;
	combinedKey = "fa7a0e9972e0da08f3889d145756c973a4b278bf380568020af57743cf6c5cb7";
	iv = "95b1cb65297354fcb49c372983cf29d7";
	plainText = "eff218fcd33a3f2dcf8a836c6affc95f280b28efbf22c020e285ef359c6204d761d37ee239cb7238823158e0fe94ec351de0b5a0facd9953f4e90533180fdb1f2ecdb8021bb214d1";
	cipherText = "2549a0af1c4ab82eb08029637ce0585cfed57bd0a41e53b08a69a8a52c5b77d04ab5e4a49ad5854524c010bbed1e057179a77607cc4aced014b691518d0c5f947c2297197b6dfea2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 336;
	dataLen = 576;
	combinedKey = "907afb28d0dc90edda42163c7d8767f8bac4d5179a844796ce878f655a58524b";
	iv = "73158a059ad044905fbf18df21bb8369";
	plainText = "d3aafaeac2581b14b1470e19437173adebdc7444cfa131a2acec3a86a92e51c0a43df2e65eaf36c673eb15c1b797692153c25896a57466473de01218eaa7cb9e9f57840231754309";
	cipherText = "6cbb50219e814e8e7fe60f75a456cbf09856f49cbb820689748f0ebad3d20072f92a105263bf417990ff03db97658d04c87fab42996d8e08cc4ba6dcf6a553170a0596425a0e8244";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 337;
	dataLen = 576;
	combinedKey = "6038d4827490c58c6b8cbcea6beaef3db246da7b6b112b6239aabab17048e808";
	iv = "263ba18258c9993b27fed5131e8b82ec";
	plainText = "3a91677a03499b43de2a0c1b7b195535db9752e5efd27b661fb5564c62ecc426f70a72a327fb6a78b71b533b92f2b8f69d360fd553451c10f134319d7bbc9717188de9d99a5e3982";
	cipherText = "437b156f5928927fc716fa4d31bfb5ed35f8afae7478f0dd7f068531920876b37cd141211b440030ae2bd95cf9587ad97b4558871dfd4ad6e4d58e9e3e8614a2a75d58bc59049558";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 338;
	dataLen = 576;
	combinedKey = "875308694fb0f5a8930e3bf8a84faa90da67c2f18a8f62748c1fd947b9707373";
	iv = "42b1398e747745d7944764d73ebb7dad";
	plainText = "66198f2058aafb96080e6762c19a23f2c7cb9db6e9280dd4176edbfd1189e3422a5f3ddbfe4e9bc7a062bc242b11055a11abb872f9fcdc9961b658e0e8dd2c1633d3cb7b764f13b5";
	cipherText = "a585effb12dd35707f0a41c046df48492618cfe39e1c6031d026ff5d2a127f98530ab73b53b9c0f5bc2f838f967eecd8832bc602076f5521ff01d2d50c36d63d871a31dfd83b31c6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 339;
	dataLen = 576;
	combinedKey = "dd495e3d9b52b2d9763df08dd4bb947f35b1233f211efa94ab1304fef4a2ba04";
	iv = "321ca4607dd72e8ee2449db1fbf9a472";
	plainText = "50d17ee0fec807f1e6bb6db75f095dd4d6b37189b3c6c91748884d70b7e2c15b9a17231977cfd62fd32bfb800261e3a10ab5d5f58310a833ccc17c889c25eb31d8717d2a1f0a9408";
	cipherText = "33ee1c12c376d37c92faf38f102f5bb985b2163daedba53aab6cb66ecbd09ae426cb2f858404dfc43e0d0e5a208d98d5b9484da60f01eb479c5d9d0866ecdb1829ad89a90caf4fec";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 340;
	dataLen = 576;
	combinedKey = "a248c38a378944343128c6ac7c7929669c788eb8e94deee2419cc74b4c18e9e5";
	iv = "1518aaf329f57e350ed4f27e307582ba";
	plainText = "5d0073b99911c55374ebb546636bec67877f1ab14fd365665bc09db380c8b8a29596164a1ba1e60cf984cbb28326162809ebd6cebbfe13b48b4c4fe86ba1776c588f1a086d3399c5";
	cipherText = "f79d61d04cb3069218864800d83527b5733433d8cb3a173a119d8669a3220d817d3ea213acbd57a44cd8aeb6abffa9b1116006ebbd1c1522dd7e286d83b5e8eba34be38f3f129f92";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 341;
	dataLen = 576;
	combinedKey = "d6210dabcefc57111b5adf9351ae52bd7d776ea5147dd48b8f6a420fffea7dbb";
	iv = "1f735b21d45e10f8a28d93436ec6a54a";
	plainText = "f5073c457dccb9578ed1cf6aefa247967ec01bd1a2eae06f1ec1a3f6f7a0a2c20e6424b1c4abc5dbc2218bfea054f3098fb5a1c3bd97affb7506897367dc05601d2952b507e3d2b4";
	cipherText = "420a680adb78a523f6167f57d162f749a368f2df0a49c21721abbea1f06e9d4ed7b416ff05e6fa750dfb2e8966d83e250b00fd9577965ebfeee8ae290d0d1789da9542edbe8423f7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 342;
	dataLen = 576;
	combinedKey = "7fea0972af79094d08bb682406f52d4ea1741aef229a5dd490d49638f92ea88d";
	iv = "e6ec98d0cd55ed82cca2914fc83cb896";
	plainText = "ce43194e7dd06db25041edda0ac94a11869763a2b4ab23ee75ca77c6a8819621dd4a99d2acf68e1fedd7c1de26b0298df54847b1e6c774bf333e6ca8b1e62c77c915127cd95f27e7";
	cipherText = "67cdc158dce60094deb2d087526716832528e17b8b0ef419205104fcfbb3a40bd98facef51eef5dd209f8c5a642bd2a698861144e845109e1612e06dc5f2eb751c98095f5708f761";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 343;
	dataLen = 576;
	combinedKey = "912ae3cb364917f4b79bb56e338b90cf37cded3cf248068d7f46cc92dfa09d93";
	iv = "fed073bf57ed6eb9605a8328b304628d";
	plainText = "0459d719daae4be50c83306accc1cd371b1be4f25fbca999c999f9dd350c60169114071dcf03ecaa4fe8ef6c5093fb26389f37dbcb16c079ee4d9686e5836afcedb72d6d078529dd";
	cipherText = "83724b429c769d7d261ffba47fae4af2a9129716eb3661291d69a5816947ad2cadd4e9a871525fe6e3e5687c3fafd762ee5db2ada776c6ff79065912073565b6ca171d831e08714f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 344;
	dataLen = 576;
	combinedKey = "2fac207ab12e8590d7f778d59e15f6d68b7761df6504ba5157a86c09bcdc208a";
	iv = "91730a6c9c71e0e423e5129196ab92d0";
	plainText = "b688dfbf65be34078e019243c1ba80ef148b7df247955f80faf291f7b38fc79fb0a3e07b50c1abe0c4342aee4aca9ab39f4ebc2a37782d3b3d22027c33f107896e018f08648a4ae6";
	cipherText = "9ac799ab29d9b414b76306ea885e39aa4f6f24c236a9c3d7f7bd5ff6c3bca5d76f8cf0055a31f7cc84174650f9b19437fa1c192ee3a81a1114fb2accd50c0c62c753a21f6c4ff44c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 345;
	dataLen = 576;
	combinedKey = "5f7dcc600c3e92e50630390d10a9bd984a86a9f4f095cbb09860836c2c0bb3ee";
	iv = "ff4e555e13f15a6e56fa352795caf435";
	plainText = "6574b4d5525a2c80393ea2d63e287e2f77d643a29f543e047c50b423e6ecb6e596b700ade103ae2eca1e4c52058603c6fa9a5b75268932ec3ea40214987178fa4eefbd277c6837cf";
	cipherText = "981354c645bfb2f6bc09c0e966b2cb6d4fe757abc30916029e8ef49c6197687b56a5ad5edbfe17c97bd74cc7bdb041cffb5d447f6adec45ead7d2f74d2c2381ed2733655b231fee8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 346;
	dataLen = 576;
	combinedKey = "99f6a5249a44580cdcd0fd93f3a5cb528f2e50e6cc3edec4466cc49dc5bf9774";
	iv = "e932057a4b8df99de4c5a4ea17f2447c";
	plainText = "43831c1f0542e5a0e0f003e74887c5dccb8a57fa91714461e0ccd248993bdf7c04b1cca015b10220866279d02b01da31fcd98496e5ee766c9761829679f802b886f5ec40053c97c9";
	cipherText = "4ba5695e397c6f21ee82fc30e07825f52e854fcba14c30a3a738c1e8cabd2c95c0eb3a60498943906ca8fbdc27ae5fe6cc61bb6c59e82730420769afaece377ce23a44cb24636492";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 347;
	dataLen = 576;
	combinedKey = "f44c424531dd4f5551205c9daf042f610797100f5c79fb445fa43a18733de966";
	iv = "0e9fa39fa0f68955e0e21ddf9bfc47f5";
	plainText = "18247a0a43a79d0538cb98e76bb937ba55cdd255e00c86dde009b647be622641adf1b940c36120282ebe5fd86ad2f08cf0c10b790a0aa3cfb7e2078de791f4b6ad0bc909ba8328fa";
	cipherText = "353c6c78bf4b49cb785c8b5feb817300b305a0226aaf69d0198f74b84b5646a8e3d0883f8577f36ad9c48d7c7c9d98635d5b3aff83f2235a614fd3b7e6cfee17f028359c0d7de421";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 348;
	dataLen = 576;
	combinedKey = "a906271c57c5eb705ee0ff4bb2ce99c294b59964db17190abf904a86368a9a9e";
	iv = "42f611ffdface3338b32d88469b646ae";
	plainText = "0d05188676e6f0d678bffbd54061fc5a624c52c0c49351ae94d9b7a11fae89c0d4fd2f612d44dc143a798fb0359dd566c929ca641b7a6e2b44a30c7b8ec3497c489a9f1c98555770";
	cipherText = "71a2141d23adb12129cd9477b13f164ba14844f342f6a95174421f87136c50656711ed9152d6ebe8a17acb8a45c74d84572fbb9ce7d88297cd9462f7fd8389f14d7c45eb19eaf5e1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 349;
	dataLen = 576;
	combinedKey = "83a212964411ba3183cdcb24a7411ff5812608053ac84a962dbfc41e43bcbd1d";
	iv = "c46cc2105a920199153a912045f833d8";
	plainText = "a5c5316303f45b978dbf24433690ac43cd2dcb1bd11686fb1228ad6e87610566f6f9356a944c20bf92271aaf0e743dd30152046e592c69aaa6a351d728a92071e0301100d778f589";
	cipherText = "7786a8511236ec83b300d41bae7d27b4fa211f920c1f368247ede63aaf5064726a9df413ee303851e3fe553725717150cb93ec5d07833335f7cac22ec21887a76415ba445a100cf3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 350;
	dataLen = 576;
	combinedKey = "0c07fcef97015f2b4f76ef6e0a31e2c5b68767a86856e3e5f49ff3857d1f307d";
	iv = "9d620524c8f26d03c056bd828462bb5d";
	plainText = "244edf56d42b2f718458a883c06cc929c7ac3dfeb0f9ea3175fce716d7cf32dc14b1419e46fe0be62583f4ae906b3f54972da57470c3b0aadeb44b088ee19bc08de729ef4ebfffec";
	cipherText = "b3198ae5723ec031564028e5fb2d5161c76c5a7aa17ecbcde62b81cae1ca30d328a0bdb616d7d3bee4b119f8014dedbf942d8cbe3ec41a84026e8fc8fc00a3ae79162f8355451101";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 351;
	dataLen = 576;
	combinedKey = "f6f6c720caf8dbc7c8db4b84fa1419ece419b43ead8619e546d665c6e4a938b6";
	iv = "0b9266470dadf5b8b0278b4004ea5c99";
	plainText = "57479cf5ded62f3674fb506e2b036a34ca6cffe0c4f2e72cd387ed688f63daaba8673fe273bb386bdc2a0ddc975781ef97f54fb8f368ef12620b4a7bbecba969f015db9b3754f09d";
	cipherText = "efe05ff4ab043455d91ce02f4d52b9ee73a50cb3d8ea8c18fd5e7686894d3fd950af0461a8c345479c9adcdca944960f41681e1a27d4ca579f99804fd5be29bfd8455fdce647c23e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 352;
	dataLen = 576;
	combinedKey = "6e32cbe45b84cb3ec350a79880b5ae25099fa1805d030784ee5c2419b62a2c53";
	iv = "8508938423b92d003ae9b5f32ce5a7e6";
	plainText = "872970164c96f1bef9d35d8ee0e6589c88fb19772a9e157f8b1afbd33ab532f57ce45348556036f15b4d2d74d466e30d829482abedfe4e650c0de8b7cfb2357f471510348d20f19f";
	cipherText = "20844441a0f061eda2e464037000109d0142d366a69bd663c058f82e1cf38e4bd53e7af7f35c3db3346660f860e8d7605ade6c51980f7fe03db4da07c1a4d6e6299af462dc01bda8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 353;
	dataLen = 576;
	combinedKey = "13dad2909cc95a8761da0ab02fae8abcd9248ddda9b7ad662d905a46a25bd5b7";
	iv = "d908df39185b382400e9a9ca1acec233";
	plainText = "35d0252282eb70694bc198979a6969509d598eda776c944ef1334984f2651e2356167d21cd3481c9fda691bf8cd3c244a1d5d9f300ed851f26010260f6b38e71772062a488a500bf";
	cipherText = "fc93752d32c493ef2f2aab74bd58be3efb394aa204e001752f05acdb90149f0846980793b6dd8a52c3664edaddd033065cb9b379b90e07a023a4aa79115bd4b68a720554801b9cd1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 354;
	dataLen = 576;
	combinedKey = "2f87c97ca5f085c1e001cd6757cd459df058f068cdc4156d8079370c94b4f3f3";
	iv = "909f913cdb83d5bc46f28998556c35db";
	plainText = "43980213520c5d04a247cca3e64162a897d12eda1610710f5e985d9765c2f55c4b54d494b1d10992f2e38e2cf87594b8d4b33f2374b6e1f44d42f32b87cfe5de43c0560937a1034a";
	cipherText = "90615f63c3680ae6eed5b452849a44edf363861309333ae4e070c60f4c21f85b9c0abb9e822e70c88c1851fcc97ea0c36d088832d5b72b9c6d33bbe16a290bffb904bb5d0bd933c6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 355;
	dataLen = 576;
	combinedKey = "3ffbba8114cbd27c6b96de8d008e978cb7fff41e8662506b8d47353a1befc202";
	iv = "3ae1194ba907fbeabe51619774b6285a";
	plainText = "e594df29ade27756dfceec1ee1b583d6467727d6b1bf6ee38010d217f0ed6cc8f377699648494aa89e03d3111984eec3eedbe3572440345594f982f7ea12615fac564dc35c38c662";
	cipherText = "6c8f80a74a56842b339a9f309f79f194c31cecab323c478cf8945fbf99fd1495f430fecf1413a4c18b6e227a464ba989a1c4e22b84e2e9a719045b08aa9219a153534ccf43d7fb11";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 356;
	dataLen = 576;
	combinedKey = "617db49e4b9e101c15a53fd515a57529f968a6e5ea45a66364dd133cf863af8d";
	iv = "d34ee8dd37efd7d4d8835d93000f3770";
	plainText = "6a3e8fe8bdd29e03f6ebcc27f7b2ce5681cdc5d950594ad9b9e57b5ca022c97bad40742e9bc318cd224516d54e57671bcf79d51c84c0c8d9f1d2e85cdcff2ac5c971eadd568e19a7";
	cipherText = "a290201df16c822e9cc30a78fc62a9f317b4a2637cb156eeeebb19aca1d507bfd481ea14f9baa52c61a4d369500d0b8151af05ce4f42e8f035c7075456b91029e68e1ca8daf5181f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 357;
	dataLen = 576;
	combinedKey = "f8a772bf7d480355894d8abc31bdb970e7643de3c439089eb08275f4d4c02229";
	iv = "454ccf6145a2ac08047b55425332fe76";
	plainText = "e36658a89c5fdb2b181d1bc83589a564e0918c4f74a7eee3dddbdfd665ac0ad5e187395c997a512eeebd155d886d88da6f16a9f78ade62bd9a3f1c249c0a59757c73112fcc3b6f5f";
	cipherText = "5d275f1d497643429ca905b53e1e0c365c17eef2bdf2f86c2d57a6c8828e7742cc735142c934b837e56e1e4765745b860bb2776d0e5c3b4d4ff18d25d8bb3a2ae08a927e71bd4184";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 358;
	dataLen = 576;
	combinedKey = "6f8516a1b12eea2b2a4787d4a3eb8648c141bd505cdd18b9caaad3375d328bbf";
	iv = "d9403bc1d9b9086ffc3a779faa9bec02";
	plainText = "e41bfac05daad07779af5e6f23ff02ea9d149a3739feebd25e3f8c4cecb1c672d71b7f2739737a8f8e71342e5d276b1096d1866275173de47a4dc8ae0ffc7134c3b4fb5b61975047";
	cipherText = "a51caba460854a75c163a7020962aa07050f7b560ba3a83574982a1e5e6b41cb0c7f1878fe1c40211d99b5c12d1fca747e7c5ca5bf4e5a7c9039d11c84a568db9c281eb2178b26ed";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 359;
	dataLen = 576;
	combinedKey = "6e58b37d71c3bc5b5696ac3b17d683a816400d10eb05ae680f4a8a5c7196c5df";
	iv = "2dd53f312d09635b043ac61d5e6999cf";
	plainText = "50aaceb52bab382088c1300617f99dd5b6ffa3fde380438b573087c809be8dfeca8a716eec0b91f2b9b0dc2fbc102a34522743fe526b617c4d8df36863316aa959cf3107377a5e6e";
	cipherText = "82b85f9d9a76f9d83a90b0c4f70f6bdd139acc2d5600b37120d33bb7ba04393bb8e056f57fdf94bcd007d54286bf018528b9cb543407e354d6fdd2f17f17a31adc9220eca1287953";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 360;
	dataLen = 576;
	combinedKey = "82d054d0790efb1032c674354dba7866ee7d109644e2f74d82dbfdcbe0e1d17a";
	iv = "8c143b917d0bf4a44d85d4823af1a57c";
	plainText = "c6ec809dc32a385fb3beccfada36360ce2be82675593222b0a404617f75af400960a431d531de15b0831a072545000a32cf429b40d6ad4c49e75c1b54ac4b556d63033ffa137d97a";
	cipherText = "3db9b03162072747d57fdf8589707a9b519d39aacfd934966eb8c107d8377238b9bc2e20c558d3766d8d13c42c95c7927a492ff0df8dbe2c6dacf10f96caf35c654ca2cf8c1b87a5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 361;
	dataLen = 576;
	combinedKey = "7445415b7641f755517b8e8c4c3219fe7208e038f18b039108af6190486ed5c4";
	iv = "d061a5f7fed8ff8a79cb2dceb54f6b98";
	plainText = "1003c277b8491d626930a769a4319d8ba1681f854cf5d8eafe923abfe909898a46575f6e0bcd9b6d9b79edc4cc958f2d33958d636fa6d86bd7b0ab7760aa4ffe6a5b7b8e6e6af763";
	cipherText = "17d3901c61b299aff4e998b450d5ca0cbf009d0d3f5e3f04de49c93918935a6f2e950ef695efa029eae5864b7db79ffc3eb893255e68596e84c6f4691da6e9b75dd6cc39e6270ce7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 362;
	dataLen = 576;
	combinedKey = "b395747aa40e3a8c51c021152de4afa985df7d89a2891e0b7eed6a0b5d8a0652";
	iv = "5335262905dad8cd929b9477594e6a61";
	plainText = "0a18bb3300039564c56667ad20714fb7639632478c054f93fa323023d175e9872f9de139358f08c11901396db6e714a8e54c21b0d8ccbd595f3fd202fc46ddc62f487d3b840d3d38";
	cipherText = "20ac0354cda11d301b5a0fab1c1403c0bae5d01b43a5835e5fb6b93053537194d46f20bb04ef341f6e6559ac608f6606b0f7da6361530dc40787f25a307e5a22153b56aca222f6c1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 363;
	dataLen = 576;
	combinedKey = "63ce9bbb1d7f21fe21c3cc47a0fb80518afa36c418eedb6895dc110c80207879";
	iv = "dcbb011f8761f046d540f211bbe29130";
	plainText = "b22780fd7e7e8c1a0196932e3f04fb63afc2dcc9b2232cc639b6c1bdcb5bec8bcdc151ec894d63ebd9abe23eae8ad2d711be9f679978ab53df5aa06941fe6bc640b40fb0c5adf9b6";
	cipherText = "b58e4753e4703355679cc98fcacebc9fc942b8aa89dc13fd110e4405fced91f5f783ab31d36c487f77c0eb77e0a325f0405dad78b3cd1eff5eb4e0bdad2dedcf2758f322e8bd8878";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 364;
	dataLen = 576;
	combinedKey = "9c44cb344f240535f4ced7f104887ce1f61dda3a394f518d904b03e67d4b19a7";
	iv = "da3ad743549874b1157e50d5664daabd";
	plainText = "8420b26418e0430758f70411f96a1dd4ffc82e17699f2cce1f2b217b5e7b8dd4dad92ef3bd07fecf1c2e3b5ae54442d4879ee22fd0b6eed2350b0a4324a86092cbc2302c792952f2";
	cipherText = "6eb983ddacda54dba3935c449d400bb02c6c2c2ffefb215e4cac2844449c7b4f94663541b8eff068fafd03fa52fa86d71879ccdc561fe4bca2a5cb93f372797a823840cb87362e66";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 365;
	dataLen = 576;
	combinedKey = "3efaeeea272fde2688edbf8b7da0680be550ebd9858cb963e4423ec977978f24";
	iv = "bc6a033507613fc1652e06515dd4d76d";
	plainText = "5eed0619cdcc8981c36925ada442011ce2cd1fccc0aed7fa0e1d5feab99d88aa3d722ce6811673c630732649e5292c84b970455f9676ad21138c91d840e2e10f94c43a8285b68f43";
	cipherText = "a616e4733a0117b7b4f62674b726c6c0274cdf6bf1e8c8329c1ceed6bb79671b986088d50f9ac4bea347c4a88c3ac40326ba2f7f04521f522b97d117115a3edbc9ca7077abcd3df5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 366;
	dataLen = 576;
	combinedKey = "08667f326c023bc59c10e85367cb6650952389fc26a8b3ca88c1923faf629a70";
	iv = "207de90540145ea39d7748e647f794f1";
	plainText = "5206784b4a8a2779cd26769459b30fa0fb391ce9f49f2cd43c3796e5649cf31ef334bb4b87b0d2aaeaacf55609e6c267a5548c6a2f2a94ccf4d76056b04f9e1edb2400d52783aa5e";
	cipherText = "9b5da9b42069fa4e45b6dd722b29b82db117e76f09a2699e6336990eaad66fb26f6722c3661b91c36076c79efed4ba9d6f26928f1af15002c22418f3b3966a85701ec6e5b60352e7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 367;
	dataLen = 576;
	combinedKey = "0a34ec9ef2f01ba79a479b3a01ff1c5ba33a6a753be028473f775fe25fe60f67";
	iv = "087f3ec018a069abe29c2b75f30b15ac";
	plainText = "ae6274ed75d1833810a83e0b00e28d438b633bca6d5bc68fe14f4297e2e2117a3fac69174e8437db81a127f51d80128095c46edf8471f457119c4e7d14bd07b152e79ce380b605ac";
	cipherText = "9aa592a965a2cf00736e24820b6dd1018161503369f16f86a21136b9f267fe16e951092f837e341dc7bc6c7d75218dc2174b9f0b2e2ec35abcf239d7122110125d9e0da72784d4f4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 368;
	dataLen = 576;
	combinedKey = "f37f1c85cf4ed3b85119aa3a93fc06708d124abceea15a8e595bf1afb6e1604d";
	iv = "f57bd7928ccabaa5cc34edd49aaa2232";
	plainText = "e1544c99279f885289fe049ee50d14e2295abbe961c4b4a7742aba93353bbf3e6acc2ad14e4648c22444262abc372ffabc06ea6a8da20eaa563aa36b2dc6e38d7978e3e738ec43e0";
	cipherText = "2eb6252243bc82565d1dbd2d79be5069394a68fcaa1b416fc532e54d4f5753fe0fe0ddd6475c3c672b990b16320e5c12c7e093b7c13c8b92c34f6275e643d9c17a64a934e70f83f0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 369;
	dataLen = 576;
	combinedKey = "bff67f90d1e2bc0d1ca73a2975b6475204eb1d64672e1c3446347d1a5940d0e7";
	iv = "acd6b9459ae360571c7a29ae34d46029";
	plainText = "794d26d7b63969b6648c8312363ad1097bb5ad655ed5fc29d222db43d50347441b277e7face6a0fb21d545147fd8ab9bdd2494b3fe9b3aba4451ff8bf13b0bed8287b76b8bc9c17a";
	cipherText = "9bbbffad0656db5118cd89e2d62e3e3559b0d8fec7905a016cdb2d734fe4a8fec69285bd137a0f0af2f44e609ef4f02c174d19d684bdc62021938d7e049047dd20574df44ec7ffe7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 370;
	dataLen = 576;
	combinedKey = "7d16a5a9312d23131041c7cfa0018d90771de2c19de907b75a3d11a1f3d92779";
	iv = "0cb12bbe22544f0bad9cc1783ce2d693";
	plainText = "727005a6eb107fb67c46d51940c67867f56df2460094b67a5ae5471c4e2fe1abd62de505347c67a7fe3d6103e5088456694022778d02c8967732869cd807c9c12636001c83da1f93";
	cipherText = "3c810ef5231624e0e2bf363812dc5df17aee5ec81ecaa6cd05136258a77513e574746e8425e9e2eb2da07c9f7fad741fb6ad67397db6eea78feb7c43d724d87b54a31b75c0a5aa0b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 371;
	dataLen = 576;
	combinedKey = "efc3504546ff587d12c073f846fd4238d9ed539c3484b09ba368d97726d3441a";
	iv = "3b7d83c8dda449cfa6e8e48a4ce1fe78";
	plainText = "d8fbed361e7544a3e2bb464d8edc1ef6538aa4a0e079796791fe898df3c2afd6add2c6a47c6dd32855de11069756b569c4bb442594a1e908e9d23e9cde260cf4ee0a6e8d9a28da39";
	cipherText = "943d5e3d69472b73de794279222b1341f58b83eaf045fbd3c0c4240f23cd8d96f569ce837766f4c55e0b1f85e64b08248367ee36ac449a65ae5d13a871c3e66eca4ab2c558aa728b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 372;
	dataLen = 576;
	combinedKey = "c0f7d1710a7c706302409893fccfb6998b0493b47239c275f3c1a27aa0bab0ec";
	iv = "93c08a7f9f8bac23041ae115589d2a1c";
	plainText = "d78a3eac7d873deba7de3060c20cc3d0a9210df8a13dce5c377eabcb6bb2f6094a20981c833c03e68723f395dc9c99f37e414898e3d3f4c83e4557874e76b1a2b315a614771f2726";
	cipherText = "98c5c8987f67a7cb16dd702185476a0f08b839ce447940e3a96e204f6e0f383e4e0c623692db9de5654c4fdb113a8d44f0f827a38fde1ab526ca0a199ba599376d703d375b7601b3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 373;
	dataLen = 576;
	combinedKey = "adda2c00b6a83e0d6128f53d2d48a9f4f1712415eb1240e551c52ca45ba0abbe";
	iv = "47bb421c7c9d12c72e9559b111767ad4";
	plainText = "63af523338896e5c63bdd22173ee0a00de7a1976eaf43c33c0322007ccd936a6a545effcaabf588ac9cbfa26ad943b9139908182964695d645da0917715feff341ee19bd01304dd9";
	cipherText = "17d76992b75cc93788d48e4c46ad5e33105f1f0b6d54b39f00c348cb075e968972eba5b228aa04e08674a9e5ba67caf0a9876ef41c6e386a0b26c5fcfc7f0a0ca158f20afdf6f6f2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 374;
	dataLen = 576;
	combinedKey = "3721c22766104b49ed99044f194446edf92e7b725d8b761cf515ae8997047450";
	iv = "9f39150ea3eca7fb389c49ceb8bc3d45";
	plainText = "7eb240b4471c12da350144db17570306ef2d9d44a71f230934a16c9a2369a3f44b14f3b6bbb4dbf749fc63b2ee31cc8302c53c559abe312a8a9e89a3b42259df9159c479a434664a";
	cipherText = "791065209a4366947b849263a6e57d4dddf0901f5468fecde062516e2135b2d02cf5e9f5cc5fcf82e387860b7b357fff71767f033da4300de3e9799719797c771d0e4ebb1fce6797";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 375;
	dataLen = 576;
	combinedKey = "844569bbe1d19de8f1998978fc70b67a88b86b5da3f2acbf12e23a544d553931";
	iv = "f5ed19cbb12ec19fe09737691965bed0";
	plainText = "f8c94ae0455549d1bdcf34ead7df0ff8d414b7b0e98b3f6003307a9181b0345c926e8a573740f1948e0595f8c99aee1cd26d205942d69814786941679a7e902c1ec3575ce43f092f";
	cipherText = "2d4ee55374340bd96629ad94ae83fb8cb7ff04696135df53d6ed957f790f1ab473b14cef449e50d2029c9d24691b061b19b5c1fae63c0e0c38dcc9ea1e6806cec569a2359e78247f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 376;
	dataLen = 576;
	combinedKey = "14c82786ddbb7fa43935cc4324921f46c706eeffcce49eb40837bae142134d5f";
	iv = "49ab9dcd2c9344ef616fc9b7a3d6157b";
	plainText = "e70222b3c7cc5bbdb29c5d0ee4b7a289ff3b324c48f8a3356b6199a9acad0d486ca6ce0675b0bc407edce7e3a6f5ec0650270bdd42d947ef20be56f709c718e3d0158d999bb4bd32";
	cipherText = "ddbaeb3a52c7014492b44f60d8bb83eb00dffb735c7326f471f3be9cb2b27cef6ecf7093ea73581fcba705b0d3fd6a116e42484ce950318c1907398ad57682384693e15ff54ae91e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 377;
	dataLen = 576;
	combinedKey = "05e54c90057e5b73dfc8dcd3eaf2aa809724ff6546b4bcba6e6bd21a8951cead";
	iv = "356bf30ffa5b1df2fde72b89a8d4cae8";
	plainText = "3bb4424bcb219fe445dd70a42bda228a27d61152aae747a33b024b3dbe8a962834c0353716ef4348dad87204b7fc0323c12a683adbdcbccbf5a8f7c19492ec97151c65713a6336f9";
	cipherText = "a9c859205319ffc7393a641ceefba6d481efbe8d0f7e0024da6866e6291cde4aab8476b87a666ecb90ff1d80c63a9f06791d64e3c5c94b46d33c7ff40c6e3d039159c59f510729dd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 378;
	dataLen = 576;
	combinedKey = "1191b7c0734b75132eecb5c879603a4f4fbb4ad95ac3e1dbded1753e64d38962";
	iv = "186ee4e62221d225936dd3f5ecf4a84c";
	plainText = "c4c5d061a241657e9611276773c734224594a7475ef0d8658ad0514aa91e4443eedc1ff5d6be35c17862c5cb1e835c9af6d36a937e05c5bd9accaa5b1fb1cd3fdf8db46a4b446e91";
	cipherText = "066d258b65a5f51551b0504fd51c8f9cb09761ac4315b9bbfed43d6d10ce0281874be429b6c2afe14affaa002363f09189fada13eb7a1b5b5cc731e3b2f17f7355adcb3903aadef8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 379;
	dataLen = 576;
	combinedKey = "a612abc25067207ef7751252e392bc74fb9ab76f7b3f8442e3788409984a7e0e";
	iv = "fb734489601c732ee2f27d3c24f61e85";
	plainText = "8660e147324b71637d439ee84383f30890ccc3660dafe27ebea81a6726d48f214c668fa64f2743ca98d866fe6486b6a4eb72c9e28905b33a690c786f0d6ef6d9991fa7cadb593bf0";
	cipherText = "a87dc6f20ad1c57f58f362f5b1b3e3955e7700d27eabdf3ada810482fb55c6786cb41da71aa290862b2b4473120bbf67d1a070a199f2a059e3023feeb6f23de6cfb1d25d8ec08b62";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 380;
	dataLen = 576;
	combinedKey = "4b8265a707011ef558bba24b721443ca150a7023ee9db9c10e79bd9b90bae527";
	iv = "56f16bb691d2b24cdf6153d6377c2120";
	plainText = "b350dffeb39740b94ce5389845026795e1d98aac6a80cb0409ac97e96908becd4fd6573bb4bedc20fbe13c6671264081ac12e593d105cf4d74b194b0e9768a6c2f6b1e9da890cf1e";
	cipherText = "e4c468f0c846f2e4cbc62240771fe11a1899705862da4cf63d92e978a1b1f24e20c39f86f5861425f370a91be53880d78107f081478156b5b0dc4369342a33d0e00e9911a87443f4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 381;
	dataLen = 576;
	combinedKey = "f84ccdb7e9c9133d81d67ac1b6b5890162c030731478c2eb616a9f4af2592677";
	iv = "2c8759fd09859eb40fa992d9b79fa738";
	plainText = "e2aa126e469ece99e32831c02e7ce1b72b2b29388e0fdd9e849476affc6f35c5879f482458015a1537100ba24e01ce05e8ba0214997de2c8065d8f74c074b4728ae6186d7f33ce94";
	cipherText = "d4c0ed5e4efcf09e1628bd0b424e93a3ca3325e6835e6b541abe62bcfb7fa8e18f56ee40fc357d4d0493d876b10d79401fc6b2cb9cff4f2fcb8c6b94f7dd3f2708b9e925ead02ac3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 382;
	dataLen = 576;
	combinedKey = "8cbeb2d49426d05ef87922e37a4fc194c6cc214f0c3e0c5a9f09cfd626870c39";
	iv = "a9f45551d8ee931c3d599ab655283a54";
	plainText = "294d4726898ef808c1e650b034780a5706409068e742b8821055c431e21a51e1af82e97452750eae3fd6012fa6093fa4ed97ed910670cba64acdb25482e2d6c4ad0e22ef08458d11";
	cipherText = "cb5e46796b8ca9860249c539e63d3feca7868d6b3619b5e6424474c570bf95e1dee28dff6522b0d57a8ec024b5e43b27b3f0c30a15e8536917602345bafe57d070d9d7ed2386a352";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 383;
	dataLen = 576;
	combinedKey = "541aafb767d69ec7c557cc2067604c2a26dbc2d731bcaea1c6e63cfea5c6a4ea";
	iv = "2d27d9bb91e75d54bcea23d9dba9998d";
	plainText = "ac08278b5027b80e1c38300b571757e5d23c2a6746fe2c2450a2a5087095ae2ed97982ed1593bc2eeac339a3e530654595742c2fa1bd294c6f3621f99344381fbc56a69ba5ce469a";
	cipherText = "c98d6d7fbcc46bce7d98e83b732d96ab4f300b967ea6337738e941e578259d251453fc422c695dd3662a8332be55b82dba714093476df1a09c53a9a575ac4ed3e08f02216a7a63a4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 384;
	dataLen = 576;
	combinedKey = "4e3c7b5b0dc41af45a24cd48ef9c0d09e882066a481fadb96797f15fa83936ce";
	iv = "951b0f0112f64b3838b956a8d897cef1";
	plainText = "f7ea9ea14e7fee21b8868187934618dbcd1f3127ef2903f56c3bda9bda21c898162335121709e16f548905553123fb27d6cc995691647de80ec212d2cd3b7050a9da050403124565";
	cipherText = "b20139d0327a83072bff32bd0fb56c43409d20e3da11a343073155f18af26394f6110ecd7378712c022eaf55418d3945dc836ad1e72eb275b039b43fc4ab37f6a950c7029846d4d3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 385;
	dataLen = 576;
	combinedKey = "f7e51bc8aebda4a2415c5e75c986852642f591bfb88a07d065980cdd057245b4";
	iv = "72f52e2b5f1cb498538a955751aa5639";
	plainText = "dadbafbe7808baf838dc71cd75e400ca83365e713571bffeb8ae98d5439e0e9c0558e372391564822d607ea326a57caf0e7f61557ab02a799d48211aef661208fc386e3bb3ef5dc7";
	cipherText = "d63b32a6c8b3bf21cedc1b05f5766070a55a6ffe5d0bf19aa176a54afb9979574b242e4171a7abb8042279a35ce02c40d66696ca674f62ac0c2f52dd475883c82e9a03ff2a85c5b9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 386;
	dataLen = 576;
	combinedKey = "5e59d3739d8adda588a7b72139f52af68d514b5f5448f6caef298aeb97fe149c";
	iv = "5ff0597bbe36e07cb3977e597c40bb7d";
	plainText = "aaf549eca5869e97c9b5bd9b7b90cfcdfe4bedef59c9bdd8ea642bf0e56a8827b094fb103b68395536de804e67289b331ca41164c27859d77c3085c2be091c3465ba246a1abcef6f";
	cipherText = "2b81b97772131272910bde7cad82bc941e51dacba12c8184c2e28a4c89f29ae796a0d865b893307ec4f28bc6c4c6da83fb5be2db989525338c5d40afa02e20ef3f7a8ce4be88b0f0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 387;
	dataLen = 576;
	combinedKey = "78d00a685263cdf0cf2802ae1447a70a133eb8405bf92b6f9b79e0a31cb156cb";
	iv = "ba64385edb04b94b0e2a11aa351def42";
	plainText = "a33ecfa6f5efb8a2ed9b24c38a0d4d6c24b93f603400a940a082ce714263e3d531fa52df309ccf0320e16bab4ec812870699d4cef4d1156de3908ee68866ca9e4b7958668e3109fc";
	cipherText = "2023a6df9780a820d1faabe47b7d3f3fbe12888bbf16a630acb4d557a5817505ef47cb9ab8d072bc33cacc7fc8485a4a81c6a86b08dcee8acf96c3c0f18b9d586c2936415145ef4e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 388;
	dataLen = 576;
	combinedKey = "0cbd07f83f298a6e3bc5187693849b5f6cbec3c5993543781d4ec0c9a895af1d";
	iv = "90819e9c84f003397aaaa7dc73313c3d";
	plainText = "736b39ce3962aa77dce103207ad59562e3a50279c70a0753d2939fad61b055f28c168c560acf109c04d2def5d90c3baa2ba9c3e844b3cb238f61aa5c01c71dbcbb80313ea87d52e3";
	cipherText = "652e19e7ddffd6dde486475c7bbfaf087850f2b0dbfdad905a035546c53f5fae8ae81eb0f777c1ad172e9786a28c55d4f55df2359fd7498a59197d1fd1d4f40370abd7b0f1aec68a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 389;
	dataLen = 576;
	combinedKey = "b61ad016541230855f9a17ff17a5cbc1ad65de760ff35c6355ffc34580406c75";
	iv = "66e620b8cbf81a80adac4d9d936308d7";
	plainText = "53a9afab3bd2dfa8dfc746d83bb4babc74d8c436d3268c656ef5e388c404139a31139a481467762f95329c6a6b2637676ecfbd50e604e3f480da6deb8c833c6e8344a6fade4ac637";
	cipherText = "2065be7ecfc4276e5f3c39e2ba65e4fb96b83b89c5bf4b2212fc1439a4dfe99c4b7d4a5c96704e080ecf0ad557716df781d94e878968dbda222a05f49d8faaa2b96be3bc10613fdf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 390;
	dataLen = 576;
	combinedKey = "09bbd53df772b094cdbf7d4825f00944dbdb19d15160f56ef32a28ca98e944bb";
	iv = "5a0aa763599245de7b8f3a79d533cf68";
	plainText = "122c4b3dee78abbd4025a860c8d822fc4fe18fa04aa29978fad1b4fd37605ee96295bbb5591d7e4233e06c51381a2eaf3d3d2caf4c3e7f04391fc471f78f746929997f05c5444498";
	cipherText = "3a883702d6f49917e81c22d041faef21a81e3056adafae768b3fd8427445ac6e8a00a1c784438bbafe5efc18130389772985f988db2e2af3f104f236c54a6b39e9899f87aebee7db";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 391;
	dataLen = 576;
	combinedKey = "f7fdd82388136a05f8f202dc908f3ec0311a5c5bce344c0515c7d105128f657a";
	iv = "abc5f1f1d3ff0e6f6f8de9c69028b340";
	plainText = "73be0cb974f597a6734e2b2083095b9cf26a18dfce2d48423906fac326d0ae866321cfb384ab491eda60cc5994b7df300a13620095eab6158396705909af234f11fb8b78c07af5d7";
	cipherText = "3700e3ab07faf1fe500a8fd396ea58a57338e02da4ff3aa2c36ad24dcfc5106d1ac7a4c80541b99339b9cb5cb791238ba9ea211a5ecdcd1087e182dabed66dcb5f3781954433586b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 392;
	dataLen = 576;
	combinedKey = "d9529ed7194842e14c2d207248147024db0551fdad6f15b003da3d36c429f1b2";
	iv = "80daacd7c9714fe28729bdc2f9cb7fe3";
	plainText = "5548304822e9f02435a2e88fac781fffc321b68004029cdc36d17786bb4978523bd0c73b4c84d8e2d12150139ac2d37f3536d27b783d7aa6e6a4554c40899be7b9e645f6819c0146";
	cipherText = "af6ceddd8a97f04a4729444ae5a06d2b5d2cd07ebd099b2ce369e0ea4abd7abe61e975e3465bae63084591c0a506037995e6d0ac687219a024e17ee9557eef0f278d2d1a6e587608";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 393;
	dataLen = 576;
	combinedKey = "8568dba21b5e96e0d87d52f4cfa202cdca87742943cbdbd6b5624e6c98a1457c";
	iv = "bf5435b813f2d925940e78bd385a5ee1";
	plainText = "668d41cad72a73a8bb58a1feb9d64f2085278f4ad814e6cb1c42b36b0fe9b3b6f89014870b557c1f5baa0e83881d8cf9bfdbaab3ca85373f16175430bf1ba14bf45c63411c60bd77";
	cipherText = "c4fe86386ae2d1e032ec4a0ede5421d0c85c6ccce6f006f1a88275a03a9a54109535568b6753a481301c533060993a77b62b9fcc54eca66495ca0008efa0ce81e24504afa82680cb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 394;
	dataLen = 576;
	combinedKey = "e198fd1acec6315f46ded41b52b7f230c3c3a77c149e6ec76ff343d36df93969";
	iv = "705e7802b1422c13ac3c038c4c4a656c";
	plainText = "2d130d9d3042fbfab5f565fa2a14b0e9ebda8d78a76500d97b2810f06976e90cd580cf61cd3525044aab93fdb43faf6013b889ee5262a6227a21ad67bd2c2f5a8936996830b61ad7";
	cipherText = "9c1e21a6e0ae8892978501019ef6b046b0e23fc16ae28690071db61c9af53359262a60c8e7ef0ad6371d6cbc96c8f942c207ffc10a7a04116753b8e59b3737612ab1212ccf564684";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 395;
	dataLen = 576;
	combinedKey = "589a8bde9acab1835077e3ffa32ea66494cdc7d21b8f06d4831da7f01e8bdf2f";
	iv = "572ac321197ea7a7924901e7de04f749";
	plainText = "73cbaddab96fd1ca9c7638662eb584ee72cf90ee30efae9f3d5566a6cf1658e6fe36e09fd9281c0544bb3b838fa2ccd3722e498e1ed1a37dedec69235d6eee64e6f804accecf66ab";
	cipherText = "20d0e366adeccff61bb2e541c0edab1bc80f9941a3d80a1e4948499eb01dd65d125a389299515a08f6051a826a12cf23fe8124886de5826b9d7defca91e9fa7600c67c6b8aed5c84";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 396;
	dataLen = 576;
	combinedKey = "18199900b93aaea8acfea41237ae0882093b39826fc500596e1cc0be1862b951";
	iv = "c8ff29287aae899b3889c40295a56d85";
	plainText = "4739cdc7c52546f968a8d485fcb528d51fd960c68ab09755ad1ac182d34e4ba90f6fff918e9512d10edcf3c6c5d1e023cca92c4bf5537cea0bb81f6400f34ea8f4eb24d1a5afad01";
	cipherText = "097411198b813f20c05809c734315d0aac720efbbeac4e392cb30acdaebaeb2b3e1a5976aed844b64d64da533c8e59a06da03949d90b6e784e8443a84d7f60af2eb2f3c088a7c793";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 397;
	dataLen = 576;
	combinedKey = "12955770a3dc6accac971c3e65e29c0558ad1703c5d4909338548fddbca49082";
	iv = "205c2f6ebf6c5bf186c59b5598e7202e";
	plainText = "012c8b3c72b5a6555ebb0a077d2475eeeabe35de72ee12e26f2c6e7146c67f5e19a59e7e7f3187a2922d654115b24425f7d4bbedbf35f1c7fb961692ac4de3fa3907f7a7d11a0028";
	cipherText = "daa492a9e52005517dfadde26f950433b6e5fa75e447023b77c7d1e537b48a072852df5daecd17acf0f03e7435e390641037f64c1004a421588786ce273ede267b9ad36b2878a16a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 398;
	dataLen = 576;
	combinedKey = "09034c6c1eac1ec34a3b27a87ff37d93abb34a0efd8a1f9e790608b41f8b7688";
	iv = "d7a028bd9523170c43f643454e056ac1";
	plainText = "1f997a2374d4f46ff53dceb86cbf38f0c9a9c0211e11768cbc0f973889f95c058a175a5e282129979aaa6c8fc05a5a035b501c05d3eb3f8af26827d666774e03ac5d1dd53a52215b";
	cipherText = "cff26b564cb7e5a66a9184b5e2d9480a57b602b6624b456fd1ec35197b039673799a944a263c92b6a58dbb8a3fc2436e18bb64901c8dea5dd7969e3d0d01d70c3b852b3d30cffd93";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 399;
	dataLen = 576;
	combinedKey = "ef8c15817473b4444bfc80bf962aa713f7e79f632e562ae3ee75b1807b3362b1";
	iv = "b7b684ba31e19b08ab2a6c82e5ea48fc";
	plainText = "45a25512893aae57f6bc8b7490ecd79ee2c1ebb9e9c28826194a508901d157de72e898fbeb0684d187c518840ce5d30bd167eedba0d9edbf7a5160edac9e931907273e12a289e3f9";
	cipherText = "399436cf867c714d9228300fe55e5cbb2dc4f18c8efc8d3b9d250c58d99cdeb739e2a1813eddce6066319d1d343332354b9bdd2dcd2a7c74233ce33e8a61ad31607f7915bb21fe13";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 400;
	dataLen = 576;
	combinedKey = "aa95e7e5cd36dded52a9b41377efebf35449c7e61c22eced52bf2f2bfc8d22f0";
	iv = "48c156dfd368f364d59240213ad1953a";
	plainText = "4fa8f5da57e80c3db271b923471cfb197dc9637910a4d16e2476ade359d5fefeef88c7be1a1b2a643968f27e354d33336bdf22cb88bf0924b151758dfe5efac9e82e2229e719a306";
	cipherText = "a731ba5282cea5d4ea19897cd3dba298cb60c144ff044e35727edba82d2404859a13ff4ed0619c7ae64ff0c16029a0da43946b9da69c384cf917e6c62ce4b575d62c91581fef83f9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 401;
	dataLen = 4096;
	combinedKey = "7743b833214e7b457a746b660774664c544d02c1125899efed80840197f88766";
	iv = "376ab22c89e4054351c664f625c4323f";
	plainText = "56ad02dbe33df5879e09f3a1a2bbc24f4c3cfa196e1a15f1ce87723f201fa3093f2d7340297f07c6bd8eac93c29a52563ff5c033b567404fd11701aff69fb597f46b4ef88f42f921319e64add4cedd83f6cc37514cf0df7fc2bea708f2d61680deb66a7962e8cb236e4933413eedb97015a7b7756e3da6c7fd2552e7fc766111d98080833a8f24f2981eee5fcd7c3b951794eba84cf2d339363373442efcde4431c1a06706c2f284de755b6ab973fd94082783b41f06c214f59364eab2719e8bfef8965527ca822dad52a9915fac45791bd207b49e6d185039ddbe199ef25fb5ef2c3a94ed345db752526801b564c1d4d638b9c2520c154e40e5fb9c86e57c15be89c9829a1d991252462c115716469cffe0a46e779c3dfaffc251a736430996185caa214423a54607d5d15f1b5cec61cfe364f355d910b5fdbba15dee3b771eb8d2ee3ef935b7d2e28003847a351826056254226dfe11b0795c25efd6e7228b45bec588e02bad9ab2c11201b3a02d5b9694ac0d07fed42c4a919f54e1b8df6e2df00b6a05ed23802fb6b25a5bd505a58152368d09304036801ce7eb86e975cba3e595a9c375847559e9b754f2195cfc597b58b28f6aa7d421e65f9c3290ecf3746a2e7746e04994196c611226fcaab252ca38600d2ef09f1a3911203fd8c3e1d6f4ab0afdafde8001da189147ff802ab603862ffe21a01f6c63eee6bf89f722";
	cipherText = "7c61c579914ed32161d277b6040ef2d53512322d8ed57fefb5437b9ff7d477f1231ee9ecce52add1eea02edf735d77234961ca2ae25422006111d7704c9518529cded3af7555fb24ffc1868fc1c8b17a1d55f1dedaf8ad89b555d9179c39d49158a2ac622b86f05fd8aa4ec06c574ae624e45e1258afe41de70c4845dba86b7ef45d693dc52557364784ca492cc6830e4c1d70dd4993bb87e8073ef048e7fcc103a6e465b24f23991000159e6570e82c916a3e935399b75e265a15d52f5824ef6595cb0c785d7b8f9f24ac45d9fbc82b5b75476e57bc2b7453aaf30bac5d083906a45d4d4e3d82099a6b014b12fc06e9e13db5cda66547eb4ef3d74ff1b9c05f73d6c94397636c11f438925dfc6375fd95c85bcb4e840bef9c89bc0948f2a055a542c67226c013331a33b559f8779cc6e7a4bb2e750918b7ae4e9798d45ed2ff8a223984e7baa52c58ffa042f2b5f1dec4466a2071f2c5ac013d4155d48edd4e18f68c06429fee5dec9d978a9a6a6955478d8fc05f8b9aa8962a63f33a90dc152af27168242bdc012863ddfd77515f9296f17c9ba81183668c052e7d54411da221894804df05a7c269c6915da8c8f0b3660df5bca23d3b3466ee4befb52b203cc7b8ff755dc76ddaa644732a3996c3caa7c7ba1ac358112219f4e9bc79c8e1719e0a0cd4a1fdc8b52a08d6b8c88b54bc5632581ceeac12d01d187e595f226c26";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 402;
	dataLen = 4096;
	combinedKey = "18e933f7f09c257a4f44c7019bbf8f1cee85a9c544c5ba3c0e89d530b9d63d1c";
	iv = "db59b8a85d388ed5391b3889afcac4ba";
	plainText = "15c2fa930ed46f0567d1d1841fe039cb5ea84c4d62bbdf33d3d698e802c88eb38a1521e12e744bd651b64546393d81bd2d20caa1666bd4998d0144c60477ff0e48a573fea340eac5514d6b02763d2f7c956a0f3f7617ce64fd180adf844d541a293604534785a892f95dd637e0a8f07fb51c5b0400f4ef230a4675e6af9126ec71aadbaabee71b822d71aa542335c3ef2b578d8e28b4b400b91a0b9e16753ca0fb647e76ad30413545b7bf89158427c8fcf0322010dc243aa3e2ed819a73f9eb1c9d0d744b46d62171de3bcaec8e0b48d29f09789abb3ed4f7bbcf1aa88d955cf8d1e55dbe4d0d0875f82b1f8df924bcb86fcca9d09d9e4a977d3c0d3753a93380d4fa0f842b0715957344f9eef0097250c23007c506aba3bc35e20dd7b94d213d359d27913c153f242171125d6ba80e214ff9b898e62407b5cbdd5878b4135acdc80fb8c829301e9701f8d71577a1f5cc5e7a9cfc55513f951137a1cb50d9e64462180752a3d36f26fbf837500ce315994ee324d1feb4d10473b6bef2acb5890c879c2a4ffe03f448b79cee113eb9411511cd77cb3d8ea6c50aaf6f5405cc56cf954aaae97afb34c13a1376b12efc63b7342db30eae18ff7507c662e4dc3ed1d1ac58941c553efe5339a19f77e6102eb2cb47ae08b48b66e4f611dc26a19c8ab9318d70a5d56fc9358ff4584ef607fadf372f7f3b0faa183700912021d83c49";
	cipherText = "bfb256833050f5e5eb3a730c1db348bdc73bf0c64f9b42cc4ba1279ed5d7bf980d34b575115e6fc933d1e10d27dd41003e57a345fe83b251c289abf3ce7203685b9c6db830466d0ff35f4387ad1a29277d03895a243181140f7fbd0a3d699dae76698f524cad8465ffc465d74a3aa27ee9b80d7b31616b15f4074f2ea5e002a40023a0ace49e8d02849cf3e93e0a1f120b5bc95459c5727afe8cbbe85b39ba53e9e1a3a46e7c127339abe4bf009ae24e8d03574542ea7fd917d26569827cb5d7ddaaf1c4e4a080843f43e229c60a9f0d405387cb9033ec43ab497a67ae7a1d6c6037353987984fe873a9d36e480e4bd8b6308b1e684cb08f41346f66e2a9ecff9fa1955a6fe005a3d954fd8fac71b44c1535cea00a953eccacfa998b356e593ef86459bcb3cbc24be4ba1e3b67def54ec0828c40636a56631312921c66d175554d737b56514173565b6e898be785ca593efc7211c0f35c7ec6bd3d41fe4f2e11964073e4cd9444a33056a16b57c0381e860488bcec1c4dc45b8f6022382b73bfe4ef0f62b4ce1b6b009b010bee2d9d7b2061049b967262c1fb87d02700a02d20fd080ce10768bf60e6b3f16facb3016a028badb061318f7c6867007fcba8be258a91a831838a44bf15a0e563caf13bd8f172fae12f1f2e33f0656302953b11212a0c52345ec033b754791bd016d1536b1040e099f6f43cbaf19023169b5439ed";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 403;
	dataLen = 4096;
	combinedKey = "693ce078657b15c4f851621770d72a7c1522d43e8e23faf7566d33008a937796";
	iv = "91728fca2dcbdd83200ec1c601855ba8";
	plainText = "6735b5cf29c39aa6820d254ac3c5f569a616f55146dc8ab8b1620d358139f4f4b9b6cb102abda523b4031ccd06bbd34a7284744d5f01b91a2acf51c030e30e94a1ec17032f3a9a8d6fd617d3f5ff3a969482fa0f63a2b24f2b5729303f92d56f5f102a0c34cff94e5ada45ff71648401dabecd579e523b1955c8b66ec3531720d26cce7db542ddfeac958f1c66785c208c31393bcf9dbb6f544909f2a7e88393c77b1850c046dcb63e7e87b2c9fcd8a9c0c607971e8067f54483339a4f71e99439bf121df7ee9299a42831c804a58219666db27d6b4bcb690d734168f20d4683647dd0dbaf53ae0e0ece337f816d589b847f08362ff9adb1fd25a6e4c77349ad15c575da02a350137b9007bd5a51e57579bd42acbf77714b92ac28c0308bf591591afd5ec32640e89e094c46a8ce7a375549986676f01d2592d2bdae4433bfd192fb46a4708e9b6220409443bfbe1eb847398dd37667a70aa3b5c50157f0d768d7cd146fa3ec98b1939817e352590897504f0b3d7294389a979385b99bbdf12a51a174992707742f27c48797fd4e964e3cb15f0259c663c62e50dbcf1c8ff63508730b9584ed9329348af8e1d16c910591adea82a66731de3b3dcaac4f8682098acd4e93fa9cee034214c10fbf8e9a7b9cc3bbdef9419990601fc3c22a84789bed03665935c3a24d95190b6266e5d1fdb0e6e7e95bc13305e4b04b0c677055b6";
	cipherText = "b3f70efb69128dddd467fd9852eaf06e743f469b49d35f1d6903a965d6923866bcc168955a7afc1ff771fcce894b127211e0e969aae68388731b8550630836c9137ba17c83a4e9f57983b3699db5e79551e254f8fe3ebe9691fd9009174e251c2711cae4b3f720ac9c9770e162334454899d0f945664e45e07423a2ad4dc634e5b5e3ae5e32172b9fed308004fd595174161565ab04a3a40abf5e4855bc02e08513312e454346f26d1a72b60ab0d16fbfb6482693ba2dc8d999e283981e12feaa575e62c9c6de045066a34016d4a02172974a50b25737b7e1ab5bfcdf8bcdd141b88c3ae154103c10404f70c4bd4cc5b0d2a734cd4f7e9f869e6e84da402ec2d1e174cc01733b7ba911c4a1b20928a64d53ac2e8cd238a7035f242bb35a2f6f287c343605ada2ee59c08720cb44f648c62a2ab94ffd23d9b8be64922ca75ea50d689f04211fedb1af04e2617cbf4810050831ffdeab86cbef8b75a30d60309887aca1260ee6a574933c6edd2fac40b9b2b853c25215b09c7db930e4437835fb1a05353d9d24bfaf20834037845ea10c3c8c4df6e77979ee035e980033325d00cd789800abaae0a2ba068c0f7e3720c9668943506689a26029eb68b294ee8af3216fdef8b3b2b458919ada348556a6cb3ff1d75841e2253cab82766046cbdfbe4e027a5ceca2908219d06868fbe4e159bb3803636e6c6f3320e0ec602c2f3ea66";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 404;
	dataLen = 4096;
	combinedKey = "c1ce5d459fd76290805cb3613851ab6b2a16369551c30e61df53f6097362f65d";
	iv = "a3c254e35e423ce67dbf81c96fc931d0";
	plainText = "9f373591fa3041663731f7e2641667f08d73ee5a8719b498a976bd986c319481422f0f789dbd691f3bb918159cf4093442d8f4f83bf8bd4542aa484d05ca0aef87497a26dcb6d127de21057763aca8f3bc652e841b9c89dffb0b7718c2e9c0730e870875ae78daac577c44f851f75c3481fdb6e0ec8bf09873b4fd1d1388428aaba8a82e9a80d13e13188f2228a20023d836d43fda0120bd207002f54e3630b18ccdcf1962a3b0eb404452e019058970c58eacdf41eae7a23afe4efd6373a97481bc11323e2af15d0d66959fcbbca962eb204614c569736d05cd02183ad652061f9c5915d26c67e4b4447d757d1ac89fba5ff47f34afce6e57c058a9284604edaaf43486d1149d4d081de2d4a46210f5793b8cf78880c3100edc08986a128109eaa9a75ec13dc6d43aeef452cc41fa0b2c796fe358c9bd80625ca1f178dbf3dd0a2a7c22491fff99dbe32ae2033fe808e70394ad730faf4e15c7b67e6808842465dd07e9410dbccde2ae5e16acbb7fa5d257332d3aafa6034b3edca2cc407b988fe8445347a09325b77a8c406ad84da8df4878cb384551cee359b45241236a9c8e4653e89eafe309ddba098202f78965493c1abfb9d2fece5a0281f6fa3ac0ea172b356f6c0522805ea9215d717b3cb3a41ef538b5dcd54456ded4c0add8dd4aa3a3064a46ec741bae265fe1a87486f7e0e4d7ce7a3e38abf6e965b536c5aca8";
	cipherText = "ca0ae211e0dee556c4477517436c8b229eb1abbea5e60070781bcc667f3ba91dfb57b58a54947c05774b3c7bf08c3e5ac45781f84984ab258a22cdbee78e06281364a5eb76f582a17c1d6174a5d1509bed042d3c8d5d98e88342398e34ccb6b0c8e9fdfbcb62f944645f10c89d6d4e21ab8c3e8760346b1fc2c082d20accf2f58c1321054043aef1b354eb7623c1f6dcd5c99c3e9b59cb41bad4727d5551d555cbf33b49395e7793a5b8fbc388d736b8bdf768c22e1df156ee5f1c6edb4137e84858d40f02deb7faf442f333375139b9e765e33966104fa81f2640796195e3db4d4eac31d51ce8c6f610409c49d4eebba2630f03e5b6e21834dfe34d1eb01887b9847d0d9738458964dfe02006c73cf5a6787cb246044b4128a8a143f013a2f389f93b0a0bc2765a2c0989f7ed05ea417786c6b933062a47bf7f1e612dde5550295519f548903b42dddb4d2d82e3b24a1878a361c2c49a2066b666d900192a297fdb239658e8dd618fd965c9abac270f823c7f0273def95b08bb8277b9117706fe2dc3fb54f4458bba754a3571ad7163fd8f26d5f18b75bcbb83fa803704a38f7d05961ed39345e52c0565ffd794689ddd72d198867b402462b28c394ed56f5ff012d2ae7265980d5d1b99f0cf390ec28364ca38b3d5ddc0d6b3baaf91d1b73aa6f2636df1ef08a4bc748b929ee902a43298adc34920a7605ea94b5b63c005cb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 405;
	dataLen = 4096;
	combinedKey = "d41b86f8ad9dbf726dff2b615b7c8dc6f921874c60dc6a3561b1d4c731ec45ae";
	iv = "c9693e01516be89749f4232be88720ad";
	plainText = "4a5ec7859aae98fa96f27830146937b05c2e5c98f3bb4e48fa4dcab65d141d5aac851b3f8b05dee295545708036b478eae1cae4c20d926b7fdc094d90fbced37ba82bb0ae46a030838ce6c4db1974b37c5ee04d92c03a3b352ff091af4144153e33e34eba5df30fe859860a160448f25d508ad6d9d67dc296e4854a9a5aba533def4043c9e08f0f662ed0245e67dbc28be3c23902143eba45202ad5b4d0737b9f6f7afd8419668ab5bbfdd6005596ee03e8a8daa0f7aed41d7849fbf5b65c656967d15de0f94b3f5550f0a6dceff95d1ec5d8e9a0dd7e1093eb37d9f2781dd23783b667171002c81fc3b9a4f35245ec63fdf866ba1e99d54a2d6c8a0807954efd1030de4c5139ba28ac6ac3833f58fca51829d752f72f770245dff047aa23fe5e6432452bfb46b972c7f39d9f1734355f0e89c6e0b553fd317d15cce43c6de6ee93aa455af43e8056e44b87359632f7ca155d41a4d03b2f391092ef67cfb1dafe70831936dcd6deb01930a646a8c01f422373c8ea0ba0a66bf1c3f633578d2b3f452a6bda2fcc8addbffa4d1b55ae37b9482370f7ed7ab9a179c21585db3db4ef1ef9d085a08d64be24539647390a5a1e46432eb10e483104166e377392baac5df07cb54e0c1dbf4f574cea57830c528697dda399d653e9a716b51a17fb901b4ede0b51d8f28c22cb9a37d710d98ed4bb3977973f8adae917bcceeaca12191cc";
	cipherText = "a3f836559dc6fa4525e0d3816c51d00dcb8816d6306742c9411e0dcb5aec59572bc50088c40730b0573e158a69c684cad66816f836cccfc292b1a73b3c56241028087f311faea07e462d89ffa38c89d31f7e5885f95a52b6db67c5f213dad38f452228b1047b47c7d6f210e44798bb2aef1de93e8a881eba019ac54545a38f93780683a704d415933ea8644dd2b7186b13257510f6f88ecb5d29a058f498f8d649f8c2b271f7c16ac3a62ee6d325fe7bb9349f59beb6bb3a4867d1e6a501ef77e75194d13c60c228b64fc2d272fa7da049a354b14451b3930758cef8214acc6e86fe2592cc94786ccd62c72608f04ce96dbc0b7027f887147c92346e055e2f7c9f3f581206bb36a80b40564e1d3f1229f3d1c90e14f49dabcfda6a79c59704cd218ecdcfe4e293a5995a1a93c8ad75d31cdb99e3551ad82a6f4d2e36c9b08ef20bb63e9710f67171e81c24a65f085be59a4b8aa6928896599a2904ba759113b60c1e41cf2cc992c0bd9fbdfc20e658fb4ff9ea35226c54ccb8055ab6f01cc963eede127ab8e5161652c20f5ee83accac4f1f574c9aaeb5d14a7a1b68c51668d9f9274726b41b1178d3c75c5d43dd9b0a5d7da7afb650a63b18aa972602f3092d42003f2f3a25ea5f07083326e1ad3d525b5a296d4c18f115d9bb380306923e71d7f8f16e7c0a75b553b79147f6ca25d60a152ebf18ee8d25489953e65b5f6114";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 406;
	dataLen = 4096;
	combinedKey = "b754261a58c8724054dae33eea0826185c3ca5bbf8afadd75cf1cd96fd00422b";
	iv = "d86e26d2526c172cfc4da0712f0408e4";
	plainText = "ac8703859310b30af8553dcc8ca4d78f81b0eda1f9eb1e59d6a68035ca64964b4eee2f3191e9a73993994baf958c018edeb190ff7e4cedc49afa711873212141f85d469ffdfabc1ee8ced2b975086b2270445be7dd1b92d33e5b3a820a96ae0b3d6f85e93e61e99f6edd4170c331e254fec71606cd1ce523332dc9350f882c49654122b3faa7bedfb9011732fe27049e52631452653563eedd26cea9df7e9829def7a8a08cc2402599bd864aa6f6284aef088bb4661722351bddf830ce223868938df008817d7806256f80e9c750776e5037647ec76b6c2e2744ca5c711ebd859793c4defc7964e0ab37836c20794d31b98cdf107711e929010e6bda216ebb32637ccc298b3634d7a2997529fdad34d1ce5df1dc684ede4432694c6ed12760a504f35c817a88c2163f5a9dca594d702a8a10991791e82bd23bf2c97ce1dda794a90808ae92167e127650942d0ff0c671d316b2e0d8f607fd1bd231e20d63e64386f891f2a5f701ab7b170917aec7223c94af1a68d0f04d455fa991edcf29a36641bed7dfafdaa11a75d13a763d29a58fdab50a8823c5e35e363adc2928551f8356641cb1e557c24c5e75dfc792db215f3102fc7946e0fe8a1d6d9689746aa1f26d4f45d10b2e2626dc34da285d72232410091a72d5aeea967333dc71520c4f556783400e19d7437a6805384ecab3c6799da21f8f459b3f2ac4fe410fce742dbe";
	cipherText = "7581fdf3533a24f9958c7e01e75e6ad750907780cf643e03ff4a2a9f871b218388f46304745573c875765fa613499beda7e93ccc92d843b855f46c795547092437d954b295dbcecaff9154703e5947cfe7ccf66ae0357aba9431fce48cd7e15bcdf46b742de2af96ccc55bca90234c6f517c96e5dc763c1956cabbf6fa7806f674ff3555bee8119b64d48e8e287d22f1c6647683f096f21faa204bf2bbc3fdd3b9e5a81dc6a102deface31839bad71c0f83d6482ca1a749c3d81492c367cd05ad4b752003ff3803d8febcc20ceb675172d537c999cc43420ad9e655707db9e037ffa09124840d877f1f16187c9811398aa7bcff064b64db78f0c59f8919ae24d38b1b3dac9d31bc0955f499614e102a83cf3c9738999e5ce4011d9057653a3fedfe48c2b77849f8cb3225f4cc8995f34311d77653eeecf84c7cb0e9f22be2b82251af26b54153d9d9f33a52f37d4627d2c9df8668fc2ec03f81ef02ba746ca57dfd5c737a4c6116e9f77163efe14c89d9d503218fb310ef4121c6278b795a4af8992cd6b3b5ab2b92adcea617942a008087293fe4e80c84e25949af32a356791229b7f7d8e52256c92c4c3b2d0955f8dea3d1f84fb0e4f7dc9ae8727eac77f97508ab97512adc847f787d2bdefaea2c690243715472883ca907ec992f0db1d2d4ec7b6663983fc3c9ac3f5528e78799645e3bba8eed66e85e934d8f8ac30c17f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 407;
	dataLen = 4096;
	combinedKey = "dcfae0b73c8878018de987ce1963ff1e1c5841caa0518b2ca00ea5fc320196d4";
	iv = "fa1223001c5aff1293e80cd4bc2f271e";
	plainText = "a2a47db3e14a4b732a3d57bda1dc0a4bd16c7adba339c38775ea1480ab2d531aff652f0785b07a85d35066f0addf6e2a93fd2c3ec492e6115de93ceb8275b18624e81a4ecf891b129fcebb2fd85fa55a5cde30f772783eced4926e4bf5612063c3668fc3011ef084b080cb2f1d116358f7306427d20db41d6a9dda12ebc61856d8b75ea77e5cf28121363289ff79bda3472ec6e1a56ec2c276ddff6f329de45e0ff9e528acdbd22b8e10af4468359759ec08653feadb0466c7a0b1795874d943746258c716718da001dd800e03554791d6f475ceb67d451ee6673d1ac72d7362fd37aa31d58fd439a6443c9912bbbb01aee8518d7db8a04eda0e78e586b43317bd16768c1065394980d0e3e3b084df353d1c4a912703c1a84a1d25511fda60debeddeba1c2e4cc47efd0a4366d168d73319f6e07342f53d9da5ee4600682407ec403e56640da8be9e9e80709896c17556dcefa4ada75db31747ab9e236fe77b4a3c88aadfc185bf424b5056f02873d8c1f98e01c44c9a09d7fa89b2733ca4cbfcd53454f4026f1302a2613862bcb3a7d2d2e354ad4a86d6fb7f7c79994b9f11ff8d847e1cf4ddc95e86b72f0afb968d3be207f49b9e8db2118e49aa10ae932dcaa9c983cc6a30a721d74a51dbb46a4794cfdb62fa857c900fe764c7aaca1bcd250cd6eab2a17941bde216de70f0583e402bede24dc0d2acce8bddfd650fb3523";
	cipherText = "e895904575592b2493539a7dafadfb7f4c0b94c084ea94f5d072329073ba7a3bf5e4cf3a21407091ef02c861715dc8efa4540e0ab2b864d4e3e6eaf6b6c30f2da155eecd79e71e57fd6a35c84d2d45837b96a967b117e89a533c316eacc00fa44677fcaf09eb57c438a0932d52dbd2624206a16c6eede3ff41b05504145a3640c2c361c4966b602cc299fdd4825aa41de37c660b691531220a2e50aad91e840980d7b04cb3069fd53ac95055a0c5e675dfb9b92e2b2395f8e5a1c6bc95aaa177a1676868bb9a0ad6c7596c33c993bbf37b345712b3eaf95d89062528a25fc96d676eeab60e299540fe39a063d8fa3983d9f9b2a60e0a28ea2b577f37cf3a0a30b68cea8761004719ec520f7dbdd9fc5e980ca9844898638bbe2fc4345ee8b8e2eedfd84d10ea08e1bdc7f4f96e174503da3e7c757d26f00086fd5b382e8bc1dcb25dc6135077ea84d4c259d983cc4b5c47e4fc787c97d4e4fe5ac02abdeb7a06c266984d19f3ddefe56167845105f061a710c5388e4698a50995d7c92865656210ad507f29cc282607258a8b771c1982fb37d4d1da5bd540e3e206ff41fcee1d0990c8b8907df0ef6313b2207ce503233ccb827917e3951a7c2609f7d00156e3d6d965ca281b0def5865687a5b423071ee585e444faf25c341a39c9786e5355a609c7fcaf4ee3e36a05105865bc35c3e7905ee5ca15e31e2f60710e47ed35dd4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 408;
	dataLen = 4096;
	combinedKey = "ceeaacc64af831f9153d3c1e0e664f1ef67bcc59a05685dd31bb8ff4e9959e09";
	iv = "2d8346dee39c0eebd14118bb0745c1d3";
	plainText = "28e8482d5ce77e5185c74b8940a0f9274cb0e4fdfd485829460c727efef34a43710dd39d0789a78292ba37e4fc80b1be7ab99c79aefbf50bb8550641eef3f1eb6d6cf1a400374e5b3be393e5f158213806fb064e17bdc2de31ccba4e538d48729cb734d9a5f48d2c2ab3e93d7df3e78c3f960ec466fa60527d4a41705abd3bec3b2bf368b32b363746873eee59c42cafe85cc5f3bbaae2baf7baa2cac322ad802ca6df45aa997bccbb8bd8498cafa513219a510bf66a075aad55dc814c4fc113e2f3aaf82944db97589dd1685c417bc6da58ad40485f675050affe50af18b1acf06d6b7a6121e498015dda77b7702caff8ba2e77edc5b17ced45dc020d5fc219c035f0ee4bc57db42bd8449f1f7ba1c3c21c0698cf4cde6a2d2f8cf83ef6f0eacf7abbfa7cca839c9d0b3ec864cff3f11482b469c76912f236161f67cc2952413002d13b4c93f3305fef5d093b4739da7b7e527d554a5c861af08b427a5bb593fb8f2d958d3f6c9376638e82fbb1f15ca06c33539ce65ba11a2f61dac4f2c38ae5e8f2c8499f721b87ba4f9d4d980030c2e9df096e864f8d2b88ea6f41c1ac97a5e4a2512125daf38c23dc51edbaea63f3a3c281a90c656dff52044eb37f2b2540ec0746e53a5bab8a2695b96e3a4f223930b87097b3b51e403b1494437eb9b4f3382cf290bfe373c1ed6152925805ade0ae1d3ffe745c815c5e36a3f2e1b6e3";
	cipherText = "bd8f3d0bc94483e7671ba4fd6ce10ce80f128b3d96d7e559789f46599a399f45fd763d58be0916e26236f04a19e1a1af3f73185bff2c0ce63d066fa8dc7f4cb8ad9b202d1bcb52906440af41abb55a59353566f6dde77aaeb089a70ae8cb46e45db346c54c239d792418a2f370f309c4d34e3817dd56f52067d3a553babb89bd9b9ed962c8442516fd8d15ebcff2f7790af2ba6b5be60c015203cef6e25d9ce941315b9a4263acfa3bf5dce8364e75ae1c20710e2ab507eb6e9a8a77997c4d07e04cd0d4fced6e75aac2f78de1c86153382177c1066df6fea074a2e2ea94425505373eb61ba184937f065643c5bac138552f21cf789afcbaf43dbecee23b8a85683677e2a883e615fdda775a5a18757b9e75af126fa51ecf2ba1b57a8957cf469bc43f8eda81b7c1fadfac5621ac5e69554c2a355e040097a399b96a1c1fde1a769c0ec7bb56f95870a252e8327a3bf5d98cb4293316c340a91f09bb881b6bcba496213f0d36118209d222c4018737d8f83911e8c41fa97779a0c20472d7da7d5f2da8342ca572560e06990972247d7bc23e475cdaf181ff46ef6c8b6b3220a161d76d81d2f52e12b7ea12ba484550cc9fde20f88df2b71588b405d92a972373b3544a43c257ead7a14ac613cb9f27951aa8035b3172a7e11e3fbc7efd5195d918aa266440763606b98166e79e3f25ebdc2159603517b9097184daf545de3797";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 409;
	dataLen = 4096;
	combinedKey = "0eb0cf1d0ca57c7e40ad3ffd80fecdab183681b53096f4e7d6d23ed91edd2ce9";
	iv = "18c95f7732f2ce6b0bbdaf8c2e8a3465";
	plainText = "0755ea622427e884025b7739e8fcdfa676fd176081822cf506c485743863d8b2a6c74435bd69a94753bee14d04a01bca8e1e3bddb7b77f3f1d3bc7d1d039a32053aaa4e1f1d3bfca4b50ad98cfc9c7d8607ce61a958c33d2d370221da0fde5aa2f07e6044a015221cb54a71441602f30114622b149556f004eafad4567c29958451a04d52cba70816281ef77de6aed35c45a8b551768f24543f506ad212782212dc2e6311a4fa5ba57c368f1836a605c585f63f0c5bdfe53b2a0e53ddf409305ef5cde83abeaab91ab19780fce818876c091e24ed0306a2e5ab528379263e34907b4a80a1fa5693a97ed9311b24124904bdea73667baf0fdae59eb629de112dfbfe7bb53a3663973a8037f7113bbc9bbe5be6d8a0cd4bb40cc10f6a9cf04969e6e5b3f5083bef5900d70ddd0f9cafc65b3645ea14344de19dd5639c8bb5f0c17fdd98b5c721bd54122503392ccac41bcfc6bd999ce59b8d6134e269b53ca385ddc011bc09c0f4d31c55e20f7259709cf9b0d50702b060ad7b0bf3f4d8bb0c5fd32f7089dfb335ed98143c4f22804897bd04355d22f574dbdba84268468831784dc5bb46b145208285ae000b33477d3b7c3083d79d078c4bc164f7f98b9c698a678310319a80ea490ec28058f1226f4d628be41502b781078ee72671c5835fc5d59652f87ab41ee4ca964ce120e5b2c8686d49fd6b9a5e5b73dded1d15f893194";
	cipherText = "a1650ff9b65f783d3ee3bf54983001e7b5168e653d9fb04fcb39e0c41c049e3c1142ef8f3286c53b52356470ab3772b7d10bde4913e165b259685c82673be1c81070f7baec8857644d076fb2babdb3e2a22f6a7c09e99882935131c1408870a2685d5be0e338c35823872b42c0cc8ee503cbd5fbe91dbc1f4bdb94925ea62aaf039b61a551dc86f9492b0a69e8e8b6c417f3e70c7eef99281f3a47253b9c4fb8d22abcd38c31cb09925e4135652f6e31e2db307a02903bb37f919c3ce5ce2c8c4664f98cd71a39096c862aeac7bf481fdc4b108082d59af272e9e68e62686796ff81ed3e5b3ee0bbc192b07d6c3a9f03206073b8639fb302d05ad999666bb3b928ce1a9859a527f766684e67b1b0c0a5b42a9382dd78ed8ef005f00929a1cfb643938ecf7b498ba4bf3762487b25b644e0f7d461787c688c58e4c1a69ddbcd188b2f0950dcf720c6f11adb7781beb23bba096f91b6b20629b0390e168b87db5790475fdbfe62bc2e4ead966e401c785537e6ca9b51139de789cb80244c3a67af3767aa5364fa556e03aa78b4ed0516559e966c3b4aa39ea2afb6ffb55bdeb2ac3d1d3b38d0ea78366f5422ac78bdf5910a06b3ea0b4e99a8268630f8e0d9073946cf7f7c4b95d912126d06d7b63f5fbb954e5808d079cc54bb51d9549b385ea76c05c0fdaf664f790c9efc8c0f4e70e1008b8907a9aeadeb1b347cad4787c592";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 410;
	dataLen = 4096;
	combinedKey = "c251b3bdeb338337062d608b3e204dcc84b91f11d3f04841b3dd286c6065a745";
	iv = "a228496b1a575b04027d768a218847f5";
	plainText = "f3df32541edd62a1872f269a11a8ec1fbc9178652078467006e1f1e1a1595224d7ae625c888d1c7349a9d1ef4c4bc60e4fd66d159e963b824e063d4348833c16ef57a5900c2f60496fafeecd540f165e310da1341ce999a3bbdf6f6c15904ce41885d48de3ff1bc7d102d283d03f976d9c9da83adefb4167ad4b79a2b46d5733287d334437d463ba55890dca8b3f61ebdeda543e826aa8bfb5749500fa2238bdad78e1da73afeee47b827ededfca9c1e664a850ea05353196441201c63b2c761d5ece82e4ce43727fbff0da0612d55265b929e70607d6503742ccbc0ef8e608100a02117e297a16944cd2e0b8df49381809390cf2c2df1ff016d992027fceb6d3adb0e52c94bdb406dee904d88792c2e7af72cfa2f861f16cc7c821c64f3cbb6e241a5cd4d86fe7a2f241d9c2d2034a66a56184d54601bf3d0f5137c2ec9f783a913d2fe03ad8761f0131e6173e573cd45a4a2573aefff812c9d870ce5256659a560901778cba2b54eaaf01be30b9c5bdef43e6b0d7c0af75480c2a00c0c38a8810356f73392bc40749982c3d5ec36a9b70fcebc48be72f952ce48a5550e482df9864b8ecb8aaf4f0c943fe11480e8c0d858a482628b30208b75abae7df5346c3f2b4235523fb6c03e1b170202532077072f57d73c22c77611e33515ce04018b61becd42f59fa4c6ca1bdca5bee7102ffe156508684964b17b7e899aa26545d2";
	cipherText = "8fda3538acd5676cfc25bd93020707f4ba74b90fc9d5e09716cd4d51d4b630fd2f08420aeb379f3a84c9cdee5d202f910964f2e03d1e1964a3b5471127a0737ec39f73d61d7b7e45bad543670ad541b12ef5468b5280511d995991da298b0ca9b711f1cde02efbb785f1a43ac498b6793d0837c51c5eae8d47f128077885407a2429dd141d63f056cbcf48c3387a8dc50ed136ea10dcac55006dd7a24a01e7835fc524955c70bf882ddb0069d7b96c7b08d9df913b7d3c25725e56e910441697712763df2f36548014fbf614e7889feba138dfdc648ab6dac01521348eb2c7a526264c11fa72eca504ec3137774e7a236facb948a40849422e0327b06fe36c20e817e12c6536779b77db5383cc0fdb3e4192f56f2f36c0b386da4596736958c3197beced7e79580a866511329778c4ba194683b852f524b64e4bbc3f43bf741b25c9ce58b13dc07197d0b871248a61c4a393469ef0081178b60a11d21fddfbfda5f0bf4698b073d6e387f5abddd5cf817a96d7a7e596e1c668c90921404407b834f852a2bf2120734be23d36c00d0c1dd7148520c92568e8e371ea144a5cfb1b3979c74322a44d5107a4b744815b1674cdc1528d8dae5cff85035d5ed491f06425675015908d67aa035cb2bc842e9b367760c291eb86196300dd869f5f08f9d44082cf577461897cf201afccb4e4921b42b0ba3c1d2f6e96b7294327b518cdb3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 411;
	dataLen = 4096;
	combinedKey = "f85abb76c72099d77a2c32e5456bb98d032f66623409b79f3a719f2b12c0edb8";
	iv = "23f2efddb471d7c3402a498614fc186a";
	plainText = "d6a158cda2e50bf147a64f48bc57d7511f853c216fd28520865012f17b79c7ebb320acec29b2f1dff58710ff223c5a929fd37ae85e9ef7a3c1dc3481ad6a3144eb4c329135cbe208553b358fd67d3ef4e22905bd101ef133d8e3299f6f83a0e58de00225b8a97efc43440a63a26ce6d26a85449c54eb7ce11fd4304c907c83e145c162c547cc2004be7abf9255b422d4fa3a5aa2dc8533743c31d30a9682e6cd153774b23101a55273db6187febf962f51428971a9dea1c39e82ca980ff5eb7c2d85715859acae53a21409883dce8a6ccc3fbb5d1a120d150838e451fd96805cd5bf03049e0ac1697347e870e8aa4472c06f02f7a4cad23eae22abb3f0f576345529b078737f924e62f1841935dfc1cc509bdf1ba6f3b0632bbfe7321bca559e035d311f97cfabed574da263d74c1f74efe85975da01cf467f8039803d574ed3121c4576d7c71c8b84c0ebb628712bd3be06351188b4b555ac0be7980e3dd56e3eea76471f641504c1f1db05d01084e2440a5d31e20a95c225001a193a6ba1ac9b041537cfd0fd6df3252a7a6dce7183d9b62f8a0ebba1e5e983c76bdc056adec03996b323f06869df3147c37839f40a37fac85ec749ffb9e9f6d94c2359f2be186411d7ed178f3ef17aefaa9271cefbcd1162e6ad44a8f5e205494ac8475f09edc108e6dd2ca0a386f3f7b053128a5f0c9646de342e95654b530843f2208f76";
	cipherText = "39f552b9cd3ce9c4821c74ac1cfb9802c06d93f7699bee0a38d6f3ddadb0c19111425fe6b78c7013403166de2df3b83aa904de0c1600a55d381eb1c1ccd6b5eb3b57124bd26fc2695c47369aafa6a580d4e1ab5fc9926e543cc89f4e8871a458bbd7e2a4e0e673e385f945ace945fd2b3aeb142e747bc1b550751fd67e7b7f2e2e0bbadb19482b70dd6d49725afb3562ea86f81cb416219b7505e07f77aac2df251eca180c40e65ed11a754086766b3fddcfa58074098021ba041ff63ac4494316936f61732b9feca94c953b7395dcd5c45a7f26720e4ff83766f0fbfb9548d621572f9ba14ee775135a50bb414b1baf8e502f492c682796ec49f3c2a39d16d529a2bf0bb3bfc292b9245a9b74d3026d90bfb5aa53ae72c616fd8bea503dc9012ebeed4c0030e06a22442a33df2e3f3cd944431db27ed598c0532c604e1b6ee95355a038f0ef382912d8c357f9a83015865cbd80c7e1a57a39635f4287103b5b2c780ce1f31738691f1123096a8633e963fbc74c9b4fd700f9428d7d23c7235c58c6ec165de7a88c6917147dae742bbf5d73e461e05778fe186043c7107f1c2b80d839a6c458a58c7d02c52fb8e2bd05df60094c76d99b6d61515f265ee87b07e2a711bdab76528e916cdb60239acc85edc598bc94f948ff81555ce76197b15ed9c7f1c2e3f4da9c174cdf2ee52ab03825432fab7b1052389326b82f04071669";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 412;
	dataLen = 4096;
	combinedKey = "5d416dec05f741b760a210763dd829de9d95d0d561735b096d84a2d8d9e8458a";
	iv = "8c99757cc8aa63488b7dd104448e1388";
	plainText = "940b4d1e035faeac494f6e2c7aecf4b67beed914f19245184d1232f88325bb1229c118092f9a5bab52f0377d51ee7669165db96bc70f10e60a625cfb4a9d8182ab9f066b135ac4149a45c321bdf5f7e494d3c8d7f4db9d0de91fa817e9de5b87f6d73a1ca22ea898ed5d10d3df23e7f72a220088785d46310b4c3b14dffe28911bed15721e271ac9b39fd4128fdf3fad060719d746e6fc4603b5b31e82e362aa44ac88825ed940873f8f13bbd8d19f335f0e981dd51da91c4b5def410c7b8c2a29677c54afb7dd1af0f9cf53c6df7f9bb2abc7e7789e5d04009dccf63a05d75c37472bd95dcaeb5ce6c5e750df5719f865d75decd555aafb7a1ab31ba05cdb2c92cf4e1841ed79b3c2341193076822dcd2b45600f8cd6360331afa3cd9679d1085e3bb4fb2032a4b71a6d5a49d81fc5f5bb478428e9b31323dd82d3c71ea2dd01a20a6425aa8a21541a8dcd4cec3542a8797af598c7e13d1048cf596471ea59d3df6e6064e0ae81c63cbb8ad3aaaebc48d315f218457a7b297e1219fdc015328b99fb4042cef7a86a2dcd75b4c7ef6cca2764c7e38d51fa7d147db4477fec3f063d8e1d0b74529375af41971245eafcffdb810fafb34fd0f99fddfebf1cf7aa491ba45a87e07bd0395604927acb796b9fa3de795e03e771241f9e93c36721e999064420efea7998749efbdcfa64b038eb59b7583452b419e62da000c9ac3484f";
	cipherText = "b719117870495b095778f17efa0221a33f0b1dc665899dd23c213621c99182b73e44ac526423060ca386d86d75b86bbc972fa4eeb9e1d3f3055580fde26796ec15787423d89132733c934408f8bcf7b20c105a647924319335a0b0d5aeda3a4cbfff6284b25889e98298984f6170af1a269b311936a2e75c00f4ef3be0a7f712b58dc63b7f6a3268ca8cf87ded14495597f8f105962917a854d02e593afd888f59673d785a3d2f0840108c02b8283b379b4a74927f70006f587686d80e13e276f7b62e1b338760fbae241d20aa324a64ec24f806d8109e8cb83e405917e9613e3b63227cd9c9df220992149580004c46737b31398e44f3aa01eea6c9f0674e551553ed3da577d3223513995b8ac491ce77e3f62dcca3d180e8ae28e1fe65e7a3fa7fa2c165c3bfb097249d60e0d535298570d77eacf08aa63865ff7573e71f38d435cb56da8bb737983d495894aa285bc44b62a26e230a6b46bc7c986baaf94a8e30f372a29ffbf16231d4ddea80e2757aa98e02014bf05fe3f55746d6ed98b1cda61703034b8b8bdc2943b893dd1afe2630bb7d203436496d5cdedf817791db74a5b346a83469a3c291b936844030cef9ddd689e3afa2a01551c3475e9737bf6f3a28dca6c7ec07cbfdc28d715445befd7fe4ee4111ebf5f6919cdfe515b69ecc872a5de512f082d1e7c230faeaacc2a7614404ea7a37ebbf2f86339aa7cb17";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 413;
	dataLen = 4096;
	combinedKey = "e41f1657f202cfb3c7fbe7dd8daf6b73c2aae4a38ced6f9b966efd8d9c241e56";
	iv = "6d5c80971cd19d2f57a2b7b97724a39e";
	plainText = "705e8d95fea2cbc9f35c1f603b9033c5a4c45033c8e937ffd162944ac8ad034c51c4882a2bfc9a5c9b895c929d7da554d531a2037f3635c10731c31379ec37af26cdeca3f687ca2eebc904185e97247c85df6aac0ea98f8ca74ea48772637b4bb83ff503a411db9c11fb986a0fb8e7d2b59249889669ca7a2770bc008487546150ac37a602a6f0801011569838f0c3b41fb1d21ecffb691159c0c2ead5f65861160b22787c7818ffadf1077a3feb7c504a09433a2935223a8d72b9f8a9ecb50d1751dc25b785ad7cf4658d9a7468bb2d4518ad0a6a175dddfddff95e50324946762bc9e83fd68c01326ad5ef47883dc334c5d29f4e44aa045e1f452559896a9fd035ae41ab5313b4fea882ffb82bbc78ebc02686726ca5b3375b4244423d2f46629b4f1fef06fcd5ddfe6430ed6cb578e66d41f6149a132f7f9c2c46a9fb63273dd97afb63e2dd73c73f4cecc3199576c3f281c9aaf01f548af45565d47a894027cecbdb3c419caee3e20d70e3c6ca77ced0bded243be35669902b1f25ef106706af4513417818720b15617c1ebc70cee6f3fb62970ee4001286e5e5801483a5fc07f62b77a301229e05a4e0c549a463f153e40e2daa2fe4dc1c6d2ac6e0c032c4c171d5f5fb2bedf62602f88db76a0e570e6e39dabe517751b7c6bddb31b74ccd1da920e36af7caf2095f750eb8164f7b51e6fd023bc18a4adb8f0aa41ab85b";
	cipherText = "1cb93d2e69c6b0817d1e26658754bfba40382a3c0bc73a613c9d14515808fb069f149ebe964a134fddd387a89ef149aa4e7fef33755fea6a15728ccbee27037b4263b7d4f4b8fd568376f8ca9f73d9cfebf318ef26c2e52f6a423cd366172a7cf3ff135f00a032fd74b11c116082255d2fa36d36a7eef75f078a680884f3dd9beb8ba74c71411372d78444201a73d420ad0577189ee2e6a94d6b9a10c4ec8b76de7995cd199aac6936ceb83b6991d7095d9d273cff6802755d7386f78b363cb4f4334fe953c71ebc80726cd5a7461d1b819df7262268c6ad1221310121ce5b9afad9986c7c42f97c8dae8261ed6b18b75f0a983c0538e68576d00187244354079389cda9232d75c3709f2c3135e2b9adf286c05b147bb348ab1f0b2a7ec87547d566415d2d95b27eddf9b7a611d33458988a3273bc0ea807993ee229bf573d93e901be360f1fc33414285e3e0ae837fe6ac44c7f86790e1e7203eedb6ad7e8b5084b8fa2c78165a7d2aeda63895524df159e4aedcefa257d5bddb7eaedc50e3a4ec89ad7d77a7d42edf30bdbed2c844863452fb634c0eb9fa4f7f71e84cbac633df1d042b5ae5283bf47eb4658365d4c8576671a0bb6033cd9aef1e924135a825ac8b7f5059da3aa6b4e97c1c48cd7f2b764e435bf15f09db514bef22829a65fd179a577b8bcc7b3e6b8601e235f51ce4d20d8938767a953c18c1ea120fb71e0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 414;
	dataLen = 4096;
	combinedKey = "c6be1cc30af42fc8689fac9f19f010ed4f1ce0d9de740dfeb84683861ac38cc6";
	iv = "2f3c153bebe6ad3af0355ed00943a8b8";
	plainText = "8a9af2bf69684ec243f3eba275832ade4b9fb83e72fd7c85a37261ec7547d1611cb6e58d041a5c7c394caae491ac1084bdf3fdc418f04f5cfe72c3d7cabfc5368105948d48ae788c379b75f670e5740f97c4a690310d24e3fd8a821e22e3670e11db55c7c775bf8783fde379e770e3ee9eba840b325e89ecb6d5125ebc2d73ade7eb68a2167f8318012f533e7b5694e07f377b10325da63a63d52e2761469674d16819b79a447bfedd5ef8f66b53468efb3b1b7778bf6c587d4c137e78b880e812dcfa4d26c2747848954600bed1b9c2ea406852bcfef6b9cb910ca0294f37d955a65db1e13263dda4c66961493960aa43858bd7dafb621410dc5479872a5703bf9da79fb610f67d5c130cf015d471575cf2b63d6e443c8862b8811c8fc5ecb810aeeb7fa91fad03d7e69caca77a232e2d21e152a4c9f2bd0639ca67669dbd05f18eb32adb090886c8e34340abe3e5b61b4a218904b95caa729c5711c41d97cdbee71434f2b4b58abaf0aec86be1838fdaf411ae6034fb6294401af6c53fb50bece5042b45b927a9041b2a8948ebac284095605a709838b974f4bfab4b89713072d2f7c746e5f75ae5800342247e57842f3b8d7b187e41a6671cfab420b085ce7b6c16d77bbe627be97a8de3b841e002d61f25263d47527cc829cb866c42cec7517c4b4fae40670589a638846ac5aa99cebb86835bbddd61f29ed8f004d2eff4";
	cipherText = "ec69b7f0075b10773796918b75191af62efc7b7c065faa2998a1305ce14f681ac4a63caf27ef93e96a1bb85b08df22b470b186bb6d35f563b1892a7872ef62b9b48ff0eb50184a5b615526a649e7dcef0052899f5829ebcbd8f0e4922c29b994990e0471a8466d924d3b96f2b57083e7a5f035daf9c9746b53d5dfdba7072a9e9d576443b4163d688f50d4959d23a4abc63cf8c7cfa420e016b30ffdaff524159f935de5bbefb96c599c3a161d44cd1fb75130b0e64f15d937c8cbeafb06fba09305c8b8aff882fe69c867fbab65d439bc885d1b4db0149bbb2b0d223abd15baa2a41673b5e5ee7fecb7e77646ee9c0f7999c9dfaf284227bc14d1db3e8eebce2d3be589f71d14ce1441bb5a5a44e3f2b8e549589ac77369f7d5dbf15b42fb8e357120437b4b3cba0e58ba36594b3908eddb846e3491dc2b330d0d259dc4d13f11c27b7c2c41a6b87eb052a07eac35149ee2ec9f220ab7805391035aee0e58d0c445b36aaa1312f6726011f3b4087c59da9f70d6f0cdfb6c031e6dba14ee3f4fae311d341af11e8ad260b44d3c0829fa7cb1109ffb6b7642160f438ce04bdab3d959dcabb2418cbb61b76d68d44602709a8e3e375d0150a16d3f49dca44e17ee5ee42c6dd2883b2c1db985d9571d1efe6cd838f0e6c6d6694abc30d1a3ec7db89bc063606c5249ab8537eb27b0f5d36768aec635ac4c9566c0c2c3b3f9eedc14";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 415;
	dataLen = 4096;
	combinedKey = "114ab485de4ca638ec8f13c267fbb44fc15eae1476c956253ed453271fc9d137";
	iv = "f067b8e98e03462432dc177cf6fabdd4";
	plainText = "28b90e27a189f3f13235c99010b8af2a24d0b79de895a5d8692f34b6a77ec101f0c6bbcb98cad87e98832a96938f4c5fdc5975f0aa4fcd2f28e397bea542b61ffeb5800d791919616108b1a9a0aefb419e091c788334ede7eee0dc3b79343058d40ae2c649fca91e40316daa6c0094a578eb82d2c3683fb53500ae6d2515392ef445eb37c3766240a859e18d218e81332b33e60d715b0ca42b201d589ae7398e792547a02c3f80b6f36ef719ec335d1b88edcdc0ffdd804e218ce3c2d80b28625079e130606d85a915d4cf1a34457883b7e689717b58ee89da4e445cd6575f76313d682243799695e280b9b6c4aa8f22b99e2b6b8efe83f7e7c0a49e4c21df15cf4fc2cffe06e1c06c5bf45880dec25d1a224696a6701fbeb292b2fbc708d96893536b358bef2e95cafdc5fc0aaa542efdf342122ba6f1266e3e13fee99dbe1c12c9a5aff5fea83b831e98fd6e49be1af26cae1e8d0a430d6ffaa42305590fcc97474010f7d21a9242e5e8f09bc786414dcfcf129114b058455016b74609bcc659a275337b73c582dea9bdeff7b44e7734b68921dcdc7b0a2c84686d0c42f6997240dcd31fd4e86d82d1e8f01e5625636fccdf5ce5f0106d1d088905444ca27602462adfc346079acd75c2d13b4f1db110dfdbd3a8c55641c13545dcf697935d331962eecf841522428f597c4826929b7bc2efe66b9bbcfca382fa0d745970d9";
	cipherText = "5f5e4dfd96a96e4aaf4c471ac68879bbc709af5c3ab88dea06387630b839ed6c7d068d1529dce9e8ab324c7a021378dd487162968fc4a1470b0394ebae5b870d1e4bb948c1167c4b9a11265baeff6c41c9fcb7b3e8e9c8eb62e1462a2d75d763ddaa4a06dce09ace871a76d25cee177cfda179e2953a2f236e28c502ed015ba1db6fd4fd1d7a2b53ccee511da1e1428249a40cfd591d4863057bbd064e58ad927beed072e45ffc53410e0a663151e4c6253aa2fcd54413aa5d9e956c1ff1e5856c487e53b908f63e2eb1e391f702253441a162db0da7794dc4060e657426acd08d06ad75190652ae5be2601b690986839c2a76b5c4f2b865b2100f918a254e74ac7332452727ad2310ce21e0dcde3775e92628c4e30bcb82e2d4f4a07a40b202f3e4a09003a178680f9c21de139a04b8c5f941cc89c60cce7fa91d5185e7ff8b3b6dc18a1d44b166cbffdc1bcf0ecee9009133aa93319628d3665b95648a3cb98601184f00c06b6efd3fde3b12271664e61f61c2c81173eb092824f679d62934d5b905f42773f7fcd04deb397cf7ab86c396f2bd6b508886927bcc31cdd9fa1c1eda2733540d65d48d16f500c25ad25c7cd75dddc1a98cdbc6eae24e1beb0d37264849db7a1b8cc26b8d4b5e6bd0e04d3f8484133b49e1764b66b083d4c532d8d5b3c190f359b34da04112a948e757e9fe191ddefbe8c5ff925e8863fa0808d8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 416;
	dataLen = 4096;
	combinedKey = "b8439084c248409b1c92689cde226dcc0f32500901371fe41271aec837adc644";
	iv = "2a84edb77b63268d8cf846c91613db64";
	plainText = "9cf200dfd2b5ac261e63db228b2c54513d68e28c88e4189bd0fe1175bd3341d646a0c7da12d0e5dda7a03ca4e0a3f4f3dc32983129a6ffc0270c7e07bd29b7476a213a9035315116e87410537586fdd1101d7ee58da7075518f7e84604e768cbe67dab4d9d5bbd67951119d7c8cd6d73eb185c296aa38ab63b10faeb234bc63c9581dc798a71cf4418af5d9da92cdf8065950fc3a530c4b9a21707b37af1f12cb0508853ff8fd127f79895b067161342041d1736192d7506505172e32eb5c9292db337a23fe60c462db603b0189be821716721ba22f1c29a029d9b060dbc8a8d82738491ab9beef4085359c16c83f46cd08503f767a3c872cc948512e56d36fa2a2f9a59c31dc129a50ac938cf1e3e056368024567f852fc4bc8ea3f89c269ea1af2947ea7814bbf9e5e51c39e5e07c726e02baf0d857c847ce67012bd0568613dbfee3649bd47fccdc1b4a425da749e6a7e8b3b65f4ee6869d859b56952b48fe2f35d7cd681ac44f8dda2dbada9a6415fcfc97bf3e47edf6d57715824a2525be6dd2db87c7a2956e99b9b7523237dc6d1b22a43e0d1e0d34a635e335a8104651a9efbc74f94b6bf05ec80b3df44d5bbf6c5191ab4fd866de56654419455e7cb0eaf3d4566898b4db3af1e6c8deff96c552a3767e0b1e5abc82eecb8fbb824199cbb495ad14d6b6426d9b9ae3873498234e9d179da988d66da73ab5e87276625";
	cipherText = "ae467cfde97e40e540a21b46cf7c35f0d66d3d9d054dbfd5bfe9ec8e41a85c4f0e64f3a1e17bfc8981123d124ddc631faeaff9273448524123c601875da896c77ddedee8bc592f9f10320db271ab9914b21b8c3839cd8988d02a1e5191a47eae2551915c9050477b0973be8be27775ad773897c098ff3cb345cd72299adae6edc03fd9b52fa755aa0a09e6d251a59931a498e1be3b71c4f0395d250d4ef70cf7dd2e525736407579d6893c614e9f37eda1fc3dd67edb5fd949f885a168d68ded6e8421bea32bb10ec28b703b9041762bb6f81b63d6bc2d8111445b36f436d6f2138e244a1b304ac2ebf16196c09a275d298aae7313109a96372d4184bd8463419aba369e0644913099391b4ef4157c8954ed94b0b88552fe600d396f0f9a0e937038f4b45a0692f52957160e80f96ee7935901e8dbe32fd1d0c239c95d21f8b445e150d802d3518c88767e342a365a7c874d7afdbf4dbc5add15bf572e45448716eae763b904a9c44f5f1605e6078d40aed4dc9d23acb301627d2cbf8663ab84a81f4eaa86d7b4474a41410fbecdf83d72ad6b97fdf7e114fdc9ad603ec20d8769e5c31ed754725aae46d56e76d1406e575a792a7d8dca732b9b1106cb33166607e3d8fe8f13bc3de972ac5c0ed1fb11e567a853f26ec77f63d6df6a050c35542484c28934836d906c84663e94d2a451482b2d9ff41affcee9b270919f03ec84";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 417;
	dataLen = 4096;
	combinedKey = "be3ca6a47c962518bb21d7e67f259fb278383eeb5d44b036e30794c3dcd2715f";
	iv = "05369aeb46ba48bd01a6ea7a4f5e6264";
	plainText = "c806120710b6434a55c593f5a11181ebe3e62983a4e41c6086e9db5bfdffda73b87af990357882bd8252ca55f44ed66835a2eeab7a9b9049a26646c323cfcb76a42075e585e609ba686628091ebde317252248a5264f5ad5d79930694b14f392a3cfed48a64a17e8cfb6905e168942060ae4520df582318fd1a3874a7fa3e5e7475088e752fb0a48bfbbbc4be9e2e9e332e2fa85df945481fa5991aaf389b51ef8566c29c9ca71c3a5dac261013c22adbf9462684c4539728e0615fd6cd64c167d47a20d299bbe8e23d0eb92ccc447f613c7794afa0f7942b18f3cdea193bc6245ebc7fc00560cbb8672a685f23511ba50c60931411eda0f3a6ec243d6bc7d8ad515008b206a33cadf6b69259cf1b3a174ce63c46297d3950830c696aac8fb11041638263249a6470350175a40f3a054581221fe9d87dfdc8ca24843f692955f0356640bf02aa7e7bd3a6a9e5b485ac25e9117dbe8da336ebf5e83efcbbcd20f96011f99989ee1d31779b72b18d3a203123024e291eb3379c73052c7ef1e1230bcd2a2d02e26dc2b6137bf5f92115f0eaa3c22128342a9779da36c9e67007479f01e5539748a8fd45a3825832b3a38030b7f2f3b099d1a1458d54035629981ffc161ab8f7a89859105c9ad561a2111bc42d430b4f418bce9c938b72e7dde1eee29c238400cb0dd3a52eb8ac175bf764374f125706c43eef80170d2fbe245da29";
	cipherText = "e18b69d617868a66fa54b624cf5f4ca9e9698890c5015dc3a489af6a482d9f3fa25ae2cd2fecd871ad84bc0b7f146a7e831e3567fbba17412c52e99e523192ce5a88f63bd479586c43c8d15efd96034bb83b10be7d1f41b59aac11a7d02c64aa358773a73963604646eac401fefe0c1eb009497589df01dc619da800ec56ed56015baf5a30240c8c0168e7d6464d2f25b9aa2d84a87c3d538f173e5113874345b3239a5ff78f3955791a0721812123b9d0c883ac3f6f0069384e43269b3c7bed07c91445d2852ce9957608c6a7e42f92bac993fcd87d73e1a8691287dc4e64d746800984b0e76eb0997290d1c9e4cfddb43531b56301ca88deb03c82d2b9df71d1f57f7a74142c5f818ada5f00a0f556cafcf12d40a6ee0b47d8c9474ab7457fb16960d874e73497e91977d7ecb9558ea079416d6af4e178b77c2dab4390e0ecaefc98bb783857086436e1ab394ec24333fce7eb771a180f532b638ea7fea8ccfa47cce11ce3adbba1894f695d339b946df7f87948b1c10f06ab85e6dba1df897665419cbfc7effdbc1aee51d4a7b554211b3d852feac2cfdc8a6f43c0433799b8c80ab94c7dd1e559f164e96b845accf46aa8984fa5ba64ac11293a22407dd2b4041a8b182dd6a39f8c7221f41c1d4d7789d7b2e386feea7c4c498cb083fbf08c0b0a92aaa15d921379790429bccab5beded9836f1b18980300a808da61211d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 418;
	dataLen = 4096;
	combinedKey = "58f12c5b13f854c7674682eb19b78c3da8dbe330c9175eec244cdc4cff8f9313";
	iv = "1d854ab2593cf694efcbf9e9fd5fda33";
	plainText = "9174285bcd5e7c992cf35b2cac68869ac1c0d70286defaee54cd69ef13946cdd7cb65fe365bd1833ff3b80ec1db8f084f9a0fdc6a55d1c54eac54c0028187c4657659dd1d7bd2fc8ca408342a1a957a709208af3ac611f2d88e3d34930cc510146a1e6da2f38b13a8ad39beae1cb497b3079302956516ecb7cbcb27237ca7e163966211d371ca8c4b42b012d68019d7b967ca11e5ed628d20cb47732d779a0835d8ef251242282066f5a202b7fa577f6244f2f1c9db9afecd5ea8418e2196fd4649255cb09eda2bd499ba88e06d9e08cb1164311b401c8920c3abcf67d1faa253c161bbe1fb89ddee7a630c779e5f4115ee708b7f15086669107576680e6eef88e6ea80b9a24d4eb0ff0cea5de1ddcbd1ac68977eae30c73b19a9d616c2af14fdb13bbe851d848f9ef3af7feb33b26c77e44a8aac3518c3a2ff0402fc05fc928dd1f6e586dcb54309d34dc5c35ff1e12439584a7f66d28032212903138763f9a38d57b1b2d80525fa68936423b11ac520be76f5b4bd41613e8cc9d7364d8cf0d0ff3b9dac1ea43b78fe1d77ceeb622f4516b1b4f216fd0de17b81b4ebd11d26d598a50c97dd5b104e0bba7bf5a312b1293f3379b3cfca4e6bfa9026faf5c66fa4596b38e45c3c783c65460d91ba18ef4b76cb0ab124fa9460f4955039b36fc68841bc82d5391ec13fad72f9e69648f69491bf1b32f73f2a83e86783b2aa507f0";
	cipherText = "8cd1c04e660c7975858cddc35c2e1ddd8b8c6ab84a3813d2f455ca4ac1990dbc52c9309bf020472220ad17a358395b6908b202a9ce489992a1c28b454ff4b69dd3a0271592c9b6f9f2102d0544e2968d507dc5fffa42abf5f6a090183f23f9a8f59daea46b9ca1c36196052a883003305c3873e36920a5178ac32a1885ac6453803907623590c31ed7ef12b10dde9f349415a75b3f5c70d02071e8526e32bc1aeaef34ef74735c7ba64a093d1dd8fe5467a8209157b85a1d09ffc492f638349a66769df233acd1e1247782f8c69ea9da116299525c7c5b4d1472ff43e94b88cf9b1c884a1f75df701dacd2d8b26e6222ad037b5405bd71ff63c51d16d2cc8da825ef49f6b254d2d8d59bf014e063f0103d908db4ac0da281ecbd96f0086ce4375505ca9221b8dfd4645975502caa87eb32be5899466cc5cfb9b1b92041bb46669ed0ce1df8e7fbc9064a70de94849b9f26f72b06a6a6d56786ff5ad2b30da9d90e3254d80a7619f2fade2476baea0e46615f6c155de32968ad91935c504b306dda17f6aa41eb1c8a18a4eb0f5eafa1e207c38f0a1c4366f6a4479fb92c50fa95c6558193e36006496cab04e66e207aba529dde83124826aa8ab4f5059960688b818dce918017372efadea7bff76204aa018fa381cd12874466e090dedc6d512331524e79a5de169e69523e265dc60f97d9d459106e21c220079dd67efb22b84b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 419;
	dataLen = 4096;
	combinedKey = "0ec04bb4d95f3827649c1f4704129aef136f7aa5e2cf719f88f93837feaf6b72";
	iv = "938e4f89c4654ec700cdbe5dcc6ff21c";
	plainText = "ebe3522bf0b8167fa4e7f2677c6c589d023f7c83e7d783be72c50bb36f7370dbd925f24ed484e1c5933acd2b07d6520930c4539a893f7f41a7ebe09b09e8b6e45ce0365640c36fb39b824ccf461c714d489181086631a84ba5a60c6dbfeafab1d14e4bb252554d3c6524dc149c1aa875afdf493042080675e3a23b0deff6286bafe3e83eda823ae8aebb35385e43d5aaa0bd73147b29892115b4daaa7069d8c5c4d0c29edef15402a7ad926e15680d52ab016de8db6601ecd1988e640cf35ce4491d6510e058954bbfbbb47d2c64880ab46b1f29be777f3dd897d4c852d3dc4454113cd777afc60dea1a2d4576c5a7f37a2abf63eb5dd3b4d8d34354f56b959e1fb5a6ce8f4c775acc37df7cd08e8d48f79e5c68245ae4944b934c942eea44bcf1d65d5575200f092c7029af2c907d2279bf1063a56206e5e1ff5da915a97879232ff37b5af5de70c0ae3bcbfcddace51ebe4a5216e459b696609998190a55b0387bd0076aa2802f88e474cbd85d5a444585deddf8703ba7c42ade353ac5184248055a57bea52e6d5f36de27be4952021de59452097ad8aef4bc2d19fd81dc7ac5841e29cf6a21e49f4d7b192ea5e98a6a4e754082c41d6b3eb07c99b05a68fd4711d472ac83f0be496f4e529330070ec334ca6cb3fb791e8c0a63bd5dc81140f28718d78ef037fd3ce41417564dfe9edde86163b508e4ea1c8834ca296407e4";
	cipherText = "1a16d2c7d802a7603c8879165233bac322f17e0a5d4ef516dcd13c348414db5b2bf92317760c598d63ad34e90ec2eb935ddad3e769ca15d925607aa2a2779b4dc8b1a1fc4a2ce801dfc0551a18d5c73fbde44a1996ecf088b1ade31fe64333778bc847ef383fb3689f941f300667c17032018626015f8ef498d2ab1144e6eac2f5ab9eea27d2bcbc22a84c5483d4c45dadd6c3e856bd4674a4fb06a79170de345f4c7cc4e2d5f5302cad2ec04ecd169e593ce93862d784e4a5cddadefdc078b87a50bcb73f83bb6d4bf610d4b692d6ae3510baa96536725b7fb73b281331c4a272983d1ec6bb54b7e3cdab2b7a6c90d56d500f279729c1c985f26606f8f97a34d8d2dd3e11e1cd8c6525f954a8b9d45534f0703eb2750a4aa137d8f7566549c3501f919ce8bb493446183e08abda8967da5429e795d3b8120c2332738fdd714e9af056003e4f3c6c8efef8acf2a4513920e056f7ed60528709444af7c3211089a4e1477a12aca0d5176a336cc50d87621dd72bcbbe8ec8c53c5c7344466efa652c0e85c1ae7018084a74ed9578f257f4d5d455c9a76697f7b061f99f7f7905ac6ed93755681bf43474e92f50bf0cf7f58eae3e0959a125368b7be1789ea630f4f781c7168564fc408c4f07b468c60fedb3eabb2d8e71651bf7e002c8a68c9a167d2e0a2c1fd9e39f31bdc5134c3ad83a22ddbd5830058ccf090fc511552aecb0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 420;
	dataLen = 4096;
	combinedKey = "75b04f2df5e54ed67f98b7c16be0310b379cf230a0e7e0e4e4a2b05a992bf4e8";
	iv = "5ec3ac82be25f05d80c60c6599eaa109";
	plainText = "b65f622f14bf6cafe65cc3cdd055466e26ef07ec74c1e96a51e668e72e1db664ee5b96994906923e1bcd37ece3a9687cca58c2d2492d77b2d4915d8404e9526b4d143dbf68a8bea9a8b5509808763ef2d8edb664f303111dfa5ababd62e8359a0b959e91d64db17feea6e2e321108e5f1bf5249726eeab7e567c7087e225b8175d49ee694bd3d18bfe3f807bf7131f855d5d8c5cc1c7b67cdb30825a57a5e340f6a9052c646c79a47526b06a59bf7b51bf6ff470205bdfd41eacaa5c1d25bf08d695c66643223edfdd53023233a9e391dbeeb391fc99a949c3c96b99cacf871cdb8017bdf806dbfa43744737c097f6f233d21d5d7b1c181c26b9d87735b40b3925e48462f79b9dbf5fc342b3ab698635f1fff82845a724b71df4777b19d2022cb06694acb0636da938205ab18d176dd28d8a7f33451f66155e30f55367d23c517f6a280de9a3fabc6c2507b40c2cd6b44e7d935f7cd49e7b3631bc2c1a0a2a7f6ea7d3dda91545b5af04b46daec723459050fc8ae5506d15287aac7677e186409dbc853e0da9af4fe727fb8b1c3af2afbc716b2777de379e2b3908e3bf457a90d47639e2306d2918b1676068178849ae1bf959b383f01017af6a89b3449ec54164f7578db861d5a5faaa3f05c88aa60af40f68194aaaa9e118b0af92d8a158765649812271bd11b54129b7cca6f37d67af3acacbeb85688e8c1298db37fa0105";
	cipherText = "1e409941c9622f0ec67e9f3d91670bf1c194fcb8e3d6435000307aa1e9607459442883f00a92162f97a3b3e6ef9dc5603eaeb6bd5b62623c318aa0187bc67c66222f14c77f6afdd732414e559748c67fd28eacf155cfb06a942bc0bf5dd28aeeaedf94070ef5e3beeb1de505700c210e7fdce1c1630f30ec01d4669934c808595f14dfc692fb900bf5072587c3e4fc8408bbeed540ba4f3ebd484d81b6520469abc7dafadbfc109f2354159d4a1ac6ec42c2e2825a3f7d4a3389f76c7050207b46f5a22f9a44710c45368fdb45d361bebb558a0e57ba0eda130f2c4943e064f69109b74c6de95f09331a81d66b3fd94dd66f89e23d548a0767e3b873dbcf265f1e4bc12080a75831c116240bc0d13ce5c0692a6e86e2051031271db3d4f9d422dcb03f1c678b95ce3b61914947d7ff279fc59e491a1d8e215cb72c6d0bfbab8edfd2ab5d822fdac84bc84fe4f620c85d26381e0b23bbf8bfa70875bf8c99cc00b782892eb329ffe1463812b6ee40553ffb2068be7423e45f58d9a53bb5a3457f098b10070f4a76edcf4e2dfe49aa00fb88de609c9fcb3fc16ca8d901430d3b5d0b24405b1e4b22640e924deb97c1e8805ff7c5c65e2cf98d9d052d11c77bad651de7ff7212778ac54e3c9f1688c15531c79e59bfd421270d031a5a336fe5e199633c1365310ce0f631c60359a3cefb1a24686cc409f2a911df7c97bb63f0dfea";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 421;
	dataLen = 4096;
	combinedKey = "e57c19c6d08a5e39ea71c0003db5ad72d79c7762415d0b71c2cef4bc458ddd2f";
	iv = "22208821eea8cde91fb47ac7f9f49348";
	plainText = "fc197329bd3265a73c5d8f97c0dd90308998186e95ccb7e85c0b40611a5a13c1cb5c1875d94c379739b26e312730beeeeb26e1fbc75cda3eeb1d02f29289044798be2ac0222be37ef4dd41fbfdb83705c6766759b17993e5b1177497fca71380bbbbdb125f8ca23ac3959d970fca7d16074c25bc516dd6808119a26dfe21551c4715769c7144598d4545e072cec194c52d81ffac219facc30f9445f067998e2febe911fc3b738ad5a94f56855781e8cd02e562ec3ca24f92aa6a4d03f1ef6532cb4f1a918e7e953206d58063924f0edf69bb2a2fb7d5e686bdea5e1f24df26ce2fd0c3bdc1cc91765f04e07c65f7e11fb62c05034be64379ec474ed20f39945e2ec34d6d51c7daa172015144128026a433004ff0c587ae8ee13c961d417716a6f2e2c6e1660331be986155038c284963f3f8ac9b08ccb037eba2bae46f7c514248ee7a5f07c7aac17a5f8ff4472668cc7ad0bd20af43f81a3d8b8d11a837b3512c29bb268b482e76b1f6142911274141dda000f16282964fae7a5f4c078fe1adbeb9153c465e7dc54b5030b0eeb675ab7822c295b198cf29da3799ffa52a397239c32880bd2234115af69fe85e71c458d6d9b8cc807e1026f73eff585fdb14c3980c9af939e442086c89b7f25dd07d112226d241d1c9f9379044349ee5a9154fa3f26fdd2b2348f4cc2a30f46d06429d918eb1e54a97354f6fcdd621221797cf";
	cipherText = "b5816b2f8312287676fe43d6c1fdd656fbc9383df2833c2d657552a9aa33c36a8a646d5ed6d2f414c42ef18674320492044a364e4b524b052172f83d3f97e5c4c563540bd1bbd212e9ca2fd84a47177b51f9a486e4a2462e48a0ae022aba3a3d5d601f498407ab5ebb625171870925814befbb935ce91f6c5bfb0c9ae0e6a5f46436448e96a0aca33d698553b17fa2b7dfb281a98136e0590286cc843ef2c13d56b359c4177f413cb8322d51d7dc0b6613f02a8a0e9809e26256a82489a1ec1386b281928af657368120ae7cde8cb8f4c52277c1bfabc57fe59a6f975d6f068796323a0d6851749dbfcb43e81c37f53be1668a86f8964e342dcf9219b57c9d87687c89c88e4455c55c16f2e2922e5e54b9f037c06d4ccd3fac10cf00b367785e1ae719d692e43b76a988ce95759a2a4c421a30aa816d6431938ea8e5139d43701eb3bb56bee06638e2e5dc4ae03a009cd298d2105cb8f5b6c68978062708e9c50bd8a8c1501323b5c5c7f7da6af56e5b47d155e77e1c3615da09aaf4668b06c06011ced4a0c239c0742c214367e54622b68e879e15b0927f5918535a789e07d29a1f038f56881dd6d02c5b16b9af898efec81cbcebd7cf61fc4344ebd64d3de4cfd1792d7a6e827961efedbdea5bf1924964ddeeca8b5b976e2455c7d1701e0b7cae12b9c82c77c49ff3032985140238d5586dee170dd9b556274142eec669c8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 422;
	dataLen = 4096;
	combinedKey = "8328e53caa5c13739f319dcfa80b88f66bff215f49e1501db317ee29e8f23dd2";
	iv = "34bcdc96cd2a76026e918d749bab04ac";
	plainText = "736cd778f421c9361dc3be55b290b645e4f31f48766c53e6b7c004c9f1f2c74c4e91671cc5cf8959c46e1bee2b8e4389641b9c82fd4e4a4d0a3ccb8d4dd000cc155f571ca627934104387539c4eb3ff33c8b3975601511117f66a6cf3006d3c0ed97d0864a885ff49f39d008f9f507fb1c984319f437cfd564b71da01c7d6df9386f28aa1b23baaa616eb1172b0638938d6658848557a330f8eb0ea852c8448efda483a67c4c5409c3cd2a17a9fa5d8ed9b9424806b41406351ba5b1e6c7eb0b44e8ab984b15ac3fc3bd15b6afd12a787687a79e33800581212231fba640272b349ffdc4c3d7b8c52684eaa30421a443b44b61d05bfee6bd89741caedbe00aff03283c265d6c4ac938fd7b521016e95039ca23c599e0a1bfe5b5a219eb1b2aab9f5cdb7c8f47cb082b516fdf19babeafc7131e53c59bb1af832b8a42e4b0c478365babab50e68122e1d21204a9265bec5da3f3d332017fc424814dea4a86c0d2557cd935f72d14bd6089e5825b48647ba3aae4a10eda09aa48be2dffd969fb14c651126aa02c7753650f967de21681d9b978fafeb1cfa6d2d918a83c1f31e20ed34a3b0a977490d0dffd326fb0a03bd51f97c22701181dd221e389b62708884a83fea01d7da3307eb0f98719711defa5995c93174df0700996052311a0ab829463bfdc1611838df75b7eda7b43816e09abb3e1e113ff95e4997a46e4e988ab10";
	cipherText = "66a4b42d1bfa63bfe4d47f29d306f84028934597952162b605167bfa11db5c57f61e45cd8c8b668cf00a1696bfcb468f3695bb08e758550f3d879880cc0da975daa5247febc2df87557db7b8b33e20974eb3e305504d66001b3fe52dcc606aac98faf064c5cda5d35b2b5f518131fbc2b0a88009658aaa145efaaa823141644ecf75744ec27bcff848f637ab1db89c6afbb9b931c0cf34e2bdf9f729ad49de1a9f27b11af2e5880c5ed6ded9dcf5dd1cf7f84d1d87c4b95e2eb0f9745166b97930174e9c1e3a1ecd5833cfce91075f13e011eb1be479ca8bd483371804bbd1cd02b600b630ba17e9fb81534c331ce068535d19532505cee60fbc5a6f43f858c7786fc82984a8b0816809503eab565f01b130ee9b97001c0287855c6428ef90f95d50c17ac73a75c5adbf80a26e99c62c7f1f321b8d8428ea9718b15edbf87e977a776c150ba3b96833115c076eef711798387c903a09e66082bc6b0db6f9706b591b1f49733239e6fcff53d7cfe525660123b9e2deb6b310ca435c3b7cd1437243c30eef1b4bcec2db6fb7c1e215d15f5d943c3bbe89e6409bc97038b59bcc4fda37058957b2cd08ea85c30c356adb5b6f7f562f77dddd7d735a3efc49c02f6d7af58bebfbd8dee9099cbbc0eebe5b8737deacbc3f93c33ca94bfd95d22ee1815733258389145ad1302d62ea886fb58ba5e05c2c9603e7d8a1cae0da8e9ee16e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 423;
	dataLen = 4096;
	combinedKey = "b160edb2e926099ad6baab267ee617b53986619100306f9612d4c5f00221c47a";
	iv = "8e3882d7da171f856165dcd66f9aaabb";
	plainText = "1391f31350cc345e8282f62a8d67e56e3c072931c81f4c22df281c5844435c2d8561af808435f9a5a4122f57e01327b8fc327120a279c084643724dd3dd49da7b20e5b0ecc650d1a6308e677894d0bc3d6a25bbf62cd98543d6d5fb96995b2c54eca558077381f64b9076bef5839ef83ce235fdc8d794220cb730213da9b946d395b2c1aefd5c31bbe79bfc9527c2583b2c689511a0e9b32e8c604cd116455af443e0d7ad2d9a4b2c2741e8b43c19f62b6fcd5188ed8872f460b70f5a55464d4afb216c1ac54882c05cc37848fb15cdd296c206b4e337c21e3b56b479ff27b2b410771faae995741f3982e2c205211f22d67c023c569beae700c65bd4cb040cfa68c1f1377429396e30165af614e13c0474f1b1f29a5959d0753567b6d7f0cfa9c6bb58890803859c754567487d653e19da9a3bc252efab879b0446033d17cb68484760fe890f6ebdef1adfef514a48b8a78e17eaa9e184e8fea9a3c26275ddf689a4171c4811ab36eee1815dc9cf17233ee8ba1db784d67e2405475d10ca5c18fe9edcad329e2890c239980c82ffbc1a81adeec660c8b14de34914a2970804d20ba0d21b8af110e390402bc69efb458fb1b0099990419994ebc3460c55a54999e6f16288fd6b47f1aceed5c930628e2999855b670a97f8c2ab39e562f96b408392a92f4007010fa0d56d6e1611745e87ab911ca10dbd5c6a61fd77461847040";
	cipherText = "7fb7aee347c6eaae17cf623c3f8d61eb3a7dc5c01137cd7af2f4a8f49d0f930948cbd6e8e725725c61aba499af1eec5662ac8d6cb398764eb8d15dc624edecee2899ad5cf6451567a9c1f8db13d7f8c3c6192f8915f1fc74afcf30e5180b0f3217957d23907a264ce365df468f4fba5e179d93adb7269d49501bcc7908c20bc813827a03056b80d8f501cfc2aded145c8a9e8f0d818d46c6c7feb0f7b071ab9cc361af97f6e1ffb1e82e3db68dc4c18d854d3cb7fabe87c0635357961d8d3db7f43dac484ef8741dc7d1c0947e30b569ea2dde73d7cc6e1be3f98509c0319a9da61dea006f0fea26dcaab57e0dcf5255e9f819589615e9d6733451067b9cbb248c9d5bc1fbc1504888f3399de85f11702ef281eb925746bb93a6bb0052e2ae7c044631998c35b491d416ec8d181f687e9aea755f0ae6589447f5aaabad9c2511471f1e4df2fb14901c7c0a18a44657e96b16bc3106d253a6412248c89fa0305a372a4a773be8e29750fb2e3070950c75e2cabeff7498e670b5d63146a90b61e7b3b4123d0aaf4689237f7abba36767d3c1c004f477c9b700d3fe227f41a42c561af0b5ed4df49a558a13be870b43c347f4fe45e90cf9368c69027aa60bafae385610214d0210d9395ec6d03440136dfb077d16bbdbc4c93a267f500d8f049c3886d4a4e549597dbedd68990db3b2bdef722f17bc9af14548cfb52384417b8bd9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 424;
	dataLen = 4096;
	combinedKey = "6451341f4cc584eaaaf013d39cfcfc71ab1126ad473cb2d2ba563412c6428912";
	iv = "1732c8e006aa61fc28d0ac4365ecd56b";
	plainText = "5abfcf609cea362c71056e2721757706c64d962301735b5a19d89ed14c389135db17b251704567b3d9fd9a3e1590dbca28cad2b0338af1fb08b518323934f801328389a563b14150b2c81f17c6adb907f7649dab907e65750bbe2e0876ab26a36739beb8bd53963e621bf63c33e762b19190e77d04dc34b1a3e62a5fe5bf9c1ec976225bb37fdcce9d4a5d8e5340e1307fc67bdd8b9ee51e9279860f6756c4e3758dfbba314ffa576a08e7f48843ff729f12dafd1331f5bb74d94f6810aa2f663b14f704e6dc8cb9067d6202bb16f160f4e52c4d766fc09533888957922479709efb4fe24f67848880bb28e31d0560d08f32455738f6f159dccb829827172ba8f75540694610f0ab06b54e1e3713917ebcfc8b6dd3c03ae8f467c2bd15cd377e66ba58992ca6e8d280efb592e27275a86643a92b9150c52a590a98c58315b88d26dd9c06870fae77e9d77b603f57748d5b9ba0ce77d809292d4f779b42729fef5c73e863e21c033bb5e1a6967dc6f127a44a79a86747e01dd29f89cb903e464dc55076831ef321e488632964c25f718673aca900eb5ae819f01bf3ed7d8922b11dee0a711697289bfb58833c0591d653c32bb761f6ef7d17761412a27adafdc153a555c9df4414e35e2c981c7eae3eedc082441f17f66af2280c7bdb5020452eb8fa9a5384908241ddf1133e69758275f4d77885a98726d6950733eba768156c";
	cipherText = "122d0a666d02661ba8fdac82d00a10f41b9f90e6768121040e745173aa4b8942f948af263bc39d0651b3d8b5fde62bd2e821c2912128f508f830973dad702c97f131930245be1410574fe67b7581fb6bbda50e7a4136a135b1a585c15a1be3dd16d8042422ab790c675efadba0eeb30749040c20269354dd7d8cf161d3ec4cd598be6216ae6d51b87c4c4ccb8f3255ea4630d9ed8ceac45373be540e11692410231214cc99390260aa5ce8f8978b6ec5df3420ae792be2cbe87888f44f3e9ca0f0ade5221bd8f997b6f7aa16fa6bc95c322925eadf1b7a0f33e82a1c6e469475de9d5a0a22f40e5d05b1603ead0c34b0e1236fd07a491292715ccc05f308d43253215f7c2fa696eab2cf00fc5b7184d4477e9cd34ec4cd1e3dc413499c2428064e71071b8ea6bfa6a4b6a3ac485824539b2297ac6f99dbb3eb94c37e84e553153ed6f8583a155f93b8b716dcc8ff0ec39c6932caf3fd754f0aad5ede346caf5c20990bd69c63c6fb72d18a64d941397e47023e27796f57f6dc068f1f68cfe4130c5b6f4e22726b1e6887b7c4dd7266a15fccd7bc9bc1ba519bfc94e00c774f3799ab81edd6dda124c428ed90b5deadf4c32e3769ebd960acd3bc0eef538da8f0bfbf07b98245a3095634b2cf9b3a229833a45b2dc7654c3ebfc7982163a3b11e98fafa0d3c4f4ec6ad1acf03764be738c3237075c01d0846d3f37f5d77a91fef";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 425;
	dataLen = 4096;
	combinedKey = "a45578a830acde5af10dfde30ee26f48a90fbc3a664a3b38bd609ce9907215cd";
	iv = "e29f0044fcdedeeba4e363391d0f3411";
	plainText = "1c7533d2a29e970210ff0fe19f53c5ee1c886e7fd3d57673ff48ecf80a51bc12343881af8c36abcef1c4d9e17b8b9b9e9612a192ca80d282b4dc7c011895c68de1d55585c8a89692ae1245602ebaebfa08bf48f32a04deac039dd12a936562edd023ec620a7b8995bde3dad7017af91e95bc56d75a326677deb974489d973231e14b171f72741faf0e0d81f0d4401b3115a212074e81ed4f2d23ca28f441120a0ba722949cad6ff217d70946069678e0d9d95963eaec1fbb6f6ed73e9e4a5769d1b582eea067ca5e1477ed2387df393abe1b21b50c0c16b5c2d2cae817a403571d609bd9ff3b9f27526032699ef153eed55657a867552ab2fe3bf9647b6decc17d19bc177c8cfd1704aa4a46ea3a86abd761e809cd549091b4d076b90c77ab17caeff124c7b3f0b8d1a46abc08a3b71269a469553413af71648f721fe9a8d7ab71e9954a6f020f7741029b7eba962083e67ea9fb08a1f94bb3153591836f6f8b97d1572c046f8152fae78707b92313c53f7dc1250baffee716487d41e6b7058af6e37b619eb445fb305b181da3eedc432732730cb5b6abe3093a69dfeb8a34e35203d815ad751adf27f3f0cf44213e1b263bd46dd6544cfc08c98457a317541dfad23ca0765f31f341c65ad12877d961258c91e6b98d081af8c8328afe8ada99daff14748f514363dc4f8c09693abec8914ca62613ad9b6efe0f98351228ffed";
	cipherText = "bd927fb1bc46b8daa2fe17e25243e3da4388662e849793c6c26a9cced21c94d2d25aca596766267719645c3a1012fa45070d9e3848b5507eb8fb7d640793eeb8f4dd2239b65f71e912f4718af3f67aea81905aa9b46e311bb0169df163598a0eed27e16295c584d7806a1f72dce818ea433272f52a11eda30410c99da24df1fcae9755d174c614e20a965a961282ce77a804c4c0db2ee136eb2138e39ac66b2101bdde02c936affe026e8fde1f153d1b7f905be1132f53832313a816644091b16fe569dd02377fdbe54b444d2afaf2a3e7764c00d61f822cda6f117819bbfafdc6a8a8c77aa3ed94e59da59b6994ed99a70fb5015504044d8c83a221a69884ee38d7e585dc487fa4ca64610f215c3459793dcc302eba9a41a9bfcdcca2da586fc353270fe7db134ef73bdd58cb84337d961c75fb7b98f9e0035ba231b3a57def6d16ceac64dca44689f511a5324b89ed9d2455d40889ada30ea3180b5de4276aec0c0f7c61a475f40eb7e582063dd1d285463d005511c1ee3d259e543bb7b7395437d202de1ea745abd38d3a0bd7fc7cd80bd68744ac0be621dcdba84cd5d08c089b3107b86b8a64e686c90bd8c2d885dbdd94d72516177dd7ebeca2f1e0321c67aa28d855cc6883f833b1f525253aa212a4d7d080029be98d0397a32039bcf48babca271d0bb7cbad6d6f9524863925e2cb5ea66cadfce300156f3e0148dc45";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 426;
	dataLen = 4096;
	combinedKey = "7c22e1b5e33ca660bc2c834c8b65d83b802cadc4aceaf23c70dd9a880a1ae171";
	iv = "0048299d5e93bbddc4c415a6f3546f31";
	plainText = "f4bf2dade50b15d84149197077f68b9d94e2bda4e02f84e6506fb4ad27660851f93e98b6ab19c28677eb361f794ca50c5e2c02338cad9de651306ca96bdbcd972dd3e30d472bcd0b416676f80e1538a4520f7ffd1ba53c3334dd87d8a67638c30eb515a6685e50c4af04dafac045a6af062bbf5980d859506acb7a103c840da2a75b4f9bec003d7e15e56199d522a825e8e25e77a11bef7fdaea942d3dd5e2e61a41eae8d4a7ebfb5f22c2ff66e637d43fc0b0b30fe6b89f57591092c77534c90c7acae28b9a38bb8af547fea1620039a8b3a1c64c3bec21378d9800624a07e72e8b45e7c6a1e43deaaf4b2867bf7ac2fc427369c2c7691ec0e02e8b9d437a349b3647c2e96fd50e30d2255023af69350e79af05688505137e668d5812d9d3aeaa14e869581a518981f4c1d0cfca73c0482fe4b3ca3998b244adae943afcd1511d0b12d0702222c92d1161ea45e1c3161ae24bc76399c9a18cc9be0aef3d159cbba24325db06ae8b76a4ca871b2aa4d94d7c7f2986dcef78c5254ea79b6ca12158916eb63e2bb647fe44d73c133c8e4b99d72441589b40e0d4347d80924e1ed45c4e2790e2adf1022ca4f7fb8e4a09929d925e5cea336707723829b95c62fb9eca6f4e2dddda8ee77c66bd0600bee8ea732984133f6771145545d45bce22bab9a5fecf7983080c6c15dc9dd63c9a25c0fe4c902e9e22234e146a83cd4cbc4b69";
	cipherText = "8c6a9e99223dc7d8e1f06d60341e4451f54ae591ccc7dd037a7459ee0eb7f1c5f71d5e1d084ec6fe9c5bb10191b280451ecceef178829652ac540507b5b407dbce9c95198aa8caeaaaacd678cc968429421c7b21d20a0236e3f31f78d482b5bbca02a4488d1d5be355f78cc1a36c858d61d86483e7867a73866b6db858660a29f3f77349c5396d79038ca7e495b72b4ad53ac8ca0068c48ebe1c9898462b31581e2fc3cfdaf84e83b422c0d7ef60ded451f5cbfddd8b6c8db0c9f7c96cf53266957618c095aea1b620bb531c41147e63092f000a2fdfd8d2b18f614fc7af44cb29eb09215a1ecfa90deeff59f2b2dcbb908ec125e0504c12f352495748842f716932d61bed7c8419593cc2e05c9be41a7083610820af79c5bc1f168eddf830931bf5d8b1aaae2c9f013ca4f6369c8eb4bc251ada4aa63de29621c2ef2c56375bb7e67923db5b0b0377f631c4d379509c49fdb279d826424e7fc91fd2cb0ade42668a6dafbd10ee5e24f374fddecbf042b35f3c76fc6cc7ad284a1e0e0d430d1c712d68897a4503cd74bab63f285a7ea0506d0b3f0e5e6e5efea6d614d831a89b98a7518124c7640ed7ec426b92617e60977d28af9fe32776648277b57b063c0b0b59fc91b79f88d1a4981ea5c34cd3962a142504c5fc68381b0f144f04b04e2c1aaa1ddc7684feab86e9be85d5601432a3d189bfcd82c572854045780d413028";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 427;
	dataLen = 4096;
	combinedKey = "4a350aa248ba36c6f4b511917681c983057089febc45ae8ba4b0f5a95ac7784d";
	iv = "9347b99f8bb354916aac1afa1c6bcab9";
	plainText = "11fb4b70a775c70e9a44475c7445f8c1e5ea00c08f557daa243bf0d4b69d75b3ae8c8eb23bf211e8f5f56d27257424d2304c417ffdfb1f069aacbde9136a124d44f88450c0e33ecee0d96373c711d4043f322f110bbdf9ad631c6e1c459d7361efb0ca6ef119a63f412807d8fb8873ee4358590a19f95631e192ba8a86150387eb40da4105121cef7a308f3d9bce64a0f085d8260562de6a859c8c69004b9247660c204d2fdb68afead28d8f91debc41d8c3bd5bd90c50d97fb91f3da60bc716ac383b657702c6785e11fd1271f4b214e91ae26f5cdefb0e94caf95910ac0176e5c9b68c42856788990fb5d5dc4f0f1275341405dc2868adde8ae5e6f3ab9b9f06f2d3ce04a301130d15a4091202c38c01fb6cd3dcf489a7beaed908229dc9f41068d1960816893bf8f4c1b43a2b275f51ee307e911227c66e4d4decd80fe3c066bf29461b6521661b7a6116bddd43d3427086b36b17293700e5ba0226e85fc61a30d24832d1d0ce75b4c9a1b77673e548add3fff4e93ee5c313546163757eb43a6698146404316dff06e76254eff8deddd0b4dfaa2f73e073b5f0e5a291a9c36c7a51c1ff77a66037bf5203888e97d16a6bc13150ce1afaeb188dd6dfaa3a41ac921fd5e7399a089bbd2fdc686701a559c73a4cf39c745aed7091020631a00bb1b030cef852a99d40bfd18faf0de2a29ef71c6c078d6995a3ed4cbe027de2c4";
	cipherText = "d69deda0ab7506ead59ff3c57864c16ac7b2b1d3d1231002264691ad39beae4ed795273ef53e6739e9b67942506b97aae043e1468386e9507e183fb658f7f23d2dd48a53873f301bdbc423fc18b84593e649a592c8c283c67b4e79668e4dd49e55a06300aa4c2772a5a5921871b3a5dd6d63ed41ddc6472952d24de56cd5fb4608331bd6c99c4402d93a9540abbe067dccf949e42a1391ff1d1e2dd9acc8769f757bb972eedabf43aa92d59be8f8c4401b9c89938e6320352ba66aafbe2c36d5032b36f3246fa11340c13ac2424b49d8da8384b307bbe748442ff5e7afd30b7a218cba971fd10d019a97c72595eb4a5c0ca0490ad7c72bd185241d15008005fdf187a268edf0973b4323b5f29c28c416b10714cf82eedad9d2805c01552f5d1d395ea4e9adbbcd23f623f380363b6a3f43b940055a16942d7b1c090d68e9c577ebed1437bbd0bb79f49678e073e8ff1dbfd69ed267a4d6ce2bd67281171e6cc74c82218dad8032502f402fb9a5a91da2b6a71dd2081c4758f21d3fa0f4ddb4cdb03655696a9f829340d926851311d215f81b29963802113ceb3315a6054e34a44a048e86f0f4655588636c8c3e3aa8fb56a6287bced2d574692be43b8cb658ea36ea069871bfbeebe03c18a06784d751ee33c82a57c438bb687e6cd471183eb5652eaffe3cf6af829961b245c03ed146aa662874fcba2db651e079095a2b53bc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 428;
	dataLen = 4096;
	combinedKey = "53f16c985f7dd214827debd92a3ae230001e6238d855ff2bc1327b9c053d553a";
	iv = "54975bd53c5eec2e290a20208b5e7d01";
	plainText = "35f8022c86534036ef659a63adaaa7e723aee2311f6c990679b8500e551f45f0bd14fefc2be28cd936b95ef47a1b4d15cdff64b215a699856952314350c1939d6c8a2c351c2f8bd42bf5624eefc41e076b8b42991410b345c8d95de48b1504eff335c5d804e38e83ec0e714e46eff715c23479e4456e30c1d180923527743e73c34f743368c6ce7175544f3cfef02866992f53da9d3f86d4eddf9e49f6abfff590cc154660b42acbb4f915696aaf4b01714a482bdf68bf881c0b5b44aa8d134e023c7d8a862c200ab1c19bb42bc45f68c9d41c0fb447b0d4f4895ba7bcc71705b25ae68b4be810f1495fcc7e116d737e15cc2cc9312f40eab6ec6e9e6744cf67546bf02aa81834ad42ad075dd352543c2528d39ffdf320d80c91297f7457c972765ece246bc1f305df8ad0eedba32532aba6139c1c5a7d5afdcc93617ea0b2f311f0ececc71e36079972cfe4aeb659a260f9607aad2000233648d363cddda7713296c63c9515d9ab8c5c4c1b29fc4d54233d64419b839c1f39e66e9b78ee523b77a99cc333e11d02b2bd7505d67d5a2c05747597c7e9b5fb64cf431eb4815ba3687ec45b66e1e4042646e2b766a1a5b59e6b1b0c59c2484a4078244373b16f75d99807c4c2bdb096d70f092c307ce3606581e3225d98f73e5bb8570e65956e3b8266d658107af0591963e88d4db23e37ed667a2562c28d31e2e3a73200ae2a4f";
	cipherText = "fa2581ec0c551514da740413a31332a75bf414b3bf59d8bf1b06593a856a8aafdce45e36ffa77b7cbe8a56c3e6dbe5bdba502a29ede89faae621e1667a036857794c82dae098df5be3b897125c46cf6dd04fe5b9fda1e2f6cfdba7f1153d0fbc06bfa788e32fac5f952fa12e29211778d1ed19b221af139cd398b052cf0e67141c9725aab946ea819bbb8475a2103a00f8e82bf14c59083bedebf8741eed8d6c8e97794a19ece8993182ae38cd35391efdba3e255ede9787822d5db9f6ceea24b4df59c5d95c74efd8405176ae1e28125843d27b846e3f39c4f78a76fac314b7eebf2f9630d92ef6897a282daed60a0fbf271c5cb135ebf396a474bbd5aa55b282f6ba3a1767fb2f410fca981357a201de7ef4ff04111a521a63055c6c2dd98d1d319755ec4ce053b27b43daa8a7be867973abc74ad2d48880528ee94222e16e09c18036e40fda22dbfda33a6d15ba418a7563bcb7e2e72bb33b44cbd3863752b46730329debfaeb2f69a4fa7a8c7368013b18463ee964ace927742c480ddd98ff4f8934fca849ed259724db0b6802ded75ed352df867665b65154923dbbb24316b0359dcc9aae990551f4382b4bde04b9006abe447aed6bc50697565e9f66b3fe7d867964cd69d96471c4fe2277399f7a32668a828deeeefd65a87f687fd34c9fd6d97f2a85ce0173db62377895f7dc63ad625dd1aab9a81ee29eb9b05aaa97";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 429;
	dataLen = 4096;
	combinedKey = "9d844fbf707f0649914a1b9f4344c0c0806d76883f115deeaefd7c3f43c16e82";
	iv = "3b79cfef91b4d2c269606bec579e0df5";
	plainText = "388515a934af34fff3e8e1e58e609b544bae7d5864d21ed36d695f5af64738395d1d73ff48341f8d6d7e880f3636cb77b117e3aa0073bc38ce800f17cb557a9d24906fb6635dc28ceabc4bdf4fe75f8c1e31be70ca2aec17ee1560bb729069f858edf7fbd23c87f9f684fa06bb3a22dd4ad4482f4c1f3e9b1847e370d97a90f5a666a95da134392b93e56ed643a6166a2f70956074e05344e362e8aefbac4d2790c5dceeeb4c2930722fdf7efdf142770fcae8f990d43ab5cfe1f322303acaea06896b7bf453e0c0c3f821e5ca9b74ccaeb37f60f1a3aea4e0c318b6139410593381497bfbda464f200d305d6a7c40d679a556893abfe235a6063d1e2e76c7b61d99672fc47c77d17fc0b3ad2ac9fbd86a6723cadb45b2dda598078648614caa14ff263cc1232493e81c09af5eca55763c1a973e4e9360598fcc9a13835e92e70ec04025b3afdacda324086c1c6c219cbb514b5e5fdd4caee99298178dbffecbb13d5f084104c7a4c5645e554da3738fefd89753c3f2dcc3a9180884054d162e8ec43bafdebc31e94315c364de8a204de323e660451a4690780d48d3aea6e11a3feb044249c8efea79a9d59caf688c0f2a1c2b804d38d246a6883fceb3e63bc5a17fb545ceca8b8ff48cc4b38e7400069af9fa12191142181c7c2d631aee6f97cbc63a1200ccdc60987ad36011ed98c75a04d3feb6e98f5a4f02dbf452cda269";
	cipherText = "3e0ed8e484442d75d2832d809808bc21fcb1240e5fbe62e42ecc8598e31f9f50c18588a065892f83faa923540e6ba24ecad4b1d1fbf12b95c364c0aeb0a238d217f3d4da687e0910f718a7c52a8a7f7a680b8a6cdca2488981a504bd3d0f8b0795ee651ce8a63ac22b8b1e9119207644cf78598afc0920fcd477123817b54dd9e40901191670ca68d662264ac241292df92085f8b733310ea2a732fc5d5d3c4582b3ca8d63809e098d490a520718d05853240c484fbed3ec12a281ea4e4b4312902cdb3b4e368519d2182a5edee7fd9b33e2753483c47346c7e03849b0e0ae3550d090ff67988964eb4c0e7d55158f92c6a96f89bf0e8f1fa408643cc24f6f05a38fb222d6084e5e8ea95f400e89e1b0ae9e9baa1ec8def1eeedb7e661a5c89e3fc0a5005f953b0d3cd94e973f6d7fbef1ecd4dbfa6ef022734d5f56538cb89a89c2ea28edaac6e546cb58ef95ad5bfb58b520607a1ffad7bbecaed62f163db8181ec5fb5422cebde704f8fa87355151f4c2cee7c28025af43ffcd0b6def8ef7dd995eedda063e50e982f2fc1457ce0b2dcd3bce5efee2e46f71cbec6d3e05f58d7cddb767b3351538ed80f5dd604efbb31e32d9aa7d03339e92e96df875b951ee904b01ee9bc0577e0ce393d5075101734f2afc33cd52c57000ccf63d8f478634adb2c05db0983a0e14ecdb68e30079e4de2150a090ac226dc5dafc09e7ec93";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 430;
	dataLen = 4096;
	combinedKey = "a18d6744d598dd5abc0ccc6f4bdbe9155ac76fa2bbd309d8e939c1ac40346377";
	iv = "58c054fdc3e2a4d51f912d8ea8b6fded";
	plainText = "56a4726a77390bcdfe467240ad0b9185dd25d97f49ed0f8c2c939a9428aae5f1e52075cabce91602ddd5fb372b2d11265dcf50bb243c1bbe2d960a0aad0cb762d7151c4afa8a708c6d4a36ef5d8e9cb29fa6cf30ef5b7b192489c1b9739178c7468b8e50e9f7095d19afd69a6bde8f8e9dfaa2626834b0ce145907ad13364f309d7fd09d8070c21cf4ba214f1798798702af508f9ab6164ee6158a06c9a04b17a01f705ec7b6e3a4a4ef00323dc88d2b2d8152224ac745808f1b283d139d926f59914a1000153ea12c7a52bd6e69253dd24f58b219b3d0f7411cdfc2bc051ce711d49659836788d7c8b911c7266b8d26582311bea44e9abdde020fbf227566de17481b5c0606a7dde12982d6c24e28cf27dc034e80479cde00a4c9552f14596b250fcf29b8673167da24226709d219d3f5d0624279894346eb170a36e674db2fe174bb7dd7b45f2fea242bd202bce9525207059ece9afb1bb390087abe85c111e4f9939dd857e09eda2324333b08ad2833d92273d411019fa2021f0aefcad135c435d5e44c638ed946197b2d78e87fb4823429b2ac0485a6ffd024d191af9a227623f69728227a9e2f6758a25df8b8dabdb36ead58507b9e356a2d55a1f0bb3899efb883f7b0980f131a1c3fa82a96f3c515f905b69ba46e9ded9736fe713cb654ec66b731faeb024530e45663e10eb288887b9dcc8330e53b3459d99ec3fd92";
	cipherText = "bb975061188941dd7496b59d3734379b8ade1c853fe73134510a4750277ad6078163b556402f87423ca49da2b3e4548dc8627c56a55005566777391a33777cf17fce110790ef879d857331a86785a65326180d5445b8b0d4185ff32b6d329bd8cc5a3418510ca4d97bb7f2dd0d4489efab2e6e9bd14fadbb5a5ef4c8a8cd642c77e945952c11db521705c5b184db3688b9d38567f9c6d56d3111a44a9a241bd920ba24f9e9f933fe1b1722b0b4c26be3ef7ba485f8bb5c4c8a584659a8eea225edfe26da0266714406fb89bbfe32b1d7ba8ce7598f1dd94f998c063753b94eb4705c3e0cd731cdc33c508c7d7f1f8248005ca2ed1a7f3c84cf67454699755ffb0b9682bd42bc614b315cd5d66824861c18455bb102f64151bfaf6ee683a7d38d84a62a9222ed4dabf17cae70767eccf24c8f2efee159db01e3c7e551a2225049e19d1131ef33bff8b8ab721f8fd14fc5bdada1f917840dfc901d632cc3962f78bbc134a10c05bf9cf4f4ddd8e521891c0c2e22fe2259e11241fcbc7d0b08a240c3d91c413d2b93bab4d09186721cda45c471a444527d7dbcef80734742c915dac117fa69102fe23fe23047d54b3f16fc92a8734d7611b636d1756b07ec1d3bd20b4e184406819c86cf0a3e7031a5a92fc52ec174723a0b6a28aba15993e50f300a6bc878dae3eeb16df05c89ef3d1f9fb36004c1cdc62b428743b90bb4598772";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 431;
	dataLen = 4096;
	combinedKey = "373b660406247b674fdc0d7e73c6f605002c45c6054d229f35c45bfd8554d9d3";
	iv = "0c7cbb5a2528bba11ec4f17d7d625e1e";
	plainText = "6be050fe1b5f72a4479daeaa920bbc5066f04c9c5e4c13aa807ea1f05044887021da615ce55155ae56aa9e3aa464b56d7875f5e53d7b30a02bcdda770a6bb2704e3c3e58581845727a390581f34056bfb81b7d94bf9fc2d23fdd9a85e057258bb866044faab7a34e18a9969a715db045c05cac5b604bfba3f8f56c563804a7950d1d3d594cb3c54a903f0eb8ae53d6588af7916d57300d3ba618f6d9f8181d7aad3a5f98f7b7a9d882cdd7706bb8ffd90ed9a406e273c1d1a62f5168e2b216b73e24b7221f90c92fab0ab957a09845f069de1d92e5a814309ad356d749f619f125321cd7be1b09655dd05f59efe0b36745beeb605763130598a88108a233f6bd6b4fb1ef25354197913589690b3db538099e47665309fe31ac6ef777c9745378c2fd4cc602534214e38512d92673d957e987cb610b1716202c0f8c2df7dd8cd8c0345b7be29c66f1b7e3119bb1335a952d5b33a0efc90d1120f1af861432f7c1c2b629a55cc3bd251e9e66aef96f3ba4c264be8137203946a316cd0ce805c39a349a1e1295f79b003bb8d31212a01f2f987dd564a38a7f467394d630010bdec52fb0729aba2d7957ccb9e938c836098464b9aa5e22fc1b7cc8317a72c4681623c3a1c2a36e56a9d11e32d42a964925642738b6142f36e8d5d3ea8e73aba227e9ec480d211a04afc71ef428bf11ab8f6546032eeb292df70f229d2eeb579382e4";
	cipherText = "9c2f5b8b72ff768bf37cee01f4c9ecd6f69fcc8864f9b56bc771daaf3a954a4f40fdabb5381273180334f70201aadab7f740da5a1406770fd220adc472b5847e4e5dc82d65ad554675b01cfd6c47c3eb480d13a4b85175e417c0c4de2ffb9e259121fcb0baa64cd676646751b25b3661cb6a3425b0d744945ea6ff559d74af5e92353b92352094eab723ce26533323175d73549497c41fe41c9c6416785497b04f92c5e4b0795aaa4d34d3ac08721044824ddf22a5eef1ca96ba1c3ac1e94809ea9b294968557f364c74a4f6dc0c3c6356381aaac6da5997255350ef648c1b7e526eef40e38707b5d08bdb7961d0ed0b80d67eae0a43175e059b6e5a7d47bbaf72b3fbf7d6c01341f0b509deeed5bbd04b229e6cfaf6da903b07276e93e8afc75e9da2e680f36e40ae704327c9a02ad4c16b9d1483cf949595e459404b21eee7ade818334b0fefd9eea68bebb79cd2f78bba0bf437b7c18c56ad1edee1f43b7b33516d986bfbf77feb4dcac518c5d58fa775bbb03ddd225930eddf69f6b5f9d88239332948885a6e5591c10f0230de9d7b6676610ef08bbb257b6814246bbfa680e73baae663171ec162f331df68be9cbc195c7948c96b33ae3a6bfd133c2d42afab998f44b73fe37e1bfd6fc47309d2dc9219baee77cc8b2b431e1fb3e8f85d7d6ee50caac41b36cfb5419cbcaa85bdd2696c4bc4043271cfe393765af3f931";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 432;
	dataLen = 4096;
	combinedKey = "66db75a1732442ccfd7c3dde1167ab2fcf92dad59f2b0f6cf153aa8cb7dca0b4";
	iv = "c1e92dec7e50e21c1a3ed9a842c479a9";
	plainText = "9fa17d7d5ec3a9cea1f7278e6cfc95146520c04fa905b5d21cb6738488457179359bb515be5303edb596811fce830a29cf646faa80999c6d28b994fcb3608340e2bf0c03db855b5662c405d2eed5a2fce332ad65551ff1ac39b0caa578bb58a48cf4dc1f5f3eda634726a877d9cef7f9bfcf1fa594314e78ec737dde24150f7f0b0f98b44ea88257da9a22607d4a2a89f4b052816ef64187740a34e50d2235bb67cb17f94caa4d67f3d5baace16f97a6df195613fc2ba6baa2f4fac281765429f127de0b2779436769522e07df597411e405d4ffcf693e2009574e8df84d9b1f9176e0cfd8f7ae0e362252786687a536e4cebc0a50857cc846fe9494d40e390938a9328ee542c68a3606702cf5bfd5dabfdeff434a77e32ecaa95e997914b19ee75a9cea2963b16806739301ec4d271dbe7d9a9b677b52c3a8cff6bef3273d048c1f53b42c4da269cc4afd91bae7e2f2257e71188901cacec2482d52b8b537a24d3745ad5fc4464fddbc9a48043a71795cb899ab4b936ac39a330a6c556785b87656ef234a6bb9fea60ec1f381d4c89f5df7ed931da6ca2012e87ab93ad224757fe1d8ed4b730cd71e75487bb5b40648e73daa0b8eb26061e33e99e6af8d73b10b68e970b7c2bac106e677c9abfcb03551a77ca6a737b5e87c80a4255e3f580e2e6bdae2f508df3aff512772ee9398f81cf5ccec868aede5377cc29e5aa7d568";
	cipherText = "d3faa6163310d7d7a66d05659619eacfdb26ed435abb75f050c86b43fe31b2cbd1a468ff123f13ec21984410a4cd317ae912ee50951b93a7dcfe6e78cb093228beaece1c3fb47ec7ffbb2f859fe3991580bb11611d8628407f24ff3c9d278be5baf5fc5297eb0fb8dd6f6a33f1fff436c00b3ce490504d77fa01baeeb8a73f759e7f98ca6db79610cae960f6c3248fe5d754babd620c47c2d101a7547bf485704fedfed2c642de93bc05528015b288d946359200811afa7c91c14e65a647da23127afee1403f8911329eac6a76bcbdc322bba99768d1b2ffc057c927f40e49fafc7f1f933413cfc182a9170bd0cffcfc6496c01afc6ff45e2d21444aa4f8623717edf595413a2e3b456503081706186ab65b28d8bbbfb47c89bd51ba3661eafe0d764ea592ecdeda69e3bf7b6c423d1e865d4f10d46d37be3ee49d8e7a8c62bdebed6a1bcc6850a686923292ce89d6483f0ca92960d010316dd2677312ea6c7c530f650f9c6cd9938c09d8a772433c2f6c74bf2bfb65f0ce454e675ecf4dda368d187a5494e128f232cacabee607a2af30aa564b6dd99b3ac9c97c6a61623e64453291c8f771b1f0b54c68dc4423da3c76eba8617e74b705fd1a8d422563b76ebe28eeb06a9e0e43b52b13f957b3fd0c50541140f51f3e00cff1f8932eb88c5618e4240d72bbf0ba71f8588c5c41ea1b6d6e6ced84567f2a8a937478572d117c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 433;
	dataLen = 4096;
	combinedKey = "d68ff4499838a47c4299a9074c42a314a0f71c3553d300a18c525d519f189828";
	iv = "d3303f38e42e44771106dea249fd5075";
	plainText = "044be5ab8ee320c5a8e3176c2c570bbea6e902ba08c455f9a1e142ea1a4581b312d4e987be387ef0ea20b7449aba263315fdecfa90144e04fada8ff9b520d1d769d9af109bd943be6d6eadb89c6079ca34f762c8bbf31d9807b77d8595125b795cd835823ba0407590bd71c3574e17e36ae094e97a8d17916c6b27d452a2918b1a79c7307cc62fbaad4d4acd5781c5508dc8333f2bdc7910eaa0fd1afb0114f2e8a7401850df8e0a99e962a2f2dd17d7492f95208215840b610cfeceb958c03c00156c2faa481c8fb6693f3c1780e67d5d5cddc0b102c7c4574492facdc85217ffa0fa3871fbf941062e7befa5c423222ed3fde6cba15fa4ba5d245075e2688597516e38d477c1971744fb6a47781da13844fdae1b8bd6642fe379dad3cf524981cbc602f807d55bf679c386619123412718d223db92f88738dfd3d93d065d329eef9249d93606c532fbf9ffd6af98b24529c152b3783d1ebd259a3e8c87caa4eb5e4378d5c808be1346eee78b8514d6ae8a382a81fc6112201260647d0cef4ff74f4f2070e81da1e644850ce74b40040792c89560e023e8a2dcfbc9342e8466c7b393901a08063056982306518b3e736012bd8569a31d71c194c8007749e8346c61f5d6eed25a68b906e20aa1a422c0fe2179018ab86413e2f0686999377d157a4544a6fd4d9b2d41d91e176a63a327636462aec3db82cd0810d68fd0fc8cff";
	cipherText = "037f69f8ce3cb419f5d86852cf5dc8dabdca4ca091f5c56e516ca1470a65e2d91161086c355e993e464f5c32985e34b586b0dfb265c77a1a09225c427ca61ce51ae32f4e70f6a253ad6e50f9b7a67228ab8ad126d1c6ad23e20b8c56ffb6279f6b37ca05e0a8012bee65a50b630e7eafdc5fa87f77c9561ab983efe04a79c6baa30f8ab48941744abcda4848a693cb7a6235ae2b8e09fb3f95b56d79df37ed859f645fad4836022dfcd8283631ba6de309dada889f334ac5a370a524568d75bf66d48e3f5f845bac72f8d06d6cf0079d8d4f35247a03b905003967ffb809386cdeee764e8e56a096c317e4def5fc6418e32c228b8c89fb4c3e96cdb3f0a25324f04e5c3122aa6a9e5218038fad495198ed9b14bfb2ca176c790f2712c8425a883e176887e6e6f75aaeeaf95e1b02b36989f9bb35d7a0a0d792001513a8e51445448c2f81e5c50db570ab326f0508fb682c599f78c049e4098c73b795f961dd8fe119967d831e134b35fde463a5df662101ec63ca09cc53ec990bf497cb7ecbb068788e95078d57af923bbcf3639fe1e24c99dd7e51076cc9fff095e414b5821776245709bab341b4911361ea331b4d224378fda692a9378be2c12d0fb6da09dc38eb7b5fa180ab28840cf40e350345e028e684280921d7bd85eb0f281fe68e9518605415dad83a18161aa6658ace328e5eb3137445b7182c8c98c08c62f5379a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 434;
	dataLen = 4096;
	combinedKey = "015142949fe4bb06046f36f84dbf899c222448d201fe223b7f37e70bacba5732";
	iv = "1df3e9bad4760ad3edde70b074e9957d";
	plainText = "1a52a6809a00d0576fb0e2fd570f162f763364df3c6cd5ce16e4b8f3e28cea3deb4d08ef58abe81d3133a809fa6a7193ac4f9cb0caa7c68c5c0813fd910c52570fec6b4cf3b79850e2fc58c4302ffb146bb6ce79f17009be6291ed514b47eb06ca0f6e94eec1ebac8fef649a4aa536e637aef288a701b9b3b70cc0e573ea34585ba87a28de79348203c1fc5132cfbf85d2047911bbbc97bf5015e3c2666d7567ef11adf43e069844aa3b1b2672e14052c1506ccdc414f790262d4f9254a3509d763b6a316a215919d9daebec75c7082369090466fe34a94ce03b156806575b925589633bc14751ce118007397f4d34fcb8eaa8e17ffe20c93394c33aab8db8a6b62e19bceaa5dd105f23b49afacdffb803a325e6cd84814ecbc063e1514fbeafd6d98b1d380c82e00f458606d3aac2ee48e983556a8d579b1e28ac2600a982bd3fe191ac6f5c238ac307ad4a5f3187d2502a41b5a4db9c75424fed463d8b80cea6cdd3fa9b4ccf47988559c0dd4c1f8c51520dbbb5d0f7db33072e957ec612ea2becbfeffa1c348b181c4c6338a03cf94699cd785d03f4128f5bab756def48e343ffec37a547cf692948c00a02b28f12d54055b32ea6ec7c3995015650dfc26769db9e93f36ea07169325488111344384ef89b0b693a5f5bc3b41670b608eb173820e6f54db292e0baec70901e1f936befe940418c7fd7ab90412691da2caf4e";
	cipherText = "55b32497e377ca6d717ea8280d5e0adc1c542baa52f44e00d6b542c9c9bad6944c7de8ea843bc49d52edf08a622a079eb55fba118c4ef0ea88b64af7a425a746df279624ce291beaaddf7fb732578ef6dc49e2bb5eb2959433df0c9f4ec27f8ecfd53f79bd2467f51cf91ab09ade12d13fe6c1e1143a6f885eaeb33c75f13a8a1f44509109316653397dda919daf92f29618df1200e20ca6fcd005de6df555485f51fe2955eacd899bb8d6809df6811d2a9553926f05358d3111d9b47cd8ff668a20784d5d6f72c2b125761d1e6af120c75d31bc3ba347a7264e02ee60aa831ab46edc8eaddcee8ca1d73baa6716b0999fc8f1c24be7495f114a16f66e8118f9d2b5569b3df7bbb063229beae67892f4810a41be3c90ead43d97302c5dc4e7532ef20c032c3b770b1d5bd35921836632337fe41ccc53d7c50d9677da04ca5f22848b635a01860c5c6d023b00988948acbe9f7b6f084256e613c443081a41c961d750d1646abdfb072b4678c0a63bb9b27f4104bae537ecf23d9b23363d0b5fce1b04dcc663ee3108963f477afe0cdeee4aefacdb504cc50c9aa86759d7f1094ca34b259d2380ba29e22fe8ed79e5892c12f3fee85f557fa68f16567cba3357b9dc1fc3c15a51b12b33f500bf4fea3d4fdf5d17a4b2ebfaf239a34f2b58742b74a83057dcb50b544ee9bd5f4e8fef19c2a053ccd87ed86b04f6c6d399e3861e77";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 435;
	dataLen = 4096;
	combinedKey = "fdb73e9aa1f85e351c8d3be49690567f1fa6a0e3681cb43ed9e52e342e7075c8";
	iv = "df9fba1b49b253f62bff31bf1781df58";
	plainText = "018fc8b5eb1a1d512d1dc6efcc04f9cd5d7165606c16540720726f74d4093fde67e8cf7dd093bab0e3ac90256b78a3df1206b7b709d6403303d6267b0ec737a1ebc9d95bccf225001ffbb2384fdf84136c06be23f304e6011667ebee6ab74e62ed4686ee6d478323f6df1f7fff1375a206b0254dd80afe4ecafca9b119b8070f39f48b6bad2823dd440221328a7b132f93825b70a011b396cd873de827ce058b71afc342bb933e87c926a420c4acdd56eae672e437bcef47e8e3512bb6ee90350ea0e4ac8192f290f4b9d1d4490a5d144f7f963cb37b88257c2d59f613264a8f4326571eea4e3689c4534509ce79a9e5a2cfef645373f5b5df351fee9736d8b885d9adcf38a76b524b86b96b49a81712ed3fc4d885c9481c2cf52e4379e4c8f0eb78d8ba53fdfe7a58baaec26e382056136786dd8484c0fdd19b6b72d6729f38fad5830814aef18f56fad548f61bd0f1cf56b2dc9f655161eac423bc5346467b7a19621c6a2dd047ac90e4b5b8e1cae6308401767911e2d326d22d8cbb603f19b0048c85cdd1dbc187c208acc662e24140bec79e7a3acb0126f7cbf9860d739a4adf61abe5b3331a57e6615b4a2c1dc00d4159e98357b8e21b731aef40f41551aa62c8a76bfd188a838d0df46dddc8dea96395a627cbd4ff06b66a2dd5d141c110f383f00561e35d646e6a1558b10c849592594782d11385a933614bc271a365";
	cipherText = "231fdabfea830c6d5bdac1395e69fafb9c733ce15e3600babae22657fb5a79155eac047c4e87fb80b0402d4d489cb41b8c5dcdc48dcdebe3f52038cee4becd2adcb71b6c82d616ae309fcc6e513eee49a1390b7ffadb984f17e5994ee70cd9182635120968f5f6d35b9e2d2c7a22eb0d4f6ea18fcb498a59f78b7ef6f52b66f37bcafed3faa66dc2fd1d2667db6249dbb0c584b514d3d209e47e93b92b3ddff1a2eeef7127408938159e32141d1e3255fa644e049c0c9e698acdb4f1b799689593af3d71d8dd1e6db68447c95fd0700833ac23b168165074cf2a63629f09dbb1a4b717b658922207d6f76c0d29b2b23d659b709b38a110e1383ccd436a3f50bd1c6561422d02ed8dd343be646edc29c24cb6a8e06added3030a6585bde58ef85f8c14b6d9033551a02d8ccc8d64190abfbe3029e29bd1e16eb522472a91ff113a389e2cb3fa4acbfaf91a537b96f5a1ac08d2ce76a1b0eec21260cecf367b2a90819626a1973c297530215b55144d7c707003c7610ddf92c325ff3accf2c9c77f4f2f01b7d88d26cbc3c7da9c6c4dcb946bcd1dc38f158da56fb41ebe4f323090f6a321f7937edfa5b29fc05c8b6c988b318bba45e391d9e59f456075dba967ff48166567fa65ed816ba21eb581626c09dc529c4759159659a09e35023452c75d72f1fc8fd6a8bd648c9b7d14d02f5c6a51999cb0e9bdf632619275a37f0d41a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 436;
	dataLen = 4096;
	combinedKey = "b37bececece411f62deab2fef5af60538c5060d7cf8b2c91fb1fd68c97a2d37c";
	iv = "7c3fd5b917be953c5064dbcba13a2561";
	plainText = "ad2694e79fda5ad122a5722c78f4c2f32376619ca15a67b43790ca65e9e8578523247c66ad2a030f7d8224a1fa283ca11994148d399f75d6fd103133d9677a62bc186b416881c10b990fad3648a731e0ba3d4720d155e146f551cb3a503c894b7512c8e9b4c566c35ef968237cfe2cac43960a8110d5753dec12baa4d04ef889b532a69792725bfce08d27d16dfa1c14a5c9a81d96dc4243de35e1cc3cb7ff54868a0fec480619c45ced03fdd6379a6e13039fdf02fc91299e49a5e9993a4ef81ac9b7e37de0eca3fb2d8105d41aabafc01501e255e144e656f60ede1ee5b7f79e2d6a3c3edb7c75fb8ab946c1946b8d6bd68ced53d929576b7fb48958cec96b5483ee9382675377544b2d112255407c67daf01011d44818bdae963938d59bfd3d722c6e8118fb9ca019bb29d1b94c7928f0cda60cae6471ff9c3fd9ecf44c9cfbfc8998485dcc258d15b88644c42e68c95115a24cfcc587da86a7b3d2e6e897d1ee67399977deb86710f7b2217b7129b2ffb99a89ee4ec52d8234ba35b5efc0f10e59d0a09402b1b76034ac93af4e68802ed198f3eb622ae479bf2481abff60490b645200a19548a6514c697622087b084634f0204d28285067f394e053bf7312af6221cdd711cf28d0a3194da994aa6d5852ed1c60ee1a4ada2a06dfc9c0199ed5d27f9b31796d132897f338eb3070e737135c00e40f09e13fe059525dbd25";
	cipherText = "c07e7e1a92d3f4864ed67d342df1eecde1bf28578991a78538fbd697537ed5312efda60c7dec2808f6c83eea492a8700595c3636a84176d654a69a4bb38010d58f148ea773b95d7ab2ee50482c231859b069802642d53fffa18709a0648f816cd63eb6204ca904c5016ab91c0626b505778150d92bd8af1dbc35206d715ee0c6f06eda7a614bee12e8309f356e6d3e5d012ced3e21ac3ede58fb76406ec45b5bb80fba80bc7a6b849426b398b0200c7c5aabf088cf2d96cd0a2978f47386261812fda53a752d59cd0a9ce9520618239063ec309e9a093035d1aae69a6a24bddd4d9a0705059d9f7334c8125adf29cf140ed0c32a7f434a1bac3aedea5afcabd4f62e89e1cf0257671cddc133f0be72a9b0087eec85110e015d7f4bb9fa28c4fc8147618b95e36d4e394432c0e58b6b660c0ddb1e6a497987bb831e765a21e84a35d13746895669c792c986562f96db748849d0d82a706db051c5fcfab1464aeaeb7d823e3823231a4b255e3c82df400232b8646e3bf9ef08638e1ebee7de4c92a1b6be9be145286e4440a718879df35c6f6e543b4722ab1bf5dbba0b503ec0a16be8d7082310962738cb817f791bbd63c843176d26b0f34573aefead7fc153ace8d8573a01b8f0a8e7432ef9a65c7ea02195a96892613abe8946b8bc76aee2b58ba44c258bc8cd528dcdf6e1512ef8cec2a93c5f947a1334012de912aa5c44a5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 437;
	dataLen = 4096;
	combinedKey = "cb48acdad38973f5c581d1d660a663a2adfa8cd55fb252da7949778a477db854";
	iv = "6f071e6600adec1acc2be6634ab537ee";
	plainText = "0546905429ee77b11f2ef1bb8df58afe20bb8627624f498e119ea58676d9c72d0b74ab30a516ab57f85dbcbfb1203bf76330783b2d79248e34fa5101636b959cb3457d19216f066b76bb2a2d792b1c452d9680b02b0c27d62e8169e80d46bb1103db9e9bba856bce9663dc25e61f9ad952e2cf6c9b65c98aaa888e6976ea0af5e3c2e0d24ac99ec3d0226128f872f19ae50b8842412b30e086d7a86182ebd621fb0b7e45d9e867a663b9a5b4c6190040f99f034001e12e3b127c979bc158895f44dba6d0bd53ac7eff5bf7d93cb50181c401c4e75b9c5b7d8c27c4905be1a349b6f0ff03808d820ea61fb6bc570f7f50af1717ab94319543c348b47fc9a0c246549a2e37cd52439fea7edfb741d4d74f24580ab7b383ca4d42d712ccd32a57745fefc735973ccd84378900b281e2a73f338ac823d1e498eee24c49935600da82b7ec788e93a95ba30db4eac6c4c8dfebbb4d1f76bf9de72e525485a49d34f7f60a05ef245184acb0b37ecb07fa90ca51e2346b3d5c38d51f21d3e4067c94cde9c4eea939b17efa8c49fe1c00e15dc7e2291256260064b5851c717d84eea539b20cac60769fa5ed79cb5520984584bc7df6d18334216f1c5db03a7735e8c658218f2935e3ab83bc6d1bbf610c27f08cfd464b464a379ce6a1612320c9d27f7712e4471f56745a960c23004de80df27f28216c973623df0e8a9d8ca3b8e30a5faa";
	cipherText = "0b3a0b4b3c1921b16b459d788270e14fb2ed2534a0bffda77798b1c1ce01d96d058e92099ab3361c63d456d00338ee3c3af1074a95ffa3e9843716a4b322e9bbcb554f0247908e49e5a522af2fb01a2ba56e6bf06745733d7a4e11f411b6fe459b84a3b25a0b78bb6f3be73c9d69381133dfe6d8fd6bedbd8400932f696a2e057f1410ffde3d732f7ab07375e648c36110ed0bbf0807bf291124337c72225dd6b42cc7968d7d88a3293bd01ecbc0a83ced66493af8b69d5e57ebdf6d260fd4574718c12931856d61ba8f80b2568186459c311fff824ba7e07c26b6d6d8302a9b72cdbdb3711dc523daaa2219949fc41fca0dc331b5d8d157569a884da6978627062cb04337bdfaf7c1782b53f1ac70cd1935bfd9e159e82da4310f89c0982a85fdbe26e07001d02242de6db020ace560a16903f275809c6bec49dd6ca305bcb56ba69cff653577def24e9b56772a0f50a3b4de52d5611c8555fe2b04b5d34de0420896fb1d9abba30ee58a07b323c9a9e01a9a580b81eca481e029097d713cd2b34825bac8f48b2d54333c9d030305b351af4f091360876d51e484f8830557997d18f358fd4f42b84cd8df2a7a1e088201361cafad33ea6f7ecdbafd44bbfdf4bf5adfdf4c41893463b04fde7de8a6aa010fa7985e36d74f22a1117b3558c81ac87d2afb53405cf32f6724a86ed8dde7a4d7f85e902d590aa853da096f169d22";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 438;
	dataLen = 4096;
	combinedKey = "4be787d53b2dfc4cf24c6213315b931c9f2cfe0a7a407bfd3283c51fd3e24c8f";
	iv = "83b66672148fa3b086c237387459b596";
	plainText = "260ccc4b46a4982bc4dfd630a1ef90481505e26a1347f92655846fc1417a79f98e7a26d817de2069f8997f194832cdf046cdf9de7b43dc69749c70d3ece9a2be75781fae957ad589c85840a5a87b5f77893528c5eff528d96a88974fd076a27fd76339744a1779765e1d649f08aebcda1e88ef818d89d3b54b7b18ca05a6cf50d166a25be5dbc3ec45828fbf7e251fa95524017e5bc37286e942fab6957de7d8b331be1788546234e1b88e386a4b28303e49c2e99aa86c30aa221953b89a4c80f8a1a98dc4f5b23bd7a2cc255458067901e72d03d08750072d75db390c96f6cac1171d6928d078fdb156a0f9f5a99aee6a6246033bd82a7f03b11e058812b6882b7f6b18ef472ae8285b41227bb297fa67d6f8b67d5f025935966214330f9f6aadd86db144ff21412df491b4e2565fa7703696a7406cc68fe3501ee94830c8e4bf3cb0f960dc223bbc4b2eb339a20ae3f6b678c6c40dc42d340092f4e817b6b5c175609f9f79b16733ce99e025c0eca777ac60fb7fbd3ade3376efbc465bca876499619c951b6e4682b7e57f7e0469f01e5492766c11a63b3127b145c68fa5c56324bac906ebd7be7deb565c7e1ac107fa454714c094af9c1902634f22f69ef778b515dc01619ff7a3a8e37f54da06ef4898f81f6cd0c7e7157a9c15b78d6ed0c501cd575005eda5e38bfeed1ea05b3eb0be15195186bb4fadd5776292ef0593";
	cipherText = "7e14cbc35c0f637b5b200293349ecc3e327d3985c5d188260ace9561cc5c2134373e217351f50153b59513be6fe43f1f8da4d1d13150bab9c133a335bbdc8252d989e4dc3d5584625d2379e6eed436ea42837552d1370d03cd2a10a5647ebb4b87af78056e914f5213cf18e12950b22824f8225f2d0c510359ce12e64b25f335fe14b10a10dcbb9cb99ad0b9f6b9a7dc72a7f3b621ab2307946bad071f627d42e2809d6102597f5581e2b1c28a9e43c2faf78f30a3d76daacb039260bda8075c885cb4ca5f2d81c6a73433d35af34f687b1a890a8bd1dbf375a6d23fd392c1aa18f96f6c42211c98aa8cd420ced38e62aa56ca2246b0bae0d20e3dad94bb6817a6345521fef8e5bdf7f2061ab61e7344a89c61d9267565a025b11909e43cf94fc7e67ae559c3d834ec45156afcd5fe292d846f166e259a3049749a350ed8c91be0ae253b1dcf5c087178b2721842c7606a4bf8ae9522b9f83e626a36b0d9f4ace36dc71a8319f4ccb5c7809a2e8993b578524890f0c9d378337af664b62860f6bffe4ffe3260d960d328b622bfe4efb995c05a2f4bfc08d6520d2635b9bf4cdc5eeaf151dabf31f95261020ea46e424c9561d9670c6edd6b245873df3037f6fb91d5db04c50f43460e81166133588d05c38b232d84e8e8d74a76f1800748e983cbe837d5374a306f72d411fd5a3689dfe982db87a3456419565e9aca5d19cccf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 439;
	dataLen = 4096;
	combinedKey = "08f8850ad90db1951979b62acac4f85235ea05447ecf9d1f1c41987459c6dc1a";
	iv = "3428c435ffc654c45132428c8ed10281";
	plainText = "304139f3de6cbcf69fd1199efa11d1df665a4020cd7d40b3c199e724476aeb4f3ef67335ad3d14e48749f4dc0dcdd31dccdbb9479e9c6e1c4092b50e4e47082a07e20f88b225461c8cbef838848e2f8e3c7529c3eaa28bc617723ebeecd09bde3c393c493e7a822bc0ba082cb69282c9ecc86a5d844679da47ce8d752fad9ab212235808446a2cf498c4d5bbf3bf203ab17c5a93eb527b2e6767615033e3c72ca4ac4427446f5d61a903d633b0137e0edf4f95dcf1dc5961bce2ea120da1d7ff9cc3f3d0c5fe9dda78eacf10d87eac604b49cd9e783e138f817cc5ff550f17e11e9a56bf31617c6757f96916ff620bf8ee558b73ba752fb26e065076a320a68ba617922cc0f78efb3f2e8e25136a50b1235140e0a2ce2ff27464640629cd26ee6c963291cc8aff57cd9d101fb73afe22f38b92bea99f671bb23310695249e9ec7588c69fd845ef86e0f772ddfb1fafab9ddd2bf16231b8f93624706bb6b2a437c026b6fd2c44e735e9947c911f14d044efea856da1515af47ebf80e98ad724a677370b69e4cfe0221831a265fdddbff85e78db2196a7ae687173d63a6105d36bec9a738cd3b3abb6caae644bb938a8de536870add042753f1508cb824f45177f961e7cfb9c31abdb51bdc6e03a196a11aed8c5deb80c404d9d57a3bfc36c9733b93300df72310833b4768fb75e4fdcfafbc4a19c4f04d6cbf6cbe242551580b9";
	cipherText = "65bf012acedcd927407bedde0b86e59eb87a46374c5e783813a1eab7b9486b8c547e0c564e1145094948a4ba627c0a14d373ba33d5388ab6c5f7be6639008a6252d4ada41d207c12a16d3eea968d889d12a1bbab3fe11fcea6810d15d36172e06184ec4dfdbbf6ce9a4c751837a044fcbd1656267e5f0e2d3139ec0967445475e230591f1d151e40db2d5586092bdd2325148cd607e4ff94d212544318d173721202cf4109c426ca074c2ccb13dc9d81ab587091189191619b9b26954505bd362993b546f0436d2032d277e26d318ab0707362f9db69c10ca864b93a0a98da7a421b53b7649bbe907b35ba1c5cfba9f4fab7ee8f6a3c4d9abc3478ad1182fb1ab9126bc64705a40f8d932adc2bad39a5a9bb5d746952638e9edacb0efd6551dcba4935190a81af1d917705cd822dc58ff7cdf28d60fccd942b564af3b461c7896a7b61efd4af092c8e03498464f2094596e79120a148f9e219f43d6c16207db8976d9967f4fe84229703e3805a331fb72a7885a5e2ce75ca8e9784ddff6e55890bc1a2a45e6c688c03f31166ffa1ecb185923a9dafa2037b8fa078d7119aadcc2a23b08ee3663fb1a32dba1ef1ba964b5fb2e23565ae3764c9fb7996107695589b5419cfffa97b27b688cd2553a12cd3b5969b434546871cd56db7d72203a936b61e68a3943b40e38aefacaa2307a8e9efc1dceb1dd42f4116bed07fb61ef8eb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 440;
	dataLen = 4096;
	combinedKey = "e29e9af7690c5338e1c5dda5218893198c74512162f49fb18b14efa44a231c48";
	iv = "24cfba144cb3bf9c75ab6af92eb787cf";
	plainText = "1701bce6a9a11658a412bd4ab833d017af92fb694231c54f95e6f05c072ef8d0897e685e8a99cb496c06bc2821a72c27ceea5b1b82e90da31234e71b915cdc2f53731351f2a51f695d6e9b0af3475d768ddf8a568fe8583f51cd061a58cbfba678e77b99fdbfa6ef2a627114fd6fc100fa7dd398384586584eccec2056b1ad0acdad9be1b1a3065d64e6c14eb0168ea7d5d1ecf45b8ad98f4d245055e1023d7efb1b14f00b699298df30e6e2af0d67adc202a3dff5269b0e03b5dd3ba649f0cb740ed2b6bc0d1183117c2b0a0ac7918e238dcb91575b87ced7d79119a93b242af65f9a5cec4c9cb3fabe9ebf302819d1cdd86bf4a2d7264976ae96222c0d92957bb250cfdd0ae26ea6b1ea86fcec63582643ec8eebb223b5b9fcee680c92007ff31c4251b3a4020a912708106e490c9b54f53f62a2e4fe410f08b8c28a02f709c7f9b65672cf366187507a01766214329131f3f47bb563f97cad90dfd459a5bcb9bba9492ce698169c8212aff8386185d953eb954f32b3a52cdb9be3e36bd342a5d83e173261ce94a2bd8a6f5fbed4d98a8768ae3a77cff80250fc392559ecef23457fcae41fff2aa2f82a6827d11a2799c7af1f23e35ab2967f55b27d23640ea8504c7759c4bb4288b6b9000f78ea8882d89075e3604e5f40d9636ce6e5b6001813fee88836cc5fb9f93d3156ccc3b6988c929da32030c05eff2d122a4ce8bc";
	cipherText = "e2623526ebd1758dbadd4ab84ff9958ad337b9a3935ab36b23d4879977c87921bbc998c6df655aadcf388e93699094fca9bdbb504150e959e8eba1b7630ffed063c389808a2d441c3b3864e8ba1594c037218b4e4b8cfba934bf15858649d97a71ffe889e6716abaf7c7a904e346813b8310580ce80cfeb79326d1708cbfe1bfeb7417376c3c349698bbf4936aed294b42865b27e37594c5df0cb3c3c8dbc6dc406bdd65d4bd565b94cdff23814c444f4bebc229da23e8407b749899e85648d57e15f7c7d9d88c1feeab6df80d9b3a79a3aaa6eb79ae39a44c31064ba00e271e532b27d1850700342bcfac53a84df31346f72f0e22de4babe81115332c851827358198bbeb2b3af1eaf4f8c9a91db4f0f274a5f54f547395e239966566beeae969edbef0f56f6dc84a5d98668c128c3804adf2da4d90e3daad954f13d138fbf0b2df60fe29af828a9f778afa26ba85d539118964c5cf4d974f60d26fd0315269f7bd6a6356c6a0d3a9dc64007bc358cb961a694ecc0a43f46b604e9d204d2749c19ed07a7764163733c8eb7f2449c39bfc8ac8765aad59614d8977953265c6448ea2514e241af0795a90dcbb9ab58d75701f9a603260f444a0eda664eb0818f2a88d9fbef7479bfcf7572ce2aa48edc08a77ba60ec665e599db8491c38c214de5c4253cf8f1c011c8242194a9e9f6e231ec65acbd4ae2fd5641768b94acffa19";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 441;
	dataLen = 4096;
	combinedKey = "a90edbf292f563a37512173caa316d0ace0406c6a34526f785e42230b44228a7";
	iv = "c1e90acec7d67e9a2a90599de9613ae7";
	plainText = "b776bf0bd9a1453f3709bb8167f9f47ed83acde2a198e509fb5dfcd6b9db8d255ea080944b60ef60fbb02808cd3c34caa5f5cf0618d66f8eef7473db78fac737cd5a5d637f0e8eaf0fa3a115a2ad74459f46135c40c933634aa17804e60c4f0e77b4313325fd4650c41d7b3295513f1d6e1f95be9f121f73adebba7c198eef0a2ef3148cec1cafe90fec3b260f910057e49f5085e1e3db794eacae4e0893bfd88637e7442fdac9f74529c6947b9bf96084cf8a995ae366a1a82bad543d036c7b3f8a8e74da8b00b793ad35995fdbe1b552fffe8186cd7457d504cabd078919993bd4e191e5253a2a294aa27eae46ea5b5d9bd2f9253ff29fe4f6ed20500001ff114575fbf91b2f0b06687520dae4227b944ab4090d369cf5edf26ab0467daeaa56a547a18affdb2a89f3bf6cc84390fa388dafd9e480c97136b5050d6ab420aafb9f0576c3df849ce8e866a5a643267af216834271df29c5ce61053470f4f91cf146fab4aa2ded25f96356a7a7bb3e60dd8eed61ae32c793e6a4ed44e4eb7b7954db16ddc86f91f1b6424f7412b86c91457fd492d099d45cd6799eadb567cc9c04f69ee1190a4d24344f87e8de6088b5112167b65b9db8d584aa020b812ab09495c7aa0820cafbd8200c09774ef9159907c8592522ea669d042f1d628ceaaeebb319b71b5687abf903d2864addd652afcc429ccb99abeec4291581ae106c482e";
	cipherText = "34b166790e9d25f98162fea0f339dff919d9af2c360f5dd5ac6704e0b32008081888cfb096b2ef7e8210c27310ce32604fb892d61bf460a2b1783834cbf9817eace72a35a7a19cac38eeb1ace1133304e57662d2b7e10c07250974f9d85a5eae505f6bc8b77a0f6bf4277e17ec39f8ace38f6c7870671280530ef5fc9b321af91445dd34fae1b36ba3d854fb345bb3d82c0b4cd61d489e94b083837cd4b188e0f528b35006147c55617f4df41d3ea23995c9bf4797384633d3682b1b9033c2b9530daf91f1a80b50286e30b499cb7162d32a29c322f8baaaa29ea101580eefbbd6ec1ee28ba1fea91dd9714027664f30250fc0807c4b80e3f9a8fbc02fe2398e8e81685d039fda31525578023b8dd0834f7cacd7d8e316b1161187eec913226aedfc57c1570ef5180398f809196239e165a0fca5dab1e18969601534d1c3fe2f3a53999e0031d3e2ba0593035403d810c60309b766f16aa897d45cdaa8b0d6a2238814f35a435add535ea40a81f7065cd21c0286f82bc10cad6f9ffef01d51aff2419c4e100f5314e5022002a2f808a907a25c83734b1574c1d769e1ef19cb77b1e95ecd9e5af04e609163d5f08d45e01dc1d439b1004bffb0c7da8957c92efd21e3509e70094d15a00fabb1cd2e586ea439720ed631d6ac4fa0f63930c1685d7e27372aa58f3046e218c45afd376dc593039a90d517668bdf5935a907c46c74";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 442;
	dataLen = 4096;
	combinedKey = "3a270185d465c1fa4b80f4afcf69b3270f292ee14d446985e02f3f495f4e5b5a";
	iv = "bcbaa058d5cb6bbe8e261aa94cb90645";
	plainText = "427a61d7d84d3f704ccac763c128a7862605dc09be1913248521f07dbc01eb2c1376b2b0fe56f32c9155c1e6fe9fa08cddb9cadc38a8aa8d72146f607960100a74443648965c6171c17c1c1cd58a195e0f0820aac4b270f5beaaa45f60f03786e29e3d7ef543bdb0cfb77c875de6283ece7828614355a824729879c304f07415233457a2d23a8b1766674ec89ba494ef004895dbfcbbfa157d61d34c12008902a1efef45572d3f8c32edb8e7a9d5c562a71a8fcc55a26f3c18e98942cc07db413ee66e53623a95e6444729a0721ea28df015c14eeb243fc479bb789e57b98363f57b1d5647adb187884dc01d6048ee71a0c48a623758ba2245b1f9c09cc3746aace6670031f0f0b1a9d381a108f82bd3130ad792092909937995dc9314c61ddacd9e11a7866870c7f5680e1fdf03b45e57e9a5e2bc28669b4791d1c39d9f44d3e0ec7e511436458fe060cb33aa9882c23ab10687487d88ae9679f4a19e0a7f954828cd2f67f32f3f7b811c38f88de8d0fb6de633a20558bd6b135a30995adc471778b827ef5f4c648d70f35160f7879192d5995c631b05fe39ea2519c9bb003a42971ad6127b1a785e8155335aafe748795b7fd951a51310b90af093d8a8ca19acea2beafb5d40ebbee481ab32cd27541c29f3c9b85cb3bd789dd10f44f144300beaf9f04342e20cc8c8f22a128a19db0dec2be28bc323978ce52dce8975c870";
	cipherText = "196c410de2f99f01b750dbb9939b7c1ba248bef27c45eee4fbb0d3145e4a91d4a258a2ddedd1b8beeae1b6056f9b201b4e2772f19ffa060dd1e92719ee32c8b828204046910191b90f589d4ab6bb42d4e14000ae6ad0363d54fe547b80d96e75a8cdfc9bdc25dd8e48ca48d8ed1c88dd1743d136e1c00e1172ae4809915596a0fd2d81ce983bcaa0fa7aa68fca482d9b4283a1ac7fe618e5d816b91915ad4ac013d1721802183a6699e1628c44db266e69af309edc3625f3018ea4106c1d607f41ed2f893c3e79ec83039ebb755776a16f45413abcecc6e52fa89357edee1660bd09460cfa4f1b074e377ff3990f33f5324d0b42bff7ba97c2299679f91ee21481eebd41364d065ab835e56f058756bb3af26b8bf2b5a682ca1ddb75c22eb74d0ca47ece81aa0b19d8b948f0d2baa5435a0489b1005765d226b29e95e6bb174d5f45081ff0642f75e47a09b147937975149718b1f67bf98895c1977c6efb4f3e0fcd620af710934785cfce2907243e9f156341a5692ec2d629b70c9e28db2177e83017b8470b4ea3b361d59bfe0c4e91eaab29ee88772b439635016ea0d7bc7080293234264f224235ccd6649b0c6f3f57caa3e7424597d7b64ead9edd8b6826ef8e1019de0e447bbe09b3f1f643028af34ba43cf6b890fcc58d47c1b14e120541acd0252fea7f9ff5c971065ad3a8d646bae6f630cea53487ec7b1814cde600";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 443;
	dataLen = 4096;
	combinedKey = "199b8f1c07d9c467ce9eada768e2f84f4018cf7233ed745fa913e929084515a4";
	iv = "52e34dcf06efcfef127992d2bfc84b34";
	plainText = "8f602953c1e9900971868d0cc76acedfe69f76132d89416c8535289ee13ca4c07d09ae0d0a8ee4ec9342824874913bfc4e3d2d07da551117ad08ec03a39bc84e50ee028e790cae8f8cad1d822a0f1bac6cd097a93f44d535bc639f0713221bada96999f41727626dd5e4c2ec1cf80a827a623f70544c8fbdbc4131d2540f7620ac7234f65727c20804ede411395950bbc994d843207300e43836dc5fb3c1421d6af1de6063e04ff9d8aec198397e289ace3185c06b53f4421e67973ff2a42cd0e838222cddd217c8e6d423399bd5266f7cefa506632d4089895c900d5bf7ebf813d1a293ac0cdaf7974ede7f9b8f2f063f1fbad450f66a8d5cd7e63f236d9aefe7dba270cea72ca83ae3e85178be4842287c5a18d92830df97b6080bd46e6bca3600fc29684d56d933fe301c00d7b2555de17426723fef6f0a618ac5db1f21b86776b5d4da0d7657e29ae1fe1f07627e936233f81e85f9e6a08c8c2d8f941b513be4c3187c3a388f3223a0eabc02156a89bf37d42756c3f6e76d4a8a5a797a8f29e3b803a490f8c811ec73ebd43a794e4472dba0220d6695ccee16cc25aa1d284f89727b1985e197ee38648ec6fadba0429ebd46d70325ffe5a02c5f0efe1518d436b64bb277a2d2ebf0631a5676b515ce846a5a43e732eed1eff9af2839b3064de2d48151df093f87243abed97eecabaf78679007043839a344dbd374f8b0b9";
	cipherText = "244f725366a9e01a16163544dddd0b244c7d9ea28a1411e4f6a8966cbe873eed7a5d41bdaaf628073a6ccd9fc6eed14533496c078760d79c5219b9e7e3c09028949ac100803641fa26631d54c1a26511acd5b74ba1c04fe69aa3d3826ad2f5953e7f0d28cd0e74b0fb7aa055b6ecf33e9c40804e95d96e80e39b11e3e754459b999171aa6574e386faeb3d45eeeef19d23708186aeacf4ede84ca2f06f959f8590214b2256f6254b86a9c81e706e58902a4d9a2dc855d5b920df211e5924dc795a00b57dd7fe88f6dc8bbbed5ddb237614ca4b84792ccd3c51c4b089747a8e44af7ee96f477a67895f887530cb2f75de4366c9d7968eabbb3e534d7687a3c5c2a2771f1570aa163bf1ded81324cde3181bb60cc70d533b5f2f30a092db433c52fb3546a528214ac8c2a3d1bd74796f0267b30f5d2f1115e3dea5557f85d2474f52e87ab1872bba29733a841c6b988081d55ab68328ea0400d4eafd111f5f270e7549c44e8f029894da126a8c694e10db1f91357958636431f82139820d7d4f4500d0964fad73ff009a64dbdf71bde9c1d80d730c5cb78d6d67fe1c680502bbbf448c1003ceb007f036185d166fe3e6eb2e013731a6e9accf48871fb89ab8d84ed1c050c991102b6473a0e29a6175974bc3da254d263f7299228e9975e9a27635f771e09e02bf3a9df26299e016766f03fb99214ec5f0c98dc56a365e7cdc5ecf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 444;
	dataLen = 4096;
	combinedKey = "123b0660c990331f1864a5f9d9cf4e5215ee478375c79795046efdcd59f2d2c4";
	iv = "97a250baa334c20a86b0e6357659c0c4";
	plainText = "9d8cf33748194efb7b5ae2c2f45fa18df202ac4f35dc28812a99b647b495a9413341a3603aceb655b6828246a943cd51f2765005db0aa6f16bc1709b64769ace58d615c93e63449b5049da0cf7d8a704bcf67682cae0c5f8b674cd62b4da2893922766b53c28b2da61fe6797bb1bf9c9460c18de6af4248a26b560143477dff0c81202bf2e2d2a83ffe401f3ed32cbd8b58bf05e679bb38c4176fc616583d5bee51aa1439f3642d6f9e8c84cd82c22b85706a87d056c939dee45945a6ca7438fc32d395da137260e3ed2420a3a2b96e65228348769ae417fad4ee35393ca6e039e7b911a91c76ab3018ec70bdfe4897fe38cc66c695099661c5cec3033b982767fdbe33ed5cb1f6fae2e6f675ba5913dec7d994555c66f719121be829124b9d081caa688865152736a2ca701bade316b7352c7d5506bad675a7c7d747807a3ea468e9f78464809037ea30161a34caa4206ce3a0d43bd5c9c2757a35d3f25b063f1774708b8f916cf9a01b72f3dab4fe96065f08281b3394b51852733261bc38b14cb5a67b3e49fa34a75fc07abb40d006502f6e627163aaa5ea7a36c064b3c42cb5fafe66e4979725e3e89290ff9d0ec8933a9e974977f2142080b3b220848db10fa3d12ae94f6d25f845d94666001fda26f7c085555120f110266d83c7283732dd41f36b90b549d5a89192e878e3f0dccd3ab9fe8c3a6f71db0a1a057263952";
	cipherText = "e1f83aa462078536b0858afde830c926f55bd648992b70bd462af86179ae3ee08b40e32568d7b696681f17f345f49b6599a6bde8187790091ff54071485eff546109f18b3ec40be3c0319f0132231d1fbc241cab244994244387d0f39bccc5679c7c7087d6e9533837f0c542bd40135d802c55d92a1354f279a497c3d7e0f613648d053f436fedf6bd6a732e7dbf0208e416aba16ebe481cf06e1709c567efeb2a87457f448dbd3ed6aed4ca354433e59646c610f7a035eb8741ff8ee3ee82ef6ef679a657410e5d08ebe345debaf38d4302100dfcb5bbcef3e41aeff86b6c4f855ec624042f44b90f3583d309b8537c7cea6999371de623e98c345c0f98b40ae6d0d3a324d3c9d138a88329cb4fde81638b9ad78aa829ec4c406f7f632366940e65c427f2e4bae966bce6dcc72bbefda32c545bc05594ec91fcb6a48c3b5bf0d67b1defb74cad01b3051493a6e05fb1daad9a3eec16d5d36d7d635517a73afc81c9e07f67b2d68bd3b85d7519acaec322351c4b2b7db15419aae882b5c68eef48053dae5d873f3f99402349e040ea6889809f67108f102ebe95c640b534e4ecb574da87663c6b7bdf777d796f5c2758826e7e54e33bfb24e1e715d6a3d7c3795d6d6b5d72c3ffc4152a53cca8839225d431980bd1f5917180f59b68f4865d21d482874bec0596908e32eee61c497eee3120108e5b1fc805424b379daef7ebf5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 445;
	dataLen = 4096;
	combinedKey = "63f36e9c397c6523c99f1644ecb1a5d9bc0f2f55fbe324444c390fae752ad4d7";
	iv = "cdb1bd3486f353cc160a840beadf0329";
	plainText = "9a0149888bf76160a81428bc9140eccd26ed18368e24d49b9cc512929a88ad1e66c763f4f56b63bb9dd9508c5d4df465ad98821482fc7194ee2354a3fadce92318548e8ce945208160497b9305d9ab1091ab41d1f09a0c7bfaf9f94fe7c8f1ea968f8f9a713acade18b68232106ffd6d4281e99e11d6a428b51653c0c7dde5a0f273e74ff015ce80277d7430f5daea8f7340645e0bec25f4040fa13c0b330693b10083a8b9bc108fe64f3a5b613cbb565aee2f09f5b204aee17228fe6531c70c0ec947d2a5147b45c51ac7dc8e85870387eb8db6251368368bf5f246b2957daff702e379022e99161749e6be8eb79d519799aae07c1831bd0ee72550b85333ab9e96a533e29725d7023d821abe1ce3a744be02e052568f84e6e3f74442bba50d02ad2d6ca58a691fd2439aa3af0c033a68c438b2d9a0a01d78c4f87c509fea0a435be71ba23706d6082dcba62625999ece09dfb3fcbe08ebb6f2151e2f12ebe8a5bf1162c259f202c1ba478b5f468a2869f1e76cf5ed38de53869adc83709e21b3f8dc13ba3d6aa7f6b0cfb3e5a43c2372e0ee60991ce1cad122a31d9397e30b921fd2f6ee696e6849aeee29e2b445c0fd9ade6556c3c069c5d60595abbdf5bae2ccc79a496e83ccab95740eb8e4f2925dbf7297a8c992756e62870edce98f6cba1aa0d5b86f092143b16da1441547d1d42b8006face695b03fdfae645f95bd6";
	cipherText = "0eeef28ca159b805f5c215610551678ab772f279374fb140ab550768db42cf6cb73637641934195ffc08cf5a9188b82b840a007d527239ea3f0d7dd1f25186ecae30877dada77f243cddb2c88e9904827d3e0982da0d13911d0e2dbbbb2d016cbe4d0676b1459da8c53a9145e83cf42f30112ca65d77c8934a26ee001f390ffcc18703662a8f71f9da0e7b68b1043c1cb52608cf0e69510d38c80fa00de43def984dff2f324ecf39894453d3e01b3d7b3bc057049d195c8eb93fe4d95a8300a5e60a7c89e40c691679fbcafad8eb418f8d1ff7b91175f8eb3c6ff2872d32ee4c57369e61b66d166fd0a43457478275fe14bf34638a9e4e1d25cc5a5f9e257e617adcdde65e2557405362c891e6546a6deeaa8fc03b122a55874d33e0a773523468325ec24d4faffb63c052c811a1c022bafccb97988b7e4567b247d4044b052ff73f4c671d27e052e2ebc72d0057cb217c5259b60950e3c8b3d9e3e7630f9ecbe548b9e36220f33c2b4568307cd0375bba1335e58bfbcde85cc84c9c9c1ce74f44b28ea1b697305bb6ba3b464e5ab74501293ef9152c0f5d3307d26a1f0741c5e5721a713d1b86c1808211f57aad09a950b68630afce4f0ad9f32e6769b5fe31929c446f7a3355f45884c748c9055415e637d9ad87d94c4657b1ad034cb14d9a72ea745fe52d7a711ba41ca035856a5a4489a4270bb30d5b63f49c0512fed4b4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 446;
	dataLen = 4096;
	combinedKey = "d329bddf0f65f9bd432d4b86fd4b759b0521ad22ed627f012ba461503c27b32c";
	iv = "efb13604cbad8ab531065200dd385b46";
	plainText = "333f807e923c021f3c8c9330da79c4f9961335b37ee8da768e9bfdab68c5a7eccf23f7430860a18e1d0b44d80e10cfa92d2eb33f376e2ab62ab271800a07648361e965be9716ef37d91a2a541b271ed3e2fb1b13eb01deddfa2cbec43951477b2daa568d8a8f84bd97fc098f052d50f8583fd47488cb6624c231461051e7f82ed3370aef8750156f75376646b4a1fae492f9ca1b02cb3249984c3be8a577e886ffb4c30d59df02b79307cd67c9a602d6529f0be432df86746c1f27e3e14a67ae2767b25f944caf686d757591e0073f847c389d8366a252a72cd6336618d1f2d8d7594eb28aafa3c315de45812df03674f5f8ec06a77ca182fa0c8b986aa6c8627bea313ac4e860710f25c8cccc55527bcb47547ac4fb200a75f29a2b2d95210b8f167fdc05f22581492f849c02afcbef8c3d71417fbabf500cea0d8d2bff0f1ae3dad53bfa878f720176f29c93eec3b30c4a98713863400d4359a8bdd003be8420cc1ca1eeeafecfe5f1e744883e2ecb004b6773ab6528d6817e4e36a8a4da812045adb5f5d8e9756d42ef6a8fd5a533b9d65e2158ce93fb19d124762dd93e44ea5d248094c7152242dfe2ebc0c6c9c6a03005e9c75424048a3ae9a21cb6d9d33a4801d86cbc8853b4882940e32f72ddacaf0b0bc1598ebf8d1ecbead72443b3298c9191978439c10c1d634a52ed4fe2232f3038d9ab057f22415cfd312f9073";
	cipherText = "a2e8212b9e9046018a54deeb7dc5579419aae24b0747759dba551e3313f8b772235bbc5c35f601033588c2b4a1dfde9ab1b1291a129571dcef6e9093876845331c5d08ccc6e7c028c21a77132ba04d5b98a64e9c99342bafc7fe67a690cf6092653ceefb4ee36af46d5448403c567af70af1c2a1e5142d34e2faf66cd6923a1a4551013efc8a251b194616ab7af2804dd0027493487a66f3a548c27bb4ac628c56f8513d291e7fb3aa01a9ffb6513322e09daba71ce47af7673422033ae365bf795c1a79f02ce953be0f07f3c37321c7e143a75092d65f18cb8da7c50893e82d1fd0db8bd63d21c7fe40197c46554ea26c624682c1d0d3f633d39bb4d1b5636199463bde21bcecfdef8155030fff0eb0b1d4880935c169097b6404dd833a9aff2bf6c5ed8a0d3d09d746f21a5db79483d4f941a6d07ee6d1c8b0d8248008b4988fd6996345f0eaf1484fd1aaa115a96aec8f7f24753991c6f8a19129d9e9de207e430e44d9ea32d10eb7f1d68446990ce01748e6c121301e611af708005fd828fe6118ed5bda775e246901e9202e54a4c1c28c91eda3501cc1cdbaf3d75364fb9e21f4444bdae5a9c6e0677e6e602872c3642f9b767f325a4573eb0e0b8980b79b7f36e83c53465361415719782c5b2d75feea722dd05ac013474921ccb9640413ff30b5388a73a2ef9f73aa21c540f45a1f4388432e4efbca6ef5f91644642f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 447;
	dataLen = 4096;
	combinedKey = "761e703527d5cb0e4d5d5ccaa7de80357d9d2427305f9324da52be65a629cda0";
	iv = "2c25a401c622815b0f4b02f80e4de3c0";
	plainText = "1b650ab23a53d31651223e78e73c436579754b510856ef97f936e39b681a2f481d5d024550347b9a4fa924bfb1c823d62837187b76767573dd3ce42603b907ef2e7a0ee8e380c3e56f8d8fd6a363c14bce305b5a33fbfb84039cf9ada37555a9a7672a57a0c5f2c97000c8c31a8a90fc8303f585d93e8d70459d728b00282e6bbf36487efdc11c413a5a57e9361998b1d6086b8cdb0885369c657e63cf6f907b5d3585f7020a6aab49b2ae35b7b57279ba01bae638af867f2005480de4b2f65890493cf1d3c8ff9cfb452d669917a78dbe255350a30960fdee1e1a7a1b93f6dc77802690808554489587fa48c417f9ff6866bd79a9a341fea71449749048bd62f92731e7f00b19d21eba07523c4a5f13503d47657336d582fa5d2d6f7cc4ec1dff8c035db0f81233e77d0fc3cb463189cd9e4730b6fe3a42c471f1c4392dfa743cd1d19add570ab78b12d8656b5af3970dd8dacead6699a977567b4f93e255b3ee5850bbcd8ecb3ef0f637b80832bbb1a4053809a17405bac95ae7d1c5c2672c687def6843e8c4263568074ed25feed513d689911f486ce81ba9eb514e5d70b1ce3071f5ae5fcefabe9279ce18acd12b79a337e241cd144126f169d837c37f6915841166ba51cdd5e4a3166e74cb8bd545218370be4e04928143a90f45720b2d094a219eec49c4d4a76b93a6f4c0d1db4a0aa7982228fb12030fad3c19733c50";
	cipherText = "d9d2b09aef911ce3e2c44e3ec6a3f08c54ea3f9218c77e42461f0831e01b6ddca1427f96a0be22412dfda024387afe8a611af66672337ffb5fc389f68b8ca4a5bb13d4f23f7ed182a76e8b2c4625da9a7bc79b8255d8789ae2fe6f6bbf057ab1f7dcec56713bbff8a2c9638ed3cc16a128b91dd3b6e39af9054f28d90c4a2ad81741f4c2a870715677366afaabd55ce764e137ee714fd8893b4ac4dfad97e64fd8270f8ad25c2cbcb686612827eb1f087fd4c8d3345314e4fc8269ba2d7dd021d44edc24c793e9da95aee88eac2b30d28cc4da7dd93d84ad109e5090d46f9d5ab892089a390ede5cac99ca9543315090c4cf8699bac9b7cb09f0657729354bfad77c1975370dbb41fd2720ac03b232ffa083b37b7906bdb5e460f69cc3678ba12a2a80a4cfac9f83a3fd0a20ce29b2542ccb8720ce791e521b7e0231ad3ac140bc37f5aac40248605ef4254bfac76ffcd805e4ad6971da7414e0ed9714a42a037f76d845f31fbab487e8c1ce863ae5c47053069b3e21f10416101c3f1b16d658ec8e7be35bb2b641fc07311c07557f57dcfca510a509e8fd5588de6116fbedae3cf2ebd2e27d02b1442ccac688200ad613b4bca1c788cc3570a1bacf26eb1662ad0ce3383b431f7887b997ecc2969b9a2f8dcfd1dc27815406b5ada7de4f35390110441a569e5466f21e44da300ea345c9a9a75c34ba8f5ab76c2b15f3c1838e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 448;
	dataLen = 4096;
	combinedKey = "5d2c9374ae9c7841298fd424cf4674792ac1cc87c682fa7a4300ed05e963aa6a";
	iv = "1a7ffc137b7e5e8dc7f5e85b5b71b9db";
	plainText = "8877a1be1107212ea60a8ed6eb90d11fbde9674c30a6b2048b1779785d9cbd9b5cb7474e9d4b1a5a90eccae78298aa9ec88f61424390eeae1574a5966c03cca83daead1d997728f8551917e2f5d904dedcc4b55129db2e50821ceedab8a0ef939a62e46f29d0fde273d5ad3ca64362726429957df35166beccc62a2c7ab00da68fe46f33c2726ad3eea4d9fdf36cf5c8f6d5a3efb120f8953cade6f0921f8d208f921e553c935012211fe3879ed9c80eead065849e10567977b9bc8cd20feb97ca5b9048e4ebce798e7d8598763175c11b48afe3b30c8d5e8569ed99643d7e932626c261ce4fdb59b17590cd23b224aabc48ce33519db10178bd03f1392e2f17396243699ae592598e6263ce26d7e7703ca89a38ca9787a0b529bf830fc3592a9f1da7e6064f9995e5c05fa87996526d51066548d7df644d96485bbe9ce4658a0b352a3223fb68e246388ebdf65d609ce8fe062fcc8eecd6809db8cbca34d460d2de5512b06f9d8ae76c155146d357af86a558a71052f167c035fe4bc8624b4bcf742a9836b3d963425075842734ed29306e1d6426606034f4fd72c3a3ee411d344e73bb88a6c0fffd6571f6184e34aef63564c46731514cad97303cd8e6db1d883edf5902533c036389d21697126e4caa7959816715ea2268d8a6e403b8c453e998f033b8f58a09b1a312f7f5810c84e629d267cf3fe9b762ee29ac9e58adea";
	cipherText = "14a7b4c349d55fb434ef5c1ae7b617ef06e17089eb4484e12c8d36dc69ab1fa725a0852c6d54b079fbf7cf7ac872ffa04699201868a4fe74c07fbe15fe9107d7c78dc3af21b1adac7b2c4dc82e11930e792c8ad8ac1582897bc1c337b4daabda7441bff04db274f1dfc09d1f9e9115f88487c7303d041f01c13b87162b55558018ec2c8917a2e1956425215663ecec3441047894bbc201430446e4324f757a4d2ec3cc07cef92633e3a52441426de90e544deb64c59b368c6ed0dc7c3247ba9ccb352d840198c43788853111527fbda0df3745d6e2a158617987eb2f16b06c9140608f29023201cfc6768a8a8d0cb43aa3c2b86d3cbba4dac5f89c30170424dab8359e37c7dbab81ca2f3f19c5bfcda3c85e5246207781a4fe5c61412cead9432ca010a79b4115c3c9af88a38b8bdd60326797a507345070082edc50bb0b45fbc639b41bf2c605388bd4c86a4a4572b91719b6a251966962a416afa2645299d2987883959da4d9204ae26c0fe327802bb4ed4490c991b60d6c29e1771ba3b94564d692a3d9db696666a20a776239e5db4c2aa74a32f1191e5c384001c66f295af94daa437baf492b4331872eb95038c4f807c4e0de729bf19ffb154c2a139fd77f9360b321f7f7e07deda0811b997c432484b23494dac0f5f5bc569712201f5b7d6f167f51907ee6ab92ce509ff01edb5a308a1e5ba213e95ee1d417acc0cfd4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 449;
	dataLen = 4096;
	combinedKey = "0ea29ddce38e5f3fa81c1711c1a4bede98c99f41f881553ab792afd5f2c770ef";
	iv = "0436d19307464047daac550478fc72e5";
	plainText = "a6e9cb68790d2604be0111dd3275e3c832c9cc5ad9e8b4e51c4a6a4c8037b71a10379745baef007344727b24cd385846cd3f8c083c4d3bb1dd6ddfb678c853ff274ebfed03cbda1a57ad0191259ecc928aa519a75086cc5e4c58ce8671f90981fb080a93d675054d62e302a4ec844034f66471e7c364e200c9fa99dc91ea0fb542b0bbdb823de9117b9dcb522264fa72baabcf277c51bcc9febf3acc00b4ac2939cc87bbcaf74ecd4441f0885c565721f3dbe758463e6b5f477e7ab337756b0ef339b73ad6bb392b941b75113044d0acb1299b4866c5196432a94999b9a03d2c9befbbdd877e4e4406923e4c2e910ba3398e0329b6f71a1c5d0a5c07d541b332ba1c5f7b8cee90344c9f19a13a50a2faa24d000c57f339677751cd16c7a1177d82b898e246ec71b5425f8b1ddeda09e355ad0ccfb4f83a3fc925da514bb3efa9fc70187d9e3306abeaa825232571dde735e4c465fd02d91d95660323c5e6d291c948e3a1c0b5024ccabbf18d7c5f1ec815ee377210a511f100df5ba56078ec1deb225dc2571138b312f1d94cdcfbed23cfef63e6d044a03e605df9da06a2ffb16fb76d2cf5f00a80cda5e7d2e62648d3a52acf71b6a52b67a89aad5958e9bf632c9ab826c5835e09a5d86e3ef7dc9a2ba432fcf7e14f0c820e99962e2ecbbe9459de1e88525f8f89b872e8be6e24d86500a815da93a69adafa743e1146cc542e";
	cipherText = "08992037c310e72fca400a2dbd7a16c698289b865c44dbb92cd6ab6644892a0fd6efd8926b76db155c826d41168565b2331fdf226aafb0e0ba3ef57848c3a572ba333385ac3c99c40583fa2ee4382faf6c7b71b22f753deef71a29e9da18f268bc344f0f581b14c90b6aeabc375083468449728a92515dd0a9011a575158c713456514671554a2460ff92ad14653bb08db1e99899495dac231d67174cfdcbcf003447cee88fd0f3f856e4b5c13fb4748a683d5f0a18d203839d2a8b451f378e8df3b29f0bd547a9a79ae49d22f3cdb422ff28b585d998702614e36afe266c22f90cf199c6866dab73ba166290059006dbee14309e72a1a2eda6ff5f3536ab6369259c6d89613f8c142616a2019f1d1e3f2c041ce620a85ad691c069a1acdf63e3e853a5b99308ef154fa97da85677b36ab14a8801b4e5f9326118caa4dbc4d696c094c375a845a02618aca6d7064fad2ce7a4ca986e4e564e24ecb431377ddacc7f2f342aa76ba9bad95240fa0179d6e9b29cbc849772586a2e8c96409c556fc44dcdeb25a6879f0d98962ae966f53eb074571b8a85668ae24637503e8514439c63ea8cedcdde9eff7b6ed3b848518ed84139d9a56047d49b50815ee79f4c62c09eba5fc8c16b2549af052c6b009564705fe075d00f6782c907d278e8f2550af2512244128d9654909da3a938af025661c627d2a7534b5e95b123555f2969857";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 450;
	dataLen = 4096;
	combinedKey = "58c14db27a299bffbdde0c0529ba2ac7fda1f84be1dcbf4eef58502c03c9c7f6";
	iv = "720020ad848ac85d0a5b43ac595fac17";
	plainText = "718e28848c324ed5dfd8dc9468468c12fc0a40b335de3c38268090643252489b836e9668c8ffa4df776dedce50ce87ddaab701b5339b268968adf959eff4d6da120234e2cb85c4dccb2bb23ddc48fc069b89962f1c81f0ce0f266f618b34391ea9a19061b85dad74661cf49aa0fbfc663d6395dba6749dc10a6459b8d089047a8793a2ea5721e2529280aa85b368bc430b7c9abbc1a0636dfe0cc553103c587c8cd5143aa59669d2fe22e9f11d863d002447001f577bcfc773e6d623f99d0cf601475622f0c8557de37590feb2a0be11c26956e3b6a9d6ff6a1a068f7cd85fed3b16d36582f00735909b20fb3015e3335a8cd4e94495dd4e342b1008e93648f65f0d2c23921c2121da259bec99d3c1936843766d39dd603c58a7d33d883e6742f592f4daff0d341ffcd871dd40ebdf6b16e375243e202741c2c4377f7e905d50751f07b47bfa0ff951343ed85cc968e1f3fee004edabfa1ac191f2a51c6b5c42ff6d466e34cc8f7268ab6b5c61e10f7c8dbc9a7ee04572f4352cc20bbc602d1251b541999dfdd270767cd5e7d9a25c21d4d2f42ad7f559104fb24d8f272faedf41955222cd663569df7f2510d21fba4782642b35f0b0cbe523b319866688107c9c1a72ec6d4ae908c954f0da0d0897b3ff3421d85078dd34ca2ae09aecc22a2bad46e2278132d63f86639286b69e4cde9a5e7dfc280cef5474bc3c6ce5d4b4d8";
	cipherText = "d7f04fb633d71e6083f3d1b74b958c2d2f60d53b0b6fcda8fb50d50dfd46a6212a823c378d37b50547cb339981c8c4978bfc896826a702138623bc96e5c15c07db44a2c10f2e1f054cd9382d236abb43affd23fe4fd08afd4a2c53166cd7a1f891b777a5a4d3e515eba0fb7fc66175c5a0eab5f81aee997d55221d668d63f1e8aed61719e306b464841e597f69cf451288a8ed69a6275625047e419f77137bc39f2052f597823a786036969297d0114e1fbd4378c66aae396aea952b72473c0e3527e8ad3c31991027a95d7158f74f760bb927eea8576809b453928f5895164964b77d2bd1041ced6d9f29389a94782d8a35078cf36262d3684d27531b111b3e8632920484897a9bd8e49a14e06902057875aa01761b56c0c6a6acc73a7f3c83397d44474a31be36844170f43b74bf3302221728bca25b4752d3784e38be0b23fdba97b07e47ee4c2b383b20bf27218b1409170eff778157a8db1a11bf3361a74748670a7bca160cd38279c62cd9c246fe93e1881230ad03b94a6534f2d9400b7d294dff95815e1fd42c36329b1bda3e1228a03d08b9c9970ed75c9b4faa17debeaf33fcb11de001771a7495f28ebcb484802f9980af20b9fff3f158daa03ac1d9ce842567716366a185d70d20039269ad5d010d40182db8b31a569966a1743eccd485314414dbb692c5d4fbecbdb44f682a7e2cafba122cff6615c7c9c913be";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 451;
	dataLen = 4096;
	combinedKey = "150749cce3e42afce1870a505160617289896208884e8a4c6274af0554dc1932";
	iv = "e59898eac7d4ca5dfbf7d925e05e5b60";
	plainText = "ae016a0d8c9e5a6eb99f6eee3b2410cac9d8854bcdee26323ba48fd7576782baf7f7d267b8eb19dacd66bce1b3782e09ddc2f2cbe42190d3145714491b953af1d97df4e6e5e9f9d072af492c665b817909d9b0ebbb32d82aa1a4546f7e46bd235d9a195ea2a0d0242f8588547b80c3dc92b9370d99ddf76b5b5b19ca5a383a3591e905729fba1dc9819ee926fe0dfe54ccde4f7b135ad026775bb8f283ddc11c6181550fb2bbaf7180bb7d3622c0f395e4eecc90f4fddc9196132a09c3ede1e2d30c597594ed86c6c0e224d06da043d2d204afb33c02d31eae39140dbe395a99fe5c4f837cca2f4ee7d65da874c8a4a6ff4809c651f951146b2ee690461d6167598f0f68873e6642530dd3c4d9f9f0b0f28c093fc70f32702fda30796ec617de02af942fba305ee3829a336f6971bd214eb5ed05fb34a31b2ed757bfb3222f16d7c2f83bb23868fb1630aef4a4547c04bc7c5fb748fcbb5c624a4403d16a9c04c8cc01674f07be17c58ee28ad3cc2f9b8c8d756e20dbfc8c8a448872a179760dd89f0d0835bc7c51ead1763e52affc043b97656ee878ab5d673c24fea366e7f0afd1bfc401f63c32f39eb7db151bc8d9044405320b1b2d7fa7ffc58d8cf9952dff99f43045f9b6aeb3190293b624ac7886ae90988a55a91aeb65eb5b50e8f8f56d30fdc6b4155cd456785374daeee280a907552e1f6c54e1564c9c55a0bae26a";
	cipherText = "e8056c813505871a3d6883a61de15bc587bb80b3a027168a1dd3398168585c7b7d711ffaba63e87bd0347949e299ad3441c29239ab38694917ff4b6c6dfa8217f12465a0d4a118b60964f1fb90a8f1971328f80614a344b6a2ca23ca7ef534495e3742beb0095bc52b2a3922464abbff46987d45c0e4b8ef3dab75437b933d4336b633ff6afa50b38f3630f9e721f4fc029f9ac0bdf69224f99d5fd6972d2cffdf84f1a5609f305b0fbc82ec30a59ea701172c9ec635d99cc3cad47bffc09f36b2e6d171de2f33cd1a15067d54832483bf0ec369001b25449404bde4a3cf108c8382399ef4df29a5fdb696d22aabba2e03edaf72d87c47103f6cd77421ffb50c5565a78846722c4ada563b00e27840e6dcae11a07a70bdec9a9ca9cd7968fe4241984c3aa69e8840b25f8ed2bad0672e2ecbe83ac20e023c3711a2fba4a9a057c35e7984d7a0c611864cf7c370af2ec9fad699c20bf7d2132f5671c530824042477568a7d09b8d6c476bd8ecb54b82d9cdd576718a00238c9147a57b9741a2f9cd56fd4ac084ea0c0130d22015ff7df2f2476145e88c41809b71b72db9a299c45494062295e18a6c6dc2babb9f408df4ba762ccbfa250bcbf3e84711222fadaad3ba228dacb814fa33fcb5788dcbe3b69931491044c8179bd9eba3f075bfbfa3b021b55da4b9201224d25e71debbe23cf3d20337efce6fbc3fff084d1a5cc52c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 452;
	dataLen = 4096;
	combinedKey = "4eca1c6d8f714d5a984c433e8ca111f1ecfb63d76fb0438165060a24449348f5";
	iv = "c81400d25fb8fc7a1afdefef25900755";
	plainText = "4982dd895c13936e25d5f297cd2d0883873b93b42cdf5bd31f488f48739dc0d2ed3b717ff8fb131cb7e49b211ef7b6b992f1f665f11f9262b60a580d8ad2157017d822a91fbe83054187b0e2ee0167227b419dc77bc9bd08d8255e70cb49e27f5cd70a341e81e25e91c844e14dfbf59a0bda33da7c341fa80728b7048981716d389c25348417b0eab25d926d144e1f9ddde66f139e23fbfcff0585de9de9861c869449c8e40a8e92bd544b7348cf01382cf73cf66bc631c029e1180dbf2bc1f2c0f9daa0cf4a337a6fadd73f90af9c1fd96ff9e1c77aac0146d3649bef25530f173060a560a106ae310101d7be096cb8998435ffbd7e71b751c4b906e7664b32ee3cec3a94dcf2347de60d58ca3d3c877b41cff0cb3f5ad402641796b89244dd28a1f301647716b6acee221281618b581b87861a55cac2e832ce8c5124b7eb7a5359642333f2e82885e46ec8533b05947dd8829a54257f97059d0fb0ce8756bd1b26a2203b1855afcb05303771b3932ab02b1916242ed6aa2aa8bb23cf03ef0736aa53653fc1186323451c21dafa4ef651a76757660a2cf16d88ba9935a3dc37b5995de59144bbbece36c37252149e69e8daf5e1ea53334e25c7e0c95542445cebc89ec09a53507a1a3491c8f3296e4f9175afee1f4f9aa8b50d7867f2a15341d60ce769913b283af5e4b1e0b7d0c762adafeb14b9d2d1a1cc9b0461375f53a1";
	cipherText = "080abe02695a811881e3279aec7f2831c74f3ecbf4d39161763b9cd7ea607df6e815610af8e70b82358c66eec734a59398a64d3b4bad11c35298dcea0631c48d0b330c9fc4960ceb252966a6cc1c49507bc63cd216e7c511b0072584b91b2576951d51302676a9ca564bdb207fe2482818ab471ae34bc885ba945e2274a1d736ed229f5182131135c98e1712d61001f464a7e0249a6ee0d31709b1bb907ff81c106ae5dc94fa5069bddb17b8d5b6f9dd1f69fbd482f63eb1e929f9ab0b3bed88619588fdd97bbf5b50b186d2e04e76dd4d4e610a750df1215534516c0bd673a7fa66be9fd6041ad3500656b6c12fc3894807c2bb43d298f9982762bcad2354b265711805d018155c6367bb7cebcb1025c00bc36d9ec4a1e446980323036b283386646697c87b333e09f232ed4c71d8eb613876be0ed0a1f8c6a63db168b866c365d6afdc6e557dfa5af9b098d28a0b133bb0f6e22f2b62e37bd0ba928c6131079a537bac0b0bea4edebcc79603344e163efe6bc287b7684f90445c89325a310d51080bae9c3c9075014c341e23265e43358953e1b104bbb48a83a96e04d5bc4e41c48d3104527b52d9913773d288bc7f9f1ffc2665e04eaf97b1d8855b53e5a7f5eced6fb63b582d6cd561463c23bfa7ed9bcf89a268b0d926c63df361d6486ce18b703203e251436e1d7569effd25bcb426faa2963ce28f18fa8f670bb3c36c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 453;
	dataLen = 4096;
	combinedKey = "9b19b51343ff130ac82fd0cdad6a336b3268e235970681bc583a0c2dff3292b7";
	iv = "1c746b49e20eef3ace544f1d03d9c376";
	plainText = "244717e3bcf60665394520ab142d4815c6168f5c5fb0165f680ea5d4fb0b5283c98af61e20ac68eed7b35b4e502fa638ebcee8492438595b8d34ebd59e98c2455c7db01ce6099be88a97e646f7183432541dd85e34e3480d7b1552a896c2f176f93359a8d78e20ce27193fa1fb3a0fee214c7ebe1f456b187e0caa3d4e7d8ab428e27776b751ed8f5e52e97dea116d889692772cc9a3e457ffa810bb4d527907b105e235b99fa6027735d7cf5cd6aac6c5835216cd61b0509c0d503723270dec5e87a3d6f17f7b856af4009ba0519db757935a158cdeb8fd44d6acd0c8dce64183d23244850d2e32a2027114cdd086b66017ba0062162c0f428a74320db5d7c128f14bb465cf541e436252d9d036d4cd5de6ec4445b232784be43258a826169e8b5a29027fa00c8f738bade9bdb0a493d6de7d2b4ac096691a3f44eb203f00ecbfe4f41783f73d5bdae6824ff17fd1325b2bf323484505612f241b4dd00e236884669d2836e02c6d33c10600a88632f9c4e913200a4dd5ab8f95a9e9e2678f7d2fe48bb408f6bbcae754ac6efac7132f4f72dd732d9349b4b63058e7e59ef4a9083ec804d3006b6a04ff8135b6610d60220e6b1a178357975e18525a8841f63f4cdfb2694ed6b2be2f0eae2cb93e67d0c23c8fd4350a3c86b63a33e601cd861de3d2d3847f2271a58977a912e443fe5bc890618764e8045f72e30ff7b988ea82";
	cipherText = "08e4762980117ab1d93af7bf715b45ff537381c1c137da5e6cb3c7044d3a9e381d2843045d8066fbda079c361701f5ea8f1e210f1344c5cce62cc3288070bfb2d5565d27b7eaf3bb6b49470258ddaaa797d1038f44bc04c344811a3fa4d5e130951bfa647b868c5301bf6fc1e6c41d930e8a110a9330e67cd9208ca59ab8773a5f644f9293886aea012a54bbb02c1f5f980a68adfe2e1e97ef27a037e27870535dd90eea1ff7422d67fe49ec9b8f3cd4674370101c8dbab657c662ba634ad418c4b2738bf14ab631cce9cef1cf928341ac200ba47ddf07f9f1de101eaabdd40c2e796639bd42761c25b042392d2b873ddf0eaeffe4fe57f748a9034098630286095fefa29d973c5928aac22ff4706092f71be896a42cefc34d57944e3bec209ed15f381cdaca7877a342c174dc1a3b786c51ff416a1bc89c5e3efcc381d7f639046c46a8fce73d994b62919cddb8b0bd5d192ed528db51116bf3c904f2dd464b4507eda343a5a762ed5d879e0bdbe7382071bd2d58e3683b4f353f1767808667a1595d53a50a1c897af48b72810243832d77f52efb6c2bf888d6eaff026897e8f97abcd749c74b1b976db6ee37aa12a41c8e52f9da88e87efcf520ec5d18d4ac22e6db94a7a2cdcd691f28f199983e07d1edab87c30299440fa363e0b8a4c17a470dbd03121190c7a3dd5b9949233fbf5f26dc5a9d35444ab6e7f3e1b2e671ed";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 454;
	dataLen = 4096;
	combinedKey = "620445ae99d4ea2d69bfdb15db5f1936e2e5f3282b5ece7b6d7ff2ed817d2a87";
	iv = "89131ff75b2182b8f5722a83211f95d1";
	plainText = "9aacb02b70641099a837d735183f9ab3941959a5b84d50e5a72a4ce06e2e28ff64f1d3cd287a96e0a2df56d7feacd2d08c991461dd3de814d87acfa4411f299c0c4893b8288eae0394bde34379e2f3958c5064b66de19601c8443f7917a7153b146636a97eafbd8fe9b3a2149241dc04d1b827563b50024eddea0e036e9b4ee048eada50894818d53ce7e808a018382cc1de29c46323e7d7e6e99d81236396eb2d899f447a6d658959b23c115f883e15c69f8867be3e9f0edf2fca6cfd8fe9b42434ab08174f6be037454f17a706387d567266ba6a075d2f2a21c5c0f4aed4ae125a3ea58a3ce4d69578da7a5a6ed9f1aa500a6d7db21908783515142d995aa391ca9556d327398efc91d79cdfb978b8d014caface2ea6fc09c4e73a71bf51941f64c26cea1236a3811437aadc32b12d1ff249706e6cb5cfc76e63a7e11dc257c3172bdbf2186641bb177c447caa879246975a2629d87d4c351784b7840e14f17c4e92c9e1bf13b9d974b06ed22d51f38b3dfb135cc1e4d5bc35740ae681dbc79425a91c687150b92e54f4c6b02391bf73386b9482c8ecd933ecac60d5b5c6ae18ef83627d6765f11881489b3038c102d0c3d0a6cc979a003bc7278fe6421623b0012f6a68c64c3a323572050f784c7fca24775173a724c5178b9d79764e7cd71477bd7a8653f32b438d257785676ef7d5baa3cbea61d98a767e24ed5e30e648";
	cipherText = "18e9e036bc18beb2f3dfbbfd2315cc8d29b779f3ac62b7c5d99c72d35fa474000558f42ecfdf23640e728dc33d154b165ea5d693e26ee9ec2d4c84fccd48ba3461f259f66f34b0971dd663d87935e6e203d308f0c377d059da59760fd349aca50fec188f83ef0b58b8c4c66381220fbd7c1d9e33d26dee44eeb1a93230b729e4216352137b62f7e9596fbd5d7771c573afc75cdee311c8ceb71fae2384ab2957bb46b6b2846c951d1b4910580cea3d8d035c081b23682cd14eaca82cc0d9332f9811a40467aa786f7460cdc3d065220c0ea4d78b9519e83e6f45cb7df6d7e3185c7275d809f23207459e3b00ca6fba649f15f7de4d70bab1dc084a2788f13e702ae5871a7a053c8f31c5127890ed8d14c0cff44d88bc757bd8e0b851682f2e277c27699aa1a0e4eb725ff549bbd7f0631579c2d3d99abc04f63fcf7c67f6d0f12a304792d707f22b6b550cb1bde8098456ef3260867a4212c0f4dfbdaedc512d3805c63ce975b8ac3e7d0ca185a7b0f03b9649dbe6d86d7bb74e09fa1c810c230904528bae2290996e66403b836f4fff4bf85b4ce0e86e86ee9be2e8da04b6d76ca3ef313a580d5bff465970559b3e319a97433d19a4ad5be710e32d02cad0b39b793cf8a4f7fb0e83fe9a0d57849aedc1ffd32877e7c17d157e3e51d488fd6a20853968acc98885534c2006ace509ebfa246010591a069b7136f8db13c5d644";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 455;
	dataLen = 4096;
	combinedKey = "0f192d24992d89dd39b84d75c0296222c04c5f0bd2509dc0f6cc661d601de6d5";
	iv = "3014ac0224ac8b5f67debefc86c7151b";
	plainText = "a4feb5b9be236b9a111b37feee5c2e35a36bcd445151bf63ad10e547c4f46d7701ae72adfdd1be3e16485d813a44db58ebc500d8dd9e6f2f2d720eb2090f4086bbf3d19ab0449c881ac0f07b08800f3cde46e381cfb4ac2d723ecafc18953697b0840a0548fc4c3a9fd6fdb5449559a8a6845b6fc848966055572dfc5df964482dce26c00b7ed25b7ea44783c352ef9ba784f0609012c7ba4179acb8b57c1d75ccd5fc0fd00bb25980317c63c5f07550619234c79740a620022234ee4bd68f068ec06acb49be998c81f11f7a10693f06a2b676699cf46f1d24f9b6a718e83f3b101fbcb55012607d249eccdb93f171ede327e207a52f9db43fe185e05b6425e03f77da25e2671dd85a4f716b218bc916f3f84495a959535ae58e6c8d5923298d1265487a5898913ccb6c9bec7cb83f7ba794db55128aa001406245979e79a86cc57700a250183c37700df7588661591776b8a33f1b4a2b17a1e0c7c671f2b309083cbab92f00b450db94103b38bbfd0f41b0975641245203703bf29bd9bf4b4b2d0c607ddf059f699fe0d38cbf79ca76d03eaae53aa37d64cc8f193533ddc125069245ecd62f425af8a80c193dc6c19928a18557ec73c74565f7ac5b65e76702e89cbedc6dbcd4fa37770fd72fede787706276351a5d5cc3d916c8c66fd2242baa8eeb16c94d93f723fd4ad42cad58a37f11310033df99f3a63c1cbeac4d0f2e";
	cipherText = "ba97fc946afcf0df8500937bcd5b05eafc287a9672dec7b007afca36454e193672b52340fa3fcc1030dae7bdfa781b4bc2e0b626f5c9114d61a548698f2fb70353c21b6a1d0fe11177cc7c0d68e8f57806cc9e1e4ccc4f75be918fb27993ec0f4e6b0ff7408da219f40cd2d3c7428f29e94d3b4714337379089dfa70fa98098ac6e9b71a11feda2508e572f2bea3b2142058b68ad42939d52d125f00960d8dfc8ea12572ea3bbe196d0f7e0fdf73973d66d8268f93045d51f0db7e3a2e23777f5909d02a24b65b6a4c92c5bf8e03d6f2ee8795a09112049f656a187a236d9fcf759f303ef7bdfa38521a69ee09b9d59f02d0b2630b147d56f675831db0c6aca786d187741ec51f63d3a37393d4edaf208518e80599ea390a1ec5f815d4171ca21316ca9f404917e6199b667ddc17b8986d061bfe9fea096042dbda3368236d6c29e3007248551359c82e9d24a50c7f911af8ca5bd79f03667c3ac26c8f92c2e911f5ce0617416383d28394949d81636c36f3cabadf3d4c39b291a5159502c9ca2b6821aeeaebdc4547ffbafa65403af21f829ba0dc4e22c72b90ba8cfb888c6a11445fa67200a8ad8b26d45609c709884269127cb5fe9c3cd42d2f9c37dc18e93993ab18387f079f442b4f79e357f3dac7a8030f1b0ca78a9d9845b02c72bbe5cbef500803094725a70c485d170a8950679295e9aeff99ee43b90a52771a40b5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 456;
	dataLen = 4096;
	combinedKey = "e359c727ab1b2f74b580e7f1ae7e289ef077394dec00dc507b4469c07f384e29";
	iv = "86b9960fafb71133aa154e44daa793f6";
	plainText = "aedf9e3dd2d63079ebcb75f9c8b58e6d99aeb56abf08173000dcda2698838e50a2cc3597df320c153e6c924e65bf6c06f64b81d679545ff7e74e75069c718af7310db41e245572278b0702e2e00dac02731fa09a15ddb153030f40fef4be17edccbd70d9250a10d6b4577d13beb13e16977dd72e4be13e9da9a1b885a5281d9fbadf98f024a41274e313ada52786880ea1732f6ee872e7f95c18b39d2f82cce69e4f2a6342af7923524a3ccdafc0742642e96d0545e9c9b8b4aad06d41d9e429b953e9e8642d7e13b96aa19112039caa1b524859915c69b464b511900aea286c2c70e4d2f2cfc1732690c00d60b29b24f4f9a94303855043ed294a43ea65051ea4f888dd9db95d5deabf2e09d4c7a5d087a7c629de898806ebc1eff9c174bb9cd34f53d0f50f1979d803b9ade73625745287d44c76bb515fb6d125a01ab1c6eceab5ad056834f56fc7fda94732acdd3bcc0f852977132db9224097d57c02941da5debf664e52aea46808fd3ea01df72b2ccb0d72a31a34c6fb0f82862b311a9e2742baa38ecb151086d2144a3d9058eeae4d6192cca796a33f2e533e4a95e2bc38169e464745f4199089afffa723215c16a1aaeba3ec2c573a333294f3c1363df53701247ce0172a328b11bbaf618ca2defc1cea9e7a5ffca479c68cc3b0c8b7fd2d69a6e581291b0fd492bcb86da8220eabfd9493d05a4f17db66ba2c64a621";
	cipherText = "e0c13b270a3836186c58af8b70ce490ef5e9ab70b3170dd6357656410cdf9170db0426f1c5140a0e552a5ca9a142a7208cfe457b9ff26b4fe91046c3507d60509b2fee525c0969a2c93c3763075b0b9f9aebf150e73564ca1f27fc3dd4734ed97acddfc31ad1cc80c01cb0e6c11b3e82fc15210c60524e0e362c855c54893940a62e808a3cacbffe16b4c9fb8167ace520b49a1c23267797e9c2b85c3bfceda3ccb4bd8c08d27b66c308b75f998def4a548b46421fd62d48a1817b5cacdf549bd91de0ed8d338eff9e60650976eca8054287ebf1e5830ae8e357153f91ee2b5c0f2d37c08e905d72bd6bc281eab034a349680563a6e843589085d27ca6a699418247ad4aa88b35cd0e217bbd329220d112d9db4ad30aaf849db12a6f81807a6b2124f92c53fdfcd8a87ee66a4d7a05dc46dc27e57133891a963c5cd96f7f8a5055ece23512c28343f23f6903c5ffe5935b4d48ba65f6ce22e0b9674c31d65d3b4468161b417cac25bc0de6ae7a86bd30c4213a0b85bfb14e52ff253cfb217287a878f50592e2ba846bc4f33e6ab45ea75b326a6cdb0b06af4590215f2126f30e3ab912f4e219b03b18c3768a0f30d5433db68b1ea7db4e2411dfca967383cf2d251bf3c629dff58ca413dd2cd4e7e67d2ff2c341a13204774c853a5cd527802217cc146967574e1f06489196e92b413c69406334f8e8fb9fea5f1bfa4f44ee39";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 457;
	dataLen = 4096;
	combinedKey = "29f18d0e40cd7ea737af3c9147cb97bfddf4eab5cc01246894182fdce042d521";
	iv = "b39a8af35dea9f7ad2741170d37335ad";
	plainText = "66a6e85e8760c8a64b977edf8765de15b51eb15aa55ff38048fb8a02c588fe091014857baedce5cba5ec1561d1a6e91391a7acf5516d3f4f5bb6595c90822b61a1addb823400744a3af34af4b5c6b7e2a6e85c39d51803da79e9385e79e1f44622aef6c0ea0642df08d0bf39e42177c2f309d8f55f6600151f5fa19deafc80a4b337bff7460ffd5936838f31c058dd90f1a46acb50d2bbdeb28144674b8dadfae367742b561d4903b8763101230cfb0753749f1c019dc37e511f8d2caff764d90be82f03c5ac3665ca9a1b2f994c3cf06ececeaa686d7c903ccd218212c7b11a88860e4c38447b004ea3e9a3cf73e16ac457f6ed8d545f338fec8f9d09495094069ba25cec6e97fd831ca07475209bd6e955845bb69784a6231a3f782443fa02015cd95372cef3d05d3f8659cb355f18d97be195ed73751ef3bf396df3cace1817e41013c7c7705073942029fc802446040a23a77d7a8a15955414db7b6180943fdd2b1c492c988a6e59ee56443114d063cfb1b10c7541bb282add7021c564812aa12f303434c9ccf666d9d4470d5532c6a45776fef52dceaf35daed096a2cfe4bb437b8e3d8d34062c870bbd35eda66cc26b5c842c8ceb27e531dad94ed7ce23108117f890156d02dca726e4b74e5ee09a374fc1d4109f669058f494e6a943fbb54bf784637c8b742195977a551c5ddf2a9d4eb8d8f57996f8de3c9be23545a";
	cipherText = "2edab1342662def2fdbd2e973f61cf9c65f5b9721dff09ea8d87c038978c202c4ffb6c1143c9c4b446889b0654b3cd2d4c3668acfe502d5473dc5761c15fabc88d7dbbd633c3eb454c096091e22c4a8418771bc7b75d38c469f35f0ea2ac2ebcaf52cdf6f4623b8494cb3c0c57732cd558c3a75853d5d0f4aca4322d846b750c80272169ab1adcc6fd8eae1ed97630d35c756d30abf7a8e3c59e4f426b33f00a8f85d60db4621a85669bb786cb84b38db6c314d0c9eb0830c49de8b8478f01c14fe9284d9cdec3fc092631311b6686f3210cdfa25b6c99a60a764034a66af0c3def5226ba82ca2237e8fede71eae281d847249241e8f66172518fe71d8e9e89b0a7bca6ba0b08f3782586370d518fe47a5af50e8d8052d7429998454e9c1a4a8c6315e9a499678be23afcd32db610128fe57cdb7a0167cf57cdf2be6fb883d1df58abf28f64f7ae5ef7c57e4581915215fa3d6216d5191a7db2eea11389a57c851775c7acd048825f624d648ee43dee47198e0643783d99a7fcb08a785623a95a0092a62615b3a68dacb893c3a141d601a8dc3df91c3baa87331e9e3115cd9cf3ccd287f1d401161fb217b3c95dfd3097bbcd8d529581a60b4203a96cd8488646e69c4a788f5e0dd9ddff4ea08a22ddebcfe7931a77b6363f838a17f75dbdc2d05ba2075fcb58f670ac38cd34a340c5c8216b4b8bc17041a09134bfb145e8a1e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 458;
	dataLen = 4096;
	combinedKey = "7034da88045bb0150fe101eac30e2257826660ed18b47defc4df7913f8cecfff";
	iv = "59fbf865b0463cab88d8f05b43a2bfab";
	plainText = "13ea3007a583109fb6acb80df19dcce132f5544787583f8bcc6f2d90a2cdeff745b4011f35f098bf63707e06b2cebd1b61477063087ef170d74503d0e2ee2c797f4529af88f4ac999eb33086d5c3906f5ed46e174ba4eded6cf84d63ccaf651f2dbec088c6ddb94bd0f999dfee955f4f623993dae1058fdeb563f3bf6a8c3331982dfdd212f06dc86481fa9a7ffec756a1bc4dec1a5829f98d50af73b80c2b3adc8764e78f4f2e050ab45ab4917f8fceb63cc57d0a6545a688d5fed3d8f54e94e463981dac9949a30ed23f8abdd8e5fa833017e1beff7dd4e77ebbad0fdcbefc5b3fcd88634053fd350bd1b1ac7ea11b6b6f9db222dac9d66a982447a7fd1d7b0d99f12049d9cf8822ab409c650d8b108c5dc107eb9cae9af95f35bdc79536f865b1af7887e663ab90f37b500bee1d5bcf3323f7e800cd84f7109a7e4ec2fa383937f1421e861359752f9993abe879d44aa1020bfd943f47d19d212402497ee0e3d709f53e489475dff179966a5c6cd3a759b449b2626c708fab24bcef9e5e3fa3cf181876a5c7192aaaa6a01da765d568674b2900d2c63f4c03022286ac8dd2e4877c2a1c928577073641de31b097479143c0cfdc4dfeef04b3fa60146b143ecc7e8088a41cf4017721dba440517e60c83b0c4aaff40e2dbbfdd5fd2122db674be78a2a0c908b97e914dc01ac76e2fceb944989ae154fde00243901dde3cdbf";
	cipherText = "53911a15353845b3ee467e0aad886c20bdbec21a1828eea0c733d216d812b1fb9c554c5aa85ead9e495b7162c23f7d0c053ceb98a5714b57f23ddb16d3474d9a43eb4666f0a419d89fcd3b4ceead2d38e9c963645a17b369234677f9a0a379023d4e34a1ff3625c6c353ac2054925b7776ad86635af9dc131a006fd6b99ea8f8176f7b80c867c6e67e66bbe996c4271a9e4fae716ccabcd82712c43959c28c10896fcb692f75908e677da35cbb31cbe9b69ec0b0777e9b7309890227a564de952b8c6d453c714e0cc6b65694ad1b58641de048a3590c78e70fe18f1dd384bb73e25d5c4696bd41f8bd3e0192c05b27c9caa77b06721153699ca8b08aee18cc44b6ae4f24c91b9576b2d40883627dd68b39792dc8736bc8e45bbb1f8d76d7662423af4d5bffc7c4aa7ef638dcdefc73b4481db8333d9ab099bd3bd5b95ac36169334875ae3b396a6a3f44bee88ff8fc93130504fca7faa70c967ac8fdabb2b594056dd785dcc015a6b568a382d897564f2718622d4f053ed5e9247ca6e3ced11bc591497b617c9ec2d53ce4cd6b9762756b8380453c81b49576e5df661c14b15c60397e875e44271d06397a3172182d56145ace1292cfcd25160662be4ed96d0928c47cd12bb7b163327513fb36d74b1864833612f93c3b17b1c7133dbd3dad7357ec2dba5835fb92ee5aa9b28cbfe831a423f29983a6c8e087326a7ade277d7b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 459;
	dataLen = 4096;
	combinedKey = "c91a6b17996dfd5cf72e48b7b46191aace9bf246320280d19be167ccb675ed7b";
	iv = "e0ed2dbcb03d60c213a2b9b51ded2ea9";
	plainText = "e7e9b510687bf5b7890e77b2fbb95cd55caddea201544d587dcb24ee88d494930e636a9465848419f8a048918204de2c5085b118fdcd55bb02a57aae6c7895e565197fede41cae889bc3b79113075623ee8cb9a895162b58920c80875ff98452918b556217bf417b7c3fbd2252f368e55f30a813c70f7e5a6d99e48c6c890912d55ffaa7ccd92e032cc9a4d4205e5a33dd6c44c2c0b26fa0a3c76ccda8ac6b3c46f009f2ee497a6c76937c5f52f258d177c5bcac2b23f768e62e35207bde5d5dc3b0840cb382b827ec2d76f5c120f4611e270e55e3884355304f55a73d08dca0f479a18b4a06af10bdf3d558db176937e19adf311f50818e8df92bf4335d2fdf6d35c0f6a2486f5999c624227404ca2a52c64e192c48e77f7140e87f6f9bc3195f4eda54360f836ade75c682c47a804962d4efc24559edc7875dc952ca7796a4ffeb5ceb6046e0e42a7dacaee7eb6e841235435ea6b05cf04e59710d804d21dca13a3b93f4e7af70d8441721c3c3b6564d6045b3adfc2e20bfae9e01d5ccd19afcb354f485c07d61cdee0d5734e1250211994f91ede415df6a72d2e569e3b79ae0b3cc77e7be6419b545c5f660cd1f63788162a109c4809ce302d764d83d796f1ea1572976e3561224943c0c7372e74f6e784b6d98a1d6275ac4f4ac36b8156ec3f728ead09e3e07518c2958ab0f339c0e7f2b4959e61400e3f1a7ea5495bb87";
	cipherText = "a4ca95acc878437c5521e18dda89c456e99d8e4f0da9bda9398ff9e8b147f8e707fa757ad7259493d19a6dddc232993810eb3cfa01fcb4e7751d4b21b44a251cb37840fb12f9cf892deaf01e39d65db6c7e7f9fdcc9014015031f856707225bd1894b2e6ea74468b3742b714610d99dbf79f52cce4e38ecf21a8bda075929dafc39d6652f812c1fae5c10210bc735123625928e685f38a440548acfb54ef64b76128714df56817606d8c8eddd98787f76496c7e690dfbaa1292130c85024448d644c195c25b457334636c6ebccfd0067d98fb11e86276ecea2f3915d3756673d8236f78436fbcb9020857cab2122db95f33fd916693f3c405b497fff7664fa129a9e2d29d803ccfacc9947294bff1c80a40493e93d8157928fc49862b17e4130b8b276ed1ca75004067f3dff86de1dfc4336b11f60e78c295adf78b4e46570a2e3d605c168a7e001cabe8e0e76c26f13564f54e7fd2813b7e7c95356102e43c4d9ae8e3459456e05daef01f01ab0e04a000f1e4cf7c5dccbfda7b4c811325cbc4ef2403af548aba2af058d25653f8521f34c89cd0da32dc13e5d2416dadd675914cf4fb14c16e4f0eea3d6998544f3d0b0b2c5f4f5f4310f6573af9b5bbbe785d265137afaf375cb39c7704cee3c0465b3bf35732da27e40d551d96fda98550a7257ea1417c1935cbfe5704fab0664430178b131c03634f8fb62f77705927fb0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 460;
	dataLen = 4096;
	combinedKey = "98a48b9948fb4d4d70edc4c96010d5e5d6d3cda3b91199cdc50ac4a48031cb81";
	iv = "c91b8605408a3e191cb033a02a3b2bda";
	plainText = "652509ea06b98060bea17630ee56f13e8e637a945d6365fe74b61628a3661307cfbd26a1da6746911cee8de6ac9e67a7e628db383c735884aad5bf6c5c88de26bfd3a354dfef6e9c0874182e20a9c884f5c9eed62147e98db8fafcb2822bc895b339da52a262323e6731ac2518267b9aaedcea14329b2a549bfd4989c4a1cc050497dd33e1cfa3c98a1b29c1c52b793d860edb56be298bf4a4736c75c2010d68f80a44f94b4d4403f77ab3031d5e3f1a2e3da58b26c381e0523abda43eb3cbe419a45adee7ae0abc78a1ada5d0dc603581afe5dbd90dbbf74608a80a0bb5dd332faf218b7342906a2715ed11e8cbc4fed61250e45dc2b30cd0bb5f12cd1e0848079cb5c944187eefd55908b3ce2bc4cf314a2dab5596ec11c81a769a2195d41e9312018729c0d21614c178871e8de7a825880fecdf564658084f08ccd7768803944c357a4ccd058335167471c01a68c664555b2d4369f320361713d02ce5a537c0cbabab99a9a6f505ac54cc005453357943df88b8fcfb7926d80413eea0488b36fcc9f24e5dfeb67cd5c51fffb6fc67a0c58db8d0549f5bac732c8badbfd28f949e0c84fa9598df9244630ac1849d53b0ca332763f42a4114a4927df22ff2c0761a6a5498d4c56f13ed75ed3cffc390323d6990d6f2d48e9e30f95ab5827bfeb74bec8c682ba3d9906d89cd6f9de5ec244defbeaf589f9b96547c0f4c608c82";
	cipherText = "8bd2a3c7127e9c9b4dc04cd8ddd1b1413acf6bd176eab6712ee98673a1fd29a705883419eb7d0ae7a82392cab969e277b8878c1b4581ec5bc282cd1927576490fc134cd96c237be3c50849d6c3092209054123d9cf98bdc1dd76b10a30e1da7935534b33b85c2af3b6d5f539ede89ddd8b9fb2eebea84f78cca087f97dde621b178255c9a4301af1aaba1a060946b788e5cdb98e6b155dc613c8e88f20aadf118d997d8b5473d2eec85eebf785ef39d50f1855b7445c69c524a12475096a3e720f80014f4f582ca8ab30776eb0fca55cbc9f4cb742bf9aeed29079a074291fe67432d9661086f576f671da3cc38be0fe94d01fd3137af7783ae0377cba33e0d738bbac5897bd4bdeb07720224f218f7fa3e13d8b0b698ae09d6b76b58673338656bc46957dc993d6b518345d3fd79bf8355d9d67321799637d4c142466f48b52c917a9a290fb3579f7320d01a7d106df5b92979bdbbd7a8c05ce7a4d412a5a8d326850f750ba68dbb12be2b9bf5271a4f4a58bc9f702475b6007de363393211288298cce5b4f17822cd94a3cc9949bbd26c402db0b83a35bed25f930ccf4fa5dcd3417b84b21d1a86cc9666e21239574cf0f60d59ac44acc4d2b83d5cb30ccfd19866e75449687966cbf05a56a3ed17aaf545c6f8727ac61ed24958d8743cdbad3bf97a3cf5d614c958b339c84523d58ac7acdb8c15a314b73223a5291c781f3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 461;
	dataLen = 4096;
	combinedKey = "f2e336f8aae1c52315971768e2670dd338f81d47774aabf801cf4c3aeead94f5";
	iv = "3367e6bc7f1fa378ce6180fb51da70e5";
	plainText = "0f496f6c39fc81b2080301018c9a03e9152674a3ba8e3b689640e758cfd037e5657220546214101efc9acfbf5c07042bcba69c31d4df43286a64e1e31d1c0fca1a9538b413577abd0529db6be777e8caf512bb86100a028088ba03209df481fd78c35486bb5b672d1aa0f4b6ae7ff4c5e4280b1d6d13df1acc65d7194eeb18883605090ba68160daf133320cb1131b3943b620778d9274f946cdfdd31675fbbe82b26988340416c25a3ce82520e6a61029709fda3fb8720a9c7a0db963a9d81977c4504703c55db86b2fdb626695bde21ebeadbeee1b910b0f2c8fc3bacf6c845ebfac4d797b1d3cde1fa2069b72a8ba9b2f0b18a50ec8b7d9e63ff895b98b66913cfaa9d249f8baa70d307300ab2c5386be73a4de829d803b8253c86b95c0e39386fffa3c23267687f40a76b4ba19b2510e21f286e22a0034671af0f1391bb31c8a9acae2aed876e955dedb01923f28a24c1fc2a3df8e1b35b19abe2b738b46a84a3c6f2e4f07a683a3df3db146b96235b4c08fcdf973fe72462f657292bfa305cdad65ac8fc8c73bdf39d8c6f2411979109e278b3a14cce20258b3bdbf906243d15bbc4ecff527e7ebdf2ba195ae3e83129a891316f459dd35e507625f16d271b09e98a61af7b8eeb6b709c34a9c1c72e1a2fac7d6f5ff67c967890e8c6a1b233f97e55c6692fd2189b301af9d52a29275249f31907ea122d2d2ea8145618a";
	cipherText = "d809ccbfe2906e537aeb2b4e742a27e8348609ac54ea2c84078ff07d5bf18f99f9d639c0f4d9112d65b25be184c22013c98b39ebd61c993979b656c1b35676bd675b778129958cc5f82c0300e55b6df8a8aa3188fb63cd6f804726581e28862c5d57861c5c417b026c62a7eb2dc966f6913fc398f7d4ac37f4e1641973c68f39c7f5969563bb0d14b8208c5205f9d11403ac754a4e932fa576337532cb8941ebcca0439518a5f18fc97cfb38332dccb82256412506516b5d9064778f4d40c27038971ebb1b25c4f84039b9dc42ad4fb8ca1a464c0654e1208eb4f68839b10465efcfe290c5ee43a02f0ae022f98662e0f155be11df2b48fca1c2997adcc7f557aa066ec1d647a6f6d735101c8fae57e07f9081302b3e899ed73757056ca53ca5e7e97e0756eb85f5fbf2b6e7daf3f3eb8f70c015286ebdaa6d2e8754e3c0b4ff445bc8cd911a2e97ae886ad254406b0cc0b97c9379cfb68fe18efeb89428fec500c5e8ff104e45c5385f32cca70fc4740b717077ee913853ce1624cdb0ab49de5053bfc61448a206bf22e05bf7e6f192c9b12f0fd37bb0ec66f8b6248ce688cec18409930056bdb0d9db95b4581552b865e88b93bf2d1fc15e47c5de8f4dc470289f0403118cd572e20f23d3007f95a4afb95589185c394dfcee0327be771e0fb539d8bbc0ac699162def97f9326df3f030ad140992b552e362a7d59bb9fb03d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 462;
	dataLen = 4096;
	combinedKey = "5a2a78a686aa1fa5fd6169736dee84ad6913a0ef9a5f4b5fe50bcf0c4499f718";
	iv = "30d463ac6ec4372d040c854d56c54feb";
	plainText = "4cbf47244029ad21f762e9c2ac15dacccb457e24307f7e104db4164abb30d23c9e0e94f4301bd2ebde9d9fc5335a779cbc337bc29dbbe540155dcf6c65a8476a91834f9df1ede769f90c187099a3915deef876cf229116edb93860c4ba2788748220d5e2839db9dd0bcb5d076d0f2012fe2406ff86927bfd52a6333a56dae0c353371ffc52b8d9998257c7cf907cea27564f0775b33cc94d54f221f128abf2d0ffd1157da0bfaed314fe122df175bf9fed97c21ebac9008fe92f775001d3aa80cf54f706f29ae7f77eabc22129e25fedbaaa7fb769ae943f1b4939ab9decfe8c6b5bad5a611395016c69409eca96d5cd524432dfa93ce457f6c470cef59b8e54ca28a17a660f9459b3e3838d84b1abc884fba1b28ce84b7239509ed4e9b457a1b9432239d957c310eec420465b9c2eafeadcddadf830c450f74c9fff0a6a9b14067d7ae50edca7836fb1bd74f498a0db362acb152ebd1ec80c6c129f2c50c2d8d8d1d61618cd6b149938b39f7a21cb298bd8a821851b9b573c0e3c1cf9ac52b200eced71b1321c4ddd3c4efa8545a4a929cc3100093c87c8fb5a8ad423994c76bea4e30948000cb5e93d6225f6e9e300d0094eb4fbc9e9299021662e9d4e8676c7ba0f3feb27ad06901d2753cf7ea82867e48a452af042e6f0347a1ef30d4aa84920617cab3390e5a22ee4020989c011aa62b09138586abb4fd658bb98b8be19";
	cipherText = "665eaefd4cb763ea46194c135501d1aebc69ada75b60547dd4d343ea4ba3a4dfabe0691396ad605a886124119b0dcad9c9144090c167bbafc5ea8125b01b6660c2438f07e0722b21d2fc59663f9972acaa8091143359bf58935148985518f0fff61492b27226b0b93c18be7f7be410fc66248e98e885dc5ebc9ae3142e8568a868c95e94d3933cbfe65af911bf44714be1d4dc054296ecbb7caeeb7beda56b976a8996ba7d90f5d3d21983d2becdb0de65e999efccaf089a1ee37444cc7a9d3c47bf0bc4cdfc0735f15c5ffac37778c2bf1c9cd58ec8031e7c7e5604efd66a309d499f097c668eee02e6e3cbfe83bcbc9bcb279e7582ec61a96a2623f718f61d5a2cf5f823e3c7ba28fd2523f54da898abc1209c0401f96b89a6995c0a7b860fd8e62a228a4b2b81adb9c238777322dc169a9301e457f7b3061267edbc7461956556acabe05c0b4dd6839a88e3d61ac1ec2e326ed6a986324207a8ce1dde5c45310ce2394687766c3d7155ba3837fdfd0bbfa089b77cf5488c92e08704dc21b417e5452ebd6bd21e1ba9758def7c8528ae15b6ac1fe7d33a7a026b1c132636fbfa64b3ac94f1dbd65fd171766007bec397e1e4f26c9b7a729653418504c1c65ce7a9e0116cd0d9df2c837cd016e0c7d38ee141a7c6488f0f5a14d1cfcd54c71f0d21d4229b60d4b2e9608ae930358850221059cb698e7441aba028a0541d8376";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 463;
	dataLen = 4096;
	combinedKey = "a5f95f844fc48b4db7afe018a8d5965dee6cf6b4124e4607c42781afb9ac3859";
	iv = "e82f8b9c4cc74f1163b0da9dabd20774";
	plainText = "0847e6f00794f8af29631501fa9c6019d8d2cb4f688104cc1336db54ad4ec07fa49ec5fdc8fae4585371e6e87cfb6bc19001ab11ee89d0c61c0defc453371f6bd25b14b26cbe144f3307821649db91a257dd8a658ed787aeeacc5cb268ea10822c69fa20369d476f15a37943d6f3628dc7fae3a314498bf4c08d6e6173ed412e0e1e6101f5cb354a2c82e5c3c3fb61d1c21c974924e9432f154cd3c840dfe7a6ef8d3ca912ba98a88ea807c2e085389f7856a0deada09ff58f051fec2cf52c960717f75a6b4229e02b2b4de14c0f26c2fe2322bc1ce0329db1edfac426391c12efadd82d8283acf527a605978410d563179064ad95dd07b89b2bbcb0f9ebc108492f9d73f53fb9947b2c055a46a0b666e8e4f2dbd454efd3cace8406f2e0aeada75488206475febc82775f9c224b1f0e3f0a1dda909eb46b53c573b18adc18ad1e2a206d45582777206c88f970779d145577f1f3c0ce63dc277840030ccde43e858343a6270aec7106c3b6754dc82dbd8ec9e39ae5dd3b52f8d796cd5c6817901acf50728c8e389211fd9f1134b46c2e1a922cd8ec61193a83a0a9def1e3a34961a3ba5dc550524f2beecdbef870567feaff2d9143446647b9509f5fbdfd65dd6dc7d996ef829a31c77ab375b7c3fdd6f12528837dd9c59b6032f4282e7f10f8e9bff4cbe77139b83a3cba11c291f99ebcc81647cd43dd871bca7f2c6fe1926b";
	cipherText = "f839b772fcd08229951a8d7411dd7c3a35a02a43602a94793ec52633f9ff8d0751959ab36746f753b860b84c417d0c2d4168aaaaa3288945f0d1bf4854503733801ba9e65d3095cdf0cc2a00e81cb287293c94513c1b6208c67bc9221890212aa6bc0b7726d40bb4e9ee527d7b065b0ab96b0a46d27b019a7b8c278b360a391262344629126cee89d8cf31c1af93e63a5799738e1d63d21bad2446793e81cb7f8b90e6fa2e93648fc149eafeab69f9a66c85cb3ef7500ea63972ea90a211f13a457a9671c29489ae7c12a322b006d66a009ce4c0ae63471b5dbce46fe32009ca0db0b737fb2790d9ebcf37fd27521765e7461a2f29290e516036757db15e02b19a39b8d9b751b9eabf76fa77b067b0e678187d0051fcbad4efa7cf7fb26f07bde7d9d374946245b6719cb40ed8e7ea57a0190770e63cd04059733729eab4a7ac122b3be16694bad4f281a341ab4760ce2187bb6112a6cd867f643093c27eb1e102cfc0ee68b0020162645162fb4022bc0ba52ec31111a8364a1ffa8f18067891f8ab0715144581e374fe6601eb7fdad945aec27513ff849cb7d0ad2f3d94f03e59ecc42b2dc6c909dd38b09fd7cac2a25476df18ef41ba361aae44bbb370a9e8f3d95d5dea1c306199a3a0552452fc59a0555a27a16b4eadf176625539047639f4ce22466455660fab5826ae348608dc9d83bb5452e397182469e82accc9f91e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 464;
	dataLen = 4096;
	combinedKey = "5ca371eb2caa563db0c2bdbc358fca1b2dee4d7630df45c9daa1d424c43b0fcb";
	iv = "ab0f8d65e716ff1351d93d36ba24e9ca";
	plainText = "7d6f9d8e65169b2f90da33bd0bd06876598b84d60e25307a38a337e8e583de58e66bcba09fa79c8dab6f14c91382e092c6047f196791774efb769409a8362d7674f790a6486ec0ae9c421aafcc42fcd8118bd3c66ca5e0ef305afebd6a8fbce48cf37f68c10f942bd544a1a4221480bcb901440b788a8b4ffbe158d6a19cbf70afeee7dd58866ab1d0eea76baaca109eb0de70de25077048bdca4d325f51451ccf639e1c65fa40d359a5ea852aa8794536c07bfb5a2cca28210673db0830298922e76a6f1fde3f31d389756d6bfa0a6f9862afc60f554c797e295b09d3eb9ed72e3858a08fa41adf6f03d1eafa212a81b725a1aadf424c461793dfa705ba36c536d4137f49c2c0a61aa3b1009c8672441c67d0ceebb8d4096546fd341a185700b3f83771d9bbffae3f7d5263184e2a16366d44c246dd8ee7d4118ab3bcb9b8c824949688d857f5c5b19dc6e667779cb0b064513d5b581aee237d4d48a7a60066ea674598e72186e05dac852bcf4e98a486269993566a0fbbffc817e13fe150f1142c7b0f6f1612096fd1ed7e2f43e34a911616b5aaae1bc21e0900abfc23bd35d7410bea5cfb891d819819872098f14e3ddaea22d8d18517fa5d099623f9c01496162fcc6376aa1f0f78be44cda6e23e1d022203720e71855c24c5441cda701140b7334f73ecda7e93f79ad05f9860df08c175d1808b6ff3c0c956baa45a1366";
	cipherText = "0fd7549899b307704c76d855165ea321f2280d0b857d6697810208fe66272650ddaec1f8af31f54af62ca4d919e118b1f36977e115ced849c89f7f9c744c0bba3f6088b9feac0722526513a0b082fbdb82cce185b2cfefdaf3f81603f4e6180f87186d039218a3214d56b8612f0e614996aa8359e138ac48ee7bdd05a5453b749ab533e137f8e82757c3ce9a18893d7de2dbe9b0f06da68e04926b0324716f9b27027cb86efc0477d46a655cfde8162630426d4312bbcb1eadb7087bb780a90754f1df5b9d5fb054b205d976c2da0318df31625326c89acf8ee6ab6b6e3670fee4939c22d89a91f8e69d7ac279e66011b1bd74ba4a2d602bba563e279c97a54d5704533258138519663fcf9f09d8d09d3ddc4b63e226cc40c8641ab91c62f0f1f9a1d4018cecf88ad7716f14d51605d2ade2f82abcdb0830e0643f40c718edcd04d6e3671774ac2a0994ca66c0f85bc10904d7feb43500d35b9def4104916ebb7c97566da260832372d96ad721523f777d1e6df2d5b96ffb1835ac60997e260650afb58f2dc24df113726732eaf3a149abca2f7ab4e3d04aef8c0d6641dd83a87c92f92459333e9178ccaf219392ca0342aa9ded6ab3c1c40a5f3fb7b76ddd5582218bf1d5b90b1bac7dca79bc11f07b17a491b916cdbf01ff1da8c556645e1b05257b56cd54e90e86663253ecfe7a0e8a4a37b2f2138fd2e626f2673221380f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 465;
	dataLen = 4096;
	combinedKey = "60a0ff7203f82b989471cf6bcd6bbf8a969de1e24760dcc05ffe8741566937d7";
	iv = "929ab70bb52d5ea5268ab3006160a273";
	plainText = "b3de47f0abcf8b416f364a1e9302dc94c796b44fd968af30495ef70f07bf6d360fc5efad46a2fa94c1cc4651b818944472563ab08e35c320f60f7b835f277545cdb4946090dde425adcd2e1cd74d5469ecd8fb25e598f7c92d370892b37e339ccf5af050e218742d4d866d9ab47bb23da258211181ecba7b2ad638fd152343a366d05de0298b49a11758cf41fea9812a38eea20c89bf78a427a8723d0b22ced2e805f9dea38e2437b996b9f162ce74cec514f961c48de6d6d2f199ff5df516d78097704d7c5e002f7e947ca3f82cfc90d06668ccbfba37c345748defbe4e1c98b68cadf93b420a227c0def8bbad4b154a5ae1c3ec6395a649976097cc8cc4fb6d87d4ad52b3ef5abe3d32fc99be736e1ce1c4c2b734ff231a201220e0d43decd16ad7b571a8a50274d783bde591232a353ca9a32a191eecceffbac395da44845728fde88d96ee56a6459eb8ee10936a730ffb391bd140d8512a7d381d11f3d65a57f4888fe7bbccf2edaf651416fabb7127d17f9a0b2c781d1f2ce8e6e8c4816e78ac84df8a61fad6808d340050fe18b89cb8efd901c138068b95b12a1bc314d1b707a5513135fde9a0f41eb71e561b6eb43b0f9baabd81e315f7680f1791310c0d3b8c040be0a3e6cf5c342137ec3620ee3cc6c386252d03e9fd17f97479b5bf8c26dca876e57b4a38fca20da6058f48b0cdbeb2de67ff1a07b188cc2f532f7";
	cipherText = "c58b5188cb69dbb138321ea4fa393236d00c00b987470e744ee907daa9b32e5f4de361a1e4ee516b8b10cf8b1c54bfdfa1cf0c0613e2dacbd3eea9174ff0c81dc2b836f025ba1ab9888464a7fb7da5c63372552d4033f2ec210ab0595b14bd37773a7f42a608162dc9120b1edc61dff5c9efb9337ade6b1e5fb7c188c8af457c1997e5b9208121e2472591aaba53101172877503a141aa691408d80716427be4d9bb8fb36f77fd26f54a23fae10517a6158e5e0239fcb704219b045e25f362111ee50c0230e78113ab17854aef224f1f80a0f5be3d533cc95c9475e6f76b6a00f5f0de6b8c08df1ebd943bf0dfbba57e5778ab81d8c299e3e18c6a34164fbf1ffc1124964f8806d48b9ce670d62939e93a5113c69dae64c21b08cea5e24daddebd739e3981b3a3b95a1d6cdb9ddb12e3f4aa3117c1338d2b1d36af6d55ffcb04194d42beb209bfb34aed2091ef29c667e40eaae45265671482f0f68b7911b1b154510f8dde68f8bf756db78e636d22681a914007d54513ddd60afe3b67a689b811b633b526cafdb4cf44af40bbef54887ee4ef63e6aa62b349892faf1c85c6483fff01ff66bbfa43a0693a7de2c7b5b7ddfe84b72a1ddfd379b744c2a34a73c331e47e75ff89a14271beb42b84fde4918db5dc1e0326a388d3b7802b9b895115572f12b7f01595143d59b2f38679cf8d73ca8ee76e3f8d8a76aa95765c1e2e53";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 466;
	dataLen = 4096;
	combinedKey = "c4bbc0ecc89841b34fe47f67574fdac5bbdf4aace86618367829a54f4196d22a";
	iv = "9da4fbaa4ec49009e2ee60a180798b6e";
	plainText = "741b927497f8410e8976b511ac2ecfdf0154341256869fe2769925c0c28b59994fd7cc1f6c0804d7d491c9cec1fa8c55ab249937094f7c688e3e0b4ab349950fd3d10689d21eee56eee92469b25fc987dd3bc97e8e43a245d17714501f02e380e9bef16e63716e7102514db79ddc092b4f312f024873d2d331ea4a86debd6faa1eb23fdc9e1f7165df6b6233e8710c7eb3a3cf81d7c04472a80b53f70550af427b4ff91a683112733f715c074a6ea8a4d57b9a52e70432f59efe7d175baa4aac1927387ad19d9e18aa1b03a326ebad2a5c7af0f99ccf452485ec84a3d7a82751802e326381ba8aa01b93db42db318795605a3b0ddaa08e3c6a5008fe2b8c35dba26fea47753bd99d083cbaad52bddaebcb8fb55099365ba85e3f6376387ff5768437db441b09823f34e8aae966f727ba6fde4fe6c6530c203914b3b53c334b2c1e962eb1a312d80b56b63e64a3ec51fbfa351c01285dac0439185c2240f7af4ab0143d2749b5a649132ae1d092219198c5b71718f121b9ba5b3ab3a104be8a665e83ec511afae7bf455ae32d3e69f9496cb01b1686f3e2fb8c325eec3a89c2d73410c3ed802b03e66d9fbca1e1917af5a8c1902871d746399fa4d3d0157f924ae0c106b2988da4a97dea4011e752363728929877b00c967c0fff56fff1710014412c267a764f992726de0f460d2c868a1f481fa8fc8a80df4d64f7749e156c85";
	cipherText = "0e513025100535827abd50fca35c9d9bd4ed98dac112d0fef4513423d9a336db9aa3054aced1e564929b807b8a0adc947c45a078961ae7ffd5ce289a786fb936c07aa32b3fbbfae76f90de76db0c56fe40527ba131946ecd9a63625dbb357f240b476d9cf85f43f20dfd8a7e14002d191ab577ccdd3c5016dc480fbe493e19b3dfa3c806163bd63fa448c754e7b0e56348b205c82756b1ce67c19545d2f187130c6c2026eb4f40d9a7341b7e6b1db897fad6e45242dc084fec97c8e7d4563e1e5b2b0290d1f8465c00bf5fcca441dfe59c125f40a50b91d12137c2f8b696fb571c628624795be2d2f2653bfcc0c84f729f5c4dd8dbbc3cf7d26f933b0c066b175266d93fc0cdad7f0e44b638179b79815a5484eb578d2a1dfc63e78900213dece9d3eb6093fddd046b8707c1c34022b9e9883003b65fcedb79d08b544a40120348f357c047e1afad851294afcafb2f54fd043548229c614611553e89e82007ed3fe11e8b992f5c54248567de59e3a99e5b305f3009d02c6242c3a1ac0db9dd8b54d1d0b7fc5c20e60f4e77c00e2bce983bb57d559cec2efeb3455ce5dbc44f878a6f268773fff56a97494c2f9e13448d8a5871bcaa1e7f7a2c76adc306b83c82d8cc669c7aaa9df54d753989b66d4aa8a208b9b04b762b64bcccab5b8c27348cbf4d40992748fc9f7ea5c619a90ab19743e9184a5574cf20df012dc7b3322d8a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 467;
	dataLen = 4096;
	combinedKey = "661b0afa2abca3b6cc03e7f659c05e5666fa2dfb7ba361b39a578f85aebee125";
	iv = "6ebeb177b44396fb6fa72baa3c4b2b53";
	plainText = "ef9413fd1127f785c8603caffd3e065c35c40fab5ef9a70a07d1c680fb411a1f69b9a3a59ecd12f30a43580dbc06fd859d23f7d9143a0bf7bf6b5794d46b5e7c2db85c5ccaaa90193d772788b54e965a580b1550a301740f1f405a1124b81b434b49ef1445fe97141b7a3d3bf5bb54a19fc42b4b66a25aac1981ba7aa485c7c91c34bd312121449dda8d7925cf6ce924862577c94a49394de1cb44ef4a48d6606b1b36d4915097dd3bb8202c58d65db9786e17857ec715db51ecd043bfc142de1bc23a7dc489738d54747337cc9ce0d17baceba752f7e38ef4b9e3f5678296b9437217c87b80f588f2b05ca3414daead1bf9e105e6503cc8b371ced78ed523c42f82b184fb961307ddb9d48195c1593b3e8280306b98c3bf02b96cba0c6287e0f7e3e72c471b1884146a999f0d0391a68f2b25b80f51ae024cce6af78489db73d15932f3a1ff39a41feadeebbf6219a3f43c0ba7c08600c5133a9f18b693dd2a91a7bb1e88ffae856055170a582ecdd444ebf7f7cd8d6616faef1510cba262e740d7608bd2bafd271b78a7f055f578fce7d7ea2116e567c76557118f32b42e881535b123ce3d9994356218d92e07a22917a83f51288a50c9c621416b7c0e48f54f6abcd54e07ca3ac96715d30d19227e44f55f352276f86cde1d826ad94afb666822aebf83837a0c932fb5082fd3b6000c7beb2788afb6646a4fc9e39b479206";
	cipherText = "48e90d2da2cf2408f969102afbeec5bce1c26a49284281468943275abd9892d02220cad6e3b995267ab265619cd6781a84f9e79aea95d703e71c6a6df562d8776193f12d8ec86b68dd365125033f6ad29f1986c80c6fd819a1d42c0558740a7a48fc151cd735312c648bac98dae6181d646d33f1953cc9e11c6a342afe2b51b31e25e635581d5a045d7017756cf853460fa926cc1e865c8585672c81f556b7709b9e80b60f942fe35bd0300410784cee3f798fc8be82da5945cd3e37828d847e9035caed78bb68a29348717ee1d17bdefe24e60064ff81865dc59f8cf9d3596814c23f50a476483e04d854e2b81d0eb8c76a37e399cefd0a96633d552978b1314b7de6774d5c85375f3fa93e2bc764481986d8256bc98098a9048e2b23293162ff08777280e931316d7f3659de1eb40bf487850ddc36a01208c7902547586ab5fa8d91ae42394d636f4bbedb0190481e63a6302f41e930344f83174418cd665d6e3b5d610ab17d126500b27a85f3469bdc0eca33644623f02373377ff15c77373dc40ec8eb3af9339b4cee0820d901860d87954d5103bfd157f852bd55fd20821c7a508dffb72f709f36cc425c1c26064121a8fc5c65cf3d9a9ef1570bb30f29279383211eacb52d62064a98263eee4c311d9b16fc04d6838d165ee91060ffc2ef34e0967b7285840a215ba1122837037d7fdfae8c9ce518bd90fa5d6dc4aeca";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 468;
	dataLen = 4096;
	combinedKey = "26f33405de629328d738b23b209f65a2e8f43de6c25ab469f4c4de13c2cacffe";
	iv = "f4bc9250acc14b8d07350cc230492ddc";
	plainText = "979700d146e4cf95cbef009eab5341f8c1d59139ac9dd8fff25340bd259be3127239b487a05b1ade1f020943c639f781a9cb0f8c4275ca6902b201f2c5bc5e7c31b4c71e9175383f3de5384e41d2a93384f8aaeb120bc894be10974105acbc9ebfeaf6b79ab8b3328159133a7ce1e950450915e48b7f4f24c16d4eb6dd2bcd1a307d05c202e5035fb9ade8eba3c02b680671b1dec7f86dc365ab5dfdd6f9d13bb422af9956369bfb919a734e366b1717e2436da96a45d74fa6a5ee103f6494a725b33d36e5c7fa076193d74f449877f98ac412d99cb6e41fb0231c76ac1cfd709b478e15c1e3b2a24e4df5942be8695bc989f61888d1316431d7b5c400303eb98dfdf198ea99b8f5d9ced319ccc164544fe3076abf1cd29814cb429e7f11b27af610d17d4881712ecb65223340e65f46c10cf3c29c4566d0c67e433a382a4308aef80d1fa0db40b7b5c80a251ab2fcfd94877053fec9c5adc4435145a3da3c6d4447c5b676662899044005b880227798db175b623928608bef879e0b2bd701ce39097ad709a48df01e28bebc9be8c5e4763f3ac32268fd31d47d0cf2f9b2aee9acc2ece384db6fc0491dc4e13026a17a7bec98d30f235a5b5176dab13fc5def0b2a52b7deb04160be8bd80abf63025c58f4a2e6c4a022054e30e2f98b1259135391def647331276ea37c401d7e30db862f1563ab587104cd52545058255e9ef5";
	cipherText = "bd6a72d59be34f4b359f9a9d9cb80647cca8b137d743fd0e4b871a724dafe8ad57bee490b5087b138506e05866e73a547b90fe305ac5a7a1bf85b284fd98be00c3bd7478d938c68d05174aca7e0558f3e0f49e44bdbdba5edb8fa0460228459b78b705db52c4ec4d5f61d4e0ba7b043940f23530709def865a8de2c04f8b16a9676559370b8227618c6e36f31b966839f7146f5c99d6737805768dcf640d8fb63c3496e35d23b14004940bef9ca7f2ef7830a92822ab927ad528e8af983884cfe8d86bd4554e14e6f44eace86b748078054fc31d0497d43f777bad23d11a9b9b837b57272ca6fce48f601be5485dcd2bf4dd88b560f72556dfb22f6f82a3124efc2477a29878cf7de06691303464cc1959c3440e54a3c5405508b5dbf1663a7b9bc23b989fd58dd1589b6e656934cc4ab822c3bf1c870e34d38f9df9c1c5902681ca2897ffaa92f23711bda65e953fca98bc3d315532460ebddb134856609432fe3ed88502c5cca9679018622616224c5a7c52e8c2900645fe9514054dd96301e1b307caf526bb2d6b168f6391c0e6be254dd6af2e31f96ca9d3756a2bad4e51e2cdab36c98f144fc6fe0c83c13338c1cb756bd6abbd78bb6bdbe3bb08dbcaabcb817bd3308ae779c831fc854b0814fcee64d92da01c110b80500aed3121b35615898718ce97b1547edae2171fbfe2de459e51a4fff476b879210bb6b8bdd6a3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 469;
	dataLen = 4096;
	combinedKey = "2d8b1d211c1d93ed0d249b41b124092e7491f2c7a80f2a3cfde091dd1224fe1d";
	iv = "89ca1de64ee3585ee4358723d45fde64";
	plainText = "ac8ab12e5af23678a9955ed0480e5c27fc697af2ff37d12aaf0ec321e525963aeed934b870ea6ce98b4c9c9df0ca85bf3c2eec6e867c86a87592272e6572114c519fc0cdf0f98cb6be98cf1b1fd014db2c97d67945c357bed44932f9351b09be2316a0ca441f445747c15c84ec20cf649670aaad7b39ead6b739565cfb1867b6570d905d845260dc63e970a3360cada0c144a791e9a45122e6a4b48bd9c3f6d174c97a77950c944726e3bebb8db53e2a90aeedfe9a69019423045e417ed46a0bbd12d98008cda2aa8aed04cab0a8b3e0e54e5633a6213a2767f1975286553972312371662e841f43fdeb83be3b9e4d390dd9d5388e5c0d1eb4cced12c76c8dc112086bb386b947e3dc95c3261fa24885b6deec69d49ef1204682bff05426c10366915e5bec532676f581128a5146025432a53c0b38052834741c51a2c70d29883eeffcdb43b3a9fa0cc7d55bc2c3681c6b44199ad82be1035f98d2ddf7eea5c9004222d4a3bfe58224ed8465bc78b7dd21b30c6b7030095838d99137a0a8bb05f35c9ba881cc3810443343a44a8b2ac3c69c7bb85633bbf6feb355a179f15d0415fb13a6269266f14106a6fb4a9719015a63d165d2f113aced1dd5bddb19bfcc9351b71972dee3289277628ff9155acd1f4195549d08518e78dced55d180dcf732143b97826bee4d8c8a0786ec0b59e9b6eada673d4cb94363e41049951996c8";
	cipherText = "4613490de986adab4d99b72bad96a5813f7c191a9431351c5fd72383d4e03cf183cc575ec1a3c06db4f6b869d579d0f945f7bfcca8a1795959b9382fc7190c3bea382f104a1ff337dcc0082259fa118b6f8fb09f37f6cd0de83c0232a56c25213a3b6e02fa9eff63f1c61a6c14e7bdc0b465aa20bf571a4a071a30bc8c880e23143e0dd464362a232ecbdbf3759c9df21b8af7cd9f1c039cd8fe6e1041f2b9f688c4967a6306d126cb285590abed7496151cbd23be7cb0d5468a26e201e65a0ed369b026ef3e272e33db67383bddb42df4feb0f4a86c4bc5d2f9c1e37e64816eee27d8c1031614660e1e5257b102e40950c1c6be0db15b0c7a123a49c4a902e8347be69eb990ed68d12558f5861ad57f146275e41eacfcb64caf506f30f8875bac7556e9badff7ce580a052bf4cb55758f979a04694f6cededb5ed2e0b0860348ae3c521963363626738cd3f1f0db69c07e61058bff7afbd9d4d24784751df2d35536006a2b93485d439894698ef005d6b17816c1d1e8ec4e1495ad6b4fc6aaa3432fae5caa47e9f7d7ccb0b66fd7f949af4da19d2d6489847c489c24bc145f1e1fc487c5292bf0463c8b9ca4de408fe14b530691c0566fa4885066a53d35eb8f16a4a90b83e972cb49545b031203270f0f097dec5009f68795a39fc0bc80b9d76be2ad3ccaab4d2b8b3bc0c04b9ecacb71be90ff92c27f8195ebb421cc7fe4c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 470;
	dataLen = 4096;
	combinedKey = "b198c2f2c002b593a2499203fd7efac30d12a148899c4ae66d6fd149ba3ecb2c";
	iv = "e6d6cd2615c9171420a02ae7d8f1ceb6";
	plainText = "ea45fb17ab98c441a5356554fb3cd6133a7b6ac6b381b8675ab1e74c287067a47f52a9757547091d0df14ee4d8fbbfe12c5d2539e676a1d3bb9b0433b7370dbc84202c14fa5fbc5ea92d6bffa96297fa9bff00ae6db1206082348dbc1a9c94b62533a6208406d9c242f1b8e59f4091e06c78124fd2c750dfc6d5dcd6601957146d03ffbe30cce514a8f5ebbe7665a2cd58f95e4eaee8f7c513822186961776e498203f0fb0a309ef323346b89fbf1048031f80e0927ebe297df989747a935e10599df29969240454bc138c81757a0f5fe43a304e3ea6779a8660cffea6f18b37cd4eadcc156face0126137c935f435970e02b56098a131951490909e4ce794e2f081ed7f7dee70e59223d2dcb07de798af9775cae770c80fd5a559bbc4b2dbd804ee8ea1719d9f80ed9e3897746a8febb79a80682843b4e4862202692bed96603cf2a948a524e6a4ace43b2096788f4e37fb65b04dab4dc0a590bdd7d9f53c9d009e9ebbc9d78d02c6b9b8e1597e4d494e5fd2d4bf780e15178174bfc4a2ccabc591946df10e8e7bc20779918a67caa5be01eadf9df45b6c20c713f767661c8f59823ed8247a1686a46862c43d3fc594ae4fe0277b03e16747ff2aa574c543620d2267aba3749f08bea5508e6679c3fa835657a1670e6e2ffb41ba0716a5d716ac6315b86aa3be68f741b8a794fc500abc8223749e6e9e1e72f994487288623e";
	cipherText = "bc49e9973944e4808750e144accd5c507ebd61595b506481d6a3faec4372f8ae65bd450647eb91d6914edb09d439c895369217fc2d5eae1e9dedcd5f0b1f0fadb0d7d9f91bf39426d37a8b3d596f78ad866fc61b783ee96c693578b3bc76f602e9ff20d8aee4b045f1326add610e609f1ba00aaa357a7478b8d9fbef17ce29bca6faa9d1a9e430942a24d206588fb5bc184275e9d15cbdcd0a72469cf1fab3f5aa41ea09e7d35f38cf26c4367ab511777debac0cbbe2d61a24fdcb86be15ca27d8ff62c1921c7dcb9ffac41afdc2cc7b2d063cb511aa080d3a37c0de947d7bcd3914390afbbf391f056ffd46954c069125036614d0a82ca4c8bb521dc1277bd74f734d88d42811b9b5264626037e136a2b5c8af996240badffea5768e5440bd9f7b84d19f37be4f960eb8dec3ffe7cbbbd2a25544701987919d7b637eb6d2c180cd9e204117115257aec8dd62328a00f43148329e51c11d943e8cf1acf22dd53c4199c8229365f3207d40ec3c32a00d7f310e8b50b12a6431af2a9f59f8e6f07f99bff1fc84fd82612558bdc5d4ad73aa919f56d962e6d9cfde7a91b9ede43787b1c8db0c51da87620eaa437d7680042b221b9c6a3ed816c8f464c9b678cc9ea82f2f3c6092dab2557cb215e8c0cdc6e6cf670b8208ff6b1218c4318320d6ac2b1aefe6551fa4b8b6486339902b5a4696238a0f430e5aae5208f5b1f8ef7a7c5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 471;
	dataLen = 4096;
	combinedKey = "144e8e52cd980dc4871ac94fbebe0203e534acadc7c52dcfc657e119f7a4e712";
	iv = "90530983c1b9cc6edbe3674ddbe0f0cf";
	plainText = "9dab237b451fe77e9ca9204550f5ae2e0f33d3c6ba67d96c1242f0e429057385656514e81fe8609521fd05c204d4921c1772f0dfe1136d8e0234d4e0b62c6e2581ddfcd81d5457e0cf64958fac4a49a0c2813a983e3a464d2d344fc8b449bfb199190f14a6a76ffc9b2e91062eaad9e56f8a05fd9a65560ae19281f882306e05d1f426fcad04d3495e2807efd9d3d111d8aba43fffbfcfb3666baae04d2e22d2ce497a3f115526d10ad1477a63d67a93f9ca1c4edbcc86bfdc892667dc0ec853f3f50871fc87ede7cc3d332ae53489e1aa1107b657bf5f37fca8debb36c370ea0aa0240a2b85e5eaa3af124f6f517178d7802a1a6580c60c1ce3532a529dff16a27b86066f8555b859eebac6ea102011127ab7c14441b051d6f59cc73c686b854d6489872b58a3b48a5c491cb6a3130025864afa9bcc27e107d6fe077c9f69adfefb00c8c2d489c49127364ef5bc57cecebe8a40e6acb5560772db3dd4d6642f5520562c5ebd0a4712b6f589ed6a6592aa800cd171b01c0f8ccbfb240b091bf0fb8efb0e100532f53716156691e6038d2f9eaf4982a1a24808eb2e8d6e9391c8474a6e0ea8abdc39a4d0609852d3284cfb6c1f235745bd2525796ec249757be8e16eb2f8b665e1b0ad273e6fa1991cf2b343f7f59c5d123b864a50255060f5ebe779378a0ae0b652ef8a2d90076934067127bcf31256c6170a3d09699c870b9e";
	cipherText = "656f93a2fa98768d0108f43f754e6fef89ed480bbb0f903b7c540649a397b7310170f86f9c1c634939392129ac6d22dfeba7b18f818a87399e31ce1a7c13c64c0c4ec7651d25cfe33de21c7a32cc5f47a034cefe30e8bb0324cc69c7e31d6f60e33bab707c1f59d2799c2fc4ca79c1862cf9fcfc003ee5d4f1737d1dbe97ff54f99f6520a9855531690eda75d97855820387e1cfb6962c3ba3a473f6def1b39f3308d30c57f210a240507e2498291588827081ffcf4b2e7394c61a890061f0d0fb63e759861acedb9978fda081d879ba1e24997b49efab532caaaa3d8bfaeb6ea2f13f83f09237fcb9bac65d4c634b3a4df94db66230da7880f362bbd9bdf7ce140f79f1450ae0366753e0c229f4ff07f5d06e6c16cbb5e38928e205a89ea64508cabac4434fceab65ece2a65405400971c48b8e40d0ed1481e076d541f35552b51ef570da92bfd1a6a79a165bc64e02c2e1179c7eb8751c6da60392a73e766c4a20bd99f6f6185a9de4687315b1a722b1f31343681e589b92394b08c3ca97511a5efc2732b4cda330a89afbde2bd3ca8406f805516bfa52e613370c0d24a2ddff89cb5e49de19bfdfbe191426fa140020ed7b45859cb502676ca56f4249cd1d09e1dd8e7085195fae9a1818b1b1e4907e9a3bdeb0ee4d6ba0864bc585ef26bb5d860837f9bad09cea4ba2ee6d31d28761417441e03a80514e81a601621c9b25";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 472;
	dataLen = 4096;
	combinedKey = "b04f8cef0b2038fe2f9014edfa083c7add861f0770c14150ae03b15a966003dd";
	iv = "019d3e3fd06dc207b6c44e76a6a67439";
	plainText = "6ca805ee740a56919d6d2fc5a898e865548ce3b005880090f26aab062809697137e34d338e10b520d855f044f75e36f49855d74b1711ce0d5546a4285b1104a486ab2c21c6fd2c33e371113e66e0572b2ba6af256dcf3b37f7530085cf05931eaee5b5fe7c9feb12d588427815546ff000c29648823f4cd6cfc9fa8f30fe8e1a91f860935c5bc0392b10407ad2a011ea766aa24ba469eeb3e0fbed67ab3a325f17c8f02d0029f5ddc36db5cbc3e00a98d7a8f2bb278d186c0da13823040633934b763a0abccc737cfac34e1b71458d97e8fe4491ed8890dee29ee31a698eb5a573db26eeae79250d65b079089481c09ee34bab428756097b3def9b0e724d824de19f218cfd39638c5fd0dd1ad523cee85cf7786cacbd8f216f88cc4a9d6b936b4821d8173338beace4885eea51108fb3112f429a7bd7a5f90ae236902ba1f82e60d6892fe3a09be1578f194f5620bff3fa575eb1cefa6d535dae308b947292cba3ac0f928f58a6f44340d52fdb6a31d355bf74238a1960e6fe5effa64aeb985dc61ca07c4765fcdc45ec356650ad2855202eca6b716ea670858ac77de1a6b3c154843c8363635de92fb4a8cca5f88e144c6b863efeb5e5cdc2279e323a410b1b80cc56392986a05c2db94a779ff2b6e6a7ef536538f5af5c51113696325639d51e3ee54938f0a835e813c529f8643302526148b6c84dd85885a1bf08de3152c9";
	cipherText = "98a919987e0e2adba93e8e55b00a7350133a357d06e809c9643ad672470024faebd8629657039cf5a8a1ba8fe3a71524dd235fa70ecd5f41cd8c2b51855e71e4d4108baac4b0e1fa6ae1721f32134007b47028986076050706292cd9d41ea15ea1f282627706e7d8634964b63448253b9bcb53302d8ee5f534f3790a7a42e998480ac60c358b40cffe589bd5b588d27f43eb864acaab080cd6d8fa61b2421cf4b817ce9795259767af47502b7906ce079f5f50cd02b5d89acffced20e976f1e1552b802628e2aca3cd742e0551cb38003e369b86c9deb58a9a8fa337e6ce6a1e329430424a698c396663edd7a63fc879f754bcbe22378f42b32e2345c2eeca02a1f2815dd98b80499956adfaaf369781d3f6d02ed8281681fae2fbb3274709bf312f0bcfa39a1c0bf4ed88bc7afcb59e5bac2f5701efdd0374acf77ab00eeb2b4f4ab4af17407207b921500abf0f75c639ee7945b6587e854643508df9bc58779b8705219e7caf632689fd3e372bee2c015610b7b202d972278fd7a18a122c94895ea3c3fae655e144f128ab245659fe3b96134b9afe5e52f494a15671333f5ae1d6ceeb6b1ecb2a99eba9ea7d0d2d58aa74cd6824a0647945e9d5ceb65c8b776b58e411ddc516b5a32ee0cc9ee242d43e189caf932a2751224fdde65e506ecf1542cb152b8388a8b36623492bcf33d16c75ff814e3df497d3229d6a8b41ce64";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 473;
	dataLen = 4096;
	combinedKey = "1800668bbe7c57bfea7ce18f3b710a8a668f1ce748bfbb3f40aae4ccd8cb8fb2";
	iv = "6ca2fea2bbab845f5a88822d22117d85";
	plainText = "67c0291ccce58e1fc0994918ff844352a70b5bfa222c3c4eedab6dcad473a5b507c50d6f805629b65a7c14443491fd1c6c05e6c46bc9e82532016cb98fdfeaefe4268614b3b9f0023175a4e4a42d8a3d8f0f0430e9eb4d252b871add3fddbcb9521c341f9c82725179125c686d75851bf50cffce8e5cbdf0aeb60447a7556afa85522ea49727355406b658d14e6d1355279b5d4d8e12ca74b84780835a2e5ab8fc4f76db207d328e54dbb0bda34ebdd64c4214e9a19ead40749a1bf9f0567d861c908183382abbd74e4538c4d28823c4894ecb8eb0022692aad96c1a59ff81975c712d0faa3a8fb6cd056ade0066dabdf74b214afe569b0658c1b42f6303d17e8ac25c1736424d2b097c2a1c252bdd24fcca13b519b9a89635ec1df4ebe021c43882771197fc4c78227c8a7f8b0bf04aa1cef18bdc64d574f8368fd0e8011da02465c0ab878bcf0d85759d6ce6dc6f8c0dc20d17b58072c00ecfdc02125fff0b2cd9156a32935196dd2489a83d4a01a76c52503a33a7c11415cb9800af75c4572d243b060a151d5a42bd9bba88b37f1d337f1ccfc5b38cffc201f17487e2ad39750f1aad62285e2c61e0f1303adcefcfd16e15f68f4c4cca4113958755a3a813948e35bc561e7da554bf91f91a6c2fbd337398936a1488424452776e5490cb4776ebf94ac75500ac457b2951257aacdae9777f08c2099aca0de9dfe78e310b96";
	cipherText = "633800accb2e245fde316be288c8c278659a35ade958b340e3ef18f24b9c46c2648a4d3751903e2665aa3337f58a1327194276521b7fa563b51f641984bae6591c4acb7b6fac49eee71a61aba502318676a2d9ae9246f4d4c98508b1f512f3012a48bb723b35a678c233ef5cadc3c42e0164dadf4a8721b81e89515927c0f99dd2f5b98d62f0be7e99cd8f6348bbc218a34c78f87071fa2f14fe0f540f47a581074e5f7f88da2015243acb20aa90390537afb31c6233a27fefb0e0e6bc61b8301ca45df498b412ee2d52574dc2b404602deb9369451211ea12fc5fc4014186bcc5ac822ce172d142c66b4e7c5f5629909f3c8d5aa921cc06aa32039654d6fe85c7cefe660cca9babffcd1550cc17ddff3d66682292a665b72bbf4822a61ab4972232d4bda36be04fa854e6c33256cac5d855247d477f8e877d4e34a447032012380884cd5619e8b2522b7d6a1dbef84706b4ca5604d3295a28284f8679a9f09d5dc6c2609a08e6fe23cc9995423588686391538824611e18ee45ff1391669ff02bd2756419021a503e450e08bd6818fd22164115990e4c7784845409ee5e9f973e7e6ae978b3396a1210a7774f2e44ac1d29dc8e2d9553054a4d5d618409bbf55da16e1307f932966153d6a5e50c40a01de94ba7121fc1f87d3dfda74cbab48123ca953870448d76597bacd19f25f48a2ba0fa91efbccc167917491339c7aabd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 474;
	dataLen = 4096;
	combinedKey = "49d09116beef570962f37bb8d70b543193217234e205629be479e134549dc41f";
	iv = "69ec6223e0940f38ba6bac29bed4d7f4";
	plainText = "fcf33228059eef843242b68965e300c029532315097331744611bf21551ee91b20e1ba4f979ac5b43ac74ea1105cae1557eb8cc789ec42cf485ec793a87c82fb8612752ef34962b237299c9204a9c6857fa2667aab2fbaccd8c117490931691e258f39b23cb1739c7adb83f4930d7db5005c850009aa4b39626a8786631c79b49e794da58d4ebf45c8fc2e50954542fb34f611aa717d6b923807948af0277651488ee00e210f76e0c6713c24d5336d30066b5d50ca094f298a35f4ff81ca1e7ac802f62a376543ebcc59de3e13b7495ea12ee9d18d132b55601cdf591d33ff32fbbf7f8a41a07b8db7c5e135b833fe77c8640e817803822c5ac273ca32d4faa5fbadb8682701e9417f66b9bcdf0807e10212954683606b6f4b849d440c4d9f97f60c1b1b2bceae860e301aea0bb9293a33c6b1077fd00012e07a65f3850bda1d330481d2f8480c5615571949a04edf8f898543f22ac070d96552de42013d1c92b1c458909dbc5bf1f1568fc7b0b1a9894d325f7da0da0797f9434a70c31b21b644bcdcf4fdee5a54710e96559b947b627522c7a99189a76ea766eb2f2f09fab00f61d3fb47d3416575b1f0a2cf3ad5aa8e5cfe74de518a2f56d862310df07ffc90dbcac967f60d51a291fc5c879435a3e7e0f22ec0653abf6585a969d28308b20012f96dd21c09fafc5b43637dc98101554235f06a9a42d2329c329fb05adfb3";
	cipherText = "4768ab7aa849283b80f991ca5f2281936cdeafe0c227ccf69d2a2c02a7f4837d2a56c67f656eed21f47c48c5a7dea793a044cb7baa374c937519ff858f40d9759392bafd0da91e1a23c088eedd72e1f8bb6eb488f17b0a0551ab8fbbf9e29c68f75d37e097f2f8c9b70796c6a517bb665995723a376aa75f7126b449d3adeb18ee105176fce738cde70b034f99e3d6a2ea304a486ebaaff0cf69784ac83c2b46732321758f77e40501e4ebe40ca9d4d0bd4dd22bc8f9f407e50d3a695f451d5c6e43d966963482bd33cba307f8c8cfb2a6bf495c1d1bafaafa39fbfafa1ae824d6fabfc889fa163b7e98b13374eea945bbf4875390e097d2575bd105e6c2dcf723a6aeea78c5cc9ae8d2658cdcb45d202afd9b8fb23f27633fb305ae0f196397fc242a533e3290d189ac5641d233caa1f7701a600c709355fcf486647b24ca8f16ad48f93063e160d9bf22e8c9a2da9e641ab9aee4ff3088b055fa6214efec41d82b2ac448f94d8482380d67cda4a3af561b555db0a1353795d8aeccbec85cc0f850bcd0de5be23a429fbcf0520a6f0fc1577b82b040ebaa0375bd391712d1232e80e6e2e679573a2bb2522a5fe781a36dd40717e66eb8fd0ef79ffd550993c3e5aa39abff22e63a46d05184b19753d72e9f06fdbd95b806c66b8456d00bff564fdc9182ea2e007d1b9265ca5aa22dc6d2280fb7d892b229f0d72d51835f5f6b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 475;
	dataLen = 4096;
	combinedKey = "483a5d4d4980240c84f0c4f4857d93020f88f78d5cf6ce130301544d84356c73";
	iv = "f1bc52802793b8bb1f28bb0b99325ce8";
	plainText = "05f26e4a927ac3f25d6d614d2c89c2124e21a327ce0a5b2aa484b18140bbf57bad57608757255a39e7fc927f44771c0b666d069be63fd67343065e018d158a15441a36958918df84d2e2724637675468aa791dd505eef5977c3822caab6374ff511bb6353700f52079fad2353487526bb7d4bb12c3f3191bbaebcc1ad89711c69f2c94e5ad0bc77c653699f7c7cb239574551af8fc1dfdd4a3501df499c606fc1ffa941ec6b671547a49e639d4074f34848c96609029abefd21c9dafbc62c11ca3b71919a1bd585e88c4a4ac9daffd17191c9ee1c473c249555d80a826f7426961f22fb8d36a4a2911c0dd856397fb8a646e71e410a53bfe3eebc68a869de1e9f6b079cfdefa0e91be91ea2059a5fc6610687658dbdee7aa5e5057bec4fce5bee7ad3aaa477001d18441b67f7d814ec0fc2e5fd6c531d517f4994b98cdbdbcb15b05e088984cae6f2728e82b0d39d5a69a42d66859d93be30c047de539912172877a62f01657252a29b221096a3d4b6e8898ce0f291685f4e84e86a95af3cecf051f1c60223d19fb6d69878386094cd5b423b6c767bab2b8775d848e23380f0722918b63bb8275c47f87a2b1832fafe4ead06b6c588121701c189a33d842305c02a2e04fababbe6431d82dc086a6c0a4a606d9806b95fae284f610c0808b02d513e14c543a6ca714de037b91e384855e9892fb903569a69d3e6a783047e98009";
	cipherText = "31451ffae917d73566f563705a764ae89281523fdb929113b8cb9ee4e1bd505eaabfe54e64ca17bf488c2ee565395dfeff25873e8340e29a7817e59d67c424216f37ad95c616c5254b47bf9c9ae258bc0134f7c767ea6f0c514b8de1d9e671a19da5e2decdc6a624f968f251b025a717f8c7732a130a3e1feb32b13fe99eb25f232d3b884e31e39052eaf67598ee1063b76d2df0208cefc21d06bcf1e0647fd79f8fcb4f2e3f8c7df2ea8da3217bb4ad8703ec647d2b2c3c1b80ba97438c920a4550880311ec945c4776636bd704c5357d5d7263d560d629ab1cb1990690ee2bcc399034c14b63eab15936ab5483a01f227dec8e1b34431102af11b6fed9b383928ddfab76acf34e7422d1dc7636653a294e786f7c48044f68605a71df287daea19a3d3da2e47ed77d4c268b15176edc3b486aa05858da17327633f6a1aadc1f6d5a9e4b073c19f83dd480df4e110b7ee504b8f0446e8fc032add28fba5167d9f085c8cd84197e542a765ff5418aab6583e4decb4b6dae3c74b454a1c8e518a3a566b7eea935591a2d09aae6f38db8644931f0e3f4c02952cb85c2836a8fac9e02bf692a225053de0341cf7f7f60ac0142be3ff1db5191ce4925123a70dc24221744be8862e53dac2b9bfdc3f68b1e17bb8a3d2110b9132465e319f56e62ced4b424e48b9c11ec17a1dfcd1f878d625232097988cca4e4eae980c55520e32a13";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 476;
	dataLen = 4096;
	combinedKey = "4b8b4a69c3c6aa3456811ac94d9974ba8c6ad4db272ece3e282169ada3a4f3b4";
	iv = "063e825a3c52111b162dea71003b2652";
	plainText = "72dc856d994daf32cd21069c88a359c3f5922854ce6fe37e65bf51afdfebdf8118d90dd572876881554f146a7348405e36a34b7e8d2ac7a3d1dd7b33bb99b41da661ae04a26ca0679dfb41995d7e87618ffe6f21dab970c108128cccd52519c18e6807d544c97367bcef8a4d35b24309843923f5b083d06f6b3cc4136d33e5ef9509979701eeec3d1b9b2d6bbdfee8f8c4f696dbc32d2c0c4d84617b7670cdd4d086fa85171b402f7bc8dd632ba7178f5a17c93337224528cbfe9a6fcd532324d6921e51dba9554610a0df032b8875c40c9cac98d658b8c72014fa20237558a6157f9ba4d05db6133b12928e37564ab9d2169fe1341d0a27774fe15c5ee15b69c9e3061d6c9dc23505bdef5579a6ad84af1e491102cd262f1cb345ece6d76f1fa6f25fddba93bbd33d38c4aee9fb96be3dae284a22870da273588785dc6327bf0025db92d3b8c6e2c0cfd04ff27ec7b949ec69321ef60fd22ab52b774e99f2670dcf6c564fd82ea94434acf572d6dd9de928bda5d6ce103af5cce21e12e97eadddaaa406fa93ba2598d3f2dca0b7b6042ef65047bc865fa4fadd32dbe936b36647128d2e140bd956454473d75956f74a30aee588291cd1729f3118b6f92e0e940ccb8ddaf44b7523b5fb023aa5ff3360669a518e8b2f380b87414db54dfee1506ed0f052415d22a3eebbfe442f61fc23f385a5f1cf2a0185814746b60aefcfca";
	cipherText = "fa8141cd3cf61538f79a306b876dfcd466e19aa1cfa60a6bb103fc2cf7c1d0483bff89f83534fc1936b74797ccfe5a0a5e659347bc0269973536b592ef14e0d842c86240d3493605f6c9b00226390248dec4e0308cc9a7264f9ef9e9d3ff3bbff1575abdecb13c2c6c0f77d93a0b057d512cb1860bd65aafeb77d5f9b015dc8fb381f1e8d3bce7645a6d22f719b25cc76bf4bebb233aa28aa0d74c45ab3f9ba66857ac09edf575120f5fe89ccfc366a3a736d5db3ea45f29d8e6102217ef405c6dba1301f02bb98a6ff8590ba66b5e338eeed75d083dffd3e74b8d926ca36b002d449d4458a9fa6bd8e5e3f12afcec43978908e36f4ebeefde1f2171691ef87d3e7dff761ee35dabfeed7743e71d5543d52efdec2364d86638cd778441c0d1138dde1e36fff75b7faf68d74cc28b74bb8c9dc2d710a6dcf2941078fdfc132f03536eca64e6d29af80681f5db0576358b70da46adeba137e02265f5bf9d84abdc9e5a4ff8f96b1d5ae39aec0006393d0b2334d264b49f8e4513af6fe570a6fbe59d78b7af6f811cb1aaedc0a6469fde0309caf58d639be41da66d0a888ca325494a7fa8e1703626c84d1e7e2aa764723115062efde3161235b72c9c961b6244b396aa52e361fcd9bf523bad1515741599f12d884ddf3804ceefe7eb88307725c23eb364532140413ec54f983cd7957b1e1db5b1a4cc5fd90061e10352ed35d980";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 477;
	dataLen = 4096;
	combinedKey = "a0debd395c9c20ef592a692052df0facb5cfc12f12ac5eacedf10b69db481560";
	iv = "f48bc54535d0cbca0b8fd3025ebd5642";
	plainText = "02a4e83f4e40dfdf6852619ab2e0c041424227b36a8927802521043df6c1eab345a4f22bed8f3837df3eac4c5b9b1bd7047353ff2b445a5153931c3aaa843f9a1507860b89af406db47580ebd6f8399cbc43be27f202e0f8f3f16a771533be99a10acd97fb30a27c0baac084cb68285eae8988cac3c464f89a0c288827632e41d647261a582d25950c1b8930e251075a37323732144c7a428bf4e34ed60383dfd1446b9d4b5ef5035cf0522cf691bc7406e80ddbf858eb7efc29cac75b9072d92c5dd9892c689908ea3e2507838f8de40c2de63e4b1caadc6444afa4453910eb99c3c5815a3bcd12423b1f04e1dc34f9b3b10ace175f3fdc2ba9c48c1bc1b61d528c76b0be1b16a766f73e2d8478581b76158e1e4a41809583fdd2bf42833c8803be58c5c9795f4137f0138dfaf865dead36d7e2a2ac3b8b1acbbafff240a385cbc6d1097190bc7f022bb34a536a059591a295fb53650b36cfd6c305d0c0a8318c5bb242f7ef35a8ab125564a9bdc0712e0d5e92867e075d615c3d7e10403cbdb3fa4bf0177d8f17b424ca1c3b4b9a52ab760ec104451861de22d905131b200e01c6bfef6edbbb650fe64d9abb6f25a2d6fe62d004fe5208930ed78e3bf06d4303ac8d190934750fb08da24c6053b3d62174f4c012f75a76324f60049a3975d1420426b93c49b36996999a0764024cebee5a4beca8a9828a9560a663b7cd2127";
	cipherText = "301c99cd1585b114c466f758feccb74419b39c9751500bf45c6c70cc158f1a87ca9f10300092cf227d24522107f34a2602bb3008e669963a4885738c327b9cd052404f992e607a50d2105c2aaea653a997cbe6520161ae701f9bd5f5c3253346960587293e846c98d2c4b35ada723229a080c85f433ebd68b5525d852fc55f78f54d3532bf3b8ac892aca5407d6857cb34a68114af3ab5c0231dbec99439a539aa3c2d68291b06eb130993d125e4b7990d7eb4e2a1dd9da28131e6f2ddece46338ad5a8fc68a5a51d04d9faf22a3bd9ac0ad3e8314fe5af36700efd690f2499c3bf04d1351bffdea08594ffd1352a254aa80320adb35d2590b73dfa8a3c112d3433ffc0218241462c91d956d37af0f5c45ffb43a761b4120650d04da48bbcbd5e34ffb55a571f409071294eb48b851b913aeaa3cd00dfe71e10f7c896157fded5b0ce65e82b429466d2b70e7889b42f87bc0457806e085dd66247358033737c30bc0de2be504a5989f0cb01812eec5b6d079e06cc81303128c43b9803c95856ccd915350dc9100a24be97a7aa1aba438dbcaabafbc0bf0f33eec8d4ddbf2421475391c64ad34628dc3539908c3bebb76a3c7fcbd8adec25c093f345cfe931d5d84330864913d32a40e69cba54654df0633031611157efbecaf4209f807e771180ece423867f91ca0c78a7076f835370697a0b8e47d0db222f4c8563b0ad5714e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 478;
	dataLen = 4096;
	combinedKey = "4199f855a3c75036caec90932f281aba5ad130218a7177ddcab5570f516c95ba";
	iv = "cfe26cdd38aa43c30a8ad21eba4ab78f";
	plainText = "bac1de9c944687b328e8e8de18c1501e28a201518a9b4e675854027c14448c4cc3416e34dbc2f58b9cf5f68b1490f04a5b1a692bfad891600c4830cccdf5b4cd44eb8bc27a1304442f684a40c54981ec6d3d0840b5170777e0071f549f0cb8368f89e32945eca21cc69046c62bea2b64853d420b591b35f8dbb6d1e134adc70556624ea369b0ad6a31716517bc46dafd967b9042caeeb1e7a67127f71eed651d7a77d0a842dd0743ec20a23348520c815319d7c697cfa81507bf1da649274c179afd478a4c95139cad2c59ee1f3988c00b74f5ba3652007fbcd6b6440e85c52fcd14786f5d489ab6ce0d51a58a20a9648b31155b0dc071f163bcf31769a562d8cfee157e1774bcf0dd09f0682a1b7a8be06b5f816202dfb78a33567e16d345fc9dfb157f2ff7946c0a1e98a529af3eab1babadb1dfad43bd86191459b73f3169e19f474f37f6d0678ef39b919afd5143c5d6f033a27e47996824400f78b715511b1557b13f9e362435e3e3b9903d6b8ac0d298130ad01f0da28c29ef967f8cde3e646d2ad01e8d9e3027d61ac6aa2aecabaf9b6ea0281e2c5a71fe56e2377bbd78b30e8791c314b12759b2343e78910918a54d049b2eb8916e4ac581811238cc50dd5dbe3fcddd4265490db064c4ccb8392f002af87be25c8c9e2a19c4b323da5d89487aa709163be02e98e47f4f6643375c96cfd34fc2f3295cdd32b6f8eaaf";
	cipherText = "7fb8f70b69b5c8bfe099dc8da4cc9dead439834a502fa73e00f51acc13b8d1953e765a84ad9efa3ed99aee6386e28fbfa63edcb6eec36201537f068e6aedb27c6de829d4d6bc4258b791aec814da5fbf13367440260edd1b1ad40f7bf63236f6272004b1d2c97d790d30215d2c3a98939602ea8adb8905e554828a997a0ebfec1106419b3d06e488822993c4eeabd59f32ca68b6b5d8c8e1b286061fb66cd2d38d3bb232f232311eff3899be07e3a5cb70fb534cbb42a73bf791b2641bcaf24e0c9834f3df4a10b4f96dc901d3c420b5fbb86ad95a162f6dd9ac77f640d022eba74ca91855e00c02ec491ce5d15710a41df1b5b574ebff17e97cb8e335c29449e54b8636d2c1d1028a9cb8897c360bb9b614481078d8a8b1f0192eba97bc8382e821b0cdbfffc815c5936c72840c5295d250e94f982877a41b0bf42ce6b4360214726387025999d9adcdaeebcf12a21f1e1fffd08b24ae8faa3d34890899fd7ffd1ad8d7d02adabe64c519a740af7835e722f93a4742ef3aff54aac4f73dba88e0de13be868bf1db6d3d9f1db94adcc288f5a3c157d2cf303429b008a24205dfad916f9321dc36458f1c00634eb1c1cd592ff3595ccfc2c1fc849fbf8543a6c4508bde620e9451583be9bf627afef716aa70bc700c163bd55054cc3828350b41e3cee25ba33b61064309926b319dee0aff561fb71d0922e073daf215d85be560";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 479;
	dataLen = 4096;
	combinedKey = "506f96f55d9e00a469dad4cd840724331146437f166b1ac707bf3a485a310c6e";
	iv = "c2adad773807fe8d46628f401ff23d57";
	plainText = "70a589854f002bd679250209b17e36059e95d615a849bf5eabafb56335d2130186308a8a44f688765d8e76ec0a4ff93195959a5c091d3f316340cbe560efbf9ec058b3700bf2827fc83534ef8bebbdc03a176bccbfea0e24eec9963afcabae661989c38d6123810bf2f44070a1339d853c02874e3445617de5831471aef4029aaa73a58768ebe06926327603851cef4078103f3f57871a238a4cad880c9d482c7186b6ab2c37139ce131f4d91be1ebcbf34b7ac3114cdd9573b75fd6ea5bd121dbeeeab99256d746f939b31213f0ac1bb6a28a86b80810d06e651e526a9683ccb65e646a97b47a570adee1454dffca12c131eabfa023dcd861cf2ad62e4285b87abfec4526b1445d72e1f16f344fb589442b9b93139928b721668f7fc49816dbdc557b11467553276b59c2fb3f5702da6f8ac6d1e5036d7aa1b0d49e1f113da8b67af2400c0bd29a7a58e376cbf20ae9f9173af75d543607f3c50dfeef8e063190ea0616067a8fd89b5526582ddef23bb2b3bab5b364aca85cb5e929c5340eeff15182ba24bb8c7dda96413a936a573027e8899fca5d2f0ce781c35ec8913292ef774ca6d298a7b541d2a3ac4307c7a349d9aab7eb21fde6204ddff9567365462995c4e44e121bc6f265d8bf0e2165f6295085b4cdb1e476e541e34cd8bf01f83cf283eb2e6fa0132647c8fdd283fc19bca6e48484212615d71269eeacba5e60";
	cipherText = "82f15216d25c9789d5483f9d0aabe216b86e1421d3b3bf3e9e4236db96c46e3ed9c219c034b90e46e7a09626c273e90886789ce68f90a648144422a460188b2e5e03369707c01a5bad393eb4691bc07fe4b62b5af611c74c969d622f0054d2420b5a437b8d74d4efffb2d2b2aba3d21b6acf8c05e7ddb34bff594145445ca1c557de9aebd2c88d348c8b8bcd08ec60f3f16baa9ff73c8eb415947876c6e1749e0d0b81cbe9ccbfc7430732e2bc9ce2b314debbf3babe7205813d5ce3d132cabc2b3ccf61c05cfa45e264f9ff882d0e4ccefda22b4ae13ffab37ef2535c87d16d1e10ed6d73ade686cd65b3a0237439b0af2a05f618fe53e74e2d68ea3bb0f4d73a6e1a51d19375bbc6fb952138644964bf9eec5946c45fd9ff050006462e28bcbd7b7ce9234ac6657d876f8c91c58d6f30ee3e33b567488157919ff61b46f1a5ce51ade2491319f0f1e7c0e6c4d43f0a819942bb7fb97838bd9234efe49cd2e49b0de8382cc1d475974a01ba430532da6f64f22a26072c013c42be8cc74d425d68a113fa4898e2bdb5cb47486e18f8dd3c04aa48511bd03dd41610a22757443c63d2f8c3ed56195efa6bfaf9fdbb72eff8de02d1a01e46716603281b23d4f3a9441aa62c0aaf0ce6354d50033b858e14fa5e8d9033c1c30450bc41d7da8f45a3dc39b4b5a5c4960be477bb8e29723b7fb54123db7699748f47843814ccbd4120";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 480;
	dataLen = 4096;
	combinedKey = "e89584bfe6323e1153eeddf1835b36a4a6f8a918e9bb6d783ae2a2f2fef40140";
	iv = "c95bdcdc106a991d0396f1b9bc4b2794";
	plainText = "5e892302064ac95dcc569a71e5e11ceb6c49795569c77a02ee105e31a87a5738a2f03380de128490ad8f7eb19aa69b87d953ddaac613d2c73df3fda89111145459daa39b47a66269878d4e6f3d8b80f380ea3949688b1371962ed7af8a391b6bad01f1a92852143ba17d202ff64f224baa64946d85df26ad18f57ec17dcb54d9b2c8229fedf1ccbeb97a4d38da7e0d925353b7881bf07e635b43f52d8dfd9822d0a7644f931c800b2c87682285ab44c91f8741daf210618bce62226d690b0b36397c4c00a64ca5f0de3c8341d5fcd931e49474bd072dd2d27e1ebd8c8248c813f4478c415adab796c8f5f3d5e175438690135ef8765cd5886836bd9b7c00787f4366bb8cef3481ca4f468fa19d3ff734c6d29ee69aa0e68aaba8c4808f640cac891fcc4c2f277bf3d2b75c55c71ab428024b1aa28cd519c9d90a66ec8c1b121190875ca7e657a5d7bbfd7c8c5be64eebbf17ac07bc1a3cd23ec47484d056cd25b39fdd3a54769c33375263b75611c37a620076c9c68df85eceacb698835e33675611e810c2c37cdde82ce9c3073d08156071f36d0e5496f34d5d4833f318e4bd334c193be80c4f2d2b206db7e2b125ad6f8f9592b8ce2f4fcc6be17389483a30fff528444635ce813a60b153d491510106cfeb0302ba31b12bfea57611cdb801c4582423e9f0fbc6a3f3dc73d15ff93ab541cef132a25dbbb64f99816faa0aff";
	cipherText = "8fc49e5957c421b9b75e5032aab52bc3f6127ed3f4387958a0970819e5dcbcc9a7e17cb9e09e7be6f532d27993fdc87c83468c7072fbcf5e268737e7207a37106d5661386c3e3e9e980718cada015a43ad617534f128012ceba6d0fe011b67f9dc0a51fdead23d87695087ea4420fd5819be4b8e10fcef9c252736a1e09307be80a442d9b0ab8bb2c91603af9103136bde88c7913792e38dcdf642d672dbd9be0276fffc3604272d68e2380b58cc06f7e0c89e5c7e284ea4df3e677d302fc59c48a003b4487b0a64ae2e34b2716c2d13888d824421e85e6b4f7a9a2fd33ca6bbb0f308635fa6a76b960e2dbfa935427c2e10b28520359771f5debd3487cf2ba6bb06cdd45c118c34d1323164a0bdacad20fa96627a56c895969a6135cc62793c1d5b5a0fae5d18892858dabfec7656124ed543d7efd0fbe6809ba1a89849111756e93d0c7993b9682bf5e1bd9ed5a40b95db2c4ec8dcbd933c18480f47cfdd853a0c9e8ce6e638f5650edd3ff8a7c52fbd57c514fb7cea0e160e606ef949a1ec7ac0ee6835039514b6842923b9597c8ca4f5c399729465511b51fb53debe8c38164db03378d4e55ced5be923e2bd5e99f889007e9e016e9484f2efd3d7895bb2187b9be1e036c4ffaafc83161db784cececab8b7aaabcef8a193c711be3aff052b837e71bd2f0e2137d33b7abd362c07fa822d6a0760cca362b4d0033154ab63";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 481;
	dataLen = 4096;
	combinedKey = "3632ea697a1731e0ec4e851463d4caa2f7a3dbac8a4d3b4e1fc7b620bb2fbc0f";
	iv = "997604e9ae26991c80a300e406df89b6";
	plainText = "c8764282273f94e731ea86a58935a308ffd49b7d7d9cf3fab204c806e3c8e709514d019f9165498bbe8f96ccc8507e30416fd0742fb78e971c7dbeec386946ba6aaa74c0f83a1ff951dbf6f577cfc066ffa2956b6a93fe0cb24b65fbd5172beca733d819868c53fe55aaeee6bd1a1c7f824010ed8debc29d48d91259ddb425fff69f8e7b60c7cd39fd72abe413d0d8672ef3b783c983793425022c94ea72a815dadf5a77ffd69aee816d28e3e0391a7ced0b802283bec0750a6090372d9d298b8c34cb95ca2024bdf71bbb3a8b649f8e4d282484feed6bc82309731cb2bfab6760752f39f7683b3f41b439346559a635ae51e54768f9355925517d46b2118e0682fae054175b3c0efc0e9901811a64b005bab35086c585e8a7c02e766e4e3572e8da373946e9aec27cc478a1625e0302a9d141683f75574b1d7034e2fd4b622f76c36ab4e227abf71a16790e3311e90b8d13f68b1d11fbd84364dcba2687e65ecdd16e0a8258673b60838fb1d9d3d40c366e698743513b7db04fa60ac35fbe4d8c85057900a6b67fb57e159fe0e3f02c900ce8b949e0ca6d528eeed73ebdaaf091aacb8c0fbde8c4e2f06fb6929cfec3b896af652d43d90e188b9a1335cfe329d1e10ebc2b6da97363c6a8d5aed658cfb2553b6922a7b25861bd03598712f3be21e6505c42e4a4b60021ca159370c4e1fdb71f538222d9f20ce214d5f4f746c7";
	cipherText = "4d35a5e72ab8a9782683494392052976e7b0df0366ffe828b5f6d2c2217fb813ec2306ace89a445484a741fd413de208f2184ea86f6f81a0350bcfc397a31d37bbcf875cf6e26350d05378123e549ea5f4fb9b33fa94151e79a675c198b89a60bb8e771511628a267b9ababfed2ea4969fb9dedaee868a47dae67c284694514ebcc68e8602d516fbb50a391d7e2ec7d34c50f68e0e9e197b5a76bbf35bffe4fe0a1d21a8d8d33e6e5aa00c7d40c4dedc72d9fa663c99e09bd8cf6ae7e710959d0350052d0b7322f710b8bc13ddd0409b2ae3cd86eb74b20827d2227a5d456c68c20a37f54e73e7177ddb3ccd48bb539b4928d0f692ecb65b66462a66d2a2505aad9d937c4428289a273be6802fd4d43b4b0003ccd1e21f9ffcb15c0abd927152e30e4a2cc4eb225bd60e49adf47abb07125d4654f036ac3f9d4b51c9f467d754ed40a2ee5fef6d401689c9b7001420c6c9b474e81614fd2570f58ee41e2474a9dcccebb90ed1ad58b92d1f591cf1ac583d43eb86325808b4a53716052ec8910e5adfb1fcb606a8f5447fc2083223699ddcfb94300fcb20448f15f469a3681d7f397090885667df4d70eb159d8a9e828b9c09c35fdacce009aa0aa851656512dfa04e5bb41730d0d91f81cbe2fe1252afcd78d0a0f3211e25ae6744f0c8f996322edafec4616d5cdbfc0889acca330adbc5da1ba602b3bddb936db6337354e049";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 482;
	dataLen = 4096;
	combinedKey = "1f6352f625aefea8129b32e7de76f9e526385e168a1180e843f910f23fd5fa20";
	iv = "af405133710c37c19032cd9ff2518843";
	plainText = "a853ac1bd296599ac76280530349bf32d38eab36a0d91d0e3ae092ec8d68481c5da27fed06498727df82ec8908efe62d434022493804eee4066474ca8a47336b2e8cd5c168a41a5503a0725ad7c8be6f8eb1f7af84f0a5b44513ddbd7384116b6675a0b0c1eaea7e3c2f515646e0548af3fb5809080998683cca21276e0f23f1f3bf9d66c4c43783077551a3f58f82ef0ee16348542c64f319e641b63f39f09e8232af6c77c54e727b7c1270e2f62cbb3d68e6e052aa9dc62bc1cb8b2fc4fd601a79438eccf218dae0e2382326a10c5ca0434f0d89990da35acc3dae5c6cb2be951b66d6bd584d2e06acdf1d0d04b9e442c9870568b1a0d5b6db4cdbc8056285eb09af48f389e83be9c3f18932f18481fe3f940f06d1ed53b835d3b99c7017976d6f9bc1b6342603c8174db1a96216ca9a224076d8e50b1298e280397a780bf99a44fef6679264c38930ceb239abb2c572ffb329e4321f0d2ec0fb9f42950524cbc701358538ca497d6d6c4229e8b0c62d85e02ee7b6355b7df7d8a691ac3d5df7b751b6b27a159c68db09a85e49a953317963d03e828651b048691eb1b3c7220195f1c0f06c22885ea2ea36da6a2d4fd92a68fc2f561845c63095530960cc5611d126f7a6fd162f1d7849624ad2679fe25f70f3bd817b8567cc2b8abc86da79052257f7c4ca3865d05322ea1e27960a29439a03e2aaf978f88a4513d9208c8a";
	cipherText = "1e232514d7e3b88de7f8068987a62c52b10bef587b83a6d0083dcca99d00adcc38956a077688b69429dfda72fb7e478496e98f552fa7ed59025338af558c089dec4f8e2f34d707655144a37e7d739106311220bea5949907ad3d77c133f7ca5b20aa3fa9e35271da0408338c2c88dfc22994d546f47ad2e7aea35cf64735b11a7c4cb70c6b10eb76485ec05844a8f2ca7e856141d056125c73783dec43f91fe7c132ad31796062682138b6f0ccb2bd09de05bc8fd1cad8d2f3d0f3ae5d0687475e92148e6ae226281ed44a75f1b9087a0aba65e987d5a5f06f74b566661fddf478ad5a94454bcddf8f94df5bd294018df428f448afc7408c5c977c42d8109ce11389885a1b30e75f219481bc752b2af1a4d9fba872aa8a3231a01ed764ff8734abe61f02bc87670dceabcb8d5ba5e2e6aee08d2f0cafef0b2d7702e648c2ba6579e900f87357db699059aa45bb32136f46402d45553a0833b49a58b209d0c605a006a33b5c400a4ed51858b47460ed2acc869b88b60fc3ecaeac526e0fe59febd918ebb32348fea656f42317a3d3e1543d3acfd36ca2d75b239475b22a7b910267b439d0ef73e13f284926b1f04410c195d2906180a7ce308057356688ec22c9812eae3c137f8bb7092e823eec2b40e31ee46ccdc827ef9a2f45b6d9a8c124743d9da974838ad1e76ed3efddfb9509b7d76f75a9ccc67c3cdc41746875dfffd8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 483;
	dataLen = 4096;
	combinedKey = "6641b2599462a4355e303b402f1f4a901dd6f10c21f58200f76cc037085e429e";
	iv = "1b921638360aa4b3860d4c2df03875e1";
	plainText = "d943b6213058d8ed1a3efe1083041f552913b7f7403a27b576ced9d97d34a45006dc86f4278d0f40f79de133118f3fef6233d32ffc991c24c67f736721fa1626aaa52680dc6f52a7afa62473ef2c0c5e735dd46c7aa3472e7dc23965485cc3177783f110be749c5f338d728d52ea05de07c635f260e948f3a01ca47a8f2b847a5e306591a68286ddcb353aaf48615061db321e702fc9b03b31a3b3711d9c5b40cd23503274a390a135e5cd1fe47ab034da51e03cbb66fd9ef6f756dbe66b9f9c240c8949a78fcb9bf702fa4c98be158338f32e5dc8921b373d4e34e39df98c827693cba67a49918f0f80f057b53509bd5305877daa9bf5089977a1f405648aa1946317360b01820cf99116f56b8e6b2dc5ce860712c7904c7015a99a0baa8510aebd4230cde0cac239392827ce872bc595f9c18fc870c87e5d1fd136e1fd1cb292442b041986c0bbea1423c1c2307f48ae799553a27c5763ce3cd3203a93f1397072885edb37fcb552931d97ac28a5d0d78ae3db7d6c35d78c042b42ca9d43894b714d87be0fe26aec7c55e902214ea3f07a1a773c255406532a9b49df9a56ec503f01986be5fd2c20731b5fe470651ec369534dfa57133e1b77707e1f4c639d3bc03bbfbd812995fbadcdb94aca0537dd4f94de8141168ceda92e42f0f39a817c8e42e5cfbd35cb2678cbaffac60c61fcc731cecbd3358cd08f0b79b05f45ac";
	cipherText = "6e18f6b095f6ca869149174f94e80deb7fe5d23a067accd3543837d56897094e2362f18861a851f0bd756bd80c1517c055ec2cd50ba438461d394d6aa2f8d5aaebeee14c78c5c8cac4b06718a9cd3d67ce707d6fb5326efe2146c15728f0c2f1aec8439976c70568300e64a365bc96ba2464f94877628f75ca074614ed69e47519161b04b7dacc8d86bd87034a37df3a56ff71a7bbe1e951fc94cade9a3f6b860c8fb39a7571bf1baab76724639d95e5c5f23016b9b69bc313cef69938b06e96a0e235f46b319e9d31d54168fc35170e7fa4b8ff2f996d4b91c58f1dadad82d2d5619baf13bc94a414b7369a704e4f1ba1c6e51f6938b945e35fccf5c505a676eb27e3cac796c80e4c4d8bbabd4e43c968552410e6686dc0548ce3debac4fe6e607afb27a6aaf70eb74dde71df692640d9d2fe974f252f1df290e2916abde08c48613a6a09fea7d2860f9433c6586b27df385ccfc342dabded3fe9b002d28df56bf9df68ab1cca91c5650fc5dcc19174e71202d8a082a8306e3d66f09fefdd1e698036e38dd69ab42e8398775216cc4e6f246730dd649318359eed6e53639c7c2a1bc6fe0f322e17e31d2ad9802d17714686a233b6a76782f4fbf4b5dba8afa9da937322c40b2ba2278334760ea4ec916a298b9cd6caf76af8c012dd93a0b9ebe8c7ba5098cc75d3c2077ef58861faba3ce24cfbccc6157a4c1d74ca5c07ca7c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 484;
	dataLen = 4096;
	combinedKey = "a1ac2aff4e4f6b1657611d227f713005d479937e46c9f0ff1ad71a3998b87753";
	iv = "8696f2b73b26011903ee7f5da968a953";
	plainText = "954c841c25daa17297120514b9781dcbb1d81fa1a76fe1ecf303fbd632ddaa8d4c12ce5a0a4287b984989e8bb5fed301cf91d8be38fb01832dc2b678e6d87d19e772add88bde12539e49367d1cef4b0b4ee37b26c2273cbdeda51f5ce89b6cafb409e9afd8017baba92b38786179f3b87356fdf68b229470064ec34497462801fde75bfcee93b668e130e8332c16e554c6ec91c28b1449e139c072356d3b4490b088f2a4e95a270c262739caa4a5cbafbd23d3fa9f6efdf2f6d77890b905be19e124af7a126baac2741e351f6f459bd5d4307a5035e67437859c1722d8d921e4719a00d0e0ccd8842fe5335544ac01565d43c62db3d8f32b55962c2308830c82cc357639ed7bb5fb4b3edde102d0c97afea624a8ea333e888e203430c7a396514ab76fd5ef5633bf2e9581265f6b20120a125a760c7a53ceb07f2ed4ebadf14f649538caf5e24328b8a199a1ae040c67146d7df0ec09cd52cac57e2d366a058dfbde8740f6023d3c7a9a083516a14f4cbedd4822b070549a122f0376d374f1d9d79f1ad9e0bee9cda30f79395f4724efcfcaff193f924cda75c91b8a79b435b256c7a4b1f1a24ff58fc59a9a8324541024f495826b8429eb4cfe2abdc7b9c7d7ec542bf7194efb496bcd84f7cd5d04befaaa1d9fb70f82d0dbf0f8ba24c29fc097908a62490afec0f8010e0f4e0b135e69fb48863ec6007fff68fa989c32ddeb";
	cipherText = "7934cfba6abe30c89a769d5d06efd25d0891466c82a93cb1c9e48dca6b2dd401099391875d63000064a9c4d0903a0669ed0e7825f25548a4b8cc8c4a460a54848e2a6851592c2fa2de17a8fb6468296733669a6939dcfffdb2c5eb4716d08a35a5405d5068b4608d82c5e27667895ec667ec91769159e089c05cf75945448451d8fab62517a754f70efda7d00d4612e5c4a926e59684fee67f4e17b5549b2109848dc858f00f98294540f92a7d27946f066e92d1bfb5c9ca6359c65e72ae0273df379c9803c9c3be8114836a39851529a378cb910c88465507d901dc1a559d48730f701d811dcb9180b2c3d026511f67269f6c626c86d400ba160856573f34d0ff30007f4bbeec944149b67840e635fc0dd4b6daaf0f2d054ce57dbcc628dd36accaad52061991338aa02b655fb81a3d9887e6a47b2f7613e70dcc9a6bf6d3f37ceefd4108d33e3647865430fe8defe887ffd5ff412d027da1412ebe0e78afbbdc100289b118af785d218b817cfbfd1f709297acdd45d728989a4c8ab6a02148e361a55bf4599873e3700e1e290d37e0d30fd2155fa70c16535b7d6ee45219b4c910bbfdcf4db621798796b03dc0bf94eba37c5528df5723c19e53a68ad753e73b9da8a9147675a733286027d9a2bfb05aef76480fcfb0d0a9db06504fb55ff30c15246148ddef30b0edf36164f02bee18ef5c4cb645f69fe64835e6787ac05c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 485;
	dataLen = 4096;
	combinedKey = "8a4c75f1dde8dcdd51b0895145c1d8c6c6b3a8d5fc1b0405f4f2edb1fe6bbeb1";
	iv = "ed06ad38a1fd44fc0d1c0a8ef3918eea";
	plainText = "03bea3e0a10b103e067c752c5f0e57812ed6599da511234873b514323f2a8614ac6c1285c293afecda5d6c19d09baf28a78894eb189d53dd152a3c0c6a903ab337e25a870e7980c2b2835394e960e2cb6813b04bedab800c04abf290d9aab8062c6a513a796160f92bf25391988c4bafdfe61187965b0fd1271a5a6d846b733d4386d24569a5e5812a3c84cbc841530bc8694b1adc1baaad61fc71436e7c7c135e0e5abdb113228497a30d309844d4a04acf4f50be2329bdef36d357cce70daa0e431462d1aea53c5c26ad8c87cbb831b33075650ade3a2479238239d075619f25c60a3a65440163c8dd3b9a33d14be084d1b8b445abc6a8a28e02c608351a90e55a076444bec8093ddb383172b1ad9154e217f362e882943783592f29f6ff43a57ed7bbd28295e0004024fb82be580fdba3d551afba932f5a785be0e366f026cc9ad72177a383e66cfb5add529070f123ca07bbc327a4e06985e9ed7b771e5b46704360f7b1b9b1c70df0a6cfb11bbc61f7025df1a22ac65aaf4757cc99329afc3d106583c66bd75d80a698e655e747c7e8df4a31ec26d5f592324b9a3e83a86ade026436e7b24b70a8cc3eff6b40ae5b753027a15d9ad20de3c890440ddddaaf86946a004c88b54a962f5c3ef6e9bf9c26da1dbcf24197e719aca98cd6cb571dc2d9e625f32d774a1cf220380f7acea77724ea47256101fa78d16b1c979555";
	cipherText = "e1b09cad16c0b01bbe81dd75af412760fc34196f4cd70d8a8c80db4ef0cd8eda7d5624c7cffec80f7132950a0eb049345c2c634d62f9715362bc0ab4418e3bad5596978ad83f2ac393d2956e3109c5e4c412b3c06339c0c5b3ca7206461a1a04c79f97650da69caf96ba9f89812a9a97f76763412bf9ac23aa8c0492b1c180a0ac8b748b0811b0e51440c8e5bb407a7dfc5a4cbd1a9f2f62e8f326e1eb0a11786c704cc67418ef19cddc19aa74cf913b9468f74f616d47c49e06078d39b6918df56442162ade8ba1988eae6338359972182511dd3584be83e338ea8e07e7538e6c30cef2049b386601629d4d111669c4eb2fd3d6cd1c1db54c4f007a3a6d698002ad3a74901ef77a0a100ec493831e949c3e59d25f632f6f887a71c5449d889e2aeb87e92155ad50bdb19dc9ef47ea4c9cd122893089680f67240cb83c8c170ea5c3a1c27b2e23a06bfd4c036cf6a9252c5ed2989b566129069d5b1a8f2738e17406407b7a00553a1f59d71115686955c1ac884eda3e7cef25500f7653a4c8965ee6c6afbfed6602582fff346ac6d3e1cd87d70775d37ddf1873a4d4b9a35c584841b5366ed00b2c36a58ef4f6fa7f11361fa3cc8fa9d0efe49072621f49a71437002bf1e1fcf4d3d93b3aa28a45953e244f5a7813377047492789ac1c6e2b84fbcb8450a9fc34db278c8fee33744b96cc13f694705a1d5b40ca45f0f7518788";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 486;
	dataLen = 4096;
	combinedKey = "c350d54d50e848d5fa5209ae545377e9bffee17b4df7c23e55faae751b4c39c9";
	iv = "c939e6df408f112b196ebcc37934b070";
	plainText = "978c1fdfca264fe98e64568f1c3647f0de5517facf78a6c3f5644faac2f216adef84e8dc99251dbc74d45cff1fe43e60f7c3617f35f7e1213cff602cf9b6e207dbb7a93a39c6daf76d5b6dce6edda8539e7ab5fd240d732c72b354d64a6eae91fd4eae90cb0a78c189d00014709631de2760b7aeb053af9f1280d49013991ec4194d4e8485915f12a03e1756183ccbcf09bf6f4174fca005c13b94d8f082744df280be9d731b90e13b48d6462eac99fb308e0ce201fccbdfcbdb5d78a655f3abd1a513e6b0bf2dbce7bbcf64bc3d6d1e7651eb2cc54a8683047f15e541a66c7bd5b71d52c569b9e7cfaad673a0a4fe9e9508189c98bf674bd1bfcc50d2d14f3e8d1c7c108e60e5bb962af68e698ff61025c9a0cbda7d9f499e60bf077ca224f8678c1b6ebcafe59de9aafde342ea683c311ae589fdab01838194c420110420c99b49e4b0921c7e147cdad13c31771b063ba2872735e03ba1d7d6c9604d3c28ed8e70928251fee2ac73b5acba4b2ee4ddf6f7817a4e191aa540e262c5f11ff455b2a2baa8435dfa307e4a8f78085662dd59eae6d1f965c6bb5fd5c8a99644832052d492d921140976bb4c09d48a58f0c08557ea34a1045e81908dbb14ddafd8fd75b0a8cb8f90c3c7aaa7e3124bed3b61ad14599afb1a152ec176ce6e91e3cf61c7eb3f8135200e7ef826cb83f97e6e3bae46190fe60058db41784ca8b034520b";
	cipherText = "eed61a1f841604f5896fd532bcaf176957830e1e3a27b460105c2dfb2a026adb55d1d9a65bc7e82e3ededb1ca5336c0911e7ff1c8e04db729bc52268f3873afcc1498b120e37313a50d194461a4b7fb6df386387ff47ce0a35fb3c42278cfcfa5eb2036878e013a322b54f49829940ed7c54f44c7557c32932f5f7f06420d408ba430e52e273e9edfe0f49c7056809a5fe98bea055d2244be8b2a1bae266ba3348a1eb72c01659a301585bfac6e29c57960e93c9ba1781376b90cf0972b9c08aa5a15194a8b790d5dc6c538c1b499f6335157fd08cdbd868bf7929b2bf486b3464a43a444477d489c6b323370b78e0eefdc703855a08689afadf227ce89097e8823d44cf224206e607ce6169af80e89ca5be5d76b797e0dbd65b5faf41699dcaf6fa04ad188a50b6c8f3e3a6607865d5748d821b32e47b108d12aac0475a71fbea47f3853d1b077818dc9bc877aa015b5c9cd69d18e4212f7fb544400a522c4b1aeda7b2d2d1e79d3796469834282ebbdc235907314406ad6a1dcc897d270ab397fdef40b57fc29abefad3739ad375efee1ae58eec9aa49417f191120f0a980cfab4a942d2d87cf364c36ce69dbae3a61bb178ff36130a6d968ca156e2a86277a08a844fe9c28122687b02334db449be4f0f4e1097080a214c12bc222ad922953f2df0b67fdadea5f7a5f6f78f8dc48db53db0556cac6689bb02135ae3aab0a8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 487;
	dataLen = 4096;
	combinedKey = "b150753da3969c79f2bd681d5652b0e0bf6dbe7c75a96ef80db7474adba41446";
	iv = "9f0b5a19d5e55727ed88a5c532fc4ea0";
	plainText = "7a7b38b7d1d3ef23c0225a9742beaf83bf3758680dfbee2d88021462f1dfc9b71871f9bfce8a6a847e2c5034c5a4184c15336e637a4ab43b4398399e15f7ca321d875af76c72289905a5029047988feb74adf281eaf250374e6107805fbe298ced78f05f9ee2069002a266f7045cbaf55a6e6372cde84418ea67273a3edb32292ebebbe707dde4bb354bacbc365cdf1f8db8b5b5e5760af4535ff893b1a31dc8a6e93c1a22facb7d9c18da25026f3bad1ff6569d1fbdc83694f77dbd78900f573b0ce3a90bf3978e1bd520374a881b4986275110b57248c6dfb6e97cd2a573777ded057f7d8c86e9b7967527a05567d66f1052dc2c39d5f81e73d6032a4909f537a8d2c89ae73d147e2857d63c99d66dc0250413f2e2048f02676cf98805b5e0d893d614f585697c23867dcbe0e5a2ee8389537d6e229642d0a3b6f8b6f6e15c03e5f10077558dec7f0ac9fb5eeb58997886ff8bec4cee85b194c403900573e661b585eb1ba984a18423430a063657be027592070d810a5fc77a6e36743a5b4e80e6c740473a5094358f22c106bb5dd4e478c48fb09e84da8fcac2eddf5eba5f95db9246e6f14b190b5115a086c6995a1b3e266dbfaf170045dd6a7a22e18b8da3f97a41144277d9d6a83688d16ef5c4d9b32161086cb5d40a66c7e644774b5d9388db38ec4a000c05444744916cb46743412d7ba6525cb3d791667b0c597de6";
	cipherText = "64c69900c8fcf20790bf62b9f6a49c39ed3068daeacfbb030a64679b332f457c51ca1100d98dc51fa5641cfe7274c4603a2c8a39eb33ed4c1d3e6768544777f1a803d968f793649bb7a5fe9e61249de813e413e1cdaf54b4b9578fa608458ee3e27193fa0863c743b5c8b3fea0a295758a168804efb6a5c57266bd5492a14744b00c97f909ee66b37961de39214f0659b7bd5164468feecf8800652e148a15d0fedc3ce2fef6b8035428a3a3782de00150366bf39045097fdffc2bc3c3acb603ac04f3314c2857ab3c8fdbf8ef584341c20b49294b2aac856f3ce9fd5bd19522d2d5def052e12d607a480be8aa1f720f244ee5da667437928ca90f5b5528948a212c2f66f1260b7975fdfb80a1ce2dc359777e455970e4c3873c397caead18068951a77d562889b5906d4291257cde3fb0a61ce9cda321f0436a082da51ed22f025f8e63d6d8d4bb37e1ea86ee47fe78b7ae6c6d6e3307c945bd62be002125aa2a452e2b2ce9410297f1aa399ae40d39161dfb109d5f54645454c6360d6b28c9a44bb1f1b531472c9acec596e9134fedbf362b6f83dce73413f42e463af9195130c868db5872ee5d0ccb3b5df0b1846cf9cf310ed389146aed1ae20ad653d453601bc7a7bfd8554a1c8183a4c3f02ada875dd767b3ab28f7bb30104d6d34cbae51c01f7a8b642fae70de25c0fb9f55cd3ce27bcf2f1466ec06278cc57a02b7b0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 488;
	dataLen = 4096;
	combinedKey = "627d80144e0025adeec127c33098475d8277c95cf1df814dd82860c9c320ffeb";
	iv = "6e7194140a1994cb0cc72a7b89b7fe0e";
	plainText = "416d6325d9d78cc2a3c501f9d2576151f9409e8505150d20fb42d48df8f5b681b5c49d8606bb4aed7e392e958e9a00fada59db1d77664e2d22b71e553494eb35f6c7dfe0e55313f9432f292e0adf1dc627da530e926848f29734ae1039b444c1f7bf44558917d58ce8a56a165a74a80f3ec0eca83bd61c254ee777d6c0c68ce730f6f7b34fec82a66f6cc01b2350589b3d3ab9e125674b6688a23076eb5dd7c2dc036acd1b083371b81bdfa6801d98ca5c517791df815ce8a1aa2777892ef15b51f16bc51a1e2fed565c7af60ab2e8528696cbfea6eff54c9aaa49e5773e1149fe28d75d4440e5e7502767d3758c3f7e25e025189c9afbe66e4aded782a6110cf106494ab6d513ef2378b6befd9db45d1bcb836e7cfe00bc8b2022e17f47e35ab3eaf7009a77f7261330d5b798be8623db7b6c7d78b08283a75bbf4bea3d8d8042062eaa5e0ca1aa74f67cfada103bf199fcb509335dcb5da11a4e28c91f8a785e6a1d92311e53f100716762b51738914699b322252423aaad3a7dd9dc22a53a3335ecb1d40c386482497126de9f7fd1cad82bf45412b8bdfdad049e75479c23f531d9f066e38ef1aa44539d5bf129a2d21f8e2c277f9955c1d7a8ac63d62f1fc7d42526b8bc7504de03e497cf55bf3e9dd4d2d9b0419761403d57b57f978aaca4fead80ba7212ccca08e1fe3e94a2e61677d4abfcac00e03dfd3fc79ddfe0ef";
	cipherText = "b2bcd4bf56694130ac93dc37f40a03c264061fe3aa4db2d5f9cd76bab3b3b187397e67b7c6b7a6dcada03ff61378c2fac212da984b1ff3ccef64aeac94db8b97a2175c1f24daac5aae2f05c9c8189559808d584853d6a61be740a5ca8f3de1668dea98fa579ee542c41a0ac529b73dc050040804710899a9b017941d3367beb7f743a36d84a2afb592124941540f428117bbe998130a6f70b6c688f71562ad11933576e708a6ff564d6a07de98c23cd8877746176101f0b8ba781dda2927625d082675c0d2650b2da4aa327adada4f5b33f8da8e53d51a284b9107196032f5c3d9681a6711a06ac2338a194c4454168d0daf2278ffac5f1cea97c4f2fb0576502b40adbd1d9ffccd8983309a4d1b5c58ce224f4a3388df1c3d0adbc4153b77e0b50b84556b777ce0b1569c12731c93ec1019c93b475a99ebdb03de361d1e055bbe0340ff2adf8a231812428793f1baf53952c9c7e23ebc0f4738941c1aa9d19824e413b3cd3d5811b2b165c4f5eb5574b7377b3d1fad2c542011e904ce253d876d612940ad065262c3612e213407178597b09a300aa1d3cef83f3568be9f8616af48d71470d8bc96f81cff6c54391e3327f827436ef6b5ec70d1cc8bedfa369ad2751720257a2a4b60e0ed9c4953aedab68b03304a539c83bb09105e440600c2f943381882c1240982fddb8473f5cd57ea25cffcb5514e27c6993e302a681109";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 489;
	dataLen = 4096;
	combinedKey = "6a6715630de9e29b34af235e98e2a973c12728add1e97567b0425a22f42b4225";
	iv = "37b261ab5062e3d4e3bcd97d532845bc";
	plainText = "0dec51d652af001c8532f0b0e6fab80fcfab130edbd8bb3737c63efb9c16d33f3fe5a62747505fcf713188678bfd470574e9c57e3d17565621cb47b7972cab615ddab87cf7e040444fd63b7f48b6a48c64aaeea9cff4d62c705f35f9540a58a594491bb1e4b4e0f15a83cdd6b272f51c6472f73ca26dd05fef6f14828b9125f69194e16a75350b23a778d885c9ab753154ca9da607c68acb02e56398b174ce2593e59da4b3bb5d83267391393485e6065b310b1e6037a0798d9fdc5f5e95787c46938898c0a528aad3779112a5315dd1107f236bb63c5bfc6e8cddb0498367e52511afec67e4aa9472d723048b2c8ee4e31abe1d89a5dc65389516ca6a0db420136ca8608955cdba4dc669f9177b21834842be9f5e45352cb9e54482219046d21b9d3da02a9bd14d326b269909d5cb8b494c11320ad82a459f362abb7ae3c6ad6d9a4e4fb78fdfbac902014268e977b1ef0e04f58efd82abbe6a3e019ec79ba062c599a15fe2454f0ba85ac00a3cc15051a523c0b38a1591a38164b92a1f4a21dd018300bd660f210cb8b78d80e2cad69d383f361a6112347eb4f4c0ef246ae747e9902af85b8290f5dddadd6dfd78c49ab81745f77e0fb53fd17ed5d2c26d49d93a9756fe542a94f88bd12f04cf0956ef3ee7e71da746b62e5750cd81b4139631446897f8ca197a3abfd7f40280bbd0ecd9564cc4e88e07eb7a84fe01e34a5d";
	cipherText = "1569e92827cbc6940ae71b90b3a5303ee6c46fffdf1d7c3c3e18a738381d805ec727ceca7e0aaed19a03e606fecd008b74c13a04ebfbfd75441d9da07842b5127ea7116d62d7ab7003568a31978a2acc73173e8f9c0bc914b8f6d58c9a5a7344c4c426084b685a8a015567f41d1cb2e58a3d4c0c6fbc386bf4c70e8410075f49606b07a2263af8e780110dde80bbefc8bd311a0b52c18c516e78365d5c30db0f2bef8e79d84faeba1879f8bdd6de4adc91afbea57fe73c507162590c82d6c657d229075b49e7353c56cfb0b7a705cb7008875d21a953226a477c6c857b4372bd71e893d8d214c94b5b323063643bedabb08adf53c4690af32b940fcce41fefcbac3ea19d0d6a3605c8b7f5147f1cadd0dbf00689b3c87a5a455f732033d1241c229513ec8d4fec743c2afdbab381d0c354addaae5ab852e33cd79fcd2979a5d7b943e8f195e3d79ac2031c97cf0f13961be2ce2a3fd4d9a17d8587dbd8caab845965477a5665c2608f56eb6ef6d7a0871f4c9e35595ed2353bbbaf6bdba61c74fa070759f710d9ef3d879b3be3498d87a1552d339331224a94d7c7cc6e8697af85628955920b627412219204aa8a450e911e449b640733ebef85f9b75725c61a7d4f86ee9c395f970b6c2d5c33d518033695b8a75914fec15a9cce8742803f241c4c143e32c0f93fcbf49732a4bc79897c4e55991af863913850a20f513b249a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 490;
	dataLen = 4096;
	combinedKey = "5e85a8ba6729cde15968279989e3792fe195f2adfe71defa76f6edc75e566997";
	iv = "f69d1d246f07898526a0c5dd7763efa4";
	plainText = "23e8e727eabd42ff10d33933ffa5ebf611e30640672ef7df5cc589de99f826226d5be4e2a0a6e077d385e92b27e8a460bf7e01d900a725ad9ad5ac1982fd5d6bbbd369fae515cb592a55bcaa3eaf4d1c94bf5a7976f3e5746bd0d7269bcc2aa723a28648b74747a77214e4f170d66ccbd4b0b9d4496bdb6a8d76844c6babc7098105f49dd77eb56bdbd4af170a1d3c0c4f419589d949482a95960f32df144f268002febef02225dc16ff81180c8aed7d6e30a897a2d3c0e9d44314b91d625c2955dcc55370a9da30c5260c25786c15416f47894fb5b39e58a1c38c126dc81e0f786e9365545835a3347d534774a8a7918e213b744073fbde1e2c78ca9c11d99f21d5e52392cf87451f263c12cf66d85a6567c5e39ae7f6990c1b9812337a19c2818bb9ecebdb6b9cf78af2aaa3c49a6fb1c66be94d92b00d1477fc096b29cfca19d6dfd3765905ea75d62b37291fd6825e86accd3993ce19a36c1b7c866cf49153fb08d96d8ed6c4597323715f103aac1d6a040956cec3ae7b54c88b5656a075030dbe1b25de4441ab6eac5006823c3857ab2a920e6a5aebc5f4694abfea66bdd3b00a2b338de0e9baa8fbf532693f6db58b9aa01656da84eb77d3c74845b91823083f643d23c4363216cf3ea866ea9228c1004c8d1193dbcbeb6cc65d72fb4a1b45d6d2aa0082eb0238980aac27cb0bbb0000e7e43a23a770eac182764c45ad";
	cipherText = "af7dd1f8e98373911141d5eae570959c516b0329ad74f1ce2d05506c98a98e1c3b65e8da915a23bd1a276d77dd81598862876dd75f753ce24986edb95f00aef9353a48b0340da9b9e26222c070d21e22dd4a6d80636990a339d2df0359bf10126b3928c168ec13ebe19f771d5d5f1c1c4bd4c8b1492755bc202e11544ee599ff2e85eba4f0193b6ee060ad4722ba391ba92499d890301fe2b3cd8966c584313a9a4f31b4344d12e731c327f207495b6a119c43e4aeb2ae9c23d3ae0ddbbd01c503c75ce2805715f62425b158bedfc79dc36a243a6d72c5dff780d3bbb6c59d256e14514db86dfd1f9317283ab5be8c89fd85974f7748342897daed5b33377cf549b3bbcdf4cfd3199b5c103a88f8eea8a78a73aa7b681caa6b30278e218ca89827619469fa799c001ee02f91bab004f98298c6e531002d60d41e37e31f19231a6f5c87d7b5547ae3510ba031c37333ff93bb8e20365633e836593108a7b36958fb62f80b5dba830829534b3d874d07af6780c8c9acc9bed525ff08e3bc9dad16299c9ebb3ce1451de0d90921de9cf7a4a571c3b9bba892f81ea68f711164246d157fa658f2d001b9e8eb5063f03a7eb9ab855306423dc4b0cef8c860d9c02eef0a90836b68a344a728e3c6859c894ff519fb28dc5814c26b0c30292e19b7d97944aaa9148325c17a2ce8613d10f3b063f080e42e40e1f242b81345f4ba830e08";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 491;
	dataLen = 4096;
	combinedKey = "58edaea9c8a2f42611719f98c6fbf3dbf6107cd4fc9faef328f87a47f95cc63b";
	iv = "ae11102d60e043c23c2cf486323cc836";
	plainText = "d6a0cb85a11d2335d7c314d184ddc50470df751bcc5a015e4911de24405b372fd56d17bc3cbf7b0411f40be3c9a6b2290be9f9845ceeecf3d7b6b134cbd8c1180bc9a9380ebe6d65aed2f0a793fda3686f4dabd5d716de7e579cc57a91f30e8cbc535920b1f3110b7e0e99ef76e39d7833f12b8a04549c9f372d44e3649c66ddddae3bc924158ae3f61e5605cb43e8725ccc124d124a9c672745462db88a8fce6c4286a8ac87d2819855b70c58f77bf76b485dc1b7d4991a8372e1f6a9149335ba0cb2069bb3200d015a2b3af6007622ccdfdcafaf1ee6b2a5734e8423e38168fa1eaae473999f9cd171bf0303fa71f1e4c659c0d864b816ed2611f2b283963b3c44e4a1b24b2e88558bb7171d3095d58de4390f7ffbedf3ffb67fa25c1f0e5546a3c6c1e1cf5db3bf0b9e5d859d4f68f7acbd358019095d997bf180d1048cd960feba9c75bbaa897065d68b2ba00e679a479bd0582044c7b8b213fcbd4c780d8749bdd08d4b773ca52419f6517dd4a2a7bdc162284ae9f236348f84b7436098c648732ed33ad47b82bb0164a974162cab9f8293c9b2db098941706bd64559b241074ca3884980e992f99bb5bc9374018a2020adc8fdb670c4f567a369601d4d609894dfd75460857e3a1adb1c24d725dd7f8eb40238307c24a109b1d5b133b5bebceca028b35181e8bfcca830122b2cb07f51d95d8d0eaceb7837e28063dcb4";
	cipherText = "b3d47290f9c068413773df35bcbfa880d537346613851e1b629cf790d10ff1a42052b0111b808a5cfdfef127e3dbc9855d114b4ef4114608a59700a1d0816f738eee47c2440cb1d05ea1f88b87cd7dd3a0b23c8df6b11694bfc2cbe4352bd936d04f2945e57741bec6410f84294e79d00c44de323f9dc3e9a585ed1e3e06c13b4629bba6f76976f022c031b8a2763f0e826b02e9b20c609b1634ea4b51dd2e4ea36d0e5bbe1ef5107e3d146f07677fd1bc73f183db5b1c1b0e4cb57d1b06ef796fe321b5fa9ca2580e798069f644f88b4cfffa443a35020c3f95b2811a943e80de1f9bf208ce72657a7604a5254a1fe12616824ddcd363bd76c81bc1e988a710a4f7dfb0b7117474cc8311948f795c29d416c39d56412486c61a5ab94db037da4c7b803a595e0ee286bf39e5ac98ba452aeedf9430f481f987c3aea40c96a9c3264ec6c297989d6c81c6d97bc4900d7bbf5825cae5d906b2b0363ffe99be06eef3965d97b8f761aa1bbd968808e381cca83bf9ac87680ff804832a08223f8b9cc328dd90b1b33a78e6a1bbb65a983af44794a06832fa6526996d24ac2b0ff08d9d516c153de6d1e9b42b3f27b112e5d70f0182f775353e508a4fe61d8a2387223d77005470c32ece482d1d04097aa09084c7d43d908235394c3c19020410ec4e470a1db1afb6df2078b762889ec285c4bffa3d1024428cd08e1894faa54026d8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 492;
	dataLen = 4096;
	combinedKey = "3fa5eab4835500bcbe72dfdc18ceb2c2f8fb8ddea4c5244f9f77afe8981d96ad";
	iv = "482a394df1392602f60b972f543c518e";
	plainText = "64fde316e46b5190cf4ccfb5e012aa59c31026187d87c56d66dc8bda913279f05734f4ed21262b5fc9d13d569a3b3824ae80ae30fdfa0c8849d78a7b349869d5e01668ba5dda0d9e24f2c38afb103ecca64d760f7c98ef29e07d2993bfc61228d2a2cc8f0bf05b8a1bfbef9bd1b7996a5964b287f54a2329e9c03118e35b801c161840fcca60e970147ba3533997c8c148299bad3581e63d23c2e8fc9f00f4da6d6fe51492a7257da48ba2d3a12b866e973f285604dfee8998c99263b50fd9e87e28bb46aacae976474446d7dd3c1d609d930e37dacdb27a406cefcaa62daf6a5344fc3167828c2e02776be7a483efc0a2d022fca6a62a531afdf7a459828c152d259e1ac4361c811f4a616fc9382d3a8ba49ad49208c6c6812346edfc7692b27f33ac10e7d130de67ee899571a113b546ffa0facb06c1a6a000235f73ecaadfe99157f3e4d7534d6ac69bc632c8a5c54e92c3ff67bc65a88de58f5f057a2b2d6975e38309984adf8f546f2fbc608222d3bc95db3e847f67be0ba64eb7dfcba9ea16cc9059f182f5391b8acb47e8ad02ae9684a0b9fa825dd926eecd7542ca351c0f03b79737573172c613c1354939f60d0bf2313e47ab847c3dd82120cb973f5e2b8bc6f502f79c0e8d7b347376b47cf4d62356bd3cca387c9569ce285afecef0b85cf5bb2ae81f47a0d5fe5d555374864b872c6c237ab66081cef717f5b6de";
	cipherText = "03dee9f52c16a00aa3b1e3eec26b1122ba48fbbce70a7a4ccd1fe92babd8c1aea7cff19a15ff9f57cf6a1c314a42a0ff8eb69c830c85d23be7aa13e393c50cc8111b4e7c2f664e7663db7ab1dcb0bb9341edcec7bd2996b01202fe4e9606d0d0e6d391c905945f1663b64f42706fdc54fd9552b0e6213561e217863321f268e267e28ddf40fa69ce5fe0a056cb82e719e6e0894aab67d2f46082e02136855c8ead50509655a50593ba8b5c633607666c64502e27410d435802a8c24552f77d3cae222f4da23e564e95c8662e08f19ebb326dcfe0320cd288f2b8973ae2c80f4784034807cd6389d15ac28777c07cc44c32458c8ea8429a7521e4b87b4b78f1f2c21d301e2ac0f0a9150344d2036b31418a5f1965c1963c68168487235383443578c5806db812f0f40a69e9e83cf46269ebd928fe8dcc63157e774f3ef3d9098c8cefb7b1857ba28b0bbd069f04999f0c52410612e0c241f7b08234f70d8f4047dbd38213852c65f26cd146d3cf0ff2b0c6d2f531274560f15573bd64772a7420d4b30e37ef4edd6f6d0993daa887704c78e117f35fca712883526c89fa49f9f015e741c55a995f81fdca8a6f54490e45296eecf0869d2d4f0311fb07803d69a185a113f7382161211e39a991d0df48e34aefcfcfbc3085e4060160f5517bab9eaddbb3372ec41fdba12f97142b1b040567334f0377c1de5683ca2e80354519bd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 493;
	dataLen = 4096;
	combinedKey = "1c21b198913a6fad46972fca208c5713e70a3c37651fdaff3c45eee9c20b7f19";
	iv = "5146c53378843aa0140ee51999b51ff4";
	plainText = "e0a38b519f51b9d16931df21fd476687381761886fd19e7085e89ca9ec2f9a1ad0ce43380d8c7555792ab44039647f70f502043daa05ed11601233fd43c7ed4018be1ce6a1dd3ee59b6fd8bb6eb83b95874d8ef3cfdc51886b8bf98d2e1171697ee460b8726c2274eaac7f85ee498e66ecb0b1e5a6bdd371281579d22e718c6e6ab4dfdda451eed19795177ef93f730f39ff04124b818544ada43a22955a21f372e8a51813b0628aeeeb6cd7a2575a346ebd512da1665e20ced92c0cb94fa7c0c7fb18026b48ca38c829f078f012513888afda039814cbef12a703b53db8d9094acab45fa80853fd838b373526d94e6f162e129527b775ebda60828bf08703713e6eb58c49427fc1beb8ec63b6cd9adcebbd0fd8fa02c79085ef14f42b079562d7df89e38a856b957fc6af983de985c6acecb3bc72269dc5fc45dd00e34538d428d107660928f0dfc5de5958b0db02db991a4d0161be110e978a69d5019ab51c0f90e31fff7563dd0ab2cc490cb1cfcb566435ffb5cef133a519df8f13a524277c8585a98021503ca80e823057c358c7d039a84fe181ce35bf059b54a2f46514ae7d03dfd9608a0f1fd8a1821e933147366097f8957ac268da1fed97d393b32a7ea412b44317c235f7e0e4174db9dcb797cfe37a666c4213ae94e33d84207bc1e909f45804588ccff9c838e7b5111502d78444348a78f3d2a08c57eab0d09ebc";
	cipherText = "7e6bafc409448348b1c11cfd661091e6afc648f98f1bdc06854afe06c49760bbb0e248cb3dcd17f21bfb0bf2ce238690038d65db30cedbd28e8513cb055ad9655df1cd49b68d17b1bc80c6f2736f94ef521de3d8a5d31231b08bb4426561d59d15cf0161e9e37b83c82f892ca227d6898ddee227559415dcc55262e5e23be98e36593995883c9ec22bafeb50154df2c7b824b2c0d6629f4d7b8fdfa7471ad173db5ea2c212a7e92587f649a1e39e28a6de9ee765aa990ff397a549265486a94a9e51eedd0cc46ad913dbb511bee7d02a0d10e8418994f9a2a79307b1fef054879e7f71df5d29924aeb235d3aed66e17e579de2939e53d7cb0c7215ff359dc5cdddce14ce222b9f8be863732dc01fd0213a8886f638bc00bccefb429731fa97cf02db3ed198dec95efcaa2c862a80ec643c680a0f06918e674fca37bdefdf03f8d9087ef3e02da8b7b97d731b746dd3835b525a0884227a37e04545625918217f5289a8779020e635a153e3ae7b2f44e1e35cd8076a13cb487f126926c0662591a62c4c62b4a35a0a256538864fe979531e0467a7f1da67df19c626f2cf3130f852cfc39b4ecee5d081b979ae7ddbc8a05a02741d6cce96555adaf477965f05577ed6980e4680e9e687e75878976cecf31495d6a0e82a6bd03387f56ccbf1bf8c2a0db777b27d442f6ab887587a2faeea4079bc4c0a159de9872380592c426dfa";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 494;
	dataLen = 4096;
	combinedKey = "147253c04ad58aa0d4b53b308ad14a0028399b6d21cdec97cd195f22945ca892";
	iv = "a51b21bf9786b02585dc871e4560566a";
	plainText = "d0a29d79490636e22c64b6f0e2398ef3b42e4cfa4064cf7d7ea8713fe17b8f141e2ca06e7c57b642cdf40302e338b6ade4ac193b54f711a850ba14871bfb67278deef1aff44302aaa183acbe0b5619289c6176227665066f87fb1bffa1e131e2e7e3a721e2a4961f705b29efa64b32d5eb0276d0a797ecf2e61c1791f1c7874be93e893834a3d304d24811ecbf3c99c201550fbfe4f8c9d2be41d02b0a15fe0cd3c8280b8cb8dc29339b68838c30543810be0a41fb24c7bbaf6d184ec70d5885206ed4753eb03927be0731734d1f01853808c71cc58b3655ee9c5ba536696deb3e2a3a958f4d7e8ae769526a2ab3cbf547fda1caf09ab69a94f5f3c289f715d662da8888cfe34d1c26f8b250c7ae0fa3eecc7f5799c80e9cbdbdfca6e2912b13e238f9efe978021ad08ca8ec6f816801c302d5cc3a435a7ab41315e7c83d84d6df2a5f001470f0c6f5f957579de322422a97699c196660befa6db1c7bab9d064a5021daae02c4c32a4c63ba8d688329d1f96517c3776b986daa5bf2e90716dbe8d749e7b5081916d27bf6d100e1954e68732e23d10b64ef3430bb8c1e5cf22e339bafd27a72eb04def5b2a5bce9d9bed18fbb74ce9a86ba84cee972ffd84713fce7d8df31ae6ff361280b4b044aa9655c88cd007593ca5e5bd2b9efa30095c395823021dd8a45372eb2e4ae59b523f70f08de8bc93a8d52a5be5e96e0223675e";
	cipherText = "b8ca233c918d22f030017f798514ee89185a4002c17b15a135b20cc5778b2771a53d314558eee8fe6f70df02c09569530580f0d0638fc93a604999d43d6bdedae0aae43e650ad12dae611ce5c804c66b07d4be2fb437ef06d0c2f4c19cbd0f4e8a05cf5807f0c1da020be7954019ce3cca6998fdfffd69ef74e32e147841e106aea6c445faaa9c93cbdc4eb000df4f61f19c05b2446dd49b31880ab14c7edb350f0e378e5c67d1fecbac7455cef913ad7972466f89cc2da49b4c70eecea0fad8cae4814f532c3c757da7d16b14c4b28e21c62788d5691c99abfd8b21b16fc8ef985066c2ae4dccefc3d9cf9d47a3dbfee6a279b6763ab841a7d393504ca0e85a5c6399fc1ef725236ec51b87cdbb94e025bcfe50f0f47db0ab4926c217f9ff8473c2601d3045589836938a076dd23765ca797cd9fe67158299dd10e81b202a1af4dc8d951c9fecf8e75e6d350c377a0f0398798e619ec0003266fdb3e6dff854ab863ec8c57448f691335e5f47ae0792cf322c15c2cad79f15dc631cb1070f91c3e8a831add85509b6155c0b2dbbfa635c342d69efb602f23f65da3d5c65bc8e199a1a46ac789ed143d689a6c1b4090860ff8d77b5299d8711c8eaa4a527fee0fbcb960c86073c3be2d855a6a6220d4f7e1c35f81c2aba824ad5bd595e575d02aa3700f787f9f57f6dc6eb52183965339e948b49c36d64db85d2625604a5c981";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 495;
	dataLen = 4096;
	combinedKey = "22c5b59653e2e52bb7883501aa801f38ca3ca1179fdd5ca37cc6c0c2078ed3b9";
	iv = "ce559fcb7181275a9440ba0a5eef53ca";
	plainText = "8e32bf8e380337915ea661b30292002e6308c5335d6cf1a780c207483af4508d7f9173979a9c3b12a74d7b015fb685bce2caf7bac5336f73f32da39f34148aa096327f38cdbb3f8b50c5fbf0564fea985ada1cba5990ba501ae3d1e5a87c228063de73fb1c27faf4ab97852622e7333c5ee83f41dfa9ae3166307296840d00387400d96d61d6b17375be0c95bd5e60a4485328036d6ebdf92ac9682f9698170e054ba5fd8ac557bc77663c0aeb1cf5c9c82880192d9d329f9424a8d182e609c87ed297ee64c10cd5b77053c3183dfc05f9934dad29637e043347346653046055e9b0041a98e8579f0abd2db6b5c58c3c9aa1d9e25841cc11a65c6540e5c7b2096978f058cebc819b41f4f39e7faeb894a2a9f196071193960569f9da1fc78fc74b3de2acb724448ae0c08414ba7953de318bd95eb6208ad5b9a005bcfe6ac6f65f4e246cbe75aa22b333fdca74886a66448526bbb32212fdd32be847432134a8924eba41616be5ff1d8b7e6d7d6f4e3341572a815e3cf0e4b347515128a47f5f83b74d32bc9338fdd51991d1acdd0f1982bf59898ca475e5f9d9347d5ef87eea7f007d9f0af3fe6fb7e1b30c7b5cee5101387b9fb974dfd8db10ec14ca17810f169c876bf3721cb815eda1a69f1962c1a28aa1483dc000714a31628e5b735ac4b2e2b23cbe2fa8f5c0f748e13fe480b9d8763592dba6aa20e48991564e22341b";
	cipherText = "53e76fc2fea65a29da72d8ea7cf36399a2b023129f2e1935b912bef249023194da34d41f7b68e82c669504e91aa9b4ba79f6a868774f89457f19e9773ed8fccc531b9d573bf8a9e9ce64f41ac868f9d7b2df951986a0887a95be393a350d5f47a053a5ae7512d9737f91351f46a6ac58147ff79ffbd6933b50eb06993f5403c48eeba03d7541b0344424ad5c43bc203edc21682433503253abb51b416864c78ca93eb930b1200f620bf42edd157d6fdfd5c5d3b3fcb10e1dc264d27a178a3446dfe5bae7a0d6c8f79b007ebadfda9a605e0e15c68e5a9c915b445571c168d80eb4219e9c41460a91998ff4ce9d9a12107ba76adc45b4eb4b9c13ffbd9a9c51da18a58748f2c4dab877e96e4787d6cd2a4a0c546e828ceed4dcd8070a6dae73b406d0d3b30db0df47d99a865a38ea59eec3d89367dbd02a09bab894f6b7f721abf859bb372a50f6d7ee7d997b58f5947ee1e5e5a1795b4b5c8075d456572830797de35b1d29436e2525c430fa5c698d732b7b38a95e03d375ec930c0e92791145fea2f13295b906d2d76be45cedad39a087a637268baa6d19d23055b213f984408f38d8c0e5274f6f2209082ec2f0b8546837154e9cc2fb1c73ab1ffb178119823c527d449f202209eab54ba2a7554c17674ce68e39cabc183f3da25515382932ec67f56b0f7bdc5b7c880e1d073a7b7a89ea72f84f4a5983b5eea3258863c2f6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 496;
	dataLen = 4096;
	combinedKey = "926355c5c5ef67fddf586f2e4883b8136c10664ac2368a0fb1611ab2286841a8";
	iv = "c65e24add9047ecea57e1773063f57ba";
	plainText = "3a0237a59c7848d45402ba2652f0494db78101ffd426fc212cea11304b5fa21494a6e7a9a0aa0230f166e3620a786b48568e913b14951b9e5e22f36c3f24998987a8764b9077cca7651ed0011f6696c817ad08e559b6951d5ac0f923c94da52642f9447878ec2d79df5e7b5e15d2a219d06a5342856e47f280409e88cbe87242cd3950503917b1f5764e8e346dc1b034a4c1dd767849379255435fdf2d83ea2e2a0a05074e3174fbd1bae040683044b2ef24613f1e47779cb168214d5a5a07976624ac4a5ff83223663eacc7136fe57b871b53366e5ac203d313b3021c10796ec1bcebd7b9edd6a14510e5ec3d1bc76cc2cfe474ce4fa23c14b2b924eac77d7bd2b26a6c5fd28a8f646b00e22194d6d2f23d7e55502d7f7302ab661ceb95182fe3a81e7a060238ad72e8f830772b35d8eb4378152296fd49c49e9379f3c2a1c9485d22bc0e3c6d21167e9392719bf36c76d25fe0429896b235e670cac7cdc029ddd909063a5c06b9887c66918a817b5cfc7431014f6c53a99ad3fa006f9b9693d77bde84cb81a115c87eb879007ce881fc5fb685a5b43cf15e672ede3efc19ea2683f4fe6a9308cb25534e0677e26ae65fc6d15ef0110dfb717f8b8cc069b74e7f7d403d412efb60854ce6bc5277ee44ba77c09394995a6db51f889ef5e6be15b6e2d3e43bddb4fb3f7628c61c4a3cacf5fb2bc585f9b4033dddbe88b935b449";
	cipherText = "2b64f9d6746361c7e5494f77b95735f6c5ac4a4c9e299adcbc465dcb30d6cfab7e92bd6de2f966ed3e471c884a97b25f77d5ec058036d84610a7c3d783b4432824ca82161ff0ab336fab66cad7b22cbe0c94c691c74b03614acc10edb01c34ba522dbff94dcb626cb5413c2a8ed406583607fde2f93617f51971b3eb5ebaafcfc460761880784b35b3bd1fdc657cc9f5d4eb3b23ab81c8181743d88302b88de364d123ed1a4f726e1ee4427614c59c8a25777f61bfae623833c2002f65381cf750799ce94667cf7ca7076ef3169aa9c04f387d8e8a1012adb807cb177ca7864e5e971ee7d3d53d3634bfe4e1348956a0031b0753bdceb358fe69220ae7e65717759b843863c17d52fce0a693b0bd677e7c56388aea1f5243c26de5ca03d79c6c93fc32b1d24d244630e4aa7c6dacbbc7cc88e3ee7a54ba5ad0c5896d72cd28a5ad8bca2b64cba813f7f3e01283d21db267a417412957acc80f980d345edd195600841cce87e18e24f1b57e65a4e9f88a51e0d5f7b5ff7a997531ed590702b91931d47ec2d60e7a940c152e528f6d71dab4e2210e325b117c22294847adbe9874bf8332244deb9b1761f856d25590e16f8547975d3182185077cdad00696c34a64de1627ea87c4738ee3ae7bb85dbc3acc89e5eee49f7b0e44aa54be425e28097bfb0e448a9e19dbf033593ba18fd38a162d755ae3d8d27258957bc5367a76d8b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 497;
	dataLen = 4096;
	combinedKey = "94d97b403d657fb51493e9b5913261303b27fe4e2d2988ecdb9e35f66a74ea97";
	iv = "92bde0206c98027da74579bec42084bc";
	plainText = "01f23eafe9bf1ee657d4454a23bf9c6833864582e6e8fe16d80451a18699d80f99269b5936e17a7c0ea0fbb503330e27db92c9461c5c46bd64f5fe13b5d1989c4beb8e86175ec9deb70b5fcbff74d64eb21380c3eabfc1804e1c5fdf5eaf2e6dda6b74ad2e96068435905a9a2635d1540872e38e29698b8e11bf4236c5e3edf228906f8dcee7d095f94bc53314d6ad9c860b4d8c5a411839d8f61ffb28a7dbddf73566a1dbf45d141cc3adfa260ba6ed50c5af5238759ae8adf3037169e71c73a70d7cffa2a60451426bbe7d638181ff2d58765bf164d9dd2e23ac557e5b0c7c5d51e21ea475f369f43320248e52c376c9df7b9485f71812f09cd6972a9c66d436d13209f60197a2793c8dce3b549c27e4463017dbca52f284fc9f767a9d397223c92da86d520fed152572e91973fd01268bfe16ebb1c0498db60b24e6417680c34b49e22b64496f240944c06a657fd78c7bc21d3a9a20f088883712ee5590173d6ec2da888a46ff1b89d637ea4e7360b3d97723ccc624f25af48ba16a05bcdfe33d9a08f38dcb981140e415bb366774ccd793c35a0b3f6e3fd4fe469784e89be1d047f4191d8055bf116ce5ba1e754d326ffafbe09b9773f646b9058749008fe57337f6acf4951f71e011d2a1f98663b96a42f27e1a72d1b74967d143c7b2837d023ffd7d07acdbec6e9705b1ad5c688726f4f9c1bb9e3acee1a8b0f38792d6";
	cipherText = "957774270ac16c75523a910a0efd035ce66adc6b24272d6a3af1ddad7899f1158e2bae85d8b673d2336fbf04ede7ca68fc52487fc1c8c3989a2dc3471a0aee5302fdfdd271ef61e932e10c102d1889d8fe59d010f15f5f4ad9ab5c10c30e6b555fcb736901a919a4d76ad1d16250e84434286c605fe52f0767085b0a32cd13ac1a5245ede8f264b6484926c7632e323fcb30975990c9f10bf03f0963760f9029607a6445f2378daa3f9c4e712e747742da73d2ba4753ecc11917bee1586f00f12560f5a15b9267ab9af071fc51f9fd5b016af785505e12dc75bbea354634ca10c3d9a8e964bd212fe6d2d878d1b86d14d00bd5406e5531cbbc94c2992e6839161987aee7bab2d7dcbb329105509db84c129c397cc3fa807802bccb343c5ea49ffabeccfde6a6fe4cad950bbff83a4723d9c86d52022f44495bf275c2ec79fd3b9d4b2d1a4521e7eeee2f48a0856d22120be84c4d63e9a714f781adc4a3f100a29ace3dd8e6e25df7f56ea8bd5902ba2061ee9efc017c5566460e12b6c1bd0df9afb8ed3326fdc11f6595f08c5dfc266d2c8b1e450b401520070091a0f47dafa56d885919f7ecd149efaf7cf68008a096d5fa8b89e62199e2fd70080ee29b28931e6720113b355ff4f992b33713adcbc726a23089c99bdd4f667df5105b5587973c9ebe52231b2d03febfba9049e6a0f3dbb05a26b7ac33ed64a981467a8a8e89";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 498;
	dataLen = 4096;
	combinedKey = "a96fe624320caa9af4cab7a86ab88570d85e7c404eca151859acb27bae067c2d";
	iv = "f5a7917dd191555f22aa6e798141a997";
	plainText = "e6aa287399d3d111471614c3198a6569f5d4958fcff47d1b8701a7ef0d2c31dd37c3272651624d989cdbbc4b7c5414a7b488d23fba0dc77dd8d7bb9b80ca7a8653223d053308a716740fa1f63ea30362f7f6d41d7c9c9e9473277102d35e23b131013796f15d351f2c30c622073600d02742faf9f33b24f84a0e0880bd9f7b99c09c005a762e1488ea79939221bd130addc961a3dce8359d26d81189eef4b93935018e0bce793e82f6fa04bb6b10f6af3780d119c674fb67fd2d2aa82aac5bc0ddea7cb64f38e5d6d0977f582a683027172b18175c1cd66a91c011562b379375bffe039f13c61ec372a597cd2a12f343192b83d93e16b3878d282dc3fec877f31c0d7b9a5e70ec915770cd711e8c8113f74461e47f67496d5794f50731a673d5002718f282267c2ed45afa5d2f1b84660620778fe0d497540c31f458424a21d425d64b5477cc626e8a6beedc42d7f50a733845461d2fe13fb46cea3b0fc2347b1f70142bf3a6eb040ed3b80903ab615ef6adafbd40157ad4f52e30fddcb15ac865fc595db65e79fec48909c195ae580ef61546b50e49e504962f6650832c2be773c7508dc258339143b81e3abd6ca7ea4921f07110427ace4a762726223936ebf439f94c199988ef44fce83c8a94355e617be8225991f497245fa5ff7d818b811975184f3ad54ddac051f8b346063260471fc2748143d2bb409cdfd3f55f4187";
	cipherText = "45c4f933a6ac901c3a6d8d17784d4758bd19cde363480c9467f1ad8c58f3605b3c6bf174ac926c66e194453ca5a5dac0d748bd603b3576681409f2e4c91557a0910ab88b8c898b9b9a6a170dce8951e7e042c9a4f471637327e23f7451228240447c0828d4cd2ff8c5a1179a2a7fc7623015cb642ea4649b13c2d47eaad05eaa81f7ff2d95175a8e2ab249f6b8f3345c61d6aa737ed777206e3fef5dc7f20bdba7923809b672c5c39e3ad952e538f643d879512ad58dacc7d0be927c98e2267786fa9307b1b0bc1f0166765363fb877bcae4d44423ec3a24817e59f6f9bedb6707402b2eaa379423071c8169419546ca33b00a8a5c6d4ea6e9e500c57163e530be61a2a2eae0434fb18ef12bd53ff4b42ca64feef98d44decbf58f1a54add6b7bc70f9988e554f34dd07a4a1e7b0b77bd27f5def9e3697fe28144b4255bdc10fcf08d9a7372f901b7a1e00255546ba79c0297662dbb18a3062ff547c12bba45ef9c1eba44a8b93771e88a058ab5f24d42ab5db5fbeb0c1c68d5417ab683886edc2365d5936bcfc5079d848bffc640d18961049adc6c2ffc451b8bebaa93342786995101b965f803c3a280c80caecb80fae985f5fd876a7b29d1635a61d673f15d774727747abc23d7cbb631d3148f4d44394961397614d5a8982a5c6eba2aabe303d37a7a37bbbc1a0108a2608c0490a6747763922efac8a075ef9c444aac038";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 499;
	dataLen = 4096;
	combinedKey = "c9efd992e96f27ef71165cd9db1ba628d7da0a1cd2cc85a8ad47d169a9d028c1";
	iv = "be3356b87021381f792b7f1b0fed71a1";
	plainText = "eeb9d19f81ee0c71d14ecefb78425c0d7ce0f3f79feaf2fb61a543b1d0802e9fc9f30507dcdb9c5edf6ffdeff52b072b12b108eeb1531c954ccd3d19c62334b7723670f02126b432e11ef289e0a38aad5f1e8dc7a64b63436599077379ed049bf543a28728ef4809da3c7e32ca63b716bcd63e68e3d3f4d0ae0a574ce77493b75dd7a4689dc8aad6f73297b74a90544f793e9553c9076a777293cbe2d574eeca41bb7cfe87e306655388085e4b0e74ffd19a91151a3daa9f451f43cf81af9e75ac8469bbfcdcb87c8fe7aebad86c6d1ae13ef54d2116a26b3419516f79992cec1f8df58427667caff18290463a929a89ccf9ae17cc6c0224ea8a8a7ce374277dc1c2db8a072e137e857f67e9e858e0fbc699d9da4c3d3bbbd668abf71f3b165fa9a5960342b5549405df373f28fcb59ffd9038e8575e446a5f02e52696b5628d5f9f8f05ce00a0647fded836aad1be0a010af79e208ca940a19dfb959a178e12b5e6d506cff70fef6ef1368cb1f8c64e835d57ef97599535a98a4ee773dc7090adc5765261ab171c3de55867066f8018a96755dc9e53605450d6c9fb7a349819354a558249c67b2d37bd42c20e70be34c4e804d7864c4cff3ccc223a47043f9da3cdd53ef2db03093e0b7f1116984641ed31f8b44cf7ccbf297034a869b71d2b8f068d58f52574ca5b33bfd27f4eeed6fe736159ba75055e51e5149d9cdf3f33";
	cipherText = "0778dd20b23e07b5764f8af0a1b117afbb928a791a4cffcd71b2892bacd49d45904b5782e8658fc3a25667d506a44314181fe8b62cb7b7b00bbb07fc104dc9241707548136c41c7ef3f39de6f8606220c5ec6ea20e260e316fd1c73927bd038332dd7e87dcfe6b08fd49a966f7cba7d59fb8c9f80a4bde0652dacf8420b43d3acc6d8b9db92fb8c12afe3690954f800efa6e6102d040b3c344f63781a16e808114df6e077c31849ca0bd93cd31206d39b7976c82ec0c29ffa982b6e4b1a8fc0131bd9f5298796f125463d017d7894d61e75ae1769c854aded4429523d1c1138f4ef7b226dd08db91b0b8bbeab9054e08ca076eba3e8592b96c8ad596a446d49246c7be1f2cec5cda888e1823ad5f998bab3bacba3c76ea8b6bd5b2715098254294acadbe80bfaf63b96db76b19614d5f4740510e2c6a594d2cca89bb5109957db32541847338fc4ce308777762c1c751e20a734f0258dec978372f2e9e10509e41d3b3b7cf301a4df15808d04832fe76e1b3af29ed74fe567f618b266df050f14aef3946cd47e03c4aaaa2bf2d43096a61485594cdfb47c16e3db9085e783e164157f833405a5262516af64dec07b0a25095c8eac46fb0ab4900d4955b727b1855c084abd2bda391d9147b14567df24ed1d79c422490faba7cb5e3ba142280ba4551833d53ec2b4bd290f956f71f8f845bea570ee58e368a944d08db61e2b4a6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 500;
	dataLen = 4096;
	combinedKey = "ada1b135b084f38e4afef1ac95fde93ea9f1b6fa9ec2f0c83e45c7363c4c0421";
	iv = "c112521197014739499e8e026c5e819c";
	plainText = "c5f9dc8266c677bf052357b0ff0c070b0ce680bb3e41dccd28e34b66bf55d69a036fe09ec6392ac45794534d83c0b0f2dd924b30ac1d991cc20ca036300d07ea907a2afb547cc259cd6e120199a9f8c213053dc7574dd8e9e038458930197e642a542b5ecd44167612c468b3c2bbf0f7418d0643f7b9b8218304299cfb0550cb57284658411af7f38fab795debe42429cf1d6a1d12480d8ebf19a8cd29f501bff3aa053c6b93a3bfd1d089283fdd0c2fef20cff29fd43430b11eb9d8edb412bba4f9562538b988ca1626b46fce67729f16c8d3b7dea6d3eaa942cb9bed81236a9bb272f285ad2ae9faf0d59224c45b08dea633d879ade132058f409f5b88ae961e704163d3e409b8a661d08a583897d27ed0da26893291e843302b4e4a556840017139d0b74b89b78258fa1d51e73d4b7cb390eef717f527bf03e8e435004560477981fb7ae39557c455af1a43f0c9d1022ac80cf27cef790d4bc72f981f4cdf1da896eb79e3d9fa8cff6404e38b270dc761f7ae43aae3e128eb5b59956f84fc19453baee21eeda3afc63f9e2c194d4b7fc56f52a99641dad332901be0f257910865a854e3d80272fbc87299d4b247de1b103d5af3b4aa25b87d0cb7b0fbe094171e489b78a8e1eeeacea861a8b7df08af6a7f44504d8e5334fd0ca8a60d6e6030dde961176a507ee352a33faab86ba2cb7e7f2e51dde49db1927aeeac7b04af";
	cipherText = "8916340006f92e1e104d89abd04da59c4c678325a0a9cad495e29ea05b6e8c2c99ce375f711c476cf9a33799530529f71ac623b56c1817b7e1a6f079f96f2cb96b67cba13f419f022b0c28e249cc2381df9f7a7fbad0a0ecc3f8582ff764b6c84e22c68ed8ae5dafdd8a60270ac93d6cb0ebc59da8db5908dc3b0e2899614c72ef104a00af23c921819534ada6e88da82f16490f30fca205f8ed1a3cdd460c3f3be8578b43f81d2168da8585b6c0a921f8154b7de88c88de087a278df38c527496c44f3ca7e21857a211aa0d6f9e2adb330ff239c4bcbd28775b158a941512a5331fc250958802c3b63f9b8238461e4c5e588aa8f17e422436c49da2cd5a519b5ddfeef4f21a87e0c4122c41bc579bc4a47ec4cf1e36e567d90924a0862bd753ff96911b1833c7253c39ff1cc156ed198a9c9c72016cba662f374a8da9667145cb12d57ee3271935d68b8c8bc788c9c27d545e101f6a68bcbf1a13bdce7feabf93c3170b267bc36e3cecf41acb22ec724e8893963de91c7bc56cc3e6d9be6ff58bf1f6de1b64a4bab87aa04b1e470fcb62cd17bc3b031a8f94cd064af6578b21aa37094f14ae7aa969c04a1c615f027236c91525ac1cb2eabc10ff9d72c2cd1fbfa2a4591ea83c7186bc0989c5ac13cdea37a6cf54084bac23fe52c1bc06b1a3308a5e91ad4da4fa3977ffaac3ce89da3dc3fe45df931127f30e471c3df3400f";
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
    
	direction = DECRYPT;
    
    
	caseNumber = 1;
	dataLen = 128;
	combinedKey = "2956cdf0462756d862b23d51d3409863adbc407ebc22cf098afad0fb578dbb08";
	iv = "620bb87ca0fd9492449bcd1ae257c2bf";
	cipherText = "ea6576bb37da365869ee51e56ed36813";
	plainText = "de3763aa76975d42c4939e737990fa7f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 2;
	dataLen = 128;
	combinedKey = "5e798fd9feeb8d966c4b59af575f669f785550c420f3b64a966824a7fce03f19";
	iv = "f6e2c65c2c1948d4d453c42d771c0985";
	cipherText = "c2846568de1ac62d42b7e2dc62fcabed";
	plainText = "8a255bd42d6604c16857a112a3f72dc7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 3;
	dataLen = 128;
	combinedKey = "14c836483a702e0021b9b9fc3febc27fe4fed2ff583934ad52a2872496e31e7b";
	iv = "db3ab77dffb2b93ba39750617378abfd";
	cipherText = "e6cd9433adae7c1cec59243526d558b7";
	plainText = "52ceacbe7c2c293ce5bf8e1fcf512d98";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 4;
	dataLen = 128;
	combinedKey = "c71fb2206a68a864f89a193feecf38ff9c76dc5fc6429a76681857cf65065247";
	iv = "7f8d21c36bf7ddd4049d7fda55906429";
	cipherText = "a338e0355098d6f4f9772142ee42f826";
	plainText = "247bab3951a51a5bd250e8c22c8614d3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 5;
	dataLen = 128;
	combinedKey = "c9037fe68164ce9f15fda4cfa9a9e7888d1c4817aa95c585043059cc932c1c4c";
	iv = "11b47506b1a4b8720115c9dcbbdba6ad";
	cipherText = "645e5dbe1fc1110ec667bd332ec847f6";
	plainText = "04d6f26ca30d9909d509396593cb0e8c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 6;
	dataLen = 128;
	combinedKey = "561884bf8936130ff3fff00eeae9d98fa01b1dabf529cb6c5872680e02315b86";
	iv = "7799bd66a61f5005ded41433add5011c";
	cipherText = "505ce7eeb143bf20131a480902a7c48a";
	plainText = "f5b7f15d73a3673ccd56c8ec1182b4ec";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 7;
	dataLen = 128;
	combinedKey = "d5f5a3893fcb87aaa54cf39f489963b0dabd2a158ca638a285e9284c19b2eb39";
	iv = "07e238828f0f75a1b41a1ccea9c8e049";
	cipherText = "11267e9a1739bed275913bab18e19463";
	plainText = "ab50c5869ebd40d8caf9c67a44238c99";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 8;
	dataLen = 128;
	combinedKey = "04ff37b093532fae21568d5e79f3c5da3bb9d6394ce7234ff9f215804ef14433";
	iv = "61618f6432c2259c1a781b5507280a92";
	cipherText = "6487e98598160f2ff50211a6314180f6";
	plainText = "4a012e9ce702431e4b0a7159f2760a9c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 9;
	dataLen = 128;
	combinedKey = "d8cb9506f97d2477a43984bbedc3a1ccb800b50f90e3c8bc6ee5b5f48daef522";
	iv = "65b6dd81fefe802119ad3ae5fc46a8a8";
	cipherText = "98444a4376196634c608b9515bb50eb7";
	plainText = "f02b0ddfd5178af6f905a65f81b8fd2a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 10;
	dataLen = 128;
	combinedKey = "6f9be68a5c84a70c60676cb6ed1d727fb87bc56596d31f1c1d04e6562a99162f";
	iv = "514d1309af897a30981d535c59f19e86";
	cipherText = "71148f3cc190d3f88864f5bc66736f7a";
	plainText = "743d0a05de8e8f0128f3def81cba8223";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 11;
	dataLen = 128;
	combinedKey = "f836901cc2fa7f67bfe8024c70f76a19680ddd2d9a122980b6c3822c32683dc6";
	iv = "a239202a2b4c42f464b470307eea712e";
	cipherText = "09ac07c0396ef394d7fd50eb13e1a147";
	plainText = "9f75c38a5b80b0d3c109a6c6fd343c3a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 12;
	dataLen = 128;
	combinedKey = "8bb5a71f8229e7277dbf3d59991b322988a70b9522bcf1722c29cf9da8363e1f";
	iv = "5aaef8905fbf5cc5bbc69664199c16b1";
	cipherText = "6b26927456e3e86b6747bcf04938a009";
	plainText = "8c89f6658fa2f0421763448023336870";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 13;
	dataLen = 128;
	combinedKey = "b7d493a31b8ed2d1bdd1943b009ae0992cb651a0254d2a74b1d6ba5ac08c332c";
	iv = "87d592932d30b8cf6f7ac67ca8b64929";
	cipherText = "384d5ef525aeee4ef7cee3833983c1a7";
	plainText = "c4755dbc8d510e5e1ab28772989d7223";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 14;
	dataLen = 128;
	combinedKey = "e857fde665a852be26f3e289132ce8ee5043254457c79f3db936850d72ad05f4";
	iv = "2c6afbbd7f8b56e6417cd7d5f68267d8";
	cipherText = "6e51e19e18b7bd0a8de0ee8c403135fc";
	plainText = "be907f21c216afd5e141038d935a5769";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 15;
	dataLen = 128;
	combinedKey = "80e5d75aa943f3a3207d10f5609e04e4e5e9732243371347a6f32fbb19fd94df";
	iv = "76bc2071b9c4518969bc84b2166ed5d7";
	cipherText = "d9ffa12e760ab28fba465936ab7dcfac";
	plainText = "4a5dc4d392fc5f1c45da75111fa77356";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 16;
	dataLen = 128;
	combinedKey = "fac9adaab6738bdc35eeac0493c4453ed22ff6f1dcea8c3cd4292e348d593042";
	iv = "63d891d68eb770a4e76f8368ec4deb91";
	cipherText = "1991a626b9e7daca316a56b26f82ea6f";
	plainText = "7ef299921fb634187ba7e0bbaf0c3ecd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 17;
	dataLen = 128;
	combinedKey = "59455366f996dd5750dec459f6183f4076d42c75682a7be55d1667b3bcf4b1e9";
	iv = "e2c19ee90091539a5c05532a57a91dab";
	cipherText = "d015ca69d2a24f72b75454738aaaf194";
	plainText = "2ff9a412338f4a7ca09a25f7ba905cb9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 18;
	dataLen = 128;
	combinedKey = "b32fa9b7a9282dfb7362f49b0091a8cb14cba26de8e09da96d3b090a39294c19";
	iv = "9868a1ead61792170833dfc66e110a37";
	cipherText = "9c48e9d632e9d3e1055de1f14ce119ef";
	plainText = "44903d058e0555c57f51f4beea200488";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 19;
	dataLen = 128;
	combinedKey = "556c2d36b4b09159d27089c8407040fe0197b9406b1bfef5d7241605b7c3ac9b";
	iv = "5a7c22b6caae54b23b68f8e9ef812eb1";
	cipherText = "3fc5c84d9ee78b762d41e037941abe76";
	plainText = "13906254cd4b7258209e185c45e022b2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 20;
	dataLen = 128;
	combinedKey = "47812e2e6f0ca9494a3c67fee4419fddabf4bc5dbddfc1250e64de0b1c224fa9";
	iv = "7b5fcec712eed0e65235b0b277ba69a4";
	cipherText = "8cb6965a87fe768c9c576fa6900220fe";
	plainText = "b865188771a1af5343468fbf3996fa2c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 21;
	dataLen = 128;
	combinedKey = "ed27ce42e7d3b6ceabb79a61642c6f3ff6a5a0d9a7498b085ebca3c88bb2fcdd";
	iv = "a623df0a4bc4eaf432754be760c6f761";
	cipherText = "f4f5b99a1af97497fcd6005941c52367";
	plainText = "7cecb88a37f3194fe206a9a3864dc1e7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 22;
	dataLen = 128;
	combinedKey = "7baf9bc3003826eb48f5ac711db1a2972c85ab868e64a7153b0330fc3154f6d4";
	iv = "89c6c6a2c57c884d9be6fe61d2d9e2a3";
	cipherText = "d6f889384f5f4d5077f5e9f75bd87355";
	plainText = "1e4410d5ea4200fd829b6e2cc306d0f3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 23;
	dataLen = 128;
	combinedKey = "04bedbdf82a56b090dcc5ee0035b10280bbb4fb05b19ce155a672907bdbaef39";
	iv = "6948ed770623580a4a09bb91c0cb549d";
	cipherText = "cf90f8b964a9edd2499926eab01f62dc";
	plainText = "ba5e20d4b8186712a199c80a1cc03947";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 24;
	dataLen = 128;
	combinedKey = "f9fd881be853868bc3d64f7a2dd7b221f00fd28e05f16d7acbdc63775fca4294";
	iv = "e3166e983a84e4583d04f7293ce217a1";
	cipherText = "20a9e3d83ae0f3c756594c8cb62164cd";
	plainText = "92a113b0a0b6577e2452d9b1f17d5eea";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 25;
	dataLen = 128;
	combinedKey = "8b1a7aefcef62a0a3449ceee135e9e2b080bfd0d69736ebe0da174556f821327";
	iv = "67508cc9dd8c1be7a64bde16b5935e5b";
	cipherText = "39130ed8621df9ca75d71524de39dea6";
	plainText = "ab4130ccf9b2ca4e9621c62b4e7c7018";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 26;
	dataLen = 128;
	combinedKey = "9e2a82903c27f9f6a0cb632fc2045fc49714832d9e16d233ee93382f3e02cdd1";
	iv = "69ac854fbba71f45a6602fc17197d826";
	cipherText = "9ca914a7424584fa5b6e05b26e2e8210";
	plainText = "a683c151f0eafd7aa4428cd86523c769";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 27;
	dataLen = 128;
	combinedKey = "31b2541a96cc6f1eeeeb1f6c115b6b55289a2581009b46b51c65c017d311377a";
	iv = "4dd6aac74805bcd9af69a11e84c99286";
	cipherText = "f397ecb15f511a0cb1254bc9af9549c2";
	plainText = "f0e180a0ff2eb8d18e65f3aad8a719b4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 28;
	dataLen = 128;
	combinedKey = "a04a8c93aa9599f272189707f1d4231e90f4100d8be39d66126e99b2a83bd9fd";
	iv = "db6c9268806d81243275fc965345851c";
	cipherText = "253f6d8ac3fce38050b37eba58ea948f";
	plainText = "5c5516e8d961b6a91023432c2363c504";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 29;
	dataLen = 128;
	combinedKey = "d3a954d1164472694953119419ad8d2102e2672f10831bb621de2fc3cd84ea20";
	iv = "ac61ca687ccadc9b7348a28a77b00a7c";
	cipherText = "a701350309cb70b971875ab9e4a60c18";
	plainText = "468595370e3c9a618019cd7ff17b154c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 30;
	dataLen = 128;
	combinedKey = "d444fb69b112ba9a9e6d73b1ded897434a83c2e9a1b21212007caaccf6b9dab7";
	iv = "fd8ceb7e850e53cce56a5d54fd752072";
	cipherText = "12045d797fc4a9ea222a416788b95f1c";
	plainText = "a88574b9777fe2bafd7743bac1a63131";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 31;
	dataLen = 128;
	combinedKey = "0b10f339a6758a535d201790799a16a9b889982ca25b4ac374d32f6799b9b927";
	iv = "f26d4b1cf2dfe1dab4bb76f059cb9459";
	cipherText = "d2b6939eea068f9ac1d37cea0265010f";
	plainText = "2997d4cf33035912caa2f7207a1cff74";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 32;
	dataLen = 128;
	combinedKey = "d0f3de5da1b72c6a5c1bfd47a94545d26f6b7ddc1d0fbd55bdadc5f9bd57bc43";
	iv = "ed90a9a88118f6bca8b25293143a77c2";
	cipherText = "427c68bb9cc6c20d4df50c3e1058a119";
	plainText = "b4d34be466d105436ba3d499e4dd874e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 33;
	dataLen = 128;
	combinedKey = "be34815a0d7bc117196c4f6e16d326494b19a2d2fd6e429323ec6f3e57216df7";
	iv = "27b23069c37659fffef9547939786ad1";
	cipherText = "c2f42d16998a97544acafcaafc0f86fd";
	plainText = "87cce6d9697f313c6add9507d0b08fd6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 34;
	dataLen = 128;
	combinedKey = "72da47dc206a1a04a2b5436fa05125dd625fbd78d2ffa2302eab5a78dd68cb8f";
	iv = "f2f41b9c98b0275474d24fde3cf16c17";
	cipherText = "1f58e5ac56f1606bc5c57bdb4c794367";
	plainText = "baa2da605e654fdaa6c1ea2e84ca1e28";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 35;
	dataLen = 128;
	combinedKey = "dade39568a19ea35d04e9685f4ef9dace551a580a26b0860dac6362bc1e20173";
	iv = "83934598b495cd7e8500120eb36842af";
	cipherText = "e74f00aa88700d072b294f46b51b55f7";
	plainText = "27dc95c41b52b62b60ae7f3ad9837f83";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 36;
	dataLen = 128;
	combinedKey = "289816605259e4d2043371f23595d41144c2e2bec5ff0d6e8a8d2bf7af47d077";
	iv = "f053091671fd0cb0d48a24901750362a";
	cipherText = "94a997aa94e70be9df27f9875ab7d55b";
	plainText = "be4f3a78c0387e0f7f7224cff5cac586";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 37;
	dataLen = 128;
	combinedKey = "3f5aee18e3d6f0281e3fe764feded959fff36716cec2fded1b49d5df7bd8507c";
	iv = "c13b63afd05a38f115d7d47dadfe8c58";
	cipherText = "f3d2a90bd5fc57085db4dfa1b894864c";
	plainText = "caed6f84bfff99752d58dc8240b13c94";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 38;
	dataLen = 128;
	combinedKey = "779c5c8cac314094b995ab434b1697516cac6c99afca05960a1a113fac813a34";
	iv = "f1ec2c192062f8358ea12eddb462505a";
	cipherText = "571e19a430f61c10d4f9fd2810761b94";
	plainText = "17b9ff76bb867bdf494211f8da770f15";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 39;
	dataLen = 128;
	combinedKey = "cd8f02aaf09be512727fa9848f1df760ebabe2667761b15cb440d7cb9b6c0961";
	iv = "5dd62d0468661796579dcc393dd108a8";
	cipherText = "a1e62d431589251ebd339b17e2dfed53";
	plainText = "82f7e2aac689cd7d3e3ca73a8cd089cb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 40;
	dataLen = 128;
	combinedKey = "3090b6c74698bf121a7b676a0b061cfdd0412baa65c05d9a4330c6cde9d76ac7";
	iv = "19021c7b4a50b3efcbd502782fef9273";
	cipherText = "64b1aa32880b410fe5e0b200b5d9ae7f";
	plainText = "718aefc5e91526ba338583db2a8fcece";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 41;
	dataLen = 128;
	combinedKey = "47bc92da0b28c128011f3b92d1931dbf3e6daeab86339596c894341f3a3ab59d";
	iv = "1a35e0dc5bc48dce4794dd2bc37f4973";
	cipherText = "0c17341c1917b813d80f4282fb31ef57";
	plainText = "7b33fa14b69f901fdc7399f6f006e499";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 42;
	dataLen = 128;
	combinedKey = "a539e990e11adcd6a21c5dd08eacc899208f855c023e29087c9a0d66d034915c";
	iv = "2d7bf84219b24f30c7dc665948d086d8";
	cipherText = "4d5bdf2a20587a087655b56f69eef502";
	plainText = "428e4201021db0991365d59b0bd048ec";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 43;
	dataLen = 128;
	combinedKey = "f95c129c224f4b0943763450e085e25037d1d706d15a1b93a75e820521df1306";
	iv = "5f2d2d1474795ae9f2a47fd239444240";
	cipherText = "24937e2d550fbcd4bef70c8721424e72";
	plainText = "01a035c9299b00a9e66d4c0914023393";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 44;
	dataLen = 128;
	combinedKey = "e9ce4f5f7be9473acf6928b87446dad667ccfbcd02ece4e5d7b20a0be834eea0";
	iv = "777902b66f7955428b7bbc882c3fe8a4";
	cipherText = "f31dde00b2d6ba4bf6d12f756faa5766";
	plainText = "db8c3a584606c5d33fdfb024c42c591e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 45;
	dataLen = 128;
	combinedKey = "422f0f3f92d667b7802e75c7786ae146d11c48f28d3507d40eafac00229f0522";
	iv = "16f75851e1d97b08345c60a9e086f8a6";
	cipherText = "0214c3ef8c1fd34df5823170b8c67d4f";
	plainText = "73a008fe65b5dd987f92dda68a7082fb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 46;
	dataLen = 128;
	combinedKey = "695d8a51cda1c5e71810defd629ddee6988fb60a702b808f21a0cfd8de4e2f71";
	iv = "c2748733475330c17cba921e4d81bc7d";
	cipherText = "23a13d752d416acbb175754c2b502f55";
	plainText = "2d2e1debfb430f10d08e94db522f5246";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 47;
	dataLen = 128;
	combinedKey = "7ad714e4f803ed8d69b0397bba36b2707ba5a025d8363140005a82e36ab5cdb5";
	iv = "7688bcd40307d017f440ba5118634eb2";
	cipherText = "e974900db1d2247f0fed1694739ac366";
	plainText = "52e66dbfab56e558da5a6a1adc117a6b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 48;
	dataLen = 128;
	combinedKey = "39cb565cefac3b5f27fe7dfe36555b85c348818d4aeae44645da14a0b9778320";
	iv = "dc96aae9a600c879ed0a5f4aa831cffe";
	cipherText = "088f88ee94479fd4b40c4171b6f39b87";
	plainText = "e08679e2d371712bc61a68d7ea1651b2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 49;
	dataLen = 128;
	combinedKey = "a4ccbfe4398ea36e218c5a04d7312c1cb5b8757932294c32ebb7f3c524adec04";
	iv = "699d2f5540b6c4693de9dd60d6839954";
	cipherText = "a6510aeeead61d4106f88b8645ea01db";
	plainText = "0828b712b1bfd1e3cda1a6427ac57282";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 50;
	dataLen = 128;
	combinedKey = "b2ff2a8dd76bb9e3f85ec9ce15379ece613b7b380141a7983b702117db0a9ee7";
	iv = "66ecc01f4ab114a77b63f974f2e218ac";
	cipherText = "1cb867c7e3df68e51e10ace08dae8bd0";
	plainText = "a1b57bd3f6559943b77fd5e04b83aa73";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 51;
	dataLen = 128;
	combinedKey = "07407688b0894fbb57e4f6133a0f544413db23944ede511e0c5ffea3c6264b03";
	iv = "abb64e0f1f392d639e2a7e9843a67043";
	cipherText = "aa6f7add7479a6638f5a0b412ddac2dc";
	plainText = "b4afda7933c65d44993a4a8bc6b889e2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 52;
	dataLen = 128;
	combinedKey = "ad1f6d7e6667066ccf32f74a2317c46d9f2b94baf71c7de999be843f43e76003";
	iv = "bf747d6f35d84770dffb6df105b084da";
	cipherText = "62137aa82029ab13505720b2ca712cb3";
	plainText = "5eef540cb1067c83dedc934d2bfe9668";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 53;
	dataLen = 128;
	combinedKey = "ce50527e35a9feddc4d202f95a1f800df18e13121a0fa4eaa921ad0f0032c875";
	iv = "83c33d56003726902c58dff4c579bb47";
	cipherText = "1967a1bd9fa4811792ab782e79807cf6";
	plainText = "403617d9bb3029dfa6fb924f28ead722";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 54;
	dataLen = 128;
	combinedKey = "78d0f928fa4061248eb473bdd1b5810246e3146ca638a06fde9ccf5c1cf3025b";
	iv = "c0b29cd87d346dbcecb0b83dd90449d0";
	cipherText = "5f985799a7f831cd180cd56be997f9eb";
	plainText = "f9702364be8da2238b02ce3abb595360";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 55;
	dataLen = 128;
	combinedKey = "a2b649ac8df362d187584107fd8ad6fd5dc54a2e7675e2de717be9ab70db8bc7";
	iv = "8eb5ce9536e9f3d547a35f7932a881b0";
	cipherText = "8b9204d9be5a04d38266eed278942935";
	plainText = "d351901195097b5e35bca09fed5ff666";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 56;
	dataLen = 128;
	combinedKey = "f5f7a89f8b5b6b3f7b85917dca85b248680d0b5a5cfcf1693105f621feb9d990";
	iv = "83e98330f2fd59a2f76948f7f09436f0";
	cipherText = "a6e46c331b751ceacbefafd5341e4a37";
	plainText = "ec67cff4031f2b8f91df44bd09898146";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 57;
	dataLen = 128;
	combinedKey = "b39e95b92f75f516428c9b0618d03725db69cae30ad7f43c98f7f9b334e8d4ce";
	iv = "1e47843de1d435b1fa7da097c574e41d";
	cipherText = "c9b7f91b723a451fbaa7af189a692904";
	plainText = "483fe6f6d82b55b0574c61308dd91627";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 58;
	dataLen = 128;
	combinedKey = "ccf2c4ab1753af90fc4c0b49efbd909eb00435f976d91ea9e8c9cc41ecb7c95d";
	iv = "cc864630724c9ea53fd8fd21e32e4045";
	cipherText = "5b3701b42f9f5c37a66b34c731bb0f13";
	plainText = "2599a9f13f429859dd415e04bbdb2e47";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 59;
	dataLen = 128;
	combinedKey = "ac3c8c186e914bf426ae3f3ddd4d425528aaf7deef57d382b05a5e37e334d901";
	iv = "78df6bcfdab6c169b7f3bca73c43bbdb";
	cipherText = "72c352a3060610bc884e2ea8a69d6162";
	plainText = "930ff6420ce6031df8d9e9043cbafe6f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 60;
	dataLen = 128;
	combinedKey = "9fe3d5b57ca90b2b209d215e44c677047ed02663e90aaf0a1026db3dd3c16f78";
	iv = "d92eacd547bbd808704d67e47fe2204c";
	cipherText = "e56562ecc68b0ab98581186eb51fb77d";
	plainText = "486cdf22464cd58f34e9d24f36032244";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 61;
	dataLen = 128;
	combinedKey = "116093256eaffddf72e92d1293fbb5390f58d02053186a4f23761dfe7fd01d7d";
	iv = "e0609c4fa1b035eb69abec0bd822c045";
	cipherText = "810d9b7e8dfbf48ca8eb2a87eaa6fb89";
	plainText = "4e155f4e0deca493224c144821d4b878";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 62;
	dataLen = 128;
	combinedKey = "15d890b1e263ddf514e211668f950d0035f02b4e0185579990e262f7fffa52e6";
	iv = "19ccde1d36e8b1ff92269e1df0a99496";
	cipherText = "1f80e1a6fb096632451a2ad0f2acc375";
	plainText = "37296689686ea00e5b9db21ec4e6dc93";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 63;
	dataLen = 128;
	combinedKey = "18b823984c34f6d5d472b6bd0b095890ae748e76228c2d702d53153927baa483";
	iv = "729790be1437e8d7ca9f71c93eb02092";
	cipherText = "65da064759d7b0cd102a2f36989d09e7";
	plainText = "50454afb87eb96488165a4bbd571d6d7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 64;
	dataLen = 128;
	combinedKey = "95db57d4bf1af7d635063ba2c456f6fcfea5400472cafedd49f0af506e41fdfc";
	iv = "26edc9a8eae428362c2f18b52673714b";
	cipherText = "cce884e6490c57da20cef7e7a1bd5a7e";
	plainText = "3dd067a8c4ada2f824cd2c9dfa6e67d0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 65;
	dataLen = 128;
	combinedKey = "4af83a212d52b39ff2db5286e4432a7a3bf71a980cf56baa0389db28695e3b16";
	iv = "72e493e04e77577d0ee398db761916d3";
	cipherText = "9e09f65d8ec358e8a8b75e255cf62d49";
	plainText = "8a001d33994022a0a8e4290153633ca6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 66;
	dataLen = 128;
	combinedKey = "8a810bf2bfa8d5070b3bc19a447422c0ef5a0ef2e27a77738465c6a5fae988ec";
	iv = "6dccf50f9dc807f12b7da0f2c43bbe8c";
	cipherText = "643cf9de685dff5d92296b91bc396cf3";
	plainText = "54777fb0b1dc7b12cda63b06d784d951";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 67;
	dataLen = 128;
	combinedKey = "b376761bc422a2ec3f106df07f1b007453cfc5258b07e3942bd618572a1bc42b";
	iv = "5c9ce9c0a9e0a54c9ec930001c52556e";
	cipherText = "363d29ce2e61aaa4077ab74b41cd71a7";
	plainText = "62f2207c8049ee99393490d7350b729d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 68;
	dataLen = 128;
	combinedKey = "ab3a5e94df11a7fe65a50a414514a8614030016cdaff7961f10daa03222fd75a";
	iv = "1b42dd979adaa545898eb00f196bbbe5";
	cipherText = "a74ba43bbb4a052f5eba62f11617d8bd";
	plainText = "340b6331be6238aeb5c092447859102f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 69;
	dataLen = 128;
	combinedKey = "edc7c4169a190cda3187e99ec9e913946ce7782ff550c564d28149e6da9940d5";
	iv = "4897bd8eeaf6f62f0b46361f6aabcc60";
	cipherText = "530a0da670fced3341b78b76f3373fc1";
	plainText = "31e68f28fb2d2f43c7a733f89a17a490";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 70;
	dataLen = 128;
	combinedKey = "90455661fda6d7164251c372962e4148affac9ba7076ce3777d0b0d542f81a51";
	iv = "1e18f988165866cbb9814d4eab56079d";
	cipherText = "e75584cd94375bd934bce3887c174946";
	plainText = "8df1052cb864fab3b3f55376a459628e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 71;
	dataLen = 128;
	combinedKey = "f0cce74b29bfa4a532ea78da2f70783a602c5af58ec3bd5eb4a4bbe155a6e57d";
	iv = "f2de5a37308f7b3c07e5f3e8237fb023";
	cipherText = "98c96ef0c3dc73ee0aaf8e7ffbbe7188";
	plainText = "87064feb9e2b4e949e89ad8ccd778862";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 72;
	dataLen = 128;
	combinedKey = "ed5985d52b5297c38d81e85b961a14b6eeecdc1340430bdb84735ca802ba5537";
	iv = "a104457b56cd7daa5389a0a37095c83c";
	cipherText = "9e8b72cefab7018100dda4bffc589dc5";
	plainText = "63230be7295776a99d592bb8f252f798";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 73;
	dataLen = 128;
	combinedKey = "bdcd0f9f187b43e5c59c501dadb6e2adac96707f40c926c0fbcc43742efc9cfd";
	iv = "8e908b9744b936d8078a3d50363c418b";
	cipherText = "bd3f5fc86276eef45f93fe1464de401f";
	plainText = "9f908501d677ae4b4ca591fb90d4411f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 74;
	dataLen = 128;
	combinedKey = "5239ce8c50df13b7ef0d8aae48703d014e2aa34258b7c9815af4dc8e57b2f6f7";
	iv = "64657a3c769175edb20cef05b3dc8260";
	cipherText = "88f0712d95f814ea843af958dfbc10ce";
	plainText = "e52e1be04b55d503f1f50cbc9ee364d7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 75;
	dataLen = 128;
	combinedKey = "c5a5e35a745ab49f3dbe31d95959e23d580a98c487b0f643c03b626598fcbc33";
	iv = "00827f90054938a80059e94c7dba693d";
	cipherText = "7f82603b8ad9fe898e97be65d8de2e97";
	plainText = "3fec7027ee8314d9fb387a5b0eaa684b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 76;
	dataLen = 128;
	combinedKey = "60ce7d7e1239b107a2d3b09c1ecae96215f515fddb2179e6d73fa9a6d61d8978";
	iv = "f5f5198c95711cd40dff00ce2d3dff0f";
	cipherText = "936bc3a8f638cf427e99dba3b3089898";
	plainText = "50518b3c78731ae3ece9a4b67a9fc13c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 77;
	dataLen = 128;
	combinedKey = "8bef72e621e79d8e3b476b2ac45b28ebb1516c1177e24317794f546205f629f7";
	iv = "520b4ee6e6dddaf48ec9ca274aa3a72d";
	cipherText = "c68d2d7413c2edbacd89290ccd48ffd3";
	plainText = "e29cf1e2de3bed6ccb267e02706e1d64";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 78;
	dataLen = 128;
	combinedKey = "707e80e2145b13a6d322c65201c511f3018567ad4bb7d0ca5e368923cb54db92";
	iv = "a7cfba407b0f4a58361f8987c0fec237";
	cipherText = "09730b060a73fef90d512ceaaa3e186d";
	plainText = "3f6a6ebc9fcfd3dd69d20e8faa433cb4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 79;
	dataLen = 128;
	combinedKey = "d5d18fa371ed56b0f9159274f6de72d2304b550305bd64cdcb2db85d0d3aa5a0";
	iv = "5c7d3b88016978254dcfd967d8c16bc2";
	cipherText = "68d968da85777cb9914594a221611e1f";
	plainText = "d4886867469d8c3e9f66f72495176da8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 80;
	dataLen = 128;
	combinedKey = "926295685037b1f7fa47013e041cc98feff8dbfa63f18c8c3a49c8ffe4ce5735";
	iv = "491ed094ca27d0b4f70754d4c10cb0c4";
	cipherText = "e6b1825c11694572b271f3c01b2a4812";
	plainText = "db6b092ad9b23463d8686c9342903b86";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 81;
	dataLen = 128;
	combinedKey = "ee3f40783897dddaddb5b5ebcc184069e38604cec88e0cc613c8724765c0d366";
	iv = "40b7e4ec3bc0f36523049aff9f58dd05";
	cipherText = "9b7b2a66cb40ded274c6c398efa577bb";
	plainText = "0e6507d5590b378993a448afbf6849b7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 82;
	dataLen = 128;
	combinedKey = "837cbdaace81c355ce81d957886d301a5425c1ec7df0da9606f489b3ee532857";
	iv = "0801ef7c707b6312502c4b0d8e6e1c48";
	cipherText = "a0bf81af49dc31adadebe60086f2117e";
	plainText = "64dae7510603bdd1059d35315984b5e6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 83;
	dataLen = 128;
	combinedKey = "9ab889c05e97c6c15c332714f3bfe000cbb8c7f999e0e6311f68cf337af67575";
	iv = "03d1df6a0baf35303cc9599b7d2535f2";
	cipherText = "bc229b99714591d609cc525ba5efc6f6";
	plainText = "21229ac8367b9e10f202ad155c0f24bd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 84;
	dataLen = 128;
	combinedKey = "17ef2b398f989c88972f0011c8ef105be025d9e0a07051bb0fcd985a6f12eec5";
	iv = "8320c6b025e661fcafe4b96142890919";
	cipherText = "6fd9b1486b6bfd3d7a56e046472aa548";
	plainText = "53f4960234ac1aee807f893ed142147b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 85;
	dataLen = 128;
	combinedKey = "ce74653c9d99c3d7d8c9a25a4a1b2ad1412de15d318ff7030115b7c242b92b87";
	iv = "b9d7f96d12fa429387f709d5cbefa3f5";
	cipherText = "23d5cd9fd34b9a2f1f04d04f5907e804";
	plainText = "0b7c6e9019521f72896678e7f503bbde";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 86;
	dataLen = 128;
	combinedKey = "9393e8126bf9a9f4a20d3f6d891d0492cc46a7882ea256ed7d2257493ed24306";
	iv = "38f3ac5a5af67a6dde0555bed2157225";
	cipherText = "056762073f5f62cd076b58d6a62bab63";
	plainText = "d015790a0daffceaf0e974f0f5c7d1fa";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 87;
	dataLen = 128;
	combinedKey = "a6b4d5b49976712b9cf5b9f3aa85263686e5d0f310ec7cc0b62faee6f5dc08f6";
	iv = "5d509f6817b3b0b420435a40441362ac";
	cipherText = "845a86b65591e0593903e3054ffb5eeb";
	plainText = "d7660e8c0a9f0f6a8346de8a1b73cb3c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 88;
	dataLen = 128;
	combinedKey = "f5a57b827619af7397227c3c5e266a389cab4fe7d7970bec095fe7a1b5bb3676";
	iv = "53e15b956f9e0ad1512475d819107ea6";
	cipherText = "2ad312a2db73ddf91501c20f6b86d94c";
	plainText = "4e407432e306e7b4b53a02fa37446fca";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 89;
	dataLen = 128;
	combinedKey = "8e4b66096ad03e9b5e17b65822011e002add4fc8de1031ea48dd68f6ee7e6f7f";
	iv = "9c8bdb6510d8feb5a7046b64a21a481e";
	cipherText = "c1708a8d0c18d7dd94a54c6a71cccff0";
	plainText = "3a74e62643e95255895e76c505691e0f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 90;
	dataLen = 128;
	combinedKey = "4e13a3de74f41874e7beddddc4f6339c8b204145fa9edc1e7d639686e9cd1218";
	iv = "a3cc068402b003dfee82f658d6f49399";
	cipherText = "810b6b690e23649f6ca8095bd2eeef4a";
	plainText = "5b67176d29120c61ae3c2ccfb682b02c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 91;
	dataLen = 128;
	combinedKey = "b7b719d5b12562b67d36f9b45de3c0eba05f674997db3a0af036613567a0e93d";
	iv = "9ea4040c1d0a1f4ff921d2cf3b5e8d4b";
	cipherText = "878e1331d9724322fa2980ba3fa1a706";
	plainText = "eec4333c9ef031c472fd9a74c0517ab4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 92;
	dataLen = 128;
	combinedKey = "c9f3f1c116698fdfe66eb867595d42b870b86454e9e5c514f47ba5e235d12cd8";
	iv = "ea80f2a55b3c289f2e730f1212515bfb";
	cipherText = "41919d26f63fd1d9e5ad44510b2affe9";
	plainText = "e1e7c54f748398d9a7070bf3b2f6a585";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 93;
	dataLen = 128;
	combinedKey = "be91758b90f19f1298430be8b1c8e7d87332469d8d94192e048d8da08931ce3a";
	iv = "8a3d0af9e42028fe45d8c22787d5a5f7";
	cipherText = "46507cb9d0bb90848335d10596269499";
	plainText = "133cf57142dc3d028111c4e19f2a8225";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 94;
	dataLen = 128;
	combinedKey = "c07d46ce9837c87f4a5c35a2585ed442d4092b4699736ee6e7eeb88f254ad381";
	iv = "774efc622f14f35fe6a14df73316f08c";
	cipherText = "909003d635a0800d3b18803810aca297";
	plainText = "4d357b71dd64b4c8d945b929f42e436b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 95;
	dataLen = 128;
	combinedKey = "9635bbd2abb86916bea1f0e52569f4f486ee562b4061b6a4b88d411cca9dfe84";
	iv = "c3b7f447f4fe6181b69ef32879d6638e";
	cipherText = "1f647b4e5a9386551d699ba024a22bf3";
	plainText = "134dc27ccc4f0ad79bed414ec902da86";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 96;
	dataLen = 128;
	combinedKey = "f2fc215b303ce9f49121177349795f5723828275bcabfb845148df1209effde3";
	iv = "9caf2e2d64053cee1df320ac70760eb0";
	cipherText = "6538b24da02f1bcd027fe4e32a821090";
	plainText = "bc7bbd41ddf1c681e4fca176dc874e76";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 97;
	dataLen = 128;
	combinedKey = "c717d145577e5c96e590a80f6870d00d5fbf8e082bf4e038f6fadce224caad81";
	iv = "0e0d6fc827512f47ce0415037f20a1fb";
	cipherText = "9ee999759d25ed4760fa113be0f2008b";
	plainText = "9b43f6273d433e24a47c78584168ff1a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 98;
	dataLen = 128;
	combinedKey = "1aa4e0abfaa28b68e67dbfb18fdb0153fd87aac75af632188d096916649dc592";
	iv = "56edcdd4d7c62beac285ff2c5b446a0d";
	cipherText = "99f9c5cc9b2db0a4de8a2ac13b21036c";
	plainText = "ec7363f3e14e8850b847f0d28c6af927";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 99;
	dataLen = 128;
	combinedKey = "33cc9daa91b2dc6ad46b1e02cbbb8851cc9b258007ca92db95922867f72219a3";
	iv = "11bfdaca0731799f6d96bf5c78e9ae34";
	cipherText = "e557d62ed9d4c764e28149a1d2be3577";
	plainText = "f32db1266c7401021aa2b826fa285d57";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 100;
	dataLen = 128;
	combinedKey = "5c063de0125df906164602d5d3d937bfc9e01efdc93357759d2376813f883d27";
	iv = "d468c22012801dcdd7c6484262988667";
	cipherText = "2b02e10caa57c7b03701be85d72c24dc";
	plainText = "cfd1109c43f6b3f1b3cd227f33f9c94d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 101;
	dataLen = 256;
	combinedKey = "db882f33c5048b302e92287fead0818a8fb76be6a99b4167e76ade1ed40c1d45";
	iv = "3479c6234db9abb731f8a80bee2bc62c";
	cipherText = "6d15522b64d3b74dbcb331e7aeaed912e2e9b44deb40bda33e21dbf7e66f3e51";
	plainText = "672b8aa47270df3bdc0ad6440e5fef31a8d296f16c01436302cc2cb3b123ed3b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 102;
	dataLen = 256;
	combinedKey = "3629c6634388091cdd60236e7506e757087f29f7e81247b073f4903495a4d671";
	iv = "073ad8369554092426c7c23657f7d780";
	cipherText = "a70e4ff0dfe2ec64473c45e3b916a6354c9d22cfef7ef7adff846c204a099fa0";
	plainText = "cf479bd77e5dcc72f91e92d61ad7876b7da3128b5f92c3f6aa4fd05dd70699cb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 103;
	dataLen = 256;
	combinedKey = "cb67aea9706482be9796e277498dc06129728cce5848dc195f607a35087d74cd";
	iv = "c5db4b647d4be26b35e3eff875db02eb";
	cipherText = "943e9b8ddd3c5b853b84f5990d0a709eabd29a9696d3919528cbb63d33033cf9";
	plainText = "ddf9a4e41ce58acbd1f7b7cbd1829c20f2d4f53b5ccf61c36acdba19a9a7de6f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 104;
	dataLen = 256;
	combinedKey = "abd8d249b6b4342489e161b91d37900cbef30a2b18ca1099d8f5ff76e60b5d87";
	iv = "ca00c3e00bdd618cc7440b7baf77c41f";
	cipherText = "3edb5b1bcb28db01f63ac368e0b2441a13ccaeeedc1b9db8179efd5869ab2e07";
	plainText = "dd602caf80f6137d7e31e263c084a0b5a0ddac60d9e8f8d6272d2db8d04305b3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 105;
	dataLen = 256;
	combinedKey = "314a38ec7a1311569f8b85ce5d76cf60803d54cc45a3dd92c526f331ce3fe08a";
	iv = "699fbb4c46977d86aeb770daac261e0c";
	cipherText = "ba6b212cefd95f10ae11cf8d01411b200f50d2ff2a6ea1d3dcd2b0733aa1614d";
	plainText = "1e5c06cd7eca14f7849ccb27b0553d2fc9bb922e0af2f9aa4cd94ad8a6bac0fd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 106;
	dataLen = 256;
	combinedKey = "af4b176ea92a4f1a28a6ba2ce522a2285afbc18ab722b62d58b208bd5aff3c27";
	iv = "971e3f13f6f6530cbfea748311d23972";
	cipherText = "2184419611074babd53a6ba28833c2e2e6e75595218bf0d11245fbe79d3cd266";
	plainText = "092be7eb5537378df439addeb147f6f6ecb0166e9f0aa0b885c7d61efbd32cea";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 107;
	dataLen = 256;
	combinedKey = "27a8892e33dcab8c29ee1899343a474c5cf1a7c535a5db210c44363c99b9d417";
	iv = "5a8505c9aa586f82f14985338280a411";
	cipherText = "12fbc52bf47e679f0b0e2c58043f0e265b4cdd3ab2f3c0425ae4d9bf2534bcc9";
	plainText = "f74e985eb1279af18bcc577f18c37d2404c2676bc0060763ff7d4046eab3e1a3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 108;
	dataLen = 256;
	combinedKey = "32c5bea7c180a41d92226610d8ce196445e358affebd5a3606bd4740875b450f";
	iv = "24f65b889b20fdc48a84302f95423b1b";
	cipherText = "792c35ef9e41ea19cd495d9096f6b6f386ab4dc5a4bcd4d71e37757a5690c7c0";
	plainText = "89e694d966a217d3d8e7dc57dda1ba141124c417186c815b6ff79cbdf7582c71";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 109;
	dataLen = 256;
	combinedKey = "e0400fed086464f3062ac911f8f986532e7d7ce18f56ba75924a3e59cbc1738b";
	iv = "1af5c26fdc4c172c07d5d63e6a1d40ed";
	cipherText = "10c4a5f189d08d57e621fe12d28b45893833214cdb712421f9e09e2233abe6a6";
	plainText = "b2e6d5a0816ce6bf297eb200b6132e28de12382a9b6fa4efe373fea7edec6c34";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 110;
	dataLen = 256;
	combinedKey = "fe2cc783829bd7a4af7ab774a106230f4ce9db1c553580ea8ade8d7261e035d8";
	iv = "ae23cc467889745817fc1a13badb7377";
	cipherText = "a1f673c3976391208df5a074bbe9876a1eeac10800eb5e2980594810318416f9";
	plainText = "020f6103e868a52e9fac6c33d488eef4d0c8ae4a3c980a83f8b93dee79d6f7e8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 111;
	dataLen = 256;
	combinedKey = "1bf2e5a326b5e410e59743ae2cc15415ccc42353fe25a8542ee00457f1e36697";
	iv = "e4b535f80022a8cd732baa923210dd3f";
	cipherText = "cddc2b03eef6866e9dc0168bcbd3f5807c6cfbbf31a481e349f49245e307ae97";
	plainText = "4dc2335c609d9edcfd37c503b53c874a18f7f3879304e026dc5eaaba4e0ef108";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 112;
	dataLen = 256;
	combinedKey = "70b2b1c785b3ab029d77befc615702dbaf235d693e54f067475b8e759a0483ff";
	iv = "ce1a0788291ed60b6f8084948f0d1533";
	cipherText = "ae8329ef0ac3dd8045f0c93a4adcb11a5248aaf18e288c04245fad29aada11f7";
	plainText = "bffac2c1c4fccb6e809013b56b0c55425590329901c2c0a8731d22f4907d60d9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 113;
	dataLen = 256;
	combinedKey = "1f1b9159f4085e981879d7b3902a98584857eadec887b08b252a3a26a3f6c79a";
	iv = "86bba567c2c1be5d310f439fdb932a12";
	cipherText = "01abb5fd533e769a471ec3fe0ea34ad34d6e12923842e57a8d3344456b231430";
	plainText = "a4817421d0abf19adee445be95586e3c15deeeff7114aa3ad2136ad40e47b29f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 114;
	dataLen = 256;
	combinedKey = "c7da49edddd2b79757f04ed70e751b8ca96af2f60d0c85abaf49dc298f2092b6";
	iv = "c2c0554b5982b54b3d3ea872bf3ce0d1";
	cipherText = "f3a3ff8a227bc50ca0e5c7f47bb405a008392b32eee1acc310e677c2a7f44eba";
	plainText = "f6d478b350e3445911c21b329c24bc11760da32f27eb2d3f39df9ad7062f4940";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 115;
	dataLen = 256;
	combinedKey = "f72e936f1d768d48bd889f82dfa4305ceb3a08ad1e0e417875942f5bf3afb613";
	iv = "885e119374b14c67753650383088b263";
	cipherText = "dbd78c7a8fc60d3d692e3bbd8ca4dab72631530778528864e52c70f71704d0b7";
	plainText = "2a27a8639e4b312eb30c2ee06572307b31981188970f57b6c34294f9229d80ea";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 116;
	dataLen = 256;
	combinedKey = "28f2e8188e6bd6ad9770282e9433596d2beb6a64a543f9e1d07b257482a84519";
	iv = "0386008a4b89a58baa8f2aa39c9d2295";
	cipherText = "e4a44b86765bf0e283b4c20b3013b18f3d9ea60982c196d25a7505c7cb11a8e6";
	plainText = "634abdf141f7b33e9b941fd50a6e4844c93f7d2b2b8c09e6cb4d0b7cfd2c038f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 117;
	dataLen = 256;
	combinedKey = "4cf969387e44a3eaa675112c3488566465efb41f0afe1a2a56569a0cef14b20d";
	iv = "75627c66582cc06c1126cd5abc04b445";
	cipherText = "be952ffb57694a454a2f42b5be880d38473e2c91f63df864d29eef3d776ea390";
	plainText = "d1e751c91151f35f69fccdfd06493e18d49a94bb7c8d02414a10f0347442b1f7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 118;
	dataLen = 256;
	combinedKey = "e6189e85075b5455f3b8e5ff51bfcb34dd3ebc464a071bff44fc3a076e134885";
	iv = "4de19bd56257d382c8669ffe68b03484";
	cipherText = "6c62434f4453d1b8cb78d4e5a78a348662b16bf20ac6677b08bd4629b3734786";
	plainText = "6d944f49bf0167c511a02f4a3df80d2f97f4e3ddce56f51026e8b224658e3ed5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 119;
	dataLen = 256;
	combinedKey = "47f8b323fad90a4b6e52bd3d2b0e4e73bee4c1f60e6a6f94139f2f91fde01806";
	iv = "de88056268f85a764965ea608e8e32f2";
	cipherText = "6d9ae349c620c0d57e46c850c8dbb70d47420f77eec23cadff00e6703d90f62c";
	plainText = "6462144ced42f21195c58d524ba2c8b87c3b1d4caae688bb1529107029fafbdf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 120;
	dataLen = 256;
	combinedKey = "5fca4c210463b2e89fd7f9b7ffc46b1bb055d80fef823b06577b438ebe810def";
	iv = "dab87e267f45c06aca437391e3ef2b93";
	cipherText = "9010ea0db832e50761a68923ac7504b41ac99a23b5b33dc67162c7f5c1576a99";
	plainText = "7d8676a704699254f0bbe4c5a1a725d5fb01dbadab25fd997e2a2fe646fd5552";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 121;
	dataLen = 256;
	combinedKey = "efb54f29334e4ec475d7a92eea05b5e7c604bfca9d3a9c4eb731b3ae05eb7936";
	iv = "4965e8e50879c8ed10a43b347150747b";
	cipherText = "950746bba1b484dd928e23df3fe73a6a62f48b673862ace994a60306c3cb81d2";
	plainText = "2d1d5701394407c4a3a49580e2e1cea79c96a3e0b2b41e16c87fc3ed142ee973";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 122;
	dataLen = 256;
	combinedKey = "dc5b03495daf20bedf1f5e0d89a930f67b99605ea62d88611d6f6c1e015ff5bd";
	iv = "eaccaaf4a82550e669ec5279de391eff";
	cipherText = "8bfa700c1c5d7f69022de28763f955c2087a48689e9b5c83d0ca5a43354bd8d5";
	plainText = "0d4fc92a52952cbd4872ffe964f0ea6bf10b8adcb95e21bc85e2e40dc139d360";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 123;
	dataLen = 256;
	combinedKey = "27a57a419286e0b36fecf477597340c9d422e95934f52840e061dfcf8add5f9a";
	iv = "acf25408e9dd21a8a0a26f2edef2ab76";
	cipherText = "dc5d2261a9fd089a013a86babc20e2694dea999f75c2edf26c15acc061b35cda";
	plainText = "1f7dce6c5ac9c726dd0fa4fdcd71a47ef3e008aacefe7656aeebf85aa0094487";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 124;
	dataLen = 256;
	combinedKey = "80d6d26bbc08094a5d51483d25c4cca63d42ac49ebb93b5e2a6934c054081e39";
	iv = "70868d1d4dca4e4ce94647ac4dcc3e4d";
	cipherText = "04c77b0e67f3e5b5436d4e43d50909bb557ec6dfe9469258b4cb08e4aab2c882";
	plainText = "77337683681539ea9f7934f81ea555902862f0ae3ff4d9f20bf4c510db33d47c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 125;
	dataLen = 256;
	combinedKey = "0de30916d9fb46126f1fbb9bc8019fe5c8a2635f29c8c3ea5522f387ec0cf6e6";
	iv = "2cecbd08dff325874c1cf6cb4a4fe7b1";
	cipherText = "009a7788092a4d66b25ff9b036a74e7b43be22f60e316cc917888bc557a046ac";
	plainText = "03349e81523e4cd7b89825ff448cc047ad1eef5754e0d2af4d919084ca430d2c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 126;
	dataLen = 256;
	combinedKey = "505ce85fbee4e124641e56ef000ecdd4c2b42b1e8fa3dc57b01196b4dfbc95cd";
	iv = "969351c5f5900f54db1f8cfa134ede74";
	cipherText = "7c081dee4ff33f09b5a5e0dd682247ea80fa8e4ee1657917b5fe881c4538b41e";
	plainText = "7d7046654800fe8a0868394173abf29d6b2a692493006440d69ea92c3a2ed5e5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 127;
	dataLen = 256;
	combinedKey = "e1aea2f0f54c1f7e9302c15cfc69cd079d051087cae36b5a6e845b08bc5f6ba5";
	iv = "68f89e9f7dbe50c799324973930782ff";
	cipherText = "6654b4fce012422229cc4019de5be0b49f52614c979e2ab176d8fd1bf01bfaf3";
	plainText = "77a5da15c3379ea2ec857cb9c0e61198f0ae368c0739397f6295fa6321464433";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 128;
	dataLen = 256;
	combinedKey = "947d285340ea36493b19e1ea31d2bb4179659f16e5c1372f3f7249e881bd1aa2";
	iv = "02356d80f96dd0a824c8fc7724eaf673";
	cipherText = "1399dae7f93ce94484c83da87f383d43416dee3d54f9cb5e492b51d4beca110f";
	plainText = "6c118a6ad20f75f2fa642da939ce5e8dbc66c4407d8b094a01f8fa3558062f4e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 129;
	dataLen = 256;
	combinedKey = "3aac43a0de3f82bd0d8dd6b883b004ca7a397dadece44ca5be84d55ca63f16e5";
	iv = "d217aa37653de9ed3e2504786b6ce063";
	cipherText = "85a6c3de3001d33d3ccfe5e18debf6dbc40fba5e95d72e6fd9835075aade7ac1";
	plainText = "ec6876bec25bcf819a038257546a1c214d1e4259986287b510900939bdec083e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 130;
	dataLen = 256;
	combinedKey = "dbef2d5d802ceb83e686e0da778f2912af7265699e09b8fa545e4b817dc4cb98";
	iv = "22870871373ad5103d7c38c16a31eb45";
	cipherText = "b4009fbf8d412776eff67745619e6e3770c4c6c2f12413999b39a4b6a51c8d97";
	plainText = "306640e0177e42f897488ece20eac2859cd6cd4f71df312221231559bf5d5f86";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 131;
	dataLen = 256;
	combinedKey = "6553c29b5269597f55ddf999eeb9bdae696833b19dcfe7e6921ee6e51ec598df";
	iv = "3f421d41d9c8b8d5535e134a6f76125e";
	cipherText = "5d1d7b4cb00b07485b674bb0714ddff95b17a0143de25bd96ab6cb99a338a333";
	plainText = "5420619fad9c86a08f2c3816d034215e2dbf6a1538a61d8f9306da57fa0e8666";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 132;
	dataLen = 256;
	combinedKey = "b83b7f41d016873342aaa19704dae3670bc6762fca3ba2f3b2ac0e65df7dde79";
	iv = "59e85a50fbf198f7b608ede83363b418";
	cipherText = "fef7d8cfba8309823c8b1adc75ccb1002a651eaf0a18ec3998ba73f826380b17";
	plainText = "44a951f87ffd7a41368e71dfc7292fc28ee01ae40c0a2e199d0af73b57170627";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 133;
	dataLen = 256;
	combinedKey = "ac15ac23f8bf8fc9dee9f31ad4e2da126505f951eef973330cf0bc53ee8193b3";
	iv = "316a7b2cba81d0185be9412a1a2a3c0f";
	cipherText = "a284b8a9e3961bf602e647669743595d51541502fa4da833cdadc3a17d2144b0";
	plainText = "1910af89b0870a9179b1d9561de2bb16d30752187e002088dd1038d6a0f32c9e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 134;
	dataLen = 256;
	combinedKey = "b4bc88d80bcf2cf7f2ad08b5b1ac4d959c6c9894514f0b374a34d4c58f088b86";
	iv = "f5e999b76c4ea71d749622857afacba5";
	cipherText = "0680487ac3489e791e7813ed4bfe9ac65e436591522e824a495ba4d9a52845da";
	plainText = "d542862356cf95898da6216bdd6b8585714294144aac9825ad30416f261d925c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 135;
	dataLen = 256;
	combinedKey = "b077d5e1ef684c36937bcf7d79674ce247b1af7fd304cd4a64c9ddd892e62793";
	iv = "57b704d6cabf2e79deb8073d17712ab4";
	cipherText = "24e253f3113b7c193f8ef49b1b2441bba2f7c85c3db069a40d5dbb8ceefe466b";
	plainText = "22db96814e23e7a49c095b78036ae58de23c1912b41b10013451d24a29ec62c7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 136;
	dataLen = 256;
	combinedKey = "367bb476e69f2631ecc6bcec6e38f84d2de363fa86fa12a6aab286f4527dec67";
	iv = "dc92d0f74b809814f042b6d834fbe421";
	cipherText = "158046203a217f588d43efe8c0ac11f432722d91025fcaccf6bad57d41a02f9b";
	plainText = "99af3e28a84fe8dec69c1d2d44735779415b2dd9bc8324bc57f8f22379382c7d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 137;
	dataLen = 256;
	combinedKey = "12bce06529ccdf05d06e55940c1992100b0eaf8d80eac4aa2baeb4316f7c2cbd";
	iv = "b8339fba52ea6f0b3c8be9f7238b365f";
	cipherText = "189f15142560298b4d0597b5593ab5d1e244cf09b81018658133340204eb40bf";
	plainText = "8f28d2857173ef3e7684ff5da94a87239da4a947058247be24baacfd8af94df1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 138;
	dataLen = 256;
	combinedKey = "095e2a9a98e650e328c71ee67e6cec432cdde51c5c9131022a1e97ceae06e92b";
	iv = "19cd7102ae9665c82919b23348a405fd";
	cipherText = "8b4738d042c14fb77ad7b8b6e7da16febdd49f9fcd0f46e62ebfcd807198c864";
	plainText = "5807ecb939ec83fd4fba0093e0a34adb46482da3de537dcef1c18fe06ba96966";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 139;
	dataLen = 256;
	combinedKey = "181da4bd34c89e8a9fa19f412bfc8986fedd27abe2095d1e6592647b5c9f68df";
	iv = "b09f9c79c462d3bd99b166a0e3eddfed";
	cipherText = "c521859c4d9eab0ce19f2ad50f6d20c031043490fcdd3a8f4bb0146fdeecac98";
	plainText = "deee66ffd3e61e660b9502815e450d46087a27d222dea43759e2075ccbfb2d2b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 140;
	dataLen = 256;
	combinedKey = "90377582570833b2230f3800b306da21c6185632bd74892505c49516c07122d8";
	iv = "4eb8293c12137ecbd3fabd6f09403b88";
	cipherText = "f70e49e5875024dd0d8b1b2c2ceb504691ddfe807b1280e589d18087fc436efe";
	plainText = "078668f44d7f852b634948d73c77842bd8ff53d68972daf3f8cec766544b6187";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 141;
	dataLen = 256;
	combinedKey = "a1ca1c7bdc10a4da9287d4921cf022b24b1af6d286b8831361002e97fbd4a39c";
	iv = "e59408a24ed500946f0d754f864a0c47";
	cipherText = "dd43acdde862be0063d1a0071cd54f2505e0ab261490588c0249550d3cf960b1";
	plainText = "d19e365fc839aabf58893137f58d84011831c76e8405244c95ab23304b026cff";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 142;
	dataLen = 256;
	combinedKey = "5626e50b50447192d76c9e48b23bec15d323ef7ad4a52a642622deb7afd5abd1";
	iv = "c58efe82998b3011ddc107b22945d79b";
	cipherText = "b774ec0d8fbf162020244513d2760028df12a5e68a91e8fa686bdd795dd6c3be";
	plainText = "4a83cdc37d805c2dda665a26ce056115390f0c5862b73fe766fc5d7fa99839d8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 143;
	dataLen = 256;
	combinedKey = "a990ef727b1f9e94c73ce392d59343e8007c1f5c1e4ab1374e44000a11c9e16f";
	iv = "2fc764bc4084bc0535a8879318734018";
	cipherText = "0e2d8b64749e03536f181f4c4db6dcdc324233606c12522885fefe100bf91088";
	plainText = "f6808e12d8c05c5937fb0a8e06a2dcc1cf11741079088312ea8e530f18f5fbdc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 144;
	dataLen = 256;
	combinedKey = "d6f7265e4fbc2dbe49852fe9eac540b82edad6fd9d2998acb7a224e2baeacfb0";
	iv = "d51d1d1412a16ab48969464aae734915";
	cipherText = "e5bc23c6adce9f6bc9627b45ffeddc347f545f25b9aa540f62792cf3dbcad1f9";
	plainText = "8b493dde8328b746525acf13e9e1988e21623c5ce1b332deaf0f8ef3dfd48b19";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 145;
	dataLen = 256;
	combinedKey = "3e6f4e19a6202282e68ff8b7264521b2b1cb1f6914b7b354dbfe88670ae24746";
	iv = "b817f7e357214510cf62cf165a253180";
	cipherText = "0d5104dfa38fbd873ee832c6c1948f111d17d5879abb437010937cdf50e11ce0";
	plainText = "b8007c7ae42fb6e971e8c245ddf85e589a0faa5628c526d3563fdf49a6918ab4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 146;
	dataLen = 256;
	combinedKey = "30e4d3758e06ed8e7a6547f3d0baf5a4b77a55ad3d2ae8ff1b1817bee5c98cb2";
	iv = "1dcb50cb9e54afe949156b2d46961c58";
	cipherText = "3da9560e8b07ff853af1e15cab84aacb5ee6939031a1d360d4909ac46e574ae8";
	plainText = "81bebb7172bb0b3c9bab155426ce38eba7b22e17c71f09a73df893f179b25a51";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 147;
	dataLen = 256;
	combinedKey = "fc22b40352b66f8dbab0ef27c4109e38e17e0a007b162ab11369785661f6356a";
	iv = "c3ce421280b32a9fc3ab265f8d253ac7";
	cipherText = "25d710182c05542c28264bd68e2e7fd4f979692d086e77023dea3c3525abe11a";
	plainText = "bd04ccdf9708c8adbf7a3a12fbbd22816fd9537522f7aefb55ed5426722dbdce";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 148;
	dataLen = 256;
	combinedKey = "4066c651048c7529010e81614be36b4d0be7d0ddd4bea953fa50d1cc4f6b4e2d";
	iv = "43728e6cd43872a563e50c2104f28f32";
	cipherText = "531d73f8fc77cc3528f070bba471c11a1dc7f766524e337af98b1b2c3ad2c6bd";
	plainText = "5b444f98d8a6305e2b3032ae7c672db28dc264961863570eec9007b0225fa0d3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 149;
	dataLen = 256;
	combinedKey = "3d5f9b7f4f1d76a474b83dfed544bf7509aa39cdde7d29789370a2b310a1c678";
	iv = "849205e46a6f607e5896c3fbcd3a1cc3";
	cipherText = "5fc53b2f6e5533e70a0c2f9275356ec746bf8d6d06b65a495060e93c63a4bce7";
	plainText = "c2d51bb2340cf7c5176cddf4b41cb08e895fd2a24bffea6d8cad69f580174ae6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 150;
	dataLen = 256;
	combinedKey = "75bc63e7037a837e6ccdaa8e72afaac14b0712e3e72e26c7dc457406e8d97718";
	iv = "60e1512ff7ac3fd04cd8c9358b3a571f";
	cipherText = "ad53ea332ae4326df46160d2cd034326e904057a2e77a56d9c3c68a5d15c7f39";
	plainText = "29b0a87040535689888a2403d00d0d31ffa890d0e0a654822402a2096ef3003e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 151;
	dataLen = 256;
	combinedKey = "b020574ec5ce0bf7baa7d19a705cf88eca79f3d037e1f1930f0c8e5fdd2fef23";
	iv = "f7f575964afd1965e3abd58e5258934f";
	cipherText = "6298eb2270120f2ef908203dd8e3fbb60418bfa97a9a2f9cdb52b65962ec13fc";
	plainText = "71c3d3d6693a5950d7c421e035c5aeec7a492b7fd0ca6644a290eb66c0eaa3da";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 152;
	dataLen = 256;
	combinedKey = "d90e86e33d0c2f91d48b70abc672a01bbf67d1ec009d55cba4dcb9dd5b6bd055";
	iv = "644d45113f35c2f2623c756aa3290f23";
	cipherText = "c9786fa6bb5239a18924df6b0ce81fe97cc5dbb9e932ae64c67c37eaba90a6cf";
	plainText = "fa497ca68ce6593e4d49a680bb5b1ad4249ab1ce1376b6fde71b1c7708da5b74";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 153;
	dataLen = 256;
	combinedKey = "4b06b66b1486ad311be59c7c4b9fe9ad6f5f7433b1ed90aae80573551399c3f9";
	iv = "cad5601ef7ef95228a0efb80bb746946";
	cipherText = "cb4cdcb4939c29018ff2d263ab71b227896d3428e9c15387d54e395884081852";
	plainText = "9f7cb08698734b2b4c70fdaff032c3357abfc0cd1051b231838caa1fe580fe04";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 154;
	dataLen = 256;
	combinedKey = "dea54bb0f022414bdb8397ad23b31124f112ecf8339c49429bb1b2607a301413";
	iv = "f98040250b58279ac4e4d4cdaa35841e";
	cipherText = "fc65f6c581ca68fe7a39db8d4796e2185ab8fed4a82a9d9759c98a9d17576dc9";
	plainText = "d6cf5ca25194a5099150f1092f30f65741d598581310038279d8b2dbc92d9319";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 155;
	dataLen = 256;
	combinedKey = "d023c85f1f3a732d6fd4e9e67d9232cac61aafea4185daa1f2ac74af422c019f";
	iv = "a7a42dd98037cc68941c6f9e15b956d4";
	cipherText = "a14503ef6d3987e05c336ccf1012be14b18df2f3bfb2005981fb6c5fc14acd7d";
	plainText = "7fa7ee4af3ee5ca37c2da62a02bceb0d2006aa2b53b421209f8a7e400555bacc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 156;
	dataLen = 256;
	combinedKey = "ddc723b045383524589cf34efa23b2d48aca0174871d8fb3a84d9bb5d1d070b9";
	iv = "48e7379ad0551195d7591131111a3073";
	cipherText = "0c25b9bc86a03e6be12762cf17b25abb86c25f1d3bce92c71fd971e833897da9";
	plainText = "de491bc2f55f5ead3237c60c72335ac5ece473a371c02d9ccdde3ef02dd070d8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 157;
	dataLen = 256;
	combinedKey = "60239b3a1c609ab43fdadfb37580fcedcfc798cf0ff434e9fe959a3605a7713c";
	iv = "42a3ca8317cb52658f0c912172255c4f";
	cipherText = "9b6a194cf0796c5c23eec5435890b5cde3fa89302ff87128a1bc77e1bcb41cd9";
	plainText = "72103c480a46ee87b3abec48f81a736a6cf54c0b317870afda8075f6e5f9bfee";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 158;
	dataLen = 256;
	combinedKey = "363f867cfe07b2d547d8cd03219672a76f9ccf7f94e196199c8f9daf54f7eebb";
	iv = "b714e5a457493c5f1eb492051f85c574";
	cipherText = "f035e3dc0c9b8f70bd41f4a5bb43a07f839d8693c3d3894a6778714f1fcab22b";
	plainText = "17556339a11d731c100b125b04a2a6ff49af9e0b3c88ec75c9b9decc742b7ccd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 159;
	dataLen = 256;
	combinedKey = "a1f9dbcdfb3328a1825c88b3cbd34b4bfe65762472c473309b9de532e8eb5ed8";
	iv = "6537b71a73932878e8779da49185b3e0";
	cipherText = "b0533ef5f6f266bb7e72cbf0a59961ac0684b6cafd7c1e7033833d69b082c1c5";
	plainText = "6a546ca8b2e7aff8e69225158386be07602f8abf9ad60aadefa04a7ddc4b40a5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 160;
	dataLen = 256;
	combinedKey = "fe7eb772c48c6a80b9386499782ab6a28c2ffdc18b6f11edd8e266ce8237dc14";
	iv = "188df3a513a4244e9b0d158836d64c37";
	cipherText = "baa064d6271a69a88923d45fc5339d49611140823ea904edd16597002bb18e3b";
	plainText = "352652a2b16af379b09a9b32f1eb425124a078907d9405a7d88a2f8caa896275";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 161;
	dataLen = 256;
	combinedKey = "ef59993eb626f405d75ffa84aed8746540b1c07982f45237d0ebf2f9c60ecc71";
	iv = "344136a18c7372866cba27da4ae5e838";
	cipherText = "8cd223f9a143e6d740f5650cec2205bf1d471270776cc7b4e739517ac3c06606";
	plainText = "f0f7dc5de403c487272b05ec074c2d87e2c1ae5fd3427b8021581f3dde8f56a0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 162;
	dataLen = 256;
	combinedKey = "6425b4ba600c61dc30d4b968399f1a64b0bee9bcfb038c900e87e9c2f0172fc5";
	iv = "21af7ed62d89989c211ef9a16e54eb43";
	cipherText = "c3ad5364c9d9e85b9b26b66a4009f7f310fcf189a57429d37670270f0c6805e4";
	plainText = "16898f3ce68ab4a9f855493a8633fbe473d35b0a54e2c541b8604e9aab0757d5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 163;
	dataLen = 256;
	combinedKey = "b3ea2535243acd6d1f02cc380123c910117e6aa3ad7d48f1e022acb67fac0fda";
	iv = "778e4d4f16e9e9c9e6d677ab46e3816c";
	cipherText = "d5aa39520a8ce6f5257e83e7bc0ce525fffefc5210298787aaffdc6914f1b397";
	plainText = "50ece505815f08cc1f8397e557b5b1f7c7a1dd5e719e21dc8dbfdb21b51aac3b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 164;
	dataLen = 256;
	combinedKey = "767e977f0780a7432acf21ede2930b249bf0a63bb6a88d4459b1a0142b7de1b5";
	iv = "ec0d37199b8b55a9a37c2c98cf5e9182";
	cipherText = "807ad5e12a25fccc71aa913565f406fafbd7317ca61b6f14b297df99b71160bc";
	plainText = "9f427b755c3dda71d9c3d88771a92ab5e4d3ad3e36c638047d1e37b9b513f6e8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 165;
	dataLen = 256;
	combinedKey = "afca211742433571ebbe065c73aee533c09c352e80bd99430bd0a019e69f15e0";
	iv = "8dfd55800efc7db4ec1fd84412fad2eb";
	cipherText = "6556573bdfe5ad000f061a0c168214088a0f230dd798dab59bc450a577a3bad5";
	plainText = "1db1206ce1700d543949c10a6c1d92321b07dfe7f98f2fb5d32470df11e9d5d5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 166;
	dataLen = 256;
	combinedKey = "3e1f44d7993d0e78a467d2bf0064481fe0e904ab2d4fef5ba8b1678688766058";
	iv = "8e7ad323eaf134e38c202f8cd9bb5c08";
	cipherText = "c13e1150b43325fc2644c170c5492548fb2e0ae1d5b0bcb10ef3777930e7cb4a";
	plainText = "f4613cbfedbab97842c94682edccfb347084f21e39c05e9346f63b70e405c33b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 167;
	dataLen = 256;
	combinedKey = "c35b8ff3e415ce578abaf259ca5eb26392ad0741846ac0ba9e30cd3b7d304f1f";
	iv = "af4fd1d47c5609a325dd69bce96331d4";
	cipherText = "54cb90b1cfab48267234492ffc4ab50bc8e01abcbc121acd726e65e290e340e1";
	plainText = "e9232f2747ae28cd18ce1c0d9933711726dd26664a18d736ca946b69b7f560e2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 168;
	dataLen = 256;
	combinedKey = "66fb05de33435354d0b1828ee072c7f58c217957b15b164fefb75e6730d95ec2";
	iv = "2e5e98187ec53ebcf386df2a42b92bac";
	cipherText = "8fb501c7c0b118c9a65587289792aab15894f7e05dd9ec87312b25ed7145322a";
	plainText = "dca0f4ca594fe88223c2a9124165d0bec412d244d6700b44627531e5107430a2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 169;
	dataLen = 256;
	combinedKey = "86f8149ab118194d0802cee00e0e16b37858cf3f81b0ef157e5d974a70eba1c3";
	iv = "cb8bbc7123b194a595208b679cbe07b3";
	cipherText = "c917d70fa7ebca1b0d271abda72a955d39f312528c9ad4a79086715875a5e3d8";
	plainText = "ae9453f6a09e17e8ab92f7c379797a5f61900064202af702fbbfff8cacae07e5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 170;
	dataLen = 256;
	combinedKey = "a2feb7db5b8b8b43e8eeabdc708b33827bafbc287550c8b728b14e2e27ff7dec";
	iv = "df2111491b05020f123b817ff21d66ee";
	cipherText = "76c77d52559800ba5b7ac84f518cbf932e518f6542265a218179adc275d7da95";
	plainText = "b4e28480a4f5e7ab8ab5eb2d34fff263567ae383aa097b80c067b79440f7d2b0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 171;
	dataLen = 256;
	combinedKey = "f95351f5efcbed389f5f8e950498a770e79570924bb869cdf90c6c9801d02c38";
	iv = "de1c4ee200e47f28a65165b6eb5ffec9";
	cipherText = "5da9a49f793dfca75b93a8e370d108d3e97249a51f042ed19d6fea3cf7d2b3f3";
	plainText = "36127d1d61b62514ba16d8f0408ab521eb13b9a5a598b4711696b90d680512e5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 172;
	dataLen = 256;
	combinedKey = "711907b409dd75d1e3510cc4d98869af09a7268cfd9f755e5fb8c4f3ca2fe071";
	iv = "b27fa52899918de07f7d00d2b527da8f";
	cipherText = "a24d3f94604b3c7c5458876497c315bab18cd8c187869d5ae535d372420d5c13";
	plainText = "a96d515ade745f94afe3e0fa68032fb00f94c6008af41871c9ec7b95cb0e14ce";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 173;
	dataLen = 256;
	combinedKey = "9b94cbac1d07dd18799bdb7ba33ef22954cee7f997acc0310aaed438e2c13832";
	iv = "feacaf0702f7c96bd9f8411a288b7353";
	cipherText = "a96e751fc2b74757e2d95584ac47f564abf8425affcd591eb0e44169b559318f";
	plainText = "db9c227d0ede81b422537472942f7ee1264dc0cd917018514c5c22cb04bc3d9d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 174;
	dataLen = 256;
	combinedKey = "0655575af417a2a914b7e154cdca734c126a1cc8bd84fe7ad654facdbd580e01";
	iv = "b2f1e60733e2f0f9606e099207572ccb";
	cipherText = "2499102169d25ade39d59efc7d5cf1112c19b53369aae4d823eb2cc5e671a31a";
	plainText = "cc0039c9e5aaa3606c5ade32c63bea3509233f7ac6ac4c039ec5becfa3ab3813";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 175;
	dataLen = 256;
	combinedKey = "d243f73aa0ac21268a3829572c03c026bee04ccb6d7acf074c518c03bb72a061";
	iv = "cadc2ac4c72493194044a42127000628";
	cipherText = "cad6b1cc16b491178c75e6c461e4125c08eb089168cad01e749d17b422513edc";
	plainText = "3e64953496755317d1908054cfd72461459a7c31c9b89a46d095bfcb73b5c88e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 176;
	dataLen = 256;
	combinedKey = "3db87ef71178b4d7bccbcba412e7e84a8bd225d4173d4525ab036e8d5fd88a7b";
	iv = "f71e65efba52e47874b1bed59e9f656b";
	cipherText = "76d52cc26f84e5c3c0f86f9b0ad46b485fed6a6c3e6d913d00c33e10ef336879";
	plainText = "a09902679bdf1e8db67547faa5c3c3b3dfbf3c9804409c2093b29877960320f7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 177;
	dataLen = 256;
	combinedKey = "24ec7f7f783c5efb3dd67e64f4a25ec84caf191120ba8515f57eef4434038892";
	iv = "4bb82f22d2af2f639a5d21b3c1263ad9";
	cipherText = "2a12508f8e4a909601c6bc1e57db37566448af0b097524dcbffc7ceca2a2f362";
	plainText = "ca74b47e04620eb3ae8e0e896d5760489ad3e7177386eaa781afed90b8af8760";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 178;
	dataLen = 256;
	combinedKey = "30d5accff4b868b8fb433deac670604cc750a13f6afdf66d08cf8e8b5ddf999f";
	iv = "4be1d679ff32e03d720ee52558547b73";
	cipherText = "e38b3030c63b71c109700774b4727ec8163c8e6dd5c13182cd0d66584445ff5a";
	plainText = "fa0a8a12d86c7a6f166a1ca7183a358a40c95515b68b82b1222b1d13af597435";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 179;
	dataLen = 256;
	combinedKey = "edd716fd637f7dc48ceaac7f54385190f72f9d926a8694d80401f9543c2ea32d";
	iv = "6e262464c7e6deef93fb882230c4787b";
	cipherText = "a7df6645d58d7aa8dc313b31629d12d35ed3979c861ece544f7cbd807641a575";
	plainText = "c48210956d49a8fa645644467883dd59daa103d1e8de5e57f3fcc3c08da05c02";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 180;
	dataLen = 256;
	combinedKey = "7726142431321b04e66ef85817756ca865f9959fce7cfd3eea2b4e279ce13054";
	iv = "f2924e89d33a18743aeb14e7fea83a6a";
	cipherText = "782194cbfc42c60eff0b3040f66fba826170fb7253adee862ed8493099e703f1";
	plainText = "3a8941917fabdb2c766a9429cea6159405d5c4a1839ef67b9b2b0fef1d0300da";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 181;
	dataLen = 256;
	combinedKey = "25b69fc28297e7626578665e703ce2f4021ec6408ea9c5e0957d4efefad3e046";
	iv = "7c7a5bccc55b4f66fda53e8fa31df30b";
	cipherText = "f60eb1a7e79d58fbef3bed6e76ac435f2f79d113fdcc0d9aca6bf643b2b8b391";
	plainText = "b76b266d8e8143450a05c4f6cda9392b77a919f25b3c1f28a5e827fa52b34f33";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 182;
	dataLen = 256;
	combinedKey = "bfd0fc840590382cd469f9c82a62a46ac07c05f31060b83c87790716a5fc30eb";
	iv = "e541c2dd1c9e282468ef6503c4d77d4d";
	cipherText = "faf6fdd7e83abe05ef2a00d9ce455fcafde37d7fb6775b36de942a427abf274a";
	plainText = "3d096b200a8525da64c3a20610eff9b5b5177a3b879f802374a43d93692c82f3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 183;
	dataLen = 256;
	combinedKey = "e1f49e3f03de7cf3bc40f6cea304fadb2510dd2ca30304690634164441fbdf88";
	iv = "d562a86e3a359c5f7121e01e62e1eaa8";
	cipherText = "46be77a0083a9c4e7062b1e07903c86e51da6c96a1326f0b01c0dcb772623502";
	plainText = "d322df20f580b79d42ddd216717a50b37b8cb2c50e9d9ed9bc61313ea89809d1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 184;
	dataLen = 256;
	combinedKey = "8af5e3068800c5b79940542eb47e2894d8cb70516c758bcd8a666907044e5d14";
	iv = "969b68a94ed0b73440fc175bc91f5280";
	cipherText = "14e7932f869bbfc839a739be9d681e98d4f5dbe2f9b0d9b22663e6853cf013b8";
	plainText = "e3d8d85930315e79fced06735e7d6c8da3d558463dcad726c9a64e3bf13ad699";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 185;
	dataLen = 256;
	combinedKey = "d8ac76f14246bd9e32f345c4ca121eecab5b83311f650e86935c689ca5439249";
	iv = "9509c6dfff06003f7db13c9c0b1d54c1";
	cipherText = "b1228b57d48be870d10532b5ba801be3fcc963f9658e7a64a9da70e8b1b19cda";
	plainText = "7b769018f587ce3340a672dbe72723704bb188636feb893b9f84d3f055541681";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 186;
	dataLen = 256;
	combinedKey = "b429f55e39d6a8aa06afeaefb0f0da4c7e71b7027d3552316a848ac82dbf56f4";
	iv = "0ff19bb74c868c82404c63b39c1f03c0";
	cipherText = "af5eeb55e631dcd80b74c47af7184d21ec3ea27c15a9ce9c32cf17fb88ce68dd";
	plainText = "a742ba4aae2b0efdeb4fadf82911c3b7bc50c1440197b8fbacc697288492329b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 187;
	dataLen = 256;
	combinedKey = "509142f11c4ac3312da3d20e265db9c4f26672b97dfecdc6b7e86d7f14e4f646";
	iv = "ceeb9ac40ae7c7f82939476efbe26a49";
	cipherText = "22446490cb65f03e5048836c37ff17b72e04aeb0ce8748ce510c0f8acf6693f3";
	plainText = "263032f1f7ea647c218bca88ce3494b87269479cdc72b331588f758dc61d336d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 188;
	dataLen = 256;
	combinedKey = "55af7a97ab44138c3f8024b5f2c1714106ebae79a6e7c7ad94a0181975e1688e";
	iv = "9629a53e1a82f2528ac3a55a14af7a59";
	cipherText = "4cfc706fab291ffa23daf1ffa91c27e687a66cd8aa7b3636ea3e3f65056fd0f3";
	plainText = "7efc61b96181a550b4c334456b84b7a202d4618098e4b3c811a0b9d3af9670db";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 189;
	dataLen = 256;
	combinedKey = "ea7eb44003e9bf171f2ea5cabb6fd0dcf678d722fe230573e9bfe86978450dd3";
	iv = "cb96cb5dceebe78e0cb89a4813de1a2b";
	cipherText = "cd2b13835dcd4f0411ef8c007bd43709c4dc166bebcc72dbb24e6686c870411f";
	plainText = "eb93a60a1b0b5b7a490ce39685aae3d35b72219de9142cc89a9fffb566638682";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 190;
	dataLen = 256;
	combinedKey = "582de2e90d130812c3e9dde1425a97610b74a8982c4c4c3adb8a5abb7552c4cc";
	iv = "4434f2f90422dc47c400a8be0c5c61e8";
	cipherText = "baf7b87c2048328f092b925740f163c1d547aacc2d8fe38ca09da9fa91998dbc";
	plainText = "3b6372cedf430c3d2dbfe0108398a389c749ba2708cab2433c8037d4c95ee8b2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 191;
	dataLen = 256;
	combinedKey = "1dc8af9c078b9a92ab30573d97f57423daac1efc437d3f216517f640cd5f2542";
	iv = "3a3ddf40860f3c96f639c5c5c980d7e9";
	cipherText = "2542b38fff8adee346fd9dbdf591de775934952ee2d5e4213c834aad9185b7bb";
	plainText = "2a08ac707249da567ed0d7b7702806641f07b372c6f7e2ee1d71b06a15172784";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 192;
	dataLen = 256;
	combinedKey = "849718a7a4c60c58f4163083a8b340ad85f178f46af362fb5fd9f7787c1cd3a2";
	iv = "d7242fa38e10e2c1912386f5e4f57f0c";
	cipherText = "5e8e9f74d7e06400bbfeac1cfec88365ede305d6dde6e06b3bffab7d05b60069";
	plainText = "c515c6a929c77c9379842310e980f9e3f07c96834740e1f78278f45ab8432cac";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 193;
	dataLen = 256;
	combinedKey = "9baefa2fbd8f07cfad8f263b6f38d528d230a605f60ec4bb8ee05c1cfd0c48cf";
	iv = "03b38f313c631dcf1193ee02c9bb4f60";
	cipherText = "1a2089e9cb179d1ff6e1cebe0d6546bb976a6ba873e77b0e9f15299cb8ff9205";
	plainText = "0ea873a0a3c2c8227f0308e50a2632ebee97f9986085bc57c332c667076c9025";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 194;
	dataLen = 256;
	combinedKey = "d26bf9a7f6ea012f1dd26183038b60bb1481dc60b5b581216aa7427b7d018089";
	iv = "e335fd5c1d6b72887d13dcbe8d40cbaf";
	cipherText = "8399175f06b764b5b8c741442e2f9ff97b27055b987d13130b03366896fcfda9";
	plainText = "d836fdee9f32341eaa0783bc8f1a009897d11842f142e61d85d85edca5ab165b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 195;
	dataLen = 256;
	combinedKey = "bed8d860a510f1f7dafc07494d5c7aee51bcb215f21cfdc62226ddbd23d0d278";
	iv = "c7ce763661ee111af174a6c88888611c";
	cipherText = "5282bac88b3a0513a5ebba6f6b96399cb23780a987782b6991949f3c1d49baa7";
	plainText = "75002b3591a162a666050020bca798e35cbb6d5750028b76a0b89fd0bd818f20";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 196;
	dataLen = 256;
	combinedKey = "3aae4f7222142deab482b5d7b903c2d3c92f3dc4c8bb8be651de26bf5da08764";
	iv = "f575885839e1ea4021de37def7360e36";
	cipherText = "ca0eb48d2559e7b9f0297a79f3732bb569150632d97f4488a426453e63c0bf1a";
	plainText = "81600fefcfade51526ec8c97020bb7fc5752121a8133994686f3bfe3e6750f85";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 197;
	dataLen = 256;
	combinedKey = "a7011dd7ec5f9d57fe7a20cef2ac28a08f92779b5ef4402a2438f486554affc5";
	iv = "0168cb2c62ab43e353fb5f0dd78d435f";
	cipherText = "b6f22116ac2d70718b65551b4ee7a6932b0466e9b6d211330a07153050b4079e";
	plainText = "0162633fb7363e2396934ea0c30dd82c4a3ea795f5c12be0f7af1f78c45f7d0f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 198;
	dataLen = 256;
	combinedKey = "aacc20ac91b4dc4e440c44d89ad933dab73306ea5f365df14cd87118b86c1be7";
	iv = "b450eade08897042b87810153f275ad5";
	cipherText = "666f1a7c6da7a9f6527cf0fb19f15e15bf184caedf46f21f1cb09ba6ed99453a";
	plainText = "d04e0bbcf0de146fd3abe6c798b258c410c75921aa6c62b06e90f602a5a0e725";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 199;
	dataLen = 256;
	combinedKey = "05470a50a66d5d73ee966331c8a1a98fd3c9cc13c2a2d1daf8ed48f3304f84e3";
	iv = "87202dfbe46437d0b59bdf7ae23abbf8";
	cipherText = "d3240c09183aaddf37f4145389950760f9ca57bf38427727114098df02db0b00";
	plainText = "4997bc4347e23b9bbce6d248c826abede86d9f364afe0be817931e95f361909d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 200;
	dataLen = 256;
	combinedKey = "5c175e5c029bf737639e81dfd8dab089550e713f3b7413f637bca92e7ca11129";
	iv = "b6e8d522ea13d066c1a624fa110a4538";
	cipherText = "661964ccb235a3083528783dfa7939e24bde6a23e8864cac526909ca5476f094";
	plainText = "326b9afdf4f9fa1d8f4e7b1fbfaea9beaa6d7ed1fb241262778d9af44cc7bf97";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 201;
	dataLen = 192;
	combinedKey = "ad402199b1b0a479b54b3643f26d9cdf42f64d0cd72bef286f94b64ab9aa074a";
	iv = "3d8b17b42d776009e3f91c094074bafa";
	cipherText = "509de6cee075a82f96fb7db5fac03f38b1899843b3d06dec";
	plainText = "cc6fb25668ae8ab46a2f5206d92cb6672607b5d1be73e267";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 202;
	dataLen = 192;
	combinedKey = "f3a98f1ba8deeb23fa1c85a434df1481686d9a8febe74312adf32420246e1a86";
	iv = "a0603273dd6cbfb42ecc98a220ad177d";
	cipherText = "b1fe4eef4b8e1f2f121b044b8ce3a5a3a95da4751c12e0cc";
	plainText = "54ee0fcf90de79cc698ddee4d48fa6a33d1d9f9670aa83ec";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 203;
	dataLen = 192;
	combinedKey = "a5cfe2abc117be0f414eb52c6e4e57957e45283bfb6b6868dbd1ac1f225abb1c";
	iv = "8bdaae3b9af4f271d505d7727066292d";
	cipherText = "80f4c032e0a12f4ac8ceb248516bd1273f58706fd2e36fdc";
	plainText = "98831b69eb1c1e1f292c6b861518822335bea62c7c5ced5c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 204;
	dataLen = 192;
	combinedKey = "a66366c4ea2648bbba447f26c3ca0fb350930b4bf47e42b874cdde59fa27bea2";
	iv = "e337aacc060c182fa07054c20913461d";
	cipherText = "2cc85ab3aecbc38e2f7f240dea82a0ce9c60422f6f70171d";
	plainText = "a3418229d798ee7c3cfd38088cc9501fad69fa366d55450e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 205;
	dataLen = 192;
	combinedKey = "f885552ca2eb4781a3f59cb68df4700c54d62ca55fc49aa851b601964af6b5f1";
	iv = "bc27d6f5c348f79728369c7ab868cc61";
	cipherText = "0036d80a988a111de21226949c2db534660fcb8380b7b42f";
	plainText = "6fb728817ea6982941439c63fd418b41d8aff57380fed7a1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 206;
	dataLen = 192;
	combinedKey = "28c561d7ae3304a06c79f362f9312d7c5e67155675d2e10975b28d04046cf800";
	iv = "024c9288eebd0e423cfff85f5931ea1b";
	cipherText = "3ed59af6a8ad4084f4ad58c8aaae09d9f88fd16223baf2fd";
	plainText = "0710579eaba9746e71719a731e1d16fcb0dd146f798a109a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 207;
	dataLen = 192;
	combinedKey = "de496c73fc5cd657c978069d3be80827053074d267ca40e2745d4aa2a13b57f3";
	iv = "6e843c9036a0e7461d78663c35aece88";
	cipherText = "a911e9e6176e7fe27d1b20425e72d0bfa2d8e98adb87eda0";
	plainText = "61458d7dda5c9aba7afd74151e1acee026ebb6b8c47f4790";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 208;
	dataLen = 192;
	combinedKey = "f0b6688206bb4cf2d915f533f1fc73d2b64f94444220606d30baa0bd51116a61";
	iv = "a3235b7f415906ed4421ab7c42adfec9";
	cipherText = "f8af835862cf1d76776cbc5307e5603279d8398db608741d";
	plainText = "b99e2c1bf4578112bb625453b13f20c88400ded6e6caa138";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 209;
	dataLen = 192;
	combinedKey = "0a80d9c0a55cf70235c4ad60dd087c8a7718707ad47b352ab118edca14e55519";
	iv = "b338bce1226a496f69bb4e99344bffce";
	cipherText = "e25bdf1ee567f7de0ddaccb6d47637e9c5e89d5ac2114ae3";
	plainText = "f57811de52f1eaf2223de4fadfd97a3d2b7013a0c2127a81";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 210;
	dataLen = 192;
	combinedKey = "7045fcdd61a53f938d1826da842ab994bab0b1d6682be5a5899e4d09ea642fe4";
	iv = "98d8326e7fafa2c2abe4b58f6abd7d9a";
	cipherText = "d9b9350ca53463283c96a7e7936b7cdea00308fe096cf747";
	plainText = "2285d8863bb1101e0c6a33673f616b524daeb1343bc999ba";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 211;
	dataLen = 192;
	combinedKey = "e52d9ca2151e8945e21240188e0bf557e1dc155f22814399c590489d5f0b32e1";
	iv = "7c6f901bb81312d80acbb8f9265d6ff8";
	cipherText = "7fd2ec8427485f110527460410ecfe95c835cd69caa402a4";
	plainText = "d03158565858e0d56a147045481a4c2cb2bc1e8a52c96d14";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 212;
	dataLen = 192;
	combinedKey = "64afda92bb22e04a1460b92c06cdfea48ef6bb05e64ce652901aac0baad02d3b";
	iv = "30818c286d60de69610ae303da0bdd61";
	cipherText = "c599c7a8ce5427c1dfa753596855ef49f472035afaadb58c";
	plainText = "0bcf3c3022dc4352bb6fb955b75534e9354f9de1b7b2ddbf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 213;
	dataLen = 192;
	combinedKey = "c7a1f079a7e4fc7941a0846bc3849cebb53c62f0ff7db9b44559f8d7ef15bd17";
	iv = "dee1b747e4df16eabdd6338eb653141b";
	cipherText = "fd11ba730bc2c646e3c81534df1512d34ef8a89eb299d165";
	plainText = "77b5f9025c6b6d73b7eb1a550e6c4ba731d432cdb86cc4a6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 214;
	dataLen = 192;
	combinedKey = "95154951cf85c2be7190776e30215f4df4687cf150421c24ef953b62d06e2b31";
	iv = "7d0b6063dbb2d66c0741c3a6eacb8e27";
	cipherText = "578c313bb9ec619dbdce89902d5ee88297d9d944cbc4be4c";
	plainText = "2c8fcc4162607a80bd1dd77251113355f39fb7ede2becebd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 215;
	dataLen = 192;
	combinedKey = "6dc75ef6aa0023af870754ee2b5becbe54cde77e526e2ea28f78f9250eb0cda8";
	iv = "f0e2a89d7fc58e53121a1cacc66bc3a5";
	cipherText = "c991fb40e24c06f9e9a8309d7122c0a3d8b36fae7f0d2ba1";
	plainText = "bdb81086afcff87606861ef449e8e317573d10714c7cbcc7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 216;
	dataLen = 192;
	combinedKey = "3afdb1553b1868c537617d2353fb582abed81750f3917a8adc7f9b84e17e8159";
	iv = "08fe618b8ba077979d0072d01decbd09";
	cipherText = "95a187bac1e5954806435141b31ad2348aaecb1699d3fa5b";
	plainText = "8bb5d31f18366c99ba2b8774bfa6f57532a547348832a09a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 217;
	dataLen = 192;
	combinedKey = "4dc48198aed7d1f3d9cfb036140f0578bca45b0484b6a5068fb415bdfeb47dbe";
	iv = "87ec7b2763727fb53e2fbbcc614a58dc";
	cipherText = "e354084304e47d2ff7b821e03c332e37dabd00f19b9aa06e";
	plainText = "8a4721342c754d89ab032ee0e00e0658634759f33b9a3fda";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 218;
	dataLen = 192;
	combinedKey = "b6d2b5a6825385e8b88624330b8c99e7966b08bfcaaadd40bbfb3ac4e6e6f2e2";
	iv = "09a1caebf99b8554d77e1224ca95acea";
	cipherText = "69915ced84b168712b6729fe87b6c9f7a32dda5d1d29e163";
	plainText = "27686b3c949101b60ae2028ff6acd404c737e28a9c07a133";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 219;
	dataLen = 192;
	combinedKey = "81b2d10f9462edd7a5c6621a056d0d7e630c57b60d1a21d8dffbbd58fa0712e2";
	iv = "d863189b6a1c195e42816a412cfd1a99";
	cipherText = "a88c57dff8f6bb4ee1f23d05db75e6c29242086d0494d001";
	plainText = "b6199c91b6ffc39731943976de167158a20e7dd83d4afcff";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 220;
	dataLen = 192;
	combinedKey = "cdf1deba386039c336e72fb9e6192aa07e1080a74ff337dd87be88bf47bc15d5";
	iv = "b770d940d0d1e050f30979dc08a556eb";
	cipherText = "9632eff040f1c1112e05c5e0774aa1ed1228098800c0c9e0";
	plainText = "04046f6b287bb96daa0f40811d9b7fb333bc9d12a6d9911a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 221;
	dataLen = 192;
	combinedKey = "814e5e6904bedd20ba656140ddd9b18a07702c933f61f1b78d1db8698491788b";
	iv = "4da80288be5336d8527170a9aa664f80";
	cipherText = "350a4e3378b4baf65e7e469ec37dfa001a9057f79f3fefcc";
	plainText = "36c4066bc713d2c4664d1c907712ee611de02d5aa936a7f4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 222;
	dataLen = 192;
	combinedKey = "922f86f5f3548b3fc76420929076a7e068f922dbfef637b8de531fabbbe1ac1a";
	iv = "50135901bbf25882617d25c0601c0ae0";
	cipherText = "60b463cc7e03998751d7e5bba215c151f63104ca5f4b8b3d";
	plainText = "0fc8c8742b10b8f93f39938a08861543801e25898f6cd67d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 223;
	dataLen = 192;
	combinedKey = "a349bacb5590a3e576e187083861144682451cb8fdf127e1684da7f9daac1f5f";
	iv = "89e6dddd6d57f441c94c11ba0833b4be";
	cipherText = "b4eb8bd39a69143b49c47e0153a46a32ee36c12339826783";
	plainText = "e66db19df660b0c1560f2ed6bb61ee105842031bc9bfc0f5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 224;
	dataLen = 192;
	combinedKey = "38a5eaeb563b05b9d639139742453ee0542b83ed37f742c816fd204b4d8dc150";
	iv = "9634c1600eab4bf1ffb4522b7d64819a";
	cipherText = "56aeb276a65efa8db15492c6705fa920467400765aa46ec7";
	plainText = "0d88e7230feda9f43a2e644ddeb86ef718e72333cbe163ea";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 225;
	dataLen = 192;
	combinedKey = "fe0aa4f84024cdae7ed25e6e3cde1d664de9ed6f82bc2e812a7d8ca4445a1c29";
	iv = "9a50f294d00e60893d93b0854228e433";
	cipherText = "d41862f403a130f243de0eb277d9687b3b0939a133efdbfd";
	plainText = "e52a058528f517c383bb2f291850af82f6658a6960745ed7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 226;
	dataLen = 192;
	combinedKey = "2e09fbbcab80e8c90697cd3b9cc27ba0a41c2f989c7878d0cfc32ded64c94c70";
	iv = "5bea661e231d4cd4e959b18e871ec21c";
	cipherText = "9e87abf1a4ae74095177f8a7c4888219594c410ec03bfd22";
	plainText = "f9314409f633f878c6636d327112a4b870906b3ea8c46d60";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 227;
	dataLen = 192;
	combinedKey = "fcc10301330bd4c7dd044b599f0e9502a423998ecd6ce749204ee78511422913";
	iv = "3a271389477a530f87f3f32efac05c15";
	cipherText = "d57018de06f673839c67fc980c971bc12838a9cab02799b5";
	plainText = "f352c5d3a25fb3e98f76becbcbcda70a176fdc9d1d678128";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 228;
	dataLen = 192;
	combinedKey = "5a78ecf8284ff5ccdaefc707d392ff19352460877dabdd24257b98ef36871c98";
	iv = "1478627ee06cfb615d12e2f35c524216";
	cipherText = "af8d75f1be37173bb286d5006b7a9f405ca53a10fef9e101";
	plainText = "2a40eef5bfa6c7386158550d7fa6f087d3826f63f900ff3c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 229;
	dataLen = 192;
	combinedKey = "d6e18cab44fd2539a77f2821639d4115e7c9595ab7b1531edcd706f239100ee8";
	iv = "b8808022f3ac4c7e08bf7420c765d343";
	cipherText = "5afef5e1f44998dd5f886516d9a46a254737ecf722b67d5e";
	plainText = "3e9de6158e86ef4badf981923195887b0250db005ee7b0ce";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 230;
	dataLen = 192;
	combinedKey = "19a168422a6d9635a6e9c4e90fe4341bfa50e0a542fdc4d2ae2fbf3447b85859";
	iv = "0bd5cd33237dcf5266fcbbddfab64a24";
	cipherText = "0b681731984a0e7d0a733670fcb6e9fca9e0bd924ae0db56";
	plainText = "1c064cff0448f17f3af699912b42820b9641f5033172a239";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 231;
	dataLen = 192;
	combinedKey = "0e5c7c0a384fa33887f3a14e576f89fe5ebadca31a043e25ccd0135a0cc0f6d5";
	iv = "7c669f6b8194a5977a3cc2cff83e8a07";
	cipherText = "d3798da1c1e3650f040ea61c9ede55d5a9e2eb7c2161b62c";
	plainText = "850b5b3d64ac70cf15e5b93579de09f87cb88666a77469fd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 232;
	dataLen = 192;
	combinedKey = "f5df7c36aa43c2a0754cf8c0fdf71b05b75b3833149f2ad4384dcdab5c379a60";
	iv = "9132c1e346d69bc8dbce42ae549e0212";
	cipherText = "c7fa084761c94b7d0069a5e08c6efd3de83e78b575933427";
	plainText = "0d25210c3e537ac814db2451d80732b971b29c3e265e3adc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 233;
	dataLen = 192;
	combinedKey = "d18aee6c4c922de9029ea18619f2cf0f0bcdbde76ceb04652b5f1d48ec1f71d6";
	iv = "9be9b4f5be5df98f729a26e101d65e31";
	cipherText = "b9aa24cbec375f66af35a8f703c45e620fa19a5dc09f7996";
	plainText = "f53facb4e5de76b2679521a3288e0dc160b797fb8d6e1917";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 234;
	dataLen = 192;
	combinedKey = "f4337f3c0cd23c97399bcaf8239e465ab1291b0d9c8efafd6d5ead3933cf3d79";
	iv = "00945540db52cca1ae44e9676ea32921";
	cipherText = "f6cf494908750d87518e78bccc19f9b85cb538d149028b16";
	plainText = "ad346eb800db283fd5dc87cda76e2c9e61c3cdbfb5c19ae4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 235;
	dataLen = 192;
	combinedKey = "7df720c967a1a33f4c21b267099e15e2b38289921cbf7e51c68debc62863fe2d";
	iv = "ca6948001c2273dfaf8273be8f44c587";
	cipherText = "d42dcc781dc0c2433c62b4f82ce286c7539a9686cd660877";
	plainText = "6c753ea2b9b96a9ca0b1d85138f8f6d4a1447c9d64a5cfea";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 236;
	dataLen = 192;
	combinedKey = "4bc1ae52f284e9dab4783949bfe2147d2cf6ca16cb244bddaf50bb0db5f93aaf";
	iv = "b274395b354f14451c42cad075b52547";
	cipherText = "40a655c99f0668131716047e20ef7ff825786eba902d5535";
	plainText = "266b58d675b9d34051f34155bf5366f0119fe1db589ffab5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 237;
	dataLen = 192;
	combinedKey = "c545f892e1119dd8264edeee9f8b6a869ebc5908954df37354e81aa2f1c1f7c0";
	iv = "c970beffd28ba86519f4709444fd7b22";
	cipherText = "4c6da9cf6dec8f21e7b83780dc5b6164dde9f2916d8dd89d";
	plainText = "254f2be7209fbcc428c0da9a9075109dc5bacded625d3109";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 238;
	dataLen = 192;
	combinedKey = "04eef3d3fb8d258a54c5053800c4811fb260f1fd7c38fe2421d09c01b91c8c62";
	iv = "2dbab7e04ec4ced492ad2f6725b85b04";
	cipherText = "6bec13704f0602f0077ef189864208b011cb9a3279e30fe0";
	plainText = "cd1b9aaf4612fcf48d284b67f5f83cb4ed52b4378a4f675b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 239;
	dataLen = 192;
	combinedKey = "2f950196bb190ca97dab458c2f1674e59eaf8f4f75634e9b1daa7373c45794ca";
	iv = "c37858b04bb23ec911c5b735838daece";
	cipherText = "fe26cdbeda67dfa728a555f01c2656ca2cab449cc4f23e8a";
	plainText = "d3b09af91cb70a2a43e31eaf82577b0b85ae87aa717ccce9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 240;
	dataLen = 192;
	combinedKey = "cacaec051f95a7c4e36dfebdb6e82e6cf8795fc22a11693d0a8a1888a1044e1f";
	iv = "9c261063c5d09841e588f39921147ef6";
	cipherText = "50726a85d6f0ca268d31b9d1c8fc6c018e0820996f390d64";
	plainText = "98b57900106916f455f214a5fc8ed2430082edf8098db385";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 241;
	dataLen = 192;
	combinedKey = "43a9191ade36d59bf18802c17a20e65ad51319c1e83c2568f9482b13e64cdc12";
	iv = "3985a633b344de5073d9a7c175f07664";
	cipherText = "44a775f3c3cadecc8b59b35628e5bae002c63720ba51cae2";
	plainText = "b2cc4d1ce682d355f8dae9b82368df138a8230b3fa163e04";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 242;
	dataLen = 192;
	combinedKey = "92778b45c8959b8ab21ed47cffb1155e59f4c9b07e850a1848fc9720a0a5d296";
	iv = "7c6ec0a8593591213195f51731525467";
	cipherText = "6001c5c20b5904f8f3f411867bef18be681273aaa7ed71d3";
	plainText = "eb660b12f22105ed2f9a35d1ace168cda9ea5a29493cdb95";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 243;
	dataLen = 192;
	combinedKey = "1c2d0745b7fb5f0ab0b3939d33828c73d96c0297c3568eaa15e2af008fbd2445";
	iv = "c8e9cc14629de3ebca3ab6154eb954bf";
	cipherText = "55fd57df44b57ed1467fe346ea569df90f73467c74fa5b92";
	plainText = "0eabd66ed2a57751a7437bc2514a4499587d43b5a674111a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 244;
	dataLen = 192;
	combinedKey = "f1a310530249c9ab1b0f0fc22fda48916a483743f98986f58c1bb394cf011317";
	iv = "887bb0e8019bb2d94845d6dd6654a111";
	cipherText = "b0b96ef52b2de944223cdd67cfe88f25f9439fbf74c0feb7";
	plainText = "4898415890ed58ffebaac11a77aa4131f5d4f2ef2cc53e89";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 245;
	dataLen = 192;
	combinedKey = "42f42af2af3a7bef1d659b9b98621c570c40b455bdfb2488cbc3e43e2e5d8746";
	iv = "82f843af20b9c1b41734ce81546d2d91";
	cipherText = "e7ffde5eddce1cd4198f920abc150ad31f7ca43a553cffdf";
	plainText = "06e569fad67663648c73fc0a32b011733b0720199bdd699e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 246;
	dataLen = 192;
	combinedKey = "78bb511b69f34547cc4ed24b61aa36e11068f64ecb838b675bc47cc950d86f48";
	iv = "f8386166321ab37e82b52accd718c588";
	cipherText = "0e0a079c0bfc2013ccca26e62289d88ae27fe1d81e79f819";
	plainText = "8559b133f68dcd86802d57525482b0a8e6005f3f7496be9e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 247;
	dataLen = 192;
	combinedKey = "dc5b217fb7c25383a2f03b7e565b552e38e66db51664e7403874f46378f5a662";
	iv = "c0d0938a0b16bc4e9ccb1aff6e9966a8";
	cipherText = "40e898e3184fbdabbeb20890e9ce95837a9b67d7356c8b20";
	plainText = "09cb6973b8bdcedfa3fcf35d0bee7c32d16d08111938caf7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 248;
	dataLen = 192;
	combinedKey = "37179c0d92764a913480e0d53a7825181953603b26561c31f736e93846454670";
	iv = "a2c95a2863220c18a40cd56e7f78ceab";
	cipherText = "ea22c0666ff1faff1cbfd10abea78cc8d0f3fb9a9a6482c6";
	plainText = "99f90218cd547e659a61e9335dbde18ef70bcd04422f966a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 249;
	dataLen = 192;
	combinedKey = "856e5aa0a6dd055afadcbc5d10dc0d453fd8c0cbc54fc9c3e2c2b61928b798c1";
	iv = "6ec307371052c5b1b4497b285b21b747";
	cipherText = "99bf7d9d8c14cbbe40da985bea19ffac56686dbbd13ed03c";
	plainText = "fff6ae06631004dc6feb0e52ea3ae00ee5e2a8ec4e5d39a1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 250;
	dataLen = 192;
	combinedKey = "dbf9911d271a2a1542f5bbadeb571bb5e5ad99a19611161b992e598649c9ee04";
	iv = "e3b5ec8e45c6eea261a3fbff7f7ff7cf";
	cipherText = "c90f8656bcb48d9bd9cbd4b3aae38e6dea673fffe2feee49";
	plainText = "286cb853533920f3d7379ae11a3e5ada23f670c81dc859b1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 251;
	dataLen = 192;
	combinedKey = "3c6539f385db825be41043258f6eeaf7ac7f4908999827b1664cc58c8f6fb4ca";
	iv = "54a9863f9553e84bb6821df71c25798d";
	cipherText = "cef73e9b6d063c90d030f50ab48368dc9c8519e492774b1b";
	plainText = "1b23fb980036b6f566dd4db2debddf6257400de0087f1b33";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 252;
	dataLen = 192;
	combinedKey = "506f93545d1c81faefd57b18cb9c95fbb502f3afeb098cc55f5a8027d7d5fe8c";
	iv = "7031715008661093a4e5b7e8cf5dc393";
	cipherText = "833f6f65c8b76e495b012392db993b3c1202cabf0c90d548";
	plainText = "de10773dd5e352b4fdb76434154ea167ef946add19265015";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 253;
	dataLen = 192;
	combinedKey = "2d9c4c89acfb1f88528df9227e2fc51adb27ba345e0438b2c9821b981cfcfac5";
	iv = "a8042cb86ca51dcee1e2c6b239c72637";
	cipherText = "bccd74df15590f5a5504185a97f05d56fe618493bfe103c4";
	plainText = "814d8e3fdbf096ef0df5fa632d18cfbbbea939207232ffc3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 254;
	dataLen = 192;
	combinedKey = "c6e1c64b5bb778abb80bc40343f712f633ad4a562730fbdaaa20f85713ddb4ba";
	iv = "52d99b089aada7e51a6de7d6c4a52b15";
	cipherText = "7cdd89b32160d75a078a40403f2b1c92ed854a353c4a0266";
	plainText = "00477a06522cad6b79efd7186eaacbc3bf2b2612ca42ddf5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 255;
	dataLen = 192;
	combinedKey = "22ad05f2c2cc63ac85f20b3083e014a8394e2b2294004ca847ddea2d95f32447";
	iv = "9d7aa7e1e1c6279b13a9b23bbcb9d6bf";
	cipherText = "aabbf784e41c0c88752e37cfcc6d0be9344ff0faf783528f";
	plainText = "9c357d6f471aeea88a08ca4a0add2e15bfa8b7c088f1c74e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 256;
	dataLen = 192;
	combinedKey = "c0823428d658cd7a91611e14e066d5ad6f0afac02d385e1d15751301e8b70209";
	iv = "0f1b9112ccdcbc77ff9c8a71f33bd206";
	cipherText = "e797ea64a27866d6c57cd6eafaa0a5551a26d52fa73847b6";
	plainText = "148a148418e65f782a3bdd7216d4268dec835ca5ef7f0475";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 257;
	dataLen = 192;
	combinedKey = "bebfbabe51b549df2dbe119e5028bea7fbced030925eb1b67844081587d3d128";
	iv = "fc960cad2edf36aea23dfdf850848e73";
	cipherText = "095e21cfff97f2734dea0de39dfb76b6b65cec9da516d135";
	plainText = "e6d43c497c45b38e9e7e31c4ea1f097ed048d438e73e0fc8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 258;
	dataLen = 192;
	combinedKey = "e6f9d5c3d38b9706f43414d2cf921f043d2e46c50c751aff983d32640267f680";
	iv = "a092c52d6b04b1a326252addf0ddf156";
	cipherText = "0cf63270cbf86d3351f2c30b197de522e782bdfc06f1e29e";
	plainText = "cf8f35dc290578c0d49a89ce39c67856a522a81a697177b9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 259;
	dataLen = 192;
	combinedKey = "33cd832ec2f51643490442b8d5ad4a24d42a78129c30e634e517417967b7917d";
	iv = "d6f3990e1725873b66a1f61aa8ae3aa7";
	cipherText = "3fe3bd9606394007bbda7825991253bfffd6953ea630b449";
	plainText = "59f6a782ce2a52987cde47961c27d4fdd91e691d0b263e04";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 260;
	dataLen = 192;
	combinedKey = "3b6af083a86066303c5343fd6263ef8c86665ebc0a9f2f9ea6f0d33cf4fcaf22";
	iv = "ac6f4174022a04ea8c8807f2b2a5d5f6";
	cipherText = "b35b67f6929c29cc6e01e4f2fec50ef85b988550a4b1c2f6";
	plainText = "ba98718e2db244235e9b02b6937ac07e08806322e9dc2e8d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 261;
	dataLen = 192;
	combinedKey = "b18e73a4ffcd0ea9c37dfe9c5d261aaabca3a62ed8a6a327e1982067da2016ab";
	iv = "e4344bffcb34ef91bc0c01f0e966a12d";
	cipherText = "d15a6cee8b071079d8ceeebfc5390f6dfda6435fdf7a2e20";
	plainText = "b3c99c099d4b7dee3163cc5ef76b5424c0d574f7ddd5ac0c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 262;
	dataLen = 192;
	combinedKey = "f021915b3047ded9a662db5a3df4a96e522a672ad2bc12b1fbfb6926e6872df4";
	iv = "8dbcf8f2eb311de320481766b2f7abf0";
	cipherText = "27a9266024a46c7f49af9db030d340bcabb4cbd751521e16";
	plainText = "4ba8050a53fb9c0c30a6312edb0a771cbdf6c4947b4fa186";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 263;
	dataLen = 192;
	combinedKey = "8e672b1d706266ecee52427826b6ac5ea7974f8fdd748640482d4beb2d7827eb";
	iv = "6f1f207a6ebdc885b908d0dc05241391";
	cipherText = "8bf44fea0006587899af41a95951afb5816c69fc9e9f1104";
	plainText = "576f6f30962d0603442b448bd092a97f7f2cb7d49f13ef74";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 264;
	dataLen = 192;
	combinedKey = "25f18bb1cc6c3b253ccdeef499d324438078518d93e7d78bc21b0925dabceba0";
	iv = "057be1fdad15d66e95434227a25fd1d3";
	cipherText = "8c28ba992d7bcb4a1757ff44a17754daa08c5aa99cbef8a7";
	plainText = "4af6742586ff69a5e470f0eaeae054726e0afa6bf6a1291f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 265;
	dataLen = 192;
	combinedKey = "05604be5a0d856c825190c24199eed707a6f56a7546becbd4002e4681772db55";
	iv = "4cc1847c96f9b4a23cc88d03bef691b7";
	cipherText = "cd350f1a27d82b69a28522f72d93eb629c2a7ad5b330283d";
	plainText = "1c210d54daddd673c3e26843085184a22d48f63103f135a0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 266;
	dataLen = 192;
	combinedKey = "b7282d5a5dc6ddb6b1fb412cdbcd2bf2344d3d83d3942dcf029043ec3b49e583";
	iv = "346e63977068b94be364d03ec221ba56";
	cipherText = "639faead01efbc42491837c947930d0d4e7c515e3c918a72";
	plainText = "e23444f9c1d94209ae914630481e7669a2b6eedb8a37ce9d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 267;
	dataLen = 192;
	combinedKey = "1f53e3737ccc9733d314201d794c39cb3fff8fb05e972a684fb3d2fa41209d06";
	iv = "9318bdbf25f7f97542ff0330ed299daa";
	cipherText = "8f71acecf0859280ad385bff0f1b615199ecc56be832ea14";
	plainText = "1a8643c7808e37ef7bd403d00342d6941f8f35ab26f196bb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 268;
	dataLen = 192;
	combinedKey = "4ab37e0e3ea876b86dbd21c7a19e019e144812a103e341e83ace81376542f1d2";
	iv = "8629a70eff99f3ba7e7f13abf434aeea";
	cipherText = "4b55b90dccf8905a4e5d4f6231984345c6311125d527b7ba";
	plainText = "04a7e41ccbcf467ef71a825fd7e6bb29ae535fdec10fadbc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 269;
	dataLen = 192;
	combinedKey = "ab2c68c3ac2763d1a2efda6d8259d4a376d67ba9a9c14a349185071c7a9bfd24";
	iv = "13ce994afdc6500f462a29b0f0f46bfd";
	cipherText = "dd0075b1b26f86e4592b08015f552caba558453492676750";
	plainText = "2ecb4f7ed8e126d65018483eb61753c894e79a0d8c1a78cf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 270;
	dataLen = 192;
	combinedKey = "4319783ecc1e62b31da7eb15be41531fd322d66b1273a36e4112ac641590327f";
	iv = "d9da589806ec6793be675ec7061506a6";
	cipherText = "6b835e14371e0be2f7087877fc207d18f51d1bbbb33b78ea";
	plainText = "6a21d529f384fcd572b264fae57efa8710ac79cb01d87051";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 271;
	dataLen = 192;
	combinedKey = "a6d00774b77baaf33bbb771af277d350fcac3c1dfdb7d95b41cf1fee6f1c2005";
	iv = "d22b3c9809b3fa82e04891c628fb5d37";
	cipherText = "ad3a6eefc8e35bdc29c43cd956a11115d282d4a9562fc4dc";
	plainText = "ecae03c6709c405f04ceefbac46c774f99a05d4fb8176ada";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 272;
	dataLen = 192;
	combinedKey = "b93ef4998b8cdb33bf10819d1c9b169ddc58ae667cd164fab6b3a10bc695f7a2";
	iv = "f35dcd480274e972936ad7e96cf5b0a4";
	cipherText = "336d6d52cad7ce8ad6fba95ff4ab0448cc35d4b0a0c145fa";
	plainText = "77e8795efb97196e25f0b974a6d4d4f3bacd70aebb55eab8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 273;
	dataLen = 192;
	combinedKey = "2ef6ada69142746480e41d5c8493954f36856c6b4716c9372e009d78073d35d8";
	iv = "6d5226ff52d960c66f01b1c3891c49f9";
	cipherText = "4a1badc3e5db6bdd565cf06fe4283c92378bfbfa7510a644";
	plainText = "fbbbb4f0714c0e133e1a7283412ecefd3e6c990ac2b89259";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 274;
	dataLen = 192;
	combinedKey = "526bb360150a589ea0c78e3c5025763cfbf0d02d06d5fffdb21da24e3d63a0da";
	iv = "875bb9f444f5ebab3b7b7c3b076eb399";
	cipherText = "ddc2528026201652c98af5dee00ae1ae734cf53f26f71d25";
	plainText = "ca62f0b494926b4d2b527a766c3caf2c172614a95075d716";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 275;
	dataLen = 192;
	combinedKey = "6b4d093bf308c5699e8abbb2c9f8e302d71e66cc5016294115a4957246139afc";
	iv = "2a9ee71e5371293e7e891d8263c54bd3";
	cipherText = "4964d7464b4915d846b42e2d2a6cee9969ef5ca745e3ef2b";
	plainText = "9b9fc0c619eec605fb7dea62312cddbdef47618998999edb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 276;
	dataLen = 192;
	combinedKey = "3a25e50653f6b3c984f6626774e1ef8baa76ec4d88788fd1e34301a3779546d8";
	iv = "e3a11fc57cff9f098d54eb1f57f35328";
	cipherText = "6c94d84a1de1081a5334bcfa9e3b4a91b65b5c3ed5140d5c";
	plainText = "4821b000575b91c9217113a2ac1a68f10fdea52026b3ea3b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 277;
	dataLen = 192;
	combinedKey = "3c2261a5c8d6f3af03d49493860ea27ccceda8e8ef1cf0e6f02db08c6b533f5a";
	iv = "d7dbae835e5964861ca09f6741afd19a";
	cipherText = "9839972e4706f9beaec7ad67eb32a4bc0a6f7f95c88d07c2";
	plainText = "8707a1ba88df08a0e66635c1256478a8bef7660d0e6d6e29";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 278;
	dataLen = 192;
	combinedKey = "5bb878f9dac5cabd8165707aefdca901bd5ee221f315c225578fe53f21406f60";
	iv = "1a63d98e8d41309dd23e4d4ffd82c08f";
	cipherText = "a8fe6a1ca14a21661acd6440f78a155e93c68d290b19ed59";
	plainText = "50de80ee005e6c965bc0f4e06992fa7412ebdec7bce0b4ad";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 279;
	dataLen = 192;
	combinedKey = "8fe2bade3d6a3be15def160d78716e1e2b5384ab524437343341b7c49b09e503";
	iv = "7b9fbd958b0a4ebaac966b05de9de33c";
	cipherText = "b8e07dcecdb738fcd01eec36389e05eb581f800005ce7277";
	plainText = "60dbdf61fd4e8c39cecf2d4567dcbe906215bb4ac55f340d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 280;
	dataLen = 192;
	combinedKey = "bf908df02fbd434b64f6a2a06905f2c50b585bfbcc9869259137a015cb4ff0fe";
	iv = "a5971fb40dbdbc72dd4584ab2226b5b8";
	cipherText = "6aaed421014cf4fb0eaffa1a73e9ffa03202145e1349d8e4";
	plainText = "299771630de17bc6ca5b4a11b52c4e2a9211f3d48317cfc2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 281;
	dataLen = 192;
	combinedKey = "795f194094d6313a3096df84652f4997e4bc2e5dcfadc48db2df0e1d9d83794d";
	iv = "859d13fe4589c239e7a9c941581b9a86";
	cipherText = "0d20326d0d42dbb94a82e0f2227e013d7baa927fd6fead02";
	plainText = "f9ee01b670413e68561be53381b12ba0331c951d35320a3b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 282;
	dataLen = 192;
	combinedKey = "53dc82c517ee150447e4b8e4aae03c944a7f526ca7eaa46b0c6286edeea0b823";
	iv = "f6882d687f62f9469681025cb3393aff";
	cipherText = "f5026e6b88870cf63cef54039f8cfab79f38197bcb3d4400";
	plainText = "6eab0616182d1814d3664f4a6d08aeba7f9994dcf7393efb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 283;
	dataLen = 192;
	combinedKey = "8b7eeeb7a8174106c6b722bae9c15b6c02522b1002e4319c75b53cb3fbe4b2c9";
	iv = "56e3d925a3eb658c40d45a720737f521";
	cipherText = "400d7265bef4847a4f22799a4eddbd03a9ba853774009b32";
	plainText = "6110bb87f2c04fe3a6a27f72d69f4da7e1d59e3f3a41776a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 284;
	dataLen = 192;
	combinedKey = "50c967f2a28b9b8731caee3e3cd541804027aac598bdfe5d942aaa9b5ceed77c";
	iv = "1d69becf315931c19ce07e98e1213490";
	cipherText = "cfd1b4810ac8e1007e562490cc54d112af44880ff11d2607";
	plainText = "3b41ef3643412ce78d8f48a389458704c9921a4e426ad101";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 285;
	dataLen = 192;
	combinedKey = "81803f16b11117fe795391d257913ffa15ee6d7c7d7ad0ec220b87b0d3985c84";
	iv = "263fd129a2280686c7d020b8c362b579";
	cipherText = "80fa8d368ac6b54aa8fc7b02c4f47478e0731c130e0f67b7";
	plainText = "c677bc916b9b4918fea99fa323d855d2fe9c8554fa2ae6f0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 286;
	dataLen = 192;
	combinedKey = "d46717db33a711c0f83dce005a81119b256f18eb40ed386a1324ee0d045ee218";
	iv = "72fd3a84c389a7b6a7964857f0d27909";
	cipherText = "ae62f9d79da3c125d3cf129dfb9a24d0775396453a3d114e";
	plainText = "314b52cb70fc08b6e00e3329b423ab8220f4abe97a8ec66b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 287;
	dataLen = 192;
	combinedKey = "b874a4b4e1c89318a1378f252201e6071ac1f29cc51bdd0cf07e3837dba3b599";
	iv = "d4f2c48d2e6d2784e4553338a9a116ae";
	cipherText = "8a4b5fb8213d4fa10d880a6beca0fe02c222b18e36016098";
	plainText = "f62ad113c5119e362d9cd0c3db311f861c21306206b43105";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 288;
	dataLen = 192;
	combinedKey = "cdb34ebd3237a0f6a8f6fe8f597d096f40b28ff91537d8ec5b3028fccafc751e";
	iv = "cfe5ecd6cfdd68c6232dd507cc675c89";
	cipherText = "3182b46434a398e0d97bcbd3db0e67ea9eb4c189762c523c";
	plainText = "1c215f425d19bd155350a7b11e42629759d1d179f6924b64";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 289;
	dataLen = 192;
	combinedKey = "8da4b1757f9df68bb6649d8120a9a8a9e73d4feefc2d4604a197e8c7b7fc96ff";
	iv = "a243617f7975e2bd76a3dc2eac1fb10a";
	cipherText = "25c2899afbfdcbad01bcc6a3c0fd092e5e26d484ba82e043";
	plainText = "f5aba9675c3d634c8f4c25f6a568eb488e8822fd4b1390b5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 290;
	dataLen = 192;
	combinedKey = "fd9cab477e7da587d87f0140916c31438911e1f0b71559b2138cdc3c214f8f5b";
	iv = "e60797b45543aadccdb15dc4666dc01c";
	cipherText = "d38695569224e856ea32f643538690c2e26c74a12d774d3a";
	plainText = "f7dcdb3f80b938e058951f067b9ecbb952c8d0912eb5f2d0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 291;
	dataLen = 192;
	combinedKey = "3de3c8398322dc3cc1f2089a3d41deaca4c22f5b07f34619a7c9c6944228aaa7";
	iv = "d835925649886d37e54350e39db941d6";
	cipherText = "6990ac32da156ef05edfec2ece17d76632c007309dbec7f0";
	plainText = "a00a3486f8de88e2d742171ff37eb7faef4358ac3de6d5fc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 292;
	dataLen = 192;
	combinedKey = "85201be1944dd15245b28ac29667fae1ce4c75f7d87e35ebdc2a99251bc8cdc5";
	iv = "8b22a3c15383c833154c1f5871e72a91";
	cipherText = "5d4f04b881cc35bbcadb0349bc3cdb0c51714972961c0959";
	plainText = "7b58d9117c0462b236d02b0d7d88510eeeeffdd21f1b5c55";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 293;
	dataLen = 192;
	combinedKey = "3ce73e6e6cc9779e5db3e4186fdfbfcd55be742d0c5348731d18a51352aa555f";
	iv = "6c6e7250869f27dfaa99d2cfff348690";
	cipherText = "efaa81fd7505fffcb3b935c5c5c186daed5593c485b79cd4";
	plainText = "4e56c9a161dbf153d8fab1aae1a47f2de701fa1e4c4a022a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 294;
	dataLen = 192;
	combinedKey = "b0dcc402fada69896e4695b9e8099d78a33ac610b56e789a769c84fa3edc6168";
	iv = "5b8a86f776218a55ab82b72c4d3b298a";
	cipherText = "cf8b9adb920b412765d92912805e0a6288fbd898a7e74815";
	plainText = "e5bc70cc2cd0084c2c5fece9dcf7256bfcd0dec3d0ad17cb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 295;
	dataLen = 192;
	combinedKey = "2e025dbed06870b25f584705c71ef8a0acfd5efb83cb1af14cfe396325cfeda9";
	iv = "8375d05b66cee52a7da31135e086c4d5";
	cipherText = "bda725302abf13a0c478f0a9053a904cc02aacfeac74afc3";
	plainText = "8d923510a8db3db577c20054f03520db6f6c834f39077a1e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 296;
	dataLen = 192;
	combinedKey = "8be81b4cc24125e8f9f061abbd9524fae28c15504a9c2e74b605df23c3b74e2d";
	iv = "f842f8de268e27d6f16ba9d83cc79ecd";
	cipherText = "5af7fd98966967ef68b0df75f0a08f1efe91b0656263eb70";
	plainText = "05a953528696013cea5076908ad754a16be9bca565290190";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 297;
	dataLen = 192;
	combinedKey = "3dbf8db94912fbc2ad7154cf27ae417d414cf15d7b288370e967f8c2e13e6a89";
	iv = "495c1427487676c45b9b55b21e63953d";
	cipherText = "b58101f5d6ded0391409e6bd56d2d851b66e4c54d241d32c";
	plainText = "12b828f0f4554ef7d730f69b89b0bf048d83c475c525a44e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 298;
	dataLen = 192;
	combinedKey = "f061c5333c3b8a50586bb14f0fe3eee3bed44fbb4fefd4c0708c745be296e0bb";
	iv = "a992611806407b927de9607e8af35973";
	cipherText = "a5dd2cb2f2f88da6c6d733158861275da9f2e7602c1059d9";
	plainText = "837a76efc7fa2764e31839fdc0918c4d66bd65662e041066";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 299;
	dataLen = 192;
	combinedKey = "5c850ebada3491074a4a2c66e9c07ab1e045ec46a1302b38cba0bf5b783fd3e3";
	iv = "ab8c31d1e3eebb17216386c68f12db8c";
	cipherText = "eba7c9532843f94077711f80620f9be4c408b0fbf592cf88";
	plainText = "3179cfa3983a0ba0881926f60eab26cfe0606d536c0e62c6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 300;
	dataLen = 192;
	combinedKey = "aa30a1cd55cee59d725e92454f2a60ad311b0a84c21052fff288ef1111bd46e0";
	iv = "5952b74c1516d2e18cc2c66c81a82b7e";
	cipherText = "5931a9cafd4a99efd1e1add98f97a6a797ec360c8dccd34e";
	plainText = "95179e8bbd1c532e90c768f12975027ce3715d830de6002a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 301;
	dataLen = 576;
	combinedKey = "bcf42fb904c73bc69001aa68cac17057151c2a17127a2b0a7b5eefb7788c9998";
	iv = "5b06a729f8e7f0e7bf10cd505f37894c";
	cipherText = "329312ed259daf871a80bb617d788496ae4f9f6014fa2956d8fcaf5ebb1bb4c6cfa3a789cbb03272018b70d802282e84c023860bbe570bb4a2f85f3f57fe2ec0c5367a6256b40e8b";
	plainText = "f599ddeffd6743cfa0fafdd219007d926e1e165131e98e54bb049dfcf7b752911b059a415e6dc151630141424eaf41ef6e5cf0c68f65604b27f58e743ff5a83ec037bfdf931b575e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 302;
	dataLen = 576;
	combinedKey = "77ea2981e2a36a0453d4262a03b5f386018f8045e5814f24987f53ede7673427";
	iv = "2d0b2b82176700ce9b87e8932b41730a";
	cipherText = "41371eb8843db8f78bc7063d4f3d9010d3b8684c510574291b359b35e8d059c733d7780726770a4549feb874915d1d75390c7646689d99be99555903e568c1b1f77c101ff81ab756";
	plainText = "6f48c6f5edfc08d666fa7b6e226e1619a4b8b1b5a5552d4fae9dab207e28c67a2c5c7c3fbafa1f8dfaa4f58f32f4dfdebe68e258a39ae61ee3c647ab22c21c091bf7a913c48d6802";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 303;
	dataLen = 576;
	combinedKey = "e929edf18d457a59c3a3315733b02b0bfd76921fb11f39109688659f8029616a";
	iv = "2a6bf2be5e5c009e318cfadaf130689a";
	cipherText = "76e179b3ef398020ba3579bd000e5c3d65a62c8964778203e77d5f64954f6933e2af5dc100cd72c9c4f810265ba6d543d8658a1e721639f2615c46cff90a682bfafd14513e43fc58";
	plainText = "387e19aa685c6c4469b0260f03ff80853525d036a26fc3c68a678dd49c70a2c251edfb94dbcdc6e810fdae4159ccad49b43d09a22c16f021369c526b1ada0f4a3c5b3943324adf24";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 304;
	dataLen = 576;
	combinedKey = "0f4896fe264b57fb7d28265eb12cf01227dbe5ccf96085b022c7caa1c38a29e2";
	iv = "8e771908edcacf29b7d135a145ecf1f8";
	cipherText = "35ad466405071005400688897222cb7d3bc63d6b120559ec936cbf3e745928048c7ca815a7f051548d1f78e7a47e31fea7e88bf8ec8278bc76f20aedd9be3318d6852a7f7a5b2483";
	plainText = "350228b649cdb6e4c79681f92ffd2d702747b2714b959b7851f02ad2b71d8c2d4d7ab861e347af5ba4483cef6dc57ff981de6e90b24d01fd694d722fe7ee2c727aee2a00c081b1a3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 305;
	dataLen = 576;
	combinedKey = "c9d027b3762ddc063d0d19968cdaa28a4b73a140bab5a991133e2ebdad53b23e";
	iv = "a0b55d83accbedf9977cd1d73689c1ca";
	cipherText = "fc28e6e172c6f4528746429634db328a3a7376f3bef73228c5578a8135de61986e165f2d63ef2a73e7759a008b403eab71be5aee213d8cb6b38d2c95f6e9191bbb8000162f948c9f";
	plainText = "931d74b14304b7b1c3b5dc4033a24782ec14e08b107c1220f88f42a13a3d3dca7fa429635e71abe39dabb646d3ad121eea5989de79771cd61ad1f58227adc25cd5d4700d073646b0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 306;
	dataLen = 576;
	combinedKey = "8b23704b4b330fc9cc3934d4d993e88e6ad934da7d34a0b724dd1f8ded11e04f";
	iv = "a3cda7a6faa3ee0353db67548bef80d6";
	cipherText = "09403676929cf4cb52f5e2f7374a975182f10e2ebb1cebfa0e4d2f93ff5cd7d7c63c332361af48c69bd7d047b12cb192cf52b7b723e2c245188f4855cee23e7dd19640b8c658e599";
	plainText = "658e5227436713dda77295d31e6b09bf49a4cbdfbcb946ef7742e3d847827b5a5d4fe3e1ebc7f8cd2064af82b185186f7439c07832b38f8fbee4a331cd5e3e7a98d0440d80d32a42";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 307;
	dataLen = 576;
	combinedKey = "bccdd0d3abb476a932a5eb0d3cb18929d2ffaf5ff6aa27036e9329c3044f7518";
	iv = "9a742db1db3583e45a21cbfa824f51d5";
	cipherText = "471eaa265f9b0645665bb4cfb1c8c4edc31a329f102869e71ee5e82aea0238f5618301fac2ad0bd5bd31299c9e981bb5262dedd640a6451c438243223c3668cb44f3973f0d286793";
	plainText = "6ce12ba0973c798606e74f0155169423189dc79aef91ad534425fd4353b3c1dea154b1d58f48de2a6bc2bac3da483934319a380dee5e7b18d7b84a525717e2900c28cedad47479b2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 308;
	dataLen = 576;
	combinedKey = "ef75c1adea471dcf8cadec5ab88f764aad7d9acd24c42b60edd5e33c1d2c823b";
	iv = "818c9a65126be4d024a31945db64292c";
	cipherText = "3a9ebad7f73e9fbcf7527c04b6d0261f8efafc2e313e9c951843219b4484920f637eaa16aeea2bcebdb3a4f53f53bce8dfc67cccd941a8c037149a9467a7c58e36e44e3da7b8d566";
	plainText = "78427450f9a5c3d08e6755f69125820bae11e0ae734248a9c3bcc7b63ec79d481d5cc7ab25e420ec2447b1560eadb95909624de8e4993c06256be0ac262d15b889797c79bc96c80a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 309;
	dataLen = 576;
	combinedKey = "987ff0d9ead572bb969edeb0d2449639828facd94cbb8a7c88f2815b491f5e74";
	iv = "cd6f1e7bc89c6e173a4e84b63d5b381b";
	cipherText = "180022a51f3208f39fefff06965f6e9e6425d9ab251d83751b70753b7ad76aa6b5f8e435097a58fbad171054197c9c1527b664def4020897d8161a39d9b4fdaccd7e56b457beeeea";
	plainText = "c4cbc6ca953a09814c220d95d68267ffd6e3c14e84dd7f637f48afcd1a8fd7dfb56c1ccf26887eb794a19e7186c17028d0f7fb50982a031cf46813122c8fc6e064a550b993e43376";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 310;
	dataLen = 576;
	combinedKey = "30790c9dfeafa4eefb0f3d261fef5b963c9d88ce55766dfbc29403aea8b76c4e";
	iv = "24eefb2590c5860b4cc5205b07d6ae77";
	cipherText = "9f67126914bb26d677652710a9d404b7c7ebffe06f0e742029dbe6c6b2f05da7892b265c0286be400698b5d91bfab6efe67c6bd4e9d89e4ed2dcb71f5c3ff822fdd5176a5a31bcb1";
	plainText = "967b392d23dffb144e33e7ef791dd3438604eeefd87b613260ae219b07d773e3632c9dd017276e7cbe2c1e7827c2f28e2f4811cd825f434a0e2defa762379bce49356139a5e81822";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 311;
	dataLen = 576;
	combinedKey = "0dafc7fdb1814bd861724164baaa5d8843c9d8b7e9c895fb81c57319d5bad32e";
	iv = "a3afc7df23facf928958504ff985c8d8";
	cipherText = "055936d43399b6b0fcacb10c5cafa2e72cc8d109fdce1c4e3ffe80e47ffee5af7a2477e3f3bb991ef7896fcf46154e1f19fa30c780060624d423440533de41b8e6303b20e5387a4b";
	plainText = "e1f4c55fbd480ed648f31da9649061ad318d95920f0c6913fde69208702037e02eb0073d3ffd84998570c2631059e12ab62252aa7befcc66035941e89d5ec3882e5c162d987bdb5c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 312;
	dataLen = 576;
	combinedKey = "922246584dcc4f39a3c285fccbf8e5ddc6d029c69af77f88615663dc50efffe0";
	iv = "9320bf151ec21b8a8c76da7d544fb456";
	cipherText = "ee844134097c2787e2d12f65e63909f464b8471569947a22e0ea814b96b22e7c42e971803c256120f6121054c7d926de9e717ea69d48a951d64396169318f575787b1f6392f31fe4";
	plainText = "f3534512f03438b8c9e1a2abe65b8c4be8a66d899541e45a8ba9a663bfa21ecbfd12ff7bf29c358ab4015d6dba2a33502d6cafdacf425a865c547efe34ae3d30d872fc14107c52a1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 313;
	dataLen = 576;
	combinedKey = "74cb0bc610ed5c8bce1f67aac0589d4ff08d88928ed1400d62c4fab975b60172";
	iv = "209f05f227954594c6851bb7fd36cd5d";
	cipherText = "472687bd8e3ba91c5a5ff6225606adc417bdacdebfdc3aa1dda084aeb1a4af845c5250e70ae0faa272defb819f76b0aa17ed3adf9ca1f8ab4c867f0dda0195d4ef8873485d31bece";
	plainText = "5e47e7fe500c034b06325bebeeacf7c33bcf5b626af6c669bc94ef2fa13e09a57e24a1f85d82a7172acf9b6577fbb3904093bbb8f9e6838af04264fc4d39f5e0198aa155818861aa";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 314;
	dataLen = 576;
	combinedKey = "c2a844a8df93f8e3342682b4a7849f720cc43cbf92ef4f743ed11c8820248def";
	iv = "a0235eba43d8cdeafb5e2590285f14bf";
	cipherText = "2db824bce5ee05a7e62fbe1bca3d3b5e5771a9f247ee1a570f3a0efa2419069c4d8ddccc927e85b0c05dfa99efdff618394d817bd4ec09c9373ffc56a24220fa20385a34c6131689";
	plainText = "b7045bc70cffc7df5776f0130d5e779a3f4fdcc01c55874e1c2080458c0661a299dc6e039b2ed438d4b4c133eeb70be2a7610984d1e5bfa3c2be7e46f0144fab177e5944bf1ba062";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 315;
	dataLen = 576;
	combinedKey = "f4cd29ade21ed77a9eb4038377cd5f382abdb823ece445d4d5783061ca3e4114";
	iv = "5b8aa5f68fb10843f18e458ae67fbd3f";
	cipherText = "b006b51ce0d8474289bc14793fb0092026b71d5f16355ac54d995b1adeba927a35c65f85e3ea11fac433e13b2dc207d0ee6804f6062f793c0fa4198faa5558c60530fd6ba0d34d71";
	plainText = "1eedf0f151e604a80dd381825b06818b52e49e4516c358400a28ab960f6f1724ad85ec51714918db741327b9e1ad3041028b6fa5f7a01f998dc37295a696a95360f13e39575fa26c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 316;
	dataLen = 576;
	combinedKey = "6129a41b7c242b0ea263b469b5b78f1922674c5b64f16979225530e3882fe657";
	iv = "58b13c78609738bd451a77bf07d021d8";
	cipherText = "aec0dbfa8c1ce986d0e57bb4ea6b7004ca69ab22aa9f4782aba18c65a311533b2d6bc56daf2d891531eea6465074a53e58e897c0cbf2de1b691f70e07b39af2f197b5100a0d9a9dd";
	plainText = "0862b6098761d9843850837dccf11b0e86f5c466ebc3524b86ddfd9ee79e1cb5ab793c91d5ed59def3a28fddb76da9f306625d0419e4caa54da33781ca1ea6ce5ab32010af5f1158";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 317;
	dataLen = 576;
	combinedKey = "c401cbb2f5135de6266f0504a6d8bed99ff905e07f8630c112eb21badd2535d1";
	iv = "7231e28c8e371c799408e18b1da8250a";
	cipherText = "d5f78d7bd717e3504ae5d8bff6aba1a78a9e211359402d0ffa4e49d369769fef09c147b49fd6652f6b3fd5165330762ef02f0c1b13dbccadc6ffb7e997b8b5fa0e7bd8c029676856";
	plainText = "68762b8880a9f51a414dd8362a45fea5bd2383057f654c08f86194148b566f7a581371fcaa0d155144a7ed1d215276b11a5800a6b177f7bc7665675814b93527350920c73c536b3c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 318;
	dataLen = 576;
	combinedKey = "eb0f132a53b50c5e7587924cc267e34b087ef074361aed191db400974e9a1260";
	iv = "cd79046e51d782ddb8d8a2249fd9a052";
	cipherText = "846825caa2880ab8b59f50427962790ef3d6a61b3d1ae2bf9c52255d985f6431104e8efbfa12e138e0adf37face517715c8af74d4452f193381249ce7f3bf0650aec4e396e9f91a3";
	plainText = "cdf48fd5912ed7b1b0e9b45abf4ac6b25cd33ee0e987289a8ee88ae112db3d2ce7521abcca0191f79bde522048a2d76ca9628fa92f9e0b435bcacaff1c826541f4c0b77381d2a55e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 319;
	dataLen = 576;
	combinedKey = "461f7a2e70ca2c3a810f07f647ea50bed57c526d49e54d5b8372c6b08df61036";
	iv = "46bc8e6c71b5a832ce2bf37547772884";
	cipherText = "a4cf1d9d26d4eaa781cb0c049bdee7e850090130738601e4339a58403699b5663ffb7640812f9ded0e9db3ab31048b08b21e59dd1fd3991f5084b791f81ba60bf3d1338d3ca18068";
	plainText = "5e1a3272f3151a5d888430df32a52ec84cb330ed3c0eec971c6e648aaa83f32a079318c08370f04092992af5b4600d05dc2f757953ade96bf3a3794211a673fd23107cd74f922000";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 320;
	dataLen = 576;
	combinedKey = "708f3cec902d4009a850f44a56279b85228c7166f446369404bf3c97827f2344";
	iv = "ec7c0b9e337c85a4406ee073ea77e1ec";
	cipherText = "90b74ef194159e6ff81e6d84e2156de4763f8cb876d031dc66150ce260b1b8085c89766dfb02b7bed53de281db2b73e0a957e44648856f0f4e10af0c0ba1dbdb5ec6d5f33eb55e2c";
	plainText = "9ceb02f9e25207dc913d2eee0beacabc29277c68bd2bf8350b52b021e60d58860967198e0afea8d923b59a2febd0197c34eebcd4f71b4b6aed873e675198df902c88e43f7dde3b5d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 321;
	dataLen = 576;
	combinedKey = "b6f89c093a57ea199a0def1c5c8c70a0316ed35ce046983f1490fbd1801d68de";
	iv = "88bb7332d09af233158dc8d7811e07dd";
	cipherText = "1498ec1e5248763eb5bfce9c29c0699ef5ffad13f90eccfc5092bffd00032551c4368f0b0277ebc8115314619acb8f68e685f478419d1188e4c666046cb3ebcab730cfb2e4f339f5";
	plainText = "d1d2b06218ae7c450943e8f63326230afe8f0d87d96508db9a4ddb771965c22b650a0d9268d3938883e9d470c8cfcdb6f3eaaa8010c45a4aca81f227b0af49d35333f4cdc1c5a78f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 322;
	dataLen = 576;
	combinedKey = "cfb1cf5c31dec9d063a271af0680ebeaff2e70f0befa4674b93a8e1580d478d2";
	iv = "f7371176e9c06a0aad8355520d52942a";
	cipherText = "96b5a723a8851a90066711a7e8817eae615dfc45088eb4f7ea34d5fb07c88ad5c31f49913b38466550c80a0813dc56ee2dad2139821ba19a12b33a89388cbae825566988d52eac74";
	plainText = "85328a032a648aea45b04523f3a0f370a1904adc503a8c1949c47b6fe08d8af3ad89d9e8fde9445506f5efe180e30fd2f34371c33c346d939e21177ab8bfc7dcba44a75a995c678d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 323;
	dataLen = 576;
	combinedKey = "9fbd2fb54a9c59649602b8320fb21871b464d209f93e6376e27f1f2fff4c130a";
	iv = "34657fe6a14d4fd51a119cec1e248193";
	cipherText = "01222a1e952a578d35ddb068cd1987ea4022801f59c70103a3671c3798ab7b60fbafedf31b3c24eb1c4dfb3603ff9329039c37ff0d1b9f8aad46a470c297a8e62e58e6c7558ab808";
	plainText = "e298da15aeda25d76fb3524fb289ced3713ba2241d88dd512b2c0c135849cb311390f78fb8210525edd053949367e6914d4f7aff2b282a2a445221aeb6073d2f1b821a0f7201923d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 324;
	dataLen = 576;
	combinedKey = "0b192489c37be705688424fd1b42df1c231fc8c9b25fea29fb15dab134ae0b7e";
	iv = "6a06aa22da12e1cf0c495b74c795adc5";
	cipherText = "4ed24ce17dc32d2c7de69ac604972264bdc62555d7097229c48c845ca6823e631ceeed20d615d5dffd5f577db16f08964784d8df543f6b43d121911fd7624891da4a16f0ed1e8ebf";
	plainText = "00aba79df02c8ca6bb691bb99dfc2729828c8aaf5b6dd5a480811b8502e4c760960504552d2eb1786871966a6a91cba9611cb8af64760d0f616f23f6f64b94747be6f66742b8920f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 325;
	dataLen = 576;
	combinedKey = "1c8e490cf9cf800b12de03a9a5c44a23ea14cc273aa435fa9d1e854758babc35";
	iv = "095ce0316952957febfba48e56ac3c64";
	cipherText = "d53b1ab743b1f02704f0170295d7d95f687a9b6ac1a6dde66437345513c7ec106af4b16531cef4f8e879e538781a61ddb3f1fa6270c09fa9a8067865766e55d3e58120e2687d1f78";
	plainText = "33d6de76b76a78b9e8d78358ed84c038eeb818371da66a825a4f7261b6a211ed012505ce606fc87a39a02b462fe6262fc1bee680dec015c0c2f6696b1fb87490ea7224e29fa376e3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 326;
	dataLen = 576;
	combinedKey = "cd07d899111994023741df55ed3f550b28343259107bf38e17f931e19a201385";
	iv = "ae96cd17a80784170ff557637ad0f35f";
	cipherText = "c8ff167e812db113a1de71fcf4251257e82e248c0a31dc1fde5fe7c526e075787a8ec5b0c6139f7f33144c4ba41c66999fa28a5148bb40ac23edf67416de777f0400893d996bed5f";
	plainText = "374e77db4f145a5329dcb2b5fea69d06d02a51aa6dd6a9f2a3d9ae17aad0dbe5ebbba4be854221d1f51221529c7db2629ac61e57744e4b23f59045720b6021b9fd76f429377b2b80";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 327;
	dataLen = 576;
	combinedKey = "aa512d7463817dbdea1c27a7dc71d51cf26cba9f9d71b53d81b9e380bf274129";
	iv = "a72a6a5baf35a24a4dc5ce4afe7ff346";
	cipherText = "186f78d0b25650952941963190b81d1d55c2f8ae95e06eb02781f037d8b31c98349ef229c7419ff35a232e5384d68615f78fb604c52813eb7d73b16da50e3282d41f883c173d65ee";
	plainText = "b80665b2170d8e1d8888ca824602e05ed9ab2b23ec398b63941c697b303fdca877d4c30fd0a57a017ea6069d3060b95917fe04d437fd3f497b714481bdade9bc6d21a609162a56a8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 328;
	dataLen = 576;
	combinedKey = "0b5fa72cc6ee1c72144952698752953187e17743ffd471e558c5207f1db5d1d5";
	iv = "792d2b7143908e1924fe250aa20aae92";
	cipherText = "b4a1baa0eba4d38fae8aaeb37b29cf91d7c29f1b37bbd872c0126711273a2a164480065d79f8bb4c82dd9c18769ece01111662691cfc130146fdc2d277e2889efb095fc9fe05c146";
	plainText = "544a9f53379bfd4ba26b082e738cdf0c9cbb8de4d2fce999e839ba24f1ac536cfb177dd8e28af765345e047d4566eef8a525221709974c869743d1928cbcabcec0b49d160bdc9322";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 329;
	dataLen = 576;
	combinedKey = "a26cf061bdf852c68c2ce8df58de1c97bbcaac0c2e695ccaa2b12a7a16f32078";
	iv = "4c90cdd79e2b7cf6ad89c4c0ffe4aa76";
	cipherText = "9fa14f9f5e07214d9433f98a7d938d8e77be629e66b078f055a382369b8b1a8773bedc4ffe825902918dc1e88f63d5165662d1c9fb10dfe2e621c1a60a8f486fe898a0fb9c26d019";
	plainText = "20e97f66e0cb78af5f862682bd2a99e90170e309a733cc7254eb4d9785d23df61f6ce814dd5ae74d989f46955c126dd3abe7152f5b31b269df1bc7b338a5a31f978685498182f5a2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 330;
	dataLen = 576;
	combinedKey = "d8162252b2f2d732987d9bf168dc50bf93eb976c8cc4ce0a1e5c508ca8e99e91";
	iv = "c7ea1ab2e0ab0982127bc325f92b802f";
	cipherText = "fc825e8f5391ce9e40ca4e264f1df5074ccb5b2ebdefad3871b92c6d485095874d1ecfc7693ee33b39e621b905e3ef9084946c690553268a4b37d50eca3cf530d702dab720236653";
	plainText = "0a89a528022a8c71a3205abdb7c48b5231b82f826b599c329f68452b26c83915b68af93d6914cec30a2dd4b9ebf6269578284cb55faa68ae1ff071e9c7e7c00fe7ddcaed7b1811d1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 331;
	dataLen = 576;
	combinedKey = "76559b79421a91341e97b65717dbcfe7e60dee61cdc40a22b1e1ba60ea617681";
	iv = "ae51e6f0e5e1b67b6fafe8b8d7c1bb5d";
	cipherText = "5984476db6d231de5966220ccd8a7ad07703cbc3524b0db1c6f62e1767b7fb3bd459065396fd05d1e7c58a54c840a5754891f67c37cd7db2a4842a225c0bd6de14b138a6b8a47405";
	plainText = "2199c5447746de3dccc641ca1efe97517671f4e725d3fdd9e20d263d61f38157ac4d37dbd710bafe092848d1425adc9109fc731af9b1ff83db32176ddf2226cbbd5903381c3b3b9f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 332;
	dataLen = 576;
	combinedKey = "1e6ee76f10f64fa57df128e4abf850a2beae14e909e4565da4312c1ee28379f6";
	iv = "16117f0260fe03569023419384efadd3";
	cipherText = "da56b2088e17524bd9a41463540fac59774611ed168c70e559f8a32e9f78e7efc7c44759c62bef0cc9a1ab35252c5d75ce9d71fb130c15a523be1f71750dac5db9a4f55d93dad925";
	plainText = "940631b7cf794dfdc093427d9b42899cf735ee372f37c0bf200e6260aa1003c0b93578559b35d632b558c52f334aea02ca1960627c8a3dbeff99f06025716ff5fce37a32b4e62ca4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 333;
	dataLen = 576;
	combinedKey = "6fca6e665f1093e50cf4b3e414b96c42c788aae069b8cf553d8092c2ea5df3a3";
	iv = "5b309c8a242e71f42b203a86b71fa728";
	cipherText = "824ab6e7ce76fd217cd00a9f9de6ee23d8fc3c9b4ab62c1798c02241f6b857624b361deb9ed458ce507838d9d14cb55d7084129b8be3198d5881fae8887e5b5b35a3e4a2932b1f62";
	plainText = "ede565bda675340d2247826c38676e6eb127a05f91a2851244c89cc72f1b2dfbf777cfad379b32abb43d0d7067cba2f23af19e0ec1d723a81c01c42014fd78b902038b58e057dcf7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 334;
	dataLen = 576;
	combinedKey = "4ff83253b3de4e08ad805aeca7da442d132027c8e79a1f2960706c799f862f50";
	iv = "60b5312cf75e96be81a76a42eaf31a7b";
	cipherText = "586ae5331aa9df84501b291a1a32923860502e0e5f0aceb11014834514cf168bc8cf959ab61b9a7f616d6d2dc0d3b3b6b43cfb3803a5f5a04114812c8a754104dae7e062d109548d";
	plainText = "554d21e5a2d20d8371b6b83fbb0c346f894bd122efc53cebf2c2a5d26332bd38e0d3205a0aeb3013403d1c43cfe73af0e76ef50108e730be83b04935a362961a7050fae3c00a7616";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 335;
	dataLen = 576;
	combinedKey = "6803dbff60f18e5c404b234991f7602a045f9d633548176a2ff8a9c615941c1a";
	iv = "86c2e225c2fe46e3aa819d0cd4f9d423";
	cipherText = "e3cfa5f89f599cc476459c8af482054be07942bbf052350eee1c4e6532a1ac8cbe613582efd1bd305bf3d270957d6365ca024ebc2bbea773fecf908830b168fca9227ece08b2e489";
	plainText = "3d075bbf28614280d84884d87de7a984ba1fa0e963a0e380014237210d50f3eb91ede821c0bf0d3ff6dc7f064b93e189b14c23be2cd74395dfb89c4ba43c9ed487c2f814a22cb632";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 336;
	dataLen = 576;
	combinedKey = "0e383214c6ef9958bcb1e9eb3bdf56fce75b4398e10eced7be30a8b83d57ca30";
	iv = "f4ad594bb1407a9602d894116881e79c";
	cipherText = "eb387a3bb2c585d1e5cd3d7ef6fcd9c3ad867573c55aef56230ca853f3aaa14f27621a9668f62fe650a3d3ddff62fc07608db1edc81df3ebf6bda7d71fce30088c97b5517342d186";
	plainText = "d106a81994d2e3c0f04ed30b81b6a4068f4a4021a46617b0820116725092515e1c2577c9056948229356ae1c078caffbb002ebdef815dbf213a3bcd54b95b3ac91ffb5e5904487f0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 337;
	dataLen = 576;
	combinedKey = "110461ed446c20c3206edbe454811c996853c7cc2b37e3bcd489e3c231e6b9da";
	iv = "afd01668154622ef7b4d5a076da424c4";
	cipherText = "def2a8baaa97b6bfd01aefdb72bf5421b8b5a6cdc177597f01cfe184d9cf1abefeb4bcac52548fb8898710fe0be7d6aecf5f694d1d63a7d15c35697bc71be798a97a9754bce8ee8f";
	plainText = "a11ee20b3a22e8929f8c6d44ddc70d8085f92d1518b6af3beb1bef8a17daf568330bbed6c0911f5a1204019c509411267d5d36bb8bef9f5d66c599d85b4e41f78e6686f640e140a4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 338;
	dataLen = 576;
	combinedKey = "9bc3bad718331e918bd0ba558ebf60c387a15c88b9c58b6b094717a5531e4bbb";
	iv = "98935bda588eba99ae4178b0dfa987da";
	cipherText = "b7025ed55bad49f8771ea28bec835aeb2867ddd27261c939623f5aa119fc53f8bf838e24e9f3b7f59d3cf155312cfb6570ad8690ddcac95579c7cb3ff1c9b6a5ca23595fb5d5934b";
	plainText = "df13a7bc496825c51325d1e9060fe6729e0c723e33058cddaadcb5516fc171095d2ebe887298a2327f17a12f50e5f0af76ac0dbc3497a0987d9e8056979fa945922f7b7a21b43970";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 339;
	dataLen = 576;
	combinedKey = "fb2a748d21f819f65a01d9cee8a31e2ed1465afd7427cfbd64bb1c4e85fb2673";
	iv = "4614cbb08632c2e5aacbefe53fbc5857";
	cipherText = "d8794a2ae0204c2610d54afe93f5e59173697625bb0c2fcc21f8b1a93a8bfa776b69bd536479fbfbfdb0b06aa6e4b7edddb2f0bc1d82f95a9aa26bba9bad7f2f80532f6beab2144f";
	plainText = "5b668143728ebf782a5da82372bfef397c0a96e03b5359b518df177fb8734cd4a6c285abfa4852496256b0d392e59f9ea8c7dba0edd86894375bc3aaaefffef6402c18906c4278ac";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 340;
	dataLen = 576;
	combinedKey = "bdb536b82e4b8a8b034913f22de696c729b8ac2011a5c05a70d4da73c25c674f";
	iv = "7581488666e29cb0ae1ce7fccc32dbb0";
	cipherText = "6340d3488f452b719f6936c1b57e31505f3dc2aac31c21765956e64c2a952578c608225ea333011475734f4d144a29d23044b02c62da45d9352e08c0694ed74e179008f4b871a633";
	plainText = "36584ad92d2eb3e51d4e9d382784db7623cbbc77eb29941d3ee1fa918b5ee625f9b72e3d51d00db76959de34e96e167ec951e251d08da0e058a345e576e2804630488d8edbe36d0a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 341;
	dataLen = 576;
	combinedKey = "6ca2be1987caaf2fd10fb3df72c70a9b24e4dfe277c6c7357d8343afc6230a7d";
	iv = "b2c3d3fd84638c1a9bc3a45852f44ba2";
	cipherText = "ffe7cacc097dd818e5aa4ccbe7a5d22713456a423a69a61628238c028a0bbff21a9897e1fd4a47dda213780372e1d6c74ce719c02ea713c0f88ccec0f5c6eedc0c5eeee711887360";
	plainText = "9bc381c74588b96e7da728707ce95224866777ae8893004d6ef8544cd3e3505e60feb22bf70a30dabe5b2f22d17e3b8dcee1e30fcbe644eb63b0702ccba0d623063b7f242fdcf991";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 342;
	dataLen = 576;
	combinedKey = "69dfffddb975e916a287c39ba6e74c3c140aabb1ff7db46fa8215849197cdef2";
	iv = "2de9ef450dbba8f99398d041f8db5f01";
	cipherText = "49ba3511b85ccdcec866ff79a239f4672e35b8f3d5ef4910bbbd0ae6e88fbb12a8a83481322262c69e9ae9c799f73d20c0dd267abb79abacae092f809f37318dc2385e82312d1c4c";
	plainText = "e59d80b65d4ddd2cd24d166ec3d56e8763d967ff82d402371eebe6ed9bc8c8ee672d23a4167b88c1f5fa9798eeb347ca3d55d25cfa17fff509878b9e37c56aa4113dd52ccab177da";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 343;
	dataLen = 576;
	combinedKey = "59b4d0366488df9768611e784b5e2abf76ff6929419796947c75b1d15bd7bb1c";
	iv = "31c46b13f6df885b64e880eaef3250c7";
	cipherText = "6dcdf9b0ac2869dc8e4fa36bd7b3bbace15d5e877d3e29b67629eebaba4cc30450011ad5ccdea94ee6892fbf18131d7a68eabf50b6f7e2296613da601b79cda4400c11049eee63b5";
	plainText = "de39a13866bad463c140d2f33eefb26a6b87b3939c1bf0517a7e26955a0d2106cab8181891a9fc9c01685dff4a638b1c3ef97ebdc671d8f288f6bff33e3847b9b37cbed2d8c562dc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 344;
	dataLen = 576;
	combinedKey = "91109c2eb89817fa106f3d046976acd087b94b92a46aefccb8c18dc7f0673776";
	iv = "b54389f60294be59d427e83393a9f6c0";
	cipherText = "175790bf6ea1db26884fb92a54154a26448f7664c2a305f0e2350040f1e2c617896b89e49bb1da65d30e1a52b547ccbbe7f2bd67f5f840e00a305d1ad372aaf294f5adcf8cc7de79";
	plainText = "cf4501690814572b3423912a67646d2391faa75b2f9d8e09511ca7ff63343c7382923332b121b2b1254a0667431bb564d3eb2f9d6a4db1dc467e5da9baf99497d8888559acc2e5fe";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 345;
	dataLen = 576;
	combinedKey = "569a72bc49774e5303bf79bd64616be976da8f504a1df10f99caa9ee790d4818";
	iv = "1a859af30ccf7e9220699e07ec6bb287";
	cipherText = "7403fd607a50e996543ce8571a14c2d4fbb968c4fe0f64d93d98615a66bd7f9cd884a5ee8afa7391777b3c121759de2094db40ed91ea9cd8004c2d24f8dae40dbd00e8984d9ab052";
	plainText = "0e1432368879f1ebf70c51bf5a28d661aa92ef905dd47f1dec51cd4b02470b86c90ed1f3c09e1609e4a003617d1509f0a8aa0c9bac0e54ebf18c33449a0219c60c894d5aadf13c9b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 346;
	dataLen = 576;
	combinedKey = "05648c0cc1104f61a979abbcd0606b7c1573b9125148c9bffacd2b26f5ba3716";
	iv = "21d568b5fbf8fde85168ddc716cf6934";
	cipherText = "f29909b0468aecb05edf154b088eb51f5e8db9ec12896bea1e7da7617c25e8a32df5652c7430f946c1f53a43f25af2b4f88d65dfa3bc941095d24f13e05d99f907cf784ec4bd70d5";
	plainText = "327bf2d8bf2596ff145c07313aa73b93c12397dbb2c5df4c8ed208d3978f4e979a84b65697a9a33adcf90d2602d0edb79b2d3d3b0155fffc434064ca44bc2e70761bd8894d28fd7a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 347;
	dataLen = 576;
	combinedKey = "7ffa09ccff11ae24cc40f171340f213787d500e08c4617c1aac7f8f4987470bb";
	iv = "5e1edf107d8191f71e2ce63bd866a3bb";
	cipherText = "9318fec66ee5be264c08dd9d79d5ea713023405cb8248bf5cd84727e88c598288f30d7b424a6cd805b43218d85e42fdedc2dbdce37e409e73f2eade0208ba6e57ea75d9fba42d3d1";
	plainText = "196e8982d423979ac15bcc6beece6f222ba67f6d6fef68efae0ee40d825492d119ff1027c390593f6bc8f486251252c7843121aff9af9b09ce9f16a3b96076a1951cd6e3b61f2e49";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 348;
	dataLen = 576;
	combinedKey = "710550e4c6f6e5c6a919ffdaa6416825aa387a4a3012a718f7eafdbfb284d217";
	iv = "a6a4c2aec5b1f655083e64df89795b04";
	cipherText = "bc85c221fd23ec9ee4ae6592daef62a0a5d9601e6313830d5744b116aa92b4b2aafe0f28c597a43b1b2d058f3266ce3e4260dc8c234be744be3437201ec4f47c32c042616d5ef1a5";
	plainText = "f1b7dbfbcc0f855a3531f18dee8d246e095c51262abcfa4015750d3d989cf364c7f9e5a231588fe28c61758b4f58e74a60a52695750b10f02916282abb9440b7c591153540ded6eb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 349;
	dataLen = 576;
	combinedKey = "14e2349f2949921410e10167580c3c5544215296dea666e45b325d0e14ff96aa";
	iv = "18862240397cf9509ad76c2aac5407ea";
	cipherText = "7b60cbd9e91b738479965c0d1176eed0e2255a7cc0f30f3b25e1395a5e00ce660a57eb844f74c6dc525308ca4d83f6f5a1001a0f23f583857b7840df59c34e2d3be25f2c98ef9e95";
	plainText = "97b5162cdd558be7378bf02fbd245d40dd07eb83c2e8bbd10f655258c0f072ff8c3639c7f48bf548e9dc3d3621b78394c1df54e360731e748c366aa9f7cbc261ddcd9177b4e4054c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 350;
	dataLen = 576;
	combinedKey = "7d44e0e7879081dcb555e490a460f7ccccbcdf5cacf0499f7d76e327e5442698";
	iv = "3da9a735533628c87a45fd2560ccde62";
	cipherText = "a8db84f4e2ffb64e4319a320ef06defda7f46b238b96b1a074fe32cf81d11142477688e16b057054592124f9c3819c29313075375b8ba91cde85fedd0e3a6e6d8c6e016b5257d013";
	plainText = "2d447cac138cf21cdb55e1d8a62d6c9cd1e0bb6a3cc95be395a2452410b8eaac9e8b0be60f5ebd806d6d766dbc930390243af507e0dabe4b6b8c60e5e94a336565a77f927ef312af";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 351;
	dataLen = 576;
	combinedKey = "9d263872750ffa464bf608e84f0e58f656db816ebec28ece5c2a6d9e31e4e554";
	iv = "93d9e4e9a8708365078791cb37f9ec88";
	cipherText = "e52dc873646202636078eb38a2af5009a7c920d1f846f58713e7a5fecfad8c234b09b1848d2878c3cb016be240a7345770cae1f3abafa202b86f749b97ddd52a3c72c6916edbdbe2";
	plainText = "75346e966cb966d2ece4e0925ee1261e4799b94bd491e2735c37920c95f478e6dd4eb54327ad800e545112e18eb9c145f730ead34be0f2ff45ee30e36d7a225d4c0d756fcbac7f4d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 352;
	dataLen = 576;
	combinedKey = "7ad33a114cae2d71d1ff6b883b453558c2ccc86676644c257f8a7c4903184c14";
	iv = "25820571fe1af71ce6ad4546f8d99de6";
	cipherText = "96e82aa414fb084387a4bb445e5e01ccedee94e47a0aeac9d7e352c517c67337097155be1e9f4a8a7cb06a82e3fe071bdacd810a1403b2e5ddb728a6e06da01ce27e51ef33fa34fc";
	plainText = "79b74f135b39e8080d985950a6fb8340002c84bc0b18416a6d07e2b3a4ba6c512a0bf6c0c82e26f59f1b1f973d6082c59a0127559a39e58c5ef8130f77f91c389814ea1f6e96189f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 353;
	dataLen = 576;
	combinedKey = "1d98922e100977f206d9b4daae0f32050e04bdd523b5f90115ffb5646db6ba59";
	iv = "39119e542800a94ab93a385a420b4bf5";
	cipherText = "46301b4af578a9c79bb8aec98946475cd943f37c1014f7c5f8608c048714a64a0ff9151482a2b0ce421e59119d907f61c5af45d2021a52f702bac7b6130301db2c33206f78d2b8b8";
	plainText = "ed3afa7c46fed058fd7f1109bad873140b9512b80e9f109afd414fdccb5865948bfdc06bd839c6971f7e783dce93422d12976d17ee93a7769c66f10899304814ce24857ef12c80d1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 354;
	dataLen = 576;
	combinedKey = "4f42af899cdc035489a3f45d76aa4c931d7dc840f3be3ba5d431306601aadf5a";
	iv = "93851517f4ab3e4b278e15d2e4feb884";
	cipherText = "c1dc68846f70226f59785831074416398b048baa94eeaebf3f3cd914b1b394ac76718425599f676294b9f558bbe0b55c1030d294cbd55034fce9fb143b098092bb3ec4590e12b650";
	plainText = "4c5e35001b282d4d19f4b4e8addf60833a3b9fb8fe881e1689f16bbd591b0e1ef3db3169b95f0d85929177f3067daaae10cc51c4e94d19f424a138631811f56f5cde4704eb3242ea";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 355;
	dataLen = 576;
	combinedKey = "c33da4d4a6dbdf7519adf131c3782cfaecf323d37dfd424a2884f009245ef19f";
	iv = "8f8911e7723e8d9800a6dc912f197403";
	cipherText = "eded15831c8df898d501cb1c6d829df07f3ddcf50f90714610d88d58283f02d2483fa1d8e04283fa4e69cf098af839ecde3351449327e867bd64ba8f8400a999a24706aecb4d8488";
	plainText = "a417f899bcf7576db5ebb43b997cfe0d5d4d94f85a4b877c3030f4b21be804dc59cf6feb944b933c584aac413e02670a02b740fa19e7ec9e99390c36271f6022a0b60d85998795e4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 356;
	dataLen = 576;
	combinedKey = "8b6cfc4efb19fb1ade49b74a2fb1ea02920bb0c3117d376ab293c1402ea1d26c";
	iv = "035a666c55f9748d3be28df7a9c890c8";
	cipherText = "808dbc7e0655e71d4979a53a395aed4cdeae62b37ffa64b5128beb9a05f84313f8d681cadbb1f77312f5b01aa8d4f50f1ad72c872e25b88c1557e5c13f302200203d1f02479efe8b";
	plainText = "160f1893bced71d55f7af670003b33ab51d41bb9c450d043476f89fb7c08e5fbedaf3aefeb1df383e0e3d1e78957209e5ed4c89de1359cffca6f375510cb3db4b6a5b0ffbcfe27ad";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 357;
	dataLen = 576;
	combinedKey = "329f37c34574935ea07440cd916e5b8d02ccd6cfec7f3d8eef5b4509bc5f8c0a";
	iv = "16fe3cf5f9fa3be195e41f93edb49b6f";
	cipherText = "0819e067f21647d6008d51984157d35e3b516dec56782f22b1caa6a37526f1da88703ea42a9502e067762579b79ff87947c442b8841855048436c918c9162f0a0688d883798bbd07";
	plainText = "ee0ef0adb8bc99f9e9dbe85362d119f696cf1c04d190a8162e60aa87ce0b6491bdcbda5ccceb175a8b576ec7080d4fdf8cc13b19d7a026aa17c08cecd7391c487ff688342eec9a68";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 358;
	dataLen = 576;
	combinedKey = "6c41271b7257e620b8230b515b40b22f15f2b5737cbb7883293cd85121d66e9f";
	iv = "f2034224e7d426a05dc4d04b5997b5da";
	cipherText = "50e00e1cc7e632fd9f62345f89cf6afdd4dcfe3b31648463c8a2b2b6e28af07ea0dfef5f57b2f0ce154f578631e5bf9978b385eecdb08b29b565ec86d6c261487d986ca599cd4d24";
	plainText = "36eba1eb8bb12d8b039e286047839ce32a0d5f2fac24ec38d67fb82342e0049728c681c7bd1877fb4582085505d633265c2aaccf2cfeb7ea5fc8f88a7fcaf4061d077ec2c210275f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 359;
	dataLen = 576;
	combinedKey = "6cb9f8cd0427d861615cf00cba3c33443947c4fbde7120bf1016154c62fcc72b";
	iv = "961b75ebdff10dda9a98a933a74d35ad";
	cipherText = "b8b3b9d95588f41986cae7867e034f161e0fe183dc3066124b75519a2d70ffb0c068ee396e19fdcdf042f8e380a4c37db5c94d01efb5b9e82ef46812a562f4b2ea3d90730b995eb6";
	plainText = "9ec7ac4097e8b3cf65bab8b405c09c0f2700e5558b425d1b685129199daba912ecf25a949ac10a1ddb3fcd15b5c6a5e1fcdd9033dbd6b1bbc4d4c3ccf3cd7a30c8da57a2c3592893";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 360;
	dataLen = 576;
	combinedKey = "0749f81bf7014dc60ca894ceaf68c67a64a2b56d1f96d1b153bbf2961643c1ea";
	iv = "215616b14fe388a082b61d07bbe84953";
	cipherText = "e3668bd6cb453481dea1c7ccf6a589190f797a14be755737b8b3983c9f06bfccc1425c0b30f54dae205d79336b5e8da692cbfe769a1c3d2b8ac698d542f0aa6f2e08e6e910fdb23e";
	plainText = "d6b6f1c3a46be63426d466d93a7f508c4d004f36bbc5ffddda812e93f29fc8b93722010c3e3e5fb5158a0319b346d01db50aa175438801890b43d33ad54680cd3c3027b9b4479b7c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 361;
	dataLen = 576;
	combinedKey = "956bb3a59af77fcc69e3838e6ebd2415260e4127dec041a6eea1cf80b756e33a";
	iv = "7251bc0db5841f76f5e55b5532f273fb";
	cipherText = "dec5c78631e1dfec089e735adef9412a0454b1feab851a042010f2974060f8cfd8f29b41131332b8781f5c20759e94479b631ec60f057dff347d0e4b1d540aed55bcbdeb007e7abb";
	plainText = "10ff4faaea85b8e442d852a38a50a8271785e0cc094cf0ac935173a2caf944933277955b09e35443d734917d299627c2c85ba1090ac6a86efc8126517dac27b6fe40ce76988a6b58";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 362;
	dataLen = 576;
	combinedKey = "bb7299695d404ab15e4153eb20731222d08edf22974de6cb08fe4001d199c808";
	iv = "7f8cb19f665e97a3a7491397c3764317";
	cipherText = "6a6b6a4614ad5969ab5467f75cbf9bcfb07cac158928464a4a412a3744fdd734c324af2b4e17b7513f95c30774054f8eac857fa089606aff1fd8fc545e46287ed4d8e1f6cc69772b";
	plainText = "f0fae22ba38cd2bbef71456ed1e06ae9724bdc2e820930e0fae8bc1cf357e1b55ee89b7bcf8d69fb43b5f27c71c80108afef0ad1184e9bcf2618a1074fc870a2b6ea5d6eddbd3a89";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 363;
	dataLen = 576;
	combinedKey = "307c9350f03ee56a481a9ea8e174b4b32d5d784806ebc3daf5b051cf83c2044d";
	iv = "ec73b13a2b76c60074c1724d0f38c955";
	cipherText = "ae61f5ed99c1e18f4dd1f53fe2aa47825fe40cf52fd73b5b4a3c33611050d56f1b89c1344d0c13a503cb2645d38f3ab294cada601042df5d30be0acc65658ae6e30d712322d43d61";
	plainText = "4e650ecfab659e8320e9a8a5437d47d87c8f23ef79bdbf71c5911519bd307756c815e50ce62ce6f532b8952de296be6485574195a19bfb01c66be925079b14e6c3d06392c6792fb6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 364;
	dataLen = 576;
	combinedKey = "0d648075b087b2b0f14c8bd5f07dec240654197823c24f94aa59e4955f1f4a3b";
	iv = "bbf9ef7f879c4a4423a742ddaaa1dd16";
	cipherText = "7119803ad0cd37eccd468a36b478b7c90f38f4cd3c40adb1e783d1e6f2ba104f6b0b5cff19b55454594614f2d30f0133adbb27e365e8606922677de5815741f1b9573a327a7a0533";
	plainText = "d90873e7cba12716f6f134af9a17efb966e91cd8c757792821e9d58e0ef2979cb51605f081e566107bee9d754a2965bd417f1bc2fd3f0e17d140909ba65137facc2a7a45256c457e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 365;
	dataLen = 576;
	combinedKey = "a5d133c8eb083ac7460a88acce73c26a8a4a1789c157becfa36e7d49e1327b91";
	iv = "de9f27ba8f1f46d25ee9264e948d9f2b";
	cipherText = "5f0978c084b2ae420224fc393d99ee65fc28004d976982c6bc52ce6627ee807ea14b248f8c042683ad77c406eabddf610a2e5f3951fb587bfc2dd9b3283c5410738bd71c0a053ebb";
	plainText = "598487fb8fb6d69261e7d7c38127a4f0c04a40aa24119e70e79e39830398336b733d26fb3a00c42dac9898e92614d3b699b08a32e2e85d6b5f813d4adef3b3d43a4d2f00a9e13228";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 366;
	dataLen = 576;
	combinedKey = "36940bbc24aa2b7be09f2e305e6ed295d1a67b52199b6de52677e5af40737ccf";
	iv = "84d1c08caed0d195577e748096b280d7";
	cipherText = "998f1beccf77e8f59c5ca35651b2651039459fb9f3cd81bbda9d89eb856e33f8ca614cf8e924225336561e4ef74cfe2dc936218c3839a8fc92cbea8a2fef2c600d7077e0f5890d5e";
	plainText = "eb2bb447f1d4c30366485d2a8c379d7c31f3ced4be79736cfe6214b41952ca3c300f947918a91187b0b6e9c46520deb23a6d1d8ee969312e9b9a68ce103d0884c96408970de9e30a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 367;
	dataLen = 576;
	combinedKey = "6c9d1abf4dcb5f8705971e34a044331487005710429cf56b1e681dd972fd7e18";
	iv = "67c240a4bf7eb1cdd41f9749e34c9f42";
	cipherText = "118a03bb97fc89a6ec36f4b6c1b9648cb7972766fbff2aa58d5819e5e72f3b7d25620d775a7dbaee19f866fa1980360aa1f8f624d5fc54dc0830801adf0d43d73d4b8f39fc4b8ff6";
	plainText = "77c64f785c1d26e26c983acc376be33bfb56cb1ad6c09a8c64453dc30393b6ca310207989c49513c9e9ba66a293b85514eb42945654ee837da7f8efc52f3615aab091d65a2744a3b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 368;
	dataLen = 576;
	combinedKey = "5568ba75c1fc3e7e654019bfc1391918545e380781729c231b32900dce12bd8e";
	iv = "0aa2b015732a683ed735396cd85c102e";
	cipherText = "3b246ca3b756d8c4fbdbb45548c0d878d5de863f3f320ae586f8111ca294cdacbc9c0a2a6fd88966893e24f0ecaf61ecef3f6b2293cca78a9cfe27e5c2f8066f5630c3be0d0d07d9";
	plainText = "2b74c553dbb01ea8ec64e5fa0958cde34c243ff658300536ba5b1e23acc1784b16ccb4278e79c057fa465601e8f2b0dabf4c336b5965397fb4e9b5c884a54d42b901a78fbc8cea1d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 369;
	dataLen = 576;
	combinedKey = "50ff48c2ee0c49e9464675810f4cb75bb26b9169c66d351792a567461ed51cd9";
	iv = "e868532013731499dce0081a740b5b57";
	cipherText = "8c2ac69ca502708919e6167daff2e17db6a35d5ad4a4af11589c0ad094fd0c2d6e8db7c3cd05d66b54a5a56a8be0493aaa79ca26b882ca494358fab5eddab7b28c4ef509310bfefe";
	plainText = "f0c3ccf97ab67cc20d44ad0a4bc901f86f91013da52bd8d4a34967939b7f0a6360c1b8d53c0c318f7c2821719ce8a70ef487fe8c2fe60c177e42c56f16b936e1dd8065edb220cdf5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 370;
	dataLen = 576;
	combinedKey = "b773ce8db448b0a5e4da592ce5f29dc6ff3fb2970152c9a49810f086f6b5a6bf";
	iv = "9a3932b1694c751ea689b27d3343a650";
	cipherText = "4637f94f1619d865e065c713abcb1af2bf0fea96b6e900e197453b8444bad35ed98a158f5ee9901b61784deedc626841fdd3fe22aa9f15bf314fa97d8de1b36a54f2728164549864";
	plainText = "7a7f2650f40ddd9a4c5d5920abd5e024e9f66cdd6218174da81e914852f95a182e407f12a8abb476681d3cee448195a9c568be2fddb5d701ea26bffdd4faa554caf8e9fd69e554a9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 371;
	dataLen = 576;
	combinedKey = "ba663b9e101b918db1d909da26daf939fbbb345493efc4ef9c7119519451981f";
	iv = "83e57723ff1a8d8fd7bc633831f5616e";
	cipherText = "346c4d379b0c2141a374a248a74f5713da2bc60f391bf75c2c730c5899c3f94fbe0da36d441336965968ddac41a9ddf5dcbede3be1938e2f5e72ec662235d0dcae85ef2b29c02c02";
	plainText = "0e797dec7797ca1773763ab3106618b6ae9d2c48e7d81a4fe0ab658d7c8f1c883b232d8237d67c72ccec25704d5b48b0719c4c41a9789881822ddd9140b2d715a05b291d26023690";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 372;
	dataLen = 576;
	combinedKey = "bbaa56f38c0678dbf01f6ddae5e17a7b037b38930596e24c23655ecda4f3f05d";
	iv = "cffddfa7aa551dbdf6c4adcae555dbfd";
	cipherText = "161afaf8dd6b72f5ef753228590544ae2c8de22077c23c07151e2e7ce0e84df5571acfde60cd0033a1c57b380e76a07f6757f9a0bbbdbf03c2059bfe7abe572294f327597d202ca0";
	plainText = "8ba9bd231f88778330b6aa7f38a9860031aac6881f62f0b545c814d1187fc831e589755462608e15f81806733d07686e085f265d2b450d7d8b1abeca15390248e2a6eab083cd2491";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 373;
	dataLen = 576;
	combinedKey = "8c2a5b2f9e20ede8568986e427f962cac25ec3ebde943dbd34e766f65be2174a";
	iv = "8c7a6d86acfaececffc73bc4539d0a8e";
	cipherText = "02d69f924eed46ab3b0fe212805ac10428d2f05473572bb9afe26c9d7d195e3a6e76c5feb0706fe6876e70dfc44da44c2f5539e5b65db821296d3f036700d625bcc571c6a5dfa0fa";
	plainText = "813ecb8b05d5574e236d7d47b3b583d9fac90c4094abf0c72d09f43d1a4561f574dc522488f726d47482fc2e0e737904ed59cb4d8572f47c81d774a463479b05d1cf39d7a5645295";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 374;
	dataLen = 576;
	combinedKey = "f52914e4f9e414eac727a7b3f5ebc944c34968ce9b1838cf1ca0f5f0f4ddb331";
	iv = "6867979394b99eb0b8e632b30a18a243";
	cipherText = "cba098516938d7689983d65aa9323a542e59f92ea15e0654ccd0aa6b6d86a81888952c843185cb80150d592e84b4e39610757c95234c184f50ecfedb07c8e6d5c0711a66aefd77c6";
	plainText = "3426434a0846ff7a1610ddfc18868e8c1c92d4779ae15323a8dd6dd1da755d456d88adc2247c4b61b192af4e0b47e1238b8122684d2b81574c2ec9f0b42431fa434481e7fbaccd45";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 375;
	dataLen = 576;
	combinedKey = "6ffc86b76b0b715ce3e409ab7e978c5a00e96c5fdd8bc416859cbef47a6df681";
	iv = "d4f224ddacf748c61b6e742924e98eb0";
	cipherText = "ac39691a5d144329323bb4c88971d18b99108dffe31b1140dbd8652e651dbcb850e428f27e82563871de2614c63556ef2de289eae521dec3a8672e23fafbb020f5e0b959d13d4e17";
	plainText = "b4a6e44ce58b43a69a18256c7bf44de6c878d8af7dbbe6e16b018019d64306c5df6a04ee7c8138dd40fa563e21b86f4fc132d0f6682cc0fef87d59b7f46550f893a0a6575f48a1b6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 376;
	dataLen = 576;
	combinedKey = "26d703262ef514bc949fee1bc9b5e0bbe0b4ee03d7081eac8bcb8c1c77c4b249";
	iv = "909020e3b5810d7dc06503df6b2def0c";
	cipherText = "4dcf09a81aece5a13cddb01e0339dffbb7b362de6f250bb5782199a7d4686d5659479fc665e99fe5f626215dc4912c3b4678fec3ade10fb656f54bffd2b47e7c81ba609714a5e780";
	plainText = "5267ef2c0837800055d68a8df7bb8ca56f748f76c8838d4b0e9b10099992705e0ee46db4831669fd296f87a6d7a9fb8fc3a7a0f19fdc49b7ce1efd5104494bcfeecff841870281cf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 377;
	dataLen = 576;
	combinedKey = "b440d50cb42218831b1f7c76ba44c9acc63e684b9a901ffa848b910041ba6d18";
	iv = "8e3d88ee9f8cc62fead3f8bf1074fec0";
	cipherText = "22dbbfabfe84487bbd13a7a98bb6973c1ec61dea09c8b0be719df0129f51464f072f61b6f98665e9501d011a179eedfb4ac2a6b8eeb8a1ffaa727278b958b1f0aa93e0bfc4c3d904";
	plainText = "300f0c014d45e2ab2ab207ca61930d86f9ffdb7d8ba3c5a34d33e4f0540b4ee4a3ee9206ed15b01ab1147815db9bfe2791a5982c174ceaa9b5b83465d50009a4fbc35e1cdc63f951";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 378;
	dataLen = 576;
	combinedKey = "a36346e3fb39e1d7160d04680e2af1d6ec0012e4b1464428a73d6932199ec82a";
	iv = "94f96e0aaf874917daa3b777676bfc7b";
	cipherText = "e63ecea73cc87a7d2a36142095ef9945a76db94fa464e4b6d2a1dee3a7439d308b8deb4386ab42e6618d1305bd4bbe0cc04dee61cf675d975bd7abc05ee28d194dc385258a6c4a53";
	plainText = "9da86c9bb56762e0c6facbb2a2809f45cecae12a6d739ce8c721f22355fec0fdab05a04b79fcde658ce89f8ae90adf9b11abaf40faf8a70993d846994943ed182fe0bd7a57c83a79";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 379;
	dataLen = 576;
	combinedKey = "f3b850ecde4a05d22399b27132334b7b2b6386d1b0da9df4aed252aa0e4466ac";
	iv = "765c5ae1f12e704f4adb7855251d8638";
	cipherText = "a94659332c83e0f1c9d30ae3248e6b00d107c6e84f45712f78415847a5af40c93369a26ca9b3147bb6f22fc82f65f866325266439219a28e613830319c952a76238f71460c76161c";
	plainText = "d514476e28bd7679bf48b39459ee81d0c9fcadeb86ee75b807807085aef3c16ec9251b7b04e71aa50563044ae3dfaf8ae270e13002fb6ed63f6e025c3472b7e89d20cc1716560e42";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 380;
	dataLen = 576;
	combinedKey = "1078d6af30d3121ef3c7599f3ae046b809cd53933372f4f7b1526baa46191cbc";
	iv = "9db3ca7d59a450f92c0d2c26e19af76c";
	cipherText = "ba4852df5f7484873a300f4aac734fd959de070d6cf36c87646349be4eb5d8401135f730ce583850f73a594ab99868cf268dffaf762e8f8480c2710417ed7dfd33c9ea51f9d23f7b";
	plainText = "577d682f4b9e0044c3ac4059b4b03af7986c766c7421c0cbf638a2a15084d0ea2667fe2e5c1028f7a3f4f1958382dc7abbb94612880b87d42676dc4cdeda58752a77cb31bd310c9b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 381;
	dataLen = 576;
	combinedKey = "cb664dbf3413bb5acd89c3623917e73a40f719c895f8a65f38aae916a40ee34c";
	iv = "8fdcf95aa5f7f538b0e03fdf51cc8a7d";
	cipherText = "a8b9d6621d6c9b98a0753b42af08617efebd9766ac2f963a6fb138630c75b1d2ae9623e571674290430c38d5d8e37b716377cab43bb2464e6caea53583fe9c901e096167b1fc8b84";
	plainText = "7166093ad8ef5ceca9dc2168005b618c9169ce66a081189e83fe986bdc92eb523c3ef9150cbe18c89f685730e63859fc7ae6bfd89171398efa811a45544b5c4e9ed0326579047e34";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 382;
	dataLen = 576;
	combinedKey = "89a01332e1579147f3b44b61bc607a281681473f37a44ea085b5254a37b31760";
	iv = "94815e3566c1b2d995f13308c0192dca";
	cipherText = "0573e189f695898b209350ea9a5017913c6efe6b8d162141e378be6d117ace6ea080e85831dfc60d9b25db7b9acbedc36a47686ea2d72fc83021df6a97ce91ae664d27678a241f87";
	plainText = "d3698d6eeccb2ae919ea47e9e659cbb0540e3a7ef48c0de63f3f1a8f3b598689c56a30be1c1c371d190da88034d427c088d70684bfdaece77d4406d1973b6766eb9d0f418bb689aa";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 383;
	dataLen = 576;
	combinedKey = "9756290aa95c942cf7ad9052b9d5b73c15c53b05415481bb685ef13a461ee1f4";
	iv = "add5d542dbef5c12f0505275143c963d";
	cipherText = "c716dd902548dd1bb5257eca07fe3af1e4b4e39b4225625636fc235280ccd8c22f2703b0893535e9115a38fe9fa6df0a64e4c93170f1078f9fb272f650e6490a699d8516a93f9a55";
	plainText = "ba5fc1de85d7c05f19a87bcc520bc38516d0cd0a5c76de4cd668999c2ebc84111503140ab476b2d4a0eb79063810f8f31f10ac93144cbbceb6442ca66965e71d40ddcead3b8f26f4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 384;
	dataLen = 576;
	combinedKey = "f69352569f832d73112dffb9d39aaf6de26216caea8643f24e9f13ca7e6b0832";
	iv = "0416d05a240d4b2f824a5f61d6b6c537";
	cipherText = "23b25ba5c93f0d004e7044b0ef413e6c01ed52f972661fbab4407bb433e26f146310c135f5af30bf6136b03dcafc74cbcd1cb1dadfeeaec71e500a4e5b7556b3eda224c4a9442a20";
	plainText = "9ee3b369c9d9893f6e1877b6643446f199d7e7b5b78ff2ed26a7c4fc2ec065cdf4e43a3da880e0921d4fcca8ef7ae5d18f90a2a4dae2bdbd77215b45b397e6a5cda05cd97e42e8cc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 385;
	dataLen = 576;
	combinedKey = "39a897aed3689dee285fbaaa565b2f4de5a2d19941faf75967a0d39f7e906c41";
	iv = "298a952f1c4ae5dad1f41f3ceb31b229";
	cipherText = "3ee4721535e3345efe92b7c826383b69acc1762971552dc1aae47bef1a2ff8f50d3a85dcd9118ddb2b2e3f05e5b3c80d29e1489b23819d55766e6204b63f64bbae61f84a7703fe59";
	plainText = "d5f475ea0e2c032da08cc54dab6bd0990e844032208f4d82ca313649430a8645563caec4bec406861f4bc8871cdd0b5131b7287bcbbde57717439a62fdbf33e97e45a444ef935e50";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 386;
	dataLen = 576;
	combinedKey = "f25658b4581bf57c290118c831562b2ba9bdd2386dfeeb2465e91db5d620f9d0";
	iv = "1543e5234bc3bbf924b80f8364440b10";
	cipherText = "731b22f69c19e226fc339810e1b43576acfcf96a2409f810df3af692728cc899160de2463246e03747a9db3b118f9dbcf9c6835e2038b16bca1f137f49c711db2fbd001e59cd76df";
	plainText = "b1dc804a94de10536f4bcfea104832b06c3861dd59a08d483a4d2e56596ff002739489995a69b45fd24599a9ac4bb87a482e6b7c7d89092d4f71bac7addd4780e63bd454e5907e49";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 387;
	dataLen = 576;
	combinedKey = "dc9f71bd12fea12a5888f64d558c14277f5809d6255131e624c2ed16421755e5";
	iv = "6f8c1d3e6e75b645d08c345e693e5ed9";
	cipherText = "f15100a35df3bcbccdaa862106236dbd8c67db033577a737d5bfd1b0a02a379a52ae3f43b5d28f00b7beb69718739a5b0e1f0fb48ee66417a89574651f60623367f14d8b28d57b7f";
	plainText = "4c1f501b80253f165e57044a4b65b062bce8594ac686e14b53686c257f35670aa06290c0d53baab086239bc793d073dc4936a565fd7a86e1f8698094a348516336faa2a1f8169aa1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 388;
	dataLen = 576;
	combinedKey = "aed58841eb72724cebb4ba7c92d532582b3274d170b33a5cc5df3e9680837fdb";
	iv = "5f32abbe7cec01f36b554c3b8f0e1634";
	cipherText = "e7056907e52627885a833312841f8b5a07bd33ee349c72adffe76459011901e906dca2f268ddce02916f867a8bb0d8e8aa8702491a039e2e49ef81b9310a668255ae1330522f34f8";
	plainText = "dd1f14e17247ee5f10962abc95eb8f2956d09e4b98837d53e0c995e962c0c3b408dd94f3bed6218818c8e76d2b9536ef7f6a5d02b833cd5f57560bac7a7c68afd7139b10fd03123d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 389;
	dataLen = 576;
	combinedKey = "b9cb591bb5083faa9c2b966fb9559f3e0558cee5aadb4ff528ee32541b5cb57c";
	iv = "364d522f6c33c7beea5869cf19dda258";
	cipherText = "a137b9c57392e0ba020fea87158f2ceac25634adc9bb581ab7914977d974609f31383a217500e79f3ef364645b9c84675389dc1bb0e4ef87842b31e77a2cda52cc772deb05a38a8b";
	plainText = "8c0ce884925d69110581fbce48092891f832481008a148ec99a73895785e5e0f57f7c2ed5cb814ac74dc58ca44974696cc3d6a5079991c7864d608bc483c4e5e97a520bc867283dc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 390;
	dataLen = 576;
	combinedKey = "e40f5b894f69ec2b34c41f379ebdfe169920fcc37a2b74e36999f4861426194d";
	iv = "c7f10524e0cc43fe5065e0dcc9f14f2f";
	cipherText = "63363fc7bded76e000e7ad526211caa4f64593174d48c49c404a4fa4083991ac392bd3771fe404ed88dee8234b6ea04abfbf55dddc288b8f61c6f9d519421947db7b438644b394d3";
	plainText = "52854c47b884c420de6eb2321e667d457c9f12227dbd3a93b9880962e79acca1a02adfc2ec20bfc198ace99771ad87954e050b8c256d1206ba149fc3f5de14c05f80e055c28710b3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 391;
	dataLen = 576;
	combinedKey = "5eb44144ea3886808bfe27a7c415b82e467387e1a74bac263871653ab87a9372";
	iv = "139ddec9246b41c16279cab6288dbcff";
	cipherText = "4caa8c0ca58192089ce44de5bb87c265f7d5b129a908af68875b2f948f562a4155bd8de61c9dc121ea23257e54083c8a6ee6cf9b0fd49173a5bfd5db8359cffff4566d197cbbd0cc";
	plainText = "abc1ab364621121e3ce48ab25d5c7bece857d7ee3914097b1ca18b452107a4c005641edcfe790a1ac1a01511623960eadbbc3cd0bef101d2dc33cfdab6ff2965fdbc9f51538de8d9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 392;
	dataLen = 576;
	combinedKey = "0f656728501d39ad22441dc1ea68b74788663dbf3bfc2d6f797ca9a7c1f149ae";
	iv = "82a0656ab2861bf91d54c4745f1c5795";
	cipherText = "fd2c927ee2adb6a4a5545f4b19737bedf45318ac36865e692a60a0b9d70a958ba6275d6e3ac062fff8c830fa285500418943c8ccd6d16e6246f2e60c86ddd0aab145c58e307cab65";
	plainText = "6c717fe2d7f1bc576104c5afa7949fd3052a49a6177297cb63161bf16e2bb5276da842f2803aef6c5509509c61195a07a8a4906f066d0db302df42e4413a1435a0d1cb08739fb347";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 393;
	dataLen = 576;
	combinedKey = "94d243615db4227e937bb0846b217da14b02c1caee95eb6ab972917a65972372";
	iv = "8395ea418a3b66c59108133f91dc7e1a";
	cipherText = "418fa88e3ce799f8dfd4ecb5656353240a2db08d334303798105f7edd188fb548565932c028b6138ade712377f96258a4cb735f563c5c045d17e4cd502bb070ab2400a0403db2504";
	plainText = "c2c79aa2c596777f6553d9aa5f13914b4039fbc41ad4b5f09d07ef79790361e8d89c3562089e93a828cb2ea7548d55e8f284ba0e9902a1f9ce842ce2b7fda5bb7eff232f4844a847";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 394;
	dataLen = 576;
	combinedKey = "c9be21480c24738100670b6653bf451cfe305c828aa4499d61741090c4e32184";
	iv = "8e238d0f860f5d9a7aba517b0f537db2";
	cipherText = "4e1aeba675dfa5fe5ea363064d66b256b1200dcc42e81a4f73d14c56fc6cc058dbe2c29e77881dce7adc44125ba428dd4217166861ceb7dab4b02364b7998ac359a972f04091b526";
	plainText = "07d8d290cb6eb6ca77949524c263334bed2de416b9f5760bcc194caed2ac259c9e8e54773f4f2ebf71215acd5a6100a19c1d6b0f47c2ed73ea96d0b4839149ce95e5e4ba710af4b6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 395;
	dataLen = 576;
	combinedKey = "3593b944ebb223a93f5a2f1174fee4d48bcf04a051e0ee55513c6b075b6b7a55";
	iv = "1cb2dbde93e5fb31d19a21aa5033044c";
	cipherText = "f12fae71bd2c60aac3d690c9f3429fab98d40dab7b9de3da3386d9fb086e8bc3198abc8912bee31f03644bd261576551daa80c834548e95c0ad8e306302d097be829057e08c1ea4b";
	plainText = "b6c29c98ef4675619d037acfea9936007d450f09848c2b06df348d8ebbda8d48fc4a55d3654de4a9f419029b34ecae653f884e51a219bf456086834199cfeb85198df824647cdd26";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 396;
	dataLen = 576;
	combinedKey = "4c3df4be3806210fc3ebae7e58cc4bfc090524b00df3846efa5bfe2b0695d45d";
	iv = "d86442ada2bf438046e7c7317c7c20c4";
	cipherText = "09f7e97ee99aaa9b888fbd2e4332b6ece31f806a387b08060c4b366ff25d5e00c29829f1d3fb39767c3b819183210ad96c60974fb54c5d44f8a52fc01a84e6a25e6afd5cb50f0cae";
	plainText = "cbdafb6727a7d3eb6a0bfc0b9a7004679a43805748427ab252054c9cf958a1d691d1b9abf85cb0cb094f82a5ff908b913e83406fea2f8b02870be08c3fdf8577c622578de40237bd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 397;
	dataLen = 576;
	combinedKey = "fe3e5ec641de89ec1aa37e447c1a996ff2744b8ff078ebe3e85a2b1879c61ed0";
	iv = "0ca43be41065b003278a9dc9440fd6f2";
	cipherText = "e4b00fbc4e3d2cbc9f1fc7ae9b49073bb251207bc1194636f07a63992f3850cf96e36234f149dccc30cc9d1321a41e5d4704f82afb95ee0c4a51a7be1b44a37b5f971df86f4de3a7";
	plainText = "779d4364028b81ad8af763b592050ee70f5f40ea22dfad1cce06897a983ae8845edf0487d0b80766ec5a7cb45b8b1de2e815f5c29af8c80d26859b5f356aa7a6da19a56b68a3bcfc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 398;
	dataLen = 576;
	combinedKey = "acbbf0d3dc81259d9a899e311504e28bc79cf437d3324802aa230efabfd30ac8";
	iv = "7f9320ea0b08a4b5e3a4714ce14a8c88";
	cipherText = "57c3e2eb6758beda22dd676fa8356e049f3198170ab0a77eba0b07bf667088399d17b7ab4694b0c66a59f2d7a7b212266018ef188158fa66a77c9b8a363abef0e63e923787700fcc";
	plainText = "8370464c0cd13cbb70cfbe8660dc3d929ad8ef7e53d242516fd4e964feb42966b346b3379602685cad29bcbd6abaeb98dd7daec691152c0ac086077d50cfbe1565ad38b5f7a7d2c8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 399;
	dataLen = 576;
	combinedKey = "1d6e3f24a6d5a956cae561dadb4efcf24c6a77e9efab3a9512899dea546fec55";
	iv = "5d417ceb9e6e3c22a44894d3caebc00b";
	cipherText = "4cdb282928f9f0e782dcdf14456bcfa839c6f94f516267feffc98d011bddd0df75490addf261fdad9faed3737beef02c27ad0c42416b72596419ccf18051e45690522ab4ea0fe6bf";
	plainText = "62a91e1d333671e9f3aa9c7afd18dce1d5d095e4c0239ad3aa4daaa291136f75f763582e32a132bd9b8a68e3254840fb46a612a4f1af2dcbb0cc99a8c5735120c5469ca063e52ceb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 400;
	dataLen = 576;
	combinedKey = "e3fb872306a0b6c34690950b1bb08838946b3a1609827a0899898d19acef8858";
	iv = "22515dd2642e85e13de8fc57d4098e7f";
	cipherText = "5f325b099d5b07347568f1f5120ede473bd50577daf3e9b7241839c3956380a1141271cda08cb3ca4092131eca2b33ea3d83c074245f5b796ddd66f0f25f298f19ebe700d6bb8a2c";
	plainText = "88a91152a37b3badb3a0664d82ad3e74d46871ce361c7460bb710360a64fdd6df5594f0ba43b9af80c2e6bfc75350605c23e7cddbb9a29f90dfd96d8370ee57d654463ee371d59c9";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 401;
	dataLen = 4096;
	combinedKey = "b6edaca686ee6be33c1f7ab6747569fdef8a3610bc3539022fcc344d2ce7499c";
	iv = "a254405123e2f368c371465ea0b2f361";
	cipherText = "414c7c6596504ace6ea0908b9580ee6da8664ebd93f44e2d7ec11368e9a2f9ada1506d574b5785c1385137da8f890caedab4968a9963f5697c5d4a94caab3758dfb53a5413bf97bb9576bc22adf1599673772007e9c5ab91695cb01b02d577bf28c6612eee008250da249c50923f8791e15735cdbf86096307e1ff4f411d32071cf1c2bba6b4e450d00d22c60d2c8985414a23524556d4b8ff3eb0ff395a8cc48fca5d2d4ec078aa2fe9a0a36447a5ece58d8380226791acbb991ebfc4bbe12b68d921f3d0ce7f24294e9f88c4ef181aa3d203fc60445586ca23d105bcd25deae303a144552611d726e481f313f9e892d54b2e583ef8ee4e49a9845d9ff6c4157efae53cc8f98472f8aedf6caf120ccc04c65fccce8ff4fc5cb8df52635e6a08c7d0b08384cd54fef3fcb0b7fcdb6f87af28ab6b32402ca2f540c9b3940100f4d3745777112a3f6d3493c9080a6b89aa69974d1f6ccc3d45b577da2f2ebeaffe9174b23917d367662fd0725821bf21bd12169bc3a687ba368749a81d19b3d7c40beb4b1ef01a5c1cee67c958aa196afd05a98d2332300e5a79761a710671ce6afacc7920a197fe2e6fae0312715cb7dee21453a650229763628c2855bdab3f59fca10c9f1a07b77aba0b5076df83f392662d7f8d1817da510d685e490029847658d4895ce0c7eaef99ba5f2b7060bf67d19c69dfb76ec4d90b59b2b9e32fc31c";
	plainText = "ba4319bcb9307914571c7d753938b202a58df948a8b64e69d64da27f1a5b5f0bc43936571cfe8ed27d9e7a7338bffcd395462c6c1f721c120a29c67183e2e452ff98d064e80815f12e0b6654e8a94fdef55c133f11f0f5768ab2f52566f05483e9f3317b3d064f8ce55bfaa7076ec2450fe030a9f4f67d7fa5d7a7395ad7d297c54914fea2fa813d912a7e023ef2aae34214da2b2098ec8bdc845bf351354d5c727e859e9e4bf90e9683f8dfd8d57415f1bd33a05a87f0279c58e77a7cdfe2fc3645b5d44fcfd5675122f73bb0b226d5e44539604b5ee98388f5dd6e4bde00de7101e0ae0cdc12ac13d55c0d9b684eab7c043091ad857fe6c02fe8ec51a5a80d3432a036ed3c2a4f49a133b30f42b37f19da69c5199fe040b0d5b7dd5012d18577d6be7db79cbc3377cb2a7dc9e8de02930eb997ae5b0adf11920d27bcc42bb1a0e5d4e42a99005110ff2a270f465b4b1d562194d0ee9ba4da967d50c3939227206d43dda7af01b0b084d3ee3ede638f145b83e2ea34dbae437136ab1d1281ca688142ef2aed60c76fbbbe9a937c8f631cc7bb14b62a31f0fd4af6591a487b56ec0f20ea11d83bfca2655cac95df7317cdb43424faee8ef74d3c249dbbd795a0a4d32e86ab39fa4ffd7c63dd7bd53a36ca4b869f5bf43e796808417df39977daadbede5ce7459ad2cc771abde445a588b4c3299ebaa8a7f0ed117f8d5aedb26b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 402;
	dataLen = 4096;
	combinedKey = "449ded836451bcc33bbbc96e3469a79b5382192b3e7de466d3d26601b4647505";
	iv = "48293056477319eb555d5e1b61afb09d";
	cipherText = "0222de83454ee8b3152860a6fbfac90c2aa2e4cbb05b240faa08d4a6a35c403f26e87c53d88382071d7f36ec61b41199255428e943f56cbb6baff7f0f2b0a742a52b8c189500d50fcfdd20a825f99399656a37bd279c562770e9fb9709fab317b902f1a34ae6f7b5ac018e21a93686555045792b4bcb181248dc9c941fff3322a3ca9a27f4bd06ef1337ade85a6c0393a7ebe3bbe7c75361f22ba7934b21cc392627da3cbf0f5ce11676c0830694e55308e0a6cf13d300bc0d3482d52be024f509f594cdba112a2a97d6b31e47f766a486172cd3f555a532cc6398a569ed3186cd8caeb8d391638050eca50731b21bdd18f310ee8cfa76eb0330d76e9fdec6cd1acc9a0b081d3b7d74e27beac132d715b728df47e6a2976c09571166ea99428b35ad65ffea19091b125d4219e0d6c607e42a0a9124f42b6afefddfe006e88009364b889a97fdcd011c5055e548190364627bb1c8a86c4a4f57f2fe15413b7650971267ff7d8973d4aead17e6bb570e11c69903e8f4336e92d9e422377474bed56baf57a8d9ba3f69391502cb96d56225ee7f54b93a31299698a864dcb61fa358f572ada55363a1149ba6909ee90127c6eaa45fb33971c54697f06f316b571207d06c57ccd8f04fcb2993c50c975fca8d5d2ea4f0350576a4d2d5c850e83ffef8cc692152b6743632153743b1c0ba92a69897eb40ec8c3854c26335ca1a281741";
	plainText = "3cd30cc616bf76c478c98cef4c21fda31d91bd1403499f074e9295c187b3bc7a73df437cb9ff23d8da90fa289e51ccb06dc8fd1c447cce81e7958807cb554313ee425727f3efb8c8ac0607569245f3d49aecbb681eccb74469ad8a02b3f693a403a9519a96a0fbbeba04adc26dd7df0986bf2c686cab4565b0378c1d670183eec3010c4982f85ae7b7402f74ffe6320bd1e1f2e8ddc877858ffc38c99133d832cbb825e29d7256f080bdf7c142f7f17b7ef4a5188cf1847ef55ada55eb37d84d3eefe46543deb96124cb88b040d198cb633e41f1aec667265b82561e34ef721f7292eeaeee3cc3f7862736085f573bddffb2abd568928c238173acef4f02d822b208b71d7c7f7ac610835bf4bf530b75d24ba062d942d5566e7fa0d5a5e8a97aacc7014f5ddb2dc972e8f3693dd6538b0f48f144a24119df8f32c3bdf2338da2cf45303191edfefbbdfad19e6feca43821551cd7d60d9f32cf2bb1a17b6494c16d51f8b736ac64583dda6e7f585fc6dd60c6b119071611df87ac7d8d38a6278580c0436825893e9ec3ea81679486e4576519bf33731519d3c1ce46cea80909914bbcfe8cc79901e5fb0e2d67c4a987e53a68b0b7dc88a57d8bc06b04470fca61614cc8d6b102610080164feb19548080f569167e628a2fe9a48996bf01fd29848e7972f683631e4d6485f35c2889e1cbf0cd0fc9778c2f83820bd4852f444437";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 403;
	dataLen = 4096;
	combinedKey = "91d8ec8b251bd8b014de2a62a9432630276c3589f8593e015ad819108eced10d";
	iv = "05335e961b2c0040bf9ead3c4f6676e8";
	cipherText = "0ea0a9c0dce07a785bd3122d9a230626f350bbfbc6ee84dec738a3b3c0a8e3d97c39ce800d003466d3458baba916bb3c33a42615e1fd8660e93acb847e1e4755d06875a62586df8296c312d9c2117c86d823cb962609fcc5721c6c4fba723776fd9aee66ed5792c0e3f21c066a31806eb28154cc2be956f349c97542584e61e811e63d776fb40f5e1c1163183e19fb823504ea17ad498c8258c6c752333743fc37b0eb434d825933387646774b76a248ca267a2c9458fa4697de89818defb38a575508da15d3b63faa4412d43a589330a45d38bb6ff7c2f82611e7068156faedabd9615195599b9606da557d1e471bdce3b368fc8202e78b13c7c364d00b5a924ff26a2f677d2b9674f7e162dac32a010950570a533fac09441613a1ca010174a98984218ddbf0ad854ddb4849a1bab4360744b9acad51288d04abec125024f7e91fed7514e70c901b7b1227c147ddf86d35a697760cf87fb0c3be7b3046b57c15a2d81dfe9604ecabf72bee8f7a501856656eccedeb5503a905f767c384ff1e83939f507ad1d6377423d3de12667b4ed71bb956f86a793a065176ba80b769c10ec4d5fedd79426d6e028567b32ac62ba68bca9bc225d3e98fa1bc3dec630b66637d933232a3780fe1c478a3e77a9153d0986d9625c2a31cc586e9af964e1f82569fccea876cb88a9e9ecd53012ac165bbf39d4ca585b5ff50011a74b4798f34";
	plainText = "faeb835b4d37f10ac69e2c66016a10122b4bb459ace2b70ec1500b88f81531ddecd878c33843f9cd456725f92c2daf720ebc31476b9ca86d00c8026052dd027b250e6f1756a8f83afa56acf3f42652302a0a05070217cd65ccb145c13630ec146d021162171baa0268ac9d796ddba41816186c336560962693aec784a8bd2b012bced36301f0fef07909e560c4a36b10b60053fe4096eb65c10d445d3f03036dcc5635388e94348ccf1234b91d3d21ccc64b73521eef219086ecf2821a2b5663ff17fcad6de75b88e86dec5beac490bd116efa4788910e86341e3d84cf49c546c327532897eaf3b66ee5765857f807f2f870cec1fcabf7cf6be68e8d699145925c55cd5aa03cc28aacd03fadb33cd2c9d41839df2e9ba5100526f01faf1f470fdf25aeff299b2ea7195ad3d2cad62c521f4f6318f31310050a6611bc319a326b575620e8f8e9e6de4810e5d008064d9b0c0eee77b720ee2e2c4f0f68660e37b53b4f245b3c877ccb740430dfdba093421d58d148b007d277259e71bc1a410ba55f50f3b0a7b7011723d1bab4233c2755bf469ad2f522c831f9f7021f301bdd7acd8febffc34702e25dac1be97e7f41177912ce303be94efcb7c99c68cc9e70e9ba6c3436d29cb759abc4ef11984fb27f72030535a04529c3d8031c30014c2774bdd1890ed60e47569a3969c3eff5d978b154f68276b87a2056fea16a231a5a84";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 404;
	dataLen = 4096;
	combinedKey = "a6a663f32d1c56eeed1ba50ac158dfcf06331013ede68d0209698bf54676a9f3";
	iv = "8cfbd1f7baec27824c47c980709f4860";
	cipherText = "802b3577d948c535dac371842441eac943d1a84f9531eff01124d7eb9ae9e5995664550161d468bdea32e9b789742b4e0f414dc16ae61ca3e002458bcc86798e307bc5845292aed69c8374b4a877170a3db8db8ac955b731bc977742c7faf3cf85061eaadeef7a6b8d54e352cd1125fa0d69bc429f05017c1cd9a2f7a5bf231a6e1b1e1dc2ac3606ba76534a68872af4ca27819ac2b17c789669c4121c32e84fdab3d2a71af2ee3634efa94a6522e7f5fcc3229f641c58fd460df7e2a383ee4f34a7b6f7424a186e6c17d9018c27685bfc431d0b3e313861837284ebcd73162e5839d7bcd479f12f5ecad3d6f3d4891e83cfb83ca843e6d929ed6831639ca01a84bb04e54ac19b3031a136cbf991822af0ee66cb1a80e764d03cb275e5ba2a6499fa3bceacc08f629ed4cf591993c9515e9dbff1f2269c194b696b72de5b61db3a9403986464e7c90071752b9912d46f7c3e1f4d4bf96c9d7611d0175641b5b0838c1eb2d52bf9b2bea7bfc06a63ee08db92e502da05e4f2fe89d69894847b7f0d572a133d4ea4ca0cca212a0ec5e0cd49f02de47752810d5d71f6defabd0317dc349d935ce97491a0535cce756b80e4d2b27fd236d1a18801334220ffb548081fe28481eedbb58b140c12351b49eeb320476b93adb1c963a7fcd90d2b886459d4d9cd5c458b126fcc817526e745029c908b99b752fd43ec4edd38a066ceb39b";
	plainText = "c15108636c53e25ea2466cbfb3a869624438ac155abe4ba1ae5df51c9b3561509083e1f6314d53b2efeaab4ade5add8bb5bac58f367b431ce8d0ef7d2cb85f864dd837d18a915865515a0b1f72576c96d892ccf1851dd2625b74800632941e3d2f5bce1804092a01446330a7b7c96427f1c1423f260c7b75ea65b08f6a5d191e9465b96a7d99e7fdb4eaa1a9ebd618b754532f3bf7d9038002ead2156ac0360dd60aebc6096eb370968de3c0acf1353539bafaa46df39d54481da77530c4ea6df591d291352bfd6262257c772f8f2e9dbe1a54f7cdb556d3a94caea9ff52252303515a9882bb70d107953f93263a4f82b8ff495f2411364ba1e10d2c4e654e376014b5130434444c5e6dc146c994ab990a0846c79228b6a16c3bff8dcf9566878ccd1be0a4d57cc32478b96ac33bc7d51e75f5fe4e74b9f9563abc72aa7a4439159e5ba16bf7d6df1aa497799b524e6aff76dce25df2ab058ba90008e58b1c4d4066e01769afdc8cf34206f9c25db3196e9afa7f30a4d9bb779d6bdcbbf2d56f42efb1da18b162947824ed95616fc73abc92adfe763e5d97956f7fa9b3a49251ed0d6cfbceb90f7c297a48fff55e08922935e5e24c63345f69d55d1c0b1fa2b3a3d2d45cf5905c578ebbfd2a8caff30034a1652969799a20439548cf21392453d70bcaac102224e136e853bf8231713942e21edc274172692bfe253a8e54b042";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 405;
	dataLen = 4096;
	combinedKey = "6766c79d788eb1d7b2ed628fc5778be1f19d5ea057e02eaf83c05507f67b1dca";
	iv = "7584a1ac1ea134814a64f49b0d1bf4d5";
	cipherText = "2b7cb06f53b1bdb5563e5def938f57fff305addb8f89eb35bfedb082d1c89d284719111313e05b5a611f78468d88ddc3a937fd78c334dfb96725037f39a4bd22fe88e35de89d4d66b91f0391644c95dbb66b1325facd6f19ea2af54b57486f773fe2514ed2caeeb78dd15c6973ca50f48e573d012196eac31239d8b48b18bba2112a8caa67ad441ce36a4ff88d782776328c8783c26ca0496637536a7b97ca5bf00ba8ad64fa198eff082fdaf9144fe450d51e45832d7cf52ee6cb24df7e3384189650bae2c24e24550c38e47bc39b6cb70ac9cba4a57a9e4ea193a0204e7dd766c47b61abc6a83b779f2f8aaf1046347a607b4c5cafaadbb57dccdc915332ce4ee6d36dad1f7c4c488d6d13d5cbd813964854daf14a849ff9552963e5730da55f89582c78c300050ed8f0383e664a8ade4eb92815309e75dad0d0f436b687b22c14cd156e091f4d6873b1b8d2946acbcbf488fc920a464ac8edd9f31231102a15689a9f5a2d3884bf19f84cc1fd4399fc8c2c2f3656e2e2d274afad2f2c661f6643da32359659e5bd58fde38810e7a2641b1660f01864aef5cc1da4594910dd5f8d12a14276beb296214d737eecb3670525bdc3254151cd93e48b9bf93a6012c66a343451b6e2c366b5d72efdb322323f6712b4e93804a10930f918260b13611c80f1ebdde89f84100467f063e1e9c385c654eb59777d50de4a26aea0f89233";
	plainText = "f73f90a57bac757be32c3087e0e9c7a960b5a6f4ae7c6838ad1db264fb25ecc17d629753732dbed8367781e77274a11bf965732e453f74e02a1e55ff83f10034ba13714c07ce33755d394fa7a75a71ebf31b57ff548deb4faf3fa07ac50521db2e48e7c1d881a91227182494199475065b8e96e5e8a53765a60abaa4d9af1956bbad9ad3cdd3778480780857d2d78b76ae3701488c026001d1e2d836f369232ffac679244f977df74f5ee73af362ffd60a918ec0745978450f7abe8499330fb7e97d860330b71f73cdea7413a5e3848088a59cfb98a056fa2406f0775d30edb51ed745b57c8878b83f5c6db88e495caaee4213c6ca024aae69c2e2ac75ef6685ecb4beabff423da89b5fe783b3e0ea75513275aba62b68ecbe5074c69e44c891cfb9ebf97a3e3a6e220f5a5781b45bd9c076e0546020ae7ae9c8e567baa685b7ba73ad189428754d19cd41c2bb80e327e7b89d5c6738d4c74d691c20bb061845c58515cbd15557ed2e2b4ec4a019443a3d5d6d7e571eff5caaf4f05e7c0d41eafafa473ed9d06cbbd99e621f5970a697b48819396e92ec8e8a2df5714311865b4fd985b05f3c27c293a5d5a8b325a66c5f4931ce279f4f24307961dfff933868eb448b769b615c78c094cd04eebf11ec0970cffbeb204a9a13f424eb56ef00e6532180b778cb56150a6065c2f45aaaae045e270bf8e5231fbee32e21bbabeb62";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 406;
	dataLen = 4096;
	combinedKey = "5437148e9daa40512b476a42b410b92d2c0850f87550a600096834e596e9b297";
	iv = "bb4e0ceafde081695deeece776d1ef4d";
	cipherText = "ec93b05534d1858c87f91be59cfcdfe904cbeaa0da3a770694f9c763ee72e6b45b52bb71b7e0fdc1f616c6baf29949ee5b3445fd72c57ac79265280f78a22aac554b1939a78bc32af322172fe758eb6feb924b9949a8de2f35ba1eaff1dc9eb96ad95d211b5e6e692b2a29a15301db57fccba3f4c7f5cef7a7a06fc56265d0458c6d4d3a633c854f87f37d218ef142eb21c16dc666fc714579002d3556b208bc0d348b5f72955a3d51b1713ed4905fd77f1847958f6d8c8a99c1e5eb75cf693059b3ef1380924817f3e635c2d683ce983ea60702e4bcb52fbec00384c570ea5a8dc427948408bb097d0b9e0be197b73d4050d845f46d73556eba6e37600e6e13ed42e6b3225b76b2859558109fbc7283c7afd6e21f9ea64e429e723afe5edb2645020ed50aa059dd12373529c8f018ff4497abb1917be301fe24d9f0054ed03d330ec142b1d43dd557f26fa388dc70e80fd762c9e97b60c62063336e3a5c07d473432797dc166c73eb5122185f7e620a0fbdc99245bf18f861b393054bb39e25a573738666a6e373458765a90742957218278ad118d9bf5315689116401c2c47c6643806c819decb3813ed6300f032ab6f531fd8b2968ed85351e87c488149c63d34265d2acbc5f623c51f7400a3f9be69ff9039574e9d80ae508e47451f24431901d26b4559b97c62058838a352cc5178e422f8201aad68a5c0f47673accd7b";
	plainText = "65ead426dc53fb4911c2902e8007560306850cb3b99a92dd5ce61be36ce9070dd8fd43f8949664a4a724d3f40d9c1ff23cbabf32c5445eb0cc61e756be25c10d857f333941e1045f2356891f34f124fca56e6c033d2100d922c4c8568b9e9e75967078903b54914564a830757a0d5501211312e8c114206023ea08c4e350dc5b368e3dccef080eb668025908011a3daffd2e2b619621a0ded7fe9674214d8d9b1edbbb6c05becc2895ce36061afea25c40af58d134e1a748f79eacdcec5589f8127e84f5836a7aab97f6e4fedbab4229b12d63c73ee8cac1b9b2629834647e816d924850626f1105c6e605fb50a508e754653cbf66d6b99e802fa4f97c509669dcdd8cfd32dfabceaa9b6a63a48fd4c88b917f514f27d2a405ee0192a13873765bc86c1a6ff669d8fc74f351addfea5f93d80910e6d08a9f93f41332ea5159eb7838488895d797a599eb43438adb59b5a2b5019a6e3ce93420cd9a198aa796b2e539846d69e7d66e017bf26a69ec76bee971e3baf57d0fae724145fff758cfa31f80ac01110521875f252e0c816154ac4bfb38199a5c506a496901e307cab380f3df5b8771c439a1639964b65843ebd402d82a550c397a485bdfb926c9fce1ddd4b6526a85da7cc2dd94c964c58b90342ee12de56f608650c38d5ea266a349183746bd9065555c73c0171a78417e6c560ff2e2b23c1ddd63d4b0bc95a5041a00";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 407;
	dataLen = 4096;
	combinedKey = "c1f6a9b439f5e824b470b5de7fafb57ebb98aa66f720849a3b9ba2c3d27edecc";
	iv = "24febe3c99a51dba8d7fb409c23b870b";
	cipherText = "6ed0f6ed80d5cf6aa6813a501bc72c9cb2b45100e7015cee5dd23ece61d0e19d5774a76c65270be82aeb9101d94e9b8fe717f8a72f6c3037508efe99b1489b7c63af31a01bc973d8e26077c6cdea6ef33564ac989e815909a985fe27665cc6bb143cdd81e8bea9ec31781c7c5d4067433525f4b43cb73820a03d14c21a6a94aa01036ee538b9a85a41565a7816dae2f9def3d1f1d67cb27a1b0a0445f2d91a2cfd1e2a55705856dde158e4cd9294b6f573eb546b1d4d2b541470068fb8ffd96958b66853558d61dc26732c7bad5168638cd629990a5c8cf1477e5f74996e6646c7e3f5156f79b1591df4d7fd0489ac3db7ed919a2f7e475a9506b2a13d7b23980d467991be4ba076371a7a56e33953227d0be2d3d746776ec035a7d45ff89afc4cecc5d1a6bc08a63ef4fb8efd6651299c3572494a9cf5d3abda49f29349415cf32f27937e1a57e27c29f14d69d92266d0767ca31becce0728260ef3c1da192b7f4fa53a86b0cce8e81c716028a6e2509b4b599c5bd29258113ae8638fccb2aa492b6cf8c2679b8bc53bc60fe03f32643dc5ddd61baffb2bdc9f5d619dc9369d16c1d4d6a1375435448675b270b187094d6339d5f798315e8120dd25af4f49f5cc58afa6a00f89fa00223f7552442c619029990f46f13e14ed270c73e360b67b28b468b99b9a8124af04ce1a91d8d60af2cee8a64891a5e22b5a49992c281e81";
	plainText = "7301505627d1b42842a85d74688b52407c1f9e20e9b0a94d8fdf1fc9561d804b85d8b50a9c5ec08ce4f0d9d8542e6ba2077840ec054cafe04a7440d5003903e455d8b39f53c940b365e573e5866d33f4cbb23b65f9119d89f60273dae20338f8048d7e921e4968b230044e287ab3d384481999dbfb9f457a91739b976969a7c43f6717281406011e68345233d4de936ff8d6b83dadaf3d11b3cd968d5d44a2e191631549d3ea5cf02aa021c5b2d52a269cb8ca4165c15a109ff60973c7f8fd3d630b03ddf6cf69254d2631863580a37bbf609af6690c3091cab385a7ea513405cb866cf7dcb7b8e8a1feeae975b043f117d777226d2bebdb585363e69c4fcd6e1e3c210a930ee289f0da6df6ce20f925f88f509ea2d6b2303e6cfa6fe63ef9b0ef5d5417b61799bc216cc5ba857834e31e312c0d1a19a3b2d6e4d871f26c9da8f713acc209368fdd555e304374d7c997a5425824e3253b9950271928e4145b041f1b60c59dcb114a916c28da46cbdc81949068dad28941446e6981a659346744afa001018bddb98039549e81be6f8742a2942db74676306107d75c7de44da4dec29c3a2703437b23d74399c63d436a79ddf1a1763bfd06617e8cc83347f586c68b2ea179d046ed30235820d073fb99404fd759a63fef4370d1420fdee7a8995e82ff4aa1ca14074b7306f60aca1e93f84a8f2b2eca3533a45a3f9eb58d9cfdc2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 408;
	dataLen = 4096;
	combinedKey = "e49c9c5389be5cdbe9e356b477192b3a689b2d590f798772c066b471f6c53f65";
	iv = "517a5162a1520ce58930d5bd25b040ac";
	cipherText = "43893c7e10b80be89bfaa9fb4fb9e896c541175107f8f797e9ae4d72c14fa0fb7bd47df6d79c869a218843cb083f8cf6e28bde65a1a6ac31a588472b324729cd2378c76fcd786a3c6dc30d4b8aaf136c921976cb023e6e464a2fb1075bed72cff9b5c32e0f54b7f5f5d26306f85bf96b14a9fe8800e6ed1d76938c5dd3021c180c3d283cda4d4a63912b7c7a74325ff8c3e72f80fd7c4724aa388a0d4bb9e8c52df3b244c396836118e6d30e8fc84f8471e9dc61df48d438b265408824f7ee6d341c44e19cb9defe3697b8cb8fe517be52eecc245b00bd9e3571d6f17e7299fbf5bd764b5335865cca332d949ddd07212ae356e07b9bc7018035193000b6b6464b89e22f3d193491bd76ba110fd5b88b3521b8a9b37e2111cad7f906446024cff2fcfd9c4785b046edf1ab27a7b2c179b7dd5be137600a9a9598d066cd3313baeb8edd2374c38ccf8fc09ab7f1ed23a3905e8a595e954fc0462589288a78f1e6b4799e34be57c719997d1eca1ff90877f35383701845d5e44b838fff5665592377b88b0c6792e07f0bd3da99fafde61952bd666d4ed03ab08bb2bcbf6273dda2f79e1d16af0e08af87e09db5f4852d23ad3d5666b5d363df436e9a885ef8ec7477331155862f4a875c426653eb27932edbebd8243aed3465a2de89d2047e272594da1df217b42d61a1c137afefd593d9db907c0e92edc7abbcaf4e995c77c947";
	plainText = "eee6d228a9ff82124efd5c8ac208df5b31675871ea014152f904a8bd26b24afc8bd525a9f74ab78b0c1b77a24ca0c69f483d0dc0c302f6cff10882b48bc075310809e661c534acc6d5c7e6cc6d6d22e8b9a69d85f86148f1dd4a9b492c25134b39412875d67e98ef552ef023993141c04d7e10dfabbad3891c34b8c777cef844a8df496b9a97127fd1c9b2a9c39a802e912270c4664c52f970da9404718c11e114ff9791b4990aec03ed6fd3bcb3e4cf2af3a8f36965306a84b1f77a5ce7c8c62ce5e5d0746bf76d9a95f0807abca8b4ab2c0fe6a87b8059698b04c77398099eaa201bf69029a96aa9b538cd5e09c550a18f943ec85397773929a32df84bd9a30cf7ba58984a131b4e3952e0be8331d5f4fe76d7577443f50fee73fdcf5a6ebb9477c03988568a9974d9339444b59bce222525b2ff61010797e20e28dddeeb6ebbaede46e693a409cf339dc05025d65679eb80b50e539a317bb886533c399d8ef59cf2cd33d9b0bdccdc781632229c4dc88a4c6b5c2574953118a7ce34897c619fc3f196038d1773e8d5e80000357ec96010a0e5d8dc7a4d3a2934f5469925772ed0db6226f3b8bbe84f1a45a424950992e6436b9bc8aeadb395e4866a94cf26ef6b891676414e25941af9d0a9fd696fa8548b26545c3113fd58d14989555c8ab0b937398dce10edb19cdb25556a4a9d07c8a989effe8cbfeffa499e354e6dfc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 409;
	dataLen = 4096;
	combinedKey = "05e9deddb073b8d8898a422c2864bedbbcf01f3588ba3b10f7dc06e33b78a5ae";
	iv = "72d7c8e450a05f1eb96ef4bb30e16508";
	cipherText = "06d4224bcf08748869c7a5008d0aea2d7056f0024ea4e027a3fad91b6c0b8356877e2451d99df85b9c3bdd99f5990846f8e1d4e83f2ac15b14995b9f22f5d432505a9fcf0985f08f3d34e26517b2361c01f97c06405a1f463b0cc0ce2c00be6018a640878f578b41985dce318c3df4dff4821e1d363c8fc2fa1396bc2dae53015b0379b741fb50fe7f83e1e025c9e819846bb38124c45dab88efce8451a639f36fe92cee8aecd1a3cfa78134bb3629f1469d3148d82bdf348a9cd13bfa29296f8ba5689be1bf4a873dacfe78d3c38546381230d17e9fa1255f3901fbb06d8c93973936f5df934ec980f63d43e94bd488c72fa9c8673e5692d3a39df904a13485cbe6877037492d041294fe93f1e889b4ff3f978dc8c592b12015b8200e87710b206ce9ca999d8d5f3e9a458bac29978108870d0a803748fafc9a976836be2a4ed39c6d0eccdea9bad9fc2ce2c717f2919e97b8772f28640001f07620ca514fb60acfbdfb18c4cdf6ff6e0a1e1f82fa12cb5099531efda3763a4a4290f22aeba4caa57aaabe6a03cf044234113d2b0b61d8bc60cd04946eed75dcee163c7c3f08658511f32244d31673aa4d97c48dc30e30af2aece91abd8dbda80ad076c41259ce01d42aed7da76968b81d658ad40c156628da62e3be2663c1a305ed8e9f921fbdd7a339b5d402076fdcd42ad527cc2845e4c7ba70fb8d943a2e86d42265d6dd";
	plainText = "c4da06c018c0f95ce7dbbe7d530a784ac6b12eb4408baa1facbfd441d24fdbb811f2bce87b1a770076ff1b75aa51b42d71b0779ad96f93914e97a5424ae6a8aaaf06829ec1fd5f49beb512611b776a4c15f5d86595eabd74aaf102d4530ed8b3cf2ae4711ecd89c35f550698b07c4e26bc4e5474dade1a0e5379f89044cd6b572a0518a831c36ba0ed74b2b89ebc87ee7b805370b596be6632a5202f69002b2408567bb2a4e6d290390ff5cf195b9310c88c6ebc1540b08acd2beffc4b2147d3266df7e26285ad3993ec38c02a6c188170206a321f53b92841878f2e19341350e944a3070ac2ecaf630065127ad0020c2bd1fce0a6ed33e90b5ec657b5db61388beef0d6a08b994468f2f50709f71747f801d6c339f8f282531fe7a740ffce3fde1ac319be8d89914d28a7bd07325f4979d78557089ea6caded3aeebca9d5ea66acf169ce708841789001335d2415de0cabf129a1198e4e6ab5d45a264106ef233e6029e0b10124bc191a5b852d01eba8a25d0ed383e3dd0c2494a863ffb83a9bae3e0bdd4ab3c2ea8a2c73a87cd5a75d0fc5835450a637811309f602146c4dcadeabe7cdbc0ed6b3db1109b8c8630ead691207d851c7b99ba7f39fe58d6979707c1292cfd74ef3966b669e23c51926e7a6753fc7f0f80223fb266fd663f6bc760b9f4b1d9dbf78d3f4d631457f66acfcc2c90fdbacf19a90dc5016ec8c984d3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 410;
	dataLen = 4096;
	combinedKey = "98a45f729bc7b8f58fa06b54923d961e25d1367cc73373c1aede039220b5ccf7";
	iv = "650ace9261b276f9567e0b1bfb5944ed";
	cipherText = "f0208448a7bd1322cc03d97416b3f6bed98dd447f97a2d269b9e95a885e64c7285389dfc58ec9db2ecb36394cfb74b3667fb71d3583d7baae55af03e5b1b38a5e8978ae52a03e161a3229992147b4afa7ca32a0cc8fa24d59787f206e71285a7703566dc231e63546970b92640d7f6262dbbf05a973346b2035c8113c4090f766add8e651e078d4f255b7e53355f3b92b90fdae4ab0a47a1285fa93ea5befeb5f5e85214870a328f33eccaf49a9f9dc2bc924032c9c16e6f39e5302b58e3ee17d4d2c569b41a3b4edaa60822e804f869e155b36e873704ec336502241e815ca8b3b83e128b39870168048151f5535bee36a7530f03d812de171b462db81f04018b2ef4aead73e044de49dafe5c45940bc95f4e88a6008e693538343e8285d0cdc959600f4c7cbb426ea2829d3653e8cb427960d2d22bb9db4750603beb33dbbe0a6cddb7a0627e5b813f1cf54690ebf3e4f601fd1a07848fc85c744f870086ce2be83e52ae25a841acf7148ba8063e78070e70ce6a884832c67a6f1f5f3874204d6d121e381f2df74045d2a0746f3c1379101d0837fb9728342a5ee1ee0a2164adc6b5931ea3aec16b1d75364cb929021c4b9fa5bfa6956d65fa28058fd308885e5c888c5dffb63825786f8f4ffceb662f163d45f2b4c41848ce80eaa276ff392ac6ca681b07971662b6047423a25dac2934f44413aa4cbd2708f662d96b53b1";
	plainText = "dd5df3e5f91849147b52b17269ca46efe2253e65784fe7274eb0bbeb2f96cbd2973540723b9c5e1bf4ab838a22a21823e8e71018c71ca4db843483b40fce7eef19725f473aed853e1cddcface5ac3352d4a88520f589b09de70e7c8498776284bceede06b0ce53aba6bab0f1fae6a2e74191f67c995891787e57152597ee926d41109f41dd0fc55230400e2971bb33dc4c30cf19b399b989f824575aac4ab710cfc123e7a7ea619ecc9a75bf3795ec44b952a08c0543e2ffffcb14ad9500b9d8aad828921e8f9c48f00415e6c26399259af0343067834bacb92a1d66a31ac9379dc354728e0cb18d0c69a63c751b3711f9a2fd609f4a55ace9706cabcb52f026058c21181c683de4a321279fcfa3080507526bf0517da8a0cd6ceb336dbb6ba30566e69b533b643a10dd2c05f4c71e7535f287e912ba51642d4c2d44b324e96d676e4592d7146ef360a5bbba34bd1fc4fbe903b34a02db2cae3599729b43bdddfc1f40e9e961a5ed73f5d4c251cda2fb44e6b711a3fd51f60423af7e9c3072618970940a3dca32928a79feac51a4d09397e0d57e98cc742ad611589c1dcdb7553d47e968a5fcdebf6a3a2d62b588d492b8d1362365c9cebdedd652da2581aaf8b889811692cc104ccc2f5f074b55ba31c43b6d32ebfec4b0675bf1ba7e60a0753f9321809d4018713de45d5395191a47a36c2efceef23e6c47627893e3f0527b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 411;
	dataLen = 4096;
	combinedKey = "d4d472255a8873c6469c75425981de0b09beb35ae7c431dd433319894fffec0f";
	iv = "dc6d784ff2e17cdf08e3cb5729a30e01";
	cipherText = "ea75e723c9bd96ebc1d25920a8a42e436a7c70c2816971aaa118fc9950b67bfe4c166f5f05026e20b86b118a3f45775c1e9841191f1b53f265a933271b389ea3d55bbc489af775a767d235368a98e0c30209727f9187617182df055579939de79396b75c4757e7302fd214dbcfde73982251fbbd16bac97689419cd88e9299528d8f371c00613d9a24347bed5d62f11f984e99184e8ab40c172c0e57dd519eed2454fa5f066c3dfb9a5cd683ba56cc2f523e05537d165e8345fb1a2f19dcfa279ee711cc793474c8cdc55bca1ddae5bed02cd925e7ac6dddda3eec4db2a663393a147af9cd047da296c2ae021a6ae156b5140febc440bb06ed56d6c4be9e3e7cecef423d236a00b50c87cc26bfcc3816469da6ad48fad433a1ee50dfde5886503ea5427a611e3b3b9a81ccf0cf17b951a4494057706881399ec060c6151ae3657b28be1873b50d48f83c15f6f52da59f76a2c6df2bc979e80efa90d2be05c9f8345f270db389879e1bf967204f47e318011fa0fc921c5264ae7a85244a96d6072a1772db74e9b2c038e934ab7d8f18de9af1979bcab3ecad2ff984e9ffaed473ceec530334a479392432838764a5d27503b65dc27358154d5a3786237e9561152853b2ef32db1c4da7379f24ee1eb77599dcbacaa0bd2d4482fb9d8472ead2a617d948587062e8dd55e37233aba842465996a90768886a3b98880bc7ecca4841";
	plainText = "61a59e05bc2f779c1682192c63373924d845360cc031e8588de238ab63f92c9a5d88a9f607633f1ad21a01ca2b6311846fdf3ad59a392b718314fe5c3b9535cb93025b457e5829fe14d859d909cfc7ec8a97e1e5613cb713375efd04fdcb244df48477bd7e148e1ace187f04d482e4b1b2ddcb76f1b3aa1c0b81ae40f020e72085277ef32b765c2977504a72d47f2fbfba77120bb5847cdd482cb2d0799b739aa2a150245e1bae94038b2d03b00aace3f951a80608086ce396dc939ebdd12ad8b1e22e9d747dcf30916022e99fc5b770507af4f43eba075844d270abdaa2d0146eac70b039836bccb14871acac18351e1ec49c062079b9ea4f8c6cfec57389015b03bc1266e279946e35a2fc530f4c22575c055c5d73b7d8f9a2977b072a38e2e4d4c712046e65d8b3572075bb9841a53305b6e53dc95c2fcace986f2e19e4aae4f25dc88a7fbe286b8eea9cedcbfcfd7df3be4861998f7f3083ddad258289f9532fdc6b1b9ec9e0dbec69d5eab7f1566007cb4be848e32265289c432e47666a76aba39a45850876888d670f37a41206fe35a6f8554be577d57f3659eabcc3d1e8aa4d2006bf2899c7420e4f756bec4eb5d0fcbd35e4c876eb35e8c5eb0ace2cd8b8d9baac125a1c4d7a77b5ade6a8b07309284a37743ffb4e6282d5fa59654d12f0942f890362a20bccf2b27611b921db52197f7ed2397b91e33405a34a4154";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 412;
	dataLen = 4096;
	combinedKey = "5d141972dd23d23bfece87a1b1bb5e46451b003b702b1d7eafdf6f8b96031899";
	iv = "a2277a33424852c65a69a4c3879e1554";
	cipherText = "44c3d5e0fefd2a0d298a2d74239fb600c80293c32da43baacf669d308b3e9424baa5bc83983c4d1cc35a37c4397fdba3e882f25fe82d7d376e8f843bad895d3a761e8c32bb0eb66b3beb78ddc7a33d6e61c4cbfa41b620b27623dded9362c5ddf569518bfee3b78e37605759bf73bac236b0e3766f698f0837ed46a8f4b921e2fe8e99d6ec6e3e68dd8be7b3396a72c5ddf24ea2c917af7032d1ce0de3eac8668327582a8dcd51acaa8392a5d2d223368d0e4e7b11500fd1ce9c24dcc65d01e6c7ec30719c38782048977df72124722c2c5b61be4e37e23c225693e8902f2576256a30a3b8d63f236eb192468d12ac4bc10f3f91e0530649eb4d58dab58a5e9a64dab7e711ab7c707dedc7df56e56bb0eaa7a8872adf9dd014c1e8b965a44c152e767a9eacebd2cd2619c72676d9209a3fa969cdc3a24d03a8d8f75bdcd55eba8b25af9784fc694c99035c6be7a4a659f7281f8506958e0bb3c80a7aad6e88809a2b6afb13a10c804b8013c6fe66845c3ab9faed16aa5eaa2192fb126a66a831893e99a60e11cf0a97302f49152cf68ef6082ebc6336fdb09fe7b87270fa14efbd1666ce9ca1d3226469a161ce5918050fd6bb73a30176b07a77b38a9ac3015ad8823bef9275d5cce740f5111b9bff3346c7796d7091b1129c310aa17a3c3e94fa9f36124af44733f02fd8792e5b223c48e400159e93cc4d0a539fe9ce105cb3";
	plainText = "38c841c74628937df1dbc1b6dbf5e9f660a86fb7abff1ffffd0e527fa627043cd7519c774cef991165ed55658d746a585b6942d15eee1746336588be61a46da2dbaf5640ecf0996c8f2a9a1cc2326da37c4773714a3a1eb56591fbecab45f2afb3ff1e6a74c1c5cd006d62508b76df145b7f6fcc2882df06e2cc135135820fd188223a0ec55cc9cffd1ca2b627d3cd047a34ae45184157bcda6fe83703d19c288e7f747fc35e9c9ddc9a3a9ba8a2abf66d941240ff70760782b2f0724aab5acfa2e092502a0a72e33d8db4891df3430faa07933ccd6b5201cf1458520ac2f7573fb7c5d06802db659fd07bd1a3c1ec942f975bac2fd2ff68e5417dabe63e45a70837f418cd251999e5e348d8f4076e6e17910d550975cc5acd01ccd9cca60db14a50cdc21d6c5b55f94d3bee3fc7563097e3ce07a0a30170df0bb960376fc7b98a07b2cd2fb23e333c3491def9fd214a62917849756a430645744260ccfc67d7aa6f8f703460226bba8329aeff8c48d72457c7355d9561ddf11736814b1ae13a3d18b272a955c26ac1a13ee19c170ca3cafd00ca1f23b944ceccfaa763f1a205d1af91631e1a65cdcde73912581b793ccaf2d18f58ce3c9b95117106a20264e18c504945ea5af7440bf59fd7d275da0210864804995098f827d8531d689639302905eecd655577399f6bc486a1eab02ae2348494299379c93caad8a874568f86";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 413;
	dataLen = 4096;
	combinedKey = "72ea7faefcb940e380e84933f26a2f342d91e417494b6328cdba202b636ec37d";
	iv = "f00e432ae06276dac9f72f004925042f";
	cipherText = "1b0bbe688d97c5fd14ec9552a2ca8ae4d43d32fb2a0f90073454fde0f9d2b34bc3539a93962e26e78af574bb5b2d6fac7e65f69305c671f7b317180495c5b2b4d91bdb10385c4af90565eee58d448daf68686603a460314a6807571c370f102da9213b04f82602202c17c6024cf06bf10731174c110c6d7f21e227531f7bd2b2bc135d3910d49fb733e42b3d94fa0627a8b0caf9133d1cc62faba41748d6bcfb1ffe7d961da1a2975c3135d1148eef53cdec9911ae8265fce293636dc61456cd11877d4684e5e2ea2799801c05f15e4f1903646212ec5044ebd8ebd642a1e97c2ed4f343bdb051a8ef0355a28e4e09c17e61161c429c661163e40f1af331ccaaf1e5bb35b3a09758ba9e02d106873dd530c2bfc59eb3677b6b4c478e55afed6a187b2d035dba8f4c758ced515af50511a03ba4c78dcbd22682933b58cc5b073852ac613cd1d809722382d0308ca274dbe2c8d4c3e7e9f17cef98d09953e851cd0bd103535fd5b6596fdfe3b25e4b6cb2e765c72617aa62c1a57fde631ff62d66d0d39475c8df451edb74c127854e1545c1961fad2c9d7fbc051c06b2d3dfb6b0c6c6e6545893f770cbde3bac1fa68836daac03e1d71677b609a11f091af71ab3a1e43722301beded836c183dedb67fe8861a088b0d42ea718bc685161d4d6591e6f4729a02dceef288d81025b88cea7c427a6881d16f6d60570779ce575b41f4";
	plainText = "77fb2f41218a03d8881a6b283b1aacc2a522f72f04fe3c491b35a34f04d8664aceac08d532add623313018643a359ef315281bc35c58ffede8559865e615170e2f7b42d2b17b8771d4454367ceef7d4a20560d87f06371dd112c0b7aee9b02b767f77074a4d3097cb0d7028ebbf6d5054f0335f4ab02dbc9c514882e1246c58a3b362c18edf1e3266d2d7446c05893908773644e94f195c56f01529aa674233d10bd316afaa15a0adbb2a809cb080bce2eedb62c7157c1f894e4d39ab7dacfa47dae3b7411de297cebc733a74e6e9026619896a12e524b8fb4b70c872766a2c64b55df9c940d6a932f8af2c7ab8921f330afc98bf7ade52379d5d7407198926e4477e6b12cbab9a86369713f96e02976d0b50ff42494b0762363a1284ab0d782bc2971ab700167781012afb1c716a76196609e95c61f954b8a37f8ff94811569bb211a2abd1c159321e684986d391917c084f2b082dad8c62815231e2f6cbe924440206df3caba63c82466d24279b714a34cf35b46a30246ff84264e1859a4d3e2298991846d9239736da584e435cdf7b183fbae86c03804fb41de9cd379ccf912c3a902219ea261d45052d684625858fb434ca31f923483f5ffb3ee1944867ecc71b3442524f91ea1675380e8fca16aa9a8bc815f77dee8615f2610f45b7c9e63fa9c2b23498fae26def794ef0e36dc2e51801c44c862b6360d1c6f4e5d801e";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 414;
	dataLen = 4096;
	combinedKey = "d9bc5729526a7531eabfaa25c5e4e35ce9c5ac09f398383ced72fae2a8dfa4fc";
	iv = "26b2cf04a74d2496d4513ec01c63811b";
	cipherText = "b03639245cc96d8ba830f7c37e6257c6521ec6eb84784ca07fd8d092059793e301400e0e2a142e90cd10446b8d7a79244af3ab375efb4bf968e110f89a0ed764da0f828413fef94c44810d19f8a3993ee00b19b04ce1ae879411770d7aef677fd5e3364e1400ca53396230e9cb24210cc7751a70ab528655be138bd47e5837f2ac0085a9515d81d8072f1d72c1cc8b982bbb8faf6359a455167549a4c3237aa69fe255189a14053310b7ce24b2b8339adb335185b5d61b64b9aaa113baa6e79655cf3f7caf730c185458b312e187ecb74db8030488469168edebe51f7a4df5556ad9b70056948e823935919bb25aa18254c7512ac83a934ccfda2a5c17c7a59dece342ddcb67b55c84ae93e950252048cf42c8bc4183dd0a71fa1b076a1647dd9a35cfebc39e25eb94014abaf959e9c2d9c4e4cbda243a13f281f2102e88abca98388b53c3984f6bb4481241efbc2065edcca758ca6e797099dab3281213d3f13cad99e1b3c2063064035392fe57f559009f1736e18e66367ed0e98e2b80ef93a02ee7f2167e7303495d6062a8456fd6ecd99dbd7c1a269a45c376760f699cb024d00ffa2db346837bb0b7468147fff0e7d79bca45fde1d0db075c39698b9deb768f28c7443d823bca37225529b39bdd7cc3c7d0cfa6cea3d7b56af7f328415a971bc94b9c2dbe43dc48c0cffb2f098bba816921c30e69756c11a1304fbeddb5";
	plainText = "6fc8c39a448ab3cf8c0061a10f75fcdd5b9df4a0aeb1b8b94eac79f4078e7b8c96e7eb6d80ae959c7fb75ec503a503934b6708fc1849be19e73baeaad1fc32656f08eb11ee4f99fa0f2600da92f1abb6ef07d9d36aa263c3fd86a6ef4c1dc95b5579b333d3e0e9e8011258caded11d1d63998eb3f7cc57f05a6b65b378e4fbc948a2847dc973397bd6c2ed849f2cd6f5c0f46c736bb7759761c81e6d90fc305931108e93b59d7fd9aff245f84c0233aa7f874ba855ecd3d42cdb722755edbc3d633f24123399c229d24afad9b07dc9b6fd63aa5ff215c57e1b7c6dec2313140b3c707c8a04042a7046c426b1618a344616016603b35fa4754ba77c77d52b266041cd562763af52f7d8fb1e834f1e884d0d364e62ffbdb4408d7cce963a576b45768fc5959588886b5ad18bef1a2b672c5873ac9d192237f6eabe90a7f956dcb0226c21c304710282ff401fd22b2374c36ff59f56ab58ac2c99eb88e50fd19f76714a6c148ad76903d173ef79a052ff7ff0cd4ad28e9dea04265511a8d2be0c3cdbbbea8d51a4f3b07876d466958f5521926d84ca3b784f352f0ccbd76e768ef6979f8acbf8da9e21918d93a6fb0731951652e039df293af44212e9e2575964b40cb3665a32d25329bedf260be5baa2d1173d4d2e53c8b8b5a0debe8b0bfbe79a68eefb97abea9244041ebe42630ba137cf9b404ca6098c309e6ef1ac98d26375";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 415;
	dataLen = 4096;
	combinedKey = "d2d5efaadeacc2e36882e32fa2bba6d1775cccd3fc4363180687cf2bf2033260";
	iv = "a4886c339910fad02430c78fe5529ada";
	cipherText = "7be58c0a4f1e9ede3f2275e5a1845c041863f1a64291c0f21299ab181c65a62512df0064625b5420b335bc1401e40a939fc3e201b1ddfd8aa7ec047ef1b253419f3c0eb5ce11cd098242ca3866521b2affa8173ae01a3d3a700b06d6ce1e0826d5a427659366b698821b2688971c3adb875c6ffb3dc73ca33c3b683c3e3e4eecfdbf263b0b1af8e1b14c82950d2cd897701033f0c76d180b90bcecf9863ca3403639e80bf02e3fa5eafd5466d6ccd04f128565ddc176ea293b09b8c1488b02306f1f2274c37115ed68ad0c4194b0bf0d9eefe6b293260febb99db89138af471a40f787c515dfc07f9de793beb7bfd90e5a5daa664157a4986c0a202225b25c6265b582e9b3cdac96fa9352c912c029401559ba34b339510a2d18e8acad0e57e3a68fd4a051e5fbc719bd947f63b8b5058787c04fc5961a8264d7c89a5a198ece28080f7c96109aae0aa770a973df888bead3cce59763c9c48f00fc9e80fe30cf82507f312e5bbf8871c9545a7aca23d00342c177bf367f99b0c39da17db148abeffed8cf8a46b68c70fb72ce017d708f3753055ef2cd74a180fc75015dd5e56a54fbb831a807f4868c368881cc32669f01ba0be662d020439c172a0c9b2c7a52bc1cad5d91e1184bf499723dcd2ae7c8424e01077ea88b3f3a477e08ccaaca022553add269c7c26b541da53b69ef71828782ed855815b9f2e5b271a61832e935";
	plainText = "65a21ec4d2b7b208bdc9b23f5de5f0a7c730cb9353ad1c5dc1293948041ca873d442178472973b6a560120142c55d09936619142498fcd4020d2322d0d3b97a9f04d7ec63abee8dc7db2fcb62b855bd79eccc3195f94694d95f48182abe10a82a11b0516450e3b5e847bd7b60831ad90b65469203518dceab8538a827f4cb653688dbdc75cb3a401d258cf61d5f61562586790e9b02f8ca5ae3314c798f88e9ba1ea0c2a50d8fb2fd071967dd8d72d0973bc219728205c885a792b35eddd1e7d702e70dd0284b155d2a4b4ecbeb3a60fadc125b6a43824727f212cac2acac094295193d5cd1d90bf53ab9d27cc05307499a94ef67d1bd9a15ad6a9919188c012a85221c4c0f5a3520c008638aeb33e99ea0de3b1c8049d800c19c630b4d239a8ceb65690b168f03bf170ca1cea3d189d168be0e258cd3c19894bccc8aea92ffc417d6ea50ef96a845c9ac521ac0b8bcff3b63d816507460895a248886cae55dc7eca731353eaa278d8dbc3d134c950ed47b79cc5a3ee1659fa48d88c11e25c587be1dd23d17a46a72f9024f574a43091d7da33cf197a057e52691786767b2ff5a55c673ce237d40b92c37fd3cd9c4b93364da090513c7b4e2104411b68053faea6b689a0fb3bc8b8cbb550fcdb7648d5a53155bb19f961deff1343cb44bfe5aa206dcf76051aaa51269da0c5c85ac6b2fa20cadeaab87a8d00c31b374939d719";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 416;
	dataLen = 4096;
	combinedKey = "4327d814b6240caef02ffe6c03fec130a1d4896d005f145acded67df014f18ad";
	iv = "266c48bce0e96200e07ca5c7bb1b0ed3";
	cipherText = "aa65a592851eb42d0ff5cdd051178e25bf4ae1a4602a78b61303f71b5a36b3299908741b59e05c1184696eaa95c446762d8ba0ff8dcd12c0c5db59a7595a239a26f894ab099dd1567a40e9e9a19a3ec12d67a3f3b51efbd8a8fb5d9b67102d7b09d796c640854c0a84b6b2b1c96bb393c80e2a7f2b588bbd2992d150c1f05ddfaf883e30493bd57242388c0749f5dc6ce59e23220540ced71b2b0fedf2ac91fc262537185867a0cfabbafda5c09b5b4a20d94440d1fc06b0042f0adfd05e1143abe94846e99f081099b3bb4bff21255d4b618c112eee40b07592151cd1d464df82b571674569f8c3c94c58f5f68fa84c6d463d55e15184512fedc2d6fa3903c8b507b9afbd0d18ba56e23e0352aac558f9bb6de8cc8cb32ef815ff326a321401fe22fcdfdc75345ec78922eb92262d835702bed2f98a81f8e18e1342ddd4e032bd095d5f4be0d438ccaa939e10beac868b5bdf1da973acb23fbbe3e56dbb3991510a8ba52797f391a49bfd9b2a439cb6f360142f6817614299e19b7b8dc489a9b929e0683caf1b7c361ba76a59ab601c21de8f4409214da28490bfbc5c5064e73fb44f77ddb81a5363e8d64db043891c3325062202b00bde3f627ab380babb271ed13811f568d06b5107442e17c8d1da4bd0e41ad7c5fa6907e0baa542fd750fd6e76f415e4c6cf79522c30acf58b88a2c5bc68200182bc0a54d5d871a5f947e";
	plainText = "49da49bb0e016af530ba9dd825a6673b83881dfc7152faed04c58d1b762a3f32d302abf2305ec8a5d2fcc605c899acaaeb76b68019cfa6644eb5005f8c1117b11053e01eb7439ddc2c72c8ad4935f7512362d2aa471223849d236f9dc2d9b263669d45b8c9462bd2dbb14d05176ab1f8e1291486e9ceabad2c45860af0c4f6f3bd084577075c259ba68f02fa50eaafaa42f56603edfad972cfdb0e56367592f8105f5d23f29df80afcb27da8970986527e8eb2a105d588cdeee03ba6acf311a215985d143a3fe5ac99fc004980e9fe45880defd8e158e53f2e7b93f313c018fc08189d814bd76555bbbc7e0f187d5397dcc3a01625a75f625649fc425dfa8ad5a36d35285108457142fa13193f4dfb6d2b4dcb0c03d16b5d315c353bad53e8e98dbfd6764f34f0979f5f18baeaf7eaad7def67d25489ab5b5f70a10df4e7f18ed4fa191fadabed4dc4aa599116bef6e7d6adb33ee889cb38da066fa7fa1f14895bb8fe6fc1f86f3eec41dcdb73aa47e69fcafb565d82382685337b5ba8b8183350b7ee6f04e5cbf76b06f412d41eb64e365bce1697cc5f33b484742b73674395e0c3fb95cc2863df0fb02499b11d7424d806f4d0b606a41743c7a256ebee06bde1ec0d36f3dce2430a78dcbd99d4a963aaa61526ecd17eceb5ff954d289669f9e6114a6f33604ea564f5ff85945abdef396f7dcac3c43b48cddafc665e83ea47";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 417;
	dataLen = 4096;
	combinedKey = "5b032dbc5c5e2e133f602db79260c36a0e16bb91fd2f7fab31847a9503e92178";
	iv = "ce55613b646dab22db297ab41b50a1d6";
	cipherText = "c10e9ba040d7f8fdd981c00546929db615fe0497bff9fa35c6cd8d15723054e3ad84bbccf8acaef11242a56789ebee81f712914720a28dbb0ac377ff67b1f9c0cf14b1662ec22e5d23924a047e7b74c0c581afc27c7ce76d66f8d09034b6b6d36e11589e7358b4a61e344bd08c0244fc468d1f86ad3bc8f4976e0669d3b4f59361456fbe29f9b0b595f6d954d5b492545f9e37342df2909fadf6adc26095b3217ba6897bd45d25371c6a54de4230c9ac2a571d65fd929016429fb208177b3de78843d5ca1d6c5a36c4e815a4fbab2d9ea614070c1e9659abde36a04dc15cc73fc62e2d16ce0c534cca66139428b8f7b1a9122a2558894bd0e46fe6b2cd7255310bd98e808cb362acce49a40d5989fdf9cae0d8d1ec41b1c6f092b85b023684b477eb91e0b0b0ece76f6331b69c233179e8116f2feb6adc0b3bf6fe3dc23e68f9118adcf57fe5bd24e6607db0e316e36c623c2dd2235dff84a1c18b01cfcc399201aa28d634d7f19e498d7a645754915257cd1683c799b7e62c9d7423af52ef06dc20b8b44024404f0d5fad6c54e5a2f472df53b70087980bb029397aa7a0b65f739834c7b9d6bf7c8887cef0c3da6ca074fdb15b20ab8a5e37fcd1ac175eb962a6d5564bfe369e0b0a9964c4f585f5bf2e084694a105d698f20e66ef88c7fe2e13b6632bb92675ca437d733358b27a2e3622f4cfb595548dd5e202203b809125";
	plainText = "3d40f51353816a3b46abee4b49f0408b514d46d82f857b3fd7e7479be3c5cade6404b2a10d85b57f16fbf5f513b4e275c7705d10791b7d842d542217a554129d238113fe0f07f7ec6eddbc40cb3ed354fd5d81878520cd7906b75d807f72dabaf32290c89db7b4187635e73b1a4a292b59df0be124e007773bc0ee2fb82fdc4102b72bf6f39e2e257e211c90f41443486d44f4abb63c31144674bc8d31516f1a5a656d8dfdd24cdfa4bb532d43f88dc9aed48b57f42ac04184931daa766d298aba7ba73f2869c1e7efa2d9f3645487576b0a412cdeb78150c42221fb9ada8cbbc5ae9c70b30311d85c22722565c6ac188bd6315363b5ca895d35ecb56770ba45e528990c81788a692dec152c8acf9957e2f37212ee81fc526f9b1c6e8867a10e130f15c0ce473af3b5c2c339c8de78eb1fe097327f0efa189da0ec3ef2ea3bafa65ac67d3e95356d1504df07ccce66a4392564e9e36bd54ca9f3b079f175193935a45042c984449388fb8072d21df8ef1ef921824268b42fd9114007517159a764d2b22e77a188ec8d2a5a7342ba221e526866538fc085db43ffc75d1a548c51736edc99d2b74fdfcde308f4131759723f28be466d0751d366109663ff072598e49792a2ddc83d25ff02f949547f0b0c449cd6078e83fe9c5305e5f63d94f30f5915cc4a9879f1d0042805a61544b7ecbeb8ceb36436280a30c92816345bd765";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 418;
	dataLen = 4096;
	combinedKey = "8f5b1b45c585fb12554542a283a3f0ac4e2f2342e9f11cb2319f73e8ee26e98d";
	iv = "a70a44912f7ce11472e9d002fc8f2a29";
	cipherText = "bcfed6fc78936f64efea842f7341f07052819e55662ca2e326d57ff049a818e6867cf405f47d7b2c6d09989b0d4ec3ad8bfa91a92fbe1a7aafe78307b22eb8f09d6b6758a256b1279ff15e1e5dedb76f35bae4eee762708a08e2a1e43b53b16727fdde1c8985aba4e98d16e92fb123f9a758980a105e044cfb917013232a30ef4711bd70d9d8a43eaae21a8c94d8614707954631777a262e5dc9030c9985754a6d35ad3b7e0b9662323684fefa2a4180662388b4723350d2cc77b324b51be35bd7bc91e8e1830c9ab083607e04c97b18c0c725498c7778e787b70d39e4816525c56be7890afd262345780347a52390098a81b3a3959b04dfdcc9a6312c218e570e7da3e1ef9fad9942b2e6b3e7c56626f99249c1ae4f3dbbc5dff7fa17c090174b45acd685ebd18b9708cfa6e7edcaa914b7bda7cf7354040425e54ae1ea06be6c8243e2163c86ed4e85b7cb305f292746fe3f87b2dcde36d19d5eafc8805b8a5e8b267f5bbcf9de1a5d4d2f98c2a65799a19d02f406052f0fb4f4c817b1eb133c122537b6f6421e78fefd55bd2be25c60c26443f82283663c0df1fe017786d5cdf062074a7a1b62bb55439e08aa6b7914d8b5e800dc26740f7f8b5c9bd92e8e27bb0df7cc0087f02b408d1447e7bf22f6061ba4ed6517e8b36319180ade3b49a85131d971b23b0a881589115e38d474e31e1c44dc14130f3341f8e34a5846a5";
	plainText = "0570b9f05b6fd2af17f9460ba691131ffa732861d119a0cbd585b96a3d72a4f601c713b089cb5086ceb6a24f9e73081f2d56c474abd92ee7d58989e803c0291e2bff684615cecb796687152af3596ee79c7137dff4cc0b48bb802fd0c3f88fc7c8a2248caa87944fd5bac67f0867d9cd6e22cd0abcf217ac8c4fa2cac32d6dcc92c7bc8558662db14e0ff9c815d03266afca22134b60e28dcde75f57e22ddee2fd14eeb5bd39f3e72d520cb9021bad38e57616534a04dfb30e6e27b479dac9644daead599eb1862be51b39f2abb4282f1b0c7f1889f57c361d3954eb8f9844cb7f06d1da7f838104ec5547369974cd6c0495326056b759d04b002064f4f46ae6d1640dce55ec53eb36d345967f96139664b7e2bbc5bb9c70f315522a10add39689bc73484d7ab6103ac35a14c4e02e2356d322212ec7701692bbba9b584eb76779255021846db222bb865f7e73264f303fb608b9abb890692287c0c12b125f65d289a0cbd62fe58f9e5e2cf629efcb799cd019b0ac117ab166df3c15a619d1c611d6316da39628b498d0e3c557adcfdc0ddaa6ddc61234763b91357de2fc7025df5ec35b422f24718d9fab270f342c0e899bd4f571b57430ee81b80df6710bc58403d20a1a1bdab068775fc3a6912bc33ee5791fbae1fc27e8868fed137fec39a6bd18498f38fc17754e0e5f5e31faa5d47db6fe3e77bc74e771307f16fdbc6c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 419;
	dataLen = 4096;
	combinedKey = "c306dbbef6adfd55422f0802c422f4812beea44812c5bfa0f1c48f0011a0186d";
	iv = "fda4a34af765b0518dd12edfe689cc16";
	cipherText = "5e25b254a32f09c010a31b7d4cae41839efcd6d48b7194d74f88d19d654332b7fea76391ab33e5ddada8913b222fbdb1a14a267302f3edd1a52a8f00b6f3e35b150858f585f32c5af2e351ef529fd1355bbb7cffbe3d555911316949379237d89d3206edf2df5bd21c4ca5024b2e8e9807ca9cfeb1ccd7c4f3e64e8a494414b6d70106eb34283077aec0d95efa46cc6d99554b7e0b8fd640076e2f7ecb24c367420433becd1d2e96c05395a6208271968d6e7be6136b55507cc9407f48dd1bf9780b66c980cc6b2c28358345826243a0fb221860172e24620d8d99ff30334d769c11995e3c14233cfc79298d9ad3bed5d02d3f3fbb149b911d61111c0f92d61ba82fe355fa40fce7f86a20d05af648fa58da36b7e4c9a5b07bce5e2379ace291bedd7cd3a23e3bd162cc8ca6c0019cc5c8f198c873b1215e94ef3cfc818ba7a66ef833093fe2aa40cf106afaa50daccb39b968a5d03ed0385f4c6beb6a31e777dd6cac7ba13568aae1d01bf46dc6e78e4fc22a3dcc86aab61c023dff26f559b6238c83da27264991c69ce36eb37f9ef1b3671402dffad4c3e93a5740340046bfbdb40edf5a1ded95b69069cdd9c58c668abf35d80a3d9b820520f6748069776eebce7974f929b51941445e8f89733e00e9aa23cf48edbd4c43f2d4fa0c676a2877fade20e0c210b8617b2c2892caeb71f94912d5394cbcf16e61c4b42ad17cdf";
	plainText = "378e1fd9072ff61ab7e802e3cbdd47ae47db82dda05195a8b11f7075d31b82728fba3bd61ce74442efaabcb3580f24724fc8733cb3fd49a2eba92fd3767cecc830b154be3ccc0767a987310b7f3db28c71f6bbbf76500814c190241613b92ce55e76ef222c1f9784a988dbe9af0bb19059bc6c64797b97618410870d3134fec99a517af8b3465d0c0befecd54826cd2f8f4dc32fb3f14bb1925200e3fcb135cc11d7fe5424f0b3f4b5942cb7e795077b8134e4184c3773fa1e5836a66fb167835f120ae41c47f8c1d57db93e8544b3998cc9586d4f483d068916e66ed7c662933d5fcf4753b6a268e4bc6c88a87072a5f5c7f20bdc1859c8026b0d9f8ecc3419c3a1f8764c72a3e024e9e2950b9e7165cc36fb5b33a4209f13101ca4d382d8b28bb6c7916370f26a76b46438e3bc5c6e9680f52b731b36764c27dc9c9f444a4dfba241f30c62c90d979e9386aab658cb5125c25d8c812990a42fae5d76a88f4b039ba5da22c4c8abda71c2836a45584750a995ce5e9db15d652730a042d286d389f0fe519d4573f0690bdc16e7c7095448478c85b0a7c6cdb5c64bd9f794d4afa9934e5bcb6290b9ad9866dd717e7324185ffe029bffb6d5cbed63b9a8907375c86030604f28bf50e15aaf638cfe33f26432e64eb18a8920b7666354f1225464498374f6fc723b592f763eb760374d7125661ef466fef85657c53b40d2ff8dcf";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 420;
	dataLen = 4096;
	combinedKey = "11b30f48ee2a5eff915f34322e1c1ec8a1232dc67b1c00de27e61edefd0341f0";
	iv = "e045b9a1ed7c9fe3ff8ddfa1e470eb52";
	cipherText = "91f0a7978364838edba62607f3ecdccbec828eadfd84780cc19f2c488f242eb4ef8056ff27ba9954e85a8a09571a15594360f2b929f8c9ca7d306d9b9bb131cd7c4b2a771a58f5fb5155e8f5d114f39725caf124fa5dc82031d88db2a35edff6a609735a5d684a8a33b5751501f36b94b732a99c81b415120d47f5193666a3a9d75b27bd2dac7e1c0e93b6cabadd75ba0c0188ba277a3e40c444e6be6d440a7bfd35288a0153e0aa8cb147767cbf22161c8835434fa2b6f9bd57f835cd82ed1cbfaee0dd0839a8942f4d3268bb6f27b30c5ab03edafd4841c07c749b3ad424051cb3d95febe198670197eba72c25e91307d7d71e888dbb9b6b0c84bfc236eff4305fa0f4b834b6c866f238aea9867bf41b6e2007aba4e7c14cf0bad62f9d6f033907c6f602f1bddf6086edb62482f45f75da0980ed513bc09cb1965ec1ed85d0c20d0388c3faf92c55754da0055254eb51146027ced9ce9c145f5adae7e55ee2beaad3a5020ba66d84b621903a894b927b1a0cc532880c0fbae2d059d267bca78d00233ef81749394b5f65d8fe549d51099b093c10823f7df3b0eb4a371c815f8773bf2ba00900e42fb5590f2247d2a4ac205575815092544cc3ada2491681a3d3bf11bb7a1d46265b7a5540a970411d1f91e377fca024fc9303f9c4a919180a150a584baefd50d6c2e445530db4ce60337912fd286bca98ff70835b5d5f8a63";
	plainText = "a0d21803f5ff48c5fa3eeea736f3e85ad92ac93584d568a27c67e92712f0fb35881bf73843a6dfb2368682098145125c8d143a50ee773c390c4b77957c0b0a3ee453a4890356cdc4bdb0f4218712a41ecfb51f86cc251254f45d4d3b30a478fa61641587199af0cacb4d6943e67eb951b5d804e0db15a09ed1bad33ee636ec86e19d1bc51e27c979934721f3f227dcd610f34af790f40d86d684ab6064455dff22813b34b959f20bd4fc152f748dbe43f90f23ab63e63be1bf717d35ffa742770b300d6c6060baf865f0155508e0ba2e6fc597632937eb846d0ab5f828439f72bb23ba301d43190fc48726d29a6d7e0347324be92711ec29a667bc381dbae91e7c62db4b1d18cac0fb38c44cfb8a2e49d7d462fbb44f805148946fac904b93351f7cd46c3153f344230f7c9a6be15739270ec4afe0eaff5c98c18fd0c6e12aedce735a7593c731182c9840415c0c3135958b94b2768e0d8ac266b0b916e4b8e856ac1a45c442b2a1ba81d555f2760143f47db0eec3d860dcc2db6697fe459760d9827dda8766a74df1fe0875475c5f642bfee72d9653076a339a22438d2f6c042f8cbf6bcc51ca85b97b0665a593fdf3d1747f6a1c2a10bcab7b1f4e0297d096505a2dfdeefa3e26e7048d21ab62453e344309463b9fcf34a2e230208f60c332f7cc0fb7f378af97e1debe4ea371347e1e08e148e808480220a721c1bd118164";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 421;
	dataLen = 4096;
	combinedKey = "6a7b3f1bb2ce5860db6bdde95d020fa5997dcd3f38d98d76380f501d1e9ef655";
	iv = "a7866ebccb0881b9d4ae1cac492959d9";
	cipherText = "7fe58564ddb897b795f2efd07f473dacc1c29cf4be4436d8364fb3671293eb8c82260c5fe73905577ad3be0df9ff66f01548ee2614f52d5103835c3d31e900305fe7e21e354b8bc584b32b9a45136dba642fab730aae211c5d35f9d60ff8fa240eae632ec16e793ffd077f8f02b8ad7efd31eb753e8ee18308a310934d5a767ff3f92dd9262cdde431bcc8e7b60ca7a974ed3aa395c9cc8eb72c114d7de09d6aafc2a2b843e7d92e50287c10ef053b6bab1e4d1eff31636ce60ef450514d441e03ca4e5ec5348e3d66aeb532bc92f55bcae578c371cea672b2afb7d764c0e4fdb716352de41d6d4439126bf3e89ac020882ad7607c127fc9959acb614f38f025958e90ee060796de7ff7a1d04fb41da20275fe9662b5c5fc163aa232846309234a05adaea275f5351c3418274526c7ff6d0bed73e601446dd6056b5a09a51dd94b4a23252b58a180bff1bf1777ebe0c534679fd664eeb8bcb59d5078297663805c744bd85fc4ff8f318dec03c8731b7bc4363af38e8931ad6d9610e0dfdd3109af99ed95460638bb7d2127738dc5596a61a2fa3d0392b0d3d35a65df9d01c2529cf461a273ddace2ccc1dd891b80e02f6f13e60ce1d49672942589bcf0f3f8d7108e07ecb9ae456db59c41f9bdda82c46af2c2ea7a37bd67a6756f148b97c988ab8588c157e98f3b888f7f674737e1a67cd9e8a214f9ec56925a6717f69cbeab";
	plainText = "495f315ac1fae79db1193f200810f00c476e7db32d9b6555621734ae7a731542fcc6d454968d38abafaac77a10e4bcbcb26df84a38002dd8257a52f80148ca3389dd71664bff86f126d2513fee439697975a3c9382844507bebbac736f0e76aa102842875635ad0451fa304961f9b2d4bbc8d8bf1d5ffaaf5c6f436028abdcfd866d8350ca0ccce2a07e4e5873a247595543efb39ff22b1c9b9001dbec5c7adf882faf80fb077ede6de8207785ce2fabc574edce78450a2ffc92c2a9c98db234b301eece115ca9ed483e6f222b2c3c4d46070687b6faead2d7fef5e91738ebbe7741f8ba4bb6162b6d78a1fb054443f05df87989e111f9f8cf313b140c653a103b0249da80f10d6e5d32138a944a744746fe7eb77f0a65096afd7fb5924da17fa66a42aa4786a23d78a7dd715ec1652049890dfd0e575900c4b2d1028c57a5a2cc0e1d55985f08c4c6a3ee2628ffc4aa7a832c6e83bb03f90d8c8920801083f4d2dffe3474890b46d6badbc366c02bb6e9ddbe3513a2b466908edc38b9ba1aedccc6927851c922b4a8b390c92a384cb662dbee67af0de4241b61a45d7ad5b07c36339fd2760b010274b6eefa13fb9d2fa08f5620ba87cfbd04dfb8dee5f23583bb7948a0b951847fd4f789fa7371020579ff8c5ec71166b8aa40e125acb3f9ddf22493304b569017a2b4496aac66b24d943a5b73119ebf9c367d4772d60a03b4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 422;
	dataLen = 4096;
	combinedKey = "b4a306bcf3971b7e6e1c67d8a18978c053bd8893ec5ecf05ad45cbae4c9894cb";
	iv = "044b649fe8a1370b77d43e2d009d86e5";
	cipherText = "704bf3d2b01690926d4e6cf187f8c82481d1d98cad44c8db9b871ab36489fe90aff5f21602038a744fbd4c26f49d5289a9cac6e67638013d6b7c9cc329aff98cd5cd81950a5ef624b8f3d03be268b808ccf90246f8b85c160e0dbbb2ee8d75ad0516da869f6e28994837bf95d868aea379928a88a072280afc39ee181506cc58adb9af433e8af164a7722525b1c9057d4f2cb8b442da783346ae002089b55d5ca93d2f57d35ab6ab241ee2100d8e669ceb1b68cb81a50456066e86a404b9c50a256559970d80c59bc5098e2e0b8b97d51878e697494106e6eae59b702ed84a9409076a1c175ba143cd5cd4b91ec6fe0c355dba6aee30b86b1007f673920ad1364ff19d0dcc3bafe0158108b1bb5417b47f0a5363ec8a7866d6eefffeff430400b6cf819270beff2dd9f6b8274e567bbad0071f9b124e39fcd5dc55800983dd4b5854b05321190045d4ddb278b9f51825fd047d249c4f2a1a36e8d3e9457fbdc948e436f3484fe386668320d0894d7397b5131f584a87995ddb09c24dc69e69defc33012bd03a2f25b20f1a50aa6227d805bb8ba68a649ce95f0e8918ac89d03683bc5fd727715228d39dfbe236f9f1cb1c141ca924bf27b707b5d8e9a508f76164e935fa23cbbbc22ccf558891a2e83d985b212215c719478d6797a3adb7723a3657f4696188c118f7a4b3b8d832adfad31eec8246a3c62d2e2181b4a9ded128";
	plainText = "a9df688cb61b1a37ef079afec1e87efaeb125af34392371421ab5923f4a6b7dc57b4b1342acca01329f97f7d111ca8d050148dfcd96d6ebf49306c032670d961a48449e30b0a5a0ee151edbc82d4bbc88f7c096a15a1b2b7eb718f59fa361dee9c722ebb61899f343a1a140def0bd4df0746f611257faf1eb3efe0dfe83952f2b480d73ab0bc318d5692ce8006c98e6f2e5073486c558996361ebf9419562f0cc044084f5dd56983e60dc88952445ce7193d2f858da0b43f417d7cefd2976a40bea96b014c123f44f61bc05abf7793ae963b386abbd518ef3c4485b8868789772736a6b788573049277d828add2c65ce9995cbc729249a0a8ce45b13d8fc82035a487fb8e2ea418702eaed0d0eb9b3f9ef39b72adabd8f550413a66f49566582ce540e4e41c1b5807bd1165adfe39af3ee8c77407f8a7b20fa871e58275a2c059c9fb8c8bfca4a28861c67a3e700212dde8011f4ce9b797487095c259728d156ca104162ff60cea918f5d0bcc71b0b7628a586ac71535a7377b99ebcfd53c5e0bd545a0b64ab7d195e4218204a2dea66ad2a1bd01de8f8a5ae6d3a2e031fccde45d1d8e623606a4d0ac54ceaacc02fe140a451e9cc38c970e2f17469d2bb2e7eae49e64768187963402f6a7da0975178a6ead18ec6b6305dde617920a1f711977fa85c2ae9c92e25625d2f69ab0a0d73c148deb10b213775b27625dd6009cec0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 423;
	dataLen = 4096;
	combinedKey = "0ddf0bccc321ed39c961f7c65f5dbb29cc90f93c894812688b091cbafe0b9c73";
	iv = "c0fe2ad041919b192164a99afec3a7f2";
	cipherText = "c87e141e646ff3a6c2ac591074a80bc8fc30e48c9d898b8cfff530bc5c765e2f6b886bf153b3acab8badb739e93ee0ab4fc6be014cd7df0e8630e4eac8f4b4d73f0937fcb7e22657e5e333143a795ce42ddb0f8b6d198a2baa3a205f49d3d91c5efd9720029f19d582ee5a05aae757ff4a79724a9cc4379b4c54056784c3ea0034538f702b07e72c18265b77decea83b9db89bf809033cd429da38e40b6d982d2ada1fa26d0d7f1f2282e189ac12b32139f9a93756ea9349c406d11b1340e603f5dfa5f520e5b622fa47fe0f02572e700401c4dce7b1aee5e334b74c091d38ada0a0b0936a13c07bf8c330f24a6b6d41a45e97ad61e102e3ac3a7708b0aa3107e51a67a7433a8a15dd9bf1dff124f0eeed399c8ccf4afc2719321c8f3a458e1539018cc88885c319ac66d2e552f0425f7f6a0956e8885c7c380035d051577194c52bb91ba39f493ce4e53f52d2a2d2256ca52f787bb44806dfbb88a8f5ab2c629f962664dccb1063bc589cc11a7f427ee5ae3f2e90d9b7e5d6edcfde924682f244236da43469b7a6ce9dedaa4cf14b08501d9ef1c674cd36d74a1cab28c68dd8bbdc0aeca52f90fcaff2688c062eaceec2246d5c4c1a418a7a506c35aebdc4a04d5f1af7d0df4d3d1798692a1d2d075cd1b8c54fdca559fdb91596fa4943a36d4df3a4f009148d11cd85d576b1b4714d6eaea6876e9b5535c2688a7dd99fbb70";
	plainText = "304d1ecb4ded131084c5aa183012a3d1cd2251794adceab6488a1a7e625f49678cbf4cf15642f28af0bb8c1b969d492f7f7428634aaab09c06e02d7903dafc5c3751fa58cca329359bb32e97d4321eff56a9b5c015192290208a14b52e71fc4b7174e3c7d6d776dde05a311955ea67faac848d95cc10d9afe3c2b78851b8515297a0e76ae4ca5dbb5272ee2092cb95e399fda423f200b42c7f589cfcd8596ab9b81f3bf6a0c2da0dead99a606135a0c8cc89ea92fc1258f4503a55585668e293f039de927e16b18711b2b32dab59e1a64245eeaacc627d573824973d7293ccef3cb2c1162f6d3e6c04d03f591f6d5d3e06d0c3ca8c3bc08b68aa8eb37c8dcc90518556973803ad84cc726873dc90704e6d1fce7e905a6479a9e1da19f2256acfa4e4b146bf70938298e4d803082ad7db841c04608c1d3c64dc55050185b8604f776a5faec45961169a59790a584f9ac9d881176f15cffab0f03fb0050c157cc86f63fc8615aabf2f2a8b2ecbd739e90f2f57f45cb9fcf2daf60e9b82b86957a4cc6c798a32b9c84e7aa5fb6d24a4ce0d80b8100afd943b671fa5204430a9d00bad22fe3f9ede6a2da87b9e3d220ada118b91b19b5a4c0c1dc2aafe351c224b649bacf4600d106d7229ba040f4d77e9ab55fbd92b4a857ecb13ea4437e159227cd1a45f273c76f6a915dc558b5d32b63c3e02b92282ecb227f6ec95d2eca0bd0b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 424;
	dataLen = 4096;
	combinedKey = "6e2f2469424cce439a9b39e81bc14ac624aa9ec12624948af10dcd14515923e1";
	iv = "d0ee55189cc4ab75a625a36861cf668d";
	cipherText = "f751fbdeed2375886d478edb94f3ad5df55835255c029a62a50ca8bc3a5d58decb6812dbb0743bffb335f4e9bacd7b81173f4b00920af3ca025f3e996156cdbef5a0d36ffc7ed05ea1d2cd405b95f5807658146c8a1585ab39771ae120f90e5eef2dec6fa0f54ebdf8f341db40433a7ce02866923765902b87115fe405eda977373e6919a7238cb487a1f7da005c938c0883f4d79f0fc422225734536567a82757bc039f714135f720172b450901d7153f4f9817df419515d8aa52289b5250d7cfc574ffc3e2587f910313d5214d4a46e2540ec9f8495e85a9322f6a9dcfaa5783b1f7104242e3c21e8f437b1555b41c4e88d786045ffd30b0563596c591ebd16563fcd267f05f9d792f82d671c98663e05b25dd26deb1b6eac89e8f3b4e5b656bed2ca95bf2f82cec5110c02530f4d7691ce0be80263f554eed80f0ed2af76dfe516370a481bb5692bb65c97cf8ff1af6011741b7e69a8637674c5150e4e8bee41a7c6d379bd91e0fddc592aa18bce16d4a6e57d881e00f5ed9329b3c58c9106a9592ea27f5aa567d17de2a7f1825ec7d7704e815ad679263e9f4b48e3a8231679eec0156854d39c459474136f0d0538b9242ac0dede52151ad9a584916c5b068255888728b54bc7cc97b67c392a2df6014568ea034e02b7453b543a509a20419d25a70cf7b2dae8dcf63d6bdc1487875ff9b2b9c2732fea4bce29aa2320463";
	plainText = "7a8e3f6bc093c5e36849998fcd460f5da037eeae3e53a398b034d97a885db3bece55a6608d1fcd70e5a8d3f096693a84dc81c55f7df6f6b97bc2fcb164678e48789faa3addf49506d21c261c798031733261d0b0a532fc14384099e2a0cc2cd3522ef638aefc214125340a4d81a15fef001d2c8c483b7e91f9db33f2704b3fd32b98d6a10b322941bbe5773b37b39a2fdb83d415c2422c75418fee2d981f5aee3a345c8cb257fc1fac9be4341d344ea12ba323796ddbc8082f65df7780a48c58ff24e742117a94a50225789d1bc7a804664ee844d8d18f8e7cb2ed472ae1f64945db4ba2a8a3cf20760bacf84c2eebcc048abea2bbc79c99bbbcc6ec0f7e9f950c088af305372eb6b02cce21f615ada168d345447f203c45702195fb7fb087e16d12a9111aca101b59629271322a42e1cf8c2d948a521268871504821a872337e11ba74590e7b61e1d4b187b3c1f653f49c68f33bbf83fbef268b8c5ab391580567b1c52352d9c651cce3349075fd5adc685c1f11ff6382fc8e21e9780c2f899aefbd95a2550636f028fa61a3d9543604328ab87f0d4222221e72b862f37416a41f962566aea7387aa5206863dc1c0886f6850e12aadb9a52f367daec2c4dbd1dd319044e69d7eabd5c53ea23c515fae68a5718707eceaac263beebd6e13a38f3f843b8a097d47c3517806ae4a9c9b38ae4d9e3617ed2ac081fb4e0e79a2e38f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 425;
	dataLen = 4096;
	combinedKey = "8f6dcb599241b2ef8b879d2c4c561c2ada465898b368cf06d40c437cfd3abbf8";
	iv = "8f72e54735b4cc30840baacc96658059";
	cipherText = "ba5a0fe55543e10d07c314fc02d6710af90edb7963c3728995bf87b3bff6e27fde7013a6d174c0fa3988f51a017713053831c67b16905f462fb6cc719a2dd731c10558080bff42404b8cbd10f99dea2290e8826dc9cd77143197501d40d194d190244a45758b40c0aaa5584ee3e5dab9a641a74642bc8682d734bc90678bdeb8740d06248edc54820ccc315c3d3ad0ebaa5c84c64bd76bed6ce6e6422c81c8a1764fd5e28aa632a7641a521edb9fdccedff5baf1a1fdc819250c5497db47094e5a0c8821127800c05c12543184f80e208d5422a36ce30d45e37b569332c771792d57565ffa8eb7f6ff375305b49f1bb916c176f0733cf9882d2bc66850a6254d30c6dcc6c517e11811effa464e3acd3f1bed6c50c5b670e6371bc34fa7be74df35d0522b1d6447f25106950d07fd648672139e10ba9858297eb0109ba50c002c9581e8e62b1d82586548caa21c228ad3d56e7ac2181c4f8bffb2c973bd7a580d85ad21ba33e68b867abb438ff51fb0e01f20e60f9f66805e0d795fefb9c4bb411df7879b1b8033d9c1321a85db771e28e61d558c2e5b4c2d6ea8909c2002b94195474a076f0659dee804c4a2d33a2f4a43465c55ed6805f454ce4430707008da2a8733ae48796d8db654b72674ede674583691a37e65110d600235c4c25a409049bfaa4b81090f2c944a864e7462bcc0b0c9f67aa1b9be337600be7df604b5b4";
	plainText = "f2c13038c9e3727bf80c8a22cdf0909263b75d7df880e39073918400a26a504b838285d94db153701cf6c2d38a517681e57b78dbc31d3a0354f2ae5b892789f11d1691f91d797e293edc39eb33342f25a333a5d82f45e6147cacf71db07132e9dad75498eb58927f1f55d05ae7d599bf81dec3ce0fe9d18b4935c3167c477d78108688589565873207a16707998a0eff5d369fe3ada33d1f3947516d1d9b6fe49665113cb3d9bf20131f234b9974fb1ef2ccf183a4ea80610cc1cab833a2e895b61d48673960199e6b3831c1ef0d8887a3af9a37894eed55eb50baa931a04788fd15c945f83255ae4f2e6a22b93e3145e0fcf886eb4df4fa45e6188766ce5139357b5aea42a3ed1c41368028a190c39ad87b359e6a86a811bacbe6c23f40e578a8876de37accefe442740ae4c0dd2e4addefed07382f127afeeb0fcb37679fb6b81c72facaf91ad98447a268649f7e4e60b789e50182f89342c339716cc00ac2120821c4a49675b8bdb1a892ebaaa30a52b00c56ea5f39b9659af64541c0cba0a2482009d7dc29857b74eb345b75299123fce7e010d705c5427df4a112bef9fd71be2215f57040080fa6d76e7e23bf620c9790f68f21be07c48e766d9c10c4f8cbd2670eb7e03ea1f439c98fa43e132937d125b064a2054639aa982bdc22d955482204ca46ce2d5a7c555559002285e33038fa58e3f3a2affff69fe88b278ea0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 426;
	dataLen = 4096;
	combinedKey = "357de13a4ea48aade3c1c60648ada956afa3406ef394e3549d1b05492bb72292";
	iv = "a5221e879af00e75a7c1278a176dbb3a";
	cipherText = "5cc8d43927bf768d4b37848dc383ff3cb5a518c53dc31ce115dc4455cf730cc5abb495e56eadc0afc9c8f65bb41de2b5da4dd3b2d79293c5cf21f10eb4c3b35de25ea923403dccd1628ba844125b4f71962a4ddd9d8325291c3d1b1b741eb4a54bc61c0ccd3f5f4eddd80a43f9241d4154b002d0c7795f59e32122855d49239f0874aa93595ec176333e424f093d7ea1088b5623b686c67fbe0f45a0162b6bdcbcd2cc3875dea3e7ed7a87b662b72db676c44387f8be756e71ec07092ae6b5e092229d2d95915799e0c1495e96fe217d39e3bf6f387dc08e92c14c35c75b4af2b5d4804af7af34e1f7063e9a7f2d5fb6c4201cdb72f86a20efba0c06bea70f98afe467ebbf4d948a483ecd117b8076c0d3f7b93c051b5e9c2bafe4b5dd882d87d5d2f6438c1235606eaabf2acfbc73fa45c6ebe10f7f6df13cce6f6b8fd4aac9cb3e073d86b2fea6ebbbdff3de2923fd12fa2a4e752642409fa414cdff9688850f7593859a0dd2636f0be70d457494b5937aa13ba6a5c3a43bb613b0e200bc9972f198887253a9befd0ebf0bd01638b69c5a2b6a80b29b3ff97b230ffe64e38a8639ef1b304a675c3cb12f0643940bdf2055aa091f0e210975022803a2da19488a52fbe0cf25458f6b617faed03ac8c6831d9a70075702c46273b016e655b4057328a13170971fef8ce217435f33d94ef064487502bd8204a680cb48be8611a6";
	plainText = "27d06c7208051a4d3e97bf143c4547d79f84dd1252307225e7a3f3d56d7266dd0ec007fff2c896dfadaa82a01799bd9e7f6f21b7764383a2edf6296d480ea55c0b217c6d7865ebb3fa1312138d4fada7fd3578fa49b152c309c3677c97653edd415100c7101eb234f607d78238daa88cb7c22b91706e2404a5f0b8c08d7c6033999946ddf91dadad1b9b676cfe3dadd085a7ae329484a8346e97b6202bc40dc8519e2ee416603d0d87d083811d1003f4a86ba42989485e40df1f798a3f4774c6d0504486b5421706fd31ba150432e5a10b2c7b85de2a017b12d7029023bc164bf8ecf8467e83529a1ea0c5426e7c631b4fad9cd320ff48f71022593fe3b977fd50ccb02ea67063754278fd431683635f1cbf2e302f2ff0f90fa9207d47fda0d89128acfcbae1da78776e2473376b3186cf3039d2d4729b0c420b9a4ab12dc10eb452ce8dac7109ef369db02124aca3698cb8579f433b5999cd2cafc47cb6ce85855f6fd8075a1564086b0905e94220289b7d1fdaf1575c560998fe159786a1985d33946860478f0f2864757341ac0c2e856b80004b5e1aab7f62a18a289dc5ae603b419443064d55ea6d5e202e0a1521f025ade00bf75260662558ef27b656d4ee5d62dbcaa06f2110126531b541c14c407e0872745ba51ed3d6a186c014d0c7a6bf213e92743265fd583714821e1b2504cc9e540482c9a1457f22f339c1d640";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 427;
	dataLen = 4096;
	combinedKey = "9e35dc5acbaa19cc17ab38d945b4750e5419ac91ac1608f14999d060d3863e10";
	iv = "3f32583112e99162059645d4f9f4dcd4";
	cipherText = "bf1475cea642a7d23bf6a3ae299f23ce19eeb4d66a0c3b1283df8968d7128cccc8915e6dacb878daa5c2b5ab6125b1d73336a72b95d4d9702556ca2fbab978eb6119c6c73bc928a9a805cd0fc4adbe1239707ac0c649707f9f30e9cbcdcdce0ea457abdccb9cc64630b029d20f632259c235dbcfe3a58857e51af5c954b7ccf72abb517453741104d0f4632c3af20d4efc6396a40b1db84eb799626ccc1f26df7a743ef905a078900b481de7b6bb63027b9be03402c37f9ba010c735549943d90748273483e105c6eaf64512c286681d45ff1a47019b05dc44af9ad155e909b7384c698b82117d0ffa8a1f9f1f797cd756f99e2bbbf0abc413af5704ed4168e4c96d5a0084dab4a354a4ae7e0dcd10a5dc85388fb2ffb059620ae5ec4c1a84016eca2c393a9232c157e85136971a8c997f12a0ca88df06ee2a68c28fa3ea5c9f4548d373c9235ae23a667cd1702b0fa9e2a506a3667ccb0e93f978a6812095a25a397137697c02963955fc684d11f8792e57aac0826a297220c5a1204a7a2adbdc1719dc1ea84fc154e404bce9f524bd0c1922b45c9606705a3d0f5e90a4d4cb9f2be8ca649a6fbdf5084c4febed0e6ac3aaf0867b8f19280bb939c04569b187b5e2beadd8fef19868c9af327d090f461b1637bdd8ef3c71d17c15c74cb54bd833c578a6bd6117543bc134c665a60c8f09da6a0e8bf6ef47e8f9b7f57000a350";
	plainText = "e3e33e4f5307e634d17ed9a7b2b11e86e959db3c77b6ab2ce4d38e1afad77277ef54964623e7883ecefca8d7ad4d8b3c1f3484df80d97a74b96a69fa3cb3173cf615bc20409b98c79ccd1ba544336e4607c799766f2e89f4936bc3501fc15802d56088b4862b0611ccb09ba68edff4707bcbfd63d535bde16fdd6c781f9ada242c4143333f92cfdd6dd610f1a915bd9203aeae67875bf79052d62775b96898e6281a70c21cb15b2c3b07a3d7f0cc76940167e078ab69acbb45ddafd9e7240bef0bc0c95d725e626a9dfdf7b15da30ef8e1f41e0abf0daa0c27616ee1a64c34112a153af67de03ec1f7d8c9f7f981482d781ec9157c7115c981eae563aaf89c0346b7f2da24908103302ad2644c234cd1d2a23b4a5944524a56368a0bb30f948791d880c0bc071cecbd3136ecd31849e02dd805f4084ac89c87b8cc7523c992ed58d00106a946dacb30e86553a98f9b7369f666a5e39420d8718a04ae591fd0cbe45e1f651b9c27f23123baa05bf7a5d03e2658d45292e18b1e21af38bd3365cc7c8b9b3f07a223fa63cf2e9dcb249032de4c5e7e1c4af264e6e51dd8fcb71e7f156fe804fea5e4698f90d12f9b362b79aecc271cd10b614fca323cf4f49fb54e68b08a46e4f7c36c0dae198b14f6217b5513123097c16ff09400933570c8285360a014b941baf2a7ff779994488504e230baa1178a6869bfea2ce94f2f719b78";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 428;
	dataLen = 4096;
	combinedKey = "40530d9a24fce5a0196100c327851a659b20475f272039e6a19572927984fec0";
	iv = "e66dd22ecc8ba0899b9ea023b2c80321";
	cipherText = "efdea21c4b917f7fd3969479084ecc709152404488a05ee82267797a3a02324495f9c07cbafbee631291f2ab1fd04e708b1668f95d7d8445381829cddb96c51dcfaa903ff6ac52e00fe230eeb85403f5fa66df7b7a4a04f0145f3b4b02b83eb923ffd8afc66cfad1d7796a4c80197abb9f8328f036514b5888e74761ba01c3747da2f5b5096cc2ef2e929e450ffc1da3df3de6651fc920e9a33cfbf624d94e84787e0bfe34261df86956cbdef31c4b765600850f969b4a55336ca456cbcfa81f00fcce4c2c4da1ee20b2ae34e07c97c07d13744ca70ae5b2339a7e648535e06e806707cdcfc465c48acda1d716773072144bada10ab7e7ee4c09b768103e35be354a94742c6582874cd06a06c9af65da05afa4fc80fcb946174c6a2d480efd4e72a86e66e88c152e5ed549281548d263cfe392bc8751d34194522c1f2356b025b21ec0898e9efbe9e83e12e71f77462d543430337e0556c3c4e639723167d0dfb60c768974df6a281808607e37791c61a2de3c6d483c719a2d01d5fc20deb7f1d89135c3107510c5972cb5bedee4463ddc76ba007cc31473e2f816d9b22328a080d25f223b38942db4d2305cc3dc8c89f4a609186418262edbd8d1efd6f8b35a284594bb7ada8a2907fe01a5d759d493dba6661ebc046e8fdcb7c0850083d78b65e82a248a12e932e53f81513335698b7d9d61e6f729616668f290331821d12d";
	plainText = "01c6e9b5cbf33b24b5449fbaac9967c536ab54bcb3609929d75c10f4f2638c58494c4afcfbc20ee808ec8fedba4f6f0d39b7263a35a117bd56e94dca696d60d08d928c7ae0c07e0a08f19ce2bcd52bb7b224b694b1698b412977b1a6a5f1d261a2e452b06a6c3fb55af316e7d2947cc28c3f1b55b96464430a32acfdc06658bd2cce756409905da9c39a3b6955f28cf5b1d709f1f386585bc39e3ce2ced8183dbe884d449d6ed1d8a28149bf468471c5a2d765f691039d6f22afd13b9e8e9d431d216919b654b3f537a83c45f36493372b35bebfb859988f3a3d38e94768b0e16727b1d8dad9b8716604814f9ddb015659d9bef57cadb349f0504d8b9ac6c8ac43dda579ce26941c14872a8ffe280b68f19c194630bf7076981de21ba9aff908f3fbf4bb26a322ae36c851211db2723f4ddab029cd976dd86ab7da758da1aaf864a69f072137309b751499ee0e3c7f6c1c45a9c3a3f1d0003682f9ba84d38c1cc437688304bcc499f0278d004dbd554c22a04ef2e96954eb90dcc7a2c35dce9269cb2ce81a7025e5f898b00068ae28ca38c520b8fb4bcc6e19fccc55c1f585f9e473b29c616cda9d0a85e525ffb24eb8f66d06add1d87fb05210c75f36b7aef4c274a42d97f7c6b0f3179ebc3791c2892b7bc8e901bbaacb6a6c75fa5e4ffcf30806c416e467fb6230c6f80df549c92c724ea24c0373296da68f02aa2fbfda73";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 429;
	dataLen = 4096;
	combinedKey = "286a9dbe7ca5d22da6109bf91e63dd0b765a36c99d81a4cb12b6cdc2db201053";
	iv = "e42fe6bb1f20bfabee11af9277e44676";
	cipherText = "7bd2b2404227cce3bb5af50bf062156fd579cc5461008754c6ecdd7000572e6154e1b9539dc862dedf0e62cc688587d4b3d18165fa6d2fcd83a48dca6b651fb621b3b7d616aac8849dce7276d10d7cb9322af54c4313b1817948de876aa71aaed40900c5cdc92848054b735071fd2bcf144c5e60f3734070bf20aa53b1a6b42db1f05f667868d61402a593a9f2dc5379f2f725a273a38719d36781fdf430b2958777716a53c7c481b3f51bd0003e68775d64a701fe9c24cb921896e01894923706fbab3f39d6ad89a511662186212e6b7564a9180e07cfecc6589c54588067482b23378118d482d590613d7f11a4babc15dcb14ad288ed80d7eb74ca52e3e60b0351cbace27950dd612a49af2ba31067e34f87b556f49aa222604296fb1493cb4481b178c69310432c6524c820f30cf464679700f672b2c38de54a66fb74dc66b6c9dfa698b337dfc19efc16f89c436c6f30cf5ee3e351b1a0850242296276b01125d020c803e4e1e8cd7dfc7938502590f5f24401ca393f6760debeefcc25a1fe4eb4df830f3f524c92ead566acdbfaa59a19fa174256200592dbf5d3fd73c113810e91a0cb172ce82f61598289500c8cf80a458e660b2cea12cd3ce6026857ed533f823d0ff99cbfa56b2e6f2c531ce6d14284369cc3cb0b2d6ad0a27a10b3c641804b8c6026c6058766cd21c799920dec377b9e77d9af8ab57d17ddf07cb6";
	plainText = "aa6eb6afa11d3e1914a09814129254171d9b7455ae80f59cb47dcfe695194c60a7892d74bede4e5136d4925960e85ca1634eb25e2d81aabbcac386989c82b489649ec184815cc8efcbe6947aececcf371a1941cc436a5eb850090694d67149704ceb028f413be495cd2b3115e7cc805845d97bb3806fb2f1ad785b1e55e658b33dbd22fb8c46b495785eb846be8e11b342abf92ffe2a97b826fb113c148875e5e6d02ffd65fcde46716ad49cd31b59a09feae0e04b097b5f190523a57e3b237d4a3765da02da58b3d7f1165cd7e8ebe680eed4491459e8cf8d2315d54022c916a4eb61a87c782c29f26ab5fea2bf6dc1c11c8ebf3f3c208e376bcb63cbe70234d37f29f0df4705d3252b2f6381ee1b25405710fe3fc2ebb7d500f29dda61accb8713350711a9a8205112a3db8d5e77be631eb25ada5bf43496312beeedc7de12750ddb123b5fab0fceeb526ca85757b4d3e20022dedb5bfb48b7022bca722150bf317012eb0740f157845c92373faf3e2676b8011e4ab6e58f3bbf8a1ccc365e24f42b5ae1bf1cc8d51014acf876b6299a3620846f95c68eba1afc61885d39d5ff7e62def9016c42ba4bd0e9bed70d37bf9e77fc86858a3bca96a64338e935c6f2e9065cca321778480f2192931a607e038aba2f556352fe5a82200ff78635c3d47a337b1d2aa753d746409b84d028c751818f1771d369dd0e2723e4c41c5d86";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 430;
	dataLen = 4096;
	combinedKey = "01e5da3273360cd87696e41f4920d417c3386a6e38f2086e9ed335655bac5d3f";
	iv = "a71024e4a153d64f957cb978bba6df87";
	cipherText = "f9c98f0bd80f62d0711f83049293e752ca036145ad5f693e2dc8690e5d48c6f4a136c6d232cf4d15221a4c00aff6f82ffee817acdcf5d97cadca85a25344bd348e4d7c70df3a2724aaa0c02dfd87d4a40389bf69ad635ccd8d8c4b6830c0029f0269d7f374e1c80ea844ffbaf4c6d273dedcf1ddfbd40ca315b9b1cc50ba8edb72b6a174cb4644a756bd59c0385df3a3e5984f580101707487795d9f644b8c72a2e08d3a727383c407bac65d201ecb0f45ee46fe1f3d0660619feb8a4c2049fb948d4b79f09512123f980b9f47b1e0a3d2a0978717658acc08a7298e12bf5b3946d44f702eb2d75dd95e50cc664d9592183506c6e416161332ad9283804b482ad0302dcc5d5273aaa8fbe4fb17c6f85e2fd1b41c75489287008b8746888101a076d5a09f89d3235b68b846e4e897f5695d8a22e3caffe2a1c32b067c6ea63592ba38e7c8db726c9a461aa410ff20e6c5d2189cbc9de6961866d8abcec94a7a622ee49a62f7241203a7a8de7eb2b920a962dd61950ec54090fc3b03cc1eb4430d10800b02ec12ef5355d818e36b9ec57d7b599500bea09dc655c076689b55d5fd36e0360cf3cef44e60dad194738e2ea83e4d185a07d02a9f40d8a9374a93630201a57bc297c59b463909c778c02695f6d70f7203d7b60ff5417f0e4f106fde216b67e226c69c4d67e822c815b8c7f3db31c95b3a402d04098cd414bd1c851782";
	plainText = "d341544b0765b43bfda663f14bc23281598bc4c9de5e7ce8a6b6b875833b77ae6009a4b0d7fddc5ef9570d17d5b3b07950c4e80b4ed684442ac96837ccb39407162639028568ba8ef56102dbd2af0ceb5971d4b1c97c0b16020db8e92d69ddb306599cd2cbabf3656bf70e843fab5a0aeacf8d6c97d96e20b2806133efe85e0ce223ab9e5a1253091c592d450a7c86f45c94bcae01ded17ab11545d6570ffaa2277352bba08230ea22f00a0127adfc8b5e76b4913736940458d5e30d9e8f17ae9419a7c28dd4f1324d42cb741aaa96d0921d47c02e152c7f23800b82c1ef89c6847bab7ac68a776469569c421ff0cee6cc3177df21e396d914db3eb2db238ba2889951967b10a81de45fbf98415ae24e144486309ec1487ad45b938fad853fc80df4e87e8175645cebe5d111556f401d4e0689b1c0277420bad30afb3cc058ab5f3dcd0516595a0a9efbba9b32b2b6736ac78cd7be1db978064c4f6d5ad4285327ebaf16cd86119bef81f6083db077e240ba833143563ca9d5a942a3efe0c14d68cc7af1ca2697a3a91f54c301bb58a40265c696a3e2aebd96017c2a10b9b741045ecfed14c46fb07770d629ee16a9f0515c729cd42aba2f81377be2bbf71979097d20b1ea0a8aea862e3a5d2c2a00d400aed4edcd049ca01f75414e1c4033914e786448deeb520ce48537d9476a66c10c801656e2c57e6e61c0ee91644e9522";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 431;
	dataLen = 4096;
	combinedKey = "64a28a3244894bd307ca18d50fc735f30e55fd75f26204838e7fc158d0021285";
	iv = "5ba74fe1cbf0c81025f7a1e4a37eb869";
	cipherText = "dfc9a861226c7c58159cd64d14eccb1156b2bbdb2ffd595d386b963eecfb52ee4811f5f1f54767b19afaba55a76ad313b8df1a952c58bdb0b676910d17a5671923a6422a4676a11f747a97ef9d2843faff31d35c0469833277c8077535af957bf0e1d965d62cdad45d90c6001e6ef1465d4f45aa3bd3caa3d44d732d7b56b7bc9e9a3158c4960642bb5528f738ab3a53c2ed13bd9a08f8a653e5516cf7b8c1065d6fc281321a110482da128d2bda87f3378519e20bf1b69a19346595c7b40638b8c6d97e2fadc0d816563cf7f653e99f2e9fc6ed4da99d435342ab0bf1b4465458bc86988ac0305bd2daf22e38b92522eb45175720959741637cc30f2aeadc8284254a0d0ec29280d2c6c9e3610b171c6493ddfa4168bab220d95d0e59d572d49efaabed3bf4f56e49c527cbb0f53a89eca862e6b32f0806658e3fa1ec9861969009036188dba8de30681ff894aa30524fb14b2acd13f79fcfbd383f6f6be0803fd1efaef32d018be14ba75743232e2e8fbdf3ba4102fbb5a38bc8028a865b875160af0a05f9e38591a76c372d81cbc69592d171600fb1d5f6bff217481d915d5af3b33da26514fbae3b8f5a7d2ab60a0d7d005404e679500c35f285c96cfa8f998340c4ab32ae3a2b051dbe7fbd780cabe98cea4d882cbe6a437c470836670725a2a92ddedd5bbf67e67b94a844d2147d465573c32bac347d8218ea953b8bd1";
	plainText = "9b60bb3a0dee7bca3542d4f6b343aab3680742db9a24ad4d6d9f51456c6af332513ee24108b8f8096493b08a73c2928b89701bbd591b0f65c8621ca19e5a1955834b45d7a2be425e400c75f66d40674e418548ecea6819abb80a1d20c251d9003c06d99dc93b8fae84a85deac837e9ba291d03b792bfe94e6066cacca5a064bd680306c57968289ec293e5cfb4b8a61d04b5a82f2b3b5fa8cd32196961592d2d185cd07fd51215f0bd393928a59032cc8391770d8617b3973cae547d2f5cc17d2d6f7f45d49a9f758b132f09ee519d002c8fac0685bdc1bbb111f46b0ec6b182e34964a99a5e3096167395198bb213ec0b1ec0c4e94e3d9fdc7401bd43ba2d54002ba0a17b2a2c0cd39f830cf97fa43600f6d13cc9e2577ee868922601a77516a6f77dc48e4eb1ddbdf2f6b356bce79b8d2b9370399cb147218aa891f89e711b74b7a5753a346993fc58121f3854af12f4e350bc2992437fe87040a681b8cd4a90fd882795ce194a2e1b5ff37916504427260acc06d862548d203c3cdb2e987e4ae610a4a798091e9e5f05ad0aceafdfd1985b76e31c2322a4956af044056c72da0e91e1c5aeb9c7e69d8f795b69e22091705c3d89acf2b155e83b9332580bbbe59544c8f591ebaf243be3d33e3213189f625e79362f19ababebae7a66ec0d9b300f034078cc49b1f35e2bb23d4d1a53ae4ad9682e779ff17dda731e654b0287";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 432;
	dataLen = 4096;
	combinedKey = "de709a7c7bd74868e030a94917db92c37aed8754376a2920b815b6d4d028cdbe";
	iv = "a17463d9c2fe55f2e3f423ae62c4ca15";
	cipherText = "5a3738a088f02c9429e0f95d9324657fadfe6edee89f685fff357da5649bce3326f4837547b10c7e84c1ea7a89cea4868172008995514f8a7aefc01c733dae7aec1c3cddbcacd3daa3b19717f0077843e15572fc696d7cab9c1f134be9d5b326d8244742bfe52e6c6f01a7a5967c928b82bc79d7e626e5b9e3f5a874a9199ae8f5a95989fb223b3b9973d128a66dce3adcd8ee0912d2eec183b32f8466c61a117a4279ea40ae6f2607d5c9116d11aa7cf1d83e02fda8b280a240ed523efc4016c9452f7e017fed79c67239c677f8d80a988e46398f5bfcd99f35497521ed72b4e068572c39a77cfb076bdad1ef1946fd97bbc8988ac7104d0cb1982c40e6bf4eef784c41296cdf9130a40b1e50caab87abf6811fb3dbacb96862f5f5298576856c9a24be62d89ebfca0f972ea228566c5ecc4536d7eb46766d0d5bbcacf3c6cd6281c65cb817e5daf2ffbf9e3350a31b3241ce3118d22519db99e97ac2675f616030b849c15d021bb9a4b0a635705e6c9ea7e6663e25b9e6af8535a002a9017b6d25d54ac1b8badd0ef3774b969e06fc6445eded1561a8a282f24204d02d825fcce531fcb0d84656290f4f45401c2fcdb033b2bac76d57b848af6eea361c0334172d30bfa3d4ff863819a774d3a38bedc87d1627f75e0f826a52041e33e79dd2d254a9c8928eef92a7095522335a68a3688e71a8b46af0d3d61535641296dfd7";
	plainText = "0e3ce042cf75362ad5ca88ee282c51b2c269cdbae2f3165bef5f228db97cc82a248e51a7005d4b34569fd0bb39b3ad02ad98602794197d0a8860b59aee57bd5dc5cd8e888dd8e5a8e29aeebe8a7bed266aa5a1dc45ca57f714b2990a4634e5931b58c7083d3d1153d01f67eec6318011963f985726ab98b30989ddb0dadd5f7876bb1bec15e89535a4cc01086f551d24fffebdc12430870d3193b92fa1a6f05b059117b4da7d9e47bd8aee4264ecf9755cdbb5e89db1d11e812d3e12a494a7f96c9ca1e5a3570dfa8f7cb9fd38d6a9766aa6402a70ccaac721c403f7b29455725a982f485c91a0c7b29be9dfb3a8e7a4df1a449d84f7fe06454f9829b76b5d34c9ea24e67fd1f5e397c8b7e93846745c82a87ab1c99d9e2cee871df384f7a095ac6943617a90e7299ecff08d8feffb8dc9ad4b21be2dafe9c8bba10100d9577c089c88f5e64906c1f1c87cb0023b24fc044383ada5aabef51df07c027232137d938bd042800d0db7519ddaa102c0e89978a7f7286bee48228b50c3c1f254057153fd5d50acf95d85f2dd5546980f8c2fd8079c4605ae7af1f1d4d24297813460af1e30014ba470fdd94cdc32d0b4308835d85ac08e412e73ad684555688301e761050d7d0c978d75a57319947679767e251eae2792ed554b4e9af735e6d675a4f2492122c378585ff7a61d473115ba1f38ed30aecbc9bfbcb00a7659fe218486";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 433;
	dataLen = 4096;
	combinedKey = "089f5dbf1f329f37d2c9adf1c5b61710386d22db3baecd5907b4f159335a4627";
	iv = "da29420e5572483836b8c5220665c7b9";
	cipherText = "246329d91cd775474194172c9ad353e0165a8b159630f7b377c9db580272451c2dd2ff9b2dfda61c51fdec3163851087e062c66dcf830a26258c7772599aa4b8d15705d579c1ffdd4ba472463f834d5df0ea7b66440372f3f86bd3c682ee52cf482e1dd9a0c5ac348299b180aeeaaa1625543365dc1eb81037af46f60f58eb3ec2f052db42413740ea5c65cc043acd08c610448b491d73c5ecc9dca99e0d8ea128e7db9520746de38b7271f756e4d4e9ab3af33d76be016b86bf54ff7a6e97691141ffc202cccccbec5e4e0be6be61d52bbdd01f62383f7b7382de4a4c09346ba10ba324fbbb5ffdbb09ce18e8d24ecaa8a02faa1e67d0570e4f2745612495c8511a14746290de6c081f6feadf8c0b996adc6b6770001adc1cc79c719b2fec6346f98d11eecbe5fe854d7471151bbfaac4aa3f77a7caa11ea2379a35171d1e640e8d5489a6ce427ee8b44ea1fd83be19ae1b61347fe01c3e2a9c9da89fc19f151ba943808cbe21b8c4708d5e94caf0bea83f618f17dd8aaf246e9b0414d2d352a645a64110a0543fc4c7f36a4ca6a51c48ba42405449b16f123da8219b21d15dfd413e89afbab4046a14da8bee51c1754aa98c0d7a471a48ba6e16675c0f81b542773208c5bdb40a0695341f53b03c590ea7e5aaea05b3419b81f9b7040b55a409d571c012dee6c55774df18ea293df21e69dab451918e8c845af418e6579a0e";
	plainText = "f1453298b75ffc1c866b30404f45f0d95ff92be512428b759385d56353264c0b24135e80a088ac711e660040809a3649d73518bda98e4ecc0d3c47bce9e89c4462d84442aa2f9bb58acd66106757415601b6a7c3240eac329979b5767c8c44cf0543b3ed17fed8938a0d326c9098c89aa893dbb3c8cf1618d09c05d8608a768f5ac3ad273f0b57c3d15d308bcc7437a300fb860071aa441c102410480df9ed344f8296756836efaaad13eba530a6e7fbd284c8a231ffd26acf1ee24dc086178ef747d1bc11cecf74412553a0c82347da9e545d8ba5c94c1bcb5668c2e0726129e3e925fc7dda0ed0fcbba68ff54ece6d69a368f90c8204fa9c4a38b15162b4da68a668a0ff98be85df2023a2411ba17944473aae9958f48333c57f5dcf3e4fc9677bebf29671777903fcff978d31db22d78e7dfc42eb3247b67536bf8b3fea06ae779c8885534404f648091d826cb81f4292c202cd84b5ed930b0557eee566275026c78eda4696e7d1502c40c57839c6ecc1b237f87b1e7b8188abbde9481e13aa84390b6c5416efec34716285e0690a6d0fea2ac9a7db238c1008529e3f72b56dfad17543a83e527727ef205763c5a721e18a35d358e1e0f7d0ae0f8604b532cd55ca0f3a2f0625bc8e0c771f3bd4927d33c3db571662f6af2c46e0c0b504f4cbd840dad0b45a6fecb16e6b5ac0b880c9aac8a5aeb75a1e5f6cf5dff2c412d4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 434;
	dataLen = 4096;
	combinedKey = "a34f9a5622a7b2373bbc802759c382a6752118c7d401a7a0f2f85f8ff4f9d0e2";
	iv = "9bdb962b6c189fd7bd738ea67a7053a1";
	cipherText = "5acd34594e56674b2167e69049a4d939a598f6ad32d48c8a38d41c826b7761e660dde307af126ae9b293e7ae10b48a58a6199cd00c89fbdae4368a625cc589f8af724a930d1fdd421a4378143d56cb65d65ee1f9f84c7df50c2615e6baceb247db84aecbc68f9677de4ae4d1c791d2ae13958c2b6b236ee4717cd16e7d79ba97fadb97a0ad30918e82b9781924072892756d72996fe2278327f32372d42f7ae58a8d0f534aa8c267f2ff0847c926d934a70d090078d53cc6d6943c436b5a59a236fde91c1037c4d933c451d4cc8d441ee81c3585547ea23d4d1b9ed8b5ef708a7ce2ab41124b81e279e94a087651cd91ae2efcc6a41a22dbc650ddf48d40083154357e974d3538b8a336a8a331b05c16736baf1a14f8c0522501e44efef6c30aed3c05ecebd5d45599aa9a957e5d187515b96fa8375935687f671be6d3e98cbc908142422b0d3b853c7bf913544930f5da49aaef1af40efdd75cca50e1997a82a7f0be5e9c242e243a545483ff2504bbbe06781bff654605787ba5ef50710c7884980132513c804657d860c9f31318737c6e3547652667636bf97aec91e326af24b1a8c7cbcd76e9747401a4ff04b34514e12db562c84359bc38f82f899220c7b1a59e33d736b1e1bdd175624cdcb883aa51cb0b61ead7cedcff4adb852ebde36aef415a48ca1a6dfaf259e9ed83fd72975c4737708663b2a4811f477b81a922";
	plainText = "826d0d1982467f5f51cc706074e04de3ebeae6d217f55f8466a11cb64ea304e99c11cfe7f0d598f786b5da9d7f3b0ac8f7da23d769275ff9757a6e2750abfd0dc0fea8e75b28041fe221900996196c50db985018832a28f5561db8b34c92bce6bc57ee7a05344e332c7743b3d4d02245ccb19c3e868b823f0395ccc89af3170c9fe27bdf1f93c10cc8d0b48aaf328ab6cf3b27740049808bf6f58fe6ca390aab112607ff82eecf0ca991317e4ed3ac338077db5a106ddee25d05e5fa94ba06b62dc4ce77732c4a5ce1512caec90da98d253013b1e773d77adb756db68be62dd4776b8d8a1d421c920d933850d804ac746602e2e57c91f09212c65371f4a83e76425929c0254982d404a2fae04781e58f24b5212aae90dd8fa7c969d5fb4cb44590c63cad7a34d54c5bff4e2f602639ea178aa01d34cf5559153b8405f85345fca45eb21765c7620db3d930b604196dcd96afa9cd19dd7cc5496daeeb9a3f003b802d03b5cfa758582fb774bbc2fe3abdf56e244452b582da7c529bd613776dba99496f793419dff2bc66c5b599d5bb410b4ab30ec3991e045d7a1698657fe2383c20a076e3d93e39bc05bf000f9235cc8ec817868bb011c38e5fbf06242d42602da5a8b5f5033899b98fd0cf064edc93a304a483e391d69a908b0b6c9ffbde5a10332350fecf04877542ec45ecd3d0312e39598a4f456084a08a878258258ee6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 435;
	dataLen = 4096;
	combinedKey = "ca72228c81b3beff323090f599cf3f6e1fc5cef36d918cd77e10fd604cf3b6bf";
	iv = "a2d4b024e3f8aa4fd9584dba99b4fbaf";
	cipherText = "188797b617c1e500ca7223a24127f9d25c16e58d4c597bb6d01a28fb42ecfd750ebd6c22ef6a57a398e41b26fd2bd9acc5dda7b4298562bac8f1e1077aa1bb6a288451c03d48f9d30c3029abed20a4096d8774c1c765fa12406a0a313a3bba76d51dad69edc13f7eb2706519fdd6731bde5463743b2ba75d792dbf6b73752098f7f5fafff0edeea67cb080877131e9d55b6290e789c9d9121676b75bc335106de149b60302d69c17154cab82356d1f06a7c94f1d31359d86ff5b4e7b40ab5550517fc202f7298cd4ea1966bc93395a26752e06a9723694da36fe1d115cf39e1c51db4c35899137443077f03cc37792e665ac6104b1efe37bc90e38e27d7c3d872ed74a64293e4cc5be86a10cf9521b640df72d31231deff8016ed88a022f12953d29ef71869b6418bd0c62f5ff735e568b86a41eb9e3ebf6cca4921edf6bcc254fe2560177053cd4812a8274ca4a887b404c5a487a745f72eda873e8551fff5ef4214d1fc058fa930dd22ea8ecb7902ed698d173449ede73aa737c109479474ce87fdfad756a88644e58dc4b08bae9f08fc0ab17a970a40f40f5f82aeb3aacf9d4f32c152dc5b65a4016eb8dd2b50d0d9132f662b9d5f757bb7c1109b86034ff205b57d1c9986856a016715373ee1c4f740c0ec6c884a60a90179045c2a8016a2ab62ef85ebbcbabbc5b6f32096487969971250ac1a890915018eb684540c151";
	plainText = "44cfa0e64be9a37c03e51aa78870780fa663807c1eafdcbb3a8578667f0c8be456f1b2a106ccdea9de78cc27b51ea8a611853b31ae0aa306f36a9d5eaaf9c0fbd0e1f2373e2964d35d138099b46eb5e956b334b142d363bfc6072bc646d7e95637cc82f91098e7516d1fa430cb87bb9249b4bdc4005608fbb6aa2020bc301cf2757703b197610d3884b928a5e27b4676a9258598b62aeb053745833da04bbad54155f0bc1a353712fa6d4ddefca1128ce4d4079e6d63dd3f873f7ab33fcd558fd0674071c67b6908cd64b8d9c916284560a52f7afdb2973d8e8394d406994b0b701b6bb2ff10e92efd181beddebdba05e809502894211978cc91dfde7832da1a31e98aa09692a4eaf593f769dacbc5fd495c6e5ad5ce220890e3e5e679bd78c2621f386adcb6b48071020dae7a9996d1bdaf38007359bde90f87d516fab1fbc1d19e580d53e8e0b19612c81bdb180cc0acfde3bd095aebb866261bdf41bbc672011d53689c55ace96df7dfc09ca37e4398e290cb047164d6cf632f98480de917c4e6e0a2f5d9a7db3b4d229be6fd626365166f2a913ee1cf352c8cc9917326fd2ca04a051651e9e02c47c78d347b21dd06aaf3c148b32f620e406a1618eb74288123f6d4274703637e3c8b8952ec15ec7155b32ce7baa38b697fa434fff4bbc8c2b5b40dc129dd046cbe57eadc9790d42348bf36c50b3680a49b13d1dd624dbb";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 436;
	dataLen = 4096;
	combinedKey = "3fc1305917cccdc9c2b4b20153a20cc56c1d30419b1463d455f00515020a5ec3";
	iv = "00ef086c4cc41d94674c2d31041abdba";
	cipherText = "6f729edc39cc3c7a59f2384e8a288f2d574e97985591b3a7634f15857812211022eb2989af1185f70162a528503eaf8bc173c8faaf542ae5349b3217910ba8771704d9c08c6541d60176e8b811cec4772e7539618d3136fbad8e392fdd827ad9c83aeb37ca32ebcde4998dd877a46c5beb1a13e15558906c82a75df8106583254bb538e542d40060b031ae43bed4abc8c8975bfe95ee22a7ce4699aa93392f671ea67050223ae10221fe93daff5d4eb41c308725fe63b25c61511c5f8fd9122aee36113d213f66a120462bde06d2fc581b63e00e066e8fff1daac16bdefc2710ed26bb455192f8a1c383b3787fee879828cf3c9db84cda195ae20ff850cd67b04be3c1150cc4ff8460af83e230708233dc97a19faef5aadd73d876bfdb511e739273975a7f8d740fd38aee79a07d6d104d21f6b5e07743a5fd975de9a774974313fe54129946822194849c4e9e6d23ca927ff9df93bb47f493ddbe8c290645267849dfb0b3241c134756ed9b20228dd692e540af69bc2e3f814d76f34c1d7be8ebcbf90300aec870548f03cc5a4f311262521368092495b086e5efa3efa2ab7cf64f1890942abb34004e3953a6d8444cb748d7c213e7c8aabd4e7bc9a3b85ed4e7c38ce4e617e0bbed38f72c1e22ea9c48c519cbe60e7a0ecd83865f8b7ff8263ab811f58262d83e5082b25d9f4858bf0430de84597aed59e0190713c7824c51";
	plainText = "9bd87b4d206cc150a61d65ff517a573bb6cc50dd19ce4e83e1a94600c8e70a54569f3c3eda0cca48c7de9cd6901435668c993a2e8662d700f24d5055cab8883700927ec545a6f65330c0883d601ea06617a600a7626116fd5a00f863faa18b67af752fe45849865150ce8072d61ba456a854292079f3497ca89c6ca604a32c2fe4e743afb32656294c58ffb635102ae03bb217add0ea430953c82d98395b4dbaa3c6cd46b6ed2b6fb7390a203d9f5014588b522643d61ad6f0c73e5761995494cf5b5a924a40e0530c95ceb969f3cf555631a5bddcad7aba03b92c0422cc2c9c6f9447c0b821d9543aa4fe7333a7cf91e17dc336d9a2caf1441dc647e3eca053db87be3bae5f5e7b3bdee7fad0e9e8d2bcb3c180ad3d88ef587343b90be4b1697f89cf1a476b05d10ac5e4540f0e7997715ce503b029aada359736b36093e559c337d2fd64543c681b74d728b36181f855accf454d5b81f4e9a8baa2b4a50363b6fa377f3dc4d517eeae50c37869d4943232d6fbe28eca0f7a86485b343b95102784aa08cb1b360530a14dc42461e7897168c3893d5540c0b572f3bc98085b5772bc8e70b00d8f66c60e5f24cde8ee1824052b41f2ecfde0138b5955f1632d03d9d4f4b354351a372fc6ea3813811eb6a7a61daf741025d0023430480e7a120fbb8d6d9f96bcdd48345ddfa60ec18b3dac3a562961eb1a239e3d6363961fcb0c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 437;
	dataLen = 4096;
	combinedKey = "679b34004990af1d2d04c7aedbd60688efb8d16f9e7aece61deb62eb1b964bb6";
	iv = "709359f074b740124538ad8ee67300b8";
	cipherText = "a9d16cbc0078f6b171b7ad1ea56779667652b2107e562f61d5a505220538d2b30048aa7375784fe0a54b19d36eff11628f1f3b868feb3d328b0d43626f7f5091ab0b10097b1ed4d047a1ec7596246871d4da1566c2717471124c956cebcce64bd9818c29cf13db46a62befdd14ca3992a9b51b8fad0d84919999fe2a6f234716a0158cd09c69201537de2f6ef4c22a9b9f7337aedb5f3752209948fe8b06e7fbbfdc91b4a7e665f2de09a5b91429f17925cc6da86fc72a71e542a501bc4f2fb941bfe2f6b33ca7bdddfde62ce28187454c6daa29afe749148457164768452eccb797691f0211016acbdb00e3b40c6d778bd021fbf4512697b61c250ae02a4718933dbdf15f46702f50b4fd150033f8c174fa058cffb506dd3a034f347bfffa7ae1c5f6653fe90d8713e723f177d30ca6a671128c61bf203bdee0abe4a85f22e3d3681fd22e21a6a19d042622e9da465ad5234c07482323f2f72c66ebd72ca6c9eb58ffc9bb24d4b2b30bf32b70ce481c2341aa6baf3544d804c9307bbc8a2ee270f1007cd055863dabdd843cdb449a5000465a31952d63bab5b54be94b6115da0a7b3c3433a4c333f2a36b72d548cceb084c158148def1124634f66867a9c7772818ee857d097869a44e76ff59b8854277aaaa4032519b41cc2e69f424fb15cfb5200ad7ceb0e90fbbf7dd7a9f93ed5384b360e3fb6d1ac16bfc028aceac178e";
	plainText = "368454fdaab99685a549d1aeb72a79ae9985abac1c73131b5f34abe99337cfc42608bd238d101dbd1bb9ea399da6caf1d8213c6563c88d47afe98003b5d5ddd3d25422148591c2ee45a8e45c44610a166ba0c80f15c8266e214356bfa8c9e64f3d31de606874c2e6f70bfb83d674187acd2d2c8b135da93a1aece76b8e002d32298bbbe207235db3aba24942d5c409990c59c11d8b141e92d2cf81a605f72de4012bb9e1b6a50bd019a59026f373c6aa9809210af3d0708002a10f771d7847302126fef01f9d1a5e031a82aafefa9c7578d82fae96394482ace1f3d76ffadbab5c80468d21f8cc3d26398839becdf5679ed854b657349553521ed64d51d1502dd40f9b9a9c0fd916f50e8a4c29a7696aeb5d226a70853ea4370cab7c86b4eb7e7f65cb06302784b6467036b9a9cb35cc05234f899cbbbe33544d5f7890abc32e384653dc07d52394cef18da20b7b3b506d57bd17b9f0e20f0911374cb63498e5bd0fe84bad7365b114ae7e48df9cafcff246bf4101caca5539d4343fdd161d6bd0b40cd95e92faa2950deb7a4297afd44bf00c7406f6623ce85785123d8bd6de70013e5d973e3412d9935924ced3638763a09ab1db0b6e980cd2f48037683d98fbae01d97d8f01137b2ce021d832f0434abf87e8128bcbe35344a72bd0917b1c97f6fc38ced25ff947de190b5ce5a00d25facdf3d7c9898ee01c1508363d0610";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 438;
	dataLen = 4096;
	combinedKey = "3990aeb1f629ae013e1d1bda0d885f2b9485c8a42a38506f365156e44abc1d62";
	iv = "fa79fd4217cf658fb8e195271d1cc7bd";
	cipherText = "d3e7347640adec2407e6eabedbd5d64c88e3d437947b49df7f997b49d78ff24a591044e8b6ccf5c4c844abac295a1df466c3e03e02351d97a2313f1c0ff1c24cafb17b63d87e8d94d540b39b6924de08d85ca795126a3b6246ab033deb1566369d2aea11aa18ea241ccbcb9816dde1872bc554e196f5286edaa4b8918b6d42efbd48a44bf5bed5a76bbbea66802fb90ed0732ac692c11c2514c6af382994fd4898d4fba7691063ee8c161a20d3777f5960c0e19dd21469a9fdf9799dfd86106af6892860cfe933117effd679c10f94a5cdd296f479f8915c989098a8a594726a5a7ddf36e589a68b9ad4f3b60f6a4c060ffdb8c51f11d90cf00ecea7efc60955e4e3f909ba6baf5838f436d21be78c86d011a9417793eec4c06ec868dd860e6e46d6918676a2139241de1cc6ad5f1d89e24c20c5978e68231622ee15ec658152d6ac2f62f5c3fb8c1f74bd47d4a82865c55677c0ccdbca3867b6e971dbb8b9fa71104ce1673ccd69c029b4ec745a4bcb3193db4b2afc7390cac5b6ad82bedf75ad49b007972dde3964b4b7a108883b26301b6361aa646bab5a71e72fa6677d75563d1a41137863e4f7321f03e826161ce371f73477b955847ec7404ed3b986fb686d2d5454a48e243a1cb7bb57216758f3b747d2981ae4b83f7afcef35b9337dfe5bc44f8d2b93ac5754a64591febd4c72cc2bb68bfb3855b9af6d9e2f42d1bc";
	plainText = "e68b5c91e0ac1cece1b4d4e3ed058b1c5e08a86ac46acacc043ca84b168cc1d08ee97b3a8934206102e43bc331f6fc73eef6d24493c0ccbaca58292f4b7e802e87355d956b8b530ab5641e5a3298a2719e861f0d099e36ba129589e30a01ea2370513fb33aa343942c767d9301289099acdd30c70473dac4d08c5204f3a935212a0aed57cacbe71d8b53899aa9c94e19ae5f3e8aaa5e12aade4c111d85dd3df0ef43f7305d315c858de5b3a8e836f414da22d03b57c18b2d9fdc6c40ce3a4384d2dff7fa46dd876a8fd991c1c19678f5ecbe06cb7fab1d24f96e246b63eb4f042709919561556a8ce3141dce144eda5339f512df64d25e701e9c6d49cb16d12229685643fd2a4500830f0a615303a0981014b5074b8cf834bc9d707f1e68b712fda9326a26896aa5aca2d9cd05a9c378ab90f08c5986aa7d9cbfe9d89ce78a0a1c5d0b5a394c084405a60d648bf2c2ff1584f6360fda4cbf4570a8e94fb47232efd6e184bb4339b90768d6819e4cdf872bae378f693bd355b823eba1d782c7fbd010901a8435a3bef17308fe740487581a5d8a2c3d4184a09c06d1624c40733d0adee9cdf2e9d6e543ce67e22775511ffbd18a860c55254626668525e94cc9b2798608afaa5988f3d5481a803ff8280a4e4992aca25f6ac47986f73ff6b7c43ce1ccbc6d7e19308c6969f983106e8703f4541550a1a7b097935b9ac0a20f8752";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 439;
	dataLen = 4096;
	combinedKey = "97b7987daf0625931f20502c9138ec335c2da147c6bbeb4197765ad1dd3767f6";
	iv = "2248a1e0a4cdf2965d3b19228e1b6168";
	cipherText = "79d5a875039100aafb54648fc529e63600d6f5e68ff6766f096d3d7de9be05de31ced45099ff1f93cb24baa0c72812166e2d1b7aaaa955afdf5ba4727ad1caa568b43a8f03044b45c7900f75a76c9ed05793fe43379a045d3b944313aaa2e5eaa1559b720e02d88c82d98f46ffad81b493f174f1a1c52a14c806c9d7cda1110d51c2da2e2a60b1f90418450562d36b12d3fce5e4a7e45c9bee297fab48e45e4de753fd925138e413215586e3fb525b5858ffae2f9346c89a8d40bf22a4fa5dba63940f74b3c894b2ec03eb73d08886485c11b246b72f838ec50e2ce129ef1256c8df8c4f5cb078f4a339cdcf00a55f04e5135eb090691c30056590a1791e36fedcf7ae7ab28cbf133f6e3939ec4e897a4a86060f901bb62a83292ed32245e6cb7ddd5f352a46070d9561748c64d15b61e7e742324fd4a7db4a71ddedd2d9d7f148de24398c1e11298022c8d885b2c17a8d9361499e2200943037ffccdda66562d9a59a37d4891c61439c82d24f2d159c048ff6c5e27371e2cad12080c0af500cf3fc204d9b167fdbe6a0759da675e1b78a58c205d87300f51edb8cd69ae1bb2122871cf35f1bfacbe0b1f3b505a4fa32aa1efe661b714a581c98f087c47884d606d0cafd9d14396b59ebd5ba02a60cac0c542eea278e93718ea069fc3ea99094788b7855c2f1e0cb246326a3a6ed83d3099d6bd032e5eaf4f23e26892fa034cb";
	plainText = "704ccf6e9c631f78b994fc3e0f8f8e19978a5e9ed4098c57a86ddbe576d6f0f35ad03b66f01769f3b16f4ceef26dec2b8d2e64073dbf47d99c26f8c378990674439456f89275ba00c693f89faf1058026837e09ae3066fde6ac44a7aea862fafd59dd99a3d6aba0ac5e187098999bc043e2980f0eba844b87d50f58a3484523a74f8f8b9a122bd267491ace56d4fdda45e847db2ff49a321ac9925c41553b158d3828dcddd73864369c66f5b528b45e5dcbd07d81af406d164f6141af200b31a1dcc78046527b103cd82ea90f56fa961ce825a2b60092732be66a86944fb062b8f4548b98dc850aee4133f78536b695282288397b47848f771fdb101207ded6e7472f7ef1e587f5c01cb462ac9e68cd0721c8b71bc29d09b70717bd238b51a20d677c3221f9ab40fa3f6b17e7ba6fe1b65b5db875a4fa1e830fbc0848377916d4449c6252392e6d94e97b57a0f12bb2538a7d211fcdb511b799e4e36b0a052316b9815c76739086c1992723b466550029a3c1bedeb059f1a0f5e969012151bbf88bb7f5bbd0e4f572abc5f2d30fd4863d8c226859380be6256fd8d39b8f3c4c32178f7c7d1c2f7d25a099779ffcf16fd2ae1e140888819715c66c8c80b6c39adea6267477f1741014dbb3dfd5eedd97bbe95311e8fe2bbe857ae211f96d4d8a8d7824afd9c6501833e696faa03a41ca04402d358a1ee316a17e70d3f5aaef8af";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 440;
	dataLen = 4096;
	combinedKey = "580a22ff06e08d0c95dca8501a78b9b68fae87ef5daf0607eead223815669b9a";
	iv = "6b272b1daec6cdb18f6b99b529f85050";
	cipherText = "c62e6b75799bc6f8b4932a13aa92bebd7d81701bff2d8d986c993fade14c075165a5b8314b1ebdfbfd436dfd02c91bf835a91444bc2dbd1edec9c203185950194f30762c0dd8258a891e6e99e344ae78b6d1f2dd74e26d169b9a86f73c6d6fec49d2d0a33c9fa538d40469daf56db32e7ef5df477ad0a38ef228a3d52480619003b8997e937c34f060d9a74c7fcae5c689b1d21faf09f5e6bbe4106170fff3bd3515fb8425e089b4507611418b352fdf01ffd329fc335180f2ced9c59b8a60d5a430d37e26cbefdf4dfa64351d8babc2cc892f56a72b52c206312b37e5e377419982ddfa57beac2ff6f1b007b8bf766360f98e17c0488ac505c7a8070640b8697b74f3f1721e4f54270940d84772391ca022390e9b598895fd19d8fc5ce8448c73e006c9c9228ca6de2920e3152fa79383ecc461a7b79bc4815aa4af96a67e3bce20a80aff20154ade3c2babbc531eb3b7c5bcb4c084460a8da0d4737e6c87a23baf952289f96092e40837a64382254ba7d09b7ff9b4a500a2cc9340a9bf8bc43d5f586f6841029da18b9d07d6ed5812b8f6cc2f3337030a3dcf5cef54409e2674acef0e5528b370c06c7eaf3ef4b5853971f457cf8a5a373f537a633ea3d517ba33ff3f0c1e2c8ed3d0d5f7b572864adc33abcabda80531174e0f4635b210e4e54c94dd9fd329ed907b9addb957626e3b2b40e2026b5827cb1fa62b89a3e378";
	plainText = "8ec1ce799e7940c736a352f37584957e506b844a2e92fcea550141125bf66f025b469dc3ee1ecfb2b46081330fab6dc175485a52b08432b30b4884d302e0443267be72ea0fd688c58e3824fcbbdee063686b596b039755f7192b88a1dfea693b4740fe887a3c854a7b80a8ca4d23b2e908241c5fd7ec26c668582e44f37134f80964e32e37e3cacb03e5b42aa3a2d84ed70e81b37a1392c4ff273e7b84a4f74d78f53d20e88b4c3a914dd63fdd5ad0ead66461eb1faafced1aa4e2f9d5293327d2b4c146125ef4ac11016c74eac94cb53a8c19d610fc4e7a5af198dcdd241e59ba2743b52c0f5ca14f5f8094323751b3e5ca4d37603d1cbd608dd21704d3d3e68637e31e77973f9a0e74c81db155b1d1dcaf704bf24d2b664811bae8d1a13df1caa24cac2a7df188d27f43e3d43a47124f8c4f1ee7510db962a25f429a0f8b6178be5212ffc762455a43b07f90923e01f57ba8ab1f795e93acece484e209f9757198bd9e8df400fa0c9680bc126ea59953cfe42270cb5bb529c6e94c5fddfdbce8c9873e7b8304f7b3c1bdd6b18a0cbd6ecd695dd48fd8c99fa044cb8dbdd80b070b10fe9746a7ee401386ee5e1fdd2ac414e282faa1c3a5d8a49482b27455aa0e81688a38674d9425d6d31806969b3dbbb3377530a4aeafb918ec767bede0e7ef99b8ae00550344e828cc30cf71bff7bc432ef89bc3ae269f80ae850e058b30";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 441;
	dataLen = 4096;
	combinedKey = "91e88a75159cc0714441fe4f239b7ced63c3b57b742ec1aa2d48529cd02536f4";
	iv = "eb803dd4355d5c860647c91aaa7d86be";
	cipherText = "27d9a57728cb859a837754a116c7ee402a4a17b4ddba5e3ea6b34bf986c61e3f5c40301989e4a4c71920c2810233187fdbe08f18c844296d9a9f59e714470118c26b81b848163f893e009e783e2d8f12356a1fe8de7c8aff525bc7888d3f461c4a5328bbb6f650da4820fca8a431a9174648f81837b32c92d0a13c244d9c10f2389b9f76a115aa60862120779f42e8b7db093d462b76e065c97464ec70e92a88f80ba1fc89e47dd42aa699b1cc9e0f9763e688640065564372367f495c5b58634639cb6c3c636b767dd91503e4108a62efc6b235082bd4a6647d14eb9331da06b3eb4acdae68f8b22ff556aaf57acf0e195751a3096c4fa220c2af0cd81544b8addf3adaa9fb2f5e06772c0229c7815bbcee4011ff277a7cf51aa80faa8fea39cae30d0368f78fef4c0eca64c1bb47351979ae79dc5729d7879871947f0bac7f357159e4f8716b5bbdcac686ba4c5616d2d57a20729fa10d3628ebd1df3633809042bf687f6e3690baf41968f331b20d7f363ca886ee6222058498929c1fc9c46b433ba4e885c93063a8ca877b525e23a734c6634d067c43c55a5061add5c882c46126a2189cfbecc87d7c7f7d2d16460cdd32e5d6a48d0a5e8e7233b4a0fbdcd310181f19ce181f3df8cbfacb3127b3acea1d4eafedc4493bf2bc3f285e7a83cc7738473b0f2719282d61c883a968b824687f9d86402329b5439e5b7b72ab37";
	plainText = "899c609c9d545013dfa0dba037fc854620ceca5d887895f6535d9462aa98fab157c9d05fa2505a0e5ebcf72c85b910bfa799b5de4b51f9f02229f2436c5b82d6c665db1e1f962db699f982bed48a2a1c7d24964110c1e74a498600338be10366f8e10c87a5a2acfae1fafd3dec898363b106eccc7eee472f1d0cbf27cd195b48439d4a43cce23f08478fe22a0eaad25b8fa7a6a6907147e512e08b3ed95c16d0a1049853356c4776b9c6b341aed6cb8439b84b29cf8da7083232ee2df70049590e4c9b1450ca8e00976b4c5d3ce2dd7cf7ed9e601ecbc9780ae0e1923610d7fedd702f57091af91a4b8886186c95d04a34132b720bfc9a5e6caca671413a702684c0cfa0e4123dee3af4ae2852bf76c2483006101ff5b1aaa26e6e52e737fcec5f4bd241d8756f92f7f3069142422f4ed2af3f690b208d41454dd39dcf1a16eb198bd1b9cce87b33c2ea33163b356bb8d6ab1dbee7ad144b9c13619fa79b7ecf8249c4c486b47004371e5095247498bffc4e0a3b292c73c08fd376b7c00341a46ce0ea109fb676b4d812b3852e2b9da444f4d51892f2a152ed70d9c2f83429204a81f9ea82941ceb1ebcb70355f1356aba60a57cd0100fb5dd9e0c2d7c2de6241a35a9880d566ffa6aba0ade86695432a161ae137ad19a8367f45b8e520203c6d8ed08891c038044966825775ae0d2734cdc2bbc15b2d8a47f3612caf3ef4a05";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 442;
	dataLen = 4096;
	combinedKey = "4971a00062e78c12091234fbf45754b7661a207d8fceced0c2e4533fa2609258";
	iv = "a1ac49f5a2432e2efcd8434a52a60c1a";
	cipherText = "a1a62cc2f302bef70e9e0386f3a6cd8f7c67a065e77f36b3b4e5ab9d25e16c9797342270942e4be6cb5a1ca265872f786d645452eac04db9c2c1b8f63a9625beede887c805ff49c7a3f6787cc804b9649a9ed8fe0504ac6fdc3fa6e9f64ec09218064033a0f18ed4ae5531a29aa7faa353cfa5dce24b9bda816f030a527064031cee39571d6f992ae3bd195b3e21bd124342baf47107d673285051d30c81374419bcdfd5b4a28184b2a0ac96953d1bf39f323c166df43f5f3ed8c04c02d4d85fd0e9f249e017000d4fc218a7ebb59c3abc0c2460a9c86c7add3240d126375d243496957ba8e8cc10669dfcc3f7684dc3f5dd1493364f7fe62e0dbf7d61211bdf0dee01dd8aa29d5d0b5ffde34227fbbe7532d70f931f7e19bf1f0d5633e57684b506bbd378acb963f9211f6115d8f82ce79a1573713c120936addeb79406a845c06613462fa2d1a698dd6a3792b8d6dd1f694cb02b21fe67487bae00977cc09ca5acf0863ecdbd7f645ea29047620906f80985ac4ba4e3881184051f3fa51e0747ea0522ff1abc3b80b611b20ba4bade9f5937e13f5550cc394e22a867e34864efdad4ff401cd0f7df933531ac24cb3ea6b1d5274effe09549d4470f1937722c0882d47f8dde55a136f86345842e0e4c8ba4849ce2cd65d84c243ec0c76ab43496005349726eff98662bb1239bcaebde0ffa4501bb17f5c838ad6d5f708774be";
	plainText = "ab1be0b2f9390ed739ee8158ac6ae2d37c8e10c832c16a79340ba63cec1fc532fe765bd5aa10fafdcc49a97e028692d4be1c50403f738db695e3d16c07b7f0ee93c57b0366b104ba7e74e5a57e8657dd9adf1ebe559c279c7f84c0f578c0d91247b546b1e334a26a167021ba94fc064039e4320f717bc202f495f5956d76edec2f1eebd54930b448e7c70772e63fa30d143d16eef475931c3ddc080c1e149fed9ab7e06147547c887bba863b38eb01613259b651f4c2c0e47bbd21fe32ba24e4cc5558656dd3144c439ef05699e539003790d6209efdae7b5bec61482feecfee601d1444d5fa0f4fd3401b1ef3215159c310b91ee9e2aa18cbc2eb36bb5cb4223f902524558e71a513c3e48fba2564a1efb0f7aa689fd8608e85c6eeba9ba3a9acaf280df7de1671aafa01fcb6d7534e517edf29d85bdd42b3e9f570d57f12585ac131ed05d85e9bc27fa7010a7e24a011c467462645eb48653379509c89da085d30599e98094a3c0192c66015eb9ca35740423713d40678f4d66d5a99496875429a0794f4370d98ecbc46a4bdc6c44529820aafdd12b528245d12b83d751e4f40942d78504c98acf3ba6f4aa6e9d6080545460658eee4870c3163033645d3dcaf8b70ee7f719aa8737028584476ad8283c1df30c27bf6ac4d1a1d8652588c41d7aea290df55c85bbdcdd47220c9a8954523007ffa0d69a55a7e2724263b3107";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 443;
	dataLen = 4096;
	combinedKey = "b721ea2f04b0bab738358240b55a5b8afb690f76a139bf64aa75536ec1d2fdb5";
	iv = "bf94a5b5bb7eaac46e4e8d9c3f133d39";
	cipherText = "c6c1235fd279fdaf368c055e575cee4300ae9ccd092f28355061a521422514fdc8ee7ae33f6e6b8c60463cf9f9ec129f57dc936d428bc68fe41b314c63b2b29fbdeefda999d6f36142fd9d64f246e02f128f82d41a9e915f679aa6c2963afe5cb4b03c71292b9ed74e4d989d90e150344fc6223e1584e62c62b1bcc3160bdace14ed3db5a1a78da2706f3545797989627d00c47b6f06d3a55baf970e9b579adaaa8018a52cedcf78c99fde97004a321b2445def9e2b9b12dcacd04ab3e0d89ad162cab14c3237e1fa56da052f92f5530d73110bde0a3913914946e22596b9662062364c680aec284bb337d0ac33383bfb5005ba065943382560d612d5281bdc4ae4586d63124b26bf910e58216ec27aff530ecd717588145fc875a755ef673def82e1309bbff82ed2f4fa0d4aec13a4b94bfcfe7f97816034a465c5a27652858a6cc7db98e880b4efde23845006bcb39ca73e94e400e4f95805854409c908bf734e7728abadb6797aee452f2aaaeb3aaed7ef0a9e83ce18cee1e659446ddd87369a3f48cbb7ba64f6fbb6354a5755178f0a67ff2d259cd4c98e97b8a5354b3c6574c93394f0ca312c7149da60842960561183f6928e7ff1af1931b5ae9287e86dd5b8d2ce839b6c315a478ca9dda9749189a3b689f7e7a7ba929cd1030f3168f01c6b649f32ec6e11fb6a71357f94acd91bd192bf30b42390fcab417e8a73539";
	plainText = "5d8899a12210655c04d954bb7fe5fe1815c583449ec6dd6b6385c1b3faa03c33c806dedc1c32ed5f37aa26408802a7120a92d487834ab548f3e85babf3bc00ec5e0f5ddac2d081d12914a933f95cf01e67075a1650522eed0ca32af662e41cc4e1e073287dbdd14da6b1826ff25c9c79884a9212841f7837746c6f5a258fcad35581432b93ab93868f814485dace1861f26e51b5a4103e4049c3744d4cca3a79176f76cdc06df80a7b4b09dd937e9f9a59faa59df3ab9ca24b678634cb3a3af97038522e4a51fecb90e9a1e5a9b14231d5fa3038f4924a80088ef50d7c01dab225454f12e89bed44b30616f195e3c91e6954ff38876cc411343f68b5877f845852e2585dce2b4ae5639ab9ab838c0983892d200ae10c3fe1f1910f3aca64588ad05c7cfe693eff276f72714a8b1855bf124fe1fc89404371cb0e28e3748a0217e503cfdd8bfad6f2d01117c2bfa974d48d9030df333b0a0356052a72b2fe089da5b75f6286a6c39073855838b732c26fbfab78d9492b76cf2c90c10c6d5f577dc23623e164eecce2cf3a35c995abf09e054deb24fcdbdea7cf99dc403cb479549e8dd560a995d8cb6d0a3f2e2c0763142c2a5bd5ecb439b832d99f416ca3a97745c284b27a41518efd9df6bf1f69ec4055748da9d8a7cb614df2004964faa004d3063c39fac9cd9ae616083426c270147d37f1eab720e798d8f30bd137864429";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 444;
	dataLen = 4096;
	combinedKey = "2c373ce4c66a87a5239aecd61bf10f01ec46ee05bee761b620804d30631e65f2";
	iv = "2a39aece2ea15c12f34d7ab89012f62f";
	cipherText = "709cacd7b100169df7e6b01662107a815d94763eb7c5c3b76a50f2ddada963c30a940b06cdf8f0c608a72081c75d7e7f23bd61e475c2dbbda195f20f0302b018b9464da0e2967b97fd80e2c0a1316407f5447ea3bbaad6c8197d55274b58a68d22ea4ef827119d0fb95d534096397acc1db57846621bae47aa26106fb8265d2dd9607ad4315a105415fb9379bcc7498f9a5cb81b1e3f00d696955f2208150bbec24fd253e94e10f3c053e21632824fc59cbe3aa8195f7505ac71658e7ab4f5033096e031e576bf1c535086684f5cfb1ce88338ee3c58063d4a10c3e42e5e7894e4325a14030c7ad2b396ed5e89cfb6eaf7d77097ac626585917df12de0406e3119bbbc2066b2df961731b4662954d9e41ca701c18b49fc2e2e5bb52ca2d0f5b5b179182c8dd7577dc59deb6ed918be452ec7b1801cb5bd8db70b179998ef53a3ae8aaa63c82f219b3814a131a14735244dc73bffd67974e97de92c52282eeb0e58e5ce452a055cad67f1985466edac23f62be4982d809e267cf0e069f1a893006668f64c9c31811c41d14473daa4e52abb36c48706290b5996dcf20b88a915f5718d830074bdb2e477c20c807d2f6eefa95c2d3b39d9796d070a703f9be61e065ea9150b717ffbbe6859e73ef1f5775de0d97f6c93482196ca4bdcd761b51ec0de603c9c609188c2107a15eccb6ece991d3ee1c24f0bc7f763e3c5c73384856c";
	plainText = "c32d097501fecb95d2e365d48e21fa68e004cf9754c64f85b318a9a2b337694ad5ac6c5a361273f11a51bf8c1a96d1860942f480080a26b773d9189438d1d50ab72bca16427c717d37487bebd5a069ba9da1c2a19496ca2e5dcf11276b2da40080f78a2da646b0cb658a2ce36771ab5821bb3fa11416a97e675080d8f3e9b9491a99cf45f9e77d7bb126e5aed5d27afaae303bc9784b2673eb23ca138aceb89009607d7a8f54c8baee283f18fc615f58f1ae6acc24925e97ff5f0a514efce3b0ee1c7099ae4e6d8da07939bb11436e99fe2b940efc6bb71275639fbdf178e517a0b4cc47e96d95dee1e3c42dfaea7965fc9881fd1dd2c140e87e6530adcc24c583f9e61edfad58fb951bf9060d56426adc78276c217a5659b321eca783309837dc96afb0372e820ea924e180b5e152ea779db300658bdbd714f71b1ac841516eb0b0ed10ce87694e41413a6c2621c64962e216a6c5e7c246b8a531ea1720616b9ffc4eed703cc0e00aa76a8f7d646952f0cc9987ab4da6e194feeaca2b2b15dd7d7897de2f5260a2fd98697086b4d0beb413571d0ef66985adc33c049b7c558dc018d5cab470b5af29034033c31aae9561a8bb317dbe64425f89c91c1be9c300ace7a5d611c4b1eef29daad8314375275f531f1509948009bba3a2b46da1eb09d7ff9424b480f40069078c6c4b6832764e32f2e603d8818d3bdb3fdcc41d4140";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 445;
	dataLen = 4096;
	combinedKey = "0d65367e1fbae0ae2ee910f36065a8233faeb8480813c88c69b2da0198b744fc";
	iv = "d00f578ee41269ffccb561b53f2b718e";
	cipherText = "434924d0a85ed8ca3c56311cc0d654d79e460c929006bf7a5e737e837194d04725dcb5671597f1d0d6e155667f47fcca8de0c53fd49ee606113a3ba9eb1caf35c9db3373f45d6b67c45d658fad68104dccd81b8ca69e1a055ff6b52b0a2cc50f955a6ca58d693316e3708b2f0088dfff3e81944c3d3b9da54c5ec4f73b26892eaec407955ad91e12952491439a227c8679fed08d2c0f31d74ff9f380e3bbcaef28474198b167afb8c6e9387f77e0c1b05a54ffbdd4ea2fd61c90ffe588b3056e99f407fc3821b54e090731655fe48e5cb21cbc545ca47b1670da248070cabdb57c3a6d4e234d88a9d3ad73c98a9dcd586651db478d2072365a339de8cb2a82b24753d21351cf1319c73abef34937de85b3c27328dc0aca2b1567a74f9a70a6ba3f7d8354a0734e2ba25966e19c1305272fe948ae03f96ee8368a142c727d1eec0464129d67b6bd46d2733eb40c96199b42fa810313ca3855d3ccae9266342c8177010a8ba2638a36d9535efaf8d6209baa6afbf9546ff3d4f08f422dec8cdb932ef9c1da0715d29c5ed7529014f15ccc0941b45f3bc7128b7f692fb19511783c3b38aa7b63ae5db92812b51a465b7ef3cfa0669b7dcc56e6d20de64d47e00b9240c7d9a37342e986de71c3ec41bdb658f4e3100d6a011de032f6d6f54bde46fa369591ef0001dfcf7982649acf3c16df73b8ff2bf84a23555a31e0608355bb70";
	plainText = "2c00216f2027b7f30d29cb6964fb25633d63b0e9222f6bb1c123f5db0bfa183d91c5d58b60ca0b4ef4639b4f87eb2ac58056dc8d4325b140c92bbfe75d144e33d0b51c91d5b0eca89fb4e06a7dd31a03b4c4bb16f1fb30b37b42e0f358aa8f5299bcfcf04df29fc7b42c54caeae0230e0b94ab80e1265782ed837aa41694e371487ca3dfc1f3df5481c7e1300fe5f450c63bbfa2019300182b2b855bbfe0f500c99975bef71056db0fcb3774d96eec423791e7ae19f1c7df6f07f27312af2f6b765e4451e2a909eee679f146a37f37eef171831d9ee3293a15b5d27cd7d29dd4dfe86f7a1760e611c3baeed2fa36701ff786fd192db74eb4a8bd88d3e837f33bac6a3d7cf2286a2a0c00054801dcce7fe3ea87665f4d8740687221d5158d95da2cbadf66d4a0d446ecf3e32771f5bf308b34baf668622365dad53e0d4d60afcfe218c9d508c34fa37ea45ece08eab23fb99dafc0cabda250c5b9d6000cb5028b3d06c94a8ff6317b47ec021112470e1c579260ca86cb90a1ba43cbf69c1ec76405bef414d5f540efcc37a4f0ae218215625f0b3f6fbb0d42a7b3f955d364536e02f1ad1b6ad0f3117fcd578a52a2dca9ce89c03fface5d8abbebff80b7053976e0b27cbd0e3d80cd6b287a2caaaadf5e31f24dcfaf6338d430d11819e8c403167222b62c7c95c2d795837c660da2c0b78a21991917b3deb2a17930201e0250ce";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 446;
	dataLen = 4096;
	combinedKey = "57efac7885a19c2d837d67f5dbd468ee04e6ec9aa86818088d28149259bbe957";
	iv = "16aec32c9703f4e08ce017f65ba8222e";
	cipherText = "24a3b68af81d2b93400d33598a1be352c359adf06612752878d6118b5fe56bef46babc511c4d408894178fd8af945e40898270e0ed77f500d0cb584c3b5f87f3cd8d83462a11bc230ef2d54106af6eefacb7e4532f176320372440f97d7e25259db15f9b078a7e14e99f5374020ae35e0705d80276bb656bb7ed48226a73066546bc10cef97fbdb783e2e1b648fefc1a58c95a010630ea7c86d638d056ab5ff3b0a31eebf5221a808fc5738e52bf85adf3f50e9be62c38700f72f637a30a7dd816040e75398a81152274efc4c670bdbf8a52c35a8eb37dc3cc486385048144474524f0fe7088839227e1e45e3a44545564eee043772925aa5398db27b628be899267155442dbc71c441ce2980d0c67c794070c256d2ea257a11120f281af23517d7138df3a4ae9765bbe83c3965fd8244989a8019b532bb7bd3e71541b1010e5c9511d886ab0c803d1e5c690a1059d3435ce9451e075ff6c3328fb8ff7a16bc3b2a70afb484737e46f185c62871e3ec6c0682be08cfe31bf62cc341b72debf62be146adccd5acfbe770495b77002bb849b84de0287bf2aeb3e70738eb078e49db02f03364c9011053e8a2171a420f9c2e3438eb2f4c799aa5524391f7315f38c2caee1d9aa5ed68ab8be263493eb54135ba20f9f25d90fc90441018306d4299af503d222c99e1ad7297c4d355ba2130dc0b77f0d3843ecde20905aeaa6d728d3";
	plainText = "a605339d3e17c3ee1e26fb56d280ab01bb1e7b8eb422843ffa765b80412b410ebffed21561b5838e7b4cce54ec5d8571b0c15341de966fd5085d392119212d966250a243a1c965388435da5f93921792aa7b1f3bfc3695c8c36b0a150bcd95a11678ebeaa33bf802de6ba698b1d7c019a59d5d0ebbd9ea0e0c997eaa067f61870802da5bd3fa4df5dc1258f3441c742e1e05e1c76f573e21219791a802e36c914ee73996078afc1bc32618431e28479557435b5ca971a407caefbccbfbe7eaf3d4344f3b94585dfba5222727f9993391aec0429861b0a947aa8e989ff81dbcefdf3e720c2c0375ee9c8b3eaeebe5cb166865f46bf4fe89a8be0068d6eb3b011493de03cf8fd2feb4b4dd8ca8262e86adcb86021230d929be40932c954d8f6f1090ae377adebec9b5fc2814d402e42ec0b3059b18ddcda24ea5ea22de898474d24ba372a5bbb9abd4d739db330c68f39936c8589610991dfecf3a58b309ff1617102d7c3cadb88ed97f845ef8731eb0e7cb976629dfe208d7f32826e90aefa2746d04b504515c38448c81b034218746fadcb024447f4f78d9670163e6ec98ced7800e8ea6e1db132f983c49d98768dd57cc1161c0c2a181380a407e4a705e846e790a1579f2ed6f02c21d6ccd1c1e0f9191857ec9889f1129df86739c03d17183f8f42a1debcc32b1e02f7da322e2747b7a8363d3589c9e6a293f94544c7d9f1d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 447;
	dataLen = 4096;
	combinedKey = "583da0aa0c6f4c6ee249a383dd738a6fac0a510374a5d152f5a1ab5c0dafc34e";
	iv = "f30fec13deade0413306a1a01d39535f";
	cipherText = "818d4aa39804fbc9550f18e3f2e77021cd1a866e49d0651823bf92432da45546be9b0575b72d8fc56c53ee6ae41f16cebc0d0f5d16d3c62158987c7d0ab28bc77d24966ac2f0e830d1adfacd6c9d3c0812d6078a735cf8e132d62d68553e12f9ef2d627a99bd87a0df3488493bc062daad0ca574fe00557c6c9812e02cef2ce43fdbca7d681f6777140e07eb5c9a729056550b0973470b27f7edadb2d05bd537f54186070ec1a94121e4db904bd258c6e967b75d266347cf7571aead0e2a0019ba5da8c116e7891e814ac7d247278dc9f975fbc36c21b10752e5828ee8164e06271d10bbe8e3349fbbc2a11caaf0526a0eae047cc6d62f6903db6182c0455aaa7f9e02c3933ddef708345d40d93637c816811442d2ab02a1fe01e417674d05826d6d10dc017fb58791f7b09f1fd94e046f3077b08abb6bb8e33978e962a84ed1ec1c850ce3e1f04c40ca589b9fc463e90209d302168be096bb7ba0c7723312f2db8051e9d5605c2eb8c2232b521e1aaa3a8a160240858420afe6638139dad78226df140f77f676085dfde05d07846d83b19cf575832a8cb4821fa34aed5ebd7c46c0df7ff49ae8de4edd7bfd304874609c2d3ac09cc68486ad5751fffc2433becf9f71134e041c56058fd35972169612cb2d6ab1bc4654e31fcc4f84ab0c7f647e9d0bfc03758c4156be4bcabe751b9ab982470fd00ae11867305bfb5827cde4";
	plainText = "97c0ed020f9c2480974c018c871b9078a3db0f2e90089e9b5eda3a55937938d25a20ede70bf18825091de0b4bc5d6033f346bf57e40929d46cc045457914ca74cb2d932954d4b1eeee72166dd80cee0c5349dc283cfeb6c39c1ac6ba773db214fe53e5769748e90aad163d8f68cbb0eed750db88c5ef1f806629321f8316fdc4a51a354ad4f10c4dbae1bd6084c3fa1b3702a7fac681565d835383c5a7341d90a609deceb1c38a1ede8b9b2bd5d3a18762423197a5eb5e77584244a1019698a46aa4c511c23036cbe5933fb485c1fde9aa8f9ec35f6dc87ff45e9d5ae9ce98fc09874b05ec2ec36a19e469fe9b6dd3618c939f3d4a62ae475d451a1ed57a427c7806307361c1d6e1eab43fb24306e2a4ff838409ba8c776907eb741d9d240dd7aa67d8670f568c44adf885b6d47d2ec7638440187c1e5a1ccfc8df377c6c86d33ee9fbb68edef6979b1530ae08e32b9de1eb8a659e001227c4ddaa294fb6c980430a7e14dd357a40fa2cd269f20c1855f7b57034b9f161d5bc386f8f98c0287d276365fac74f3697cda30db6bb519491b4d810a2edca1ed0959fc449f7d9c8d5c64090c5831ff665a2a778dd8bf07ace7d38b68699d70217e813b589c5db4f877f6dc7c4ad74aeeb8a57529a060a641b4f996fd6e2fe07beaac88f98df576709842f021ce50cae0ae08bbb9c01955b945437bb033b861f4ccd02edf5926d787b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 448;
	dataLen = 4096;
	combinedKey = "ee1f9fa8631b0c57098ffba378739cede014dd8cee9ba782c9a9cda96503e108";
	iv = "c80fa14994e93a0e5e10132c35c5dad1";
	cipherText = "1c9a70ff3e22b811b767962478f00e60b25d69e97eda443185d05b213c6283768666ef7255322c025fcf0fff3f4eff8c6efa29e6b7e7ddd15f77fe7bad0c8018e3416bb970f8fbfc8d71657d5ce372727f4795d7b88a734b44fbef15866f43c3f0635749e618ee37f9dc9438f6e4a3961c7bb5d9a54458e450ed1067e4a3e5fa0d24b7c2939acdc5db85b3c68b73157c859afd9be4aafc3d575243b4ca2b1bbef3b82dc3481f4f87de7fe1a1be279500b41d7791352bc3296053b3c5a9343eb960689424682236610632b5bedfd50a62c10814ad3e2adbc702d2d63e2fe9db8ab440f6db73a25837a72c03760fc2a4d7181655730a75bf9d26562338f1595bb4a7f6de4dd67e4e279cf300bb8ffe5379ac1b53fc412803c1cae443429cb00ad5c4bbdf4012f02a264da51f58b20bcc54230e5c7754871c061eb24ee3faa3041a3bf3d7fc0464fa2ef920def016274eb6db333a8165f9114d21fa0caf6e79e037ead777e978b45daaaaa4fec06c325e6aae82e8c4df231b5c056b2c114a7db1b690b6da7887d1a5a05a17d5a35ff95926ab1f1813aa6eae8ee39ff9b92a77dc886d4d7f282d17a70cde310651e02a3ab29503d459724cc9a92af19aef5853aa8dac94d928c76b4a488c7eb46a51897493235d9c705f33b91492f0d98584e3fb3fc74884d94df601e34e1ee3b255d8112cad1d0a04a1366eacded24a3677f68b18";
	plainText = "ab1189031f41bc7f8fe8a5907f5d74f32ea68890323387920bee3403abddf5c92be6612855a94dbedfa84665dedbd1e4d87904a3441a722b09e73b74b1b14d6ec6a9ad2b082899f3fd438e9c3e62e612d8f12bfd64d2fda4d6a3e0bf1eaaa65658b083bb4dff04c51af978b95f877502e1158e9084bce59a875812cf104d9d9be383bbe5613e9bacf7049d31ed9df0e18d9d4aa3cfabc5f2e27d06d9968ce619c785bcd703363fe59e3f1c8ca291bce9439f23f05baeb1751b8e5c2cec26d21caf10579676b90c8f6a8e73d334cd945c2a0d81542bfef44dd22e4d94e0a3ad1d2c7ae94913030aa5de5fb5cb71862190a96c3e83e421b27a0e650500c548c5d48c3ebccee4f75270d24517ef0d2f41f8ee694fbd8bc9754cb236d46a867da4ca06967d915bda898905f7db9ba514e64e66fd31fe8b7da324055485d110209fa1770b692f8c5dceb531037434b293b64359082dcd316b6fd3a6c75b6b26205aa3dc355f0dea916fc485724f9fff90ba70d3767c21d0fdf63f705082f5fbdefe616d20f20061d546fe2f6577ada9264c5842ac16a6a609c3d197da90e6e779e93b0c717395d7c4e6029c4679411ea639aae749dbd97cfbace4333b39419f3f417b63343f247784e0387fcc62f27f79220e73c8bfd8d042ecfdb2e848f3b88082cd776eb8a17a653f967effb16b7af261742a03bc8a765a2f6e5e66ec53f8d5fbb6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 449;
	dataLen = 4096;
	combinedKey = "17d63e24d12b87bfde473d05ece0d94e702b758fd6c40acd1a7b106038c3aa0b";
	iv = "b61d43f205b4ac9897ebe3b59e3b7c61";
	cipherText = "c51b3db014a74662b0a8b48e15d99f541a9e876c1f84c333d2fa37e4988c66098752e9304c2c26a8b48e2cf7c16f89b3ba9eaf4017cb0298f548655a595bd6c37dc49dba236d2d4633402061e56d468895672d9799a53cd43bbdaae583f9d13d9ce7d7a7e4318c76b990c9e3c946ae4888bbe1cd833337976047163657a3e56bccaeaa0ed8613b9a9f962e73a01813c1b9982592ea3de9288bd3499739ba0b39da9e49f9df30fee9b039d14466a3cd8e015b2465894808ddd5782fd7311bcf8d2b32d782d329f71ea06bc73b944b372a1f60e1774dcd022db860ce86a5ba5fa5b0bfabf019c5b47428e8661ce8d0de1a14d55bd254cc320a6189a712c68b0a1042666a520d8690e5191500ca9ee915d47dfcc19d7bd03be549cf52120051220e567ae78ac7a47c5b170590bf1255f8f7f34e42597e19e7012a28778b1c328f0df21855fb9be71d4a3457864fbd344a5c7365e01aafd5277cd9e79b79967bb486250eebd2bcd4e2f01667b45fa01bd76b679f40f003dfbe4ccd9fae47e601671f71830a91a6e4eac996d6c31db2488064c8d6d56f7ee0fcb16cf541552e6b17c2760147c5713beabd9e832c7dc8bf4df0669950c0e2d987bbecc03592c76333debee6d96057898140e448528ade1945c59479bbe72bf00b41e702661c0f3a1f1fbb1c9b7d9b0529801d55e22b2ab28d84d4441c193989d878b063d811b60d808f";
	plainText = "04d1ef28d37a544a37e4588afbb9697a12474cefc17b4ca37981e7cf1411b6975184f4657adc4ed7c6178526ddfbc793240a67149093ed2604dc2cfeec64c155fd2f1da57f2ad85ff134711028abe42c6c9461880f5817eb8f58ffe304c9433e9041a4dfa6e06e9737c64a03ee88b6fe6175ea2f2af6ac0f06d735df503d3e600eaffff35236834e0913d68a8e12136796204dee12fd25fd85a276399ac5a3bec607d8a46d67e48fa84517cdfda5b35abe4c21f66f48f23c5f03f6a4e90cf799f5254a67bf49ad359023dd924b2efa75b502adcc7270916df1a66aaa14eec8d5b4f472d7b9f0ddcdbf8b266eb707603249d0637f7acdba65801e3e27def2686dd6d5825cb95f273571d3b8ebfa086d79a7a24d1189fb6b0e95a57367f32547b0642904bdd92eba0d719ffc7b72cb8150108b150685e96a5120245e054bc4918e7cdbd6f4964bd3c51214df4e6885d32c8fb15d85a48bf7c3567f2ee3c09bf2172a1db3b2acdc59db984512eae261de996a6893bc06a02e64682a06da182aee4af1e96fb515cfa3319fd0f3d0f18e0fc6a8525a921bfdcced2271a87b458d10c5d34c379ae8d5a39966387eec77fd6f9b72326f98e134d73788bd2f536dd65f486cd0bd3cf83cc21a249324ef858f4067d2940f5ae465a80fd82142635def7628fa8faa4b6f5cd78635b14cc494aa6cb351766e33b405117ae6e71ade70ebc4b5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 450;
	dataLen = 4096;
	combinedKey = "9d121a0540a7be3122340afcc86d849a6cabcafeec49b788a06eff15ffe39cab";
	iv = "6e0f1ff2e9d8e157b95c1e3e41409528";
	cipherText = "af2c8b1fc641e61374637cc19f3221d36b54c3145fcb4024c3c3b301b366603bbc19ad60d66769d02b55548eab194812082f864f8b318ade17dd9c9ee1c56c8fd8d05f7e8a5d77a445bcec94bd4ff3d0bc0fddae2847cd4c83d4424ff6e7f7a36420453a58535a0408039fffcb25d5487392a537d667881335ea309e17d674ca59d0d35be5def49a7456bbd90fee7a480570437fba6e7fd1c074a14e3cfeb8f47717f7800a6063586b94d0efe5d1e07d3799641523fbfd2a070e8b3083dd5aab6ed956b3b424afc2397335ddb03aebd8aa334969fa181f66bfb7a847372cbf15c7ccb24537ea94bde56dcfa3b85043a05268187be1657d44ac1fe01f7c61cf5882849cdb56a2f034a37fb4651ff25b963f5dab62f8fb4fef3afa2e8c058d3cd50fcd700691160b3c21e1601a44cc082266053a7961cbece06fb7f5ddaff012d69495f4fa59eca30ffe411d38f66908aa3ce005186fa2ee6e80e79598b60a6510932cd3afa6b9407444a638753ce09c65939c49121b500521fc577f9b83a47215afc33de72d6e62885085f57fed069ca20202ddbfc4c8f537d06707a7e0204eba4fa4fb0b3bf84243fd56eabcdfa37e0b5423e39a27c0c0da3002e46fd9b841ea58b892e2d627349fe9e8486e58c814b96f9ce27fa4d7830ed0220d0db5bb5e6071be56559f93db43892dd43532c4e5765d5cc23ce7ca624edb8fca0e0c458f30";
	plainText = "05f2a4085e5a2e7c88e104ecf1772d18d73566f68456e9a0c1c46a301928a1c98b1777ae9910fac1907ac59e341d94e2349601c2994b524e500390bba1ef77c5abd42f1ddb17357e54aa662001f81cafe4e27370a6e7d6f78154196a18623803fe843a39d8731d129310d799e6ec9070e0fd020975f50926f89ea0362a7eba6cdb6e6dd68888dbb36261f85b5d45467583c3bfcce502f50476a84be35b8390ef07fbc842074897564d1f97321aa57b4fdde79f008ac11661ee78424b0f630b05a2e97e45412118cd6b34da446eece3447ce29397dc3b1f384cf20e6859540252227c25f15dd6270d804aaf295874edc156e144af335667f7460124afffb9104e0600a4c4b51d10f0941d7e54954d5ae9698176523a8b48dca3e23d1ebc33c974d29c60550bcc945e03075827fece647391e4cd765998aa40fdfbe0b0629db245032ee6daf82ab239faaf96cc90d6905a74369a86b0a9cd38edc1b0cad7800db01ed75188356836f24f0cfbbefd6775f8eeb8f426019b72793396b112bd87f3a9ff41af7c2c5ce1daa6e0b59d2a77721e551b4ebccb76dc91ce3a47aa59065566275a02f330a68ef77d19ea0da08e33c9a45e9b8f39e5de07c97d70daffc9af98fd96d28e5b72d05fa51c26ba87cee9be197ed604a23591ac7bfec987d2df7a4180550b39678d1005a5756fcd7f691dae0dde9b0cd3ef40048d369540d5e8fda3";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 451;
	dataLen = 4096;
	combinedKey = "2bb19eff0c3906d1b0f00bcfab74fb650debbf86ce4a27ccb3d1c8daa6b77b27";
	iv = "4c499703961b1df64b7488a1d6937d9f";
	cipherText = "5a11907b989c774927b962f5a206f25f2f7dd0a26d3bfaabc967c6433fa9f3363acbfb4e4e353a2fa2ad880fa7b83e943856e6347c4ddbcd37341b7503f26b031677321d7a87f88f737d6769050938a1e407805dc28c449219408b0342232d01df77915e4d0d72c806d70375eec2a7cabfe1ab0821ebe099f2d7b589cae910718cbc90f877b4f1b624462ba2ff1f47b4fecb01fa2c87d5ad461e697b5b224f530a7489b3c9e6d7459c0068cbf741cf7d3c2ca0430dc59a0353857a4925285f29f035c46878fdfa1894ed470e9c51fb457313d2394830f084e1e694656a382e9119970f8f9fb3fc14b1b8174d761427b3e03527d541e1416747c39503468a1fa62b755f796634237a8a6d425f186f0513f5d72c22f53edd9c16009cae5c1cd853b68c4b155f410db703b212be1ea6956ccf944e9971c0080462763aae13c151addaf440193e81ddc3ad0747a261df8fc754d185801a0094303c1d473d0be607eee513f2675fa1356936bdfec1862df74c43368b8d6dc73875b3a2d244ca9e45d500e0b894a66ff005f2c998cf3ab2aa278dfd81bea1139073f5bc0ab1f77bcb9836cdb5264021f0b3b28db0d77cfa127e9b671de51731a23e8cf9c1c7a47f9f33bc2e7f1f5486a201a27fe94f31e4d501f4d0778556ef7c1e7bc8c5142911f1f4a0d96d7f2e911bef27cb3aefa24cf6e516a7f277238e1afd1adff9c731c20364";
	plainText = "0a961b3b5ef16b22e7a6f4d4c593bc5c5d2c6057f2afd03360d7fdc47a915545ecf6aa5a75c83151bbc98bb39c0a03b700d02f7bd8be9c79eda24d9550af02c4c66395fad80d120e50d00c68d91b0dfda8ab2cdc91785e67da5f49d22212d59c30ef369f6389912e36b93010285f413aad80b988f99c82ace9459c886db1cdf46dcf5be86fe35046c897654e0db980a67a6726341a16612956bb1c9959769e99256d502975770c7bc2c558f398b4bfe229dbc9d15dbbc2afbdd878e77783eadd934d0e9da710130c243eb9e78dddce5f543b25ba70178fba43c1697375c2b94753e0eee4aa4403217ffa31c733a8c4699b06723c04963be8390320254097953f2796446dd04b658bfe7427a3378812d286a6a69a2f1c958d4470fbd87bd74fb1823af0caca5ab7f81a0aa7e4cad2b6b76caeb8fc911e366aa68f128d613cd3ea2e937a6ea28e0e1e0fe47e1d37c6f0bd36014a5dcff61b4264dc9ae9a8ff8bb45cea57463bf2389dc4cb15dc889ed61bd3220356a4350ccfb2003cd41aaedd50ad9ea9dc17bbd855b46611b29583969f850c2f36266a3ab32b2b4878049dfd436fa6dd783e5a5987e9eafc2e3f916ecf35e76d5f8db5d697dce26a17f343e5b4b50426812e147c4b0d9c412ab60f68ed089fb1b0a0b0f2f336ed27ff226ae032265d99429dff7be894c78e1d9cfc44a2e219b821c14f0730b4c1028822753d9b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 452;
	dataLen = 4096;
	combinedKey = "925ae4794fc1d6f52fe02712e42d5417ca2767377455b017d58e697778cad6a3";
	iv = "0eb7393422a4eaea20403cb7ccbb9b01";
	cipherText = "125c05b7dcfa3cd531b9ba12328133a6c0be54a2cff18a78e19f9efce24cb2eaa13b3ee7cdbfcaccbd76635a05035eb39a8bbd20715916e076e642095d0426cc146ef1fa8dd98ce3a82bf9a6f4c982a8f861c6c06633f29faca17610c82de6ccde89e409c576fd0e2b7302d6a2f9c92405817bb7600b14d39d5bd1e86798dc1aae864c6faf19d8c791d617b77d337d118402ff454e5738e15ca12ebe4e29507dfaea54239e561c29ab3e089e08dccdb9e8ae42ba98333cff34e323fa33ff1c5574616f12803b86940be8db257e821e8682a6e6e1d0a51e47f0433e400e126e75ffbf319404ccf8b4b9516fdf854ec2113acfc5dce6f02b2c3c1268374762949f7f88ff93304429c6796117855e5bb49151cad9b682d99f5cb44a04ac1fefd689e1d65f69f14ae0426a3015550718b6574f4478305d4606a7a77ee76d340a879e31b2ab6d5dad1f40a0d0edd950cb866e56f4e44a223ed566a8f3dbb71ea07485e29dc73ad04eb09de07d234f9c8db00b44b44b33d05dfa79482aa4c7ef406129b6979f4373e193b2facaf5931761dd956f5d200c88784151c5e1d6b23aab06f58c2f93cd4defc533c031939a6f389983c78abfc83baf7f5fe5d4405a0d33f69ecc6e4a12b2d13058eadf7abc93afe4f9e0027dec528780d4537b3cc38bf3d390fee18f7ad96755b0c73373ddd07d718eda05d104fb499822f12d25e9c8c43265";
	plainText = "00426f585405082d9dab223693908ab049d26d552cbf8c400c85cc0fd1b3059a84fc1e983b2f9ce2b83d67f5708f7ece117bdf39c715fc8a507e355746eb6f772ab461b4202ae325d7f4fe3600c14f044222582fa224e3d443fb5b45132ce14d2ec997feab68035b55ad40204e1ba83202df3510283f3aad73a0e7640bcae6490d9eb6260fa8f933d3847c84301a9f828b50370089f7461c7d3b7a73e99b55765a4c5294a74e1e5b363f87c46bee32f6ca8566ecdc7d036af11470344543c5ef3508502a9df054efdb63f983b39a4659a20413fb22f21e91ceb854d53f0d0a8a02bb1c8e8bad1c82a04a0c451cb7fbbd0db53822e80f78e262eef48794a6aee8273d91e81784e84166d2aa814f8a39e9e03efa11177f3ffcdb24f64fb8f48c73734a990574377de9680833411a7e26a655e79a10b3ca35cb52c6c09eb238f14e35aac1c8c1d9eb1f8003e209190dc622f1f833f21c81e82f32c401f4fccaa57a132279b521a7ade240ff69fb6f208836d875d10a091d67cf84cd2f32c6fddf05ab9e46111ec8798eb8448f2f0cdece07e2650278ddb6f28991eff503c16bb2351eee1270f3e60b3be8af15eebe1e9dfded7bac27c6d9fc216e371d031edcc37cabdb328879c0502e3b50480e24061bcbaa0c9f6d7a70f26d1b98cfb0eb5239cdfb6f83b10969b3186d8be1580ccea02653b8062c055d07bf6b86fb012bbe4bfe";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 453;
	dataLen = 4096;
	combinedKey = "20ce63b73b552a4b8b9cddffb1ea564deda7dbc775607386e9e01c6408051cb5";
	iv = "6430893dddc6be9f3d7ed2ced0cf69fd";
	cipherText = "20c6cc17359695f1a256f980bcd36d96f9850801d74bd6d4c57158796f1b5527cf9483f089c91407b0745db3e1a327c4617494db59dc1c851ba738e02e62d683af108c0b5c911ec7cc1280ea7e5aa86dd5952420f4703e82f58b883391a022af284327de495205ee7e59f1d383a3a230cbb069c225f4be3af4befbfefd8c7c4a7d65326ae65990c5df3cfd5c8986b02fd11b4cb1fc2be780a1c51a6189f66f90033c3c96e6e60a3947d41a5a86f6acb00994715feec07dd96d8d19194e12cc9dc306956f7e693960f89f6286a30db4bc0292b4bafffc98fbfd8078bccf2c164eeed2849751233658da5a6a238460e526dab54dc56e885e3bc8dc212b97d359e92ce48fd89f9b8d998eb571a63c4040906fb23a254f8250ab43be260199e4d3cacd61043e0ab06e1931bf16b0c8e6a32a522c8d85c24a68d6158432e41f9afb4188c502d07ea32d1e32e3c7b2abf944848b2649071bc8079f3a3362a30c33c3cb286c99c98ad99af296f8c262f32ea03aa49cd227f4cf1f06d5ba28e4d9e878263b93703a1561729d6657bcba799546fb68acd7cb0475f6b03e0302b758cda9c2a78b4558b86f19556ee0b86c134a4594268be28a735f2380d61fa9844ba61503acb2a10a8877d1cb98cc72b918cd8111c88b6cfd4d88bba5cc241b9620eb263bafe0ecb1a53f8a5b4d82fce151b0ff01e791e608ec81142c3adeb5140af406f7";
	plainText = "67a2be4dd962817cca7b2a6159aa6f3dabdf64bfdd6fe881fc2a2c3576271aa401474e810609fc6f2e6295eede52cc7bdbf024128a3e9a75dc1f18c6750b1c2b04706386f835ea537d55672e4149a38902e09c9a952d4e1070f9161a7fef5e2c45ad3cbf89ff7dd99bd1684cd9a7dba2df686c5f2728fe38b32f1fbbd7ea008de255d21516422d446cf62ce5bb54663be16eccc4b142ea18435279e44064839cb33bb9f9cfdb8c127c03ff12724fa5d25a4fb9e08762db1dcaa7bc0b91aedfcdccce017637b8be718b0d0303b04b87241cbea9ba2dac0680213dbfd05f11c2f3b9a5fd27b02bc5cefcaa635bc4608d8a8210cdbcfd92b325409c6a35091b7de308633beff1e3108713876066763c1e44e6ac0ed6a85f7ca2c60227a4c35fef960803851a76dceab21d7e2080a39491440ea3e844cff19d7476a13349378d342e9b4efefdc32bbdcc9bfaf8c8b372fd9002d9c05990cda8c661d113ab2d8d4af1e40545f3dcb649d30294cb25181978e36fc0aaefa7ea6f273763748ae7c2c56884070b03c41e166c5321d645a14d1f981dde2cecf48774d647562527ba679ed8d1bf1757a35a2578f0f21c01b7a14020218d8f3c01278cf25777a08f01d0dec95c5416017887c6e70aa51ce0414f5f09875be58c9c591d7614d118bdca1b9b94c3f26da1ed0dff3189f08048287de4ea82b77e9514cbce2544b721e57c1d5fb8";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 454;
	dataLen = 4096;
	combinedKey = "be9a144bf21056c143fe4b7944dce24ea338290c58c28d702c55555e6e768ff5";
	iv = "b6c68ea7fe34a2a6cdd54d5bc3473560";
	cipherText = "66d692046334887ba81b43acd1ce387a83a35126542b77914ebe72d00fb21b479f650ba2d84198f81051b882d18a9f130f352cb8fd0c4578392c22aa048bd72f3bfec03e63a105fd44b35e9b43c731f4bfa066e6761bc2e4b1c30e46ea9373b6e4386b7d4c0a2b8d4e82c30349c04caa909e1380533875cf7f11989f1b8106b42d4fb869f846ad672255ed929677b6e6021943c060e4e2800a701f96928bcb91ee49d4f676b7e19d75f1948456ec76b7708b5384709d1a866816e862fd5dee3ea0d9960957172d4f8d5f751bcf240a8649671dc102ab386c2c1e843e565bdf611b2fc3cdf16f5039276e292a170554550f787aea12e9240872240f4465d6fcff4c2a0efb82d09d3c603a4dfd05c5a9de46937330d930722dc41dcf4710a1cb39417686bca11d9352d20dcdfd499f30c6e4f01c0d7c4d6b54121f9e9abeb8c9921d447609686290ad1669112a54e245eae344b7528909f96206d37b89c036faab3e2a5d8b5047a7f259f0c96bf2867c4b825060b7a0c3dc74de7be9442ebc0d98876c2d4600fe02d4064635c132464ac1b795eb7f5e5fd4faccc6e5c175c319fb65673c18a060505b2fc087318685621b36e2973ec94e712f5f2a8c6d8f2c8137caa1ed9b14adf5ca898ac617fc0d49a0faf5339ba57394f1fe3e968e4f7d68c4311a9451c4a9a38db0c2b66109d4e8ebb56f9a6e13e7a30a69479b104bf48d50";
	plainText = "cd6c4df8f39ace16db661e6ee65d5b47b477e2173f422e28be0bc1d63b792dd678277b665982e4901e5fe104b6d2193872df38999cb668c82d0342ef22bfdc242976fcf98090b596e9d79af4173f714ff5974e9afdc80aa30c4151f4bc9e30f73e35ac3e5c49f8fb07b6ec7ab637e7c0092050785644b13bdf418ed7d33cb4eca6fbbd9d561adfa40027bee1667694f153df1735932883967a70d2a713dfa482f781b0fb8f3bb9a09b6eb89f3a2ed9ae4a7285bb3cc842dcb976b576c4c2da69361124c56473591872cfead44bbf6e86c3455dcfd7cfe308fdf218e8911ca2c6d7ce0f19aa4f15f6851cee839630aedf66936ae0ed3b0a9f3bc280b131d031f1a2ad19c4f7d87d6d212d04b80625448722a9a06aefb40db910cdd046b0fbad1dae99a936ca62df2d33364e76ab5c29d26aaab4ba054d9e724c7ee2acf6226132b28bfb79109262b2858ee4bbc7da8cfae9bda4d36bd14b3fef9bb8f95dccbfdba342712709c0669f5fb2b43665fe60f2a1fce6fdc612751675ef8c23a3db195ee9f6882c2ae2f62c63a32d5b34cb73eec36bacc502a3e43493c12daf76c7037d08ad8b52a423687b0417be0cc1f07c6f1ba6390c52afcbd324acc48a33b4ae855a69be1aa76ef573acd8440f8d10b5450ba8f22765b122650bddedb02fea7f2ddfb497e99f2e6de5bc5d1a7c8a95ce807d321d41544fb2e6842322e98eb976e1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 455;
	dataLen = 4096;
	combinedKey = "395d2678044c419c9971536cd905433ce43e5bee07dd31b56a2af9e769eda5cc";
	iv = "deba734ec4d3a55af04fef6cd3bb70cb";
	cipherText = "0788791221379f24847679ac8d215ffaf1210947b68ee6864babdf70bfb89dd6c1995badd806584c60348e333c32ac981410ded7415fe305e0f6ae4a3021014dbd74073a6fb66d376cde8c254b1a11a3d6a3fd74e7ddd376e8f8b38d28120429cb4c9967f19700884e224aae7d46ce7ddbfedaac9391542d278e650cbc8688c17ffb2df17948c5b75cdde7cf68c47ec92d41108f8c0448ac75b622436f8d0bb06421e8c2cddf42d92fdd7a790989b950e54ae0967005a959a6bed9f0cce267a11ec2cfd305f4033c8cc39e4139bacb49eb241972f934b2228e745ace6661cf236d2770979296b59d830f4de9bd4f679084e2f5d6f1ebb683249aa9357e59e88ca517f500bd721e36ec57b0be3af130b7e5fef7fe0ed0446f84b07f86f570eb3c4bda6fee2eb233ea530c9aa540c2528664e6b592cf617fed854e412eb3b4204949f7e50c9643ef2920966f386462f52c79cb43ad3738849e2c87267ab1d980232c22d61ba6297e3e3deeb8b33e4997dd4f34909aa675e05e8e985f21da990683b0092feadb0956aa5b0e7bd7abbcb9e489248778b9b58dbd212ade44aef99b9d1f68adc8a3dd6bec2002bcf9dd697d0c6a17b2bda11788842c4b2f73297dd41bfc1b871cfc89f8d6fb87655962aa7c5532472ee51d27737b81ca55f9a52508930b99507d9ec6a000f5b812b6d68b1232f01dfe0c20341ac45b250b6b2dc6daad";
	plainText = "23ed5cb5eae86ccee8c3cb3fe8e997125abaa3f58bfe4f7172117425eb9c409842d139f002f5632e63ec91c746d7a2fefde9cb09ab98c53d280cb7cb7a8830326c50f32ea7d05a84cdccf16b2cce149fdb54adf4bca45a72c4df6df11aca96026708671a6eb8c3b89daadd659d71fb529da5a521e5c94484902040a4c9004249d89bbef7d5df0cb85884d2d418dae6d9daab738005ebac9888338c4938dd3a296f908cbb67207e75bd54b839037dd685c599ce2919e4c5d3c10c1e225df9ddbfbc9ab11f5bb866f5525c9c1ada3588f9ea595e66b14c86a98d31ca3a5a13d27244d3a741677da87f04edaa5ef2ef02e8244bcda673eaaba72d449f3b036767bdbaa328209f458561a7cc2dad1b1ac110bd9562016169c4bf1b07f0f9b8da297a9d03475e88c537401d891852aa4fc2b11489e9e0d5865224737697aa32bce2fd0666c4049a31c981d6067633f9dd943d3804eba83fc0ae21b242662eae5f744c0e4c93285107d0a25e11274c41991029c41740fc64e657bf2164c5d9caf8c7989eb748b19151bc044ee5fb310de650fbaa407f976a6400b752c9c1d685d9f3c964cfe141b4fb1b78f03c582e18f6000e373e7bdd9bc71434877686adf5ac6dbf4ec0e05476457276cb3bbc3c91e3ac8364c5baf47e01d78c1c09b26e02b20699b2b999a887cadf82f78d58c5d9a44b2b5863430915a81e21bea40fe51ba609c0";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 456;
	dataLen = 4096;
	combinedKey = "85d618079c5a0554766b59e47070fb449fa1967303101d705da875efd1af9f87";
	iv = "88402d1cd26e254bcf58331ddb96f56c";
	cipherText = "81f23a71e129d0098159289b514d37d409d9e5949a6afd6ebca48c66604e19242302f79eb8c5cb8f8cd9156fec04adb44baefff066e3b728532a27939db50aa4e08c418b92d21491303da33359deacbb8d06cb2ca8f3e4fa65d5cf3f350f9467b92be9e242bbe3a7c968a8e6ed1937909dd06eeeec71e0b5acdf3181a48fae254684c377915c46c5ff793e90c9d832b13641555d77c19b757a7e3cbeb3a81fe75ce83d4cde567dc0847b7554786b2c9e413cf3e8b143b7246bed44d764c755248c1a684eea85bc4c7c231d4b17f2594b2a83b117dfec0fa3254615ea8087745bafd3f69d731932fa2cd3fe9e2021d8aa7ccc3c530d226dda90084e24aabd7d37d35dca0a4907a37809f34226658dc511c2109c115a28406f077b4e0f0fccdcf4b0d68874d16828f2e06743eb55cf77810f0ddd91f1434e502f7e7c50da360b1caad58d8207c6e8bc391d03df411c7fca5cafd9a79309175d29d69d1c75dbf2b8dd2b689d88c2ad728a1fa50e19e0e59d62934d0405a7767896d371b184d424f07ae8419a7119c2d77b8875f08d9e0c26e3c4529ba0b4193c25e7cb2a1e6adf3cd7f242d128491667446fe61d2aed819ed89822be3ceb37f5d5833394088d9e700e490fc6edbbc03ccd0b9bf921ab994fb1154e238e21d67ae1dd4575de090f42704bd8765f26fa95a77058b75690c9a8284111e07856da19fc8d11917ff4a0a5";
	plainText = "a4378c00f3c183b9062c4afe4855764f6047479aebb59a4429c1b05944a3533b01e6c6f2a37065a4c67d23d9ea39a8fee0c566a29ae4b3efc68a66c730811c0de818b5a6d65535444298d19f5c6d936b800947d1c696c7d2691e07551cb0342b6139982e2bffa58fdcd01cf89f4be7fb0e6a7d4a0f08b6ae112bcfddeca9d955a3a802af7c5b5701971b96bda118a4ee5f6b07f13aba8e19f77df144fb29b17aed2052e9e484ade536f3770e4e26bb1602c905aa4ff7abc60cd78b5cc28738083e767b336ff87ab7312bee9e543d972d2cce1b592acfdc073f50f231eed8aeb6977eb6cda20af9c34819220e00b3d5421c09619245bbff74bb15a2e95cb69e09912acbbc28ec858a92b96fa0d30fd145137186a5cf1d4b0cbabb6225a33ef71a69219ac45febc093a02417b99b2ad14e5fda6e28212a6567571e3559a8a9a0d07fc3621fe64ab28874497832c3793be57a965f840597679e4d3ec1261fa891bb2684b20a139ca9502bd9a3ea7a751d716a88948ea963d81b0818476318538d4b40c61815044e8eb0b55afa1218994e51035b1ce9435cc208a0e332229f79e5673a487d7e913cabce8cf2e3c59e36efc11e15cc097766b9567a1cf882ddb1afd61d17e6082b60d56c70a8b9800d889d62da256d93f679b1511396f7714b0dd8124a53caf2947e2df041a584b68bb199f4e17622f04571c2d1b832b612a51a80b2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 457;
	dataLen = 4096;
	combinedKey = "69aba8028a195f1219a3b961f05c33dc411eb99dc458e86cebb5c8d6af3b6664";
	iv = "dbd7392f87ede5b21a337df2f511dd9b";
	cipherText = "14528e9f22dc97a9231d7c75eb2cf029999930b229c8fd6f860e6c6ad217a319983061b2c06038fda32dc6fab7419ce3ec06837fe79313709d74e1d0dd5fe91dfff7c7514cb2995ad0a2b1ed45016b76566d8459fed1e12d8f2425c6646de42e29785017fc4c8fa470786d6ee6eb1accc3464545a182e45761a560613954cfa29fc0bf4b5a4490e9dc45a8ba3b23fd350f6931c40a0e453bdcc48a8af98cc7b691aaa4f614328c34bc0313b8e6d3d181c80cd909baf42c406c8232a556c2d0289d7eb907223dbbebb221007952c0906a44f8d76d3789308821db3df3a32a149e6e275be9e479110d5b2dcdf2a0b965e9c943c534d19c2e46b4da5fd531cbc0b36fc89e5275e5ca7ef45465fe5ce2b67b278f01d505fc7fab1660dd044f6886a0eb787a807d1957e1ca577f7ee760bafdc67e5d03595e6a8f099da45eac45135de6fa60734f41c44f22876654477e5592e0a5607388d8bfd544b42d391e719aed0526434e812ae458e0f2bd15ff85b2b3cbbc486d12bde0a13e440d96abbca42ed179e2b544d0cef8ddfce60585fd71c67136f0fc4c56bcdc79a507ba686c09b2011642340db294e75ad6565f4cd204487452c3fa23dbd73c72708b0e84f9c42fd992e3e63a953c28a6f439217d92dde27310042a14d3759087e6e07e11be038d2caabd042e07b01131897d90b15c3a7093426ada26ad90badacb17f76a5f8c33";
	plainText = "c93f4cc620ad48b06f637df4e1121246b86bee2aad63be533b7b3bb63c8f0bbaa61a5bdc9997bd7612310aac06e1517e2e6378d34062461300676ff300f463a0c7d2bb44bae3bd35fc70151e2cae23c3aebebda2cb69d43c73d531ebd50c118cbee536959b55a0bce94d000aa730aaea22f03f8507aeb29861f81796fe9e8800862d522e776a5ddf572e1253acf60f5fad98cde9edde77271df97cb89ce8fb4e517281af7767d7f9b4927755d7ed5fd705ac163b8f4e4c31792f986b45b75d3abd34fdbb20dee02ac47de7fff36bd83bfa557fa6006692f40040f0f96a8f487fc12b6809ca7c3e782bd072c24608e16790701eed02d401c6535d1da7dd2dcc1f0760ac5c895bbf70fed15850855b8c13517688d00b10f97aacd6d1d4e52dcf69a6b073c7a3ef4646097435a2153669449e1def8595e15add4aa37a838027c9a969c314b501d47faf83583f6066d16450727965b530a8a8c6ef1da7fbf398852f880810ab56063cd7aa7396971c0d0ba56fd081dc3e5524906226036b28639d943c328fdbca68b1639f66b2933b06896d9bd0d2e66f7fe42f8724383b63f2cfefa210c5c91a6f86b1cae7d7f8349f8b567846b19c118690c789d64e53a3665042206b20f2557bda9964a59a4cb659501c86da7d159251f970a4527c70091cb8ec33867681e86b665b12a52e02b8a5b4105256c30901e51e0572ba6e2e6e9c20a7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 458;
	dataLen = 4096;
	combinedKey = "40dcc8b399fe21e0a8737050130ab048b9d24255baab4e90b47faf2e7736f0a6";
	iv = "e184fcd5247bfbf6fb285c982505bae4";
	cipherText = "aa7b870e99440364cc0956d05b26194a9e869fb76524a831faf6268044de0e939ecb032e15ac7b3d795e5bc9b10434b230634ac101fdc78c45c1f79f0f0f6469a97a71e58f712778058d706f6cd1b5720b87c6c1d3cc8ed5e68b99c1b1b0276bad61af11ab2b72a4fc3a5642ad40b3e44459be8e36987a8927ddbb9b41438e8f62a0e7754dc6b503753bcda53a3e0a62dbad39067d6c66d30ab0c4f99bef1979509d302cd8f3f244a43113929a1e23625f8ef46f6a5925c044ff684afb681dbe0624733a3238b8c6439f011860992dd2205e991db2c073d076323031a00e71adb9f2ada3448aaf7480632a68a7155da73c0c271b61bfa8ab8914793c7c0521e48c5910e1bb2945dd04844d700b4bdbcf1ce2d017984914063f700911cff80be4388df0375f049cd0c973d1613d490504435ea0541ddc17b18f01ff2b3c71c18018fdb92248f4446e5179a785a1d0ed802bfb7bbd160cbdcda8c6604e4de4304dcbd21c4e35e96316c5ec635d7c3056e587b86f97cf6575470004f5304fca79a3b11eda9727d4f3fc11f6ad79ac983bdc23de9fd1a7ab22632c044c85a05e7d48558d24bb1250023e1fe772628e694c7a7befbbae6fb656dcfc2919e9d7dd4db415b2dc8132a41bd321815918108f277137b86b6143816a929e70a57d3138c51228dfb1a7274363441a7dafc70a1222f8e34222774abc91e430c434f47742795a";
	plainText = "396667f6983180f33f89391cd8aa0ad21379c049ef2857a98ede609aac5fea2a1cca15e9bb8af5843767357a3c8083ff8b66a234df8798b66934d8a7e296b1ba84c339030424f9b2d4c665c332d9b9297f930c7ffe71bca4e9bb97976d787cfbdb07f2e3840435b1bc8a2930a906ad6801f2d5ec6c7edbd80f5e1dc75f8bf8eaaf1978431143bd23659bc01ae964182bf7f99a67d9a0d6d87b357de8566d9f61949b0c635c13567f66dd4f808188dd16b85390ceb30dd9f8c4bcfb60d6bbb25b68b28f399fcec8364646b112241a19d2f3cc28974bfc9335dc1e701f645191c35db9eea4b9f442134309b5041ffdfb2a1b590dccd636fdc3a230005aba67369fb2150dbc2e55f05ce0abebac398826e884d941dbc9fd742a4691bce97517e19c45df250ea56da650fc5041d90137ece42111453556cb22361dcb242771256f9bdfeb7e1c5dbe4e7c7464a1fb0d1be1973b302c2dce1ea609bad420bcf13a3a21d8714442b299af59058a8c0c0501176ce5e686f1c841ad24637a28bf47f3e92c962eec398ca9ad00ddefe2995da92b5b96ee0c2c5f7e14fe6d87b18a63cafad3b6c3d2ec8c0bcab071d6d1e994148dc1de5d38e0edc72797fb1b0a45efbcb968d4c55f190587dc7d015d5ccd3083f0b12b03bbd4961833fee4fb9ad4e362db174bb6673bc7409dcdca82de66fb2a7fb34a1f94ab934250c14dc53f7b1f9d88a4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 459;
	dataLen = 4096;
	combinedKey = "3996e01aa945f478abfc95f64d87abf03c50e565dc6c87420655e647e559879f";
	iv = "7ed162be1294eabfcbe6363aa0e0d1ea";
	cipherText = "9705644d91b8a05a1fa4e44c71d1a84a94a658cf7838bf970cc5f4d3c66a77742d1100e0b9d425716e32cac0ec389bdd9cc68c6d6fc196525467c8555b69a9c6f0b34b4dfb4efc9ebe4a98499dabe63443507220077eee2255ce94edcf48b8bd03deb83b96af7cd20793f82214646208eb14dc2dcfbde68264f1317bc0d70205ea482e16a7d03b9283d4baae8b0a066b159deb2c1ca2952e500ce86b69118ca315311abba4c299804456a5d7152c92b04a252d7d46cedf6331677aaec5ef3d799832676a4e690208b1d14d4d282b612820c386589d25cd7e16f680ac5368af9bdc658a464ef96524e8007361a8ddc141844eac8d44fdc3a1db2cf5b18bb0d39c22823e3d4713675dfc119123ca0084a0e078d93084268e58a058e92cd99938f6ae38083d4285630a1a06c79eb1c1d0d9dce8babb272f72277f099b1cf630ee800dc95a00cf7ca469fd6c1e9b650a7dbe2c1881b8c0f655034f6d3c42c5b0aba7e1c50c7c83fc99018b1ef4c10a237422c946b4ad30c465afba2d5cabfb57664d0ec451bef89c13cbe5e32c3595fc136e9e381627e92fd6ffbfae1140855331cde3050368b14e42ce4f1bc4c5da72b2c1529fa4af3bb20d3308f4ce8683285565bed0ec7ab4e4ebc6f9d5d2ef160daf1babd5bfb8a33c36041228c05c2cd5dc590ee0a3181b411a3e7731b3f7f5554993199d16b8f9187fc70365aa089e746fcc";
	plainText = "26ba29e4b38476ddd5664161d3b0f8cc6e7414e9e29cab05cb1f32c59bf2256f76443253bbdc6fab5cc6219710971f7927e68b0ec29b240aa1ae68c5fad9399877c6728c9a9bb6b226dc74abd45fedf23692b1fb2114d44f1cd3faa7a60897cc548b4ad31f9c299930b5db9891edb8e1d966a9d11b11ed06f8347f020324c81745246160ac9bf68be36d1af2fd99ea5b89d16230ce7f7426de6e9b1efc12fff9a7188ec5af8fda7a4fe2fe582c9b6c4c3bbe4a40c5dfd822ee973bd0e456c1a3f642d097c1bbbfffd6260b6e1621cb02da7ffb4bde0180147077aadab232722b8f6e0b3950760c435baf95668cc3a64c170545229be15cce8cbbaf339abb42ca411f5990d1f56fc3f6c06799d5b7ff7522e68c586eb79ed7737059564a78a0ab449ff954c594b0b1396de13642c8c54c122e2efbd54a6806368cce67e78cc3bf6a7462ba0f37cab36ed292337a41988b9623e8cd1bbed5a74f78171c42a40def6bc8a3e4d4e754c00b056fe0221435798445ed3db2c8aaaf838cbd8a6975d0ae6b37b54a3ddb90ba45a0671f8fb4af81217be644c0a4d210b588781bbb550ee6cf28b40844afc58644d936ceb54bdd03be830400c4832e2613906a842ddf895156249605b49396c1dde39ca02898562cde69af653b2203834612714d61346960d6aebff220bb3d8578a69ef8712cfa1446eed63cd106ce244e1db1044e998a99";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 460;
	dataLen = 4096;
	combinedKey = "93610b15a88dbfd1abd7a8cc1355c1816850a3f786f7226db891e277bae6a228";
	iv = "d0a779ac785feb3a35626f19e158db76";
	cipherText = "1569e189cf201f26e8213704205c7765d1fcbdaf646a780c821d33e05fbb00b6ea8145a914c07f6b0831973a32602102fc392e928dda226165f1481f8cc49e28f7769fd0ffd8e611764496612d7f968c102c9d65d72b16b152238717efcdc26e44142587880626362842080539ee49d94ba531a7353a8e36b876555ad2ff232a73b0b6d082e4b6310e8c4ba61e5630e8b403068e3004fff2c199193f48f89b8534ca35fcec4ec6378082f2d40e04d2d3bba0253ef50fe5de7f611c49a21e0439a5c5960431991b26c1ac629ba619c5797799553a1a534ad8e89fd71914841768bf3a5b4dc78cab4db4dc724b154ac085fdc82a19bde1f4e5e83dd45519b701f4971a8e209641022fd58b15d35392dc0ec8b77a8c7f0321b22c18f0deca7b2a247143c2e8c70f05695f7494b626abf7b8950d33d37387c182774abca58f2324801acba84c23a0fa798e98815a5a306c8e9c93b6798081dbe2effcd48db4c3a58efc538e0a5b335de207f5b461c3edf8a0a8a7994b0e8e5e4d25b4ee3f31f14858fb5728cc3c5bf9bab7ab4e3c1ca3eec9dc70d2a6a8411031f92dc1d3e88c4fd49ae1e6889d5defa3f03c5865ad910b5d1455300a7502bb2ba623a06eefd1747d55ed466a7d519a07d7c32093ee46db3573a1c4df8aaf526eab178e411da7acb3c1305541044809d1b995d07d1d930e7052ec14e7989cf49c4dc183a5eec18eef";
	plainText = "34d4aff2be8dc87451aa7ca6a544e45efa9c78963cb0ed907d07c6ccc398b0e00f2c738af0f0abde808d1bf32849e89bd083b21892ef91d292bc32bf1ca7ac47cdb5d808ec332734d783d7c39e6d859f0e9ea72cfe03accd9dbf9e90ed9bdc17604f0594bb2ab2783e89cb348c138f49a4670285333f10230b5c8f208c2766da85c17b9f41d4eee238e9671d0dfaef57dbb37d24e64dc3e545f959054e04a8e9ef64e9d00b94856f881649f52dd060219a0f017a39a680a9052f347a858865756a5a76af5578c7d29bc781d16551f89c2fed6bdc8afe2b4daa89d23f0dd18072445e577d1f9bc6f943758d70534114d8abbc99c3bb8333667a340e2cf71527759e461466b96b87a86f905388779f097f025172f270c15a35133be8223d08af4f5b095507513c302e3b6a2d6cab9e603de19e6897098e1737ab4f4a2d6ea8de5cc4a28c1059a183fa4753aad7ad25039118e42cea45457d258fbb5ef53f093532443b1de4fc67c40f805a547e5daf41e040a4ba5b11af52fbed95196d1e8f1411fa40a2eaa14e5cb60eed54492af9eb21af9544fbfc4d4e1b8a54a54c4d9279b511911b364372a2b40e9b4cf4de8d27f3e42495ce35d203a17cb3d4bb7c4fa7d85826629d36e301d9daee445a2d52b734588928f36a4b5afa6731b1f21956735e26f444fe1ecdaaf40da2d1d16111113f8192d633956ac750aa8bd8d7bf4acb56";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 461;
	dataLen = 4096;
	combinedKey = "4621ef4da668b0000d863ad7974dfc6a06750fc6b18c57ee7b8109d78658034d";
	iv = "e40b22231b673cc685589188e225586a";
	cipherText = "c80ca764a221951bcf5da619161061401489b9758636eb37f4b10b7a3f139176955068988b739c5ddee22ba9e20f94ecad3958d6417019aca865a40b8b5f5fa3f04786bc1146738a02b9bf40761f49208aeafb04d0db82d607f5bd606f9cd73702a0f1b1c03e4021626a9cc0a1ebce01f6ee02d03b41156ea57c8b15e56bfcd89d8a1b23c0b059a3b288b28919732f2c111926c53c3e08d59257b8bd79c420fccaa5377e9ab3b7a2480d75ceb6fea3196ba2856003797366e5386b2d06a1814ad816d8be40e59572763c877e9bc6ae727a6d2b680d6be9f8ace190ad50d69233ef6b5caac6d8013f700ffd331f715ef64e418d3df91c65d5e86e1278eed12dad9c19d7b65899ebbb5e7dbc1430b5d16fa04e99f29a826b1d78146295e5e4fd52cc54795e42891fab6f912876344ddfa4a66c0636ca59e81bd97d61a0535cf0340811e3358ef3b5b83fa5307fb025b3894304e9ca33714d1f06c16e454d0311dfd789b82977494b399fa55736e8f8e7d5c6ad6e8a02ec49f80f0a2c2b16e63a94cb1086e2be0b5fe780be0c87a3ac87f2c91bf25723e01d221256af075bb1afed593a81963287024ace0ca8b7cfa6dd8bcb2e98a62d3999d1d27a1c7cdbae7a221115d14bae625d83fe5b0016cb5128e4e8cde0cedc1a1c786d435e3eb39d2647c252d3723055f662d494414bc0c1a932d074836cf81873edb83253b6d74837cc";
	plainText = "7769fbdf43df9a638dceb79d10c6d74259f289bf5d43103954d9205dd5b1869f8119a6ed39945e25fe9199f78b00dadbccad335ff228d18970bbbd83e36b07e19a180e9e6f20a2c93cf504c6cd91609f27301e63fb93a516f33b714fd7ac2ea9dbd4420e3bced280e8160fd8f5164d7b3583d80f6711bae83ffa50126522d99e6961a3cf2aecbfcf728fadc46d28740bb0cb1f1f347dc1c8a5a7f476e732ce8ba49a4033e93daea719c60d3f4ee58ea10ddbd1b18b20903844b0dc7cc86feaf1b437fd9d924f6dfa283651f30ffff480977d04b5ab45ffde20280f1bd0fab6f20f0afcc7822636618a9b8e7e0ba11ecd45236e4fa8e385afd8be947c99d73b4c827889806c04f27e20a168c2ed9a20ef22a21c37edcb2f5a8e8e0f6487582fb90ff8b894ce0fa3b2003b520a3378bf37c0cdc1ee51403fa6052a7992f48f7887085b854b9b0a5b1c30fef43552b48ffaa178cad977917f114bb300ef7d7e17d81cf0792c554644af035bd6180472cda17ed0b87b19e8e0487bbaf8315ce9b57b34fd1ab21103dc8b71db9af6e90e113fb304344a99a528fb123e26f709a80c9b03606c84b4dff7e755ab687beb60daab13dca485dd7d0d7c70425452d169468d293958d2ab2eda6143a9cd66a3f5ef58f91b7624421f4c2a655c7f3002c164c5e1bff974a1f9cc1490a66c0fcb5ac68ee9475444549286b3f605c574a1de743a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 462;
	dataLen = 4096;
	combinedKey = "8e51022b0f43a38a4412101015f36f656dfe24a465f496e45258250abf57bbb2";
	iv = "7563b78ef16f73b8c89c6f530d58e57b";
	cipherText = "ed9438a2bcbafab012dd7ada434c8b9088443647d7e95a27fd8286fcc103dad1d67382fb8de19d739c43285b5cbc3b87c0bdfda2253ef81aa951f69e2708d06b875322d3ab45d1fcfd27c87e16bf8294c21224efb9032c8daf9a8a4401f2c60467d69db6311a50306a6dffb23dfecd2a93722f2d4eabab34db7fbe660474a59600caa9420d1b46efc14ed23743ea51e7ed22c71b4a610a0f2a60b5094cebc26062ffba28e46016ea8f9257ec2465e0f2c9efbcf3fab3a830ab2a47300db4fc5d780e8cd1529f880f43c2ec3243b9d14c4942e901460e2dfec4fd8688ba96b2779b7c532cea9245ac6cc382ba1d4d6afdb8919f763c13e9e99c6c77dce14973b96383377cc6009e130f975bfe7b8709cd51ab1a49f97e83ff2ea96f757c43c5d9c1d5c963b3ecfcb70686950c6696d85b6f63d6b0def536b9eab922a4ffd335c06bf0971d26dee99e09c1b5cb168e05991da6d2d64a6304d1a65957d4f8aea348f6b7e59476061ab086ff86c6c5525f75f3ae95182db5232a1c61e0355dea750a7597a0dbdd0b4c0944be639eee8f3ae76d1e2beb63b899eaa7bef7fc5f86d16f4085632d4f1770015853922ff75148e824283d74bbbb1ff43b377936d1cec7dcf20fee34bd20c61f099295eff8d66542039988ab958db31f7036302466b97e4fa8fadd5c29aad30051735afb5def09ebd0a3f2c0bbb9da664819bc7e2f97403e";
	plainText = "0518b1a9594eadd0dbcb290b448bbe7760c6b9077cf1f824897ebfaa5f5cfe3e5bf7d10ba8b85f642c4fa2f3c8944a1b9d2e2bc7aa0401cbc16aa84612e5608bee6c434e537abd8283d4f3de04421dd07c0ef5e910a267038c571fb0a8d52b679d67754730e254aab1d8286fac0700172d3fa50b121a05abaf05ebb98e042bab4f7c2faf5f202437bff34bf4cf1b23ae72a23d164f28425aab2247f80210d75fed84213329838d601d047ae02f325cc743b246ef38ca2b8211f87dd8991cb44f589fcdae6c72a8e13c76358c1c31c09252c02ab81eb8be86d6911b714945795173af55f5ae7ce9199e99145e625fdfdb42bc73d8c67cc7a616d0fc8948200cc6509e4d500fc7aa75625a62f7b94c24d9663cde18fdb65ee6bdf31551e8d4271daf806ebe7d5338fea68a9dfe8821d1812678b029c376c9053bb53619f7c0d9901c7d5cbc7e96d733b9d11a892a6330b68795dc769de37a1a25399964ab0667231c61f3b91c5125909d8a3988ecfed17d9a2445ff7feec2d84da620382d73d713edf4421fa7ad5b5568ab1b242d7eec3f701f1c6e79888304b4f91fc612e1912d191118c3631d4b4e5668b0d54de30f890a4baa36147b37fe726c5c2e189eb33c26a27fa620ade19db8e7fd53aba382cd57ff6ad1681aab1295cf4ea5fc9d30223f3b130d687cbbe9fcd9fb2cecddd14e311a3244db7b0b51055d4c4d26eedf26";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 463;
	dataLen = 4096;
	combinedKey = "d3cc33816b25f4c16290bb6d7732b0f10a772e21aa52f3fbf98dcda4c79a80a9";
	iv = "26530bb2acf82d4abd726e07695839d6";
	cipherText = "4f283c05388226766b5abae0d41003131112774fa458b64e058fb548de5f0db9b6dd6f0e3f895887f6bfffe38439e11f94af25f1ef59c83f7fbc8ad29f19a450a52561eab60bce3d334490e87e662c17984526ca5a7ddd86deb9006d19fd2052cc66fbcc0ee3860ab5d6c2021026c9c7e7b5b0f572f0196d8b82ae30ae95a07eb63eed9ae11ccb7b8f4b40876f5decc618e2ce0057992812d3e4328d13b9e3e40ebf37f1ae64010bfdc028d69a1edb3927bd83da53b1a3f7b7717edd83b8bff3c7d7152733ebea892dc5d9dfa93686ec6a85f4110baa4821026893fc185040da5e39b3ee8e5a311b7510cde002713796b2ec50566958276f01da14cd4c82f72aea71172660c2debdd25767f23404e310861e93dd8b573f2f82be7676d31d498aca00516b353be2547f0947f09f499372ceff314bba5b90188997c404289c296bec9214a216e885886a9008d0c73aafdfbabf26afc6977b11fb3e2368ca3090b28d65c69fb42ea8a75d30322d5b9553bd3ece288e7179b9c4586d436fa45e24c1ac33543d28ddc387ed1625360a69a41820c085c37a6c6c579a04d171b81deaaaa1359fd16e5e80c8d2d557ccaf99e9d7c703b126323b81f7aba9110bec0ee2c65a67a0a5f3a24eee9396c40849a0b241456f72b114509570ae1865e4635ec557ba6449b9a1d2e02789dff36e59ecdf2e390ee8711b2a022a71b7fb3160268ee0";
	plainText = "ffd222eed6da99c36333d7ea3807fa7946d998f9f765a2ad1450619072a50d8f4762cd0750f9ff9e5463999a70c04d97a3b42a6cf050a941c985dc10e2eb150e319404ca334ade7e12df60cf20d9e0ce473d18d6af80f8c55b87781bd0ed5026ce936bc299f7b9ede0d7b113bd53a1dc3e533a40b1e8b5f0ed4869c154e0f13d039399d3cda683e440c906839e043fe2597acffdcbdac9cbd000618ceb06dfad80c205d6b7558c94b07de687f54e7910f4b32cf9ebd9f0e06533d343929bc85f5519244b8bb3f42248a5f57e4c4c1e08df8f1e99a011963266c3aff14fed44907fdb95aaa02419f896e8852626b6be935af6850027c78cfa44144c4e972a314182a253f490a9ac0eababd69309c546b079625e8685f9e9b0047a24f863a2f698652b88bd508b13b24a9b4f8896a9ecd46573d6a55b28bf3ef0f03469637e7ebfb47c03e795f3d6e05f3491276c37f2fdd896b80e585b9c97a474b485bdba60378149846410443a957a737bba811fe66db45dd8730d2a5a7c9e00f3bbb9d4412e9ac5ed39ccfaad071826537cd85c0e04b8ecf2ce30d1c3884894f4c4e28700da2ffe5dfb378b7c0449731bd177b273022da0f0012314d13044c066ef6f2988fd2a6b224b2df0144424f8dbb2f637e59aaa2a0cb8e4d2d7f9b7e2456ba5b1e4eb08f3eb991f2bc0295841dd0a9a5a548e554f42293865bc86c9a8967daeadc1f5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 464;
	dataLen = 4096;
	combinedKey = "9073c760011e2253790d9a2857d59622ea1b6df45f59ff51758232c2855bc059";
	iv = "63585913a74c3f00ad5486af44e8c37b";
	cipherText = "33a99370572577f535193a8ce63a868232806d95df84cad4501b8cbe00e2413335d72208106a780a0e7cafa204b59a284ddbe667eb95f7f62ab458fc65f8e00c05646f48945d7db8bb5fa7404c0aad94ae408268d6d59163f883476edc7d80b864046084680e1906126789b0b8c5835846745f2bf650c56ef9d5cc949e118d8c9fd531cac5f5e1ef9602d4588f0ed170af394996379738b5f87c97f8fcd79d9e49253a423737c1754e12758bd9ca9c1a774c5f823411b42065d433ce7b85987e04e80c0f149102f49cc2b80c8b2ef407af08c7e9f5ce67deef4259fc20a5a4bde284f76b1908ca9ce2cf56e48861fcde1b47706ef02af016ba3f958f2c3730321a9d53bd5d20013a514c884139facb18bfb987fe2749c7480368c03a8a921fdb5f07f0f8efc5ea27d5c6853ee545f84de1183ca8b49da6cbc5a25215b0d99db352d556b670c4230626a0cfd06c36b7606ad9cbc4fbc61114f1160ad1dab612157132784e62bda224167f59f7f000213623cd80f6d594983383563d80730275f425f2e921505f0438fe3e5e4670d8b4d5c81f097fa024029d7a2f7090123432420f503b7fa9091063890b5cff176bffa9fb39eab2a0ec7e721d5cca69e1b8744e2603777a966cf7ab03ae22e7a112afd34ea99d4468904d0f28466b3febf411f84ebb96e68940a6fa6b482999184032bb245d95b0a4baa84338646e8fafda30da";
	plainText = "3bc0160036336ec81982e9bb213441f1a7418d1d68fd36c5cf2ad43ecc491610909c7dc8f9fecf3c624ce68c08c422e57e85347800ce831c8f8b4fe07c2b3cdf5e628a380e440af516d7898c010075c70124cdaf1697fae0c06ff1cb0c34de0dd06183046d1a1b1767e129dc8502df8d48fe17c3a6f8604f3e18312e22deb195846709aa2e8d44c7a6de097d050512c00340e72bd13160c97b350019267090dd6016ce10b86ec548d0db936ba553208f28981577f5c429e48f4e57f4147d6c9b48e2f4455bd340f129277a7c566e94fdbaf42a180567d69fdb47ba65d8ee8f86195a775a62570ccc9abac762aef51c7520421fa40e7597454eaeb0a26506aa7ce5f9a939434fece777d9708e3a952ceca251b9d729e5c6b94daee16b673e1f737f7c72b8e2b2effc0be0a85ffa5490f75300b0702220c0dc9047a5d4f7cdc6fac0241470e0e350dac26ef03d22ff6b594c1e3077a86b232d17437515dcb255cf69b8f7508971a6b057409d324a516f0ae5b0c3a4fe57c0311a167bbc56ce486402b93135a8cd2c0c4bbdea2855447679967a680f8a40223fc0333332fd2f212dda05bed0a2c059a12b18b867ce35e4965dab2009b8c52e4bcac5f2ff676e7c8dda8316847fa3c82767778f68ca11c9ae3baf3656e53228e5e8147e65aab99a50c4f19d7da24ab628ad4754f3a43879cf024ab4531b0284c579d797fd70553010";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 465;
	dataLen = 4096;
	combinedKey = "e2703781032ccf1fd5b183b4c6ff1f27f36f2849a704cd4f2353ff6cc63bd747";
	iv = "94f6d548b3657d9a356d5c58ff24ef13";
	cipherText = "f25c2a1a3a1fedad9e3c5bb107e6525a6bd75b3562dd4e26daa32d7cb10379776ac5013729d67b101e95fbf9af234ed2ea0ae29c0e0c213ddd3172cb79d4dc62efb8bcc2efe71e1b28abaa7925baa9d8f73a9c36da5266ea72f4fdf3ad0ee19e455ce558d251e535c6da1925e36b2b471aec0e323d49e326c29937de66cddb3f846332d32744efaddbd21ff9994a60f4e16f6a6121b4b2907d40ae4c6c28adadf89ca21da34c9953359ed575ae4ad49ef9f151d00430ff4eb4afecdf884d7b5caaf6dab3950f53ed205b78538bdb261f649d7dec609c22ffb6924ce28631d8961ae70e24f486c5899c8d6c72e248ce6b4f3e30a46462a924f4cd5ddbc769d127e551a077e1a87d63055903ef25f4cc0ce367de8b8aad5b8360e27ecbe09bdfe26dd301363cda94edbb7208f216ff496b5096df306d9ca46ea00e19fd542dab10b5ddd564b8e9d3d6370bc7b416e5e20ac4048872ec66b9ec07b2c8d5845fef1b01c5d5082786000762b0e9124831a329983690cb2edd72011e9694526956f8b3ae74e0e01c9fb5fbe06fc638f3f7489fa4b7d377fcc5fb83b5c29e65f3d6092fbae33ceb974fdd6de0556612c59374ded6e71aa3597d66c6257461db2600d0cf96bbc85f9bc4dae60247d1c3fe976e23089d8ea1977edc04430fa32087de830f71579f87eb358701c530ad13f6825e2e9430a2213ec2a338529f56eeedfbe43f";
	plainText = "dedea2562c903def6d013810770184b235864cabfee80a3adaed1430ede25b018260b676efb66ac0d3b191301c381ba9068a89d34c20804da58098208df9670977242219acc9ec83cd43cea884388a01744592508e5aa2004144c7346cd55ae06c7862a168e3d63a2549a82f38ee221d5465d52753918681d9be409dde6edc8323c9a972e731e0343fca1047d08bb9510f9bb49cae26291b7573f34721a6300957b66ab72ffe58cdbe2b7f8e90bdd49ab3e05dac4386474ba1ea82552e2fcb5f2df8cde4b4ef8390f07285f84772bd7ef5709aac5f0f978c727816ca2ebb4142da8b04b2214703ff444c13b526c8dbfbeca4e679effa1cc28673a69df8f92820d52aefaf33fce5cc6fadf8884d1208888b283276ba5fa546e92083f74583f55028dd0c07d29c8b9c87bf362ad5b02b6831e9e3e7824be1f6941e27c6e0ab06789545ded2b5527b5e911ccb502e5573fff4e8afb42c2c67fa033ab9f52f2f38622e488e3a7778d1e256435dfd29426135ec4ff5341fa2d0b415255f1e43784cdfe5975a8d69aa1a1210d0537a4a6e24e22d2dcf7a4f3ecc146c8c9f35de8473ed5b50803d9c3745571ce0bb5a290dfb00df824761b6c43dc9b4fc33f0cd6b0782ddee1b7a12d05ea5a0508a97c140a4c5a78e0bcd78e8aa2fa332343e8aa14f857184355f15008ce8d6760721eed4a3abad295c755c46ae90b75d6397e7c72efa";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 466;
	dataLen = 4096;
	combinedKey = "8da6f43fc4e87d417759ad97deb5d6d077ff803aec336b55e72dd4bcf29dffc0";
	iv = "b9488128273d2e51df47208be9819e63";
	cipherText = "d99077cc9c27a08cd1765be2402bcba5db9070af9b3c03272645e03cb0ede839900cb9254156b3a38c5b7df9d00dea6fa84955558a37da7132290444c9e21bcb4b426ef8cb16a8ec66737c2e163a4ced216792c7e839c1c9a9633af581f0c8e365176c699edb884d86df30f47c34218c3eae72790daaf16a7487fb149a617f267247686eee2b40259ff1b3c875fd14d98da9cad727de76973ab2160874ba94789a09c5dc09f7e58e922650bd160644590daeea4675ff1ef39d52881142b4ec98ea658735439142a1010655c690e68fba61bc4fafa72897b01353c56f47dfa506051d5091277115d029385d16559a879b90ffcc3c3e7af8def62ffc09eeee712229a82fc13529822adf60d581e0169d3e84519965c18098caa3e3fa1b88709354eb0c41bc7fe55cf611809fd9902b640a2c231d436222b884086aaa095829a1bc892d0e4c3d9cb5b331b9974b0aabe75ef2c433ff2170e9674217b6fe880676ac668fed1d15c32b5f306d948cd3b1df6a8506992219413cb2fad373027a0bda80a30a2a5a0112100205d97c83f8c35f3ee640f39fcb94b5866c2d03284acd6149f9e18fa688df32a85162d71cac4a0df11b37547e9d60706c46fd7c7f4e4bc3572ce349b345fa1a481737042acee44d21c730159275aa5e41ecac8339d55a7348734f23c0ca5247c2618476ad460848127febc5af28268e6b452588c3d7eaeb62";
	plainText = "b10bde517763496a812aaa6547f33dff8e1ea7fe4b70e207ffd2d72131c45a2000ed52b84fc4ce7db20f8ee2a2a1cdae394a5754121d201637b60db0a21c5638a8721723ff1c9cd404031c96ccc41e4755615bd958732c849373fc247e26dd6b519b83c644aa0c070d56771f93887538c16a0badef448a5fbfdc384edf3d99cc59d210ba412aaafa110810c4fe29a720946b483fb7903e98a8a68b9b1c020fbfc6a4cf8f439f47cd240d3af56d3c6b8a81aa9ed73d144a784bc7241b17c250f3e1aa72f98bd03ac381857c3a7ba5a48180c4b1792660153f8f017a69f1f1b37909fbef843e91d348cdf7e0ad0400f883e5261c79c94caa934c513e8cdc72070409e4b869f0403fca35c21358f0b127177690c97cfb3210a4e53ff61ed531de56669f401667d020f56ea5db9192c945c97e26c2542ecc999bef63635fcd788e7aa6f27f1e6ce5698cb28af097aa70d74deaefe90ed9834dfbcc48ce5671e7cdb7dedc44c87a0c81f4b696f3cd67c81919e062cd204261b1905607afae5e6fcc7e4af26e56e7c7601822721828ad1c6676ad93eac392c781e52dd8b613eb6783a794a81595cecb9600d193d3a3cb35885a8c00e729613e0911c47034d79d2a4cbf7341bbc13dd0f991de5412d92129a1da7cdbec618640968e47a31063aa99d3ae12ed6bd473284755215187afa83f2c8d2bb6d323143ce6fc008aec231771d7d7";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 467;
	dataLen = 4096;
	combinedKey = "4c25aa88665574774f45e96de8bec4be18bd38fbd2e608ee9ec9f19b3f0cfae5";
	iv = "01b48c45866b1c18a325ad39260c627a";
	cipherText = "abaf75f6bc82a9543d26ebe41d94483a37b6d79ba692f98bceb5068ba43961c59775047a7395b030a0afc8c9103ddd310d48888b54ff6be65e498aa1aceb1ace2b2b25faae232780944f93491c2c98ab28c8ef056a8f020e585aff0a5759f56cf5ee5565ded31ebea90914d6fcd45c6185b4d31b8dc5e81b9175b3f77fa33ee58bf75393634f743e1a84d84cb7ed3c2877beeab0ec2302ba735ab3a0ab4e704d1251a5e43dc933761a7795d426cc0524c8030f3af2ab7b34d177fd1a30236b8cebf23ea9010520219bb64b5f714cfb3e6b148ff3d8288f8ae422f82a1a5688fa89a4dcfcb21a4eb7c884163593bd3c8dfadc7571a465ea86ab39552368f84b2806352030ca4971a9ba4a7f85a0e0a7793b3c950111ee811c9184d5a423c41cbcc205a96e70fb394fe6bd0ad9e94145fbac8a5e52d63aa8193d70268928b7d761eba7fbff1e6a1038ab45bf7c7a0eedbf17cbc7eac7cdc6c329880bf4e0b950e217e5868d96146cac1f2082945ee54dbdee5fe539abc4e730383f530fb0bd12515a22abc0fb3a7928ccee73cfe6756c84f9b9026e7c487be16c60f8d1aa64b2b5512b41335b734e99fc464b6258f7434147a4b6568f25ce86008f338884d494448995ce642404cf208deb4a415672f382da6613aa17af47e80c525cf97a8df4d3419cee2cb318d40b460a8d813d9b2137406558e221887bcb5e50ebb697d724b3";
	plainText = "57ac8a133188352a3b05c35d5d7d1f31a210fc7cbb83bbc67cfbb483e126fc964d67f28697f42f7499f7ba51264661216ec65685bc89d8a1deb23086e49c141c2592c8382b7e3f3bf46395243dbf85a7edb606fcdb3a7cfbcbf3646a4b3d27bf97c30ed3b85a2efb1cc052f95e4f5e00ecf19d7f120f7c4bc8b1b51b1391525110f066a96c5460ca219848a1d33e265a88fb770580fc93cfbb6c9aa409b48df91833add3b7b230793d69e5010af876d52d9bc1b2f935fb62d49234236c66d2a3e059d6a84919d49a68599863f863d6476c0ecf6ba0fba1e5c750a91bfd1b6179da48410a34cdccc5d98d4637dd38691a85c1c7194874abaf462c8461cc2836983eec7621d57fc1f79f6cff0507769ca94e30f32c91c78156365c1de2cfa6cedf60f4939b936b855679f9fbebe2536d54d9c5d36b98bb95d59bcc3b20e38095b94188981005b0e3c7b0854dda150936cf3e846881675e0f83ee42ceef6ec2411094480c1344c1fcd70ce22b47c38408c5140f487a2655ee259614be6f7c41b975bceba9f12c2e3331ade61eb410a3bd2dc29059421d2a979a9055286808b44d5fe5d06cb247ae7f1f59336dd2594bb6c98425d6b47faa72d20f734733bc83c25c6f5c3d6eb02a4293ba05072256c40368ac1b3a6b329c66c976d03c787e0ca23fb2713c0702696900b9e8756c5e9db70d6193ce1e367d841c41ac9d8baa8f235a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 468;
	dataLen = 4096;
	combinedKey = "8bc05e8cfaee9777af2afc5f22a7477311c9e57d8cd0659b8cf5631ac58e77b1";
	iv = "a62e692479ae168d3dbc002243fc9640";
	cipherText = "0609ed4c86a6868025e444a69e525643004d5e7c92e4fe37b9251c477d2347d4c2572c53aee0efff43a389d87c7b09201e9b995560f2c72e4e1a169a683ed9d28d356de237779d5c146249ea4672534a439eecd8c02b5442b8a6ef0f32aea0d610cd2f60cd58359fb82a819fdd198567f33aa3f31694b7ea05a89d1bb53c0c5c270ec126d4de3918ea6cd17c18072f512f28c7db1e1c8e575bba9c6b95bd92f9806e1f643c15518379e833b7a8d068b823fc7226c5f325831bf60046f122c76c0eb60491c97fb659fb116f54339f970fca9bcc0286ae513fa3b7c1ab5b08a315bde494a8d899e8adce07124e8bbb6dfed09230b0d44a67bd2f426236dff06252df7a7fff4b50b00568f4dece481c711cee075ea75ea9ac9dcff554c11759135c96e6876f86574f6b7b8a51af3ffd3904c0aadbb19cba1c66d9711fff3c9e6844a1955210e52f605d8b248080fe751d96a7ea0db385fcf9e00bac1b9843a1a3e906c382ec10bfb47762030cfffe316c498a829f2cb3f2cf6c18892348a500a298aee6de37129c732d8b37233b1c2f1d5c79cb5d7ae59d2bb5c8acca6c0f5dcc9ca98b829bb1f8d6de698c7ba817ab5fda9a2891859d964c55aa3e659455f6fbb6bf869194ccdd5982f58903373010b34232e2e2aa3c0f626d3572804b14f67c453e8a152a443fcb29f618c81c3d02b685abf6fa3bff9f6eedb8be7fc9251ba471";
	plainText = "390c6ae6e4efdea354ecc9cfca6f7c9f158fe5b2419489c5ac614320a2756fe80138ea50cd68c924e58f5c55bfd9bee297601fffb2c0af2bfcceb7d1f45a2b5ab3bb771f896a7b44754a558cfc4711d1478b82b9cbd9e154aa71bc6bf50e6c7275426ba69bfca27f87f6eef1d75df689127eec5184b69ad641dcf4c919e757476f394650c6aa25c0bebb7369afb9853c7b943b2850684fb661ac50f16c13fc9ff0a69db7c7cccdc8649116f53ad616b512a479cd5d33f22ca64f7d86f4dd0ec24ca58fa4f1cc25ec458d4f41d9afc11f8b337409152458875522a6fcd7002b41a89fbf01e5b509bc9285715704adb0e67e2806a2fd8e681aae5166210d4df8cb4a3507e2088723fdb0168fb25984cca995bb7124cf9f10fbaa120dc2c3d519f458d98bb09bb8b9a97c90b25adba858e44656f96c50197c5c3ad7da431fcd8f436e19717dcdbb1c333b51906c2df1af64ce4fbec654898d5729b1c43c485f1b5e595971782e23992dd94cb3ff81863e6bcc07e2e6a753208616000580d567e3df0fcc9695a78a12f70719702ac75fd9f87c9ba2aae213a4e817397c0ce434b99de9c621e1158a772dbb5d339a33bc5362e592f9d7624612c545a369c65d776621d6707b96b5660d578f83dee71769017a956a611f62cefcf71be719afb475cf77a3fd8596fe71105c07ef52f72e040cbef8d570ed100bec5a4153cfd9c8c1e8dc";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 469;
	dataLen = 4096;
	combinedKey = "77ef63d532c72ef5c37c27395edc16db0f079f19ab22174b1f6557dd91b5519a";
	iv = "8b620a90592fb5df64d0a0fb1822df8a";
	cipherText = "2a2d56ce673ebadc771dbc6f24ce900828e805f2fe79f4a0d1670f36c4e4b1487d2bb4c2489a584a9fbbbaf9d0f9ae29c135fe0949e9295197b509ffb66b065308aa5e3da86c7100eb82fc9851076eaf540134d971a21dce083ce315eebf54bef74e29add98bbffa24366c74cc2997fb1e9d090983114ccac7532b7b8124c5529a0bf1114d29de055f7ad9fd34a107f9c9bad8dced3e28e2adb09326b38f823577a912e1f52ec5455fd8b13d22c1589f3bc749d9d9bd276eb3d6557e0b3f3168a8271ddb9a7c368fc240a1c8c4f78de6fb9fd298ca72f4a881c7e9c8b2f54cab8da1d89058d532316c3312cb13664f925dd7cffae44b98f8ad1e445c2d72027a009de59204873e100871e79015475d5a0b3af9f05b61c29d9376234534d8a73aaca989aa7eb445575ee101b214036c00a0bb728be0018b4d1b8f4bb399e5c08340ca24e66809281fd9a4537f08551c0aa3eabcdd6cf745ef83a40f1e7a0a554218bb72c3955f158efe39a98f91a480e3771483e6acd07a6e1e954db3221db6f61e3214c7c7fe90916f9b9312c334ca12105669687704e7bab5554bf1d568e6f8b94625815627abd37812db2bc75cfdd4db8b9e50e8d22050b98c9d95aa0f774b716505194defd943c5b24369c6d630a97d542d87e4fcace85cf5ee7811d3cbf0b666d440389afc8a57ffbf60948b04602007edc14c54a67f97682a6b562c56e8";
	plainText = "a5e3d862cbb020c59c6af3a9d241bbfefdcae413c1e93d7226a169fcfb96bf6cc7c45d4052e1c9af8f8c81e90ec888e52e49be74a09290e6a25ff11c2296021b21d88c814285cf7babc49b441728c7d0fbb68f464887754695e4e70b2557052cb69934cd55c36a7720642bb9084b232ef46ad807b8d8a1e739177fa3e312350694651451fa9e83b29591c356c22d4c81360d032f136052eb7a3cf0d5e7c1330eeca70fddc24dc1a5c821d22e020983a1705b4e4744aa4511445926463643716d804fd3a9d93cfc298415c5de492ab6a8af875a5d65b032180b68814505e727c1bf5f17f00dae4531a522f52e600cd3ee57811640f0664423a2742a828607ef5595206d043f8582b1ca1f5d587083462c9358f5ab79cef28023cf7af2450f47f204ad051f43acd8cc1012b1bf67961c230eecf3d0e41bcc938bec175d35854e832faf280d2d56179256d2c3e5d51e024b7b1770b1d12591d442cf7f5c0cedbe9204e56ba660c71ad184697cc64f4c5667fdb2a4f4f8a528825c9f88e5327d642ea81e27690576af36adeee9e5c293efbb30d66055aa3b8ace5b8ff1ce3418ccd36f3529d43ea640692e20f09db8bb4b04436afb0a4e5e21b26ddcd283799a3be03ba768c37989ff5b0547a5b17209d78ef00458d029ebb9a3b5a0935a609336bc2b29a8ad6740ed6a1d40580fc4ee948eed7ab5903a97426670b76be4d352b393";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 470;
	dataLen = 4096;
	combinedKey = "fbcf5a4cf65c4c0f8167f5a5711a34b112bc489602f31e2cfa86981da9ff3524";
	iv = "06aafaaa6b1a4f3bd61eaa9b890a623a";
	cipherText = "13721b993bd7b9812ccb1f931de39ee18fe00934b155c58f45ded64b905a0a39a4b7735ec23efca4a88b3bd2ec33b9f63f93e30d4031cff0654f844ec4e865c191c499a2e10baf2f6032d97c971d77cca07d39a06c23ca1cda83272b0431b3d87d4a67514912853b9d757dcfe6ce8e8a3637c305abb271288c038ec14ad1b0207a8cc0c1f855643cc85c330a382e0f5167cbf934b49349761624b2938ba8f106d9bdfd762402e3e3e6c8108f6f0ce21404a8a80f3bc79c3cff83ad81c8a8ff4adcaed8f6b2eb2242e5289796720cb00004644e01442bb901ef7bc328a24a3615bd162c2b3bb745d2b2079b9631360ea31cda413b721fadf83d43e0cbf9c04e36233e16cae37800e05c11669d90c50a6c27d1050abb04bd5b927ff77043bf7b281222ff412b9022c231b5504d1a992901940413bf770d08e43e7600f2655d6131c46d81a990de79d59d6989ee808dacf93a32b0fce5340e732dbd0390d1138c9e850e04bd00ca1ced95c6f88ab78dbc860824f27a74851c55218004d3db5da20eeb386a072f327c25cdd5d1e6d5d91b82ef2f18c71f6322627a1b5b50182cf7ad2eeb525b4c77ebd09bfc630e4897fa76c5357168e49ea75bffeb285c473cddb6ac6dfc90b9f13d5301c4a43b06f7fe74f560aad802226fb928685973568709edc291bf4b879cf25628c2b900d4b25802e8fd1c80694cf551817d4017ff7157f4";
	plainText = "82a075bef2289cfb13cf6c05a27c7c56d1e5e845f009f845a06417852dc67bf380bacc56453898a23725328ea6823f917b70edaf05eb2de7a022997f311d33bc1ffa148e31adbb725dfe5bd7af37fd318eb2b4674dd508e113ec29f5f8d54716b12558c55e0057e683b4b6135ca257e18b9a962437b0621e8be8d392887a3db5fdac353fcd220603a2055af5160d64591f665919a28b2380cecb226eb1881b28932803f0a065e7d47e2c2188e5123fcf9a58627d799c14cacf1d98f3a51ad15beabbf9433cddcaa1665bc375637a134d01ef90470c09dd6658990e69e6ca2c673f11c7312e83b880457383f3dc379c73838b09b3992fe438a15e0923f4742c42795efd6077b730618109ce0f8fdfcc43db90d3385bed20ea95f2da95b7bf59ee170bceacbc86fcc20d808e120bca8a6d06d805ae19126ee07a88165432f90aa48d4bcd63c429aed0d740756089bd5bd84cb4d6781fc9d358b7f28ea06135690203846a4a1fe252e282c3ff047ac71ebfa9d3928e6e08642940c394770965bea09e53274f528cddffd9dd73c123e833811debec681d7cfca64c6fb1461e31f99c546d333cf4efe6d7faa810bc1e97e5221742df108c171d110e5b83132b32636527cc3a734c806b61dd8b48ad9d8bec63f8931abe6717b5fb5ae76497c776757963494ed20bf5c6d1a2dcba46abd1517848ed8fb75c2431105c958116efd49e9f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 471;
	dataLen = 4096;
	combinedKey = "9450dec98808f1b1bb8ea4bc6a612f11bf35ea6932953882a89d1ac5acd9b5fd";
	iv = "b7211a355aab6768d16a5824ca11de29";
	cipherText = "3ab65fe5bce36f019e52a03f97e10ca7ab7a5d564922079a059ba2f26617d9ffb8e623f5b81af1f0f7423f7503f18be441a6df5c7ecf2f76f8210d8c78502becca419af7000f15f2264e85a4034904b5e03dc63e6fddaa72b2cfd1a7bc98b2b3d7da7b4ad01dfbc37b700c4414b934b3b28bc3df0f5ea4bf3e8d085fe143a79486c2b341ddc2958fd4690df3f6c661f7dcbb715db8483e3280ead3d461dc68ac20dd2702a01cb478421db838722437b39427d4474aa4184d005fb0c258eba5493347fc6a957c7709e4fbdcfdf28dcc4974d32497814f45964cecd6d11a445eb8ccab852685af6c255bb5103f8ed43af98ba89bc77b1851732b6a749b9a21a40e2f43c05af0a0c7fa9cdfce52d82ebadffe4642ac8e1a946ae56d063539897a07032d1b5254afef6718f8081e6c2fc44131a2f5f7a266e2651deb75a99bafdfa85e2104ff5b1748661ae2f49a317955e1b2b9b3b9556c80a0392d7e7ed2270314e3bc630df923764bb25863cfa35d0222c0f953fbfe3c13849d2452bc8a5f1ac70b0092232e1db6325a6427d2b3456da058dc56758dbfb563d6416decba18c5e16a6aa33e445644b756fce502dc280da3879e145f4916a9119f308f7f5676589a6679f8690e91952670fd0ea68de527d5950230d6b583605c8c4eec7356167d8e7820f9ab174e6a9e50b777d10b1fe95a00ec5f915121d811ccec64b500e6b659";
	plainText = "1507fde6ebae2803194c17208ad0f8c0ef7cb035aa714a6873df5efdeffaf799488d7fc0b400de46429ac967f71cdb0dc96896c1274927c3773196d74cf5adb4276292b1bf22ffaf963c8c89acaecdfd36a79d8def6baf9e52990280d0815f705bb349f1f0df439f255e9a3747048836ee458117f3efd61ff691de180e5c1eeefe32e6a74906f8214f61e62b8b365c60113843829f57502e5d9d4bb58f1bc5a38398080aa281161de48601e9bc09ad146e7a78c6638ae9552fefb16ec4d12454c9ced56689ace6e43a6c0b43abf31c1dce332182c923436ee6b0c035fba2cacf7d5c0e37af35bac87ba549e6b81712782cb709be1364e0f82f643134319547b4205501e3467726e002d7de9be9dc05a54ce2e2936b121772cb7ed51c083f30819a5f7c59c15f88cea1ff5b34a06a704c83716e8fbc0b92cdd42805d62a720cfdafb2e6815a1ff7bc17ba1d55e72c0bbd3cc745746f1c225ea721321adee3fcfc2bad6cd8a46fc8ea30f79ee001975c29ddc1e92c6cdf75d5bd8f2b3ab230eebbb53a78e20731dbbd8de03ebd1606633a5ca9b2edaccc7c50e2380891a57de87d9732ff4068526cf60ae372251bfd5bccd6807e32c6b6cb2df15c5cf94a2a503848ee509d865769470f050973716c7d65a9c771550b9347fa909444d7eb0aa6dd7126a1e3c3d2f1b65c64a0985910c1d83dc13c7e52985df9012cfdab3e6c573c";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 472;
	dataLen = 4096;
	combinedKey = "8b125b26d59f1d53d1bfa87d603ba3c282a0f369f148d2078d6716699b7b1bbd";
	iv = "7adb0546de1b558bcbd886c8ddcc1b10";
	cipherText = "a12736f19531f5b80801484f1f781d742c5f09d515f8473de84264670bdbf80fbc13209e44f7aa72cadaae41262a9f13b10b03f4db0639c0065d35152087f2494e3ca9fcdcfa0252f489ba17293f29fc495db9a3fe745217660a136cce17dcfceb542d1b4356a3728a383aec8d7536735b6102f84890979785da0d8d7ae1044f94a0fbb441bbf55faa424a40ef2613aaab233a5d5f7167073da9b92c5cdce2e0ee5834d483ffe8b32010f0feda154aad49c3cbfa8d214271196482f1a7be995dd2647e2926401cc2d7ec9740d026602cee595ed51c81dd91f8067daa1cba350b82430376f13fa1857b016565d1f788f2636c2bd7d6fcc685b2ffdbe16510b9a072cb5b48c7abdb87f7337d97f4f6849c16a72a28044a596bf2ea934483a40c1d7ecc3820f7a98af34f633b1c65e88f515bf7ecf6183117a19e5049ae1c8e25b82989e816f38b0b5c2288b87c0d97e36aef16f425b257105f641b45f670b8cb8ea6cd5f2b5301be7ccf4aba50f55534126360c9d2fec5b62346f94951e4168cf3816f87819d5826fba91a0bb6f8ff0b9e2165ba31b11451fd1bc04689c339de1c75e26b35fe3de2388ae2b80c603bde516eba9166f5b32bf989e22e7bc72a34410890ae2fd9caadcad5fd7c02a25f7821f4fc672efc5c27070991302ef67188ca624abdcea1767521ea3ba65ff2081b065977132974caa9bfb17d71ee1805b3e2";
	plainText = "7ba9a79ad3e44f063b981fbd4ab925d7a4acab8454a0435a4e9c19480a0aed9fb5bc3919269b51256a836d9b7578ad15b7b67331f969c0e87c2b7c4df6fc8521a7116408a329d9aa51a52ce1faf94939081d971f69c79e60cfc1fe9c34bf1838dcbd27886a0390861eefea77b318463b90ac780ba1cda29cdcb2a3ef5dcb56b6332998df96edf3c79a2259469a4c922adb6f2c3b0ea143981424de120e747a33a7f1a147bd5d594bcf373859b28fcff42a45c61d9a3030127813cef8f1be3a16137259af771af9d2694f47a4cbe2691911bf615fc1671687b4b5eba18e5dd140cf8ce900a4640e908528e8d4a2c80509214afb0262de6dc9691cba9003628403fecdf0f6d12f6d81b6ac536b97e7482c5b9f1344823593cd5cb0d8cc97802ef3ede80601742eb92a639931d59a1bb4539434186e3ac221b7bb5d18fd9a89e2d8aba555b271b6373f584990850d7ceca29f99553b96143c46a5e80ed5af09d33c36387f02f87276e6e904de69c9144ac21f4ea65327b8ef30f81df053411f6de1611fa6e8db4215673937943ff5cf3fb107c9461a4d619f4d0c7eb79caab46cb4c0a147a1bdcca521a8365beb17af495c7ca00d7272406e473a3bcd839b13c77bc2d422c1353a60ab7554b82c4f029d152bc76ecfc874f19d064391ff77bc6d61d6fdb4751125c416bb1dc65bc1acc36064a5e791108256d76335efea48bd2cdd";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 473;
	dataLen = 4096;
	combinedKey = "45a3877fc117179cec5e3a11d3ac2764a2b1db2d3cb95ec8460cdfb77c53db11";
	iv = "f6b716337a9710b5221a461418d5a145";
	cipherText = "d00d7234ba80530b08472cb172f47eecb5e93e2b1428b059f7cd5b58eb4d96ddc06f7f829f0b2a9ceb2c5fb526d149b44917e302da311ea3232af7178deb678ad29753fb9ca066fb0544dfee37ab144e517a2045a993618e157b456d6b7923373da4faf10a437c541ff224d038197f341bde7821d022f5716d5668090c60b8a4ddc557afe3dbed8361b7c09069edf1e3ef4ac7cd3f5dfaa0c591f4a37388937d047e7f9d0862b2fadd402f7d684baac3e079c47579ba228ccae520f00077fb20e59d27f981c6c570276085d8952502631567f36ead0636ac4e241ea8588ea24ee54d46fec5399e834eab0363dbb31ce558870ad6e074a187a09e68a33c2b6891fdabc5305327e10edaa94de686fb9244fefc790d80fce2bf0eec4dc56f8b3394259ffaacc86bcbda8a4d3860dabf73d5e238f7ef2b2b5c994136db5197f24c318a3a4528307c45571e05506d56add9b109ae6acffa270e46c857e406470515df2127917bc5382a05e66aa4114e60879806c296a50addb26b06d1a35c10603d42a6e688a4639acfcf6c970e4dfd73aa24de08519c2c090d23db4f93023adce3df5c0539f84111c47821038191fd31e4a880dfb83da709b16fe1a07e0ddaa19944caf940dd609bf605426722260315a6c130b7ba254d3bc8b442ab7cf21a4419e124862f4eef0d7f8a6b2f2f3c004f48dda44ff9b5b081db00244db04b8ca9a16a";
	plainText = "9f7e8dcadea6e35728d2dba284edd35b505eeb61ed855f471cb464a5b0eb33b4e85d20043125ad535f698d72ba35b58265f099635fc4e0618f663ed28d7fe15d14a9a00894a26d972b81050aaf7f9f37074695bff003445af29a86a7426c3f1e4b6ed12601e9d151da6900be0f9c2d67cc88e99be065f449d588f13e5ec0feadad5cf0ee8b74c715850fb06e0d87b7897167b93d355812a6f61a743b336a9c8f2d7902b6f6a26e502cd21796700291c0216dc3771d69009dd6b76557a0b596fca8dceb0a7fe1c1e693b60b7a7aaff13a1fedf7846cda87031c2f245c07a528ea8223c1a1b67b1cdcf7f7664873769f5a18827a3688adf809595d637bcd8461e643ef2d76d4d83b18bbfa626ab795bedcb8c2bd1f2d75901928c105bd0c2ec6e7697cb034b2bc9869257857c7b5aa7ea31f9cdac3d4302776a69853fb9cba3e74471ebb2e208357ce634b0055d2eba3ffb4a9f38ef705b0063bf473a7ca787a1590dd06ac092eaab253683245fb88d58a2b40774a814eefa249179db97300b178a2b9a364804702e4d7599d38aa1735e8cb629e1d38a59b8087e9b19a70e70afc846f866670842f3de8c29f8b4ca8c5e8b28b308e0e1cc29137539278031d2d3927e0d529e4e22f100a260f23e2493dc874ee2c40d21a533c68f756f063aa46c0534cc136cbf2527bc278665bca3217cda1b39b20a151e5e70ba89abf404e5130";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 474;
	dataLen = 4096;
	combinedKey = "13d9e9a1180a2cb6df0f4a8fe8dccb636070acef9472a95e0198039afa995ad4";
	iv = "b3e841cb9a9c49e097bb0d83eb92692c";
	cipherText = "38e3997c3fc04230f55c195cddeda3c261169b7843d199fc503d3354c5acfc7ec39fe2b262fdc3dabc52b87c58bc62cae495973c555dd26b76000bccdde3a58812d334bbef102bb7bffb4d7d2221f835af4a76001727af021fdceebe02975f9eeb7ab8410546ef0db0eb19daef66e5199499b0e1f3173bf590926ec8223669b028839662ac12877211511a356539188fa433677ac646da87dd43aab3398143b19af66b5a6142796be4af96aaa4c04357aee8870a43377ffa0cf7142fa6dfff17912594b43dced7e487994012c6ee1b1816c54c7f3a2e05c5bf799c59724161d05a86ca7f1525407fc5216ebc26a7c2e3303c13f45e7369726cc62298dc773abc51eb8ffb4f42a2a8f3beb21eacdf8b2b00ee583587b6ef53563ebc382250c0c7a62921c150af3282cd842ee224580473e8a1b5d9e2b152556a34498f5411cd24caf5b3d9a2270f4b41ca257e19f813bfd08a48a50f56fb5de6d3a36f476af2db7fa5531b69033eaefc9396ed13d0c9b731258c81a93943a6636f9315f871446bc18a17aa5accc74e7f949cf3d85326c386335836367e5c4ad074f740970754639dfac3d9272501f48bda62147d45b548f001c06726037588e73aacf3d983374c0e8a75acc31a3ee5173500e8e63ac6ffc2d5b88c53b73192e2736e034961d7fe48bde59e9de8bb66fd95dabff0945202ee2924c7edceba64376ad5996cd5a58b";
	plainText = "682a58a5f5409142424f3defda73834d8b57cd15a60703eaa732f457cdbc17cff42bc50251db5f08511a7d9596e17a1fbda0204c723615dfbff2256f6961995e45e5361113c007c4252afe76226db68c7e8240164074c578966a81c92fcef1390cd5dbc181c33fa6368575c852cbb45274a3d6a2f302ee694cd95eddbe9ac7523c8abe641c5ca1633d159341f3af567ff9b378c63339e7a7eb547887b1c5a24e53d5f13fdc4bd504da059ce321bb65b5d223a91f04684c6bf28e2ff9140b915c92844efd9515515e610867db06262d7060d82663415c5974c3fa30aa9d064e7a1c85ba1c89ab44fd0c35f16efa331e0b416d24a35d1b8e156b1ea1c687bb214db908f698454470dacd07b1d5059b514e1cb84e617405a7892db164046e557130de3a8dc630f86a4cb0b847b528f4263f9fde3e7e3979ec990ea609097ffe587be79882ebcf8d55c2343242a10097f8c77de0a2ffa5d22ac710c3fd8c381fa409a8e478602b369a43d95caf11371091108a12f5175d297dede3f88003cbd3763665221849f55da907db43ff06c1d13159be4503b1fea111531897103b593864fc92a789582ae4f918f85356801eeb813e5f3567775bba4ee2763a32f7bc9c09ddbd98557dd40234438ac09e66060ba49fd0559cfd7542769ff928aa6f585b11d4afbe599aa0ae58cc06ddf32aae4aebe27c830a28d2f18c28ea14bb55b1a88657";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 475;
	dataLen = 4096;
	combinedKey = "40a384e06fbaea08b54c264fd7e5fadf64ef615dc4ddb4affa67d65c17c586bd";
	iv = "f1ac80934bcf35f8ca7e5a21d1a4e7e8";
	cipherText = "c56c0a66c9c2bd986fa1801626aefeedcdb9091920576c54dcb5749946a1108232491f860246831b7f09a886c18a536d3307d753f8926eb3520c7d37becce6e965ddc708f86024bd63e366417a04e78f295c45d838b4561de0eea7c1c1c568352a81fc8d9f8679b1bccb900f161bf39946124e35dfe3b5df0e614c9399e43f39e8e5501d09512830ea1a471a9f726a5df41e32b964d85ed8247b2c3e4bb19ea93cf38f9831959af9912a3459e01c32f0da946a8e79ed881a085876b3accbb9ce4b0eb58be66520d33631b0a9cfd1c75939b1f67dcbe3b53959046d66f425677dbd36a4dcded4a7fbfad8502bcf98cf014fadecfbd75052d61ba3ab19a16e6251440c2a25c81058950cb04aa123b855cd8db0e2c7591036930a0c5763864430b295d257b3287be68e007ed5c465571ab20808432006e35ff714252833f45be99df99dcdc4b7de5266f2193cee88b390b1e9674e93c921448a09c1be511c4c7c391ab1dffeb9bb58360d087be136734f3d7c13396f78486d270e12bba06f5d832f109893e5683703df487b4f9b64f566fcbe8550ffc459fad6051cc9c5ed03c587ac1f27fbf1743b17768d439adba302f25a5e54251e58144000bc3f0a5b44df0fd4b061a2e069a5dc426fc91fef5c5c52d1b9e2c2c741ad4211129fb40284de16b195c60734445cf418bdf0b0b3938e1147df219e0a27e6b4b9049f0361a1eb8f";
	plainText = "491777b00eb3873127f83ba784fde004558e42e39508b9143263a086377ee1efdb5e77216606b70cb67c4d50fdd22e4171853a817290238ec1e1fce71e1dbac9c72f15718042cbd3cb79ae15a660d218cc45669c9879c1846914c81a288c0e2cbe8698c8c6895a0f248cc8b3e0b24c78655a3dbc1cb2aaaa9a96570a7b9a0b229ea306d314506318a7ac89dfc6cf97961cb0ee9358a1bce3b65050234b60982ef6d66e03e9489575e8ce251e38191e130c92236d5b973f71a1ca8fd7633817a1a5da294e766219f601ee39f65788f3b9869a39ba79c88a2e33695fbdb36f77e8e917c0809e4dfca9e36db820434afbef1b6e37a63307f98382d6b9ff17c7fe2f61e5c3b12472e83849689f58d4f00a4615ddaf448bab7fd3f6e7537aefe85356a3a7329e2bec500f34bf8af62d95e70f3550c85f539f56f7efe7961ec11919842f10f6e961b07672baaafb828048c71f812ff125551ba221952ae2eb0c0e04cf32b18d2145e7ff2eb1f4165e248d555bc3b990de4c633f79174ac41b6712d9e856894befc27ce35b286b05325b005bb0e9f4415ebae3837d60590da26efa3137973e21beb2bb460462398d7d4d5bef79afd9623d88478ce3b35955e429ca625893913adf7d545ccaadda7b300f48dbf930bee1680caddd7ecf402de04ba168dafcc93b60e3cf4ead1486b5787262574a18547ef75e9c2356886bfd515930e4a6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 476;
	dataLen = 4096;
	combinedKey = "ff195c2bae8d8e8637a3e8160b0ce33006b3c0822a2f1b4b149c500159e24790";
	iv = "9c8f141474e6de023f5946e3087b412e";
	cipherText = "f87b2593b1b4fab18b9359bc544f4883b75688e6477cb40e783c284f9110587f476d5e5ef70a74b0e9086a50adac21449833e0e4d11109c8d3e7c123733321a0f01345635ebcdd9cc4bae80c6210fb672842ebd142ee9a066e59446db5d635d4a3d4b455e11a20634ccb13f3b4e729b82fdeb61e366db1fa68c47f93334eac3d0a9393f0b7831f539d8120287ba663883ae0c2fe82d169f19ac55063d9af0d5c9dc22463dca39235b76fc2dad86e66a386a50b35864ad80f9672b431386e703810bfc7b074e827137de82a9267e5166a686d9e20e808bd2654185342c24ea6916db3179906f5b16e5b859eab9a074b193b2f87261c9b5ed4cb88c9082a54f4b7e136276336532a563ae9cf6827359513f7b907de968ccad6459da21f422b88e3d61dc2e70723e9fb340f797727f74a1087e293640fc3c0db636d443d04a331a945a8d6419cc0a133e832d5d0ba60f02e98716b678d83c0fdaba4518922cb0ad5d650b1c776fb846618e62478be2be4f6968d6802426d460a4d7fab19db21ad26eccda09610c8ea0530721a58eb54b6944f9ba3afec3db0caf931199829a31b2d94fae1225829a77fd838691ba60aa2030874374ef56f21376ff27288c07c14e85405c9d3b9d8da97c58c5aa30c0e591509e21ad82d7d294a248e197d68943e4373a2acf92edca8d91b986a6ef2a16c06071f9e81c77f5b2e01e092cccc3dfb54";
	plainText = "0e5c0d5caf23f42965415b87c7a2789c920050919d64ee431b6a833974210774a3e58c12e29e74573978826bcc685168924beffe75a6aaf036f689fbe90b9abb7deed64b8bd1cdf52840b9a905e21e61bb82110e40a72575f71a5013848af9e96827502022e49952cdde1f9e445fdbddf96828890ad60dd8ff1a2fa2b996c6781330ece831c3aed94bea04fdc996d9436d240a05d05a14b6dadbc1f5778308ad59594024c62612f5d3d4df41d8abb38cd64fc880bc17bc05bec06cd8d1485d7103ed226c296b5fe435f869b4c435edc42fde41fc52d8ed84ac147932e0b6c47a938c85ad1b2a4c1d058388b517e5202f19aa76f921b4a4b5853db3ee613f5928f307136a5fc6ac8e0e7d88f19a3beae0d799a800d2fbdc77ea629a4d61dde91e4691a7720afa8111fe8e460e5f5163ecaa9e4f7804a3d3a72604efd5e220433c0d7f13291f697fff155561c94963a138048f208cbd85b4c89a0e59d8e6ca58a8ee34b6cc99c2f806cdd38e1ec1170779302ba3dca81cbc791f5d9aa4b911cde0fe8cfb2acbaaced41579618ee00bdbbc7898b1405614da5eeccbe497485987c10252e818e8985c3dd4978e3a247e327fb4829d4afc50f69682b404313b221df7e76ae23f45e9c0a4ee31001cfe4ff15d62ddfb880cd171a2e57df69453de30d8b5f258b42b57c10d7e442127c99d29e7898ba0bf318dac6f5e2546b561089bb6";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 477;
	dataLen = 4096;
	combinedKey = "0d5641fdabc0697bbbfbef488ab018445bf92cd20cb933258725e6909c69c6fb";
	iv = "9ebc0f75cbf4971a18f37e69ce8ac5cc";
	cipherText = "21334520d2bd9d333d04925930a0f9b5e2da27de3c9247e622470b32265d09379da5fa3ae83aea8452faad718cdb1bd85e5edca48d211e845d3ebd5adba6bc0e4276baa2c1fa78b59c65ee67b06617c633aa2c8cac8298a2b17b2d708b3f86a80e39c04f6d4ec5664482c58ed0b3ff401f43fb48c7e98a443bbc0482a53980bc32468ff98669f182815d06bc8ed7df2a97b077279f7f6bb10d3e72df016ef05dced7d0f0b11418784b1ae4d835ea1916e58a193d5aabf57a984451bd37c9e6e7ca8fbf6204d93bb9a89275b043aae70bd93c4a74260522ab6d59b395db23b69c0a728db1ee8a174ea43d7bb80abae7a3ed617a5b6ed1cb8b334ff1152889834fcd653908139395012883b8a10137d45c844d6c9f4523ab200cc45518c64098cc99875256f7a03ecb74166ba200a1071e02d2c3ce050a090991542b9a3deea3381db4369c118558dc86c5c526e7720a23cec4d6402df1050cbf6e7e0b4b444b1aff8676fa394d151936c38d8d86a76367b1034c71fd07d12253160d519b38fbc13f2b0bf8b6381ff984bad21c72d27501d02477a81304c2c3f3f0c6fc73f73ec34276fa847604ddaaf7b01655131f95dce4f9bfb4b24e7939828519e55021155809972c9437065118677011e49572e1f1a4402583726e9740963e72fdcf45037024df1582d250f992bb3fa127605a1abded0c34e21a0cebca2302991e11eb872e";
	plainText = "b4243d3d3fb60b62e42f127a3dfd811e5767ab22635cc39ab6d7138112c2c5a9beea5e8637f643b71ef11d8eaf717befd6365ba578495fd13c90075dc23d2bfcfcc3169d1819826a5c2cc3145a0d022347ea4ffd4db77a08dfc4ab8f78e34d33dc1ea02088710130f9677b8eb0f641a62b467069dea3f13034f42c5953cc4c6295c9d782575c0216beb5520f1c01d45bd9ae4896d6e17b080e8d36fa481734911abf7c4437eb6375919e05004b424a4b1c09c095f354e153b4d7556a181e6d599d5c57436bd791ecac85670e56ce9226d083b40b72a075db126468d0dc99d147435d597c83516bab2ef9c8dfead61f739203d5f82381b006ac1a58799bb0632add39aaa9cd2b43f95662480e359d633b995cfeedbae72d5d1b15e88d02c325a7d81dd2b1cebbe9cc286f22d0c7fb4bda00c9c3cff54da1f5732e85253747191664d20392928bdd2606decae4330a11c7cbdd1aeade95b181a9e700692d1262aabcd65e15047bffa58223616a7284c63a983386e5bb1babbb31900cdec39d213dfa7f177cea3b315617f2662c5d8297d78471b6e5960790b0b258888be3a5a48f6a7fadb7da4f8af112d0c09c39a340b2645a3bda9e94227e59905376f257caec812dbcccf6f3a395f857fd85418ede14656a133e5006dd328142a6370d4e060fc935c85e0fee81e911d24bf97b67e2c4b77cebcfae9076fdd0df0359bc3a0649";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 478;
	dataLen = 4096;
	combinedKey = "c505dadcdfa43719ab4cf5cc162546855fe9dfc269c1a5d2311307f678543ea6";
	iv = "ba2f13f6f342edfcdd4a071f21a3049a";
	cipherText = "23fe4abd9dc89a462c45ad57c17099fcea6959aff59542a969f9a17df45130c3298f33dfc322922c4c96298efe92f7a84b6b97f31c32da47569873518c99db97a67adf4396ec4e1c75ee904d8c4df9ea20371a16c2accd1064ded9472995565998e3e7dfa0cdf4742f35a8adbf02951fb28add16a0dae8149d730483650abe5bdc2f8f91b44356471f5a074cd80e104a4f0044e0fa221c0e53e2758f20432c8349502362b78781235058115ff96db62c3700b13bf3e0c3c6bce94616800312da108640560d03cf03113c090405bcb56b2f27395e517c31aa0932b0b402d4ed280a8d98369f661e69d766318a605bffca611eebbad0e2a02e9d5010fac523c0c78da0fd4aa8e9001d4ea7981b1ae9b3ca86ba87ded1624b4ff43c4ceaafc4d324029dc26416a3967dc135e1ac9eeec4e9c57e9d4230e61491f5f3d57a5285a68908958a9a578705c0b6b5242d41ed1b14883b7a22319054a01f40340ab5d952b74103d09424232940ad4417cbabb26aa5349eabea5797eab86a63ebdd83069c64df473ffbb72b606307fdbff5d4d6716c4124708d19882e0a18ede3bcb0ca6426933051ee00bb3e29b03e0f1ba25b38807b27c17c4d909c4d947d9da19bb3c2be7aae1b05ab1fbc7be824dcea2fa274e625fe016d09960c02082fa80b8cd4d1ea0c9380f08360d34018437cff0de5cc71386d9856884af876ae6dbde0443807b6";
	plainText = "cefb698cf504fba683ab8d128049ce86f82ad47ba0eadc76c7a0c08ffcd070a7048fe2f6361701ab2206cfd9c5cb725a2c1541ad9c5d630fd2e16853bf29587041c8ae634ac5dc3fce12114cad9178e99c07ccd283aab319812ab388bb596956e246a4235ced3f38626f554a5c3bda1dda940a45a4c7e615539f9afc82b5ffc34935d80d0d3b55f4282bd53fec00ecba079a394d711232af31dce09112f374f850db55c0889a74b2c0b74c0a907d6be7747031d960882b0d31767677497a29ff096754baab6c4ed7d7b1c850d0d6d43e1b0ba0ab4496052ce252af20f68698e54c960b99a9139d2e71273f3892d799cb8224520b1624440484eaf30bf1fdb7467fe3e20bda42a8f5cca6671fa67436cb75da43afa256bbdac85f8d8b30b166b324d520b66a1b77b55e76ffccfc66ba69b13f28de5642163c426af5ee0b5e1616d1038c777683749ef81798932dba4cf1313368c1fff642a032172726b58c9fa35ab8d47f09a577ff3702cab13b68fcc23e5eca91dd8222da04c944a63edde3e901c9ca186468f2234a87f22033ef92b66cf5025b27903188c5e5bdfc24c6cc890fe4b1669aa961d6a283a6fc24109be074f4582eea6fcb8b06b55a140097cf72e3765d75343fac5074702f6cea23b5d4f6e24b9ba964581b79a7890ad3261cf81cffcfa49be7bf4ea01e631e8758515c9a27031621e48623500b12c623196717";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 479;
	dataLen = 4096;
	combinedKey = "a3a579e2abb201b2ccb92102233cc646575d39ee3b21bc76e2649a1528ffd4f2";
	iv = "a86f6ac68a3b1a01020a98e5c5f5ba29";
	cipherText = "6ac15e5d72dac7e52107fe55b89b0ad6edf04722e037e18a9111763198039fceaab035a70d50397c9c3a5867a3a467cedc028182f1a87e89717624bfb598656c47a56f0e3de506fd0f6d9e72a922f5bead036131626a536eaf24dacfc5bece440ce144232edd74a7cbe883afad06959bf63ec8f67fda370d393942a20fbb4cecb5dbb0bc4ba48d225c54f2e26a9d807c827bbb6a5c0de7e1a0d321a274f80ffe527212c170a5c0e083c1d8f08b4fe3908a380d2346ee49a50c7faaa314694028a9eb55f6c52c59ecfe536e3cf0dd2279a374d8036ba3639091f9a50cbbf654114a65b935c43121ae3c90c078f95b64220a655b9e41307de4804b67c335c37335adb402f1621b220353b9cf4776b1089f0a164e7d75fa9b345edf147c1c9a8ef70a0516a7c012be6c1c1acb40d9b8f4c470f95acc11fc3883f3bd756ab07ae6b1587e1dfe2d9647fb25318d53c3b91a412f8989c7035a507c67ff2a5d1c9f1e1a523e77ce6299de2c25549bd75bb542e84e0c16e8356835768e838ea47e936d9570116e30e564d6d2be46b9f144540822a1178c381f46307b7fb138e8f59bffd4e719feaea89e08f49c38a874fa491c908597ee5f34f46695664d877481aa1edbfe50249a5140006d7e225e9ceeff567ddde941ea153c7260ab453946e9c173026bb0e0a1048fc872d122777ec38f8ad582cf0f017f2100c7875c2d551b1e1389";
	plainText = "1f76cb1c13667a504e8501f470a7c2219f10cf6ed8152d938331e58de916c16c0ccd8ee07e4276d3ba2bf07bc48272281113736d548fa8a7f89933b6af3f3610d504b7d893738e79a14780e790fddffdb5e7e2587aacec5660f9b212084130331e6f9c46db1c15917a7103dd13849eb3cb3d2156148f34e0269d277769e0ce184ddb1d6cd125f9125689a75d1335dcdb7aad1a480c6fe4d3b1bf55935ed694c340677fd1f3bf0d2196cf536beac68ddbbccb62023a1c94933957886b2b39d6ac176b425667b91fe3e11624ec8d43d793e16353af5ce186f4b9a0c779779c52a7afa987d79100b01c303d61287533899441740bf1f6ec17fd74110fe12534bcf54d90c3480e24613f42a3b449d7961da5310e27770429d2d1ceefdb5943e9946f78782552e7a61b05f35ae794a651a5646036575c31137874719e281d33853ada20e0c73c61114c8d804655ff89948e75b1fc0e4f6e86c72d2876ebe4c3d358daa39dd12777d4193c0060044460ddf2d945495bb3caa717af5d9691de711e26ee9458b99d8c7789ffd2e882a1f77d465b94fca6bc35b3e0380e99b5ec5b0d342a8ea77713785aea5e87139e4cb2310d65aa6be2e1e09983ea0111efceca80a2808f3e081f6b9a4d0a66440b078a899c507e59ce59b6392cbbbddfa208e4058e8e534884a9fa3f9bb83185eab97d2ea9216c8382294885486f331606463d92853a";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 480;
	dataLen = 4096;
	combinedKey = "eb8e10c5ffb43564d4b8acc1e74ec7077267cf9ffcbb0e5b4c93e09e049e2758";
	iv = "094fcb920863afc7bb9f1aaa19d49467";
	cipherText = "d2b35d7dc4013955449c7150cab41846c336080cef9dce4f3a9c635f574f1bb1b6fb1b1e537fa1e7681d1a0c57135f125ed644c693b67be7de8826e9616181945629d6dc779fcdcf7afc728dd96061051777aa2e229f0f9f7725f2d53fbff3a6271927af168949aa4a8a6d465e13ceb74fa3602a70005245f293de9010c79652ef5ad1711827d8639d7dfa9ed41f2431c03db91e944fa92d5ab736326916b2d2919e0b90ff9eed5e66fcd9f5bbd693e7217e49ee9e81a7aced86b6e5d9a0d769bd60cc3e4ebb659a3f7025ee6cc57c72873375d9ba2ee54af8dafbc6eb8efaab3b13ff1c487b465e4abbd77039000adbf183b036efb350dd7decb7392e2888578cc17ef4c3223438d5a5a4d2b796f4a64306a41c3bc4e3f667a20a024c1cd3f0d30bed960577442dbaf7e6e4293b89fd78eccb99769796704fd34fb99b1511158cd147035a1125eb93e1020ff2d2557e843c9cdaf8cf928c4cb3844abd90c6101059d7cec71aa4cf75146f09aade9e93eec41a3bddbef304c128438b5f95c02bbd521a451c81bfee414c9120febf0f77c16a465821205fb648926e2924d407da9b3ad69068818616601d712cd77faa50b0f663dc568228999a029f6d4385cb8a027ad8e827693c72dc38822e3690bb1ae57ee1d3c2fa9d991ae0702ebe7730d7da8791528b055b74c7fa5c50916f57908448486fe475dbd05c78146aad0bfc66";
	plainText = "eef51b48fcc2173c8c6da4c91d8ddb12d40237640b54cd8bb552ef214f8aee47d01a734b8a3294cdc7f00143a85402796a61825912d63325ecb0346add1534ec6929aea32cda0c899ff7983c4a1e60634ec1495409494300b4791307192dcb5da71e84c4e6dbf89b0a7b11fe766a329c103140867df746b6f8bc94d62df79e0425d4907913b38e118d6544a2b104273ed283b72a8bfcadf138da315f66f99d63fd94cc8da7152e7f8b935179d85bae655d34de448b0a1b56c7e9610782cf79521c26e5a48becbfdb37e47ce992262121d27b96af14fab84e7a9c3208a430ec41e72b69163d1c5eb7a23fa4f3c86bdaad8d949cd5e852f1168a81b9d3cfeac0b0224de160431da3ab7718be52ffb9a4baac56f51d4ce154626e4669d18b09000e19cbac2348b444e6a633dd32c925c367d9d45eb79b72aaf21c7abb92b9b734c939da610ff6297f449a5680215eade65ab5f5cd7ce36b13c1b417634c6100676539f50ada854a850a0fd51c26712c89bf3ae836cd954252b64ff996efac67c854cbb335ee5a6644877f688c2d67da6df7076af304fead865ea6ebc02360dc29a493bf4f1114b099f32ee6ca243ddfae4e9fd0402ff3ad1719f57f8ddf970a8388e46686b92f4f4a88c8f556f2e52774995abbd2dd7a3f64f91110db8617e8ff4ac03a21b38fcc30d8dd1dd88a34cbb186b572afc16c6c3c62f40193bbfa44cd35";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 481;
	dataLen = 4096;
	combinedKey = "51f3cbfd0497f914d1c27a0677d12f66844c5b771e341ebb833ceb206aeef877";
	iv = "c4b1ec2d8640ad1d705f65663626009e";
	cipherText = "289ec2a907353736416aebccb6a61612537654b51a3fd97b6726321a980c1c07bd790cad3a7e1dcc6f3150d6587cd96a712936e905faa12b0d83046c092cd2fa435004ea039521813dd08dd158ee65b87a124e1dd4091c7350d4e94036a72a15aebd6bccace14a64eac2a0111f7f99dab08d8b75c20f95d27a933465c23ea0c9d1d567cb82f3eda17603d992419599d666a17923b4ce340f6b67e5ee4993296b8a21a9211c8e5611933c0235b8b9f846881acd5aaf2ccd18ac1e761f6d40680415106aebbcbab9303e296221f8b6753f813544f841b9ae737e696f70ea0aafb457fda1c48cda10567abadccf33a35e2ce524504facb96665aaf119cd46f0218374d0c759af2a185aa8efdb4a5a898dbe639e091e56a01737f54c5e2936e790629e3a2fc5b48b5438020467b5554e2a20dd35550f300594bd5e0f27a6a9aae5c5296d6210efcd27ae43bd0498e1cfdd73da3b8c29d57edae3a04f616bfbe6646815ff0085487d1a2e5667bd850eb959d643129043e78c742b6475c33db7c89990ee5ee9edd53b241146c977d93af7b82ce8f3a2ad26fcc7164cfa83103b0b3db7655962e2df98126248566a0811cc778d36e68e2d967cbf8a1ea6846f978ca96c1ef34863f2172f9ee2c498ecd0011a7ebb1e065c16e16adf05652ee02d46ec5ca1061c4d2d26e120c1946561aae66d5670ebe40804859c5c270d49bc89d0a8e7";
	plainText = "9b1eb6dc214446ecbf1812461025fa9c268932d1d2ba945e766e84a134dcb6ebc794b22a7622d8c1eb374da281279501d06fd6241fba6c52bcc4d8c2eeba2029ce13f5472f362740dd98cae0205346da36db74d729d61cf44365a815d6961d13a95e7ebe1cc779f9cb4ebed806b827d43149a1953ca66b3a3a86ff1d9d5e8741bebcb6993a285f14d6b14c713895a2f3fde60ad09f2e8eb58dd6638e6ca9508da39078635a578be35d92873b271dd3e5014667644f29db6c5c7caad4ff1aef5068f2d2f4e0da5891747b16c87a8721c448c2d36c9d180e9927b5cbb9b915fe77b7c0ee68a9bc1c1f1f2746e803ff0f2ba17249fd2ad3cf7634608f8276f206e1c94b76482b14e4686254c1cc2ac65364675912767fda6e13e36bfd05d959173c6ad59595a3fb6d6111f024d251a820097a3cd39ba2fb391b55797c053e43def451eca7b32a0e94010a1b1e14ea615450bf0551ec6dcea44326a072daad266912bf86577269c6f5272d06c615e47d0d842bdd565838976ccada59253f6d8d23dbb0be2bea9a993ff169d6c8ddc7372d2bc120edad68aab0471d9d3404bf337fefc343744ba6428d7e85c23ef800d28a963ac0853375247c397c5f36a186f838c4c2993337cc139eb74541520b41c7b693806afcd5c4f7ddffa4327c6ef400dfddc42258ea1ea6e698a8eee51c0859a6be36a44637de0211b31a0faeb2326ecb73";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 482;
	dataLen = 4096;
	combinedKey = "e3ed136ba58349ddb3279169a3bcc795c26656aac5d87b33dd24316c1da12b95";
	iv = "51d0e4e4c424a15eb58d61668534516b";
	cipherText = "33ff298e8267932b3778b692fdaebaaf50deb33d610ce23a7af6f7f6c3b7b149e535b5064eef4851cd8723eb7c1e56eea7c7d22e3367756ca2fbf5d595c1c0922bfa4553f9c12a16de73857e74a8cfcfdbb1e3fd4628df3cbfc9b26b4ecc839e20164a6159bf8cddd6c222ce3f90901796ff33ae1536f62fcb26641b6419b8c51e8f098a59731cab8f0a4d366a708a0a5381790a45fd7a432150474976e8f9f2fecd6270c0177ee8007c2e8dc35f8ff0bf7a5315f8691c3394d26dc683b080cab113fb5ea60a8b74c179183e38e991a0b3ec3522d15255a38bd6d786e85444d242d0e684ce517a365f3ebfb4de6c2caca3455cf96407cbcbc755f733ff45d90d43ce3ff07c3a3a1bdb58fe911d2e571c27bb9967d44f9c6e68bc968b2c929b88316c4d14d9c44369d27730b3d3cfdfe519b0074ce66a1ed023ee6f0ad0125472710473dd23b2a971550a88879ea33ae8c691da5e7200873a1b79f2a91848b480fa7a19094ada12505c541ae4bca48c3f6fae6c09aaf60c2a95f82117944413c9fb1f6d0b2711170d266323dad6be63b67a88404e123346854c2df10ae4d6b55fbc0395a55254cb8133fddeb3b3ba6d2a2976dca2aeb5b6b2b56ee46dc1f529923b960b559947dd8012475f7be6476ccd6cf33ec578c861791e25bf8a4619168da8f8d772378ef47ee27d33e04b305d08f9f5c5a0c547a94a3c749031ebd61aa5";
	plainText = "f5b324240c150063002d4041a5d6dabc5a2c8d86e82ce9bc20f99c654a10bd87a1e559a7007adfed095ec1b701ec61192de1ef83f3aa7576923a706e677aaa061d2ef43d2da2fa4271a6238f95d809d73ef75cffd394f34604d7a7b8d284b19623c87ff13c4e82795e2b625b7b60ea89b00478964daefeb8eec2ea80c4b903c84f99cebdd2a1e616f7452d390fdc75b2c63de74138e81523b67cd157da7174eace6dc7d28ec967f4fa786bc8b466f13c9d6ec4d69c504147b33e7be48b0b65001fc7cb9f17a1414ebc16ad0780263b0a9f2e1f2a041a75c3982f129993ef670833d8cc2096e364278328925ed5a6ce9a2bd64bd6fd9165496d587e9de8e3437108e23bd238a24378927ba9f308114d469e57882a1f90f3645ec2eb4abf54817a980829e1a0708a806d8f1068024ce22c1d792e37fcdc8bd8a6b8e04634be337f240f1068c0a15538fffd07bf9cec0b63a92ae92a8e166cc130d2b78a4b1cb4102e3010675e5c9dae3a6c2c5de90bbbfdce822165082d6e0bf2261232c5116b9fb32e88d2fd518fab9cdc727fb9c3383670da29e7897124128d5c6e57d8398e16fa8c0c8a78d79c2199b8f587f7151f2991581d8d685bfbfbd3d19c0f48a90dfaaacc4cfbd7c52cfe55ddef2bc209c00354c77b7e445c08f05a518bdd54b1b43e4771a20e8af3ce72a9876ce2e33c8baaff5557ee465a6ea18321f13a0afad951";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 483;
	dataLen = 4096;
	combinedKey = "f13c1c9eba8ef928f46aa3f70dcc2baa0e6b20bd003b1a38761765e18414a307";
	iv = "6c44bf958ff9b513e318b68b5c7dadd6";
	cipherText = "3183d1a95602ab4b196b50efb81907308a986a10026ababb7bf75608d913ff0163d08cb0eb8c55fee4d8591bd6256c2a3bb2fd99ae0a3dd6e0aca3e19d7264625c7db860e42e0221990170a91d9b01880416e7aa1aae89d3b77a5fa5e81e810118a4c9cf13c4f40a9e1c7ba0f1e2e0ca76e0553a5e5fa01db70d82225b7bddca8285a9e03c315b432451eec23220c3f4fbcd3a2623da502b3a98d3e1ea5a9f60ddcf98104a7ad46679b9cbc51555d4bc4083f0147285ba6e0f56050a68b47beb1a1704451655d2732bc5c7c1a0805488b4e4c1279b2cb122ed3c57abefe382f17095992ac280f766fb3380bdd15f7c66374cbafc739661626f66c572ea63a7256fb8e1a28da2ab9d656250012157ce3fcfb7db741542383d822a151550c9f7ea04e8ab6b6c3bd9abfa14712dfef8cbc8da0e5f4ceb4856ce08d33cce313668a2b791fed1d5af91d4326753df668e88f5c2448213c8b41f39749739c1cbd47ce7a74e93ae4e8fd1a02a80903f65e6c6ecc401466038f4566085f023076d7fb628bd4946b8c28522a26d42c692ad1d2d45495baca6d9a3f35b857d4aac865cb1b229d888a18bc389a5ffa5eab8b44ff930a44c920ded84277459140f6662801ba4326f90461a0241ebb7d9bfbc3cb42bb23b71632a131046d9d3af5e6251c5dbf89856f59017e043124ce9221b9ec59c1547e2972b7e030769f8dd56326a7b5f35";
	plainText = "6700c7a1b99c1c7d33b773187e3be71df6eed7bf2c5c971a937285ae06acde4647f27a7a839b6ef7bd6f0e12cc612373eeed6f3c1cb5262386573b3e179b5d3d15b037dce50b71e52cd02e89158852d39fc9b14eebb96cacc4a2874972d56736f66cbfd2fa3e1a9e6a98eea4c540a093ad318f6b9d083a0a504c5a57768ba560d6a88ece4c94a30162351df40d6eab7837f01025986871e8806f177567a539c8a3f6e17953bd184aa8e1b139105e997c1fae3a1963f2af76a66119262256b6d2bd7186b1c6b2985c28a9708623daf0dd40fa24b832c6ed5262927f9dd728c38f8420413293bd7c712eed4b026c7cca73808d722bd786ba059abc382ce8ab93c4b4c9a2ef70888a1bf54921a24ab75d0af9d8d94440ed95d9ace4a079d14938c3d121021992f44446afb35c412ab93a578605e0ed00bd2d866e78c6183d3b955fe76f3ae68f508c6e31627e1f58c0f6ffbf7a13868f9c1c742fb0998c0acb513b3f0c4946c062a82acf03f227c2d6f43233806d0ae6cb828b28701dabf6d77a28b9a6076f3772753cfbd12dae5f6fd42c7485913284f698ceb5759d424234fa2834f29ba1feef0dbc2bfaab4eb71620334c5e4cb880715de5b52068de6ad97a652d367d40dc3d002020ef6ed275056c7d0904c6bd2588e730a030cbc0112090bf9b8f7eb5b411c7fcd3c10958b336ed910dc6999f15b183729333f3f50e56efa4";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 484;
	dataLen = 4096;
	combinedKey = "7fb0117b5886c1b927aeb07b6c15ce3ece5f52b064194ba6841eccaecc61a4ac";
	iv = "f001bb080d3109fe7a44719ab4f9bb54";
	cipherText = "81319a2d6c8864ea5fe82e5e362c594a5b399b90ba566aa957f5c73542bb3791ba25f919accfa12857d1f9dae4054f5ea27eb5e799412fa38cf3d71db97d5137887c1cb43c918870d06a4aaadf2f18f85d58049e2bd8fa800247dcd79fe54d773dff703f2866d6407b176519032f5967816ff1bc361cc1db1ff9926b8f92bf836c290b9883138aff20477e6a938be3248ca6d3291aeec01d6029b40398135c85dcd8ec08f192d060820e0a01c31498454f33a5d4ffcd234f9d485c33c19ad2a278d22ba6555b1b204675d65085e26216df05773ac2947352c25e23f36485ddf40baae37c66d8a7ccbaeb9667f4816af2ad00a3bfa20bf8b682aa46ca15d55f3d09cfdc1f26379b2bac199b83aba8b40693291dea0d9dffab41e4f3b2ee17777b1a7a4bcd16bbc5e86a56dc726e603e422bb7d2cfcdc71dca66d071fb4be5f9b628272dd73d593ecc2e8fe353aa6b6ca3981417ad3eafca2a2459bd39de7878439ea0a1085135ab6789067ce8bc08993af9ec96e84a2bf51a86c55e8b405f205b7b515680e2b1507a5459833f789ef6a2bdc2974ed6dfb2493c47b85dde343d0d30f4dd0aaa858d260557f7045f37723bec943e7e63945ed14a8ae37eaf4c5d136b77afb9ede5d0be0cd776779cebece4d31034bad00a7b0f3c725313d4ad9b96d259c8145eb9dbfb8b1efdf04db3ba06ca0093e040082a932d7cea9f42d6e49a";
	plainText = "09c80ed6e555e6cb0ce6eaf6738a8bc8d5547665566e1c3604f348fa02c519f45d0755673c58bef3851117b09471f4e1f658b2925c1a9a1870f6d67feaf4f441a888452f3809dd6308f2bc1a4b707c6154905571fdb7cd063b9d3209ab95d0fe82a5af9da5b133bbd145a66b9e0c6bed460296815629aa41a6c4fb3e2ea5b4e94a0108cc0aa4366b44f2853f0c26ed1af9edfbcdfae4a68673a7ffe22dc60110d431daea073357a9809888bd7651611f4ed34e986cca84907d0ee9568be98832bcd51892f3d4edf841a8baea3f08727358c68ff01543bd2d8351bfa348ff552b1d6083dc5773dea0d0f37b8bc579da847715feaeab149abe0c9b65a527cf80be3558c783caf69a44e137972bea026b7324822b1bc818372adc04e9970b6320a27ec123832aab38e32c6e4c9945f0ee280576240b06a75bdfb1d8fe6d5e5c958853a661c9fecfa74265bf8b0cef3b9028c9beafa44fe91cddc99ad8f8bf6c8325275a12de23f29afe5a0ee8f7f81aa997f2f33ae6ffb91e4e2f37b9a0b75c60231fa145256b64611fee8b969853470580d6228d4e9c3fa6440b45de82c2757e2ea0813973511d9df5a04bcbb40e2d2634e8c760415eea6e8f4f0034e484e5b0adb206f785844233c206e21c6fd6b5675d527c22d895451a204a6cf6df7a1971e32e6d6fdfe336b4ef0ff3ffdf8003bdb486c669eb3416d7ee584b92536c29e122";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 485;
	dataLen = 4096;
	combinedKey = "2f04c956015b9e45c1bfb657652a3cdfb04ea9f6c4928a16c6ac86c93d4dc6d3";
	iv = "1ca3f42d69053286ca5b65c55ca15535";
	cipherText = "22df67a1c263487d9d5a609c85edb94ffb8bc0d639bd0b2e6b6c67e05c7a2fb2f5142ce8d30a98de84b624211305c594a66d0a433a21c102a8d5947ac7b8bef7901282e827e12ce7bf4137b2300d3e71a479a9724395e6aeb0089a5134f3908eeec04567ada60e26fac02e576cc0140412823aebcbbdbf8e1f070850930e2c1926402949cf61b8e68803cd1304a5d218ff91c68a08cd2a1c042019e30580fdb4352f1b95c4f731fa84fcb63f151f194c03ddf5366aa206ce71899683ef363507e8c05e8b8e7d1f0c7f1dc233a7ca76d7415d826d41eb624e98583a55c82cd80835421e3a699de92391220ae4abd5985340730ed44ba39582d6b3f6461d986f241e79b4d9108d7981b36f9a54cf43a8228def2253e5fd0aa4639f4fa3052bc8d65d354a8f6b1597c70e267c214d25ce40e8efbea0e774777526ca01dd74d296085208006e68f8cf162a80d1db02fe72fb0b10c8db85466a8982882d5ea5d9bceb9dfeedbd698bbc2243dadf7814f88614b0d6561126181f76cc2b177616c46338180b0e7eee665c1b23fe3a1a2b7a7b12df30e69fbb77644227f2006d77d9b7213c085e2103d5dbdf930f2b6999954b74d2ca0f04bcc74a5fdcdee098cfaa623c3e2b77eef4d08d9547d09fde9898f75777c8dffab565f13114d3527e609d699a9b02de2cd00be7f62f6cf739baa2fe904d12a14fe0bb26713c3c5b38d7a56ff7";
	plainText = "932f9d808282beca376f0d8ecc77c500f35877715a88d132630d44191fda5fc9f0f7d505eb86141a297d092234818ffbb830821cccedb12c75dff0bfc3ec1e21aec78eb4973e37ec0fd387e5436daea2fed3135f5a17cd7a6112b0d6c249e7c78c44124309658564a4fee8e52fdb6c0ce2e708ac408f27d80a224575023f236ac46a881fa7896cbd8ea5546265c4cb4d59e42203dde0b2ccdaa05238e5caf551c910e9072f10e9d098bd5e265c8281be205bfe428687bc459f753e4e45e1e5333d591e068fbb2972f791d99df40e8d4325db609034f2cdb1e522e9bfaf52c2ff26e61b9f832825c570b2b7241a861356628f5d56ea3a99dab7996b9bc361300009de1e6372ceef46a21823d985e42a35c67e45719deaebd0e1e4ad538d8688a0f8df923d44bf37310ed9281e643d377e3345395e998d6d8089a226947b7d300a7d7f1e2c3abb8a91e9bcaedb0e40f9ea975205fd51c40780208e98fbf6c616d3fb714a6a2085f8ac82569973e2cd682ec896680d65ed6db7fa625ebf53bbdd711933607da7091dd556d4bedf71fcbbf3c7d89cdc28a5a0236945d004f2c97c2862fed5a6dba6a8bd05cb68aae72fd6ca25c296e0e851e5c5027664fa5a0369b89bf2127753cbbec672d7a9043355d22d0e9ac929bd27d43eafa5bbf730e3869395620c14ba500e71ab0e0612ef30de7b4cfcfa4b2779f3db83ffee84fd8e12ff";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 486;
	dataLen = 4096;
	combinedKey = "0508ac7b6b68fac12ce71b581519b61b377b4348096df0df876037adb77bd18e";
	iv = "292910480c1708f487b93d8171a930a1";
	cipherText = "d5a072fa6857ea7615cf678b722be7c6360e70e5c546ccdcccc5f5c1961efdf5c4bbfddfa67288c4348aa446882f3bdc121015ea4b02abbaa1894bc0342d53f8cdea86c3b2248e4dea5f5c7ac5ea320c1e4737369781fe49c9d710dccaab2dff07fdaa3f309fa28bac945bd7fd19eab1ce0e36ab1723b0757f8ec306e608dd4d1d5570eabb358fb2c993c0a40919524a420cd167cdd50d203d6e51a419885331d4306ca5c1301dc5426441221474b477b261132a75a5c559544693458e8d13516bc2f4c7515cd9a6e319ac677057a3a68bfaee1203e1118ec321b3761695e6f25debd324b9f13476c1f3182e17698f7dbe46a99c5e7bd411176355072ad423538c8448166d047b4d3ff8de3380ada8be88289949c530cccecfdc4f1d1ce114dd679dfd62e4efe1fa62ea82302ebac63750af284115ebd20843f3dfe4b388fb99d61e32d692461401b521f056c376e54f0c5f3e0d4aa752340438e6047814da9578eea486ef656509b16cfdfc30222c9d343c68763be72b463da4fbd0eb2c4afd73ace50017aecb086de18d28c7a358d4846f6a007d4b4abfda3b013ed7706781487a8c16e4b8e1d8f4df1ca0ab105686c12db624e87ee5d37f7f71cb05601571dd6457df4cdd4388769c19c1adaf3d5b8fe23be1ec7658a4d4602a3cfe7714b71853170433aac007f434cf84abbaea312309486787671a0c11ac7d18284749c6";
	plainText = "d489b08b54acc9a06b708757f07fa490ec72f7273e0f04f8b6258dbeaf38a9082b5b53aaa2b21d18bf102bc860133fa85d290f014c53b9269095c690edf438dc45d952d3220ad71102f08d47ffa9604b4ff334f95a44b2cfbc63a582614ab7d9bc524cf064d251809f59ea2c623cd699b96015c1bf31abe3b7d93e38507f7e62841bfe1f0ba222db3eaa660c44ade3f1f6f6e401d672a80e4d7a3dbf430ca97a30c40d379e83fa190d0539b0a785d83d2cd2f47cff6cdbcc2ee9ea5dd8ba80ad33d045c198775cbe9a8bb584911f92980238d1cd9be71978d615d0dc26771792b5c093b8967f898fcbbed1f81050f64c52c739fc2e4f2d299f14772f20a52bc474c80c245683420743a40488be09bb5bf6986fc271e892c19fb11fdcb8a09a5439777869aa412fae612b5c1f4c259e21937c27248fd6c08aee83cafc5e1337d3eac950ebdc732f31df9dd37274fa6ac6ea3d4102e18c74e5ebfa8c314b7dfbce78b9194e479555069d6561de5a9750d6928caa8b02c82da91b2ad599d0fcf3419b8cebfa9ebfc38e46712b4206d3fb2def2aa16d198d9b410f3bdd396a6e5c9e11a3ebfbe44856331cda8ea70eea09f5a7bb28141887672b18bd76b78ebda2c4750b7d7e431a596a583b8f9cec29ae4de13863e1963bd2e78ab2d0d7ed4e00696466fa45c1bfa9c2d67706949c0ba29b23a66c621582d2959b9f4921d3053478";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 487;
	dataLen = 4096;
	combinedKey = "8098ee94be761a6366dc142135af57de6daa672ab7444b1bb89715387c4c9c9c";
	iv = "e9f79eefb6783fd6990051d21719a0ec";
	cipherText = "a0090a7c79a07e83fcebcb6db1cbdae0d9751d4aec45c97f5a2e4875a8612c1b93c8eac9b3ddf82cc6f035315c6175621a287d42800371b1169069de2ecab80d86f412d9ef0da4a08205a85ee1a6b46091a73618d800bf15f119c4c284943c245572c0500b07e0a6a81ab6b5e1b27189cc9938b48ad2be4359fafddd71d5965d493b0a0308d973952917f9a69741d4928c180d9f64952a4d89c80566d6ec7b68c2fa1202f17726db18f93245cbe4bc4faff81c6e3197a78b8cdb6cf62b7baae724d7842db7a0ea52fbe53a60e49e23904f1071efa6301fefa8676a9c003581b370c724455fc8c2326f149b5abc94f7519e80deabfce3eb09fb0dc3a72050a4c43fd83260914ea75d763e782fe47e797534d736c582d68f0c779f4658b39b95598efc5983da2c00f4307d89576b48157076a58926da78090c19cb3f71027b6d819fe0e4bd1ccda1f045228ba2ebabc158ec55f424ff3a148f6ec2cbca3cad19e25d0cf7f4d0c67d187c66c8906718c5f08fa8b25cda1d27ded45cea50ab0a79ec66bb423c419b4399fb589812fe85e6b711443b1a505cc6a822448077448e4af49dcb270f6e6d4ee3d71f4e4986f606a92eb6294be2bc637909d17e4b0edd078d9ef479b2a610c69365345f28efb12054a0ae0f4424cdadd4df65201239be6249ae0c6f2f15e310c9f2decf36c9048fe45dad0f2f269f0fb066e4a71efe4bdf28";
	plainText = "affde635e843d673326da80912c88b6b2dd9ae7e1d2a75828fa9e801deab13bb87fe0832aedd872db000c494ff94fae0100c2141d91a03755b7c3f381258aae187dae25ecf5dddfbdd9fd31ecb47ce15ecb96f2deee03924481fb32bc75a0addd18a6f5a845cc0196988602867fc963d29adf384da3a3b48e2689f89b993d072f52930d2df2270adf37ec0a6a21094c34f30def0c41ceffda5cfdf3a5eb5f01fbcf819018fa6e9a75ed6c9e94ca93f25a8e1557a39452baa9e6d80a346070871586e73ae75eca8d9db86cc96d0e77277d388269c167df91eee75c80d08e066ba767cd98b369a6567633a206402d32cac42a9ef8b0c87183be6f9b459e981bb81cc9142669fe53bc857a727d58f448888525f7fd553a0586294ca1db89aa27a5d10a0991b17e276c5954f401635f6733a63dd570893885080b86dd0ad568e660e58a9b6438d7e9756934aba6b26b1a9dcb68ca88465f7ff4db51d027c28b9fd10310e5c8d16689839b8929d13b5d0aeacb673b9e5fb3b3a50c85f0b56f8cc3f01d68f2c9ec9aef97cf408ffa657feb3cfbdb3d871116cb3ce11de37c0f64433a9bca726f1423a42b3be201f7e4a5a9a7db376540d09f8b621ef338f06be35b8b8e65f7922c889424094ac28a245a6e5d7155c3c1d2a9d2b93d1b9c51d3bc3a933c125f6b61e7ceca61801b532f5b2155f246d26819dc050a235ace9e37b6bb11d";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 488;
	dataLen = 4096;
	combinedKey = "594958dc5ca1f46c873fe253cd8af04cf77ffae50c9409ededa4249c42f172a1";
	iv = "524f4d4d9986780c4058e851de45340f";
	cipherText = "0e6a6915c30258eec6e9afe94ba020ef7459debb2d79b88430e92074dd63e9607e81218deb2b2863047f1b1802afc776e55d4956f98199e9e07fa53be27744398b684416b7bdc7e90b1d9a2799e79d30c1800539544a87321a2811d005a7fa6026e523d6ce5104e337d79fa4abcb7b7e7ba53d78e2e153bc4bb6116b6059eba1997aa72c6bf426762cdffb3c8a96ec92dabf092d6397998a085b6a89ca7bcb363ee00d3385cbaca814501c0dd997536af09a299be55e21ca1284863d5d0e93161a58576b535f9c7b0ce4022bb7a7bc0b2dd330ed00a46bce0c9a2010d7fcf94bead96372b386f4b45e27ad270f7e2e58d86a08137439c58858c65c4517232c6b56533bdc7751d8b5d068fe5d7671f508f415f8b6c593620c5a56dbcb153aef693b90ac90f4686652c0dc7f1d852c681430186fa12080fa42d456ec4542f254a53b0f3b7a2147ec591f8be9d0b9fa424516e4721c62e7d83835cbe8d5b27c5fda55998b0be1f99084d759f3dfa86534b7a302a70c3879ff937f5f9111dad790d0d069827ff96696cce7c3a75cae37f120830a6ab03877b0ec0f597fc8cf28df7b76445e9d330d1bc7148cac488d9e9e264adfdeb77c8041796faecc630b268da91f16aee33c9f216db1aa4717dfd7ab38194f4872a2d2b6bcbefaef43f791594f91671a8ff92377edb1d4c0d65b181b399daf98263334b35d22e9a7d78219b36d";
	plainText = "5be06f7f89f901bdffa56ed223caa6f585b0ace5d4e65a6eb59b083d785f9a5cf85ff0a8bdab25cf12f65cdf58f8e7e710fbcebad6c94174746b5b103a7ae32a98e71e7f9369719aeb9ac971695306afac8b0596dad678ea3e2c291345272ec7f70d4f91ea630b8a8cd5bbb4cd1e186d0ed86fd93ee6c247f866ad5906684fd9574694c7773e417cb7f433e07bafee988494e610ab3537272e620e573600119b2db60d472d4805ea156d5f4008f02c79a3fca71ab5cfff8444d514a203ba716c77bdddbca62de803c110bb3a44a335ae804284c93511169141ff1514919a6fd2d4768588aadedd5dd0961603123c64a8bbccff9107b668add5b18ab39ae93d3678a8e10075ef93c7d3c965d1192d73bb1a449d4d39d91e127b776bf7139c15b6b64d81932ac0c57ebd16503a9c371d1027dad47e318c5d12f48672411fefb193cf50467de377f1bdc036eb4dc3029a28edfe71c000f178004f046df7b936398350ce52e8418b0ece3dd687078199a4920a3e84f7f5dd1dfb94e3b8b3fa3f5eee076fd277c7f8735e7f223f608c507b07d9b55ed2603cea878abcc9d7e656d88491eb17ef995980c7394c0df8e410c2fa3a91c9874fe873f77e7c852113755a613f6775bb7d573743fafbbda09625616d1b86d6a3c1b999b23336248a10de13b4546c5ef6593dd076a5e83524392d7eef86b24128e857dd206dae92d36646f094";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 489;
	dataLen = 4096;
	combinedKey = "c1e8b9d6aed40b74b5ff3fdec13999dec218459feb70343c7e8ed1283d2fc5c0";
	iv = "c2ae363e12534351c0a0cc8042e3c8d2";
	cipherText = "162d1143d79b8fde90eb85bd4bfe8442f3549b2f71b220c31e1432722490b82293f1fdfd30bccdf6ddde5a0c60e136343659da6b5ad04353775afbaba4f49354921675ce350e29f40a70eaca15f830a90d83983680143a0696514f667842abce28b4d706118644be0f7d8f2fb6deb4cd2fafcc350a2eeb8582107900887abc5975f2f12fa6297311ce6bb08f0acac3944af355197aadebe75aa6eae8cc25d2aa1e54520e1416ce09d5b2097d5051ac198e307b7a2d4aa47d8be9b37f659ed8eda297cbadd8180cca1c0622880544301c5fe898b5263564d7ce47f2c5eda28e9c7715c155111dd46855897c9a60e15500e905eab1d4149e912d59ef19cf13d211053245e60b8f97c613bdfc4f9a096523a4f48e4e4e81c5c77518fa4c3a4d16dd268750e623c8127b0f8576dbac53030010d1d264bf634fae38a2283235b664e1a2079f000c56463eb67f54fc6001838a82743d575b45cbe97f861c6176fff19b245d45297407bfa2e0b0219a1e2d1360b2452891415882f3374b27562d47261d6c7083007d5b55a14d8aa487d0290ee0b18d1032f70b03ccb41d0446fa24941821031e6ec068322e6dc1fe12419d4c046b9f5b1cdd57db999e820bc40096d27c01a01af0fe7f26043993570c8c295c0d6265270e018118d9036f679ebd9cf41b2c8248c70d0bededc609852158da1b550cbc6143cc2f43c11b23a017278782cf";
	plainText = "33e412b6f92cd34641d8f0427abd17f1163c60b9bd1d41ef512e3ad7f5891f8385e5ae411c35c1f3a8e8978fad841a5e3875b06e567b74c27e5b528dfbae6b9be5627729b350d7747c9b78ab59912c9d0e49189521b5e9ad5a74fd0510a976627ce578f5754b04ff5fc9d7833e795aeae6e3908cfdba4b17c300325d23496427673bf0a96d48615d9bb4fbbc167f857a36decaefe0060b5bb7aa3ba28a0802f24f11b73c493db64108d493856d0650e3b114921d23eb54d731aef6788cebe9f798d8ff191f27ee32fe398a9ff04d6882301a1287828957f300c406a04ba74f1bb8573624cc6a6c8112d8718bb7f7e1340950e44502dc77f2f98bcedfc3655b507bfb33b68511310c1601acb9a8aaa33812ac04eb9a9fa72b9c2391d6daafda835768f8480dd2216e38064f2a7fa27fcad96bbb817efcc8b4363615145bbc08f3ea0b2f18d6a7a43d586967e599b1e7263106635cfe5f1b2bd4f6e09a588046105efb8a16cb7d94de9a4e6310fa4664458adcc523d632688b8b200eec23509c1e8c21fa28f245c8ad0b6dbfee671682fdbfc33571a3eed1ffa8d00aeab22c8b0bb240b4f9a9e300249e1518a450119643f4a0e452d6a8df6da320020bb5916a8ccc92023c19f0036440d56fc7a4382b6caaafdad8a73add90b19bf11ad3540e707e7480028ff5c41fc7412b8256364a4b3c69c9376e2470c0bd8638037964bd9f";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 490;
	dataLen = 4096;
	combinedKey = "43feb5319da39d9b777d35797e5859133318cf92d4e08377512b8781560a05f3";
	iv = "2fca74ad7ff0741c26c5505bd75f2708";
	cipherText = "5dbc7747699bc0abe914c0f4065483ca2842c175227257dc82ae25804103562e0bb05a8d4344f6129181f600373cdf0a755aab02825ab6a95ff9ef7f38b985ced3e3bb802d00a63c006270bf12fb7b4e30b6a0df7e89441fe11ae760d346c326cc00d668c0d2e36a6803bf53b5b348af5267601490cfbbd1e7d7610501ff560d0c0b1fd5bc5c34e09ff6dfe9573000318530f5c382eaff6ddf331261cc519048e6404d83ee52c459bf68597221028479965270e5b2e884d834f4152fb298104b17e599e71896fb3ae273ac7327e82bb5975c1a918dc8297c9c3e96ae709b237005b80f8cbb8365759509516012dffead2d2157690e670053d00e70865801cee853509b2029422ab74b4c4065caeb8b5e86e2ae9dedf4f0398d467266ab3fec64170764bcee1b38260ac74fd689c18d784ce38d0e510347c3fbf83c212500eb28f1423ec1b4464091cded8c2ef89ca2752a905985581c88cc93892ce4d498b81ddfcba1ddc2594811af3199f3c6229c6753148c8828fd2b0e7e298f17dbee0e3f0cf93376b89377cc282b5d8224706b0e2c9f00edda831398033f1296f044f4247c225915644dfbf57f05748720140de72c36fe775eff6779f10bee2217eff9629f3a07c39610344decfbb36b885fe9ab44b0002d8bbc57c4d8fb04675dcbde2892ef2e139d67bf36f64d5e609ff6be36d80e4910eb5e9b7ce355f36a9a5aa5a5";
	plainText = "bca86f67ef2b9238c927956abd0880df8e6322a58f7deab7979169c8aaad1b41422e6ca744eea8a27da3e5579297fc0ffdae5d79d0ee8cdbf0ed976c03db46403389b73e56d8ecca1b6b038a67ac78a87ef00419d3d5b29b8890e11e8c92b1e454445cd30c326fb986c5abe73e8451b971acbb8d7144b00ed3e0dfcea5a17448a3dafa4347a33da422d6042be9dd28903ddd1eb2c741c06330900bad1d22fffbaf1e24a186f1098845f9226a39e93c8746d816e7fa432583fa07bea6522256cebae4968b8c825c635e4732fc4d0a89de2f37f4e8a6c0c80b6e8d316845442f61cda5375d886ac63bd1b60d0211b251370ba4fb774b816d07bd66f7a43e811f581d91db0da74b4d45d683af50a2fde17000a6f620ec63ef319daa6862d8da4a97b7cfd78d3384322d07cb087bb9f0e83231b9cba98e9097c5258b9374dd9b3c3fce3ccc8d1dca6ba0ce94ca577968d8944f17c94fe60ee814eb509e332b18d124458ad690bd8886463f16af4115d9d5c024da92879fcb35706fe729008ed67abb84008a93b708049efeeb7e157ff5dc53061f62740d5ac343ac5e6fadf5fc446d3308ddd0ee4c2735e4e607f41d545780be8bbddbcbf0cfdfdf0a2b18499be08d446515e67ab8115e536e9bf4d8944780c193c46c1fccf0e108a54fb539d61aaa672afed9087a8548f2c846426a2cd489cea8487cc4810b40c191e419b62b649b";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 491;
	dataLen = 4096;
	combinedKey = "7ef144de40295060488c348f224d6118d0dd08dc7679cd95b2aa88a4d763f9cd";
	iv = "a4a564eb6d1079742288c7336d422749";
	cipherText = "9702b144179fbc5d740a07be50e933c45d23b640b6b4404898fb5b59cae0655f1b6758428df96c70caba03b8daafa4202533ba4181191f1fc7181a621bb7a78fa5d565ff49116b1189809edb781e68cdd3f30aa5703bdaceb07162bada2bd5f52b40164ec6d89839900054220d80432d7dfae3aba24318af1d2d047712245ece6220173406077ec1ba0181d32fce588f45c6b6de211360b419e52849a7a464010bd5ff77296768d19a75588338950d7648914e1b037afc9c869ba13826b8d72be240f5f0bd608ebd599e03647607576537e74d6e4fa633c72a84d63b657d309de8c2e4afa53fa919bf89cc1af1b247e47f03c2a2615ac792e9ac359b6e13e03b4c25d3c5e0428ae6870d5d04115f868905c818242ad46e3e18220d3991e18608ed3bdf1881c1d83d35a9528d0db0a795ea67addcf346c0e12c1dd63a6528d40d05c6b7c0a6223950dcbde00cea9c7da8b3d3b3e795fa30da61be6aee72fc01a4c3b1266967e0214f372f3a64d45d8e1fabf127723e037f86897d95fc564a972de78a77d3953a9b23d02bfdebfc3c5a668001f1f0f48db4430482d1822811d6b9d7f1b7bb538cc791bac7415729631e30af675aa4609d1e96966b19a2cff30da259f835e71129c2e24f04f74d9e0b56c2de139e64a5af97a6b147e5df05eb774d2b3a52fb57119084b52e535d489cfabf3de55e0740d635f2f86566a3aab081af";
	plainText = "b53b93601aceea44f8bde7e18beb46a69fa7283c0d01ec95da344caf07547d0e84da2a02de876bb1e0100ef5954192a7768c8455cbe5408081c327c17f52a56582aa89bf14d08062828366176790e32486c92910eb82bf0aab9e5e308a575466ef4e1c51bfc7c6f0665a16738c8d97e425fc611a0c42160cde56f9af0eaa05cbcfc785200efa6cdabd2c58cb5d85bb05ace736586f3e7f1517a094e3aef137afebdff87b4dd3d341b3f83f6cd2da89001a913faca1b5f8c356d032b4316baabafcd6a03a3dff25179a16d5ae1d97c27279dac0df5213dc922e4ba014e9c54c6ee3a73529d73757ecb87953aed0db66b218f7bb9f8418e46320d19a05e62ccaa8ca6ec3abba392f43dfc739d0c7190f8aecab81015dceee65a7d2ae1dc15cacbab6392b159ea60079bf246f6bbc06df6108dd48b49841dbf5cb5f5d523bad08f6d26668348fab2d66d42891a76027d0a839e1b664b7a2952fdf9a4c5db8deff1b942115350adfb8bc0165b5edf8408cdf06c5998a0244169efb2fe44b3ebf8b6ed5a330e6695375ce7b453b589285b8fab45914214dc5473cb276e9a38d79a5997794355f4aa595b2f69b6d4636a00762af3a6e108abd0e9ca514bdc3a540290019aef97aaa1b1256182399caa24213edf6e89f775c2164e28c3283bda7e55e26b0c19d6be24d3278b7fe217bca8e74777a32c71c560b9d9b59b0363065e74d51";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 492;
	dataLen = 4096;
	combinedKey = "0e185134cba33a283a99a21d5b730649fda62ef8922dd302f953c5b7276d03cd";
	iv = "ab4f726c86bf3c084f1c8b8530703f26";
	cipherText = "d9da8c30ab7ad2a0b02775659f7ccff57f095fb0867bfb495b0c16d3bd1e3c6c2004c09931d0094bf54dbbe655f154d7f1d7c4aaaaae69d3c0c8ccac3a47ec1eae3fd6ba306a35f9dfefbe82cd013efada7c37f08603f5885bbb704696820a24e7fee698ad076f5a0e7f5d6f0aa78f54cdeca31dfd8a88deaa4fb58c3629ef28685fc54c90a404ba915e8d6be589fe0bca4606ccfdc0918bf60ea6665cd4cf994fe3d636463f918fe586dc18ee28ff92ea11f860ee2617cd4cb1b8c3dca8d42978e50c37c959248cf7c114c8ed586034b24ddec753620e84d3116a73a255e8a85fc1d71cfdaa7234ca6438d373c47df07aaa0cf9afdc5e4addb8d0557591c87f65bef469ff738f23283d540161379e3c9ee69d10810f0d71f766bc5651e4280c2197f62cc43300d46ebddcb1c1373b80d4ff8313d5052311d3e6422df60518bcc4801ecd9037aac028a6ba005358c9638f61de6df7aa78df4d55059f4dde32a60ad21293768dabb3d6acddfbd27b7c0761035c6e5ab8a3629a825c8175de21ff100610970a6f060c873d6a6d9e61031fa137450cd427e3670349572f65823b2ee5c026e3a9bd06f16db5f845535083ba9f195825d499fa04182eee0b7481353f4a3ed5737a54f1405ed82406419432ef945595bf63e2663d92420bb5f0ac4e02c02c2d725c89236638bf586dadc89106dfcfa50ff1bb18f3b37ccaf38aa6b3a2";
	plainText = "5421fd6a9b3b22c63013f7cd4ec7534ed979ccebfa393b71bae68b09c6f16a1c49f605be32c1b3b7785008bf69c7f27d284ef99344ca50e2105a1c72fbae0ca240946fdf693f1fe8fdff372a6a1510b65f65dc07696f587472b8e6b3d40e12b37bca4e8437619efc690d6d71dc4b3e4252a1bb5a10c68d36c31e36870b4b46a46be7afd80249c60751c42e734a4fb3b632ea21c8405fe71d09a1d49ea5d4d992d8854bc0ea4e860fb9939929d6c17d6e8fa9ffc1c6696a3ec91b29761be37e3ca3c80bd7ad2bfee425544186533a45f310f764962815590fc5b24ae35b0ad041c3e67d1248b1cc7daad861540059db2e16d5b913d31f2f47e26fc6c2b4d00fda551c448b345a503fc8c316215cb35882c52fa8a4fffe213617632eea8e9fcfe3404d75ae9fe30ad0cf0b61bc92bec04c1f592d4c0db83e4e87d96b73c6ce823dec3270e097d92b65f10e1302d1ab3688b666d5579dd1059d459c241a232becb8e41d4247c81e30c4d4f2dad28de0bee9bb62a5ad72a06c42e95efb8db5a6243c5e9eb2b5a747e24126f935da36d0d7d0428befc8b2869b6351d269014a6b8970b116ecb25c8f39ea606d9014c5812eb943e85eadc0ed5407e9ac483360b02ba54e4a4e815544eb8846ad7f42ddd277ceafb2b0857a3aceb45fbde7d9b0bb08b8e50d515102908b326d8a2367765d2c8dbacd806eb8d2004c3a1fec51e26ac2b2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 493;
	dataLen = 4096;
	combinedKey = "542cf1c7c6f4a1f2c06e3b04d66010e2c86be1e729d968d2d0f2e1a66a795a93";
	iv = "7cacdee720695232a27bd6150ac95ee6";
	cipherText = "df00fc2c79db45e92f6abe619ea5d29ebb5f3f637e1356d4dc1425cdd57c2c29deb21790f2c3cb5487629599c191a2550a6f0bf3d82347ea7d882862a3cf07ae61a98baa796821aa6db8093a87099e1cfc283ab381b25057f35cf67abcd95e170bf106999c5660beff08dd1bdbc8249801f0b0bb66b6bd355a9671eb53817c2af3d273427cefb79b044bf404151c0f5efb1b75e9ebc7fa1d1fca3dddd7aa3f19a078f221d5f8013e878899cf27b4c3d5972ae50df94215cf53085ea675ab11120df96dc71252a64e6257d31518043ef738afc95756d124a61d768fff6c2b523dce254569eee73a67aa7a2fe8605e3f6dc52f53914f4667d23c095dad733a835e68949a20743fa3e2d3c2adbbb06ae0e941e28de358560b1de21b3e0453771f767289dd1b6842766de2604829e1171c9953ff1858f668e3d55fb86d76c6bdcd6d37c483968322a015646f8df1871826d4d20a20bf6572c9eebad63baa1e8079fcc55abd2469f46a969565ca0c85c0881103355b4425557e25ee6e32612fda8352c1ce8eb87df95b86b3e82e1e15a0d2b5c36bd532b4d07bc5075f45aa7e06a50fe285e2a544b3648cec088b29a7f1e8b026ec7de82e79d93926d1187e1dd19ebadfa4c0464739dd395159bdcbc13017a70e732f394831ee0b7f2ed0e5cce318e2c941be8f29a6b50b418dddef592482048e8274785eb54c97f158ca26b7bedae6";
	plainText = "173ae7eb10a6278464639727d2a4cbdc86c088766a80db20eb20b793fca63c5de2d2d2c08697f4abb0762463ebe6d16bbf8ba7f2549bd50f921d7ef43d140afb86c6abc69162b75a7de7b3b081142fa093fd6406f75bda4bc83e6bc43f34a8a375c240fc63a7cc7ffcb8e7ed9288f306376a2a2a44edf4ab26821698b22eb4984b165eef836680fa3131154a4a119ffc9e0517462dfbe8553c4fff0be70a1674845ce3bf2e4b0e6ef560b899c225b1204a8bdc7201ac8e5e352630adb051b35c31c390f14d53d28013d65bb3359aef834a69ab0f2c446298d54f18e9689d5e18d0148a9ebe55158b45ba00b47427ca024579929577638cc3f46340f2b5e9edc4dbe4e4567add6f965a9d049049f62f90d7617107418d568db32ed1827e92aa08afa2748ca4f741de5346542cbe183098d9585f4cb3cb9afada6076dd29b5d5e5dc4b8e39629d6bd044fb919e88fd195d901983fd6626a4e54f31eed2cbf02b29e4751b99c42b01026182c2a4c3860bae566e844b9711ff8dacefc93f7ba810524469817e4b3926eb7df1a674caf0488094b863e59a0a8d2ca3f592e9f101ce0c05b9d6a45b9532801713730ddbf734d3dda8b75d896b13a2ce8861c1865a0607498a1d6fc658ea70494ebaf1917070edce1c7780d67f4d45684eaa5dad1d38478b22810a58e6b4a3e837037d754c158d145ff59cd5718fbf88c0453dc629c248";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 494;
	dataLen = 4096;
	combinedKey = "7a649d76930a91198fcb363dcce69a14522a861ce6b1d99c4aebd607ad63afe9";
	iv = "cbd0ca180dc0b8c704621a98db7d59c1";
	cipherText = "86c7265f461aa8086633c19528c1ee11c8a7011720da637e6e74bce4a34ad19778f660f68519aaaceab36e93cd7e7995d92f77d5d8f7bf058bf3f1e8bc68b8f7b0edba11598e5012da0c7404fd201c1c28ad5362ceaca08fdd68ad95f657760da8fc89da4e926be21dc8139a07378c7b7ba19640361978cfa51ad22cc9444266dd775c5e2ebefab7aa07d0608c96e5f257aff762a06f264a5b6b1cd68546144847c87acf6d72ad0258688956b2335e735fb521d27c33ab954dc858607bc7ac5c9e61c837906c081dfbda6b0153f7beb0746532871ad70d28507d12d22c937ffd17ea8966a565e6e35a59d094b246f7eb7a68ab37b1347f960ef12a4e5e0252d623b4bb486405c71f22b9681cdae84e50b28110a072d0ed9178d5552dfa82096bec6ab4685122bed356ec1a90641a6066ab4ebed86755ca7172fd53772e440c6f5654817882fc757f2c468f1a1d1f4c268b01e95ef1f7aae9b61abb497c182db72a1685dfca8f84e4609d5d58da60fed1c06c86a6a3fbf45268562d343352d08bad43b9897f432e133c4c07b8ca726049f82e4a914f8318e239bc3260cf04f29a93141f1fec92c1dc96af19a1b249c9a4c41e00d7b3ef1ac3331da3fd5fee47b4492c51ea027760173a52606cfafd5ae54eb64f8b3439a9d7aeab5a35fbbfa5836efd95a09468ec4542a411a43cedd2eb4af3b5e2e019a6b5979373ea6ae1358c";
	plainText = "758bb64af4cf4d966e3cc318172250c3b8cbaf1acfca7fee81fcc021888ba10466c5524466224cf838d91d30f48cdf417a8060b0ec6eaf59e031141c4bc8e7c251721a38509d8b427eff43ef7096badbb079e928d33c850540cd76c3017e1e72a793a630064c6695f89e6a44f85e14be4e01bbcaca5d7845c840b3b9e23cc192567c20a2d4c79c6da750a77f71d404dfc9fdba34b10b18801c06e432e2d78708ca9c19a148e0cbf930d1aa5ad479dcc67835bb40b564d583766a0eb09ff3b0370d2e1e1ce47c1f05cfc8bd8e0a8f77bd798c8cffbaaf79a9039b0cf0b9e03c580b3ed2c7a3a107a204e9a26db55fdc1e48228f5b22dd625c80f2a74bb7fb70342984c567b859fb8d33f4a995aefafd0644f96df6edbe6e8d9918cdd80862995877107dbc3e9f25c801efa1bf22c0688c2627b8c376176dd72198edc521eef3f9d65313ff50ac652bd798718d1d5d8e158f12811e462a23e3839a74776b075077fc9ece15df0c5728401100bb4f88c5030fdc558cc1220cb7ed43d38df06853fe390eaaae1d71643241835000a79ca80b387165e793e3eede10ae382dff5874660141aee239768feb1e4ecceada8b125d5ffd793e14704a303d738e1211de0ff5abe921c31f9cb51beaa651d8beda99733f7ab49a48a31af8158eaf9f3625c60b1eb0fd0ff653bf0bc5954fdcf76e44fd9aa15fae1ad080769e690b84d9701c78";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 495;
	dataLen = 4096;
	combinedKey = "33fab98e7dccfa592bba1c205dfda1392c17d33381878ea6c06bbfc675c4ca5d";
	iv = "91599b660246095bcba482ab527e0a59";
	cipherText = "e6aee2ff318e668567cd003d5173ac6fa633b04e70790864c9d2316b9b8443b85c8a416cdc1ac910530b1a08280a1dbf5c0eb84f50e8ccc036d0c33c489e241039b64b8242dd2ed0ec0ad38e59c6118cb97d05da83ca9f7ca48e055105a7e441c7e68a6108988497f334bb13458dea5e50cc4492ac51b79c9b449d1df230d481fe50b3553aea643200d304104ab76b314f8ab85b9fe031aa011ab6483f3139f8186f297e4962ce502d73d0286b8af7daa4795662e8a17584838d3cf7bd71caa8a5c06be958c8fc5356d7a2597c3828ea7807be3b735781f1ed2ee3e0fe268af5e79e93e507f25567a597a53ac15282e6941e8725109030ca3cba0bbb6ed125dc019c3d9f5a44e5dc6c244f053db958695a8140d2d0c3052d1905e5c675bacaaee1a60ba3e01899a7410e203050daec9319a14e5cd9311ccff8d79619d38d52e3a20150490156a18cf90b09aa64c763aaefeee52a62b484a7add07c53d316266d92d38f77d3ac5a374cb74f64e25595b678144f2aa3a9eaee845ddfc32a9df1b0d3a03301d7837299d6374f40d78b5f796b969c7bad3d84d153680b8dfbfe946cb441f33bb10aaec93866dcd858192c654c9e0cba397619779b048dd8808918d0c3ab33fa60e1d0bf8564364ef98fcefb32522ebb74c011eb8079be165126294a405198307f9d56b798ef677ede7c6d5405b496462cfc8f8b104159364af0edae";
	plainText = "c6f179fcc3961c1e423cf253e1f85676754ea98d026f6c84babf1baf2d76ef11ca7fcc42d7b5c16335af5c99b8cdfdd80db5354bddf1f2db03e8c95fdce20c773f740045d61b0afe2eabb7186ebd8846da2ce75d10a32909a3f38309bf29b9bb691fc8ecde0f433828e728feefddc1e7ba56a2f10d558af20348ec7a35dc9ff1b4eeadf89214a8dc1c14b71f21538fcd6b80c457d81fb09fc94e161dac2fe9b4f49a97659a63c9ad0a9b0ec0a3c3949a007d932e8fd490478bbffafe00ddc49a61dd178fb49322322544bc2dc0029496498f2d559ae4233a6425a36a4cd0b86ece3e4fd054a6cbcdff8ca66a020bdf3791ac67052f71234a7748d053e1e3918f5b2eaab88c08ef1fa9fabddfca9e7246a583585f5e41c1fd4539f46fc4656aaf44c671a1859d909cd8aa4da11107b855c0ca3ee49e3d3cc4595bde6ffb0ea6bd4e24b2368ba0a10f1ebbdee9323f0187dcd5254e674d46065c22c84e149cef8e72044a56389ee478701b1540cf116dabd8d2b110d941de983a02d019abafd5d6fceaac8e226f7ca8a1c823c6fb05b8398a42573f7b7c166b54ff47ac5308c7d23d79ef5534e6b748c9dc786f4b1834fc2a50976438cda619c03a14174dfd703775425cb8be45d1b6c7e5315e0b3ab128e80ad76ce3fce5ff41b369da638a21480fc71673bc40d098ed93a8a3150d7b249755740f3d27207e67976f7086764919";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 496;
	dataLen = 4096;
	combinedKey = "6899ec5a821a682067166d2331e7524c58a502105673b3e2372090f9e4db7a2e";
	iv = "31466925ad4956dd1ffcab9f1bc05096";
	cipherText = "d96f20c1a04f7b586f6471489405ecd923e6bd28ab3504626738afd3e8f6c66fbe5c396be92df50528f4e59c752a8d94fd3370bcfa9118ffc6d50aa8f98276ab41601aeee0edaeee3e2a075ffe979d30379a1836d85f0e7f69c73b289605f2499e37d70156d1e13a56062ff61249dad9c4a0dbbfbe00c2a062d506455bbd9638f94002ec2da1c2f42b79d8dfcccc34d8f1c6ce05f44012a8642fa12a751b7a163c781d73687d6715f05001ce002660059cdfbb3785756a79037d2e51dd4426774fa520426b08260466342495cdd35c89aefb55d24c3d9e8cdd1877ca4f763b3dda6ed0780b05bf2b1cb38db04c79d1c8028ec6f170bdd5d36aee5e127c666ce6cf5765669555a1b31e7d4781afe81ae7a61df57ea3ff52cf8c9a4aea7a6f48bec82d0f746efc35c169ba8e61590cb4c6c21e0901e8a211832de25db4e13c73fcd8be3303c99f5d0ba048a626ef2e84c3b093b5b8df986acaaa684571818b6a130d2b0f6b5afb8badd566654b5998839e9eff99d2e2e0f01f0b84a532cd058f94668a258e7c4c4cbe4adea3a8f0b40b81ed3c7b6a1389bb32f3ce7e91572206e9b96349f35a647fc60894af51a5f2be918524d997f166bb1f24db380c002e93da5e9ea523a1fc73661ea3159fc7351cbdb8745d76d72e2caef454b04d5049cf8202bbdce03f9be2df4544e290a582b702b242b6710366a1502c8857eecc7f608d";
	plainText = "a85f46594e831881d9b2f94ab7413c4953598a4a02aa26b3eba1d0ad4979318c019fee0327526255bbfb94af48cb63ea6ca52cc86b2eddd1f8f9683484229b04eb20bba3dd66a987ae25ea0de3d7f06ed7a37dd3e3e31b0eaf2250e5cd1d1dd5ebdd942f2fb0e43777eff8de0012c794ec4c847f0d3f889085c795e649c69156d0bf0639be452ea5e85d96bff4641382eee0903deb41a79a3f3fa3753331bc25b9cf72a367d913090f914a47c25d1eb3a6b5c78ed32e15684bcf70db8d33570c10ea65d2722b7879bdabcb3cc79806f5a7d18ab8100671104c226352a6e63671040350d1432fc0a4802155ed1dcf2dbdd9c7b2bd2bf7e1b05c9f679c50db434040f5af8e86d7891e722830af1889d51f681b6a7b40d7cfd82aff22888fca148301afdba07939242996a33debc67ffe6e1d6048c5ab71e5c04c84ef24bd6bd8bbf82b75bf2322d3fc286f841b1f1f5fb375cd0697647c6797af86c7b4b553ffd6ca37fff508fc1ef408731137a3c200562ef896f286376fa503f5af54ac28b2cf5c862a284660e4761e01cba34ab5ea40dd5ba0059d41e328148c6b197a032271f22fd0dc00cfaec84da30822d9e67bc548aa3eaa4cf51b83b4c878b5daa23ffa4ef4ffbc0ee458b4a060a2dce2144cebb5e3acd5014494a7d6b770a0c7b2e291d6df52411e3b4bbe5cdbe45da964a283d5457201fcfa83eb3521e4c32e9b61c5";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 497;
	dataLen = 4096;
	combinedKey = "2b850a053287d5e65a70767fc8ac0a38600cebb8e73cb906a0863a06e0cfc9a4";
	iv = "5c4063ec205c866750e038b66df7893c";
	cipherText = "1ffe6987cda60dfe462bd8a32bc1b64e70122872d3336766bf352d908d933de4239683e05394d144be91e201f27c9f15456b216a2d402f1737e70e8087499e23d6c63ad4d517e510a77374d5aef3f0986bee4826cd80b1759fd0672057d30289d82a6a273b4f5926df1e86b9e783d87f28be9145679fb14b1a096d53aeb59674dbed5b1ddd70c0da6963fbb11e6870f829ea7db5b6e17cc42d17aa34cbca15638b1688ed4a162390a456751c42e5f407940f0f8edf31484b5b147ded7e20b92744e0e61efcf7a73716ce5ca6228784e9d7195b9e698bf5e5c83ffd4100f1a3a9494774946055daaa6c2cd0563978a1f5ea0a3b16e786f91b96ff73ae68a1c92e160864704d759667d201ebd9015661a9323f817c79ffd92c0e7559a4f844637626de7fb0c2848c27a511134a558d5f9260e0e49ab0b230b81dc1deca488dc622d73490e8a0a0c974e101eb6093868eda06ec2174d9c34430348f546e34bf30e05694682791cf4445e448252f914ab0bdd997fd795f60ec8815bb00a59b8f8b7f25955fda656681db671bb07719cf729dd3b554a3706fab22b5386f9290081dc81f39da4786ef1924b77bc00f2674ef25a002e0750857295d79180a01af348daba4dd1e0fa2add89bbdee094f6f8237a84f927c37decbbfcb3602fc68e2b4c5e120b444dd76f71e70c973ce3bd6d3c9c44f93945d3931c60570adad58cced9aac";
	plainText = "bcdeef44d16969138c4fda1224136ce1e019ce8fed0102d4b83dc1998b61a4c80bc2db2914b9ba702150ad081c55529773fc01177cdbe34e70ce67758f90f0b7d39771b1e1e43b50f1706eac4fb6abe35b80211b8eea10d05bbcf23b74c4348eca6e9ba8f4ee214c1e2601489d480cd6cced6db384b8b432df03e810327f9c75d37df3f1b83dbc58cdc19a3864ce52304d36e7da1600342cebf48a618a46828d268e4e72f02ed5050a849643772f9ef765078818d5c4a1fd11898bc4608a39f12408bfa11a9fc20140931418b1f1bab1bce47add171be34b1bf2f2809e48f89a08424bf0896d87bd0524d8304ef6c455936e1cf918dc40a1b6d6e5d8f4f797d0aff1ecd6ff1b165014711988f0680c01e2758c5e4213fe2d2eca54cb454b33bc2eb1f9d8d57bd77167ec4359e99e4401445da8e784097bd19dfb2ccabc82142ce3a7fadd63e6ede8cfca157d9c7463ff774adf4c287925d71f2b38182f06962f3b9cc54341ded43f7aecf4e98d6f2aec6399e9b6392172fd4eb14f19b3a1ced2efd4820b9236a6cf22c0d5cfc6b415ce9448676f8d5ff8b3a51dbd60c64c4c2a5ccc2379d5753e9ffe56aa572b7d8d4196a6fcaa3d5655907519c039f92e2fd0a617702ded16449ba639cea90456abc8aaaad4241e7cf4fce4a1ab189fd8e53fe8476345f4c7bf6ceded09552a9f1ce4b782443caa9ec2d4d5eb44e9d8b4cfb2";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 498;
	dataLen = 4096;
	combinedKey = "fdfaa13b1e7d6ccc944a6cf296bbe610a238bfca37fd9fa689248d9006d21251";
	iv = "45ea5768788b455720c6a46aa09399ae";
	cipherText = "c06bc279f61f3b45b4dbfdd7fe4ed5e4180ba450288e3173126994b2e7605e59a4dc247a5805975c1c1cb489494c9925c97228d76e78abf5fb706cdff92c53683cb965c8d5733e29073a10b34c8125106bfd8369fdbe901c0f8546e147f0c9ebdcfd1cc00ad097abbb04313d38457b9bdbe607ae79c434344b7a3c342b3cd2966ca5d6332b5e5e159f12bb958b4640be969a9cdb756d24c7d18064643438809cf6864b3c9fecc7d3a3983a6d44d41dd577033bb13740e2dca56527404f8743599ca7cda4f5c57fd4d0ba6d611209ab5acdb4aba71121b97c8f2dd72b286ebb21dd8b685883478bcc01b7f11caaceaa12ffee8c1604280cf1b73457df14eca984e8051d7faaa2569136574a5064ba0c1584963a822155a56915ed10ef1d3787bb7fc6f697bebdc2ddfc9a99d518a66bec2ba28c1ed2608cef45664293b45ec60c3e656020871a5e80ed15f0ce017875989139b0ee0b8f66db7802e90253a3a4ca08ea8b100c8f0f5a355d4ca36d200337adb8fcad60e671be2f3a556667d636f84bc18d366bbf8897d5dc065b1a5134a3284f54777931b99455a7512d57c9a0f942f49f7d7b56126623ff6a575a21f374a7c287cce9d3c524da068a52f45a4ea19ea72b43ee3b4b4129ff012e1111c4477f747be2e731f21f7c4c690513990e59a5a0c4573a51bb7ec28268f36d4ed362a6393b9030b637b10553809f01fcb882";
	plainText = "8a5258bc0e300d26aec1d3bed13f77f1b3c79ab17333e0624459c959f56ef86efffb318b663491067fe4931fe5523ab2372d82d1b0bb4359a24fbb906d91f5d6d5a1bfbcd3ffd0726e2d609701fa7c9ec9795d31d48a09332ad57725a94b1dfc96b46fd49e387217562d0ef59181c46c2eeaf8eed3516c05f53b46951b279abbf657f0ea11145f79247c5893930fb16c4e531ab6ffc327657cfaa34cfd7f888b6088661ffc085a85174d4ab46b5277be9a07e617ea659e55cdd19c35462f88f6a775a2e1ddaa924a37d24d1f510707f88fdb371d08d0149bfec813b46ffba14317e738e14b437c34e6fd87d890d45acd00a5c295c64084edd29d86648e1436197997c50ed63c8b939d551c8781697994097a0c639e65bfecffe5268866d58c8691cdfa9bc802bd4033f21863f165707e8f7c97e9d9cc578264d1079580c25d4cf8c9cd78a6662da22c562d379e4360e7d05ce64bb6a391602a30152814dca5a0b805ea4f304c5b3b41e526cd0efc989f5b8f1d86ea76cef1a60940271062dac697e9fd3b35d364791b85f92090416414d4ee285ae12f7bd51aebde6f4dcc5b2a0cec39ea8f7e369f0cd5dee98f9cd5f0abc829eed816a6c46a4b9d51795b7893a7bbd87237500d03abac8f5257e2b495e789f29d671fcc09ff2d8afc12902a5244da7310b8eeb70643eba55d7db24a394c61e8f8109d83ae01e2cb685702f289";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 499;
	dataLen = 4096;
	combinedKey = "fd6058bbf1f3caa296afec24bf972da1b627e08547aebaec690f147204d324f1";
	iv = "b0c912bbc0c85646891470f564a8a71d";
	cipherText = "ebae2d62859409989ba0bc3801bf6592869b3e0b5a92d1322dc21a1dfc82cb387cf8c5b4e0b4d3f5b3aba0ccf0107492c6adc1dadd9582d6ac64a038fd5a0b073d47d7302570ad3c9b778770ef84b1a759fbb7f3f4d42f4428f08be012aed235d3104354f75faac53c72f4a1d07962803130d2034dae86c5a0464285fed6153e2ce7b9d198f18f954d9aa9487be1cf9b8191269aadd2f0ec08b16d10b466fff26063ebb7e7762b17a790cd3a16fa840054b6197f30295892e742d05f0fb326056f91c78a045f79d59257190248e81dd4d2d47c658f63782b3f90666555d03af36b3a25b921e63388e097355651c3b88de750351dfc10ee1c05ec75614d186625b59888155645781b962bf14a2e53d5a77efad71dec328bc99a8d330129d354d2ceb9e781975f69cc6476df9e2e8b150910e07f03a289f63c813707d277d167795624c98a80b68f6bff6144e946cff384df2b7797750448c7d4bc108a3eafc6101cfbb97f69cc8026d16744a073f47c8c441de0411065b462ec59619afe502b8e17e74876b706958035a3fd1a862d7c0827274dd9ade6962df211678a6cee34fd403431626b4eca05adf5865af005cdba5b2250b71933a2d8d67d8488dd014fb1825cbc8648fddffe512588fbcf7186c8acae8ecc0cfdcaae5d84dbb9fb749477ae4a52445b01971742cff6112f2ebb37ae6cbc082333b3e57c7a30c4c080491f";
	plainText = "a59436eb03f8e2d4a17f8a3020676fd6452827875fb1e0977ace07609d07d2d1eebfcc7bbc5e9544ea50d9ab2e804e78e1f4d1eb91027c2d3ed6e2964b5e142551c65e4d67f428dcb91a5cd699278c92821f676d0b3f3f4ddfcb02e79775c8194914846068530ed7dd5c3ef8ad33b71a1dbb4e4b69b47389308798d32d3b1e0b50bb5fcd4ebe7cb8f9c4c1e71beea8de6952525fd833f4610c78fdbd6c68840d37a10e230a383609a6634beb287ec13e006e882e40fddf013072a185ef2ba306a8ca2bc207a65f2183014cafda16ebfce401a4013b01faa2b6d6ea41c9892fbbc52bd75bc0f5770c90cf17852b8dc2fe5441ac9862fd0860ef177e002887044619cfc080038dd82aa838385dc3486c0cdfef4c9aa611e6bd78567d73402a48687461c70d23896ebdcec20f333e4126b94c630495afff9d73a6596f7285743314cddbc24d0069db5d102213265bc28eec212724080bba619a11781d7f7f78de1e42d5b7f15061d10a94853391451427ac5b656c11a7d0721371411245e0d58f6f8b251d15ae1d1532e232cd4323c3523b40a4f6472ada2688d9eb5b4e541cda5a3422453d05f4ce33fe757f72303ef18b63205ec27f1e1fceefbbcba0d46fb831e90b4d7c8310225fe566c583df2654a1af979d6affd70298e4d3422dbe234d79f57ad3eb37d8f569f3f4efaf4e9062585281ff9d82d99cf0dbdec6154be88612";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
	caseNumber = 500;
	dataLen = 4096;
	combinedKey = "badfd2102e1e180a634204249c5a6933b2e5d93f9a1cb62ab1ecd419e399dc46";
	iv = "84c06c16c151007ca9ed9bb926e66eec";
	cipherText = "4b52b5e85aecaaaf886bd9e8805390c62e12e13357e4beb3b713e37d217c6f7a9e432a04f87bd8a4dd0ef79eb7bf41b5a2a27e63361d7cb7af7b3c9a8f0b56ae27dc9cfd6c10eb1a79c7be35d31c3965b8e7099775f7644029bd79321f5dd12c55280a30fabd1b95e27c2d4dec6ca4d8716f36e7abe3408f5120560b573e5495ae7aad668fa84d6a8a1156c231a5b6d983ece3e27d199a806dc629c1a60c08ccb0e4807d9fed88f28ce0f59583708f540f97110b2620b1679220abe13e3c4b727186b289794583b20154ce9a07a284df3e63572f462142cae8949d7dd6f2b26fb90d556ec75e93dd33b59d697883312af89e52945b9baedfebe28759cdba4dfbf6e6f201b087478642cf0b34f983593c68947e4ee05bd17716e6cfb7c74c876c0ba650f3979f5eceb72a71d0d46aac4474ae2048d2a9884aa12e292950c77b17de11e8d3e895e60b1c584b1c8d9edd40ba7917e396d1d3bfd1941923aa40213195e8b8f7f4d5ae1057cbecdf89c8959745d1fcece59115819dc661e7b097c132e8f98720a57a83469cb82c374fdebc97badd7cef8d160a7f27d50f35b7e4af6f1b78361828e32a55b25fd56efbc12f8fcb7e2e4f882afa0c7747a455a1fae00a561cbb878e01b32fafb23f397371a8b3441c8da654b902d8489383542188821859a44f0fb2b63a49835f8ba5f0231ff0f8f5fc3d5c812331b11e39bc03394e28";
	plainText = "9f1d65e6a345a2bad796cb1493395fb792a2273de720b9b1b62825eed5042af968c35b17eb2c655ba7b32760cac310ef8869d59b7938f5ed0d07f9988aa543f16e4d8031a07e112e67e8a77434a6a8321d0b1595b21bd459794d4d1dd4935b19b2efbed9b740c4b40c8409b9b83410a2e4f988c93d055d4c6770d71ea3f43024bd2187d6647fd3858521c900f6c8c61bf9cad2c31acdea49246ee15488cc8a066532539401a8a5d88f1fce836bad97948922fbe36c6c1d27f7e6203e601859527aefc655156a42606f159d19b2e03812f311bb0d3469f53e89be04d3dc75d6a1afef7a42ad8859b4e2ca6731e89923178f141c2324b2e52d18374f9c6df9e49c1c4faa4322b093d8ff37a1a11db17724000e58b08c47162614940a852e65ea1e6167721dd473e1e7a9596e1398d4196a7cb83672c93a01db4565056a25c57662f541efabd24d8f739db5c187718b5f78076a99d54bdf46314ce681721b68a02c5a0cbd566b9afecc0fc6f53299f722e7a58f29b374d3bd7b56525118b6f3502ac626e7445bdcf0ce7cfe88d05b04bd50416a3761500748c3cd4d9835a7e571d681a5c47e69b87761be168e9a5857d66ddbb385d6c9a28c11584f86f38e26febf2636fd51ebe5ff851810f73d032d84210f93f4327cd71aceec451ac91bd818f8feda2c365c77d4b6051fe6cdcd2a5816cd6c8cafdc8bd0b74eefd34531d4deb1";
    
    
	doXTSTestCase(caseNumber, direction, dataLen, iv, cipherText, plainText, combinedKey);
    
    return 0;
}
#endif

