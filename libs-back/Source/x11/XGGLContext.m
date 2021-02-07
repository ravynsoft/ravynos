/* -*- mode:ObjC -*-
   XGGLContext - backend implementation of NSOpenGLContext

   Copyright (C) 1998,2002 Free Software Foundation, Inc.

   Written by:  Frederic De Jaeger
   Date: Nov 2002
   
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
#ifdef HAVE_GLX
#include <Foundation/NSDebug.h>
#include <Foundation/NSException.h>
#include <Foundation/NSDictionary.h>
#include <GNUstepGUI/GSDisplayServer.h>
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>
#include "x11/XGServerWindow.h"
#include "x11/XGOpenGL.h"
#include <X11/Xlib.h>

//FIXME
//should I store the display ?
#define MAKE_DISPLAY(dpy) Display *dpy;\
  dpy = [(XGServer *)GSCurrentServer() xDisplay];\
  NSAssert(dpy != NULL, NSInternalInconsistencyException)

@interface XGXSubWindow : NSObject
{
  @public
  Window  xwindowid;
  NSView *attached;
}

+ subwindowOnView:(NSView *)view visualinfo:(XVisualInfo *)xVisualInfo;

- (void) update;

@end

@implementation XGXSubWindow

//We assume that the current context is the same and is an XGServer
- initWithView: (NSView *)view visualinfo: (XVisualInfo *)xVisualInfo
{
  NSRect rect;
  gswindow_device_t *win_info;
  XGServer *server;
  NSWindow *window;
  int x, y, width, height;
  int mask;
  XSetWindowAttributes window_attributes;

  self = [super init];

  if (!self)
    {
      return nil;
    }

  window = [view window];
  NSAssert(window, @"Request of an X window attachment on a view that is not\
                     on a NSWindow");

  if ([view isRotatedOrScaledFromBase])
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Cannot attach an Xwindow to a view that is\
                           rotated or scaled"];
    }
  
  server = (XGServer *)GSServerForWindow(window);
  NSAssert(server != nil, NSInternalInconsistencyException);

  NSAssert([server isKindOfClass: [XGServer class]], 
           NSInternalInconsistencyException);

  win_info = [XGServer _windowWithTag: [window windowNumber]];
  NSAssert(win_info, NSInternalInconsistencyException);

  if ([server handlesWindowDecorations] == YES)
    {
      /* The window manager handles window decorations, so the
       * the parent X window is equal to the content view and
       * we must therefore use content view coordinates.
       */
      rect = [view convertRect: [view bounds]
                        toView: [window contentView]];
      if ([[window contentView] isFlipped])
        {
          rect.origin.y = NSHeight([[window contentView] frame]) - (rect.size.height + rect.origin.y);
        }
    }
  else
    {
      /* The GUI library handles window decorations, so the
       * the parent X window is equal to the NSWindow frame
       * and we can use window base coordinates.
       */
      rect = [view convertRect: [view bounds] toView: nil];
    }

  x = NSMinX(rect);
  y = NSHeight(win_info->xframe) - NSMaxY(rect);
  width = NSWidth(rect);
  height = NSHeight(rect);

  window_attributes.border_pixel = 0;
  window_attributes.background_pixel = 0;
  window_attributes.colormap = XCreateColormap(win_info->display, 
                                               win_info->ident,
                                               xVisualInfo->visual, AllocNone);

  window_attributes.event_mask = StructureNotifyMask
                               | VisibilityChangeMask
                               | ExposureMask;

  mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

  xwindowid = XCreateWindow(win_info->display, win_info->ident,
                            x, y, width, height, 0, 
                            xVisualInfo->depth, InputOutput,
                            xVisualInfo->visual, mask, &window_attributes);

  XMapWindow(win_info->display, xwindowid);

  attached = view;

  return self;
}

- (void) map
{
  MAKE_DISPLAY(dpy);
  XMapWindow(dpy, xwindowid);
}

- (void) detach
{
  //FIXME
  //I assume that the current server is correct. 
  MAKE_DISPLAY(dpy);
  attached = nil;
  XDestroyWindow(dpy, xwindowid);
}

