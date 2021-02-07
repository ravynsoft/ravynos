/*
   Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.

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

#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSBezierPath.h>
#include <AppKit/NSColor.h>

#include "ARTGState.h"

#include "blit.h"
#include "ftfont.h"

#ifndef RDS
#include "x11/XWindowBuffer.h"
#endif

#include <libart_lgpl/libart.h>

draw_info_t ART_DI;

@implementation ARTGState

/* TODO:
   optimize all this. passing device_color_t structures around by value is
   very expensive
*/
-(void) setColor: (device_color_t *)color  state: (color_state_t)cState
{
  device_color_t c;
  unsigned char r,g,b;

  [super setColor: color  state: cState];

  if (cState & COLOR_FILL)
    {
      c = fillColor;
      gsColorToRGB(&c); /* TODO: check this */
      if (c.field[0] > 1.0) c.field[0] = 1.0;
      if (c.field[0] < 0.0) c.field[0] = 0.0;
      r = c.field[0] * 255;
      if (c.field[1] > 1.0) c.field[1] = 1.0;
      if (c.field[1] < 0.0) c.field[1] = 0.0;
      g = c.field[1] * 255;
      if (c.field[2] > 1.0) c.field[2] = 1.0;
      if (c.field[2] < 0.0) c.field[2] = 0.0;
      b = c.field[2] * 255;

      fill_color[0] = r;
      fill_color[1] = g;
      fill_color[2] = b;
      fill_color[3] = fillColor.field[AINDEX] * 255;
    }

  if (cState & COLOR_STROKE)
    {
      c = strokeColor;
      gsColorToRGB(&c); /* TODO: check this */
      if (c.field[0] > 1.0) c.field[0] = 1.0;
      if (c.field[0] < 0.0) c.field[0] = 0.0;
      r = c.field[0] * 255;
      if (c.field[1] > 1.0) c.field[1] = 1.0;
      if (c.field[1] < 0.0) c.field[1] = 0.0;
      g = c.field[1] * 255;
      if (c.field[2] > 1.0) c.field[2] = 1.0;
      if (c.field[2] < 0.0) c.field[2] = 0.0;
      b = c.field[2] * 255;

      stroke_color[0] = r;
      stroke_color[1] = g;
      stroke_color[2] = b;
      stroke_color[3] = strokeColor.field[AINDEX] * 255;
    }
}

/* specially optimized versions (since these are common and simple) */
-(void) DPSsetgray: (CGFloat)gray
{
  if (gray < 0.0) gray = 0.0;
  if (gray > 1.0) gray = 1.0;

  fillColor.space = strokeColor.space = gray_colorspace;
  fillColor.field[0] = strokeColor.field[0] = gray;
  cstate = COLOR_FILL | COLOR_STROKE;

  stroke_color[0] = stroke_color[1] = stroke_color[2] =
    fill_color[0] = fill_color[1] = fill_color[2] = gray * 255;
}

-(void) DPSsetalpha: (CGFloat)a
{
  if (a < 0.0) a = 0.0;
  if (a > 1.0) a = 1.0;
  fillColor.field[AINDEX] = strokeColor.field[AINDEX] = a;
  stroke_color[3] = fill_color[3] = a * 255;
}

- (void) DPSsetrgbcolor: (CGFloat)r : (CGFloat)g : (CGFloat)b
{
  if (r < 0.0) r = 0.0; if (r > 1.0) r = 1.0;
  if (g < 0.0) g = 0.0; if (g > 1.0) g = 1.0;
  if (b < 0.0) b = 0.0; if (b > 1.0) b = 1.0;

  fillColor.space = strokeColor.space = rgb_colorspace;
  fillColor.field[0] = strokeColor.field[0] = r;
  fillColor.field[1] = strokeColor.field[1] = g;
  fillColor.field[2] = strokeColor.field[2] = b;
  cstate = COLOR_FILL | COLOR_STROKE;

  stroke_color[0] = fill_color[0] = r * 255;
  stroke_color[1] = fill_color[1] = g * 255;
  stroke_color[2] = fill_color[2] = b * 255;
}


