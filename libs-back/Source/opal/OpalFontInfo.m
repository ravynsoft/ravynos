/*
   OpalFontInfo.m
 
   Copyright (C) 2013 Free Software Foundation, Inc.

   Author: Ivan Vucica <ivan@vucica.net>
   Date: September 2013

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

#include "GNUstepBase/Unicode.h"
#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSBezierPath.h>
#include "opal/OpalFontInfo.h"
#include "opal/OpalFontEnumerator.h"

#include <math.h>
/*
#include <cairo-ft.h>
*/

@implementation OpalFontInfo 

- (CGFloat) _fontUnitToUserSpace: (CGFloat)fontDimension
{
  CGFontRef face = [_faceInfo fontFace];
  CGFloat unitsPerEm = CGFontGetUnitsPerEm(face);

  CGFloat pointSize = matrix[0]; // from GSFontInfo

  return (fontDimension / unitsPerEm) * pointSize;
}

- (BOOL) setupAttributes
{
/*
  cairo_font_extents_t font_extents;
  cairo_font_face_t *face;
  cairo_matrix_t font_matrix;
  cairo_matrix_t ctm;
  cairo_font_options_t *options;
*/
  CGFontRef face;
  CGSize maximumAdvancementCG;
  CGRect fontBBoxCG;

  if (![super setupAttributes])
    {
      return NO;
    }
#if 0
  /* setting GSFontInfo:
   * xHeight, pix_width, pix_height
   */
  cairo_matrix_init(&font_matrix, matrix[0], matrix[1], -matrix[2],
                    matrix[3], matrix[4], matrix[5]);
  //cairo_matrix_scale(&font_matrix, 0.9, 0.9);
  cairo_matrix_init_identity(&ctm);
#endif
  face = [_faceInfo fontFace];
  if (!face)
    {
      return NO;
    }

  ascender = [self _fontUnitToUserSpace: CGFontGetAscent(face)];
  descender = [self _fontUnitToUserSpace: CGFontGetDescent(face)];
  xHeight = [self _fontUnitToUserSpace: CGFontGetXHeight(face)];

  CGFloat pointSize = matrix[0];

  maximumAdvancementCG = OPFontGetMaximumAdvancement(face);
  maximumAdvancement = NSMakeSize(maximumAdvancementCG.width * pointSize,
                                  maximumAdvancementCG.height * pointSize);
  
  fontBBoxCG = CGFontGetFontBBox(face);
  fontBBox = NSMakeRect([self _fontUnitToUserSpace: fontBBoxCG.origin.x],
			[self _fontUnitToUserSpace: fontBBoxCG.origin.y],
                        [self _fontUnitToUserSpace: fontBBoxCG.size.width],
			[self _fontUnitToUserSpace: fontBBoxCG.size.height]);

  CGFloat leading = [self _fontUnitToUserSpace: CGFontGetLeading(face)];

  if (xHeight == 0.0)
    xHeight = ascender * 0.6;

  // derived from code calculating CGFontGetLeading() value.
  // we may instead want to extend Opal to include OPFontGetLineHeight(),
  // containing this code:
  //   cairo_scaled_font_extents(_scaled, &font_extents);
  //   lineHeight = font_extents.height
  // alternatively: line spacing = (ascent + descent + "external leading")
  // (internal discussion between ivucica and ericwa, 2013-09-17)
  lineHeight = leading + ascender + descender;

#if 0
  // Get default font options
  options = cairo_font_options_create();
  if (cairo_font_options_status(options) != CAIRO_STATUS_SUCCESS)
    {
      return NO;
    }

  // We must not leave the hinting settings as their defaults,
  // because if we did, that would mean using the surface defaults
  // which might or might not use hinting (xlib does by default.)
  //
  // Since we make measurements outside of the context of a surface
  // (-advancementForGlyph:), we need to ensure that the same
  // hinting settings are used there as when we draw. For now,
  // just force hinting to be off.
  cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_ON);
  cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_NONE);

  _scaled = cairo_scaled_font_create(face, &font_matrix, &ctm, options);
  cairo_font_options_destroy(options);
  if (cairo_scaled_font_status(_scaled) != CAIRO_STATUS_SUCCESS)
    {
      return NO;
    }

  cairo_scaled_font_extents(_scaled, &font_extents);
  if (cairo_scaled_font_status(_scaled) != CAIRO_STATUS_SUCCESS)
    {
      return NO;
    }

  ascender = font_extents.ascent;
  descender = -font_extents.descent;
  xHeight = ascender * 0.6;
  lineHeight = font_extents.height;
  maximumAdvancement = NSMakeSize(font_extents.max_x_advance, 
                                  font_extents.max_y_advance);
  fontBBox = NSMakeRect(0, descender, 
                        maximumAdvancement.width, ascender - descender);
