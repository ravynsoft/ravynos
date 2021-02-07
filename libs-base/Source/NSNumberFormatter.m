/**
   NSNumberFormatter class
   Copyright (C) 2000 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Created: July 2000
   Updated by: Richard Frith-Macdonald <rfm@gnu.org> Sept 2001

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

   <title>NSNumberFormatter class reference</title>
   $Date$ $Revision$
   */

/* Unfortunately, libicu does not define the maximum values allowed for all
   of these attributes.  We define them here to, though.
   These are based off libicu version 4.6.
 */
#define MAX_SYMBOLS 27
#define MAX_TEXTATTRIBUTES 8
#define MAX_ATTRIBUTES 20

#define GS_NSNumberFormatter_IVARS \
  NSUInteger	_behavior; \
  BOOL		_genDecimal; \
  NSUInteger	_style; \
  NSLocale	*_locale; \
  void		*_formatter; \
  id		_symbols[MAX_SYMBOLS]; \
  id		_textAttributes[MAX_TEXTATTRIBUTES]; \
  int		_attributes[MAX_ATTRIBUTES]

#import "common.h"
#define	EXPOSE_NSNumberFormatter_IVARS	1
#import "Foundation/NSAttributedString.h"
#import "Foundation/NSDecimalNumber.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSError.h"
#import "Foundation/NSException.h"
#import "Foundation/NSLocale.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSNumberFormatter.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSCharacterSet.h"

#import "GNUstepBase/GSLocale.h"

@class NSDoubleNumber;

#if	defined(HAVE_UNICODE_UNUM_H)
# include <unicode/unum.h>
#endif

#define BUFFER_SIZE 1024

#if GS_USE_ICU == 1
static inline UNumberFormatStyle
NSToICUFormatStyle (NSNumberFormatterStyle style)
{
  UNumberFormatStyle result;
  
  switch (style)
    {
      case NSNumberFormatterDecimalStyle:
        result = UNUM_DECIMAL;
        break;
      case NSNumberFormatterCurrencyStyle:
        result = UNUM_CURRENCY;
        break;
      case NSNumberFormatterPercentStyle:
        result = UNUM_PERCENT;
        break;
      case NSNumberFormatterScientificStyle:
        result = UNUM_SCIENTIFIC;
        break;
      case NSNumberFormatterSpellOutStyle:
        result = UNUM_SPELLOUT;
        break;
      case NSNumberFormatterNoStyle:
      default:
        result = UNUM_IGNORE;
    }
  
  return result;
}

static inline UNumberFormatPadPosition
NSToICUPadPosition (NSNumberFormatterPadPosition position)
{
  UNumberFormatPadPosition result = 0;
  
  switch (position)
    {
      case NSNumberFormatterPadBeforePrefix:
        result = UNUM_PAD_BEFORE_PREFIX;
        break;
      case NSNumberFormatterPadAfterPrefix:
        result = UNUM_PAD_AFTER_PREFIX;
        break;
      case NSNumberFormatterPadBeforeSuffix:
        result = UNUM_PAD_BEFORE_SUFFIX;
        break;
      case NSNumberFormatterPadAfterSuffix:
        result = UNUM_PAD_AFTER_SUFFIX;
        break;
    }
  
  return result;
}

static inline NSNumberFormatterPadPosition
ICUToNSPadPosition (UNumberFormatPadPosition position)
{
  NSNumberFormatterPadPosition result = 0;
  
  switch (position)
    {
      case UNUM_PAD_BEFORE_PREFIX:
        result = NSNumberFormatterPadBeforePrefix;
        break;
      case UNUM_PAD_AFTER_PREFIX:
        result = NSNumberFormatterPadAfterPrefix;
        break;
      case UNUM_PAD_BEFORE_SUFFIX:
        result = NSNumberFormatterPadBeforeSuffix;
        break;
      case UNUM_PAD_AFTER_SUFFIX:
        result = NSNumberFormatterPadAfterSuffix;
        break;
    }
  
  return result;
}

static inline UNumberFormatRoundingMode
NSToICURoundingMode (NSNumberFormatterRoundingMode mode)
{
  UNumberFormatRoundingMode result = 0;
  
  switch (mode)
    {
      case NSNumberFormatterRoundCeiling:
        result = UNUM_ROUND_CEILING;
        break;
      case NSNumberFormatterRoundFloor:
        result = UNUM_ROUND_FLOOR;
        break;
      case NSNumberFormatterRoundDown:
        result = UNUM_ROUND_DOWN;
        break;
      case NSNumberFormatterRoundUp:
        result = UNUM_ROUND_UP;
        break;
      case NSNumberFormatterRoundHalfEven:
        result = UNUM_ROUND_HALFEVEN;
        break;
      case NSNumberFormatterRoundHalfDown:
        result = UNUM_ROUND_HALFDOWN;
        break;
      case NSNumberFormatterRoundHalfUp:
        result = UNUM_ROUND_HALFUP;
        break;
    }
  
  return result;
}

static inline NSNumberFormatterRoundingMode
ICUToNSRoundingMode (UNumberFormatRoundingMode mode)
{
  NSNumberFormatterRoundingMode result = 0;
  
  switch (mode)
    {
      case UNUM_ROUND_CEILING:
        result = NSNumberFormatterRoundCeiling;
        break;
      case UNUM_ROUND_FLOOR:
        result = NSNumberFormatterRoundFloor;
        break;
      case UNUM_ROUND_DOWN:
        result = NSNumberFormatterRoundDown;
        break;
      case UNUM_ROUND_UP:
        result = NSNumberFormatterRoundUp;
        break;
      case UNUM_ROUND_HALFEVEN:
        result = NSNumberFormatterRoundHalfEven;
        break;
      case UNUM_ROUND_HALFDOWN:
        result = NSNumberFormatterRoundHalfDown;
        break;
      case UNUM_ROUND_HALFUP:
        result = NSNumberFormatterRoundHalfUp;
        break;
      default:
        result = NSNumberFormatterRoundHalfUp;
    }
  
  return result;
}

#else

/* Define fake ICU constants so we can use them when ICU is not available.
 */
#define NSToICUFormatStyle(X) X
#define	NSToICUPadPosition(X) X
#define	ICUToNSPadPosition(X) X
#define	NSToICURoundingMode(X) X
#define	ICUToNSRoundingMode(X) X

#define	UNUM_CURRENCY_CODE	0
#define	UNUM_CURRENCY_SYMBOL	0
#define	UNUM_DECIMAL_ALWAYS_SHOWN	0
#define	UNUM_DECIMAL_SEPARATOR_SYMBOL	0
#define	UNUM_EXPONENTIAL_SYMBOL	0
#define	UNUM_FORMAT_WIDTH	0
#define	UNUM_GROUPING_SEPARATOR_SYMBOL	0
#define	UNUM_GROUPING_SIZE	0
#define	UNUM_GROUPING_USED	0
#define	UNUM_INFINITY_SYMBOL	0
#define	UNUM_INTL_CURRENCY_SYMBOL	0
#define	UNUM_LENIENT_PARSE	0
#define	UNUM_MAX_FRACTION_DIGITS	0
#define	UNUM_MAX_INTEGER_DIGITS	0
#define	UNUM_MAX_SIGNIFICANT_DIGITS	0
#define	UNUM_MIN_FRACTION_DIGITS	0
#define	UNUM_MIN_INTEGER_DIGITS	0
#define	UNUM_MIN_SIGNIFICANT_DIGITS	0
#define	UNUM_MINUS_SIGN_SYMBOL	0
#define	UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL	0
#define	UNUM_MONETARY_SEPARATOR_SYMBOL	0
#define	UNUM_MULTIPLIER	0
#define	UNUM_NAN_SYMBOL	0
#define	UNUM_NEGATIVE_PREFIX	0
#define	UNUM_NEGATIVE_SUFFIX	0
#define	UNUM_PADDING_CHARACTER	0
#define	UNUM_PADDING_POSITION	0
#define	UNUM_PERCENT_SYMBOL	0
#define	UNUM_PERMILL_SYMBOL	0
#define	UNUM_PLUS_SIGN_SYMBOL	0
#define	UNUM_POSITIVE_PREFIX	0
#define	UNUM_POSITIVE_SUFFIX	0
#define	UNUM_ROUNDING_MODE	0
#define	UNUM_SECONDARY_GROUPING_SIZE	0
#define	UNUM_SIGNIFICANT_DIGITS_USED	0
#define	UNUM_ZERO_DIGIT_SYMBOL	0