/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */
- (void) DPSashow: (CGFloat)ax : (CGFloat)ay : (const char*)s
{ /* adds (ax,ay) in user space to each glyph's x/y advancement */
  NSPoint p;
  int x, y;
  CGFloat numarray[2];

  if (!wi || !wi->data) return;
  if (all_clipped)
    return;

  if ([path isEmpty]) return;
  p = [path currentPoint];

  numarray[0] = ax; 
  numarray[1] = ay;

  x = p.x - offset.x;
  y = offset.y - p.y;
  [(id<FTFontInfo>)font
    drawString: s
    at: x : y
    to: clip_x0 : clip_y0 : clip_x1 : clip_y1 : CLIP_DATA : wi->bytes_per_line
      : (wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL) : wi->sx
    color: fill_color[0] : fill_color[1] : fill_color[2] : fill_color[3]
    transform: ctm
    deltas: numarray : 1 : 4
    widthChar: 0
    drawinfo: &DI];
  UPDATE_UNBUFFERED
}

- (void) DPSawidthshow: (CGFloat)cx : (CGFloat)cy : (int)c : (CGFloat)ax : (CGFloat)ay 
		      : (const char*)s
{ /* adds (ax,ay) in user space to every glyph's advancement and (cx,cy)
     to character c's x/y advancement */
  NSPoint p;
  int x, y;
  CGFloat numarray[4];

  if (!wi || !wi->data) return;
  if (all_clipped)
    return;

  if ([path isEmpty]) return;
  p = [path currentPoint];

  numarray[0] = ax; 
  numarray[1] = ay;
  numarray[2] = cx; 
  numarray[3] = cy;

  x = p.x - offset.x;
  y = offset.y - p.y;
  [(id<FTFontInfo>)font
    drawString: s
    at: x : y
    to: clip_x0 : clip_y0 : clip_x1 : clip_y1 : CLIP_DATA : wi->bytes_per_line
      : (wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL) : wi->sx
    color: fill_color[0] : fill_color[1] : fill_color[2] : fill_color[3]
    transform: ctm
    deltas: numarray : 1 : 12
    widthChar: c
    drawinfo: &DI];
  UPDATE_UNBUFFERED
}

- (void) DPScharpath: (const char*)s : (int)b
{ /* TODO: handle b? will freetype ever give us a stroke-only font? */
  NSPoint p;

  if ([path isEmpty]) return;
  p=[self currentPoint];

  [(id<FTFontInfo>)font
     outlineString: s
     at: p.x : p.y
     gstate: self];
  [self DPSclosepath];
}

- (void) DPSshow: (const char*)s
{
  NSPoint p;
  int x, y;

  if (!wi || !wi->data) return;
  if (all_clipped)
    return;

  if ([path isEmpty]) return;
  p = [path currentPoint];

  x = p.x - offset.x;
  y = offset.y - p.y;
  [(id<FTFontInfo>)font
    drawString: s
    at: x : y
    to: clip_x0 : clip_y0 : clip_x1 : clip_y1 : CLIP_DATA : wi->bytes_per_line
      : (wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL) : wi->sx
    color: fill_color[0] : fill_color[1] : fill_color[2] : fill_color[3]
    transform: ctm
    deltas: NULL : 0 : 0
    widthChar: 0
    drawinfo: &DI];
  UPDATE_UNBUFFERED
}

- (void) DPSwidthshow: (CGFloat)cx : (CGFloat)cy : (int)c : (const char*)s
{ /* adds (x,y) in user space to character c's x/y advancement */
  NSPoint p;
  int x, y;
  CGFloat numarray[2];

  if (!wi || !wi->data) return;
  if (all_clipped)
    return;

  if ([path isEmpty]) return;
  p = [path currentPoint];

  numarray[0] = cx; numarray[1] = cy;

  x = p.x - offset.x;
  y = offset.y - p.y;
  [(id<FTFontInfo>)font
    drawString: s
    at: x : y
    to: clip_x0:clip_y0:clip_x1:clip_y1 : CLIP_DATA : wi->bytes_per_line
      : (wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL) : wi->sx
    color: fill_color[0]:fill_color[1]:fill_color[2]:fill_color[3]
    transform: ctm
    deltas: numarray : 1 : 8
    widthChar: c
    drawinfo: &DI];
  UPDATE_UNBUFFERED
}

