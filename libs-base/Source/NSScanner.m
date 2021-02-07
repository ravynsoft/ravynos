/** Implemenation of NSScanner class
   Copyright (C) 1996,1999 Free Software Foundation, Inc.

   Author:  Eric Norum <eric@skatter.usask.ca>
   Date: 1996
   Rewrite/optimisation by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: 1998

   This file is part of the GNUstep Objective-C Library.

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

   <title>NSScanner class reference</title>
   $Date$ $Revision$
*/

#import "common.h"

#if	defined(HAVE_FLOAT_H)
#include	<float.h>
#endif

#if	!defined(LLONG_MAX)
#  if	defined(__LONG_LONG_MAX__)
#    define LLONG_MAX __LONG_LONG_MAX__
#    define LLONG_MIN	(-LLONG_MAX-1)
#    define ULLONG_MAX	(LLONG_MAX * 2ULL + 1)
#  else
#    error Neither LLONG_MAX nor __LONG_LONG_MAX__ found
#  endif
#endif
#if	!defined(ULLONG_MAX)
#  define ULLONG_MAX	(LLONG_MAX * 2ULL + 1)
#endif

#include <math.h>
#include <ctype.h>    /* FIXME: May go away once I figure out Unicode */

#define	EXPOSE_NSScanner_IVARS	1
#import "GNUstepBase/Unicode.h"
#import "Foundation/NSScanner.h"
#import "Foundation/NSException.h"
#import "Foundation/NSUserDefaults.h"

#import "GSPThread.h"

#import "GSPrivate.h"


@class	GSCString;
@interface GSCString : NSObject	// Help the compiler
@end
@class	GSUnicodeString;
@interface GSUnicodeString : NSObject	// Help the compiler
@end
@class	GSMutableString;
@class	GSPlaceholderString;
@interface GSPlaceholderString : NSObject	// Help the compiler
@end

static Class		NSStringClass;
static Class		GSCStringClass;
static Class		GSUnicodeStringClass;
static Class		GSMutableStringClass;
static Class		GSPlaceholderStringClass;
static id		_holder;
static NSString		*_empty;
static NSCharacterSet	*defaultSkipSet;
static SEL		memSel;
static NSStringEncoding internalEncoding = NSISOLatin1StringEncoding;

/* Table of binary powers of 10 represented by bits in a byte.
 * Used to convert decimal integer exponents to doubles.
 */
static double powersOf10[] = {
  1.0e1, 1.0e2, 1.0e4, 1.0e8, 1.0e16, 1.0e32, 1.0e64, 1.0e128, 1.0e256
};

static inline unichar myGetC(unsigned char c)
{
  unsigned int  size = 1;
  unichar       u = 0;
  unichar       *dst = &u;

  /* If this fails, u is zero (unchanged) ...  return that rather
   * than raising an exception.
   */
  (void)GSToUnicode(&dst, &size, &c, 1, internalEncoding, 0, 0);
  return u;
}
/*
 * Hack for direct access to internals of an concrete string object.
 */
typedef GSString	*ivars;
#define	myLength()	(((ivars)_string)->_count)
#define	myByte(I)	(((ivars)_string)->_contents.c[I])
#define	myUnichar(I)	(((ivars)_string)->_contents.u[I])
#define	myChar(I)	myGetC((((ivars)_string)->_contents.c[I]))
#define	myCharacter(I)	(_isUnicode ? myUnichar(I) : myChar(I))
/* Macro for getting character values when we do not care about values
 * outside the ASCII range (other than to know they are outside the range).
 */
#define	mySevenBit(I)	(_isUnicode ? myUnichar(I) : myByte(I))

/*
 * Scan characters to be skipped.
 * Return YES if there are more characters to be scanned.
 * Return NO if the end of the string is reached.
 * For internal use only.
 */
#define	skipToNextField()	({\
  while (_scanLocation < myLength() && _charactersToBeSkipped != nil \
    && (*_skipImp)(_charactersToBeSkipped, memSel, myCharacter(_scanLocation)))\
    _scanLocation++;\
  (_scanLocation >= myLength()) ? NO : YES;\
})


/**
 * <p>
 *   The <code>NSScanner</code> class cluster (currently a single class in
 *   GNUstep) provides a mechanism to parse the contents of a string into
 *   number and string values by making a sequence of scan operations to
 *   step through the string retrieving successive items.
 * </p>
 * <p>
 *   You can tell the scanner whether its scanning is supposed to be
 *   case sensitive or not, and you can specify a set of characters
 *   to be skipped before each scanning operation (by default,
 *   whitespace and newlines).
 * </p>
 */
@implementation NSScanner

