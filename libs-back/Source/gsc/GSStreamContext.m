/* -*-objc-*-
   GSStreamContext - Drawing context to a stream.

   Copyright (C) 1995-2016 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Nov 1995
   
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
#include "gsc/GSContext.h"
#include "gsc/GSStreamContext.h"
#include "gsc/GSStreamGState.h"
#include <GNUstepGUI/GSFontInfo.h>
#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSBezierPath.h>
#include <AppKit/NSView.h>
#include <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSFontDescriptor.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSData.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSString.h>
#include <Foundation/NSUserDefaults.h>
#include <Foundation/NSValue.h>
#include <string.h>

@interface GSFontInfo (experimental_glyph_printing_extension)
// This method is currently only present in the libart backend
-(const char *) nameOfGlyph: (NSGlyph)g;
@end

/* Print a floating point number regardless of localization */
static void
fpfloat(FILE *stream, float f)
{
  char buffer[80], *p;
  sprintf(buffer, "%g ", f);
  p = buffer;
  while (*p)
    {
      if (*p == ',')
	*p = '.';
      p++;
    }
  fprintf(stream, "%s", buffer);
}

@interface GSStreamContext (Private)

- (void) output: (const char*)s length: (size_t)length;
- (void) output: (const char*)s;

@end

@implementation GSStreamContext 

+ (Class) GStateClass
{
  return [GSStreamGState class];
}

+ (BOOL) handlesPS
{
  return YES;
}

- (void) dealloc
{
  if (gstream)
    fclose(gstream);
  [super dealloc];
}

- initWithContextInfo: (NSDictionary *)info
{
  self = [super initWithContextInfo: info];
  if (!self)
    return nil;

  if (info && [info objectForKey: @"NSOutputFile"])
    {
      NSString *path = [info objectForKey: @"NSOutputFile"];
      NSDebugLLog(@"GSContext", @"Printing to %@", path);
#if	defined(__MINGW32__)
      gstream = _wfopen([path fileSystemRepresentation], L"wb");
#else
      gstream = fopen([path fileSystemRepresentation], "w");
#endif
      if (!gstream)
        {
          NSDebugLLog(@"GSContext", @"%@: Could not open printer file %@",
                      DPSinvalidfileaccess, path);
          return nil;
        }
    }
  else
    {
      NSDebugLLog(@"GSContext", @"%@: No stream file specified",
                  DPSconfigurationerror);
      DESTROY(self);
      return nil;
    }

  return self;
}

- (BOOL)isDrawingToScreen
{
  return NO;
}

@end

@implementation GSStreamContext (Ops)
/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
- (void) DPSsetalpha: (CGFloat)a
{
  [super DPSsetalpha: a];
  /* This needs to be defined base on the the language level, etc. in
     the Prolog section. */
  fpfloat(gstream, a);
  fprintf(gstream, "GSsetalpha\n");
}

- (void) DPSsetcmykcolor: (CGFloat)c : (CGFloat)m : (CGFloat)y : (CGFloat)k
{
  [super DPSsetcmykcolor: c : m : y : k];
  fpfloat(gstream, c);
  fpfloat(gstream, m);
  fpfloat(gstream, y);
  fpfloat(gstream, k);
  fprintf(gstream, "setcmykcolor\n");
}

- (void) DPSsetgray: (CGFloat)gray
{
  [super DPSsetgray: gray];
  fpfloat(gstream, gray);
  fprintf(gstream, "setgray\n");
}

- (void) DPSsethsbcolor: (CGFloat)h : (CGFloat)s : (CGFloat)b
{
  [super DPSsethsbcolor: h : s : b];
  fpfloat(gstream, h);
  fpfloat(gstream, s);
  fpfloat(gstream, b);
  fprintf(gstream, "sethsbcolor\n");
}

- (void) DPSsetrgbcolor: (CGFloat)r : (CGFloat)g : (CGFloat)b
{
  [super DPSsetrgbcolor: r : g : b];
  fpfloat(gstream, r);
  fpfloat(gstream, g);
  fpfloat(gstream, b);
  fprintf(gstream, "setrgbcolor\n");
}