- (void) DPSxshow: (const char*)s : (const CGFloat*)numarray : (int)size
{
  NSPoint p;
  int x, y;

  if (!wi || !wi->data) return;
  if (all_clipped)
    return;

  if ([path isEmpty]) return;
  p = [path currentPoint];

  x = p.x - offset.x;
  y = offset.y - p.y;
  [(id<FTFontInfo>)font
    drawString: s
    at: x : y
    to: clip_x0 : clip_y0 : clip_x1 : clip_y1 : CLIP_DATA : wi->bytes_per_line
      : (wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL) : wi->sx
    color: fill_color[0] : fill_color[1] : fill_color[2] : fill_color[3]
    transform: ctm
    deltas: numarray : size : 1
    widthChar: 0
    drawinfo: &DI];
  UPDATE_UNBUFFERED
}

- (void) DPSxyshow: (const char*)s : (const CGFloat*)numarray : (int)size
{
  NSPoint p;
  int x, y;

  if (!wi || !wi->data) return;
  if (all_clipped)
    return;

  if ([path isEmpty]) return;
  p = [path currentPoint];

  x = p.x - offset.x;
  y = offset.y - p.y;
  [(id<FTFontInfo>)font
    drawString: s
    at: x : y
    to: clip_x0 : clip_y0 : clip_x1 : clip_y1 : CLIP_DATA : wi->bytes_per_line
      : (wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL) : wi->sx
    color: fill_color[0] : fill_color[1] : fill_color[2] : fill_color[3]
    transform: ctm
    deltas: numarray : size : 3
    widthChar: 0
    drawinfo: &DI];
  UPDATE_UNBUFFERED
}

- (void) DPSyshow: (const char*)s : (const CGFloat*)numarray : (int)size
{
  NSPoint p;
  int x, y;

  if (!wi || !wi->data) return;
  if (all_clipped)
    return;

  if (!path || [path isEmpty]) return;
  p = [path currentPoint];

  x = p.x - offset.x;
  y = offset.y - p.y;
  [(id<FTFontInfo>)font
    drawString: s
    at: x : y
    to: clip_x0 : clip_y0 : clip_x1 : clip_y1 : CLIP_DATA : wi->bytes_per_line
      : (wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL) : wi->sx
    color: fill_color[0] : fill_color[1] : fill_color[2] : fill_color[3]
    transform: ctm
    deltas: numarray : size : 2
    widthChar: 0
    drawinfo: &DI];
  UPDATE_UNBUFFERED
}


- (void) GSShowGlyphsWithAdvances: (const NSGlyph *)glyphs : (const NSSize *)advances : (size_t) length
{
  // FIXME: Currently advances is ignored
  NSPoint p;
  int x, y;

  if (!wi || !wi->data) return;
  if (all_clipped)
    return;

  if (!path || [path isEmpty]) return;
  p = [path currentPoint];

  x = p.x - offset.x;
  y = offset.y - p.y;
  if (wi->has_alpha)
    {
      [(id<FTFontInfo>)font
	drawGlyphs: glyphs : length
	at: x : y
	to: clip_x0 : clip_y0 : clip_x1 : clip_y1 : CLIP_DATA : wi->bytes_per_line
	alpha: wi->alpha + clip_x0 + clip_y0 * wi->sx : wi->sx
	color: fill_color[0] : fill_color[1] : fill_color[2] : fill_color[3]
	transform: ctm
	drawinfo: &DI];
    }
  else
    {
      [(id<FTFontInfo>)font
	drawGlyphs: glyphs : length
	at: x : y
	to: clip_x0 : clip_y0 : clip_x1 : clip_y1 : CLIP_DATA : wi->bytes_per_line
	color: fill_color[0] : fill_color[1] : fill_color[2] : fill_color[3]
	transform: ctm
	drawinfo: &DI];
    }
  UPDATE_UNBUFFERED
}


