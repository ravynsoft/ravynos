/* 
 * ccHmacClone - test CommonCrypto's clone context for HMAC.  
 *
 * Written 3/30/2006 by Doug Mitchell. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonHMAC.h>
#include <CommonCrypto/CommonHMacSPI.h>
#include <CommonCrypto/CommonKeyDerivationSPI.h>
#include <CommonCrypto/CommonDigestSPI.h>
#include "testmore.h"
#include "testbyteBuffer.h"
#include "capabilities.h"

#if (CCHMACCLONE == 0)
entryPoint(CommonHMacClone,"Common HMac Cloning")
#else

/*
 * Defaults.
 */
#define LOOPS_DEF		200

#define MIN_DATA_SIZE	8
#define MAX_DATA_SIZE	10000			/* bytes */
#define MIN_KEY_SIZE	1
#define MAX_KEY_SIZE	256				/* bytes */
#define LOOP_NOTIFY		20

/*
 * Enumerate algs our own way to allow iteration.
 */
typedef enum {
	ALG_MD5 = 1,
	ALG_SHA1,
	ALG_SHA224,
	ALG_SHA256,
	ALG_SHA384,
	ALG_SHA512,
} HmacAlg;
#define ALG_FIRST			ALG_MD5
#define ALG_LAST			ALG_SHA512

#define LOG_SIZE			0
#if		LOG_SIZE
#define logSize(s)	diag(s)
#else
#define logSize(s)
#endif

/* 
 * Given an initialized CCHmacContext, feed it some data and get the result.
 */
static void hmacRun(
	CCHmacContext *ctx,
	bool randomUpdates,
	const unsigned char *ptext,
	size_t ptextLen,
	void *dataOut)
{
	while(ptextLen) {
		size_t thisMoveIn;			/* input to CCryptUpdate() */
		
		if(randomUpdates) {
			thisMoveIn = genRandomSize(1, ptextLen);
		}
		else {
			thisMoveIn = ptextLen;
		}
		logSize(("###ptext segment (1) len %lu\n", (unsigned long)thisMoveIn)); 
		CCHmacUpdate(ctx, ptext, thisMoveIn);
		ptext	 += thisMoveIn;
		ptextLen -= thisMoveIn;
	}
	CCHmacFinal(ctx, dataOut);
}


#define MAX_HMAC_SIZE	CC_SHA512_DIGEST_LENGTH

static int doHMacCloneTest(const uint8_t *ptext,
	size_t ptextLen,
	CCHmacAlgorithm hmacAlg,			
	uint32_t keySizeInBytes,
	bool stagedOrig,
	bool stagedClone,
	__unused bool quiet,
	bool verbose)
{
	uint8_t			*keyBytes;
	uint8_t			hmacOrig[MAX_HMAC_SIZE];
	uint8_t			hmacClone[MAX_HMAC_SIZE];
	int				rtn = 1;
	CCHmacContext	ctxOrig;
	CCHmacContext	ctxClone;
    //CCHmacContextRef ctxClone2;
	unsigned		die;		/* 0..3 indicates when to clone */
	unsigned		loopNum = 0;
	size_t			hmacLen;
	bool			didClone = false;
	
	switch(hmacAlg) {
		case kCCHmacAlgSHA1:
            if(verbose) diag("hmac-sha1\n");
			hmacLen = CC_SHA1_DIGEST_LENGTH;
			break;
		case kCCHmacAlgMD5:
            if(verbose) diag("hmac-md5\n");
			hmacLen = CC_MD5_DIGEST_LENGTH;
			break;
		case kCCHmacAlgSHA224:
            if(verbose) diag("hmac-sha224\n");
			hmacLen = CC_SHA224_DIGEST_LENGTH;
			break;
		case kCCHmacAlgSHA256:
            if(verbose) diag("hmac-sha256\n");
			hmacLen = CC_SHA256_DIGEST_LENGTH;
			break;
		case kCCHmacAlgSHA384:
            if(verbose) diag("hmac-sha384\n");
			hmacLen = CC_SHA384_DIGEST_LENGTH;
			break;
		case kCCHmacAlgSHA512:
            if(verbose) diag("hmac-sha512\n");
			hmacLen = CC_SHA512_DIGEST_LENGTH;
			break;
		default:
			if(verbose) diag("***BRRRZAP!\n");
			return 0;
	}
	
	/* random key */
    byteBuffer keyBuffer = genRandomByteBuffer(keySizeInBytes, keySizeInBytes);
    keyBytes = keyBuffer->bytes;
	
	/* cook up first context */
	CCHmacInit(&ctxOrig, hmacAlg, keyBytes, keySizeInBytes);
	
	/* roll the dice */
	die = (unsigned) genRandomSize(0, 3);
	
	/* 
	 * In this loop we do updates to the ctxOrig up until we
	 * clone it, then we use hmacRun to finish both of them.
	 */
	while(ptextLen) {
		if((die == loopNum) || !stagedOrig) {
			/* make the clone now */
			if(verbose) {
				diag("   ...cloning at loop %u\n", loopNum);
			}
			ctxClone = ctxOrig;
			didClone = true;
            if(memcmp(&ctxClone, &ctxOrig, CC_HMAC_CONTEXT_SIZE * sizeof(uint32_t))) {
                if(verbose) diag("*** context miscompare\n");
            } else {
                if(verbose) diag("*** context clone worked\n");
            }
            
			/* do all of the clone's updates and final here */
			hmacRun(&ctxClone, stagedClone, ptext, ptextLen, hmacClone);

			/* now do all remaining updates and final for original */
			hmacRun(&ctxOrig, stagedOrig, ptext, ptextLen, hmacOrig);
			
			/* we're all done, time to check the HMAC values */
			break;
		}	/* making clone */
		
		/* feed some data into cryptorOrig */
		size_t thisMove;
		if(stagedOrig) {
			thisMove = genRandomSize(1, ptextLen);
		}
		else {
			thisMove = ptextLen;
		}
		logSize(("###ptext segment (2) len %lu\n", (unsigned long)thisMove)); 
		CCHmacUpdate(&ctxOrig, ptext, thisMove);
		ptext += thisMove;
		ptextLen -= thisMove;
		loopNum++;
	}
		
	/* 
	 * It's possible to get here without cloning or doing any finals,
	 * if we ran thru multiple updates and finished ptextLen for cryptorOrig
	 * before we hit the cloning spot.
	 */
	if(!didClone) {
		if(verbose) {
			diag("...ctxOrig finished before we cloned; skipping test\n");
		}
		return 1;
	}
	if(memcmp(hmacOrig, hmacClone, hmacLen)) {
		diag("***data miscompare\n");
		rtn = 0;
	} else {
        if(verbose) diag("*** clone worked\n");
        rtn = 1;
    }
    if(keyBuffer) free(keyBuffer);
    
	return rtn;
}

