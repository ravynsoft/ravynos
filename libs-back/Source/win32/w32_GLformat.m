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
#include <Foundation/NSData.h>
#include <GNUstepGUI/GSDisplayServer.h>
#include "win32/WIN32Server.h"
#include "win32/WIN32OpenGL.h"

static void attributesNS2WGL( NSOpenGLPixelFormatAttribute *attribs, LPPIXELFORMATDESCRIPTOR ppfd )
{
  // TODO : switch to wglChoosePixelFormatEXT 
  NSOpenGLPixelFormatAttribute *ptr = attribs;

  ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
  ppfd->nVersion = 1;
  ppfd->dwFlags = 0;

  ppfd->dwFlags |= PFD_SUPPORT_OPENGL;
  ppfd->iPixelType = PFD_TYPE_RGBA;

  while (*ptr)
    {
      switch(*ptr)
	{
	// it means all the same on WGL - there is no difference here
	case NSOpenGLPFAWindow:
	  ppfd->dwFlags |= PFD_DRAW_TO_WINDOW;
	  break;
	case NSOpenGLPFAOffScreen:
	  ppfd->dwFlags |= PFD_DRAW_TO_BITMAP;
	  break;
        case NSOpenGLPFAPixelBuffer: // TODO
	  //ppfd->dwFlags |= PFD_DRAW_TO_WINDOW;
	  break;
	case NSOpenGLPFASingleRenderer:
	case NSOpenGLPFAAllRenderers:
	case NSOpenGLPFAAccelerated:
	  ppfd->dwFlags |= PFD_GENERIC_ACCELERATED;
	  break;
	case  NSOpenGLPFADoubleBuffer:
	  ppfd->dwFlags |= PFD_DOUBLEBUFFER;
	  break;
	case NSOpenGLPFAStereo:
	  ppfd->dwFlags |= PFD_STEREO;
	  break;
	case NSOpenGLPFABackingStore:
	  ppfd->dwFlags |= PFD_SWAP_COPY;
	  break;
	case NSOpenGLPFAAuxBuffers:
	  ptr++;
	  ppfd->cAuxBuffers = *ptr;
	  break;
	case NSOpenGLPFAColorSize:
	  ptr++;
	  ppfd->cColorBits = *ptr;
	  break;
	case NSOpenGLPFAAlphaSize:
	  ptr++;
	  ppfd->cAlphaBits = *ptr;
	  break;
	case NSOpenGLPFADepthSize:
	  ptr++;
	  ppfd->cDepthBits = *ptr;
	  break;
	case NSOpenGLPFAStencilSize:
	  ptr++;
	  ppfd->cStencilBits = *ptr;
	  break;
	case NSOpenGLPFAAccumSize:
	  ptr++;
	  ppfd->cAccumBits = *ptr;
	  break;
	switch ((int)*ptr)
		{
		case 8:
		 	ppfd->cAccumRedBits = 3;
		 	ppfd->cAccumGreenBits = 3;
		 	ppfd->cAccumBlueBits = 2;
		 	ppfd->cAccumAlphaBits = 0;
		 	break;
		case 15:
		case 16:
		 	ppfd->cAccumRedBits = 5;
		 	ppfd->cAccumGreenBits = 5;
		 	ppfd->cAccumBlueBits = 5;
		 	ppfd->cAccumAlphaBits = 0;
			break;
		case 24:
		 	ppfd->cAccumRedBits = 8;
		 	ppfd->cAccumGreenBits = 8;
		 	ppfd->cAccumBlueBits = 8;
		 	ppfd->cAccumAlphaBits = 0;
			break;
		case 32:
		 	ppfd->cAccumRedBits = 8;
		 	ppfd->cAccumGreenBits = 8;
		 	ppfd->cAccumBlueBits = 8;
		 	ppfd->cAccumAlphaBits = 8;
			break;
		}
		break;
	//can not be handle by WGL
	case NSOpenGLPFAMinimumPolicy:
	  break;
	// can not be handle by WGL
	case NSOpenGLPFAMaximumPolicy:
	  break;

	//FIXME all of this stuff...
	case NSOpenGLPFAFullScreen:
	case NSOpenGLPFASampleBuffers:
	case NSOpenGLPFASamples:
	case NSOpenGLPFAAuxDepthStencil:
	case NSOpenGLPFARendererID:
	case NSOpenGLPFANoRecovery:
	case NSOpenGLPFAClosestPolicy:
	case NSOpenGLPFARobust:
	case NSOpenGLPFAMPSafe:
	case NSOpenGLPFAMultiScreen:
	case NSOpenGLPFACompliant:
	case NSOpenGLPFAScreenMask:
	case NSOpenGLPFAVirtualScreenCount:

        case NSOpenGLPFAAllowOfflineRenderers:
        case NSOpenGLPFAColorFloat:
        case NSOpenGLPFAMultisample:
        case NSOpenGLPFASupersample:
        case NSOpenGLPFASampleAlpha:
	  break;
	}
      ptr ++;
    }
}