- (void) update
{
  NSRect rect;
  gswindow_device_t *win_info;
  GSDisplayServer *server;
  NSWindow *win;
  int x, y, width, height;

  NSAssert(attached, NSInternalInconsistencyException);

  win = [attached window];
  NSAssert1(win, @"%@'s window is nil now!", attached);

  NSAssert1(![attached isRotatedOrScaledFromBase],
	    @"%@ is rotated or scaled, now!", attached);
  
  server = GSServerForWindow(win);
  NSAssert(server != nil, NSInternalInconsistencyException);

  NSAssert([server isKindOfClass: [XGServer class]], 
	   NSInternalInconsistencyException);

  //FIXME
  //we should check that the window hasn't changed, maybe.

  win_info = [XGServer _windowWithTag: [win windowNumber]];
  NSAssert(win_info, NSInternalInconsistencyException);

  if ([server handlesWindowDecorations] == YES)
    {
      /* The window manager handles window decorations, so the
       * the parent X window is equal to the content view and
       * we must therefore use content view coordinates.
       */
      rect = [attached convertRect: [attached bounds]
			    toView: [[attached window] contentView]];
      if ([[[attached window] contentView] isFlipped])
        {
          rect.origin.y = NSHeight([[[attached window] contentView] frame]) - (rect.size.height + rect.origin.y);
        }
    }
  else
    {
      /* The GUI library handles window decorations, so the
       * the parent X window is equal to the NSWindow frame
       * and we can use window base coordinates.
       */
      rect = [attached convertRect: [attached bounds] toView: nil];
    }

  x = NSMinX(rect);
  y = NSHeight(win_info->xframe) - NSMaxY(rect);
  width = NSWidth(rect);
  height = NSHeight(rect);

  
  XMoveResizeWindow(win_info->display, xwindowid,x, y, width, height);
}

- (void) dealloc
{
  NSDebugMLLog(@"GLX", @"Deallocating");

  [self detach];
  [super dealloc];
}

+ subwindowOnView:(NSView *)view visualinfo:(XVisualInfo *)xVisualInfo
{
  XGXSubWindow *win = [[self alloc] initWithView: view visualinfo: xVisualInfo];

  return AUTORELEASE(win);
}

@end

//FIXME:
//should be on per thread basis.
static XGGLContext *currentGLContext;

@implementation XGGLContext

+ (void)clearCurrentContext
{
  MAKE_DISPLAY(dpy);

  if ([XGGLPixelFormat glxMinorVersion] >= 3)
    {
      if (!glXMakeContextCurrent(dpy, None, None, NULL))
        {
          NSDebugMLLog(@"GLX", 
                       @"Cannot clear current GL context - Error %s",
                       glGetString(glGetError()));
        }
    }
  else
    {
      if (!glXMakeCurrent(dpy, None, NULL))
        {
          NSDebugMLLog(@"GLX", 
                       @"Can not clear current GL context - Error %s",
                       glGetString(glGetError()));
        }
    }

  currentGLContext = nil;
}

+ (NSOpenGLContext *)currentContext
{
  return currentGLContext;
}

- (void) _detach
{
  if (xSubWindow)
    {
      if (currentGLContext == self)
        {
          [XGGLContext clearCurrentContext];
        }

      if ( glx_drawable != xSubWindow->xwindowid )
        {
          MAKE_DISPLAY(dpy);

          glXDestroyWindow(dpy, glx_drawable);
          glx_drawable = None;
        }

      DESTROY(xSubWindow);
    }
}

- (GLXContext)glxcontext
{
  return glx_context;
}

- (void *)CGLContextObj
{
  // FIXME: Until we have a wrapper library
  // return the underlying context directly
  return (void*)glx_context;
}

- (void)clearDrawable
{
  [self _detach];
}

- (void)copyAttributesFromContext:(NSOpenGLContext *)context 
			 withMask:(unsigned long)mask
{
  GLenum error;
  MAKE_DISPLAY(dpy);

  if (context == nil || ![context isKindOfClass: [XGGLContext class]])
    {
        [NSException raise: NSInvalidArgumentException
		            format: @"%@ is an invalid context", context];
    }

  glXCopyContext(dpy, ((XGGLContext *)context)->glx_context, 
                 glx_context, mask);

  error = glGetError();
  if (error != GL_NO_ERROR)
    {
      NSDebugMLLog(@"GLX", 
                   @"Cannot copy GL context %@ from context %@ - Error %s",
                   self, context, glGetString(error));
    }
}