#endif
 
#define GSInternal              NSNumberFormatterInternal
#include        "GSInternal.h"
GS_PRIVATE_INTERNAL(NSNumberFormatter)

#if GS_NONFRAGILE
@interface NSNumberFormatter (Internal)
#else
@interface	NSNumberFormatterInternal (Methods)
#endif
- (int32_t) attributeForKey: (int)key;
- (NSString*) symbolForKey: (int)key;
- (NSString*) textAttributeForKey: (int)key;
- (void) setAttribute: (int32_t)value forKey: (int)key;
- (void) setSymbol: (NSString*)value forKey: (int)key;
- (void) setTextAttribute: (NSString*)value forKey: (int)key;
@end

#if GS_NONFRAGILE
@implementation NSNumberFormatter (Internal)
#else
@implementation	NSNumberFormatterInternal (Methods)
#endif

- (int32_t) attributeForKey: (int)key
{
  NSAssert(key >= 0
    && key < sizeof(_attributes) / sizeof(*_attributes),
    NSInvalidArgumentException);
#if GS_USE_ICU == 1
  if (_attributes[key] <= 0)
    {
      _attributes[key] = unum_getAttribute (_formatter, key);
    }
#endif
  return _attributes[key];
}

- (BOOL) boolForKey: (int)key
{
  NSAssert(key >= 0
    && key < sizeof(_attributes) / sizeof(*_attributes),
    NSInvalidArgumentException);
#if GS_USE_ICU == 1
  if (0 == _attributes[key])
    {
      _attributes[key]
	= (1 == unum_getAttribute (_formatter, key)) ? 2 : 1;
    }
#endif
  if (2 == _attributes[key])
    {
      return YES;
    }
  return NO;
}

- (NSString*) symbolForKey: (int)key
{
#if GS_USE_ICU == 1
  UChar		buffer[BUFFER_SIZE];
  int32_t	length;
  UErrorCode	err = U_ZERO_ERROR;

  NSAssert(key >= 0
    && key < sizeof(_symbols) / sizeof(*_symbols),
    NSInvalidArgumentException);
  length = unum_getSymbol(_formatter, key, buffer, BUFFER_SIZE, &err);
  if (length > BUFFER_SIZE)
    length = BUFFER_SIZE;
  return [NSString stringWithCharacters: buffer length: length];
#else
  return nil;
#endif
}

- (NSString*) textAttributeForKey: (int)key
{
#if GS_USE_ICU == 1
  UChar		buffer[BUFFER_SIZE];
  int32_t	length;
  UErrorCode	err = U_ZERO_ERROR;

  NSAssert(key >= 0
    && key < sizeof(_textAttributes) / sizeof(*_textAttributes),
    NSInvalidArgumentException);
  length = unum_getTextAttribute(_formatter, key, buffer, BUFFER_SIZE, &err);
  if (length > BUFFER_SIZE)
    length = BUFFER_SIZE;
  return [NSString stringWithCharacters: buffer length: length];
#else
  return nil;
#endif
}

- (void) setAttribute: (int32_t)value forKey: (int)key
{
  NSAssert(key >= 0
    && key < sizeof(_attributes) / sizeof(*_attributes),
    NSInvalidArgumentException);
#if GS_USE_ICU == 1
  if (value < 0)
    value = -1;
  _attributes[key] = value;
  unum_setAttribute (_formatter, key, value);
#endif
  return;
}

- (void) setBool: (BOOL)value forKey: (int)key
{
  NSAssert(key >= 0
    && key < sizeof(_symbols) / sizeof(*_symbols),
    NSInvalidArgumentException);
#if GS_USE_ICU == 1
  _attributes[key] = (value ? 2 : 1);
  unum_setAttribute (_formatter, key, (int32_t)(value ? 1 : 0));
#endif
  return;
}

- (void) setSymbol: (NSString*)value forKey: (int)key
{
#if GS_USE_ICU == 1
  unichar	buffer[BUFFER_SIZE];
  NSUInteger	length;
  UErrorCode	err = U_ZERO_ERROR;

  NSAssert(key >= 0
    && key < sizeof(_symbols) / sizeof(*_symbols),
    NSInvalidArgumentException);
  ASSIGNCOPY(_symbols[key], value);
  length = [value length];
  if (length > BUFFER_SIZE)
    length = BUFFER_SIZE;
  [value getCharacters: buffer range: NSMakeRange (0, length)];
  unum_setSymbol (_formatter, key, buffer, length, &err);
#endif
  return;
}

- (void) setTextAttribute: (NSString*)value forKey: (int)key
{
#if GS_USE_ICU == 1
  unichar	buffer[BUFFER_SIZE];
  NSUInteger	length;
  UErrorCode	err = U_ZERO_ERROR;

  NSAssert(key >= 0
    && key < sizeof(_textAttributes) / sizeof(*_textAttributes),
    NSInvalidArgumentException);
  ASSIGNCOPY(_textAttributes[key], value);
  length = [value length];
  if (length > BUFFER_SIZE)
    length = BUFFER_SIZE;
  [value getCharacters: buffer range: NSMakeRange (0, length)];
  unum_setTextAttribute (_formatter, key, buffer, length, &err);
#endif
  return;
}
@end

@interface NSNumberFormatter (PrivateMethods)
- (void) _resetUNumberFormat;
@end

@implementation NSNumberFormatter

#if GS_USE_ICU == 1
#define	MYBEHAVIOR	internal->_behavior
#else
#define	MYBEHAVIOR	NSNumberFormatterBehavior10_0
#endif
static NSUInteger _defaultBehavior = NSNumberFormatterBehavior10_4;

- (BOOL) allowsFloats
{
  return _allowsFloats;
}

- (NSAttributedString*) attributedStringForObjectValue: (id)anObject
				 withDefaultAttributes: (NSDictionary*)attr
{
  NSString *stringForObjectValue;
  NSDecimalNumber *zeroNumber = [NSDecimalNumber zero];
  NSDecimalNumber *nanNumber = [NSDecimalNumber notANumber];

  if (anObject == nil)
    {
      return [self attributedStringForNil];
    }
  else if (![anObject isKindOfClass: [NSNumber class]])
    {
      return [self attributedStringForNotANumber];
    }
  else if ([anObject isEqual: nanNumber])
    {
      return [self attributedStringForNotANumber];
    }
  else if ([anObject isEqual: zeroNumber])
    {
      return [self attributedStringForZero];
    }

  if (([(NSNumber*)anObject compare: zeroNumber] == NSOrderedDescending)
    && (_attributesForPositiveValues))
    {
      attr = _attributesForPositiveValues;
    }
  else if (([(NSNumber*)anObject compare: zeroNumber] == NSOrderedAscending)
    && (_attributesForNegativeValues))
    {
      attr = _attributesForNegativeValues;
    }

  stringForObjectValue = [self stringForObjectValue: anObject];
  
  if (stringForObjectValue == nil)
    {
      stringForObjectValue = @"";
    }

  return AUTORELEASE([[NSAttributedString alloc] initWithString:
     stringForObjectValue attributes: attr]);
}

- (NSAttributedString*) attributedStringForNil
{
  return _attributedStringForNil;
}

- (NSAttributedString*) attributedStringForNotANumber
{
  return _attributedStringForNotANumber;
}

- (NSAttributedString*) attributedStringForZero
{
  return _attributedStringForZero;
}

