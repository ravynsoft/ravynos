/*
   Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>

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

#import <Foundation/NSObject.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSDebug.h>
#import <GNUstepBase/Unicode.h>

#import <GNUstepGUI/GSFontInfo.h>
#import <AppKit/NSAffineTransform.h>
#import <AppKit/NSBezierPath.h>

#import "gsc/GSGState.h"

#import "ftfont.h"
#import "FTFontEnumerator.h"

#define DI (*di)

/** font handling interface **/

#include FT_FREETYPE_H

#include FT_CACHE_IMAGE_H
#include FT_CACHE_SMALL_BITMAPS_H
#include FT_CACHE_CHARMAP_H

#include FT_OUTLINE_H


/* TODO: finish screen font handling */


/*
from the back-art-subpixel-text defaults key
0: normal rendering
1: subpixel, rgb
2: subpixel, bgr
*/
static int subpixel_text;

@interface FTFontInfo_subpixel : FTFontInfo
@end

static FT_Library ft_library;
static FTC_Manager ftc_manager;
static FTC_ImageCache ftc_imagecache;
static FTC_SBitCache ftc_sbitcache;
static FTC_CMapCache ftc_cmapcache;

/*
 * Helper method used inside of FTC_Manager to create an FT_FACE.
 */
static FT_Error ft_get_face(FTC_FaceID fid, FT_Library lib, 
                            FT_Pointer data, FT_Face *pface)
{
  FT_Error err;
  NSArray *rfi = (NSArray *)fid;
  int i, c = [rfi count];
  const char *face_name = [[rfi objectAtIndex: 0] fileSystemRepresentation];

  NSDebugLLog(@"ftfont", @"ft_get_face: %@ '%s'", rfi, face_name);
  err = FT_New_Face(lib, face_name, 0, pface);
  if (err)
    {
      NSLog(@"Error when loading '%@' (%08x)", [rfi objectAtIndex: 0], err);
      return err;
    }

  for (i = 1; i < c; i++)
    {
      face_name = [[rfi objectAtIndex: i] fileSystemRepresentation];

      NSDebugLLog(@"ftfont", @"   do '%s'", face_name);
      err = FT_Attach_File(*pface, face_name);
      if (err)
        {
          NSLog(@"Error when loading '%@' (%08x)", [rfi objectAtIndex: i], err);
          /* pretend it's alright */
        }
    }

  return 0;
}


@implementation FTFontInfo

- (id) initWithFontName: (NSString *)name
                 matrix: (const CGFloat *)fmatrix
             screenFont: (BOOL)p_screenFont
{
  NSArray *rfi;
  FTFaceInfo *font_entry;
  FT_Error error;

  if (subpixel_text)
    {
      [self release];
      self = [FTFontInfo_subpixel alloc];
    }

  self = [super init];
  if (!self)
    return nil;

  screenFont = p_screenFont;

  NSDebugLLog(@"ftfont",
    @"[%@ -initWithFontName: %@  matrix: (%g %g %g %g %g %g)] %i",
    self, name,
    fmatrix[0], fmatrix[1], fmatrix[2],
    fmatrix[3], fmatrix[4], fmatrix[5],
    p_screenFont);

  font_entry = [FTFontEnumerator fontWithName: name];
  if (!font_entry)
    {
      RELEASE(self);
      return nil;
    }

  face_info = font_entry;

  weight = font_entry->weight;
  traits = font_entry->traits;

  fontName = [name copy];
  familyName = [face_info->familyName copy];
  memcpy(matrix, fmatrix, sizeof(matrix));

  /* Using utf8 is a bit ugly, but it works.  Besides, the bulk of the text
     comes as glyphs anyway.  */
  mostCompatibleStringEncoding = NSUTF8StringEncoding;
  encodingScheme = @"iso10646-1";

  if (screenFont)
    {
      /* Round up; makes the text more legible.  */
      matrix[0] = ceil(matrix[0]);
      if (matrix[3] < 0.0)
        matrix[3] = floor(matrix[3]);
      else
        matrix[3] = ceil(matrix[3]);
    }

  pix_width = fabs(matrix[0]);
  pix_height = fabs(matrix[3]);

  rfi = font_entry->files;
  if (screenFont && font_entry->num_sizes && pix_width == pix_height)
    {
      int i;

      for (i = 0; i < font_entry->num_sizes; i++)
        {
          if (font_entry->sizes[i].pixel_size == pix_width)
            {
              rfi = font_entry->sizes[i].files;
              break;
            }
        }
    }

  faceId = (FTC_FaceID)rfi;

  imageType.face_id = faceId;
  imageType.width = pix_width;
  imageType.height = pix_height;
  /* TODO: Flags? */

  scaler.face_id = faceId;
  scaler.width = pix_width;
  scaler.height = pix_height;
  scaler.pixel = 1;
  
  if ((error = FTC_Manager_LookupSize(ftc_manager, &scaler, &ft_size)))
    {
      NSLog(@"FTC_Manager_LookupSize() failed for '%@', error %08x!",
            name, error);
      RELEASE(self);
      return nil;
    }

  /* TODO: these are _really_ messed up when fonts are flipped */
  /* TODO: need to look carefully at these and make sure they are correct */
  ascender = fabs(((int)ft_size->metrics.ascender) / 64.0);
  descender = fabs(((int)ft_size->metrics.descender) / 64.0);
  lineHeight = (int)ft_size->metrics.height / 64.0;
  xHeight = (int)ft_size->metrics.y_ppem / 64.0;
  maximumAdvancement = NSMakeSize((ft_size->metrics.max_advance / 64.0), 0.0);

  fontBBox
    = NSMakeRect(0, -descender, maximumAdvancement.width, ascender + descender);
  descender = -descender;

/*        printf("(%@) h=%g  a=%g d=%g  max=(%g %g)  (%g %g)+(%g %g)\n",name,
                xHeight, ascender, descender,
                maximumAdvancement.width, maximumAdvancement.height,
                fontBBox.origin.x, fontBBox.origin.y,
                fontBBox.size.width, fontBBox.size.height);*/

  {
    int i;
    FT_Face face = ft_size->face;
    FT_CharMap cmap;
    FT_Encoding e;
    unicodeCmap = -1;

    if (!face)
      {
        NSLog(@"Found no face for font '%@'", name);
        RELEASE(self);
        return nil;
      }

    for (i = 0; i < face->num_charmaps; i++)
      {
        cmap = face->charmaps[i];
        e = cmap->encoding;
        if (e == FT_ENCODING_UNICODE)
          {
            unicodeCmap = i;
            break;
          }
      }
  }

  if (screenFont)
    {
      int flags;

      if (pix_width == pix_height && pix_width < 16 && pix_height >= 8)
        {
          int rh = face_info->render_hints_hack;

          if (rh & 0x10000)
            {
              flags = FT_LOAD_TARGET_NORMAL;
              rh = (rh >> 8) & 0xff;
            }
          else
            {
              flags = FT_LOAD_TARGET_MONO;
              rh = rh & 0xff;
            }
          if (rh & 1)
            flags |= FT_LOAD_FORCE_AUTOHINT;
          if (!(rh & 2))
            flags |= FT_LOAD_NO_HINTING;
        }
      else if (pix_width < 8)
        flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_HINTING;
      else
        flags = FT_LOAD_TARGET_NORMAL;
      imageType.flags = flags;
    }
  else
    {
      imageType.flags = FT_LOAD_NO_HINTING;
    }

  /*
  Here, we simply need to make sure that we don't get any false matches
  the first time a particular cache entry is used. Thus, we only need to
  initialize the first entry. For all other entries, cachedGlyph[i] will
  be 0, and that's a glyph that can't possibly hash to any entry except
  entry #0, so it won't cause any false matches.
  */
  cachedGlyph[0] = 1;

  return self;
}

- (NSString*) displayName
{
  return face_info->displayName;
}

- (void) set
{
  NSLog(@"ignore -set method of font '%@'", fontName);
}

