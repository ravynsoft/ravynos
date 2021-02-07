/**
   NSDecimalNumber class
   Copyright (C) 2000 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Created: July 2000

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

   <title>NSDecimalNumber class reference</title>
   $Date$ $Revision$
   */

#import "common.h"

#include <math.h>

#define	EXPOSE_NSDecimalNumber_IVARS	1
#define	EXPOSE_NSDecimalNumberHandler_IVARS	1
#import "Foundation/NSCoder.h"
#import "Foundation/NSDecimal.h"
#import "Foundation/NSDecimalNumber.h"
#import "Foundation/NSException.h"
#import "Foundation/NSPortCoder.h"

#import "GSPrivate.h"

#ifdef fpclassify
#define GSIsNAN(n) (fpclassify(n) == FP_NAN)
#define GSIsInf(n) (fpclassify(n) == FP_INFINITE)
#else
#warning C99 macro fpclassify not found: Cannot determine NAN/Inf float-values
#define GSIsNAN(n) (0)
#define GSIsInf(n) (0)
#endif

// shared default behavior for NSDecimalNumber class
static NSDecimalNumberHandler *handler;

@implementation NSDecimalNumberHandler

+ (id) defaultDecimalNumberHandler
{
  if (handler == nil)
    handler = [[self alloc] initWithRoundingMode: NSRoundPlain
			    scale: 38
			    raiseOnExactness: NO
			    raiseOnOverflow: YES
			    raiseOnUnderflow: YES
			    raiseOnDivideByZero: YES];

  return handler;
}

+ (id) decimalNumberHandlerWithRoundingMode: (NSRoundingMode)roundingMode
				      scale: (short)scale
			   raiseOnExactness: (BOOL)raiseOnExactness
			    raiseOnOverflow: (BOOL)raiseOnOverflow
			   raiseOnUnderflow: (BOOL)raiseOnUnderflow
			raiseOnDivideByZero: (BOOL)raiseOnDivideByZero
{
  return AUTORELEASE([[self alloc] initWithRoundingMode: roundingMode
				   scale: scale
				   raiseOnExactness: raiseOnExactness
				   raiseOnOverflow: raiseOnOverflow
				   raiseOnUnderflow: raiseOnUnderflow
				   raiseOnDivideByZero: raiseOnDivideByZero]);
}

- (id) initWithRoundingMode: (NSRoundingMode)roundingMode
		      scale: (short)scale
	   raiseOnExactness: (BOOL)raiseOnExactness
	    raiseOnOverflow: (BOOL)raiseOnOverflow
	   raiseOnUnderflow: (BOOL)raiseOnUnderflow
	raiseOnDivideByZero: (BOOL)raiseOnDivideByZero
{
  _roundingMode = roundingMode;
  _scale = scale;
  _raiseOnExactness = raiseOnExactness;
  _raiseOnOverflow = raiseOnOverflow;
  _raiseOnUnderflow = raiseOnUnderflow;
  _raiseOnDivideByZero = raiseOnDivideByZero;

  return self;
}

- (NSDecimalNumber*) exceptionDuringOperation: (SEL)method
					error: (NSCalculationError)error
				  leftOperand: (NSDecimalNumber*)leftOperand
				 rightOperand: (NSDecimalNumber*)rightOperand
{
  switch (error)
    {
      case NSCalculationNoError: return nil;
      case NSCalculationUnderflow:
	if (_raiseOnUnderflow)
	  // FIXME: What exception to raise?
	  [NSException raise: @"NSDecimalNumberException"
		      format: @"Underflow"];
	else
	  return [NSDecimalNumber minimumDecimalNumber];
	break;
      case NSCalculationOverflow:
	if (_raiseOnOverflow)
	  [NSException raise: @"NSDecimalNumberException"
		      format: @"Overflow"];
	else
	  return [NSDecimalNumber maximumDecimalNumber];
	break;
      case NSCalculationLossOfPrecision:
	if (_raiseOnExactness)
	  [NSException raise: @"NSDecimalNumberException"
		      format: @"Loss of precision"];
	else
	  return nil;
	break;
      case NSCalculationDivideByZero:
	if (_raiseOnDivideByZero)
	  [NSException raise: @"NSDecimalNumberException"
		      format: @"Divide by zero"];
	else
	  return [NSDecimalNumber notANumber];
	break;
    }

  return nil;
}

