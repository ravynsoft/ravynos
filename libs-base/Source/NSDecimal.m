/**
   NSDecimal functions
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

   <title>NSDecimal class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#include <math.h>
#if !defined(__APPLE__) || !defined(GNU_RUNTIME)
#include <ctype.h>
#endif
#import "Foundation/NSDecimal.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSUserDefaults.h"

#ifndef NAN
#define NAN 0.0
#endif 

/*
  This file provides two implementations of the NSDecimal functions.
  One is based on pure simple decimal mathematics, as we all learned it
  in school. This version is rather slow and may be inexact in the extreme
  cases.
  THIS IS TESTED AND WORKING.

  The second implemenation requires the GMP library, the GNU math package,
  to do the hard work. This is very fast and accurate. But as GMP is not
  available on all computers this has to be switched on at compile time.
  THIS IS STILL NOT IMPLEMENTED.

  The data structure used for NSDecimals is a bit strange. It also does not
  correspond to the description in the OpenStep specification. But this is
  not consistent, so a decision had to be made.
  The mantissa part (I know D. Knuth does not like this term, but it is used
  in the specification so we stay with it) consists of up to 38 digits, this
  are stored as an integer (in decimal representation or limps depending on the
  USE_GMP flag). And the exponent is stored in a signed character. As a result
  the numbers that can be represented are the ranges from -9(38 times)*10**127
  to  -1*10**-128, the number 0 and 1*10**-128 to 9(38 times)*10**127.
  This means we have more big numbers than one would expect (almost up to 10**165)
  but small numbers can only be represented with limited exactness (one digit
  for -128, two for -127 and so on).
  I think this is as close as possible to the specification, but other
  interpretations are also valid. (Changing the exponent either absolut
  [eg minus 38] or relative to the number of digits in the mantissa [minus length].)

 */

#if	USE_GMP

// Define GSDecimal as using a character vector
typedef struct {
  signed char	exponent;	/* Signed exponent - -128 to 127	*/
  BOOL	isNegative;	/* Is this negative?			*/
  BOOL	validNumber;	/* Is this a valid number?		*/
  unsigned char	length;		/* digits in mantissa.			*/
  unsigned char  cMantissa[2*NSDecimalMaxDigit]; /* Make this big enough for multiplication */
} GSDecimal;

static NSDecimal zero = {0, NO, YES, 0, {0}};
static NSDecimal one = {0, NO, YES, 1, {1}};

#define NSDECIMAL_IS_ZERO(num) (0 == num->size)
#define GSDECIMAL_IS_ZERO(num) (0 == num->length)

#else

// Make GSDecimal a synonym of NSDecimal
/** <code>
typedef struct {<br/>
  signed char	exponent;   // Signed exponent - -128 to 127<br/>
  BOOL	isNegative;         // Is this negative?<br/>
  BOOL	validNumber;        // Is this a valid number?<br/>
  unsigned char	length;	    // digits in mantissa.<br/>
  unsigned char  cMantissa[2*NSDecimalMaxDigit];<br/>
}<br/>
</code> */
typedef NSDecimal GSDecimal;

static NSDecimal zero = {0, NO, YES, 0, {0}};
static NSDecimal one = {0, NO, YES, 1, {1}};

#define NSDECIMAL_IS_ZERO(num) (0 == num->length)
#define GSDECIMAL_IS_ZERO(num) (0 == num->length)

#endif


void
NSDecimalCopy(NSDecimal *destination, const NSDecimal *source)
{
  memcpy(destination, source, sizeof(NSDecimal));
}

static void
GSDecimalCompact(GSDecimal *number)
{
  int i, j;

  //NSLog(@"Compact start %@ ", NSDecimalString(number, nil));
  if (!number->validNumber)
    return;

  // Cut off leading 0's
  for (i = 0; i < number->length; i++)
    {
      if (number->cMantissa[i] != 0)
	break;
    }
  if (i > 0)
    {
      for (j = 0; j < number->length-i; j++)
	{
	  number->cMantissa[j] = number->cMantissa[j+i];
	}
      number->length -= i;
    }

  // Cut off trailing 0's
  for (i = number->length-1; i >= 0; i--)
    {
      if (0 == number->cMantissa[i])
        {
	  if (127 == number->exponent)
	    {
	      // Overflow in compacting!!
	      // Leave the remaining 0s there.
	      break;
	    }
	  number->length--;
	  number->exponent++;
	}
      else
	break;
    }

  if (GSDECIMAL_IS_ZERO(number))
    {
      number->exponent = 0;
      number->isNegative = NO;
    }
  //NSLog(@"Compact end %@ ", NSDecimalString(number, nil));
}

