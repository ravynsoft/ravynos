/* Interface of NSDecimalNumber class
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

#ifndef __NSDecimalNumber_h_GNUSTEP_BASE_INCLUDE
#define __NSDecimalNumber_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#import	<Foundation/NSObject.h>
#import	<Foundation/NSDecimal.h>
#import	<Foundation/NSValue.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class	NSDecimalNumber;

/**
 *  This protocol encapsulates information about how an [NSDecimalNumber]
 *  should round and process exceptions.  Usually you can just create objects
 *  of the [NSDecimalNumberHandler] class, which implements this protocol, but
 *  if you don't want to use that class you can create your own implementing
 *  it.
 */
@protocol	NSDecimalNumberBehaviors

/**
 *  <p>Specifies behavior when, in the course of applying method to leftOperand
 *  and rightOperand, an [NSDecimalNumber] instance encounters given error.</p>
 *  <p>error has four possible constant values:</p>
 *  <deflist>
 *  <term><code>NSCalculationLossOfPrecision</code></term>
 *  <desc>The number can't be represented in 38 significant digits.</desc>
 *  <term><code>NSCalculationOverflow</code></term>
 *  <desc>The number is too large to represent.</desc>
 *  <term><code>NSCalculationUnderflow</code></term>
 *  <desc>The number is too small to represent.</desc>
 *  <term><code>NSCalculationDivideByZero</code></term>
 *  <desc>The caller tried to divide by 0.</desc>
 *  </deflist>
 *
 *  <p>Behavior on error can be one of the following:</p>
 *  <list>
 *  <item>Raise an exception.</item>
 *  <item>Return nil.  The calling method will return its value as though no
 *    error had occurred. If error is
 *    <code>NSCalculationLossOfPrecision</code>, method will return an
 *    imprecise value, constrained to 38 significant digits.  If error is
 *    <code>NSCalculationUnderflow</code> or
 *    <code>NSCalculationOverflow</code>, method will return
 *    <code>NSDecimalNumber</code>'s <code>notANumber</code>. You shouldn't
 *    return nil if error is <code>NSDivideByZero</code>.
 *  </item>
 *  <item>Correct the error and return a valid <code>NSDecimalNumber</code>.
 *    The calling method will use this as its own return value.</item>
 *  </list>
 */
- (NSDecimalNumber*) exceptionDuringOperation: (SEL)method 
					error: (NSCalculationError)error 
				  leftOperand: (NSDecimalNumber*)leftOperand 
				 rightOperand: (NSDecimalNumber*)rightOperand; 

/**
 *  Specifies how [NSDecimalNumber]'s <code>decimalNumberBy...</code> methods
 *  round their return values.  This should be set to one of the following
 *  constants:
 *  <deflist>
 *  <term><code>NSRoundDown</code></term>
 *  <desc>Always round down.</desc>
 *  <term><code>NSRoundUp</code></term>
 *  <desc>Always round up.</desc>
 *  <term><code>NSRoundPlain</code></term>
 *  <desc>Round to the closest possible return value.  Halfway (e.g. .5)
 *  rounds up for positive numbers, down for negative (towards larger absolute
 *  value).</desc>
 *  <term><code>NSRoundBankers</code></term>
 *  <desc>Round to the closest possible return value, but halfway (e.g. .5)
 *  rounds towards  possibility whose last digit is even.</desc>
 * </deflist>
 */
- (NSRoundingMode) roundingMode;

/**
 *  Specifies the precision of the values returned by [NSDecimalNumber]'s
 *  <code>decimalNumberBy...</code> methods, in terms of the number of
 *  digits allowed after the decimal point.  This can be negative, implying
 *  that the precision should be, e.g., 100's, 1000's, etc..  For unlimited
 *  precision, set to <code>NSDecimalNoScale</code>.
 */
- (short) scale;
@end

/**
 *  A utility class adopting [(NSDecimalNumberBehaviors)] protocol.  Can be used
 *  to control [NSDecimalNumber] rounding and exception-handling behavior, by
 *  passing an instance as an argument to any [NSDecimalNumber] method ending
 *  with <code>...Behavior:</code>.
 */
