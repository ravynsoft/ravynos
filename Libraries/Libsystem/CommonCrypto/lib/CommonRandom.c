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

// #define COMMON_RANDOM_FUNCTIONS
#include <CommonCrypto/CommonRandomSPI.h>
#include "ccDispatch.h"
#include <corecrypto/ccaes.h>
#include <corecrypto/ccdrbg.h>
#include <corecrypto/ccrng.h>
#include "ccGlobals.h"
#include "ccErrors.h"
#include "ccdebug.h"

/* These values are ignored, but we have to keep them around for binary compatibility */
static const int ccRandomDefaultStruct;

const CCRandomRef kCCRandomDefault = &ccRandomDefaultStruct;
const CCRandomRef kCCRandomDevRandom = &ccRandomDefaultStruct;

/*
  We don't use /dev/random anymore, use the corecrypto rng instead.
*/
struct ccrng_state *
ccDRBGGetRngState(void)
{
    int status;
    struct ccrng_state *rng = ccrng(&status);
    CC_DEBUG_LOG("ccrng returned %d\n", status);
    return rng;
}

struct ccrng_state *
ccDevRandomGetRngState(void)
{
    return ccDRBGGetRngState();
}

int CCRandomCopyBytes(CCRandomRef rnd, void *bytes, size_t count)
{
    (void) rnd;

    return CCRandomGenerateBytes(bytes, count);
}

CCRNGStatus CCRandomGenerateBytes(void *bytes, size_t count)
{
    int err;
    struct ccrng_state *rng;

    if (0 == count) {
        return kCCSuccess;
    }

    if (NULL == bytes) {
        return kCCParamError;
    }

    rng = ccDRBGGetRngState();
    err = ccrng_generate(rng, count, bytes);
    if (err == CCERR_OK) {
        return kCCSuccess;
    }

    return kCCRNGFailure;
}

CCRNGStatus CCRandomUniform(uint64_t bound, uint64_t *rand)
{
    int err;
    struct ccrng_state *rng;

    rng = ccDRBGGetRngState();
    err = ccrng_uniform(rng, bound, rand);
    if (err == CCERR_OK) {
        return kCCSuccess;
    }

    return kCCRNGFailure;
}