- (void) GSSetFillColor: (const CGFloat *)values
{
  [self notImplemented: _cmd];
}

- (void) GSSetStrokeColor: (const CGFloat *)values
{
  [self notImplemented: _cmd];
}

- (void) GSSetPatterColor: (NSImage*)image 
{
  [self notImplemented: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */
- (void) DPSashow: (CGFloat)x : (CGFloat)y : (const char*)s
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fprintf(gstream, "(");
  [self output:s];
  fprintf(gstream, ") ashow\n");
}

- (void) DPSawidthshow: (CGFloat)cx : (CGFloat)cy : (int)c : (CGFloat)ax : (CGFloat)ay : (const char*)s
{
  fpfloat(gstream, cx);
  fpfloat(gstream, cy);
  fprintf(gstream, "%d ", c);
  fpfloat(gstream, ax);
  fpfloat(gstream, ay);
  fprintf(gstream, "(");
  [self output:s];
  fprintf(gstream, ") awidthshow\n");
}

- (void) DPScharpath: (const char*)s : (int)b
{
  fprintf(gstream, "(");
  [self output:s];
  fprintf(gstream, ") %d charpath\n", b);
}

- (void) DPSshow: (const char*)s
{
  fprintf(gstream, "(");
  [self output:s];
  fprintf(gstream, ") show\n");
}

- (void) DPSwidthshow: (CGFloat)x : (CGFloat)y : (int)c : (const char*)s
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fprintf(gstream, "%d (", c);
  [self output:s];
  fprintf(gstream, ") widthshow\n");
}

- (void) DPSxshow: (const char*)s : (const CGFloat*)numarray : (int)size
{
  [self notImplemented: _cmd];
}

- (void) DPSxyshow: (const char*)s : (const CGFloat*)numarray : (int)size
{
  [self notImplemented: _cmd];
}

- (void) DPSyshow: (const char*)s : (const CGFloat*)numarray : (int)size
{
  [self notImplemented: _cmd];
}


- (void) GSSetCharacterSpacing: (CGFloat)extra
{
  [self notImplemented: _cmd];
}

- (void) GSSetFont: (void *)fontref
{
  const CGFloat *m = [(GSFontInfo *)fontref matrix];
  NSString *postscriptName;

  postscriptName = [[(GSFontInfo *)fontref fontDescriptor] postscriptName];
  if (nil == postscriptName)
    {
      postscriptName = [(GSFontInfo *)fontref fontName];
    }
  fprintf(gstream, "/%s findfont ", [postscriptName cString]);
  fprintf(gstream, "[");
  fpfloat(gstream, m[0]);
  fpfloat(gstream, m[1]);
  fpfloat(gstream, m[2]);
  fpfloat(gstream, m[3]);
  fpfloat(gstream, m[4]);
  fpfloat(gstream, m[5]);
  fprintf(gstream, "] ");
  fprintf(gstream, " makefont setfont\n");
  [super GSSetFont: fontref];
}

- (void) GSSetFontSize: (CGFloat)size
{
  [self notImplemented: _cmd];
}

- (void) GSShowText: (const char *)string : (size_t)length
{
  fprintf(gstream, "(");
  [self output:string length: length];
  fprintf(gstream, ") show\n");
}

- (void) GSShowGlyphs: (const NSGlyph *)glyphs : (size_t)length
{
  GSFontInfo *font = gstate->font;
  if ([font respondsToSelector: @selector(nameOfGlyph:)])
    {
      unsigned int i;
      
      for (i = 0; i < length; i++)
	{
	  fprintf(gstream, "/%s glyphshow\n", [font nameOfGlyph: glyphs[i]]);
	}
    }
  else
    {
      /* If backend doesn't handle nameOfGlyph, assume the glyphs are
	 just mapped to characters. This is the case for the xlib backend
	 (at least for now). */
      char string[length + 1];
      unsigned int i;
      
      for (i = 0; i < length; i++)
	{
	  string[i] = glyphs[i];
	} 
      string[length] = 0;
      [self DPSshow: string];
    }
}