- (id) copyWithZone: (NSZone *)zone
{
  NSNumberFormatter	*o = (NSNumberFormatter*) NSCopyObject(self, 0, zone);

  IF_NO_GC(RETAIN(o->_negativeFormat);)
  IF_NO_GC(RETAIN(o->_positiveFormat);)
  IF_NO_GC(RETAIN(o->_attributesForPositiveValues);)
  IF_NO_GC(RETAIN(o->_attributesForNegativeValues);)
  IF_NO_GC(RETAIN(o->_maximum);)
  IF_NO_GC(RETAIN(o->_minimum);)
  IF_NO_GC(RETAIN(o->_roundingBehavior);)
  IF_NO_GC(RETAIN(o->_attributedStringForNil);)
  IF_NO_GC(RETAIN(o->_attributedStringForNotANumber);)
  IF_NO_GC(RETAIN(o->_attributedStringForZero);)
  if (0 != internal)
    {
      int	idx;

      GS_COPY_INTERNAL(o, zone)
      IF_NO_GC(
	[GSIVar(o,_locale) retain];
	for (idx = 0; idx < MAX_SYMBOLS; ++idx)
	  {
	    [GSIVar(o,_symbols)[idx] retain];
	  }
	for (idx = 0; idx < MAX_TEXTATTRIBUTES; ++idx)
	  {
	    [GSIVar(o,_textAttributes)[idx] retain];
	  }
      )
#if GS_USE_ICU == 1
      {
        UErrorCode err = U_ZERO_ERROR;
        GSIVar(o,_formatter) = unum_clone (internal->_formatter, &err);
      }
#endif
    }
  return o;
}

- (void) dealloc
{
  RELEASE(_negativeFormat);
  RELEASE(_positiveFormat);
  RELEASE(_attributesForPositiveValues);
  RELEASE(_attributesForNegativeValues);
  RELEASE(_maximum);
  RELEASE(_minimum);
  RELEASE(_roundingBehavior);
  RELEASE(_attributedStringForNil);
  RELEASE(_attributedStringForNotANumber);
  RELEASE(_attributedStringForZero);
  if (internal != 0)
    {
      int idx;

      RELEASE(internal->_locale);
#if GS_USE_ICU == 1
      unum_close (internal->_formatter);
#endif
      for (idx = 0; idx < MAX_SYMBOLS; ++idx)
	{
	  [internal->_symbols[idx] release];
	}
      for (idx = 0; idx < MAX_TEXTATTRIBUTES; ++idx)
	{
	  [internal->_textAttributes[idx] release];
        }
      GS_DESTROY_INTERNAL(NSNumberFormatter)
    }
  [super dealloc];
}

- (NSString*) decimalSeparator
{
  if (MYBEHAVIOR == NSNumberFormatterBehavior10_4
    || MYBEHAVIOR == NSNumberFormatterBehaviorDefault)
    {
      return [internal symbolForKey: UNUM_DECIMAL_SEPARATOR_SYMBOL];
    }
  else if (MYBEHAVIOR == NSNumberFormatterBehavior10_0)
    {
      if (_decimalSeparator == 0)
        return @"";
      else
        return [NSString stringWithCharacters: &_decimalSeparator length: 1];
    }
  return nil;
}

- (NSString*) editingStringForObjectValue: (id)anObject
{
  return [self stringForObjectValue: anObject];
}

- (void) encodeWithCoder: (NSCoder*)encoder
{
  [encoder encodeValueOfObjCType: @encode(BOOL) at: &_hasThousandSeparators];
  [encoder encodeValueOfObjCType: @encode(BOOL) at: &_allowsFloats];
  [encoder encodeValueOfObjCType: @encode(BOOL) at: &_localizesFormat];
  [encoder encodeValueOfObjCType: @encode(unichar) at: &_thousandSeparator];
  [encoder encodeValueOfObjCType: @encode(unichar) at: &_decimalSeparator];

  [encoder encodeObject: _roundingBehavior];
  [encoder encodeObject: _maximum];
  [encoder encodeObject: _minimum];
  [encoder encodeObject: _attributedStringForNil];
  [encoder encodeObject: _attributedStringForNotANumber];
  [encoder encodeObject: _attributedStringForZero];
  [encoder encodeObject: _negativeFormat];
  [encoder encodeObject: _positiveFormat];
  [encoder encodeObject: _attributesForPositiveValues];
  [encoder encodeObject: _attributesForNegativeValues];
  // FIXME: Add new ivars
}

- (NSString*) format
{
  if (_attributedStringForZero != nil)
    {
      return [NSString stringWithFormat: @"%@;%@;%@",
	_positiveFormat, [_attributedStringForZero string], _negativeFormat];
    }
  else
    {
      return [NSString stringWithFormat: @"%@;%@",
	_positiveFormat, _negativeFormat];
    }
}

- (BOOL) getObjectValue: (id*)anObject
              forString: (NSString*)string
       errorDescription: (NSString**)error
{
  if (MYBEHAVIOR == NSNumberFormatterBehavior10_4
    || MYBEHAVIOR == NSNumberFormatterBehaviorDefault)
    {
      BOOL result;
      NSRange range = NSMakeRange (0, [string length]);
      NSError *outError = nil;
      
      result = [self getObjectValue: anObject
                          forString: string
                              range: &range
                              error: &outError];
      if (!result && error)
        *error = [outError localizedDescription];
      
      return result;
    }
  else if (MYBEHAVIOR == NSNumberFormatterBehavior10_0)
    {
      /* FIXME: This is just a quick hack implementation.  */
      NSLog(@"NSNumberFormatter-getObjectValue:forString:... not fully implemented");

      if (nil == string)
        {
          if (0 != error)
	    {
	      *error = _(@"nil string");
	    }
          return NO;
        }

      if (NO == [self allowsFloats] && [string rangeOfString: @"."].length > 0)
        {
          if (0 != error)
	    {
	      *error = _(@"Floating Point not allowed");
	    }
          return NO;
        }

      /* Just assume nothing else has been setup and do a simple conversion. */
      if ([self hasThousandSeparators])
        {
          NSRange range;
          
          range = [string rangeOfString: [self thousandSeparator]];
          if (range.length != 0)
            {
	      string = AUTORELEASE([string mutableCopy]);
	      [(NSMutableString*)string replaceOccurrencesOfString:
	        [self thousandSeparator]
	        withString: @""
	        options: 0
	        range: NSMakeRange(0, [string length])];
	    }
        }

      if (anObject)
        {
          NSDictionary *locale;
          
          locale = [NSDictionary dictionaryWithObject: [self decimalSeparator] 
			         forKey: NSDecimalSeparator];
          *anObject = [NSDecimalNumber decimalNumberWithString: string
				       locale: locale];
          if (*anObject)
            {
	      return YES;
	    }
        }
    }

  return NO;
}

- (BOOL) hasThousandSeparators
{
  return _hasThousandSeparators;
}

- (id) init
{
  id	o;
  int idx;
  
  GS_CREATE_INTERNAL(NSNumberFormatter)
  
  _allowsFloats = YES;
  _decimalSeparator = '.';
  _thousandSeparator = ',';
  _hasThousandSeparators = YES;
  o = [[NSAttributedString alloc] initWithString: @""];
  [self setAttributedStringForNil: o];
  RELEASE(o);
  o = [[NSAttributedString alloc] initWithString: @"NaN"];
  [self setAttributedStringForNotANumber: o];
  RELEASE(o);
  
  internal->_behavior = _defaultBehavior;
  internal->_locale = RETAIN([NSLocale currentLocale]);
  internal->_style = NSNumberFormatterNoStyle;

  /* Set all attributes to -1 before resetting the formatter.  When
   * resetting them only values < 0 will be skipped.
   */
  for (idx = 0; idx < MAX_ATTRIBUTES; ++idx)
    internal->_attributes[idx] = -1;
  
  [self _resetUNumberFormat];
#if GS_USE_ICU == 1
  if (internal->_formatter == NULL)
    {
      RELEASE(self);
      return nil;
    }
#endif
  
  [self setMaximumFractionDigits: 0];
  
  return self;
}