+ (void) initialize
{
  if (self == [NSScanner class])
    {
      NSStringEncoding externalEncoding;

      memSel = @selector(characterIsMember:);
      defaultSkipSet = [NSCharacterSet whitespaceAndNewlineCharacterSet];
      IF_NO_GC(RETAIN(defaultSkipSet));
      NSStringClass = [NSString class];
      GSCStringClass = [GSCString class];
      GSUnicodeStringClass = [GSUnicodeString class];
      GSMutableStringClass = [GSMutableString class];
      GSPlaceholderStringClass = [GSPlaceholderString class];
      _holder = (id)NSAllocateObject(GSPlaceholderStringClass, 0, 0);
      _empty = [_holder initWithString: @""];
      externalEncoding = [NSString defaultCStringEncoding];
      if (GSPrivateIsByteEncoding(externalEncoding) == YES)
	{
	  internalEncoding = externalEncoding;
	}
    }
}

/**
 * Create and return a scanner that scans aString.<br />
 * Uses -initWithString: and with no locale set.
 */
+ (id) scannerWithString: (NSString *)aString
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithString: aString]);
}

/**
 * Returns an NSScanner instance set up to scan aString
 * (using -initWithString: and with a locale set the default locale
 * (using -setLocale:
 */
+ (id) localizedScannerWithString: (NSString*)aString
{
  NSScanner		*scanner = [self scannerWithString: aString];

  if (scanner != nil)
    {
      NSDictionary	*loc;

      loc = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
      [scanner setLocale: loc];
    }
  return scanner;
}

- (void) _setString: (NSString*)aString
{
  _scanLocation = 0;
  _isUnicode = NO;
  if (nil == aString)
    {
      aString = _empty;
    }
  if (aString != _string)
    {
      Class	c = object_getClass(aString);

      DESTROY(_string);
      if (GSObjCIsKindOf(c, GSMutableStringClass) == YES)
	{
	  _string = [_holder initWithString: aString];
	}
      else if (GSObjCIsKindOf(c, GSUnicodeStringClass) == YES)
	{
	  _string = RETAIN(aString);
	}
      else if (GSObjCIsKindOf(c, GSCStringClass) == YES)
	{
	  _string = RETAIN(aString);
	}
      else if (GSObjCIsKindOf(c, NSStringClass) == YES)
	{
	  _string = [_holder initWithString: aString];
	}
      else
	{
	  _string = [_holder initWithString: [aString description]];
	}
      c = object_getClass(_string);
      if (GSObjCIsKindOf(c, GSUnicodeStringClass) == YES)
	{
	  _isUnicode = YES;
	}
    }
}

/** Used by NSString/GSString to avoid creating/destroying a new scanner
 * every time we want to scan a double.
 * Since this is a private method, we trust that the caller supplies a
 * valid string argument.
 */
+ (BOOL) _scanDouble: (double*)value from: (NSString*)str
{
  static pthread_mutex_t myLock = PTHREAD_MUTEX_INITIALIZER;
  static NSScanner	*doubleScanner = nil;
  BOOL	ok = NO;

  pthread_mutex_lock(&myLock);
  if (nil == doubleScanner)
    {
      doubleScanner = [[self alloc] initWithString: _empty];
    }
  [doubleScanner _setString: str];
  ok = [doubleScanner scanDouble: value];
  [doubleScanner _setString: _empty];		// Release scanned string
  pthread_mutex_unlock(&myLock);
  return ok;
}

/**
 * Initialises the scanner to scan aString.  The GNUstep
 * implementation may make an internal copy of the original
 * string - so it is not safe to assume that if you modify a
 * mutable string that you initialised a scanner with, the changes
 * will be visible to the scanner.
 * <br/>
 * Returns the scanner object.
 */
- (id) initWithString: (NSString *)aString
{
  if ((self = [super init]) != nil)
    {
      /* Ensure that we have a known string so we can access
       * its internals directly.
       */
      if (aString == nil)
	{
	  NSLog(@"Scanner initialised with nil string");
	  aString = _empty;
	}
      if ([aString isKindOfClass: NSStringClass] == NO)
	{
	  NSLog(@"Scanner initialised with something not a string");
	  DESTROY(self);
	}
      else
	{
	  [self _setString: aString];
	  [self setCharactersToBeSkipped: defaultSkipSet];
	  _decimal = '.';
	}
    }
  return self;
}

/*
 * Deallocate a scanner and all its associated storage.
 */
- (void) dealloc
{
  RELEASE(_string);
  TEST_RELEASE(_locale);
  RELEASE(_charactersToBeSkipped);
  [super dealloc];
}

/**
 * Returns YES if no more characters remain to be scanned.<br />
 * Returns YES if all characters remaining to be scanned
 * are to be skipped.<br />
 * Returns NO if there are characters left to scan.
 */
- (BOOL) isAtEnd
{
  unsigned int	save__scanLocation;
  BOOL		ret;

  if (_scanLocation >= myLength())
    return YES;
  save__scanLocation = _scanLocation;
  ret = !skipToNextField();
  _scanLocation = save__scanLocation;
  return ret;
}

