/* GSGStateOPS - Ops for GSGState

   Copyright (C) 1998 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Nov 1998
   
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

#ifndef _GSGStateOps_h_INCLUDE
#define _GSGStateOps_h_INCLUDE

@class NSGradient;

@interface GSGState (Ops)
/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
- (void) DPScurrentalpha: (CGFloat*)a;
- (void) DPScurrentcmykcolor: (CGFloat*)c : (CGFloat*)m : (CGFloat*)y : (CGFloat*)k;
- (void) DPScurrentgray: (CGFloat*)gray;
- (void) DPScurrenthsbcolor: (CGFloat*)h : (CGFloat*)s : (CGFloat*)b;
- (void) DPScurrentrgbcolor: (CGFloat*)r : (CGFloat*)g : (CGFloat*)b;
- (void) DPSsetalpha: (CGFloat)a;
- (void) DPSsetcmykcolor: (CGFloat)c : (CGFloat)m : (CGFloat)y : (CGFloat)k;
- (void) DPSsetgray: (CGFloat)gray;
- (void) DPSsethsbcolor: (CGFloat)h : (CGFloat)s : (CGFloat)b;
- (void) DPSsetrgbcolor: (CGFloat)r : (CGFloat)g : (CGFloat)b;

- (void) GSSetFillColorspace: (void *)spaceref;
- (void) GSSetStrokeColorspace: (void *)spaceref;
- (void) GSSetFillColor: (const CGFloat *)values;
- (void) GSSetStrokeColor: (const CGFloat *)values;

/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */
- (void) DPSashow: (CGFloat)x : (CGFloat)y : (const char*)s;
- (void) DPSawidthshow: (CGFloat)cx : (CGFloat)cy : (int)c
                      : (CGFloat)ax : (CGFloat)ay : (const char*)s;
- (void) DPScharpath: (const char*)s : (int)b;
- (void) appendBezierPathWithPackedGlyphs: (const char *)packedGlyphs
                                     path: (NSBezierPath*)aPath;
- (void) DPSshow: (const char*)s;
- (void) DPSwidthshow: (CGFloat)x : (CGFloat)y : (int)c : (const char*)s;
- (void) DPSxshow: (const char*)s : (const CGFloat*)numarray : (int)size;
- (void) DPSxyshow: (const char*)s : (const CGFloat*)numarray : (int)size;
- (void) DPSyshow: (const char*)s : (const CGFloat*)numarray : (int)size;

- (void) GSSetCharacterSpacing: (CGFloat)extra;
- (void) GSSetFont: (GSFontInfo *)fontref;
- (void) GSSetFontSize: (CGFloat)size;
- (NSAffineTransform *) GSGetTextCTM;
- (NSPoint) GSGetTextPosition;
- (void) GSSetTextCTM: (NSAffineTransform *)ctm;
- (void) GSSetTextDrawingMode: (GSTextDrawingMode)mode;
- (void) GSSetTextPosition: (NSPoint)loc;
- (void) GSShowText: (const char *)string : (size_t) length;
- (void) GSShowGlyphs: (const NSGlyph *)glyphs : (size_t) length;
- (void) GSShowGlyphsWithAdvances: (const NSGlyph *)glyphs : (const NSSize *)advances : (size_t) length;

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
- (void) DPSinitgraphics;

- (void) DPScurrentflat: (CGFloat*)flatness;
- (void) DPScurrentlinecap: (int*)linecap;
- (void) DPScurrentlinejoin: (int*)linejoin;
- (void) DPScurrentlinewidth: (CGFloat*)width;
- (void) DPScurrentmiterlimit: (CGFloat*)limit;
- (void) DPScurrentpoint: (CGFloat*)x : (CGFloat*)y;
- (void) DPScurrentstrokeadjust: (int*)b;
- (void) DPSsetdash: (const CGFloat*)pat : (NSInteger)size : (CGFloat)offset;
- (void) DPSsetflat: (CGFloat)flatness;
- (void) DPSsetlinecap: (int)linecap;
- (void) DPSsetlinejoin: (int)linejoin;
- (void) DPSsetlinewidth: (CGFloat)width;
- (void) DPSsetmiterlimit: (CGFloat)limit;
- (void) DPSsetstrokeadjust: (int)b;

