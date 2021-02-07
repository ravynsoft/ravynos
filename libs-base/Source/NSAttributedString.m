/**
   NSAttributedString.m

   Implementation of string class with attributes

   Copyright (C) 1997,1999 Free Software Foundation, Inc.

   Written by: ANOQ of the sun <anoq@vip.cybercity.dk>
   Date: November 1997
   Rewrite by: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: April 1999

   This file is part of GNUstep-base

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   If you are interested in a warranty or support for this source code,
   contact Scott Christley <scottc@net-community.com> for more information.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   <title>NSAttributedString class reference</title>
   $Date$ $Revision$
*/

/* Warning -	[-initWithString:attributes:] is the designated initialiser,
 *		but it doesn't provide any way to perform the function of the
 *		[-initWithAttributedString:] initialiser.
 *		In order to work round this, the string argument of the
 *		designated initialiser has been overloaded such that it
 *		is expected to accept an NSAttributedString here instead of
 *		a string.  If you create an NSAttributedString subclass, you
 *		must make sure that your implementation of the initialiser
 *		copes with either an NSString or an NSAttributedString.
 *		If it receives an NSAttributedString, it should ignore the
 *		attributes argument and use the values from the string.
 */

#import "common.h"
#import "GNUstepBase/Unicode.h"

#import "Foundation/NSAttributedString.h"
#import "Foundation/NSData.h"
#import "Foundation/NSException.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSPortCoder.h"
#import "Foundation/NSRange.h"

@class	GSAttributedString;
@interface GSAttributedString : NSObject	// Help the compiler
@end
@class	GSMutableAttributedString;
@interface GSMutableAttributedString : NSObject	// Help the compiler
@end
@class	GSMutableDictionary;
@interface GSMutableDictionary : NSObject	// Help the compiler
@end
static Class	dictionaryClass = 0;

static SEL	eqSel;
static SEL	setSel;
static SEL	getSel;
static SEL	allocDictSel;
static SEL	initDictSel;
static SEL	addDictSel;
static SEL	setDictSel;
static SEL	relDictSel;
static SEL	remDictSel;

static IMP	allocDictImp;
static IMP	initDictImp;
static IMP	addDictImp;
static IMP	setDictImp;
static IMP	relDictImp;
static IMP	remDictImp;

@interface GSMutableAttributedStringTracker : NSMutableString
{
  NSMutableAttributedString	*_owner;
}
+ (NSMutableString*) stringWithOwner: (NSMutableAttributedString*)as;
@end

/**
 *  A string in which name-value pairs represented by an [NSDictionary] may
 *  be associated to ranges of characters.  Used for text rendering by the
 *  GUI/AppKit framework, in which fonts, sizes, etc. are stored under standard
 *  attributes in the dictionaries.
 *
 */
@implementation NSAttributedString

static Class NSAttributedStringClass;
static Class GSAttributedStringClass;
static Class NSMutableAttributedStringClass;
static Class GSMutableAttributedStringClass;

+ (void) initialize
{
  if (self == [NSAttributedString class])
    {
      NSAttributedStringClass = self;
      GSAttributedStringClass = [GSAttributedString class];
      NSMutableAttributedStringClass
	= [NSMutableAttributedString class];
      GSMutableAttributedStringClass
	= [GSMutableAttributedString class];
      dictionaryClass = [GSMutableDictionary class];

      eqSel = @selector(isEqual:);
      setSel = @selector(setAttributes:range:);
      getSel = @selector(attributesAtIndex:effectiveRange:);
      allocDictSel = @selector(allocWithZone:);
      initDictSel = @selector(initWithDictionary:);
      addDictSel = @selector(addEntriesFromDictionary:);
      setDictSel = @selector(setObject:forKey:);
      relDictSel = @selector(release);
      remDictSel = @selector(removeObjectForKey:);

      allocDictImp = [dictionaryClass methodForSelector: allocDictSel];
      initDictImp = [dictionaryClass instanceMethodForSelector: initDictSel];
      addDictImp = [dictionaryClass instanceMethodForSelector: addDictSel];
      setDictImp = [dictionaryClass instanceMethodForSelector: setDictSel];
      remDictImp = [dictionaryClass instanceMethodForSelector: remDictSel];
      relDictImp = [dictionaryClass instanceMethodForSelector: relDictSel];
    }
}

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSAttributedStringClass)
    return NSAllocateObject(GSAttributedStringClass, 0, z);
  else
    return NSAllocateObject(self, 0, z);
}

