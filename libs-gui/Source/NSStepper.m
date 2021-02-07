/** <title>NSStepper</title>

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author: Pierre-Yves Rivaille <pyrivail@ens-lyon.fr>
   Date: August 2001

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

#include "config.h"

#import "AppKit/NSStepper.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSStepperCell.h"

//
// class variables
//
id _nsstepperCellClass = nil;

@implementation NSStepper

//
// Class methods
//
+ (void) initialize
{
  if (self == [NSStepper class])
    {
      [self setVersion: 1];
      [self setCellClass: [NSStepperCell class]];
    }
}

//
// Initializing the NSStepper Factory
//
+ (Class) cellClass
{
  return _nsstepperCellClass;
}

+ (void) setCellClass: (Class)classId
{
  _nsstepperCellClass = classId;
}

//
// Instance methods
//

//
// Determining the first responder
//
- (BOOL) becomeFirstResponder
{
  [_cell setShowsFirstResponder: YES];
  [self setNeedsDisplay: YES];

  return YES;
}

- (BOOL) resignFirstResponder
{
  [_cell setShowsFirstResponder: NO];
  [self setNeedsDisplay: YES];

  return YES;
}

- (BOOL)acceptsFirstResponder
{
  // FIXME: change to `YES` after `keyDown:` implementation.
  return NO;
}

- (BOOL) acceptsFirstMouse: (NSEvent *)theEvent
{
  return YES;
}

- (void) keyDown: (NSEvent*)theEvent
{
  // FIXME
  [super keyDown: theEvent];
}

- (double) maxValue
{
  return [_cell maxValue];
}

- (void) setMaxValue: (double)maxValue
{
  [_cell setMaxValue: maxValue];
}

- (double) minValue
{
  return [_cell minValue];
}

- (void) setMinValue: (double)minValue
{
  [_cell setMinValue: minValue];
}

- (double) increment
{
  return [_cell increment];
}

- (void) setIncrement: (double)increment
{
  [_cell setIncrement: increment];
}

- (BOOL)autorepeat
{
  return [_cell autorepeat];
}

- (void)setAutorepeat: (BOOL)autorepeat
{
  [_cell setAutorepeat: autorepeat];
}

- (BOOL)valueWraps
{
  return [_cell valueWraps];
}

- (void)setValueWraps: (BOOL)valueWraps
{
  [_cell setValueWraps: valueWraps];
}

@end
