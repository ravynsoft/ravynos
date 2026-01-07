/*
 * Copyright (c) 2011 Apple Computer, Inc. All Rights Reserved.
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


#ifndef	_CC_CMACSPI_H_
#define _CC_CMACSPI_H_

#include <stdint.h>
#include <sys/types.h>

#if defined(_MSC_VER)
#include <availability.h>
#else
#include <os/availability.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define CC_CMACAES_DIGEST_LENGTH     16          /* CMAC length in bytes - copy and paste error - */

#define CC_CMACAES_OUTPUT_LENGTH     16          /* CMAC length in bytes */

/*!
    @function   CCAESCmac
    @abstract   Stateless, one-shot AES CMAC function

    @param      key         Raw key bytes.
    @param      data        The data to process.
    @param      dataLength  The length of the data to process.
    @param      macOut      The MAC bytes (space provided by the caller).
                            Output is written to caller-supplied buffer.

    @discussion The length of the MAC written to *macOut is 16
                The MAC must be verified by comparing the computed and expected values
                using timingsafe_bcmp. Other comparison functions (e.g. memcmp)
                must not be used as they may be vulnerable to practical timing attacks,
                leading to MAC forgery.
*/

void
CCAESCmac(const void *key, const uint8_t *data, size_t dataLength, void *macOut)
API_AVAILABLE(macos(10.7), ios(6.0));


typedef struct CCCmacContext * CCCmacContextPtr;


/*!
    @function   CCAESCmacCreate
    @abstract   Create a CMac context.

    @param      key         The bytes of the AES key.
    @param      keyLength   The length (in bytes) of the AES key.

    @discussion This returns an AES-CMac context to be used with
                CCAESCmacUpdate(), CCAESCmacFinal() and CCAESCmacDestroy().
 */

CCCmacContextPtr
CCAESCmacCreate(const void *key, size_t keyLength)
API_AVAILABLE(macos(10.10), ios(8.0));

/*!
    @function   CCAESCmacUpdate
    @abstract   Process some data.

    @param      ctx         An HMAC context.
    @param      data        Data to process.
    @param      dataLength  Length of data to process, in bytes.

    @discussion This can be called multiple times.
 */

void CCAESCmacUpdate(CCCmacContextPtr ctx, const void *data, size_t dataLength)
API_AVAILABLE(macos(10.10), ios(8.0));


/*!
    @function   CCAESCmacFinal
    @abstract   Obtain the final Message Authentication Code.

    @param      ctx         A CMAC context.
    @param      macOut      Destination of MAC; allocated by caller.

    @discussion The length of the MAC written to *macOut is 16
         The MAC must be verified by comparing the computed and expected values
         using timingsafe_bcmp. Other comparison functions (e.g. memcmp)
         must not be used as they may be vulnerable to practical timing attacks,
         leading to MAC forgery.
*/

void CCAESCmacFinal(CCCmacContextPtr ctx, void *macOut)
API_AVAILABLE(macos(10.10), ios(8.0));

void
CCAESCmacDestroy(CCCmacContextPtr ctx)
API_AVAILABLE(macos(10.10), ios(8.0));

size_t
CCAESCmacOutputSizeFromContext(CCCmacContextPtr ctx)
API_AVAILABLE(macos(10.10), ios(8.0));


#ifdef __cplusplus
}
#endif

#endif /* _CC_CMACSPI_H_ */