- (Class) classForCoder
{
  return NSAttributedStringClass;
}

static void
appendUIntData(NSMutableData *d, NSUInteger i)
{
  unsigned int aux = i;
  unsigned int len = 1;

  while (aux >= 128)
    {
      aux /= 128;
      len++;
    }

  {
    unsigned char *p, buf[len];

    p = buf;
    while (i >= 128)
      {
	*p++ = (i & 0x7f) + 128;
	i /= 128;
      }
    *p = i;

     [d appendBytes: buf length: len];
  }
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      NSUInteger length = [self length];

      [aCoder  encodeObject: [self string] forKey: @"NSString"];
      if (length > 0) 
	{
	  NSRange range;
	  NSDictionary	*attrs;

	  attrs = [self attributesAtIndex: 0 effectiveRange: &range];
	  if (range.length == length)
	    {
	      [aCoder encodeObject: attrs forKey: @"NSAttributes"];
	    }
	  else
	    {
	      NSUInteger i = 0;
	      NSUInteger pos = 0;
	      NSMutableArray *attrs = [NSMutableArray arrayWithCapacity: 1];
	      NSMutableData *info = [NSMutableData dataWithCapacity: 2];

	      while (pos < length)
		{
		  [attrs addObject: [self attributesAtIndex: pos
					     effectiveRange: &range]];
		  appendUIntData(info, range.length);
		  appendUIntData(info, i++);
		  pos = NSMaxRange(range);
		}
	      [aCoder encodeObject: [[attrs copy] autorelease]
			    forKey: @"NSAttributes"];
	      [aCoder encodeObject: [[info copy] autorelease]
			    forKey: @"NSAttributeInfo"];
	    }
	}
    }
  else
    {
      NSRange		r = NSMakeRange(0, 0);
      unsigned		index = NSMaxRange(r);
      unsigned		length = [self length];
      NSString		*string = [self string];
      NSDictionary	*attrs;

      [aCoder encodeObject: string];
      while (index < length)
	{
	  attrs = [self attributesAtIndex: index effectiveRange: &r];
	  index = NSMaxRange(r);
	  [aCoder encodeValueOfObjCType: @encode(unsigned) at: &index];
	  [aCoder encodeObject: attrs];
	}
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      NSString *string = [aDecoder decodeObjectForKey: @"NSString"];

      if (![aDecoder containsValueForKey: @"NSAttributeInfo"])
        {
          NSDictionary *attributes;

          attributes = [aDecoder decodeObjectForKey: @"NSAttributes"];
          self = [self initWithString: string attributes: attributes];
        }
      else
        {
          NSArray *attributes = [aDecoder decodeObjectForKey: @"NSAttributes"];
          NSData *info = [aDecoder decodeObjectForKey: @"NSAttributeInfo"];
          unsigned int pos = 0;
          const unsigned char *p = [info bytes];
          const unsigned char *end = p + [info length];
          NSMutableAttributedString *m = [[NSMutableAttributedString alloc] 
                                           initWithString: string 
                                               attributes: nil];

          while (p < end)
            {
              unsigned int idx;
              unsigned int len;
	      unsigned int shift;
              NSRange r;

	      len = shift = 0;
	      while (*p & 0x80)
		{
		  len += (*p++ - 128) << shift;
		  shift += 7;
		}
	      len += *p++ << shift;

	      idx = shift = 0;
	      while (*p & 0x80)
		{
		  idx += (*p++ - 128) << shift;
		  shift += 7;
		}
	      idx += *p++ << shift;

              r = NSMakeRange(pos, len);
	      [m setAttributes: [attributes objectAtIndex: idx] range: r];
              pos = NSMaxRange(r);
            }
          DESTROY(self);
          self = [m copy];
          RELEASE(m);
        }
    }
  else
    {
      NSString	*string = [aDecoder decodeObject];
      unsigned	length = [string length];

      if (length == 0)
        {
	  self = [self initWithString: string attributes: nil];
	}
      else
        {
	  unsigned	index;
	  NSDictionary	*attrs;
	
	  [aDecoder decodeValueOfObjCType: @encode(unsigned) at: &index];
	  attrs = [aDecoder decodeObject];
	  if (index == length)
	    {
	      self = [self initWithString: string attributes: attrs];
	    }
	  else
	    {
	      NSRange	r = NSMakeRange(0, index);
	      unsigned	last = index;
	      NSMutableAttributedString	*m;
	
	      m = [NSMutableAttributedString alloc];
	      m = [m initWithString: string attributes: nil];
	      [m setAttributes: attrs range: r];
	      while (index < length)
	        {
		  [aDecoder decodeValueOfObjCType: @encode(unsigned)
			 		       at: &index];
		  attrs = [aDecoder decodeObject];
		  r = NSMakeRange(last, index - last);
		  [m setAttributes: attrs range: r];
		  last = index;
		}
	      DESTROY(self);
	      self = [m copy];
	      RELEASE(m);
	    }
	}
    }

  return self;
}

