/*
   GSXftFontInfo

   NSFont helper for GNUstep GUI X/GPS Backend

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: July 2001

   This file is part of the GNUstep GUI X/GPS Backend.

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
#include "xlib/XGContext.h"
#include "xlib/XGPrivate.h"
#include "xlib/XGGState.h"
#include "x11/XGServer.h"
#include <Foundation/NSByteOrder.h>
#include <Foundation/NSCharacterSet.h>
#include <Foundation/NSData.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSValue.h>
// For the encoding functions
#include <GNUstepBase/Unicode.h>

#include <AppKit/NSBezierPath.h>
#include "xlib/GSXftFontInfo.h"

#define id _gs_avoid_id_collision
#include <fontconfig/fontconfig.h>
#undef id

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

@implementation GSXftFaceInfo
@end

@implementation GSXftFontEnumerator
+ (Class) faceInfoClass
{
  return [GSXftFaceInfo class];
}
+ (GSXftFaceInfo *) fontWithName: (NSString *) name
{
  return (GSXftFaceInfo *) [super fontWithName: name];
}
@end

@interface GSXftFontInfo (Private)

- (BOOL) setupAttributes;
- (XGlyphInfo *)xGlyphInfo: (NSGlyph) glyph;

@end

@implementation GSXftFontInfo

- initWithFontName: (NSString*)name
	    matrix: (const CGFloat*)fmatrix
	screenFont: (BOOL)screenFont
{
  if (screenFont)
    {
      RELEASE(self);
      return nil;
    }

  [super init];
  ASSIGN(fontName, name);
  memcpy(matrix, fmatrix, sizeof(matrix));

  if (![self setupAttributes])
    {
      RELEASE(self);
      return nil;
    }

  return self;
}

- (void) dealloc
{
  if (font_info != NULL)
    XftFontClose([XGServer xDisplay], (XftFont *)font_info);
  [super dealloc];
}

- (CGFloat) widthOfString: (NSString*)string
{
  XGlyphInfo extents;
  int len = [string length];
  XftChar16 str[len]; 

  [string getCharacters: (unichar*)str];
  XftTextExtents16 ([XGServer xDisplay],
		    font_info,
		    str, 
		    len,
		    &extents);

  return extents.width;
}

- (CGFloat) widthOfGlyphs: (const NSGlyph *) glyphs length: (int) len
{
  XGlyphInfo extents;
  XftChar16 buf[len];
  int i;

  for (i = 0; i < len; i++)
    {
      buf[i] = glyphs[i];
    }

  XftTextExtents16 ([XGServer xDisplay],
		    font_info,
		    buf,
		    len,
		    &extents);

  return extents.width;
}

- (NSMultibyteGlyphPacking)glyphPacking
{
  return NSTwoByteGlyphPacking;
}

- (NSSize) advancementForGlyph: (NSGlyph)glyph
{
  if (_cachedSizes)
    {
      int entry = glyph % _cacheSize;

      if (_cachedGlyphs[entry] == glyph)
        return _cachedSizes[entry];

      XGlyphInfo *pc = [self xGlyphInfo: glyph];

      if (!pc)
        return NSMakeSize((float)(font_info)->max_advance_width, 0);

      _cachedGlyphs[entry] = glyph;
      _cachedSizes[entry] = NSMakeSize((float)pc->xOff, (float)pc->yOff);

      return _cachedSizes[entry];
    }

  XGlyphInfo *pc = [self xGlyphInfo: glyph];

  // if per_char is NULL assume max bounds
  if (!pc)
    return  NSMakeSize((float)(font_info)->max_advance_width, 0);

  return NSMakeSize((float)pc->xOff, (float)pc->yOff);
}

- (NSRect) boundingRectForGlyph: (NSGlyph)glyph
{
  XGlyphInfo *pc = [self xGlyphInfo: glyph];

  // if per_char is NULL assume max bounds
  if (!pc)
      return NSMakeRect(0.0, 0.0,
		    (float)font_info->max_advance_width,
		    (float)(font_info->ascent + font_info->descent));

  return NSMakeRect((float)pc->x, (float)-pc->y, 
		    (float)(pc->width), 
		    (float)(pc->height));
}

- (BOOL) glyphIsEncoded: (NSGlyph)glyph
{
  return XftGlyphExists([XGServer xDisplay],
			(XftFont *)font_info, glyph);
}

- (NSGlyph) glyphWithName: (NSString*)glyphName
{
  // FIXME: There is a mismatch between PS names and X names, that we should 
  // try to correct here
  KeySym k = XStringToKeysym([glyphName cString]);

  if (k == NoSymbol)
    return 0;
  else
    return (NSGlyph)k;
}

- (NSPoint) positionOfGlyph: (NSGlyph)curGlyph
	    precededByGlyph: (NSGlyph)prevGlyph
		  isNominal: (BOOL*)nominal
{
  if (nominal)
    *nominal = YES;

  if (curGlyph == NSControlGlyph || prevGlyph == NSControlGlyph)
    return NSZeroPoint;

//  if (curGlyph == NSNullGlyph)
    {
      NSSize advance = [self advancementForGlyph: prevGlyph];
      return NSMakePoint(advance.width, advance.height);
    }
}

/*
- (CGFloat) pointSize
{
  Display	*xdpy = [XGServer xDisplay];

  return XGFontPointSize(xdpy, font_info);
}
*/