- (NSRoundingMode) roundingMode
{
  return _roundingMode;
}

- (short) scale
{
  return _scale;
}

@end


@implementation NSDecimalNumber

static Class NSDecimalNumberClass;
static NSDecimalNumber *maxNumber;
static NSDecimalNumber *minNumber;
static NSDecimalNumber *notANumber;
static NSDecimalNumber *zero;
static NSDecimalNumber *one;

+ (void) initialize
{
  /* Initialize d to an empty structure to avoid compiler warnings.
   * This also sets d.validNumber to NO ... which is what is actually needed.
   */
  NSDecimal d = { 0 };

  notANumber = [[self alloc] initWithDecimal: d];
  [[NSObject leakAt: &notANumber] release];
  NSDecimalMax(&d);
  maxNumber = [[self alloc] initWithDecimal: d];
  [[NSObject leakAt: &maxNumber] release];
  NSDecimalMin(&d);
  minNumber = [[self alloc] initWithDecimal: d];
  [[NSObject leakAt: &minNumber] release];
  zero = [[self alloc] initWithMantissa: 0
			       exponent: 0
			     isNegative: NO];
  [[NSObject leakAt: &zero] release];
  one = [[self alloc] initWithMantissa: 1
			      exponent: 0
			    isNegative: NO];
  [[NSObject leakAt: &one] release];
  NSDecimalNumberClass = [NSDecimalNumber class];
}

+ (id <NSDecimalNumberBehaviors>) defaultBehavior
{
  // Reuse the handler from the class NSDecimalNumberHandler
  return [NSDecimalNumberHandler defaultDecimalNumberHandler];
}

+ (void) setDefaultBehavior: (id <NSDecimalNumberBehaviors>)behavior
{
  // Reuse the handler from the class NSDecimalNumberHandler
  // Might give interessting result on this class as behavior may came
  // from a different class
  ASSIGN(handler, (id)behavior);
}

+ (NSDecimalNumber*) maximumDecimalNumber
{
  return maxNumber;
}

+ (NSDecimalNumber*) minimumDecimalNumber
{
  return minNumber;
}

+ (NSDecimalNumber*) notANumber
{
  return notANumber;
}

+ (NSDecimalNumber*) zero
{
  return zero;
}

+ (NSDecimalNumber*) one
{
  return one;
}

+ (NSDecimalNumber*) decimalNumberWithDecimal: (NSDecimal)decimal
{
  return AUTORELEASE([[self alloc] initWithDecimal: decimal]);
}

+ (NSDecimalNumber*) decimalNumberWithMantissa: (unsigned long long)mantissa
				      exponent: (short)exponent
				    isNegative: (BOOL)isNegative
{
  return AUTORELEASE([[self alloc] initWithMantissa: mantissa
					   exponent: exponent
					 isNegative: isNegative]);
}

+ (NSDecimalNumber*) decimalNumberWithString: (NSString*)numericString
{
  return AUTORELEASE([[self alloc] initWithString: numericString]);
}

+ (NSDecimalNumber*) decimalNumberWithString: (NSString*)numericString
				      locale: (NSDictionary*)locale
{
  return AUTORELEASE([[self alloc] initWithString: numericString
					   locale: locale]);
}

/**
 * Inefficient ... quick hack by converting double value to string,
 * then initialising from string.
 */