/*
  NSLog(@"Font matrix (%g, %g, %g, %g, %g, %g) type %d", 
        matrix[0], matrix[1], matrix[2],
        matrix[3], matrix[4], matrix[5], cairo_scaled_font_get_type(_scaled));
	NSLog(@"(%@) h=%g  a=%g d=%g  max=(%g %g)  (%g %g)+(%g %g)\n", fontName,
		xHeight, ascender, descender,
		maximumAdvancement.width, maximumAdvancement.height,
		fontBBox.origin.x, fontBBox.origin.y,
		fontBBox.size.width, fontBBox.size.height);
*/
#endif
  return YES;
}

- (id) initWithFontName: (NSString *)name 
                 matrix: (const CGFloat *)fmatrix 
             screenFont: (BOOL)p_screenFont
{
  self = [super init];
  if (!self)
    return nil;

  _screenFont = p_screenFont;
  fontName = [name copy];
  memcpy(matrix, fmatrix, sizeof(matrix));

  if (_screenFont)
    {
      /* Round up; makes the text more legible. */
      matrix[0] = ceil(matrix[0]);
      if (matrix[3] < 0.0)
        matrix[3] = floor(matrix[3]);
      else
        matrix[3] = ceil(matrix[3]);
    }

  if (![self setupAttributes])
    {
      RELEASE(self);
      return nil;
    }

  return self;
}

- (void) dealloc
{
#if 0
  if (_scaled)
    {
      cairo_scaled_font_destroy(_scaled);
    }
#endif
  [super dealloc];
}

- (BOOL) glyphIsEncoded: (NSGlyph)glyph
{
  CGFontRef face = [_faceInfo fontFace];
  size_t numGlyphs = CGFontGetNumberOfGlyphs(face);

  return glyph < numGlyphs;
}
#if 0
static
BOOL _cairo_extents_for_NSGlyph(cairo_scaled_font_t *scaled_font, NSGlyph glyph,
                                cairo_text_extents_t *ctext)
{
  unichar ustr[2];
  char str[4];
  unsigned char *b;
  unsigned int size = 4;
  int length = 1;

  ustr[0] = glyph;
  ustr[1] = 0;

  b = (unsigned char *)str;
  if (!GSFromUnicode(&b, &size, ustr, length, 
                     NSUTF8StringEncoding, NULL, GSUniTerminate))
    {
      NSLog(@"Conversion failed for %@", 
            [NSString stringWithCharacters: ustr length: length]);
      return NO;
    }

  cairo_scaled_font_text_extents(scaled_font, str, ctext);
  return cairo_scaled_font_status(scaled_font) == CAIRO_STATUS_SUCCESS;
}
#endif

- (NSSize) advancementForGlyph: (NSGlyph)glyph
{
  CGFontRef face = [_faceInfo fontFace];
  int advance = 0;
  CGGlyph cgglyph = glyph;
  CGFontGetGlyphAdvances(face, &cgglyph, 1, &advance);

  CGFloat advanceUserSpace = [self _fontUnitToUserSpace: advance];

  return NSMakeSize(advanceUserSpace, 0);

#if 0
  cairo_text_extents_t ctext;

  if (_cachedSizes)
    {
      int entry = glyph % _cacheSize;

      if (_cachedGlyphs[entry] == glyph)
        {
          return _cachedSizes[entry];
        }
      
      if (_cairo_extents_for_NSGlyph(_scaled, glyph, &ctext))
        {
          _cachedGlyphs[entry] = glyph;
          _cachedSizes[entry] = NSMakeSize(ctext.x_advance, ctext.y_advance);
          
          return _cachedSizes[entry];
        }
    }
  else
    {
      if (_cairo_extents_for_NSGlyph(_scaled, glyph, &ctext))
        {
          return NSMakeSize(ctext.x_advance, ctext.y_advance);
        }
    }
#endif
  return NSZeroSize;
}