- (NSCharacterSet*) coveredCharacterSet
{
  if (coveredCharacterSet == nil)
    {
      NSMutableCharacterSet *m = [NSMutableCharacterSet new];
      unsigned count = 0;
      FT_Face face;
      FT_ULong charcode;
      FT_UInt glyphindex;
      FT_Size size;

      if (FTC_Manager_LookupSize(ftc_manager, &scaler, &size))
          return nil;
      face = size->face;

     charcode = FT_Get_First_Char(face, &glyphindex);
      if (glyphindex != 0)
        {
          NSRange range;
          
          range.location = charcode;
          range.length = 1;
          
          while (glyphindex != 0)
            {
              count++;
              if (charcode >= 1114112)
                {
                  break;
                }
              if (charcode == NSMaxRange(range))
                {
                  range.length++;
                }
              else
                {
                  [m addCharactersInRange: range];
                  range.location = charcode;
                  range.length = 1;
                }
              charcode = FT_Get_Next_Char(face, charcode, &glyphindex);
            }
          if (range.length > 0)
            {
              [m addCharactersInRange: range];
            }
        }
      coveredCharacterSet = [m copy];
      numberOfGlyphs = count;
      RELEASE(m);
    }
  return coveredCharacterSet;
}

- (CGFloat) defaultLineHeightForFont
{
  return lineHeight;
}

- (NSUInteger) numberOfGlyphs
{
  if (coveredCharacterSet == nil)
    {
      [self coveredCharacterSet];
    }
  return numberOfGlyphs;
}

/* TODO: the current point probably needs updating after drawing is done */

/* draw string at point, clipped, w/given color and alpha, and possible deltas:
   flags & 0x1: data contains x offsets, use instead of glyph x advance
   flags & 0x2: data contains y offsets, use instead of glyph y advance
   flags & 0x4: data contains a single x and y offset, which should be added to
                font's advancements for each glyph; results are undefined if
                this option is combined with either x or y offsets (0x1,0x2)
   flags & 0x8: data contains a single x and y offset, which should be added to
                font's advancement for glyph identified by 'wch'; if combined
                with 0x4 deltas contain exactly two offsets for x and y, the
                first for every character, the second for 'wch'; results are
                undefined if 0x8 is combined with 0x2 or 0x1
 */
- (void) drawString: (const char *)s
  at: (int)x : (int)y
  to: (int)x0 : (int)y0 : (int)x1 : (int)y1
  : (unsigned char *)buf : (int)bpl
  : (unsigned char *)abuf : (int)abpl
  color: (unsigned char)r
  : (unsigned char)g
  : (unsigned char)b
  : (unsigned char)alpha
  transform: (NSAffineTransform *)transform
  deltas: (const CGFloat *)delta_data : (int)delta_size : (int)delta_flags
  widthChar: (int) wch
  drawinfo: (draw_info_t *)di
{
  const unsigned char *c;
  unsigned char ch;
  unsigned int uch;
  int d;

  unsigned int glyph;

  int use_sbit;

  FTC_SBit sbit;

  FT_Matrix ftmatrix;
  FT_Vector ftdelta;

  FT_Error error;
  NSAffineTransformStruct ts;


  if (!alpha)
    return;

  /* TODO: if we had guaranteed upper bounds on glyph image size we
     could do some basic clipping here */

  x1 -= x0;
  y1 -= y0;
  x -= x0;
  y -= y0;

  ts = [transform transformStruct];

/*        NSLog(@"[%@ draw using matrix: (%g %g %g %g %g %g)] transform=%@",
                self,
                matrix[0], matrix[1], matrix[2],
                matrix[3], matrix[4], matrix[5],
                transform
                );*/

  {
    float xx, xy, yx, yy;

    xx = matrix[0] * ts.m11 + matrix[1] * ts.m21;
    yx = matrix[0] * ts.m12 + matrix[1] * ts.m22;
    xy = matrix[2] * ts.m11 + matrix[3] * ts.m21;
    yy = matrix[2] * ts.m12 + matrix[3] * ts.m22;

    /* If we're drawing 'normal' text (unscaled, unrotated, reasonable
       size), we can and should use the sbit cache for screen fonts. */
    if (screenFont
      && fabs(xx - ((int)xx)) < 0.01 && fabs(yy - ((int)yy)) < 0.01
      && fabs(xy) < 0.01 && fabs(yx) < 0.01
      && xx < 72 && yy < 72 && xx > 0.5 && yy > 0.5)
      {
        use_sbit = 1;
      }
    else
      {
        float f;
        use_sbit = 0;

        /* TODO: Think hard about this.  Fancy truetype magic can
           significantly change the outline, e.g. by adding fancy adornments
           at large sizes.  If the font is then scaled down, we should
           probably set the font size to the original size and scale down
           using the matrix.  This ensures that adornments and such are
           still present, just scaled down.

           This is somewhat complicated by the fact that we don't have any
           real size, just a matrix.  Thus, we average pix_width and
           pix_height; we'll get the right answer for normal cases, and
           we can't really do anything about the weird cases.  */
        f = abs(pix_width) + abs(pix_height);
        if (f > 1)
          f = f / 2.0;
        else
          f = 1.0;
        f = (int)f;

        scaler.width = scaler.height = f;
        ftmatrix.xx = xx / f * 65536.0;
        ftmatrix.xy = xy / f * 65536.0;
        ftmatrix.yx = yx / f * 65536.0;
        ftmatrix.yy = yy / f * 65536.0;
        ftdelta.x = ftdelta.y = 0;
      }
  }


/*        NSLog(@"drawString: '%s' at: %i:%i  to: %i:%i:%i:%i:%p",
                s, x, y, x0, y0, x1, y1, buf);*/
  d=0;
  for (c = (const unsigned char *)s; *c; c++)
    {
/* TODO: do the same thing in outlineString:... */
      ch = *c;
      if (ch < 0x80)
        {
          uch = ch;
        }
      else if (ch < 0xc0)
        {
          uch = 0xfffd;
        }
      else if (ch < 0xe0)
        {
#define ADD_UTF_BYTE(shift, internal) \
  ch = *++c; \
  if (ch >= 0x80 && ch < 0xc0) \
    { \
      uch |= (ch & 0x3f) << shift; \
      internal \
    } \
  else \
    { \
      uch = 0xfffd; \
      c--; \
    }

          uch = (ch & 0x1f) << 6;
          ADD_UTF_BYTE(0,)
        }
      else if (ch < 0xf0)
        {
          uch = (ch & 0x0f) << 12;
          ADD_UTF_BYTE(6, ADD_UTF_BYTE(0,))
        }
      else if (ch < 0xf8)
        {
          uch = (ch & 0x07) << 18;
          ADD_UTF_BYTE(12, ADD_UTF_BYTE(6, ADD_UTF_BYTE(0,)))
        }
      else if (ch < 0xfc)
        {
          uch = (ch & 0x03) << 24;
          ADD_UTF_BYTE(18, ADD_UTF_BYTE(12, ADD_UTF_BYTE(6, ADD_UTF_BYTE(0,))))
        }
      else if (ch < 0xfe)
        {
          uch = (ch & 0x01) << 30;
          ADD_UTF_BYTE(24, ADD_UTF_BYTE(18, ADD_UTF_BYTE(12,
            ADD_UTF_BYTE(6, ADD_UTF_BYTE(0,)))))
        }
      else
        {
          uch = 0xfffd;
        }
#undef ADD_UTF_BYTE

      glyph = FTC_CMapCache_Lookup(ftc_cmapcache, faceId, unicodeCmap, uch);

      if (use_sbit)
        {
          if ((error = FTC_SBitCache_Lookup(ftc_sbitcache, &imageType,
            glyph, &sbit, NULL)))
            {
              NSLog(@"FTC_SBitCache_Lookup() failed with error %08x "
                @"(%08x, %08x, %ix%i, %08x)",
                error, glyph, (unsigned)imageType.face_id, imageType.width,
                imageType.height, imageType.flags);
              continue;
            }

          if (!sbit->buffer)
            {
              if (!delta_flags)
                {
                  x += sbit->xadvance;
                }
              else
                {
                  if (delta_flags & 0x1)
                    x += delta_data[d++];
                  if (delta_flags & 0x2)
                    y += (ts.m22 < 0) ?
                      delta_data[d++] : -delta_data[d++];
                  if (delta_flags & 0x4)
                    {
                      x += sbit->xadvance + delta_data[0];
                      y += /*sbit->yadvance +*/ (ts.m22 < 0) ?
                        delta_data[1] : -delta_data[1];
                      if ((delta_flags & 0x8) && (uch == wch))
                        {
                          x += delta_data[2];
                          y += (ts.m22 < 0) ?  delta_data[3] : -delta_data[3];
                        }
                    }
                  else if (delta_flags & 0x8)
                    {
                      if (uch == wch)
                        {
                          x += sbit->xadvance + delta_data[0];
                          y += /*sbit->yadvance +*/ (ts.m22 < 0) ?
                            delta_data[1] : -delta_data[1];
                        }
                      else
                        {
                          x += sbit->xadvance;
                          /*y += sbit->yadvance;*/
                        }
                    }
                }
              continue;
            }

          if (sbit->format == ft_pixel_mode_grays)
            {
              int gx = x + sbit->left, gy = y - sbit->top;
              int sbpl = sbit->pitch;
              int sx = sbit->width, sy = sbit->height;
              const unsigned char *src = sbit->buffer;
              unsigned char *dst = buf;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  if (alpha >= 255)
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA_OPAQUE(dst, src, r, g, b, sx);
                  else
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA(dst, src, r, g, b, alpha, sx);
                }
            }
          else if (sbit->format == ft_pixel_mode_mono)
            {
              int gx = x + sbit->left, gy = y - sbit->top;
              int sbpl = sbit->pitch;
              int sx = sbit->width, sy = sbit->height;
              const unsigned char *src = sbit->buffer;
              unsigned char *dst = buf;
              int src_ofs = 0;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx / 8;
                  src_ofs = (-gx) & 7;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  if (alpha >= 255)
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_MONO_OPAQUE(dst, src, src_ofs, r, g, b, sx);
                  else
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_MONO(dst, src, src_ofs, r, g, b, alpha, sx);
                }
            }
          else
            {
              NSLog(@"unhandled font bitmap format %i", sbit->format);
            }

          if (!delta_flags)
            {
              x += sbit->xadvance;
            }
          else
            {
              if (delta_flags & 0x1)
                  x += delta_data[d++];
              if (delta_flags & 0x2)
                  y += (ts.m22 < 0) ?  delta_data[d++] : -delta_data[d++];
              if (delta_flags & 0x4)
                {
                  x += sbit->xadvance + delta_data[0];
                  y += /*sbit->yadvance +*/ (ts.m22 < 0) ?
                          delta_data[1] : -delta_data[1];
                  if ((delta_flags & 0x8) && (uch == wch))
                    {
                      x += delta_data[2];
                      y += (ts.m22 < 0) ?  delta_data[3] : -delta_data[3];
                    }
                }
              else if (delta_flags & 0x8)
                {
                  if (uch == wch)
                    {
                      x += sbit->xadvance + delta_data[0];
                      y += /*sbit->yadvance +*/ (ts.m22 < 0) ?
                          delta_data[1] : -delta_data[1];
                    }
                  else
                    {
                      x += sbit->xadvance;
                      /*y += sbit->yadvance;*/
                    }
                }
            }
        }
      else
        {
          FT_Face face;
          FT_Glyph gl;
          FT_Size size;
          FT_BitmapGlyph gb;

          if ((error=FTC_Manager_LookupSize(ftc_manager, &scaler, &size)))
            {
              NSLog(@"FTC_Manager_Lookup_Size() failed with error %08x", error);
              continue;
            }
          face = size->face;

          /* TODO: for screen fonts we should probably still hint */
          if ((error=FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)))
            {
              NSLog(@"FT_Load_Glyph() failed with error %08x", error);
              continue;
            }

          if ((error=FT_Get_Glyph(face->glyph, &gl)))
            {
              NSLog(@"FT_Get_Glyph() failed with error %08x", error);
              continue;
            }

          if ((error=FT_Glyph_Transform(gl, &ftmatrix, &ftdelta)))
            {
              NSLog(@"FT_Glyph_Transform() failed with error %08x", error);
              continue;
            }
          if ((error=FT_Glyph_To_Bitmap(&gl, ft_render_mode_normal, 0, 1)))
            {
              NSLog(@"FT_Glyph_To_Bitmap() failed with error %08x", error);
              FT_Done_Glyph(gl);
              continue;
            }
          gb = (FT_BitmapGlyph)gl;


          if (gb->bitmap.pixel_mode == ft_pixel_mode_grays)
            {
              int gx = x + gb->left, gy = y - gb->top;
              int sbpl = gb->bitmap.pitch;
              int sx = gb->bitmap.width, sy = gb->bitmap.rows;
              const unsigned char *src = gb->bitmap.buffer;
              unsigned char *dst = buf;
              unsigned char *dsta = abuf;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                  if (dsta)
                    dsta += abpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                  if (dsta)
                    dsta += gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  if (dsta)
                    for (; gy < sy; gy++, src += sbpl, dst += bpl, dsta += abpl)
                      RENDER_BLIT_ALPHA_A(dst, dsta, src, r, g, b, alpha, sx);
                  else if (alpha >= 255)
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA_OPAQUE(dst, src, r, g, b, sx);
                  else
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA(dst, src, r, g, b, alpha, sx);
                }
            }