- (id) replacementObjectForPortCoder: (NSPortCoder*)aCoder
{
  if ([aCoder isByref] == NO)
    return self;
  return [super replacementObjectForPortCoder: aCoder];
}

//NSCopying protocol
- (id) copyWithZone: (NSZone*)zone
{
  if ([self isKindOfClass: [NSMutableAttributedString class]]
    || NSShouldRetainWithZone(self, zone) == NO)
    return [[GSAttributedStringClass allocWithZone: zone]
      initWithAttributedString: self];
  else
    return RETAIN(self);
}

//NSMutableCopying protocol
- (id) mutableCopyWithZone: (NSZone*)zone
{
  return [[GSMutableAttributedStringClass allocWithZone: zone]
    initWithAttributedString: self];
}

//Creating an NSAttributedString
- (id) init
{
  return [self initWithString: @"" attributes: nil];
}

/**
 *  Initialize to aString with no attributes.
 */
- (id) initWithString: (NSString*)aString
{
  return [self initWithString: aString attributes: nil];
}

/**
 *  Initialize to copy of attributedString.
 */
- (id) initWithAttributedString: (NSAttributedString*)attributedString
{
  return [self initWithString: (NSString*)attributedString attributes: nil];
}

/**
 *  Initialize to aString with given attributes applying over full range of
 *  string.
 */
- (id) initWithString: (NSString*)aString attributes: (NSDictionary*)attributes
{
  //This is the designated initializer
  [self subclassResponsibility: _cmd];/* Primitive method! */
  return nil;
}

- (NSString*) description
{
  NSRange		r = NSMakeRange(0, 0);
  unsigned		index = NSMaxRange(r);
  unsigned		length = [self length];
  NSString		*string = [self string];
  NSDictionary		*attrs;
  NSMutableString	*desc;

  desc = [NSMutableString stringWithCapacity: length];
  while (index < length &&
    (attrs = [self attributesAtIndex: index effectiveRange: &r]) != nil)
    {
      index = NSMaxRange(r);
      [desc appendFormat: @"%@%@", [string substringWithRange: r], attrs];
    }
  return desc;
}

//Retrieving character information
/**
 *  Return length of the underlying string.
 */
- (NSUInteger) length
{
  return [[self string] length];
}

/**
 *  Return the underlying string, stripped of attributes.
 */
- (NSString*) string
{
  [self subclassResponsibility: _cmd];/* Primitive method! */
  return nil;
}

//Retrieving attribute information
/**
 *  Returns attributes and values at index, and, if effectiveRange
 *  is non-nil, this gets filled with a range over which these attributes
 *  and values still hold.  This may not be the maximum range, depending
 *  on the implementation.
 */
- (NSDictionary*) attributesAtIndex: (NSUInteger)index
		     effectiveRange: (NSRange*)aRange
{
  [self subclassResponsibility: _cmd];/* Primitive method! */
  return nil;
}

/**
 *  Returns attributes and values at index, and, if longestEffectiveRange
 *  is non-nil, this gets filled with the range over which the attribute-value
 *  set is the same as at index, clipped to rangeLimit.
 */
