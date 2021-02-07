/* GSGState - Generic graphic state

   Copyright (C) 1998-2010 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Mar 2002
   
   This file is part of the GNU Objective C User Interface Library.

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
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSDictionary.h>
#import <AppKit/NSAffineTransform.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSColorSpace.h>
#import <AppKit/NSImage.h>
#import <GNUstepGUI/GSFontInfo.h>
#import <AppKit/NSGraphics.h>
#import "gsc/GSContext.h"
#import "gsc/GSGState.h"
#import "gsc/GSFunction.h"
#include "math.h"
#import <GNUstepBase/Unicode.h>

#define CHECK_PATH \
  if (!path) \
    { \
      path = [NSBezierPath new]; \
    }

@implementation GSGState

/* Designated initializer. */
- initWithDrawContext: (GSContext *)drawContext
{  
  self = [super init];
  if (!self)
    return nil;

  drawcontext = drawContext;
  offset = NSMakePoint(0, 0);
  path   = nil;
  font   = nil;
  fillColorS   = nil;
  strokeColorS = nil;
  [self DPSinitgraphics];
  return self;
}

- (void) dealloc
{
  TEST_RELEASE(font);
  TEST_RELEASE(path);
  RELEASE(ctm);
  RELEASE(textCtm);
  RELEASE(fillColorS);
  RELEASE(strokeColorS);
  TEST_RELEASE(pattern);
  [super dealloc];
}

- (id) deepen
{
  NSZone *zone = [self zone];

  if (path)
    self->path = [path copyWithZone: zone];

  self->ctm     = [ctm copyWithZone: zone];
  self->textCtm = [textCtm copyWithZone: zone];

  // Just retain the other objects
  if (font != nil)
    RETAIN(font);
  if (fillColorS != nil)
    RETAIN(fillColorS);
  if (strokeColorS != nil)
    RETAIN(strokeColorS);
  if (pattern != nil)
    RETAIN(pattern);

  return self;
}

- (NSString*) description
{
  NSMutableString *description = [[super description] mutableCopy];
  [description appendFormat: @" drawcontext: %@",drawcontext];
  [description appendFormat: @" ctm: %@",ctm];
  return [description copy];
}

- (id)copyWithZone: (NSZone *)zone
{
  GSGState *new = (GSGState *)NSCopyObject(self, 0, zone);  
  /* Do a deep copy since gstates are isolated from each other */
  return [new deepen];
}

- (void) setOffset: (NSPoint)theOffset
{
  offset = theOffset;
}

- (NSPoint) offset
{
  return offset;
}

/** Subclasses should override this method to be notified of changes
    in the current color */
- (void) setColor: (device_color_t *)color state: (color_state_t)cState
{
  if ((cState & COLOR_FILL) && (&fillColor != color))
    {
      fillColor = *color;
    }
  if ((cState & COLOR_STROKE) && (&strokeColor != color))
    {
      strokeColor = *color;
    }
  cstate = cState;
  DESTROY(pattern);
}

- (void) GSSetPatterColor: (NSImage*)image 
{
  ASSIGN(pattern, image);
}

- (void) setShouldAntialias: (BOOL)antialias
{
  _antialias = antialias;
}

- (BOOL) shouldAntialias
{
  return _antialias;
}

- (NSPoint) patternPhase
{
  return _patternPhase;
}

- (void) setPatternPhase: (NSPoint)phase
{
  _patternPhase = phase;
}

- (NSCompositingOperation) compositingOperation
{
  return _compositingOperation;
}

- (void) setCompositingOperation: (NSCompositingOperation)operation
{
  _compositingOperation = operation;
}

// This is only a fall back, the method should not be called any more.
- (void) compositeGState: (GSGState *)source
                fromRect: (NSRect)aRect
                 toPoint: (NSPoint)aPoint
                      op: (NSCompositingOperation)op
{
  [self compositeGState: source 
        fromRect: aRect 
        toPoint: aPoint 
        op: op
        fraction: 1.0];

}

// This is only a fall back, the method should not be called any more.
- (void) dissolveGState: (GSGState *)source
               fromRect: (NSRect)aRect
                toPoint: (NSPoint)aPoint
                  delta: (CGFloat)delta
{
  [self compositeGState: source 
        fromRect: aRect 
        toPoint: aPoint 
        op: NSCompositeSourceOver
        fraction: delta];
}

- (void) compositeGState: (GSGState *)source
                fromRect: (NSRect)aRect
                 toPoint: (NSPoint)aPoint
                      op: (NSCompositingOperation)op
                fraction: (CGFloat)delta
{
  [self subclassResponsibility: _cmd];
}

- (void) compositerect: (NSRect)aRect
                    op: (NSCompositingOperation)op
{
  [self subclassResponsibility: _cmd];
}

- (NSPoint) pointInMatrixSpace: (NSPoint)aPoint
{
  return [ctm transformPoint: aPoint];
}