/*
 * Internal version of scanInt: method.
 * Does the actual work for scanInt: except for the initial skip.
 * For internal use only.  This method may move the scan location
 * even if a valid integer is not scanned.
 * Based on the strtol code from the GNU C library.  A little simpler since
 * we deal only with base 10.
 * FIXME: I don't use the decimalDigitCharacterSet here since it
 * includes many more characters than the ASCII digits.  I don't
 * know how to convert those other characters, so I ignore them
 * for now.  For the same reason, I don't try to support all the
 * possible Unicode plus and minus characters.
 */
- (BOOL) _scanInt: (int*)value
{
  unsigned int num = 0;
  const unsigned int limit = UINT_MAX / 10;
  BOOL negative = NO;
  BOOL overflow = NO;
  BOOL got_digits = NO;

  /* Check for sign */
  if (_scanLocation < myLength())
    {
      switch (myCharacter(_scanLocation))
	{
	  case '+':
	    _scanLocation++;
	    break;
	  case '-':
	    negative = YES;
	    _scanLocation++;
	    break;
	}
    }

  /* Process digits */
  while (_scanLocation < myLength())
    {
      unichar digit = myCharacter(_scanLocation);

      if ((digit < '0') || (digit > '9'))
	break;
      if (!overflow)
	{
	  if (num >= limit)
	    overflow = YES;
	  else
	    num = num * 10 + (digit - '0');
	}
      _scanLocation++;
      got_digits = YES;
    }

  /* Save result */
  if (!got_digits)
    return NO;
  if (value)
    {
      if (overflow
	|| (num > (negative ? (NSUInteger)INT_MIN : (NSUInteger)INT_MAX)))
	*value = negative ? INT_MIN: INT_MAX;
      else if (negative)
	*value = -num;
      else
	*value = num;
    }
  return YES;
}

/**
 * After initial skipping (if any), this method scans a integer value,
 * placing it in <em>intValue</em> if that is not null.
 * <br/>
 * Returns YES if anything is scanned, NO otherwise.
 * <br/>
 * On overflow, INT_MAX or INT_MIN is put into <em>intValue</em>
 * <br/>
 * Scans past any excess digits
 */
- (BOOL) scanInt: (int*)value
{
  unsigned int saveScanLocation = _scanLocation;

  if (skipToNextField() && [self _scanInt: value])
    return YES;
  _scanLocation = saveScanLocation;
  return NO;
}

/* Scan an unsigned long long of the given radix into value.
 * Internal version used by scanHexInt:, scanHexLongLong: etc.
 */
- (BOOL) scanUnsignedLongLong_: (unsigned long long int*)value
                         radix: (NSUInteger)radix
                       maximum: (unsigned long long)max
                     gotDigits: (BOOL)gotDigits
{
  unsigned long long int        num = 0;
  unsigned long long int        numLimit = max / radix;
  unsigned long long int        digitLimit = max % radix;
  unsigned long long int        digitValue = 0;
  BOOL                          overflow = NO;
  unsigned int                  saveScanLocation = _scanLocation;

  /* Process digits */
  while (_scanLocation < myLength())
    {
      unichar digit = myCharacter(_scanLocation);

      switch (digit)
        {
          case '0': digitValue = 0; break;
          case '1': digitValue = 1; break;
          case '2': digitValue = 2; break;
          case '3': digitValue = 3; break;
          case '4': digitValue = 4; break;
          case '5': digitValue = 5; break;
          case '6': digitValue = 6; break;
          case '7': digitValue = 7; break;
          case '8': digitValue = 8; break;
          case '9': digitValue = 9; break;
          case 'a': digitValue = 0xA; break;
          case 'b': digitValue = 0xB; break;
          case 'c': digitValue = 0xC; break;
          case 'd': digitValue = 0xD; break;
          case 'e': digitValue = 0xE; break;
          case 'f': digitValue = 0xF; break;
          case 'A': digitValue = 0xA; break;
          case 'B': digitValue = 0xB; break;
          case 'C': digitValue = 0xC; break;
          case 'D': digitValue = 0xD; break;
          case 'E': digitValue = 0xE; break;
          case 'F': digitValue = 0xF; break;
          default:
            digitValue = radix;
            break;
        }
      if (digitValue >= radix)
        {
          break;
        }
      if (!overflow)
        {
          if ((num > numLimit)
            || ((num == numLimit) && (digitValue > digitLimit)))
            {
              overflow = YES;
            }
           else
            {
              num = num * radix + digitValue;
            }
        }
      _scanLocation++;
      gotDigits = YES;
    }

  /* Save result */
  if (!gotDigits)
    {
      _scanLocation = saveScanLocation;
      return NO;
    }
  if (value)
    {
      if (overflow)
        {
          *value = ULLONG_MAX;
        }
      else
        {
          *value = num;
        }
    }
  return YES;
}

/**
 * After initial skipping (if any), this method scans an unsigned
 * integer placing it in <em>value</em> if that is not null.
 * If the number begins with "0x" or "0X" it is treated as hexadecimal,
 * otherwise if the number begins with "0" it is treated as octal,
 * otherwise the number is treated as decimal.
 * <br/>
 * Returns YES if anything is scanned, NO otherwise.
 * <br/>
 * On overflow, UINT_MAX is put into <em>value</em>
 * <br/>
 * Scans past any excess digits
 */
