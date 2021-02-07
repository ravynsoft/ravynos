/** <title>GSThemeTools</title>

   <abstract>Useful/configurable drawing functions</abstract>

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author: Adam Fedor <fedor@gnu.org>
   Date: Jan 2004
   
   This file is part of the GNU Objective C User interface library.

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
#import "AppKit/PSOperators.h"
#import "GSThemePrivate.h"

#include <math.h>
#include <float.h>


@implementation	GSTheme (MidLevelDrawing)

- (NSRect) drawButton: (NSRect)border withClip: (NSRect)clip
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, 
			   NSMinXEdge, NSMaxYEdge, 
			   NSMaxXEdge, NSMinYEdge};
  NSRectEdge dn_sides[] = {NSMaxXEdge, NSMaxYEdge, 
			   NSMinXEdge, NSMinYEdge, 
			   NSMaxXEdge, NSMaxYEdge};
  // These names are role names not the actual colours
  NSColor *black = [NSColor controlDarkShadowColor];
  NSColor *dark = [NSColor controlShadowColor];
  NSColor *white = [NSColor controlLightHighlightColor];
  NSColor *colors[] = {black, black, white, white, dark, dark};

  if ([[NSView focusView] isFlipped] == YES)
    {
      return NSDrawColorTiledRects(border, clip, dn_sides, colors, 6);
    }
  else
    {
      return NSDrawColorTiledRects(border, clip, up_sides, colors, 6);
    }
}

- (NSRect) drawDarkBezel: (NSRect)border withClip: (NSRect)clip
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge,
			   NSMinXEdge, NSMaxYEdge, NSMaxXEdge, NSMinYEdge};
  NSRectEdge dn_sides[] = {NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge,
			   NSMinXEdge, NSMinYEdge, NSMaxXEdge, NSMaxYEdge};
  // These names are role names not the actual colours
  NSColor *black = [NSColor controlDarkShadowColor];
  NSColor *dark = [NSColor controlShadowColor];
  NSColor *light = [NSColor controlColor];
  NSColor *white = [NSColor controlLightHighlightColor];
  NSColor *colors[] = {white, white, dark, dark, black, black, light, light};
  NSRect rect;

  if ([[NSView focusView] isFlipped] == YES)
    {
      rect = NSDrawColorTiledRects(border, clip, dn_sides, colors, 8);
  
      [dark set];
      PSrectfill(NSMinX(border) + 1., NSMinY(border) - 2., 1., 1.);
      PSrectfill(NSMaxX(border) - 2., NSMaxY(border) + 1., 1., 1.);
    }
  else
    {
      rect = NSDrawColorTiledRects(border, clip, up_sides, colors, 8);
  
      [dark set];
      PSrectfill(NSMinX(border) + 1., NSMinY(border) + 1., 1., 1.);
      PSrectfill(NSMaxX(border) - 2., NSMaxY(border) - 2., 1., 1.);
    }
  return rect;
}

- (NSRect) drawDarkButton: (NSRect)border withClip: (NSRect)clip
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, 
			   NSMinXEdge, NSMaxYEdge}; 
  NSRectEdge dn_sides[] = {NSMaxXEdge, NSMaxYEdge, 
			   NSMinXEdge, NSMinYEdge}; 
  // These names are role names not the actual colours
  NSColor *black = [NSColor controlDarkShadowColor];
  NSColor *white = [NSColor controlHighlightColor];
  NSColor *colors[] = {black, black, white, white};

  if ([[NSView focusView] isFlipped] == YES)
    {
      return NSDrawColorTiledRects(border, clip, dn_sides, colors, 4);
    }
  else
    {
      return NSDrawColorTiledRects(border, clip, up_sides, colors, 4);
    }
}

- (NSRect) drawFramePhoto: (NSRect)border withClip: (NSRect)clip
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, 
			   NSMinXEdge, NSMaxYEdge, 
			   NSMaxXEdge, NSMinYEdge};
  NSRectEdge dn_sides[] = {NSMaxXEdge, NSMaxYEdge, 
			   NSMinXEdge, NSMinYEdge, 
			   NSMaxXEdge, NSMaxYEdge};
  // These names are role names not the actual colours
  NSColor *black = [NSColor controlDarkShadowColor];
  NSColor *dark = [NSColor controlShadowColor];
  NSColor *colors[] = {dark, dark, dark, dark, black,black};

  if ([[NSView focusView] isFlipped] == YES)
    {
      return NSDrawColorTiledRects(border, clip, dn_sides, colors, 6);
    }
  else
    {
      return NSDrawColorTiledRects(border, clip, up_sides, colors, 6);
    }
}

#if 1
- (NSRect) drawGradientBorder: (NSGradientType)gradientType 
                       inRect: (NSRect)border 
                     withClip: (NSRect)clip
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, 
			   NSMinXEdge, NSMaxYEdge};
  NSRectEdge dn_sides[] = {NSMaxXEdge, NSMaxYEdge, 
			   NSMinXEdge, NSMinYEdge};
  NSColor *black = [NSColor controlDarkShadowColor];
  NSColor *dark = [NSColor controlShadowColor];
  NSColor *light = [NSColor controlColor];
  NSColor **colors;
  NSColor *concaveWeak[] = {dark, dark, light, light};
  NSColor *concaveStrong[] = {black, black, light, light};
  NSColor *convexWeak[] = {light, light, dark, dark};
  NSColor *convexStrong[] = {light, light, black, black};
  NSRect rect;
  
  switch (gradientType)
    {
      case NSGradientConcaveWeak:
	colors = concaveWeak;
	break;
      case NSGradientConcaveStrong:
	colors = concaveStrong;
	break;
      case NSGradientConvexWeak:
	colors = convexWeak;
	break;
      case NSGradientConvexStrong:
	colors = convexStrong;
	break;
      case NSGradientNone:
      default:
	return border;
    }

  if ([[NSView focusView] isFlipped] == YES)
    {
      rect = NSDrawColorTiledRects(border, clip, dn_sides, colors, 4);
    }
  else
    {
      rect = NSDrawColorTiledRects(border, clip, up_sides, colors, 4);
    }
 
  return rect;
}

#else
// FIXME: I think this method is wrong.
- (NSRect) drawGradientBorder: (NSGradientType)gradientType 
                       inRect: (NSRect)cellFrame 
                     withClip: (NSRect)clip
{
  float   start_white = 0.0;
  float   end_white = 0.0;
  float   white = 0.0;
  float   white_step = 0.0;
  float   h, s, v, a;
  NSPoint p1, p2;
  NSColor *gray = nil;
  NSColor *darkGray = nil;
  NSColor *lightGray = nil;

  lightGray = [NSColor colorWithDeviceRed: NSLightGray 
                       green: NSLightGray 
                       blue: NSLightGray 
                       alpha:1.0];
  gray = [NSColor colorWithDeviceRed: NSGray 
                  green: NSGray 
                  blue: NSGray 
                  alpha:1.0];
  darkGray = [NSColor colorWithDeviceRed: NSDarkGray 
                      green: NSDarkGray 
                      blue: NSDarkGray 
                      alpha:1.0];

  switch (gradientType)
    {
      case NSGradientNone:
        return NSZeroRect;
        break;

      case NSGradientConcaveWeak:
        [gray getHue: &h saturation: &s brightness: &v alpha: &a];
        start_white = [lightGray brightnessComponent];
        end_white = [gray brightnessComponent];
        break;
        
      case NSGradientConvexWeak:
        [darkGray getHue: &h saturation: &s brightness: &v alpha: &a];
        start_white = [gray brightnessComponent];
        end_white = [lightGray brightnessComponent];
        break;
        
      case NSGradientConcaveStrong:
        [lightGray getHue: &h saturation: &s brightness: &v alpha: &a];
        start_white = [lightGray brightnessComponent];
        end_white = [darkGray brightnessComponent];
        break;
        
      case NSGradientConvexStrong:
        [darkGray getHue: &h saturation: &s brightness: &v alpha: &a];
        start_white = [darkGray brightnessComponent];
        end_white = [lightGray brightnessComponent];
        break;

      default:
        break;
    }

  white = start_white;
  white_step = fabs(start_white - end_white)
    / (cellFrame.size.width + cellFrame.size.height);

  // Start from top left
  p1 = NSMakePoint(cellFrame.origin.x,
    cellFrame.size.height + cellFrame.origin.y);
  p2 = NSMakePoint(cellFrame.origin.x, 
    cellFrame.size.height + cellFrame.origin.y);

  // Move by Y
  while (p1.y > cellFrame.origin.y)
    {
      [[NSColor 
        colorWithDeviceHue: h saturation: s brightness: white alpha: 1.0] set];
      [NSBezierPath strokeLineFromPoint: p1 toPoint: p2];
      
      if (start_white > end_white)
        white -= white_step;
      else
        white += white_step;

      p1.y -= 1.0;
      if (p2.x < (cellFrame.size.width + cellFrame.origin.x))
        p2.x += 1.0;
      else
        p2.y -= 1.0;
    }
      
  // Move by X
  while (p1.x < (cellFrame.size.width + cellFrame.origin.x))
    {
      [[NSColor 
        colorWithDeviceHue: h saturation: s brightness: white alpha: 1.0] set];
      [NSBezierPath strokeLineFromPoint: p1 toPoint: p2];
      
      if (start_white > end_white)
        white -= white_step;
      else
        white += white_step;

      p1.x += 1.0;
      if (p2.x >= (cellFrame.size.width + cellFrame.origin.x))
        p2.y -= 1.0;
      else
        p2.x += 1.0;
    }

  return NSZeroRect;
}

#endif

- (NSRect) drawGrayBezel: (NSRect)border withClip: (NSRect)clip
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge,
			   NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge};
  NSRectEdge dn_sides[] = {NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge,
			     NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge};
  // These names are role names not the actual colours
  NSColor *black = [NSColor controlDarkShadowColor];
  NSColor *dark = [NSColor controlShadowColor];
  NSColor *light = [NSColor controlColor];
  NSColor *white = [NSColor controlLightHighlightColor];
  NSColor *colors[] = {white, white, dark, dark,
		       light, light, black, black};
  NSRect rect;

  if ([[NSView focusView] isFlipped] == YES)
    {
      rect = NSDrawColorTiledRects(border, clip, dn_sides, colors, 8);
      [dark set];
      PSrectfill(NSMinX(border) + 1., NSMaxY(border) - 2., 1., 1.);
      PSrectfill(NSMaxX(border) - 2., NSMinY(border) + 1., 1., 1.);
    }
  else
    {
      rect = NSDrawColorTiledRects(border, clip, up_sides, colors, 8);
      [dark set];
      PSrectfill(NSMinX(border) + 1., NSMinY(border) + 1., 1., 1.);
      PSrectfill(NSMaxX(border) - 2., NSMaxY(border) - 2., 1., 1.);
    }
  return rect;
}

- (NSRect) drawGroove: (NSRect)border withClip: (NSRect)clip
{
  // go clockwise from the top twice -- makes the groove come out right
  NSRectEdge up_sides[] = {NSMaxYEdge, NSMaxXEdge, NSMinYEdge, NSMinXEdge,
			   NSMaxYEdge, NSMaxXEdge, NSMinYEdge, NSMinXEdge};
  NSRectEdge dn_sides[] = {NSMinYEdge, NSMaxXEdge, NSMaxYEdge, NSMinXEdge,
			   NSMinYEdge, NSMaxXEdge, NSMaxYEdge, NSMinXEdge};
  // These names are role names not the actual colours
  NSColor *dark = [NSColor controlShadowColor];
  NSColor *white = [NSColor controlLightHighlightColor];
  NSColor *colors[] = {dark, white, white, dark,
		       white, dark, dark, white};

  if ([[NSView focusView] isFlipped] == YES)
    {
      return NSDrawColorTiledRects(border, clip, dn_sides, colors, 8);
    }
  else
    {
      return NSDrawColorTiledRects(border, clip, up_sides, colors, 8);
    }
}

- (NSRect) drawLightBezel: (NSRect)border withClip: (NSRect)clip
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge, 
  			   NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge};
  NSRectEdge dn_sides[] = {NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge, 
			   NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge};
  // These names are role names not the actual colours
  NSColor *dark = [NSColor controlShadowColor];
  NSColor *light = [NSColor controlColor];
  NSColor *white = [NSColor controlLightHighlightColor];
  NSColor *colors[] = {white, white, dark, dark,
		       light, light, dark, dark};

  if ([[NSView focusView] isFlipped] == YES)
    {
      return NSDrawColorTiledRects(border, clip, dn_sides, colors, 8);
    }
  else
    {
      return NSDrawColorTiledRects(border, clip, up_sides, colors, 8);
    }
}

- (NSRect) drawWhiteBezel: (NSRect)border withClip: (NSRect)clip
{
  NSRectEdge up_sides[] = {NSMaxYEdge, NSMaxXEdge, NSMinYEdge, NSMinXEdge,
  			   NSMaxYEdge, NSMaxXEdge, NSMinYEdge, NSMinXEdge};
  NSRectEdge dn_sides[] = {NSMinYEdge, NSMaxXEdge, NSMaxYEdge, NSMinXEdge, 
  			     NSMinYEdge, NSMaxXEdge, NSMaxYEdge, NSMinXEdge};
  // These names are role names not the actual colours
  NSColor *dark = [NSColor controlShadowColor];
  NSColor *light = [NSColor controlColor];
  NSColor *white = [NSColor controlLightHighlightColor];
  NSColor *colors[] = {dark, white, white, dark,
		       dark, light, light, dark};

  if ([[NSView focusView] isFlipped] == YES)
    {
      return NSDrawColorTiledRects(border, clip, dn_sides, colors, 8);
    }
  else
    {
      return NSDrawColorTiledRects(border, clip, up_sides, colors, 8);
    }
}

- (void) drawRoundBezel: (NSRect)cellFrame withColor: (NSColor*)backgroundColor
{
  NSBezierPath *p;
  NSPoint point;
  CGFloat radius;

  // make smaller than enclosing frame
  cellFrame = NSInsetRect(cellFrame, 4, 4);
  radius = cellFrame.size.height / 2.0;
  point = cellFrame.origin;
  point.x += radius;
  point.y += radius - 0.5;

  // Draw initial path to enclose the button...
  // left half-circle
  p = [NSBezierPath bezierPath];
  [p appendBezierPathWithArcWithCenter: point
				radius: radius
			    startAngle: 90.0
			      endAngle: 270.0];

  // line to first point and right halfcircle
  point.x += cellFrame.size.width - cellFrame.size.height;
  [p appendBezierPathWithArcWithCenter: point
				radius: radius
			    startAngle: 270.0
			      endAngle: 90.0];
  [p closePath];

  // fill with background color
  [backgroundColor set];
  [p fill];

  // and stroke rounded button
  [[NSColor shadowColor] set];
  [p stroke];

  // Add highlights...
  point = cellFrame.origin;
  point.x += radius - 0.5;
  point.y += radius - 0.5;
  p = [NSBezierPath bezierPath];
  [p setLineWidth: 1.0];
  [p appendBezierPathWithArcWithCenter: point
				radius: radius
			    startAngle: 135.0
			      endAngle: 270.0];

  // line to first point and right halfcircle
  point.x += cellFrame.size.width - cellFrame.size.height;
  [p appendBezierPathWithArcWithCenter: point
				radius: radius
			    startAngle: 270.0
			      endAngle: 315.0];
  [[NSColor controlLightHighlightColor] set];
  [p stroke];
}

- (void) drawCircularBezel: (NSRect)cellFrame
		 withColor: (NSColor*)backgroundColor
{
  // make smaller so that it does not touch frame
  NSBezierPath *oval;

  oval = [NSBezierPath bezierPathWithOvalInRect: NSInsetRect(cellFrame, 1, 1)];

  // fill oval with background color
  [backgroundColor set];
  [oval fill];

  // and stroke rounded button
  [[NSColor shadowColor] set];
  [oval stroke];
}

@end



@implementation	GSTheme (LowLevelDrawing)

- (void) fillHorizontalRect: (NSRect)rect
		  withImage: (NSImage*)image
		   fromRect: (NSRect)source
		    flipped: (BOOL)flipped
{
  NSGraphicsContext	*ctxt = GSCurrentContext();
  NSBezierPath		*path;
  unsigned		repetitions;
  float			remainder;
  unsigned		count;
  NSPoint		p;
  float			y;

  if (rect.size.width <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] rect width is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (rect.size.height <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] rect height is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (source.size.width <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] source width is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (source.size.height <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] source height is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (image == nil)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] image is nil",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];

  ctxt = GSCurrentContext();
  DPSgsave (ctxt);
  path = [NSBezierPath bezierPathWithRect: rect];
  [path addClip];
  repetitions = rect.size.width / source.size.width;
  remainder = rect.size.width - repetitions * source.size.width;
  y = rect.origin.y;

  if (flipped) y = rect.origin.y + rect.size.height;
  
  for (count = 0; count < repetitions; count++)
    {
      p = NSMakePoint (rect.origin.x + count * source.size.width, y);
      [image compositeToPoint: p
		     fromRect: source
		    operation: NSCompositeSourceOver];
    }
  if (remainder > 0)
    {
      p = NSMakePoint (rect.origin.x + repetitions * source.size.width, y);
      source.size.width = remainder;
      [image compositeToPoint: p
		     fromRect: source
		    operation: NSCompositeSourceOver];
    }
  DPSgrestore (ctxt);	
}

- (void) fillRect: (NSRect)rect
withRepeatedImage: (NSImage*)image
	 fromRect: (NSRect)source
	   center: (BOOL)center
{
  NSGraphicsContext	*ctxt;
  NSBezierPath		*path;
  NSSize		size;
  unsigned		xrepetitions;
  unsigned		yrepetitions;
  unsigned		x;
  unsigned		y;

  if (rect.size.width <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] rect width is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (rect.size.height <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] rect height is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (source.size.width <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] source width is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (source.size.height <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] source height is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (image == nil)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] image is nil",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];

  ctxt = GSCurrentContext ();
  DPSgsave (ctxt);
  path = [NSBezierPath bezierPathWithRect: rect];
  [path addClip];
  size = [image size];
  xrepetitions = (rect.size.width / size.width) + 1;
  yrepetitions = (rect.size.height / size.height) + 1;

  for (x = 0; x < xrepetitions; x++)
    {
      for (y = 0; y < yrepetitions; y++)
	{
	  NSPoint p;

	  p = NSMakePoint (rect.origin.x + x * size.width,
	    rect.origin.y + y * size.height);
	  [image compositeToPoint: p
			 fromRect: source
			operation: NSCompositeSourceOver];
      }
  }
  DPSgrestore (ctxt);	
}

- (NSRect) fillRect: (NSRect)rect
	  withTiles: (GSDrawTiles*)tiles
	 background: (NSColor*)color
{
  return [self fillRect: rect
	      withTiles: tiles
	     background: color
	      fillStyle: [tiles fillStyle]];
}

- (NSRect) fillRect: (NSRect)rect
	  withTiles: (GSDrawTiles*)tiles
{
  return [self fillRect: rect
	      withTiles: tiles
	     background: [NSColor clearColor]
	      fillStyle: [tiles fillStyle]];
}

- (NSRect) fillRect: (NSRect)rect
	  withTiles: (GSDrawTiles*)tiles
	 background: (NSColor*)color
	  fillStyle: (GSThemeFillStyle)style
{
  if (rect.size.width <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] rect width is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (rect.size.height <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] rect height is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (tiles == nil)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] tiles is nil",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];

  return [tiles fillRect: rect background: color fillStyle: style];
}

- (void) fillVerticalRect: (NSRect)rect
		withImage: (NSImage*)image
		 fromRect: (NSRect)source
		  flipped: (BOOL)flipped
{
  NSGraphicsContext	*ctxt;
  NSBezierPath		*path;
  unsigned		repetitions;
  float			remainder;
  unsigned		count;
  NSPoint		p;

  if (rect.size.width <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] rect width is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (rect.size.height <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] rect height is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (source.size.width <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] source width is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (source.size.height <= 0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] source height is not positive",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  if (image == nil)
    [NSException raise: NSInvalidArgumentException
		format: @"[%@-%@] image is nil",
      NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
  ctxt = GSCurrentContext();
  DPSgsave (ctxt);
  path = [NSBezierPath bezierPathWithRect: rect];
  [path addClip];
  repetitions = rect.size.height / source.size.height;
  remainder = rect.size.height - repetitions * source.size.height;

  if (flipped)
    {
      for (count = 0; count < repetitions; count++)
	{
	  p = NSMakePoint (rect.origin.x,
	    rect.origin.y + rect.size.height - count * source.size.height);
	  [image compositeToPoint: p
			 fromRect: source
			operation: NSCompositeSourceOver];
	}
      if (remainder > 0)
	{
	  p = NSMakePoint (rect.origin.x,
	    rect.origin.y + rect.size.height
	    - repetitions * source.size.height);
	  source.origin.y += source.size.height - remainder;
	  source.size.height = remainder;
	  [image compositeToPoint: p
			 fromRect: source
			operation: NSCompositeSourceOver];
	}
    }
  else
    {
      for (count = 0; count < repetitions; count++)
	{
	  p = NSMakePoint (rect.origin.x,
	    rect.origin.y + count * source.size.height);
	  [image compositeToPoint: p
			 fromRect: source
			operation: NSCompositeSourceOver];
	}
      if (remainder > 0)
	{
	  p = NSMakePoint (rect.origin.x,
	    rect.origin.y + repetitions * source.size.height);
	  source.size.height = remainder;
	  [image compositeToPoint: p
			 fromRect: source
			operation: NSCompositeSourceOver];
	}
    }
  DPSgrestore (ctxt);	
}

@end



@implementation	GSDrawTiles
- (id) copyWithZone: (NSZone*)zone
{
  GSDrawTiles	*c = (GSDrawTiles*)NSCopyObject(self, 0, zone);
  unsigned	i;

  for (i = 0; i < 9; i++)
    {
      c->images[i] = [images[i] copyWithZone: zone];
    }
  c->style = style;
  return c;
}

- (void) dealloc
{
  unsigned	i;

  for (i = 0; i < 9; i++)
    {
      RELEASE(images[i]);
    }
  [super dealloc];
}

- (NSString*) description
{
  NSMutableString	*d = [[[super description] mutableCopy] autorelease];
  unsigned		i;

  for (i = 0; i < 9; i++)
    {
      [d appendFormat: @"\n  %@ %@", NSStringFromRect(rects[i]), images[i]];
    }

  return d;
}

/**
 * Simple initialiser, assume the single image is split into nine equal tiles.
 * If the image size is not divisible by three, the corners are made equal
 * in size and the central parts slightly smaller.
 */