static NSComparisonResult
GSDecimalCompare(const GSDecimal *leftOperand, const GSDecimal *rightOperand)
{
  int i, l;
  int s1 = leftOperand->exponent + leftOperand->length;
  int s2 = rightOperand->exponent + rightOperand->length;

  if (leftOperand->validNumber != rightOperand->validNumber)
    {
      if (rightOperand->validNumber)
	return NSOrderedDescending;
      else
	return NSOrderedAscending;
    }

  if (leftOperand->isNegative != rightOperand->isNegative)
    {
      if (rightOperand->isNegative)
	return NSOrderedDescending;
      else
	return NSOrderedAscending;
    }

  // Same sign, check size
  if (s1 < s2)
    {
      if (rightOperand->isNegative)
	return NSOrderedDescending;
      else
	return NSOrderedAscending;
    }
  if (s1 > s2)
    {
      if (rightOperand->isNegative)
	return NSOrderedAscending;
      else
	return NSOrderedDescending;
    }

  // Same size, check digits
  l = MIN(leftOperand->length, rightOperand->length);
  for (i = 0; i < l; i++)
    {
      int d = rightOperand->cMantissa[i] - leftOperand->cMantissa[i];

      if (d > 0)
        {
	  if (rightOperand->isNegative)
	    return NSOrderedDescending;
	  else
	    return NSOrderedAscending;
	}
      if (d < 0)
        {
	  if (rightOperand->isNegative)
	    return NSOrderedAscending;
	  else
	    return NSOrderedDescending;
	}
    }

  // Same digits, check length
  if (leftOperand->length > rightOperand->length)
    {
      if (rightOperand->isNegative)
	return NSOrderedAscending;
      else
	return NSOrderedDescending;
    }

  if (leftOperand->length < rightOperand->length)
    {
      if (rightOperand->isNegative)
	return NSOrderedDescending;
      else
	return NSOrderedAscending;
    }

  return NSOrderedSame;
}

static NSComparisonResult
NSSimpleCompare(const NSDecimal *leftOperand, const NSDecimal *rightOperand);

static void
GSDecimalRound(GSDecimal *result, int scale, NSRoundingMode mode)
{
  int i;
  // last valid digit in number
  int l = scale + result->exponent + result->length;

  if (NSDecimalNoScale == scale)
      return;

  if (!result->validNumber)
    return;

  if (result->length <= l)
    return;
  else if (l < 0)
    {
      result->length = 0;
      result->exponent = 0;
      result->isNegative = NO;
      return;
    }
  else
    {
      int c, n;
      BOOL up;

      if (l == 0)
        {
          int x;
             
          x = result->length;
          result->length += 1;
          l += 1;
          while (x > 0)
            {
               result->cMantissa[x] = result->cMantissa[x-1];
               x--;
            }
          result->cMantissa[0] = 0;
        }

      // Adjust length and exponent
      result->exponent += result->length - l;
      result->length = l;

      switch (mode)
        {
	  case NSRoundDown:
	    up = result->isNegative;
	    break;
	  case NSRoundUp:
	    up = !result->isNegative;
	    break;
	  case NSRoundPlain:
	    n = result->cMantissa[l];
	    up = (n >= 5);
	    break;
	  case NSRoundBankers:
	    n = result->cMantissa[l];
	    if (n > 5)
              {
	        up = YES;
              }
	    else if (n < 5)
              {
                up = NO;
              }
	    else
	      {
		c = result->cMantissa[l-1];
		up = ((c % 2) != 0);
	      }
	    break;
	  default: // No way to get here
	    up = NO;
	    break;
	}

      if (up)
      {
	for (i = l-1; i >= 0; i--)
	  {
	    if (result->cMantissa[i] != 9)
	      {
		result->cMantissa[i]++;
		break;
	      }
	    result->cMantissa[i] = 0;
	  }
	// Final overflow?
	if (-1 == i)
	  {
	    // As all digits are zeros, just change the first
	    result->cMantissa[0] = 1;
	    if (127 == result->exponent)
	      {
		// Overflow in rounding!!
		// Add one zero add the end. There must be space as
		// we just cut off some digits.
		result->cMantissa[l] = 0;
		result->length++;
	      }
	    else
	      result->exponent++;
	  }
      }
    }

  GSDecimalCompact(result);
}

static NSCalculationError
GSDecimalNormalize(GSDecimal *n1, GSDecimal *n2, NSRoundingMode mode)
{
  // Both are valid numbers and the exponents are not equal
  int e1 = n1->exponent;
  int e2 = n2->exponent;
  int i, l;

  // make sure n2 has the bigger exponent
  if (e1 > e2)
    {
      GSDecimal *t;

      t = n1;
      n1 = n2;
      n2 = t;
      i = e2;
      e2 = e1;
      e1 = i;
    }

  // Add zeros to n2, as far as possible
  l = MIN(NSDecimalMaxDigit - n2->length, e2 - e1);
  for (i = 0; i < l; i++)
    {
      n2->cMantissa[i + n2->length] = 0;
    }
  n2->length += l;
  n2->exponent -= l;

  if (l != e2 - e1)
    {
      // Round of some digits from n1 to increase exponent
      GSDecimalRound(n1, -n2->exponent, mode);
      if (n1->exponent != n2->exponent)
	{
	  // Some zeros where cut of again by compacting
	  l = MIN(NSDecimalMaxDigit - n1->length, n1->exponent - n2->exponent);
	  for (i = 0; i < l; i++)
	    {
		n1->cMantissa[(NSInteger)n1->length] = 0;
		n1->length++;
	    }
	  n1->exponent = n2->exponent;
	}
      return NSCalculationLossOfPrecision;
    }

  return NSCalculationNoError;
}

static NSCalculationError
GSSimpleAdd(NSDecimal *result, const NSDecimal *left, const NSDecimal *right,
	    NSRoundingMode mode);