- (void)createTexture:(unsigned long)target 
	     fromView:(NSView*)view 
       internalFormat:(unsigned long)format
{
  [self notImplemented: _cmd];
}


- (int)currentVirtualScreen
{
  [self notImplemented: _cmd];

  return 0;
}

- (void)flushBuffer
{
  MAKE_DISPLAY(dpy);

  glXSwapBuffers(dpy, glx_drawable);
}

- (void)getValues:(long *)vals 
     forParameter:(NSOpenGLContextParameter)param
{
  //  TODO
  [self notImplemented: _cmd];
}

- (id)initWithCGLContextObj: (void *)context
{
  self = [super init];

  if (!self)
    {
      return nil;
    }

  // FIXME: Need to set the pixelFormat ivar
  glx_context = context;
  return self;
}

- (id)initWithFormat: (NSOpenGLPixelFormat *)_format 
        shareContext: (NSOpenGLContext *)share
{
  self = [super init];

  if (!self)
    {
      return nil;
    }

  glx_context = None;
  
  if (!_format || ![_format isKindOfClass: [XGGLPixelFormat class]])
    {
      NSDebugMLLog(@"GLX", @"Invalid format %@", _format);
      RELEASE(self);

      return nil;
    }

  ASSIGN(pixelFormat, (XGGLPixelFormat *)_format);
  
  glx_context = [pixelFormat createGLXContext: (XGGLContext *)share];
  
  return self;
}

- (void) dealloc
{
  NSDebugMLLog(@"GLX", @"Deallocating");

  [self _detach];
  RELEASE(pixelFormat);

  if (glx_context != None)
    {
      MAKE_DISPLAY(dpy);
      glXDestroyContext(dpy, glx_context);
    }

  [super dealloc];
}

- (void) makeCurrentContext
{
  MAKE_DISPLAY(dpy);

  if (xSubWindow == nil)
    {
      [NSException raise: NSGenericException
		          format: @"GL Context has no view attached, cannot be\
                            made current"];
    }
  
  NSAssert(glx_context != None && glx_drawable != None,
	   NSInternalInconsistencyException);

  if ([XGGLPixelFormat glxMinorVersion] >= 3)
    {
      if (!glXMakeContextCurrent(dpy, glx_drawable, glx_drawable, glx_context) )
        {
          NSDebugMLLog(@"GLX", @"Cannot make GL context %@ current - Error %s",
                       self, glGetString(glGetError()));
        }
    }
  else
    {
      if (!glXMakeCurrent(dpy, glx_drawable, glx_context) )
        {
          NSDebugMLLog(@"GLX", @"Cannot make GL context %@ current - Error %s",
                       self, glGetString(glGetError()));
        }
    }

  currentGLContext = self;
}

- (void)setCurrentVirtualScreen:(int)screen
{
  [self notImplemented: _cmd];
}

- (void)setFullScreen
{
  [self notImplemented: _cmd];
}

- (void)setOffScreen:(void *)baseaddr 
               width:(long)width 
              height:(long)height 
            rowbytes:(long)rowbytes
{
  [self notImplemented: _cmd];
}

- (void)setValues:(const long *)vals 
     forParameter:(NSOpenGLContextParameter)param
{
  [self notImplemented: _cmd];
}

- (void)setView:(NSView *)view
{
  NSView *current_view;

  if (!view)
    {
      [NSException raise: NSInvalidArgumentException
		          format: @"setView called with a nil value"];
    }

  NSAssert(pixelFormat, NSInternalInconsistencyException);
  current_view = [self view];

  if ( current_view != nil )
    {
      [current_view _setIgnoresBacking: saved_ignores_backing];
    }

  ASSIGN(xSubWindow, [XGXSubWindow subwindowOnView: view 
                                        visualinfo: [pixelFormat visualinfo]]);

  glx_drawable = [pixelFormat drawableForWindow: xSubWindow->xwindowid];
  saved_ignores_backing = [view _ignoresBacking];
  [view _setIgnoresBacking: YES];

  NSDebugMLLog(@"GLX", @"glx_window : %lu", (unsigned long) glx_drawable);
}

- (void)update
{
  [xSubWindow update];
}

- (NSView *)view
{
  if (xSubWindow)
    {
      return xSubWindow->attached;
    }
  else
    {
      return nil;
    }
}

@end
#endif
