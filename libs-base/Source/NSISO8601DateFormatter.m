/* Implementation of class NSISO8601DateFormatter
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
   Date: Tue Oct 29 04:43:13 EDT 2019

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

#import "Foundation/NSCoder.h"
#import "Foundation/NSDateFormatter.h"
#import "Foundation/NSISO8601DateFormatter.h"
#import "Foundation/NSString.h"
#import "Foundation/NSTimeZone.h"

@implementation NSISO8601DateFormatter

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      _formatter = [[NSDateFormatter alloc] init];
      _timeZone = RETAIN([NSTimeZone localTimeZone]);
      _formatOptions = NSISO8601DateFormatWithInternetDateTime;
    }
  return self;
}

- (oneway void) dealloc
{
  RELEASE(_formatter);
  RELEASE(_timeZone);
  [super dealloc];
}
  
- (NSTimeZone *) timeZone
{
  return _timeZone;
}

- (void) setTimeZone: (NSTimeZone *)tz
{
  _timeZone = tz;
}

- (NSISO8601DateFormatOptions) formatOptions
{
  return _formatOptions;
}

- (NSString *) _buildFormatWithOptions
{
  NSString *result = @"";

  // Build date...
  if (_formatOptions & NSISO8601DateFormatWithYear)
    {
      result = [result stringByAppendingString: @"yyyy"];
    }
  if (_formatOptions & NSISO8601DateFormatWithDashSeparatorInDate &&
      _formatOptions & NSISO8601DateFormatWithMonth)
    {
      result = [result stringByAppendingString: @"-"];
    }
  if (_formatOptions & NSISO8601DateFormatWithMonth)
    {
      result = [result stringByAppendingString: @"MM"];
    }
  if (_formatOptions & NSISO8601DateFormatWithDashSeparatorInDate &&
      _formatOptions & NSISO8601DateFormatWithDay)
    {
      result = [result stringByAppendingString: @"-"];
    }
  if (_formatOptions & NSISO8601DateFormatWithDay)
    {
      result = [result stringByAppendingString: @"dd"];
    }
  
  // Build time...
  if (_formatOptions & NSISO8601DateFormatWithSpaceBetweenDateAndTime &&
      _formatOptions & NSISO8601DateFormatWithTime)
    {
      result = [result stringByAppendingString: @" "];
    }
  else
    {
      // Add T in format if we have a time component...
      result = [result stringByAppendingString: @"'T'"];
    }
  if (_formatOptions & NSISO8601DateFormatWithTime)
    {
      if (_formatOptions & NSISO8601DateFormatWithColonSeparatorInTime)
        {
          result = [result stringByAppendingString: @"HH:mm:ss"];
        }
      else
        {
          result = [result stringByAppendingString: @"HHmmss"];
        }
    }
  if (_formatOptions & NSISO8601DateFormatWithFractionalSeconds)
    {
      result = [result stringByAppendingString: @".SSSSSS"];
    }
  if (_formatOptions & NSISO8601DateFormatWithTimeZone)
    {
      if (_formatOptions & NSISO8601DateFormatWithColonSeparatorInTimeZone)
        {
          result = [result stringByAppendingString: @"ZZ:ZZ"];
        }
      else
        {
          result = [result stringByAppendingString: @"ZZZZ"];
        }
    }
  
  return result;
}

- (void) setFormatOptions: (NSISO8601DateFormatOptions)options
{
  _formatOptions = options;
}
  
- (NSString *) stringFromDate: (NSDate *)date
{
  NSString *formatString = [self _buildFormatWithOptions];
  [_formatter setTimeZone: _timeZone];
  [_formatter setDateFormat: formatString];
  return [_formatter stringFromDate: date];
}

- (NSDate *) dateFromString: (NSString *)string
{
  NSString *formatString = [self _buildFormatWithOptions];
  [_formatter setTimeZone: _timeZone];
  [_formatter setDateFormat: formatString];
  return [_formatter dateFromString: string];
}

+ (NSString *) stringFromDate: (NSDate *)date
                     timeZone: (NSTimeZone *)timeZone
                formatOptions: (NSISO8601DateFormatOptions)formatOptions
{
  NSISO8601DateFormatter *formatter = [[NSISO8601DateFormatter alloc] init];
  AUTORELEASE(formatter);
  [formatter setTimeZone: timeZone];
  [formatter setFormatOptions: formatOptions];
  return [formatter stringFromDate: date];
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: _timeZone forKey: @"NS.timeZone"];
      [coder encodeInteger: _formatOptions forKey: @"NS.formatOptions"];
    }
  else
    {
      [coder encodeObject: _timeZone];
      [coder encodeValueOfObjCType: @encode(NSUInteger) at: &_formatOptions];
    }
}

- (id) initWithCoder: (NSCoder *)decoder
{
  if ((self = [super init]) != nil)
    {
      if ([decoder allowsKeyedCoding])
        {
          ASSIGN(_timeZone, [decoder decodeObjectForKey: @"NS.timeZone"]);
          _formatOptions = [decoder decodeIntegerForKey: @"NS.formatOptions"];
        }
      else
        {
          ASSIGN(_timeZone, [decoder decodeObject]);
          [decoder decodeValueOfObjCType: @encode(NSUInteger) at: &_formatOptions];
        }
    }
  return self;
}
@end

