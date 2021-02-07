/* -*- mode:ObjC -*-
   Win32GLContext - backend implementation of NSOpenGLContext

   Copyright (C) 1998, 2002, 2007 Free Software Foundation, Inc.

   Written by:  Xavier Glattard
   Date: Jan 2007
   
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
#ifdef HAVE_WGL
#include <Foundation/NSDebug.h>
#include <Foundation/NSException.h>
#include <GNUstepGUI/GSDisplayServer.h>
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSOpenGLView.h>
#include "win32/WIN32Server.h"
#include "win32/WIN32OpenGL.h"

#define NSOPENGLSUBWINDOWCLASS "NSOpenGLSubwindow"
#define NSOPENGLSUBWINDOWNAME  "NSOpenGLSubwindow"

extern LRESULT CALLBACK MainWndProc(
     HWND hWnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam );

@interface Win32Subwindow : NSObject
{
@public
  HWND	 	winid;
  HDC           hDC;
  NSOpenGLView	*attached;
}
- (void) update;
+ subwindowOnView: (NSOpenGLView *) view;
@end

int
setupPixelFormat(HDC hDC, LPPIXELFORMATDESCRIPTOR ppfd );

LRESULT CALLBACK win32SubwindowProc( 
    HWND hWnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam );

@implementation Win32Subwindow 

+ (void) initialize
{
  WNDCLASS wclss;
  HINSTANCE hInstance;
  ATOM atom;

  hInstance = GetModuleHandle(NULL);

  wclss.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wclss.lpfnWndProc = (WNDPROC) win32SubwindowProc;
  wclss.cbClsExtra = 0;
  wclss.cbWndExtra = 0;
  wclss.hInstance = hInstance;
  wclss.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wclss.hCursor = LoadCursor(NULL, IDC_ARROW);
  wclss.hbrBackground = NULL;
  wclss.lpszMenuName = NULL;
  wclss.lpszClassName = NSOPENGLSUBWINDOWCLASS;

  // Attempt To Register The Window Class
  atom = RegisterClass(&wclss);
  NSAssert(atom, @"Failed To Register The Win32Subwindow MS window class.");

  NSDebugMLLog(@"WGL", @"MS window class initialized (%u)", atom);
}

- (id) initWithView: (NSOpenGLView *)view
{
  NSRect rect;
  WIN32Server *server;
  NSWindow *win;
  int x, y, width, height;
  HINSTANCE hInstance;
  WNDCLASS wclss;
  ATOM atom;
  RECT parent_rect;

  self = [super init];
  if (!self)
    return nil;

  attached = (NSOpenGLView*)view;

  win = [view window];
  NSAssert(win, @"request of a window attachment on a view that is not on a NSWindow");
  
  if ([view isRotatedOrScaledFromBase])
    [NSException raise: NSInvalidArgumentException
                 format: @"Cannot attach an window to a view that is rotated or scaled"];
  
  server = (WIN32Server *)GSServerForWindow(win);
  NSAssert(server != nil, NSInternalInconsistencyException);

  NSAssert([server isKindOfClass: [WIN32Server class]], 
           NSInternalInconsistencyException);

  hInstance = GetModuleHandle(NULL);

  /* Grab the window class we have registered on [+initialize] */
  atom = GetClassInfo( hInstance, NSOPENGLSUBWINDOWCLASS, &wclss );
  NSAssert(atom, @"MS window class not found !");

  GetClientRect((HWND)[win windowNumber], &parent_rect);
  if ([server handlesWindowDecorations] == YES)
    {
      /* The window manager handles window decorations, so the
       * the parent X window is equal to the content view and
       * we must therefore use content view coordinates.
       */
      rect = [view convertRect: [view bounds] toView: [[attached window] contentView]];
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
      rect = [view convertRect: [view bounds] toView: nil];
    }
  x = NSMinX(rect);
  y = (parent_rect.bottom - parent_rect.top) - NSMaxY(rect);
  width = NSWidth(rect);
  height = NSHeight(rect);
  
  NSDebugMLLog(@"WGL", @"MS window creation (%d, %d, %u, %u)", x, y, width, height);
  
  /* WS_DISABLED causes mouse/keyboard events to be forwarded to the parent window
     so they can be processed normally; otherwise the OpenGL window would eat them */
  winid = CreateWindow(
      NSOPENGLSUBWINDOWCLASS, NSOPENGLSUBWINDOWNAME, 
      WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_DISABLED, 
      x, y, width, height, 
      (HWND)[win windowNumber], (HMENU)NULL, hInstance, (LPVOID)self);

  NSAssert(winid, @"Failed to create a MS window");
  
  ShowCursor( TRUE );

  return self;
}

- (void) map
{
  ShowWindow(winid, SW_SHOW);
}