- (id) initWithBytes: (const void*)value objCType: (const char*)type
{
  unsigned long long val = 0ll;
  long long llval = 0ll;
  NSDecimal decimal;
  BOOL negative, llvalSet;

  if (strlen(type) != 1)
    {
      DESTROY(self);
      return nil;
    }

  llvalSet = YES;
  negative = NO;

  switch (*type)
    {
    case _C_CHR:
      {
	signed char v = *(signed char *)value;
	llval = (long long)v;
	break;
      }
    case _C_UCHR:
      {
	unsigned char v = *(unsigned char *)value;
	llval = (long long)v;
	break;
      }
    case _C_SHT:
      {
	short v = *(short *)value;
	llval = (long long)v;
	break;
      }
    case _C_USHT:
      {
	unsigned short v = *(unsigned short *)value;
	llval = (long long)v;
	break;
      }
    case _C_INT:
      {
	int v = *(int *)value;
	llval = (long long)v;
	break;
      }
    case _C_UINT:
      {
	unsigned int v = *(unsigned int *)value;
	llval = (long long)v;
	break;
      }
    case _C_LNG:
      {
	long v = *(long *)value;
	llval = (long long)v;
	break;
      }
    case _C_ULNG:
      {
	unsigned long v = *(unsigned long *)value;
	llval = (long long)v;
	break;
      }
#if __GNUC__ > 2 && defined(_C_BOOL)
    case _C_BOOL:
      {
	llval = (long long)((*(unsigned char *)value == 0) ? 0 : 1);
	break;
      }
#endif
#ifdef _C_LNGLNG
    case _C_LNGLNG:
#else
    case 'q':
#endif
      {
	long long v = *(long long *)value;
	llval = (long long)v;
	break;
      }
#ifdef	_C_ULNGLNG
    case _C_ULNGLNG:
#else
    case 'Q':
#endif
    default:
      {
	llvalSet = NO;
	break;

      }
    }
  if (llvalSet)
    {
      if (llval<0)
	{
	  negative = YES;
	  llval *= -1;
	}
      val = llval;
    }
  else
    {
      switch (*type)
	{
	case _C_FLT:
	  /* FIXME: This is better implemented with GMP where available.  */
	  {
	    NSString *s;
	    float v = *(float *)value;
	    if (GSIsNAN(v))
	      {
		DESTROY(self);
		return RETAIN(notANumber);
	      }
	    if (GSIsInf(v))
	      {
		DESTROY(self);
		return (v < 0.0) ? RETAIN(minNumber) : RETAIN(maxNumber);
	      }
	    s = [[NSString alloc] initWithFormat: @"%g"
				  locale: GSPrivateDefaultLocale(), (double)v];
	    self = [self initWithString: s];
	    RELEASE(s);
	    return self;
	    break;
	  }
	case _C_DBL:
	  /* FIXME: This is better implemented with GMP where available.  */
	  {
	    NSString *s;
	    double v = *(double *)value;
	    if (GSIsNAN(v))
	      {
		DESTROY(self);
		return RETAIN(notANumber);
	      }
	    if (GSIsInf(v))
	      {
		DESTROY(self);
		return (v < 0.0) ? RETAIN(minNumber) : RETAIN(maxNumber);
	      }
	    s = [[NSString alloc] initWithFormat: @"%g"
				  locale: GSPrivateDefaultLocale(), v];
	    self = [self initWithString: s];
	    RELEASE(s);
	    return self;
	    break;
	  }
#ifdef  _C_ULNGLNG
	case _C_ULNGLNG: 
#else
	case 'Q':
#endif
	  {
	    val = *(unsigned long long *)value;
	    break;
	  }
	}
    }

  NSDecimalFromComponents(&decimal, val,
			  0, negative);
  return [self initWithDecimal: decimal];
}

- (id) initWithDecimal: (NSDecimal)decimal
{
  NSDecimalCopy(&data, &decimal);
  return self;
}

- (id) initWithMantissa: (unsigned long long)mantissa
	       exponent: (short)exponent
	     isNegative: (BOOL)flag
{
  NSDecimal decimal;

  NSDecimalFromComponents(&decimal, mantissa, exponent, flag);
  return [self initWithDecimal: decimal];
}

- (id) initWithString: (NSString*)numberValue
{
  return [self initWithString: numberValue
    locale: GSPrivateDefaultLocale()];
}

- (id) initWithString: (NSString*)numberValue
	       locale: (NSDictionary*)locale
{
  NSDecimal decimal;

  NSDecimalFromString(&decimal, numberValue, locale);
  return [self initWithDecimal: decimal];
}

- (id) initWithBool: (BOOL)value
{
  return [self initWithMantissa: (value == YES) ? 1 : 0
		       exponent: 0
		     isNegative: NO];
}

- (id) initWithChar: (signed char)value
{
  if (value < 0)
    {
      return [self initWithMantissa: -value
			   exponent: 0
			 isNegative: YES];
    }
  else
    {
      return [self initWithMantissa: value
			   exponent: 0
			 isNegative: NO];
    }
}

- (id) initWithDouble: (double)value
{
  return [self initWithBytes: &value objCType: "d"];
}

- (id) initWithFloat: (float)value
{
  double	d = (double)value;

  return [self initWithBytes: &d objCType: "d"];
}

- (id) initWithInt: (int)value
{
  if (value < 0)
    {
      return [self initWithMantissa: -value
			   exponent: 0
			 isNegative: YES];
    }
  else
    {
      return [self initWithMantissa: value
			   exponent: 0
			 isNegative: NO];
    }
}