- (NSDictionary*) attributesAtIndex: (NSUInteger)index
	      longestEffectiveRange: (NSRange*)aRange
			    inRange: (NSRange)rangeLimit
{
  NSDictionary	*attrDictionary, *tmpDictionary;
  NSRange	tmpRange;
  IMP		getImp;

  if (NSMaxRange(rangeLimit) > [self length])
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method -attributesAtIndex:longestEffectiveRange:inRange: in class NSAttributedString"];
    }
  getImp = [self methodForSelector: getSel];
  attrDictionary = (*getImp)(self, getSel, index, aRange);
  if (aRange == 0)
    return attrDictionary;

  while (aRange->location > rangeLimit.location)
    {
      //Check extend range backwards
      tmpDictionary = (*getImp)(self, getSel, aRange->location-1, &tmpRange);
      if ([tmpDictionary isEqualToDictionary: attrDictionary])
	{
	  aRange->length = NSMaxRange(*aRange) - tmpRange.location;
	  aRange->location = tmpRange.location;
	}
      else
	{
	  break;
	}
    }
  while (NSMaxRange(*aRange) < NSMaxRange(rangeLimit))
    {
      //Check extend range forwards
      tmpDictionary = (*getImp)(self, getSel, NSMaxRange(*aRange), &tmpRange);
      if ([tmpDictionary isEqualToDictionary: attrDictionary])
	{
	  aRange->length = NSMaxRange(tmpRange) - aRange->location;
	}
      else
	{
	  break;
	}
    }
  *aRange = NSIntersectionRange(*aRange,rangeLimit);//Clip to rangeLimit
  return attrDictionary;
}

/**
 *  Returns value for given attribute at index, and, if effectiveRange is
 *  non-nil, this gets filled with a range over which this value holds.  This
 *  may not be the maximum range, depending on the implementation.
 */
- (id) attribute: (NSString*)attributeName
	 atIndex: (NSUInteger)index
  effectiveRange: (NSRange*)aRange
{
  NSDictionary *tmpDictionary;
  id attrValue;

  tmpDictionary = [self attributesAtIndex: index effectiveRange: aRange];

  if (attributeName == nil)
    {
      if (aRange != 0)
	{
	  *aRange = NSMakeRange(0,[self length]);
	  /*
	   * If attributeName is nil, then the attribute will not exist in the
	   * entire text - therefore aRange of the entire text must be correct
	   */
        }
      return nil;
    }
  attrValue = [tmpDictionary objectForKey: attributeName];
  return attrValue;
}

/**
 *  Returns value for given attribute at index, and, if longestEffectiveRange
 *  is non-nil, this gets filled with the range over which the attribute
 *  applies, clipped to rangeLimit.
 */
- (id) attribute: (NSString*)attributeName
	 atIndex: (NSUInteger)index
  longestEffectiveRange: (NSRange*)aRange
	 inRange: (NSRange)rangeLimit
{
  NSDictionary	*tmpDictionary;
  id		attrValue;
  id		tmpAttrValue;
  NSRange	tmpRange;
  BOOL		(*eImp)(id,SEL,id);
  IMP		getImp;

  if (NSMaxRange(rangeLimit) > [self length])
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method %@ in class %@",
        NSStringFromSelector(_cmd), NSStringFromClass([self class])];
    }

  if (attributeName == nil)
    return nil;

  attrValue = [self attribute: attributeName
		      atIndex: index
	       effectiveRange: aRange];

  if (aRange == 0)
    return attrValue;

  /*
   * If attrValue == nil then eImp will be zero
   */
  eImp = (BOOL(*)(id,SEL,id))[attrValue methodForSelector: eqSel];
  getImp = [self methodForSelector: getSel];

  while (aRange->location > rangeLimit.location)
    {
      //Check extend range backwards
      tmpDictionary = (*getImp)(self, getSel,  aRange->location-1, &tmpRange);
      tmpAttrValue = [tmpDictionary objectForKey: attributeName];
      if (tmpAttrValue == attrValue
	|| (eImp != 0 && (*eImp)(attrValue, eqSel, tmpAttrValue)))
	{
	  aRange->length = NSMaxRange(*aRange) - tmpRange.location;
	  aRange->location = tmpRange.location;
	}
      else
	{
	  break;
	}
    }
  while (NSMaxRange(*aRange) < NSMaxRange(rangeLimit))
    {
      //Check extend range forwards
      tmpDictionary = (*getImp)(self, getSel,  NSMaxRange(*aRange), &tmpRange);
      tmpAttrValue = [tmpDictionary objectForKey: attributeName];
      if (tmpAttrValue == attrValue
	|| (eImp != 0 && (*eImp)(attrValue, eqSel, tmpAttrValue)))
	{
	  aRange->length = NSMaxRange(tmpRange) - aRange->location;
	}
      else
	{
	  break;
	}
    }
  *aRange = NSIntersectionRange(*aRange,rangeLimit);//Clip to rangeLimit
  return attrValue;
}