- (void) GSShowGlyphsWithAdvances: (const NSGlyph *)glyphs
                                 : (const NSSize *)advances
                                 : (size_t)length
{
  // FIXME: Currently advances is ignored
  [self GSShowGlyphs: glyphs : length];
}


/* ----------------------------------------------------------------------- */
/* Gstate Handling */
/* ----------------------------------------------------------------------- */
- (void) DPSgrestore
{
  [super DPSgrestore];
  fprintf(gstream, "grestore\n");
}

- (void) DPSgsave
{
  [super DPSgsave];
  fprintf(gstream, "gsave\n");
}

- (void) DPSgstate
{
  [super DPSgsave];
  fprintf(gstream, "gstaten");
}

- (void) DPSinitgraphics
{
  [super DPSinitgraphics];
  fprintf(gstream, "initgraphics\n");
}

- (void) DPSsetgstate: (int)gst
{
  [self notImplemented: _cmd];
}

- (int) GSDefineGState
{
  [self notImplemented: _cmd];
  return 0;
}

- (void) GSUndefineGState: (int)gst
{
  [self notImplemented: _cmd];
}

- (void) GSReplaceGState: (int)gst
{
  [self notImplemented: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
- (void) DPSsetdash: (const CGFloat*)pat : (NSInteger)size : (CGFloat)offset
{
  int i;
  fprintf(gstream, "[");
  for (i = 0; i < size; i++)
    fpfloat(gstream, pat[i]);
  fprintf(gstream, "] ");
  fpfloat(gstream, offset);
  fprintf(gstream, "setdash\n");
}

- (void) DPSsetflat: (CGFloat)flatness
{
  [super DPSsetflat: flatness];
  fpfloat(gstream, flatness);
  fprintf(gstream, "setflat\n");
}

- (void) DPSsethalftonephase: (CGFloat)x : (CGFloat)y
{
  [super DPSsethalftonephase: x : y];
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fprintf(gstream, "sethalftonephase\n");
}

- (void) DPSsetlinecap: (int)linecap
{
  [super DPSsetlinecap: linecap];
  fprintf(gstream, "%d setlinecap\n", linecap);
}

- (void) DPSsetlinejoin: (int)linejoin
{
  [super DPSsetlinejoin: linejoin];
  fprintf(gstream, "%d setlinejoin\n", linejoin);
}

- (void) DPSsetlinewidth: (CGFloat)width
{
  [super DPSsetlinewidth: width];
  fpfloat(gstream, width);
  fprintf(gstream, "setlinewidth\n");
}

- (void) DPSsetmiterlimit: (CGFloat)limit
{
  [super DPSsetmiterlimit: limit];
  fpfloat(gstream, limit);
  fprintf(gstream, "setmiterlimit\n");
}

- (void) DPSsetstrokeadjust: (int)b
{
  [super DPSsetstrokeadjust: b];
  fprintf(gstream, "%s setstrokeadjust\n", b? "true" : "false");
}


/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
- (void) DPSconcat: (const CGFloat*)m
{
  [super DPSconcat: m];

  if ((m[0] == 1.0) && (m[1] == 0.0) &&
      (m[2] == 0.0) && (m[3] == 1.0))
    {
      if ((m[4] != 0.0) || (m[5] != 0.0))
	{
	  fpfloat(gstream, m[4]);
	  fpfloat(gstream, m[5]);
	  fprintf(gstream, "translate\n");
	}
    }
  else 
    {
      fprintf(gstream, "[");
      fpfloat(gstream, m[0]);
      fpfloat(gstream, m[1]);
      fpfloat(gstream, m[2]);
      fpfloat(gstream, m[3]);
      fpfloat(gstream, m[4]);
      fpfloat(gstream, m[5]);
      fprintf(gstream, "] concat\n");
    }
}

- (void) DPSinitmatrix
{
  [super DPSinitmatrix];
  fprintf(gstream, "initmatrix\n");
}

- (void) DPSrotate: (CGFloat)angle
{
  [super DPSrotate: angle];
  fpfloat(gstream, angle);
  fprintf(gstream, "rotate\n");
}

- (void) DPSscale: (CGFloat)x : (CGFloat)y
{
  [super DPSscale: x : y];
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fprintf(gstream, "scale\n");
}

- (void) DPStranslate: (CGFloat)x : (CGFloat)y
{
  [super DPStranslate: x : y];
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fprintf(gstream, "translate\n");
}

- (void) GSSetCTM: (NSAffineTransform *)ctm
{
  NSAffineTransformStruct matrix = [ctm transformStruct];

  fprintf(gstream, "[");
  fpfloat(gstream, matrix.m11);
  fpfloat(gstream, matrix.m12);
  fpfloat(gstream, matrix.m21);
  fpfloat(gstream, matrix.m22);
  fpfloat(gstream, matrix.tX);
  fpfloat(gstream, matrix.tY);
  fprintf(gstream, "] setmatrix\n");
}

- (void) GSConcatCTM: (NSAffineTransform *)ctm
{
  NSAffineTransformStruct matrix = [ctm transformStruct];

  fprintf(gstream, "[");
  fpfloat(gstream, matrix.m11);
  fpfloat(gstream, matrix.m12);
  fpfloat(gstream, matrix.m21);
  fpfloat(gstream, matrix.m22);
  fpfloat(gstream, matrix.tX);
  fpfloat(gstream, matrix.tY);
  fprintf(gstream, "] concat\n");
}


/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
- (void) DPSarc: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 : (CGFloat)angle2
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fpfloat(gstream, r);
  fpfloat(gstream, angle1);
  fpfloat(gstream, angle2);
  fprintf(gstream, "arc\n");
}

- (void) DPSarcn: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 : (CGFloat)angle2
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fpfloat(gstream, r);
  fpfloat(gstream, angle1);
  fpfloat(gstream, angle2);
  fprintf(gstream, "arcn\n");
}