- (id) initWithInteger: (NSInteger)value
{
  if (value < 0)
    {
      return [self initWithMantissa: -value
			   exponent: 0
			 isNegative: YES];
    }
  else
    {
      return [self initWithMantissa: value
			   exponent: 0
			 isNegative: NO];
    }
}

- (id) initWithLong: (signed long)value
{
  if (value < 0)
    {
      return [self initWithMantissa: -value
			   exponent: 0
			 isNegative: YES];
    }
  else
    {
      return [self initWithMantissa: value
			   exponent: 0
			 isNegative: NO];
    }
}

- (id) initWithLongLong: (signed long long)value
{
  if (value < 0)
    {
      return [self initWithMantissa: -value
			   exponent: 0
			 isNegative: YES];
    }
  else
    {
      return [self initWithMantissa: value
			   exponent: 0
			 isNegative: NO];
    }
}

- (id) initWithShort: (signed short)value
{
  if (value < 0)
    {
      return [self initWithMantissa: -value
			   exponent: 0
			 isNegative: YES];
    }
  else
    {
      return [self initWithMantissa: value
			   exponent: 0
			 isNegative: NO];
    }
}

- (id) initWithUnsignedChar: (unsigned char)value
{
  return [self initWithMantissa: value
		       exponent: 0
		     isNegative: NO];
}

- (id) initWithUnsignedInt: (unsigned int)value
{
  return [self initWithMantissa: value
		       exponent: 0
		     isNegative: NO];
}

- (id) initWithUnsignedInteger: (NSUInteger)value
{
  return [self initWithMantissa: value
		       exponent: 0
		     isNegative: NO];
}

- (id) initWithUnsignedLong: (unsigned long)value
{
  return [self initWithMantissa: value
		       exponent: 0
		     isNegative: NO];
}

- (id) initWithUnsignedLongLong: (unsigned long long)value
{
  return [self initWithMantissa: value
		       exponent: 0
		     isNegative: NO];
}

- (id) initWithUnsignedShort: (unsigned short)value
{
  return [self initWithMantissa: value
		       exponent: 0
		     isNegative: NO];
}

- (NSString*) descriptionWithLocale: (id)locale
{
  return NSDecimalString(&data, locale);
}

- (const char*) objCType
{
  return "d";
}

- (NSDecimal) decimalValue
{
  NSDecimal decimal;

  NSDecimalCopy(&decimal, &data);
  return decimal;
}

- (BOOL) boolValue
{
  return NSDecimalDouble(&data) == 0.0 ? NO : YES;
}

- (double) doubleValue
{
  return NSDecimalDouble(&data);
}

- (float) floatValue
{
  return (float)NSDecimalDouble(&data);
}

- (signed char) charValue
{
  return (char)NSDecimalDouble(&data);
}

- (int) intValue
{
  return (int)NSDecimalDouble(&data);
}

- (NSInteger) integerValue
{
  return (NSInteger)NSDecimalDouble(&data);
}

- (long) longValue
{
  return (long)NSDecimalDouble(&data);
}

- (long long) longLongValue
{
  return (long long)NSDecimalDouble(&data);
}

- (short) shortValue
{
  return (short)NSDecimalDouble(&data);
}

- (unsigned char) unsignedCharValue
{
  return (unsigned char)NSDecimalDouble(&data);
}

- (unsigned int) unsignedIntValue
{
  return (unsigned int)NSDecimalDouble(&data);
}

- (NSUInteger) unsignedIntegerValue
{
  return (NSUInteger)NSDecimalDouble(&data);
}

- (unsigned long) unsignedLongValue
{
  return (unsigned long)NSDecimalDouble(&data);
}

- (unsigned long long) unsignedLongLongValue
{
  return (unsigned long long)NSDecimalDouble(&data);
}

- (unsigned short) unsignedShortValue
{
  return (unsigned short)NSDecimalDouble(&data);
}

/**
 * Get the approximate value of the decimal number into a buffer
 * as a double.
 */
- (void) getValue: (void*)buffer
{
  double	tmp = NSDecimalDouble(&data);

  memcpy(buffer, &tmp, sizeof(tmp));
}

