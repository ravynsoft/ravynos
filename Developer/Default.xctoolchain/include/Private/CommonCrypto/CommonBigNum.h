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

#ifndef _CC_BIGNUM_H_
#define _CC_BIGNUM_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(_MSC_VER)
#include <availability.h>
#else
#include <os/availability.h>
#endif

#include <CommonCrypto/CommonCryptoError.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This is an SPI - it isn't intended to be generally used.  If you
 * intend to use this we strongly urge you to talk to someone in the
 * Information Security Group to see if there isn't an alternative
 * set of functions to implement your cryptographic needs.
 */

/*
 * This shares the error status of CommonCryptor.h
 */

typedef struct _CCBigNumRef *CCBigNumRef;

/*!
	@function   CCCreateBigNum
	@abstract   Creates a BigNum - must be freed later with
             	CCBigNumFree.

	@param      status  A pointer to a CCStatus for return codes.

	@result		On success this returns a newly
    			allocated BigNum (must be freed with
                CCBigNumFree).
                Returns NULL on failure.
 */

CCBigNumRef
CCCreateBigNum(CCStatus *status)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumClear
	@abstract   Zeroes (clears) a BigNum.

	@param      bn The BigNum to clear.
	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumClear(CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumFree
	@abstract   Frees and clears a BigNum.

	@param      bn The BigNum to free.

 */

void
CCBigNumFree(CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumCopy
	@abstract   Copies a BigNum.

	@param      status  A pointer to a CCStatus for return codes.
	@param      bn The BigNum to copy.
	@result		On success this returns a newly
    			allocated BigNum (must be freed with
                CCBigNumFree).
                Returns NULL on failure.
 */

CCBigNumRef
CCBigNumCopy(CCStatus *status, const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumBitCount
	@abstract   Returns the number of significant bits
    			in a BigNum.

	@param      bn The BigNum.
	@result		Returns number of bits.

 */

uint32_t
CCBigNumBitCount(const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumZeroLSBCount
	@abstract   Returns the number of zero bits
    			before the least significant 1 bit.

	@param      bn The BigNum.
	@result		Returns number of bits.

 */

uint32_t
CCBigNumZeroLSBCount(const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumByteCount
	@abstract   Returns the number of bytes if
    			converted to binary data.

	@param      bn The BigNum.
	@result		Returns number of bytes.

 */

uint32_t
CCBigNumByteCount(const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumFromData
	@abstract   Creates a BigNum from binary data.

	@param      status  A pointer to a CCStatus for return codes.
	@param      s - the data pointer.  The data is expected to be
    			an array of octets in big endian format.
	@param      len - the length of the data.
	@result		On success this returns a newly
    			allocated BigNum (must be freed with
                CCBigNumFree).
                Returns NULL on failure.
 */

CCBigNumRef
CCBigNumFromData(CCStatus *status, const void *s, size_t len)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumToData
	@abstract   Dumps a BigNum into binary data.

	@param      status  A pointer to a CCStatus for return codes.
	@param      bn The BigNum.
	@param      to - the pointer to the data area.
    			You can use CCBigNumByteCount() to
                determine the size of the data area
                to provide.
	@result     Returns the length of the data area.

 */

size_t
CCBigNumToData(CCStatus *status, const CCBigNumRef bn, void *to)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumFromHexString
	@abstract   Creates a BigNum from a hexadecimal string.

	@param      status  A pointer to a CCStatus for return codes.
	@param      in - a null terminated hexadecimal string.
	@result		On success this returns a newly
    			allocated BigNum (must be freed with
                CCBigNumFree).
                Returns NULL on failure.
 */

CCBigNumRef
CCBigNumFromHexString(CCStatus *status, const char *in)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumToHexString
	@abstract   Dumps a BigNum into hexadecimal string.

	@param      status  A pointer to a CCStatus for return codes.
	@param      bn The BigNum.
	@result     Returns a hexadecimal string representation
    			of the BigNum.  This must be freed by the caller.
                Returns NULL on failure.

 */

char *
CCBigNumToHexString(CCStatus *status, const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));

/*!
 @function   CCBigNumFromDecimalString
 @abstract   Creates a BigNum from a decimal string.

 @param      status  A pointer to a CCStatus for return codes.
 @param      in - a null terminated decimal string.
 @result		On success this returns a newly
 allocated BigNum (must be freed with
 CCBigNumFree).
 Returns NULL on failure.
 */

CCBigNumRef
CCBigNumFromDecimalString(CCStatus *status, const char *in)
API_AVAILABLE(macos(10.9), ios(6.0));


/*!
 @function   CCBigNumToDecimalString
 @abstract   Dumps a BigNum into a decimal string.

 @param      status  A pointer to a CCStatus for return codes.
 @param      bn The BigNum.
 @result     Returns a decimal string representation
 of the BigNum.  This must be freed by the caller.
 Returns NULL on failure.

 */

char *
CCBigNumToDecimalString(CCStatus *status, const CCBigNumRef bn)
API_AVAILABLE(macos(10.9), ios(6.0));


/*!
	@function   CCBigNumByteCount
	@abstract   Returns the number of bytes that will result from
    			converting a BigNum to octets.

	@param      bn The BigNum.
	@result		The number of bytes.
 */

uint32_t
CCBigNumByteCount(const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumCompare
	@abstract   Compares two BigNums.

	@param      bn1 - a BigNum.
	@param      bn2 - a BigNum.
	@result		Returns -1 if bn1 is less than bn2.
                Returns 0 if bn1 and bn2 are equal.
                Returns 1 if bn1 is greater than bn2.

 */

int
CCBigNumCompare(const CCBigNumRef bn1, const CCBigNumRef bn2)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumCompareI
	@abstract   Compares a BigNum and a 32 bit integer.

	@param      bn1 - a BigNum.
	@param      num - an integer.
	@result		Returns -1 if bn1 is less than num.
                Returns 0 if bn1 and num are equal.
                Returns 1 if bn1 is greater than num.

 */


int
CCBigNumCompareI(const CCBigNumRef bn1, const uint32_t num)
API_AVAILABLE(macos(10.8), ios(5.0));


/*!
	@function   CCBigNumSetNegative
	@abstract   Negates a BigNum.

	@param      bn - a BigNum.
	@result		returns a CCStatus (See CommonCryptor.h for values).

 */

CCStatus
CCBigNumSetNegative(CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumSetI
	@abstract   Sets a BigNum value using an unsigned integer.

	@param      bn The BigNum.
	@param      num The value to set.
	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumSetI(CCBigNumRef bn, uint64_t num)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumGetI
	@abstract   Get an unsigned integer representation of the BigNum.
                This assumes the BigNum can actually fit within the
                unsigned integer representation.

	@param      status  A pointer to a CCStatus for return codes.
	@param      bn The BigNum.
	@result		returns the unsigned integer value.
 */

uint32_t
CCBigNumGetI(CCStatus *status, const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumAdd
	@abstract   Adds two BigNums.

	@param		result  A bigNum in which to place the result.
    @param		a		The first BigNum to add.
    @param		b		The second BigNum to add.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumAdd(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef b)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumAddI
	@abstract   Adds a BigNum and an unsigned integer.

	@param		result  A bigNum in which to place the result.
    @param		a		The first BigNum to add.
    @param		b		The unsigned integer to add.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumAddI(CCBigNumRef result, const CCBigNumRef a, const uint32_t b)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumSub
	@abstract   Subtracts a BigNum from a BigNum.

	@param		result  A bigNum in which to place the result.
    @param		a		The BigNum.
    @param		b		The BigNum to subtract.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumSub(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef b)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumSubI
	@abstract   Subtracts an unsigned integer from a BigNum.

	@param		result  A bigNum in which to place the result.
    @param		a		The BigNum.
    @param		b		The unsigned integer to subtract.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumSubI(CCBigNumRef result, const CCBigNumRef a, const uint32_t b)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumMul
	@abstract   Multiplies two BigNums.

	@param		result  A bigNum in which to place the result.
    @param		a		The first BigNum to multiply.
    @param		b		The second BigNum to multiply.

    @result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumMul(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef b)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumMulI
	@abstract   Multiplies a BigNum and an unsigned integer.

	@param		result  A bigNum in which to place the result.
    @param		a		The first BigNum to multiply.
    @param		b		The unsigned integer to multiply.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumMulI(CCBigNumRef result, const CCBigNumRef a, const uint32_t b)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumDiv
	@abstract   Divides a BigNum (a) by another Bignum (b).

	@param		quotient  A bigNum in which to place the quotient (a div b).
	@param		remainder  A bigNum in which to place the remainder (a mod b).
    @param		a		The BigNum to divide.
    @param		b		The BigNum used to divide a.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumDiv(CCBigNumRef quotient, CCBigNumRef remainder, const CCBigNumRef a, const CCBigNumRef b)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumDiv2
	@abstract   Divides a BigNum (a) by 2.

	@param		result  A bigNum in which to place the result.
    @param		a		The BigNum to divide.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumDiv2(CCBigNumRef result, const CCBigNumRef a)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumMod
	@abstract   Find the remainder of a BigNum for a given modulus.

	@param		result  A bigNum in which to place the result.
    @param		dividend	The BigNum to divide.
    @param		modulus		The BigNum used to divide a and produce the mod value.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumMod(CCBigNumRef result, CCBigNumRef dividend, CCBigNumRef modulus)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumModI
	@abstract   Find the remainder of a BigNum for a given modulus (unsigned integer version).

	@param		result      A pointer to an unsigned integer in which to place the result.
    @param		dividend	The BigNum to divide.
    @param		modulus		The BigNum used to divide a and produce the mod value.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumModI(uint32_t *result, CCBigNumRef dividend, uint32_t modulus)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumSquare
	@abstract   Find the square of a BigNum.

	@param		result  A bigNum in which to place the result.
    @param		a	The BigNum to square.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumSquare(CCBigNumRef result, const CCBigNumRef a)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumGCD
	@abstract   Find the Greatest Common Denominator of two BigNums.

	@param		result  A bigNum in which to place the result.
    @param		a	A BigNum.
    @param		b	A BigNum.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumGCD(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef b)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumLCM
	@abstract   Find the Least Common Multiple of two BigNums.

	@param		result  A bigNum in which to place the result.
    @param		a	A BigNum.
    @param		b	A BigNum.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumLCM(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef b)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumMulMod
	@abstract   Perform Modular Multiplication.

	@param		result  A bigNum in which to place the result.
    @param		a	A BigNum.
    @param		b	A BigNum.
    @param		modulus	The Modulus.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumMulMod(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef b, const CCBigNumRef modulus)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumSquareMod
	@abstract   Perform Modular Squaring.

	@param		result  A bigNum in which to place the result.
    @param		a	A BigNum.
    @param		modulus	The Modulus.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumSquareMod(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef modulus)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumInverseMod
	@abstract   Perform Modular Inversion.

	@param		result  A bigNum in which to place the result.
    @param		a		A BigNum.
    @param		modulus	The Modulus.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumInverseMod(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef modulus)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumModExp
	@abstract   Perform Modular Exponentiation.

	@param		result  A bigNum in which to place the result.
    @param		a		The base integer.
    @param		power	The power integer.
    @param		modulus	The Modulus.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumModExp(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef power, const CCBigNumRef modulus)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumLeftShift
	@abstract   Shift a BigNum left

	@param		result  A bigNum in which to place the result.
	@param      a		The BigNum.
	@param      digits	How many bit places to shift left.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumLeftShift(CCBigNumRef result, const CCBigNumRef a, const uint32_t digits)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumRightShift
	@abstract   Shift a BigNum right

	@param		result  A bigNum in which to place the result.
	@param      a		The BigNum.
	@param      digits	How many bit places to shift right.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumRightShift(CCBigNumRef result, const CCBigNumRef a, const uint32_t digits)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumMontgomerySetup
	@abstract   Setup Montgomery

	@param      num	The modulus.
	@param      rho	The destination for the reduction digit.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumMontgomerySetup(CCBigNumRef num, uint32_t *rho)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumMontgomeryNormalization
	@abstract   Get the normalization value.

	@param		result  A bigNum in which to place the result.
	@param      modulus	The modulus.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumMontgomeryNormalization(CCBigNumRef result, CCBigNumRef modulus)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumMontgomeryReduce
	@abstract   Reduce a number

	@param      x 	The BigNum to reduce - the result will be stored back into
    				this BigNum.
	@param      modulus	The modulus.
	@param      rho		The reduction digit.

	@result		returns a CCStatus (See CommonCryptor.h for values).
 */

CCStatus
CCBigNumMontgomeryReduce(CCBigNumRef x, CCBigNumRef modulus, uint32_t rho)
API_AVAILABLE(macos(10.8), ios(5.0));



/*!
	@function   CCBigNumCreateRandom
	@abstract   Creates a BigNum with a random value.
    ZZZZZ

	@param      status  A pointer to a CCStatus for return codes.
	@result		On success this returns a newly
    			allocated BigNum (must be freed with
                CCBigNumFree).
                Returns NULL on failure.
 */

CCBigNumRef
CCBigNumCreateRandom(CCStatus *status, int bits, int top, int bottom)
API_AVAILABLE(macos(10.8), ios(5.0));

/*!
	@function   CCBigNumIsPrime
	@abstract   Determines if a BigNum is prime.

	@param      status  A pointer to a CCStatus for return codes.
	@param      bn - a BigNum.
	@result		Returns true or false.
 */

bool
CCBigNumIsPrime(CCStatus *status, const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));

/*!
	@function   CCBigNumIsOdd
	@abstract   Determines if a BigNum is odd.

	@param      status  A pointer to a CCStatus for return codes.
	@param      bn - a BigNum.
	@result		Returns true or false.
 */

bool
CCBigNumIsOdd(CCStatus *status, const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));

/*!
	@function   CCBigNumIsZero
	@abstract   Determines if a BigNum is zero.

	@param      status  A pointer to a CCStatus for return codes.
	@param      bn - a BigNum.
	@result		Returns true or false.
 */

bool
CCBigNumIsZero(CCStatus *status, const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));

/*!
	@function   CCBigNumIsNegative
	@abstract   Determines if a BigNum is negative.

	@param      status  A pointer to a CCStatus for return codes.
	@param      bn - a BigNum.
	@result		Returns true or false.
 */

bool
CCBigNumIsNegative(CCStatus *status, const CCBigNumRef bn)
API_AVAILABLE(macos(10.8), ios(5.0));

#ifdef __cplusplus
}
#endif

#endif  /* _CC_BIGNUM_H_ */
