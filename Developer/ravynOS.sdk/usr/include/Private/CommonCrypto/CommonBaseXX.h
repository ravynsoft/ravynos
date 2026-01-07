/*
 * Copyright (c) 2011 Apple Inc. All Rights Reserved.
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


#ifndef COMMON_BASE_XX_H
#define COMMON_BASE_XX_H

#if defined(_MSC_VER)
#include <availability.h>
#else
#include <os/availability.h>
#endif


#if !defined(COMMON_NUMERICS_H)
#include <CommonNumerics/CommonNumerics.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
    @enum       CNEncodings
    @abstract   Encodings available through CommonBaseXX().

    @constant   kCNEncodingBase64   		Base64 Encoding.
    @constant   kCNEncodingBase32       	Base32 Encoding.
    @constant   kCNEncodingBase32HEX        Base32 Encoding -
    @constant   kCNEncodingBase32Recovery   Base32 Simplified Encoding.
*/
enum {
    kCNEncodingBase64   			= 0x0001,
	kCNEncodingBase32   			= 0x0002,
    kCNEncodingBase32Recovery       = 0x0003,
    kCNEncodingBase32HEX            = 0x0004,
    kCNEncodingBase16               = 0x0005,
    kCNEncodingCustom               = 0xcafe
};
typedef uint32_t CNEncodings;

/*!
 @enum       kCNEncodingDirection
 @abstract   Determine whether the CNEncoderRef is to be used
             to encode or decode.

 @constant   kCNEncode   		Encode (base256 to baseXX)
 @constant   kCNDecode       	Decode (baseXX to base256)
 */

enum {
    kCNEncode            = 0x0001,
    kCNDecode            = 0x0002,
};
typedef uint32_t CNEncodingDirection;

/*!
    @typedef    CNEncoderRef
    @abstract   Opaque reference to a CNEncoder object.
 */

typedef struct _CNEncoder *CNEncoderRef;

/*!
    @function   CNEncode
    @abstract   One-Shot baseXX encode or decode.
    @param      encoding selects one of the base encodings above.
    @param      direction   Designate the direction (encode or decode)
    @param      in          The bytes to be processed.
    @param		inLen		The number of bytes to be processed.
    @param		out			The destination of the processed data.
    @param		outLen		The length of the processed data.
    @result     kCCSuccess or one of kCCParamErr, kCCMemoryFailure.

 */

CNStatus
CNEncode(CNEncodings encoding,
                  CNEncodingDirection direction,
                  const void *in, const size_t inLen,
                  void *out,  size_t *outLen)
API_AVAILABLE(macos(10.9), ios(5.0));




/*!
    @function   CCEncoderCreate
    @abstract   Create a base encoder context.
    @param      encoding    selects one of the base encodings above.
    @param      direction   Designate the direction (encode or decode) for this
                            CNEncoderRef.
    @param      encoderRef  A (required) pointer to the returned CNEncoderRef.
*/

CNStatus
CNEncoderCreate(CNEncodings encoding,
                         CNEncodingDirection direction,
                         CNEncoderRef *encoderRef)  /* RETURNED */
API_AVAILABLE(macos(10.9), ios(5.0));

/*!
    @function   CCEncoderCreateCustom
    @abstract   Create a custom base encoder context.
    @param      name        A name for this encoder (optional)
    @param      baseNum     The base of the encoding (16, 32, 64)
    @param      charMap     A string containing the characters to map an encoded
                            byte to for output.
    @param      padChar     The character to use for padding (usually '=')
    @param      direction   Designate the direction (encode or decode) for this
                            CNEncoderRef.
    @param      coderRef  A (required) pointer to the returned CNEncoderRef.
 */

CNStatus
CNEncoderCreateCustom(const void *name,
                      const uint8_t baseNum,
                      const void *charMap,
                      const char padChar,
                      CNEncodingDirection direction,
                      CNEncoderRef *coderRef)  /* RETURNED */
API_AVAILABLE(macos(10.9), ios(5.0));

/*!
     @function   CNEncoderRelease
     @abstract   Release a CNEncoderRef and associated objects.
 */
CNStatus
CNEncoderRelease(CNEncoderRef *coderRef)
API_AVAILABLE(macos(10.9), ios(5.0));