- (void) DPSarct: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 : (CGFloat)r
{
  fpfloat(gstream, x1);
  fpfloat(gstream, y1);
  fpfloat(gstream, x2);
  fpfloat(gstream, y2);
  fpfloat(gstream, r);
  fprintf(gstream, "arct\n");
}

- (void) DPSclip
{
  fprintf(gstream, "clip\n");
}

- (void) DPSclosepath
{
  fprintf(gstream, "closepath\n");
}

- (void)DPScurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 
                  : (CGFloat)x3 : (CGFloat)y3
{
  fpfloat(gstream, x1);
  fpfloat(gstream, y1);
  fpfloat(gstream, x2);
  fpfloat(gstream, y2);
  fpfloat(gstream, x3);
  fpfloat(gstream, y3);
  fprintf(gstream, "curveto\n");
}

- (void) DPSeoclip
{
  fprintf(gstream, "eoclip\n");
}

- (void) DPSeofill
{
  fprintf(gstream, "eofill\n");
}

- (void) DPSfill
{
  fprintf(gstream, "fill\n");
}

- (void) DPSflattenpath
{
  fprintf(gstream, "flattenpath\n");
}

- (void) DPSinitclip
{
  fprintf(gstream, "initclip\n");
}

- (void) DPSlineto: (CGFloat)x : (CGFloat)y
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fprintf(gstream, "lineto\n");
}

- (void) DPSmoveto: (CGFloat)x : (CGFloat)y
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fprintf(gstream, "moveto\n");
}

- (void) DPSnewpath
{
  fprintf(gstream, "newpath\n");
}

- (void) DPSpathbbox: (CGFloat*)llx : (CGFloat*)lly : (CGFloat*)urx : (CGFloat*)ury
{
}