/* TODO: will this case ever appear? */
/*                        else if (gb->bitmap.pixel_mode==ft_pixel_mode_mono)*/
          else
            {
              NSLog(@"unhandled font bitmap format %i", gb->bitmap.pixel_mode);
            }

          if (!delta_flags)
            {
              ftdelta.x += gl->advance.x >> 10;
              ftdelta.y += gl->advance.y >> 10;
            }
          else
            {
              if (delta_flags & 0x1)
                ftdelta.x += delta_data[d++] * 64.0;
              if (delta_flags & 0x2)
                ftdelta.y += delta_data[d++] * 64.0;
              if (delta_flags & 0x4)
                {
                  ftdelta.x += (gl->advance.x >> 10) + delta_data[0] * 64.0;
                  ftdelta.y += (gl->advance.y >> 10) + delta_data[1] * 64.0;
                  if ((delta_flags & 0x8) && (uch == wch))
                    {
                      ftdelta.x += delta_data[2] * 64.0;
                      ftdelta.y += delta_data[3] * 64.0;
                    }
                }
              else if (delta_flags & 0x8)
                {
                  if (uch == wch)
                    {
                      ftdelta.x += (gl->advance.x >> 10) + delta_data[0] * 64.0;
                      ftdelta.y += (gl->advance.y >> 10) + delta_data[1] * 64.0;
                    }
                  else
                    {
                      ftdelta.x += gl->advance.x >> 10;
                      ftdelta.y += gl->advance.y >> 10;
                    }
                }
            }

          FT_Done_Glyph(gl);
        }
    }
}