- (NSGlyph) glyphForCharacter: (unichar)theChar
{
  CGFontRef face = [_faceInfo fontFace];

  CGGlyph result = OPFontGetGlyphWithCharacter(face, theChar);
  //NSLog(@"%s: Mapped '%@' to glyph # %d", __PRETTY_FUNCTION__, str, (int)result);
  return result;
}

- (NSGlyph) glyphWithName: (NSString *) glyphName
{
  CGFontRef face = [_faceInfo fontFace];
  CGGlyph result = CGFontGetGlyphWithGlyphName(face, glyphName);
  //  NSLog(@"%s: Mapped '%@' to glyph # %d", __PRETTY_FUNCTION__, glyphName, (int)result);
  return result;
}

- (NSRect) boundingRectForGlyph: (NSGlyph)glyph
{
#if 0
  cairo_text_extents_t ctext;

  if (_cairo_extents_for_NSGlyph(_scaled, glyph, &ctext))
    {
      return NSMakeRect(ctext.x_bearing, ctext.y_bearing,
                        ctext.width, ctext.height);
    }
#endif
  return NSMakeRect(0,0,10,10);
}

- (CGFloat) widthOfString: (NSString *)string
{
#if 0
  cairo_text_extents_t ctext;

  if (!string)
    {
      return 0.0;
    }

  cairo_scaled_font_text_extents(_scaled, [string UTF8String], &ctext);
  if (cairo_scaled_font_status(_scaled) == CAIRO_STATUS_SUCCESS)
    {
      return ctext.width;
    }
#endif
  return 100.0;
}

- (void) appendBezierPathWithGlyphs: (NSGlyph *)glyphs 
                              count: (int)length 
                       toBezierPath: (NSBezierPath *)path
{
#if 0
  cairo_format_t format = CAIRO_FORMAT_ARGB32;
  cairo_surface_t *isurface;
  cairo_t *ct;
  int ix = 400;
  int iy = 400;
  unsigned char *cdata;
  int i;
  unichar ustr[length+1];
  char str[3*length+1];
  unsigned char *b;
  unsigned int size = 3*length+1;
  cairo_status_t status;
  cairo_matrix_t font_matrix;

  for (i = 0; i < length; i++)
    {
      ustr[i] = glyphs[i];
    }
  ustr[length] = 0;

  b = (unsigned char *)str;
  if (!GSFromUnicode(&b, &size, ustr, length, 
                     NSUTF8StringEncoding, NULL, GSUniTerminate))
    {
      NSLog(@"Conversion failed for %@", 
            [NSString stringWithCharacters: ustr length: length]);
      return;
    }

  cdata = malloc(sizeof(char) * 4 * ix * iy);
  if (!cdata)
    {
      NSLog(@"Could not allocate drawing space for glyphs");
      return;
    }

  isurface = cairo_image_surface_create_for_data(cdata, format, ix, iy, 4*ix);
  status = cairo_surface_status(isurface);
  if (status != CAIRO_STATUS_SUCCESS)
    {
      NSLog(@"Error while creating surface: %s", 
            cairo_status_to_string(status));
      cairo_surface_destroy(isurface);
      free(cdata);
      return;
    }
 
  ct = cairo_create(isurface);
  if (cairo_status(ct) != CAIRO_STATUS_SUCCESS)
    {
      NSLog(@"Error while creating context: %s", 
            cairo_status_to_string(cairo_status(ct)));
      cairo_destroy(ct);
      cairo_surface_destroy(isurface);
      free(cdata);
      return;
    }

  // Use flip matrix
  cairo_matrix_init(&font_matrix, matrix[0], matrix[1], matrix[2],
                    -matrix[3], matrix[4], matrix[5]);
  cairo_set_font_matrix(ct, &font_matrix);
  if (cairo_status(ct) != CAIRO_STATUS_SUCCESS)
    {
      NSLog(@"Error while setting font matrix: %s", 
            cairo_status_to_string(cairo_status(ct)));
      cairo_destroy(ct);
      cairo_surface_destroy(isurface);
      free(cdata);
      return;
    }

  cairo_set_font_face(ct, [_faceInfo fontFace]);
  if (cairo_status(ct) != CAIRO_STATUS_SUCCESS)
    {
      NSLog(@"Error while setting font face: %s", 
            cairo_status_to_string(cairo_status(ct)));
      cairo_destroy(ct);
      cairo_surface_destroy(isurface);
      free(cdata);
      return;
    }

  // Set font options from the scaled font
  // FIXME: Instead of setting the matrix, setting the face, and setting
  // the options, we should be using cairo_set_scaled_font
  {
    cairo_font_options_t *options = cairo_font_options_create();
    cairo_scaled_font_get_font_options(_scaled, options);
    cairo_set_font_options(ct, options);
    cairo_font_options_destroy(options);
  }
  if (cairo_status(ct) != CAIRO_STATUS_SUCCESS)
    {
      NSLog(@"Error while setting font options: %s", 
            cairo_status_to_string(cairo_status(ct)));
      cairo_destroy(ct);
      cairo_surface_destroy(isurface);
      free(cdata);
      return;
    }

  if ([path elementCount] > 0)
    {
      NSPoint p;

      p = [path currentPoint];
      cairo_move_to(ct, floorf(p.x), floorf(p.y));
    }

  cairo_text_path(ct, str);
  if (cairo_status(ct) == CAIRO_STATUS_SUCCESS)
     {
      cairo_path_t *cpath;
      cairo_path_data_t *data;
      
      cpath = cairo_copy_path(ct);
      
      for (i = 0; i < cpath->num_data; i += cpath->data[i].header.length) 
        {
          data = &cpath->data[i];
          switch (data->header.type) 
            {
              case CAIRO_PATH_MOVE_TO:
                [path moveToPoint: NSMakePoint(data[1].point.x, data[1].point.y)];
                break;
              case CAIRO_PATH_LINE_TO:
                [path lineToPoint: NSMakePoint(data[1].point.x, data[1].point.y)];
                break;
              case CAIRO_PATH_CURVE_TO:
                [path curveToPoint: NSMakePoint(data[3].point.x, data[3].point.y) 
                      controlPoint1: NSMakePoint(data[1].point.x, data[1].point.y)
                      controlPoint2: NSMakePoint(data[2].point.x, data[2].point.y)];
                break;
              case CAIRO_PATH_CLOSE_PATH:
                [path closePath];
                break;
            }
        }
      cairo_path_destroy(cpath);
    }
  cairo_destroy(ct);
  cairo_surface_destroy(isurface);
  free(cdata);
#endif
}