- (BOOL) scanRadixUnsignedInt: (unsigned int*)value
{
  unsigned int	        radix;
  unsigned long long    tmp;
  BOOL		        gotDigits = NO;
  unsigned int	        saveScanLocation = _scanLocation;

  /* Skip whitespace */
  if (!skipToNextField())
    {
      _scanLocation = saveScanLocation;
      return NO;
    }

  /* Check radix */
  radix = 10;
  if ((_scanLocation < myLength()) && (myCharacter(_scanLocation) == '0'))
    {
      radix = 8;
      _scanLocation++;
      gotDigits = YES;
      if (_scanLocation < myLength())
	{
	  switch (myCharacter(_scanLocation))
	    {
	      case 'x':
	      case 'X':
		_scanLocation++;
		radix = 16;
		gotDigits = NO;
		break;
	    }
	}
    }
  if ([self scanUnsignedLongLong_: &tmp
                            radix: radix
                          maximum: UINT_MAX
                        gotDigits: gotDigits])
    {
      if (tmp > UINT_MAX)
        {
          *value = UINT_MAX;
        }
      else
        {
          *value = (unsigned int)tmp;
        }
      return YES;
    }
  _scanLocation = saveScanLocation;
  return NO;
}

/**
 * After initial skipping (if any), this method scans an unsigned
 * long long integer placing it in <em>value</em> if that is not null.
 * If the number begins with "0x" or "0X" it is treated as hexadecimal,
 * otherwise if the number begins with "0" it is treated as octal,
 * otherwise the number is treated as decimal.
 * <br/>
 * Returns YES if anything is scanned, NO otherwise.
 * <br/>
 * On overflow, ULLONG_MAX is put into <em>value</em>
 * <br/>
 * Scans past any excess digits
 */
- (BOOL) scanRadixUnsignedLongLong: (unsigned long long*)value
{
  unsigned int	        radix;
  BOOL		        gotDigits = NO;
  unsigned int	        saveScanLocation = _scanLocation;

  /* Skip whitespace */
  if (!skipToNextField())
    {
      _scanLocation = saveScanLocation;
      return NO;
    }

  /* Check radix */
  radix = 10;
  if ((_scanLocation < myLength()) && (myCharacter(_scanLocation) == '0'))
    {
      radix = 8;
      _scanLocation++;
      gotDigits = YES;
      if (_scanLocation < myLength())
	{
	  switch (myCharacter(_scanLocation))
	    {
	      case 'x':
	      case 'X':
		_scanLocation++;
		radix = 16;
		gotDigits = NO;
		break;
	    }
	}
    }
  if ([self scanUnsignedLongLong_: value
                            radix: radix
                          maximum: ULLONG_MAX
                        gotDigits: gotDigits])
    {
      return YES;
    }
  _scanLocation = saveScanLocation;
  return NO;
}

/**
 * After initial skipping (if any), this method scans a hexadecimal
 * integer value (optionally prefixed by "0x" or "0X"),
 * placing it in <em>intValue</em> if that is not null.
 * <br/>
 * Returns YES if anything is scanned, NO otherwise.
 * <br/>
 * On overflow, INT_MAX or INT_MIN is put into <em>intValue</em>
 * <br/>
 * Scans past any excess digits
 */
- (BOOL) scanHexInt: (unsigned int*)value
{
  unsigned int          saveScanLocation = _scanLocation;
  unsigned long long    tmp;

  /* Skip whitespace */
  if (!skipToNextField())
    {
      _scanLocation = saveScanLocation;
      return NO;
    }

  if ((_scanLocation < myLength()) && (myCharacter(_scanLocation) == '0'))
    {
      _scanLocation++;
      if (_scanLocation < myLength())
	{
	  switch (myCharacter(_scanLocation))
	    {
	      case 'x':
	      case 'X':
		_scanLocation++;	// Scan beyond the 0x prefix
		break;
	      default:
		_scanLocation--;	// Scan from the initial digit
	        break;
	    }
	}
      else
	{
	  _scanLocation--;	// Just scan the zero.
	}
    }
  if ([self scanUnsignedLongLong_: &tmp
                            radix: 16
                          maximum: UINT_MAX
                        gotDigits: NO])
    {
      *value = (unsigned int)tmp;
      return YES;
    }
  _scanLocation = saveScanLocation;
  return NO;
}

/**
 * After initial skipping (if any), this method scans a long
 * decimal integer value placing it in <em>longLongValue</em> if that
 * is not null.
 * <br/>
 * Returns YES if anything is scanned, NO otherwise.
 * <br/>
 * On overflow, LLONG_MAX or LLONG_MIN is put into
 * <em>longLongValue</em>
 * <br/>
 * Scans past any excess digits
 */