/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
- (void) DPSinitgraphics
{
  [super DPSinitgraphics];

  line_width = 1.0;
  linecapstyle = ART_PATH_STROKE_CAP_BUTT;
  linejoinstyle = ART_PATH_STROKE_JOIN_MITER;
  strokeadjust = 1;
  miter_limit = 10.0;
  [self DPSsetalpha: 1.0];

  if (dash.n_dash)
    {
      free(dash.dash);
      dash.dash = NULL;
      dash.n_dash = 0;
      do_dash = 0;
    }
}

- (void) DPScurrentlinecap: (int*)linecap
{
  switch (linecapstyle)
    {
      default:
      case ART_PATH_STROKE_CAP_BUTT:
	*linecap = NSButtLineCapStyle;
	break;
      case ART_PATH_STROKE_CAP_ROUND:
	*linecap = NSRoundLineCapStyle;
	break;
      case ART_PATH_STROKE_CAP_SQUARE:
	*linecap = NSSquareLineCapStyle;
	break;
    }
}

- (void) DPScurrentlinejoin: (int*)linejoin
{
  switch (linejoinstyle)
    {
      default:
      case ART_PATH_STROKE_JOIN_MITER:
	*linejoin = NSMiterLineJoinStyle;
	break;
      case ART_PATH_STROKE_JOIN_ROUND:
	*linejoin = NSRoundLineJoinStyle;
	break;
      case ART_PATH_STROKE_JOIN_BEVEL:
	*linejoin = NSBevelLineJoinStyle;
	break;
    }
}

- (void) DPScurrentlinewidth: (CGFloat*)width
{
  *width = line_width;
}

- (void) DPScurrentmiterlimit: (CGFloat*)limit
{
  *limit = miter_limit;
}

- (void) DPSsetdash: (const CGFloat*)pat : (NSInteger)size : (CGFloat)offs
{
  NSInteger i;

  if (dash.n_dash)
    {
      free(dash.dash);
      dash.dash = NULL;
      dash.n_dash = 0;
      do_dash = 0;
    }
  
  if (size>0)
    {
      dash.offset = offs;
      dash.n_dash = size;
      dash.dash = malloc(sizeof(double)*size);
      if (!dash.dash)
        {
	  /* Revert to no dash. Better than crashing. */
	  dash.n_dash = 0;
	  dash.offset = 0;
	  do_dash = 0;
	}
      else
        {
	  for (i = 0; i < size; i++)
	      dash.dash[i] = pat[i];
	  do_dash = 1;
	}
    }
}

- (void) DPSsetlinecap: (int)linecap
{
  switch (linecap)
    {
      default:
      case NSButtLineCapStyle:
	linecapstyle = ART_PATH_STROKE_CAP_BUTT;
	break;
      case NSRoundLineCapStyle:
	linecapstyle = ART_PATH_STROKE_CAP_ROUND;
	break;
      case NSSquareLineCapStyle:
	linecapstyle = ART_PATH_STROKE_CAP_SQUARE;
	break;
    }
}

- (void) DPSsetlinejoin: (int)linejoin
{
  switch (linejoin)
    {
      default:
      case NSMiterLineJoinStyle:
	linejoinstyle = ART_PATH_STROKE_JOIN_MITER;
	break;
      case NSRoundLineJoinStyle:
	linejoinstyle = ART_PATH_STROKE_JOIN_ROUND;
	break;
      case NSBevelLineJoinStyle:
	linejoinstyle = ART_PATH_STROKE_JOIN_BEVEL;
	break;
    }
}

- (void) DPSsetlinewidth: (CGFloat)width
{
  line_width = width;
  /* TODO? handle line_width=0 properly */
  if (line_width <= 0) line_width = 1;
}

- (void) DPSsetmiterlimit: (CGFloat)limit
{
  miter_limit=limit;
}

- (void) DPScurrentstrokeadjust: (int*)b
{
  *b = strokeadjust;
}

- (void) DPSsetstrokeadjust: (int)b
{
  strokeadjust = b;
}

@end

@implementation ARTGState (internal_stuff)

- (void) dealloc
{
  if (dash.dash)
    free(dash.dash);

  if (clip_span)
    free(clip_span);
  if (clip_index)
    free(clip_index);

  DESTROY(wi);
 
  [super dealloc];
}


