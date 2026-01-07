/*
 * Copyright (c) 2012 Apple Inc. All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

// #define COMMON_KEYDERIVATION_FUNCTIONS

#include <CommonCrypto/CommonKeyDerivation.h>
#include <corecrypto/ccpbkdf2.h>
#include "CommonDigestPriv.h"
#include <CommonCrypto/CommonDigestSPI.h>
#include "ccdebug.h"
#include "ccGlobals.h"

int 
CCKeyDerivationPBKDF( CCPBKDFAlgorithm algorithm, const char *password, size_t passwordLen,
					 const uint8_t *salt, size_t saltLen,
					 CCPseudoRandomAlgorithm prf, unsigned rounds,
					 uint8_t *derivedKey, size_t derivedKeyLen)
{
    const struct ccdigest_info *di;

    CC_DEBUG_LOG("PasswordLen %lu SaltLen %lU PRF %d Rounds %u DKLen %lu\n", passwordLen, saltLen, prf, rounds, derivedKeyLen);
    if(algorithm != kCCPBKDF2) return kCCParamError;
    switch(prf) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        case kCCPRFHmacAlgSHA1: di = CCDigestGetDigestInfo(kCCDigestSHA1); break;
#pragma clang diagnostic pop
        case kCCPRFHmacAlgSHA224: di = CCDigestGetDigestInfo(kCCDigestSHA224); break;
        case kCCPRFHmacAlgSHA256: di = CCDigestGetDigestInfo(kCCDigestSHA256); break;
        case kCCPRFHmacAlgSHA384: di = CCDigestGetDigestInfo(kCCDigestSHA384); break;
        case kCCPRFHmacAlgSHA512: di = CCDigestGetDigestInfo(kCCDigestSHA512); break;
        default: return kCCParamError;
    }
    if(!password || !derivedKey || (derivedKeyLen == 0) || (rounds == 0)) return kCCParamError;
    if(salt==NULL && saltLen!=0) return kCCParamError;
    
    int rc = ccpbkdf2_hmac(di, passwordLen, password, saltLen, salt, rounds, derivedKeyLen, derivedKey);
    return rc==0?kCCSuccess:kCCParamError ;
}

//time functions are from corecrypto
#if defined(_WIN32)
#include <windows.h>
static uint64_t absolute_time(void) {
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time); //resolution < 1us
    return (uint64_t)time.QuadPart;
}

static uint64_t to_msec(uint64_t at){
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq); //performance counter freq in Hz
    double msec = (double)at / freq.QuadPart / 1000;
    return (uint64_t) msec;
}
#elif defined(__ANDROID__)
#include <corecrypto/cc_absolute_time.h>
#define absolute_time cc_absolute_time
static uint64_t to_msec(uint64_t at){
    double msec = at / (uint64_t)1000000;
    return (uint64_t)msec;
}
#else
#include <mach/mach_time.h>
#define absolute_time() (mach_absolute_time())
static uint64_t to_msec(uint64_t at){
    struct mach_timebase_info info;
    mach_timebase_info(&info);
    double msec =  (double)at * info.numer / info.denom / 1000000;
    return (uint64_t) msec;
}
#endif

#define ROUNDMEASURE 100000
// This is for the scratchspace - it's twice the size of the max PRF buffer + 4 to work within the pbkdf2 code we currently have.
#define CC_MAX_PRF_WORKSPACE 128+4
#define CC_MIN_PBKDF2_ITERATIONS 10000

unsigned CCCalibratePBKDF(CCPBKDFAlgorithm algorithm, size_t passwordLen, size_t saltLen,
				 CCPseudoRandomAlgorithm prf, size_t derivedKeyLen, uint32_t msec)
{
	char        *password;
	uint8_t     *salt=NULL, *derivedKey=NULL;
	uint64_t	startTime, elapsedTime;
	size_t       i;
    int retval = -1;
    
    CC_DEBUG_LOG("Entering\n");
	if (derivedKeyLen == 0) return -1; // bad parameters
	if (saltLen > CC_MAX_PRF_WORKSPACE) return -1; // out of bounds parameters
	if (passwordLen == 0 ) passwordLen = 1;
	if(algorithm != kCCPBKDF2) return -1;
    
	if((password = malloc(passwordLen)) == NULL) goto error;
	for(i=0; i<passwordLen; i++) password[i] = 'a';
    
    size_t saltLen2 = saltLen==0? 1: saltLen;
    
    if((salt=malloc(saltLen2)) == NULL ) goto error;
    for(i=0; i<saltLen2; i++) salt[i] = (uint8_t)(i%256);
    
	if((derivedKey = malloc(derivedKeyLen)) == NULL) goto error;
    
    for(elapsedTime=i=0; i < 5 && elapsedTime == 0; i++) {
        startTime = absolute_time();
        if(CCKeyDerivationPBKDF(algorithm, password, passwordLen, salt, saltLen, prf, ROUNDMEASURE, derivedKey, derivedKeyLen)) goto error;
        elapsedTime = absolute_time() - startTime;
	}
    
    if(elapsedTime == 0){
        retval = CC_MIN_PBKDF2_ITERATIONS; // something is seriously wrong
        goto error;
    }
    
    retval = CC_MAX(CC_MIN_PBKDF2_ITERATIONS,(int)((msec * ROUNDMEASURE)/to_msec(elapsedTime)));
error:
	free(password);
	free(salt);
	free(derivedKey);
    
	return retval;
}