//Comparing attributed strings
/**
 *  Returns whether all characters and attributes are equal between this
 *  string and otherString.
 */
- (BOOL) isEqualToAttributedString: (NSAttributedString*)otherString
{
  NSRange ownEffectiveRange,otherEffectiveRange;
  unsigned int length;
  NSDictionary *ownDictionary,*otherDictionary;
  BOOL result;

  if (!otherString)
    return NO;
  if (![[otherString string] isEqual: [self string]])
    return NO;

  length = [otherString length];
  if (length == 0)
    return YES;

  ownDictionary = [self attributesAtIndex: 0
			   effectiveRange: &ownEffectiveRange];
  otherDictionary = [otherString attributesAtIndex: 0
				    effectiveRange: &otherEffectiveRange];
  result = YES;

  while (YES)
    {
      if (NSIntersectionRange(ownEffectiveRange, otherEffectiveRange).length > 0
	&& ![ownDictionary isEqualToDictionary: otherDictionary])
	{
	  result = NO;
	  break;
	}
      if (NSMaxRange(ownEffectiveRange) < NSMaxRange(otherEffectiveRange))
	{
	  ownDictionary = [self attributesAtIndex: NSMaxRange(ownEffectiveRange)
				   effectiveRange: &ownEffectiveRange];
	}
      else
	{
	  if (NSMaxRange(otherEffectiveRange) >= length)
	    {
	      break;//End of strings
	    }
	  otherDictionary = [otherString
	    attributesAtIndex: NSMaxRange(otherEffectiveRange)
	    effectiveRange: &otherEffectiveRange];
	}
    }
  return result;
}

- (BOOL) isEqual: (id)anObject
{
  if (anObject == self)
    return YES;
  if ([anObject isKindOfClass: NSAttributedStringClass])
    return [self isEqualToAttributedString: anObject];
  return NO;
}


//Extracting a substring
/**
 *  Returns substring with attribute information.
 */
- (NSAttributedString*) attributedSubstringFromRange: (NSRange)aRange
{
  NSAttributedString	*newAttrString;
  NSString		*newSubstring;
  NSDictionary		*attrs;
  NSRange		range;
  unsigned		len = [self length];

  GS_RANGE_CHECK(aRange, len);

  newSubstring = [[self string] substringWithRange: aRange];

  attrs = [self attributesAtIndex: aRange.location effectiveRange: &range];
  range = NSIntersectionRange(range, aRange);
  if (NSEqualRanges(range, aRange) == YES)
    {
      newAttrString = [GSAttributedStringClass alloc];
      newAttrString = [newAttrString initWithString: newSubstring
					 attributes: attrs];
    }
  else
    {
      NSMutableAttributedString	*m;
      NSRange			rangeToSet = range;

      m = [GSMutableAttributedStringClass alloc];
      m = [m initWithString: newSubstring attributes: nil];
      rangeToSet.location = 0;
      [m setAttributes: attrs range: rangeToSet];
      while (NSMaxRange(range) < NSMaxRange(aRange))
	{
	  attrs = [self attributesAtIndex: NSMaxRange(range)
			   effectiveRange: &range];
	  rangeToSet = NSIntersectionRange(range, aRange);
	  rangeToSet.location -= aRange.location;
	  [m setAttributes: attrs range: rangeToSet];
	}
      newAttrString = [m copy];
      RELEASE(m);
    }

  IF_NO_GC(AUTORELEASE(newAttrString));
  return newAttrString;
}

@end //NSAttributedString

/**
 *  Mutable version of [NSAttributedString].
 */
@implementation NSMutableAttributedString

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSMutableAttributedStringClass)
    return NSAllocateObject(GSMutableAttributedStringClass, 0, z);
  else
    return NSAllocateObject(self, 0, z);
}