- (void) drawGlyphs: (const NSGlyph *)glyphs : (int)length
        at: (int)x : (int)y
        to: (int)x0 : (int)y0 : (int)x1 : (int)y1
        : (unsigned char *)buf : (int)bpl
        color: (unsigned char)r : (unsigned char)g : (unsigned char)b
        : (unsigned char)alpha
        transform: (NSAffineTransform *)transform
        drawinfo: (struct draw_info_s *)di
{
  unsigned int glyph;

  int use_sbit;

  FTC_SBit sbit;

  FT_Matrix ftmatrix;
  FT_Vector ftdelta;

  FT_Error error;
  NSAffineTransformStruct ts;


  if (!alpha)
    return;

  /* TODO: if we had guaranteed upper bounds on glyph image size we
     could do some basic clipping here */

  x1 -= x0;
  y1 -= y0;
  x -= x0;
  y -= y0;

  ts = [transform transformStruct];

/*        NSLog(@"[%@ draw using matrix: (%g %g %g %g %g %g)] transform=%@",
                self,
                matrix[0], matrix[1], matrix[2],
                matrix[3], matrix[4], matrix[5],
                transform
                );*/

  {
    float xx, xy, yx, yy;

    xx = matrix[0] * ts.m11 + matrix[1] * ts.m21;
    yx = matrix[0] * ts.m12 + matrix[1] * ts.m22;
    xy = matrix[2] * ts.m11 + matrix[3] * ts.m21;
    yy = matrix[2] * ts.m12 + matrix[3] * ts.m22;
 
    /* If we're drawing 'normal' text (unscaled, unrotated, reasonable
       size), we can and should use the sbit cache for screen fonts. */
    if (screenFont
      && fabs(xx - ((int)xx)) < 0.01 && fabs(yy - ((int)yy)) < 0.01
      && fabs(xy) < 0.01 && fabs(yx) < 0.01
      && xx < 72 && yy < 72 && xx > 0.5 && yy > 0.5)
      {
        use_sbit = 1;
      }
    else
      {
        float f;
        use_sbit = 0;

        f = abs(pix_width) + abs(pix_height);
        if (f > 1)
          f = f / 2.0;
        else
          f = 1.0;
        f = (int)f;

        scaler.width = scaler.height = f;
        ftmatrix.xx = xx / f * 65536.0;
        ftmatrix.xy = xy / f * 65536.0;
        ftmatrix.yx = yx / f * 65536.0;
        ftmatrix.yy = yy / f * 65536.0;
        ftdelta.x = ftdelta.y = 0;
      }
  }

/*        NSLog(@"drawGlyphs: '%p' at: %i:%i  to: %i:%i:%i:%i:%p",
                glyphs, x, y, x0, y0, x1, y1, buf);*/

  for (; length; length--, glyphs++)
    {
      glyph = *glyphs - 1;

      if (use_sbit)
        {
          if ((error = FTC_SBitCache_Lookup(ftc_sbitcache, &imageType,
            glyph, &sbit, NULL)))
            {
              NSLog(@"FTC_SBitCache_Lookup() failed with error %08x "
                @"(%08x, %08x, %ix%i, %08x)",
                error, glyph, (unsigned)imageType.face_id,
                imageType.width, imageType.height,
                imageType.flags);
              continue;
            }

          if (!sbit->buffer)
            {
              x += sbit->xadvance;
              continue;
            }

          if (sbit->format == ft_pixel_mode_grays)
            {
              int gx = x + sbit->left, gy = y - sbit->top;
              int sbpl = sbit->pitch;
              int sx = sbit->width, sy = sbit->height;
              const unsigned char *src = sbit->buffer;
              unsigned char *dst = buf;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  if (alpha >= 255)
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA_OPAQUE(dst, src, r, g, b, sx);
                  else
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA(dst, src, r, g, b, alpha, sx);
                }
            }
          else if (sbit->format == ft_pixel_mode_mono)
            {
              int gx = x + sbit->left, gy = y - sbit->top;
              int sbpl = sbit->pitch;
              int sx = sbit->width, sy = sbit->height;
              const unsigned char *src = sbit->buffer;
              unsigned char *dst = buf;
              int src_ofs = 0;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx / 8;
                  src_ofs = (-gx) & 7;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  if (alpha >= 255)
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_MONO_OPAQUE(dst, src, src_ofs, r, g, b, sx);
                  else
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_MONO(dst, src, src_ofs, r, g, b, alpha, sx);
                }
            }
          else
            {
              NSLog(@"unhandled font bitmap format %i", sbit->format);
            }

          x += sbit->xadvance;
        }
      else
        {
          FT_Face face;
          FT_Size size;
          FT_Glyph gl;
          FT_BitmapGlyph gb;

          if ((error=FTC_Manager_LookupSize(ftc_manager, &scaler, &size)))
            {
              NSLog(@"FTC_Manager_Lookup_Size() failed with error %08x",error);
              continue;
            }
          face = size->face;

          /* TODO: for screen fonts ...see above... rotations of 90, 180, 270,
           * and integer scales hinting might still be a good idea. */
          if ((error = FT_Load_Glyph(face, glyph,
            FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)))
            {
              NSLog(@"FT_Load_Glyph() failed with error %08x", error);
              continue;
            }

          if ((error=FT_Get_Glyph(face->glyph, &gl)))
            {
              NSLog(@"FT_Get_Glyph() failed with error %08x", error);
              continue;
            }

          if ((error=FT_Glyph_Transform(gl, &ftmatrix, &ftdelta)))
            {
              NSLog(@"FT_Glyph_Transform() failed with error %08x", error);
              continue;
            }
          if ((error=FT_Glyph_To_Bitmap(&gl, ft_render_mode_normal, 0, 1)))
            {
              NSLog(@"FT_Glyph_To_Bitmap() failed with error %08x", error);
              FT_Done_Glyph(gl);
              continue;
            }
          gb = (FT_BitmapGlyph)gl;


          if (gb->bitmap.pixel_mode == ft_pixel_mode_grays)
            {
              int gx = x + gb->left, gy = y - gb->top;
              int sbpl = gb->bitmap.pitch;
              int sx = gb->bitmap.width, sy = gb->bitmap.rows;
              const unsigned char *src = gb->bitmap.buffer;
              unsigned char *dst = buf;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  if (alpha >= 255)
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA_OPAQUE(dst, src, r, g, b, sx);
                  else
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA(dst, src, r, g, b, alpha, sx);
                }
            }
/* TODO: will this case ever appear? */
/*                        else if (gb->bitmap.pixel_mode==ft_pixel_mode_mono)*/
          else
            {
              NSLog(@"unhandled font bitmap format %i", gb->bitmap.pixel_mode);
            }

          ftdelta.x += gl->advance.x >> 10;
          ftdelta.y += gl->advance.y >> 10;

          FT_Done_Glyph(gl);
        }
    }
}

- (void) drawGlyphs: (const NSGlyph *)glyphs : (int)length
        at: (int)x : (int)y
        to: (int)x0 : (int)y0 : (int)x1 : (int)y1
        : (unsigned char *)buf : (int)bpl
        alpha: (unsigned char *)abuf : (int)abpl
        color: (unsigned char)r : (unsigned char)g : (unsigned char)b
        : (unsigned char)alpha
        transform: (NSAffineTransform *)transform
        drawinfo: (struct draw_info_s *)di
{
  unsigned int glyph;

  int use_sbit;

  FTC_SBit sbit;

  FT_Matrix ftmatrix;
  FT_Vector ftdelta;

  FT_Error error;
  NSAffineTransformStruct ts;


  if (!alpha)
    return;

  /* TODO: if we had guaranteed upper bounds on glyph image size we
     could do some basic clipping here */

  x1 -= x0;
  y1 -= y0;
  x -= x0;
  y -= y0;

  ts = [transform transformStruct];

/*        NSLog(@"[%@ draw using matrix: (%g %g %g %g %g %g)] transform=%@",
                self,
                matrix[0], matrix[1], matrix[2],
                matrix[3], matrix[4], matrix[5],
                transform
                );*/

  {
    float xx, xy, yx, yy;

    xx = matrix[0] * ts.m11 + matrix[1] * ts.m21;
    yx = matrix[0] * ts.m12 + matrix[1] * ts.m22;
    xy = matrix[2] * ts.m11 + matrix[3] * ts.m21;
    yy = matrix[2] * ts.m12 + matrix[3] * ts.m22;
 
    /* If we're drawing 'normal' text (unscaled, unrotated, reasonable
       size), we can and should use the sbit cache for screen fonts. */
    if (screenFont
      && fabs(xx - ((int)xx)) < 0.01 && fabs(yy - ((int)yy)) < 0.01
      && fabs(xy) < 0.01 && fabs(yx) < 0.01
      && xx < 72 && yy < 72 && xx > 0.5 && yy > 0.5)
      {
        use_sbit = 1;
      }
    else
      {
        float f;
        use_sbit = 0;

        f = abs(pix_width) + abs(pix_height);
        if (f > 1)
          f = f / 2.0;
        else
          f = 1.0;
        f = (int)f;

        scaler.width = scaler.height = f;
        ftmatrix.xx = xx / f * 65536.0;
        ftmatrix.xy = xy / f * 65536.0;
        ftmatrix.yx = yx / f * 65536.0;
        ftmatrix.yy = yy / f * 65536.0;
        ftdelta.x = ftdelta.y = 0;
      }
  }

/*        NSLog(@"drawString: '%s' at: %i:%i  to: %i:%i:%i:%i:%p",
                s, x, y, x0, y0, x1, y1, buf);*/

  for (; length; length--, glyphs++)
    {
      glyph = *glyphs - 1;

      if (use_sbit)
        {
          if ((error = FTC_SBitCache_Lookup(ftc_sbitcache, &imageType, glyph, &sbit, NULL)))
            {
              if (glyph != 0xffffffff)
                NSLog(@"FTC_SBitCache_Lookup() failed with error %08x (%08x, %08x, %ix%i, %08x)",
                  error, glyph, (unsigned)imageType.face_id, imageType.width, imageType.height,
                  imageType.flags
                );
              continue;
            }

          if (!sbit->buffer)
            {
              x += sbit->xadvance;
              continue;
            }

          if (sbit->format == ft_pixel_mode_grays)
            {
              int gx = x + sbit->left, gy = y - sbit->top;
              int sbpl = sbit->pitch;
              int sx = sbit->width, sy = sbit->height;
              const unsigned char *src = sbit->buffer;
              unsigned char *dst = buf;
              unsigned char *adst = abuf;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                  adst += abpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                  adst += gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  for (; gy < sy; gy++, src += sbpl, dst += bpl, adst += abpl)
                    RENDER_BLIT_ALPHA_A(dst, adst, src, r, g, b, alpha, sx);
                }
            }
          else if (sbit->format == ft_pixel_mode_mono)
            {
              int gx = x + sbit->left, gy = y - sbit->top;
              int sbpl = sbit->pitch;
              int sx = sbit->width, sy = sbit->height;
              const unsigned char *src = sbit->buffer;
              unsigned char *dst = buf;
              unsigned char *adst = abuf;
              int src_ofs = 0;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                  adst += abpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx / 8;
                  src_ofs = (-gx) & 7;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                  adst += gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  for (; gy < sy; gy++, src += sbpl, dst += bpl, adst += bpl)
                    RENDER_BLIT_MONO_A(dst, adst, src, src_ofs, r, g, b, alpha, sx);
                }
            }
          else
            {
              NSLog(@"unhandled font bitmap format %i", sbit->format);
            }

          x += sbit->xadvance;
        }
      else
        {
          FT_Face face;
          FT_Size size;
          FT_Glyph gl;
          FT_BitmapGlyph gb;

          if ((error=FTC_Manager_LookupSize(ftc_manager, &scaler, &size)))
            {
              NSLog(@"FTC_Manager_Lookup_Size() failed with error %08x", error);
              continue;
            }
          face = size->face;

          /* TODO: for screen fonts, see above, etc., rotations of 90, 180, 270, and integer
             scales hinting might still be a good idea. */
          if ((error=FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)))
            {
              NSLog(@"FT_Load_Glyph() failed with error %08x", error);
              continue;
            }

          if ((error=FT_Get_Glyph(face->glyph, &gl)))
            {
              NSLog(@"FT_Get_Glyph() failed with error %08x", error);
              continue;
            }

          if ((error=FT_Glyph_Transform(gl, &ftmatrix, &ftdelta)))
            {
              NSLog(@"FT_Glyph_Transform() failed with error %08x", error);
              continue;
            }
          if ((error=FT_Glyph_To_Bitmap(&gl, ft_render_mode_normal, 0, 1)))
            {
              NSLog(@"FT_Glyph_To_Bitmap() failed with error %08x", error);
              FT_Done_Glyph(gl);
              continue;
            }
          gb = (FT_BitmapGlyph)gl;


          if (gb->bitmap.pixel_mode == ft_pixel_mode_grays)
            {
              int gx = x + gb->left, gy = y - gb->top;
              int sbpl = gb->bitmap.pitch;
              int sx = gb->bitmap.width, sy = gb->bitmap.rows;
              const unsigned char *src = gb->bitmap.buffer;
              unsigned char *dst = buf;
              unsigned char *adst = abuf;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                  adst += abpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                  adst += gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  for (; gy < sy; gy++, src += sbpl, dst += bpl, adst += bpl)
                    RENDER_BLIT_ALPHA_A(dst, adst, src, r, g, b, alpha, sx);
                }
            }
