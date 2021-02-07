/* NSTextBlock.m

   Copyright (C) 2008 Free Software Foundation, Inc.

   Author:  H. Nikolaus Schaller
   Date: 2007
   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: January 2008
   
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

#import <Foundation/NSCoder.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>

#import "AppKit/NSColor.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSTextTable.h"
#import "GSGuiPrivate.h"

@implementation NSTextBlock

- (id) init
{
  // FIXME
  return self;
}

- (void) dealloc
{
  RELEASE(_backgroundColor);
  RELEASE(_borderColorForEdge[NSMinXEdge]);
  RELEASE(_borderColorForEdge[NSMinYEdge]);
  RELEASE(_borderColorForEdge[NSMaxXEdge]);
  RELEASE(_borderColorForEdge[NSMaxYEdge]);
  [super dealloc];
}

- (NSColor *) backgroundColor
{
  return _backgroundColor;
}

- (void) setBackgroundColor: (NSColor *)color
{
    ASSIGN(_backgroundColor, color);
}

- (NSColor *) borderColorForEdge: (NSRectEdge)edge
{
  return _borderColorForEdge[edge];
}

- (void) setBorderColor: (NSColor *)color forEdge: (NSRectEdge)edge
{
  if (edge >= sizeof(_borderColorForEdge) / sizeof(_borderColorForEdge[0]))
    [NSException raise: NSInvalidArgumentException
                 format: @"invalid edge %lu", (unsigned long) edge];
  ASSIGN(_borderColorForEdge[edge], color);
}

- (void) setBorderColor: (NSColor *)color
{
  ASSIGN(_borderColorForEdge[NSMinXEdge], color);
  ASSIGN(_borderColorForEdge[NSMinYEdge], color);
  ASSIGN(_borderColorForEdge[NSMaxXEdge], color);
  ASSIGN(_borderColorForEdge[NSMaxYEdge], color);
}

- (CGFloat) contentWidth
{
  return [self valueForDimension: NSTextBlockWidth];
}

- (NSTextBlockValueType) contentWidthValueType
{
  return [self valueTypeForDimension: NSTextBlockWidth];
}

- (void) setContentWidth: (CGFloat)val type: (NSTextBlockValueType)type
{
  [self setValue: val type: type forDimension: NSTextBlockWidth];
}

- (NSTextBlockVerticalAlignment) verticalAlignment
{
  return _verticalAlignment;
}

- (void) setVerticalAlignment: (NSTextBlockVerticalAlignment)alignment
{
 _verticalAlignment = alignment; 
}

- (CGFloat) valueForDimension: (NSTextBlockDimension)dimension
{
  if (dimension >= sizeof(_valueType) / sizeof(_valueType[0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid dimension %d", dimension];
  return _value[dimension];
}

- (NSTextBlockValueType) valueTypeForDimension: (NSTextBlockDimension)dimension
{
  if (dimension >= sizeof(_valueType) / sizeof(_valueType[0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid dimension %d", dimension];
  return _valueType[dimension];
}

- (CGFloat) _scaledValue: (NSTextBlockDimension)dimension : (NSSize)size
{
  if (_valueType[dimension] == NSTextBlockAbsoluteValueType)
    {
      return _value[dimension];
    }
  else
    {
      // specified in percent
      switch(dimension)
        {
        case NSTextBlockWidth:
        case NSTextBlockMinimumWidth:
        case NSTextBlockMaximumWidth:
          return _value[dimension] * size.width;
        case NSTextBlockHeight:
        case NSTextBlockMinimumHeight:
        case NSTextBlockMaximumHeight:
          return _value[dimension] * size.height;
        }
    }
  return 0.0;	
}

- (void) setValue: (CGFloat)val 
             type: (NSTextBlockValueType)type
     forDimension: (NSTextBlockDimension)dimension
{
  if (dimension >= sizeof(_valueType) / sizeof(_valueType[0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid dimension %d", dimension];
  _value[dimension] = val;
  _valueType[dimension] = type;
}

- (CGFloat) widthForLayer: (NSTextBlockLayer)layer edge: (NSRectEdge)edge
{
  if (layer >= sizeof(_width) / sizeof(_width[0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid layer %d", layer];
  if (edge >= sizeof(_width[0]) / sizeof(_width[0][0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid edge %lu", (unsigned long) edge];
  return _width[layer][edge];
}

- (NSTextBlockValueType) widthValueTypeForLayer: (NSTextBlockLayer)layer
                                           edge: (NSRectEdge)edge
{
  if (layer >= sizeof(_width) / sizeof(_width[0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid layer %d", layer];
  if (edge >= sizeof(_width[0]) / sizeof(_width[0][0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid edge %lu", (unsigned long) edge];
  return _widthType[layer][edge];
}

- (void) setWidth: (CGFloat)val
             type: (NSTextBlockValueType)type
         forLayer: (NSTextBlockLayer)layer
             edge: (NSRectEdge)edge
{
  if (layer >= sizeof(_width) / sizeof(_width[0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid layer %d", layer];
  if (edge >= sizeof(_width[0]) / sizeof(_width[0][0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid edge %lu", (unsigned long) edge];
  _width[layer][edge] = val;
  _widthType[layer][edge] = type;
}

- (void) setWidth: (CGFloat)val
             type: (NSTextBlockValueType)type 
         forLayer: (NSTextBlockLayer)layer
{
  if (layer >= sizeof(_width) / sizeof(_width[0]))
    [NSException raise: NSInvalidArgumentException
		 format: @"invalid layer %d", layer];
  _width[layer][NSMinXEdge] = val;
  _widthType[layer][NSMinXEdge] = type;
  _width[layer][NSMinYEdge] = val;
  _widthType[layer][NSMinYEdge] = type;
  _width[layer][NSMaxXEdge] = val;
  _widthType[layer][NSMaxXEdge] = type;
  _width[layer][NSMaxYEdge] = val;
  _widthType[layer][NSMaxYEdge] = type;
}

- (CGFloat) _scaledWidthValue: (NSTextBlockLayer) layer : (NSRectEdge) edge : (NSSize) size
{
  if (_widthType[layer][edge] == NSTextBlockAbsoluteValueType)
    {
      // absolute
      return _width[layer][edge];
    }
  else
    {
      // specified in percent
      switch(edge)
        {
        case NSMinXEdge:
        case NSMaxXEdge:
          return _widthType[layer][edge]*size.width;
        case NSMinYEdge:
        case NSMaxYEdge:
          return _widthType[layer][edge]*size.height;
        }
    }
  return 0.0;	
}

- (NSRect) boundsRectForContentRect: (NSRect)cont
                             inRect: (NSRect)rect
                      textContainer: (NSTextContainer *)container
                     characterRange: (NSRange)range
{
  CGFloat minx = [self _scaledWidthValue: NSTextBlockPadding : NSMinXEdge: rect.size] 
    + [self _scaledWidthValue: NSTextBlockBorder : NSMinXEdge : rect.size]
    + [self _scaledWidthValue: NSTextBlockMargin : NSMinXEdge : rect.size];

  CGFloat maxx = [self _scaledWidthValue: NSTextBlockPadding: NSMaxXEdge: rect.size]
    + [self _scaledWidthValue: NSTextBlockBorder : NSMaxXEdge : rect.size]
    + [self _scaledWidthValue: NSTextBlockMargin : NSMaxXEdge : rect.size];
  CGFloat miny= [self _scaledWidthValue: NSTextBlockPadding : NSMinYEdge: rect.size] 
    + [self _scaledWidthValue: NSTextBlockBorder : NSMinYEdge : rect.size]
    + [self _scaledWidthValue: NSTextBlockMargin : NSMinYEdge : rect.size];

  CGFloat maxy = [self _scaledWidthValue: NSTextBlockPadding: NSMaxYEdge: rect.size]
    + [self _scaledWidthValue: NSTextBlockBorder : NSMaxYEdge : rect.size]
    + [self _scaledWidthValue: NSTextBlockMargin : NSMaxYEdge : rect.size];

  cont.origin.x -= minx;
  cont.size.width += minx + maxx;
  cont.origin.y -= miny;
  cont.size.height += miny + maxy;
  return cont;
}

/**
 * POINT is the point in NSTextContainer where the TextBlock should be laid out.
 * RECT is the bounding rect (e.g. the rect of the container or the rect of the 
 * outer table cell) 
 * what are we doing with CONT? Do we limit to width of container?
 * what are we doing with RANGE? We don't know the layout manager
 * This is the default implementation for a single-cell 
 * (NSTextTableBlock can handle cell span)
 * raises internal inconsisteny exception if the layout manager 
 * (the one owning the textContainer)
 * does not have a table at the given characterRange
 */