- (BOOL) scanLongLong: (long long *)value
{
  unsigned long long		num = 0;
  const unsigned long long	limit = ULLONG_MAX / 10;
  BOOL				negative = NO;
  BOOL				overflow = NO;
  BOOL				got_digits = NO;
  unsigned int			saveScanLocation = _scanLocation;

  /* Skip whitespace */
  if (!skipToNextField())
    {
      _scanLocation = saveScanLocation;
      return NO;
    }

  /* Check for sign */
  if (_scanLocation < myLength())
    {
      switch (myCharacter(_scanLocation))
	{
	  case '+':
	    _scanLocation++;
	    break;
	  case '-':
	    negative = YES;
	    _scanLocation++;
	    break;
	}
    }

    /* Process digits */
  while (_scanLocation < myLength())
    {
      unichar digit = myCharacter(_scanLocation);

      if ((digit < '0') || (digit > '9'))
	break;
      if (!overflow) {
	if (num >= limit)
	  overflow = YES;
	else
	  num = num * 10 + (digit - '0');
      }
      _scanLocation++;
      got_digits = YES;
    }

    /* Save result */
  if (!got_digits)
    {
      _scanLocation = saveScanLocation;
      return NO;
    }
  if (value)
    {
      if (negative)
	{
	  if (overflow || (num > (unsigned long long)LLONG_MIN))
	    *value = LLONG_MIN;
	  else
	    *value = -num;
	}
      else
	{
	  if (overflow || (num > (unsigned long long)LLONG_MAX))
	    *value = LLONG_MAX;
	  else
	    *value = num;
	}
    }
  return YES;
}

/**
 * After initial skipping (if any), this method scans a hexadecimal
 * long long value (optionally prefixed by "0x" or "0X"),
 * placing it in <em>longLongValue</em> if that is not null.
 * <br/>
 * Returns YES if anything is scanned, NO otherwise.
 * <br/>
 * On overflow, ULLONG_MAX or ULLONG_MAX is put into <em>longLongValue</em>
 * <br/>
 * Scans past any excess digits
 */
- (BOOL) scanHexLongLong: (unsigned long long*)value
{
  unsigned int saveScanLocation = _scanLocation;

  /* Skip whitespace */
  if (!skipToNextField())
    {
      _scanLocation = saveScanLocation;
      return NO;
    }

  if ((_scanLocation < myLength()) && (myCharacter(_scanLocation) == '0'))
    {
      _scanLocation++;
      if (_scanLocation < myLength())
       {
         switch (myCharacter(_scanLocation))
           {
             case 'x':
             case 'X':
               _scanLocation++;        // Scan beyond the 0x prefix
               break;
             default:
               _scanLocation--;        // Scan from the initial digit
               break;
           }
       }
      else
       {
         _scanLocation--;      // Just scan the zero.
       }
    }
  if ([self scanUnsignedLongLong_: value
                            radix: 16
                          maximum: ULLONG_MAX
                        gotDigits: NO])
    {
      return YES;
    }
  _scanLocation = saveScanLocation;
  return NO;
}

/**
 * Not implemented.
 */
- (BOOL) scanDecimal: (NSDecimal*)value
{
  [self notImplemented:_cmd];			/* FIXME */
  return NO;
}

/**
 * After initial skipping (if any), this method scans a double value,
 * placing it in <em>doubleValue</em> if that is not null.
 * Returns YES if anything is scanned, NO otherwise.
 * <br/>
 * On overflow, HUGE_VAL or - HUGE_VAL is put into <em>doubleValue</em>
 * <br/>
 * On underflow, 0.0 is put into <em>doubleValue</em>
 * <br/>
 * Scans past any excess digits
 */
