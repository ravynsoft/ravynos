/* XGGState - Implements graphic state drawing for Xlib

   Copyright (C) 1998-2010 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Nov 1998
   
   This file is part of the GNU Objective C User Interface Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#import "config.h"
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSGraphics.h>

#import "xlib/XGGeometry.h"
#import "xlib/XGContext.h"
#import "xlib/XGGState.h"
#import "xlib/XGContext.h"
#import "xlib/XGPrivate.h"
#include "math.h"

#define XDPY (((RContext *)context)->dpy)

static BOOL shouldDrawAlpha = YES;

#define CHECK_GC \
  if (!xgcntxt) \
    [self createGraphicContext]

#define COPY_GC_ON_CHANGE \
  CHECK_GC; \
  if (sharedGC == YES) \
    [self copyGraphicContext]


u_long   
xrRGBToPixel(RContext* context, device_color_t color)
{
  XColor cc;
  RColor rcolor;
  rcolor.red = 255. * color.field[0];
  rcolor.green = 255. * color.field[1];
  rcolor.blue = 255. * color.field[2];
  rcolor.alpha = 0;
  RGetClosestXColor(context, &rcolor, &cc);
  return cc.pixel;
}

@interface XGGState (Private)
- (void) _alphaBuffer: (gswindow_device_t *)dest_win;
- (void) _paintPath: (ctxt_object_t) drawType;
- (void) createGraphicContext;
- (void) copyGraphicContext;
- (void) setAlphaColor: (float)color;
@end

@implementation XGGState

static Region emptyRegion;

+ (void) initialize
{
  static BOOL beenHere = NO;

  if (beenHere == NO)
    {
      XPoint pts[5];
      id obj = [[NSUserDefaults standardUserDefaults]
                 stringForKey: @"GraphicCompositing"];
      if (obj)
        shouldDrawAlpha = [obj boolValue];

      beenHere = YES;
      pts[0].x = 0; pts[0].y = 0;
      pts[1].x = 0; pts[1].y = 0;
      pts[2].x = 0; pts[2].y = 0;
      pts[3].x = 0; pts[3].y = 0;
      pts[4].x = 0; pts[4].y = 0;
      emptyRegion = XPolygonRegion(pts, 5, WindingRule);
      NSAssert(XEmptyRegion(emptyRegion), NSInternalInconsistencyException);
    }
}

/* Designated initializer. */
- initWithDrawContext: (GSContext *)drawContext
{
  [super initWithDrawContext: drawContext];

  drawMechanism = -1;
  draw = 0;
  alpha_buffer = 0;
  xgcntxt = None;
  agcntxt = None;
#ifdef HAVE_XFT
  xft_draw = NULL;
  xft_alpha_draw = NULL;
  memset(&xft_color, 0, sizeof(XftColor));
#endif
  return self;
}

- (void) dealloc
{
  if (sharedGC == NO && xgcntxt) 
    {
      XFreeGC(XDPY, xgcntxt);
    }
  if (agcntxt)
    XFreeGC(XDPY, agcntxt);
  if (clipregion)
    XDestroyRegion(clipregion);

#ifdef HAVE_XFT
  if (xft_draw != NULL)
    {
      XftDrawDestroy(xft_draw);
    }
  if (xft_alpha_draw != NULL)
    {
      XftDrawDestroy(xft_alpha_draw);
    }
#endif
  [super dealloc];
}

- (id) deepen
{
  [super deepen];

  // Copy the GC 
  if (draw != 0)
    [self copyGraphicContext];

  /* Force a new one to be created */
  agcntxt = None;

  // Copy the clipregion
  if (clipregion)
    {
      Region region = XCreateRegion();

      XIntersectRegion(clipregion, clipregion, region);
      self->clipregion = region;
    }

#ifdef HAVE_XFT
  xft_draw = NULL;
  xft_alpha_draw = NULL;
#endif
  return self;
}

- (void) setWindowDevice: (void *)device
{
  XGServer *srv;
  gswindow_device_t *gs_win;

  gs_win = windevice = device;
  draw = GET_XDRAWABLE(gs_win);
  [self setGraphicContext: gs_win->gc];
  alpha_buffer = 0;
  drawingAlpha = NO;

  /* We know the current server sent us this */
  srv = (XGServer *)GSCurrentServer();
  context = [srv screenRContext];
  drawMechanism = [srv screenDrawMechanism];

  if (gs_win != NULL && gs_win->alpha_buffer != 0)
    {
      alpha_buffer = gs_win->alpha_buffer;
      if (shouldDrawAlpha)
        drawingAlpha = YES;
    }
}

- (void) setDrawable: (Drawable)theDrawable;
{
  draw = theDrawable;
}

- (void) setGraphicContext: (GC)xGraphicContext
{
  GC source;
  unsigned long mask;
  BOOL old_shared;

  source = xgcntxt;
  old_shared = sharedGC;
  if (xGraphicContext == None)
    return;
  if (xGraphicContext == xgcntxt)
    return;
  
  xgcntxt = xGraphicContext;
  sharedGC = YES; /* Not sure if we really own the GC */
  /* Update the GC to reflect our settings */
  if (source == None)
    return;
  mask = GCForeground | GCFont | GCFunction | GCFillRule | 
    GCBackground | GCCapStyle | GCJoinStyle | GCLineWidth | 
    GCLineStyle | GCDashOffset | GCDashList;
  XCopyGC(XDPY, source, mask, xgcntxt); 

  if (source != None && old_shared == NO)
    XFreeGC(XDPY, source);
}

/* Set various characteristics of the graphic context */
- (void) setGCValues: (XGCValues)values withMask: (int)mask
{
  COPY_GC_ON_CHANGE;
  if (xgcntxt == 0)
    return;
  XChangeGC(XDPY, xgcntxt, mask, &values);
}

/* Set the GC clipmask.  */
- (void) setClipMask
{
  COPY_GC_ON_CHANGE;
  if (xgcntxt == 0)
    return;
  if (!clipregion)
    {
      XSetClipMask(XDPY, xgcntxt, None);
      return;
    }

  XSetRegion(XDPY, xgcntxt, clipregion);
  NSDebugLLog(@"XGGraphics", @"Clip %@ set to X rect %@",
  self, NSStringFromRect([self clipRect]));
}

/* Returns the clip region, which must be freed by the caller */
- (Region) xClipRegion
{
  Region region = XCreateRegion();

  if (clipregion)
    XIntersectRegion(clipregion, clipregion, region);
  else 
    XIntersectRegion(emptyRegion, emptyRegion, region);

  return region;
}