- (id) initWithImage: (NSImage*)image
{
  NSSize	s = [image size];

  return [self initWithImage: image
		  horizontal: s.width / 3.0
		    vertical: s.height / 3.0];
}

- (id) initWithNinePatchImage: (NSImage*)image
{
  int i;
  CGFloat r,g,b,a;
  int x1 = -1; // x1, x2, y1, y2, are in flipped coordinates
  int x2 = -1; // 0,0 is the top-left pixel
  int y1 = -1;
  int y2 = -1;
  NSSize s = [image size];
  NSBitmapImageRep* rep = [[image representations] objectAtIndex: 0];

  for (i = 0; i < s.width; i++)
    {
      NSColor	*pixelColor = [rep colorAtX: i y: 0];

      [pixelColor getRed: &r green: &g blue: &b alpha: &a];
      if (a > 0 && x1 == -1)
        {
          x1 = i;
        }
      else if (a == 0 && x1 != -1)
        {
          x2 = i - 1;
          break;
        }
    }

  for (i = 0; i < s.height; i++)
    {
      NSColor	*pixelColor = [rep colorAtX: 0 y: i];

      [pixelColor getRed: &r green: &g blue: &b alpha: &a];
      if (a > 0 && y1 == -1)
        {
          y1 = i;
        }
      else if (a == 0 && y1 != -1)
        {
          y2 = i - 1;
          break;
        }
    }

  scaleFactor  = 1.0f;
  style = GSThemeFillStyleScaleAll;

  // These are all in _unflipped_ coordinates
  rects[TileTL] = NSMakeRect(1, s.height - y1, x1 - 1, y1 - 1);
  rects[TileTM] = NSMakeRect(x1, s.height - y1, 1 + x2 - x1, y1 - 1);
  rects[TileTR] = NSMakeRect(x2 + 1, s.height - y1, s.width - x2 - 2, y1 - 1);
  rects[TileCL] = NSMakeRect(1, s.height - y2 - 1, x1 - 1, 1 + y2 - y1);
  rects[TileCM] = NSMakeRect(x1, s.height - y2 - 1, 1 + x2 - x1, 1 + y2 - y1);
  rects[TileCR] = NSMakeRect(x2 + 1, s.height - y2 - 1, s.width - x2 - 2, 1 + y2 - y1);
  rects[TileBL] = NSMakeRect(1, 1, x1 - 1, s.height - y2 - 2);
  rects[TileBM] = NSMakeRect(x1, 1, 1 + x2 - x1, s.height - y2 - 2);
  rects[TileBR] = NSMakeRect(x2 + 1, 1, s.width - x2 - 2, s.height - y2 - 2);

  // Measure the content rect (the right and bottom edges of the nine-patch
  // data)

  x1 = -1;
  x2 = -1;
  y1 = -1;
  y2 = -1;

  for (i = 0; i < s.width; i++)
    {
      NSColor	*pixelColor = [rep colorAtX: i y: s.height - 1];

      [pixelColor getRed: &r green: &g blue: &b alpha: &a];
      if ((a == 1 && r == 0 && g == 0 && b == 0) && x1 == -1)
        {
          x1 = i;
        }
      else if (!(a == 1 && r == 0 && g == 0 && b == 0) && x1 != -1)
        {
          x2 = i - 1;
          break;
        }
    }

  for (i = 0; i < s.height; i++)
    {
      NSColor	*pixelColor = [rep colorAtX: s.width - 1 y: i];

      [pixelColor getRed: &r green: &g blue: &b alpha: &a];
      if ((a == 1 && r == 0 && g == 0 && b == 0) && y1 == -1)
        {
          y1 = i;
        }
      else if (!(a == 1 && r == 0 && g == 0 && b == 0) && y1 != -1)
        {
          y2 = i - 1;
          break;
        }
    }

  // Specifying the content rect part of the nine-patch image is optional
  // ; if either the horizontal or vertical information is missing, use the
  // geometry from rects[TileCM]

  // contentRect is in unflipped coordinates, like rects[]

  if (x1 == -1)
    {
      contentRect.origin.x = rects[TileCM].origin.x;
      contentRect.size.width = rects[TileCM].size.width;
    }
  else
    {
      contentRect.origin.x = x1;
      contentRect.size.width = 1 + x2 - x1;
    }

  if (y1 == -1)
    {
      contentRect.origin.y = rects[TileCM].origin.y;
      contentRect.size.height = rects[TileCM].size.height;
    }
  else
    {
      contentRect.origin.y = s.height - y2 - 1; 
      contentRect.size.height = 1 + y2 - y1;
    }

  // Measure the layout rect (the right and bottom edges of the nine-patch
  // data which  _isn't_ red pixels)

  x1 = -1;
  x2 = -1;
  y1 = -1;
  y2 = -1;

  for (i = 1; i < (s.width - 1); i++)
    {
      NSColor	*pixelColor = [rep colorAtX: i y: s.height - 1];

      [pixelColor getRed: &r green: &g blue: &b alpha: &a];
      if (!(a == 1 && r == 1 && g == 0 && b == 0) && x1 == -1)
        {
          x1 = i;
        }
      else if ((a == 1 && r == 1 && g == 0 && b == 0) && x1 != -1)
        {
          x2 = i - 1;
          break;
        }
    }

  for (i = 1; i < (s.height - 1); i++)
    {
      NSColor	*pixelColor = [rep colorAtX: s.width - 1 y: i];

      [pixelColor getRed: &r green: &g blue: &b alpha: &a];
      if (!(a == 1 && r == 1 && g == 0 && b == 0) && y1 == -1)
        {
          y1 = i;
        }
      else if ((a == 1 && r == 1 && g == 0 && b == 0) && y1 != -1)
        {
          y2 = i - 1;
          break;
        }
    }


  if (x2 == -1)
    {
      x2 = s.width - 2;
    }

  if (y2 == -1)
    {
      y2 = s.height - 2;
    }

  layoutRect = NSMakeRect(x1, s.height - y2 - 1, 1 + x2 - x1, 1 + y2 - y1);

  [self validateTilesSizeWithImage: image];
  return self;
}

