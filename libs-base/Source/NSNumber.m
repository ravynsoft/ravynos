/** Implementation of NSNumber for GNUStep
   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  David Chisnall
   Partial rewrite:  Richard Frith-Macdonld <rfm@gnu.org>
    (to compile on gnu/linux and mswindows,
    to meet coding/style standards,
    to restore lost functionality)

   Date: February 2010

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


#import "common.h"

#if	!defined(LLONG_MAX)
#  if	defined(__LONG_LONG_MAX__)
#    define LLONG_MAX __LONG_LONG_MAX__
#    define LLONG_MIN	(-LLONG_MAX-1)
#    define ULLONG_MAX	(LLONG_MAX * 2ULL + 1)
#  else
#    error Neither LLONG_MAX nor __LONG_LONG_MAX__ found
#  endif
#endif


#import "common.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSDecimalNumber.h"
#import "Foundation/NSException.h"
#import "Foundation/NSValue.h"
#if __has_include(<objc/runtime.h>)
#  include <objc/runtime.h>
#endif

/*
 * NSNumber implementation.  This matches the behaviour of Apple's
 * implementation.  Values in the range -1 to 12 inclusive are mapped to
 * singletons.  All other values are mapped to the smallest signed value that
 * will store them, unless they are greater than LLONG_MAX, in which case
 * they are stored in an unsigned long long.
 * Booleans are handled as a special case since some stuff (notably interface
 * builder (nib) archives) needs to differentiate between booleans and integers.
 */

@interface NSSignedIntegerNumber : NSNumber
@end

@interface NSIntNumber : NSSignedIntegerNumber
{
@public
  int value;
}
@end

/* Some code needs to differentiate between booleans and other NSNumber
 * instances, so we need a special subclass to permit that.
 */
@interface NSBoolNumber : NSIntNumber
@end

@interface NSLongLongNumber : NSSignedIntegerNumber
{
@public
  long long int value;
}
@end

@interface NSUnsignedLongLongNumber : NSNumber
{
@public
  unsigned long long int value;
}
@end

// The value ivar in all of the concrete classes contains the real value.
#define VALUE value
#define COMPARE(value, other) \
if (value < other)\
  {\
    return NSOrderedAscending;\
  }\
if (value > other)\
  {\
    return NSOrderedDescending;\
  }\
return NSOrderedSame;

#define DCOMPARE(value, other) \
  if (isnan(value)) \
    { \
      if (isnan(other)) \
	{ \
	  return NSOrderedSame; \
	} \
      else \
	{ \
	  return NSOrderedAscending; \
	} \
    } \
  else \
    { \
      if (isnan(other)) \
	{ \
	  if (value < 0.0) \
	    { \
	      return NSOrderedAscending; \
	    } \
	  return NSOrderedDescending; \
	} \
      else if (value < other) \
	{ \
	  return NSOrderedAscending; \
	} \
      else if (value > other) \
	{ \
	  return NSOrderedDescending; \
	} \
      return NSOrderedSame; \
    }

@implementation NSSignedIntegerNumber
- (NSComparisonResult) compare: (NSNumber*)aNumber
{
  if (aNumber == self)
    {
      return NSOrderedSame;
    }
  if (aNumber == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"nil argument for compare:"];
    }

  switch ([aNumber objCType][0])
    {
      /* For cases smaller than or equal to an int, we could get the int
       * value and compare.
       */
      case 'c':
      case 'C':
      case 's':
      case 'S':
      case 'i':
      case 'I':
      case 'l':
      case 'L':
      case 'q':
	{
	  long long value = [self longLongValue];
	  long long other = [aNumber longLongValue];

	  COMPARE (value, other);
	}
      case 'Q':
	{
	  unsigned long long other;
	  unsigned long long value;
	  long long v;

	  /* According to the C type promotion rules, we should cast this to
	   * an unsigned long long, however Apple's code does not do this.
	   * Instead, it performs a real comparison.
	   */
	  v = [self longLongValue];

	  /* If this value is less than 0, then it is less than any value
	   * that can possibly be stored in an unsigned value.
	   */
	  if (v < 0)
	    {
	      return NSOrderedAscending;
	    }

	  other = [aNumber unsignedLongLongValue];
	  value = (unsigned long long) v;
	  COMPARE (value, other);
	}
      case 'f':
      case 'd':
	{
	  double other = [aNumber doubleValue];
	  double value = [self doubleValue];

	  DCOMPARE(value, other)
	}
      default:
	[NSException raise: NSInvalidArgumentException
		    format: @"unrecognised type for compare:"];
    }
  return 0;			// Not reached.
}
@end