- (void) detach
{
  DestroyWindow(winid);
  attached = nil;
}

- (void) update
{
  NSRect rect;
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

  NSAssert([server isKindOfClass: [WIN32Server class]], 
	   NSInternalInconsistencyException);

  //FIXME
  //we should check that the window hasn't changed, maybe.

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

  RECT parent_rect;
  GetClientRect((HWND)[win windowNumber], &parent_rect);

  x = NSMinX(rect);
  y = (parent_rect.bottom - parent_rect.top) - NSMaxY(rect);
  width = NSWidth(rect);
  height = NSHeight(rect);
  
  MoveWindow(winid, x, y, width, height, /*FIXME*/TRUE);
}

- (void) dealloc
{
  NSDebugMLLog(@"WGL", @"deallocating");
  [self detach];
  [super dealloc];
}

+ (id) subwindowOnView: (NSOpenGLView *) view
{
  Win32Subwindow *win = [[self alloc] initWithView: view];

  return AUTORELEASE(win);
}
@end

//FIXME:
//should be on per thread basis.
static Win32GLContext *currentGLContext;

@implementation Win32GLContext

+ (void)clearCurrentContext
{
  wglMakeCurrent(NULL, NULL);
  currentGLContext = nil;
}

+ (NSOpenGLContext *)currentContext
{
  return currentGLContext;
}

- (void) _detach
{
  if (wsubwin)
    {
      if (currentGLContext == self)
	{
	  [Win32GLContext clearCurrentContext];
	}
      wsubwin->hDC = (HDC)NULL;
      DESTROY(wsubwin);
    }
}

- (void *)CGLContextObj
{
  // FIXME: Until we have a wrapper library
  // return the underlying context directly
  return (void*)wgl_context;
}

- (void)clearDrawable
{
  [self _detach];
}

- (void)copyAttributesFromContext:(NSOpenGLContext *)context 
			 withMask:(unsigned long)mask
{
  HGLRC other;

  if (context == nil ||  ![context isKindOfClass: [Win32GLContext class]])
    [NSException raise: NSInvalidArgumentException
		 format: @"%@ is an invalid context", context];
  other = ((Win32GLContext *)context)->wgl_context;
  wglCopyContext(other, wgl_context, mask);
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
  SwapBuffers(wsubwin->hDC);
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
  wgl_context = context;
  return self;
}

- (id)initWithFormat:(NSOpenGLPixelFormat *)aFormat
	shareContext:(NSOpenGLContext *)share
{
  NSDebugMLLog(@"WGL", @"will init with format %@", aFormat);
  self = [super init];
  if(self)
  {
    wgl_context = NULL;
  
    if (aFormat && [aFormat isKindOfClass: [Win32GLPixelFormat class]])
    {
      ASSIGN(format, (Win32GLPixelFormat *)aFormat);
      //FIXME: allow index mode and sharing
      wgl_context = NULL;
    }
    else
    {
      NSDebugMLLog(@"WGL", @"invalid format %@", aFormat);
      DESTROY(self);
    }
  }
  return self;
}


- (void) dealloc
{
  NSDebugMLLog(@"WGL", @"deallocating");
  [self _detach];
  RELEASE(format);
  if (wgl_context)
    {
      wglDeleteContext(wgl_context);
    }
  [super dealloc];
}

- (void) makeCurrentContext
{
  if (wsubwin == nil)
    [NSException raise: NSGenericException
		 format: @"GL Context is not bind, cannot be made current"];
  
  NSAssert(wgl_context && wsubwin->hDC, 
	   NSInternalInconsistencyException);

  NSDebugMLLog(@"WGL", @"before wglMakeCurrent");
  wglMakeCurrent(wsubwin->hDC, wgl_context);
  NSDebugMLLog(@"WGL", @"after wglMakeCurrent");

//   NSAssert(glx_context != None,   NSInternalInconsistencyException);

//   glXMakeCurrent(dpy, xsubwin->winid, glx_context);

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
  Win32Subwindow *win;
  NSView *current_view;
  
  if (!view)
    [NSException raise: NSInvalidArgumentException
		 format: @"setView called with a nil value"];

  NSAssert(format, NSInternalInconsistencyException);
  win = [Win32Subwindow subwindowOnView: (NSOpenGLView*) view];
  current_view = [self view];
  if ( current_view != nil )
    {
      [current_view _setIgnoresBacking: saved_ignores_backing];
    }
  ASSIGN(wsubwin, win);
  saved_ignores_backing = [view _ignoresBacking];
  [view _setIgnoresBacking: YES];

//   {
//     GLXFBConfig  *conf_tab;
//     int		n_elem;
//     int attrs[] = { 
//       GLX_DOUBLEBUFFER, 1, 
//       GLX_DEPTH_SIZE, 16, 
//       GLX_RED_SIZE, 1, 
//       GLX_BLUE_SIZE, 1, 
//       GLX_GREEN_SIZE, 1, 
//       None
//     };    
  
//     conf_tab = glXChooseFBConfig(dpy, DefaultScreen(dpy), attrs,  &n_elem);
//     if (n_elem > 0)
//       {
// 	printf("found %d context\n", n_elem);
// // 	win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 10, 10, 
// // 				  800, 600, 1, 0, 1);
// 	glx_drawable = glXCreateWindow(dpy, *conf_tab, xsubwin->winid,  NULL);
      
//       }
//     else
//       puts("no context found");


//   }	

//FIXME
//The following line should be the good one.  But it crashes my X server...

//   glx_drawable = glXCreateWindow(dpy, *format->conf_tab, xsubwin->winid, 
// 				 NULL);
  NSDebugMLLog(@"WGL", @"wgl_window : %u", win);
}


