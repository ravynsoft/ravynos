/* Copyright 2006 Apple Computer, Inc.
 *
 * ccSymTest.c - test CommonCrypto symmetric encrypt/decrypt.
 */
#include "testmore.h"
#include "capabilities.h"
#if (CCSYMREGRESSION == 0)
entryPoint(CommonCryptoSymRegression,"CommonCrypto Base Behavior Regression Tests")
#else


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <CommonCrypto/CommonCryptor.h>
#include <fcntl.h>
#include <sys/types.h>

/*
 * Defaults.
 */
#define LOOPS_DEF		500
#define MIN_DATA_SIZE	8
#define MAX_DATA_SIZE	10000						/* bytes */
#define MAX_KEY_SIZE	kCCKeySizeMaxRC4			/* bytes */
#define MAX_BLOCK_SIZE	kCCBlockSizeAES128			/* bytes */
#define LOOP_NOTIFY		250

/*
 * Enumerate algs our own way to allow iteration.
 */
typedef enum {
	ALG_AES_128 = 1,	/* 128 bit block, 128 bit key */
	ALG_AES_192,		/* 128 bit block, 192 bit key */
	ALG_AES_256,		/* 128 bit block, 256 bit key */
	ALG_DES,
	ALG_3DES,
	ALG_CAST,
	ALG_RC4,
	/* these aren't in CommonCrypto (yet?) */
	ALG_RC2,
	ALG_RC5,
	ALG_BFISH,
	ALG_ASC,
	ALG_NULL					/* normally not used */
} SymAlg;
#define ALG_FIRST			ALG_AES_128
#define ALG_LAST			ALG_RC4


#define LOG_SIZE			0
#if		LOG_SIZE
#define logSize(s)	diag s
#else
#define logSize(s)
#endif


#if defined (_WIN32)
#include <windows.h>
static int
appGetRandomBytes(void *keyBytes, size_t keySizeInBytes)
{
    HCRYPTPROV hProvider;
    
    BOOL rc = CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
    if (rc == TRUE) {
        rc = CryptGenRandom(hProvider, keySizeInBytes, keyBytes);
        CryptReleaseContext(hProvider, 0);
    }
    
    return rc == TRUE ? 0 : -1;

}
#elif defined (__ANDROID__)
static int
appGetRandomBytes(void *keyBytes, size_t keySizeInBytes)
{
    int fd;

    if((fd = open("/dev/urandom", O_RDONLY)) < 0) {
        diag("Can't open URandom\n");
        exit(0);
    }
    if(read(fd, keyBytes, keySizeInBytes) != (int) keySizeInBytes) {
        diag("Can't read URandom\n");
        exit(0);
    }
    close(fd);

}
#else
#include <unistd.h>
static void
appGetRandomBytes(void *keyBytes, size_t keySizeInBytes)
{
	int fd;
    
    if((fd = open("/dev/random", O_RDONLY)) < 0) {
    	diag("Can't open Random\n");
        exit(0);
    }
    if(read(fd, keyBytes, keySizeInBytes) != (int) keySizeInBytes) {
		diag("Can't read Random\n");
        exit(0);
    }
    close(fd);
}
#endif

/* min <= return <= max */
static unsigned 
genRand(unsigned min, unsigned max)
{
	unsigned i;
	if(min == max) {
		return min;
	}
	appGetRandomBytes(&i, 4);
	return (min + (i % (max - min + 1)));
}

static void printCCError(const char *str, CCCryptorStatus crtn)
{
	const char *errStr;
	char unknownStr[200];
	
	switch(crtn) {
		case kCCSuccess: errStr = "kCCSuccess"; break;
		case kCCParamError: errStr = "kCCParamError"; break;
		case kCCBufferTooSmall: errStr = "kCCBufferTooSmall"; break;
		case kCCMemoryFailure: errStr = "kCCMemoryFailure"; break;
		case kCCAlignmentError: errStr = "kCCAlignmentError"; break;
		case kCCDecodeError: errStr = "kCCDecodeError"; break;
		case kCCUnimplemented: errStr = "kCCUnimplemented"; break;
		default:
			sprintf(unknownStr, "Unknown(%ld)\n", (long)crtn);
			errStr = unknownStr;
			break;
	}
	diag("***%s returned %s\n", str, errStr);
}

