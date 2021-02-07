/* GSMethodTable.h - Definitions of PostScript methods for NSGraphicsContext

   Copyright (C) 1998 Free Software Foundation, Inc.
   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Nov 1998
   Updated by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   
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

#ifndef _GSMethodTable_h_INCLUDE
#define _GSMethodTable_h_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSFont.h>

@class NSAffineTransform;
@class NSDate;
@class NSString;
@class NSBezierPath;
@class NSColor;
@class NSEvent;
@class NSGraphicsContext;

typedef struct {

/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
  void (*DPScurrentalpha_)
        (NSGraphicsContext*, SEL, CGFloat*);
  void (*DPScurrentcmykcolor____)
        (NSGraphicsContext*, SEL, CGFloat*, CGFloat*, CGFloat*, CGFloat*);
  void (*DPScurrentgray_)
        (NSGraphicsContext*, SEL, CGFloat*);
  void (*DPScurrenthsbcolor___)
        (NSGraphicsContext*, SEL, CGFloat*, CGFloat*, CGFloat*);
  void (*DPScurrentrgbcolor___)
        (NSGraphicsContext*, SEL, CGFloat*, CGFloat*, CGFloat*);
  void (*DPSsetalpha_)
        (NSGraphicsContext*, SEL, CGFloat);
  void (*DPSsetcmykcolor____)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat);
  void (*DPSsetgray_)
        (NSGraphicsContext*, SEL, CGFloat);
  void (*DPSsethsbcolor___)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat);
  void (*DPSsetrgbcolor___)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat);

  void (*GSSetFillColorspace_)
        (NSGraphicsContext*, SEL, NSDictionary *);
  void (*GSSetStrokeColorspace_)
        (NSGraphicsContext*, SEL, NSDictionary *);
  void (*GSSetFillColor_)
        (NSGraphicsContext*, SEL, CGFloat *);
  void (*GSSetStrokeColor_)
        (NSGraphicsContext*, SEL, CGFloat *);

/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */
  void (*DPSashow___)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, const char*);
  void (*DPSawidthshow______)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, int, CGFloat, CGFloat, const char*);
  void (*DPScharpath__)
        (NSGraphicsContext*, SEL, const char*, int);
  void (*DPSshow_)
        (NSGraphicsContext*, SEL, const char*);
  void (*DPSwidthshow____)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, int, const char*);
  void (*DPSxshow___)
        (NSGraphicsContext*, SEL, const char*, const CGFloat*, int);
  void (*DPSxyshow___)
        (NSGraphicsContext*, SEL, const char*, const CGFloat*, int);
  void (*DPSyshow___)
        (NSGraphicsContext*, SEL, const char*, const CGFloat*, int);

  void (*GSSetCharacterSpacing_)
        (NSGraphicsContext*, SEL, CGFloat);
  void (*GSSetFont_)
        (NSGraphicsContext*, SEL, NSFont*);
  void (*GSSetFontSize_)
        (NSGraphicsContext*, SEL, CGFloat);
  NSAffineTransform * (*GSGetTextCTM)
        (NSGraphicsContext*, SEL);
  NSPoint (*GSGetTextPosition)
        (NSGraphicsContext*, SEL);
  void (*GSSetTextCTM_)
        (NSGraphicsContext*, SEL, NSAffineTransform *);
  void (*GSSetTextDrawingMode_)
        (NSGraphicsContext*, SEL, GSTextDrawingMode);
  void (*GSSetTextPosition_)
        (NSGraphicsContext*, SEL, NSPoint);
  void (*GSShowText__)
        (NSGraphicsContext*, SEL, const char *, size_t);
  void (*GSShowGlyphs__)
        (NSGraphicsContext*, SEL, const NSGlyph *, size_t);
  void (*GSShowGlyphsWithAdvances__)
        (NSGraphicsContext*, SEL, const NSGlyph *, const NSSize *, size_t);

/* ----------------------------------------------------------------------- */
/* Gstate Handling */
/* ----------------------------------------------------------------------- */
  void (*DPSgrestore)
        (NSGraphicsContext*, SEL);
  void (*DPSgsave)
        (NSGraphicsContext*, SEL);
  void (*DPSinitgraphics)
        (NSGraphicsContext*, SEL);
  void (*DPSsetgstate_)
        (NSGraphicsContext*, SEL, NSInteger);

  NSInteger (*GSDefineGState)
        (NSGraphicsContext*, SEL);
  void (*GSUndefineGState_)
        (NSGraphicsContext*, SEL, NSInteger);
  void (*GSReplaceGState_)
        (NSGraphicsContext*, SEL, NSInteger);

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
  void (*DPScurrentflat_)
        (NSGraphicsContext*, SEL, CGFloat*);
  void (*DPScurrentlinecap_)
        (NSGraphicsContext*, SEL, int*);
  void (*DPScurrentlinejoin_)
        (NSGraphicsContext*, SEL, int*);
  void (*DPScurrentlinewidth_)
        (NSGraphicsContext*, SEL, CGFloat*);
  void (*DPScurrentmiterlimit_)
        (NSGraphicsContext*, SEL, CGFloat*);
  void (*DPScurrentpoint__)
        (NSGraphicsContext*, SEL, CGFloat*, CGFloat*);
  void (*DPScurrentstrokeadjust_)
        (NSGraphicsContext*, SEL, int*);
  void (*DPSsetdash___)
        (NSGraphicsContext*, SEL, const CGFloat*, NSInteger, CGFloat);
  void (*DPSsetflat_)
        (NSGraphicsContext*, SEL, CGFloat);
  void (*DPSsethalftonephase__)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat);
  void (*DPSsetlinecap_)
        (NSGraphicsContext*, SEL, int);
  void (*DPSsetlinejoin_)
        (NSGraphicsContext*, SEL, int);
  void (*DPSsetlinewidth_)
        (NSGraphicsContext*, SEL, CGFloat);
  void (*DPSsetmiterlimit_)
        (NSGraphicsContext*, SEL, CGFloat);
  void (*DPSsetstrokeadjust_)
        (NSGraphicsContext*, SEL, int);