- (void) setColor: (device_color_t *)color state: (color_state_t)cState
{
  device_color_t c;
  [super setColor: color state: cState];
  if (context == NULL)
    {
      /* Window device isn't set yet */
      return;
    }
  c = *color;
  gsColorToRGB(&c);
  gcv.foreground = xrRGBToPixel(context, c);
  [self setGCValues: gcv withMask: GCForeground];
#ifdef HAVE_XFT
  xft_color.color.red = 65535.0 * c.field[0];
  xft_color.color.green = 65535.0 * c.field[1];
  xft_color.color.blue = 65535.0 * c.field[2];
  xft_color.color.alpha = 0xffff;
  xft_color.pixel = gcv.foreground;
#endif
}

- (void) setAlphaColor: (float)value
{
  device_color_t color;
  if (context == NULL)
    {
      /* Window device isn't set yet */
      return;
    }
  gsMakeColor(&color, rgb_colorspace, value, value, value, 0);
  gcv.foreground = xrRGBToPixel(context, color);
  if (agcntxt == None)
    agcntxt = XCreateGC(XDPY, draw, GCForeground, &gcv);
  else
    XChangeGC(XDPY, agcntxt, GCForeground, &gcv);
#ifdef HAVE_XFT
  xft_color.color.alpha = 65535.0 * value;
#endif
}

- (void) copyGraphicContext
{
  GC source;
  unsigned long mask;
  
  if (draw == 0)
    {
      DPS_ERROR(DPSinvalidid, @"Copying a GC with no Drawable defined");
      return;
    }

  source = xgcntxt;
  mask = 0xffffffff; /* Copy everything (Hopefully) */
  xgcntxt = XCreateGC(XDPY, draw, 0, NULL);
  XCopyGC(XDPY, source, mask, xgcntxt); 
  sharedGC = NO;
  return;
}

// Create a default graphics context.
- (void) createGraphicContext
{
  if (draw == 0)
    {
      /* This could happen with a defered window */
      DPS_WARN(DPSinvalidid, @"Creating a GC with no Drawable defined");
      return;
    }
  gcv.function = GXcopy;
  gcv.background = ((RContext *)context)->white;
  gcv.foreground = ((RContext *)context)->black;
  gcv.plane_mask = AllPlanes;
  gcv.line_style = LineSolid;
  gcv.fill_style = FillSolid;
  gcv.fill_rule  = WindingRule;
  xgcntxt = XCreateGC(XDPY, draw,
                      GCFunction | GCForeground | GCBackground | GCPlaneMask 
                      | GCFillStyle | GCFillRule| GCLineStyle,
                      &gcv);
  [self setClipMask];
  sharedGC = NO;
  return;
}

- (NSRect)clipRect
{
  XRectangle r;
  r.width = 0; r.height = 0;
  if (clipregion)
    XClipBox(clipregion, &r);
  return NSMakeRect(r.x, r.y, r.width-1, r.height-1);
}

- (BOOL) hasGraphicContext
{
  return (xgcntxt) ? YES : NO;
}

- (BOOL) hasDrawable
{
  return (draw ? YES : NO);
}

- (void *) windevice
{
  return windevice;
}

- (Drawable) drawable
{
  return draw;
}

- (GC) graphicContext
{
  return xgcntxt;
}

#ifdef HAVE_XFT
- (XftColor) xftColor
{
    return xft_color;
}

- (XftDraw *)xftDrawForDrawable: (Drawable)d
{
    if (d == 0)
        return 0;  //PENDING: warn? throw?

    if (d == draw)
      {
        if (xft_draw == NULL)
            xft_draw =
                XftDrawCreate(XDPY, d, DefaultVisual(XDPY, DefaultScreen(XDPY)),
                              DefaultColormap(XDPY, DefaultScreen(XDPY)));
        if (clipregion != None)
            XftDrawSetClip(xft_draw, clipregion);
        return xft_draw;
      }
    else if (d == alpha_buffer)
      {
        if (xft_alpha_draw == NULL)
            xft_alpha_draw =
                XftDrawCreate(XDPY, d, DefaultVisual(XDPY, DefaultScreen(XDPY)),
                              DefaultColormap(XDPY, DefaultScreen(XDPY)));
        if (clipregion != None)
            XftDrawSetClip(xft_alpha_draw, clipregion);
        return xft_alpha_draw;
      }

    return 0;  //PENDING: warn? throw?
}
#endif

- (void) copyBits: (XGGState*)source fromRect: (NSRect)aRect 
                                      toPoint: (NSPoint)aPoint
{
  NSAffineTransformStruct sctms;
  NSAffineTransformStruct ctms;
  XRectangle dst;
  XRectangle src;
  NSRect flushRect;
  Drawable from;

  CHECK_GC;
  if (draw == 0)
    {
      DPS_WARN(DPSinvalidid, @"No Drawable defined for copyBits");
      return;
    }
  from = source->draw;
  if (from == 0)
    {
      DPS_ERROR(DPSinvalidid, @"No source Drawable defined for copyBits");
      return;
    }

  src = XGViewRectToX(source, aRect);
  flushRect.size = aRect.size;
  flushRect.origin = aPoint;
  dst = XGViewRectToX(self, flushRect);
  sctms = [source->ctm transformStruct];
  ctms = [ctm transformStruct];
  if (sctms.m22 < 0 && ctms.m22 > 0) dst.y += src.height;
  if (sctms.m22 > 0 && ctms.m22 < 0) dst.y -= src.height;
  NSDebugLLog(@"XGGraphics", @"Copy area from %@ to %@",
    NSStringFromRect(aRect), NSStringFromPoint(aPoint));
  XCopyArea(XDPY, from, draw, xgcntxt,
    src.x, src.y, src.width, src.height, dst.x, dst.y);
}

- (void) _alphaBuffer: (gswindow_device_t *)dest_win
{
  if (dest_win->alpha_buffer == 0 
      && dest_win->type != NSBackingStoreNonretained)    
    {    
      dest_win->alpha_buffer = XCreatePixmap(XDPY, draw, 
                                             NSWidth(dest_win->xframe),
                                             NSHeight(dest_win->xframe), 
                                             dest_win->depth);
      
     /* Fill alpha also (opaque by default) */
      [self setAlphaColor: 1.0];
      XFillRectangle(XDPY, dest_win->alpha_buffer, agcntxt, 0, 0,
                     NSWidth(dest_win->xframe), NSHeight(dest_win->xframe));
    }
  if (shouldDrawAlpha && dest_win->alpha_buffer != 0)
    {
      alpha_buffer = dest_win->alpha_buffer;
      drawingAlpha = YES;
    }
}

