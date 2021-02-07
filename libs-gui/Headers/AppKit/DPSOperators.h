/* DPSOperators - Drawing engine operators that require context

   Copyright (C) 1999 Free Software Foundation, Inc.
   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Based on code by Adam Fedor
   Date: Feb 1999
   
   This file is part of the GNU Objective C User Interface library.

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

#ifndef _DPSOperators_h_INCLUDE
#define _DPSOperators_h_INCLUDE
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSGraphicsContext.h>

#define	GSCTXT	NSGraphicsContext

/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
static inline void
DPScurrentalpha(GSCTXT *ctxt, CGFloat* a)
__attribute__((unused));

static inline void
DPScurrentcmykcolor(GSCTXT *ctxt, CGFloat* c, CGFloat* m, CGFloat* y, CGFloat* k)
__attribute__((unused));

static inline void
DPScurrentgray(GSCTXT *ctxt, CGFloat* gray)
__attribute__((unused));

static inline void
DPScurrenthsbcolor(GSCTXT *ctxt, CGFloat* h, CGFloat* s, CGFloat* b)
__attribute__((unused));

static inline void
DPScurrentrgbcolor(GSCTXT *ctxt, CGFloat* r, CGFloat* g, CGFloat* b)
__attribute__((unused));

static inline void
DPSsetalpha(GSCTXT *ctxt, CGFloat a)
__attribute__((unused));

static inline void
DPSsetcmykcolor(GSCTXT *ctxt, CGFloat c, CGFloat m, CGFloat y, CGFloat k)
__attribute__((unused));

static inline void
DPSsetgray(GSCTXT *ctxt, CGFloat gray)
__attribute__((unused));

static inline void
DPSsethsbcolor(GSCTXT *ctxt, CGFloat h, CGFloat s, CGFloat b)
__attribute__((unused));

static inline void
DPSsetrgbcolor(GSCTXT *ctxt, CGFloat r, CGFloat g, CGFloat b)
__attribute__((unused));


static inline void
GSSetFillColorspace(GSCTXT *ctxt, NSDictionary * dict)
__attribute__((unused));

static inline void
GSSetStrokeColorspace(GSCTXT *ctxt, NSDictionary * dict)
__attribute__((unused));

static inline void
GSSetFillColor(GSCTXT *ctxt, CGFloat * values)
__attribute__((unused));

static inline void
GSSetStrokeColor(GSCTXT *ctxt, CGFloat * values)
__attribute__((unused));


/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */
static inline void
DPSashow(GSCTXT *ctxt, CGFloat x, CGFloat y, const char* s)
__attribute__((unused));

static inline void
DPSawidthshow(GSCTXT *ctxt, CGFloat cx, CGFloat cy, int c, CGFloat ax, CGFloat ay, const char* s)
__attribute__((unused));

static inline void
DPScharpath(GSCTXT *ctxt, const char* s, int b)
__attribute__((unused));

static inline void
DPSshow(GSCTXT *ctxt, const char* s)
__attribute__((unused));

static inline void
DPSwidthshow(GSCTXT *ctxt, CGFloat x, CGFloat y, int c, const char* s)
__attribute__((unused));

static inline void
DPSxshow(GSCTXT *ctxt, const char* s, const CGFloat* numarray, int size)
__attribute__((unused));

static inline void
DPSxyshow(GSCTXT *ctxt, const char* s, const CGFloat* numarray, int size)
__attribute__((unused));

static inline void
DPSyshow(GSCTXT *ctxt, const char* s, const CGFloat* numarray, int size)
__attribute__((unused));


static inline void
GSSetCharacterSpacing(GSCTXT *ctxt, CGFloat extra)
__attribute__((unused));

static inline void
GSSetFont(GSCTXT *ctxt, NSFont* font)
__attribute__((unused));

static inline void
GSSetFontSize(GSCTXT *ctxt, CGFloat size)
__attribute__((unused));

static inline NSAffineTransform *
GSGetTextCTM(GSCTXT *ctxt)
__attribute__((unused));

static inline NSPoint
GSGetTextPosition(GSCTXT *ctxt)
__attribute__((unused));

static inline void
GSSetTextCTM(GSCTXT *ctxt, NSAffineTransform * ctm)
__attribute__((unused));

static inline void
GSSetTextDrawingMode(GSCTXT *ctxt, GSTextDrawingMode mode)
__attribute__((unused));

static inline void
GSSetTextPosition(GSCTXT *ctxt, NSPoint loc)
__attribute__((unused));