- (id) initWithCoder: (NSCoder*)decoder
{
  /* We can call our -init method to set up default values, then
   * override settings with information from the archive.
   */
  if (nil == (self = [self init]))
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"Failed to initialise number formatter"];
    }

  if ([decoder allowsKeyedCoding])
    {
      if ([decoder containsValueForKey: @"NS.allowsfloats"])
        {
	  [self setAllowsFloats:
	    [decoder decodeBoolForKey: @"NS.allowsfloats"]];
	}
      if ([decoder containsValueForKey: @"NS.decimal"])
        {
	  [self setDecimalSeparator:
	    [decoder decodeObjectForKey: @"NS.decimal"]];
	}
      if ([decoder containsValueForKey: @"NS.hasthousands"])
        {
	  [self setHasThousandSeparators:
	    [decoder decodeBoolForKey: @"NS.hasthousands"]];
	}
      if ([decoder containsValueForKey: @"NS.localized"])
        {
	  [self setLocalizesFormat:
	    [decoder decodeBoolForKey: @"NS.localized"]];
	}
      if ([decoder containsValueForKey: @"NS.max"])
        {
	  [self setMaximum: [decoder decodeObjectForKey: @"NS.max"]];
	}
      if ([decoder containsValueForKey: @"NS.min"])
        {
	  [self setMinimum: [decoder decodeObjectForKey: @"NS.min"]];
	}
      if ([decoder containsValueForKey: @"NS.nan"])
        {
	  [self setAttributedStringForNotANumber:
	    [decoder decodeObjectForKey: @"NS.nan"]];
	}
      if ([decoder containsValueForKey: @"NS.negativeattrs"])
        {
	  [self setTextAttributesForNegativeValues:
	    [decoder decodeObjectForKey: @"NS.negativeattrs"]];
	}
      if ([decoder containsValueForKey: @"NS.negativeformat"])
        {
	  [self setNegativeFormat:
	    [decoder decodeObjectForKey: @"NS.negativeformat"]];
	}
      if ([decoder containsValueForKey: @"NS.nil"])
        {
	  [self setAttributedStringForNil:
	    [decoder decodeObjectForKey: @"NS.nil"]];
	}
      if ([decoder containsValueForKey: @"NS.positiveattrs"])
        {
	  [self setTextAttributesForPositiveValues:
	    [decoder decodeObjectForKey: @"NS.positiveattrs"]];
	}
      if ([decoder containsValueForKey: @"NS.positiveformat"])
        {
	  [self setPositiveFormat:
	    [decoder decodeObjectForKey: @"NS.positiveformat"]];
	}
      if ([decoder containsValueForKey: @"NS.rounding"])
        {
	  [self setRoundingBehavior:
	    [decoder decodeObjectForKey: @"NS.rounding"]];
	}
      if ([decoder containsValueForKey: @"NS.thousand"])
        {
	  [self setThousandSeparator:
	    [decoder decodeObjectForKey: @"NS.thousand"]];
	}
      if ([decoder containsValueForKey: @"NS.zero"])
        {
	  [self setAttributedStringForZero:
	    [decoder decodeObjectForKey: @"NS.zero"]];
	}
    }
  else
    {
      [decoder decodeValueOfObjCType: @encode(BOOL)
				  at: &_hasThousandSeparators];
      [decoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsFloats];
      [decoder decodeValueOfObjCType: @encode(BOOL) at: &_localizesFormat];
      [decoder decodeValueOfObjCType: @encode(unichar) at: &_thousandSeparator];
      [decoder decodeValueOfObjCType: @encode(unichar) at: &_decimalSeparator];

      [decoder decodeValueOfObjCType: @encode(id) at: &_roundingBehavior];
      [decoder decodeValueOfObjCType: @encode(id) at: &_maximum];
      [decoder decodeValueOfObjCType: @encode(id) at: &_minimum];
      [decoder decodeValueOfObjCType: @encode(id) at: &_attributedStringForNil];
      [decoder decodeValueOfObjCType: @encode(id)
	                          at: &_attributedStringForNotANumber];
      [decoder decodeValueOfObjCType: @encode(id)
				  at: &_attributedStringForZero];
      [decoder decodeValueOfObjCType: @encode(id) at: &_negativeFormat];
      [decoder decodeValueOfObjCType: @encode(id) at: &_positiveFormat];
      [decoder decodeValueOfObjCType: @encode(id)
	                          at: &_attributesForPositiveValues];
      [decoder decodeValueOfObjCType: @encode(id)
	                          at: &_attributesForNegativeValues];
      // FIXME: add new ivars
    }
  return self;
}

- (BOOL) isPartialStringValid: (NSString*)partialString
	     newEditingString: (NSString**)newString
	     errorDescription: (NSString**)error
{
  // FIXME
  if (newString != NULL)
    {
      *newString = partialString;
    }
  if (error)
    {
      *error = nil;
    }

  return YES;
}

- (BOOL) localizesFormat
{
  return _localizesFormat;
}

- (NSDecimalNumber*) maximum
{
  return _maximum;
}

- (NSDecimalNumber*) minimum
{
  return _minimum;
}

- (NSString*) negativeFormat
{
  return _negativeFormat;
}

- (NSString*) positiveFormat
{
  return _positiveFormat;
}

- (NSDecimalNumberHandler*) roundingBehavior
{
  return _roundingBehavior;
}

- (void) setAllowsFloats: (BOOL)flag
{
  _allowsFloats = flag;
}

- (void) setAttributedStringForNil: (NSAttributedString*)newAttributedString
{
  ASSIGN(_attributedStringForNil, newAttributedString);
}

- (void) setAttributedStringForNotANumber:
  (NSAttributedString*)newAttributedString
{
  ASSIGN(_attributedStringForNotANumber, newAttributedString);
}

- (void) setAttributedStringForZero: (NSAttributedString*)newAttributedString
{
  ASSIGN(_attributedStringForZero, newAttributedString);
}

- (void) setDecimalSeparator: (NSString*)newSeparator
{
  if (MYBEHAVIOR == NSNumberFormatterBehavior10_4
    || MYBEHAVIOR == NSNumberFormatterBehaviorDefault)
    {
      [internal setSymbol: newSeparator
		    forKey: UNUM_DECIMAL_SEPARATOR_SYMBOL];
    }
  else if (MYBEHAVIOR == NSNumberFormatterBehavior10_0)
    {
      if ([newSeparator length] > 0)
        _decimalSeparator = [newSeparator characterAtIndex: 0];
      else
        _decimalSeparator = 0;
    }
}

- (void) setFormat: (NSString*)aFormat
{
  NSRange	r;

  r = [aFormat rangeOfString: @";"];
  if (r.length == 0)
    {
      [self setPositiveFormat: aFormat];
      [self setNegativeFormat: [@"-" stringByAppendingString: aFormat]];
    }
  else
    {
      [self setPositiveFormat: [aFormat substringToIndex: r.location]];
      aFormat = [aFormat substringFromIndex: NSMaxRange(r)];
      r = [aFormat rangeOfString: @";"];
      if (r.length == 0)
	{
	  [self setNegativeFormat: aFormat];
	}
      else
	{
	  RELEASE(_attributedStringForZero);
	  _attributedStringForZero = [[NSAttributedString alloc] initWithString:
	    [aFormat substringToIndex: r.location]];
	  [self setNegativeFormat: [aFormat substringFromIndex: NSMaxRange(r)]];
	}
    }
}

- (void) setHasThousandSeparators: (BOOL)flag
{
  _hasThousandSeparators = flag;
}

- (void) setLocalizesFormat: (BOOL)flag
{
  _localizesFormat = flag;
}

- (void) setMaximum: (NSDecimalNumber*)aMaximum
{
  // FIXME: NSNumberFormatterBehavior10_4
  ASSIGN(_maximum, aMaximum);
}

- (void) setMinimum: (NSDecimalNumber*)aMinimum
{
  // FIXME: NSNumberFormatterBehavior10_4
  ASSIGN(_minimum, aMinimum);
}

- (void) setNegativeFormat: (NSString*)aFormat
{
  // FIXME: Should extract separators and attributes
  ASSIGN(_negativeFormat, aFormat);
}

- (void) setPositiveFormat: (NSString*)aFormat
{
  // FIXME: Should extract separators and attributes
  ASSIGN(_positiveFormat, aFormat);
}

- (void) setRoundingBehavior: (NSDecimalNumberHandler*)newRoundingBehavior
{
  ASSIGN(_roundingBehavior, newRoundingBehavior);
}

- (void) setTextAttributesForNegativeValues: (NSDictionary*)newAttributes
{
  ASSIGN(_attributesForNegativeValues, newAttributes);
}

- (void) setTextAttributesForPositiveValues: (NSDictionary*)newAttributes
{
  ASSIGN(_attributesForPositiveValues, newAttributes);
}

- (void) setThousandSeparator: (NSString*)newSeparator
{
  if ([newSeparator length] > 0)
    _thousandSeparator = [newSeparator characterAtIndex: 0];
  else
    _thousandSeparator = 0;
}

