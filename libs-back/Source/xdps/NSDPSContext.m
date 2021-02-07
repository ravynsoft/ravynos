/* 
   NSDPSContext.m

   Encapsulation of Display Postscript contexts

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
   This file is part of the GNUstep GUI Library.

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
#include <math.h>
#include <Foundation/NSString.h>
#include <Foundation/NSThread.h>
#include <Foundation/NSLock.h>
#include <Foundation/NSData.h>
#include <Foundation/NSDictionary.h>

#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSWindow.h>
#include <GNUstepGUI/GSFontInfo.h>

#include "x11/XGServer.h"
#include "xdps/NSDPSContext.h"

#define BOOL XWINDOWSBOOL
#include <DPS/dpsXclient.h>
#include <DPS/dpsXshare.h>
#undef BOOL

#ifdef HAVE_WRASTER_H
#include "wraster.h"
#else
#include "x11/wraster.h"
#endif

#include "general.h"
#include "extensions.h"
#include "drawingfuncs.h"
#include "AFMFileFontInfo.h"

//
// DPS exceptions
//
NSString *DPSPostscriptErrorException = @"DPSPostscriptErrorException";
NSString *DPSNameTooLongException = @"DPSNameTooLongException";
NSString *DPSResultTagCheckException = @"DPSResultTagCheckException";
NSString *DPSResultTypeCheckException = @"DPSResultTypeCheckException";
NSString *DPSInvalidContextException = @"DPSInvalidContextException";
NSString *DPSSelectException = @"DPSSelectException";
NSString *DPSConnectionClosedException = @"DPSConnectionClosedException";
NSString *DPSReadException = @"DPSReadException";
NSString *DPSWriteException = @"DPSWriteException";
NSString *DPSInvalidFDException = @"DPSInvalidFDException";
NSString *DPSInvalidTEException = @"DPSInvalidTEException";
NSString *DPSInvalidPortException = @"DPSInvalidPortException";
NSString *DPSOutOfMemoryException = @"DPSOutOfMemoryException";
NSString *DPSCantConnectException = @"DPSCantConnectException";

#define XDPY (((RContext *)context)->dpy)
#define XDRW (((RContext *)context)->drawable)

enum {
  A_COEFF = 0,
  B_COEFF,
  C_COEFF,
  D_COEFF,
  TX_CONS,
  TY_CONS
};

//
// Class variables
//
static BOOL GNU_CONTEXT_TRACED = NO;
static BOOL GNU_CONTEXT_SYNCHRONIZED = NO;

static NSDPSContext *context_list = nil;
static FILE *gstream = NULL;

/* Text handler for text contexts */
static void
GNUstepOutputTextProc (DPSContext ctxt, char *buf, long unsigned int count)
{
  /* FIXME: If there's a possibility of having more than one text context
     we should have some simple hash to determine what stream to write to */
  if (gstream)
    fwrite(buf, 1, count, gstream);
  else
    DPSDefaultTextBackstop (ctxt, buf, count);
}

/* Text handler for screen contexts */
static void
GNUstepTextProc (DPSContext ctxt, char *buf, long unsigned int count)
{
  DPSDefaultTextBackstop (ctxt, buf, count);
}

/* Error handler for all types of contexts */
static void
GNUstepErrorProc (DPSContext ctxt, DPSErrorCode errCode, 
		  unsigned arg1, unsigned args)
{
  DPSDefaultErrorProc (ctxt, errCode, arg1, arg1);
}

@interface NSDPSContext (Private)
- (void) createDPSContext;
- (void) createTextContext;
@end

@implementation NSDPSContext

+ (void)initialize
{
  if (self == [NSDPSContext class])
    {
      // Set initial version
      [self setVersion: 1];

     GNU_CONTEXT_TRACED = NO;
     GNU_CONTEXT_SYNCHRONIZED = NO;
    }
}

/* Initialize AppKit backend */
+ (void)initializeBackend
{
  NSDebugLog(@"Initializing GNUstep GUI X/DPS Backend.\n");
  [NSGraphicsContext setDefaultContextClass: [NSDPSContext class]];
  [GSFontEnumerator setDefaultClass: [PXKFontEnumerator class]];
  [GSFontInfo setDefaultClass: [AFMFileFontInfo class]];
}

