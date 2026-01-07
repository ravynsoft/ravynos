#ifndef  COMMONRANDOM_H
#define  COMMONRANDOM_H 1

/*
 *  CommonRandom.h
 *
 * Copyright © 2010-2011 by Apple, Inc. All rights reserved.
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
 *
 */

#include <stdint.h>
#include <sys/types.h>

#if defined(_MSC_VER)
#include <availability.h>
#else
#include <os/availability.h>
#endif

#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonRandom.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
    @typedef    CCRandomRef
    @abstract   Abstract Reference to a random number generator.

*/
typedef const void *CCRandomRef;

/*!
 @function      CCRandomCopyBytes

 @abstract      Return random bytes in a buffer allocated by the caller.

 @discussion    The default PRNG returns cryptographically strong random
                bits suitable for use as cryptographic keys, IVs, nonces etc.

 @param         rnd     Unused; should be set to @p NULL.
 @param         bytes   Pointer to the return buffer.
 @param         count   Number of random bytes to return.

 @result        Return kCCSuccess on success.
 */

int CCRandomCopyBytes(CCRandomRef rnd, void *bytes, size_t count)
API_DEPRECATED_WITH_REPLACEMENT("CCRandomGenerateBytes", macos(10.7, 10.15), ios(5.0, 13.0));

extern const CCRandomRef kCCRandomDefault
API_DEPRECATED_WITH_REPLACEMENT("CCRandomGenerateBytes", macos(10.7, 10.15), ios(5.0, 13.0));

extern const CCRandomRef kCCRandomDevRandom
API_DEPRECATED_WITH_REPLACEMENT("CCRandomGenerateBytes", macos(10.7, 10.15), ios(5.0, 13.0));

#include <corecrypto/ccrng.h>

struct ccrng_state *ccDevRandomGetRngState(void)
API_DEPRECATED_WITH_REPLACEMENT("ccDRBGGetRngState", macos(10.8, 10.12), ios(6.0, 10.0));

struct ccrng_state *ccDRBGGetRngState(void)
API_AVAILABLE(macos(10.8), ios(6.0));

/*!
  @function CCRandomUniform
  @abstract Generate a random value in @p [0, bound).

  @param bound The exclusive upper bound on the output.
  @param rand  A pointer to a single @p uint64_t to store the result.

  @result Returns kCCSuccess iff the operation is successful.
 */
CCRNGStatus CCRandomUniform(uint64_t bound, uint64_t *rand)
API_AVAILABLE(macos(10.15), ios(13.0));

#if defined(__cplusplus)
}
#endif

#endif /* COMMONRANDOM_H */