static void attributesWGL2NS( LPPIXELFORMATDESCRIPTOR ppfd, NSOpenGLPixelFormatAttribute *attribs )
{
  // TODO
}

@implementation Win32GLPixelFormat

- (void)getValues:(long *)vals 
     forAttribute:(NSOpenGLPixelFormatAttribute)attrib 
 forVirtualScreen:(int)screen
{
  PIXELFORMATDESCRIPTOR pfd;

  //glXGetConfig(dpy, conf.visual, attrib, (int *)vals);
  DescribePixelFormat( wgl_drawable, wgl_pixelformat, sizeof(pfd), &pfd);
}

- (id)initWithAttributes: (NSOpenGLPixelFormatAttribute *) attribs
{
  NSDebugMLLog(@"WGL", @"will init");
  self = [super init];
  if(self)      
  {
    wgl_drawable = 0;
    wgl_pixelformat = 0;

    attributesNS2WGL(attribs, &pfd);
  }
  return self;
}
#if 0
  //FIXME, what screen number ?
  if (GSglxMinorVersion (dpy) >= 3)
    conf.tab = glXChooseFBConfig(dpy, DefaultScreen(dpy), [data mutableBytes], 
				 &n_elem);
  else
    conf.visual = glXChooseVisual(dpy, DefaultScreen(dpy), 
				  [data mutableBytes]);
  
  if (((GSglxMinorVersion (dpy) >= 3) 
	? (void *)conf.tab : (void *)conf.visual)
       == NULL)
    {
      NSDebugMLLog(@"GLX", @"no pixel format found matching what is required");
      RELEASE(self);
      return nil;
    }
  else
    {
      
      NSDebugMLLog(@"GLX", @"We found %d pixel formats", n_elem);
#if 0
      if (GSglxMinorVersion (dpy) >= 3)
	{	
	  int i;
	  for (i = 0; i < n_elem; ++i)
	    {
	      int val;
	      NSDebugMLLog(@"GLX", @"inspecting %dth", i+1);
	      glXGetFBConfigAttrib(dpy, conf.tab[i], GLX_BUFFER_SIZE, &val);
	      NSDebugMLLog(@"GLX", @"buffer size %d", val);
	      
	      
	      glXGetFBConfigAttrib(dpy, conf.tab[i], GLX_DOUBLEBUFFER, &val);
	      NSDebugMLLog(@"GLX", @"double buffer %d", val);
	      
	      glXGetFBConfigAttrib(dpy, conf.tab[i], GLX_DEPTH_SIZE, &val);
	      NSDebugMLLog(@"GLX", @"depth size %d", val);
	      
	    }
	}
      else
	{
	  glXGetConfig(dpy, conf.visual, GLX_BUFFER_SIZE, &val);
	  NSDebugMLLog(@"GLX", @"buffer size %d", val);
	  
	  
	  glXGetConfig(dpy, conf.visual, GLX_DOUBLEBUFFER, &val);
	  NSDebugMLLog(@"GLX", @"double buffer %d", val);
	  
	  glXGetConfig(dpy, conf.visual, GLX_DEPTH_SIZE, &val);
	  NSDebugMLLog(@"GLX", @"depth size %d", val);
	}
#endif      
      return self;
    }
}
#endif

- (void) _setDrawable: (HDC) aDrawable
{
  // TODO : switch to wglChoosePixelFormatEXT 

  NSCAssert(
      wgl_pixelformat = ChoosePixelFormat(aDrawable, &pfd), 
      @"ChoosePixelFormat failed.");
  NSCAssert(
      SetPixelFormat(aDrawable, wgl_pixelformat, &pfd), 
      @"SetPixelFormat failed.");
  wgl_drawable = aDrawable;

  NSDebugFLLog(@"WGL", @"found : %u", wgl_pixelformat);
}

- (void) dealloc
{
  NSDebugMLLog(@"WGL", @"deallocation");
  [super dealloc];
}

- (int)numberOfVirtualScreens
{
  //  [self notImplemented: _cmd];
  //FIXME
  //This looks like a reasonable value to return...
  return 1;
}

@end
#endif