@implementation NSIntNumber
#define FORMAT @"%i"
#include "NSNumberMethods.h"
@end

@implementation	NSBoolNumber
- (void) getValue: (void*)buffer
{
  BOOL *ptr = (BOOL*)buffer;
  *ptr = VALUE;
}
- (const char *) objCType
{
  return @encode(BOOL);
}
@end

@implementation NSLongLongNumber
#define FORMAT @"%lli"
#include "NSNumberMethods.h"
@end

@implementation NSUnsignedLongLongNumber
#define FORMAT @"%llu"
#include "NSNumberMethods.h"
- (NSComparisonResult) compare: (NSNumber*)aNumber
{
  if (aNumber == self)
    {
      return NSOrderedSame;
    }
  if (aNumber == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"nil argument for compare:"];
    }

  switch ([aNumber objCType][0])
    {
      /* For cases smaller than or equal to an int, we could get the int
       * value and compare.
       */
      case 'c':
      case 'C':
      case 's':
      case 'S':
      case 'i':
      case 'I':
      case 'l':
      case 'L':
      case 'q':
	{
	  long long other = [aNumber longLongValue];

	  if (other < 0)
	    {
	      return NSOrderedDescending;
	    }
	  COMPARE (value, ((unsigned long long) other));
	}
      case 'Q':
	{
	  unsigned long long other = [aNumber unsignedLongLongValue];

	  COMPARE (value, other);
	}
      case 'f':
      case 'd':
	{
	  double other = [aNumber doubleValue];
	  double selfv = [self doubleValue];

	  DCOMPARE(selfv, other)
	}
      default:
	[NSException raise: NSInvalidArgumentException
		    format: @"unrecognised type for compare:"];
    }
  return 0;			// Not reached.
}
@end

/*
 * Abstract superclass for floating point numbers.
 */
@interface NSFloatingPointNumber : NSNumber
@end

@implementation NSFloatingPointNumber
/* For floats, the type promotion rules say that we always promote to a
 * floating point type, even if the other value is really an integer.
 */
- (BOOL) isEqualToNumber: (NSNumber*)aNumber
{
  return ([self doubleValue] == [aNumber doubleValue]) ? YES : NO;
}

- (NSComparisonResult) compare: (NSNumber*)aNumber
{
  double other;
  double value;

  if (aNumber == self)
    {
      return NSOrderedSame;
    }
  if (aNumber == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"nil argument for compare:"];
    }

  other = [aNumber doubleValue];
  value = [self doubleValue];

  DCOMPARE(value, other)
}
@end

@interface NSFloatNumber : NSFloatingPointNumber
{
@public
  float value;
}
@end

@implementation NSFloatNumber
#define FORMAT @"%0.7g"
#include "NSNumberMethods.h"
@end

@interface NSDoubleNumber : NSFloatingPointNumber
{
@public
  double value;
}
@end

@implementation NSDoubleNumber
#define FORMAT @"%0.16g"
#include "NSNumberMethods.h"
@end