/* TODO: will this case ever appear? */
/*                        else if (gb->bitmap.pixel_mode==ft_pixel_mode_mono)*/
          else
            {
              NSLog(@"unhandled font bitmap format %i", gb->bitmap.pixel_mode);
            }

          ftdelta.x += gl->advance.x >> 10;
          ftdelta.y += gl->advance.y >> 10;

          FT_Done_Glyph(gl);
        }
    }
}


- (BOOL) glyphIsEncoded: (NSGlyph)glyph
{
  FT_Face face;
  FT_Size size;
  FT_Error error;

  glyph--;
  if ((error=FTC_Manager_LookupSize(ftc_manager, &scaler, &size)))
    {
      NSLog(@"FTC_Manager_Lookup_Size() failed with error %08x",error);
      return NO;
    }
  face = size->face;

  if ((error=FT_Load_Glyph(face, glyph, 0)))
    {
      NSLog(@"FT_Load_Glyph() failed with error %08x",error);
      return NO;
    }

  return YES;
}


- (NSSize) advancementForGlyph: (NSGlyph)glyph
{
  FT_Error error;

  if (glyph == NSControlGlyph
   || glyph == GSAttachmentGlyph)
    return NSZeroSize;

  if (glyph != NSNullGlyph)
    glyph--;
  if (screenFont)
    {
      int entry = glyph % CACHE_SIZE;
      FTC_SBit sbit;

      if (cachedGlyph[entry] == glyph)
        return cachedSize[entry];

      if ((error = FTC_SBitCache_Lookup(ftc_sbitcache, &imageType, glyph, &sbit, NULL)))
        {
          NSLog(@"FTC_SBitCache_Lookup() failed with error %08x (%08x, %08x, %ix%i, %08x)",
            error, glyph, (unsigned)imageType.face_id,
            imageType.width, imageType.height,
            imageType.flags
        );
          return NSZeroSize;
        }

      cachedGlyph[entry] = glyph;
      cachedSize[entry] = NSMakeSize(sbit->xadvance, sbit->yadvance);
      return cachedSize[entry];
    }
  else
    {
      FT_Face face;
      FT_Size size;
      FT_Glyph gl;
      FT_Matrix ftmatrix;
      FT_Vector ftdelta;
      float f;
      NSSize s;

      f = fabs(matrix[0] * matrix[3] - matrix[1] * matrix[2]);
      if (f > 1)
        f = sqrt(f);
      else
        f = 1.0;

      f = (int)f;

      /* TODO? scalers and stuff, need to review */
      ftmatrix.xx = matrix[0] / f * 65536.0;
      ftmatrix.xy = matrix[1] / f * 65536.0;
      ftmatrix.yx = matrix[2] / f * 65536.0;
      ftmatrix.yy = matrix[3] / f * 65536.0;
      ftdelta.x = ftdelta.y = 0;

      if (FTC_Manager_LookupSize(ftc_manager, &scaler, &size))
        return NSZeroSize;
      face = size->face;

      if (FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP))
        return NSZeroSize;

      if (FT_Get_Glyph(face->glyph, &gl))
        return NSZeroSize;

      if (FT_Glyph_Transform(gl, &ftmatrix, &ftdelta))
        return NSZeroSize;

      s = NSMakeSize(gl->advance.x / 65536.0, gl->advance.y / 65536.0);

      FT_Done_Glyph(gl);

      return s;
    }
}

- (NSRect) boundingRectForGlyph: (NSGlyph)glyph
{
  FT_BBox bbox;
  FT_Glyph g;
  FT_Error error;

  glyph--;
/* TODO: this is ugly */
  if ((error=FTC_ImageCache_Lookup(ftc_imagecache, &imageType, glyph, &g, NULL)))
    {
      NSLog(@"FTC_ImageCache_Lookup() failed with error %08x",error);
//                NSLog(@"boundingRectForGlyph: %04x -> %i", aGlyph, glyph);
      return fontBBox;
    }

  FT_Glyph_Get_CBox(g, ft_glyph_bbox_gridfit, &bbox);

/*        printf("got cbox for %04x: %i, %i - %i, %i",
                aGlyph, bbox.xMin, bbox.yMin, bbox.xMax, bbox.yMax);*/

  return NSMakeRect(bbox.xMin / 64.0, bbox.yMin / 64.0,
                    (bbox.xMax - bbox.xMin) / 64.0, (bbox.yMax - bbox.yMin) / 64.0);
}

- (NSPoint) positionOfGlyph: (NSGlyph)g
        precededByGlyph: (NSGlyph)prev
        isNominal: (BOOL *)nominal
{
  NSPoint a;
  FT_Face face;
  FT_Size size;
  FT_Vector vec;
  FT_GlyphSlot glyph;

  if (nominal)
    *nominal = YES;

  if (g == NSControlGlyph || prev == NSControlGlyph)
    return NSZeroPoint;

  g--;
  prev--;

  if (FTC_Manager_LookupSize(ftc_manager, &scaler, &size))
    return NSZeroPoint;
  face = size->face;

  if (FT_Load_Glyph(face, prev, FT_LOAD_DEFAULT))
    return NSZeroPoint;

  glyph = face->glyph;
  a = NSMakePoint(glyph->advance.x / 64.0, glyph->advance.y / 64.0);

  if (FT_Get_Kerning(face, prev, g, ft_kerning_default, &vec))
    return a;

  if (vec.x == 0 && vec.y == 0)
    return a;

  if (nominal)
    *nominal = NO;

  a.x += vec.x / 64.0;
  a.y += vec.y / 64.0;
  return a;
}


