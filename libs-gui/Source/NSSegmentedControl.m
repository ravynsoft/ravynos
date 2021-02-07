/* NSSegmentedControl.m
 *
 * Copyright (C) 2007 Free Software Foundation, Inc.
 *
 * Author:	Gregory John Casamento <greg_casamento@yahoo.com>
 * Date:	2007
 * 
 * This file is part of GNUstep.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 * 
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110 
 * USA.
 */

#import "AppKit/NSControl.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSSegmentedControl.h"
#import "AppKit/NSSegmentedCell.h"

static Class segmentedControlCellClass;

@implementation NSSegmentedControl 

+ (void) initialize
{
  if(self == [NSSegmentedControl class])
    {
      [self setVersion: 1];
      segmentedControlCellClass = [NSSegmentedCell class];
    }
}

+ (Class) cellClass
{
  return segmentedControlCellClass;
}

// Specifying number of segments...
- (void) setSegmentCount: (NSInteger) count
{
  [_cell setSegmentCount: count];
}

- (NSInteger) segmentCount
{
  return [_cell segmentCount];
} 

// Specifying selected segment...
- (void) setSelectedSegment: (NSInteger) segment
{
  [_cell setSelectedSegment: segment];
}

- (NSInteger) selectedSegment
{
  return [_cell selectedSegment];
}

- (void) selectSegmentWithTag: (NSInteger) tag
{
  [_cell selectSegmentWithTag: tag];
}

// Working with individual segments...
- (void) setWidth: (CGFloat)width forSegment: (NSInteger)segment
{
  [_cell setWidth: width forSegment: segment];
}

- (CGFloat) widthForSegment: (NSInteger)segment
{
  return [_cell widthForSegment: segment];
}

- (void) setImage: (NSImage *)image forSegment: (NSInteger)segment
{
  [_cell setImage: image forSegment: segment];
}

- (NSImage *) imageForSegment: (NSInteger)segment
{
  return [_cell imageForSegment: segment];
}

- (void) setLabel: (NSString *)label forSegment: (NSInteger)segment
{
  [_cell setLabel: label forSegment: segment];
}

- (NSString *) labelForSegment: (NSInteger)segment
{
  return [_cell labelForSegment: segment];
}

- (void) setMenu: (NSMenu *)menu forSegment: (NSInteger)segment
{
  [_cell setMenu: menu forSegment: segment];
}

- (NSMenu *) menuForSegment: (NSInteger)segment
{
  return [_cell menuForSegment: segment];
}

- (void) setSelected: (BOOL)flag forSegment: (NSInteger)segment
{
  [_cell setSelected: flag forSegment: segment];
}

- (BOOL) isSelectedForSegment: (NSInteger)segment
{
  return [_cell isSelectedForSegment: segment];
}

- (void) setEnabled: (BOOL)flag forSegment: (NSInteger)segment
{
  [_cell setEnabled: flag forSegment: segment];
}

- (BOOL) isEnabledForSegment: (NSInteger)segment
{
  return [_cell isEnabledForSegment: segment];
}

- (void) setSegmentStyle: (NSSegmentStyle)style
{
  [_cell setSegmentStyle: style];
}

- (NSSegmentStyle) segmentStyle
{
  return [_cell segmentStyle];
}

/*
- (void) mouseDown: (NSEvent *)event
{
  NSPoint location = [self convertPoint: [event locationInWindow] 
                           fromView: nil];

  [super mouseDown: event];
  [_cell _detectHit: location];
  NSLog(@"%@",NSStringFromPoint(location));
}
*/
@end