- (NSString*) stringForObjectValue: (id)anObject
{
  if (MYBEHAVIOR == NSNumberFormatterBehaviorDefault
    || MYBEHAVIOR == NSNumberFormatterBehavior10_4)
    {
#if GS_USE_ICU == 1

#define STRING_FROM_NUMBER(function, number) do \
  { \
    UErrorCode err = U_ZERO_ERROR; \
    int32_t len; \
    NSString *result; \
    \
    len = function (internal->_formatter, number, buffer, \
      BUFFER_SIZE, NULL, &err); \
    if (len > BUFFER_SIZE) \
      len = BUFFER_SIZE; \
    result = [NSString stringWithCharacters: buffer length: len]; \
    return result; \
  } while (0)

      /* This is quite inefficient.  See the GSUText stuff for how
       * to use ICU 4.6 UText objects as NSStrings.  This saves us from
       * needing to do a load of O(n) things.  In 4.6, these APIs in ICU
       * haven't been updated to use UText (so we have to use the UChar buffer
       * approach), but they probably will be in the future.  We should
       * revisit this code when they have been.
       */
      UChar buffer[BUFFER_SIZE];
      
      /* FIXME: What to do with unsigned types?
       *
       * The only unsigned case we actually need to worry about is unsigned
       * long long - all of the others are stored as signed values.  We're now
       * falling through to the double case for this, which will lose us some
       * precision, but hopefully not matter too much...
       */
      if (nil == anObject)
        return [self nilSymbol];
      if (![anObject isKindOfClass: [NSNumber class]])
        return [self notANumberSymbol];
      switch ([anObject objCType][0])
        {
          case _C_LNG_LNG:
            STRING_FROM_NUMBER(unum_formatInt64, [anObject longLongValue]);
            break;
          case _C_INT:
            STRING_FROM_NUMBER(unum_format, [anObject intValue]);
            break;
          /* Note: This case is probably wrong: the compiler doesn't generate B
           * for bool, it generates C or c (depending on the platform).  I
           * don't think it matters, because we don't bother with anything
           * smaller than int for NSNumbers
	   */
#if __GNUC__ > 2 && defined(_C_BOOL)
          case _C_BOOL:
            STRING_FROM_NUMBER(unum_format, (int)[anObject boolValue]);
            break;
#endif
          /* If it's not a type encoding that we recognise, let the receiver
           * cast it to a double, which probably has enough precision for what
           * we need.  This needs testing with NSDecimalNumber though, because
           * I managed to break stuff last time I did anything with NSNumber by
           * forgetting that NSDecimalNumber existed...
           */
          default:
          case _C_DBL:
            STRING_FROM_NUMBER(unum_formatDouble, [anObject doubleValue]);
            break;
          case _C_FLT:
            STRING_FROM_NUMBER(unum_formatDouble,
	      (double)[anObject floatValue]);
            break;
        }
#endif
    }
  else if (MYBEHAVIOR == NSNumberFormatterBehavior10_0)
    {
      NSMutableDictionary	*locale;
      NSCharacterSet	*formattingCharacters;
      NSCharacterSet	*placeHolders;
      NSString		*prefix;
      NSString		*suffix;
      NSString		*wholeString;
      NSString		*fracPad = nil;
      NSString		*fracPartString;
      NSMutableString	*intPartString;
      NSMutableString	*formattedNumber;
      NSMutableString	*intPad;
      NSRange		prefixRange;
      NSRange		decimalPlaceRange;
      NSRange		suffixRange;
      NSRange		intPartRange;
      NSDecimal		representativeDecimal;
      NSDecimal		roundedDecimal;
      NSDecimalNumber	*roundedNumber;
      NSDecimalNumber	*intPart;
      NSDecimalNumber	*fracPart;
      int		decimalPlaces = 0;
      BOOL		displayThousandsSeparators = NO;
      BOOL		displayFractionalPart = NO;
      BOOL		negativeNumber = NO;
      NSString		*useFormat;
      NSString		*defaultDecimalSeparator = nil;
      NSString		*defaultThousandsSeparator = nil;

      if (_localizesFormat)
        {
          NSDictionary *defaultLocale = GSDomainFromDefaultLocale();

          defaultDecimalSeparator 
	    = [defaultLocale objectForKey: NSDecimalSeparator];
          defaultThousandsSeparator 
	    = [defaultLocale objectForKey: NSThousandsSeparator];
        }

      if (defaultDecimalSeparator == nil)
        {
          defaultDecimalSeparator = @".";
        }
      if (defaultThousandsSeparator == nil)
        {
          defaultThousandsSeparator = @",";
        }
      formattingCharacters = [NSCharacterSet
        characterSetWithCharactersInString: @"0123456789#.,_"];
      placeHolders = [NSCharacterSet 
        characterSetWithCharactersInString: @"0123456789#_"];

      if (nil == anObject)
        return [[self attributedStringForNil] string];
      if (![anObject isKindOfClass: [NSNumber class]])
        return [[self attributedStringForNotANumber] string];
      if ([anObject isEqual: [NSDecimalNumber notANumber]])
        return [[self attributedStringForNotANumber] string];
      if (_attributedStringForZero
          && [anObject isEqual: [NSDecimalNumber zero]])
        return [[self attributedStringForZero] string];
      
      useFormat = _positiveFormat;
      if ([(NSNumber*)anObject compare: [NSDecimalNumber zero]]
        == NSOrderedAscending)
        {
          useFormat = _negativeFormat;
          negativeNumber = YES;
        }

      // if no format specified, use the same default that Cocoa does
      if (nil == useFormat)
        {
          useFormat = [NSString stringWithFormat: @"%@#%@###%@##",
	    negativeNumber ? @"-" : @"",
	    defaultThousandsSeparator,
	    defaultDecimalSeparator];
        }

      prefixRange = [useFormat rangeOfCharacterFromSet: formattingCharacters];
      if (NSNotFound != prefixRange.location)
        {
          prefix = [useFormat substringToIndex: prefixRange.location];
        }
      else
        {
          prefix = @"";
        }

      locale = [NSMutableDictionary dictionaryWithCapacity: 3];
      [locale setObject: @"" forKey: NSThousandsSeparator];
      [locale setObject: @"" forKey: NSDecimalSeparator];

      //should also set NSDecimalDigits?
      
      if ([self hasThousandSeparators]
        && (0 != [useFormat rangeOfString: defaultThousandsSeparator].length))
        {
          displayThousandsSeparators = YES;
        }

      if (NSNotFound
	!= [useFormat rangeOfString: defaultDecimalSeparator].location)
        {
          decimalPlaceRange = [useFormat rangeOfString: defaultDecimalSeparator
			           options: NSBackwardsSearch];
          if (NSMaxRange(decimalPlaceRange) == [useFormat length])
            {
              decimalPlaces = 0;
            }
          else
            {
              while ([placeHolders characterIsMember:
		[useFormat characterAtIndex: NSMaxRange(decimalPlaceRange)]])
                {
                  decimalPlaceRange.length++;
                  if (NSMaxRange(decimalPlaceRange) == [useFormat length])
                    break;
                }
              decimalPlaces=decimalPlaceRange.length -= 1;
              decimalPlaceRange.location += 1;
              fracPad = [useFormat substringWithRange:decimalPlaceRange];
            } 
          if (0 != decimalPlaces)
            displayFractionalPart = YES;
        }

      representativeDecimal = [anObject decimalValue];
      NSDecimalRound(&roundedDecimal, &representativeDecimal, decimalPlaces,
        NSRoundPlain);
      roundedNumber =
        [NSDecimalNumber decimalNumberWithDecimal: roundedDecimal];

      /* Arguably this fiddling could be done by GSDecimalString() but I
       * thought better to leave that behaviour as it is and provide the
       * desired prettification here
       */
      if (negativeNumber)
        roundedNumber = [roundedNumber decimalNumberByMultiplyingBy:
          (NSDecimalNumber*)[NSDecimalNumber numberWithInt: -1]];
      intPart = (NSDecimalNumber*)
        [NSDecimalNumber numberWithInt: (int)[roundedNumber doubleValue]];
      fracPart = [roundedNumber decimalNumberBySubtracting: intPart];
      intPartString
        = AUTORELEASE([[intPart descriptionWithLocale: locale] mutableCopy]);
      
      //sort out the padding for the integer part
      intPartRange = [useFormat rangeOfCharacterFromSet: placeHolders];
      if (intPartRange.location != NSNotFound)
        {
          int nextFormatCharLoc = intPartRange.location;
          while (([placeHolders characterIsMember:
            [useFormat characterAtIndex: nextFormatCharLoc]]
            || [[useFormat substringWithRange:
              NSMakeRange(nextFormatCharLoc, 1)] isEqual:
          defaultThousandsSeparator])
            && nextFormatCharLoc < [useFormat length] - 1)
            {
              intPartRange.length++;
              nextFormatCharLoc++;
            }
        }
      intPad = [[[useFormat substringWithRange: intPartRange]
	mutableCopy] autorelease];
      [intPad replaceOccurrencesOfString: defaultThousandsSeparator
        withString: @""
        options: 0
        range: NSMakeRange(0, [intPad length])];
      [intPad replaceOccurrencesOfString: @"#"
        withString: @""
        options: NSAnchoredSearch
        range: NSMakeRange(0, [intPad length])];
      if ([intPad length] > [intPartString length])
        {
          NSRange		ipRange;

          ipRange =
            NSMakeRange(0, [intPad length] - [intPartString length]);
          [intPartString insertString:
            [intPad substringWithRange: ipRange] atIndex: 0];
          [intPartString replaceOccurrencesOfString: @"_"
	    withString: @" "
	    options: 0
	    range: NSMakeRange(0, [intPartString length])];
          [intPartString replaceOccurrencesOfString: @"#"
	    withString: @"0"
	    options: 0
	    range: NSMakeRange(0, [intPartString length])];
        }
      // fix the thousands separators up
      if (displayThousandsSeparators && [intPartString length] > 3)
        {
          int index = [intPartString length];

          while (0 < (index -= 3))
	    {
	      [intPartString insertString: [self thousandSeparator]
				  atIndex: index];
	    }
        }

      formattedNumber = [intPartString mutableCopy];

      //fix up the fractional part
      if (displayFractionalPart)
        {
          NSMutableString	*ms;

          fracPart = [fracPart decimalNumberByMultiplyingByPowerOf10:
            decimalPlaces];
          ms = [[fracPart descriptionWithLocale: locale] mutableCopy];
          if ([fracPad length] > [ms length])
            {
              NSRange fpRange;

              fpRange = NSMakeRange([ms length],
                ([fracPad length] - [ms length]));
              [ms insertString:
                [fracPad substringWithRange: fpRange] atIndex: 0];
              [ms replaceOccurrencesOfString: @"#"
                withString: @""
                options: (NSBackwardsSearch | NSAnchoredSearch)
                range: NSMakeRange(0, [ms length])];
              [ms replaceOccurrencesOfString: @"#"
                withString: @"0"
                options: 0
                range: NSMakeRange(0, [ms length])];
              [ms replaceOccurrencesOfString: @"_"
                withString: @" "
                options: 0
                range: NSMakeRange(0, [ms length])];
            }
          [ms replaceOccurrencesOfString: @"0"
            withString: @""
            options: (NSBackwardsSearch | NSAnchoredSearch)
            range: NSMakeRange(0, [ms length])];
          fracPartString = AUTORELEASE(ms);
          [formattedNumber appendString: [self decimalSeparator]];
          [formattedNumber appendString: fracPartString];
        }
      /*FIXME - the suffix doesn't behave the same as on Mac OS X.
       * Our suffix is everything which follows the final formatting
       * character.  Cocoa's suffix is everything which isn't a
       * formatting character nor in the prefix
       */
      suffixRange = [useFormat rangeOfCharacterFromSet: formattingCharacters
        options: NSBackwardsSearch];
      suffix = [useFormat substringFromIndex: NSMaxRange(suffixRange)];
      wholeString = [[prefix stringByAppendingString: formattedNumber]
        stringByAppendingString: suffix];
      [formattedNumber release];
      return wholeString;
    }
  return nil;
}