- (void) _compositeGState: (XGGState *) source 
                 fromRect: (NSRect) fromRect
                  toPoint: (NSPoint) toPoint
                       op: (NSCompositingOperation) op
                 fraction: (CGFloat)delta
{
  XRectangle srect;    
  XRectangle drect;    

  XPoint toXPoint;  
  
  RXImage *source_im;
  RXImage *source_alpha;

  RXImage *dest_im;
  RXImage *dest_alpha;

  gswindow_device_t *source_win;
  gswindow_device_t *dest_win;

  
  // --- get source information --------------------------------------------------
  NSDebugLLog(@"XGGraphics", @"Composite from %@ to %@",
              NSStringFromRect(fromRect), NSStringFromPoint(toPoint));

  if (!source)
    source = self;
  
  source_win = (gswindow_device_t *)source->windevice;
  if (!source_win)
    {
      DPS_ERROR(DPSinvalidid, @"Invalid composite source gstate");
      return;
    }

  if (source_win->buffer == 0 && source_win->map_state != IsViewable)
    {
      /* Can't get pixel information from a window that isn't mapped */
      DPS_ERROR(DPSinvalidaccess, @"Invalid gstate buffer");
      return;
    }


  // --- get destination information ----------------------------------------------

  dest_win = (gswindow_device_t *)windevice;
  if (!dest_win)
    {
      DPS_ERROR(DPSinvalidid, @"Invalid composite gstate");
      return;
    }

  if (dest_win->buffer == 0 && dest_win->map_state != IsViewable)
    {
      /* Why bother drawing? */
      return;
    }

     
  // --- determine region to draw --------------------------------------

  srect = XGViewRectToX(source, fromRect);
  toXPoint = XGViewPointToX(self, toPoint);
  drect.x = toXPoint.x; 
  drect.y = toXPoint.y - srect.height; 
  drect.width = srect.width;
  drect.height = srect.height;

  clipXRectsForCopying(source_win, &srect, dest_win, &drect);
  if (XGIsEmptyRect(drect))
      return;

  // --- get destination XImage ----------------------------------------
  
  if (draw == dest_win->ident && dest_win->visibility < 0)
    {
      /* Non-backingstore window isn't visible, so just make up the image */
      dest_im = RCreateXImage(context, dest_win->depth, 
                              XGWidth(drect), XGHeight(drect));
    }
  else
    {
      dest_im = RGetXImage(context, draw, XGMinX(drect), XGMinY (drect), 
                           XGWidth (drect), XGHeight (drect));
    }

  if (dest_im->image == 0)
    {//FIXME: Should not happen, 
      DPS_ERROR (DPSinvalidaccess, @"unable to fetch destination image");
      return;
    }
  

  // --- get source XImage ---------------------------------------------
    
  source_im = RGetXImage ((RContext *)context, 
                          GET_XDRAWABLE (source_win),
                          XGMinX(srect), XGMinY(srect), 
                          XGWidth(srect), XGHeight(srect));
  
  // --- create alpha XImage -------------------------------------------
  /* Force creation of our alpha buffer */
  [self _alphaBuffer: dest_win];

  /* Composite it */
  source_alpha = RGetXImage((RContext *)context, source_win->alpha_buffer, 
                      XGMinX(srect), XGMinY(srect), 
                      XGWidth(srect), XGHeight(srect));

  if (alpha_buffer)
    {
      dest_alpha = RGetXImage((RContext *)context, alpha_buffer,
                              XGMinX(drect), XGMinY(drect), 
                              XGWidth(drect), XGHeight(drect));
    }
  else
    {
      dest_alpha = NULL;
    }

  // --- THE REAL WORK IS DONE HERE! -----------------------------------
  
  {
    XRectangle xdrect = { 0, 0, XGWidth (drect), XGHeight (drect) };
      
    _pixmap_combine_alpha((RContext *)context, source_im, source_alpha, 
                          dest_im, dest_alpha, xdrect,
                          op, drawMechanism, delta);
  }
  

  // --- put result back in the drawable -------------------------------

  RPutXImage((RContext *)context, draw, xgcntxt, dest_im, 0, 0, 
             XGMinX(drect), XGMinY(drect), XGWidth(drect), XGHeight(drect));
  
  if (dest_alpha)
    {
      RPutXImage((RContext *)context, dest_win->alpha_buffer, 
                 xgcntxt, dest_alpha, 0, 0, 
                 XGMinX(drect), XGMinY(drect), 
                 XGWidth(drect), XGHeight(drect));
      RDestroyXImage((RContext *)context, dest_alpha);
    }

  // --- clean up ------------------------------------------------------
  
  RDestroyXImage((RContext *)context, dest_im);
  RDestroyXImage((RContext *)context, source_im);
  if (source_alpha)
    RDestroyXImage((RContext *)context, source_alpha);
}

- (void) compositeGState: (GSGState *)source 
                fromRect: (NSRect)aRect
                 toPoint: (NSPoint)aPoint
                      op: (NSCompositingOperation)op
                fraction: (CGFloat)delta
{
  BOOL do_copy, source_alpha;
  XGCValues comp_gcv;

  if (!source)
    source = self;

  /* If we have no drawable, we can't proceed. */
  if (draw == 0)
    {
      DPS_WARN(DPSinvalidid, @"No Drawable defined for composite");
      return;
    }

  /* Check alpha */
#define CHECK_ALPHA                                                \
  do {                                                                \
    gswindow_device_t *source_win;                                \
    source_win = (gswindow_device_t *)[(XGGState *)source windevice];        \
    source_alpha = (source_win && source_win->alpha_buffer);        \
  } while (0)

  do_copy = NO;
  switch (op)
    {
    case NSCompositeClear:
      do_copy = YES;
      comp_gcv.function = GXclear;
      break;
    case NSCompositeCopy:
      do_copy = YES;
      comp_gcv.function = GXcopy;
      break;
    case NSCompositeSourceOver:
    case NSCompositeHighlight:
      CHECK_ALPHA;
      if (source_alpha == NO)
        do_copy = YES;
      else
        do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    case NSCompositeSourceIn:
      CHECK_ALPHA;
      if (source_alpha == NO && drawingAlpha == NO)
        do_copy = YES;
      else
        do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    case NSCompositeSourceOut:
      do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    case NSCompositeSourceAtop:
      CHECK_ALPHA;
      if (source_alpha == NO && drawingAlpha == NO)
        do_copy = YES;
      else
        do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    case NSCompositeDestinationOver:
      CHECK_ALPHA;
      if (drawingAlpha == NO)
        return;
      else
        do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    case NSCompositeDestinationIn:
      CHECK_ALPHA;
      if (source_alpha == NO && drawingAlpha == NO)
        return;
      else
        do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    case NSCompositeDestinationOut:
      do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    case NSCompositeDestinationAtop:
      CHECK_ALPHA;
      if (source_alpha == NO && drawingAlpha == NO)
        return;
      else
        do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    case NSCompositeXOR:
      do_copy = NO;
      comp_gcv.function = GXxor;
      break;
    case NSCompositePlusDarker:
      do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    case GSCompositeHighlight:
      do_copy = NO;
      comp_gcv.function = GXxor;
      break;
    case NSCompositePlusLighter:
      do_copy = NO;
      comp_gcv.function = GXcopy;
      break;
    default:
      comp_gcv.function = GXcopy;
    }

  if (comp_gcv.function != GXcopy)
    [self setGCValues: comp_gcv withMask: GCFunction];

  if (shouldDrawAlpha == NO)
    do_copy = YES;

  if (do_copy)
    {
      [self copyBits: (XGGState *)source fromRect: aRect toPoint: aPoint];
    }
  else
    {
      [self _compositeGState: (XGGState *)source 
            fromRect: aRect
            toPoint: aPoint
            op: op
            fraction: delta];
    }
  

  if (comp_gcv.function != GXcopy)
    {
      comp_gcv.function = GXcopy;
      [self setGCValues: comp_gcv withMask: GCFunction];
    }
}

