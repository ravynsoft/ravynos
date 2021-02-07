/* NSDecimal types and functions
   Copyright (C) 1998 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: November 1998

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   */

#ifndef __NSDecimal_h_GNUSTEP_BASE_INCLUDE
#define __NSDecimal_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import <GNUstepBase/GSConfig.h>

#import	<Foundation/NSObject.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#if	USE_GMP
#include <gmp.h>
#endif


#if	defined(__cplusplus)
extern "C" {
#endif

/**
 *  Enumerated type for specifying decimal rounding behavior.  Can be one of
 *  <code>NSRoundDown</code> (always round down), <code>NSRoundUp</code>
 *  (always round up), <code>NSRoundPlain</code> ("normal" rounding (up from
 *  .5 or above, down otherwise), <code>NSRoundBankers</code> (as "Plain" but
 *  .5 rounds to make last remaining digit even).  See the
 *  [(NSDecimalNumberBehaviors)] protocol.
 */
enum {
  NSRoundPlain,		/* Round .5 up		*/
  NSRoundDown,
  NSRoundUp,
  NSRoundBankers	/* Make last digit even	*/
};
typedef NSUInteger NSRoundingMode;

/**
 *  Enumerated type for specifying a decimal calculation error.  Can be one of
 *  the following:
 *  <deflist>
 *  <term><code>NSCalculationNoError</code></term>
 *  <desc>No error occurred.</desc>
 *  <term><code>NSCalculationLossOfPrecision</code></term>
 *  <desc>The number can't be represented in 38 significant digits.</desc>
 *  <term><code>NSCalculationOverflow</code></term>
 *  <desc>The number is too large to represent.</desc>
 *  <term><code>NSCalculationUnderflow</code></term>
 *  <desc>The number is too small to represent.</desc>
 *  <term><code>NSCalculationDivideByZero</code></term>
 *  <desc>The caller tried to divide by 0.</desc>
 *  </deflist>
 */
enum {
  NSCalculationNoError = 0,
  NSCalculationLossOfPrecision,
  NSCalculationUnderflow,	/* result became zero */
  NSCalculationOverflow,
  NSCalculationDivideByZero
};
typedef NSUInteger  NSCalculationError;

/**
 *	Give a precision of at least 38 decimal digits
 *	requires 128 bits.
 */
#define NSDecimalMaxSize (16/sizeof(mp_limb_t))

#define NSDecimalMaxDigit 38
#define NSDecimalNoScale 128

/**
 *  <p>Structure providing equivalent functionality, in conjunction with a set
 *  of functions, to the [NSDecimalNumber] class.</p>
<example>
typedef struct {
  signed char	exponent;   // Signed exponent - -128 to 127
  BOOL	isNegative;         // Is this negative?
  BOOL	validNumber;        // Is this a valid number?
  unsigned char	length;	    // digits in mantissa.
  unsigned char  cMantissa[2*NSDecimalMaxDigit];
} NSDecimal;
</example>
* <p>Instances can be initialized using the NSDecimalFromString(NSString *)
* function.</p>
 */
typedef struct {
  signed char	exponent;	/* Signed exponent - -128 to 127	*/
  BOOL	isNegative;	/* Is this negative?			*/
  BOOL	validNumber;	/* Is this a valid number?		*/
#if	USE_GMP
  mp_size_t size;
  mp_limb_t lMantissa[NSDecimalMaxSize];
#else
  unsigned char	length;		/* digits in mantissa.			*/
  unsigned char cMantissa[NSDecimalMaxDigit];
#endif
} NSDecimal;

/** Returns whether decimal represents an invalid number (i.e., an "NaN" as
    might result from an overflow or a division by zero). */
static inline BOOL
NSDecimalIsNotANumber(const NSDecimal *decimal)
{
  return (decimal->validNumber == NO);
}

/** Copies value of decimal number to preallocated destination. */
GS_EXPORT void
NSDecimalCopy(NSDecimal *destination, const NSDecimal *source);

/** Tries to reduce memory used to store number internally. */
GS_EXPORT void
NSDecimalCompact(NSDecimal *number);

/**
 *  Returns <code>NSOrderedDescending</code>, <code>NSOrderedSame</code>, or
 *  <code>NSOrderedAscending</code> depending on whether leftOperand is
 *  greater than, equal to, or less than rightOperand.
 */
GS_EXPORT NSComparisonResult
NSDecimalCompare(const NSDecimal *leftOperand, const NSDecimal *rightOperand);

/**
 *  Rounds number to result such that it has at most scale digits to the right
 *  of its decimal point, according to mode (see the
 *  [(NSDecimalNumberBehaviors)] protocol).  The result should be preallocated
 *  but can be the same as number.
 */
GS_EXPORT void
NSDecimalRound(NSDecimal *result, const NSDecimal *number, NSInteger scale,
  NSRoundingMode mode);

/**
 *  Sets the exponents of n1 and n2 equal to one another, adjusting mantissas
 *  as necessary to preserve values.  This makes certain operations quicker.
 */
GS_EXPORT NSCalculationError
NSDecimalNormalize(NSDecimal *n1, NSDecimal *n2, NSRoundingMode mode);

/**
 *  Adds two decimals and returns result to 38-digit precision.  See the
 *  [(NSDecimalNumberBehaviors)] protocol for a description of mode and the
 *  return value.  The result should be preallocated but can be the same as
 *  left or right.
 */
GS_EXPORT NSCalculationError
NSDecimalAdd(NSDecimal *result, const NSDecimal *left, const NSDecimal *right, NSRoundingMode mode);

/**
 *  Subtracts two decimals and returns result to 38-digit precision.  See the
 *  [(NSDecimalNumberBehaviors)] protocol for a description of mode and the
 *  return value.  The result should be preallocated but can be the same as
 *  left or right.
 */
GS_EXPORT NSCalculationError
NSDecimalSubtract(NSDecimal *result, const NSDecimal *left, const NSDecimal *right, NSRoundingMode mode);

/**
 *  Multiplies two decimals and returns result to 38-digit precision.  See the
 *  [(NSDecimalNumberBehaviors)] protocol for a description of mode and the
 *  return value.  The result should be preallocated but can be the same as
 *  l or r.
 */
GS_EXPORT NSCalculationError
NSDecimalMultiply(NSDecimal *result, const NSDecimal *l, const NSDecimal *r, NSRoundingMode mode);

/**
 *  Divides l by rr and returns result to 38-digit precision.  See the
 *  [(NSDecimalNumberBehaviors)] protocol for a description of mode and the
 *  return value.  The result should be preallocated but can be the same as
 *  l or rr.
 */
GS_EXPORT NSCalculationError
NSDecimalDivide(NSDecimal *result, const NSDecimal *l, const NSDecimal *rr, NSRoundingMode mode);
    
/**
 *  Raises n to power and returns result to 38-digit precision.  See the
 *  [(NSDecimalNumberBehaviors)] protocol for a description of mode and the
 *  return value.  The result should be preallocated but can be the same as
 *  n or power.
 */
GS_EXPORT NSCalculationError
NSDecimalPower(NSDecimal *result, const NSDecimal *n, NSUInteger power,
  NSRoundingMode mode);

/**
 *  Multiplies n by 10^power and returns result to 38-digit precision.  See the
 *  [(NSDecimalNumberBehaviors)] protocol for a description of mode and the
 *  return value.  The result should be preallocated but can be the same as
 *  n.
 */
GS_EXPORT NSCalculationError
NSDecimalMultiplyByPowerOf10(NSDecimal *result, const NSDecimal *n, short power, NSRoundingMode mode);

/**
 *  Returns a string representing the full decimal value, formatted according
 *  to locale (send nil here for default locale).
 */
GS_EXPORT NSString*
NSDecimalString(const NSDecimal *decimal, NSDictionary *locale);


// GNUstep extensions to make the implementation of NSDecimalNumber totaly 
// independent for NSDecimals internal representation

/** Give back the biggest NSDecimal in (preallocated) result. */
GS_EXPORT void
NSDecimalMax(NSDecimal *result);

/** Give back the smallest NSDecimal in (preallocated) result. */
GS_EXPORT void
NSDecimalMin(NSDecimal *result);

/** Give back the value of a NSDecimal as a double in (preallocated) result. */
GS_EXPORT double
NSDecimalDouble(NSDecimal *number);

/**
 *  Create a NSDecimal with a mantissa, exponent and a negative flag in
 *  (preallocated) result.
 */
GS_EXPORT void
NSDecimalFromComponents(NSDecimal *result, unsigned long long mantissa, 
		      short exponent, BOOL negative);

/**
 *  Create a NSDecimal from a string using the locale, in (preallocated)
 *  result.
 */
GS_EXPORT void
NSDecimalFromString(NSDecimal *result, NSString *numberValue, 
		    NSDictionary *locale);

#if	defined(__cplusplus)
}
#endif

#endif
#endif