@interface	NSDecimalNumberHandler : NSObject <NSDecimalNumberBehaviors>
{
#if	GS_EXPOSE(NSDecimalNumberHandler)
  NSRoundingMode _roundingMode;
  short _scale;
  BOOL _raiseOnExactness;
  BOOL _raiseOnOverflow; 
  BOOL _raiseOnUnderflow;
  BOOL _raiseOnDivideByZero;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

/**
 *  Provides an instance implementing the default behavior for the
 *  [NSDecimalNumber] class.  38 decimal digits, rounded to closest return
 *  value (<code>NSRoundPlain</code>).  Exceptions raised on overflow,
 *  underflow, and divide by zero.
 */
+ (id)defaultDecimalNumberHandler;

/**
 * Constructor setting all behavior.  (For more precise control over error
 * handling, create your own class implementing the [(NSDecimalNumberBehaviors)]
 * protocol.)
 */
+ (id)decimalNumberHandlerWithRoundingMode:(NSRoundingMode)roundingMode 
				     scale:(short)scale
			  raiseOnExactness:(BOOL)raiseOnExactness 
			   raiseOnOverflow:(BOOL)raiseOnOverflow 
			  raiseOnUnderflow:(BOOL)raiseOnUnderflow
		       raiseOnDivideByZero:(BOOL)raiseOnDivideByZero;

/**
 * Initializer setting all behavior.  (For more precise control over error
 * handling, create your own class implementing the [(NSDecimalNumberBehaviors)]
 * protocol.)
 */
- (id)initWithRoundingMode:(NSRoundingMode)roundingMode 
		     scale:(short)scale 
	  raiseOnExactness:(BOOL)raiseOnExactness
	   raiseOnOverflow:(BOOL)raiseOnOverflow 
	  raiseOnUnderflow:(BOOL)raiseOnUnderflow
       raiseOnDivideByZero:(BOOL)raiseOnDivideByZero;
@end

/**
 *  <p>Class that implements a number of methods for performing decimal
 *  arithmetic to arbitrary precision. The behavior in terms of rounding
 *  choices and exception handling may be customized using the
 *  [NSDecimalNumberHandler] class, and defaults to
 *  [NSDecimalNumberHandler+defaultDecimalNumberHandler].</p>
 *
 *  <p>Equivalent functionality to the <code>NSDecimalNumber</code> class may
 *  be accessed through functions, mostly named <code>NSDecimalXXX</code>,
 *  e.g., NSDecimalMin().  Both the class and the functions use a structure
 *  called <code>NSDecimal</code>.</p>
 *
 *  <p>Note that instances of <code>NSDecimalNumber</code> are immutable.</p>
 */
@interface	NSDecimalNumber : NSNumber <NSDecimalNumberBehaviors>
{
#if	GS_EXPOSE(NSDecimalNumber)
  NSDecimal data;
#endif
}

/**
 *  Returns the default rounding/precision/exception handling behavior, which
 *  is same as [NSDecimalNumberHandler+defaultDecimalNumberHandler] unless it
 *  has been explicitly set otherwise.
 */
+ (id <NSDecimalNumberBehaviors>)defaultBehavior;

/**
 *  Sets the default rounding/precision/exception handling behavior to the
 *  given behavior.  If this is not called, behavior defaults to
 *  [NSDecimalNumberHandler+defaultDecimalNumberHandler].
 */
+ (void)setDefaultBehavior:(id <NSDecimalNumberBehaviors>)behavior;

/**
 *  Return maximum positive value that can be represented.
 */
+ (NSDecimalNumber *)maximumDecimalNumber;

/**
 *  Return minimum negative value (<em>not</em> the smallest positive value)
 *  that can be represented.
 */
+ (NSDecimalNumber *)minimumDecimalNumber;

/**
 *  Return a fixed value representing an NaN.
 */
+ (NSDecimalNumber *)notANumber;

/**
 *  Return a constant object with a value of one.
 */
+ (NSDecimalNumber *)one;

/**
 *  Return a constant object with a value of zero.
 */
+ (NSDecimalNumber *)zero;

/**
 *  New instance with given value.  Note an NSDecimal may be created using the
 *  function NSDecimalFromString().
 */
+ (NSDecimalNumber *)decimalNumberWithDecimal:(NSDecimal)decimal;

/**
 *  New instance by component.  Note that the precision of this initializer is
 *  limited.
 */
+ (NSDecimalNumber *)decimalNumberWithMantissa:(unsigned long long)mantissa 
				      exponent:(short)exponent
				    isNegative:(BOOL)isNegative;

/**
 *  New instance from string.  Arbitrary precision is preserved, though calling
 *  one of the <code>decimalNumberBy...</code> methods will return a result
 *  constrained by the current <em><code>scale</code></em>.  Number format
 *  is parsed according to current default locale.
 */
+ (NSDecimalNumber *)decimalNumberWithString:(NSString *)numericString;

/**
 *  New instance from string.  Arbitrary precision is preserved, though calling
 *  one of the <code>decimalNumberBy...</code> methods will return a result
 *  constrained by the current <em><code>scale</code></em>.  Number format
 *  is parsed according to given locale.
 */
+ (NSDecimalNumber *)decimalNumberWithString:(NSString *)numericString 
				      locale:(NSDictionary *)locale;

/**
 *  Initialize with given value.  Note an NSDecimal may be created using the
 *  function NSDecimalFromString().
 */
- (id)initWithDecimal:(NSDecimal)decimal;

/**
 *  Initialize by component.  Note that the precision of this initializer is
 *  limited.
 */
- (id)initWithMantissa:(unsigned long long)mantissa 
	      exponent:(short)exponent 
	    isNegative:(BOOL)flag;

/**
 *  Initialize from string.  Arbitrary precision is preserved, though calling
 *  one of the <code>decimalNumberBy...</code> methods will return a result
 *  constrained by the current <em><code>scale</code></em>.  Number format
 *  is parsed according to current default locale.
 */
- (id)initWithString:(NSString *)numberValue;

/**
 *  Initialize from string.  Arbitrary precision is preserved, though calling
 *  one of the <code>decimalNumberBy...</code> methods will return a result
 *  constrained by the current <em><code>scale</code></em>.  Number format
 *  is parsed according to given locale.
 */
- (id)initWithString:(NSString *)numberValue 
	      locale:(NSDictionary *)locale;

/**
 *  Returns the Objective-C type (<code>@encode(...)</code> compatible) of the
 *  data contained <code>NSDecimalNumber</code>, which is by convention "d"
 *  (for double), even though this is not strictly accurate.
 */
- (const char *)objCType;

/**
 *  Return underlying value as an <code>NSDecimal</code> structure.
 */
- (NSDecimal)decimalValue;

/**
 *  Returns string version of number formatted according to locale.
 */
- (NSString *)descriptionWithLocale:(id)locale;

/**
 *  Returns underlying value as a <code>double</code>, which may be an
 *  approximation if precision greater than the default of 38.
 */
- (double)doubleValue;


/**
 *  Compares with other number, returning less, greater, or equal.
 */
- (NSComparisonResult)compare:(NSNumber *)decimalNumber;

/**
 *  Adds self to decimalNumber and returns new result, using +defaultBehavior
 *  for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByAdding:(NSDecimalNumber *)decimalNumber;

/**
 *  Adds self to decimalNumber and returns new result, using given behavior
 *  for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByAdding:(NSDecimalNumber *)decimalNumber 
			      withBehavior:(id<NSDecimalNumberBehaviors>)behavior;

/**
 *  Divides self by decimalNumber and returns new result, using +defaultBehavior
 *  for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByDividingBy:(NSDecimalNumber *)decimalNumber;

/**
 *  Divides self by decimalNumber and returns new result, using given behavior
 *  for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByDividingBy:(NSDecimalNumber *)decimalNumber 
				  withBehavior:(id <NSDecimalNumberBehaviors>)behavior;

/**
 *  Multiplies self by decimalNumber and returns new result, using
 *  +defaultBehavior for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByMultiplyingBy:(NSDecimalNumber *)decimalNumber;

/**
 *  Multiplies self by decimalNumber and returns new result, using given
 *  behavior for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByMultiplyingBy:(NSDecimalNumber *)decimalNumber 
				     withBehavior:(id <NSDecimalNumberBehaviors>)behavior;

/**
 *  Multiplies self by given power of 10 and returns new result, using
 *  +defaultBehavior for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByMultiplyingByPowerOf10:(short)power;

/**
 *  Multiplies self by given power of 10 and returns new result, using given
 *  behavior for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByMultiplyingByPowerOf10:(short)power 
  withBehavior:(id <NSDecimalNumberBehaviors>)behavior;

/**
 *  Raises self to given positive integer power and returns new result, using
 *  +defaultBehavior for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByRaisingToPower:(NSUInteger)power;

/**
 *  Raises self to given positive integer power and returns new result, using
 *  given behavior for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberByRaisingToPower:(NSUInteger)power 
  withBehavior:(id <NSDecimalNumberBehaviors>)behavior;

/**
 *  Subtracts decimalNumber from self and returns new result, using
 *  +defaultBehavior for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberBySubtracting:
  (NSDecimalNumber *)decimalNumber;

/**
 *  Subtracts decimalNumber from self and returns new result, using given
 *  behavior for rounding/precision/error handling.
 */
- (NSDecimalNumber *)decimalNumberBySubtracting:
  (NSDecimalNumber *)decimalNumber 
  withBehavior:(id <NSDecimalNumberBehaviors>)behavior;

/**
 *  Returns rounded version of underlying decimal.
 */
- (NSDecimalNumber *)decimalNumberByRoundingAccordingToBehavior:(id <NSDecimalNumberBehaviors>)behavior;

@end

/**
 *  Interface for obtaining an NSDecimalNumber value from an ordinary
 *  NSNumber.
 */
@interface NSNumber (NSDecimalNumber)
/**
 *  Obtaining an NSDecimalNumber version of an ordinary NSNumber.
 */
- (NSDecimal) decimalValue;
@end

#if	defined(__cplusplus)
}
#endif

#endif
#endif