- (void) compositerect: (NSRect)aRect
                    op: (NSCompositingOperation)op
{
  CGFloat gray;

  [self DPScurrentgray: &gray];
  if (fabs(gray - 0.667) < 0.005)
    [self DPSsetgray: 0.333];
  else    
    [self DPSsetrgbcolor: 0.121 : 0.121 : 0];

  /* FIXME: Really need alpha dithering to do this right - combine with
     XGBitmapImageRep code? */
  switch (op)
    {
    case NSCompositeClear:
      gcv.function = GXclear;
      break;
    case NSCompositeCopy:
      gcv.function = GXcopy;
      break;
    case NSCompositeSourceOver:
    case NSCompositeHighlight:
      gcv.function = GXcopy;
      break;
    case NSCompositeSourceIn:
      gcv.function = GXcopy;
      break;
    case NSCompositeSourceOut:
      gcv.function = GXcopy;
      break;
    case NSCompositeSourceAtop:
      gcv.function = GXcopy;
      break;
    case NSCompositeDestinationOver:
      gcv.function = GXcopy;
      break;
    case NSCompositeDestinationIn:
      gcv.function = GXcopy;
      break;
    case NSCompositeDestinationOut:
      gcv.function = GXcopy;
      break;
    case NSCompositeDestinationAtop:
      gcv.function = GXcopy;
      break;
    case NSCompositeXOR:
      gcv.function = GXcopy;
      break;
    case NSCompositePlusDarker:
      gcv.function = GXcopy;
      break;
    case GSCompositeHighlight:
      gcv.function = GXxor;
      break;
    case NSCompositePlusLighter:
      gcv.function = GXcopy;
      break;
    default:
      gcv.function = GXcopy;
      break;
    }
  [self setGCValues: gcv withMask: GCFunction];
  [self DPSrectfill: NSMinX(aRect) : NSMinY(aRect) 
        : NSWidth(aRect) : NSHeight(aRect)];

  if (gcv.function != GXcopy)
    {
      gcv.function = GXcopy;
      [self setGCValues: gcv withMask: GCFunction];
    }
  [self DPSsetgray: gray];
}

/* Paint the current path using Xlib calls. All coordinates should already
   have been transformed to device coordinates. */
- (void) _doPath: (XPoint*)pts : (int)count draw: (ctxt_object_t)type
{
  int fill_rule;

  COPY_GC_ON_CHANGE;
  if (draw == 0)
    {
      DPS_WARN(DPSinvalidid, @"No Drawable defined for path");
      return;
    }
  fill_rule = WindingRule;
  switch (type)
    {
    case path_stroke:
      // Hack: Only draw when alpha is not zero
      if (drawingAlpha == NO || strokeColor.field[AINDEX] != 0.0)
        XDrawLines(XDPY, draw, xgcntxt, pts, count, CoordModeOrigin);
      if (drawingAlpha)
        {
          NSAssert(alpha_buffer, NSInternalInconsistencyException);
          
          [self setAlphaColor: strokeColor.field[AINDEX]];
          XDrawLines(XDPY, alpha_buffer, agcntxt, pts, count, CoordModeOrigin);
        }
      break;
    case path_eofill:
      fill_rule = EvenOddRule;
      /* NO BREAK */
    case path_fill:
      gcv.fill_rule = fill_rule;
      [self setGCValues: gcv withMask: GCFillRule];
      // Hack: Only draw when alpha is not zero
      if (drawingAlpha == NO || fillColor.field[AINDEX] != 0.0)
        XFillPolygon(XDPY, draw, xgcntxt, pts, count, Complex, 
                     CoordModeOrigin);
      if (drawingAlpha)
        {
          NSAssert(alpha_buffer, NSInternalInconsistencyException);
          
          [self setAlphaColor: fillColor.field[AINDEX]];
          XFillPolygon(XDPY, alpha_buffer, agcntxt, pts, count, Complex, 
                       CoordModeOrigin);
        }

      break;
    case path_eoclip:
      fill_rule = EvenOddRule;
      /* NO BREAK */
    case path_clip:
      {
        Region region, new_region;
        region = XPolygonRegion(pts, count, fill_rule);
        if (clipregion)
          {
            new_region=XCreateRegion();
            XIntersectRegion(clipregion, region, new_region);
            XDestroyRegion(region);
            XDestroyRegion(clipregion);
          } else
            new_region = region;
        clipregion = new_region;
        [self setClipMask];
      }
      break;
    default:
      break;
    }
}

/* fill a complex path. All coordinates should already have been
   transformed to device coordinates. */