- (Class) classForCoder
{
  return NSMutableAttributedStringClass;
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      NSString *string = [aDecoder decodeObjectForKey: @"NSString"];

      if (![aDecoder containsValueForKey: @"NSAttributeInfo"])
        {
          NSDictionary *attributes;

          attributes = [aDecoder decodeObjectForKey: @"NSAttributes"];
          self = [self initWithString: string attributes: attributes];
        }
      else
        {
          NSArray *attributes = [aDecoder decodeObjectForKey: @"NSAttributes"];
          NSData *info = [aDecoder decodeObjectForKey: @"NSAttributeInfo"];
          unsigned int pos = 0;
          const unsigned char *p = [info bytes];
          const unsigned char *end = p + [info length];

	  self = [self initWithString: string attributes: nil];
          while (p < end)
            {
              unsigned int idx;
              unsigned int len;
	      unsigned int shift;
              NSRange r;

	      len = shift = 0;
	      while (*p & 0x80)
		{
		  len += (*p++ - 128) << shift;
		  shift += 7;
		}
	      len += *p++ << shift;

	      idx = shift = 0;
	      while (*p & 0x80)
		{
		  idx += (*p++ - 128) << shift;
		  shift += 7;
		}
	      idx += *p++ << shift;

              r = NSMakeRange(pos, len);
	      [self setAttributes: [attributes objectAtIndex: idx] range: r];
              pos = NSMaxRange(r);
            }
        }
    }
  else
    {
      NSString	*string = [aDecoder decodeObject];
      unsigned	length = [string length];

      if (length == 0)
	{
	  self = [self initWithString: string attributes: nil];
	}
      else
	{
	  unsigned	index;
	  NSDictionary	*attrs;

	  [aDecoder decodeValueOfObjCType: @encode(unsigned) at: &index];
	  attrs = [aDecoder decodeObject];
	  if (index == length)
	    {
	      self = [self initWithString: string attributes: attrs];
	    }
	  else
	    {
	      NSRange	r = NSMakeRange(0, index);
	      unsigned	last = index;
	      
	      self = [self initWithString: string attributes: nil];
	      [self setAttributes: attrs range: r];
	      while (index < length)
		{
		  [aDecoder decodeValueOfObjCType: @encode(unsigned)
					   at: &index];
		  attrs = [aDecoder decodeObject];
		  r = NSMakeRange(last, index - last);
		  [self setAttributes: attrs range: r];
		  last = index;
		}
	    }
	}
    }

  return self;
}

//Retrieving character information
/**
 *  Returns mutable version of the underlying string.
 */
- (NSMutableString*) mutableString
{
  return [GSMutableAttributedStringTracker stringWithOwner: self];
}

//Changing characters
/**
 *  Removes characters and attributes applying to them.
 */
- (void) deleteCharactersInRange: (NSRange)aRange
{
  [self replaceCharactersInRange: aRange withString: nil];
}

//Changing attributes
/**
 *  Sets attributes to apply over range, replacing any previous attributes.
 */
- (void) setAttributes: (NSDictionary*)attributes range: (NSRange)aRange
{
  [self subclassResponsibility: _cmd];// Primitive method!
}

/**
 *  Adds attribute applying to given range.
 */
- (void) addAttribute: (NSString*)name value: (id)value range: (NSRange)aRange
{
  NSRange		effectiveRange;
  NSDictionary		*attrDict;
  NSMutableDictionary	*newDict;
  unsigned int		tmpLength;
  IMP			getImp;

  tmpLength = [self length];
  GS_RANGE_CHECK(aRange, tmpLength);

  getImp = [self methodForSelector: getSel];
  attrDict = (*getImp)(self, getSel, aRange.location, &effectiveRange);

  if (effectiveRange.location < NSMaxRange(aRange))
    {
      IMP	setImp;

      setImp = [self methodForSelector: setSel];

      [self beginEditing];
      while (effectiveRange.location < NSMaxRange(aRange))
	{
	  effectiveRange = NSIntersectionRange(aRange, effectiveRange);
	
	  newDict = (*allocDictImp)(dictionaryClass, allocDictSel,
	    NSDefaultMallocZone());
	  newDict = (*initDictImp)(newDict, initDictSel, attrDict);
	  (*setDictImp)(newDict, setDictSel, value, name);
	  (*setImp)(self, setSel, newDict, effectiveRange);
	  IF_NO_GC((*relDictImp)(newDict, relDictSel));
	
	  if (NSMaxRange(effectiveRange) >= NSMaxRange(aRange))
	    {
	      effectiveRange.location = NSMaxRange(aRange);// stop the loop...
	    }
	  else if (NSMaxRange(effectiveRange) < tmpLength)
	    {
	      attrDict = (*getImp)(self, getSel, NSMaxRange(effectiveRange),
		&effectiveRange);
	    }
	}
      [self endEditing];
    }
}