- (BOOL) scanDouble: (double *)value
{
  unichar	c = 0;
  char          mantissa[20];
  char		*ptr;
  double        *d;
  double        result;
  double        e;
  int		exponent = 0;
  BOOL          negativeMantissa = NO;
  BOOL          negativeExponent = NO;
  unsigned      shift = 0;
  int           mantissaLength;
  int           dotPos = -1;
  int           hi = 0;
  int           lo = 0;
  BOOL		mantissaDigit = NO;
  unsigned int	saveScanLocation = _scanLocation;

  /* Skip whitespace */
  if (!skipToNextField())
    {
      _scanLocation = saveScanLocation;
      return NO;
    }

  /* Check for sign */
  if (_scanLocation < myLength())
    {
      switch (mySevenBit(_scanLocation))
	{
	  case '+':
	    _scanLocation++;
	    break;
	  case '-':
	    _scanLocation++;
	    negativeMantissa = YES;
	    break;
	}
    }
  if (_scanLocation >= myLength())
    {
      _scanLocation = saveScanLocation;
      return NO;
    }

  /* Now we build up the mantissa digits.  Leading zeros are ignored, but
   * those after the decimal point are counted in order to adjust the
   * exponent later.
   * Excess digits are also ignored ... a double can only handle up to 18
   * digits of precision.
   */
  for (mantissaLength = 0; _scanLocation < myLength(); _scanLocation++)
    {
      c = mySevenBit(_scanLocation);
      if (c < '0' || c > '9')
        {
          if (dotPos >= 0)
            {
              break;    // Already found dot; must be end of mantissa
            }
	  /* The decimal separator can in theory be outside the ascii range,
	   * and if it is we must fetch the current unicode character in order
	   * to perform the comparison.
	   */
	  if (_decimal == c
	    || (_decimal > 127 && _decimal == myCharacter(_scanLocation)))
	    {
	      dotPos = mantissaLength;
	    }
	  else
	    {
              break;    // Not a dot; must be end of mantissa
	    }
        }
      else
        {
	  mantissaDigit = YES;
	  if (0 == mantissaLength && '0' == c)
	    {
	      if (dotPos >= 0)
		{
		  shift++;	// Leading zero after decimal place
		}
	    }
	  else if (mantissaLength < 19)
	    {
	      mantissa[mantissaLength++] = c;
	    }
        }
    }
  if (NO == mantissaDigit)
    {
      _scanLocation = saveScanLocation;
      return NO;
    }
  if (mantissaLength > 18)
    {
      /* Mantissa too long ... ignore excess.
       */
      mantissaLength = 18;
    }
  if (dotPos < 0)
    {
      dotPos = mantissaLength;
    }
  dotPos -= mantissaLength;
 
  /* Convert mantissa characters to a double value
   */
  for (ptr = mantissa; mantissaLength > 9; mantissaLength -= 1)
    {
      c = *ptr;
      ptr += 1;
      hi = hi * 10 + (c - '0');
    }
  for (; mantissaLength > 0; mantissaLength -= 1)
    {
      c = *ptr;
      ptr += 1;
      lo = lo * 10 + (c - '0');
    }
  result = (1.0e9 * hi) + lo;

  /* Scan the exponent (if any)
   */
  if (_scanLocation < myLength()
    && ((c = mySevenBit(_scanLocation)) == 'e' || c == 'E'))
    {
      unsigned	saveExpLoc = _scanLocation;

      _scanLocation++;			// Step past E/e
      if (_scanLocation >= myLength())
	{
	  _scanLocation = saveExpLoc;	// No exponent
	}
      else
	{
	  switch (mySevenBit(_scanLocation))
	    {
	      case '+':
		_scanLocation++;
		break;
	      case '-':
		_scanLocation++;
		negativeExponent = YES;
		break;
	    }
	  if (_scanLocation >= myLength()
	    || (c = mySevenBit(_scanLocation)) < '0' || c > '9')
	    {
	      _scanLocation = saveExpLoc;	// No exponent
	    }
	  else
	    {
	      exponent = c - '0';
	      _scanLocation++;
	      while (_scanLocation < myLength()
		&& (c = mySevenBit(_scanLocation)) >= '0' && c <= '9')
		{
		  exponent = exponent * 10 + (c - '0');
		  _scanLocation++;
		}
	    }
	}
    }
 
  /* Add in the amount to shift the exponent depending on the position
   * of the decimal point in the mantissa and check the adjusted sign
   * of the exponent.
   */
  if (YES == negativeExponent)
    {
      exponent = dotPos - exponent;
    }
  else
    {
      exponent = dotPos + exponent;
    }
  exponent -= shift;
  if (exponent < 0)
    {
      negativeExponent = YES;
      exponent = -exponent;
    }
  else
    {
      negativeExponent = NO;
    }
  if (exponent > 511)
    {
      _scanLocation = saveScanLocation;
      return NO;        // Maximum exponent exceeded
    }

  /* Convert the exponent to a double then apply it to the value from
   * the mantissa.
   */
  e = 1.0;
  for (d = powersOf10; exponent != 0; exponent >>= 1, d += 1)
    {
      if (exponent & 1)
        {
          e *= *d;
        }
    }
  if (YES == negativeExponent)
    {
      result /= e;
    }
  else
    {
      result *= e;
    }

  if (0 != value)
    {
      if (YES == negativeMantissa)
        {
          *value = -result;
        }
      else
        {
          *value = result;
        }
    }
  return YES;
}

/**
 * After initial skipping (if any), this method scans a float value,
 * placing it in <em>floatValue</em> if that is not null.
 * Returns YES if anything is scanned, NO otherwise.
 * <br/>
 * On overflow, HUGE_VAL or - HUGE_VAL is put into <em>floatValue</em>
 * <br/>
 * On underflow, 0.0 is put into <em>floatValue</em>
 * <br/>
 * Scans past any excess digits
 */
- (BOOL) scanFloat: (float*)value
{
  double num;

  if (value == NULL)
    return [self scanDouble: NULL];
  if ([self scanDouble: &num])
    {
      *value = num;
      return YES;
    }
  return NO;
}