static bool isBitSet(unsigned bit, unsigned word) 
{
	if(bit > 31) {
		diag("We don't have that many bits\n");
		return -1;
	}
	unsigned mask = 1 << bit;
	return (word & mask) ? true : false;
}


static int kTestTestCount = 1200;


int CommonHMacClone(int __unused argc, char *const * __unused argv)
{
	unsigned			loop;
	uint8_t				*ptext;
	size_t				ptextLen;
	bool				stagedOrig;
	bool				stagedClone;
	const char			*algStr;
	CCHmacAlgorithm		hmacAlg;	
	HmacAlg					currAlg;		// ALG_xxx
	uint32_t				keySizeInBytes;
	int					rtn = 0;
	
	/*
	 * User-spec'd params
	 */
	bool		keySizeSpec = false;		// false: use rand key size
	HmacAlg		minAlg = ALG_FIRST;
	HmacAlg		maxAlg = ALG_LAST;
	unsigned	loops = LOOPS_DEF;
	bool		verbose = false;
	size_t		minPtextSize = MIN_DATA_SIZE;
	size_t		maxPtextSize = MAX_DATA_SIZE;
	bool		quiet = true;
	bool		stagedSpec = false;		// true means caller fixed stagedOrig and stagedClone
	
	/* ptext length set in test loop */
	plan_tests(kTestTestCount);
	
	for(currAlg=minAlg; currAlg<=maxAlg; currAlg++) {
		/* when zero, set size randomly or per user setting */
		switch(currAlg) {
			case ALG_MD5:
				hmacAlg = kCCHmacAlgMD5;
				algStr = "HMACMD5";
				break;
			case ALG_SHA1:
				hmacAlg = kCCHmacAlgSHA1;
				algStr = "HMACSHA1";
				break;
			case ALG_SHA224:
				hmacAlg = kCCHmacAlgSHA224;
				algStr = "HMACSHA224";
				break;
			case ALG_SHA256:
				hmacAlg = kCCHmacAlgSHA256;
				algStr = "HMACSHA256";
				break;
			case ALG_SHA384:
				hmacAlg = kCCHmacAlgSHA384;
				algStr = "HMACSHA384";
				break;
			case ALG_SHA512:
				hmacAlg = kCCHmacAlgSHA512;
				algStr = "HMACSHA512";
				break;
			default:
				diag("***BRRZAP!\n");
				return -1;
		}
		if(verbose) {
			diag("Testing alg %s\n", algStr);
		}
		for(loop=0; loop < loops; loop++) {
            byteBuffer bb =  genRandomByteBuffer(minPtextSize, maxPtextSize);
            ptextLen = bb->len; ptext = bb->bytes;
			if(!keySizeSpec) {
				keySizeInBytes = (uint32_t)genRandomSize(MIN_KEY_SIZE, MAX_KEY_SIZE);
			}
			
			/* per-loop settings */
			if(!stagedSpec) {
				stagedOrig = isBitSet(1, loop);
				stagedClone = isBitSet(2, loop);
			}
			
			if(!quiet) {
			   	if(verbose || ((loop % LOOP_NOTIFY) == 0)) {
					diag("..loop %d ptextLen %4lu keySize %3lu stagedOrig=%d "
						"stagedClone=%d\n", 
						loop, (unsigned long)ptextLen, (unsigned long)keySizeInBytes,
						(int)stagedOrig, (int)stagedClone);
				}
			}
			
			ok(doHMacCloneTest(ptext, ptextLen, hmacAlg, keySizeInBytes, stagedOrig, stagedClone, quiet, verbose), "HMacClone Test");
            free(bb);
            if(loops && (loop == loops)) {
				break;
			}
		}	/* main loop */
		
	}	/* for algs */
	
	if((rtn != 0) && verbose) {
		diag("%s test complete\n", argv[0]);
	}
	return rtn;
}

#endif

