/* 	-*-ObjC-*- */
/* Win32OpenGL - openGL management using wgl

   Copyright (C) 2002 Free Software Foundation, Inc.

   Author: Xavier Glattard
   Date: Jan 2007

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

#ifndef _GNUstep_H_WIN32OpenGL
#define _GNUstep_H_WIN32OpenGL

#include <AppKit/NSOpenGL.h>

#define id _gs_avoid_id_collision
#define BOOL WINDOWSBOOL
#include <windows.h>
#undef id
#undef BOOL

@class NSView;
@class Win32Subwindow;
@class Win32GLPixelFormat;

@interface Win32GLContext : NSOpenGLContext
{
  @public
  HGLRC			wgl_context;
  Win32Subwindow	*wsubwin;
  Win32GLPixelFormat 	*format;
  BOOL              saved_ignores_backing;
}
@end

@interface Win32GLPixelFormat : NSOpenGLPixelFormat
{
  @public
  PIXELFORMATDESCRIPTOR pfd;
  HDC	wgl_drawable;
  int	wgl_pixelformat;
}
- (void) _setDrawable: (HDC) aDrawable;
@end

#endif