- (void) _doComplexPath: (XPoint*)pts 
                       : (int*)types 
                       : (int)count
                     ll: (XPoint)ll 
                     ur: (XPoint)ur 
                   draw: (ctxt_object_t)type
{
  int x, y, i, j, cnt, nseg = 0;
  XSegment segments[count];
  Window root_rtn;
  unsigned int width, height, b_rtn, d_rtn;
  
  COPY_GC_ON_CHANGE;
  if (draw == 0)
    {
      DPS_WARN (DPSinvalidid, @"No Drawable defined for path");
      return;
    }

  XGetGeometry (XDPY, draw, &root_rtn, &x, &y, &width, &height,
                &b_rtn, &d_rtn);
  if (ur.x < x  ||  ll.x > x + (int)width)
    {
      return;
    }
  
  if (ll.y < y)
    {
      ll.y = y;
    }
  if (ur.y > y + (int)height)
    {
      ur.y = y + height;
    }
  
  /* draw horizontal lines from the bottom to the top of the path */
  for (y = ll.y; y <= ur.y; y++)
    {
      int    x[count], w[count], y0, y1;
      int    yh = y * 2 + 1;   // shift y of horizontal line
      XPoint lastP, p1;
      int wi = 0;

      // To keep compiler happy
      lastP.x = 0;
      lastP.y = 0;

      /* intersect horizontal line with path */
      for (i = 0, cnt = 0; i < count - 1; i++)
        {
          if (types[i] == 0)    // move (new subpath)
            {
              lastP = pts[i];
            }
          if (types[i+1] == 0)  // last line of subpath
            {
              if (lastP.y == pts[i].y)
                {
                  continue;
                }
              p1 = lastP;       // close subpath
            }
          else
            {
              p1 = pts[i+1];
            }
          y0 = pts[i].y * 2;
          y1 = p1.y * 2;
          if ((y0 < yh  &&  yh < y1) || (y1 < yh  &&  yh < y0))
            {
              int dy = yh - pts[i].y * 2;
              int ldy = y1 - y0;
              int ldx = (p1.x - pts[i].x) * 2;
              
              x[cnt] = pts[i].x + (ldx * dy / ldy) / 2;
              // Get winding for segment
              if (type == path_fill)
                {
                  w[cnt] = ((y0 < y1) ? -1 : 1);
                }
              cnt++;
            }
        }

      /* sort intersections */
      for (i = 0; i < cnt-1; i++)
        {
          for (j=i+1; j<cnt; j++)
            {
              if (x[j] < x[i])
                {
                  x[i] ^= x[j]; 
                  x[j] ^= x[i]; 
                  x[i] ^= x[j];
                  if (type == path_fill)
                    {
                      w[i] ^= w[j]; 
                      w[j] ^= w[i]; 
                      w[i] ^= w[j];
                    }
                 }
            }
        }

      /* draw lines between intersections */
      for (i = 0; i < cnt-1; i++)
        {
          /* sum up winding directions */
          wi += w[i];
          /* eofill -> start line on odd intersection count
           * winding fill -> start line on odd winding count
           */
          if ((type == path_eofill && !(i%2)) || (type == path_fill && wi))
            {
              segments[nseg].x1 = x[i];
              segments[nseg].x2 = x[i+1];
              segments[nseg].y1 = segments[nseg].y2 = y;
              nseg++;
            }
        }
      
      // Hack: Only draw when alpha is not zero
      if (drawingAlpha == NO || fillColor.field[AINDEX] != 0.0)
        XDrawSegments (XDPY, draw, xgcntxt, segments, nseg);
      if (drawingAlpha)
        {
          NSAssert (alpha_buffer, NSInternalInconsistencyException);
          
          [self setAlphaColor: fillColor.field[AINDEX]];
          XDrawSegments (XDPY, alpha_buffer, agcntxt, segments, nseg);
        }
      nseg = 0;
    } // for y
}

- (void) _paintPath: (ctxt_object_t) drawType
{
  unsigned count;
  NSBezierPath *flatPath;
  XPoint ll, ur;
  
  if (!path)
    {
      return;
    }
  
  ll.x = ll.y = 0x7FFF;
  ur.x = ur.y = 0;
  flatPath = [path bezierPathByFlatteningPath];
  count = [flatPath elementCount];
  if (count)
    {
      XPoint pts[count];
      int ts[count];
      unsigned j, i = 0;
      NSBezierPathElement type;
      NSPoint points[3];
      BOOL first = YES;
      NSPoint p, last_p;
      BOOL doit;
      BOOL complex = NO;
      
      // To keep compiler happy
      last_p.x = 0;
      last_p.y = 0;
      p.x = 0;
      p.y = 0;

      for (j = 0; j < count; j++) 
        {
          doit = NO;
          type = [flatPath elementAtIndex: j associatedPoints: points];
          switch(type) 
            {
            case NSMoveToBezierPathElement:
              if (drawType != path_eofill && drawType != path_fill)
                {
                  if (i > 1)
                    {
                      [self _doPath: pts : i draw: drawType];
                    }
                  i = 0;
                }
              else if (i > 1)
                {
                  complex = YES;
                }
              last_p = p = points[0];
              ts[i] = 0;
              first = NO;
              break;
            case NSLineToBezierPathElement:
              p = points[0];
              ts[i] = 1;
              if (first)
                {
                  last_p = points[0];
                  first = NO;
                }
              break;
            case NSCurveToBezierPathElement:
              // This should not happen, as we flatten the path
              p = points[2];
              ts[i] = 1;
              if (first)
                {
                  last_p = points[2];
                  first = NO;
                }
              break;
            case NSClosePathBezierPathElement:
              p = last_p;
              ts[i] = 1;
//              doit = YES;
              if (drawType != path_eofill && drawType != path_fill)
                {
                  doit = YES;
                }
              else
                {
                  complex = YES;
                }
/*
*/
              break;
            default:
              break;
            }
          pts[i] = XGWindowPointToX (self, p);
          if (pts[i].x < ll.x)
            {
              ll.x = pts[i].x;
            }
          if (pts[i].y > ur.x)
            {
              ur.x = pts[i].x;
            }
          if (pts[i].y < ll.y)
            {
              ll.y = pts[i].y;
            }
          if (pts[i].y > ur.y)
            {
              ur.y = pts[i].y;
            }
          i++;
          
          if (doit && i > 1) 
            {
              if (complex)
                {
                  [self _doComplexPath: pts  : ts  : i
                        ll: ll  ur: ur  draw: drawType];
                }
              else
                {
                  [self _doPath: pts : i draw: drawType];
                }
              i = 0;
            }
        } /* for */

      if (i > 1) 
        {
          if (complex)
            {
              [self _doComplexPath: pts  : ts  : i
                    ll: ll  ur: ur  draw: drawType];
            }
          else
            {
              [self _doPath: pts : i draw: drawType];
            }
        }
    }

  /*
   * clip does not delete the current path, so we only clear the path if the
   * operation was not a clipping operation.
   */
  if ((drawType != path_clip) && (drawType != path_eoclip))
    {
      [path removeAllPoints];
    }
}

- (XPoint) viewPointToX: (NSPoint)aPoint
{
  return XGViewPointToX(self, aPoint);
}

- (XRectangle) viewRectToX: (NSRect)aRect
{
  return XGViewRectToX(self, aRect);
}

- (XPoint) windowPointToX: (NSPoint)aPoint
{
  return XGWindowPointToX(self, aPoint);
}

- (XRectangle) windowRectToX: (NSRect)aRect
{
  return XGWindowRectToX(self, aRect);
}

@end

@implementation XGGState (Ops)

- (void) DPSsetalpha: (CGFloat)a
{
  gswindow_device_t *gs_win;
  [super DPSsetalpha: a];
  gs_win = (gswindow_device_t *)windevice;
  if (!gs_win)
    return;
  if (fillColor.field[AINDEX] < 1.0)
    [self _alphaBuffer: gs_win];
}

/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */

- (void)DPSshow: (const char *)s 
{
  int len;
  int width;
  NSSize scale;
  XPoint xp;

  if (font == nil)
    {
      NSLog(@"DPS (xgps): no font set\n");
      return;
    }

  COPY_GC_ON_CHANGE; 
  if (draw == 0)
    {
      DPS_WARN(DPSinvalidid, @"No Drawable defined for show");
      return;
    }
  
  if ((cstate & COLOR_FILL) == 0)
    [self setColor: &fillColor state: COLOR_FILL];

  len = strlen(s);
  width = [(XGFontInfo *)font widthOf: s length: len];
  xp = XGWindowPointToX(self, [path currentPoint]);
  // Hack: Only draw when alpha is not zero
  if (drawingAlpha == NO || fillColor.field[AINDEX] != 0.0)
    [(XGFontInfo *)font draw: s length: len 
               onDisplay: XDPY drawable: draw
               with: xgcntxt at: xp];

  if (drawingAlpha)
    {
      NSAssert(alpha_buffer, NSInternalInconsistencyException);

      [self setAlphaColor: fillColor.field[AINDEX]];
      [(XGFontInfo *)font draw: s length: len 
                 onDisplay: XDPY drawable: alpha_buffer
                 with: agcntxt at: xp];
    }
  /* Note we update the current point according to the current 
     transformation scaling, although the text isn't currently
     scaled (FIXME). */
  scale = [ctm transformSize: NSMakeSize(1, 1)];
  //scale = NSMakeSize(1, 1);
  [path relativeMoveToPoint: NSMakePoint(width * scale.width, 0)];
}


- (void) GSShowGlyphsWithAdvances: (const NSGlyph *)glyphs : (const NSSize *)advances : (size_t) length
{
  // FIXME: Currently advances is ignored
  int width;
  NSSize scale;
  XPoint xp;

  if (font == nil)
    {
      NSLog(@"DPS (xgps): no font set\n");
      return;
    }

  COPY_GC_ON_CHANGE;
  if (draw == 0)
    {
      DPS_WARN(DPSinvalidid, @"No Drawable defined for show");
      return;
    }
  
  if ((cstate & COLOR_FILL) == 0)
    [self setColor: &fillColor state: COLOR_FILL];

  width = [(XGFontInfo *)font widthOfGlyphs: glyphs length: length];
  xp = XGWindowPointToX(self, [path currentPoint]);
  // Hack: Only draw when alpha is not zero
  if (drawingAlpha == NO || fillColor.field[AINDEX] != 0.0)
    [(XGFontInfo *)font drawGlyphs: glyphs length: length
               onDisplay: XDPY drawable: draw
               with: xgcntxt at: xp];

  if (drawingAlpha)
    {
      NSAssert(alpha_buffer, NSInternalInconsistencyException);

      [self setAlphaColor: fillColor.field[AINDEX]];
      [(XGFontInfo *)font drawGlyphs: glyphs length: length
                 onDisplay: XDPY drawable: alpha_buffer
                 with: agcntxt at: xp];
    }
  /* Note we update the current point according to the current 
     transformation scaling, although the text isn't currently
     scaled (FIXME). */
  scale = [ctm transformSize: NSMakeSize(1, 1)];
  //scale = NSMakeSize(1, 1);
  [path relativeMoveToPoint: NSMakePoint(width * scale.width, 0)];
}

- (void) GSSetFont: (GSFontInfo *)newFont
{
  if (font == newFont)
    return;
  [super GSSetFont: newFont];

  COPY_GC_ON_CHANGE;
  if (xgcntxt == 0)
    return;

  [(XGFontInfo *)font setActiveFor: XDPY gc: xgcntxt];
}

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
- (void)DPScurrentlinecap: (int *)linecap 
{
  *linecap = gcv.cap_style - CapButt;
}

- (void)DPScurrentlinejoin: (int *)linejoin 
{
  *linejoin = gcv.join_style - JoinMiter;
}

- (void)DPScurrentlinewidth: (CGFloat *)width 
{
  *width = gcv.line_width;
}

- (void)DPSinitgraphics 
{
  [super DPSinitgraphics];
  if (clipregion)
    XDestroyRegion(clipregion);
  clipregion = 0;
}

- (void)DPSsetdash: (const CGFloat *)pat : (NSInteger)size : (CGFloat)pat_offset 
{
  int dash_offset;
  char dash_list[size];
  int i;

  if ((pat == NULL) || (size == 0))
    {
      gcv.line_style = LineSolid;
      [self setGCValues: gcv withMask: GCLineStyle];
      return;
    }

  gcv.line_style = LineOnOffDash;
  [self setGCValues: gcv withMask: GCLineStyle];

  // FIXME: How to convert those values?
  dash_offset = (int)pat_offset;
  for (i = 0; i < size; i++)
    {
      dash_list[i] = (char)pat[i];
    }

  // We can only set the dash pattern, if xgcntxt exists.
  if (xgcntxt == 0)
    return;
  XSetDashes(XDPY, xgcntxt, dash_offset, dash_list, size);
}

- (void)DPSsetlinecap: (int)linecap 
{
  gcv.cap_style = linecap + CapButt;
  [self setGCValues: gcv withMask: GCCapStyle];
}

- (void)DPSsetlinejoin: (int)linejoin 
{
  gcv.join_style = linejoin + JoinMiter;
  [self setGCValues: gcv withMask: GCJoinStyle];
}

- (void)DPSsetlinewidth: (CGFloat)width 
{
  int w;
  NSSize ws;

  ws = [ctm transformSize: NSMakeSize(width,width)];
  width = (ws.width + ws.height) / 2;

  /*
   * Evil hack to get drawing to work - with a line thickness of 1, the
   * rectangles we draw seem to lose their bottom right corners irrespective
   * of the join/cap settings - but with a thickness of zero things work.
   */
  if (width < 1.5)
    width = 0.0;

  w = (int)width;
  if (gcv.line_width != w)
    {
      gcv.line_width = w;
      [self setGCValues: gcv withMask: GCLineWidth];
    }
}

- (void) DPSsetmiterlimit: (CGFloat)limit
{
  /* Do nothing. X11 does its own thing and doesn't give us a choice */
}

/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
- (void)DPSclip 
{
  [self _paintPath: path_clip];
}

- (void)DPSeoclip 
{
  [self _paintPath: path_eoclip];
}

- (void)DPSeofill 
{
  if (pattern != nil)
    {
      [self eofillPath: path withPattern: pattern];
      return;
    }

  if ((cstate & COLOR_FILL) == 0)
    [self setColor: &fillColor state: COLOR_FILL];

  [self _paintPath: path_eofill];
}

- (void)DPSfill 
{
  if (pattern != nil)
    {
      [self fillPath: path withPattern: pattern];
      return;
    }

  if ((cstate & COLOR_FILL) == 0)
    [self setColor: &fillColor state: COLOR_FILL];

  [self _paintPath: path_fill];
}

- (void)DPSinitclip 
{
  if (clipregion)
    XDestroyRegion(clipregion);
  clipregion = 0;
  [self setClipMask];
}