/* max context size */
#define CC_MAX_CTX_SIZE	kCCContextSizeRC4

/* 
 * We write a marker at end of expected output and at end of caller-allocated 
 * CCCryptorRef, and check at the end to make sure they weren't written 
 */
#define MARKER_LENGTH	8
#define MARKER_BYTE		0x7e

/* 
 * Test harness for CCCryptor with lots of options. 
 */
static CCCryptorStatus doCCCrypt(
	bool forEncrypt,
	CCAlgorithm encrAlg,			
	bool doCbc,
	bool doPadding,
	const void *keyBytes, size_t keyLen,
	const void *iv,
	bool randUpdates,
	bool inPlace,								/* !doPadding only */
	size_t ctxSize,								/* if nonzero, we allocate ctx */
	bool askOutSize,
	const uint8_t *inText, size_t inTextLen,
	uint8_t **outText, size_t *outTextLen)		/* both returned, WE malloc */
{
	CCCryptorRef	cryptor = NULL;
	CCCryptorStatus crtn;
	CCOperation		op = forEncrypt ? kCCEncrypt : kCCDecrypt;
	CCOptions		options = 0;
	uint8_t			*outBuf = NULL;			/* mallocd output buffer */
	uint8_t			*outp;					/* running ptr into outBuf */
	const uint8_t		*inp;					/* running ptr into inText */
	size_t			outLen = 0;					/* bytes remaining in outBuf */
	size_t			toMove;					/* bytes remaining in inText */
	size_t			thisMoveOut;			/* output from CCCryptUpdate()/CCCryptFinal() */
	size_t			outBytes;				/* total bytes actually produced in outBuf */
	uint8_t			ctx[CC_MAX_CTX_SIZE];	/* for CCCryptorCreateFromData() */
	uint8_t			*textMarker = NULL;		/* 8 bytes of marker here after expected end of 
											 * output */
	uint8_t			*ctxMarker = NULL;		/* ditto for caller-provided context */
	unsigned		dex;
	size_t			askedOutSize;			/* from the lib */
	size_t			thisOutLen;				/* dataOutAvailable we use */
	

	if(0) diag("%s %s %s keylen %d %s %s %s %s %s input length %ld\n", 
    	forEncrypt ? "Encrypting": "Decrypting", 
    	doCbc ? "CBC": "ECB", 
    	doPadding ? "Padding ON": "Padding OFF",
        (int) keyLen,
        iv ? "IV Provided": "No IV Provided",
		randUpdates ? "Random Updates": "Non-Random Updates", 
		inPlace ? "In Place": "Separate Buffers", 
    	ctxSize ? "We Allocate": "CC Allocated", 
    	askOutSize ? "Ask OutSize": "Don't Ask OutSize", 
		inTextLen);
         
	if(ctxSize > CC_MAX_CTX_SIZE) {
		diag("***HEY! Adjust CC_MAX_CTX_SIZE!\n");
		exit(1);
	}
	if(!doCbc) {
		options |= kCCOptionECBMode;
	}
	if(doPadding) {
		options |= kCCOptionPKCS7Padding;
	}
	
	/* just hack this one */
	outLen = inTextLen;
	if(forEncrypt) {
		outLen += MAX_BLOCK_SIZE;
	}
	
	outBuf = (uint8_t *)malloc(outLen + MARKER_LENGTH);
	memset(outBuf, 0xEE, outLen + MARKER_LENGTH);

	/* library should not touch this memory */
	textMarker = outBuf + outLen;
	memset(textMarker, MARKER_BYTE, MARKER_LENGTH);

	/* subsequent errors to errOut: */

	if(inPlace) {
		memmove(outBuf, inText, inTextLen);
		inp = outBuf;
	}
	else {
		inp = inText;
	}

	if(!randUpdates) {
		/* one shot */
		if(askOutSize) {
			crtn = CCCrypt(op, encrAlg, options,
				keyBytes, keyLen, iv,
				inp, inTextLen,
				outBuf, 0, &askedOutSize);
			if(crtn != kCCBufferTooSmall) {
				diag("***Did not get kCCBufferTooSmall as expected\n");
				diag("   alg %d inTextLen %lu cbc %d padding %d keyLen %lu\n",
					(int)encrAlg, (unsigned long)inTextLen, (int)doCbc, (int)doPadding,
					(unsigned long)keyLen);
				printCCError("CCCrypt", crtn);
				crtn = -1;
				goto errOut;
			}
			outLen = askedOutSize;
		}
		crtn = CCCrypt(op, encrAlg, options,
			keyBytes, keyLen, iv,
			inp, inTextLen,
			outBuf, outLen, &outLen);
		if(crtn) {
			printCCError("CCCrypt", crtn);
			goto errOut;
		}
		*outText = outBuf;
		*outTextLen = outLen;
		goto errOut;
	}
	
	/* random multi updates */
	if(ctxSize) {
		size_t ctxSizeCreated;
		
		if(askOutSize) {
			crtn = CCCryptorCreateFromData(op, encrAlg, options,
				keyBytes, keyLen, iv,
				ctx, 0 /* ctxSize */,
				&cryptor, &askedOutSize);
			if(crtn != kCCBufferTooSmall) {
				diag("***Did not get kCCBufferTooSmall as expected\n");
				printCCError("CCCryptorCreateFromData", crtn);
				crtn = -1;
				goto errOut;
			}
			ctxSize = askedOutSize;
		}
		crtn = CCCryptorCreateFromData(op, encrAlg, options,
			keyBytes, keyLen, iv,
			ctx, ctxSize, &cryptor, &ctxSizeCreated);
		if(crtn) {
			printCCError("CCCryptorCreateFromData", crtn);
			return crtn;
		}
		ctxMarker = ctx + ctxSizeCreated;
		memset(ctxMarker, MARKER_BYTE, MARKER_LENGTH);
	}
	else {
		crtn = CCCryptorCreate(op, encrAlg, options,
			keyBytes, keyLen, iv,
			&cryptor);
		if(crtn) {
			printCCError("CCCryptorCreate", crtn);
			return crtn;
		}
	}
	
	toMove = inTextLen;		/* total to go */
	outp = outBuf;
	outBytes = 0;			/* bytes actually produced in outBuf */
	
	while(toMove) {
		size_t thisMoveIn;			/* input to CCryptUpdate() */
		
		thisMoveIn = (size_t) genRand(1, (unsigned int) toMove);
		logSize(("###ptext segment len %lu\n", (unsigned long)thisMoveIn)); 
		if(askOutSize) {
			thisOutLen = CCCryptorGetOutputLength(cryptor, thisMoveIn, false);
		}
		else {
			thisOutLen = outLen;
		}
		crtn = CCCryptorUpdate(cryptor, inp, thisMoveIn,
			outp, thisOutLen, &thisMoveOut);
		if(crtn) {
			printCCError("CCCryptorUpdate", crtn);
			goto errOut;
		}
		inp			+= thisMoveIn;
		toMove		-= thisMoveIn;
		outp		+= thisMoveOut;
		outLen   	-= thisMoveOut;
		outBytes	+= thisMoveOut;
	}
	
	if(doPadding) {
		/* Final is not needed if padding is disabled */
		if(askOutSize) {
			thisOutLen = CCCryptorGetOutputLength(cryptor, 0, true);
		}
		else {
			thisOutLen = outLen;
		}
		crtn = CCCryptorFinal(cryptor, outp, thisOutLen, &thisMoveOut);
	}
	else {
		thisMoveOut = 0;
		crtn = kCCSuccess;
	}
	
	if(crtn) {
		printCCError("CCCryptorFinal", crtn);
		goto errOut;
	}
	
	outBytes += thisMoveOut;
	*outText = outBuf;
	*outTextLen = outBytes;
	crtn = kCCSuccess;

	for(dex=0; dex<MARKER_LENGTH; dex++) {
		if(textMarker[dex] != MARKER_BYTE) {
			diag("***lib scribbled on our textMarker memory (op=%s)!\n",
				forEncrypt ? "encrypt" : "decrypt");
			crtn = (CCCryptorStatus)-1;
		}
	}
	if(ctxSize) {
		for(dex=0; dex<MARKER_LENGTH; dex++) {
			if(ctxMarker[dex] != MARKER_BYTE) {
				diag("***lib scribbled on our ctxMarker memory (op=%s)!\n",
					forEncrypt ? "encrypt" : "decrypt");
				crtn = (CCCryptorStatus)-1;
			}
		}
	}
	
errOut:
	if(crtn) {
		if(outBuf) {
			free(outBuf);
		}
	}
	if(cryptor) {
		CCCryptorRelease(cryptor);
	}
	return crtn;
}

