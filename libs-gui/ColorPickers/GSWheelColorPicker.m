/* GSWheelColorPicker.m

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author:  Fred Kiefer <FredKiefer@gmx.de>
   Date: Febuary 2001
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

#include <math.h>

#ifndef PI
#define PI 3.141592653589793
#endif

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <GNUstepGUI/GSHbox.h>
#include "GSStandardColorPicker.h"

@interface GSColorWheelMarker : NSView
{
}

@end

@implementation GSColorWheelMarker : NSView

-(BOOL) isOpaque
{
  return YES;
}

-(void) drawRect: (NSRect)rect
{
  NSRect bounds = [self bounds];
  [[NSColor whiteColor] set];
  NSRectFill(bounds);
  [[NSColor blackColor] set];
  NSFrameRect(bounds);
}

@end


@interface GSColorWheel : NSView
{
  float hue, saturation, brightness;

  id target;
  SEL action;

  GSColorWheelMarker *marker;
  NSImage *image;
}

-(float) hue;
-(float) saturation;

-(void) regenerateImage;
-(NSRect) markerRect;

-(void) setHue: (float)h saturation: (float)s brightness: (float)brightness;

-(void) setTarget: (id)t;
-(void) setAction: (SEL)a;

@end

@implementation GSColorWheel

-(id) initWithFrame: (NSRect)frame
{
  self = [super initWithFrame: frame];
  if (nil == self)
    {
      return nil;
    }

  [self setPostsFrameChangedNotifications: YES];
  [[NSNotificationCenter defaultCenter] 
    addObserver: self
       selector: @selector(_frameChanged:)
	   name: NSViewFrameDidChangeNotification
	 object: self];

  return self;
}

-(void) _frameChanged: (id)sender
{
  [self regenerateImage];
  [marker setFrame: [self markerRect]];
}

-(void) dealloc
{
  [image release];
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  [super dealloc];
}

-(void) setTarget: (id)t
{
  target = t;
}
-(void) setAction: (SEL)a
{
  action = a;
}

-(float) hue
{
  return hue;
}
-(float) saturation
{
  return saturation;
}

-(NSRect) markerRect
{
  NSRect frame = [self bounds];
  float a,r,x,y,cr,cx,cy;

  cx = (frame.origin.x + frame.size.width) / 2;
  cy = (frame.origin.y + frame.size.height) / 2;

  cr = frame.size.width;
  if (cr > frame.size.height)
    cr = frame.size.height;

  cr = cr / 2 - 2;

  a = hue * 2 * PI;
  r = saturation * cr;

  x = cos(a) * r + cx;
  y = sin(a) * r + cy;

  return NSMakeRect(x-2,y-2,4,4);
}

-(void) setHue: (float)h saturation: (float)s brightness: (float)b
{
  if (nil == marker)
    {
      marker = [[[GSColorWheelMarker alloc] initWithFrame: [self markerRect]] autorelease];
      [self addSubview: marker];
    }
  
  if (hue != h || saturation != s || brightness != b)
    {
      BOOL regenerate = (brightness != b);
      
      hue = h;
      saturation = s;
      brightness = b;
      
      if (regenerate)
	[self regenerateImage];
    
      [marker setFrame: [self markerRect]];

      [self setNeedsDisplay: YES];
    }
}

-(void) regenerateImage
{
  NSSize size = [self convertSizeToBase: [self bounds].size];
  CGFloat cx, cy, cr;

  [image release];
  image = nil;

  cx = (size.width) / 2;
  cy = (size.height) / 2;

  cr = size.width;
  if (cr > size.height)
    cr = size.height;

  cr = cr / 2 - 2;

  {
    NSUInteger width = size.width;
    NSUInteger height = size.height;
    NSUInteger bytesPerRow;
    NSBitmapImageRep *bmp;
    unsigned char *data;
    NSUInteger x, y;

    if (width < 1 || height < 1)
      return;

    bmp = [[NSBitmapImageRep alloc]
			      initWithBitmapDataPlanes: NULL
					    pixelsWide: width
					    pixelsHigh: height
					 bitsPerSample: 8
				       samplesPerPixel: 4
					      hasAlpha: YES
					      isPlanar: NO
					colorSpaceName: NSCalibratedRGBColorSpace
					   bytesPerRow: 0
					  bitsPerPixel: 32];

    bytesPerRow = [bmp bytesPerRow];
    data = [bmp bitmapData];
    
    for (y = 0; y < height; y++)
      {
	uint32_t *row = (uint32_t*)(data + (y * bytesPerRow));

	for (x = 0; x < width; x++)
	  {
	    CGFloat dx, dy, dist;
	    CGFloat h, s, v;
	    CGFloat R, G, B, A;

	    dx = x - cx;
	    dy = cy - y; // compensate for flipped coordinates
	    dist = sqrt(dx * dx + dy * dy);

	    // calculate h,s,v from x,y
	    {
	      h = atan2(dy, dx) / 2.0 / PI;
	      if (h < 0)
		h += 1;
	  
	      s = dist/cr;
	      if (s > 1)
		s = 1;

	      v = brightness;
	    }

	    // calculate R,G,B from h,s,v
	    {
	      int	I = (int)(h * 6);
	      CGFloat V = v;
	      CGFloat S = s;
	      CGFloat F = (h * 6) - I;
	      CGFloat M = V * (1 - S);
	      CGFloat N = V * (1 - S * F);
	      CGFloat K = M - N + V;

	      switch (I)
		{
		default: R = V; G = K; B = M; break;
		case 1: R = N; G = V; B = M; break;
		case 2: R = M; G = V; B = K; break;
		case 3: R = M; G = N; B = V; break;
		case 4: R = K; G = M; B = V; break;
		case 5: R = V; G = M; B = N; break;
		}
	    }

	    // calculate alpha
	    {
	      A = (cr - dist) + 0.5;
	      if (A > 1) A = 1;
	      if (A < 0) A = 0;
	    }

	    // premultiply color with alpha
	    R *= A;
	    G *= A;
	    B *= A;
 
	    // store pixel
#if GS_WORDS_BIGENDIAN
	    row[x] = ((uint32_t)(255 * R) << 24)
	      | (((uint32_t)(255 * G)) << 16)
	      | (((uint32_t)(255 * B)) << 8)
	      | (((uint32_t)(255 * A)));
#else
	    row[x] = ((uint32_t)(255 * R))
	      | (((uint32_t)(255 * G)) << 8)
	      | (((uint32_t)(255 * B)) << 16)
	      | (((uint32_t)(255 * A)) << 24);
#endif
	  }
      }

    image = [[NSImage alloc] initWithSize: [self bounds].size];
    [image addRepresentation: bmp];
    [bmp release];
  }
}

-(void) drawRect: (NSRect)rect
{
  if (nil == image)
    {
      [self regenerateImage];
    }

  [image drawInRect: [self bounds]
           fromRect: NSZeroRect
          operation: NSCompositeSourceOver
           fraction: 1.0];  
}

-(BOOL) acceptsFirstMouse: (NSEvent *)theEvent
{
  return YES;
}

-(BOOL) acceptsFirstResponder
{
  return NO;
}

-(void) handleMouseAtPoint: (NSPoint)point
{
  NSRect frame = [self bounds];
  CGFloat cx, cy, cr, dx, dy, new_hue, new_saturation;

  cx = (frame.origin.x + frame.size.width) / 2;
  cy = (frame.origin.y + frame.size.height) / 2;
  cr = frame.size.width;
  if (cr > frame.size.height)
    cr = frame.size.height;
  cr = cr / 2 - 2;

  dx = point.x - cx;
  dy = point.y - cy;

  new_saturation = sqrt(dx * dx + dy * dy) / cr;
  if (new_saturation > 1)
    new_saturation = 1;

  new_hue = atan2(dy, dx) / 2.0 / PI;
  if (new_hue < 0)
    new_hue += 1;

  [self setHue: new_hue saturation: new_saturation brightness: brightness];

  if (target)
    {
      [target performSelector: action withObject: self];
    }
}

-(void) mouseDown: (NSEvent *)theEvent
{
  if ([theEvent type] == NSLeftMouseDown)
    {
      [self handleMouseAtPoint: [self convertPoint: [theEvent locationInWindow] fromView: nil]];
    }
}

-(void) mouseDragged: (NSEvent *)theEvent
{
  if ([theEvent type] == NSLeftMouseDragged)
    {
      [self handleMouseAtPoint: [self convertPoint: [theEvent locationInWindow] fromView: nil]];
    }
}

@end


@interface GSWheelColorPicker: NSColorPicker <NSColorPickingCustom>
{
  GSHbox *baseView;
  NSSlider *brightnessSlider;
  GSColorWheel *wheel;
}

- (void) sliderChanged: (id) sender;
- (void) loadViews;

@end

@implementation GSWheelColorPicker

- (void) dealloc
{
  RELEASE(baseView);
  [super dealloc];
}

- (id)initWithPickerMask:(int)aMask
	      colorPanel:(NSColorPanel *)colorPanel
{
  if (aMask & NSColorPanelWheelModeMask)
    return [super initWithPickerMask: aMask
		  colorPanel: colorPanel];
  RELEASE(self);
  return nil;
}

- (int)currentMode
{
  return NSWheelModeColorPanel;
}

- (BOOL)supportsMode:(int)mode
{
  return mode == NSWheelModeColorPanel;
}

- (NSView *)provideNewView:(BOOL)initialRequest
{
  if (initialRequest)
    {
      [self loadViews];
    }
  return baseView;
}

- (void)setColor:(NSColor *)color
{
  CGFloat hue, saturation, brightness, alpha;
  NSColor *c;

  c = [color colorUsingColorSpaceName: NSCalibratedRGBColorSpace];
  [c getHue: &hue saturation: &saturation brightness: &brightness alpha: &alpha];

  [(GSColorSliderCell *)[brightnessSlider cell]
    _setColorSliderCellValues: hue : saturation : brightness];
  [brightnessSlider setNeedsDisplay: YES];
  [brightnessSlider setFloatValue: brightness];
  [wheel setHue: hue saturation: saturation brightness: brightness];
}

- (void) loadViews
{
  NSSlider *s;
  NSCell *c;

  baseView = [[GSHbox alloc] init];
  [baseView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];

  wheel = [[GSColorWheel alloc] init];
  [wheel setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
  [wheel setTarget: self];
  [wheel setAction: @selector(sliderChanged:)];
  [baseView addView: wheel];

  s = brightnessSlider = [[NSSlider alloc] initWithFrame: NSMakeRect(0,0,16,0)];
  [s setAutoresizingMask: NSViewHeightSizable];
  c = [[GSColorSliderCell alloc] init];
  [s setCell: c];
  RELEASE(c);
  [(GSColorSliderCell *)[s cell] _setColorSliderCellMode: 10];
  [s setContinuous: YES];
  [s setMinValue: 0.0];
  [s setMaxValue: 1.0];
  [s setTarget: self];
  [s setAction: @selector(sliderChanged:)];
  [[s cell] setBezeled: YES];

  [baseView addView: brightnessSlider enablingXResizing: NO];
}

- (void) sliderChanged: (id) sender
{
  float brightness = [brightnessSlider floatValue];
  float hue = [wheel hue];
  float saturation = [wheel saturation];
  float alpha = [_colorPanel alpha];
  NSColor *c;

  [(GSColorSliderCell *)[brightnessSlider cell]
    _setColorSliderCellValues: hue : saturation : brightness];
  [brightnessSlider setNeedsDisplay: YES];

  c = [NSColor colorWithCalibratedHue: hue
			saturation: saturation
			brightness: brightness
			alpha: alpha];
  [_colorPanel setColor: c];
}

@end

