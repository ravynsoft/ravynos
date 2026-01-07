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

#ifndef	_CC_HmacSPI_H_
#define _CC_HmacSPI_H_

#include <CommonCrypto/CommonHMAC.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef CCHmacContext * CCHmacContextRef;

CCHmacContextRef
CCHmacCreate(CCDigestAlg alg, const void *key, size_t keyLength)
API_AVAILABLE(macos(10.7), ios(5.0));

/* Create a clone of an initialized CCHmacContextRef - you must do this before use.  */
CCHmacContextRef
CCHmacClone(CCHmacContextRef ctx)
API_AVAILABLE(macos(10.10), ios(7.0));

/* Update and Final are reused from existing api, type changed from struct CCHmacContext * to CCHmacContextRef though */

void
CCHmacDestroy(CCHmacContextRef ctx)
API_AVAILABLE(macos(10.7), ios(5.0));

size_t
CCHmacOutputSizeFromRef(CCHmacContextRef ctx)
API_AVAILABLE(macos(10.7), ios(5.0));


size_t
CCHmacOutputSize(CCDigestAlg alg)
API_AVAILABLE(macos(10.7), ios(5.0));

/*
 * Stateless, one-shot HMAC function using digest constants
 * Output is written to caller-supplied buffer, as in CCHmacFinal().

 *
 * The tag must be verified by comparing the computed and expected values
 * using timingsafe_bcmp. Other comparison functions (e.g. memcmp)
 * must not be used as they may be vulnerable to practical timing attacks,
 * leading to tag forgery.
 */
void CCHmacOneShot(
            CCDigestAlg alg,  /* kCCHmacAlgSHA1, kCCHmacAlgMD5 */
            const void *key,
            size_t keyLength,           /* length of key in bytes */
            const void *data,
            size_t dataLength,          /* length of data in bytes */
            void *macOut)               /* MAC written here */
API_AVAILABLE(macos(10.10), ios(7.0));



#ifdef __cplusplus
}
#endif

#endif /* _CC_HmacSPI_H_ */