/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
  void (*DPSconcat_)
        (NSGraphicsContext*, SEL, const CGFloat*);
  void (*DPSinitmatrix)
        (NSGraphicsContext*, SEL);
  void (*DPSrotate_)
        (NSGraphicsContext*, SEL, CGFloat);
  void (*DPSscale__)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat);
  void (*DPStranslate__)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat);

  NSAffineTransform * (*GSCurrentCTM)
        (NSGraphicsContext*, SEL);
  void (*GSSetCTM_)
        (NSGraphicsContext*, SEL, NSAffineTransform *);
  void (*GSConcatCTM_)
        (NSGraphicsContext*, SEL, NSAffineTransform *);

/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
  void (*DPSarc_____)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat, CGFloat);
  void (*DPSarcn_____)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat, CGFloat);
  void (*DPSarct_____)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat, CGFloat);
  void (*DPSclip)
        (NSGraphicsContext*, SEL);
  void (*DPSclosepath)
        (NSGraphicsContext*, SEL);
  void (*DPScurveto______)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat, CGFloat, CGFloat);
  void (*DPSeoclip)
        (NSGraphicsContext*, SEL);
  void (*DPSeofill)
        (NSGraphicsContext*, SEL);
  void (*DPSfill)
        (NSGraphicsContext*, SEL);
  void (*DPSflattenpath)
        (NSGraphicsContext*, SEL);
  void (*DPSinitclip)
        (NSGraphicsContext*, SEL);
  void (*DPSlineto__)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat);
  void (*DPSmoveto__)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat);
  void (*DPSnewpath)
        (NSGraphicsContext*, SEL);
  void (*DPSpathbbox____)
        (NSGraphicsContext*, SEL, CGFloat*, CGFloat*, CGFloat*, CGFloat*);
  void (*DPSrcurveto______)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat, CGFloat, CGFloat);
  void (*DPSrectclip____)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat);
  void (*DPSrectfill____)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat);
  void (*DPSrectstroke____)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat);
  void (*DPSreversepath)
        (NSGraphicsContext*, SEL);
  void (*DPSrlineto__)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat);
  void (*DPSrmoveto__)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat);
  void (*DPSstroke)
        (NSGraphicsContext*, SEL);

  void (*GSSendBezierPath_)
        (NSGraphicsContext*, SEL, NSBezierPath *);
  void (*GSRectClipList__)
        (NSGraphicsContext*, SEL, const NSRect *, int);
  void (*GSRectFillList__)
        (NSGraphicsContext*, SEL, const NSRect *, int);

/* ----------------------------------------------------------------------- */
/* Window system ops */
/* ----------------------------------------------------------------------- */
  void (*GSCurrentDevice___)
        (NSGraphicsContext*, SEL, void**, int*, int*);
  void (*DPScurrentoffset__)
        (NSGraphicsContext*, SEL, int*, int*);
  void (*GSSetDevice___)
        (NSGraphicsContext*, SEL, void*, int, int);
  void (*DPSsetoffset__)
        (NSGraphicsContext*, SEL, short int, short int);

/*-------------------------------------------------------------------------*/
/* Graphics Extensions Ops */
/*-------------------------------------------------------------------------*/
  void (*DPScomposite________)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat, NSInteger, CGFloat, CGFloat, NSCompositingOperation);
  void (*DPScompositerect_____)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat, NSCompositingOperation);
  void (*DPSdissolve________)
        (NSGraphicsContext*, SEL, CGFloat, CGFloat, CGFloat, CGFloat, NSInteger, CGFloat, CGFloat, CGFloat);

  void (*GSDrawImage__)
        (NSGraphicsContext*, SEL, NSRect, void *);

/* ----------------------------------------------------------------------- */
/* Postscript Client functions */
/* ----------------------------------------------------------------------- */
  void (*DPSPrintf__)
        (NSGraphicsContext*, SEL, const char *, va_list);
  void (*DPSWriteData__)
        (NSGraphicsContext*, SEL, const char *, unsigned int);

/* ----------------------------------------------------------------------- */
/* NSGraphics Ops */	
/* ----------------------------------------------------------------------- */
  NSDictionary * (*GSReadRect_)
        (NSGraphicsContext*, SEL, NSRect);

  void (*NSBeep)
        (NSGraphicsContext*, SEL);

/* Context helper wraps */
  void (*GSWSetViewIsFlipped_)
        (NSGraphicsContext*, SEL, BOOL);
  BOOL (*GSWViewIsFlipped)
        (NSGraphicsContext*, SEL);

/*
 * Render Bitmap Images
 */
  void (*NSDrawBitmap___________)(NSGraphicsContext*, SEL, NSRect rect,
                  NSInteger pixelsWide,
                  NSInteger pixelsHigh,
                  NSInteger bitsPerSample,
                  NSInteger samplesPerPixel,
                  NSInteger bitsPerPixel,
                  NSInteger bytesPerRow, 
                  BOOL isPlanar,
                  BOOL hasAlpha, 
                  NSString *colorSpaceName, 
                  const unsigned char *const data[5]);

  /* This probably belongs next to DPSstroke, but inserting members in this
  struct breaks apps that use PS* or DPS* functions and were compiled with
  an earlier version, so it's here until we figure out how to handle that.
  */
  void (*DPSshfill)
        (NSGraphicsContext*, SEL, NSDictionary *);
} gsMethodTable;

#endif