- (CGFloat) widthOfString: (NSString*)string
{
  unichar ch;
  int i, c = [string length];
  int total;

  unsigned int glyph;

  FTC_SBit sbit;


  total = 0;
  for (i = 0; i < c; i++)
    {
      ch = [string characterAtIndex: i];
      glyph = FTC_CMapCache_Lookup(ftc_cmapcache, faceId, unicodeCmap, ch);

      /* TODO: shouldn't use sbit cache for this */
      if (1)
        {
          if (FTC_SBitCache_Lookup(ftc_sbitcache, &imageType, glyph, &sbit, NULL))
            continue;

          total += sbit->xadvance;
        }
      else
        {
          NSLog(@"non-sbit code not implemented");
        }
    }
  return total;
}


- (NSGlyph) glyphWithName: (NSString *)glyphName
{
  FT_Face face;
  FT_Size size;
  NSGlyph g;

  if (FTC_Manager_LookupSize(ftc_manager, &scaler, &size))
    return NSNullGlyph;
  face = size->face;

  g = FT_Get_Name_Index(face, (FT_String *)[glyphName lossyCString]);
  if (g)
    return g + 1;

  return NSNullGlyph;
}


/*

conic: (a,b,c)
p=(1-t)^2*a + 2*(1-t)*t*b + t^2*c

cubic: (a,b,c,d)
p=(1-t)^3*a + 3*(1-t)^2*t*b + 3*(1-t)*t^2*c + t^3*d



p(t)=(1-t)^3*a + 3*(1-t)^2*t*b + 3*(1-t)*t^2*c + t^3*d
t=m+ns=
n=l-m


q(s)=p(m+ns)=

(d-3c+3b-a)*n^3 * s^3 +
((3d-9c+9b-3a)*m+3c-6b+3a)*n^2 * s^2 +
((3d-9c+9b-3a)*m^2+(6c-12b+6a)*m+3b-3a)*n * s +
(d-3c+3b-a)*m^3+(3c-6b+3a)*m^2+(3b-3a)m+a


q(t)=(1-t)^3*aa + 3*(1-t)^2*t*bb + 3*(1-t)*t^2*cc + t^3*dd =

(dd-3cc+3bb-aa)*t^3 +
(3cc-6bb+3aa)*t^2 +
(3bb-3aa)*t +
aa


aa = (d-3*c+3*b-a)*m^3+(3*c-6*b+3*a)*m^2+(3*b-3*a)*m+a
3*bb-3*aa = ((3*d-9*c+9*b-3*a)*m^2+(6*c-12*b+6*a)*m+3*b-3*a)*n
3*cc-6*bb+3*aa = ((3*d-9*c+9*b-3*a)*m+3*c-6*b+3*a)*n^2
dd-3*cc+3*bb-aa = (d-3*c+3*b-a)*n^3


aa= (d - 3c + 3b - a) m^3  + (3c - 6b + 3a) m^2  + (3b - 3a) m + a

bb= ((d - 3c + 3b -  a) m^2  + (2c - 4b + 2a) m +  b -  a) n
  + aa

cc= ((d - 3c + 3b - a) m + c - 2b + a) n^2
 + 2*bb
 + aa

dd= (d - 3c + 3b - a) n^3
 + 3*cc
 + 3*bb
 + aa




p(t) = (1-t)^2*e + 2*(1-t)*t*f + t^2*g
 ~=
q(t) = (1-t)^3*a + 3*(1-t)^2*t*b + 3*(1-t)*t^2*c + t^3*d


p(0)=q(0) && p(1)=q(1) ->
a=e
d=g


p(0.5) = 1/8*(2a + 4f + 2d)
q(0.5) = 1/8*(a + 3*b + 3*c + d)

b+c=1/3*(a+4f+d)

p(1/4) = 1/64*
p(3/4) = 1/64*(4e+24f+36g)

q(1/4) = 1/64*
q(3/4) = 1/64*(a +  9b + 27c + 27d)

3b+c=1/3*(3a+8f+d)


3b+c=1/3*(3a+8f+d)
 b+c=1/3*(a+4f+d)

b=1/3*(e+2f)
c=1/3*(2f+g)


q(t) = (1-t)^3*e + (1-t)^2*t*(e+2f) + (1-t)*t^2*(2f+g) + t^3*g =
((1-t)^3+(1-t)^2*t)*e + (1-t)^2*t*2f + (1-t)*t^2*2f + (t^3+(1-t)*t^2)*g =

((1-t)^3+(1-t)^2*t)*e + 2f*(t*(1-t)*((1-t)+t)) + (t^3+(1-t)*t^2)*g =
((1-t)^3+(1-t)^2*t)*e + 2*(1-t)*t*f + (t^3+(1-t)*t^2)*g =
(1-t)^2*e + 2*(1-t)*t*f + t^2*g

p(t)=q(t)

*/

/* TODO: try to combine charpath and NSBezierPath handling? */
#if 0
static int charpath_move_to(const FT_Vector *to, void *user)
{
  GSGState *self = (GSGState *)user;
  NSPoint d;
  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
  [self DPSclosepath]; /* TODO: this isn't completely correct */
  [self DPSmoveto: d.x:d.y];
  return 0;
}

static int charpath_line_to(const FT_Vector *to, void *user)
{
  GSGState *self = (GSGState *)user;
  NSPoint d;
  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
  [self DPSlineto: d.x:d.y];
  return 0;
}

static int charpath_conic_to(const FT_Vector *c1, const FT_Vector *to, void *user)
{
  GSGState *self = (GSGState *)user;
  NSPoint a, b, c, d;
  [self DPScurrentpoint: &a.x:&a.y];
  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
  b.x = c1->x / 65536.0;
  b.y = c1->y / 65536.0;
  c.x = (b.x * 2 + d.x) / 3.0;
  c.y = (b.y * 2 + d.y) / 3.0;
  b.x = (b.x * 2 + a.x) / 3.0;
  b.y = (b.y * 2 + a.y) / 3.0;
  [self DPScurveto: b.x:b.y : c.x:c.y : d.x:d.y];
  return 0;
}

static int charpath_cubic_to(const FT_Vector *c1, const FT_Vector *c2, 
                             const FT_Vector *to, void *user)
{
  GSGState *self = (GSGState *)user;
  NSPoint b, c, d;
  b.x = c1->x / 65536.0;
  b.y = c1->y / 65536.0;
  c.x = c2->x / 65536.0;
  c.y = c2->y / 65536.0;
  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
  [self DPScurveto: b.x:b.y : c.x:c.y : d.x:d.y];
  return 0;
}

static FT_Outline_Funcs charpath_funcs = {
  move_to:charpath_move_to,
  line_to:charpath_line_to,
  conic_to:charpath_conic_to,
  cubic_to:charpath_cubic_to,
  shift:10,
  delta:0,
};
#endif

static int bezierpath_move_to(const FT_Vector *to, void *user)
{
  NSBezierPath *path = (NSBezierPath *)user;
  NSPoint d;
  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
  [path closePath]; /* TODO: this isn't completely correct */
  [path moveToPoint: d];
  return 0;
}

static int bezierpath_line_to(const FT_Vector *to, void *user)
{
  NSBezierPath *path = (NSBezierPath *)user;
  NSPoint d;
  d.x = to->x / 65536.0;
  d.y = to->y / 65536.0;
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
  [path curveToPoint: d controlPoint1: b controlPoint2: c];
  return 0;
}

static FT_Outline_Funcs bezierpath_funcs = {
  .move_to = bezierpath_move_to,
  .line_to = bezierpath_line_to,
  .conic_to = bezierpath_conic_to,
  .cubic_to = bezierpath_cubic_to,
  .shift = 10,
  .delta = 0,
};


