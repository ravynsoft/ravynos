/* GSGState - Implements generic graphic state drawing for non-PS backends

   Copyright (C) 1995 Free Software Foundation, Inc.

   Written by: Adam Fedor <fedor@boulder.colorado.edu>
   Date: Nov 1995
   Extracted from XGPS: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2002
   
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

#ifndef _GSGState_h_INCLUDE
#define _GSGState_h_INCLUDE

#include <AppKit/NSGraphicsContext.h>   // needed for NSCompositingOperation
#include <Foundation/NSArray.h>
#include <Foundation/NSObject.h>
#include <gsc/gscolors.h>

@class NSAffineTransform;
@class NSBezierPath;
@class NSFont;
@class NSColorSpace;
@class GSContext;

typedef enum {
  path_stroke, path_fill, path_eofill, path_clip, path_eoclip
} ctxt_object_t;

typedef enum {
  COLOR_STROKE = 1,
  COLOR_FILL = 2,
  COLOR_BOTH = 3               /* COLOR_BOTH = COLOR_FILL || COLOR_STROKE */
} color_state_t;

@interface GSGState : NSObject <NSCopying>
{
@public
  GSContext *drawcontext;
  NSAffineTransform *ctm;
  NSPoint offset;               /* Offset from Drawable origin */
  NSBezierPath *path;	        /* Current path */
  GSFontInfo *font;             /* Current font reference */
  NSColorSpace *fillColorS;     /* Colorspace used for fill drawing */
  NSColorSpace *strokeColorS;   /* Colorspace used for stroke drawing */
  device_color_t fillColor;     /* fill color */
  device_color_t strokeColor;   /* stroke color */
  color_state_t  cstate;        /* state last time color was set */
  NSImage *pattern;             /* If set, image to draw with */   

  CGFloat   charSpacing;
  NSAffineTransform *textCtm;   /* Text transform - concat with ctm */
  GSTextDrawingMode textMode;
  BOOL viewIsFlipped;
  BOOL    _antialias;
  NSPoint _patternPhase;
  NSCompositingOperation _compositingOperation;
}

- initWithDrawContext: (GSContext *)context;
- deepen;

- (void) setOffset: (NSPoint)theOffset;
- (NSPoint) offset;

- (void) setColor: (device_color_t *)color state: (color_state_t)cState;
- (void) GSSetPatterColor: (NSImage*)image;

- (void) setShouldAntialias: (BOOL)antialias;
- (BOOL) shouldAntialias;
- (NSPoint) patternPhase;
- (void) setPatternPhase: (NSPoint)phase;
- (NSCompositingOperation) compositingOperation;
- (void) setCompositingOperation:(NSCompositingOperation) operation;

- (void) compositeGState: (GSGState *)source
                fromRect: (NSRect)aRect
                 toPoint: (NSPoint)aPoint
                      op: (NSCompositingOperation)op;

- (void) dissolveGState: (GSGState *)source
               fromRect: (NSRect)aRect
                toPoint: (NSPoint)aPoint
                  delta: (CGFloat)delta;

- (void) compositeGState: (GSGState *)source
                fromRect: (NSRect)aRect
                 toPoint: (NSPoint)aPoint
                      op: (NSCompositingOperation)op
                fraction: (CGFloat)delta;

- (void) compositerect: (NSRect)aRect
                    op: (NSCompositingOperation)op;

- (NSPoint) pointInMatrixSpace: (NSPoint)point;
- (NSPoint) deltaPointInMatrixSpace: (NSPoint)point;
- (NSRect) rectInMatrixSpace: (NSRect)rect;

@end

/** Informal protocol to which backends can conform to when they support drawing a 
graphics state with arbitrary transforms on the current graphics context. */
@interface NSObject (GSDrawGState)
- (void) drawGState: (GSGState *)source 
           fromRect: (NSRect)aRect 
            toPoint: (NSPoint)aPoint 
                 op: (NSCompositingOperation)op
           fraction: (CGFloat)delta;
@end

#include "GSGStateOps.h"

#endif /* _GSGState_h_INCLUDE */