/*!
    @function   CNEncoderGetOutputLength
    @abstract   Determine the size required to hold the result of processing the
				input.

    @param      coderRef    A CNEncoderRef obtained through CNEncoderCreate()
                            or CNEncoderCreateCustom().

	@param		inLen		The number of bytes to be processed.

    @result     The length required for the encoding.  Zero (0) will be returned in
				the result of a NULL input pointer for the "in" parameter.
*/

size_t
CNEncoderGetOutputLength(CNEncoderRef coderRef, const size_t inLen)
API_AVAILABLE(macos(10.9), ios(6.0));

/*!
     @function   CNEncoderGetOutputLengthFromEncoding
     @abstract   Determine the size required to hold the result of processing the
                 input given an encoding constant and direction and length.

     @param      encoding    selects one of the base encodings above.
     @param      direction   Designate the direction (encode or decode) for this
                             CNEncoderRef.

     @param		 inLen		The number of bytes to be processed.

     @result     The length required for the encoding.  Zero (0) will be returned in
                 the result of a NULL input pointer for the "in" parameter.
 */

size_t
CNEncoderGetOutputLengthFromEncoding(CNEncodings encoding,
                                         CNEncodingDirection direction,
                                         const size_t inLen)
API_AVAILABLE(macos(10.9), ios(6.0));

/*!
    @function   CNEncoderUpdate
    @abstract   Encode or decode the input string to or from the designated
                encoded format.

	@param      coderRef  A CNEncoderRef obtained through CNEncoderCreate()
                            or CNEncoderCreateCustom().

	@param      in          The bytes to be processed.

	@param		inLen		The number of bytes to be processed.

	@param		out			The destination of the processed data.

	@param		outLen		The length of the processed data.

    @result     kCCSuccess or one of kCCParamErr, kCCMemoryFailure.
*/

CNStatus
CNEncoderUpdate(CNEncoderRef coderRef, const void *in, const size_t inLen, void *out,
         size_t *outLen)
API_AVAILABLE(macos(10.9), ios(6.0));


/*!
 @function   CNEncoderFinal
 @abstract   Complete coding for all available inputs, padding where necessary.

 @param      coderRef  A CNEncoderRef obtained through CNEncoderCreate()
                        or CNEncoderCreateCustom().

 @param		out			The destination of the processed data.

 @param		outLen		The length of the processed data.

 @result     kCCSuccess or one of kCCParamErr, kCCMemoryFailure.
 */

CNStatus
CNEncoderFinal(CNEncoderRef coderRef, void *out, size_t *outLen)
API_AVAILABLE(macos(10.9), ios(6.0));


/*!
 @function   CNEncoderBlocksize
 @abstract   Get the number of bytes per block of input (base256) to output (base of encoder).

 @param      encoding    The encoding format.

 @param      inputSize   The number of raw bytes upon which an encoding operation should be performed
                         if padding should not be added.  If CNEncode is called with this many bytes
                         the resulting coded bytes will not need padding.
 @param      outputSize  The output block size in bytes for this encoding.

 @result     kCCSuccess or kCCParamErr.
 */



CNStatus
CNEncoderBlocksize(CNEncodings encoding, size_t *inputSize, size_t *outputSize)
API_AVAILABLE(macos(10.9), ios(6.0));

/*!
 @function   CNEncoderBlocksizeFromRef
 @abstract   Get the number of bytes per block of input (base256) to output (base of encoder).

 @param      encoderRef  A CNEncoderRef gotten from CNEncoderCreate or CNEncoderCreateCustom.

 @param      inputSize   The number of raw bytes upon which an encoding operation should be performed
                        if padding should not be added.  If CNEncode is called with this many bytes
                        the resulting coded bytes will not need padding.
 @param      outputSize  The output block size in bytes for this encoding.

 @result     kCCSuccess or kCCParamErr.
 */

CNStatus
CNEncoderBlocksizeFromRef(CNEncoderRef encoderRef, size_t *inputSize, size_t *outputSize)
API_AVAILABLE(macos(10.9), ios(6.0));


#ifdef __cplusplus
}
#endif

#endif /* COMMON_BASE_XX_H */