NSCalculationError
NSDecimalAdd(NSDecimal *result, const NSDecimal *left, const NSDecimal *right,
	     NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  NSCalculationError error1;
  NSDecimal n1;
  NSDecimal n2;
  NSComparisonResult comp;

  if (!left->validNumber || !right->validNumber)
    {
      result->validNumber = NO;
      return error;
    }

  // check for zero
  if (NSDECIMAL_IS_ZERO(left))
    {
      NSDecimalCopy(result, right);
      return error;
    }
  if (NSDECIMAL_IS_ZERO(right))
    {
      NSDecimalCopy(result, left);
      return error;
    }

  // For different signs use subtraction
  if (left->isNegative != right->isNegative)
    {
      if (left->isNegative)
        {
	  NSDecimalCopy(&n1, left);
	  n1.isNegative = NO;
	  return NSDecimalSubtract(result, right, &n1, mode);
	}
      else
        {
	  NSDecimalCopy(&n1, right);
	  n1.isNegative = NO;
	  return NSDecimalSubtract(result, left, &n1, mode);
	}
    }

  NSDecimalCopy(&n1, left);
  NSDecimalCopy(&n2, right);
  error = NSDecimalNormalize(&n1, &n2, mode);
  comp = NSSimpleCompare(&n1, &n2);
/*
  NSLog(@"Add left %@ right %@", NSDecimalString(left, nil),
	NSDecimalString(right, nil));
  NSLog(@"Add n1 %@ n2 %@ comp %d", NSDecimalString(&n1, nil),
	NSDecimalString(&n2, nil), comp);
*/
  // both negative, make positive
  if (left->isNegative)
    {
      n1.isNegative = NO;
      n2.isNegative = NO;
      // SimpleCompare does not look at sign
      if (NSOrderedDescending == comp)
        {
	  error1 = GSSimpleAdd(result, &n1, &n2, mode);
	}
      else
        {
	  error1 = GSSimpleAdd(result, &n2, &n1, mode);
	}
      result->isNegative = YES;
      if (NSCalculationUnderflow == error1)
	error1 = NSCalculationOverflow;
      else if (NSCalculationOverflow == error1)
	error1 = NSCalculationUnderflow;
    }
  else
    {
      if (NSOrderedAscending == comp)
        {
	  error1 = GSSimpleAdd(result, &n2, &n1, mode);
	}
      else
        {
	  error1 = GSSimpleAdd(result, &n1, &n2, mode);
	}
    }

  NSDecimalCompact(result);

  if (NSCalculationNoError == error1)
    return error;
  else
    return error1;
}

static NSCalculationError
GSSimpleSubtract(NSDecimal *result, const NSDecimal *left,
		 const NSDecimal *right, NSRoundingMode mode);

NSCalculationError
NSDecimalSubtract(NSDecimal *result, const NSDecimal *left,
		  const NSDecimal *right, NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  NSCalculationError error1;
  NSDecimal n1;
  NSDecimal n2;
  NSComparisonResult comp;

  if (!left->validNumber || !right->validNumber)
    {
      result->validNumber = NO;
      return error;
    }

  if (NSDECIMAL_IS_ZERO(right))
    {
      NSDecimalCopy(result, left);
      return error;
    }
  if (NSDECIMAL_IS_ZERO(left))
    {
      NSDecimalCopy(result, right);
      result->isNegative = !result->isNegative;
      return error;
    }

  // For different signs use addition
  if (left->isNegative != right->isNegative)
    {
      if (left->isNegative)
        {
	  NSDecimalCopy(&n1, left);
	  n1.isNegative = NO;
	  error1 = NSDecimalAdd(result, &n1, right, mode);
	  result->isNegative = YES;
	  if (NSCalculationUnderflow == error1)
	    error1 = NSCalculationOverflow;
	  else if (NSCalculationOverflow == error1)
	    error1 = NSCalculationUnderflow;
	  return error1;
	}
      else
        {
	  NSDecimalCopy(&n1, right);
	  n1.isNegative = NO;
	  return NSDecimalAdd(result, left, &n1, mode);
	}
    }

  NSDecimalCopy(&n1, left);
  NSDecimalCopy(&n2, right);
  error = NSDecimalNormalize(&n1, &n2, mode);

  comp = NSDecimalCompare(left, right);
/*
  NSLog(@"Sub left %@ right %@", NSDecimalString(left, nil),
	NSDecimalString(right, nil));
  NSLog(@"Sub n1 %@ n2 %@ comp %d", NSDecimalString(&n1, nil),
	NSDecimalString(&n2, nil), comp);
*/

  if (NSOrderedSame == comp)
    {
      NSDecimalCopy(result, &zero);
      return NSCalculationNoError;
    }

  // both negative, make positive and change order
  if (left->isNegative)
    {
      n1.isNegative = NO;
      n2.isNegative = NO;
      if (NSOrderedAscending == comp)
        {
	  error1 = GSSimpleSubtract(result, &n1, &n2, mode);
	  result->isNegative = YES;
	}
      else
        {
	  error1 = GSSimpleSubtract(result, &n2, &n1, mode);
	}
    }
  else
    {
      if (NSOrderedAscending == comp)
        {
	  error1 = GSSimpleSubtract(result, &n2, &n1, mode);
	  result->isNegative = YES;
	}
      else
        {
	  error1 = GSSimpleSubtract(result, &n1, &n2, mode);
	}
    }

  NSDecimalCompact(result);

  if (NSCalculationNoError == error1)
    return error;
  else
    return error1;
}

static NSCalculationError
GSSimpleMultiply(NSDecimal *result, NSDecimal *l, NSDecimal *r,
		 NSRoundingMode mode);