static inline void
GSShowText(GSCTXT *ctxt, const char * string, size_t length)
__attribute__((unused));

static inline void
GSShowGlyphs(GSCTXT *ctxt, const NSGlyph * glyphs, size_t length)
__attribute__((unused));

static inline void
GSShowGlyphsWithAdvances(GSCTXT *ctxt, const NSGlyph * glyphs, const NSSize * advances, size_t length)
__attribute__((unused));



/* ----------------------------------------------------------------------- */
/* Gstate Handling */
/* ----------------------------------------------------------------------- */
static inline void
DPSgrestore(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSgsave(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSinitgraphics(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSsetgstate(GSCTXT *ctxt, NSInteger gst)
__attribute__((unused));


static inline NSInteger
GSDefineGState(GSCTXT *ctxt)
__attribute__((unused));

static inline void
GSUndefineGState(GSCTXT *ctxt, NSInteger gst)
__attribute__((unused));

static inline void
GSReplaceGState(GSCTXT *ctxt, NSInteger gst)
__attribute__((unused));

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
static inline void
DPScurrentflat(GSCTXT *ctxt, CGFloat* flatness)
__attribute__((unused));

static inline void
DPScurrentlinecap(GSCTXT *ctxt, int* linecap)
__attribute__((unused));

static inline void
DPScurrentlinejoin(GSCTXT *ctxt, int* linejoin)
__attribute__((unused));

static inline void
DPScurrentlinewidth(GSCTXT *ctxt, CGFloat* width)
__attribute__((unused));

static inline void
DPScurrentmiterlimit(GSCTXT *ctxt, CGFloat* limit)
__attribute__((unused));

static inline void
DPScurrentpoint(GSCTXT *ctxt, CGFloat* x, CGFloat* y)
__attribute__((unused));

static inline void
DPScurrentstrokeadjust(GSCTXT *ctxt, int* b)
__attribute__((unused));

static inline void
DPSsetdash(GSCTXT *ctxt, const CGFloat* pat, NSInteger size, CGFloat offset)
__attribute__((unused));

static inline void
DPSsetflat(GSCTXT *ctxt, CGFloat flatness)
__attribute__((unused));

static inline void
DPSsethalftonephase(GSCTXT *ctxt, CGFloat x, CGFloat y)
__attribute__((unused));

static inline void
DPSsetlinecap(GSCTXT *ctxt, int linecap)
__attribute__((unused));

static inline void
DPSsetlinejoin(GSCTXT *ctxt, int linejoin)
__attribute__((unused));

static inline void
DPSsetlinewidth(GSCTXT *ctxt, CGFloat width)
__attribute__((unused));

static inline void
DPSsetmiterlimit(GSCTXT *ctxt, CGFloat limit)
__attribute__((unused));

static inline void
DPSsetstrokeadjust(GSCTXT *ctxt, int b)
__attribute__((unused));


/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
static inline void
DPSconcat(GSCTXT *ctxt, const CGFloat* m)
__attribute__((unused));

static inline void
DPSinitmatrix(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSrotate(GSCTXT *ctxt, CGFloat angle)
__attribute__((unused));

static inline void
DPSscale(GSCTXT *ctxt, CGFloat x, CGFloat y)
__attribute__((unused));

static inline void
DPStranslate(GSCTXT *ctxt, CGFloat x, CGFloat y)
__attribute__((unused));


static inline NSAffineTransform *
GSCurrentCTM(GSCTXT *ctxt)
__attribute__((unused));

static inline void
GSSetCTM(GSCTXT *ctxt, NSAffineTransform * ctm)
__attribute__((unused));

static inline void
GSConcatCTM(GSCTXT *ctxt, NSAffineTransform * ctm)
__attribute__((unused));


/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
static inline void
DPSarc(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat r, CGFloat angle1, CGFloat angle2)
__attribute__((unused));

static inline void
DPSarcn(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat r, CGFloat angle1, CGFloat angle2)
__attribute__((unused));

static inline void
DPSarct(GSCTXT *ctxt, CGFloat x1, CGFloat y1, CGFloat x2, CGFloat y2, CGFloat r)
__attribute__((unused));

static inline void
DPSclip(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSclosepath(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPScurveto(GSCTXT *ctxt, CGFloat x1, CGFloat y1, CGFloat x2, CGFloat y2, CGFloat x3, CGFloat y3)
__attribute__((unused));

static inline void
DPSeoclip(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSeofill(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSfill(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSflattenpath(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSinitclip(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSlineto(GSCTXT *ctxt, CGFloat x, CGFloat y)
__attribute__((unused));

static inline void
DPSmoveto(GSCTXT *ctxt, CGFloat x, CGFloat y)
__attribute__((unused));

static inline void
DPSnewpath(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSpathbbox(GSCTXT *ctxt, CGFloat* llx, CGFloat* lly, CGFloat* urx, CGFloat* ury)
__attribute__((unused));

static inline void
DPSrcurveto(GSCTXT *ctxt, CGFloat x1, CGFloat y1, CGFloat x2, CGFloat y2, CGFloat x3, CGFloat y3)
__attribute__((unused));

static inline void
DPSrectclip(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h)
__attribute__((unused));

static inline void
DPSrectfill(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h)
__attribute__((unused));

static inline void
DPSrectstroke(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h)
__attribute__((unused));

static inline void
DPSreversepath(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSrlineto(GSCTXT *ctxt, CGFloat x, CGFloat y)
__attribute__((unused));

static inline void
DPSrmoveto(GSCTXT *ctxt, CGFloat x, CGFloat y)
__attribute__((unused));

static inline void
DPSstroke(GSCTXT *ctxt)
__attribute__((unused));

static inline void
DPSshfill(GSCTXT *ctxt, NSDictionary *shaderDictionary)
__attribute__((unused));


static inline void
GSSendBezierPath(GSCTXT *ctxt, NSBezierPath * path)
__attribute__((unused));

static inline void
GSRectClipList(GSCTXT *ctxt, const NSRect * rects, int count)
__attribute__((unused));

static inline void
GSRectFillList(GSCTXT *ctxt, const NSRect * rects, int count)
__attribute__((unused));


/* ----------------------------------------------------------------------- */
/* Window system ops */
/* ----------------------------------------------------------------------- */
static inline void
GSCurrentDevice(GSCTXT *ctxt, void** device, int* x, int* y)
__attribute__((unused));

static inline void
DPScurrentoffset(GSCTXT *ctxt, int* x, int* y)
__attribute__((unused));

static inline void
GSSetDevice(GSCTXT *ctxt, void* device, int x, int y)
__attribute__((unused));

static inline void
DPSsetoffset(GSCTXT *ctxt, short int x, short int y)
__attribute__((unused));


/*-------------------------------------------------------------------------*/
/* Graphics Extensions Ops */
/*-------------------------------------------------------------------------*/
static inline void
DPScomposite(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h, NSInteger gstateNum, CGFloat dx, CGFloat dy, NSCompositingOperation op)
__attribute__((unused));

static inline void
DPScompositerect(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h, NSCompositingOperation op)
__attribute__((unused));

static inline void
DPSdissolve(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h, NSInteger gstateNum, CGFloat dx, CGFloat dy, CGFloat delta)
__attribute__((unused));


static inline void
GSDrawImage(GSCTXT *ctxt, NSRect rect, void * imageref)
__attribute__((unused));

/* ----------------------------------------------------------------------- */
/* Postscript Client functions */
/* ----------------------------------------------------------------------- */
static void
DPSPrintf(GSCTXT *ctxt, const char * fmt, ...)
__attribute__((unused));

static inline void
DPSWriteData(GSCTXT *ctxt, const char * buf, unsigned int count)
__attribute__((unused));

/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
static inline void
DPScurrentalpha(GSCTXT *ctxt, CGFloat* a)
{
  (ctxt->methods->DPScurrentalpha_)
    (ctxt, @selector(DPScurrentalpha:), a);
}

static inline void
DPScurrentcmykcolor(GSCTXT *ctxt, CGFloat* c, CGFloat* m, CGFloat* y, CGFloat* k)
{
  (ctxt->methods->DPScurrentcmykcolor____)
    (ctxt, @selector(DPScurrentcmykcolor: : : :), c, m, y, k);
}

static inline void
DPScurrentgray(GSCTXT *ctxt, CGFloat* gray)
{
  (ctxt->methods->DPScurrentgray_)
    (ctxt, @selector(DPScurrentgray:), gray);
}

static inline void
DPScurrenthsbcolor(GSCTXT *ctxt, CGFloat* h, CGFloat* s, CGFloat* b)
{
  (ctxt->methods->DPScurrenthsbcolor___)
    (ctxt, @selector(DPScurrenthsbcolor: : :), h, s, b);
}

static inline void
DPScurrentrgbcolor(GSCTXT *ctxt, CGFloat* r, CGFloat* g, CGFloat* b)
{
  (ctxt->methods->DPScurrentrgbcolor___)
    (ctxt, @selector(DPScurrentrgbcolor: : :), r, g, b);
}

static inline void
DPSsetalpha(GSCTXT *ctxt, CGFloat a)
{
  (ctxt->methods->DPSsetalpha_)
    (ctxt, @selector(DPSsetalpha:), a);
}

static inline void
DPSsetcmykcolor(GSCTXT *ctxt, CGFloat c, CGFloat m, CGFloat y, CGFloat k)
{
  (ctxt->methods->DPSsetcmykcolor____)
    (ctxt, @selector(DPSsetcmykcolor: : : :), c, m, y, k);
}

static inline void
DPSsetgray(GSCTXT *ctxt, CGFloat gray)
{
  (ctxt->methods->DPSsetgray_)
    (ctxt, @selector(DPSsetgray:), gray);
}

static inline void
DPSsethsbcolor(GSCTXT *ctxt, CGFloat h, CGFloat s, CGFloat b)
{
  (ctxt->methods->DPSsethsbcolor___)
    (ctxt, @selector(DPSsethsbcolor: : :), h, s, b);
}

static inline void
DPSsetrgbcolor(GSCTXT *ctxt, CGFloat r, CGFloat g, CGFloat b)
{
  (ctxt->methods->DPSsetrgbcolor___)
    (ctxt, @selector(DPSsetrgbcolor: : :), r, g, b);
}


static inline void
GSSetFillColorspace(GSCTXT *ctxt, NSDictionary * dict)
{
  (ctxt->methods->GSSetFillColorspace_)
    (ctxt, @selector(GSSetFillColorspace:), dict);
}

static inline void
GSSetStrokeColorspace(GSCTXT *ctxt, NSDictionary * dict)
{
  (ctxt->methods->GSSetStrokeColorspace_)
    (ctxt, @selector(GSSetStrokeColorspace:), dict);
}

static inline void
GSSetFillColor(GSCTXT *ctxt, CGFloat * values)
{
  (ctxt->methods->GSSetFillColor_)
    (ctxt, @selector(GSSetFillColor:), values);
}

static inline void
GSSetStrokeColor(GSCTXT *ctxt, CGFloat * values)
{
  (ctxt->methods->GSSetStrokeColor_)
    (ctxt, @selector(GSSetStrokeColor:), values);
}


/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */
static inline void
DPSashow(GSCTXT *ctxt, CGFloat x, CGFloat y, const char* s)
{
  (ctxt->methods->DPSashow___)
    (ctxt, @selector(DPSashow: : :), x, y, s);
}

static inline void
DPSawidthshow(GSCTXT *ctxt, CGFloat cx, CGFloat cy, int c, CGFloat ax, CGFloat ay, const char* s)
{
  (ctxt->methods->DPSawidthshow______)
    (ctxt, @selector(DPSawidthshow: : : : : :), cx, cy, c, ax, ay, s);
}

static inline void
DPScharpath(GSCTXT *ctxt, const char* s, int b)
{
  (ctxt->methods->DPScharpath__)
    (ctxt, @selector(DPScharpath: :), s, b);
}

static inline void
DPSshow(GSCTXT *ctxt, const char* s)
{
  (ctxt->methods->DPSshow_)
    (ctxt, @selector(DPSshow:), s);
}

static inline void
DPSwidthshow(GSCTXT *ctxt, CGFloat x, CGFloat y, int c, const char* s)
{
  (ctxt->methods->DPSwidthshow____)
    (ctxt, @selector(DPSwidthshow: : : :), x, y, c, s);
}

static inline void
DPSxshow(GSCTXT *ctxt, const char* s, const CGFloat* numarray, int size)
{
  (ctxt->methods->DPSxshow___)
    (ctxt, @selector(DPSxshow: : :), s, numarray, size);
}

static inline void
DPSxyshow(GSCTXT *ctxt, const char* s, const CGFloat* numarray, int size)
{
  (ctxt->methods->DPSxyshow___)
    (ctxt, @selector(DPSxyshow: : :), s, numarray, size);
}

static inline void
DPSyshow(GSCTXT *ctxt, const char* s, const CGFloat* numarray, int size)
{
  (ctxt->methods->DPSyshow___)
    (ctxt, @selector(DPSyshow: : :), s, numarray, size);
}


static inline void
GSSetCharacterSpacing(GSCTXT *ctxt, CGFloat extra)
{
  (ctxt->methods->GSSetCharacterSpacing_)
    (ctxt, @selector(GSSetCharacterSpacing:), extra);
}

static inline void
GSSetFont(GSCTXT *ctxt, NSFont* font)
{
  (ctxt->methods->GSSetFont_)
    (ctxt, @selector(GSSetFont:), font);
}

static inline void
GSSetFontSize(GSCTXT *ctxt, CGFloat size)
{
  (ctxt->methods->GSSetFontSize_)
    (ctxt, @selector(GSSetFontSize:), size);
}

static inline NSAffineTransform *
GSGetTextCTM(GSCTXT *ctxt)
{
  return (ctxt->methods->GSGetTextCTM)
    (ctxt, @selector(GSGetTextCTM));
}

static inline NSPoint
GSGetTextPosition(GSCTXT *ctxt)
{
  return (ctxt->methods->GSGetTextPosition)
    (ctxt, @selector(GSGetTextPosition));
}

static inline void
GSSetTextCTM(GSCTXT *ctxt, NSAffineTransform * ctm)
{
  (ctxt->methods->GSSetTextCTM_)
    (ctxt, @selector(GSSetTextCTM:), ctm);
}

static inline void
GSSetTextDrawingMode(GSCTXT *ctxt, GSTextDrawingMode mode)
{
  (ctxt->methods->GSSetTextDrawingMode_)
    (ctxt, @selector(GSSetTextDrawingMode:), mode);
}

static inline void
GSSetTextPosition(GSCTXT *ctxt, NSPoint loc)
{
  (ctxt->methods->GSSetTextPosition_)
    (ctxt, @selector(GSSetTextPosition:), loc);
}

static inline void
GSShowText(GSCTXT *ctxt, const char * string, size_t length)
{
  (ctxt->methods->GSShowText__)
    (ctxt, @selector(GSShowText: :), string, length);
}

static inline void
GSShowGlyphs(GSCTXT *ctxt, const NSGlyph * glyphs, size_t length)
{
  (ctxt->methods->GSShowGlyphs__)
    (ctxt, @selector(GSShowGlyphs: :), glyphs, length);
}

static inline void
GSShowGlyphsWithAdvances(GSCTXT *ctxt, const NSGlyph * glyphs, const NSSize * advances, size_t length)
{
  (ctxt->methods->GSShowGlyphsWithAdvances__)
    (ctxt, @selector(GSShowGlyphsWithAdvances: :), glyphs, advances, length); 
}


/* ----------------------------------------------------------------------- */
/* Gstate Handling */
/* ----------------------------------------------------------------------- */
static inline void
DPSgrestore(GSCTXT *ctxt)
{
  (ctxt->methods->DPSgrestore)
    (ctxt, @selector(DPSgrestore));
}

static inline void
DPSgsave(GSCTXT *ctxt)
{
  (ctxt->methods->DPSgsave)
    (ctxt, @selector(DPSgsave));
}

static inline void
DPSinitgraphics(GSCTXT *ctxt)
{
  (ctxt->methods->DPSinitgraphics)
    (ctxt, @selector(DPSinitgraphics));
}

static inline void
DPSsetgstate(GSCTXT *ctxt, NSInteger gst)
{
  (ctxt->methods->DPSsetgstate_)
    (ctxt, @selector(DPSsetgstate:), gst);
}


static inline NSInteger
GSDefineGState(GSCTXT *ctxt)
{
  return (ctxt->methods->GSDefineGState)
    (ctxt, @selector(GSDefineGState));
}

static inline void
GSUndefineGState(GSCTXT *ctxt, NSInteger gst)
{
  (ctxt->methods->GSUndefineGState_)
    (ctxt, @selector(GSUndefineGState:), gst);
}

static inline void
GSReplaceGState(GSCTXT *ctxt, NSInteger gst)
{
  (ctxt->methods->GSReplaceGState_)
    (ctxt, @selector(GSReplaceGState:), gst);
}

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
static inline void
DPScurrentflat(GSCTXT *ctxt, CGFloat* flatness)
{
  (ctxt->methods->DPScurrentflat_)
    (ctxt, @selector(DPScurrentflat:), flatness);
}

static inline void
DPScurrentlinecap(GSCTXT *ctxt, int* linecap)
{
  (ctxt->methods->DPScurrentlinecap_)
    (ctxt, @selector(DPScurrentlinecap:), linecap);
}

static inline void
DPScurrentlinejoin(GSCTXT *ctxt, int* linejoin)
{
  (ctxt->methods->DPScurrentlinejoin_)
    (ctxt, @selector(DPScurrentlinejoin:), linejoin);
}

static inline void
DPScurrentlinewidth(GSCTXT *ctxt, CGFloat* width)
{
  (ctxt->methods->DPScurrentlinewidth_)
    (ctxt, @selector(DPScurrentlinewidth:), width);
}

static inline void
DPScurrentmiterlimit(GSCTXT *ctxt, CGFloat* limit)
{
  (ctxt->methods->DPScurrentmiterlimit_)
    (ctxt, @selector(DPScurrentmiterlimit:), limit);
}

static inline void
DPScurrentpoint(GSCTXT *ctxt, CGFloat* x, CGFloat* y)
{
  (ctxt->methods->DPScurrentpoint__)
    (ctxt, @selector(DPScurrentpoint: :), x, y);
}

static inline void
DPScurrentstrokeadjust(GSCTXT *ctxt, int* b)
{
  (ctxt->methods->DPScurrentstrokeadjust_)
    (ctxt, @selector(DPScurrentstrokeadjust:), b);
}

static inline void
DPSsetdash(GSCTXT *ctxt, const CGFloat* pat, NSInteger size, CGFloat offset)
{
  (ctxt->methods->DPSsetdash___)
    (ctxt, @selector(DPSsetdash: : :), pat, size, offset);
}

static inline void
DPSsetflat(GSCTXT *ctxt, CGFloat flatness)
{
  (ctxt->methods->DPSsetflat_)
    (ctxt, @selector(DPSsetflat:), flatness);
}

static inline void
DPSsethalftonephase(GSCTXT *ctxt, CGFloat x, CGFloat y)
{
  (ctxt->methods->DPSsethalftonephase__)
    (ctxt, @selector(DPSsethalftonephase: :), x, y);
}

static inline void
DPSsetlinecap(GSCTXT *ctxt, int linecap)
{
  (ctxt->methods->DPSsetlinecap_)
    (ctxt, @selector(DPSsetlinecap:), linecap);
}

static inline void
DPSsetlinejoin(GSCTXT *ctxt, int linejoin)
{
  (ctxt->methods->DPSsetlinejoin_)
    (ctxt, @selector(DPSsetlinejoin:), linejoin);
}

static inline void
DPSsetlinewidth(GSCTXT *ctxt, CGFloat width)
{
  (ctxt->methods->DPSsetlinewidth_)
    (ctxt, @selector(DPSsetlinewidth:), width);
}

static inline void
DPSsetmiterlimit(GSCTXT *ctxt, CGFloat limit)
{
  (ctxt->methods->DPSsetmiterlimit_)
    (ctxt, @selector(DPSsetmiterlimit:), limit);
}

static inline void
DPSsetstrokeadjust(GSCTXT *ctxt, int b)
{
  (ctxt->methods->DPSsetstrokeadjust_)
    (ctxt, @selector(DPSsetstrokeadjust:), b);
}


/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
static inline void
DPSconcat(GSCTXT *ctxt, const CGFloat* m)
{
  (ctxt->methods->DPSconcat_)
    (ctxt, @selector(DPSconcat:), m);
}

static inline void
DPSinitmatrix(GSCTXT *ctxt)
{
  (ctxt->methods->DPSinitmatrix)
    (ctxt, @selector(DPSinitmatrix));
}

static inline void
DPSrotate(GSCTXT *ctxt, CGFloat angle)
{
  (ctxt->methods->DPSrotate_)
    (ctxt, @selector(DPSrotate:), angle);
}

static inline void
DPSscale(GSCTXT *ctxt, CGFloat x, CGFloat y)
{
  (ctxt->methods->DPSscale__)
    (ctxt, @selector(DPSscale: :), x, y);
}

static inline void
DPStranslate(GSCTXT *ctxt, CGFloat x, CGFloat y)
{
  (ctxt->methods->DPStranslate__)
    (ctxt, @selector(DPStranslate: :), x, y);
}


static inline NSAffineTransform *
GSCurrentCTM(GSCTXT *ctxt)
{
  return (ctxt->methods->GSCurrentCTM)
    (ctxt, @selector(GSCurrentCTM));
}

static inline void
GSSetCTM(GSCTXT *ctxt, NSAffineTransform * ctm)
{
  (ctxt->methods->GSSetCTM_)
    (ctxt, @selector(GSSetCTM:), ctm);
}

static inline void
GSConcatCTM(GSCTXT *ctxt, NSAffineTransform * ctm)
{
  (ctxt->methods->GSConcatCTM_)
    (ctxt, @selector(GSConcatCTM:), ctm);
}


/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
static inline void
DPSarc(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat r, CGFloat angle1, CGFloat angle2)
{
  (ctxt->methods->DPSarc_____)
    (ctxt, @selector(DPSarc: : : : :), x, y, r, angle1, angle2);
}

static inline void
DPSarcn(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat r, CGFloat angle1, CGFloat angle2)
{
  (ctxt->methods->DPSarcn_____)
    (ctxt, @selector(DPSarcn: : : : :), x, y, r, angle1, angle2);
}

static inline void
DPSarct(GSCTXT *ctxt, CGFloat x1, CGFloat y1, CGFloat x2, CGFloat y2, CGFloat r)
{
  (ctxt->methods->DPSarct_____)
    (ctxt, @selector(DPSarct: : : : :), x1, y1, x2, y2, r);
}

static inline void
DPSclip(GSCTXT *ctxt)
{
  (ctxt->methods->DPSclip)
    (ctxt, @selector(DPSclip));
}

static inline void
DPSclosepath(GSCTXT *ctxt)
{
  (ctxt->methods->DPSclosepath)
    (ctxt, @selector(DPSclosepath));
}

static inline void
DPScurveto(GSCTXT *ctxt, CGFloat x1, CGFloat y1, CGFloat x2, CGFloat y2, CGFloat x3, CGFloat y3)
{
  (ctxt->methods->DPScurveto______)
    (ctxt, @selector(DPScurveto: : : : : :), x1, y1, x2, y2, x3, y3);
}

static inline void
DPSeoclip(GSCTXT *ctxt)
{
  (ctxt->methods->DPSeoclip)
    (ctxt, @selector(DPSeoclip));
}

static inline void
DPSeofill(GSCTXT *ctxt)
{
  (ctxt->methods->DPSeofill)
    (ctxt, @selector(DPSeofill));
}

static inline void
DPSfill(GSCTXT *ctxt)
{
  (ctxt->methods->DPSfill)
    (ctxt, @selector(DPSfill));
}

static inline void
DPSflattenpath(GSCTXT *ctxt)
{
  (ctxt->methods->DPSflattenpath)
    (ctxt, @selector(DPSflattenpath));
}

static inline void
DPSinitclip(GSCTXT *ctxt)
{
  (ctxt->methods->DPSinitclip)
    (ctxt, @selector(DPSinitclip));
}

static inline void
DPSlineto(GSCTXT *ctxt, CGFloat x, CGFloat y)
{
  (ctxt->methods->DPSlineto__)
    (ctxt, @selector(DPSlineto: :), x, y);
}

static inline void
DPSmoveto(GSCTXT *ctxt, CGFloat x, CGFloat y)
{
  (ctxt->methods->DPSmoveto__)
    (ctxt, @selector(DPSmoveto: :), x, y);
}

static inline void
DPSnewpath(GSCTXT *ctxt)
{
  (ctxt->methods->DPSnewpath)
    (ctxt, @selector(DPSnewpath));
}

static inline void
DPSpathbbox(GSCTXT *ctxt, CGFloat* llx, CGFloat* lly, CGFloat* urx, CGFloat* ury)
{
  (ctxt->methods->DPSpathbbox____)
    (ctxt, @selector(DPSpathbbox: : : :), llx, lly, urx, ury);
}

static inline void
DPSrcurveto(GSCTXT *ctxt, CGFloat x1, CGFloat y1, CGFloat x2, CGFloat y2, CGFloat x3, CGFloat y3)
{
  (ctxt->methods->DPSrcurveto______)
    (ctxt, @selector(DPSrcurveto: : : : : :), x1, y1, x2, y2, x3, y3);
}

static inline void
DPSrectclip(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h)
{
  (ctxt->methods->DPSrectclip____)
    (ctxt, @selector(DPSrectclip: : : :), x, y, w, h);
}

static inline void
DPSrectfill(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h)
{
  (ctxt->methods->DPSrectfill____)
    (ctxt, @selector(DPSrectfill: : : :), x, y, w, h);
}

static inline void
DPSrectstroke(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h)
{
  (ctxt->methods->DPSrectstroke____)
    (ctxt, @selector(DPSrectstroke: : : :), x, y, w, h);
}

static inline void
DPSreversepath(GSCTXT *ctxt)
{
  (ctxt->methods->DPSreversepath)
    (ctxt, @selector(DPSreversepath));
}

static inline void
DPSrlineto(GSCTXT *ctxt, CGFloat x, CGFloat y)
{
  (ctxt->methods->DPSrlineto__)
    (ctxt, @selector(DPSrlineto: :), x, y);
}

static inline void
DPSrmoveto(GSCTXT *ctxt, CGFloat x, CGFloat y)
{
  (ctxt->methods->DPSrmoveto__)
    (ctxt, @selector(DPSrmoveto: :), x, y);
}

static inline void
DPSstroke(GSCTXT *ctxt)
{
  (ctxt->methods->DPSstroke)
    (ctxt, @selector(DPSstroke));
}

static inline void
DPSshfill(GSCTXT *ctxt, NSDictionary *shaderDictionary)
{
  (ctxt->methods->DPSshfill)
    (ctxt, @selector(DPSshfill:), shaderDictionary);
}


static inline void
GSSendBezierPath(GSCTXT *ctxt, NSBezierPath * path)
{
  (ctxt->methods->GSSendBezierPath_)
    (ctxt, @selector(GSSendBezierPath:), path);
}

static inline void
GSRectClipList(GSCTXT *ctxt, const NSRect * rects, int count)
{
  (ctxt->methods->GSRectClipList__)
    (ctxt, @selector(GSRectClipList: :), rects, count);
}

static inline void
GSRectFillList(GSCTXT *ctxt, const NSRect * rects, int count)
{
  (ctxt->methods->GSRectFillList__)
    (ctxt, @selector(GSRectFillList: :), rects, count);
}


/* ----------------------------------------------------------------------- */
/* Window system ops */
/* ----------------------------------------------------------------------- */
static inline void
GSCurrentDevice(GSCTXT *ctxt, void** device, int* x, int* y)
{
  (ctxt->methods->GSCurrentDevice___)
    (ctxt, @selector(GSCurrentDevice: : :), device, x, y);
}

static inline void
DPScurrentoffset(GSCTXT *ctxt, int* x, int* y)
{
  (ctxt->methods->DPScurrentoffset__)
    (ctxt, @selector(DPScurrentoffset: :), x, y);
}

static inline void
GSSetDevice(GSCTXT *ctxt, void* device, int x, int y)
{
  (ctxt->methods->GSSetDevice___)
    (ctxt, @selector(GSSetDevice: : :), device, x, y);
}

static inline void
DPSsetoffset(GSCTXT *ctxt, short int x, short int y)
{
  (ctxt->methods->DPSsetoffset__)
    (ctxt, @selector(DPSsetoffset: :), x, y);
}


/*-------------------------------------------------------------------------*/
/* Graphics Extensions Ops */
/*-------------------------------------------------------------------------*/
static inline void
DPScomposite(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h,
             NSInteger gstateNum, CGFloat dx, CGFloat dy, NSCompositingOperation op)
{
  (ctxt->methods->DPScomposite________)
    (ctxt, @selector(DPScomposite: : : : : : : :), x, y, w, h, gstateNum, dx, dy, op);
}

static inline void
DPScompositerect(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h, NSCompositingOperation op)
{
  (ctxt->methods->DPScompositerect_____)
    (ctxt, @selector(DPScompositerect: : : : :), x, y, w, h, op);
}

static inline void
DPSdissolve(GSCTXT *ctxt, CGFloat x, CGFloat y, CGFloat w, CGFloat h, NSInteger gstateNum, CGFloat dx, CGFloat dy, CGFloat delta)
{
  (ctxt->methods->DPSdissolve________)
    (ctxt, @selector(DPSdissolve: : : : : : : :), x, y, w, h, gstateNum, dx, dy, delta);
}


static inline void
GSDrawImage(GSCTXT *ctxt, NSRect rect, void * imageref)
{
  (ctxt->methods->GSDrawImage__)
    (ctxt, @selector(GSDrawImage: :), rect, imageref);
}


/* ----------------------------------------------------------------------- */
/* Postscript Client functions */
/* ----------------------------------------------------------------------- */
static void
DPSPrintf(GSCTXT *ctxt, const char * fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  if (fmt != NULL)
    (ctxt->methods->DPSPrintf__)
      (ctxt, @selector(DPSPrintf: :), fmt, ap);
  va_end(ap);
}

static inline void
DPSWriteData(GSCTXT *ctxt, const char * buf, unsigned int count)
{
  (ctxt->methods->DPSWriteData__)
    (ctxt, @selector(DPSWriteData: :), buf, count);
}

#endif	