- (void)DPSrectclip: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h 
{
  XRectangle xrect;
  NSRect orect;

  CHECK_GC;

  orect = NSMakeRect(x, y, w, h);
  xrect = XGViewRectToX(self, orect);

  if (clipregion == 0)
    {
      clipregion = XCreateRegion();
      XUnionRectWithRegion(&xrect, clipregion, clipregion);
    }
  else
    {
      Region region;
      region = XCreateRegion();
      XUnionRectWithRegion(&xrect, region, region);
      XIntersectRegion(clipregion, region, clipregion);
      XDestroyRegion(region);
    }
  [self setClipMask];
  [self DPSnewpath];
}

- (void)DPSrectfill: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h 
{
  XRectangle bounds;
  
  CHECK_GC;
  if (draw == 0)
    {
      DPS_WARN(DPSinvalidid, @"No Drawable defined for drawing");
      return;
    }

  if (pattern != nil)
    {
      NSBezierPath *rpath;

      rpath = [[NSBezierPath alloc] init];
      [rpath appendBezierPathWithRect: NSMakeRect(x, y, w, h)];
      [rpath transformUsingAffineTransform: ctm];
      [self fillPath: rpath withPattern: pattern];
      RELEASE(rpath);
      return;
    }

  if ((cstate & COLOR_FILL) == 0)
    [self setColor: &fillColor state: COLOR_FILL];

  bounds = XGViewRectToX(self, NSMakeRect(x, y, w, h));
NSDebugLLog(@"XGGraphics", @"Fill %@ X rect %d,%d,%d,%d",
  self, bounds.x, bounds.y, bounds.width, bounds.height);

// Hack: Only draw when alpha is not zero
  if (drawingAlpha == NO || fillColor.field[AINDEX] != 0.0)
    XFillRectangle(XDPY, draw, xgcntxt,
                   bounds.x, bounds.y, bounds.width, bounds.height);

  if (drawingAlpha)
    {
      /* Fill alpha also */
      NSAssert(alpha_buffer, NSInternalInconsistencyException);
      
      [self setAlphaColor: fillColor.field[AINDEX]];
      XFillRectangle(XDPY, alpha_buffer, agcntxt,
                 bounds.x, bounds.y, bounds.width, bounds.height);
    }
}

- (void)DPSrectstroke: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h 
{
  XRectangle bounds;
  
  CHECK_GC;
  if (draw == 0)
    {
      DPS_WARN(DPSinvalidid, @"No Drawable defined for drawing");
      return;
    }

  if ((cstate & COLOR_STROKE) == 0)
    [self setColor: &fillColor state: COLOR_STROKE];

  bounds = XGViewRectToX(self, NSMakeRect(x, y, w, h));
  // Hack: Only draw when alpha is not zero
  if (drawingAlpha == NO || strokeColor.field[AINDEX] != 0.0)
    XDrawRectangle(XDPY, draw, xgcntxt,
                   bounds.x, bounds.y, bounds.width, bounds.height);

  if (drawingAlpha)
    {
      /* Fill alpha also */
      NSAssert(alpha_buffer, NSInternalInconsistencyException);

      [self setAlphaColor: strokeColor.field[AINDEX]];
      XDrawRectangle(XDPY, alpha_buffer, agcntxt,
                 bounds.x, bounds.y, bounds.width, bounds.height);
    }
}

- (void)DPSstroke 
{
  if ((cstate & COLOR_STROKE) == 0)
    [self setColor: &fillColor state: COLOR_STROKE];

  [self _paintPath: path_stroke];
}

/* ----------------------------------------------------------------------- */
/* NSGraphics Ops */
/* ----------------------------------------------------------------------- */
- (void)DPSimage: (NSAffineTransform*) matrix : (NSInteger) pixelsWide : (NSInteger) pixelsHigh
                : (NSInteger) bitsPerSample : (NSInteger) samplesPerPixel 
                : (NSInteger) bitsPerPixel : (NSInteger) bytesPerRow : (BOOL) isPlanar
                : (BOOL) hasAlpha : (NSString *) colorSpaceName
                : (const unsigned char *const [5]) data
{
  BOOL one_is_black, fast_min;
  NSRect rect;
  XRectangle sr, dr, cr;
  RXImage *dest_im, *dest_alpha;
  gswindow_device_t *dest_win;
  int cspace;
  NSAffineTransform *old_ctm = nil;

  // FIXME for now we hard code the minification behaviour
  fast_min = YES;

  rect = NSZeroRect;
  one_is_black = NO;
  cspace = rgb_colorspace;
  rect.size.width = (CGFloat) pixelsWide;
  rect.size.height = (CGFloat) pixelsHigh;

  // default is 8 bit grayscale 
  if (!bitsPerSample)
    bitsPerSample = 8;
  if (!samplesPerPixel)
    samplesPerPixel = 1;

  // FIXME - does this work if we are passed a planar image but no hints ?
  if (!bitsPerPixel)
    bitsPerPixel = bitsPerSample * samplesPerPixel;
  if (!bytesPerRow)
    bytesPerRow = (bitsPerPixel * pixelsWide) / 8;

  /* make sure its sane - also handles row padding if hint missing */
  while ((bytesPerRow * 8) < (bitsPerPixel * pixelsWide))
    bytesPerRow++;

  /* get the colour space */
  if (colorSpaceName)
    {
      if ([colorSpaceName isEqualToString: NSDeviceRGBColorSpace] ||
          [colorSpaceName isEqualToString: NSCalibratedRGBColorSpace])
        cspace = rgb_colorspace;
      else if ([colorSpaceName isEqualToString: NSDeviceCMYKColorSpace])
        cspace = cmyk_colorspace;
      else if ([colorSpaceName isEqualToString: NSDeviceWhiteColorSpace] ||
              [colorSpaceName isEqualToString: NSCalibratedWhiteColorSpace])
        cspace = gray_colorspace;
      else if ([colorSpaceName isEqualToString: NSDeviceBlackColorSpace] ||
              [colorSpaceName isEqualToString: NSCalibratedBlackColorSpace])
        {
          cspace = gray_colorspace;
          one_is_black = YES;
        } 
      else 
        {
          // if we dont recognise the name use RGB or greyscale as appropriate
          NSLog(@"XGContext (DPSImage): Unknown colour space %@", colorSpaceName);
          if (samplesPerPixel > 2)
            cspace = rgb_colorspace;
          else
            cspace = gray_colorspace;
        }
    }

  // Apply the additional transformation
  if (matrix)
    {
      old_ctm = [ctm copy];
      [ctm prependTransform: matrix];
    }

  // --- Get our drawable info -----------------------------------------
  dest_win = (gswindow_device_t *)windevice;
  if (!dest_win)
    {
      DPS_ERROR(DPSinvalidid, @"Invalid image gstate");
      return;
    }
  

  // --- Determine screen coverage --------------------------------------
  sr = [self viewRectToX: rect];

  // --- Determine region to draw --------------------------------------
  if (clipregion)
    XClipBox (clipregion, &cr);
  else
    cr = sr;

  dr = XGIntersectionRect (sr, cr);


  // --- If there is nothing to draw return ----------------------------
  if (XGIsEmptyRect (dr))
    {
      if (old_ctm != nil)
        {
          RELEASE(ctm);
          // old_ctm is already retained
          ctm = old_ctm;
        }
      return;
    }
  
  if (dest_win->buffer == 0 && dest_win->map_state != IsViewable)
    {
      if (old_ctm != nil)
        {
          RELEASE(ctm);
          // old_ctm is already retained
          ctm = old_ctm;
        }
      return;
    }


  // --- Get the destination images ------------------------------------
  dest_im = RGetXImage ((RContext *)context, draw, XGMinX (dr), XGMinY (dr),
                        XGWidth (dr), XGHeight (dr));
  
  // Force creation of our alpha buffer
  if (hasAlpha)
    {
      [self _alphaBuffer: dest_win];
    }

  // Composite it
  if (alpha_buffer != 0)
    {
      dest_alpha = RGetXImage ((RContext *)context, alpha_buffer,
                               XGMinX (dr), XGMinY (dr),
                               XGWidth (dr), XGHeight (dr));
    }
  else
    {
      dest_alpha = 0;
    }
  

  if (hasAlpha && alpha_buffer && 
      (dest_alpha == 0 || dest_alpha->image == 0))
    {
      NSLog(@"XGContext (DPSimage): Cannot create alpha image\n");
      if (old_ctm != nil)
        {
          RELEASE(ctm);
          // old_ctm is already retained
          ctm = old_ctm;
        }
      return;
    }

  // --- The real work is done HERE ------------------------------------
  _bitmap_combine_alpha((RContext *)context, (unsigned char **)data,
                        pixelsWide, pixelsHigh,
                        bitsPerSample, samplesPerPixel,
                        bitsPerPixel, bytesPerRow,
                        cspace, one_is_black,
                        isPlanar, hasAlpha, fast_min,
                        dest_im, dest_alpha, sr, dr,
                        0, drawMechanism);

  /* Draw into the window/buffer */
  RPutXImage((RContext *)context, draw, xgcntxt, dest_im, 0, 0, 
             XGMinX (dr), XGMinY (dr), XGWidth (dr), XGHeight (dr));
  if (dest_alpha)
    {
      RPutXImage((RContext *)context, dest_win->alpha_buffer, 
                 xgcntxt, dest_alpha, 0, 0,
                 XGMinX (dr), XGMinY (dr), XGWidth (dr), XGHeight (dr));

      RDestroyXImage((RContext *)context, dest_alpha);
    }
  RDestroyXImage((RContext *)context, dest_im);

  if (old_ctm != nil)
    {
      RELEASE(ctm);
      // old_ctm is already retained
      ctm = old_ctm;
    }
}