/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
- (void) DPSconcat: (const CGFloat*)m;
- (void) DPSinitmatrix;
- (void) DPSrotate: (CGFloat)angle;
- (void) DPSscale: (CGFloat)x : (CGFloat)y;
- (void) DPStranslate: (CGFloat)x : (CGFloat)y;

- (NSAffineTransform *) GSCurrentCTM;
- (void) GSSetCTM: (NSAffineTransform *)ctm;
- (void) GSConcatCTM: (NSAffineTransform *)ctm;

/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
- (NSPoint) currentPoint;

- (void) DPSarc: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 
	       : (CGFloat)angle2;
- (void) DPSarcn: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 
		: (CGFloat)angle2;
- (void) DPSarct: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 : (CGFloat)r;
- (void) DPSclip;
- (void) DPSclosepath;
- (void) DPScurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 
		   : (CGFloat)x3 : (CGFloat)y3;
- (void) DPSeoclip;
- (void) DPSeofill;
- (void) DPSfill;
- (void) DPSflattenpath;
- (void) DPSinitclip;
- (void) DPSlineto: (CGFloat)x : (CGFloat)y;
- (void) DPSmoveto: (CGFloat)x : (CGFloat)y;
- (void) DPSnewpath;
- (void) DPSpathbbox: (CGFloat*)llx : (CGFloat*)lly : (CGFloat*)urx : (CGFloat*)ury;
- (void) DPSrcurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 
		    : (CGFloat)x3 : (CGFloat)y3;
- (void) DPSrectclip: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h;
- (void) DPSrectfill: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h;
- (void) DPSrectstroke: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h;
- (void) DPSreversepath;
- (void) DPSrlineto: (CGFloat)x : (CGFloat)y;
- (void) DPSrmoveto: (CGFloat)x : (CGFloat)y;
- (void) DPSstroke;

- (void) GSSendBezierPath: (NSBezierPath *)path;
- (void) GSRectClipList: (const NSRect *)rects : (int) count;
- (void) GSRectFillList: (const NSRect *)rects : (int) count;

- (NSDictionary *) GSReadRect: (NSRect)rect;

- (void)DPSimage: (NSAffineTransform*) matrix 
		: (NSInteger) pixelsWide : (NSInteger) pixelsHigh
		: (NSInteger) bitsPerSample : (NSInteger) samplesPerPixel 
		: (NSInteger) bitsPerPixel : (NSInteger) bytesPerRow : (BOOL) isPlanar
		: (BOOL) hasAlpha : (NSString *) colorSpaceName
		: (const unsigned char *const [5]) data;

- (void) DPSshfill: (NSDictionary *)shader;

@end

@interface GSGState (PatternColor)

- (void *) saveClip;
- (void) restoreClip: (void *)savedClip;
- (void) fillRect: (NSRect)rect withPattern: (NSImage*)pattern;
- (void) fillPath: (NSBezierPath*)fillPath withPattern: (NSImage*)pattern;
- (void) eofillPath: (NSBezierPath*)fillPath withPattern: (NSImage*)pattern;

@end

@interface GSGState (NSGradient)

- (void) drawGradient: (NSGradient*)gradient
           fromCenter: (NSPoint)startCenter
               radius: (CGFloat)startRadius
             toCenter: (NSPoint)endCenter 
               radius: (CGFloat)endRadius
              options: (NSUInteger)options;

- (void) drawGradient: (NSGradient*)gradient
            fromPoint: (NSPoint)startPoint
              toPoint: (NSPoint)endPoint
              options: (NSUInteger)options;

@end

#endif
