/* -*-objc-*-
   NSLevelIndicatorCell.h

   The level indicator cell class

   Copyright (C) 2007 Free Software Foundation, Inc.

   Author:  H. Nikolaus Schaller
   Date: 2006
   
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

#ifndef _GNUstep_H_NSLevelIndicatorCell
#define _GNUstep_H_NSLevelIndicatorCell

#import "AppKit/NSActionCell.h"
// For the tick mark 
#import "AppKit/NSSliderCell.h"

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

typedef enum _NSLevelIndicatorStyle
{
  NSRelevancyLevelIndicatorStyle,
  NSContinuousCapacityLevelIndicatorStyle,
  NSDiscreteCapacityLevelIndicatorStyle,
  NSRatingLevelIndicatorStyle
} NSLevelIndicatorStyle;

@interface NSLevelIndicatorCell : NSActionCell
{
  double _minValue;
  double _maxValue;
  double _warningValue;
  double _criticalValue;
  NSInteger _numberOfMajorTickMarks;
  NSInteger _numberOfTickMarks;
  NSLevelIndicatorStyle _style;
  NSTickMarkPosition _tickMarkPosition;
  NSRect _cellFrame;
}

- (id)initWithLevelIndicatorStyle:(NSLevelIndicatorStyle)style;

// value handling
- (double)criticalValue;
- (void)setCriticalValue:(double)val;
- (double)maxValue;
- (void)setMaxValue:(double)val;
- (double)minValue;
- (void)setMinValue:(double)val;
- (void)setWarningValue:(double)val;
- (double)warningValue;


- (NSLevelIndicatorStyle)style;
- (void)setLevelIndicatorStyle:(NSLevelIndicatorStyle)style;
- (NSInteger)numberOfMajorTickMarks;
- (void)setNumberOfMajorTickMarks:(NSInteger)count;
- (NSInteger)numberOfTickMarks;
- (void)setNumberOfTickMarks:(NSInteger)count;
- (NSTickMarkPosition)tickMarkPosition;
- (void)setTickMarkPosition:(NSTickMarkPosition) pos;
- (double)tickMarkValueAtIndex:(NSInteger)index;
- (NSRect)rectOfTickMarkAtIndex:(NSInteger)index;

@end

#endif /* MAC_OS_X_VERSION_10_4 */
#endif /* _GNUstep_H_NSLevelIndicatorCell */