NSCalculationError
NSDecimalMultiply(NSDecimal *result, const NSDecimal *l, const NSDecimal *r,
		  NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  NSDecimal n1;
  NSDecimal n2;
  int exp = l->exponent + r->exponent;
  BOOL neg = l->isNegative != r->isNegative;
  NSComparisonResult comp;

  if (!l->validNumber || !r->validNumber)
    {
      result->validNumber = NO;
      return error;
    }

  // check for zero
  if (NSDECIMAL_IS_ZERO(l) || NSDECIMAL_IS_ZERO(r))
    {
      NSDecimalCopy(result, &zero);
      return error;
    }

  if (exp > 127)
    {
      result->validNumber = NO;
      if (neg)
	return NSCalculationUnderflow;
      else
	return NSCalculationOverflow;
    }

  NSDecimalCopy(&n1, l);
  NSDecimalCopy(&n2, r);
  n1.exponent = 0;
  n2.exponent = 0;
  n1.isNegative = NO;
  n2.isNegative = NO;
  comp = NSSimpleCompare(&n1, &n2);

  if (NSOrderedDescending == comp)
    {
      error = GSSimpleMultiply(result, &n1, &n2, mode);
    }
  else
    {
      error = GSSimpleMultiply(result, &n2, &n1, mode);
    }

  NSDecimalCompact(result);
  if (result->exponent + exp > 127)
    {
      result->validNumber = NO;
      if (neg)
	return NSCalculationUnderflow;
      else
	return NSCalculationOverflow;
    }
  else if (result->exponent + exp < -128)
    {
      // We must cut off some digits
      NSDecimalRound(result, result, exp+128, mode);
      error = NSCalculationLossOfPrecision;

      if (result->exponent + exp < -128)
        {
	  NSDecimalCopy(result, &zero);
	  return error;
        }
    }

  result->exponent += exp;
  result->isNegative = neg;

  return error;
}

static NSCalculationError
GSSimpleDivide(NSDecimal *result, const NSDecimal *l, const NSDecimal *r,
  NSRoundingMode mode);

NSCalculationError
NSDecimalDivide(NSDecimal *result, const NSDecimal *l, const NSDecimal *rr,
  NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  NSDecimal n1;
  NSDecimal n2;
  int exp = l->exponent - rr->exponent;
  BOOL neg = l->isNegative != rr->isNegative;

  if (!l->validNumber || !rr->validNumber)
    {
      result->validNumber = NO;
      return NSCalculationNoError;
    }

  // Check for zero
  if (NSDECIMAL_IS_ZERO(rr))
    {
      result->validNumber = NO;
      return NSCalculationDivideByZero;
    }
  if (NSDECIMAL_IS_ZERO(l))
    {
      NSDecimalCopy(result, &zero);
      return error;
    }

  // Should also check for one

  NSDecimalCopy(&n1, l);
  n1.exponent = 0;
  n1.isNegative = NO;
  NSDecimalCopy(&n2, rr);
  n2.exponent = 0;
  n2.isNegative = NO;

  error = GSSimpleDivide(result, &n1, &n2, mode);
  NSDecimalCompact(result);

  if (result->exponent + exp > 127)
    {
      result->validNumber = NO;
      if (neg)
	return NSCalculationUnderflow;
      else
	return NSCalculationOverflow;
    }
  else if (result->exponent + exp < -128)
    {
      // We must cut off some digits
      NSDecimalRound(result, result, exp+128, mode);
      error = NSCalculationLossOfPrecision;

      if (result->exponent + exp < -128)
        {
	  NSDecimalCopy(result, &zero);
	  return error;
        }
    }

  result->exponent += exp;
  result->isNegative = neg;

  return error;
}

NSCalculationError
NSDecimalPower(NSDecimal *result, const NSDecimal *n, NSUInteger power, NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  unsigned int e = power;
  NSDecimal n1;
  BOOL neg = (n->isNegative && (power % 2));

  NSDecimalCopy(&n1, n);
  n1.isNegative = NO;
  NSDecimalCopy(result, &one);
//  NSDecimalCopy(result, &zero);
//  result->length = 1;
//  result->cMantissa[0] = 1;

  while (e)
    {
      if (e & 1)
        {
	  error = NSDecimalMultiply(result, result, &n1, mode);
	  if (NSCalculationNoError != error)
	    {
	      break;
	    }
	}
      // keep on squaring the number
      error = NSDecimalMultiply(&n1, &n1, &n1, mode);
      if (NSCalculationNoError != error)
	{
	  break;
	}
      e >>= 1;
    }

  result->isNegative = neg;
  NSDecimalCompact(result);

  return error;
}

NSCalculationError
NSDecimalMultiplyByPowerOf10(NSDecimal *result, const NSDecimal *n, short power, NSRoundingMode mode)
{
  int p;

  NSDecimalCopy(result, n);
  p = result->exponent + power;
  if (p > 127)
    {
      result->validNumber = NO;
      return NSCalculationOverflow;
    }
  if (p < -128)
    {
      result->validNumber = NO;
      return NSCalculationUnderflow;
    }
  result->exponent += power;
  return NSCalculationNoError;
}

