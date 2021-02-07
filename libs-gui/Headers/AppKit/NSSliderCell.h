/*
   NSSliderCell.h

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: September 1997
   
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

#ifndef _GNUstep_H_NSSliderCell
#define _GNUstep_H_NSSliderCell

#import <AppKit/NSActionCell.h>

@class NSString;
@class NSColor;
@class NSFont;
@class NSImage;

typedef enum _NSTickMarkPosition
{
    NSTickMarkBelow = 0,
    NSTickMarkAbove,
    NSTickMarkLeft = 0,
    NSTickMarkRight
} NSTickMarkPosition;

typedef enum _NSSliderType
{
    NSLinearSlider = 0,
    NSCircularSlider
} NSSliderType;

@interface NSSliderCell : NSActionCell <NSCoding>
{
  double	_value;
  double	_minValue;
  double	_maxValue;
  double	_altIncrementValue;
  id		_titleCell;
  id		_knobCell;
  NSRect	_trackRect;
  BOOL		_isVertical;
  BOOL          _allowsTickMarkValuesOnly;
  NSInteger     _numberOfTickMarks;
  NSTickMarkPosition _tickMarkPosition;
  NSSliderType		_type;
}

/* Asking about the cell's behavior */
- (double) altIncrementValue;
+ (BOOL) prefersTrackingUntilMouseUp;
- (NSRect) trackRect;

/* Changing the cell's behavior */
- (void) setAltIncrementValue: (double)increment;

/* Displaying the cell */
- (NSRect) knobRectFlipped: (BOOL)flipped;
- (void) drawBarInside: (NSRect)rect flipped: (BOOL)flipped;
- (void) drawKnob;
- (void) drawKnob: (NSRect)knobRect;

/* Asking about the cell's appearance */
- (CGFloat) knobThickness;
- (NSInteger) isVertical;
- (NSString*) title;
- (id) titleCell;
- (NSColor*) titleColor;
- (NSFont*) titleFont;
- (NSSliderType) sliderType;

/* Changing the cell's appearance */
- (void) setKnobThickness: (CGFloat)thickness;
- (void) setTitle: (NSString*)title;
- (void) setTitleCell: (NSCell*)aCell;
- (void) setTitleColor: (NSColor*)color;
- (void) setTitleFont: (NSFont*)font;
- (void) setSliderType:(NSSliderType)type;

/* Asking about the value limits */
- (double) minValue;
- (double) maxValue;

/* Changing the value limits */
- (void) setMinValue: (double)aDouble;
- (void) setMaxValue: (double)aDouble;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
// ticks
- (BOOL) allowsTickMarkValuesOnly;
- (double) closestTickMarkValueToValue: (double)aValue;
- (NSInteger) indexOfTickMarkAtPoint: (NSPoint)point;
- (NSInteger) numberOfTickMarks;
- (NSRect) rectOfTickMarkAtIndex: (NSInteger)index;
- (void) setAllowsTickMarkValuesOnly: (BOOL)flag;
- (void) setNumberOfTickMarks: (NSInteger)numberOfTickMarks;
- (void) setTickMarkPosition: (NSTickMarkPosition)position;
- (NSTickMarkPosition) tickMarkPosition;
- (double) tickMarkValueAtIndex: (NSInteger)index;
#endif

@end

#endif /* _GNUstep_H_NSSliderCell */