#ifdef OBJC_SMALL_OBJECT_SHIFT
static BOOL useSmallInt;
#if OBJC_SMALL_OBJECT_SHIFT == 3
static BOOL useSmallExtendedDouble;
static BOOL useSmallRepeatingDouble;
static BOOL useSmallFloat;
#endif
#define SMALL_INT_MASK 1
#define SMALL_EXTENDED_DOUBLE_MASK 2
#define SMALL_REPEATING_DOUBLE_MASK 3
// 4 is GSTinyString
#define SMALL_FLOAT_MASK 5

@interface NSSmallInt : NSSignedIntegerNumber
@end

@implementation NSSmallInt
#undef VALUE
#define VALUE (((intptr_t)self) >> OBJC_SMALL_OBJECT_SHIFT)
#define FORMAT @"%"PRIdPTR
#include "NSNumberMethods.h"

+ (void) load
{
  useSmallInt = objc_registerSmallObjectClass_np(self, SMALL_INT_MASK);
}

+ (id) alloc
{
  return (id)1;
}

+ (id) allocWithZone: (NSZone*)aZone
{
  return (id)1;
}

- (id) copy
{
  return self;
}

- (id) copyWithZone: (NSZone*)aZone
{
  return self;
}

- (id) retain
{
  return self;
}

- (NSUInteger) retainCount
{
  return UINT_MAX;
}

- (id) autorelease
{
  return self;
}

- (oneway void) release
{
  return;
}
@end

#if OBJC_SMALL_OBJECT_SHIFT == 3

union BoxedDouble
{
  id obj;
  uintptr_t bits;
  double d;
};

@interface NSSmallExtendedDouble : NSFloatingPointNumber
@end

@implementation NSSmallExtendedDouble
static double
unboxSmallExtendedDouble(uintptr_t boxed)
{
  // The low bit of the mantissa
  uintptr_t mask = boxed & 8;
  union BoxedDouble ret;
  // Clear the class pointer
  boxed &= ~7;
  ret.bits = boxed | (mask >> 1) | (mask >> 2) | (mask >> 3);
  return ret.d;
}

static BOOL
isSmallExtendedDouble(double d)
{
  union BoxedDouble b = {.d=d};
  return unboxSmallExtendedDouble(b.bits) == d;
}

static double
unboxSmallRepeatingDouble(uintptr_t boxed)
{
  // The low bit of the mantissa
  uintptr_t mask = boxed & 56;
  union BoxedDouble ret;
  // Clear the class pointer
  boxed &= ~7;
  ret.bits = boxed | (mask >> 3);
  return ret.d;
}

static BOOL
isSmallRepeatingDouble(double d)
{
  union BoxedDouble b = {.d=d};
  return unboxSmallRepeatingDouble(b.bits) == d;
}

static id
boxDouble(double d, uintptr_t mask)
{
  union BoxedDouble b = {.d=d};
  b.bits &= ~OBJC_SMALL_OBJECT_MASK;
  b.bits |= mask;
  return b.obj;
}

#undef VALUE
#define VALUE (unboxSmallExtendedDouble((uintptr_t)self))
#define FORMAT @"%0.16g"
#include "NSNumberMethods.h"

+ (void) load
{
  useSmallExtendedDouble = objc_registerSmallObjectClass_np
    (self, SMALL_EXTENDED_DOUBLE_MASK);
}

+ (id) alloc
{
  return (id)SMALL_EXTENDED_DOUBLE_MASK;
}

+ (id) allocWithZone: (NSZone*)aZone
{
  return (id)SMALL_EXTENDED_DOUBLE_MASK;
}

- (id) copy
{
  return self;
}

- (id) copyWithZone: (NSZone*)aZone
{
  return self;
}

- (id) retain
{
  return self;
}

- (NSUInteger) retainCount
{
  return UINT_MAX;
}

- (id) autorelease
{
  return self;
}

- (oneway void) release
{
  return;
}
@end

@interface NSSmallRepeatingDouble : NSFloatingPointNumber
@end