-(id) deepen
{
  [super deepen];

  if (dash.dash)
    {
      double *tmp = malloc(sizeof(double) * dash.n_dash);

      if (tmp)
	{
	  memcpy(tmp, dash.dash, sizeof(double) * dash.n_dash);
	  dash.dash = tmp;
	}
      else
        {
	    dash.dash = NULL;
	    dash.n_dash = 0;
	    do_dash = 0;
	}
  }

  if (clip_span)
    {
      unsigned int *n;

      n = malloc(sizeof(unsigned int) * clip_num_span);
      if (n)
	{
	  memcpy(n, clip_span, sizeof(unsigned int) * clip_num_span);
	  clip_span = n;
	  n = malloc(sizeof(unsigned int *) * (clip_sy+1));
	  if (n)
	    {
	      memcpy(n, clip_index, sizeof(unsigned int *) * (clip_sy + 1));
	      clip_index = n;
	    }
	  else
	    {
	      free(clip_span);
	      clip_span=clip_index = NULL;
	      clip_num_span = 0;
	    }
	}
      else
	{
	  clip_span = clip_index = NULL;
	  clip_num_span = 0;
	}
  }

  wi = RETAIN(wi);

  return self;
}

-(void) GSSetDevice: (gswindow_device_t *)window : (int)x : (int)y
{
  struct XWindowBuffer_depth_info_s di;

  [self setOffset: NSMakePoint(x, y)];

#ifndef RDS
  di.drawing_depth = DI.drawing_depth;
#endif
  di.bytes_per_pixel = DI.bytes_per_pixel;
  di.inline_alpha = DI.inline_alpha;
  di.inline_alpha_ofs = DI.inline_alpha_ofs;
  di.byte_order = ImageByteOrder(window->display);

  ASSIGN(wi, [XWindowBuffer windowBufferForWindow: window depthInfo: &di]);
}

-(void) GSCurrentDevice: (void **)device : (int *)x : (int *)y
{
  NSPoint theOffset = [self offset];

  if (x)
      *x = theOffset.x;
  if (y)
      *y = theOffset.y;

  if (device)
    {
      if (wi)
        *device = wi->window;
      else
        *device = NULL;
    }
}

@end

@implementation ARTGState (PatternColor)
typedef struct _SavedClip {
  int clip_x0,clip_y0,clip_x1,clip_y1;
  BOOL all_clipped;
  int clip_sx,clip_sy;
  unsigned int *clip_span;
  unsigned int *clip_index;
  int clip_num_span;
} SavedClip;

- (void *) saveClip
{
  SavedClip *savedClip = malloc(sizeof(SavedClip));
  int i;

  savedClip->clip_x0 = clip_x0;
  savedClip->clip_y0 = clip_y0;
  savedClip->clip_x1 = clip_x1;
  savedClip->clip_y1 = clip_y1;
  savedClip->all_clipped = all_clipped;
  savedClip->clip_sx = clip_sx;
  savedClip->clip_sy = clip_sy;
  if (clip_num_span == 0)
    {
      savedClip->clip_span = NULL;
      savedClip->clip_index = NULL;
    }
  else
    {
      savedClip->clip_span = malloc(sizeof(int) * clip_num_span);
      savedClip->clip_index = malloc(sizeof(int) * clip_num_span);
      for (i = 0; i < clip_num_span; i++)
        {
          savedClip->clip_span[i] = clip_span[i];
        }
      for (i = 0; i < clip_num_span; i++)
        {
          savedClip->clip_index[i] = clip_index[i];
        }
    }
  savedClip->clip_num_span = clip_num_span;

  return savedClip;
}

- (void) restoreClip: (void *)saved
{
  SavedClip *savedClip = (SavedClip *)saved;

  clip_x0 = savedClip->clip_x0;
  clip_y0 = savedClip->clip_y0;
  clip_x1 = savedClip->clip_x1;
  clip_y1 = savedClip->clip_y1;
  all_clipped = savedClip->all_clipped;
  clip_sx = savedClip->clip_sx;
  clip_sy = savedClip->clip_sy;
  if (clip_span)
    {
      free(clip_span);
      free(clip_index);
    }
  clip_span = savedClip->clip_span;
  clip_index = savedClip->clip_index;
  clip_num_span = savedClip->clip_num_span;
  free(savedClip);
}

@end
