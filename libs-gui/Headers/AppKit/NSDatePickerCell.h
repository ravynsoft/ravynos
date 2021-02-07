/* -*-objc-*-
   NSDatePickerCell.h

   The date picker cell class

   Copyright (C) 2020 Free Software Foundation, Inc.

   Created by Dr. H. Nikolaus Schaller on Sat Jan 07 2006.

   Author:	Fabian Spillner
   Date:	22. October 2007

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _GNUstep_H_NSDatePickerCell
#define _GNUstep_H_NSDatePickerCell

#import <AppKit/NSActionCell.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

enum {
      NSTextFieldAndStepperDatePickerStyle    = 0,
      NSClockAndCalendarDatePickerStyle       = 1,
      NSTextFieldDatePickerStyle              = 2
};
typedef NSUInteger NSDatePickerStyle;

enum {
      NSSingleDateMode = 0,
      NSRangeDateMode = 1
};
typedef NSUInteger NSDatePickerMode;

enum {
      NSHourMinuteDatePickerElementFlag       = 0x000c,
      NSHourMinuteSecondDatePickerElementFlag = 0x000e,
      NSTimeZoneDatePickerElementFlag         = 0x0010,

      NSYearMonthDatePickerElementFlag        = 0x00c0,
      NSYearMonthDayDatePickerElementFlag     = 0x00e0,
      NSEraDatePickerElementFlag              = 0x0100,
};
typedef NSUInteger NSDatePickerElementFlags;

@class NSColor, NSDate, NSCalendar, NSLocale, NSTimeZone;

@interface NSDatePickerCell : NSActionCell
{
  NSColor *_backgroundColor;
  NSColor *_textColor;
  NSDate *_maxDate;
  NSDate *_minDate;
  id _delegate;
  NSTimeInterval _timeInterval;
  // FIXME: pack into a bitfield?
  NSDatePickerElementFlags _datePickerElements;
  NSDatePickerMode _datePickerMode;
  NSDatePickerStyle _datePickerStyle;
  BOOL _drawsBackground;
}

- (NSColor *) backgroundColor;
- (NSCalendar *) calendar;
- (NSDatePickerElementFlags) datePickerElements;
- (NSDatePickerMode) datePickerMode;
- (NSDatePickerStyle) datePickerStyle;
- (NSDate *) dateValue;
- (id) delegate;
- (BOOL) drawsBackground;
- (NSLocale *) locale;
- (NSDate *) maxDate;
- (NSDate *) minDate;
- (void) setBackgroundColor:(NSColor *) color;
- (void) setCalendar:(NSCalendar *) calendar;
- (void) setDatePickerElements:(NSDatePickerElementFlags) flags;
- (void) setDatePickerMode:(NSDatePickerMode) mode;
- (void) setDatePickerStyle:(NSDatePickerStyle) style;
- (void) setDateValue:(NSDate *) date;
- (void) setDelegate:(id) obj;
- (void) setDrawsBackground:(BOOL) flag;
- (void) setLocale:(NSLocale *) locale;
- (void) setMaxDate:(NSDate *) date;
- (void) setMinDate:(NSDate *) date;
- (void) setTextColor:(NSColor *) color;
- (void) setTimeInterval:(NSTimeInterval) interval;
- (void) setTimeZone:(NSTimeZone *) zone;
- (NSColor *) textColor;
- (NSTimeInterval) timeInterval;
- (NSTimeZone *) timeZone;

@end

@interface NSObject (NSDataPickerCellDelegate)

- (void)datePickerCell:(NSDatePickerCell *) aDatePickerCell
validateProposedDateValue:(NSDate **) proposedDateValue
		  timeInterval:(NSTimeInterval *) proposedTimeInterval;

@end

#endif
#endif /* _GNUstep_H_NSDatePickerCell */
