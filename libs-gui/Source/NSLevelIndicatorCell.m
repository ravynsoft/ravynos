/* 
   NSLevelIndicatorCell.m

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

#import <Foundation/NSException.h>

#import "AppKit/NSBezierPath.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSLevelIndicatorCell.h"
#import "GSGuiPrivate.h"

@implementation NSLevelIndicatorCell

- (id) init
{
  return [self initWithLevelIndicatorStyle: NSRatingLevelIndicatorStyle];
}

- (id) initWithLevelIndicatorStyle: (NSLevelIndicatorStyle)style
{
  self = [super init];
  if (self)
    {
      [self setAlignment: NSCenterTextAlignment];
      [self setLevelIndicatorStyle: style];
      //_minValue = 0.0;
      if ((style == NSContinuousCapacityLevelIndicatorStyle) ||
          (style == NSRelevancyLevelIndicatorStyle))
        {
          _maxValue = 100.0;
        }
      else
        {
          _maxValue = 5.0;
        }
      [self setDoubleValue: 0.0];
    }

  return self;
}

- (id) copyWithZone:(NSZone *) zone
{
  NSLevelIndicatorCell *c = [super copyWithZone: zone];

  return c;
}

- (void) dealloc
{
  [super dealloc];
}

- (NSLevelIndicatorStyle) style
{
  return _style; 
}


- (double) maxValue
{
  return _maxValue;
}

- (void) setMaxValue: (double)val
{
  _maxValue = val;
}

- (double) minValue 
{
  return _minValue;
}

- (void) setMinValue:(double) val
{
  _minValue = val;
}

- (double) criticalValue
{ 
  return _criticalValue; 
}

- (void) setCriticalValue: (double)val
{
  _criticalValue = val;
}

- (double) warningValue
{
  return _warningValue;
}

- (void) setWarningValue: (double)val
{
  _warningValue = val;
}

- (void) setLevelIndicatorStyle: (NSLevelIndicatorStyle)style
{
  _style = style;
}

- (NSInteger) numberOfMajorTickMarks
{
  return _numberOfMajorTickMarks;
}

- (void) setNumberOfMajorTickMarks: (NSInteger)count
{
  _numberOfMajorTickMarks = count;
}

- (NSInteger) numberOfTickMarks
{
  return _numberOfTickMarks;
}

- (void) setNumberOfTickMarks: (NSInteger)count
{
  _numberOfTickMarks = count;
}

- (NSTickMarkPosition) tickMarkPosition
{
  return _tickMarkPosition;
}

- (void) setTickMarkPosition: (NSTickMarkPosition)pos
{
  _tickMarkPosition = pos;
}

- (double) tickMarkValueAtIndex: (NSInteger)index
{
  if ((index < 0) || (index >= _numberOfTickMarks))
    {
      [NSException raise: NSRangeException
                   format: @"tick mark index invalid"];
    }

  return _minValue + index * (_maxValue - _minValue) / _numberOfTickMarks;
}

- (NSRect) rectOfTickMarkAtIndex: (NSInteger)index
{
  NSRect cellRect = NSZeroRect;
  float frameWidth = _cellFrame.size.width;

  if ((index < 0) || (index >= _numberOfTickMarks))
    {
      [NSException raise: NSRangeException
        format: @"tick mark index invalid"];
    }

  // Create default minor tickmark size in cellRect
  cellRect.size.width = 1;
  cellRect.size.height = 4;
  
  // If all tick marks are major:
  if (_numberOfTickMarks <= _numberOfMajorTickMarks)
    {
      // Use major tick mark size
      cellRect.size.width = 3;
      cellRect.size.height = 7;
    }
  // If major tick marks fit with even spacing
  else if ((_numberOfTickMarks -1) % (_numberOfMajorTickMarks - 1) == 0)
    {
      NSInteger minorTicksPerMajor = (_numberOfTickMarks - 1) / (_numberOfMajorTickMarks - 1);

      // If index is a major tick mark
      if (index % minorTicksPerMajor == 0)
        {
          // Use major tick mark size
          cellRect.size.width = 3;
          cellRect.size.height = 7;
        }
    }
  // FIXME: Extra tick mark code, when all major tick marks don't fit but a lesser amount will
        
  // Last tick mark
  if (index == (_numberOfTickMarks - 1))
    {
      cellRect.origin.x = (frameWidth - cellRect.size.width);
    }
  // Not the first tick mark. (First tick mark will use 0,0 default values already set)
  else if (index != 0)
    {
      float spacing = (frameWidth / (_numberOfTickMarks - 1)) * index;
      cellRect.origin.x = spacing - (cellRect.size.width / 2);
    }
        
  // Set origins if tick marks are above the indicator
  if (_tickMarkPosition == NSTickMarkAbove)
    {
      switch (_style)
        {
          case NSContinuousCapacityLevelIndicatorStyle:
            {
              cellRect.origin.y = 16;
              break;
            }
          case NSRelevancyLevelIndicatorStyle:
            {
              cellRect.origin.y = 12;
              break;
            }
          case NSRatingLevelIndicatorStyle:
            {
              cellRect.origin.y = 13;
              break;
            }
          case NSDiscreteCapacityLevelIndicatorStyle:
            {
              cellRect.origin.y = 18;
              break;
            }
        }
    }
  // If tick mark is minor and below indicator, use y origin of 3. If not, default value of 0 is used
  else if (cellRect.size.width == 1)
    {
      cellRect.origin.y = 3;
    }
        
  return NSIntegralRect(cellRect);
}

- (NSSize) cellSize
{
  // Sizes are the same as those from OSX
  NSSize cellSize = NSMakeSize(400000, 25);

  // Change cellSize to the correct size:
  switch (_style)
    {
      case NSContinuousCapacityLevelIndicatorStyle:
        {
          cellSize.height = 23;
          break;
        }
      case NSRelevancyLevelIndicatorStyle:
        {
          cellSize.height = 19;
          break;
        }
      case NSRatingLevelIndicatorStyle:
        {
          cellSize.height = 20;
          cellSize.width = 13 * _maxValue;
          break;
        }
      case NSDiscreteCapacityLevelIndicatorStyle:
        {
          cellSize.height = 25;
          break;
        }
    }
    
  return cellSize;
}

- (void) drawInteriorWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  NSColor *fillColor;
  double value = [self doubleValue];
  double val = (value -_minValue) / (_maxValue -_minValue);
  BOOL vertical = (cellFrame.size.height > cellFrame.size.width);
        
  _cellFrame = cellFrame;
  if (value < _warningValue)
    fillColor = [NSColor greenColor];
  else if (value < _criticalValue)
    fillColor = [NSColor yellowColor];
  else
    fillColor = [NSColor redColor];

  if (_numberOfTickMarks != 0)
    {
      // FIXME: this code only works for horizontal frames
      float x;
      float y0, y1;
      float step = _numberOfTickMarks > 1 ? 
          (cellFrame.size.width - 1.0) / (_numberOfTickMarks - 1) : 
          1.0;
      int tick;

      if (_tickMarkPosition == NSTickMarkBelow)
        {
          cellFrame.origin.y += 8.0;
          cellFrame.size.height -= 8.0;
          y0 = 4.0;
          y1 = 8.0;
        }
      else
        {
          cellFrame.size.height -= 8.0;
          y0 = cellFrame.size.height;
          y1 = y0 + 4.0;
        }

      [[NSColor darkGrayColor] set];
      for(x = 0.0, tick = 0; tick <= _numberOfTickMarks; x += step, tick++)
        {
          [NSBezierPath strokeLineFromPoint: NSMakePoint(x, y0) toPoint: NSMakePoint(x, y1)];
          // FIXME: draw _numberOfMajorTickMarks thick ticks (ignore if more than _numberOfTickMarks)
        }
    }

  switch(_style)
    {
      case NSDiscreteCapacityLevelIndicatorStyle:
        {
          int segments = (int)(_maxValue - _minValue);
          // width of one segment
          float step = (segments > 0) ? 
              ((vertical ? cellFrame.size.height : cellFrame.size.width) / segments) : 
              10.0;
          int i;
          int ifill = val * segments + 0.5;

          for( i = 0; i < segments; i++)
            {
              // draw segments
              NSRect seg = cellFrame;

              if (vertical)
                {
                  seg.size.height = step - 1.0;
                  seg.origin.y += i * step;
                }
              else
                {
                  seg.size.width = step - 1.0;
                  seg.origin.x += i * step;
                }

              if (i < ifill)
                  [fillColor set];
              else
                  // FIXME: Should not be needed.
                  [[NSColor controlBackgroundColor] set];

              // we could also fill with a scaled horizontal/vertical image
              NSRectFill(seg);
              // draw border
              [[NSColor lightGrayColor] set];
              NSFrameRect(seg);
            }
          break;
        }
      case NSContinuousCapacityLevelIndicatorStyle:
        {
          NSRect ind, fill;

          if (vertical)
            NSDivideRect(cellFrame, &ind, &fill, cellFrame.size.height * val, NSMinYEdge);
          else
            NSDivideRect(cellFrame, &ind, &fill, cellFrame.size.width * val, NSMinXEdge);

          [fillColor set];
          // we could also fill with a scaled horizontal/vertical image
          NSRectFill(ind);
          // FIXME: Not needed
          [[NSColor controlBackgroundColor] set];
          NSRectFill(fill);
          // draw border
          [[NSColor lightGrayColor] set];
          NSFrameRect(cellFrame);
          break;
        }
      case NSRelevancyLevelIndicatorStyle:
        {
          // FIXME: Not needed
          [[NSColor controlBackgroundColor] set];
          NSRectFill(cellFrame);

          [[NSColor darkGrayColor] set];
          if (vertical)
            {
              float y;
              float yfill = val * cellFrame.size.height + 0.5;

              for (y = 0.0; y < yfill; y += 2.0)
                {
                  [NSBezierPath strokeLineFromPoint: NSMakePoint(0.0, y) 
                                toPoint: NSMakePoint(cellFrame.size.width, y)];
                }
            }
          else
            {
              float x;
              float xfill = val * cellFrame.size.width + 0.5;
              for (x = 0.0; x < xfill; x += 2.0)
                {
                  [NSBezierPath strokeLineFromPoint: NSMakePoint(x, 0.0) 
                                toPoint: NSMakePoint(x, cellFrame.size.height)];
                }
            }
          break;
        }
      case NSRatingLevelIndicatorStyle:
        {
          NSImage *indicator = [self image];
          NSSize isize;

          if (!indicator)
              indicator = [NSImage imageNamed: @"NSRatingLevelIndicator"];

          isize = [indicator size];

          // FIXME: Not needed
          [[NSColor controlBackgroundColor] set];
          NSRectFill(cellFrame);

          if (vertical)
            {
              int y;

              for (y = 0.0; y < (val + 0.5); y++)
                {
                  NSPoint pos = NSMakePoint(0, y * isize.height + 2.0);

                  if (pos.y >= cellFrame.size.height)
                    break;
                        
                  // here we can strech the image as needed by using drawInRect:
                  [indicator drawAtPoint: pos 
                             fromRect: (NSRect){NSZeroPoint, isize} 
                             operation: NSCompositeCopy 
                             fraction:1.0];
                }

              // FIXME: Should draw place holder for the rest of the cell frame
            }
          else
            {
              int x;

              for (x = 0.0; x < (val + 0.5); x++)
                {
                  NSPoint pos = NSMakePoint(x * isize.width + 2.0, 0.0);

                  if(pos.x >= cellFrame.size.width)
                    break;
                        
                  [indicator drawAtPoint: pos 
                             fromRect: (NSRect){NSZeroPoint, isize} 
                             operation: NSCompositeCopy 
                             fraction: 1.0];
                }

              // FIXME: Should draw place holder for the rest of the cell frame
            }
        }
    }
}

- (void) encodeWithCoder: (NSCoder *) aCoder
{
  [super encodeWithCoder:aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeDouble: _minValue forKey: @"NSMinValue"];
      [aCoder encodeDouble: _maxValue forKey: @"NSMaxValue"];
      [aCoder encodeDouble: _warningValue forKey: @"NSWarningValue"];
      [aCoder encodeDouble: _criticalValue forKey: @"NSCriticalValue"];
      [aCoder encodeDouble: [self doubleValue] forKey: @"NSValue"];
      [aCoder encodeInt: _style forKey: @"NSIndicatorStyle"];
      [aCoder encodeInt: _numberOfMajorTickMarks forKey: @"NSNumberOfMajorTickMarks"];
      [aCoder encodeInt: _numberOfTickMarks forKey: @"NSNumberOfTickMarks"];
      [aCoder encodeInt: _tickMarkPosition forKey: @"NSTickMarkPosition"];
    }
  else
    {
      NSInteger tmp;

      [aCoder encodeValueOfObjCType: @encode(double) at: &_minValue];
      [aCoder encodeValueOfObjCType: @encode(double) at: &_maxValue];
      [aCoder encodeValueOfObjCType: @encode(double) at: &_warningValue];
      [aCoder encodeValueOfObjCType: @encode(double) at: &_criticalValue];
      tmp = _style;
      encode_NSInteger(aCoder, &tmp);
      encode_NSInteger(aCoder, &_numberOfMajorTickMarks);
      encode_NSInteger(aCoder, &_numberOfTickMarks);
      tmp = _tickMarkPosition;
      encode_NSInteger(aCoder, &tmp);
    }
}

- (id) initWithCoder:(NSCoder *) aDecoder
{
  self = [super initWithCoder:aDecoder];

  if ([aDecoder allowsKeyedCoding])
    {
      if ([aDecoder containsValueForKey: @"NSMinValue"])
        {
          _minValue = [aDecoder decodeDoubleForKey: @"NSMinValue"];
        }
      if ([aDecoder containsValueForKey: @"NSMaxValue"])
        {
          _maxValue = [aDecoder decodeDoubleForKey: @"NSMaxValue"];
        }
      if ([aDecoder containsValueForKey: @"NSWarningValue"])
        {
          _warningValue = [aDecoder decodeDoubleForKey: @"NSWarningValue"];
        }
      if ([aDecoder containsValueForKey: @"NSCriticalValue"])
        {
          _criticalValue = [aDecoder decodeDoubleForKey: @"NSCriticalValue"];
        }
       if ([aDecoder containsValueForKey: @"NSValue"])
        {
          [self setDoubleValue: [aDecoder decodeDoubleForKey: @"NSValue"]];
        }
      if ([aDecoder containsValueForKey: @"NSIndicatorStyle"])
        {
          _style = [aDecoder decodeIntForKey: @"NSIndicatorStyle"];
        }
      if ([aDecoder containsValueForKey: @"NSNumberOfMajorTickMarks"])
        {
          _numberOfMajorTickMarks = [aDecoder decodeIntForKey: @"NSNumberOfMajorTickMarks"];
        }
      if ([aDecoder containsValueForKey: @"NSNumberOfTickMarks"])
        {
          _numberOfTickMarks = [aDecoder decodeIntForKey: @"NSNumberOfTickMarks"];
        }
      if ([aDecoder containsValueForKey: @"NSTickMarkPosition"])
        {
          _tickMarkPosition = [aDecoder decodeIntForKey: @"NSTickMarkPosition"];
        }
    }
  else
    {
      NSInteger tmp;

      [aDecoder decodeValueOfObjCType: @encode(double) at: &_minValue];
      [aDecoder decodeValueOfObjCType: @encode(double) at: &_maxValue];
      [aDecoder decodeValueOfObjCType: @encode(double) at: &_warningValue];
      [aDecoder decodeValueOfObjCType: @encode(double) at: &_criticalValue];
      decode_NSInteger(aDecoder, &tmp);
      _style = tmp;
      decode_NSInteger(aDecoder, &_numberOfMajorTickMarks);
      decode_NSInteger(aDecoder, &_numberOfTickMarks);
      decode_NSInteger(aDecoder, &tmp);
      _tickMarkPosition = tmp;
    }
  
  return self;
}

@end