+ (void) setCurrentContext: (NSGraphicsContext *)aContext
{
  [super setCurrentContext: aContext];
  DPSSetContext([(NSDPSContext *)aContext xDPSContext]);
}

//
// Initializing a Context
//
- init
{
  return [self initWithContextInfo: nil];
}

- (id) initWithContextInfo: (NSDictionary *)info
{
  NSString *contextType;

  [super initWithContextInfo: info];

  /* A context is only associated with one server. Do not retain
     the server, however */
  server = GSCurrentServer();

  chained_parent = nil;
  chained_child = nil;
  contextType = [info objectForKey: 
		  NSGraphicsContextRepresentationFormatAttributeName];
  if (contextType && [contextType isEqual: NSGraphicsContextPSFormat])
    {
      NSString *path;
      is_screen_context = NO;
      error_proc = GNUstepErrorProc;
      text_proc = GNUstepOutputTextProc;
      path = [info objectForKey: @"NSOutputFile"];
      if (path == nil)
	{
	  NSLog(@"Warning: No path set for stream context, default temp.ps");
	  path = @"temp.ps";
	}
      gstream = fopen([path cString], "w");
      if (gstream == NULL)
	{
	  NSLog(@"Error: Could not open output path for stream context");
	}
    }
  else
    {
      is_screen_context = YES;
      error_proc = GNUstepErrorProc;
      text_proc = GNUstepTextProc;
    }

  /*
   * Create the context
   */
  if (is_screen_context)
    {
      [self createDPSContext];
    }
  else
    {
      [self createTextContext];
    }

  if (dps_context == NULL)
    {
      [self dealloc];
      return nil;
    }

  /* Add context to global list of contexts */
  next_context = context_list;
  context_list = self;

  if (GSDebugSet(@"NSDPSContext") == YES)
    {
      NSLog(@"NSDPSContext: Tracing Postscript \n");
      [self setOutputTraced: YES];
    }

  return self;
}

- (void)dealloc
{
  DESTROY(chained_child);

  DPSDestroySpace(DPSSpaceFromContext(dps_context));
  if (is_screen_context == 0)
    {
      if (gstream)
	fclose(gstream);
    }
  /* Remove context from global list of contexts */
  {
    NSDPSContext *ctxt = context_list, *previous=nil;
    
    while (ctxt) 
      {
	if (ctxt == self)
	  break;
	previous = ctxt;
	ctxt = ctxt->next_context;
      }
    if (!ctxt)
      NSLog(@"Internal Error: Couldn't find context to delete");
    else 
      {
	if (previous)
	  previous->next_context = next_context;
	else
	  context_list = next_context;
      }
  }
  [super dealloc];
}

//
// Testing the Drawing Destination
//
- (BOOL)isDrawingToScreen
{
  return is_screen_context;
}

- (NSDPSContext *)DPSContext
{
  return self;
}

- (void)wait
{
  DPSWaitContext (dps_context);
}

+ (void) waitAllContexts
{
  NSDPSContext *ctxt;
  ctxt = context_list;
  while (ctxt) 
    {
      [ctxt wait];
      ctxt = ctxt->next_context;
    }
}

//
// Managing Returned Text and Errors
//
+ (NSString *)stringForDPSError:(const DPSBinObjSeqRec *)error
{
  return nil;
}

- (DPSErrorProc)errorProc
{
  return error_proc;
}

- (void)setErrorProc:(DPSErrorProc)proc
{
  error_proc = proc;
}

- (void)setTextProc:(DPSTextProc)proc
{
  text_proc = proc;
}

- (DPSTextProc)textProc
{
  return text_proc;
}

//
// Managing Chained Contexts
//
- (void)setParentContext:(NSDPSContext *)parent
{
  chained_parent = parent;
}

- (void)chainChildContext:(NSDPSContext *)child
{
  if (child)
    {
      chained_child = [child retain];
      [child setParentContext: self];
    }
}

- (NSDPSContext *)childContext
{
  return chained_child;
}

- (NSDPSContext *)parentContext
{
  return chained_parent;
}

- (void)unchainContext
{
  if (chained_child)
    {
      [chained_child setParentContext: nil];
      [chained_child release];
      chained_child = nil;
    }
}