- (NSImage*) extractImageFrom: (NSImage*) image withRect: (NSRect) rect
{
  NSImage *img = [[NSImage alloc] initWithSize: rect.size];

  [img lockFocus];
  [image drawAtPoint: NSMakePoint(0, 0)
	    fromRect: rect
	   operation: NSCompositeSourceOver
	    fraction: 1.0];
  [img unlockFocus];
  [img TIFFRepresentation]; // creates a proper NSBitmapImageRep
  return [img autorelease];
}

- (void) validateTilesSizeWithImage: (NSImage*)image
{
  int i;

  originalRectCM = rects[TileCM];

  for (i = 0; i < 9; i++)
    {
      if (rects[i].origin.x < 0.0 || rects[i].origin.y < 0.0
        || rects[i].size.width <= 0.0 || rects[i].size.height <= 0.0)
        {
          images[i] = nil;
          rects[i] = NSZeroRect;
        }
      else
        {
          images[i]
	    = [[self extractImageFrom: image withRect: rects[i]] retain];
	  // FIXME: This makes no sense to me, why not leave the
	  // rect origins at their original values?
          rects[i].origin.x = 0;
          rects[i].origin.y = 0;
        }
    }

    if (contentRect.origin.x < 0.0 || contentRect.origin.y < 0.0
      || contentRect.size.width <= 0.0 || contentRect.size.height <= 0.0)
      {
        contentRect = rects[TileCM];
      }
}