static NSString*
GSDecimalString(const GSDecimal *number, NSDictionary *locale)
{
  int i;
  int d;
  NSString *s;
  NSMutableString *string;
  NSString *sep;
  int size;

  if (!number->validNumber)
    return @"NaN";

  if ((nil == locale) ||
      (sep = [locale objectForKey: NSDecimalSeparator]) == nil)
    sep = @".";

  string = [NSMutableString stringWithCapacity: 45];

  if (!number->length)
    {
      [string appendString: @"0"];
      [string appendString: sep];
      [string appendString: @"0"];
      return string;
    }

  if (number->isNegative)
    [string appendString: @"-"];

  size = number->length + number->exponent;
  if ((number->length <= 6) && (0 < size) && (size < 7))
    {
      // For small numbers use the normal format
      for (i = 0; i < number->length; i++)
        {
	  if (size == i)
	    [string appendString: sep];
	  d = number->cMantissa[i];
	  s = [NSString stringWithFormat: @"%d", d];
	  [string appendString: s];
	}
      for (i = 0; i < number->exponent; i++)
        {
	  [string appendString: @"0"];
	}
    }
  else if ((number->length <= 6) && (0 >= size) && (size > -3))
    {
      // For small numbers use the normal format
      [string appendString: @"0"];
      [string appendString: sep];

      for (i = 0; i > size; i--)
        {
	  [string appendString: @"0"];
	}
      for (i = 0; i < number->length; i++)
        {
	  d = number->cMantissa[i];
	  s = [NSString stringWithFormat: @"%d", d];
	  [string appendString: s];
	}
    }
  else
    {
      // Scientific format
      for (i = 0; i < number->length; i++)
        {
	  if (1 == i)
	    [string appendString: sep];
	  d = number->cMantissa[i];
	  s = [NSString stringWithFormat: @"%d", d];
	  [string appendString: s];
	}
      if (size != 1)
        {
	  s = [NSString stringWithFormat: @"E%d", size-1];
	  [string appendString: s];
	}
    }

  return string;
}

// GNUstep extensions to make the implementation of NSDecimalNumber totaly
// independent for NSDecimals internal representation

// Give back the biggest NSDecimal
void
NSDecimalMax(NSDecimal *result)
{
  // FIXME: this is too small
  NSDecimalFromComponents(result, 9, 127, NO);
}

// Give back the smallest NSDecimal
void
NSDecimalMin(NSDecimal *result)
{
  // This is the smallest possible not the smallest positive number
  // FIXME: this is too big
  NSDecimalFromComponents(result, 9, 127, YES);
}

// Give back the value of a NSDecimal as a double
static double
GSDecimalDouble(GSDecimal *number)
{
  double d = 0.0;
  int i;

  if (!number->validNumber)
    return NAN;

  // Sum up the digits
  for (i = 0; i < number->length; i++)
    {
      d *= 10;
      d += number->cMantissa[i];
    }

  // multiply with the exponent
  // There is also a GNU extension pow10!!
  d *= pow(10, number->exponent);

  if (number->isNegative)
    d = -d;

  return d;
}


// Create a NSDecimal with a cMantissa, exponent and a negative flag
static void
GSDecimalFromComponents(GSDecimal *result, unsigned long long mantissa,
			short exponent, BOOL negative)
{
  unsigned char digit;
  int i, j;
  result->isNegative = negative;
  result->exponent = exponent;
  result->validNumber = YES;

  i = 0;
  while (mantissa)
    {
      digit = mantissa % 10;
      // Store the digit starting from the end of the array
      result->cMantissa[NSDecimalMaxDigit-i-1] = digit;
      mantissa = mantissa / 10;
      i++;
    }

  for (j = 0; j < i; j++)
    {
      // Move the digits to the beginning
      result->cMantissa[j] = result->cMantissa[j + NSDecimalMaxDigit-i];
    }

  result->length = i;

  GSDecimalCompact(result);
}

// Create a NSDecimal from a string using the local
static void
GSDecimalFromString(GSDecimal *result, NSString *numberValue,
		    NSDictionary *locale)
{
  NSRange found;
  NSString *sep = [locale objectForKey: NSDecimalSeparator];
  const char *s;
  int i;

  if (nil == sep)
    sep = @".";

  result->isNegative = NO;
  result->exponent = 0;
  result->validNumber = YES;
  result->length = 0;

  found = [numberValue rangeOfString: sep];
  if (found.length)
    {
      s = [[numberValue substringToIndex: found.location] lossyCString];
      if ('-' == *s)
        {
	  result->isNegative = YES;
	  s++;
	}
      while ((*s) && (!isdigit(*s))) s++;
      i = 0;
      while ((*s) && (isdigit(*s)))
        {
	  result->cMantissa[i++] = *s - '0';
	  result->length++;
	  s++;
	}
      s = [[numberValue substringFromIndex: NSMaxRange(found)] lossyCString];
      while ((*s) && (isdigit(*s)))
        {
	  result->cMantissa[i++] = *s - '0';
	  result->length++;
	  result->exponent--;
	  s++;
	}	
    }
  else
    {
      s = [numberValue lossyCString];
      if ('-' == *s)
        {
	  result->isNegative = YES;
	  s++;
	}
      while ((*s) && (!isdigit(*s))) s++;
      i = 0;
      while ((*s) && (isdigit(*s)))
        {
	  result->cMantissa[i++] = *s - '0';
	  result->length++;
	  s++;
	}
    }

  if ((*s == 'e') || (*s == 'E'))
    {
      s++;
      result->exponent += atoi(s);
    }

  if (!result->length)
    result->validNumber = NO;

  GSDecimalCompact(result);
}

#if	USE_GMP