- (NSPoint) deltaPointInMatrixSpace: (NSPoint)aPoint
{
  return [ctm deltaPointInMatrixSpace: aPoint];
}

- (NSRect) rectInMatrixSpace: (NSRect)rect
{
  return [ctm rectInMatrixSpace: rect];
}

@end

@implementation GSGState (Ops)

/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
- (void) DPScurrentalpha: (CGFloat*)a
{
  *a = fillColor.field[AINDEX];
}

- (void) DPScurrentcmykcolor: (CGFloat*)c : (CGFloat*)m : (CGFloat*)y : (CGFloat*)k
{
  device_color_t new = fillColor;
  gsColorToCMYK(&new);
  *c = new.field[0];
  *m = new.field[1];
  *y = new.field[2];
  *k = new.field[3];
}

- (void) DPScurrentgray: (CGFloat*)gray
{
  device_color_t gcolor = fillColor;
  gsColorToGray(&gcolor);
  *gray = gcolor.field[0];
}

- (void) DPScurrenthsbcolor: (CGFloat*)h : (CGFloat*)s : (CGFloat*)b
{
  device_color_t gcolor = fillColor;
  gsColorToHSB(&gcolor);
  *h = gcolor.field[0]; *s = gcolor.field[1]; *b = gcolor.field[2];
}

- (void) DPScurrentrgbcolor: (CGFloat*)r : (CGFloat*)g : (CGFloat*)b
{
  device_color_t gcolor = fillColor;
  gsColorToRGB(&gcolor);
  *r = gcolor.field[0]; *g = gcolor.field[1]; *b = gcolor.field[2];
}

#define CLAMP(x) \
  if (x < 0.0) x = 0.0; \
  if (x > 1.0) x = 1.0;

- (void) DPSsetalpha: (CGFloat)a
{
  CLAMP(a)
  fillColor.field[AINDEX] = strokeColor.field[AINDEX] = a;
  [self setColor: &fillColor state: COLOR_FILL];
  [self setColor: &strokeColor state: COLOR_STROKE];
}

- (void) DPSsetcmykcolor: (CGFloat)c : (CGFloat)m : (CGFloat)y : (CGFloat)k
{
  device_color_t col;
  CLAMP(c)
  CLAMP(m)
  CLAMP(y)
  CLAMP(k)
  gsMakeColor(&col, cmyk_colorspace, c, m, y, k);
  // Keep the old alpha value
  col.field[AINDEX] = fillColor.field[AINDEX];
  [self setColor: &col state: COLOR_BOTH];
}

- (void) DPSsetgray: (CGFloat)gray
{
  device_color_t col;
  CLAMP(gray)
  gsMakeColor(&col, gray_colorspace, gray, 0, 0, 0);
  // Keep the old alpha value
  col.field[AINDEX] = fillColor.field[AINDEX];
  [self setColor: &col  state: COLOR_BOTH];
}

- (void) DPSsethsbcolor: (CGFloat)h : (CGFloat)s : (CGFloat)b
{
  device_color_t col;
  CLAMP(h)
  CLAMP(s)
  CLAMP(b)
  gsMakeColor(&col, hsb_colorspace, h, s, b, 0);
  // Keep the old alpha value
  col.field[AINDEX] = fillColor.field[AINDEX];
  [self setColor: &col state: COLOR_BOTH];
}

- (void) DPSsetrgbcolor: (CGFloat)r : (CGFloat)g : (CGFloat)b
{
  device_color_t col;
  CLAMP(r)
  CLAMP(g)
  CLAMP(b)
  gsMakeColor(&col, rgb_colorspace, r, g, b, 0);
  // Keep the old alpha value
  col.field[AINDEX] = fillColor.field[AINDEX];
  [self setColor: &col state: COLOR_BOTH];
}


- (void) GSSetFillColorspace: (void *)spaceref
{
  device_color_t col;

  ASSIGN(fillColorS, (NSColorSpace*)spaceref);
  gsMakeColor(&col, rgb_colorspace, 0, 0, 0, 0);
  // Keep the old alpha value
  col.field[AINDEX] = fillColor.field[AINDEX];
  [self setColor: &col state: COLOR_FILL];
}

- (void) GSSetStrokeColorspace: (void *)spaceref
{
  device_color_t col;

  ASSIGN(strokeColorS, (NSColorSpace*)spaceref);
  gsMakeColor(&col, rgb_colorspace, 0, 0, 0, 0);
  // Keep the old alpha value
  col.field[AINDEX] = fillColor.field[AINDEX];
  [self setColor: &col state: COLOR_STROKE];
}