- (void)update
{
  [wsubwin update];
}


- (NSView *)view
{
  if (wsubwin)
    return (NSView*)wsubwin->attached;
  else
    return nil;
}

@end

LRESULT CALLBACK win32SubwindowProc( 
    HWND hWnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam )
{
  Win32Subwindow *wsubwin;
  Win32GLContext* ctx = nil;

  NSDebugFLLog(@"WGLEvents", @"Entering.");
  wsubwin = (Win32Subwindow*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if(wsubwin) ctx = (Win32GLContext*)[wsubwin->attached openGLContext];

  switch (message) {
    case WM_CREATE:
      NSDebugFLLog(@"WGLEvents", @"WM_CREATE event received.");
      wsubwin = (Win32Subwindow*)(((LPCREATESTRUCT) lParam)->lpCreateParams);
      ctx = (Win32GLContext*)[wsubwin->attached openGLContext];
      NSDebugFLLog(@"WGLEvents", @"subwindow : %@", wsubwin);
      // initialize OpenGL rendering
      NSCAssert(wsubwin->hDC = GetDC(hWnd), @"No DC");
      [ctx->format _setDrawable: wsubwin->hDC];
      //TODO setupPalette(wsubwin->hDC);
      NSCAssert(
	  ctx->wgl_context = wglCreateContext(wsubwin->hDC), 
	  @"wglCreateContext failed");
//      wglMakeCurrent(wsubwin->hDC, hGLRC)
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wsubwin);
      NSDebugFLLog(@"WGLEvents", @"WM_CREATE done.");
      return 0;
    case WM_DESTROY:
      NSDebugFLLog(@"WGLEvents", @"WM_DESTROY event received.");
      // finish OpenGL rendering

      if(ctx->wgl_context) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(ctx->wgl_context);
      }
/*      if (hPalette) {
        DeleteObject(hPalette);
      }*/
      ReleaseDC(hWnd, wsubwin->hDC);
      NSDebugFLLog(@"WGLEvents", @"WM_DESTROY done.");
      return 0;
/*    case WM_SIZE:
      // track window size changes
      if (wsubwin->context->wgl_context) {
        winWidth = (int) LOWORD(lParam);
        winHeight = (int) HIWORD(lParam);
        resize();
        return 0;
      }*/
/*    case WM_PALETTECHANGED:
      // realize palette if this is *not* the current window
      if (wsubwin->context->wgl_context && hPalette && (HWND) wParam != hWnd) {
        UnrealizeObject(hPalette);
        SelectPalette(wsubwin->hDC, hPalette, FALSE);
        RealizePalette(wsubwin->hDC);
        redraw();
        break;
      }
      break;*/
/*    case WM_QUERYNEWPALETTE:
      // realize palette if this is the current window
      if ((wsubwin->context->wgl_context && hPalette) {
        UnrealizeObject(hPalette);
        SelectPalette(wsubwin->hDC, hPalette, FALSE);
        RealizePalette(wsubwin->hDC);
        redraw();
        return TRUE;
      }
      break;*/
    case WM_PAINT:
      {
        RECT wr;
        RECT ir;
        
        if ( GetWindowRect(hWnd,&wr) && GetUpdateRect(hWnd, &ir, FALSE) )
          {
            ir.top  += wr.top;
            ir.left += wr.left;
            InvalidateRect( GetParent(hWnd), &ir,FALSE );
          }
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
      break;
/*    case WM_CHAR:
      // handle keyboard input
      switch ((int)wParam) {
        case VK_ESCAPE:
          DestroyWindow(hWnd);
          return 0;
        default:
          break;
      }
      break;*/
    default:
      NSDebugFLLog(@"WGLEvents", @"other event received (%u).", message);
//      return MainWndProc((HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPARENT), message, wParam, lParam);
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
  }
  NSDebugFLLog(@"WGLEvents", @"Failed.");
  return FALSE;
}


#endif
