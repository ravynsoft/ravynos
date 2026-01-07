/*
 * CommonRSACryptorSPI.h
 * Copyright (c) 2017 Apple Inc. All Rights Reserved.
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


#ifndef CommonRSACryptorSPI_h
#define CommonRSACryptorSPI_h

#include <CommonCrypto/CommonRSACryptor.h>

#ifdef __cplusplus
extern "C" {
#endif
    


// The following SPIs return the RSA CRT parameters
    
/*!
@function   CCRSAGetCRTComponentsSizes
@abstract   Returns the size of the RSA CRT components dp, dq, qinv, where dp = d (mod p) and dq = d (mod q) and qinv = q ^ -1 (mod p)
@param      rsaKey     A pointer to a CCRSACryptorRef.
@param      dpSize     Input pointer to return the size of the RSA CRT parameter dp.
 param      dqSize     Input pointer to return the size of the RSA CRT parameter dq.
@param      qinvSize   Input pointer to return the size of the RSA CRT parameter qinv.
@result                If the function is successful (kCCSuccess)
*/

CCCryptorStatus CCRSAGetCRTComponentsSizes(CCRSACryptorRef rsaKey, size_t *dpSize, size_t *dqSize, size_t *qinvSize);

/*!
 @function   CCRSAGetCRTComponents
 @abstract   Returns the RSA CRT components dp, dq and qinv in big endian form. The required size of dp, dq and qinv buffers can be obtained by calling to CCRSAGetCRTDpSize, CCRSAGetCRTDqSize and CCRSAGetCRTQinvSize SPIs, respectively
 @param      rsaKey     A pointer to a CCRSACryptorRef
 @param      dp         The buffer to return the RSA CRT parameter dp
 @param      dpSize     The size of the input buffer dp
 @param      dq       	The buffer to return the RSA CRT parameter dq
 @param      dqSize     The size of the input buffer dq
 @param      qinv       The buffer to return the RSA CRT parameter qinv
 @param      qinvSize   The size of the input buffer qinv
 @result                If the function is successful (kCCSuccess)
 */
CCCryptorStatus CCRSAGetCRTComponents(CCRSACryptorRef rsaKey, void *dp, size_t dpSize, void *dq, size_t dqSize, void *qinv, size_t qinvSize);


#ifdef __cplusplus
}
#endif


#endif /* CommonRSACryptorSPI_h */
