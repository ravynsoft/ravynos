/** <title>NSDatePickerCell</title>

   <abstract>The date picker cell class</abstract>

   Copyright (C) 2020 Free Software Foundation, Inc.

   Author:  Nikolaus Schaller <hns@computer.org>
   Date:    April 2006

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date:   January 2020

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#import <Foundation/NSString.h>
#import <Foundation/NSDateFormatter.h>
#import "AppKit/NSDatePickerCell.h"
#import "AppKit/NSColor.h"

@implementation NSDatePickerCell

- (id) initTextCell: (NSString*)aString
{
  if ((self = [super  initTextCell: aString]))
    {
      NSDateFormatter *formatter = [[NSDateFormatter alloc] init];

      [self setFormatter: formatter];
      RELEASE(formatter);
    }

  return self;
}

- (NSColor *) backgroundColor
{
  return _backgroundColor;
}

- (void) setBackgroundColor: (NSColor *)color
{
  ASSIGN(_backgroundColor, color);
}

- (NSCalendar *) calendar
{
  return [[self formatter] calendar];
}

- (void) setCalendar: (NSCalendar *)calendar
{
  [[self formatter] setCalendar: calendar];
}

- (NSDatePickerElementFlags) datePickerElements
{
  return _datePickerElements;
}

- (void) setDatePickerElements: (NSDatePickerElementFlags)flags
{
  _datePickerElements = flags;
}

- (NSDatePickerMode) datePickerMode
{
  return _datePickerMode;
}

- (void) setDatePickerMode: (NSDatePickerMode)mode
{
  _datePickerMode = mode;
}

- (NSDatePickerStyle) datePickerStyle
{
  return _datePickerStyle;
}

- (void) setDatePickerStyle: (NSDatePickerStyle)style
{
  _datePickerStyle = style;
}

- (NSDate *) dateValue
{
  return (NSDate *)[self objectValue];
}

- (void) setDateValue: (NSDate *)date
{
  [self setObjectValue: date];
}

- (id) delegate
{
  return _delegate;
}

- (void) setDelegate: (id)obj
{
  _delegate = obj;
}

- (BOOL) drawsBackground
{
  return _drawsBackground;
}

- (void) setDrawsBackground: (BOOL)flag
{
  _drawsBackground = flag;
}

- (NSLocale *) locale
{
  return [[self formatter] locale];
}

- (void) setLocale: (NSLocale *)locale
{
  [[self formatter] setLocale: locale];
}

- (NSDate *) maxDate
{
  return _maxDate;
}

- (void) setMaxDate: (NSDate *)date
{
  ASSIGN(_maxDate, date);
}

- (NSDate *) minDate
{
  return _minDate;
}

- (void) setMinDate: (NSDate *)date
{
  ASSIGN(_minDate, date);
}

- (NSColor *) textColor
{
  return _textColor;
}

- (void) setTextColor: (NSColor *)color
{
  ASSIGN(_textColor, color);
}

- (NSTimeInterval) timeInterval
{
  return _timeInterval;
}

- (void) setTimeInterval: (NSTimeInterval)interval
{
  _timeInterval = interval;
}

- (NSTimeZone *) timeZone
{
  return [[self formatter] timeZone];
}

- (void) setTimeZone: (NSTimeZone *)zone
{
  [[self formatter] setTimeZone: zone];
}

- (void) encodeWithCoder: (NSCoder *)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeDouble: [self timeInterval] forKey: @"NSTimeInterval"];
      [aCoder encodeInt: [self datePickerElements] forKey: @"NSDatePickerElements"];
      [aCoder encodeInt: [self datePickerStyle] forKey: @"NSDatePickerType"];
      [aCoder encodeObject: [self backgroundColor] forKey: @"NSBackgroundColor"];
    }
  else
    {
    }
}

- (id) initWithCoder: (NSCoder *)aDecoder
{
  if ((self = [super initWithCoder: aDecoder]))
    {
      if ([aDecoder allowsKeyedCoding])
        {
          [self setTimeInterval: [aDecoder decodeDoubleForKey: @"NSTimeInterval"]];
          [self setDatePickerElements: [aDecoder decodeIntForKey: @"NSDatePickerElements"]];
          [self setDatePickerStyle: [aDecoder decodeIntForKey: @"NSDatePickerType"]];
          [self setBackgroundColor: [aDecoder decodeObjectForKey: @"NSBackgroundColor"]];
        }
      else
        {
        }
    }

  return self;
}

@end
