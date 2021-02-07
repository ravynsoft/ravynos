/* -*-objc-*-
   NSDatePicker.h

   The date picker class

   Copyright (C) 2020 Free Software Foundation, Inc.

   Created by Dr. H. Nikolaus Schaller on Sat Jan 07 2006.
   Copyright (c) 2005 DSITRI.

   Author:	Fabian Spillner
   Date:	22. October 2007

   Author:	Fabian Spillner <fabian.spillner@gmail.com>
   Date:	7. November 2007 - aligned with 10.5

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

#ifndef _GNUstep_H_NSDatePicker
#define _GNUstep_H_NSDatePicker

#import <AppKit/NSControl.h>
#import <AppKit/NSDatePickerCell.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

@interface NSDatePicker : NSControl

- (NSColor *) backgroundColor;
- (NSCalendar *) calendar;
- (NSDatePickerElementFlags) datePickerElements;
- (NSDatePickerMode) datePickerMode;
- (NSDatePickerStyle) datePickerStyle;
- (NSDate *) dateValue;
- (id) delegate;
- (BOOL) drawsBackground;
- (BOOL) isBezeled;
- (BOOL) isBordered;
- (NSLocale *) locale;
- (NSDate *) maxDate;
- (NSDate *) minDate;
- (void) setBackgroundColor:(NSColor *) color;
- (void) setBezeled:(BOOL) flag;
- (void) setBordered:(BOOL) flag;
- (void) setCalendar:(NSCalendar *) calendar;
- (void) setDatePickerElements:(NSDatePickerElementFlags) flags;
- (void) setDatePickerMode:(NSDatePickerMode) mode;
- (void) setDatePickerStyle:(NSDatePickerStyle) style;
- (void) setDateValue:(NSDate *) date;
- (void) setDelegate:(id) obj; /* DOESNT EXIST IN API */
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

#endif
#endif /* _GNUstep_H_NSDatePicker */
