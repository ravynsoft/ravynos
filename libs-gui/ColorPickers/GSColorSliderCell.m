/* GSStandardColorPicker.m

   Copyright (C) 2007 Free Software Foundation, Inc.

   Author:  Fred Kiefer <FredKiefer@gmx.de>
   Date: December 2007
   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: May 2002

   This file is part of GNUstep.
   
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

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include "GSColorSliderCell.h"

#define KNOB_WIDTH 6

@implementation GSColorSliderCell : NSSliderCell

-(void) _setColorSliderCellMode: (int)m
{
  mode = m;
  switch (mode)
    {
      case 0:
      case 1:
      case 2:
      case 3:
      case 10:
        [_titleCell setTextColor: [NSColor whiteColor]];
        break;
      case 4:
      case 5:
      case 6:
      case 7:
        [_titleCell setTextColor: [NSColor blackColor]];
        break;
    }
  [_titleCell setAlignment: NSLeftTextAlignment];
}

-(void) _setColorSliderCellValues: (float)a : (float)b : (float)c
{
  values[0] = a;
  values[1] = b;
  values[2] = c;
  if (mode == 8 || mode == 9)
    {
      if (c > 0.7)
        [_titleCell setTextColor: [NSColor blackColor]];
      else
        [_titleCell setTextColor: [NSColor whiteColor]];
    }
}

- (NSRect) knobRectFlipped: (BOOL)flipped
{
  NSPoint origin;
  float floatValue = [self floatValue];

  if (_isVertical && flipped)
    {
      floatValue = _maxValue + _minValue - floatValue;
    }

  floatValue = (floatValue - _minValue) / (_maxValue - _minValue);

  origin = _trackRect.origin;
  if (_isVertical == YES)
    {
      origin.y += (_trackRect.size.height - KNOB_WIDTH) * floatValue;
      return NSMakeRect (origin.x, origin.y, _trackRect.size.width, KNOB_WIDTH);
    }
  else
    {
      origin.x += (_trackRect.size.width - KNOB_WIDTH) * floatValue;
      return NSMakeRect (origin.x, origin.y, KNOB_WIDTH, _trackRect.size.height);
    }
}

- (void) drawKnob: (NSRect)knobRect
{
  [[NSColor blackColor] set];
  NSDrawButton(knobRect, knobRect);
}

-(void) drawBarInside: (NSRect)r  flipped: (BOOL)flipped
{
  float i, f;

  PSsetalpha(1);

  if (_isVertical)
    {
      for (i = 0; i < r.size.height; i += 1)
        {
          f = (0.5 + i) / r.size.height;
          if (flipped)
            {
              f = 1 - f;
            }
          switch (mode)
            {
              case 0: PSsetgray(f); break;

              case 1: PSsetrgbcolor(f, values[1], values[2]); break;
              case 2: PSsetrgbcolor(values[0], f, values[2]); break;
              case 3: PSsetrgbcolor(values[0], values[1], f); break;
                  
              case 4: PSsetcmykcolor(f, values[1], values[2], 0); break;
              case 5: PSsetcmykcolor(values[0], f, values[2], 0); break;
              case 6: PSsetcmykcolor(values[0], values[1], f, 0); break;
              case 7: PSsetcmykcolor(values[0], values[1], values[2], f); break;
                  
              case  8: PSsethsbcolor(f, values[1], values[2]); break;
              case  9: PSsethsbcolor(values[0], f, values[2]); break;
              case 10: PSsethsbcolor(values[0], values[1], f); break;
            }
          if (i + 1 < r.size.height)
            PSrectfill(r.origin.x, i + r.origin.y, r.size.width, 1);
          else
            PSrectfill(r.origin.x, i + r.origin.y, r.size.width, r.size.height - i);
        }
    }
  else
    {
      for (i = 0; i < r.size.width; i += 1)
        {
          f = (0.5 + i) / r.size.width;
          switch (mode)
            {
              case 0: PSsetgray(f); break;

              case 1: PSsetrgbcolor(f, values[1], values[2]); break;
              case 2: PSsetrgbcolor(values[0], f, values[2]); break;
              case 3: PSsetrgbcolor(values[0], values[1], f); break;
                  
              case 4: PSsetcmykcolor(f, values[1], values[2], 0); break;
              case 5: PSsetcmykcolor(values[0], f, values[2], 0); break;
              case 6: PSsetcmykcolor(values[0], values[1], f, 0); break;
              case 7: PSsetcmykcolor(values[0], values[1], values[2], f); break;
                  
              case  8: PSsethsbcolor(f, values[1], values[2]); break;
              case  9: PSsethsbcolor(values[0], f, values[2]); break;
              case 10: PSsethsbcolor(values[0], values[1], f); break;
            }
          if (i + 1 < r.size.width)
            PSrectfill(i, r.origin.y, 1, r.size.height);
          else
            PSrectfill(i, r.origin.y, r.size.width - i, r.size.height);
        }

      [_titleCell drawInteriorWithFrame: r inView: _control_view];
    }
}

-(void) drawInteriorWithFrame: (NSRect)cellFrame inView: (NSView *)controlView
{
  _isVertical = (cellFrame.size.height > cellFrame.size.width);
  cellFrame = [self drawingRectForBounds: cellFrame];

  cellFrame.origin.x -= 1;
  cellFrame.origin.y -= 1;
  cellFrame.size.width += 2;
  cellFrame.size.height += 2;

  _trackRect = cellFrame;

  [self drawBarInside: cellFrame flipped: [controlView isFlipped]];

  [self drawKnob];
}

- (CGFloat) knobThickness
{
  return KNOB_WIDTH;
}

@end
