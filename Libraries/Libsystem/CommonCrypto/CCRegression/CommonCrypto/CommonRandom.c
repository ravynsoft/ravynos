/*
 * Copyright (c) 2010 Apple Inc. All Rights Reserved.
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

#include <corecrypto/ccrng_system.h>
/*
 *  randomTest
 *  CommonCrypto
 */
#include "testmore.h"
#include "capabilities.h"
#include "testbyteBuffer.h"

#if (CCRANDOM == 0)
entryPoint(CommonRandom,"Random Number Generation")
#else
#include <CommonCrypto/CommonRandomSPI.h>

static const int kTestTestCount = 1000;
static const int bufmax = kTestTestCount + 16;

static double chisq(unsigned ncells, uint64_t *cells)
{
    unsigned i;
    double v = 0;
    uint64_t sum = 0;
    double Oi, Ei, vi;

    for (i = 0; i < ncells; i += 1) {
        sum += cells[i];
    }

    Ei = (double)sum / ncells;

    for (i = 0; i < ncells; i += 1) {
        Oi = cells[i];
        vi = Oi - Ei;
        vi *= vi;
        vi /= Ei;
        v += vi;
    }

    return v;
}

static void CommonRandomUniform(void)
{
    unsigned bound_nbits;
    uint64_t bound_mask;
    uint64_t bound;
    uint64_t rand;
    uint64_t cells[3] = {};
    unsigned i;
    double v;

    // Test some basic edge-case bounds.
    isnt(CCRandomUniform(0, &rand), kCCSuccess, "reject bound = 0");
    isnt(CCRandomUniform(1, &rand), kCCSuccess, "reject bound = 1");
    is(CCRandomUniform(2, &rand), kCCSuccess, "accept bound = 2");
    ok(rand < 2, "rand out of range (bound = 2)");
    is(CCRandomUniform(UINT64_MAX, &rand), kCCSuccess, "accept bound = UINT64_MAX");
    ok(rand < UINT64_MAX, "rand out of range (bound = UINT64_MAX)");

    // Test with random bounds of random lengths.
    for (i = 0; i < (1 << 10);) {
        CCRandomGenerateBytes(&bound_nbits, sizeof(bound_nbits));
        bound_nbits &= 0x3f;    /* bound_nbits in [0, 63] */
        bound_nbits += 1;       /* bound_nbits in [1, 64] */
        if (bound_nbits == 1) {
            continue;
        }

        /* bound_nbits in [2, 64] */
        bound_mask = (~0ULL) >> (64 - bound_nbits);

        CCRandomGenerateBytes(&bound, sizeof(bound));
        bound &= bound_mask;
        if (bound < 2) {
            continue;
        }

        is(CCRandomUniform(bound, &rand), kCCSuccess, "accept bound = %llu", bound);
        ok(rand < bound, "rand out of range (bound = %llu)", bound);

        i += 1;
    }

    // This is a very weak statistical test designed to catch
    // catastrophic failures. It should not report false positives. It
    // does not attempt to do a rigorous statistical analysis of the
    // generated distribution.
    //
    // from R:
    //
    // > qchisq(1 - 2^-32, df=2)
    // [1] 44.36142
    //
    // If the null hypothesis (i.e. the function generates a uniform
    // distribution) is true, we should see a test statistic greater
    // than 44.36142 with probability 2^-32.
    //
    // In other words, if we fail this check, something is badly
    // broken. This is a basic sanity test and no more.

    for (i = 0; i < (1 << 20); i += 1) {
        is(CCRandomUniform(3, &rand), kCCSuccess, "accept bound = 3");
        ok(rand < 3, "rand out of range (bound = 3)");

        cells[rand] += 1;
    }

    v = chisq(3, cells);
    ok(v < 44.36142, "chi-squared: reject null hypothesis");
}

int CommonRandom(int __unused argc, char *const * __unused argv)
{
    int i;
    uint8_t buf1[bufmax], buf2[bufmax], buf3[bufmax], buf4[bufmax], buf5[bufmax], buf6[bufmax];

	plan_tests(kTestTestCount * 12 +
               11 +
               2099145          /* CCRandomUniform tests */
               );

    struct ccrng_state *devRandom = NULL;
    struct ccrng_state *drbg = NULL;

    // ============================================
    //          Positive testing
    // ============================================
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    devRandom = ccDevRandomGetRngState();
    isnt(devRandom,NULL, "Dev random first state ok");
    drbg = ccDRBGGetRngState();
    isnt(drbg,NULL, "DRBG first state ok");
    if (devRandom==NULL || drbg==NULL) return 1;
    for(i=0; i < kTestTestCount; i++) {
        size_t len = i+16;
        is(CCRandomCopyBytes(kCCRandomDefault, buf1, len),0, "Random success");
        is(CCRandomCopyBytes(kCCRandomDevRandom, buf2, len),0, "Random success");
        is(CCRandomCopyBytes(NULL, buf3, len),0, "Random success");
        is(ccrng_generate(devRandom, len, buf4),0, "Random success");
        is(ccrng_generate(drbg, len, buf5),0, "Random success");
        is(CCRandomGenerateBytes(buf6, len),0, "Random success");

        ok(memcmp(buf1, buf2, len), "Buffers aren't the same");
        ok(memcmp(buf3, buf4, len), "Buffers aren't the same");
        ok(memcmp(buf2, buf3, len), "Buffers aren't the same");
        ok(memcmp(buf5, buf6, len), "Buffers aren't the same");
        ok(memcmp(buf5, buf2, len), "Buffers aren't the same");
        ok(memcmp(buf6, buf1, len), "Buffers aren't the same");
    }

    // Bad inputs
    is(CCRandomCopyBytes(kCCRandomDefault, buf1, 0),0, "Zero Length");
    is(CCRandomCopyBytes(kCCRandomDevRandom, buf2, 0),0, "Zero Length");
    is(CCRandomGenerateBytes(buf6, 0),0, "Zero Length");

    isnt(CCRandomCopyBytes(kCCRandomDefault, NULL, 1),0, "NULL pointer");
    isnt(CCRandomCopyBytes(kCCRandomDevRandom, NULL, 1),0, "NULL pointer");
    isnt(CCRandomGenerateBytes(NULL, 1),0, "NULL pointer");

    is(CCRandomCopyBytes(kCCRandomDefault, NULL, 0),0, "Zero Length, NULL pointer");
    is(CCRandomCopyBytes(kCCRandomDevRandom, NULL, 0),0, "Zero Length, NULL pointer");
    is(CCRandomGenerateBytes(NULL, 0),0, "Zero Length, NULL pointer");
#pragma clang diagnostic pop

    CommonRandomUniform();

    return 0;
}
#endif //CCRANDOM