@implementation NSSmallRepeatingDouble
#undef VALUE
#define VALUE (unboxSmallRepeatingDouble((uintptr_t)self))
#define FORMAT @"%0.16g"
#include "NSNumberMethods.h"

+ (void) load
{
  useSmallRepeatingDouble = objc_registerSmallObjectClass_np
    (self, SMALL_REPEATING_DOUBLE_MASK);
}

+ (id) alloc
{
  return (id)SMALL_REPEATING_DOUBLE_MASK;
}

+ (id) allocWithZone: (NSZone*)aZone
{
  return (id)SMALL_REPEATING_DOUBLE_MASK;
}

- (id) copy
{
  return self;
}

- (id) copyWithZone: (NSZone*)aZone
{
  return self;
}

- (id) retain
{
  return self;
}

- (NSUInteger) retainCount
{
  return UINT_MAX;
}

- (id) autorelease
{
  return self;
}

- (oneway void) release
{
  return;
}
@end


/*
 * Technically, all floats are small on 64bit and fit into a NSRepeatingDouble,
 * but we want to get the description FORMAT right for floats (i.e. "%0.7g" and
 * not "%0.16g".
 */
@interface NSSmallFloat : NSSmallRepeatingDouble
@end
@implementation NSSmallFloat
#undef VALUE
#define VALUE (unboxSmallRepeatingDouble((uintptr_t)self))
#define FORMAT @"%0.7g"
#include "NSNumberMethods.h"

+ (void) load
{
  useSmallFloat = objc_registerSmallObjectClass_np
    (self, SMALL_FLOAT_MASK);
}

+ (id) alloc
{
  return (id)SMALL_FLOAT_MASK;
}

+ (id) allocWithZone: (NSZone*)aZone
{
  return (id)SMALL_FLOAT_MASK;
}
@end


#endif
#endif

@implementation NSNumber

static Class NSNumberClass;
static Class NSBoolNumberClass;
static Class NSIntNumberClass;
static Class NSLongLongNumberClass;
static Class NSUnsignedLongLongNumberClass;
static Class NSFloatNumberClass;
static Class NSDoubleNumberClass;

/*
 * Numbers from -1 to 12 inclusive that are reused.
 */
static NSNumber *ReusedInstances[14];
static NSBoolNumber *boolY;		// Boolean YES (integer 1)
static NSBoolNumber *boolN;		// Boolean NO (integer 0)

+ (void) initialize
{
  int i;

  if ([NSNumber class] != self)
    {
      return;
    }

  NSNumberClass = self;
  NSBoolNumberClass = [NSBoolNumber class];
  NSIntNumberClass = [NSIntNumber class];
  NSLongLongNumberClass = [NSLongLongNumber class];
  NSUnsignedLongLongNumberClass = [NSUnsignedLongLongNumber class];
  NSFloatNumberClass = [NSFloatNumber class];
  NSDoubleNumberClass = [NSDoubleNumber class];

  boolY = NSAllocateObject(NSBoolNumberClass, 0, 0);
  [[NSObject leakAt: &boolY] release];
  boolY->value = 1;
  boolN = NSAllocateObject(NSBoolNumberClass, 0, 0);
  boolN->value = 0;
  [[NSObject leakAt: &boolN] release];

  for (i = 0; i < 14; i++)
    {
      NSIntNumber *n = NSAllocateObject(NSIntNumberClass, 0, 0);

      n->value = i - 1;
      ReusedInstances[i] = n;
      [[NSObject leakAt: &ReusedInstances[i]] release];
    }
}

- (const char *) objCType
{
  /* All concrete NSNumber types must implement this so we know which one
   * they are.
   */
  [self subclassResponsibility: _cmd];
  return NULL;			// Not reached
}

- (BOOL) isEqualToNumber: (NSNumber*)aNumber
{
  return ([self compare: aNumber] == NSOrderedSame) ? YES : NO;
}