static void CharvecToDecimal(const GSDecimal *m, NSDecimal *n)
{
  // Convert from a GSDecimal to a NSDecimal
  n->exponent = m->exponent;
  n->isNegative = m->isNegative;
  n->validNumber = m->validNumber;

  if (0 == m->length)
    n->size = 0;
  else
    {
      n->size = mpn_set_str(n->lMantissa, m->cMantissa, m->length, 10);
    }
}

static void DecimalToCharvec(const NSDecimal *n, GSDecimal *m)
{
  // Convert from a NSDecimal to a GSDecimal
  m->exponent = n->exponent;
  m->isNegative = n->isNegative;
  m->validNumber = n->validNumber;

  if (0 == n->size)
    {
      m->length = 0;
    }
  else
    {
      NSDecimal n1;

      NSDecimalCopy(&n1, n);
      m->length = mpn_get_str(m->cMantissa, 10, n1.lMantissa, n->size);
      // Do a compact only if the first digit is zero
      if (0 == m->cMantissa[0])
	GSDecimalCompact(m);
    }
}

void
NSDecimalCompact(NSDecimal *number)
{
  GSDecimal m;

  DecimalToCharvec(number, &m);
  GSDecimalCompact(&m);
  // FIXME: Here we need a check if the string fits into a GSDecimal,
  // if not we must round some limbs off.
  if (NSDecimalMaxDigit < m.length)
    GSDecimalRound(&m, NSDecimalMaxDigit - m.exponent, NSRoundPlain);
  CharvecToDecimal(&m, number);
}

NSComparisonResult
NSDecimalCompare(const NSDecimal *leftOperand, const NSDecimal *rightOperand)
{
  GSDecimal m1;
  GSDecimal m2;

  DecimalToCharvec(leftOperand, &m1);
  DecimalToCharvec(rightOperand, &m2);
  return GSDecimalCompare(&m1, &m2);
}

static NSComparisonResult
NSSimpleCompare(const NSDecimal *leftOperand, const NSDecimal *rightOperand)
{
  // This only checks the size of the operands.
  if (leftOperand->size == rightOperand->size)
    return NSOrderedSame;
  else if (leftOperand->size > rightOperand->size)
    return NSOrderedDescending;
  else
    return NSOrderedAscending;
}

void
NSDecimalRound(NSDecimal *result, const NSDecimal *number, NSInteger scale,
	       NSRoundingMode mode)
{
  GSDecimal m;

  DecimalToCharvec(number, &m);
  GSDecimalRound(&m, scale, mode);
  CharvecToDecimal(&m, result);
}

NSCalculationError
NSDecimalNormalize(NSDecimal *n1, NSDecimal *n2, NSRoundingMode mode)
{
  NSCalculationError error;
  GSDecimal m1;
  GSDecimal m2;

  if (!n1->validNumber || !n2->validNumber)
    return NSCalculationNoError;

  // Do they have the same exponent already?
  if (n1->exponent == n2->exponent)
    return NSCalculationNoError;

  DecimalToCharvec(n1, &m1);
  DecimalToCharvec(n2, &m2);
/*
  NSLog(@"Normalize n1 %@ n2 %@", NSDecimalString(n1, nil),
	NSDecimalString(n2, nil));
  NSLog(@"Normalize m1 %@ m2 %@", GSDecimalString(&m1, nil),
	GSDecimalString(&m2, nil));
*/
  error = GSDecimalNormalize(&m1, &m2, mode);
  CharvecToDecimal(&m1, n1);
  CharvecToDecimal(&m2, n2);
/*
  NSLog(@"Normalized m1 %@ m2 %@", GSDecimalString(&m1, nil),
	GSDecimalString(&m2, nil));
  NSLog(@"Normalized n1 %@ n2 %@", NSDecimalString(n1, nil),
	NSDecimalString(n2, nil));
*/
}

static NSCalculationError
GSSimpleAdd(NSDecimal *result, const NSDecimal *left, const NSDecimal *right,
	     NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  mp_limb_t carry;

  NSDecimalCopy(result, left);
  if (0 == right->size)
    return error;

  carry = mpn_add(result->lMantissa, left->lMantissa, left->size,
		  right->lMantissa, right->size);
  result->size = left->size;

  // check carry
  if (carry)
   {
     result->lMantissa[result->size] = carry;
     result->size++;
   }

  return error;
}

static NSCalculationError
GSSimpleSubtract(NSDecimal *result, const NSDecimal *left,
		 const NSDecimal *right, NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  mp_limb_t borrow;
/*
  NSLog(@"SimpleSub left %@ right %@ size %d", NSDecimalString(left, nil),
	NSDecimalString(right, nil), right->size);
*/
  NSDecimalCopy(result, left);
  if (0 == right->size)
    return error;

  borrow = mpn_sub(result->lMantissa, left->lMantissa, left->size,
		  right->lMantissa, right->size);
  result->size = left->size;

  // check borrow
  if (borrow)
    NSLog(@"Impossible error in subtraction");

  return error;
}

static NSCalculationError
GSSimpleMultiply(NSDecimal *result, NSDecimal *left, NSDecimal *right, NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  mp_limb_t limb;
/*
  NSLog(@"SimpleMul left %@ right %@ size %d", NSDecimalString(left, nil),
	NSDecimalString(right, nil), right->size);
*/
  NSDecimalCopy(result, &zero);
  // FIXME: Make sure result is big enougth
  limb = mpn_mul(result->lMantissa, left->lMantissa, left->size,
			 right->lMantissa, right->size);

  if (limb)
    {
      result->size = left->size + right->size;
    }
  else
    {
      //NSLog(@"Limb not set");
      result->size = left->size + right->size - 1;
    }
//  NSLog(@"SimpleMul result %@", NSDecimalString(result, nil));

  return error;
}