- (void) GSSetFillColor: (const CGFloat *)values
{
  device_color_t dcolor;
  NSColor *color;

  if ((fillColorS == nil) 
      || ((color = [NSColor colorWithColorSpace: fillColorS
                            components: values
                            count: [fillColorS numberOfColorComponents] + 1]) == nil)
      || ((color = [color colorUsingColorSpaceName: NSDeviceRGBColorSpace]) == nil))
    {
      DPS_ERROR(DPSundefined, @"No fill colorspace defined, assume DeviceRGB");
      gsMakeColor(&dcolor, rgb_colorspace, values[0], values[1], 
                  values[2], values[3]);
      dcolor.field[AINDEX] = values[4];
    }
  else 
    {
      CGFloat r, g, b, a;
      [color getRed: &r
             green: &g
             blue: &b
             alpha: &a];
      dcolor.space = rgb_colorspace;
      dcolor.field[0] = r;
      dcolor.field[1] = g;
      dcolor.field[2] = b;
      dcolor.field[AINDEX] = a;
    }

  [self setColor: &dcolor state: COLOR_FILL];
}

- (void) GSSetStrokeColor: (const CGFloat *)values
{
  device_color_t dcolor;
  NSColor *color;

  if ((strokeColorS == nil) 
      || ((color = [NSColor colorWithColorSpace: strokeColorS
                            components: values
                            count: [strokeColorS numberOfColorComponents] + 1]) == nil)
      || ((color = [color colorUsingColorSpaceName: NSDeviceRGBColorSpace]) == nil))
    {
      DPS_ERROR(DPSundefined, @"No stroke colorspace defined, assume DeviceRGB");
      gsMakeColor(&dcolor, rgb_colorspace, values[0], values[1], 
                  values[2], values[3]);
      dcolor.field[AINDEX] = values[4];
    }
  else 
    {
      CGFloat r, g, b, a;
      [color getRed: &r
             green: &g
             blue: &b
             alpha: &a];
      dcolor.space = rgb_colorspace;
      dcolor.field[0] = r;
      dcolor.field[1] = g;
      dcolor.field[2] = b;
      dcolor.field[AINDEX] = a;
    }

  [self setColor: &dcolor state: COLOR_STROKE];
}

/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */

typedef enum {
  show_delta, show_array_x, show_array_y, show_array_xy
} show_array_t;

/* Omnibus show string routine that combines that characteristics of
   ashow, awidthshow, widthshow, xshow, xyshow, and yshow */
- (void) _showString: (const char *)s
	    xCharAdj: (CGFloat)cx
	    yCharAdj: (CGFloat)cy
		char: (char)c
	    adjArray: (const CGFloat *)arr
	     arrType: (show_array_t)type
	  isRelative: (BOOL)relative;
{
  NSPoint point = [path currentPoint];
  unichar *uch;
  unsigned int ulen;
  int i;

  /* 
     FIXME: We should use proper glyph generation here.
  */
  uch = NULL;
  ulen = 0;
  GSToUnicode(&uch, &ulen, (const unsigned char*)s, strlen(s),
    [font mostCompatibleStringEncoding], NSDefaultMallocZone(), 0);

  for (i = 0; i < ulen; i++)
    {
      NSPoint delta;
      NSGlyph glyph;

      glyph = (NSGlyph)uch[i];
      [self GSShowGlyphs: &glyph : 1];
      /* Note we update the current point according to the current 
	 transformation scaling, although the text isn't currently
	 scaled (FIXME). */
      if (type == show_array_xy)
	{
	  delta.x = arr[2*i]; delta.y = arr[2*i+1];
	}
      else if (type == show_array_x)
	{
	  delta.x = arr[i]; delta.y = 0;
	}
      else if (type == show_array_y)
	{
	  delta.x = 0; delta.y = arr[i];
	}
      else
	{
	  delta.x = arr[0]; delta.y = arr[1];
	}
      delta = [ctm deltaPointInMatrixSpace: delta];
      if (relative == YES)
	{
	  NSSize advancement;

	  advancement = [font advancementForGlyph: glyph];
	  /* Use only delta transformations (no offset). Is this conversion needed?*/
	  advancement = [ctm transformSize: NSMakeSize(advancement.width, 
						       [font ascender])];
	  delta.x += advancement.width;
	  delta.y += advancement.height;
	}
      if (c && *(s+i) == c)
	{
	  NSPoint cdelta;

	  cdelta.x = cx; cdelta.y = cy;
	  cdelta = [ctm deltaPointInMatrixSpace: cdelta];
	  delta.x += cdelta.x; delta.y += cdelta.y;
	}
      point.x += delta.x;
      if (type != show_delta)
        {
	  point.y += delta.y;
	}
      [path moveToPoint: point];
    }
  free(uch);
}

- (void) DPSashow: (CGFloat)x : (CGFloat)y : (const char*)s
{
  CGFloat arr[2];

  arr[0] = x; arr[1] = y;
  [self _showString: s
    xCharAdj: 0 yCharAdj: 0 char: 0 adjArray: arr arrType: show_delta
    isRelative: YES];
}