- (BOOL) isEqual: (id)anObject
{
  if ([anObject isKindOfClass: NSNumberClass])
    {
      return [self isEqualToNumber: anObject];
    }
  return [super isEqual: anObject];
}

- (BOOL) isEqualToValue: (NSValue*)aValue
{
  if ([aValue isKindOfClass: NSNumberClass])
    {
      return [self isEqualToNumber: (NSNumber*)aValue];
    }
  return NO;
}

- (NSUInteger) hash
{
  return (unsigned)[self doubleValue];
}

- (NSString*) stringValue
{
  return [self descriptionWithLocale: nil];
}

- (NSString*) descriptionWithLocale: (id)aLocale
{
  [self subclassResponsibility: _cmd];
  return nil;			// Not reached
}

- (NSComparisonResult) compare: (NSNumber*)aNumber
{
  [self subclassResponsibility: _cmd];
  return 0;			// Not reached
}

#define INTEGER_MACRO(encoding,type, ignored, name) \
- (id) initWith ## name: (type)aValue \
{\
  DESTROY(self);\
  return [[NSNumberClass numberWith ## name: aValue] retain];\
}

#include "GSNumberTypes.h"

- (id) initWithBool: (BOOL)aValue
{
  DESTROY(self);
  return [(aValue == 0 ? boolN : boolY) retain];\
}

/*
 * Macro for checking whether this value is the same as one of the singleton
 * instances.
 */
#define CHECK_SINGLETON(aValue) \
if (aValue >= -1 && aValue <= 12)\
{\
  return ReusedInstances[aValue+1];\
}

+ (NSNumber *) numberWithBool: (BOOL)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(BOOL)] autorelease];
    }
  if (0 == aValue)
    {
      return boolN;
    }
  return boolY;
}

+ (NSNumber *) numberWithChar: (signed char)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(signed char)] autorelease];
    }
  return [self numberWithInt: aValue];
}

+ (NSNumber *) numberWithUnsignedChar: (unsigned char)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(unsigned char)] autorelease];
    }
  return [self numberWithInt: aValue];
}

+ (NSNumber *) numberWithShort: (short)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(short)] autorelease];
    }
  return [self numberWithInt: aValue];
}

+ (NSNumber *) numberWithUnsignedShort: (unsigned short)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(unsigned short)] autorelease];
    }
  return [self numberWithInt: aValue];
}

+ (NSNumber *) numberWithInt: (int)aValue
{
  NSIntNumber *n;

  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(int)] autorelease];
    }

  CHECK_SINGLETON(aValue);
#ifdef OBJC_SMALL_OBJECT_SHIFT
  if (useSmallInt
    && (aValue < (INT_MAX>>OBJC_SMALL_OBJECT_SHIFT))
    && (aValue > -(INT_MAX>>OBJC_SMALL_OBJECT_SHIFT)))
    {
      return (id)((((NSInteger)aValue) << OBJC_SMALL_OBJECT_SHIFT)
        | SMALL_INT_MASK);
    }
#endif
  n = NSAllocateObject(NSIntNumberClass, 0, 0);
  n->value = aValue;
  return AUTORELEASE(n);
}

+ (NSNumber *) numberWithUnsignedInt: (unsigned int)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(unsigned int)] autorelease];
    }

  CHECK_SINGLETON(aValue);
  if (aValue < (unsigned int) INT_MAX)
    {
      return [self numberWithInt: (int)aValue];
    }
  return [self numberWithLongLong: aValue];
}

+ (NSNumber *) numberWithLong: (long)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(long)] autorelease];
    }
  return [self numberWithLongLong: aValue];
}

+ (NSNumber *) numberWithUnsignedLong: (unsigned long)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(unsigned long)] autorelease];
    }
  return [self numberWithUnsignedLongLong: aValue];
}

