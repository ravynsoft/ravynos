/*
   XGFontInfo

   NSFont helper for GNUstep GUI X/GPS Backend

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley
   Author: Ovidiu Predescu <ovidiu@bx.logicnet.ro>
   Date: February 1997
   Author:  Felipe A. Rodriguez <far@ix.netcom.com>
   Date: May, October 1998
   Author:  Michael Hanni <mhanni@sprintmail.com>
   Date: August 1998
   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: September 2000

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

#include <config.h>

#include "xlib/XGContext.h"
#include "xlib/XGPrivate.h"
#include "xlib/XGGState.h"
#include "x11/XGServer.h"
#include <Foundation/NSDebug.h>
#include <Foundation/NSData.h>
#include <Foundation/NSValue.h>
// For the encoding functions
#include <GNUstepBase/GSMime.h>
#include <GNUstepBase/Unicode.h>

static Atom XA_SLANT = (Atom)0;
static Atom XA_SETWIDTH_NAME = (Atom)0;
static Atom XA_CHARSET_REGISTRY = (Atom)0;
static Atom XA_CHARSET_ENCODING = (Atom)0;
static Atom XA_SPACING = (Atom)0;
static Atom XA_PIXEL_SIZE = (Atom)0;
static Atom XA_WEIGHT_NAME = (Atom)0;

/*
 * Initialise the X atoms we are going to use
 */
static BOOL XGInitAtoms(Display *dpy)
{
  // X atoms used to query a font

  if (!dpy)
    {
      NSDebugLog(@"No Display opened in XGInitAtoms");      
      return NO;
    }

  XA_PIXEL_SIZE = XInternAtom(dpy, "PIXEL_SIZE", False);
  XA_SPACING = XInternAtom(dpy, "SPACING", False);
  XA_WEIGHT_NAME = XInternAtom(dpy, "WEIGHT_NAME", False);
  XA_SLANT = XInternAtom(dpy, "SLANT", False);
  XA_SETWIDTH_NAME = XInternAtom(dpy, "SETWIDTH_NAME", False);
  XA_CHARSET_REGISTRY = XInternAtom(dpy, "CHARSET_REGISTRY", False);
  XA_CHARSET_ENCODING = XInternAtom(dpy, "CHARSET_ENCODING", False);

/*
  XA_ADD_STYLE_NAME = XInternAtom(dpy, "ADD_STYLE_NAME", False);
  XA_RESOLUTION_X = XInternAtom(dpy, "RESOLUTION_X", False);
  XA_RESOLUTION_Y = XInternAtom(dpy, "RESOLUTION_Y", False);
  XA_AVERAGE_WIDTH = XInternAtom(dpy, "AVERAGE_WIDTH", False);
  XA_FACE_NAME = XInternAtom(dpy, "FACE_NAME", False);
*/

  return YES;
}


@interface XGFontInfo (Private)

- (BOOL) setupAttributes;
- (XCharStruct *)xCharStructForGlyph: (NSGlyph) glyph;

@end

@implementation XGFontInfo

- (XFontStruct*) xFontStruct
{
  return font_info;
}

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
    {
      XFreeFont([XGServer xDisplay], font_info);
    }
  [super dealloc];
}

- (NSMultibyteGlyphPacking)glyphPacking
{
  if (font_info->min_byte1 == 0 && 
      font_info->max_byte1 == 0)
    return NSOneByteGlyphPacking;
  else 
    return NSTwoByteGlyphPacking;
}

- (NSSize) advancementForGlyph: (NSGlyph)glyph
{
  XCharStruct *pc = [self xCharStructForGlyph: glyph];

  // if per_char is NULL assume max bounds
  if (!pc)
    pc = &(font_info->max_bounds);

  return NSMakeSize((float)pc->width, 0);
}

- (NSRect) boundingRectForGlyph: (NSGlyph)glyph
{
  XCharStruct *pc = [self xCharStructForGlyph: glyph];

  // if per_char is NULL assume max bounds
  if (!pc)
    return fontBBox;

  return NSMakeRect((float)pc->lbearing, (float)-pc->descent, 
		    (float)(pc->rbearing - pc->lbearing), 
		    (float)(pc->ascent + pc->descent));
}