/* TODO: sometimes gets 'glyph transformation failed', probably need to
add code to avoid loading bitmaps for glyphs */
- (void) outlineString: (const char *)s
                   at: (CGFloat)x : (CGFloat)y
               gstate: (void *)func_param
{
#if 0
  unichar *c;
  int i;
  FTC_CMapDescRec cmap;
  unsigned int glyph;

  unichar *uch;
  int ulen;

  FTC_ImageTypeRec cur;


  FT_Matrix ftmatrix;
  FT_Vector ftdelta;

  ftmatrix.xx = 65536;
  ftmatrix.xy = 0;
  ftmatrix.yx = 0;
  ftmatrix.yy = 65536;
  ftdelta.x = x * 64.0;
  ftdelta.y = y * 64.0;


  uch = NULL;
  ulen = 0;
  GSToUnicode(&uch, &ulen, s, strlen(s), NSUTF8StringEncoding, NSDefaultMallocZone(), 0);


  cur = imgd;

  cmap.face_id = imgd.font.face_id;
  cmap.u.encoding = ft_encoding_unicode;
  cmap.type = FTC_CMAP_BY_ENCODING;

  for (c = uch, i = 0; i < ulen; i++, c++)
    {
      FT_Face face;
      FT_Glyph gl;
      FT_OutlineGlyph og;

      glyph = FTC_CMapCache_Lookup(ftc_cmapcache, &cmap, *c);
      cur.font.face_id = imgd.font.face_id;

      if (FTC_Manager_Lookup_Size(ftc_manager, &cur.font, &face, 0))
        continue;
      if (FT_Load_Glyph(face, glyph, FT_LOAD_DEFAULT))
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

      FT_Outline_Decompose(&og->outline, &charpath_funcs, func_param);

      FT_Done_Glyph(gl);

    }

  if (ulen)
    {
      [(GSGState *)func_param DPSmoveto: ftdelta.x / 64.0 : ftdelta.y / 64.0];
    }

  free(uch);
#endif
}

