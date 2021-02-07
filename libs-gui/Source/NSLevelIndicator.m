/* 
   NSLevelIndicator.m

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

#import "AppKit/NSLevelIndicator.h"

static Class cellClass;

@implementation NSLevelIndicator

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSLevelIndicator class])
    {
      // Initial version
      [self setVersion: 1];

      cellClass = [NSLevelIndicatorCell class];
    }
}
 
/**<p> Returns the cell Class used by NSLevelIndicator. Used by subclasses.</p>
   <p>See Also: +setCellClass:</p>    
 */
+ (Class) cellClass
{
  return cellClass;
}

/**<p> Sets the cell Class used by NSLevelIndicator to <var>factoryId</var>.
   Used by subclasses.</p> <p>See Also: +setCellClass:</p>    
 */
+ (void) setCellClass: (Class)class
{
  cellClass = class;
}

- (double) maxValue
{
  return [_cell maxValue];
}

- (void) setMaxValue: (double) val
{
  [_cell setMaxValue:val];
}

- (double) minValue
{
  return [_cell minValue];
}

- (void) setMinValue: (double)val
{
  [_cell setMinValue: val];
}

- (double) warningValue
{
  return [_cell warningValue];
}

- (void) setWarningValue: (double)val
{
  [_cell setWarningValue: val];
}

- (double) criticalValue
{
  return [_cell criticalValue];
}

- (void) setCriticalValue: (double)val
{
  [_cell setCriticalValue: val];
}

- (NSInteger) numberOfMajorTickMarks
{
  return [_cell numberOfMajorTickMarks];
}

- (void) setNumberOfMajorTickMarks: (NSInteger)count
{
  [_cell setNumberOfMajorTickMarks: count];
}

- (NSInteger) numberOfTickMarks
{
  return [_cell numberOfTickMarks];
}

- (void) setNumberOfTickMarks: (NSInteger)count
{
  [_cell setNumberOfTickMarks: count];
}

- (NSTickMarkPosition) tickMarkPosition
{
  return [_cell tickMarkPosition];
}

- (void) setTickMarkPosition: (NSTickMarkPosition)pos
{
  [_cell setTickMarkPosition:pos];
}

- (double) tickMarkValueAtIndex: (NSInteger)index
{
  return [_cell tickMarkValueAtIndex:index];
}

- (NSRect) rectOfTickMarkAtIndex: (NSInteger)index
{
  return [_cell rectOfTickMarkAtIndex:index];
}

@end