- (id) initWithImage: (NSImage*)image horizontal: (float)x vertical: (float)y
{
  NSSize s = [image size];

  x = floor(x);
  y = floor(y);

  scaleFactor  = 1.0;

  rects[TileTL] = NSMakeRect(0.0, s.height - y, x, y);
  rects[TileTM] = NSMakeRect(x, s.height - y, s.width - 2.0 * x, y);
  rects[TileTR] = NSMakeRect(s.width - x, s.height - y, x, y);
  rects[TileCL] = NSMakeRect(0.0, y, x, s.height - 2.0 * y);
  rects[TileCM] = NSMakeRect(x, y, s.width - 2.0 * x, s.height - 2.0 * y);
  rects[TileCR] = NSMakeRect(s.width - x, y, x, s.height - 2.0 * y);
  rects[TileBL] = NSMakeRect(0.0, 0.0, x, y);
  rects[TileBM] = NSMakeRect(x, 0.0, s.width - 2.0 * x, y);
  rects[TileBR] = NSMakeRect(s.width - x, 0.0, x, y);
  contentRect = rects[TileCM];

  style = GSThemeFillStyleNone;

  [self validateTilesSizeWithImage: image];
  return self;
}

- (void) scaleTo: (float)scale
{
  unsigned	i;
  NSSize	s;

  if (scale == scaleFactor)
    {
      return;
    }

  [images[0] setScalesWhenResized: YES];
  s = [images[0] size];
  s.width *= scale;
  s.height *= scale;
  [images[0] setSize: s];
  rects[0].size.height *= scale;
  rects[0].size.width *= scale;
  rects[0].origin.x *= scale;
  rects[0].origin.y *= scale;
  for (i = 1; i < 9; i++)
    {
      unsigned	j;

      for (j = 0; j < i; j++)
	{
	  if (images[i] == images[j])
	    {
	      break;
	    }
	}
      if (j == i)
	{
	  [images[i] setScalesWhenResized: YES];
	  s = [images[i] size];
	  s.width *= scale;
	  s.height *= scale;
	  [images[i] setSize: s];
	}
      rects[i].size.height *= scale;
      rects[i].size.width *= scale;
      rects[i].origin.x *= scale;
      rects[i].origin.y *= scale;
    }
  contentRect.size.height *= scale;
  contentRect.size.width *= scale;
  contentRect.origin.x *= scale;
  contentRect.origin.y *= scale;
}