- (void) appendBezierPathWithGlyphs: (NSGlyph *)glyphs
                              count: (int)count
                       toBezierPath: (NSBezierPath *)path
{
  int i;
  NSGlyph glyph;

  FT_Matrix ftmatrix;
  FT_Vector ftdelta;

  NSPoint p = [path currentPoint];

  ftmatrix.xx = 65536;
  ftmatrix.xy = 0;
  ftmatrix.yx = 0;
  ftmatrix.yy = 65536;
  ftdelta.x = p.x * 64.0;
  ftdelta.y = p.y * 64.0;

  for (i = 0; i < count; i++, glyphs++)
    {
      FT_Face face;
      FT_Glyph gl;
      FT_OutlineGlyph og;
      FT_Size size;

      glyph = *glyphs - 1;

      if (FTC_Manager_LookupSize(ftc_manager, &scaler, &size))
        continue;

      face = size->face;
      
      if (FT_Load_Glyph(face, glyph, FT_LOAD_DEFAULT))
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

  if (count)
    {
      [path moveToPoint: NSMakePoint(ftdelta.x / 64.0, ftdelta.y / 64.0)];
    }
}


static int filters[3][7]=
{
{ 0*65536/9, 1*65536/9, 2*65536/9, 3*65536/9, 2*65536/9, 1*65536/9, 0*65536/9},
{ 0*65536/9, 1*65536/9, 2*65536/9, 3*65536/9, 2*65536/9, 1*65536/9, 0*65536/9},
{ 0*65536/9, 1*65536/9, 2*65536/9, 3*65536/9, 2*65536/9, 1*65536/9, 0*65536/9}
};


+ (void) initializeBackend
{
  [GSFontEnumerator setDefaultClass: [FTFontEnumerator class]];
  [GSFontInfo setDefaultClass: [FTFontInfo class]];

  if (FT_Init_FreeType(&ft_library))
    NSLog(@"FT_Init_FreeType failed");
  if (FTC_Manager_New(ft_library, 0, 0, 4096 * 24, ft_get_face, 0, &ftc_manager))
    NSLog(@"FTC_Manager_New failed");
  if (FTC_SBitCache_New(ftc_manager, &ftc_sbitcache))
    NSLog(@"FTC_SBitCache_New failed");
  if (FTC_ImageCache_New(ftc_manager, &ftc_imagecache))
    NSLog(@"FTC_ImageCache_New failed");
  if (FTC_CMapCache_New(ftc_manager, &ftc_cmapcache))
    NSLog(@"FTC_CMapCache_New failed");

  {
    NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
    NSString *s;
    NSArray *a;
    int i;

    subpixel_text = [ud integerForKey: @"back-art-subpixel-text"];

    /* To make it easier to find an optimal (or at least good) filter,
    the filters are configurable (for now). */
    for (i = 0; i < 3; i++)
      {
        s = [ud stringForKey:
              [NSString stringWithFormat: @"back-art-subpixel-filter-%i",i]];
        if (s)
          {
            int j, c, sum, v;
            a = [s componentsSeparatedByString: @" "];
            c = [a count];
            if (!c)
              continue;
            if (!(c & 1) || c > 7)
              {
                NSLog(@"invalid number of components in filter (must be odd number, 1<=n<=7)");
                continue;
              }
            memset(filters[i], 0, sizeof(filters[0]));
            sum = 0;
            for (j = 0; j < c; j++)
              {
                v = [[a objectAtIndex: j] intValue];
                sum += v;
                filters[i][j + (7 - c) / 2] = v * 65536;
              }
            if (sum)
              {
                for (j = 0; j < 7; j++)
                  {
                    filters[i][j] /= sum;
                  }
              }
            NSLog(@"filter %i: %04x %04x %04x %04x %04x %04x %04x",
                  i,
                  filters[i][0],filters[i][1],filters[i][2],filters[i][3],
                  filters[i][4],filters[i][5],filters[i][6]);
          }
      }
  }
}

- (NSGlyph) glyphForCharacter: (unichar)ch
{
  NSGlyph g;

  g = FTC_CMapCache_Lookup(ftc_cmapcache, faceId, unicodeCmap, ch);
  if (g)
    return g + 1;
  else
    return NSNullGlyph;
}

- (NSMultibyteGlyphPacking) glyphPacking
{
  return NSFourByteGlyphPacking;
}

@end


/* TODO: this whole thing needs cleaning up */
@implementation FTFontInfo_subpixel

#if 0
- (void) drawGlyphs: (const NSGlyph *)glyphs : (int)length
        at: (int)x : (int)y
        to: (int)x0 : (int)y0 : (int)x1 : (int)y1
        : (unsigned char *)buf : (int)bpl
        color: (unsigned char)r : (unsigned char)g : (unsigned char)b
        : (unsigned char)alpha
        transform: (NSAffineTransform *)transform
        drawinfo: (struct draw_info_s *)di
{
  FTC_CMapDescRec cmap;
  unsigned int glyph;

  int use_sbit;

  FTC_SBit sbit;
  FTC_ImageTypeRec cur;

  FT_Matrix ftmatrix;
  FT_Vector ftdelta;
  NSAffineTransformStruct ts;

  BOOL subpixel = NO;

  if (!alpha)
    return;

  /* TODO: if we had guaranteed upper bounds on glyph image size we
     could do some basic clipping here */

  x1 -= x0;
  y1 -= y0;
  x -= x0;
  y -= y0;

  ts = [transform transformStruct];

/*        NSLog(@"[%@ draw using matrix: (%g %g %g %g %g %g)]",
                self,
                matrix[0], matrix[1], matrix[2],
                matrix[3], matrix[4], matrix[5]
                );*/

  cur = imgd;
  {
    float xx, xy, yx, yy;

    xx = matrix[0] * ts.m11 + matrix[1] * ts.m21;
    yx = matrix[0] * ts.m12 + matrix[1] * ts.m22;
    xy = matrix[2] * ts.m11 + matrix[3] * ts.m21;
    yy = matrix[2] * ts.m12 + matrix[3] * ts.m22;

    /* if we're drawing 'normal' text (unscaled, unrotated, reasonable
       size), we can and should use the sbit cache */
    if (fabs(xx - ((int)xx)) < 0.01 && fabs(yy - ((int)yy)) < 0.01
      && fabs(xy) < 0.01 && fabs(yx) < 0.01
      && xx < 72 && yy < 72 && xx > 0.5 && yy > 0.5)
      {
        use_sbit = 1;
        cur.font.pix_width = xx;
        cur.font.pix_height = yy;

/*        if (cur.font.pix_width < 16 && cur.font.pix_height < 16 &&
            cur.font.pix_width > 6 && cur.font.pix_height > 6)
          cur.type = ftc_image_mono;
        else*/
          cur.flags = FT_LOAD_TARGET_LCD, subpixel = YES;
//                        imgd.type|=|ftc_image_flag_unhinted; /* TODO? when? */
      }
    else
      {
        float f;
        use_sbit = 0;

        f = fabs(xx * yy - xy * yx);
        if (f > 1)
          f = sqrt(f);
        else
          f = 1.0;

        f = (int)f;

        cur.font.pix_width = cur.font.pix_height = f;
        ftmatrix.xx = xx / f * 65536.0;
        ftmatrix.xy = xy / f * 65536.0;
        ftmatrix.yx = yx / f * 65536.0;
        ftmatrix.yy = yy / f * 65536.0;
        ftdelta.x = ftdelta.y = 0;
      }
  }


/*        NSLog(@"drawString: '%s' at: %i:%i  to: %i:%i:%i:%i:%p",
                s, x, y, x0, y0, x1, y1, buf);*/

  cmap.face_id = imgd.font.face_id;
  cmap.u.encoding = ft_encoding_unicode;
  cmap.type = FTC_CMAP_BY_ENCODING;

  for (; length; length--, glyphs++)
    {
      glyph = *glyphs - 1;

      if (use_sbit)
        {
          if (FTC_SBitCache_Lookup(ftc_sbitcache, &cur, glyph, &sbit, NULL))
            continue;

          if (!sbit->buffer)
            {
              x += sbit->xadvance;
              continue;
            }

          if (sbit->format == FT_PIXEL_MODE_LCD)
            {
              int gx = 3 * x + sbit->left, gy = y - sbit->top;
              int px0 = (gx - 2 < 0? gx - 4 : gx - 2) / 3;
              int px1 = (gx + sbit->width + 2 < 0? gx + sbit->width + 2: gx + sbit->width + 4) / 3;
              int llip = gx - px0 * 3;
              int sbpl = sbit->pitch;
              int sx = sbit->width, sy = sbit->height;
              int psx = px1 - px0;
              const unsigned char *src = sbit->buffer;
              unsigned char *dst = buf;
              unsigned char scratch[psx * 3];
              int mode = subpixel_text == 2? 2 : 0;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (px1 > x1)
                px1 = x1;
              if (px0 < 0)
                {
                  px0 = -px0;
                }
              else
                {
                  px1 -= px0;
                  dst += px0 * DI.bytes_per_pixel;
                  px0 = 0;
                }

              if (px1 <= 0)
                {
                  x += sbit->xadvance;
                  continue;
                }

              for (; gy < sy; gy++, src += sbpl, dst += bpl)
                {
                  int i, j;
                  int v0, v1, v2;
                  for (i = 0, j = -llip; i < psx * 3; i+=3)
                    {
                      v0 = (0 +
                       + (j >  2 && j<sx + 3? src[j - 3] * filters[0][0] : 0)
                       + (j >  1 && j<sx + 2? src[j - 2] * filters[0][1] : 0)
                       + (j >  0 && j<sx + 1? src[j - 1] * filters[0][2] : 0)
                       + (j > -1 && j<sx    ? src[j    ] * filters[0][3] : 0)
                       + (j > -2 && j<sx - 1? src[j + 1] * filters[0][4] : 0)
                       + (j > -3 && j<sx - 2? src[j + 2] * filters[0][5] : 0)
                       + (j > -4 && j<sx - 3? src[j + 3] * filters[0][6] : 0)
                ) / 65536;
                      j++;
                      v1 = (0 +
                       + (j >  2 && j<sx + 3? src[j - 3] * filters[1][0] : 0)
                       + (j >  1 && j<sx + 2? src[j - 2] * filters[1][1] : 0)
                       + (j >  0 && j<sx + 1? src[j - 1] * filters[1][2] : 0)
                       + (j > -1 && j<sx    ? src[j    ] * filters[1][3] : 0)
                       + (j > -2 && j<sx - 1? src[j + 1] * filters[1][4] : 0)
                       + (j > -3 && j<sx - 2? src[j + 2] * filters[1][5] : 0)
                       + (j > -4 && j<sx - 3? src[j + 3] * filters[1][6] : 0)
                ) / 65536;
                      j++;
                      v2 = (0 +
                       + (j >  2 && j<sx + 3? src[j - 3] * filters[2][0] : 0)
                       + (j >  1 && j<sx + 2? src[j - 2] * filters[2][1] : 0)
                       + (j >  0 && j<sx + 1? src[j - 1] * filters[2][2] : 0)
                       + (j > -1 && j<sx    ? src[j    ] * filters[2][3] : 0)
                       + (j > -2 && j<sx - 1? src[j + 1] * filters[2][4] : 0)
                       + (j > -3 && j<sx - 2? src[j + 2] * filters[2][5] : 0)
                       + (j > -4 && j<sx - 3? src[j + 3] * filters[2][6] : 0)
                ) / 65536;
                      j++;

                      scratch[i + mode] = v0>0?v0:0;
                      scratch[i + 1] = v1>0?v1:0;
                      scratch[i + (mode ^ 2)] = v2>0?v2:0;
                    }
                  DI.render_blit_subpixel(dst,
                                          scratch + px0 * 3, r, g, b, alpha,
                                          px1);
                }
            }
          else if (sbit->format == ft_pixel_mode_mono)
            {
              int gx = x + sbit->left, gy = y - sbit->top;
              int sbpl = sbit->pitch;
              int sx = sbit->width, sy = sbit->height;
              const unsigned char *src = sbit->buffer;
              unsigned char *dst = buf;
              int src_ofs = 0;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx / 8;
                  src_ofs = (-gx) & 7;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  if (alpha >= 255)
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_MONO_OPAQUE(dst, src, src_ofs, r, g, b, sx);
                  else
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_MONO(dst, src, src_ofs, r, g, b, alpha, sx);
                }
            }
          else
            {
              NSLog(@"unhandled font bitmap format %i", sbit->format);
            }

          x += sbit->xadvance;
        }
      else
        {
          FT_Face face;
          FT_Glyph gl;
          FT_BitmapGlyph gb;

          if (FTC_Manager_Lookup_Size(ftc_manager, &cur.font, &face, 0))
            continue;

          /* TODO: for rotations of 90, 180, 270, and integer
             scales hinting might still be a good idea. */
          if (FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP))
            continue;

          if (FT_Get_Glyph(face->glyph, &gl))
            continue;

          if (FT_Glyph_Transform(gl, &ftmatrix, &ftdelta))
            {
              NSLog(@"glyph transformation failed!");
              continue;
            }
          if (FT_Glyph_To_Bitmap(&gl, ft_render_mode_normal, 0, 1))
            {
              FT_Done_Glyph(gl);
              continue;
            }
          gb = (FT_BitmapGlyph)gl;


          if (gb->bitmap.pixel_mode == ft_pixel_mode_grays)
            {
              int gx = x + gb->left, gy = y - gb->top;
              int sbpl = gb->bitmap.pitch;
              int sx = gb->bitmap.width, sy = gb->bitmap.rows;
              const unsigned char *src = gb->bitmap.buffer;
              unsigned char *dst = buf;

              if (gy < 0)
                {
                  sy += gy;
                  src -= sbpl * gy;
                  gy = 0;
                }
              else if (gy > 0)
                {
                  dst += bpl * gy;
                }

              sy += gy;
              if (sy > y1)
                sy = y1;

              if (gx < 0)
                {
                  sx += gx;
                  src -= gx;
                  gx = 0;
                }
              else if (gx > 0)
                {
                  dst += DI.bytes_per_pixel * gx;
                }

              sx += gx;
              if (sx > x1)
                sx = x1;
              sx -= gx;

              if (sx > 0)
                {
                  if (alpha >= 255)
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA_OPAQUE(dst, src, r, g, b, sx);
                  else
                    for (; gy < sy; gy++, src += sbpl, dst += bpl)
                      RENDER_BLIT_ALPHA(dst, src, r, g, b, alpha, sx);
                }
            }
/* TODO: will this case ever appear? */
/*                        else if (gb->bitmap.pixel_mode==ft_pixel_mode_mono)*/
          else
            {
              NSLog(@"unhandled font bitmap format %i", gb->bitmap.pixel_mode);
            }

          ftdelta.x += gl->advance.x >> 10;
          ftdelta.y += gl->advance.y >> 10;

          FT_Done_Glyph(gl);
        }
    }
}
#endif

@end

@interface FTFontInfo (experimental_glyph_printing_extension)
- (const char *) nameOfGlyph: (NSGlyph)g;
@end

@implementation FTFontInfo (experimental_glyph_printing_extension)
- (const char *) nameOfGlyph: (NSGlyph)g
{
static char buf[1024]; /* !!TODO!! */
  FT_Size size;
  FT_Face face;

  g--;
  if (FTC_Manager_LookupSize(ftc_manager, &scaler, &size))
    return ".notdef";
  face = size->face;

  if (FT_Get_Glyph_Name(face, g, buf, sizeof(buf)))
    return ".notdef";

  return buf;
}
@end