//
// Debugging Aids
//
+ (BOOL)areAllContextsOutputTraced
{
  return GNU_CONTEXT_TRACED;
}

+ (BOOL)areAllContextsSynchronized
{
  return GNU_CONTEXT_SYNCHRONIZED;
}

+ (void)setAllContextsOutputTraced:(BOOL)flag
{
  GNU_CONTEXT_TRACED = flag;
}

+ (void)setAllContextsSynchronized:(BOOL)flag
{
  GNU_CONTEXT_SYNCHRONIZED = flag;
}

- (BOOL)isOutputTraced
{
  return is_output_traced;
}

- (BOOL)isSynchronized
{
  return is_synchronized;
}

- (void)setOutputTraced:(BOOL)flag
{
  is_output_traced = flag;
  XDPSChainTextContext(dps_context, flag);
}

- (void)setSynchronized:(BOOL)flag
{
  is_synchronized = flag;
}

@end

//
// Methods for XWindows implementation
//
@implementation NSDPSContext (GNUstepXDPS)

- (Display*)xDisplay
{
  if (is_screen_context)
    return [(XGServer *)server xDisplay];
  else
    return NULL;
}

- (DPSContext)xDPSContext
{
  return dps_context;
}

- (void)createDPSContext
{
  int x, y, supported;
  unsigned long valuemask;
  XGCValues values;

  // Where should the screen number come from?
  context = [(XGServer *)server xrContextForScreen: 0];
  if (!XDPSExtensionPresent(XDPY)) 
    {
#if HAVE_DPS_DPSNXARGS_H    
      /* Make it possible for this client to start a DPS NX agent */
      XDPSNXSetClientArg(XDPSNX_AUTO_LAUNCH, (void *)True);
#else
      NSLog (@"DPS extension not in server!");
      exit (1);
#endif
    }

  /* Create a GC for the initial window */
  values.foreground = ((RContext *)context)->black;
  values.background = ((RContext *)context)->white;
  values.function = GXcopy;
  values.plane_mask = AllPlanes;
  values.clip_mask = None;
  valuemask = (GCFunction | GCPlaneMask | GCClipMask 
         | GCForeground|GCBackground);
  ((RContext *)context)->copy_gc = 
    XCreateGC(XDPY, XDRW, valuemask, &values);

  /* Create the context if need be */
  if (!dps_context)
    {
      /* Pass None as the drawable argument; the program will execute correctly
         but will not render any text or graphics. */
      dps_context = XDPSCreateSimpleContext(XDPY, None, 
					    ((RContext *)context)->copy_gc, 
					    0, 0,
					    text_proc,
					    error_proc,
					    NULL);
      if (dps_context == NULL)
	{
	  NSLog(@"Could not connect to DPS\n");
	  NSLog(@"Trying again...\n");
       	  dps_context = XDPSCreateSimpleContext(XDPY, None, 
					   ((RContext *)context)->copy_gc,
					   0, 0,
					   text_proc,
					   error_proc,
					   NULL);

	  if (dps_context == NULL)
	    {
	      NSLog(@"DPS is not available\n");
	      exit(1);
	    }
	}

      // Make it the active context
      DPSSetContext(dps_context);
      XDPSRegisterContext(dps_context, NO);

      // Use pass-through event handling
      XDPSSetEventDelivery(XDPY, dps_event_pass_through);
    }

  PSWinitcontext (XGContextFromGC (((RContext *)context)->copy_gc), 
		  XDRW, 0, 0);
  PSWGetTransform (dps_context, ctm, invctm, &x, &y);
  PSWinitcontext (XGContextFromGC (((RContext *)context)->copy_gc), 
  		  None, 0, 0);
  PSWRevision(&dps_revision);

  /* Check for operator extensions */
  DPSWKnownExtensions(dps_context, &ext_flags);
  /* Check if composite-related extensions work */
  /* FIXME: This crashes DPS on some implementations */
  //DPSWWorkingExtensions(dps_context, &supported);
  supported = 0;
  if (supported == 0)
    ext_flags = (ext_flags 
		 & ~(COMPOSITE_EXT | ALPHAIMAGE_EXT | COMPOSITERECT_EXT
		     | DISSOLVE_EXT | READIMAGE_EXT | SETALPHA_EXT));
  /* FIXME: alphaimage and composite work badly in DGS 5.50. Perhaps when 
     we find a version that works we can put an additional test here. For now,
     just turn them off.
  */
  ext_flags = (ext_flags & ~(COMPOSITE_EXT | ALPHAIMAGE_EXT | DISSOLVE_EXT));

  NSDebugLLog(@"NSDPSContext", @"Using DPS Revision: %d\n", dps_revision);
  NSDebugLLog(@"NSDPSContext", @"DPS Default Matrix: [%f %f %f %f %f %f]\n", 
	ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);
  NSDebugLLog(@"NSDPSContext", @"DPS Extensions flags: %d\n", ext_flags); 
  if ([[NSUserDefaults standardUserDefaults] boolForKey: @"DPSDefaultMatrix"]
      == NO)
    {
      NSDebugLog(@"Reseting default matrix\n");
      ctm[0] /= fabs(ctm[0]);
      ctm[3] /= fabs(ctm[3]);
      PSWSetMatrix(ctm);
    }
}