- (NSDictionary*) textAttributesForNegativeValues
{
  return _attributesForNegativeValues;
}

- (NSDictionary*) textAttributesForPositiveValues
{
  return _attributesForPositiveValues;
}

- (NSString*) thousandSeparator
{
  if (!_thousandSeparator)
    return @"";
  else
    return [NSString stringWithCharacters: &_thousandSeparator length: 1];
}

- (NSString *) stringFromNumber: (NSNumber *)number
{
// This is a 10.4 and above method and should not work with earlier version.
  return [self stringForObjectValue: number];
}

- (NSNumber *) numberFromString: (NSString *)string
{
// This is a 10.4 and above method and should not work with earlier version.
#if GS_USE_ICU == 1
  NSNumber *result;
  NSUInteger length;
  NSRange range;
  UErrorCode err = U_ZERO_ERROR;
  unichar *ustring;
  int64_t intNum;
  double doubleNum;
  
  if (string == nil)
    return nil;
  
  length = [string length];
  ustring = NSZoneMalloc ([self zone], sizeof(unichar) * length);
  if (ustring == NULL)
    return nil;
  
  [string getCharacters: ustring range: NSMakeRange(0, length)];
  
  // FIXME: Not sure if this is correct....
  range = [string rangeOfString: @"."];
  if (range.location == NSNotFound)
    {
      intNum = unum_parseInt64(internal->_formatter,
        ustring, length, NULL, &err);
      if (U_FAILURE(err))
        return nil;
      if (intNum == 0 || intNum == 1)
        result = [NSNumber numberWithBool: (BOOL) intNum];
      else if (intNum < INT_MAX && intNum > INT_MIN)
        result = [NSNumber numberWithInt: (int32_t)intNum];
      else
        result = [NSNumber numberWithLongLong: intNum];
    }
  else
    {
      doubleNum = unum_parseDouble(internal->_formatter,
        ustring, length, NULL, &err);
      if (U_FAILURE(err))
        return nil;
      result = [NSNumber numberWithDouble: doubleNum];
    }
  
  NSZoneFree ([self zone], ustring);
  return result;
#else
  return nil;
#endif
}



- (void) setFormatterBehavior: (NSNumberFormatterBehavior)behavior
{
  if (NSNumberFormatterBehavior10_4 == behavior
    || NSNumberFormatterBehavior10_0 == behavior
    || NSNumberFormatterBehaviorDefault == behavior)
    {
      internal->_behavior = behavior;
    }
}

- (NSNumberFormatterBehavior)formatterBehavior
{
  return internal->_behavior;
}

+ (void) setDefaultFormatterBehavior: (NSNumberFormatterBehavior)behavior
{
  if (NSNumberFormatterBehavior10_0 == behavior)
    {
      _defaultBehavior = behavior;
    }
  else if (NSNumberFormatterBehavior10_4 == behavior
    || NSNumberFormatterBehaviorDefault == behavior)
    {
      _defaultBehavior = NSNumberFormatterBehavior10_4;
    }
  else
    {
      _defaultBehavior = behavior;	// Yeuch ... but OSX accepts any value
    }
}

+ (NSNumberFormatterBehavior) defaultFormatterBehavior
{
  return _defaultBehavior;
}

- (void) setNumberStyle: (NSNumberFormatterStyle) style
{
  internal->_style = style;
  [self _resetUNumberFormat];
}

- (NSNumberFormatterStyle) numberStyle
{
  return internal->_style;
}

- (void) setGeneratesDecimalNumbers: (BOOL) flag
{
  internal->_genDecimal = flag;
}

- (BOOL) generatesDecimalNumbers
{
  return internal->_genDecimal; // FIXME
}

- (void) setLocale: (NSLocale *)locale
{
  if (nil == locale)
    {
      locale = [NSLocale currentLocale];
    }
  if (NO == [locale isEqual: internal->_locale])
    {
      ASSIGN(internal->_locale, locale);
      [self _resetUNumberFormat];
    }
}

- (NSLocale *) locale
{
  return internal->_locale;
}


- (void) setRoundingIncrement: (NSNumber *) number
{
  switch ([number objCType][0])
    {
      case 'd':
      case 'f':
#if GS_USE_ICU == 1
    unum_setDoubleAttribute (internal->_formatter, UNUM_ROUNDING_INCREMENT,
      [number doubleValue]);
#endif
      default:
        return;
    }
}

- (NSNumber *) roundingIncrement
{
#if GS_USE_ICU == 1
  double value = unum_getDoubleAttribute (internal->_formatter, UNUM_ROUNDING_INCREMENT);
  return [NSNumber numberWithDouble: value];
#else
  return nil;
#endif
}

