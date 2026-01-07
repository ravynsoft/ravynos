/*
 * Copyright (c) 2012 Apple, Inc. All Rights Reserved.
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

#ifndef _CC_COMMON_CRC_
#define _CC_COMMON_CRC_

#if defined(_MSC_VER)
#include <availability.h>
#else
#include <os/availability.h>
#endif


#if !defined(COMMON_NUMERICS_H)
#include <CommonNumerics/CommonNumerics.h>
#endif

typedef struct _CNCRCRef_t *CNCRCRef;

enum {
    kCN_CRC_8 = 10,
    kCN_CRC_8_ICODE = 11,
    kCN_CRC_8_ITU = 12,
    kCN_CRC_8_ROHC = 13,
    kCN_CRC_8_WCDMA = 14,
    kCN_CRC_16 = 20,
    kCN_CRC_16_CCITT_TRUE = 21,
    kCN_CRC_16_CCITT_FALSE = 22,
    kCN_CRC_16_USB = 23,
    kCN_CRC_16_XMODEM = 24,
    kCN_CRC_16_DECT_R = 25,
    kCN_CRC_16_DECT_X = 26,
    kCN_CRC_16_ICODE = 27,
    kCN_CRC_16_VERIFONE = 28,
    kCN_CRC_16_A = 29,
    kCN_CRC_16_B = 30,
    kCN_CRC_16_Fletcher = 31,
    kCN_CRC_32_Adler = 40,
    kCN_CRC_32 = 41,
    kCN_CRC_32_CASTAGNOLI = 42,
    kCN_CRC_32_BZIP2 = 43,
    kCN_CRC_32_MPEG_2 = 44,
    kCN_CRC_32_POSIX = 45,
    kCN_CRC_32_XFER = 46,
    kCN_CRC_64_ECMA_182 = 60,
};
typedef uint32_t CNcrc;

/*!
 @function   CNCRC
 @abstract   One-shot CRC function.

 @param      algorithm  Designates the CRC algorithm to use.
 @param      in         The data to be checksummed.
 @param      len        The length of the data to be checksummed.
 @param      result     The resulting checksum.

 @result     Possible error returns are kCNParamError and kCNUnimplemented.
 */

CNStatus
CNCRC(CNcrc algorithm, const void *in, size_t len, uint64_t *result)
API_AVAILABLE(macos(10.9), ios(6.0));

/*!
 @function   CNCRCInit
 @abstract   Initialize a CNCRCRef.

 @param      algorithm  Designates the CRC algorithm to use.
 @param      crcRef     The resulting CNCRCRef.

 @result     Possible error returns are kCNParamError, kCNMemoryFailure and kCNUnimplemented.
 */

CNStatus
CNCRCInit(CNcrc algorithm, CNCRCRef *crcRef)
API_AVAILABLE(macos(10.9), ios(6.0));

/*!
 @function   CNCRCRelease
 @abstract   Release a CNCRCRef.

 @param      crcRef     The CNCRCRef to release.

 @result     kCNSuccess is always returned.
 */

CNStatus
CNCRCRelease(CNCRCRef crcRef)
API_AVAILABLE(macos(10.9), ios(6.0));

/*!
 @function   CNCRCUpdate
 @abstract   Process data through the CRC function.  This can be called multiple times
             with a valid crcRef created using CNCRCInit().

 @param      crcRef     The CNCRCRef to use.
 @param      in         The data to be checksummed.
 @param      len        The length of the data to be checksummed.

 @result     Possible error return is kCNParamError.
 */

CNStatus
CNCRCUpdate(CNCRCRef crcRef, const void *in, size_t len)
API_AVAILABLE(macos(10.9), ios(6.0));

/*!
 @function   CNCRCFinal
 @abstract   Process remaining data through the CRC function and return the resulting checksum.
 @param      crcRef     The CNCRCRef to use.
 @param      result     The resulting checksum.

 @result     Possible error return is kCNParamError.
 */

CNStatus
CNCRCFinal(CNCRCRef crcRef, uint64_t *result)
API_AVAILABLE(macos(10.9), ios(6.0));

/*!
 @function   CNCRCWeakTest
 @abstract   Perform a "weak" test of a checksum.
 @param      algorithm  Designates the CRC algorithm to test.

 @result     Possible error return is kCNFailure.
 */

CNStatus
CNCRCWeakTest(CNcrc algorithm)
API_AVAILABLE(macos(10.9), ios(6.0));

CNStatus
CNCRCDumpTable(CNcrc algorithm)
API_AVAILABLE(macos(10.9), ios(6.0));

#endif /* _CC_COMMON_CRC_ */