- (void) DPSawidthshow: (CGFloat)cx : (CGFloat)cy : (int)c : (CGFloat)ax : (CGFloat)ay 
		      : (const char*)s
{
  CGFloat arr[2];

  arr[0] = ax; arr[1] = ay;
  [self _showString: s
    xCharAdj: cx yCharAdj: cy char: c adjArray: arr arrType: show_delta
    isRelative: YES];
}

- (void) DPScharpath: (const char*)s : (int)count
{
  NSGlyph glBuf[count];
  int i;

  if (!font)
    return;

  // FIXME
  for (i = 0; i < count; i++)
    {
      glBuf[i] = [font glyphForCharacter: s[i]];  
    }
  
  CHECK_PATH;
  [font appendBezierPathWithGlyphs: glBuf
        count: count
        toBezierPath: path];
}

- (void) appendBezierPathWithPackedGlyphs: (const char *)packedGlyphs
                                     path: (NSBezierPath*)aPath
{
  unsigned int count = packedGlyphs[0];
  NSMultibyteGlyphPacking packing;
  NSGlyph glBuf[count];
  int i;
  int j;
  unsigned char a, b, c, d;

  if (!font)
    return;

  packing = [font glyphPacking];
  j = 1;
  for (i = 0; i < count; i++)
    {
      switch (packing)
        {
          case NSOneByteGlyphPacking: 
            glBuf[i] = (NSGlyph)packedGlyphs[j++]; 
            break;
          case NSTwoByteGlyphPacking: 
            a= packedGlyphs[j++];
            glBuf[i] = (NSGlyph)((a << 8) | packedGlyphs[j++]);
            break;
          case NSFourByteGlyphPacking:
            a = packedGlyphs[j++];
            b = packedGlyphs[j++];
            c = packedGlyphs[j++];
            d = packedGlyphs[j++];
            glBuf[i] = (NSGlyph)((a << 24) | (b << 16) 
                                 | (c << 8) | d);
            break;          
          case NSJapaneseEUCGlyphPacking:
          case NSAsciiWithDoubleByteEUCGlyphPacking:
          default:
            // FIXME
            break;
        }
    }

  [font appendBezierPathWithGlyphs: glBuf
        count: count
        toBezierPath: aPath];
}

- (void) DPSshow: (const char*)s
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSwidthshow: (CGFloat)x : (CGFloat)y : (int)c : (const char*)s
{
  CGFloat arr[2];

  arr[0] = 0; arr[1] = 0;
  [self _showString: s
    xCharAdj: x yCharAdj: y char: c adjArray: arr arrType: show_delta
    isRelative: YES];
}

- (void) DPSxshow: (const char*)s : (const CGFloat*)numarray : (int)size
{
  [self _showString: s
    xCharAdj: 0 yCharAdj: 0 char: 0 adjArray: numarray arrType: show_array_x
    isRelative: NO];
}

- (void) DPSxyshow: (const char*)s : (const CGFloat*)numarray : (int)size
{
  [self _showString: s
    xCharAdj: 0 yCharAdj: 0 char: 0 adjArray: numarray arrType: show_array_xy
    isRelative: NO];
}

- (void) DPSyshow: (const char*)s : (const CGFloat*)numarray : (int)size
{
  [self _showString: s
    xCharAdj: 0 yCharAdj: 0 char: 0 adjArray: numarray arrType: show_array_y
    isRelative: NO];
}

- (void) GSSetCharacterSpacing: (CGFloat)extra
{
  charSpacing = extra;
}

- (void) GSSetFont: (GSFontInfo *)fontref
{
  if (font == fontref)
    return;
  ASSIGN(font, fontref);
}

- (void) GSSetFontSize: (CGFloat)size
{
}

- (NSAffineTransform *) GSGetTextCTM
{
  return textCtm;
}

- (NSPoint) GSGetTextPosition
{
  return [textCtm transformPoint: NSMakePoint(0,0)];
}

- (void) GSSetTextCTM: (NSAffineTransform *)newCtm
{
  ASSIGN(textCtm, newCtm);
}

- (void) GSSetTextDrawingMode: (GSTextDrawingMode)mode
{
  textMode = mode;
}

- (void) GSSetTextPosition: (NSPoint)loc
{
  [textCtm translateToPoint: loc];
}

- (void) GSShowText: (const char *)string : (size_t) length
{
  [self subclassResponsibility: _cmd];
}

- (void) GSShowGlyphs: (const NSGlyph *)glyphs : (size_t) length
{
  int i;
  NSSize advances[length];

  for (i=0; i<length; i++)
    {
      advances[i] = [font advancementForGlyph: glyphs[i]];
    }
  
  [self GSShowGlyphsWithAdvances: glyphs : advances : length];
}

