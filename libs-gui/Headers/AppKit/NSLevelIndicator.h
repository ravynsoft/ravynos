/* -*-objc-*-
   NSLevelIndicator.h

   The level indicator class

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

#ifndef _GNUstep_H_NSLevelIndicator
#define _GNUstep_H_NSLevelIndicator

#import "AppKit/NSControl.h"
// For the tick mark 
#import "AppKit/NSLevelIndicatorCell.h"

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

@interface NSLevelIndicator : NSControl
{
}

- (double) maxValue;
- (void) setMaxValue: (double)val;
- (double) minValue;
- (void) setMinValue: (double)val;
- (double) warningValue;
- (void) setWarningValue: (double)val;
- (double) criticalValue;
- (void) setCriticalValue: (double) val;

- (NSInteger) numberOfMajorTickMarks;
- (NSInteger) numberOfTickMarks;
- (NSRect) rectOfTickMarkAtIndex: (NSInteger)index;
- (void) setNumberOfMajorTickMarks: (NSInteger)count;
- (void) setNumberOfTickMarks: (NSInteger)count;
- (void) setTickMarkPosition: (NSTickMarkPosition)pos;
- (NSTickMarkPosition) tickMarkPosition;
- (double) tickMarkValueAtIndex: (NSInteger)index;

@end

#endif /* MAC_OS_X_VERSION_10_4 */
#endif /* _GNUstep_H_NSLevelIndicator */