#if 0
- (void) drawGlyphs: (const NSGlyph*)glyphs
             length: (int)length 
                 on: (cairo_t*)ct
{
  cairo_matrix_t font_matrix;
  unichar ustr[length+1];
  char str[3*length+1];
  unsigned char *b;
  int i;
  unsigned int size = 3*length+1;

  for (i = 0; i < length; i++)
    {
      ustr[i] = glyphs[i];
    }
  ustr[length] = 0;

  b = (unsigned char *)str;
  if (!GSFromUnicode(&b, &size, ustr, length, 
                     NSUTF8StringEncoding, NULL, GSUniTerminate))
    {
      NSLog(@"Conversion failed for %@", 
            [NSString stringWithCharacters: ustr length: length]);
      return;
    }

  cairo_matrix_init(&font_matrix, matrix[0], matrix[1], -matrix[2],
                    matrix[3], matrix[4], matrix[5]);
  cairo_set_font_matrix(ct, &font_matrix);
  if (cairo_status(ct) != CAIRO_STATUS_SUCCESS)
    {
      NSLog(@"Error while setting font matrix: %s", 
            cairo_status_to_string(cairo_status(ct)));
      return;
    }

  cairo_set_font_face(ct, [_faceInfo fontFace]);
  if (cairo_status(ct) != CAIRO_STATUS_SUCCESS)
    {
      NSLog(@"Error while setting font face: %s", 
            cairo_status_to_string(cairo_status(ct)));
      return;
    }

  // Set font options from the scaled font
  // FIXME: Instead of setting the matrix, setting the face, and setting
  // the options, we should be using cairo_set_scaled_font
  {
    cairo_font_options_t *options = cairo_font_options_create();
    cairo_scaled_font_get_font_options(_scaled, options);
    cairo_set_font_options(ct, options);
    cairo_font_options_destroy(options);
  }
  if (cairo_status(ct) != CAIRO_STATUS_SUCCESS)
    {
      NSLog(@"Error while setting font options: %s", 
            cairo_status_to_string(cairo_status(ct)));
      return;
    }

  cairo_show_text(ct, str);
  if (cairo_status(ct) != CAIRO_STATUS_SUCCESS)
    {
      NSLog(@"Error drawing string: '%s' for string %s", 
            cairo_status_to_string(cairo_status(ct)), str);
    }
}
#endif

@end