/**
 * After initial skipping (if any), this method scans any characters
 * from aSet, terminating when a character not in the set
 * is found.<br />
 * Returns YES if any character is scanned, NO otherwise.<br />
 * If value is not null, any character scanned are
 * stored in a string returned in this location.
 */
- (BOOL) scanCharactersFromSet: (NSCharacterSet *)aSet
		    intoString: (NSString **)value
{
  unsigned int	saveScanLocation = _scanLocation;

  if (skipToNextField())
    {
      unsigned int	start;
      BOOL		(*memImp)(NSCharacterSet*, SEL, unichar);

      if (aSet == _charactersToBeSkipped)
	memImp = _skipImp;
      else
	memImp = (BOOL (*)(NSCharacterSet*, SEL, unichar))
	  [aSet methodForSelector: memSel];

      start = _scanLocation;
      if (_isUnicode)
	{
	  while (_scanLocation < myLength())
	    {
	      if ((*memImp)(aSet, memSel, myUnichar(_scanLocation)) == NO)
		break;
	      _scanLocation++;
	    }
	}
      else
	{
	  while (_scanLocation < myLength())
	    {
	      if ((*memImp)(aSet, memSel, myChar(_scanLocation)) == NO)
		break;
	      _scanLocation++;
	    }
	}
      if (_scanLocation != start)
	{
	  if (value != 0)
	    {
	      NSRange	range;

	      range.location = start;
	      range.length = _scanLocation - start;
	      *value = [_string substringWithRange: range];
	    }
	  return YES;
	}
    }
  _scanLocation = saveScanLocation;
  return NO;
}

/**
 * After initial skipping (if any), this method scans characters until
 * it finds one in <em>set</em>.  The scanned characters are placed in
 * <em>stringValue</em> if that is not null.
 * <br/>
 * Returns YES if anything is scanned, NO otherwise.
 */
- (BOOL) scanUpToCharactersFromSet: (NSCharacterSet *)aSet
		        intoString: (NSString **)value
{
  unsigned int	saveScanLocation = _scanLocation;
  unsigned int	start;
  BOOL		(*memImp)(NSCharacterSet*, SEL, unichar);

  if (!skipToNextField())
    return NO;

  if (aSet == _charactersToBeSkipped)
    memImp = _skipImp;
  else
    memImp = (BOOL (*)(NSCharacterSet*, SEL, unichar))
      [aSet methodForSelector: memSel];

  start = _scanLocation;
  if (_isUnicode)
    {
      while (_scanLocation < myLength())
	{
	  if ((*memImp)(aSet, memSel, myUnichar(_scanLocation)) == YES)
	    break;
	  _scanLocation++;
	}
    }
  else
    {
      while (_scanLocation < myLength())
	{
	  if ((*memImp)(aSet, memSel, myChar(_scanLocation)) == YES)
	    break;
	  _scanLocation++;
	}
    }

  if (_scanLocation == start)
    {
      _scanLocation = saveScanLocation;
      return NO;
    }
  if (value)
    {
      NSRange	range;

      range.location = start;
      range.length = _scanLocation - start;
      *value = [_string substringWithRange: range];
    }
  return YES;
}

/**
 * After initial skipping (if any), this method scans for string
 * and places the characters found in value if that is not null.<br/>
 * Returns YES if anything is scanned, NO otherwise.
 */
- (BOOL) scanString: (NSString *)string intoString: (NSString **)value
{
  NSRange	range;
  unsigned int	saveScanLocation = _scanLocation;

  if (skipToNextField() == NO)
    {
      return NO;
    }
  range.location = _scanLocation;
  range.length = [string length];
  if (range.location + range.length > myLength())
    return NO;
  range = [_string rangeOfString: string
			options: _caseSensitive ? 0 : NSCaseInsensitiveSearch
			  range: range];
  if (range.length == 0)
    {
      _scanLocation = saveScanLocation;
      return NO;
    }
  if (value)
    *value = [_string substringWithRange: range];
  _scanLocation += range.length;
  return YES;
}

/**
 * <p>After initial skipping (if any), this method scans characters until
 * it finds string.  The scanned characters are placed in
 * value if that is not null.  If string is not found, all the characters
 * up to the end of the scanned string will be returned.
 * </p>
 * Returns YES if anything is scanned, NO otherwise.<br />
 * <p>NB. If the current scanner location points to a copy of string, or
 * points to skippable characters immediately before a copy of string
 * then this method returns NO since it finds no characters to store
 * in value before it finds string.
 * </p>
 * <p>To count the occurrences of string, this should be used in
 * conjunction with the -scanString:intoString: method.
 * </p>
 * <example>
 * NSString *ch = @"[";
 * unsigned total = 0;
 *
 * [scanner scanUpToString: ch intoString: NULL];
 * while ([scanner scanString: ch intoString: NULL] == YES)
 *  {
 *    total++;
 *    [scanner scanUpToString: ch intoString: NULL];
 *  }
 * NSLog(@"total %d", total);
 * </example>
 */
