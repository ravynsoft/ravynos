/* Implementation of extension methods to base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/
#import "common.h"
#include <ctype.h>

#import "Foundation/NSException.h"
#import "GNUstepBase/NSMutableString+GNUstepBase.h"

/* Test for ASCII whitespace which is safe for unicode characters */
#define	space(C)	((C) > 127 ? NO : isspace(C))

/* This private class is used for the -immutableProxy method in the category
 * on NSMutableString.
 * It is needed for [NSAttributedString-string] and [NSTextStorage-string]
 *
 * NB. The _parent instance variable is private but must *not* be changed
 * because it is actually used by GSMutableString code to permit some
 * optimisation (see GSString.m).
 */
@interface	GSImmutableString : NSString
{
  NSString	*_parent;	// Do not change this ivar declaration
}
- (id) initWithString: (NSString*)parent;
@end

@implementation GSImmutableString

- (BOOL) canBeConvertedToEncoding: (NSStringEncoding)enc
{
  return [_parent canBeConvertedToEncoding: enc];
}

- (unichar) characterAtIndex: (NSUInteger)index
{
  return [_parent characterAtIndex: index];
}

- (NSComparisonResult) compare: (NSString*)aString
		       options: (NSUInteger)mask
			 range: (NSRange)aRange
{
  return [_parent compare: aString options: mask range: aRange];
}

- (const char *) cString
{
  return [_parent cString];
}

- (const char *) cStringUsingEncoding: (NSStringEncoding)encoding
{
  return [_parent cStringUsingEncoding: encoding];
}

- (NSUInteger) cStringLength
{
  return [_parent cStringLength];
}

- (NSData*) dataUsingEncoding: (NSStringEncoding)encoding
	 allowLossyConversion: (BOOL)flag
{
  return [_parent dataUsingEncoding: encoding allowLossyConversion: flag];
}

- (void) dealloc
{
  RELEASE(_parent);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)z
{
  return [_parent copyWithZone: z];
}

- (id) mutableCopyWithZone: (NSZone*)z
{
  return [_parent mutableCopyWithZone: z];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [_parent encodeWithCoder: aCoder];
}

- (NSStringEncoding) fastestEncoding
{
  return [_parent fastestEncoding];
}

- (void) getCharacters: (unichar*)buffer
{
  [_parent getCharacters: buffer];
}

- (void) getCharacters: (unichar*)buffer range: (NSRange)aRange
{
  [_parent getCharacters: buffer range: aRange];
}

- (void) getCString: (char*)buffer
{
  [_parent getCString: buffer];
}

- (void) getCString: (char*)buffer
	  maxLength: (NSUInteger)maxLength
{
  [_parent getCString: buffer maxLength: maxLength];
}

- (BOOL) getCString: (char*)buffer
	  maxLength: (NSUInteger)maxLength
	   encoding: (NSStringEncoding)encoding
{
  return [_parent getCString: buffer maxLength: maxLength encoding: encoding];
}

- (void) getCString: (char*)buffer
	  maxLength: (NSUInteger)maxLength
	      range: (NSRange)aRange
     remainingRange: (NSRange*)leftoverRange
{
  [_parent getCString: buffer
	    maxLength: maxLength
		range: aRange
       remainingRange: leftoverRange];
}

- (NSUInteger) hash
{
  return [_parent hash];
}

- (id) initWithString: (NSString*)parent
{
  _parent = RETAIN(parent);
  return self;
}

- (BOOL) isEqual: (id)anObject
{
  return [_parent isEqual: anObject];
}

- (BOOL) isEqualToString: (NSString*)anObject
{
  return [_parent isEqualToString: anObject];
}

- (BOOL) isProxy
{
  return YES;
}

- (NSUInteger) length
{
  return [_parent length];
}

- (NSUInteger) lengthOfBytesUsingEncoding: (NSStringEncoding)encoding
{
  return [_parent lengthOfBytesUsingEncoding: encoding];
}

- (const char*) lossyCString
{
  return [_parent lossyCString];
}

- (NSUInteger) maximumLengthOfBytesUsingEncoding: (NSStringEncoding)encoding
{
  return [_parent maximumLengthOfBytesUsingEncoding: encoding];
}

- (NSRange) rangeOfComposedCharacterSequenceAtIndex: (NSUInteger)anIndex
{
  return [_parent rangeOfComposedCharacterSequenceAtIndex: anIndex];
}

- (NSRange) rangeOfCharacterFromSet: (NSCharacterSet*)aSet
			    options: (NSUInteger)mask
			      range: (NSRange)aRange
{
  return [_parent rangeOfCharacterFromSet: aSet options: mask range: aRange];
}