static int doTest(const uint8_t *ptext,
	size_t ptextLen,
	CCAlgorithm encrAlg,			
	bool doCbc,
	bool doPadding,
	bool nullIV,			/* if CBC, use NULL IV */
	uint32_t keySizeInBytes,
	bool stagedEncr,
	bool stagedDecr,
	bool inPlace,	
	size_t ctxSize,		
	bool askOutSize,
	__unused bool quiet)
{
	uint8_t			keyBytes[MAX_KEY_SIZE];
	uint8_t			iv[MAX_BLOCK_SIZE];
	uint8_t			*ivPtrEncrypt;
	uint8_t			*ivPtrDecrypt;
	uint8_t			*ctext = NULL;		/* mallocd by doCCCrypt */
	size_t			ctextLen = 0;
	uint8_t			*rptext = NULL;		/* mallocd by doCCCrypt */
	size_t			rptextLen = 0;
	CCCryptorStatus	crtn;
	int				rtn = 0;
	
	/* random key */
	appGetRandomBytes(keyBytes, keySizeInBytes);
	
	/* random IV if needed */
	if(doCbc) {
		if(nullIV) {
			cc_clear(MAX_BLOCK_SIZE, iv);

			/* flip a coin, give one side NULL, the other size zeroes */
			if(genRand(1,2) == 1) {
				ivPtrEncrypt = NULL;
				ivPtrDecrypt = iv;
			}
			else {
				ivPtrEncrypt = iv;
				ivPtrDecrypt = NULL;
			}
		}
		else {
			appGetRandomBytes(iv, MAX_BLOCK_SIZE);
			ivPtrEncrypt = iv;
			ivPtrDecrypt = iv;
		}
	}	
	else {
		ivPtrEncrypt = NULL;
		ivPtrDecrypt = NULL;
	}

	crtn = doCCCrypt(true, encrAlg, doCbc, doPadding,
		keyBytes, keySizeInBytes, ivPtrEncrypt,
		stagedEncr, inPlace, ctxSize, askOutSize,
		ptext, ptextLen,
		&ctext, &ctextLen);
    
    ok(crtn == 0, "doCCCrypt");
	if(crtn) {
        diag("Test Failure Encrypt encrAlg = %d dodCbc = %d doPadding %d\n", encrAlg, doCbc, doPadding);
	}
	
	logSize(("###ctext len %lu\n", ctextLen)); 
	crtn = doCCCrypt(false, encrAlg, doCbc, doPadding,
		keyBytes, keySizeInBytes, ivPtrDecrypt,
		stagedDecr, inPlace, ctxSize, askOutSize,
		ctext, ctextLen,
		&rptext, &rptextLen);
    ok(crtn == 0, "doCCCrypt");
	if(crtn) {
        diag("Test Failure Encrypt encrAlg = %d dodCbc = %d doPadding %d\n", encrAlg, doCbc, doPadding);
	}

	logSize(("###rptext len %lu\n", rptextLen)); 
	
	/* compare ptext, rptext */
	if(ptextLen != rptextLen) {
		diag("Ptext length mismatch: expect %lu, got %lu\n", ptextLen, rptextLen);
	} else  if(memcmp(ptext, rptext, ptextLen)) {
		diag("***data miscompare\n");
	}

	if(ctext) {
		free(ctext);
	}
	if(rptext) {
		free(rptext);
	}
	return rtn;
}