- (void) DPSrcurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 
                    : (CGFloat)x3 : (CGFloat)y3
{
  fpfloat(gstream, x1);
  fpfloat(gstream, y1);
  fpfloat(gstream, x2);
  fpfloat(gstream, y2);
  fpfloat(gstream, x3);
  fpfloat(gstream, y3);
  fprintf(gstream, "rcurveto\n");
}

- (void) DPSrectclip: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fpfloat(gstream, w);
  fpfloat(gstream, h);
  fprintf(gstream, "rectclip\n");
}

- (void) DPSrectfill: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fpfloat(gstream, w);
  fpfloat(gstream, h);
  fprintf(gstream, "rectfill\n");
}

- (void) DPSrectstroke: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fpfloat(gstream, w);
  fpfloat(gstream, h);
  fprintf(gstream, "rectstroke\n");
}

- (void) DPSreversepath
{
  fprintf(gstream, "reversepath\n");
}

- (void) DPSrlineto: (CGFloat)x : (CGFloat)y
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fprintf(gstream, "rlineto\n");
}

- (void) DPSrmoveto: (CGFloat)x : (CGFloat)y
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fprintf(gstream, "rmoveto\n");
}

- (void) DPSstroke
{
  fprintf(gstream, "stroke\n");
}

- (void) GSSendBezierPath: (NSBezierPath *)path
{
  NSBezierPathElement type;
  NSPoint pts[3];
  NSInteger i, count = 10;
  CGFloat pattern[10];
  CGFloat phase = 0.0;

  [self DPSnewpath];
  [self DPSsetlinewidth: [path lineWidth]];
  [self DPSsetlinejoin: [path lineJoinStyle]];
  [self DPSsetlinecap: [path lineCapStyle]];
  [self DPSsetmiterlimit: [path miterLimit]];
  [self DPSsetflat: [path flatness]];

  [path getLineDash: pattern count: &count phase: &phase];
  // Always sent the dash pattern. When NULL this will reset to a solid line.
  [self DPSsetdash: pattern : count : phase];

  count = [path elementCount];
  for (i = 0; i < count; i++)
    {
      type = [path elementAtIndex: i associatedPoints: pts];
      switch (type)
        {
	case NSMoveToBezierPathElement:
	  [self DPSmoveto: pts[0].x : pts[0].y];
	  break;
	case NSLineToBezierPathElement:
	  [self DPSlineto: pts[0].x : pts[0].y];
	  break;
	case NSCurveToBezierPathElement:
	  [self DPScurveto: pts[0].x : pts[0].y
	   : pts[1].x : pts[1].y : pts[2].x : pts[2].y];
	  break;
	case NSClosePathBezierPathElement:
	  [self DPSclosepath];
	  break;
	default:
	  break;
	}
    }
}

- (void) GSRectClipList: (const NSRect *)rects : (int)count
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

- (void) GSRectFillList: (const NSRect *)rects : (int)count
{
  int i;
  for (i = 0; i < count; i++)
    [self DPSrectfill: NSMinX(rects[i]) : NSMinY(rects[i])
	  : NSWidth(rects[i]) : NSHeight(rects[i])];
}

/* ----------------------------------------------------------------------- */
/* Window system ops */
/* ----------------------------------------------------------------------- */
- (void) DPScurrentgcdrawable: (void**)gc : (void**)draw : (int*)x : (int*)y
{
  NSLog(@"DPSinvalidcontext: getting gcdrawable from stream context");
}

- (void) DPScurrentoffset: (int*)x : (int*)y
{
  NSLog(@"DPSinvalidcontext: getting drawable offset from stream context");
}

- (void) DPSsetgcdrawable: (void*)gc : (void*)draw : (int)x : (int)y
{
  NSLog(@"DPSinvalidcontext: setting gcdrawable from stream context");
}

- (void) DPSsetoffset: (short int)x : (short int)y
{
  NSLog(@"DPSinvalidcontext: setting drawable offset from stream context");
}