- (NSComparisonResult) compare: (NSNumber*)decimalNumber
{
  if (self == decimalNumber)
    {
      return NSOrderedSame;
    }
  if (self == notANumber)
    {
      return NSOrderedAscending;	// NaN is considered less than anything
    }
  if ([decimalNumber isKindOfClass: NSDecimalNumberClass])
    {
      NSDecimal d1 = [self decimalValue];
      NSDecimal d2 = [(NSDecimalNumber*)decimalNumber decimalValue];

      return NSDecimalCompare(&d1, &d2);
    }
  else if ([decimalNumber isKindOfClass: [NSNumber class]])
    {
      NSComparisonResult	r = [decimalNumber compare: self];

      if (r == NSOrderedAscending)
	{
	  return NSOrderedDescending;
	}
      else if (r == NSOrderedDescending)
	{
	  return NSOrderedAscending;
	}
      return NSOrderedSame;
    }
  else
    {
      return [super compare: decimalNumber];
    }
}

- (NSDecimalNumber*) decimalNumberByAdding: (NSDecimalNumber*)decimalNumber
{
  return [self decimalNumberByAdding: decimalNumber
			withBehavior: [[self class] defaultBehavior]];
}

- (NSDecimalNumber*) decimalNumberByAdding: (NSDecimalNumber*)decimalNumber
  withBehavior: (id<NSDecimalNumberBehaviors>)behavior
{
  NSDecimal result;
  NSDecimal d1 = [self decimalValue];
  NSDecimal d2 = [decimalNumber decimalValue];
  NSCalculationError error;
  NSDecimalNumber *res;

  error = NSDecimalAdd(&result, &d1, &d2, [behavior roundingMode]);
  if (error)
    {
      res = [behavior exceptionDuringOperation: _cmd
					 error: error
				   leftOperand: self
				  rightOperand: decimalNumber];
      if (res != nil)
	return res;
    }

  return [NSDecimalNumber decimalNumberWithDecimal: result];
}

- (NSDecimalNumber*) decimalNumberBySubtracting: (NSDecimalNumber*)decimalNumber
{
  return [self decimalNumberBySubtracting: decimalNumber
			     withBehavior: [[self class] defaultBehavior]];
}

- (NSDecimalNumber*) decimalNumberBySubtracting: (NSDecimalNumber*)decimalNumber
  withBehavior: (id <NSDecimalNumberBehaviors>)behavior
{
  NSDecimal result;
  NSDecimal d1 = [self decimalValue];
  NSDecimal d2 = [decimalNumber decimalValue];
  NSCalculationError error;
  NSDecimalNumber *res;

  error = NSDecimalSubtract(&result, &d1, &d2, [behavior roundingMode]);
  if (error)
    {
      res = [behavior exceptionDuringOperation: _cmd
					 error: error
				   leftOperand: self
				  rightOperand: decimalNumber];
      if (res != nil)
	return res;
    }

  return [NSDecimalNumber decimalNumberWithDecimal: result];
}

- (NSDecimalNumber*) decimalNumberByMultiplyingBy:
  (NSDecimalNumber*)decimalNumber
{
  return [self decimalNumberByMultiplyingBy: decimalNumber
			       withBehavior: [[self class] defaultBehavior]];
}

- (NSDecimalNumber*) decimalNumberByMultiplyingBy:
  (NSDecimalNumber*)decimalNumber
  withBehavior: (id <NSDecimalNumberBehaviors>)behavior
{
  NSDecimal result;
  NSDecimal d1 = [self decimalValue];
  NSDecimal d2 = [decimalNumber decimalValue];
  NSCalculationError error;
  NSDecimalNumber *res;

  error = NSDecimalMultiply(&result, &d1, &d2, [behavior roundingMode]);
  if (error)
    {
      res = [behavior exceptionDuringOperation: _cmd
					 error: error
				   leftOperand: self
				  rightOperand: decimalNumber];
      if (res != nil)
	return res;
    }

  return [NSDecimalNumber decimalNumberWithDecimal: result];
}

- (NSDecimalNumber*) decimalNumberByDividingBy: (NSDecimalNumber*)decimalNumber
{
  return [self decimalNumberByDividingBy: decimalNumber
			    withBehavior: [[self class] defaultBehavior]];
}

