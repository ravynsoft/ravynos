/* 
 * Copyright (c) 2004-2010 Apple, Inc. All Rights Reserved.
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

/*
 * CommonDigestPriv.h - private typedefs and defines for ComonCrypto digest routines
 */
 
#ifndef	_COMMON_DIGEST_PRIV_H_
#define _COMMON_DIGEST_PRIV_H_

#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>

// This has to fit in 1032 bytes for static context clients - until we move them.
typedef struct ccDigest_s {
    const struct ccdigest_info *di;
    uint8_t            md[512];
} CCDigestCtx_t, *CCDigestCtxPtr;


// This should remain internal only.  This bridges the CommonCrypto->corecrypto structures

const struct ccdigest_info *
CCDigestGetDigestInfo(CCDigestAlgorithm algorithm);

#endif	/* _COMMON_DIGEST_PRIV_H_ */