- (void) drawString:  (NSString*)string
	  onDisplay: (Display*) xdpy drawable: (Drawable) draw
	       with: (GC) xgcntxt at: (XPoint) xp
{
  NSData *d = [string dataUsingEncoding: mostCompatibleStringEncoding
		      allowLossyConversion: YES];
  int length = [d length];
  const char *cstr = (const char*)[d bytes];
  XGGState *state = (XGGState *)[(XGContext *)GSCurrentContext() currentGState];
  XftDraw *xftdraw = [state xftDrawForDrawable: draw];
  XftColor xftcolor = [state xftColor];

  /* do it */
  XftDrawString16(xftdraw, &xftcolor, font_info, 
		  xp.x, xp.y, (XftChar16*)cstr, length);
}

- (void) drawGlyphs: (const NSGlyph *) glyphs length: (int) len
	  onDisplay: (Display*) xdpy drawable: (Drawable) draw
	       with: (GC) xgcntxt at: (XPoint) xp
{
  XGGState *state = (XGGState *)[(XGContext *)GSCurrentContext() currentGState];
  XftDraw *xftdraw = [state xftDrawForDrawable: draw];
  XftColor xftcolor = [state xftColor];
  XftChar16 buf[len];
  int i;
 
  for (i = 0; i < len; i++)
    {
      buf[i] = glyphs[i];
    }

  /* do it */
  XftDrawString16(xftdraw, &xftcolor, font_info, 
		  xp.x, xp.y, (XftChar16*)buf, len);
}

- (void) draw: (const char*) s length: (int) len 
    onDisplay: (Display*) xdpy drawable: (Drawable) draw
	 with: (GC) xgcntxt at: (XPoint) xp
{
  int length = strlen(s);
  XGGState *state = (XGGState *)[(XGContext *)GSCurrentContext() currentGState];
  XftDraw *xftdraw = [state xftDrawForDrawable: draw];
  XftColor xftcolor = [state xftColor];

#ifdef HAVE_UTF8
  /* do it */
  if (NSUTF8StringEncoding == mostCompatibleStringEncoding)
    {
      XftDrawStringUtf8(xftdraw, &xftcolor, font_info,
                        xp.x, xp.y, (XftChar8 *)s, length);
    }
  else
#endif
    {
      XftDrawString8(xftdraw, &xftcolor, font_info, 
                   xp.x, xp.y, (XftChar8*)s, length);
    }
}