- (BOOL) scanUpToString: (NSString *)string
	     intoString: (NSString **)value
{
  NSRange	range;
  NSRange	found;
  unsigned int	saveScanLocation = _scanLocation;

  if (skipToNextField() == NO)
    {
      return NO;
    }
  range.location = _scanLocation;
  range.length = myLength() - _scanLocation;
  found = [_string rangeOfString: string
			 options: _caseSensitive ? 0 : NSCaseInsensitiveSearch
			   range: range];
  if (found.length)
    range.length = found.location - _scanLocation;
  if (range.length == 0)
    {
      _scanLocation = saveScanLocation;
      return NO;
    }
  if (value)
    *value = [_string substringWithRange: range];
  _scanLocation += range.length;
  return YES;
}

/**
 * Returns the string being scanned.
 */
- (NSString *) string
{
  return _string;
}

/**
 * Returns the current position that the scanner has reached in
 * scanning the string.  This is the position at which the next scan
 * operation will begin.
 */
- (NSUInteger) scanLocation
{
  return _scanLocation;
}

/**
 * This method sets the location in the scanned string at which the
 * next scan operation begins.
 * Raises an NSRangeException if index is beyond the end of the
 * scanned string.
 */
- (void) setScanLocation: (NSUInteger)anIndex
{
  if (_scanLocation <= myLength())
    _scanLocation = anIndex;
  else
    [NSException raise: NSRangeException
		format: @"Attempt to set scan location beyond end of string"];
}

/**
 * If the scanner is set to be case-sensitive in its scanning of
 * the string (other than characters to be skipped), this method
 * returns YES, otherwise it returns NO.
 * <br/>
 * The default is for a scanner to <em>not</em> be case sensitive.
 */
- (BOOL) caseSensitive
{
  return _caseSensitive;
}

/**
 * Sets the case sensitivity of the scanner.
 * <br/>
 * Case sensitivity governs matching of characters being scanned,
 * but does not effect the characters in the set to be skipped.
 * <br/>
 * The default is for a scanner to <em>not</em> be case sensitive.
 */
- (void) setCaseSensitive: (BOOL)flag
{
  _caseSensitive = flag;
}

/**
 * Returns a set of characters containing those characters that the
 * scanner ignores when starting any scan operation.  Once a character
 * not in this set has been encountered during an operation, skipping
 * is finished, and any further characters from this set that are
 * found are scanned normally.
 * <br/>
 * The default for this is the whitespaceAndNewlineCharacterSet.
 */
- (NSCharacterSet *) charactersToBeSkipped
{
  return _charactersToBeSkipped;
}

/**
 * Sets the set of characters that the scanner will skip over at the
 * start of each scanning operation to be <em>skipSet</em>.
 * Skipping is performed by literal character matching - the case
 * sensitivity of the scanner does not effect it.
 * If this is set to nil, no skipping is done.
 * <br/>
 * The default for this is the whitespaceAndNewlineCharacterSet.
 */
- (void) setCharactersToBeSkipped: (NSCharacterSet *)aSet
{
  ASSIGNCOPY(_charactersToBeSkipped, aSet);
  _skipImp = (BOOL (*)(NSCharacterSet*, SEL, unichar))
    [_charactersToBeSkipped methodForSelector: memSel];
}

/**
 * Returns the locale set for the scanner, or nil if no locale has
 * been set.  A scanner uses it's locale to alter the way it handles
 * scanning - it uses the NSDecimalSeparator value for scanning
 * numbers.
 */
- (NSDictionary *) locale
{
  return _locale;
}

/**
 * This method sets the locale used by the scanner to <em>aLocale</em>.
 * The locale may be set to nil.
 */
- (void) setLocale: (NSDictionary *)localeDictionary
{
  ASSIGN(_locale, localeDictionary);
  /*
   * Get decimal point character from locale if necessary.
   */
  if (_locale == nil)
    {
      _decimal = '.';
    }
  else
    {
      NSString	*pointString;

      pointString = [_locale objectForKey: NSDecimalSeparator];
      if ([pointString length] > 0)
	_decimal = [pointString characterAtIndex: 0];
      else
	_decimal = '.';
    }
}

/*
 * NSCopying protocol
 */
- (id) copyWithZone: (NSZone *)zone
{
  NSScanner	*n = [[self class] allocWithZone: zone];

  n = [n initWithString: _string];
  [n setCharactersToBeSkipped: _charactersToBeSkipped];
  [n setLocale: _locale];
  [n setScanLocation: _scanLocation];
  [n setCaseSensitive: _caseSensitive];
  return n;
}

- (BOOL) scanHexDouble: (double *)result
{
  return NO;    // FIXME
}
- (BOOL) scanHexFloat: (float *)result
{
  return NO;    // FIXME
}
- (BOOL) scanInteger: (NSInteger *)value
{
#if GS_SIZEOF_VOIDP == GS_SIZEOF_INT
  return [self scanInt: (int *)value];
#else
  return [self scanLongLong: (long long *)value];
#endif
}
@end

