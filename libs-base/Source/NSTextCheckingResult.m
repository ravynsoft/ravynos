/* Copyright (C) 2011 Free Software Foundation, Inc.
   
   This file is part of the GNUstep Library.
   
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
#import "Foundation/NSException.h"
#import "Foundation/NSTextCheckingResult.h"
#import "Foundation/NSRegularExpression.h"

/**
 * Private class encapsulating a regular expression match.
 */
@interface GSRegularExpressionCheckingResult : NSTextCheckingResult
{
  // TODO: This could be made more efficient by adding a variant that only
  // contained a single range.
@public
  /** The number of ranges matched */
  NSUInteger rangeCount;
  /** The array of ranges. */
  NSRange *ranges;
  /** The regular expression object that generated this match. */
  NSRegularExpression *regularExpression;
}
@end

@implementation NSTextCheckingResult
+ (NSTextCheckingResult*)
  regularExpressionCheckingResultWithRanges: (NSRangePointer)ranges
  count: (NSUInteger)count
  regularExpression: (NSRegularExpression*)regularExpression
{
  GSRegularExpressionCheckingResult *result =
    [GSRegularExpressionCheckingResult new];

  result->rangeCount = count;
  result->ranges = calloc(sizeof(NSRange), count);
  memcpy(result->ranges, ranges, (sizeof(NSRange) * count));
  ASSIGN(result->regularExpression, regularExpression);
  return [result autorelease];
}

- (NSDictionary*) addressComponents
{
  return nil;
}

- (NSDictionary*) components
{
  return nil;
}

- (NSDate*) date
{
  return nil;
}

- (NSTimeInterval) duration
{
  return 0;
}

- (NSArray*) grammarDetails
{
  return 0;
}

- (NSUInteger) numberOfRanges
{
  return 1;
}

- (NSOrthography*) orthography
{
  return nil;
}

- (NSString*) phoneNumber
{
  return nil;
}

- (NSRange) range
{
  [self subclassResponsibility: _cmd];
  return NSMakeRange(NSNotFound, 0);
}

- (NSRegularExpression*) regularExpression
{
  return nil;
}

- (NSString*) replacementString
{
  return nil;
}

- (NSTextCheckingType) resultType
{
  [self subclassResponsibility: _cmd];
  return -1;
}

- (NSTimeZone*) timeZone
{
  return nil;
}

- (NSURL*) URL
{
  return nil;
}


- (NSRange) rangeAtIndex: (NSUInteger)idx
{
  if (idx >= [self numberOfRanges])
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"index %"PRIuPTR" out of range", idx];
    }
  return [self range];
}

- (NSTextCheckingResult *) resultByAdjustingRangesWithOffset: (NSInteger)offset
{
  [self subclassResponsibility: _cmd];
  return nil;
}

@end



@implementation GSRegularExpressionCheckingResult

- (void) dealloc
{
  [regularExpression release];
  free(ranges);
  [super dealloc];
}

- (NSUInteger) numberOfRanges
{
  return rangeCount;
}

- (NSRange) range
{
  return ranges[0];
}

- (NSRange) rangeAtIndex: (NSUInteger)idx
{
  if (idx >= rangeCount)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"index %"PRIuPTR" out of range", idx];
    }
  return ranges[idx];
}

- (NSRegularExpression*) regularExpression
{
  return regularExpression;
}

- (NSTextCheckingType) resultType
{
  return NSTextCheckingTypeRegularExpression;
}

- (NSTextCheckingResult*) resultByAdjustingRangesWithOffset: (NSInteger)offset
{
  NSUInteger i;
  GSRegularExpressionCheckingResult *result =
    [[GSRegularExpressionCheckingResult new] autorelease];

  result->rangeCount = rangeCount;
  result->ranges = calloc(sizeof(NSRange), rangeCount);
  for (i = 0; i < rangeCount; i++)
    {
      NSRange r = ranges[i];

      if ((offset > 0 && NSNotFound - r.location <= offset)
        || (offset < 0 && r.location < -offset))
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"Invalid offset %"PRIdPTR" for range: %@",
		       offset, NSStringFromRange(r)];
	}
      r.location += offset;
      result->ranges[i] = r;
    }
  ASSIGN(result->regularExpression, regularExpression);
  return result;
}

@end