+ (NSNumber *) numberWithLongLong: (long long)aValue
{
  NSLongLongNumber *n;

  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(long long)] autorelease];
    }
  CHECK_SINGLETON(aValue);
  if (aValue < (long long)INT_MAX && aValue > (long long)INT_MIN)
    {
      return [self numberWithInt: (int) aValue];
    }
  n = NSAllocateObject(NSLongLongNumberClass, 0, 0);
  n->value = aValue;
  return AUTORELEASE(n);
}

+ (NSNumber *) numberWithUnsignedLongLong: (unsigned long long)aValue
{
  NSUnsignedLongLongNumber *n;

  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(unsigned long long)] autorelease];
    }
  if (aValue < (unsigned long long) LLONG_MAX)
    {
      return [self numberWithLongLong: (long long) aValue];
    }
  n = NSAllocateObject(NSUnsignedLongLongNumberClass, 0, 0);
  n->value = aValue;
  return AUTORELEASE(n);
}

+ (NSNumber *) numberWithFloat: (float)aValue
{
  NSFloatNumber *n;

  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(float)] autorelease];
    }
#if OBJC_SMALL_OBJECT_SHIFT == 3
  if (useSmallFloat)
    {
      return boxDouble(aValue, SMALL_FLOAT_MASK);
    }
#endif
  n = NSAllocateObject(NSFloatNumberClass, 0, 0);
  n->value = aValue;
  return AUTORELEASE(n);
}

+ (NSNumber *) numberWithDouble: (double)aValue
{
  NSDoubleNumber *n;

  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(double)] autorelease];
    }
#if OBJC_SMALL_OBJECT_SHIFT == 3
  if (useSmallRepeatingDouble && isSmallRepeatingDouble(aValue))
    {
      return boxDouble(aValue, SMALL_REPEATING_DOUBLE_MASK);
    }
  if (useSmallExtendedDouble && isSmallExtendedDouble(aValue))
    {
      return boxDouble(aValue, SMALL_EXTENDED_DOUBLE_MASK);
    }
#endif
  n = NSAllocateObject(NSDoubleNumberClass, 0, 0);
  n->value = aValue;
  return AUTORELEASE(n);
}

+ (NSNumber *) numberWithInteger: (NSInteger)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(NSInteger)] autorelease];
    }
  // Compile time constant; the compiler will remove this conditional
  if (sizeof (NSInteger) == sizeof (int))
    {
      return [self numberWithInt: aValue];
    }
  return [self numberWithLongLong: aValue];
}

+ (NSNumber *) numberWithUnsignedInteger: (NSUInteger)aValue
{
  if (self != NSNumberClass)
    {
      return [[[self alloc] initWithBytes: (const void *)&aValue
        objCType: @encode(NSUInteger)] autorelease];
    }
  // Compile time constant; the compiler will remove this conditional
  if (sizeof (NSUInteger) == sizeof (unsigned int))
    {
      return [self numberWithUnsignedInt: aValue];
    }
  return [self numberWithUnsignedLongLong: aValue];
}

- (id) initWithBytes: (const void *)
      value objCType: (const char *)type
{
  switch (type[0])
    {
      case 'c':
	return [self initWithInteger: *(signed char *) value];
      case 'C':
	return [self initWithInteger: *(unsigned char *) value];
      case 's':
	return [self initWithInteger: *(short *) value];
      case 'S':
	return [self initWithInteger: *(unsigned short *) value];
      case 'i':
	return [self initWithInteger: *(int *) value];
      case 'I':
	return [self initWithInteger: *(unsigned int *) value];
      case 'l':
	return [self initWithLong: *(long *) value];
      case 'L':
	return [self initWithUnsignedLong: *(unsigned long *) value];
      case 'q':
	return [self initWithLongLong: *(long long *) value];
      case 'Q':
	return [self initWithUnsignedLongLong: *(unsigned long long *) value];
      case 'f':
	return [self initWithFloat: *(float *) value];
      case 'd':
	return [self initWithDouble: *(double *) value];
    }
  return [super initWithBytes: value objCType: type];
}