- (NSDictionary *) GSReadRect: (NSRect)rect
{
  NSSize ssize;
  XRectangle srect;    
  RXImage *source_im;
  RXImage *source_alpha;
  gswindow_device_t *source_win;
  NSMutableDictionary *dict;
  NSData *data;
  NSAffineTransform *matrix;

  source_win = (gswindow_device_t *)windevice;
  if (!source_win)
    {
      DPS_ERROR(DPSinvalidid, @"Invalid read gstate");
      return nil;
    }

  if (source_win->buffer == 0 && source_win->map_state != IsViewable)
    {
      /* Can't read anything */
      DPS_ERROR(DPSinvalidid, @"Invalid window not readable");
      return nil;
    }

  dict = [NSMutableDictionary dictionary];

  // --- determine region to read --------------------------------------

  rect.origin = [ctm transformPoint: rect.origin];
  srect = XGWindowRectToX(self, rect);
  srect = XGIntersectionRect (srect, accessibleRectForWindow (source_win));
  ssize.width = srect.width;
  ssize.height = srect.height;
  [dict setObject: [NSValue valueWithSize: ssize] forKey: @"Size"];

  [dict setObject: NSDeviceRGBColorSpace forKey: @"ColorSpace"];
  [dict setObject: [NSNumber numberWithUnsignedInt: 8] forKey: @"BitsPerSample"];
  [dict setObject: [NSNumber numberWithUnsignedInt: source_win->depth]
        forKey: @"Depth"];
  [self _alphaBuffer: source_win];
  if (alpha_buffer)
    {
      [dict setObject: [NSNumber numberWithUnsignedInt: 4] 
            forKey: @"SamplesPerPixel"];
      [dict setObject: [NSNumber numberWithUnsignedInt: 1]
            forKey: @"HasAlpha"];
    }
  else
    {
      [dict setObject: [NSNumber numberWithUnsignedInt: 3] 
            forKey: @"SamplesPerPixel"];
      [dict setObject: [NSNumber numberWithUnsignedInt: 0]
            forKey: @"HasAlpha"];
    }
  matrix = [ctm copy];
  [matrix translateXBy: -srect.x - offset.x yBy: srect.y + srect.height - offset.y];
  [dict setObject: matrix forKey: @"Matrix"];
  DESTROY(matrix);

  if (XGIsEmptyRect(srect))
    return dict;

  // --- get source XImage ----------------------------------------
  
  if (draw == source_win->ident && source_win->visibility < 0)
    {
      /* Non-backingstore window isn't visible, so we can't read it. */
      return nil;
    }
  else
    {
      source_im = RGetXImage(context, draw, XGMinX(srect), XGMinY (srect), 
                           XGWidth (srect), XGHeight (srect));
    }

  if (source_im->image == 0)
    {
      // Should not happen, 
      return nil;
    }
  
  if (alpha_buffer)
    {
      source_alpha = RGetXImage((RContext *)context, alpha_buffer,
                              XGMinX(srect), XGMinY(srect), 
                              XGWidth(srect), XGHeight(srect));
    }
  else
    {
      source_alpha = NULL;
    }

  data = _pixmap_read_alpha(context, source_im, source_alpha, srect,
                            drawMechanism);
  [dict setObject: data forKey: @"Data"];
  /* Pixmap routine always returns image in same format (FIXME?).  */

  // --- clean up ------------------------------------------------------
  
  RDestroyXImage((RContext *)context, source_im);
  RDestroyXImage((RContext *)context, source_alpha);
  return dict;
}

@end

@implementation XGGState (PatternColor)

- (void *) saveClip
{
  if (clipregion)
    {
      Region region = XCreateRegion();

      XIntersectRegion(clipregion, clipregion, region);
      return region;
    }
  return clipregion;
}

- (void) restoreClip: (void *)savedClip
{
  if (clipregion)
    {
      XDestroyRegion(clipregion);
    }
  clipregion = savedClip;
  [self setClipMask];
}

@end