static bool isBitSet(unsigned bit, unsigned word) 
{
	if(bit > 31) {
		diag("We don't have that many bits\n");
		exit(1);
	}
	unsigned mask = 1 << bit;
	return (word & mask) ? true : false;
}

static int kTestTestCount = 7001;


#if defined (_WIN32)
#include <conio.h>
static void purge_stdin(){
    while (_kbhit())
        _getch();
}
#else
static void purge_stdin(){
    fpurge(stdin);
}
#endif

int CommonCryptoSymRegression(int __unused argc, char *const * __unused argv)
{
	unsigned			loop;
	uint8_t				*ptext;
	size_t				ptextLen;
	bool				stagedEncr = false;
	bool				stagedDecr = false;
	bool				doPadding;
	bool				doCbc = false;
	bool				nullIV;
	const char			*algStr;
	CCAlgorithm			encrAlg;	
	int					i;
	SymAlg					currAlg;		// ALG_xxx
	uint32_t				minKeySizeInBytes;
	uint32_t				maxKeySizeInBytes;
	uint32_t				keySizeInBytes = 0;
	int					rtn = 0;
	uint32_t				blockSize;		// for noPadding case
	size_t				ctxSize;		// always set per alg
	size_t				ctxSizeUsed;	// passed to doTest
	bool				askOutSize;		// inquire output size each op
	
	/*
	 * User-spec'd params
	 */
	bool		keySizeSpec = false;		// false: use rand key size
	SymAlg		minAlg = ALG_FIRST;
	SymAlg		maxAlg = ALG_LAST;
	unsigned	loops = LOOPS_DEF;
	bool		verbose = false;
	size_t		minPtextSize = MIN_DATA_SIZE;
	size_t		maxPtextSize = MAX_DATA_SIZE;
	bool		quiet = true;
	unsigned	pauseInterval = 0;
	bool		paddingSpec = false;		// true: user calls doPadding, const
	bool		cbcSpec = false;			// ditto for doCbc
	bool		stagedSpec = false;			// ditto for stagedEncr and stagedDecr
	bool		inPlace = false;			// en/decrypt in place for ECB
	bool		allocCtxSpec = false;		// use allocCtx
	bool		allocCtx = false;			// allocate context ourself

	plan_tests(kTestTestCount);
	

	ptext = (uint8_t *)malloc(maxPtextSize);
	if(ptext == NULL) {
		diag("Insufficient heap space\n");
		exit(1);
	}
	/* ptext length set in test loop */
	
	if(!quiet) diag("Starting ccSymTest; args: ");
	for(i=1; i<argc; i++) {
		if(!quiet) diag("%s ", argv[i]);
	}
	if(!quiet) diag("\n");
	
	if(pauseInterval) {
        purge_stdin();
		diag("Top of test; hit CR to proceed: ");
		getchar();
	}

	for(currAlg=minAlg; currAlg<=maxAlg; currAlg++) {
		switch(currAlg) {
			case ALG_DES:
				encrAlg = kCCAlgorithmDES;
				blockSize = kCCBlockSizeDES;
				minKeySizeInBytes = kCCKeySizeDES;
				maxKeySizeInBytes = minKeySizeInBytes;
				ctxSize = kCCContextSizeDES;
				algStr = "DES";
                if(verbose) diag("Running DES Tests");
				break;
			case ALG_3DES:
				encrAlg = kCCAlgorithm3DES;
				blockSize = kCCBlockSize3DES;
				minKeySizeInBytes = kCCKeySize3DES;
				maxKeySizeInBytes = minKeySizeInBytes;
				ctxSize = kCCContextSize3DES;
				
				algStr = "3DES";
                if(verbose) diag("Running 3DES Tests");
				break;
			case ALG_AES_128:
				encrAlg = kCCAlgorithmAES128;
				blockSize = kCCBlockSizeAES128;
				minKeySizeInBytes = kCCKeySizeAES128;
				maxKeySizeInBytes = minKeySizeInBytes;
				ctxSize = kCCContextSizeAES128;
				algStr = "AES128";
                if(verbose) diag("Running AES (128 bit key) Tests");
				break;
			case ALG_AES_192:
				encrAlg = kCCAlgorithmAES128;
				blockSize = kCCBlockSizeAES128;
				minKeySizeInBytes = kCCKeySizeAES192;
				maxKeySizeInBytes = minKeySizeInBytes;
				ctxSize = kCCContextSizeAES128;
				algStr = "AES192";
                if(verbose) diag("Running AES (192 bit key) Tests");
				break;
			case ALG_AES_256:
				encrAlg = kCCAlgorithmAES128;
				blockSize = kCCBlockSizeAES128;
				minKeySizeInBytes = kCCKeySizeAES256;
				maxKeySizeInBytes = minKeySizeInBytes;
				ctxSize = kCCContextSizeAES128;
				algStr = "AES256";
                if(verbose) diag("Running AES (256 bit key) Tests");
				break;
			case ALG_CAST:
				encrAlg = kCCAlgorithmCAST;
				blockSize = kCCBlockSizeCAST;
				minKeySizeInBytes = kCCKeySizeMinCAST;
				maxKeySizeInBytes = kCCKeySizeMaxCAST;
				ctxSize = kCCContextSizeCAST;
				algStr = "CAST";
                if(verbose) diag("Running CAST Tests");
				break;
			case ALG_RC4:
				encrAlg = kCCAlgorithmRC4;
				blockSize = 0;
				minKeySizeInBytes = kCCKeySizeMinRC4;
				maxKeySizeInBytes = kCCKeySizeMaxRC4;
				ctxSize = kCCContextSizeRC4;
				algStr = "RC4";
                if(verbose) diag("Running RC4 Tests");
				break;
			default:
				diag("***BRRZAP!\n");
				exit(1);
		}
		if(!quiet || verbose) {
			diag("Testing alg %s\n", algStr);
		}
		for(loop=1; ; loop++) {
			ptextLen = (size_t) genRand((unsigned int) minPtextSize, (unsigned int) maxPtextSize);
			appGetRandomBytes(ptext, ptextLen);
			
			/* per-loop settings */
			if(!keySizeSpec) {
				if(minKeySizeInBytes == maxKeySizeInBytes) {
					keySizeInBytes = minKeySizeInBytes;
				}
				else {
					keySizeInBytes = genRand(minKeySizeInBytes, maxKeySizeInBytes);
				}
			}
			if(blockSize == 0) {
				/* stream cipher */
				doCbc = false;
				doPadding = false;
			}
			else {
				if(!cbcSpec) {
					doCbc = isBitSet(0, loop);
				}
				if(!paddingSpec) {
					doPadding = isBitSet(1, loop);
				}
			}
			if(!doPadding && (blockSize != 0)) {
				/* align plaintext */
				ptextLen = (ptextLen / blockSize) * blockSize;
				if(ptextLen == 0) {
					ptextLen = blockSize;
				}
			}
			if(!stagedSpec) {
				stagedEncr = isBitSet(2, loop);
				stagedDecr = isBitSet(3, loop);
			}
			if(doCbc) {
				nullIV = isBitSet(4, loop);
			}
			else {
				nullIV = false;
			}
			inPlace = isBitSet(5, loop);
			if(allocCtxSpec) {
				ctxSizeUsed = allocCtx ? ctxSize : 0;
			}
			else if(isBitSet(6, loop)) {
				ctxSizeUsed = ctxSize;
			}
			else {
				ctxSizeUsed = 0;
			}
			askOutSize = isBitSet(7, loop);
			if(!quiet) {
			   	if(verbose || ((loop % LOOP_NOTIFY) == 0)) {
					diag("..loop %3d ptextLen %lu keyLen %d cbc=%d padding=%d stagedEncr=%d "
							"stagedDecr=%d\n",
						loop, (unsigned long)ptextLen, (int)keySizeInBytes, 
						(int)doCbc, (int)doPadding,
					 	(int)stagedEncr, (int)stagedDecr);
					diag("           nullIV %d inPlace %d ctxSize %d askOutSize %d\n",
						(int)nullIV, (int)inPlace, (int)ctxSizeUsed, (int)askOutSize);
				}
			}
			
			if(doTest(ptext, ptextLen,
					encrAlg, doCbc, doPadding, nullIV,
					keySizeInBytes,
					stagedEncr,	stagedDecr, inPlace, ctxSizeUsed, askOutSize,
					quiet)) {
				rtn = 1;
				break;
			}
			if(pauseInterval && ((loop % pauseInterval) == 0)) {
				int c;
                purge_stdin();
				diag("Hit CR to proceed, q to abort: ");
				c = getchar();
				if(c == 'q') {
					goto testDone;
				}
			}
			if(loops && (loop == loops)) {
				break;
			}
		}	/* main loop */
		if(rtn) {
			break;
		}
		
	}	/* for algs */
	
testDone:

    ok(rtn == 0, "ccSymTest");

	if(pauseInterval) {
        purge_stdin();
		diag("ModuleDetach/Unload complete; hit CR to exit: ");
		getchar();
	}
	if((rtn == 0) && !quiet) {
		diag("%s test complete\n", argv[0]);
	}
	free(ptext);
	return rtn;
}
#endif
