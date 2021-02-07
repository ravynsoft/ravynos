/** <title>NSTableHeaderCell</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: 1999
   
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

#import "AppKit/NSColor.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSTableHeaderCell.h"
#import "GNUstepGUI/GSTheme.h"

@implementation NSTableHeaderCell

// Default appearance of NSTableHeaderCell
- (id) initTextCell: (NSString *)aString
{
  self = [super initTextCell: aString];
  if (!self)
    return nil;

  [self setAlignment: NSCenterTextAlignment];
  [self setTextColor: [[GSTheme theme] tableHeaderTextColorForState: GSThemeNormalState]];
  [self setBackgroundColor: [NSColor controlShadowColor]];
  [self setDrawsBackground: YES];
  [self setFont: [NSFont titleBarFontOfSize: 0]];
  [self setWraps: NO];
  // This is not exactly true 
  _cell.is_bezeled = YES;
  _cell.is_bordered = NO;

  return self;
}

- (NSRect) drawingRectForBounds: (NSRect)theRect
{
  return [[GSTheme theme] tableHeaderCellDrawingRectForBounds: theRect];
}

- (void) _drawBorderAndBackgroundWithFrame: (NSRect)cellFrame 
                                    inView: (NSView*)controlView
{
  GSThemeControlState state;
  if (_cell.is_highlighted == YES)
    {
      state = GSThemeHighlightedState;
    }
  else
    {
      state = GSThemeNormalState;
    }

  [[GSTheme theme] drawTableHeaderCell: self
                             withFrame: cellFrame
                                inView: controlView
                                 state: state];
}

- (void) drawSortIndicatorWithFrame: (NSRect)cellFrame
                             inView: (NSView *)controlView
                          ascending: (BOOL)ascending
                           priority: (int)priority
{
  NSImage *img;

  cellFrame = [self sortIndicatorRectForBounds: cellFrame];
  if (ascending)
    img = [NSImage imageNamed: @"NSAscendingSortIndicator"];
  else
    img = [NSImage imageNamed: @"NSDescendingSortIndicator"];

  [img drawAtPoint: cellFrame.origin 
       fromRect: NSZeroRect
       operation: NSCompositeSourceOver
       fraction: 1.0];
}

- (NSRect) sortIndicatorRectForBounds: (NSRect)theRect
{
  NSImage *img = [NSImage imageNamed: @"NSAscendingSortIndicator"];
  NSSize size = [img size];

  theRect.origin.x = NSMaxX(theRect) - size.width;
  theRect.size = size;

  return theRect;
}

- (void) setHighlighted: (BOOL)flag
{
  _cell.is_highlighted = flag;
  
  if (flag == YES)
    {
      [self setBackgroundColor: [NSColor controlHighlightColor]];
      [self setTextColor: [[GSTheme theme] tableHeaderTextColorForState: GSThemeHighlightedState]];
    }
  else
    {
      [self setBackgroundColor: [NSColor controlShadowColor]];
      [self setTextColor: [[GSTheme theme] tableHeaderTextColorForState: GSThemeNormalState]];
    }
}

@end