- (CGFloat) widthOf: (const char*) s length: (int) len
{
  XGlyphInfo extents;

#ifdef HAVE_UTF8
  if (mostCompatibleStringEncoding == NSUTF8StringEncoding)
    XftTextExtentsUtf8([XGServer xDisplay],
                       font_info,
                       (XftChar8 *)s,
                       len,
                       &extents);
  else
#endif
    XftTextExtents8([XGServer xDisplay],
                    font_info,
                    (XftChar8*)s, 
                    len,
                    &extents);

  return extents.width;
}


- (void) setActiveFor: (Display*) xdpy gc: (GC) xgcntxt
{
}

static int bezierpath_move_to(const FT_Vector *to, void *user)
{
  NSBezierPath *path = (NSBezierPath *)user;
  NSPoint d;

  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
/*
  d.x = to->x;
  d.y = to->y;
*/
  [path closePath];
  [path moveToPoint: d];
  return 0;
}

static int bezierpath_line_to(const FT_Vector *to, void *user)
{
  NSBezierPath *path = (NSBezierPath *)user;
  NSPoint d;

  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
/*
  d.x = to->x;
  d.y = to->y;
*/
  [path lineToPoint: d];
  return 0;
}

static int bezierpath_conic_to(const FT_Vector *c1, const FT_Vector *to, void *user)
{
  NSBezierPath *path = (NSBezierPath *)user;
  NSPoint a, b, c, d;

  a = [path currentPoint];
  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
  b.x = c1->x / 65536.0;
  b.y = c1->y / 65536.0;
/*
  d.x = to->x;
  d.y = to->y;
  b.x = c1->x;
  b.y = c1->y;
*/
  c.x = (b.x * 2 + d.x) / 3.0;
  c.y = (b.y * 2 + d.y) / 3.0;
  b.x = (b.x * 2 + a.x) / 3.0;
  b.y = (b.y * 2 + a.y) / 3.0;
  [path curveToPoint: d controlPoint1: b controlPoint2: c];
  return 0;
}

static int bezierpath_cubic_to(const FT_Vector *c1, const FT_Vector *c2, 
                               const FT_Vector *to, void *user)
{
  NSBezierPath *path = (NSBezierPath *)user;
  NSPoint b, c, d;

  b.x = c1->x / 65536.0;
  b.y = c1->y / 65536.0;
  c.x = c2->x / 65536.0;
  c.y = c2->y / 65536.0;
  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
/*
  b.x = c1->x;
  b.y = c1->y;
  c.x = c2->x;
  c.y = c2->y;
  d.x = to->x;
  d.y = to->y;
*/
  [path curveToPoint: d controlPoint1: b controlPoint2: c];
  return 0;
}

static FT_Outline_Funcs bezierpath_funcs = {
  move_to: bezierpath_move_to,
  line_to: bezierpath_line_to,
  conic_to: bezierpath_conic_to,
  cubic_to: bezierpath_cubic_to,
  shift: 10,
//  delta: 0,
};