- (NSRange) rangeOfString: (NSString*)aString
		  options: (NSUInteger)mask
		    range: (NSRange)aRange
{
  return [_parent rangeOfString: aString options: mask range: aRange];
}

- (NSStringEncoding) smallestEncoding
{
  return [_parent smallestEncoding];
}

@end

/**
 * GNUstep specific (non-standard) additions to the NSMutableString class.
 */
@implementation NSMutableString (GNUstepBase)

/**
 * Removes the specified suffix from the string.  Raises an exception
 * if the suffix is not present.
 */
- (void) deleteSuffix: (NSString*)suffix
{
  NSCAssert2([self hasSuffix: suffix],
    @"'%@' does not have the suffix '%@'", self, suffix);
  [self deleteCharactersInRange:
    NSMakeRange([self length] - [suffix length], [suffix length])];
}

/**
 * Removes the specified prefix from the string.  Raises an exception
 * if the prefix is not present.
 */
- (void) deletePrefix: (NSString*)prefix
{
  NSCAssert2([self hasPrefix: prefix],
    @"'%@' does not have the prefix '%@'", self, prefix);
  [self deleteCharactersInRange: NSMakeRange(0, [prefix length])];
}

/**
 * Returns a proxy to the receiver which will allow access to the
 * receiver as an NSString, but which will not allow any of the
 * extra NSMutableString methods to be used.  You can use this method
 * to provide other code with read-only access to a mutable string
 * you own.
 */
- (NSString*) immutableProxy
{
  return AUTORELEASE([[GSImmutableString alloc] initWithString: self]);
}

/**
 * Replaces all occurrences of the string replace with the string by
 * in the receiver.<br />
 * Has no effect if replace does not occur within the
 * receiver.  NB. an empty string is not considered to exist within
 * the receiver.<br />
 * Calls - replaceOccurrencesOfString:withString:options:range: passing
 * zero for the options and a range from 0 with the length of the receiver.
 *
 * Note that is has to work for
 * [tmp replaceString: @"&amp;" withString: @"&amp;amp;"];
 */
- (void) replaceString: (NSString*)replace
	    withString: (NSString*)by
{
  NSRange       range;
  unsigned int  count = 0;
  unsigned int	newEnd;
  NSRange	searchRange;

  if (replace == nil)
    {
      [NSException raise: NSInvalidArgumentException
                  format: @"%@ nil search string", NSStringFromSelector(_cmd)];
    }
  if (by == nil)
    {
      [NSException raise: NSInvalidArgumentException
                  format: @"%@ nil replace string", NSStringFromSelector(_cmd)];
    }

  searchRange = NSMakeRange(0, [self length]);
  range = [self rangeOfString: replace options: 0 range: searchRange];

  if (range.length > 0)
    {
      unsigned  byLen = [by length];

      do
        {
          count++;
          [self replaceCharactersInRange: range
                              withString: by];

	  newEnd = NSMaxRange(searchRange) + byLen - range.length;
	  searchRange.location = range.location + byLen;
	  searchRange.length = newEnd - searchRange.location;

          range = [self rangeOfString: replace
                              options: 0
                                range: searchRange];
        }
      while (range.length > 0);
    }
}

/**
 * Removes all leading white space from the receiver.
 */
- (void) trimLeadSpaces
{
  unsigned	length = [self length];

  if (length > 0)
    {
      unsigned	start = 0;
      unichar	(*caiImp)(NSString*, SEL, NSUInteger);
      SEL caiSel = @selector(characterAtIndex:);

      caiImp = (unichar (*)())[self methodForSelector: caiSel];
      while (start < length && space((*caiImp)(self, caiSel, start)))
	{
	  start++;
	}
      if (start > 0)
	{
	  [self deleteCharactersInRange: NSMakeRange(0, start)];
	}
    }
}

/**
 * Removes all trailing white space from the receiver.
 */
- (void) trimTailSpaces
{
  unsigned	length = [self length];

  if (length > 0)
    {
      unsigned	end = length;
      unichar	(*caiImp)(NSString*, SEL, NSUInteger);
      SEL caiSel = @selector(characterAtIndex:);

      caiImp = (unichar (*)())[self methodForSelector: caiSel];
      while (end > 0 && space((*caiImp)(self, caiSel, end - 1)))
	{
	  end--;
	}
      if (end < length)
	{
	  [self deleteCharactersInRange: NSMakeRange(end, length - end)];
	}
    }
}

/**
 * Removes all leading or trailing white space from the receiver.
 */
- (void) trimSpaces
{
  [self trimTailSpaces];
  [self trimLeadSpaces];
}

@end

