/* 
   NSDPSContext.h

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

#ifndef _GNUstep_H_NSDPSContext
#define _GNUstep_H_NSDPSContext

/* Define this to avoid including redefinitions of ps functions introduced
   by NSGraphicsContext */
#define _PSOperators_h_INCLUDE

#define BOOL XWINDOWSBOOL	// prevent X windows BOOL
#include <X11/Xmd.h>		// warning
#undef BOOL
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <DPS/dpsclient.h>
#include <DPS/psops.h>
#include <Foundation/NSObject.h>
#include <AppKit/AppKit.h>
#include <AppKit/NSGraphicsContext.h>
#include <stdarg.h>

typedef enum {
  ALPHAIMAGE_EXT = 1, 
  COMPOSITE_EXT = 2, 
  COMPOSITERECT_EXT = 4, 
  DISSOLVE_EXT = 8, 
  READIMAGE_EXT = 16, 
  SETALPHA_EXT = 32, 
  FLUSHPAGE_EXT = 64 
} op_extensions_t;

@class NSData;
@class NSMutableData;
@class NSAffineTransform;
@class GSDisplayServer;

//
// NSDPSContext class interface
//
@interface NSDPSContext : NSGraphicsContext
{
  DPSContext dps_context;
  BOOL is_screen_context;
  DPSErrorProc error_proc;
  DPSTextProc text_proc;
  NSDPSContext *chained_parent;
  NSDPSContext *chained_child;
  BOOL is_output_traced;
  BOOL is_synchronized;
  float ctm[6], invctm[6];
  int dps_revision;
  op_extensions_t ext_flags;

  NSDPSContext *next_context;
  GSDisplayServer *server;

@public
  void          *context;
}

- (void)wait;

//
// Managing Returned Text and Errors
//
+ (NSString *)stringForDPSError:(const DPSBinObjSeqRec *)error;
- (DPSErrorProc)errorProc;
- (void)setErrorProc:(DPSErrorProc)proc;
- (void)setTextProc:(DPSTextProc)proc;
- (DPSTextProc)textProc;

//
// Managing Chained Contexts
//
- (void)chainChildContext:(NSDPSContext *)child;
- (NSDPSContext *)childContext;
- (NSDPSContext *)parentContext;
- (void)unchainContext;

//
// Debugging Aids
//
+ (BOOL)areAllContextsOutputTraced;
+ (BOOL)areAllContextsSynchronized;
+ (void)setAllContextsOutputTraced:(BOOL)flag;
+ (void)setAllContextsSynchronized:(BOOL)flag;
- (BOOL)isOutputTraced;
- (BOOL)isSynchronized;
- (void)setOutputTraced:(BOOL)flag;
- (void)setSynchronized:(BOOL)flag;

@end

@interface NSDPSContext (GNUstepXDPS)

- (Display *)xDisplay;
- (DPSContext)xDPSContext;
- (void *)xrContext;

- (NSPoint) userPointFromXPoint: (NSPoint)xPoint;
- (NSPoint) XPointFromUserPoint: (NSPoint)userPoint;
- (NSRect) userRectFromXRect: (NSRect)xrect;
- (NSRect) XRectFromUserRect: (NSRect)urect;

- (op_extensions_t) operatorExtensions;

@end

#endif /* _GNUstep_H_NSDPSContext */