- (void) appendBezierPathWithGlyphs: (NSGlyph *)glyphs
                              count: (int)count
                       toBezierPath: (NSBezierPath *)path
{
  int i;
  FT_Matrix ftmatrix;
  FT_Vector ftdelta;
  FT_Face face;
  FT_Int load_flags = FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP;
  NSPoint p = [path currentPoint];

  ftmatrix.xx = 65536;
  ftmatrix.xy = 0;
  ftmatrix.yx = 0;
  ftmatrix.yy = 65536;
  ftdelta.x = p.x * 64.0;
  ftdelta.y = p.y * 64.0;

  face = XftLockFace((XftFont *)font_info);
  for (i = 0; i < count; i++)
    {
      NSGlyph glyph;
      FT_Glyph gl;
      FT_OutlineGlyph og;

      glyph = glyphs[i];
      // FIXME: Should do this conversion in the glyph creation!
      glyph = XftCharIndex([XGServer xDisplay], 
                           (XftFont *)font_info, glyph);
     
      if (FT_Load_Glyph(face, glyph, load_flags))
        continue;

      if (FT_Get_Glyph(face->glyph, &gl))
        continue;

      if (FT_Glyph_Transform(gl, &ftmatrix, &ftdelta))
        {
          NSLog(@"glyph transformation failed!");
          continue;
        }

      og = (FT_OutlineGlyph)gl;

      ftdelta.x += gl->advance.x >> 10;
      ftdelta.y += gl->advance.y >> 10;

      FT_Outline_Decompose(&og->outline, &bezierpath_funcs, path);

      FT_Done_Glyph(gl);
    }

  XftUnlockFace((XftFont *)font_info);

  if (count)
    {
      [path moveToPoint: NSMakePoint(ftdelta.x / 64.0, ftdelta.y / 64.0)];
    }
}

@end

@implementation GSXftFontInfo (Private)

- (BOOL) setupAttributes
{
  Display *xdpy = [XGServer xDisplay];
  int defaultScreen = DefaultScreen(xdpy);

  GSXftFaceInfo *realFont = [GSXftFontEnumerator fontWithName: fontName];
  FcPattern *fontPattern;
  FcPattern *pattern; 
  FcResult fc_result;

  if (![super setupAttributes])
    return NO;

  if (!realFont)
    {
      return NO;
    }

  if (!xdpy)
    return NO;

  fontPattern = FcPatternDuplicate([realFont matchedPattern]);

  // the only thing needs customization here is the size
  // FIXME: It would be correcter to use FC_SIZE as GNUstep should be
  // using point measurements, but as the rest of the library uses pixel,
  // we need to stick with that here.
  FcPatternAddDouble(fontPattern, FC_PIXEL_SIZE, (double)(matrix[0]));
  // Should do this only when size > 8
  FcPatternAddBool(fontPattern, FC_AUTOHINT, FcTrue);
  pattern = XftFontMatch(xdpy, defaultScreen, fontPattern, &fc_result);
  // tide up
  FcPatternDestroy(fontPattern);

  // Derek Zhou claims that this takes over the ownership of the pattern
  if ((font_info = XftFontOpenPattern(xdpy, pattern)))
    {
      NSDebugLLog(@"NSFont", @"Loaded font: %@", fontName);
    }
  else
    {
      NSDebugLLog(@"NSFont", @"Cannot load font: %@", fontName);
      return NO;
    }

  // Fill the ivars
  isBaseFont = NO;
  ascender = font_info->ascent;
  descender = -(font_info->descent);
  capHeight = ascender - descender;   // TODO
  xHeight = capHeight*0.6;   //Errr... TODO
  lineHeight = capHeight;
  fontBBox = NSMakeRect(
    (float)(0),
    (float)(-font_info->descent),
    (float)(font_info->max_advance_width),
    (float)(font_info->ascent + font_info->descent));
  maximumAdvancement = NSMakeSize(font_info->max_advance_width, 0.0);
  minimumAdvancement = NSMakeSize(0,0);
//   printf("h=%g  a=%g d=%g  max=(%g %g)  (%g %g)+(%g %g)\n",
//          xHeight, ascender, descender,
//          maximumAdvancement.width, maximumAdvancement.height,
//          fontBBox.origin.x, fontBBox.origin.y,
//          fontBBox.size.width, fontBBox.size.height);

  return YES;
}

- (XGlyphInfo *)xGlyphInfo: (NSGlyph) glyph
{
  static XGlyphInfo glyphInfo;

  XftTextExtents32 ([XGServer xDisplay],
                    (XftFont *)font_info,
                    &glyph,
                    1,
                    &glyphInfo);

  return &glyphInfo;
}

@end
