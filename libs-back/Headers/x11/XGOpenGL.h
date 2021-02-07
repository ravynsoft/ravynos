/* 	-*-ObjC-*- */
/* XGOpenGL - openGL management using glX

   Copyright (C) 2002 Free Software Foundation, Inc.

   Author: Frederic De Jaeger
   Date: Nov 2002

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

#ifndef _GNUstep_H_XGOpenGL_
#define _GNUstep_H_XGOpenGL_

#include <AppKit/NSOpenGL.h>

#define id _gs_avoid_id_collision
#define BOOL XWINDOWSBOOL
#include <GL/glx.h>
#undef id
#undef BOOL

@class NSView;
@class XGXSubWindow;
@class XGGLPixelFormat;

@interface XGGLContext : NSOpenGLContext
{
  int glxminorversion;
  GLXContext        glx_context;
  GLXWindow         glx_drawable;
  XGXSubWindow     *xSubWindow;
  XGGLPixelFormat  *pixelFormat;
  BOOL              saved_ignores_backing;
}

- (GLXContext)glxcontext;

@end

@interface XGGLPixelFormat : NSOpenGLPixelFormat
{
  Display * display;
  int glxminorversion;

  GLXFBConfig  *fbconfig;
  int pickedFBConfig;
  int configurationCount;
  BOOL shouldRequestARGBVisual;

  XVisualInfo  *visualinfo;

}

+ (int) glxMinorVersion;
- (Display *) display;
- (XVisualInfo *) visualinfo;
- (GLXContext) createGLXContext: (XGGLContext *)share;
- (GLXWindow) drawableForWindow: (Window)xwindowid;

@end

#endif