static NSCalculationError
GSSimpleDivide(NSDecimal *result, const NSDecimal *left, const NSDecimal *right,
	       NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  mp_limb_t limb;
  mp_size_t x = 38 + right->size - left->size;
  NSDecimal n;

  NSDecimalCopy(&n, left);

  // FIXME: I don't understand how to do this
  limb = mpn_divrem (result->lMantissa, x, n.lMantissa, left->size,
		     right->lMantissa, right->size);

  return error;
}

NSString*
NSDecimalString(const NSDecimal *decimal, NSDictionary *locale)
{
  GSDecimal n;

  DecimalToCharvec(decimal, &n);
  return GSDecimalString(&n, locale);
}

double
NSDecimalDouble(NSDecimal *number)
{
  GSDecimal n;

  DecimalToCharvec(number, &n);
  return GSDecimalDouble(&n);
}

void
NSDecimalFromComponents(NSDecimal *result, unsigned long long mantissa,
			short exponent, BOOL negative)
{
  GSDecimal n;
  //GSDecimal n1;

  GSDecimalFromComponents(&n, mantissa, exponent, negative);
  CharvecToDecimal(&n, result);
  //NSLog(@"GSDecimal 1: %@", GSDecimalString(&n, nil));
  //NSLog(@"NSDecimal 1: %@", NSDecimalString(result, nil));
  //DecimalToCharvec(result, &n1);
  //NSLog(@"GSDecimal 2: %@", GSDecimalString(&n1, nil));
  //NSLog(@"NSDecimal 2: %@", NSDecimalString(result, nil));
}

void
NSDecimalFromString(NSDecimal *result, NSString *numberValue,
		    NSDictionary *locale)
{
  GSDecimal n;

  GSDecimalFromString(&n, numberValue, locale);
  CharvecToDecimal(&n, result);
}

#else

// First implementations of the functions defined in NSDecimal.h

void
NSDecimalCompact(NSDecimal *number)
{
  GSDecimalCompact(number);
}

NSComparisonResult
NSDecimalCompare(const NSDecimal *leftOperand, const NSDecimal *rightOperand)
{
  return GSDecimalCompare(leftOperand, rightOperand);
}

void
NSDecimalRound(NSDecimal *result, const NSDecimal *number, NSInteger scale,
	       NSRoundingMode mode)
{
  NSDecimalCopy(result, number);

  GSDecimalRound(result, scale, mode);
}

NSCalculationError
NSDecimalNormalize(NSDecimal *n1, NSDecimal *n2, NSRoundingMode mode)
{
  if (!n1->validNumber || !n2->validNumber)
    return NSCalculationNoError;

  // Do they have the same exponent already?
  if (n1->exponent == n2->exponent)
    return NSCalculationNoError;

  return GSDecimalNormalize(n1, n2, mode);
}

static NSComparisonResult
NSSimpleCompare(const NSDecimal *leftOperand, const NSDecimal *rightOperand)
{
  // This only checks the length of the operands.
  if (leftOperand->length == rightOperand->length)
    return NSOrderedSame;
  else if (leftOperand->length > rightOperand->length)
    return NSOrderedDescending;
  else
    return NSOrderedAscending;
}

static NSCalculationError
GSSimpleAdd(NSDecimal *result, const NSDecimal *left, const NSDecimal *right,
	    NSRoundingMode mode)
{
  // left and right are both valid and positive, non-zero. The have been normalized and
  // left is bigger than right. result, left and right all point to different entities.
  // Result will not be compacted.
  NSCalculationError error = NSCalculationNoError;
  int i, j, l, d;
  int carry = 0;

  NSDecimalCopy(result, left);
  j = left->length - right->length;
  l = right->length;

  // Add all the digits
  for (i = l-1; i >= 0; i--)
    {
      d = right->cMantissa[i] + result->cMantissa[i + j] + carry;
      if (d >= 10)
        {
	  d = d % 10;
	  carry = 1;
	}
      else
	carry = 0;

      result->cMantissa[i + j] = d;
    }

  if (carry)
    {
      for (i = j-1; i >= 0; i--)
	{
	  if (result->cMantissa[i] != 9)
	    {
	      result->cMantissa[i]++;
	      carry = 0;
	      break;
	    }
	  result->cMantissa[i] = 0;
	}

      if (carry)
	{
	  // The number must be shifted to the right
	  if (NSDecimalMaxDigit == result->length)
	    {
	      NSDecimalRound(result, result,
			     NSDecimalMaxDigit - 1 - result->exponent,
			     mode);
	    }

	  if (127 == result->exponent)
	    {
	      result->validNumber = NO;
	      error = NSCalculationOverflow;
	    }

	  for (i = result->length-1; i >= 0; i--)
	    {
	      result->cMantissa[i+1] = result->cMantissa[i];
	    }
	  result->cMantissa[0] = 1;
	  result->length++;
	}
    }

  return error;
}