/**
 *  Add attributes to apply over given range.
 */
- (void) addAttributes: (NSDictionary*)attributes range: (NSRange)aRange
{
  NSRange		effectiveRange;
  NSDictionary		*attrDict;
  NSMutableDictionary	*newDict;
  unsigned int		tmpLength;
  IMP			getImp;

  if (!attributes)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attributes is nil in method -addAttributes:range: "
			  @"in class NSMutableAtrributedString"];
    }
  tmpLength = [self length];
  if (NSMaxRange(aRange) > tmpLength)
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method -addAttribute:value:range: "
			  @"in class NSMutableAttributedString"];
    }

  getImp = [self methodForSelector: getSel];
  attrDict = (*getImp)(self, getSel, aRange.location, &effectiveRange);

  if (effectiveRange.location < NSMaxRange(aRange))
    {
      IMP	setImp;

      setImp = [self methodForSelector: setSel];

      [self beginEditing];
      while (effectiveRange.location < NSMaxRange(aRange))
	{
	  effectiveRange = NSIntersectionRange(aRange,effectiveRange);
	
	  newDict = (*allocDictImp)(dictionaryClass, allocDictSel,
	    NSDefaultMallocZone());
	  newDict = (*initDictImp)(newDict, initDictSel, attrDict);
	  (*addDictImp)(newDict, addDictSel, attributes);
	  (*setImp)(self, setSel, newDict, effectiveRange);
	  IF_NO_GC((*relDictImp)(newDict, relDictSel));
	
	  if (NSMaxRange(effectiveRange) >= NSMaxRange(aRange))
	    {
	      effectiveRange.location = NSMaxRange(aRange);// stop the loop...
	    }
	  else if (NSMaxRange(effectiveRange) < tmpLength)
	    {
	      attrDict = (*getImp)(self, getSel, NSMaxRange(effectiveRange),
		&effectiveRange);
	    }
	}
      [self endEditing];
    }
}

/**
 *  Removes given attribute from aRange.
 */
- (void) removeAttribute: (NSString*)name range: (NSRange)aRange
{
  NSRange		effectiveRange;
  NSDictionary		*attrDict;
  NSMutableDictionary	*newDict;
  unsigned int		tmpLength;
  IMP			getImp;

  tmpLength = [self length];
  GS_RANGE_CHECK(aRange, tmpLength);

  getImp = [self methodForSelector: getSel];
  attrDict = (*getImp)(self, getSel, aRange.location, &effectiveRange);

  if (effectiveRange.location < NSMaxRange(aRange))
    {
      IMP	setImp;

      setImp = [self methodForSelector: setSel];

      [self beginEditing];
      while (effectiveRange.location < NSMaxRange(aRange))
	{
	  effectiveRange = NSIntersectionRange(aRange,effectiveRange);
	
	  newDict = (*allocDictImp)(dictionaryClass, allocDictSel,
	    NSDefaultMallocZone());
	  newDict = (*initDictImp)(newDict, initDictSel, attrDict);
	  (*remDictImp)(newDict, remDictSel, name);
	  (*setImp)(self, setSel, newDict, effectiveRange);
	  IF_NO_GC((*relDictImp)(newDict, relDictSel));
	
	  if (NSMaxRange(effectiveRange) >= NSMaxRange(aRange))
	    {
	      effectiveRange.location = NSMaxRange(aRange);// stop the loop...
	    }
	  else if (NSMaxRange(effectiveRange) < tmpLength)
	    {
	      attrDict = (*getImp)(self, getSel, NSMaxRange(effectiveRange),
		&effectiveRange);
	    }
	}
      [self endEditing];
    }
}

//Changing characters and attributes
/**
 *  Appends attributed string to end of this one, preserving attributes.
 */
- (void) appendAttributedString: (NSAttributedString*)attributedString
{
  [self replaceCharactersInRange: NSMakeRange([self length],0)
	    withAttributedString: attributedString];
}