- (NSRect) fillRect: (NSRect)rect
         background: (NSColor*)color
{
  return [self fillRect: rect background: color fillStyle: style];
}

- (NSRect) fillRect: (NSRect)rect
         background: (NSColor*)color
          fillStyle: (GSThemeFillStyle)aStyle
{
  if (color == nil)
    {
      [[NSColor redColor] set];
    }
  else
    {
      [color set];
    }
//  NSRectFill(rect);

  switch (aStyle)
    {
      case GSThemeFillStyleNone:
           return [self noneStyleFillRect: rect];
      case GSThemeFillStyleScale:
           return [self scaleStyleFillRect: rect];
      case GSThemeFillStyleRepeat:
           return [self repeatStyleFillRect: rect];
      case GSThemeFillStyleCenter:
           return [self centerStyleFillRect: rect];
      case GSThemeFillStyleMatrix:
           return [self matrixStyleFillRect: rect];
      case GSThemeFillStyleScaleAll:
           return [self scaleAllStyleFillRect: rect];
    }

  return NSZeroRect;
}

- (NSSize) computeTotalTilesSize
{
  NSSize tsz;

  tsz.width = rects[TileTL].size.width + rects[TileTR].size.width;
  if (images[TileTM] != nil)
    {
      tsz.width += rects[TileTM].size.width;
    }
  tsz.height = rects[TileTL].size.height + rects[TileBL].size.height;
  if (images[TileCL] != nil)
    {
      tsz.height += rects[TileCL].size.height;
    }
  return tsz;
}