static NSCalculationError
GSSimpleSubtract(NSDecimal *result, const NSDecimal *left,
		 const NSDecimal *right, NSRoundingMode mode)
{
  // left and right are both valid and positive, non-zero. The have been normalized and
  // left is bigger than right. result, left and right all point to different entities.
  // Result will not be compacted.
  NSCalculationError error = NSCalculationNoError;
  int i, j, l, d;
  int borrow = 0;

  j = left->length - right->length;
  NSDecimalCopy(result, left);
  l = right->length;

  // Now subtract all digits
  for (i = l-1; i >= 0; i--)
    {
      d = result->cMantissa[i + j] - right->cMantissa[i] - borrow;
      if (d < 0)
        {
	  d = d + 10;
	  borrow = 1;
	}
      else
	borrow = 0;

      result->cMantissa[i + j] = d;
    }

  if (borrow)
    {
      for (i = j-1; i >= 0; i--)
	{
	  if (result->cMantissa[i] != 0)
	    {
	      result->cMantissa[i]--;
	      break;
	    }
	  result->cMantissa[i] = 9;
	}

      if (-1 == i)
	{
	  NSLog(@"Impossible error in subtraction left: %@, right: %@",
		NSDecimalString(left, nil), NSDecimalString(right, nil));
	}
    }

  return error;
}

static NSCalculationError
GSSimpleMultiply(NSDecimal *result, NSDecimal *l, NSDecimal *r, NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  NSCalculationError error1;
  int i, j, d, e;
  int carry = 0;
  NSDecimal n;
  int exp = 0;

  NSDecimalCopy(result, &zero);
  n.validNumber = YES;
  n.isNegative = NO;

  // if l->length = 38 round one off
  if (NSDecimalMaxDigit == l->length)
    {
      exp = -l->exponent;
      NSDecimalRound(l, l, -1-l->exponent, mode);
      // This might changed more than one
      exp += l->exponent;
    }

  // Do every digit of the second number
  for (i = 0; i < r->length; i++)
    {
      n.length = l->length+1;
      n.exponent = r->length - i - 1;
      carry = 0;
      d = r->cMantissa[i];

      if (0 == d)
	continue;

      for (j = l->length-1; j >= 0; j--)
        {
	  e = l->cMantissa[j] * d + carry;

	  if (e >= 10)
	    {
	      carry = e / 10;
	      e = e % 10;
	    }
	  else
	    carry = 0;
	  // This is one off to allow final carry
	  n.cMantissa[j+1] = e;
	}
      n.cMantissa[0] = carry;
      NSDecimalCompact(&n);
      error1 = NSDecimalAdd(result, result, &n, mode);
      if (NSCalculationNoError != error1)
	error = error1;
    }

  if (result->exponent + exp > 127)
    {
      // This should almost never happen
      result->validNumber = NO;
      return NSCalculationOverflow;
    }
  result->exponent += exp;
  return error;
}

static NSCalculationError
GSSimpleDivide(NSDecimal *result, const NSDecimal *l, const NSDecimal *r,
	       NSRoundingMode mode)
{
  NSCalculationError error = NSCalculationNoError;
  NSCalculationError error1;
  int k;
  int used; // How many digits of l have been used?
  NSDecimal n1;

  NSDecimalCopy(&n1, &zero);
  NSDecimalCopy(result, &zero);
  k = 0;
  used = 0;

  while ((k < l->length) || (n1.length))
    {
      while (NSOrderedAscending == NSDecimalCompare(&n1, r))
        {
	  if (NSDecimalMaxDigit-1 == k)
	    break;
	  if (n1.exponent)
	    {
              // Put back zeros removed by compacting
	      n1.cMantissa[(NSInteger)n1.length] = 0;
	      n1.length++;
	      n1.exponent--;
	    }
	  else
	    {
	      if (used < l->length)
	        {
		  // Fill up with own digits
		  if (n1.length || l->cMantissa[used])
		    {
		      // only add 0 if there is already something
		      n1.cMantissa[(NSInteger)n1.length] = l->cMantissa[used];
		      n1.length++;
		    }
		  used++;
		}
	      else
	        {
		  if (-128 == result->exponent)
		    {
		      // use this as an end flag
		      k = NSDecimalMaxDigit-1;
		      break;
		    }
		  // Borrow one digit
		  n1.cMantissa[(NSInteger)n1.length] = 0;
		  n1.length++;
		  result->exponent--;
		}
	      k++;
	      result->cMantissa[k-1] = 0;
	      result->length++;
	    }
	}

      if (NSDecimalMaxDigit-1 == k)
        {
	  error = NSCalculationLossOfPrecision;
	  break;
	}

      error1 = NSDecimalSubtract(&n1, &n1, r, mode);
      if (NSCalculationNoError != error1)
	error = error1;
      result->cMantissa[k-1]++;
    }

  return error;
}

NSString*
NSDecimalString(const NSDecimal *decimal, NSDictionary *locale)
{
  return GSDecimalString(decimal, locale);
}

// GNUstep extensions to make the implementation of NSDecimalNumber totaly
// independent for NSDecimals internal representation

double
NSDecimalDouble(NSDecimal *number)
{
  return GSDecimalDouble(number);
}

void
NSDecimalFromComponents(NSDecimal *result, unsigned long long mantissa,
		      short exponent, BOOL negative)
{
  GSDecimalFromComponents(result, mantissa, exponent, negative);
}

void
NSDecimalFromString(NSDecimal *result, NSString *numberValue,
		    NSDictionary *locale)
{
  GSDecimalFromString(result, numberValue, locale);
}

#endif