/*-------------------------------------------------------------------------*/
/* Graphics Extensions Ops */
/*-------------------------------------------------------------------------*/
- (void) DPScomposite: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h 
                     : (NSInteger)gstateNum : (CGFloat)dx : (CGFloat)dy : (NSCompositingOperation)op
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fpfloat(gstream, w);
  fpfloat(gstream, h);
  fprintf(gstream, "%d ", (int)gstateNum);
  fpfloat(gstream, dx);
  fpfloat(gstream, dy);
  fprintf(gstream, "%d composite\n", (int)op);
}

- (void) DPScompositerect: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h : (NSCompositingOperation)op
{
  fpfloat(gstream, x);
  fpfloat(gstream, y);
  fpfloat(gstream, w);
  fpfloat(gstream, h);
  fprintf(gstream, "%d compositerect\n", (int)op);
}

- (void) DPSdissolve: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h 
                    : (NSInteger)gstateNum : (CGFloat)dx : (CGFloat)dy : (CGFloat)delta
{
  NSLog(@"DPSinvalidcontext: dissolve in a stream context");
}

- (void) GScomposite: (NSInteger)gstateNum
	     toPoint: (NSPoint)aPoint
	    fromRect: (NSRect)srcRect
	   operation: (NSCompositingOperation)op
	    fraction: (CGFloat)delta
{
 [self DPScomposite: NSMinX(srcRect) : NSMinY(srcRect) : NSWidth(srcRect) : NSHeight(srcRect) 
                   : gstateNum : aPoint.x : aPoint.y : op];
}

- (void) GSDrawImage: (NSRect)rect : (void *)imageref
{
  id image = (id)imageref;
  unsigned char *imagePlanes[5];

  if([image isKindOfClass: [NSBitmapImageRep class]])
    {
      fprintf(gstream,"%%%% BeginImage\n");
      [image getBitmapDataPlanes: imagePlanes];
      [self NSDrawBitmap: rect
            : [image pixelsWide]
            : [image pixelsHigh]
            : [image bitsPerSample]
            : [image samplesPerPixel]
            : [image bitsPerPixel]
            : [image bytesPerRow]
            : [image isPlanar]
            : [image hasAlpha]
            : [image colorSpaceName]
            : (const unsigned char **)imagePlanes];
      fprintf(gstream,"%%%% EndImage\n");
    }
}


/* ----------------------------------------------------------------------- */
/* Client functions */
/* ----------------------------------------------------------------------- */
- (void) DPSPrintf: (const char *)fmt  : (va_list)args
{
  vfprintf(gstream, fmt, args);
}

- (void) DPSWriteData: (const char *)buf : (unsigned int)count
{
  /* Not sure here. Should we translate to ASCII if it's not
     already? */
}

@end


static void
writeHex(FILE *gstream, const unsigned char *data, int count)
{
static const char *hexdigits = "0123456789abcdef";
  int i;
  for (i = 0; i < count; i++)
    {
      fputc(hexdigits[(int)(data[i] / 16)], gstream);
      fputc(hexdigits[(int)(data[i] % 16)], gstream);
      if (i && i % 40 == 0)
	fprintf(gstream, "\n");
    }
}

@implementation GSStreamContext (Graphics)