- (GSThemeMargins) themeMargins
{
  NSRect cm = originalRectCM;
  GSThemeMargins margins;
  
  margins.left = rects[TileCL].size.width;
  margins.right = rects[TileCR].size.width;
  margins.top = rects[TileTM].size.height;
  margins.bottom = rects[TileBM].size.height;

  // Adjust for contentRect != cm

  margins.left += (contentRect.origin.x - cm.origin.x);
  margins.bottom += (contentRect.origin.y - cm.origin.y);

  margins.right += (NSMaxX(cm) - NSMaxX(contentRect));
  margins.top += (NSMaxY(cm) - NSMaxY(contentRect));
  
  return margins;
}

- (NSRect) contentRectForRect: (NSRect)rect
		    isFlipped: (BOOL)flipped
{
  GSThemeMargins margins = [self themeMargins];
  
  rect.origin.x += margins.left;
  rect.origin.y += flipped ? margins.top : margins.bottom;
    
  rect.size.width -= (margins.left + margins.right);
  rect.size.height -= (margins.top + margins.bottom);
  
  return rect;
}

- (NSRect) noneStyleFillRect: (NSRect)rect
{
  NSRect inFill = [self contentRectForRect: rect isFlipped: NO];
  [self repeatFillRect: rect];
  [self drawCornersRect: rect];
  return inFill;
}

- (NSRect) centerStyleFillRect: (NSRect)rect
{
  BOOL flipped = [[GSCurrentContext() focusView] isFlipped];

  NSRect r = rects[TileCM];
  NSRect inFill = [self contentRectForRect: rect isFlipped: flipped];
  [self repeatFillRect: rect];
  [self drawCornersRect: rect];

  r.origin.x = inFill.origin.x + (inFill.size.width - r.size.width) / 2;
  r.origin.y = inFill.origin.y + (inFill.size.height - r.size.height) / 2;

  if (flipped)
    {
      r.origin.y += r.size.height; 
    }

  [images[TileCM] compositeToPoint: r.origin
                          fromRect: rects[TileCM]
                         operation: NSCompositeSourceOver];

  return inFill;
}

- (NSRect) repeatStyleFillRect: (NSRect)rect
{
  BOOL flipped = [[GSCurrentContext() focusView] isFlipped];

  NSSize tsz = [self computeTotalTilesSize];
  NSRect inFill = [self contentRectForRect: rect isFlipped: flipped];
  [self repeatFillRect: rect];
  [self drawCornersRect: rect];

  [[GSTheme theme] fillRect: inFill
    withRepeatedImage: images[TileCM]
    fromRect: rects[TileCM]
    center: !flipped];
  
  NSLog(@"rect %@ too small for tiles %@",
    NSStringFromSize(rect.size), NSStringFromSize(tsz));

  return inFill;
}

- (NSRect) scaleStyleFillRect: (NSRect)rect
{
  BOOL flipped = [[GSCurrentContext() focusView] isFlipped];

  NSRect inFill = [self contentRectForRect: rect isFlipped: flipped];

  NSImage *im = [images[TileCM] copy];
  NSRect r =  rects[TileCM];
  NSSize s = [images[TileCM] size];
  NSPoint p = inFill.origin;
  float sx = inFill.size.width / r.size.width;
  float sy = inFill.size.height / r.size.height;

  [self repeatFillRect: rect];
  [self drawCornersRect: rect];

  r.size.width = inFill.size.width;
  r.size.height = inFill.size.height;
  r.origin.x *= sx;
  r.origin.y *= sy;
  s.width *= sx;
  s.height *= sy;

  if (flipped)
    {
      p.y += inFill.size.height;
    }
  
  [im setScalesWhenResized: YES];
  [im setSize: s];
  [im compositeToPoint: p
              fromRect: r
             operation: NSCompositeSourceOver];
  RELEASE(im);

  return inFill;
}