- (NSRect) rectForLayoutAtPoint: (NSPoint)point
                         inRect: (NSRect)rect
                  textContainer: (NSTextContainer *)cont
                 characterRange: (NSRange)range
{
  NSRect r;
  NSSize size = (NSSize){[self _scaledValue: NSTextBlockWidth : rect.size], 
			 [self _scaledValue: NSTextBlockHeight : rect.size]};
  // when and how do we define size by content? If size is (0, 0)?
  // or is this the input to calculating size of enclosed text?
  size.width = MAX(size.width, [self _scaledValue: NSTextBlockMinimumWidth : rect.size]);
  // not smaller than minimum
  size.height = MAX(size.height, [self _scaledValue: NSTextBlockMinimumHeight : rect.size]);
  size.width = MIN(size.width, [self _scaledValue: NSTextBlockMaximumWidth : rect.size]);
  // but also not larger than maximum
  size.height = MIN(size.height, [self _scaledValue: NSTextBlockMaximumHeight : rect.size]);
  r = (NSRect){point, size};
  // who handles vertical alignment?
  // limit to what is available
  return NSIntersectionRect(r, rect);
}

- (void) drawBackgroundWithFrame: (NSRect)rect	// this is the frame of the cell
                          inView: (NSView *)view 
                  characterRange: (NSRange)range
                   layoutManager: (NSLayoutManager *)lm
{
  CGFloat minx = [self _scaledWidthValue: NSTextBlockPadding : NSMinXEdge : rect.size];
  CGFloat maxx = [self _scaledWidthValue: NSTextBlockPadding : NSMaxXEdge : rect.size];
  CGFloat miny = [self _scaledWidthValue: NSTextBlockPadding : NSMinYEdge : rect.size];
  CGFloat maxy = [self _scaledWidthValue: NSTextBlockPadding : NSMaxYEdge : rect.size];

  // FIXME - inset from frame by margin in the first step
  rect.origin.x -= minx;
  rect.size.width += minx + maxx;
  rect.origin.y -= miny;
  rect.size.height += miny + maxy;
  [_backgroundColor set];
  // fill inner rect
  NSRectFill(rect);
  
  minx = [self _scaledWidthValue: NSTextBlockBorder : NSMinXEdge : rect.size];
  maxx = [self _scaledWidthValue: NSTextBlockBorder : NSMaxXEdge : rect.size];
  miny = [self _scaledWidthValue: NSTextBlockBorder : NSMinYEdge : rect.size];
  maxy = [self _scaledWidthValue: NSTextBlockBorder : NSMaxYEdge : rect.size];
  [_borderColorForEdge[NSMinXEdge] set];
  NSRectFill(NSMakeRect(rect.origin.x - minx, rect.origin.y, minx, rect.size.height));
  [_borderColorForEdge[NSMaxYEdge] set];
  NSRectFill(NSMakeRect(rect.origin.x, rect.origin.y + rect.size.height + maxy, rect.size.width, maxy));
  [_borderColorForEdge[NSMaxXEdge] set];
  NSRectFill(NSMakeRect(rect.origin.x + rect.size.width, rect.origin.y, maxx, rect.size.height));
  [_borderColorForEdge[NSMinYEdge] set];
  NSRectFill(NSMakeRect(rect.origin.x, rect.origin.y - maxy, rect.size.width, miny));
  // FIXME: how do we handle the corners of differenly sized and colored borders? 
  // Do we have to fill trapezoids?
}

- (id) copyWithZone: (NSZone*)zone
{
  NSTextBlock *t = (NSTextBlock*)NSCopyObject(self, 0, zone);

  _backgroundColor = TEST_RETAIN(_backgroundColor);
  _borderColorForEdge[NSMinXEdge] =
      TEST_RETAIN(_borderColorForEdge[NSMinXEdge]);
  _borderColorForEdge[NSMinYEdge] =
      TEST_RETAIN(_borderColorForEdge[NSMinYEdge]);
  _borderColorForEdge[NSMaxXEdge] =
      TEST_RETAIN(_borderColorForEdge[NSMaxXEdge]);
  _borderColorForEdge[NSMaxYEdge] =
      TEST_RETAIN(_borderColorForEdge[NSMaxYEdge]);

  return t;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  // FIXME
  if ([aCoder allowsKeyedCoding])
    {
    }
  else
    {
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  // FIXME
  if ([aDecoder allowsKeyedCoding])
    {
    }
  else
    {
    }
  return self;
}

@end