- (NSDecimalNumber*) decimalNumberByDividingBy: (NSDecimalNumber*)decimalNumber
  withBehavior: (id <NSDecimalNumberBehaviors>)behavior
{
  NSDecimal result;
  NSDecimal d1 = [self decimalValue];
  NSDecimal d2 = [decimalNumber decimalValue];
  NSCalculationError error;
  NSDecimalNumber *res;

  error = NSDecimalDivide(&result, &d1, &d2, [behavior roundingMode]);
  if (error)
    {
      res = [behavior exceptionDuringOperation: _cmd
					 error: error
				   leftOperand: self
				  rightOperand: decimalNumber];
      if (res != nil)
	return res;
    }

  return [NSDecimalNumber decimalNumberWithDecimal: result];
}

- (NSDecimalNumber*) decimalNumberByMultiplyingByPowerOf10: (short)power
{
  return [self decimalNumberByMultiplyingByPowerOf10: power
					withBehavior: [[self class] defaultBehavior]];
}

- (NSDecimalNumber*) decimalNumberByMultiplyingByPowerOf10: (short)power
  withBehavior: (id <NSDecimalNumberBehaviors>)behavior
{
  NSDecimal result;
  NSDecimal d1 = [self decimalValue];
  NSCalculationError error;
  NSDecimalNumber *res;

  error = NSDecimalMultiplyByPowerOf10(&result, &d1,
    power, [behavior roundingMode]);
  if (error)
    {
      res = [behavior exceptionDuringOperation: _cmd
					 error: error
				   leftOperand: self
				  rightOperand: nil];
      if (res != nil)
	return res;
    }

  return [NSDecimalNumber decimalNumberWithDecimal: result];
}

- (NSDecimalNumber*) decimalNumberByRaisingToPower: (NSUInteger)power
{
  return [self decimalNumberByRaisingToPower: power
				withBehavior: [[self class] defaultBehavior]];
}

- (NSDecimalNumber*) decimalNumberByRaisingToPower: (NSUInteger)power
  withBehavior: (id <NSDecimalNumberBehaviors>)behavior
{
  NSDecimal result;
  NSDecimal d1 = [self decimalValue];
  NSCalculationError error;
  NSDecimalNumber *res;

  error = NSDecimalPower(&result, &d1,
    power, [behavior roundingMode]);
  if (error)
    {
      res = [behavior exceptionDuringOperation: _cmd
					 error: error
				   leftOperand: self
				  rightOperand: nil];
      if (res != nil)
	return res;
    }

  return [NSDecimalNumber decimalNumberWithDecimal: result];
}

- (NSDecimalNumber*) decimalNumberByRoundingAccordingToBehavior:
  (id <NSDecimalNumberBehaviors>)behavior
{
  NSDecimal result;
  NSDecimal d1 = [self decimalValue];

  NSDecimalRound(&result, &d1, [behavior scale], [behavior roundingMode]);
  return [NSDecimalNumber decimalNumberWithDecimal: result];
}


// Methods for NSDecimalNumberBehaviors
- (NSDecimalNumber*) exceptionDuringOperation: (SEL)method
					error: (NSCalculationError)error
				  leftOperand: (NSDecimalNumber*)leftOperand
				 rightOperand: (NSDecimalNumber*)rightOperand
{
  return [[[self class] defaultBehavior] exceptionDuringOperation: method
    error: error
    leftOperand: leftOperand
    rightOperand: rightOperand];
}

- (NSRoundingMode) roundingMode
{
  return [[[self class] defaultBehavior] roundingMode];
}

- (short) scale
{
  return [[[self class] defaultBehavior] scale];
}

- (Class) classForCoder
{
  return [NSDecimalNumber class];
}

- (id) replacementObjectForPortCoder: (NSPortCoder*)aCoder
{
  if ([aCoder isByref] == NO)
    return self;
  return [super replacementObjectForPortCoder: aCoder];
}

- (void) encodeWithCoder: (NSCoder*)coder
{
  NSString	*s = [self descriptionWithLocale: nil];

  [coder encodeObject: s];
}

- (id) initWithCoder: (NSCoder*)coder
{
  NSString	*s = [coder decodeObject];

  return [self initWithString: s locale: nil];
}

@end

@implementation NSNumber (NSDecimalNumber)
/** Returns an NSDecimal representation of the number. Float and double
    values may not be converted exactly */
- (NSDecimal) decimalValue
{
  double num;
  NSDecimalNumber *dnum;
  num = [self doubleValue];
  dnum
    = AUTORELEASE([[NSDecimalNumber alloc] initWithBytes: &num objCType: "d"]);
  return [dnum decimalValue];
}
@end