- (NSRect) scaleAllStyleFillRect: (NSRect)rect
{
  BOOL flipped = [[GSCurrentContext() focusView] isFlipped];
  NSSize cls = rects[TileCL].size;
  NSSize bms = rects[TileBM].size;
  NSSize crs = rects[TileCR].size;
  NSSize tms = rects[TileTM].size;
  NSImage *img;
  NSRect imgRect;

  NSRect inFill = [self contentRectForRect: rect isFlipped: flipped];
  [self scaleFillRect: rect];
  [self drawCornersRect: rect];

  // Draw center scaled

  img = images[TileCM];
  imgRect = NSMakeRect(0, 0,
    rect.size.width - cls.width - crs.width,
    rect.size.height - tms.height - bms.height);

  if (imgRect.size.width > 0 && imgRect.size.height > 0)
    {
      NSPoint p;

      [img setScalesWhenResized: YES];
      [img setSize: imgRect.size];
      p = NSMakePoint(rect.origin.x + cls.width,
	rect.origin.y + bms.height);
      if (flipped)
        {
          p.y = rect.origin.y + rect.size.height - bms.height;
        }
      [img compositeToPoint: p
                   fromRect: imgRect
                  operation: NSCompositeSourceOver]; 
    }

  return inFill;
}

- (NSRect) matrixStyleFillRect: (NSRect)rect
{
  NSSize tsz = [self computeTotalTilesSize];
  NSRect grid = NSZeroRect;
  float x;
  float y;
  float space = 3.0;
  float scale;
  BOOL flipped = [[GSCurrentContext() focusView] isFlipped];

  if (images[TileTM] == nil)
    {
      grid.size.width = tsz.width + space * 3.0;
    }
  else
    {
      grid.size.width = tsz.width + space * 4.0;
    }
  scale = rect.size.width / grid.size.width;

  if (images[TileCL] == nil)
    {
      grid.size.height = tsz.height + space * 3.0;
    }
  else
    {
      grid.size.height = tsz.height + space * 4.0;
    }

  if ((rect.size.height / grid.size.height) < scale)
    {
      scale = rect.size.height / grid.size.height;
    }

  if (floor(scale) >= 1)
    {
      scale = floor(scale);
    }

  if (scale != 1)
    {
      // We need to scale the tiles down to fit. 
      grid.size.width *= scale;
      grid.size.height *= scale;
      space *= scale;
      [self scaleTo: scale];
    }

  grid.origin.x = rect.origin.x + (rect.size.width - grid.size.width) / 2;
  x = grid.origin.x;
  if (flipped)
    {
      grid.origin.y = NSMaxY(rect) - (rect.size.height - grid.size.height) / 2;
      y = NSMaxY(grid);
    }
  else
    {
      grid.origin.y = rect.origin.y + (rect.size.height - grid.size.height) / 2;
      y = grid.origin.y;
    }

  // Draw bottom row
  if (flipped)
    {
      y -= (rects[TileBL].size.height + space);
    }
  else
    {
      y += space;
    }
  [images[TileBL] compositeToPoint: NSMakePoint(x, y)
                                 fromRect: rects[TileBL]
                                operation: NSCompositeSourceOver];
  x += rects[TileBL].size.width + space;
  if (images[TileBM] != nil)
    {
      [images[TileBM] compositeToPoint: NSMakePoint(x, y)
                                     fromRect: rects[TileBM]
	                            operation: NSCompositeSourceOver];
      x += rects[TileBM].size.width + space;
    }
  [images[TileBR] compositeToPoint: NSMakePoint(x, y)
	                         fromRect: rects[TileBR]
                                operation: NSCompositeSourceOver];
  if (!flipped)
    {
      y += rects[TileBL].size.height;
    }

  // Draw middle row
  if (images[TileCL] != nil)
    {
      x = grid.origin.x;
      if (flipped)
        {
          y -= (rects[TileCL].size.height + space);
	}
      else
	{
          y += space;
	}
      [images[TileCL] compositeToPoint: NSMakePoint(x, y)
                                     fromRect: rects[TileCL]
                                    operation: NSCompositeSourceOver];
      x += rects[TileCL].size.width + space;
      if (images[TileCM] != nil)
        {
	  [images[TileCM] compositeToPoint: NSMakePoint(x, y)
                                         fromRect: rects[TileCM]
                                        operation: NSCompositeSourceOver];
          x += rects[TileCM].size.width + space;
        }
      [images[TileCR] compositeToPoint: NSMakePoint(x, y)
                                     fromRect: rects[TileCR]
                                    operation: NSCompositeSourceOver];
      if (!flipped)
        {
          y += rects[TileCL].size.height;
        }
    }

  // Draw top row
  x = grid.origin.x;
  if (flipped)
    {
      y -= (rects[TileTL].size.height + space);
    }
  else
    {
      y += space;
    }
  [images[TileTL] compositeToPoint: NSMakePoint(x, y)
                                 fromRect: rects[TileTL]
                                operation: NSCompositeSourceOver];
  x += rects[TileTL].size.width + space;
  if (images[TileTM] != nil)
    {
      [images[TileTM] compositeToPoint: NSMakePoint(x, y)
                                     fromRect: rects[TileTM]
                                    operation: NSCompositeSourceOver];
      x += rects[TileTM].size.width + space;
    }
  [images[TileTR] compositeToPoint: NSMakePoint(x, y)
                                 fromRect: rects[TileTR]
                                 operation: NSCompositeSourceOver];
  if (scale != 1)
    {
      [self scaleTo: 1.0f/scale];
    }
  return NSZeroRect;
}

- (void) repeatFillRect: (NSRect)rect
{
  BOOL flipped = [[GSCurrentContext() focusView] isFlipped];

  NSSize tls = rects[TileTL].size;
  NSSize tms = rects[TileTM].size;
  NSSize trs = rects[TileTR].size;
  NSSize cls = rects[TileCL].size;
  NSSize crs = rects[TileCR].size;
  NSSize bls = rects[TileBL].size;
  NSSize bms = rects[TileBM].size;
  NSSize brs = rects[TileBR].size;

  // Draw Top-Middle image repeated
  if (rect.size.width > tls.width + trs.width && tms.height > 0)
    {
      float y = rect.origin.y + rect.size.height - tms.height;

      if (flipped)
        {
          y = rect.origin.y;
        }

      [[GSTheme theme] fillHorizontalRect:
        NSMakeRect (rect.origin.x + tls.width, y,
	    rect.size.width - tls.width - trs.width,
	    tms.height)
	withImage: images[TileTM]
	fromRect: rects[TileTM]
	flipped: flipped];
    }

  // Draw Bottom-Middle image repeated
  if (rect.size.width > bls.width + brs.width && bms.height > 0)
    {
      float y = rect.origin.y;

      if (flipped)
        {
          y = rect.origin.y + rect.size.height - bms.height;
        }
      [[GSTheme theme] fillHorizontalRect:
	NSMakeRect (rect.origin.x + bls.width, y,
	      rect.size.width - bls.width - brs.width,
	      bms.height)
	withImage: images[TileBM]
	fromRect: rects[TileBM]
	flipped: flipped];
    }

  // Draw Center-Left image repeated

  if (rect.size.height > bls.height + tls.height && cls.width > 0)
    {
      [[GSTheme theme] fillVerticalRect:
	NSMakeRect (rect.origin.x,
	      rect.origin.y + bls.height,
	      cls.width,
	      rect.size.height - bls.height - tls.height)
	withImage: images[TileCL]
	fromRect: rects[TileCL]
	flipped: flipped];
    }

  // Draw Center-Right image repeated

  if (rect.size.height > brs.height + trs.height && crs.width > 0)
    {
      [[GSTheme theme] fillVerticalRect:
	NSMakeRect (rect.origin.x + rect.size.width - crs.width,
	      rect.origin.y + brs.height,
	      crs.width,
	      rect.size.height - brs.height - trs.height)
	withImage: images[TileCR]
	fromRect: rects[TileCR]
	flipped: flipped];
    }    
}