- (void) setRoundingMode: (NSNumberFormatterRoundingMode) mode
{
  [internal setAttribute: NSToICURoundingMode(mode) forKey: UNUM_ROUNDING_MODE];
}

- (NSNumberFormatterRoundingMode) roundingMode
{
  return ICUToNSRoundingMode([internal attributeForKey: UNUM_ROUNDING_MODE]);
}


- (void) setFormatWidth: (NSUInteger) number
{
  [internal setAttribute: number forKey: UNUM_FORMAT_WIDTH];
}

- (NSUInteger) formatWidth
{
  return (NSUInteger)[internal attributeForKey: UNUM_FORMAT_WIDTH];
}

- (void) setMultiplier: (NSNumber *) number
{
  [internal setAttribute: [number intValue] forKey: UNUM_MULTIPLIER];
}

- (NSNumber *) multiplier
{
  return [NSNumber numberWithInt: [internal attributeForKey: UNUM_MULTIPLIER]];
}


- (void) setPercentSymbol: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_PERCENT_SYMBOL];
}

- (NSString *) percentSymbol
{
  return [internal symbolForKey: UNUM_PERCENT_SYMBOL];
}

- (void) setPerMillSymbol: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_PERMILL_SYMBOL];
}

- (NSString *) perMillSymbol
{
  return [internal symbolForKey: UNUM_PERMILL_SYMBOL];
}

- (void) setMinusSign: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_MINUS_SIGN_SYMBOL];
}

- (NSString *) minusSign
{
  return [internal symbolForKey: UNUM_MINUS_SIGN_SYMBOL];
}

- (void) setPlusSign: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_PLUS_SIGN_SYMBOL];
}

- (NSString *) plusSign
{
  return [internal symbolForKey: UNUM_PLUS_SIGN_SYMBOL];
}

- (void) setExponentSymbol: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_EXPONENTIAL_SYMBOL];
}

- (NSString *) exponentSymbol
{
  return [internal symbolForKey: UNUM_EXPONENTIAL_SYMBOL];
}

- (void) setZeroSymbol: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_ZERO_DIGIT_SYMBOL];
}

- (NSString *) zeroSymbol
{
  return [internal symbolForKey: UNUM_ZERO_DIGIT_SYMBOL];
}

- (void) setNilSymbol: (NSString *) string
{
  return; // FIXME
}

- (NSString *) nilSymbol
{
  return nil; // FIXME
}

- (void) setNotANumberSymbol: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_NAN_SYMBOL];
}

- (NSString *) notANumberSymbol
{
  return [internal symbolForKey: UNUM_NAN_SYMBOL];
}

- (void) setNegativeInfinitySymbol: (NSString *) string
{
  // FIXME: ICU doesn't differenciate between positive and negative infinity.
  [internal setSymbol: string forKey: UNUM_INFINITY_SYMBOL];
}

- (NSString *) negativeInfinitySymbol
{
  return [internal symbolForKey: UNUM_INFINITY_SYMBOL];
}

- (void) setPositiveInfinitySymbol: (NSString *) string
{
  // FIXME: ICU doesn't differenciate between positive and negative infinity.
  [internal setSymbol: string forKey: UNUM_INFINITY_SYMBOL];
}

- (NSString *) positiveInfinitySymbol
{
  return [internal symbolForKey: UNUM_INFINITY_SYMBOL];
}


- (void) setCurrencySymbol: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_CURRENCY_SYMBOL];
}

- (NSString *) currencySymbol
{
  return [internal symbolForKey: UNUM_CURRENCY_SYMBOL];
}

- (void) setCurrencyCode: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_CURRENCY_CODE];
}

- (NSString *) currencyCode
{
  return [internal textAttributeForKey: UNUM_CURRENCY_CODE];
}

- (void) setInternationalCurrencySymbol: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_INTL_CURRENCY_SYMBOL];
}

- (NSString *) internationalCurrencySymbol
{
  return [internal symbolForKey: UNUM_INTL_CURRENCY_SYMBOL];
}


- (void) setPositivePrefix: (NSString *) string
{
  [internal setTextAttribute: string forKey: UNUM_POSITIVE_PREFIX];
}

- (NSString *) positivePrefix
{
  return [internal textAttributeForKey: UNUM_POSITIVE_PREFIX];
}

- (void) setPositiveSuffix: (NSString *) string
{
  [internal setTextAttribute: string forKey: UNUM_POSITIVE_SUFFIX];
}

- (NSString *) positiveSuffix
{
  return [internal textAttributeForKey: UNUM_POSITIVE_SUFFIX];
}

- (void) setNegativePrefix: (NSString *) string
{
  [internal setTextAttribute: string forKey: UNUM_NEGATIVE_PREFIX];
}

- (NSString *) negativePrefix
{
  return [internal textAttributeForKey: UNUM_NEGATIVE_PREFIX];
}

- (void) setNegativeSuffix: (NSString *) string
{
  [internal setTextAttribute: string forKey: UNUM_NEGATIVE_SUFFIX];
}

- (NSString *) negativeSuffix
{
  return [internal textAttributeForKey: UNUM_NEGATIVE_SUFFIX];
}


- (void) setTextAttributesForZero: (NSDictionary *) newAttributes
{
  return;  // FIXME
}

- (NSDictionary *) textAttributesForZero
{
  return nil; // FIXME
}

- (void) setTextAttributesForNil: (NSDictionary *) newAttributes
{
  return;
}

- (NSDictionary *) textAttributesForNil
{
  return nil; // FIXME
}

- (void) setTextAttributesForNotANumber: (NSDictionary *) newAttributes
{
  return; // FIXME
}

- (NSDictionary *) textAttributesForNotANumber
{
  return nil; // FIXME
}

- (void) setTextAttributesForPositiveInfinity: (NSDictionary *) newAttributes
{
  return; // FIXME
}

- (NSDictionary *) textAttributesForPositiveInfinity
{
  return nil; // FIXME
}

- (void) setTextAttributesForNegativeInfinity: (NSDictionary *) newAttributes
{
  return; // FIXME
}

- (NSDictionary *) textAttributesForNegativeInfinity
{
  return nil; // FIXME
}


- (void) setGroupingSeparator: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_GROUPING_SEPARATOR_SYMBOL];
}

- (NSString *) groupingSeparator
{
  return [internal symbolForKey: UNUM_GROUPING_SEPARATOR_SYMBOL];
}

- (void) setUsesGroupingSeparator: (BOOL)flag
{
#if GS_USE_ICU == 1
  [internal setBool: flag forKey: UNUM_GROUPING_USED];
#else
  return;
#endif
}

- (BOOL) usesGroupingSeparator
{
  return [internal boolForKey: UNUM_GROUPING_USED];
}

- (void) setAlwaysShowsDecimalSeparator: (BOOL)flag
{
#if GS_USE_ICU == 1
  [internal setBool: flag forKey: UNUM_DECIMAL_ALWAYS_SHOWN];
#else
  return;
#endif
}

- (BOOL) alwaysShowsDecimalSeparator
{
  return [internal boolForKey: UNUM_DECIMAL_ALWAYS_SHOWN];
}

- (void) setCurrencyDecimalSeparator: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_MONETARY_SEPARATOR_SYMBOL];
}

- (NSString *) currencyDecimalSeparator
{
  return [internal symbolForKey: UNUM_MONETARY_SEPARATOR_SYMBOL];
}

- (void) setGroupingSize: (NSUInteger) number
{
  [internal setAttribute: number forKey: UNUM_GROUPING_SIZE];
}

- (NSUInteger) groupingSize
{
#if GS_USE_ICU == 1
  return (NSUInteger)[internal attributeForKey: UNUM_GROUPING_SIZE];
#else
  return 3;
#endif
}

- (void) setSecondaryGroupingSize: (NSUInteger) number
{
  [internal setAttribute: number forKey: UNUM_SECONDARY_GROUPING_SIZE];
}

- (NSUInteger) secondaryGroupingSize
{
#if GS_USE_ICU == 1
  return (NSUInteger)[internal attributeForKey: UNUM_SECONDARY_GROUPING_SIZE];
#else
  return 3;
#endif
}


- (void) setPaddingCharacter: (NSString *) string
{
  [internal setTextAttribute: string forKey: UNUM_PADDING_CHARACTER];
}

