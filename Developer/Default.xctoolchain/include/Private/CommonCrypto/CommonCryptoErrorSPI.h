//
//  CommonCryptoErrorSPI.h
//  CommonCrypto
//
//  Copyright © 2018 Platform Security. All rights reserved.
//

#ifndef CommonCryptoErrorSPI_h
#define CommonCryptoErrorSPI_h

/*
 * Copyright (c) 2018 Apple Inc. All Rights Reserved.
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

#include <CommonCrypto/CommonCryptoError.h>

#if defined(__cplusplus)
extern "C" {
#endif

    /*!
     @enum       CCCryptorStatus
     @abstract   Return values from CommonCryptor operations.

     @constant   kCCNotVerified   Signature verification failed.
     */
    enum {
        /* -4350 and below reserved for SPI error codes. */
        kCCNotVerified = -4350,
    };

#if defined(__cplusplus)
}
#endif

#endif /* CommonCryptoErrorSPI_h */