- (void *) pointerValue
{
  return (void *)[self unsignedIntegerValue];
}

- (id) replacementObjectForPortCoder: (NSPortCoder *) encoder
{
  return self;
}

- (Class) classForCoder
{
  return NSNumberClass;
}

- (void) encodeWithCoder: (NSCoder *) coder
{
  const char *type = [self objCType];
  unsigned char charbuf;
  unsigned short shortbuf;
  unsigned int intbuf;
  unsigned long longbuf;
  unsigned long long llongbuf;
  float floatbuf;
  double doublebuf;
  void  *buffer;

  [coder encodeValueOfObjCType: @encode(char) at: type];

  switch (type[0])
    {
      case 'c':
      case 'C':
        buffer = &charbuf; break;
      case 's':
      case 'S':
        buffer = &shortbuf; break;
      case 'i':
      case 'I':
        buffer = &intbuf; break;
      case 'l':
      case 'L':
        buffer = &longbuf; break;
      case 'q':
      case 'Q':
        buffer = &llongbuf; break;
      case 'f':
        buffer = &floatbuf; break;
      case 'd':
        buffer = &doublebuf; break;
      default:
        [NSException raise: NSInternalInconsistencyException
                    format: @"unknown NSNumber type '%s'", type];
        return; // Avoid spurious compiler warning.
    }

  [self getValue: buffer];
  [coder encodeValueOfObjCType: type at: buffer];
}

- (id) copyWithZone: (NSZone *) aZone
{
  // OSX just returns the receive with no copy.
  return RETAIN (self);
}

- (id) initWithCoder: (NSCoder *) coder
{
  char type[2] = { 0 };
  unsigned char charbuf;
  unsigned short shortbuf;
  unsigned int intbuf;
  unsigned long longbuf;
  unsigned long long llongbuf;
  float floatbuf;
  double doublebuf;
  void *buffer;

  [coder decodeValueOfObjCType: @encode(char) at: type];
  switch (type[0])
    {
      case 'c':
      case 'C':
        buffer = &charbuf; break;
      case 's':
      case 'S':
        buffer = &shortbuf; break;
      case 'i':
      case 'I':
        buffer = &intbuf; break;
      case 'l':
      case 'L':
        buffer = &longbuf; break;
      case 'q':
      case 'Q':
        buffer = &llongbuf; break;
      case 'f':
        buffer = &floatbuf; break;
      case 'd':
        buffer = &doublebuf; break;
      default:
        [NSException raise: NSInternalInconsistencyException
                    format: @"unknown NSNumber type '%c'", type[0]];
        return nil;     // Avoid spurious compiler warning.
     }
  [coder decodeValueOfObjCType: type at: buffer];
  return [self initWithBytes: buffer objCType: type];
}

- (NSString *) description
{
  return [self stringValue];
}

/* Return nil for an NSNumber that is allocated and initialized without
 * providing a real value.  Yes, this seems weird, but it is actually what
 * happens on OS X.
 */
- (id) init
{
  if (object_getClass(self) != NSNumberClass)
    {
      return [super init];
    }
  DESTROY(self);
  return nil;
}

/* Stop the compiler complaining about unimplemented methods.  Throwing an
 * exception here matches OS X behaviour, although they throw an invalid
 * argument exception.
 */
#define INTEGER_MACRO(encoding, type, name, ignored) \
- (type) name ## Value\
{\
  [self subclassResponsibility: _cmd];\
  return (type)0;\
}

#include "GSNumberTypes.h"

- (BOOL) boolValue
{
  [self subclassResponsibility: _cmd];
  return NO;
}

- (NSDecimal) decimalValue
{
  NSDecimalNumber *dn;
  NSDecimal decimal;

  dn = [[NSDecimalNumber alloc] initWithString: [self stringValue]];
  decimal = [dn decimalValue];
  [dn release];
  return decimal;
}

@end