- (BOOL) glyphIsEncoded: (NSGlyph)glyph
{
  XCharStruct *pc = [self xCharStructForGlyph: glyph];

  return (pc != NULL);
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

- (void) drawString:  (NSString*)string
	  onDisplay: (Display*) xdpy drawable: (Drawable) draw
	       with: (GC) xgcntxt at: (XPoint) xp
{
  XGCValues gcv;
  NSData *d = [string dataUsingEncoding: mostCompatibleStringEncoding
		      allowLossyConversion: YES];
  int length = [d length];
  const char *cstr = (const char*)[d bytes];

  // Select this font, although it might already be current.
  gcv.font = font_info->fid;
  XChangeGC(xdpy, xgcntxt, GCFont, &gcv);

  // FIXME: Use XDrawString16 for NSTwoByteGlyphPacking
  XDrawString(xdpy, draw, xgcntxt, xp.x, xp.y, cstr, length);
}

- (void) draw: (const char*) s length: (int) len 
    onDisplay: (Display*) xdpy drawable: (Drawable) draw
	 with: (GC) xgcntxt at: (XPoint) xp
{
  // This font must already be active!
  XDrawString(xdpy, draw, xgcntxt, xp.x, xp.y, s, len);
}

- (void) drawGlyphs: (const NSGlyph *) glyphs length: (int) len
	  onDisplay: (Display*) xdpy drawable: (Drawable) draw
	       with: (GC) xgcntxt at: (XPoint) xp
{
  // This font must already be active!
  char buf[len];
  int i;

  for (i = 0; i < len; i++)
    {
      buf[i] = glyphs[i];
    }

  XDrawString(xdpy, draw, xgcntxt, xp.x, xp.y, buf, len);
}

- (CGFloat) widthOfString: (NSString*)string
{
  NSData *d = [string dataUsingEncoding: mostCompatibleStringEncoding
		      allowLossyConversion: YES];
  int length = [d length];
  const char *cstr = (const char*)[d bytes];

  // FIXME: Use XTextWidth16 for NSTwoByteGlyphPacking
  return XTextWidth(font_info, cstr, length);
}

- (CGFloat) widthOf: (const char*) s length: (int) len
{
  return XTextWidth(font_info, s, len);
}

- (CGFloat) widthOfGlyphs: (const NSGlyph *) glyphs length: (int) len
{
  char buf[len];
  int i;

  for (i = 0; i < len; i++)
    {
      buf[i] = glyphs[i];
    }

  return XTextWidth(font_info, buf, len);
}

- (void) setActiveFor: (Display*) xdpy gc: (GC) xgcntxt
{
  XGCValues gcv;

  // Select this font, although it might already be current.
  gcv.font = font_info->fid;
  XChangeGC(xdpy, xgcntxt, GCFont, &gcv);
}

@end

@implementation XGFontInfo (Private)

- (BOOL) setupAttributes
{
  Display *xdpy = [XGServer xDisplay];
  NSString *reg;
  long height;      
  NSString *xfontname;

  if (!xdpy)
    return NO;

  if (!XA_PIXEL_SIZE)
    XGInitAtoms(xdpy);

  // Retrieve the XLFD matching the given fontName. DPS->X.
  xfontname = XGXFontName(fontName, matrix[0]);

  // Load X font and get font info structure.
  if ((xfontname == nil) ||
      (font_info = XLoadQueryFont(xdpy, [xfontname cString])) == NULL)
    {
      NSLog(@"XGFont: selected font: %@ at %f (%@) is not available.\n", 
	    fontName, matrix[0], xfontname);
      return NO;
    }
  else
    NSDebugLLog(@"NSFont", @"Loaded font: %@", xfontname);

  // Fill the ivars
  ASSIGN(familyName, XGFontFamily(xdpy, font_info));
  isFixedPitch = XGFontIsFixedPitch(xdpy, font_info);
  isBaseFont = NO;
  ascender = font_info->ascent;
  descender = -(font_info->descent);
  fontBBox = NSMakeRect(
    (float)(font_info->min_bounds.lbearing),
    (float)(-font_info->max_bounds.descent),
    (float)(font_info->max_bounds.rbearing - font_info->min_bounds.lbearing),
    (float)(font_info->max_bounds.ascent + font_info->max_bounds.descent));
  maximumAdvancement = NSMakeSize(font_info->max_bounds.width, 0.0);
  minimumAdvancement = NSMakeSize(0,0);
  weight = XGWeightOfFont(xdpy, font_info);
  traits = XGTraitsOfFont(xdpy, font_info);

  reg = XGFontPropString(xdpy, font_info, XA_CHARSET_REGISTRY);
  if (reg != nil)
    { 
      NSString *enc = XGFontPropString(xdpy, font_info, XA_CHARSET_ENCODING);

      if (enc != nil)
        {
	  if ([enc length] != 0 && [enc isEqualToString: @"0"] == NO)
	    {
	      encodingScheme = [NSString stringWithFormat: @"%@-%@", reg, enc];
	    }
	  else
	    {
	      encodingScheme = reg;
	    }
	  mostCompatibleStringEncoding
	    = [GSMimeDocument encodingFromCharset: encodingScheme];
	  NSDebugLLog(@"NSFont", @"Found encoding %d for %@", 
		     mostCompatibleStringEncoding, encodingScheme);
	  if (mostCompatibleStringEncoding == GSUndefinedEncoding)
	    {
	      mostCompatibleStringEncoding = NSASCIIStringEncoding;
	    }
	  RETAIN(encodingScheme);
	}
    }

  height = XGFontPropULong(xdpy, font_info, XA_X_HEIGHT);
  if (height != 0)
    {
      xHeight = (float)height;
    }

  height = XGFontPropULong(xdpy, font_info, XA_CAP_HEIGHT);
  if (height != 0)
    {
      capHeight = (float)height;
    }
  
  // FIXME: italicAngle, underlinePosition, underlineThickness are not set.
  // Should use XA_ITALIC_ANGLE, XA_UNDERLINE_POSITION, XA_UNDERLINE_THICKNESS

  return YES;
}

- (XCharStruct *)xCharStructForGlyph: (NSGlyph) glyph
{
  XCharStruct *pc = NULL;

  if (font_info->per_char)
    {
      unsigned index;
      unsigned min1 = font_info->min_byte1;
      unsigned max1 = font_info->max_byte1;
      unsigned min2 = font_info->min_char_or_byte2;
      unsigned max2 = font_info->max_char_or_byte2;

      // glyph is an unicode char value
      // if the font has non-standard encoding we need to remap it.
      if ((mostCompatibleStringEncoding != NSASCIIStringEncoding)
	&& (mostCompatibleStringEncoding != NSISOLatin1StringEncoding)
	&& (mostCompatibleStringEncoding != NSUnicodeStringEncoding))
        {
	  // FIXME: This only works for 8-Bit characters
	  unsigned int  size = 1;
	  unsigned char c = 0;
	  unsigned char *dst = &c;
	  unichar ch = glyph;

	  GSFromUnicode(&dst, &size, &ch, 1,
	    mostCompatibleStringEncoding, 0, 0);
	  index = c;
	}
      else 
	{
	  index = glyph;
	}

      if (min1 == 0 && max1 == 0)
        {
	  if (index >= min2 && index <= max2)
	    pc = &(font_info->per_char[index - min2]);
        }
      else 
        {
	  unsigned b1 = index >> 8;
	  unsigned b2 = index & 255;
  
	  if (b1 >= min1 && b1 <= max1 && b2 >= min2 && b2 <= max2)
	    pc = &(font_info->per_char[(b1 - min1) * (max2 - min2 + 1) + 
				     b2 - min2]);
        }
    }

  return pc;
}

@end