- (NSString *) paddingCharacter
{
  return [internal textAttributeForKey: UNUM_PADDING_CHARACTER];
}

- (void) setPaddingPosition: (NSNumberFormatterPadPosition) position
{
  [internal setAttribute: NSToICUPadPosition(position)
		  forKey: UNUM_PADDING_POSITION];
}

- (NSNumberFormatterPadPosition) paddingPosition
{
  return ICUToNSPadPosition([internal attributeForKey: UNUM_PADDING_POSITION]);
}


- (void) setMinimumIntegerDigits: (NSUInteger) number
{
  [internal setAttribute: number forKey: UNUM_MIN_INTEGER_DIGITS];
}

- (NSUInteger) minimumIntegerDigits
{
  return (NSUInteger)[internal attributeForKey: UNUM_MIN_INTEGER_DIGITS];
}

- (void) setMinimumFractionDigits: (NSUInteger) number
{
  [internal setAttribute: number forKey: UNUM_MIN_FRACTION_DIGITS];
}

- (NSUInteger) minimumFractionDigits
{
  return (NSUInteger)[internal attributeForKey: UNUM_MIN_FRACTION_DIGITS];
}

- (void) setMaximumIntegerDigits: (NSUInteger) number
{
  [internal setAttribute: number forKey: UNUM_MAX_INTEGER_DIGITS];
}

- (NSUInteger) maximumIntegerDigits
{
  return (NSUInteger)[internal attributeForKey: UNUM_MAX_INTEGER_DIGITS];
}

- (void) setMaximumFractionDigits: (NSUInteger) number
{
  [internal setAttribute: number forKey: UNUM_MAX_FRACTION_DIGITS];
}

- (NSUInteger) maximumFractionDigits
{
  return (NSUInteger)[internal attributeForKey: UNUM_MAX_FRACTION_DIGITS];
}


- (BOOL) getObjectValue: (out id *) anObject
              forString: (NSString *) aString
                  range: (NSRange *) rangep
                  error: (out NSError **) error
{
#if GS_USE_ICU == 1
  BOOL result;
  BOOL genDec = [self generatesDecimalNumbers];
  NSUInteger inLen;
  int32_t parsePos = rangep->location;
  UChar inBuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  inLen = [aString length];
  if (inLen > BUFFER_SIZE)
    inLen = BUFFER_SIZE;
  [aString getCharacters: inBuffer range: NSMakeRange(0, inLen)];
  
  if (genDec)  // Generate decimal number?  This should be the default.
    {
#if 0 // FIXME: The unum_parseDecimal function is only available on ICU > 4.4
      int32_t outLen;
      char outBuffer[BUFFER_SIZE];
      
      outLen = 
        unum_parseDecimal (internal->_formatter, inBuffer, inLen, &parsePos,
          outBuffer, BUFFER_SIZE-1, &err);
      if (U_SUCCESS(err))
        {
          NSString *outStr = [NSString stringWithCString: outBuffer
                                                  length: outLen];
          *anObject = [NSDecimalNumber decimalNumberWithString: outStr];
          result = YES;
        }
      else
        {
          if (error)
            *error = [NSError errorWithDomain: @"NSNumberFormatterParseError"
                                         code: err
                                     userInfo: nil];
          *anObject = nil;
          *rangep = NSMakeRange (rangep->location, parsePos);
          result = NO;
        }
#endif
      result = NO;
    }
  else  // If not generating NSDecimalNumber use unum_parseDouble()
    {
      double output;
      
      output = 
        unum_parseDouble (internal->_formatter, inBuffer, inLen, &parsePos,
          &err);
      if (U_SUCCESS(err))
        {
          *anObject = [NSNumber numberWithDouble: output];
          result = YES;
        }
      else
        {
          if (error)
            *error = [NSError errorWithDomain: @"NSNumberFormatterParseError"
                                         code: err
                                     userInfo: nil];
          *anObject = nil;
          *rangep = NSMakeRange (rangep->location, parsePos);
          result = NO;
        }
    }
  
  return result;
#else
  return NO;
#endif
}


- (void) setUsesSignificantDigits: (BOOL)flag
{
#if GS_USE_ICU == 1
  [internal setBool: flag forKey: UNUM_SIGNIFICANT_DIGITS_USED];
#else
  return;
#endif
}

- (BOOL) usesSignificantDigits
{
  return [internal boolForKey: UNUM_SIGNIFICANT_DIGITS_USED];
}

- (void) setMinimumSignificantDigits: (NSUInteger) number
{
  [internal setAttribute: number forKey: UNUM_MIN_SIGNIFICANT_DIGITS];
}

- (NSUInteger) minimumSignificantDigits
{
  return (NSUInteger)[internal attributeForKey: UNUM_MIN_SIGNIFICANT_DIGITS];
}

- (void) setMaximumSignificantDigits: (NSUInteger) number
{
  [internal setAttribute: number forKey: UNUM_MAX_SIGNIFICANT_DIGITS];
}

- (NSUInteger) maximumSignificantDigits
{
  return (NSUInteger)[internal attributeForKey: UNUM_MAX_SIGNIFICANT_DIGITS];
}


- (void) setCurrencyGroupingSeparator: (NSString *) string
{
  [internal setSymbol: string forKey: UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL];
}

- (NSString *) currencyGroupingSeparator
{
  return [internal symbolForKey: UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL];
}


- (void) setLenient: (BOOL)flag
{
  [internal setBool: flag forKey: UNUM_LENIENT_PARSE];
}

- (BOOL) isLenient
{
  return [internal boolForKey: UNUM_LENIENT_PARSE];
}


- (void) setPartialStringValidationEnabled: (BOOL) enabled
{
  return;
}

- (BOOL) isPartialStringValidationEnabled
{
  return NO;
}


+ (NSString *) localizedStringFromNumber: (NSNumber *) num
    numberStyle: (NSNumberFormatterStyle) localizationStyle
{
#if GS_USE_ICU == 1
  NSNumberFormatter *fmt;
  NSString *result;
  
  fmt = [[NSNumberFormatter alloc] init];
  [fmt setLocale: [NSLocale currentLocale]];
  [fmt setNumberStyle: localizationStyle];
  
  result = [fmt stringFromNumber: num];
  RELEASE(fmt);
  return result;
#else
  return nil;
#endif
}

@end

@implementation NSNumberFormatter (PrivateMethods)
- (void) _resetUNumberFormat
{
#if GS_USE_ICU == 1
  unichar               buffer[BUFFER_SIZE];
  NSUInteger            length;
  UNumberFormatStyle    style;
  UErrorCode            err = U_ZERO_ERROR;
  const char            *cLocaleId;
  int32_t               idx;
  
  if (internal->_formatter)
    {
      unum_close(internal->_formatter);
    } 
  cLocaleId = [[internal->_locale localeIdentifier] UTF8String];
  style = NSToICUFormatStyle (internal->_style);
  internal->_formatter = unum_open (style, NULL, 0, cLocaleId, NULL, &err);
  if (U_FAILURE(err))
    {
      internal->_formatter = NULL;
     } 
  // Reset all properties
  for (idx = 0; idx < MAX_SYMBOLS; ++idx)
    {
      if (nil != internal->_symbols[idx])
	{
	  length = [internal->_symbols[idx] length];
	  if (length > BUFFER_SIZE)
            {
              length = BUFFER_SIZE;
            }
	  [internal->_symbols[idx] getCharacters: buffer
					   range: NSMakeRange (0, length)];
	  unum_setSymbol (internal->_formatter, idx, buffer, length, &err);
	}
    }

  for (idx = 0; idx < MAX_TEXTATTRIBUTES; ++idx)
    {
      if (nil != internal->_textAttributes[idx])
	{
	  length = [internal->_textAttributes[idx] length];
	  if (length > BUFFER_SIZE)
            {
              length = BUFFER_SIZE;
            }
	  [internal->_textAttributes[idx] getCharacters: buffer
            range: NSMakeRange (0, length)];
	  unum_setTextAttribute
	    (internal->_formatter, idx, buffer, length, &err);
	}
    }

  for (idx = 0; idx < MAX_ATTRIBUTES; ++idx)
    {
      if (internal->_attributes[idx] >= 0)
	{
          unum_setAttribute (internal->_formatter, idx,
	    internal->_attributes[idx]);
	}
    }
#else
  return;
#endif
}
@end