- (void) NSDrawBitmap: (NSRect)rect : (NSInteger)pixelsWide : (NSInteger)pixelsHigh
		     : (NSInteger)bitsPerSample : (NSInteger)samplesPerPixel 
		     : (NSInteger)bitsPerPixel : (NSInteger)bytesPerRow : (BOOL)isPlanar
		     : (BOOL)hasAlpha : (NSString *)colorSpaceName
		     : (const unsigned char *const [5])data
{
  NSInteger bytes, spp;
  CGFloat y;
  BOOL flipped = NO;

  /* In a flipped view, we don't want to flip the image again, which would
     make it come out upsidedown. FIXME: This can't be right, can it? */
  if ([[NSView focusView] isFlipped])
    flipped = YES;

  /* Save scaling */
  fprintf(gstream, "matrix\ncurrentmatrix\n");
  y = NSMinY(rect);
  if (flipped)
    y += NSHeight(rect);
  fpfloat(gstream, NSMinX(rect));
  fpfloat(gstream, y);
  fprintf(gstream, "translate ");
  fpfloat(gstream, NSWidth(rect));
  fpfloat(gstream, NSHeight(rect));
  fprintf(gstream, "scale\n");

  if (bitsPerSample == 0)
    bitsPerSample = 8;
  bytes = 
    (bitsPerSample * pixelsWide * pixelsHigh + 7) / 8;
  if (bytes * samplesPerPixel != bytesPerRow * pixelsHigh) 
    {
      NSLog(@"Image Rendering Error: Dodgy bytesPerRow value %d", (int)bytesPerRow);
      NSLog(@"   pixelsHigh=%d, bytes=%d, samplesPerPixel=%d",
	    (int)bytesPerRow, (int)pixelsHigh, (int)bytes);
      return;
    }
  if (hasAlpha)
    spp = samplesPerPixel - 1;
  else
    spp = samplesPerPixel;

  if (samplesPerPixel > 1) 
    {
      if (isPlanar || hasAlpha) 
	{
	  if (bitsPerSample != 8) 
	    {
	      NSLog(@"Image format conversion not supported for bps!=8");
	      return;
	    }
	}
      fprintf(gstream, "%d %d %d [%d 0 0 %d 0 %d]\n",
	      (int)pixelsWide, (int)pixelsHigh, (int)bitsPerSample, (int)pixelsWide,
	      (flipped) ? (int)pixelsHigh : (int)-pixelsHigh, (int)pixelsHigh);
      fprintf(gstream, "{currentfile %d string readhexstring pop}\n",
	      (int)(pixelsWide * spp));
      fprintf(gstream, "false %d colorimage\n", (int)spp);
    } 
  else
    {
      fprintf(gstream, "%d %d %d [%d 0 0 %d 0 %d]\n",
	      (int)pixelsWide, (int)pixelsHigh, (int)bitsPerSample, (int)pixelsWide,
	      (flipped) ? (int)pixelsHigh : (int)-pixelsHigh, (int)pixelsHigh);
      fprintf(gstream, "currentfile image\n");
    }
  
  // The context is now waiting for data on its standard input
  if (isPlanar || hasAlpha) 
    {
      // We need to do a format conversion.
      // We do this on the fly, sending data to the context as soon as
      // it is computed.
      int i, j;
      // Preset this variable to keep compiler happy.
      int alpha = 0;
      unsigned char val;

      for (j = 0; j < bytes; j++)
	{
	  if (hasAlpha)
	    {
	      if (isPlanar)
		alpha = data[spp][j];
	      else
		alpha = data[0][spp + j * samplesPerPixel];
	    }
	  for (i = 0; i < spp; i++) 
	    {
	      if (isPlanar)
		val = data[i][j];
	      else
		val = data[0][i + j * samplesPerPixel];
	      if (hasAlpha)
		val = 255 - ((255 - val) * (long)alpha) / 255;
	      writeHex(gstream, &val, 1);
	    }
	  if (j && j % 40 == 0)
	    fprintf(gstream, "\n");
	}
      fprintf(gstream, "\n");
    } 
  else 
    {
      // The data is already in the format the context expects it in
      writeHex(gstream, data[0], bytes * samplesPerPixel);
    }

  /* Restore original scaling */
  fprintf(gstream, "setmatrix\n");
}

@end

@implementation GSStreamContext (Private)

  - (void) output: (const char*)s length: (size_t)length
{
  unsigned int i;

  for (i = 0; i < length; i++)
    {
      switch (s[i])
      {
	case '(':
	    fputs("\\(", gstream);
	    break;
	case ')':
	    fputs("\\)", gstream);
	    break;
	default:
	    fputc(s[i], gstream);
	    break;
      }
    }
}

- (void) output: (const char*)s
{
  [self output: s length: strlen(s)];
}

@end