- (void)createTextContext
{
  if (dps_context)
    return;

  dps_context = DPSCreateTextContext(text_proc, error_proc);
  if (dps_context == NULL)
    {
      NSLog(@"Could not create DPS text context");
      return;
    }
}

- (void) flushGraphics
{
  DPSFlushContext(dps_context);
  XFlush([(XGServer *)server xDisplay]);
}

- (void) _localTransform: (float *)local_ctm inverse: (float *)local_inv
           offset: (NSPoint *)offset
{
  int x, y;
  if (local_ctm == NULL || local_inv == NULL)
    return;
  PSWGetTransform (dps_context, local_ctm, local_inv, &x, &y);
  if (offset)
    {
      offset->x = x;
      offset->y = y;
    }
}

- (NSPoint)userPointFromXPoint:(NSPoint)xPoint
{
  float lctm[6], linv[6];
  NSPoint offset, userPoint;

  [self _localTransform: lctm inverse: linv offset: &offset];
  //xPoint.x -= offset.x;
  //xPoint.y -= offset.y;
  userPoint.x = linv[A_COEFF] * xPoint.x + linv[C_COEFF] * xPoint.y
  		+ linv[TX_CONS];
  userPoint.y = linv[B_COEFF] * xPoint.x + linv[D_COEFF] * xPoint.y
                + linv[TY_CONS];
  return userPoint;
}

- (NSPoint)XPointFromUserPoint:(NSPoint)userPoint
{
  float lctm[6], linv[6];
  NSPoint offset, xPoint;

  [self _localTransform: lctm inverse: linv offset: &offset];
  xPoint.x = lctm[A_COEFF] * userPoint.x + lctm[C_COEFF] * userPoint.y
  		+  lctm[TX_CONS] + offset.x;
  xPoint.y = lctm[B_COEFF] * userPoint.x + lctm[D_COEFF] * userPoint.y
                +  lctm[TY_CONS] + offset.y;
  xPoint.x = floor (xPoint.x);
  xPoint.y = floor (xPoint.y);
  NSDebugLLog(@"CTM", @"Matrix [%f,%f,%f,%f,%f,%f] (%f,%f)\n",
  	lctm[A_COEFF], lctm[B_COEFF], lctm[C_COEFF], lctm[D_COEFF], 
	lctm[TX_CONS], lctm[TY_CONS], offset.x, offset.y);
  return xPoint;
}

- (NSRect)userRectFromXRect:(NSRect)xrect
{
  float lctm[6], linv[6];
  float x, y, w, h;

  [self _localTransform: lctm inverse: linv offset: NULL];
  x = linv[A_COEFF] * NSMinX(xrect) + linv[C_COEFF] * NSMinY(xrect)
  		+ linv[TX_CONS];
  y = linv[B_COEFF] * NSMinX(xrect) + linv[D_COEFF] * NSMinY(xrect)
                + linv[TY_CONS];
  w = linv[A_COEFF] * NSWidth(xrect) + linv[C_COEFF] * NSHeight(xrect);
  h = linv[B_COEFF] * NSWidth(xrect) + linv[D_COEFF] * NSHeight(xrect);
  if (h < 0)
    y -= h;
  h = fabs(h);
  if (w < 0)
    x -= w;
  w = fabs(w);
  return NSMakeRect(x, y, w, h);
}