- (void) GSShowGlyphsWithAdvances: (const NSGlyph *)glyphs : (const NSSize *)advances : (size_t) length
{
  [self subclassResponsibility: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
- (void) DPSinitgraphics
{
  DESTROY(path);
  DESTROY(font);
  DESTROY(fillColorS);
  DESTROY(strokeColorS);
  if (ctm)
    [ctm makeIdentityMatrix];
  else
    ctm = [[NSAffineTransform allocWithZone: [self zone]] init];

   /* Initialize colors. By default the same color is used for filling and 
     stroking unless fill and/or stroke color is set explicitly */
  gsMakeColor(&fillColor, gray_colorspace, 0, 0, 0, 0);
  fillColor.field[AINDEX] = 1.0;
  [self setColor: &fillColor state: COLOR_BOTH];

  charSpacing = 0;
  textMode    = GSTextFill;
  if (textCtm)
    [textCtm makeIdentityMatrix];
  else
    textCtm = [[NSAffineTransform allocWithZone: [self zone]] init];
}

- (void)DPScurrentflat: (CGFloat *)flatness 
{
  if (path)
    *flatness = [path flatness];
  else 
    *flatness = 1.0;
}

- (void) DPScurrentlinecap: (int*)linecap
{
  [self subclassResponsibility: _cmd];
}

- (void) DPScurrentlinejoin: (int*)linejoin
{
  [self subclassResponsibility: _cmd];
}

- (void) DPScurrentlinewidth: (CGFloat*)width
{
  [self subclassResponsibility: _cmd];
}

- (void) DPScurrentmiterlimit: (CGFloat*)limit
{
  [self subclassResponsibility: _cmd];
}

- (NSPoint) currentPoint
{
  NSAffineTransform *ictm;
  NSPoint user;

  if (path == nil)
    {
      return NSMakePoint(0, 0);
    }

  // This is rather slow, but it is not used very often
  ictm = [ctm copyWithZone: [self zone]];
  [ictm invert];
  user = [ictm transformPoint: [path currentPoint]];
  RELEASE(ictm);
  return user;
}

- (void)DPScurrentpoint: (CGFloat *)x : (CGFloat *)y 
{
  NSPoint user;

  user = [self currentPoint];
  *x = user.x;
  *y = user.y;
}

- (void) DPScurrentstrokeadjust: (int*)b
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSsetdash: (const CGFloat*)pat : (NSInteger)size : (CGFloat)offset
{
  [self subclassResponsibility: _cmd];
}

- (void)DPSsetflat: (CGFloat)flatness 
{
  if (path)
    [path setFlatness: flatness];
}

- (void) DPSsetlinecap: (int)linecap
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSsetlinejoin: (int)linejoin
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSsetlinewidth: (CGFloat)width
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSsetmiterlimit: (CGFloat)limit
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSsetstrokeadjust: (int)b
{
  [self subclassResponsibility: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
- (void)DPSconcat: (const CGFloat *)m
{
  NSAffineTransformStruct matrix;
  NSAffineTransform *new_ctm = [NSAffineTransform new];

  matrix.m11 = m[0];
  matrix.m12 = m[1];
  matrix.m21 = m[2];
  matrix.m22 = m[3];
  matrix.tX  = m[4];
  matrix.tY  = m[5];
  [new_ctm setTransformStruct: matrix];

  [ctm prependTransform: new_ctm];
  RELEASE(new_ctm);
}

- (void)DPSinitmatrix 
{
  [ctm makeIdentityMatrix];
}

- (void)DPSrotate: (CGFloat)angle 
{
  [ctm rotateByDegrees: angle];
}

- (void)DPSscale: (CGFloat)x : (CGFloat)y 
{
  [ctm scaleXBy: x  yBy: y];
}

- (void)DPStranslate: (CGFloat)x : (CGFloat)y 
{
  [ctm translateToPoint: NSMakePoint(x, y)];
}

- (NSAffineTransform *) GSCurrentCTM
{
  return AUTORELEASE([ctm copy]);
}

- (void) GSSetCTM: (NSAffineTransform *)newctm
{
  ASSIGN(ctm, newctm);
}

- (void) GSConcatCTM: (NSAffineTransform *)newctm
{
  [ctm prependTransform: newctm];
}

/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
- (void) DPSarc: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 : (CGFloat)angle2 
{
  NSBezierPath *newPath;

  newPath = [[NSBezierPath alloc] init];
  if ((path != nil) && ([path elementCount] != 0))
    {
      [newPath lineToPoint: [self currentPoint]];
    }
  [newPath appendBezierPathWithArcWithCenter: NSMakePoint(x, y)  
	   radius: r
	   startAngle: angle1
	   endAngle: angle2
	   clockwise: NO];
  [newPath transformUsingAffineTransform: ctm];
  CHECK_PATH;
  [path appendBezierPath: newPath];
  RELEASE(newPath);
}

- (void) DPSarcn: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 : (CGFloat)angle2 
{
  NSBezierPath *newPath;

  newPath = [[NSBezierPath alloc] init];
  if ((path != nil) && ([path elementCount] != 0))
    {
      [newPath lineToPoint: [self currentPoint]];
    }
  [newPath appendBezierPathWithArcWithCenter: NSMakePoint(x, y)  
	   radius: r
	   startAngle: angle1
	   endAngle: angle2
	   clockwise: YES];
  [newPath transformUsingAffineTransform: ctm];
  CHECK_PATH;
  [path appendBezierPath: newPath];
  RELEASE(newPath);
}

- (void)DPSarct: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 : (CGFloat)r 
{
  NSBezierPath *newPath;

  newPath = [[NSBezierPath alloc] init];
  if ((path != nil) && ([path elementCount] != 0))
    {
	[newPath lineToPoint: [self currentPoint]];
    }
  [newPath appendBezierPathWithArcFromPoint: NSMakePoint(x1, y1)
	   toPoint: NSMakePoint(x2, y2)
	   radius: r];
  [newPath transformUsingAffineTransform: ctm];
  CHECK_PATH;
  [path appendBezierPath: newPath];
  RELEASE(newPath);
}

- (void) DPSclip
{
  [self subclassResponsibility: _cmd];
}

- (void)DPSclosepath 
{
  CHECK_PATH;
  [path closePath];
}

- (void)DPScurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 : (CGFloat)x3 
		  : (CGFloat)y3 
{
  NSPoint p1 = [ctm transformPoint: NSMakePoint(x1, y1)];
  NSPoint p2 = [ctm transformPoint: NSMakePoint(x2, y2)];
  NSPoint p3 = [ctm transformPoint: NSMakePoint(x3, y3)];

  CHECK_PATH;
  [path curveToPoint: p3 controlPoint1: p1 controlPoint2: p2];
}

- (void) DPSeoclip
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSeofill
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSfill
{
  [self subclassResponsibility: _cmd];
}

- (void)DPSflattenpath 
{
  if (path)
    ASSIGN(path, [path bezierPathByFlatteningPath]);
}

- (void) DPSinitclip
{
  [self subclassResponsibility: _cmd];
}

- (void)DPSlineto: (CGFloat)x : (CGFloat)y 
{
  NSPoint p = [ctm transformPoint: NSMakePoint(x, y)];

  CHECK_PATH;
  [path lineToPoint: p];
}

- (void)DPSmoveto: (CGFloat)x : (CGFloat)y 
{
  NSPoint p = [ctm transformPoint: NSMakePoint(x, y)];

  CHECK_PATH;
  [path moveToPoint: p];
}

- (void)DPSnewpath 
{
  if (path)
    [path removeAllPoints];
}

- (NSBezierPath *) bezierPath
{
  // This is rather slow, but it is not used very often
  NSBezierPath *newPath = [path copy];
  NSAffineTransform *ictm = [ctm copyWithZone: [self zone]];

  [ictm invert];
  [newPath transformUsingAffineTransform: ictm];
  RELEASE(ictm);
  return AUTORELEASE(newPath);
}

- (void)DPSpathbbox: (CGFloat *)llx : (CGFloat *)lly : (CGFloat *)urx : (CGFloat *)ury 
{
  NSBezierPath *bpath = [self bezierPath];
  NSRect rect = [bpath controlPointBounds];

  if (llx)
    *llx = NSMinX(rect);
  if (lly)
    *lly = NSMinY(rect);
  if (urx)
    *urx = NSMaxX(rect);
  if (ury)
    *ury = NSMaxY(rect);
}

- (void)DPSrcurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 : (CGFloat)x3 
		   : (CGFloat)y3 
{
  NSPoint p1 = [ctm deltaPointInMatrixSpace: NSMakePoint(x1, y1)];
  NSPoint p2 = [ctm deltaPointInMatrixSpace: NSMakePoint(x2, y2)];
  NSPoint p3 = [ctm deltaPointInMatrixSpace: NSMakePoint(x3, y3)];
 
  CHECK_PATH;
  [path relativeCurveToPoint: p3
	controlPoint1: p1
	controlPoint2: p2];
}

- (void) DPSrectclip: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  NSBezierPath *oldPath = path;

  path = [[NSBezierPath alloc] init];
  [path appendBezierPathWithRect: NSMakeRect(x, y, w, h)];
  [path transformUsingAffineTransform: ctm];
  [self DPSclip];
  RELEASE(path);
  path = oldPath;
  if (path)
    [path removeAllPoints];
}

- (void) DPSrectfill: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  NSBezierPath *oldPath = path;

  path = [[NSBezierPath alloc] init];
  [path appendBezierPathWithRect: NSMakeRect(x, y, w, h)];
  [path transformUsingAffineTransform: ctm];
  [self DPSfill];
  RELEASE(path);
  path = oldPath;
}

- (void) DPSrectstroke: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  NSBezierPath *oldPath = path;

  path = [[NSBezierPath alloc] init];
  [path appendBezierPathWithRect: NSMakeRect(x, y, w, h)];
  [path transformUsingAffineTransform: ctm];
  [self DPSstroke];
  RELEASE(path);
  path = oldPath;
}

- (void)DPSreversepath 
{
  if (path)
    ASSIGN(path, [path bezierPathByReversingPath]);
}

- (void)DPSrlineto: (CGFloat)x : (CGFloat)y 
{
  NSPoint p = [ctm deltaPointInMatrixSpace: NSMakePoint(x, y)];
 
  CHECK_PATH;
  [path relativeLineToPoint: p];
}

- (void)DPSrmoveto: (CGFloat)x : (CGFloat)y 
{
  NSPoint p = [ctm deltaPointInMatrixSpace: NSMakePoint(x, y)];
 
  CHECK_PATH;
  [path relativeMoveToPoint: p];
}

- (void) DPSstroke;
{
  [self subclassResponsibility: _cmd];
}

- (void) GSSendBezierPath: (NSBezierPath *)newpath
{
  NSInteger count = 10;
  CGFloat dash_pattern[10];
  CGFloat phase;

  // Appending to the current path is a lot faster than copying!
  //ASSIGNCOPY(path, newpath);
  CHECK_PATH;
  [path removeAllPoints];
  [path appendBezierPath: newpath];
  [path transformUsingAffineTransform: ctm];

  // The following should be moved down into the specific subclasses
  [self DPSsetlinewidth: [newpath lineWidth]];
  [self DPSsetlinejoin: [newpath lineJoinStyle]];
  [self DPSsetlinecap: [newpath lineCapStyle]];
  [self DPSsetmiterlimit: [newpath miterLimit]];
  [self DPSsetflat: [newpath flatness]];

  [newpath getLineDash: dash_pattern count: &count phase: &phase];
  [self DPSsetdash: dash_pattern : count : phase];
}

- (void) GSRectClipList: (const NSRect *)rects : (int) count
{
  int i;
  NSRect union_rect;

  if (count == 0)
    return;

  /* 
     The specification is not clear if the union of the rects 
     should produce the new clip rect or if the outline of all rects 
     should be used as clip path.
  */
  union_rect = rects[0];
  for (i = 1; i < count; i++)
    union_rect = NSUnionRect(union_rect, rects[i]);

  [self DPSrectclip: NSMinX(union_rect) : NSMinY(union_rect)
	  : NSWidth(union_rect) : NSHeight(union_rect)];
}


- (void) GSRectFillList: (const NSRect *)rects : (int) count
{
  int i;
  for (i=0; i < count; i++)
    [self DPSrectfill: NSMinX(rects[i]) : NSMinY(rects[i])
	  : NSWidth(rects[i]) : NSHeight(rects[i])];
}

- (NSDictionary *) GSReadRect: (NSRect)r
{
  return nil;
}

- (void)DPSimage: (NSAffineTransform*) matrix 
		: (NSInteger) pixelsWide : (NSInteger) pixelsHigh
		: (NSInteger) bitsPerSample : (NSInteger) samplesPerPixel 
		: (NSInteger) bitsPerPixel : (NSInteger) bytesPerRow : (BOOL) isPlanar
		: (BOOL) hasAlpha : (NSString *) colorSpaceName
		: (const unsigned char *const [5]) data
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSshfill: (NSDictionary *)shader
{
  NSNumber *v;
  NSDictionary *function_dict;
  GSFunction2in3out *function;
  NSAffineTransform *matrix, *inverse;
  NSAffineTransformStruct	ts;
  NSRect rect;
  int iwidth, iheight;
  double x, y;
  int i;
  unsigned char *data;

  v = [shader objectForKey: @"ShadingType"];

  /* only type 1 shaders */
  if ([v intValue] != 1)
    {
      NSLog(@"ShadingType != 1 not supported.");
      return;
    }

  /* in device rgb space */
  if ([shader objectForKey: @"ColorSpace"])
    if (![[shader objectForKey: @"ColorSpace"] isEqual: NSDeviceRGBColorSpace])
      {
        NSLog(@"Only device RGB ColorSpace supported for shading.");
        return;
      }

  function_dict = [shader objectForKey: @"Function"];
  if (!function_dict)
    {
      NSLog(@"Shading function not set.");
      return;
    }

  function = [[GSFunction2in3out alloc] initWith: function_dict];
  if (!function)
    return;

  matrix = [ctm copy];
  if ([shader objectForKey: @"Matrix"])
    {
      [matrix prependTransform: [shader objectForKey: @"Matrix"]];
    }

  inverse = [matrix copy];
  [inverse invert];
  ts = [inverse transformStruct];

  rect = [function affectedRect];
  iwidth = rect.size.width;
  iheight = rect.size.height;
  data = malloc(sizeof(char) * iwidth * iheight * 4);
  i = 0;

  for (y = NSMinY(rect); y < NSMaxY(rect); y++)
    {
      double in[2], out[3];
      NSPoint p;

      p = [inverse transformPoint: NSMakePoint(NSMinX(rect), y)];
      in[0] = p.x;
      in[1] = p.y;
          
      out[0] = out[1] = out[2] = 0.0;
      for (x = NSMinX(rect); x < NSMaxX(rect); x++)
        {
          unsigned char r, g, b, a;
            
          [function eval: in : out];
     
          // Set data at x - NSMinX(rect), y - NSMinY(rect) to out
          r = out[0] * 255;
          g = out[1] * 255;
          b = out[2] * 255;
          a = 255;
          data[i++] = r;
          data[i++] = g;
          data[i++] = b;
          data[i++] = a;

          // This gives the same result as: 
          // p = [inverse transformPoint: NSMakePoint(x, y)];
          in[0] += ts.m11;
          in[1] += ts.m12;
        }
    }

  // Copy data to device
  DESTROY(matrix);
  matrix = [NSAffineTransform new];
  [matrix translateXBy: NSMinX(rect) yBy: NSMinY(rect)];
  [self DPSimage: matrix 
        : iwidth : iheight
        : 8 : 4 
        : 32 : 4 * iwidth  : NO
        : YES : NSDeviceRGBColorSpace
        : (const unsigned char **)&data];
  free(data);

  DESTROY(matrix);
  DESTROY(inverse);
  DESTROY(function);
}

@end


@implementation GSGState (PatternColor)

- (void *) saveClip
{
  [self subclassResponsibility: _cmd];
  return NULL;
}

- (void) restoreClip: (void *)savedClip
{
  [self subclassResponsibility: _cmd];
}

- (void) _fillRect: (NSRect)rect withPattern: (NSImage*)color_pattern
{
  NSSize size;
  NSAffineTransform *ictm;
  NSPoint patternPhase, startPoint, endPoint, point;

  // The coordinates we get here are already in device space,
  // but compositeToPoint needs user space coordinates
  ictm = [ctm copyWithZone: [self zone]];
  [ictm invert];

  size = [color_pattern size];
  patternPhase = [self patternPhase];

  if (!NSEqualPoints(patternPhase, NSZeroPoint))
    {
      // patternPhase % pattern size
      patternPhase.x -= floor(patternPhase.x / size.width) * size.width;
      patternPhase.y -= floor(patternPhase.y / size.height) * size.height;
    }

  startPoint = NSMakePoint(floor((NSMinX(rect) - patternPhase.x) / size.width) * size.width 
                           + patternPhase.x,
                           floor((NSMinY(rect) - patternPhase.y) / size.height) * size.height
                           + patternPhase.y);

  endPoint = NSMakePoint(NSMaxX(rect), NSMaxY(rect));

  for (point.y = startPoint.y; point.y < endPoint.y; point.y += size.height)
    {
      for (point.x = startPoint.x; point.x < endPoint.x; point.x += size.width)
        {
	  [color_pattern compositeToPoint: [ictm transformPoint: point]
                                operation: NSCompositeSourceOver];
        }
    }
  RELEASE(ictm);
}

- (void) fillRect: (NSRect)rect withPattern: (NSImage*)color_pattern
{
  NSBezierPath *oldPath = path;
  void *oldClip;

  oldClip = [self saveClip];
  path = [[NSBezierPath alloc] init];
  [path appendBezierPathWithRect: rect];
  [self DPSclip];

  [self _fillRect: rect withPattern: color_pattern];

  [self restoreClip: oldClip];
  RELEASE(path);
  path = oldPath;
}

- (void) fillPath: (NSBezierPath*)fillPath withPattern: (NSImage*)color_pattern
{
  NSBezierPath *oldPath = path;
  NSRect rect;
  void *oldClip;

  oldClip = [self saveClip];
  rect = [fillPath bounds];
  path = fillPath;
  [self DPSclip];

  [self _fillRect: rect withPattern: color_pattern];

  [self restoreClip: oldClip];
  path = oldPath;
  [self DPSnewpath];
}

- (void) eofillPath: (NSBezierPath*)fillPath withPattern: (NSImage*)color_pattern
{
  NSBezierPath *oldPath = path;
  NSRect rect;
  void *oldClip;

  oldClip = [self saveClip];
  rect = [fillPath bounds];
  path = fillPath;
  [self DPSeoclip];

  [self _fillRect: rect withPattern: color_pattern];

  [self restoreClip: oldClip];
  path = oldPath;
  [self DPSnewpath];
}

@end

@implementation GSGState (NSGradient)

- (void) drawGradient: (NSGradient*)gradient
           fromCenter: (NSPoint)startCenter
               radius: (CGFloat)startRadius
             toCenter: (NSPoint)endCenter 
               radius: (CGFloat)endRadius
              options: (NSUInteger)options
{
  [self subclassResponsibility: _cmd];
}

- (void) drawGradient: (NSGradient*)gradient
            fromPoint: (NSPoint)startPoint
              toPoint: (NSPoint)endPoint
              options: (NSUInteger)options
{
  [self subclassResponsibility: _cmd];
}

@end
