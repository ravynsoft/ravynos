/*
   NSSlider.h

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

#ifndef _GNUstep_H_NSSlider
#define _GNUstep_H_NSSlider

#import <AppKit/NSControl.h>
#import <AppKit/NSSliderCell.h>

@class NSString;
@class NSImage;
@class NSCell;
@class NSFont;
@class NSColor;
@class NSEvent;

@interface NSSlider : NSControl
// appearance 
- (double) altIncrementValue;
- (NSImage*) image;
- (NSInteger) isVertical;
- (CGFloat) knobThickness;
- (void) setAltIncrementValue: (double)increment;
- (void) setImage: (NSImage*)backgroundImage;
- (void) setKnobThickness: (CGFloat)aFloat;

// title
- (NSString*) title;
- (id) titleCell;
- (NSColor*) titleColor;
- (NSFont*) titleFont;
- (void) setTitle: (NSString*)aString;
- (void) setTitleCell: (NSCell*)aCell;
- (void) setTitleColor: (NSColor*)aColor;
- (void) setTitleFont: (NSFont*)fontObject;

// value limits 
- (double) maxValue;
- (double) minValue;
- (void) setMaxValue: (double)aDouble;
- (void) setMinValue: (double)aDouble;

// mouse handling
- (BOOL) acceptsFirstMouse: (NSEvent*)theEvent;

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

#endif // _GNUstep_H_NSSlider