- (void) scaleFillRect: (NSRect)rect
{
  BOOL flipped = [[GSCurrentContext() focusView] isFlipped];
  NSImage *img;
  NSRect imgRect;
  NSPoint p;

  NSSize tls = rects[TileTL].size;
  NSSize tms = rects[TileTM].size;
  NSSize trs = rects[TileTR].size;
  NSSize cls = rects[TileCL].size;
  NSSize crs = rects[TileCR].size;
  NSSize bls = rects[TileBL].size;
  NSSize bms = rects[TileBM].size;
  NSSize brs = rects[TileBR].size;

  // Draw Top-Middle image scaled

  img = images[TileTM];
  imgRect = NSMakeRect(0, 0,
    rect.size.width - tls.width - trs.width, tms.height);
  if (imgRect.size.width > 0 && imgRect.size.height > 0)
    {
      [img setScalesWhenResized: YES];
      [img setSize: imgRect.size];
      p = NSMakePoint(rect.origin.x + tls.width,
	rect.origin.y + rect.size.height - tms.height);
      if (flipped)
        {
          p.y = rect.origin.y + tms.height;
        }
      [img compositeToPoint: p
                   fromRect: imgRect
                  operation: NSCompositeSourceOver]; 
    }

  // Draw Bottom-Middle image scaled

  img = images[TileBM];
  imgRect = NSMakeRect(0, 0,
    rect.size.width - bls.width - brs.width, bms.height);
  if (imgRect.size.width > 0 && imgRect.size.height > 0)
    {
      [img setScalesWhenResized: YES];
      [img setSize: imgRect.size];
      p = NSMakePoint(rect.origin.x + bls.width, rect.origin.y);
      if (flipped)
        {
          p.y = rect.origin.y + rect.size.height;
        }
      [img compositeToPoint: p
                   fromRect: imgRect
                  operation: NSCompositeSourceOver]; 
    }

  // Draw Center-Left image scaled

  img = images[TileCL];
  imgRect = NSMakeRect(0, 0,
    cls.width, rect.size.height - tls.height - bls.height);
  if (imgRect.size.width > 0 && imgRect.size.height > 0)
    {
      [img setScalesWhenResized: YES];
      [img setSize: imgRect.size];
      p = NSMakePoint(rect.origin.x, rect.origin.y + bls.height);
      if (flipped)
        {
          p.y = rect.origin.y + rect.size.height - bls.height;
        }
      [img compositeToPoint: p
                   fromRect: imgRect
                  operation: NSCompositeSourceOver]; 
    }

  // Draw Center-Right image scaled

  img = images[TileCR];
  imgRect = NSMakeRect(0, 0,
    crs.width, rect.size.height - trs.height - brs.height);
  if (imgRect.size.width > 0 && imgRect.size.height > 0)
    {
      [img setScalesWhenResized: YES];
      [img setSize: imgRect.size];
      p = NSMakePoint(rect.origin.x + rect.size.width - crs.width,
	rect.origin.y + brs.height);
      if (flipped)
        {
          p.y = rect.origin.y + rect.size.height - brs.height;
        }
      [img compositeToPoint: p
                   fromRect: imgRect
                  operation: NSCompositeSourceOver]; 
    }
}


- (void) drawCornersRect: (NSRect)rect
{
  BOOL flipped = [[GSCurrentContext() focusView] isFlipped];

  NSSize tls = rects[TileTL].size;
  NSSize trs = rects[TileTR].size;
  NSSize brs = rects[TileBR].size;
  NSPoint p;

  p = NSMakePoint (rect.origin.x,
    rect.origin.y + rect.size.height - tls.height);
  if (flipped)
    {
      p.y = rect.origin.y + tls.height;
    }
  [images[TileTL] compositeToPoint: p
                          fromRect: rects[TileTL]
                         operation: NSCompositeSourceOver];
// Is this right?
//  p = NSMakePoint(rect.origin.x + rect.size.width - trs.width + 1,
  p = NSMakePoint(rect.origin.x + rect.size.width - trs.width,
    rect.origin.y + rect.size.height - trs.height);
  if (flipped)
    {
      p.y = rect.origin.y + tls.height;
    }
  [images[TileTR] compositeToPoint: p
                          fromRect: rects[TileTR]
                         operation: NSCompositeSourceOver];

  p = NSMakePoint(rect.origin.x, rect.origin.y);
  if (flipped)
    {
      p.y = rect.origin.y + rect.size.height;
    }
  [images[TileBL] compositeToPoint: p
                          fromRect: rects[TileBL]
                         operation: NSCompositeSourceOver];

// Is this right?
//  p = NSMakePoint(rect.origin.x + rect.size.width - brs.width + 1,
  p = NSMakePoint(rect.origin.x + rect.size.width - brs.width,
    rect.origin.y);
  if (flipped)
    {
      p.y = rect.origin.y + rect.size.height;
    }
  [images[TileBR] compositeToPoint: p
                          fromRect: rects[TileBR]
                         operation: NSCompositeSourceOver];
}

- (GSThemeFillStyle) fillStyle
{
  return style;
}

- (void) setFillStyle: (GSThemeFillStyle)aStyle
{
  style = aStyle;
}

- (NSSize) size
{
  const CGFloat width = rects[TileCL].size.width
    + rects[TileCM].size.width
    + rects[TileCR].size.width;

  const CGFloat height = rects[TileTM].size.height
    + rects[TileCM].size.height
    + rects[TileBM].size.height;

  return NSMakeSize(width, height);
}

@end