- (NSRect)XRectFromUserRect:(NSRect)urect
{
  NSPoint offset;
  float lctm[6], linv[6];
  float x, y, w, h;

  [self _localTransform: lctm inverse: linv offset: &offset];
  x = lctm[A_COEFF] * NSMinX(urect) + lctm[C_COEFF] * NSMinY(urect)
  		+  lctm[TX_CONS] + offset.x;
  y = lctm[B_COEFF] * NSMinX(urect) + lctm[D_COEFF] * NSMinY(urect)
                +  lctm[TY_CONS] + offset.y;
  w = lctm[A_COEFF] * NSWidth(urect) + lctm[C_COEFF] * NSHeight(urect);
  h = lctm[B_COEFF] * NSWidth(urect) + lctm[D_COEFF] * NSHeight(urect);
  NSDebugLLog(@"CTM", @"Matrix [%f,%f,%f,%f,%f,%f] (%f,%f)\n",
  	lctm[A_COEFF], lctm[B_COEFF], lctm[C_COEFF], lctm[D_COEFF], 
	lctm[TX_CONS], lctm[TY_CONS], offset.x, offset.y);
  if (h < 0)
    y += h;
  h = fabs(floor(h));
  if (w < 0)
    x += w;
  w = fabs(floor(w));
  x = floor(x);
  y = floor(y);
  return NSMakeRect(x, y, w, h);
}

- (op_extensions_t) operatorExtensions
{
  return ext_flags;
}

@end

@implementation NSDPSContext (NSGraphics)

/* Optimized drawing functions */
- (void) NSRectFillList: (const NSRect *)rects : (int) count
{
  int i;
  float rectvals[count*4];

  if (count*4 > 65536)
    {
      NSLog(@"DPS Rendering Error: RectFillList with > 16384 rects");
      return;
    }

  for (i = 0; i < count; i++)
    {
      rectvals[i*4    ] = NSMinX(rects[i]);
      rectvals[i*4 + 1] = NSMinY(rects[i]);
      rectvals[i*4 + 2] = NSWidth(rects[i]);
      rectvals[i*4 + 3] = NSHeight(rects[i]);
    }
  PSWRectFillList(rectvals, count*4);
}

- (void) NSRectFillListWithGrays: (const NSRect *)rects : (const float *)grays
				:(int) count
{
#if 1
  int i;
  float rectvals[count*5];

  if (count*5 > 65536)
    {
      NSLog(@"DPS Rendering Error: RectFillListGray with > 13107 rects");
      return;
    }

  for (i = 0; i < count; i++)
    {
      rectvals[i*5    ] = NSMinX(rects[i]);
      rectvals[i*5 + 1] = NSMinY(rects[i]);
      rectvals[i*5 + 2] = NSWidth(rects[i]);
      rectvals[i*5 + 3] = NSHeight(rects[i]);
      rectvals[i*5 + 4] = grays[i];
    }
  PSWRectFillListGray(rectvals, count*5);
#else
  int i, last_gray, tcount;

  PSsetgray(grays[0]);
  last_gray = grays[0];
  tcount = 0;
  for (i = 0; i < count; i++)
    {
      if (grays[i] != last_gray)
	{
	  NSRectFillList(&rects[i-tcount], tcount);
	  tcount = 0;
	  PSsetgray(grays[i]);
	  last_gray = grays[i];
	}
      else
	tcount++;
    }
#endif
}

- (void) NSDottedFrameRect: (const NSRect) aRect
{
  PSWDottedFrameRect (aRect.origin.x, aRect.origin.y,
		      aRect.size.width, aRect.size.height);
}

- (void) NSFrameRect: (const NSRect) aRect
{
  PSWFrameRect (aRect.origin.x, aRect.origin.y,
		  aRect.size.width, aRect.size.height);
}

- (void) NSFrameRectWithWidth: (const NSRect) aRect :  (float) frameWidth
{
  PSWFrameRectWithWidth (aRect.origin.x, aRect.origin.y,
			   aRect.size.width, aRect.size.height, frameWidth);
}

//
// Read the Color at a Screen Position
//
- (NSColor *) NSReadPixel: (NSPoint) location
{
  return nil;
}

@end