/**
 *  Inserts attributed string within this one, preserving attributes.
 */
- (void) insertAttributedString: (NSAttributedString*)attributedString
			atIndex: (NSUInteger)index
{
  [self replaceCharactersInRange: NSMakeRange(index,0)
	    withAttributedString: attributedString];
}

/**
 *  Replaces substring and attributes.
 */
- (void) replaceCharactersInRange: (NSRange)aRange
	     withAttributedString: (NSAttributedString*)attributedString
{
  NSDictionary	*attrDict;
  NSString	*tmpStr;
  unsigned	max;

  if (attributedString == nil)
    {
      [self replaceCharactersInRange: aRange withString: nil];
      return;
    }

  [self beginEditing];
  tmpStr = [attributedString string];
  [self replaceCharactersInRange: aRange withString: tmpStr];
  max = [tmpStr length];

  if (max > 0)
    {
      unsigned	loc = 0;
      NSRange	effectiveRange = NSMakeRange(0, loc);
      NSRange	clipRange = NSMakeRange(0, max);
      IMP	getImp;
      IMP	setImp;

      getImp = [attributedString methodForSelector: getSel];
      setImp = [self methodForSelector: setSel];
      while (loc < max)
	{
	  NSRange	ownRange;

	  attrDict = (*getImp)(attributedString, getSel, loc, &effectiveRange);
	  ownRange = NSIntersectionRange(clipRange, effectiveRange);
	  ownRange.location += aRange.location;
	  (*setImp)(self, setSel, attrDict, ownRange);
	  loc = NSMaxRange(effectiveRange);
	}
    }
  [self endEditing];
}

/** <override-subclass />
 *  Replaces substring; replacement is granted attributes equal to those of
 *  the first character of the portion replaced.
 */
- (void) replaceCharactersInRange: (NSRange)aRange
		       withString: (NSString*)aString
{
  [self subclassResponsibility: _cmd];// Primitive method!
}

/**
 *  Replaces entire contents (so this object can be reused).
 */
- (void) setAttributedString: (NSAttributedString*)attributedString
{
  [self replaceCharactersInRange: NSMakeRange(0,[self length])
	    withAttributedString: attributedString];
}

/** <override-dummy />
 *  Call before executing a collection of changes, for optimization.
 */
- (void) beginEditing
{
}

/** <override-dummy />
 *  Call after executing a collection of changes, for optimization.
 */
- (void) endEditing
{
}

@end //NSMutableAttributedString




/*
 * The GSMutableAttributedStringTracker class is a concrete subclass of
 * NSMutableString which keeps it's owner informed of any changes made
 * to it.
 */
@implementation GSMutableAttributedStringTracker

+ (NSMutableString*) stringWithOwner: (NSMutableAttributedString*)as
{
  GSMutableAttributedStringTracker	*str;
  NSZone	*z = NSDefaultMallocZone();

  str = (GSMutableAttributedStringTracker*) NSAllocateObject(self, 0, z);

  str->_owner = RETAIN(as);
  return AUTORELEASE(str);
}

- (void) dealloc
{
  RELEASE(_owner);
  [super dealloc];
}

- (NSUInteger) length
{
  return [[_owner string] length];
}

- (unichar) characterAtIndex: (NSUInteger)index
{
  return [[_owner string] characterAtIndex: index];
}

- (void)getCharacters: (unichar*)buffer
{
  [[_owner string] getCharacters: buffer];
}

- (void)getCharacters: (unichar*)buffer range: (NSRange)aRange
{
  [[_owner string] getCharacters: buffer range: aRange];
}

- (const char*) cString
{
  return [[_owner string] cString];
}

- (NSUInteger) cStringLength
{
  return [[_owner string] cStringLength];
}

- (NSStringEncoding) fastestEncoding
{
  return [[_owner string] fastestEncoding];
}

- (NSStringEncoding) smallestEncoding
{
  return [[_owner string] smallestEncoding];
}

- (int) _baseLength
{
  return [[_owner string] _baseLength];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [[_owner string] encodeWithCoder: aCoder];
}

- (Class) classForCoder
{
  return [[_owner string] classForCoder];
}

- (void) replaceCharactersInRange: (NSRange)aRange
		       withString: (NSString*)aString
{
  [_owner replaceCharactersInRange: aRange withString: aString];
}

@end

